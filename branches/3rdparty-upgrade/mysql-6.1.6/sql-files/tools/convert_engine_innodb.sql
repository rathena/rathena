--
-- rAthena Database Converter ( MyISAM -> InnoDB )
--

ALTER TABLE `auction` ENGINE = InnoDB;
ALTER TABLE `cart_inventory` ENGINE = InnoDB;
ALTER TABLE `char` ENGINE = InnoDB;
ALTER TABLE `charlog` ENGINE = InnoDB;
ALTER TABLE `friends` ENGINE = InnoDB;
ALTER TABLE `global_reg_value` ENGINE = InnoDB;
ALTER TABLE `guild` ENGINE = InnoDB;
ALTER TABLE `guild_alliance` ENGINE = InnoDB;
ALTER TABLE `guild_castle` ENGINE = InnoDB;
ALTER TABLE `guild_expulsion` ENGINE = InnoDB;
ALTER TABLE `guild_member` ENGINE = InnoDB;
ALTER TABLE `guild_position` ENGINE = InnoDB;
ALTER TABLE `guild_skill` ENGINE = InnoDB;
ALTER TABLE `guild_storage` ENGINE = InnoDB;
ALTER TABLE `homunculus` ENGINE = InnoDB;
ALTER TABLE `hotkey` ENGINE = InnoDB;
ALTER TABLE `interlog` ENGINE = InnoDB;
ALTER TABLE `inventory` ENGINE = InnoDB;
ALTER TABLE `ipbanlist` ENGINE = InnoDB;
#ALTER TABLE `item_db` ENGINE = InnoDB;
#ALTER TABLE `item_db2` ENGINE = InnoDB;
ALTER TABLE `login` ENGINE = InnoDB;
ALTER TABLE `mail` ENGINE = InnoDB;
ALTER TABLE `mapreg` ENGINE = InnoDB;
ALTER TABLE `memo` ENGINE = InnoDB;
ALTER TABLE `mercenary` ENGINE = InnoDB;
ALTER TABLE `mercenary_owner` ENGINE = InnoDB;
#ALTER TABLE `mob_db` ENGINE = InnoDB;
#ALTER TABLE `mob_db2` ENGINE = InnoDB;
ALTER TABLE `party` ENGINE = InnoDB;
ALTER TABLE `pet` ENGINE = InnoDB;
ALTER TABLE `quest` ENGINE = InnoDB;
ALTER TABLE `ragsrvinfo` ENGINE = InnoDB;
ALTER TABLE `sc_data` ENGINE = InnoDB;
ALTER TABLE `skill` ENGINE = InnoDB;
ALTER TABLE `skill_homunculus` ENGINE = InnoDB;
ALTER TABLE `sstatus` ENGINE = InnoDB;
ALTER TABLE `storage` ENGINE = InnoDB;
