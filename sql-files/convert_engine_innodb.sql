--
-- eAthena Database Converter ( MyISAM -> InnoDB )
--

ALTER TABLE `cart_inventory` TYPE = InnoDB;
ALTER TABLE `char` TYPE = InnoDB;
ALTER TABLE `charlog` TYPE = InnoDB;
ALTER TABLE `friends` TYPE = InnoDB;
ALTER TABLE `global_reg_value` TYPE = InnoDB;
ALTER TABLE `guild` TYPE = InnoDB;
ALTER TABLE `guild_alliance` TYPE = InnoDB;
ALTER TABLE `guild_castle` TYPE = InnoDB;
ALTER TABLE `guild_expulsion` TYPE = InnoDB;
ALTER TABLE `guild_member` TYPE = InnoDB;
ALTER TABLE `guild_position` TYPE = InnoDB;
ALTER TABLE `guild_skill` TYPE = InnoDB;
ALTER TABLE `guild_storage` TYPE = InnoDB;
ALTER TABLE `homunculus` TYPE = InnoDB;
ALTER TABLE `interlog` TYPE = InnoDB;
ALTER TABLE `inventory` TYPE = InnoDB;
ALTER TABLE `ipbanlist` TYPE = InnoDB;
ALTER TABLE `item_db` TYPE = InnoDB;
ALTER TABLE `item_db2` TYPE = InnoDB;
ALTER TABLE `login` TYPE = InnoDB;
#ALTER TABLE `loginlog` TYPE = InnoDB;
#ALTER TABLE `mail` TYPE = InnoDB;
ALTER TABLE `mapreg` TYPE = InnoDB;
ALTER TABLE `memo` TYPE = InnoDB;
ALTER TABLE `mob_db` TYPE = InnoDB;
ALTER TABLE `mob_db2` TYPE = InnoDB;
ALTER TABLE `party` TYPE = InnoDB;
ALTER TABLE `pet` TYPE = InnoDB;
ALTER TABLE `ragsrvinfo` TYPE = InnoDB;
ALTER TABLE `sc_data` TYPE = InnoDB;
ALTER TABLE `skill` TYPE = InnoDB;
ALTER TABLE `skill_homunculus` TYPE = InnoDB;
ALTER TABLE `sstatus` TYPE = InnoDB;
ALTER TABLE `storage` TYPE = InnoDB;
