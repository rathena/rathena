-- Verus police quests
DELETE FROM `char_reg_num` WHERE `key` = 'trap_doom_prayers' AND `index` = 0;
DELETE FROM `char_reg_num` WHERE `key` = 'count_stone_seiden' AND `index` = 0;
-- Verus Wandering Bard quest
DELETE FROM `char_reg_num` WHERE `key` = 'wandering_bard_quest' AND `index` = 0;
-- To Phantasmagorika! quest
UPDATE `char_reg_num` SET `key` = 'ep15_1_elb' WHERE `key` = 'VER_ELEVATOR' AND `value` < 100 AND `index` = 0;
UPDATE `char_reg_num` SET `key` = 'ep15_1_elb', `value` = `value` - 94 WHERE `key` = 'VER_ELEVATOR' AND `value` >= 100 AND `value` < 1000 AND `index` = 0;
-- Vestige quest
UPDATE `char_reg_num` SET `key` = 'ep15_2_bslast', `value` = `value` - 999 WHERE `key` = 'VER_ELEVATOR' AND `value` >= 1000 AND `index` = 0;
-- Krotzel's Request quests
UPDATE `char_reg_num` SET `key` = 'ep15_2_brigan' WHERE `key` = 'VER_REPORTER' AND `index` = 0;
-- Main quest
INSERT INTO `char_reg_num` (`char_id`, `key`, `index`, `value`) SELECT `char_id`, 'ep15_2_permit', 0, 1 FROM `char_reg_num` WHERE `key` = 'VER_MAIN' AND `value` >= 19 AND `index` = 0;
UPDATE `char_reg_num` SET `key` = 'ep15_1_atnad' WHERE `key` = 'VER_MAIN' AND `value` < 31 AND `index` = 0;
UPDATE `char_reg_num` SET `key` = 'ep15_1_atnad', `value` = 30 WHERE `key` = 'VER_MAIN' AND `value` > 30 AND `value` < 37 AND `index` = 0;
UPDATE `char_reg_num` SET `key` = 'ep15_1_atnad', `value` = `value` - 6 WHERE `key` = 'VER_MAIN' AND `value` > 36 AND `index` = 0;
DELETE FROM `char_reg_num` WHERE `key` = 'VERUS_DAILY_QUEST' AND `index` = 0;
-- Memory quest
DELETE FROM `char_reg_num` WHERE `key` = 'recorder_quest_type' AND `index` = 0;
DELETE FROM `char_reg_num` WHERE `key` = 'recorder_quest_status' AND `index` = 0;
