-	script	adventurer_quest	-1,{
	function CheckRewards;
	function Update;
	function ClearUnfinished;
	function ShowCutin;
	function AnnounceNewChallenge;
	end;

OnInit:
	// Total of Points necessary to get the daily reward:
	.DailyReward = 100;
	.WeeklyReward = 1500;
	.MonthlyReward = 8000;
	
	//Reward items
	setarray .DailyRewardItem[0],12209,14209,12900,12209,14209,521,522;
	setarray .WeeklyRewardItem[0],516,517,518,519,520;
	setarray .MonthlyRewardItem[0],516,517,518,519,520;

	//Control Variables
	setarray .Points$[0],	"#AQ_DailyPoints",	"#AQ_WeeklyPoints",	"#AQ_MonthlyPoints";
	setarray .Limit$[0],	".DailyReward",		".WeeklyReward",	".MonthlyReward";
	setarray .Chkpoint$[0],	"#AQ_Yesterday",	"#AQ_LastWeek",		"#AQ_LastMonth";
	setarray .Current$[0],	"#AQ_CurDay",		"#AQ_CurWeek",		"#AQ_CurMonth";
	setarray .Period$[0],	".Day",				".Week",			".Month";

	//Mail Constants
	.sender$ = "Adventurer's Challenge";
	.zeny = 0;	// Default zeny sent

OnClock0000:
	if(gettime(DT_DAYOFYEAR) < .Day) .Day = 1;
	else .Day = gettime(DT_DAYOFYEAR);

	if(gettime(DT_DAYOFWEEK) == MONDAY && (gettime(DT_DAYOFYEAR)/7)+1 < .Week) .Week = 1;
	else .Week = (gettime(DT_DAYOFYEAR)/7)+1;

	if(gettime(DT_DAYOFMONTH) == 1 || !.Month) .Month = gettime(DT_MONTH);

	addrid(0,0);
	if(ClearUnfinished()==1){
		AnnounceNewChallenge();
		ShowCutin();
	}
	end;

OnClock2300:
	announce "Adventurer Tom : Hey folks! Just a reminder that in one hour's time clock will reset the Adventurer's Challenge!",bc_all|bc_blue;
	end;
OnClock2350:
	announce "Adventurer Tom : Another kind reminder that in 10 minutes clock will reset the Adventurer's Challenge!",bc_all|bc_blue;
	end;

//DEBUG
// OnPCLogoutEvent:
	// #AQ_DailyPoints = 0;
	// #AQ_WeeklyPoints = 0;
	// #AQ_MonthlyPoints = 0;
	// #AQ_Yesterday = 0;
	// #AQ_LastWeek = 0;
	// #AQ_LastMonth = 0;
	// #AQ_CurDay = 0;
	// #AQ_CurWeek = 0;
	// #AQ_CurMonth = 0;
	// end;

OnPcLoginEvent:
	sleep2 500;
	if(ClearUnfinished()==1) {
		AnnounceNewChallenge();
		ShowCutin();
	}
	end;

// OnPCBaseExpGain:
OnNPCKillEvent:
	switch(callfunc("abs",((BaseLevel>80?80:BaseLevel) - getmonsterinfo(killedrid,MOB_LV)))) {
		case 0: case 1: case 2: case 3: case 4: case 5:
			Update(5);
			break;
		case 6: case 7: case 8: case 9: case 10:
			Update(2);
			break;
		default:
			Update(1);
			break;
	}

	// dispbottom "Adventurer Quest Daily Points: " + #AQ_DailyPoints + "/" + .DailyReward;
	// dispbottom "Adventurer Quest Weekly Points: " + #AQ_WeeklyPoints + "/" + .WeeklyReward;
	// dispbottom "Adventurer Quest Monthly Points: " + #AQ_MonthlyPoints + "/" + .MonthlyReward;

	//Check for daily rewards...
	if(CheckRewards()==1)
		ShowCutin();
	end;


	// This runs at mid-night for all those online
	// or OnPcLoginEvent
	// Current(day/week/month) is recorded first time player login and current is different from the actual period(d/w/m)
	// whrn current is different from period, system will clean the points for the period
	function ClearUnfinished {
		for(.@i=0;.@i<3;.@i++) {
			if(getd(.Current$[.@i]) != getd(.Period$[.@i])) {
				set getd(.Points$[.@i]),0;
				set getd(.Current$[.@i]),getd(.Period$[.@i]);
				.@x=1;
			}
		}
		return .@x;
	}


	function AnnounceNewChallenge {
		dispbottom "Adventurer Tom : Howdy, fellow adventurer!",0xaed6f1;
		dispbottom "Adventurer Tom : A new challenge has started",0xaed6f1;
		dispbottom "Adventurer Tom : Check your " +getitemname(24501)+" to find out more.",0xaed6f1;
		dispbottom "Adventurer Tom : Good luck and take care out there, always.",0xaed6f1;
		return;
	}


	function Update {
		.@amount = getarg(0);
		for(.@i=0;.@i<3;.@i++){
			//Met the challenge, skip. 
			if(getd(.Points$[.@i]) == getd(.Limit$[.@i]))
				continue;
			if(getd(.Chkpoint$[.@i]) == getd(.Period$[.@i]))
				continue;
			//Update the points accordingly and limit them to their max amount.
			if(getd(.Points$[.@i]) + .@amount > getd(.Limit$[.@i]))
				set getd(.Points$[.@i]), getd(.Limit$[.@i]);
			else
				set getd(.Points$[.@i]), getd(.Points$[.@i]) + .@amount;
		}
		return;
	}


	function CheckRewards {
		for(.@i=0;.@i<3;.@i++){
			if(getd(.Points$[.@i]) >= getd(.Limit$[.@i]) && (getd(.Chkpoint$[.@i]) != getd(.Period$[.@i]))) {
				dispbottom "Adventurer Tom : Congratulations!",0xaed6f1;
				if(.Period$[.@i] == ".Day") {
					dispbottom "Adventurer Tom : You've just completed your Daily Adventurer's Challenge!",0xaed6f1;
					dispbottom "Unlocked:",0xf1d6a6;
					if(vip_status(1)) dispbottom " - Free repairs and items identification at Sweetie;",0xf1d6a6;
					dispbottom " - Access to new towns and dungeons via teleport",0xf1d6a6;
					.@title$ = "Daily Reward";
					.@body$ = "Brave, fellow adventurer! I see you've met today's Adventurer's Challenge! Here is something for you, hope you like it and see you tomorrow!";
					setarray .@mailitem[0],.DailyRewardItem[gettime(DT_DAYOFWEEK)];
				} else if(.Period$[.@i] == ".Week") {
					dispbottom "Adventurer Tom : You've just completed your Weekly Adventurer's Challenge!",0xaed6f1;
					dispbottom "Unlocked:",0xf1d6a6;
					dispbottom " - Access to new towns and dungeons via teleport",0xf1d6a6;
					.@title$ = "Weekly Reward";
					.@body$ = "Brave, fellow adventurer! I see you've met this week's Adventurer's Challenge! Here is something for you, hope you like it and see you next week!";
					setarray .@mailitem[0],.WeeklyRewardItem[rand(getarraysize(.WeeklyRewardItem))];
				} else if(.Period$[.@i]== ".Month") {
					dispbottom "Adventurer Tom : You've just completed your Monthly Adventurer's Challenge!",0xaed6f1;
					.@title$ = "Monthly Reward";
					.@body$ = "Brave, fellow adventurer! I see you've met this month's Adventurer's Challenge! Here is something for you, hope you like it and see you next month!";
					setarray .@mailitem[0],.MonthlyRewardItem[rand(getarraysize(.MonthlyRewardItem))];
				}
				dispbottom "Adventurer Tom : Also, check your Rodex for the prize.",0xaed6f1;
				setarray .@mailqt[0],1; 
				mail getcharid(0), .sender$, .@title$, .@body$, .zeny, .@mailitem, .@mailqt;
				set getd(.Chkpoint$[.@i]), getd(.Period$[.@i]);
				set getd(.Points$[.@i]),0;
				specialeffect 709,AREA;
				.@ret=1;
			}
		}

		return .@ret;
	}

	function ShowCutin {
		cutin "ma_tomas03",2;
		sleep2 800;
		cutin "ma_tomas04",2;
		soundeffect "wild_rose_die.wav",0;
		sleep2 1000;
		cutin "ma_tomas03",2;
		sleep2 800;
		cutin "",255;
		sleep2 1000;
		return;
	}
}

function	script	F_DailyBar	{
	.@bars		= 15;
	.@limit 	= getvariableofnpc(.DailyReward,"adventurer_quest");
	.@current	= #AQ_DailyPoints;
	.@blue 		= (.@current * .@bars) / .@limit;
	if(#AQ_Yesterday == gettime(DT_DAYOFYEAR)) 
		return "^0000FF[complete]^000000";
	freeloop(1);
	for(.@i=0;.@i<.@bars;.@i++) {
		if(.@i <= .@blue)
			.@m$ += "^0000FF|^000000";
		else
			.@m$ += "^FF0000|^000000";
	}
	freeloop(0);
	return .@m$;
}

function	script	F_WeeklyBar	{
	.@bars		= 15;
	.@limit 	= getvariableofnpc(.WeeklyReward,"adventurer_quest");
	.@current	= #AQ_WeeklyPoints;
	.@blue 		= (.@current * .@bars) / .@limit;
	if(#AQ_LastWeek == (gettime(DT_DAYOFYEAR)/7)+1) 
		return "^0000FF[complete]^000000";
	freeloop(1);
	for(.@i=0;.@i<.@bars;.@i++) {
		if(.@i <= .@blue)
			.@m$ += "^0000FF|^000000";
		else
			.@m$ += "^FF0000|^000000";
	}
	freeloop(0);
	return .@m$;
}

function	script	F_MonthlyBar	{
	.@bars		= 15;
	.@limit 	= getvariableofnpc(.MonthlyReward,"adventurer_quest");
	.@current	= #AQ_MonthlyPoints;
	.@blue 		= (.@current * .@bars) / .@limit;
	if(#AQ_LastMonth == gettime(DT_MONTH)) 
		return "^0000FF[complete]^000000";
	freeloop(1);
	for(.@i=0;.@i<.@bars;.@i++) {
		if(.@i <= .@blue)
			.@m$ += "^0000FF|^000000";
		else
			.@m$ += "^FF0000|^000000";
	}
	freeloop(0);
	return .@m$;
}