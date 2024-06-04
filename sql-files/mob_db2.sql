#
# Table structure for table `mob_db2`
#

DROP TABLE IF EXISTS `mob_db2`;
CREATE TABLE `mob_db2` (
  `id` int(11) unsigned NOT NULL,
  `name_aegis` varchar(24) DEFAULT NULL,
  `name_english` text DEFAULT NULL,
  `name_japanese` text DEFAULT NULL,
  `level` smallint(6) unsigned DEFAULT NULL,
  `hp` int(11) unsigned DEFAULT NULL,
  `sp` mediumint(9) unsigned DEFAULT NULL,
  `base_exp` int(11) unsigned DEFAULT NULL,
  `job_exp` int(11) unsigned DEFAULT NULL,
  `mvp_exp` int(11) unsigned DEFAULT NULL,
  `attack` smallint(6) unsigned DEFAULT NULL,
  `attack2` smallint(6) unsigned DEFAULT NULL,
  `defense` smallint(6) unsigned DEFAULT NULL,
  `magic_defense` smallint(6) unsigned DEFAULT NULL,
  `str` smallint(6) unsigned DEFAULT NULL,
  `agi` smallint(6) unsigned DEFAULT NULL,
  `vit` smallint(6) unsigned DEFAULT NULL,
  `int` smallint(6) unsigned DEFAULT NULL,
  `dex` smallint(6) unsigned DEFAULT NULL,
  `luk` smallint(6) unsigned DEFAULT NULL,
  `attack_range` tinyint(4) unsigned DEFAULT NULL,
  `skill_range` tinyint(4) unsigned DEFAULT NULL,
  `chase_range` tinyint(4) unsigned DEFAULT NULL,
  `size` varchar(24) DEFAULT NULL,
  `race` varchar(24) DEFAULT NULL,
  `racegroup_goblin` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_kobold` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_orc` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_golem` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_guardian` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_ninja` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_gvg` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_battlefield` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_treasure` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_biolab` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_manuk` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_splendide` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_scaraba` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_ogh_atk_def` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_ogh_hidden` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_bio5_swordman_thief` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_bio5_acolyte_merchant` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_bio5_mage_archer` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_bio5_mvp` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_clocktower` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_thanatos` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_faceworm` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_hearthunter` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_rockridge` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_werner_lab` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_temple_demon` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_illusion_vampire` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_malangdo` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_ep172alpha` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_ep172beta` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_ep172bath` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_illusion_turtle` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_rachel_sanctuary` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_illusion_luanda` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_illusion_frozen` tinyint(1) unsigned DEFAULT NULL,
  `racegroup_illusion_moonlight` tinyint(1) unsigned DEFAULT NULL,
  `element` varchar(24) DEFAULT NULL,
  `element_level` tinyint(4) unsigned DEFAULT NULL,
  `walk_speed` smallint(6) unsigned DEFAULT NULL,
  `attack_delay` smallint(6) unsigned DEFAULT NULL,
  `attack_motion` smallint(6) unsigned DEFAULT NULL,
  `damage_motion` smallint(6) unsigned DEFAULT NULL,
  `damage_taken` smallint(6) unsigned DEFAULT NULL,
  `ai` varchar(2) DEFAULT NULL,
  `class` varchar(50) DEFAULT NULL,
  `mode_canmove` tinyint(1) unsigned DEFAULT NULL,
  `mode_looter` tinyint(1) unsigned DEFAULT NULL,
  `mode_aggressive` tinyint(1) unsigned DEFAULT NULL,
  `mode_assist` tinyint(1) unsigned DEFAULT NULL,
  `mode_castsensoridle` tinyint(1) unsigned DEFAULT NULL,
  `mode_norandomwalk` tinyint(1) unsigned DEFAULT NULL,
  `mode_nocast` tinyint(1) unsigned DEFAULT NULL,
  `mode_canattack` tinyint(1) unsigned DEFAULT NULL,
  `mode_castsensorchase` tinyint(1) unsigned DEFAULT NULL,
  `mode_changechase` tinyint(1) unsigned DEFAULT NULL,
  `mode_angry` tinyint(1) unsigned DEFAULT NULL,
  `mode_changetargetmelee` tinyint(1) unsigned DEFAULT NULL,
  `mode_changetargetchase` tinyint(1) unsigned DEFAULT NULL,
  `mode_targetweak` tinyint(1) unsigned DEFAULT NULL,
  `mode_randomtarget` tinyint(1) unsigned DEFAULT NULL,
  `mode_ignoremelee` tinyint(1) unsigned DEFAULT NULL,
  `mode_ignoremagic` tinyint(1) unsigned DEFAULT NULL,
  `mode_ignoreranged` tinyint(1) unsigned DEFAULT NULL,
  `mode_mvp` tinyint(1) unsigned DEFAULT NULL,
  `mode_ignoremisc` tinyint(1) unsigned DEFAULT NULL,
  `mode_knockbackimmune` tinyint(1) unsigned DEFAULT NULL,
  `mode_teleportblock` tinyint(1) unsigned DEFAULT NULL,
  `mode_fixeditemdrop` tinyint(1) unsigned DEFAULT NULL,
  `mode_detector` tinyint(1) unsigned DEFAULT NULL,
  `mode_statusimmune` tinyint(1) unsigned DEFAULT NULL,
  `mode_skillimmune` tinyint(1) unsigned DEFAULT NULL,
  `mvpdrop1_item` varchar(50) DEFAULT NULL,
  `mvpdrop1_rate` smallint(9) unsigned DEFAULT NULL,
  `mvpdrop1_option` varchar(50) DEFAULT NULL,
  `mvpdrop1_index` tinyint(2) unsigned DEFAULT NULL,
  `mvpdrop2_item` varchar(50) DEFAULT NULL,
  `mvpdrop2_rate` smallint(9) unsigned DEFAULT NULL,
  `mvpdrop2_option` varchar(50) DEFAULT NULL,
  `mvpdrop2_index` tinyint(2) unsigned DEFAULT NULL,
  `mvpdrop3_item` varchar(50) DEFAULT NULL,
  `mvpdrop3_rate` smallint(9) unsigned DEFAULT NULL,
  `mvpdrop3_option` varchar(50) DEFAULT NULL,
  `mvpdrop3_index` tinyint(2) unsigned DEFAULT NULL,
  `drop1_item` varchar(50) DEFAULT NULL,
  `drop1_rate` smallint(9) unsigned DEFAULT NULL,
  `drop1_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop1_option` varchar(50) DEFAULT NULL,
  `drop1_index` tinyint(2) unsigned DEFAULT NULL,
  `drop2_item` varchar(50) DEFAULT NULL,
  `drop2_rate` smallint(9) unsigned DEFAULT NULL,
  `drop2_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop2_option` varchar(50) DEFAULT NULL,
  `drop2_index` tinyint(2) unsigned DEFAULT NULL,
  `drop3_item` varchar(50) DEFAULT NULL,
  `drop3_rate` smallint(9) unsigned DEFAULT NULL,
  `drop3_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop3_option` varchar(50) DEFAULT NULL,
  `drop3_index` tinyint(2) unsigned DEFAULT NULL,
  `drop4_item` varchar(50) DEFAULT NULL,
  `drop4_rate` smallint(9) unsigned DEFAULT NULL,
  `drop4_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop4_option` varchar(50) DEFAULT NULL,
  `drop4_index` tinyint(2) unsigned DEFAULT NULL,
  `drop5_item` varchar(50) DEFAULT NULL,
  `drop5_rate` smallint(9) unsigned DEFAULT NULL,
  `drop5_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop5_option` varchar(50) DEFAULT NULL,
  `drop5_index` tinyint(2) unsigned DEFAULT NULL,
  `drop6_item` varchar(50) DEFAULT NULL,
  `drop6_rate` smallint(9) unsigned DEFAULT NULL,
  `drop6_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop6_option` varchar(50) DEFAULT NULL,
  `drop6_index` tinyint(2) unsigned DEFAULT NULL,
  `drop7_item` varchar(50) DEFAULT NULL,
  `drop7_rate` smallint(9) unsigned DEFAULT NULL,
  `drop7_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop7_option` varchar(50) DEFAULT NULL,
  `drop7_index` tinyint(2) unsigned DEFAULT NULL,
  `drop8_item` varchar(50) DEFAULT NULL,
  `drop8_rate` smallint(9) unsigned DEFAULT NULL,
  `drop8_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop8_option` varchar(50) DEFAULT NULL,
  `drop8_index` tinyint(2) unsigned DEFAULT NULL,
  `drop9_item` varchar(50) DEFAULT NULL,
  `drop9_rate` smallint(9) unsigned DEFAULT NULL,
  `drop9_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop9_option` varchar(50) DEFAULT NULL,
  `drop9_index` tinyint(2) unsigned DEFAULT NULL,
  `drop10_item` varchar(50) DEFAULT NULL,
  `drop10_rate` smallint(9) unsigned DEFAULT NULL,
  `drop10_nosteal` tinyint(1) unsigned DEFAULT NULL,
  `drop10_option` varchar(50) DEFAULT NULL,
  `drop10_index` tinyint(2) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY (`name_aegis`)
) ENGINE=MyISAM;

# rAthena Dev Team
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`mvp_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`mode_looter`,`mode_castsensoridle`,`mode_norandomwalk`,`mode_nocast`,`mode_angry`,`mode_changetargetmelee`,`mvpdrop1_item`,`mvpdrop1_rate`,`mvpdrop2_item`,`mvpdrop2_rate`,`mvpdrop3_item`,`mvpdrop3_rate`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1900,'VALARIS','Valaris','Valaris',99,668000,107250,37895,13000,3220,4040,35,45,152,96,85,120,95,2,10,10,'Large','Demon','Dark',3,100,1068,768,576,'25',true,true,true,true,true,true,'Seed_Of_Yggdrasil',1000,'Baphomet_Doll',400,'Evil_Horn',3800,'Crescent_Scythe',200,'Magestic_Goat',200,'Clip',800,'Emperium',500,'Old_Violet_Box',3000,'Oridecon',4300,'Elunium',5600,'Baphomet_Card',1,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`mode_canmove`,`mode_castsensorchase`,`mode_changechase`,`mode_changetargetmelee`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1901,'VALARIS_WORSHIPPER','Valaris\'s Worshipper','Valaris\'s Worshipper',50,8578,2706,1480,487,590,15,25,75,55,93,45,1,10,12,'Small','Demon','Dark',1,100,868,480,120,'10',true,true,true,true,'Evil_Horn',500,'Oridecon',63,'Halberd_',2,'Yggdrasilberry',50,'Leaf_Of_Yggdrasil',100,'Yellow_Potion',300,'Boots',50,'Baphomet__Card',1,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`mvp_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`mode_looter`,`mode_castsensoridle`,`mode_norandomwalk`,`mode_nocast`,`mode_angry`,`mode_changetargetmelee`,`mvpdrop1_item`,`mvpdrop1_rate`,`mvpdrop2_item`,`mvpdrop2_rate`,`mvpdrop3_item`,`mvpdrop3_rate`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1902,'MC_CAMERI','MC Cameri','MC Cameri',99,668000,107250,37895,13000,3220,4040,35,45,152,96,85,120,95,2,10,10,'Large','Demon','Dark',3,100,1068,768,576,'25',true,true,true,true,true,true,'Seed_Of_Yggdrasil',1000,'Baphomet_Doll',400,'Evil_Horn',3800,'Crescent_Scythe',200,'Magestic_Goat',200,'Clip',800,'Emperium',500,'Old_Violet_Box',3000,'Oridecon',4300,'Elunium',5600,'Baphomet_Card',1,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`mvp_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`mode_looter`,`mode_castsensoridle`,`mode_norandomwalk`,`mode_nocast`,`mode_angry`,`mode_changetargetmelee`,`mvpdrop1_item`,`mvpdrop1_rate`,`mvpdrop2_item`,`mvpdrop2_rate`,`mvpdrop3_item`,`mvpdrop3_rate`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1903,'POKI','Poki#3','Poki#3',99,1349000,4093000,1526000,92100,4892,9113,22,35,180,39,67,193,130,9,10,12,'Medium','Demihuman','Wind',3,120,500,672,480,'25',true,true,true,true,true,true,'Old_Blue_Box',5500,'Old_Violet_Box',3000,'Luna_Bow',1000,'Combat_Knife',100,'Sucsamad',500,'Old_Violet_Box',2500,'Moonlight_Sword',75,'Grimtooth_',125,'Balistar',250,'Dragon_Wing',100,'Bow_Of_Rudra',50,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`mvp_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`mode_looter`,`mode_castsensoridle`,`mode_norandomwalk`,`mode_nocast`,`mode_angry`,`mode_changetargetmelee`,`mvpdrop1_item`,`mvpdrop1_rate`,`mvpdrop2_item`,`mvpdrop2_rate`,`mvpdrop3_item`,`mvpdrop3_rate`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1904,'SENTRY','Sentry','Sentry',99,668000,107250,37895,13000,3220,4040,35,45,152,96,85,120,95,2,10,10,'Large','Demon','Dark',3,100,1068,768,576,'25',true,true,true,true,true,true,'Seed_Of_Yggdrasil',1000,'Baphomet_Doll',400,'Evil_Horn',3800,'Crescent_Scythe',200,'Magestic_Goat',200,'Clip',800,'Emperium',500,'Old_Violet_Box',3000,'Oridecon',4300,'Elunium',5600,'Baphomet_Card',1,true);

# Custom Hollow Poring (overrrides/collides with META_ANDRE)
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`magic_defense`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1237,'HOLLOW_PORING','Hollow Poring','Hollow Poring',1,50,2,1,7,10,5,6,30,1,10,12,'Medium','Plant','Water',1,400,1872,672,480,'02','Jellopy',7000,'Knife_',100,'Sticky_Mucus',400,'Apple',1000,'Empty_Bottle',1500,'Apple',150,'Unripe_Apple',20,'Poring_Card',10,true);
# Custom Fire Poring. Warning, Colides with META_DENIRO
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`magic_defense`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`mode_castsensoridle`,`mode_norandomwalk`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1239,'FIRE_PORING','Fire Poring','Fire Poring',1,50,2,1,7,10,5,6,30,1,10,12,'Medium','Plant','Water',1,400,1872,672,480,'25',true,true,'Jellopy',7000,'Knife_',100,'Sticky_Mucus',400,'Apple',1000,'Empty_Bottle',1500,'Poring_Doll',5,'Unripe_Apple',20,'Poring_Card',20,true);

# Lunar New Year 2008 Event Monster overrides
# Uncomment if event is enabled, as these drops modifications are nessecary.
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`magic_defense`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`,`drop9_item`,`drop9_rate`,`drop9_nosteal`) VALUES (1145,'MARTIN','Martin','Martin',18,1109,134,86,52,63,5,12,18,30,15,15,5,1,10,12,'Small','Brute','Earth',2,300,1480,480,480,'01','Moustache_Of_Mole',9000,'Nail_Of_Mole',500,'Jur_',10,'Goggle_',5,'Safety_Helmet',1,'Battered_Pot',10,'Goggle',15,'RicePouch',1500,true,'Martin_Card',1,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop7_nosteal`) VALUES (1175,'TAROU','Tarou','Tarou',11,284,57,28,34,45,20,11,10,24,5,1,10,12,'Small','Brute','Dark',1,150,1744,1044,684,'17','Rat_Tail',9000,'Animal\'s_Skin',3000,'Feather',800,'Monster\'s_Feed',1000,'Ora_Ora',2,'RicePouch',2500,'Tarou_Card',1,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`,`drop9_item`,`drop9_rate`,`drop9_nosteal`) VALUES (1209,'CRAMP','Cramp','Cramp',56,4720,2300,1513,395,465,5,85,35,5,65,60,1,10,12,'Small','Brute','Poison',2,100,1000,500,1000,'09','Claw_Of_Rat',4656,'Monster\'s_Feed',1000,'Blue_Jewel',80,'Glass_Bead',110,'Lemon',250,'Blue_Herb',70,'Oridecon',95,'RicePouch',1500,true,'Cramp_Card',1,true);

# iRO St. Patricks Day 2008 Event Monster overrides
# Uncomment if event is enabled, as these drops modifications are nessecary.
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`) VALUES (1841,'G_SNAKE_','Snake Lord\'s Minon','Snake Lord\'s Minon',15,471,72,48,46,55,15,15,10,35,5,1,10,12,'Medium','Brute','Earth',1,200,1576,576,576,'01','Copper_Coin_',1000,'Silver_Coin_',100,'Gold_Coin_US',30,'Black_Treasure_Box',7);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`) VALUES (1842,'G_ANACONDAQ_','Snake Lord\'s Minon','Snake Lord\'s Minon',23,1109,300,149,124,157,23,28,10,36,5,1,10,12,'Medium','Brute','Poison',1,200,1576,576,576,'17','Copper_Coin_',1000,'Silver_Coin_',100,'Gold_Coin_US',30,'Black_Treasure_Box',7);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`) VALUES (1843,'SIDE_WINDER_','Snake Lord\'s Minon','Snake Lord\'s Minon',43,4929,1996,993,240,320,5,10,38,43,40,15,115,20,1,10,12,'Medium','Brute','Poison',1,200,1576,576,576,'09','Copper_Coin_',1000,'Silver_Coin_',100,'Gold_Coin_US',30,'Black_Treasure_Box',7);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`) VALUES (1844,'G_ISIS_','Snake Lord\'s Minon','Snake Lord\'s Minon',47,7003,3709,1550,423,507,10,35,38,65,43,50,66,15,1,10,12,'Large','Demon','Dark',1,200,1384,768,336,'09','Copper_Coin_',1000,'Silver_Coin_',100,'Gold_Coin_US',30,'Black_Treasure_Box',8);

# iRO Christmas 2008 Event
# Uncomment if event is enabled, as these drops modifications are nessecary.
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1244,'JAKK_XMAS','Christmas Jakk','Christmas Jakk',38,3581,1113,688,315,382,5,30,38,38,43,75,45,1,10,12,'Medium','Formless','Fire',2,200,1180,480,648,'01','Candy',1000,'Candy_Striper',1000,'Fire_Cracker_Love',1000,'Fire_Cracker_Xmas',1000,'Packing_Ribbon',1000,'Packing_Paper',1000,'Singing_Crystal_Piece',1000,'Xmas_Gift',1250,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1245,'GOBLINE_XMAS','Christmas Goblin','Christmas Goblin',25,1176,282,171,118,140,10,5,53,25,20,38,45,1,10,12,'Medium','Demihuman','Wind',1,100,1120,620,240,'01','Candy',1000,'Candy_Striper',1000,'Fire_Cracker_Love',1000,'Fire_Cracker_Xmas',1000,'Packing_Ribbon',1000,'Packing_Paper',1000,'Singing_Crystal_Piece',1000,'Xmas_Gift',1250,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`magic_defense`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1246,'COOKIE_XMAS','Christmas Cookie','Christmas Cookie',28,2090,461,284,140,170,50,24,30,53,45,100,1,10,12,'Small','Demihuman','Holy',2,400,1248,1248,240,'17','Candy',1000,'Candy_Striper',1000,'Fire_Cracker_Love',1000,'Fire_Cracker_Xmas',1000,'Packing_Ribbon',1000,'Packing_Paper',1000,'Singing_Crystal_Piece',1000,'Xmas_Gift',1250,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`mode_nocast`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (1247,'ANTONIO','Antonio','Antonio',10,10,3,2,13,20,100,50,100,100,1,10,12,'Medium','Plant','Holy',3,100,720,720,432,'01',true,'Branch_Of_Dead_Tree',500,'Buche_De_Noel',500,'Fire_Cracker_Xmas',500,'Santa\'s_Hat_',500,'Red_Bag',500,'Sweet_Candy_Striper',500,'Santa_Beard',500,'Antonio_Card',1,true);

# iRO Halloween 2009 Event
# Uncomment if event is enabled. Uncomment the skills for Halloween Whisper in mob_skill_db2.
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`attack`,`attack2`,`magic_defense`,`agi`,`vit`,`dex`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`) VALUES (3014,'HALLOWEEN_WHISPER','Halloween Whisper','Halloween Whisper',1,800,10,13,45,51,14,60,1,10,12,'Small','Demon','Ghost',3,150,1960,960,504,'01','Fools_Day_Box',150,'Worn_Cloth_Piece',5335);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`attack`,`attack2`,`magic_defense`,`agi`,`vit`,`dex`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`) VALUES (3015,'HALLOWEEN_DARK_LORD','Halloween Dark Lord','Halloween Dark Lord',1,45,10,13,45,51,14,60,1,10,12,'Large','Demon','Undead',4,100,868,768,480,'01','Fools_Day_Box',800,'Fools_Day_Box2',5335);

# iRO Halloween 2008 Event
# Uncomment if event is enabled.
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`magic_defense`,`agi`,`vit`,`dex`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop6_nosteal`) VALUES (3000,'ZOMBIE','Zombie','Zombie',15,534,50,33,67,79,10,8,7,15,1,10,12,'Medium','Undead','Undead',1,400,2612,912,288,'04','Decayed_Nail',9000,'Cardinal_Jewel_',5,'Sticky_Mucus',1000,'Horrendous_Mouth',50,'White_Jewel',70,'Zombie_Card',1,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`agi`,`vit`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (3001,'GHOUL','Ghoul','Ghoul',40,5418,1088,622,420,500,5,20,20,29,45,20,1,10,12,'Medium','Undead','Undead',2,250,2456,912,504,'04','Horrendous_Mouth',6000,'Oridecon_Stone',110,'White_Herb',700,'Green_Herb',800,'Skul_Ring',60,'Mementos',150,'Ghoul_Leg',1,'Ghoul_Card',1,true);
#REPLACE INTO `mob_db2` (`id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`base_exp`,`job_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`ai`,`drop1_item`,`drop1_rate`,`drop2_item`,`drop2_rate`,`drop3_item`,`drop3_rate`,`drop4_item`,`drop4_rate`,`drop5_item`,`drop5_rate`,`drop6_item`,`drop6_rate`,`drop7_item`,`drop7_rate`,`drop8_item`,`drop8_rate`,`drop8_nosteal`) VALUES (3002,'ZOMBIE_MASTER','Zombie Master','Zombie Master',62,14211,7610,2826,824,1084,37,26,25,20,30,5,77,35,1,10,12,'Medium','Undead','Undead',1,175,2612,912,288,'21','Tatters_Clothes',4413,'Sticky_Mucus',1500,'Horrendous_Mouth',1500,'Cardinal_Jewel',200,'White_Jewel',100,'Ghoul_Leg',1,'Scapulare_',2,'Zombie_Master_Card',1,true);
