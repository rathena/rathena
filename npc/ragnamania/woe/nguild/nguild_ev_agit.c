//===== rAthena Script =======================================
//= War of Emperium - NGuild Wars Events
//===== By: ==================================================
//= kobra_k88
//===== Current Version: =====================================
//= 1.5
//===== Compatible With: =====================================
//= rAthena Project; RO Episode 4+
//===== Description: =========================================
//= Event Triggers for NGuild Wars
//===== Additional Comments: =================================
// Based off existing guild scripts.  Do not know if it is accurate.[kobra_k88]
//= 1.3 Added code for abandoning captured castles on /breakguild [Lupus]
//= 1.4 AGIT Functions added, treasure spawning added. [L0ne_W0lf]
//= 1.5 Emperium should now properly respawn. [L0ne_W0lf]
//============================================================


// Function for OnAgitStart =========================================
function	script	F_AgitStart	{
	set .@map$, getarg(0);
	set .@castle$, getarg(1);
	set .@empx, getarg(2);
	set .@empy, getarg(3);

	MapRespawnGuildID .@map$,GetCastleData(.@map$,1),2;
	monster .@map$,.@empx,.@empy,"Emperium",1288,1,"Agit_"+.@castle$+"::OnAgitBreak";
	GvgOn .@map$;
	if (GetCastleData(.@map$,1) != 0) return;
	end;
}

// Function for OnGuildBreak ======================================
function	script	F_GuildBreak	{
	set .@map$, getarg(0);
	set .@castle$, getarg(1);

	killmonsterall .@map$;

	Announce "Guild Base [" + GetCastleName(.@map$) + "] has been abandoned.",0;
	disablenpc "Kafra Staff#"+.@castle$;

	SetCastleData .@map$,0,0;
	return;
}

// Function for OnAgitBreak ======================================
function	script	F_AgitBreak	{
	set .@map$, getarg(0);
	set .@castle$, getarg(1);

	set .@GID,getcharid(2);
	if (.@GID <= 0) return;

	set .@Economy,GetCastleData(.@map$,2) - 5;
	if (.@Economy < 0) set .@Economy, 0;
	SetCastleData .@map$, 2, .@Economy;

	set .@Defence,GetCastleData(.@map$,3) - 5;
	if (.@Defence < 0) set .@Defence, 0;
	SetCastleData .@map$, 3, .@Defence;

	SetCastleData .@map$,1, .@GID;
	MapAnnounce .@map$,"The emperium has been destroyed.",bc_map,0x00CCFF;
	Announce "The [" + GetCastleName(.@map$) + "] castle has been conquered by the [" + GetGuildName(.@GID) + "] guild.",bc_all;
	donpcevent "::OnRecvCastle"+.@castle$;

	disablenpc "Kafra Staff#"+.@castle$;

	// remove investment data and kafra
	for( set .@i, 4; .@i <= 9; set .@i, .@i+1 )
		SetCastleData .@map$, .@i, 0;

	// if the new guild doesn't have Guardian Research, erase guardians
	if( getgdskilllv(.@GID,10002) == 0 )
		for( set .@i, 10; .@i <= 17; set .@i, .@i+1 )
			SetCastleData .@map$, .@i, 0;

	return;
}


// Function for OnAgitEnd ======================================
function	script	F_AgitEnd	{
	set .@map$, getarg(0);
	set .@castle$, getarg(1);

	GvgOff .@map$;
	// Disable the following if statment to keep empty
	// castles from being aquired after after WoE ends.
	if (GetCastleData(.@map$,1) == 0) return;
	MapRespawnGuildID .@map$,GetCastleData(.@map$,1),4;
	KillMonster .@map$,"Agit_"+.@castle$+"::OnAgitBreak";
	end;
}

// Castle 1 ================================================================
nguild_alde,0,0,0	script	Agit_N01	-1,{
OnInterIfInitOnce:
	donpcevent "::OnRecvCastleN01";
	end;
OnRecvCastleN01:
	RequestGuildInfo GetCastleData("nguild_alde",1);
	end;
OnAgitStart:
	callfunc "F_AgitStart","nguild_alde","N01",216,24;
	end;
OnAgitBreak:
	callfunc "F_AgitBreak","nguild_alde","N01";
	goto OnAgitEliminate;
	end;
OnGuildBreak:
	callfunc "F_GuildBreak","nguild_alde","N01";
	end;
OnAgitEliminate:
	MapRespawnGuildID "nguild_alde",GetCastleData("nguild_alde",1),6;
	Monster "nguild_alde",216,24,"EMPERIUM",1288,1,"Agit_N01::OnAgitBreak";
	end;
OnAgitEnd:
	callfunc "F_AgitEnd","nguild_alde","N01";
	end;
}

// Castle 2 ================================================================
nguild_gef,0,0,0	script	Agit_N02	-1,{
OnInterIfInitOnce:
	donpcevent "::OnRecvCastleN02";
	end;
OnRecvCastleN02:
	RequestGuildInfo GetCastleData("nguild_gef",1);
	end;
OnAgitStart:
	callfunc "F_AgitStart","nguild_gef","N02",198,182;
	end;
OnAgitBreak:
	callfunc "F_AgitBreak","nguild_gef","N02";
	goto OnAgitEliminate;
	end;
OnGuildBreak:
	callfunc "F_GuildBreak","nguild_gef","N02";
	end;
OnAgitEliminate:
	MapRespawnGuildID "nguild_gef",GetCastleData("nguild_gef",1),6;
	Monster "nguild_gef",198,182,"EMPERIUM",1288,1,"Agit_N02::OnAgitBreak";
	end;
OnAgitEnd:
	callfunc "F_AgitEnd","nguild_gef","N02";
	end;
}

// Castle 3 ================================================================
nguild_pay,0,0,0	script	Agit_N03	-1,{
OnInterIfInitOnce:
	donpcevent "::OnRecvCastleN03";
	end;
OnRecvCastleN03:
	RequestGuildInfo GetCastleData("nguild_pay",1);
	end;
OnAgitStart:
	callfunc "F_AgitStart","nguild_pay","N03",139,139;
	end;
OnAgitBreak:
	callfunc "F_AgitBreak","nguild_pay","N03";
	goto OnAgitEliminate;
	end;
OnGuildBreak:
	callfunc "F_GuildBreak","nguild_pay","N03";
	end;
OnAgitEliminate:
	MapRespawnGuildID "nguild_pay",GetCastleData("nguild_pay",1),6;
	Monster "nguild_pay",139,139,"EMPERIUM",1288,1,"Agit_N03::OnAgitBreak";
	end;
OnAgitEnd:
	callfunc "F_AgitEnd","nguild_pay","N03";
	end;
}

// Castle 4 ================================================================
nguild_prt,0,0,0	script	Agit_N04	-1,{
OnInterIfInitOnce:
	donpcevent "::OnRecvCastleN04";
	end;
OnRecvCastleN04:
	RequestGuildInfo GetCastleData("nguild_prt",1);
	end;
OnAgitStart:
	callfunc "F_AgitStart","nguild_prt","N04",197,197;
	end;
OnAgitBreak:
	callfunc "F_AgitBreak","nguild_prt","N04";
	goto OnAgitEliminate;
	end;
OnGuildBreak:
	callfunc "F_GuildBreak","nguild_prt","N04";
	end;
OnAgitEliminate:
	MapRespawnGuildID "nguild_prt",GetCastleData("nguild_prt",1),6;
	Monster "nguild_prt",197,197,"EMPERIUM",1288,1,"Agit_N04::OnAgitBreak";
	end;
OnAgitEnd:
	callfunc "F_AgitEnd","nguild_prt","N04";
	end;
}

// Treasure Spawn Time
//========================================
-	script	TreasSpawn	-1,{
	end;

OnClock0005:
	callfunc "F_GldTreas","nguild_alde","N01",$boxNumN01,$@bxN01,$@boxIdN01,1324,114,218,123,227,0;
	callfunc "F_GldTreas","nguild_gef","N02",$boxNumN02,$@bxN02,$@boxIdN02,1334,150,108,158,114,0;
	callfunc "F_GldTreas","nguild_pay","N03",$boxNumN03,$@bxN03,$@boxIdN03,1344,286,4,295,13,0;
	callfunc "F_GldTreas","nguild_prt","N04",$boxNumN04,$@bxN04,$@boxIdN04,1354,6,204,15,213,0;
	end;
}
