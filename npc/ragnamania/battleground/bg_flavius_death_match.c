//===== rAthena Script =======================================
//= Battleground Extended + : Team Death Match
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
-	script	Flavius_TD	FAKE_NPC,{
	end;

OnTeam1Quit:
	bg_desert;
OnTeam1Die:
	if ($@BG14 == 1) {
		if (killerrid == 0) end;
		$@score_deathmatch_blue -= 1;
		donpcevent "Flavius_TD::OnValidateScore";
	}
	end;

OnTeam1Active:
	warp "bat_b03",390,10;
	end;
OnTeam2Active:
	warp "bat_b03",10,290;
	end;

OnTeam2Quit:
	bg_desert;
OnTeam2Die:
	if ($@BG14 == 1) {
		if (killerrid == 0) end;
		$@score_deathmatch_red -= 1;
		donpcevent "Flavius_TD::OnValidateScore";
	}
	end;

OnReady:
	//if ($@BG14)
	//	end;

	set $@BG14,1;
	initnpctimer;
	set $@score_deathmatch_blue, 50;
	set $@score_deathmatch_red, 50;
	bg_updatescore "bat_b03",$@score_deathmatch_blue,$@score_deathmatch_red;
	sleep 2100;
	bg_warp $@BG14_Team1,"bat_b03",328,150;
	bg_warp $@BG14_Team2,"bat_b03",62,150;
	sleep 2100;
	bg_team_reveal $@BG14_Team1;
	bg_team_reveal $@BG14_Team2;
	donpcevent "#guitd_respawn::OnBGStart";
	donpcevent "#crotd_respawn::OnBGStart";
	end;

OnValidateScore:
	if ($@BG14 != 1)
		end;
	bg_updatescore "bat_b03",$@score_deathmatch_blue,$@score_deathmatch_red;
	set .@Team1Count, bg_get_data($@BG14_Team1, 0);
	set .@Team2Count, bg_get_data($@BG14_Team2, 0);
	.@reward_win = 5;
	.@reward_lose = 3;
	// Team 1 Won
	if ($@score_deathmatch_red <= 0) {
		callfunc "BG_get_Rewards",$@BG14_Team1,7829,5;
		callfunc "BG_get_Rewards",$@BG14_Team2,7829,1;
		mapannounce "bat_b03","The " + bg_get_data($@BG14_Team1,2) + " army has won the Battle of Flavius TD!",1,bg_get_data($@BG14_Team1,4);
	}
	// Team 2 Won
	else if ($@score_deathmatch_blue <= 0) {
		callfunc "BG_get_Rewards",$@BG14_Team1,7829,1;
		callfunc "BG_get_Rewards",$@BG14_Team2,7829,5;
		mapannounce "bat_b03","The " + bg_get_data($@BG14_Team2,2) + " army has won the Battle of Flavius TD!",1,bg_get_data($@BG14_Team2,4);
	}
	// All Team 2 Players quit
	else if (.@Team2Count == 0) {
		callfunc "BG_get_Rewards",$@BG14_Team1,7829,1;
		mapannounce "bat_b03","The " + bg_get_data($@BG14_Team1,2) + " army has won the Battle of Flavius TD!",1,bg_get_data($@BG14_Team1,4);
	}
	// All Team 1 Players quit
	else if (.@Team1Count == 0) {
		callfunc "BG_get_Rewards",$@BG14_Team2,7829,1;
		mapannounce "bat_b03","The " + bg_get_data($@BG14_Team2,2) + " army has won the Battle of Flavius TD!",1,bg_get_data($@BG14_Team2,4);
	}
	else end;
	donpcevent "Flavius_TD::OnMatchEnd";
	end;

OnTimer600000:
	mapannounce "bat_b03","The Battle will ends in 5 minutes!!",1,0x808000;
	end;

OnTimer840000:
	mapannounce "bat_b03","The Battle will ends in 1 minute!!",1,0x808000;
	end;

OnTimer900000:
	.@reward_win = 5;
	.@reward_lose = 3;
	// Team 1 Won
	if ($@score_deathmatch_blue > $@score_deathmatch_red) {
		callfunc "BG_get_Rewards",$@BG14_Team1,7829,5;
		callfunc "BG_get_Rewards",$@BG14_Team2,7829,1;
		mapannounce "bat_b03","The " + bg_get_data($@BG14_Team1,2) + " army has won the Battle of Flavius TD!",1,bg_get_data($@BG14_Team1,4);
	}
	// Team 2 Won
	else if ($@score_deathmatch_blue < $@score_deathmatch_red) {
		callfunc "BG_get_Rewards",$@BG14_Team1,7829,1;
		callfunc "BG_get_Rewards",$@BG14_Team2,7829,5;
		mapannounce "bat_b03","The " + bg_get_data($@BG14_Team2,2) + " army has won the Battle of Flavius TD!",1,bg_get_data($@BG14_Team2,4);
	}
	else {
		callfunc "BG_get_Rewards",$@BG14_Team1,7829,1;
		callfunc "BG_get_Rewards",$@BG14_Team2,7829,1;
		mapannounce "bat_b03","The battle is over. This is a Tie...!",1,0x808000;
	}
	donpcevent "Flavius_TD::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	donpcevent "#guitd_respawn::OnBGStop";
	donpcevent "#crotd_respawn::OnBGStop";
	set $@BG14, 2;
	set $@score_deathmatch_blue, 50;
	set $@score_deathmatch_red, 50;
	sleep 5000;
	bg_updatescore "bat_b03",$@score_deathmatch_blue,$@score_deathmatch_red;
	$@BG14 = 0;
	mapwarp "bat_b03","bat_room",154,150;
	if($@BG14_Team1) { bg_destroy $@BG14_Team1; $@BG14_Team1 = 0; }
	if($@BG14_Team2) { bg_destroy $@BG14_Team2; $@BG14_Team2 = 0; }
	bg_unbook "bat_b03";
	end;
}

// ===========================================================
// Heal
// ===========================================================
bat_b03,390,13,5	duplicate(BGHeal)	Therapist in battle#td1	4_F_SISTER
bat_b03,10,293,5	duplicate(BGHeal)	Therapist in battle#td2	4_F_SISTER

// ===========================================================
// Respawn
// ===========================================================
bat_b03,390,10,0	script	#guitd_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "bat_b03",382,2,397,17,100,100;
	areawarp "bat_b03",382,2,397,17,"bat_b03",306,138,327,161;
	initnpctimer;
	end;
}

bat_b03,10,290,0	script	#crotd_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "bat_b03",2,282,17,297,100,100;
	areawarp "bat_b03",2,282,17,297,"bat_b03",72,138,93,161;
	initnpctimer;
	end;
}

// ===========================================================
// Flags
// ===========================================================
bat_b03,304,231,1	duplicate(Base Flag#bg)	Alpha Base#td_1	1_FLAG_LION
bat_b03,319,231,1	duplicate(Base Flag#bg)	Alpha Base#td_2	1_FLAG_LION
bat_b03,304,218,1	duplicate(Base Flag#bg)	Alpha Base#td_3	1_FLAG_LION
bat_b03,319,218,1	duplicate(Base Flag#bg)	Alpha Base#td_4	1_FLAG_LION
bat_b03,304,231,1	duplicate(Base Flag#bg)	Alpha Base#td_5	1_FLAG_LION
bat_b03,304,231,1	duplicate(Base Flag#bg)	Alpha Base#td_6	1_FLAG_LION
bat_b03,335,142,1	duplicate(Base Flag#bg)	Alpha Base#td_7	1_FLAG_LION
bat_b03,335,157,1	duplicate(Base Flag#bg)	Alpha Base#td_8	1_FLAG_LION
bat_b03,390,16,1	duplicate(Base Flag#bg)	Alpha Base#td_9	1_FLAG_LION
bat_b03,292,163,1	duplicate(Base Flag#bg)	Alpha Base#td_10	1_FLAG_LION
bat_b03,292,136,1	duplicate(Base Flag#bg)	Alpha Base#td_11	1_FLAG_LION
bat_b03,241,185,1	duplicate(Base Flag#bg)	Alpha Base#td_12	1_FLAG_LION
bat_b03,247,179,1	duplicate(Base Flag#bg)	Alpha Base#td_13	1_FLAG_LION

bat_b03,96,81,1	duplicate(Base Flag#bg)	Omega Base#td_1	1_FLAG_EAGLE
bat_b03,96,68,1	duplicate(Base Flag#bg)	Omega Base#td_2	1_FLAG_EAGLE
bat_b03,79,81,1	duplicate(Base Flag#bg)	Omega Base#td_3	1_FLAG_EAGLE
bat_b03,79,68,1	duplicate(Base Flag#bg)	Omega Base#td_4	1_FLAG_EAGLE
bat_b03,96,81,1	duplicate(Base Flag#bg)	Omega Base#td_5	1_FLAG_EAGLE
bat_b03,96,81,1	duplicate(Base Flag#bg)	Omega Base#td_6	1_FLAG_EAGLE
bat_b03,59,164,1	duplicate(Base Flag#bg)	Omega Base#td_7	1_FLAG_EAGLE
bat_b03,59,137,1	duplicate(Base Flag#bg)	Omega Base#td_8	1_FLAG_EAGLE
bat_b03,10,296,1	duplicate(Base Flag#bg)	Omega Base#td_9	1_FLAG_EAGLE
bat_b03,110,162,1	duplicate(Base Flag#bg)	Omega Base#td_10	1_FLAG_EAGLE
bat_b03,110,137,1	duplicate(Base Flag#bg)	Omega Base#td_11	1_FLAG_EAGLE
bat_b03,152,120,1	duplicate(Base Flag#bg)	Omega Base#td_12	1_FLAG_EAGLE
bat_b03,158,114,1	duplicate(Base Flag#bg)	Omega Base#td_13	1_FLAG_EAGLE

// ===========================================================
// Map Flags
// ===========================================================
bat_b03	mapflag	battleground	2
bat_b03	mapflag	nomemo
bat_b03	mapflag	nosave	SavePoint
bat_b03	mapflag	noteleport
bat_b03	mapflag	nowarp
bat_b03	mapflag	nowarpto
bat_b03	mapflag	noreturn
bat_b03	mapflag	nobranch
bat_b03	mapflag	nopenalty
// bat_b03	mapflag	bg_consume
