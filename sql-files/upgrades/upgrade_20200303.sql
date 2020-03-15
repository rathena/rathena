UPDATE `char_reg_num` SET `key` = 'ep14_3_newerabs' WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` < 2;
UPDATE `char_reg_num` SET `key` = 'ep14_3_newerabs', `value` = 3 WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` = 2;
UPDATE `char_reg_num` SET `key` = 'ep14_3_newerabs', `value` = `value` + 2 WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` < 8;
UPDATE `char_reg_num` SET `key` = 'ep14_3_newerabs', `value` = `value` + 7 WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` > 7;
