#Pick_Log types (M)onsters Drop, (P)layers Drop/Take, Mobs Drop (L)oot Drop/Take,
# Players (T)rade Give/Take, Players (V)ending Sell/Take, (S)hop Sell/Take, (N)PC Give/Take,
# (C)onsumable Items, (A)dministrators Create/Delete

#Database: log
#Table: picklog
CREATE TABLE `picklog` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_id` int(11) NOT NULL default '0',
  `type` set('M','P','L','T','V','S','N','C','A') NOT NULL default 'P',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '1',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#ZenyLog types (M)onsters,(T)rade,(V)ending Sell/Buy,(S)hop Sell/Buy,(N)PC Change amount,(A)dministrators
#Database: log
#Table: zenylog
CREATE TABLE `zenylog` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_id` int(11) NOT NULL default '0',
  `src_id` int(11) NOT NULL default '0',
  `type` set('M','T','V','S','N','A') NOT NULL default 'S',
  `amount` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: branchlog
CREATE TABLE `branchlog` (
  `branch_id` mediumint(9) unsigned NOT NULL auto_increment,
  `branch_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `char_name` varchar(30) NOT NULL default '',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  PRIMARY KEY  (`branch_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: droplog
CREATE TABLE `droplog` (
  `drop_id` mediumint(9) unsigned NOT NULL auto_increment,
  `drop_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `kill_char_id` int(11) NOT NULL default '0',
  `monster_id` smallint(6) NOT NULL default '0',
  `item1` int(11) NOT NULL default '0',
  `item2` int(11) NOT NULL default '0',
  `item3` int(11) NOT NULL default '0',
  `item4` int(11) NOT NULL default '0',
  `item5` int(11) NOT NULL default '0',
  `item6` int(11) NOT NULL default '0',
  `item7` int(11) NOT NULL default '0',
  `item8` int(11) NOT NULL default '0',
  `item9` int(11) NOT NULL default '0',
  `itemCard` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  PRIMARY KEY  (`drop_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: mvplog
CREATE TABLE `mvplog` (
  `mvp_id` mediumint(9) unsigned NOT NULL auto_increment,
  `mvp_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `kill_char_id` int(11) NOT NULL default '0',
  `monster_id` smallint(6) NOT NULL default '0',
  `prize` int(11) NOT NULL default '0',
  `mvpexp` mediumint(9) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  PRIMARY KEY  (`mvp_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;


#Database: log
#Table: presentlog
CREATE TABLE `presentlog` (
  `present_id` mediumint(9) unsigned NOT NULL auto_increment,
  `present_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `src_id` tinyint(1) NOT NULL default '0',
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `char_name` varchar(30) NOT NULL default '',
  `nameid` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  PRIMARY KEY  (`present_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: producelog
CREATE TABLE `producelog` (
  `produce_id` mediumint(9) unsigned NOT NULL auto_increment,
  `produce_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `char_name` varchar(30) NOT NULL default '',
  `nameid` int(11) NOT NULL default '0',
  `slot1` int(11) NOT NULL default '0',
  `slot2` int(11) NOT NULL default '0',
  `slot3` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  `success` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`produce_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: refinelog
CREATE TABLE `refinelog` (
  `refine_id` mediumint(9) unsigned NOT NULL auto_increment,
  `refine_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `char_name` varchar(30) NOT NULL default '',
  `nameid` int(11) NOT NULL default '0',
  `refine` tinyint(2) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  `success` tinyint(1) NOT NULL default '0',
  `item_level` tinyint(2) NOT NULL default '0',
  PRIMARY KEY  (`refine_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: tradelog
CREATE TABLE `tradelog` (
  `trade_id` mediumint(9) unsigned NOT NULL auto_increment,
  `trade_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `src_account_id` int(11) NOT NULL default '0',
  `src_char_id` int(11) NOT NULL default '0',
  `src_char_name` varchar(30) NOT NULL default '',
  `des_account_id` int(11) NOT NULL default '0',
  `des_char_id` int(11) NOT NULL default '0',
  `des_char_name` varchar(30) NOT NULL default '',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '1',
  `refine` tinyint(4) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  `zeny` int(11) NOT NULL default '0',
  PRIMARY KEY  (`trade_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: vendlog
CREATE TABLE `vendlog` (
  `vend_id` mediumint(9) unsigned NOT NULL auto_increment,
  `vend_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `vend_account_id` int(11) NOT NULL default '0',
  `vend_char_id` int(11) NOT NULL default '0',
  `vend_char_name` varchar(30) NOT NULL default '',
  `buy_account_id` int(11) NOT NULL default '0',
  `buy_char_id` int(11) NOT NULL default '0',
  `buy_char_name` varchar(30) NOT NULL default '',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '1',
  `refine` tinyint(4) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  `zeny` int(11) NOT NULL default '0',
  KEY `vend_id` (`vend_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: atcommandlog
CREATE TABLE `atcommandlog` (
  `atcommand_id` mediumint(9) unsigned NOT NULL auto_increment,
  `atcommand_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `account_id` int(11) unsigned NOT NULL default '0',
  `char_id` int(11) unsigned NOT NULL default '0',
  `char_name` varchar(30) NOT NULL default '',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  `command` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`atcommand_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#Database: log
#Table: npclog
CREATE TABLE `npclog` (
  `npc_id` mediumint(9) unsigned NOT NULL auto_increment,
  `npc_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `account_id` int(11) unsigned NOT NULL default '0',
  `char_id` int(11) unsigned NOT NULL default '0',
  `char_name` varchar(30) NOT NULL default '',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  `mes` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`npc_id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;


#ChatLOG
CREATE TABLE `chatlog` (
  `id` bigint(20) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `type` set('W','P','G') NOT NULL default 'W',
  `type_id` int(11) NOT NULL default '0',
  `src_charid` int(11) NOT NULL default '0',
  `src_accountid` int(11) NOT NULL default '0',
  `src_map` varchar(17) NOT NULL default 'prontera.gat',
  `src_map_x` smallint(4) NOT NULL default '0',
  `src_map_y` smallint(4) NOT NULL default '0',
  `dst_charname` varchar(25) NOT NULL default '',
  `message` varchar(150) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
