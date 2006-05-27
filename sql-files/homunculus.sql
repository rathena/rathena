-- Table for the Homunculus.
-- [blackhole89]

CREATE TABLE `homunculus` (
  `id` int(11) NOT NULL auto_increment,
  `char_id` int(11) unsigned NOT NULL default '0',
  `class` mediumint(9) unsigned NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `level` smallint(4) NOT NULL default '0',
  `exp` int(12) NOT NULL default '0',
  `hunger` tinyint(10) NOT NULL default '0',
  `hp` int(12) NOT NULL default '1',
  `sp` int(12) NOT NULL default '1',
  `skill1lv` smallint(4) NOT NULL default '0',
  `skill2lv` smallint(4) NOT NULL default '0',
  `skill3lv` smallint(4) NOT NULL default '0',
  `skill4lv` smallint(4) NOT NULL default '0',
  `skillpts` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
)