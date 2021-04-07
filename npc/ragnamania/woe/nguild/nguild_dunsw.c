//===== rAthena Script =======================================
//= War of Emperium Dungeon Switch for NGuild Castles
//===== By: ==================================================
//= kobra_k88
//===== Current Version: =====================================
//= 1.1
//===== Compatible With: =====================================
//= rAthena Project; RO Episode 4+
//===== Description: =========================================
//= Switch that warps guild members to the guild dungeon
//===== Additional Comments: =================================
//= Based off existing guild scripts.  Do not know if it is accurate.[kobra_k88]
//= 1.1 Guild Dungeon Switch Fucntion added. [L0ne_W0lf]
//============================================================


//==================================================
function	script	F_GldDunSw	{
	set .@GID, GetCastleData(getarg(0),1);
	if (.@GID == 0) {
		mes "[ Echoing Voice ]";
		mes " ' The one who can overcome an ordeal and show true bravery... will find the way... ' ";
		close;
	}
	else {
		mes "[ Echoing Voice ]";
		mes " ' Only the one who can show true bravery can take this test. '";
		next;
		mes " ";
		mes "There's a small lever. Will you pull it?";
		next;

		if (select("Pull.:Do not.") == 1) {
			if (getcharid(2) == .@GID) {
				warp "gld_dun"+getarg(1),getarg(2),getarg(3);
				end;
			}
			mes " ";
			mes " Nothing happened.";
		}
		return;
	}
}

// Castle 1 ===============================================
nguild_alde,212,181,0	script	Switch#DunN01	111,{
	callfunc "F_GldDunSw","nguild_alde","02",32,122;
	close;
}

// Castle 2 ===============================================
nguild_gef,78,84,0	script	Switch#DunN02	111,{
	callfunc "F_GldDunSw","nguild_gef","04",39,258;
	close;
}

// Castle 3 ===============================================
nguild_pay,101,25,0	script	Switch#DunN03	111,{
	callfunc "F_GldDunSw","nguild_pay","01",186,165;
	close;
}

// Castle 4 ===============================================
nguild_prt,94,200,0	script	Switch#DunN04	111,{
	callfunc "F_GldDunSw","nguild_prt","03",28,251;
	close;
}
