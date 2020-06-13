ALTER TABLE `login`
	ADD COLUMN `web_auth_token` VARCHAR(255) NULL AFTER `old_group`;
