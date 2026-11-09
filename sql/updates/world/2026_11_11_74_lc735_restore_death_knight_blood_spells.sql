-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_190_restore_death_knight_blood_spells.sql (2026-11-09).
-- Restore 7.3.5 Blood Death Knight mechanics that require per-target logic.
DELETE FROM `spell_script_names`
WHERE `ScriptName` IN
(
    'spell_dk_vampiric_aura',
    'spell_dk_crimson_scourge'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(238698, 'spell_dk_vampiric_aura'),
(81136,  'spell_dk_crimson_scourge');

-- The generic trigger cannot enforce caster-owned Blood Plague or restore
-- Death and Decay's charge. The bound AuraScript performs both 7.3.5 rules.
DELETE FROM `spell_trigger`
WHERE `spell_id` = 81136 AND `spell_trigger` = 81141;
