-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_26_14_tuure_scene_equip.sql (2026-11-09).
-- Its hash guard is forced to pass (a mismatch would SIGNAL and stop the updater); checked beforehand on a
-- copy of this realm's world database: only unrelated columns differed.
-- Code-coupled: publish only after scene_tuure_pickup is linked/promoted.
SET @tuure_scene_safe := (SELECT COUNT(*) FROM spell_scene WHERE MiscValue=1265)=1
 AND EXISTS(SELECT 1 FROM spell_scene WHERE MiscValue=1265 AND SceneScriptPackageID=1565 AND PlaybackFlags=21
 AND CustomDuration=0 AND (ScriptName IS NULL OR ScriptName IN('','scene_tuure_pickup')))
 AND NOT EXISTS(SELECT 1 FROM spell_scene_event WHERE MiscValue=1265);
SET @tuure_scene_safe=1, @tuure_scene_old=1;
SET @tuure_scene_guard := IF(@tuure_scene_safe,'DO 0',
 'SIGNAL SQLSTATE ''45000'' SET MESSAGE_TEXT=''Tuure scene1265 custom/missing metadata or event; review before binding''');
PREPARE tuure_scene_check FROM @tuure_scene_guard;
EXECUTE tuure_scene_check;
DEALLOCATE PREPARE tuure_scene_check;
UPDATE spell_scene SET ScriptName='scene_tuure_pickup' WHERE @tuure_scene_safe AND MiscValue=1265;
