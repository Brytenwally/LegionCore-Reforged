-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_169_restore_warrior_protection_talents.sql (2026-11-09).
-- Restore the server-side dummy and conditional portions of Protection
-- talents and artifact traits. Native damage, Rage and spell-mod effects
-- remain in the final client data.

DELETE FROM `spell_script_names`
WHERE `ScriptName` IN
(
    'spell_warr_devastator',
    'spell_warr_heavy_repercussions',
    'spell_warr_shatter_the_bones',
    'spell_warr_shield_block_artifact',
    'spell_warr_reflective_plating',
    'spell_warr_scales_of_earth'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(236279, 'spell_warr_devastator'),
(23922,  'spell_warr_heavy_repercussions'),
(188639, 'spell_warr_shatter_the_bones'),
(132404, 'spell_warr_shield_block_artifact'),
(23920,  'spell_warr_reflective_plating'),
(189059, 'spell_warr_scales_of_earth');
