ALTER TABLE  `char` ADD COLUMN `sex` ENUM('M','F','U') NOT NULL default 'U';

CREATE TABLE `db_roulette` (
  `level` smallint(5) unsigned NOT NULL,
  `item_id` smallint(5) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `flag` smallint(5) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`level`,`item_id`)
) ENGINE=MyISAM;

-- ----------------------------
-- Records of db_roulette
-- ----------------------------
-- Info: http://ro.gnjoy.com/news/update/View.asp?seq=157&curpage=1

INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 675, 1, 1 ); -- Silver_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 671, 1, 0 ); -- Gold_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 678, 1, 0 ); -- Poison_Bottle
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 604, 1, 0 ); -- Branch_Of_Dead_Tree
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 522, 1, 0 ); -- Fruit_Of_Mastela
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 671, 1, 0 ); -- Old_Ore_Box
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 12523, 1, 0 ); -- E_Inc_Agi_10_Scroll
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 985, 1, 0 ); -- Elunium
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 1, 984, 1, 0 ); -- Oridecon

INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 675, 1, 1 ); -- Silver_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 671, 1, 0 ); -- Gold_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 603, 1, 0 ); -- Old_Blue_Box
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 608, 1, 0 ); -- Seed_Of_Yggdrasil
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 607, 1, 0 ); -- Yggdrasilberry
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 12522, 1, 0 ); -- E_Blessing_10_Scroll
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 6223, 1, 0 ); -- Carnium
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 2, 6224, 1, 0 ); -- Bradium

INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 3, 675, 1, 1 ); -- Silver_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 3, 671, 1, 0 ); -- Gold_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 3, 12108, 1, 0 ); -- Bundle_Of_Magic_Scroll
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 3, 617, 1, 0 ); -- Old_Violet_Box
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 3, 12514, 1, 0 ); -- E_Abrasive
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 3, 7444, 1, 0 ); -- Treasure_Box
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 3, 969, 1, 0 ); -- Gold

INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 4, 675, 1, 1 ); -- Silver_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 4, 671, 1, 0 ); -- Gold_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 4, 616, 1, 0 ); -- Old_Card_Album
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 4, 12516, 1, 0 ); -- E_Small_Life_Potion
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 4, 22777, 1, 0 ); -- Gift_Buff_Set
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 4, 6231, 1, 0 ); -- Guarantee_Weapon_6Up

INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 5, 671, 1, 1 ); -- Gold_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 5, 12246, 1, 0 ); -- Magic_Card_Album
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 5, 12263, 1, 0 ); -- Comp_Battle_Manual
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 5, 671, 1, 0 ); -- Potion_Box
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 5, 6235, 1, 0 ); -- Guarantee_Armor_6Up

INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 6, 671, 1, 1 ); -- Gold_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 6, 12766, 1, 0 ); -- Reward_Job_BM25
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 6, 6234, 1, 0 ); -- Guarantee_Armor_7Up
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 6, 6233, 1, 0 ); -- Guarantee_Armor_8Up

INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 7, 671, 1, 1 ); -- Gold_Coin
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 7, 6233, 1, 0 ); -- Guarantee_Armor_8Up
INSERT INTO `db_roulette`(`level`, `item_id`, `amount`, `flag` ) VALUES ( 7, 6233, 1, 0 ); -- Guarantee_Armor_8Up	// KRO lists this twice
