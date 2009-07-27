-- Creates expire_time column on Inventory

ALTER TABLE `inventory` ADD COLUMN `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `cart_inventory` ADD COLUMN `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `storage` ADD COLUMN `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `guild_storage` ADD COLUMN `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0';
