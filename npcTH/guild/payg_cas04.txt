//===== rAthena Script =======================================
//= War of Emperium (Payon) - Sacred Altar
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.4
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Payon Guild Castle 4
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
-	script	Sacred Altar#1::OutsideFlagsPA4	722,{

	set .@GID, GetCastleData("payg_cas04",1);

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
			if (getcharid(2) == GetCastleData("payg_cas04",1)) {
					warp "payg_cas04",40,235;
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

OnRecvCastlePy04:
	FlagEmblem GetCastleData("payg_cas04",1);
	end;
}

pay_gld,137,160,0	duplicate(OutsideFlagsPA4)	Sacred Altar#1-2	722
pay_gld,143,160,0	duplicate(OutsideFlagsPA4)	Sacred Altar#2	722
pay_gld,133,151,2	duplicate(OutsideFlagsPA4)	Sacred Altar#3	722
pay_gld,153,166,1	duplicate(OutsideFlagsPA4)	Sacred Altar#4	722

// Guild Flags (Inside Castle)
//============================================================
-	script	Sacred Altar#5::InsideFlagsPA4	722,{
	end;

OnRecvCastlePy04:
	FlagEmblem GetCastleData("payg_cas04",1);
	end;
}

payg_cas04,255,259,0	duplicate(InsideFlagsPA4)	Sacred Altar#5-2	722
payg_cas04,248,259,0	duplicate(InsideFlagsPA4)	Sacred Altar#6	722
payg_cas04,248,168,6	duplicate(InsideFlagsPA4)	Sacred Altar#7	722
payg_cas04,248,160,6	duplicate(InsideFlagsPA4)	Sacred Altar#8	722
payg_cas04,232,181,4	duplicate(InsideFlagsPA4)	Sacred Altar#9	722
payg_cas04,239,181,4	duplicate(InsideFlagsPA4)	Sacred Altar#10	722

// Guild Flag (Town)
//============================================================
payon,118,322,4	duplicate(InsideFlagsPA4)	Sacred Altar#11	722
payon,166,165,3	duplicate(InsideFlagsPA4)	Sacred Altar#12	722

// Kafra Employee
//============================================================
payg_cas04,40,235,1	duplicate(guildkafra)	Kafra Staff#payg_cas04	117

// Steward
//============================================================
payg_cas04,38,284,3	duplicate(cm)	DJ#payg_cas04	55

// Treasure Room Exit
//============================================================
payg_cas04,151,47,0	script	#lever_PA04	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "payg_cas04",38,285;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
payg_cas04,52,48,0	duplicate(gdlever)	#payg_cas04	111

// Guild Treasure Spawns
//============================================================
payg_cas04,147,48,0	duplicate(Gld_Trea_Spawn)	Treasure#payg_cas04	-1

// AGIT Manager
//============================================================
payg_cas04,271,29,0	duplicate(Gld_Agit_Manager)	Agit#payg_cas04	-1

// Guardians
//============================================================
payg_cas04,38,284,3	duplicate(Gld_Guard_Template)	Guardian#payg_cas04	-1
