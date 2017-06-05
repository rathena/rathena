//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Rothenburg
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Al De Baran Guild Castle 5
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Rothenburg#1::OutsideFlagsA5	722,{

	set .@GID, GetCastleData("aldeg_cas05",1);

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
			if (getcharid(2) == GetCastleData("aldeg_cas05",1)) {
				warp "aldeg_cas05",167,225;
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

OnRecvCastleA05:
	FlagEmblem GetCastleData("aldeg_cas05",1);
	end;
}

alde_gld,265,93,6	duplicate(OutsideFlagsA5)	Rothenburg#1-2	722
alde_gld,265,87,6	duplicate(OutsideFlagsA5)	Rothenburg#2	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Rothenburg#3::InsideFlagsA5	722,{
	end;

OnRecvCastleA05:
	FlagEmblem GetCastleData("aldeg_cas05",1);
	end;
}

aldeg_cas05,170,85,2	duplicate(InsideFlagsA5)	Rothenburg#3-2	722
aldeg_cas05,142,212,2	duplicate(InsideFlagsA5)	Rothenburg#4	722
aldeg_cas05,149,196,2	duplicate(InsideFlagsA5)	Rothenburg#5	722
aldeg_cas05,41,180,2	duplicate(InsideFlagsA5)	Rothenburg#6	722
aldeg_cas05,38,201,2	duplicate(InsideFlagsA5)	Rothenburg#7	722
aldeg_cas05,65,182,2	duplicate(InsideFlagsA5)	Rothenburg#8	722
aldeg_cas05,65,205,2	duplicate(InsideFlagsA5)	Rothenburg#9	722
aldeg_cas05,10,218,2	duplicate(InsideFlagsA5)	Rothenburg#10	722
aldeg_cas05,10,218,2	duplicate(InsideFlagsA5)	Rothenburg#11	722
aldeg_cas05,164,201,2	duplicate(InsideFlagsA5)	Rothenburg#12	722
aldeg_cas05,14,117,2	duplicate(InsideFlagsA5)	Rothenburg#13	722
aldeg_cas05,10,225,2	duplicate(InsideFlagsA5)	Rothenburg#14	722
aldeg_cas05,187,59,2	duplicate(InsideFlagsA5)	Rothenburg#15	722
aldeg_cas05,154,51,2	duplicate(InsideFlagsA5)	Rothenburg#16	722
aldeg_cas05,22,211,2	duplicate(InsideFlagsA5)	Rothenburg#17	722
aldeg_cas05,150,202,2	duplicate(InsideFlagsA5)	Rothenburg#18	722

// Guild Flag (Town)
//============================================================
aldebaran,128,97,4	duplicate(InsideFlagsA5)	Rothenburg#19	722

// Kafra Employee
//============================================================
aldeg_cas05,31,190,0	duplicate(guildkafra)	Kafra Staff#aldeg_cas05	117

// Steward
//============================================================
aldeg_cas05,51,179,0	duplicate(cm)	Esmarehk#aldeg_cas05	55

// Treasure Room Exit
//============================================================
aldeg_cas05,64,8,0	script	#lever_A05	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "aldeg_cas05",51,179;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
aldeg_cas05,22,205,0	duplicate(gdlever)	#aldeg_cas05	111

// Guild Treasure Spawns
//============================================================
aldeg_cas05,62,12,0	duplicate(Gld_Trea_Spawn)	Treasure#aldeg_cas05	-1

// AGIT Manager
//============================================================
aldeg_cas05,28,102,0	duplicate(Gld_Agit_Manager)	Agit#aldeg_cas05	-1

// Guardians
//============================================================
aldeg_cas05,52,179,0	duplicate(Gld_Guard_Template)	Guardian#aldeg_cas05	-1
