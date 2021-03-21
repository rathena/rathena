#
# Table structure for table `mob_db2`
#

DROP TABLE IF EXISTS `mob_db2`;
CREATE TABLE `mob_db2` (
  `ID` mediumint(9) unsigned NOT NULL default '0',
  `Sprite` text NOT NULL,
  `kName` text NOT NULL,
  `iName` text NOT NULL,
  `LV` tinyint(6) unsigned NOT NULL default '0',
  `HP` int(9) unsigned NOT NULL default '0',
  `SP` mediumint(9) unsigned NOT NULL default '0',
  `EXP` mediumint(9) unsigned NOT NULL default '0',
  `JEXP` mediumint(9) unsigned NOT NULL default '0',
  `Range1` tinyint(4) unsigned NOT NULL default '0',
  `ATK1` smallint(6) unsigned NOT NULL default '0',
  `ATK2` smallint(6) unsigned NOT NULL default '0',
  `DEF` smallint(6) unsigned NOT NULL default '0',
  `MDEF` smallint(6) unsigned NOT NULL default '0',
  `STR` smallint(6) unsigned NOT NULL default '0',
  `AGI` smallint(6) unsigned NOT NULL default '0',
  `VIT` smallint(6) unsigned NOT NULL default '0',
  `INT` smallint(6) unsigned NOT NULL default '0',
  `DEX` smallint(6) unsigned NOT NULL default '0',
  `LUK` smallint(6) unsigned NOT NULL default '0',
  `Range2` tinyint(4) unsigned NOT NULL default '0',
  `Range3` tinyint(4) unsigned NOT NULL default '0',
  `Scale` tinyint(4) unsigned NOT NULL default '0',
  `Race` tinyint(4) unsigned NOT NULL default '0',
  `Element` tinyint(4) unsigned NOT NULL default '0',
  `Mode` int(11) unsigned NOT NULL default '0',
  `Speed` smallint(6) unsigned NOT NULL default '0',
  `aDelay` smallint(6) unsigned NOT NULL default '0',
  `aMotion` smallint(6) unsigned NOT NULL default '0',
  `dMotion` smallint(6) unsigned NOT NULL default '0',
  `MEXP` mediumint(9) unsigned NOT NULL default '0',
  `MVP1id` int(10) unsigned NOT NULL default '0',
  `MVP1per` smallint(9) unsigned NOT NULL default '0',
  `MVP2id` int(10) unsigned NOT NULL default '0',
  `MVP2per` smallint(9) unsigned NOT NULL default '0',
  `MVP3id` int(10) unsigned NOT NULL default '0',
  `MVP3per` smallint(9) unsigned NOT NULL default '0',
  `Drop1id` int(10) unsigned NOT NULL default '0',
  `Drop1per` smallint(9) unsigned NOT NULL default '0',
  `Drop2id` int(10) unsigned NOT NULL default '0',
  `Drop2per` smallint(9) unsigned NOT NULL default '0',
  `Drop3id` int(10) unsigned NOT NULL default '0',
  `Drop3per` smallint(9) unsigned NOT NULL default '0',
  `Drop4id` int(10)) unsigned NOT NULL default '0',
  `Drop4per` smallint(9) unsigned NOT NULL default '0',
  `Drop5id` int(10) unsigned NOT NULL default '0',
  `Drop5per` smallint(9) unsigned NOT NULL default '0',
  `Drop6id` int(10) unsigned NOT NULL default '0',
  `Drop6per` smallint(9) unsigned NOT NULL default '0',
  `Drop7id` int(10) unsigned NOT NULL default '0',
  `Drop7per` smallint(9) unsigned NOT NULL default '0',
  `Drop8id` int(10) unsigned NOT NULL default '0',
  `Drop8per` smallint(9) unsigned NOT NULL default '0',
  `Drop9id` int(10) unsigned NOT NULL default '0',
  `Drop9per` smallint(9) unsigned NOT NULL default '0',
  `DropCardid` int(10) unsigned NOT NULL default '0',
  `DropCardper` smallint(9) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM;

REPLACE INTO `mob_db2` VALUES (2100,'BG_ARCHER_GUARDIAN','Archer Guardian','Archer Guardian',74,128634,0,1,1,12,1120,1600,35,60,95,80,80,90,165,55,14,16,2,7,80,0x6281E85,265,1200,1200,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (2101,'BG_KNIGHT_GUARDIAN','Knight Guardian','Knight Guardian',86,130214,0,1,1,2,1280,1560,55,30,110,40,140,65,125,65,14,16,2,7,80,0x6281E85,275,1200,1200,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (2102,'BG_SOLDIER_GUARDIAN','Soldier Guardian','Soldier Guardian',56,115670,0,1,1,1,873,1036,35,0,85,56,100,45,103,43,10,12,0,4,22,0x6282085,265,1288,288,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (2103,'BG_SOLDIER_GUARDIAN2','Soldier Guardian','Soldier Guardian',56,115670,0,1,1,1,873,1036,35,0,85,56,100,45,103,43,10,12,0,4,22,0x6282085,265,1288,288,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (2104,'BG_CAPTAIN','Captain','Captain',56,215670,0,1,1,1,873,1036,35,0,85,56,100,45,103,43,10,12,0,4,22,0x6282085,265,1288,288,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (2105,'BARRICADE_BG','Barricade','Barricade',98,60,1,0,0,1,0,0,160,99,1,17,1,80,126,20,10,12,2,0,20,0x6370000,300,1288,288,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (20400,'Guardian_Stone','Guardian Stone','Guardian Stone',90,50,0,0,0,0,1,2,40,50,1,1,1,1,1,1,0,0,0,0,20,0x190000,300,1288,288,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (20401,'Guardian_Stone2','Guardian Stone','Guardian Stone',90,50,0,0,0,0,1,2,40,50,1,1,1,1,1,1,0,0,0,0,20,0x190000,300,1288,288,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (20402,'Emperium_BG','Emperium','Emperium',90,68430,0,0,0,1,60,71,40,50,1,17,80,50,26,20,10,12,0,8,26,0x6280000,300,1288,288,384,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (20403,'BomberMan_Mushroom','BomberMan Mushroom','BomberMan Mushroom',1,2000,0,0,0,1,1,1,100,99,0,0,0,0,0,90,7,12,0,3,26,0x370048,2000,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (20404,'BomberMan_Xmas','BomberMan Xmas','BomberMan Xmas',1,2000,0,0,0,1,1,1,100,99,0,0,0,0,0,90,7,12,0,3,26,0x370048,2000,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (3293,'BomberMan_Mushroom2','BomberMan Mushroom','BomberMan Mushroom',1,2000,0,0,0,1,1,1,100,99,0,0,0,0,0,90,7,12,0,3,26,0x370048,2000,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (20405,'BomberMan_bug','BomberMan bug','BomberMan bug',1,2000,0,0,0,1,1,1,100,99,0,0,0,0,0,90,7,12,0,3,26,0x370048,2000,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
REPLACE INTO `mob_db2` VALUES (20574,'MD_C_WHITEKNIGHT','White Knight','White Knight',1,60,1,27,20,1,8,1,2,5,6,1,1,0,6,5,10,12,1,3,21,0x83,400,1872,672,480,0,0,0,0,0,0,0,909,7000,1202,100,938,400,512,1000,713,1500,512,150,619,20,0,0,0,0,4001,1);
REPLACE INTO `mob_db2` VALUES (3805,'E_HAPPY_EGG','Happy Egg','Happy Egg',2,45,1,27,20,1,12,13,16,0,8,1,1,0,6,2,10,12,1,3,23,0x83,400,1372,672,480,0,0,0,0,0,0,0,909,7500,1602,80,938,500,512,1100,713,1700,512,800,620,20,0,0,0,0,0,0);

