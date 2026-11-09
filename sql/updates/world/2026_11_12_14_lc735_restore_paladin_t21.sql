-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_21_264_restore_paladin_t21.sql (2026-11-09).
-- Restore the scripted interaction between the Antorus Protection Paladin
-- 2-piece and 4-piece bonuses. Holy and Retribution T21 effects are handled
-- by their client aura data and the core's existing heal/proc paths.
DELETE FROM `spell_script_names`
WHERE `spell_id` = 251869 AND `ScriptName` = 'spell_pal_t21_prot';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(251869, 'spell_pal_t21_prot');
