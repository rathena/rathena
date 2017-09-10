ALTER TABLE `char`
	ADD COLUMN `show_equip` tinyint(3) unsigned NOT NULL default '0' AFTER `title_id`;
