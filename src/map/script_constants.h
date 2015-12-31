// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _SCRIPT_CONSTANTS_H_
	#define _SCRIPT_CONSTANTS_H_

	#define export_constant(a) script_set_constant(###a,a,false)

	/* server defines */
	export_constant(PACKETVER);
	export_constant(MAX_LEVEL);
	export_constant(MAX_STORAGE);
	export_constant(MAX_INVENTORY);
	export_constant(MAX_CART);
	export_constant(MAX_ZENY);
	export_constant(MAX_PARTY);
	export_constant(MAX_GUILD);
	export_constant(MAX_GUILDLEVEL);
	export_constant(MAX_GUILD_STORAGE);
	export_constant(MAX_BG_MEMBERS);
	export_constant(MAX_CHAT_USERS);
	export_constant(VIP_SCRIPT);
	export_constant(MIN_STORAGE);

	/* jobs */
	export_constant(JOB_NOVICE);
	export_constant(JOB_SWORDMAN);
	export_constant(JOB_MAGE);
	export_constant(JOB_ARCHER);
	export_constant(JOB_ACOLYTE);
	export_constant(JOB_MERCHANT);
	export_constant(JOB_THIEF);
	export_constant(JOB_KNIGHT);
	export_constant(JOB_PRIEST);
	export_constant(JOB_WIZARD);
	export_constant(JOB_BLACKSMITH);
	export_constant(JOB_HUNTER);
	export_constant(JOB_ASSASSIN);
	export_constant(JOB_KNIGHT2);
	export_constant(JOB_CRUSADER);
	export_constant(JOB_MONK);
	export_constant(JOB_SAGE);
	export_constant(JOB_ROGUE);
	export_constant(JOB_ALCHEMIST);
	// For backwards compatability
	script_set_constant("Job_Alchem",JOB_ALCHEMIST,false); // TODO: Check if this is used anywhere and remove if possible
	export_constant(JOB_BARD);
	export_constant(JOB_DANCER);
	export_constant(JOB_CRUSADER2);
	export_constant(JOB_WEDDING);
	export_constant(JOB_SUPER_NOVICE);
	// For backwards compatability
	script_set_constant("Job_SuperNovice",JOB_SUPER_NOVICE,false); // TODO: Refactor NPCs to use the new constant
	export_constant(JOB_GUNSLINGER);
	export_constant(JOB_NINJA);
	export_constant(JOB_XMAS);
	export_constant(JOB_SUMMER);
	export_constant(JOB_HANBOK);
	export_constant(JOB_OKTOBERFEST);

	export_constant(JOB_NOVICE_HIGH);
	export_constant(JOB_SWORDMAN_HIGH);
	export_constant(JOB_MAGE_HIGH);
	export_constant(JOB_ARCHER_HIGH);
	export_constant(JOB_ACOLYTE_HIGH);
	export_constant(JOB_MERCHANT_HIGH);
	export_constant(JOB_THIEF_HIGH);
	export_constant(JOB_LORD_KNIGHT);
	export_constant(JOB_HIGH_PRIEST);
	export_constant(JOB_HIGH_WIZARD);
	export_constant(JOB_WHITESMITH);
	export_constant(JOB_SNIPER);
	export_constant(JOB_ASSASSIN_CROSS);
	export_constant(JOB_LORD_KNIGHT2);
	export_constant(JOB_PALADIN);
	export_constant(JOB_CHAMPION);
	export_constant(JOB_PROFESSOR);
	export_constant(JOB_STALKER);
	export_constant(JOB_CREATOR);
	export_constant(JOB_CLOWN);
	export_constant(JOB_GYPSY);
	export_constant(JOB_PALADIN2);

	export_constant(JOB_BABY);
	export_constant(JOB_BABY_SWORDMAN);
	export_constant(JOB_BABY_MAGE);
	export_constant(JOB_BABY_ARCHER);
	export_constant(JOB_BABY_ACOLYTE);
	export_constant(JOB_BABY_MERCHANT);
	export_constant(JOB_BABY_THIEF);
	export_constant(JOB_BABY_KNIGHT);
	export_constant(JOB_BABY_PRIEST);
	export_constant(JOB_BABY_WIZARD);
	export_constant(JOB_BABY_BLACKSMITH);
	export_constant(JOB_BABY_HUNTER);
	export_constant(JOB_BABY_ASSASSIN);
	export_constant(JOB_BABY_KNIGHT2);
	export_constant(JOB_BABY_CRUSADER);
	export_constant(JOB_BABY_MONK);
	export_constant(JOB_BABY_SAGE);
	export_constant(JOB_BABY_ROGUE);
	export_constant(JOB_BABY_ALCHEMIST);
	// For backwards compatability
	script_set_constant("Job_Baby_Alchem",JOB_BABY_ALCHEMIST,false); // TODO: Check if this is used anywhere and remove if possible
	export_constant(JOB_BABY_BARD);
	export_constant(JOB_BABY_DANCER);
	export_constant(JOB_BABY_CRUSADER2);
	export_constant(JOB_SUPER_BABY);

	export_constant(JOB_TAEKWON);
	export_constant(JOB_STAR_GLADIATOR);
	export_constant(JOB_STAR_GLADIATOR2);
	export_constant(JOB_SOUL_LINKER);

	export_constant(JOB_GANGSI);
	export_constant(JOB_DEATH_KNIGHT);
	export_constant(JOB_DARK_COLLECTOR);

	export_constant(JOB_RUNE_KNIGHT);
	export_constant(JOB_WARLOCK);
	export_constant(JOB_RANGER);
	export_constant(JOB_ARCH_BISHOP);
	export_constant(JOB_MECHANIC);
	export_constant(JOB_GUILLOTINE_CROSS);

	export_constant(JOB_RUNE_KNIGHT_T);
	export_constant(JOB_WARLOCK_T);
	export_constant(JOB_RANGER_T);
	export_constant(JOB_ARCH_BISHOP_T);
	export_constant(JOB_MECHANIC_T);
	export_constant(JOB_GUILLOTINE_CROSS_T);

	export_constant(JOB_ROYAL_GUARD);
	export_constant(JOB_SORCERER);
	export_constant(JOB_MINSTREL);
	export_constant(JOB_WANDERER);
	export_constant(JOB_SURA);
	export_constant(JOB_GENETIC);
	export_constant(JOB_SHADOW_CHASER);

	export_constant(JOB_ROYAL_GUARD_T);
	export_constant(JOB_SORCERER_T);
	export_constant(JOB_MINSTREL_T);
	export_constant(JOB_WANDERER_T);
	export_constant(JOB_SURA_T);
	export_constant(JOB_GENETIC_T);
	export_constant(JOB_SHADOW_CHASER_T);

	export_constant(JOB_RUNE_KNIGHT2);
	export_constant(JOB_RUNE_KNIGHT_T2);
	export_constant(JOB_ROYAL_GUARD2);
	export_constant(JOB_ROYAL_GUARD_T2);
	export_constant(JOB_RANGER2);
	export_constant(JOB_RANGER_T2);
	export_constant(JOB_MECHANIC2);
	export_constant(JOB_MECHANIC_T2);

	export_constant(JOB_BABY_RUNE);
	export_constant(JOB_BABY_WARLOCK);
	export_constant(JOB_BABY_RANGER);
	export_constant(JOB_BABY_BISHOP);
	export_constant(JOB_BABY_MECHANIC);
	export_constant(JOB_BABY_CROSS);
	export_constant(JOB_BABY_GUARD);
	export_constant(JOB_BABY_SORCERER);
	export_constant(JOB_BABY_MINSTREL);
	export_constant(JOB_BABY_WANDERER);
	export_constant(JOB_BABY_SURA);
	export_constant(JOB_BABY_GENETIC);
	export_constant(JOB_BABY_CHASER);

	export_constant(JOB_BABY_RUNE2);
	export_constant(JOB_BABY_GUARD2);
	export_constant(JOB_BABY_RANGER2);
	export_constant(JOB_BABY_MECHANIC2);

	export_constant(JOB_SUPER_NOVICE_E);
	export_constant(JOB_SUPER_BABY_E);

	export_constant(JOB_KAGEROU);
	export_constant(JOB_OBORO);

	export_constant(JOB_REBELLION);

	/* EA jobs */
	script_set_constant("EAJL_2_1",JOBL_2_1,false);
	script_set_constant("EAJL_2_2",JOBL_2_2,false);
	script_set_constant("EAJL_2",JOBL_2,false);
	script_set_constant("EAJL_UPPER",JOBL_UPPER,false);
	script_set_constant("EAJL_BABY",JOBL_BABY,false);
	script_set_constant("EAJL_THIRD",JOBL_THIRD,false);

	script_set_constant("EAJ_BASEMASK",MAPID_BASEMASK,false);
	script_set_constant("EAJ_UPPERMASK",MAPID_UPPERMASK,false);
	script_set_constant("EAJ_THIRDMASK",MAPID_THIRDMASK,false);

	script_set_constant("EAJ_NOVICE",MAPID_NOVICE,false);
	script_set_constant("EAJ_SWORDMAN",MAPID_SWORDMAN,false);
	script_set_constant("EAJ_MAGE",MAPID_MAGE,false);
	script_set_constant("EAJ_ARCHER",MAPID_ARCHER,false);
	script_set_constant("EAJ_ACOLYTE",MAPID_ACOLYTE,false);
	script_set_constant("EAJ_MERCHANT",MAPID_MERCHANT,false);
	script_set_constant("EAJ_THIEF",MAPID_THIEF,false);
	script_set_constant("EAJ_TAEKWON",MAPID_TAEKWON,false);
	script_set_constant("EAJ_GUNSLINGER",MAPID_GUNSLINGER,false);
	script_set_constant("EAJ_NINJA",MAPID_NINJA,false);
	script_set_constant("EAJ_GANGSI",MAPID_GANGSI,false);

	script_set_constant("EAJ_KNIGHT",MAPID_KNIGHT,false);
	script_set_constant("EAJ_WIZARD",MAPID_WIZARD,false);
	script_set_constant("EAJ_HUNTER",MAPID_HUNTER,false);
	script_set_constant("EAJ_PRIEST",MAPID_PRIEST,false);
	script_set_constant("EAJ_BLACKSMITH",MAPID_BLACKSMITH,false);
	script_set_constant("EAJ_ASSASSIN",MAPID_ASSASSIN,false);
	script_set_constant("EAJ_STAR_GLADIATOR",MAPID_STAR_GLADIATOR,false);
	script_set_constant("EAJ_REBELLION",MAPID_REBELLION,false);
	script_set_constant("EAJ_KAGEROUOBORO",MAPID_KAGEROUOBORO,false);
	script_set_constant("EAJ_DEATH_KNIGHT",MAPID_DEATH_KNIGHT,false);

	script_set_constant("EAJ_CRUSADER",MAPID_CRUSADER,false);
	script_set_constant("EAJ_SAGE",MAPID_SAGE,false);
	script_set_constant("EAJ_BARDDANCER",MAPID_BARDDANCER,false);
	script_set_constant("EAJ_MONK",MAPID_MONK,false);
	script_set_constant("EAJ_ALCHEMIST",MAPID_ALCHEMIST,false);
	script_set_constant("EAJ_ROGUE",MAPID_ROGUE,false);
	script_set_constant("EAJ_SOUL_LINKER",MAPID_SOUL_LINKER,false);
	script_set_constant("EAJ_DARK_COLLECTOR",MAPID_DARK_COLLECTOR,false);

	script_set_constant("EAJ_NOVICE_HIGH",MAPID_NOVICE_HIGH,false);
	script_set_constant("EAJ_SWORDMAN_HIGH",MAPID_SWORDMAN_HIGH,false);
	script_set_constant("EAJ_MAGE_HIGH",MAPID_MAGE_HIGH,false);
	script_set_constant("EAJ_ARCHER_HIGH",MAPID_ARCHER_HIGH,false);
	script_set_constant("EAJ_ACOLYTE_HIGH",MAPID_ACOLYTE_HIGH,false);
	script_set_constant("EAJ_MERCHANT_HIGH",MAPID_MERCHANT_HIGH,false);
	script_set_constant("EAJ_THIEF_HIGH",MAPID_THIEF_HIGH,false);

	script_set_constant("EAJ_LORD_KNIGHT",MAPID_LORD_KNIGHT,false);
	script_set_constant("EAJ_HIGH_WIZARD",MAPID_HIGH_WIZARD,false);
	script_set_constant("EAJ_SNIPER",MAPID_SNIPER,false);
	script_set_constant("EAJ_HIGH_PRIEST",MAPID_HIGH_PRIEST,false);
	script_set_constant("EAJ_WHITESMITH",MAPID_WHITESMITH,false);
	script_set_constant("EAJ_ASSASSIN_CROSS",MAPID_ASSASSIN_CROSS,false);

	script_set_constant("EAJ_PALADIN",MAPID_PALADIN,false);
	script_set_constant("EAJ_PROFESSOR",MAPID_PROFESSOR,false);
	script_set_constant("EAJ_CLOWNGYPSY",MAPID_CLOWNGYPSY,false);
	script_set_constant("EAJ_CHAMPION",MAPID_CHAMPION,false);
	script_set_constant("EAJ_CREATOR",MAPID_CREATOR,false);
	script_set_constant("EAJ_STALKER",MAPID_STALKER,false);

	script_set_constant("EAJ_BABY",MAPID_BABY,false);
	script_set_constant("EAJ_BABY_SWORDMAN",MAPID_BABY_SWORDMAN,false);
	script_set_constant("EAJ_BABY_MAGE",MAPID_BABY_MAGE,false);
	script_set_constant("EAJ_BABY_ARCHER",MAPID_BABY_ARCHER,false);
	script_set_constant("EAJ_BABY_ACOLYTE",MAPID_BABY_ACOLYTE,false);
	script_set_constant("EAJ_BABY_MERCHANT",MAPID_BABY_MERCHANT,false);
	script_set_constant("EAJ_BABY_THIEF",MAPID_BABY_THIEF,false);

	script_set_constant("EAJ_BABY_KNIGHT",MAPID_BABY_KNIGHT,false);
	script_set_constant("EAJ_BABY_WIZARD",MAPID_BABY_WIZARD,false);
	script_set_constant("EAJ_BABY_HUNTER",MAPID_BABY_HUNTER,false);
	script_set_constant("EAJ_BABY_PRIEST",MAPID_BABY_PRIEST,false);
	script_set_constant("EAJ_BABY_BLACKSMITH",MAPID_BABY_BLACKSMITH,false);
	script_set_constant("EAJ_BABY_ASSASSIN",MAPID_BABY_ASSASSIN,false);

	script_set_constant("EAJ_BABY_CRUSADER",MAPID_BABY_CRUSADER,false);
	script_set_constant("EAJ_BABY_SAGE",MAPID_BABY_SAGE,false);
	script_set_constant("EAJ_BABY_BARDDANCER",MAPID_BABY_BARDDANCER,false);
	script_set_constant("EAJ_BABY_MONK",MAPID_BABY_MONK,false);
	script_set_constant("EAJ_BABY_ALCHEMIST",MAPID_BABY_ALCHEMIST,false);
	script_set_constant("EAJ_BABY_ROGUE",MAPID_BABY_ROGUE,false);

	script_set_constant("EAJ_RUNE_KNIGHT",MAPID_RUNE_KNIGHT,false);
	script_set_constant("EAJ_WARLOCK",MAPID_WARLOCK,false);
	script_set_constant("EAJ_RANGER",MAPID_RANGER,false);
	script_set_constant("EAJ_ARCH_BISHOP",MAPID_ARCH_BISHOP,false);
	script_set_constant("EAJ_MECHANIC",MAPID_MECHANIC,false);
	script_set_constant("EAJ_GUILLOTINE_CROSS",MAPID_GUILLOTINE_CROSS,false);

	script_set_constant("EAJ_ROYAL_GUARD",MAPID_ROYAL_GUARD,false);
	script_set_constant("EAJ_SORCERER",MAPID_SORCERER,false);
	script_set_constant("EAJ_MINSTRELWANDERER",MAPID_MINSTRELWANDERER,false);
	script_set_constant("EAJ_SURA",MAPID_SURA,false);
	script_set_constant("EAJ_GENETIC",MAPID_GENETIC,false);
	script_set_constant("EAJ_SHADOW_CHASER",MAPID_SHADOW_CHASER,false);

	script_set_constant("EAJ_RUNE_KNIGHT_T",MAPID_RUNE_KNIGHT_T,false);
	script_set_constant("EAJ_WARLOCK_T",MAPID_WARLOCK_T,false);
	script_set_constant("EAJ_RANGER_T",MAPID_RANGER_T,false);
	script_set_constant("EAJ_ARCH_BISHOP_T",MAPID_ARCH_BISHOP_T,false);
	script_set_constant("EAJ_MECHANIC_T",MAPID_MECHANIC_T,false);
	script_set_constant("EAJ_GUILLOTINE_CROSS_T",MAPID_GUILLOTINE_CROSS_T,false);

	script_set_constant("EAJ_ROYAL_GUARD_T",MAPID_ROYAL_GUARD_T,false);
	script_set_constant("EAJ_SORCERER_T",MAPID_SORCERER_T,false);
	script_set_constant("EAJ_MINSTRELWANDERER_T",MAPID_MINSTRELWANDERER_T,false);
	script_set_constant("EAJ_SURA_T",MAPID_SURA_T,false);
	script_set_constant("EAJ_GENETIC_T",MAPID_GENETIC_T,false);
	script_set_constant("EAJ_SHADOW_CHASER_T",MAPID_SHADOW_CHASER_T,false);

	script_set_constant("EAJ_BABY_RUNE",MAPID_BABY_RUNE,false);
	script_set_constant("EAJ_BABY_WARLOCK",MAPID_BABY_WARLOCK,false);
	script_set_constant("EAJ_BABY_RANGER",MAPID_BABY_RANGER,false);
	script_set_constant("EAJ_BABY_BISHOP",MAPID_BABY_BISHOP,false);
	script_set_constant("EAJ_BABY_MECHANIC",MAPID_BABY_MECHANIC,false);
	script_set_constant("EAJ_BABY_CROSS",MAPID_BABY_CROSS,false);

	script_set_constant("EAJ_BABY_GUARD",MAPID_BABY_GUARD,false);
	script_set_constant("EAJ_BABY_SORCERER",MAPID_BABY_SORCERER,false);
	script_set_constant("EAJ_BABY_MINSTRELWANDERER",MAPID_BABY_MINSTRELWANDERER,false);
	script_set_constant("EAJ_BABY_SURA",MAPID_BABY_SURA,false);
	script_set_constant("EAJ_BABY_GENETIC",MAPID_BABY_GENETIC,false);
	script_set_constant("EAJ_BABY_CHASER",MAPID_BABY_CHASER,false);

	script_set_constant("EAJ_SUPER_NOVICE",MAPID_SUPER_NOVICE,false);
	script_set_constant("EAJ_SUPER_BABY",MAPID_SUPER_BABY,false);
	script_set_constant("EAJ_SUPER_NOVICE_E",MAPID_SUPER_NOVICE_E,false);
	script_set_constant("EAJ_SUPER_BABY_E",MAPID_SUPER_BABY_E,false);

	/* broadcasts */
	export_constant(BC_ALL);
	export_constant(BC_MAP);
	export_constant(BC_AREA);
	export_constant(BC_SELF);
	export_constant(BC_PC);
	export_constant(BC_NPC);
	export_constant(BC_YELLOW);
	export_constant(BC_BLUE);
	export_constant(BC_WOE);

	/* mapflags */
	export_constant(MF_NOMEMO);
	export_constant(MF_NOTELEPORT);
	export_constant(MF_NOSAVE);
	export_constant(MF_NOBRANCH);
	export_constant(MF_NOPENALTY);
	export_constant(MF_NOZENYPENALTY);
	export_constant(MF_PVP);
	export_constant(MF_PVP_NOPARTY);
	export_constant(MF_PVP_NOGUILD);
	export_constant(MF_GVG);
	export_constant(MF_GVG_NOPARTY);
	export_constant(MF_NOTRADE);
	export_constant(MF_NOSKILL);
	export_constant(MF_NOWARP);
	export_constant(MF_PARTYLOCK);
	export_constant(MF_NOICEWALL);
	export_constant(MF_SNOW);
	export_constant(MF_FOG);
	export_constant(MF_SAKURA);
	export_constant(MF_LEAVES);
	// Intentionally disabled mf_rain because Ind removed it source side
	//export_constant(MF_RAIN);
	export_constant(MF_NOGO);
	export_constant(MF_CLOUDS);
	export_constant(MF_CLOUDS2);
	export_constant(MF_FIREWORKS);
	export_constant(MF_GVG_CASTLE);
	export_constant(MF_GVG_DUNGEON);
	export_constant(MF_NIGHTENABLED);
	export_constant(MF_NOBASEEXP);
	export_constant(MF_NOJOBEXP);
	export_constant(MF_NOMOBLOOT);
	export_constant(MF_NOMVPLOOT);
	export_constant(MF_NORETURN);
	export_constant(MF_NOWARPTO);
	export_constant(MF_NIGHTMAREDROP);
	export_constant(MF_RESTRICTED);
	export_constant(MF_NOCOMMAND);
	export_constant(MF_NODROP);
	export_constant(MF_JEXP);
	export_constant(MF_BEXP);
	export_constant(MF_NOVENDING);
	export_constant(MF_LOADEVENT);
	export_constant(MF_NOCHAT);
	export_constant(MF_NOEXPPENALTY);
	export_constant(MF_GUILDLOCK);
	export_constant(MF_TOWN);
	export_constant(MF_AUTOTRADE);
	export_constant(MF_ALLOWKS);
	export_constant(MF_MONSTER_NOTELEPORT);
	export_constant(MF_PVP_NOCALCRANK);
	export_constant(MF_BATTLEGROUND);
	export_constant(MF_RESET);
	// TODO: Check why it is called differently on source and const
	//export_constant(MF_CHANNELAUTOJOIN);
	script_set_constant("mf_nomapchannelautojoin",MF_CHANNELAUTOJOIN,false);
	export_constant(MF_NOUSECART);
	export_constant(MF_NOITEMCONSUMPTION);
	export_constant(MF_SUMSTARTMIRACLE);
	export_constant(MF_NOMINEEFFECT);
	export_constant(MF_NOLOCKON);
	export_constant(MF_NOTOMB);
	export_constant(MF_SKILL_DAMAGE);

	/* setcell types */
	export_constant(CELL_WALKABLE);
	export_constant(CELL_SHOOTABLE);
	export_constant(CELL_WATER);
	export_constant(CELL_NPC);
	export_constant(CELL_BASILICA);
	export_constant(CELL_LANDPROTECTOR);
	export_constant(CELL_NOVENDING);
	export_constant(CELL_NOCHAT);
	export_constant(CELL_MAELSTROM);
	export_constant(CELL_ICEWALL);

	/* getcell types */
	export_constant(CELL_CHKWALL);
	export_constant(CELL_CHKWATER);
	export_constant(CELL_CHKCLIFF);
	export_constant(CELL_CHKPASS);
	export_constant(CELL_CHKREACH);
	export_constant(CELL_CHKNOPASS);
	export_constant(CELL_CHKNOREACH);
	// TODO: Check why this was commented until now
	//export_constant(CELL_CHKSTACK);
	export_constant(CELL_CHKNPC);
	export_constant(CELL_CHKBASILICA);
	export_constant(CELL_CHKLANDPROTECTOR);
	export_constant(CELL_CHKNOVENDING);
	export_constant(CELL_CHKNOCHAT);
	export_constant(CELL_CHKMAELSTROM);
	export_constant(CELL_CHKICEWALL);

	/* status options */
	export_constant(OPTION_NOTHING);
	export_constant(OPTION_SIGHT);
	export_constant(OPTION_HIDE);
	export_constant(OPTION_CLOAK);
	export_constant(OPTION_FALCON);
	export_constant(OPTION_RIDING);
	export_constant(OPTION_INVISIBLE);
	export_constant(OPTION_ORCISH);
	export_constant(OPTION_WEDDING);
	export_constant(OPTION_RUWACH);
	export_constant(OPTION_CHASEWALK);
	export_constant(OPTION_FLYING);
	export_constant(OPTION_XMAS);
	export_constant(OPTION_TRANSFORM);
	export_constant(OPTION_SUMMER);
	export_constant(OPTION_DRAGON1);
	export_constant(OPTION_WUG);
	export_constant(OPTION_WUGRIDER);
	export_constant(OPTION_MADOGEAR);
	export_constant(OPTION_DRAGON2);
	export_constant(OPTION_DRAGON3);
	export_constant(OPTION_DRAGON4);
	export_constant(OPTION_DRAGON5);
	export_constant(OPTION_HANBOK);
	export_constant(OPTION_OKTOBERFEST);

	/* status option compounds */
	export_constant(OPTION_DRAGON);
	export_constant(OPTION_COSTUME);

	/* sc_start flags */
	export_constant(SCSTART_NONE);
	export_constant(SCSTART_NOAVOID);
	export_constant(SCSTART_NOTICKDEF);
	export_constant(SCSTART_LOADED);
	export_constant(SCSTART_NORATEDEF);
	export_constant(SCSTART_NOICON);

	/* unit control - mob */
	export_constant(UMOB_SIZE);
	export_constant(UMOB_LEVEL);
	export_constant(UMOB_HP);
	export_constant(UMOB_MAXHP);
	export_constant(UMOB_MASTERAID);
	export_constant(UMOB_MAPID);
	export_constant(UMOB_X);
	export_constant(UMOB_Y);
	export_constant(UMOB_SPEED);
	export_constant(UMOB_MODE);
	export_constant(UMOB_AI);
	export_constant(UMOB_SCOPTION);
	export_constant(UMOB_SEX);
	export_constant(UMOB_CLASS);
	export_constant(UMOB_HAIRSTYLE);
	export_constant(UMOB_HAIRCOLOR);
	export_constant(UMOB_HEADBOTTOM);
	export_constant(UMOB_HEADMIDDLE);
	export_constant(UMOB_HEADTOP);
	export_constant(UMOB_CLOTHCOLOR);
	export_constant(UMOB_SHIELD);
	export_constant(UMOB_WEAPON);
	export_constant(UMOB_LOOKDIR);
	export_constant(UMOB_STR);
	export_constant(UMOB_AGI);
	export_constant(UMOB_VIT);
	export_constant(UMOB_INT);
	export_constant(UMOB_DEX);
	export_constant(UMOB_LUK);
	export_constant(UMOB_SLAVECPYMSTRMD);
	export_constant(UMOB_DMGIMMUNE);
	export_constant(UMOB_ATKRANGE);
	export_constant(UMOB_ATKMIN);
	export_constant(UMOB_ATKMAX);
	export_constant(UMOB_MATKMIN);
	export_constant(UMOB_MATKMAX);
	export_constant(UMOB_DEF);
	export_constant(UMOB_MDEF);
	export_constant(UMOB_HIT);
	export_constant(UMOB_FLEE);
	export_constant(UMOB_PDODGE);
	export_constant(UMOB_CRIT);
	export_constant(UMOB_RACE);
	export_constant(UMOB_ELETYPE);
	export_constant(UMOB_ELELEVEL);
	export_constant(UMOB_AMOTION);
	export_constant(UMOB_ADELAY);
	export_constant(UMOB_DMOTION);

	/* unit control - homunculus */
	export_constant(UHOM_SIZE);
	export_constant(UHOM_LEVEL);
	export_constant(UHOM_HP);
	export_constant(UHOM_MAXHP);
	export_constant(UHOM_SP);
	export_constant(UHOM_MAXSP);
	export_constant(UHOM_MASTERCID);
	export_constant(UHOM_MAPID);
	export_constant(UHOM_X);
	export_constant(UHOM_Y);
	export_constant(UHOM_HUNGER);
	export_constant(UHOM_INTIMACY);
	export_constant(UHOM_SPEED);
	export_constant(UHOM_LOOKDIR);
	export_constant(UHOM_CANMOVETICK);
	export_constant(UHOM_STR);
	export_constant(UHOM_AGI);
	export_constant(UHOM_VIT);
	export_constant(UHOM_INT);
	export_constant(UHOM_DEX);
	export_constant(UHOM_LUK);
	export_constant(UHOM_DMGIMMUNE);
	export_constant(UHOM_ATKRANGE);
	export_constant(UHOM_ATKMIN);
	export_constant(UHOM_ATKMAX);
	export_constant(UHOM_MATKMIN);
	export_constant(UHOM_MATKMAX);
	export_constant(UHOM_DEF);
	export_constant(UHOM_MDEF);
	export_constant(UHOM_HIT);
	export_constant(UHOM_FLEE);
	export_constant(UHOM_PDODGE);
	export_constant(UHOM_CRIT);
	export_constant(UHOM_RACE);
	export_constant(UHOM_ELETYPE);
	export_constant(UHOM_ELELEVEL);
	export_constant(UHOM_AMOTION);
	export_constant(UHOM_ADELAY);
	export_constant(UHOM_DMOTION);

	/* unit control - pet */
	export_constant(UPET_SIZE);
	export_constant(UPET_LEVEL);
	export_constant(UPET_HP);
	export_constant(UPET_MAXHP);
	export_constant(UPET_MASTERAID);
	export_constant(UPET_MAPID);
	export_constant(UPET_X);
	export_constant(UPET_Y);
	export_constant(UPET_HUNGER);
	export_constant(UPET_INTIMACY);
	export_constant(UPET_SPEED);
	export_constant(UPET_LOOKDIR);
	export_constant(UPET_CANMOVETICK);
	export_constant(UPET_STR);
	export_constant(UPET_AGI);
	export_constant(UPET_VIT);
	export_constant(UPET_INT);
	export_constant(UPET_DEX);
	export_constant(UPET_LUK);
	export_constant(UPET_DMGIMMUNE);
	export_constant(UPET_ATKRANGE);
	export_constant(UPET_ATKMIN);
	export_constant(UPET_ATKMAX);
	export_constant(UPET_MATKMIN);
	export_constant(UPET_MATKMAX);
	export_constant(UPET_DEF);
	export_constant(UPET_MDEF);
	export_constant(UPET_HIT);
	export_constant(UPET_FLEE);
	export_constant(UPET_PDODGE);
	export_constant(UPET_CRIT);
	export_constant(UPET_RACE);
	export_constant(UPET_ELETYPE);
	export_constant(UPET_ELELEVEL);
	export_constant(UPET_AMOTION);
	export_constant(UPET_ADELAY);
	export_constant(UPET_DMOTION);

	/* unit control - mercenary */
	export_constant(UMER_SIZE);
	export_constant(UMER_HP);
	export_constant(UMER_MAXHP);
	export_constant(UMER_MASTERCID);
	export_constant(UMER_MAPID);
	export_constant(UMER_X);
	export_constant(UMER_Y);
	export_constant(UMER_KILLCOUNT);
	export_constant(UMER_LIFETIME);
	export_constant(UMER_SPEED);
	export_constant(UMER_LOOKDIR);
	export_constant(UMER_CANMOVETICK);
	export_constant(UMER_STR);
	export_constant(UMER_AGI);
	export_constant(UMER_VIT);
	export_constant(UMER_INT);
	export_constant(UMER_DEX);
	export_constant(UMER_LUK);
	export_constant(UMER_DMGIMMUNE);
	export_constant(UMER_ATKRANGE);
	export_constant(UMER_ATKMIN);
	export_constant(UMER_ATKMAX);
	export_constant(UMER_MATKMIN);
	export_constant(UMER_MATKMAX);
	export_constant(UMER_DEF);
	export_constant(UMER_MDEF);
	export_constant(UMER_HIT);
	export_constant(UMER_FLEE);
	export_constant(UMER_PDODGE);
	export_constant(UMER_CRIT);
	export_constant(UMER_RACE);
	export_constant(UMER_ELETYPE);
	export_constant(UMER_ELELEVEL);
	export_constant(UMER_AMOTION);
	export_constant(UMER_ADELAY);
	export_constant(UMER_DMOTION);

	/* unit control - elemental */
	export_constant(UELE_SIZE);
	export_constant(UELE_HP);
	export_constant(UELE_MAXHP);
	export_constant(UELE_SP);
	export_constant(UELE_MAXSP);
	export_constant(UELE_MASTERCID);
	export_constant(UELE_MAPID);
	export_constant(UELE_X);
	export_constant(UELE_Y);
	export_constant(UELE_LIFETIME);
	export_constant(UELE_MODE);
	export_constant(UELE_SPEED);
	export_constant(UELE_LOOKDIR);
	export_constant(UELE_CANMOVETICK);
	export_constant(UELE_STR);
	export_constant(UELE_AGI);
	export_constant(UELE_VIT);
	export_constant(UELE_INT);
	export_constant(UELE_DEX);
	export_constant(UELE_LUK);
	export_constant(UELE_DMGIMMUNE);
	export_constant(UELE_ATKRANGE);
	export_constant(UELE_ATKMIN);
	export_constant(UELE_ATKMAX);
	export_constant(UELE_MATKMIN);
	export_constant(UELE_MATKMAX);
	export_constant(UELE_DEF);
	export_constant(UELE_MDEF);
	export_constant(UELE_HIT);
	export_constant(UELE_FLEE);
	export_constant(UELE_PDODGE);
	export_constant(UELE_CRIT);
	export_constant(UELE_RACE);
	export_constant(UELE_ELETYPE);
	export_constant(UELE_ELELEVEL);
	export_constant(UELE_AMOTION);
	export_constant(UELE_ADELAY);
	export_constant(UELE_DMOTION);

	/* unit control - NPC */
	export_constant(UNPC_DISPLAY);
	export_constant(UNPC_LEVEL);
	export_constant(UNPC_HP);
	export_constant(UNPC_MAXHP);
	export_constant(UNPC_MAPID);
	export_constant(UNPC_X);
	export_constant(UNPC_Y);
	export_constant(UNPC_LOOKDIR);
	export_constant(UNPC_STR);
	export_constant(UNPC_AGI);
	export_constant(UNPC_VIT);
	export_constant(UNPC_INT);
	export_constant(UNPC_DEX);
	export_constant(UNPC_LUK);
	export_constant(UNPC_PLUSALLSTAT);
	export_constant(UNPC_DMGIMMUNE);
	export_constant(UNPC_ATKRANGE);
	export_constant(UNPC_ATKMIN);
	export_constant(UNPC_ATKMAX);
	export_constant(UNPC_MATKMIN);
	export_constant(UNPC_MATKMAX);
	export_constant(UNPC_DEF);
	export_constant(UNPC_MDEF);
	export_constant(UNPC_HIT);
	export_constant(UNPC_FLEE);
	export_constant(UNPC_PDODGE);
	export_constant(UNPC_CRIT);
	export_constant(UNPC_RACE);
	export_constant(UNPC_ELETYPE);
	export_constant(UNPC_ELELEVEL);
	export_constant(UNPC_AMOTION);
	export_constant(UNPC_ADELAY);
	export_constant(UNPC_DMOTION);

	#undef export_constant

#endif /* _SCRIPT_CONSTANTS_H_ */
