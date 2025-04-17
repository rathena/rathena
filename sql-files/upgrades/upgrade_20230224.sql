ALTER TABLE `char`
	ADD COLUMN `last_instanceid` int unsigned NOT NULL DEFAULT '0' AFTER `last_y`
;
