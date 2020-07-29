-- Verus police quests
UPDATE `char_reg_num` SET `key` = 'ep15_1_mang' WHERE `key` = 'trap_doom_prayers';
UPDATE `char_reg_num` SET `key` = 'ep15_1_mang' WHERE `key` = 'count_stone_seiden';
-- Verus Wandering Bard quest
DELETE FROM `char_reg_num` WHERE `key` = 'wandering_bard_quest';
-- To Phantasmagorika! quest
UPDATE `char_reg_num` SET `key` = 'ep15_1_elb' WHERE `key` = 'VER_ELEVATOR' `value` < 100;
UPDATE `char_reg_num` SET `key` = 'ep15_1_elb', `value` = `value` - 94 WHERE `key` = 'VER_ELEVATOR' AND `value` >= 100 AND `value` < 1000;
-- Vestige quest
UPDATE `char_reg_num` SET `key` = 'ep15_2_bslast', `value` = `value` - 999 WHERE `key` = 'VER_ELEVATOR' AND `value` >= 1000;
-- Krotzel's Request quests
UPDATE `char_reg_num` SET `key` = 'ep15_2_brigan' WHERE `key` = 'VER_REPORTER';
