
// Information NPC
//============================================================
-	script	#terriInfo	-1,{
	//@terriData$[0] = Region ID
	//@terriData$[1] = castle map
	explode(@terriData$,strnpcinfo(2),"-");

	doevent "TERRI_CONTROL$::OnMenu";
	end;
}

// Script Core
//============================================================
-	script	TERRI_CONTROL$	-1,{
	function Add_Zero;
	function Region;
	function Castle;
	function Weekday2Str; //Returns weekday as Str. Parameters: [0] Weekday as Int
	function Weekday; // Returns Weekday as a number. Parameters: [0] Weekday as Str
	function DaysMenu;
	function CheckBattleEnd;
	function CastleMapName2CastleId;
	function BuildSchedule;
	function AttackingGuild;
	function ClearSchedule;

OnInit:

	//BIALI DEBUG
	// deletearray $TERRI_CONTROL$[0],getarraysize($TERRI_CONTROL$);

// -----------------------------------------------------------
//  Configuration settings.
// -----------------------------------------------------------

	set .AutoKick,1;		// Automatically kick players from inactive castles during WOE? (1:yes / 0:no)
	set .NoOwner,1; 		// Automatically kick players from unconquered castles outside of WOE? (1:yes / 0:no)

	set .Cost2attack, 8000000;
	set .BattleDuration, 20;
	setarray .PrimeTime[0],16,12,19,1; //Prontera,Payon,Geffen,Al de Baran

	// Initializers
	.total_castle =1;


// -----------------------------------------------------------
//  Castles data.
// -----------------------------------------------------------

// Region function defines the region of the castle defined by Castle function.
// Region("<region name>");
// Castle("<castle map name>", "<event name when woe end>", "<event killmonster name>", "<map name special warp>",<x>,<y>)
	Region("Prontera");
		Castle("prtg_cas01", "Agit#prtg_cas01::OnAgitEnd", "Agit#prtg_cas01::OnAgitBreak", "prt_gld",134,65);
		Castle("prtg_cas02", "Agit#prtg_cas02::OnAgitEnd", "Agit#prtg_cas02::OnAgitBreak", "prt_gld",240,128);
		Castle("prtg_cas03", "Agit#prtg_cas03::OnAgitEnd", "Agit#prtg_cas03::OnAgitBreak", "prt_gld",153,137);
		Castle("prtg_cas04", "Agit#prtg_cas04::OnAgitEnd", "Agit#prtg_cas04::OnAgitBreak", "prt_gld",111,240);
		Castle("prtg_cas05", "Agit#prtg_cas05::OnAgitEnd", "Agit#prtg_cas05::OnAgitBreak", "prt_gld",208,240);
	Region("Payon");
		Castle("payg_cas01", "Agit#payg_cas01::OnAgitEnd", "Agit#payg_cas01::OnAgitBreak", "pay_gld",121,233);
		Castle("payg_cas02", "Agit#payg_cas02::OnAgitEnd", "Agit#payg_cas02::OnAgitBreak", "pay_gld",295,116);
		Castle("payg_cas03", "Agit#payg_cas03::OnAgitEnd", "Agit#payg_cas03::OnAgitBreak", "pay_gld",317,293);
		Castle("payg_cas04", "Agit#payg_cas04::OnAgitEnd", "Agit#payg_cas04::OnAgitBreak", "pay_gld",140,160);
		Castle("payg_cas05", "Agit#payg_cas05::OnAgitEnd", "Agit#payg_cas05::OnAgitBreak", "pay_gld",204,266);
	Region("Geffen");
		Castle("gefg_cas01", "Agit#gefg_cas01::OnAgitEnd", "Agit#gefg_cas01::OnAgitBreak", "gef_fild13",214,75);
		Castle("gefg_cas02", "Agit#gefg_cas02::OnAgitEnd", "Agit#gefg_cas02::OnAgitBreak", "gef_fild13",308,240);
		Castle("gefg_cas03", "Agit#gefg_cas03::OnAgitEnd", "Agit#gefg_cas03::OnAgitBreak", "gef_fild13",143,240);
		Castle("gefg_cas04", "Agit#gefg_cas04::OnAgitEnd", "Agit#gefg_cas04::OnAgitBreak", "gef_fild13",193,278);
		Castle("gefg_cas05", "Agit#gefg_cas05::OnAgitEnd", "Agit#gefg_cas05::OnAgitBreak", "gef_fild13",305,87);
	Region("Aldebaran");
		Castle("aldeg_cas01", "Agit#aldeg_cas01::OnAgitEnd", "Agit#aldeg_cas01::OnAgitBreak", "alde_gld",48,83);
		Castle("aldeg_cas02", "Agit#aldeg_cas02::OnAgitEnd", "Agit#aldeg_cas02::OnAgitBreak", "alde_gld",95,249);
		Castle("aldeg_cas03", "Agit#aldeg_cas03::OnAgitEnd", "Agit#aldeg_cas03::OnAgitBreak", "alde_gld",142,85);
		Castle("aldeg_cas04", "Agit#aldeg_cas04::OnAgitEnd", "Agit#aldeg_cas04::OnAgitBreak", "alde_gld",239,242);
		Castle("aldeg_cas05", "Agit#aldeg_cas05::OnAgitEnd", "Agit#aldeg_cas05::OnAgitBreak", "alde_gld",264,90);

// -----------------------------------------------------------

	set .Size, getarraysize($TERRI_CONTROL$);

	for(.@i=0;.@i<.Size;.@i=.@i+5) {
		.@j++;
		setarray .Castles[.@j],getd("."+.Castles$[.@j]);
	}

	if (.AutoKick || .NoOwner) {
		for(set .@i,1; .@i<.total_castle; set .@i,.@i+1) {
			setmapflag .Castles$[.@i], mf_loadevent;
			setd "."+.Castles$[.@i], .@i;
		}
	}
	else {
		for(set .@i,1; .@i<.total_castle; set .@i,.@i+1)
			setd "."+.Castles$[.@i], 0;
	}
	if (!agitcheck()) sleep 500;
	set .Init,1;
	.current_region = 0;

OnMinute00: // Prime Time Start
	cleararray .castle_active[0],0,getarraysize(.castle_active); //Just to be on the safe side...
OnMinute20: // Prime Time End
	freeloop(1);
	if(agitcheck()) { // territory battles is running
		announce "Prime time for territories in "+.Regions$[.Region]+" is over!",bc_all|bc_woe;
		AgitEnd;
		for(.@j=1; .@j<.total_castle; .@j++) {
			if (.castle_region[.@j] == .Region) {
				maprespawnguildid .Castles$[.@j],0,2;
				.castle_active[.@j] = 0;
			}
		}
		ClearSchedule(.Region);
		.Region = -1;
	}
	if(!agitcheck() || .Init) { // territory battles not started
		set .Init,0;
		for(.@i=0;.@i<getarraysize(.Regions$);.@i++){
			if(gettime(DT_HOUR) == .PrimeTime[.@i]  && gettime(DT_MINUTE) < .BattleDuration) {
				announce "Prime time for territories in "+.Regions$[.@i]+" is on!",bc_all|bc_woe;
				.Region = .@i; // region id
				sleep 500;
				AgitStart;
				// doevent "Gld_Agit_Manager::OnAgitInit";
				break;
			}
		}
		if(.Region) {
			for(.@k=1;.@k<.total_castle;.@k++){
				// run through all castles in this region
				if(.castle_region[.@k] == .Region) {
					for(.@i=0;.@i<.Size;.@i=.@i+5) {
						if(atoi($TERRI_CONTROL$[.@i+3]) == getd("."+.Castles$[.@k]) || !getcastledata(.Castles$[.@k],CD_GUILD_ID))
							.castle_active[.@k] = 1;	
					}
				}
			}
		}
	}
	set .Init,0;
	end;

OnPCLoadMapEvent:
	if(compare(strcharinfo(3),"cas0") && !compare(strcharinfo(3),"gl_cas")) {
		.@index = inarray(.Castles$,strcharinfo(3));

		if(!getcharid(2) || (.AutoKick && .castle_active[.@index] == 0) || 
		  (.NoOwner && getcastledata(strcharinfo(3),CD_GUILD_ID)==0 && !agitcheck()) || 
		  (.castle_active[.@index] == 1 && !AttackingGuild(getcharid(2))) ) {
			// One should be allowed to enter a territory if they own it
			if(getcastledata(strcharinfo(3),CD_GUILD_ID) == getcharid(2) && getcharid(2) > 0 ) end;
			// One should be able to enter active, empty territories during Prime Time
			if(agitcheck() && .castle_active[.@index] == 1 && getcastledata(strcharinfo(3),CD_GUILD_ID) == 0) end;
			.@castle_name$ = getcastlename(strcharinfo(3));
			sleep2 500;
			message strcharinfo(0), .@castle_name$ + " is currently inactive. Leave now or you will be warped to your Save Point";
			sleep2 5000;
			if(compare(strcharinfo(3),"cas0") && !compare(strcharinfo(3),"gl_cas"))
				warp "SavePoint",0,0;
		}
	}
	end;

OnAgitStart:
	for(.@k=1;.@k<.total_castle;.@k++){
		if(.castle_region[.@k] == .Region) {
			set .@GID, GetCastleData(.Castles$[.@k],CD_GUILD_ID);
			if (.@GID == 0) {
				if (compare(.Castles$[.@k],"aldeg")) {
					// Normal Spawns
					monster .Castles$[.@k],0,0,"Evil Druid",1117,10;
					monster .Castles$[.@k],0,0,"Khalitzburg",1132,4;
					monster .Castles$[.@k],0,0,"Abysmal Knight",1219,2;
					monster .Castles$[.@k],0,0,"Executioner",1205,1;
					monster .Castles$[.@k],0,0,"Penomena",1216,10;
					monster .Castles$[.@k],0,0,"Alarm",1193,18;
					monster .Castles$[.@k],0,0,"Clock",1269,9;
					monster .Castles$[.@k],0,0,"Raydric Archer",1276,7;
					monster .Castles$[.@k],0,0,"Wanderer",1208,3;
					monster .Castles$[.@k],0,0,"Alice",1275,1;
					monster .Castles$[.@k],0,0,"Bloody Knight",1268,1;
					monster .Castles$[.@k],0,0,"Dark Lord",1272,1;
					// Set Emperium room spawn coordinates and spawn monsters.
					if (.Castles$[.@k] == "aldeg_cas01") { setarray .@emproom[0],216,23; }
					else if (.Castles$[.@k] == "aldeg_cas02") { setarray .@emproom[0],213,23; }
					else if (.Castles$[.@k] == "aldeg_cas03") { setarray .@emproom[0],205,31; }
					else if (.Castles$[.@k] == "aldeg_cas04") { setarray .@emproom[0],36,217; }
					else if (.Castles$[.@k] == "aldeg_cas05") { setarray .@emproom[0],27,101; }
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Dark Lord",1272,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Tower Keeper",1270,4;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Bloody Knight",1268,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Abysmal Knight",1219,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Raydric Archer",1276,5;
				}
				else if (compare(.Castles$[.@k],"gefg")) {
					// Normal Spawns
					monster .Castles$[.@k],0,0,"Evil Druid",1117,10;
					monster .Castles$[.@k],0,0,"Wind Ghost",1263,11;
					monster .Castles$[.@k],0,0,"Bathory",1102,10;
					monster .Castles$[.@k],0,0,"Jakk",1130,10;
					monster .Castles$[.@k],0,0,"Marduk",1140,20;
					monster .Castles$[.@k],0,0,"Raydric",1163,9;
					monster .Castles$[.@k],0,0,"Alice",1275,1;
					monster .Castles$[.@k],0,0,"Abysmal Knight",1219,1;
					monster .Castles$[.@k],0,0,"Moonlight Flower",1150,1;
					monster .Castles$[.@k],0,0,"Phreeoni",1159,1;
					// Set Emperium room spawn coordinates and spawn monsters.
					if (.Castles$[.@k] == "gefg_cas01") { setarray .@emproom[0],197,181; }
					else if (.Castles$[.@k] == "gefg_cas02") { setarray .@emproom[0],176,178; }
					else if (.Castles$[.@k] == "gefg_cas03") { setarray .@emproom[0],244,166; }
					else if (.Castles$[.@k] == "gefg_cas04") { setarray .@emproom[0],174,177; }
					else if (.Castles$[.@k] == "gefg_cas05") { setarray .@emproom[0],194,184; }
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Mysteltainn",1203,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Orc Hero",1087,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"High Orc",1213,10;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Orc Archer",1189,10;
				}
				else if (compare(.Castles$[.@k],"payg")) {
					// Normal Spawns
					monster .Castles$[.@k],0,0,"Greatest General",1277,9;
					monster .Castles$[.@k],0,0,"Wanderer",1208,10;
					monster .Castles$[.@k],0,0,"Mutant Dragonoid",1262,5;
					monster .Castles$[.@k],0,0,"Bathory",1102,5;
					monster .Castles$[.@k],0,0,"Moonlight Flower",1150,1;
					monster .Castles$[.@k],0,0,"Eddga",1115,1;
					monster .Castles$[.@k],0,0,"Horong",1129,11;
					monster .Castles$[.@k],0,0,"Raydric Archer",1276,5;
					monster .Castles$[.@k],0,0,"Kobold Archer",1282,4;
					monster .Castles$[.@k],0,0,"Gargoyle",1253,5;
					// Set Emperium room spawn coordinates and spawn monsters.
					if (.Castles$[.@k] == "payg_cas01") { setarray .@emproom[0],139,139; }
					else if (.Castles$[.@k] == "payg_cas02") { setarray .@emproom[0],38,25; }
					else if (.Castles$[.@k] == "payg_cas03") { setarray .@emproom[0],269,265; }
					else if (.Castles$[.@k] == "payg_cas04") { setarray .@emproom[0],270,28; }
					else if (.Castles$[.@k] == "payg_cas05") { setarray .@emproom[0],30,30; }
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Moonlight Flower",1150,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Eddga",1115,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Wanderer",1208,6;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Raydric Archer",1276,5;
				}
				else if (compare(.Castles$[.@k],"prtg")) {
					// Normal Spawns
					monster .Castles$[.@k],0,0,"Raydric",1163,1;
					monster .Castles$[.@k],0,0,"Khalitzburg",1132,10;
					monster .Castles$[.@k],0,0,"Abysmal Knight",1219,5;
					monster .Castles$[.@k],0,0,"Bloody Knight",1268,5;
					monster .Castles$[.@k],0,0,"Stormy Knight",1251,1;
					monster .Castles$[.@k],0,0,"Hatii",1252,1;
					monster .Castles$[.@k],0,0,"Raydric Archer",1276,5;
					monster .Castles$[.@k],0,0,"Gryphon",1259,2;
					monster .Castles$[.@k],0,0,"Chimera",1283,3;
					monster .Castles$[.@k],0,0,"Alice",1275,1;
					monster .Castles$[.@k],0,0,"Zealotus",1200,1;
					// Set Emperium room spawn coordinates and spawn monsters.
					if (.Castles$[.@k] == "prtg_cas01") { setarray .@emproom[0],197,197; }
					else if (.Castles$[.@k] == "prtg_cas02") { setarray .@emproom[0],157,174; }
					else if (.Castles$[.@k] == "prtg_cas03") { setarray .@emproom[0],16,220; }
					else if (.Castles$[.@k] == "prtg_cas04") { setarray .@emproom[0],291,14; }
					else if (.Castles$[.@k] == "prtg_cas05") { setarray .@emproom[0],266,266; }
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Guardian Knight",1268,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Guardian Master",1251,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Hatii",1252,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Guardian Knight",1219,1;
					monster .Castles$[.@k],.@emproom[0],.@emproom[1],"Raydric Archer",1276,5;
				}
			}
		}
	}
	end;

OnMenu:
	//debug
	// input(.@r);
	// ClearSchedule(.@r);

	mes "[Territory Information]";
	mes "Prime Time : ^AA0000"+Add_Zero(.PrimeTime[atoi(@terriData$[0])])+"^000000";
	mes "Territory : " + getcastlename(@terriData$[1]);

	if(getarraysize($TERRI_CONTROL$)==0)
		next;

	mes "Schedule for the next 7 days:";

	// if(gettime(DT_HOUR) >= .PrimeTime[atoi(@terriData$[0])-1] && gettime(DT_MINUTE)> .BattleDuration) {
	// 	// Prime Time of this region is over for today
	// 	// Lets skip today and start from tomorrow
	// 	@start = gettime(DT_DAYOFWEEK)+1;
	// 	if(.@start > 6) .@start -= 7;
	// } else {
		// Prime time today is still current (or in the future)
		// Lets include today's schedule to show
		.@start = gettime(DT_DAYOFWEEK);
	// }
	// We are doing this to show the schedure organized from
	// the earliest to the furtherest in the future scheduled attadck

	for(.@k=.@start;.@k<.@start+7;.@k++){
		if(.@k > 6) .@day = .@k - 7; else .@day = .@k;
		if(BuildSchedule(.@day)) {
			.@c++;
			mes " ";
			if(.@day == gettime(DT_DAYOFWEEK)) mes "^FF0000--- ATTACKING TODAY ---^000000";
			else mes "> ^0000FF"+Weekday2Str(.@day)+"^000000";

			for(set .@i,0; .@i<.Size; set .@i,.@i+5)
				if (.@day == atoi($TERRI_CONTROL$[.@i]) && atoi($TERRI_CONTROL$[.@i+3]) == getd("."+@terriData$[1]))
					mes "  ^EE0000" + getguildname(atoi($TERRI_CONTROL$[.@i+2])) + "^000000 is attacking";
			
			if(.@day == gettime(DT_DAYOFWEEK))
				mes "^FF0000----------------------------------^000000";
		}
	}
	if(!.@c) mes "^A0A0A0>No attacks announced<";
	next;
	if(select(" ~ Declare an Attack...: ~ ^777777Cancel^000000") == 2 )
		close;

	// DECLARE ATTACK
	// ------------------------------
	mes "[Declare Attack]";
	if(!is_guild_leader(getcharid(2))) {
		mes "Only Guild Masters can declare attacks to a territory.";
		close;
	}
	if(getcharid(2) == getcastledata(@terriData$[1],CD_GUILD_ID)) {
		mes "You cannot declare attack to your own territory.";
		close;
	}
	if(.Primetime[atoi(@terriData$[0])] == gettime(DT_DAYOFWEEK)) {
		mes "You cannot declare attack during Prime Time. Please come back later";
		close;
	}
	if(getcastledata(@terriData$[1],CD_GUILD_ID) == 0) {
		mes "You don't need to declare an attack to this territory because it has no owner at the moment. All you guys need to do is show up during the Prime Time.";
		close;
	}
	mes "Day for the attack : ";
	DaysMenu(); // returns @menu$
	set .@Day, select(@menu$) -1;
	set .@Day, Weekday(@m$[.@Day]);
	clear;
	mes "[Declare Attack]";

	// Check if the same guild has not booked an attack already on that day to the same territory (just to avoid bullshit complains)
	for(set .@i,0; .@i<.Size; set .@i,.@i+5) {
		if (.@Day == atoi($TERRI_CONTROL$[.@i]) && atoi($TERRI_CONTROL$[.@i+2]) == getcharid(2) && atoi($TERRI_CONTROL$[.@i+3]) == getd("."+@terriData$[1])) {
			mes "You already have an attack booked in for the selected day.";
			close;
		}
	}
	mes "Day for the attack : ^0000AA" + Weekday2Str(.@Day)+"^000000";
	mes "Prime time : ^AA0000" + Add_Zero(.Primetime[atoi(@terriData$[0])])+"^000000";
	next;
	mes "[Declare Attack]";
	mes "^AA0000Territory will be vulnerable for "+.BattleDuration+" minutes - starting at Prime Time - to be attacked by "+getguildname(getcharid(2))+" and any other guild who declares attack to it on " + Weekday2Str(.@Day) + ".^000000";
	next;
	mes "[Declare Attack]";
	@totalCost = .Cost2attack * ((100 + getcastledata(@terriData$[1],CD_CURRENT_ECONOMY))/100);
	mes "This will cost you " + F_InsertComma(@totalCost);
	mes "Do you want to continue?";
	next;
	if(select("Yes, please:No thank you") == 2)
		close;
	mes "[Declare Attack]";
	if(Zeny < @totalCost) {
		mes "You don't have enough Zeny.";
		close;
	}
	//$TERRI_CONTROL$[0] = Weekday (number);
	//$TERRI_CONTROL$[1] = Start time (prime time of the region);
	//$TERRI_CONTROL$[2] = Guild ID declaring attack;
	//$TERRI_CONTROL$[3] = Castle ID;
	//$TERRI_CONTROL$[4] = Region ID;
	set Zeny, Zeny - @totalCost;
	setarray $TERRI_CONTROL$[getarraysize($TERRI_CONTROL$)], .@Day, .Primetime[atoi(@terriData$[0])], getcharid(2), getd("."+@terriData$[1]),atoi(@terriData$[0]);
	set .Size, getarraysize($TERRI_CONTROL$);
	mes "All set. Good luck on "+Weekday2Str(.@Day)+"!";
	close;




// ****************************************
// FUNCITONS
// ****************************************

	function Weekday2Str {
		switch(getarg(0)){
			case MONDAY: return "Monday";
			case TUESDAY: return "Tuesday";
			case WEDNESDAY: return "Wednesday";
			case THURSDAY: return "Thursday";
			case FRIDAY: return "Friday";
			case SATURDAY: return "Saturday";
			case SUNDAY: return "Sunday";
		}
	}

	function Weekday {
		.@d$ = getarg(0);
		if(.@d$ == "Sunday") return 0;
		if(.@d$ == "Monday") return 1;
		if(.@d$ == "Tuesday") return 2;
		if(.@d$ == "Wednesday") return 3;
		if(.@d$ == "Thursday") return 4;
		if(.@d$ == "Friday") return 5;
		if(.@d$ == "Saturday") return 6;
	}

	function DaysMenu {
		.@menu$ = "";
		deletearray @m$[0],getarraysize(@m$);
		.@start = gettime(DT_DAYOFWEEK) + 2;
		if(.@start > 6) .@start -= 7;
		for(.@i=.@start;.@i<.@start+5;.@i++){
			if(.@i > 6) .@day = .@i - 7; else .@day = .@i; 
			setarray @m$[getarraysize(@m$)],Weekday2Str(.@day);
		}
		@menu$ = " ~ "+implode(@m$,": ~ ");
		return;
	}

	function CheckBattleEnd {
		.@region = atoi(@terriData$[0]);
		.@primetime = .PrimeTime[.@region-1];
		if(gettime(DT_HOUR) == .@primetime && gettime(DT_MINUTE >= .BattleDuration))
			return true;
		else
			return false;
	}

	function CastleMapName2CastleId {
		.@map$ = getarg(0,@terriData$[1])
		.@index = inarray(.Castles$,.@map$);
		return .Castles[.@index];
	}

	function BuildSchedule {
		.@day = getarg(0);
		for(.@i=0; .@i<.Size; set .@i,.@i+5) {
			// debugmes ".@day : " + .@day + " $TERRI_CONTROL$[.@i] : " + $TERRI_CONTROL$[.@i] + "  getd(.@terriData$[1]) : " + getd("."+@terriData$[1]); 
			if (.@day == atoi($TERRI_CONTROL$[.@i]) && atoi($TERRI_CONTROL$[(.@i+3)]) == getd("."+@terriData$[1])){
				return true;
			}
		}
		return false;
	}

	function AttackingGuild {
		.@guild = getarg(0,getcharid(2));
		.@map$ = getarg(1,strcharinfo(3));
		for(set .@i,0; .@i<.Size; set .@i,.@i+5) {
			if (.@guild == atoi($TERRI_CONTROL$[.@i+2]) && atoi($TERRI_CONTROL$[.@i+3]) == CastleMapName2CastleId(.@map$));
				return true;
		}
		return false;
	}

	function ClearSchedule {
		.@r = getarg(0); //Region Id
		for(.@i=4; .@i<.Size; set .@i,.@i+5)
			if(atoi($TERRI_CONTROL$[.@i]) == .@r && atoi($TERRI_CONTROL$[.@i-4]) == atoi($TERRI_CONTROL$[.@i+3]))
				deletearray $TERRI_CONTROL$[.@i-4],5;
	
		set .Size, getarraysize($TERRI_CONTROL$);
		
		// debugmes "merda : " + getd(".castle_index_in_region_3["+.@i+"");
		// for(.@i=0; .@i<.Size; set .@i,.@i+4) {
		// 	if( $TERRI_CONTROL$[.@i] == gettime(DT_DAYOFWEEK)) {
		// 		for(.@j=1;.@j<.total_castle;.@j++){
		// 			if(.castle_region[.@j] == .@r){
		// 				debugmes " .Castles["+.@j+"] : " + .Castles[.@j] + " $TERRI_CONTROL$["+.@i+3+"] : " + $TERRI_CONTROL$[.@i+3];
		// 				if(.Castles[.@j] == $TERRI_CONTROL$[.@i+3]) {
		// 					deletearray $TERRI_CONTROL$[.@i],4;
		// 					set .Size, getarraysize($TERRI_CONTROL$);
		// 				}
		// 			}
		// 		}
		// 	}
		// }
		return;
	}

	function Add_Zero {
		return ((getarg(0)<10)?"0":"")+getarg(0)+(getarg(1,0)?".":":")+"00";
	}

	function Castle {
		.@map$ = getarg(0);
		if (getd( "." + .@map$ ) > 0) {
			debugmes sprintf("TERRI_CONTROL$: map %s already defined.", .@map$);
			return;
		}
		if (.total_castle > 61) {
			debugmes sprintf("TERRI_CONTROL$: Maximum of 62 castles defined, %s skipped.", .@map$);
			return;
		}
		.Castles$[.total_castle] = .@map$;
		.event_name_agitend$[.total_castle] = getarg(1);
		.event_name_killmob$[.total_castle] = getarg(2);
		.Map$[.total_castle] = getarg(3);
		.MapX[.total_castle] = getarg(4);
		.MapY[.total_castle] = getarg(5);
		.castle_region[.total_castle] = .current_region;
		// .castle_id[.total_castle] = getcastleid(.@map$);

		setd ".castle_index_in_region_" + .current_region + "[" + .size_region[.current_region] + "]", .total_castle;
		.total_castle++;
		.size_region[.current_region]++;

		setd "." + .Castles$[.total_castle-1], .total_castle; //Castle Ids
		return;
	}


	function Region {
		.@zone$ = getarg(0);
		for ( .@i = 0; .@i < .total_region; ++.@i ) {
			if (.Regions$[.@i] == .@zone$) {
				.current_region = .@index;
				return;
			}
		}
		.Regions$[.total_region] = .@zone$;
		.current_region = .total_region;
		.total_region++;
		return;
	}

}

// ****************************************
// DUPLICATES
// ****************************************

prt_gld,135,73,6	duplicate(#terriInfo)	Information#0-prtg_cas01	105
prt_gld,231,125,0	duplicate(#terriInfo)	Information#0-prtg_cas02	105
prt_gld,164,141,4	duplicate(#terriInfo)	Information#0-prtg_cas03	105
prt_gld,124,255,4	duplicate(#terriInfo)	Information#0-prtg_cas04	105
prt_gld,195,255,4	duplicate(#terriInfo)	Information#0-prtg_cas05	105

pay_gld,113,233,4	duplicate(#terriInfo)	Information#1-payg_cas01	780
pay_gld,290,105,6	duplicate(#terriInfo)	Information#1-payg_cas02	780
pay_gld,323,285,2	duplicate(#terriInfo)	Information#1-payg_cas03	780
pay_gld,149,163,1	duplicate(#terriInfo)	Information#1-payg_cas04	780
pay_gld,196,279,4	duplicate(#terriInfo)	Information#1-payg_cas05	780

gef_fild13,158,59,6	duplicate(#terriInfo)	Information#2-gefg_cas01	708
gef_fild13,316,244,4	duplicate(#terriInfo)	Information#2-gefg_cas02	708
gef_fild13,131,279,0	duplicate(#terriInfo)	Information#2-gefg_cas03	708
gef_fild13,203,270,3	duplicate(#terriInfo)	Information#2-gefg_cas04	708
gef_fild13,318,77,1	duplicate(#terriInfo)	Information#2-gefg_cas05	708

alde_gld,67,88,0	duplicate(#terriInfo)	Information#3-aldeg_cas01	707
alde_gld,93,242,6	duplicate(#terriInfo)	Information#3-aldeg_cas02	707
alde_gld,144,94,3	duplicate(#terriInfo)	Information#3-aldeg_cas03	707
alde_gld,240,262,3	duplicate(#terriInfo)	Information#3-aldeg_cas04	707
alde_gld,264,97,6	duplicate(#terriInfo)	Information#3-aldeg_cas05	707