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

#include "TokenPackets.h"

void WorldPackets::Token::UpdateListedAuctionableTokens::Read()
{
    _worldPacket >> Type;
}

WorldPacket const* WorldPackets::Token::UpdateListedAuctionableTokensResponse::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;
    _worldPacket << static_cast<uint32>(AuctionableTokenAuctionableList.size());
    for (auto const& v : AuctionableTokenAuctionableList)
    {
        _worldPacket << v.DistributionID;
        _worldPacket << v.DateCreated;
        _worldPacket << v.Owner;
        _worldPacket << v.BuyoutPrice;
        _worldPacket << v.EndTime;
    }

    return &_worldPacket;
}

void WorldPackets::Token::RequestWowTokenMarketPrice::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenMarketPriceResponse::Write()
{
    _worldPacket << CurrentMarketPrice;
    _worldPacket << UnkInt;
    _worldPacket << Result;
    _worldPacket << UnkInt2;

    return &_worldPacket;
}

void WorldPackets::Token::CheckVeteranTokenEligibility::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenCanVeteranBuyResult::Write()
{
    _worldPacket << UnkLong;
    _worldPacket << UnkInt;
    _worldPacket << UnkInt2;

    return &_worldPacket;
}

// --- Buy Token ---

void WorldPackets::Token::BuyWowTokenStart::Read()
{
    _worldPacket >> UnkInt;
    _worldPacket >> Buyer;
    _worldPacket >> CurrentMarketPrice;
}

void WorldPackets::Token::BuyWowTokenConfirm::Read()
{
    _worldPacket >> UnkInt;
    _worldPacket >> PendingBuyConfirmations;
    _worldPacket >> GuaranteedPrice;
    Confirmed = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Token::WowTokenBuyRequestConfirmation::Write()
{
    _worldPacket << CurrentMarketPrice;
    _worldPacket << TokenCount;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Token::WowTokenBuyResultConfirmation::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

// --- Sell Token ---

void WorldPackets::Token::SellWowTokenStart::Read()
{
    _worldPacket >> UnkInt64;
    _worldPacket >> CurrentMarketPrice;
    _worldPacket >> UnkInt;
}

void WorldPackets::Token::SellWowTokenConfirm::Read()
{
    _worldPacket >> TokenGuid;
    _worldPacket >> UnkInt;
    _worldPacket >> PendingBuyConfirmations;
    _worldPacket >> GuaranteedPrice;
    Confirmed = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Token::WowTokenSellRequestConfirmation::Write()
{
    _worldPacket << GuaranteedPrice;
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Token::WowTokenSellResultConfirmation::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

// --- Redeem Token ---

void WorldPackets::Token::RedeemWowTokenStart::Read()
{
    _worldPacket >> Count;
    _worldPacket >> UnkInt;
    _worldPacket >> UnkInt2;
}

void WorldPackets::Token::RedeemWowTokenConfirm::Read()
{
    _worldPacket >> UnkInt;
    _worldPacket >> Count;
    _worldPacket >> TokenGuid;
    _worldPacket >> UnkInt2;
    Confirm = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Token::WowTokenRedeemRequestConfirmation::Write()
{
    _worldPacket << Count;
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Token::WowTokenRedeemResult::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

// --- Update Token Count ---

void WorldPackets::Token::UpdateWowTokenCount::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenDistributionGlueUpdate::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << UnkInt2;
    _worldPacket << int32(DistributionIDs1.size());
    _worldPacket << int32(DistributionIDs2.size());
    for (int64 id : DistributionIDs1)
        _worldPacket << id;
    for (int64 id : DistributionIDs2)
        _worldPacket << id;

    return &_worldPacket;
}

// --- Can Redeem For Balance ---

void WorldPackets::Token::CanRedeemWowTokenForBalance::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenCanRedeemForBalanceResult::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}
