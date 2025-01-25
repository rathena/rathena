ALTER TABLE `char`
	ADD COLUMN `disable_partyinvite` tinyint(3) unsigned NOT NULL default '0' AFTER `disable_call`
