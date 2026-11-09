-- From Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_30_03_acquisition_combat_speakers.sql (2026-11-09).
-- Varedis Felsoul (94836, Havoc artifact scenario) and Allaris Narassin (110556) said their health and combat lines
-- with the victim as target (target_type 2): when the victim is a creature (a pet), that creature spoke them. They
-- speak themselves now. Its hash guard is left out: on this realm the guarded tables also hold changes of
-- LegionCore-7.3.5 files not ported yet, so the rows are matched by their current values instead.
UPDATE `smart_scripts` SET `target_type`=1
WHERE `source_type`=0 AND `action_type`=1 AND `target_type`=2
  AND ((`entryorguid`=94836 AND `id` IN (4,5,10)) OR (`entryorguid`=110556 AND `id` IN (5,6,7)));
