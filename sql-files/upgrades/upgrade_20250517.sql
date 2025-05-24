ALTER TABLE `mob_db`
	ADD COLUMN `groupid` smallint(6) unsigned DEFAULT NULL AFTER `damage_taken`,
	ADD COLUMN `title` text DEFAULT NULL AFTER `groupid`
;
ALTER TABLE `mob_db2`
	ADD COLUMN `groupid` smallint(6) unsigned DEFAULT NULL AFTER `damage_taken`,
	ADD COLUMN `title` text DEFAULT NULL AFTER `groupid`
;
ALTER TABLE `mob_db2_re`
	ADD COLUMN `groupid` smallint(6) unsigned DEFAULT NULL AFTER `damage_taken`,
	ADD COLUMN `title` text DEFAULT NULL AFTER `groupid`
;
ALTER TABLE `mob_db_re`
	ADD COLUMN `groupid` smallint(6) unsigned DEFAULT NULL AFTER `damage_taken`,
	ADD COLUMN `title` text DEFAULT NULL AFTER `groupid`
;
