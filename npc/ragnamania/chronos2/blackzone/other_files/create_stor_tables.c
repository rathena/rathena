-	script	create_tables	-1,{
	end;
	OnInit:
		setarray .maps$[0], "ars_fild04","ars_fild27","ars_fild16","ars_fild09","ars_fild26","ars_fild10","ars_fild13";

		for(.@j=0;.@j<getarraysize(.maps$);.@j++) {
			for(.@i=3;.@i<=5;.@i++){
				query_sql("CREATE TABLE `stor_"+ .maps$[.@j] +"_char_"+ .@i +"` (`id` int(11) UNSIGNED NOT NULL,`account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',`nameid` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',`equip` int(11) UNSIGNED NOT NULL DEFAULT '0',`identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',`refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',`attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',`card0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`card1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`card2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`card3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',`option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',`option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',`option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',`option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',`option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',`expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',`bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',`unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0') ENGINE=MyISAM DEFAULT CHARSET=utf8;");
				query_sql("ALTER TABLE `stor_"+ .maps$[.@j]+"_char_"+.@i+"` ADD PRIMARY KEY (`id`), ADD KEY `account_id` (`account_id`);");
			}
		}
	end;
}