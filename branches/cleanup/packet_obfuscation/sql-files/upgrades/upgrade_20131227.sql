ALTER TABLE  `bonus_script` CHANGE  `char_id`  `char_id` VARCHAR( 11 ) NOT NULL;
ALTER TABLE  `bonus_script` CHANGE  `tick`  `tick` VARCHAR( 11 ) NOT NULL DEFAULT '0';
ALTER TABLE  `bonus_script` CHANGE  `flag`  `flag` VARCHAR( 3 ) NOT NULL DEFAULT '0';
ALTER TABLE  `bonus_script` CHANGE  `type`  `type` CHAR( 1 ) NOT NULL DEFAULT '0';
ALTER TABLE  `bonus_script` ADD  `icon` VARCHAR( 3 ) NOT NULL DEFAULT  '-1';
