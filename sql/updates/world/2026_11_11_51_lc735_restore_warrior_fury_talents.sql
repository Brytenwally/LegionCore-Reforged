-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_167_restore_warrior_fury_talents.sql (2026-11-09).
-- Restore Fury talent behavior which the final client data cannot complete on
-- its own. Fresh Meat and Focus in Chaos are handled in the core calculations.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_war_machine';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(215556, 'spell_warr_war_machine');
