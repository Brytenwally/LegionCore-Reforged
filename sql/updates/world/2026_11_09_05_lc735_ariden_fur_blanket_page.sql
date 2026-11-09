-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_20_01_ariden_fur_blanket_page.sql (2026-11-09).
-- The optional Fur Blanket inspection is a text object, not quest credit.
-- Restore its missing page without replacing an existing/localized page.
INSERT INTO page_text (ID, Text, NextPageID, PlayerConditionID, Flags, VerifiedBuild)
SELECT 5234, 'The bedding in the tent has been undisturbed for quite some time.', 0, 0, 0, 0
WHERE NOT EXISTS (SELECT 1 FROM page_text WHERE ID=5234);
