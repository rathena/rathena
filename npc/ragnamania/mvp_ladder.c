prontera,164,175,3	script	MvP Ladder	1_M_KNIGHTMASTER,{
	.@md_name$ = "MvP Ladder";
	mes "[MvP Ladder Warper]";
	mes "Say... do you want to play the MvP Ladder game?";
	next;
	switch(select("Yes, let's get it on!:Information.:Show me the best record.:No.")) {
	case 1:
		.@pid = getcharid(1);
		if (!.@pid) {
			mes "[MvP Ladder Warper]";
			mes "You have to be in a party to play.";
			close;
		}
			.@menu$ = "Generate the MVP Ladder:Enter the MVP Ladder";
		mes "[MvP Ladder Warper]";
		mes "Okay, are you ready?";
		next;
		switch(select(.@menu$)) {
			case 1:
				if(MvP_C_Out > gettimetick(2)) {
					mes "You have to wait  "+ callfunc("__Time2Str",MvP_C_Out)+" hours until you can create the MvP Ladder again.";
					close;
				}
				mes "[MvP Ladder Warper]";
				if (getpartyleader(.@pid, 2) != getcharid(0)) // Sanity check
					close;
				if(!instance_check_party(.@pid,.register_min)) {
					mes "You don't have enough party member to join MVP Ladder.";
					close;
				}
				if (.register_cost && Zeny < .register_cost) {
					mes "You don't have enough zeny. Please come back when you do.";
					close;
				}

				.@instance = instance_create( .@md_name$, getcharid( 1 ), IOT_PARTY);
				if( .@instance < 0 ) { 
					mes "Party Name: "+ getpartyname(.@pid);
					mes "Party Leader: "+strcharinfo(0);
					mes "^0000ffMVP Ladder ^000000- Reservation Failed!";
					close;
				}
				//if( instance_attachmap("1@mir",.@instance) == "" ) {
				instance_destroy( .@instance );
				mes "Failed to instance ^0000ffMVP Ladder ^000000";
				close;
	
				MvP_C_Out = gettimetick(2) + .enter_cooldown;

				Zeny -= .register_cost;
				//instance_set_timeout( 7200, 300, .@instance ); // 3 Hours to complete
//				instance_init( .@instance );
				//instance_attach .@instance;
				announce "The party ["+ strcharinfo(1) +"] has initiated the MvP ladder instance.", bc_all;
				mes "The MVP Ladder instance has been generated.";
				close;
			case 2:
				if(MvP_Timeout > gettimetick(2)) {
					mes "You have to wait " + callfunc("__Time2Str",MvP_Timeout)+" hours until you can enter the MvP Ladder again.";
					close;
				}
				//if ( has_instance("1@mir") != "" ) {
				if ('has_instance) { //'
					mapannounce "prontera",strcharinfo(0)+", member of the party "+ getpartyname(.@pid) +" has entered the MVP Ladder",bc_map,"0x00ff99";
					MvP_Timeout = gettimetick(2) + .enter_cooldown;
					warp instance_mapname("1@mir",.@instance ),101,95;
					end;
				} else if ( !'has_instance ){ //'
						mes "Instance not generated yet.";
						close;
				}
			break;
		}
	case 2:
		mes "[MvP Ladder Warper]";
		mes "In this game, your party has to kill every single MvP monster in ascending order, from the weakest to strongest.";
		if ( .register_cost )
			mes "The entrance fee is "+ callfunc( "F_InsertComma", .register_cost ) +" zeny.";
		next;
		mes "[MvP Ladder Warper]";
		mes "You lose the game if you can't finish in 60 minutes, or if your entire party is killed.";
		mes "Good luck!";
		close;
	case 3:
		mes "[MvP Ladder Warper]";
		if ( !$mvpladdderparty_time ) {
			mes "Nobody has finished this game yet.";
			close;
		}
		mes "The best record is";
		mes "[ "+( $mvpladdderparty_time / 60 )+" min "+( $mvpladdderparty_time % 60 )+" sec ]";
		mes "By the party ^FF0000"+ $mvpladdderparty_name$ +"^000000.";
		.@size = getarraysize( $mvpladderparty_member$ );
		for ( .@i = 0; .@i < .@size; .@i++ )
			mes "^000000"+ ( .@i +1 ) +". ^0000FF"+ $mvpladderparty_member$[.@i];
		if ( getgmlevel() < .gmlvlreset ) close;
		next;
		if ( select( "Close.", "Reset the record." ) == 1 ) close;
		if ( select( "Never mind.", "I really want to reset it." ) == 1 ) close;
		$mvpladdderparty_time = 0;
		$mvpladdderparty_name$ = "";
		deletearray $mvpladderparty_member$[.@i];
		mes "[MvP Ladder Warper]";
		mes "Record reset successfully.";
		close;
	case 4:
		mes "[MvP Ladder Warper]";
		mes "When you are strong enough to complete the game, please come back.";
		close;
	}
	end;
OnInit:
	// Cooldown of entering in seconds
	.enter_cooldown = 86400;
	
	// entrance fee (in Zeny)
	.register_cost = 300000;

	// amount of party members needed to start the game
	.register_min = 1;

	// minimum GM level to reset the best record
	.gmlvlreset = 99;

	// mapflag configuration
	setarray .@mapflag,
		mf_nowarp,
		mf_nowarpto,
		mf_nosave,
		mf_nomemo,
		mf_noteleport,
		mf_monster_noteleport, // disable teleporting for monsters
//		mf_nopenalty, // disable exp loss
		mf_noreturn,
//		mf_nobranch,
//		mf_nomobloot, // disable monster drop loots,
//		mf_nomvploot, // 2 of these
		mf_partylock;

	.@size = getarraysize( .@mapflag );
	for ( .@i = 0; .@i < .@size; .@i++ )
		setmapflag "1@mir", .@mapflag[.@i];
	end;
}

1@mir,0,0,0	script	#SCMLControl	EF_NONE,{ // Ugly :P
	end;
OnInstanceInit:
	// id of each mvp written as string. You can make the script randomize mob ids by putting | as delimiter.
	setarray 'mvpid$[0],
		"1086",//	Golden Thief Bug	64
		"1115",//	Eddga				65
		"1150",//	Moonlight Flower	67
		"1159",//	Phreeoni			69
		"1112",//	Drake				70
		"1583",//	Tao Gunka			70
		"1492",//	Incantation Samurai	71
		"1046",//	Doppelgangger		72
		"1252",//	Garm				73
		"1418",//	Evil Snake Lord		73
		"1059",//	Mistress			74
		"1190",//	Orc Lord			74
		"1087",//	Orc Hero			77
		"1251",//	Knight of Windstorm	77
		"1038",//	Osiris				78
		"1658",//	Ygnizem				79
		"1272",//	Dark Lord			80
		"1871",//	Fallen Bishop		80
		"1039",//	Baphomet			81
		"1147",//	Maya				81
		"1785",//	Atroce				82
		"1389",//	Dracula				85
		"1630",//	Bacsojin			85
		"1885",//	Gorynych			85
		"1623",//	RSX 0806			86
		"1511",//	Amon Ra				88
		"1688",//	Lady Tanee			89
		"1768",//	Gloom Under Night	89
		"1719",//	Datale				90
		"1734",//	Kiel D-01			90
		"1157",//	Pharaoh				93
		"1373",//	Lord of Death		94
		"1312",//	Turtle General		97
		"1779",//	Ktullanux			98
		"1874",//	Beelzebub			98
		"1646|1647|1648|1649|1650|1651",	// 	Bio3 bosses		    99
		"1708",//	Thanatos			99
		"1751",//	Valkyrie Randgris	99
		"1832";//	Ifrit				99

	// number of rounds (default: 39)
	'mvpamount = getarraysize('mvpid$)-1;
	'round = -1;

	// item reward for completing each round
	'round_item_id = 675;
	'round_item_amount = 5;

	// item reward for completing the entire ladder
	'finish_item_id = 675;
	'finish_item_amount = 50;

	// bonus reward if ladder completed within a certain time (in minutes)
	'bonus_time = 30; // if completed within 45 minutes, this reward is given
	'bonus_item_id = 675;
	'bonus_item_amount = 20;

	// time delay between rounds, in seconds (default: 3)
	'delay = 3;
	killmonsterall instance_mapname("1@mir");
	initnpctimer;
	end;

OnTimer1000:
	if('state && !getmapusers(instance_mapname("1@mir"))){
		initnpctimer;
		end;
	}
	sleep 5000;
	instance_announce 0,"The MVP Ladder is going to start in 30 seconds. Prepare yourself!",bc_map|bc_blue;
	sleep 20000;
	instance_announce 0,"The MVP Ladder is going to start in 10 seconds.",bc_map|bc_blue;
	sleep 5000;
	instance_announce 0,"The MVP Ladder is going to start in 5 seconds.",bc_map|bc_blue;
	sleep 1000;
	instance_announce 0,"The MVP Ladder is going to start in 4 seconds.",bc_map|bc_blue;
	sleep 1000;
	instance_announce 0,"The MVP Ladder is going to start in 3 seconds.",bc_map|bc_blue;
	sleep 1000;
	instance_announce 0,"The MVP Ladder is going to start in 2 seconds.",bc_map|bc_blue;
	sleep 1000;
	instance_announce 0,"The MVP Ladder is going to start in a second.",bc_map|bc_blue;
	sleep 1000;
	instance_announce 0,"The MVP Ladder has started!",bc_map|bc_blue;
	'time_enter = gettimetick(2);
	'state = 1;
	stopnpctimer;
	// Fall through
OnMyMobDead:
	++'round;
	if('round == 'mvpamount) {
		.@party_id = getcharid(1);
		getpartymember .@party_id, 1;
		getpartymember .@party_id, 2;
		instance_announce 0, "Congratulations... You were able to defeat all the MVPs!", bc_map;
		for ( .@i = 0; .@i < $@partymembercount; .@i++ ) {
			if ( isloggedin( $@partymemberaid[.@i], $@partymembercid[.@i] ) ) {
				attachrid $@partymemberaid[.@i];
				if ( strcharinfo(3) == instance_mapname("1@mir") )
					getitem 'finish_item_id, 'finish_item_amount;
			}
		}
		.@timeused = gettimetick(2) - 'time_enter;
		if ( 'bonus_item_amount && .@timeused < 'bonus_time * 60 ) {
			instance_announce 0, "You are rewarded a bonus item for completing the ladder within "+ 'bonus_time +" minutes.", bc_map;
			for ( .@i = 0; .@i < $@partymembercount; .@i++ ) {
				if ( isloggedin( $@partymemberaid[.@i], $@partymembercid[.@i] ) ) {
					attachrid $@partymemberaid[.@i];
					if ( strcharinfo(3) == instance_mapname("1@mir") ){
						getitem 'bonus_item_id, 'bonus_item_amount;
						.@name$[.@c++] = strcharinfo(0);
					}
				}
			}
		}

		if ( !$mvpladdderparty_time || .@timeused < $mvpladdderparty_time ) {
			instance_announce 0, "And you broke the record! [ "+( .@timeused / 60 )+" min "+( .@timeused % 60 )+" sec ]", bc_map;
			$mvpladdderparty_time = .@timeused;
			$mvpladdderparty_name$ = getpartyname( .@party_id );
			copyarray $mvpladderparty_member$, .@name$, .@c;
		}
		else
			instance_announce 0, "Time used [ "+( .@timeused / 60 )+" min "+( .@timeused % 60 )+" sec ]", bc_map;
		sleep 10000;
		announce "The party ["+ getpartyname( .@party_id ) +"] has finished the MvP ladder game!", bc_all;
		instance_warpall "prontera",163,169;
		instance_destroy();
		end;
	}
	instance_announce 0,"A new MVP will spawn at the center of the map in " + 'delay + " seconds. Prepare yourself!",bc_map|bc_blue;
	sleep 'delay * 1000;
	explode(.@mobids$,'mvpid$['round],"|");
	.@mobid = atoi(.@mobids$[rand(getarraysize(.@mobids$))]);
	monster instance_mapname("1@mir"),101,76,"--ja--",.@mobid,1,instance_npcname("#SCMLControl") + "::OnMyMobDead";
	end;
}

function	script	__Time2Str	{
	set .@time_left, getarg(0) - gettimetick(2);
	
	set .@Days, .@time_left / 86400;
	set .@time_left, .@time_left - (.@Days * 86400);
	set .@Hours, .@time_left / 3600;
	set .@time_left, .@time_left - (.@Hours * 3600);
	set .@Minutes, .@time_left / 60;
	set .@time_left, .@time_left - (.@Minutes * 60);
	
	set .@Time$, "";
	if( .@Days > 1 )
		set .@Time$, .@Time$ + .@Days + " days, ";
	else if( .@Days > 0 )
		set .@Time$, .@Time$ + .@Days + " day, ";

	if( .@Hours > 1 )
		set .@Time$, .@Time$ + .@Hours + " hours, ";
	else if( .@Hours > 0 )
		set .@Time$, .@Time$ + .@Hours + " hour, ";

	if( .@Minutes > 1 )
		set .@Time$, .@Time$ + .@Minutes + " minutes, ";
	else if( .@Minutes > 0 )
		set .@Time$, .@Time$ + .@Minutes + " minute, ";

	if( .@time_left > 1 || .@time_left == 0 )
		set .@Time$, .@Time$ + .@time_left + " seconds";
	else if( .@time_left == 1 )
		set .@Time$, .@Time$ + .@time_left + " second";
	
	return .@Time$;
}