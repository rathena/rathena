//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Hohenschwangau
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Al De Baran Guild Castle 2
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flags (Outside Castle)
//============================================================
-	script	Hohenschwangau#1::OutsideFlagsA2	722,{

	set .@GID, GetCastleData("aldeg_cas02",1);

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
			if (getcharid(2) == GetCastleData("aldeg_cas02",1)) {
				warp "aldeg_cas02",220,190;
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

OnRecvCastleA02:
	FlagEmblem GetCastleData("aldeg_cas02",1);
	end;
}

alde_gld,99,251,4	duplicate(OutsideFlagsA2)	Hohenschwangau#1-2	722
alde_gld,99,244,4	duplicate(OutsideFlagsA2)	Hohenschwangau#2	722

// Guild Flags (Inside Castle)
//============================================================
-	script	Hohenschwangau#3::InsideFlagsA2	722,{
	end;

OnRecvCastleA02:
	FlagEmblem GetCastleData("aldeg_cas02",1);
	end;
}

aldeg_cas02,82,71,2	duplicate(InsideFlagsA2)	Hohenschwangau#3-2	722
aldeg_cas02,67,30,2	duplicate(InsideFlagsA2)	Hohenschwangau#4	722
aldeg_cas02,183,140,2	duplicate(InsideFlagsA2)	Hohenschwangau#5	722
aldeg_cas02,212,152,2	duplicate(InsideFlagsA2)	Hohenschwangau#6	722
aldeg_cas02,108,39,2	duplicate(InsideFlagsA2)	Hohenschwangau#7	722
aldeg_cas02,57,213,2	duplicate(InsideFlagsA2)	Hohenschwangau#8	722
aldeg_cas02,103,53,2	duplicate(InsideFlagsA2)	Hohenschwangau#9	722
aldeg_cas02,73,53,2	duplicate(InsideFlagsA2)	Hohenschwangau#10	722
aldeg_cas02,63,41,2	duplicate(InsideFlagsA2)	Hohenschwangau#11	722
aldeg_cas02,229,6,2	duplicate(InsideFlagsA2)	Hohenschwangau#12	722
aldeg_cas02,230,40,2	duplicate(InsideFlagsA2)	Hohenschwangau#13	722
aldeg_cas02,197,40,2	duplicate(InsideFlagsA2)	Hohenschwangau#14	722
aldeg_cas02,32,213,2	duplicate(InsideFlagsA2)	Hohenschwangau#15	722
aldeg_cas02,121,29,2	duplicate(InsideFlagsA2)	Hohenschwangau#16	722

// Guild Flag (Town)
//============================================================
aldebaran,149,97,4	duplicate(InsideFlagsA2)	Hohenschwangau#17	722

// Kafra Employee
//============================================================
aldeg_cas02,84,74,0	duplicate(guildkafra)	Kafra Staff#aldeg_cas02	117

// Steward
//============================================================
aldeg_cas02,78,74,0	duplicate(cm)	Chenchenlie#aldeg_cas02	55

// Treasure Room Lever
//============================================================
aldeg_cas02,139,234,0	script	#lever_A02	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "aldeg_cas02",78,75;
		end;
	}
	close;
}

// Guild Dungeon Lever
//============================================================
aldeg_cas02,194,136,0	duplicate(gdlever)	#aldeg_cas02	111

// Guild Treasure Spawns
//============================================================
aldeg_cas02,135,230,0	duplicate(Gld_Trea_Spawn)	Treasure#aldeg_cas02	-1

// AGIT Manager
//============================================================
aldeg_cas02,214,24,0	duplicate(Gld_Agit_Manager)	Agit#aldeg_cas02	-1

// Guardians
//============================================================
aldeg_cas02,78,74,0	duplicate(Gld_Guard_Template)	Guardian#aldeg_cas02	-1
