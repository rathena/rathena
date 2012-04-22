-- Adds 'D' to `type` in `zenylog`

ALTER TABLE `zenylog` MODIFY `type` ENUM('M','T','V','S','N','A','E','B','I','D') NOT NULL DEFAULT 'S';
