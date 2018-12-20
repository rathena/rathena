ALTER TABLE `bonus_script` MODIFY COLUMN `tick` bigint(20) NOT NULL default '0';

ALTER TABLE `elemental` MODIFY COLUMN `life_time` bigint(20) NOT NULL default '0';

ALTER TABLE `mercenary` MODIFY COLUMN `life_time` bigint(20) NOT NULL default '0';

ALTER TABLE `sc_data` MODIFY COLUMN `tick` bigint(20) NOT NULL;

ALTER TABLE `skillcooldown` MODIFY COLUMN `tick` bigint(20) NOT NULL;
