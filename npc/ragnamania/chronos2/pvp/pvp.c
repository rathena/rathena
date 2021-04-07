prontera,152,192,5	script	Arenas	430,{
	function ValidateEntry;
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

OnPvpRanking:	//-- Menus
	.@Menu = (select("Enter the Arenas:Week's PvP Ranking:Season's Ranking"));
	switch(.@Menu){ 
		case 3: //Season Ranking
			mes "Under construction. Please come back later.";
			close;
			//SELECT char_id,SUM(score)
			//FROM `season_pvp_rank`
			//WHERE season = 1 
			//GROUP BY char_id;
		case 2: //Pvp Ranking
			mes "~ ^990099Weekly PvP Ranking^000000 ~";
			if (!getarraysize(.kills))
				mes "The rankings are empty.";
			else 
				for(.@i = 0; .@i<5; .@i++)
					if(.cname$[.@i] == "") break;
					else mes "#"+(.@i+1)+" ^0000FF"+.cname$[.@i]+" ^000000[ ^00AA00"+ .kills[.@i] +"^000000 | ^AA0000"+.deaths[.@i]+"^000000 | ^0000AA"+ .score[.@i] +"^000000 ]";
			close;

		case 1:	
			//===== Dont Touch - Menu Generation ======================
			.@m = select("^008800PvP Kitten Arena^000000 [ "+getmapusers(.MapName$[0]) + "/" + .MaxPlayers[0] + " ]:^FF0000PvP Full-Loot Arena^000000 [ "+getmapusers(.MapName$[1]) +  "/" + .MaxPlayers[1] + " ]") - 1;
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
			} else if(BaseLevel < 50) {
				mes "You must be at least Base Level 50 to enter the PvP Arenas";
				close;
			} else if(ValidateEntry() == 0) {
				mes "You are undergeared to join the Arenas.";
				close;
			//else if( .@m == 1 && (eaclass()&EAJL_UPPER || BaseLevel>99)) {
			// 	mes "You don't meet the criteria to join the Classic Arena.";
			// 	close;
			} else {
				mapannounce .MapName$[.@m], "[ PvP Warper ] A new fighter has entered the Arena",0,0x00CC99;
				announce "[ Arenas Announcer] "+strcharinfo(0)+" has entered the Arenas",bc_blue|bc_map;
				warp .MapName$[.@m],0,0;
			}
	}
	end;

OnTimer10100:
	if(.HappyHour == 1){
		misceffect 231;
		stopnpctimer;
		initnpctimer;
	}
	end;

OnPCKillEvent:	//-- Adding Stats
	if(!getmapflag(strcharinfo(3),mf_pvp)) end;
	
	if(LastPK == killedrid) 
		RepeatKill++;
	else {
		LastPK = killedrid;
		RepeatKill = 0;
	}

	//Debug
	RepeatKill = 0;

	announce "[Arenas Announcer ]: "+strcharinfo(0)+" has slain "+ rid2name(LastPK) + ((RepeatKill)?" for the "+callfunc("F_GetNumSuffix",RepeatKill)+" time in a row":"") +".",bc_all;
	if(RepeatKill < .RepeatCount) {
		.@qt = 1;
		if(.HappyHour) .@qt += 2;
		if(getmapflag(strcharinfo(3),MF_FULLLOOT)) .@qt += 5;
		getitem 7829,.@qt;
	} else {
		atcommand "@request [ Arenas Announcer ]: "+strcharinfo(0)+" is showing signs of abusing the PvP Ranking. Please Investigate";
	}
	end;


OnClock0000:
	if(gettime(DT_DAYOFWEEK) == MONDAY) {
		//Find the Winners
		query_sql("SELECT `char_pvp`.char_id, `char_pvp`.`score`, `char`.name, `char_pvp`.`kill_count`, `char_pvp`.`death_count` FROM `char_pvp` INNER JOIN `char` ON `char`.char_id = `char_pvp`.`char_id` ORDER BY `char_pvp`.`score` DESC LIMIT 5",.@cid, .@score, .@name$, .@k, .@d);
		//Distribute their prizes
		for(.@i = 0; .@i< getarraysize(.@cid); .@i++)
			mail .@cid[.@i], "Arena Master", "PvP Ranking", "Congratulations! You have placed #"+(.@i+1)+" in the PvP Ranking this Week.", .PVP_Zeny[.@i], getd(".Pitem"+.@i), getd(".Pqnt"+.@i);
		//Announce the beginning of a new sasonal round of PvP Pit Fights
		announce "[PVP Ranking]: Rewards have been mailed to the top "+getarraysize(.PVP_Items$)+" in the PvP Arenas!",bc_blue|bc_all;
		// Save the winner's numbers from this week's round for the season's rank
		query_sql("INSERT into `season_pvp_rank` VALUES ( "+.@cid[0]+","+.@score[0]+","+.@k[0]+","+.@d[0]+","+$SEASON_WEEK+","+ $SEASON_CURRENT+")");
		// resets the pvp struct of all players online
		rankreset(2);
		//Remove old records and start a new Round
		query_sql("DELETE FROM `char_pvp` WHERE 1=1");
		end;
	}
	end;

OnPcLogoutEvent:
	if(inarray(.MapName$,strcharinfo(3)) >= 0)
		callfunc "F_Dispell";
	end;	

OnInit:		//-- Config	
	questinfo QTYPE_CLICKME;
	// Lets you change NPC name without breaking anything
	.NPC$ = "^FF0000Arena Master^000000";

	// Weekly Reward Types
	// 1: Pvp
	.RewardType = 1;

	// PVP Rewards
	// Rewards are Mailed to winners
	// Zeny Rewards
	setarray .PVP_Zeny,
		25000,10000,7500,5000,1000;// 1st - 5th

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
		"607,5",// 1st
		"607,2",// 2nd
		"12405,12",// 3rd
		"12405,10", //4
		"12405,7";//5

	// Number of kills on same person before it detects abuse
	.RepeatCount = 3;

	// Map Names
	setarray .MapName$,"pvp_n_1-3","pvp_n_1-1";

	// Max Players Per Map
	// 0 = No Limit
	setarray .MaxPlayers,20,20;

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
	//  256 = Full Loot
	setarray .MapMode,8,264;

	// Mapflags for all PvP maps
	// regardless of Mode
	setarray .mapflag,
		mf_nowarp,		mf_nowarpto,		mf_nosave,
		mf_nomemo,		mf_noteleport,		mf_nopenalty,
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
		} else if(.MapMode[.@i]&256) {
			setmapflag .MapName$[.@i], MF_PVP;
			setmapflag .MapName$[.@i], MF_PVP_CONSUME;
			setmapflag .MapName$[.@i], MF_FULLLOOT;
		} else { 	
			setmapflag .MapName$[.@i], MF_PVP_NOGUILD;
			setmapflag .MapName$[.@i], MF_PVP;
			setmapflag .MapName$[.@i], MF_PVP_CONSUME;
		}
		if(.MapMode[.@i]&128)
			setmapflag .MapName$[.@i], MF_ANCIENT;
	}

	// Prepare the arrays with the items e qtty to reward the top 5 pvp players of the week  
	freeloop(1);
	for(.@i = 0; .@i < getarraysize(.PVP_Items$); .@i++){
		explode(.@convert$,.PVP_Items$[.@i],",");
		for(.@c = 0; .@c < getarraysize(.@convert$) && .@c < 5; .@c++)
			.@convert[.@c] = atoi(.@convert$[.@c]);
		for(.@k = 0; .@k < getarraysize(.@convert); .@k += 2){
			setd ".Pitem"+.@i+"["+getarraysize(getd(".Pitem"+.@i))+"]",.@convert[.@k];
			setd ".Pqnt"+.@i+"["+getarraysize(getd(".Pqnt"+.@i))+"]",.@convert[.@k+1];
		}
	}

OnMinute00:
	deletearray .kills[0],getarraysize(.kills);
	deletearray .deaths[0],getarraysize(.kills);
	deletearray .score[0],getarraysize(.score);
	deletearray .cname$[0],getarraysize(.cname$);
	query_sql("SELECT `char_pvp`.`kill_count`,`char_pvp`.`death_count`, `char_pvp`.`score`, `char`.`name` FROM `char_pvp` LEFT JOIN `char` ON `char`.`char_id` = `char_pvp`.`char_id`  ORDER BY `score` DESC LIMIT 5",.kills,.deaths,.score,.cname$);

	// Arenas Happy Hour
	if(agitcheck() || agitcheck2())	end;
	if(gettime(DT_HOUR)%2 == 0 && getusers(1) >= 50) {
		.HappyHour = 1;
		announce "[ Arenas Announcer ] The PvP Arena Happy Hour is on!",bc_blue|bc_all;
		initnpctimer;
	} else if (.HappyHour > 0) {
		.HappyHour = 0;
		announce "[ Arenas Announcer ] The PvP Arena Happy Hour is over.",bc_blue|bc_all;
	}
	end;

	function	ValidateEntry	{
		for(.@i=0;.@i<10;.@i++) {
			if(getequipid(.@i) == -1) return 0;
			if(.@i == 3) .@i = 5;
			if(.@i == 7) .@i = 8;
		}
		return 1;

	}
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
