//===== rAthena Script =======================================
//= War of Emperium (Payon) - Bright Arbor
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.3
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Payon Guild Castle 1
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Corrected flag-in spawn point. [L0ne_W0lf]
//= 1.2 Removed dialog from interior flags. [L0ne_W0lf]
//=     Fixed position of Guild dungeon switch.
//= 1.3 Partially rewrote Script. [Masao]
//============================================================

// Guild Flags (Outside Castle)
//============================================================
-	script	Bright Arbor#1::OutsideFlagsPA1	722,{

	set .@GID, GetCastleData("payg_cas01",1);

	if (.@GID == 0) {
		mes "[ Edict of the Divine Rune-Midgarts Kingdom ]";
		mes " ";
		mes "1. Follow the ordinance of The Divine Rune-Midgarts Kingdom, ";
		mes "We declare that";
		mes "there is no formal master of this castle.";
		mes " ";
		mes "2. To the one who can ";
		mes "overcome all trials";
		mes "and destroy the Emperium,";
		mes "the king will endow the one with";
		mes "ownership of this castle.";
		close;
	}
	if (getcharid(2) == .@GID) {
		mes "[ Echoing Voice ]";
		mes "Brave ones...";
		mes "Do you wish to return to your honorable place?";
		next;
		if(select("Return to the guild castle.:Quit.") == 1) {
			close2;
			if (getcharid(2) == GetCastleData("payg_cas01",1)) {
					warp "payg_cas01",54,144;
				end;
			}
			end;
		}
		close;
	}
	mes "[ Edict of the Divine Rune-Midgarts Kingdom ]";
	mes " ";
	mes "1. Follow the ordinance of The Divine Rune-Midgarts Kingdom, ";
	mes "we approve that this place is in";
	mes "the private prossession of ^ff0000"+GetGuildName(.@GID)+"^000000 Guild.";
	mes " ";
	mes "2. The guild Master of ^ff0000"+GetGuildName(.@GID)+"^000000 Guild is";
	mes "^ff0000"+GetGuildMaster(.@GID)+"^000000";
	mes "If there is anyone who objects to this,";
	mes "prove your strength and honor with a steel blade in your hand.";
	close;

OnRecvCastlePy01:
	FlagEmblem GetCastleData("payg_cas01",1);
	end;
}

pay_gld,125,236,4	duplicate(OutsideFlagsPA1)	Bright Arbor#1-2	722
pay_gld,110,233,4	duplicate(OutsideFlagsPA1)	Bright Arbor#2	722
pay_gld,116,233,4	duplicate(OutsideFlagsPA1)	Bright Arbor#3	722
pay_gld,91,239,2	duplicate(OutsideFlagsPA1)	Bright Arbor#4	722

// Guild Flags (Inside Castle)
//============================================================
-	script	Bright Arbor#5::InsideFlagsPA1	722,{
	end;

OnRecvCastlePy01:
	FlagEmblem GetCastleData("payg_cas01",1);
	end;
}

payg_cas01,238,67,4	duplicate(InsideFlagsPA1)	Bright Arbor#5-2	722
payg_cas01,233,67,4	duplicate(InsideFlagsPA1)	Bright Arbor#6	722
payg_cas01,221,123,4	duplicate(InsideFlagsPA1)	Bright Arbor#7	722
payg_cas01,221,116,4	duplicate(InsideFlagsPA1)	Bright Arbor#8	722
payg_cas01,206,108,4	duplicate(InsideFlagsPA1)	Bright Arbor#9	722
payg_cas01,212,108,4	duplicate(InsideFlagsPA1)	Bright Arbor#10	722

// Guild Flag (Town)
//============================================================
payon,90,322,4	duplicate(InsideFlagsPA1)	Bright Arbor#11	722
payon,166,177,3	duplicate(InsideFlagsPA1)	Bright Arbor#12	722

// Kafra Employee
//============================================================
payg_cas01,128,58,3	duplicate(guildkafra)	Kafra Staff#payg_cas01	117

// Steward
//============================================================
payg_cas01,120,58,4	duplicate(cm)	Kurunnadi#payg_cas01	55

// Treasure Room Exit
//============================================================
payg_cas01,295,8,0	script	#lever_PA01	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "payg_cas01",120,59;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
payg_cas01,101,25,0	duplicate(gdlever)	#payg_cas01	111

// Guild Treasure Spawns
//============================================================
payg_cas01,291,8,0	duplicate(Gld_Trea_Spawn)	Treasure#payg_cas01	-1

// AGIT Manager
//============================================================
payg_cas01,139,139,0	duplicate(Gld_Agit_Manager)	Agit#payg_cas01	-1

// Guardians
//============================================================
payg_cas01,120,58,4	duplicate(Gld_Guard_Template)	Guardian#payg_cas01	-1
