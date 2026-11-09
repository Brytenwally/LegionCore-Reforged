-- Scenario checkpoint across logouts (Player::RecordScenarioEntrance / PrepareScenarioLogin, PlayerScenario.cpp).
-- From LegionCore-7.3.5's 2026_10_15_00_scenario_resume.sql (MariaDB's ADD COLUMN IF NOT EXISTS is not MySQL 8 syntax).
ALTER TABLE `characters` ADD COLUMN `scenarioResume` VARCHAR(255) NOT NULL DEFAULT '';
