ALTER TABLE  `login` ADD  `vip_time` int(11) UNSIGNED NULL DEFAULT '0';
ALTER TABLE  `login` ADD  `old_group` tinyint(3) NOT NULL default '0';
ALTER TABLE `char` ADD `unban_time` int(11) unsigned NOT NULL default '0';
