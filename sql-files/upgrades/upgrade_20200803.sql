ALTER TABLE `sc_data` ADD COLUMN `tick_total` BIGINT(20) NOT NULL AFTER `tick`;
UPDATE `sc_data` SET `tick_total` = `tick`;

ALTER TABLE `sc_data` ADD COLUMN `tick_time` BIGINT(20) NOT NULL AFTER `tick_total`;
UPDATE `sc_data` SET `tick_time` = `tick`;
