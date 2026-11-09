-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_199_restore_mage_arcane_mechanics.sql (2026-11-09).
-- Restore missing Legion 7.3.5 Arcane Mage mechanics from build 26972 data.

DELETE FROM `spell_script_names`
WHERE `ScriptName` IN
(
    'spell_mage_arcane_missiles',
    'spell_mage_arcane_missiles_damage',
    'spell_mage_arcane_missiles_charges',
    'spell_mage_erosion_driver',
    'spell_mage_evocation',
    'spell_mage_mark_of_aluneth'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5143,   'spell_mage_arcane_missiles'),
(7268,   'spell_mage_arcane_missiles_damage'),
(79683,  'spell_mage_arcane_missiles_charges'),
(12051,  'spell_mage_evocation'),
(205039, 'spell_mage_erosion_driver'),
(224968, 'spell_mage_mark_of_aluneth');
