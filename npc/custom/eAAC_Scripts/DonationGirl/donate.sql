CREATE TABLE `donate` (
  `account_id` int(11) unsigned NOT NULL,
  `amount` tinyint(3) unsigned NOT NULL,
  `claimed` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`amount`)
) TYPE=MyISAM;