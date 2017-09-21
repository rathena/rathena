ALTER TABLE `char`
	MODIFY `max_hp` int(11) unsigned NOT NULL default '0',
	MODIFY `hp` int(11) unsigned NOT NULL default '0',
	MODIFY `max_sp` int(11) unsigned NOT NULL default '0',
	MODIFY `sp` int(11) unsigned NOT NULL default '0';

ALTER TABLE `elemental`
	MODIFY `hp` int(11) unsigned NOT NULL default '0',
	MODIFY `sp` int(11) unsigned NOT NULL default '0',
	MODIFY `max_hp` int(11) unsigned NOT NULL default '0',
	MODIFY `max_sp` int(11) unsigned NOT NULL default '0';

ALTER TABLE `homunculus`
	MODIFY `hp` int(11) unsigned NOT NULL default '0',
	MODIFY `max_hp` int(11) unsigned NOT NULL default '0',
	MODIFY `sp` int(11) unsigned NOT NULL default '0',
	MODIFY `max_sp` int(11) unsigned NOT NULL default '0';

ALTER TABLE `mercenary`
	MODIFY `hp` int(11) unsigned NOT NULL default '0',
	MODIFY `sp` int(11) unsigned NOT NULL default '0';
