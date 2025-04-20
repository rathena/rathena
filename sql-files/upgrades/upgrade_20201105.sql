ALTER TABLE `auction`
	ADD COLUMN `enchantgrade` tinyint unsigned NOT NULL default '0';

ALTER TABLE `cart_inventory`
	ADD COLUMN `enchantgrade` tinyint unsigned NOT NULL default '0';

ALTER TABLE `guild_storage`
	ADD COLUMN `enchantgrade` tinyint unsigned NOT NULL default '0';

ALTER TABLE `guild_storage_log`
	ADD COLUMN `enchantgrade` tinyint unsigned NOT NULL default '0';

ALTER TABLE `inventory`
	ADD COLUMN `enchantgrade` tinyint unsigned NOT NULL default '0';

ALTER TABLE `mail_attachments`
	ADD COLUMN `enchantgrade` tinyint unsigned NOT NULL default '0';

ALTER TABLE `storage`
	ADD COLUMN `enchantgrade` tinyint unsigned NOT NULL default '0';
