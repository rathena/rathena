CREATE TABLE `donate` (
  `account_id` int(11) unsigned NOT NULL,
  `amount` float(5,2) unsigned NOT NULL,
  `claimed` float(5,2) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`amount`)
) TYPE=MyISAM;