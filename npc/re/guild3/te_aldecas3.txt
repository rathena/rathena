//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Kafragarten 3
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Al De Baran Training Guild Castle 3
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
alde_gld,218,182,4	duplicate(flag_te)	Sorin#te_aldecas3	GUILD_FLAG

// Simple infos flags
//============================================================
te_aldecas3,176,175,2	duplicate(simple_info_TE)	Sorin#1	GUILD_FLAG
te_aldecas3,77,115,2	duplicate(simple_info_TE)	Sorin#2	GUILD_FLAG
te_aldecas3,77,215,2	duplicate(simple_info_TE)	Sorin#3	GUILD_FLAG
te_aldecas3,112,107,2	duplicate(simple_info_TE)	Sorin#4	GUILD_FLAG
te_aldecas3,112,117,2	duplicate(simple_info_TE)	Sorin#5	GUILD_FLAG
te_aldecas3,69,71,2	duplicate(simple_info_TE)	Sorin#6	GUILD_FLAG
te_aldecas3,85,62,6	duplicate(simple_info_TE)	Sorin#7	GUILD_FLAG
te_aldecas3,108,60,2	duplicate(simple_info_TE)	Sorin#8	GUILD_FLAG
te_aldecas3,121,73,2	duplicate(simple_info_TE)	Sorin#9	GUILD_FLAG
te_aldecas3,121,73,2	duplicate(simple_info_TE)	Sorin#10	GUILD_FLAG
te_aldecas3,75,102,2	duplicate(simple_info_TE)	Sorin#11	GUILD_FLAG
te_aldecas3,199,169,2	duplicate(simple_info_TE)	Sorin#12	GUILD_FLAG
te_aldecas3,181,179,2	duplicate(simple_info_TE)	Sorin#13	GUILD_FLAG
te_aldecas3,192,44,2	duplicate(simple_info_TE)	Sorin#14	GUILD_FLAG
te_aldecas3,208,145,2	duplicate(simple_info_TE)	Sorin#15	GUILD_FLAG
te_aldecas3,207,75,2	duplicate(simple_info_TE)	Sorin#16	GUILD_FLAG
te_aldecas3,96,62,2	duplicate(simple_info_TE)	Sorin#17	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_alde_gld,146,82,0	script	Sorin#18	GUILD_FLAG,{
	callfunc( "F_flag_woe_TE",1,"te_aldecas3",118,76 );
OnFlagTESorin:
	flagemblem getcastledata( "te_aldecas3",1 );
	end;
}
te_alde_gld,138,83,0	duplicate(Sorin#18)	Sorin#19	GUILD_FLAG

// Guild kafra
//============================================================
te_aldecas3,118,76,0	duplicate(Kafra_Staff_TE)	Kafra Employee#te_alde3	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_aldecas3,219,269,0	duplicate(lever1_TE)	#lever1_te_aldecas3	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_aldecas3,200,177,0	duplicate(lever2_TE)	#lever2_te_aldecas3	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_aldecas3,110,118,0	duplicate(invest_TE)	Valerian	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_aldecas3,1,1,0	duplicate(treasure_TE)	treasure_TE#te_aldecas3	-1

// Rental items npc
//============================================================
te_alde_gld,137,84,3	duplicate(rental_woe_TE)	Rental Manager#ag03	4_F_HUWOMAN

// Manager woe
//============================================================
te_aldecas3,1,1,0	duplicate(Manager_TE)	Manager_TE#Sorin	-1
