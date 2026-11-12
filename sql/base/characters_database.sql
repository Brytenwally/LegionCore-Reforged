/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
--
-- ------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;

--
-- Table structure for table `account_achievement`
--

DROP TABLE IF EXISTS `account_achievement`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_achievement` (
  `account` int unsigned NOT NULL,
  `first_guid` int unsigned NOT NULL,
  `achievement` int unsigned NOT NULL,
  `date` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`achievement`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_achievement_progress`
--

DROP TABLE IF EXISTS `account_achievement_progress`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_achievement_progress` (
  `account` int unsigned NOT NULL,
  `criteria` int unsigned NOT NULL,
  `counter` int unsigned NOT NULL,
  `date` bigint unsigned NOT NULL DEFAULT '0',
  `achievID` int unsigned NOT NULL DEFAULT '0',
  `completed` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`criteria`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `criteria` (`criteria`) USING BTREE,
  KEY `achievID` (`achievID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_ban_attempts`
--

DROP TABLE IF EXISTS `account_ban_attempts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_ban_attempts` (
  `accountId` int unsigned NOT NULL DEFAULT '0',
  `banAttempts` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accountId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_battlepet`
--

DROP TABLE IF EXISTS `account_battlepet`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_battlepet` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `account` int unsigned NOT NULL DEFAULT '0',
  `slot` int NOT NULL DEFAULT '-1',
  `name` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `nameTimeStamp` int unsigned NOT NULL DEFAULT '0',
  `species` int unsigned NOT NULL DEFAULT '0',
  `quality` int unsigned NOT NULL DEFAULT '0',
  `breed` int unsigned NOT NULL DEFAULT '0',
  `level` int unsigned NOT NULL DEFAULT '0',
  `xp` int unsigned NOT NULL DEFAULT '0',
  `display` int unsigned NOT NULL DEFAULT '0',
  `health` int NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  `infoPower` int NOT NULL DEFAULT '0',
  `infoMaxHealth` int NOT NULL DEFAULT '0',
  `infoSpeed` int NOT NULL DEFAULT '0',
  `infoGender` int NOT NULL DEFAULT '0',
  `declinedGenitive` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `declinedNative` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `declinedAccusative` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `declinedInstrumental` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `declinedPrepositional` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_bot_detected`
--

DROP TABLE IF EXISTS `account_bot_detected`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_bot_detected` (
  `accountId` int unsigned NOT NULL,
  `playerName` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `botName` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`accountId`,`playerName`,`botName`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_data`
--

DROP TABLE IF EXISTS `account_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_data` (
  `accountId` int unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `type` tinyint unsigned NOT NULL DEFAULT '0',
  `time` int unsigned NOT NULL DEFAULT '0',
  `data` blob NOT NULL,
  PRIMARY KEY (`accountId`,`type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_flagged`
--

DROP TABLE IF EXISTS `account_flagged`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_flagged` (
  `id` int unsigned NOT NULL DEFAULT '0' COMMENT 'Account Id',
  `banduration` int unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `banreason` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_heirlooms`
--

DROP TABLE IF EXISTS `account_heirlooms`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_heirlooms` (
  `accountId` int unsigned NOT NULL,
  `itemId` int unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accountId`,`itemId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_item_favorite_appearances`
--

DROP TABLE IF EXISTS `account_item_favorite_appearances`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_item_favorite_appearances` (
  `battlenetAccountId` int unsigned NOT NULL,
  `itemModifiedAppearanceId` int unsigned NOT NULL,
  PRIMARY KEY (`battlenetAccountId`,`itemModifiedAppearanceId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_mounts`
--

DROP TABLE IF EXISTS `account_mounts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_mounts` (
  `account` int unsigned NOT NULL,
  `spell` int unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`spell`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_progress`
--

DROP TABLE IF EXISTS `account_progress`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_progress` (
  `account` int unsigned NOT NULL,
  `totaltime` int unsigned NOT NULL DEFAULT '0',
  `leveltime` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_toys`
--

DROP TABLE IF EXISTS `account_toys`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_toys` (
  `accountId` int unsigned NOT NULL,
  `itemId` int NOT NULL DEFAULT '0',
  `isFavourite` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`accountId`,`itemId`) USING BTREE,
  KEY `accountId` (`accountId`) USING BTREE,
  KEY `itemId` (`itemId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_transmogs`
--

DROP TABLE IF EXISTS `account_transmogs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_transmogs` (
  `account` int unsigned NOT NULL DEFAULT '0',
  `guid` int unsigned DEFAULT '0',
  `ModelID` int unsigned NOT NULL DEFAULT '0',
  `Condition` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`ModelID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_tutorial`
--

DROP TABLE IF EXISTS `account_tutorial`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `account_tutorial` (
  `accountId` int unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `tut0` int unsigned NOT NULL DEFAULT '0',
  `tut1` int unsigned NOT NULL DEFAULT '0',
  `tut2` int unsigned NOT NULL DEFAULT '0',
  `tut3` int unsigned NOT NULL DEFAULT '0',
  `tut4` int unsigned NOT NULL DEFAULT '0',
  `tut5` int unsigned NOT NULL DEFAULT '0',
  `tut6` int unsigned NOT NULL DEFAULT '0',
  `tut7` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accountId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `addons`
--

DROP TABLE IF EXISTS `addons`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `addons` (
  `name` varchar(120) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `crc` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Addons';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `auctionhouse`
--

DROP TABLE IF EXISTS `auctionhouse`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `auctionhouse` (
  `id` int unsigned NOT NULL DEFAULT '0',
  `houseid` tinyint unsigned NOT NULL DEFAULT '7',
  `itemguid` bigint unsigned NOT NULL DEFAULT '0',
  `itemowner` bigint unsigned NOT NULL DEFAULT '0',
  `buyoutprice` bigint unsigned NOT NULL DEFAULT '0',
  `time` int unsigned NOT NULL DEFAULT '0',
  `buyguid` bigint unsigned NOT NULL DEFAULT '0',
  `lastbid` bigint unsigned NOT NULL DEFAULT '0',
  `startbid` bigint unsigned NOT NULL DEFAULT '0',
  `deposit` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `item_guid` (`itemguid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bad_sentences`
--

DROP TABLE IF EXISTS `bad_sentences`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `bad_sentences` (
  `id` int unsigned NOT NULL,
  `sentence` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `hash` bigint unsigned NOT NULL,
  `penalty` int NOT NULL DEFAULT '10',
  `sourceMask` int unsigned NOT NULL DEFAULT '1',
  `output` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`hash`) USING BTREE,
  KEY `hash` (`hash`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `blackmarket_auctions`
--

DROP TABLE IF EXISTS `blackmarket_auctions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `blackmarket_auctions` (
  `marketId` int NOT NULL DEFAULT '0',
  `currentBid` bigint unsigned NOT NULL DEFAULT '0',
  `time` int NOT NULL DEFAULT '0',
  `numBids` int NOT NULL DEFAULT '0',
  `bidder` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`marketId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `calendar_events`
--

DROP TABLE IF EXISTS `calendar_events`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `calendar_events` (
  `EventID` bigint unsigned NOT NULL DEFAULT '0',
  `Owner` bigint unsigned NOT NULL DEFAULT '0',
  `Title` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `Description` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `EventType` tinyint unsigned NOT NULL DEFAULT '4',
  `TextureID` int NOT NULL DEFAULT '-1',
  `Date` int unsigned NOT NULL DEFAULT '0',
  `Flags` int unsigned NOT NULL DEFAULT '0',
  `LockDate` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`EventID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `calendar_invites`
--

DROP TABLE IF EXISTS `calendar_invites`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `calendar_invites` (
  `InviteID` bigint unsigned NOT NULL DEFAULT '0',
  `EventID` bigint unsigned NOT NULL DEFAULT '0',
  `Invitee` bigint unsigned NOT NULL DEFAULT '0',
  `Sender` bigint unsigned NOT NULL DEFAULT '0',
  `Status` tinyint unsigned NOT NULL DEFAULT '0',
  `ResponseTime` int unsigned NOT NULL DEFAULT '0',
  `ModerationRank` tinyint unsigned NOT NULL DEFAULT '0',
  `Note` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`InviteID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `challenge`
--

DROP TABLE IF EXISTS `challenge`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `challenge` (
  `ID` int NOT NULL,
  `GuildID` bigint NOT NULL DEFAULT '0',
  `MapID` mediumint NOT NULL DEFAULT '0',
  `ChallengeID` mediumint NOT NULL DEFAULT '0',
  `RecordTime` int NOT NULL DEFAULT '0',
  `Date` int NOT NULL DEFAULT '0',
  `ChallengeLevel` smallint NOT NULL DEFAULT '0',
  `TimerLevel` smallint NOT NULL DEFAULT '0',
  `Affixes` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_unicode_ci NOT NULL,
  `ChestID` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`) USING BTREE,
  KEY `mapID` (`MapID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `challenge_key`
--

DROP TABLE IF EXISTS `challenge_key`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `challenge_key` (
  `guid` int NOT NULL DEFAULT '0',
  `ID` mediumint unsigned NOT NULL DEFAULT '0',
  `Level` tinyint unsigned NOT NULL DEFAULT '0',
  `Affix` tinyint unsigned NOT NULL DEFAULT '0',
  `Affix1` tinyint unsigned NOT NULL DEFAULT '0',
  `Affix2` tinyint unsigned NOT NULL DEFAULT '0',
  `KeyIsCharded` tinyint unsigned NOT NULL DEFAULT '0',
  `timeReset` int unsigned NOT NULL DEFAULT '0',
  `InstanceID` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `timeReset` (`timeReset`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `challenge_member`
--

DROP TABLE IF EXISTS `challenge_member`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `challenge_member` (
  `id` int NOT NULL,
  `member` bigint NOT NULL DEFAULT '0',
  `specID` mediumint NOT NULL DEFAULT '0',
  `ChallengeLevel` smallint NOT NULL DEFAULT '0',
  `Date` int NOT NULL DEFAULT '0',
  `ChestID` int NOT NULL DEFAULT '0',
  KEY `id` (`id`,`member`) USING BTREE,
  KEY `member` (`member`) USING BTREE,
  KEY `Date` (`Date`) USING BTREE,
  KEY `ChallengeLevel` (`ChallengeLevel`) USING BTREE,
  KEY `ChestID` (`ChestID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `challenge_oplote_loot`
--

DROP TABLE IF EXISTS `challenge_oplote_loot`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `challenge_oplote_loot` (
  `guid` int NOT NULL DEFAULT '0',
  `chestListID` varchar(150) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `date` int NOT NULL DEFAULT '0',
  `ChallengeLevel` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `channels`
--

DROP TABLE IF EXISTS `channels`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `channels` (
  `name` varchar(128) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `team` int unsigned NOT NULL,
  `announce` tinyint unsigned NOT NULL DEFAULT '1',
  `ownership` tinyint unsigned NOT NULL DEFAULT '1',
  `password` varchar(32) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  `bannedList` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `lastUsed` int unsigned NOT NULL,
  PRIMARY KEY (`name`,`team`) USING BTREE,
  KEY `name` (`name`) USING BTREE,
  KEY `team` (`team`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Channel System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_account_data`
--

DROP TABLE IF EXISTS `character_account_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_account_data` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `type` tinyint unsigned NOT NULL DEFAULT '0',
  `time` int unsigned NOT NULL DEFAULT '0',
  `data` blob NOT NULL,
  PRIMARY KEY (`guid`,`type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_achievement`
--

DROP TABLE IF EXISTS `character_achievement`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_achievement` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `achievement` int unsigned NOT NULL,
  `date` int unsigned NOT NULL,
  PRIMARY KEY (`guid`,`achievement`) USING BTREE,
  KEY `achievement` (`achievement`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_achievement_progress`
--

DROP TABLE IF EXISTS `character_achievement_progress`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_achievement_progress` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `criteria` int unsigned NOT NULL,
  `counter` int unsigned NOT NULL,
  `date` bigint unsigned NOT NULL DEFAULT '0',
  `achievID` int unsigned NOT NULL DEFAULT '0',
  `completed` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`criteria`) USING BTREE,
  KEY `criteria` (`criteria`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `achievID` (`achievID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_action`
--

DROP TABLE IF EXISTS `character_action`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_action` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `spec` tinyint unsigned NOT NULL DEFAULT '0',
  `button` tinyint unsigned NOT NULL DEFAULT '0',
  `action` bigint unsigned NOT NULL DEFAULT '0',
  `type` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spec`,`button`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_adventure_quest`
--

DROP TABLE IF EXISTS `character_adventure_quest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_adventure_quest` (
  `guid` bigint unsigned NOT NULL,
  `questID` int unsigned NOT NULL,
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `questID` (`questID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_ak_book_weekly`
--

DROP TABLE IF EXISTS `character_ak_book_weekly`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_ak_book_weekly` (
  `guid` bigint unsigned NOT NULL COMMENT 'GUID du personnage ayant deja loote son livre cette semaine',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Verrou hebdomadaire du livre de connaissance (custom)';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_archaeology`
--

DROP TABLE IF EXISTS `character_archaeology`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_archaeology` (
  `guid` bigint NOT NULL,
  `sites` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `counts` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `projects` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Archaeology System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_archaeology_finds`
--

DROP TABLE IF EXISTS `character_archaeology_finds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_archaeology_finds` (
  `guid` bigint NOT NULL,
  `id` int NOT NULL,
  `count` int NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`guid`,`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_army_training_info`
--

DROP TABLE IF EXISTS `character_army_training_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_army_training_info` (
  `guid` bigint NOT NULL,
  `opened_berserk` int NOT NULL DEFAULT '0',
  `opened_mana_wanted` int NOT NULL DEFAULT '0',
  `opened_mage` int NOT NULL DEFAULT '0',
  `opened_hp` tinyint(1) NOT NULL DEFAULT '0',
  `opened_dmg` tinyint(1) NOT NULL DEFAULT '0',
  `opened_fixate` tinyint(1) NOT NULL DEFAULT '0',
  `opened_brave` tinyint(1) NOT NULL DEFAULT '0',
  `opened_chests` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_aura`
--

DROP TABLE IF EXISTS `character_aura`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_aura` (
  `guid` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `caster_guid` binary(16) NOT NULL COMMENT 'Full Global Unique Identifier',
  `item_guid` binary(16) NOT NULL,
  `spell` mediumint unsigned NOT NULL DEFAULT '0',
  `effect_mask` mediumint unsigned NOT NULL DEFAULT '0',
  `recalculate_mask` tinyint unsigned NOT NULL DEFAULT '0',
  `stackcount` tinyint unsigned NOT NULL DEFAULT '1',
  `maxduration` int NOT NULL DEFAULT '0',
  `remaintime` int NOT NULL DEFAULT '0',
  `remaincharges` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`caster_guid`,`item_guid`,`spell`,`effect_mask`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `caster_guid` (`caster_guid`) USING BTREE,
  KEY `item_guid` (`item_guid`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE,
  KEY `effect_mask` (`effect_mask`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_aura_effect`
--

DROP TABLE IF EXISTS `character_aura_effect`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_aura_effect` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `effect` tinyint unsigned NOT NULL DEFAULT '0',
  `baseamount` int NOT NULL DEFAULT '0',
  `amount` int NOT NULL,
  PRIMARY KEY (`guid`,`slot`,`effect`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `slot` (`slot`) USING BTREE,
  KEY `effect` (`effect`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_bag_slot_flags`
--

DROP TABLE IF EXISTS `character_bag_slot_flags`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_bag_slot_flags` (
  `guid` bigint unsigned NOT NULL,
  `bagFlags` varchar(64) NOT NULL DEFAULT '' COMMENT '4 valeurs, sacs du sac a dos',
  `bankBagFlags` varchar(128) NOT NULL DEFAULT '' COMMENT '7 valeurs, sacs de banque',
  `bagSettings` int unsigned NOT NULL DEFAULT '0' COMMENT 'reglages de tri, bit 0 : ignorer le sac a dos',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Filtres de sac par personnage';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_banned`
--

DROP TABLE IF EXISTS `character_banned`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_banned` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `bandate` int unsigned NOT NULL DEFAULT '0',
  `unbandate` int unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `banreason` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `active` tinyint unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`guid`,`bandate`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `bandate` (`bandate`) USING BTREE,
  KEY `unbandate` (`unbandate`) USING BTREE,
  KEY `active` (`active`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Ban List';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_battleground_data`
--

DROP TABLE IF EXISTS `character_battleground_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_battleground_data` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `instanceId` int unsigned NOT NULL COMMENT 'Instance Identifier',
  `team` smallint unsigned NOT NULL,
  `joinX` float NOT NULL DEFAULT '0',
  `joinY` float NOT NULL DEFAULT '0',
  `joinZ` float NOT NULL DEFAULT '0',
  `joinO` float NOT NULL DEFAULT '0',
  `joinMapId` smallint unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `taxiStart` int unsigned NOT NULL DEFAULT '0',
  `taxiEnd` int unsigned NOT NULL DEFAULT '0',
  `mountSpell` mediumint unsigned NOT NULL DEFAULT '0',
  `lastActiveSpec` smallint DEFAULT NULL,
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_battleground_random`
--

DROP TABLE IF EXISTS `character_battleground_random`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_battleground_random` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `bg` tinyint unsigned NOT NULL DEFAULT '0',
  `rbg` tinyint unsigned NOT NULL DEFAULT '0',
  `skirmish` tinyint unsigned NOT NULL DEFAULT '0',
  `arena` tinyint unsigned NOT NULL DEFAULT '0',
  `arena2v2` tinyint unsigned NOT NULL DEFAULT '0',
  `arena3v3` tinyint unsigned NOT NULL DEFAULT '0',
  `brawl` tinyint unsigned NOT NULL DEFAULT '0',
  `brawlArena` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_brackets_info`
--

DROP TABLE IF EXISTS `character_brackets_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_brackets_info` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `bracket` smallint NOT NULL,
  `rating` mediumint NOT NULL DEFAULT '0',
  `oldRating` mediumint NOT NULL DEFAULT '0',
  `best` mediumint NOT NULL DEFAULT '0',
  `oldBest` mediumint NOT NULL DEFAULT '0',
  `bestWeek` smallint NOT NULL DEFAULT '0',
  `mmr` mediumint NOT NULL DEFAULT '0',
  `oldMmr` mediumint NOT NULL DEFAULT '0',
  `games` int NOT NULL DEFAULT '0',
  `oldGames` int NOT NULL DEFAULT '0',
  `wins` int NOT NULL DEFAULT '0',
  `oldWins` int NOT NULL DEFAULT '0',
  `weekGames` mediumint NOT NULL DEFAULT '0',
  `weekWins` mediumint NOT NULL DEFAULT '0',
  `bestWeekLast` mediumint NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`bracket`) USING BTREE,
  KEY `BracketID` (`bracket`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_brackets_info_season`
--

DROP TABLE IF EXISTS `character_brackets_info_season`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_brackets_info_season` (
  `season` tinyint unsigned NOT NULL,
  `guid` bigint unsigned NOT NULL,
  `bracket` smallint NOT NULL,
  `rating` mediumint NOT NULL DEFAULT '0',
  `best` mediumint NOT NULL DEFAULT '0',
  `bestWeek` smallint NOT NULL DEFAULT '0',
  `mmr` mediumint NOT NULL DEFAULT '0',
  `games` int NOT NULL DEFAULT '0',
  `wins` int NOT NULL DEFAULT '0',
  `weekGames` mediumint NOT NULL DEFAULT '0',
  `weekWins` mediumint NOT NULL DEFAULT '0',
  `bestWeekLast` mediumint NOT NULL DEFAULT '0',
  `archived` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`season`,`guid`,`bracket`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='PvP ratings saved per season';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_cuf_profiles`
--

DROP TABLE IF EXISTS `character_cuf_profiles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_cuf_profiles` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `profileId` tinyint unsigned NOT NULL,
  `profileName` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `frameHeight` smallint unsigned NOT NULL DEFAULT '0',
  `frameWidth` smallint unsigned NOT NULL DEFAULT '0',
  `sortBy` tinyint unsigned NOT NULL DEFAULT '0',
  `healthText` tinyint unsigned NOT NULL DEFAULT '0',
  `someOptions` int unsigned NOT NULL DEFAULT '0',
  `unk146` tinyint unsigned NOT NULL DEFAULT '0',
  `unk147` tinyint unsigned NOT NULL DEFAULT '0',
  `unk148` tinyint unsigned NOT NULL DEFAULT '0',
  `unk150` smallint unsigned NOT NULL DEFAULT '0',
  `unk152` smallint unsigned NOT NULL DEFAULT '0',
  `unk154` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`profileId`) USING BTREE,
  KEY `profileId` (`profileId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_currency`
--

DROP TABLE IF EXISTS `character_currency`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_currency` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `currency` smallint unsigned NOT NULL,
  `total_count` int unsigned NOT NULL,
  `week_count` int unsigned NOT NULL,
  `season_total` int DEFAULT '0',
  `flags` int DEFAULT '0',
  `curentcap` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`currency`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_custom_event_reapeter`
--

DROP TABLE IF EXISTS `character_custom_event_reapeter`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_custom_event_reapeter` (
  `guid` bigint NOT NULL,
  `event` int unsigned NOT NULL,
  `repeat_id` int unsigned NOT NULL,
  PRIMARY KEY (`guid`,`event`,`repeat_id`) USING BTREE,
  KEY `select` (`guid`,`event`,`repeat_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_declinedname`
--

DROP TABLE IF EXISTS `character_declinedname`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_declinedname` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `genitive` varchar(15) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `dative` varchar(15) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `accusative` varchar(15) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `instrumental` varchar(15) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `prepositional` varchar(15) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_demon_invasion_progress`
--

DROP TABLE IF EXISTS `character_demon_invasion_progress`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_demon_invasion_progress` (
  `guid` int unsigned NOT NULL,
  `zone_index` tinyint unsigned NOT NULL,
  `completed_time` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`zone_index`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_equipmentsets`
--

DROP TABLE IF EXISTS `character_equipmentsets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_equipmentsets` (
  `guid` bigint NOT NULL DEFAULT '0',
  `setguid` bigint NOT NULL AUTO_INCREMENT,
  `setindex` tinyint unsigned NOT NULL DEFAULT '0',
  `name` varchar(31) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `iconname` varchar(100) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `ignore_mask` int unsigned NOT NULL DEFAULT '0',
  `AssignedSpecIndex` int NOT NULL DEFAULT '-1',
  `item0` bigint unsigned NOT NULL DEFAULT '0',
  `item1` bigint unsigned NOT NULL DEFAULT '0',
  `item2` bigint unsigned NOT NULL DEFAULT '0',
  `item3` bigint unsigned NOT NULL DEFAULT '0',
  `item4` bigint unsigned NOT NULL DEFAULT '0',
  `item5` bigint unsigned NOT NULL DEFAULT '0',
  `item6` bigint unsigned NOT NULL DEFAULT '0',
  `item7` bigint unsigned NOT NULL DEFAULT '0',
  `item8` bigint unsigned NOT NULL DEFAULT '0',
  `item9` bigint unsigned NOT NULL DEFAULT '0',
  `item10` bigint unsigned NOT NULL DEFAULT '0',
  `item11` bigint unsigned NOT NULL DEFAULT '0',
  `item12` bigint unsigned NOT NULL DEFAULT '0',
  `item13` bigint unsigned NOT NULL DEFAULT '0',
  `item14` bigint unsigned NOT NULL DEFAULT '0',
  `item15` bigint unsigned NOT NULL DEFAULT '0',
  `item16` bigint unsigned NOT NULL DEFAULT '0',
  `item17` bigint unsigned NOT NULL DEFAULT '0',
  `item18` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`setguid`) USING BTREE,
  UNIQUE KEY `idx_set` (`guid`,`setguid`,`setindex`) USING BTREE,
  KEY `Idx_setindex` (`setindex`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison`
--

DROP TABLE IF EXISTS `character_garrison`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison` (
  `CharacterGuid` bigint unsigned NOT NULL,
  `SiteLevelId` int unsigned NOT NULL DEFAULT '0',
  `GarrTypeId` int unsigned NOT NULL DEFAULT '0',
  `FollowerActivationsRemainingToday` int unsigned NOT NULL DEFAULT '0',
  `NumFollowerActivationRegenTimestamp` int unsigned NOT NULL DEFAULT '0',
  `CacheLastUsage` int NOT NULL DEFAULT '0',
  `_MissionGen` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`CharacterGuid`,`SiteLevelId`,`GarrTypeId`) USING BTREE,
  KEY `guid` (`CharacterGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison_blueprints`
--

DROP TABLE IF EXISTS `character_garrison_blueprints`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison_blueprints` (
  `guid` bigint unsigned NOT NULL,
  `GarrTypeId` int unsigned NOT NULL DEFAULT '0',
  `buildingId` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`buildingId`,`GarrTypeId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `buildingId` (`buildingId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison_buildings`
--

DROP TABLE IF EXISTS `character_garrison_buildings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison_buildings` (
  `guid` bigint unsigned NOT NULL,
  `GarrTypeId` int unsigned NOT NULL DEFAULT '0',
  `plotInstanceId` int unsigned NOT NULL DEFAULT '0',
  `buildingId` int unsigned NOT NULL DEFAULT '0',
  `timeBuilt` bigint unsigned NOT NULL,
  `data` varchar(64) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  `active` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`GarrTypeId`,`plotInstanceId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `plotInstanceId` (`plotInstanceId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison_follower_abilities`
--

DROP TABLE IF EXISTS `character_garrison_follower_abilities`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison_follower_abilities` (
  `dbId` bigint unsigned NOT NULL,
  `abilityId` int unsigned NOT NULL,
  `GarrTypeId` int unsigned NOT NULL DEFAULT '0',
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `isItemTrait` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`,`slot`,`GarrTypeId`) USING BTREE,
  KEY `dbId` (`dbId`) USING BTREE,
  KEY `abilityId` (`abilityId`) USING BTREE,
  KEY `slot` (`slot`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison_followers`
--

DROP TABLE IF EXISTS `character_garrison_followers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison_followers` (
  `dbId` bigint unsigned NOT NULL,
  `guid` bigint unsigned NOT NULL,
  `followerId` int unsigned NOT NULL DEFAULT '0',
  `GarrTypeId` int unsigned NOT NULL DEFAULT '0',
  `Durability` smallint unsigned NOT NULL DEFAULT '0',
  `quality` int unsigned NOT NULL DEFAULT '2',
  `level` int unsigned NOT NULL DEFAULT '90',
  `itemLevelWeapon` int unsigned NOT NULL DEFAULT '600',
  `itemLevelArmor` int unsigned NOT NULL DEFAULT '600',
  `xp` int unsigned NOT NULL DEFAULT '0',
  `currentBuilding` int unsigned NOT NULL DEFAULT '0',
  `currentMission` int unsigned NOT NULL DEFAULT '0',
  `status` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`) USING BTREE,
  KEY `idx_guid_id` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison_missions`
--

DROP TABLE IF EXISTS `character_garrison_missions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison_missions` (
  `dbId` bigint unsigned NOT NULL,
  `guid` bigint unsigned NOT NULL,
  `missionRecID` int unsigned NOT NULL DEFAULT '0',
  `GarrTypeId` int unsigned NOT NULL DEFAULT '0',
  `offerTime` int unsigned NOT NULL DEFAULT '0',
  `offerDuration` int unsigned NOT NULL DEFAULT '0',
  `startTime` int unsigned NOT NULL DEFAULT '0',
  `travelDuration` int unsigned NOT NULL DEFAULT '0',
  `missionDuration` int unsigned NOT NULL DEFAULT '0',
  `missionState` int unsigned NOT NULL DEFAULT '0',
  `chance` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`) USING BTREE,
  UNIQUE KEY `idx_guid_id` (`guid`,`missionRecID`) USING BTREE,
  KEY `dbId` (`dbId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `missionRecID` (`missionRecID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison_shipment`
--

DROP TABLE IF EXISTS `character_garrison_shipment`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison_shipment` (
  `dbId` bigint NOT NULL,
  `guid` bigint NOT NULL,
  `shipmentID` smallint NOT NULL,
  `GarrTypeId` smallint NOT NULL DEFAULT '0',
  `start` int unsigned NOT NULL DEFAULT '0',
  `end` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`) USING BTREE,
  KEY `dbId` (`dbId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_garrison_talents`
--

DROP TABLE IF EXISTS `character_garrison_talents`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_garrison_talents` (
  `guid` bigint NOT NULL,
  `GarrTypeId` int unsigned NOT NULL DEFAULT '0',
  `talentID` int NOT NULL,
  `orderTime` int NOT NULL,
  `flags` int NOT NULL,
  PRIMARY KEY (`guid`,`GarrTypeId`,`talentID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_gifts`
--

DROP TABLE IF EXISTS `character_gifts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_gifts` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `item_guid` bigint unsigned NOT NULL DEFAULT '0',
  `entry` int unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_glyphs`
--

DROP TABLE IF EXISTS `character_glyphs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_glyphs` (
  `guid` bigint NOT NULL,
  `talentGroup` tinyint unsigned NOT NULL DEFAULT '0',
  `glyphId` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talentGroup`,`glyphId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `talentGroup` (`talentGroup`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_homebind`
--

DROP TABLE IF EXISTS `character_homebind`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_homebind` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `mapId` smallint unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `zoneId` smallint unsigned NOT NULL DEFAULT '0' COMMENT 'Zone Identifier',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_honor`
--

DROP TABLE IF EXISTS `character_honor`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_honor` (
  `Guid` bigint NOT NULL DEFAULT '0',
  `CurrentHonorAtLevel` mediumint NOT NULL DEFAULT '0',
  `HonorLevel` smallint NOT NULL DEFAULT '0',
  `PrestigeLevel` smallint NOT NULL DEFAULT '0',
  PRIMARY KEY (`Guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Honor info system';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_instance`
--

DROP TABLE IF EXISTS `character_instance`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_instance` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `instance` int unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint unsigned NOT NULL DEFAULT '0',
  `map` smallint unsigned NOT NULL DEFAULT '0',
  `difficulty` tinyint unsigned NOT NULL DEFAULT '0',
  `completedEncounters` int unsigned NOT NULL DEFAULT '0',
  `data` tinytext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `Extended` tinyint unsigned NOT NULL DEFAULT '0',
  `resetTime` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`instance`) USING BTREE,
  KEY `instance` (`instance`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_inventory`
--

DROP TABLE IF EXISTS `character_inventory`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_inventory` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `bag` bigint unsigned NOT NULL DEFAULT '0',
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `item` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item`) USING BTREE,
  UNIQUE KEY `guid` (`guid`,`bag`,`slot`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `bag` (`bag`) USING BTREE,
  KEY `slot` (`slot`) USING BTREE,
  KEY `item` (`item`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_kill`
--

DROP TABLE IF EXISTS `character_kill`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_kill` (
  `guid` bigint NOT NULL DEFAULT '0',
  `victim_guid` bigint NOT NULL DEFAULT '0',
  `count` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`victim_guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `victim_guid` (`victim_guid`) USING BTREE,
  KEY `count` (`count`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Kills Yesterday';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_lfg_cooldown`
--

DROP TABLE IF EXISTS `character_lfg_cooldown`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_lfg_cooldown` (
  `guid` int NOT NULL DEFAULT '0',
  `dungeonId` int NOT NULL DEFAULT '0',
  `respawnTime` int DEFAULT '0',
  PRIMARY KEY (`guid`,`dungeonId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_loot_cooldown`
--

DROP TABLE IF EXISTS `character_loot_cooldown`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_loot_cooldown` (
  `guid` int unsigned NOT NULL,
  `type` tinyint NOT NULL DEFAULT '0',
  `entry` int NOT NULL DEFAULT '0',
  `difficultyID` tinyint NOT NULL DEFAULT '0',
  `respawnTime` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`type`,`entry`,`difficultyID`) USING BTREE,
  KEY `entry` (`entry`) USING BTREE,
  KEY `respawnTime` (`respawnTime`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_mythic_plus_deserter`
--

DROP TABLE IF EXISTS `character_mythic_plus_deserter`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_mythic_plus_deserter` (
  `guid` int unsigned NOT NULL,
  `desertions` tinyint unsigned NOT NULL DEFAULT '0' COMMENT 'desertions in a row, capped at 3',
  `last_desertion` int unsigned NOT NULL DEFAULT '0' COMMENT 'unix time of the last desertion',
  `sanction_until` int unsigned NOT NULL DEFAULT '0' COMMENT 'unix time the running penalty ends',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_pet`
--

DROP TABLE IF EXISTS `character_pet`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_pet` (
  `id` int unsigned NOT NULL DEFAULT '0',
  `entry` int unsigned NOT NULL DEFAULT '0',
  `owner` bigint unsigned NOT NULL DEFAULT '0',
  `modelid` int unsigned DEFAULT '0',
  `CreatedBySpell` mediumint unsigned NOT NULL DEFAULT '0',
  `PetType` tinyint unsigned NOT NULL DEFAULT '0',
  `level` smallint unsigned NOT NULL DEFAULT '1',
  `exp` int unsigned NOT NULL DEFAULT '0',
  `Reactstate` tinyint unsigned NOT NULL DEFAULT '0',
  `name` varchar(21) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT 'Pet',
  `renamed` tinyint unsigned NOT NULL DEFAULT '0',
  `curhealth` int unsigned NOT NULL DEFAULT '1',
  `curmana` int unsigned NOT NULL DEFAULT '0',
  `savetime` int unsigned NOT NULL DEFAULT '0',
  `abdata` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `specialization` int unsigned NOT NULL DEFAULT '0',
  `slot` smallint NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `owner` (`owner`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `entry` (`entry`) USING BTREE,
  KEY `idx_slot` (`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Pet System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_pet_declinedname`
--

DROP TABLE IF EXISTS `character_pet_declinedname`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_pet_declinedname` (
  `id` int unsigned NOT NULL DEFAULT '0',
  `owner` bigint unsigned NOT NULL DEFAULT '0',
  `genitive` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `dative` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `accusative` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `instrumental` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `prepositional` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `owner_key` (`owner`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_pvp_talent`
--

DROP TABLE IF EXISTS `character_pvp_talent`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_pvp_talent` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `talent` mediumint unsigned NOT NULL,
  `spec` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talent`,`spec`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `talent` (`talent`) USING BTREE,
  KEY `spec` (`spec`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_queststatus`
--

DROP TABLE IF EXISTS `character_queststatus`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_queststatus` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `account` int unsigned NOT NULL DEFAULT '0',
  `quest` int unsigned NOT NULL DEFAULT '0',
  `status` tinyint unsigned NOT NULL DEFAULT '0',
  `timer` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_queststatus_daily`
--

DROP TABLE IF EXISTS `character_queststatus_daily`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_queststatus_daily` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `account` int NOT NULL DEFAULT '0',
  `quest` int unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  `time` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_queststatus_objectives`
--

DROP TABLE IF EXISTS `character_queststatus_objectives`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_queststatus_objectives` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `account` int unsigned NOT NULL DEFAULT '0',
  `quest` int unsigned NOT NULL DEFAULT '0',
  `objective` tinyint NOT NULL DEFAULT '0',
  `data` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`,`objective`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `quest` (`quest`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_queststatus_rewarded`
--

DROP TABLE IF EXISTS `character_queststatus_rewarded`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_queststatus_rewarded` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `account` int NOT NULL DEFAULT '0',
  `quest` int unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_queststatus_seasonal`
--

DROP TABLE IF EXISTS `character_queststatus_seasonal`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_queststatus_seasonal` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `account` int NOT NULL DEFAULT '0',
  `quest` int unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  `event` int unsigned NOT NULL DEFAULT '0' COMMENT 'Event Identifier',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_queststatus_weekly`
--

DROP TABLE IF EXISTS `character_queststatus_weekly`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_queststatus_weekly` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `account` int NOT NULL DEFAULT '0',
  `quest` int unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_queststatus_world`
--

DROP TABLE IF EXISTS `character_queststatus_world`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_queststatus_world` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `account` int unsigned NOT NULL DEFAULT '0',
  `quest` int unsigned NOT NULL DEFAULT '0',
  `resetTime` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `resetTime` (`resetTime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_rates`
--

DROP TABLE IF EXISTS `character_rates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_rates` (
  `guid` bigint NOT NULL DEFAULT '0',
  `rate` tinyint DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_reputation`
--

DROP TABLE IF EXISTS `character_reputation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_reputation` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `faction` smallint unsigned NOT NULL DEFAULT '0',
  `standing` int NOT NULL DEFAULT '0',
  `flags` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`faction`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `faction` (`faction`) USING BTREE,
  KEY `standing` (`standing`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_reward`
--

DROP TABLE IF EXISTS `character_reward`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_reward` (
  `guid` bigint unsigned NOT NULL AUTO_INCREMENT,
  `owner_guid` bigint unsigned NOT NULL DEFAULT '0',
  `type` int unsigned NOT NULL DEFAULT '0',
  `id` int unsigned NOT NULL DEFAULT '0',
  `count` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `owner_guid` (`owner_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_skills`
--

DROP TABLE IF EXISTS `character_skills`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_skills` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `skill` smallint unsigned NOT NULL,
  `value` smallint unsigned NOT NULL,
  `max` smallint unsigned NOT NULL,
  PRIMARY KEY (`guid`,`skill`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `skill` (`skill`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_social`
--

DROP TABLE IF EXISTS `character_social`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_social` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `friend` bigint unsigned NOT NULL DEFAULT '0',
  `flags` tinyint unsigned NOT NULL DEFAULT '0' COMMENT 'Friend Flags',
  `note` varchar(48) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '' COMMENT 'Friend Note',
  PRIMARY KEY (`guid`,`friend`,`flags`) USING BTREE,
  KEY `friend` (`friend`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_spell`
--

DROP TABLE IF EXISTS `character_spell`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_spell` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `spell` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `active` tinyint unsigned NOT NULL DEFAULT '1',
  `disabled` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_spell_cooldown`
--

DROP TABLE IF EXISTS `character_spell_cooldown`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_spell_cooldown` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `spell` mediumint NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `item` int unsigned NOT NULL DEFAULT '0' COMMENT 'Item Identifier',
  `time` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_stat_kill_creature`
--

DROP TABLE IF EXISTS `character_stat_kill_creature`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_stat_kill_creature` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `entry` int unsigned NOT NULL DEFAULT '0',
  `count` int unsigned DEFAULT '0',
  `point` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`entry`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `entry` (`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_talent`
--

DROP TABLE IF EXISTS `character_talent`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_talent` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `talent` mediumint unsigned NOT NULL,
  `spec` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talent`,`spec`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `spec` (`spec`) USING BTREE,
  KEY `talent` (`talent`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_transmog_outfits`
--

DROP TABLE IF EXISTS `character_transmog_outfits`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_transmog_outfits` (
  `guid` bigint NOT NULL DEFAULT '0',
  `setguid` bigint NOT NULL AUTO_INCREMENT,
  `setindex` tinyint unsigned NOT NULL DEFAULT '0',
  `name` varchar(128) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `iconname` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `ignore_mask` int NOT NULL DEFAULT '0',
  `appearance0` int NOT NULL DEFAULT '0',
  `appearance1` int NOT NULL DEFAULT '0',
  `appearance2` int NOT NULL DEFAULT '0',
  `appearance3` int NOT NULL DEFAULT '0',
  `appearance4` int NOT NULL DEFAULT '0',
  `appearance5` int NOT NULL DEFAULT '0',
  `appearance6` int NOT NULL DEFAULT '0',
  `appearance7` int NOT NULL DEFAULT '0',
  `appearance8` int NOT NULL DEFAULT '0',
  `appearance9` int NOT NULL DEFAULT '0',
  `appearance10` int NOT NULL DEFAULT '0',
  `appearance11` int NOT NULL DEFAULT '0',
  `appearance12` int NOT NULL DEFAULT '0',
  `appearance13` int NOT NULL DEFAULT '0',
  `appearance14` int NOT NULL DEFAULT '0',
  `appearance15` int NOT NULL DEFAULT '0',
  `appearance16` int NOT NULL DEFAULT '0',
  `appearance17` int NOT NULL DEFAULT '0',
  `appearance18` int NOT NULL DEFAULT '0',
  `mainHandEnchant` int NOT NULL DEFAULT '0',
  `offHandEnchant` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`setguid`) USING BTREE,
  UNIQUE KEY `idx_set` (`guid`,`setguid`,`setindex`) USING BTREE,
  KEY `Idx_setindex` (`setindex`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_visuals`
--

DROP TABLE IF EXISTS `character_visuals`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_visuals` (
  `guid` bigint NOT NULL DEFAULT '0',
  `head` int NOT NULL DEFAULT '0',
  `shoulders` int NOT NULL DEFAULT '0',
  `chest` int NOT NULL DEFAULT '0',
  `waist` int NOT NULL DEFAULT '0',
  `legs` int NOT NULL DEFAULT '0',
  `feet` int NOT NULL DEFAULT '0',
  `wrists` int NOT NULL DEFAULT '0',
  `hands` int NOT NULL DEFAULT '0',
  `back` int NOT NULL DEFAULT '0',
  `main` int NOT NULL DEFAULT '0',
  `off` int NOT NULL DEFAULT '0',
  `ranged` int NOT NULL DEFAULT '0',
  `tabard` int unsigned NOT NULL DEFAULT '0',
  `shirt` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `character_void_storage`
--

DROP TABLE IF EXISTS `character_void_storage`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `character_void_storage` (
  `itemId` bigint unsigned NOT NULL,
  `playerGuid` bigint unsigned NOT NULL DEFAULT '0',
  `itemEntry` mediumint unsigned NOT NULL,
  `slot` int unsigned NOT NULL,
  `creatorGuid` bigint unsigned NOT NULL DEFAULT '0',
  `randomPropertyType` tinyint unsigned NOT NULL DEFAULT '0',
  `randomProperty` int unsigned NOT NULL DEFAULT '0',
  `suffixFactor` int unsigned NOT NULL DEFAULT '0',
  `itemGuid` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemId`) USING BTREE,
  UNIQUE KEY `idx_player_slot` (`playerGuid`,`slot`) USING BTREE,
  KEY `idx_player` (`playerGuid`) USING BTREE,
  KEY `randomProperty` (`randomProperty`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `characters`
--

DROP TABLE IF EXISTS `characters`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `characters` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `account` int unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `name` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `race` tinyint unsigned NOT NULL DEFAULT '0',
  `class` tinyint unsigned NOT NULL DEFAULT '0',
  `gender` tinyint unsigned NOT NULL DEFAULT '0',
  `level` tinyint unsigned NOT NULL DEFAULT '0',
  `xp` int unsigned NOT NULL DEFAULT '0',
  `money` bigint unsigned NOT NULL DEFAULT '0',
  `skin` tinyint unsigned NOT NULL DEFAULT '0',
  `face` tinyint unsigned NOT NULL DEFAULT '0',
  `hairStyle` tinyint unsigned NOT NULL DEFAULT '0',
  `hairColor` tinyint unsigned NOT NULL DEFAULT '0',
  `tattoo` tinyint unsigned NOT NULL DEFAULT '0',
  `horn` tinyint unsigned NOT NULL DEFAULT '0',
  `inventorySlots` tinyint unsigned NOT NULL DEFAULT '20',
  `blindfold` tinyint unsigned NOT NULL DEFAULT '0',
  `facialStyle` tinyint unsigned NOT NULL DEFAULT '0',
  `bankSlots` tinyint unsigned NOT NULL DEFAULT '0',
  `drunk` tinyint unsigned NOT NULL DEFAULT '0',
  `playerFlags` int unsigned NOT NULL DEFAULT '0',
  `playerFlagsEx` int unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `map` smallint unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `instance_id` int unsigned NOT NULL DEFAULT '0',
  `dungeonDifficulty` tinyint unsigned NOT NULL DEFAULT '1',
  `raidDifficulty` tinyint unsigned NOT NULL DEFAULT '14',
  `legacyRaidDifficulty` tinyint unsigned NOT NULL DEFAULT '3',
  `orientation` float NOT NULL DEFAULT '0',
  `taximask` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `online` tinyint unsigned NOT NULL DEFAULT '0',
  `cinematic` tinyint unsigned NOT NULL DEFAULT '0',
  `totaltime` int unsigned NOT NULL DEFAULT '0',
  `leveltime` int unsigned NOT NULL DEFAULT '0',
  `created_time` int NOT NULL DEFAULT '0',
  `logout_time` int unsigned NOT NULL DEFAULT '0',
  `is_logout_resting` tinyint unsigned NOT NULL DEFAULT '0',
  `rest_bonus` float NOT NULL DEFAULT '0',
  `trans_x` float NOT NULL DEFAULT '0',
  `trans_y` float NOT NULL DEFAULT '0',
  `trans_z` float NOT NULL DEFAULT '0',
  `trans_o` float NOT NULL DEFAULT '0',
  `transguid` bigint unsigned NOT NULL DEFAULT '0',
  `extra_flags` smallint unsigned NOT NULL DEFAULT '0',
  `summonedPetNumber` int unsigned NOT NULL DEFAULT '0',
  `at_login` smallint unsigned NOT NULL DEFAULT '0',
  `zone` smallint unsigned NOT NULL DEFAULT '0',
  `death_expire_time` int unsigned NOT NULL DEFAULT '0',
  `taxi_path` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `totalKills` int unsigned NOT NULL DEFAULT '0',
  `todayKills` smallint unsigned NOT NULL DEFAULT '0',
  `yesterdayKills` smallint unsigned NOT NULL DEFAULT '0',
  `killPoints` float NOT NULL DEFAULT '0',
  `chosenTitle` int unsigned NOT NULL DEFAULT '0',
  `watchedFaction` int NOT NULL DEFAULT '-1',
  `lfgBonusFaction` int unsigned NOT NULL DEFAULT '0',
  `health` int unsigned NOT NULL DEFAULT '0',
  `mana` int unsigned NOT NULL DEFAULT '0',
  `latency` mediumint unsigned NOT NULL DEFAULT '0',
  `activespec` tinyint unsigned NOT NULL DEFAULT '0',
  `specialization` int NOT NULL,
  `lootspecialization` int NOT NULL DEFAULT '0',
  `exploredZones` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `equipmentCache` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `knownTitles` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `actionBars` tinyint unsigned NOT NULL DEFAULT '0',
  `grantableLevels` tinyint unsigned NOT NULL DEFAULT '0',
  `deleteInfos_Account` int unsigned DEFAULT NULL,
  `deleteInfos_Name` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  `deleteDate` int unsigned DEFAULT NULL,
  `LastCharacterUndelete` int unsigned DEFAULT NULL,
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `online` (`online`) USING BTREE,
  KEY `name` (`name`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `corpse`
--

DROP TABLE IF EXISTS `corpse`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `corpse` (
  `corpseGuid` bigint unsigned NOT NULL DEFAULT '0',
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `mapId` smallint unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `phaseMask` smallint unsigned NOT NULL DEFAULT '1',
  `displayId` int unsigned NOT NULL DEFAULT '0',
  `itemCache` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `bytes1` int unsigned NOT NULL DEFAULT '0',
  `bytes2` int unsigned NOT NULL DEFAULT '0',
  `flags` tinyint unsigned NOT NULL DEFAULT '0',
  `dynFlags` tinyint unsigned NOT NULL DEFAULT '0',
  `time` int unsigned NOT NULL DEFAULT '0',
  `corpseType` tinyint unsigned NOT NULL DEFAULT '0',
  `instanceId` int unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`corpseGuid`) USING BTREE,
  KEY `idx_type` (`corpseType`) USING BTREE,
  KEY `idx_instance` (`instanceId`) USING BTREE,
  KEY `idx_player` (`guid`) USING BTREE,
  KEY `idx_time` (`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Death System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `creature_respawn`
--

DROP TABLE IF EXISTS `creature_respawn`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `creature_respawn` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `respawnTime` int unsigned NOT NULL DEFAULT '0',
  `mapId` smallint unsigned NOT NULL DEFAULT '0',
  `instanceId` int unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`guid`,`instanceId`) USING BTREE,
  KEY `instanceId` (`instanceId`) USING BTREE,
  KEY `respawnTime` (`respawnTime`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `mapId` (`mapId`) USING BTREE,
  KEY `all` (`guid`,`respawnTime`,`mapId`,`instanceId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Grid Loading System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `game_event_condition_save`
--

DROP TABLE IF EXISTS `game_event_condition_save`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `game_event_condition_save` (
  `eventEntry` mediumint unsigned NOT NULL,
  `condition_id` int unsigned NOT NULL DEFAULT '0',
  `done` float DEFAULT '0',
  PRIMARY KEY (`eventEntry`,`condition_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `game_event_save`
--

DROP TABLE IF EXISTS `game_event_save`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `game_event_save` (
  `eventEntry` mediumint unsigned NOT NULL,
  `state` tinyint unsigned NOT NULL DEFAULT '1',
  `next_start` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`eventEntry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `gameobject_respawn`
--

DROP TABLE IF EXISTS `gameobject_respawn`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `gameobject_respawn` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `respawnTime` int unsigned NOT NULL DEFAULT '0',
  `mapId` smallint unsigned NOT NULL DEFAULT '0',
  `instanceId` int unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`guid`,`instanceId`) USING BTREE,
  KEY `instanceId` (`instanceId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `mapId` (`mapId`) USING BTREE,
  KEY `respawnTime` (`respawnTime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Grid Loading System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `gm_subsurveys`
--

DROP TABLE IF EXISTS `gm_subsurveys`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `gm_subsurveys` (
  `surveyId` int unsigned NOT NULL AUTO_INCREMENT,
  `subsurveyId` int unsigned NOT NULL DEFAULT '0',
  `rank` int unsigned NOT NULL DEFAULT '0',
  `comment` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`surveyId`,`subsurveyId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `gm_surveys`
--

DROP TABLE IF EXISTS `gm_surveys`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `gm_surveys` (
  `surveyId` int unsigned NOT NULL AUTO_INCREMENT,
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `mainSurvey` int unsigned NOT NULL DEFAULT '0',
  `overallComment` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `createTime` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`surveyId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `gm_tickets`
--

DROP TABLE IF EXISTS `gm_tickets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `gm_tickets` (
  `ticketId` int unsigned NOT NULL AUTO_INCREMENT,
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `name` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL COMMENT 'Name of ticket creator',
  `message` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `createTime` int unsigned NOT NULL DEFAULT '0',
  `mapId` smallint unsigned NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `lastModifiedTime` int unsigned NOT NULL DEFAULT '0',
  `closedBy` bigint NOT NULL DEFAULT '0',
  `assignedTo` bigint unsigned NOT NULL DEFAULT '0',
  `comment` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `completed` tinyint unsigned NOT NULL DEFAULT '0',
  `escalated` tinyint unsigned NOT NULL DEFAULT '0',
  `viewed` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ticketId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `group_instance`
--

DROP TABLE IF EXISTS `group_instance`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `group_instance` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `instance` int unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint unsigned NOT NULL DEFAULT '0',
  `map` smallint unsigned NOT NULL DEFAULT '0',
  `difficulty` tinyint unsigned NOT NULL DEFAULT '0',
  `completedEncounters` int unsigned NOT NULL DEFAULT '0',
  `resetTime` int unsigned NOT NULL DEFAULT '0',
  `data` tinytext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`guid`,`instance`) USING BTREE,
  KEY `instance` (`instance`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `group_member`
--

DROP TABLE IF EXISTS `group_member`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `group_member` (
  `guid` int unsigned NOT NULL,
  `memberGuid` bigint unsigned NOT NULL DEFAULT '0',
  `memberFlags` tinyint unsigned NOT NULL DEFAULT '0',
  `subgroup` tinyint unsigned NOT NULL DEFAULT '0',
  `roles` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`memberGuid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `memberGuid` (`memberGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Groups';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `groups`
--

DROP TABLE IF EXISTS `groups`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `groups` (
  `guid` int unsigned NOT NULL,
  `leaderGuid` bigint unsigned NOT NULL DEFAULT '0',
  `lootMethod` tinyint unsigned NOT NULL,
  `looterGuid` bigint unsigned NOT NULL DEFAULT '0',
  `lootThreshold` tinyint unsigned NOT NULL,
  `icon1` binary(16) NOT NULL,
  `icon2` binary(16) NOT NULL,
  `icon3` binary(16) NOT NULL,
  `icon4` binary(16) NOT NULL,
  `icon5` binary(16) NOT NULL,
  `icon6` binary(16) NOT NULL,
  `icon7` binary(16) NOT NULL,
  `icon8` binary(16) NOT NULL,
  `groupType` tinyint unsigned NOT NULL,
  `difficulty` tinyint unsigned NOT NULL DEFAULT '1',
  `raidDifficulty` tinyint unsigned NOT NULL DEFAULT '14',
  `legacyRaidDifficulty` tinyint unsigned NOT NULL DEFAULT '3',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `leaderGuid` (`leaderGuid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Groups';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild`
--

DROP TABLE IF EXISTS `guild`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild` (
  `guildid` bigint unsigned NOT NULL DEFAULT '0',
  `name` varchar(24) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `leaderguid` bigint unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  `EmblemStyle` int NOT NULL DEFAULT '-1',
  `EmblemColor` int NOT NULL DEFAULT '-1',
  `BorderStyle` int NOT NULL DEFAULT '-1',
  `BorderColor` int NOT NULL DEFAULT '-1',
  `BackgroundColor` int NOT NULL DEFAULT '-1',
  `info` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `motd` varchar(128) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `createdate` int unsigned NOT NULL DEFAULT '0',
  `BankMoney` bigint unsigned NOT NULL DEFAULT '0',
  `level` int unsigned DEFAULT '1',
  PRIMARY KEY (`guildid`) USING BTREE,
  KEY `guildid` (`guildid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Guild System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_achievement`
--

DROP TABLE IF EXISTS `guild_achievement`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_achievement` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `achievement` int unsigned NOT NULL,
  `date` int unsigned NOT NULL DEFAULT '0',
  `guids` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`guildId`,`achievement`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_achievement_progress`
--

DROP TABLE IF EXISTS `guild_achievement_progress`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_achievement_progress` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `criteria` int unsigned NOT NULL,
  `counter` int unsigned NOT NULL,
  `date` bigint unsigned NOT NULL DEFAULT '0',
  `completedGuid` bigint unsigned NOT NULL DEFAULT '0',
  `achievID` int unsigned NOT NULL,
  `completed` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`criteria`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_bank_eventlog`
--

DROP TABLE IF EXISTS `guild_bank_eventlog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_bank_eventlog` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `LogGuid` int unsigned NOT NULL DEFAULT '0' COMMENT 'Log record identificator - auxiliary column',
  `TabId` tinyint unsigned NOT NULL DEFAULT '0' COMMENT 'Guild bank TabId',
  `EventType` tinyint unsigned NOT NULL DEFAULT '0' COMMENT 'Event type',
  `PlayerGuid` bigint unsigned NOT NULL DEFAULT '0',
  `ItemOrMoney` bigint unsigned NOT NULL DEFAULT '0',
  `ItemStackCount` smallint unsigned NOT NULL DEFAULT '0',
  `DestTabId` tinyint unsigned NOT NULL DEFAULT '0' COMMENT 'Destination Tab Id',
  `TimeStamp` int unsigned NOT NULL DEFAULT '0' COMMENT 'Event UNIX time',
  PRIMARY KEY (`guildId`,`LogGuid`,`TabId`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE,
  KEY `Idx_PlayerGuid` (`PlayerGuid`) USING BTREE,
  KEY `Idx_LogGuid` (`LogGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_bank_item`
--

DROP TABLE IF EXISTS `guild_bank_item`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_bank_item` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint unsigned NOT NULL DEFAULT '0',
  `SlotId` tinyint unsigned NOT NULL DEFAULT '0',
  `item_guid` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`TabId`,`SlotId`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE,
  KEY `Idx_item_guid` (`item_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_bank_right`
--

DROP TABLE IF EXISTS `guild_bank_right`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_bank_right` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint unsigned NOT NULL DEFAULT '0',
  `rid` tinyint unsigned NOT NULL DEFAULT '0',
  `gbright` tinyint unsigned NOT NULL DEFAULT '0',
  `SlotPerDay` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`TabId`,`rid`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_bank_tab`
--

DROP TABLE IF EXISTS `guild_bank_tab`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_bank_tab` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint unsigned NOT NULL DEFAULT '0',
  `TabName` varchar(16) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `TabIcon` varchar(100) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `TabText` varchar(500) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  PRIMARY KEY (`guildId`,`TabId`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_challenges`
--

DROP TABLE IF EXISTS `guild_challenges`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_challenges` (
  `GuildId` int unsigned NOT NULL,
  `ChallengeType` int NOT NULL DEFAULT '0',
  `ChallengeCount` int DEFAULT NULL,
  PRIMARY KEY (`GuildId`,`ChallengeType`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_eventlog`
--

DROP TABLE IF EXISTS `guild_eventlog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_eventlog` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `LogGuid` int unsigned NOT NULL COMMENT 'Log record identificator - auxiliary column',
  `EventType` tinyint unsigned NOT NULL COMMENT 'Event type',
  `PlayerGuid1` bigint unsigned NOT NULL DEFAULT '0',
  `PlayerGuid2` bigint unsigned NOT NULL DEFAULT '0',
  `NewRank` tinyint unsigned NOT NULL COMMENT 'New rank(in case promotion/demotion)',
  `TimeStamp` int unsigned NOT NULL COMMENT 'Event UNIX time',
  PRIMARY KEY (`guildId`,`LogGuid`) USING BTREE,
  KEY `Idx_PlayerGuid1` (`PlayerGuid1`) USING BTREE,
  KEY `Idx_PlayerGuid2` (`PlayerGuid2`) USING BTREE,
  KEY `Idx_LogGuid` (`LogGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Guild Eventlog';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_finder_applicant`
--

DROP TABLE IF EXISTS `guild_finder_applicant`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_finder_applicant` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `playerGuid` bigint unsigned NOT NULL DEFAULT '0',
  `availability` tinyint unsigned DEFAULT '0',
  `classRole` tinyint unsigned DEFAULT '0',
  `interests` tinyint unsigned DEFAULT '0',
  `comment` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  `submitTime` int unsigned DEFAULT NULL,
  UNIQUE KEY `guildId` (`guildId`,`playerGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_finder_guild_settings`
--

DROP TABLE IF EXISTS `guild_finder_guild_settings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_finder_guild_settings` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `availability` tinyint unsigned NOT NULL DEFAULT '0',
  `classRoles` tinyint unsigned NOT NULL DEFAULT '0',
  `interests` tinyint unsigned NOT NULL DEFAULT '0',
  `level` tinyint unsigned NOT NULL DEFAULT '1',
  `listed` tinyint unsigned NOT NULL DEFAULT '0',
  `comment` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  PRIMARY KEY (`guildId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_member`
--

DROP TABLE IF EXISTS `guild_member`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_member` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `rank` tinyint unsigned NOT NULL,
  `pnote` varchar(31) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `offnote` varchar(31) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `BankResetTimeMoney` int unsigned NOT NULL DEFAULT '0',
  `BankRemMoney` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab0` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab0` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab1` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab1` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab2` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab2` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab3` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab3` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab4` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab4` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab5` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab5` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab6` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab6` int unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab7` int unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab7` int unsigned NOT NULL DEFAULT '0',
  `XpContrib` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Total guild XP contributed.',
  `XpContribWeek` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Guild XP contributed this week.',
  `AchPoint` int unsigned NOT NULL DEFAULT '0' COMMENT 'Achievement Points',
  `profId1` smallint unsigned NOT NULL DEFAULT '0',
  `profValue1` smallint unsigned NOT NULL DEFAULT '0',
  `profRank1` smallint unsigned NOT NULL DEFAULT '0',
  `recipesMask1` varchar(1300) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `profId2` smallint unsigned NOT NULL DEFAULT '0',
  `profValue2` smallint unsigned NOT NULL DEFAULT '0',
  `profRank2` smallint unsigned NOT NULL DEFAULT '0',
  `recipesMask2` varchar(1300) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `RepWeek` smallint unsigned NOT NULL DEFAULT '0' COMMENT 'Guild rep earned this week.',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guildid_rank_key` (`guildId`,`rank`) USING BTREE,
  KEY `guildId` (`guildId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Guild System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_newslog`
--

DROP TABLE IF EXISTS `guild_newslog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_newslog` (
  `guildid` bigint unsigned NOT NULL DEFAULT '0',
  `LogGuid` int unsigned NOT NULL DEFAULT '0',
  `EventType` tinyint unsigned NOT NULL DEFAULT '0',
  `PlayerGuid` bigint unsigned NOT NULL DEFAULT '0',
  `Flags` int unsigned NOT NULL DEFAULT '0',
  `Value` int unsigned NOT NULL DEFAULT '0',
  `TimeStamp` int unsigned NOT NULL DEFAULT '0',
  `Data` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`guildid`,`LogGuid`) USING BTREE,
  KEY `guildid_key` (`guildid`) USING BTREE,
  KEY `Idx_PlayerGuid` (`PlayerGuid`) USING BTREE,
  KEY `Idx_LogGuid` (`LogGuid`) USING BTREE,
  KEY `TimeStamp` (`TimeStamp`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_rank`
--

DROP TABLE IF EXISTS `guild_rank`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_rank` (
  `guildId` bigint unsigned NOT NULL DEFAULT '0',
  `rid` tinyint unsigned NOT NULL,
  `rname` varchar(20) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `rights` mediumint unsigned NOT NULL DEFAULT '0',
  `BankMoneyPerDay` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`rid`) USING BTREE,
  KEY `Idx_rid` (`rid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Guild System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_instance`
--

DROP TABLE IF EXISTS `item_instance`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_instance` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `itemEntry` mediumint unsigned NOT NULL DEFAULT '0',
  `owner_guid` bigint unsigned NOT NULL DEFAULT '0',
  `creatorGuid` bigint unsigned NOT NULL DEFAULT '0',
  `giftCreatorGuid` bigint unsigned NOT NULL DEFAULT '0',
  `count` int unsigned NOT NULL DEFAULT '1',
  `duration` int NOT NULL DEFAULT '0',
  `charges` tinytext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `flags` mediumint unsigned NOT NULL DEFAULT '0',
  `enchantments` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `randomPropertyType` tinyint unsigned NOT NULL DEFAULT '0',
  `randomPropertyId` int unsigned NOT NULL DEFAULT '0',
  `durability` smallint unsigned NOT NULL DEFAULT '0',
  `playedTime` int unsigned NOT NULL DEFAULT '0',
  `text` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `upgradeId` int unsigned NOT NULL DEFAULT '0',
  `battlePetSpeciesId` int unsigned NOT NULL DEFAULT '0',
  `battlePetBreedData` int unsigned NOT NULL DEFAULT '0',
  `battlePetLevel` int unsigned NOT NULL DEFAULT '0',
  `battlePetDisplayId` int unsigned NOT NULL DEFAULT '0',
  `bonusListIDs` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `itemLevel` mediumint NOT NULL DEFAULT '0',
  `dungeonEncounterID` mediumint NOT NULL DEFAULT '0',
  `contextID` tinyint unsigned NOT NULL DEFAULT '0',
  `createdTime` int unsigned NOT NULL DEFAULT '0',
  `challengeMapID` int unsigned NOT NULL DEFAULT '0',
  `challengeLevel` int unsigned NOT NULL DEFAULT '0',
  `challengeAffix` int unsigned NOT NULL DEFAULT '0',
  `challengeAffix1` int unsigned NOT NULL DEFAULT '0',
  `challengeAffix2` int unsigned NOT NULL DEFAULT '0',
  `challengeKeyIsCharded` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `owner_guid` (`owner_guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `itemEntry` (`itemEntry`) USING BTREE,
  KEY `randomPropertyId` (`randomPropertyId`) USING BTREE,
  KEY `contextID` (`contextID`) USING BTREE,
  KEY `dungeonEncounterID` (`dungeonEncounterID`) USING BTREE,
  KEY `createdTime` (`createdTime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Item System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_instance_artifact`
--

DROP TABLE IF EXISTS `item_instance_artifact`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_instance_artifact` (
  `itemGuid` bigint unsigned NOT NULL,
  `xp` bigint unsigned NOT NULL DEFAULT '0',
  `artifactAppearanceId` int unsigned NOT NULL DEFAULT '0',
  `itemEntry` int NOT NULL,
  `tier` int NOT NULL DEFAULT '0',
  `char_guid` bigint NOT NULL,
  `totalrank` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemEntry`,`char_guid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE,
  KEY `itemEntry` (`itemEntry`) USING BTREE,
  KEY `tier` (`tier`) USING BTREE,
  KEY `char_guid` (`char_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_instance_artifact_powers`
--

DROP TABLE IF EXISTS `item_instance_artifact_powers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_instance_artifact_powers` (
  `itemGuid` bigint unsigned NOT NULL,
  `artifactPowerId` int unsigned NOT NULL,
  `purchasedRank` tinyint unsigned DEFAULT '0',
  `itemEntry` int NOT NULL,
  `char_guid` bigint NOT NULL,
  `totalrank` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemEntry`,`char_guid`,`artifactPowerId`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE,
  KEY `itemEntry` (`itemEntry`) USING BTREE,
  KEY `char_guid` (`char_guid`) USING BTREE,
  KEY `artifactPowerId` (`artifactPowerId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_instance_gems`
--

DROP TABLE IF EXISTS `item_instance_gems`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_instance_gems` (
  `itemGuid` bigint unsigned NOT NULL,
  `gemItemId1` int unsigned NOT NULL DEFAULT '0',
  `gemBonuses1` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `gemContext1` tinyint unsigned NOT NULL DEFAULT '0',
  `gemScalingLevel1` int unsigned NOT NULL DEFAULT '0',
  `gemItemId2` int unsigned NOT NULL DEFAULT '0',
  `gemBonuses2` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `gemScalingLevel2` int unsigned NOT NULL DEFAULT '0',
  `gemContext2` tinyint unsigned NOT NULL DEFAULT '0',
  `gemItemId3` int unsigned NOT NULL DEFAULT '0',
  `gemBonuses3` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `gemContext3` tinyint unsigned NOT NULL DEFAULT '0',
  `gemScalingLevel3` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_instance_modifiers`
--

DROP TABLE IF EXISTS `item_instance_modifiers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_instance_modifiers` (
  `itemGuid` bigint unsigned NOT NULL,
  `fixedScalingLevel` int unsigned DEFAULT '0',
  `artifactKnowledgeLevel` int unsigned DEFAULT '0',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_instance_relics`
--

DROP TABLE IF EXISTS `item_instance_relics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_instance_relics` (
  `itemGuid` bigint unsigned NOT NULL,
  `char_guid` bigint NOT NULL,
  `first_relic` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `second_relic` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `third_relic` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_instance_transmog`
--

DROP TABLE IF EXISTS `item_instance_transmog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_instance_transmog` (
  `itemGuid` bigint unsigned NOT NULL,
  `itemModifiedAppearanceAllSpecs` int NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec1` int NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec2` int NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec3` int NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec4` int NOT NULL DEFAULT '0',
  `spellItemEnchantmentAllSpecs` int NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec1` int NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec2` int NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec3` int NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec4` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_refund_instance`
--

DROP TABLE IF EXISTS `item_refund_instance`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_refund_instance` (
  `item_guid` bigint unsigned NOT NULL DEFAULT '0',
  `player_guid` bigint unsigned NOT NULL DEFAULT '0',
  `paidMoney` int unsigned NOT NULL DEFAULT '0',
  `paidExtendedCost` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`,`player_guid`) USING BTREE,
  KEY `item_guid` (`item_guid`) USING BTREE,
  KEY `player_guid` (`player_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Item Refund System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `item_soulbound_trade_data`
--

DROP TABLE IF EXISTS `item_soulbound_trade_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_soulbound_trade_data` (
  `itemGuid` bigint unsigned NOT NULL DEFAULT '0',
  `allowedPlayers` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL COMMENT 'Space separated GUID list of players who can receive this item in trade',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Item Refund System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `lfg_data`
--

DROP TABLE IF EXISTS `lfg_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `lfg_data` (
  `guid` int unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `dungeon` int unsigned NOT NULL DEFAULT '0',
  `state` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='LFG Data';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `log_faction_change`
--

DROP TABLE IF EXISTS `log_faction_change`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `log_faction_change` (
  `id` int NOT NULL AUTO_INCREMENT,
  `guid` bigint NOT NULL,
  `account` int NOT NULL,
  `OldRace` int NOT NULL,
  `NewRace` int NOT NULL,
  `date` datetime NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `log_rename`
--

DROP TABLE IF EXISTS `log_rename`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `log_rename` (
  `guid` bigint unsigned NOT NULL,
  `date` datetime NOT NULL,
  `oldName` tinytext CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `newName` tinytext CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  KEY `guid` (`guid`) USING BTREE,
  KEY `oldName` (`oldName`(50)) USING BTREE,
  KEY `newName` (`newName`(50)) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mail`
--

DROP TABLE IF EXISTS `mail`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `mail` (
  `id` int unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `messageType` tinyint unsigned NOT NULL DEFAULT '0',
  `stationery` tinyint NOT NULL DEFAULT '41',
  `mailTemplateId` smallint unsigned NOT NULL DEFAULT '0',
  `sender` bigint unsigned NOT NULL DEFAULT '0',
  `receiver` bigint unsigned NOT NULL DEFAULT '0',
  `subject` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `body` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `has_items` tinyint unsigned NOT NULL DEFAULT '0',
  `expire_time` int unsigned NOT NULL DEFAULT '0',
  `deliver_time` int unsigned NOT NULL DEFAULT '0',
  `money` bigint unsigned NOT NULL DEFAULT '0',
  `cod` bigint unsigned NOT NULL DEFAULT '0',
  `checked` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `receiver` (`receiver`) USING BTREE,
  KEY `checked` (`checked`) USING BTREE,
  KEY `deliver_time` (`deliver_time`) USING BTREE,
  KEY `expire_time` (`expire_time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Mail System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mail_items`
--

DROP TABLE IF EXISTS `mail_items`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `mail_items` (
  `mail_id` int unsigned NOT NULL DEFAULT '0',
  `item_guid` bigint unsigned NOT NULL DEFAULT '0',
  `receiver` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`) USING BTREE,
  KEY `idx_receiver` (`receiver`) USING BTREE,
  KEY `idx_mail_id` (`mail_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mailbox_queue`
--

DROP TABLE IF EXISTS `mailbox_queue`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `mailbox_queue` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `messageType` int unsigned NOT NULL DEFAULT '0',
  `stationery` int unsigned NOT NULL DEFAULT '0',
  `sender_guid` bigint unsigned NOT NULL DEFAULT '0',
  `receiver_guid` bigint unsigned NOT NULL DEFAULT '0',
  `subject` varchar(200) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT 'Support Message',
  `message` varchar(500) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT 'Support Message',
  `money` int unsigned NOT NULL DEFAULT '0',
  `item` int unsigned NOT NULL DEFAULT '0',
  `item_count` int unsigned NOT NULL DEFAULT '0',
  `store_history` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pet_aura`
--

DROP TABLE IF EXISTS `pet_aura`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `pet_aura` (
  `guid` int unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `caster_guid` binary(16) NOT NULL COMMENT 'Full Global Unique Identifier',
  `spell` mediumint unsigned NOT NULL DEFAULT '0',
  `effect_mask` tinyint unsigned NOT NULL DEFAULT '0',
  `recalculate_mask` tinyint unsigned NOT NULL DEFAULT '0',
  `stackcount` tinyint unsigned NOT NULL DEFAULT '1',
  `maxduration` int NOT NULL DEFAULT '0',
  `remaintime` int NOT NULL DEFAULT '0',
  `remaincharges` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`,`effect_mask`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Pet System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pet_aura_effect`
--

DROP TABLE IF EXISTS `pet_aura_effect`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `pet_aura_effect` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `effect` tinyint unsigned NOT NULL DEFAULT '0',
  `amount` int NOT NULL DEFAULT '0',
  `baseamount` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`slot`,`effect`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pet_spell`
--

DROP TABLE IF EXISTS `pet_spell`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `pet_spell` (
  `guid` int unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `spell` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `active` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Pet System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pet_spell_cooldown`
--

DROP TABLE IF EXISTS `pet_spell_cooldown`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `pet_spell_cooldown` (
  `guid` int unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier, Low part',
  `spell` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `time` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `petition`
--

DROP TABLE IF EXISTS `petition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `petition` (
  `ownerguid` bigint unsigned NOT NULL DEFAULT '0',
  `petitionguid` bigint unsigned NOT NULL DEFAULT '0',
  `name` varchar(24) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `type` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`type`) USING BTREE,
  UNIQUE KEY `index_ownerguid_petitionguid` (`ownerguid`,`petitionguid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Guild System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `petition_sign`
--

DROP TABLE IF EXISTS `petition_sign`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `petition_sign` (
  `ownerguid` bigint unsigned NOT NULL DEFAULT '0',
  `petitionguid` bigint unsigned NOT NULL DEFAULT '0',
  `playerguid` bigint unsigned NOT NULL DEFAULT '0',
  `player_account` int unsigned NOT NULL DEFAULT '0',
  `type` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`petitionguid`,`playerguid`) USING BTREE,
  KEY `Idx_playerguid` (`playerguid`) USING BTREE,
  KEY `Idx_ownerguid` (`ownerguid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Guild System';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pool_quest_save`
--

DROP TABLE IF EXISTS `pool_quest_save`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `pool_quest_save` (
  `pool_id` int unsigned NOT NULL DEFAULT '0',
  `quest_id` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`pool_id`,`quest_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `report_complaints`
--

DROP TABLE IF EXISTS `report_complaints`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `report_complaints` (
  `ID` bigint unsigned NOT NULL COMMENT 'Identifier',
  `ReportPlayer` bigint unsigned NOT NULL DEFAULT '0',
  `ReportAccount` int unsigned NOT NULL DEFAULT '0',
  `ReportTime` int unsigned NOT NULL DEFAULT '0',
  `SpammerGuid` bigint unsigned NOT NULL,
  `ComplaintType` smallint unsigned NOT NULL DEFAULT '0',
  `MailID` int unsigned NOT NULL DEFAULT '0',
  `TimeSinceOffence` int unsigned NOT NULL DEFAULT '0',
  `MessageLog` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `JustBanned` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Spam report system';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `reserved_name`
--

DROP TABLE IF EXISTS `reserved_name`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `reserved_name` (
  `name` varchar(12) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`name`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Player Reserved Names';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `updates`
--

DROP TABLE IF EXISTS `updates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `updates` (
  `name` varchar(200) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL COMMENT 'filename with extension of the update.',
  `hash` char(40) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT '' COMMENT 'sha1 hash of the sql file.',
  `state` enum('RELEASED','ARCHIVED') CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if an update is released or archived.',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'timestamp when the query was applied.',
  `speed` int unsigned NOT NULL DEFAULT '0' COMMENT 'time the query takes to apply in ms.',
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='List of all applied updates in this database.';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `updates_include`
--

DROP TABLE IF EXISTS `updates_include`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `updates_include` (
  `path` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL COMMENT 'directory to include. $ means relative to the source directory.',
  `state` enum('RELEASED','ARCHIVED') CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if the directory contains released or archived updates.',
  PRIMARY KEY (`path`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='List of directories where we want to include sql updates.';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `version`
--

DROP TABLE IF EXISTS `version`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `version` (
  `core_version` varchar(120) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL COMMENT 'Core revision dumped at startup.',
  `core_revision` varchar(120) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  `db_version` varchar(120) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL COMMENT 'Version of characters DB.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Version Notes';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `world_quest`
--

DROP TABLE IF EXISTS `world_quest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `world_quest` (
  `QuestID` int unsigned NOT NULL DEFAULT '0',
  `QuestInfoID` int unsigned NOT NULL DEFAULT '0',
  `QuestSortID` int NOT NULL DEFAULT '0',
  `VariableID` int NOT NULL DEFAULT '0',
  `Value` int NOT NULL DEFAULT '0',
  `Timer` int NOT NULL DEFAULT '0',
  `StartTime` int NOT NULL DEFAULT '0',
  `ResetTime` int NOT NULL DEFAULT '0',
  `CurrencyID` int NOT NULL DEFAULT '0',
  `CurrencyCount` int NOT NULL DEFAULT '0',
  `Gold` int NOT NULL DEFAULT '0',
  `RewardType` int NOT NULL DEFAULT '0',
  `ItemList` longtext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  PRIMARY KEY (`QuestID`) USING BTREE,
  KEY `ResetTime` (`ResetTime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `worldstate_data`
--

DROP TABLE IF EXISTS `worldstate_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `worldstate_data` (
  `VariableID` int unsigned NOT NULL DEFAULT '0' COMMENT 'WorldState ID (can be 0)',
  `InstanceID` int unsigned NOT NULL DEFAULT '0' COMMENT 'WorldState instance',
  `Type` int unsigned NOT NULL DEFAULT '0' COMMENT 'WorldStatesData::Types',
  `ConditionID` int unsigned NOT NULL DEFAULT '0' COMMENT 'Condition (dependent from type)',
  `Flags` int unsigned NOT NULL DEFAULT '0' COMMENT 'Current flags see WorldStatesData::Falgs',
  `Value` int unsigned NOT NULL DEFAULT '0',
  `RenewTime` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Time of last renew of WorldState',
  PRIMARY KEY (`VariableID`,`InstanceID`,`Type`,`ConditionID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='WorldState data storage';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `worldstates`
--

DROP TABLE IF EXISTS `worldstates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `worldstates` (
  `entry` int unsigned NOT NULL DEFAULT '0',
  `value` int unsigned NOT NULL DEFAULT '0',
  `comment` tinytext CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  PRIMARY KEY (`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=COMPACT COMMENT='Variable Saves';
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

--
-- ------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;

--
-- Dumping data for table `game_event_save`
--

LOCK TABLES `game_event_save` WRITE;
/*!40000 ALTER TABLE `game_event_save` DISABLE KEYS */;
INSERT INTO `game_event_save` VALUES (33,2,0),(34,2,0),(35,2,0),(36,2,0),(37,2,0),(38,2,0),(39,2,0),(40,2,0),(41,2,0),(42,2,0),(43,2,0),(44,2,0),(45,2,0),(46,2,0),(47,2,0);
/*!40000 ALTER TABLE `game_event_save` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `pool_quest_save`
--

LOCK TABLES `pool_quest_save` WRITE;
/*!40000 ALTER TABLE `pool_quest_save` DISABLE KEYS */;
INSERT INTO `pool_quest_save` VALUES (348,24635),(349,14102),(350,13905),(352,11377),(353,11667),(354,13422),(357,11385),(5662,13675),(5663,13762),(5664,13770),(5665,13775),(5666,13779),(5667,13785),(5668,13670),(5669,13603),(5670,13743),(5671,13748),(5672,13759),(5673,13754),(5674,13103),(5675,13116),(5676,13832),(5677,12960),(5678,29346),(5679,29320),(5680,26557),(5681,26536),(5682,29347),(5683,29321),(5684,29363),(5685,29334),(5686,26234),(5687,26177),(5688,29356),(5689,29314),(5707,29037),(60015,60018);
/*!40000 ALTER TABLE `pool_quest_save` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `worldstates`
--

LOCK TABLES `worldstates` WRITE;
/*!40000 ALTER TABLE `worldstates` DISABLE KEYS */;
INSERT INTO `worldstates` VALUES (3781,9000000,NULL),(3801,0,NULL),(3802,1,NULL),(5333,9000000,NULL),(5384,0,NULL),(5387,0,NULL),(20001,1791306000,NULL),(20002,1791306000,NULL),(20003,1791284400,NULL),(20005,1791255600,NULL),(20006,1791277200,NULL),(20008,1791306000,NULL),(20011,1729791234,NULL),(20015,1791306000,NULL),(20016,6,NULL),(20017,4,NULL),(20018,9,NULL),(20019,1791230400,NULL),(20020,1791252000,NULL),(20021,1790701200,NULL),(20022,1791450000,NULL),(20023,1,NULL),(20024,1791230400,NULL),(20100,63,'Raid opening: EmeraldNightmare'),(20101,15,'Raid opening: TrialOfValor'),(20102,127,'Raid opening: Nighthold'),(20103,127,'Raid opening: TombOfSargeras'),(20104,127,'Raid opening: Antorus'),(20110,0,NULL),(20111,0,NULL),(20121,1790513386,NULL),(20123,1790513386,NULL),(20124,1790513386,NULL);
/*!40000 ALTER TABLE `worldstates` ENABLE KEYS */;
UNLOCK TABLES;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
INSERT INTO `updates` VALUES ('0001_default_enable_2fa_backpack_slots.sql','CBB545BBCFEAC9A681C08FBC57E25017F240580D','ARCHIVED','2024-10-23 17:32:54',33);
INSERT INTO `updates` VALUES ('0002_ahbot_market_price.sql','B4738D53365908FFF60CCB9E33E9964614649BE6','ARCHIVED','2024-10-23 17:32:54',388);
INSERT INTO `updates` VALUES ('0003_ahbot_market_price_avg.sql','644A22D52D0D9FBA132A3DDDFC77FF7FC1ACBBAC','ARCHIVED','2024-10-23 17:32:55',362);
INSERT INTO `updates` VALUES ('0004_fix_auctionhouse_house_id.sql','7FC06FF0A47B468628CAFC2CA764556B4D52A23E','ARCHIVED','2024-10-23 17:32:55',112);
INSERT INTO `updates` VALUES ('0005_character_action_expansion.sql','7B040C9ABE03CFD8D351DAC82B2F5F558698D163','ARCHIVED','2024-10-23 17:32:55',100);
INSERT INTO `updates` VALUES ('0006_character_pet_slot.sql','5FFAEF34AAC93B8C0AA2AD1D311B5D8006247566','ARCHIVED','2024-10-23 17:32:55',92);
INSERT INTO `updates` VALUES ('0007_character_pet_slot.sql','E2867C8AB20FB67E26710A85D73A3CBDB833379C','ARCHIVED','2024-10-23 17:32:55',183);
INSERT INTO `updates` VALUES ('2024_11_24_drop_transfers.sql','9F36DAB4966158BD16929E7862E4C8FAE9034EEF','ARCHIVED','2026-02-23 01:44:58',57);
INSERT INTO `updates` VALUES ('2024_11_26_remove_donate_system.sql','C41C01D9C08424F580AF10A2A72AB85585D956FA','ARCHIVED','2026-02-23 01:44:58',253);
INSERT INTO `updates` VALUES ('2026_08_28_00_artifact_knowledge_book_weekly.sql','B89936ABF9D7333D60D5135C95AED0653F307854','ARCHIVED','2026-08-28 16:56:23',50);
INSERT INTO `updates` VALUES ('2026_08_29_01_bag_slot_flags.sql','A8F2A8475E999A449A0C02ACCE442EF1ECC710FB','ARCHIVED','2026-08-29 19:55:22',80);
INSERT INTO `updates` VALUES ('2026_08_30_00_drop_ahbot_market_data.sql','2800056B72572500CB46F2E37037BD4D38F543C2','ARCHIVED','2026-08-29 22:15:16',0);
INSERT INTO `updates` VALUES ('2026_08_30_01_artifact_tier_rollback.sql','D4C06063240D92CF1621511E5CABD3CA1D913FC1','ARCHIVED','2026-08-30 03:30:12',45);
INSERT INTO `updates` VALUES ('2026_09_24_00_lorlathil_taxi_node.sql','E8E9FB495658E58BA7E977D36DF1BED5C096D9A8','RELEASED','2026-09-24 09:19:32',26);
INSERT INTO `updates` VALUES ('2026_09_25_01_challenge_karazhan_ids.sql','C83390205DE143CC4081B8BB8789275E02040834','RELEASED','2026-09-26 17:54:01',30);
INSERT INTO `updates` VALUES ('2026_09_26_00_pvp_season_archive.sql','03EBEA0B3E7409DCB129E5EADB52712647CD8D4D','RELEASED','2026-09-26 20:49:00',62);
INSERT INTO `updates` VALUES ('demon_invasion_phase4_progress.sql','862D45DB763AC7A02ADE0EFDB792E1A23DFBBB6F','ARCHIVED','2026-02-24 14:46:33',100);
INSERT INTO `updates_include` VALUES ('$/sql/old/characters','ARCHIVED');
INSERT INTO `updates_include` VALUES ('$/sql/updates/characters','RELEASED');
INSERT INTO `version` VALUES ('DemonCore','','DemonCore Characters Database 2024-10-23');
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
