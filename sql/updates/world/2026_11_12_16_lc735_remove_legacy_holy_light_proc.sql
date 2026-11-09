-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_21_277_remove_legacy_holy_light_proc.sql (2026-11-09).
-- The same T16 rule already contains Legion Holy Light (82326).  Remove the
-- duplicate condition for retired Holy Light spell 635.
DELETE FROM `spell_proc_check`
WHERE `entry` = 144624 AND `checkspell` = 635;
