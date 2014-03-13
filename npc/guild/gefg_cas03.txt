//===== rAthena Script =======================================
//= War of Emperium (Geffen) - Yesnelph
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Geffen Guild Castle 3
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Yesnelph#1::OutsideFlagsG3	722,{

	set .@GID, GetCastleData("gefg_cas03",1);

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
			if (getcharid(2) == GetCastleData("gefg_cas03",1)) {
				warp "gefg_cas03",221,30;
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

OnRecvCastleG03:
	FlagEmblem GetCastleData("gefg_cas03",1);
	end;
}

gef_fild13,78,182,4	duplicate(OutsideFlagsG3)	Yesnelph#1-2	722
gef_fild13,87,182,4	duplicate(OutsideFlagsG3)	Yesnelph#2	722
gef_fild13,73,295,7	duplicate(OutsideFlagsG3)	Yesnelph#3	722
gef_fild13,113,274,7	duplicate(OutsideFlagsG3)	Yesnelph#4	722
gef_fild13,144,235,6	duplicate(OutsideFlagsG3)	Yesnelph#5	722
gef_fild13,144,244,6	duplicate(OutsideFlagsG3)	Yesnelph#6	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Yesnelph#7::InsideFlagsG3	722,{
	end;

OnRecvCastleG03:
	FlagEmblem GetCastleData("gefg_cas03",1);
	end;
}

gefg_cas03,122,220,6	duplicate(InsideFlagsG3)	Yesnelph#7-2	722
gefg_cas03,122,229,6	duplicate(InsideFlagsG3)	Yesnelph#8	722
gefg_cas03,91,257,7	duplicate(InsideFlagsG3)	Yesnelph#9	722
gefg_cas03,52,276,7	duplicate(InsideFlagsG3)	Yesnelph#10	722
gefg_cas03,56,164,4	duplicate(InsideFlagsG3)	Yesnelph#11	722
gefg_cas03,65,164,4	duplicate(InsideFlagsG3)	Yesnelph#12	722
gefg_cas03,37,214,1	duplicate(InsideFlagsG3)	Yesnelph#13	722
gefg_cas03,34,208,1	duplicate(InsideFlagsG3)	Yesnelph#14	722

// Guild Flag (Town)
//============================================================
geffen,120,132,8	duplicate(InsideFlagsG3)	Yesnelph#15	722

// Kafra Employee
//============================================================
gefg_cas03,116,89,5	duplicate(guildkafra)	Kafra Staff#gefg_cas03	117

// Steward
//============================================================
gefg_cas03,106,23,5	duplicate(cm)	Jyang#gefg_cas03	55

// Treasure Room Exit
//============================================================
gefg_cas03,275,289,0	script	#lever_G03	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "gefg_cas03",106,24;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
gefg_cas03,221,43,0	duplicate(gdlever)	#gefg_cas03	111

// Guild Treasure Spawns
//============================================================
gefg_cas03,271,290,0	duplicate(Gld_Trea_Spawn)	Treasure#gefg_cas03	-1

// AGIT Manager
//============================================================
gefg_cas03,245,167,0	duplicate(Gld_Agit_Manager)	Agit#gefg_cas03	-1

// Guardians
//============================================================
gefg_cas03,106,23,5	duplicate(Gld_Guard_Template)	Guardian#gefg_cas03	-1
