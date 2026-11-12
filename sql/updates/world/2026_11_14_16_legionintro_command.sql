-- .legionintro: GM test helper, skips the Legion introduction and goes to Dalaran
DELETE FROM `command` WHERE `name` = 'legionintro';
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('legionintro', 2, 'Syntax: .legionintro [$playername]\n\nFor a character of level 98 or more (the selected one, or yourself): mark his faction''s Legion introduction quests done (The Legion Returns to Illidari Allies / Keep Your Friends Close, plus the demon hunter route), turn in "In the Blink of an Eye" and teleport him to Dalaran on the Broken Isles. For testing the artifact and class hall quests.');
