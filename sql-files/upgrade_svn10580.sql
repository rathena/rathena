ALTER TABLE `chatlog` CHANGE `type` `type` ENUM( 'O', 'W', 'P', 'G', 'M' ) NOT NULL DEFAULT 'O';
