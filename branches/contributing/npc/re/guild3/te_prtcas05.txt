//===== rAthena Script =======================================
//= War of Emperium (Prontera) - Gloria 5
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Prontera Training Guild Castle 5
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
prt_gld,128,155,4	duplicate(flag_te)	Nerious#te_prtcas05	GUILD_FLAG

// Simple infos flags
//============================================================
te_prtcas05,19,247,4	duplicate(simple_info_TE)	Nerious#1	GUILD_FLAG
te_prtcas05,19,243,4	duplicate(simple_info_TE)	Nerious#2	GUILD_FLAG
te_prtcas05,26,247,4	duplicate(simple_info_TE)	Nerious#3	GUILD_FLAG
te_prtcas05,26,243,4	duplicate(simple_info_TE)	Nerious#4	GUILD_FLAG
te_prtcas05,249,289,4	duplicate(simple_info_TE)	Nerious#5	GUILD_FLAG
te_prtcas05,256,289,4	duplicate(simple_info_TE)	Nerious#6	GUILD_FLAG
te_prtcas05,253,271,4	duplicate(simple_info_TE)	Nerious#7	GUILD_FLAG
te_prtcas05,273,257,4	duplicate(simple_info_TE)	Nerious#8	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_prt_gld,199,243,2	script	Nerious#9	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE",1,"te_prtcas05",52,41 );
OnFlagTENerious:
	flagemblem getcastledata( "te_prtcas05",1 );
	end;
}
te_prt_gld,199,236,2	duplicate(Nerious#9)	Nerious#10	GUILD_FLAG
te_prt_gld,197,243,2	duplicate(Nerious#9)	Nerious#11	GUILD_FLAG
te_prt_gld,197,236,2	duplicate(Nerious#9)	Nerious#12	GUILD_FLAG

// Guild kafra
//============================================================
te_prtcas05,52,41,4	duplicate(Kafra_Staff_TE)	Kafra Employee#te_prt05	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_prtcas05,280,177,0	duplicate(lever1_TE)	#lever1_te_prtcas05	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_prtcas05,212,95,0	duplicate(lever2_TE)	#lever2_te_prtcas05	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_prtcas05,36,37,4	duplicate(invest_TE)	Pisaro	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_prtcas05,1,1,0	duplicate(treasure_TE)	treasure_TE#te_prtcas05	-1

// Rental items npc
//============================================================
te_prt_gld,203,243,3	duplicate(rental_woe_TE)	Rental Manager#pg05	4_F_HUWOMAN

// Manager woe
//============================================================
te_prtcas05,1,1,0	duplicate(Manager_TE)	Manager_TE#Nerious	-1
