//
// When converting from oA to eA, my oA db was in ragnarok so I created
// a new db called eragnarok and then ran the following commands...
// 
// /usr/local/mysql/bin/mysql -Deragnarok -u eragnarok -p< athena/sql-files/main.sql
// /usr/local/mysql/bin/mysql -Deragnarok -u eragnarok -p< athena/sql-files/db_tables.sql
// /usr/local/mysql/bin/mysql -Deragnarok -u eragnarok -p< athena/sql-files/mail.sql
// /usr/local/mysql/bin/mysql -Deragnarok -u eragnarok -p< athena/sql-files/oA2eA-rc5.sql
//
// This creates a eragnarok database and fills everything in from the oA
// database.
// 
//     -- MouseJstr

CREATE TABLE `item_db` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `name_english` varchar(24) NOT NULL default '',
  `name_japanese` varchar(24) NOT NULL default '',
  `type` tinyint(2) unsigned NOT NULL default '0',
  `price_buy` int(10) unsigned default NULL,
  `price_sell` int(10) unsigned default NULL,
  `weight` int(10) unsigned NOT NULL default '0',
  `attack` mediumint(9) unsigned default NULL,
  `defence` mediumint(9) unsigned default NULL,
  `range` tinyint(2) unsigned default NULL,
  `slots` tinyint(1) unsigned default NULL,
  `equip_jobs` mediumint(8) unsigned default NULL,
  `equip_genders` tinyint(1) unsigned default NULL,
  `equip_locations` smallint(4) unsigned default NULL,
  `weapon_level` tinyint(1) unsigned default NULL,
  `equip_level` tinyint(3) unsigned default NULL,
  `view` tinyint(3) unsigned default NULL,
  `script_use` text,
  `script_equip` text,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `mob_db` (
  `ID` mediumint(9) NOT NULL default '0',
  `Name` text NOT NULL,
  `Name2` text NOT NULL,
  `LV` smallint(6) NOT NULL default '0',
  `HP` mediumint(9) NOT NULL default '0',
  `SP` mediumint(9) NOT NULL default '0',
  `EXP` mediumint(9) NOT NULL default '0',
  `JEXP` mediumint(9) NOT NULL default '0',
  `Range1` tinyint(4) NOT NULL default '0',
  `ATK1` smallint(6) NOT NULL default '0',
  `ATK2` smallint(6) NOT NULL default '0',
  `DEF` smallint(6) NOT NULL default '0',
  `MDEF` smallint(6) NOT NULL default '0',
  `STR` tinyint(4) NOT NULL default '0',
  `AGI` tinyint(4) NOT NULL default '0',
  `VIT` tinyint(4) NOT NULL default '0',
  `INT` tinyint(4) NOT NULL default '0',
  `DEX` tinyint(4) NOT NULL default '0',
  `LUK` tinyint(4) NOT NULL default '0',
  `Range2` tinyint(4) NOT NULL default '0',
  `Range3` tinyint(4) NOT NULL default '0',
  `Scale` tinyint(4) NOT NULL default '0',
  `Race` tinyint(4) NOT NULL default '0',
  `Element` tinyint(4) NOT NULL default '0',
  `Mode` smallint(6) NOT NULL default '0',
  `Speed` smallint(6) NOT NULL default '0',
  `ADelay` smallint(6) NOT NULL default '0',
  `aMotion` smallint(6) NOT NULL default '0',
  `dMotion` smallint(6) NOT NULL default '0',
  `Drop1id` mediumint(9) NOT NULL default '0',
  `Drop1per` mediumint(9) NOT NULL default '0',
  `Drop2id` mediumint(9) NOT NULL default '0',
  `Drop2per` mediumint(9) NOT NULL default '0',
  `Drop3id` mediumint(9) NOT NULL default '0',
  `Drop3per` mediumint(9) NOT NULL default '0',
  `Drop4id` mediumint(9) NOT NULL default '0',
  `Drop4per` mediumint(9) NOT NULL default '0',
  `Drop5id` mediumint(9) NOT NULL default '0',
  `Drop5per` mediumint(9) NOT NULL default '0',
  `Drop6id` mediumint(9) NOT NULL default '0',
  `Drop6per` mediumint(9) NOT NULL default '0',
  `Drop7id` mediumint(9) NOT NULL default '0',
  `Drop7per` mediumint(9) NOT NULL default '0',
  `Drop8id` mediumint(9) NOT NULL default '0',
  `Drop8per` mediumint(9) NOT NULL default '0',
  `MEXP` mediumint(9) NOT NULL default '0',
  `ExpPer` mediumint(9) NOT NULL default '0',
  `MVP1id` mediumint(9) NOT NULL default '0',
  `MVP1per` mediumint(9) NOT NULL default '0',
  `MVP2id` mediumint(9) NOT NULL default '0',
  `MVP2per` mediumint(9) NOT NULL default '0',
  `MVP3id` mediumint(9) NOT NULL default '0',
  `MVP3per` mediumint(9) NOT NULL default '0'
) TYPE=MyISAM;

INSERT INTO eragnarok.storage(`id`, `account_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) SELECT `id`, `account_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` from ragnarok.storage;

 INSERT INTO eragnarok.cart_inventory(`id`, `char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `broken`) SELECT `id`, `char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `broken` from ragnarok.cart_inventory;

INSERT INTO eragnarok.char( `char_id`, `account_id`, `char_num`, `name`, `class`, `base_level`, `job_level`, `base_exp`, `job_exp`, `zeny`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `max_hp`, `hp`, `max_sp`, `sp`, `status_point`, `skill_point`, `option`, `karma`, `manner`, `party_id`, `guild_id`, `pet_id`, `hair`, `hair_color`, `clothes_color`, `weapon`, `shield`, `head_top`, `head_mid`, `head_bottom`, `last_map`, `last_x`, `last_y`, `save_map`, `save_x`, `save_y`, `partner_id`, `online`) SELECT  `char_id`, `account_id`, `char_num`, `name`, `class`, `base_level`, `job_level`, `base_exp`, `job_exp`, `zeny`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `max_hp`, `hp`, `max_sp`, `sp`, `status_point`, `skill_point`, `option`, `karma`, `manner`, `party_id`, `guild_id`, `pet_id`, `hair`, `hair_color`, `clothes_color`, `weapon`, `shield`, `head_top`, `head_mid`, `head_bottom`, `last_map`, `last_x`, `last_y`, `save_map`, `save_x`, `save_y`, `partner_id`, `online` from ragnarok.char;

INSERT INTO eragnarok.charlog( `time`, `char_msg`, `account_id`, `char_num`, `name`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hair`, `hair_color` ) SELECT  `time`, `char_msg`, `account_id`, `char_num`, `name`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hair`, `hair_color`  from ragnarok.charlog;

INSERT INTO eragnarok.global_reg_value ( `char_id`, `str`, `value`, `type`, `account_id`) SELECT  `char_id`, `str`, `value`, `type`, `account_id` from ragnarok.global_reg_value ;

INSERT INTO eragnarok.guild( `guild_id`, `name`, `master`, `guild_lv`, `connect_member`, `max_member`, `average_lv`, `exp`, `next_exp`, `skill_point`, `castle_id`, `mes1`, `mes2`, `emblem_len`, `emblem_id`, `emblem_data` ) SELECT  `guild_id`, `name`, `master`, `guild_lv`, `connect_member`, `max_member`, `average_lv`, `exp`, `next_exp`, `skill_point`, `castle_id`, `mes1`, `mes2`, `emblem_len`, `emblem_id`, `emblem_data`  from ragnarok.guild;

INSERT INTO eragnarok.guild_alliance ( `guild_id`, `opposition`, `alliance_id`, `name` ) SELECT  `guild_id`, `opposition`, `alliance_id`, `name`  from ragnarok.guild_alliance ;

INSERT INTO eragnarok.guild_castle ( `castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`, `visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7` ) SELECT  `castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`, `visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`  from ragnarok.guild_castle ;

INSERT INTO eragnarok.guild_expulsion ( `guild_id`, `name`, `mes`, `acc`, `account_id`, `rsv1`, `rsv2`, `rsv3` ) SELECT  `guild_id`, `name`, `mes`, `acc`, `account_id`, `rsv1`, `rsv2`, `rsv3`  from ragnarok.guild_expulsion ;

INSERT INTO eragnarok.guild_member ( `guild_id`, `account_id`, `char_id`, `hair`, `hair_color`, `gender`, `class`, `lv`, `exp`, `exp_payper`, `online`, `position`, `rsv1`, `rsv2`, `name` ) SELECT  `guild_id`, `account_id`, `char_id`, `hair`, `hair_color`, `gender`, `class`, `lv`, `exp`, `exp_payper`, `online`, `position`, `rsv1`, `rsv2`, `name`  from ragnarok.guild_member ;

INSERT INTO eragnarok.guild_position ( `guild_id`, `position`, `name`, `mode`, `exp_mode` ) SELECT  `guild_id`, `position`, `name`, `mode`, `exp_mode`  from ragnarok.guild_position ;

INSERT INTO eragnarok.guild_skill ( `guild_id`, `id`, `lv` ) SELECT  `guild_id`, `id`, `lv`  from ragnarok.guild_skill ;

INSERT INTO eragnarok.guild_storage ( `id`, `guild_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` ) SELECT  `id`, `guild_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`  from ragnarok.guild_storage ;

INSERT INTO eragnarok.interlog ( `time`, `log` ) SELECT  `time`, `log`  from ragnarok.interlog ;

INSERT INTO eragnarok.inventory ( `id`, `char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` ) SELECT  `id`, `char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`  from ragnarok.inventory ;

INSERT INTO eragnarok.ipbanlist ( `list`, `btime`, `rtime`, `reason` ) SELECT  `list`, `btime`, `rtime`, `reason`  from ragnarok.ipbanlist ;


# INSERT INTO eragnarok.login_error ( `err_id`, `reason` ) SELECT  `err_id`, `reason`  from ragnarok.login_error ;

INSERT INTO eragnarok.loginlog ( `time`, `ip`, `user`, `rcode`, `log` ) SELECT  `time`, `ip`, `user`, `rcode`, `log`  from ragnarok.loginlog ;

INSERT INTO eragnarok.memo ( `memo_id`, `char_id`, `map`, `x`, `y` ) SELECT  `memo_id`, `char_id`, `map`, `x`, `y`  from ragnarok.memo ;

INSERT INTO eragnarok.party ( `party_id`, `name`, `exp`, `item`, `leader_id` ) SELECT  `party_id`, `name`, `exp`, `item`, `leader_id`  from ragnarok.party ;

INSERT INTO eragnarok.pet ( `pet_id`, `class`, `name`, `account_id`, `char_id`, `level`, `egg_id`, `equip`, `intimate`, `hungry`, `rename_flag`, `incuvate` ) SELECT  `pet_id`, `class`, `name`, `account_id`, `char_id`, `level`, `egg_id`, `equip`, `intimate`, `hungry`, `rename_flag`, `incuvate`  from ragnarok.pet ;

INSERT INTO eragnarok.ragsrvinfo ( `index`, `name`, `exp`, `jexp`, `drop` ) SELECT  `index`, `name`, `exp`, `jexp`, `drop`  from ragnarok.ragsrvinfo ;

INSERT INTO eragnarok.skill ( `char_id`, `id`, `lv` ) SELECT  `char_id`, `id`, `lv`  from ragnarok.skill ;

INSERT INTO eragnarok.sstatus ( `index`, `name`, `user` ) SELECT  `index`, `name`, `user`  from ragnarok.sstatus ;


# INSERT INTO eragnarok.abra_db ( `ID`, `Dummy`, `Req_Lvl`, `Per`, `Comment`) SELECT `ID`, `Dummy`, `Req_Lvl`, `Per`, `Comment` from ragnarok.abra_db;

# INSERT INTO eragnarok.attr_fix ( `Level`, `None`, `Water`, `Earth`, `Fire`, `Wind`, `Poison`, `Saint`, `Darkness`, `Sense`, `Immortality`, `Comment`) SELECT `Level`, `None`, `Water`, `Earth`, `Fire`, `Wind`, `Poison`, `Saint`, `Darkness`, `Sense`, `Immortality`, `Comment` from ragnarok.attr_fix;

# INSERT INTO eragnarok.cast_db (  `ID`, `Cast_List`, `Delay_List`, `Upkeep_Time`, `Upkeep_Time2`, `Comment` ) SELECT  `ID`, `Cast_List`, `Delay_List`, `Upkeep_Time`, `Upkeep_Time2`, `Comment`  from ragnarok.cast_db;

#INSERT INTO eragnarok.castle_db (  `CastleID`, `map_name`, `castle_name`, `switch_flag`, `Comment` ) SELECT  `CastleID`, `map_name`, `castle_name`, `switch_flag`, `Comment`  from ragnarok.castle_db;

#INSERT INTO eragnarok.create_arrow_db (  `SourceID`, `MakeID1`, `MakeNum1`, `MakeID2`, `MakeNum2`, `MakeID3`, `MakeNum3`, `MakeID4`, `MakeNum4`, `MakeID5`, `MakeNum5`, `Comment` ) SELECT  `SourceID`, `MakeID1`, `MakeNum1`, `MakeID2`, `MakeNum2`, `MakeID3`, `MakeNum3`, `MakeID4`, `MakeNum4`, `MakeID5`, `MakeNum5`, `Comment`  from ragnarok.create_arrow_db;

#INSERT INTO eragnarok.exp (  `EXP1`, `EXP2`, `EXP3`, `EXP4`, `EXP5`, `EXP6`, `EXP7`, `EXP8`, `EXP9`, `EXP10`, `EXP11`, `EXP12`, `EXP13`, `EXP14` ) SELECT  `EXP1`, `EXP2`, `EXP3`, `EXP4`, `EXP5`, `EXP6`, `EXP7`, `EXP8`, `EXP9`, `EXP10`, `EXP11`, `EXP12`, `EXP13`, `EXP14`  from ragnarok.exp;

#INSERT INTO eragnarok.exp_guild (  `Level`, `EXP`, `Comment` ) SELECT  `Level`, `EXP`, `Comment`  from ragnarok.exp_guild;

#INSERT INTO eragnarok.item_bluebox (  `NameID`, `item_name`, `rate`, `Comment` ) SELECT  `NameID`, `item_name`, `rate`, `Comment`  from ragnarok.item_bluebox;

#INSERT INTO eragnarok.item_cardalbum (  `NameID`, `item_name`, `rate`, `Comment` ) SELECT  `NameID`, `item_name`, `rate`, `Comment`  from ragnarok.item_cardalbum;

INSERT INTO eragnarok.item_db2 (  `ID`, `Name`, `Name2`, `Type`, `Price`, `Sell`, `Weight`, `ATK`, `DEF`, `Range`, `Slot`, `Job`, `Gender`, `Loc`, `wLV`, `eLV`, `View`, `UseScript`, `EquipScript`, `Comment` ) SELECT  `ID`, `Name`, `Name2`, `Type`, `Price`, `Sell`, `Weight`, `ATK`, `DEF`, `Range`, `Slot`, `Job`, `Gender`, `Loc`, `wLV`, `eLV`, `View`, `UseScript`, `EquipScript`, `Comment`  from ragnarok.item_db2;

# INSERT INTO eragnarok.item_giftbox (  `NameID`, `item_name`, `rate`, `Comment` ) SELECT  `NameID`, `item_name`, `rate`, `Comment`  from ragnarok.item_giftbox;

#INSERT INTO eragnarok.item_scroll (  `NameID`, `item_name`, `rate`, `Comment` ) SELECT  `NameID`, `item_name`, `rate`, `Comment`  from ragnarok.item_scroll;

#INSERT INTO eragnarok.item_violetbox (  `NameID`, `item_name`, `rate`, `Comment` ) SELECT  `NameID`, `item_name`, `rate`, `Comment`  from ragnarok.item_violetbox;

#INSERT INTO eragnarok.job_db1 (  `Class_ID`, `Weight`, `HP`, `HP2`, `SP`, `Empty`, `Dagger`, `Sword`, `Two_Handed_Sword`, `Spear`, `Two_Handed_Spear`, `Axe`, `Two_Handed_Axe`, `Rod`, `Club`, `Stick`, `Bow`, `Fist`, `Musical`, `Whip`, `Book`, `Katar`, `Comment` ) SELECT  `Class_ID`, `Weight`, `HP`, `HP2`, `SP`, `Empty`, `Dagger`, `Sword`, `Two_Handed_Sword`, `Spear`, `Two_Handed_Spear`, `Axe`, `Two_Handed_Axe`, `Rod`, `Club`, `Stick`, `Bow`, `Fist`, `Musical`, `Whip`, `Book`, `Katar`, `Comment`  from ragnarok.job_db1;

INSERT INTO eragnarok.mob_boss (  `MobID`, `MobName`, `Rate`, `Comment` ) SELECT  `MobID`, `MobName`, `Rate`, `Comment`  from ragnarok.mob_boss;

INSERT INTO eragnarok.mob_branch (  `MobID`, `MobName`, `Rate`, `Comment` ) SELECT  `MobID`, `MobName`, `Rate`, `Comment`  from ragnarok.mob_branch;

INSERT INTO eragnarok.mob_db2 (  `ID`, `Name`, `Name2`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `ADelay`, `aMotion`, `dMotion`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7d`, `Drop7per`, `Drop8id`, `Drop8per`, `Item1`, `Item2`, `MEXP`, `ExpPer`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per`, `Comment` ) SELECT  `ID`, `Name`, `Name2`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `ADelay`, `aMotion`, `dMotion`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7d`, `Drop7per`, `Drop8id`, `Drop8per`, `Item1`, `Item2`, `MEXP`, `ExpPer`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per`, `Comment`  from ragnarok.mob_db2;

INSERT INTO eragnarok.mob_poring (  `MobID`, `MobName`, `Rate`, `Comment` ) SELECT  `MobID`, `MobName`, `Rate`, `Comment`  from ragnarok.mob_poring;

INSERT INTO eragnarok.mob_skill_db (  `Mob_ID`, `Dummy`, `State`, `Skill_ID`, `Skill_LV`, `Use_Rate`, `Cast_Time`, `Delay`, `Disturbance`, `Target`, `Condition_Type`, `Condition_Value`, `Value1`, `Value2`, `Value3`, `Value4`, `Value5`, `Comment` ) SELECT  `Mob_ID`, `Dummy`, `State`, `Skill_ID`, `Skill_LV`, `Use_Rate`, `Cast_Time`, `Delay`, `Disturbance`, `Target`, `Condition_Type`, `Condition_Value`, `Value1`, `Value2`, `Value3`, `Value4`, `Value5`, `Comment`  from ragnarok.mob_skill_db;

#INSERT INTO eragnarok.pet_db (  `MobID`, `Name`, `JName`, `ItemID`, `EggID`, `AcceID`, `FoodID`, `Fullness`, `HungryDeray`, `R_Hungry`, `R_Full`, `Intimate`, `Die`, `Capture`, `Speed`, `S_Performance`, `Talk_Convert_Class`, `Attack_Rate`, `Defence_Attack_Rate`, `Change_Target_Rate`, `Pet_Script`, `Comment` ) SELECT  `MobID`, `Name`, `JName`, `ItemID`, `EggID`, `AcceID`, `FoodID`, `Fullness`, `HungryDeray`, `R_Hungry`, `R_Full`, `Intimate`, `Die`, `Capture`, `Speed`, `S_Performance`, `Talk_Convert_Class`, `Attack_Rate`, `Defence_Attack_Rate`, `Change_Target_Rate`, `Pet_Script`, `Comment`  from ragnarok.pet_db;

#INSERT INTO eragnarok.produce_db (  `ID`, `ItemLV`, `RequireSkill`, `MaterialID1`, `MaterialAmount1`, `MaterialID2`, `MaterialAmount2`, `MaterialID3`, `MaterialAmount3`, `MaterialID4`, `MaterialAmount4`, `MaterialID5`, `MaterialAmount5`, `Comment` ) SELECT  `ID`, `ItemLV`, `RequireSkill`, `MaterialID1`, `MaterialAmount1`, `MaterialID2`, `MaterialAmount2`, `MaterialID3`, `MaterialAmount3`, `MaterialID4`, `MaterialAmount4`, `MaterialID5`, `MaterialAmount5`, `Comment`  from ragnarok.produce_db;

#INSERT INTO eragnarok.refine_db (  `Refine_Bonus`, `Danger_Bonus`, `Safe_Limit`, `RefineChance1`, `RefineChance2`, `RefineChance3`, `RefineChance4`, `RefineChance5`, `RefineChance6`, `RefineChance7`, `RefineChance8`, `RefineChance9`, `RefineChance10`, `Comment` ) SELECT  `Refine_Bonus`, `Danger_Bonus`, `Safe_Limit`, `RefineChance1`, `RefineChance2`, `RefineChance3`, `RefineChance4`, `RefineChance5`, `RefineChance6`, `RefineChance7`, `RefineChance8`, `RefineChance9`, `RefineChance10`, `Comment`  from ragnarok.refine_db;

#INSERT INTO eragnarok.size_fix (  `Element`, `Dagger`, `Sword`, `Two_Handed_Sword`, `Spear`, `Two_Handed_Spear`, `Axe`, `Two_Handed_Axe`, `Club`, `Whip`, `Stick`, `Bow`, `Fist`, `Musical`, `Rod`, `Book`, `Katar`, `Comment` ) SELECT  `Element`, `Dagger`, `Sword`, `Two_Handed_Sword`, `Spear`, `Two_Handed_Spear`, `Axe`, `Two_Handed_Axe`, `Club`, `Whip`, `Stick`, `Bow`, `Fist`, `Musical`, `Rod`, `Book`, `Katar`, `Comment`  from ragnarok.size_fix;

#INSERT INTO eragnarok.skill_db (  `ID`, `Range`, `Hit`, `inf`, `nk`, `max`, `list_num`, `castcancel`, `cast_defence_rate`, `inf2`, `maxcount`, `skill_type`, `blow_count`, `Comment` ) SELECT  `ID`, `Range`, `Hit`, `inf`, `nk`, `max`, `list_num`, `castcancel`, `cast_defence_rate`, `inf2`, `maxcount`, `skill_type`, `blow_count`, `Comment`  from ragnarok.skill_db;

#INSERT INTO eragnarok.skill_require_db (  `ID`, `List_HP`, `List_SP`, `List_HP_Rate`, `List_SP_Rate`, `List_Zeny`, `List_Weapon`, `State`, `Spiritball`, `ItemID1`, `Amount1`, `ItemID2`, `Amount2`, `ItemID3`, `Amount3`, `ItemID4`, `Amount4`, `ItemID5`, `Amount5`, `ItemID6`, `Amount6`, `ItemID7`, `Amount7`, `ItemID8`, `Amount8`, `ItemID9`, `Amount9`, `ItemID10`, `Amount10`, `Comment` ) SELECT  `ID`, `List_HP`, `List_SP`, `List_HP_Rate`, `List_SP_Rate`, `List_Zeny`, `List_Weapon`, `State`, `Spiritball`, `ItemID1`, `Amount1`, `ItemID2`, `Amount2`, `ItemID3`, `Amount3`, `ItemID4`, `Amount4`, `ItemID5`, `Amount5`, `ItemID6`, `Amount6`, `ItemID7`, `Amount7`, `ItemID8`, `Amount8`, `ItemID9`, `Amount9`, `ItemID10`, `Amount10`, `Comment`  from ragnarok.skill_require_db;

#INSERT INTO eragnarok.skill_tree (  `Upper`, `JobNo`, `Skill_ID`, `MaxLV`, `Skill_ID_Require1`, `Skill_LV_Require1`, `Skill_ID_Require2`, `Skill_LV_Require2`, `Skill_ID_Require3`, `Skill_LV_Require3`, `Skill_ID_Require4`, `Skill_LV_Require4`, `Skill_ID_Require5`, `Skill_LV_Require5`, `Comment` ) SELECT  `Upper`, `JobNo`, `Skill_ID`, `MaxLV`, `Skill_ID_Require1`, `Skill_LV_Require1`, `Skill_ID_Require2`, `Skill_LV_Require2`, `Skill_ID_Require3`, `Skill_LV_Require3`, `Skill_ID_Require4`, `Skill_LV_Require4`, `Skill_ID_Require5`, `Skill_LV_Require5`, `Comment`  from ragnarok.skill_tree;
    
INSERT INTO eragnarok.item_db (  `id`, `name_english`, `name_japanese`, `type`, `price_buy`, `price_sell`, `weight`, `attack`, `defence`, `range`, `slots`, `equip_jobs`, `equip_genders`, `equip_locations`, `weapon_level`, `equip_level`, `view`, `script_use`, `script_equip` ) SELECT  `id`, `Name`, `Name2`, `type`, `Price`, `Sell`, `Weight`, `Atk`, `Def`, `range`, `Slot`, `Job`, `Gender`, `Loc`, `wLV`, `eLV`, `view`, `UseScript`, `EquipScript`  from ragnarok.item_db;

UPDATE item_db SET price_sell = price_buy/2;

INSERT INTO eragnarok.mob_db (`ID`, `Name`, `Name2`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `ADelay`, `aMotion`, `dMotion`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7id`, `Drop7per`, `Drop8id`, `Drop8per`, `MEXP`, `ExpPer`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per`) SELECT `ID`, `Name`, `Name2`, `LV`, `HP`, `SP`, `EXP`, `JEXP`, `Range1`, `ATK1`, `ATK2`, `DEF`, `MDEF`, `STR`, `AGI`, `VIT`, `INT`, `DEX`, `LUK`, `Range2`, `Range3`, `Scale`, `Race`, `Element`, `Mode`, `Speed`, `ADelay`, `aMotion`, `dMotion`, `Drop1id`, `Drop1per`, `Drop2id`, `Drop2per`, `Drop3id`, `Drop3per`, `Drop4id`, `Drop4per`, `Drop5id`, `Drop5per`, `Drop6id`, `Drop6per`, `Drop7id`, `Drop7per`, `Drop8id`, `Drop8per`, `MEXP`, `ExpPer`, `MVP1id`, `MVP1per`, `MVP2id`, `MVP2per`, `MVP3id`, `MVP3per` from ragnarok.mob_db;

DELETE FROM eragnarok.login;

INSERT INTO eragnarok.login ( `account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level` ) SELECT  `account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level`  from ragnarok.login ;
