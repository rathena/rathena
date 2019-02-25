DROP TABLE IF EXISTS `mission_board`;
CREATE TABLE IF NOT EXISTS `mission_board` (
	`id` int(11) unsigned NOT NULL,
	`title` varchar(30) NOT NULL default '',
	`desc` varchar(255) NOT NULL default '',
	`mob_list` varchar(50) NOT NULL default '',
	`mob_qty` varchar(50) NOT NULL default '',
	`item_list` varchar(50) NOT NULL default '',
	`item_qty` varchar(50) NOT NULL default '',
	`class_limitation` int(11) unsigned NOT NULL default '0',
	`class_branch` int(11) unsigned NOT NULL default '0',
	`min_lv` smallint(6) unsigned NOT NULL default '1',
	`max_lv` smallint(6) unsigned NOT NULL default '99',
	`repeat` smallint(6) unsigned NOT NULL default '0',
	`duration` int(11) unsigned NOT NULL default '0',
	`reward_list` varchar(50) NOT NULL default '',
	`reward_qty` varchar(50) NOT NULL default '',
	`base_exp` int(11) unsigned NOT NULL default '0',
	`job_exp` int(11) unsigned NOT NULL default '0',
	`zeny` int(11) unsigned NOT NULL default '0',
	`cash` int(11) unsigned NOT NULL default '0',
	`aid` int(11) unsigned NOT NULL default '0',
	`name` varchar(30) NOT NULL default '',
	`time_update` datetime NOT NULL default '0000-00-00 00:00:00',
	`npc_id` varchar(255) NOT NULL default '',
	`redo_delay` smallint(6) unsigned NOT NULL default '0',
	PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `player_mission`;
CREATE TABLE IF NOT EXISTS `player_mission` (
	`id` int(11) unsigned NOT NULL,
	`mission_id` int(11) unsigned NOT NULL,
	`aid` int(11) unsigned NOT NULL default '0',
	`cid` int(11) unsigned NOT NULL default '0',
	`name` varchar(30) NOT NULL default '',
	`mob_hunt` varchar(50) NOT NULL default '',
	`expire` int(11) unsigned NOT NULL default '0',
	`starting` datetime NOT NULL default '0000-00-00 00:00:00',
	`completion` datetime NOT NULL default '0000-00-00 00:00:00',
	PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

REPLACE INTO `mission_board` (`id`, `title`, `desc`, `mob_list`, `mob_qty`, `item_list`, `item_qty`, `class_limitation`, `class_branch`, `min_lv`, `max_lv`, `repeat`, `duration`, `reward_list`, `reward_qty`, `base_exp`, `job_exp`, `zeny`, `cash`, `aid`, `name`, `time_update`, `npc_id`, `redo_delay`) VALUES
(1542649146, 'Rune Circlet [1]', ' A simple quest to get Rune Circlet [1].', '|1163|1276|1195|1192|1203|1204|1205|', '|50|50|50|50|5|5|5|', '', '', 1, 29, 100, 175, 1, 0, '|5746|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 01:39:06', '|0|', 0),
(1542649795, 'Reissue Schmitz Helm [1]', ' A simple quest to get Reissue Schmitz Helm [1].', '|1117|1036|', '|15|15|', '|7097|', '|300|', 1, 30, 100, 175, 1, 0, '|5757|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 01:49:55', '|0|', 0),
(1542656903, 'Magic Stone Hat [1]', ' A simple quest to make Magic Stone Hat [1].', '|2049|2047|1993|2024|2015|2016|', '|15|15|40|40|40|40|', '', '', 2, 29, 100, 175, 1, 0, '|5753|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 03:48:23', '|1|', 0),
(1542685464, 'Wispers of Wind [1]', ' A simple quest to get Wispers of Wind [1].', '|1776|1833|1169|1410|1408|', '|15|15|15|9|9|', '|990|991|993|992|', '|60|60|60|60|', 2, 30, 100, 175, 1, 0, '|5756|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 11:44:24', '|2|', 0),
(1542687644, 'Sniper Goggles [1]', ' A simple quest to get Sniper Goggles [1].', '|1321|1314|1316|1318|1319|', '|40|40|40|40|40|', '|7064|967|', '|40|160|', 4, 29, 100, 175, 1, 0, '|5748|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 12:20:44', '|3|', 0),
(1542699198, 'Minstrel Song Hat [1]', ' A simple quest to get Minstrel Song Hat [1].', '|1622|1718|1715|1776|1165|', '|60|30|30|30|30|', '|7066|7043|1036|', '|30|30|40|', 4, 30, 100, 175, 1, 0, '|5751|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 15:33:18', '|3|', 0),
(1542700215, 'Resting Swan [1]', ' A simple quest to get Resting Swan [1].', '|1715|1718|', '|80|80|', '|1036|', '|160|', 4, 30, 100, 175, 1, 0, '|5758|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 15:50:15', '|4|', 0),
(1542702582, 'Driver Band [1] (Red)', ' A simple quest to get Driver Band [1] (Red).', '|1676|1677|1672|1673|', '|70|70|10|10|', '|7356|7357|7354|7355|', '|70|70|2|2|', 16, 29, 100, 175, 1, 0, '|5749|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 16:29:42', '|5|', 0),
(1542702592, 'Driver Band [1] (Yellow)', ' A simple quest to get Driver Band [1] (Yellow).', '|1678|1679|1670|1671|', '|70|70|10|10|', '|7358|7359|7352|7353|', '|70|70|2|2|', 16, 29, 100, 175, 1, 0, '|5760|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 16:29:42', '|5|', 0),
(1542705456, 'Midas Whispers [1]', ' A simple quest to get Midas Whispers [1].', '|1077|1020|', '|80|80|', '|905|921|972|', '|80|80|20|', 16, 30, 100, 175, 1, 0, '|5752|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 17:17:36', '|6|', 0),
(1542706619, 'Silent Enforcer', ' A simple quest to get Silent Enforcer.', '|1781|1780|', '|120|120|', '|1032|6259|', '|120|120|', 32, 29, 100, 175, 1, 0, '|5755|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 17:36:59', '|1|', 0),
(1542708851, 'Shadow Crown [1]', ' A simple quest to get Shadow Crown [1].', '|1314|1318|1319|1506|1508|1775|', '|30|30|30|30|30|60|', '|967|7216|7205|7066|', '|90|30|30|60|', 32, 30, 100, 175, 1, 0, '|6121|6122|5750|2795|', '|1|1|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 18:14:11', '|1|', 0),
(1542709404, 'Mitra [1]', ' A simple quest to get Mitra [1].', '|1753|1752|', '|80|80|', '|7511|', '|160|', 8, 29, 100, 175, 1, 0, '|5747|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 18:23:24', '|0|', 0),
(1542711840, 'Burning Spirit [1]', ' A simple quest to get Burning Spirit [1].', '|1257|1195|1196|1197|1201|', '|40|40|40|40|40|', '|7015|1098|1099|7017|', '|50|50|50|50|', 8, 30, 100, 175, 1, 0, '|5754|2795|', '|1|1|', 1000000, 1000000, 150000, 0, 2000000, '<GM>Nubs', '2018-11-20 19:04:00', '|7|', 0),
(1542784449, 'Purple Cowboy Hat [1] ', 'Please clean up the bandits in Rockridge Fields. As a reward, I will craft you a Purple Cowbow Hat [1], if you bring me the materials.', '|3736|3737|3738|', '|50|50|50|', '|949|1059|981|5075|5018|', '|50|50|10|1|1|', 1023, 31, 1, 175, 0, 0, '|5409|', '|1|', 1000000, 1000000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|8|', 168),
(1542784509, 'Red Baby Dragon [1]', 'I been collecting dolls from all over the world. To complete my collection, I manage to hire a seemstress to make me a Red Baby Dragon. But she requires a lot of materials that I cannot acquire myself. - Doll Enthusiast', '', '', '|914|1035|1036|1037|740|975|', '|150|150|150|150|50|15|', 1023, 31, 1, 175, 0, 0, '|19116|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|7|', 168),
(1542784459, 'Blue Ribbon [1]', 'I need the materials to make Blue Ribbons for my shop. If you bring me enough materials, I will give you 1 complete product. - Ribbon Seemstress', '', '', '|2209|10007|978|5085|', '|25|25|15|5|', 1023, 31, 1, 175, 0, 0, '|5404|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|4|', 168),
(1542784469, 'QueenAnzRevenge [1]', 'There was a legend about Queen Anz said amongs the sailors of Alberta. I want to make a headgear of Queen Anz. Please help me gather the required materials. As a reward, I will make an extra for you. - Legend Lover', '|1071|', '|100|', '|914|1020|1059|7005|932|969|975|982|983|6654|', '|250|150|150|50|50|25|15|15|15|2|', 1023, 31, 1, 175, 0, 0, '|5969|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|3|', 168),
(1542784479, 'Needle And Thread', 'My wife lost her needle and thread. I can make a set of needle and thread for her only if I have enough materials. - Local Artisan', '', '', '|7215|7213|7197|1059|7166|', '|100|100|25|25|25|', 1023, 31, 1, 175, 0, 0, '|6654|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|6|', 0),
(1542784489, 'Adventurer\'s Backpack [1]', 'Every adventurer needs to carry provisions and items to prepare for their journey but no one is selling good backpacks. I came up with the design to such an Adventurer\'s Backpack [1], but I am too weak to gather the materials.', '', '', '|914|1059|7166|1950|741|6654|', '|250|250|250|15|5|2|', 1023, 31, 1, 175, 0, 0, '|2576|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|0|', 168),
(1542784499, 'Rideword Hat [1]', 'I can\'t sleep at night. Rideword is always flapping at night and the noise is very irritating. Please help me clear them. I\'ll make a Rideword Hat out of the items from all the Ridewords you killed. - Disturbed Citizen', '|1195|2478|', '|100|100|', '|1097|7015|', '|100|100|', 1023, 31, 1, 175, 0, 0, '|5208|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|6|', 168),
(1542784519, 'Imp Hat [1]', 'I been collecting dolls from all over the world. To complete my collection, I manage to hire a seemstress to make me a Imp Hat. But she requires a lot of materials that I cannot acquire myself. - Doll Enthusiast', '', '', '|914|7097|7122|740|975|', '|150|150|150|50|15|', 1023, 31, 1, 175, 0, 0, '|19116|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|7|', 168),
(1542784529, 'Isabella Red Ear [1]', 'In my travels, I saw a small girl wearing a very sweet design ribbon. But because of the danger, I was afraid to approach her. I believe I could recreate the ribbon if I have enough materials. - Ribbon Seemstress', '', '', '|7166|2244|10007|5085|975|', '|150|25|25|10|15|', 1023, 31, 1, 175, 0, 0, '|18908|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|4|', 168),
(1542784539, 'Ferlock\'s Hat', 'We are commissioning a mission to kill Captain Ferlock. Captain Ferlock has gone rogue and needs to killed. - Bounty Guild', '|3181|', '|1|', '', '', 1023, 31, 1, 175, 0, 0, '|19914|', '|1|', 100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|4|', 168);


REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(5658, 'Imp_Hat', 'Imp Hat', 4, 20, NULL, 400, NULL, 1, NULL, 1, 4294967295, 63, 2, 256, NULL, '1', 1, 589, 'bonus3 bAutoSpell,\"SA_FLAMELAUNCHER\",1,5;\r\n.@r = getrefine();\r\nautobonus \"{ bonus3 bAddEle,Ele_Fire,10,BF_MAGIC; }\",(.@r > 6 ? 10 : 5),5000,BF_MAGIC,\"{ specialeffect2 EF_FLAMELAUNCHER; }\";\r\nif(.@r>8){ bonus3 bAutoSpellWhenHit,\"NPC_PULSESTRIKE\",1,5; }\r\n', NULL, NULL),
(18908, 'Isabella_Red_Ear', 'Isabella Red Ear', 4, 10, NULL, 300, NULL, 8, NULL, 1, 4294967295, 63, 2, 256, NULL, '1', 1, 1030, 'bonus bStr,5; bonus bMaxHPrate,5; bonus2 bSubDefEle,Ele_Fire,10; .@r = getrefine(); if (.@r >= 9) bonus bAspd,(.@r-8)/2;', NULL, NULL);

# fix equip update
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(2971, 'Pocket_Watch', 'Pocket Watch', 4, 10, NULL, 200, NULL, 1, NULL, 1, 8454660, 63, 2, 136, NULL, '80', 0, NULL, 'bonus bHPrecovRate,15; bonus bSPrecovRate,15; bonus bMatkRate,7;', NULL, NULL);

#25184 | Portable Sewingbox