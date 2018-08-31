UPDATE `char` SET `char`.`skill_point` = `char`.`skill_point` + `skill`.`lv` WHERE `skill`.`id` = 2049 AND `char`.`char_id` = `skill`.`char_id`;

DELETE FROM `skill` WHERE `id` = 2049;
