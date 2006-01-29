###################################################################################################
# This one is also necessary, since foreign keys may only reference
# InnoDB tables.

ALTER TABLE `char` TYPE=InnoDB;

###################################################################################################
# Add the new guild column char_id and populate it with Guild Master ids
# Note that the auto-fill is case sensitive!

ALTER TABLE `guild` ADD COLUMN `char_id` int(11) NOT NULL DEFAULT '10000' AFTER `name`;
UPDATE `guild`,`char` SET `guild`.`char_id`=`char`.`char_id` WHERE `guild`.`master` = `char`.`name`;

###################################################################################################
# Now we go on altering stuff - dropping old keys (just in case),
# converting table types, and then creating new keys.

ALTER TABLE guild DROP PRIMARY KEY;
ALTER TABLE guild TYPE=InnoDB;
ALTER TABLE guild 
 ADD PRIMARY KEY  (guild_id,char_id),
 MODIFY COLUMN `guild_id` INTEGER NOT NULL AUTO_INCREMENT, AUTO_INCREMENT = 10000,
 ADD KEY char_id (char_id),
 ADD UNIQUE KEY guild_id (guild_id),
 ADD CONSTRAINT `guild_ibfk_1` FOREIGN KEY (`char_id`) REFERENCES `char` 
(`char_id`) ON DELETE CASCADE;

ALTER TABLE guild_alliance DROP INDEX `guild_id`;
ALTER TABLE guild_alliance TYPE=InnoDB;
ALTER TABLE guild_alliance
 ADD PRIMARY KEY  (guild_id,alliance_id),
 ADD KEY alliance_id (alliance_id),
 ADD CONSTRAINT `guild_alliance_ibfk_1` FOREIGN KEY (`guild_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE,
 ADD CONSTRAINT `guild_alliance_ibfk_2` FOREIGN KEY (`alliance_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE;

ALTER TABLE guild_castle DROP PRIMARY KEY, DROP INDEX `guild_id`;
ALTER TABLE guild_castle TYPE=InnoDB;
ALTER TABLE guild_castle
 ADD PRIMARY KEY  (castle_id);

ALTER TABLE guild_expulsion DROP INDEX `guild_id`;
ALTER TABLE guild_expulsion TYPE=InnoDB;
ALTER TABLE guild_expulsion
 ADD  PRIMARY KEY  (guild_id,name),
 ADD CONSTRAINT `guild_expulsion_ibfk_1` FOREIGN KEY (`guild_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE;

ALTER TABLE guild_member DROP INDEX `guild_id`, DROP INDEX `account_id`; 
ALTER TABLE guild_member TYPE=InnoDB;
ALTER TABLE guild_member DROP INDEX `char_id`;
ALTER TABLE guild_member
 ADD  PRIMARY KEY  (guild_id,char_id),
 ADD  KEY char_id (char_id),
 ADD  CONSTRAINT `guild_member_ibfk_1` FOREIGN KEY (`guild_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE,
 ADD  CONSTRAINT `guild_member_ibfk_2` FOREIGN KEY (`char_id`) 
REFERENCES `char` (`char_id`) ON DELETE CASCADE;

ALTER TABLE guild_position DROP INDEX `guild_id`;
ALTER TABLE guild_position TYPE=InnoDB;
ALTER TABLE guild_position
 ADD  PRIMARY KEY  (guild_id,position),
ADD  KEY guild_id (guild_id),
ADD CONSTRAINT `guild_position_ibfk_1` FOREIGN KEY (`guild_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE;

ALTER TABLE guild_skill DROP INDEX `guild_id`;
ALTER TABLE guild_skill TYPE=InnoDB;
ALTER TABLE guild_skill
 ADD  PRIMARY KEY  (guild_id,id),
 ADD  CONSTRAINT `guild_skill_ibfk_1` FOREIGN KEY (`guild_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE;

ALTER TABLE guild_storage DROP INDEX `guild_id`;
ALTER TABLE guild_storage TYPE=InnoDB;
ALTER TABLE guild_storage
 ADD  KEY guild_id (guild_id),
 ADD  CONSTRAINT `guild_storage_ibfk_1` FOREIGN KEY (`guild_id`) 
REFERENCES `guild` (`guild_id`) ON DELETE CASCADE;

