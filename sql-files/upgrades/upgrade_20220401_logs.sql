
ALTER TABLE `zenylog`
	MODIFY `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','K','J','X') NOT NULL default 'S'
;
