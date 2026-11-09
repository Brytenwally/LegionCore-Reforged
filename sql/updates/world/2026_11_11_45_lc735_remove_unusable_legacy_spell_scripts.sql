-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_153_remove_unusable_legacy_spell_scripts.sql (2026-11-09).
DELETE FROM `spell_script_names`
WHERE (`spell_id`, `ScriptName`) IN
(
    (47977, 'spell_magic_broom'),
    (48025, 'spell_headless_horseman_mount'),
    (52408, 'spell_gen_seaforium_blast'),
    (71342, 'spell_big_love_rocket'),
    (72286, 'spell_invincible'),
    (75614, 'spell_celestial_steed'),
    (75973, 'spell_x53_touring_rocket')
);
