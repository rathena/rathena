CREATE TABLE IF NOT EXISTS `interreg` (
  `varname` varchar(11) NOT NULL,
  `value` varchar(20) NOT NULL,
  PRIMARY KEY (`varname`)
) ENGINE=InnoDB;
INSERT INTO `interreg` (`varname`, `value`) VALUES
('nsiuid', '0');

ALTER TABLE `auction` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `cart_inventory` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `guild_storage` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `inventory` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `mail` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `storage` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
