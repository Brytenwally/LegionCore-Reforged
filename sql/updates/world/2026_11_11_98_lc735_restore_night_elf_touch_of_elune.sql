-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_20_214_restore_night_elf_touch_of_elune.sql (2026-11-09).
-- Touch of Elune (154748) is a passive selector. Its DB2 effect is only a
-- dummy aura; the server must apply the day (154796) or night (154797) stat
-- aura according to realm time.
DELETE FROM `spell_script_names`
WHERE `spell_id` = 154748 AND `ScriptName` = 'spell_gen_touch_of_elune';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(154748, 'spell_gen_touch_of_elune');
