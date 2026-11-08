-- English for the French left in English columns (owner, 2026-11-08: an enUS client must see no French).
-- Upstream writes French; most of it rightly goes to the frFR locale rows, but these few rows held French in
-- the base (English) column itself: shop entries, a currency name, a custom quest, a few NPC lines without a
-- broadcast text, a gossip option and a script text. The calendar's server-locale columns get the English too.

UPDATE `battlepay_display_info` SET `Name1` = 'Winged Guardian', `Name3` = 'This majestic lion will carry your enemies away in a whirlwind of feathers and claws.' WHERE `DisplayInfoId` = 1;
UPDATE `battlepay_display_info` SET `Name1` = 'Celestial Steed', `Name3` = 'An ethereal mount from the stars. Flies and swims.' WHERE `DisplayInfoId` = 2;
UPDATE `battlepay_display_info` SET `Name1` = 'Heart of the Aspects', `Name3` = 'A golden drake blessed by the Dragon Aspects.' WHERE `DisplayInfoId` = 3;
UPDATE `battlepay_display_info` SET `Name1` = 'Swift Windsteed', `Name3` = 'A graceful mount from Pandaria.' WHERE `DisplayInfoId` = 4;
UPDATE `battlepay_display_info` SET `Name1` = 'Armored Bloodwing', `Name3` = 'A war bat clad in plate armor.' WHERE `DisplayInfoId` = 5;
UPDATE `battlepay_display_info` SET `Name1` = 'Imperial Quilen', `Name3` = 'An ancient quilen with wings of lightning.' WHERE `DisplayInfoId` = 6;
UPDATE `battlepay_display_info` SET `Name1` = 'Enchanted Fey Dragon', `Name3` = 'A whimsical, colorful dragon from the enchanted woods.' WHERE `DisplayInfoId` = 7;
UPDATE `battlepay_display_info` SET `Name1` = 'Iron Skyreaver', `Name3` = 'A mechanical chimaera forged by the Blackrock smiths.' WHERE `DisplayInfoId` = 8;
UPDATE `battlepay_display_info` SET `Name1` = 'Grinning Reaver', `Name3` = 'A nightmarish mount with an eternal grin.' WHERE `DisplayInfoId` = 9;
UPDATE `battlepay_display_info` SET `Name1` = 'Forged Nightmare', `Name3` = 'A steed of flame and steel forged in darkness.' WHERE `DisplayInfoId` = 10;
UPDATE `battlepay_display_info` SET `Name1` = 'Mystic Runesaber', `Name3` = 'A runic cat of the night.' WHERE `DisplayInfoId` = 11;
UPDATE `battlepay_display_info` SET `Name1` = 'Luminous Starseeker', `Name3` = 'An astral creature that lights up the skies.' WHERE `DisplayInfoId` = 12;
UPDATE `battlepay_display_info` SET `Name1` = 'Lil\' K.T.', `Name3` = 'A miniature version of the dreaded Kel\'Thuzad.' WHERE `DisplayInfoId` = 21;
UPDATE `battlepay_display_info` SET `Name1` = 'Pandaren Monk', `Name3` = 'A little pandaren who meditates at your side.' WHERE `DisplayInfoId` = 22;
UPDATE `battlepay_display_info` SET `Name1` = 'Lil\' XT', `Name3` = 'The heart of the Deconstructor, pocket-sized.' WHERE `DisplayInfoId` = 23;
UPDATE `battlepay_display_info` SET `Name1` = 'Lil\' Ragnaros', `Name3` = 'The Firelord, adorable edition.' WHERE `DisplayInfoId` = 25;
UPDATE `battlepay_display_info` SET `Name1` = 'Soul of the Aspects', `Name3` = 'A small dragon born from the essence of the five Aspects.' WHERE `DisplayInfoId` = 26;
UPDATE `battlepay_display_info` SET `Name1` = 'Cinder Kitten', `Name3` = 'An elemental kitten of fire.' WHERE `DisplayInfoId` = 27;
UPDATE `battlepay_display_info` SET `Name1` = 'Lucky Quilen Cub', `Name3` = 'A baby quilen that brings good luck.' WHERE `DisplayInfoId` = 28;
UPDATE `battlepay_display_info` SET `Name1` = 'Blossoming Ancient', `Name3` = 'A miniature treant in everlasting bloom.' WHERE `DisplayInfoId` = 29;
UPDATE `battlepay_display_info` SET `Name1` = 'Alterac Brew-Pup', `Name3` = 'An adorable Alterac brewery puppy.' WHERE `DisplayInfoId` = 30;
UPDATE `battlepay_display_info` SET `Name1` = 'Argi', `Name3` = 'A baby talbuk from the Exodar.' WHERE `DisplayInfoId` = 31;
UPDATE `battlepay_display_info` SET `Name1` = 'Shimmerpaw', `Name3` = 'A magical cat with glowing paws.' WHERE `DisplayInfoId` = 32;
UPDATE `battlepay_display_info` SET `Name1` = 'Snaggletooth', `Name3` = 'A crocolisk that only has eyes for you.' WHERE `DisplayInfoId` = 33;
UPDATE `battlepay_display_info` SET `Name1` = 'Shadow', `Name3` = 'A fox of darkness, loyal and mysterious.' WHERE `DisplayInfoId` = 34;
UPDATE `battlepay_display_info` SET `Name1` = 'Twilight', `Name3` = 'A young twilight dragon.' WHERE `DisplayInfoId` = 35;
UPDATE `battlepay_display_info` SET `Name1` = 'Nightmarish Hitching Post', `Name3` = 'Summons a hitching post that turns allied mounts into flaming nightmares for 20 min.' WHERE `DisplayInfoId` = 41;
UPDATE `battlepay_display_info` SET `Name1` = 'Crown of Eternal Winter', `Name3` = 'A helm of eternal ice.' WHERE `DisplayInfoId` = 51;
UPDATE `battlepay_display_info` SET `Name1` = 'Hood of Devouring Darkness', `Name3` = 'A mask of living darkness.' WHERE `DisplayInfoId` = 52;
UPDATE `battlepay_display_info` SET `Name1` = 'Jewel of the Firelord', `Name3` = 'Ragnaros\' flaming crown.' WHERE `DisplayInfoId` = 53;
UPDATE `battlepay_display_info` SET `Name1` = 'Level 100 Character Boost' WHERE `DisplayInfoId` = 110;
UPDATE `battlepay_display_info` SET `Name1` = 'Character Restoration' WHERE `DisplayInfoId` = 200;
UPDATE `battlepay_tokens` SET `name` = 'Credits' WHERE `tokenType` = 1 AND `name` = 'Crédits';
UPDATE `quest_template` SET `LogTitle` = 'Order Archives', `LogDescription` = 'Speak to your order\'s archivist to consult the research your companions have already done.', `QuestDescription` = 'The research of your order\'s other champions has not been lost. Their notes, their attempts and their mistakes are recorded here. Take the time to study them: in a few days you will catch up on what took others months.', `QuestCompletionLog` = 'Study your order\'s archives.' WHERE `ID` = 316999;
UPDATE `creature_text` SET `Text` = 'Here we go, $N, you and your group: let the fight begin!' WHERE `CreatureID` = 56056 AND `GroupID` = 0 AND `ID` = 0 AND `Text` LIKE 'C\'est parti%';
UPDATE `creature_text` SET `Text` = 'Tremble, tremble beneath this quake!' WHERE `CreatureID` = 57106 AND `GroupID` = 1 AND `ID` = 0 AND `Text` LIKE 'Tremblez%';
UPDATE `creature_text` SET `Text` = 'I told you that you would not survive...' WHERE `CreatureID` = 57106 AND `GroupID` = 3 AND `ID` = 0 AND `Text` LIKE 'Je vous%';
UPDATE `gossip_menu_option` SET `OptionText` = 'Help us with the Ball-O-Tron 800!' WHERE `MenuID` = 44202 AND `OptionID` = 0 AND `OptionText` LIKE 'Aldez-nous%';
UPDATE `script_texts` SET `content_default` = 'Queue for Wintergrasp.' WHERE `entry` = -1850507 AND `content_default` LIKE 'Se mettre%';
UPDATE `custom_calendar_event` SET `NameLocalized` = `Name`, `DescriptionLocalized` = `Description`;
