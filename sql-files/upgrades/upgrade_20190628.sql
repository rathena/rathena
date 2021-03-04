ALTER TABLE `mob_db`
	MODIFY `Sprite` varchar(24) NOT NULL,
	ADD UNIQUE KEY (`Sprite`)
;

ALTER TABLE `mob_db_re`
	MODIFY `Sprite` varchar(24) NOT NULL,
	ADD UNIQUE KEY (`Sprite`)
;

ALTER TABLE `mob_db2`
	MODIFY `Sprite` varchar(24) NOT NULL,
	ADD UNIQUE KEY (`Sprite`)
;

ALTER TABLE `mob_db2_re`
	MODIFY `Sprite` varchar(24) NOT NULL,
	ADD UNIQUE KEY (`Sprite`)
;
