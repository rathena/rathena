CREATE TABLE `donate_item_db` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `name` varchar(30) NOT NULL default '',
  `price` float(5,2) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

#(ID,'Name',Price);
REPLACE INTO `donate_item_db` VALUES (601,'Fly_Wing',0.06);
REPLACE INTO `donate_item_db` VALUES (602,'Butterfly_Wing',0.33);
REPLACE INTO `donate_item_db` VALUES (603,'Old_Blue_Box',10);
REPLACE INTO `donate_item_db` VALUES (604,'Dead_Branch',0.05);
REPLACE INTO `donate_item_db` VALUES (605,'Anodyne',2);
REPLACE INTO `donate_item_db` VALUES (606,'Aloevera',2);