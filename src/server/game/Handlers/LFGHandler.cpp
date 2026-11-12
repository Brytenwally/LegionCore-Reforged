////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///   Copyright 2005-2009 MaNGOS         ///
///   <http://getmangos.com/>            ///
///                                      ///
////////////////////////////////////////////

#include "LFGMgr.h"
#include "Group.h"
#include "QuestData.h"
#include "LFGPackets.h"
#include "BattlegroundDefines.h"
#include "BattlegroundPackets.h"

void WorldSession::HandleLfgJoin(WorldPackets::LFG::LfgJoin& packet)
{
    if (packet.Slot.empty())
        return;

    if (!sLFGMgr->isOptionEnabled(lfg::LFG_OPTION_ENABLE_DUNGEON_FINDER | lfg::LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    // the A'shran button belongs to the Ashran battleground queue, not to the dungeon finder (slot = dungeon id in its low 24 bits)
    if (packet.Slot.size() == 1 && (*packet.Slot.begin() & 0x00FFFFFF) == lfg::LFG_DUNGEON_ASHRAN)
    {
        // the client flags this button's join as a group join even for a player alone
        JoinBattlegroundQueue(MS::Battlegrounds::BattlegroundTypeId::BattlegroundAshran, uint8(packet.Roles), GetPlayer()->GetGroup() != nullptr, WorldPackets::Battleground::IgnorMapInfo());
        return;
    }

    sLFGMgr->JoinLfg(GetPlayer(), packet.Roles, packet.Slot);
}

void WorldSession::HandleLfgLeave(WorldPackets::LFG::LfgLeave& packet)
{
    Player* player = GetPlayer();
    // a queue the server no longer knows (lost with a reconnection) is still shown by the client: it is told it left, or it stays stuck
    if (packet.Ticket.Id && !sLFGMgr->HasPlayerData(player->GetGUID(), packet.Ticket.Id))
    {
        WorldPackets::LFG::QueueStatusUpdate update;
        update.Ticket = packet.Ticket;
        update.SubType = LFG_QUEUE_DUNGEON;
        update.Reason = lfg::LFG_UPDATETYPE_REMOVED_FROM_QUEUE;
        update.IsParty = player->GetGroup() ? player->GetGroup()->GetLeaderGUID() == player->GetGUID() : true;
        update.NotifyUI = true;
        update.Joined = false;
        update.LfgJoined = false;
        update.Queued = false;
        SendPacket(update.Write());
        return;
    }

    sLFGMgr->LeaveLfg(player->GetGUID(), packet.Ticket.Id);
}

void WorldSession::HandleLfgProposalResponse(WorldPackets::LFG::LfgProposalResponse& packet)
{
    sLFGMgr->UpdateProposal(packet.Data, GetPlayer()->GetGUID());
}

void WorldSession::HandleLfgCompleteRoleCheck(WorldPackets::LFG::LfgCompleteRoleCheck& packet)
{
    if (Group* group = GetPlayer()->GetGroup())
        sLFGMgr->UpdateRoleCheck(group->GetGUID(), GetPlayer()->GetGUID(), packet.RolesDesired, packet.PartyIndex);
}

void WorldSession::HandleLfgBootPlayerVote(WorldPackets::LFG::LfgBootPlayerVote& packet)
{
    if (Group* group = GetPlayer()->GetGroup())
        sLFGMgr->UpdateBoot(group->GetGUID(), GetPlayer()->GetGUID(), packet.Vote);
}

void WorldSession::HandleLfgCompleteReadyCheck(WorldPackets::LFG::CompleteReadyCheck& /*packet*/)
{ }

void WorldSession::HandleLfgTeleport(WorldPackets::LFG::LfgTeleport& packet)
{
    sLFGMgr->TeleportPlayer(GetPlayer(), packet.TeleportOut, true);
}

void WorldSession::HandleLockInfoRequest(WorldPackets::LFG::LockInfoRequest& packet)
{
    Player* player = GetPlayer();
    if (!player->IsInWorld() || !player->GetSession())
        return;

    if (packet.Player)
        sLFGMgr->SendLfgPlayerLockInfo(player);
    else
        sLFGMgr->SendLfgPartyLockInfo(player);
}

void WorldSession::HandleDfGetJoinStatus(WorldPackets::LFG::NullCmsg& /*packet*/)
{
    if (!sLFGMgr->HasQueue(GetPlayer()->GetGUID()))
        return;

    if (GetPlayer()->IsInWorld())
        sLFGMgr->SendLfgUpdateQueue(GetPlayer()->GetGUID());
}

void WorldSession::HandleBonusFactionID(WorldPackets::LFG::BonusFactionID& packet)
{
    GetPlayer()->SetLfgBonusFaction(packet.FactionID);
}
