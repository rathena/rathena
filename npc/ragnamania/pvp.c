prontera,152,192,5	script	Arenas	430,{
if(checkquest(64501) == 1){
	mes .NPC$;
	mes "Ah! "+strcharinfo(0)+" here you are!";
	mes "Here, please... take these...";
	next;
	getitem 2510,1; //Novice_Hood
	getitem 2414,1; //Novice_Boots
	getitem 5055,1; //Novice_Egg_Cap
	getitem 2352,1; //Novice_Plate
	getitem 1243,1; //Novice_Knife
	getitem 2112,1; //Novice_Guard
	completequest 64501;
	mes .NPC$;
	mes "Good luck! Hope to see you again soon!!";
	close;
}

OnPvpLadder:	//-- Menus
	.@Menu = (select("Enter the Arenas:PvP Ladder:GvG Ladder:Personal Stats"+((getgmlevel() >= 99)?":Reset PvP Stats":"")));
	switch(.@Menu){ 
		case 2:
		case 3:
			if(.@Menu == 2)
				.@key$ = "m_kda";
			else
				.@key$ = "m_gvg_kda";

			query_sql("SELECT `char_reg_num`.`value`,`char`.`name`,`char`.`char_id` FROM `char` INNER JOIN `char_reg_num` ON `char_reg_num`.`char_id`=`char`.`char_id` WHERE `char_reg_num`.`key`='" + .@key$ + "' ORDER BY `char_reg_num`.`value` DESC LIMIT 10",.@value,.@name$,.@cid);
			for(.@i=0; .@i<getarraysize(.@cid);.@i++) {
				if(.@i == 2)
					.@key$ = "m_dthcount";
				else
					.@key$ = "m_gvg_dcount";

				query_sql("SELECT CAST(`value` AS SIGNED) FROM `char_reg_num` WHERE char_id = "+.@cid[.@i]+" AND `key` = '';",.@j);
				.@Death[.@i] = .@j;
			}

			mes "~ ^990099Weekly " + ((.@Menu == 2)?"PvP":"GvG") +" Ladder^000000 ~";
			if (!getarraysize(.@cid))
				mes "The rankings are empty.";
			else 
				for(.@i = 0; .@i<5; .@i++)
					if((.@i+1) % 10 != 0)
						mes "#"+(.@i+1)+" ^0000FF"+.@name$[.@i]+" ^000000[ ^00AA00"+ .@value[.@i] +"^000000 | ^AA0000"+.@Death[.@i]+"^000000 | ^0000AA"+ (.@value[.@i] - .@Death[.@i])+"^000000 ]";
			close;
		case 4:
			mes "[PvP]";
			mes "Lifetime:","^00AA00"+ callfunc("F_InsertPlural",pl_pkcount,"kill") +"^000000 | ^AA0000"+ callfunc("F_InsertPlural",pl_dthcount,"death") + " | ^0000AA"+ kda +" Total^000000";
			mes "Weekly:","^00AA00"+ callfunc("F_InsertPlural",m_pkcount,"kill") +"^000000 | ^AA0000"+ callfunc("F_InsertPlural",m_dthcount,"death") + " | ^0000AA"+ m_kda +" Total^000000";
			next;
			mes "[GvG]";
			mes "Lifetime:","^00AA00"+ callfunc("F_InsertPlural",gvg_kcount,"kill") +"^000000 | ^AA0000"+ callfunc("F_InsertPlural",gvg_dcount,"death") + " | ^0000AA"+ gvg_kda +" Total^000000";
			mes "Weekly:","^00AA00"+ callfunc("F_InsertPlural",m_gvg_kcount,"kill") +"^000000 | ^AA0000"+ callfunc("F_InsertPlural",m_gvg_dcount,"death") + " | ^0000AA"+ m_gvg_kda +" Total^000000";
			close;

		case 5:
			.@L$[1] = "Pvp Ladder";
			.@L$[2] = "GvG Ladder";
			.@j = select("Pvp Ladder:GvG Ladder");
			mes "Are you sure you want to reset this the "+.@L$[.@j]+"?";
			if(select("No:Yes")!=2)
				close;
			close2;
			switch(.@j) {
				case 1: //PvP Weekly
					query_sql("DELETE FROM `char_reg_num` WHERE `key` = 'm_pkcount' OR `key` = 'm_kda' OR `key` = 'pl_dthcount'");
					debugmes "PvP Ranking : Weekly PvP Ladder wiped.";
					addrid(0);
					m_pkcount = m_kda = m_dthcount = 0;
					break;
				case 2: //Gvg Weekly
					query_sql("DELETE FROM `char_reg_num` WHERE `key` = 'm_gvg_kcount' OR `key` = 'm_gvg_kda' OR `key` = 'm_gvg_dcount'");
					debugmes "PvP Ranking : Weekly PvP Ladder wiped.";
					addrid(0);
					m_gvg_kcount = m_gvg_kda = m_gvg_dcount = 0;
					break;
			}
			end;

		case 1:	//===== Dont Touch - Menu Generation ======================
		//.@m = select("PvP Arena [ "+getmapusers(.MapName$[0])+" ]:PvP Classic Arena [ "+getmapusers(.MapName$[1])+" ]:GvG Arena [ "+getmapusers(.MapName$[2])+" ]") - 1;
		.@m = select("PvP Arena [ "+getmapusers(.MapName$[0])+" ]:GvG Arena [ "+getmapusers(.MapName$[2])+" ]") - 1;
			if(.MaxPlayers[.@m] && getmapusers(.MapName$[.@m]) >= .MaxPlayers[.@m]){
				mes "Room is Full.";
				close;
			} else if(.MapMode[.@m]&4 && getcharid(1) <= 0 ) {
				mes "You must be in a Party to join.";
				close;
			} else if(.MapMode[.@m]&1 && getcharid(2) <= 0 ) {
				mes "You must be in a Guild to join.";
				close;
			} else if(BaseLevel < 50 && .@m != 1) {
				mes "You must be Base Level 50 or more to join.";
				close;
			// } else if(BaseLevel > 50 && .@m == 1) {
			// 	mes "You must be Base Level 99 or less to join.";
			// 	close;
			// } else if( .@m == 1 && (eaclass()&EAJL_UPPER || BaseLevel>99)) {
			// 	mes "You don't meet the criteria to join the Classic Arena.";
			// 	close;
			} else {
				mapannounce .MapName$[.@m], "[ PvP Warper ] A new fighter has entered the Arena",0,0x00CC99;
				announce "[ Arenas Announcer] "+strcharinfo(0)+" has entered the Arenas",bc_blue|bc_map;
				warp .MapName$[.@m],0,0;
			}
	}
	end;
// OnMinute00:
// 	if(agitcheck() || agitcheck2())
// 		end;
// 	if(gettime(DT_HOUR)%2 == 0) {
// 		if(getusers(1) >= 180){
// 			.GvGHappyHour = 1;
// 			announce "[ Arenas Announcer ] The GvG Arena Happy Hour is on!",bc_blue|bc_all;
// 			initnpctimer;
// 		}
// 	} else {
// 		if (.GvGHappyHour > 0) {
// 			.GvGHappyHour = 0;
// 			announce "[ Arenas Announcer ] The GvG Arena Happy Hour is over.",bc_blue|bc_all;
// 		}
// 	}
// 	end;

// OnTimer10100:
// 	if(.GvGHappyHour == 1){
// 		misceffect 231;
// 		stopnpctimer;
// 		initnpctimer;
// 	}
// 	end;
OnPCKillEvent:	//-- Adding Stats
	if(strcharinfo(3) != "pvp_n_1-3" && strcharinfo(3) != "guild_vs2")
		end;

	// Death Sick - Nao conta para calculos do ranking nem dropa badges
	.@charid = getcharid(0,rid2name(killedrid));
	if(getvar(#KILLSICK,.@charid) > gettimetick(2)) {
	 	dispbottom "Arenas Announcer ]: This player grants no honor at the present.";
	 	end;
	} else {
		if(LastPK == killedrid) 
			RepeatKill++;
		else {
			LastPK = killedrid;
			RepeatKill = 0;
		}
		announce "[Arenas Announcer ]: "+strcharinfo(0)+" has slain "+ rid2name(LastPK) + ((RepeatKill)?" for the "+callfunc("F_GetNumSuffix",RepeatKill)+" time in a row":"") +".",bc_all;
		if(RepeatKill < .RepeatCount) {
			pl_pkcount++;
			m_pkcount++;
			kda++;
			m_kda++;
			if((getmapflag(strcharinfo(3),MF_GVG) || getmapflag(strcharinfo(3),MF_GVG_DUNGEON) || getmapflag(strcharinfo(3),MF_GVG_CASTLE)) && getcharid(2)) {
				gvg_kcount++;
				m_gvg_kcount++;
				gvg_kda++;
				m_gvg_kda++;
				// if (.GvGHappyHour == 1)
				// 	getitem 7773,5;
				// else
					getitem 7773,3;
			} else
				getitem 7829,1;

			// Sick no killer
			set .@sick, gettimetick(2) + 60;
			set #KILLSICK,.@sick;

			attachrid(LastPK);
			                                                                                                                                                                                                                                                                                                                           
			pl_dthcount++;
			m_dthcount++;
			kda -= 1;
			m_kda -= 1;
			if((getmapflag(strcharinfo(3),MF_GVG) || getmapflag(strcharinfo(3),MF_GVG_DUNGEON) || getmapflag(strcharinfo(3),MF_GVG_CASTLE)) && getcharid(2)) {
				gvg_dcount++;
				m_gvg_dcount++;
				gvg_kda -= 1;
				m_gvg_kda -= 1;
				$Guild_Deaths[getcharid(2)]++;
				$M_Guild_Deaths[getcharid(2)]++;
				$Guild_Kda[getcharid(2)] -= 1;
				$M_Guild_Kda[getcharid(2)] -= 1;
			}
		} else 
			atcommand "@request [ Arenas Announcer ]: "+strcharinfo(0)+" is showing signs of abusing the PvP Ladder Please Investigate";
	}
	end;


OnHour00:	//-- Weekly Rewards
	//if(gettime(DT_DAYOFMONTH) == 1) {
	if(gettime(DT_DAYOFWEEK) == MONDAY) {
		// if(.RewardType&1){
			query_sql("SELECT `char_reg_num`.char_id,`char`.name,CAST(`value` AS SIGNED) FROM `char_reg_num` INNER JOIN `char` ON `char`.char_id = `char_reg_num`.char_id WHERE `key` = 'm_kda' ORDER BY CAST(`value` AS SIGNED) DESC LIMIT "+getarraysize(.PVP_Items$)+"",.@cid,.@name$,.@value);
			for(.@i = 0; .@i< getarraysize(.@cid); .@i++)
				mail .@cid[.@i], "no-reply", "PvP Ranking", "Congratulations!% You have placed #"+(.@i+1)+" in the PvP Ranking this Month % % %[ Your reward is attached. ]", .PVP_Zeny[.@i], getd(".Pitem"+.@i), getd(".Pqnt"+.@i);
			announce "[PVP Ranking]: Rewards have been mailed to the top "+getarraysize(.PVP_Items$)+" in PvP~!",bc_blue|bc_all;
			query_sql("UPDATE `mail` SET message = REPLACE(message,'%',CHAR(13)) WHERE send_name = 'no-reply'");
		// }
		// sleep 10000;
		// if(.RewardType&2){
		// 	query_sql("SELECT char_id,`char`.name,CAST(`value` AS SIGNED) FROM `char_reg_num` INNER JOIN `char` ON `char`.char_id = `char_reg_num`.char_id WHERE `key` = 'm_gvg_kda' ORDER BY CAST(`value` AS SIGNED) DESC LIMIT "+getarraysize(.GVG_Items$)+"",.@cid,.@name$,.@value);
		// 	for(.@i = 0; .@i<getarraysize(.@cid); .@i++) 
		// 		mail .@cid[.@i], "no-reply", "GvG Ranking", "Congratulations!% You have placed #"+(.@i+1)+" in the GvG Ranking this Month % % %[ Your reward is attached. ]", .GVG_Zeny[.@i], getd(".Gitem"+.@i), getd(".Gqnt"+.@i);
		// 	announce "[GVG Ranking]: Rewards have been mailed to the top "+getarraysize(.GVG_Items$)+" in GvG~!",bc_blue|bc_all;
		// 	query_sql("UPDATE `mail` SET message = REPLACE(message,'%',CHAR(13)) WHERE send_name = 'no-reply'");
		// }

		query_sql("DELETE FROM `char_reg_num` WHERE `key` = 'm_pkcount' OR `key` = 'm_kda' OR `key` = 'm_dthcount' OR `key` = 'm_gvg_kcount' OR `key` = 'm_gvg_kda' OR `key` = 'm_gvg_dcount'");
		addrid(0);
		m_pkcount = 0;
		m_kda = 0;
		m_dthcount = 0;
		m_gvg_kcount = 0;
		m_gvg_kda = 0;	
		m_gvg_dcount = 0;
		end;
	}
	end;

OnPcLogoutEvent:
	if(inarray(.MapName$,strcharinfo(3)) >= 0)
		callfunc "F_Dispell";
	end;	

OnInit:		//-- Config
	// Biali Fix:
	cleararray $M_Guild_Kills[0],0,getarraysize($M_Guild_Kills);
	cleararray $M_Guild_Kda[0],0,getarraysize($M_Guild_Kda);
	cleararray $M_Guild_Deaths[0],0,getarraysize($M_Guild_Deaths);
	cleararray $Guild_Kills[0],0,getarraysize($Guild_Kills);
	cleararray $Guild_Kda[0],0,getarraysize($Guild_Kda);
	cleararray $Guild_Deaths[0],0,getarraysize($Guild_Deaths);
	
	questinfo QTYPE_CLICKME;
	// Lets you change NPC name without breaking anything
	.NPC$ = "^FF0000Arena Master^000000";

	// Weekly Reward Types
	// 1: Pvp
	// 2: Gvg
	.RewardType = 1;

	// PVP Rewards
	// Rewards are Mailed to winners
	// Zeny Rewards
	setarray .PVP_Zeny,
		250000,100000,75000,50000,10000;// 1st - 5th
		//   100000, 100000,  100000,  100000, 100000;// 6th - 10th

	setarray .GVG_Zeny,
		250000,100000,75000,50000,10000;// 1st - 5th
		//   100000, 100000,  100000,  100000, 100000;// 6th - 10th

	// Ro Dex Supports 5 Items
	// Old Mail DOES NOT support multiple items
	// "Item 1, Qnt 1, Item 2, Qnt 2, Item 3, Qnt 3, Item 4, Qnt 4, Item 5, Qnt 5"
		  //607 yggdrasil berry
		  //12549 box with 200 white slim pot
		  //616 OCA
		  //12103 Bloody Branch
		  //13582 Convex Mirror box with 5
		  //12405 yggdrasil seed
		  //547 condensed white potion
		  //12109 Poring Box
		  //12214 Convex Mirror
		  //13889 Elunion box x10 elu
	setarray .PVP_Items$,
		"607,5,12549,1,616,3,12103,1,13582,1",// 1st
		"607,2,12549,1,616,2,12103,1,13582,1",// 2nd
		"12405,12,547,20,616,1,12103,1,13582,1",// 3rd
		"12405,10,547,15,616,1,12109,1,13582,1", //4
		"12405,7,547,10,616,1,12109,1,12214,1";//5
		// "12405,20,547,20,616,1,12109,1,12214,1",
		// "12405,20,547,20,616,1,12109,1,12214,1",
		// "12405,20,547,20,616,1,12109,1,12214,1",
		// "12405,20,547,20,616,1,12109,1,12214,1", 
		// "12405,20,547,20,616,1,12109,1,12214,1"; // 10th

	setarray .GvG_Items$,
		"607,5,12549,1,616,3,12103,1,13582,1",// 1st
		"607,2,12549,1,616,2,12103,1,13582,1",// 2nd
		"12405,12,547,20,616,1,12103,1,13582,1",// 3rd
		"12405,10,547,15,616,1,12109,1,13582,1", //4
		"12405,7,547,10,616,1,12109,1,12214,1";//5
		// "12405,20,547,20,616,1,12109,1,12214,1",
		// "12405,20,547,20,616,1,12109,1,12214,1",
		// "12405,20,547,20,616,1,12109,1,12214,1",
		// "12405,20,547,20,616,1,12109,1,12214,1", 
		// "12405,20,547,20,616,1,12109,1,12214,1"; // 10th

	// Number of kills on same person before it detects abuse
	.RepeatCount = 5;

	// Players Shown in PvP Ladder
	.MaxRank = 10;

	// Map Names
	setarray .MapName$,"pvp_n_1-3","pvp_n_1-1","guild_vs2";


	// Max Players Per Map
	// 0 = No Limit
	setarray .MaxPlayers,0,0,0;

	// Pvp Mode
	// Modes are Addative
	//	0 = Normal Pvp
	//	1 = Gvg
	//	2 = Nightmare
	//	4 = Party
	//	8 = No Ygg
	//	16 = No 3rds
	//	32 = No Trans
	//	64 = Show Map Name
	//  128 = Classic
	setarray .MapMode,0,128,1;

	// Mapflags for all PvP maps
	// regardless of Mode
	setarray .mapflag,
		mf_nowarp,	mf_nowarpto,		mf_nosave,
		mf_nomemo,	mf_noteleport,		mf_nopenalty,
		mf_noreturn,	mf_nobranch,		mf_nomobloot,
		mf_nomvploot,	MF_NOZENYPENALTY,	MF_NOTRADE,
		mf_nodrop;

//===== Dont Touch - Mapflag & Waiting Room Generation =====
	for(.@i = 0; .@i < getarraysize(.MapName$); .@i++){
		for ( .@f = 0; .@f < getarraysize(.mapflag); .@f++ )
			setmapflag .MapName$[.@i], .mapflag[.@f];

		if(.MapMode[.@i]&1){
			setmapflag .MapName$[.@i], MF_GUILDLOCK;
			setmapflag .MapName$[.@i], MF_GVG;
			setmapflag .MapName$[.@i], MF_WOE_CONSUME;
		} else{
			setmapflag .MapName$[.@i], MF_PVP_NOGUILD;
			setmapflag .MapName$[.@i], MF_PVP;
			setmapflag .MapName$[.@i], MF_PVP_CONSUME;
		}
		if(.MapMode[.@i]&128)
			setmapflag .MapName$[.@i], MF_ANCIENT;

	}
	freeloop(1);
	for(.@i = 0; .@i < getarraysize(.PVP_Items$); .@i++){
		explode(.@convert$,.PVP_Items$[.@i],",");
		for(.@c = 0; .@c < getarraysize(.@convert$) && .@c < 10; .@c++)
			.@convert[.@c] = atoi(.@convert$[.@c]);
		for(.@k = 0; .@k < getarraysize(.@convert); .@k += 2){
			setd ".Pitem"+.@i+"["+getarraysize(getd(".Pitem"+.@i))+"]",.@convert[.@k];
			setd ".Pqnt"+.@i+"["+getarraysize(getd(".Pqnt"+.@i))+"]",.@convert[.@k+1];
		}
	}
	for(.@i = 0; .@i < getarraysize(.GVG_Items$); .@i++){
		explode(.@convert$,.GVG_Items$[.@i],",");
		for(.@c = 0; .@c < getarraysize(.@convert$) && .@c < 10; .@c++)
			.@convert[.@c] = atoi(.@convert$[.@c]);
		for(.@k = 0; .@k < getarraysize(.@convert); .@k += 2){
			setd ".Gitem"+.@i+"["+getarraysize(getd(".Gitem"+.@i))+"]",.@convert[.@k];
			setd ".Gqnt"+.@i+"["+getarraysize(getd(".Gqnt"+.@i))+"]",.@convert[.@k+1];
		}
	}

	// if(gettime(DT_HOUR)%2 == 0) {
	// 	.GvGHappyHour = 1;
	// 	initnpctimer;
	// 	misceffect 231;
	// } else {
	// 	.GvGHappyHour = 0;
	// }
	end;
}

-	script	#pvpwarp	-1,2,2,{
	end;
OnTouch:
	progressbar "0xFF0000",5;
	warp "SavePoint",0,0;
	end;
}
//pvp
pvp_n_1-3,48,99,0	duplicate(#pvpwarp)	#pvpwarp1	406,2,2
pvp_n_1-3,99,149,0	duplicate(#pvpwarp)	#pvpwarp2	406,2,2
pvp_n_1-3,151,99,0	duplicate(#pvpwarp)	#pvpwarp3	406,2,2
pvp_n_1-3,100,48,0	duplicate(#pvpwarp)	#pvpwarp4	406,2,2

//gvg
guild_vs2,7,49,0	duplicate(#pvpwarp)	#gvgwarp1	406,2,2
guild_vs2,50,92,0	duplicate(#pvpwarp)	#gvgwarp2	406,2,2
guild_vs2,92,49,0	duplicate(#pvpwarp)	#gvgwarp3	406,2,2
guild_vs2,50,7,0	duplicate(#pvpwarp)	#gvgwarp4	406,2,2

//classic
pvp_n_1-1,100,15,0	duplicate(#pvpwarp)	#pvpwarp_c1	406,2,2
pvp_n_1-1,102,182,0	duplicate(#pvpwarp)	#pvpwarp_c2	406,2,2