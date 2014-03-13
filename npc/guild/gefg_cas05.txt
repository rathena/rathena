//===== rAthena Script =======================================
//= War of Emperium (Geffen) - Mersetzdeitz
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Geffen Guild Castle 5
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Mersetzdeitz#1::OutsideFlagsG5	722,{

	set .@GID, GetCastleData("gefg_cas05",1);

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
			if (getcharid(2) == GetCastleData("gefg_cas05",1)) {
				warp "gefg_cas05",168,31;
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

OnRecvCastleG05:
	FlagEmblem GetCastleData("gefg_cas05",1);
	end;
}

gef_fild13,302,87,7	duplicate(OutsideFlagsG5)	Mersetzdeitz#1-2	722
gef_fild13,313,83,0	duplicate(OutsideFlagsG5)	Mersetzdeitz#2	722
gef_fild13,252,51,2	duplicate(OutsideFlagsG5)	Mersetzdeitz#3	722
gef_fild13,26,147,2	duplicate(OutsideFlagsG5)	Mersetzdeitz#4	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Mersetzdeitz#5::InsideFlagsG5	722,{
	end;

OnRecvCastleG05:
	FlagEmblem GetCastleData("gefg_cas05",1);
	end;
}

gefg_cas05,77,185,7	duplicate(InsideFlagsG5)	Mersetzdeitz#5-2	722
gefg_cas05,92,181,0	duplicate(InsideFlagsG5)	Mersetzdeitz#6	722
gefg_cas05,83,158,1	duplicate(InsideFlagsG5)	Mersetzdeitz#7	722
gefg_cas05,62,144,7	duplicate(InsideFlagsG5)	Mersetzdeitz#8	722
gefg_cas05,62,66,4	duplicate(InsideFlagsG5)	Mersetzdeitz#9	722
gefg_cas05,69,66,4	duplicate(InsideFlagsG5)	Mersetzdeitz#10	722

// Guild Flag (Town)
//============================================================
geffen,131,123,6	duplicate(InsideFlagsG5)	Mersetzdeitz#11	722

// Kafra Employee
//============================================================
gefg_cas05,61,52,5	duplicate(guildkafra)	Kafra Staff#gefg_cas05	117

// Steward
//============================================================
gefg_cas05,70,52,3	duplicate(cm)	Byohre#gefg_cas05	55

// Treasure Room Exit
//============================================================
gefg_cas05,149,107,0	script	#lever_G05	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "gefg_cas05",70,53;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
gefg_cas05,65,22,0	duplicate(gdlever)	#gefg_cas05	111

// Guild Treasure Spawns
//============================================================
gefg_cas05,144,110,0	duplicate(Gld_Trea_Spawn)	Treasure#gefg_cas05	-1

// AGIT Manager
//============================================================
gefg_cas05,194,184,0	duplicate(Gld_Agit_Manager)	Agit#gefg_cas05	-1

// Guardians
//============================================================
gefg_cas05,70,52,3	duplicate(Gld_Guard_Template)	Guardian#gefg_cas05	-1
