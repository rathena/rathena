ALTER TABLE `auction` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `auction` MODIFY `card0` int(10) unsigned NOT NULL default '0';
ALTER TABLE `auction` MODIFY `card1` int(10) unsigned NOT NULL default '0';
ALTER TABLE `auction` MODIFY `card2` int(10) unsigned NOT NULL default '0';
ALTER TABLE `auction` MODIFY `card3` int(10) unsigned NOT NULL default '0';
ALTER TABLE `cart_inventory` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `cart_inventory` MODIFY `card0` int(10) unsigned NOT NULL default '0';
ALTER TABLE `cart_inventory` MODIFY `card1` int(10) unsigned NOT NULL default '0';
ALTER TABLE `cart_inventory` MODIFY `card2` int(10) unsigned NOT NULL default '0';
ALTER TABLE `cart_inventory` MODIFY `card3` int(10) unsigned NOT NULL default '0';
ALTER TABLE `db_roulette` MODIFY `item_id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage` MODIFY `card0` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage` MODIFY `card1` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage` MODIFY `card2` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage` MODIFY `card3` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage_log` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage_log` MODIFY `card0` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage_log` MODIFY `card1` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage_log` MODIFY `card2` int(10) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage_log` MODIFY `card3` int(10) unsigned NOT NULL default '0';
ALTER TABLE `inventory` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `inventory` MODIFY `card0` int(10) unsigned NOT NULL default '0';
ALTER TABLE `inventory` MODIFY `card1` int(10) unsigned NOT NULL default '0';
ALTER TABLE `inventory` MODIFY `card2` int(10) unsigned NOT NULL default '0';
ALTER TABLE `inventory` MODIFY `card3` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mail_attachments` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mail_attachments` MODIFY `card0` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mail_attachments` MODIFY `card1` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mail_attachments` MODIFY `card2` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mail_attachments` MODIFY `card3` int(10) unsigned NOT NULL default '0';
ALTER TABLE `market` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `pet` MODIFY `egg_id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `pet` MODIFY `equip` int(10) unsigned NOT NULL default '0';
ALTER TABLE `storage` MODIFY `nameid` int(10) unsigned NOT NULL default '0';
ALTER TABLE `storage` MODIFY `card0` int(10) unsigned NOT NULL default '0';
ALTER TABLE `storage` MODIFY `card1` int(10) unsigned NOT NULL default '0';
ALTER TABLE `storage` MODIFY `card2` int(10) unsigned NOT NULL default '0';
ALTER TABLE `storage` MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_cash_db` MODIFY `item_id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_cash_db2` MODIFY `item_id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db` MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db_re` MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db2` MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db2_re` MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db` MODIFY `MVP1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `MVP2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `MVP3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop4id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop5id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop6id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop7id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop8id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `Drop9id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db` MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db_re` MODIFY `MVP1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `MVP2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `MVP3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop4id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop5id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop6id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop7id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop8id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `Drop9id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db_re` MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db2` MODIFY `MVP1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `MVP2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `MVP3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop4id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop5id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop6id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop7id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop8id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `Drop9id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2` MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db2_re` MODIFY `MVP1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `MVP2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `MVP3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop1id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop2id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop3id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop4id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop5id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop6id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop7id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop8id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `Drop9id` int(10) unsigned NOT NULL default '0';
ALTER TABLE `mob_db2_re` MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';
