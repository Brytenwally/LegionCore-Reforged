-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_172_restore_paladin_retribution_spells.sql (2026-11-09).
-- Restore the Legion 7.3.5 server-side parts of Templar's Verdict and Divine Hammer.
DELETE FROM `spell_script_names`
WHERE `spell_id` IN (85256, 198034)
  AND `ScriptName` IN ('spell_pal_templars_verdict', 'spell_pal_divine_hammer');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(85256,  'spell_pal_templars_verdict'),
(198034, 'spell_pal_divine_hammer');
