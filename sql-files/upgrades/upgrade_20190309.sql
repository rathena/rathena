ALTER TABLE `pet`
	ADD COLUMN `autofeed` tinyint(2) NOT NULL default '0' AFTER `incubate`;
