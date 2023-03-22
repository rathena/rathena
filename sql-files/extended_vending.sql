ALTER TABLE `vendings` ADD COLUMN `extended_vending_item` int(10) unsigned NOT NULL DEFAULT '0' AFTER `sit`;

-- REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (30000,'Zeny','Zeny','Etc',10,10);
-- REPLACE INTO `item_db2_re` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (30001,'Cash','Cash','Etc',10,10);
-- REPLACE INTO `item_db2` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (30000,'Zeny','Zeny','Etc',10,10);
-- REPLACE INTO `item_db2` (`id`,`name_aegis`,`name_english`,`type`,`price_sell`,`weight`) VALUES (30001,'Cash','Cash','Etc',10,10);
