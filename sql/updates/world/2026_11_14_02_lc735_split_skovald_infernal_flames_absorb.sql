-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_149_split_skovald_infernal_flames_absorb.sql (2026-11-09).
-- Infernal Flames Aura (193983) uses SCHOOL_ABSORB on effect 1, while the
-- Aegis Override spell sharing its removal script uses DUMMY there. Keep the
-- common removal lifecycle and bind absorb behavior only to Infernal Flames.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (193983, 'spell_skovald_infernal_flames_absorb');
