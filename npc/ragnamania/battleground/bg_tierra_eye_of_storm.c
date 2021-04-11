//===== rAthena Script =======================================
//= Battleground Extended + : Tierra Eye of Storm
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
-	script	Tierra_EOS	FAKE_NPC,{
	end;

OnTeam1Quit:
	set @killer_bg_src, 0;
	bg_desert;
OnTeam1Die:
	// Drop Flag
	if ($@BG18 == 1 && getvariableofnpc(.Flag_Carrier,"Neutral_Flag") == getcharid(0)) {
		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
		getmapxy .@m$, .@x, .@y, BL_PC;
		movenpc "Neutral_Flag", .@x, .@y;
		mapannounce "bat_a02",bg_get_data($@BG_Team1,2) + " have droped the Flag",1,0xFFFFFF;
		//bg_rankpoints "fame",1,@killer_bg_src;
		set getvariableofnpc(.Flag_Status,"Neutral_Flag"), 0; // OnFloor
		set getvariableofnpc(.Flag_Carrier,"Neutral_Flag"), 0;
		initnpctimer "Neutral_Flag";
		enablenpc "Neutral_Flag";
	}
	end;

OnTeam2Quit:
	set @killer_bg_src, 0;
	bg_desert;
OnTeam2Die:
	// Drop Flag
	if ($@BG18 == 1 && getvariableofnpc(.Flag_Carrier,"Neutral_Flag") == getcharid(0)) {
		setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
		getmapxy .@m$, .@x, .@y, BL_PC;
		movenpc "Neutral_Flag", .@x, .@y;
		mapannounce "bat_a02",bg_get_data($@BG_Team2,2) + " have droped the Flag",1,0xFFFFFF;
		//bg_rankpoints "fame",1,@killer_bg_src;
		set getvariableofnpc(.Flag_Status,"Neutral_Flag"), 0; // OnFloor
		set getvariableofnpc(.Flag_Carrier,"Neutral_Flag"), 0;
		initnpctimer "Neutral_Flag";
		enablenpc "Neutral_Flag";
	}
	end;

OnTeam1Active:
	warp "bat_a02",353,344;
	end;
OnTeam2Active:
	warp "bat_a02",353,52;
	end;

OnReady:
	//if ($@BG18)
	//	end;

	set $@BG18,1;
	// BG Variables
	set $@score_eyeofstorm_blue, 0;
	set $@score_eyeofstorm_red, 0;
	sleep 2100;
	bg_warp $@BG18_Team1,"bat_a02",353,344;
	bg_warp $@BG18_Team2,"bat_a02",353,52;
	sleep 2100;
	// Respawn NPC's
	donpcevent "#guieoe_respawn::OnBGStart";
	donpcevent "#croeoe_respawn::OnBGStart";
	// Start Match!!
	donpcevent "North_Base::OnBase";
	donpcevent "South_Base::OnBase";
	donpcevent "Neutral_Flag::OnBase";
	donpcevent "::OnStartStorm";
	mapannounce "bat_a02","The Battle of Tierra Valley - Eye of Storm has begun",1,0x4169E1;
	end;

OnFlash:
	if (getvariableofnpc(.Flag_Carrier,"Neutral_Flag") == getcharid(0) && $@BG18 == 1 ) {
		getmapxy .@m$, .@x, .@y, BL_PC;
		viewpointmap "bat_a02",1,.@x,.@y,3,0xFFFFFF;
		specialeffect 73;
		emotion ET_HELP,getcharid(3);
		addtimer 2100, "Tierra_EOS::OnFlash";
		percentheal -5,-5;
	}
	end;

OnTeam1Flag:
	viewpointmap "bat_a02",2,0,0,3,0xFFFFFF;
	set .@North, getvariableofnpc(.Owner,"North_Base");
	set .@South, getvariableofnpc(.Owner,"South_Base");

	if (.@North == .@South) {
		set $@score_eyeofstorm_blue, $@score_eyeofstorm_blue + 5;
		mapannounce "bat_a02",bg_get_data($@BG18_Team1,2) + " captured the Flag [+5 points]",1,0x0000FF;
	} else {
		set $@score_eyeofstorm_blue, $@score_eyeofstorm_blue + 3;
		mapannounce "bat_a02",bg_get_data($@BG18_Team1,2) + " captured the Flag [+3 points]",1,0x0000FF;
	}

	donpcevent "Tierra_EOS::OnValidateScore";
	donpcevent "Neutral_Flag::OnBase";
	end;

OnTeam2Flag:
	viewpointmap "bat_a02",2,0,0,3,0xFFFFFF;
	set .@North, getvariableofnpc(.Owner,"North_Base");
	set .@South, getvariableofnpc(.Owner,"South_Base");
	
	if (.@North == .@South) {
		set $@score_eyeofstorm_red, $@score_eyeofstorm_red + 5;
		mapannounce "bat_a02",bg_get_data($@BG18_Team2,2) + " captured the Flag [+5 points]",1,0xFF0000;
	} else {
		set $@score_eyeofstorm_red, $@score_eyeofstorm_red + 3;
		mapannounce "bat_a02",bg_get_data($@BG18_Team2,2) + " captured the Flag [+3 points]",1,0xFF0000;
	}

	donpcevent "Tierra_EOS::OnValidateScore";
	donpcevent "Neutral_Flag::OnBase";
	end;

OnNorthScore:
	if (set(.@North, getvariableofnpc(.Owner,"North_Base")) == 0)
		end; // No Owner
	set .@South, getvariableofnpc(.Owner,"South_Base");
	// Double Domination
	if (.@North == .@South) {
		if (.@North == $@BG18_Team1)
			set $@score_eyeofstorm_blue, $@score_eyeofstorm_blue + 2;
		else
			set $@score_eyeofstorm_red, $@score_eyeofstorm_red + 2;
	} else if (.@North == $@BG18_Team1)
		set $@score_eyeofstorm_blue, $@score_eyeofstorm_blue + 1;
	else
		set $@score_eyeofstorm_red, $@score_eyeofstorm_red + 1;

	donpcevent "Tierra_EOS::OnValidateScore";
	end;
	
OnSouthScore:
	if (set(.@South, getvariableofnpc(.Owner,"South_Base")) == 0)
		end; // No Owner
	set .@North, getvariableofnpc(.Owner,"North_Base");
	// Double Domination
	if (.@North == .@South) {
		if (.@South == $@BG18_Team1)
			set $@score_eyeofstorm_blue, $@score_eyeofstorm_blue + 2;
		else
			set $@score_eyeofstorm_red, $@score_eyeofstorm_red + 2;
	} else if (.@South == $@BG18_Team1)
		set $@score_eyeofstorm_blue, $@score_eyeofstorm_blue + 1;
	else
		set $@score_eyeofstorm_red, $@score_eyeofstorm_red + 1;

	donpcevent "Tierra_EOS::OnValidateScore";
	end;

OnValidateScore:
	if ($@score_eyeofstorm_blue > 99)
		set $@score_eyeofstorm_blue, 99;
	if ($@score_eyeofstorm_red > 99)
		set $@score_eyeofstorm_red, 99;

	bg_updatescore "bat_a02",$@score_eyeofstorm_blue,$@score_eyeofstorm_red; // Update Visual Score

	if ($@score_eyeofstorm_blue < 99 && $@score_eyeofstorm_red < 99)
		end; // No winners
OnMatchEnd:

	donpcevent "Neutral_Flag::OnDisable";
	stopnpctimer "North_Base";
	stopnpctimer "South_Base";
	donpcevent "#guieoe_respawn::OnBGStop";
	donpcevent "#croeoe_respawn::OnBGStop";
	set $@BG18, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	.@reward_win = 5;
	.@reward_lose = 3;

	// Tie
	if ($@score_eyeofstorm_blue >= 99 && $@score_eyeofstorm_red >= 99) {
		callfunc "BG_get_Rewards",$@BG18_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG18_Team2,7828,1;
		mapannounce "bat_a02","The battle is over. This is a Tie...!",1,0x4169E1;
	}
	// Team 1 Won
	else if ($@score_eyeofstorm_blue >= 99) {
		callfunc "BG_get_Rewards",$@BG18_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG18_Team2,7828,1;
		mapannounce "bat_a02","The " + bg_get_data($@BG18_Team1,2) + " has won the Battle of Tierra EoS!",1,bg_get_data($@BG18_Team1,4);
	}
	// Team 2 Won
	else if ($@score_eyeofstorm_red >= 99) {
		callfunc "BG_get_Rewards",$@BG18_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG18_Team2,7828,5;
		mapannounce "bat_a02","The " + bg_get_data($@BG18_Team2,2) + " has won the Battle of Tierra EoS!",1,bg_get_data($@BG18_Team2,4);
	}
	// =======================================================
	set $@score_eyeofstorm_blue, 0;
	set $@score_eyeofstorm_red, 0;
	sleep 5000;
	$@BG18 = 0;
	mapwarp "bat_a02","bat_room",154,150;
	if($@BG18_Team1) { bg_destroy $@BG18_Team1; $@BG18_Team1 = 0; }
	if($@BG18_Team2) { bg_destroy $@BG18_Team2; $@BG18_Team2 = 0; }
	bg_unbook "bat_a02";
	bg_updatescore "bat_a02",$@score_eyeofstorm_blue,$@score_eyeofstorm_red;
	donpcevent "GEoSNBW::OnDisable";
	//donpcevent "GEoSCBW::OnDisable";
	donpcevent "GEoSSBW::OnDisable";
	donpcevent "CEoSNBW::OnDisable";
	//donpcevent "CEoSCBW::OnDisable";
	donpcevent "CEoSSBW::OnDisable";
	end;

OnBreak:
	if ($@BG18 != 1 )
		end;

	if (getcharid(4) == $@BG18_Team1)
		mapannounce "bat_a02","Netraul Flag Taken by " + bg_get_data($@BG18_Team1,2),1,bg_get_data($@BG18_Team1,4);
	else if (getcharid(4) == $@BG18_Team2)
		mapannounce "bat_a02","Netraul Flag Taken by " + bg_get_data($@BG18_Team2,2),1,bg_get_data($@BG18_Team2,4);
	else end;

	set getvariableofnpc(.Flag_Status,"Neutral_Flag"), 1;
	set getvariableofnpc(.Flag_Carrier,"Neutral_Flag"), getcharid(0);

	sc_end SC_HIDING;
	sc_end SC_CLOAKING;
	sc_end SC_CHASEWALK;

	// Renewal invisibility
	sc_end SC_CLOAKINGEXCEED;
	sc_end SC_CAMOUFLAGE;
	sc_end SC__INVISIBILITY;

	setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

	stopnpctimer "Neutral_Flag";
	disablenpc "Neutral_Flag";

	addtimer 2100, "Tierra_EOS::OnFlash";
	end;
}

// ===========================================================
// Flags
// ===========================================================
bat_a02,273,204,0	script	Neutral Flag::Neutral_Flag	1911,1,1,{
	end;

OnTouch:
	if ($@BG18 != 1 || Hp < 1 || .Flag_Status)
		end;

	if (getcharid(4) == $@BG18_Team1)
		mapannounce "bat_a02","Netraul Flag Taken by " + bg_get_data($@BG18_Team1,2),1,bg_get_data($@BG18_Team1,4);
	else if (getcharid(4) == $@BG18_Team2)
		mapannounce "bat_a02","Netraul Flag Taken by " + bg_get_data($@BG18_Team2,2),1,bg_get_data($@BG18_Team2,4);
	else end;

	set .Flag_Status, 1;
	set .Flag_Carrier, getcharid(0);
	//bg_rankpoints "fame",1;

	sc_end SC_HIDING;
	sc_end SC_CLOAKING;
	sc_end SC_CHASEWALK;

	// Renewal invisibility
	sc_end SC_CLOAKINGEXCEED;
	sc_end SC_CAMOUFLAGE;
	sc_end SC__INVISIBILITY;

	setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,true);

	disablenpc "Neutral_Flag";
	addtimer 2100, "Tierra_EOS::OnFlash";
	stopnpctimer;
	end;

OnTimer2000:
	stopnpctimer;
	if (.Flag_Status == 0 && $@BG18 == 1 ) {
		getmapxy .@m$, .@x, .@y, BL_NPC;
		viewpointmap "bat_a02",1,.@x,.@y,3,0xFFFFFF;
		specialeffect 223;
		initnpctimer;
	}
	end;

OnDisable:
	movenpc "Neutral_Flag",273,204;
	set .Flag_Status, 0;
	stopnpctimer;
	disablenpc "Neutral_Flag";
	killmonster "bat_a02","Tierra_EOS::OnBreak";
	end;

OnBase:
	if ($@BG18 != 1 )
		end;

	movenpc "Neutral_Flag",273,204;
	set .Flag_Status, 0;
	set .Flag_Carrier, 0;
	initnpctimer;
	disablenpc "Neutral_Flag";
	mapannounce "bat_a02","The Neutral Flag have been set!!",1,0xFFFFFF;
	bg_monster 0,"bat_a02",273,204,"Neutral Flag",1911,"Tierra_EOS::OnBreak";
	end;
}

// ===========================================================
// Bases
// ===========================================================
bat_a02,173,345,0	script	North Base::North_Base	1911,3,3,{
	end;

OnTouch:
	if ($@BG18 != 1 || .Owner == 0 || .Owner != getcharid(4) || getvariableofnpc(.Flag_Carrier,"Neutral_Flag") != getcharid(0))
		end;

	//bg_rankpoints "eos_flags",1;
	set getvariableofnpc(.Flag_Carrier,"Neutral_Flag"), 0;
	setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
	if (.Owner == $@BG18_Team1)
		donpcevent "Tierra_EOS::OnTeam1Flag";
	else
		donpcevent "Tierra_EOS::OnTeam2Flag";
	end;

OnBase:
	set .Owner, 0;
	set .Balance, 0;
	set .Tick, 0;
	setnpcdisplay "North_Base","North Base",1911;
	initnpctimer;
	end;

OnTimer1000:
	stopnpctimer;
	if ($@BG18 != 1 )
		end;

	set .@Team1Count, bg_getareausers($@BG18_Team1,"bat_a02",136,329,186,361);
	set .@Team2Count, bg_getareausers($@BG18_Team2,"bat_a02",136,329,186,361);
	set .Balance, .Balance + set(.@Balance, .@Team1Count - .@Team2Count);

	if (.Balance < -50)
		set .Balance, -50;
	else if (.Balance > 50)
		set .Balance, 50;

	if (.Owner == 0) {
		if (.Balance == 50) {
			set .Balance, 50;
			set .Owner, $@BG18_Team1; // Team 1
			setnpcdisplay "North_Base",bg_get_data($@BG18_Team1,2) + " Base",1912;
			mapannounce "bat_a02","North Base captured by " + bg_get_data($@BG18_Team1,2),1,bg_get_data($@BG18_Team1,4);
			//bg_rankpoints_area $@BG18_Team1,"bat_a02",136,329,186,361,"eos_bases",1;
			donpcevent "GEoSNBW::OnEnable";
		} else if (.Balance == -50) {
			set .Balance, -50;
			set .Owner, $@BG18_Team2; // Team 2
			setnpcdisplay "North_Base",bg_get_data($@BG18_Team2,2) + " Base",1913;
			mapannounce "bat_a02","North Base captured by " + bg_get_data($@BG18_Team2,2),1,bg_get_data($@BG18_Team2,4);
			//bg_rankpoints_area $@BG18_Team2,"bat_a02",136,329,186,361,"eos_bases",1;
			donpcevent "CEoSNBW::OnEnable";
		}
	} else if (.Owner == $@BG18_Team1) {
		if (.Balance <= 0) { // Team 1 lost Control
			set .Owner, 0;
			setnpcdisplay "North_Base","North Base",1911;
			mapannounce "bat_a02",bg_get_data($@BG18_Team1,2) + " lost control of the North Base",1,bg_get_data($@BG18_Team1,4);
			donpcevent "GEoSNBW::OnDisable";
		}
	} else if (.Owner == $@BG18_Team2) {
		if (.Balance >= 0) { // Team 2 lost Control
			set .Owner, 0;
			setnpcdisplay "North_Base","North Base",1911;
			mapannounce "bat_a02",bg_get_data($@BG18_Team2,2) + " lost control of the North Base",1,bg_get_data($@BG18_Team2,4);
			donpcevent "CEoSNBW::OnDisable";
		}
	}

	if (.@Balance > 0)
		specialeffect 236;
	else if (.@Balance < 0)
		specialeffect 225;
	else if (.Owner == $@BG18_Team1)
		specialeffect 236;
	else if (.Owner == $@BG18_Team2)
		specialeffect 225;
	else
		specialeffect 223;
	
	if (.Owner == 0)
		viewpointmap "bat_a02",1,173,345,1,0xFFFFFF;
	else if (.Owner == $@BG18_Team1)
		viewpointmap "bat_a02",1,173,345,1,bg_get_data($@BG18_Team1,4);
	else if (.Owner == $@BG18_Team2)
		viewpointmap "bat_a02",1,173,345,1,bg_get_data($@BG18_Team2,4);

	if (.Owner == 0)
		set .Tick, 0;
	else if (set(.Tick, .Tick + 1) == 7) {
		donpcevent "Tierra_EOS::OnNorthScore";
		set .Tick, 0;
	}

	initnpctimer;
	end;
}

bat_a02,164,50,0	script	South Base::South_Base	1911,3,3,{
	end;

OnTouch:
	if ($@BG18 != 1 || .Owner == 0 || .Owner != getcharid(4) || getvariableofnpc(.Flag_Carrier,"Neutral_Flag") != getcharid(0))
		end;

	//bg_rankpoints "eos_flags",1;
	set getvariableofnpc(.Flag_Carrier,"Neutral_Flag"), 0;
	setpcblock(PCBLOCK_SKILL|PCBLOCK_USEITEM,false);
	if (.Owner == $@BG18_Team1)
		donpcevent "Tierra_EOS::OnTeam1Flag";
	else
		donpcevent "Tierra_EOS::OnTeam2Flag";
	end;

OnBase:
	set .Owner, 0;
	set .Balance, 0;
	set .Tick, 0;
	setnpcdisplay "South_Base","South Base",1911;
	initnpctimer;
	end;

OnTimer1000:
	stopnpctimer;
	if ($@BG18 != 1)
		end;

	set .@Team1Count, bg_getareausers($@BG18_Team1,"bat_a02",129,34,175,65);
	set .@Team2Count, bg_getareausers($@BG18_Team2,"bat_a02",129,34,175,65);
	set .Balance, .Balance + set(.@Balance, .@Team1Count - .@Team2Count);

	if (.Balance < -50)
		set .Balance, -50;
	else if (.Balance > 50)
		set .Balance, 50;

	if (.Owner == 0) {
		if (.Balance == 50) {
			set .Balance, 50;
			set .Owner, $@BG18_Team1; // Team 1
			setnpcdisplay "South_Base",bg_get_data($@BG18_Team1,2) + " Base",1912;
			mapannounce "bat_a02","South Base captured by " + bg_get_data($@BG18_Team1,2),1,bg_get_data($@BG18_Team1,4);
			//bg_rankpoints_area $@BG18_Team1,"bat_a02",129,34,175,65,"eos_bases",1;
			donpcevent "GEoSSBW::OnEnable";
		} else if (.Balance == -50) {
			set .Balance, -50;
			set .Owner, $@BG18_Team2; // Team 2
			setnpcdisplay "South_Base",bg_get_data($@BG18_Team2,2) + " Base",1913;
			mapannounce "bat_a02","South Base captured by " + bg_get_data($@BG18_Team2,2),1,bg_get_data($@BG18_Team2,4);
			//bg_rankpoints_area $@BG18_Team2,"bat_a02",129,34,175,65,"eos_bases",1;
			donpcevent "CEoSSBW::OnEnable";
		}
	} else if (.Owner == $@BG18_Team1) {
		if (.Balance <= 0) { // Team 1 lost Control
			set .Owner, 0;
			setnpcdisplay "South_Base","North Base",1911;
			mapannounce "bat_a02",bg_get_data($@BG18_Team1,2) + " lost control of the South Base",1,bg_get_data($@BG18_Team1,4);
			donpcevent "GEoSSBW::OnDisable";
		}
	} else if (.Owner == $@BG18_Team2) {
		if (.Balance >= 0) { // Team 2 lost Control
			set .Owner, 0;
			setnpcdisplay "South_Base","North Base",1911;
			mapannounce "bat_a02",bg_get_data($@BG18_Team2,2) + " lost control of the South Base",1,bg_get_data($@BG18_Team2,4);
			donpcevent "CEoSSBW::OnDisable";
		}
	}

	if (.@Balance > 0)
		specialeffect 236;
	else if (.@Balance < 0)
		specialeffect 225;
	else if (.Owner == $@BG18_Team1)
		specialeffect 236;
	else if (.Owner == $@BG18_Team2)
		specialeffect 225;
	else
		specialeffect 223;
	
	if (.Owner == 0)
		viewpointmap "bat_a02",1,164,50,2,0xFFFFFF;
	else if (.Owner == $@BG18_Team1)
		viewpointmap "bat_a02",1,164,50,2,bg_get_data($@BG18_Team1,4);
	else if (.Owner == $@BG18_Team2)
		viewpointmap "bat_a02",1,164,50,2,bg_get_data($@BG18_Team2,4);

	if (.Owner == 0)
		set .Tick, 0;
	else if (set(.Tick, .Tick + 1) == 7) {
		donpcevent "Tierra_EOS::OnSouthScore";
		set .Tick, 0;
	}

	initnpctimer;
	end;
}

// Battleground Therapist
// *********************************************************************

// ===========================================================
// Healer
// ===========================================================
bat_a02,53,377,3	duplicate(BGHeal)	Therapist in battle#eoe2	4_F_SISTER
bat_a02,45,18,3	duplicate(BGHeal)	Therapist in battle#eoe1	4_F_SISTER

// ===========================================================
// Respawn
// ===========================================================
bat_a02,46,377,0	script	North Base Warp::GEoSNBW	1_SHADOW_NPC,{
	end;

	OnEnable:
		waitingroom "Join To Warp",20;
		end;

	OnDisable:
		delwaitingroom;
		end;

	OnWarp:
		warpwaitingpc "bat_a02",115,346,20;
		end;
}

bat_a02,53,370,0	script	South Base Warp::GEoSSBW	1_SHADOW_NPC,{
	end;

	OnEnable:
		waitingroom "Join To Warp",20;
		end;

	OnDisable:
		delwaitingroom;
		end;

	OnWarp:
		warpwaitingpc "bat_a02",106,48,20;
		end;
}

bat_a02,38,19,0	script	North Base Warp::CEoSNBW	1_SHADOW_NPC,{
	end;

	OnEnable:
		waitingroom "Join To Warp",20;
		end;

	OnDisable:
		delwaitingroom;
		end;

	OnWarp:
		warpwaitingpc "bat_a02",115,346,20;
		end;
}

bat_a02,45,12,0	script	South Base Warp::CEoSSBW	1_SHADOW_NPC,{
	end;

	OnEnable:
		waitingroom "Join To Warp",20;
		end;

	OnDisable:
		delwaitingroom;
		end;

	OnWarp:
		warpwaitingpc "bat_a02",106,48,20;
		end;
}

bat_a02,50,374,0	script	#guieoe_respawn	HIDDEN_WARP_NPC,{
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
		areapercentheal "bat_a02",46,370,54,378,100,100;
		donpcevent "GEoSNBW::OnWarp";
		donpcevent "GEoSSBW::OnWarp";
		areawarp "bat_a02",46,370,54,378,"bat_a02",353,344;
		initnpctimer;
		end;
}

bat_a02,42,16,0	script	#croeoe_respawn	HIDDEN_WARP_NPC,{
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
		areapercentheal "bat_a02",38,12,47,21,100,100;
		donpcevent "CEoSNBW::OnWarp";
		donpcevent "CEoSSBW::OnWarp";
		areawarp "bat_a02",38,12,47,21,"bat_a02",353,52;
		initnpctimer;
		end;
}

// MapFlags
// *********************************************************************

bat_a02	mapflag	battleground	2
bat_a02	mapflag	nomemo
bat_a02	mapflag	nosave	SavePoint
bat_a02	mapflag	noteleport
bat_a02	mapflag	nowarp
bat_a02	mapflag	nowarpto
bat_a02	mapflag	noreturn
bat_a02	mapflag	nobranch
bat_a02	mapflag	nopenalty
bat_a02	mapflag	noecall
//bat_a02	mapflag	bg_consume

// Eye of the Storm Effects
// *********************************************************************

bat_a02,269,189,0	script	#stormef1	HIDDEN_WARP_NPC,{
	end;

OnStartStorm:
	if ($@BG18 != 1) end;

	sleep (rand(15,40) * 1000);
	specialeffect 622;
	specialeffect 537;

	goto OnStartStorm;
	end;
}

bat_a02,284,189,0	duplicate(#stormef1)	#stormef2	HIDDEN_WARP_NPC
bat_a02,267,204,0	duplicate(#stormef1)	#stormef3	HIDDEN_WARP_NPC
bat_a02,278,210,0	duplicate(#stormef1)	#stormef4	HIDDEN_WARP_NPC
bat_a02,262,210,0	duplicate(#stormef1)	#stormef5	HIDDEN_WARP_NPC
bat_a02,256,201,0	duplicate(#stormef1)	#stormef6	HIDDEN_WARP_NPC
bat_a02,284,218,0	duplicate(#stormef1)	#stormef7	HIDDEN_WARP_NPC
bat_a02,263,220,0	duplicate(#stormef1)	#stormef8	HIDDEN_WARP_NPC
bat_a02,289,207,0	duplicate(#stormef1)	#stormef9	HIDDEN_WARP_NPC
bat_a02,279,182,0	duplicate(#stormef1)	#stormef10	HIDDEN_WARP_NPC
bat_a02,272,229,0	duplicate(#stormef1)	#stormef11	HIDDEN_WARP_NPC
