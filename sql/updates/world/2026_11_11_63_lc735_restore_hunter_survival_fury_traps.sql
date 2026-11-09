-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_19_179_restore_hunter_survival_fury_traps.sql (2026-11-09).
-- Restore Fury of the Eagle's snapshotted Mongoose Fury scaling and the
-- missing Expert Trapper effects for Freezing Trap and Tar Trap.
DELETE FROM `spell_script_names`
WHERE (`spell_id` = 203415 AND `ScriptName` = 'spell_hun_fury_of_the_eagle')
   OR (`spell_id` = 203413 AND `ScriptName` = 'spell_hun_fury_of_the_eagle_damage')
   OR (`spell_id` = 3355 AND `ScriptName` = 'spell_hun_freezing_trap_expert')
   OR (`spell_id` = 135299 AND `ScriptName` = 'spell_hun_tar_trap_aura');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(203415, 'spell_hun_fury_of_the_eagle'),
(203413, 'spell_hun_fury_of_the_eagle_damage'),
(3355,   'spell_hun_freezing_trap_expert'),
(135299, 'spell_hun_tar_trap_aura');
