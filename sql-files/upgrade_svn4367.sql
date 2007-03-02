DROP TABLE IF EXISTS `mapreg`;
CREATE TABLE `mapreg` (
  `varname` char(32) NOT NULL,
  `index` int(11) unsigned NOT NULL default '0',
  `value` char(255) NOT NULL
) TYPE=MyISAM;

ALTER TABLE `mapreg` ADD INDEX ( `varname` );
ALTER TABLE `mapreg` ADD INDEX ( `index` );
