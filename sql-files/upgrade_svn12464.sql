ALTER TABLE `login` CHANGE `connect_until` `expiration_time` INT( 11 ) UNSIGNED NOT NULL DEFAULT '0'
ALTER TABLE `login` CHANGE `ban_until` `unban_time` INT( 11 ) UNSIGNED NOT NULL DEFAULT '0'
