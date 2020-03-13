UPDATE `char_reg_num` SET `key` = 'ep16_royal' WHERE `key` = 'banquet_main_quest' AND `value` < 16;
UPDATE `char_reg_num` SET `key` = 'ep16_royal', `value` = `value` - 1 WHERE `key` = 'banquet_main_quest' AND `value` < 20;
UPDATE `char_reg_num` SET `key` = 'ep16_royal', `value` = `value` - 2 WHERE `key` = 'banquet_main_quest' AND `value` < 26;
UPDATE `char_reg_num` SET `key` = 'ep16_royal', `value` = 25 WHERE `key` = 'banquet_main_quest' AND `value` > 25;

DELETE FROM `char_reg_num` WHERE `key` = 'banquet_nerius_quest';
