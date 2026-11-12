////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2015 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#include "ArtifactPackets.h"
#include "PacketUtilities.h"

WorldPacket const* WorldPackets::Artifact::XpGain::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << Xp;

    return &_worldPacket;
}

void WorldPackets::Artifact::AddPower::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    Powers.resize(CheckElementCount(_worldPacket, _worldPacket.read<uint32>(), 5, 64));
    for (auto& v : Powers)
    {
        _worldPacket >> v.PowerID;
        _worldPacket >> v.Rank;
    }
}

void WorldPackets::Artifact::ConfirmRespec::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> NpcGUID;
}

void WorldPackets::Artifact::SetAppearance::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    _worldPacket >> AppearanceID;
}

WorldPacket const* WorldPackets::Artifact::ForgeOpenResult::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << GameObjectGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Artifact::ArtifactKnowledge::Write()
{
    _worldPacket << int32(ArtifactCategoryID);
    _worldPacket << int8(KnowledgeLevel);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Artifact::RespecResult::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << NpcGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Artifact::ArtifactTraitsRefunded::Write()
{
    _worldPacket << Guid;
    _worldPacket << UnkInt;
    _worldPacket << UnkInt2;

    return &_worldPacket;
}

void WorldPackets::Artifact::ArtifactAddRelicTalent::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    _worldPacket >> SlotIndex;
    _worldPacket >> TalentIndex;
}

void WorldPackets::Artifact::ArtifactAttuneSocketedRelic::Read()
{
    _worldPacket >> ArtifactGUID;
    _worldPacket >> GameObjectGUID;
    _worldPacket >> RelicSlotIndex;
}

void WorldPackets::Artifact::ArtifactAttunePreviewdRelic::Read()
{
    _worldPacket >> RelicGUID;
    _worldPacket >> GameObjectGUID;
}

WorldPacket const* WorldPackets::Artifact::ArtifactAttuneSocketedRelicData::Write()
{
    _worldPacket << ArtifactGUID;
    _worldPacket << Result;

    return &_worldPacket;
}
