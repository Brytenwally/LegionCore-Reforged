////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2016 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#include "BattlenetPackets.h"
#include "PacketUtilities.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Battlenet::MethodCall const& method)
{
    data << uint64(method.Type);
    data << uint64(method.ObjectId);
    data << uint32(method.Token);
    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Battlenet::MethodCall& method)
{
    data >> method.Type;
    data >> method.ObjectId;
    data >> method.Token;
    return data;
}

WorldPacket const* WorldPackets::Battlenet::Notification::Write()
{
    _worldPacket << Method;
    _worldPacket << uint32(Data.size());
    _worldPacket.append(Data);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battlenet::Response::Write()
{
    _worldPacket << uint32(BnetStatus);
    _worldPacket << Method;
    _worldPacket << uint32(Data.size());
    _worldPacket.append(Data);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battlenet::SetSessionState::Write()
{
    _worldPacket.WriteBits(State, 2);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battlenet::RealmListTicket::Write()
{
    _worldPacket << uint32(Token);
    _worldPacket.WriteBit(Allow);
    _worldPacket << uint32(Ticket.size());
    _worldPacket.append(Ticket);

    return &_worldPacket;
}

void WorldPackets::Battlenet::Request::Read()
{
    uint32 protoSize;

    _worldPacket >> Method;
    _worldPacket >> protoSize;

    CheckElementCount(_worldPacket, protoSize, 1, 0x40000);
    Data.Resize(protoSize);
    _worldPacket.read(Data.GetWritePointer(), Data.GetRemainingSpace());
    Data.WriteCompleted(protoSize);
}

void WorldPackets::Battlenet::RequestRealmListTicket::Read()
{
    _worldPacket >> Token;
    _worldPacket.read(Secret.data(), Secret.size());
}
