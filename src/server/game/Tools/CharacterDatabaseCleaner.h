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

#ifndef CHARACTERDATABASECLEANER_H
#define CHARACTERDATABASECLEANER_H

namespace CharacterDatabaseCleaner
{
    enum CleaningFlags
    {
        CLEANING_FLAG_ACHIEVEMENT_PROGRESS  = 0x1,
        CLEANING_FLAG_SKILLS                = 0x2,
        CLEANING_FLAG_SPELLS                = 0x4,
        CLEANING_FLAG_QUESTSTATUS           = 0x10,
        CLEANING_FLAG_PETSLOT               = 0x20      //Now used for store pet slots on new plr field, after for find lost link pets.
    };

    void CleanDatabase();
    void ResetCharacterDB();

    void CheckUnique(const char* column, const char* table, bool (*check)(uint32));

    bool AchievementProgressCheck(uint32 criteria);
    bool SkillCheck(uint32 skill);
    bool SpellCheck(uint32 spell_id);

    void CleanAllAchievementProgress();
    void CleanGuildAchievementProgress();
    void CleanAccountAchievementProgress();
    void CleanCharacterAchievementProgress();
    void CleanCharacterSkills();
    void CleanCharacterSpell();
    void CleanCharacterQuestStatus();
}

#endif
