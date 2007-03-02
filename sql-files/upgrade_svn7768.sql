DROP TABLE IF EXISTS `skill_homunculus`;
CREATE TABLE `skill_homunculus` (
  `homun_id` int(11) NOT NULL,
  `id` int(11) NOT NULL,
  `lv` smallint(6) NOT NULL,
  PRIMARY KEY  (`homun_id`,`id`),
  KEY `homun_id` (`homun_id`)
) TYPE=MyISAM;