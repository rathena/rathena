//===== rAthena Script =======================================
//= War of Emperium (Geffen) - Bergel
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Geffen Guild Castle 4
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Bergel#1::OutsideFlagsG4	722,{

	set .@GID, GetCastleData("gefg_cas04",1);

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
			if (getcharid(2) == GetCastleData("gefg_cas04",1)) {
				warp "gefg_cas04",168,43;
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

OnRecvCastleG04:
	FlagEmblem GetCastleData("gefg_cas04",1);
	end;
}

gef_fild13,190,283,3	duplicate(OutsideFlagsG4)	Bergel#1-2	722
gef_fild13,199,274,3	duplicate(OutsideFlagsG4)	Bergel#2	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Bergel#3::InsideFlagsG4	722,{
	end;

OnRecvCastleG04:
	FlagEmblem GetCastleData("gefg_cas04",1);
	end;
}

gefg_cas04,24,157,4	duplicate(InsideFlagsG4)	Bergel#3-2	722
gefg_cas04,35,158,4	duplicate(InsideFlagsG4)	Bergel#4	722
gefg_cas04,44,184,4	duplicate(InsideFlagsG4)	Bergel#5	722
gefg_cas04,51,184,4	duplicate(InsideFlagsG4)	Bergel#6	722
gefg_cas04,39,212,7	duplicate(InsideFlagsG4)	Bergel#7	722
gefg_cas04,29,212,1	duplicate(InsideFlagsG4)	Bergel#8	722
gefg_cas04,24,73,1	duplicate(InsideFlagsG4)	Bergel#9	722
gefg_cas04,35,73,4	duplicate(InsideFlagsG4)	Bergel#10	722

// Guild Flag (Town)
//============================================================
geffen,127,130,7	duplicate(InsideFlagsG4)	Bergel#11	722

// Kafra Employee
//============================================================
gefg_cas04,59,70,3	duplicate(guildkafra)	Kafra Staff#gefg_cas04	117

// Steward
//============================================================
gefg_cas04,73,46,3	duplicate(cm)	Kellvahni#gefg_cas04	55

// Treasure Room Exit
//============================================================
gefg_cas04,116,123,0	script	#lever_G04	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "gefg_cas04",73,47;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
gefg_cas04,58,75,0	duplicate(gdlever)	#gefg_cas04	111

// Guild Treasure Spawns
//============================================================
gefg_cas04,116,119,0	duplicate(Gld_Trea_Spawn)	Treasure#gefg_cas04	-1

// AGIT Manager
//============================================================
gefg_cas04,174,178,0	duplicate(Gld_Agit_Manager)	Agit#gefg_cas04	-1

// Guardians
//============================================================
gefg_cas04,73,46,3	duplicate(Gld_Guard_Template)	Guardian#gefg_cas04	-1
