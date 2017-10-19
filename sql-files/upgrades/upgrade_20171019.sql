INSERT INTO `skill` (`char_id`, `id`, `lv`, `flag`)
  SELECT `char_id`, 5065, 1, 0 FROM `char` WHERE `father` > 0 OR `mother` > 0;
