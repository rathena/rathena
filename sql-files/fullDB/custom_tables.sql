-- phpMyAdmin SQL Dump
-- version 5.0.4
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Apr 02, 2021 at 04:11 AM
-- Server version: 10.1.48-MariaDB
-- PHP Version: 7.2.30

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `ragadmin_pts`
--

-- --------------------------------------------------------

--
-- Table structure for table `bg_skill_count`
--

DROP TABLE IF EXISTS `bg_skill_count`;
CREATE TABLE `bg_skill_count` (
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `char_bg`
--

DROP TABLE IF EXISTS `char_bg`;
CREATE TABLE `char_bg` (
  `char_id` int(11) NOT NULL,
  `top_damage` int(11) NOT NULL DEFAULT '0',
  `damage_done` int(11) NOT NULL DEFAULT '0',
  `damage_received` int(11) NOT NULL DEFAULT '0',
  `skulls` int(11) NOT NULL DEFAULT '0',
  `ti_wins` int(11) NOT NULL DEFAULT '0',
  `ti_lost` int(11) NOT NULL DEFAULT '0',
  `ti_tie` int(11) NOT NULL DEFAULT '0',
  `eos_flags` int(11) NOT NULL DEFAULT '0',
  `eos_bases` int(11) NOT NULL DEFAULT '0',
  `eos_wins` int(11) NOT NULL DEFAULT '0',
  `eos_lost` int(11) NOT NULL DEFAULT '0',
  `eos_tie` int(11) NOT NULL DEFAULT '0',
  `boss_killed` int(11) NOT NULL DEFAULT '0',
  `boss_damage` int(11) NOT NULL DEFAULT '0',
  `boss_flags` int(11) NOT NULL DEFAULT '0',
  `boss_wins` int(11) NOT NULL DEFAULT '0',
  `boss_lost` int(11) NOT NULL DEFAULT '0',
  `boss_tie` int(11) NOT NULL DEFAULT '0',
  `dom_bases` int(11) NOT NULL DEFAULT '0',
  `dom_off_kills` int(11) NOT NULL DEFAULT '0',
  `dom_def_kills` int(11) NOT NULL DEFAULT '0',
  `dom_wins` int(11) NOT NULL DEFAULT '0',
  `dom_lost` int(11) NOT NULL DEFAULT '0',
  `dom_tie` int(11) NOT NULL DEFAULT '0',
  `td_kills` int(11) NOT NULL DEFAULT '0',
  `td_deaths` int(11) NOT NULL DEFAULT '0',
  `td_wins` int(11) NOT NULL DEFAULT '0',
  `td_lost` int(11) NOT NULL DEFAULT '0',
  `td_tie` int(11) NOT NULL DEFAULT '0',
  `sc_stole` int(11) NOT NULL DEFAULT '0',
  `sc_captured` int(11) NOT NULL DEFAULT '0',
  `sc_droped` int(11) NOT NULL DEFAULT '0',
  `sc_wins` int(11) NOT NULL DEFAULT '0',
  `sc_lost` int(11) NOT NULL DEFAULT '0',
  `sc_tie` int(11) NOT NULL DEFAULT '0',
  `ctf_taken` int(11) NOT NULL DEFAULT '0',
  `ctf_captured` int(11) NOT NULL DEFAULT '0',
  `ctf_droped` int(11) NOT NULL DEFAULT '0',
  `ctf_wins` int(11) NOT NULL DEFAULT '0',
  `ctf_lost` int(11) NOT NULL DEFAULT '0',
  `ctf_tie` int(11) NOT NULL DEFAULT '0',
  `emperium_kill` int(11) NOT NULL DEFAULT '0',
  `barricade_kill` int(11) NOT NULL DEFAULT '0',
  `gstone_kill` int(11) NOT NULL DEFAULT '0',
  `cq_wins` int(11) NOT NULL DEFAULT '0',
  `cq_lost` int(11) NOT NULL DEFAULT '0',
  `kill_count` int(11) NOT NULL DEFAULT '0',
  `death_count` int(11) NOT NULL DEFAULT '0',
  `win` int(11) NOT NULL DEFAULT '0',
  `lost` int(11) NOT NULL DEFAULT '0',
  `tie` int(11) NOT NULL DEFAULT '0',
  `leader_win` int(11) NOT NULL DEFAULT '0',
  `leader_lost` int(11) NOT NULL DEFAULT '0',
  `leader_tie` int(11) NOT NULL DEFAULT '0',
  `deserter` int(11) NOT NULL DEFAULT '0',
  `score` int(11) NOT NULL DEFAULT '0',
  `points` int(11) NOT NULL DEFAULT '0',
  `sp_heal_potions` int(11) NOT NULL DEFAULT '0',
  `hp_heal_potions` int(11) NOT NULL DEFAULT '0',
  `yellow_gemstones` int(11) NOT NULL DEFAULT '0',
  `red_gemstones` int(11) NOT NULL DEFAULT '0',
  `blue_gemstones` int(11) NOT NULL DEFAULT '0',
  `poison_bottles` int(11) NOT NULL DEFAULT '0',
  `acid_demostration` int(11) NOT NULL DEFAULT '0',
  `acid_demostration_fail` int(11) NOT NULL DEFAULT '0',
  `support_skills_used` int(11) NOT NULL DEFAULT '0',
  `healing_done` int(11) NOT NULL DEFAULT '0',
  `wrong_support_skills_used` int(11) NOT NULL DEFAULT '0',
  `wrong_healing_done` int(11) NOT NULL DEFAULT '0',
  `sp_used` int(11) NOT NULL DEFAULT '0',
  `zeny_used` int(11) NOT NULL DEFAULT '0',
  `spiritb_used` int(11) NOT NULL DEFAULT '0',
  `ammo_used` int(11) NOT NULL DEFAULT '0',
  `rank_points` int(11) NOT NULL DEFAULT '0',
  `rank_games` int(11) NOT NULL DEFAULT '0',
  `ru_captures` int(11) NOT NULL DEFAULT '0',
  `ru_wins` int(11) NOT NULL DEFAULT '0',
  `ru_lost` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `char_bg_log`
--

DROP TABLE IF EXISTS `char_bg_log`;
CREATE TABLE `char_bg_log` (
  `id` int(11) NOT NULL,
  `time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `killer` varchar(25) NOT NULL,
  `killer_id` int(11) NOT NULL,
  `killed` varchar(25) NOT NULL,
  `killed_id` int(11) NOT NULL,
  `map` varchar(11) NOT NULL DEFAULT '',
  `skill` int(11) NOT NULL DEFAULT '0',
  `assists` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `char_pk`
--

DROP TABLE IF EXISTS `char_pk`;
CREATE TABLE `char_pk` (
  `char_id` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL DEFAULT '0',
  `death_count` int(11) NOT NULL DEFAULT '0',
  `score` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `char_pk_log`
--

DROP TABLE IF EXISTS `char_pk_log`;
CREATE TABLE `char_pk_log` (
  `id` int(11) NOT NULL,
  `time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `killer` varchar(25) NOT NULL,
  `killer_id` int(11) NOT NULL,
  `killed` varchar(25) NOT NULL,
  `killed_id` int(11) NOT NULL,
  `map` varchar(11) NOT NULL DEFAULT '',
  `skill` int(11) NOT NULL DEFAULT '0',
  `assists` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `char_pvp`
--

DROP TABLE IF EXISTS `char_pvp`;
CREATE TABLE `char_pvp` (
  `char_id` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL DEFAULT '0',
  `death_count` int(11) NOT NULL DEFAULT '0',
  `score` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `char_pvp_log`
--

DROP TABLE IF EXISTS `char_pvp_log`;
CREATE TABLE `char_pvp_log` (
  `id` int(11) NOT NULL,
  `time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `killer` varchar(25) NOT NULL,
  `killer_id` int(11) NOT NULL,
  `killed` varchar(25) NOT NULL,
  `killed_id` int(11) NOT NULL,
  `map` varchar(11) NOT NULL DEFAULT '',
  `skill` int(11) NOT NULL DEFAULT '0',
  `assists` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `char_wstats`
--

DROP TABLE IF EXISTS `char_wstats`;
CREATE TABLE `char_wstats` (
  `char_id` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL DEFAULT '0',
  `death_count` int(11) NOT NULL DEFAULT '0',
  `score` int(11) NOT NULL DEFAULT '0',
  `top_damage` int(11) NOT NULL DEFAULT '0',
  `damage_done` int(11) NOT NULL DEFAULT '0',
  `damage_received` int(11) NOT NULL DEFAULT '0',
  `emperium_damage` int(11) NOT NULL DEFAULT '0',
  `guardian_damage` int(11) NOT NULL DEFAULT '0',
  `barricade_damage` int(11) NOT NULL DEFAULT '0',
  `gstone_damage` int(11) NOT NULL DEFAULT '0',
  `emperium_kill` int(11) NOT NULL DEFAULT '0',
  `guardian_kill` int(11) NOT NULL DEFAULT '0',
  `barricade_kill` int(11) NOT NULL DEFAULT '0',
  `gstone_kill` int(11) NOT NULL DEFAULT '0',
  `sp_heal_potions` int(11) NOT NULL DEFAULT '0',
  `hp_heal_potions` int(11) NOT NULL DEFAULT '0',
  `yellow_gemstones` int(11) NOT NULL DEFAULT '0',
  `red_gemstones` int(11) NOT NULL DEFAULT '0',
  `blue_gemstones` int(11) NOT NULL DEFAULT '0',
  `poison_bottles` int(11) NOT NULL DEFAULT '0',
  `acid_demostration` int(11) NOT NULL DEFAULT '0',
  `acid_demostration_fail` int(11) NOT NULL DEFAULT '0',
  `support_skills_used` int(11) NOT NULL DEFAULT '0',
  `healing_done` int(11) NOT NULL DEFAULT '0',
  `wrong_support_skills_used` int(11) NOT NULL DEFAULT '0',
  `wrong_healing_done` int(11) NOT NULL DEFAULT '0',
  `sp_used` int(11) NOT NULL DEFAULT '0',
  `zeny_used` int(11) NOT NULL DEFAULT '0',
  `spiritb_used` int(11) NOT NULL DEFAULT '0',
  `ammo_used` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `season_pvp_rank`
--

DROP TABLE IF EXISTS `season_pvp_rank`;
CREATE TABLE `season_pvp_rank` (
  `char_id` int(11) NOT NULL,
  `score` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL,
  `death_count` int(11) NOT NULL,
  `week` tinyint(4) NOT NULL,
  `season` smallint(6) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild04_char_0`
--

DROP TABLE IF EXISTS `stor_ars_fild04_char_0`;
CREATE TABLE `stor_ars_fild04_char_0` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild04_char_1`
--

DROP TABLE IF EXISTS `stor_ars_fild04_char_1`;
CREATE TABLE `stor_ars_fild04_char_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild04_char_2`
--

DROP TABLE IF EXISTS `stor_ars_fild04_char_2`;
CREATE TABLE `stor_ars_fild04_char_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild04_char_3`
--

DROP TABLE IF EXISTS `stor_ars_fild04_char_3`;
CREATE TABLE `stor_ars_fild04_char_3` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild04_char_4`
--

DROP TABLE IF EXISTS `stor_ars_fild04_char_4`;
CREATE TABLE `stor_ars_fild04_char_4` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild04_char_5`
--

DROP TABLE IF EXISTS `stor_ars_fild04_char_5`;
CREATE TABLE `stor_ars_fild04_char_5` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild09_char_0`
--

DROP TABLE IF EXISTS `stor_ars_fild09_char_0`;
CREATE TABLE `stor_ars_fild09_char_0` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild09_char_1`
--

DROP TABLE IF EXISTS `stor_ars_fild09_char_1`;
CREATE TABLE `stor_ars_fild09_char_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild09_char_2`
--

DROP TABLE IF EXISTS `stor_ars_fild09_char_2`;
CREATE TABLE `stor_ars_fild09_char_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild09_char_3`
--

DROP TABLE IF EXISTS `stor_ars_fild09_char_3`;
CREATE TABLE `stor_ars_fild09_char_3` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild09_char_4`
--

DROP TABLE IF EXISTS `stor_ars_fild09_char_4`;
CREATE TABLE `stor_ars_fild09_char_4` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild09_char_5`
--

DROP TABLE IF EXISTS `stor_ars_fild09_char_5`;
CREATE TABLE `stor_ars_fild09_char_5` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild10_char_0`
--

DROP TABLE IF EXISTS `stor_ars_fild10_char_0`;
CREATE TABLE `stor_ars_fild10_char_0` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild10_char_1`
--

DROP TABLE IF EXISTS `stor_ars_fild10_char_1`;
CREATE TABLE `stor_ars_fild10_char_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild10_char_2`
--

DROP TABLE IF EXISTS `stor_ars_fild10_char_2`;
CREATE TABLE `stor_ars_fild10_char_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild10_char_3`
--

DROP TABLE IF EXISTS `stor_ars_fild10_char_3`;
CREATE TABLE `stor_ars_fild10_char_3` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild10_char_4`
--

DROP TABLE IF EXISTS `stor_ars_fild10_char_4`;
CREATE TABLE `stor_ars_fild10_char_4` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild10_char_5`
--

DROP TABLE IF EXISTS `stor_ars_fild10_char_5`;
CREATE TABLE `stor_ars_fild10_char_5` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild13_char_0`
--

DROP TABLE IF EXISTS `stor_ars_fild13_char_0`;
CREATE TABLE `stor_ars_fild13_char_0` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild13_char_1`
--

DROP TABLE IF EXISTS `stor_ars_fild13_char_1`;
CREATE TABLE `stor_ars_fild13_char_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild13_char_2`
--

DROP TABLE IF EXISTS `stor_ars_fild13_char_2`;
CREATE TABLE `stor_ars_fild13_char_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild13_char_3`
--

DROP TABLE IF EXISTS `stor_ars_fild13_char_3`;
CREATE TABLE `stor_ars_fild13_char_3` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild13_char_4`
--

DROP TABLE IF EXISTS `stor_ars_fild13_char_4`;
CREATE TABLE `stor_ars_fild13_char_4` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild13_char_5`
--

DROP TABLE IF EXISTS `stor_ars_fild13_char_5`;
CREATE TABLE `stor_ars_fild13_char_5` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild16_char_0`
--

DROP TABLE IF EXISTS `stor_ars_fild16_char_0`;
CREATE TABLE `stor_ars_fild16_char_0` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild16_char_1`
--

DROP TABLE IF EXISTS `stor_ars_fild16_char_1`;
CREATE TABLE `stor_ars_fild16_char_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild16_char_2`
--

DROP TABLE IF EXISTS `stor_ars_fild16_char_2`;
CREATE TABLE `stor_ars_fild16_char_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild16_char_3`
--

DROP TABLE IF EXISTS `stor_ars_fild16_char_3`;
CREATE TABLE `stor_ars_fild16_char_3` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild16_char_4`
--

DROP TABLE IF EXISTS `stor_ars_fild16_char_4`;
CREATE TABLE `stor_ars_fild16_char_4` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild16_char_5`
--

DROP TABLE IF EXISTS `stor_ars_fild16_char_5`;
CREATE TABLE `stor_ars_fild16_char_5` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild26_char_0`
--

DROP TABLE IF EXISTS `stor_ars_fild26_char_0`;
CREATE TABLE `stor_ars_fild26_char_0` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild26_char_1`
--

DROP TABLE IF EXISTS `stor_ars_fild26_char_1`;
CREATE TABLE `stor_ars_fild26_char_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild26_char_2`
--

DROP TABLE IF EXISTS `stor_ars_fild26_char_2`;
CREATE TABLE `stor_ars_fild26_char_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild26_char_3`
--

DROP TABLE IF EXISTS `stor_ars_fild26_char_3`;
CREATE TABLE `stor_ars_fild26_char_3` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild26_char_4`
--

DROP TABLE IF EXISTS `stor_ars_fild26_char_4`;
CREATE TABLE `stor_ars_fild26_char_4` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild26_char_5`
--

DROP TABLE IF EXISTS `stor_ars_fild26_char_5`;
CREATE TABLE `stor_ars_fild26_char_5` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild27_char_0`
--

DROP TABLE IF EXISTS `stor_ars_fild27_char_0`;
CREATE TABLE `stor_ars_fild27_char_0` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild27_char_1`
--

DROP TABLE IF EXISTS `stor_ars_fild27_char_1`;
CREATE TABLE `stor_ars_fild27_char_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild27_char_2`
--

DROP TABLE IF EXISTS `stor_ars_fild27_char_2`;
CREATE TABLE `stor_ars_fild27_char_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild27_char_3`
--

DROP TABLE IF EXISTS `stor_ars_fild27_char_3`;
CREATE TABLE `stor_ars_fild27_char_3` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild27_char_4`
--

DROP TABLE IF EXISTS `stor_ars_fild27_char_4`;
CREATE TABLE `stor_ars_fild27_char_4` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_ars_fild27_char_5`
--

DROP TABLE IF EXISTS `stor_ars_fild27_char_5`;
CREATE TABLE `stor_ars_fild27_char_5` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `stor_vip`
--

DROP TABLE IF EXISTS `stor_vip`;
CREATE TABLE `stor_vip` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `bg_skill_count`
--
ALTER TABLE `bg_skill_count`
  ADD PRIMARY KEY (`char_id`,`id`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `char_bg`
--
ALTER TABLE `char_bg`
  ADD PRIMARY KEY (`char_id`);

--
-- Indexes for table `char_bg_log`
--
ALTER TABLE `char_bg_log`
  ADD PRIMARY KEY (`id`),
  ADD KEY `killer_id` (`killer_id`),
  ADD KEY `killed_id` (`killed_id`);

--
-- Indexes for table `char_pk`
--
ALTER TABLE `char_pk`
  ADD PRIMARY KEY (`char_id`);

--
-- Indexes for table `char_pk_log`
--
ALTER TABLE `char_pk_log`
  ADD PRIMARY KEY (`id`),
  ADD KEY `killer_id` (`killer_id`),
  ADD KEY `killed_id` (`killed_id`);

--
-- Indexes for table `char_pvp`
--
ALTER TABLE `char_pvp`
  ADD PRIMARY KEY (`char_id`);

--
-- Indexes for table `char_pvp_log`
--
ALTER TABLE `char_pvp_log`
  ADD PRIMARY KEY (`id`),
  ADD KEY `killer_id` (`killer_id`),
  ADD KEY `killed_id` (`killed_id`);

--
-- Indexes for table `char_wstats`
--
ALTER TABLE `char_wstats`
  ADD PRIMARY KEY (`char_id`);

--
-- Indexes for table `season_pvp_rank`
--
ALTER TABLE `season_pvp_rank`
  ADD UNIQUE KEY `week` (`week`,`season`);

--
-- Indexes for table `stor_ars_fild04_char_0`
--
ALTER TABLE `stor_ars_fild04_char_0`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild04_char_1`
--
ALTER TABLE `stor_ars_fild04_char_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild04_char_2`
--
ALTER TABLE `stor_ars_fild04_char_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild04_char_3`
--
ALTER TABLE `stor_ars_fild04_char_3`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild04_char_4`
--
ALTER TABLE `stor_ars_fild04_char_4`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild04_char_5`
--
ALTER TABLE `stor_ars_fild04_char_5`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild09_char_0`
--
ALTER TABLE `stor_ars_fild09_char_0`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild09_char_1`
--
ALTER TABLE `stor_ars_fild09_char_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild09_char_2`
--
ALTER TABLE `stor_ars_fild09_char_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild09_char_3`
--
ALTER TABLE `stor_ars_fild09_char_3`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild09_char_4`
--
ALTER TABLE `stor_ars_fild09_char_4`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild09_char_5`
--
ALTER TABLE `stor_ars_fild09_char_5`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild10_char_0`
--
ALTER TABLE `stor_ars_fild10_char_0`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild10_char_1`
--
ALTER TABLE `stor_ars_fild10_char_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild10_char_2`
--
ALTER TABLE `stor_ars_fild10_char_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild10_char_3`
--
ALTER TABLE `stor_ars_fild10_char_3`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild10_char_4`
--
ALTER TABLE `stor_ars_fild10_char_4`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild10_char_5`
--
ALTER TABLE `stor_ars_fild10_char_5`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild13_char_0`
--
ALTER TABLE `stor_ars_fild13_char_0`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild13_char_1`
--
ALTER TABLE `stor_ars_fild13_char_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild13_char_2`
--
ALTER TABLE `stor_ars_fild13_char_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild13_char_3`
--
ALTER TABLE `stor_ars_fild13_char_3`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild13_char_4`
--
ALTER TABLE `stor_ars_fild13_char_4`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild13_char_5`
--
ALTER TABLE `stor_ars_fild13_char_5`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild16_char_0`
--
ALTER TABLE `stor_ars_fild16_char_0`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild16_char_1`
--
ALTER TABLE `stor_ars_fild16_char_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild16_char_2`
--
ALTER TABLE `stor_ars_fild16_char_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild16_char_3`
--
ALTER TABLE `stor_ars_fild16_char_3`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild16_char_4`
--
ALTER TABLE `stor_ars_fild16_char_4`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild16_char_5`
--
ALTER TABLE `stor_ars_fild16_char_5`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild26_char_0`
--
ALTER TABLE `stor_ars_fild26_char_0`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild26_char_1`
--
ALTER TABLE `stor_ars_fild26_char_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild26_char_2`
--
ALTER TABLE `stor_ars_fild26_char_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild26_char_3`
--
ALTER TABLE `stor_ars_fild26_char_3`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild26_char_4`
--
ALTER TABLE `stor_ars_fild26_char_4`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild26_char_5`
--
ALTER TABLE `stor_ars_fild26_char_5`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild27_char_0`
--
ALTER TABLE `stor_ars_fild27_char_0`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild27_char_1`
--
ALTER TABLE `stor_ars_fild27_char_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild27_char_2`
--
ALTER TABLE `stor_ars_fild27_char_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild27_char_3`
--
ALTER TABLE `stor_ars_fild27_char_3`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild27_char_4`
--
ALTER TABLE `stor_ars_fild27_char_4`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_ars_fild27_char_5`
--
ALTER TABLE `stor_ars_fild27_char_5`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `stor_vip`
--
ALTER TABLE `stor_vip`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `char_bg_log`
--
ALTER TABLE `char_bg_log`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `char_pk_log`
--
ALTER TABLE `char_pk_log`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `char_pvp_log`
--
ALTER TABLE `char_pvp_log`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `stor_vip`
--
ALTER TABLE `stor_vip`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
