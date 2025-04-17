ALTER TABLE `feedinglog`
	MODIFY `item_id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `mvplog`
	MODIFY `prize` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `picklog`
	MODIFY `nameid` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card0` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card1` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card2` int unsigned NOT NULL DEFAULT '0',
	MODIFY `card3` int unsigned NOT NULL DEFAULT '0';
