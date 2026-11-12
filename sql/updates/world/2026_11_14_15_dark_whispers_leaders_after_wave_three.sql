-- Dark Whispers (42125, scenario 1097), stage 4 "The Legion": Lady Tyrana (106825) and Pain Mistress Nikta (106827)
-- did not appear after the third wave (owner's report, 2026-11-12).
--
-- The wave controller (Wild Imps 99739) summoned them on data 3 3, which a wave creature sent only when it died while
-- the two other wave-3 creature types were alive within 50 yards of its KILLER. A ranged killer or a pet away from the
-- others, or a first kill made after another type was already dead, never sent it, and nothing sent it later: the
-- leaders never came. When it did fire, it fired on the first wave-3 kill, not after the wave as on retail.
--
-- Now the leaders come with the last death of wave 3: the same "wave over" message (data 1 1 from a Soul Binder,
-- data 2 2 from a Hellblade or Fel Mongrel) that already counts the wave for the scenario. The controller enters
-- event phase 1 one second after summoning wave 3 (a phase set inside the same data event would let the end-of-wave-2
-- message itself summon the leaders), takes the first end-of-wave message in phase 1, then goes to phase 2.
-- The wave death checks now look around the dying creature, not its killer (as LegionCore-7.3.5 did for the leaders).

-- 1. The "3 3" senders and their conditions go.
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND ((`entryorguid`=106657 AND `id`=6) OR (`entryorguid`=106660 AND `id`=5) OR (`entryorguid`=106687 AND `id`=6));
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceId`=0 AND ((`SourceEntry`=106657 AND `SourceGroup`=7) OR (`SourceEntry`=106660 AND `SourceGroup`=6) OR (`SourceEntry`=106687 AND `SourceGroup`=7));

-- 2. "Is anyone of my wave still alive?" is asked around the dying creature (ConditionTarget 1), not its killer.
UPDATE `conditions` SET `ConditionTarget`=1
WHERE `SourceTypeOrReferenceId`=22 AND `SourceId`=0 AND `ConditionTypeOrReference`=29 AND `NegativeCondition`=1
  AND ((`SourceEntry`=106646 AND `SourceGroup`=5) OR (`SourceEntry`=106657 AND `SourceGroup`=5)
    OR (`SourceEntry`=106660 AND `SourceGroup`=4) OR (`SourceEntry`=106687 AND `SourceGroup`=5));

-- 3. Controller: phase 1 one second after wave 3 is summoned; the leaders on the next end-of-wave message; phase 2.
UPDATE `smart_scripts` SET `link`=15 WHERE `entryorguid`=99739 AND `source_type`=0 AND `id`=14;
UPDATE `smart_scripts` SET `event_type`=38, `event_phase_mask`=1, `event_flags`=1, `event_param1`=1, `event_param2`=1,
  `comment`='Dark Whispers - wave 3 over (Soul Binder last) - Summon Lady Tyrana'
WHERE `entryorguid`=99739 AND `source_type`=0 AND `id`=12;
UPDATE `smart_scripts` SET `link`=17 WHERE `entryorguid`=99739 AND `source_type`=0 AND `id`=13;
DELETE FROM `smart_scripts` WHERE `entryorguid`=99739 AND `source_type`=0 AND `id` BETWEEN 15 AND 19;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,
  `event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,
  `action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,
  `target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(99739,0,15,0,61,0,100,1, 0,0,0,0, 67,1,1000,1000,0,0,100, 1,0,0,0, 0,0,0,0,'Dark Whispers - wave 3 summoned - timed event 1 in 1 s'),
(99739,0,16,0,59,0,100,1, 1,0,0,0, 22,1,0,0,0,0,0, 1,0,0,0, 0,0,0,0,'Dark Whispers - wave 3 under way - set event phase 1'),
(99739,0,17,0,61,0,100,1, 0,0,0,0, 22,2,0,0,0,0,0, 1,0,0,0, 0,0,0,0,'Dark Whispers - leaders summoned - set event phase 2'),
(99739,0,18,19,38,1,100,1, 2,2,0,0, 12,106825,8,0,0,0,0, 8,0,0,0, 0,0,0,0,'Dark Whispers - wave 3 over (Hellblade / Mongrel last) - Summon Lady Tyrana'),
(99739,0,19,17,61,0,100,1, 0,0,0,0, 12,106827,8,0,0,0,0, 8,0,0,0, 0,0,0,0,'Dark Whispers - Link - Summon Pain Mistress Nikta');
