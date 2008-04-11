ALTER TABLE `login` CHANGE `expiration_time` `connect_until` INT( 11 ) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `login` CHANGE `unban_time` `ban_until` INT( 11 ) UNSIGNED NOT NULL DEFAULT '0';
