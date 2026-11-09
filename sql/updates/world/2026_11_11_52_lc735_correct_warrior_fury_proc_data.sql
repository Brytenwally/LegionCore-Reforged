-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_168_correct_warrior_fury_proc_data.sql (2026-11-09).
-- Match Rage of the Valarjar to the final 7.3.5.26972 client proc data.
-- A zero custom chance makes the core use SpellAuraOptions::ProcChance (10%)
-- while preserving the existing family masks and the client's 15 second ICD.
UPDATE `spell_proc_event`
SET `CustomChance` = 0
WHERE `entry` = 200845;
