UPDATE `char` ch, `skill` sk SET `ch`.`skill_point` = `ch`.`skill_point` + `sk`.`lv` WHERE `sk`.`id` = 2049 AND `ch`.`char_id` = `sk`.`char_id`;

DELETE FROM `skill` WHERE `id` = 2049;
