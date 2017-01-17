ALTER TABLE `chatlog` 
	CHANGE COLUMN `type` `type` ENUM('O','W','P','G','M','C') NOT NULL DEFAULT 'O';
