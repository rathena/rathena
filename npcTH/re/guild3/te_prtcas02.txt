//===== rAthena Script =======================================
//= War of Emperium (Prontera) - Gloria 2
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Prontera Training Guild Castle 2
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
prt_gld,116,155,4	duplicate(flag_te)	Richard#te_prtcas02	GUILD_FLAG

// Simple infos flags
//============================================================
te_prtcas02,40,227,4	duplicate(simple_info_TE)	Richard#1	GUILD_FLAG
te_prtcas02,46,227,4	duplicate(simple_info_TE)	Richard#2	GUILD_FLAG
te_prtcas02,11,219,4	duplicate(simple_info_TE)	Richard#3	GUILD_FLAG
te_prtcas02,11,214,4	duplicate(simple_info_TE)	Richard#4	GUILD_FLAG
te_prtcas02,20,219,4	duplicate(simple_info_TE)	Richard#5	GUILD_FLAG
te_prtcas02,20,214,4	duplicate(simple_info_TE)	Richard#6	GUILD_FLAG
te_prtcas02,79,227,8	duplicate(simple_info_TE)	Richard#7	GUILD_FLAG
te_prtcas02,70,227,8	duplicate(simple_info_TE)	Richard#8	GUILD_FLAG
te_prtcas02,38,189,8	duplicate(simple_info_TE)	Richard#9	GUILD_FLAG
te_prtcas02,34,189,8	duplicate(simple_info_TE)	Richard#10	GUILD_FLAG
te_prtcas02,153,161,4	duplicate(simple_info_TE)	Richard#11	GUILD_FLAG
te_prtcas02,162,161,4	duplicate(simple_info_TE)	Richard#12	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_prt_gld,244,126,8	script	Richard#13	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE",1,"te_prtcas02",71,36 );
OnFlagTERichard:
	flagemblem getcastledata( "te_prtcas02",1 );
	end;
}
te_prt_gld,244,128,8	duplicate(Richard#13)	Richard#14	GUILD_FLAG
te_prt_gld,236,126,8	duplicate(Richard#13)	Richard#15	GUILD_FLAG
te_prt_gld,236,128,8	duplicate(Richard#13)	Richard#16	GUILD_FLAG

// Guild kafra
//============================================================
te_prtcas02,71,36,4	duplicate(Kafra_Staff_TE)	Kafra Employee#te_prt02	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_prtcas02,206,228,0	duplicate(lever1_TE)	#lever1_te_prtcas02	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_prtcas02,84,72,0	duplicate(lever2_TE)	#lever2_te_prtcas02	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_prtcas02,94,61,4	duplicate(invest_TE)	Kamiyu	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_prtcas02,1,1,0	duplicate(treasure_TE)	treasure_TE#te_prtcas02	-1

// Rental items npc
//============================================================
te_prt_gld,246,134,3	duplicate(rental_woe_TE)	Rental Manager#pg02	4_F_HUWOMAN

// Manager woe
//============================================================
te_prtcas02,1,1,0	duplicate(Manager_TE)	Manager_TE#Richard	-1
