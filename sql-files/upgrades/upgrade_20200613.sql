ALTER TABLE `login`
	ADD COLUMN `web_auth_token` VARCHAR(17) NULL AFTER `old_group`,
	ADD UNIQUE KEY `web_auth_token_key` (`web_auth_token`)
;
