INSERT IGNORE INTO `sc_data` (`account_id`, `char_id`, `type`, `tick`) SELECT `account_id`, `char_id`, '752', '-1' FROM `char` WHERE `option` & '4194304' != '0';
