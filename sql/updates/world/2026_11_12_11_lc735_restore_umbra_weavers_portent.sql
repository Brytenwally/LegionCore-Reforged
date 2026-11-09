-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_21_258_restore_umbra_weavers_portent.sql (2026-11-09).
-- Restore the conditional three-piece mastery bonus for Umbra-Weaver's Portent.
DELETE FROM `spell_script_names`
WHERE (`spell_id` = 253826 AND `ScriptName` = 'spell_gen_umbral_shift')
   OR (`spell_id` = 253825 AND `ScriptName` = 'spell_gen_umbra_weavers_portent');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(253826, 'spell_gen_umbral_shift'),
(253825, 'spell_gen_umbra_weavers_portent');
