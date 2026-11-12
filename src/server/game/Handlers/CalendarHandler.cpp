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

#include "CalendarPackets.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "DatabaseEnv.h"
#include "SocialMgr.h"
#include "Util.h"

void WorldSession::HandleCalendarGetCalendar(WorldPackets::Calendar::CalendarGetCalendar& /*packet*/)
{
    ObjectGuid guid = _player->GetGUID();
    time_t currTime = GameTime::GetGameTime();

    WorldPackets::Calendar::CalendarSendCalendar packet;
    packet.ServerTime = currTime;

    // outside the calendar lock: no instance data is read under it
    for (Difficulty const difficulty : { DIFFICULTY_NORMAL, DIFFICULTY_HEROIC, DIFFICULTY_MYTHIC_DUNGEON })
    {
        // one difficulty per bound type: several difficulties share a type and would list its locks again
        Player::BoundInstancesMap boundInstances = _player->GetBoundInstancesCopy(difficulty);
        for (auto const& boundInstance : boundInstances)
        {
            if (!boundInstance.second.perm)
                continue;

            if (InstanceSave* save = boundInstance.second.save)
            {
                WorldPackets::Calendar::CalendarSendCalendarRaidLockoutInfo lockoutInfo;
                lockoutInfo.MapID = save->GetMapId();
                lockoutInfo.DifficultyID = save->GetDifficultyID();
                lockoutInfo.ExpireTime = save->GetResetTime() - currTime;
                lockoutInfo.InstanceID = save->GetInstanceId();
                packet.RaidLockouts.push_back(lockoutInfo);
            }
        }
    }

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    CalendarInviteStore playerInvites = sCalendarMgr->GetPlayerInvites(guid);
    for (auto const& invite : playerInvites)
    {
        WorldPackets::Calendar::CalendarSendCalendarInviteInfo inviteInfo;
        inviteInfo.EventID = invite->GetEventId();
        inviteInfo.InviteID = invite->GetInviteId();
        inviteInfo.InviterGuid = invite->GetSenderGUID();
        inviteInfo.Status = invite->GetStatus();
        inviteInfo.Moderator = invite->GetRank();
        if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(invite->GetEventId()))
            inviteInfo.InviteType = calendarEvent->IsGuildEvent() && calendarEvent->GetGuildId() == _player->GetGuildId();

        packet.Invites.push_back(inviteInfo);
    }

    CalendarEventStore playerEvents = sCalendarMgr->GetPlayerEvents(guid);
    for (auto const& event : playerEvents)
    {
        WorldPackets::Calendar::CalendarSendCalendarEventInfo eventInfo;
        eventInfo.EventID = event->GetEventId();
        eventInfo.Date = event->GetDate();
        if (Guild* guild = sGuildMgr->GetGuildById(event->GetGuildId()))
            eventInfo.EventGuildID = guild->GetGUID();
        eventInfo.EventName = event->GetTitle();
        eventInfo.EventType = event->GetType();
        eventInfo.Flags = event->GetFlags();
        eventInfo.OwnerGuid = event->GetOwnerGUID();
        eventInfo.TextureID = event->GetTextureId();

        packet.Events.push_back(eventInfo);
    }

    SendPacket(packet.Write());
}

namespace
{
    // Event and invite ids are sequential and come from the client: every handler checks who may act on what

    bool CanSeeCalendarEvent(CalendarEvent const& calendarEvent, Player const* player)
    {
        if (calendarEvent.IsServerEvent() || calendarEvent.GetOwnerGUID() == player->GetGUID())
            return true;

        if ((calendarEvent.IsGuildEvent() || calendarEvent.IsGuildAnnouncement()) && calendarEvent.GetGuildId() && calendarEvent.GetGuildId() == player->GetGuildId())
            return true;

        for (CalendarInvite const* invite : sCalendarMgr->GetEventInvites(calendarEvent.GetEventId()))
            if (invite->GetInviteeGUID() == player->GetGUID())
                return true;

        return false;
    }

    CalendarInvite* GetEventInvite(uint64 inviteId, uint64 eventId)
    {
        CalendarInvite* invite = sCalendarMgr->GetInvite(inviteId);
        return invite && invite->GetEventId() == eventId ? invite : nullptr;
    }

    bool IsInvitedTo(uint64 eventId, ObjectGuid guid)
    {
        for (CalendarInvite const* invite : sCalendarMgr->GetEventInvites(eventId))
            if (invite->GetInviteeGUID() == guid)
                return true;

        return false;
    }
}

void WorldSession::HandleCalendarGetEvent(WorldPackets::Calendar::CalendarGetEvent& packet)
{
    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());
    CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID);
    if (calendarEvent && CanSeeCalendarEvent(*calendarEvent, _player))
        sCalendarMgr->SendCalendarEvent(_player->GetGUID(), *calendarEvent, CALENDAR_SENDTYPE_GET);
    else
        sCalendarMgr->SendCalendarCommandResult(_player->GetGUID(), CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarGuildFilter(WorldPackets::Calendar::CalendarGuildFilter& packet)
{
    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
        guild->MassInviteToEvent(this, packet.MinLevel, packet.MaxLevel, packet.MaxRankOrder);
}

namespace
{
    // Blizzard's caps: 30 upcoming events of one's own, 100 upcoming guild events per guild. Past events are left
    // out of the count, since nothing removes them.
    bool IsOverEventCap(Player const* player, bool guildEvent)
    {
        uint32 ownEvents;
        uint32 guildEvents;
        sCalendarMgr->CountUpcomingEvents(player->GetGUID(), player->GetGuildId(), ownEvents, guildEvents);

        return guildEvent ? guildEvents >= CALENDAR_MAX_GUILD_EVENTS : ownEvents >= CALENDAR_MAX_EVENTS;
    }
}

void WorldSession::HandleCalendarAddEvent(WorldPackets::Calendar::CalendarAddEvent& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (packet.EventInfo.Time < (GameTime::GetGameTime() - time_t(86400L)))
        return;

    // calendar_events.Description is varchar(255), the packet allows 2047
    utf8truncate(packet.EventInfo.Description, 255);

    // Strip invisible characters for non-addon messages
    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(packet.EventInfo.Title);
        stripLineInvisibleChars(packet.EventInfo.Description);

        if (strchr(packet.EventInfo.Title.c_str(), '|') || strchr(packet.EventInfo.Description.c_str(), '|'))
        {
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            return;
        }
    }

    bool const guildEvent = (packet.EventInfo.Flags & (CALENDAR_FLAG_GUILD_EVENT | CALENDAR_FLAG_WITHOUT_INVITES)) != 0;

    // The list at creation is checked like an invitation added later (existence, faction, ignore list);
    // the packet array already caps it at CALENDAR_MAX_INVITES. The query is synchronous: it runs before the lock.
    std::set<ObjectGuid> ignoringCreator;
    if (!(packet.EventInfo.Flags & CALENDAR_FLAG_WITHOUT_INVITES)
        && std::any_of(packet.EventInfo.Invites.begin(), packet.EventInfo.Invites.end(), [guid](WorldPackets::Calendar::CalendarAddEventInviteInfo const& invite) { return invite.Guid != guid; }))
    {
        if (QueryResult result = CharacterDatabase.PQuery("SELECT guid FROM character_social WHERE friend = %u AND (flags & %u) <> 0", guid.GetGUIDLow(), uint32(SOCIAL_FLAG_IGNORED)))
        {
            do
                ignoringCreator.insert(ObjectGuid::Create<HighGuid::Player>(result->Fetch()[0].GetUInt64()));
            while (result->NextRow());
        }
    }

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (IsOverEventCap(_player, guildEvent))
    {
        sCalendarMgr->SendCalendarCommandResult(guid, guildEvent ? CALENDAR_ERROR_GUILD_EVENTS_EXCEEDED : CALENDAR_ERROR_EVENTS_EXCEEDED);
        return;
    }

    CalendarEvent* calendarEvent = new CalendarEvent(sCalendarMgr->GetFreeEventId(), guid, UI64LIT(0), CalendarEventType(packet.EventInfo.EventType), packet.EventInfo.TextureID,
        packet.EventInfo.Time, packet.EventInfo.Flags, packet.EventInfo.Title, packet.EventInfo.Description, time_t(0));

    if (calendarEvent->IsGuildEvent() || calendarEvent->IsGuildAnnouncement())
        calendarEvent->SetGuildId(_player->GetGuildId());

    if (calendarEvent->IsGuildAnnouncement())
    {
        CalendarInvite invite(0, calendarEvent->GetEventId(), ObjectGuid::Empty, guid, CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_STATUS_NOT_SIGNED_UP, CALENDAR_RANK_PLAYER, "");
        sCalendarMgr->AddInvite(calendarEvent, &invite);
    }
    else
    {
        CharacterDatabaseTransaction trans;
        if (packet.EventInfo.Invites.size() > 1)
            trans = CharacterDatabase.BeginTransaction();

        std::set<ObjectGuid> invited;
        for (auto i = 0; i < packet.EventInfo.Invites.size(); ++i)
        {
            // a forged list could name the same player many times: one invite, one alert each
            if (!invited.insert(packet.EventInfo.Invites[i].Guid).second)
                continue;

            if (packet.EventInfo.Invites[i].Guid != guid)
            {
                CharacterInfo const* inviteeInfo = sWorld->GetCharacterInfo(packet.EventInfo.Invites[i].Guid);
                if (!inviteeInfo || inviteeInfo->IsDeleted)
                    continue;

                if (_player->GetTeam() != Player::TeamForRace(inviteeInfo->Race) && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CALENDAR))
                    continue;

                if (ignoringCreator.count(packet.EventInfo.Invites[i].Guid))
                    continue;
            }

            CalendarInvite* invite = new CalendarInvite(sCalendarMgr->GetFreeInviteId(), calendarEvent->GetEventId(), packet.EventInfo.Invites[i].Guid,
                guid, CALENDAR_DEFAULT_RESPONSE_TIME, CalendarInviteStatus(packet.EventInfo.Invites[i].Status),
                CalendarModerationRank(packet.EventInfo.Invites[i].Moderator), "");
            sCalendarMgr->AddInvite(calendarEvent, invite, trans);
        }

        if (packet.EventInfo.Invites.size() > 1)
            CharacterDatabase.CommitTransaction(trans);
    }

    sCalendarMgr->AddEvent(calendarEvent, CALENDAR_SENDTYPE_ADD);
}

void WorldSession::HandleCalendarUpdateEvent(WorldPackets::Calendar::CalendarUpdateEvent& packet)
{
    auto oldEventTime = time_t(0);

    if (packet.EventInfo.Time < (GameTime::GetGameTime() - time_t(86400L)))
        return;

    // calendar_events.Description is varchar(255), the packet allows 2047
    utf8truncate(packet.EventInfo.Description, 255);

    // Strip invisible characters for non-addon messages
    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(packet.EventInfo.Title);
        stripLineInvisibleChars(packet.EventInfo.Description);

        if (strchr(packet.EventInfo.Title.c_str(), '|') || strchr(packet.EventInfo.Description.c_str(), '|'))
        {
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            return;
        }
    }

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventInfo.EventID))
    {
        if (!sCalendarMgr->CanModify(calendarEvent, _player->GetGUID()))
        {
            sCalendarMgr->SendCalendarCommandResult(_player->GetGUID(), CALENDAR_ERROR_PERMISSIONS);
            return;
        }

        oldEventTime = calendarEvent->GetDate();

        calendarEvent->SetType(CalendarEventType(packet.EventInfo.EventType));
        calendarEvent->SetFlags(packet.EventInfo.Flags);
        calendarEvent->SetDate(packet.EventInfo.Time);
        calendarEvent->SetTextureId(packet.EventInfo.TextureID);
        calendarEvent->SetTitle(packet.EventInfo.Title);
        calendarEvent->SetDescription(packet.EventInfo.Description);

        sCalendarMgr->UpdateEvent(calendarEvent);
        sCalendarMgr->SendCalendarEventUpdateAlert(*calendarEvent, oldEventTime);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(_player->GetGUID(), CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarRemoveEvent(WorldPackets::Calendar::CalendarRemoveEvent& packet)
{
    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());
    if (!sCalendarMgr->CanModify(sCalendarMgr->GetEvent(packet.EventID), _player->GetGUID()))
    {
        sCalendarMgr->SendCalendarCommandResult(_player->GetGUID(), CALENDAR_ERROR_PERMISSIONS);
        return;
    }

    sCalendarMgr->RemoveEvent(packet.EventID, _player->GetGUID());
}

void WorldSession::HandleCalendarCopyEvent(WorldPackets::Calendar::CalendarCopyEvent& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (packet.Date < (GameTime::GetGameTime() - time_t(86400L)))
        return;

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (CalendarEvent* oldEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        // the copy keeps the original owner and invites: only who may edit the event copies it
        if (!sCalendarMgr->CanModify(oldEvent, guid))
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_PERMISSIONS);
            return;
        }

        bool const guildEvent = oldEvent->IsGuildEvent() || oldEvent->IsGuildAnnouncement();
        if (IsOverEventCap(_player, guildEvent))
        {
            sCalendarMgr->SendCalendarCommandResult(guid, guildEvent ? CALENDAR_ERROR_GUILD_EVENTS_EXCEEDED : CALENDAR_ERROR_EVENTS_EXCEEDED);
            return;
        }

        auto newEvent = new CalendarEvent(*oldEvent, sCalendarMgr->GetFreeEventId());
        newEvent->SetDate(packet.Date);
        sCalendarMgr->AddEvent(newEvent, CALENDAR_SENDTYPE_COPY);

        CalendarInviteStore invites = sCalendarMgr->GetEventInvites(packet.EventID);
        CharacterDatabaseTransaction trans;
        if (invites.size() > 1)
            trans = CharacterDatabase.BeginTransaction();

        for (CalendarInviteStore::const_iterator itr = invites.begin(); itr != invites.end(); ++itr)
            sCalendarMgr->AddInvite(newEvent, new CalendarInvite(**itr, sCalendarMgr->GetFreeInviteId(), newEvent->GetEventId()), trans);

        if (invites.size() > 1)
            CharacterDatabase.CommitTransaction(trans);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventInvite(WorldPackets::Calendar::CalendarEventInvite& packet)
{
    ObjectGuid playerGuid = _player->GetGUID();

    ObjectGuid inviteeGuid;
    uint32 inviteeTeam = 0;
    ObjectGuid::LowType inviteeGuildId = UI64LIT(0);

    // Strip invisible characters for non-addon messages
    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(packet.Name);

        if (strchr(packet.Name.c_str(), '|'))
        {

            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            
            return;
        }
    }

    // the invitee may stand on another map
    ObjectGuid onlineGuid = ObjectAccessor::FindPlayerGuidByName(packet.Name);
    if (!onlineGuid.IsEmpty())
    {
        ObjectAccessor::WithPlayer(onlineGuid, [&inviteeGuid, &inviteeTeam, &inviteeGuildId](Player* player)
        {
            inviteeGuid = player->GetGUID();
            inviteeTeam = player->GetTeam();
            inviteeGuildId = player->GetGuildId();
        });
    }

    if (!inviteeGuid)
    {
        if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(packet.Name))
        {
            inviteeGuid = ObjectGuid::Create<HighGuid::Player>(nameData->Guid);
            inviteeTeam = Player::TeamForRace(nameData->Race);
            inviteeGuildId = nameData->GuildId;
        }
    }

    if (!inviteeGuid)
    {
        sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_PLAYER_NOT_FOUND);
        return;
    }

    if (_player->GetTeam() != inviteeTeam && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CALENDAR))
    {
        sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_NOT_ALLIED);
        return;
    }

    if (QueryResult result = CharacterDatabase.PQuery("SELECT flags FROM character_social WHERE guid = %u AND friend = %u", inviteeGuid.GetGUIDLow(), playerGuid.GetGUIDLow()))
    {
        Field* fields = result->Fetch();
        if (fields[0].GetUInt8() & SOCIAL_FLAG_IGNORED)
        {
            sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_IGNORING_YOU_S, packet.Name.c_str());
            return;
        }
    }

    // after the synchronous query above
    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (!packet.Creating)
    {
        if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
        {
            if (!sCalendarMgr->CanModify(calendarEvent, playerGuid) || calendarEvent->IsGuildAnnouncement())
            {
                sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_PERMISSIONS);
                return;
            }

            if (IsInvitedTo(calendarEvent->GetEventId(), inviteeGuid))
            {
                sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_ALREADY_INVITED_TO_EVENT_S, packet.Name.c_str());
                return;
            }

            if (sCalendarMgr->GetEventInvites(calendarEvent->GetEventId()).size() >= CALENDAR_MAX_INVITES)
            {
                sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_INVITES_EXCEEDED);
                return;
            }

            if (calendarEvent->IsGuildEvent() && calendarEvent->GetGuildId() == inviteeGuildId)
            {
                sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_NO_GUILD_INVITES);
                return;
            }

            CalendarInvite* invite = new CalendarInvite(sCalendarMgr->GetFreeInviteId(), packet.EventID, inviteeGuid, playerGuid, CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_STATUS_INVITED, CALENDAR_RANK_PLAYER, "");
            sCalendarMgr->AddInvite(calendarEvent, invite);
        }
        else
            sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_EVENT_INVALID);
    }
    else
    {
        if (packet.IsSignUp && inviteeGuildId == _player->GetGuildId())
        {
            sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_NO_GUILD_INVITES);
            return;
        }

        CalendarInvite invite(sCalendarMgr->GetFreeInviteId(), 0L, inviteeGuid, playerGuid, CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_STATUS_INVITED, CALENDAR_RANK_PLAYER, "");
        sCalendarMgr->SendCalendarEventInvite(invite);
    }
}

void WorldSession::HandleCalendarEventSignup(WorldPackets::Calendar::CalendarEventSignUp& packet)
{
    ObjectGuid guid = _player->GetGUID();

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        // signing up is for guild events of the player guild, once (announcements keep no invites)
        if (!calendarEvent->IsGuildEvent() || calendarEvent->IsGuildAnnouncement() || !calendarEvent->GetGuildId() || calendarEvent->GetGuildId() != _player->GetGuildId())
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_GUILD_PLAYER_NOT_IN_GUILD);
            return;
        }

        if (IsInvitedTo(calendarEvent->GetEventId(), guid))
        {
            sCalendarMgr->SendCalendarClearPendingAction(guid);
            return;
        }

        CalendarInviteStatus status = packet.Tentative ? CALENDAR_STATUS_TENTATIVE : CALENDAR_STATUS_SIGNED_UP;
        CalendarInvite* invite = new CalendarInvite(sCalendarMgr->GetFreeInviteId(), packet.EventID, guid, guid, GameTime::GetGameTime(), status, CALENDAR_RANK_PLAYER, "");
        sCalendarMgr->AddInvite(calendarEvent, invite);
        sCalendarMgr->SendCalendarClearPendingAction(guid);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventRsvp(WorldPackets::Calendar::CalendarEventRSVP& packet)
{
    ObjectGuid guid = _player->GetGUID();

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (packet.Status != CALENDAR_STATUS_REMOVED && calendarEvent->IsLocked())
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_LOCKED);
            return;
        }

        CalendarInvite* invite = GetEventInvite(packet.InviteID, packet.EventID);
        if (invite && invite->GetInviteeGUID() == guid) // a player answers his own invite only
        {
            invite->SetStatus(CalendarInviteStatus(packet.Status));
            invite->SetResponseTime(GameTime::GetGameTime());

            sCalendarMgr->UpdateInvite(invite);
            sCalendarMgr->SendCalendarEventStatus(*calendarEvent, *invite);
            sCalendarMgr->SendCalendarClearPendingAction(guid);
        }
        else
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventRemoveInvite(WorldPackets::Calendar::CalendarRemoveInvite& packet)
{
    ObjectGuid guid = _player->GetGUID();

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        CalendarInvite* invite = GetEventInvite(packet.InviteID, packet.EventID);
        if (!invite)
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
            return;
        }

        // the real invitee, not the guid the client names
        if (calendarEvent->GetOwnerGUID() == invite->GetInviteeGUID())
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_DELETE_CREATOR_FAILED);
            return;
        }

        // a player leaves by himself, or is removed by the owner or a moderator
        if (invite->GetInviteeGUID() != guid && !sCalendarMgr->CanModify(calendarEvent, guid))
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_PERMISSIONS);
            return;
        }

        sCalendarMgr->RemoveInvite(packet.InviteID, packet.EventID, guid);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
}

void WorldSession::HandleCalendarEventStatus(WorldPackets::Calendar::CalendarEventStatus& packet)
{
    ObjectGuid guid = _player->GetGUID();

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (!sCalendarMgr->CanModify(calendarEvent, guid))
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_PERMISSIONS);
            return;
        }

        if (CalendarInvite* invite = GetEventInvite(packet.InviteID, packet.EventID))
        {
            invite->SetStatus(static_cast<CalendarInviteStatus>(packet.Status));

            sCalendarMgr->UpdateInvite(invite);
            sCalendarMgr->SendCalendarEventStatus(*calendarEvent, *invite);
            sCalendarMgr->SendCalendarClearPendingAction(invite->GetInviteeGUID());
        }
        else
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventModeratorStatus(WorldPackets::Calendar::CalendarEventModeratorStatus& packet)
{
    ObjectGuid guid = _player->GetGUID();

    std::lock_guard<std::recursive_mutex> guard(sCalendarMgr->GetLock());

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (!sCalendarMgr->CanModify(calendarEvent, guid) || packet.Status > CALENDAR_RANK_MODERATOR)
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_PERMISSIONS);
            return;
        }

        if (CalendarInvite* invite = GetEventInvite(packet.InviteID, packet.EventID))
        {
            invite->SetRank(CalendarModerationRank(packet.Status));
            sCalendarMgr->UpdateInvite(invite);
            sCalendarMgr->SendCalendarEventModeratorStatusAlert(*calendarEvent, *invite);
        }
        else
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarComplain(WorldPackets::Calendar::CalendarComplain& /*packet*/)
{ }

void WorldSession::HandleCalendarGetNumPending(WorldPackets::Calendar::CalendarGetNumPending& /*packet*/)
{
    SendPacket(WorldPackets::Calendar::CalendarSendNumPending(sCalendarMgr->GetPlayerNumPending(_player->GetGUID())).Write());
}
