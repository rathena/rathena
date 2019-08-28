alter table `item_db2_re`
	change `attack` `atk:matk` varchar(11) DEFAULT NULL,
	modify `equip_level` varchar(10) DEFAULT NULL
;

alter table `mob_db2`
	modify `Mode` int(11) unsigned NOT NULL default '0'
;

alter table `mob_db2_re`
	modify `Mode` int(11) unsigned NOT NULL default '0'
;
