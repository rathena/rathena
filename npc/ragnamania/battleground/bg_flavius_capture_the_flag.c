//===== rAthena Script =======================================
//= Battleground Extended + : Flavius Capture the Flag
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
// Map Flags
// ===========================================================
bat_b02	mapflag	battleground	2
bat_b02	mapflag	nomemo
bat_b02	mapflag	nosave	SavePoint
bat_b02	mapflag	noteleport
bat_b02	mapflag	nowarp
bat_b02	mapflag	nowarpto
bat_b02	mapflag	noreturn
bat_b02	mapflag	nobranch
bat_b02	mapflag	nopenalty
bat_b02	mapflag	noecall
// bat_b02	mapflag	bg_consume

// ===========================================================
// Flags
// ===========================================================
bat_b02,328,150,0	script	Alpha Crystal::Team1_Flag	1914,1,1,{
	unitwalk getcharid(3),328,150;
	end;

OnTouch:
	if ($@BG10 != 1 || Hp < 1)
		end;

	// Flag Captured
	if (getcharid(4) == $@BG10_Team2 && .Flag_Status < 2) {
		set .Flag_Status, 2; // Taken
		set .Flag_Carrier, getcharid(0);

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		// Renewal invisibility
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b02","Alpha Crystal Taken by [ " + strcharinfo(0) + " ]",1,bg_get_data($@BG10_Team1,4);
		//bg_rankpoints "ctf_taken",1;
		disablenpc "Team1_Flag";
		addtimer 2100, "Flavius_CTF::OnAlphaFlash";
		stopnpctimer;
	} else if (getcharid(4) == $@BG10_Team1) {
		if (.Flag_Status == 0 && getvariableofnpc(.Flag_Carrier,"Team2_Flag") == getcharid(0)) {
			set getvariableofnpc(.Flag_Carrier,"Team2_Flag"),0;
			set .Flag_Carrier, 0;
			mapannounce "bat_b02","Omega Crystal Captured by [ " + strcharinfo(0) + " ]!!",1,bg_get_data($@BG10_Team2,4);
			//bg_rankpoints "ctf_captured",1;
			setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
			stopnpctimer;
			$@score_capture_blue++;
			donpcevent "Flavius_CTF::OnMatchStop";
		} else if (.Flag_Status == 1) {
			mapannounce "bat_b02","Alpha Crystal Returned by [ " + strcharinfo(0) + " ]!!",1,bg_get_data($@BG10_Team1,4);
			//bg_rankpoints "fame",1;
			disablenpc "Team1_Flag";
			sleep 2100;
			movenpc "Team1_Flag",328,150; // Back to Base
			set .Flag_Status, 0;
			initnpctimer;
			enablenpc "Team1_Flag";
		}
	}
	end;

OnTimer2000:
	stopnpctimer;
	if (.Flag_Status < 2) {
		getmapxy .@m$, .@x, .@y, BL_NPC;
		viewpointmap "bat_b02",1, .@x, .@y, 1, bg_get_data($@BG10_Team1,4);
		specialeffect 223;
		initnpctimer;
	}
	end;

OnBase:
	movenpc "Team1_Flag",328,150;
	set .Flag_Status, 0;
	set .Flag_Carrier, 0;
	initnpctimer;
	enablenpc "Team1_Flag";
	end;
}

bat_b02,62,150,0	script	Omega Crystal::Team2_Flag	1915,1,1,{
	unitwalk getcharid(3),62,150;
	end;

OnTouch:
	if ($@BG10 != 1 || Hp < 1)
		end;

	// Flag Captured
	if (getcharid(4) == $@BG10_Team1 && .Flag_Status < 2) {
		set .Flag_Status, 2; // Taken
		set .Flag_Carrier, getcharid(0);

		sc_end SC_HIDING;
		sc_end SC_CLOAKING;
		sc_end SC_CHASEWALK;
		// Renewal invisibility
		sc_end SC_CLOAKINGEXCEED;
		sc_end SC_CAMOUFLAGE;
		sc_end SC__INVISIBILITY;

		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

		mapannounce "bat_b02","Omega Crystal Taken by [ " + strcharinfo(0) + " ]",1,bg_get_data($@BG10_Team2,4);
		//bg_rankpoints "ctf_taken",1;
		disablenpc "Team2_Flag";
		addtimer 2100, "Flavius_CTF::OnOmegaFlash";
		stopnpctimer;
	} else if (getcharid(4) == $@BG10_Team2) {
		if (.Flag_Status == 0 && getvariableofnpc(.Flag_Carrier,"Team1_Flag") == getcharid(0)) {
			set getvariableofnpc(.Flag_Carrier,"Team1_Flag"),0;
			set .Flag_Carrier, 0;
			mapannounce "bat_b02","Alpha Crystal Captured by [ " + strcharinfo(0) + " ]!!",1,bg_get_data($@BG10_Team1,4);
			//bg_rankpoints "ctf_captured",1;
			setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
			stopnpctimer;
			$@score_capture_red++;
			donpcevent "Flavius_CTF::OnMatchStop";
		} else if (.Flag_Status == 1) {
			mapannounce "bat_b02","Omega Crystal Returned by [ " + strcharinfo(0) + " ]!!",1,bg_get_data($@BG10_Team2,4);
			//bg_rankpoints "fame",1;
			disablenpc "Team2_Flag";
			sleep 2100;
			movenpc "Team2_Flag",62,150; // Back to Base
			set .Flag_Status, 0;
			initnpctimer;
			enablenpc "Team2_Flag";
		}
	}
	end;

OnTimer2000:
	stopnpctimer;
	if (.Flag_Status < 2) {
		getmapxy .@m$, .@x, .@y, BL_NPC;
		viewpointmap "bat_b02",1, .@x, .@y, 2, bg_get_data($@BG10_Team2,4);
		specialeffect 223;
		initnpctimer;
	}
	end;

OnBase:
	movenpc "Team2_Flag",62,150;
	set .Flag_Status, 0;
	set .Flag_Carrier, 0;
	initnpctimer;
	enablenpc "Team2_Flag";
	end;
}

// ===========================================================
// Configuration
// ===========================================================
-	script	Flavius_CTF	FAKE_NPC,{
	end;

OnAlphaFlash:
	if (getvariableofnpc(.Flag_Carrier,"Team1_Flag") == getcharid(0) && $@BG10 == 1) {
		getmapxy .@m$, .@x, .@y, BL_PC;
		viewpointmap "bat_b02",1, .@x, .@y, 1, bg_get_data($@BG10_Team1,4);
		specialeffect 73;
		emotion ET_FLAG,getcharid(3);
		addtimer 2100, "Flavius_CTF::OnAlphaFlash";
		percentheal -5,-5;
	}
	end;

OnOmegaFlash:
	if (getvariableofnpc(.Flag_Carrier,"Team2_Flag") == getcharid(0) && $@BG10 == 1) {
		getmapxy .@m$, .@x, .@y, BL_PC;
		viewpointmap "bat_b02",1, .@x, .@y, 2, bg_get_data($@BG10_Team2,4);
		specialeffect 73;
		emotion ET_FLAG,getcharid(3);
		addtimer 2100, "Flavius_CTF::OnOmegaFlash";
		percentheal -5,-5;
	}
	end;

OnInit:
	disablenpc "Team1_Flag";
	disablenpc "Team2_Flag";
	end;

OnTeam1Quit:
	bg_desert;
OnTeam1Die:
	// Drop Flag
	if ($@BG10 == 1 && getvariableofnpc(.Flag_Carrier,"Team2_Flag") == getcharid(0)) {
		set getvariableofnpc(.Flag_Carrier,"Team2_Flag"), 0;
		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
		getmapxy .@m$, .@x, .@y, BL_PC;
		movenpc "Team2_Flag", .@x, .@y;
		mapannounce "bat_b02","Omega Flag Droped by [ " + strcharinfo(0) + " ]",1,bg_get_data($@BG10_Team2,4);
		//bg_rankpoints "ctf_droped",1;
		//bg_rankpoints "fame",1,@killer_bg_src;
		set getvariableofnpc(.Flag_Status,"Team2_Flag"), 1; // OnFloor
		initnpctimer "Team2_Flag";
		enablenpc "Team2_Flag";
	}
	end;
	
OnTeam2Quit:
	bg_desert;
OnTeam2Die:
	// Drop Flag
	if ($@BG10 == 1 && getvariableofnpc(.Flag_Carrier,"Team1_Flag") == getcharid(0)) {
		set getvariableofnpc(.Flag_Carrier,"Team1_Flag"), 0;
		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
		getmapxy .@m$, .@x, .@y, BL_PC;
		movenpc "Team1_Flag", .@x, .@y;
		mapannounce "bat_b02","Alpha Flag Droped by [ " + strcharinfo(0) + " ]",1,bg_get_data($@BG10_Team1,4);
		//bg_rankpoints "ctf_droped",1;
		//bg_rankpoints "fame",1,@killer_bg_src;
		set getvariableofnpc(.Flag_Status,"Team1_Flag"), 1; // OnFloor
		initnpctimer "Team1_Flag";
		enablenpc "Team1_Flag";
	}
	end;

OnTeam1Active:
	warp "bat_b02",311,224;
	end;
OnTeam2Active:
	warp "bat_b02",87,75;
	end;

OnReady:
	//if ($@BG10)
	//	end;

	initnpctimer;
	// BG Variables
	set $@BG10,1;
	set $@score_capture_blue, 0;
	set $@score_capture_red, 0;
	sleep 2100;
	bg_warp $@BG10_Team1,"bat_b02",311,224;
	bg_warp $@BG10_Team2,"bat_b02",87,75;
	sleep 2100;
	// Respawn NPC's
	donpcevent "#guictf_respawn::OnBGStart";
	donpcevent "#croctf_respawn::OnBGStart";
	// Start Match!!
	donpcevent "Flavius_CTF::OnMatchStart";
	end;

OnMatchStart:
	if ($@BG10 != 1)
		end;
	// Reset Position Members
	if ($@score_capture_blue || $@score_capture_red) {
		bg_warp $@BG10_Team1,"bat_b02",311,224;
		bg_warp $@BG10_Team2,"bat_b02",87,75;
	}

	// Flags2Base
	donpcevent "Team1_Flag::OnBase";
	donpcevent "Team2_Flag::OnBase";
	mapannounce "bat_b02","The Flags have been set to their Bases!!",8;
	end;

OnMatchStop:
	disablenpc "Team1_Flag";
	disablenpc "Team2_Flag";
	bg_updatescore "bat_b02",$@score_capture_blue,$@score_capture_red;

	viewpointmap "bat_b02",2, 0, 0, 1, 0x0000FF;
	viewpointmap "bat_b02",2, 0, 0, 2, 0xFF0000;

	// Team 1 Won
	if ($@score_capture_blue > 2) {
		mapannounce "bat_b02","The " + bg_get_data($@BG10_Team1,2) + " army has won the Battle of Flavius CTF!",1,bg_get_data($@BG10_Team1,4);
		donpcevent "Flavius_CTF::OnMatchEnd";
	}
	// Team 2 Won
	else if ($@score_capture_red > 2) {
		mapannounce "bat_b02","The " + bg_get_data($@BG10_Team2,2) + " army has won the Battle of Flavius CTF!",1,bg_get_data($@BG10_Team2,4);
		donpcevent "Flavius_CTF::OnMatchEnd";
	}
	// Keep Playing
	else {
		sleep 8000;
		donpcevent "Flavius_CTF::OnMatchStart";
	}
	end;

OnTimer600000:
	mapannounce "bat_b02","The Battle will ends in 5 minutes!!",1,0xA0522D;
	end;

OnTimer840000:
	mapannounce "bat_b02","The Battle will ends in 1 minute!!",1,0xA0522D;
	end;

OnTimer900000:
	disablenpc "Team1_Flag";
	disablenpc "Team2_Flag";

	viewpointmap "bat_b02",2, 0, 0, 1, bg_get_data($@BG10_Team1,4);
	viewpointmap "bat_b02",2, 0, 0, 2, bg_get_data($@BG10_Team2,4);

	if ($@score_capture_blue > $@score_capture_red)
		mapannounce "bat_b02","The " + bg_get_data($@BG10_Team1,2) + " army has won the Battle of Flavius CTF!",1,bg_get_data($@BG10_Team1,4);
	else if ($@score_capture_blue < $@score_capture_red)
		mapannounce "bat_b02","The " + bg_get_data($@BG10_Team2,2) + " army has won the Battle of Flavius CTF!",1,bg_get_data($@BG10_Team2,4);
	else
		mapannounce "bat_b02","The battle is over. This is a Tie...!",1,0xA0522D;
	donpcevent "Flavius_CTF::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	disablenpc "Team1_Flag";
	disablenpc "Team2_Flag";
	donpcevent "#guictf_respawn::OnBGStop";
	donpcevent "#croctf_respawn::OnBGStop";
	set $@BG10, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	if ($@score_capture_blue > $@score_capture_red) {
		callfunc "BG_get_Rewards",$@BG10_Team1,7829,5;
		callfunc "BG_get_Rewards",$@BG10_Team2,7829,1;
	} else if ($@score_capture_red > $@score_capture_blue) {
		callfunc "BG_get_Rewards",$@BG10_Team1,7829,1;
		callfunc "BG_get_Rewards",$@BG10_Team2,7829,5;
	} else {
		callfunc "BG_get_Rewards",$@BG10_Team1,7829,1;
		callfunc "BG_get_Rewards",$@BG10_Team2,7829,1;
	}

	// =======================================================
	sleep 5000;
	$@BG10 = 0;
	mapwarp "bat_b02","bat_room",154,150;
	set $@score_capture_blue, 0;
	set $@score_capture_red, 0;
	if($@BG10_Team1) { bg_destroy $@BG10_Team1; $@BG10_Team1 = 0; }
	if($@BG10_Team2) { bg_destroy $@BG10_Team2; $@BG10_Team2 = 0; }
	bg_unbook "bat_b02";
	end;
}

// ===========================================================
// Healer
// ===========================================================
bat_b02,390,13,5	duplicate(BGHeal)	Therapist in battle#ctf1	4_F_SISTER
bat_b02,10,293,5	duplicate(BGHeal)	Therapist in battle#ctf2	4_F_SISTER

// ===========================================================
// Respawn
// ===========================================================
bat_b02,390,10,0	script	#guictf_respawn	HIDDEN_WARP_NPC,{
	end;

OnBGStart:
	initnpctimer;
	end;

OnBGStop:
	stopnpctimer;
	end;

OnTimer24000:
	specialeffect 83;
	end;

OnTimer25000:
	areapercentheal "bat_b02",382,2,397,17,100,100;
	areawarp "bat_b02",382,2,397,17,"bat_b02",311,224;
	initnpctimer;
	end;
}

bat_b02,10,290,0	script	#croctf_respawn	HIDDEN_WARP_NPC,{
	end;

OnBGStart:
	initnpctimer;
	end;

OnBGStop:
	stopnpctimer;
	end;

OnTimer24000:
	specialeffect 83;
	end;

OnTimer25000:
	areapercentheal "bat_b02",2,282,17,297,100,100;
	areawarp "bat_b02",2,282,17,297,"bat_b02",87,75;
	initnpctimer;
	end;
}

// ===========================================================
// Flags
// ===========================================================
bat_b02,304,231,1	duplicate(Base Flag#bg)	Alpha Base#bat23	1_FLAG_LION
bat_b02,319,231,1	duplicate(Base Flag#bg)	Alpha Base#bat24	1_FLAG_LION
bat_b02,304,218,1	duplicate(Base Flag#bg)	Alpha Base#bat25	1_FLAG_LION
bat_b02,319,218,1	duplicate(Base Flag#bg)	Alpha Base#bat26	1_FLAG_LION
bat_b02,304,231,1	duplicate(Base Flag#bg)	Alpha Base#bat27	1_FLAG_LION
bat_b02,304,231,1	duplicate(Base Flag#bg)	Alpha Base#bat28	1_FLAG_LION
bat_b02,335,142,1	duplicate(Base Flag#bg)	Alpha Base#bat29	1_FLAG_LION
bat_b02,335,157,1	duplicate(Base Flag#bg)	Alpha Base#bat30	1_FLAG_LION
bat_b02,390,16,1	duplicate(Base Flag#bg)	Alpha Base#bat31	1_FLAG_LION
bat_b02,292,163,1	duplicate(Base Flag#bg)	Alpha Base#bat32	1_FLAG_LION
bat_b02,292,136,1	duplicate(Base Flag#bg)	Alpha Base#bat33	1_FLAG_LION
bat_b02,241,185,1	duplicate(Base Flag#bg)	Alpha Base#bat34	1_FLAG_LION
bat_b02,247,179,1	duplicate(Base Flag#bg)	Alpha Base#bat35	1_FLAG_LION

bat_b02,96,81,1	duplicate(Base Flag#bg)	Omega Base#bat22	1_FLAG_EAGLE
bat_b02,96,68,1	duplicate(Base Flag#bg)	Omega Base#bat23	1_FLAG_EAGLE
bat_b02,79,81,1	duplicate(Base Flag#bg)	Omega Base#bat24	1_FLAG_EAGLE
bat_b02,79,68,1	duplicate(Base Flag#bg)	Omega Base#bat25	1_FLAG_EAGLE
bat_b02,96,81,1	duplicate(Base Flag#bg)	Omega Base#bat26	1_FLAG_EAGLE
bat_b02,96,81,1	duplicate(Base Flag#bg)	Omega Base#bat27	1_FLAG_EAGLE
bat_b02,59,164,1	duplicate(Base Flag#bg)	Omega Base#bat28	1_FLAG_EAGLE
bat_b02,59,137,1	duplicate(Base Flag#bg)	Omega Base#bat29	1_FLAG_EAGLE
bat_b02,10,296,1	duplicate(Base Flag#bg)	Omega Base#bat30	1_FLAG_EAGLE
bat_b02,110,162,1	duplicate(Base Flag#bg)	Omega Base#bat31	1_FLAG_EAGLE
bat_b02,110,137,1	duplicate(Base Flag#bg)	Omega Base#bat32	1_FLAG_EAGLE
bat_b02,152,120,1	duplicate(Base Flag#bg)	Omega Base#bat33	1_FLAG_EAGLE
bat_b02,158,114,1	duplicate(Base Flag#bg)	Omega Base#bat34	1_FLAG_EAGLE
