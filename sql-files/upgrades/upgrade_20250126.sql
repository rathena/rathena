ALTER TABLE `char`
	ADD COLUMN `disable_partyinvite` tinyint unsigned NOT NULL DEFAULT '0' AFTER `disable_call`
