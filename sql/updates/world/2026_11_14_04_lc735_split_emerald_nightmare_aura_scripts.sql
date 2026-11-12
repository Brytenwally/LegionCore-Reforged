-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_152_split_emerald_nightmare_aura_scripts.sql (2026-11-09).
UPDATE `spell_script_names`
SET `ScriptName` = 'spell_nythendra_rot_dot'
WHERE `spell_id` = 203096
  AND `ScriptName` = 'spell_nythendra_volatile_rot';

INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (215300, 'spell_elerethe_web_of_pain_periodic');
