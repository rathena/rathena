CREATE TABLE `donate` (
  `account_id` int(11) unsigned NOT NULL,
  `amount` float(9,2) unsigned NOT NULL,
  `claimed` float(9,2) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`)
) TYPE=MyISAM;