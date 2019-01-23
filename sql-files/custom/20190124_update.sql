REPLACE INTO `item_db2_re` (`id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `atk:matk`, `defence`, `range`, `slots`, `equip_jobs`, `equip_upper`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `refineable`, `view`, `script`, `equip_script`, `unequip_script`) VALUES
(13210, 'Slug_Bullet_1', 'Slug Ammunition L', 10, 250, NULL, 250, '30', NULL, NULL, NULL, 1090519040, 63, 2, 32768, NULL, '1', NULL, 3, NULL, NULL, NULL),
(13211, 'Slug_Bullet_2', 'Slug Ammunition M', 10, 500, NULL, 500, '30', NULL, NULL, NULL, 1090519040, 63, 2, 32768, NULL, '1', NULL, 3, NULL, NULL, NULL),
(13212, 'Slug_Bullet_3', 'Slug Ammunition H', 10, 750, NULL, 750, '30', NULL, NULL, NULL, 1090519040, 63, 2, 32768, NULL, '1', NULL, 3, NULL, NULL, NULL),
(13213, 'Slug_Bullet_4', 'Slug Ammunition SH', 10, 1000, NULL, 1000, '30', NULL, NULL, NULL, 1090519040, 63, 2, 32768, NULL, '1', NULL, 3, NULL, NULL, NULL),
(13214, 'Slug_Bullet_5', 'Slug Ammunition XH', 10, 1200, NULL, 1200, '30', NULL, NULL, NULL, 1090519040, 63, 2, 32768, NULL, '1', NULL, 3, NULL, NULL, NULL),
(28605, 'Book_of_Vicious_Mind', 'Book of Vicious Mind', 5, 20, NULL, 950, '95', NULL, 1, 1, 4260096, 63, 2, 2, 4, '160', 1, 15, 'bonus bAtk,pow(min(getrefine(),15),2); bonus bMatk,pow(min(getrefine(),15),2); bonus bUnbreakableWeapon;', NULL, NULL),
(16979, 'F_Silvervine_Fruit_Box10', '[F] Silvervine Box (10)', 2, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_Silvervine_Fruit_Box10);', NULL, NULL),
(8503, 'F_10_Food_Box', '[F] +10 Food Box', 18, 20, NULL, 0, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_CASH_FOOD);', NULL, NULL),
(8507, 'F_Acidbomb_10_Box', '[F] Acid Bomb Box (10)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_ACIDBOMB_10_BOX);', NULL, NULL),
(8508, 'F_Acidbomb_50_Box', '[F] Acid Bomb Box (50)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_ACIDBOMB_50_BOX);', NULL, NULL),
(8509, 'F_Poison_Bottle_10_Box', '[F] Deadly Poison Box (10)', 18, 0, NULL, 0, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_POISON_10_BOX);', NULL, NULL),
(8510, 'F_Poison_Bottle_50_Box', '[F] Deadly Poison Box (50)', 18, 0, NULL, 0, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_POISON_50_BOX);', NULL, NULL),
(8511, 'F_Blacksmith_Blessing_20Box', '[F] Blacksmith Blessing Box (20)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_BLESSING_20_BOX);', NULL, NULL),
(14219, 'F_Enriched_Elunium_Box5', '[F] Enriched Elunium Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_ENELU_5_BOX);', NULL, NULL),
(14176, 'F_Enriched_Elunium_Box10', '[F] Enriched Elunium Box (10)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_ENELU_10_BOX);', NULL, NULL),
(14220, 'F_Enriched_Oridecon_Box5', '[F] Enriched Oridecon Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_ENORI_5_BOX);', NULL, NULL),
(14177, 'F_Enriched_Oridecon_Box10', '[F] Enriched Oridecon Box (10)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_ENORI_10_BOX);', NULL, NULL),
(16262, 'F_HD_Bradium_Box5', '[F] HD Bradium Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_HDBRA_5_BOX);', NULL, NULL),
(16263, 'F_HD_Carnium_Box5', '[F] HD Carnium Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_HDCAR_5_BOX);', NULL, NULL),
(17246, 'F_HD_Elunium_Box_5', '[F] HD Elunium Box (5)', 18, 10, NULL, 10, NULL, NULL, NULL, 0, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_HDELU_5_BOX);', NULL, NULL),
(17247, 'F_HD_Oridecon_Box_5', '[F] HD Oridecon Box (5)', 18, 10, NULL, 10, NULL, NULL, NULL, 0, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_HDORI_5_BOX);', NULL, NULL),
(14158, 'F_Bubble_Gum_Box', '[F] Bubble Gum Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_BG_5_BOX);', NULL, NULL),
(14168, 'F_Convex_Mirror_Box', '[F] Convex Mirror Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_CM_5_BOX);', NULL, NULL),
(14156, 'F_Battle_Manual_Box', '[F] Battle Manual Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_BM_5_BOX);', NULL, NULL),
(14178, 'F_Token_Of_Siegfried_Box', '[F] Token of Siegfried Box (5)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_TS_5_BOX);', NULL, NULL),
(12535, 'F_Yggdrasilberry_Box', '[F] Yggdrasilberry Box (5)', 2, 0, NULL, 0, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_YGG_5_BOX);', NULL, NULL),
(8512, 'F_GuildPax1', '[F] Guild Investment Pack (Set 1)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_GPS1_BOX); getgroupitem(IG_MILKY_GP1_BOX);', NULL, NULL),
(8513, 'F_GuildPax2', '[F] Guild Investment Pack (Set 2)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_GPS2_BOX); getgroupitem(IG_MILKY_GP1_BOX);', NULL, NULL),
(8514, 'F_GuildPax3', '[F] Guild Investment Pack (Set 3)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_GPS3_BOX); getgroupitem(IG_MILKY_GP1_BOX);', NULL, NULL),
(8515, 'F_GuildPaxNoEQ', '[F] Guild Investment Pack (No EQ)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_GP2_BOX);', NULL, NULL),
(8516, 'F_GuildPaxSC', '[F] Guild Investment Pack (SC)', 18, 20, NULL, 10, NULL, NULL, NULL, NULL, 4294967295, 63, 2, NULL, NULL, NULL, NULL, NULL, 'getgroupitem(IG_MILKY_GPSC_BOX);', NULL, NULL),

# Guild Invite
CREATE TABLE IF NOT EXISTS `guild_invite_members` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `guild_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `unique_id` int(11) NOT NULL COMMENT 'gepard unique_id',
  `selection` tinyint(1) NOT NULL COMMENT '1 to 4',
  `date_registered` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '0 and 1',
  `claimed` tinyint(1) NOT NULL DEFAULT '0',
  `date_claimed` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `deleted` int(11) NOT NULL DEFAULT '0' COMMENT '0 or 1',
  `date_deleted` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `multi_pc` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account_id` int(11) NOT NULL,
  `unique_id` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


SELECT * FROM `acc_reg_num` WHERE `account_id` > 2000656 AND `key` = '#xlaunch_vipx' AND `account_id` != 2000734;