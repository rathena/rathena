--
-- Table `barter` for barter shop persistency
--

CREATE TABLE IF NOT EXISTS `barter` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `index` smallint unsigned NOT NULL,
  `amount` smallint unsigned NOT NULL,
  PRIMARY KEY  (`name`,`index`)
) ENGINE=MyISAM;
