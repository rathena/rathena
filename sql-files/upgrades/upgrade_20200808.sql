ALTER TABLE `auction`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `cart_inventory`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `db_roulette`
	MODIFY `item_id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `guild_storage`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `guild_storage_log`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `inventory`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `mail_attachments`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `market`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `pet`
	MODIFY `egg_id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `equip` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `sales`
	MODIFY `nameid` int unsigned NOT NULL;

ALTER TABLE `storage`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `item_cash_db`
	MODIFY `item_id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `item_cash_db2`
	MODIFY `item_id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `item_db`
	MODIFY `id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `item_db_re`
	MODIFY `id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `item_db2`
	MODIFY `id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `item_db2_re`
	MODIFY `id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `mob_db`
	MODIFY `MVP1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop4id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop5id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop6id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop7id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop8id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop9id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `DropCardid` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `mob_db_re`
	MODIFY `MVP1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop4id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop5id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop6id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop7id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop8id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop9id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `DropCardid` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `mob_db2`
	MODIFY `MVP1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop4id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop5id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop6id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop7id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop8id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop9id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `DropCardid` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `mob_db2_re`
	MODIFY `MVP1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `MVP3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop1id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop2id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop3id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop4id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop5id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop6id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop7id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop8id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `Drop9id` int unsigned NOT NULL DEFAULT '0',
	MODIFY `DropCardid` int unsigned NOT NULL DEFAULT '0';
