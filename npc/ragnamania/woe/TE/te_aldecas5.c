//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Kafragarten 5
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Al De Baran Training Guild Castle 5
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
alde_gld,214,178,4	duplicate(flag_te)	W#te_aldecas5	GUILD_FLAG

// Simple infos flags
//============================================================
te_aldecas5,170,85,2	duplicate(simple_info_TE)	W#1	GUILD_FLAG
te_aldecas5,142,212,2	duplicate(simple_info_TE)	W#2	GUILD_FLAG
te_aldecas5,149,196,2	duplicate(simple_info_TE)	W#3	GUILD_FLAG
te_aldecas5,41,180,2	duplicate(simple_info_TE)	W#4	GUILD_FLAG
te_aldecas5,38,201,2	duplicate(simple_info_TE)	W#5	GUILD_FLAG
te_aldecas5,65,182,2	duplicate(simple_info_TE)	W#6	GUILD_FLAG
te_aldecas5,65,205,2	duplicate(simple_info_TE)	W#7	GUILD_FLAG
te_aldecas5,10,218,2	duplicate(simple_info_TE)	W#8	GUILD_FLAG
te_aldecas5,10,218,2	duplicate(simple_info_TE)	W#9	GUILD_FLAG
te_aldecas5,164,201,2	duplicate(simple_info_TE)	W#10	GUILD_FLAG
te_aldecas5,14,117,2	duplicate(simple_info_TE)	W#11	GUILD_FLAG
te_aldecas5,10,225,2	duplicate(simple_info_TE)	W#12	GUILD_FLAG
te_aldecas5,187,59,2	duplicate(simple_info_TE)	W#13	GUILD_FLAG
te_aldecas5,154,51,2	duplicate(simple_info_TE)	W#14	GUILD_FLAG
te_aldecas5,22,211,2	duplicate(simple_info_TE)	W#15	GUILD_FLAG
te_aldecas5,150,202,2	duplicate(simple_info_TE)	W#16	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_alde_gld,265,93,6	script	W#17	GUILD_FLAG,{
	// callfunc( "F_flag_woe_TE",1,"te_aldecas1",218,170 );
	callfunc( "F_flag_woe_TE",1,"te_aldecas5",31,190 );
OnFlagTEW:
	flagemblem getcastledata( "te_aldecas5",1 );
	end;
}
// te_alde_gld,45,87,8	duplicate(W#17)	Glaris	GUILD_FLAG
te_alde_gld,265,87,6	duplicate(W#17)	W#18	GUILD_FLAG

// Guild kafra
//============================================================
te_aldecas5,31,190,0	duplicate(Kafra_Staff_TE)	Kafra Employee#te_alde5	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_aldecas5,68,13,0	duplicate(lever1_TE)	#lever1_te_aldecas5	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_aldecas5,22,205,0	duplicate(lever2_TE)	#lever2_te_aldecas5	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_aldecas5,51,179,0	duplicate(invest_TE)	Anpere	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_aldecas5,1,1,0	duplicate(treasure_TE)	treasure_TE#te_aldecas5	-1

// Rental items npc
//============================================================
te_alde_gld,264,94,3	duplicate(rental_woe_TE)	Rental Manager#ag05	4_F_HUWOMAN

// Manager woe
//============================================================
te_aldecas5,1,1,0	duplicate(Manager_TE)	Manager_TE#W	-1
