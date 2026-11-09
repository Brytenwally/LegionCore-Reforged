-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_176_restore_hunter_marksmanship_proc_filters.sql (2026-11-09).
-- Restore the Legion 7.3.5 Marksmanship proc restrictions whose DB2 masks
-- are intentionally broad and require spell-specific server validation.
DELETE FROM `spell_script_names`
WHERE `spell_id` IN (35110, 223138)
  AND `ScriptName` IN ('spell_hun_bombardment', 'spell_hun_marking_targets');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(35110,  'spell_hun_bombardment'),
(223138, 'spell_hun_marking_targets');
