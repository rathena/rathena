--
-- Table structure for table `acc_reg_num`
--

CREATE TABLE IF NOT EXISTS `acc_reg_num` (
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) binary NOT NULL DEFAULT '',
  `index` int unsigned NOT NULL DEFAULT 0,
  `value` bigint NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `acc_reg_str`
--

CREATE TABLE IF NOT EXISTS `acc_reg_str` (
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) binary NOT NULL DEFAULT '',
  `index` int unsigned NOT NULL DEFAULT 0,
  `value` varchar(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `achievement`
--

CREATE TABLE IF NOT EXISTS `achievement` (
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `id` bigint unsigned NOT NULL,
  `count1` int unsigned NOT NULL DEFAULT 0,
  `count2` int unsigned NOT NULL DEFAULT 0,
  `count3` int unsigned NOT NULL DEFAULT 0,
  `count4` int unsigned NOT NULL DEFAULT 0,
  `count5` int unsigned NOT NULL DEFAULT 0,
  `count6` int unsigned NOT NULL DEFAULT 0,
  `count7` int unsigned NOT NULL DEFAULT 0,
  `count8` int unsigned NOT NULL DEFAULT 0,
  `count9` int unsigned NOT NULL DEFAULT 0,
  `count10` int unsigned NOT NULL DEFAULT 0,
  `completed` datetime,
  `rewarded` datetime,
  PRIMARY KEY (`char_id`,`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `auction`
--

CREATE TABLE IF NOT EXISTS `auction` (
  `auction_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `seller_id` int unsigned NOT NULL DEFAULT 0,
  `seller_name` varchar(30) NOT NULL DEFAULT '',
  `buyer_id` int unsigned NOT NULL DEFAULT 0,
  `buyer_name` varchar(30) NOT NULL DEFAULT '',
  `price` int unsigned NOT NULL DEFAULT 0,
  `buynow` int unsigned NOT NULL DEFAULT 0,
  `hours` smallint NOT NULL DEFAULT 0,
  `timestamp` int unsigned NOT NULL DEFAULT 0,
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `item_name` varchar(50) NOT NULL DEFAULT '',
  `type` smallint NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`auction_id`)
) ENGINE=MyISAM;

--
-- Table `barter` for barter shop persistency
--

CREATE TABLE IF NOT EXISTS `barter` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `index` smallint unsigned NOT NULL,
  `amount` smallint unsigned NOT NULL,
  PRIMARY KEY  (`name`,`index`)
) ENGINE=MyISAM;

--
-- Table structure for `db_roulette`
--

CREATE TABLE IF NOT EXISTS `db_roulette` (
  `index` int NOT NULL DEFAULT 0,
  `level` smallint unsigned NOT NULL,
  `item_id` int unsigned NOT NULL,
  `amount` smallint unsigned NOT NULL DEFAULT 1,
  `flag` smallint unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`index`)
) ENGINE=MyISAM;

--
-- Table structure for table `bonus_script`
--

CREATE TABLE IF NOT EXISTS `bonus_script` (
  `char_id` int unsigned NOT NULL,
  `script` text NOT NULL,
  `tick` bigint NOT NULL DEFAULT 0,
  `flag` smallint unsigned NOT NULL DEFAULT 0,
  `type` tinyint unsigned NOT NULL DEFAULT 0,
  `icon` smallint NOT NULL DEFAULT -1,
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB;

--
-- Table structure for table `buyingstore_items`
--

CREATE TABLE IF NOT EXISTS `buyingstore_items` (
  `buyingstore_id` int unsigned NOT NULL,
  `index` smallint unsigned NOT NULL,
  `item_id` int unsigned NOT NULL,
  `amount` smallint unsigned NOT NULL,
  `price` int unsigned NOT NULL,
  PRIMARY KEY (`buyingstore_id`, `index`)
) ENGINE=MyISAM;

--
-- Table structure for table `buyingstores`
--

CREATE TABLE IF NOT EXISTS `buyingstores` (
  `id` int unsigned NOT NULL,
  `account_id` int unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `sex` enum('F','M') NOT NULL DEFAULT 'M',
  `map` varchar(20) NOT NULL,
  `x` smallint unsigned NOT NULL,
  `y` smallint unsigned NOT NULL,
  `title` varchar(80) NOT NULL,
  `limit` int unsigned NOT NULL,
  `body_direction` char(1) NOT NULL DEFAULT '4',
  `head_direction` char(1) NOT NULL DEFAULT '0',
  `sit` char(1) NOT NULL DEFAULT '1',
  `autotrade` tinyint NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `cart_inventory`
--

CREATE TABLE IF NOT EXISTS `cart_inventory` (
  `id` int NOT NULL AUTO_INCREMENT,
  `char_id` int NOT NULL DEFAULT 0,
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `amount` int NOT NULL DEFAULT 0,
  `equip` int unsigned NOT NULL DEFAULT 0,
  `identify` smallint NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint NOT NULL DEFAULT 0,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `char`
--

CREATE TABLE IF NOT EXISTS `char` (
  `char_id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `char_num` tinyint NOT NULL DEFAULT 0,
  `name` varchar(30) NOT NULL DEFAULT '',
  `class` smallint unsigned NOT NULL DEFAULT 0,
  `base_level` smallint unsigned NOT NULL DEFAULT 1,
  `job_level` smallint unsigned NOT NULL DEFAULT 1,
  `base_exp` bigint unsigned NOT NULL DEFAULT 0,
  `job_exp` bigint unsigned NOT NULL DEFAULT 0,
  `zeny` int unsigned NOT NULL DEFAULT 0,
  `str` smallint unsigned NOT NULL DEFAULT 0,
  `agi` smallint unsigned NOT NULL DEFAULT 0,
  `vit` smallint unsigned NOT NULL DEFAULT 0,
  `int` smallint unsigned NOT NULL DEFAULT 0,
  `dex` smallint unsigned NOT NULL DEFAULT 0,
  `luk` smallint unsigned NOT NULL DEFAULT 0,
  `pow` smallint unsigned NOT NULL DEFAULT 0,
  `sta` smallint unsigned NOT NULL DEFAULT 0,
  `wis` smallint unsigned NOT NULL DEFAULT 0,
  `spl` smallint unsigned NOT NULL DEFAULT 0,
  `con` smallint unsigned NOT NULL DEFAULT 0,
  `crt` smallint unsigned NOT NULL DEFAULT 0,
  `max_hp` int unsigned NOT NULL DEFAULT 0,
  `hp` int unsigned NOT NULL DEFAULT 0,
  `max_sp` int unsigned NOT NULL DEFAULT 0,
  `sp` int unsigned NOT NULL DEFAULT 0,
  `max_ap` int unsigned NOT NULL DEFAULT 0,
  `ap` int unsigned NOT NULL DEFAULT 0,
  `status_point` int unsigned NOT NULL DEFAULT 0,
  `skill_point` int unsigned NOT NULL DEFAULT 0,
  `trait_point` int unsigned NOT NULL DEFAULT 0,
  `option` int NOT NULL DEFAULT 0,
  `karma` tinyint NOT NULL DEFAULT 0,
  `manner` smallint NOT NULL DEFAULT 0,
  `party_id` int unsigned NOT NULL DEFAULT 0,
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `pet_id` int unsigned NOT NULL DEFAULT 0,
  `homun_id` int unsigned NOT NULL DEFAULT 0,
  `elemental_id` int unsigned NOT NULL DEFAULT 0,
  `hair` tinyint unsigned NOT NULL DEFAULT 0,
  `hair_color` smallint unsigned NOT NULL DEFAULT 0,
  `clothes_color` smallint unsigned NOT NULL DEFAULT 0,
  `body` smallint unsigned NOT NULL DEFAULT 0,
  `weapon` smallint unsigned NOT NULL DEFAULT 0,
  `shield` smallint unsigned NOT NULL DEFAULT 0,
  `head_top` smallint unsigned NOT NULL DEFAULT 0,
  `head_mid` smallint unsigned NOT NULL DEFAULT 0,
  `head_bottom` smallint unsigned NOT NULL DEFAULT 0,
  `robe` smallint unsigned NOT NULL DEFAULT 0,
  `last_map` varchar(11) NOT NULL DEFAULT '',
  `last_x` smallint unsigned NOT NULL DEFAULT 53,
  `last_y` smallint unsigned NOT NULL DEFAULT 111,
  `last_instanceid` int unsigned NOT NULL DEFAULT 0,
  `save_map` varchar(11) NOT NULL DEFAULT '',
  `save_x` smallint unsigned NOT NULL DEFAULT 53,
  `save_y` smallint unsigned NOT NULL DEFAULT 111,
  `partner_id` int unsigned NOT NULL DEFAULT 0,
  `online` tinyint NOT NULL DEFAULT 0,
  `father` int unsigned NOT NULL DEFAULT 0,
  `mother` int unsigned NOT NULL DEFAULT 0,
  `child` int unsigned NOT NULL DEFAULT 0,
  `fame` int unsigned NOT NULL DEFAULT 0,
  `rename` smallint unsigned NOT NULL DEFAULT 0,
  `delete_date` int unsigned NOT NULL DEFAULT 0,
  `moves` int unsigned NOT NULL DEFAULT 0,
  `unban_time` int unsigned NOT NULL DEFAULT 0,
  `font` tinyint unsigned NOT NULL DEFAULT 0,
  `uniqueitem_counter` int unsigned NOT NULL DEFAULT 0,
  `sex` ENUM('M','F') NOT NULL,
  `hotkey_rowshift` tinyint unsigned NOT NULL DEFAULT 0,
  `hotkey_rowshift2` tinyint unsigned NOT NULL DEFAULT 0,
  `clan_id` int unsigned NOT NULL DEFAULT 0,
  `last_login` datetime DEFAULT NULL,
  `title_id` int unsigned NOT NULL DEFAULT 0,
  `show_equip` tinyint unsigned NOT NULL DEFAULT 0,
  `inventory_slots` smallint NOT NULL DEFAULT 100,
  `body_direction` tinyint unsigned NOT NULL DEFAULT 0,
  `disable_call` tinyint unsigned NOT NULL DEFAULT 0,
  `disable_partyinvite` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`char_id`),
  UNIQUE KEY `name_key` (`name`),
  KEY `account_id` (`account_id`),
  KEY `party_id` (`party_id`),
  KEY `guild_id` (`guild_id`),
  KEY `online` (`online`)
) ENGINE=MyISAM AUTO_INCREMENT=150000; 

--
-- Table structure for table `char_reg_num`
--

CREATE TABLE IF NOT EXISTS `char_reg_num` (
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) binary NOT NULL DEFAULT '',
  `index` int unsigned NOT NULL DEFAULT 0,
  `value` bigint NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `char_reg_str`
--

CREATE TABLE IF NOT EXISTS `char_reg_str` (
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) binary NOT NULL DEFAULT '',
  `index` int unsigned NOT NULL DEFAULT 0,
  `value` varchar(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `charlog`
--

CREATE TABLE IF NOT EXISTS `charlog` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_msg` varchar(255) NOT NULL DEFAULT 'char select',
  `account_id` int NOT NULL DEFAULT 0,
  `char_num` tinyint NOT NULL DEFAULT 0,
  `name` varchar(23) NOT NULL DEFAULT '',
  `str` int unsigned NOT NULL DEFAULT 0,
  `agi` int unsigned NOT NULL DEFAULT 0,
  `vit` int unsigned NOT NULL DEFAULT 0,
  `int` int unsigned NOT NULL DEFAULT 0,
  `dex` int unsigned NOT NULL DEFAULT 0,
  `luk` int unsigned NOT NULL DEFAULT 0,
  `hair` tinyint NOT NULL DEFAULT 0,
  `hair_color` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM; 

--
-- Table structure for table `clan`
--

CREATE TABLE IF NOT EXISTS `clan` (
  `clan_id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(24) NOT NULL DEFAULT '',
  `master` varchar(24) NOT NULL DEFAULT '',
  `mapname` varchar(24) NOT NULL DEFAULT '',
  `max_member` smallint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`clan_id`)
) ENGINE=MyISAM AUTO_INCREMENT=5;

-- ----------------------------
-- Records of clan
-- ----------------------------

INSERT INTO `clan` VALUES ('1', 'Swordman Clan', 'Raffam Oranpere', 'prontera', '500');
INSERT INTO `clan` VALUES ('2', 'Arcwand Clan', 'Devon Aire', 'geffen', '500');
INSERT INTO `clan` VALUES ('3', 'Golden Mace Clan', 'Berman Aire', 'prontera', '500');
INSERT INTO `clan` VALUES ('4', 'Crossbow Clan', 'Shaam Rumi', 'payon', '500');

--
-- Table structure for `clan_alliance`
--

CREATE TABLE IF NOT EXISTS `clan_alliance` (
  `clan_id` int unsigned NOT NULL DEFAULT 0,
  `opposition` int unsigned NOT NULL DEFAULT 0,
  `alliance_id` int unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  PRIMARY KEY (`clan_id`,`alliance_id`),
  KEY `alliance_id` (`alliance_id`)
) ENGINE=MyISAM;

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

--
-- Table structure for table `elemental`
--

CREATE TABLE IF NOT EXISTS `elemental` (
  `ele_id` int unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int NOT NULL,
  `class` mediumint unsigned NOT NULL DEFAULT 0,
  `mode` int unsigned NOT NULL DEFAULT 1,
  `hp` int unsigned NOT NULL DEFAULT 0,
  `sp` int unsigned NOT NULL DEFAULT 0,
  `max_hp` int unsigned NOT NULL DEFAULT 0,
  `max_sp` int unsigned NOT NULL DEFAULT 0,
  `atk1` mediumint unsigned NOT NULL DEFAULT 0,
  `atk2` mediumint unsigned NOT NULL DEFAULT 0,
  `matk` mediumint unsigned NOT NULL DEFAULT 0,
  `aspd` smallint unsigned NOT NULL DEFAULT 0,
  `def` smallint unsigned NOT NULL DEFAULT 0,
  `mdef` smallint unsigned NOT NULL DEFAULT 0,
  `flee` smallint unsigned NOT NULL DEFAULT 0,
  `hit` smallint unsigned NOT NULL DEFAULT 0,
  `life_time` bigint NOT NULL DEFAULT 0,
  PRIMARY KEY  (`ele_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `friends`
--

CREATE TABLE IF NOT EXISTS `friends` (
  `char_id` int NOT NULL DEFAULT 0,
  `friend_id` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`, `friend_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `global_acc_reg_num`
--

CREATE TABLE IF NOT EXISTS `global_acc_reg_num` (
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) binary NOT NULL DEFAULT '',
  `index` int unsigned NOT NULL DEFAULT 0,
  `value` bigint NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `global_acc_reg_str`
--

CREATE TABLE IF NOT EXISTS `global_acc_reg_str` (
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `key` varchar(32) binary NOT NULL DEFAULT '',
  `index` int unsigned NOT NULL DEFAULT 0,
  `value` varchar(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild`
--

CREATE TABLE IF NOT EXISTS `guild` (
  `guild_id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(24) NOT NULL DEFAULT '',
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `master` varchar(24) NOT NULL DEFAULT '',
  `guild_lv` tinyint unsigned NOT NULL DEFAULT 0,
  `connect_member` tinyint unsigned NOT NULL DEFAULT 0,
  `max_member` tinyint unsigned NOT NULL DEFAULT 0,
  `average_lv` smallint unsigned NOT NULL DEFAULT 1,
  `exp` bigint unsigned NOT NULL DEFAULT 0,
  `next_exp` bigint unsigned NOT NULL DEFAULT 0,
  `skill_point` tinyint unsigned NOT NULL DEFAULT 0,
  `mes1` varchar(60) NOT NULL DEFAULT '',
  `mes2` varchar(120) NOT NULL DEFAULT '',
  `emblem_len` int unsigned NOT NULL DEFAULT 0,
  `emblem_id` int unsigned NOT NULL DEFAULT 0,
  `emblem_data` blob,
  `last_master_change` datetime,
  PRIMARY KEY  (`guild_id`,`char_id`),
  UNIQUE KEY `guild_id` (`guild_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_alliance`
--

CREATE TABLE IF NOT EXISTS `guild_alliance` (
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `opposition` int unsigned NOT NULL DEFAULT 0,
  `alliance_id` int unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  PRIMARY KEY  (`guild_id`,`alliance_id`),
  KEY `alliance_id` (`alliance_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_castle`
--

CREATE TABLE IF NOT EXISTS `guild_castle` (
  `castle_id` int unsigned NOT NULL DEFAULT 0,
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `economy` int unsigned NOT NULL DEFAULT 0,
  `defense` int unsigned NOT NULL DEFAULT 0,
  `triggerE` int unsigned NOT NULL DEFAULT 0,
  `triggerD` int unsigned NOT NULL DEFAULT 0,
  `nextTime` int unsigned NOT NULL DEFAULT 0,
  `payTime` int unsigned NOT NULL DEFAULT 0,
  `createTime` int unsigned NOT NULL DEFAULT 0,
  `visibleC` int unsigned NOT NULL DEFAULT 0,
  `visibleG0` int unsigned NOT NULL DEFAULT 0,
  `visibleG1` int unsigned NOT NULL DEFAULT 0,
  `visibleG2` int unsigned NOT NULL DEFAULT 0,
  `visibleG3` int unsigned NOT NULL DEFAULT 0,
  `visibleG4` int unsigned NOT NULL DEFAULT 0,
  `visibleG5` int unsigned NOT NULL DEFAULT 0,
  `visibleG6` int unsigned NOT NULL DEFAULT 0,
  `visibleG7` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`castle_id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_expulsion`
--

CREATE TABLE IF NOT EXISTS `guild_expulsion` (
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `mes` varchar(40) NOT NULL DEFAULT '',
  `char_id` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`guild_id`,`name`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_member`
--

CREATE TABLE IF NOT EXISTS `guild_member` (
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `exp` bigint unsigned NOT NULL DEFAULT 0,
  `position` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`guild_id`,`char_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_position`
--

CREATE TABLE IF NOT EXISTS `guild_position` (
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `position` tinyint unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `mode` smallint unsigned NOT NULL DEFAULT 0,
  `exp_mode` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`guild_id`,`position`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_skill`
--

CREATE TABLE IF NOT EXISTS `guild_skill` (
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `id` smallint unsigned NOT NULL DEFAULT 0,
  `lv` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`guild_id`,`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_storage`
--

CREATE TABLE IF NOT EXISTS `guild_storage` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `amount` int unsigned NOT NULL DEFAULT 0,
  `equip` int unsigned NOT NULL DEFAULT 0,
  `identify` smallint unsigned NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_storage_log`
--

CREATE TABLE IF NOT EXISTS `guild_storage_log` (
  `id` int NOT NULL AUTO_INCREMENT,
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  `time` datetime NOT NULL,
  `char_id` int NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `amount` int NOT NULL DEFAULT 1,
  `identify` smallint NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`id`),
  INDEX (`guild_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `homunculus`
--

CREATE TABLE IF NOT EXISTS `homunculus` (
  `homun_id` int NOT NULL AUTO_INCREMENT,
  `char_id` int NOT NULL,
  `class` mediumint unsigned NOT NULL DEFAULT 0,
  `prev_class` mediumint NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `level` smallint NOT NULL DEFAULT 0,
  `exp` bigint unsigned NOT NULL DEFAULT 0,
  `intimacy` int NOT NULL DEFAULT 0,
  `hunger` smallint NOT NULL DEFAULT 0,
  `str` smallint unsigned NOT NULL DEFAULT 0,
  `agi` smallint unsigned NOT NULL DEFAULT 0,
  `vit` smallint unsigned NOT NULL DEFAULT 0,
  `int` smallint unsigned NOT NULL DEFAULT 0,
  `dex` smallint unsigned NOT NULL DEFAULT 0,
  `luk` smallint unsigned NOT NULL DEFAULT 0,
  `hp` int unsigned NOT NULL DEFAULT 0,
  `max_hp` int unsigned NOT NULL DEFAULT 0,
  `sp` int unsigned NOT NULL DEFAULT 0,
  `max_sp` int unsigned NOT NULL DEFAULT 0,
  `skill_point` smallint unsigned NOT NULL DEFAULT 0,
  `alive` tinyint NOT NULL DEFAULT 1,
  `rename_flag` tinyint NOT NULL DEFAULT 0,
  `vaporize` tinyint NOT NULL DEFAULT 0,
  `autofeed` tinyint NOT NULL DEFAULT 0,
  PRIMARY KEY  (`homun_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `hotkey`
--

CREATE TABLE IF NOT EXISTS `hotkey` (
  `char_id` int NOT NULL,
  `hotkey` tinyint unsigned NOT NULL,
  `type` tinyint unsigned NOT NULL DEFAULT 0,
  `itemskill_id` int unsigned NOT NULL DEFAULT 0,
  `skill_lvl` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`hotkey`)
) ENGINE=MyISAM;

-- 
-- Table structure for table `interlog`
--

CREATE TABLE IF NOT EXISTS `interlog` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `log` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX `time` (`time`)
) ENGINE=MyISAM;

--
-- Table structure for table `inventory`
--

CREATE TABLE IF NOT EXISTS `inventory` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `amount` int unsigned NOT NULL DEFAULT 0,
  `equip` int unsigned NOT NULL DEFAULT 0,
  `identify` smallint NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int unsigned NOT NULL DEFAULT 0,
  `favorite` tinyint unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `equip_switch` int unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `ipbanlist`
--

CREATE TABLE IF NOT EXISTS `ipbanlist` (
  `list` varchar(15) NOT NULL DEFAULT '',
  `btime` datetime NOT NULL,
  `rtime` datetime NOT NULL,
  `reason` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`list`, `btime`)
) ENGINE=MyISAM;

--
-- Table structure for table `login`
--

CREATE TABLE IF NOT EXISTS `login` (
  `account_id` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` varchar(23) NOT NULL DEFAULT '',
  `user_pass` varchar(32) NOT NULL DEFAULT '',
  `sex` enum('M','F','S') NOT NULL DEFAULT 'M',
  `email` varchar(39) NOT NULL DEFAULT '',
  `group_id` tinyint NOT NULL DEFAULT 0,
  `state` int unsigned NOT NULL DEFAULT 0,
  `unban_time` int unsigned NOT NULL DEFAULT 0,
  `expiration_time` int unsigned NOT NULL DEFAULT 0,
  `logincount` mediumint unsigned NOT NULL DEFAULT 0,
  `lastlogin` datetime,
  `last_ip` varchar(100) NOT NULL DEFAULT '',
  `birthdate` DATE,
  `character_slots` tinyint unsigned NOT NULL DEFAULT 0,
  `pincode` varchar(4) NOT NULL DEFAULT '',
  `pincode_change` int unsigned NOT NULL DEFAULT 0,
  `vip_time` int unsigned NOT NULL DEFAULT 0,
  `old_group` tinyint NOT NULL DEFAULT 0,
  `web_auth_token` varchar(17) NULL,
  `web_auth_token_enabled` tinyint NOT NULL DEFAULT 0,
  PRIMARY KEY  (`account_id`),
  KEY `name` (`userid`),
  UNIQUE KEY `web_auth_token_key` (`web_auth_token`)
) ENGINE=MyISAM AUTO_INCREMENT=2000000; 

-- added standard accounts for servers, VERY INSECURE!!!
-- inserted into the table called login which is above

INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`) VALUES ('1', 's1', 'p1', 'S','athena@athena.com');

--
-- Table structure for table `mail`
--

CREATE TABLE IF NOT EXISTS `mail` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `send_name` varchar(30) NOT NULL DEFAULT '',
  `send_id` int unsigned NOT NULL DEFAULT 0,
  `dest_name` varchar(30) NOT NULL DEFAULT '',
  `dest_id` int unsigned NOT NULL DEFAULT 0,
  `title` varchar(45) NOT NULL DEFAULT '',
  `message` varchar(500) NOT NULL DEFAULT '',
  `time` int unsigned NOT NULL DEFAULT 0,
  `status` tinyint NOT NULL DEFAULT 0,
  `zeny` int unsigned NOT NULL DEFAULT 0,
  `type` smallint NOT NULL DEFAULT 0,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM;

-- ----------------------------
-- Table structure for `mail_attachments`
-- ----------------------------

CREATE TABLE IF NOT EXISTS `mail_attachments` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `index` smallint unsigned NOT NULL DEFAULT 0,
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `amount` int unsigned NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `identify` smallint NOT NULL DEFAULT 0,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`,`index`)
) ENGINE=MyISAM;

--
-- Table structure for table `mapreg`
--

CREATE TABLE IF NOT EXISTS `mapreg` (
  `varname` varchar(32) binary NOT NULL,
  `index` int unsigned NOT NULL DEFAULT 0,
  `value` varchar(255) NOT NULL,
  PRIMARY KEY (`varname`,`index`)
) ENGINE=MyISAM;

--
-- Table `market` for market shop persistency
--

CREATE TABLE IF NOT EXISTS `market` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `nameid` int unsigned NOT NULL,
  `price` int unsigned NOT NULL,
  `amount` int NOT NULL,
  `flag` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`name`,`nameid`)
) ENGINE = MyISAM;

--
-- Table structure for table `memo`
--

CREATE TABLE IF NOT EXISTS `memo` (
  `memo_id` int unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  `x` smallint unsigned NOT NULL DEFAULT 0,
  `y` smallint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`memo_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `mercenary`
--

CREATE TABLE IF NOT EXISTS `mercenary` (
  `mer_id` int unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int NOT NULL,
  `class` mediumint unsigned NOT NULL DEFAULT 0,
  `hp` int unsigned NOT NULL DEFAULT 0,
  `sp` int unsigned NOT NULL DEFAULT 0,
  `kill_counter` int NOT NULL,
  `life_time` bigint NOT NULL DEFAULT 0,
  PRIMARY KEY  (`mer_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `mercenary_owner`
--

CREATE TABLE IF NOT EXISTS `mercenary_owner` (
  `char_id` int NOT NULL,
  `merc_id` int NOT NULL DEFAULT 0,
  `arch_calls` int NOT NULL DEFAULT 0,
  `arch_faith` int NOT NULL DEFAULT 0,
  `spear_calls` int NOT NULL DEFAULT 0,
  `spear_faith` int NOT NULL DEFAULT 0,
  `sword_calls` int NOT NULL DEFAULT 0,
  `sword_faith` int NOT NULL DEFAULT 0,
  PRIMARY KEY  (`char_id`)
) ENGINE=MyISAM;

-- ----------------------------
-- Table structure for `sales`
-- ----------------------------

CREATE TABLE IF NOT EXISTS `sales` (
  `nameid` int unsigned NOT NULL,
  `start` datetime NOT NULL,
  `end` datetime NOT NULL,
  `amount` int NOT NULL,
  PRIMARY KEY (`nameid`)
) ENGINE=MyISAM;

--
-- Table structure for table `sc_data`
--

CREATE TABLE IF NOT EXISTS `sc_data` (
  `account_id` int unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `type` smallint unsigned NOT NULL,
  `tick` bigint NOT NULL,
  `val1` int NOT NULL DEFAULT 0,
  `val2` int NOT NULL DEFAULT 0,
  `val3` int NOT NULL DEFAULT 0,
  `val4` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`, `type`)
) ENGINE=MyISAM;

--
-- Table structure for table `skillcooldown`
--

CREATE TABLE IF NOT EXISTS `skillcooldown` (
  `account_id` int unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `skill` smallint unsigned NOT NULL DEFAULT 0,
  `tick` bigint NOT NULL,
  PRIMARY KEY (`char_id`, `skill`)
) ENGINE=MyISAM;

--
-- Table structure for table `party`
--

CREATE TABLE IF NOT EXISTS `party` (
  `party_id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(24) NOT NULL DEFAULT '',
  `exp` tinyint unsigned NOT NULL DEFAULT 0,
  `item` tinyint unsigned NOT NULL DEFAULT 0,
  `leader_id` int unsigned NOT NULL DEFAULT 0,
  `leader_char` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`party_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `party_bookings`
--

CREATE TABLE IF NOT EXISTS `party_bookings` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `char_name` varchar(23) NOT NULL,
  `purpose` smallint unsigned NOT NULL DEFAULT 0,
  `assist` tinyint unsigned NOT NULL DEFAULT 0,
  `damagedealer` tinyint unsigned NOT NULL DEFAULT 0,
  `healer` tinyint unsigned NOT NULL DEFAULT 0,
  `tanker` tinyint unsigned NOT NULL DEFAULT 0,
  `minimum_level` smallint unsigned NOT NULL,
  `maximum_level` smallint unsigned NOT NULL,
  `comment` varchar(255) NOT NULL DEFAULT '',
  `created` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`world_name`, `account_id`, `char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `pet`
--

CREATE TABLE IF NOT EXISTS `pet` (
  `pet_id` int unsigned NOT NULL AUTO_INCREMENT,
  `class` mediumint unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `level` smallint unsigned NOT NULL DEFAULT 0,
  `egg_id` int unsigned NOT NULL DEFAULT 0,
  `equip` int unsigned NOT NULL DEFAULT 0,
  `intimate` smallint unsigned NOT NULL DEFAULT 0,
  `hungry` smallint unsigned NOT NULL DEFAULT 0,
  `rename_flag` tinyint unsigned NOT NULL DEFAULT 0,
  `incubate` int unsigned NOT NULL DEFAULT 0,
  `autofeed` tinyint NOT NULL DEFAULT 0,
  PRIMARY KEY  (`pet_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `quest`
--

CREATE TABLE IF NOT EXISTS `quest` (
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `quest_id` int unsigned NOT NULL,
  `state` enum('0','1','2') NOT NULL DEFAULT '0',
  `time` int unsigned NOT NULL DEFAULT 0,
  `count1` mediumint unsigned NOT NULL DEFAULT 0,
  `count2` mediumint unsigned NOT NULL DEFAULT 0,
  `count3` mediumint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`char_id`,`quest_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `skill`
--

CREATE TABLE IF NOT EXISTS `skill` (
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `id` smallint unsigned NOT NULL DEFAULT 0,
  `lv` tinyint unsigned NOT NULL DEFAULT 0,
  `flag` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`char_id`,`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `skill_homunculus`
--

CREATE TABLE IF NOT EXISTS `skill_homunculus` (
  `homun_id` int NOT NULL,
  `id` int NOT NULL,
  `lv` smallint NOT NULL,
  PRIMARY KEY  (`homun_id`,`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `skillcooldown_homunculus`
--

CREATE TABLE IF NOT EXISTS `skillcooldown_homunculus` (
  `homun_id` int NOT NULL,
  `skill` smallint unsigned NOT NULL DEFAULT 0,
  `tick` bigint NOT NULL,
  PRIMARY KEY (`homun_id`,`skill`)
) ENGINE=MyISAM;

--
-- Table structure for table `skillcooldown_mercenary`
--

CREATE TABLE IF NOT EXISTS `skillcooldown_mercenary` (
  `mer_id` int NOT NULL,
  `skill` smallint unsigned NOT NULL DEFAULT 0,
  `tick` bigint NOT NULL,
  PRIMARY KEY (`mer_id`,`skill`)
) ENGINE=MyISAM;

--
-- Table structure for table `storage`
--

CREATE TABLE IF NOT EXISTS `storage` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `amount` smallint unsigned NOT NULL DEFAULT 0,
  `equip` int unsigned NOT NULL DEFAULT 0,
  `identify` smallint unsigned NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `option_id0` smallint NOT NULL DEFAULT 0,
  `option_val0` smallint NOT NULL DEFAULT 0,
  `option_parm0` tinyint NOT NULL DEFAULT 0,
  `option_id1` smallint NOT NULL DEFAULT 0,
  `option_val1` smallint NOT NULL DEFAULT 0,
  `option_parm1` tinyint NOT NULL DEFAULT 0,
  `option_id2` smallint NOT NULL DEFAULT 0,
  `option_val2` smallint NOT NULL DEFAULT 0,
  `option_parm2` tinyint NOT NULL DEFAULT 0,
  `option_id3` smallint NOT NULL DEFAULT 0,
  `option_val3` smallint NOT NULL DEFAULT 0,
  `option_parm3` tinyint NOT NULL DEFAULT 0,
  `option_id4` smallint NOT NULL DEFAULT 0,
  `option_val4` smallint NOT NULL DEFAULT 0,
  `option_parm4` tinyint NOT NULL DEFAULT 0,
  `expire_time` int unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `vending_items`
--

CREATE TABLE IF NOT EXISTS `vending_items` (
  `vending_id` int unsigned NOT NULL,
  `index` smallint unsigned NOT NULL,
  `cartinventory_id` int unsigned NOT NULL,
  `amount` smallint unsigned NOT NULL,
  `price` int unsigned NOT NULL,
  PRIMARY KEY (`vending_id`, `index`)
) ENGINE=MyISAM;

--
-- Table structure for table `vendings`
--

CREATE TABLE IF NOT EXISTS `vendings` (
  `id` int unsigned NOT NULL,
  `account_id` int unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `sex` enum('F','M') NOT NULL DEFAULT 'M',
  `map` varchar(20) NOT NULL,
  `x` smallint unsigned NOT NULL,
  `y` smallint unsigned NOT NULL,
  `title` varchar(80) NOT NULL,
  `body_direction` char(1) NOT NULL DEFAULT '4',
  `head_direction` char(1) NOT NULL DEFAULT '0',
  `sit` char(1) NOT NULL DEFAULT '1',
  `autotrade` tinyint NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM;
