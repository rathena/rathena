
// Information NPC
//============================================================
-	script	#terriInfo	-1,{
	//@terriData$[0] = Region ID
	//@terriData$[1] = castle map
	explode(@terriData$,strnpcinfo(2),"-");

	doevent "TERRI_CONTROL::OnMenu";
	end;
}

// Script Core
//============================================================
-	script	TERRI_CONTROL	-1,{
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
	// deletearray $TERRI_CONTROL[0],getarraysize($TERRI_CONTROL);

// -----------------------------------------------------------
//  Configuration settings.
// -----------------------------------------------------------

	set .AutoKick,1;		// Automatically kick players from inactive castles during WOE? (1:yes / 0:no)
	set .NoOwner,1; 		// Automatically kick players from unconquered castles outside of WOE? (1:yes / 0:no)

	set .Cost2attack, 8000000;
	set .BattleDuration, 20;
	setarray .PrimeTime[0],16,12,20,1; //Prontera,Payon,Geffen,Al de Baran


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

	set .Size, getarraysize($TERRI_CONTROL);

	for(.@i=0;.@i<.Size;.@i=.@i+4) {
		setarray .Castles[.@j],getd("."+.Castles$[.@j]);
		.@j++;
	}

	if (.AutoKick || .NoOwner) {
		for(set .@i,0; .@i<.total_castle; set .@i,.@i+1) {
			setmapflag .Castles$[.@i], mf_loadevent;
			setd "."+.Castles$[.@i], .@i;
		}
	}
	else {
		for(set .@i,0; .@i<.total_castle; set .@i,.@i+1)
			setd "."+.Castles$[.@i], 0;
	}
	if (!agitcheck()) sleep 500;
	set .Init,1;
	.current_region = 0;

OnMinute00: // Prime Time Start
	cleararray .castle_active[0],0,getarraysize(.castle_active); //Just to be on the safe side...
OnMinute20: // Prime Time End
	freeloop(1);
	if(agitcheck()) { // territory battles started
		announce "Prime time for territories in "+.Regions$[.Region]+" is over!",bc_all|bc_woe;
		AgitEnd;
		for(.@j=0; .@j<.total_castle; .@j++) {
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
				break;
			}
		}
		if(.Region) {
			for(.@k=0;.@k<.total_castle;.@k++){
				// run through all castles in this region
				if(.castle_region[.@k] == .Region) {
					for(.@i=0;.@i<.Size;.@i=.@i+4) {
						if($TERRI_CONTROL[.@i+3] == getd("."+.Castles$[.@k]) || !getcastledata(.Castles$[.@k],CD_GUILD_ID))
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

		if ( (.AutoKick && .castle_active[.@index] == 0) || (.NoOwner && !getcastledata(strcharinfo(3),CD_GUILD_ID) && !agitcheck()) || (.castle_active[.@index] == 1 && !AttackingGuild(getcharid(2))) ) {
			// if (getcharid(2) && getcastledata(strcharinfo(3),1) == getcharid(2)) end;
			.@castle_name$ = getcastlename(strcharinfo(3));
			sleep2 500;
			message strcharinfo(0), .@castle_name$ + " is currently inactive. Leave now or you will be teleported to your Save Point";
			sleep2 5000;
			if(compare(strcharinfo(3),"cas0") && !compare(strcharinfo(3),"gl_cas"))
				warp "SavePoint",0,0;
		}
	}
	end;

OnMenu:
	mes "[Territory Information]";
	mes "Prime Time : ^AA0000"+Add_Zero(.PrimeTime[atoi(@terriData$[0])])+"^000000";
	mes "Territory : " + getcastlename(@terriData$[1]);

	if(getarraysize($TERRI_CONTROL)==0)
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

			for(set .@i,0; .@i<.Size; set .@i,.@i+4)
				if (.@day == $TERRI_CONTROL[.@i] && $TERRI_CONTROL[.@i+3] == getd("."+@terriData$[1]))
					mes "  ^EE0000" + getguildname($TERRI_CONTROL[.@i+2]) + "^000000 is attacking";
			
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
	mes "Day for the attack : ";
	DaysMenu(); // returns @menu$
	set .@Day, select(@menu$) -1;
	set .@Day, Weekday(@m$[.@Day]);
	clear;
	mes "[Declare Attack]";

	// Check if the same guild has not booked an attack already on that day to the same territory (just to avoid bullshit complains)
	for(set .@i,0; .@i<.Size; set .@i,.@i+4) {
		if (.@Day == $TERRI_CONTROL[.@i] && $TERRI_CONTROL[.@i+2] == getcharid(2) && $TERRI_CONTROL[.@i+3] == getd("."+@terriData$[1])) {
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
	@totalCost = .Cost2attack * (getcastledata(@terriData$[1],CD_CURRENT_ECONOMY)+1);
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
	//$TERRI_CONTROL[0] = Weekday (number);
	//$TERRI_CONTROL[1] = Start time (prime time of the region);
	//$TERRI_CONTROL[2] = Guild ID declaring attack;
	//$TERRI_CONTROL[3] = Castle ID;
	set Zeny, Zeny - @totalCost;
	setarray $TERRI_CONTROL[getarraysize($TERRI_CONTROL)], .@Day, .Primetime[atoi(@terriData$[0])], getcharid(2), getd("."+@terriData$[1]);
	set .Size, getarraysize($TERRI_CONTROL);
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
		for(set .@i,0; .@i<.Size; set .@i,.@i+4) {
			if (.@day == $TERRI_CONTROL[.@i] && $TERRI_CONTROL[(.@i+3)] == getd("."+@terriData$[1])){
				return true;
			}
		}
		return false;
	}

	function AttackingGuild {
		.@guild = getarg(0,getcharid(2));
		.@map$ = getarg(1,strcharinfo(3));
		for(set .@i,0; .@i<.Size; set .@i,.@i+4) {
			if (.@guild == $TERRI_CONTROL[.@i+2] && $TERRI_CONTROL[.@i+3] == CastleMapName2CastleId(.@map$));
				return true;
		}
		return false;
	}

	function ClearSchedule {
		.@r = getarg(0); //Region Id
		for(.@i=0; .@i<.Size; set .@i,.@i+4) {
			if( $TERRI_CONTROL[.@i] == gettime(DT_DAYOFWEEK) && gettime(DT_MINUTE) >= .BattleDuration ) {
				for(.@j=0;.@j<.total_castle;.@j++){
					if(.castle_region[.@j] == .@r && .Castles[.@j] == $TERRI_CONTROL[.@i+3]) {
						deletearray $TERRI_CONTROL[.@i],4;
						.Size--;
					}
				}
			}
		}
		return;
	}

	function Add_Zero {
		return ((getarg(0)<10)?"0":"")+getarg(0)+(getarg(1,0)?".":":")+"00";
	}

	function Castle {
		.@map$ = getarg(0);
		if (getd( "." + .@map$ ) > 0) {
			debugmes sprintf("TERRI_CONTROL: map %s already defined.", .@map$);
			return;
		}
		if (.total_castle > 61) {
			debugmes sprintf("TERRI_CONTROL: Maximum of 62 castles defined, %s skipped.", .@map$);
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