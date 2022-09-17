ALTER TABLE `picklog`
	MODIFY `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U','$','F','Y','Z','Q','H','J') NOT NULL default 'P'
;

ALTER TABLE `zenylog`
	MODIFY `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','K','J') NOT NULL default 'S'
;
