#
# Table structure for table `item_db2_re`
#

DROP TABLE IF EXISTS `item_db2_re`;
CREATE TABLE `item_db2_re` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `name_aegis` varchar(50) NOT NULL DEFAULT '',
  `name_english` varchar(50) NOT NULL DEFAULT '',
  `type` varchar(20) NOT NULL DEFAULT 'Etc',
  `subtype` varchar(20) NOT NULL DEFAULT '',
  `price_buy` mediumint(8) unsigned DEFAULT NULL,
  `price_sell` mediumint(8) unsigned DEFAULT NULL,
  `weight` smallint(5) unsigned DEFAULT NULL,
  `attack` smallint(5) unsigned DEFAULT NULL,
  `defense` smallint(5) unsigned DEFAULT NULL,
  `range` tinyint(2) unsigned DEFAULT NULL,
  `slots` tinyint(2) unsigned DEFAULT NULL,
  `equip_jobs` text,
  `equip_upper` text,
  `equip_genders` varchar(10) DEFAULT NULL,
  `equip_locations` text,
  `weapon_level` tinyint(1) unsigned DEFAULT NULL,
  `equip_level_min` tinyint(3) unsigned DEFAULT NULL,
  `equip_level_max` tinyint(3) unsigned DEFAULT NULL,
  `refineable` tinyint(1) unsigned DEFAULT NULL,
  `view` smallint(5) unsigned DEFAULT NULL,
  `alias_name` varchar(50) NOT NULL DEFAULT '',
  `flag_buyingstore` tinyint(1) unsigned DEFAULT NULL,
  `flag_deadbranch` tinyint(1) unsigned DEFAULT NULL,
  `flag_container` tinyint(1) unsigned DEFAULT NULL,
  `flag_uniqueid` tinyint(1) unsigned DEFAULT NULL,
  `flag_bindonequip` tinyint(1) unsigned DEFAULT NULL,
  `flag_dropannounce` tinyint(1) unsigned DEFAULT NULL,
  `flag_noconsume` tinyint(1) unsigned DEFAULT NULL,
  `flag_dropeffect` varchar(20) DEFAULT NULL,
  `delay_duration` bigint(20) unsigned DEFAULT NULL,
  `delay_status` varchar(30) DEFAULT NULL,
  `stack_amount` smallint(5) unsigned DEFAULT NULL,
  `stack_inventory` tinyint(1) unsigned DEFAULT NULL,
  `stack_cart` tinyint(1) unsigned DEFAULT NULL,
  `stack_storage` tinyint(1) unsigned DEFAULT NULL,
  `stack_guildstorage` tinyint(1) unsigned DEFAULT NULL,
  `nouse_override` smallint(5) unsigned DEFAULT NULL,
  `nouse_sitting` tinyint(1) unsigned DEFAULT NULL,
  `trade_override` smallint(5) unsigned DEFAULT NULL,
  `trade_nodrop` tinyint(1) unsigned DEFAULT NULL,
  `trade_notrade` tinyint(1) unsigned DEFAULT NULL,
  `trade_tradepartner` tinyint(1) unsigned DEFAULT NULL,
  `trade_nosell` tinyint(1) unsigned DEFAULT NULL,
  `trade_nocart` tinyint(1) unsigned DEFAULT NULL,
  `trade_nostorage` tinyint(1) unsigned DEFAULT NULL,
  `trade_noguildstorage` tinyint(1) unsigned DEFAULT NULL,
  `trade_nomail` tinyint(1) unsigned DEFAULT NULL,
  `trade_noauction` tinyint(1) unsigned DEFAULT NULL,
  `script` text,
  `equip_script` text,
  `unequip_script` text,
  PRIMARY KEY (`id`),
  UNIQUE INDEX `UniqueAegisName` (`name_aegis`)
) ENGINE=MyISAM;

# THQ Quest Items
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (7950,'THG_Membership','THG Membership','Etc',10,10);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (7951,'Token_Bag','Token Bag','Etc',10,10);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (1998,'Jeramiah\'s_Jur','Jeramiah\'s Jur','Etc',10,10);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (1999,'Zed\'s_Staff','Zed\'s Staff','Etc',10,10);

# Official Event Items that had their Effects removed after the event was completed
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`script`) VALUES (585,'Wurst','Brusti','Delayconsume',2,40,'itemheal rand(15,20),0; itemskill "PR_MAGNIFICAT",3;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`script`) VALUES (679,'Gold_Pill','Pilule','Healing',5000,300,'percentheal 50,50;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`equip_locations`,`script`) VALUES (2681,'Republic_Ring','Republic Anniversary Ring','Armor',20,100,'Right_Accessory:true|Left_Accessory:true','bonus bAllStats,3;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5134,'Pumpkin_Hat','Pumpkin-Head','Armor',20,200,2,'Head_Top:true',true,206,'bonus2 bSubRace,RC_Demon,5;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5136,'Santa\'s_Hat_','Louise\'s Santa Hat','Armor',20,100,3,'Head_Top:true',true,20,'bonus bMdef,1; bonus bLuk,1; bonus3 bAutoSpellWhenHit,"AL_HEAL",3,50; bonus3 bAutoSpellWhenHit,"AL_BLESSING",10,50;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5145,'Carnival_Joker_Jester','Carnival Jester','Armor',10,100,'Head_Top:true',true,89,'bonus bAllStats,3;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5147,'Baseball_Cap','Baseball Cap','Armor',200,3,'Head_Top:true',true,216,'bonus2 bExpAddRace,RC_Boss,50; bonus2 bExpAddRace,RC_NonBoss,50;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5201,'Party_Hat_B','2nd Anniversary Party Hat','Armor',20,300,3,'Head_Top:true',true,144,'bonus bAllStats,3;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5202,'Pumpkin_Hat_','Pumpkin Hat','Armor',20,200,2,'Head_Top:true',true,206,'bonus bAllStats,2; bonus2 bSubRace,RC_Demon,5; bonus3 bAddMonsterDropItem,529,RC_DemiHuman,1500;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`equip_locations`,`view`,`script`) VALUES (5204,'Event_Pierrot_Nose','Rudolf\'s Red Nose','Armor',20,100,'Head_Low:true',49,'bonus2 bResEff,Eff_Blind,3000; bonus2 bAddMonsterDropItem,12130,30;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5264,'Aussie_Flag_Hat','Australian Flag Hat','Armor',20,500,4,'Head_Top:true',true,304,'bonus bAllStats,2;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5356,'Pumpkin_Hat_H','Pumpkin Hat','Armor',20,200,2,'Head_Top:true',true,206,'bonus bAllStats,2; bonus2 bSubRace,RC_Demon,5; bonus2 bMagicAddRace,RC_Demon,5;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`slots`,`equip_locations`,`equip_level_min`,`refineable`,`view`,`script`) VALUES (5384,'Santa_Hat_1','Twin Pompom By JB','Armor',20,200,2,1,'Head_Top:true',20,true,390,'bonus bLuk,3; bonus2 bResEff,Eff_Curse,2000; bonus bVariableCastrate,-2; bonus bAspdRate,4; bonus2 bAddMonsterDropItem,539,100; bonus2 bAddMonsterDropItem,529,200; bonus2 bAddMonsterDropItem,530,200; autobonus "{ bonus bCritical,10; }",10,5000;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`equip_locations`,`view`,`script`) VALUES (5811,'Santa_Beard','Santa Beard','Armor',20,100,5,'Head_Low:true',25,'bonus2 bSubRace,RC_Brute,5;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`script`) VALUES (11702,'Moon_Cookie','Moon Cookie','Delayconsume',10,'sc_end SC_POISON; sc_end SC_SILENCE; sc_end SC_BLIND; sc_end SC_CONFUSION; sc_end SC_CURSE; sc_end SC_HALLUCINATION; itemskill "AL_BLESSING",7;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`script`) VALUES (12131,'Lucky_Potion','Lucky Potion','Healing',2,100,'sc_start SC_LUKFOOD,180000,15;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`script`) VALUES (12143,'Red_Can','Red Can','Usable',50000,300,'percentheal 25,25;');

# Event effect: Summon monster? Probably Rice_Cake. x_x
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`) VALUES (12199,'Rice_Scroll','Rice Scroll','Usable');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`script`) VALUES (12200,'Event_Cake','Event Cake','Usable',20,50,'itemskill "PR_MAGNIFICAT",3;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`script`) VALUES (12238,'New_Year_Rice_Cake_1','New Year Rice Cake','Healing',20,100,'percentheal 20,15; sc_start SC_STRFOOD,1200000,3; sc_start SC_INTFOOD,1200000,3; sc_start SC_LUKFOOD,1200000,3; sc_start SC_SPEEDUP1,5000,0;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`script`) VALUES (12239,'New_Year_Rice_Cake_2','New Year Rice Cake','Healing',20,100,'percentheal 20,15; sc_start SC_DEXFOOD,1200000,3; sc_start SC_AGIFOOD,1200000,3; sc_start SC_VITFOOD,1200000,3; sc_start SC_SPEEDUP1,5000,0;');

# iRO St. Patrick's Day Event 2008
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`script`) VALUES (12715,'Black_Treasure_Chest','Black Treasure Chest','Usable',200,'callfunc "F_08stpattyseventbox";');

# iRO Valentine's Day Event 2009
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`script`) VALUES (12742,'Valentine_Gift_Box_M','Valentine Gift Box','Usable',10,'getitem 7946,1;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`script`) VALUES (12743,'Valentine_Gift_Box_F','Valentine Gift Box','Usable',10,'getitem 7947,1;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`script`) VALUES (12744,'Chocolate_Box','Chocolate Box','Usable',10,'getitem 558,1;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`script`) VALUES (14466,'Valentine\'s_Emblem_Box','Valentine\'s Emblem Box','Usable',10,'getitem 5817,1;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`) VALUES (7946,'Gold_Ring_Of_Valentine','Gold Ring Of Valentine','Etc',10);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`) VALUES (7947,'Silver_Ring_Of_Valentine','Silver Ring Of Valentine','Etc',10);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`) VALUES (7948,'Box','Box','Etc',10,10);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`defense`,`equip_locations`,`script`) VALUES (5817,'Valentine\'s_Emblem','Valentine\'s Emblem','Armor',10,3,'Right_Accessory:true|Left_Accessory:true','bonus bAtkRate,3; bonus bMatkRate,3; bonus bAllStats,2; bonus bFlee,10; bonus bAspd,1; bonus bMdef,3; bonus2 bSkillAtk,"AL_HEAL",10; bonus2 bSkillHeal,"AL_HEAL",10; bonus2 bSkillHeal,"AM_POTIONPITCHER",10; bonus2 bAddItemGroupHealRate,IG_Potion,10;');

# iRO Halloween Event 2009
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`defense`,`equip_locations`,`refineable`,`view`,`script`) VALUES (5668,'Weird_Pumpkin_Hat','Weird Pumpkin Hat','Armor',20,5,'Head_Top:true',true,206,'bonus bMdef,5; bonus2 bAddMonsterDropItem,12192,2500;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`) VALUES (6298,'Crushed_Pumpkin','Crushed Pumpkin','Etc');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`) VALUES (6299,'Worn_Fabric','Worn Fabric','Etc');

# Old Tuxedo and Wedding Dress, will display the outfit when worn.
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`equip_jobs`,`equip_locations`,`refineable`,`equip_script`,`unequip_script`) VALUES (2338,'Wedding_Dress','Wedding Dress','Armor',43000,500,'All:true|Novice:false','Armor:true',true,'sc_start SC_WEDDING,INFINITE_TICK,0;','sc_end SC_WEDDING;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`equip_jobs`,`equip_genders`,`equip_locations`,`refineable`,`equip_script`,`unequip_script`) VALUES (7170,'Tuxedo','Tuxedo','Armor',43000,10,'All:true|Novice:false','Male','Armor:true',true,'sc_start SC_WEDDING,INFINITE_TICK,0;','sc_end SC_WEDDING;');

# Non-kRO Eden Group Mark effect
#=============================================================
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`script`) VALUES (22508,'Para_Team_Mark_','Eden Group Mark','Delayconsume','unitskilluseid getcharid(3),"AL_TELEPORT",3;');
