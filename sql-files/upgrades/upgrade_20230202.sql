ALTER TABLE `login`
	ADD COLUMN `passwd_type` tinyint unsigned NOT NULL default 0,
	MODIFY `user_pass` VARCHAR(72) NOT NULL default ''
;
