-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_165_restore_warrior_leap_ravager.sql (2026-11-09).
-- Restore the destination launch spells for Heroic Leap and Ravager. Heroic
-- Leap already has its script binding; Ravager needs the cast handler on both
-- Legion 7.3.5 specialization variants.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_ravager_cast';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(152277, 'spell_warr_ravager_cast'),
(228920, 'spell_warr_ravager_cast');
