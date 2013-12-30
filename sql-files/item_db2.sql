#
# Table structure for table `item_db2`
#

DROP TABLE IF EXISTS `item_db2`;
CREATE TABLE `item_db2` (
  `id` smallint(5) unsigned NOT NULL DEFAULT '0',
  `name_english` varchar(50) NOT NULL DEFAULT '',
  `name_japanese` varchar(50) NOT NULL DEFAULT '',
  `type` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `price_buy` mediumint(8) unsigned DEFAULT NULL,
  `price_sell` mediumint(8) unsigned DEFAULT NULL,
  `weight` smallint(5) unsigned NOT NULL DEFAULT '0',
  `attack` smallint(5) unsigned DEFAULT NULL,
  `defence` smallint(5) unsigned DEFAULT NULL,
  `range` tinyint(2) unsigned DEFAULT NULL,
  `slots` tinyint(2) unsigned DEFAULT NULL,
  `equip_jobs` int(10) unsigned DEFAULT NULL,
  `equip_upper` tinyint(2) unsigned DEFAULT NULL,
  `equip_genders` tinyint(1) unsigned DEFAULT NULL,
  `equip_locations` mediumint(7) unsigned DEFAULT NULL,
  `weapon_level` tinyint(1) unsigned DEFAULT NULL,
  `equip_level` tinyint(3) unsigned DEFAULT NULL,
  `refineable` tinyint(1) unsigned DEFAULT NULL,
  `view` smallint(5) unsigned DEFAULT NULL,
  `script` text,
  `equip_script` text,
  `unequip_script` text,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM;

# Items Additional Database
#
# Structure of Database:
#REPLACE INTO `item_db2` VALUES ( ID,'Name','Name',Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Class,Gender,Loc,wLV,eLV,Refineable,View,'Script','OnEquip_Script','OnUnequip_Script');
#
# THQ Quest Items
#=============================================================
#REPLACE INTO `item_db2` VALUES (7950,'THG_Membership','THG Membership',3,NULL,10,10,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (7951,'Token_Bag','Token Bag',3,NULL,10,10,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (1998,'Jeramiah\'s_Jur','Jeramiah\'s Jur',3,NULL,10,10,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (1999,'Zed\'s_Staff','Zed\'s Staff',3,NULL,10,10,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

# Official Event Items that had their Effects removed after the event was completed
#REPLACE INTO `item_db2` VALUES (585,'Wurst','Brusti',11,2,NULL,40,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'itemheal rand(15,20),0; itemskill "PR_MAGNIFICAT",3;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (679,'Gold_Pill','Pilule',0,5000,NULL,300,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'percentheal 50,50;',NULL,NULL);

#REPLACE INTO `item_db2` VALUES (2681,'Republic_Ring','Republic Anniversary Ring',4,20,NULL,100,NULL,0,NULL,0,0xFFFFFFFF,7,2,136,NULL,0,0,0,'bonus bAllStats,3;',NULL,NULL);

#REPLACE INTO `item_db2` VALUES (5134,'Pumpkin_Hat','Pumpkin-Head',4,20,NULL,200,NULL,2,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,206,'bonus2 bSubRace,RC_Demon,5;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5136,'Santa\'s_Hat_','Louise\'s Santa Hat',4,20,NULL,100,NULL,3,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,20,'bonus bMdef,1; bonus bLuk,1; bonus3 bAutoSpellWhenHit,"AL_HEAL",3,50; bonus3 bAutoSpellWhenHit,"AL_BLESSING",10,50;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5145,'Carnival_Joker_Jester','Carnival Jester',4,10,NULL,100,NULL,0,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,89,'bonus bAllStats,3;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5147,'Baseball_Cap','Baseball Cap',4,0,NULL,200,NULL,3,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,216,'bonus2 bExpAddRace,RC_Boss,50; bonus2 bExpAddRace,RC_NonBoss,50;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5201,'Party_Hat_B','2nd Anniversary Party Hat',4,20,NULL,300,NULL,3,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,144,'bonus bAllStats,3;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5202,'Pumpkin_Hat_','Pumpkin Hat',4,20,NULL,200,NULL,2,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,206,'bonus bAllStats,2; bonus2 bSubRace,RC_Demon,5; bonus3 bAddMonsterDropItem,529,RC_DemiHuman,1500;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5204,'Event_Pierrot_Nose','Rudolf\'s Red Nose',4,20,NULL,100,NULL,0,NULL,0,0xFFFFFFFF,7,2,1,NULL,0,0,49,'bonus2 bResEff,Eff_Blind,3000; bonus2 bAddMonsterDropItem,12130,30;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5264,'Aussie_Flag_Hat','Australian Flag Hat',4,20,NULL,500,NULL,4,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,304,'bonus bAllStats,2;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5356,'Pumpkin_Hat_H','Pumpkin Hat',4,20,NULL,200,NULL,2,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,206,'bonus bAllStats,2; bonus2 bSubRace,RC_Demon,5; bonus2 bMagicAddRace,RC_Demon,5;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5811,'Santa_Beard','Santa Beard',4,20,NULL,100,NULL,5,NULL,0,0xFFFFFFFF,7,2,1,NULL,0,0,25,'bonus2 bSubRace,RC_Brute,5;',NULL,NULL);

#REPLACE INTO `item_db2` VALUES (11702,'Moon_Cookie','Moon Cookie',11,0,NULL,10,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'sc_end SC_Poison; sc_end SC_Silence; sc_end SC_Blind; sc_end SC_Confusion; sc_end SC_Curse; sc_end SC_Hallucination; itemskill "AL_BLESSING",7;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (12131,'Lucky_Potion','Lucky Potion',0,2,NULL,100,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'sc_start SC_LUKFood,180000,15;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (12143,'Red_Can','Red Can',2,50000,NULL,300,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'percentheal 25,25;',NULL,NULL);
#Event effect: Summon monster? Probably Rice_Cake. x_x
#REPLACE INTO `item_db2` VALUES (12199,'Rice_Scroll','Rice Scroll',2,0,NULL,0,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (12200,'Event_Cake','Event Cake',2,20,NULL,50,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'itemskill "PR_MAGNIFICAT",3;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (12238,'New_Year_Rice_Cake_1','New Year Rice Cake',0,20,NULL,100,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'percentheal 20,15; sc_start SC_STRFood,1200000,3; sc_start SC_INTFood,1200000,3; sc_start SC_LUKFood,1200000,3; sc_start SC_SpeedUp1,5000,0;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (12239,'New_Year_Rice_Cake_2','New Year Rice Cake',0,20,NULL,100,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'percentheal 20,15; sc_start SC_DEXFood,1200000,3; sc_start SC_AGIFood,1200000,3; sc_start SC_VITFood,1200000,3; sc_start SC_SpeedUp1,5000,0;',NULL,NULL);

# iRO St. Patrick's Day Event 2008
#=============================================================
#REPLACE INTO `item_db2` VALUES (12715,'Black_Treasure_Chest','Black Treasure Chest',2,0,NULL,200,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'callfunc "F_08stpattyseventbox";',NULL,NULL);

# iRO Valentine's Day Event 2009
#=============================================================
#REPLACE INTO `item_db2` VALUES (12742,'Valentine_Gift_Box_M','Valentine Gift Box',2,10,NULL,0,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'getitem 7946,1;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (12743,'Valentine_Gift_Box_F','Valentine Gift Box',2,10,NULL,0,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'getitem 7947,1;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (12744,'Chocolate_Box','Chocolate Box',2,10,NULL,0,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'getitem 558,1;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (14466,'Valentine\'s_Emblem_Box','Valentine\'s Emblem Box',2,10,NULL,0,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'getitem 5817,1;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (7946,'Gold_Ring_Of_Valentine','Gold Ring Of Valentine',3,10,NULL,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (7947,'Silver_Ring_Of_Valentine','Silver Ring Of Valentine',3,10,NULL,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (7948,'Box','Box',3,10,NULL,10,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (5817,'Valentine\'s_Emblem','Valentine\'s Emblem',4,10,NULL,0,NULL,3,NULL,0,0xFFFFFFFF,7,2,136,NULL,0,0,0,'bonus bAtkRate,3; bonus bMatkRate,3; bonus bAllStats,2; bonus bFlee,10; bonus bAspd,1; bonus bMdef,3; bonus2 bSkillAtk,"AL_HEAL",10; bonus2 bSkillHeal,"AL_HEAL",10; bonus2 bSkillHeal,"AM_POTIONPITCHER",10; bonus2 bAddItemHealRate,IG_Potion,10;',NULL,NULL);

# iRO Halloween Event 2009
#=============================================================
#REPLACE INTO `item_db2` VALUES (5668,'Weird_Pumpkin_Hat','Weird Pumpkin Hat',4,20,NULL,0,NULL,5,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,206,'bonus bMdef,5; bonus2 bAddMonsterDropItem,12192,2500;',NULL,NULL);
#REPLACE INTO `item_db2` VALUES (6298,'Crushed_Pumpkin','Crushed Pumpkin',3,0,NULL,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
#REPLACE INTO `item_db2` VALUES (6299,'Worn_Fabric','Worn Fabric',3,0,NULL,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

# Old Tuxedo and Wedding Dress, will display the outfit when worn.
#==================================================================
#REPLACE INTO `item_db2` VALUES (2338,'Wedding_Dress','Wedding Dress',4,43000,NULL,500,NULL,0,NULL,0,0xFFFFFFFE,7,0,16,NULL,0,1,0,NULL,'setoption Option_Wedding,1;','setoption Option_Wedding,0;');
#REPLACE INTO `item_db2` VALUES (7170,'Tuxedo','Tuxedo',4,43000,NULL,10,NULL,0,NULL,0,0xFFFFFFFE,7,1,16,NULL,0,1,0,NULL,'setoption Option_Wedding,1;','setoption Option_Wedding,0;');

