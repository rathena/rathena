#-------------------------
#
# LOGIN SERVER DATABASE
#
#-------------------------

# Database: Ragnarok
# Table: 'login'
# 

CREATE TABLE `login` (
  `account_id` mediumint(7) UNSIGNED NOT NULL AUTO_INCREMENT,
  `userid` varchar(32) NOT NULL default '',
  `user_pass` varchar(32) NOT NULL default '',
  `lastlogin` datetime NOT NULL default '0000-00-00 00:00:00',
  `logincount` smallint(4) UNSIGNED NOT NULL default '0',
  `sex` CHAR NOT NULL default 'M',
  `email` varchar(60) NOT NULL default '',
  `level` tinyint(2) UNSIGNED NOT NULL default '0',
  `connect_until` int(11) UNSIGNED NOT NULL default '0',
  `last_ip` varchar(15) NOT NULL default '',
  `ban_until` int(11) UNSIGNED NOT NULL default '0',
  `state` tinyint(2) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`account_id`),
  INDEX (`account_id`,`userid`),
) TYPE=INNODB AUTO_INCREMENT=2000000;

# added standard accounts for servers, VERY INSECURE!!!
# inserted into the table called login which is above

INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`) VALUES ('1', 's1', 'p1', 'S','athena@athena.com');
INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`) VALUES ('2', 's2', 'p2', 'S','athena@athena.com');

# Database: Ragnarok
# Table: 'login_error'
# 
CREATE TABLE `login_error` (
  `err_id` tinyint(2) UNSIGNED NOT NULL default '0',
  `reason` varchar(100) NOT NULL default 'Unknown',
  PRIMARY KEY (`err_id`),
  INDEX (`err_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'loginlog'
# 
CREATE TABLE `loginlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `ip` varchar(15) NOT NULL default '',
  `user` varchar(32) NOT NULL default '',
  `rcode` tinyint(4) NOT NULL default '0',
  `log` varchar(255) NOT NULL default ''
) TYPE=MyISAM; 


# Database: Ragnarok
# Table: 'ragsrvinfo'
# 
CREATE TABLE `ragsrvinfo` (
  `index` tinyint(2) UNSIGNED NOT NULL default '0',
  `name` varchar(16) NOT NULL default '',
  `exp` smallint(4) UNSIGNED NOT NULL default '0',
  `jexp` smallint(4) UNSIGNED NOT NULL default '0',
  `drop` tinyint(2) UNSIGNED NOT NULL default '0',
  `motd` varchar(255) NOT NULL default ''
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'sstatus'
# 
CREATE TABLE `sstatus` (
  `index` tinyint(4) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `user` smallint(3) UNSIGNED NOT NULL default '0'
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'interlog'
# 
CREATE TABLE `interlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `log` varchar(255) NOT NULL default ''
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'ipbanlist'
# 
CREATE TABLE `ipbanlist` (
  `list` varchar(255) NOT NULL default '',
  `btime` datetime NOT NULL default '0000-00-00 00:00:00',
  `rtime` datetime NOT NULL default '0000-00-00 00:00:00',
  `reason` varchar(255) NOT NULL default ''
) TYPE=MyISAM; 


# Database: Rangarok
# Table: 'errors'
#
CREATE TABLE `errors` (
  `result` tinyint(3) UNSIGNED NOT NULL,
  `error`  varchar(20) NOT NULL,
  INDEX (`result`)
) TYPE=MyISAM;

INSERT INTO `errors` (`result`,`error`) VALUES 
('1','Unregistered ID'),
('2','Incorrect Password'),
('3','Account Expired'),
('4','Rejected from Server'),
('5','Blocked by GM'),
('6','Not latest game EXE'),
('7','Banned'),
('8','Server OverPopulated'),
('9',''),
('100','Account Gone');

##########################
#
# Inter server / Char server databases
# By CLOWNISIUS aka Anthony
#
##########################

# Database: Ragnarok
# Table: 'party'
# 
CREATE TABLE `party` (
  `party_id` smallint(3) UNSIGNED NOT NULL default '100',
  `name` varchar(100) NOT NULL default '',
  `exp` int(11) UNSIGNED NOT NULL default '0',
  `item` int(11) UNSIGNED NOT NULL default '0',
  `leader_id` smallint(4) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`party_id`),
  INDEX (`party_id`,`leader_id`)
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'char'
# 
CREATE TABLE `char` (
  `char_id` mediumint(6) UNSIGNED NOT NULL auto_increment,
  `account_id` mediumint(7) UNSIGNED NOT NULL default '0',
  `char_num` tinyint(1) UNSIGNED NOT NULL default '0',
  `name` varchar(32) NOT NULL default '',
  `class` smallint(4) UNSIGNED NOT NULL default '0',
  `base_level` tinyint(3) UNSIGNED NOT NULL default '1',
  `job_level` tinyint(3) UNSIGNED NOT NULL default '1',
  `base_exp` int(9) UNSIGNED NOT NULL default '0',
  `job_exp` int(9) UNSIGNED NOT NULL default '0',
  `zeny` int(11) UNSIGNED NOT NULL default '500',
  `str` tinyint(3) UNSIGNED NOT NULL default '0',
  `agi` tinyint(3) UNSIGNED NOT NULL default '0',
  `vit` tinyint(3) UNSIGNED NOT NULL default '0',
  `int` tinyint(3) UNSIGNED NOT NULL default '0',
  `dex` tinyint(3) UNSIGNED NOT NULL default '0',
  `luk` tinyint(3) UNSIGNED NOT NULL default '0',
  `max_hp` smallint(5) UNSIGNED NOT NULL default '0',
  `hp` smallint(5) UNSIGNED NOT NULL default '0',
  `max_sp` smallint(5) UNSIGNED NOT NULL default '0',
  `sp` smallint(5) UNSIGNED NOT NULL default '0',
  `status_point` smallint(4) UNSIGNED NOT NULL default '0',
  `skill_point` smallint(4) UNSIGNED NOT NULL default '0',
  `option` smallint(5) UNSIGNED NOT NULL default '0',
  `karma` tinyint(3) UNSIGNED NOT NULL default '0',
  `manner` tinyint(3) UNSIGNED NOT NULL default '0',
  `party_id` smallint(3) UNSIGNED NULL,
  `guild_id` smallint(5) UNSIGNED NULL,
  `pet_id` smallint(4) UNSIGNED NOT NULL default '0',
  `hair` tinyint(3) UNSIGNED NOT NULL default '0',
  `hair_color` tinyint(3) UNSIGNED NOT NULL default '0',
  `clothes_color` tinyint(3) UNSIGNED NOT NULL default '0',
  `weapon` tinyint(3) UNSIGNED NOT NULL default '1',
  `shield` tinyint(3) UNSIGNED NOT NULL default '0',
  `head_top` tinyint(3) UNSIGNED NOT NULL default '0',
  `head_mid` tinyint(3) UNSIGNED NOT NULL default '0',
  `head_bottom` tinyint(3) UNSIGNED NOT NULL default '0',
  `last_map` varchar(20) NOT NULL default 'new_5-1.gat',
  `last_x` smallint(3) UNSIGNED NOT NULL default '53',
  `last_y` smallint(3) UNSIGNED NOT NULL default '111',
  `save_map` varchar(20) NOT NULL default 'new_5-1.gat',
  `save_x` smallint(3) UNSIGNED NOT NULL default '53',
  `save_y` smallint(3) UNSIGNED NOT NULL default '111',
  `partner_id` mediumint(6) UNSIGNED NULL,
  `online` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`account_id`),
  INDEX (`account_id`,`char_id`,`name`),
  KEY (`partner_id`),
  KEY (`party_id`),
  KEY (`guild_id`),
  FOREIGN KEY (`account_id`) REFERENCES `login` (`account_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY (`partner_id`) REFERENCES `char` (`char_id`) ON DELETE SET NULL
) TYPE=INNODB AUTO_INCREMENT=150001; 

CREATE TABLE `friend` (
  `char_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `position` tinyint(1) UNSIGNED NOT NULL default '0',
  `friend_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `friend_name` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`char_id`,`friend_id`),
  INDEX (`char_id`),
  INDEX (`friend_id`),
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE,
  FOREIGN KEY (`friend_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=InnoDB;

# Database: Ragnarok
# Table: 'global_reg_value'
# 
CREATE TABLE `global_reg_value` (
  `char_id` mediumint(6) UNSIGNED,
  `str` varchar(255) NOT NULL default '',
  `value` varchar(255) NOT NULL default '0',
  `type` tinyint(3) UNSIGNED NOT NULL default '3',
  `account_id` mediumint(7) UNSIGNED,
  PRIMARY KEY  (`char_id`,`str`,`account_id`),
  INDEX (`account_id`),
  INDEX (`char_id`),
  FOREIGN KEY (`account_id`) REFERENCES `login`(`account_id`) ON DELETE CASCADE,
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=INNODB; 

## Cannot use due to char and account load from same database, saving will be done to diferent DBs soon.
## so that dependencies can be placed in.

# Login register value.
#

CREATE TABLE `account_reg_value` (
  `account_id` mediumint(7) UNSIGNED,
  `str` varchar(255) NOT NULL default '',
  `value` varchar(255) NOT NULL default '0',
  KEY (`account_id`, `str`),
  FOREIGN KEY (`account_id`) REFERENCES `login`(`account_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Character register value.
#

CREATE TABLE `char_reg_value` (
  `char_id` mediumint(6) UNSIGNED,
  `str` varchar(255) NOT NULL default '',
  `value` varchar(255) NOT NULL default '0',
  PRIMARY KEY (`char_id`, `str`),
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=INNODB;


# Database: Ragnarok
# Table: 'inventory'
# 
CREATE TABLE `inventory` (
  `char_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `id` smallint(4) UNSIGNED NOT NULL auto_increment,
  `nameid` smallint(4) UNSIGNED NOT NULL default '0',
  `amount` tinyint(3) UNSIGNED NOT NULL default '0',
  `equip` smallint(5) UNSIGNED NOT NULL default '0',
  `identify` tinyint(1) UNSIGNED NOT NULL default '0',
  `refine` tinyint(1) UNSIGNED NOT NULL default '0',
  `attribute` tinyint(3) UNSIGNED NOT NULL default '0',
  `card0` smallint(4) UNSIGNED NOT NULL default '0',
  `card1` smallint(4) UNSIGNED NOT NULL default '0',
  `card2` smallint(4) UNSIGNED NOT NULL default '0',
  `card3` smallint(4) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`id`,`char_id`),
  INDEX(`char_id`,`id`),
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=INNODB;

# Database: Ragnarok
# Table: 'memo'
# 
CREATE TABLE `memo` (
  `char_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `memo_id` smallint(4) UNSIGNED NOT NULL default '0',
  `map` varchar(20) NOT NULL default '',
  `x` smallint(3) UNSIGNED NOT NULL default '0',
  `y` smallint(3) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`memo_id`),
  INDEX (`char_id`,`memo_id`),
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'pet'
# 
CREATE TABLE `pet` (
  `pet_id` smallint(4) UNSIGNED NOT NULL default '0',
  `class` smallint(4) UNSIGNED NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `account_id` mediumint(7) UNSIGNED NOT NULL default '0',
  `char_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `level` tinyint(3) UNSIGNED NOT NULL default '0',
  `egg_id` smallint(4) UNSIGNED NOT NULL default '0',
  `equip` smallint(5) UNSIGNED NOT NULL default '0',
  `intimate` smallint(4) UNSIGNED NOT NULL default '0',
  `hungry` tinyint(3) UNSIGNED NOT NULL default '0',
  `rename_flag` tinyint(1) UNSIGNED NOT NULL default '0',
  `incuvate` tinyint(1) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`pet_id`),
  INDEX (`char_id`,`pet_id`),
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'skill'
# 
CREATE TABLE `skill` (
  `char_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `id` smallint(3) UNSIGNED NOT NULL default '0',
  `lv` tinyint(3) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`id`),
  INDEX (`char_id`,`id`),
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'cart_inventory'
# 
CREATE TABLE `cart_inventory` (
  `char_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `id` smallint(4) UNSIGNED NOT NULL auto_increment,
  `nameid` smallint(4) UNSIGNED NOT NULL default '0',
  `amount` tinyint(3) UNSIGNED NOT NULL default '0',
  `equip` smallint(5) UNSIGNED NOT NULL default '0',
  `identify` tinyint(1) UNSIGNED NOT NULL default '0',
  `refine` tinyint(1) UNSIGNED NOT NULL default '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL default '0',
  `card0` smallint(4) UNSIGNED NOT NULL default '0',
  `card1` smallint(4) UNSIGNED NOT NULL default '0',
  `card2` smallint(4) UNSIGNED NOT NULL default '0',
  `card3` smallint(4) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`id`,`char_id`),
  INDEX  (`char_id`,`id`),
  FOREIGN KEY (`char_id`) REFERENCES `char`(`char_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'storage'
# 
CREATE TABLE `storage` (
  `account_id` mediumint(7) UNSIGNED NOT NULL default '0',
  `id` smallint(4) UNSIGNED NOT NULL default '0',
  `nameid` smallint(4) UNSIGNED NOT NULL default '0',
  `amount` tinyint(3) UNSIGNED NOT NULL default '0',
  `equip` smallint(5) UNSIGNED  NOT NULL default '0',
  `identify` tinyint(1) UNSIGNED NOT NULL default '0',
  `refine` tinyint(1) UNSIGNED NOT NULL default '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL default '0',
  `card0` smallint(4) UNSIGNED NOT NULL default '0',
  `card1` smallint(4) UNSIGNED NOT NULL default '0',
  `card2` smallint(4) UNSIGNED NOT NULL default '0',
  `card3` smallint(4) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`id`),
  INDEX  (`account_id`,`id`),
  FOREIGN KEY (`account_id`) REFERENCES `login` (`account_id`) ON DELETE CASCADE
) TYPE=INNODB; 

##########################
#
# Inter server / Guild server databases
# By CLOWNISIUS aka Anthony
#
##########################


# Database: Ragnarok
# Table: 'guild'
# 
CREATE TABLE `guild` (
  `guild_id` smallint(5) UNSIGNED NOT NULL default '10000',
  `name` varchar(24) NOT NULL default '',
  `char_id` mediumint(6) UNSIGNED NOT NULL default '10000',
  `master` varchar(24) NOT NULL default '',
  `guild_lv` tinyint(3) UNSIGNED NOT NULL default '0',
  `connect_member` tinyint(3) UNSIGNED NOT NULL default '0',
  `max_member` tinyint(3) UNSIGNED NOT NULL default '0',
  `average_lv` tinyint(3) UNSIGNED NOT NULL default '0',
  `exp` int(9) UNSIGNED NOT NULL default '0',
  `next_exp` int(9) UNSIGNED NOT NULL default '0',
  `skill_point` smallint(4) UNSIGNED NOT NULL default '0',
  `castle_id` smallint(5) UNSIGNED NOT NULL default '-1',
  `mes1` varchar(60) NOT NULL default '',
  `mes2` varchar(120) NOT NULL default '',
  `emblem_len` int(11) UNSIGNED NOT NULL default '0',
  `emblem_id` int(11) UNSIGNED NOT NULL default '0',
  `emblem_data` blob NOT NULL,
  PRIMARY KEY  (`guild_id`,`char_id`),
  INDEX  (`char_id`,`guild_id`),
  FOREIGN KEY (`char_id`) REFERENCES `char`(`char_id`) ON DELETE CASCADE
) TYPE=INNODB; 


CREATE TABLE `guild_alliance` (
  `guild_id` smallint(5) UNSIGNED NOT NULL default '0',
  `opposition` smallint(5) UNSIGNED NOT NULL default '0',
  `alliance_id` smallint(5) UNSIGNED NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  PRIMARY KEY (`guild_id`,`alliance_id`),
  INDEX (`guild_id`),
  INDEX (`alliance_id`),
  FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE,
  FOREIGN KEY (`alliance_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'guild_castle'
# 
CREATE TABLE `guild_castle` (
  `castle_id` smallint(5) UNSIGNED NOT NULL default '0',
  `guild_id` smallint(5) UNSIGNED NOT NULL default '0',
  `economy` int(11) NOT NULL default '0',
  `defense` int(11) NOT NULL default '0',
  `triggerE` int(11) NOT NULL default '0',
  `triggerD` int(11) NOT NULL default '0',
  `nextTime` int(11) NOT NULL default '0',
  `payTime` int(11) NOT NULL default '0',
  `createTime` int(11) NOT NULL default '0',
  `visibleC` int(11) NOT NULL default '0',
  `visibleG0` int(11) NOT NULL default '0',
  `visibleG1` int(11) NOT NULL default '0',
  `visibleG2` int(11) NOT NULL default '0',
  `visibleG3` int(11) NOT NULL default '0',
  `visibleG4` int(11) NOT NULL default '0',
  `visibleG5` int(11) NOT NULL default '0',
  `visibleG6` int(11) NOT NULL default '0',
  `visibleG7` int(11) NOT NULL default '0',
  `gHP0` smallint(5) UNSIGNED NOT NULL default '0',
  `ghP1` smallint(5) UNSIGNED NOT NULL default '0',
  `gHP2` smallint(5) UNSIGNED NOT NULL default '0',
  `gHP3` smallint(5) UNSIGNED NOT NULL default '0',
  `gHP4` smallint(5) UNSIGNED NOT NULL default '0',
  `gHP5` smallint(5) UNSIGNED NOT NULL default '0',
  `gHP6` smallint(5) UNSIGNED NOT NULL default '0',
  `gHP7` smallint(5) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`castle_id`),
  INDEX (`guild_id`,`castle_id`),
  FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'guild_expulsion'
# 
CREATE TABLE `guild_expulsion` (
  `guild_id` smallint(5) UNSIGNED NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `mes` varchar(40) NOT NULL default '',
  `acc` varchar(40) NOT NULL default '',
  `account_id` mediumint(7) UNSIGNED NOT NULL default '0',
  `rsv1` int(11) NOT NULL default '0',
  `rsv2` int(11) NOT NULL default '0',
  `rsv3` int(11) NOT NULL default '0',
  PRIMARY KEY (`guild_id`,`name`),
  INDEX (`guild_id`,`name`),
  FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=INNODB; 


##########################
#
# Linked Database to the CHAR section and LOGIN
#
##########################

# Database: Ragnarok
# Table: 'guild_member'
# 

CREATE TABLE `guild_member` (
  `guild_id` smallint(5) UNSIGNED NOT NULL default '0',
  `account_id` mediumint(7) UNSIGNED NOT NULL default '0',
  `char_id` mediumint(6) UNSIGNED NOT NULL default '0',
  `hair` tinyint(3) UNSIGNED NOT NULL default '0',
  `hair_color` tinyint(3) UNSIGNED NOT NULL default '0',
  `gender` tinyint(3) UNSIGNED NOT NULL default '0',
  `class` smallint(4) UNSIGNED NOT NULL default '0',
  `lv` tinyint(3) UNSIGNED NOT NULL default '0',
  `exp` int(9) UNSIGNED NOT NULL default '0',
  `exp_payper` int(9) UNSIGNED NOT NULL default '0',
  `online` tinyint(1) UNSIGNED NOT NULL default '0',
  `position` smallint(6) UNSIGNED NOT NULL default '0',
  `rsv1` int(11) UNSIGNED NOT NULL default '0',
  `rsv2` int(11) UNSIGNED NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  PRIMARY KEY  (`guild_id`,`char_id`),
  INDEX  (`guild_id`),
  INDEX (`char_id`),
  FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE,
  FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=INNODB;

# Database: Ragnarok
# Table: 'guild_position'
# 
CREATE TABLE `guild_position` (
  `guild_id` smallint(5) UNSIGNED NOT NULL default '0',
  `position` smallint(5) UNSIGNED NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `mode` int(11) UNSIGNED NOT NULL default '0',
  `exp_mode` int(11) UNSIGNED NOT NULL default '0',
  PRIMARY KEY (`guild_id`),
  INDEX (`guild_id`),
  FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'guild_skill'
# 
CREATE TABLE `guild_skill` (
  `guild_id` smallint(5) UNSIGNED NOT NULL default '0',
  `id` smallint(4) UNSIGNED NOT NULL default '0',
  `lv` tinyint(3) UNSIGNED NOT NULL default '0',
  PRIMARY KEY (`guild_id`,`id`),
  INDEX (`guild_id`,`id`),
  FOREIGN KEY (`guild_id`) REFERENCES `guild`(`guild_id`) ON DELETE CASCADE
) TYPE=INNODB; 

# Database: Ragnarok
# Table: 'guild_storage'
# 
CREATE TABLE `guild_storage` (
  `id` mediumint(5) UNSIGNED NOT NULL auto_increment,
  `guild_id` smallint(5) UNSIGNED NOT NULL default '0',
  `nameid` smallint(4) UNSIGNED NOT NULL default '0',
  `amount` tinyint(3) UNSIGNED NOT NULL default '0',
  `equip` smallint(5) UNSIGNED NOT NULL default '0',
  `identify` tinyint(1) UNSIGNED NOT NULL default '0',
  `refine` tinyint(1) UNSIGNED NOT NULL default '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL default '0',
  `card0` smallint(4) UNSIGNED NOT NULL default '0',
  `card1` smallint(4) UNSIGNED NOT NULL default '0',
  `card2` smallint(4) UNSIGNED NOT NULL default '0',
  `card3` smallint(4) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`guild_id`,`id`),
  INDEX  (`id`,`guild_id`),
  FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=INNODB; 


# Database: Ragnarok
# Table: 'charlog'
# 
CREATE TABLE `charlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_msg` varchar(255) NOT NULL default 'char select',
  `account_id` mediumint(7) UNSIGNED NOT NULL default '0',
  `char_num` tinyint(1) UNSIGNED NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `str` tinyint(3) UNSIGNED NOT NULL default '0',
  `agi` tinyint(3) UNSIGNED NOT NULL default '0',
  `vit` tinyint(3) UNSIGNED NOT NULL default '0',
  `int` tinyint(3) UNSIGNED NOT NULL default '0',
  `dex` tinyint(3) UNSIGNED NOT NULL default '0',
  `luk` tinyint(3) UNSIGNED NOT NULL default '0',
  `hair` tinyint(3) UNSIGNED NOT NULL default '0',
  `hair_color` tinyint(3) UNSIGNED NOT NULL default '0'
) TYPE=MyISAM; 

ALTER TABLE `char` ADD FOREIGN KEY (`party_id`) REFERENCES `party` (`party_id`) ON DELETE SET NULL ON UPDATE SET NULL;
ALTER TABLE `char` ADD FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE SET NULL ON UPDATE SET NULL;