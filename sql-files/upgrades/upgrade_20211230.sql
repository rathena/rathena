ALTER TABLE `char` 
	ADD COLUMN `pow` smallint unsigned NOT NULL DEFAULT '0' AFTER `luk`,
	ADD COLUMN `sta` smallint unsigned NOT NULL DEFAULT '0' AFTER `pow`,
	ADD COLUMN `wis` smallint unsigned NOT NULL DEFAULT '0' AFTER `sta`,
	ADD COLUMN `spl` smallint unsigned NOT NULL DEFAULT '0' AFTER `wis`,
	ADD COLUMN `con` smallint unsigned NOT NULL DEFAULT '0' AFTER `spl`,
	ADD COLUMN `crt` smallint unsigned NOT NULL DEFAULT '0' AFTER `con`,
	ADD COLUMN `max_ap` int unsigned NOT NULL DEFAULT '0' AFTER `sp`,
	ADD COLUMN `ap` int unsigned NOT NULL DEFAULT '0' AFTER `max_ap`,
	ADD COLUMN `trait_point` int unsigned NOT NULL DEFAULT '0' AFTER `skill_point`
;
