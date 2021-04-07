//===== rAthena Script =======================================
//= War of Emperium (Prontera) - Gloria 4
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Prontera Training Guild Castle 4
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
prt_gld,124,155,4	duplicate(flag_te)	Heine#te_prtcas04	GUILD_FLAG

// Simple infos flags
//============================================================
te_prtcas04,82,29,4	duplicate(simple_info_TE)	Heine#1	GUILD_FLAG
te_prtcas04,75,29,4	duplicate(simple_info_TE)	Heine#2	GUILD_FLAG
te_prtcas04,75,27,4	duplicate(simple_info_TE)	Heine#3	GUILD_FLAG
te_prtcas04,82,27,4	duplicate(simple_info_TE)	Heine#4	GUILD_FLAG
te_prtcas04,59,29,4	duplicate(simple_info_TE)	Heine#5	GUILD_FLAG
te_prtcas04,67,29,4	duplicate(simple_info_TE)	Heine#6	GUILD_FLAG
te_prtcas04,258,25,4	duplicate(simple_info_TE)	Heine#7	GUILD_FLAG
te_prtcas04,258,20,4	duplicate(simple_info_TE)	Heine#8	GUILD_FLAG
te_prtcas04,263,20,4	duplicate(simple_info_TE)	Heine#9	GUILD_FLAG
te_prtcas04,263,27,4	duplicate(simple_info_TE)	Heine#10	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_prt_gld,120,243,6	script	Heine#11	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE",1,"te_prtcas04",258,247 );
OnFlagTEHeine:
	flagemblem getcastledata( "te_prtcas04",1 );
	end;
}
te_prt_gld,120,236,6	duplicate(Heine#11)	Heine#12	GUILD_FLAG
te_prt_gld,122,243,6	duplicate(Heine#11)	Heine#13	GUILD_FLAG
te_prt_gld,122,236,6	duplicate(Heine#11)	Heine#14	GUILD_FLAG

// Guild kafra
//============================================================
te_prtcas04,258,247,4	duplicate(Kafra_Staff_TE)	Kafra Employee#te_prt04	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_prtcas04,274,161,0	duplicate(lever1_TE)	#lever1_te_prtcas04	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_prtcas04,56,283,0	duplicate(lever2_TE)	#lever2_te_prtcas04	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_prtcas04,259,265,4	duplicate(invest_TE)	Casate	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_prtcas04,1,1,0	duplicate(treasure_TE)	treasure_TE#te_prtcas04	-1

// Rental items npc
//============================================================
te_prt_gld,117,243,3	duplicate(rental_woe_TE)	Rental Manager#pg04	4_F_HUWOMAN

// Manager woe
//============================================================
te_prtcas04,1,1,0	duplicate(Manager_TE)	Manager_TE#Heine	-1
