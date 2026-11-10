-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_11_10_highmountain_stonedark_crystal.sql (2026-11-09).
-- TDB735 25549: restore the second objective flags for Stonedark Focus.
-- The item count and loot chances are already valid; do not invent new drops.
UPDATE quest_objectives SET Flags2=1
WHERE ID=279391 AND QuestID=39425 AND Type=1 AND ObjectID=128343;
