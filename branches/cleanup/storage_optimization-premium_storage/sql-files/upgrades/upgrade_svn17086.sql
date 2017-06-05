UPDATE `interreg` SET `varname` = 'unique_id' WHERE `interreg`.`varname` = 'nsiuid';

ALTER TABLE `auction` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `cart_inventory` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `guild_storage` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `inventory` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `mail` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `storage` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
