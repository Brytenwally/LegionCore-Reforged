-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_14_12_stormheim_thorignir_handoff.sql (2026-11-09).
-- Both faction-specific Thorignir handoffs follow the shared Watchers blessing.
-- Horde's old Seal Your Fate prerequisite exposed this handoff before the trials.
-- Keep the native race masks, Havi/Lorna/Cullen relations and empty breadcrumb objectives.
UPDATE quest_template_addon SET PrevQuestID=39597
WHERE ID=38611 AND PrevQuestID=38459 AND NextQuestID=38612;
