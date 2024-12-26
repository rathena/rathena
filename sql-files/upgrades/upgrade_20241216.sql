CREATE TABLE IF NOT EXISTS `skillcooldown_homunculus` (
  `homun_id` int(11) NOT NULL,
  `skill` smallint(11) unsigned NOT NULL DEFAULT '0',
  `tick` bigint(20) NOT NULL,
  PRIMARY KEY (`homun_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `skillcooldown_mercenary` (
  `mer_id` int(11) NOT NULL,
  `skill` smallint(11) unsigned NOT NULL DEFAULT '0',
  `tick` bigint(20) NOT NULL,
  PRIMARY KEY (`mer_id`)
) ENGINE=MyISAM;
