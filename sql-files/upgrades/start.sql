/*
Navicat MySQL Data Transfer

Source Server         : localhost_3306
Source Server Version : 50505
Source Host           : localhost:3306
Source Database       : start

Target Server Type    : MYSQL
Target Server Version : 50505
File Encoding         : 65001

Date: 2020-02-04 09:41:00
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for acc_reg_num
-- ----------------------------
DROP TABLE IF EXISTS `acc_reg_num`;
CREATE TABLE `acc_reg_num` (
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) CHARACTER SET latin1 COLLATE latin1_bin NOT NULL DEFAULT '',
  `index` int(11) unsigned NOT NULL DEFAULT 0,
  `value` bigint(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of acc_reg_num
-- ----------------------------

-- ----------------------------
-- Table structure for acc_reg_str
-- ----------------------------
DROP TABLE IF EXISTS `acc_reg_str`;
CREATE TABLE `acc_reg_str` (
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) CHARACTER SET latin1 COLLATE latin1_bin NOT NULL DEFAULT '',
  `index` int(11) unsigned NOT NULL DEFAULT 0,
  `value` varchar(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of acc_reg_str
-- ----------------------------

-- ----------------------------
-- Table structure for achievement
-- ----------------------------
DROP TABLE IF EXISTS `achievement`;
CREATE TABLE `achievement` (
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `id` bigint(11) unsigned NOT NULL,
  `count1` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count2` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count3` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count4` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count5` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count6` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count7` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count8` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count9` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count10` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `completed` datetime DEFAULT NULL,
  `rewarded` datetime DEFAULT NULL,
  PRIMARY KEY (`char_id`,`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of achievement
-- ----------------------------

-- ----------------------------
-- Table structure for atcommandlog
-- ----------------------------
DROP TABLE IF EXISTS `atcommandlog`;
CREATE TABLE `atcommandlog` (
  `atcommand_id` mediumint(9) unsigned NOT NULL AUTO_INCREMENT,
  `atcommand_date` datetime NOT NULL,
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `char_name` varchar(25) NOT NULL DEFAULT '',
  `map` varchar(11) NOT NULL DEFAULT '',
  `command` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`atcommand_id`),
  KEY `account_id` (`account_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of atcommandlog
-- ----------------------------

-- ----------------------------
-- Table structure for auction
-- ----------------------------
DROP TABLE IF EXISTS `auction`;
CREATE TABLE `auction` (
  `auction_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `seller_id` int(11) unsigned NOT NULL DEFAULT 0,
  `seller_name` varchar(30) NOT NULL DEFAULT '',
  `buyer_id` int(11) unsigned NOT NULL DEFAULT 0,
  `buyer_name` varchar(30) NOT NULL DEFAULT '',
  `price` int(11) unsigned NOT NULL DEFAULT 0,
  `buynow` int(11) unsigned NOT NULL DEFAULT 0,
  `hours` smallint(6) NOT NULL DEFAULT 0,
  `timestamp` int(11) unsigned NOT NULL DEFAULT 0,
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `item_name` varchar(50) NOT NULL DEFAULT '',
  `type` smallint(6) NOT NULL DEFAULT 0,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`auction_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of auction
-- ----------------------------

-- ----------------------------
-- Table structure for bonus_script
-- ----------------------------
DROP TABLE IF EXISTS `bonus_script`;
CREATE TABLE `bonus_script` (
  `char_id` int(11) unsigned NOT NULL,
  `script` text NOT NULL,
  `tick` bigint(20) NOT NULL DEFAULT 0,
  `flag` smallint(5) unsigned NOT NULL DEFAULT 0,
  `type` tinyint(1) unsigned NOT NULL DEFAULT 0,
  `icon` smallint(3) NOT NULL DEFAULT -1,
  PRIMARY KEY (`char_id`,`type`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of bonus_script
-- ----------------------------

-- ----------------------------
-- Table structure for branchlog
-- ----------------------------
DROP TABLE IF EXISTS `branchlog`;
CREATE TABLE `branchlog` (
  `branch_id` mediumint(9) unsigned NOT NULL AUTO_INCREMENT,
  `branch_date` datetime NOT NULL,
  `account_id` int(11) NOT NULL DEFAULT 0,
  `char_id` int(11) NOT NULL DEFAULT 0,
  `char_name` varchar(25) NOT NULL DEFAULT '',
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`branch_id`),
  KEY `account_id` (`account_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of branchlog
-- ----------------------------

-- ----------------------------
-- Table structure for buyingstores
-- ----------------------------
DROP TABLE IF EXISTS `buyingstores`;
CREATE TABLE `buyingstores` (
  `id` int(10) unsigned NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(10) unsigned NOT NULL,
  `sex` enum('F','M') NOT NULL DEFAULT 'M',
  `map` varchar(20) NOT NULL,
  `x` smallint(5) unsigned NOT NULL,
  `y` smallint(5) unsigned NOT NULL,
  `title` varchar(80) NOT NULL,
  `limit` int(10) unsigned NOT NULL,
  `body_direction` char(1) NOT NULL DEFAULT '4',
  `head_direction` char(1) NOT NULL DEFAULT '0',
  `sit` char(1) NOT NULL DEFAULT '1',
  `autotrade` tinyint(4) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of buyingstores
-- ----------------------------

-- ----------------------------
-- Table structure for buyingstore_items
-- ----------------------------
DROP TABLE IF EXISTS `buyingstore_items`;
CREATE TABLE `buyingstore_items` (
  `buyingstore_id` int(10) unsigned NOT NULL,
  `index` smallint(5) unsigned NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL,
  `price` int(10) unsigned NOT NULL,
  PRIMARY KEY (`buyingstore_id`,`index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of buyingstore_items
-- ----------------------------

-- ----------------------------
-- Table structure for cart_inventory
-- ----------------------------
DROP TABLE IF EXISTS `cart_inventory`;
CREATE TABLE `cart_inventory` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL DEFAULT 0,
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `amount` int(11) NOT NULL DEFAULT 0,
  `equip` int(11) unsigned NOT NULL DEFAULT 0,
  `identify` smallint(6) NOT NULL DEFAULT 0,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint(4) NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `expire_time` int(11) unsigned NOT NULL DEFAULT 0,
  `bound` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of cart_inventory
-- ----------------------------

-- ----------------------------
-- Table structure for cashlog
-- ----------------------------
DROP TABLE IF EXISTS `cashlog`;
CREATE TABLE `cashlog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int(11) NOT NULL DEFAULT 0,
  `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','$') NOT NULL DEFAULT 'S',
  `cash_type` enum('O','K','C') NOT NULL DEFAULT 'O',
  `amount` int(11) NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `type` (`type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of cashlog
-- ----------------------------

-- ----------------------------
-- Table structure for char
-- ----------------------------
DROP TABLE IF EXISTS `char`;
CREATE TABLE `char` (
  `char_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `char_num` tinyint(1) NOT NULL DEFAULT 0,
  `name` varchar(30) NOT NULL DEFAULT '',
  `class` smallint(6) unsigned NOT NULL DEFAULT 0,
  `base_level` smallint(6) unsigned NOT NULL DEFAULT 1,
  `job_level` smallint(6) unsigned NOT NULL DEFAULT 1,
  `base_exp` bigint(20) unsigned NOT NULL DEFAULT 0,
  `job_exp` bigint(20) unsigned NOT NULL DEFAULT 0,
  `zeny` int(11) unsigned NOT NULL DEFAULT 0,
  `str` smallint(4) unsigned NOT NULL DEFAULT 0,
  `agi` smallint(4) unsigned NOT NULL DEFAULT 0,
  `vit` smallint(4) unsigned NOT NULL DEFAULT 0,
  `int` smallint(4) unsigned NOT NULL DEFAULT 0,
  `dex` smallint(4) unsigned NOT NULL DEFAULT 0,
  `luk` smallint(4) unsigned NOT NULL DEFAULT 0,
  `max_hp` int(11) unsigned NOT NULL DEFAULT 0,
  `hp` int(11) unsigned NOT NULL DEFAULT 0,
  `max_sp` int(11) unsigned NOT NULL DEFAULT 0,
  `sp` int(11) unsigned NOT NULL DEFAULT 0,
  `status_point` int(11) unsigned NOT NULL DEFAULT 0,
  `skill_point` int(11) unsigned NOT NULL DEFAULT 0,
  `option` int(11) NOT NULL DEFAULT 0,
  `karma` tinyint(3) NOT NULL DEFAULT 0,
  `manner` smallint(6) NOT NULL DEFAULT 0,
  `party_id` int(11) unsigned NOT NULL DEFAULT 0,
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `pet_id` int(11) unsigned NOT NULL DEFAULT 0,
  `homun_id` int(11) unsigned NOT NULL DEFAULT 0,
  `elemental_id` int(11) unsigned NOT NULL DEFAULT 0,
  `hair` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `hair_color` smallint(5) unsigned NOT NULL DEFAULT 0,
  `clothes_color` smallint(5) unsigned NOT NULL DEFAULT 0,
  `body` smallint(5) unsigned NOT NULL DEFAULT 0,
  `weapon` smallint(6) unsigned NOT NULL DEFAULT 0,
  `shield` smallint(6) unsigned NOT NULL DEFAULT 0,
  `head_top` smallint(6) unsigned NOT NULL DEFAULT 0,
  `head_mid` smallint(6) unsigned NOT NULL DEFAULT 0,
  `head_bottom` smallint(6) unsigned NOT NULL DEFAULT 0,
  `robe` smallint(6) unsigned NOT NULL DEFAULT 0,
  `last_map` varchar(11) NOT NULL DEFAULT '',
  `last_x` smallint(4) unsigned NOT NULL DEFAULT 53,
  `last_y` smallint(4) unsigned NOT NULL DEFAULT 111,
  `save_map` varchar(11) NOT NULL DEFAULT '',
  `save_x` smallint(4) unsigned NOT NULL DEFAULT 53,
  `save_y` smallint(4) unsigned NOT NULL DEFAULT 111,
  `partner_id` int(11) unsigned NOT NULL DEFAULT 0,
  `online` tinyint(2) NOT NULL DEFAULT 0,
  `father` int(11) unsigned NOT NULL DEFAULT 0,
  `mother` int(11) unsigned NOT NULL DEFAULT 0,
  `child` int(11) unsigned NOT NULL DEFAULT 0,
  `fame` int(11) unsigned NOT NULL DEFAULT 0,
  `rename` smallint(3) unsigned NOT NULL DEFAULT 0,
  `delete_date` int(11) unsigned NOT NULL DEFAULT 0,
  `moves` int(11) unsigned NOT NULL DEFAULT 0,
  `unban_time` int(11) unsigned NOT NULL DEFAULT 0,
  `font` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `uniqueitem_counter` int(11) unsigned NOT NULL DEFAULT 0,
  `sex` enum('M','F','U') NOT NULL DEFAULT 'U',
  `hotkey_rowshift` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `clan_id` int(11) unsigned NOT NULL DEFAULT 0,
  `last_login` datetime DEFAULT NULL,
  `title_id` int(11) unsigned NOT NULL DEFAULT 0,
  `show_equip` tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`),
  UNIQUE KEY `name_key` (`name`),
  KEY `account_id` (`account_id`),
  KEY `party_id` (`party_id`),
  KEY `guild_id` (`guild_id`),
  KEY `online` (`online`)
) ENGINE=MyISAM AUTO_INCREMENT=150000 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of char
-- ----------------------------

-- ----------------------------
-- Table structure for charlog
-- ----------------------------
DROP TABLE IF EXISTS `charlog`;
CREATE TABLE `charlog` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_msg` varchar(255) NOT NULL DEFAULT 'char select',
  `account_id` int(11) NOT NULL DEFAULT 0,
  `char_num` tinyint(4) NOT NULL DEFAULT 0,
  `name` varchar(23) NOT NULL DEFAULT '',
  `str` int(11) unsigned NOT NULL DEFAULT 0,
  `agi` int(11) unsigned NOT NULL DEFAULT 0,
  `vit` int(11) unsigned NOT NULL DEFAULT 0,
  `int` int(11) unsigned NOT NULL DEFAULT 0,
  `dex` int(11) unsigned NOT NULL DEFAULT 0,
  `luk` int(11) unsigned NOT NULL DEFAULT 0,
  `hair` tinyint(4) NOT NULL DEFAULT 0,
  `hair_color` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of charlog
-- ----------------------------

-- ----------------------------
-- Table structure for char_reg_num
-- ----------------------------
DROP TABLE IF EXISTS `char_reg_num`;
CREATE TABLE `char_reg_num` (
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) CHARACTER SET latin1 COLLATE latin1_bin NOT NULL DEFAULT '',
  `index` int(11) unsigned NOT NULL DEFAULT 0,
  `value` bigint(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of char_reg_num
-- ----------------------------

-- ----------------------------
-- Table structure for char_reg_str
-- ----------------------------
DROP TABLE IF EXISTS `char_reg_str`;
CREATE TABLE `char_reg_str` (
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) CHARACTER SET latin1 COLLATE latin1_bin NOT NULL DEFAULT '',
  `index` int(11) unsigned NOT NULL DEFAULT 0,
  `value` varchar(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of char_reg_str
-- ----------------------------

-- ----------------------------
-- Table structure for chatlog
-- ----------------------------
DROP TABLE IF EXISTS `chatlog`;
CREATE TABLE `chatlog` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `type` enum('O','W','P','G','M','C') NOT NULL DEFAULT 'O',
  `type_id` int(11) NOT NULL DEFAULT 0,
  `src_charid` int(11) NOT NULL DEFAULT 0,
  `src_accountid` int(11) NOT NULL DEFAULT 0,
  `src_map` varchar(11) NOT NULL DEFAULT '',
  `src_map_x` smallint(4) NOT NULL DEFAULT 0,
  `src_map_y` smallint(4) NOT NULL DEFAULT 0,
  `dst_charname` varchar(25) NOT NULL DEFAULT '',
  `message` varchar(150) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `src_accountid` (`src_accountid`),
  KEY `src_charid` (`src_charid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of chatlog
-- ----------------------------

-- ----------------------------
-- Table structure for clan
-- ----------------------------
DROP TABLE IF EXISTS `clan`;
CREATE TABLE `clan` (
  `clan_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(24) NOT NULL DEFAULT '',
  `master` varchar(24) NOT NULL DEFAULT '',
  `mapname` varchar(24) NOT NULL DEFAULT '',
  `max_member` smallint(6) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`clan_id`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of clan
-- ----------------------------
INSERT INTO `clan` VALUES ('1', 'Swordman Clan', 'Raffam Oranpere', 'prontera', '500');
INSERT INTO `clan` VALUES ('2', 'Arcwand Clan', 'Devon Aire', 'geffen', '500');
INSERT INTO `clan` VALUES ('3', 'Golden Mace Clan', 'Berman Aire', 'prontera', '500');
INSERT INTO `clan` VALUES ('4', 'Crossbow Clan', 'Shaam Rumi', 'payon', '500');

-- ----------------------------
-- Table structure for clan_alliance
-- ----------------------------
DROP TABLE IF EXISTS `clan_alliance`;
CREATE TABLE `clan_alliance` (
  `clan_id` int(11) unsigned NOT NULL DEFAULT 0,
  `opposition` int(11) unsigned NOT NULL DEFAULT 0,
  `alliance_id` int(11) unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  PRIMARY KEY (`clan_id`,`alliance_id`),
  KEY `alliance_id` (`alliance_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of clan_alliance
-- ----------------------------
INSERT INTO `clan_alliance` VALUES ('1', '0', '3', 'Golden Mace Clan');
INSERT INTO `clan_alliance` VALUES ('2', '0', '3', 'Golden Mace Clan');
INSERT INTO `clan_alliance` VALUES ('2', '1', '4', 'Crossbow Clan');
INSERT INTO `clan_alliance` VALUES ('3', '0', '1', 'Swordman Clan');
INSERT INTO `clan_alliance` VALUES ('3', '0', '2', 'Arcwand Clan');
INSERT INTO `clan_alliance` VALUES ('3', '0', '4', 'Crossbow Clan');
INSERT INTO `clan_alliance` VALUES ('4', '0', '3', 'Golden Mace Clan');
INSERT INTO `clan_alliance` VALUES ('4', '1', '2', 'Arcwand Clan');

-- ----------------------------
-- Table structure for db_roulette
-- ----------------------------
DROP TABLE IF EXISTS `db_roulette`;
CREATE TABLE `db_roulette` (
  `index` int(11) NOT NULL DEFAULT 0,
  `level` smallint(5) unsigned NOT NULL,
  `item_id` smallint(5) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL DEFAULT 1,
  `flag` smallint(5) unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of db_roulette
-- ----------------------------
INSERT INTO `db_roulette` VALUES ('0', '1', '675', '1', '1');
INSERT INTO `db_roulette` VALUES ('1', '1', '671', '1', '0');
INSERT INTO `db_roulette` VALUES ('2', '1', '678', '1', '0');
INSERT INTO `db_roulette` VALUES ('3', '1', '604', '1', '0');
INSERT INTO `db_roulette` VALUES ('4', '1', '522', '1', '0');
INSERT INTO `db_roulette` VALUES ('5', '1', '12609', '1', '0');
INSERT INTO `db_roulette` VALUES ('6', '1', '12523', '1', '0');
INSERT INTO `db_roulette` VALUES ('7', '1', '985', '1', '0');
INSERT INTO `db_roulette` VALUES ('8', '1', '984', '1', '0');
INSERT INTO `db_roulette` VALUES ('9', '2', '675', '1', '1');
INSERT INTO `db_roulette` VALUES ('10', '2', '671', '1', '0');
INSERT INTO `db_roulette` VALUES ('11', '2', '603', '1', '0');
INSERT INTO `db_roulette` VALUES ('12', '2', '608', '1', '0');
INSERT INTO `db_roulette` VALUES ('13', '2', '607', '1', '0');
INSERT INTO `db_roulette` VALUES ('14', '2', '12522', '1', '0');
INSERT INTO `db_roulette` VALUES ('15', '2', '6223', '1', '0');
INSERT INTO `db_roulette` VALUES ('16', '2', '6224', '1', '0');
INSERT INTO `db_roulette` VALUES ('17', '3', '675', '1', '1');
INSERT INTO `db_roulette` VALUES ('18', '3', '671', '1', '0');
INSERT INTO `db_roulette` VALUES ('19', '3', '12108', '1', '0');
INSERT INTO `db_roulette` VALUES ('20', '3', '617', '1', '0');
INSERT INTO `db_roulette` VALUES ('21', '3', '12514', '1', '0');
INSERT INTO `db_roulette` VALUES ('22', '3', '7444', '1', '0');
INSERT INTO `db_roulette` VALUES ('23', '3', '969', '1', '0');
INSERT INTO `db_roulette` VALUES ('24', '4', '675', '1', '1');
INSERT INTO `db_roulette` VALUES ('25', '4', '671', '1', '0');
INSERT INTO `db_roulette` VALUES ('26', '4', '616', '1', '0');
INSERT INTO `db_roulette` VALUES ('27', '4', '12516', '1', '0');
INSERT INTO `db_roulette` VALUES ('28', '4', '22777', '1', '0');
INSERT INTO `db_roulette` VALUES ('29', '4', '6231', '1', '0');
INSERT INTO `db_roulette` VALUES ('30', '5', '671', '1', '1');
INSERT INTO `db_roulette` VALUES ('31', '5', '12246', '1', '0');
INSERT INTO `db_roulette` VALUES ('32', '5', '12263', '1', '0');
INSERT INTO `db_roulette` VALUES ('33', '5', '12831', '1', '0');
INSERT INTO `db_roulette` VALUES ('34', '5', '6235', '1', '0');
INSERT INTO `db_roulette` VALUES ('35', '6', '671', '1', '1');
INSERT INTO `db_roulette` VALUES ('36', '6', '12766', '1', '0');
INSERT INTO `db_roulette` VALUES ('37', '6', '6234', '1', '0');
INSERT INTO `db_roulette` VALUES ('38', '6', '6233', '1', '0');
INSERT INTO `db_roulette` VALUES ('39', '7', '671', '1', '1');
INSERT INTO `db_roulette` VALUES ('40', '7', '6233', '1', '0');
INSERT INTO `db_roulette` VALUES ('41', '7', '6233', '1', '0');

-- ----------------------------
-- Table structure for elemental
-- ----------------------------
DROP TABLE IF EXISTS `elemental`;
CREATE TABLE `elemental` (
  `ele_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) unsigned NOT NULL DEFAULT 0,
  `mode` int(11) unsigned NOT NULL DEFAULT 1,
  `hp` int(11) unsigned NOT NULL DEFAULT 0,
  `sp` int(11) unsigned NOT NULL DEFAULT 0,
  `max_hp` int(11) unsigned NOT NULL DEFAULT 0,
  `max_sp` int(11) unsigned NOT NULL DEFAULT 0,
  `atk1` mediumint(6) unsigned NOT NULL DEFAULT 0,
  `atk2` mediumint(6) unsigned NOT NULL DEFAULT 0,
  `matk` mediumint(6) unsigned NOT NULL DEFAULT 0,
  `aspd` smallint(4) unsigned NOT NULL DEFAULT 0,
  `def` smallint(4) unsigned NOT NULL DEFAULT 0,
  `mdef` smallint(4) unsigned NOT NULL DEFAULT 0,
  `flee` smallint(4) unsigned NOT NULL DEFAULT 0,
  `hit` smallint(4) unsigned NOT NULL DEFAULT 0,
  `life_time` bigint(20) NOT NULL DEFAULT 0,
  PRIMARY KEY (`ele_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of elemental
-- ----------------------------

-- ----------------------------
-- Table structure for feedinglog
-- ----------------------------
DROP TABLE IF EXISTS `feedinglog`;
CREATE TABLE `feedinglog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int(11) NOT NULL,
  `target_id` int(11) NOT NULL,
  `target_class` smallint(11) NOT NULL,
  `type` enum('P','H','O') NOT NULL,
  `intimacy` int(11) unsigned NOT NULL,
  `item_id` smallint(5) unsigned NOT NULL,
  `map` varchar(11) NOT NULL,
  `x` smallint(5) unsigned NOT NULL,
  `y` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of feedinglog
-- ----------------------------

-- ----------------------------
-- Table structure for friends
-- ----------------------------
DROP TABLE IF EXISTS `friends`;
CREATE TABLE `friends` (
  `char_id` int(11) NOT NULL DEFAULT 0,
  `friend_id` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`friend_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of friends
-- ----------------------------

-- ----------------------------
-- Table structure for global_acc_reg_num
-- ----------------------------
DROP TABLE IF EXISTS `global_acc_reg_num`;
CREATE TABLE `global_acc_reg_num` (
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) CHARACTER SET latin1 COLLATE latin1_bin NOT NULL DEFAULT '',
  `index` int(11) unsigned NOT NULL DEFAULT 0,
  `value` bigint(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of global_acc_reg_num
-- ----------------------------

-- ----------------------------
-- Table structure for global_acc_reg_str
-- ----------------------------
DROP TABLE IF EXISTS `global_acc_reg_str`;
CREATE TABLE `global_acc_reg_str` (
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) CHARACTER SET latin1 COLLATE latin1_bin NOT NULL DEFAULT '',
  `index` int(11) unsigned NOT NULL DEFAULT 0,
  `value` varchar(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of global_acc_reg_str
-- ----------------------------

-- ----------------------------
-- Table structure for guild
-- ----------------------------
DROP TABLE IF EXISTS `guild`;
CREATE TABLE `guild` (
  `guild_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(24) NOT NULL DEFAULT '',
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `master` varchar(24) NOT NULL DEFAULT '',
  `guild_lv` tinyint(6) unsigned NOT NULL DEFAULT 0,
  `connect_member` tinyint(6) unsigned NOT NULL DEFAULT 0,
  `max_member` tinyint(6) unsigned NOT NULL DEFAULT 0,
  `average_lv` smallint(6) unsigned NOT NULL DEFAULT 1,
  `exp` bigint(20) unsigned NOT NULL DEFAULT 0,
  `next_exp` int(11) unsigned NOT NULL DEFAULT 0,
  `skill_point` tinyint(11) unsigned NOT NULL DEFAULT 0,
  `mes1` varchar(60) NOT NULL DEFAULT '',
  `mes2` varchar(120) NOT NULL DEFAULT '',
  `emblem_len` int(11) unsigned NOT NULL DEFAULT 0,
  `emblem_id` int(11) unsigned NOT NULL DEFAULT 0,
  `emblem_data` blob DEFAULT NULL,
  `last_master_change` datetime DEFAULT NULL,
  PRIMARY KEY (`guild_id`,`char_id`),
  UNIQUE KEY `guild_id` (`guild_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild
-- ----------------------------

-- ----------------------------
-- Table structure for guild_alliance
-- ----------------------------
DROP TABLE IF EXISTS `guild_alliance`;
CREATE TABLE `guild_alliance` (
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `opposition` int(11) unsigned NOT NULL DEFAULT 0,
  `alliance_id` int(11) unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  PRIMARY KEY (`guild_id`,`alliance_id`),
  KEY `alliance_id` (`alliance_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_alliance
-- ----------------------------

-- ----------------------------
-- Table structure for guild_castle
-- ----------------------------
DROP TABLE IF EXISTS `guild_castle`;
CREATE TABLE `guild_castle` (
  `castle_id` int(11) unsigned NOT NULL DEFAULT 0,
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `economy` int(11) unsigned NOT NULL DEFAULT 0,
  `defense` int(11) unsigned NOT NULL DEFAULT 0,
  `triggerE` int(11) unsigned NOT NULL DEFAULT 0,
  `triggerD` int(11) unsigned NOT NULL DEFAULT 0,
  `nextTime` int(11) unsigned NOT NULL DEFAULT 0,
  `payTime` int(11) unsigned NOT NULL DEFAULT 0,
  `createTime` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleC` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG0` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG1` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG2` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG3` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG4` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG5` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG6` int(11) unsigned NOT NULL DEFAULT 0,
  `visibleG7` int(11) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`castle_id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_castle
-- ----------------------------

-- ----------------------------
-- Table structure for guild_expulsion
-- ----------------------------
DROP TABLE IF EXISTS `guild_expulsion`;
CREATE TABLE `guild_expulsion` (
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `mes` varchar(40) NOT NULL DEFAULT '',
  PRIMARY KEY (`guild_id`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_expulsion
-- ----------------------------

-- ----------------------------
-- Table structure for guild_member
-- ----------------------------
DROP TABLE IF EXISTS `guild_member`;
CREATE TABLE `guild_member` (
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `exp` bigint(20) unsigned NOT NULL DEFAULT 0,
  `position` tinyint(6) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guild_id`,`char_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_member
-- ----------------------------

-- ----------------------------
-- Table structure for guild_position
-- ----------------------------
DROP TABLE IF EXISTS `guild_position`;
CREATE TABLE `guild_position` (
  `guild_id` int(9) unsigned NOT NULL DEFAULT 0,
  `position` tinyint(6) unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `mode` smallint(11) unsigned NOT NULL DEFAULT 0,
  `exp_mode` tinyint(11) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guild_id`,`position`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_position
-- ----------------------------

-- ----------------------------
-- Table structure for guild_skill
-- ----------------------------
DROP TABLE IF EXISTS `guild_skill`;
CREATE TABLE `guild_skill` (
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `id` smallint(11) unsigned NOT NULL DEFAULT 0,
  `lv` tinyint(11) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guild_id`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_skill
-- ----------------------------

-- ----------------------------
-- Table structure for guild_storage
-- ----------------------------
DROP TABLE IF EXISTS `guild_storage`;
CREATE TABLE `guild_storage` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `amount` int(11) unsigned NOT NULL DEFAULT 0,
  `equip` int(11) unsigned NOT NULL DEFAULT 0,
  `identify` smallint(6) unsigned NOT NULL DEFAULT 0,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `expire_time` int(11) unsigned NOT NULL DEFAULT 0,
  `bound` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_storage
-- ----------------------------

-- ----------------------------
-- Table structure for guild_storage_log
-- ----------------------------
DROP TABLE IF EXISTS `guild_storage_log`;
CREATE TABLE `guild_storage_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `guild_id` int(11) unsigned NOT NULL DEFAULT 0,
  `time` datetime NOT NULL,
  `char_id` int(11) NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `amount` int(11) NOT NULL DEFAULT 1,
  `identify` smallint(6) NOT NULL DEFAULT 0,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `expire_time` int(11) unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  `bound` tinyint(1) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of guild_storage_log
-- ----------------------------

-- ----------------------------
-- Table structure for homunculus
-- ----------------------------
DROP TABLE IF EXISTS `homunculus`;
CREATE TABLE `homunculus` (
  `homun_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) unsigned NOT NULL DEFAULT 0,
  `prev_class` mediumint(9) NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `level` smallint(4) NOT NULL DEFAULT 0,
  `exp` bigint(20) unsigned NOT NULL DEFAULT 0,
  `intimacy` int(12) NOT NULL DEFAULT 0,
  `hunger` smallint(4) NOT NULL DEFAULT 0,
  `str` smallint(4) unsigned NOT NULL DEFAULT 0,
  `agi` smallint(4) unsigned NOT NULL DEFAULT 0,
  `vit` smallint(4) unsigned NOT NULL DEFAULT 0,
  `int` smallint(4) unsigned NOT NULL DEFAULT 0,
  `dex` smallint(4) unsigned NOT NULL DEFAULT 0,
  `luk` smallint(4) unsigned NOT NULL DEFAULT 0,
  `hp` int(11) unsigned NOT NULL DEFAULT 0,
  `max_hp` int(11) unsigned NOT NULL DEFAULT 0,
  `sp` int(11) NOT NULL DEFAULT 0,
  `max_sp` int(11) NOT NULL DEFAULT 0,
  `skill_point` smallint(4) unsigned NOT NULL DEFAULT 0,
  `alive` tinyint(2) NOT NULL DEFAULT 1,
  `rename_flag` tinyint(2) NOT NULL DEFAULT 0,
  `vaporize` tinyint(2) NOT NULL DEFAULT 0,
  `autofeed` tinyint(2) NOT NULL DEFAULT 0,
  PRIMARY KEY (`homun_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of homunculus
-- ----------------------------

-- ----------------------------
-- Table structure for hotkey
-- ----------------------------
DROP TABLE IF EXISTS `hotkey`;
CREATE TABLE `hotkey` (
  `char_id` int(11) NOT NULL,
  `hotkey` tinyint(2) unsigned NOT NULL,
  `type` tinyint(1) unsigned NOT NULL DEFAULT 0,
  `itemskill_id` int(11) unsigned NOT NULL DEFAULT 0,
  `skill_lvl` tinyint(4) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`hotkey`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of hotkey
-- ----------------------------

-- ----------------------------
-- Table structure for interlog
-- ----------------------------
DROP TABLE IF EXISTS `interlog`;
CREATE TABLE `interlog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `log` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of interlog
-- ----------------------------

-- ----------------------------
-- Table structure for interreg
-- ----------------------------
DROP TABLE IF EXISTS `interreg`;
CREATE TABLE `interreg` (
  `varname` varchar(11) NOT NULL,
  `value` varchar(20) NOT NULL,
  PRIMARY KEY (`varname`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of interreg
-- ----------------------------

-- ----------------------------
-- Table structure for inventory
-- ----------------------------
DROP TABLE IF EXISTS `inventory`;
CREATE TABLE `inventory` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `amount` int(11) unsigned NOT NULL DEFAULT 0,
  `equip` int(11) unsigned NOT NULL DEFAULT 0,
  `identify` smallint(6) NOT NULL DEFAULT 0,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `expire_time` int(11) unsigned NOT NULL DEFAULT 0,
  `favorite` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `bound` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  `equip_switch` int(11) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of inventory
-- ----------------------------

-- ----------------------------
-- Table structure for ipbanlist
-- ----------------------------
DROP TABLE IF EXISTS `ipbanlist`;
CREATE TABLE `ipbanlist` (
  `list` varchar(15) NOT NULL DEFAULT '',
  `btime` datetime NOT NULL,
  `rtime` datetime NOT NULL,
  `reason` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`list`,`btime`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of ipbanlist
-- ----------------------------

-- ----------------------------
-- Table structure for login
-- ----------------------------
DROP TABLE IF EXISTS `login`;
CREATE TABLE `login` (
  `account_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `userid` varchar(23) NOT NULL DEFAULT '',
  `user_pass` varchar(32) NOT NULL DEFAULT '',
  `sex` enum('M','F','S') NOT NULL DEFAULT 'M',
  `email` varchar(39) NOT NULL DEFAULT '',
  `group_id` tinyint(3) NOT NULL DEFAULT 0,
  `state` int(11) unsigned NOT NULL DEFAULT 0,
  `unban_time` int(11) unsigned NOT NULL DEFAULT 0,
  `expiration_time` int(11) unsigned NOT NULL DEFAULT 0,
  `logincount` mediumint(9) unsigned NOT NULL DEFAULT 0,
  `lastlogin` datetime DEFAULT NULL,
  `last_ip` varchar(100) NOT NULL DEFAULT '',
  `birthdate` date DEFAULT NULL,
  `character_slots` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `pincode` varchar(4) NOT NULL DEFAULT '',
  `pincode_change` int(11) unsigned NOT NULL DEFAULT 0,
  `vip_time` int(11) unsigned NOT NULL DEFAULT 0,
  `old_group` tinyint(3) NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`),
  KEY `name` (`userid`)
) ENGINE=MyISAM AUTO_INCREMENT=2000000 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of login
-- ----------------------------
INSERT INTO `login` VALUES ('1', 'starttemplateid', 'starttemplatepw', 'S', 'athena@athena.com', '0', '0', '0', '0', '0', null, '', null, '0', '', '0', '0', '0');

-- ----------------------------
-- Table structure for loginlog
-- ----------------------------
DROP TABLE IF EXISTS `loginlog`;
CREATE TABLE `loginlog` (
  `time` datetime NOT NULL,
  `ip` varchar(15) NOT NULL DEFAULT '',
  `user` varchar(23) NOT NULL DEFAULT '',
  `rcode` tinyint(4) NOT NULL DEFAULT 0,
  `log` varchar(255) NOT NULL DEFAULT '',
  KEY `ip` (`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of loginlog
-- ----------------------------

-- ----------------------------
-- Table structure for mail
-- ----------------------------
DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `send_name` varchar(30) NOT NULL DEFAULT '',
  `send_id` int(11) unsigned NOT NULL DEFAULT 0,
  `dest_name` varchar(30) NOT NULL DEFAULT '',
  `dest_id` int(11) unsigned NOT NULL DEFAULT 0,
  `title` varchar(45) NOT NULL DEFAULT '',
  `message` varchar(500) NOT NULL DEFAULT '',
  `time` int(11) unsigned NOT NULL DEFAULT 0,
  `status` tinyint(2) NOT NULL DEFAULT 0,
  `zeny` int(11) unsigned NOT NULL DEFAULT 0,
  `type` smallint(5) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of mail
-- ----------------------------

-- ----------------------------
-- Table structure for mail_attachments
-- ----------------------------
DROP TABLE IF EXISTS `mail_attachments`;
CREATE TABLE `mail_attachments` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `index` smallint(5) unsigned NOT NULL DEFAULT 0,
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `amount` int(11) unsigned NOT NULL DEFAULT 0,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `identify` smallint(6) NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  `bound` tinyint(1) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`,`index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of mail_attachments
-- ----------------------------

-- ----------------------------
-- Table structure for mapreg
-- ----------------------------
DROP TABLE IF EXISTS `mapreg`;
CREATE TABLE `mapreg` (
  `varname` varchar(32) CHARACTER SET latin1 COLLATE latin1_bin NOT NULL,
  `index` int(11) unsigned NOT NULL DEFAULT 0,
  `value` varchar(255) NOT NULL,
  PRIMARY KEY (`varname`,`index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of mapreg
-- ----------------------------

-- ----------------------------
-- Table structure for market
-- ----------------------------
DROP TABLE IF EXISTS `market`;
CREATE TABLE `market` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `nameid` smallint(5) unsigned NOT NULL,
  `price` int(11) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL,
  `flag` tinyint(2) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`name`,`nameid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of market
-- ----------------------------

-- ----------------------------
-- Table structure for memo
-- ----------------------------
DROP TABLE IF EXISTS `memo`;
CREATE TABLE `memo` (
  `memo_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  `x` smallint(4) unsigned NOT NULL DEFAULT 0,
  `y` smallint(4) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`memo_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of memo
-- ----------------------------

-- ----------------------------
-- Table structure for mercenary
-- ----------------------------
DROP TABLE IF EXISTS `mercenary`;
CREATE TABLE `mercenary` (
  `mer_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) unsigned NOT NULL DEFAULT 0,
  `hp` int(11) unsigned NOT NULL DEFAULT 0,
  `sp` int(11) unsigned NOT NULL DEFAULT 0,
  `kill_counter` int(11) NOT NULL,
  `life_time` bigint(20) NOT NULL DEFAULT 0,
  PRIMARY KEY (`mer_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of mercenary
-- ----------------------------

-- ----------------------------
-- Table structure for mercenary_owner
-- ----------------------------
DROP TABLE IF EXISTS `mercenary_owner`;
CREATE TABLE `mercenary_owner` (
  `char_id` int(11) NOT NULL,
  `merc_id` int(11) NOT NULL DEFAULT 0,
  `arch_calls` int(11) NOT NULL DEFAULT 0,
  `arch_faith` int(11) NOT NULL DEFAULT 0,
  `spear_calls` int(11) NOT NULL DEFAULT 0,
  `spear_faith` int(11) NOT NULL DEFAULT 0,
  `sword_calls` int(11) NOT NULL DEFAULT 0,
  `sword_faith` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of mercenary_owner
-- ----------------------------

-- ----------------------------
-- Table structure for mvplog
-- ----------------------------
DROP TABLE IF EXISTS `mvplog`;
CREATE TABLE `mvplog` (
  `mvp_id` mediumint(9) unsigned NOT NULL AUTO_INCREMENT,
  `mvp_date` datetime NOT NULL,
  `kill_char_id` int(11) NOT NULL DEFAULT 0,
  `monster_id` smallint(6) NOT NULL DEFAULT 0,
  `prize` smallint(5) unsigned NOT NULL DEFAULT 0,
  `mvpexp` bigint(20) unsigned NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`mvp_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of mvplog
-- ----------------------------

-- ----------------------------
-- Table structure for npclog
-- ----------------------------
DROP TABLE IF EXISTS `npclog`;
CREATE TABLE `npclog` (
  `npc_id` mediumint(9) unsigned NOT NULL AUTO_INCREMENT,
  `npc_date` datetime NOT NULL,
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `char_name` varchar(25) NOT NULL DEFAULT '',
  `map` varchar(11) NOT NULL DEFAULT '',
  `mes` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`npc_id`),
  KEY `account_id` (`account_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of npclog
-- ----------------------------

-- ----------------------------
-- Table structure for party
-- ----------------------------
DROP TABLE IF EXISTS `party`;
CREATE TABLE `party` (
  `party_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(24) NOT NULL DEFAULT '',
  `exp` tinyint(11) unsigned NOT NULL DEFAULT 0,
  `item` tinyint(11) unsigned NOT NULL DEFAULT 0,
  `leader_id` int(11) unsigned NOT NULL DEFAULT 0,
  `leader_char` int(11) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`party_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of party
-- ----------------------------

-- ----------------------------
-- Table structure for pet
-- ----------------------------
DROP TABLE IF EXISTS `pet`;
CREATE TABLE `pet` (
  `pet_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `class` mediumint(9) unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `level` smallint(4) unsigned NOT NULL DEFAULT 0,
  `egg_id` smallint(5) unsigned NOT NULL DEFAULT 0,
  `equip` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `intimate` smallint(9) unsigned NOT NULL DEFAULT 0,
  `hungry` smallint(9) unsigned NOT NULL DEFAULT 0,
  `rename_flag` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `incubate` int(11) unsigned NOT NULL DEFAULT 0,
  `autofeed` tinyint(2) NOT NULL DEFAULT 0,
  PRIMARY KEY (`pet_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of pet
-- ----------------------------

-- ----------------------------
-- Table structure for picklog
-- ----------------------------
DROP TABLE IF EXISTS `picklog`;
CREATE TABLE `picklog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int(11) NOT NULL DEFAULT 0,
  `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U','$','F','Y','Z','Q','H') NOT NULL DEFAULT 'P',
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `amount` int(11) NOT NULL DEFAULT 1,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  `bound` tinyint(1) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `type` (`type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of picklog
-- ----------------------------

-- ----------------------------
-- Table structure for quest
-- ----------------------------
DROP TABLE IF EXISTS `quest`;
CREATE TABLE `quest` (
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `quest_id` int(10) unsigned NOT NULL,
  `state` enum('0','1','2') NOT NULL DEFAULT '0',
  `time` int(11) unsigned NOT NULL DEFAULT 0,
  `count1` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count2` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `count3` mediumint(8) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`quest_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of quest
-- ----------------------------

-- ----------------------------
-- Table structure for sales
-- ----------------------------
DROP TABLE IF EXISTS `sales`;
CREATE TABLE `sales` (
  `nameid` smallint(5) unsigned NOT NULL,
  `start` datetime NOT NULL,
  `end` datetime NOT NULL,
  `amount` int(11) NOT NULL,
  PRIMARY KEY (`nameid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of sales
-- ----------------------------

-- ----------------------------
-- Table structure for sc_data
-- ----------------------------
DROP TABLE IF EXISTS `sc_data`;
CREATE TABLE `sc_data` (
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(11) unsigned NOT NULL,
  `type` smallint(11) unsigned NOT NULL,
  `tick` bigint(20) NOT NULL,
  `val1` int(11) NOT NULL DEFAULT 0,
  `val2` int(11) NOT NULL DEFAULT 0,
  `val3` int(11) NOT NULL DEFAULT 0,
  `val4` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of sc_data
-- ----------------------------

-- ----------------------------
-- Table structure for skill
-- ----------------------------
DROP TABLE IF EXISTS `skill`;
CREATE TABLE `skill` (
  `char_id` int(11) unsigned NOT NULL DEFAULT 0,
  `id` smallint(11) unsigned NOT NULL DEFAULT 0,
  `lv` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `flag` tinyint(1) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of skill
-- ----------------------------

-- ----------------------------
-- Table structure for skillcooldown
-- ----------------------------
DROP TABLE IF EXISTS `skillcooldown`;
CREATE TABLE `skillcooldown` (
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(11) unsigned NOT NULL,
  `skill` smallint(11) unsigned NOT NULL DEFAULT 0,
  `tick` bigint(20) NOT NULL,
  PRIMARY KEY (`char_id`,`skill`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of skillcooldown
-- ----------------------------

-- ----------------------------
-- Table structure for skill_homunculus
-- ----------------------------
DROP TABLE IF EXISTS `skill_homunculus`;
CREATE TABLE `skill_homunculus` (
  `homun_id` int(11) NOT NULL,
  `id` int(11) NOT NULL,
  `lv` smallint(6) NOT NULL,
  PRIMARY KEY (`homun_id`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of skill_homunculus
-- ----------------------------

-- ----------------------------
-- Table structure for storage
-- ----------------------------
DROP TABLE IF EXISTS `storage`;
CREATE TABLE `storage` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int(11) unsigned NOT NULL DEFAULT 0,
  `nameid` smallint(5) unsigned NOT NULL DEFAULT 0,
  `amount` smallint(11) unsigned NOT NULL DEFAULT 0,
  `equip` int(11) unsigned NOT NULL DEFAULT 0,
  `identify` smallint(6) unsigned NOT NULL DEFAULT 0,
  `refine` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `card0` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card1` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card2` smallint(5) unsigned NOT NULL DEFAULT 0,
  `card3` smallint(5) unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint(5) NOT NULL DEFAULT 0,
  `option_val0` smallint(5) NOT NULL DEFAULT 0,
  `option_parm0` tinyint(3) NOT NULL DEFAULT 0,
  `option_id1` smallint(5) NOT NULL DEFAULT 0,
  `option_val1` smallint(5) NOT NULL DEFAULT 0,
  `option_parm1` tinyint(3) NOT NULL DEFAULT 0,
  `option_id2` smallint(5) NOT NULL DEFAULT 0,
  `option_val2` smallint(5) NOT NULL DEFAULT 0,
  `option_parm2` tinyint(3) NOT NULL DEFAULT 0,
  `option_id3` smallint(5) NOT NULL DEFAULT 0,
  `option_val3` smallint(5) NOT NULL DEFAULT 0,
  `option_parm3` tinyint(3) NOT NULL DEFAULT 0,
  `option_id4` smallint(5) NOT NULL DEFAULT 0,
  `option_val4` smallint(5) NOT NULL DEFAULT 0,
  `option_parm4` tinyint(3) NOT NULL DEFAULT 0,
  `expire_time` int(11) unsigned NOT NULL DEFAULT 0,
  `bound` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of storage
-- ----------------------------

-- ----------------------------
-- Table structure for vendings
-- ----------------------------
DROP TABLE IF EXISTS `vendings`;
CREATE TABLE `vendings` (
  `id` int(10) unsigned NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(10) unsigned NOT NULL,
  `sex` enum('F','M') NOT NULL DEFAULT 'M',
  `map` varchar(20) NOT NULL,
  `x` smallint(5) unsigned NOT NULL,
  `y` smallint(5) unsigned NOT NULL,
  `title` varchar(80) NOT NULL,
  `body_direction` char(1) NOT NULL DEFAULT '4',
  `head_direction` char(1) NOT NULL DEFAULT '0',
  `sit` char(1) NOT NULL DEFAULT '1',
  `autotrade` tinyint(4) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of vendings
-- ----------------------------

-- ----------------------------
-- Table structure for vending_items
-- ----------------------------
DROP TABLE IF EXISTS `vending_items`;
CREATE TABLE `vending_items` (
  `vending_id` int(10) unsigned NOT NULL,
  `index` smallint(5) unsigned NOT NULL,
  `cartinventory_id` int(10) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL,
  `price` int(10) unsigned NOT NULL,
  PRIMARY KEY (`vending_id`,`index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of vending_items
-- ----------------------------

-- ----------------------------
-- Table structure for zenylog
-- ----------------------------
DROP TABLE IF EXISTS `zenylog`;
CREATE TABLE `zenylog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int(11) NOT NULL DEFAULT 0,
  `src_id` int(11) NOT NULL DEFAULT 0,
  `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','K') NOT NULL DEFAULT 'S',
  `amount` int(11) NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `type` (`type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of zenylog
-- ----------------------------
