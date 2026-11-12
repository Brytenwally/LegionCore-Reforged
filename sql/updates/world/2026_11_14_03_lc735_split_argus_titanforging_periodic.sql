-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_150_split_argus_titanforging_periodic.sql (2026-11-09).
UPDATE `spell_script_names`
SET `ScriptName` = 'spell_argus_titanforging_reduced_energy_periodic'
WHERE `spell_id` = 257214
  AND `ScriptName` = 'spell_argus_titanforging_energize_periodic';
