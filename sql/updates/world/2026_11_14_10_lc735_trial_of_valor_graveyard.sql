-- Ported from Brytenwally/LegionCore-7.3.5 sql/updates/world/2026_10_20_13_trial_of_valor_graveyard.sql (2026-11-09).
-- Native WorldSafeLocs5779: Helheim Raid - Exit Target - MRN (Stormheim).
-- Already a neutral Stormheim graveyard link; also serve the raid's external
-- repop path during an encounter. Internal checkpoints remain instance-owned.
INSERT IGNORE INTO `game_graveyard_zone` (`id`, `ghost_zone`, `faction`) VALUES
(5779, 8440, 0);
