//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Kafragarten 2
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Al De Baran Training Guild Castle 2
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
alde_gld,220,184,4	duplicate(flag_te)	Defolty#te_aldecas2	GUILD_FLAG

// Simple infos flags
//============================================================
te_aldecas2,82,71,2	duplicate(simple_info_TE)	Defolty#1	GUILD_FLAG
te_aldecas2,67,30,2	duplicate(simple_info_TE)	Defolty#2	GUILD_FLAG
te_aldecas2,183,140,2	duplicate(simple_info_TE)	Defolty#3	GUILD_FLAG
te_aldecas2,212,152,2	duplicate(simple_info_TE)	Defolty#4	GUILD_FLAG
te_aldecas2,108,39,2	duplicate(simple_info_TE)	Defolty#5	GUILD_FLAG
te_aldecas2,57,213,2	duplicate(simple_info_TE)	Defolty#6	GUILD_FLAG
te_aldecas2,103,53,2	duplicate(simple_info_TE)	Defolty#7	GUILD_FLAG
te_aldecas2,73,53,2	duplicate(simple_info_TE)	Defolty#8	GUILD_FLAG
te_aldecas2,63,41,2	duplicate(simple_info_TE)	Defolty#9	GUILD_FLAG
te_aldecas2,229,6,2	duplicate(simple_info_TE)	Defolty#10	GUILD_FLAG
te_aldecas2,230,40,2	duplicate(simple_info_TE)	Defolty#11	GUILD_FLAG
te_aldecas2,197,40,2	duplicate(simple_info_TE)	Defolty#12	GUILD_FLAG
te_aldecas2,32,213,2	duplicate(simple_info_TE)	Defolty#13	GUILD_FLAG
te_aldecas2,121,29,2	duplicate(simple_info_TE)	Defolty#14	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_alde_gld,99,251,4	script	Defolty#15	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE", 1, "te_aldecas2" ,84,74 );
OnFlagTEDefolty:
	flagemblem getcastledata( "te_aldecas2",1 );
	end;
}
te_alde_gld,99,244,4	duplicate(Defolty#15)	Defolty#16	GUILD_FLAG

// Guild kafra
//============================================================
te_aldecas2,84,74,0	duplicate(Kafra_Staff_TE)	Kafra Employee#te_alde2	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_aldecas2,139,234,0	duplicate(lever1_TE)	#lever1_te_aldecas2	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_aldecas2,194,136,0	duplicate(lever2_TE)	#lever2_te_aldecas2	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_aldecas2,78,74,0	duplicate(invest_TE)	Lares	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_aldecas2,1,1,0	duplicate(treasure_TE)	treasure_TE#te_aldecas2	-1

// Rental items npc
//============================================================
te_alde_gld,99,252,3	duplicate(rental_woe_TE)	Rental Manager#ag02	4_F_HUWOMAN

// Manager woe
//============================================================
te_aldecas2,1,1,0	duplicate(Manager_TE)	Manager_TE#Defolty	-1
