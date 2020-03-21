UPDATE `char_reg_num` SET `key` = 'ep16_royal' WHERE `key` = 'banquet_main_quest' AND `value` < 16;
UPDATE `char_reg_num` SET `key` = 'ep16_royal', `value` = `value` - 1 WHERE `key` = 'banquet_main_quest' AND `value` < 20;
UPDATE `char_reg_num` SET `key` = 'ep16_royal', `value` = `value` - 2 WHERE `key` = 'banquet_main_quest' AND `value` < 26;
UPDATE `char_reg_num` SET `key` = 'ep16_royal', `value` = 25 WHERE `key` = 'banquet_main_quest' AND `value` > 25;

DELETE FROM `char_reg_num` WHERE `key` = 'banquet_nerius_quest';
DELETE FROM `char_reg_num` WHERE `key` = 'banquet_heine_quest';
DELETE FROM `char_reg_num` WHERE `key` = 'banquet_richard_quest';

UPDATE `char_reg_num` SET `key` = 'ep16_wal' WHERE `key` = 'banquet_walther_quest' AND `value` < 2;
UPDATE `char_reg_num` SET `key` = 'ep16_wal', `value` = `value` - 1 WHERE `key` = 'banquet_walther_quest';
UPDATE `char_reg_num` SET `key` = 'ep16_lug' WHERE `key` = 'banquet_roegenburg_quest';
UPDATE `char_reg_num` SET `key` = 'ep16_gaobs' WHERE `key` = 'banquet_geoborg_quest';

UPDATE `char_reg_num` SET `key` = 'ep16_wig' WHERE `key` = 'banquet_wigner_quest' AND `value` < 5;
UPDATE `char_reg_num` SET `key` = 'ep16_wig', `value` = `value` + 5 WHERE `key` = 'banquet_wigner_quest' AND `value` > 5;
UPDATE `char_reg_num` SET `key` = 'ep16_wig', `value` = 10 FROM `char_reg_num` INNER JOIN `quest` ON `char_reg_num`.`char_id` = `quest`.`char_id`
WHERE `key` = 'banquet_wigner_quest' AND `value` = 5 AND `quest_id` = 14482 AND `state` = 1;
UPDATE `char_reg_num` SET `key` = 'ep16_wig', `value` = 9 FROM `char_reg_num` INNER JOIN `quest` ON `char_reg_num`.`char_id` = `quest`.`char_id`
WHERE `key` = 'banquet_wigner_quest' AND `value` = 5 AND `quest_id` = 14480 AND `state` = 2;
UPDATE `char_reg_num` SET `key` = 'ep16_wig', `value` = 8 FROM `char_reg_num` INNER JOIN `quest` ON `char_reg_num`.`char_id` = `quest`.`char_id`
WHERE `key` = 'banquet_wigner_quest' AND `value` = 5 AND `quest_id` = 14480 AND `state` = 1;
UPDATE `char_reg_num` SET `key` = 'ep16_wig', `value` = 7 FROM `char_reg_num` INNER JOIN `quest` ON `char_reg_num`.`char_id` = `quest`.`char_id`
WHERE `key` = 'banquet_wigner_quest' AND `value` = 5 AND `quest_id` = 14481 AND `state` = 2;
UPDATE `char_reg_num` SET `key` = 'ep16_wig', `value` = 6 FROM `char_reg_num` INNER JOIN `quest` ON `char_reg_num`.`char_id` = `quest`.`char_id`
WHERE `key` = 'banquet_wigner_quest' AND `value` = 5 AND `quest_id` = 14481 AND `state` = 1;
UPDATE `char_reg_num` SET `key` = 'ep16_wig' WHERE `key` = 'banquet_wigner_quest' AND `value` = 5;

UPDATE `char_reg_num` SET `key` = 'ep16_cookbs' WHERE `key` = 'banquet_quest_cooking' AND `value` < 3;
UPDATE `char_reg_num` SET `key` = 'ep16_cookbs', `value` = 2 WHERE `key` = 'banquet_quest_cooking' AND `value` = 3;
UPDATE `char_reg_num` SET `key` = 'ep16_cookbs', `value` = `value` + 8 WHERE `key` = 'banquet_quest_cooking';

DELETE FROM `quest` WHERE `quest_id` = 11428;
DELETE FROM `quest` WHERE `quest_id` = 11429;
DELETE FROM `quest` WHERE `quest_id` = 11430;
DELETE FROM `quest` WHERE `quest_id` = 11431;

DELETE FROM `char_reg_num` WHERE `key` = 'banquet_quest_sauce';
