# Upgrade login and character server tables for eAthena 0.5.2 to 1.0 RC 2

ALTER TABLE `cart_inventory` ADD `broken` int(11) NOT NULL default '0';

ALTER TABLE `char` MODIFY `base_level` bigint(20) unsigned NOT NULL default '1';
ALTER TABLE `char` MODIFY `job_level` bigint(20) unsigned NOT NULL default '1';
ALTER TABLE `char` MODIFY `base_exp` bigint(20) NOT NULL default '0';
ALTER TABLE `char` MODIFY `job_exp` bigint(20) NOT NULL default '0';
ALTER TABLE `char` ADD `partner_id` int(11) NOT NULL default '0';

ALTER TABLE `charlog` MODIFY `str` int(11) unsigned NOT NULL default '0';
ALTER TABLE `charlog` MODIFY `agi` int(11) unsigned NOT NULL default '0';
ALTER TABLE `charlog` MODIFY `vit` int(11) unsigned NOT NULL default '0';
ALTER TABLE `charlog` MODIFY `int` int(11) unsigned NOT NULL default '0';
ALTER TABLE `charlog` MODIFY `dex` int(11) unsigned NOT NULL default '0';
ALTER TABLE `charlog` MODIFY `luk` int(11) unsigned NOT NULL default '0';

ALTER TABLE `global_reg_value` MODIFY `value` varchar(255) NOT NULL default '0';
ALTER TABLE `global_reg_value` ADD PRIMARY KEY (`char_id`, `str`, `account_id`);
ALTER TABLE `global_reg_value` DROP INDEX `account_id`;
ALTER TABLE `global_reg_value` ADD INDEX `account_id` (`account_id`);

ALTER TABLE `guild_castle` ADD `gHP0` int(11) NOT NULL default '0';
ALTER TABLE `guild_castle` ADD `gHP1` int(11) NOT NULL default '0';
ALTER TABLE `guild_castle` ADD `gHP2` int(11) NOT NULL default '0';
ALTER TABLE `guild_castle` ADD `gHP3` int(11) NOT NULL default '0';
ALTER TABLE `guild_castle` ADD `gHP4` int(11) NOT NULL default '0';
ALTER TABLE `guild_castle` ADD `gHP5` int(11) NOT NULL default '0';
ALTER TABLE `guild_castle` ADD `gHP6` int(11) NOT NULL default '0';
ALTER TABLE `guild_castle` ADD `gHP7` int(11) NOT NULL default '0';

ALTER TABLE `guild_member` MODIFY `exp` bigint(20) NOT NULL default '0';

ALTER TABLE `guild_storage` ADD `broken` int(11) NOT NULL default '0';

ALTER TABLE `inventory` ADD `broken` int(11) NOT NULL default '0';

ALTER TABLE `login` MODIFY `account_id` int(11) NOT NULL default '0' AUTO_INCREMENT, AUTO_INCREMENT=1000057;
ALTER TABLE `login` MODIFY `userid` varchar(255) NOT NULL default '';
ALTER TABLE `login` MODIFY `user_pass` varchar(32) NOT NULL default '';
ALTER TABLE `login` ADD `error_message` int(11) NOT NULL default '0';
ALTER TABLE `login` ADD `connect_until` int(11) NOT NULL default '0';
ALTER TABLE `login` ADD `last_ip` varchar(100) NOT NULL default '';
ALTER TABLE `login` ADD `memo` int(11) NOT NULL default '0';
ALTER TABLE `login` ADD `ban_until` int(11) NOT NULL default '0';
ALTER TABLE `login` ADD `state` int(11) NOT NULL default '0';
ALTER TABLE `login` DROP INDEX `account_id`;
ALTER TABLE `login` DROP INDEX `userid`;
ALTER TABLE `login` ADD PRIMARY KEY (`account_id`), ADD INDEX `name` (`userid`);

CREATE TABLE `login_error` (
  `err_id` int(11) NOT NULL default '0',
  `reason` varchar(100) NOT NULL default 'Unknown',
  PRIMARY KEY  (`err_id`)
) TYPE=MyISAM; 

ALTER TABLE `ragsrvinfo` DROP `type`, DROP `text1`, DROP `text2`, DROP `text3`, DROP `text4`;
ALTER TABLE `ragsrvinfo` ADD `motd` varchar(255) NOT NULL default '';

ALTER TABLE `skill` ADD PRIMARY KEY (`char_id`,`id`);

ALTER TABLE `storage` ADD `broken` int(11) NOT NULL default '0';
