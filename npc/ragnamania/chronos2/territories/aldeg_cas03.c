//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Nuernberg
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.2
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Al De Baran Guild Castle 3
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Removed dialog from interior flags. [L0ne_W0lf]
//= 1.2 Partially rewrote Script. [Masao]
//============================================================

// Guild Flag (Outside Castle)
//============================================================
-	script	Nuernberg#1::OutsideFlagsA3	722,{

	set .@GID, GetCastleData("aldeg_cas03",1);

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
			if (getcharid(2) == GetCastleData("aldeg_cas03",1)) {
				warp "aldeg_cas03",205,186;
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

OnRecvCastleA03:
	FlagEmblem GetCastleData("aldeg_cas03",1);
	end;
}

alde_gld,146,82,8	duplicate(OutsideFlagsA3)	Nuernberg#1-2	722
alde_gld,138,82,8	duplicate(OutsideFlagsA3)	Nuernberg#2	722

// Guild Flag (Inside Castle)
//============================================================
-	script	Nuernberg#3::InsideFlagsA3	722,{
	end;

OnRecvCastleA03:
	FlagEmblem GetCastleData("aldeg_cas03",1);
	end;
}

aldeg_cas03,176,175,2	duplicate(InsideFlagsA3)	Nuernberg#3-2	722
aldeg_cas03,77,115,2	duplicate(InsideFlagsA3)	Nuernberg#4	722
aldeg_cas03,77,215,2	duplicate(InsideFlagsA3)	Nuernberg#5	722
aldeg_cas03,112,107,2	duplicate(InsideFlagsA3)	Nuernberg#6	722
aldeg_cas03,112,117,2	duplicate(InsideFlagsA3)	Nuernberg#7	722
aldeg_cas03,69,71,2	duplicate(InsideFlagsA3)	Nuernberg#8	722
aldeg_cas03,91,69,2	duplicate(InsideFlagsA3)	Nuernberg#9	722
aldeg_cas03,108,60,2	duplicate(InsideFlagsA3)	Nuernberg#10	722
aldeg_cas03,121,73,2	duplicate(InsideFlagsA3)	Nuernberg#11	722
aldeg_cas03,121,73,2	duplicate(InsideFlagsA3)	Nuernberg#12	722
aldeg_cas03,75,102,2	duplicate(InsideFlagsA3)	Nuernberg#13	722
aldeg_cas03,199,169,2	duplicate(InsideFlagsA3)	Nuernberg#14	722
aldeg_cas03,181,179,2	duplicate(InsideFlagsA3)	Nuernberg#15	722
aldeg_cas03,192,44,2	duplicate(InsideFlagsA3)	Nuernberg#16	722
aldeg_cas03,208,145,2	duplicate(InsideFlagsA3)	Nuernberg#17	722
aldeg_cas03,207,75,2	duplicate(InsideFlagsA3)	Nuernberg#18	722
aldeg_cas03,96,62,2	duplicate(InsideFlagsA3)	Nuernberg#19	722

// Guild Flag (Town)
//============================================================
aldebaran,134,97,4	duplicate(InsideFlagsA3)	Nuernberg#20	722

// Kafra Employee
//============================================================
aldeg_cas03,118,76,0	duplicate(guildkafra)	Kafra Staff#aldeg_cas03	117

// Steward
//============================================================
aldeg_cas03,110,118,0	duplicate(cm)	Nahzarf#aldeg_cas03	55

// Treasure Room Exit
//============================================================
aldeg_cas03,229,267,0	script	#lever_A03	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "aldeg_cas03",110,119;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
aldeg_cas03,200,177,0	duplicate(gdlever)	#aldeg_cas03	111

// Guild Treasure Spawns
//============================================================
aldeg_cas03,225,269,0	duplicate(Gld_Trea_Spawn)	Treasure#aldeg_cas03	-1

// AGIT Manager
//============================================================
aldeg_cas03,206,32,0	duplicate(Gld_Agit_Manager)	Agit#aldeg_cas03	-1

// Guardians
//============================================================
aldeg_cas03,110,118,0	duplicate(Gld_Guard_Template)	Guardian#aldeg_cas03	-1
