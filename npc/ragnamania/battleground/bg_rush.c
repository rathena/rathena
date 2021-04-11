//===== rAthena Script =======================================
//= Battleground Extended + : Rush
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
-	script	Rush	FAKE_NPC,{
	end;

OnInit:
	setwall "rush_cas01",198,228,10,6,0,"Rush_wall_a";
	setcell "rush_cas01",198,224,207,233,cell_basilica,1;
	setwall "rush_cas02",142,48,10,6,0,"Rush_wall_b";
	setcell "rush_cas02",142,44,151,53,cell_basilica,1;
	setwall "rush_cas03",62,8,10,0,0,"Rush_wall_c";
	setcell "rush_cas03",56,6,69,17,cell_basilica,1;
	setwall "rush_cas04",266,290,10,6,0,"Rush_wall_d";
	setcell "rush_cas04",266,286,275,295,cell_basilica,1;
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;

OnTeam1Active:
OnTeam2Active:
	warp .Castle$,43,229;
	end;

OnReady:
	set .Castle$,"rush_cas01";
	set .GX,202; set .GY,230; set .CX,202; set .CY,226;

	set $@BG15,1;
	initnpctimer;
	initnpctimer "Rush_Respawn";
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	set .Defender, 0; // No Defender
	// Emperium =====================
	bg_monster 0,.Castle$,158,174,"Emperium",20402,"Rush::OnEmperium";

	// ======================================
	sleep 2100;
	bg_warp $@BG15_Team1,.Castle$,43,229;
	bg_warp $@BG15_Team2,.Castle$,43,229;
	.x = 43; .y = 229;
	sleep 4000;
	mapannounce .Castle$,"Move on warriors!! Let's capture this Castle!!",1,0xDDA0DD;
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Rush will ends in 6 minutes",1,0xDDA0DD;
	end;

OnTimer360000:
	mapannounce .Castle$,"Battle of Rush will ends in 1 minute",1,0xDDA0DD;
	end;

OnEmperium:
	if ($@BG15 != 1)
		end;

	if (.Defender == 0) { // First Capture
		initnpctimer;
		set .Defender,getcharid(4);
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle captured, now prepare to Defend it!!",1,bg_get_data(.Defender,4);

		if (.Defender == $@BG15_Team1) {
			set $@score_rush_blue,1;
			set .Attacker,$@BG15_Team2;
		} else {
			set $@score_rush_red,1;
			set .Attacker,$@BG15_Team1;
		}

		bg_warp .Attacker,.Castle$,.x,.y; // To Cementery and Wait
		bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;

		// Emperium =====================
		bg_monster .Defender,.Castle$,158,174,"Emperium",20402,"Rush::OnEmperium";

		// ======================================
		initnpctimer "Rush_Respawn";

		sleep 5000;
		mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Capture the Castle, it's our last chance to Win!!",1,bg_get_data(.Attacker,4);
		end;
	}

	// Second Capture
	mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Castle captured, we won the Battle!!",1,bg_get_data(.Attacker,4);
	if (.Defender == $@BG15_Team1)
		set $@score_rush_red,2;
	else
		set $@score_rush_blue,2;

	donpcevent "Rush::OnMatchEnd";
	end;

OnTimer420000:
	if (.Defender == 0)
		mapannounce .Castle$,"Castle captured Fail. No winners",1,0xDDA0DD;
	else {
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle protected, we won the Battle!!",1,bg_get_data(.Defender,4);
		if (.Defender == $@BG15_Team1)
			set $@score_rush_blue,3;
		else
			set $@score_rush_red,3;
	}

	donpcevent "Rush::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Rush_Respawn";
	killmonster .Castle$,"Rush::OnEmperium";
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	set $@BG15, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	if ($@score_rush_blue > $@score_rush_red) {
		set .@Won,0;
		callfunc "BG_get_Rewards",$@BG15_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG15_Team2,7828,1;
	} else if ($@score_rush_red > $@score_rush_blue) {
		set .@Won,0;
		callfunc "BG_get_Rewards",$@BG15_Team2,7828,5;
		callfunc "BG_get_Rewards",$@BG15_Team1,7828,1;
	} else {
		callfunc "BG_get_Rewards",$@BG15_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG15_Team2,7828,1;
	}
	// =======================================================
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	sleep 5000;
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	$@BG15 = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if($@BG15_Team1) { bg_destroy $@BG15_Team1; $@BG15_Team1 = 0; }
	if($@BG15_Team2) { bg_destroy $@BG15_Team2; $@BG15_Team2 = 0; }
	bg_unbook .Castle$;
	end;
}


-	script	Rush1	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;

OnTeam1Active:
OnTeam2Active:
	warp .Castle$,252,271;
	end;
OnReady:
	if ($@BG15b)
		end;

	set .Castle$,"rush_cas02";
	set .GX,146; set .GY, 50; set .CX,146; set .CY, 46;

	set $@BG15b,1;
	initnpctimer;
	initnpctimer "Rush_Respawn1";
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	set .Defender, 0; // No Defender
	// Emperium =====================
	bg_monster 0,.Castle$,271,29,"Emperium",20402,"Rush1::OnEmperium";

	// ======================================
	sleep 2100;
	bg_warp $@BG15b_Team1,.Castle$,252,271;
	bg_warp $@BG15b_Team2,.Castle$,252,271;
	.x = 252; .y = 271;
	sleep 4000;
	mapannounce .Castle$,"Move on warriors!! Let's capture this Castle!!",1,0xDDA0DD;
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Rush will ends in 6 minutes",1,0xDDA0DD;
	end;

OnTimer360000:
	mapannounce .Castle$,"Battle of Rush will ends in 1 minute",1,0xDDA0DD;
	end;

OnEmperium:
	if ($@BG15 != 1)
		end;

	if (.Defender == 0) { // First Capture
		initnpctimer;
		set .Defender,getcharid(4);
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle captured, now prepare to Defend it!!",1,bg_get_data(.Defender,4);

		if (.Defender == $@BG15b_Team1) {
			set $@score_rush_blue,1;
			set .Attacker,$@BG15b_Team2;
		} else {
			set $@score_rush_red,1;
			set .Attacker,$@BG15b_Team1;
		}

		bg_warp .Attacker,.Castle$,.x,.y; // To Cementery and Wait
		bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;

		// Emperium =====================
		bg_monster .Defender,.Castle$,271,29,"Emperium",20402,"Rush1::OnEmperium";

		// ======================================
		initnpctimer "Rush_Respawn1";

		sleep 5000;
		mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Capture the Castle, it's our last chance to Win!!",1,bg_get_data(.Attacker,4);
		end;
	}

	// Second Capture
	mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Castle captured, we won the Battle!!",1,bg_get_data(.Attacker,4);
	if (.Defender == $@BG15b_Team1)
		set $@score_rush_red,2;
	else
		set $@score_rush_blue,2;

	donpcevent "Rush1::OnMatchEnd";
	end;

OnTimer420000:
	if (.Defender == 0)
		mapannounce .Castle$,"Castle captured Fail. No winners",1,0xDDA0DD;
	else {
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle protected, we won the Battle!!",1,bg_get_data(.Defender,4);
		if (.Defender == $@BG15b_Team1)
			set $@score_rush_blue,3;
		else
			set $@score_rush_red,3;
	}

	donpcevent "Rush1::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Rush_Respawn1";
	killmonster .Castle$,"Rush1::OnEmperium";
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	set $@BG15, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	if ($@score_rush_blue > $@score_rush_red) {
		callfunc "BG_get_Rewards",$@BG15b_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG15b_Team2,7828,1;
	} else if ($@score_rush_red > $@score_rush_blue) {
		callfunc "BG_get_Rewards",$@BG15b_Team2,7828,5;
		callfunc "BG_get_Rewards",$@BG15b_Team1,7828,1;
	} else {
		callfunc "BG_get_Rewards",$@BG15b_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG15b_Team2,7828,1;
	}
	// =======================================================
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	sleep 5000;
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	$@BG15b = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if($@BG15b_Team1) { bg_destroy $@BG15b_Team1; $@BG15b_Team1 = 0; }
	if($@BG15b_Team2) { bg_destroy $@BG15b_Team2; $@BG15b_Team2 = 0; }
	bg_unbook .Castle$;
	end;
}

-	script	Rush2	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;

OnTeam1Active:
OnTeam2Active:
	warp .Castle$,216,103;
	end;

OnReady:
	if ($@BG15c)
		end;

	set .Castle$,"rush_cas03";
	set .GX, 60; set .GY, 13; set .CX, 64; set .CY, 13;

	set $@BG15c,1;
	initnpctimer;
	initnpctimer "Rush_Respawn2";
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	set .Defender, 0; // No Defender
	// Emperium =====================
	bg_monster 0,.Castle$,28,102,"Emperium",20402,"Rush2::OnEmperium";

	// ======================================
	sleep 2100;
	bg_warp $@BG15c_Team1,.Castle$,216,103;
	bg_warp $@BG15c_Team2,.Castle$,216,103;
	.x = 216; .y = 103;
	sleep 4000;
	mapannounce .Castle$,"Move on warriors!! Let's capture this Castle!!",1,0xDDA0DD;
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Rush will ends in 6 minutes",1,0xDDA0DD;
	end;

OnTimer360000:
	mapannounce .Castle$,"Battle of Rush will ends in 1 minute",1,0xDDA0DD;
	end;

OnEmperium:
	if ($@BG15 != 1)
		end;

	if (.Defender == 0) { // First Capture
		initnpctimer;
		set .Defender,getcharid(4);
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle captured, now prepare to Defend it!!",1,bg_get_data(.Defender,4);

		if (.Defender == $@BG15c_Team1) {
			set $@score_rush_blue,1;
			set .Attacker,$@BG15c_Team2;
		} else {
			set $@score_rush_red,1;
			set .Attacker,$@BG15c_Team1;
		}

		bg_warp .Attacker,.Castle$,.x,.y; // To Cementery and Wait
		bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;

		// Emperium =====================
		bg_monster .Defender,.Castle$,28,102,"Emperium",20402,"Rush2::OnEmperium";

		// ======================================
		initnpctimer "Rush_Respawn2";

		sleep 5000;
		mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Capture the Castle, it's our last chance to Win!!",1,bg_get_data(.Attacker,4);
		end;
	}

	// Second Capture
	mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Castle captured, we won the Battle!!",1,bg_get_data(.Attacker,4);
	if (.Defender == $@BG15c_Team1)
		set $@score_rush_red,2;
	else
		set $@score_rush_blue,2;

	donpcevent "Rush2::OnMatchEnd";
	end;

OnTimer420000:
	if (.Defender == 0)
		mapannounce .Castle$,"Castle captured Fail. No winners",1,0xDDA0DD;
	else {
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle protected, we won the Battle!!",1,bg_get_data(.Defender,4);
		if (.Defender == $@BG15c_Team1)
			set $@score_rush_blue,3;
		else
			set $@score_rush_red,3;
	}

	donpcevent "Rush2::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Rush_Respawn2";
	killmonster .Castle$,"Rush2::OnEmperium";
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	set $@BG15, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	if ($@score_rush_blue > $@score_rush_red) {
		callfunc "BG_get_Rewards",$@BG15c_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG15c_Team2,7828,1;
	} else if ($@score_rush_red > $@score_rush_blue) {
		callfunc "BG_get_Rewards",$@BG15c_Team2,7828,5;
		callfunc "BG_get_Rewards",$@BG15c_Team1,7828,1;
	} else {
		callfunc "BG_get_Rewards",$@BG15c_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG15c_Team2,7828,1;
	}
	// =======================================================
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	sleep 5000;
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	$@BG15c = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if($@BG15c_Team1) { bg_destroy $@BG15c_Team1; $@BG15c_Team1 = 0; }
	if($@BG15c_Team2) { bg_destroy $@BG15c_Team2; $@BG15c_Team2 = 0; }
	bg_unbook .Castle$;
	end;
}

-	script	Rush3	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;

OnTeam1Active:
OnTeam2Active:
	warp .Castle$,100,280;
	end;

OnReady:
	if ($@BG15d)
		end;

	set .Castle$,"rush_cas04";
	set .GX,270; set .GY,292; set .CX,270; set .CY,288;

	set $@BG15d,1;
	initnpctimer;
	initnpctimer "Rush_Respawn3";
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	set .Defender, 0; // No Defender
	// Emperium =====================
	bg_monster 0,.Castle$,245,167,"Emperium",20402,"Rush3::OnEmperium";

	// ======================================
	sleep 2100;
	bg_warp $@BG15d_Team1,.Castle$,100,280;
	bg_warp $@BG15d_Team2,.Castle$,100,280;
	.x = 100; .y = 280;
	sleep 4000;
	mapannounce .Castle$,"Move on warriors!! Let's capture this Castle!!",1,0xDDA0DD;
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Rush will ends in 6 minutes",1,0xDDA0DD;
	end;

OnTimer360000:
	mapannounce .Castle$,"Battle of Rush will ends in 1 minute",1,0xDDA0DD;
	end;

OnEmperium:
	if ($@BG15 != 1)
		end;

	if (.Defender == 0) { // First Capture
		initnpctimer;
		set .Defender,getcharid(4);
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle captured, now prepare to Defend it!!",1,bg_get_data(.Defender,4);

		if (.Defender == $@BG15d_Team1) {
			set $@score_rush_blue,1;
			set .Attacker,$@BG15d_Team2;
		} else {
			set $@score_rush_red,1;
			set .Attacker,$@BG15d_Team1;
		}

		bg_warp .Attacker,.Castle$,.x,.y; // To Cementery and Wait
		bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;

		// Emperium =====================
		bg_monster .Defender,.Castle$,245,167,"Emperium",20402,"Rush3::OnEmperium";

		// ======================================
		initnpctimer "Rush_Respawn3";

		sleep 5000;
		mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Capture the Castle, it's our last chance to Win!!",1,bg_get_data(.Attacker,4);
		end;
	}

	// Second Capture
	mapannounce .Castle$,bg_get_data(.Attacker,3) + " : Castle captured, we won the Battle!!",1,bg_get_data(.Attacker,4);
	if (.Defender == $@BG15d_Team1)
		set $@score_rush_red,2;
	else
		set $@score_rush_blue,2;

	donpcevent "Rush3::OnMatchEnd";
	end;

OnTimer420000:
	if (.Defender == 0)
		mapannounce .Castle$,"Castle captured Fail. No winners",1,0xDDA0DD;
	else {
		mapannounce .Castle$,bg_get_data(.Defender,3) + " : Castle protected, we won the Battle!!",1,bg_get_data(.Defender,4);
		if (.Defender == $@BG15d_Team1)
			set $@score_rush_blue,3;
		else
			set $@score_rush_red,3;
	}

	donpcevent "Rush3::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Rush_Respawn3";
	killmonster .Castle$,"Rush3::OnEmperium";
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	set $@BG15, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	if ($@score_rush_blue > $@score_rush_red) {
		callfunc "BG_get_Rewards",$@BG15d_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG15d_Team2,7828,1;
	} else if ($@score_rush_red > $@score_rush_blue) {
		callfunc "BG_get_Rewards",$@BG15d_Team2,7828,5;
		callfunc "BG_get_Rewards",$@BG15d_Team1,7828,1;
	} else {
		callfunc "BG_get_Rewards",$@BG15d_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG15d_Team2,7828,1;
	}
	// =======================================================
	set $@score_rush_blue, 0;
	set $@score_rush_red, 0;
	sleep 5000;
	bg_updatescore .Castle$,$@score_rush_blue,$@score_rush_red;
	$@BG15d = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if($@BG15d_Team1) { bg_destroy $@BG15d_Team1; $@BG15d_Team1 = 0; }
	if($@BG15d_Team2) { bg_destroy $@BG15d_Team2; $@BG15d_Team2 = 0; }
	bg_unbook .Castle$;
	end;
}

// Battleground Respawn
// *********************************************************************

-	script	Rush_Respawn	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Rush"),"-- Reinforcements entering the Battle of Rush --",1,0xDDA0DD;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Rush");
	set .@Defender,getvariableofnpc(.Defender,"Rush");

		// rush_cas01 ========================================================================
			areapercentheal .@Castle$,198,224,207,233,100,100;
			if (.@Defender == 0)
				areawarp .@Castle$,198,224,207,233,.@Castle$,43,229;
			else if ($@BG15_Team1 == .@Defender) {
				areawarp .@Castle$,198,229,207,233,.@Castle$,71,36;
				areawarp .@Castle$,198,224,207,227,.@Castle$,43,229;
			} else {
				areawarp .@Castle$,198,229,207,233,.@Castle$,43,229;
				areawarp .@Castle$,198,224,207,227,.@Castle$,71,36;
			}

	initnpctimer;
	end;
}
-	script	Rush_Respawn1	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Rush1"),"-- Reinforcements entering the Battle of Rush --",1,0xDDA0DD;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Rush1");
	set .@Defender,getvariableofnpc(.Defender,"Rush1");

		// rush_cas02 ========================================================================
			areapercentheal .@Castle$,142,44,151,53,100,100;
			if (.@Defender == 0)
				areawarp .@Castle$,142,44,151,53,.@Castle$,252,271;
			else if ($@BG15b_Team1 == .@Defender) {
				areawarp .@Castle$,142,49,151,53,.@Castle$,40,235;
				areawarp .@Castle$,142,44,151,47,.@Castle$,252,271;
			} else {
				areawarp .@Castle$,142,49,151,53,.@Castle$,252,271;
				areawarp .@Castle$,142,44,151,47,.@Castle$,40,235;
			}

	initnpctimer;
	end;
}
-	script	Rush_Respawn2	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Rush2"),"-- Reinforcements entering the Battle of Rush --",1,0xDDA0DD;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Rush2");
	set .@Defender,getvariableofnpc(.Defender,"Rush2");

		// rush_cas03 ========================================================================
			areapercentheal .@Castle$,56,6,69,17,100,100;
			if (.@Defender == 0)
				areawarp .@Castle$,56,6,69,17,.@Castle$,216,103;
			else if ($@BG15c_Team1 == .@Defender) {
				areawarp .@Castle$,56,6,61,17,.@Castle$,31,190;
				areawarp .@Castle$,63,6,69,17,.@Castle$,216,103;
			} else {
				areawarp .@Castle$,56,6,61,17,.@Castle$,216,103;
				areawarp .@Castle$,63,6,69,17,.@Castle$,31,190;
			}
	initnpctimer;
	end;
}
-	script	Rush_Respawn3	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Rush3"),"-- Reinforcements entering the Battle of Rush --",1,0xDDA0DD;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Rush3");
	set .@Defender,getvariableofnpc(.Defender,"Rush3");

		// rush_cas04 ========================================================================
			areapercentheal .@Castle$,266,286,275,295,100,100;
			if (.@Defender == 0)
				areawarp .@Castle$,266,286,275,295,.@Castle$,100,280;
			else if ($@BG15d_Team1 == .@Defender) {
				areawarp .@Castle$,266,291,275,295,.@Castle$,116,89;
				areawarp .@Castle$,266,286,275,289,.@Castle$,100,280;
			} else {
				areawarp .@Castle$,266,291,275,295,.@Castle$,100,280;
				areawarp .@Castle$,266,286,275,289,.@Castle$,116,89;
			}

	initnpctimer;
	end;
}

// ===========================================================
// Heal
// ===========================================================
rush_cas01,198,230,6	duplicate(BGHeal)	Therapist in battle#rh1	4_F_SISTER
rush_cas01,198,226,6	duplicate(BGHeal)	Therapist in battle#rh2	4_F_SISTER
rush_cas02,142,50,6	duplicate(BGHeal)	Therapist in battle#rh3	4_F_SISTER
rush_cas02,142,46,6	duplicate(BGHeal)	Therapist in battle#rh4	4_F_SISTER
rush_cas03,60,17,4	duplicate(BGHeal)	Therapist in battle#rh5	4_F_SISTER
rush_cas03,64,17,4	duplicate(BGHeal)	Therapist in battle#rh6	4_F_SISTER
rush_cas04,266,292,6	duplicate(BGHeal)	Therapist in battle#rh7	4_F_SISTER
rush_cas04,266,288,6	duplicate(BGHeal)	Therapist in battle#rh8	4_F_SISTER

// ===========================================================
// Warp Portals
// ===========================================================
rush_cas01,157,135,0	warp	rush106-1	1,1,rush_cas01,184,40
rush_cas01,161,41,0	warp	rush102-1	1,1,rush_cas01,57,202
rush_cas01,184,44,0	warp	rush106	1,1,rush_cas01,157,140
rush_cas01,203,21,0	warp	rush105-1	1,1,rush_cas01,45,25
rush_cas01,210,41,0	warp	rush101-1	1,1,rush_cas01,84,215
rush_cas01,35,183,0	warp	rush104	1,1,rush_cas01,71,82
rush_cas01,45,21,0	warp	rush105	1,1,rush_cas01,203,25
rush_cas01,53,202,0	warp	rush102	1,1,rush_cas01,165,41
rush_cas01,64,164,0	warp	rush103	1,1,rush_cas01,98,25
rush_cas01,71,86,0	warp	rush104-1	1,1,rush_cas01,35,187
rush_cas01,88,215,0	warp	rush101	1,1,rush_cas01,206,41
rush_cas01,98,21,0	warp	rush103-1	1,1,rush_cas01,64,168

rush_cas02,259,212,0	warp	rush201	1,1,rush_cas02,72,240
rush_cas02,75,240,0	warp	rush201-1	1,5,rush_cas02,256,212
rush_cas02,232,189,0	warp	rush202	1,1,rush_cas02,74,261
rush_cas02,78,261,0	warp	rush202-1	1,1,rush_cas02,236,189
rush_cas02,229,208,0	warp	rush203	1,1,rush_cas02,70,282
rush_cas02,74,282,0	warp	rush203-1	1,1,rush_cas02,225,208
rush_cas02,7,261,0	warp	rush204	1,1,rush_cas02,55,30
rush_cas02,59,30,0	warp	rush204-1	1,1,rush_cas02,11,261
rush_cas02,28,31,0	warp	rush205	1,1,rush_cas02,251,42
rush_cas02,254,45,0	warp	rush205-1	1,1,rush_cas02,24,31

rush_cas03,194,71,0	warp	rush301	1,1,rush_cas03,129,194
rush_cas03,125,194,0	warp	rush301-1	1,1,rush_cas03,199,70
rush_cas03,164,86,0	warp	rush302	1,1,rush_cas03,66,189
rush_cas03,70,189,0	warp	rush302-1	1,1,rush_cas03,166,81
rush_cas03,150,67,0	warp	rush303	1,1,rush_cas03,9,187
rush_cas03,5,187,0	warp	rush303-1	1,1,rush_cas03,151,62
rush_cas03,165,232,0	warp	rush304	1,1,rush_cas03,193,49
rush_cas03,188,49,0	warp	rush304-1	1,1,rush_cas03,165,228
rush_cas03,195,42,0	warp	rush305	1,1,rush_cas03,19,227
rush_cas03,15,227,0	warp	rush305-1	1,1,rush_cas03,195,46
rush_cas03,13,175,0	warp	rush306	1,1,rush_cas03,162,194
rush_cas03,166,194,0	warp	rush306-1	1,1,rush_cas03,13,179
rush_cas03,156,231,0	warp	rush307	1,1,rush_cas03,18,88
rush_cas03,14,88,0	warp	rush307-1	1,1,rush_cas03,156,227

rush_cas04,106,217,0	warp	rush407	1,1,rush_cas04,131,15	
rush_cas04,115,210,0	warp	rush408	1,1,rush_cas04,92,215	
rush_cas04,135,15,0	warp	rush407-1	1,1,rush_cas04,110,217	
rush_cas04,135,92,0	warp	rush402-1	1,1,rush_cas04,34,282	
rush_cas04,152,92,0	warp	rush404-1	1,1,rush_cas04,59,255	
rush_cas04,154,16,0	warp	rush414	1,1,rush_cas04,252,11	
rush_cas04,17,206,0	warp	rush406-1	1,1,rush_cas04,29,219	
rush_cas04,212,46,0	warp	rush415	1,1,rush_cas04,225,158	
rush_cas04,225,154,0	warp	rush415-1	1,1,rush_cas04,212,42	
rush_cas04,237,74,0	warp	rush412-1	1,1,rush_cas04,62,213	
rush_cas04,256,11,0	warp	rush414-1	1,1,rush_cas04,159,16	
rush_cas04,266,47,0	warp	rush409-1	1,1,rush_cas04,45,175	
rush_cas04,27,215,0	warp	rush406-2	1,1,rush_cas04,17,202	
rush_cas04,34,286,0	warp	rush402	1,1,rush_cas04,131,92	
rush_cas04,38,243,0	warp	rush406	1,1,rush_cas04,29,219	
rush_cas04,38,259,0	warp	rush403-1	1,1,rush_cas04,43,271	
rush_cas04,42,175,0	warp	rush409	1,1,rush_cas04,266,43	
rush_cas04,43,191,0	warp	rush410-1	1,1,rush_cas04,70,185	
rush_cas04,47,271,0	warp	rush403	1,1,rush_cas04,38,255	
rush_cas04,50,248,0	warp	rush405	1,1,rush_cas04,54,229	
rush_cas04,58,232,0	warp	rush405-1	1,1,rush_cas04,62,213	
rush_cas04,63,255,0	warp	rush404	1,1,rush_cas04,156,92	
rush_cas04,65,215,0	warp	rush412	1,1,rush_cas04,233,74	
rush_cas04,66,223,0	warp	rush401	1,1,rush_cas04,96,53	
rush_cas04,70,182,0	warp	rush410	1,1,rush_cas04,39,191	
rush_cas04,79,244,0	warp	rush413-1	1,1,rush_cas04,91,250	
rush_cas04,88,248,0	warp	rush411-1	1,1,rush_cas04,76,242	
rush_cas04,90,218,0	warp	rush408-1	1,1,rush_cas04,111,210	
rush_cas04,92,53,0	warp	rush401-1	1,1,rush_cas04,62,223	
rush_cas04,93,209,0	warp	rush411	1,1,rush_cas04,92,250	
rush_cas04,95,251,0	warp	rush413	1,1,rush_cas04,91,209	

// ===========================================================
// Map Flags
// ===========================================================
rush_cas01	mapflag	battleground	2
rush_cas01	mapflag	nomemo
rush_cas01	mapflag	nosave	SavePoint
rush_cas01	mapflag	noteleport
rush_cas01	mapflag	nowarp
rush_cas01	mapflag	nowarpto
rush_cas01	mapflag	noreturn
rush_cas01	mapflag	nobranch
rush_cas01	mapflag	nopenalty
// rush_cas01	mapflag	bg_consume

rush_cas02	mapflag	battleground	2
rush_cas02	mapflag	nomemo
rush_cas02	mapflag	nosave	SavePoint
rush_cas02	mapflag	noteleport
rush_cas02	mapflag	nowarp
rush_cas02	mapflag	nowarpto
rush_cas02	mapflag	noreturn
rush_cas02	mapflag	nobranch
rush_cas02	mapflag	nopenalty
// rush_cas02	mapflag	bg_consume

rush_cas03	mapflag	battleground	2
rush_cas03	mapflag	nomemo
rush_cas03	mapflag	nosave	SavePoint
rush_cas03	mapflag	noteleport
rush_cas03	mapflag	nowarp
rush_cas03	mapflag	nowarpto
rush_cas03	mapflag	noreturn
rush_cas03	mapflag	nobranch
rush_cas03	mapflag	nopenalty
// rush_cas03	mapflag	bg_consume

rush_cas04	mapflag	battleground	2
rush_cas04	mapflag	nomemo
rush_cas04	mapflag	nosave	SavePoint
rush_cas04	mapflag	noteleport
rush_cas04	mapflag	nowarp
rush_cas04	mapflag	nowarpto
rush_cas04	mapflag	noreturn
rush_cas04	mapflag	nobranch
rush_cas04	mapflag	nopenalty
// rush_cas04	mapflag	bg_consume
