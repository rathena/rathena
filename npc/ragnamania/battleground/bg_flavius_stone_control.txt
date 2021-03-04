//===== rAthena Script =======================================
//= Battleground Extended + : Flavius Stone Control
//===== By: ==================================================
//= eAmod / Grenat
//===== Description: =========================================
//= Battleground Extended + scripts.
//===== Additional Comments: =================================
//= Adaptation for rAthena 2020.
//= This is a free release.
//= Please do NOT take ownership.
//= The original author will always stay eAmod,
//= It would be a disgrace claiming it's yours and making
//= money out of it. [Grenat]
//============================================================

// ===========================================================
// Configuration
// ===========================================================
-	script	Flavius_SC	-1,{
	end;

OnGuillaumeQuit:
OnCroixQuit:
	bg_desert;
OnGuillaumeDie:
OnCroixDie:
	if ($@BG12 == 1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Stone Carrier
		set .Stone[.@Stone],0;
		getmapxy .@m$, .@x, .@y, BL_PC;
		movenpc "Neutral Stone#" + .@Stone, .@x, .@y;
		mapannounce "bat_b04","Neutral Stone Dropped by [ " + strcharinfo(0) + " ]",1,0xFFFFFF;
		initnpctimer "Neutral Stone#" + .@Stone;
		deltimer "Flavius_SC::OnFlash";
		enablenpc "Neutral Stone#" + .@Stone;
	}
	end;

OnGuillaumeActive:
	warp "bat_b04",328,150;
	end;
OnCroixActive:
	warp "bat_b04",62,150;
	end;

OnReady:
	set $@BG12,1;

	initnpctimer;
	set $@score_stonecontrol_blue, 0;
	set $@score_capture_red, 0;
	setarray .Stone[1],0,0,0,0,0,0;
	setarray .x[1],177,222,222,177,200,199;
	setarray .y[1],182,182,117,117,105,194;
	for( set .@i, 1; .@i < 7; set .@i, .@i + 1)
		donpcevent "Neutral Stone#" + .@i + "::OnBGStart";

	bg_updatescore "bat_b04",$@score_stonecontrol_blue,$@score_capture_red;
	sleep 2000;
	bg_warp $@BG12_Team1,"bat_b04",328,150;
	bg_warp $@BG12_Team2,"bat_b04",62,150;
	sleep 2000;
	donpcevent "#guisc_respawn::OnBGStart";
	donpcevent "#crosc_respawn::OnBGStart";
	end;

OnGuillaumeScore:
	set $@score_stonecontrol_blue, $@score_stonecontrol_blue + 1;
	donpcevent "Flavius_SC::OnValidateScore";
	end;

OnCroixScore:
	set $@score_capture_red, $@score_capture_red + 1;
	donpcevent "Flavius_SC::OnValidateScore";
	end;

OnValidateScore:
	if ($@BG12 != 1)
		end;
	if ($@score_stonecontrol_blue > 99)
		set $@score_stonecontrol_blue,99;
	if ($@score_capture_red > 99)
		set $@score_capture_red,99;

	bg_updatescore "bat_b04",$@score_stonecontrol_blue,$@score_capture_red;
	if ($@score_capture_red >= 99 || $@score_stonecontrol_blue >= 99)
		donpcevent "Flavius_SC::OnMatchEnd";
	end;

OnTimer600000:
	mapannounce "bat_b04","The Battle will end in 5 minutes!!",1,0x9ACD32;
	end;

OnTimer840000:
	mapannounce "bat_b04","The Battle will end in 1 minute!!",1,0x9ACD32;
	end;

OnTimer900000:
OnMatchEnd:
	stopnpctimer;
	donpcevent "#guisc_respawn::OnBGStop";
	donpcevent "#crosc_respawn::OnBGStop";
	set $@BG12, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	if ($@score_stonecontrol_blue > $@score_capture_red) { // Guillaume Won
		callfunc "BG_get_Rewards",$@BG12_Team1,7829,5;
		callfunc "BG_get_Rewards",$@BG12_Team2,7829,1;
		mapannounce "bat_b04","The Guillaume army has won the Battle of Flavius Stone Control!",1,0x0000FF;
	} else if ($@score_stonecontrol_blue < $@score_capture_red) { // Croix Won
		callfunc "BG_get_Rewards",$@BG12_Team1,7829,1;
		callfunc "BG_get_Rewards",$@BG12_Team2,7829,5;
		mapannounce "bat_b04","The Croix army has won the Battle of Flavius Stone Control!",1,0xFF0000;
	} else {
		callfunc "BG_get_Rewards",$@BG12_Team1,7829,1;
		callfunc "BG_get_Rewards",$@BG12_Team2,7829,1;
		mapannounce "bat_b04","The battle is over. This is a Tie...!",1,0x9ACD32;
	}
	// =======================================================
	set $@score_stonecontrol_blue, 0;
	set $@score_capture_red, 0;
	for( set .@i, 1; .@i < 7; set .@i, .@i + 1) { // Stop Running Timers
		stopnpctimer "Neutral Stone#" + .@i;
		stopnpctimer "csp" + .@i; // Croix Stone Point
		stopnpctimer "gsp" + .@i; // Guillaume Stone Point
	}
	sleep 5000;
	setarray .Stone[1],0,0,0,0,0,0;
	bg_updatescore "bat_b04",0,0;
	for( set .@i, 1; .@i < 7; set .@i, .@i + 1) { // Return Stones to Starting Position and Normalize Stone Points
		donpcevent "Neutral Stone#" + .@i + "::OnBGStop";
		donpcevent "csp" + .@i + "::OnBGStop";
		donpcevent "gsp" + .@i + "::OnBGStop";
	}
	$@BG12 = 0;
	mapwarp "bat_b04","bat_room",154,150;
	if($@BG12_Team1) { bg_destroy $@BG12_Team1; $@BG12_Team1 = 0; }
	if($@BG12_Team2) { bg_destroy $@BG12_Team2; $@BG12_Team2 = 0; }
	set $@score_stonecontrol_blue, 0;
	set $@score_stonecontrol_red, 0;
	bg_unbook "bat_b04";
	end;

OnFlash:
	if ($@BG12 == 1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) {
		getmapxy .@m$, .@x, .@y, BL_PC;
		if (getcharid(4) == $@BG12_Team1)
			viewpointmap "bat_b04",1, .@x, .@y, .@Stone, 0x0000FF;
		else if (getcharid(4) == $@BG12_Team2)
			viewpointmap "bat_b04",1, .@x, .@y, .@Stone, 0xFF0000;
		specialeffect(EF_BOWLINGBASH);
		emotion ET_HELP,getcharid(3);
		addtimer 2000, "Flavius_SC::OnFlash";
		percentheal -5,-5;
	}
	end;
}

// ===========================================================
// Function
// ===========================================================
function	script	SC_StoneCheck	{
	for( set .@i, 1; .@i < 7; set .@i, .@i + 1) {
		if (getvariableofnpc(.Stone[.@i],"Flavius_SC") == getarg(0))
			return .@i;
	}
	return 0;
}

// ===========================================================
// Neutral Stones
// ===========================================================
bat_b04,177,182,0	script	Neutral Stone#1	1905,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 ||  Hp < 1 || getcharid(4) == 0)
		end;
	set .@Stone,atoi(strnpcinfo(2));
	if (getvariableofnpc(.Stone[.@Stone],"Flavius_SC") != 0)
		end; // Already Captured
	if (callfunc("SC_StoneCheck",getcharid(0)) != 0)
		end; // Already with a Stone

	sc_end SC_HIDING;
	sc_end SC_CLOAKING;
	sc_end SC_CHASEWALK;
	sc_end SC_CLOAKINGEXCEED;
	sc_end SC_CAMOUFLAGE;
	sc_end SC__INVISIBILITY;

	setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

	set getvariableofnpc(.Stone[.@Stone],"Flavius_SC"),getcharid(0);
	addtimer 2000, "Flavius_SC::OnFlash";
	disablenpc strnpcinfo(0);
	stopnpctimer;
	end;

OnBGStart:
	initnpctimer;
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, atoi(strnpcinfo(2)), 0xFFFFFF;
	end;

OnBGStop:
	set .@Stone,atoi(strnpcinfo(2));
	movenpc strnpcinfo(0),getvariableofnpc(.x[.@Stone],"Flavius_SC"),getvariableofnpc(.y[.@Stone],"Flavius_SC");
	enablenpc strnpcinfo(0);
	stopnpctimer;
	end;
}

bat_b04,222,182,0	duplicate(Neutral Stone#1)	Neutral Stone#2	1905,1,1
bat_b04,222,117,0	duplicate(Neutral Stone#1)	Neutral Stone#3	1905,1,1
bat_b04,177,117,0	duplicate(Neutral Stone#1)	Neutral Stone#4	1905,1,1
bat_b04,200,105,0	duplicate(Neutral Stone#1)	Neutral Stone#5	1905,1,1
bat_b04,199,194,0	duplicate(Neutral Stone#1)	Neutral Stone#6	1905,1,1

// ===========================================================
// Stone Point - Croix Team
// ===========================================================
bat_b04,85,159,0	script	Stone Point::csp1	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team1 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Guillaume Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Croix Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team2 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";
		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
		mapannounce "bat_b04","Croix Stone captured by [ " + strcharinfo(0) + " ]",1,0xFF0000;
		setnpcdisplay strnpcinfo(3),"Croix Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0xFF0000;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnCroixScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,78,159,0	script	Stone Point::csp2	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team1 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Guillaume Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Croix Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team2 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Croix Stone captured by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		setnpcdisplay strnpcinfo(3),"Croix Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0xFF0000;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnCroixScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,71,159,0	script	Stone Point::csp3	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team1 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Guillaume Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Croix Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team2 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Croix Stone captured by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		setnpcdisplay strnpcinfo(3),"Croix Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0xFF0000;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnCroixScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,85,140,0	script	Stone Point::csp4	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team1 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Guillaume Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Croix Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team2 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Croix Stone captured by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		setnpcdisplay strnpcinfo(3),"Croix Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0xFF0000;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnCroixScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,78,140,0	script	Stone Point::csp5	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team1 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Guillaume Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Croix Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team2 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Croix Stone captured by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		setnpcdisplay strnpcinfo(3),"Croix Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0xFF0000;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnCroixScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,71,140,0	script	Stone Point::csp6	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team1 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Guillaume Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Croix Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team2 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Croix Stone captured by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		setnpcdisplay strnpcinfo(3),"Croix Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0xFF0000;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnCroixScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

// ===========================================================
// Stone Point - Guillaume Team
// ===========================================================
bat_b04,312,159,0	script	Stone Point::gsp1	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team2 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Croix Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Guillaume Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Guillaume Stone captured by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		setnpcdisplay strnpcinfo(3),"Guillaume Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0x0000FF;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnGuillaumeScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,319,159,0	script	Stone Point::gsp2	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team2 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Croix Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Guillaume Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Guillaume Stone captured by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		setnpcdisplay strnpcinfo(3),"Guillaume Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0x0000FF;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnGuillaumeScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,326,159,0	script	Stone Point::gsp3	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team2 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Croix Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Guillaume Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Guillaume Stone captured by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		setnpcdisplay strnpcinfo(3),"Guillaume Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0x0000FF;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnGuillaumeScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,312,140,0	script	Stone Point::gsp4	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team2 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Croix Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Guillaume Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Guillaume Stone captured by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		setnpcdisplay strnpcinfo(3),"Guillaume Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0x0000FF;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnGuillaumeScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,319,140,0	script	Stone Point::gsp5	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team2 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Croix Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Guillaume Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Guillaume Stone captured by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		setnpcdisplay strnpcinfo(3),"Guillaume Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0x0000FF;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnGuillaumeScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

bat_b04,326,140,0	script	Stone Point::gsp6	1309,1,1,{
	unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y);
	end;

OnTouch:
	if ($@BG12 != 1 || Hp < 1)
		end;
	if (.Point != 0 && getcharid(4) == $@BG12_Team2 && callfunc("SC_StoneCheck",getcharid(0)) == 0) { // Croix Stole
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),getcharid(0);
		setnpcdisplay strnpcinfo(3),"Stone Point",1309;

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b04","Guillaume Stone has been stolen by [ " + strcharinfo(0) + " ]",1,0xFF0000;

		addtimer 2000, "Flavius_SC::OnFlash";
		set .Point,0;
		stopnpctimer;
	} else if (.Point == 0 && getcharid(4) == $@BG12_Team1 && set(.@Stone,callfunc("SC_StoneCheck",getcharid(0))) > 0) { // Check if user got a Stone
		initnpctimer;
		set .Point,.@Stone;
		set .Count, 0;
		deltimer "Flavius_SC::OnFlash";

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);

		mapannounce "bat_b04","Guillaume Stone captured by [ " + strcharinfo(0) + " ]",1,0x0000FF;

		setnpcdisplay strnpcinfo(3),"Guillaume Stone",1905;
		set getvariableofnpc(.Stone[.Point],"Flavius_SC"),0;
	}
	end;

OnTimer2000:
	initnpctimer;
	getmapxy .@m$, .@x, .@y, BL_NPC;
	viewpointmap "bat_b04",1, .@x, .@y, .Point, 0x0000FF;
	specialeffect 223;
	if (set(.Count, .Count + 1) >= 5) {
		set .Count, 0;
		donpcevent "Flavius_SC::OnGuillaumeScore";
	}
	end;

OnBGStop:
	stopnpctimer;
	setnpcdisplay strnpcinfo(3),"Stone Point",1309;
	set .Point, 0;
	set .Count, 0;
	end;
}

// ===========================================================
// Battleground Respawn
// ===========================================================
bat_b04,390,10,0	script	#guisc_respawn	HIDDEN_WARP_NPC,{
	end;

OnBGStart:
	initnpctimer;
	end;

OnBGStop:
	stopnpctimer;
	end;

OnTimer24000:
	specialeffect(EF_SANCTUARY);
	end;

OnTimer25000:
	areapercentheal "bat_b04",382,2,397,17,100,100;
	areawarp "bat_b04",382,2,397,17,"bat_b04",311,224;
	initnpctimer;
	end;
}

bat_b04,10,290,0	script	#crosc_respawn	HIDDEN_WARP_NPC,{
	end;

OnBGStart:
	initnpctimer;
	end;

OnBGStop:
	stopnpctimer;
	end;

OnTimer24000:
	specialeffect(EF_SANCTUARY);
	end;

OnTimer25000:
	areapercentheal "bat_b04",2,282,17,297,100,100;
	areawarp "bat_b04",2,282,17,297,"bat_b04",87,75;
	initnpctimer;
	end;
}

// ===========================================================
// Flags
// ===========================================================
bat_b04,304,231,1	duplicate(Base Flag#bg)	Alpha Base#sc_1	973
bat_b04,319,231,1	duplicate(Base Flag#bg)	Alpha Base#sc_2	973
bat_b04,304,218,1	duplicate(Base Flag#bg)	Alpha Base#sc_3	973
bat_b04,319,218,1	duplicate(Base Flag#bg)	Alpha Base#sc_4	973
bat_b04,304,231,1	duplicate(Base Flag#bg)	Alpha Base#sc_5	973
bat_b04,304,231,1	duplicate(Base Flag#bg)	Alpha Base#sc_6	973
bat_b04,335,142,1	duplicate(Base Flag#bg)	Alpha Base#sc_7	973
bat_b04,335,157,1	duplicate(Base Flag#bg)	Alpha Base#sc_8	973
bat_b04,390,16,1	duplicate(Base Flag#bg)	Alpha Base#sc_9	973
bat_b04,292,163,1	duplicate(Base Flag#bg)	Alpha Base#sc_10	973
bat_b04,292,136,1	duplicate(Base Flag#bg)	Alpha Base#sc_11	973
bat_b04,241,185,1	duplicate(Base Flag#bg)	Alpha Base#sc_12	973
bat_b04,247,179,1	duplicate(Base Flag#bg)	Alpha Base#sc_13	973

bat_b04,96,81,1	duplicate(Base Flag#bg)	Omega Base#sc_1	974
bat_b04,96,68,1	duplicate(Base Flag#bg)	Omega Base#sc_2	974
bat_b04,79,81,1	duplicate(Base Flag#bg)	Omega Base#sc_3	974
bat_b04,79,68,1	duplicate(Base Flag#bg)	Omega Base#sc_4	974
bat_b04,96,81,1	duplicate(Base Flag#bg)	Omega Base#sc_5	974
bat_b04,96,81,1	duplicate(Base Flag#bg)	Omega Base#sc_6	974
bat_b04,59,164,1	duplicate(Base Flag#bg)	Omega Base#sc_7	974
bat_b04,59,137,1	duplicate(Base Flag#bg)	Omega Base#sc_8	974
bat_b04,10,296,1	duplicate(Base Flag#bg)	Omega Base#sc_9	974
bat_b04,110,162,1	duplicate(Base Flag#bg)	Omega Base#sc_10	974
bat_b04,110,137,1	duplicate(Base Flag#bg)	Omega Base#sc_11	974
bat_b04,152,120,1	duplicate(Base Flag#bg)	Omega Base#sc_12	974
bat_b04,158,114,1	duplicate(Base Flag#bg)	Omega Base#sc_13	974

// ===========================================================
// Map Flags
// ===========================================================
bat_b04	mapflag	battleground	2
bat_b04	mapflag	nomemo
bat_b04	mapflag	nosave	SavePoint
bat_b04	mapflag	noteleport
bat_b04	mapflag	nowarp
bat_b04	mapflag	nowarpto
bat_b04	mapflag	noreturn
bat_b04	mapflag	nobranch
bat_b04	mapflag	nopenalty
bat_b04	mapflag	noecall