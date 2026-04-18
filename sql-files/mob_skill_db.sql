#
# Table structure for table `mob_skill_db`
#

DROP TABLE IF EXISTS `mob_skill_db`;
CREATE TABLE IF NOT EXISTS `mob_skill_db` (
  `MOB_ID` smallint(6) NOT NULL,
  `INFO` text NOT NULL,
  `STATE` text NOT NULL,
  `SKILL_ID` smallint(6) NOT NULL,
  `SKILL_LV` tinyint(4) NOT NULL,
  `RATE` smallint(4) NOT NULL,
  `CASTTIME` mediumint(9) NOT NULL,
  `DELAY` int(9) NOT NULL,
  `CANCELABLE` text NOT NULL,
  `TARGET` text NOT NULL,
  `CONDITION` text NOT NULL,
  `CONDITION_VALUE` text,
  `VAL1` mediumint(9) DEFAULT NULL,
  `VAL2` mediumint(9) DEFAULT NULL,
  `VAL3` mediumint(9) DEFAULT NULL,
  `VAL4` mediumint(9) DEFAULT NULL,
  `VAL5` mediumint(9) DEFAULT NULL,
  `EMOTION` text,
  `CHAT` text
) ENGINE=MyISAM;

# Mob Skill Database
# Based on Aegis Episode 11.3
#
# Structure of Database:
#REPLACE INTO `mob_skill_db` VALUES ( MobID,'Dummy value (info only)','State',SkillID,SkillLv,Rate,CastTime,Delay,'Cancelable','Target','Condition type','Condition value',val1,val2,val3,val4,val5,'Emotion','Chat');
#
# RATE: the chance of the skill being casted when the condition is fulfilled (10000 = 100%).
# DELAY: the time (in milliseconds) before attempting to recast the same skill.
#
# STATE:
#	any (except dead) / idle (in standby) / walk (in movement) / dead (on killed) /
#	loot /attack / angry (like attack, except player has not attacked mob yet) /
#	chase (following target, after being attacked) / follow (following target,
#	without being attacked) / anytarget (attack+angry+chase+follow)
#
# TARGET:
#	target (current target) / self / friend / master / randomtarget (any enemy within skill's range)
#
#	The following are for ground-skills, a random target tile is selected from the specified area:
#	    around1 (3x3 area around self) / around2 (5x5 area around self) /
#	    around3 (7x7 area around self) / around4 (9x9 area around self) /
#	    around5 (3x3 area around target) / around6 (5x5 area around target) /
#	    around7 (7x7 area around target) / around8 (9x9 area around target) /
#	    around = around4
#
# CONDITION:
#	always			Unconditional (no condition value).
#	onspawn			When mob spawns/respawns (no condition value).
#	myhpltmaxrate		When mob's HP drops to the specified %.
#	myhpinrate		When mob's HP is in a certain % range (condition value = lower bound, val1 = upper bound).
#	mystatuson		If mob has the specified abnormality in status.
#	mystatusoff		If mob has ended the specified abnormality in status.
#	friendhpltmaxrate	When mob's friend's HP drops to the specified %.
#	friendhpinrate		When mob's friend's HP is in a certain % range (condition value = lower bound, val1 = upper bound).
#	friendstatuson		If friend has the specified abnormality in status.
#	friendstatusoff		If friend has ended the specified abnormality in status.
#	attackpcgt		When attack PCs become greater than specified number.
#	attackpcge		When attack PCs become greater than or equal to the specified number.
#	slavelt			When number of slaves is less than the original specified number.
#	slavele			When number of slaves is less than or equal to the original specified number.
#	closedattacked		When close range melee attacked (no condition value).
#	longrangeattacked	When long range attacked, ex. bows, guns, ranged skills (no condition value).
#	skillused		When the specified skill is used on the mob.
#	afterskill		After mob casts the specified skill.
#	casttargeted		When a target is in cast range (no condition value).
#	rudeattacked		When mob is rude attacked (no condition value).
#
#	Status abnormalities specified through the statuson/statusoff system:
#	    anybad (any type of state change) / stone / freeze / stun / sleep /
#	    poison / curse / silence / confusion / blind / hiding / sight (unhidden)
#
# Note: if a negative MobID is provided, the skill will be treated as 'global':
#	-1: added for all boss types.
#	-2: added for all normal types.
#	-3: added for all mobs.

-- Has to be converted via yaml2sql

