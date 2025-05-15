ALTER TABLE `char`
	ADD COLUMN `disable_costume` tinyint(3) unsigned NOT NULL default '1' AFTER `disable_partyinvite`
;
