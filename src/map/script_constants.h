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
