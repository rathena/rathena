ALTER TABLE `item_db`
	MODIFY `equip_level_min` smallint unsigned DEFAULT NULL,
	MODIFY `equip_level_max` smallint unsigned DEFAULT NULL
;

ALTER TABLE `item_db2`
	MODIFY `equip_level_min` smallint unsigned DEFAULT NULL,
	MODIFY `equip_level_max` smallint unsigned DEFAULT NULL
;

ALTER TABLE `item_db_re`
	MODIFY `equip_level_min` smallint unsigned DEFAULT NULL,
	MODIFY `equip_level_max` smallint unsigned DEFAULT NULL
;

ALTER TABLE `item_db2_re`
	MODIFY `equip_level_min` smallint unsigned DEFAULT NULL,
	MODIFY `equip_level_max` smallint unsigned DEFAULT NULL
;

ALTER TABLE `mob_db`
	MODIFY `ai` varchar(50) DEFAULT NULL
;

ALTER TABLE `mob_db2`
	MODIFY `ai` varchar(50) DEFAULT NULL
;

ALTER TABLE `mob_db_re`
	MODIFY `ai` varchar(50) DEFAULT NULL
;

ALTER TABLE `mob_db2_re`
	MODIFY `ai` varchar(50) DEFAULT NULL
;
