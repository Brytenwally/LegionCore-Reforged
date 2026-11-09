-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_25_06_magnar_intro_scene.sql (2026-11-09).
INSERT INTO spell_scene (SceneScriptPackageID, MiscValue, PlaybackFlags, CustomDuration, ScriptName, comment)
SELECT 1458,1059,16,0,'','189721 Magnar reveal native package'
WHERE NOT EXISTS (SELECT 1 FROM spell_scene WHERE MiscValue=1059 OR SceneScriptPackageID=1458)
  AND NOT EXISTS (SELECT 1 FROM spell_scene_event WHERE MiscValue=1059);
