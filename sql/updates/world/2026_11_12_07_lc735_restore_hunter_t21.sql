-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_21_254_restore_hunter_t21.sql (2026-11-09).
-- Restore the scripted Beast Mastery effect for the Antorus hunter Tier 21 set.
-- Marksmanship 4P is handled by the existing Marked Shot script.
DELETE FROM `spell_script_names`
WHERE `spell_id` = 251756 AND `ScriptName` = 'spell_hun_t21_beast_mastery_4p';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(251756, 'spell_hun_t21_beast_mastery_4p');
