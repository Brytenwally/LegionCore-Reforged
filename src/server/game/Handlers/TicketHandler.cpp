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
#include "Language.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "TicketMgr.h"
#include "TicketPackets.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "WordFilterMgr.h"
#include "Timer.h"
#include <mutex>
#include <unordered_map>

// Every complaint is kept in memory and written to the database, and enough of them mute the offender
static bool ComplaintAllowed(uint32 accountId)
{
    static std::mutex lock;
    static std::unordered_map<uint32, uint32> lastComplaint;

    uint32 const cooldown = 10 * IN_MILLISECONDS;
    uint32 const now = getMSTime();

    std::lock_guard<std::mutex> guard(lock);
    auto itr = lastComplaint.find(accountId);
    if (itr != lastComplaint.end() && getMSTimeDiff(itr->second, now) < cooldown)
        return false;

    lastComplaint[accountId] = now;
    return true;
}

void WorldSession::HandleComplaint(WorldPackets::Ticket::Complaint& packet)
{
    ObjectGuid const& offender = packet.Offender.PlayerGuid;
    bool const valid = offender.IsPlayer() && offender != GetPlayer()->GetGUID() && sWorld->GetCharacterInfo(offender)
        && ComplaintAllowed(GetAccountId());

    uint64 complaintId = valid ? sObjectMgr->GenerateReportComplaintID() : 0;
    if (valid && sWordFilterMgr->AddComplaintForUser(packet.Offender.PlayerGuid, GetPlayer()->GetGUID(), complaintId, packet.Chat.MessageLog))
    {
        uint8 index = 0;
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_COMPLAINTS);
        stmt->setUInt64(index++, complaintId);
        stmt->setUInt64(index++, GetPlayer()->GetGUIDLow());
        stmt->setUInt32(index++, GetAccountId());
        stmt->setUInt32(index++, GameTime::GetGameTime());
        stmt->setUInt64(index++, packet.Offender.PlayerGuid.GetGUIDLow());
        stmt->setUInt8(index++, packet.ComplaintType);
        stmt->setUInt32(index++, packet.MailID);
        stmt->setUInt32(index++, packet.Offender.TimeSinceOffence);
        stmt->setString(index++, packet.Chat.MessageLog);
        CharacterDatabase.Execute(stmt);
    }

    WorldPackets::Ticket::ComplaintResult result;
    result.ComplaintType = packet.ComplaintType;
    result.Result = 0;
    SendPacket(result.Write());
}

void WorldSession::HandleSupportTicketSubmitBug(WorldPackets::Ticket::SupportTicketSubmitBug& packet)
{
    // Don't accept tickets if the ticket queue is disabled. (Ticket UI is greyed out but not fully dependable)
    if (!sTicketMgr->GetStatus())
        return;

    if (GetPlayer()->getLevel() < sWorld->getIntConfig(CONFIG_TICKET_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_TICKET_REQ), sWorld->getIntConfig(CONFIG_TICKET_LEVEL_REQ));
        return;
    }

    // Account must not have an open ticket (any character)
    if (!sTicketMgr->HasOpenTicketForAccount(GetAccountId()))
    {
        GmTicket* ticket = new GmTicket(GetPlayer(), packet);
        sTicketMgr->AddTicket(ticket);
        sTicketMgr->UpdateLastChange();

        sWorld->SendGMText(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName(), ticket->GetId());
    }
}

void WorldSession::HandleGMTicketGetSystemStatus(WorldPackets::Ticket::GMTicketGetSystemStatus& /*packet*/)
{
    // Note: This only disables the ticket UI at client side and is not fully reliable
    WorldPackets::Ticket::GMTicketSystemStatus response;
    response.Status = sTicketMgr->GetStatus() ? GMTICKET_QUEUE_STATUS_ENABLED : GMTICKET_QUEUE_STATUS_DISABLED;
    SendPacket(response.Write());
}

void WorldSession::HandleGMTicketAcknowledgeSurvey(WorldPackets::Ticket::GMTicketAcknowledgeSurvey& /*packet*/)
{ }

void WorldSession::HandleGMTicketGetCaseStatus(WorldPackets::Ticket::GMTicketGetCaseStatus& /*packet*/)
{
    // TODO: Implement GmCase and handle this packet properly
    WorldPackets::Ticket::GMTicketCaseStatus status;
    SendPacket(status.Write());
}

void WorldSession::HandleSupportTicketSubmitComplaint(WorldPackets::Ticket::SupportTicketSubmitComplaint& /*packet*/)
{ }

void WorldSession::HandleSupportTicketSubmitSuggestion(WorldPackets::Ticket::SupportTicketSubmitSuggestion& /*packet*/)
{ }
