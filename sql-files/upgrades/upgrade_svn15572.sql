-- Rename `level` column to `group_id` in `login` table

ALTER TABLE `login` CHANGE COLUMN `level` `group_id` TINYINT(3) NOT NULL DEFAULT '0';
