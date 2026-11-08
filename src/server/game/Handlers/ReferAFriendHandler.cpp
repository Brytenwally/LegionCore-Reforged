/*
 * Copyright (C) 2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "WorldSession.h"
#include "Player.h"
#include "Opcodes.h"
#include "ReferAFriendPackets.h"
#include "DatabaseEnv.h"
#include "World.h"
#include "ChatPackets.h"
#include "Chat.h"
#include "Config.h"
#include "Timer.h"
#include <algorithm>
#include <mutex>
#include <unordered_map>

// Each invitation costs several synchronous login database queries
static bool RafInviteAllowed(uint32 accountId)
{
    static std::mutex lock;
    static std::unordered_map<uint32, uint32> lastInvite;

    uint32 const cooldown = 5 * IN_MILLISECONDS;
    uint32 const now = getMSTime();

    std::lock_guard<std::mutex> guard(lock);
    auto itr = lastInvite.find(accountId);
    if (itr != lastInvite.end() && getMSTimeDiff(itr->second, now) < cooldown)
        return false;

    lastInvite[accountId] = now;
    return true;
}

// Level grants proposed and not yet answered, by recipient. Without it the recruiter could accept a
// grant the recruit never offered and drain the recruit's grantable levels.
struct RafPendingGrant
{
    ObjectGuid Granter;
    uint32 Time;
};

static std::mutex RafPendingGrantsLock;
static std::unordered_map<ObjectGuid, RafPendingGrant> RafPendingGrants;

// Days an unanswered invitation stays valid. The config documents 0 as "never expires", which the
// queries express by reaching further back than any realm could possibly have existed.
static uint32 RafExpireDays()
{
    uint32 const days = uint32(sConfigMgr->GetIntDefault("RecruitAFriend.InviteExpireDays", 7));
    return days ? days : 36500;
}

// Second line of every in-chat reminder: how to answer. Nothing is said when the commands are
// off, since naming a command the player cannot run would only mislead.
static void SendRafAnswerHint(ChatHandler& chat)
{
    if (!sWorld->getBoolConfig(CONFIG_RECRUIT_A_FRIEND_COMMANDS_ENABLE))
        return;

    chat.SendSysMessage("Type |cff00ccff.account raf list|r to view it, then |cff00ccff.account raf accept|r or |cff00ccff.account raf refuse|r followed by its number.");
}

// Answers the check the client sends when the social panel opens. Without it the invitation
// interface stays inert client-side.
void WorldSession::HandleCheckRafEmailEnabled(WorldPackets::ReferAFriend::CheckRafEmailEnabled& /*packet*/)
{
    WorldPackets::ReferAFriend::RafEmailEnabledResponse response;
    response.Enabled = sWorld->getBoolConfig(CONFIG_RECRUIT_A_FRIEND_ENABLE);
    SendPacket(response.Write());
}

// Recruit-A-Friend invitation. With no mail service, the address is resolved directly in the
// account table, so the recruit must have registered with that very address. The invitation is
// recorded as pending; account.recruiter is only written once it is accepted.
void WorldSession::HandleRecruitAFriend(WorldPackets::ReferAFriend::RecruitAFriend& packet)
{
    // No error code is sent back, on purpose. The Recruit-A-Friend codes were designed for
    // level granting: none of them means "unknown address", and the client dresses them up with
    // an unrelated red message and a voice line from the character ("I have no target"). A plain
    // notification explains the refusal with no side effect.
    auto sendFailure = [this](uint8 /*reason*/, std::string const& /*text*/, char const* message)
    {
        SendNotification("%s", message);
    };

    if (!sWorld->getBoolConfig(CONFIG_RECRUIT_A_FRIEND_ENABLE))
    {
        sendFailure(ERR_REFER_A_FRIEND_NOT_NOW, "", "Recruit-a-Friend is not active on this realm.");
        return;
    }

    std::string email = packet.Email;
    std::transform(email.begin(), email.end(), email.begin(), ::tolower);

    if (email.empty() || email.find('@') == std::string::npos)
    {
        sendFailure(ERR_REFER_A_FRIEND_NOT_NOW, email, "Invalid e-mail address.");
        return;
    }

    if (!RafInviteAllowed(GetAccountId()))
    {
        sendFailure(ERR_REFER_A_FRIEND_NOT_NOW, email, "Please wait a few seconds before sending another invitation.");
        return;
    }

    // Unknown, already recruited or already invited addresses all get the answer of a sent invitation:
    // a distinct answer would let anyone test which e-mail addresses own an account here.
    auto sendGenericAnswer = [this]()
    {
        SendRafAddonMessage("SENT");

        WorldPackets::ReferAFriend::RecruitAFriendResponse response;
        response.Result = 0;
        SendPacket(response.Write());
    };

    std::string escapedEmail = email;
    LoginDatabase.EscapeString(escapedEmail);

    QueryResult target = LoginDatabase.PQuery("SELECT id, recruiter FROM account WHERE LOWER(email) = '%s'",
        escapedEmail.c_str());

    if (!target)
    {
        sendGenericAnswer();
        return;
    }

    Field* fields = target->Fetch();
    uint32 const recruitedId = fields[0].GetUInt32();
    uint32 const existingRecruiter = fields[1].GetUInt32();

    if (recruitedId == GetAccountId())
    {
        sendFailure(ERR_REFER_A_FRIEND_NOT_NOW, email, "You cannot recruit yourself.");
        return;
    }

    // Already recruited: the link is permanent, it is never replaced.
    if (existingRecruiter)
    {
        sendGenericAnswer();
        return;
    }

    // Only one pending invitation between any two accounts.
    if (LoginDatabase.PQuery("SELECT 1 FROM account_raf_invite WHERE recruiter_id = %u AND recruited_id = %u AND status = 0",
        GetAccountId(), recruitedId))
    {
        sendGenericAnswer();
        return;
    }

    std::string note = packet.Note;
    if (note.size() > 255)
        note.resize(255);
    LoginDatabase.EscapeString(note);

    LoginDatabase.PExecute("INSERT INTO account_raf_invite (recruiter_id, recruited_id, recruited_email, note) VALUES (%u, %u, '%s', '%s')",
        GetAccountId(), recruitedId, escapedEmail.c_str(), note.c_str());

    TC_LOG_INFO("misc", "Recruit-a-Friend: account %u invites account %u (%s).",
        GetAccountId(), recruitedId, email.c_str());

    // Already online: tell them now rather than waiting for their next login, and mark the
    // invitation as announced. The UPDATE is queued behind the INSERT on the same connection, so
    // it cannot miss the row it was meant for.
    if (WorldSessionPtr recruit = sWorld->FindSession(recruitedId))
    {
        if (recruit->GetPlayer())
        {
            recruit->SendRafInviteReceived(GetPlayer() ? std::string(GetPlayer()->GetName()) : std::string("?"));
            LoginDatabase.PExecute("UPDATE account_raf_invite SET notified = 1 WHERE recruited_id = %u AND status = 0",
                recruitedId);
        }
    }

    // Nothing is shown to the sender on success: the client already confirms the send on its own.
    // The acknowledgement goes over the addon channel instead, where it stays invisible.
    sendGenericAnswer();
}

void WorldSession::HandleGrantLevel(WorldPackets::ReferAFriend::GrantLevel& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    // Same map only, as in TrinityCore: the target must be in the granter's group anyway, and a player on
    // another map belongs to another map thread
    Player* target = ObjectAccessor::GetPlayer(*player, packet.Target);

    // check cheating
    uint8 levels = player->GetGrantableLevels();
    uint8 error = 0;
    if (!target)
        error = ERR_REFER_A_FRIEND_NO_TARGET;
    else if (levels == 0)
        error = ERR_REFER_A_FRIEND_INSUFFICIENT_GRANTABLE_LEVELS;
    else if (GetRecruiterId() != target->GetSession()->GetAccountId())
        error = ERR_REFER_A_FRIEND_NOT_REFERRED_BY;
    else if (target->GetTeamId() != player->GetTeamId())
        error = ERR_REFER_A_FRIEND_DIFFERENT_FACTION;
    else if (target->getLevel() >= player->getLevel())
        error = ERR_REFER_A_FRIEND_TARGET_TOO_HIGH;
    else if (target->getLevel() >= sWorld->getIntConfig(CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL))
        error = ERR_REFER_A_FRIEND_GRANT_LEVEL_MAX_I;
    else if (target->GetGroup() != player->GetGroup())
        error = ERR_REFER_A_FRIEND_NOT_IN_GROUP;

    if (error) 
    {
        WorldPackets::ReferAFriend::ReferAFriendFailure failure;
        if (error == ERR_REFER_A_FRIEND_NOT_IN_GROUP)
            failure.Str =  target->GetName();
        failure.Reason = error;
        SendPacket(failure.Write());
        return;
    }

    {
        std::lock_guard<std::mutex> guard(RafPendingGrantsLock);
        RafPendingGrants[target->GetGUID()] = { player->GetGUID(), getMSTime() };
    }

    WorldPackets::ReferAFriend::ProposeLevelGrant grant;
    grant.Sender = player->GetGUID();
    target->SendDirectMessage(grant.Write());
}

void WorldSession::HandleAcceptGrantLevel(WorldPackets::ReferAFriend::AcceptLevelGrant& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    {
        std::lock_guard<std::mutex> guard(RafPendingGrantsLock);
        auto itr = RafPendingGrants.find(player->GetGUID());
        if (itr == RafPendingGrants.end())
            return;

        RafPendingGrant const offer = itr->second;
        RafPendingGrants.erase(itr);

        if (offer.Granter != packet.Granter || getMSTimeDiff(offer.Time, getMSTime()) > 2 * MINUTE * IN_MILLISECONDS)
            return;
    }

    Player* other = ObjectAccessor::GetPlayer(*player, packet.Granter);
    if (!(other && other->GetSession()))
        return;

    if (GetAccountId() != other->GetSession()->GetRecruiterId())
        return;

    // The situation may have changed since the offer: check again what HandleGrantLevel checked
    if (player->getLevel() >= other->getLevel() || player->getLevel() >= sWorld->getIntConfig(CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL)
        || player->GetGroup() != other->GetGroup() || player->GetTeamId() != other->GetTeamId())
        return;

    if (other->GetGrantableLevels())
        other->SetGrantableLevels(other->GetGrantableLevels() - 1);
    else
        return;

    player->GiveLevel(player->getLevel() + 1);
}

// --------------------------------------------------------------------------------------------
// Addon channel API.
//
// The same operations as the .account raf commands, carried over the RAF_ADDON_PREFIX addon
// chat prefix so a client-side interface can drive them. Nothing on the server depends on it.
//
// Client -> server : "LIST", "ACCEPT <id>", "REFUSE <id>"
// Server -> client : "INVITE <id> <sender> <note>", "NONE", "DONE ACCEPT|REFUSE", "ERR <text>",
//                    "SENT" once an invitation has been recorded.
//
// The note is sent last because it is free text and may contain spaces.
// --------------------------------------------------------------------------------------------

void WorldSession::SendRafAddonMessage(std::string const& text)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    WorldPackets::Chat::Chat packet;
    packet.Initialize(CHAT_MSG_WHISPER, LANG_ADDON, player, player, text, 0, "", DEFAULT_LOCALE, RAF_ADDON_PREFIX);
    SendPacket(packet.Write());
}

// Pending invitations addressed to this account, newest last. Expired ones are filtered out here
// as well as deleted daily, so an invitation stops being answerable the moment it ages out.
void WorldSession::SendRafPendingInvites()
{
    if (!sWorld->getBoolConfig(CONFIG_RECRUIT_A_FRIEND_ENABLE))
    {
        SendRafAddonMessage("NONE");
        return;
    }

    QueryResult result = LoginDatabase.PQuery(
        "SELECT i.id, a.username, i.note FROM account_raf_invite i "
        "JOIN account a ON a.id = i.recruiter_id "
        "WHERE i.recruited_id = %u AND i.status = 0 "
        "AND i.created_at >= NOW() - INTERVAL %u DAY ORDER BY i.id",
        GetAccountId(), RafExpireDays());

    if (!result)
    {
        SendRafAddonMessage("NONE");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        SendRafAddonMessage(Trinity::StringFormat("INVITE %u %s %s",
            fields[0].GetUInt32(), fields[1].GetCString(), fields[2].GetCString()));
    } while (result->NextRow());
}

void WorldSession::HandleRafAddonMessage(std::string const& message)
{
    if (!sWorld->getBoolConfig(CONFIG_RECRUIT_A_FRIEND_ENABLE))
        return;

    if (message == "LIST")
    {
        SendRafPendingInvites();
        return;
    }

    bool accepted;
    if (message.compare(0, 7, "ACCEPT ") == 0)
        accepted = true;
    else if (message.compare(0, 7, "REFUSE ") == 0)
        accepted = false;
    else
        return;

    uint32 const inviteId = uint32(atoul(message.c_str() + 7));
    if (!inviteId)
        return;

    uint32 const accountId = GetAccountId();

    QueryResult result = LoginDatabase.PQuery(
        "SELECT recruiter_id FROM account_raf_invite WHERE id = %u AND recruited_id = %u AND status = 0 "
        "AND created_at >= NOW() - INTERVAL %u DAY",
        inviteId, accountId, RafExpireDays());

    if (!result)
    {
        SendRafAddonMessage("ERR This invitation is no longer valid.");
        return;
    }

    uint32 const recruiterId = result->Fetch()[0].GetUInt32();

    LoginDatabase.PExecute("UPDATE account_raf_invite SET status = %u, responded_at = NOW() WHERE id = %u",
        accepted ? 1 : 2, inviteId);

    if (!accepted)
    {
        SendRafAddonMessage("DONE REFUSE");
        return;
    }

    // The link is only written here: while the invitation was pending no bonus applied. Any other
    // pending invitation becomes moot, an account can only ever have one recruiter.
    LoginDatabase.PExecute("UPDATE account SET recruiter = %u WHERE id = %u", recruiterId, accountId);
    LoginDatabase.PExecute("UPDATE account_raf_invite SET status = 3, responded_at = NOW() "
        "WHERE recruited_id = %u AND status = 0", accountId);

    SendRafAddonMessage("DONE ACCEPT");
    TC_LOG_INFO("misc", "Recruit-a-Friend: account %u accepted the invitation of account %u.",
        accountId, recruiterId);
}

// --------------------------------------------------------------------------------------------
// In-chat reminders.
//
// An invitation is easy to miss, and there is nothing in the interface to go back to. A chat line
// therefore announces it the first time it is seen ("you have received"), then reminds about it
// at every later login while it is still unanswered ("you have a pending"). The distinction is
// carried by account_raf_invite.notified.
// --------------------------------------------------------------------------------------------

// Announced live, while the recruit is already in game. The caller marks the invitation as
// announced, so the next login falls back to the "pending" wording.
void WorldSession::SendRafInviteReceived(std::string const& recruiterName)
{
    if (!GetPlayer())
        return;

    ChatHandler chat(this);
    chat.PSendSysMessage("|cff00ff00[Recruit-a-Friend]|r %s has just sent you a recruitment request.",
        recruiterName.c_str());
    SendRafAnswerHint(chat);
}

// Announced at login, from the state stored in database.
void WorldSession::SendRafInviteNotification()
{
    if (!sWorld->getBoolConfig(CONFIG_RECRUIT_A_FRIEND_ENABLE) || !GetPlayer())
        return;

    uint32 const accountId = GetAccountId();

    // Unannounced invitations first, so the name shown belongs to the one being announced.
    QueryResult result = LoginDatabase.PQuery(
        "SELECT i.notified, a.username FROM account_raf_invite i "
        "JOIN account a ON a.id = i.recruiter_id "
        "WHERE i.recruited_id = %u AND i.status = 0 "
        "AND i.created_at >= NOW() - INTERVAL %u DAY ORDER BY i.notified, i.id",
        accountId, RafExpireDays());

    if (!result)
        return;

    uint32 total = 0;
    uint32 fresh = 0;
    std::string firstRecruiter;

    do
    {
        Field* fields = result->Fetch();
        if (!total)
            firstRecruiter = fields[1].GetString();
        if (!fields[0].GetUInt8())
            ++fresh;
        ++total;
    } while (result->NextRow());

    ChatHandler chat(this);

    if (fresh)
    {
        if (fresh > 1)
            chat.PSendSysMessage("|cff00ff00[Recruit-a-Friend]|r You have received %u recruitment requests, including one from %s.",
                fresh, firstRecruiter.c_str());
        else
            chat.PSendSysMessage("|cff00ff00[Recruit-a-Friend]|r You have received a recruitment request from %s.",
                firstRecruiter.c_str());

        LoginDatabase.PExecute("UPDATE account_raf_invite SET notified = 1 WHERE recruited_id = %u AND status = 0",
            accountId);
    }
    else if (total > 1)
        chat.PSendSysMessage("|cff00ff00[Recruit-a-Friend]|r You have %u pending recruitment requests, including one from %s.",
            total, firstRecruiter.c_str());
    else
        chat.PSendSysMessage("|cff00ff00[Recruit-a-Friend]|r You have a pending recruitment request from %s.",
            firstRecruiter.c_str());

    SendRafAnswerHint(chat);
}
