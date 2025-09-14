ALTER TABLE `guild_member`
	DROP COLUMN `account_id`,
	DROP COLUMN `hair`,
	DROP COLUMN `hair_color`,
	DROP COLUMN `gender`,
	DROP COLUMN `class`,
	DROP COLUMN `lv`,
	DROP COLUMN `exp_payper`,
	DROP COLUMN `online`,
	DROP COLUMN `name`;

ALTER TABLE `friends`
	DROP COLUMN `friend_account`;
