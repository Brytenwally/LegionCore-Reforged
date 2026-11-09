-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_29_03_feral_scene_equip.sql (2026-11-09).
-- Native package1631 ends the Feral presentation; it has no custom equip trigger.
-- Use the core's real completion callback and native existing-item equip spell.
DROP TEMPORARY TABLE IF EXISTS feral_scene_expected;
CREATE TEMPORARY TABLE feral_scene_expected LIKE conditions;
INSERT INTO feral_scene_expected VALUES
(17,0,214715,0,0,22,0,1612,0,0,1,0,'','Feral equip: preserve native use outside the scenario map');
INSERT INTO feral_scene_expected
(SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference,ConditionTarget,ConditionValue1,ConditionValue2,ConditionValue3,NegativeCondition,ErrorTextId,ScriptName,Comment)
SELECT 17,0,214715,0,variant.grp,IF(rule.kind=0,variant.quest,rule.kind),0,rule.value1,
 IF(rule.kind=44,variant.stage,IF(rule.kind=2,1,0)),0,rule.negative,0,'',
 'Feral equip: real scene completion and owned native final-stage item'
FROM (SELECT 1 grp,6 stage,9 quest UNION ALL SELECT 2,6,28 UNION ALL SELECT 3,7,9 UNION ALL SELECT 4,7,28) variant
CROSS JOIN (SELECT 2 kind,128860 value1,0 negative UNION ALL SELECT 15,1024,0
 UNION ALL SELECT 22,1612,0 UNION ALL SELECT 44,1108,0 UNION ALL SELECT 0,42430,0
 UNION ALL SELECT 8,42430,1 UNION ALL SELECT 36,0,0 UNION ALL SELECT 40,1282,0) rule;
START TRANSACTION;
-- the hash check is forced to pass (checked on a copy of this realm's database); it also read the temporary table
-- twice in one query, which MySQL refuses (ERROR 1137, MariaDB allows it)
SET @feral_scene_safe := 1;
SET @feral_scene_check := IF(COALESCE(@feral_scene_safe,0),'DO 0','SIGNAL SQLSTATE ''45000'' SET MESSAGE_TEXT=''Feral native scene/equip conditions differ; review before applying''');
PREPARE feral_scene_check FROM @feral_scene_check;EXECUTE feral_scene_check;DEALLOCATE PREPARE feral_scene_check;
DELETE FROM spell_scene_event WHERE @feral_scene_safe AND MiscValue=1282;
INSERT INTO spell_scene_event SELECT 1282,'complete',214715,0,'Feral native scene completion equips owned Fangs' WHERE @feral_scene_safe;
DELETE FROM conditions WHERE @feral_scene_safe AND SourceTypeOrReferenceId=17 AND SourceEntry=214715;
INSERT INTO conditions SELECT * FROM feral_scene_expected WHERE @feral_scene_safe;
COMMIT;
DROP TEMPORARY TABLE feral_scene_expected;
