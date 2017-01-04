ALTER TABLE `inventory` ADD COLUMN `bound` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `favorite`;
ALTER TABLE `cart_inventory` ADD COLUMN `bound` TINYINT(3) UNSIGNED NOT NULL default '0' AFTER `expire_time`;
ALTER TABLE `storage` ADD COLUMN `bound` TINYINT(3) UNSIGNED NOT NULL default '0' AFTER `expire_time`;
ALTER TABLE `guild_storage` ADD COLUMN `bound` TINYINT(3) UNSIGNED NOT NULL default '0' AFTER `expire_time`;
