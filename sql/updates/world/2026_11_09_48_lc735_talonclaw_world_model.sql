-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_23_14_talonclaw_world_model.sql (2026-11-09).
UPDATE creature_template AS actor
JOIN creature_template_wdb AS native ON native.Entry=actor.entry
SET actor.flags_extra=actor.flags_extra | 128
WHERE actor.entry=100310 AND actor.flags_extra=0
  AND actor.AIName='' AND actor.ScriptName=''
  AND native.Displayid1=33225 AND native.Displayid2=66609
  AND native.Displayid3=0 AND native.Displayid4=0;
