-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_197_restore_shaman_hot_hand_and_sundering.sql (2026-11-09).
-- Complete two Enhancement Shaman talent chains that were only partially active.

DELETE FROM `spell_script_names`
WHERE `ScriptName` IN ('spell_sha_hot_hand', 'spell_sha_sundering');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(201900, 'spell_sha_hot_hand'),
(197214, 'spell_sha_sundering');

-- Hot Hand uses its build-26972 RPPM data through the core's active legacy
-- proc path. The AuraScript only adds the missing Flametongue requirement.
DELETE FROM `spell_proc` WHERE `spellId` = 201900;
