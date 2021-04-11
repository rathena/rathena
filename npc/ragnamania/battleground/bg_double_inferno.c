//===== rAthena Script =======================================
//= Battleground Extended + : Double Inferno
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
-	script	Tierra_TI	-1,{
	end;

OnInit:
	setwall "region_8",27,52,8,0,0,"ti_wall_a";
	setwall "region_8",20,52,8,0,0,"ti_wall_a2";
	setwall "region_8",72,46,8,0,0,"ti_wall_b";
	setwall "region_8",79,46,8,0,0,"ti_wall_b2";
	end;

OnTeam1Quit:
OnTeam2Quit:
OnTeam3Quit:
	bg_desert;
	end;
OnTeam1Active:
	warp "region_8",29,49;
	end;
OnTeam2Active:
	warp "region_8",70,50;
	end;

OnTeam1Die:
OnTeam2Die:
OnTeam3Die:
	if ($@BG19 == 1) {
		// Killed Position
		getmapxy .@m$, .@x, .@y;
		set .@Bid, getcharid(4);
		// Current Skulls
		set .@id8965,countitem(8965);
		set .@id8966,countitem(8966);
		set .@id8967,countitem(8967);
		// Remove Skulls
		if (.@id8965) delitem 8965,.@id8965;
		if (.@id8966) delitem 8966,.@id8966;
		if (.@id8967) delitem 8967,.@id8967;
		// Drop Skulls
		if (.@id8965) makeitem 8965,.@id8965,"region_8",.@x,.@y;
		if (.@id8966) makeitem 8966,.@id8966,"region_8",.@x,.@y;
		if (.@id8967) makeitem 8967,.@id8967,"region_8",.@x,.@y;
		// Drop a New Skull
		if (.@Bid == $@BG19_Team1)
			makeitem 8965,1,"region_8",.@x,.@y;
		else if (.@Bid == $@BG19_Team2)
			makeitem 8966,1,"region_8",.@x,.@y;
		else if (.@Bid == $@BG19_Team3)
			makeitem 8967,1,"region_8",.@x,.@y;
	}
	end;

OnReady:
	set $@BG19,1;
	initnpctimer;
	$@score_inferno_red = 0;
	$@score_inferno_blue = 0;
	donpceventall "OnEmblemTI";
	bg_warp $@BG19_Team1,"region_8",29,49;
	bg_warp $@BG19_Team2,"region_8",70,50;
	bg_warp $@BG19_Team3,"region_8",49,70;
	donpcevent "#gti_respawn::OnBGStart";
	donpcevent "#cti_respawn::OnBGStart";
	donpcevent "#tti_respawn::OnBGStart";
	sleep 2000;
	bg_team_reveal $@BG19_Team1;
	bg_team_reveal $@BG19_Team2;
	end;

OnValidateScore:
	if (!$@BG19)
		end;
	if($@score_inferno_red >= 80 || $@score_inferno_blue >= 80)
		donpcevent "Tierra_TI::OnMatchEnd";
	end;

OnTimer600000:
	mapannounce "region_8","The Battle will ends in 5 minutes!!",1,0x696969;
	end;

OnTimer840000:
	mapannounce "region_8","The Battle will ends in 1 minute!!",1,0x696969;
	end;

OnTimer900000:
OnMatchEnd:
	stopnpctimer;
	cleanmap "region_8"; // Removes all ground items
	donpcevent "#gti_respawn::OnBGStop";
	donpcevent "#cti_respawn::OnBGStop";
	donpcevent "#tti_respawn::OnBGStop";
	set $@BG19, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	.@reward_win = 5;
	.@reward_lose = 3;
	if ($@score_inferno_blue > $@score_inferno_red) {
		callfunc "BG_get_Rewards",$@BG19_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG19_Team2,7828,1;
		mapannounce "region_8","The " + bg_get_data($@BG19_Team1,2) + " has won the Battle of Tierra Inferno!",1,bg_get_data($@BG19_Team1,4);
	} else if ($@score_inferno_red > $@score_inferno_blue) {
		callfunc "BG_get_Rewards",$@BG19_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG19_Team2,7828,5;
		mapannounce "region_8","The " + bg_get_data($@BG19_Team2,2) + " has won the Battle of Tierra Inferno!",1,bg_get_data($@BG19_Team2,4);
	} else {
		callfunc "BG_get_Rewards",$@BG19_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG19_Team2,7828,1;
		mapannounce "region_8","DRAW !!",1;
	}
	// =======================================================
	sleep 5000;
	$@BG19 = 0;
	mapwarp "region_8","bat_room",154,150;
	$@score_inferno_red = 0;
	$@score_inferno_blue = 0;
	if($@BG19_Team1) { bg_destroy $@BG19_Team1; $@BG19_Team1 = 0; }
	if($@BG19_Team2) { bg_destroy $@BG19_Team2; $@BG19_Team2 = 0; }
	bg_unbook "region_8";
	end;
}

region_8,29,45,0	script	Sacrifice Totem::BlueTotem	1277,2,2,{ unitwalk getcharid(3),29,45; end; OnTouch: callfunc "totembg",0,1,8965,8966; }
region_8,29,54,0	script	Sacrifice Totem::BlueTotem2	1277,2,2,{ unitwalk getcharid(3),29,54; end; OnTouch: callfunc "totembg",0,1,8965,8966; }
region_8,70,54,0	script	Sacrifice Totem::RedTotem	1277,2,2,{ unitwalk getcharid(3),70,54; end; OnTouch: callfunc "totembg",1,2,8966,8965; }
region_8,70,45,0	script	Sacrifice Totem::RedTotem2	1277,2,2,{ unitwalk getcharid(3),70,45; end; OnTouch: callfunc "totembg",1,2,8966,8965; }


function	script	totembg	{
	if (!$@BG19)
		end;
	if (bg_teamid() != getarg(0))
		end;

	set .@Points, 0;
	delitem getarg(2),countitem(getarg(2));

	if (set(.@n,countitem(getarg(3))) > 0) { // Team 2 Skulls
		delitem getarg(3),.@n;
		set .@Points, .@Points + .@n;
	}

	if (.@Points > 0) {
		emotion ET_BEST;
		specialeffect 622;
		if(bg_teamid()) { $@score_inferno_red += .@Points; .@Score = $@score_inferno_red; }
		else { $@score_inferno_blue += .@Points; .@Score = $@score_inferno_blue; }
		bg_updatescore strnpcinfo(4),$@score_inferno_blue,$@score_inferno_red;
		mapannounce "region_8",bg_get_data(getd("$@BG19_Team"+getarg(1)+""),2) + " : " + .@Points + " slaughtered skulls by " + strcharinfo(0) + " [" + .@Score + "/80]",1,bg_get_data(getd("$@BG19_Team"+getarg(1)+""),4);
		donpcevent "Tierra_TI::OnValidateScore";
	}
	end;
}

// ===========================================================
// MapFlags
// ===========================================================
region_8	mapflag	bg_topscore	80
region_8	mapflag	battleground	2
region_8	mapflag	nomemo
region_8	mapflag	nosave	SavePoint
region_8	mapflag	noteleport
region_8	mapflag	nowarp
region_8	mapflag	nowarpto
region_8	mapflag	noreturn
region_8	mapflag	nobranch
region_8	mapflag	nopenalty
// region_8	mapflag	bg_consume

// ===========================================================
// Flags
// ===========================================================
region_8,75,47,2	script	Croix Base::TIF_Croix	1_FLAG_EAGLE,{ end; }
region_8,75,52,2	duplicate(TIF_Croix)	Bravo Base#ti_2	1_FLAG_EAGLE
region_8,24,47,6	script	Guillaume Base::TIF_Guillaume	1_FLAG_LION,{ end; }
region_8,24,52,6	duplicate(TIF_Guillaume)	Alpha Base#ti_2	1_FLAG_LION

// ===========================================================
// Heal
// ===========================================================
region_8,7,52,5	duplicate(BGHeal)	Therapist in battle#ti_1	4_F_SISTER,
region_8,52,92,3	duplicate(BGHeal)	Therapist in battle#ti_2	4_F_SISTER
region_8,92,52,3	duplicate(BGHeal)	Therapist in battle#ti_3	4_F_SISTER

// ===========================================================
// Respawn
// ===========================================================
region_8,49,89,0	script	#tti_respawn	HIDDEN_WARP_NPC,{
	end;

OnBGStart:
	initnpctimer;
	end;

OnBGStop:
	stopnpctimer;
	end;

OnTimer19000:
	specialeffect 83;
	end;

OnTimer20000:
	areapercentheal "region_8",46,86,53,93,100,100;
	areawarp "region_8",46,86,53,93,"region_8",46,70,53,72;
	initnpctimer;
	end;
}

region_8,10,49,0	script	#gti_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "region_8",6,46,13,53,100,100;
	areawarp "region_8",6,46,13,53,"region_8",27,46,29,53;
	initnpctimer;
	end;
}

region_8,89,49,0	script	#cti_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "region_8",86,46,93,53,100,100;
	areawarp "region_8",86,46,93,53,"region_8",70,46,72,53;
	initnpctimer;
	end;
}
