ALTER TABLE `picklog`
	MODIFY `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U','$','F','Y','Z','Q','H','J','W','0','1','2','3') NOT NULL default 'P'
;
