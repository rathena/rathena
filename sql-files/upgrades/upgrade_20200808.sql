ALTER TABLE `auction`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `cart_inventory`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `db_roulette`
	MODIFY `item_id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `guild_storage`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `guild_storage_log`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `inventory`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mail_attachments`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `market`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0';

ALTER TABLE `pet`
	MODIFY `egg_id` int(10) unsigned NOT NULL default '0',
	MODIFY `equip` int(10) unsigned NOT NULL default '0';

ALTER TABLE `sales`
	MODIFY `nameid` int(10) unsigned NOT NULL;

ALTER TABLE `storage`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_cash_db`
	MODIFY `item_id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_cash_db2`
	MODIFY `item_id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db`
	MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db_re`
	MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db2`
	MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `item_db2_re`
	MODIFY `id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db`
	MODIFY `MVP1id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP2id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop1id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop2id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop4id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop5id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop6id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop7id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop8id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop9id` int(10) unsigned NOT NULL default '0',
	MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db_re`
	MODIFY `MVP1id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP2id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop1id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop2id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop4id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop5id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop6id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop7id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop8id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop9id` int(10) unsigned NOT NULL default '0',
	MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db2`
	MODIFY `MVP1id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP2id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop1id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop2id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop4id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop5id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop6id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop7id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop8id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop9id` int(10) unsigned NOT NULL default '0',
	MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mob_db2_re`
	MODIFY `MVP1id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP2id` int(10) unsigned NOT NULL default '0',
	MODIFY `MVP3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop1id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop2id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop3id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop4id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop5id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop6id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop7id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop8id` int(10) unsigned NOT NULL default '0',
	MODIFY `Drop9id` int(10) unsigned NOT NULL default '0',
	MODIFY `DropCardid` int(10) unsigned NOT NULL default '0';
