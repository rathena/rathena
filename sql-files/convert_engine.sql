--
-- eAthena Database Converter ( InnoDB -> MyISAM ) by Jaguar
--

ALTER TABLE `sc_data` DROP FOREIGN KEY `scdata_ibfk_1`, DROP FOREIGN KEY `scdata_ibfk_2`;
ALTER TABLE `guild` DROP FOREIGN KEY `guild_ibfk_1`;
ALTER TABLE `guild_alliance` DROP FOREIGN KEY `guild_alliance_ibfk_1`, DROP FOREIGN KEY `guild_alliance_ibfk_2`;
ALTER TABLE `guild_expulsion` DROP FOREIGN KEY `guild_expulsion_ibfk_1`;
ALTER TABLE `guild_member` DROP FOREIGN KEY `guild_member_ibfk_1`, DROP FOREIGN KEY `guild_member_ibfk_2`;
ALTER TABLE `guild_position` DROP FOREIGN KEY `guild_position_ibfk_1`;
ALTER TABLE `guild_skill` DROP FOREIGN KEY `guild_skill_ibfk_1`;
ALTER TABLE `guild_storage` DROP FOREIGN KEY `guild_storage_ibfk_1`;
ALTER TABLE `sc_data` ENGINE = MYISAM;
ALTER TABLE `login` ENGINE = MYISAM;
ALTER TABLE `char` ENGINE = MYISAM;
ALTER TABLE `guild` ENGINE = MYISAM;
ALTER TABLE `guild_alliance` ENGINE = MYISAM;
ALTER TABLE `guild_expulsion` ENGINE = MYISAM;
ALTER TABLE `guild_member` ENGINE = MYISAM;
ALTER TABLE `guild_position` ENGINE = MYISAM;
ALTER TABLE `guild_skill` ENGINE = MYISAM;
ALTER TABLE `guild_storage` ENGINE = MYISAM;
