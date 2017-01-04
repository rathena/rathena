//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Wuerzburg
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Al De Baran Guild Castle 4
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Wuerzburg#1::OutsideFlagsA4	722,{

	set .@GID, GetCastleData("aldeg_cas04",1);

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
			if (getcharid(2) == GetCastleData("aldeg_cas04",1)) {
				warp "aldeg_cas04",116,217;
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

OnRecvCastleA04:
	FlagEmblem GetCastleData("aldeg_cas04",1);
	end;
}

alde_gld,239,246,2	duplicate(OutsideFlagsA4)	Wuerzburg#1-2	722
alde_gld,239,239,2	duplicate(OutsideFlagsA4)	Wuerzburg#2	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Wuerzburg#3::InsideFlagsA4	722,{
	end;

OnRecvCastleA04:
	FlagEmblem GetCastleData("aldeg_cas04",1);
	end;
}

aldeg_cas04,167,61,2	duplicate(InsideFlagsA4)	Wuerzburg#3-2	722
aldeg_cas04,164,90,2	duplicate(InsideFlagsA4)	Wuerzburg#4	722
aldeg_cas04,129,193,2	duplicate(InsideFlagsA4)	Wuerzburg#5	722
aldeg_cas04,112,206,2	duplicate(InsideFlagsA4)	Wuerzburg#6	722
aldeg_cas04,113,212,2	duplicate(InsideFlagsA4)	Wuerzburg#7	722
aldeg_cas04,77,117,2	duplicate(InsideFlagsA4)	Wuerzburg#8	722
aldeg_cas04,186,42,2	duplicate(InsideFlagsA4)	Wuerzburg#9	722
aldeg_cas04,30,69,2	duplicate(InsideFlagsA4)	Wuerzburg#10	722
aldeg_cas04,55,97,2	duplicate(InsideFlagsA4)	Wuerzburg#11	722
aldeg_cas04,45,98,2	duplicate(InsideFlagsA4)	Wuerzburg#12	722
aldeg_cas04,33,116,2	duplicate(InsideFlagsA4)	Wuerzburg#13	722
aldeg_cas04,130,180,2	duplicate(InsideFlagsA4)	Wuerzburg#14	722
aldeg_cas04,129,193,2	duplicate(InsideFlagsA4)	Wuerzburg#15	722
aldeg_cas04,33,107,2	duplicate(InsideFlagsA4)	Wuerzburg#16	722
aldeg_cas04,133,220,2	duplicate(InsideFlagsA4)	Wuerzburg#17	722
aldeg_cas04,169,22,2	duplicate(InsideFlagsA4)	Wuerzburg#18	722
aldeg_cas04,169,15,2	duplicate(InsideFlagsA4)	Wuerzburg#19	722

// Guild Flag (Town)
//============================================================
aldebaran,131,97,4	duplicate(InsideFlagsA4)	Wuerzburg#20	722

// Kafra Employee
//============================================================
aldeg_cas04,45,88,0	duplicate(guildkafra)	Kafra Staff#aldeg_cas04	117

// Steward
//============================================================
aldeg_cas04,67,116,0	duplicate(cm)	Brymhensen#aldeg_cas04	55

// Treasure Room Exit
//============================================================
aldeg_cas04,83,17,0	script	#lever_A04	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "aldeg_cas04",67,117;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
aldeg_cas04,76,64,0	duplicate(gdlever)	#aldeg_cas04	111

// Guild Treasure Spawns
//============================================================
aldeg_cas04,84,13,0	duplicate(Gld_Trea_Spawn)	Treasure#aldeg_cas04	-1

// AGIT Manager
//============================================================
aldeg_cas04,36,218,0	duplicate(Gld_Agit_Manager)	Agit#aldeg_cas04	-1

// Guardians
//============================================================
aldeg_cas04,67,116,0	duplicate(Gld_Guard_Template)	Guardian#aldeg_cas04	-1
