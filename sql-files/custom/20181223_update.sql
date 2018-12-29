# add and changes to some items
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(32204, 'Immortal_Dog_Tag', 'Immortal Dog Tag', 4, 10, NULL, 100, NULL, 0, NULL, 1, 4294967295, 63, 2, 136, NULL, '100', 0, 0, 'bonus bAtk,50; bonus bMatk,50; bonus2 bSubRace,Ele_Undead,4;', NULL, NULL),
(28594, 'Temporal_Ring', 'Temporal Ring', 4, 10, NULL, 100, NULL, 0, NULL, 1, 4294967295, 63, 2, 136, NULL, '100', 0, 0, 'bonus bAtk,50; bonus bMatk,50; bonus bMaxHPrate,10; bonus bMaxSPrate,10;', NULL, NULL),
(28901, 'Cursed_Mad_Bunny', 'Cursed Mad Bunny', 4, 20, NULL, 100, NULL, 0, NULL, 0, 4294967295, 63, 2, 32, NULL, '1', 1, 1, 'bonus bAspd,3; bonus2 bAddRace,RC_All,5; bonus2 bMagicAddRace,RC_All,5; bonus bShortWeaponDamageReturn,10; autobonus2 \"{ bonus bMagicDamageReturn,60; }\",10,2000,BF_MAGIC,\"{ specialeffect2 EF_WIND; }\"; .@r = getrefine(); if(.@r>=9) { bonus bBaseAtk,5; bonus bMatk,5; } if(.@r>=12) { bonus bBaseAtk,15; bonus bMatk,15; } if(.@r>=15) { bonus bCritical,10; bonus bNoCastCancel; }', NULL, NULL),
(28902, 'Mad_Bunny_', 'Cursed Mad Bunny', 4, 20, NULL, 100, NULL, 0, NULL, 1, 4294967295, 63, 2, 32, NULL, '1', 1, 1, 'bonus bAspd,3; bonus2 bAddRace,RC_All,5; bonus2 bMagicAddRace,RC_All,5; bonus bShortWeaponDamageReturn,10; autobonus2 \"{ bonus bMagicDamageReturn,60; }\",10,2000,BF_MAGIC,\"{ specialeffect2 EF_WIND; }\"; .@r = getrefine(); if(.@r>=9) { bonus bBaseAtk,5; bonus bMatk,5; } if(.@r>=12) { bonus bBaseAtk,15; bonus bMatk,15; } if(.@r>=15) { bonus bCritical,10; bonus bNoCastCancel; }', NULL, NULL),
(18597, 'Mercury_Helm', 'Mercury Riser', 4, 40, NULL, 200, NULL, 10, NULL, 1, 4294967295, 63, 2, 256, NULL, '0', 1, 759, 'bonus2 bSubRace,RC_DemiHuman,10; bonus2 bAddRace,RC_DemiHuman,10; bonus2 bSubRace,RC_Player,10; bonus2 bAddRace,RC_Player,10; bonus bAspdRate,3; bonus bDelayrate,-3; .@r = getrefine(); if(.@r >= 7) { bonus bAspdRate,2; bonus bDelayrate,-2; } if(.@r >= 9) { bonus bAspdRate,2; bonus bDelayrate,-2; }', NULL, NULL);


# new cash items
REPLACE INTO `item_cash_db` (`tab`, `item_id`, `price`) VALUES
(0, 32204, 35), # Immortal Dog Tag
(0, 28594, 35), # Temporal Ring
(0, 28902, 35), # Cursed Mad Bunny [1]
(0, 2589, 45), # FAW
(7, 32204, 35), # Immortal Dog Tag
(7, 28594, 35), # Temporal Ring
(7, 28902, 35), # Cursed Mad Bunny [1]
(7, 2589, 45); # FAW


# Moved all honor mission closer to the castle
UPDATE `mission_board` SET `npc_id` = '|13|' WHERE `id` IN (1542786300,1542786310,1542786320,1542786330,1542786340,1542786350,1542786360);

# Reduce mission min level req for all mission but 3rd class headgear
UPDATE `mission_board` SET `min_lv` = 50 WHERE `min_lv` = 99;

# Moved all the 3rd Class Headgear mission to Midgard Expedition Camp
UPDATE `mission_board` SET `npc_id` = '|14|' WHERE `id` IN (1542649146,1542649795,1542656903,1542685464,1542687644,1542699198,1542700215,1542702582,1542702592,1542705456,1542706619,1542708851,1542709404,1542711840);

# Missions
REPLACE INTO `mission_board` (`id`, `title`, `desc`, `mob_list`, `mob_qty`, `item_list`, `item_qty`, `class_limitation`, `class_branch`, `min_lv`, `max_lv`, `repeat`, `duration`, `reward_list`, `reward_qty`, `base_exp`, `job_exp`, `zeny`, `cash`, `aid`, `name`, `time_update`, `npc_id`, `redo_delay`) VALUES
(1545814000, 'Immortal Dog Tag [1]', 'Please kill some monsters and gather the materials it drops. The guild will reward you with an Immortal Dog Tag for your effort. - Adventurer\'s Guild', '|1032|1044|1129|1367|1778|1652|1653|1654|1655|1656|1657|1700|1701|1702|1703|', '|500|30|30|30|30|25|25|25|25|25|25|5|5|5|5', '|929|7345|950|953|7097|7561|7442|', '|1500|500|100|100|100|100|50|', 1023, 31, 50, 175, 0, 0, '|32204|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-12-26 00:01:00', '|0|', 21),
(1545814010, 'Temporal Ring [1]', 'My master recently made a new ring. It gives bonus to the previous boots that he made. If you want them, please give me the required material. - Hugin\s Other Butler', '', '', '|6607|6608|', '|50|1000|', 1023, 31, 50, 175, 0, 0, '|28594|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-12-26 00:01:00', '|0|', 21),
(1545814020, 'Isabella Brown Ear [1]', 'In my travels, I saw a small girl wearing a very sweet design ribbon. I already made the red version, now i want to recolour it to brown. - Ribbon Seemstress', '', '', '|18908|983|980|', '|1|10|10|', 1023, 31, 50, 175, 0, 0, '|18909|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-12-26 00:01:00', '|4|', 21),
(1545814020, 'Isabella Blue Ear [1]', 'In my travels, I saw a small girl wearing a very sweet design ribbon. I already made the red version, now i want to recolour it to blue. - Ribbon Seemstress', '', '', '|18908|978|', '|1|20|', 1023, 31, 50, 175, 0, 0, '|18910|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-12-26 00:01:00', '|4|', 21);

# T_W_O and mad bunny
REPLACE INTO `mob_db2_re` (`ID`, `Sprite`, `kName`, `iName`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `aDelay`, `aMotion`, `dMotion`, `MEXP`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7id`, `Drop7per`, `Drop8id`, `Drop8per`, `Drop9id`, `Drop9per`, `DropCardid`, `DropCardper`) VALUES
(3254, 'T_W_O', 'T_W_O', 'T_W_O', 165, 48000000, 1, 0, 0, 1, 3955, 196, 158, 134, 90, 141, 7, 87, 267, 70, 10, 12, 2, 6, 67, 103284869, 150, 1250, 500, 350, 0, 6832, 5000, 617, 5000, 617, 5000, 22699, 3000, 1531, 500, 7319, 2000, 28901, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27020, 1);
