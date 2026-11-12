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

#include "BlackMarketPackets.h"
#include "BlackMarketMgr.h"
#include "Player.h"

void WorldPackets::BlackMarket::BlackMarketItem::Initialize(BlackMarketEntry* const entry, Player* player)
{
    BlackMarketTemplate* templ = entry->GetTemplate();

    MarketID = entry->GetMarketId();
    SellerNPC = templ->SellerNPC;
    Item = templ->Item;
    Quantity = templ->Quantity;

    if (!entry->GetNumBids())
    {
        MinBid = templ->MinBid;
        MinIncrement = 1;
    }
    else
    {
        MinIncrement = entry->GetMinIncrement();
        MinBid = entry->GetCurrentBid() + MinIncrement;
    }

    CurrentBid = entry->GetCurrentBid();
    SecondsRemaining = entry->GetSecondsRemaining();
    HighBid = (entry->GetBidder() == player->GetGUIDLow());
    NumBids = entry->GetNumBids();
}

void WorldPackets::BlackMarket::BlackMarketOpen::Read()
{
    _worldPacket >> Guid;
}

WorldPacket const* WorldPackets::BlackMarket::BlackMarketOpenResult::Write()
{
    _worldPacket << Guid;
    _worldPacket.WriteBit(Enable);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::BlackMarket::BlackMarketRequestItems::Read()
{
    _worldPacket >> Guid;
    _worldPacket >> LastUpdateID;
}

WorldPacket const* WorldPackets::BlackMarket::BlackMarketRequestItemsResult::Write()
{
    _worldPacket << int32(LastUpdateID);
    _worldPacket << uint32(Items.size());

    for (auto const&  item : Items)
        _worldPacket << item;

    return &_worldPacket;
}

void WorldPackets::BlackMarket::BlackMarketBidOnItem::Read()
{
    _worldPacket >> Guid;
    _worldPacket >> MarketID;
    _worldPacket >> BidAmount;
    _worldPacket >> Item;
}

WorldPacket const* WorldPackets::BlackMarket::BlackMarketBidOnItemResult::Write()
{
    _worldPacket << int32(MarketID);
    _worldPacket << int32(Result);
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BlackMarket::BlackMarketOutbid::Write()
{
    _worldPacket << int32(MarketID);
    _worldPacket << int32(RandomPropertiesID);
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BlackMarket::BlackMarketWon::Write()
{
    _worldPacket << int32(MarketID);
    _worldPacket << int32(RandomPropertiesID);
    _worldPacket << Item;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BlackMarket::BlackMarketItem const& blackMarketItem)
{
    data << int32(blackMarketItem.MarketID);
    data << int32(blackMarketItem.SellerNPC);
    data << int32(blackMarketItem.Quantity);
    data << uint64(blackMarketItem.MinBid);
    data << uint64(blackMarketItem.MinIncrement);
    data << uint64(blackMarketItem.CurrentBid);
    data << int32(blackMarketItem.SecondsRemaining);
    data << int32(blackMarketItem.NumBids);
    data << blackMarketItem.Item;
    data.WriteBit(blackMarketItem.HighBid);
    data.FlushBits();

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::BlackMarket::BlackMarketItem& blackMarketItem)
{
    data >> blackMarketItem.MarketID;
    data >> blackMarketItem.SellerNPC;
    data >> blackMarketItem.Quantity;
    data >> blackMarketItem.MinBid;
    data >> blackMarketItem.MinIncrement;
    data >> blackMarketItem.CurrentBid;
    data >> blackMarketItem.SecondsRemaining;
    data >> blackMarketItem.NumBids;
    data >> blackMarketItem.Item;
    blackMarketItem.HighBid = data.ReadBit();

    return data;
}