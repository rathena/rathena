//===== rAthena Script =======================================
//= War of Emperium (Payon) - Bamboo Grove Hill
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.3
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Payon Guild Castle 5
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Corrected flag-in spawn point. [L0ne_W0lf]
//= 1.2 Removed dialog from interior flags. [L0ne_W0lf]
//=     Fixed position of Guild dungeon switch.
//= 1.3 Partially rewrote Script. [Masao]
//============================================================

// Guild Flags (Outside Castle)
//============================================================
-	script	Bamboo Grove Hill#1::OutsideFlagsPA5	722,{

	set .@GID, GetCastleData("payg_cas05",1);

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
			if (getcharid(2) == GetCastleData("payg_cas05",1)) {
					warp "payg_cas05",243,27;
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

OnRecvCastlePy05:
	FlagEmblem GetCastleData("payg_cas05",1);
	end;
}

pay_gld,208,268,4	duplicate(OutsideFlagsPA5)	Bamboo Grove Hill#1-2	722
pay_gld,199,268,4	duplicate(OutsideFlagsPA5)	Bamboo Grove Hill#2	722
pay_gld,190,277,3	duplicate(OutsideFlagsPA5)	Bamboo Grove Hill#3	722
pay_gld,187,294,2	duplicate(OutsideFlagsPA5)	Bamboo Grove Hill#4	722

// Guild Flags (Inside Castle)
//============================================================
-	script	Bamboo Grove Hill#5::InsideFlagsPA5	722,{
	end;

OnRecvCastlePy05:
	FlagEmblem GetCastleData("payg_cas05",1);
	end;
}

payg_cas05,32,249,4	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#5-2	722
payg_cas05,24,249,4	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#6	722
payg_cas05,62,271,0	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#7	722
payg_cas05,57,271,0	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#8	722
payg_cas05,55,252,2	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#9	722
payg_cas05,55,260,2	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#10	722

// Guild Flag (Town)
//============================================================
payon,123,322,4	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#11	722
payon,166,161,3	duplicate(InsideFlagsPA5)	Bamboo Grove Hill#12	722

// Kafra Employee
//============================================================
payg_cas05,276,227,1	duplicate(guildkafra)	Kafra Staff#payg_cas05	117

// Steward
//============================================================
payg_cas05,277,249,3	duplicate(cm)	Nahzarf#payg_cas05	55

// Treasure Room Exit
//============================================================
payg_cas05,161,136,0	script	#lever_PA05	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "payg_cas05",277,250;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
payg_cas05,249,15,0	duplicate(gdlever)	#payg_cas05	111

// Guild Treasure Spawns
//============================================================
payg_cas05,157,132,0	duplicate(Gld_Trea_Spawn)	Treasure#payg_cas05	-1

// AGIT Manager
//============================================================
payg_cas05,30,30,0	duplicate(Gld_Agit_Manager)	Agit#payg_cas05	-1

// Guardians
//============================================================
payg_cas05,277,249,3	duplicate(Gld_Guard_Template)	Guardian#payg_cas05	-1
