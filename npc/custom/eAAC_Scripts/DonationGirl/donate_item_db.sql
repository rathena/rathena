CREATE TABLE `donate_item_db` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `name` varchar(30) NOT NULL default '',
  `amount` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

-- // (ID,'Name',Amount);
REPLACE INTO `donate_item_db` VALUES (601,'Fly_Wing',165);
REPLACE INTO `donate_item_db` VALUES (602,'Butterfly_Wing',30);
REPLACE INTO `donate_item_db` VALUES (603,'Old_Blue_Box',1);
REPLACE INTO `donate_item_db` VALUES (604,'Dead_Branch',200);
REPLACE INTO `donate_item_db` VALUES (605,'Anodyne',5);
REPLACE INTO `donate_item_db` VALUES (606,'Aloevera',5);