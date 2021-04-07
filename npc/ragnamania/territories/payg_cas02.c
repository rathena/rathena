//===== rAthena Script =======================================
//= War of Emperium (Payon) - Scarlet Palace
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.4
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Payon Guild Castle 2
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Corrected flag-in spawn point. [L0ne_W0lf]
//= 1.2 Removed dialog from interior flags. [L0ne_W0lf]
//=     Fixed position of Guild dungeon switch.
//= 1.3 Partially rewrote Script. [Masao]
//= 1.4 Corrected guild name. [Euphy]
//============================================================

// Guild Flags (Outside Castle)
//============================================================
-	script	Scarlet Palace#1::OutsideFlagsPA2	722,{

	set .@GID, GetCastleData("payg_cas02",1);

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
			if (getcharid(2) == GetCastleData("payg_cas02",1)) {
					warp "payg_cas02",278,251;
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

OnRecvCastlePy02:
	FlagEmblem GetCastleData("payg_cas02",1);
	end;
}

pay_gld,292,112,6	duplicate(OutsideFlagsPA2)	Scarlet Palace#1-2	722
pay_gld,292,120,6	duplicate(OutsideFlagsPA2)	Scarlet Palace#2	722
pay_gld,291,135,6	duplicate(OutsideFlagsPA2)	Scarlet Palace#3	722
pay_gld,271,163,0	duplicate(OutsideFlagsPA2)	Scarlet Palace#4	722

// Guild Flags (Inside Castle)
//============================================================
-	script	Scarlet Palace#5::InsideFlagsPA2	722,{
	end;

OnRecvCastlePy02:
	FlagEmblem GetCastleData("payg_cas02",1);
	end;
}

payg_cas02,254,40,6	duplicate(InsideFlagsPA2)	Scarlet Palace#5-2	722
payg_cas02,254,48,6	duplicate(InsideFlagsPA2)	Scarlet Palace#6	722
payg_cas02,202,49,0	duplicate(InsideFlagsPA2)	Scarlet Palace#7	722
payg_cas02,209,49,0	duplicate(InsideFlagsPA2)	Scarlet Palace#8	722
payg_cas02,59,282,4	duplicate(InsideFlagsPA2)	Scarlet Palace#9	722
payg_cas02,70,282,4	duplicate(InsideFlagsPA2)	Scarlet Palace#10	722

// Guild Flag (Town)
//============================================================
payon,97,322,4	duplicate(InsideFlagsPA2)	Scarlet Palace#11	722
payon,166,173,3	duplicate(InsideFlagsPA2)	Scarlet Palace#12	722

// Kafra Employee
//============================================================
payg_cas02,22,275,5	duplicate(guildkafra)	Kafra Staff#payg_cas02	117

// Steward
//============================================================
payg_cas02,22,260,7	duplicate(cm)	Cherieos#payg_cas02	55

// Treasure Room Exit
//============================================================
payg_cas02,149,149,0	script	#lever_PA02	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "payg_cas02",22,261;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
payg_cas02,278,247,0	duplicate(gdlever)	#payg_cas02	111

// Guild Treasure Spawns
//============================================================
payg_cas02,145,144,0	duplicate(Gld_Trea_Spawn)	Treasure#payg_cas02	-1

// AGIT Manager
//============================================================
payg_cas02,39,25,0	duplicate(Gld_Agit_Manager)	Agit#payg_cas02	-1

// Guardians
//============================================================
payg_cas02,22,260,7	duplicate(Gld_Guard_Template)	Guardian#payg_cas02	-1
