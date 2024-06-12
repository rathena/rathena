ALTER TABLE `feedinglog`
	MODIFY `item_id` int(10) unsigned NOT NULL default '0';

ALTER TABLE `mvplog`
	MODIFY `prize` int(10) unsigned NOT NULL default '0';

ALTER TABLE `picklog`
	MODIFY `nameid` int(10) unsigned NOT NULL default '0',
	MODIFY `card0` int(10) unsigned NOT NULL default '0',
	MODIFY `card1` int(10) unsigned NOT NULL default '0',
	MODIFY `card2` int(10) unsigned NOT NULL default '0',
	MODIFY `card3` int(10) unsigned NOT NULL default '0';
