-- this will covert the old `mail` table to new format, convert columns and fill in default values --

-- change structure --
ALTER TABLE `mail` CHANGE `message_id` `id` bigint(20) unsigned NOT NULL auto_increment;
ALTER TABLE `mail` CHANGE `from_char_name` `send_name` varchar(30) NOT NULL default '' AFTER `id`;
ALTER TABLE `mail` CHANGE `from_account_id` `send_id` int(11) unsigned NOT NULL default 0 AFTER `send_name`;
ALTER TABLE `mail` CHANGE `to_char_name` `dest_name` varchar(30) NOT NULL default '' AFTER `send_id`;
ALTER TABLE `mail` CHANGE `to_account_id` `dest_id` int(11) unsigned NOT NULL default 0 AFTER `dest_name`;
ALTER TABLE `mail` ADD `title` varchar(45) NOT NULL default '' AFTER `dest_id`;
ALTER TABLE `mail` CHANGE `message` `message` varchar(255) NOT NULL default '' AFTER `title`;
ALTER TABLE `mail` ADD `time` int(11) unsigned NOT NULL default 0 AFTER `message`;
ALTER TABLE `mail` CHANGE `read_flag` `read_flag` tinyint(1) NOT NULL default 0 AFTER `time`;
ALTER TABLE `mail` ADD `zeny` int(11) unsigned NOT NULL default 0 AFTER `read_flag`;
ALTER TABLE `mail` ADD `nameid` int(11) unsigned NOT NULL default 0 AFTER `zeny`;
ALTER TABLE `mail` ADD `amount` int(11) unsigned NOT NULL default 0 AFTER `nameid`;
ALTER TABLE `mail` ADD `refine` tinyint(3) unsigned NOT NULL default 0 AFTER `amount`;
ALTER TABLE `mail` ADD `attribute` tinyint(4) unsigned NOT NULL default 0 AFTER `refine`;
ALTER TABLE `mail` ADD `identify` smallint(6) NOT NULL default 0 AFTER `attribute`;
ALTER TABLE `mail` ADD `card0` smallint(11) NOT NULL default 0 AFTER `identify`;
ALTER TABLE `mail` ADD `card1` smallint(11) NOT NULL default 0 AFTER `card0`;
ALTER TABLE `mail` ADD `card2` smallint(11) NOT NULL default 0 AFTER `card1`;
ALTER TABLE `mail` ADD `card3` smallint(11) NOT NULL default 0 AFTER `card2`;
ALTER TABLE `mail` DROP `priority`;
ALTER TABLE `mail` DROP `check_flag`;

-- correct values in some columns --
UPDATE `mail` SET `time` = UNIX_TIMESTAMP(NOW());
UPDATE `mail` SET `send_id` = (SELECT `char_id` FROM `char` WHERE `name` = `send_name`);
UPDATE `mail` SET `dest_id` = (SELECT `char_id` FROM `char` WHERE `name` = `dest_name`);
