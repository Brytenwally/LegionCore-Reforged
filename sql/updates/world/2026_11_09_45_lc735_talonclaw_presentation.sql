-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_23_11_talonclaw_presentation.sql (2026-11-09).
INSERT INTO spell_script_names (spell_id,ScriptName)
SELECT 205589,'spell_talonclaw_presentation'
WHERE NOT EXISTS (SELECT 1 FROM spell_script_names WHERE spell_id=205589 AND ScriptName='spell_talonclaw_presentation');
