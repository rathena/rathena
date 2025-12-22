#
# Table structure for table `item_db2_re`
#

DROP TABLE IF EXISTS `item_db2_re`;
CREATE TABLE `item_db2_re` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `name_aegis` varchar(50) DEFAULT NULL,
  `name_english` varchar(100) DEFAULT NULL,
  `type` varchar(20) DEFAULT NULL,
  `subtype` varchar(20) DEFAULT NULL,
  `price_buy` mediumint(8) unsigned DEFAULT NULL,
  `price_sell` mediumint(8) unsigned DEFAULT NULL,
  `weight` smallint(5) unsigned DEFAULT NULL,
  `attack` smallint(5) unsigned DEFAULT NULL,
  `magic_attack` smallint(5) unsigned DEFAULT NULL,
  `defense` smallint(5) unsigned DEFAULT NULL,
  `range` tinyint(2) unsigned DEFAULT NULL,
  `slots` tinyint(2) unsigned DEFAULT NULL,
  `job_all` tinyint(1) unsigned DEFAULT NULL,
  `job_acolyte` tinyint(1) unsigned DEFAULT NULL,
  `job_alchemist` tinyint(1) unsigned DEFAULT NULL,
  `job_archer` tinyint(1) unsigned DEFAULT NULL,
  `job_assassin` tinyint(1) unsigned DEFAULT NULL,
  `job_barddancer` tinyint(1) unsigned DEFAULT NULL,
  `job_blacksmith` tinyint(1) unsigned DEFAULT NULL,
  `job_crusader` tinyint(1) unsigned DEFAULT NULL,
  `job_gunslinger` tinyint(1) unsigned DEFAULT NULL,
  `job_hunter` tinyint(1) unsigned DEFAULT NULL,
  `job_kagerouoboro` tinyint(1) unsigned DEFAULT NULL,
  `job_knight` tinyint(1) unsigned DEFAULT NULL,
  `job_mage` tinyint(1) unsigned DEFAULT NULL,
  `job_merchant` tinyint(1) unsigned DEFAULT NULL,
  `job_monk` tinyint(1) unsigned DEFAULT NULL,
  `job_ninja` tinyint(1) unsigned DEFAULT NULL,
  `job_novice` tinyint(1) unsigned DEFAULT NULL,
  `job_priest` tinyint(1) unsigned DEFAULT NULL,
  `job_rebellion` tinyint(1) unsigned DEFAULT NULL,
  `job_rogue` tinyint(1) unsigned DEFAULT NULL,
  `job_sage` tinyint(1) unsigned DEFAULT NULL,
  `job_soullinker` tinyint(1) unsigned DEFAULT NULL,
  `job_spirit_handler` tinyint(1) unsigned DEFAULT NULL,
  `job_stargladiator` tinyint(1) unsigned DEFAULT NULL,
  `job_summoner` tinyint(1) unsigned DEFAULT NULL,
  `job_supernovice` tinyint(1) unsigned DEFAULT NULL,
  `job_swordman` tinyint(1) unsigned DEFAULT NULL,
  `job_taekwon` tinyint(1) unsigned DEFAULT NULL,
  `job_thief` tinyint(1) unsigned DEFAULT NULL,
  `job_wizard` tinyint(1) unsigned DEFAULT NULL,
  `class_all` tinyint(1) unsigned DEFAULT NULL,
  `class_normal` tinyint(1) unsigned DEFAULT NULL,
  `class_upper` tinyint(1) unsigned DEFAULT NULL,
  `class_baby` tinyint(1) unsigned DEFAULT NULL,
  `class_third` tinyint(1) unsigned DEFAULT NULL,
  `class_third_upper` tinyint(1) unsigned DEFAULT NULL,
  `class_third_baby` tinyint(1) unsigned DEFAULT NULL,
  `class_fourth` tinyint(1) unsigned DEFAULT NULL,
  `gender` varchar(10) DEFAULT NULL,
  `location_head_top` tinyint(1) unsigned DEFAULT NULL,
  `location_head_mid` tinyint(1) unsigned DEFAULT NULL,
  `location_head_low` tinyint(1) unsigned DEFAULT NULL,
  `location_armor` tinyint(1) unsigned DEFAULT NULL,
  `location_right_hand` tinyint(1) unsigned DEFAULT NULL,
  `location_left_hand` tinyint(1) unsigned DEFAULT NULL,
  `location_garment` tinyint(1) unsigned DEFAULT NULL,
  `location_shoes` tinyint(1) unsigned DEFAULT NULL,
  `location_right_accessory` tinyint(1) unsigned DEFAULT NULL,
  `location_left_accessory` tinyint(1) unsigned DEFAULT NULL,
  `location_costume_head_top` tinyint(1) unsigned DEFAULT NULL,
  `location_costume_head_mid` tinyint(1) unsigned DEFAULT NULL,
  `location_costume_head_low` tinyint(1) unsigned DEFAULT NULL,
  `location_costume_garment` tinyint(1) unsigned DEFAULT NULL,
  `location_ammo` tinyint(1) unsigned DEFAULT NULL,
  `location_shadow_armor` tinyint(1) unsigned DEFAULT NULL,
  `location_shadow_weapon` tinyint(1) unsigned DEFAULT NULL,
  `location_shadow_shield` tinyint(1) unsigned DEFAULT NULL,
  `location_shadow_shoes` tinyint(1) unsigned DEFAULT NULL,
  `location_shadow_right_accessory` tinyint(1) unsigned DEFAULT NULL,
  `location_shadow_left_accessory` tinyint(1) unsigned DEFAULT NULL,
  `weapon_level` tinyint(1) unsigned DEFAULT NULL,
  `armor_level` tinyint(1) unsigned DEFAULT NULL,
  `equip_level_min` smallint unsigned DEFAULT NULL,
  `equip_level_max` smallint unsigned DEFAULT NULL,
  `refineable` tinyint(1) unsigned DEFAULT NULL,
  `gradable` tinyint(1) unsigned DEFAULT NULL,
  `view` smallint(5) unsigned DEFAULT NULL,
  `alias_name` varchar(50) DEFAULT NULL,
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

REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`subtype`,`price_buy`,`weight`,`attack`,`magic_attack`,`range`,`location_right_hand`,`weapon_level`,`equip_level_min`,`refineable`,`script`) VALUES (1599,'Angra_Manyu','Angra Manyu','Weapon','Mace',1,10,5000,5000,2,true,1,1,true,'skill "WZ_STORMGUST",10;\nSkill "WZ_METEOR",10;\nSkill "WZ_VERMILION",10;\nskill "GM_SANDMAN",1;\nbonus bAllStats,500;\nbonus2 bHPDrainRate,1000,100;\nbonus2 bSPDrainRate,1000,100;\nbonus bHealPower,1000;\nbonus bAspdRate,1000;\nbonus bNoSizeFix;\nbonus2 bAddClass,Class_All,100;\nbonus bHit,1000;\nbonus bVariableCastrate,-100; // remove 100% do cast variável\nbonus bFixedCastrate,-100;    // remove 100% do cast fixo (Renewal)\nbonus bDelayrate,-100;        // (opcional) remove delay pós-cast');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`location_left_hand`,`armor_level`,`equip_level_min`,`view`,`script`) VALUES (2199,'Ahura_Mazda','Ahura Mazdah','Armor',1,10,127,true,1,1,1,'skill "CR_FULLPROTECTION",5;\nSkill "WZ_ESTIMATION",1;\nSkill "ST_FULLSTRIP",5;\nSkill "HW_MAGICPOWER",10;\nbonus bAllStats,500;\nbonus bMdef,100;\nbonus bUnbreakableShield;\nbonus bNoMagicDamage,100;\nbonus bNoGemStone;\nbonus bSpeedRate,0;\nbonus bNoWalkDelay;\nbonus bMaxHP,99999999;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`) VALUES (1230001,'Bronze_Coin_Custom','Bronze Coin','Etc',0);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`) VALUES (1230002,'Silver_Coin_Custom','Silver Coin','Etc',0);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`trade_nodrop`,`trade_notrade`,`trade_nosell`,`trade_nocart`,`trade_noguildstorage`,`trade_nomail`,`trade_noauction`) VALUES (1230003,'Gold_Coin_Custom','Gold Coin','Etc',0,true,true,true,true,true,true,true);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`flag_noconsume`,`trade_nodrop`,`trade_notrade`,`trade_nosell`,`trade_nocart`,`trade_nostorage`,`trade_noguildstorage`,`trade_nomail`,`trade_noauction`,`script`) VALUES (12887,'C_Wing_Of_Fly','Infinite Flywing','Usable',0,true,true,true,true,true,true,true,true,true,'itemskill "AL_TELEPORT",1;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`flag_noconsume`,`trade_nodrop`,`trade_notrade`,`trade_nosell`,`trade_nocart`,`trade_nostorage`,`trade_noguildstorage`,`trade_nomail`,`trade_noauction`,`script`) VALUES (1230004,'C_Wing_Of_Butterfly','Infinite Butterfly Flywing','Usable',0,true,true,true,true,true,true,true,true,true,'itemskill "AL_TELEPORT",3;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`slots`,`job_priest`,`location_head_top`,`armor_level`,`equip_level_min`,`refineable`,`view`,`script`) VALUES (5747,'Mitra','Mitra','Armor',20,10,3,1,true,true,1,90,true,624,'bonus bVit,1;\nbonus bInt,1;\nbonus bMdef,5;\nbonus bHealPower,5;\nif (readparam(bInt)>=90) {\n   bonus bMatk,10;\n}');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`slots`,`location_head_top`,`location_head_mid`,`armor_level`,`equip_level_min`,`refineable`,`view`,`script`) VALUES (18542,'Love_Guard','Love Guard','Armor',20,50,10,1,true,true,1,10,true,716,'bonus bHealPower2,5;\nbonus bAddItemHealRate,5;\nbonus bHealPower,(getrefine()>6)?5:2;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`defense`,`job_assassin`,`job_priest`,`location_head_low`,`armor_level`,`equip_level_min`,`view`,`script`) VALUES (18774,'Improved_Assassin_Mask','Advanced Assassin Mask','Armor',20,10,1,true,true,true,1,70,180,'bonus bCritical,1;\nbonus bCritAtkRate,1;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`defense`,`location_head_low`,`armor_level`,`equip_level_min`,`view`,`script`) VALUES (123000,'Umbala_Spirit','Umbala Spirit','Armor',1,true,1,1,675,'bonus bMaxHPrate,1;\nbonus2 bAddMonsterDropItem,517,500;\nbonus2 bAddItemHealRate,517,50;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`equip_level_min`,`view`) VALUES (1230005,'c_loli_ruri_moon_b','Costume Blue Moon Loli Ruri','Armor',0,true,1,1,5000);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`equip_level_min`,`view`) VALUES (1230006,'c_helm_of_ra','Costume Sun God','Armor',0,true,1,1,5001);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230007,'C_SantaTeddyBear','Costume Santa Teddy Bear Doll Bag','Armor',0,true,1,5002);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230008,'c_leticia_wing','Costume Leticia Wing','Armor',0,true,1,5003);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230009,'greatdevilwing2','Costume Greatest Devil Wing','Armor',0,true,1,5004);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230010,'c_aqua_fin_decoration','Costume Aqua Fin Decoration','Armor',0,true,1,5005);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230011,'c_poring_basket','Costume Poring Basket','Armor',0,true,1,5006);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230012,'c_pretty_bear_wh','Costume Pretty White Bear','Armor',0,true,1,5007);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230013,'gold_majestic_gout','Costume Gold Baphomet Horns','Armor',0,true,1,5008);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230014,'Red_Baby_Dragon_LT','Costume Red Baby Dragon','Armor',0,true,1,5009);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230015,'Let_It_Snow','Costume Let It Snow','Armor',0,true,1,5010);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230016,'C_Vanargandr_Helm_GL','Costume Gold Helm of Vanagandr','Armor',0,true,1,5011);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230017,'C_Baby_Panda','Costume Baby Panda','Armor',0,true,1,5012);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230018,'dancingbutterfly','Costume Dancing Butterfly','Armor',0,true,1,5013);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230019,'flowery_vision_th','Costume Flowery Vision','Armor',0,true,1,5014);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230020,'Larva_Yellow_Mini','Costume Larva Yellow Mini-Me','Armor',0,true,1,5015);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230021,'c_seyren_mini','Costume Seyren Mini-Me','Armor',0,true,1,5016);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230022,'c_eremes_scarf_bu','Costume Eremes\' Scarf Blue','Armor',0,true,1,5017);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230023,'autumn_full_moon','Costume Autumn Fullmoon','Armor',0,true,1,5018);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230024,'C_Alpha_Long_Pony','Costume Alpha Long Ponytail','Armor',0,true,1,5019);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230025,'C_Deep_You_N','Costume Abyssal','Armor',0,true,1,5020);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230026,'OpenAir_Headset','Costume Abyssal','Armor',0,true,1,5021);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230027,'C_Poring_Muffler','Costume Poring Muffler','Armor',0,true,1,5022);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230028,'C_Protect_Cloth_b','Costume Protective Scarf','Armor',0,true,1,5023);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230029,'Cons_Of_Darkness','Costume Shadowbender','Armor',0,true,1,5024);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230030,'Bow_On_the_Back','Costume Bow on Back','Armor',0,true,1,5025);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230031,'C_Baphomet_Scythe','Costume Baphomet Scythe','Armor',0,true,1,5026);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230032,'C_Book_Of_Magic','Costume Book of Magic','Armor',0,true,1,5027);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230033,'C_Cat_Footprints','Costume Cat Footprints','Armor',0,true,1,5028);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230034,'c_wanderer_grim_reaper','Costume Wanderer Reaper','Armor',0,true,1,5029);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230035,'C_Rabbit_Hopping_BL','Costume Black Rabbit Ears','Armor',0,true,1,5030);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230036,'khalitzburg_kn_helm_bl','Costume Black Khalitzburg Knight Helm','Armor',0,true,1,5031);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230037,'C_Choco_Banana','Costume Choco Banana','Armor',0,true,1,5032);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230038,'C_B_Shiba_Inu_TW','Costume Black Shiba Inu','Armor',0,true,1,5033);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230039,'C_N_Forest_Cat','Costume Forest Cat','Armor',0,true,1,5034);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230040,'C_Fire_Giant_Gold','Costume Fire Gold Giant','Armor',0,true,1,5035);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230041,'C_Fire_Giant_Red','Costume Fire Red Giant','Armor',0,true,1,5036);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230042,'C_Braids_Wave','Costume Braid Wave','Armor',0,true,1,5037);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`location_costume_head_Mid`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230043,'c_helm_of_galaxia','Costume Helm of Galaxia','Armor',0,true,true,true,1,5038);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`location_costume_head_Mid`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230044,'c_jormun_cape','Costume Jormungandr Church Cape','Armor',0,true,true,true,1,5039);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230045,'floating_ball','Costume Floating Taegeuk Beads','Armor',0,true,1,5040);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230046,'C_Hemel_Sword','Costume Himmelmez Sword','Armor',0,true,1,5041);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`flag_buyingstore`) VALUES (6471,'Goast_Chill','Chills Of Death','Etc',1600,10,true);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`flag_buyingstore`) VALUES (25130,'Particles_Of_Energy4','Sinister Energy Particle','Etc',true);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`flag_buyingstore`) VALUES (6470,'Blood_Thirst','Blood Thirst','Etc',1200,10,true);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`flag_buyingstore`,`trade_nodrop`) VALUES (6635,'Blacksmith_Blessing','Blacksmith Blessing','Etc',20,true,true);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230047,'C_99LV_ShadowC_Black','Costume Aura of Night','Armor',0,true,1,5042);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_buy`,`weight`,`trade_nodrop`,`trade_notrade`,`trade_nosell`,`trade_nocart`,`trade_noguildstorage`,`trade_nomail`,`trade_noauction`,`script`) VALUES (1230048,'Comp_Bubble_Gum_24h','Bubble Gum 24h','Usable',2,10,true,true,true,true,true,true,true,'sc_start SC_ITEMBOOST,86400000,100;');
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230049,'C_Helm_Of_Darkness','Costume Helm Of Darkness','Armor',0,true,true,1,5043);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230050,'C_Spinning_Skulls','Costume HBook of Skulls','Armor',0,true,1,5044);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_top`,`armor_level`,`view`) VALUES (1230051,'nydhog_wig_wh','Costume White Nidhogg Wig','Armor',0,true,1,5045);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_garment`,`armor_level`,`view`) VALUES (1230052,'C_Valkyrie_Spear','Costume Valkyrie Spear','Armor',0,true,1,5046);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Low`,`armor_level`,`view`) VALUES (1230053,'Sleep_Sheep','Costume Sleep Sheep','Armor',0,true,1,5047);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230054,'C_Midnight_Candle','Costume Midnight Candle','Armor',0,true,1,5048);
REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`weight`,`location_costume_head_Mid`,`armor_level`,`view`) VALUES (1230055,'Snow_Fox','Costume Snow Fox','Armor',0,true,1,5049);
