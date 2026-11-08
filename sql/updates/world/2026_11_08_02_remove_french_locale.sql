-- No French text for an enUS realm (owner, 2026-11-08: "get rid of all the French text and switch it to English").
-- Upstream Reforged is maintained in French and keeps frFR translations; this realm only serves enUS clients, so the
-- frFR rows go. English (the base columns) is untouched. A later upstream update may add French rows again: they are
-- only ever sent to a frFR client. Re-runnable.

DELETE FROM `battlepay_display_info_locales` WHERE `Locale` = 'frFR';
DELETE FROM `battlepay_product_group_locales` WHERE `Locale` = 'frFR';
DELETE FROM `creature_template_wdb_locale` WHERE `Locale` = 'frFR';
DELETE FROM `creature_text_locale` WHERE `Locale` = 'frFR';
DELETE FROM `gossip_menu_option_locale` WHERE `Locale` = 'frFR';
DELETE FROM `page_text_locale` WHERE `Locale` = 'frFR';
DELETE FROM `playerchoice_locale` WHERE `locale` = 'frFR';
DELETE FROM `playerchoice_response_locale` WHERE `locale` = 'frFR';
DELETE FROM `quest_objectives_locale` WHERE `locale` = 'frFR';
DELETE FROM `quest_offer_reward_locale` WHERE `Locale` = 'frFR';
DELETE FROM `quest_request_items_locale` WHERE `Locale` = 'frFR';
DELETE FROM `quest_template_locale` WHERE `locale` = 'frFR';
UPDATE `custom_texts` SET `content_loc2` = '' WHERE `content_loc2` <> '';
UPDATE `db_script_string` SET `content_loc2` = '' WHERE `content_loc2` <> '';
UPDATE `locales_achievement_reward` SET `subject_loc2` = '', `text_loc2` = '' WHERE `subject_loc2` <> '' OR `text_loc2` <> '';
UPDATE `locales_gameobject` SET `name_loc2` = '', `castbarcaption_loc2` = '' WHERE `name_loc2` <> '' OR `castbarcaption_loc2` <> '';
UPDATE `locales_page_text` SET `Text_loc2` = '' WHERE `Text_loc2` <> '';
UPDATE `locales_points_of_interest` SET `icon_name_loc2` = '' WHERE `icon_name_loc2` <> '';
UPDATE `script_texts` SET `content_loc2` = '' WHERE `content_loc2` <> '';
UPDATE `trinity_string` SET `content_loc2` = '' WHERE `content_loc2` <> '';
