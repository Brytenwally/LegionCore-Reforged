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

#include "ChatTextBuilder.h"
#include "ChatPackets.h"

Trinity::BroadcastTextBuilder::BroadcastTextBuilder(Unit const* obj, ChatMsg msgType, uint32 textId, WorldObject const* target, uint32 achievementId): _source(obj), _target(target), _msgType(msgType), _textId(textId), _achievementId(achievementId) { }

WorldPackets::Packet* Trinity::BroadcastTextBuilder::operator()(LocaleConstant locale) const
{
    auto bct = sBroadcastTextStore.LookupEntry(_textId);

    // null means "send nothing", LocalizedPacketDo skips the receiver
    if (bct && _target)
        if (_target->IsUnit() && !sConditionMgr->IsPlayerMeetingCondition(const_cast<Unit*>(_target->ToUnit()), bct->ConditionID))
            return nullptr;

    auto chat = new WorldPackets::Chat::Chat();
    chat->Initialize(_msgType, bct ? Language(bct->LanguageID) : LANG_UNIVERSAL, _source, _target, bct ? DB2Manager::GetBroadcastTextValue(bct, locale, _source? _source->getGender() : GENDER_MALE) : "", _achievementId, "", locale);
    return chat;
}

Trinity::CustomChatTextBuilder::CustomChatTextBuilder(WorldObject const* obj, ChatMsg msgType, std::string const& text, Language language, WorldObject const* target): _source(obj), _target(target), _msgType(msgType), _language(language), _text(text) { }

WorldPackets::Packet* Trinity::CustomChatTextBuilder::operator()(LocaleConstant locale) const
{
    WorldPackets::Chat::Chat* chat = new WorldPackets::Chat::Chat();
    chat->Initialize(_msgType, _language, _source, _target, _text, 0, "", locale);
    return chat;
}

Trinity::TrinityStringChatBuilder::TrinityStringChatBuilder(WorldObject const* obj, ChatMsg msgType, uint32 textId, WorldObject const* target, va_list* args): _source(obj), _target(target), _args(args), _msgType(msgType), _textId(textId) { }

WorldPackets::Packet* Trinity::TrinityStringChatBuilder::operator()(LocaleConstant locale) const
{
    auto packet = new WorldPackets::Chat::Chat();

    auto text = sObjectMgr->GetTrinityString(_textId, locale);

    if (_args)
    {
        // we need copy va_list before use or original va_list will corrupted
        va_list ap;
        va_copy(ap, *_args);

        static size_t const BufferSize = 2048;
        char strBuffer[BufferSize];
        vsnprintf(strBuffer, BufferSize, text, ap);
        va_end(ap);

        packet->Initialize(_msgType, LANG_UNIVERSAL, _source, _target, strBuffer, 0, "", locale);
    }
    else
        packet->Initialize(_msgType, LANG_UNIVERSAL, _source, _target, text, 0, "", locale);

    return packet;
}
