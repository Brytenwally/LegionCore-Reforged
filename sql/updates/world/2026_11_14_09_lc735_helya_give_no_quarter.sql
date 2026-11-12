-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_20_00_helya_give_no_quarter.sql (2026-11-09).
-- Restore the native Mariner selector before launching destination missiles.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(228633, 'spell_tov_give_no_quarter');
