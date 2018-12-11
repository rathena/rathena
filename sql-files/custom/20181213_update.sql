# 12370#Girl's Naivety drops on incubus
REPLACE INTO `mob_db2_re` (`ID`, `Sprite`, `kName`, `iName`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `aDelay`, `aMotion`, `dMotion`, `MEXP`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7id`, `Drop7per`, `Drop8id`, `Drop8per`, `Drop9id`, `Drop9per`, `DropCardid`, `DropCardper`) VALUES
(1374, 'INCUBUS', 'Incubus', 'Incubus', 120, 28000, 1, 3928, 3646, 2, 1256, 375, 72, 46, 120, 56, 52, 75, 99, 70, 10, 12, 1, 6, 67, 33568405, 165, 850, 600, 336, 0, 0, 0, 0, 0, 0, 0, 522, 1500, 509, 5500, 5072, 1, 2621, 1, 2610, 500, 2613, 150, 509, 2200, 12370, 1, 0, 0, 4269, 1);

# Reduce cooldown for daily quest
REPLACE INTO `mission_board` (`id`, `title`, `desc`, `mob_list`, `mob_qty`, `item_list`, `item_qty`, `class_limitation`, `class_branch`, `min_lv`, `max_lv`, `repeat`, `duration`, `reward_list`, `reward_qty`, `base_exp`, `job_exp`, `zeny`, `cash`, `aid`, `name`, `time_update`, `npc_id`, `redo_delay`) VALUES
(1542786300, 'Honor Token - Jewel and Cloth', 'I need materials to make my dress. I will trade materials for Honor Token - Agnes Roegenburg', '', '', '|722|949|', '|3|50|', 1023, 31, 1, 175, 0, 0, '|6919|', '|5|', 75000, 75000, 30000, 0, 2000000, '<GM>Nubs', '2018-11-28 01:14:09', '|0|', 21),
(1542786310, 'Honor Token - Pink Petal-like Dress', 'I need materials to make my dress. I will trade materials for Honor Token - Agnes Roegenburg', '', '', '|7166|507|509|', '|15|30|10|', 1023, 31, 1, 175, 0, 0, '|6919|', '|5|', 75000, 75000, 30000, 0, 2000000, '<GM>Nubs', '2018-11-28 01:14:09', '|0|', 21),
(1542786320, 'Honor Token - Beautiful Flower Decoration', 'I need materials to make my dress. I will trade materials for Honor Token - Agnes Roegenburg', '', '', '|6511|6509|6510|', '|5|5|5|', 1023, 31, 1, 175, 0, 0, '|6919|', '|5|', 75000, 75000, 30000, 0, 2000000, '<GM>Nubs', '2018-11-28 01:14:09', '|0|', 21),
(1542786330, 'Honor Token - Dress of the Night Sky', 'I need materials to make my dress. I will trade materials for Honor Token - Agnes Roegenburg', '', '', '|7205|', '|20|', 1023, 31, 1, 175, 0, 0, '|6919|', '|5|', 75000, 75000, 30000, 0, 2000000, '<GM>Nubs', '2018-11-28 01:14:09', '|0|', 21),
(1542786340, 'Honor Token - Shawl of the Blazing Sun', 'I need materials to make my dress. I will trade materials for Honor Token - Agnes Roegenburg', '', '', '|7122|', '|20|', 1023, 31, 1, 175, 0, 0, '|6919|', '|5|', 75000, 75000, 30000, 0, 2000000, '<GM>Nubs', '2018-11-28 01:14:09', '|0|', 21),
(1542786350, 'Honor Token - Step of the Fairy', 'I need materials to make my dress. I will trade materials for Honor Token - Agnes Roegenburg', '', '', '|6557|', '|5|', 1023, 31, 1, 175, 0, 0, '|6919|', '|5|', 75000, 75000, 30000, 0, 2000000, '<GM>Nubs', '2018-11-28 01:14:09', '|0|', 21),
(1542786360, 'Honor Token - Sparkling Star', 'I need materials to make my dress. I will trade materials for Honor Token - Agnes Roegenburg', '', '', '|1001|', '|5|', 1023, 31, 1, 175, 0, 0, '|6919|', '|5|', 75000, 75000, 30000, 0, 2000000, '<GM>Nubs', '2018-11-28 01:14:09', '|0|', 21);

# increase weapon drop rate for mechaspider from 1% to 10%
REPLACE INTO `mob_db2_re` (`ID`, `Sprite`, `kName`, `iName`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `aDelay`, `aMotion`, `dMotion`, `MEXP`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7id`, `Drop7per`, `Drop8id`, `Drop8per`, `Drop9id`, `Drop9per`, `DropCardid`, `DropCardper`) VALUES
(3741, 'MECHASPIDER', 'Mechaspider', 'Spider Chariot', 158, 9799123, 1, 3150895, 2112795, 1, 7657, 3062, 394, 123, 116, 123, 154, 99, 217, 98, 10, 12, 2, 0, 40, 103299205, 250, 768, 0, 0, 2092500, 607, 250, 732, 250, 616, 250, 7317, 100, 999, 100, 7094, 100, 985, 100, 984, 50, 13192, 50, 13122, 50, 13198, 50, 13138, 50, 27180, 1);

# SkyFortress Items
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(14505, 'Dungeon_1_hour_Ticket', 'Dungeon 1 Hour Ticket', 2, 20, NULL, 0, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'warp \"dali02\",117,69;', NULL, NULL),
(14506, 'Dungeon_1_hour_Ticket_', 'Dungeon 1 Hour Ticket', 2, 20, NULL, 0, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'select(\"Invasion of Air Stronghold - Lv. 145 or higher\"); warp \"dali02\",117,69;', NULL, NULL),
(17569, 'Dungeon_1_hour_Ticket_Box', 'Sky Fortress Ticket 1 Hour Box', 2, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getitem 14505,1;', NULL, NULL),
(31024, 'Immortal_Cursed_Knight_Card', 'Immortal Cursed Knight Card', 6, 20, NULL, 10, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 2, NULL, NULL, NULL, NULL, 'bonus2 bAddClass,Class_All,10; bonus5 bAutoSpell,\"RK_IGNITIONBREAK\",5,20,BF_WEAPON,1;', NULL, NULL),
(31025, 'Immortal_Wind_Ghost_Card', 'Immortal Wind Ghost Card', 6, 20, NULL, 10, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 2, NULL, NULL, NULL, NULL, 'bonus bMatkRate,10; bonus5 bAutoSpell,\"SO_CLOUD_KILL\",5,10,BF_MAGIC,1;', NULL, NULL),
(31026, 'Stephen_Jack_Ernest_Wolf_Card', 'Stephen Jack Ernest Wolf Card', 6, 20, NULL, 10, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 64, NULL, NULL, NULL, NULL, 'bonus5 bAutoSpellWhenHit,\"SO_FIREWALK\",5,35,BF_WEAPON,0; bonus5 bAutoSpellWhenHit,\"SO_ELECTRICWALK\",5,80,BF_MAGIC,0; autobonus \"{ bonus bFlee,200; }\",30,10000,BF_WEAPON; autobonus \"{ bonus bSpeedRate,25; }\",25,10000,BF_MAGIC; /* unknown rate and specialeffect */', NULL, NULL);

# SkyFortress Mobs
REPLACE INTO `mob_db2_re` (`ID`, `Sprite`, `kName`, `iName`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `aDelay`, `aMotion`, `dMotion`, `MEXP`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7id`, `Drop7per`, `Drop8id`, `Drop8per`, `Drop9id`, `Drop9per`, `DropCardid`, `DropCardper`) VALUES
(3473, 'AS_D_RAGGED_GOLEM', 'Stefan J.E Wolf', 'Stefan J.E Wolf', 160, 20000000, 1, 2800090, 2675920, 3, 4500, 6500, 100, 68, 200, 100, 200, 200, 220, 100, 10, 12, 2, 1, 89, 103298709, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 984, 250, 985, 250, 607, 250, 608, 500, 2027, 75, 21018, 75, 28010, 75, 0, 0, 0, 0, 31026, 1),
(3474, 'AS_BLOODY_KNIGHT', 'Immortal Cursed Knight', 'Immortal Cursed Knight', 160, 7000000, 1, 7513, 8457, 2, 4785, 4977, 138, 53, 102, 104, 72, 57, 98, 57, 10, 12, 1, 0, 87, 102774421, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31024, 1),
(3475, 'AS_WIND_GHOST', 'Immortal Wind Ghost', 'Immortal Wind Ghost', 160, 7000000, 1, 9086, 17532, 5, 2513, 3315, 105, 41, 100, 63, 35, 99, 106, 61, 10, 12, 1, 6, 64, 102774421, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31025, 1),
(3476, 'AS_ZOMBIE', 'Immortal Zombie Soldier', 'Immortal Zombie Soldier', 160, 405694, 1, 10570, 10100, 1, 1504, 1625, 128, 84, 5, 99, 39, 48, 30, 24, 10, 12, 1, 1, 29, 12437, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3477, 'AS_IMMORTAL_CORPS', 'Immortal Fortress Legion', 'Immortal Fortress Legion', 160, 405694, 1, 11040, 10730, 1, 1851, 2022, 103, 25, 109, 76, 61, 53, 98, 30, 10, 12, 1, 6, 89, 12437, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3478, 'AS_ZOMBIE_SLAUGHTER', 'Immortal Fortress Key Keeper', 'Immortal Fortress Key Keeper', 160, 423332, 1, 11040, 10730, 1, 1851, 2022, 103, 25, 109, 76, 61, 53, 98, 30, 10, 12, 1, 1, 69, 33566869, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3479, 'AS_ZOMBIE_MASTER', 'Immortal Zombie Assaulter', 'Immortal Zombie Assaulter', 160, 405694, 1, 11500, 10100, 1, 1653, 3274, 140, 44, 155, 126, 89, 108, 213, 76, 10, 12, 1, 1, 29, 12437, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3480, 'AS_CURSED_SOLDIER', 'Immortal Cursed Zombie', 'Immortal Cursed Zombie', 160, 405694, 1, 10101, 10003, 1, 1850, 3451, 81, 87, 119, 87, 86, 60, 94, 55, 10, 12, 1, 1, 69, 12437, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3481, 'AS_EVIL_SHADOW1', 'Immortal Nightmare Shadow', 'Immortal Nightmare Shadow', 160, 423330, 1, 6666, 7332, 2, 1444, 1944, 106, 44, 44, 166, 44, 44, 166, 44, 10, 12, 1, 6, 47, 12437, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3482, 'AS_EVIL_SHADOW2', 'Immortal Angry Shadow', 'Immortal Angry Shadow', 160, 388054, 1, 279836, 260199, 1, 2730, 3990, 80, 180, 155, 88, 110, 135, 154, 59, 10, 12, 1, 6, 47, 12437, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3483, 'AS_EVIL_SHADOW3', 'Immortal Death Shadow', 'Immortal Death Shadow', 160, 423330, 1, 280009, 267592, 1, 3020, 4265, 80, 150, 165, 82, 110, 122, 154, 52, 10, 12, 1, 6, 47, 12437, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3484, 'AS_D_RAGGED_GOLEM', 'Stefan J.E Wolf', 'Stefan J.E Wolf', 160, 5000000, 1, 0, 0, 3, 2800, 3200, 100, 68, 100, 50, 100, 100, 110, 50, 10, 12, 2, 1, 89, 103298709, 500, 1500, 500, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3485, 'AS_D_CURSED_SOLDIER', 'Cursed Soldier of Bijou', 'Cursed Soldier of Bijou', 1, 100000, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 10, 12, 1, 3, 21, 12437, 400, 1872, 672, 480, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

# SkyFortress Mobs Skills
REPLACE INTO mob_skill_db2_re (`mob_id`, `info`, `state`, `skill_id`, `skill_lv`, `rate`, `casttime`, `delay`, `cancelable`, `target`, `condition`, `val1`, `val2`, `val3`, `val4`, `val5`, `emotion`, `chat`) VALUES
(3476,"ImmortalZombie@NPC_POISON","attack",176,1,500,800,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3476,"ImmortalZombie@NPC_POISON","angry",176,1,500,800,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3476,"ImmortalZombie@NPC_UNDEADATTACK","attack",347,1,2000,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3476,"ImmortalZombie@NPC_UNDEADATTACK","angry",347,1,2000,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3477,"ImmortalLegion@AS_SONICBLOW","attack",136,10,2000,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3477,"ImmortalLegion@KN_BRANDISHSPEAR","attack",57,10,1000,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3477,"ImmortalLegion@NPC_ARMORBRAKE","attack",344,10,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3478,"ImmortalKeyKeeper@NPC_BLINDATTACK","attack",177,5,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3478,"ImmortalKeyKeeper@NPC_POISON","attack",176,5,500,800,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3478,"ImmortalKeyKeeper@AS_SONICBLOW","attack",136,5,1000,800,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3478,"ImmortalKeyKeeper@NPC_COMBOATTACK","attack",171,1,500,700,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3479,"ImmortalMaster@KN_BRANDISHSPEAR","attack",57,5,500,1000,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3479,"ImmortalMaster@NPC_POISON","attack",176,3,500,800,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3479,"ImmortalMaster@NPC_SUMMONSLAVE","idle",196,2,10000,2000,60000,"no","self","slavele",0,3476,NULL,NULL,NULL,NULL,NULL,NULL),
(3480,"ImmortalCursed@NPC_BLINDATTACK","attack",177,5,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3480,"ImmortalCursed@AC_DOUBLE","attack",46,5,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3480,"ImmortalCursed@NPC_MAGICALATTACK","attack",192,1,500,800,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3480,"ImmortalCursed@AC_SHOWER","attack",47,3,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3481,"ImmortalShadow@NPC_GUIDEDATTACK","attack",172,2,500,1000,20000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3481,"ImmortalShadow@WZ_METEOR","attack",83,5,500,1500,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3481,"ImmortalShadow@WZ_METEOR","chase",83,5,500,1500,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3482,"ImmortalShadow2@AS_SONICBLOW","attack",136,10,500,800,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3482,"ImmortalShadow2@MO_BODYRELOCATION","chase",264,1,2000,500,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3482,"ImmortalShadow2@NPC_WINDATTACK","attack",187,3,500,500,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,6,NULL),
(3482,"ImmortalShadow2@TF_BACKSLIDING","attack",150,1,10000,500,600000,"no","self","myhpltmaxrate",10,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3483,"ImmortalShadow3@NPC_BLOODDRAIN","attack",199,1,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,2,NULL),
(3483,"ImmortalShadow3@CR_AUTOGUARD","chase",249,2,2000,0,300000,"yes","self","longrangeattacked",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3483,"ImmortalShadow3@CR_AUTOGUARD","attack",249,2,500,0,300000,"yes","self","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3483,"ImmortalShadow3@NPC_CRITICALSLASH","attack",170,1,500,500,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,6,NULL),
(3483,"ImmortalShadow3@NPC_BLOODDRAIN","angry",199,1,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3483,"ImmortalShadow3@NPC_BLOODDRAIN","attack",199,1,500,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3473,"Stefan@NPC_POWERUP","attack",349,5,10000,0,120000,"yes","self","myhpltmaxrate",30,NULL,NULL,NULL,NULL,NULL,6,NULL),
(3473,"Stefan@NPC_DRAGONFEAR","chase",659,5,10000,1000,10000,"no","self","myhpltmaxrate",80,NULL,NULL,NULL,NULL,NULL,32,NULL),
(3473,"Stefan@NPC_DRAGONFEAR","attack",659,5,10000,1000,10000,"no","self","myhpltmaxrate",80,NULL,NULL,NULL,NULL,NULL,32,NULL),
(3473,"Stefan@NPC_GUIDEDATTACK","chase",172,5,500,0,20000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3473,"Stefan@AL_HEAL","idle",28,11,10000,0,5000,"yes","self","mystatuson","hiding",NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3473,"Stefan@NPC_CRITICALSLASH","attack",170,1,2000,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3473,"Stefan@NPC_CRITICALSLASH","chase",170,1,2000,0,5000,"yes","target","always",0,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3473,"Stefan@WZ_METEOR","attack",83,10,2000,500,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,21,NULL),
(3473,"Stefan@WZ_METEOR","chase",83,10,2000,500,5000,"no","target","always",0,NULL,NULL,NULL,NULL,NULL,21,NULL),
(3473,"Stefan@WZ_METEOR","chase",83,10,10000,500,5000,"no","target","myhpltmaxrate",50,NULL,NULL,NULL,NULL,NULL,32,NULL);