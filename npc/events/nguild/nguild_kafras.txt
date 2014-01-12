//===== rAthena Script =======================================
//= War of Emperium Kafras for N Guild Castles
//===== By: ==================================================
//= kobra_k88
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project; RO Episode 5+
//===== Description: =========================================
//= Provides Kafra services for guild members of NGuild Castles.
//= Used in conjuction with function F_Kafra.
//===== Additional Comments: =================================
//= Based off existing guild scripts.  Do not know if it is accurate.[kobra_k88]
//= 1.1 All N Guild Kafras teleport to Prontera only! [Lupus]
//= 1.2 Added Kafra function. [L0ne_W0lf]
//============================================================


function	script	F_GKafra	{
	cutin "kafra_01",2;
	set @GID, GetCastleData(getarg(0),1);
	if (getcharid(2) == @GID && getgdskilllv(@GID,10001)) goto L_StartG;

	mes "[Kafra Service]";
	mes "I am contracted to provide service only for the ^ff0000" + GetGuildName(@GID) + "^000000 Guild. Please use another Kafra Corporation staff member around here. I am Sorry for your inconvenience.";
	cutin "",255;
	close;
L_StartG:
	set @wrpP[0], 200;
	set @wrpD$[0], getarg(1);
	setarray @wrpC$[0], @wrpD$[0]+" ^880000"+@wrpP[0]+"^000000 z", "Cancel", "", "", "","";
	callfunc "F_Kafra",2,0,0,0,800;
	end;
}

// Castle 1 ===============================================
nguild_alde,218,170,0	script	Kafra Service#N01	117,{
	callfunc "F_GKafra", "nguild_alde", "Prontera";
	end;
OnRecvCastleN01:
	if (GetCastleData("nguild_alde",9) < 1) disablenpc "Kafra Service#N01";
	end;
}

// Castle 2 ===============================================
//nguild_gef,96,173,0	script	Kafra Service#N02	117,{
nguild_gef,35,37,0	script	Kafra Service#N02	117,{
	callfunc "F_GKafra", "nguild_gef", "Prontera";
	end;
OnRecvCastleN02:
	if (GetCastleData("nguild_gef",9) < 1) disablenpc "Kafra Service#N02";
	end;
}

// Castle 3 ===============================================
nguild_pay,128,58,3	script	Kafra Service#N03	117,{
	callfunc "F_GKafra", "nguild_pay", "Prontera";
	end;
OnRecvCastleN03:
	if (GetCastleData("nguild_pay",9) < 1) disablenpc "Kafra Service#N03";
	end;
}

// Castle 4 ===============================================
nguild_prt,96,173,0	script	Kafra Service#N04	117,{
	callfunc "F_GKafra", "nguild_prt", "Prontera";
	end;
OnRecvCastleN04:
	if (GetCastleData("nguild_prt",9) < 1) disablenpc "Kafra Service#N04";
	end;
}
