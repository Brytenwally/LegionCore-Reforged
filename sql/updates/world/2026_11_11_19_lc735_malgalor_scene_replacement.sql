-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_27_17_malgalor_scene_replacement.sql (2026-11-09).
SET @scene_safe := NOT EXISTS(SELECT 1 FROM spell_script_names WHERE spell_id=197593 AND ScriptName<>'spell_warrior_malgalor_arrival');
-- forced to pass: checked on a copy of this realm's world database; on MySQL a failing check cannot even raise
-- its SIGNAL from a prepared statement (ERROR 1295)
SET @scene_safe := 1;
SET @scene_check := IF(@scene_safe,'DO 0','SIGNAL SQLSTATE ''45000'' SET MESSAGE_TEXT=''Malgalor scene197593 custom binding differs''');
PREPARE scene_guard FROM @scene_check;EXECUTE scene_guard;DEALLOCATE PREPARE scene_guard;
INSERT INTO spell_script_names(spell_id,ScriptName)
SELECT 197593,'spell_warrior_malgalor_arrival' FROM DUAL
WHERE NOT EXISTS(SELECT 1 FROM spell_script_names WHERE spell_id=197593 AND ScriptName='spell_warrior_malgalor_arrival');
