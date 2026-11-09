-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_21_250_restore_warlock_t21.sql (2026-11-09).
-- Restore the scripted effect for the Antorus destruction warlock 4-piece bonus.
DELETE FROM `spell_script_names`
WHERE `spell_id` = 251855 AND `ScriptName` = 'spell_warl_t21_destruction_4p';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(251855, 'spell_warl_t21_destruction_4p');
