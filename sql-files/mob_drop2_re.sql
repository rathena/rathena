#
# Table structure for table `mob_drop2_re`
#

DROP TABLE IF EXISTS `mob_drop2_re`;
CREATE TABLE `mob_drop2_re` (
  `MOB_ID` mediumint(9) unsigned NOT NULL default '0',
  `Type` varchar(30) NOT NULL,
  `Index` tinyint(4) NOT NULL,
  `Item_ID` smallint(5) unsigned NOT NULL,
  `Rate` smallint(9) unsigned NOT NULL,
  `Flag` smallint(5) unsigned NOT NULL DEFAULT 0,
  `RNDOPTG` text NOT NULL,
  PRIMARY KEY (`MOB_ID`, `Type`, `Index`)
)  ENGINE=MyISAM;
