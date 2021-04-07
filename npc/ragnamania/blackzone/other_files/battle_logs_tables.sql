DROP TABLE IF EXISTS `char_pk`;
CREATE TABLE `char_pk` (
  `char_id` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL default '0',
  `death_count` int(11) NOT NULL default '0',
  `score` int(11) NOT NULL default '0',
  PRIMARY KEY  (`char_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `char_pvp`;
CREATE TABLE `char_pvp` (
  `char_id` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL default '0',
  `death_count` int(11) NOT NULL default '0',
  `score` int(11) NOT NULL default '0',
  PRIMARY KEY  (`char_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `guild_rank`;
CREATE TABLE `guild_rank` (
  `guild_id` int(11) NOT NULL,
  `castle_id` int(11) NOT NULL,
  `capture` int(11) unsigned NOT NULL default '0',
  `emperium` int(11) unsigned NOT NULL default '0',
  `treasure` int(11) unsigned NOT NULL default '0',
  `top_eco` int(11) unsigned NOT NULL default '0',
  `top_def` int(11) unsigned NOT NULL default '0',
  `invest_eco` int(11) unsigned NOT NULL default '0',
  `invest_def` int(11) unsigned NOT NULL default '0',
  `offensive_score` int(11) unsigned NOT NULL default '0',
  `defensive_score` int(11) unsigned NOT NULL default '0',
  `posesion_time` int(11) unsigned NOT NULL default '0',
  `zeny_eco` int(11) unsigned NOT NULL default '0',
  `zeny_def` int(11) unsigned NOT NULL default '0',
  
  `skill_battleorder` int(11) unsigned NOT NULL default '0',
  `skill_regeneration` int(11) unsigned NOT NULL default '0',
  `skill_restore` int(11) unsigned NOT NULL default '0',
  `skill_emergencycall` int(11) unsigned NOT NULL default '0',

  `off_kill` int(11) unsigned NOT NULL default '0',
  `off_death` int(11) unsigned NOT NULL default '0',
  `def_kill` int(11) unsigned NOT NULL default '0',
  `def_death` int(11) unsigned NOT NULL default '0',
  `ext_kill` int(11) unsigned NOT NULL default '0',
  `ext_death` int(11) unsigned NOT NULL default '0',
  `ali_kill` int(11) unsigned NOT NULL default '0',
  `ali_death` int(11) unsigned NOT NULL default '0',
  
  PRIMARY KEY  (`guild_id`,`castle_id`),
  KEY `castle_id` (`castle_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `char_bg`;
CREATE TABLE `char_bg` (
  `char_id` int(11) NOT NULL,
  `top_damage` int(11) NOT NULL default '0',
  `damage_done` int(11) NOT NULL default '0',
  `damage_received` int(11) NOT NULL default '0',
  `skulls` int(11) NOT NULL default '0',
  `ti_wins` int(11) NOT NULL default '0',
  `ti_lost` int(11) NOT NULL default '0',
  `ti_tie` int(11) NOT NULL default '0',
  `eos_flags` int(11) NOT NULL default '0',
  `eos_bases` int(11) NOT NULL default '0',
  `eos_wins` int(11) NOT NULL default '0',
  `eos_lost` int(11) NOT NULL default '0',
  `eos_tie` int(11) NOT NULL default '0',
  `boss_killed` int(11) NOT NULL default '0',
  `boss_damage` int(11) NOT NULL default '0',
  `boss_flags` int(11) NOT NULL default '0',
  `boss_wins` int(11) NOT NULL default '0',
  `boss_lost` int(11) NOT NULL default '0',
  `boss_tie` int(11) NOT NULL default '0',
  `td_kills` int(11) NOT NULL default '0',
  `td_deaths` int(11) NOT NULL default '0',
  `td_wins` int(11) NOT NULL default '0',
  `td_lost` int(11) NOT NULL default '0',
  `td_tie` int(11) NOT NULL default '0',
  `sc_stole` int(11) NOT NULL default '0',
  `sc_captured` int(11) NOT NULL default '0',
  `sc_droped` int(11) NOT NULL default '0',
  `sc_wins` int(11) NOT NULL default '0',
  `sc_lost` int(11) NOT NULL default '0',
  `sc_tie` int(11) NOT NULL default '0',
  `ctf_taken` int(11) NOT NULL default '0',
  `ctf_captured` int(11) NOT NULL default '0',
  `ctf_droped` int(11) NOT NULL default '0',
  `ctf_wins` int(11) NOT NULL default '0',
  `ctf_lost` int(11) NOT NULL default '0',
  `ctf_tie` int(11) NOT NULL default '0',
  `emperium_kill` int(11) NOT NULL default '0',
  `barricade_kill` int(11) NOT NULL default '0',
  `gstone_kill` int(11) NOT NULL default '0',
  `cq_wins` int(11) NOT NULL default '0',
  `cq_lost` int(11) NOT NULL default '0',
  `kill_count` int(11) NOT NULL default '0',
  `death_count` int(11) NOT NULL default '0',
  `win` int(11) NOT NULL default '0',
  `lost` int(11) NOT NULL default '0',
  `tie` int(11) NOT NULL default '0',
  `leader_win` int(11) NOT NULL default '0',
  `leader_lost` int(11) NOT NULL default '0',
  `leader_tie` int(11) NOT NULL default '0',
  `deserter` int(11) NOT NULL default '0',
  `score` int(11) NOT NULL default '0',
  `points` int(11) NOT NULL default '0',
  `sp_heal_potions` int(11) NOT NULL default '0',
  `hp_heal_potions` int(11) NOT NULL default '0',
  `yellow_gemstones` int(11) NOT NULL default '0',
  `red_gemstones` int(11) NOT NULL default '0',
  `blue_gemstones` int(11) NOT NULL default '0',
  `poison_bottles` int(11) NOT NULL default '0',
  `acid_demostration` int(11) NOT NULL default '0',
  `acid_demostration_fail` int(11) NOT NULL default '0',
  `support_skills_used` int(11) NOT NULL default '0',
  `healing_done` int(11) NOT NULL default '0',
  `wrong_support_skills_used` int(11) NOT NULL default '0',
  `wrong_healing_done` int(11) NOT NULL default '0',
  `sp_used` int(11) NOT NULL default '0',
  `zeny_used` int(11) NOT NULL default '0',
  `spiritb_used` int(11) NOT NULL default '0',
  `ammo_used` int(11) NOT NULL default '0',
  `rank_points` int(11) NOT NULL default '0',
  `rank_games` int(11) NOT NULL default '0',
  `ru_captures` int(11) NOT NULL default '0',
  `ru_wins` int(11) NOT NULL default '0',
  `ru_lost` int(11) NOT NULL default '0',
PRIMARY KEY  (`char_id`)
) ENGINE=MyISAM;


DROP TABLE IF EXISTS `char_bg_log`;
CREATE TABLE `char_bg_log` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `killer` varchar(25) NOT NULL,
  `killer_id` int(11) NOT NULL,
  `killed` varchar(25) NOT NULL,
  `killed_id` int(11) NOT NULL,
  `map` varchar(11) NOT NULL default '',
  `skill` int(11) NOT NULL default '0',
  `assists` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `killer_id` (`killer_id`),
  KEY `killed_id` (`killed_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `char_pk_log`;
CREATE TABLE `char_bz_log` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `killer` varchar(25) NOT NULL,
  `killer_id` int(11) NOT NULL,
  `killed` varchar(25) NOT NULL,
  `killed_id` int(11) NOT NULL,
  `map` varchar(11) NOT NULL default '',
  `skill` int(11) NOT NULL default '0',
  `assists` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `killer_id` (`killer_id`),
  KEY `killed_id` (`killed_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `char_pvp_log`;
CREATE TABLE `char_pvp_log` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `killer` varchar(25) NOT NULL,
  `killer_id` int(11) NOT NULL,
  `killed` varchar(25) NOT NULL,
  `killed_id` int(11) NOT NULL,
  `map` varchar(11) NOT NULL default '',
  `skill` int(11) NOT NULL default '0',
  `assists` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `killer_id` (`killer_id`),
  KEY `killed_id` (`killed_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `char_woe_log`;
CREATE TABLE `char_woe_log` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `killer` varchar(25) NOT NULL,
  `killer_id` int(11) NOT NULL,
  `killed` varchar(25) NOT NULL,
  `killed_id` int(11) NOT NULL,
  `map` varchar(11) NOT NULL default '',
  `skill` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `killer_id` (`killer_id`),
  KEY `killed_id` (`killed_id`)
) ENGINE=MyISAM;


DROP TABLE IF EXISTS `char_woe`;
DROP TABLE IF EXISTS `char_wstats`;
CREATE TABLE `char_wstats` (
  `char_id` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL default '0',
  `death_count` int(11) NOT NULL default '0',
  `score` int(11) NOT NULL default '0',
  `top_damage` int(11) NOT NULL default '0',
  `damage_done` int(11) NOT NULL default '0',
  `damage_received` int(11) NOT NULL default '0',
  `emperium_damage` int(11) NOT NULL default '0',
  `guardian_damage` int(11) NOT NULL default '0',
  `barricade_damage` int(11) NOT NULL default '0',
  `gstone_damage` int(11) NOT NULL default '0',
  `emperium_kill` int(11) NOT NULL default '0',
  `guardian_kill` int(11) NOT NULL default '0',
  `barricade_kill` int(11) NOT NULL default '0',
  `gstone_kill` int(11) NOT NULL default '0',
  `sp_heal_potions` int(11) NOT NULL default '0',
  `hp_heal_potions` int(11) NOT NULL default '0',
  `yellow_gemstones` int(11) NOT NULL default '0',
  `red_gemstones` int(11) NOT NULL default '0',
  `blue_gemstones` int(11) NOT NULL default '0',
  `poison_bottles` int(11) NOT NULL default '0',
  `acid_demostration` int(11) NOT NULL default '0',
  `acid_demostration_fail` int(11) NOT NULL default '0',
  `support_skills_used` int(11) NOT NULL default '0',
  `healing_done` int(11) NOT NULL default '0',
  `wrong_support_skills_used` int(11) NOT NULL default '0',
  `wrong_healing_done` int(11) NOT NULL default '0',
  `sp_used` int(11) NOT NULL default '0',
  `zeny_used` int(11) NOT NULL default '0',
  `spiritb_used` int(11) NOT NULL default '0',
  `ammo_used` int(11) NOT NULL default '0',
  PRIMARY KEY  (`char_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `skill_count`;
CREATE TABLE `skill_count` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `id` smallint(11) unsigned NOT NULL default '0',
  `count` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `bg_skill_count`;
CREATE TABLE `bg_skill_count` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `id` smallint(11) unsigned NOT NULL default '0',
  `count` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;



ALTER TABLE `char_bg` ADD COLUMN `dom_bases` int(11) NOT NULL default '0' AFTER `boss_tie`,
ADD COLUMN `dom_off_kills` int(11) NOT NULL default '0' AFTER `dom_bases`,
ADD COLUMN `dom_def_kills` int(11) NOT NULL default '0' AFTER `dom_off_kills`,
ADD COLUMN `dom_wins` int(11) NOT NULL default '0' AFTER `dom_def_kills`,
ADD COLUMN `dom_lost` int(11) NOT NULL default '0' AFTER `dom_wins`,
ADD COLUMN `dom_tie` int(11) NOT NULL default '0' AFTER `dom_lost`;
