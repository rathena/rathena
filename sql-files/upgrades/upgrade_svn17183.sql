ALTER TABLE `login` ADD COLUMN `pincode` varchar(4) NOT NULL DEFAULT '';
ALTER TABLE `login` ADD COLUMN `pincode_change` int(11) unsigned NOT NULL DEFAULT '0';