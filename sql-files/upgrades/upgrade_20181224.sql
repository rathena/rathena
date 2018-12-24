alter table `inventory`
	add column `equip_switch` int(11) unsigned NOT NULL default '0' after `unique_id`;
