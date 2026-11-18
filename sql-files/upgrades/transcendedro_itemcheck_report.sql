-- Transcended RO - Item Checker "Report to GM" queue
-- Written by npc/custom/transcendedro_itemcheck.txt
-- Relayed to Discord by tools/discord_item_report.php

CREATE TABLE IF NOT EXISTS `itemcheck_report` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `nameid` int(10) unsigned NOT NULL,
  `item_name` varchar(100) NOT NULL DEFAULT '',
  `account_id` int(11) unsigned NOT NULL DEFAULT '0',
  `char_id` int(11) unsigned NOT NULL DEFAULT '0',
  `char_name` varchar(30) NOT NULL DEFAULT '',
  `report_time` datetime NOT NULL,
  `discord_sent` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `nameid` (`nameid`),
  KEY `discord_sent` (`discord_sent`)
) ENGINE=MyISAM;
