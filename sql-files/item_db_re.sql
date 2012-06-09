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
  `atk:matk` varchar(10) default '',
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
# specially for renewal-modified items, when compiled in REMODE entries in this file override item_db.txt and are overriden by item_db2.txt
# 
# Structure of Database:
# REPLACE INTO `item_db_re` VALUES ('ID','Name','Name','Type','Price','Sell','Weight','ATK:MATK','DEF','Range','Slot','Job','Upper','Gender','Loc','wLV','eLV','Refineable','View','Script','OnEquip_Script','OnUnequip_Script');

# Usable Items
REPLACE INTO `item_db` VALUES (678,'Poison_Bottle','Poison Bottle',2,5000,NULL,100,NULL,NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'if(Class == Job_Assassin_Cross || Class == Job_Guillotine_Cross || Class == Job_Guillotine_Cross_T) { sc_start SC_DPoison,60000,0; sc_start SC_ASPDPOTION3,60000,0; } else percentheal -100,-100;',NULL,NULL);

# Matk updates. Work in progress. 
# Daggers
REPLACE INTO `item_db_re` VALUES (645,'Center_Potion','Concentration Potion',2,800,NULL,100,'',NULL,NULL,NULL,0xFFFFFFFF,7,2,NULL,NULL,NULL,NULL,NULL,'sc_start SC_ASPDPOTION0,1800000,4;','','');
REPLACE INTO `item_db_re` VALUES (656,'Awakening_Potion','Awakening Potion',2,1500,NULL,150,'',NULL,NULL,NULL,0xFFF7FEEF,7,2,NULL,NULL,40,NULL,NULL,'sc_start SC_ASPDPOTION1,1800000,6;','','');
REPLACE INTO `item_db_re` VALUES (657,'Berserk_Potion','Berserk Potion',2,3000,NULL,200,'',NULL,NULL,NULL,0x01E646A6,7,2,NULL,NULL,85,NULL,NULL,'sc_start SC_ASPDPOTION2,1800000,9;','','');
REPLACE INTO `item_db_re` VALUES (13010,'Asura','Asura',4,3000,NULL,600,'50:50',NULL,1,2,0x02000000,7,2,2,1,12,1,1,'','','');
REPLACE INTO `item_db_re` VALUES (13011,'Asura_','Asura',4,3000,NULL,600,'50:50',NULL,1,3,0x02000000,7,2,2,1,12,1,1,'','','');
REPLACE INTO `item_db_re` VALUES (1231,'Bazerald','Bazerald',4,20,NULL,500,'70:105',NULL,1,0,0x028F5EEE,7,2,2,4,36,1,1,'bonus bAtkEle,Ele_Fire; bonus bInt,5;','','');

# Staffs
REPLACE INTO `item_db_re` VALUES (1472,'Staff_Of_Soul','Soul Staff',4,20,NULL,1400,'25:200',NULL,1,0,0x00810204,7,2,34,3,73,1,10,'bonus bInt,5; bonus bAgi,2; if(isequipped(2677) || isequipped(2711)){ bonus bMatkRate,6; bonus bDex,2; bonus bCastrate,-getrefine(); }','','');
REPLACE INTO `item_db_re` VALUES (1601,'Rod','Rod',4,50,NULL,400,'15:30',NULL,1,3,0x00818315,7,2,2,1,1,1,10,'','','');
REPLACE INTO `item_db_re` VALUES (1602,'Rod_','Rod',4,50,NULL,400,'15:30',NULL,1,4,0x00818315,7,2,2,1,1,1,10,'','','');
REPLACE INTO `item_db_re` VALUES (1603,'Rod__','Rod',4,50,NULL,400,'15:30',NULL,1,0,0x00818315,7,2,2,1,1,1,10,'','','');
REPLACE INTO `item_db_re` VALUES (1604,'Wand','Wand',4,2500,NULL,400,'25:45',NULL,1,2,0x00818315,7,2,2,2,12,1,10,'','','');
REPLACE INTO `item_db_re` VALUES (1605,'Wand_','Wand',4,2500,NULL,400,'25:45',NULL,1,3,0x00818315,7,2,2,2,12,1,10,'','','');
REPLACE INTO `item_db_re` VALUES (1606,'Wand__','Wand',4,2500,NULL,400,'25:45',NULL,1,0,0x00818315,7,2,2,2,12,1,10,'bonus bInt,1;','','');
REPLACE INTO `item_db_re` VALUES (1607,'Staff','Staff',4,9500,NULL,400,'40:70',NULL,1,2,0x00818314,7,2,2,2,12,1,10,'bonus bInt,2;','','');
REPLACE INTO `item_db_re` VALUES (1608,'Staff_','Staff',4,9500,NULL,400,'40:70',NULL,1,3,0x00818314,7,2,2,2,12,1,10,'bonus bInt,2;','','');
REPLACE INTO `item_db_re` VALUES (1609,'Staff__','Staff',4,9500,NULL,400,'40:70',NULL,1,0,0x00818314,7,2,2,2,12,1,10,'bonus bInt,2;','','');
REPLACE INTO `item_db_re` VALUES (1610,'Arc_Wand','Arc Wand',4,45000,NULL,400,'60:95',NULL,1,1,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3;','','');
REPLACE INTO `item_db_re` VALUES (1611,'Arc_Wand_','Arc Wand',4,45000,NULL,400,'60:95',NULL,1,2,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3;','','');
REPLACE INTO `item_db_re` VALUES (1612,'Arc_Wand__','Arc Wand',4,45000,NULL,400,'60:95',NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3;','','');
REPLACE INTO `item_db_re` VALUES (1613,'Mighty_Staff','Mighty Staff',4,20,NULL,700,'130:100',NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bStr,10; bonus bSPDrainValue,-2;','','');
REPLACE INTO `item_db_re` VALUES (1614,'Blessed_Wand','Wand of Occult',4,20,NULL,700,'75:105',NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3;','','');
REPLACE INTO `item_db_re` VALUES (1615,'Bone_Wand','Evil Bone Wand',4,20,NULL,700,'40:110',NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bInt,4; bonus bAtkEle,Ele_Undead; bonus bMatkRate,15;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1616,'Staff_Of_Wing','Wing Staff',4,20,NULL,500,'60:115',NULL,1,0,0x00810204,7,2,2,4,40,1,10,'bonus bCastrate,-5;','','');
REPLACE INTO `item_db_re` VALUES (1617,'Survival_Rod','Survivor\'s Rod',4,85000,NULL,1000,'50:120',NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bDex,2; bonus bMaxHP,300;','','');
REPLACE INTO `item_db_re` VALUES (1618,'Survival_Rod_','Survivor\'s Rod',4,85000,NULL,1000,'50:120',NULL,1,1,0x00818314,7,2,2,3,24,1,10,'bonus bDex,3; bonus bMaxHP,400;','','');
REPLACE INTO `item_db_re` VALUES (1619,'Survival_Rod2','Survivor\'s Rod',4,85000,NULL,1000,'50:120',NULL,1,0,0x00818314,7,2,2,3,24,1,10,'bonus bInt,2; bonus bMaxHP,300;','','');
REPLACE INTO `item_db_re` VALUES (1620,'Survival_Rod2_','Survivor\'s Rod',4,85000,NULL,1000,'50:120',NULL,1,1,0x00818314,7,2,2,3,24,1,10,'bonus bInt,3; bonus bMaxHP,400;','','');
REPLACE INTO `item_db_re` VALUES (1621,'Hypnotist\'s_Staff','Hypnotist\'s Staff',4,43000,NULL,500,'70:120',NULL,1,1,0x00000001,7,2,2,3,30,1,10,'bonus bInt,1;','','');
REPLACE INTO `item_db_re` VALUES (1622,'Hypnotist\'s_Staff_','Hypnotist\'s Staff',4,20,NULL,500,'70:120',NULL,1,2,0x00000001,7,2,2,3,30,1,10,'bonus bInt,1;','','');
REPLACE INTO `item_db_re` VALUES (1623,'Mighty_Staff_C','Mighty Staff',4,1,NULL,0,'165:100',NULL,1,0,0x00818314,7,2,2,3,1,0,10,'bonus bStr,10; bonus bInt,4; bonus bSPDrainValue,-1;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1624,'Lich_Bone_Wand','Lich\'s Bone Wand',4,20,NULL,800,'60:170',NULL,1,2,0x00018314,2,2,2,3,70,1,10,'bonus bInt,1; bonus bDex,1; bonus bAtkEle,Ele_Undead; bonus3 bAutoSpellWhenHit,"NPC_WIDECURSE",5,10+getrefine(); if(getrefine()>=9){ bonus bMatkRate,3; bonus bMaxSP,300; }','','');
REPLACE INTO `item_db_re` VALUES (1625,'Healing_Staff','Healing Staff',4,20,NULL,400,'10:105',NULL,1,0,0x00008110,7,2,2,3,55,1,10,'bonus bAtkEle,Ele_Holy; bonus bHealPower,(getrefine()*3/2);','','');
REPLACE INTO `item_db_re` VALUES (1626,'Piercing_Staff','Piercing Staff',4,20,NULL,500,'80:145',NULL,1,0,0x00018314,2,2,2,3,70,1,10,'bonus bInt,4; bonus bIgnoreMdefRate,10+getrefine();','','');
REPLACE INTO `item_db_re` VALUES (1627,'Staffy','Staffy',4,20,NULL,0,'40:120',NULL,1,0,0x00818314,7,2,2,1,0,0,10,'bonus2 bAddRace,RC_Boss,50; bonus2 bAddRace,RC_NonBoss,50;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1628,'Survival_Rod_C','Refined Survivor\'s Rod',4,1,NULL,0,'71:145',NULL,1,0,0x00818314,7,2,2,3,0,0,10,'bonus bDex,4; bonus bMaxHP,500;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1629,'Walking_Stick','Gentleman\'s Staff',4,20,NULL,500,'40:125',NULL,1,1,0x00818314,7,2,2,4,50,1,10,'bonus bDex,1; if (isequipped(5045)) { bonus bDex,2; bonus bInt,2; bonus bSPrecovRate,5; bonus bMatkRate,getrefine(); }','','');
REPLACE INTO `item_db_re` VALUES (1630,'Release_Of_Wish','Release of Wish',4,20,NULL,500,'30:125',NULL,1,0,0x00810204,7,2,2,3,50,1,10,'bonus bInt,3; bonus bHealPower,5; autobonus "{ bonus2 bSPRegenRate,100,2000; bonus2 bHPRegenRate,50,2000; }",10,10000,BF_MAGIC,"{ specialeffect2 EF_HEAL; }";','','');
REPLACE INTO `item_db_re` VALUES (1631,'Holy_Stick','Holy Stick',4,20,NULL,500,'50:140',NULL,1,1,0x00008100,7,2,2,4,70,1,10,'bonus bAtkEle,Ele_Holy; bonus2 bCastrate,156,-25; bonus2 bCastrate,77,-25; bonus2 bCastrate,79,-25;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1632,'BF_Staff1','Warlock\'s Magic Wand',4,20,NULL,0,'70:125',NULL,1,0,0x00818314,7,2,2,3,80,1,10,'bonus bInt,4; bonus bDex,3; bonus2 bIgnoreMdefRate,RC_DemiHuman,25; bonus3 bAddEff,Eff_Stun,500,ATF_SKILL; bonus bUnbreakableWeapon,0;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1633,'BF_Staff2','Warlock\'s Battle Wand',4,20,NULL,0,'70:125',NULL,1,0,0x00818314,7,2,2,3,80,1,10,'bonus bInt,3; bonus bDex,3; bonus2 bMagicAddRace,RC_DemiHuman,15; bonus3 bAddEff,Eff_Stun,500,ATF_SKILL; bonus bUnbreakableWeapon,0;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1634,'BF_Staff3','Strong Recovery Wand',4,20,NULL,0,'70:125',NULL,1,0,0x00818314,7,2,2,3,80,1,10,'bonus bHealPower,14; bonus2 bSPRegenRate,5,10000; bonus bUnbreakableWeapon,0;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1635,'BF_Staff4','Speedy Recovery Wand',4,20,NULL,0,'70:125',NULL,1,0,0x00818314,7,2,2,3,80,1,10,'bonus bInt,3; bonus bDex,2; bonus bDelayRate,-15; bonus2 bSPRegenRate,5,10000; bonus bUnbreakableWeapon,0;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1636,'Thorn_Staff','Thorn Staff of Darkness',4,20,NULL,700,'60:160',NULL,1,0,0x00018314,2,2,2,4,75,1,10,'bonus bInt,3; bonus bDex,3; bonus bIgnoreMdefRate,getrefine(); bonus bDelayRate,-(getrefine()*3/2);','','');
REPLACE INTO `item_db_re` VALUES (1637,'Eraser','Eraser',4,20,NULL,500,'80:170',NULL,1,0,0x00018314,2,2,2,4,70,1,10,'bonus bInt,3; bonus bDex,2; bonus bSPrecovRate,8; if( getrefine() > 9 ) bonus5 bAutoSpell,"NPC_WIDESOULDRAIN",3,5,BF_MAGIC,0; else bonus5 bAutoSpell,"NPC_WIDESOULDRAIN",1,5,BF_MAGIC,0;','','');
REPLACE INTO `item_db_re` VALUES (1638,'Healing_Staff_C','Staff Of Healing',4,20,NULL,0,'10:125',NULL,1,0,0x00008110,7,2,2,3,1,0,10,'bonus bAtkEle,Ele_Holy; bonus bHealPower,(getrefine()*3/2);',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1639,'N_Rod','Novice Rod',4,0,NULL,0,'15:32',NULL,1,3,0x00818315,7,2,2,1,1,0,10,'',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1640,'Krieger_Onehand_Staff1','Glorious Arc Wand',4,20,NULL,0,'70:135',NULL,1,0,0x00818314,7,2,2,4,80,1,10,'bonus2 bMagicAddRace,RC_DemiHuman,15; bonus2 bIgnoreMdefRate,RC_DemiHuman,25 + ((getrefine() > 5) ? 5 : 0); bonus bUnbreakableWeapon,0; if(getrefine() > 8) { bonus bMatkRate,5; bonus bCastrate,-5; bonus bDelayRate,-5; }',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1641,'Krieger_Onehand_Staff2','Glorious Cure Wand',4,20,NULL,0,'70:135',NULL,1,0,0x00818314,7,2,2,4,80,1,10,'bonus bHealPower,14; bonus bDelayRate,-10; bonus bUnbreakableWeapon,0; if(getrefine() > 5) { bonus2 bIgnoreMdefRate,RC_DemiHuman,5; bonus bHealPower,5+(getrefine()-5)*2; } if(getrefine() > 8) bonus5 bAutoSpellOnSkill,\"AL_HEAL\",\"AL_HEAL\",10,100,1; if(getrefine() > 9) { bonus bHealPower,10; }',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1642,'Staff_Of_Darkness','Staff Of Darkness',4,20,NULL,0,'100:120',NULL,1,0,0x00818314,7,2,2,2,0,0,10,'bonus bCastrate,-5; bonus bInt,2;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1643,'Dead_Tree_Cane','Dead Tree Cane',4,20,NULL,100,'100:155',NULL,1,0,0x00818314,7,2,2,4,70,1,10,'bonus bInt,4; if (getrefine()>5) { bonus bInt,getrefine()-5; bonus bMaxHP,-200; bonus bMaxSP,-100; }',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1644,'Piercing_Staff_M','Staff of Piercing',4,20,NULL,500,'80:145',NULL,1,0,0x00018314,2,2,2,3,70,1,10,'bonus bInt,4; bonus bIgnoreMdefRate,10+getrefine();',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1645,'Lich_Bone_Wand_M','Lich\'s Bone Wand',4,20,NULL,800,'60:170',NULL,1,2,0x00018314,2,2,2,3,70,1,10,'bonus bInt,1; bonus bDex,1; bonus bAtkEle,Ele_Undead; bonus3 bAutoSpellWhenHit,\"NPC_WIDECURSE\",5,10+getrefine(); if(getrefine()>=9){ bonus bMatkRate,3; bonus bMaxSP,300; }',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1646,'La\'cryma_Stick','La\'cryma Stick',4,20,NULL,500,'30:180',NULL,1,2,0x00010204,2,2,2,3,50,1,10,'bonus bInt,4; bonus bMdef,1; bonus2 bSkillAtk,\"WZ_STORMGUST\",getrefine(); if (getrefine() > 9) bonus2 bCastRate,\"WZ_STORMGUST\",-8;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1647,'Croce_Staff','Croce Staff',4,20,NULL,500,'30:175',NULL,1,1,0x00008110,2,2,2,3,50,1,10,'bonus bAtkEle,Ele_Holy; bonus bInt,4; bonus4 bAutoSpellOnSkill,\"AL_HEAL\",\"AL_BLESSING\",getskilllv(\"AL_BLESSING\")>1?getskilllv(\"AL_BLESSING\"):1,20;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1648,'Staff_Of_Bordeaux','Staff Of Bordeaux',4,20,NULL,500,'30:170',NULL,1,0,0x00010200,2,2,2,4,50,1,10,'bonus bInt,2; bonus bDex,1; if(getskilllv(\"SA_DRAGONOLOGY\") == 5) { bonus bUseSPrate,-15; bonus bInt,3; }',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1649,'Rafini_Staff','Laphine Staff',4,20,NULL,500,'30:180',NULL,1,0,0x00818315,7,2,2,3,100,1,10,'/* bonus bFixedCastRate,-getrefine(); */',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1650,'P_Staff1','Eden Staff I',4,0,NULL,0,'60:125',NULL,1,0,0x00818314,7,2,2,2,26,0,10,'bonus bInt,2;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1651,'P_Staff2','Eden Staff II',4,0,NULL,0,'60:150',NULL,1,0,0x00818314,7,2,2,2,40,0,10,'bonus bInt,3;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1652,'Tourist_Staff','Tourist Staff',4,0,NULL,500,'35:0',NULL,1,0,0x00818315,7,2,2,1,1,0,10,'bonus bInt,2; bonus bAgi,1;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1653,'Staff_Of_Healing_C','Staff of Healing',4,'10:100',NULL,0,10,NULL,1,0,0x00008110,7,2,2,3,1,0,10,'bonus bAtkEle,Ele_Holy;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1654,'Mental_Stick','Mental Stick',4,20,NULL,500,'40:170',NULL,1,1,0x00818315,7,2,2,3,102,1,10,'if (getrefine() > 5) { /* bonus bSkillAtk,\"SO_PSYCHIC_WAVE\",(getrefine()-5)*2; */ bonus bMaxHPRate,-(getrefine()-5)*2; } /* bonus2 bVariableCastTime,\"SO_PSYCHIC_WAVE\",-3000; bonus2 bUseSPAmount,\"SO_PSYCHIC_WAVE\",-60; */',NULL,'itemheal 0,-100;');
# 1655,
# 1656,
REPLACE INTO `item_db_re` VALUES (1657,'Mercy_Staff1','Mercy Staff I',4,20,NULL,500,'30:160',NULL,1,2,0x00000100,7,2,2,3,100,1,10,'bonus bInt,2; bonus bAtkEle,Ele_Holy; bonus bHealPower,10; if(isequipped(2471,2569,15029)){ bonus bHealPower,25; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1658,'P_Staff3','Eden Staff III',4,0,NULL,0,'60:170',NULL,1,0,0x00818314,7,2,2,3,60,0,10,'bonus bInt,4;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1659,'Light_of_Recovery','Light of Recovery',4,56000,NULL,400,'30:160',NULL,1,1,0x00000100,7,2,2,4,110,1,10,'bonus bAtkEle,Ele_Holy; bonus bUnbreakableWeapon,0; bonus bHealPower,(getrefine()*3/2); bonus bUseSPrate,(getrefine()*3);',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1660,'Mercy_Staff2','Mercy Staff II',4,20,NULL,500,'30:180',NULL,1,1,0x00000100,7,2,2,3,130,1,10,'bonus bAtkEle,Ele_Holy; bonus bInt,4; bonus bHealPower,20; if(isequipped(2471,2569,15029)){ bonus bHealPower,45; /* bonus3 bAutoSpellWhenHit,\"AB_SILENTIUM\",1,10; */ };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (1661,'Mental_Destroyer','Mental Destroyer',4,20,NULL,1400,'100:50',NULL,1,0,0x00000200,7,2,2,4,95,1,10,'bonus bInt,10; bonus bMdef,20; bonus bUnbreakableWeapon,0; bonus2 bSPVanishRate,10000,5; if(getrefine()>5) { bonus2 bSPVanishRate,10000,5; }',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2000,'Destruction_Rod','Staff of Destruction',4,20,NULL,2500,'130:280',NULL,1,1,0x00000200,2,2,34,4,80,1,23,'bonus bInt,3; bonus bAgi,10; bonus bUseSPrate,(getrefine()*2); bonus3 bAutoSpellWhenHit,"WZ_JUPITEL",5,(getrefine()*20); bonus2 bCastrate,366,-50;','','');
REPLACE INTO `item_db_re` VALUES (2001,'Divine_Cross','Divine Cross',4,20,NULL,1500,'120:210',NULL,1,0,0x00008100,7,2,34,4,70,1,23,'bonus bAtkEle,Ele_Holy; bonus bDex,4; bonus2 bSubRace,RC_Demon,15; bonus2 bSubRace,RC_Undead,15; if (isequipped(2677) || isequipped(2711)) { bonus bMatkRate,10; bonus bDex,2; bonus2 bSubRace,RC_Demon,10; bonus2 bSubRace,RC_Undead,10; };','','');
REPLACE INTO `item_db_re` VALUES (2002,'Krieger_Twohand_Staff1','Glorious Destruction Staff',4,20,NULL,0,'70:210',NULL,1,0,0x00018314,7,2,34,4,80,1,23,'bonus bMatkRate,(getrefine()>14)?14:getrefine(); bonus2 bMagicAddRace,RC_DemiHuman,15; bonus2 bIgnoreMdefRate,RC_DemiHuman,25; bonus bUnbreakableWeapon,0; if(getrefine() > 5) { bonus2 bMagicAddRace,RC_DemiHuman,(getrefine()-5)*2; bonus2 bIgnoreMdefRate,RC_DemiHuman,(getrefine()-5)*2; } if(getrefine() > 8) { bonus5 bAutoSpellOnSkill,\"WZ_STORMGUST\",\"MG_SAFETYWALL\",10,200,1; bonus5 bAutoSpellOnSkill,\"WZ_METEOR\",\"MG_SAFETYWALL\",10,200,1; bonus5 bAutoSpellOnSkill,\"WZ_VERMILION\",\"MG_SAFETYWALL\",10,200,1; }',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2003,'Destruction_Rod_M','Staff of Destruction',4,20,NULL,2500,'130:280',NULL,1,1,0x00000200,2,2,34,4,80,1,23,'bonus bInt,3; bonus bAgi,10; bonus bUseSPrate,(getrefine()*2); bonus3 bAutoSpellWhenHit,\"WZ_JUPITEL\",5,(getrefine()*20); bonus2 bCastrate,366,-50;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2004,'Kronos','Kronos',4,20,NULL,1000,'30:240',NULL,1,0,0x00010204,2,2,34,4,50,1,23,'bonus bInt,3+(getrefine()/2); bonus bMaxHP,300+(50*getrefine()/2); autobonus \"{ bonus bMatkRate,12; bonus buseSPRate,20; }\",1,5000,BF_MAGIC,\"{ specialeffect2 EF_ENHANCE; }\";',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2005,'Dea_Staff','Dea Staff',4,20,NULL,1000,'30:220',NULL,1,1,0x00008110,2,2,34,3,50,1,23,'bonus bAtkEle,Ele_Holy; bonus bMatkRate,getrefine()/2; bonus bInt,6; bonus bVit,2; autobonus3 \"{ }\",20,1000,\"AL_HEAL\",\"{ specialeffect2 EF_MAGICALATTHIT;  heal 0,200;  }\";',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2006,'G_Staff_Of_Light','Staff Of Light',4,20,NULL,1900,'80:150',NULL,1,0,0x00810204,7,2,34,4,60,1,23,'bonus bAtkEle,Ele_Holy; bonus bInt,6;',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2007,'Golden_Rod_Staff1','Golden Rod Staff I',4,20,NULL,900,'30:230',NULL,1,2,0x00000200,7,2,34,4,100,1,23,'bonus bAtkEle,Ele_Wind; bonus bInt,3; bonus2 bSkillAtk,\"WZ_JUPITEL\",12; if (isequipped(2467,2859,15025)) { bonus2 bSubEle,Ele_Earth,-50; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2008,'Aqua_Staff1','Aqua Staff I',4,20,NULL,900,'30:230',NULL,1,2,0x00000200,7,2,34,4,100,1,23,'bonus bAtkEle,Ele_Water; bonus bInt,3; bonus2 bSkillAtk,\"MG_COLDBOLT\",12; bonus2 bSkillAtk,\"MG_FROSTDIVER\",12; if (isequipped(2468,2860,15026)) { bonus2 bSubEle,Ele_Wind,-50; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2009,'Crimson_Staff1','Crimson Staff I',4,20,NULL,900,'30:230',NULL,1,2,0x00000200,7,2,34,4,100,1,23,'bonus bAtkEle,Ele_Fire; bonus bInt,3; bonus2 bSkillAtk,\"MG_FIREBOLT\",10; bonus2 bSkillAtk,\"MG_FIREBALL\",10; if (isequipped(2469,2861,15027)) { bonus2 bSubEle,Ele_Water,-50; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2010,'Forest_Staff1','Forest Staff I',4,20,NULL,900,'30:230',NULL,1,2,0x00000200,7,2,34,4,100,1,23,'bonus bAtkEle,Ele_Earth; bonus bInt,3; bonus2 bSkillAtk,\"WZ_EARTHSPIKE\",10; bonus2 bSkillAtk,\"WZ_HEAVENDRIVE\",10; if (isequipped(2470,2862,15028)) { bonus2 bSubEle,Ele_Fire,-50; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2011,'Golden_Rod_Staff2','Golden Rod Staff II',4,'30:270',NULL,900,30,NULL,1,1,0x00000200,7,2,34,4,130,1,10,'bonus bAtkEle,Ele_Wind; bonus bInt,5; bonus2 bSkillAtk,\"WZ_JUPITEL\",30; if (isequipped(2467,2859,15025)) { bonus2 bSubEle,Ele_Earth,-50; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2012,'Aqua_Staff2','Aqua Staff II',4,20,NULL,900,'30:270',NULL,1,1,0x00000200,7,2,34,4,130,1,10,'bonus bAtkEle,Ele_Water; bonus bInt,5; bonus2 bSkillAtk,\"MG_COLDBOLT\",30; bonus2 bSkillAtk,\"MG_FROSTDIVER\",30; if (isequipped(2468,2860,15026)) { bonus2 bSubEle,Ele_Wind,-50; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2013,'Crimson_Staff2','Crimson Staff II',4,20,NULL,900,'30:270',NULL,1,1,0x00000200,7,2,34,4,130,1,10,'bonus bAtkEle,Ele_Fire; bonus bInt,5; bonus2 bSkillAtk,\"MG_FIREBOLT\",30; bonus2 bSkillAtk,\"MG_FIREBALL\",30; if (isequipped(2469,2861,15027)) { bonus2 bSubEle,Ele_Water,-50; };',NULL,NULL);
REPLACE INTO `item_db_re` VALUES (2014,'Forest_Staff2','Forest Staff II',4,20,NULL,900,'30:270',NULL,1,1,0x00000200,7,2,34,4,130,1,10,'bonus bAtkEle,Ele_Earth; bonus bInt,5; bonus2 bSkillAtk,\"WZ_EARTHSPIKE\",30; bonus2 bSkillAtk,\"WZ_HEAVENDRIVE\",30; if (isequipped(2470,2862,15028)) { bonus2 bSubEle,Ele_Fire,-50; };',NULL,NULL);
# 2015,
REPLACE INTO `item_db_re` VALUES (2016,'Bellum_Arcwand','Bellum Arcwand',4,20,NULL,800,'110:220',NULL,1,0,0x00818314,7,2,34,4,95,1,10,'bonus bUnbreakableWeapon,0; bonus2 bMagicAddRace,RC_DemiHuman,25; bonus bIgnoreMdefRate,25; if(getrefine() > 5) { bonus2 bMagicAddRace,RC_DemiHuman,15; } if(getrefine() > 8) { bonus bCastrate,-20; }',NULL,NULL);

# Maces
REPLACE INTO `item_db_re` VALUES (1549,'Pilebuncker','Pile Bunker',4,10000,NULL,3500,450,NULL,1,0,0x00000400,8,2,2,3,99,1,8,NULL,NULL,NULL);

# Books
REPLACE INTO `item_db_re` VALUES (1560,'Diary_Of_Great_Sage','Sage\'s Diary',4,20,NULL,1100,'100:120',NULL,1,2,0x00410100,7,2,2,3,60,1,15,'if(readparam(bStr)>=50) bonus bAspdRate,5; if(readparam(bInt)>=70) bonus bMatkRate,5;','','');
REPLACE INTO `item_db_re` VALUES (1561,'Hardback','Hardcover Book',4,20,NULL,1500,'140',NULL,1,1,0x00410100,7,2,2,4,55,1,15,'bonus bStr,3; bonus bDex,2;','','');
REPLACE INTO `item_db_re` VALUES (1572,'Principles_Of_Magic','Principles of Magic',4,20,NULL,300,'60:160',NULL,1,2,0x00410100,7,2,2,3,60,1,15,'bonus bInt,3; bonus bSPrecovRate,5;','','');
REPLACE INTO `item_db_re` VALUES (1573,'Ancient_Magic','Ancient Magic',4,20,NULL,700,'30:140',NULL,1,2,0x00410100,7,2,2,3,70,1,15,'if (isequipped(2334) || isequipped(2372)) { bonus bMdef,8; bonus bMaxSPRate,10; bonus bInt,4; };','','');
REPLACE INTO `item_db_re` VALUES (1564,'Encyclopedia','Encyclopedia',4,20,NULL,2000,'110:100',NULL,1,2,0x00410100,7,2,2,3,70,1,15,'bonus bInt,3; bonus bDex,2; bonus bCritical,20+((readparam(bLuk)*2)/10);','','');

# DEF updates
# Headgears - Work in progress.
REPLACE INTO `item_db_re` VALUES (2208,'Ribbon','Ribbon',5,800,NULL,100,NULL,1,NULL,0,0xFFFFFFFF,7,0,256,NULL,0,1,17,'bonus bMdef,3;','','');
REPLACE INTO `item_db_re` VALUES (2209,'Ribbon_','Ribbon',5,800,NULL,100,NULL,1,NULL,1,0xFFFFFFFF,7,0,256,NULL,0,1,17,'bonus bMdef,3;','','');
REPLACE INTO `item_db_re` VALUES (2216,'Biretta','Biretta',5,9000,NULL,100,NULL,8,NULL,0,0x00008110,7,2,256,NULL,0,1,11,'','','');
REPLACE INTO `item_db_re` VALUES (2217,'Biretta_','Biretta',5,9000,NULL,100,NULL,8,NULL,1,0x00008110,7,2,256,NULL,0,1,11,'','','');
REPLACE INTO `item_db_re` VALUES (2252,'Star_Sparkling','Wizard Hat',5,20,NULL,300,NULL,7,NULL,0,0x00810204,7,2,256,NULL,0,1,36,'bonus bMaxSP,100;','','');
REPLACE INTO `item_db_re` VALUES (2266,'Iron_Cane','Iron Cain',5,20,NULL,300,NULL,4,NULL,0,0x00004082,7,2,1,NULL,50,0,53,'','','');
REPLACE INTO `item_db_re` VALUES (2280,'Sahkkat','Sakkat',5,20,NULL,300,NULL,4,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,67,'bonus bAgi,1;','','');
REPLACE INTO `item_db_re` VALUES (2285,'Apple_Of_Archer','Apple of Archer',5,20,NULL,200,NULL,1,NULL,0,0xFFFFFFFE,7,2,256,NULL,30,1,72,'bonus bDex,3;','','');
REPLACE INTO `item_db_re` VALUES (2287,'Pirate_Bandana','Pirate Bandana',5,20,NULL,100,NULL,4,NULL,0,0xFFFFFFFE,7,2,256,NULL,0,1,74,'bonus bStr,1;','','');
REPLACE INTO `item_db_re` VALUES (2299,'Viking_Helm','Orc Helm',5,20,NULL,500,NULL,9,NULL,0,0x000654E2,7,2,256,NULL,0,1,86,'','','');
REPLACE INTO `item_db_re` VALUES (5007,'Loard_Circlet','Grand Circlet',5,20,NULL,200,NULL,7,NULL,0,0xFFFFFFFE,7,2,256,NULL,55,1,93,'bonus bStr,1; bonus bInt,1; bonus bLuk,1; bonus bMdef,4;','','');
REPLACE INTO `item_db_re` VALUES (5010,'Indian_Hair_Piece','Indian Fillet',5,20,NULL,100,NULL,5,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,96,'','','');
REPLACE INTO `item_db_re` VALUES (5015,'Egg_Shell','Egg Shell',5,20,NULL,200,NULL,6,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,0,101,'','','');
REPLACE INTO `item_db_re` VALUES (5017,'Bone_Helm','Bone Helm',5,20,NULL,800,NULL,15,NULL,0,0x000444A2,7,2,256,NULL,70,1,103,'bonus2 bSubEle,Ele_Dark,-15;','','');
REPLACE INTO `item_db_re` VALUES (5058,'Drooping_Kitty','Drooping Cat',5,250000,NULL,500,NULL,3,NULL,0,0xFFFFFFFE,7,2,256,NULL,0,1,142,'bonus bMdef,15; bonus2 bResEff,Eff_Curse,3000;','','');
REPLACE INTO `item_db_re` VALUES (5111,'Galapago_Cap','Galapago Cap',5,20,NULL,500,NULL,4,NULL,0,0xFFFFFFFF,7,2,256,NULL,55,1,192,'bonus2 bAddMonsterDropItem,605,100;','','');
REPLACE INTO `item_db_re` VALUES (5118,'Ear_Of_Puppy','Puppy Headband',5,20,NULL,100,NULL,3,NULL,0,0xFFFFFFFF,7,2,256,NULL,0,1,199,'','','');
REPLACE INTO `item_db_re` VALUES (5122,'Magni_Cap','Magni\'s Cap',5,30000,NULL,1000,NULL,9,NULL,0,0xFFFFFFFE,7,2,256,NULL,65,1,250,'bonus bStr,2;','','');
REPLACE INTO `item_db_re` VALUES (5128,'Goibne\'s_Helmet','Goibne\'s Helm',5,30000,NULL,500,NULL,10,NULL,0,0xFFFFFFFE,7,2,256,NULL,54,1,258,'bonus bVit,3; bonus bMdef,3; if(isequipped(2354,2419,2520)) { bonus bVit,5; bonus bMaxHPrate,15; bonus bMaxSPrate,5; bonus bDef,5; bonus bMdef,15; bonus2 bSubEle,Ele_Water,10; bonus2 bSubEle,Ele_Earth,10; bonus2 bSubEle,Ele_Fire,10; bonus2 bSubEle,Ele_Wind,10; }','','');
REPLACE INTO `item_db_re` VALUES (5157,'Viking_Helm_','Orc Helm',5,20,NULL,500,NULL,9,NULL,1,0x000654E2,7,2,256,NULL,0,1,86,'','','');
REPLACE INTO `item_db_re` VALUES (5350,'Pirate_Bandana_','Pirate Bandana',5,20,NULL,100,NULL,4,NULL,1,0xFFFFFFFE,7,2,256,NULL,0,1,74,'bonus bStr,1;','','');

# Shields
REPLACE INTO `item_db_re` VALUES (2101,'Guard','Guard',5,500,NULL,300,NULL,20,NULL,0,0xFFFFFFFF,7,2,32,NULL,0,1,1,'','','');
REPLACE INTO `item_db_re` VALUES (2102,'Guard_','Guard',5,500,NULL,300,NULL,20,NULL,1,0xFFFFFFFF,7,2,32,NULL,0,1,1,'','','');
REPLACE INTO `item_db_re` VALUES (2103,'Buckler','Buckler',5,14000,NULL,600,NULL,40,NULL,0,0x000ED5F2,7,2,32,NULL,0,1,2,'','','');
REPLACE INTO `item_db_re` VALUES (2104,'Buckler_','Buckler',5,14000,NULL,600,NULL,40,NULL,1,0x000ED5F2,7,2,32,NULL,0,1,2,'','','');
REPLACE INTO `item_db_re` VALUES (2105,'Shield','Shield',5,56000,NULL,1300,NULL,60,NULL,0,0x00004082,7,2,32,NULL,0,1,3,'','','');
REPLACE INTO `item_db_re` VALUES (2106,'Shield_','Shield',5,56000,NULL,1300,NULL,60,NULL,1,0x00004082,7,2,32,NULL,0,1,3,'','','');
REPLACE INTO `item_db_re` VALUES (2107,'Mirror_Shield','Mirror Shield',5,60000,NULL,1000,NULL,45,NULL,0,0x00404082,7,2,32,NULL,0,1,4,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2108,'Mirror_Shield_','Mirror Shield',5,60000,NULL,1000,NULL,45,NULL,1,0x00404082,7,2,32,NULL,0,1,4,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2109,'Memorize_Book','Memory Book',5,20,NULL,1000,NULL,25,NULL,0,0x00810204,7,2,32,NULL,0,1,5,'bonus bInt,1; bonus bMdef,2; if (isequipped(2717,2239)) { bonus bHPrecovRate,15; bonus bSPrecovRate,15; bonus bMatkRate,7; };','','');
REPLACE INTO `item_db_re` VALUES (2110,'Holy_Guard','Holy Guard',5,85000,NULL,1400,NULL,110,NULL,0,0x00004000,7,2,32,NULL,68,0,4,'bonus bVit,2; bonus bMdef,2;','','');
REPLACE INTO `item_db_re` VALUES (2111,'Herald_Of_GOD','Sacred Mission',5,128000,NULL,1600,NULL,120,NULL,0,0x00004000,7,2,32,NULL,83,1,4,'bonus bVit,3; bonus bInt,2; bonus bMdef,3; bonus bUnbreakableShield,0;','','');
REPLACE INTO `item_db_re` VALUES (2112,'Novice_Guard','Novice Guard',5,1,NULL,1,NULL,20,NULL,0,0x00000001,7,2,32,NULL,0,0,1,'','','');
REPLACE INTO `item_db_re` VALUES (2113,'Novice_Shield','Novice Shield',5,5000,NULL,1000,NULL,20,NULL,1,0x00000001,7,2,32,NULL,40,1,3,'bonus2 bSubEle,Ele_Water,20; bonus2 bSubEle,Ele_Earth,20; bonus2 bSubEle,Ele_Fire,20; bonus2 bSubEle,Ele_Wind,20; bonus2 bSubEle,Ele_Poison,20; bonus2 bSubEle,Ele_Ghost,20; bonus2 bSubEle,Ele_Undead,20;','','');
REPLACE INTO `item_db_re` VALUES (2114,'Stone_Buckler','Stone Buckler',5,30000,NULL,1500,NULL,45,NULL,1,0xFFFFFFFE,7,2,32,NULL,65,1,2,'bonus2 bSubSize,2,5; if (isequipped(2353,5122)) { bonus bStr,2; bonus bDef,5; bonus bMdef,5; if(BaseClass == Job_Swordman) bonus bDef,6; }','','');
REPLACE INTO `item_db_re` VALUES (2115,'Valkyrja\'s_Shield','Valkyrja\'s Shield',5,30000,NULL,500,NULL,80,NULL,1,0xFFFFFFFE,7,2,32,NULL,65,1,4,'bonus2 bSubEle,Ele_Water,20; bonus2 bSubEle,Ele_Fire,20; bonus2 bSubEle,Ele_Dark,20; bonus2 bSubEle,Ele_Undead,20; bonus bMdef,5; if(isequipped(2353,5124)) { bonus bDef,2-getrefine()-getequiprefinerycnt(EQI_HEAD_TOP); bonus bMdef,5+getrefine()+getequiprefinerycnt(EQI_HEAD_TOP); }','','');
REPLACE INTO `item_db_re` VALUES (2116,'Angel\'s_Safeguard','Angelic Guard',5,10000,NULL,400,NULL,30,NULL,1,0x00000001,7,2,32,NULL,20,1,1,'bonus2 bSubRace,RC_Demon,5;','','');
REPLACE INTO `item_db_re` VALUES (2117,'Arm_Guard','Arm Guard',5,10000,NULL,150,NULL,50,NULL,0,0x02000000,7,2,32,NULL,20,1,1,'','','');
REPLACE INTO `item_db_re` VALUES (2118,'Arm_Guard_','Arm Guard',5,10000,NULL,150,NULL,50,NULL,1,0x02000000,7,2,32,NULL,20,1,1,'','','');
REPLACE INTO `item_db_re` VALUES (2119,'Improved_Arm_Guard','Advanced Arm Guard',5,40000,NULL,150,NULL,45,NULL,0,0x02000000,7,2,32,NULL,50,1,1,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2120,'Improved_Arm_Guard_','Advanced Arm Guard',5,40000,NULL,150,NULL,45,NULL,1,0x02000000,7,2,32,NULL,50,1,1,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2121,'Memorize_Book_','Memory Book',5,20,NULL,1000,NULL,25,NULL,1,0x00810204,7,2,32,NULL,0,1,5,'bonus bInt,1; bonus bMdef,2; if (isequipped(2717,2239)) { bonus bHPrecovRate,15; bonus bSPrecovRate,15; bonus bMatkRate,7; };','','');
REPLACE INTO `item_db_re` VALUES (2122,'Platinum_Shield','Platinum Shield',5,20,NULL,1200,NULL,95,NULL,0,0xFFFFFFFE,2,2,32,NULL,68,1,4,'bonus bMdef,5; bonus2 bSubSize,1,15; bonus2 bSubSize,2,15; bonus2 bSubRace,RC_Undead,10; bonus5 bAutoSpellWhenHit,NPC_MAGICMIRROR,2,150,BF_MAGIC,0;','','');
REPLACE INTO `item_db_re` VALUES (2123,'Orleans_Server','Orleans\'s Server',5,20,NULL,1000,NULL,75,NULL,1,0xFFFFFFFE,2,2,32,NULL,55,1,4,'bonus bMdef,2; bonus bMagicDamageReturn,5; if (isequipped(2701)) bonus bCastrate,-10;','','');
REPLACE INTO `item_db_re` VALUES (2124,'Thorny_Buckler','Thorny Buckler',5,20,NULL,1000,NULL,85,NULL,1,0xFFFFFFFE,2,2,32,NULL,55,1,2,'bonus bMdef,2; if (isequipped(2702)) { bonus bAspdRate,10; bonus bShortWeaponDamageReturn,5; }','','');
REPLACE INTO `item_db_re` VALUES (2125,'Strong_Shield','Strong Shield',5,20,NULL,2500,NULL,90,NULL,1,0xFFFFFFFE,2,2,32,NULL,75,1,4,'bonus bNoKnockback,0; bonus2 bSubEle,Ele_Neutral,-20; bonus2 bSubEle,Ele_Fire,-20; bonus2 bSubEle,Ele_Water,-20; bonus2 bSubEle,Ele_Wind,-20; bonus2 bSubEle,Ele_Earth,-20; bonus2 bSubEle,Ele_Dark,-20; bonus2 bSubEle,Ele_Holy,-20; bonus2 bSubEle,Ele_Ghost,-20;','','');
REPLACE INTO `item_db_re` VALUES (2128,'Herald_Of_GOD_','Sacred Mission',5,128000,NULL,1600,NULL,120,NULL,1,0x00004000,7,2,32,NULL,83,1,4,'bonus bVit,3; bonus bInt,2; bonus bMdef,3; bonus bUnbreakableShield,0;','','');
REPLACE INTO `item_db_re` VALUES (2129,'Exorcism_Bible','Exorcism Bible',5,20,NULL,600,NULL,80,NULL,0,0x00008100,7,2,32,NULL,50,1,5,'bonus bHPrecovRate,3; bonus bSPrecovRate,3; bonus bInt,1; if(isequipped(1631)) { bonus2 bSkillAtk,"PR_MAGNUS",20; bonus3 bAutoSpellWhenHit,"PR_TURNUNDEAD",1,20; }','','');
REPLACE INTO `item_db_re` VALUES (2130,'Cross_Shield','Cross Shield',5,20,NULL,2000,NULL,130,NULL,1,0x00004000,7,2,32,NULL,80,1,4,'bonus bStr,1; bonus2 bSkillAtk,PA_SHIELDCHAIN,30; bonus2 bSkillAtk,CR_SHIELDBOOMERANG,30; bonus bUseSPrate,10;','','');
REPLACE INTO `item_db_re` VALUES (2131,'Magic_Study_Vol1','Magic Bible Vol1',5,20,NULL,1000,NULL,18,NULL,1,0x00810204,2,2,32,NULL,70,1,5,'bonus bMdef,3; bonus bInt,2; bonus2 bAddEffWhenHit,Eff_Stun,1000;','','');
REPLACE INTO `item_db_re` VALUES (2133,'Tournament_Shield','Tournament Shield',5,20,NULL,1000,NULL,105,NULL,1,0x00004082,2,2,32,NULL,50,1,4,'bonus2 bAddRace,RC_NonBoss,1; bonus2 bAddRace,RC_Boss,1; if( Class == Job_Lord_Knight ) bonus bAspdRate,-5; if( isequipped(1420) || isequipped(1421) || isequipped(1422) ) { bonus2 bAddRace,RC_NonBoss,4; bonus2 bAddRace,RC_Boss,4; bonus bDef,2; }','','');
REPLACE INTO `item_db_re` VALUES (2134,'Shield_Of_Naga','Shield of Naga',5,20,NULL,500,NULL,35,NULL,1,0x00CFFF80,2,2,32,NULL,70,1,2,'bonus bMdef,3; autobonus2 "{ bonus bShortWeaponDamageReturn,(getrefine()*3); }",10,5000,BF_WEAPON,"{ specialeffect2 EF_GUARD; }";','','');
REPLACE INTO `item_db_re` VALUES (2135,'Shadow_Guard','Shadow Guard',5,20,NULL,800,NULL,52,NULL,1,0x00020000,2,2,32,NULL,70,1,2,'if( isequipped(2426) ) { bonus2 bAddEff,Eff_Blind,500; autobonus "{ bonus bFlee,20; }",200,10000,BF_WEAPON,"{ specialeffect2 EF_INCAGILITY; }"; }','','');
REPLACE INTO `item_db_re` VALUES (2138,'Bradium_Shield','Bradium Shield',5,20,NULL,1800,NULL,98,NULL,1,0x00CFFF80,2,2,32,NULL,65,1,3,'bonus2 bSkillAtk,CR_SHIELDBOOMERANG,60; bonus bAgi,-1; bonus bMaxHP,500;','','');
REPLACE INTO `item_db_re` VALUES (2139,'Flame_Thrower','Flame Thrower',5,20000,NULL,2000,NULL,60,NULL,0,0xFFFFFFFF,7,2,32,NULL,99,0,1,'','','');

# Footgear
REPLACE INTO `item_db_re` VALUES (2401,'Sandals','Sandals',5,400,NULL,200,NULL,5,NULL,0,0xFFFFFFFF,7,2,64,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2402,'Sandals_','Sandals',5,400,NULL,200,NULL,5,NULL,1,0xFFFFFFFF,7,2,64,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2403,'Shoes','Shoes',5,3500,NULL,400,NULL,10,NULL,0,0xFFFFFFFE,7,2,64,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2404,'Shoes_','Shoes',5,3500,NULL,400,NULL,10,NULL,1,0xFFFFFFFE,7,2,64,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2405,'Boots','Boots',5,18000,NULL,600,NULL,16,NULL,0,0x016E5CEA,7,2,64,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2406,'Boots_','Boots',5,18000,NULL,600,NULL,16,NULL,1,0x016E5CEA,7,2,64,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2407,'Chrystal_Pumps','Crystal Pumps',5,20,NULL,100,NULL,5,NULL,0,0xFFFFFFFE,7,0,64,NULL,0,1,0,'bonus bMdef,10; bonus bLuk,5;','','');
REPLACE INTO `item_db_re` VALUES (2408,'Cuffs','Shackles',5,5000,NULL,3000,NULL,15,NULL,0,0xFFFFFFFF,7,2,64,NULL,0,1,0,'if (isequipped(2655)) { bonus bBaseAtk,50; bonus2 bAddDefClass,1196,20; bonus2 bAddDefClass,1197,20; }','','');
REPLACE INTO `item_db_re` VALUES (2409,'Spiky_Heel','High Heels',5,8500,NULL,600,NULL,4,NULL,0,0xFFFFFFFE,7,2,64,NULL,0,1,0,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2410,'Sleipnir','Sleipnir',5,20,NULL,3500,NULL,40,NULL,0,0xFFFFFFFF,7,2,64,NULL,94,0,0,'bonus bMdef,10; bonus bMaxHPrate,20; bonus bMaxSPrate,20; bonus bSPrecovRate,15; bonus bSpeedRate,25;','','');
REPLACE INTO `item_db_re` VALUES (2411,'Grave','Greaves',5,48000,NULL,750,NULL,27,NULL,0,0x00004080,7,2,64,NULL,65,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2412,'Grave_','Greaves',5,54000,NULL,750,NULL,15,NULL,1,0x00004080,7,2,64,NULL,65,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2414,'Novice_Boots','Novice Slippers',5,1,NULL,1,NULL,5,NULL,0,0x00000001,7,2,64,NULL,0,0,0,'','','');
REPLACE INTO `item_db_re` VALUES (2416,'Novice_Shoes','Novice Shoes',5,35000,NULL,500,NULL,8,NULL,1,0x00000001,7,2,64,NULL,40,1,0,'bonus bMaxHPrate,5;','','');
REPLACE INTO `item_db_re` VALUES (2417,'Fricco_Shoes','Fricco\'s Shoes',5,30000,NULL,500,NULL,12,NULL,0,0xFFFFFFFE,7,2,64,NULL,65,1,0,'bonus bAgi,2; bonus2 bAddItemHealRate,IG_Potion,20; if(isequipped(2353,2516)){ bonus bAgi,3; bonus bMaxHPrate,5; bonus bMaxSPrate,5; }','','');
REPLACE INTO `item_db_re` VALUES (2418,'Vidar\'s_Boots','Vidar\'s Boots',5,30000,NULL,650,NULL,13,NULL,0,0xFFFFFFFE,7,2,64,NULL,65,1,0,'bonus bMaxHPrate,9; bonus bMaxSPrate,9; if(isequipped(2353,2517)){ bonus bVit,5; bonus bHPrecovRate,10; bonus bSPrecovRate,10; }','','');
REPLACE INTO `item_db_re` VALUES (2419,'Goibne\'s_Combat_Boots','Goibne\'s Greaves',5,30000,NULL,700,NULL,13,NULL,0,0xFFFFFFFE,7,2,64,NULL,54,1,0,'bonus bMdef,3; bonus bMaxHPrate,5; bonus bMaxSPrate,5;','','');
REPLACE INTO `item_db_re` VALUES (2420,'Angel\'s_Arrival','Angel\'s Reincarnation',5,10000,NULL,300,NULL,8,NULL,1,0x00000001,7,2,64,NULL,25,1,0,'bonus bMaxHP,100;','','');
REPLACE INTO `item_db_re` VALUES (2421,'Valkyrie_Shoes','Valkyrian Shoes',5,0,NULL,500,NULL,13,NULL,1,0xFFFFFFFE,2,2,64,NULL,1,1,0,'bonus bUnbreakableShoes,0; if(BaseClass==Job_Mage||BaseClass==Job_Archer||BaseClass==Job_Acolyte) bonus bMaxHP,(BaseLevel*5); else if(BaseClass==Job_Swordman||BaseClass==Job_Merchant||BaseClass==Job_Thief) bonus bMaxSP,(JobLevel*2);','','');
REPLACE INTO `item_db_re` VALUES (2422,'High_Fashion_Sandals','High Fashion Sandals',5,24000,NULL,200,NULL,7,NULL,1,0x00818314,7,2,64,NULL,40,1,0,'bonus bMdef,10;','','');
REPLACE INTO `item_db_re` VALUES (2423,'Variant_Shoes','Variant Shoes',5,20,NULL,500,NULL,13,NULL,0,0xFFFFFFFE,2,2,64,NULL,85,1,0,'bonus bMaxHPRate,20-getrefine(); bonus bMaxSPRate,20-getrefine(); bonus bDef,getrefine()/2;','','');
REPLACE INTO `item_db_re` VALUES (2424,'Tidal_Shoes','Tidal Shoes',5,20,NULL,300,NULL,13,NULL,1,0xFFFFFFFE,2,2,64,NULL,55,1,0,'bonus2 bSubEle,Ele_Water,5; if (isequipped(2528)) { bonus bHPrecovRate,5; bonus bMaxHPrate,10; }','','');
REPLACE INTO `item_db_re` VALUES (2425,'Black_Leather_Boots','Black Leather Boots',5,20,NULL,500,NULL,16,NULL,0,0xFFFFFFFE,2,2,64,NULL,55,1,0,'bonus bAgi,1; if(getrefine() >= 9) bonus bAgi,2;','','');
REPLACE INTO `item_db_re` VALUES (2426,'Shadow_Walk','Shadow Walk',5,20,NULL,2000,NULL,0,NULL,0,0xFFFFFFFE,2,2,64,NULL,75,1,0,'bonus bMdef,10; if(getskilllv("AS_CLOAKING") < 3){ bonus5 bAutoSpellWhenHit,"AS_CLOAKING",3,100,BF_MAGIC,0; } else bonus5 bAutoSpellWhenHit,"AS_CLOAKING",getskilllv("AS_CLOAKING"),100,BF_MAGIC,0;','','');
REPLACE INTO `item_db_re` VALUES (2432,'Spiky_Heel_','High Heels',5,8500,NULL,600,NULL,10,NULL,1,0xFFFFFFFE,7,2,64,NULL,0,1,0,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2433,'Diabolus_Boots','Diabolus Boots',5,20,NULL,250,NULL,15,NULL,1,0x00CFFF80,2,2,64,NULL,0,1,0,'bonus bMaxHP,(BaseLevel*10);','','');
REPLACE INTO `item_db_re` VALUES (2434,'Black_Leather_Boots_','Black Leather Boots',5,20,NULL,500,NULL,16,NULL,1,0xFFFFFFFE,2,2,64,NULL,55,1,0,'bonus bAgi,1; if(getrefine() >= 9) bonus bAgi,2;','','');
REPLACE INTO `item_db_re` VALUES (2435,'Battle_Greave','Battle Greaves',5,10,NULL,0,NULL,15,NULL,1,0x026654E2,7,2,64,NULL,80,1,0,'bonus bMaxHP,100; bonus bMdef,1; bonus2 bSubRace,RC_DemiHuman,1;','','');
REPLACE INTO `item_db_re` VALUES (2436,'Combat_Boots','Combat Boots',5,10,NULL,0,NULL,9,NULL,1,0x00898B1C,7,2,64,NULL,80,1,0,'bonus bMaxHP,100; bonus bMdef,1; bonus2 bSubRace,RC_DemiHuman,1;','','');
REPLACE INTO `item_db_re` VALUES (2437,'Battle_Boots','Battle Boots',5,10,NULL,0,NULL,9,NULL,1,0x01000000,7,2,64,NULL,80,1,0,'bonus bMaxHP,100; bonus bMdef,1; bonus2 bSubRace,RC_DemiHuman,1;','','');
REPLACE INTO `item_db_re` VALUES (2440,'Sprint_Shoes','Sprint Shoes',5,20,NULL,300,NULL,10,NULL,1,0x00CFFF80,2,2,64,NULL,70,1,0,'bonus bAgi,1; bonus bSPrecovRate,5;','','');
REPLACE INTO `item_db_re` VALUES (2444,'Krieger_Shoes1','Glorious Shoes',5,20,NULL,0,NULL,0,NULL,0,0xFFFFFFFE,7,2,64,NULL,81,1,0,'bonus bMaxHPRate,10; bonus2 bSubRace,RC_DemiHuman,4; bonus3 bAutoSpellWhenHit,"AL_INCAGI",1,10;','','');
REPLACE INTO `item_db_re` VALUES (2445,'Krieger_Shoes2','Glorious Popularized Shoes',5,20,NULL,0,NULL,0,NULL,0,0xFFFFFFFE,7,2,64,NULL,61,1,0,'bonus bMaxHPRate,5; bonus bMaxSPRate,5;','','');
REPLACE INTO `item_db_re` VALUES (2446,'Krieger_Shoes3','Glorious Mass-Production Shoes',5,20,NULL,0,NULL,10,NULL,0,0xFFFFFFFE,7,2,64,NULL,0,1,0,'bonus bMaxHPRate,5;','','');
REPLACE INTO `item_db_re` VALUES (2447,'Military_Boots','Army Boots',5,0,NULL,1000,NULL,5,NULL,0,0xFFFFFFFE,7,2,64,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2450,'Vital_Tree_Shoes','Vital Tree Shoes',5,20,NULL,500,NULL,16,NULL,0,0xFFFFFFFE,2,2,64,NULL,60,1,0,'bonus bMaxHPrate,10; bonus2 bHPRegenRate,30,10000; bonus bHealpower2,5; bonus bAddItemHealRate,5; bonus bMdef,3; bonus bVit,2;','','');
REPLACE INTO `item_db_re` VALUES (2456,'Para_Team_Boots1','Eden Team Boots I',5,0,NULL,0,NULL,14,NULL,0,0xFFFFFFFF,7,2,64,NULL,12,0,0,'bonus bHPrecovRate,10; bonus bSPrecovRate,2;','','');
REPLACE INTO `item_db_re` VALUES (2457,'Para_Team_Boots2','Eden Team Boots II',5,0,NULL,0,NULL,16,NULL,0,0xFFFFFFFF,7,2,64,NULL,26,0,0,'bonus bHPrecovRate,12; bonus bSPrecovRate,4;','','');
REPLACE INTO `item_db_re` VALUES (2458,'Para_Team_Boots3','Eden Team Boots III',5,0,NULL,0,NULL,18,NULL,0,0xFFFFFFFF,7,2,64,NULL,40,0,0,'bonus bHPrecovRate,14; bonus bSPrecovRate,6;','','');
REPLACE INTO `item_db_re` VALUES (2463,'Feral_Boots','Feral Boots',5,20,NULL,0,NULL,12,NULL,0,0xFFFFFFFF,7,2,64,NULL,75,0,0,'bonus bMdef,2;','','');

# Armor
REPLACE INTO `item_db_re` VALUES (2301,'Cotton_Shirt','Cotton Shirt',5,10,NULL,100,NULL,10,NULL,0,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2302,'Cotton_Shirt_','Cotton Shirt',5,10,NULL,100,NULL,10,NULL,1,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2303,'Leather_Jacket','Jacket',5,200,NULL,200,NULL,15,NULL,0,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2304,'Leather_Jacket_','Jacket',5,200,NULL,200,NULL,15,NULL,1,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2305,'Adventure_Suit','Adventurer\'s Suit',5,1000,NULL,300,NULL,20,NULL,0,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2306,'Adventurere\'s_Suit_','Adventurer\'s Suit',5,1000,NULL,300,NULL,20,NULL,1,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2307,'Mantle','Mantle',5,10000,NULL,600,NULL,37,NULL,0,0xFFFFFFFE,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2308,'Mantle_','Mantle',5,10000,NULL,600,NULL,37,NULL,1,0xFFFFFFFE,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2309,'Coat','Coat',5,22000,NULL,1200,NULL,42,NULL,0,0xFFFFFFFE,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2310,'Coat_','Coat',5,22000,NULL,1200,NULL,42,NULL,1,0xFFFFFFFE,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2311,'Mink_Coat','Mink Coat',5,20,NULL,2300,NULL,30,NULL,1,0xFFFFFFFE,7,2,16,NULL,30,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2312,'Padded_Armor','Padded Armor',5,48000,NULL,2800,NULL,35,NULL,0,0x000654E2,7,2,16,NULL,0,1,0,'if(isequipped(2656)) { bonus bDef,5; bonus bMaxHP,150; }','','');
REPLACE INTO `item_db_re` VALUES (2313,'Padded_Armor_','Padded Armor',5,48000,NULL,2800,NULL,35,NULL,1,0x000654E2,7,2,16,NULL,0,1,0,'if(isequipped(2656)) { bonus bDef,5; bonus bMaxHP,150; }','','');
REPLACE INTO `item_db_re` VALUES (2314,'Chain_Mail','Chain Mail',5,65000,NULL,3300,NULL,55,NULL,0,0x000654E2,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2315,'Chain_Mail_','Chain Mail',5,65000,NULL,3300,NULL,55,NULL,1,0x000654E2,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2316,'Plate_Armor','Full Plate',5,80000,NULL,4500,NULL,70,NULL,0,0x00004082,7,2,16,NULL,40,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2317,'Plate_Armor_','Full Plate',5,80000,NULL,4500,NULL,70,NULL,1,0x00004082,7,2,16,NULL,40,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2318,'Clothes_Of_The_Lord','Lord\'s Clothes',5,20,NULL,2500,NULL,59,NULL,1,0x00040420,7,2,16,NULL,70,1,0,'bonus bMdef,5; bonus bInt,1;','','');
REPLACE INTO `item_db_re` VALUES (2319,'Glittering_Clothes','Glittering Jacket',5,20,NULL,2500,NULL,58,NULL,1,0xFFFFFFFE,7,2,16,NULL,60,1,0,'bonus bMdef,5; bonus2 bAddEff,Eff_Blind,300;','','');
REPLACE INTO `item_db_re` VALUES (2320,'Formal_Suit','Formal Suit',5,20,NULL,300,NULL,40,NULL,1,0xFFFFFFFE,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2321,'Silk_Robe','Silk Robe',5,8000,NULL,400,NULL,20,NULL,0,0x0085C7B6,7,2,16,NULL,0,1,0,'bonus bMdef,10;','','');
REPLACE INTO `item_db_re` VALUES (2322,'Silk_Robe_','Silk Robe',5,8000,NULL,400,NULL,20,NULL,1,0x0085C7B6,7,2,16,NULL,0,1,0,'bonus bMdef,10;','','');
REPLACE INTO `item_db_re` VALUES (2323,'Scapulare','Scapulare',5,6500,NULL,400,NULL,24,NULL,0,0x00008110,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2324,'Scapulare_','Scapulare',5,6500,NULL,400,NULL,24,NULL,1,0x00008110,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2325,'Saint_Robe','Saint\'s Robe',5,54000,NULL,600,NULL,50,NULL,0,0x00048530,7,2,16,NULL,0,1,0,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2326,'Saint_Robe_','Saint\'s Robe',5,54000,NULL,600,NULL,50,NULL,1,0x00048530,7,2,16,NULL,0,1,0,'bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2327,'Holy_Robe','Holy Robe',5,20,NULL,1700,NULL,57,NULL,0,0x00008110,7,2,16,NULL,60,1,0,'bonus bMdef,5; bonus2 bSubRace,RC_Demon,15; bonus2 bSubEle,Ele_Dark,10;','','');
REPLACE INTO `item_db_re` VALUES (2328,'Wooden_Mail','Wooden Mail',5,5500,NULL,1000,NULL,25,NULL,0,0x000444A2,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2329,'Wooden_Mail_','Wooden Mail',5,5500,NULL,1000,NULL,25,NULL,1,0x000444A2,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2330,'Tights','Tights',5,71000,NULL,500,NULL,27,NULL,0,0x00080808,7,2,16,NULL,45,1,0,'bonus bDex,1;','','');
REPLACE INTO `item_db_re` VALUES (2331,'Tights_','Tights',5,71000,NULL,500,NULL,27,NULL,1,0x00080808,7,2,16,NULL,45,1,0,'bonus bDex,1;','','');
REPLACE INTO `item_db_re` VALUES (2332,'Silver_Robe','Silver Robe',5,7000,NULL,700,NULL,23,NULL,0,0x00810204,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2333,'Silver_Robe_','Silver Robe',5,7000,NULL,700,NULL,23,NULL,1,0x00810204,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2334,'Mage_Coat','Mage Coat',5,20,NULL,600,NULL,40,NULL,0,0x00810204,7,2,16,NULL,50,1,0,'bonus bMdef,5; bonus bInt,1;','','');
REPLACE INTO `item_db_re` VALUES (2335,'Thief_Clothes','Thief Clothes',5,74000,NULL,100,NULL,40,NULL,0,0x02021040,7,2,16,NULL,0,1,0,'bonus bAgi,1;','','');
REPLACE INTO `item_db_re` VALUES (2336,'Thief_Clothes_','Thief Clothes',5,74000,NULL,100,NULL,40,NULL,1,0x02021040,7,2,16,NULL,0,1,0,'bonus bAgi,1;','','');
REPLACE INTO `item_db_re` VALUES (2337,'Ninja_Suit','Ninja Suit',5,20,NULL,1500,NULL,58,NULL,0,0x02021040,7,2,16,NULL,50,1,0,'bonus bAgi,1; bonus bMdef,3; if(isequipped(2654)) { bonus bUseSPrate,-20; bonus bMaxHP,300; }','','');
REPLACE INTO `item_db_re` VALUES (2338,'Wedding_Dress','Wedding Dress',5,43000,NULL,500,NULL,10,NULL,0,0xFFFFFFFE,7,2,16,NULL,0,1,0,'bonus bMdef,15;','','');
REPLACE INTO `item_db_re` VALUES (2339,'G_Strings','Pantie',5,1000,NULL,100,NULL,22,NULL,0,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2341,'Full_Plate_Armor','Legion Plate Armor',5,94000,NULL,5500,NULL,79,NULL,0,0x00004000,7,2,16,NULL,70,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2342,'Full_Plate_Armor_','Legion Plate Armor',5,102500,NULL,5500,NULL,79,NULL,1,0x00004000,7,2,16,NULL,70,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2343,'Robe_Of_Casting','Robe of Cast',5,124800,NULL,1100,NULL,40,NULL,0,0x00810200,7,2,16,NULL,75,1,0,'bonus bCastrate,-3; bonus bMdef,4;','','');
REPLACE INTO `item_db_re` VALUES (2344,'Flame_Sprits_Armor','Lucius\'s Fierce Armor of Volcano',5,136000,NULL,2200,NULL,25,NULL,0,0x000444A2,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Fire;','','');
REPLACE INTO `item_db_re` VALUES (2345,'Flame_Sprits_Armor_','Lucius\'s Fierce Armor of Volcano',5,136000,NULL,2200,NULL,25,NULL,1,0xFFFFFFFE,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Fire;','','');
REPLACE INTO `item_db_re` VALUES (2346,'Water_Sprits_Armor','Saphien\'s Armor of Ocean',5,136000,NULL,2200,NULL,25,NULL,0,0x000444A2,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Water;','','');
REPLACE INTO `item_db_re` VALUES (2347,'Water_Sprits_Armor_','Saphien\'s Armor of Ocean',5,136000,NULL,2200,NULL,25,NULL,1,0xFFFFFFFE,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Water;','','');
REPLACE INTO `item_db_re` VALUES (2348,'Wind_Sprits_Armor','Aebecee\'s Raging Typhoon Armor',5,136000,NULL,2200,NULL,25,NULL,0,0x000444A2,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Wind;','','');
REPLACE INTO `item_db_re` VALUES (2349,'Wind_Sprits_Armor_','Aebecee\'s Raging Typhoon Armor',5,136000,NULL,2200,NULL,25,NULL,1,0xFFFFFFFE,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Wind;','','');
REPLACE INTO `item_db_re` VALUES (2350,'Earth_Sprits_Armor','Claytos Cracking Earth Armor',5,136000,NULL,2200,NULL,25,NULL,0,0x000444A2,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Earth;','','');
REPLACE INTO `item_db_re` VALUES (2351,'Earth_Sprits_Armor_','Claytos Cracking Earth Armor',5,136000,NULL,2200,NULL,25,NULL,1,0xFFFFFFFE,7,2,16,NULL,45,1,0,'bonus bDefEle,Ele_Earth;','','');
REPLACE INTO `item_db_re` VALUES (2352,'Novice_Plate','Tattered Novice Ninja Suit',5,1,NULL,1,NULL,25,NULL,0,0x00000001,7,2,16,NULL,0,0,0,'','','');
REPLACE INTO `item_db_re` VALUES (2353,'Odin\'s_Blessing','Odin\'s Blessing',5,30000,NULL,2500,NULL,53,NULL,1,0xFFFFFFFE,7,2,16,NULL,65,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2354,'Goibne\'s_Armor','Goibne\'s Armor',5,50000,NULL,3500,NULL,58,NULL,0,0xFFFFFFFE,7,2,16,NULL,54,1,0,'bonus bVit,2; bonus bMaxHPrate,10;','','');
REPLACE INTO `item_db_re` VALUES (2355,'Angel\'s_Protection','Angelic Protection',5,10000,NULL,600,NULL,25,NULL,1,0x00000001,7,2,16,NULL,40,1,0,'bonus bMdef,20; if(isequipped(2116,2420,2521,5125)) { bonus bMaxHP,900; bonus bMaxSP,100; bonus3 bAutoSpellWhenHit,HP_ASSUMPTIO,1,30; }','','');
REPLACE INTO `item_db_re` VALUES (2356,'Vestment_Of_Grace','Blessed Holy Robe',5,20,NULL,2500,NULL,45,NULL,1,0x00008100,7,2,16,NULL,70,1,0,'bonus bMdef,5; bonus2 bResEff,Eff_Blind,8000;','','');
REPLACE INTO `item_db_re` VALUES (2357,'Valkyrie_Armor','Valkyrian Armor',5,0,NULL,2800,NULL,55,NULL,1,0xFFFFFFFE,2,2,16,NULL,1,1,0,'bonus bAllStats,1; bonus bUnbreakableArmor,0; if(BaseClass==Job_Mage||BaseClass==Job_Archer||BaseClass==Job_Acolyte) bonus2 bResEff,Eff_Silence,5000; else if(BaseClass==Job_Swordman||BaseClass==Job_Merchant||BaseClass==Job_Thief) bonus2 bResEff,Eff_Stun,5000;','','');
REPLACE INTO `item_db_re` VALUES (2359,'Ninja_Suit_','Ninja Suit',5,20,NULL,1500,NULL,58,NULL,1,0x02021040,7,2,16,NULL,50,1,0,'bonus bAgi,1; bonus bMdef,3; if(isequipped(2654)) { bonus bUseSPrate,-20; bonus bMaxHP,300; }','','');
REPLACE INTO `item_db_re` VALUES (2360,'Robe_Of_Casting_','Robe of Cast',5,124800,NULL,1100,NULL,40,NULL,1,0x00810200,7,2,16,NULL,75,1,0,'bonus bCastrate,-3; bonus bMdef,4;','','');
REPLACE INTO `item_db_re` VALUES (2364,'Meteo_Plate_Armor','Meteo Plate Armor',5,20,NULL,3000,NULL,85,NULL,1,0x000444A2,2,2,16,NULL,55,1,0,'bonus2 bResEff,Eff_Stun,3000; bonus2 bResEff,Eff_Freeze,3000;','','');
REPLACE INTO `item_db_re` VALUES (2365,'Orleans_Gown','Orleans\'s Gown',5,20,NULL,300,NULL,15,NULL,1,0xFFFFFFFE,2,2,16,NULL,55,1,0,'bonus bCastrate,15; bonus bNoCastCancel,0;','','');
REPLACE INTO `item_db_re` VALUES (2366,'Divine_Cloth','Divine Cloth',5,20,NULL,1500,NULL,50,NULL,1,0xFFFFFFFE,2,2,16,NULL,55,1,0,'bonus2 bResEff,Eff_Curse,500; bonus2 bResEff,Eff_Silence,500; bonus2 bResEff,Eff_Stun,500; bonus2 bResEff,Eff_Stone,500; bonus2 bResEff,Eff_Sleep,500;','','');
REPLACE INTO `item_db_re` VALUES (2367,'Sniping_Suit','Sniping Suit',5,20,NULL,750,NULL,42,NULL,1,0x00000800,2,2,16,NULL,50,1,0,'bonus bMdef,5; bonus bCritical,6+(readparam(bLuk)/10); bonus bDelayRate,-23;','','');
REPLACE INTO `item_db_re` VALUES (2371,'G_Strings_','Pantie',5,1000,NULL,100,NULL,22,NULL,1,0xFFFFFFFF,7,2,16,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2372,'Mage_Coat_','Mage Coat',5,20,NULL,600,NULL,40,NULL,1,0x00810204,7,2,16,NULL,50,1,0,'bonus bMdef,5; bonus bInt,1;','','');
REPLACE INTO `item_db_re` VALUES (2373,'Holy_Robe_','Holy Robe',5,20,NULL,1700,NULL,57,NULL,1,0x00008110,7,2,16,NULL,60,1,0,'bonus bMdef,5; bonus2 bSubRace,RC_Demon,15; bonus2 bSubEle,Ele_Dark,10;','','');
REPLACE INTO `item_db_re` VALUES (2374,'Diabolus_Robe','Diabolus Robe',5,20,NULL,300,NULL,57,NULL,1,0x00098B1C,2,2,16,NULL,55,1,0,'bonus bMaxSP,150; bonus bMdef,5; bonus bHealPower,6; bonus bDelayRate,-10; if (isequipped(2729)) { bonus2 bAddRace,RC_NonBoss,3; bonus2 bAddRace,RC_Boss,3; bonus bMatkRate,3; }','','');
REPLACE INTO `item_db_re` VALUES (2375,'Diabolus_Armor','Diabolus Armor',5,20,NULL,600,NULL,79,NULL,1,0x000654E2,2,2,16,NULL,55,1,0,'bonus bStr,2; bonus bDex,1; bonus bMaxHP,150; bonus2 bResEff,Eff_Stun,500; bonus2 bResEff,Eff_Stone,500; if (isequipped(2729)) { bonus2 bAddRace,RC_NonBoss,3; bonus2 bAddRace,RC_Boss,3; bonus bMatkRate,3; }','','');
REPLACE INTO `item_db_re` VALUES (2376,'Assaulter_Plate','Assaulter Plate',5,10,NULL,0,NULL,57,NULL,1,0x006444A2,7,2,16,NULL,80,1,0,'bonus bMaxHP,150; bonus bMdef,2; bonus2 bSubRace,RC_DemiHuman,2; if (isequipped(2538,2435)) { bonus2 bSubRace,RC_NonDemiHuman,-300; bonus bVit,3; bonus bMaxHPRate,12; bonus bHealpower2,10; bonus bAddItemHealRate,10; autobonus2 "{ bonus2 bHPRegenRate,600,1000; }",5,10000,BF_WEAPON,"{ specialeffect2 EF_HEAL; }"; };','','');
REPLACE INTO `item_db_re` VALUES (2377,'Elite_Engineer_Armor','Elite Engineer Armor',5,10,NULL,0,NULL,50,NULL,1,0x00040420,7,2,16,NULL,80,1,0,'bonus bMaxHP,150; bonus bMdef,2; bonus2 bSubRace,RC_DemiHuman,2; if (isequipped(2538,2435)) { bonus2 bSubRace,RC_NonDemiHuman,-300; bonus bStr,3; bonus bMaxHPRate,12; bonus2 bSkillAtk,"MC_MAMMONITE",20; bonus2 bSkillHeal,"AM_POTIONPITCHER",10; bonus2 bSkillHeal2,"AM_POTIONPITCHER",10; bonus2 bSkillHeal2,"AL_HEAL",10; bonus bUnbreakableArmor,0; };','','');
REPLACE INTO `item_db_re` VALUES (2378,'Assassin_Robe','Assassin Robe',5,10,NULL,0,NULL,41,NULL,1,0x02021040,7,2,16,NULL,80,1,0,'bonus bMaxHP,150; bonus bMdef,2; bonus2 bSubRace,RC_DemiHuman,2; if (isequipped(2538,2435)) { bonus2 bSubRace,RC_NonDemiHuman,-300; bonus bAgi,3; bonus bMaxHPRate,12; bonus bCritical,5; bonus bAspdRate,5; autobonus "{ bonus2 bHPRegenRate,300,1000; }",10,10000,BF_WEAPON,"{ specialeffect2 EF_HEAL; }"; };','','');
REPLACE INTO `item_db_re` VALUES (2379,'Warlock_Battle_Robe','Warlock\'s Battle Robe',5,10,NULL,0,NULL,36,NULL,1,0x00810204,7,2,16,NULL,80,1,0,'bonus bMaxHP,150; bonus bMdef,2; bonus2 bSubRace,RC_DemiHuman,2; if (isequipped(2539,2436)) { bonus2 bSubRace,RC_NonDemiHuman,-300; bonus bInt,3; bonus bMaxHPRate,12; bonus2 bResEff,Eff_Stun,2000; autobonus2 "{ bonus bDefEle,Ele_Ghost; }",30,10000,BF_WEAPON,"{ specialeffect2 EF_ENERGYCOAT; }"; };','','');
REPLACE INTO `item_db_re` VALUES (2380,'Medic_Robe','Medic\'s Robe',5,10,NULL,0,NULL,25,NULL,1,0x00008110,7,2,16,NULL,80,1,0,'bonus bMaxHP,150; bonus bMdef,2; bonus2 bSubRace,RC_DemiHuman,2; if (isequipped(2539,2436)) { bonus2 bSubRace,RC_NonDemiHuman,-300; bonus bInt,3; bonus bMaxHPRate,12; bonus2 bCastrate,156,-50; bonus bHealPower,6; autobonus2 "{ bonus bDefEle,Ele_Ghost; }",30,10000,BF_WEAPON,"{ specialeffect2 EF_ENERGYCOAT; }"; };','','');
REPLACE INTO `item_db_re` VALUES (2381,'Elite_Archer_Suit','Elite Archer Suit',5,10,NULL,0,NULL,35,NULL,1,0x00080808,7,2,16,NULL,80,1,0,'bonus bMaxHP,150; bonus bMdef,2; bonus2 bSubRace,RC_DemiHuman,2; if (isequipped(2539,2436)) { bonus2 bSubRace,RC_NonDemiHuman,-300; bonus bDex,3; bonus bMaxHPRate,12; bonus bLongAtkDef,10; bonus bDelayRate,-25; };','','');
REPLACE INTO `item_db_re` VALUES (2382,'Elite_Shooter_Suit','Elite Shooter Suit',5,10,NULL,0,NULL,25,NULL,1,0x01000000,7,2,16,NULL,80,1,0,'bonus bMaxHP,150; bonus bMdef,2; bonus2 bSubRace,RC_DemiHuman,2; if (isequipped(2540,2437)) { bonus2 bSubRace,RC_NonDemiHuman,-300; bonus bDex,3; bonus bMaxHPRate,12; bonus bLongAtkDef,10; bonus bDelayRate,-25; };','','');
REPLACE INTO `item_db_re` VALUES (2383,'Brynhild','Brynhild',5,20,NULL,400,NULL,120,NULL,0,0xFFFFFFFF,7,2,16,NULL,94,0,0,'bonus bMdef,10; bonus bMaxHP,20*BaseLevel; bonus bMaxSP,5*BaseLevel; bonus2 bAddRace,RC_NonBoss,10; bonus2 bAddRace,RC_Boss,10; bonus bMatkRate,10; bonus bUnbreakableArmor,0; bonus bNoKnockback,0;','','');
REPLACE INTO `item_db_re` VALUES (2386,'Chameleon_Armor','Chameleon Armor',5,20,NULL,1700,NULL,55,NULL,0,0x00CFFF80,2,2,16,NULL,70,1,0,'bonus bMaxHP,(BaseLevel*7); bonus bMaxSP,(BaseLevel/2); autobonus2 "{ bonus bNoMagicDamage,100; }",10,2000,BF_MAGIC,"{ specialeffect2 EF_ENERGYCOAT; }"; if( BaseClass == Job_Mage || BaseClass == Job_Archer || BaseClass == Job_Acolyte ) bonus bMdef,5; else if( BaseClass == Job_Swordman || BaseClass == Job_Merchant || BaseClass == Job_Thief ) bonus bDef,3;','','');
REPLACE INTO `item_db_re` VALUES (2387,'Sprint_Mail','Sprint Mail',5,20,NULL,1000,NULL,20,NULL,1,0x00CFFF80,2,2,16,NULL,70,1,0,'bonus bVit,1; bonus bHPrecovRate,5; bonus bAddItemHealRate,3; bonus2 bSkillHeal,AL_HEAL,3; if( isequipped(2440,2744) ) { bonus bMaxHPrate,7; bonus bMaxSPrate,7; bonus bCastrate,-3; bonus bDelayrate,-15; }','','');
REPLACE INTO `item_db_re` VALUES (2388,'Kandura','Kandura',5,20,NULL,300,NULL,36,NULL,1,0x00001000,2,2,16,NULL,70,1,0,'bonus bAgi,1; bonus bFlee,5; bonus bAspdRate,2;','','');
REPLACE INTO `item_db_re` VALUES (2389,'Armor_Of_Naga','Armor of Naga',5,20,NULL,1000,NULL,45,NULL,1,0x00CFFF80,2,2,16,NULL,70,1,0,'bonus bMdef,2; autobonus "{ bonus bBaseAtk,20; }",10,10000,BF_WEAPON,"{ specialeffect2 EF_ENHANCE; }";','','');
REPLACE INTO `item_db_re` VALUES (2390,'Improved_Tights','Improved Tights',5,20,NULL,400,NULL,38,NULL,1,0x00080808,2,2,16,NULL,75,1,0,'bonus bMdef,2; bonus bFlee2,3;','','');
REPLACE INTO `item_db_re` VALUES (2391,'Life_Link','Life Link',5,20,NULL,3500,NULL,75,NULL,1,0x00004082,2,2,16,NULL,82,1,0,'bonus bVit,2; bonus bMdef,5; bonus bHPrecovRate,50;','','');

# Garment
REPLACE INTO `item_db_re` VALUES (2501,'Hood','Hood',5,1000,NULL,200,NULL,4,NULL,0,0xFFFFFFFF,7,2,4,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2502,'Hood_','Hood',5,1000,NULL,200,NULL,4,NULL,1,0xFFFFFFFF,7,2,4,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2503,'Muffler','Muffler',5,5000,NULL,400,NULL,8,NULL,0,0xFFFFFFFE,7,2,4,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2504,'Muffler_','Muffler',5,5000,NULL,400,NULL,8,NULL,1,0xFFFFFFFE,7,2,4,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2505,'Manteau','Manteau',5,32000,NULL,600,NULL,13,NULL,0,0x006654E2,7,2,4,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2506,'Manteau_','Manteau',5,32000,NULL,600,NULL,13,NULL,1,0x006654E2,7,2,4,NULL,0,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2507,'Cape_Of_Ancient_Lord','Ancient Cape',5,82000,NULL,600,NULL,9,NULL,0,0xFFFFFFFE,7,2,4,NULL,40,1,0,'bonus bAgi,1;','','');
REPLACE INTO `item_db_re` VALUES (2508,'Ragamuffin_Cape','Ragamuffin Manteau',5,56000,NULL,500,NULL,4,NULL,0,0xFFFFFFFE,7,2,4,NULL,0,1,0,'bonus bUnbreakableGarment,0; bonus bMdef,10;','','');
REPLACE INTO `item_db_re` VALUES (2509,'Clack_Of_Servival','Survivor\'s Manteau',5,20000,NULL,550,NULL,10,NULL,0,0x00810204,7,2,4,NULL,75,1,0,'bonus bMdef,5; bonus bVit,10; if(isequipped(1618) || isequipped(1620)){ bonus bMaxHP,300; bonus bMatkRate,getequiprefinerycnt(EQI_HAND_R)-5; bonus2 bSubEle,Ele_Neutral,getrefine()*3; }','','');
REPLACE INTO `item_db_re` VALUES (2510,'Novice_Hood','Somber Novice Hood',5,1,NULL,1,NULL,4,NULL,0,0x00000001,7,2,4,NULL,0,0,0,'bonus2 bSubEle,Ele_Neutral,20;','','');
REPLACE INTO `item_db_re` VALUES (2512,'Novice_Manteau','Novice Manteau',5,50000,NULL,500,NULL,7,NULL,1,0x00000001,7,2,4,NULL,40,1,0,'bonus2 bSubEle,Ele_Neutral,10;','','');
REPLACE INTO `item_db_re` VALUES (2513,'Celestial_Robe','Heavenly Maiden Robe',5,20,NULL,500,NULL,18,NULL,1,0xFFFFFFFE,7,2,4,NULL,80,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2514,'Pauldron','Pauldron',5,20,NULL,800,NULL,25,NULL,1,0x000654E2,7,2,4,NULL,80,1,0,'','','');
REPLACE INTO `item_db_re` VALUES (2515,'Wing_Of_Eagle','Eagle Wing',5,20000,NULL,300,NULL,12,NULL,1,0x00810204,7,2,4,NULL,85,1,0,'if(isequipped(1616)) bonus bSpeedRate,25;','','');
REPLACE INTO `item_db_re` VALUES (2516,'Falcon_Robe','Falcon Muffler',5,30000,NULL,400,NULL,8,NULL,0,0xFFFFFFFE,7,2,4,NULL,65,1,0,'bonus bFlee,15; bonus bFlee2,5;','','');
REPLACE INTO `item_db_re` VALUES (2517,'Vali\'s_Manteau','Vali\'s Manteau',5,30000,NULL,600,NULL,13,NULL,0,0xFFFFFFFE,7,2,4,NULL,65,1,0,'bonus2 bSubEle,Ele_Neutral,15;','','');
REPLACE INTO `item_db_re` VALUES (2518,'Morpheus\'s_Shawl','Morpheus\'s Shawl',5,30000,NULL,600,NULL,8,NULL,0,0xFFFFFFFE,7,2,4,NULL,33,1,0,'bonus bMaxSPrate,10; bonus bMdef,3;','','');
REPLACE INTO `item_db_re` VALUES (2519,'Morrigane\'s_Manteau','Morrigane\'s Manteau',5,30000,NULL,600,NULL,9,NULL,0,0xFFFFFFFE,7,2,4,NULL,61,1,0,'bonus bLuk,2; bonus bFlee2,8;','','');
REPLACE INTO `item_db_re` VALUES (2520,'Goibne\'s_Shoulder_Arms','Goibne\'s Spaulders',5,30000,NULL,700,NULL,11,NULL,0,0xFFFFFFFE,7,2,4,NULL,54,1,0,'bonus bLongAtkDef,10; bonus bMdef,2; bonus bVit,1;','','');
REPLACE INTO `item_db_re` VALUES (2521,'Angel\'s_Warmth','Angelic Cardigan',5,10000,NULL,400,NULL,5,NULL,1,0x00000001,7,2,4,NULL,20,1,0,'bonus bHPrecovRate,5;','','');
REPLACE INTO `item_db_re` VALUES (2522,'Undershirt','Undershirt',5,20000,NULL,150,NULL,5,NULL,0,0xFFFFFFFF,7,2,4,NULL,1,1,0,'bonus bMdef,1; if(isequipped(2339) || isequipped(2371)) { bonus bAgi,5; bonus bFlee,10; }','','');
REPLACE INTO `item_db_re` VALUES (2523,'Undershirt_','Undershirt',5,20000,NULL,150,NULL,5,NULL,1,0xFFFFFFFF,7,2,4,NULL,1,1,0,'bonus bMdef,1; if(isequipped(2339) || isequipped(2371)) { bonus bAgi,5; bonus bFlee,10; }','','');
REPLACE INTO `item_db_re` VALUES (2524,'Valkyrie_Manteau','Valkyrian Manteau',5,0,NULL,500,NULL,10,NULL,1,0xFFFFFFFE,2,2,4,NULL,1,1,0,'bonus bUnbreakableGarment,0; if(BaseClass==Job_Mage||BaseClass==Job_Archer||BaseClass==Job_Acolyte) bonus bFlee2,5+(getequiprefinerycnt(EQI_GARMENT)*2); else if(BaseClass==Job_Swordman||BaseClass==Job_Merchant||BaseClass==Job_Thief) bonus bShortWeaponDamageReturn,5+(getequiprefinerycnt(EQI_GARMENT)*2);','','');
REPLACE INTO `item_db_re` VALUES (2525,'Cape_Of_Ancient_Lord_','Ancient Cape',5,82000,NULL,600,NULL,9,NULL,1,0xFFFFFFFE,7,2,4,NULL,40,1,0,'bonus bAgi,1;','','');
REPLACE INTO `item_db_re` VALUES (2527,'Dragon_Breath','Dragon Breath',5,20,NULL,600,NULL,16,NULL,1,0xFFFFFFFE,2,2,4,NULL,48,1,0,'bonus2 bSubRace,RC_Dragon,15; if (isequipped(1166) || isequipped(13001) || isequipped(1474)) bonus2 bAddRace,RC_Dragon,5;','','');
REPLACE INTO `item_db_re` VALUES (2528,'Wool_Scarf','Wool Scarf',5,20,NULL,500,NULL,11,NULL,1,0xFFFFFFFE,2,2,4,NULL,55,1,0,'bonus bMdef,4;','','');
REPLACE INTO `item_db_re` VALUES (2529,'Rider_Insignia','Rider Insignia',5,20,NULL,500,NULL,13,NULL,0,0xFFFFFFFE,2,2,4,NULL,55,1,0,'bonus bAgi,2; if (isequipped(2425) || isequipped(2434)) bonus bFlee,10;','','');
REPLACE INTO `item_db_re` VALUES (2530,'Rider_Insignia_','Rider Insignia',5,20,NULL,500,NULL,13,NULL,1,0xFFFFFFFE,2,2,4,NULL,55,1,0,'bonus bAgi,2; if (isequipped(2425) || isequipped(2434)) bonus bFlee,10;','','');
REPLACE INTO `item_db_re` VALUES (2531,'Ulfhedinn','Ulfhedinn',5,20,NULL,700,NULL,13,NULL,1,0x000654E2,2,2,4,NULL,70,1,0,'bonus3 bAutoSpellWhenHit,NPC_STONESKIN,1,20;','','');
REPLACE INTO `item_db_re` VALUES (2532,'Mithril_Magic_Cape','Mithril Magic Cape',5,20,NULL,400,NULL,8,NULL,1,0x00098B1C,2,2,4,NULL,70,1,0,'bonus bMdef,3; bonus5 bAutoSpellWhenHit,NPC_ANTIMAGIC,1,200,BF_MAGIC,0;','','');
REPLACE INTO `item_db_re` VALUES (2536,'Skin_Of_Ventus','Skin of Ventus',5,20,NULL,250,NULL,7,NULL,1,0xFFFFFFFE,7,2,4,NULL,60,1,0,'bonus bMdef,2; bonus bMaxHP,200; bonus bFlee,10;','','');
REPLACE INTO `item_db_re` VALUES (2537,'Diabolus_Manteau','Diabolus Manteau',5,20,NULL,250,NULL,15,NULL,1,0x00CFFF80,2,2,4,NULL,0,1,0,'bonus2 bSubEle,Ele_Neutral,5; bonus bMaxHP,100; bonus2 bAddDamageClass,1916,10; bonus2 bAddDamageClass,1917,10; if (isequipped(2433)) bonus bMaxHPRate,6;','','');
REPLACE INTO `item_db_re` VALUES (2538,'Commander_Manteau','Captain\'s Manteau',5,10,NULL,0,NULL,28,NULL,1,0x026654E2,7,2,4,NULL,80,1,0,'bonus bMaxHP,50; bonus bMdef,1; bonus2 bSubRace,RC_DemiHuman,1;','','');
REPLACE INTO `item_db_re` VALUES (2539,'Commander_Manteau_','Commander\'s Manteau',5,10,NULL,0,NULL,20,NULL,1,0x00898B1C,7,2,4,NULL,80,1,0,'bonus bMaxHP,50; bonus bMdef,1; bonus2 bSubRace,RC_DemiHuman,1;','','');
REPLACE INTO `item_db_re` VALUES (2540,'Sheriff_Manteau','Sheriff\'s Manteau',5,10,NULL,0,NULL,20,NULL,1,0x01000000,7,2,4,NULL,80,1,0,'bonus bMaxHP,50; bonus bMdef,1; bonus2 bSubRace,RC_DemiHuman,1;','','');
REPLACE INTO `item_db_re` VALUES (2541,'Asprika','Asprika',5,20,NULL,400,NULL,40,NULL,0,0xFFFFFFFF,7,2,4,NULL,94,0,0,'bonus bMdef,5; bonus3 bSubEle,Ele_Neutral,30,BF_SHORT; bonus3 bSubEle,Ele_Water,30,BF_SHORT; bonus3 bSubEle,Ele_Earth,30,BF_SHORT; bonus3 bSubEle,Ele_Fire,30,BF_SHORT; bonus3 bSubEle,Ele_Wind,30,BF_SHORT; bonus3 bSubEle,Ele_Poison,30,BF_SHORT; bonus3 bSubEle,Ele_Holy,30,BF_SHORT; bonus3 bSubEle,Ele_Dark,30,BF_SHORT; bonus3 bSubEle,Ele_Ghost,30,BF_SHORT; bonus3 bSubEle,Ele_Undead,30,BF_SHORT; bonus bFlee,30; skill "AL_TELEPORT",1; bonus bUnbreakableGarment,0;','','');
REPLACE INTO `item_db_re` VALUES (2542,'Flame_Manteau','Flame Manteau of Naght Sieger',5,20,NULL,70,NULL,16,NULL,1,0xFFFFFFFE,2,2,4,NULL,70,1,0,'bonus bMaxHPRate,5; bonus bMdef,2; bonus bMatkRate,1; bonus2 bAddEle,Ele_Fire,2;','','');
REPLACE INTO `item_db_re` VALUES (2544,'Leather_Of_Tendrilion','Leather of Tendrilion',5,20,NULL,300,NULL,14,NULL,1,0x00CFDF80,2,2,4,NULL,0,1,0,'bonus2 bSubEle,Ele_Water,5; bonus2 bSubEle,Ele_Earth,5; bonus2 bSubRace,RC_Plant,5; bonus2 bSubRace,RC_Brute,5;','','');
REPLACE INTO `item_db_re` VALUES (2545,'Musika','Musika',5,20,NULL,500,NULL,10,NULL,1,0x00008100,2,2,4,NULL,70,1,0,'bonus bMdef,3; bonus3 bAutoSpellwhenhit,AL_HEAL,getskilllv("AL_HEAL") ? getskilllv("AL_HEAL") : 1,20;','','');
REPLACE INTO `item_db_re` VALUES (2548,'Muffler_C','Neo Muffler',5,0,NULL,0,NULL,5,NULL,0,0xFFFFFFFE,2,2,4,NULL,95,0,0,'bonus2 bSubRace,RC_DemiHuman,10; bonus bMaxHPrate,10; bonus2 bSubEle,Ele_Water,5; bonus2 bSubEle,Ele_Fire,5; bonus2 bSubEle,Ele_Holy,5; bonus2 bSubEle,Ele_Dark,5;','','');
REPLACE INTO `item_db_re` VALUES (2549,'Krieger_Muffler1','Glorious Muffler',5,20,NULL,0,NULL,0,NULL,0,0xFFFFFFFE,7,2,4,NULL,81,1,0,'bonus bMaxHPRate,5; bonus2 bSubRace,RC_DemiHuman,5;','','');
REPLACE INTO `item_db_re` VALUES (2553,'Dragon_Manteau','Dragon Manteau',5,20,NULL,1000,NULL,14,NULL,1,0xFFFFFFFE,2,2,4,NULL,0,1,0,'bonus bAgi,1; bonus bMdef,5;','','');
REPLACE INTO `item_db_re` VALUES (2554,'Piece_Of_Angent_Skin','Nydhorgg\'s Shadow Garb',5,20,NULL,400,NULL,25,NULL,1,0xFFFFFFFE,2,2,4,NULL,90,1,0,'bonus2 bSubEle,Ele_Neutral,7; bonus2 bSubEle,Ele_Water,7; bonus2 bSubEle,Ele_Earth,7; bonus2 bSubEle,Ele_Fire,7; bonus2 bSubEle,Ele_Wind,7; bonus2 bSubEle,Ele_Poison,7; bonus2 bSubEle,Ele_Holy,7; bonus2 bSubEle,Ele_Dark,7; bonus2 bSubEle,Ele_Ghost,7; bonus2 bSubEle,Ele_Undead,7; bonus bMaxSP,(BaseLevel/3)+(getrefine()*10); bonus3 bSPDrainRate,10,1,0; bonus bMdef,3;','','');
REPLACE INTO `item_db_re` VALUES (2560,'Para_Team_Manteau','Eden Team Manteau',5,0,NULL,0,NULL,14,NULL,0,0xFFFFFFFF,7,2,4,NULL,12,0,0,'bonus2 bSubEle,Ele_Neutral,10;','','');
REPLACE INTO `item_db_re` VALUES (2564,'Feral_Tail','Feral Tail',5,20,NULL,0,NULL,16,NULL,0,0xFFFFFFFF,7,2,4,NULL,75,0,0,'','','');

#  Accessories
REPLACE INTO `item_db` VALUES (2629,'Magingiorde','Megingjard',5,20,NULL,8000,NULL,2,NULL,0,0xFFFFFFFF,7,2,136,NULL,94,0,0,'bonus bStr,40+BaseLevel/5; bonus bMdef,7; if(readparam(bStr)==120) bonus2 bAddRace,RC_Boss,10;',NULL,NULL);

#Cards
REPLACE INTO `item_db` VALUES (4302,'Tao_Gunka_Card','Tao Gunka Card',6,20,NULL,10,NULL,NULL,NULL,NULL,NULL,NULL,NULL,16,NULL,NULL,NULL,NULL,'bonus bMaxHPrate,100; bonus bDefRate,-50; bonus bMdefRate,-50;',NULL,NULL);