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

#include "Common.h"
#include "TicketMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "TicketPackets.h"
#include "Chat.h"
#include "World.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// GM ticket
GmTicket::GmTicket(): _id(0), _mapId(0), _createTime(0), _lastModifiedTime(0), _completed(false), _escalatedStatus(TICKET_UNASSIGNED), _viewed(false)
{
}

GmTicket::GmTicket(Player* player, WorldPackets::Ticket::SupportTicketSubmitBug& packet) : _createTime(GameTime::GetGameTime()), _lastModifiedTime(GameTime::GetGameTime()), _completed(false), _escalatedStatus(TICKET_UNASSIGNED), _viewed(false)
{
    _id = sTicketMgr->GenerateTicketId();
    _playerName = player->GetName();
    _playerGuid = player->GetGUID();
    _mapId = packet.Header.MapID;

    _pos = packet.Header.Pos;

    _message = packet.Note;
}

GmTicket::~GmTicket() { }

bool GmTicket::LoadFromDB(Field* fields)
{
    //     0       1     2      3          4        5      6     7     8           9            10         11         12        13        14        15
    // ticketId, guid, name, message, createTime, mapId, posX, posY, posZ, lastModifiedTime, closedBy, assignedTo, comment, completed, escalated, viewed
    uint8 index = 0;
    _id                 = fields[  index].GetUInt32();
    _playerGuid         = ObjectGuid::Create<HighGuid::Player>(fields[++index].GetUInt64());
    _playerName         = fields[++index].GetString();
    _message            = fields[++index].GetString();
    _createTime         = fields[++index].GetUInt32();
    _mapId              = fields[++index].GetUInt16();
    auto x        = fields[++index].GetFloat();
    auto y        = fields[++index].GetFloat();
    auto z        = fields[++index].GetFloat();
    _pos                = Position(x, y, z);
    _lastModifiedTime   = fields[++index].GetUInt32();
    int64 closedBy = fields[++index].GetInt64();
    // 0xFFFFFFFFFF is how older versions saved a console closure (counter mask of -1)
    if (closedBy < 0 || closedBy == int64(0xFFFFFFFFFF))
        _closedBy.SetRawValue(0, uint64(-1));
    else if (!closedBy)
        _closedBy = ObjectGuid::Empty;
    else
        _closedBy = ObjectGuid::Create<HighGuid::Player>(uint64(closedBy));

    // a zero counter would build a non-empty guid and make every unassigned ticket look assigned
    uint64 assignedTo   = fields[++index].GetUInt64();
    _assignedTo         = assignedTo ? ObjectGuid::Create<HighGuid::Player>(assignedTo) : ObjectGuid::Empty;
    _comment            = fields[++index].GetString();
    _completed          = fields[++index].GetBool();
    _escalatedStatus    = GMTicketEscalationStatus(fields[++index].GetUInt8());
    _viewed             = fields[++index].GetBool();
    return true;
}

void GmTicket::SaveToDB(CharacterDatabaseTransaction& trans) const
{
    //     0       1     2      3          4        5      6     7     8           9            10         11         12        13        14        15
    // ticketId, guid, name, message, createTime, mapId, posX, posY, posZ, lastModifiedTime, closedBy, assignedTo, comment, completed, escalated, viewed
    uint8 index = 0;
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GM_TICKET);
    stmt->setUInt32(  index, _id);
    stmt->setUInt64(++index, _playerGuid.GetCounter());
    stmt->setString(++index, _playerName);
    stmt->setString(++index, _message);
    stmt->setUInt32(++index, uint32(_createTime));
    stmt->setUInt16(++index, _mapId);
    stmt->setFloat (++index, _pos.GetPositionX());
    stmt->setFloat (++index, _pos.GetPositionY());
    stmt->setFloat (++index, _pos.GetPositionZ());
    stmt->setUInt32(++index, uint32(_lastModifiedTime));
    // console closure is stored as a raw low part with no high part, see LoadFromDB
    stmt->setInt64 (++index, _closedBy.GetHighPart() ? int64(_closedBy.GetCounter()) : int64(_closedBy.GetLowPart()));
    stmt->setUInt64(++index, _assignedTo.GetCounter());
    stmt->setString(++index, _comment);
    stmt->setBool  (++index, _completed);
    stmt->setUInt8 (++index, uint8(_escalatedStatus));
    stmt->setBool  (++index, _viewed);

    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

void GmTicket::DeleteFromDB()
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GM_TICKET);
    stmt->setUInt32(0, _id);
    CharacterDatabase.Execute(stmt);
}

std::string GmTicket::FormatMessageString(ChatHandler& handler, bool detailed) const
{
    time_t curTime = GameTime::GetGameTime();

    std::stringstream ss;
    std::string nameLink = handler.playerLink(_playerName);

    ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTGUID, _id);
    ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTNAME, nameLink.c_str());
    ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTAGECREATE, (secsToTimeString(curTime - _createTime, true, false)).c_str());
    ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTAGE, (secsToTimeString(curTime - _lastModifiedTime, true, false)).c_str());

    std::string name;
    if (ObjectMgr::GetPlayerNameByGUID(_assignedTo, name))
        ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTASSIGNEDTO, name.c_str());

    if (detailed)
    {
        ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTMESSAGE, _message.c_str());
        if (!_comment.empty())
            ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTCOMMENT, _comment.c_str());
    }
    return ss.str();
}

std::string GmTicket::FormatMessageString(ChatHandler& handler, const char* szClosedName, const char* szAssignedToName, const char* szUnassignedName, const char* szDeletedName) const
{
    std::stringstream ss;
    ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTGUID, _id);
    ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTNAME, _playerName.c_str());
    if (szClosedName)
        ss << handler.PGetParseString(LANG_COMMAND_TICKETCLOSED, szClosedName);
    if (szAssignedToName)
        ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTASSIGNEDTO, szAssignedToName);
    if (szUnassignedName)
        ss << handler.PGetParseString(LANG_COMMAND_TICKETLISTUNASSIGNED, szUnassignedName);
    if (szDeletedName)
        ss << handler.PGetParseString(LANG_COMMAND_TICKETDELETED, szDeletedName);
    return ss.str();
}

void GmTicket::SetUnassigned()
{
    _assignedTo.Clear();
    switch (_escalatedStatus)
    {
        case TICKET_ASSIGNED: _escalatedStatus = TICKET_UNASSIGNED; break;
        case TICKET_ESCALATED_ASSIGNED: _escalatedStatus = TICKET_IN_ESCALATION_QUEUE; break;
        case TICKET_UNASSIGNED:
        case TICKET_IN_ESCALATION_QUEUE:
        default:
            break;
    }
}

void GmTicket::TeleportTo(Player* player) const
{
    player->TeleportTo(_mapId, &_pos, 0.0f, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Ticket manager
TicketMgr::TicketMgr() : _status(true), _lastTicketId(0), _lastSurveyId(0), _openTicketCount(0), _lastChange(GameTime::GetGameTime()) { }

TicketMgr::~TicketMgr()
{
    for (GmTicketList::const_iterator itr = _ticketList.begin(); itr != _ticketList.end(); ++itr)
        delete itr->second;
}

TicketMgr* TicketMgr::instance()
{
    static TicketMgr instance;
    return &instance;
}

void TicketMgr::Initialize() { SetStatus(sWorld->getBoolConfig(CONFIG_ALLOW_TICKETS)); }

void TicketMgr::ResetTickets()
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    for (GmTicketList::iterator itr = _ticketList.begin(); itr != _ticketList.end();)
    {
        if (itr->second->IsClosed())
        {
            GmTicket* ticket = itr->second;
            ticket->DeleteFromDB();
            itr = _ticketList.erase(itr);
            delete ticket;
        }
        else
            ++itr;
    }

    _lastTicketId = 0;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GM_TICKETS);

    CharacterDatabase.Execute(stmt);
}

void TicketMgr::LoadTickets()
{
    uint32 oldMSTime = getMSTime();

    std::lock_guard<std::recursive_mutex> guard(_lock);
    for (GmTicketList::const_iterator itr = _ticketList.begin(); itr != _ticketList.end(); ++itr)
        delete itr->second;
    _ticketList.clear();

    _lastTicketId = 0;
    _openTicketCount = 0;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GM_TICKETS);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 GM tickets. DB table `gm_tickets` is empty!");

        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        GmTicket* ticket = new GmTicket();
        if (!ticket->LoadFromDB(fields))
        {
            delete ticket;
            continue;
        }
        if (!ticket->IsClosed())
            ++_openTicketCount;

        // Update max ticket id if necessary
        uint32 id = ticket->GetId();
        if (_lastTicketId < id)
            _lastTicketId = id;

        _ticketList[id] = ticket;
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u GM tickets in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void TicketMgr::LoadSurveys()
{
    // we don't actually load anything into memory here as there's no reason to
    _lastSurveyId = 0;

    uint32 oldMSTime = getMSTime();
    if (QueryResult result = CharacterDatabase.Query("SELECT MAX(surveyId) FROM gm_surveys"))
        _lastSurveyId = (*result)[0].GetUInt32();

    TC_LOG_INFO("server.loading", ">> Loaded GM Survey count from database in %u ms", GetMSTimeDiffToNow(oldMSTime));

}

bool TicketMgr::HasOpenTicketForAccount(uint32 accountId)
{
    std::vector<ObjectGuid> owners;
    {
        std::lock_guard<std::recursive_mutex> guard(_lock);
        for (auto const& itr : _ticketList)
            if (itr.second && !itr.second->IsClosed())
                owners.push_back(itr.second->GetPlayerGUID());
    }

    for (ObjectGuid const& guid : owners)
        if (ObjectMgr::GetPlayerAccountIdByGUID(guid) == accountId)
            return true;

    return false;
}

void TicketMgr::AddTicket(GmTicket* ticket)
{
    {
        std::lock_guard<std::recursive_mutex> guard(_lock);
        _ticketList[ticket->GetId()] = ticket;
        if (!ticket->IsClosed())
            ++_openTicketCount;
    }

    CharacterDatabaseTransaction trans = CharacterDatabaseTransaction(nullptr);
    ticket->SaveToDB(trans);
}

void TicketMgr::CloseTicket(uint32 ticketId, ObjectGuid source)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    if (GmTicket* ticket = GetTicket(ticketId))
    {
        CharacterDatabaseTransaction trans = CharacterDatabaseTransaction(nullptr);
        ticket->SetClosedBy(source);
        if (!source.IsEmpty())
            --_openTicketCount;
        ticket->SaveToDB(trans);
    }
}

void TicketMgr::RemoveTicket(uint32 ticketId)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    if (GmTicket* ticket = GetTicket(ticketId))
    {
        ticket->DeleteFromDB();
        _ticketList.erase(ticketId);
        delete ticket;
    }
}

void TicketMgr::ShowList(ChatHandler& handler, bool onlineOnly) const
{
    handler.SendSysMessage(onlineOnly ? LANG_COMMAND_TICKETSHOWONLINELIST : LANG_COMMAND_TICKETSHOWLIST);
    std::vector<std::string> lines;
    {
        std::lock_guard<std::recursive_mutex> guard(_lock);
        for (GmTicketList::const_iterator itr = _ticketList.begin(); itr != _ticketList.end(); ++itr)
            if (!itr->second->IsClosed() && !itr->second->IsCompleted())
                if (!onlineOnly || itr->second->GetPlayer())
                    lines.push_back(itr->second->FormatMessageString(handler));
    }
    for (std::string const& line : lines)
        handler.SendSysMessage(line.c_str());
}

void TicketMgr::ShowClosedList(ChatHandler& handler) const
{
    handler.SendSysMessage(LANG_COMMAND_TICKETSHOWCLOSEDLIST);
    std::vector<std::string> lines;
    {
        std::lock_guard<std::recursive_mutex> guard(_lock);
        for (GmTicketList::const_iterator itr = _ticketList.begin(); itr != _ticketList.end(); ++itr)
            if (itr->second->IsClosed())
                lines.push_back(itr->second->FormatMessageString(handler));
    }
    for (std::string const& line : lines)
        handler.SendSysMessage(line.c_str());
}

void TicketMgr::ShowEscalatedList(ChatHandler& handler) const
{
    handler.SendSysMessage(LANG_COMMAND_TICKETSHOWESCALATEDLIST);
    std::vector<std::string> lines;
    {
        std::lock_guard<std::recursive_mutex> guard(_lock);
        for (GmTicketList::const_iterator itr = _ticketList.begin(); itr != _ticketList.end(); ++itr)
            if (!itr->second->IsClosed() && itr->second->GetEscalatedStatus() == TICKET_IN_ESCALATION_QUEUE)
                lines.push_back(itr->second->FormatMessageString(handler));
    }
    for (std::string const& line : lines)
        handler.SendSysMessage(line.c_str());
}
