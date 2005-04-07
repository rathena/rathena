# phpMyAdmin SQL Dump
# version 2.5.6
# http://www.phpmyadmin.net
#
# Host: localhost
# Generation Time: May 13, 2004 at 11:50 PM
# Server version: 4.0.18
# PHP Version: 4.3.6
# 
# Database : `ragnarok_database`
# 

# --------------------------------------------------------

#
# Table structure for table `abra_db`
#

CREATE TABLE `abra_db` (
  `ID` smallint(6) NOT NULL default '0',
  `Dummy` text NOT NULL,
  `Req_Lvl` smallint(6) NOT NULL default '0',
  `Per` smallint(6) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `attr_fix`
#

CREATE TABLE `attr_fix` (
  `Level` tinyint(4) NOT NULL default '0',
  `None` tinyint(4) NOT NULL default '0',
  `Water` tinyint(4) NOT NULL default '0',
  `Earth` tinyint(4) NOT NULL default '0',
  `Fire` tinyint(4) NOT NULL default '0',
  `Wind` tinyint(4) NOT NULL default '0',
  `Poison` tinyint(4) NOT NULL default '0',
  `Saint` tinyint(4) NOT NULL default '0',
  `Darkness` tinyint(4) NOT NULL default '0',
  `Sense` tinyint(4) NOT NULL default '0',
  `Immortality` tinyint(4) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `cast_db`
#

CREATE TABLE `cast_db` (
  `ID` smallint(6) NOT NULL default '0',
  `Cast_List` mediumint(9) NOT NULL default '0',
  `Delay_List` text NOT NULL,
  `Upkeep_Time` text NOT NULL,
  `Upkeep_Time2` text NOT NULL,
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `castle_db`
#

CREATE TABLE `castle_db` (
  `CastleID` tinyint(4) NOT NULL default '0',
  `map_name` text NOT NULL,
  `castle_name` text NOT NULL,
  `switch_flag` tinyint(4) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `create_arrow_db`
#

CREATE TABLE `create_arrow_db` (
  `SourceID` mediumint(9) NOT NULL default '0',
  `MakeID1` mediumint(9) NOT NULL default '0',
  `MakeNum1` mediumint(9) NOT NULL default '0',
  `MakeID2` mediumint(9) NOT NULL default '0',
  `MakeNum2` mediumint(9) NOT NULL default '0',
  `MakeID3` mediumint(9) NOT NULL default '0',
  `MakeNum3` mediumint(9) NOT NULL default '0',
  `MakeID4` mediumint(9) NOT NULL default '0',
  `MakeNum4` mediumint(9) NOT NULL default '0',
  `MakeID5` mediumint(9) NOT NULL default '0',
  `MakeNum5` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `exp`
#

CREATE TABLE `exp` (
  `EXP1` bigint(9) NOT NULL default '0',
  `EXP2` bigint(9) NOT NULL default '0',
  `EXP3` bigint(9) NOT NULL default '0',
  `EXP4` bigint(9) NOT NULL default '0',
  `EXP5` bigint(9) NOT NULL default '0',
  `EXP6` bigint(9) NOT NULL default '0',
  `EXP7` bigint(9) NOT NULL default '0',
  `EXP8` bigint(9) NOT NULL default '0',
  `EXP9` bigint(9) NOT NULL default '0',
  `EXP10` bigint(9) NOT NULL default '0',
  `EXP11` bigint(9) NOT NULL default '0',
  `EXP12` bigint(9) NOT NULL default '0',
  `EXP13` bigint(9) NOT NULL default '0',
  `EXP14` bigint(9) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `exp_guild`
#

CREATE TABLE `exp_guild` (
  `Level` tinyint(4) NOT NULL default '0',
  `EXP` int(11) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `item_bluebox`
#

CREATE TABLE `item_bluebox` (
  `NameID` mediumint(9) NOT NULL default '0',
  `item_name` text NOT NULL,
  `rate` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `item_cardalbum`
#

CREATE TABLE `item_cardalbum` (
  `NameID` mediumint(9) NOT NULL default '0',
  `item_name` text NOT NULL,
  `rate` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `item_db2`
#

CREATE TABLE `item_db2` (
  `ID` int(11) NOT NULL auto_increment,
  `Name` text NOT NULL,
  `Name2` text NOT NULL,
  `Type` tinyint(4) NOT NULL default '0',
  `Price` mediumint(11) NOT NULL default '0',
  `Sell` mediumint(11) NOT NULL default '0',
  `Weight` mediumint(11) NOT NULL default '0',
  `ATK` mediumint(11) NOT NULL default '0',
  `DEF` mediumint(11) NOT NULL default '0',
  `Range` mediumint(11) NOT NULL default '0',
  `Slot` mediumint(11) NOT NULL default '0',
  `Job` int(20) NOT NULL default '0',
  `Gender` smallint(11) NOT NULL default '0',
  `Loc` smallint(11) NOT NULL default '0',
  `wLV` mediumint(11) NOT NULL default '0',
  `eLV` mediumint(11) NOT NULL default '0',
  `View` int(11) NOT NULL default '0',
  `UseScript` text NOT NULL,
  `EquipScript` text NOT NULL,
  `Comment` text,
  UNIQUE KEY `ID` (`ID`)
) TYPE=MyISAM PACK_KEYS=0 AUTO_INCREMENT=1 ;

# --------------------------------------------------------

#
# Table structure for table `item_giftbox`
#

CREATE TABLE `item_giftbox` (
  `NameID` mediumint(9) NOT NULL default '0',
  `item_name` text NOT NULL,
  `rate` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `item_scroll`
#

CREATE TABLE `item_scroll` (
  `NameID` mediumint(9) NOT NULL default '0',
  `item_name` text NOT NULL,
  `rate` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `item_violetbox`
#

CREATE TABLE `item_violetbox` (
  `NameID` mediumint(9) NOT NULL default '0',
  `item_name` text NOT NULL,
  `rate` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `job_db1`
#

CREATE TABLE `job_db1` (
  `Class_ID` tinyint(4) NOT NULL default '0',
  `Weight` mediumint(9) NOT NULL default '0',
  `HP` smallint(6) NOT NULL default '0',
  `HP2` smallint(6) NOT NULL default '0',
  `SP` smallint(6) NOT NULL default '0',
  `Empty` smallint(6) NOT NULL default '0',
  `Dagger` smallint(6) NOT NULL default '0',
  `Sword` smallint(6) NOT NULL default '0',
  `Two_Handed_Sword` smallint(6) NOT NULL default '0',
  `Spear` smallint(6) NOT NULL default '0',
  `Two_Handed_Spear` smallint(6) NOT NULL default '0',
  `Axe` smallint(6) NOT NULL default '0',
  `Two_Handed_Axe` smallint(6) NOT NULL default '0',
  `Rod` smallint(6) NOT NULL default '0',
  `Club` smallint(6) NOT NULL default '0',
  `Stick` smallint(6) NOT NULL default '0',
  `Bow` smallint(6) NOT NULL default '0',
  `Fist` smallint(6) NOT NULL default '0',
  `Musical` smallint(6) NOT NULL default '0',
  `Whip` smallint(6) NOT NULL default '0',
  `Book` smallint(6) NOT NULL default '0',
  `Katar` smallint(6) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `mob_boss`
#

CREATE TABLE `mob_boss` (
  `MobID` mediumint(9) NOT NULL default '0',
  `MobName` text NOT NULL,
  `Rate` int(11) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `mob_branch`
#

CREATE TABLE `mob_branch` (
  `MobID` mediumint(9) NOT NULL default '0',
  `MobName` text NOT NULL,
  `Rate` int(11) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `mob_db2`
#

CREATE TABLE `mob_db2` (
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
  `Drop9id` mediumint(9) NOT NULL default '0',
  `Drop9per` mediumint(9) NOT NULL default '0',
  `DropCardid` mediumint(9) NOT NULL default '0',
  `DropCardper` mediumint(9) NOT NULL default '0',
  `MEXP` mediumint(9) NOT NULL default '0',
  `ExpPer` mediumint(9) NOT NULL default '0',
  `MVP1id` mediumint(9) NOT NULL default '0',
  `MVP1per` mediumint(9) NOT NULL default '0',
  `MVP2id` mediumint(9) NOT NULL default '0',
  `MVP2per` mediumint(9) NOT NULL default '0',
  `MVP3id` mediumint(9) NOT NULL default '0',
  `MVP3per` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `mob_poring`
#

CREATE TABLE `mob_poring` (
  `MobID` smallint(6) NOT NULL default '0',
  `MobName` text NOT NULL,
  `Rate` mediumint(9) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `mob_skill_db`
#

CREATE TABLE `mob_skill_db` (
  `Mob_ID` smallint(6) NOT NULL default '0',
  `Dummy` text NOT NULL,
  `State` text NOT NULL,
  `Skill_ID` smallint(6) NOT NULL default '0',
  `Skill_LV` tinyint(4) NOT NULL default '0',
  `Use_Rate` smallint(6) NOT NULL default '0',
  `Cast_Time` smallint(6) NOT NULL default '0',
  `Delay` smallint(6) NOT NULL default '0',
  `Disturbance` text NOT NULL,
  `Target` text NOT NULL,
  `Condition_Type` text NOT NULL,
  `Condition_Value` smallint(6) NOT NULL default '0',
  `Value1` mediumint(9) NOT NULL default '0',
  `Value2` mediumint(9) NOT NULL default '0',
  `Value3` mediumint(9) NOT NULL default '0',
  `Value4` mediumint(9) NOT NULL default '0',
  `Value5` mediumint(9) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `pet_db`
#

CREATE TABLE `pet_db` (
  `MobID` smallint(6) NOT NULL default '0',
  `Name` text NOT NULL,
  `JName` text NOT NULL,
  `ItemID` smallint(6) NOT NULL default '0',
  `EggID` smallint(6) NOT NULL default '0',
  `AcceID` smallint(6) NOT NULL default '0',
  `FoodID` smallint(6) NOT NULL default '0',
  `Fullness` smallint(6) NOT NULL default '0',
  `HungryDeray` smallint(6) NOT NULL default '0',
  `R_Hungry` smallint(6) NOT NULL default '0',
  `R_Full` smallint(6) NOT NULL default '0',
  `Intimate` smallint(6) NOT NULL default '0',
  `Die` smallint(6) NOT NULL default '0',
  `Capture` smallint(6) NOT NULL default '0',
  `Speed` smallint(6) NOT NULL default '0',
  `S_Performance` smallint(6) NOT NULL default '0',
  `Talk_Convert_Class` smallint(6) NOT NULL default '0',
  `Attack_Rate` smallint(6) NOT NULL default '0',
  `Defence_Attack_Rate` smallint(6) NOT NULL default '0',
  `Change_Target_Rate` smallint(6) NOT NULL default '0',
  `Pet_Script` text NOT NULL,
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `produce_db`
#

CREATE TABLE `produce_db` (
  `ID` smallint(6) NOT NULL default '0',
  `ItemLV` tinyint(4) NOT NULL default '0',
  `RequireSkill` smallint(6) NOT NULL default '0',
  `MaterialID1` smallint(6) NOT NULL default '0',
  `MaterialAmount1` smallint(6) NOT NULL default '0',
  `MaterialID2` smallint(6) NOT NULL default '0',
  `MaterialAmount2` smallint(6) NOT NULL default '0',
  `MaterialID3` smallint(6) NOT NULL default '0',
  `MaterialAmount3` smallint(6) NOT NULL default '0',
  `MaterialID4` smallint(6) NOT NULL default '0',
  `MaterialAmount4` smallint(6) NOT NULL default '0',
  `MaterialID5` smallint(6) NOT NULL default '0',
  `MaterialAmount5` smallint(6) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `refine_db`
#

CREATE TABLE `refine_db` (
  `Refine_Bonus` tinyint(4) NOT NULL default '0',
  `Danger_Bonus` tinyint(4) NOT NULL default '0',
  `Safe_Limit` tinyint(4) NOT NULL default '0',
  `RefineChance1` tinyint(4) NOT NULL default '0',
  `RefineChance2` tinyint(4) NOT NULL default '0',
  `RefineChance3` tinyint(4) NOT NULL default '0',
  `RefineChance4` tinyint(4) NOT NULL default '0',
  `RefineChance5` tinyint(4) NOT NULL default '0',
  `RefineChance6` tinyint(4) NOT NULL default '0',
  `RefineChance7` tinyint(4) NOT NULL default '0',
  `RefineChance8` tinyint(4) NOT NULL default '0',
  `RefineChance9` tinyint(4) NOT NULL default '0',
  `RefineChance10` tinyint(4) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `size_fix`
#

CREATE TABLE `size_fix` (
  `Element` tinyint(4) NOT NULL default '0',
  `Dagger` tinyint(4) NOT NULL default '0',
  `Sword` tinyint(4) NOT NULL default '0',
  `Two_Handed_Sword` tinyint(4) NOT NULL default '0',
  `Spear` tinyint(4) NOT NULL default '0',
  `Two_Handed_Spear` tinyint(4) NOT NULL default '0',
  `Axe` tinyint(4) NOT NULL default '0',
  `Two_Handed_Axe` tinyint(4) NOT NULL default '0',
  `Club` tinyint(4) NOT NULL default '0',
  `Whip` tinyint(4) NOT NULL default '0',
  `Stick` tinyint(4) NOT NULL default '0',
  `Bow` tinyint(4) NOT NULL default '0',
  `Fist` tinyint(4) NOT NULL default '0',
  `Musical` tinyint(4) NOT NULL default '0',
  `Rod` tinyint(4) NOT NULL default '0',
  `Book` tinyint(4) NOT NULL default '0',
  `Katar` tinyint(4) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `skill_db`
#

CREATE TABLE `skill_db` (
  `ID` smallint(6) NOT NULL default '0',
  `Range` smallint(6) NOT NULL default '0',
  `Hit` smallint(6) NOT NULL default '0',
  `inf` smallint(6) NOT NULL default '0',
  `nk` smallint(6) NOT NULL default '0',
  `max` smallint(6) NOT NULL default '0',
  `list_num` smallint(6) NOT NULL default '0',
  `castcancel` text NOT NULL,
  `cast_defence_rate` smallint(6) NOT NULL default '0',
  `inf2` smallint(6) NOT NULL default '0',
  `maxcount` smallint(6) NOT NULL default '0',
  `skill_type` text NOT NULL,
  `blow_count` smallint(6) NOT NULL default '0',
  `Comment` text
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `skill_require_db`
#

CREATE TABLE `skill_require_db` (
  `ID` smallint(6) NOT NULL default '0',
  `List_HP` text NOT NULL,
  `List_SP` text NOT NULL,
  `List_HP_Rate` text NOT NULL,
  `List_SP_Rate` text NOT NULL,
  `List_Zeny` text NOT NULL,
  `List_Weapon` text NOT NULL,
  `State` text NOT NULL,
  `Spiritball` tinyint(4) NOT NULL default '0',
  `ItemID1` mediumint(9) NOT NULL default '0',
  `Amount1` tinyint(4) NOT NULL default '0',
  `ItemID2` mediumint(9) NOT NULL default '0',
  `Amount2` tinyint(4) NOT NULL default '0',
  `ItemID3` mediumint(9) NOT NULL default '0',
  `Amount3` tinyint(4) NOT NULL default '0',
  `ItemID4` mediumint(9) NOT NULL default '0',
  `Amount4` tinyint(4) NOT NULL default '0',
  `ItemID5` mediumint(9) NOT NULL default '0',
  `Amount5` tinyint(4) NOT NULL default '0',
  `ItemID6` mediumint(9) NOT NULL default '0',
  `Amount6` tinyint(4) NOT NULL default '0',
  `ItemID7` mediumint(9) NOT NULL default '0',
  `Amount7` tinyint(4) NOT NULL default '0',
  `ItemID8` mediumint(9) NOT NULL default '0',
  `Amount8` tinyint(4) NOT NULL default '0',
  `ItemID9` mediumint(9) NOT NULL default '0',
  `Amount9` tinyint(4) NOT NULL default '0',
  `ItemID10` mediumint(9) NOT NULL default '0',
  `Amount10` tinyint(4) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `skill_tree`
#

CREATE TABLE `skill_tree` (
  `Upper` tinyint(4) NOT NULL default '0',
  `JobNo` tinyint(4) NOT NULL default '0',
  `Skill_ID` smallint(6) NOT NULL default '0',
  `MaxLV` tinyint(4) NOT NULL default '0',
  `Skill_ID_Require1` smallint(6) NOT NULL default '0',
  `Skill_LV_Require1` tinyint(4) NOT NULL default '0',
  `Skill_ID_Require2` smallint(6) NOT NULL default '0',
  `Skill_LV_Require2` tinyint(4) NOT NULL default '0',
  `Skill_ID_Require3` smallint(6) NOT NULL default '0',
  `Skill_LV_Require3` tinyint(4) NOT NULL default '0',
  `Skill_ID_Require4` smallint(6) NOT NULL default '0',
  `Skill_LV_Require4` tinyint(4) NOT NULL default '0',
  `Skill_ID_Require5` smallint(6) NOT NULL default '0',
  `Skill_LV_Require5` tinyint(4) NOT NULL default '0',
  `Comment` text NOT NULL
) TYPE=MyISAM;
    