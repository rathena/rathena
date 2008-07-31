START TRANSACTION;

ALTER TABLE `zenylog` MODIFY COLUMN `type` enum('M','T','V','S','N','A','E') NOT NULL default 'S';

COMMIT;
