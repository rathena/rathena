# Upgrade database for version 817 to 0.5.2

#DROP TABLE `charlog`;
#DROP TABLE `interlog`;
#DROP TABLE `guild_storage`;

ALTER TABLE `global_reg_value` DROP COLUMN `type`;
ALTER TABLE `global_reg_value` DROP COLUMN `account_id`;

# --------------------------------------------------------------
# Fix the bug that some fields cannot exceed 127
ALTER TABLE `char` MODIFY `str` int(11) unsigned NOT NULL;
ALTER TABLE `char` MODIFY `agi` int(11) unsigned NOT NULL;
ALTER TABLE `char` MODIFY `vit` int(11) unsigned NOT NULL;
ALTER TABLE `char` MODIFY `int` int(11) unsigned NOT NULL;
ALTER TABLE `char` MODIFY `dex` int(11) unsigned NOT NULL;
ALTER TABLE `char` MODIFY `luk` int(11) unsigned NOT NULL;
ALTER TABLE `char` MODIFY `base_level` int(11) unsigned NOT NULL default '1';
ALTER TABLE `char` MODIFY `job_level` int(11) unsigned NOT NULL default '1';

# --------------------------------------------------------------------
# Bug fix : wrong index
ALTER TABLE `storage` DROP INDEX `char_id`, ADD INDEX (`account_id`);

# ----------------------------------------------------------------
# Add log tables


# Table: `charlog`
#
CREATE TABLE `charlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_msg` varchar(255) NOT NULL default 'char select',
  `account_id` int(11) NOT NULL default '0',
  `char_num` tinyint(4) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `str` int(11) unsigned NOT NULL default '0',
  `agi` int(11) unsigned NOT NULL default '0',
  `vit` int(11) unsigned NOT NULL default '0',
  `int` int(11) unsigned NOT NULL default '0',
  `dex` int(11) unsigned NOT NULL default '0',
  `luk` int(11) unsigned NOT NULL default '0',
  `hair` tinyint(4) NOT NULL default '0',
  `hair_color` int(11) NOT NULL default '0'
) TYPE=MyISAM;

# Table: 'interlog'
# 
CREATE TABLE `interlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `log` varchar(255) NOT NULL default ''
) TYPE=MyISAM; 

# ----------------------------------------------------------
# Add new table guild_storage


# Table: 'guild_storage'
# 
CREATE TABLE `guild_storage` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `guild_id` int(11) NOT NULL default '0',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '0',
  `equip` mediumint(8) unsigned NOT NULL default '0',
  `identify` smallint(6) NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# -------------------------------------------------------------------
# Change global_reg_value table
# type = 1   account_reg for all connected map-server ( like Chaos Loki Sakary )
# type = 2   account_reg for current map-server
# type = 3   char_reg for current map-server

ALTER TABLE `global_reg_value` ADD `type` int(11) NOT NULL default '3';
ALTER TABLE `global_reg_value` ADD `account_id` int(11) NOT NULL default '0', ADD INDEX (`account_id`);

ALTER TABLE `guild_member` CHANGE `exp` `exp` BIGINT DEFAULT '0' NOT NULL;

ALTER TABLE `login` CHANGE `email` `email` varchar(100) NOT NULL default 'user@athena';
ALTER TABLE `login` CHANGE `user_pass` `user_pass` varchar(32) NOT NULL default '0';

ALTER TABLE `char` CHANGE `class` `class` int(11) NOT NULL default '0';

DROP TABLE `ragsrvinfo`;

CREATE TABLE `ragsrvinfo` (
  `index` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `exp` int(11) NOT NULL default '0',
  `jexp` int(11) NOT NULL default '0',
  `drop` int(11) NOT NULL default '0',
  `motd` varchar(255) NOT NULL default ''
) TYPE=MyISAM;

