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

#ifndef TRINITYCORE_WORDFILTERMGR_H
#define TRINITYCORE_WORDFILTERMGR_H

// #include <locale>
// #include <codecvt>
#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>

struct BadSentenceInfo
{
    std::string text{};
    uint32 id = 0; 
    int32 penalty = 0; 
    uint32 sourceMask = 0;
};


typedef std::map<ObjectGuid, uint64> ComplaintsByUsers; // guid -> complaint id
struct ComplaintInfoUser
{
    ComplaintsByUsers m_complaintsByUsers{};
    uint64 m_muteTime = 0;
    uint32 m_muteCount = 0;
};

typedef std::unordered_map<ObjectGuid, ComplaintInfoUser> ComplaintsInfo; // guid of offender -> him complaints

enum BadSentenceMask : uint32
{
    BS_PM       = 0x00000001,
    BS_MAIL     = 0x00000002,
    BS_LFG      = 0x00000004,
};

struct PlayerFloodInfo
{
    uint32 lastSayTimeLFG = 0;
    uint32 lastSayTimeGeneral = 0;
    std::map<size_t, uint32> phrasesCheck{};
    std::set<ObjectGuid> players{};

    std::set<std::string> mailPhrases{};
    std::set<size_t> mailFoundedBadWords{};

    std::recursive_mutex lock; // held by FloodInfoRef
};

// keeps the entry alive and locked for as long as the caller holds it
class FloodInfoRef
{
public:
    explicit FloodInfoRef(std::shared_ptr<PlayerFloodInfo> info) : _info(std::move(info)), _guard(_info->lock) { }
    PlayerFloodInfo* operator->() const { return _info.get(); }
    PlayerFloodInfo& operator*() const { return *_info; }

private:
    std::shared_ptr<PlayerFloodInfo> _info;
    std::unique_lock<std::recursive_mutex> _guard;
};

class TC_GAME_API WordFilterMgr
{
    WordFilterMgr();
    ~WordFilterMgr();

public:
    static WordFilterMgr* instance();

    /// Note for 0.6v : Will used std::map instead of std::unordered_map, because of problems with cross-platform compilation.
    // [letter][analogs] 
    typedef std::map<wchar_t, wchar_t> LetterAnalogMap;
    // [converted][original]
    typedef std::set<std::wstring> BadWordMap;
    typedef std::set<std::wstring> BadWordMapMail;
    typedef std::map <size_t, BadSentenceInfo> BadSentences;

    void LoadLetterAnalogs();
    void LoadBadWords();
    void LoadBadSentences();
    void LoadComplaints();

    std::string FindBadWord(std::string const& text, bool mail = false);

    // manipulations with container 
    bool AddBadWord(std::string const& badWord, bool toDB = false);
    void AddConvert(std::string const& badWord);
    bool AddBadWordMail(std::string const& badWord, bool toDB = false);
    bool RemoveBadWord(std::string const& badWord, bool fromDB = false);

    bool AddBadSentence(std::string badSentence, size_t hash, uint32 mask, int32 penalty = 10);
    void UpdatePenaltyForBadSentenceById(uint32 id, int32 penalty);

    size_t FilterMsgAndGetHash(std::string lowedMsg, LocaleConstant locale = LOCALE_enUS, bool* isValid = nullptr);
    void GeneralFilterWstring(std::wstring& msg, LocaleConstant locale = LOCALE_enUS, bool* isValid = nullptr);

    // element (const) accessor 
    BadWordMap GetBadWords() const;

    uint32 GetIdOfBadSentence(size_t hash);
    int32 GetPenaltyForBadSentence(size_t hash);
    uint32 GetSourceMaskForBadSentence(size_t hash);
    std::vector<BadSentenceInfo> GetBadSentenceList(uint32 page);
    uint32 GetLastIdBadSentences() const { return lastIdBadSentences.load(); }

    FloodInfoRef GetFloodInfo(uint32 accountid)
    {
        std::shared_ptr<PlayerFloodInfo> info;
        {
            std::lock_guard<std::mutex> guard(m_floodLock);
            std::shared_ptr<PlayerFloodInfo>& slot = m_floodInfo[accountid];
            if (!slot)
                slot = std::make_shared<PlayerFloodInfo>();
            info = slot;
        }
        return FloodInfoRef(std::move(info));
    }
    bool AddComplaintForUser(const ObjectGuid& offender, const ObjectGuid& complainant, uint64 complaintId, const std::string& text);
private:
    LetterAnalogMap m_letterAnalogs;
    BadWordMap m_badWords;
    BadWordMapMail m_badWordsMail;

    BadSentences m_badSentences{};
    std::map<uint32, size_t> hashById{};
    std::atomic<uint32> lastIdBadSentences{0};
    std::hash<std::wstring> hash_fn;

    // chat checks run in map threads while commands and reloads change the lists in the world thread
    mutable std::shared_mutex m_lock;
    
    ComplaintsInfo m_complaints{};
    std::mutex m_floodLock; // protects the map only, each entry has its own lock
    std::map<uint32, std::shared_ptr<PlayerFloodInfo>> m_floodInfo{}; // account -> info
};

#define sWordFilterMgr WordFilterMgr::instance()

#endif