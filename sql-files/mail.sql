CREATE TABLE `mail` (
  `message_id` int(11) NOT NULL auto_increment,
  `to_account_id` int(11) NOT NULL default '0',
  `to_char_name` varchar(24) NOT NULL default '',
  `from_account_id` int(11) NOT NULL default '0',
  `from_char_name` varchar(24) NOT NULL default '',
  `message` varchar(80) NOT NULL default '',
  `read_flag` tinyint(1) NOT NULL default '0',
  `priority` tinyint(1) NOT NULL default '0',
  `check_flag` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`message_id`)
) TYPE=MyISAM;
