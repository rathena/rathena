ALTER TABLE `picklog` CHANGE `type` `type` ENUM( 'M', 'P', 'L', 'T', 'V', 'S', 'N', 'C', 'A', 'R', 'G', 'E' ) NOT NULL DEFAULT 'P';
