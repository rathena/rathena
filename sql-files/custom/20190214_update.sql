REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(32013, 'Metal_Stick', 'Metal Stick', 5, 20, NULL, 2000, '180', NULL, 3, 3, 16514, 63, 2, 2, 4, '150', 1, 4, 'bonus2 bSkillAtk,\"LG_CANNONSPEAR\",10;\r\n.@r = getrefine();\r\nif(.@r>6){ bonus2 bSkillAtk,\"LG_OVERBRAND\",10; }\r\nif(.@r>8){ bonus2 bSkillAtk,\"LG_OVERBRAND\",15; }\r\nif(.@r>11){ bonus bDelayrate,-15; }', NULL, NULL);

# item update/fixes
REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(5675, 'Poring_Letter', 'Poring Letter', 4, 20, NULL, 100, NULL, 0, NULL, 0, 4294967295, 63, 2, 1, NULL, '10', 0, 604, 'bonus2 bAddMonsterDropItem,619,10; bonus2 bAddRace,RC_Plant,5; bonus bUnbreakableHelm;', NULL, NULL),
(18546, 'Lover_In_Mouth', 'Lover In Mouth', 4, 20, NULL, 300, NULL, 0, NULL, 0, 4294967295, 63, 2, 1, NULL, '0', 0, 720, 'bonus2 bAddMonsterDropItem,529,10; bonus2 bAddMonsterDropItem,530,10; bonus2 bAddItemHealRate,529,50; bonus2 bAddItemHealRate,530,50;', NULL, NULL);

# Removed Flower Hairpin from mission
DELETE FROM `mission_board` WHERE `mission_board`.`id` = 1542786130;

# Changed Golden Thread materials
REPLACE INTO `mission_board` (`id`, `title`, `desc`, `mob_list`, `mob_qty`, `item_list`, `item_qty`, `class_limitation`, `class_branch`, `min_lv`, `max_lv`, `repeat`, `duration`, `reward_list`, `reward_qty`, `base_exp`, `job_exp`, `zeny`, `cash`, `aid`, `name`, `time_update`, `npc_id`, `redo_delay`) VALUES
(1548750837, 'Golden Thread', 'Now, my wife ran out of her Golden Thread. I can make more for her only if I have enough materials. I give you a spindle of the thread if you bring me the materials. - Local Artisan', '', '', '|969|7217|1025|', '|20|20|20|', 1023, 31, 50, 175, 0, 0, '|7879|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-01-29 09:33:57', '|6|', 0),
(1550075309, 'I Love You', 'To celebrate the Love Festival, I offer my service to make an love shaped cushion. Get me the materials I need, and I will make the cushion for you. -  Local Tailor', '', '', '|741|975|7879|6654|', '|50|5|1|1|', 1023, 31, 50, 175, 0, 0, '|6206|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075310, 'Love Valentine\'s Hat', 'Lets spread the love in this world. Get yourself a Love Valentine\'s Hat. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|1757|6206|6654|5049|', '|25|2|2|1|', 1023, 31, 50, 175, 0, 0, '|5393|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075311, 'Poring Letter', 'Lets spread the love in this world. Get yourself a Poring Letter. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|12370|12373|6206|', '|25|25|1|', 1023, 31, 50, 175, 0, 0, '|5675|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075312, 'Love Chick Hat', 'Lets spread the love in this world. Get yourself a Love Chick Hat. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|975|6206|6654|5283|19607 1|', '|5|2|2|2|Costume Love Chick Hat|', 1023, 31, 50, 175, 0, 0, '|5822|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075313, 'Love Piece', 'Lets spread the love in this world. Get yourself a Love In Mouth. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|7353|975|6206|5110|20022 2|', '|100|5|2|2|Costume Love Fragment|', 1023, 31, 50, 175, 0, 0, '|18564|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075314, 'Memories Of Lovers', 'Lets spread the love in this world. Get yourself a Memories Of Lovers. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|7353|975|6206|20376|', '|100|5|2|2|', 1023, 31, 50, 175, 0, 0, '|18937|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075315, 'Costume Blush', 'Lets spread the love in this world. Get yourself a Costume Blush. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|6654|5040|', '|2|2|', 1023, 31, 50, 175, 0, 0, '|19550|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075316, 'Costume First Love Cheek', 'Lets spread the love in this world. Get yourself a Costume First Love Cheek. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|6654|6206|19550|', '|2|2|2|', 1023, 31, 50, 175, 0, 0, '|31047|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075317, 'Costume Love Cheeks', 'Lets spread the love in this world. Get yourself a Costume First Love Cheek. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|6654|6206|31047|', '|2|2|2|', 1023, 31, 50, 175, 0, 0, '|20255|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075318, 'Costume Two Tone Beret', 'Lets spread the love in this world. Get yourself a Costume Two Tone Beret. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|983|980|6206|6654|18791|5379|5020|', '|15|5|2|2|1|1|1|', 1023, 31, 50, 175, 0, 0, '|20463|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075319, 'Costume Sword Master Crown', 'Lets spread the love in this world. Get yourself a Costume Sword Master Crown. Get me the materials I need, and I will make the headgear for you. -  White Eyed Peas', '', '', '|6206|6654|25239|18865|', '|2|2|2|1|', 1023, 31, 50, 175, 0, 0, '|20036|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|99|', 21),	
(1550075320, 'Chick Hat', 'Chicks are cute and I breed them. If you want to put a chick on your head, I will require a few items to support my family. -  Chicken Breeder', '', '', '|8501|603|644|969|', '|25|25|25|25|', 1023, 31, 50, 175, 0, 0, '|5283|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|9|', 21),	
(1550075321, 'Tam[1]', 'I got an inspiration to design a balloon styled headgear. I called it Tam. If you want me to make one for you, please get me the materials. -  Hat Maker', '', '', '|7166|1059|914|5076|5070|5061|2269|6654|', '|100|100|100|1|1|1|1|1|', 1023, 31, 50, 175, 0, 0, '|5379|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|15|', 21),	
(1550075322, 'Kafra Band', 'Kafra wears a very sweet looking band. If you want to wear the same band, get me the materials and I will make one for you. -  Hat Maker', '', '', '|7166|1059|914|5049|2209|', '|50|50|50|1|2|', 1023, 31, 50, 175, 0, 0, '|5020|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|6|', 21),	
(1550075323, 'Costume Ribbon Hat', 'I designed a very sweet hat to protect you from the sunlight. If you want me to make one for you, get me the materials I need. -  Hat Maker', '', '', '|982|983|975|6654|5052|5083|5379|', '|10|5|5|2|1|1|1|', 1023, 31, 50, 175, 0, 0, '|20372|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|12|', 21),	
(1550075324, 'Costume Straight Pony Black', 'I believe hair should be more stylistic. But our hair could only grow so long. To remedy this, I designed hair extensions. Get me enough materials and I will make you the extensions. -  Hair Otaku', '', '', '|983|1020|1060|1034|7152|', '|25|375|375|375|375|', 1023, 31, 50, 175, 0, 0, '|20340|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|2|', 21),	
(1550075325, 'Costume Bouncing Hair Black', 'I believe hair should be more stylistic. But our hair could only grow so long. To remedy this, I designed hair extensions. Get me enough materials and I will make you the extensions. -  Hair Otaku', '', '', '|983|1020|1060|1034|7152|', '|15|150|150|150|150|', 1023, 31, 50, 175, 0, 0, '|20341|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|2|', 21),	
(1550075326, 'Costume Loose Wave Twin', 'I believe hair should be more stylistic. But our hair could only grow so long. To remedy this, I designed hair extensions. Get me enough materials and I will make you the extensions. -  Hair Otaku', '', '', '|983|1020|1060|1034|7152|', '|25|375|375|375|375|', 1023, 31, 50, 175, 0, 0, '|20342|', '|1|', 100000, 100000, 250000, 0, 2000000, '<GM>Nubs', '2019-02-13 17:28:29', '|2|', 21);

# clear broken data in multi_pc table, clear testing data in guild invite.
TRUNCATE TABLE `multi_pc`;
TRUNCATE TABLE `guild_invite_members`;

# Survey NPC
CREATE TABLE IF NOT EXISTS `nubs_survey` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `question` varchar(255) NOT NULL,
  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `active` tinyint(4) NOT NULL COMMENT '0 = inactive, 1 = active',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

REPLACE INTO `nubs_survey` (`id`, `question`, `date`, `active`) VALUES
(1, 'WoE Castle rotation starting March.', '2019-02-14 00:15:00', 1);

CREATE TABLE IF NOT EXISTS `nubs_survey_options` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `options` varchar(255) NOT NULL,
  `survey_id` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;

REPLACE INTO `nubs_survey_options` (`id`, `options`, `survey_id`) VALUES
(1, 'No', 1),
(2, 'Yes', 1);

CREATE TABLE IF NOT EXISTS `nubs_survey_votes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `ip_address` varchar(20) NOT NULL,
  `unique_id` int(11) NOT NULL,
  `char_id` tinyint(4) NOT NULL,
  `survey_id` int(11) NOT NULL,
  `vote_id` tinyint(4) NOT NULL,
  `date_voted` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=latin1;