CREATE TABLE IF NOT EXISTS `skillcooldown_homunculus` (
  `homun_id` int NOT NULL,
  `skill` smallint unsigned NOT NULL DEFAULT '0',
  `tick` bigint NOT NULL,
  PRIMARY KEY (`homun_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `skillcooldown_mercenary` (
  `mer_id` int NOT NULL,
  `skill` smallint unsigned NOT NULL DEFAULT '0',
  `tick` bigint NOT NULL,
  PRIMARY KEY (`mer_id`)
) ENGINE=MyISAM;
