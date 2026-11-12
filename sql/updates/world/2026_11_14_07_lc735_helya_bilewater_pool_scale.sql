-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_19_01_helya_bilewater_pool_scale.sql (2026-11-09).
-- 227992 already receives the slime's health percent in effect1. Apply it to
-- both the collision radius and client scale, scoped to Bilewater Corrosion.
INSERT INTO areatrigger_scripts (entry, ScriptName)
VALUES (12892, 'at_bilewater_corrosion')
ON DUPLICATE KEY UPDATE ScriptName = VALUES(ScriptName);
