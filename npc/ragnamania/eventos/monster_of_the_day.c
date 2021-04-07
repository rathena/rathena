prontera,125,210,5	script	Connor::motdBoard	632,{
	mes "^0000FF[ Connor ]^000000";
	mes "Hello, traveler!";
	mes "Do you want to help me out?";
	next;
	mes "^0000FF[ Connor ]^000000";
	mes "Today I've been requested to kill";
	mes "some ^FF0000"+getmonsterinfo($@motd_mobid,MOB_NAME)+"^000000.";
	mes "I've set a bounty on their head:";
	next;
	mes "^0000FF[ Connor ]^000000";
	mes "Exp Bonus : ^EE00AA" + callfunc("F_InsertComma",$@motd_XP)+"^000000.";
	mes "A chance to get ^DD0088"+getitemname(675)+"s^000000.";
	mes "A chance to get ^CC0066"+getitemname(30022)+"s^000000.";
	mes "A chance to get ^BB0044"+getitemname(30027)+"es^000000.";
	next;
	mes "^0000FF[ Connor ]^000000";
	mes "So, what are you waiting for?";
	close2;
	atcommand "#whereis \""+ strcharinfo(0)+"\" "+$@motd_mobid;
	end;

OnInit:
	setarray .mobs[0],1207,1310,1213,1206,1102,1197,1196,1278,1379,1713,1753,1777,1505,1317,1193,1408,1679,1870;
	//$@motd_mobid 	= getelementofarray(.mobs,rand(getarraysize(.mobs)));
	//$@motd_XP		= rand(1000000,15000000);
	callfunc "ChangeMob","motdMob";
	end;


// OnNPCKillEvent:
// 	if(killedrid == $@motd_mobid) {
// 		getexp2($@motd_XP,$@motd_XP);
// 		//BaseExp += getmonsterinfo(killedrid,MOB_BASEEXP) * rand(3);
// 		//JobExp += getmonsterinfo(killedrid,MOB_JOBEXP) * rand(3);
// 	}
// 		// if(BaseLevel > 159 && getmonsterinfo(killedrid,MOB_BASEEXP) < NextBaseExp/200) {
// 		// 	BaseExp = BaseExp + (NextBaseExp/200);
// 		// 	JobExp = JobExp + (NextJobExp/200);
// 		// }
// 	end;

OnBite:
	npctalk "Wow! Careful there! Don't get that close!";
	end;
}



prontera,123,209,5	script	MotD::motdMob	1002,{
	percentheal(-10,0);
	soundeffect "matyr_attack.wav",0;
	donpcevent "motdBoard"+"::OnBite";
	end;

OnClock0001:
	//$@motd_mobid = getelementofarray(getvariableofnpc(.mobs,"motdBoard"),rand(getarraysize(getvariableofnpc(.mobs,"motdBoard"))));
	callfunc "ChangeMob";
	end;

OnTimer10100:
	misceffect 231;
	stopnpctimer;
	initnpctimer;
	end;

OnInit:
	initnpctimer;
	misceffect 231;
	end;
}



function	script	ChangeMob	{
	atcommand "@reloadmobdb";
	.@npc$ = getarg(0,strnpcinfo(3));
	$@motd_mobid = getelementofarray(getvariableofnpc(.mobs,"motdBoard"),rand(getarraysize(getvariableofnpc(.mobs,"motdBoard"))));
	$@motd_XP = rand(1000000,15000000);
	setnpcdisplay(.@npc$,getmonsterinfo($@motd_mobid,MOB_NAME), $@motd_mobid);
	addmonsterdrop $@motd_mobid,30027,200;	// Ragnamania Loot Boxes
	addmonsterdrop $@motd_mobid,30023,200;	// Ragnamania Bag of Manias
	addmonsterdrop $@motd_mobid,675,200;	// Ragnamania Hunting Coins

	return;
}