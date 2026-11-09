-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_26_11_warrior_creation_actions.sql (2026-11-09).
-- Spell78 and88161 are absent from matching7.3.5.26972 Spell.db2/hotfix data.
-- Do not replace them with an unlearned ability or alter saved character bars.
DELETE FROM playercreateinfo_action
WHERE class=1 AND type=0
AND ((button=1 AND action=78) OR (button=73 AND action=88161));
