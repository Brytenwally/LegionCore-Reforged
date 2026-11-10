-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_18_06_rockaway_channel.sql (2026-11-09).
-- Reconstructed Kvaldir beam; original Rockaway cast binding is unproven.
-- Script scope is caster115132/map1220 and its admitted summoner GUID.
-- Preserve native target38 and all shared spell conditions/client data.
DELETE FROM spell_script_names WHERE spell_id=231433 AND ScriptName='spell_rockaway_channel';
INSERT INTO spell_script_names (spell_id,ScriptName) VALUES (231433,'spell_rockaway_channel');
