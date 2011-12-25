CREATE TABLE `donate` (
  `account_id` INT(11) UNSIGNED NOT NULL,
  `amount` FLOAT(9,2) UNSIGNED NOT NULL,
  `claimed` FLOAT(9,2) UNSIGNED NOT NULL,
  PRIMARY KEY  (`account_id`)
) TYPE=MYISAM;
