-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_12_15_dalaran_pad_single_trigger.sql (2026-11-09).
-- Both visual pads already run go_eye_portrait_teleporter, which preserves
-- quest arrival credit and ignores pending teleports. The overlapping legacy
-- event objects bypass those checks and use combat-reach-expanded proximity.
-- Keep one activation path; preserve the native spells and arrival coordinates.
UPDATE `eventobject_template` e
JOIN `gameobject_template` g ON g.`entry`=IF(e.`entry`=4,244534,244560)
SET e.`radius`=0, e.`SpellID`=0
WHERE ((e.`entry`=4 AND e.`SpellID`=192293) OR (e.`entry`=5 AND e.`SpellID`=192295))
  AND e.`radius`=1 AND e.`WorldSafeLocID`=0 AND e.`Flags`=0 AND e.`ScriptName`=''
  AND g.`ScriptName`='go_eye_portrait_teleporter';
