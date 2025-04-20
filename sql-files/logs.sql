--
-- Table structure for table `atcommandlog`
--

CREATE TABLE IF NOT EXISTS `atcommandlog` (
  `atcommand_id` mediumint unsigned NOT NULL AUTO_INCREMENT,
  `atcommand_date` datetime NOT NULL,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `char_name` varchar(25) NOT NULL DEFAULT '',
  `map` varchar(11) NOT NULL DEFAULT '',
  `command` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY  (`atcommand_id`),
  INDEX (`account_id`),
  INDEX (`char_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `branchlog`
--

CREATE TABLE IF NOT EXISTS `branchlog` (
  `branch_id` mediumint unsigned NOT NULL AUTO_INCREMENT,
  `branch_date` datetime NOT NULL,
  `account_id` int NOT NULL DEFAULT 0,
  `char_id` int NOT NULL DEFAULT 0,
  `char_name` varchar(25) NOT NULL DEFAULT '',
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY  (`branch_id`),
  INDEX (`account_id`),
  INDEX (`char_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `cashlog`
--

CREATE TABLE IF NOT EXISTS `cashlog` (
  `id` int NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int NOT NULL DEFAULT 0,
  `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','$') NOT NULL DEFAULT 'S',
  `cash_type` enum('O','K','C') NOT NULL DEFAULT 'O',
  `amount` int NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX `type` (`type`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `chatlog`
--
# ChatLog types
# Gl(O)bal
# (W)hisper
# (P)arty
# (G)uild
# (M)ain chat
# (C)lan

CREATE TABLE IF NOT EXISTS `chatlog` (
  `id` bigint NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `type` enum('O','W','P','G','M','C') NOT NULL DEFAULT 'O',
  `type_id` int NOT NULL DEFAULT 0,
  `src_charid` int NOT NULL DEFAULT 0,
  `src_accountid` int NOT NULL DEFAULT 0,
  `src_map` varchar(11) NOT NULL DEFAULT '',
  `src_map_x` smallint NOT NULL DEFAULT 0,
  `src_map_y` smallint NOT NULL DEFAULT 0,
  `dst_charname` varchar(25) NOT NULL DEFAULT '',
  `message` varchar(150) NOT NULL DEFAULT '',
  PRIMARY KEY  (`id`),
  INDEX (`src_accountid`),
  INDEX (`src_charid`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `feedinglog`
--

CREATE TABLE IF NOT EXISTS `feedinglog` (
  `id` int NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int NOT NULL,
  `target_id` int NOT NULL,
  `target_class` smallint NOT NULL,
  `type` ENUM('P','H','O') NOT NULL, -- P: Pet, H: Homunculus, O: Other
  `intimacy` int unsigned NOT NULL,
  `item_id` int unsigned NOT NULL,
  `map` varchar(11) NOT NULL,
  `x` smallint unsigned NOT NULL,
  `y` smallint unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE = MyISAM AUTO_INCREMENT = 1;

--
-- Table structure for table `loginlog`
--

CREATE TABLE IF NOT EXISTS `loginlog` (
  `time` datetime NOT NULL,
  `ip` varchar(15) NOT NULL DEFAULT '',
  `user` varchar(23) NOT NULL DEFAULT '',
  `rcode` tinyint NOT NULL DEFAULT 0,
  `log` varchar(255) NOT NULL DEFAULT '',
  INDEX (`ip`)
) ENGINE=MyISAM ;

--
-- Table structure for table `mvplog`
--

CREATE TABLE IF NOT EXISTS `mvplog` (
  `mvp_id` mediumint unsigned NOT NULL AUTO_INCREMENT,
  `mvp_date` datetime NOT NULL,
  `kill_char_id` int NOT NULL DEFAULT 0,
  `monster_id` smallint NOT NULL DEFAULT 0,
  `prize` int unsigned NOT NULL DEFAULT 0,
  `mvpexp` bigint unsigned NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY  (`mvp_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `npclog`
--

CREATE TABLE IF NOT EXISTS `npclog` (
  `npc_id` mediumint unsigned NOT NULL AUTO_INCREMENT,
  `npc_date` datetime NOT NULL,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `char_name` varchar(25) NOT NULL DEFAULT '',
  `map` varchar(11) NOT NULL DEFAULT '',
  `mes` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY  (`npc_id`),
  INDEX (`account_id`),
  INDEX (`char_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `picklog`
--
# PickLog types
# (M)onsters Drop
# (P)layers Drop/Take
# Mobs Drop (L)oot Drop/Take
# Players (T)rade Give/Take
# Players (V)ending Sell/Take
# (S)hop Sell/Take
# (N)PC Give/Take
# (C)onsumable Items
# (A)dministrators Create/Delete
# Sto(R)age
# (G)uild Storage
# (E)mail attachment
# (B)uying Store
# Pr(O)duced Items/Ingredients
# Auct(I)oned Items
# (X) Other
# (D) Stolen from mobs
# (U) MVP Prizes
# ($) Cash
# (F) Guild/Party Bound retrieval
# Lotter(Y)
# (Z) Merged Items
# (Q)uest
# Private Airs(H)ip
# Barter Shop (J)
# Laphine systems (W)
# Enchantgrade UI (0)
# Reform UI (1)
# Enchant UI (2)

CREATE TABLE IF NOT EXISTS `picklog` (
  `id` int NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int NOT NULL DEFAULT 0,
  `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U','$','F','Y','Z','Q','H','J','W','0','1','2','3') NOT NULL DEFAULT 'P',
  `nameid` int unsigned NOT NULL DEFAULT 0,
  `amount` int NOT NULL DEFAULT 1,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
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
  `map` varchar(11) NOT NULL DEFAULT '',
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY  (`id`),
  INDEX (`type`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `zenylog`
--
# ZenyLog types
# (T)rade
# (V)ending Sell/Buy
# (P)layers Drop/Take
# (M)onsters
# (S)hop Sell/Buy
# (N)PC Change amount
# (D) Stolen from mobs
# (C)onsumable Items
# (A)dministrators
# (E)Mail
# Auct(I)on
# (B)uying Store
# Ban(K) Transactions
# Barter Shop (J)
# (X) Other
# Enchantgrade UI (0)
# Enchant UI (2)

CREATE TABLE IF NOT EXISTS `zenylog` (
  `id` int NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `char_id` int NOT NULL DEFAULT 0,
  `src_id` int NOT NULL DEFAULT 0,
  `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','K','J','X','0','2') NOT NULL DEFAULT 'S',
  `amount` int NOT NULL DEFAULT 0,
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY  (`id`),
  INDEX (`type`)
) ENGINE=MyISAM AUTO_INCREMENT=1;
