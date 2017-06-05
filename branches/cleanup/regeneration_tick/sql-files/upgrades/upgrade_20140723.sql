ALTER TABLE `vendings` ADD `body_direction` CHAR( 1 ) NOT NULL DEFAULT '4',
ADD `head_direction` CHAR( 1 ) NOT NULL DEFAULT '0',
ADD `sit` CHAR( 1 ) NOT NULL DEFAULT '1';

ALTER TABLE `buyingstores` ADD `body_direction` CHAR( 1 ) NOT NULL DEFAULT '4',
ADD `head_direction` CHAR( 1 ) NOT NULL DEFAULT '0',
ADD `sit` CHAR( 1 ) NOT NULL DEFAULT '1';
