ALTER TABLE `homunculus`
	CHANGE COLUMN `sp` `sp` int unsigned NOT NULL DEFAULT '0',
	CHANGE COLUMN `max_sp` `max_sp` int unsigned NOT NULL DEFAULT '0';
