//===== rAthena Script =======================================
//= War of Emperium TE (Al De Baran) - Kafragarten 1
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Al De Baran Training Guild Castle 1
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
alde_gld,222,186,4	duplicate(flag_te)	Glaris#te_aldecas1	GUILD_FLAG

// Simple infos flags
//============================================================
te_aldecas1,30,248,4	duplicate(simple_info_TE)	Glaris#1	GUILD_FLAG
te_aldecas1,37,248,4	duplicate(simple_info_TE)	Glaris#2	GUILD_FLAG
te_aldecas1,37,246,4	duplicate(simple_info_TE)	Glaris#3	GUILD_FLAG
te_aldecas1,30,246,4	duplicate(simple_info_TE)	Glaris#4	GUILD_FLAG
te_aldecas1,95,80,2	duplicate(simple_info_TE)	Glaris#5	GUILD_FLAG
te_aldecas1,90,09,2	duplicate(simple_info_TE)	Glaris#6	GUILD_FLAG
te_aldecas1,62,75,2	duplicate(simple_info_TE)	Glaris#7	GUILD_FLAG
te_aldecas1,66,75,2	duplicate(simple_info_TE)	Glaris#8	GUILD_FLAG
te_aldecas1,70,75,2	duplicate(simple_info_TE)	Glaris#9	GUILD_FLAG
te_aldecas1,74,75,2	duplicate(simple_info_TE)	Glaris#10	GUILD_FLAG
te_aldecas1,62,64,2	duplicate(simple_info_TE)	Glaris#11	GUILD_FLAG
te_aldecas1,66,64,2	duplicate(simple_info_TE)	Glaris#12	GUILD_FLAG
te_aldecas1,70,64,2	duplicate(simple_info_TE)	Glaris#13	GUILD_FLAG
te_aldecas1,74,64,2	duplicate(simple_info_TE)	Glaris#14	GUILD_FLAG
te_aldecas1,74,64,2	duplicate(simple_info_TE)	Glaris#15	GUILD_FLAG
te_aldecas1,203,150,4	duplicate(simple_info_TE)	Glaris#16	GUILD_FLAG
te_aldecas1,210,150,4	duplicate(simple_info_TE)	Glaris#17	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_alde_gld,61,87,6	script	Glaris#18	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE", 1, "te_aldecas1" ,218,170 );
OnFlagTEGlaris:
	flagemblem getcastledata( "te_aldecas1",1 );
	end;
}
te_alde_gld,61,79,6	duplicate(Glaris#18)	Glaris#19	GUILD_FLAG
te_alde_gld,45,87,8	duplicate(Glaris#18)	Glaris#20	GUILD_FLAG
te_alde_gld,51,87,8	duplicate(Glaris#18)	Glaris#21	GUILD_FLAG

// Guild kafra
//============================================================
te_aldecas1,218,170,0	duplicate(Kafra_Staff_TE)	Kafra Employee#te_alde1	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_aldecas1,123,223,0	duplicate(lever1_TE)	#lever1_te_aldecas1	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_aldecas1,211,181,0	duplicate(lever2_TE)	#lever2_te_aldecas1	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_aldecas1,218,175,0	duplicate(invest_TE)	Clode	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_aldecas1,1,1,0	duplicate(treasure_TE)	treasure_TE#te_aldecas1	-1

// Rental items npc
//============================================================
te_alde_gld,45,84,3	duplicate(rental_woe_TE)	Rental Manager#ag01	4_F_HUWOMAN

// Manager woe
//============================================================
te_aldecas1,1,1,0	duplicate(Manager_TE)	Manager_TE#Glaris	-1
