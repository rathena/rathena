--
-- Table `barter` for barter shop persistency
--

CREATE TABLE IF NOT EXISTS `barter` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `index` SMALLINT(5) UNSIGNED NOT NULL,
  `amount` SMALLINT(5) UNSIGNED NOT NULL,
  PRIMARY KEY  (`name`,`index`)
) ENGINE=MyISAM;
