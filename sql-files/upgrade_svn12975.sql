START TRANSACTION;

-- delete columns
ALTER TABLE `login` DROP `error_message`;
ALTER TABLE `login` DROP `memo`;

-- rename columns
ALTER TABLE `login` CHANGE `connect_until` `expiration_time` INT( 11 ) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `login` CHANGE `ban_until` `unban_time` INT( 11 ) UNSIGNED NOT NULL DEFAULT '0';

-- reorder columns
ALTER TABLE `login` MODIFY `sex` enum('M','F','S') NOT NULL default 'M' AFTER `user_pass`;
ALTER TABLE `login` MODIFY `email` varchar(39) NOT NULL default '' AFTER `sex`;
ALTER TABLE `login` MODIFY `level` tinyint(3) NOT NULL default '0' AFTER `email`;
ALTER TABLE `login` MODIFY `state` int(11) unsigned NOT NULL default '0' AFTER `level`;
ALTER TABLE `login` MODIFY `unban_time` int(11) unsigned NOT NULL default '0' AFTER `state`;
ALTER TABLE `login` MODIFY `expiration_time` int(11) unsigned NOT NULL default '0' AFTER `unban_time`;
ALTER TABLE `login` MODIFY `logincount` mediumint(9) unsigned NOT NULL default '0' AFTER `expiration_time`;

-- change ip format
ALTER TABLE `loginlog` CHANGE `ip` `ip` VARCHAR( 15 ) NOT NULL default '';
UPDATE `loginlog` SET `ip` = inet_ntoa(`ip`);

COMMIT;
