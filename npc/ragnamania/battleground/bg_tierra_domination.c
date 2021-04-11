//===== rAthena Script =======================================
//= Battleground Extended + : Tierra Domination
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
-	script	Tierra_DOM	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;

OnTeam1Active:
	warp "bat_a04",353,344;
	end;
OnTeam2Active:
	warp "bat_a04",353,52;
	end;

OnDie:
	// Check for Offensive or Defensive Kills
	if ($@BG17 != 1 )
		end;

	if (!@killer_bg_id || @killer_bg_id == getcharid(4))
		end;

	set .@Base, 0;
	getmapxy .@m$, .@x, .@y, BL_PC;

	if (.@x >= 136 && .@y >= 329 && .@x <= 186 && .@y <= 361)
		set .@Base, getvariableofnpc(.Owner,"Dom_N_Base"); // North
	else if (.@x >= 260 && .@y >= 194 && .@x <= 287 && .@y <= 213)
		set .@Base, getvariableofnpc(.Owner,"Dom_C_Base"); // Center
	else if (.@x >= 129 && .@y >= 34 && .@x <= 175 && .@y <= 65)
		set .@Base, getvariableofnpc(.Owner,"Dom_S_Base"); // South
	else end; // Not Killed on Base territory

	//if (.@Base == 1)
	//{ // Team 1
		//if (getcharid(4) == $@BG_Team1)
		//	//bg_rankpoints "dom_off_kills",1,@killer_bg_src;
		//else
		//	//bg_rankpoints "dom_def_kills",1,@killer_bg_src;
	//}
	//else if (.@Base == 2)
	//{ // Team 2
		//if (getcharid(4) == $@BG_Team2)
		//	bg_rankpoints "dom_off_kills",1,@killer_bg_src;
		//else
		//	bg_rankpoints "dom_def_kills",1,@killer_bg_src;
	//}
	end;

OnReady:
	//if ($@BG17)
	//	end;

	set $@BG17,1;
	set $@score_domination_blue, 0;
	set $@score_domination_red, 0;
	bg_updatescore "bat_a04",$@score_domination_blue,$@score_domination_red;
	donpcevent "Dom_N_Base::OnBase";
	donpcevent "Dom_S_Base::OnBase";
	donpcevent "Dom_C_Base::OnBase";
	sleep 2100;
	bg_warp $@BG17_Team1,"bat_a04",353,344;
	bg_warp $@BG17_Team2,"bat_a04",353,52;
	sleep 2100;
	donpcevent "#guiDOM_respawn::OnBGStart";
	donpcevent "#croDOM_respawn::OnBGStart";
	end;

OnNScore:
	if (set(.@North, getvariableofnpc(.Owner,"Dom_N_Base")) == 0)
		end; // No Owner

	if (.@North == 1)
		set $@score_domination_blue, $@score_domination_blue + 1;
	else
		set $@score_domination_red, $@score_domination_red + 1;

	donpcevent "Tierra_DOM::OnValidateScore";
	end;
	
OnSScore:
	if (set(.@South, getvariableofnpc(.Owner,"Dom_S_Base")) == 0)
		end; // No Owner

	if (.@South == 1)
		set $@score_domination_blue, $@score_domination_blue + 1;
	else
		set $@score_domination_red, $@score_domination_red + 1;

	donpcevent "Tierra_DOM::OnValidateScore";
	end;

OnCScore:
	if (set(.@Center, getvariableofnpc(.Owner,"Dom_C_Base")) == 0)
		end; // No Owner

	if (.@Center == 1)
		set $@score_domination_blue, $@score_domination_blue + 1;
	else
		set $@score_domination_red, $@score_domination_red + 1;

	donpcevent "Tierra_DOM::OnValidateScore";
	end;

OnValidateScore:
	if ($@score_domination_blue > 99)
		set $@score_domination_blue, 99;
	if ($@score_domination_red > 99)
		set $@score_domination_red, 99;

	bg_updatescore "bat_a04",$@score_domination_blue,$@score_domination_red; // Update Visual Score

	if ($@score_domination_blue < 99 && $@score_domination_red < 99)
		end; // No winners

OnMatchEnd:
	stopnpctimer "Dom_N_Base";
	stopnpctimer "Dom_S_Base";
	stopnpctimer "Dom_C_Base";
	donpcevent "#guiDOM_respawn::OnBGStop";
	donpcevent "#croDOM_respawn::OnBGStop";
	set $@BG17, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	.@reward_win = 5;
	.@reward_lose = 3;

	// Tie
	if ($@score_domination_blue >= 99 && $@score_domination_red >= 99) {
		callfunc "BG_get_Rewards",$@BG17_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG17_Team2,7828,1;
		mapannounce "bat_a04","The battle is over. This is a Tie...!",1,0x4169E1;
	}
	// Team 1 Won
	else if ($@score_domination_blue >= 99) {
		callfunc "BG_get_Rewards",$@BG17_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG17_Team2,7828,1;
		mapannounce "bat_a04","The " + bg_get_data($@BG17_Team1,2) + " army has won the Battle of Tierra Domination!",1,bg_get_data($@BG17_Team1,4);
	}
	// Team 2 Won
	else if ($@score_domination_red >= 99) {
		callfunc "BG_get_Rewards",$@BG17_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG17_Team2,7828,5;
		mapannounce "bat_a04","The " + bg_get_data($@BG17_Team2,2) + " army has won the Battle of Tierra Domination!",1,bg_get_data($@BG17_Team2,4);
	}
	// =======================================================
	set $@score_domination_blue, 0;
	set $@score_domination_red, 0;
	sleep 5000;
	bg_updatescore "bat_a04",$@score_domination_blue,$@score_domination_red;
	$@BG17 = 0;
	mapwarp "bat_a04","bat_room",154,150;
	if($@BG17_Team1) { bg_destroy $@BG17_Team1; $@BG17_Team1 = 0; }
	if($@BG17_Team2) { bg_destroy $@BG17_Team2; $@BG17_Team2 = 0; }
	bg_unbook "bat_a04";
	end;
}

// ===========================================================
// Battleground Bases
// ===========================================================
bat_a04,173,345,0	script	North Base::Dom_N_Base	1911,{
	end;

OnBase:
	set .Owner, 0;
	set .Balance, 0;
	set .Tick, 0;
	setnpcdisplay "Dom_N_Base","North Base",1911;
	initnpctimer;
	end;

OnTimer1000:
	stopnpctimer;
	if ($@BG17 != 1)
		end;

	set .@Team1Count, bg_getareausers($@BG17_Team1,"bat_a04",136,329,186,361);
	set .@Team2Count, bg_getareausers($@BG17_Team2,"bat_a04",136,329,186,361);
	set .Balance, .Balance + set(.@Balance, .@Team1Count - .@Team2Count);

	if (.Balance < -50)
		set .Balance, -50;
	else if (.Balance > 50)
		set .Balance, 50;

	switch(.Owner) {
		case 0:
			if (.Balance == 50) {
				set .Balance, 50;
				set .Owner, 1; // Team 1
				setnpcdisplay "Dom_N_Base",bg_get_data($@BG17_Team1,2) + " Base",1912;
				mapannounce "bat_a04","North Base captured by " + bg_get_data($@BG17_Team1,2),1,bg_get_data($@BG17_Team1,4);
				//bg_rankpoints_area $@BG17_Team1,"bat_a04",136,329,186,361,"dom_bases",1;
				donpcevent "GDomNBW::OnEnable";
			}
			else if (.Balance == -50) {
				set .Balance, -50;
				set .Owner, 2; // Team 2
				setnpcdisplay "Dom_N_Base",bg_get_data($@BG17_Team2,2) + " Base",1913;
				mapannounce "bat_a04","North Base captured by " + bg_get_data($@BG17_Team2,2),1,bg_get_data($@BG17_Team2,4);
				//bg_rankpoints_area $@BG17_Team2,"bat_a04",136,329,186,361,"dom_bases",1;
				donpcevent "CDomNBW::OnEnable";
			}
			break;
		case 1:
			if (.Balance <= 0) { // Team 1 lost Control
				set .Owner, 0;
				setnpcdisplay "Dom_N_Base","North Base",1911;
				mapannounce "bat_a04",bg_get_data($@BG17_Team1,2) + " lost control of the North Base",1,bg_get_data($@BG17_Team1,4);
				donpcevent "GDomNBW::OnDisable";
			}
			break;
		case 2:
			if (.Balance >= 0) { // Team 2 lost Control
				set .Owner, 0;
				setnpcdisplay "Dom_N_Base","North Base",1911;
				mapannounce "bat_a04",bg_get_data($@BG17_Team2,2) + " lost control of the North Base",1,bg_get_data($@BG17_Team2,4);
				donpcevent "CDomNBW::OnDisable";
			}
			break;
	}

	if (.@Balance > 0)
		specialeffect 236;
	else if (.@Balance < 0)
		specialeffect 225;
	else if (.Owner == 1)
		specialeffect 236;
	else if (.Owner == 2)
		specialeffect 225;
	else
		specialeffect 223;
	
	switch(.Owner) {
		case 0: viewpointmap "bat_a04",1,173,345,1,0xFFFFFF; break;
		case 1: viewpointmap "bat_a04",1,173,345,1,bg_get_data($@BG17_Team1,4); break;
		case 2: viewpointmap "bat_a04",1,173,345,1,bg_get_data($@BG17_Team2,4); break;
	}
	
	if (.Owner == 0)
		set .Tick, 0;
	else if (set(.Tick, .Tick + 1) == 7) {
		donpcevent "Tierra_DOM::OnNScore";
		set .Tick, 0;
	}

	initnpctimer;
	end;
}

bat_a04,273,204,0	script	Center Base::Dom_C_Base	1911,{
	end;

OnBase:
	set .Owner, 0;
	set .Balance, 0;
	set .Tick, 0;
	setnpcdisplay "Dom_C_Base","Center Base",1911;
	initnpctimer;
	end;

OnTimer1000:
	stopnpctimer;
	if ($@BG17 != 1 )
		end;

	set .@Team1Count, bg_getareausers($@BG17_Team1,"bat_a04",260,194,287,213);
	set .@Team2Count, bg_getareausers($@BG17_Team2,"bat_a04",260,194,287,213);
	set .Balance, .Balance + set(.@Balance, .@Team1Count - .@Team2Count);

	if (.Balance < -50)
		set .Balance, -50;
	else if (.Balance > 50)
		set .Balance, 50;

	switch(.Owner) {
		case 0:
			if (.Balance == 50) {
				set .Balance, 50;
				set .Owner, 1; // Team 1
				setnpcdisplay "Dom_C_Base",bg_get_data($@BG17_Team1,2) + " Base",1912;
				mapannounce "bat_a04","Center Base captured by " + bg_get_data($@BG17_Team1,2),1,bg_get_data($@BG17_Team1,4);
				//bg_rankpoints_area $@BG17_Team1,"bat_a04",260,194,287,213,"dom_bases",1;
				donpcevent "GDomCBW::OnEnable";
			}
			else if (.Balance == -50) {
				set .Balance, -50;
				set .Owner, 2; // Team 2
				setnpcdisplay "Dom_C_Base",bg_get_data($@BG17_Team2,2) + " Base",1913;
				mapannounce "bat_a04","Center Base captured by " + bg_get_data($@BG17_Team2,2),1,bg_get_data($@BG17_Team2,4);
				//bg_rankpoints_area $@BG17_Team2,"bat_a04",260,194,287,213,"dom_bases",1;
				donpcevent "CDomCBW::OnEnable";
			}
			break;
		case 1:
			if (.Balance <= 0) { // Team 1 lost Control
				set .Owner, 0;
				setnpcdisplay "Dom_C_Base","Center Base",1911;
				mapannounce "bat_a04",bg_get_data($@BG17_Team1,2) + " lost control of the Center Base",1,bg_get_data($@BG17_Team1,4);
				donpcevent "GDomCBW::OnDisable";
			}
			break;
		case 2:
			if (.Balance >= 0) { // Team 2 lost Control
				set .Owner, 0;
				setnpcdisplay "Dom_C_Base","Center Base",1911;
				mapannounce "bat_a04",bg_get_data($@BG17_Team2,2) + " lost control of the Center Base",1,bg_get_data($@BG17_Team2,4);
				donpcevent "CDomCBW::OnDisable";
			}
			break;
	}

	if (.@Balance > 0)
		specialeffect 236;
	else if (.@Balance < 0)
		specialeffect 225;
	else if (.Owner == 1)
		specialeffect 236;
	else if (.Owner == 2)
		specialeffect 225;
	else
		specialeffect 223;
	
	switch(.Owner) {
		case 0: viewpointmap "bat_a04",1,273,204,3,0xFFFFFF; break;
		case 1: viewpointmap "bat_a04",1,273,204,3,bg_get_data($@BG17_Team1,4); break;
		case 2: viewpointmap "bat_a04",1,273,204,3,bg_get_data($@BG17_Team2,4); break;
	}
	
	if (.Owner == 0)
		set .Tick, 0;
	else if (set(.Tick, .Tick + 1) == 7) {
		donpcevent "Tierra_DOM::OnCScore";
		set .Tick, 0;
	}

	initnpctimer;
	end;
}

bat_a04,164,50,0	script	South Base::Dom_S_Base	1911,{
	end;

OnBase:
	set .Owner, 0;
	set .Balance, 0;
	set .Tick, 0;
	setnpcdisplay "Dom_S_Base","South Base",1911;
	initnpctimer;
	end;

OnTimer1000:
	stopnpctimer;
	if ($@BG17 != 1 )
		end;

	set .@Team1Count, bg_getareausers($@BG17_Team1,"bat_a04",129,34,175,65);
	set .@Team2Count, bg_getareausers($@BG17_Team2,"bat_a04",129,34,175,65);
	set .Balance, .Balance + set(.@Balance, .@Team1Count - .@Team2Count);

	if (.Balance < -50)
		set .Balance, -50;
	else if (.Balance > 50)
		set .Balance, 50;

	switch(.Owner) {
		case 0:
			if (.Balance == 50) {
				set .Balance, 50;
				set .Owner, 1; // Team 1
				setnpcdisplay "Dom_S_Base",bg_get_data($@BG17_Team1,2) + " Base",1912;
				mapannounce "bat_a04","South Base captured by " + bg_get_data($@BG17_Team1,2),1,bg_get_data($@BG17_Team1,4);
				//bg_rankpoints_area $@BG17_Team1,"bat_a04",129,34,175,65,"dom_bases",1;
				donpcevent "GDomSBW::OnEnable";
			}
			else if (.Balance == -50) {
				set .Balance, -50;
				set .Owner, 2; // Team 2
				setnpcdisplay "Dom_S_Base",bg_get_data($@BG17_Team2,2) + " Base",1913;
				mapannounce "bat_a04","South Base captured by " + bg_get_data($@BG17_Team2,2),1,bg_get_data($@BG17_Team2,4);
				//bg_rankpoints_area $@BG17_Team2,"bat_a04",129,34,175,65,"dom_bases",1;
				donpcevent "CDomSBW::OnEnable";
			}
			break;
		case 1:
			if (.Balance <= 0) { // Team 1 lost Control
				set .Owner, 0;
				setnpcdisplay "Dom_S_Base","North Base",1911;
				mapannounce "bat_a04",bg_get_data($@BG17_Team1,2) + " lost control of the South Base",1,bg_get_data($@BG17_Team1,4);
				donpcevent "GDomSBW::OnDisable";
			}
			break;
		case 2:
			if (.Balance >= 0) { // Team 2 lost Control
				set .Owner, 0;
				setnpcdisplay "Dom_S_Base","North Base",1911;
				mapannounce "bat_a04",bg_get_data($@BG17_Team2,2) + " lost control of the South Base",1,bg_get_data($@BG17_Team2,4);
				donpcevent "CDomSBW::OnDisable";
			}
			break;
	}

	if (.@Balance > 0)
		specialeffect 236;
	else if (.@Balance < 0)
		specialeffect 225;
	else if (.Owner == 1)
		specialeffect 236;
	else if (.Owner == 2)
		specialeffect 225;
	else
		specialeffect 223;
	
	switch(.Owner) {
		case 0: viewpointmap "bat_a04",1,164,50,2,0xFFFFFF; break;
		case 1: viewpointmap "bat_a04",1,164,50,2,bg_get_data($@BG17_Team1,4); break;
		case 2: viewpointmap "bat_a04",1,164,50,2,bg_get_data($@BG17_Team2,4); break;
	}

	if (.Owner == 0)
		set .Tick, 0;
	else if (set(.Tick, .Tick + 1) == 7) {
		donpcevent "Tierra_DOM::OnSScore";
		set .Tick, 0;
	}

	initnpctimer;
	end;
}

// ===========================================================
// Healer
// ===========================================================
bat_a04,53,377,3	duplicate(BGHeal)	Therapist in battle#DOM2	4_F_SISTER
bat_a04,45,18,3	duplicate(BGHeal)	Therapist in battle#DOM1	4_F_SISTER

// ===========================================================
// Respawn
// ===========================================================
bat_a04,46,377,0	script	North Base Warp::GDomNBW	1_SHADOW_NPC,{
	end;

OnEnable:
	waitingroom "Join To Warp",20;
	end;

OnDisable:
	delwaitingroom;
	end;

OnWarp:
	warpwaitingpc "bat_a04",115,346,20;
	end;
}

bat_a04,46,370,0	script	Center Base Warp::GDomCBW	1_SHADOW_NPC,{
	end;

OnEnable:
	waitingroom "Join To Warp",20;
	end;

OnDisable:
	delwaitingroom;
	end;

OnWarp:
	warpwaitingpc "bat_a04",285,226,20;
	end;
}

bat_a04,53,370,0	script	South Base Warp::GDomSBW	1_SHADOW_NPC,{
	end;

OnEnable:
	waitingroom "Join To Warp",20;
	end;

OnDisable:
	delwaitingroom;
	end;

OnWarp:
	warpwaitingpc "bat_a04",106,48,20;
	end;
}

bat_a04,38,19,0	script	North Base Warp::CDomNBW	1_SHADOW_NPC,{
	end;

OnEnable:
	waitingroom "Join To Warp",20;
	end;

OnDisable:
	delwaitingroom;
	end;

OnWarp:
	warpwaitingpc "bat_a04",115,346,20;
	end;
}

bat_a04,38,12,0	script	Center Base Warp::CDomCBW	1_SHADOW_NPC,{
	end;

OnEnable:
	waitingroom "Join To Warp",20;
	end;

OnDisable:
	delwaitingroom;
	end;
OnWarp:
	warpwaitingpc "bat_a04",260,183,20;
	end;
}

bat_a04,45,12,0	script	South Base Warp::CDomSBW	1_SHADOW_NPC,{
	end;

OnEnable:
	waitingroom "Join To Warp",20;
	end;

OnDisable:
	delwaitingroom;
	end;

OnWarp:
	warpwaitingpc "bat_a04",106,48,20;
	end;
}

bat_a04,50,374,0	script	#guiDOM_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "bat_a04",46,370,54,378,100,100;
	donpcevent "GDomNBW::OnWarp";
	donpcevent "GDomCBW::OnWarp";
	donpcevent "GDomSBW::OnWarp";
	areawarp "bat_a04",46,370,54,378,"bat_a04",353,344;
	initnpctimer;
	end;
}

bat_a04,42,16,0	script	#croDOM_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "bat_a04",38,12,47,21,100,100;
	donpcevent "GDomNBW::OnWarp";
	donpcevent "GDomCBW::OnWarp";
	donpcevent "GDomSBW::OnWarp";
	areawarp "bat_a04",38,12,47,21,"bat_a04",353,52;
	initnpctimer;
	end;
}

// ===========================================================
// Map Flags
// ===========================================================
bat_a04	mapflag	battleground	2
bat_a04	mapflag	nomemo
bat_a04	mapflag	nosave	SavePoint
bat_a04	mapflag	noteleport
bat_a04	mapflag	nowarp
bat_a04	mapflag	nowarpto
bat_a04	mapflag	noreturn
bat_a04	mapflag	nobranch
bat_a04	mapflag	nopenalty
bat_a04	mapflag	noecall
//bat_a04	mapflag	bg_consume
