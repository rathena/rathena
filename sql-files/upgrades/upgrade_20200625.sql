ALTER TABLE `login`
	ADD COLUMN `web_auth_token` varchar(17) NULL AFTER `old_group`,
	ADD COLUMN `web_auth_token_enabled` tinyint NOT NULL DEFAULT '0' AFTER `web_auth_token`,
	ADD UNIQUE KEY `web_auth_token_key` (`web_auth_token`)
;
