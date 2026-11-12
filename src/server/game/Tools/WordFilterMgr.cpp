////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#include "WordFilterMgr.h"
#include "DatabaseEnv.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include <boost/locale/encoding_utf.hpp>
#include <cwctype>

#define MAX_SIZE_SENTENCE 27

WordFilterMgr::WordFilterMgr() 
{
}

WordFilterMgr::~WordFilterMgr()
{
}

WordFilterMgr* WordFilterMgr::instance()
{
    static WordFilterMgr instance;
    return &instance;
}

void WordFilterMgr::LoadLetterAnalogs()
{
    uint32 oldMSTime = getMSTime();

    m_letterAnalogs.clear();

    QueryResult result = WorldDatabase.Query("SELECT letter, analogs FROM letter_analogs");
    if (!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 letter analogs. DB table `letter_analogs` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        std::wstring letter = boost::locale::conv::utf_to_utf<wchar_t>(fields[0].GetString()); // !fields[0].GetInt8()
        std::wstring analogs = boost::locale::conv::utf_to_utf<wchar_t>(fields[1].GetString());

        for (uint32 i = 0; i < analogs.size(); ++i)
            m_letterAnalogs[analogs[i]] = letter[0];

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u (%zu) letter analogs in %u ms", count, m_letterAnalogs.size(), GetMSTimeDiffToNow(oldMSTime));
}


void WordFilterMgr::LoadBadWords()
{
    uint32 oldMSTime = getMSTime();

    {
        std::unique_lock<std::shared_mutex> lock(m_lock);
        m_badWords.clear();
        m_badWordsMail.clear();
    }

    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT `bad_word`, `convert` FROM bad_word");
    if (!result)
        TC_LOG_INFO("server.loading",">> Loaded 0 bad words. DB table `bad_word` is empty!");
    else
    {
        do
        {
            Field* fields = result->Fetch();
            std::string analog = fields[0].GetString();
            std::string convert = fields[1].GetString();

            if (convert.empty())
                AddBadWord(analog);
            else
                AddConvert(convert);

            ++count;
        }
        while (result->NextRow());
    }

    result = WorldDatabase.Query("SELECT bad_word FROM bad_word_mail");
    if (!result)
        TC_LOG_INFO("server.loading",">> Loaded 0 bad words. DB table `bad_word_mail` is empty!");
    else
    {
        do
        {
            Field* fields = result->Fetch();
            std::string analog = fields[0].GetString();

            AddBadWordMail(analog);

            ++count;
        }
        while (result->NextRow());
    }

    TC_LOG_INFO("server.loading",">> Loaded %u bad words in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WordFilterMgr::LoadBadSentences()
{
    uint32 oldMSTime = getMSTime();

    BadSentences sentences;
    std::map<uint32, size_t> idToHash;
    uint32 lastId = 0;

    auto install = [&]()
    {
        std::unique_lock<std::shared_mutex> lock(m_lock);
        m_badSentences = std::move(sentences);
        hashById = std::move(idToHash);
        lastIdBadSentences = lastId;
    };

    QueryResult result = CharacterDatabase.Query("SELECT hash, sentence, id, penalty, sourceMask, output FROM bad_sentences order by id");
    if (!result)
    {
        install();
        TC_LOG_INFO("server.loading", ">> Loaded 0 bad sentences. DB table `bad_sentences` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        size_t hash_bd = fields[0].GetUInt64();
               
        size_t hash = FilterMsgAndGetHash(fields[1].GetString());

        // only the first characters are hashed: a later sentence sharing them is left out of memory, its row stays
        auto sameHash = sentences.find(hash);
        if (sameHash != sentences.end())
        {
            TC_LOG_WARN("sql.sql", "bad_sentences: id %u shares its normalized prefix with id %u and is ignored.", fields[2].GetUInt32(), sameHash->second.id);
            continue;
        }

        if (hash != hash_bd)
            CharacterDatabase.PExecute("UPDATE bad_sentences set hash = " UI64FMTD " where id = %u", hash, fields[2].GetUInt32());


        BadSentenceInfo & info = sentences[hash];
        info.text = fields[1].GetString();
        info.id = fields[2].GetUInt32();
        lastId = std::max(lastId, info.id);
        info.penalty = fields[3].GetInt32();
        info.sourceMask = fields[4].GetUInt32();
        if (fields[5].GetString().empty())
        {
            std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(info.text);
            GeneralFilterWstring(_badWord);
            if (_badWord.size() > MAX_SIZE_SENTENCE)
                _badWord.resize(MAX_SIZE_SENTENCE);
            std::string _badWordEsc = boost::locale::conv::utf_to_utf<char>(_badWord);
            CharacterDatabase.EscapeString(_badWordEsc);
            CharacterDatabase.PExecute("UPDATE bad_sentences set output = '%s' where id = %u", _badWordEsc.c_str(), info.id);
        }

        idToHash[info.id] = hash;

        ++count;
    } while (result->NextRow());

    install();

    TC_LOG_INFO("server.loading", ">> Loaded %u bad sentences in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WordFilterMgr::LoadComplaints()
{
    uint32 oldMSTime = getMSTime();

    m_complaints.clear();

    QueryResult result = CharacterDatabase.Query("SELECT ID, ReportPlayer, SpammerGuid, MessageLog FROM report_complaints where JustBanned = 0 and (CAST(ReportTime AS SIGNED) - CAST(TimeSinceOffence AS SIGNED)) >= CAST(UNIX_TIMESTAMP() AS SIGNED) - 86400");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 report_complaints. DB table `report_complaints` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        AddComplaintForUser(ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64()),
                            ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt64()), fields[0].GetUInt32(), fields[3].GetString());
        
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u report_complaints in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}


std::string WordFilterMgr::FindBadWord(std::string const& text, bool mail)
{
    std::wstring _text = boost::locale::conv::utf_to_utf<wchar_t>(text);
    
    GeneralFilterWstring(_text);

    std::shared_lock<std::shared_mutex> lock(m_lock);

    if (_text.empty() || m_badWords.empty())
        return "";

    for (BadWordMap::const_iterator it = m_badWords.begin(); it != m_badWords.end(); ++it)
    {
        if (_text.find(*it) != std::wstring::npos)
        {
            // TC_LOG_INFO("server.loading", "Compare text %s _text %s *it %s", text.c_str(), boost::locale::conv::utf_to_utf<char>(_text).c_str(), boost::locale::conv::utf_to_utf<char>(*it).c_str());
            return boost::locale::conv::utf_to_utf<char>(*it);
        }
    }

    if(mail)
    {
        for (BadWordMapMail::const_iterator it = m_badWordsMail.begin(); it != m_badWordsMail.end(); ++it)
        {
        //    TC_LOG_INFO("server.loading", "Compare %s and %s", boost::locale::conv::utf_to_utf<char>(*it).c_str(), boost::locale::conv::utf_to_utf<char>(_text).c_str());
            if (_text.find(*it) != std::wstring::npos)
                return boost::locale::conv::utf_to_utf<char>(*it);
        }
    }
    return "";
}

bool WordFilterMgr::AddBadWord(std::string const& badWord, bool toDB)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);

    GeneralFilterWstring(_badWord);

    // a word made only of ignored characters would match every text
    if (_badWord.empty())
        return false;

    std::string escapedWord = badWord;
    WorldDatabase.EscapeString(escapedWord);
    std::string escapedConvert = boost::locale::conv::utf_to_utf<char>(_badWord);
    WorldDatabase.EscapeString(escapedConvert);

    {
        std::unique_lock<std::shared_mutex> lock(m_lock);

        // is already exist
        if (!m_badWords.insert(_badWord).second)
        {
            lock.unlock();
            WorldDatabase.PExecute("DELETE FROM bad_word WHERE bad_word = '%s'", escapedWord.c_str());
            return false;
        }
    }

    if (toDB)
        WorldDatabase.PExecute("REPLACE INTO bad_word VALUES ('%s', '%s')", escapedWord.c_str(), escapedConvert.c_str());
    else
        WorldDatabase.PExecute("UPDATE bad_word SET `convert` = '%s' WHERE bad_word = '%s'", escapedConvert.c_str(), escapedWord.c_str());

    return true;
}

void WordFilterMgr::AddConvert(std::string const& badWord)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);
    if (_badWord.empty())
        return;

    std::unique_lock<std::shared_mutex> lock(m_lock);
    m_badWords.insert(_badWord);
}

bool WordFilterMgr::AddBadWordMail(std::string const& badWord, bool toDB)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);
    GeneralFilterWstring(_badWord);

    // a word made only of ignored characters would match every text
    if (_badWord.empty())
        return false;

    {
        std::unique_lock<std::shared_mutex> lock(m_lock);

        // is already exist
        if (!m_badWordsMail.insert(_badWord).second)
            return false;
    }

    if (toDB)
    {
        std::string escapedWord = badWord;
        WorldDatabase.EscapeString(escapedWord);
        WorldDatabase.PExecute("REPLACE INTO bad_word_mail VALUES ('%s')", escapedWord.c_str());
    }

    return true;
}

bool WordFilterMgr::RemoveBadWord(std::string const& badWord, bool fromDB)
{
    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(badWord);
    GeneralFilterWstring(_badWord);

    {
        std::unique_lock<std::shared_mutex> lock(m_lock);

        // is not exist
        BadWordMap::iterator it = m_badWords.find(_badWord);
        if (it == m_badWords.end())
            return false;

        m_badWords.erase(it);
    }

    if (fromDB)
    {
        std::string escapedWord = badWord;
        WorldDatabase.EscapeString(escapedWord);
        WorldDatabase.PExecute("DELETE FROM bad_word WHERE `bad_word` = '%s'", escapedWord.c_str());
    }

    return true;
}

bool WordFilterMgr::AddBadSentence(std::string badSentence, size_t hash, uint32 mask, int32 penalty /* = 10 */)
{
    std::unique_lock<std::shared_mutex> lock(m_lock);

    BadSentenceInfo& info = m_badSentences[hash];
    if (info.id != 0 && mask == info.sourceMask && penalty == info.penalty)
        return false;

    info.text = badSentence;
    info.id = (info.id == 0 ? ++lastIdBadSentences : info.id);
    info.penalty = penalty;
    info.sourceMask = info.sourceMask | mask;

    hashById[info.id] = hash;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_BAD_SENTENCES);
    stmt->setUInt32(0, info.id);
    stmt->setString(1, info.text);
    stmt->setUInt64(2, hash);
    stmt->setInt32(3, penalty);
    stmt->setUInt32(4, mask);

    std::wstring _badWord = boost::locale::conv::utf_to_utf<wchar_t>(info.text);

    GeneralFilterWstring(_badWord);

    if (_badWord.size() > MAX_SIZE_SENTENCE)
        _badWord.resize(MAX_SIZE_SENTENCE);

    stmt->setString(5, boost::locale::conv::utf_to_utf<char>(_badWord).c_str());

    CharacterDatabase.Execute(stmt);

    return true;
}

void WordFilterMgr::UpdatePenaltyForBadSentenceById(uint32 id, int32 penalty)
{
    {
        std::unique_lock<std::shared_mutex> lock(m_lock);
        auto hashItr = hashById.find(id);
        if (hashItr != hashById.end())
        {
            auto sentenceItr = m_badSentences.find(hashItr->second);
            if (sentenceItr != m_badSentences.end())
                sentenceItr->second.penalty = penalty;
        }
    }

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_BAD_SENTENCES);
    stmt->setInt32(0, penalty);
    stmt->setUInt32(1, id);
    CharacterDatabase.Execute(stmt);
}

size_t WordFilterMgr::FilterMsgAndGetHash(std::string lowedMsg, LocaleConstant locale /*= LOCALE_enUS*/, bool* isValid /*= nullptr*/)
{
    std::wstring wstrLowed = boost::locale::conv::utf_to_utf<wchar_t>(lowedMsg);

    GeneralFilterWstring(wstrLowed, locale, isValid);

    //if (isValid && !(*isValid))
    //    return 0;

    if (wstrLowed.size() > MAX_SIZE_SENTENCE)
        wstrLowed.resize(MAX_SIZE_SENTENCE);
    
    size_t hash;
    if (wstrLowed.size() <= 2) // but was 5
        hash = hash_fn(boost::locale::conv::utf_to_utf<wchar_t>(lowedMsg)); // origial message
    else
        hash = hash_fn(wstrLowed);

    return hash;
}

void WordFilterMgr::GeneralFilterWstring(std::wstring& _text, LocaleConstant locale /*= LOCALE_enUS*/, bool* isValid /*= nullptr*/)
{
    if (_text.empty())
        return;

    // ::tolower is undefined above 255 and a wchar_t holds more than that
    std::transform(_text.begin(), _text.end(), _text.begin(), [](wchar_t c) -> wchar_t { return wchar_t(std::towlower(wint_t(c))); });

    std::wstring letters = boost::locale::conv::utf_to_utf<wchar_t>(sObjectMgr->GetTrinityString(20087, LOCALE_ruRU)); // all eng + all rus chars
    for (uint16 i = 0; i < _text.size(); ++i)
    {
        auto posItr = letters.find(_text[i]);
        if (posItr != std::wstring::npos && posItr >= 52 + 33)  // big russian symbols to lower
            _text[i] = letters[posItr - 33];
    }
    

    std::string examples = sObjectMgr->GetTrinityString(20087, LOCALE_enUS); // some strange symbols like {треугольник}
    Tokenizer tokens(examples, ' ');

    for (auto exstring : tokens)
    {
        std::wstring example = boost::locale::conv::utf_to_utf<wchar_t>(exstring);
        do
        {
            auto first = _text.find(example);
            if (first == std::wstring::npos)
                break;
            _text.erase(first, example.size());
        } while (true);
    }

    std::wstring symbols = boost::locale::conv::utf_to_utf<wchar_t>(sObjectMgr->GetTrinityString(20087, LOCALE_koKR)); // all correct symbols

    bool tempValid = true;
    uint8 rusSymbols = 0;
    uint8 i = 0;
    for (; i < _text.size() && i < 50;)
    {
        
     /*   auto it = m_letterAnalogs.find(_text[i]);
        if (it != m_letterAnalogs.end())
            _text[i] = (*it).second; */

        auto posItr = letters.find(_text[i]);
        if (posItr == letters.npos) // if this char not a-z а-я, then erase it
        {
            if (isValid)
                if (symbols.find(_text[i]) == symbols.npos) // it is not correct symbols too
                {
                    if (locale == LOCALE_ruRU || locale == LOCALE_enUS)
                        *isValid = false;
                    else
                        tempValid = false;
                }

            _text.erase(i, 1);
        }
        else
        {
            if (posItr >= 52)
                ++rusSymbols;
            ++i;
        }
    }

    if (!tempValid && (float(rusSymbols) / std::min(float(_text.size()), 50.0f)) >= 0.9f)
        *isValid = false;

    // the checks above only look at the first 50 kept characters, but ignored characters are dropped everywhere
    // so that a forbidden word cannot be hidden behind filler
    for (size_t j = i; j < _text.size();)
    {
        if (letters.find(_text[j]) == letters.npos)
            _text.erase(j, 1);
        else
            ++j;
    }
}

WordFilterMgr::BadWordMap WordFilterMgr::GetBadWords() const
{
    std::shared_lock<std::shared_mutex> lock(m_lock);
    return m_badWords;
}


// find, not operator[]: every checked message would otherwise insert an empty entry
uint32 WordFilterMgr::GetIdOfBadSentence(size_t hash)
{
    std::shared_lock<std::shared_mutex> lock(m_lock);
    auto itr = m_badSentences.find(hash);
    return itr != m_badSentences.end() ? itr->second.id : 0;
}

int32 WordFilterMgr::GetPenaltyForBadSentence(size_t hash)
{
    std::shared_lock<std::shared_mutex> lock(m_lock);
    auto itr = m_badSentences.find(hash);
    return itr != m_badSentences.end() && itr->second.id ? itr->second.penalty : 0;
}

uint32 WordFilterMgr::GetSourceMaskForBadSentence(size_t hash)
{
    std::shared_lock<std::shared_mutex> lock(m_lock);
    auto itr = m_badSentences.find(hash);
    return itr != m_badSentences.end() && itr->second.id ? itr->second.sourceMask : 0;
}

std::vector<BadSentenceInfo> WordFilterMgr::GetBadSentenceList(uint32 page)
{
    std::shared_lock<std::shared_mutex> lock(m_lock);

    std::vector<BadSentenceInfo> list{};
    for (uint32 i = 1 + (page - 1) * 25; i <= page * 25 && i <= lastIdBadSentences; ++i)
    {
        auto hashItr = hashById.find(i);
        if (hashItr == hashById.end())
            continue;

        auto sentenceItr = m_badSentences.find(hashItr->second);
        if (sentenceItr != m_badSentences.end())
            list.push_back(sentenceItr->second);
    }

    return list;
}

bool WordFilterMgr::AddComplaintForUser(const ObjectGuid & offender, const ObjectGuid & complainant, uint64 complaintId, const std::string & fullInfo)
{
    auto itr = fullInfo.find("Text: [");
    if (itr == std::string::npos)
        return false;

    std::string text(fullInfo, itr+7);

    if (text.empty())
        return false;

    text.pop_back(); // delete ]

    if (text.empty())
        return false;

    // Created only once the complaint is valid: invented offender GUIDs no longer grow the map
    ComplaintInfoUser& info = m_complaints[offender];

    if (info.m_muteTime > static_cast<uint64>(GameTime::GetGameTime()) && info.m_muteCount >= 2) // just banned maximum ??
        return false;

    if (info.m_complaintsByUsers.find(complainant) != info.m_complaintsByUsers.end())
        return false;

    // One complainant per account: several characters of one account must not reach the penalty threshold alone
    if (CharacterInfo const* complainantInfo = sWorld->GetCharacterInfo(complainant))
        for (auto const& previous : info.m_complaintsByUsers)
            if (CharacterInfo const* previousInfo = sWorld->GetCharacterInfo(previous.first))
                if (previousInfo->AccountId == complainantInfo->AccountId)
                    return false;

    info.m_complaintsByUsers[complainant] = complaintId;

    WorldIntConfigs config = (info.m_muteCount <= 2 ? WorldIntConfigs(CONFIG_COMPLAINTS_PENALTY1 + info.m_muteCount) : WorldIntConfigs(CONFIG_COMPLAINTS_PENALTY1 + 2));

    if (sWorld->getIntConfig(CONFIG_COMPLAINTS_REQUIRED) != 0 && sWorld->getIntConfig(config) != 0
        && info.m_complaintsByUsers.size() >= sWorld->getIntConfig(CONFIG_COMPLAINTS_REQUIRED))
    {
        std::stringstream ss_complaintIds;
        for (auto& pair : info.m_complaintsByUsers)
            ss_complaintIds << pair.second << ", ";

        std::string complaintIds = ss_complaintIds.str();
        for (int i = 0; i < 2; ++i) // clear ", "
            complaintIds.pop_back();

        std::stringstream ss;
        ss << "Complaints: " << complaintIds.c_str() << " Text: " << text.c_str();

        // at load time the character info may not be there yet: nothing is applied then, so nothing is marked as done
        bool applied = false;

        if (sWorld->getIntConfig(config) < 0)
        {
            std::string name;
            if (ObjectMgr::GetPlayerNameByGUID(offender, name))
            {
                sWorld->BanCharacter(name, "-1", ss.str().c_str(), "Server");
                applied = true;
            }
            else if (uint32 accountid = ObjectMgr::GetPlayerAccountIdByGUID(offender))
            {
                LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO);
                stmt->setInt32(0, int32(realm.Id.Realm));
                stmt->setUInt32(1, accountid);
                PreparedQueryResult result = LoginDatabase.Query(stmt);

                if (result)
                {
                    Field* fields = result->Fetch();
                    std::string accountName = fields[0].GetString();

                    sWorld->BanAccount(BAN_ACCOUNT, accountName.c_str(), "-1", ss.str().c_str(), "Server");
                    info.m_muteTime = 4294967295;
                    applied = true;
                }
            }
        }
        else if (uint32 accountid = ObjectMgr::GetPlayerAccountIdByGUID(offender))
        {
            sWorld->MuteAccount(accountid, sWorld->getIntConfig(config), ss.str().c_str(), "Server");

            info.m_muteTime = GameTime::GetGameTime() + sWorld->getIntConfig(config)*MINUTE;
            applied = true;
        }

        if (applied)
        {
            ++info.m_muteCount;

            CharacterDatabase.PExecute("UPDATE report_complaints set JustBanned = 1 where ID in (%s)", complaintIds.c_str());
            info.m_complaintsByUsers.clear();
        }
    }

    return true;
}
