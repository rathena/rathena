//===== rAthena Script =======================================
//= Battleground Extended + : Conquest
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
// Configuration Conquest 1
// ===========================================================
-	script	Conquest	FAKE_NPC,{
	end;

OnInit:
	// Cementery Walls
	setwall "schg_cas06",291,376,8,0,0,"conquest_wall_a";
	setwall "schg_cas06",268,376,8,0,0,"conquest_wall_b";
	setwall "schg_cas07",93,301,21,0,0,"conquest_wall_c";
	setwall "schg_cas07",110,301,21,0,0,"conquest_wall_d";
	setwall "schg_cas08",84,43,6,6,0,"conquest_wall_e";
	setwall "schg_cas08",26,43,6,6,0,"conquest_wall_f";
	setwall "arug_cas06",121,353,10,0,0,"conquest_wall_g";
	setwall "arug_cas06",25,377,10,6,0,"conquest_wall_h";
	setwall "arug_cas07",291,335,7,6,0,"conquest_wall_i1";
	setwall "arug_cas07",300,335,6,6,0,"conquest_wall_j1";
	setwall "arug_cas07",291,312,7,6,0,"conquest_wall_i2";
	setwall "arug_cas07",300,312,4,6,0,"conquest_wall_j2";
	setwall "arug_cas08",317,150,8,0,0,"conquest_wall_k";
	setwall "arug_cas08",326,150,8,0,0,"conquest_wall_l";
	set .Index,-1;
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;
OnTeam1Active:
	warp .Castle$,.DX,.DY; // Flag Respawn - Emperium
	end;
OnTeam2Active:
	warp .Castle$,.AX,.AY; // Castle Entrance
	end;

OnReady:
	set .Castle$,"schg_cas06";
	set .DX,120; set .DY,290; set .AX,119; set .AY,  8;
	set $@BG11,1;
	initnpctimer;
	initnpctimer "Conquest_Respawn";
	set .Score, 0;

	switch(rand(2)) {
		case 1:
			set .Defender, $@BG11_Team1;
			set .Atacker, $@BG11_Team2;
			break;
		case 0:
			set .Defender, $@BG11_Team2;
			set .Atacker, $@BG11_Team1;
			break;
	}
	// =========================================================================
	// Build Structures
	// =========================================================================
			setwall .Castle$,114,48,13,6,0,"conq_RL00";
			setwall .Castle$,114,51,13,6,1,"conq_RL01";
			setwall .Castle$,114,154,13,6,1,"conq_RL02";
			setwall .Castle$,116,241,11,6,1,"conq_RL03";

			bg_monster .Defender,.Castle$,115,49," ",1906,"Conquest::OnBarricade0";
			bg_monster .Defender,.Castle$,117,49," ",1906,"Conquest::OnBarricade0";
			bg_monster .Defender,.Castle$,119,49," ",1906,"Conquest::OnBarricade0";
			bg_monster .Defender,.Castle$,121,49," ",1906,"Conquest::OnBarricade0";
			bg_monster .Defender,.Castle$,123,49," ",1906,"Conquest::OnBarricade0";
			bg_monster .Defender,.Castle$,125,49," ",1906,"Conquest::OnBarricade0";

			bg_monster .Defender,.Castle$,115,50," ",1906,"Conquest::OnBarricade1";
			bg_monster .Defender,.Castle$,117,50," ",1906,"Conquest::OnBarricade1";
			bg_monster .Defender,.Castle$,119,50," ",1906,"Conquest::OnBarricade1";
			bg_monster .Defender,.Castle$,121,50," ",1906,"Conquest::OnBarricade1";
			bg_monster .Defender,.Castle$,123,50," ",1906,"Conquest::OnBarricade1";
			bg_monster .Defender,.Castle$,125,50," ",1906,"Conquest::OnBarricade1";

			bg_monster .Defender,.Castle$,115,153," ",1906,"Conquest::OnBarricade2";
			bg_monster .Defender,.Castle$,117,153," ",1906,"Conquest::OnBarricade2";
			bg_monster .Defender,.Castle$,119,153," ",1906,"Conquest::OnBarricade2";
			bg_monster .Defender,.Castle$,121,153," ",1906,"Conquest::OnBarricade2";
			bg_monster .Defender,.Castle$,123,153," ",1906,"Conquest::OnBarricade2";
			bg_monster .Defender,.Castle$,125,153," ",1906,"Conquest::OnBarricade2";

			bg_monster .Defender,.Castle$,116,240," ",1906,"Conquest::OnBarricade3";
			bg_monster .Defender,.Castle$,118,240," ",1906,"Conquest::OnBarricade3";
			bg_monster .Defender,.Castle$,120,240," ",1906,"Conquest::OnBarricade3";
			bg_monster .Defender,.Castle$,122,240," ",1906,"Conquest::OnBarricade3";

			bg_monster .Defender,.Castle$,27,35,"1st Guardian Stone",20400,"Conquest::OnGuardianStone1";
			bg_monster .Defender,.Castle$,207,75,"2st Guardian Stone",20401,"Conquest::OnGuardianStone2";

			bg_monster .Defender,.Castle$,120,272,"Emperium",20402,"Conquest::OnEmperium";
	set .Stone, 2;
	set .Wall1, 6;
	set .Wall2, 6;
	set .Wall3, 4;
	// ======================================
	donpceventall("OnEmblemConquest");
	sleep 2100;
	bg_warp .Defender,.Castle$,.DX,.DY; // Flag Respawn - Emperium
	bg_warp .Atacker,.Castle$,.AX,.AY; // Castle Entrance
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Atacker,3) + " : Move on warriors!! Let's capture this Castle!!",1,bg_get_data(.Atacker,4);
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Defender,3) + " : Protect the Castle and it's defenses to all cost!!",1,bg_get_data(.Defender,4);
	end;

OnGuardianStone1:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"1st Guardian Stone has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
	donpcevent "Conquest::OnCheckGStone";
	end;

OnGuardianStone2:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"2nd Guardian Stone has fallen - 6 minutes to destroy the 1st",1,0xFFA500;
	donpcevent "Conquest::OnCheckGStone";
	end;

OnCheckGStone:
	initnpctimer;
	if (.Stone < 1) {
		mapannounce .Castle$,"Both Guardian Stones has fallen - 6 minutes to destroy the 1st Fortress Gate",1,0xFFA500;
		killmonster .Castle$,"Conquest::OnBarricade0";
		delwall "conq_RL00";
	}
	end;

OnBarricade0:
	end;

OnBarricade1:
	if(set(.Wall1,.Wall1 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"1st Fortress Gate has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL01";
	}
	end;
OnBarricade2:
	if (set(.Wall2,.Wall2 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"2nd Fortress Gate has fallen - 6 minutes to destroy the 3rd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL02";
	}
	end;
OnBarricade3:
	if(set(.Wall3,.Wall3 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"3rd Fortress Gate has fallen - 6 minutes to destroy the Emperium",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL03";
	}
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Conquest will ends in 5 minutes",1,0xFFA500;
	end;

OnTimer300000:
	mapannounce .Castle$,"Battle of Conquest will ends in 1 minute",1,0xFFA500;
	end;

OnEmperium:
	set .Score, .Score + 1;
	mapannounce .Castle$,"The " + bg_get_data(.Atacker,2) + " army has won the Battle and captured the Castle!!",1,bg_get_data(.Atacker,4);
	donpcevent "Conquest::OnMatchEnd";
	end;

OnTimer360000:
	mapannounce .Castle$,"The " + bg_get_data(.Defender,2) + " army has won the Battle and protected the Castle!!",1,bg_get_data(.Defender,4);
	donpcevent "Conquest::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Conquest_Respawn";
	killmonster .Castle$,"Conquest::OnBarricade0";
	killmonster .Castle$,"Conquest::OnBarricade1";
	killmonster .Castle$,"Conquest::OnBarricade2";
	killmonster .Castle$,"Conquest::OnBarricade3";
	killmonster .Castle$,"Conquest::OnGuardianStone1";
	killmonster .Castle$,"Conquest::OnGuardianStone2";
	killmonster .Castle$,"Conquest::OnEmperium";
	set $@BG11, 2;

	// =======================================================
	// Team Rewards
	// =======================================================
	if (.Score >= 6 ) { // Emperium Taken
		callfunc "BG_get_Rewards",.Atacker,7828,20;
		callfunc "BG_get_Rewards",.Defender,7828,5;
	} else {
		callfunc "BG_get_Rewards",.Atacker,7828,1;
		callfunc "BG_get_Rewards",.Defender,7828,5;
	}
	// =======================================================
	set .Score, 0;
	sleep 5000;
	$@BG11 = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if($@BG11_Team1) { bg_destroy $@BG11_Team1; $@BG11_Team1 = 0; }
	if($@BG11_Team2) { bg_destroy $@BG11_Team2; $@BG11_Team2 = 0; }
	bg_unbook .Castle$;
	if(checkwall("conq_RL00")) delwall "conq_RL00";
	if(checkwall("conq_RL01")) delwall "conq_RL01";
	if(checkwall("conq_RL02")) delwall "conq_RL02";
	if(checkwall("conq_RL03")) delwall "conq_RL03";
	end;
}
-	script	Conquest1	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;
OnTeam1Active:
	warp .Castle$,.DX,.DY; // Flag Respawn - Emperium
	end;
OnTeam2Active:
	warp .Castle$,.AX,.AY; // Castle Entrance
	end;

OnReady:
	if( $@BG11b )
		end;

	set .Castle$,"schg_cas07";
	set .DX,136; set .DY,188; set .AX,339; set .AY, 78;
	set $@BG11b,1;
	initnpctimer;
	initnpctimer "Conquest_Respawn1";
	set .Score, 0;

	switch(rand(2)) {
		case 1:
			set .Defender, $@BG11b_Team1;
			set .Atacker, $@BG11b_Team2;
			break;
		case 0:
			set .Defender, $@BG11b_Team2;
			set .Atacker, $@BG11b_Team1;
			break;
	}
	// =========================================================================
	// Build Structures
	// =========================================================================
			setwall .Castle$,290,98,8,0,0,"conq_RL00";
			setwall .Castle$,279,98,8,0,1,"conq_RL01";
			setwall .Castle$,230,213,6,0,1,"conq_RL02";
			setwall .Castle$,160,141,6,6,1,"conq_RL03";

			bg_monster .Defender,.Castle$,289,98," ",1906,"Conquest1::OnBarricade0";
			bg_monster .Defender,.Castle$,289,100," ",1906,"Conquest1::OnBarricade0";
			bg_monster .Defender,.Castle$,289,102," ",1906,"Conquest1::OnBarricade0";
			bg_monster .Defender,.Castle$,289,104," ",1906,"Conquest1::OnBarricade0";

			bg_monster .Defender,.Castle$,280,98," ",1906,"Conquest1::OnBarricade1";
			bg_monster .Defender,.Castle$,280,100," ",1906,"Conquest1::OnBarricade1";
			bg_monster .Defender,.Castle$,280,102," ",1906,"Conquest1::OnBarricade1";
			bg_monster .Defender,.Castle$,281,99," ",1906,"Conquest1::OnBarricade1";
			bg_monster .Defender,.Castle$,281,101," ",1906,"Conquest1::OnBarricade1";
			bg_monster .Defender,.Castle$,281,103," ",1906,"Conquest1::OnBarricade1";

			bg_monster .Defender,.Castle$,231,214," ",1906,"Conquest1::OnBarricade2";
			bg_monster .Defender,.Castle$,231,216," ",1906,"Conquest1::OnBarricade2";
			bg_monster .Defender,.Castle$,231,218," ",1906,"Conquest1::OnBarricade2";
			bg_monster .Defender,.Castle$,232,213," ",1906,"Conquest1::OnBarricade2";
			bg_monster .Defender,.Castle$,232,215," ",1906,"Conquest1::OnBarricade2";
			bg_monster .Defender,.Castle$,232,217," ",1906,"Conquest1::OnBarricade2";

			bg_monster .Defender,.Castle$,160,140," ",1906,"Conquest1::OnBarricade3";
			bg_monster .Defender,.Castle$,162,140," ",1906,"Conquest1::OnBarricade3";
			bg_monster .Defender,.Castle$,164,140," ",1906,"Conquest1::OnBarricade3";
			bg_monster .Defender,.Castle$,166,140," ",1906,"Conquest1::OnBarricade3";

			bg_monster .Defender,.Castle$,231,58,"1st Guardian Stone",20400,"Conquest1::OnGuardianStone1";
			bg_monster .Defender,.Castle$,335,230,"2nd Guardian Stone",20401,"Conquest1::OnGuardianStone2";

			bg_monster .Defender,.Castle$,162,193,"Emperium",20402,"Conquest1::OnEmperium";
	set .Stone, 2;
	set .Wall1, 6;
	set .Wall2, 6;
	set .Wall3, 4;
	// ======================================
	donpceventall("OnEmblemConquest");
	sleep 2100;
	bg_warp .Defender,.Castle$,.DX,.DY; // Flag Respawn - Emperium
	bg_warp .Atacker,.Castle$,.AX,.AY; // Castle Entrance
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Atacker,3) + " : Move on warriors!! Let's capture this Castle!!",1,bg_get_data(.Atacker,4);
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Defender,3) + " : Protect the Castle and it's defenses to all cost!!",1,bg_get_data(.Defender,4);
	end;

OnGuardianStone1:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"1st Guardian Stone has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
	donpcevent "Conquest1::OnCheckGStone";
	end;

OnGuardianStone2:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"2nd Guardian Stone has fallen - 6 minutes to destroy the 1st",1,0xFFA500;
	donpcevent "Conquest1::OnCheckGStone";
	end;

OnCheckGStone:
	initnpctimer;
	if (.Stone < 1) {
		mapannounce .Castle$,"Both Guardian Stones has fallen - 6 minutes to destroy the 1st Fortress Gate",1,0xFFA500;
		killmonster .Castle$,"Conquest1::OnBarricade0";
		delwall "conq_RL00";
	}
	end;

OnBarricade0:
	end;

OnBarricade1:
	if(set(.Wall1,.Wall1 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"1st Fortress Gate has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL01";
	}
	end;
OnBarricade2:
	if (set(.Wall2,.Wall2 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"2nd Fortress Gate has fallen - 6 minutes to destroy the 3rd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL02";
	}
	end;
OnBarricade3:
	if(set(.Wall3,.Wall3 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"3rd Fortress Gate has fallen - 6 minutes to destroy the Emperium",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL03";
	}
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Conquest will ends in 5 minutes",1,0xFFA500;
	end;

OnTimer300000:
	mapannounce .Castle$,"Battle of Conquest will ends in 1 minute",1,0xFFA500;
	end;

OnEmperium:
	set .Score, .Score + 1;
	mapannounce .Castle$,"The " + bg_get_data(.Atacker,2) + " army has won the Battle and captured the Castle!!",1,bg_get_data(.Atacker,4);
	donpcevent "Conquest1::OnMatchEnd";
	end;

OnTimer360000:
	mapannounce .Castle$,"The " + bg_get_data(.Defender,2) + " army has won the Battle and protected the Castle!!",1,bg_get_data(.Defender,4);
	donpcevent "Conquest1::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Conquest_Respawn1";
	killmonster .Castle$,"Conquest1::OnBarricade0";
	killmonster .Castle$,"Conquest1::OnBarricade1";
	killmonster .Castle$,"Conquest1::OnBarricade2";
	killmonster .Castle$,"Conquest1::OnBarricade3";
	killmonster .Castle$,"Conquest1::OnGuardianStone1";
	killmonster .Castle$,"Conquest1::OnGuardianStone2";
	killmonster .Castle$,"Conquest1::OnEmperium";
	set $@BG11, 2;

	// =======================================================
	// Team Rewards
	// =======================================================
	if (.Score >= 6 ) { // Emperium Taken
		callfunc "BG_get_Rewards",.Atacker,7828,20;
		callfunc "BG_get_Rewards",.Defender,7828,5;
	} else {
		callfunc "BG_get_Rewards",.Atacker,7828,5;
		callfunc "BG_get_Rewards",.Defender,7828,20;
	}

	// =======================================================
	set .Score, 0;
	sleep 5000;
	$@BG11 = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if(.Atacker) { bg_destroy .Atacker; $@BG11_Team1 = 0; }
	if(.Defender) { bg_destroy .Defender; $@BG11_Team2 = 0; }
	bg_unbook .Castle$;
	if(checkwall("conq_RL00")) delwall "conq_RL00";
	if(checkwall("conq_RL01")) delwall "conq_RL01";
	if(checkwall("conq_RL02")) delwall "conq_RL02";
	if(checkwall("conq_RL03")) delwall "conq_RL03";
	end;
}

-	script	Conquest2	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;
OnTeam1Active:
	warp .Castle$,.DX,.DY; // Flag Respawn - Emperium
	end;
OnTeam2Active:
	warp .Castle$,.AX,.AY; // Castle Entrance
	end;

OnReady:
	if( $@BG11c )
		end;

	set .Castle$,"schg_cas08";
	set .DX,308; set .DY,208; set .AX,337; set .AY,330;
	set $@BG11c,1;
	initnpctimer;
	initnpctimer "Conquest_Respawn2";
	set .Score, 0;

	switch(rand(2)) {
		case 1:
			set .Defender, $@BG11c_Team1;
			set .Atacker, $@BG11c_Team2;
			break;
		case 0:
			set .Defender, $@BG11c_Team2;
			set .Atacker, $@BG11c_Team1;
			break;
	}
	// =========================================================================
	// Build Structures
	// =========================================================================
			setwall .Castle$,326,301,6,6,0,"conq_RL00";
			setwall .Castle$,325,277,8,6,1,"conq_RL01";
			setwall .Castle$,200,230,8,0,1,"conq_RL02";
			setwall .Castle$,285,198,8,0,1,"conq_RL03";

			bg_monster .Defender,.Castle$,326,300," ",1906,"Conquest2::OnBarricade0";
			bg_monster .Defender,.Castle$,328,300," ",1906,"Conquest2::OnBarricade0";
			bg_monster .Defender,.Castle$,330,300," ",1906,"Conquest2::OnBarricade0";

			bg_monster .Defender,.Castle$,326,278," ",1906,"Conquest2::OnBarricade1";
			bg_monster .Defender,.Castle$,328,278," ",1906,"Conquest2::OnBarricade1";
			bg_monster .Defender,.Castle$,330,278," ",1906,"Conquest2::OnBarricade1";
			bg_monster .Defender,.Castle$,327,279," ",1906,"Conquest2::OnBarricade1";
			bg_monster .Defender,.Castle$,329,279," ",1906,"Conquest2::OnBarricade1";
			bg_monster .Defender,.Castle$,331,279," ",1906,"Conquest2::OnBarricade1";

			bg_monster .Defender,.Castle$,201,231," ",1906,"Conquest2::OnBarricade2";
			bg_monster .Defender,.Castle$,201,233," ",1906,"Conquest2::OnBarricade2";
			bg_monster .Defender,.Castle$,201,235," ",1906,"Conquest2::OnBarricade2";
			bg_monster .Defender,.Castle$,202,232," ",1906,"Conquest2::OnBarricade2";
			bg_monster .Defender,.Castle$,202,234," ",1906,"Conquest2::OnBarricade2";
			bg_monster .Defender,.Castle$,202,236," ",1906,"Conquest2::OnBarricade2";

			bg_monster .Defender,.Castle$,284,199," ",1906,"Conquest2::OnBarricade3";
			bg_monster .Defender,.Castle$,284,201," ",1906,"Conquest2::OnBarricade3";
			bg_monster .Defender,.Castle$,284,203," ",1906,"Conquest2::OnBarricade3";
			bg_monster .Defender,.Castle$,284,205," ",1906,"Conquest2::OnBarricade3";

			bg_monster .Defender,.Castle$,242,309,"1st Guardian Stone",20400,"Conquest2::OnGuardianStone1";
			bg_monster .Defender,.Castle$,376,251,"2nd Guardian Stone",20401,"Conquest2::OnGuardianStone2";

			bg_monster .Defender,.Castle$,338,202,"Emperium",20402,"Conquest2::OnEmperium";
	set .Stone, 2;
	set .Wall1, 6;
	set .Wall2, 6;
	set .Wall3, 4;
	// ======================================
	donpceventall("OnEmblemConquest");
	sleep 2100;
	bg_warp .Defender,.Castle$,.DX,.DY; // Flag Respawn - Emperium
	bg_warp .Atacker,.Castle$,.AX,.AY; // Castle Entrance
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Atacker,3) + " : Move on warriors!! Let's capture this Castle!!",1,bg_get_data(.Atacker,4);
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Defender,3) + " : Protect the Castle and it's defenses to all cost!!",1,bg_get_data(.Defender,4);
	end;

OnGuardianStone1:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"1st Guardian Stone has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
	donpcevent "Conquest2::OnCheckGStone";
	end;

OnGuardianStone2:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"2nd Guardian Stone has fallen - 6 minutes to destroy the 1st",1,0xFFA500;
	donpcevent "Conquest2::OnCheckGStone";
	end;

OnCheckGStone:
	initnpctimer;
	if (.Stone < 1) {
		mapannounce .Castle$,"Both Guardian Stones has fallen - 6 minutes to destroy the 1st Fortress Gate",1,0xFFA500;
		killmonster .Castle$,"Conquest2::OnBarricade0";
		delwall "conq_RL00";
	}
	end;

OnBarricade0:
	end;

OnBarricade1:
	if(set(.Wall1,.Wall1 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"1st Fortress Gate has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL01";
	}
	end;
OnBarricade2:
	if (set(.Wall2,.Wall2 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"2nd Fortress Gate has fallen - 6 minutes to destroy the 3rd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL02";
	}
	end;
OnBarricade3:
	if(set(.Wall3,.Wall3 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"3rd Fortress Gate has fallen - 6 minutes to destroy the Emperium",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL03";
	}
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Conquest will ends in 5 minutes",1,0xFFA500;
	end;

OnTimer300000:
	mapannounce .Castle$,"Battle of Conquest will ends in 1 minute",1,0xFFA500;
	end;

OnEmperium:
	set .Score, .Score + 1;
	mapannounce .Castle$,"The " + bg_get_data(.Atacker,2) + " army has won the Battle and captured the Castle!!",1,bg_get_data(.Atacker,4);
	donpcevent "Conquest2::OnMatchEnd";
	end;

OnTimer360000:
	mapannounce .Castle$,"The " + bg_get_data(.Defender,2) + " army has won the Battle and protected the Castle!!",1,bg_get_data(.Defender,4);
	donpcevent "Conquest2::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Conquest_Respawn2";
	killmonster .Castle$,"Conquest2::OnBarricade0";
	killmonster .Castle$,"Conquest2::OnBarricade1";
	killmonster .Castle$,"Conquest2::OnBarricade2";
	killmonster .Castle$,"Conquest2::OnBarricade3";
	killmonster .Castle$,"Conquest2::OnGuardianStone1";
	killmonster .Castle$,"Conquest2::OnGuardianStone2";
	killmonster .Castle$,"Conquest2::OnEmperium";
	set $@BG11, 2;

	// =======================================================
	// Team Rewards
	// =======================================================
	if (.Score >= 6 ) { // Emperium Taken
		callfunc "BG_get_Rewards",.Atacker,7828,20;
		callfunc "BG_get_Rewards",.Defender,7828,5;
	} else {
		callfunc "BG_get_Rewards",.Atacker,7828,5;
		callfunc "BG_get_Rewards",.Defender,7828,20;
	}

	// =======================================================
	set .Score, 0;
	sleep 5000;
	$@BG11 = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if(.Atacker) { bg_destroy .Atacker; $@BG11_Team1 = 0; }
	if(.Defender) { bg_destroy .Defender; $@BG11_Team2 = 0; }
	bg_unbook .Castle$;
	if(checkwall("conq_RL00")) delwall "conq_RL00";
	if(checkwall("conq_RL01")) delwall "conq_RL01";
	if(checkwall("conq_RL02")) delwall "conq_RL02";
	if(checkwall("conq_RL03")) delwall "conq_RL03";
	end;
}


-	script	Conquest3	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;
OnTeam1Active:
	warp .Castle$,.DX,.DY; // Flag Respawn - Emperium
	end;
OnTeam2Active:
	warp .Castle$,.AX,.AY; // Castle Entrance
	end;

OnReady:
	if( $@BG11d )
		end;

	set .Castle$,"arug_cas06";
	set .DX, 67; set .DY,193; set .AX,275; set .AY, 93;
	set $@BG11d,1;
	initnpctimer;
	initnpctimer "Conquest_Respawn3";
	set .Score, 0;

	switch(rand(2)) {
		case 1:
			set .Defender, $@BG11d_Team1;
			set .Atacker, $@BG11d_Team2;
			break;
		case 0:
			set .Defender, $@BG11d_Team2;
			set .Atacker, $@BG11d_Team1;
			break;
	}
	// =========================================================================
	// Build Structures
	// =========================================================================
			setwall .Castle$,238,74,8,6,0,"conq_RL00";
			setwall .Castle$,239,53,8,6,1,"conq_RL01";
			setwall .Castle$,107,124,6,6,1,"conq_RL02";
			setwall .Castle$,84,171,8,6,1,"conq_RL03";

			bg_monster .Defender,.Castle$,239,73," ",1906,"Conquest3::OnBarricade0";
			bg_monster .Defender,.Castle$,241,73," ",1906,"Conquest3::OnBarricade0";
			bg_monster .Defender,.Castle$,243,73," ",1906,"Conquest3::OnBarricade0";
			bg_monster .Defender,.Castle$,245,73," ",1906,"Conquest3::OnBarricade0";

			bg_monster .Defender,.Castle$,239,55," ",1906,"Conquest3::OnBarricade1";
			bg_monster .Defender,.Castle$,241,55," ",1906,"Conquest3::OnBarricade1";
			bg_monster .Defender,.Castle$,243,55," ",1906,"Conquest3::OnBarricade1";
			bg_monster .Defender,.Castle$,240,54," ",1906,"Conquest3::OnBarricade1";
			bg_monster .Defender,.Castle$,242,54," ",1906,"Conquest3::OnBarricade1";
			bg_monster .Defender,.Castle$,244,54," ",1906,"Conquest3::OnBarricade1";

			bg_monster .Defender,.Castle$,107,122," ",1906,"Conquest3::OnBarricade2";
			bg_monster .Defender,.Castle$,109,122," ",1906,"Conquest3::OnBarricade2";
			bg_monster .Defender,.Castle$,111,122," ",1906,"Conquest3::OnBarricade2";
			bg_monster .Defender,.Castle$,108,123," ",1906,"Conquest3::OnBarricade2";
			bg_monster .Defender,.Castle$,110,123," ",1906,"Conquest3::OnBarricade2";
			bg_monster .Defender,.Castle$,112,123," ",1906,"Conquest3::OnBarricade2";

			bg_monster .Defender,.Castle$,84,170," ",1906,"Conquest3::OnBarricade3";
			bg_monster .Defender,.Castle$,86,170," ",1906,"Conquest3::OnBarricade3";
			bg_monster .Defender,.Castle$,88,170," ",1906,"Conquest3::OnBarricade3";
			bg_monster .Defender,.Castle$,90,170," ",1906,"Conquest3::OnBarricade3";

			bg_monster .Defender,.Castle$,210,234,"1st Guardian Stone",20400,"Conquest3::OnGuardianStone1";
			bg_monster .Defender,.Castle$,308,189,"2nd Guardian Stone",20401,"Conquest3::OnGuardianStone2";

			bg_monster .Defender,.Castle$,87,219,"Emperium",20402,"Conquest3::OnEmperium";
	set .Stone, 2;
	set .Wall1, 6;
	set .Wall2, 6;
	set .Wall3, 4;
	// ======================================
	donpceventall("OnEmblemConquest");
	sleep 2100;
	bg_warp .Defender,.Castle$,.DX,.DY; // Flag Respawn - Emperium
	bg_warp .Atacker,.Castle$,.AX,.AY; // Castle Entrance
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Atacker,3) + " : Move on warriors!! Let's capture this Castle!!",1,bg_get_data(.Atacker,4);
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Defender,3) + " : Protect the Castle and it's defenses to all cost!!",1,bg_get_data(.Defender,4);
	end;

OnGuardianStone1:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"1st Guardian Stone has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
	donpcevent "Conquest3::OnCheckGStone";
	end;

OnGuardianStone2:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"2nd Guardian Stone has fallen - 6 minutes to destroy the 1st",1,0xFFA500;
	donpcevent "Conquest3::OnCheckGStone";
	end;

OnCheckGStone:
	initnpctimer;
	if (.Stone < 1) {
		mapannounce .Castle$,"Both Guardian Stones has fallen - 6 minutes to destroy the 1st Fortress Gate",1,0xFFA500;
		killmonster .Castle$,"Conquest3::OnBarricade0";
		delwall "conq_RL00";
	}
	end;

OnBarricade0:
	end;

OnBarricade1:
	if(set(.Wall1,.Wall1 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"1st Fortress Gate has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL01";
	}
	end;
OnBarricade2:
	if (set(.Wall2,.Wall2 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"2nd Fortress Gate has fallen - 6 minutes to destroy the 3rd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL02";
	}
	end;
OnBarricade3:
	if(set(.Wall3,.Wall3 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"3rd Fortress Gate has fallen - 6 minutes to destroy the Emperium",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL03";
	}
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Conquest will ends in 5 minutes",1,0xFFA500;
	end;

OnTimer300000:
	mapannounce .Castle$,"Battle of Conquest will ends in 1 minute",1,0xFFA500;
	end;

OnEmperium:
	set .Score, .Score + 1;
	mapannounce .Castle$,"The " + bg_get_data(.Atacker,2) + " army has won the Battle and captured the Castle!!",1,bg_get_data(.Atacker,4);
	donpcevent "Conquest3::OnMatchEnd";
	end;

OnTimer360000:
	mapannounce .Castle$,"The " + bg_get_data(.Defender,2) + " army has won the Battle and protected the Castle!!",1,bg_get_data(.Defender,4);
	donpcevent "Conquest3::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Conquest_Respawn3";
	killmonster .Castle$,"Conquest3::OnBarricade0";
	killmonster .Castle$,"Conquest3::OnBarricade1";
	killmonster .Castle$,"Conquest3::OnBarricade2";
	killmonster .Castle$,"Conquest3::OnBarricade3";
	killmonster .Castle$,"Conquest3::OnGuardianStone1";
	killmonster .Castle$,"Conquest3::OnGuardianStone2";
	killmonster .Castle$,"Conquest3::OnEmperium";
	set $@BG11, 2;

	// =======================================================
	// Team Rewards
	// =======================================================
	if (.Score >= 6 ) { // Emperium Taken
		callfunc "BG_get_Rewards",.Atacker,7828,20;
		callfunc "BG_get_Rewards",.Defender,7828,5;
	} else {
		callfunc "BG_get_Rewards",.Atacker,7828,5;
		callfunc "BG_get_Rewards",.Defender,7828,20;
	}
	// =======================================================
	set .Score, 0;
	sleep 5000;
	$@BG11 = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if(.Atacker) { bg_destroy .Atacker; $@BG11_Team1 = 0; }
	if(.Defender) { bg_destroy .Defender; $@BG11_Team2 = 0; }
	bg_unbook .Castle$;
	if(checkwall("conq_RL00")) delwall "conq_RL00";
	if(checkwall("conq_RL01")) delwall "conq_RL01";
	if(checkwall("conq_RL02")) delwall "conq_RL02";
	if(checkwall("conq_RL03")) delwall "conq_RL03";
	end;
}


-	script	Conquest4	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;
OnTeam1Active:
	warp .Castle$,.DX,.DY; // Flag Respawn - Emperium
	end;
OnTeam2Active:
	warp .Castle$,.AX,.AY; // Castle Entrance
	end;

OnReady:
	if( $@BG11e )
		end;

	set .Castle$,"arug_cas07";
	set .DX, 43; set .DY,256; set .AX,139; set .AY, 31;
	set $@BG11e,1;
	initnpctimer;
	initnpctimer "Conquest_Respawn4";
	set .Score, 0;

	switch(rand(2)) {
		case 1:
			set .Defender, $@BG11e_Team1;
			set .Atacker, $@BG11e_Team2;
			break;
		case 0:
			set .Defender, $@BG11e_Team2;
			set .Atacker, $@BG11e_Team1;
			break;
	}
	// =========================================================================
	// Build Structures
	// =========================================================================
			setwall .Castle$,136,136,8,6,0,"conq_RL00";
			setwall .Castle$,150,223,12,6,1,"conq_RL01";
			setwall .Castle$,125,342,8,0,1,"conq_RL02";
			setwall .Castle$,38,314,12,6,1,"conq_RL03";

			bg_monster .Defender,.Castle$,137,137," ",1906,"Conquest4::OnBarricade0";
			bg_monster .Defender,.Castle$,139,137," ",1906,"Conquest4::OnBarricade0";
			bg_monster .Defender,.Castle$,141,137," ",1906,"Conquest4::OnBarricade0";
			bg_monster .Defender,.Castle$,143,137," ",1906,"Conquest4::OnBarricade0";

			bg_monster .Defender,.Castle$,151,222," ",1906,"Conquest4::OnBarricade1";
			bg_monster .Defender,.Castle$,153,222," ",1906,"Conquest4::OnBarricade1";
			bg_monster .Defender,.Castle$,155,222," ",1906,"Conquest4::OnBarricade1";
			bg_monster .Defender,.Castle$,157,222," ",1906,"Conquest4::OnBarricade1";
			bg_monster .Defender,.Castle$,159,222," ",1906,"Conquest4::OnBarricade1";
			bg_monster .Defender,.Castle$,161,222," ",1906,"Conquest4::OnBarricade1";

			bg_monster .Defender,.Castle$,126,343," ",1906,"Conquest4::OnBarricade2";
			bg_monster .Defender,.Castle$,126,345," ",1906,"Conquest4::OnBarricade2";
			bg_monster .Defender,.Castle$,126,347," ",1906,"Conquest4::OnBarricade2";
			bg_monster .Defender,.Castle$,127,344," ",1906,"Conquest4::OnBarricade2";
			bg_monster .Defender,.Castle$,127,346," ",1906,"Conquest4::OnBarricade2";
			bg_monster .Defender,.Castle$,127,348," ",1906,"Conquest4::OnBarricade2";

			bg_monster .Defender,.Castle$,40,315," ",1906,"Conquest4::OnBarricade3";
			bg_monster .Defender,.Castle$,42,315," ",1906,"Conquest4::OnBarricade3";
			bg_monster .Defender,.Castle$,44,315," ",1906,"Conquest4::OnBarricade3";
			bg_monster .Defender,.Castle$,46,315," ",1906,"Conquest4::OnBarricade3";

			bg_monster .Defender,.Castle$,33,168,"1st Guardian Stone",20400,"Conquest4::OnGuardianStone1";
			bg_monster .Defender,.Castle$,245,168,"2nd Guardian Stone",20401,"Conquest4::OnGuardianStone2";

			bg_monster .Defender,.Castle$,89,256,"Emperium",20402,"Conquest4::OnEmperium";
	set .Stone, 2;
	set .Wall1, 6;
	set .Wall2, 6;
	set .Wall3, 4;
	// ======================================
	donpceventall("OnEmblemConquest");
	sleep 2100;
	bg_warp .Defender,.Castle$,.DX,.DY; // Flag Respawn - Emperium
	bg_warp .Atacker,.Castle$,.AX,.AY; // Castle Entrance
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Atacker,3) + " : Move on warriors!! Let's capture this Castle!!",1,bg_get_data(.Atacker,4);
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Defender,3) + " : Protect the Castle and it's defenses to all cost!!",1,bg_get_data(.Defender,4);
	end;

OnGuardianStone1:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"1st Guardian Stone has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
	donpcevent "Conquest4::OnCheckGStone";
	end;

OnGuardianStone2:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"2nd Guardian Stone has fallen - 6 minutes to destroy the 1st",1,0xFFA500;
	donpcevent "Conquest4::OnCheckGStone";
	end;

OnCheckGStone:
	initnpctimer;
	if (.Stone < 1) {
		mapannounce .Castle$,"Both Guardian Stones has fallen - 6 minutes to destroy the 1st Fortress Gate",1,0xFFA500;
		killmonster .Castle$,"Conquest4::OnBarricade0";
		delwall "conq_RL00";
	}
	end;

OnBarricade0:
	end;

OnBarricade1:
	if(set(.Wall1,.Wall1 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"1st Fortress Gate has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL01";
	}
	end;
OnBarricade2:
	if (set(.Wall2,.Wall2 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"2nd Fortress Gate has fallen - 6 minutes to destroy the 3rd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL02";
	}
	end;
OnBarricade3:
	if(set(.Wall3,.Wall3 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"3rd Fortress Gate has fallen - 6 minutes to destroy the Emperium",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL03";
	}
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Conquest will ends in 5 minutes",1,0xFFA500;
	end;

OnTimer300000:
	mapannounce .Castle$,"Battle of Conquest will ends in 1 minute",1,0xFFA500;
	end;

OnEmperium:
	set .Score, .Score + 1;
	mapannounce .Castle$,"The " + bg_get_data(.Atacker,2) + " army has won the Battle and captured the Castle!!",1,bg_get_data(.Atacker,4);
	donpcevent "Conquest4::OnMatchEnd";
	end;

OnTimer360000:
	mapannounce .Castle$,"The " + bg_get_data(.Defender,2) + " army has won the Battle and protected the Castle!!",1,bg_get_data(.Defender,4);
	donpcevent "Conquest4::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Conquest_Respawn4";
	killmonster .Castle$,"Conquest4::OnBarricade0";
	killmonster .Castle$,"Conquest4::OnBarricade1";
	killmonster .Castle$,"Conquest4::OnBarricade2";
	killmonster .Castle$,"Conquest4::OnBarricade3";
	killmonster .Castle$,"Conquest4::OnGuardianStone1";
	killmonster .Castle$,"Conquest4::OnGuardianStone2";
	killmonster .Castle$,"Conquest4::OnEmperium";
	set $@BG11, 2;

	// =======================================================
	// Team Rewards
	// =======================================================
	if (.Score >= 6 ) { // Emperium Taken
		callfunc "BG_get_Rewards",.Atacker,7828,20;
		callfunc "BG_get_Rewards",.Defender,7828,5;
	} else {
		callfunc "BG_get_Rewards",.Atacker,7828,5;
		callfunc "BG_get_Rewards",.Defender,7828,20;
	}

	// =======================================================
	set .Score, 0;
	sleep 5000;
	$@BG11 = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if(.Atacker) { bg_destroy .Atacker; $@BG11_Team1 = 0; }
	if(.Defender) { bg_destroy .Defender; $@BG11_Team2 = 0; }
	bg_unbook .Castle$;
	if(checkwall("conq_RL00")) delwall "conq_RL00";
	if(checkwall("conq_RL01")) delwall "conq_RL01";
	if(checkwall("conq_RL02")) delwall "conq_RL02";
	if(checkwall("conq_RL03")) delwall "conq_RL03";
	end;
}

-	script	Conquest5	FAKE_NPC,{
	end;

OnTeam1Quit:
OnTeam2Quit:
	bg_desert;
	end;
OnTeam1Active:
	warp .Castle$,.DX,.DY; // Flag Respawn - Emperium
	end;
OnTeam2Active:
	warp .Castle$,.AX,.AY; // Castle Entrance
	end;

OnReady:
	if( $@BG11f )
		end;

	set .Castle$,"arug_cas08";
	set .CX,330; set .CY,154; set .DX,121; set .DY,318; set .AX,141; set .AY, 45;
	set $@BG11f,1;
	initnpctimer;
	initnpctimer "Conquest_Respawn5";
	set .Score, 0;

	switch(rand(2)) {
		case 1:
			set .Defender, $@BG11f_Team1;
			set .Atacker, $@BG11f_Team2;
			break;
		case 0:
			set .Defender, $@BG11f_Team2;
			set .Atacker, $@BG11f_Team1;
			break;
	}
	// =========================================================================
	// Build Structures
	// =========================================================================
			setwall .Castle$,138,110,8,6,0,"conq_RL00";
			setwall .Castle$,139,158,6,6,1,"conq_RL01";
			setwall .Castle$,138,210,8,6,1,"conq_RL02";
			setwall .Castle$,138,263,8,6,1,"conq_RL03";

			bg_monster .Defender,.Castle$,139,111," ",1906,"Conquest5::OnBarricade0";
			bg_monster .Defender,.Castle$,141,111," ",1906,"Conquest5::OnBarricade0";
			bg_monster .Defender,.Castle$,143,111," ",1906,"Conquest5::OnBarricade0";
			bg_monster .Defender,.Castle$,145,111," ",1906,"Conquest5::OnBarricade0";

			bg_monster .Defender,.Castle$,140,157," ",1906,"Conquest5::OnBarricade1";
			bg_monster .Defender,.Castle$,142,157," ",1906,"Conquest5::OnBarricade1";
			bg_monster .Defender,.Castle$,144,157," ",1906,"Conquest5::OnBarricade1";
			bg_monster .Defender,.Castle$,139,156," ",1906,"Conquest5::OnBarricade1";
			bg_monster .Defender,.Castle$,141,156," ",1906,"Conquest5::OnBarricade1";
			bg_monster .Defender,.Castle$,143,156," ",1906,"Conquest5::OnBarricade1";

			bg_monster .Defender,.Castle$,140,209," ",1906,"Conquest5::OnBarricade2";
			bg_monster .Defender,.Castle$,142,209," ",1906,"Conquest5::OnBarricade2";
			bg_monster .Defender,.Castle$,144,209," ",1906,"Conquest5::OnBarricade2";
			bg_monster .Defender,.Castle$,139,208," ",1906,"Conquest5::OnBarricade2";
			bg_monster .Defender,.Castle$,141,208," ",1906,"Conquest5::OnBarricade2";
			bg_monster .Defender,.Castle$,143,208," ",1906,"Conquest5::OnBarricade2";

			bg_monster .Defender,.Castle$,139,262," ",1906,"Conquest5::OnBarricade3";
			bg_monster .Defender,.Castle$,141,262," ",1906,"Conquest5::OnBarricade3";
			bg_monster .Defender,.Castle$,143,262," ",1906,"Conquest5::OnBarricade3";
			bg_monster .Defender,.Castle$,145,262," ",1906,"Conquest5::OnBarricade3";

			bg_monster .Defender,.Castle$,65,171,"1st Guardian Stone",20400,"Conquest5::OnGuardianStone1";
			bg_monster .Defender,.Castle$,212,149,"2nd Guardian Stone",20401,"Conquest5::OnGuardianStone2";

			bg_monster .Defender,.Castle$,141,293,"Emperium",20402,"Conquest5::OnEmperium";
	set .Stone, 2;
	set .Wall1, 6;
	set .Wall2, 6;
	set .Wall3, 4;
	// ======================================
	donpceventall("OnEmblemConquest");
	sleep 2100;
	bg_warp .Defender,.Castle$,.DX,.DY; // Flag Respawn - Emperium
	bg_warp .Atacker,.Castle$,.AX,.AY; // Castle Entrance
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Atacker,3) + " : Move on warriors!! Let's capture this Castle!!",1,bg_get_data(.Atacker,4);
	sleep 4000;
	mapannounce .Castle$,bg_get_data(.Defender,3) + " : Protect the Castle and it's defenses to all cost!!",1,bg_get_data(.Defender,4);
	end;

OnGuardianStone1:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"1st Guardian Stone has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
	donpcevent "Conquest5::OnCheckGStone";
	end;

OnGuardianStone2:
	set .Stone, .Stone - 1;
	set .Score, .Score + 1;
	if (.Stone > 0) mapannounce .Castle$,"2nd Guardian Stone has fallen - 6 minutes to destroy the 1st",1,0xFFA500;
	donpcevent "Conquest5::OnCheckGStone";
	end;

OnCheckGStone:
	initnpctimer;
	if (.Stone < 1) {
		mapannounce .Castle$,"Both Guardian Stones has fallen - 6 minutes to destroy the 1st Fortress Gate",1,0xFFA500;
		killmonster .Castle$,"Conquest5::OnBarricade0";
		delwall "conq_RL00";
	}
	end;

OnBarricade0:
	end;

OnBarricade1:
	if(set(.Wall1,.Wall1 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"1st Fortress Gate has fallen - 6 minutes to destroy the 2nd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL01";
	}
	end;
OnBarricade2:
	if (set(.Wall2,.Wall2 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"2nd Fortress Gate has fallen - 6 minutes to destroy the 3rd",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL02";
	}
	end;
OnBarricade3:
	if(set(.Wall3,.Wall3 - 1) < 1) {
		initnpctimer; // Restart Timer
		mapannounce .Castle$,"3rd Fortress Gate has fallen - 6 minutes to destroy the Emperium",1,0xFFA500;
		set .Score, .Score + 1;
		delwall "conq_RL03";
	}
	end;

OnTimer60000:
	mapannounce .Castle$,"Battle of Conquest will ends in 5 minutes",1,0xFFA500;
	end;

OnTimer300000:
	mapannounce .Castle$,"Battle of Conquest will ends in 1 minute",1,0xFFA500;
	end;

OnEmperium:
	set .Score, .Score + 1;
	mapannounce .Castle$,"The " + bg_get_data(.Atacker,2) + " army has won the Battle and captured the Castle!!",1,bg_get_data(.Atacker,4);
	donpcevent "Conquest5::OnMatchEnd";
	end;

OnTimer360000:
	mapannounce .Castle$,"The " + bg_get_data(.Defender,2) + " army has won the Battle and protected the Castle!!",1,bg_get_data(.Defender,4);
	donpcevent "Conquest5::OnMatchEnd";
	end;

OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Conquest_Respawn5";
	killmonster .Castle$,"Conquest5::OnBarricade0";
	killmonster .Castle$,"Conquest5::OnBarricade1";
	killmonster .Castle$,"Conquest5::OnBarricade2";
	killmonster .Castle$,"Conquest5::OnBarricade3";
	killmonster .Castle$,"Conquest5::OnGuardianStone1";
	killmonster .Castle$,"Conquest5::OnGuardianStone2";
	killmonster .Castle$,"Conquest5::OnEmperium";
	set $@BG11, 2;

	// =======================================================
	// Team Rewards
	// =======================================================
	if (.Score >= 6 ) { // Emperium Taken
		callfunc "BG_get_Rewards",.Atacker,7828,20;
		callfunc "BG_get_Rewards",.Defender,7828,5;
	} else {
		callfunc "BG_get_Rewards",.Atacker,7828,5;
		callfunc "BG_get_Rewards",.Defender,7828,20;
	}
	// =======================================================
	set .Score, 0;
	sleep 5000;
	$@BG11 = 0;
	mapwarp .Castle$,"bat_room",154,150;
	if(.Atacker) { bg_destroy .Atacker; $@BG11_Team1 = 0; }
	if(.Defender) { bg_destroy .Defender; $@BG11_Team2 = 0; }
	bg_unbook .Castle$;
	if(checkwall("conq_RL00")) delwall "conq_RL00";
	if(checkwall("conq_RL01")) delwall "conq_RL01";
	if(checkwall("conq_RL02")) delwall "conq_RL02";
	if(checkwall("conq_RL03")) delwall "conq_RL03";
	end;
}

//============================================================
// = Respawn
//============================================================
-	script	Conquest_Respawn	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Conquest"),"-- Reinforcements entering the Battle of Conquest --",1,0xFFA500;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Conquest");
	set .@DX,getvariableofnpc(.DX,"Conquest");
	set .@DY,getvariableofnpc(.DY,"Conquest");
	set .@AX,getvariableofnpc(.AX,"Conquest");
	set .@AY,getvariableofnpc(.AY,"Conquest");

	// schg_cas01 ========================================================================
			areapercentheal .@Castle$,260,376,267,383,100,100;
			areapercentheal .@Castle$,292,376,299,383,100,100;
			if ($@BG11_Team1 == getvariableofnpc(.Defender,"Conquest")) {
				areawarp .@Castle$,260,376,267,383,.@Castle$,.@DX,.@DY;
				areawarp .@Castle$,292,376,299,383,.@Castle$,.@AX,.@AY;
			} else {
				areawarp .@Castle$,260,376,267,383,.@Castle$,.@AX,.@AY;
				areawarp .@Castle$,292,376,299,383,.@Castle$,.@DX,.@DY;
			}

	initnpctimer;
	end;
}
-	script	Conquest_Respawn1	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Conquest1"),"-- Reinforcements entering the Battle of Conquest --",1,0xFFA500;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Conquest1");
	set .@DX,getvariableofnpc(.DX,"Conquest1");
	set .@DY,getvariableofnpc(.DY,"Conquest1");
	set .@AX,getvariableofnpc(.AX,"Conquest1");
	set .@AY,getvariableofnpc(.AY,"Conquest1");

	// schg_cas02 ========================================================================
			areapercentheal .@Castle$,80,301,92,321,100,100;
			areapercentheal .@Castle$,111,301,123,321,100,100;
			if ($@BG11b_Team1 == getvariableofnpc(.Defender,"Conquest1")) {
				areawarp .@Castle$,80,301,92,321,.@Castle$,.@DX,.@DY;
				areawarp .@Castle$,111,301,123,321,.@Castle$,.@AX,.@AY;
			} else {
				areawarp .@Castle$,80,301,92,321,.@Castle$,.@AX,.@AY;
				areawarp .@Castle$,111,301,123,321,.@Castle$,.@DX,.@DY;
			}

	initnpctimer;
	end;
}
-	script	Conquest_Respawn2	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Conquest2"),"-- Reinforcements entering the Battle of Conquest --",1,0xFFA500;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Conquest2");
	set .@DX,getvariableofnpc(.DX,"Conquest2");
	set .@DY,getvariableofnpc(.DY,"Conquest2");
	set .@AX,getvariableofnpc(.AX,"Conquest2");
	set .@AY,getvariableofnpc(.AY,"Conquest2");

	// schg_cas03 ========================================================================
			areapercentheal .@Castle$,18,44,33,55,100,100;
			areapercentheal .@Castle$,82,44,97,55,100,100;
			if ($@BG11c_Team1 == getvariableofnpc(.Defender,"Conquest2")) {
				areawarp .@Castle$,18,44,33,55,.@Castle$,.@DX,.@DY;
				areawarp .@Castle$,82,44,97,55,.@Castle$,.@AX,.@AY;
			} else {
				areawarp .@Castle$,18,44,33,55,.@Castle$,.@AX,.@AY;
				areawarp .@Castle$,82,44,97,55,.@Castle$,.@DX,.@DY;
			}

	initnpctimer;
	end;
}
-	script	Conquest_Respawn3	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Conquest3"),"-- Reinforcements entering the Battle of Conquest --",1,0xFFA500;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Conquest3");
	set .@DX,getvariableofnpc(.DX,"Conquest3");
	set .@DY,getvariableofnpc(.DY,"Conquest3");
	set .@AX,getvariableofnpc(.AX,"Conquest3");
	set .@AY,getvariableofnpc(.AY,"Conquest3");

	// arug_cas01 ========================================================================
			areapercentheal .@Castle$,26,378,33,385,100,100;
			areapercentheal .@Castle$,122,354,129,361,100,100;
			if ($@BG11d_Team1 == getvariableofnpc(.Defender,"Conquest3")) {
				areawarp .@Castle$,26,378,33,385,.@Castle$,.@DX,.@DY;
				areawarp .@Castle$,122,354,129,361,.@Castle$,.@AX,.@AY;
			} else {
				areawarp .@Castle$,26,378,33,385,.@Castle$,.@AX,.@AY;
				areawarp .@Castle$,122,354,129,361,.@Castle$,.@DX,.@DY;
			}

	initnpctimer;
	end;
}
-	script	Conquest_Respawn4	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Conquest4"),"-- Reinforcements entering the Battle of Conquest --",1,0xFFA500;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Conquest4");
	set .@DX,getvariableofnpc(.DX,"Conquest4");
	set .@DY,getvariableofnpc(.DY,"Conquest4");
	set .@AX,getvariableofnpc(.AX,"Conquest4");
	set .@AY,getvariableofnpc(.AY,"Conquest4");

	// arug_cas02 ========================================================================
			areapercentheal .@Castle$,291,336,304,342,100,100;
			areapercentheal .@Castle$,291,305,304,311,100,100;
			if ($@BG11e_Team1 == getvariableofnpc(.Defender,"Conquest4")) {
				areawarp .@Castle$,291,336,304,342,.@Castle$,.@DX,.@DY;
				areawarp .@Castle$,291,305,304,311,.@Castle$,.@AX,.@AY;
			} else {
				areawarp .@Castle$,291,336,304,342,.@Castle$,.@AX,.@AY;
				areawarp .@Castle$,291,305,304,311,.@Castle$,.@DX,.@DY;
			}

	initnpctimer;
	end;
}
-	script	Conquest_Respawn5	FAKE_NPC,{
	end;

OnTimer24000:
	mapannounce getvariableofnpc(.Castle$,"Conquest5"),"-- Reinforcements entering the Battle of Conquest --",1,0xFFA500;
	end;

OnTimer25000:
	set .@Castle$,getvariableofnpc(.Castle$,"Conquest5");
	set .@DX,getvariableofnpc(.DX,"Conquest5");
	set .@DY,getvariableofnpc(.DY,"Conquest5");
	set .@AX,getvariableofnpc(.AX,"Conquest5");
	set .@AY,getvariableofnpc(.AY,"Conquest5");

	// arug_cas02 ========================================================================
			areapercentheal .@Castle$,291,336,304,342,100,100;
			areapercentheal .@Castle$,291,305,304,311,100,100;
			if ($@BG11e_Team1 == getvariableofnpc(.Defender,"Conquest5")) {
				areawarp .@Castle$,291,336,304,342,.@Castle$,.@DX,.@DY;
				areawarp .@Castle$,291,305,304,311,.@Castle$,.@AX,.@AY;
			} else {
				areawarp .@Castle$,291,336,304,342,.@Castle$,.@AX,.@AY;
				areawarp .@Castle$,291,305,304,311,.@Castle$,.@DX,.@DY;
			}

	initnpctimer;
	end;
}

//============================================================
// = Healer
//============================================================
schg_cas06,260,383,5	duplicate(BGHeal)	Therapist in battle#cq1	4_F_SISTER
schg_cas06,299,383,3	duplicate(BGHeal)	Therapist in battle#cq2	4_F_SISTER
schg_cas07,86,315,5	duplicate(BGHeal)	Therapist in battle#cq3	4_F_SISTER
schg_cas07,117,315,3	duplicate(BGHeal)	Therapist in battle#cq4	4_F_SISTER
schg_cas08,23,50,6	duplicate(BGHeal)	Therapist in battle#cq5	4_F_SISTER
schg_cas08,92,50,4	duplicate(BGHeal)	Therapist in battle#cq6	4_F_SISTER
arug_cas06,26,385,5	duplicate(BGHeal)	Therapist in battle#cq7	4_F_SISTER
arug_cas06,122,361,5	duplicate(BGHeal)	Therapist in battle#cq8	4_F_SISTER
arug_cas07,304,342,3	duplicate(BGHeal)	Therapist in battle#cq9	4_F_SISTER
arug_cas07,304,305,1	duplicate(BGHeal)	Therapist in battle#cq0	4_F_SISTER
arug_cas08,310,154,6	duplicate(BGHeal)	Therapist in battle#cqa	4_F_SISTER
arug_cas08,333,154,4	duplicate(BGHeal)	Therapist in battle#cqb	4_F_SISTER

// Flags schg_cas06
// *********************************************************************

schg_cas06,106,302,0	script	LF-01#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("First Gate House:Second Gate House:Cancel")) {
			case 1:
				warp "schg_cas06",19,26;
				end;
			case 2:
				warp "schg_cas06",219,90;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas06,109,302,0	script	LF-02#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-1:Defense Area 1-2:Cancel")) {
			case 1:
				warp "schg_cas06",89,43;
				end;
			case 2:
				warp "schg_cas06",141,45;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas06,112,302,0	script	LF-03#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-1:Defense Area 2-2:Cancel")) {
			case 1:
				warp "schg_cas06",137,54;
				end;
			case 2:
				warp "schg_cas06",102,54;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas06,115,302,0	script	LF-04#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-3:Defense Area 2-4:Cancel")) {
			case 1:
				warp "schg_cas06",94,147;
				end;
			case 2:
				warp "schg_cas06",163,140;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas06,118,302,0	script	LF-05#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-3:Defense Area 2-4:Cancel")) {
			case 1:
				warp "schg_cas06",87,220;
				end;
			case 2:
				warp "schg_cas06",151,220;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas06,121,302,0	script	LF-06#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 3-1:Defense Area 3-2:Cancel")) {
			case 1:
				warp "schg_cas06",100,242;
				end;
			case 2:
				warp "schg_cas06",136,242;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas06,124,302,0	script	LF-07#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Center 1 Area:Center 2 Area:Cancel")) {
			case 1:
				warp "schg_cas06",120,168;
				end;
			case 2:
				warp "schg_cas06",119,211;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas06,127,302,0	script	LF-08#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 1-1:Area 2-1:Area 3-1:Cancel")) {
			case 1:
				warp "schg_cas06",89,43;
				end;
			case 2:
				warp "schg_cas06",94,147;
				end;
			case 3:
				warp "schg_cas06",100,242;
				end;
			case 4:
				close;
		}
	}
	end;
}

schg_cas06,130,302,0	script	LF-09#schg_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 1-2:Area 2-3:Area 3-2:Cancel")) {
			case 1:
				warp "schg_cas06",141,45;
				end;
			case 2:
				warp "schg_cas06",163,140;
				end;
			case 3:
				warp "schg_cas06",136,243;
				end;
			case 4:
				close;
		}
	}
	end;
}

schg_cas06,17,45,0	script	Castle#LF_sc06_1::LF_sc06_1	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Flag Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "schg_cas06",120,290;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

schg_cas06,207,95,0	duplicate(LF_sc06_1)	Castle#LF_sc06_2	HIDDEN_NPC
schg_cas06,99,77,0	duplicate(LF_sc06_1)	Castle#LF_sc06_5	HIDDEN_NPC
schg_cas06,140,77,0	duplicate(LF_sc06_1)	Castle#LF_sc06_6	HIDDEN_NPC
schg_cas06,112,212,0	duplicate(LF_sc06_1)	Castle#LF_sc06_9	HIDDEN_NPC
schg_cas06,127,212,0	duplicate(LF_sc06_1)	Castle#LF_sc06_10	HIDDEN_NPC
schg_cas06,113,238,0	duplicate(LF_sc06_1)	Castle#LF_sc06_11	HIDDEN_NPC
schg_cas06,126,238,0	duplicate(LF_sc06_1)	Castle#LF_sc06_12	HIDDEN_NPC
schg_cas06,95,247,0	duplicate(LF_sc06_1)	Castle#LF_sc06_13	HIDDEN_NPC
schg_cas06,144,247,0	duplicate(LF_sc06_1)	Castle#LF_sc06_14	HIDDEN_NPC

schg_cas06,111,46,4	script	Castle#LF_sc06_3::LF_sc06_2	GUILD_FLAG,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Flag Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "schg_cas06",120,290;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

schg_cas06,129,46,4	duplicate(LF_sc06_2)	Castle#LF_sc06_4	GUILD_FLAG
schg_cas06,109,150,4	duplicate(LF_sc06_2)	Castle#LF_sc06_7	GUILD_FLAG
schg_cas06,130,150,4	duplicate(LF_sc06_2)	Castle#LF_sc06_8	GUILD_FLAG

//============================================================
// = Flags
//============================================================
schg_cas07,143,198,0	script	LF-01#schg_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("First Gate House:Second Gate House:Cancel")) {
			case 1:
				warp "schg_cas07",235,44;
				end;
			case 2:
				warp "schg_cas07",302,233;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas07,143,202,0	script	LF-02#schg_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense area 1-1:Defense area 1-2:Cancel")) {
			case 1:
				warp "schg_cas07",317,83;
				end;
			case 2:
				warp "schg_cas07",359,83;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas07,143,206,0	script	LF-03#schg_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense area 2-1:Defense area 2-2:Cancel")) {
			case 1:
				warp "schg_cas07",283,79;
				end;
			case 2:
				warp "schg_cas07",280,122;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas07,145,208,0	script	LF-04#schg_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense area 3-1:Defense area 3-2:Cancel")) {
			case 1:
				warp "schg_cas07",215,110;
				end;
			case 2:
				warp "schg_cas07",255,215;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas07,180,208,0	script	LF-05#schg_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Center 1 Area:Center 2 Area:Cancel")) {
			case 1:
				warp "schg_cas07",338,153;
				end;
			case 2:
				warp "schg_cas07",213,226;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas07,182,206,0	script	LF-06#schg_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense area 1-1:Defense area 2-1:Defense area 3-1:Cancel")) {
			case 1:
				warp "schg_cas07",317,83;
				end;
			case 2:
				warp "schg_cas07",283,79;
				end;
			case 3:
				warp "schg_cas07",215,110;
				end;
			case 4:
				close;
		}
	}
	end;
}

schg_cas07,182,202,0	script	LF-07#schg_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 1-2:Area 2-2:Area 3-2:Cancel")) {
			case 1:
				warp "schg_cas07",359,83;
				end;
			case 2:
				warp "schg_cas07",280,122;
				end;
			case 3:
				warp "schg_cas07",255,215;
				end;
			case 4:
				close;
		}
	}
	end;
}

schg_cas07,235,222,0	script	Andlangr#LF_sc07_1::LF_sc07_1	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "schg_cas07",136,188;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

schg_cas07,157,136,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_2	HIDDEN_NPC
schg_cas07,168,136,4	duplicate(LF_sc07_1)	Andlangr#LF_sc07_3	HIDDEN_NPC
schg_cas07,320,232,4	duplicate(LF_sc07_1)	Andlangr#LF_sc07_4	HIDDEN_NPC
schg_cas07,295,109,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_5	HIDDEN_NPC
schg_cas07,295,92,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_6	HIDDEN_NPC
schg_cas07,285,90,4	duplicate(LF_sc07_1)	Andlangr#LF_sc07_7	HIDDEN_NPC
schg_cas07,285,190,4	duplicate(LF_sc07_1)	Andlangr#LF_sc07_8	HIDDEN_NPC
schg_cas07,238,66,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_9	HIDDEN_NPC
schg_cas07,230,45,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_10	HIDDEN_NPC
schg_cas07,233,120,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_11	HIDDEN_NPC
schg_cas07,247,120,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_12	HIDDEN_NPC
schg_cas07,261,162,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_13	HIDDEN_NPC
schg_cas07,244,162,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_14	HIDDEN_NPC
schg_cas07,235,207,0	duplicate(LF_sc07_1)	Andlangr#LF_sc07_15	HIDDEN_NPC

schg_cas08,346,211,0	script	LF-01#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("First Gate House:Second Gate House:Cancel")) {
			case 1:
				warp "schg_cas08",262,323;
				end;
			case 2:
				warp "schg_cas08",378,263;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas08,346,207,0	script	LF-02#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Center 1 Area:Cancel")) {
			case 1:
				warp "schg_cas08",306,320;
				end;
			case 2:
				close;
		}
	}
	end;
}

schg_cas08,346,203,0	script	LF-03#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense area 2-1:Defense area 2-2:Cancel")) {
			case 1:
				warp "schg_cas08",309,292;
				end;
			case 2:
				warp "schg_cas08",348,292;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas08,346,199,0	script	LF-04#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Center 2 Area:Cancel")) {
			case 1:
				warp "schg_cas08",266,263;
				end;
			case 2:
				close;
		}
	}
	end;
}

schg_cas08,346,195,0	script	LF-05#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 3-1:Defense Area 3-2:Cancel")) {
			case 1:
				warp "schg_cas08",226,220;
				end;
			case 2:
				warp "schg_cas08",185,249;
				end;
			case 3:
				close;
		}
	}
	end;
}

schg_cas08,346,191,0	script	LF-06#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Center 3 Area:Cancel")) {
			case 1:
				warp "schg_cas08",271,226;
				end;
			case 2:
				close;
		}
	}
	end;
}

schg_cas08,301,213,0	script	LF-07#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 1-2:Area 2-2:Area 3-2:Cancel")) {
			case 1:
				warp "schg_cas08",262,323;
				end;
			case 2:
				warp "schg_cas08",378,263;
				end;
			case 3:
				warp "schg_cas08",306,320;
				end;
			case 4:
				close;
		}
	}
	end;
}

schg_cas08,301,209,0	script	LF-08#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 2-1:Area 2-2:Center 2 Area:Cancel")) {
			case 1:
				warp "schg_cas08",309,292;
				end;
			case 2:
				warp "schg_cas08",348,292;
				end;
			case 3:
				warp "schg_cas08",326,263;
				end;
			case 4:
				close;
		}
	}
	end;
}

schg_cas08,301,194,0	script	LF-09#schg_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 3-1:Area 3-2:Center 3 Area:Cancel")) {
			case 1:
				warp "schg_cas08",226,220;
				end;
			case 2:
				warp "schg_cas08",185,249;
				end;
			case 3:
				warp "schg_cas08",271,226;
				end;
			case 4:
				close;
		}
	}
	end;
}

schg_cas08,335,305,0	script	Vidblainn#LF_sc08_1::LF_sc08_1	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "schg_cas08",308,202;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0)
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

schg_cas08,322,305,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_2	HIDDEN_NPC
schg_cas08,352,248,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_3	HIDDEN_NPC
schg_cas08,320,283,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_4	HIDDEN_NPC
schg_cas08,337,283,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_5	HIDDEN_NPC
schg_cas08,233,320,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_6	HIDDEN_NPC
schg_cas08,207,239,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_7	HIDDEN_NPC
schg_cas08,207,228,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_8	HIDDEN_NPC
schg_cas08,266,206,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_9	HIDDEN_NPC
schg_cas08,266,197,0	duplicate(LF_sc08_1)	Vidblainn#LF_sc08_10	HIDDEN_NPC

schg_cas08,283,206,2	script	Vidblainn#LF_sc08_11::LF_sc08_2	GUILD_FLAG,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "schg_cas08",308,202;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

schg_cas08,283,197,2	duplicate(LF_sc08_2)	Vidblainn#LF_sc08_12	GUILD_FLAG
schg_cas08,332,323,6	duplicate(LF_sc08_2)	Vidblainn#LF_sc08_13	GUILD_FLAG
schg_cas08,343,323,2	duplicate(LF_sc08_2)	Vidblainn#LF_sc08_14	GUILD_FLAG


arug_cas06,74,232,0	script	LF-01#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("First Gate House:Second Gate House:Cancel")) {
			case 1:
				warp "arug_cas06",158,237;
				end;
			case 2:
				warp "arug_cas06",297,248;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas06,77,232,0	script	LF-02#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-1:Defense Area 1-2:Cancel")) {
			case 1:
				warp "arug_cas06",197,144;
				end;
			case 2:
				warp "arug_cas06",245,103;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas06,80,232,0	script	LF-03#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-1:Defense Area 2-2:Cancel")) {
			case 1:
				warp "arug_cas06",256,35;
				end;
			case 2:
				warp "arug_cas06",186,26;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas06,83,232,0	script	LF-04#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-3:Defense Area 2-4:Cancel")) {
			case 1:
				warp "arug_cas06",146,65;
				end;
			case 2:
				warp "arug_cas06",176,111;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas06,86,232,0	script	LF-05#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 3-1:Defense Area 3-2:Cancel")) {
			case 1:
				warp "arug_cas06",94,126;
				end;
			case 2:
				warp "arug_cas06",126,126;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas06,89,232,0	script	LF-06#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 3-3:Defense Area 3-4:Cancel")) {
			case 1:
				warp "arug_cas06",68,171;
				end;
			case 2:
				warp "arug_cas06",105,182;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas06,92,232,0	script	LF-07#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Gate House Fork:Cancel")) {
			case 1:
				warp "arug_cas06",233,130;
				end;
			case 2:
				close;
		}
	}
	end;
}

arug_cas06,95,232,0	script	LF-08#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 1-1:Area 2-1:Area 3-1:Cancel")) {
			case 1:
				warp "arug_cas06",197,144;
				end;
			case 2:
				warp "arug_cas06",256,35;
				end;
			case 3:
				warp "arug_cas06",146,65;
				end;
			case 4:
				close;
		}
	}
	end;
}

arug_cas06,98,232,0	script	LF-09#arug_cas06	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Area 1-2:Area 2-2:Area 3-3:Cancel")) {
			case 1:
				warp "arug_cas06",245,103;
				end;
			case 2:
				warp "arug_cas06",186,26;
				end;
			case 3:
				warp "arug_cas06",68,171;
				end;
			case 4:
				close;
		}
	}
	end;
}

arug_cas06,72,176,0	script	Mardol#LF_ar06_1::LF_ar06_1	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "arug_cas06",67,193;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

arug_cas06,103,186,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_2	HIDDEN_NPC,{
arug_cas06,150,102,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_4	HIDDEN_NPC
arug_cas06,208,68,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_5	HIDDEN_NPC
arug_cas06,249,52,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_6	HIDDEN_NPC
arug_cas06,234,76,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_7	HIDDEN_NPC
arug_cas06,249,76,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_8	HIDDEN_NPC
arug_cas06,204,142,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_9	HIDDEN_NPC
arug_cas06,183,244,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_10	HIDDEN_NPC
arug_cas06,292,219,0	duplicate(LF_ar06_1)	Mardol#LF_ar06_11	HIDDEN_NPC

arug_cas06,92,126,4	script	Mardol#LF_ar06_3::LF_ar06_2	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "arug_cas06",67,193;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

arug_cas06,127,126,4	duplicate(LF_ar06_2)	Mardol#LF_ar06_3	HIDDEN_NPC
arug_cas06,102,120,4	duplicate(LF_ar06_2)	Mardol#LF_ar06_12	GUILD_FLAG
arug_cas06,117,120,4	duplicate(LF_ar06_2)	Mardol#LF_ar06_13	GUILD_FLAG
arug_cas06,233,140,4	duplicate(LF_ar06_2)	Mardol#LF_ar06_14	GUILD_FLAG


arug_cas07,98,270,0	script	LF-01#arug_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("First Gate House:Second Gate House:Cancel")) {
			case 1:
				warp "arug_cas07",10,187;
				end;
			case 2:
				warp "arug_cas07",268,187;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas07,98,266,0	script	LF-02#arug_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-1:Defense Area 1-2:Cancel")) {
			case 1:
				warp "arug_cas07",66,31;
				end;
			case 2:
				warp "arug_cas07",212,31;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas07,98,262,0	script	LF-03#arug_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-3:Defense Area 1-4:Cancel")) {
			case 1:
				warp "arug_cas07",90,120;
				end;
			case 2:
				warp "arug_cas07",188,119;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas07,98,258,0	script	LF-04#arug_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-1:Defense Area 2-2:Cancel")) {
			case 1:
				warp "arug_cas07",119,183;
				end;
			case 2:
				warp "arug_cas07",159,183;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas07,98,253,0	script	LF-05#arug_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-3:Defense Area 2-4:Cancel")) {
			case 1:
				warp "arug_cas07",156,324;
				end;
			case 2:
				warp "arug_cas07",174,372;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas07,98,249,0	script	LF-06#arug_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 3-1:Defense Area 3-2:Cancel")) {
			case 1:
				warp "arug_cas07",28,325;
				end;
			case 2:
				warp "arug_cas07",57,325;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas07,98,245,0	script	LF-07#arug_cas07	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Center 2nd Area:Center 3rd Area:Cancel")) {
			case 1:
				warp "arug_cas07",156,263;
				end;
			case 2:
				warp "arug_cas07",43,354;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas07,216,92,0	script	Cyr#LF_ar07_1::LF_ar07_1	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "arug_cas07",43,256;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

arug_cas07,63,92,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_2	HIDDEN_NPC
arug_cas07,127,133,4	duplicate(LF_ar07_1)	Cyr#LF_ar07_3	HIDDEN_NPC
arug_cas07,152,133,4	duplicate(LF_ar07_1)	Cyr#LF_ar07_4	HIDDEN_NPC
arug_cas07,149,218,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_5	HIDDEN_NPC
arug_cas07,162,218,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_6	HIDDEN_NPC
arug_cas07,128,350,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_7	HIDDEN_NPC
arug_cas07,128,341,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_8	HIDDEN_NPC
arug_cas07,49,317,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_9	HIDDEN_NPC
arug_cas07,30,317,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_10	HIDDEN_NPC
arug_cas07,9,190,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_11	HIDDEN_NPC
arug_cas07,271,190,0	duplicate(LF_ar07_1)	Cyr#LF_ar07_12	HIDDEN_NPC

arug_cas07,129,178,4	script	Cyr#LF_ar07_13::LF_ar07_2	GUILD_FLAG,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "arug_cas07",43,256;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

arug_cas07,149,178,4	duplicate(LF_ar07_2)	Cyr#LF_ar07_14	GUILD_FLAG
arug_cas07,132,38,4	duplicate(LF_ar07_2)	Cyr#LF_ar07_15	GUILD_FLAG
arug_cas07,147,38,4	duplicate(LF_ar07_2)	Cyr#LF_ar07_16	GUILD_FLAG


arug_cas08,122,314,0	script	LF-01#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("First Gate House:Second Gate House:Cancel")) {
			case 1:
				warp "arug_cas08",84,158;
				end;
			case 2:
				warp "arug_cas08",197,136;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas08,125,314,0	script	LF-02#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-1:Defense Area 1-2:Cancel")) {
			case 1:
				warp "arug_cas08",65,94;
				end;
			case 2:
				warp "arug_cas08",211,97;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas08,128,314,0	script	LF-03#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-3:Defense Area 1-4:Cancel")) {
			case 1:
				warp "arug_cas08",112,73;
				end;
			case 2:
				warp "arug_cas08",171,73;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas08,131,314,0	script	LF-04#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-1:Defense Area 2-2:Cancel")) {
			case 1:
				warp "arug_cas08",112,152;
				end;
			case 2:
				warp "arug_cas08",172,152;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas08,134,314,0	script	LF-05#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 2-3:Defense Area 2-4:Cancel")) {
			case 1:
				warp "arug_cas08",120,186;
				end;
			case 2:
				warp "arug_cas08",162,186;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas08,149,314,0	script	LF-06#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 3-1:Defense Area 3-2:Cancel")) {
			case 1:
				warp "arug_cas08",116,235;
				end;
			case 2:
				warp "arug_cas08",164,235;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas08,152,314,0	script	LF-07#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-1:Defense Area 2-1:Defense Area 3-1:Cancel")) {
			case 1:
				warp "arug_cas08",65,94;
				end;
			case 2:
				warp "arug_cas08",112,152;
				end;
			case 3:
				warp "arug_cas08",116,235;
				end;
			case 4:
				close;
		}
	}
	end;
}

arug_cas08,155,314,0	script	LF-08#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-2:Defense Area 2-2:Defense Area 3-2:Cancel")) {
			case 1:
				warp "arug_cas08",211,97;
				end;
			case 2:
				warp "arug_cas08",172,152;
				end;
			case 3:
				warp "arug_cas08",164,235;
				end;
			case 4:
				close;
		}
	}
	end;
}

arug_cas08,158,314,0	script	LF-09#arug_cas08	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Please";
		mes "choose a destination";
		mes "within the stronghold.^000000";
		switch(select("Defense Area 1-4:Defense Area 2-4:Cancel")) {
			case 1:
				warp "arug_cas08",171,73;
				end;
			case 2:
				warp "arug_cas08",162,186;
				end;
			case 3:
				close;
		}
	}
	end;
}

arug_cas08,45,158,0	script	Horn#LF_ar08_01::LF_ar08_01	HIDDEN_NPC,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "arug_cas08",121,318;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

arug_cas08,226,156,0	duplicate(LF_ar08_01)	Horn#LF_ar08_01	HIDDEN_NPC
arug_cas08,134,62,4	duplicate(LF_ar08_01)	Horn#LF_ar08_02	HIDDEN_NPC
arug_cas08,149,62,4	duplicate(LF_ar08_01)	Horn#LF_ar08_03	HIDDEN_NPC
arug_cas08,123,154,0	duplicate(LF_ar08_01)	Horn#LF_ar08_04	HIDDEN_NPC
arug_cas08,160,154,0	duplicate(LF_ar08_01)	Horn#LF_ar08_05	HIDDEN_NPC
arug_cas08,135,205,0	duplicate(LF_ar08_01)	Horn#LF_ar08_06	HIDDEN_NPC
arug_cas08,148,205,0	duplicate(LF_ar08_01)	Horn#LF_ar08_07	HIDDEN_NPC
arug_cas08,134,260,0	duplicate(LF_ar08_01)	Horn#LF_ar08_08	HIDDEN_NPC

arug_cas08,148,103,4	script	Horn#LF_ar08_10::LF_ar08_02	GUILD_FLAG,{
	if (getcharid(4) == getvariableofnpc(.Defender,"Conquest")) {
		mes "^3355FFThis is the Stronghold";
		mes "Teleport Service. Would";
		mes "you like to teleport to";
		mes "the Emperium Center?^000000";
		switch(select("Teleport:Cancel")) {
			case 1:
				warp "arug_cas08",121,318;
				end;
			case 2:
				close;
		}
	}
	end;

OnEmblemConquest:
	if( $@BG11 != 0 )
		flagemblembg getvariableofnpc(.Defender,"Conquest"),1;
	end;
}

arug_cas08,135,103,4	duplicate(LF_ar08_02)	Horn#LF_ar08_11	GUILD_FLAG
arug_cas08,63,51,7	duplicate(LF_ar08_02)	Horn#LF_ar08_12	GUILD_FLAG
arug_cas08,214,51,1	duplicate(LF_ar08_02)	Horn#LF_ar08_13	GUILD_FLAG

// MapFlags
// *********************************************************************

schg_cas06	mapflag	battleground
schg_cas06	mapflag	nomemo
schg_cas06	mapflag	nosave	SavePoint
schg_cas06	mapflag	noteleport
schg_cas06	mapflag	nowarp
schg_cas06	mapflag	nowarpto
schg_cas06	mapflag	noreturn
schg_cas06	mapflag	nobranch
schg_cas06	mapflag	nopenalty
// schg_cas06	mapflag	bg_consume

schg_cas07	mapflag	battleground
schg_cas07	mapflag	nomemo
schg_cas07	mapflag	nosave	SavePoint
schg_cas07	mapflag	noteleport
schg_cas07	mapflag	nowarp
schg_cas07	mapflag	nowarpto
schg_cas07	mapflag	noreturn
schg_cas07	mapflag	nobranch
schg_cas07	mapflag	nopenalty
// schg_cas07	mapflag	bg_consume

schg_cas08	mapflag	battleground
schg_cas08	mapflag	nomemo
schg_cas08	mapflag	nosave	SavePoint
schg_cas08	mapflag	noteleport
schg_cas08	mapflag	nowarp
schg_cas08	mapflag	nowarpto
schg_cas08	mapflag	noreturn
schg_cas08	mapflag	nobranch
schg_cas08	mapflag	nopenalty
// schg_cas08	mapflag	bg_consume

arug_cas06	mapflag	battleground
arug_cas06	mapflag	nomemo
arug_cas06	mapflag	nosave	SavePoint
arug_cas06	mapflag	noteleport
arug_cas06	mapflag	nowarp
arug_cas06	mapflag	nowarpto
arug_cas06	mapflag	noreturn
arug_cas06	mapflag	nobranch
arug_cas06	mapflag	nopenalty
// arug_cas06	mapflag	bg_consume

arug_cas07	mapflag	battleground
arug_cas07	mapflag	nomemo
arug_cas07	mapflag	nosave	SavePoint
arug_cas07	mapflag	noteleport
arug_cas07	mapflag	nowarp
arug_cas07	mapflag	nowarpto
arug_cas07	mapflag	noreturn
arug_cas07	mapflag	nobranch
arug_cas07	mapflag	nopenalty
// arug_cas07	mapflag	bg_consume

arug_cas08	mapflag	battleground
arug_cas08	mapflag	nomemo
arug_cas08	mapflag	nosave	SavePoint
arug_cas08	mapflag	noteleport
arug_cas08	mapflag	nowarp
arug_cas08	mapflag	nowarpto
arug_cas08	mapflag	noreturn
arug_cas08	mapflag	nobranch
arug_cas08	mapflag	nopenalty
// arug_cas08	mapflag	bg_consume
