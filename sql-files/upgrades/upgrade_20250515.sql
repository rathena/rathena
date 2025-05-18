ALTER TABLE `char`
	ADD COLUMN `disable_showcostumes` tinyint(1) unsigned NOT NULL default '1' AFTER `disable_partyinvite`
;
