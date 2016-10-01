//===== rAthena Script =======================================
//= War of Emperium (Prontera) - Gloria 1
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Prontera Training Guild Castle 1
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
prt_gld,112,155,4	duplicate(flag_te)	Gaeborg#te_prtcas01	GUILD_FLAG

// Simple infos flags
//============================================================
te_prtcas01,58,56,4	duplicate(simple_info_TE)	Gaebolg#1	GUILD_FLAG
te_prtcas01,64,56,4	duplicate(simple_info_TE)	Gaebolg#2	GUILD_FLAG
te_prtcas01,76,32,4	duplicate(simple_info_TE)	Gaebolg#3	GUILD_FLAG
te_prtcas01,84,32,4	duplicate(simple_info_TE)	Gaebolg#4	GUILD_FLAG
te_prtcas01,94,39,4	duplicate(simple_info_TE)	Gaebolg#5	GUILD_FLAG
te_prtcas01,94,24,4	duplicate(simple_info_TE)	Gaebolg#6	GUILD_FLAG
te_prtcas01,73,14,4	duplicate(simple_info_TE)	Gaebolg#7	GUILD_FLAG
te_prtcas01,73,6,4	duplicate(simple_info_TE)	Gaebolg#8	GUILD_FLAG
te_prtcas01,55,46,4	duplicate(simple_info_TE)	Gaebolg#9	GUILD_FLAG
te_prtcas01,45,46,4	duplicate(simple_info_TE)	Gaebolg#10	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_prt_gld,131,60,6	script	Gaebolg#11	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE",1,"te_prtcas01",96,173 );
OnFlagTEGaebolg:
	flagemblem getcastledata( "te_prtcas01",1 );
	end;
}
te_prt_gld,138,68,6	duplicate(Gaebolg#11)	Gaebolg#12	GUILD_FLAG
te_prt_gld,138,60,6	duplicate(Gaebolg#11)	Gaebolg#13	GUILD_FLAG
te_prt_gld,135,60,6	duplicate(Gaebolg#11)	Gaebolg#14	GUILD_FLAG

// Guild kafra
//============================================================
te_prtcas01,96,173,0	duplicate(Kafra_Staff_TE)	Kafra Employee#te_prt01	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_prtcas01,7,205,0	duplicate(lever1_TE)	#lever1_te_prtcas01	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_prtcas01,94,200,0	duplicate(lever2_TE)	#lever2_te_prtcas01	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_prtcas01,112,181,0	duplicate(invest_TE)	Kurbe	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_prtcas01,1,1,0	duplicate(treasure_TE)	treasure_TE#te_prtcas01	-1

// Rental items npc
//============================================================
te_prt_gld,136,72,3	duplicate(rental_woe_TE)	Rental Manager#pg01	4_F_HUWOMAN

// Manager woe
//============================================================
te_prtcas01,1,1,0	duplicate(Manager_TE)	Manager_TE#Gaebolg	-1
