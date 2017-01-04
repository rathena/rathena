//===== rAthena Script =======================================
//= War of Emperium (Prontera) - Gloria 3
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Prontera Training Guild Castle 3
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
prt_gld,120,155,4	duplicate(flag_te)	Wigner#te_prtcas03	GUILD_FLAG

// Simple infos flags
//============================================================
te_prtcas03,168,28,4	duplicate(simple_info_TE)	Wigner#1	GUILD_FLAG
te_prtcas03,182,28,4	duplicate(simple_info_TE)	Wigner#2	GUILD_FLAG
te_prtcas03,43,50,4	duplicate(simple_info_TE)	Wigner#3	GUILD_FLAG
te_prtcas03,48,50,4	duplicate(simple_info_TE)	Wigner#4	GUILD_FLAG
te_prtcas03,43,58,4	duplicate(simple_info_TE)	Wigner#5	GUILD_FLAG
te_prtcas03,48,58,4	duplicate(simple_info_TE)	Wigner#6	GUILD_FLAG
te_prtcas03,158,210,4	duplicate(simple_info_TE)	Wigner#7	GUILD_FLAG
te_prtcas03,169,210,4	duplicate(simple_info_TE)	Wigner#8	GUILD_FLAG
te_prtcas03,162,201,4	duplicate(simple_info_TE)	Wigner#9	GUILD_FLAG
te_prtcas03,165,201,4	duplicate(simple_info_TE)	Wigner#10	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_prt_gld,147,140,4	script	Wigner#11	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE",1,"te_prtcas03",181,215 );
OnFlagTEWigner:
	flagemblem getcastledata( "te_prtcas03",1 );
	end;
}
te_prt_gld,147,136,4	duplicate(Wigner#11)	Wigner#12	GUILD_FLAG
te_prt_gld,158,140,4	duplicate(Wigner#11)	Wigner#13	GUILD_FLAG
te_prt_gld,158,136,4	duplicate(Wigner#11)	Wigner#14	GUILD_FLAG

// Guild kafra
//============================================================
te_prtcas03,181,215,4	duplicate(Kafra_Staff_TE)	Kafra Employee#te_prt03	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_prtcas03,192,134,0	duplicate(lever1_TE)	#lever1_te_prtcas03	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_prtcas03,5,70,0	duplicate(lever2_TE)	#lever2_te_prtcas03	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_prtcas03,51,100,4	duplicate(invest_TE)	Eduare	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_prtcas03,1,1,0	duplicate(treasure_TE)	treasure_TE#te_prtcas03	-1

// Rental items npc
//============================================================
te_prt_gld,162,141,3	duplicate(rental_woe_TE)	Rental Manager#pg03	4_F_HUWOMAN

// Manager woe
//============================================================
te_prtcas03,1,1,0	duplicate(Manager_TE)	Manager_TE#Wigner	-1
