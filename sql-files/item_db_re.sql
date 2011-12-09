#
# Table structure for table `item_db_re`
#

DROP TABLE IF EXISTS `item_db_re`;
CREATE TABLE `item_db_re` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `name_english` varchar(50) NOT NULL default '',
  `name_japanese` varchar(50) NOT NULL default '',
  `type` tinyint(2) unsigned NOT NULL default '0',
  `price_buy` mediumint(10) unsigned default NULL,
  `price_sell` mediumint(10) unsigned default NULL,
  `weight` smallint(5) unsigned NOT NULL default '0',
  `attack` smallint(3) unsigned default NULL,
  `defence` tinyint(3) unsigned default NULL,
  `range` tinyint(2) unsigned default NULL,
  `slots` tinyint(2) unsigned default NULL,
  `equip_jobs` int(12) unsigned default NULL,
  `equip_upper` tinyint(8) unsigned default NULL,
  `equip_genders` tinyint(2) unsigned default NULL,
  `equip_locations` smallint(4) unsigned default NULL,
  `weapon_level` tinyint(2) unsigned default NULL,
  `equip_level` tinyint(3) unsigned default NULL,
  `refineable` tinyint(1) unsigned default NULL,
  `view` smallint(3) unsigned default NULL,
  `script` text,
  `equip_script` text,
  `unequip_script` text,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM;

# Renewal-Specific Database
# specially for renewal-modified items, when compiled in RRMODE entries in this file override item_db.txt and are overriden by item_db2.txt
# 
# Structure of Database:
# REPLACE INTO `item_db_re` VALUES ('ID','Name','Name','Type','Price','Sell','Weight','ATK','DEF','Range','Slot','Job','Upper','Gender','Loc','wLV','eLV','Refineable','View','Script','OnEquip_Script','OnUnequip_Script');

# Matk updates. Work in progress. 
# Daggers
REPLACE INTO `item_db_re` VALUES (13010,'Asura','Asura',4,3000,NULL,600,50,NULL,1,2,0x02000000,7,2,2,1,12,1,1,'bonus bMatk,50;','','');
REPLACE INTO `item_db_re` VALUES (13011,'Asura_','Asura',4,3000,NULL,600,50,NULL,1,3,0x02000000,7,2,2,1,12,1,1,'bonus bMatk,50;','','');
REPLACE INTO `item_db_re` VALUES (1231,'Bazerald','Bazerald',4,20,NULL,500,70,NULL,1,0,0x028F5EEE,7,2,2,4,36,1,1,'bonus bAtkEle,Ele_Fire; bonus bInt,5; bonus bMatk,105;','','');

# Staffs
REPLACE INTO `item_db_re` VALUES (1472,'Staff_Of_Soul','Soul Staff',4,20,NULL,1400,25,NULL,1,0,0x00810204,7,2,34,3,73,1,10,'bonus bInt,5; bonus bAgi,2; bonus bMatk,200; if(isequipped(2677) || isequipped(2711)){ bonus bMatkRate,6; bonus bDex,2; bonus bCastrate,-getrefine(); }','','');
REPLACE INTO `item_db_re` VALUES (1601,'Rod','Rod',4,50,NULL,400,15,NULL,1,3,0x00818315,7,2,2,1,1,1,10,'bonus bMatk,30;','','');
REPLACE INTO `item_db_re` VALUES (1602,'Rod_','Rod',4,50,NULL,400,15,NULL,1,4,0x00818315,7,2,2,1,1,1,10,'bonus bMatk,30;','','');
REPLACE INTO `item_db_re` VALUES (1603,'Rod__','Rod',4,50,NULL,400,15,NULL,1,0,0x00818315,7,2,2,1,1,1,10,'bonus bMatk,30;','','');
REPLACE INTO `item_db_re` VALUES (1604,'Wand','Wand',4,2500,NULL,400,25,NULL,1,2,0x00818315,7,2,2,2,12,1,10,'bonus bInt,1; bonus bMatk,45;','','');
REPLACE INTO `item_db_re` VALUES (1605,'Wand_','Wand',4,2500,NULL,400,25,NULL,1,3,0x00818315,7,2,2,2,12,1,10,'bonus bInt,1; bonus bMatk,45;','','');
REPLACE INTO `item_db_re` VALUES (1606,'Wand__','Wand',4,2500,NULL,400,25,NULL,1,0,0x00818315,7,2,2,2,12,1,10,'bonus bInt,1; bonus bMatk,45;','','');
REPLACE INTO `item_db_re` VALUES (1607,'Staff','Staff',4,9500,NULL,400,40,NULL,1,2,0x00818314,7,2,2,2,12,1,10,'bonus bInt,2; bonus bMatk,70;','','');
REPLACE INTO `item_db_re` VALUES (1608,'Staff_','Staff',4,9500,NULL,400,40,NULL,1,3,0x00818314,7,2,2,2,12,1,10,'bonus bInt,2; bonus bMatk,70;','','');
REPLACE INTO `item_db_re` VALUES (1609,'Staff__','Staff',4,9500,NULL,400,40,NULL,1,0,0x00818314,7,2,2,2,12,1,10,'bonus bInt,2; bonus bMatk,70;','','');
REPLACE INTO `item_db_re` VALUES (1610,'Arc_Wand','Arc Wand',4,45000,NULL,400,60,NULL,1,1,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3; bonus bMatk,95;','','');
REPLACE INTO `item_db_re` VALUES (1611,'Arc_Wand_','Arc Wand',4,45000,NULL,400,60,NULL,1,2,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3; bonus bMatk,95;','','');
REPLACE INTO `item_db_re` VALUES (1612,'Arc_Wand__','Arc Wand',4,45000,NULL,400,60,NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3; bonus bMatk,95;','','');
REPLACE INTO `item_db_re` VALUES (1613,'Mighty_Staff','Mighty Staff',4,20,NULL,700,130,NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bStr,10; bonus bMatk,100; bonus bSPDrainValue,-2;','','');
REPLACE INTO `item_db_re` VALUES (1614,'Blessed_Wand','Wand of Occult',4,20,NULL,700,75,NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3; bonus bMatk,105;','','');
REPLACE INTO `item_db_re` VALUES (1616,'Staff_Of_Wing','Wing Staff',4,20,NULL,500,60,NULL,1,0,0x00810204,7,2,2,4,40,1,10,'bonus bMatk,115; bonus bCastrate,-5;','','');
REPLACE INTO `item_db_re` VALUES (1617,'Survival_Rod','Survivor\'s Rod',4,85000,NULL,1000,50,NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bDex,2; bonus bMatk,120; bonus bMaxHP,300;','','');
REPLACE INTO `item_db_re` VALUES (1618,'Survival_Rod_','Survivor\'s Rod',4,85000,NULL,1000,50,NULL,1,1,0x00818314,7,2,2,3,24,1,10,'bonus bDex,3; bonus bMatk,120; bonus bMaxHP,400;','','');
REPLACE INTO `item_db_re` VALUES (1619,'Survival_Rod2','Survivor\'s Rod',4,85000,NULL,1000,50,NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bInt,2; bonus bMatk,120; bonus bMaxHP,300;','','');
REPLACE INTO `item_db_re` VALUES (1620,'Survival_Rod2_','Survivor\'s Rod',4,85000,NULL,1000,50,NULL,1,1,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3; bonus bMatk,120; bonus bMaxHP,400;','','');
REPLACE INTO `item_db_re` VALUES (1621,'Hypnotist\'s_Staff','Hypnotist\'s Staff',4,43000,NULL,500,70,NULL,1,1,0x00000001,7,2,2,3,30,1,10,'bonus bInt,1; bonus bMatk,120;','','');
REPLACE INTO `item_db_re` VALUES (1622,'Hypnotist\'s_Staff_','Hypnotist\'s Staff',4,20,NULL,500,70,NULL,1,2,0x00000001,7,2,2,3,30,1,10,'bonus bInt,1; bonus bMatk,120;','','');
REPLACE INTO `item_db_re` VALUES (1624,'Lich_Bone_Wand','Lich\'s Bone Wand',4,20,NULL,800,60,NULL,1,2,0x00018314,2,2,2,3,70,1,10,'bonus bInt,1; bonus bDex,1; bonus bAtkEle,Ele_Undead; bonus bMatk,170; bonus3 bAutoSpellWhenHit,"NPC_WIDECURSE",5,10+getrefine(); if(getrefine()>=9){ bonus bMatkRate,3; bonus bMaxSP,300; }','','');
REPLACE INTO `item_db_re` VALUES (1625,'Healing_Staff','Healing Staff',4,20,NULL,400,10,NULL,1,0,0x00008110,7,2,2,3,55,1,10,'bonus bAtkEle,Ele_Holy; bonus bMatk,105; bonus bHealPower,(getrefine()*3/2);','','');
REPLACE INTO `item_db_re` VALUES (1626,'Piercing_Staff','Piercing Staff',4,20,NULL,500,80,NULL,1,0,0x00018314,2,2,2,3,70,1,10,'bonus bInt,4; bonus bMatk,145; bonus bIgnoreMdefRate,10+getrefine();','','');
REPLACE INTO `item_db_re` VALUES (1629,'Walking_Stick','Gentleman\'s Staff',4,20,NULL,500,40,NULL,1,1,0x00818314,7,2,2,4,50,1,10,'bonus bMatk,125; bonus bDex,1; if (isequipped(5045)) { bonus bDex,2; bonus bInt,2; bonus bSPrecovRate,5; bonus bMatkRate,getrefine(); }','','');
REPLACE INTO `item_db_re` VALUES (1630,'Release_Of_Wish','Release of Wish',4,20,NULL,500,30,NULL,1,0,0x00810204,7,2,2,3,50,1,10,'bonus bMatk,125; bonus bInt,3; bonus bHealPower,5; autobonus "{ bonus2 bSPRegenRate,100,2000; bonus2 bHPRegenRate,50,2000; }",10,10000,BF_MAGIC,"{ specialeffect2 EF_HEAL; }";','','');
REPLACE INTO `item_db_re` VALUES (1636,'Thorn_Staff','Thorn Staff of Darkness',4,20,NULL,700,60,NULL,1,0,0x00018314,2,2,2,4,75,1,10,'bonus bInt,3; bonus bDex,3; bonus bMatk,160; bonus bIgnoreMdefRate,getrefine(); bonus bDelayRate,-(getrefine()*3/2);','','');
REPLACE INTO `item_db_re` VALUES (1637,'Eraser','Eraser',4,20,NULL,500,80,NULL,1,0,0x00018314,2,2,2,4,70,1,10,'bonus bMatk,170; bonus bInt,3; bonus bDex,2; bonus bSPrecovRate,8; if( getrefine() > 9 ) bonus5 bAutoSpell,"NPC_WIDESOULDRAIN",3,5,BF_MAGIC,0; else bonus5 bAutoSpell,"NPC_WIDESOULDRAIN",1,5,BF_MAGIC,0;','','');
REPLACE INTO `item_db_re` VALUES (2000,'Destruction_Rod','Staff of Destruction',4,20,NULL,2500,130,NULL,1,1,0x00000200,2,2,34,4,80,1,23,'bonus bMatk,280; bonus bInt,3; bonus bAgi,10; bonus bUseSPrate,(getrefine()*2); bonus3 bAutoSpellWhenHit,"WZ_JUPITEL",5,(getrefine()*20); bonus2 bCastrate,366,-50;','','');
REPLACE INTO `item_db_re` VALUES (2001,'Divine_Cross','Divine Cross',4,20,NULL,1500,120,NULL,1,0,0x00008100,7,2,34,4,70,1,23,'bonus bAtkEle,Ele_Holy; bonus bMatk,210; bonus bDex,4; bonus2 bSubRace,RC_Demon,15; bonus2 bSubRace,RC_Undead,15; if (isequipped(2677) || isequipped(2711)) { bonus bMatkRate,10; bonus bDex,2; bonus2 bSubRace,RC_Demon,10; bonus2 bSubRace,RC_Undead,10; };','','');

# Books
REPLACE INTO `item_db_re` VALUES (1560,'Diary_Of_Great_Sage','Sage\'s Diary',4,20,NULL,1100,100,NULL,1,2,0x00410100,7,2,2,3,60,1,15,'bonus bMatk,120; if(readparam(bStr)>=50) bonus bAspdRate,5; if(readparam(bInt)>=70) bonus bMatkRate,5;','','');
REPLACE INTO `item_db_re` VALUES (1561,'Hardback','Hardcover Book',4,20,NULL,1500,140,NULL,1,1,0x00410100,7,2,2,4,55,1,15,'bonus bStr,3; bonus bDex,2;','','');
REPLACE INTO `item_db_re` VALUES (1572,'Principles_Of_Magic','Principles of Magic',4,20,NULL,300,60,NULL,1,2,0x00410100,7,2,2,3,60,1,15,'bonus bMatk,160; bonus bInt,3; bonus bSPrecovRate,5;','','');
REPLACE INTO `item_db_re` VALUES (1573,'Ancient_Magic','Ancient Magic',4,20,NULL,700,30,NULL,1,2,0x00410100,7,2,2,3,70,1,15,'bonus bMatk,140; if (isequipped(2334) || isequipped(2372)) { bonus bMdef,8; bonus bMaxSPRate,10; bonus bInt,4; };','','');
REPLACE INTO `item_db_re` VALUES (1564,'Encyclopedia','Encyclopedia',4,20,NULL,2000,110,NULL,1,2,0x00410100,7,2,2,3,70,1,15,'bonus bMatk,100; bonus bInt,3; bonus bDex,2; bonus bCritical,20+(readparam(bLuk)*2);','','');

# Training Grounds Items
REPLACE INTO `item_db_re` VALUES (2819,'Swordman_Manual','Swordsman Manual',5,0,NULL,100,NULL,0,NULL,0,0x00000001,7,2,136,NULL,1,0,0,'bonus bMaxSP,100; skill \"SM_BASH\",1; skill \"SM_PROVOKE\",1; skill \"SM_MAGNUM\",1;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2820,'Thief_Manual','Thief Manual',5,0,NULL,100,NULL,0,NULL,0,0x00000001,7,2,136,NULL,1,0,0,'bonus bMaxSP,100; skill \"TF_DOUBLE\",3; skill \"TF_STEAL\",1; skill \"TF_HIDING\",1; skill \"TF_POISON\",1;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2821,'Acolyte_Manual','Acolyte Manual',5,0,NULL,100,NULL,0,NULL,0,0x00000001,7,2,136,NULL,1,0,0,'bonus bMaxSP,100; skill \"AL_HEAL\",1; skill \"AL_INCAGI\",1; skill \"AL_BLESSING\",1;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2822,'Archer_Manual','Archer Manual',5,0,NULL,100,NULL,0,NULL,0,0x00000001,7,2,136,NULL,1,0,0,'bonus bMaxSP,100; skill \"AC_OWL\",1; skill \"AC_CONCENTRATION\",1;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2823,'Merchant_Manual','Merchant Manual',5,0,NULL,100,NULL,0,NULL,0,0x00000001,7,2,136,NULL,1,0,0,'bonus bMaxSP,100; skill \"MC_DISCOUNT\",1; skill \"MC_OVERCHARGE\",10; skill \"MC_IDENTIFY\",1; skill \"MC_MAMMONITE\",1;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2824,'Mage_Manual','Mage Manual',5,0,NULL,100,NULL,0,NULL,0,0x00000001,7,2,136,NULL,1,0,0,'bonus bMaxSP,100; skill \"MG_SRECOVERY\",1; skill \"MG_COLDBOLT\",1; skill \"MG_FIREWALL\",1; skill \"MG_FIREBOLT\",1;',NULL,NULL);

# //[Ind] keeping these 2 here until we confirm: these ids are conflicting, I think they're wrong.
# // Minstrel And Wanderer Cough Drop
# //11513,Cough_Drop,Cough Drop,3,200,,10,,,,,,,,,,,,,{},{},{}
# 
# // Genetic Cure Free
# //11518,Cure_Free,Cure Free,0,10,,30,,,,,0xFFFFFFFF,7,2,,,,,,{ itemheal rand(1000,1200),0; sc_end SC_Silence; sc_end SC_Bleeding; sc_end SC_Curse; },{},{}
# 
# 
# // Unknown Item. Rune Knight Armor???
# //15002,Rune_Plate,Rune Plate,3,10,,10,,,,,,,,,,,,,{},{},{}
# 
