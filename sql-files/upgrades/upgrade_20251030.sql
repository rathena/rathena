ALTER TABLE `cart_inventory`
	MODIFY `char_id` int(11) unsigned NOT NULL default '0'
;

ALTER TABLE `charlog`
	MODIFY `account_id` int(11) unsigned NOT NULL default '0'
;

ALTER TABLE `elemental`
	MODIFY `char_id` int(11) unsigned NOT NULL
;

ALTER TABLE `friends`
	MODIFY `char_id` int(11) unsigned NOT NULL default '0'
;

ALTER TABLE `friends`
	MODIFY `friend_id` int(11) unsigned NOT NULL default '0'
;

ALTER TABLE `homunculus`
	MODIFY `char_id` int(11) unsigned NOT NULL
;

ALTER TABLE `hotkey`
	MODIFY `char_id` int(11) unsigned NOT NULL
;

ALTER TABLE `mercenary`
	MODIFY `char_id` int(11) unsigned NOT NULL
;

ALTER TABLE `mercenary_owner`
	MODIFY `char_id` int(11) unsigned NOT NULL
;

ALTER TABLE `mercenary_owner`
	MODIFY `merc_id` int(11) unsigned NOT NULL default '0'
;

ALTER TABLE `skillcooldown_mercenary`
	MODIFY `mer_id` int(11) unsigned NOT NULL
;
