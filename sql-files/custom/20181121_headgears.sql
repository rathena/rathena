REPLACE INTO `mission_board` (`id`, `title`, `desc`, `mob_list`, `mob_qty`, `item_list`, `item_qty`, `class_limitation`, `class_branch`, `min_lv`, `max_lv`, `repeat`, `duration`, `reward_list`, `reward_qty`, `base_exp`, `job_exp`, `zeny`, `cash`, `aid`, `name`, `time_update`, `npc_id`, `redo_delay`) VALUES
(	1542784459,
	'Blue Ribbon [1]',
	'I need the materials to make Blue Ribbons for my shop. If you bring me enough materials, I will give you 1 complete product. - Ribbon Seemstress',

	'', # no mobs
	'',

	'|2209|10007|978|5085|',
	'|25|25|15|5|',

	1023, 31, 1, 175, 0, 0,
	'|5404|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|4|', 168),
(	1542784469,
	'QueenAnzRevenge [1]',
	'There was a legend about Queen Anz said amongs the sailors of Alberta. I want to make a headgear of Queen Anz. Please help me gather the required materials. As a reward, I will make an extra for you. - Legend Lover',

	'|1071|', # Skeleton Pirate
	'|100|',

	'|914|1020|1059|7005|932|969|975|982|983|6654|',
	'|250|150|150|50|50|25|15|15|15|2|',

	1023, 31, 1, 175, 0, 0,
	'|5969|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|3|', 168),
(	1542784479,
	'Needle And Thread',
	'My wife lost her needle and thread. I can make a set of needle and thread for her only if I have enough materials. - Local Artisan',

	'', # no mobs
	'',

	'|7215|7213|7197|1059|7166|',
	'|100|100|25|25|25|',

	1023, 31, 1, 175, 0, 0,
	'|6654|', '|1|',

	100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|6|', 0),
(	1542784489,
	'Adventurer\'s Backpack [1]',
	'Every adventurer needs to carry provisions and items to prepare for their journey but no one is selling good backpacks. I came up with the design to such an Adventurer\'s Backpack [1], but I am too weak to gather the materials.',

	'', # no mobs
	'',

	'|914|1059|7166|1950|741|6654|',
	'|250|250|250|15|5|2|',

	1023, 31, 1, 175, 0, 0,
	'|2576|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|0|', 168),
(	1542784499,
	'Rideword Hat [1]',
	'I can\'t sleep at night. Rideword is always flapping at night and the noise is very irritating. Please help me clear them. I\'ll make a Rideword Hat out of the items from all the Ridewords you killed. - Disturbed Citizen',

	'|1195|2478|',
	'|100|100|',

	'|1097|7015|',
	'|100|100|',

	1023, 31, 1, 175, 0, 0,
	'|5208|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|6|', 168),
(	1542784509,
	'Red Baby Dragon [1]',
	'I been collecting dolls from all over the world. To complete my collection, I manage to hire a seemstress to make me a Red Baby Dragon. But she requires a lot of materials that I cannot acquire myself. - Doll Enthusiast',

	'',  # no mobs
	'',

	'|914|1035|1036|1037|740|975|',
	'|150|150|150|150|50|15|',

	1023, 31, 1, 175, 0, 0,
	'|19116|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|7|', 168),
(	1542784519,
	'Imp Hat [1]',
	'I been collecting dolls from all over the world. To complete my collection, I manage to hire a seemstress to make me a Imp Hat. But she requires a lot of materials that I cannot acquire myself. - Doll Enthusiast',

	'',  # no mobs
	'',

	'|914|7097|7122|740|975|',
	'|150|150|150|50|15|',

	1023, 31, 1, 175, 0, 0,
	'|19116|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|7|', 168),
(	1542784519,
	'Isabella Red Ear [1]',
	'In my travels, I saw a small girl wearing a very sweet design ribbon. But because of the danger, I was afraid to approach her. I believe I could recreate the ribbon if I have enough materials. - Ribbon Seemstress',

	'',  # no mobs
	'',

	'|7166|2244|10007|5085|975|',
	'|150|25|25|10|15|',

	1023, 31, 1, 175, 0, 0,
	'|18908|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|4|', 168);
(	1542784519,
	'Ferlock\'s Hat',
	'We are commissioning a mission to kill Captain Ferlock. Captain Ferlock has gone rogue and needs to killed. - Bounty Guild',

	'|3181|',  # no mobs
	'|1|',

	'',
	'',

	1023, 31, 1, 175, 0, 0,
	'|19914|', '|1|',

	100000, 100000, 500000, 0, 2000000, '<GM>Nubs', '2018-11-21 15:14:09', '|4|', 168);

REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(5658, 'Imp_Hat', 'Imp Hat', 4, 20, NULL, 400, NULL, 1, NULL, 1, 4294967295, 63, 2, 256, NULL, '1', 1, 589, 'bonus3 bAutoSpell,\"SA_FLAMELAUNCHER\",1,5;\r\n.@r = getrefine();\r\nautobonus \"{ bonus3 bAddEle,Ele_Fire,10,BF_MAGIC; }\",(.@r > 6 ? 10 : 5),5000,BF_MAGIC,\"{ specialeffect2 EF_FLAMELAUNCHER; }\";\r\nif(.@r>8){ bonus3 bAutoSpellWhenHit,\"NPC_PULSESTRIKE\",1,5; }\r\n', NULL, NULL),
(18908, 'Isabella_Red_Ear', 'Isabella Red Ear', 4, 10, NULL, 300, NULL, 8, NULL, 1, 4294967295, 63, 2, 256, NULL, '1', 1, 1030, 'bonus bStr,5; bonus bMaxHPrate,5; bonus2 bSubDefEle,Ele_Fire,10; .@r = getrefine(); if (.@r >= 9) bonus bAspd,(.@r-8)/2;', NULL, NULL);

# fix equip update
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(2971, 'Pocket_Watch', 'Pocket Watch', 4, 10, NULL, 200, NULL, 1, NULL, 1, 8454660, 63, 2, 136, NULL, '80', 0, NULL, 'bonus bHPrecovRate,15; bonus bSPrecovRate,15; bonus bMatkRate,7;', NULL, NULL);

#25184 | Portable Sewingbox