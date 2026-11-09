-- Game.Patch is gone (owner, 2026-11-09: the server is always the full 7.3.5 game). The content scripts that opened and
-- closed each tier's content at startup are removed with it, so a tier a realm had closed would stay closed for good:
-- each tier's former enable script runs here, but only for a tier still marked closed (a `disables` row of its disable
-- script, comment "<tier> content - ..."). On a realm that always ran the full game this file changes nothing.

SET @closed_7_1 = EXISTS (SELECT 1 FROM disables WHERE comment LIKE '7.1 content - %');
SET @closed_7_1_5 = EXISTS (SELECT 1 FROM disables WHERE comment LIKE '7.1.5 content - %');
SET @closed_7_2 = EXISTS (SELECT 1 FROM disables WHERE comment LIKE '7.2 content - %');
SET @closed_7_3 = EXISTS (SELECT 1 FROM disables WHERE comment LIKE '7.3 content - %');

-- 7.1
DELETE FROM disables WHERE sourceType = 2 AND entry IN (1651) AND @closed_7_1;
DELETE FROM disables WHERE sourceType = 1 AND entry IN (45271, 45260, 45422, 44720) AND @closed_7_1;

-- 7.1.5
DELETE FROM disables WHERE sourceType = 1 AND entry IN (46119) AND @closed_7_1_5;
UPDATE game_event SET start_time = '2019-02-23 19:00:00' WHERE eventEntry = 69 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2019-04-28 21:00:00' WHERE eventEntry = 70 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2019-01-21 09:01:00' WHERE eventEntry = 78 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2019-01-31 19:00:00' WHERE eventEntry = 86 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2018-05-10 09:00:00' WHERE eventEntry = 87 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2018-05-09 11:00:00' WHERE eventEntry = 98 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2018-05-27 21:00:00' WHERE eventEntry = 301 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2019-04-05 09:01:00' WHERE eventEntry = 305 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2019-01-24 09:00:00' WHERE eventEntry = 306 AND @closed_7_1_5;
UPDATE game_event SET start_time = '2019-06-06 09:01:00' WHERE eventEntry = 307 AND @closed_7_1_5;

-- 7.2
DELETE FROM disables WHERE sourceType = 2 AND entry IN (1677) AND @closed_7_2;
DELETE FROM disables WHERE sourceType = 1 AND entry IN (46730, 48641) AND @closed_7_2;
UPDATE creature SET spawnMask = 1 WHERE zoneId = 7543 AND areaId != 8143 AND @closed_7_2;
UPDATE gameobject SET spawnMask = 1 WHERE zoneId = 7543 AND areaId != 8143 AND @closed_7_2;
UPDATE creature SET spawnMask = 1 WHERE id IN (120687, 120906) AND @closed_7_2;

-- 7.3
DELETE FROM disables WHERE sourceType = 2 AND entry IN (1669, 1753) AND @closed_7_3;
DELETE FROM disables WHERE sourceType = 1 AND entry IN (47221, 47835, 48506, 48507, 47867, 47222, 47223, 48639, 48642) AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 09:57:00' WHERE eventEntry = 180 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 11:57:00' WHERE eventEntry = 181 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-07 01:57:00' WHERE eventEntry = 182 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-07 03:57:00' WHERE eventEntry = 183 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 17:57:00' WHERE eventEntry = 184 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 23:57:00' WHERE eventEntry = 185 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 21:57:00' WHERE eventEntry = 186 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 19:57:00' WHERE eventEntry = 187 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 15:57:00' WHERE eventEntry = 188 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-06 13:57:00' WHERE eventEntry = 189 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-07 05:57:00' WHERE eventEntry = 190 AND @closed_7_3;
UPDATE game_event SET start_time = '2018-08-07 07:57:00' WHERE eventEntry = 191 AND @closed_7_3;
UPDATE world_quest_template SET MinItemLevel = 890 WHERE ZoneId = 0 AND QuestInfoId IN (113, 137) AND @closed_7_3;
