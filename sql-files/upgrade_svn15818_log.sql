-- Adds 'I' to `type` in `picklog` and `zenylog`

ALTER TABLE `zenylog` MODIFY `type` ENUM('M','T','V','S','N','A','E','B','I') NOT NULL DEFAULT 'S';
