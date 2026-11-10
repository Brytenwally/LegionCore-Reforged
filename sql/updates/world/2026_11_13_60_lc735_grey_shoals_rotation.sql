-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_18_08_grey_shoals_rotation.sql (2026-11-09).
-- Preserve the native SmartAI dispatcher/events while adding the outdoor
-- rotation lifecycle. The C++ factory excludes summons and other maps/areas.
UPDATE `creature_template`
SET `ScriptName`='npc_grey_shoals'
WHERE `entry` IN(115025,115026,115031,115751)
  AND `AIName`='SmartAI' AND `ScriptName` IN('','npc_grey_shoals');
