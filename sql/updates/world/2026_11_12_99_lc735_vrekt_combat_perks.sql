-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_18_12_vrekt_combat_perks.sql (2026-11-09).
-- Player artifact perks imported into the Vrekt automatic creature spell pool:
-- Dreadblades213738 and Maw of the Damned215377 are not NPC abilities.
-- Keep the existing AggressorAI path and every other spell/difficulty row.
DELETE FROM `creature_template_spell`
WHERE `entry` IN(115846,116180,116600,116601)
  AND `spell` IN(213738,215377);
