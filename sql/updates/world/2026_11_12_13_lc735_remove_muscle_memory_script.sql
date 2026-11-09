-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_09_21_260_remove_muscle_memory_script.sql (2026-11-09).
-- Muscle Memory was removed in patch 6.0.2. Its proc and passive spells are
-- absent from the 7.3.5 client data, so do not bind its legacy handler.
DELETE FROM `spell_script_names`
WHERE `ScriptName` = 'spell_monk_clone_cast'
  AND `spell_id` IN (100780, 101546, 116847);
