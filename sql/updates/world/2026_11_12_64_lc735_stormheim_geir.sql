-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_16_03_stormheim_geir.sql (2026-11-09).
-- Geir39853: preserve SmartAI combat/death conversation2256. The derived AI
-- supplies the eligible pet owner's Player invoker to that native death event.
UPDATE creature_template SET AIName='SmartAI',ScriptName='npc_stormheim_geir'
WHERE entry=91948 AND AIName IN('','SmartAI') AND ScriptName IN('','npc_stormheim_geir');
