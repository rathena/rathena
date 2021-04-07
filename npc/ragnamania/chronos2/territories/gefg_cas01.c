//===== rAthena Script =======================================
//= War of Emperium (Geffen) - Repherion
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Geffen Guild Castle 1
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Repherion#1::OutsideFlagsG1	722,{

	set .@GID, GetCastleData("gefg_cas01",1);

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
			if (getcharid(2) == GetCastleData("gefg_cas01",1)) {
				warp "gefg_cas01",197,36;
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

OnRecvCastleG01:
	FlagEmblem GetCastleData("gefg_cas01",1);
	end;
}

gef_fild13,148,51,5	duplicate(OutsideFlagsG1)	Repherion#1-2	722
gef_fild13,155,54,5	duplicate(OutsideFlagsG1)	Repherion#2	722
gef_fild13,212,79,6	duplicate(OutsideFlagsG1)	Repherion#3	722
gef_fild13,211,71,6	duplicate(OutsideFlagsG1)	Repherion#4	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Repherion#5::InsideFlagsG1	722,{
	end;

OnRecvCastleG01:
	FlagEmblem GetCastleData("gefg_cas01",1);
	end;
}

gefg_cas01,28,157,4	duplicate(InsideFlagsG1)	Repherion#5-2	722
gefg_cas01,22,156,5	duplicate(InsideFlagsG1)	Repherion#6	722
gefg_cas01,68,185,3	duplicate(InsideFlagsG1)	Repherion#7	722
gefg_cas01,17,171,5	duplicate(InsideFlagsG1)	Repherion#8	722
gefg_cas01,59,16,4	duplicate(InsideFlagsG1)	Repherion#9	722
gefg_cas01,64,16,4	duplicate(InsideFlagsG1)	Repherion#10	722

// Guild Flag (Town)
//============================================================
geffen,109,123,2	duplicate(InsideFlagsG1)	Repherion#11	722

// Kafra Employee
//============================================================
gefg_cas01,83,47,3	duplicate(guildkafra)	Kafra Staff#gefg_cas01	117

// Steward
//============================================================
gefg_cas01,40,48,5	duplicate(cm)	Gnahcher#gefg_cas01	55

// Treasure Room Exit
//============================================================
gefg_cas01,152,117,0	script	#lever_G01	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "gefg_cas01",40,49;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
gefg_cas01,78,84,0	duplicate(gdlever)	#gefg_cas01	111

// Guild Treasure Spawns
//============================================================
gefg_cas01,154,112,0	duplicate(Gld_Trea_Spawn)	Treasure#gefg_cas01	-1

// AGIT Manager
//============================================================
gefg_cas01,198,182,0	duplicate(Gld_Agit_Manager)	Agit#gefg_cas01	-1

// Guardians
//============================================================
gefg_cas01,40,48,5	duplicate(Gld_Guard_Template)	Guardian#gefg_cas01	-1
