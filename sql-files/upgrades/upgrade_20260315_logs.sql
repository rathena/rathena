-- Migrate old log tables to their log database equivalent
INSERT INTO `log`.`charlog`
SELECT * FROM charlog;
DROP TABLE IF EXISTS charlog;

INSERT INTO `log`.`guildstoragelog`
SELECT * FROM guild_storage_log;
DROP TABLE IF EXISTS guild_storage_log;

INSERT INTO `log`.`interlog`
SELECT * FROM interlog;
DROP TABLE IF EXISTS interlog;

-- Convert the logs' engine to InnoDB to avoid table locks
ALTER TABLE IF EXISTS `log`.`atcommandlog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`branchlog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`cashlog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`charlog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`chatlog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`feedinglog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`guildstoragelog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`interlog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`loginlog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`mvplog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`npclog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`picklog` ENGINE = InnoDB;
ALTER TABLE IF EXISTS `log`.`zenylog` ENGINE = InnoDB;
