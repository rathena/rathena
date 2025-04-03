ALTER TABLE `picklog`
	MODIFY `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U','$','F','Y','Z','Q','H','J','W','0','1','2') NOT NULL default 'P'
;

ALTER TABLE `zenylog`
	MODIFY `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','K','J','X','0','2') NOT NULL default 'S'
;
