UPDATE `loginlog` SET `ip` = inet_aton(`ip`);
ALTER TABLE `loginlog` CHANGE `ip` `ip` INT( 10 ) UNSIGNED ZEROFILL NOT NULL DEFAULT '0';