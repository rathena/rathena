
geffen,100,72,4	script	Geferus	735,{

	if(getgmlevel() == 99) {
		mes "DEBUG";
		if(select("resetar quests:continuar") == 1 ) {
			erasequest(64181);
			erasequest(64182);
			erasequest(64183);
			erasequest(64184);
			erasequest(64185);
		}
		next;
	}

	//if (BaseLevel < 120 || BaseLevel > 180) {
	if (BaseLevel < 120) {

		setarray .@names$[0],"Wraith Dead","Mini Deamon","Zombie Master","Ghoul","Deviruchi";
		mes "[Geferus]";
		mes "You must be at least level 120 to accept these missions.";
		for(set .@quest,64181; .@quest<=64185; set .@quest,.@quest+1) {
			set .@hunting, checkquest(.@quest,HUNTING);
			if (.@hunting == 0 || .@hunting == 1) {
				next;
				mes ":: You cannot proceed in";
				mes ":: ^0000FF"+.@names$[.@quest-64181]+" Hunting^000000.";
				mes ":: The registration to the mission";
				mes ":: is cancelled.";
				erasequest .@quest;
				close;
			} else if (.@hunting == 2) {
				next;
				mes ":: You added information";
				mes ":: about the mission";
				mes ":: ^0000FF"+.@names$[.@quest-64181]+" Hunting^000000";
				mes ":: on the mission board.";
				erasequest .@quest;
				close;
			}
		}
		close;
	}
	mes "[Geferus]";
	mes "There are several requests on my list.";
	if(BaseLevel > 220) {
		mes "I am afraid you won't get any EXP from me at the end of these quests because you're already over level 220.";
	}
	next;
	switch(select("^8B4513Geffen Guild Dungeons^000000")) {
	case 1:
		mes "^8B4513Geffen Guild Dungeons^000000.";
		mes "Which monster will you hunt or have you finished hunting?";
		next;
		switch(select("^0000FFWraith Dead^000000:^0000FFMini Deamon^000000:^0000FFZombie Master^000000:^0000FFGhoul^000000:^0000FFDeviruchi^000000")) {
			case 1: callsub L_Quest,64181,"Wraith Dead",1551150,"레이스데드카드","Find it in ^8B4513Geffen Guild Dungeons^000000.",542700;
			case 2: callsub L_Quest,64182,"Mini Deamon",1259400,"미니데몬카드","Find it in ^8B4513Geffen Guild Dungeons^000000.",558300;
			case 3: callsub L_Quest,64183,"Zombie Master",1141500,"좀비마스터카드","Find it in ^8B4513Geffen Guild Dungeons^000000.",423900;
			case 4: callsub L_Quest,64184,"Ghoul",326400,"구울카드","Find it in ^8B4513Geffen Guild Dungeons^000000.",186600;
			case 5: callsub L_Quest,64185,"Deviruchi",798600,"데비루치카드","Find it in ^8B4513Geffen Guild Dungeons^000000.",383700;
		}
	}
	end;

L_Quest:
	set .@quest1, getarg(0);
	set .@quest2, .@quest1+100;
	.@cutin$ = getarg(3);
	set .@playtime, checkquest(.@quest2,PLAYTIME);
	if (.@playtime == 0 || .@playtime == 1) {
		mes "[Geferus]";
		mes "You need to wait 2 hours before you can take this mission again.";
		mes "Or you can reset that time using a ^0000FF" + getitemname(6320) + "^000000.";
		next;
		if(select("Please reset:Not now") == 2){
			close;
		} else {
			if(countitem(6320) == 0) {
				mes "[Geferus]";
				mes "I am sorry but I could not find your ^0000FF" + getitemname(6320) + "^000000.";
				close;
			} else {
				mes "[Geferus]";
				mes "Prontinho!";
				delitem 6320,1;
				erasequest .@quest2;
				close;
			}
		}
		close;
	}
	set .@hunting, checkquest(.@quest1,HUNTING);
	if (.@hunting == 0 || .@hunting == 1) {
		mes "[Geferus]";
		mes "Have you finished hunting all the "+getarg(1)+" yet?";
		mes "Do You want to enter the Dungeons?";
		if(select("Yes, please:Not yet") == 2) {
			close;
		} else {
			warp "gld_dun04",0,0;
			end;
		}
	} else if (.@hunting == 2) {
		mes "[Geferus]";
		mes "You have completed the hunting.";
		mes "Please accept this reward as a compensation.";
		erasequest .@quest1;
		if (.@playtime > -1) erasequest .@quest2;
		setquest .@quest2;

		set .@bexp, getarg(2) * 75;
		set .@jexp, getarg(5) * 75;

		 if(BaseLevel <= 220) 
			getexp .@bexp,.@jexp;

		getitem 675,6;
		close;
	} else {
		mes "[Geferus]";
		mes getarg(4);
		next;
		if (.@cutin$ != "") cutin .@cutin$,3;
		mes "[Geferus]";
		mes "Hunt the "+getarg(1)+" and post your success on this board.";
		mes "It will cost you 100.000z";
		next;
		if (.@cutin$ != "") cutin .@cutin$,255;
		if(select("I'll hunt them.:No thanks.") == 1) {
			mes "[Geferus]";
			if(Zeny < 100000) {
				mes "I am sorry, you don't have enough zenys.";
				close;
			} else {
				mes "When you have completed the mission, post it on this board and collect your reward.";
				if (.@playtime > -1) erasequest .@quest2;
				setquest .@quest1;	
				set Zeny, Zeny - 100000;
			}
		}
		close;
	}
}

-	script	ExtraDrops	-1,{
	end;
	
	OnNPCKillEvent:
	if (killedrid == 1291 || killedrid == 1292 || killedrid == 1298) {
		getmapxy(.@map$,.@x,.@y,BL_PC);
		if(.@map$ == "gld_dun04") {
			if(rand(0,1000) > 980)
				makeitem 675,1,.@map$,.@x,.@y;
		}
	}
}
