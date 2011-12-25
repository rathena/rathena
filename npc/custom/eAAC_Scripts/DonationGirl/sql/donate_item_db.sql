CREATE TABLE `donate_item_db` (
  `id` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
  `price` FLOAT(9,2) UNSIGNED NOT NULL,
  PRIMARY KEY  (`id`)
) TYPE=MYISAM;

#(ID,Price);
REPLACE INTO `donate_item_db` VALUES (601,0.06); #Wing_Of_Fly
REPLACE INTO `donate_item_db` VALUES (602,0.33); #Wing_Of_Butterfly
REPLACE INTO `donate_item_db` VALUES (603,10); #Old_Blue_Box
REPLACE INTO `donate_item_db` VALUES (604,0.05); #Branch_Of_Dead_Tree
REPLACE INTO `donate_item_db` VALUES (605,2); #Anodyne
REPLACE INTO `donate_item_db` VALUES (606,2); #Aloebera
