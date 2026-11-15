ALTER TABLE `char`
	ADD COLUMN `disable_rodex_nonfriend` tinyint(1) unsigned NOT NULL default '0' AFTER `disable_partyinvite`;
