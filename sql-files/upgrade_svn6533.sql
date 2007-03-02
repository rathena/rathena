ALTER TABLE `loginlog` CHANGE `ip` `ip` CHAR( 15 ) NOT NULL;
ALTER TABLE `loginlog` ADD INDEX ( `ip` ( 15 ) );
