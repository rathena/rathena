-	script	Marathon	-1,{
	function	EndGame; function	CleanAfterEvent;
	end;

OnStartEvent:
OnMinute15:
	if(agitcheck() || agitcheck2() || agitcheck3())
		end;

	if(gettime(DT_HOUR)%2 == 0)
		end;

	query_sql("SELECT count(`char`.`char_id`) FROM `char` JOIN `login` ON `login`.`account_id` = `char`.`account_id` WHERE `char`.`online` = 1 AND `char`.`base_level` > 99  AND `char`.`char_id` NOT IN (SELECT `char_id` FROM `vendings`) AND login.group_id < 20 GROUP BY `login`.`last_ip`",.@count);
	if( .@multi=(.@count[0]-5)/.Participants < 2) end;

	CleanAfterEvent();

	for(.@k=0;.@k<.@multi;.@k++){
		// Find Participant
		debugmes "Marathon : Finding Participants...";
		if(.@k == 0)
			query_sql("SELECT `char`.`char_id` FROM `char` JOIN `login` ON `login`.`account_id` = `char`.`account_id` WHERE `char`.`online` = 1 AND `char`.`base_level` > 99  AND `char`.`char_id` NOT IN (SELECT `char_id` FROM `vendings`) AND login.group_id < 20 GROUP BY `login`.`last_ip` ORDER BY RAND() LIMIT "+.Participants,.@chid$);
		else
			query_sql("SELECT `char`.`char_id` FROM `char` JOIN `login` ON `login`.`account_id` = `char`.`account_id` WHERE `char`.`online` = 1 AND `char`.`base_level` > 99  AND `char`.`char_id` NOT IN (SELECT `char_id` FROM `vendings`) AND login.group_id < 20 AND instr('"+.@bl$+"',char_id) = 0 GROUP BY `login`.`last_ip` ORDER BY RAND() LIMIT "+.Participants,.@chid$);
		
		// if(.@k == 0)
		// 	query_sql("SELECT `char`.`char_id` FROM `char` JOIN `login` ON `login`.`account_id` = `char`.`account_id` WHERE `char`.`online` = 1 AND `char`.`base_level` > 99  AND `char`.`char_id` NOT IN (SELECT `char_id` FROM `vendings`) AND login.group_id < 100 ORDER BY RAND() LIMIT "+.Participants,.@chid$);
		// else
		// 	query_sql("SELECT `char`.`char_id` FROM `char` JOIN `login` ON `login`.`account_id` = `char`.`account_id` WHERE `char`.`online` = 1 AND `char`.`base_level` > 99  AND `char`.`char_id` NOT IN (SELECT `char_id` FROM `vendings`) AND login.group_id < 100 AND instr('"+.@bl$+"',char_id) = 0 ORDER BY RAND() LIMIT "+.Participants,.@chid$);
		
		// .@count = getarraysize(.@chid$);
		// if(.@count < .Participants * .Multi)
		// 	end;

		.@count = getarraysize(.@chid$);
		copyarray .@blist$[getarraysize(.@blist$)],.@chid$[0],.Participants;
		.@bl$ = implode(.@blist$,",");
		
		// Build Group
		debugmes "Marathon : Building group " + .@k + "...";
		freeloop(1);
		// this to create the group's name
		for(.@i=0;.@i<.@count;.@i++) 
			.@gname$ += substr(strcharinfo(0,atoi(.@chid$[.@i])),0,1);
		
		// then this to populate the array with the group members
		debugmes "Marathon : Populating group " + .@k + "...";
		for(.@i=0;.@i<.@count;.@i++) {
			setarray getd("$@mt_"+.@gname$+"$["+.@i+"]"),strcharinfo(0,atoi(.@chid$[.@i]));
			//debugmes "array set : " + "$@mt_"+.@gname$+"$["+.@i+"]" + " com " + strcharinfo(0,.@chid[.@i]);
		}
		freeloop(0);

		// Define Monster
		debugmes "Marathon : Defining monster for group " + .@k + "...";
		query_sql("SELECT ID FROM `mob_db` WHERE left(Sprite, 4) != 'meta' AND left(Sprite, 2) != 'E_' AND left(Sprite, 4) != 'Gobl' AND ~Mode & 32 AND EXP > 0 AND MVP1id = 0 AND DropCardid > 4000 AND DropCardid < 5000 AND ID < 2000 AND LV <40 AND  instr('1062,1088,1089,1090,1183,1186,1200,1212,1220,1221,1234,1235,1244,1245,1247,1250,1268,1290,1293,1294,1296,1298,1299,1300,1301,1303,1304,1305,1306,1308,1309,1311,1313,1515,1582,1588,1618,1676,1677,1678,1679,1796,1797,1974,1975,1976,1977,1978,1979',ID) = 0 ORDER BY rand() LIMIT 1",.@mobid);
		setarray getd("$@mt_"+.@gname$+"_id"),.@mobid[0];

		// Start the event
		freeloop(1);
		for(.@i=0;.@i<.@count;.@i++) {
			debugmes "Marathon : Starting the Event for group " + .@k + "...";
			copyarray .@temp$[0],getd("$@mt_"+.@gname$+"$[0]"),.@count; 
			.@index = inarray(.@temp$[0],strcharinfo(0,atoi(.@chid$[.@i])));
			deletearray .@temp$[.@index],1;
			.@rivals$ = implode(.@temp$,", ");
			if(attachrid(getcharid(3,strcharinfo(0,atoi(.@chid$[.@i]))))) {
				soundeffect "choochoo.wav",1;
				dispbottom "[Marathon] You have been challenged!",0xccff99;
				dispbottom "[Marathon] Your rivals are: " + .@rivals$,0xccff99;
				dispbottom "[Marathon] You guys have 60 minutes",0xccff99;
				dispbottom "[Marathon] to bring me 20x " +getitemname(.Item)+".",0xccff99;
				dispbottom "[Marathon] This time, you will get them from " + getmonsterinfo(getd("$@mt_"+.@gname$+"_id"),MOB_NAME)+".",0xccff99;
				dispbottom "[Marathon] The event ends when the first of you collect all the items.",0xccff99;
				dispbottom "[Marathon] The winner will receive a bunch of special stuff!",0xccff99;
				dispbottom "[Marathon] Time is ticking, good luck!",0xccff99;
				set @gname$,.@gname$;
 				set @marathon,1;
				detachrid;
			}
		}
		freeloop(0);
		initnpctimer;
	}
	end;

OnTimer3600000: //60 minutos
	stopnpctimer;
	CleanAfterEvent();
	end;

OnPcLogoutEvent:
	if(countitem(.Item)>0)
		delitem .Item,countitem(.Item);
	if(!@marathon)
		end;
	.@index = inarray(getd("$@mt_"+@gname$+"$"),strcharinfo(0));
	deletearray getd("$@mt_"+@gname$+"$["+.@index+"]"),1;
	end;

OnNpcKillEvent:
	if(!@marathon) 
		end;
	if(killedrid == getd("$@mt_"+@gname$+"_id")) {
		if(rand(1000)>100) {
			getitembound .Item,1,Bound_Char;
			if(countitem(.Item) < 20) {
				announce "1 " + getitemname(.Item) + " obtained. " + (20-countitem(.Item)) +" left.",bc_self,0xccff99,FW_BOLD,28;
			} else {
				announce "MARATHON WINNER",bc_self,0xccff99,FW_BOLD,40;
				//setd "$@mt_"+@gname$+"_w$",strcharinfo(0);
				specialeffect2 1038,AREA,strcharinfo(0);
				specialeffect2 709,AREA,strcharinfo(0);
				specialeffect2 68,AREA,strcharinfo(0);
				// atcommand "@effect 1038";
				// atcommand "@effect 709";
				// atcommand "@effect 68";
				EndGame(@gname$,strcharinfo(0));
			}
		}
	}
	end;

OnInit:
	bindatcmd("start","Marathon::OnStartEvent",99,99);
	.Participants   = 3;
	.Multi			= 3;
	.Item			= 7729; //Rok Star Badge
	end;


	function	EndGame	{
		set .@gname$, getarg(0);
		set .@winner$,getarg(1);
		getitem 616,1;	 // oca
		getitem 675,15; // hunting coin
		getitem 7859,3; //Game ticket
		getitem 13517,1; // Box containing 3 ygg berry
		getitem 30023,1; // Bag of Manias
		getitem 30027,1; // Ragnamania Lootbox;
		set @marathon,0;
		set @gname$,0;
		delitem .Item,countitem(.Item);
		for(.@i=0;.@i<getarraysize(getd("$@mt_"+.@gname$+"$"));.@i++) {
			// if(getarraysize(getd("$@mt_"+.@gname$+"$")) == 0)
			// 	continue;
			set .@name$, getd("$@mt_"+.@gname$+"$["+.@i+"]");
			if( isloggedin(getcharid(3,.@name$),getcharid(0,.@name$)) ) {
				if(.@name$ != .@winner$) {
					attachrid(getcharid(3,.@name$));
					announce "Marathon is Over",bc_self,0xccff99,FW_BOLD,40;
					dispbottom "[Marathon] The winner is " + .@winner$,0xccff99;
					dispbottom "[Marathon] Thank you for participating!",0xccff99;
					getitem 7859,1; //Game ticket
					getitem 675,2; // hunting coin
					set @marathon,0;
					set @gname$,0;
					if(countitem(.Item)>0)
						delitem .Item,countitem(.Item);
					detachrid;
				}
			}
		}
		//.@index = inarray(getd("$@mt_"+@gname$+"$"),strcharinfo(0));
		deletearray getd("$@mt_"+.@gname$+"$[0]"),getarraysize(getd("$@mt_"+.@gname$+"$"));
		//setd "$@mt_"+.@gname$+"_w$","";
		return;
	}

	function	CleanAfterEvent {
		.@num = getunits(BL_PC,.@onlineplayers$[0]);
		freeloop(1);
		for(.@i=0;.@i<.@num;.@i++){
			set .@cid, getcharid(0,.@onlineplayers$[.@i]);
			if(getvar(@marathon,.@cid) == 1) {
				set .@gid, getcharid(3,.@onlineplayers$[.@i]);
				if(attachrid(.@gid)) {
					if(@marathon == 1 ) {
						dispbottom "[Marathon] Time is over! Better luck next time!",0xccff99;
						if(countitem(.Item)>0)
							delitem .Item,countitem(.Item);				
						.@index = inarray(getd("$@mt_"+@gname$+"$"),strcharinfo(0));
						deletearray getd("$@mt_"+@gname$+"$["+.@index+"]"),1;
						set @marathon,0;
						set @gname$,0;
					}
					detachrid;
				}
			}
		}
		freeloop(0);
		return;
	}
}