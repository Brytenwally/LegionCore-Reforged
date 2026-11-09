-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_205_restore_monk_brewmaster_mechanics.sql (2026-11-09).
-- Restore Legion 7.3.5 Brewmaster mechanics that require server-side spell handling.

DELETE FROM `spell_script_names`
WHERE `spell_id` = 195630
  AND `ScriptName` = 'spell_monk_elusive_brawler';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(195630, 'spell_monk_elusive_brawler');
