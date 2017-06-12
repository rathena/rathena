ALTER TABLE `char`
	ADD COLUMN `last_login` datetime DEFAULT NULL AFTER `clan_id`;
