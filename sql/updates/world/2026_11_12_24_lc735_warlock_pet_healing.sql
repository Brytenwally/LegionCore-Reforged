-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_12_10_warlock_pet_healing.sql (2026-11-09).
-- 7.3.5 Soul Link heals from Soul Leech absorption, not split incoming damage.
-- The core now handles that healing for damage procs and Demon Skin recharge.
DELETE FROM `spell_script_names`
WHERE `spell_id` = 108446 AND `ScriptName` = 'spell_warl_soul_link';
