-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_181_restore_hunter_survival_fluffy_go.sql (2026-11-09).
-- Keep Fluffy, Go synchronized on an active pet when its artifact rank changes.
DELETE FROM `spell_script_names`
WHERE `spell_id` = 203669 AND `ScriptName` = 'spell_hun_fluffy_go';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(203669, 'spell_hun_fluffy_go');
