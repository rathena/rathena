//===== rAthena Script =======================================
//= War of Emperium (Al De Baran) - Kafragarten 4
//===== Description: =========================================
//= [Official Conversion]
//= NPCs that relate to Al De Baran Training Guild Castle 4
//===== Changelogs: ==========================================
//= 1.0 First Version. [Capuche]
//============================================================

// Simple flags (no message)
//============================================================
alde_gld,216,180,4	duplicate(flag_te)	Bennit#te_aldecas4	GUILD_FLAG

// Simple infos flags
//============================================================
te_aldecas4,167,61,2	duplicate(simple_info_TE)	Bennit#1	GUILD_FLAG
te_aldecas4,164,90,2	duplicate(simple_info_TE)	Bennit#2	GUILD_FLAG
te_aldecas4,129,193,2	duplicate(simple_info_TE)	Bennit#3	GUILD_FLAG
te_aldecas4,112,206,2	duplicate(simple_info_TE)	Bennit#4	GUILD_FLAG
te_aldecas4,113,212,2	duplicate(simple_info_TE)	Bennit#5	GUILD_FLAG
te_aldecas4,77,117,2	duplicate(simple_info_TE)	Bennit#6	GUILD_FLAG
te_aldecas4,186,42,2	duplicate(simple_info_TE)	Bennit#7	GUILD_FLAG
te_aldecas4,30,69,2	duplicate(simple_info_TE)	Bennit#8	GUILD_FLAG
te_aldecas4,55,97,2	duplicate(simple_info_TE)	Bennit#9	GUILD_FLAG
te_aldecas4,45,98,2	duplicate(simple_info_TE)	Bennit#10	GUILD_FLAG
te_aldecas4,33,116,2	duplicate(simple_info_TE)	Bennit#11	GUILD_FLAG
te_aldecas4,130,180,2	duplicate(simple_info_TE)	Bennit#12	GUILD_FLAG
te_aldecas4,129,193,2	duplicate(simple_info_TE)	Bennit#13	GUILD_FLAG
te_aldecas4,33,107,2	duplicate(simple_info_TE)	Bennit#14	GUILD_FLAG
te_aldecas4,133,220,2	duplicate(simple_info_TE)	Bennit#15	GUILD_FLAG
te_aldecas4,169,22,2	duplicate(simple_info_TE)	Bennit#16	GUILD_FLAG
te_aldecas4,169,15,2	duplicate(simple_info_TE)	Bennit#17	GUILD_FLAG

// Infos flags + warp guildmaster inside
//============================================================
te_alde_gld,239,246,2	script	Bennit#18	GUILD_FLAG,{
	// callfunc( "F_flag_woe_TE",1,"te_aldecas1",218,170 );
	callfunc( "F_flag_woe_TE",1,"te_aldecas4",45,88 );
OnFlagTEBennit:
	flagemblem getcastledata( "te_aldecas4",1 );
	end;
}
// te_alde_gld,45,87,8	duplicate(Bennit#18)	Bennit#19	GUILD_FLAG
te_alde_gld,239,239,2	duplicate(Bennit#18)	Bennit#20	GUILD_FLAG

// Guild kafra
//============================================================
te_aldecas4,45,88,0	duplicate(Kafra_Staff_TE)	Kafra Employee#te_alde4	4_F_KAFRA1

// Lever 1 (warp outside treasure)
//============================================================
te_aldecas4,79,15,0	duplicate(lever1_TE)	#lever1_te_aldecas4	HIDDEN_NPC

// Lever 2 (warp to dungeon)
//============================================================
te_aldecas4,76,64,0	duplicate(lever2_TE)	#lever2_te_aldecas4	HIDDEN_NPC

// Invest eco / def / guardian
//============================================================
te_aldecas4,67,116,0	duplicate(invest_TE)	Alpores	1_M_JOBTESTER

// Treasure spawn
//============================================================
te_aldecas4,1,1,0	duplicate(treasure_TE)	treasure_TE#te_aldecas4	-1

// Rental items npc
//============================================================
te_alde_gld,240,245,3	duplicate(rental_woe_TE)	Rental Manager#ag04	4_F_HUWOMAN

// Manager woe
//============================================================
te_aldecas4,1,1,0	duplicate(Manager_TE)	Manager_TE#Bennit	-1
