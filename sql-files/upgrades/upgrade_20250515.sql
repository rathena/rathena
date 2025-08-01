ALTER TABLE `char`
	ADD COLUMN `disable_showcostumes` tinyint(1) unsigned NOT NULL default '0' AFTER `disable_partyinvite`
;
