//===== rAthena Script =======================================
//= Battleground Extended + : Tierra Bossnia
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
-	script	Tierra_Boss	-1,{
	end;

OnGuillaumeQuit:
OnCroixQuit:
	bg_desert;
	end;

OnTeam1Active:
	warp "bat_a03",353,344;
	end;
OnTeam2Active:
	warp "bat_a03",353,52;
	end;

OnReady:
	//if ($@BG16)
	//	end;

	set $@BG16, 1;
	initnpctimer;
	set $@score_bossnia_blue, 0;
	set $@score_bossnia_red, 0;
	set .Neutral_Base, 0;
	sleep 2000;
	bg_warp $@BG16_Team1,"bat_a03",353,344;
	bg_warp $@BG16_Team2,"bat_a03",353,52;
	sleep 2000;
	donpcevent "#gtb_respawn::OnBGStart";
	donpcevent "#ctb_respawn::OnBGStart";
	bg_updatescore "bat_a03",$@score_bossnia_red,$@score_bossnia_blue;
	sleep 3000;
	donpcevent "Balance_Flag::OnBGStart";
	donpcevent "Guillaume_Boss::OnSummon";
	donpcevent "Croix_Boss::OnSummon";
	mapannounce "bat_a03","-- Capture the Balance Flag to remove immunity of Enemy Guardians --",1,0x483D8B;
	end;

OnNeutralReset:
	set .Neutral_Base, 0;
	killmonster "bat_a03","Tierra_Boss::OnNeutralBreak";
	bg_monster_immunity getvariableofnpc(.Boss,"Guillaume_Boss"),1;
	bg_monster_immunity getvariableofnpc(.Boss,"Croix_Boss"),1;
	bg_monster 0,"bat_a03",273,204,"Balance Flag",1911,"Tierra_Boss::OnNeutralBreak";
	sleep 2000;
	if ($@BG16 != 1)
		end;
	mapannounce "bat_a03","-- Balance Flag returned to normality --",1,0x483D8B;
	end;

OnNeutralBreak:
	if ($@BG16 != 1)
		end;
	set .Neutral_Base, getcharid(4);
	if (getcharid(4) == $@BG16_Team1) {
		mapannounce "bat_a03","-- Guillaume captured the Balance Flag. Croix guardian is now vulnerable to attacks!! --",1,0x0000FF;
		bg_monster_immunity getvariableofnpc(.Boss,"Guillaume_Boss"),1;
		bg_monster_immunity getvariableofnpc(.Boss,"Croix_Boss"),0;
		bg_monster .Neutral_Base,"bat_a03",273,204,"Guillaume Flag",1912,"Tierra_Boss::OnNeutralBreak";
	} else if (getcharid(4) == $@BG16_Team2) {
		mapannounce "bat_a03","-- Croix captured the Balance Flag. Guillaume guardian is now vulnerable to attacks!! --",1,0xFF0000;
		bg_monster_immunity getvariableofnpc(.Boss,"Guillaume_Boss"),0;
		bg_monster_immunity getvariableofnpc(.Boss,"Croix_Boss"),1;
		bg_monster .Neutral_Base,"bat_a03",273,204,"Croix Flag",1913,"Tierra_Boss::OnNeutralBreak";
	} else {
		set .Neutral_Base, 0;
		bg_monster_immunity getvariableofnpc(.Boss,"Guillaume_Boss"),1;
		bg_monster_immunity getvariableofnpc(.Boss,"Croix_Boss"),1;
		bg_monster 0,"bat_a03",273,204,"Balance Flag",1911,"Tierra_Boss::OnNeutralBreak";
	}
	end;

OnTimer600000:
	mapannounce "bat_a03","Battle of Tierra Bossnia will end in 5 minutes",1,0x483D8B;
	end;
OnTimer840000:
	mapannounce "bat_a03","Battle of Tierra Bossnia will end in 1 minute",1,0x483D8B;
	end;

OnTimer900000:
OnMatchEnd:
	stopnpctimer;
	stopnpctimer "Guillaume_Boss";
	stopnpctimer "Croix_Boss";
	donpcevent "#gtb_respawn::OnBGStop";
	donpcevent "#ctb_respawn::OnBGStop";
	donpcevent "Balance_Flag::OnBGStop";
	killmonster "bat_a03","Guillaume_Boss::OnBoss";
	killmonster "bat_a03","Croix_Boss::OnBoss";
	set $@BG16, 2;
	// =======================================================
	// Team Rewards
	// =======================================================
	if ($@score_bossnia_blue < $@score_bossnia_red) {
		callfunc "BG_get_Rewards",$@BG16_Team1,7828,5;
		callfunc "BG_get_Rewards",$@BG16_Team2,7828,1;
		mapannounce "bat_a03","The Guillaume army has won the Battle of Tierra Bossnia!",1,0x0000FF;
	} else if ($@score_bossnia_blue > $@score_bossnia_red) {
		callfunc "BG_get_Rewards",$@BG16_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG16_Team2,7828,5;
		mapannounce "bat_a03","The Croix army has won the Battle of Tierra Bossnia!",1,0xFF0000;
	} else {
		callfunc "BG_get_Rewards",$@BG16_Team1,7828,1;
		callfunc "BG_get_Rewards",$@BG16_Team2,7828,1;
		mapannounce "bat_a03","The battle is over. This is a Tie...!",1,0x483D8B;
	}
	// =======================================================
	set .Neutral_Base, 0;
	set $@score_bossnia_blue, 0;
	set $@score_bossnia_red, 0;
	bg_updatescore "bat_a03",$@score_bossnia_red,$@score_bossnia_blue;
	$@BG16 = 0;
	mapwarp "bat_a03","bat_room",154,150;
	if($@BG16_Team1) { bg_destroy $@BG16_Team1; $@BG16_Team1 = 0; }
	if($@BG16_Team2) { bg_destroy $@BG16_Team2; $@BG16_Team2 = 0; }
	bg_unbook "bat_a03";
	end;
}

-	script	Guillaume_Boss	-1,{
	end;

OnSummon:
	set .@Boss, 2100 + $@score_bossnia_blue;
	mapannounce "bat_a03","-- Guillaume Guardian [" + strmobinfo(2,.@Boss) + "] entering the battle --",1,0x0000FF;
	set .Boss, bg_monster($@BG16_Team1,"bat_a03",176,345,"Guillaume Guardian",.@Boss,"Guillaume_Boss::OnBoss");
	bg_monster_immunity .Boss,1;
	initnpctimer;
	end;

OnTimer2000:
	stopnpctimer;
	if (.Boss) {
		bg_monster_reveal .Boss,1,0x0000FF;
		initnpctimer;
	}
	end;

OnBoss:
	stopnpctimer;
	bg_monster_reveal .Boss,2,0x0000FF; // Hide Spot
	$@score_bossnia_blue++;
	bg_updatescore "bat_a03",$@score_bossnia_red,$@score_bossnia_blue;
	set .Boss, 0;
	if ($@score_bossnia_blue > 4)
		donpcevent "Tierra_Boss::OnMatchEnd";
	else
	{
		donpcevent "Tierra_Boss::OnNeutralReset";
		donpcevent "Guillaume_Boss::OnSummon";
	}
	end;
}

-	script	Croix_Boss	-1,{
	end;

OnSummon:
	set .@Boss, 2100 + $@score_bossnia_red;
	mapannounce "bat_a03","-- Croix Guardian [" + strmobinfo(2,.@Boss) + "] entering the battle --",1,0xFF0000;
	set .Boss, bg_monster($@BG16_Team2,"bat_a03",167,50,"Croix Guardian",.@Boss,"Croix_Boss::OnBoss");
	bg_monster_immunity .Boss,1;
	initnpctimer;
	end;

OnTimer2000:
	stopnpctimer;
	if (.Boss) {
		bg_monster_reveal .Boss,1,0xFF0000;
		initnpctimer;
	}
	end;

OnBoss:
	stopnpctimer;
	bg_monster_reveal .Boss,2,0xFF0000; // Hide Spot
	$@score_bossnia_red++;
	bg_updatescore "bat_a03",$@score_bossnia_red,$@score_bossnia_blue;
	set .Boss, 0;
	if ($@score_bossnia_red > 4)
		donpcevent "Tierra_Boss::OnMatchEnd";
	else
	{
		donpcevent "Tierra_Boss::OnNeutralReset";
		donpcevent "Croix_Boss::OnSummon";
	}
	end;
}

-	script	Balance_Flag	-1,{
	end;

OnBGStart:
	viewpointmap "bat_a03",1,273,204,1,0xFFFFFF;
	bg_monster 0,"bat_a03",273,204,"Balance Flag",1911,"Tierra_Boss::OnNeutralBreak";
	initnpctimer;
	end;

OnBGStop:
	stopnpctimer;
	viewpointmap "bat_a03",2,273,204,1,0xFFFFFF;
	killmonster "bat_a03","Tierra_Boss::OnNeutralBreak";
	end;

OnTimer2000:
	initnpctimer;
	set .@NB, getvariableofnpc(.Neutral_Base,"Tierra_Boss");
	if (.@NB == $@BG16_Team1)
		viewpointmap "bat_a03",1,273,204,1,0x0000FF;
	else if (.@NB == $@BG16_Team2)
		viewpointmap "bat_a03",1,273,204,1,0xFF0000;
	else
		viewpointmap "bat_a03",1,273,204,1,0xFFFFFF;
	end;
}

// ===========================================================
// Map Flags
// ===========================================================
bat_a03	mapflag	battleground	2
bat_a03	mapflag	nomemo
bat_a03	mapflag	nosave	SavePoint
bat_a03	mapflag	noteleport
bat_a03	mapflag	monster_noteleport
bat_a03	mapflag	nowarp
bat_a03	mapflag	nowarpto
bat_a03	mapflag	noreturn
bat_a03	mapflag	nobranch
bat_a03	mapflag	nopenalty

// ===========================================================
// Flags
// ===========================================================
bat_a03,148,85,1	duplicate(Base Flag#bg)	Omega Base#tb_1	974
bat_a03,155,85,1	duplicate(Base Flag#bg)	Omega Base#tb_2	974
bat_a03,357,75,1	duplicate(Base Flag#bg)	Omega Base#tb_3	974
bat_a03,348,74,1	duplicate(Base Flag#bg)	Omega Base#tb_4	974
bat_a03,199,49,1	duplicate(Base Flag#bg)	Omega Base#tb_5	974
bat_a03,168,16,1	duplicate(Base Flag#bg)	Omega Base#tb_6	974
bat_a03,138,12,1	duplicate(Base Flag#bg)	Omega Base#tb_7	974
bat_a03,108,35,1	duplicate(Base Flag#bg)	Omega Base#tb_8	974
bat_a03,164,308,1	duplicate(Base Flag#bg)	Alpha Base#tb_1	973
bat_a03,157,308,1	duplicate(Base Flag#bg)	Alpha Base#tb_2	973
bat_a03,359,327,1	duplicate(Base Flag#bg)	Alpha Base#tb_3	973
bat_a03,350,326,1	duplicate(Base Flag#bg)	Alpha Base#tb_4	973
bat_a03,209,344,1	duplicate(Base Flag#bg)	Alpha Base#tb_5	973
bat_a03,173,380,1	duplicate(Base Flag#bg)	Alpha Base#tb_6	973
bat_a03,150,380,1	duplicate(Base Flag#bg)	Alpha Base#tb_7	973
bat_a03,118,357,1	duplicate(Base Flag#bg)	Alpha Base#tb_8	973
bat_a03,119,336,1	duplicate(Base Flag#bg)	Alpha Base#tb_9	973

// ===========================================================
// Respawn
// ===========================================================
bat_a03,50,374,0	script	#gtb_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "bat_a03",46,370,54,378,100,100;
	areawarp "bat_a03",46,370,54,378,"bat_a03",354,340;
	initnpctimer;
	end;
}

bat_a03,42,16,0	script	#ctb_respawn	HIDDEN_WARP_NPC,{
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
	areapercentheal "bat_a03",38,12,47,21,100,100;
	areawarp "bat_a03",38,12,47,21,"bat_a03",354,57;
	initnpctimer;
	end;
}