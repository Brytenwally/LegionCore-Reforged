-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_21_249_restore_warrior_t21.sql (2026-11-09).
-- Restore the scripted effects for the Antorus warrior Tier 21 bonuses.
DELETE FROM `spell_script_names`
WHERE (`spell_id` = 251880 AND `ScriptName` = 'spell_warr_t21_fury_2p')
   OR (`spell_id` = 251883 AND `ScriptName` = 'spell_warr_t21_prot_2p');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(251880, 'spell_warr_t21_fury_2p'),
(251883, 'spell_warr_t21_prot_2p');
