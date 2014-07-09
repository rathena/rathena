//===== rAthena Script =======================================
//= War of Emperium (Payon) - Holy Shadow
//===== By: ==================================================
//= L0ne_W0lf
//===== Current Version: =====================================
//= 1.3
//===== Compatible With: =====================================
//= rAthena Project
//===== Description: =========================================
//= NPCs that relate to Payon Guild Castle 3
//===== Additional Comments: =================================
//= 1.0 First Version. No longer uses functions. [L0ne_W0lf]
//= 1.1 Corrected flag-in spawn point. [L0ne_W0lf]
//= 1.2 Removed dialog from interior flags. [L0ne_W0lf]
//=     Fixed position of Guild dungeon switch.
//= 1.3 Partially rewrote Script. [Masao]
//============================================================

// Guild Flags (Outside Castle)
//============================================================
-	script	Holy Shadow#1::OutsideFlagsPA3	722,{

	set .@GID, GetCastleData("payg_cas03",1);

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
			if (getcharid(2) == GetCastleData("payg_cas03",1)) {
					warp "payg_cas03",9,263;
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

OnRecvCastlePy03:
	FlagEmblem GetCastleData("payg_cas03",1);
	end;
}

pay_gld,321,298,2	duplicate(OutsideFlagsPA3)	Holy Shadow#1-2	722
pay_gld,321,289,2	duplicate(OutsideFlagsPA3)	Holy Shadow#2	722
pay_gld,327,304,1	duplicate(OutsideFlagsPA3)	Holy Shadow#3	722
pay_gld,333,254,4	duplicate(OutsideFlagsPA3)	Holy Shadow#4	722

// Guild Flags (Inside Castle)
//============================================================
-	script	Holy Shadow#5::InsideFlagsPA3	722,{
	end;

OnRecvCastlePy03:
	FlagEmblem GetCastleData("payg_cas03",1);
	end;
}

payg_cas03,236,54,2	duplicate(InsideFlagsPA3)	Holy Shadow#5-2	722
payg_cas03,236,45,2	duplicate(InsideFlagsPA3)	Holy Shadow#6	722
payg_cas03,259,66,4	duplicate(InsideFlagsPA3)	Holy Shadow#7	722
payg_cas03,266,66,4	duplicate(InsideFlagsPA3)	Holy Shadow#8	722
payg_cas03,34,31,4	duplicate(InsideFlagsPA3)	Holy Shadow#9	722
payg_cas03,43,31,4	duplicate(InsideFlagsPA3)	Holy Shadow#10	722

// Guild Flag (Town)
//============================================================
payon,113,322,4	duplicate(InsideFlagsPA3)	Holy Shadow#11	722
payon,166,169,3	duplicate(InsideFlagsPA3)	Holy Shadow#12	722

// Kafra Employee
//============================================================
payg_cas03,9,263,5	duplicate(guildkafra)	Kafra Staff#payg_cas03	117

// Steward
//============================================================
payg_cas03,50,261,3	duplicate(cm)	Gheriot#payg_cas03	55

// Treasure Room Exit
//============================================================
payg_cas03,163,167,0	script	#lever_PA03	111,{
	mes " ";
	mes "There's a small lever. Will you pull it?";
	next;
	if(select("Pull.:Do not.") == 1) {
		close2;
		warp "payg_cas03",50,261;
		end;
	}
	close;
}

// Guild Dungeon Switch
//============================================================
payg_cas03,38,42,0	duplicate(gdlever)	#payg_cas03	111

// Guild Treasure Spawns
//============================================================
payg_cas03,159,168,0	duplicate(Gld_Trea_Spawn)	Treasure#payg_cas03	-1

// AGIT Manager
//============================================================
payg_cas03,269,265,0	duplicate(Gld_Agit_Manager)	Agit#payg_cas03	-1

// Guardians
//============================================================
payg_cas03,10,277,0	duplicate(Gld_Guard_Template)	Guardian#payg_cas03	-1
