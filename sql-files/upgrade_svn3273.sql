CREATE TABLE `sc_data` (
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(11) unsigned NOT NULL,
  `type` smallint(11) unsigned NOT NULL,
  `tick` int(11) NOT NULL,
  `val1` int(11) NOT NULL default '0',
  `val2` int(11) NOT NULL default '0',
  `val3` int(11) NOT NULL default '0',
  `val4` int(11) NOT NULL default '0',
  CONSTRAINT `scdata_ibfk_1` FOREIGN KEY (`account_id`) REFERENCES `login` (`account_id`) ON DELETE CASCADE,
  CONSTRAINT `scdata_ibfk_2` FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=InnoDB;
