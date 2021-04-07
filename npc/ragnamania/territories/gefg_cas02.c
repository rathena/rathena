//===== rAthena Script =======================================
//= War of Emperium (Geffen) - Eeyorbriggar
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Geffen Guild Castle 2
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Eeyorbriggar#1::OutsideFlagsG2	722,{

	set .@GID, GetCastleData("gefg_cas02",1);

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
			if (getcharid(2) == GetCastleData("gefg_cas02",1)) {
				warp "gefg_cas02",178,43;
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

OnRecvCastleG02:
	FlagEmblem GetCastleData("gefg_cas02",1);
	end;
}

gef_fild13,303,243,4	duplicate(OutsideFlagsG2)	Eeyorbriggar#1-2	722
gef_fild13,312,243,4	duplicate(OutsideFlagsG2)	Eeyorbriggar#2	722
gef_fild13,290,243,4	duplicate(OutsideFlagsG2)	Eeyorbriggar#3	722
gef_fild13,324,243,4	duplicate(OutsideFlagsG2)	Eeyorbriggar#4	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Eeyorbriggar#5::InsideFlagsG2	722,{
	end;

OnRecvCastleG02:
	FlagEmblem GetCastleData("gefg_cas02",1);
	end;
}

gefg_cas02,65,130,5	duplicate(InsideFlagsG2)	Eeyorbriggar#5-2	722
gefg_cas02,30,123,5	duplicate(InsideFlagsG2)	Eeyorbriggar#6	722
gefg_cas02,65,139,6	duplicate(InsideFlagsG2)	Eeyorbriggar#7	722
gefg_cas02,37,177,6	duplicate(InsideFlagsG2)	Eeyorbriggar#8	722
gefg_cas02,37,168,6	duplicate(InsideFlagsG2)	Eeyorbriggar#9	722
gefg_cas02,68,47,2	duplicate(InsideFlagsG2)	Eeyorbriggar#10	722
gefg_cas02,68,36,2	duplicate(InsideFlagsG2)	Eeyorbriggar#11	722

// Guild Flag (Town)
//============================================================
geffen,112,129,1	duplicate(InsideFlagsG2)	Eeyorbriggar#12	722

// Kafra Employee
//============================================================
gefg_cas02,23,66,3	duplicate(guildkafra)	Kafra Staff#gefg_cas02	117

// Steward
//============================================================
gefg_cas02,12,66,5	duplicate(cm)	Esmarehk#gefg_cas02	55

// Treasure Room Exit
//============================================================
gefg_cas02,145,114,0	script	#lever_G02	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "gefg_cas02",12,67;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
gefg_cas02,167,40,0	duplicate(gdlever)	#gefg_cas02	111

// Guild Treasure Spawns
//============================================================
gefg_cas02,140,116,0	duplicate(Gld_Trea_Spawn)	Treasure#gefg_cas02	-1

// AGIT Manager
//============================================================
gefg_cas02,176,178,0	duplicate(Gld_Agit_Manager)	Agit#gefg_cas02	-1

// Guardians
//============================================================
gefg_cas02,12,66,5	duplicate(Gld_Guard_Template)	Guardian#gefg_cas02	-1
