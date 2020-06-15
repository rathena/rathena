INSERT IGNORE `sc_data` (`account_id`, `char_id`, `type`, `tick`) SELECT `account_id`, `char_id`, '746', '-1' FROM `char` WHERE `option` & '4194304' != '0';
