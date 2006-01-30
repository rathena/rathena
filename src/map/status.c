// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "pc.h"
#include "map.h"
#include "pet.h"
#include "mob.h"
#include "clif.h"
#include "guild.h"
#include "skill.h"
#include "itemdb.h"
#include "battle.h"
#include "chrif.h"
#include "status.h"

#include "timer.h"
#include "nullpo.h"
#include "script.h"
#include "showmsg.h"

int SkillStatusChangeTable[]={
	-1,-1,-1,-1,-1,-1,
	SC_PROVOKE,
	SC_WATK_ELEMENT,		//Adds part of your final attack as elemental damage. [Skotlex]
	SC_ENDURE,
	-1,
/* 10- */
	SC_SIGHT,			/* ÉTÉCÉg */
	-1,
	SC_SAFETYWALL,		/* ÉZÅ[ÉtÉeÉBÅ[ÉEÉHÅ[Éã */
	-1,-1,-1,
	SC_FREEZE,			/* ÉtÉçÉXÉgÉ_ÉCÉo? */
	SC_STONE,			/* ÉXÉg?ÉìÉJ?ÉX */
	-1,-1,
/* 20- */
	-1,-1,-1,-1,
	SC_RUWACH,			/* ÉãÉAÉt */
	-1,//SC_PNEUMA, Pneuma is no longer a status change. It is a cell type.
	-1,-1,-1,
	SC_INCREASEAGI,		/* ë¨ìx?â¡ */
/* 30- */
	SC_DECREASEAGI,		/* ë¨ìxå∏è≠ */
	-1,
	SC_SIGNUMCRUCIS,	/* ÉVÉOÉiÉÄÉNÉãÉVÉX */
	SC_ANGELUS,			/* ÉGÉìÉWÉFÉâÉX */
	SC_BLESSING,		/* ÉuÉåÉbÉVÉìÉO */
	-1,-1,-1,-1,-1,
/* 40- */
	-1,-1,-1,-1,-1,
	SC_CONCENTRATE,		/* èWíÜóÕå¸è„ */
	-1,-1,-1,-1,
/* 50- */
	-1,
	SC_HIDING,			/* ÉnÉCÉfÉBÉìÉO */
	-1,-1,-1,-1,-1,-1,-1,-1,
/* 60- */
	SC_TWOHANDQUICKEN,	/* 2HQ */
	SC_AUTOCOUNTER,
	-1,-1,-1,-1,
	SC_IMPOSITIO,		/* ÉCÉìÉ|ÉVÉeÉBÉIÉ}ÉkÉX */
	SC_SUFFRAGIUM,		/* ÉTÉtÉâÉMÉEÉÄ */
	SC_ASPERSIO,		/* ÉAÉXÉyÉãÉVÉI */
	SC_BENEDICTIO,		/* êπ?ç~ïü */
/* 70- */
	-1,
	SC_SLOWPOISON,
	-1,
	SC_KYRIE,			/* ÉLÉäÉGÉGÉåÉCÉ\Éì */
	SC_MAGNIFICAT,		/* É}ÉOÉjÉtÉBÉJ?Ég */
	SC_GLORIA,			/* ÉOÉçÉäÉA */
	SC_SILENCE,			/* ÉåÉbÉNÉXÉfÉBÉr?Éi */
	-1,
	SC_AETERNA,			/* ÉåÉbÉNÉXÉG?ÉeÉãÉi */
	-1,
/* 80- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 90- */
	-1,-1,
	SC_QUAGMIRE,		/* ÉNÉ@ÉOÉ}ÉCÉA */
	-1,-1,-1,-1,-1,-1,-1,
/* 100- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 110- */
	-1,
	SC_ADRENALINE,		/* ÉAÉhÉåÉiÉäÉìÉâÉbÉVÉÖ */
	SC_WEAPONPERFECTION,/* ÉEÉFÉ|ÉìÉp?ÉtÉFÉNÉVÉáÉì */
	SC_OVERTHRUST,		/* ÉI?Éo?ÉgÉâÉXÉg */
	SC_MAXIMIZEPOWER,	/* É}ÉLÉVÉ}ÉCÉYÉpÉè? */
	-1,-1,-1,-1,-1,
/* 120- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 130- */
	-1,-1,-1,-1,-1,
	SC_CLOAKING,		/* ÉNÉç?ÉLÉìÉO */
	SC_STAN,			/* É\ÉjÉbÉNÉuÉç? */
	-1,
	SC_ENCPOISON,		/* ÉGÉìÉ`ÉÉÉìÉgÉ|ÉCÉYÉì */
	SC_POISONREACT,		/* É|ÉCÉYÉìÉäÉAÉNÉg */
/* 140- */
	SC_POISON,			/* ÉxÉmÉÄÉ_ÉXÉg */
	SC_SPLASHER,		/* ÉxÉiÉÄÉXÉvÉâÉbÉVÉÉ? */
	-1,
	SC_TRICKDEAD,		/* éÄÇÒÇæÇ”ÇË */
	-1,-1,
	SC_AUTOBERSERK,
	-1,-1,-1,
/* 150- */
	-1,-1,-1,-1,-1,
	SC_LOUD,			/* ÉâÉEÉhÉ{ÉCÉX */
	-1,
	SC_ENERGYCOAT,		/* ÉGÉiÉW?ÉR?Ég */
	-1,-1,
/* 160- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 200 */
	-1,
	SC_KEEPING,
	-1,
	SC_COMA,
	SC_BARRIER,
	-1,
	SC_STAN,
	SC_HALLUCINATION,
	-1,-1,
/* 210- */
	-1,-1,-1,-1,-1,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR,
	SC_STRIPHELM,
	-1,
/* 220- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 230- */
	-1,-1,-1,-1,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	-1,-1,
/* 240- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
	SC_AUTOGUARD,
/* 250- */
	-1,-1,
	SC_REFLECTSHIELD,
	-1,-1,
	SC_DEVOTION,
	SC_PROVIDENCE,
	SC_DEFENDER,
	SC_SPEARSQUICKEN,
	-1,
/* 260- */
	-1,-1,-1,-1,-1,-1,-1,-1,
	SC_STEELBODY,
	SC_BLADESTOP_WAIT,
/* 270- */
	SC_EXPLOSIONSPIRITS,
	SC_EXTREMITYFIST,
	-1,-1,-1,-1,
	SC_MAGICROD,
	-1,-1,-1,
/* 280- */
	SC_FIREWEAPON,
	SC_WATERWEAPON,
	SC_WINDWEAPON,
	SC_EARTHWEAPON,
	-1,
	SC_VOLCANO,
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_LANDPROTECTOR,
	-1,
/* 290- */
	-1,-1,-1,-1,
	SC_ORCISH,
	-1,-1,-1,-1,-1,
/* 300- */
	-1,-1,-1,
	SC_COMA,
	-1,-1,
	SC_LULLABY,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
/* 310- */
	SC_NIBELUNGEN,
	SC_ROKISWEIL,
	SC_INTOABYSS,
	SC_SIEGFRIED,
	-1,-1,-1,-1,-1,
	SC_WHISTLE,
/* 320- */
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	-1,-1,
	SC_UGLYDANCE,
	-1,
	SC_HUMMING,
	SC_DONTFORGETME,
	SC_FORTUNE,
/* 330- */
	SC_SERVICE4U,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 340- */
	-1,-1,
	SC_STOP,
	-1,-1,-1,-1,-1,-1,-1,
/* 350- */
	-1,-1,-1,-1,-1,
	SC_AURABLADE,
	SC_PARRYING,
	SC_CONCENTRATION,
	SC_TENSIONRELAX,
	SC_BERSERK,
/* 360- */
	-1,
	SC_ASSUMPTIO,
	SC_BASILICA,
	-1,-1,-1,
	SC_MAGICPOWER,
	-1,
	SC_SACRIFICE,
	SC_GOSPEL,
/* 370- */
	-1,-1,-1,-1,-1,-1,-1,-1,
	SC_EDP,
	-1,
/* 380- */
	SC_TRUESIGHT,
	-1,-1,
	SC_WINDWALK,
	SC_MELTDOWN,
	-1,-1,
	SC_CARTBOOST,
	-1,
	SC_CHASEWALK,
/* 390- */
	SC_REJECTSWORD,
	-1,-1,-1,-1,
	SC_MOONLIT,
	SC_MARIONETTE,
	-1,
	SC_BLEEDING,
	SC_JOINTBEAT,
/* 400 */
	-1,-1,
	SC_MINDBREAKER,
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	-1,-1,
	SC_BABY,
	-1,
/* 410- */
	-1,
	SC_RUN,
	SC_READYSTORM,
	-1,
	SC_READYDOWN,
	-1,
	SC_READYTURN,
	-1,
	SC_READYCOUNTER,
	-1,
/* 420- */
	SC_DODGE,
	-1,-1,
	SC_TKDORI,
	-1,-1,-1,-1,
	SC_WARM,
	SC_WARM,
/* 430- */
	SC_WARM,
	SC_SUN_COMFORT,
	SC_MOON_COMFORT,
	SC_STAR_COMFORT,
	-1,-1,-1,-1,-1,-1,
/* 440- */
	-1,-1,-1,-1,
	SC_FUSION,
	MAPID_ALCHEMIST, // Storing the target job rather than simply SC_SPIRIT simplifies code later on.
	-1,
	MAPID_MONK,
	MAPID_STAR_GLADIATOR,
	MAPID_SAGE,
/* 450- */
	MAPID_CRUSADER,
	MAPID_SUPER_NOVICE,
	MAPID_KNIGHT,
	MAPID_WIZARD,
	MAPID_PRIEST,
	MAPID_BARDDANCER,
	MAPID_ROGUE,
	MAPID_ASSASSIN,
	MAPID_BLACKSMITH,
	SC_ADRENALINE2,
/* 460- */
	MAPID_HUNTER,
	MAPID_SOUL_LINKER,
	SC_KAIZEL,
	SC_KAAHI,
	SC_KAUPE,
	SC_KAITE,
	-1,-1,-1,-1,
/* 470- */
	SC_SWOO, // [marquis007]
	SC_SKE,
	SC_SKA, // [marquis007]
	-1,-1,
	SC_PRESERVE,
	-1,-1,-1,-1,
/* 480- */
	-1,-1,
	SC_DOUBLECAST,
	-1,
	SC_GRAVITATION,
	-1,
	SC_MAXOVERTHRUST,
	SC_LONGING,
	SC_HERMODE,
	-1,
/* 490- */
	-1,-1,-1,-1,
	SC_SPIRIT,
	SC_ONEHAND,
	-1,-1,-1,-1,
/* 500- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 510- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 520- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 530- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 540- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 550- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 560- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 570- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 580- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 590- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 600- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 610- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 620- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 630- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 640- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 650- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 660- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 670- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 680- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 690- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 700- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 710- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 720- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 730- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 740- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 750- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 760- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 770- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 780- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 790- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 800- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 810- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 820- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 830- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 840- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 850- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 860- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 870- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 880- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 890- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 900- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 910- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 920- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 930- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 940- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 950- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 960- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 970- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 980- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 990- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1000- */
	-1,-1,
	SC_SHRINK,
	-1,-1,
	SC_CLOSECONFINE2,
	SC_SIGHTBLASTER,
	-1,-1,-1,
/* 1010- */
	-1,
	SC_WINKCHARM,
	-1,-1,
	SC_COMA,
	-1,-1,-1,-1,-1,
/* 1020- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1030- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1040- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1050- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1060- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1070- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1080- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 1090- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

int StatusIconChangeTable[SC_MAX];

static int max_weight_base[MAX_PC_CLASS];
static int hp_coefficient[MAX_PC_CLASS];
static int hp_coefficient2[MAX_PC_CLASS];
static int hp_sigma_val[MAX_PC_CLASS][MAX_LEVEL];
static int sp_coefficient[MAX_PC_CLASS];
static int aspd_base[MAX_PC_CLASS][20];
#define MAX_REFINE_BONUS 5
static int refinebonus[MAX_REFINE_BONUS][3];	// ê∏òBÉ{Å[ÉiÉXÉeÅ[ÉuÉã(refine_db.txt)
int percentrefinery[5][MAX_REFINE+1];	// ê∏òBê¨å˜ó¶(refine_db.txt)
static int atkmods[3][20];	// ïêäÌATKÉTÉCÉYèCê≥(size_fix.txt)
static char job_bonus[MAX_PC_CLASS][MAX_LEVEL];

int current_equip_item_index; //Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
//we need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only
//to avoid cards exploits

//Initializes the StatusIconChangeTable variable. May seem somewhat slower than directly defining the array,
//but it is much less prone to errors. [Skotlex]
void initStatusIconChangeTable(void) {
	int i;
	for (i = 0; i < SC_MAX; i++)
		StatusIconChangeTable[i] = SI_BLANK;

	StatusIconChangeTable[SC_PROVOKE] = SI_PROVOKE;
	StatusIconChangeTable[SC_ENDURE] = SI_ENDURE;
	StatusIconChangeTable[SC_TWOHANDQUICKEN] = SI_TWOHANDQUICKEN;
	StatusIconChangeTable[SC_CONCENTRATE] = SI_CONCENTRATE;
	StatusIconChangeTable[SC_HIDING] = SI_HIDING;
	StatusIconChangeTable[SC_CLOAKING] = SI_CLOAKING;
	StatusIconChangeTable[SC_ENCPOISON] = SI_ENCPOISON;
	StatusIconChangeTable[SC_POISONREACT] = SI_POISONREACT;
	StatusIconChangeTable[SC_QUAGMIRE] = SI_QUAGMIRE;
	StatusIconChangeTable[SC_ANGELUS] = SI_ANGELUS;
	StatusIconChangeTable[SC_BLESSING] = SI_BLESSING;
	StatusIconChangeTable[SC_SIGNUMCRUCIS] = SI_SIGNUMCRUCIS;
	StatusIconChangeTable[SC_INCREASEAGI] = SI_INCREASEAGI;
	StatusIconChangeTable[SC_DECREASEAGI] = SI_DECREASEAGI;
	StatusIconChangeTable[SC_SLOWPOISON] = SI_SLOWPOISON;
	StatusIconChangeTable[SC_IMPOSITIO] = SI_IMPOSITIO;
	StatusIconChangeTable[SC_SUFFRAGIUM] = SI_SUFFRAGIUM;
	StatusIconChangeTable[SC_ASPERSIO] = SI_ASPERSIO;
	StatusIconChangeTable[SC_BENEDICTIO] = SI_BENEDICTIO;
	StatusIconChangeTable[SC_KYRIE] = SI_KYRIE;
	StatusIconChangeTable[SC_MAGNIFICAT] = SI_MAGNIFICAT;
	StatusIconChangeTable[SC_GLORIA] = SI_GLORIA;
	StatusIconChangeTable[SC_AETERNA] = SI_AETERNA;
	StatusIconChangeTable[SC_ADRENALINE] = SI_ADRENALINE;
	StatusIconChangeTable[SC_WEAPONPERFECTION]	= SI_WEAPONPERFECTION;
	StatusIconChangeTable[SC_OVERTHRUST] = SI_OVERTHRUST;
	StatusIconChangeTable[SC_MAXIMIZEPOWER] = SI_MAXIMIZEPOWER;
	StatusIconChangeTable[SC_TRICKDEAD] = SI_TRICKDEAD;
	StatusIconChangeTable[SC_LOUD] = SI_LOUD;
	StatusIconChangeTable[SC_ENERGYCOAT] = SI_ENERGYCOAT;
	StatusIconChangeTable[SC_BROKENARMOR] = SI_BROKENARMOR;
	StatusIconChangeTable[SC_BROKENWEAPON] = SI_BROKENWEAPON;
	StatusIconChangeTable[SC_HALLUCINATION] = SI_HALLUCINATION;
	StatusIconChangeTable[SC_WEIGHT50 ] = SI_WEIGHT50;
	StatusIconChangeTable[SC_WEIGHT90] = SI_WEIGHT90;
	StatusIconChangeTable[SC_ASPDPOTION0] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_ASPDPOTION1] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_ASPDPOTION2] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_ASPDPOTION3] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_SPEEDUP0] = SI_SPEEDPOTION;
	StatusIconChangeTable[SC_SPEEDUP1] = SI_SPEEDPOTION;
	StatusIconChangeTable[SC_STRIPWEAPON] = SI_STRIPWEAPON;
	StatusIconChangeTable[SC_STRIPSHIELD] = SI_STRIPSHIELD;
	StatusIconChangeTable[SC_STRIPARMOR] = SI_STRIPARMOR;
	StatusIconChangeTable[SC_STRIPHELM] = SI_STRIPHELM;
	StatusIconChangeTable[SC_CP_WEAPON] = SI_CP_WEAPON;
	StatusIconChangeTable[SC_CP_SHIELD] = SI_CP_SHIELD;
	StatusIconChangeTable[SC_CP_ARMOR] = SI_CP_ARMOR;
	StatusIconChangeTable[SC_CP_HELM] = SI_CP_HELM;
	StatusIconChangeTable[SC_AUTOGUARD] = SI_AUTOGUARD;
	StatusIconChangeTable[SC_REFLECTSHIELD] = SI_REFLECTSHIELD;
	StatusIconChangeTable[SC_PROVIDENCE] = SI_PROVIDENCE;
	StatusIconChangeTable[SC_DEFENDER] = SI_DEFENDER;
	StatusIconChangeTable[SC_AUTOSPELL] = SI_AUTOSPELL;
	StatusIconChangeTable[SC_SPEARSQUICKEN] = SI_SPEARQUICKEN;
	StatusIconChangeTable[SC_EXPLOSIONSPIRITS] = SI_EXPLOSIONSPIRITS;
	StatusIconChangeTable[SC_FURY] = SI_FURY;
	StatusIconChangeTable[SC_FIREWEAPON] = SI_FIREWEAPON;
	StatusIconChangeTable[SC_WATERWEAPON] = SI_WATERWEAPON;
	StatusIconChangeTable[SC_WINDWEAPON] = SI_WINDWEAPON;
	StatusIconChangeTable[SC_EARTHWEAPON] = SI_EARTHWEAPON;
	StatusIconChangeTable[SC_AURABLADE] = SI_AURABLADE;
	StatusIconChangeTable[SC_PARRYING] = SI_PARRYING;
	StatusIconChangeTable[SC_CONCENTRATION] = SI_CONCENTRATION;
	StatusIconChangeTable[SC_TENSIONRELAX] = SI_TENSIONRELAX;
	StatusIconChangeTable[SC_BERSERK] = SI_BERSERK;
	StatusIconChangeTable[SC_ASSUMPTIO] = SI_ASSUMPTIO;
	StatusIconChangeTable[SC_GUILDAURA] = SI_GUILDAURA;
	StatusIconChangeTable[SC_MAGICPOWER] = SI_MAGICPOWER;
	StatusIconChangeTable[SC_EDP] = SI_EDP;
	StatusIconChangeTable[SC_TRUESIGHT] = SI_TRUESIGHT;
	StatusIconChangeTable[SC_WINDWALK] = SI_WINDWALK;
	StatusIconChangeTable[SC_MELTDOWN] = SI_MELTDOWN;
	StatusIconChangeTable[SC_CARTBOOST] = SI_CARTBOOST;
	StatusIconChangeTable[SC_CHASEWALK] = SI_CHASEWALK;
	StatusIconChangeTable[SC_REJECTSWORD] = SI_REJECTSWORD;
	StatusIconChangeTable[SC_MARIONETTE] = SI_MARIONETTE;
	StatusIconChangeTable[SC_MARIONETTE2] = SI_MARIONETTE2;
	StatusIconChangeTable[SC_BLEEDING] = SI_BLEEDING;
	StatusIconChangeTable[SC_MOONLIT] = SI_MOONLIT;
	StatusIconChangeTable[SC_DEVOTION] = SI_DEVOTION;
	StatusIconChangeTable[SC_STEELBODY] = SI_STEELBODY;
	StatusIconChangeTable[SC_SPURT] = SI_SPURT;
	StatusIconChangeTable[SC_SPIRIT] = SI_SPIRIT;
	StatusIconChangeTable[SC_READYSTORM] = SI_READYSTORM;
	StatusIconChangeTable[SC_READYDOWN] = SI_READYDOWN;
	StatusIconChangeTable[SC_READYTURN] = SI_READYTURN;
	StatusIconChangeTable[SC_READYCOUNTER] = SI_READYCOUNTER;
	StatusIconChangeTable[SC_DODGE] = SI_DODGE;
	StatusIconChangeTable[SC_SHADOWWEAPON] = SI_SHADOWWEAPON;
	StatusIconChangeTable[SC_WARM] = SI_WARM;
	StatusIconChangeTable[SC_SUN_COMFORT] = SI_SUN_COMFORT;
	StatusIconChangeTable[SC_MOON_COMFORT] = SI_MOON_COMFORT;
	StatusIconChangeTable[SC_STAR_COMFORT] = SI_STAR_COMFORT;
	StatusIconChangeTable[SC_ADRENALINE2] = SI_ADRENALINE2;
	StatusIconChangeTable[SC_GHOSTWEAPON] = SI_GHOSTWEAPON;
	StatusIconChangeTable[SC_KAITE] = SI_KAITE;
	StatusIconChangeTable[SC_KAIZEL] = SI_KAIZEL;
	StatusIconChangeTable[SC_KAAHI] = SI_KAAHI;
	StatusIconChangeTable[SC_KAUPE] = SI_KAUPE;
	StatusIconChangeTable[SC_ONEHAND] = SI_ONEHAND;
	StatusIconChangeTable[SC_PRESERVE] = SI_PRESERVE;
	StatusIconChangeTable[SC_BATTLEORDERS] = SI_BATTLEORDERS;
	StatusIconChangeTable[SC_DOUBLECAST] = SI_DOUBLECAST;
	StatusIconChangeTable[SC_MAXOVERTHRUST] = SI_MAXOVERTHRUST;
	StatusIconChangeTable[SC_SHRINK] = SI_SHRINK;
	StatusIconChangeTable[SC_SIGHTBLASTER] = SI_SIGHTBLASTER;
	StatusIconChangeTable[SC_WINKCHARM] = SI_WINKCHARM;
	StatusIconChangeTable[SC_CLOSECONFINE] = SI_CLOSECONFINE;
	StatusIconChangeTable[SC_CLOSECONFINE2] = SI_CLOSECONFINE2;
}
/*==========================================
 * ê∏òBÉ{Å[ÉiÉX
 *------------------------------------------
 */
int status_getrefinebonus(int lv,int type)
{
	if (lv >= 0 && lv < 5 && type >= 0 && type < 3)
		return refinebonus[lv][type];
	return 0;
}

/*==========================================
 * Checks whether the src can use the skill on the target,
 * taking into account status/option of both source/target. [Skotlex]
 * flag: 1 to indicate this call is done after the casting (target already selected)
 * src MAY be null to indicate we shouldn't check it, this is a ground-based skill attack.
 * target MAY Be null, in which case the checks are only to see 
 * whether the source can cast or not the skill on the ground.
 *------------------------------------------
 */
int status_check_skilluse(struct block_list *src, struct block_list *target, int skill_num, int flag)
{
	int mode, race, hide_flag;
	struct status_change *sc_data=NULL, *tsc_data;
	short *option=NULL, *opt1=NULL;

	if (src && status_isdead(src))
		return 0;
	if (target && status_isdead(target) && skill_num != ALL_RESURRECTION)
		return 0;
	
	if (skill_num == PA_PRESSURE && flag)
		return 1; //Once Gloria Domini has been casted, there's nothing you can do to stop it. [Skotlex]
	
	mode = src?status_get_mode(src):MD_CANATTACK;
	
	if (!skill_num && !(mode&MD_CANATTACK))
		return 0; //This mode is only needed for melee attacking.
	
	if (((src && map_getcell(src->m,src->x,src->y,CELL_CHKBASILICA)) ||
		(target && target != src && map_getcell(target->m,target->x,target->y,CELL_CHKBASILICA)))
		&& !(mode&MD_BOSS))
	{	//Basilica Check
		if (!skill_num) return 0;
		race = skill_get_inf(skill_num);
		if (race&INF_ATTACK_SKILL)
			return 0;
		if (race&INF_GROUND_SKILL && skill_get_unit_target(skill_num)&BCT_ENEMY)
			return 0;
	}	
	if (src) {
		option = status_get_option(src);
		opt1 = status_get_opt1(src);
		sc_data = status_get_sc_data(src);
	}
	
	if (opt1 && (*opt1) >0)
		return 0;
	
	if(sc_data)
	{
		if (
			(sc_data[SC_TRICKDEAD].timer != -1 && skill_num != NV_TRICKDEAD)
			|| (sc_data[SC_AUTOCOUNTER].timer != -1 && skill_num != KN_AUTOCOUNTER)
			|| (sc_data[SC_GOSPEL].timer != -1 && sc_data[SC_GOSPEL].val4 == BCT_SELF && skill_num != PA_GOSPEL)
			|| sc_data[SC_GRAVITATION].timer != -1
		)
			return 0;

		if (sc_data[SC_BLADESTOP].timer != -1) {
			switch (sc_data[SC_BLADESTOP].val1)
			{
				case 1: return 0;
				case 2: if (skill_num != MO_FINGEROFFENSIVE) return 0; break;
				case 3: if (skill_num != MO_FINGEROFFENSIVE && skill_num != MO_INVESTIGATE) return 0; break;
				case 4: if (skill_num != MO_FINGEROFFENSIVE && skill_num != MO_INVESTIGATE && skill_num != MO_CHAINCOMBO) return 0; break;
				case 5: if (skill_num != MO_FINGEROFFENSIVE && skill_num != MO_INVESTIGATE && skill_num != MO_CHAINCOMBO && skill_num!=MO_EXTREMITYFIST) return 0; break;
				default: return 0;
			}
		}
		if (skill_num)
		{	//Skills blocked through status changes...
			if ((sc_data[SC_VOLCANO].timer != -1 && skill_num == WZ_ICEWALL) ||
				(sc_data[SC_ROKISWEIL].timer != -1 && skill_num != BD_ADAPTATION) ||
				(sc_data[SC_MARIONETTE].timer != -1 && skill_num != CG_MARIONETTE) ||
				(sc_data[SC_MARIONETTE2].timer != -1 && skill_num == CG_MARIONETTE) ||
				(sc_data[SC_HERMODE].timer != -1 && skill_get_inf(skill_num) & INF_SUPPORT_SKILL) ||
				sc_data[SC_SILENCE].timer != -1 || sc_data[SC_STEELBODY].timer != -1 ||
				sc_data[SC_BERSERK].timer != -1 || sc_data[SC_SKA].timer != -1
			)
				return 0;

			if (sc_data[SC_DANCING].timer != -1)
			{
				if (skill_num != BD_ADAPTATION && skill_num != CG_LONGINGFREEDOM
					&& skill_num != BA_MUSICALSTRIKE && skill_num != DC_THROWARROW)
					return 0;
				if (sc_data[SC_DANCING].val1 == CG_HERMODE && skill_num == BD_ADAPTATION)
					return 0;	//Can't amp out of Wand of Hermode :/ [Skotlex]
			}
		}
	}

	if (option)
	{
		if ((*option)&OPTION_HIDE && skill_num != TF_HIDING && skill_num != AS_GRIMTOOTH
			&& skill_num != RG_BACKSTAP && skill_num != RG_RAID)
			return 0;
		if ((*option)&OPTION_CLOAK && skill_num == TF_HIDING)
			return 0;
		if ((*option)&OPTION_CHASEWALK && skill_num != ST_CHASEWALK)
			return 0;
	}
	if (target == NULL || target == src) //No further checking needed.
		return 1;

	tsc_data = status_get_sc_data(target);
	if(tsc_data)
	{	
		if (!(mode & MD_BOSS) && tsc_data[SC_TRICKDEAD].timer != -1)
			return 0;
		if(skill_num == WZ_STORMGUST && tsc_data[SC_FREEZE].timer != -1)
			return 0;
		if(skill_num == PR_LEXAETERNA && (tsc_data[SC_FREEZE].timer != -1 || (tsc_data[SC_STONE].timer != -1 && tsc_data[SC_STONE].val2 == 0)))
			return 0;
	}

	if (src) {
		race = status_get_race(src); 
	} else { //Ground skill, only earth-elemental skills have detecting-hitting capabilities.
		race = 0;
		if(skill_get_pl(skill_num) == 2)
			mode|= MD_DETECTOR;
	}
	option = status_get_option(target);
	hide_flag = flag?OPTION_HIDE:(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK); //If targetting, cloak+hide protect you, otherwise only hiding does.
		
	switch (target->type)
	{
	case BL_PC:
		{
			struct map_session_data *sd = (struct map_session_data*) target;
			if (pc_isinvisible(sd))
				return 0;
			if (((*option)&hide_flag || sd->state.gangsterparadise)
				&& (sd->state.perfect_hiding || !(race == 4 || race == 6 || mode&MD_DETECTOR))
				&& !(mode&MD_BOSS))
				return 0;
		}
		break;
	case BL_PET:
		return 0;
	case BL_ITEM:	//Allow targetting of items to pick'em up (or in the case of mobs, to loot them).
		//TODO: Would be nice if this could be used to judge whether the player can or not pick up the item it targets. [Skotlex]
		if (mode&MD_LOOTER)
			return 1;
		else
			return 0;
	default:
		//Check for chase-walk/hiding/cloaking opponents.
		if (option && !(mode&MD_BOSS))
		{
			if ((*option)&hide_flag && !(race == 4 || race == 6 || mode&MD_DETECTOR))
				return 0;
		}
	}
	return 1;
}

//Skotlex: Calculates the stats of the given pet.
int status_calc_pet(struct map_session_data *sd, int first)
{
	struct pet_data *pd;
	
	nullpo_retr(0, sd);
	if (sd->status.pet_id == 0 || sd->pd == NULL)
		return 1;

	pd = sd->pd;
	
	if (battle_config.pet_lv_rate && pd->status)
	{
		sd->pet.level = sd->status.base_level*battle_config.pet_lv_rate/100;
		if (sd->pet.level < 0)
			sd->pet.level = 1;
		if (pd->status->level != sd->pet.level || first)
		{
			if (!first) //Lv Up animation
				clif_misceffect(&pd->bl, 0);
			pd->status->level = sd->pet.level;
			pd->status->atk1 = (pd->db->atk1*pd->status->level)/pd->db->lv;
			pd->status->atk2 = (pd->db->atk2*pd->status->level)/pd->db->lv;
			pd->status->str = (pd->db->str*pd->status->level)/pd->db->lv;
			pd->status->agi = (pd->db->agi*pd->status->level)/pd->db->lv;
			pd->status->vit = (pd->db->vit*pd->status->level)/pd->db->lv;
			pd->status->int_ = (pd->db->int_*pd->status->level)/pd->db->lv;
			pd->status->dex = (pd->db->dex*pd->status->level)/pd->db->lv;
			pd->status->luk = (pd->db->luk*pd->status->level)/pd->db->lv;
		
			if (pd->status->atk1 > battle_config.pet_max_atk1) pd->status->atk1 = battle_config.pet_max_atk1;
			if (pd->status->atk2 > battle_config.pet_max_atk2) pd->status->atk2 = battle_config.pet_max_atk2;

			if (pd->status->str > battle_config.pet_max_stats) pd->status->str = battle_config.pet_max_stats;
			else if (pd->status->str < 1) pd->status->str = 1;
			if (pd->status->agi > battle_config.pet_max_stats) pd->status->agi = battle_config.pet_max_stats;
			else if (pd->status->agi < 1) pd->status->agi = 1;
			if (pd->status->vit > battle_config.pet_max_stats) pd->status->vit = battle_config.pet_max_stats;
			else if (pd->status->vit < 1) pd->status->vit = 1;
			if (pd->status->int_ > battle_config.pet_max_stats) pd->status->int_ = battle_config.pet_max_stats;
			else if (pd->status->int_ < 1) pd->status->int_ = 1;
			if (pd->status->dex > battle_config.pet_max_stats) pd->status->dex = battle_config.pet_max_stats;
			else if (pd->status->dex < 1) pd->status->dex = 1;
			if (pd->status->luk > battle_config.pet_max_stats) pd->status->luk = battle_config.pet_max_stats;
			else if (pd->status->luk < 1) pd->status->luk = 1;

			if (!first)	//Not done the first time because the pet is not visible yet
				clif_send_petstatus(sd);
		}
	}
	//Support rate modifier (1000 = 100%)
	pd->rate_fix = 1000*(sd->pet.intimate - battle_config.pet_support_min_friendly)/(1000- battle_config.pet_support_min_friendly) +500;
	if(battle_config.pet_support_rate != 100)
		pd->rate_fix = pd->rate_fix*battle_config.pet_support_rate/100;
	return 0;
}	

/*==========================================
 * ÉpÉâÉÅÅ[É^åvéZ
 * first==0ÇÃéûÅAåvéZëŒè€ÇÃÉpÉâÉÅÅ[É^Ç™åƒÇ—èoÇµëOÇ©ÇÁ
 * ïœ âªÇµÇΩèÍçáé©ìÆÇ≈sendÇ∑ÇÈÇ™ÅA
 * î\ìÆìIÇ…ïœâªÇ≥ÇπÇΩÉpÉâÉÅÅ[É^ÇÕé©ëOÇ≈sendÇ∑ÇÈÇÊÇ§Ç…
 *------------------------------------------
 */

int status_calc_pc(struct map_session_data* sd,int first)
{
	static int calculating = 0; //Check for recursive call preemption. [Skotlex]
	int b_speed,b_max_hp,b_max_sp,b_hp,b_sp,b_weight,b_max_weight,b_paramb[6],b_parame[6],b_hit,b_flee;
	int b_aspd,b_watk,b_def,b_watk2,b_def2,b_flee2,b_critical,b_attackrange,b_matk1,b_matk2,b_mdef,b_mdef2,b_class;
	int b_base_atk;
	struct skill b_skill[MAX_SKILL];
	int i,bl,index;
	int skill,wele,wele_,def_ele,refinedef=0;
	int pele=0,pdef_ele=0;
	int str,dstr,dex;

	nullpo_retr(0, sd);
	if (++calculating > 10) //Too many recursive calls to status_calc_pc!
		return -1;
	
	b_speed = sd->speed;
	b_max_hp = sd->status.max_hp;
	b_max_sp = sd->status.max_sp;
	b_hp = sd->status.hp;
	b_sp = sd->status.sp;
	b_weight = sd->weight;
	b_max_weight = sd->max_weight;
	memcpy(b_paramb,&sd->paramb,sizeof(b_paramb));
	memcpy(b_parame,&sd->paramc,sizeof(b_parame));
	memcpy(b_skill,&sd->status.skill,sizeof(b_skill));
	b_hit = sd->hit;
	b_flee = sd->flee;
	b_aspd = sd->aspd;
	b_watk = sd->right_weapon.watk;
	b_def = sd->def;
	b_watk2 = sd->right_weapon.watk2;
	b_def2 = sd->def2;
	b_flee2 = sd->flee2;
	b_critical = sd->critical;
	b_attackrange = sd->attackrange;
	b_matk1 = sd->matk1;
	b_matk2 = sd->matk2;
	b_mdef = sd->mdef;
	b_mdef2 = sd->mdef2;
	b_class = sd->view_class;
	sd->view_class = sd->status.class_;
	b_base_atk = sd->base_atk;

	pc_calc_skilltree(sd);	// ÉXÉLÉãÉcÉä?ÇÃåvéZ
	
	sd->max_weight = max_weight_base[sd->status.class_]+sd->status.str*300;

	if(first&1) {
		sd->weight=0;
		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid==0 || sd->inventory_data[i] == NULL)
				continue;
			sd->weight += sd->inventory_data[i]->weight*sd->status.inventory[i].amount;
		}
		sd->cart_max_weight=battle_config.max_cart_weight;
		sd->cart_weight=0;
		sd->cart_max_num=MAX_CART;
		sd->cart_num=0;
		for(i=0;i<MAX_CART;i++){
			if(sd->status.cart[i].nameid==0)
				continue;
			sd->cart_weight+=itemdb_weight(sd->status.cart[i].nameid)*sd->status.cart[i].amount;
			sd->cart_num++;
		}
	}

	// these are not zeroed. [zzo]

	sd->speed = DEFAULT_WALK_SPEED;
	sd->hprate=battle_config.hp_rate;
	sd->sprate=battle_config.sp_rate;
	sd->castrate=100;
	sd->delayrate=100;
	sd->dsprate=100;
	sd->aspd_rate = 100;
	sd->speed_rate = 100;
	sd->hprecov_rate = 100;
	sd->sprecov_rate = 100;
	sd->atk_rate = sd->matk_rate = 100;
	sd->critical_rate = sd->hit_rate = sd->flee_rate = sd->flee2_rate = 100;
	sd->def_rate = sd->def2_rate = sd->mdef_rate = sd->mdef2_rate = 100;
	sd->speed_add_rate = sd->aspd_add_rate = 100;

	// zeroed arays, order follows the order in map.h.
	// add new arrays to the end of zeroed area in map.h (see comments) and size here. [zzo]
	memset (sd->paramb, 0, sizeof(sd->paramb)
		+ sizeof(sd->parame)
		+ sizeof(sd->subele)
		+ sizeof(sd->subrace)
		+ sizeof(sd->subrace2)
		+ sizeof(sd->subsize)
		+ sizeof(sd->addeff)
		+ sizeof(sd->addeff2)
		+ sizeof(sd->reseff)
		+ sizeof(sd->weapon_coma_ele)
		+ sizeof(sd->weapon_coma_race)
		+ sizeof(sd->weapon_atk)
		+ sizeof(sd->weapon_atk_rate)
		+ sizeof(sd->arrow_addele) 
		+ sizeof(sd->arrow_addrace)
		+ sizeof(sd->arrow_addsize)
		+ sizeof(sd->arrow_addeff)
		+ sizeof(sd->arrow_addeff2)
		+ sizeof(sd->magic_addele)
		+ sizeof(sd->magic_addrace)
		+ sizeof(sd->magic_addsize)
		+ sizeof(sd->critaddrace)
		+ sizeof(sd->expaddrace)
		+ sizeof(sd->itemhealrate)
		+ sizeof(sd->addeff3)
		+ sizeof(sd->addeff3_type)
		+ sizeof(sd->sp_gain_race)
		+ sizeof(sd->unequip_losehp)
		+ sizeof(sd->unequip_losesp)
		);


	memset (&sd->right_weapon.watk, 0, sizeof(sd->right_weapon) - sizeof(sd->right_weapon.atkmods));
	memset (&sd->left_weapon.watk, 0, sizeof(sd->left_weapon) - sizeof(sd->left_weapon.atkmods));

	memset(&sd->special_state,0,sizeof(sd->special_state));

	sd->status.max_hp = 0;
	sd->status.max_sp = 0;

	//zero up structures...
	memset(&sd->autospell,0,sizeof(sd->autospell)
		+ sizeof(sd->autospell2)
		+ sizeof(sd->skillatk)
		+ sizeof(sd->skillblown)
		+ sizeof(sd->add_def)
		+ sizeof(sd->add_mdef)
		+ sizeof(sd->add_dmg)
		+ sizeof(sd->add_mdmg)
		+ sizeof(sd->add_drop)
	);
	
	// vars zeroing. ints, shorts, chars. in that order.
	memset (&sd->hit, 0, sizeof(sd->hit)
		+ sizeof(sd->flee)
		+ sizeof(sd->flee2)
		+ sizeof(sd->critical)
		+ sizeof(sd->aspd)
		+ sizeof(sd->def)
		+ sizeof(sd->mdef)
		+ sizeof(sd->def2)
		+ sizeof(sd->mdef2)
		+ sizeof(sd->def_ele)
		+ sizeof(sd->matk1)
		+ sizeof(sd->matk2)
		+ sizeof(sd->base_atk)
		+ sizeof(sd->arrow_atk)
		+ sizeof(sd->arrow_ele)
		+ sizeof(sd->arrow_cri)
		+ sizeof(sd->arrow_hit)
		+ sizeof(sd->arrow_range)
		+ sizeof(sd->nhealhp)
		+ sizeof(sd->nhealsp)
		+ sizeof(sd->nshealhp)
		+ sizeof(sd->nshealsp)
		+ sizeof(sd->nsshealhp)
		+ sizeof(sd->nsshealsp)
		+ sizeof(sd->critical_def)
		+ sizeof(sd->double_rate)
		+ sizeof(sd->long_attack_atk_rate)
		+ sizeof(sd->near_attack_def_rate)
		+ sizeof(sd->long_attack_def_rate)
		+ sizeof(sd->magic_def_rate)
		+ sizeof(sd->misc_def_rate)
		+ sizeof(sd->ignore_mdef_ele)
		+ sizeof(sd->ignore_mdef_race)
		+ sizeof(sd->perfect_hit)
		+ sizeof(sd->perfect_hit_add)
		+ sizeof(sd->get_zeny_rate)
		+ sizeof(sd->get_zeny_num)
		+ sizeof(sd->double_add_rate)
		+ sizeof(sd->short_weapon_damage_return)
		+ sizeof(sd->long_weapon_damage_return)
		+ sizeof(sd->magic_damage_return)
		+ sizeof(sd->random_attack_increase_add)
		+ sizeof(sd->random_attack_increase_per)
		+ sizeof(sd->break_weapon_rate)
		+ sizeof(sd->break_armor_rate)
		+ sizeof(sd->crit_atk_rate)
		+ sizeof(sd->hp_loss_rate)
		+ sizeof(sd->sp_loss_rate)
		+ sizeof(sd->classchange)
		+ sizeof(sd->setitem_hash)
		+ sizeof(sd->setitem_hash2)
		// shorts
		+ sizeof(sd->attackrange)
		+ sizeof(sd->attackrange_)
		+ sizeof(sd->splash_range)
		+ sizeof(sd->splash_add_range)
		+ sizeof(sd->add_steal_rate)
		+ sizeof(sd->hp_loss_value)
		+ sizeof(sd->sp_loss_value)
		+ sizeof(sd->hp_loss_type)
		+ sizeof(sd->sp_drain_type)
		+ sizeof(sd->hp_gain_value)
		+ sizeof(sd->sp_gain_value)
		+ sizeof(sd->add_drop_count)
		+ sizeof(sd->unbreakable)
		+ sizeof(sd->unbreakable_equip)
		+ sizeof(sd->unstripable_equip)
		+ sizeof(sd->no_regen)
		+ sizeof(sd->add_def_count)
		+ sizeof(sd->add_mdef_count)
		+ sizeof(sd->add_dmg_count)
		+ sizeof(sd->add_mdmg_count)
		);

	if(!sd->state.disguised && sd->disguise) {
		pc_stop_walking(sd,0);
		clif_clearchar(&sd->bl, 0);
		sd->disguise=0;
		clif_changeoption(&sd->bl);
		clif_spawnpc(sd);
	}

	for(i=0;i<10;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == 9 && sd->equip_index[8] == index)
			continue;
		if(i == 5 && sd->equip_index[4] == index)
			continue;
		if(i == 6 && (sd->equip_index[5] == index || sd->equip_index[4] == index))
			continue;

		if(sd->inventory_data[index]) {
			if(sd->inventory_data[index]->type == 4) { // Weapon cards
				if(sd->status.inventory[index].card[0]!=0x00ff && sd->status.inventory[index].card[0]!=0x00fe && sd->status.inventory[index].card[0]!=(short)0xff00) {
					int j;
					for(j=0;j<sd->inventory_data[index]->slot;j++){	// ÉJ?Éh
						int c=sd->status.inventory[index].card[j];
						if(c>0){
							if(i == 8 && sd->status.inventory[index].equip == 0x20)
								sd->state.lr_flag = 1;
							run_script(itemdb_equipscript(c),0,sd->bl.id,0);
							sd->state.lr_flag = 0;
							if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
								return 1;
						}
					}
				}
			}
			else if(sd->inventory_data[index]->type==5){ // Non-weapon equipment cards
				if(sd->status.inventory[index].card[0]!=0x00ff && sd->status.inventory[index].card[0]!=0x00fe && sd->status.inventory[index].card[0]!=(short)0xff00) {
					int j;
					for(j=0;j<sd->inventory_data[index]->slot;j++){	// ÉJ?Éh
						int c=sd->status.inventory[index].card[j];
						if(c>0) {
							run_script(itemdb_equipscript(c),0,sd->bl.id,0);
							if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
								return 1;
						}
					}
				}
			}
		}
	}
	wele = sd->right_weapon.atk_ele;
	wele_ = sd->left_weapon.atk_ele;
	def_ele = sd->def_ele;

	if(sd->status.pet_id > 0) { // Pet
		struct pet_data *pd=sd->pd;
		if((pd && battle_config.pet_status_support) && (!battle_config.pet_equip_required || pd->equip > 0)) {
			if(sd->pet.intimate > 0 && pd->state.skillbonus == 1 && pd->bonus) { //Skotlex: Readjusted for pets
				pc_bonus(sd,pd->bonus->type, pd->bonus->val);
			}
			pele = sd->right_weapon.atk_ele;
			pdef_ele = sd->def_ele;
			sd->right_weapon.atk_ele = sd->def_ele = 0;
		}
	}
	memcpy(sd->paramcard,sd->parame,sizeof(sd->paramcard));

	// ?îıïiÇ…ÇÊÇÈÉXÉe?É^ÉX?âªÇÕÇ±Ç±Ç≈?çs
	for(i=0;i<10;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == 9 && sd->equip_index[8] == index)
			continue;
		if(i == 5 && sd->equip_index[4] == index)
			continue;
		if(i == 6 && (sd->equip_index[5] == index || sd->equip_index[4] == index))
			continue;
		if(sd->inventory_data[index]) {
			sd->def += sd->inventory_data[index]->def;
			if(sd->inventory_data[index]->type == 4) {
				int r,wlv = sd->inventory_data[index]->wlv;
				if (wlv >= MAX_REFINE_BONUS) 
					wlv = MAX_REFINE_BONUS - 1;
				if(i == 8 && sd->status.inventory[index].equip == 0x20) { // Left-hand weapon
					sd->left_weapon.watk += sd->inventory_data[index]->atk;
					sd->left_weapon.watk2 = (r=sd->status.inventory[index].refine)*	// ê∏?çU?óÕ
						refinebonus[wlv][0];
					if( (r-=refinebonus[wlv][2])>0 )	// âﬂ?ê∏?É{?ÉiÉX
						sd->left_weapon.overrefine = r*refinebonus[wlv][1];

					if(sd->status.inventory[index].card[0]==0x00ff){	// Forged weapon
						sd->left_weapon.star = (sd->status.inventory[index].card[1]>>8);	// êØÇÃÇ©ÇØÇÁ
						if(sd->left_weapon.star >= 15) sd->left_weapon.star = 40; // 3 Star Crumbs now give +40 dmg
						if (pc_istop10fame( MakeDWord(sd->status.inventory[index].card[2],sd->status.inventory[index].card[3]) ,MAPID_BLACKSMITH))
							sd->left_weapon.star += 10;
						wele_= (sd->status.inventory[index].card[1]&0x0f);	// ? ê´
					}
					sd->attackrange_ += sd->inventory_data[index]->range;
					sd->state.lr_flag = 1;
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
					if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
						return 1;
				}
				else {	// Right-hand weapon
					sd->right_weapon.watk += sd->inventory_data[index]->atk;
					sd->right_weapon.watk2 += (r=sd->status.inventory[index].refine)*	// ê∏?çU?óÕ
						refinebonus[wlv][0];
					if( (r-=refinebonus[wlv][2])>0 )	// âﬂ?ê∏?É{?ÉiÉX
						sd->right_weapon.overrefine += r*refinebonus[wlv][1];

					if(sd->status.inventory[index].card[0]==0x00ff){	// Forged weapon
						sd->right_weapon.star += (sd->status.inventory[index].card[1]>>8);	// êØÇÃÇ©ÇØÇÁ
						if(sd->right_weapon.star >= 15) sd->right_weapon.star = 40; // 3 Star Crumbs now give +40 dmg
						if (pc_istop10fame( MakeDWord(sd->status.inventory[index].card[2],sd->status.inventory[index].card[3]) ,MAPID_BLACKSMITH))
							sd->right_weapon.star += 10;
						wele = (sd->status.inventory[index].card[1]&0x0f);	// ? ê´
					}
					sd->attackrange += sd->inventory_data[index]->range;
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
					if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
						return 1;
				}
			}
			else if(sd->inventory_data[index]->type == 5) {
				sd->right_weapon.watk += sd->inventory_data[index]->atk;
				refinedef += sd->status.inventory[index].refine*refinebonus[0][0];
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
					return 1;
			}
		}
	}

	if(sd->equip_index[10] >= 0){ // ñÓ
		index = sd->equip_index[10];
		if(sd->inventory_data[index]){		// Arrows
			sd->state.lr_flag = 2;
			run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			sd->arrow_atk += sd->inventory_data[index]->atk;
		}
	}
	sd->def += (refinedef+50)/100;

	if(sd->attackrange < 1) sd->attackrange = 1;
	if(sd->attackrange_ < 1) sd->attackrange_ = 1;
	if(sd->attackrange < sd->attackrange_)
		sd->attackrange = sd->attackrange_;
	if(sd->status.weapon == 11)
		sd->attackrange += sd->arrow_range;
	if(wele > 0)
		sd->right_weapon.atk_ele = wele;
	if(wele_ > 0)
		sd->left_weapon.atk_ele = wele_;
	if(def_ele > 0)
		sd->def_ele = def_ele;
	if(battle_config.pet_status_support) {
		if(pele > 0 && !sd->right_weapon.atk_ele)
			sd->right_weapon.atk_ele = pele;
		if(pdef_ele > 0 && !sd->def_ele)
			sd->def_ele = pdef_ele;
	}
	sd->double_rate += sd->double_add_rate;
	sd->perfect_hit += sd->perfect_hit_add;
	sd->splash_range += sd->splash_add_range;
	if(sd->aspd_add_rate != 100)	
		sd->aspd_rate += sd->aspd_add_rate-100;
	if(sd->speed_add_rate != 100)	
		sd->speed_rate += sd->speed_add_rate-100;

	// Damage modifiers from weapon type
	sd->right_weapon.atkmods[0] = atkmods[0][sd->weapontype1];
	sd->right_weapon.atkmods[1] = atkmods[1][sd->weapontype1];
	sd->right_weapon.atkmods[2] = atkmods[2][sd->weapontype1];
	sd->left_weapon.atkmods[0] = atkmods[0][sd->weapontype2];
	sd->left_weapon.atkmods[1] = atkmods[1][sd->weapontype2];
	sd->left_weapon.atkmods[2] = atkmods[2][sd->weapontype2];

// ----- STATS CALCULATION -----

	// Job bonuses
	for(i=0;i<(int)sd->status.job_level && i<MAX_LEVEL;i++){
		if(job_bonus[sd->status.class_][i])
			sd->paramb[job_bonus[sd->status.class_][i]-1]++;
	}

	// If a Super Novice has never died and is at least joblv 70, he gets all stats +10
	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->die_counter == 0 && sd->status.job_level >= 70){
		sd->paramb[0] += 10;
		sd->paramb[1] += 10;
		sd->paramb[2] += 10;
		sd->paramb[3] += 10;
		sd->paramb[4] += 10;
		sd->paramb[5] += 10;
	}

	// Absolute modifiers from passive skills
	if(pc_checkskill(sd,BS_HILTBINDING)>0)
		sd->paramb[0] ++;
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0)
		sd->paramb[3] += (skill+1)/2; // +1 INT / 2 lv
	if((skill=pc_checkskill(sd,AC_OWL))>0)
		sd->paramb[4] += skill;

	// Improve Concentration adds a percentage of base stat + job bonus + equipment bonus, but not from card bonus nor status change bonus
	if(sd->sc_count && sd->sc_data[SC_CONCENTRATE].timer!=-1 && sd->sc_data[SC_QUAGMIRE].timer == -1){
		sd->paramb[1] += (sd->status.agi+sd->paramb[1]+sd->parame[1]-sd->paramcard[1])*(2+sd->sc_data[SC_CONCENTRATE].val1)/100;
		sd->paramb[4] += (sd->status.dex+sd->paramb[4]+sd->parame[4]-sd->paramcard[4])*(2+sd->sc_data[SC_CONCENTRATE].val1)/100;
	}

	// Absolute modifiers from status changes (only for PC)
	if(sd->sc_count){
		if (sd->sc_data[SC_BATTLEORDERS].timer != -1) {
			sd->paramb[0] += 5;
			sd->paramb[3] += 5;
			sd->paramb[4] += 5;
		}
		if (sd->sc_data[SC_GUILDAURA].timer != -1) {
			int guildflag = sd->sc_data[SC_GUILDAURA].val4;
			for (i = 12; i >= 0; i -= 4) {
				skill = guildflag >> i;
				switch (i) {
					case 12: sd->paramb[0] += skill; break;
					case 8: sd->paramb[2] += skill; break;
					case 4: sd->paramb[1] += skill; break;
					case 0: sd->paramb[4] += skill; break;
				}
				guildflag ^= skill << i;
			}
		}
	}

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->paramb[0] = status_calc_str(&sd->bl,sd->paramb[0]);
	sd->paramb[1] = status_calc_agi(&sd->bl,sd->paramb[1]);
	sd->paramb[2] = status_calc_vit(&sd->bl,sd->paramb[2]);
	sd->paramb[3] = status_calc_int(&sd->bl,sd->paramb[3]);
	sd->paramb[4] = status_calc_dex(&sd->bl,sd->paramb[4]);
	sd->paramb[5] = status_calc_luk(&sd->bl,sd->paramb[5]);
	
	// Relative modifiers from status changes (only for PC)
	if(sd->sc_count){
		if(sd->sc_data[SC_MARIONETTE].timer!=-1){
			sd->paramb[0]-= sd->status.str/2;
			sd->paramb[1]-= sd->status.agi/2;
			sd->paramb[2]-= sd->status.vit/2;
			sd->paramb[3]-= sd->status.int_/2;
			sd->paramb[4]-= sd->status.dex/2;
			sd->paramb[5]-= sd->status.luk/2;
		}
		else if(sd->sc_data[SC_MARIONETTE2].timer!=-1){
			struct map_session_data *psd = map_id2sd(sd->sc_data[SC_MARIONETTE2].val3);
			if (psd) {	// if partner is found
				sd->paramb[0] += sd->status.str+psd->status.str/2 > 99 ? 99-sd->status.str : psd->status.str/2;
				sd->paramb[1] += sd->status.agi+psd->status.agi/2 > 99 ? 99-sd->status.agi : psd->status.agi/2;
				sd->paramb[2] += sd->status.vit+psd->status.vit/2 > 99 ? 99-sd->status.vit : psd->status.vit/2;
				sd->paramb[3] += sd->status.int_+psd->status.int_/2 > 99 ? 99-sd->status.int_ : psd->status.int_/2;
				sd->paramb[4] += sd->status.dex+psd->status.dex/2 > 99 ? 99-sd->status.dex : psd->status.dex/2;
				sd->paramb[5] += sd->status.luk+psd->status.luk/2 > 99 ? 99-sd->status.luk : psd->status.luk/2;
			}
		}
	}

	// Calculate total stats
	sd->paramc[0]=sd->status.str+sd->paramb[0]+sd->parame[0];
	sd->paramc[1]=sd->status.agi+sd->paramb[1]+sd->parame[1];
	sd->paramc[2]=sd->status.vit+sd->paramb[2]+sd->parame[2];
	sd->paramc[3]=sd->status.int_+sd->paramb[3]+sd->parame[3];
	sd->paramc[4]=sd->status.dex+sd->paramb[4]+sd->parame[4];
	sd->paramc[5]=sd->status.luk+sd->paramb[5]+sd->parame[5];

	if(sd->sc_count){
		if(sd->sc_data[SC_SPIRIT].timer!=-1 && sd->sc_data[SC_SPIRIT].val2 == SL_HIGH)
	  	{	//Ups any status under 50 to 50.
			if (sd->paramc[0] < 50) {
				sd->paramb[0] += 50-sd->paramc[0];
				sd->paramc[0] = 50;
			}
			if (sd->paramc[1] < 50) {
				sd->paramb[1] += 50-sd->paramc[1];
				sd->paramc[1] = 50;
			}
			if (sd->paramc[2] < 50) {
				sd->paramb[2] += 50-sd->paramc[2];
				sd->paramc[2] = 50;
			}
			if (sd->paramc[3] < 50) {
				sd->paramb[3] += 50-sd->paramc[3];
				sd->paramc[3] = 50;
			}
			if (sd->paramc[4] < 50) {
				sd->paramb[4] += 50-sd->paramc[4];
				sd->paramc[4] = 50;
			}
			if (sd->paramc[5] < 50) {
				sd->paramb[5] += 50-sd->paramc[5];
				sd->paramc[5] = 50;
			}
		}
	}
	for(i=0;i<6;i++)
		if(sd->paramc[i] < 0) sd->paramc[i] = 0;

	if (sd->sc_count && sd->sc_data[SC_CURSE].timer!=-1)
		sd->paramc[5] = 0;

// ------ BASE ATTACK CALCULATION ------

	// Basic Base ATK value
	switch(sd->status.weapon){
		case 11: // Bows
		case 13: // Musical Instruments
		case 14: // Whips
			str = sd->paramc[4];
			dex = sd->paramc[0];
			break;
		default:
			str = sd->paramc[0];
			dex = sd->paramc[4];
			break;
	}
	dstr = str/10;
	sd->base_atk += str + dstr*dstr + dex/5 + sd->paramc[5]/5;

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,BS_HILTBINDING))>0)
		sd->base_atk += 4;

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
		sd->base_atk = status_calc_batk(&sd->bl,sd->base_atk);

	if(sd->base_atk < 1)
		sd->base_atk = 1;

// ----- WEAPON ATK CALCULATION -----

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->right_weapon.watk = status_calc_watk(&sd->bl,sd->right_weapon.watk);
	if((index= sd->equip_index[8]) >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == 4)
		sd->left_weapon.watk = status_calc_watk(&sd->bl,sd->left_weapon.watk);

// ----- WEAPON UPGRADE ATK CALCULATION -----

	// Absolute modifiers from status changes (only for PC)
	if(sd->sc_count){
		if(sd->sc_data[SC_NIBELUNGEN].timer!=-1){
			index = sd->equip_index[9];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				sd->right_weapon.watk2 += sd->sc_data[SC_NIBELUNGEN].val2;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				sd->left_weapon.watk2 += sd->sc_data[SC_NIBELUNGEN].val2;
		}
	}

// ----- MATK CALCULATION -----

	// Basic MATK value
	sd->matk1 += sd->paramc[3]+(sd->paramc[3]/5)*(sd->paramc[3]/5);
	sd->matk2 += sd->paramc[3]+(sd->paramc[3]/7)*(sd->paramc[3]/7);
	if(sd->matk1 < sd->matk2) {
		int temp = sd->matk2;
		sd->matk2 = sd->matk1;
		sd->matk1 = temp;
	}

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->matk1 = status_calc_matk(&sd->bl,sd->matk1);
	sd->matk2 = status_calc_matk(&sd->bl,sd->matk2);

	// Apply relative modifiers from equipment
	if(sd->matk_rate != 100){
		sd->matk1 = sd->matk1 * sd->matk_rate/100;
		sd->matk2 = sd->matk2 * sd->matk_rate/100;
	}

// ----- CRIT CALCULATION -----

	// Basic Crit value
	sd->critical += (sd->paramc[5]*3)+10;

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->critical = status_calc_critical(&sd->bl,sd->critical);

	// Apply relative modifiers from equipment
	if(sd->critical_rate != 100)
		sd->critical = sd->critical * sd->critical_rate/100;

	if(sd->critical < 10) sd->critical = 10;

// ----- HIT CALCULATION -----

	// Basic Hit value
	sd->hit += sd->paramc[4] + sd->status.base_level;

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,BS_WEAPONRESEARCH))>0)
		sd->hit += skill*2;
	if((skill=pc_checkskill(sd,AC_VULTURE))>0){
		sd->hit += skill;
		if(sd->status.weapon == 11)
			sd->attackrange += skill;
	}

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->hit = status_calc_hit(&sd->bl,sd->hit);

	// Apply relative modifiers from equipment
	if(sd->hit_rate != 100)
		sd->hit = sd->hit * sd->hit_rate/100;

	if(sd->hit < 1) sd->hit = 1;

// ----- FLEE CALCULATION -----

	// Basic Flee value
	sd->flee += sd->paramc[1] + sd->status.base_level;

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,TF_MISS))>0)
		sd->flee += skill*(sd->class_&JOBL_2 && (sd->class_&MAPID_BASEMASK) == MAPID_THIEF? 4 : 3);
	if((skill=pc_checkskill(sd,MO_DODGE))>0)
		sd->flee += (skill*3)>>1;

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->flee = status_calc_flee(&sd->bl,sd->flee);

	// Apply relative modifiers from equipment
	if(sd->flee_rate != 100)
		sd->flee = sd->flee * sd->flee_rate/100;

	if(sd->flee < 1) sd->flee = 1;

// ----- PERFECT DODGE CALCULATION -----

	// Basic Perfect Dodge value
	sd->flee2 += sd->paramc[5]+10;

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->flee2 = status_calc_flee2(&sd->bl,sd->flee2);

	// Apply relative modifiers from equipment
	if(sd->flee2_rate != 100)
		sd->flee2 = sd->flee2 * sd->flee2_rate/100;

	if(sd->flee2 < 10) sd->flee2 = 10;

// ----- VIT-DEF CALCULATION -----

	// Special fixed values from status changes
	if(sd->sc_count && sd->sc_data[SC_BERSERK].timer!=-1)
		sd->def2 = 0;
	else if(sd->sc_count && sd->sc_data[SC_ETERNALCHAOS].timer!=-1)
		sd->def2 = 0;
	else {
		// Basic VIT-DEF value
		sd->def2 += sd->paramc[2];

		// Absolute, then relative modifiers from status changes (shared between PC and NPC)
		sd->def2 = status_calc_def2(&sd->bl,sd->def2);

		// Apply relative modifiers from equipment
		if(sd->def2_rate != 100)
			sd->def2 = sd->def2 * sd->def2_rate/100;

		if(sd->def2 < 1) sd->def2 = 1;
	}

// ----- EQUIPMENT-DEF CALCULATION -----

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->def = status_calc_def(&sd->bl,sd->def);

	// Apply relative modifiers from equipment
	if(sd->def_rate != 100)
		sd->def = sd->def * sd->def_rate/100;

	if(sd->def < 0) sd->def = 0;

	else if (!battle_config.player_defense_type && sd->def > battle_config.max_def)
	{
		sd->def2 += battle_config.over_def_bonus*(sd->def -battle_config.max_def);
		sd->def = battle_config.max_def;
	}

// ----- INT-MDEF CALCULATION -----

	// Special fixed values from status changes
	if(sd->sc_count && sd->sc_data[SC_BERSERK].timer!=-1)
		sd->mdef2 = 0;
	else {
		// Basic INT-MDEF value
		sd->mdef2 += sd->paramc[3];

		// sd->mdef2 = status_calc_mdef2(&sd->bl,sd->mdef2);

		// Apply relative modifiers from equipment
		if(sd->mdef2_rate != 100)
			sd->mdef2 = sd->mdef2 * sd->mdef2_rate/100;

		if(sd->mdef2 < 1) sd->mdef2 = 1;
	}

// ----- EQUIPMENT-MDEF CALCULATION -----

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->mdef = status_calc_mdef(&sd->bl,sd->mdef);

	// Apply relative modifiers from equipment
	if(sd->mdef_rate != 100)
		sd->mdef = sd->mdef * sd->mdef_rate/100;

	if(sd->mdef < 0) sd->mdef = 0;

	else if (!battle_config.player_defense_type && sd->mdef > battle_config.max_def)
	{
		sd->mdef2 += battle_config.over_def_bonus*(sd->mdef -battle_config.max_def);
		sd->mdef = battle_config.max_def;
	}
	
// ----- WALKING SPEED CALCULATION -----

	// Relative, then absolute modifiers from status changes (shared between PC and NPC)
	sd->speed = status_calc_speed(&sd->bl,sd->speed);

	// Relative modifiers from passive skills
	if((skill=pc_checkskill(sd,TF_MISS))>0 && (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN && sd->sc_data[SC_CLOAKING].timer==-1)
		sd->speed -= sd->speed * skill/100;
	if(pc_isriding(sd) && pc_checkskill(sd,KN_RIDING)>0)
		sd->speed -= sd->speed * 25/100;
	if(pc_ishiding(sd) && (skill=pc_checkskill(sd,RG_TUNNELDRIVE))>0)
		sd->speed += sd->speed * (100-16*skill)/100;
	if(pc_iscarton(sd) && (skill=pc_checkskill(sd,MC_PUSHCART))>0)
		sd->speed += sd->speed * (100-10*skill)/100;
 	if(sd->skilltimer != -1 && (skill=pc_checkskill(sd,SA_FREECAST))>0) {
		sd->prev_speed = sd->speed; //Store previous speed to correctly restore it. [Skotlex]
		sd->speed += sd->speed * (75-5*skill)/100;
	}
	if(sd->sc_count && sd->sc_data[SC_DANCING].timer!=-1){
			int s_rate = 500-40*pc_checkskill(sd,(sd->status.sex?BA_MUSICALLESSON:DC_DANCINGLESSON));
			if (sd->sc_data[SC_SPIRIT].timer != -1 && sd->sc_data[SC_SPIRIT].val2 == SL_BARDDANCER)
				s_rate -= 40; //TODO: Figure out real bonus rate.
			if (sd->sc_data[SC_LONGING].timer!=-1)
				s_rate -= 20 * sd->sc_data[SC_LONGING].val1;
			sd->speed += sd->speed * s_rate/100;
	}
	if(sd->sc_data[SC_FUSION].timer != -1) //Additional movement speed from SG_FUSION [Komurka]
		sd->speed -= sd->speed * 25/100;

	// Apply relative modifiers from equipment
	if(sd->speed_rate != 100)
		sd->speed = sd->speed*sd->speed_rate/100;

	if(sd->speed < battle_config.max_walk_speed)
		sd->speed = battle_config.max_walk_speed;

// ----- ASPD CALCULATION -----
// Unlike other stats, ASPD rate modifiers from skills/SCs/items/etc are first all added together, then the final modifier is applied

	// Basic ASPD value
	if (sd->status.weapon <= 16)
		sd->aspd += aspd_base[sd->status.class_][sd->status.weapon]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[sd->status.class_][sd->status.weapon]/1000;
	else
		sd->aspd += (
			(aspd_base[sd->status.class_][sd->weapontype1]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[sd->status.class_][sd->weapontype1]/1000) +
			(aspd_base[sd->status.class_][sd->weapontype2]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[sd->status.class_][sd->weapontype2]/1000)
			) *2/3; //From what I read in rodatazone, 2/3 should be more accurate than 0.7 -> 140 / 200; [Skotlex]

	// Relative modifiers from passive skills
	if((skill=pc_checkskill(sd,SA_ADVANCEDBOOK))>0)
		sd->aspd_rate -= (skill/2);
	if((skill = pc_checkskill(sd,SG_DEVIL)) > 0 && sd->status.job_level >= battle_config.max_job_level)
		sd->aspd_rate -= (skill*3);

	if(pc_isriding(sd))
		sd->aspd_rate += 50-10*pc_checkskill(sd,KN_CAVALIERMASTERY);

	// Relative modifiers from status changes (shared between PC and NPC)
	sd->aspd_rate = status_calc_aspd_rate(&sd->bl,sd->aspd_rate);

	// Apply all relative modifiers
	if(sd->aspd_rate != 100)
		sd->aspd = sd->aspd*sd->aspd_rate/100;

	if(sd->aspd < battle_config.max_aspd) sd->aspd = battle_config.max_aspd;
	sd->amotion = sd->aspd;

// ----- HP MAX AND REGEN CALCULATION -----

	// Basic MaxHP value
	// here we recycle variable index, and do this calc apart to avoid mixing up the 30% bonus with card bonuses. [Skotlex]
	bl = sd->status.base_level;
	index = (3500 + bl*hp_coefficient2[sd->status.class_] +
		hp_sigma_val[sd->status.class_][(bl > 0)? bl-1:0])/100 *
		(100 + sd->paramc[2])/100 + (sd->parame[2] - sd->paramcard[2]);
	if (sd->class_&JOBL_UPPER)
		index += index * 30/100;
	else if (sd->class_&JOBL_BABY)
		index -= index * 30/100;
	if ((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && pc_istop10fame(sd->char_id, MAPID_TAEKWON))
		index *= 3; //Triple max HP for top ranking Taekwons over level 90.
	
	sd->status.max_hp += index;

	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 99)
		sd->status.max_hp = sd->status.max_hp + 2000;

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,CR_TRUST))>0)
		sd->status.max_hp += skill*200;

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->status.max_hp = status_calc_maxhp(&sd->bl,sd->status.max_hp);

	// Apply relative modifiers from equipment
	if(sd->hprate!=100)
		sd->status.max_hp = sd->status.max_hp * sd->hprate/100;

	if(sd->status.max_hp > battle_config.max_hp)
		sd->status.max_hp = battle_config.max_hp;
	if(sd->status.hp>sd->status.max_hp)
		sd->status.hp=sd->status.max_hp;
	if(sd->status.max_hp <= 0) sd->status.max_hp = 1;

	// Basic natural HP regeneration value
	sd->nhealhp = 1 + (sd->paramc[2]/5) + (sd->status.max_hp/200);

	// Apply relative modifiers from equipment
	if(sd->hprecov_rate != 100)
		sd->nhealhp = sd->nhealhp*sd->hprecov_rate/100;

	if(sd->nhealhp < 1) sd->nhealhp = 1;
	if(sd->nhealhp > 0x7fff) sd->nhealhp = 0x7fff;

	// Skill-related HP recovery
	if((skill=pc_checkskill(sd,SM_RECOVERY)) > 0)
		sd->nshealhp = skill*5 + (sd->status.max_hp*skill/500);
	// Skill-related HP recovery (only when sit)
	if((skill=pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0)
		sd->nsshealhp = skill*4 + (sd->status.max_hp*skill/500);
	if((skill=pc_checkskill(sd,TK_HPTIME)) > 0 && sd->state.rest == 1)
		sd->nsshealhp = skill*30 + (sd->status.max_hp*skill/500);

	if(sd->nshealhp > 0x7fff) sd->nshealhp = 0x7fff;
	if(sd->nsshealhp > 0x7fff) sd->nsshealhp = 0x7fff;

// ----- SP MAX AND REGEN CALCULATION -----

	// Basic MaxSP value
	// here we recycle variable index, and do this calc apart to avoid mixing up the 30% bonus with card bonuses. [Skotlex]
	index = ((sp_coefficient[sd->status.class_] * bl) + 1000)/100 * (100 + sd->paramc[3])/100 + (sd->parame[3] - sd->paramcard[3]);
	if (sd->class_&JOBL_UPPER)
		index += index * 30/100;
	else if (sd->class_&JOBL_BABY)
		index -= index * 30/100;
	if ((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && pc_istop10fame(sd->char_id, MAPID_TAEKWON))
		index *= 3; //Triple max SP for top ranking Taekwons over level 90.
	
	sd->status.max_sp += index;

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,SL_KAINA))>0)
		sd->status.max_sp += 30*skill;
	if((skill=pc_checkskill(sd,HP_MEDITATIO))>0)
		sd->status.max_sp += sd->status.max_sp * skill/100;
	if((skill=pc_checkskill(sd,HW_SOULDRAIN))>0)
		sd->status.max_sp += sd->status.max_sp * 2*skill/100;

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	sd->status.max_sp = status_calc_maxsp(&sd->bl,sd->status.max_sp);

	// Apply relative modifiers from equipment
	if(sd->sprate!=100)
		sd->status.max_sp = sd->status.max_sp * sd->sprate/100;

	if(sd->status.max_sp > battle_config.max_sp)
		sd->status.max_sp = battle_config.max_sp;
	if(sd->status.sp>sd->status.max_sp)
		sd->status.sp=sd->status.max_sp;
	if(sd->status.max_sp <= 0) sd->status.max_sp = 1;

	if(sd->sc_data[SC_DANCING].timer==-1){
		// Basic natural SP regeneration value
		sd->nhealsp = 1 + (sd->paramc[3]/6) + (sd->status.max_sp/100);
		if(sd->paramc[3] >= 120)
			sd->nhealsp += ((sd->paramc[3]-120)>>1) + 4;

		// Relative modifiers from passive skills
		if((skill=pc_checkskill(sd,HP_MEDITATIO)) > 0)
			sd->nhealsp += sd->nhealsp * 3*skill/100;

		// Apply relative modifiers from equipment
		if(sd->sprecov_rate != 100)
			sd->nhealsp = sd->nhealsp*sd->sprecov_rate/100;

		if(sd->nhealsp < 1) sd->nhealsp = 1;
		if(sd->nhealsp > 0x7fff) sd->nhealsp = 0x7fff;

		// Skill-related SP recovery
		if((skill=pc_checkskill(sd,MG_SRECOVERY)) > 0)
			sd->nshealsp = skill*3 + (sd->status.max_sp*skill/500);
		// Skill-related SP recovery (only when sit)
		if((skill = pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0)
			sd->nsshealsp = skill*2 + (sd->status.max_sp*skill/500);
		if((skill=pc_checkskill(sd,TK_SPTIME)) > 0 && sd->state.rest == 1) {
			sd->nsshealsp = skill*3 + (sd->status.max_sp*skill/500);
			if ((skill=pc_checkskill(sd,SL_KAINA)) > 0) //Power up Enjoyable Rest
				sd->nsshealsp += (30+10*skill)*sd->nsshealsp/100;
		}
		if(sd->nshealsp > 0x7fff) sd->nshealsp = 0x7fff;
		if(sd->nsshealsp > 0x7fff) sd->nsshealsp = 0x7fff;
	}

// ----- MISC CALCULATIONS -----

	//Even though people insist this is too slow, packet data reports this is the actual real equation.
	sd->dmotion = 800-sd->paramc[1]*4;
	if(sd->dmotion<400)	sd->dmotion = 400;

	// Weight
	if((skill=pc_checkskill(sd,MC_INCCARRY))>0)
		sd->max_weight += 2000*skill;
	if(pc_isriding(sd) && pc_checkskill(sd,KN_RIDING)>0)
		sd->max_weight += 10000;
	if( (skill=pc_checkskill(sd,SG_KNOWLEDGE))>0) //SG skill [Komurka]
		if(sd->bl.m == sd->feel_map[0].m || sd->bl.m == sd->feel_map[1].m || sd->bl.m == sd->feel_map[2].m)
			sd->max_weight += sd->max_weight*skill/10;

	// Skill SP cost
	if((skill=pc_checkskill(sd,HP_MANARECHARGE))>0 )
		sd->dsprate -= 4*skill;

	if(sd->sc_count){
		if(sd->sc_data[SC_SERVICE4U].timer!=-1)
			sd->dsprate -= sd->sc_data[SC_SERVICE4U].val3;
	}

	if(sd->dsprate < 0) sd->dsprate = 0;

	// Anti-element and anti-race
	if((skill=pc_checkskill(sd,CR_TRUST))>0)
		sd->subele[6] += skill*5;
	if((skill=pc_checkskill(sd,BS_SKINTEMPER))>0) {
		sd->subele[0] += skill;
		sd->subele[3] += skill*4;
	}
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0 ){
		skill = skill*4;
		sd->right_weapon.addrace[9]+=skill;
		sd->left_weapon.addrace[9]+=skill;
		sd->magic_addrace[9]+=skill;
		sd->subrace[9]+=skill;
	}

	if(sd->sc_count){
       	if(sd->sc_data[SC_SIEGFRIED].timer!=-1){
			sd->subele[1] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[2] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[3] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[4] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[5] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[6] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[7] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[8] += sd->sc_data[SC_SIEGFRIED].val2;
			sd->subele[9] += sd->sc_data[SC_SIEGFRIED].val2;
		}
		if(sd->sc_data[SC_PROVIDENCE].timer!=-1){
			sd->subele[6] += sd->sc_data[SC_PROVIDENCE].val2;
			sd->subrace[6] += sd->sc_data[SC_PROVIDENCE].val2;
		}
	}

// ----- CLIENT-SIDE REFRESH -----
	if(first&1) { //Since this is the initial loading, the Falcon and Peco icons must be loaded. [Skotlex]
		if (sd->status.option&OPTION_FALCON)
			clif_status_load(&sd->bl, SI_FALCON, 1);
		if (sd->status.option&OPTION_RIDING)
			clif_status_load(&sd->bl, SI_RIDING, 1);
	}
	if(first&4) {
		calculating = 0;
		return 0;
	}
	if(first&3) {
		clif_updatestatus(sd,SP_SPEED);
		clif_updatestatus(sd,SP_MAXHP);
		clif_updatestatus(sd,SP_MAXSP);
		if(first&1) {
			clif_updatestatus(sd,SP_HP);
			clif_updatestatus(sd,SP_SP);
		}
		calculating = 0;
		return 0;
	}

	
	if(sd->sc_data[SC_WEDDING].timer != -1 && sd->view_class != JOB_WEDDING)
		sd->view_class=JOB_WEDDING;
	
	if(sd->sc_data[SC_XMAS].timer != -1 && sd->view_class != JOB_XMAS)
		sd->view_class=JOB_XMAS;

	if(b_class != sd->view_class) {
		clif_changelook(&sd->bl,LOOK_BASE,sd->view_class);
#if PACKETVER < 4
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
#else
		clif_changelook(&sd->bl,LOOK_WEAPON,0);
#endif
	//Restoring cloth dye color after the view class changes. [Skotlex]
	// Added Xmas Suit [Valaris]
	if(battle_config.save_clothcolor && sd->status.clothes_color > 0 &&
		((sd->view_class != JOB_WEDDING && sd->view_class !=JOB_XMAS) || (sd->view_class==JOB_WEDDING && !battle_config.wedding_ignorepalette) ||
			 (sd->view_class==JOB_XMAS && !battle_config.xmas_ignorepalette)))
			clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->status.clothes_color);
	}

	if( memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill)) || b_attackrange != sd->attackrange)
		clif_skillinfoblock(sd);

	if(b_speed != sd->speed)
		clif_updatestatus(sd,SP_SPEED);
	if(b_weight != sd->weight)
		clif_updatestatus(sd,SP_WEIGHT);
	if(b_max_weight != sd->max_weight) {
		clif_updatestatus(sd,SP_MAXWEIGHT);
		pc_checkweighticon(sd);
	}
	for(i=0;i<6;i++)
		if(b_paramb[i] + b_parame[i] != sd->paramb[i] + sd->parame[i])
			clif_updatestatus(sd,SP_STR+i);
	if(b_hit != sd->hit)
		clif_updatestatus(sd,SP_HIT);
	if(b_flee != sd->flee)
		clif_updatestatus(sd,SP_FLEE1);
	if(b_aspd != sd->aspd)
		clif_updatestatus(sd,SP_ASPD);
	if(b_watk != sd->right_weapon.watk || b_base_atk != sd->base_atk)
		clif_updatestatus(sd,SP_ATK1);
	if(b_def != sd->def)
		clif_updatestatus(sd,SP_DEF1);
	if(b_watk2 != sd->right_weapon.watk2)
		clif_updatestatus(sd,SP_ATK2);
	if(b_def2 != sd->def2)
		clif_updatestatus(sd,SP_DEF2);
	if(b_flee2 != sd->flee2)
		clif_updatestatus(sd,SP_FLEE2);
	if(b_critical != sd->critical)
		clif_updatestatus(sd,SP_CRITICAL);
	if(b_matk1 != sd->matk1)
		clif_updatestatus(sd,SP_MATK1);
	if(b_matk2 != sd->matk2)
		clif_updatestatus(sd,SP_MATK2);
	if(b_mdef != sd->mdef)
		clif_updatestatus(sd,SP_MDEF1);
	if(b_mdef2 != sd->mdef2)
		clif_updatestatus(sd,SP_MDEF2);
	if(b_attackrange != sd->attackrange)
		clif_updatestatus(sd,SP_ATTACKRANGE);
	if(b_max_hp != sd->status.max_hp)
		clif_updatestatus(sd,SP_MAXHP);
	if(b_max_sp != sd->status.max_sp)
		clif_updatestatus(sd,SP_MAXSP);
	if(b_hp != sd->status.hp)
		clif_updatestatus(sd,SP_HP);
	if(b_sp != sd->status.sp)
		clif_updatestatus(sd,SP_SP);

	/* I don't think there's a need for this here. It should be handled in pc_damage and pc_heal. [Skotlex]
	if(sd->status.hp<sd->status.max_hp>>2 && sd->sc_data[SC_AUTOBERSERK].timer!=-1 &&
		(sd->sc_data[SC_PROVOKE].timer==-1 || sd->sc_data[SC_PROVOKE].val2==0) && !pc_isdead(sd))
		status_change_start(&sd->bl,SC_PROVOKE,10,1,0,0,0,0);
	*/
	calculating = 0;
	return 0;
}

/*==========================================
 * Apply shared stat mods from status changes [DracoRPG]
 *------------------------------------------
 */
int status_calc_str(struct block_list *bl, int str)
{
	struct status_change *sc_data;
	nullpo_retr(str,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCALLSTATUS].timer!=-1)
			str += sc_data[SC_INCALLSTATUS].val1;
		if(sc_data[SC_INCSTR].timer!=-1)
			str += sc_data[SC_INCSTR].val1;
		if(sc_data[SC_STRFOOD].timer!=-1)
			str += sc_data[SC_STRFOOD].val1;
		if(sc_data[SC_LOUD].timer!=-1)
			str += 4;
  		if(sc_data[SC_TRUESIGHT].timer!=-1)
			str += 5;
		if(sc_data[SC_SPURT].timer!=-1)
			str += 10; //Bonus is +!0 regardless of skill level
		if(sc_data[SC_BLESSING].timer != -1){
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6)
				str >>= 1;
			else str += sc_data[SC_BLESSING].val1;
		}
	}

	return str;
}

int status_calc_agi(struct block_list *bl, int agi)
{
	struct status_change *sc_data;
	nullpo_retr(agi,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCALLSTATUS].timer!=-1)
			agi += sc_data[SC_INCALLSTATUS].val1;
		if(sc_data[SC_INCAGI].timer!=-1)
			agi += sc_data[SC_INCAGI].val1;
		if(sc_data[SC_AGIFOOD].timer!=-1)
			agi += sc_data[SC_AGIFOOD].val1;
  		if(sc_data[SC_TRUESIGHT].timer!=-1)
			agi += 5;
		if(sc_data[SC_INCREASEAGI].timer!=-1)
			agi += 2 + sc_data[SC_INCREASEAGI].val1;
		if(sc_data[SC_DECREASEAGI].timer!=-1)
			agi -= 2 + sc_data[SC_DECREASEAGI].val1;
		if(sc_data[SC_QUAGMIRE].timer!=-1)
			agi -= sc_data[SC_QUAGMIRE].val1*(bl->type==BL_PC?5:10);
	}

	return agi;
}

int status_calc_vit(struct block_list *bl, int vit)
{
	struct status_change *sc_data;
	nullpo_retr(vit,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCALLSTATUS].timer!=-1)
			vit += sc_data[SC_INCALLSTATUS].val1;
		if(sc_data[SC_INCVIT].timer!=-1)
			vit += sc_data[SC_INCVIT].val1;
		if(sc_data[SC_VITFOOD].timer!=-1)
			vit += sc_data[SC_VITFOOD].val1;
  		if(sc_data[SC_TRUESIGHT].timer!=-1)
			vit += 5;
		if(sc_data[SC_STRIPARMOR].timer!=-1 && bl->type != BL_PC)
			vit -= vit * 8*sc_data[SC_STRIPARMOR].val1/100;
	}

	return vit;
}

int status_calc_int(struct block_list *bl, int int_)
{
	struct status_change *sc_data;
	nullpo_retr(int_,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCALLSTATUS].timer!=-1)
			int_ += sc_data[SC_INCALLSTATUS].val1;
		if(sc_data[SC_INCINT].timer!=-1)
			int_ += sc_data[SC_INCINT].val1;
		if(sc_data[SC_INTFOOD].timer!=-1)
			int_ += sc_data[SC_INTFOOD].val1;
  		if(sc_data[SC_TRUESIGHT].timer!=-1)
			int_ += 5;
		if(sc_data[SC_BLESSING].timer != -1){
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6)
				int_ >>= 1;
			else int_ += sc_data[SC_BLESSING].val1;
		}
		if(sc_data[SC_STRIPHELM].timer!=-1 && bl->type != BL_PC)
			int_ -= int_ * 8*sc_data[SC_STRIPHELM].val1/100;
	}

	return int_;
}

int status_calc_dex(struct block_list *bl, int dex)
{
	struct status_change *sc_data;
	nullpo_retr(dex,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCALLSTATUS].timer!=-1)
			dex += sc_data[SC_INCALLSTATUS].val1;
		if(sc_data[SC_INCDEX].timer!=-1)
			dex += sc_data[SC_INCDEX].val1;
		if(sc_data[SC_DEXFOOD].timer!=-1)
			dex += sc_data[SC_DEXFOOD].val1;
  		if(sc_data[SC_TRUESIGHT].timer!=-1)
			dex += 5;
		if(sc_data[SC_QUAGMIRE].timer!=-1)
			dex -= sc_data[SC_QUAGMIRE].val1*(bl->type==BL_PC?5:10);
		if(sc_data[SC_BLESSING].timer != -1){
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6)
				dex >>= 1;
			else dex += sc_data[SC_BLESSING].val1;
		}
	}

	return dex;
}

int status_calc_luk(struct block_list *bl, int luk)
{
	struct status_change *sc_data;
	nullpo_retr(luk,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCALLSTATUS].timer!=-1)
			luk += sc_data[SC_INCALLSTATUS].val1;
		if(sc_data[SC_INCLUK].timer!=-1)
			luk += sc_data[SC_INCLUK].val1;
		if(sc_data[SC_LUKFOOD].timer!=-1)
			luk += sc_data[SC_LUKFOOD].val1;
  		if(sc_data[SC_TRUESIGHT].timer!=-1)
			luk += 5;
  		if(sc_data[SC_GLORIA].timer!=-1)
			luk += 30;
	}

	return luk;
}

int status_calc_batk(struct block_list *bl, int batk)
{
	struct status_change *sc_data;
	nullpo_retr(batk,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_ATKPOTION].timer!=-1)
			batk += sc_data[SC_ATKPOTION].val1;
		if(sc_data[SC_BATKFOOD].timer!=-1)
			batk += sc_data[SC_BATKFOOD].val1;
		if(sc_data[SC_INCATKRATE].timer!=-1)
			batk += batk * sc_data[SC_INCATKRATE].val1/100;
		if(sc_data[SC_PROVOKE].timer!=-1)
			batk += batk * (2+3*sc_data[SC_PROVOKE].val1)/100;
		if(sc_data[SC_CONCENTRATION].timer!=-1)
			batk += batk * 5*sc_data[SC_CONCENTRATION].val1/100;
		if(sc_data[SC_SKE].timer!=-1)
			batk += batk * 3;
		if(sc_data[SC_JOINTBEAT].timer!=-1 && sc_data[SC_JOINTBEAT].val2==4)
  			batk -= batk * 25/100;
		if(sc_data[SC_CURSE].timer!=-1)
  			batk -= batk * 25/100;
	}

	return batk;
}

int status_calc_watk(struct block_list *bl, int watk)
{
	struct status_change *sc_data;
	nullpo_retr(watk,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_IMPOSITIO].timer!=-1)
			watk += 5*sc_data[SC_IMPOSITIO].val1;
		if(sc_data[SC_WATKFOOD].timer!=-1)
			watk += sc_data[SC_WATKFOOD].val1;
		if(sc_data[SC_DRUMBATTLE].timer!=-1)
			watk += sc_data[SC_DRUMBATTLE].val2;
		if(sc_data[SC_VOLCANO].timer!=-1 && status_get_elem_type(bl)==3)
			watk += sc_data[SC_VOLCANO].val3;
		if(sc_data[SC_INCATKRATE].timer!=-1)
			watk += watk * sc_data[SC_INCATKRATE].val1/100;
		if(sc_data[SC_PROVOKE].timer!=-1)
			watk += watk * (2+3*sc_data[SC_PROVOKE].val1)/100;
		if(sc_data[SC_CONCENTRATION].timer!=-1)
			watk += watk * 5*sc_data[SC_CONCENTRATION].val1/100;
		if(sc_data[SC_SKE].timer!=-1)
			watk += watk * 3;
		if(sc_data[SC_NIBELUNGEN].timer!=-1 && bl->type != BL_PC && (status_get_element(bl)/10)>=8)
			watk += sc_data[SC_NIBELUNGEN].val2;
		if(sc_data[SC_EXPLOSIONSPIRITS].timer!=-1 && bl->type != BL_PC)
			watk += (1000*sc_data[SC_EXPLOSIONSPIRITS].val1);
		if(sc_data[SC_CURSE].timer!=-1)
			watk -= watk * 25/100;
		if(sc_data[SC_STRIPWEAPON].timer!=-1 && bl->type != BL_PC)
			watk -= watk * 5*sc_data[SC_STRIPWEAPON].val1/100;
	}
	return watk;
}

int status_calc_matk(struct block_list *bl, int matk)
{
	struct status_change *sc_data;
	nullpo_retr(matk,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_MATKPOTION].timer!=-1)
			matk += sc_data[SC_MATKPOTION].val1;
		if(sc_data[SC_MATKFOOD].timer!=-1)
			matk += sc_data[SC_MATKFOOD].val1;
		if(sc_data[SC_MAGICPOWER].timer!=-1)
			matk += matk * 5*sc_data[SC_MAGICPOWER].val1/100;
		if(sc_data[SC_MINDBREAKER].timer!=-1)
			matk += matk * 20*sc_data[SC_MINDBREAKER].val1/100;
		if(sc_data[SC_INCMATKRATE].timer!=-1)
			matk += matk * sc_data[SC_INCMATKRATE].val1/100;
	}

	return matk;
}

int status_calc_critical(struct block_list *bl, int critical)
{
	struct status_change *sc_data;
	nullpo_retr(critical,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if (sc_data[SC_EXPLOSIONSPIRITS].timer!=-1)
			critical += sc_data[SC_EXPLOSIONSPIRITS].val2;
		if (sc_data[SC_FORTUNE].timer!=-1)
			critical += sc_data[SC_FORTUNE].val2*10;
		if (sc_data[SC_TRUESIGHT].timer!=-1)
			critical += sc_data[SC_TRUESIGHT].val1*10;
		if(sc_data[SC_CLOAKING].timer!=-1)
			critical += critical;
	}

	return critical;
}

int status_calc_hit(struct block_list *bl, int hit)
{
	struct status_change *sc_data;
	nullpo_retr(hit,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCHIT].timer != -1)
			hit += sc_data[SC_INCHIT].val1;
		if(sc_data[SC_HITFOOD].timer!=-1)
			hit += sc_data[SC_HITFOOD].val1;
		if(sc_data[SC_TRUESIGHT].timer != -1)
			hit += 3*sc_data[SC_TRUESIGHT].val1;
		if(sc_data[SC_HUMMING].timer!=-1)
			hit += sc_data[SC_HUMMING].val2;
		if(sc_data[SC_CONCENTRATION].timer != -1)
			hit += 10*sc_data[SC_CONCENTRATION].val1;
		if(sc_data[SC_INCHITRATE].timer != -1)
			hit += hit * sc_data[SC_INCHITRATE].val1/100;
		if(sc_data[SC_BLIND].timer != -1)
			hit -= hit * 25 / 100;
	}

	return hit;
}

int status_calc_flee(struct block_list *bl, int flee)
{
	struct status_change *sc_data;
	nullpo_retr(flee,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_INCFLEE].timer!=-1)
			flee += sc_data[SC_INCFLEE].val1;
		if(sc_data[SC_FLEEFOOD].timer!=-1)
			flee += sc_data[SC_FLEEFOOD].val1;
		if(sc_data[SC_WHISTLE].timer!=-1)
			flee += sc_data[SC_WHISTLE].val2;
		if(sc_data[SC_WINDWALK].timer!=-1)
			flee += flee * sc_data[SC_WINDWALK].val2/100;
		if(sc_data[SC_INCFLEERATE].timer!=-1)
			flee += flee * sc_data[SC_INCFLEERATE].val1/100;
		if(sc_data[SC_VIOLENTGALE].timer!=-1 && status_get_elem_type(bl)==4)
			flee += flee * sc_data[SC_VIOLENTGALE].val3/100;
		if(sc_data[SC_MOON_COMFORT].timer!=-1) //SG skill [Komurka]
			flee += (status_get_lv(bl) + status_get_dex(bl) + status_get_luk(bl))/10;
		if(sc_data[SC_CLOSECONFINE].timer!=-1)
			flee += 10;
		if(sc_data[SC_SPIDERWEB].timer!=-1)
			flee -= flee * 50/100;
		if(sc_data[SC_BERSERK].timer!=-1)
			flee -= flee * 50/100;
		if(sc_data[SC_BLIND].timer!=-1)
			flee -= flee * 25/100;
	}

	if (bl->type == BL_PC && map_flag_gvg(bl->m)) //GVG grounds flee penalty, placed here because it's "like" a status change. [Skotlex]
		flee -= flee * battle_config.gvg_flee_penalty/100;
	return flee;
}

int status_calc_flee2(struct block_list *bl, int flee2)
{
	struct status_change *sc_data;
	nullpo_retr(flee2,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_WHISTLE].timer!=-1)
			flee2 += sc_data[SC_WHISTLE].val3*10;
	}

	return flee2;
}

int status_calc_def(struct block_list *bl, int def)
{
	struct status_change *sc_data;
	nullpo_retr(def,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc_data[SC_KEEPING].timer!=-1)
			return 100;
		if(sc_data[SC_STEELBODY].timer!=-1)
			return 90;
		if(sc_data[SC_SKA].timer != -1) // [marquis007]
			return 90;
		if(sc_data[SC_DRUMBATTLE].timer!=-1)
			def += sc_data[SC_DRUMBATTLE].val3;
		if(sc_data[SC_INCDEFRATE].timer!=-1)
			def += def * sc_data[SC_INCDEFRATE].val1/100;
		if(sc_data[SC_SIGNUMCRUCIS].timer!=-1)
			def -= def * sc_data[SC_SIGNUMCRUCIS].val2/100;
		if(sc_data[SC_CONCENTRATION].timer!=-1)
			def -= def * 5*sc_data[SC_CONCENTRATION].val1/100;
		if(sc_data[SC_SKE].timer!=-1)
			def -= def  * 50/100;
		if(sc_data[SC_PROVOKE].timer!=-1 && bl->type != BL_PC) // Provoke doesn't alter player defense.
			def -= def * (5+5*sc_data[SC_PROVOKE].val1)/100;
		if(sc_data[SC_STRIPSHIELD].timer!=-1 && bl->type != BL_PC)
			def -= def * 3*sc_data[SC_STRIPSHIELD].val1/100;
	}

	return def;
}

int status_calc_def2(struct block_list *bl, int def2)
{
	struct status_change *sc_data;
	nullpo_retr(def2,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc_data[SC_ETERNALCHAOS].timer!=-1)
			return 0;
		if(sc_data[SC_SUN_COMFORT].timer!=-1)
			def2 += (status_get_lv(bl) + status_get_dex(bl) + status_get_luk(bl))/2;
		if(sc_data[SC_ANGELUS].timer!=-1)
			def2 += def2 * (10+5*sc_data[SC_ANGELUS].val1)/100;
		if(sc_data[SC_CONCENTRATION].timer!=-1)
			def2 -= def2 * 5*sc_data[SC_CONCENTRATION].val1/100;
		if(sc_data[SC_POISON].timer!=-1)
			def2 -= def2 * 25/100;
		if(sc_data[SC_SKE].timer!=-1)
			def2 -= def2 * 50/100;
		if(sc_data[SC_PROVOKE].timer!=-1)
			def2 -= def2 * (5+5*sc_data[SC_PROVOKE].val1)/100;
		if(sc_data[SC_SKE].timer!=-1)
			def2 /= 2;
		if(sc_data[SC_JOINTBEAT].timer!=-1){
			if(sc_data[SC_JOINTBEAT].val2==3)
				def2 -= def2 * 50/100;
			else if(sc_data[SC_JOINTBEAT].val2==4)
				def2 -= def2 * 25/100;
		}
	}

	return def2;
}

int status_calc_mdef(struct block_list *bl, int mdef)
{
	struct status_change *sc_data;
	nullpo_retr(mdef,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc_data[SC_BARRIER].timer!=-1)
			return 100;
		if(sc_data[SC_STEELBODY].timer!=-1)
			return 90;
		if(sc_data[SC_SKA].timer != -1) // [marquis007]
			return 90; // should it up mdef too?
		if(sc_data[SC_ENDURE].timer!=-1)
			mdef += sc_data[SC_ENDURE].val1;
	}

	return mdef;
}

int status_calc_mdef2(struct block_list *bl, int mdef2)
{
	struct status_change *sc_data;
	nullpo_retr(mdef2,bl);
	sc_data = status_get_sc_data(bl);

	if(sc_data){
		if(sc_data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc_data[SC_MINDBREAKER].timer!=-1)
			mdef2 -= mdef2 * 12*sc_data[SC_MINDBREAKER].val1/100;
	}

	return mdef2;
}

int status_calc_speed(struct block_list *bl, int speed)
{
	struct status_change *sc_data;
	sc_data = status_get_sc_data(bl);

		if(sc_data) {
			if(sc_data[SC_CURSE].timer!=-1)
				speed += 450;
			if(sc_data[SC_SWOO].timer != -1) // [marquis007]
				speed += 450; //Let's use Curse's slow down momentarily (exact value unknown)
			if(sc_data[SC_SPEEDUP1].timer!=-1)
				speed -= speed*50/100;
			else if(sc_data[SC_SPEEDUP0].timer!=-1)
				speed -= speed*25/100;
			else if(sc_data[SC_INCREASEAGI].timer!=-1)
				speed -= speed * 25/100;
			else if(sc_data[SC_CARTBOOST].timer!=-1)
				speed -= speed * 20/100;
			else if(sc_data[SC_BERSERK].timer!=-1)
				speed -= speed * 20/100;
			else if(sc_data[SC_WINDWALK].timer!=-1)
				speed -= speed * 4*sc_data[SC_WINDWALK].val2/100;
			if(sc_data[SC_WEDDING].timer!=-1)
				speed += speed * 50/100;
			if(sc_data[SC_SLOWDOWN].timer!=-1)
				speed += speed * 50/100;
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				speed += speed * 25/100;
			if(sc_data[SC_STEELBODY].timer!=-1)
				speed += speed * 25/100;
			if(sc_data[SC_SKA].timer!=-1)
				speed += speed * 25/100;
			if(sc_data[SC_QUAGMIRE].timer!=-1)
				speed += speed * 50/100;
			if(sc_data[SC_DONTFORGETME].timer!=-1)
				speed += speed * sc_data[SC_DONTFORGETME].val3/100;
			if(sc_data[SC_DEFENDER].timer!=-1)
				speed += speed * (55-5*sc_data[SC_DEFENDER].val1)/100;
			if(sc_data[SC_GOSPEL].timer!=-1 && sc_data[SC_GOSPEL].val4 == BCT_ENEMY)
				speed += speed * 25/100;
			if(sc_data[SC_JOINTBEAT].timer!=-1) {
				if (sc_data[SC_JOINTBEAT].val2 == 0)
					speed += speed * 50/100;
				else if (sc_data[SC_JOINTBEAT].val2 == 2)
					speed += speed * 30/100;
			}
			if(sc_data[SC_CLOAKING].timer!=-1)
				speed = speed * (sc_data[SC_CLOAKING].val3-3*sc_data[SC_CLOAKING].val1) /100;
			if(sc_data[SC_CHASEWALK].timer!=-1)
				speed = speed * sc_data[SC_CHASEWALK].val3/100;
			if(sc_data[SC_RUN].timer!=-1)/*ãÏÇØë´Ç…ÇÊÇÈë¨ìxïœâª*/
				speed -= speed * 25/100;

		}

	return speed;
}

int status_calc_aspd_rate(struct block_list *bl, int aspd_rate)
{
	struct status_change *sc_data;
	sc_data = status_get_sc_data(bl);

		if(sc_data) {
			int i;
			if(sc_data[SC_QUAGMIRE].timer==-1 && sc_data[SC_DONTFORGETME].timer==-1){
				if(sc_data[SC_TWOHANDQUICKEN].timer!=-1)
					aspd_rate -= 30;
				else if(sc_data[SC_ONEHAND].timer!=-1)
					aspd_rate -= 30;
				else if(sc_data[SC_ADRENALINE2].timer!=-1)
					aspd_rate -= (sc_data[SC_ADRENALINE2].val2 || !battle_config.party_skill_penalty)?30:20;
				else if(sc_data[SC_ADRENALINE].timer!=-1)
					aspd_rate -= (sc_data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)?30:20;
				else if(sc_data[SC_SPEARSQUICKEN].timer!=-1)
					aspd_rate -= sc_data[SC_SPEARSQUICKEN].val2;
				else if(sc_data[SC_ASSNCROS].timer!=-1 && (bl->type!=BL_PC || ((struct map_session_data*)bl)->status.weapon != 11))
					aspd_rate -= sc_data[SC_ASSNCROS].val2;
			}
			if(sc_data[SC_BERSERK].timer!=-1)
				aspd_rate -= 30;
			if(sc_data[i=SC_ASPDPOTION3].timer!=-1 || sc_data[i=SC_ASPDPOTION2].timer!=-1 || sc_data[i=SC_ASPDPOTION1].timer!=-1 || sc_data[i=SC_ASPDPOTION0].timer!=-1)
				aspd_rate -= sc_data[i].val2;
			if(sc_data[SC_DONTFORGETME].timer!=-1)
				aspd_rate += sc_data[SC_DONTFORGETME].val2;
			if(sc_data[SC_STEELBODY].timer!=-1)
				aspd_rate += 25;
			if(sc_data[SC_SKA].timer!=-1)
				aspd_rate += 25;
			if(sc_data[SC_DEFENDER].timer != -1)
				aspd_rate += 25 -sc_data[SC_DEFENDER].val1*5;
			if(sc_data[SC_GOSPEL].timer!=-1 && sc_data[SC_GOSPEL].val4 == BCT_ENEMY)
				aspd_rate += 25;
			if(sc_data[SC_GRAVITATION].timer!=-1)
				aspd_rate += sc_data[SC_GRAVITATION].val2;
			if(sc_data[SC_JOINTBEAT].timer!=-1) {
				if (sc_data[SC_JOINTBEAT].val2 == 1)
					aspd_rate += 25;
				else if (sc_data[SC_JOINTBEAT].val2 == 2)
					aspd_rate += 10;

		if(sc_data[SC_STAR_COMFORT].timer!=-1 && bl->m == ((struct map_session_data *)bl)->feel_map[2].m) //SG skill [Komurka]
			aspd_rate -= (status_get_lv(bl) + status_get_dex(bl) + status_get_luk(bl))/10;
			}
		}

	return aspd_rate;
}

int status_calc_maxhp(struct block_list *bl, int maxhp)
{
	struct status_change *sc_data;
	sc_data = status_get_sc_data(bl);

		if(sc_data) {
			if(sc_data[SC_INCMHPRATE].timer!=-1)
				maxhp += maxhp * sc_data[SC_INCMHPRATE].val1/100;
	 		if(sc_data[SC_APPLEIDUN].timer!=-1)
				maxhp += maxhp * sc_data[SC_APPLEIDUN].val2/100;
			if(sc_data[SC_DELUGE].timer!=-1 && status_get_elem_type(bl)==1)
				maxhp += maxhp * deluge_eff[sc_data[SC_DELUGE].val1-1]/100;
			if(sc_data[SC_BERSERK].timer!=-1)
				maxhp += maxhp * 2;
		}

	return maxhp;
}

int status_calc_maxsp(struct block_list *bl, int maxsp)
{
	struct status_change *sc_data;
	sc_data = status_get_sc_data(bl);

		if(sc_data) {
			if(sc_data[SC_INCMSPRATE].timer!=-1)
				maxsp += maxsp * sc_data[SC_INCMSPRATE].val1/100;
			if(sc_data[SC_SERVICE4U].timer!=-1)
				maxsp += maxsp * sc_data[SC_SERVICE4U].val2/100;
		}

	return maxsp;
}

/*==========================================
 * For quick calculating [Celest] Adapted by [Skotlex]
 *------------------------------------------
 */
int status_quick_recalc_speed(struct map_session_data *sd, int skill_num, int skill_lv, char start)
{
	/*	[Skotlex]
	This function individually changes a character's speed upon a skill change and restores it upon it's ending.
	Should only be used on non-inclusive skills to avoid exploits.
	Currently used for freedom of cast
	and when cloaking changes it's val3 (in which case the new val3 value comes in the level.
	*/
	
	int b_speed;
	
	b_speed = sd->speed;
	
	switch (skill_num)
	{
		case SA_FREECAST:
			if (start)
			{
				sd->prev_speed = sd->speed;
				sd->speed = sd->speed*(175 - skill_lv*5)/100;
			}
			else
				sd->speed = sd->prev_speed;
			break;
		case AS_CLOAKING:
			if (start && sd->sc_data[SC_CLOAKING].timer != -1)
			{	//There shouldn't be an "stop" case here.
				//If the previous upgrade was 
				//SPEED_ADD_RATE(3*sd->sc_data[SC_CLOAKING].val1 -sd->sc_data[SC_CLOAKING].val3);
				//Then just changing val3 should be a net difference of....
				if (3*sd->sc_data[SC_CLOAKING].val1 != sd->sc_data[SC_CLOAKING].val3)	//This reverts the previous value.
					sd->speed = sd->speed * 100 /(sd->sc_data[SC_CLOAKING].val3-3*sd->sc_data[SC_CLOAKING].val1);
				sd->sc_data[SC_CLOAKING].val3 = skill_lv;
					sd->speed = sd->speed * (sd->sc_data[SC_CLOAKING].val3-sd->sc_data[SC_CLOAKING].val1*3) /100;
			}
			break;
	}

	if(sd->speed < battle_config.max_walk_speed)
		sd->speed = battle_config.max_walk_speed;

	if(b_speed != sd->speed)
		clif_updatestatus(sd,SP_SPEED);

	return 0;
}

/*==========================================
 * ëŒè€ÇÃClassÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_class(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->class_;
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->status.class_;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->class_;
	return 0;
}
/*==========================================
 * ëŒè€ÇÃï˚å¸Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_dir(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->dir;
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->dir;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->dir;
	return 0;
}
/*==========================================
 * ëŒè€ÇÃÉåÉxÉãÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_lv(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->level;
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->status.base_level;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->msd->pet.level;
	return 0;
}

/*==========================================
 * ëŒè€ÇÃéÀíˆÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_range(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->db->range;
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->attackrange;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->range;
	return 0;
}
/*==========================================
 * ëŒè€ÇÃHPÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_hp(struct block_list *bl)
{
	nullpo_retr(1, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->hp;
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->status.hp;
	return 1;
}
/*==========================================
 * ëŒè€ÇÃMHPÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_max_hp(struct block_list *bl)
{
	nullpo_retr(1, bl);

	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->status.max_hp;
	else {
		int max_hp = 1;

		if(bl->type == BL_MOB) {
			struct mob_data *md;
			nullpo_retr(1, md = (struct mob_data *)bl);
			max_hp = md->max_hp;

			if(battle_config.mobs_level_up) // mobs leveling up increase [Valaris]
				max_hp += (md->level - md->db->lv) * status_get_vit(bl);

		}
		else if(bl->type == BL_PET) {
			struct pet_data *pd;
			nullpo_retr(1, pd = (struct pet_data*)bl);
			max_hp = pd->db->max_hp;
		}

		max_hp = status_calc_maxhp(bl,max_hp);
		if(max_hp < 1) max_hp = 1;
		return max_hp;
	}
}
/*==========================================
 * ëŒè€ÇÃStrÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_str(struct block_list *bl)
{
	int str = 0;
	nullpo_retr(0, bl);

	if (bl->type == BL_PC)
		return ((struct map_session_data *)bl)->paramc[0];
	else {
		if(bl->type == BL_MOB) {
			str = ((struct mob_data *)bl)->db->str;
			if(battle_config.mobs_level_up) // mobs leveling up increase [Valaris]
				str += ((struct mob_data *)bl)->level - ((struct mob_data *)bl)->db->lv;
			if(((struct mob_data*)bl)->special_state.size==1) // change for sized monsters [Valaris]
				str/=2;
			else if(((struct mob_data*)bl)->special_state.size==2)
				str*=2;
		} else if(bl->type == BL_PET){	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				str = ((struct pet_data *)bl)->status->str;
			else
				str = ((struct pet_data *)bl)->db->str;
		}

		str = status_calc_str(bl,str);
	}
	if(str < 0) str = 0;
	return str;
}
/*==========================================
 * ëŒè€ÇÃAgiÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */

int status_get_agi(struct block_list *bl)
{
	int agi=0;
	nullpo_retr(0, bl);

	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->paramc[1];
	else {
		if(bl->type == BL_MOB) {
			agi = ((struct mob_data *)bl)->db->agi;
			if(battle_config.mobs_level_up) // increase of mobs leveling up [Valaris]
				agi += ((struct mob_data *)bl)->level - ((struct mob_data *)bl)->db->lv;
			if(((struct mob_data*)bl)->special_state.size==1) // change for sized monsters [Valaris]
				agi/=2;
			else if(((struct mob_data*)bl)->special_state.size==2)
				agi*=2;
		} else if(bl->type == BL_PET) {	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				agi = ((struct pet_data *)bl)->status->agi;
			else
				agi = ((struct pet_data *)bl)->db->agi;
		}

		agi = status_calc_agi(bl,agi);
	}
	if(agi < 0) agi = 0;
	return agi;
}
/*==========================================
 * ëŒè€ÇÃVitÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_vit(struct block_list *bl)
{
	int vit = 0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->paramc[2];
	else {
		if(bl->type == BL_MOB) {
			vit = ((struct mob_data *)bl)->db->vit;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				vit += ((struct mob_data *)bl)->level - ((struct mob_data *)bl)->db->lv;
			if(((struct mob_data*)bl)->special_state.size==1) // change for sizes monsters [Valaris]
				vit/=2;
			else if(((struct mob_data*)bl)->special_state.size==2)
				vit*=2;
		} else if(bl->type == BL_PET) {	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				vit = ((struct pet_data *)bl)->status->vit;
			else
				vit = ((struct pet_data *)bl)->db->vit;
		}

		vit = status_calc_vit(bl,vit);
	}
	if(vit < 0) vit = 0;
	return vit;
}
/*==========================================
 * ëŒè€ÇÃIntÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_int(struct block_list *bl)
{
	int int_=0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->paramc[3];
	else {
		if(bl->type == BL_MOB) {
			int_ = ((struct mob_data *)bl)->db->int_;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				int_ += ((struct mob_data *)bl)->level - ((struct mob_data *)bl)->db->lv;
			if(((struct mob_data*)bl)->special_state.size==1) // change for sized monsters [Valaris]
				int_/=2;
			else if(((struct mob_data*)bl)->special_state.size==2)
				int_*=2;
		} else if(bl->type == BL_PET) {	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				int_ = ((struct pet_data *)bl)->status->int_;
			else
				int_ = ((struct pet_data *)bl)->db->int_;
		}

		int_ = status_calc_int(bl,int_);
	}
	if(int_ < 0) int_ = 0;
	return int_;
}
/*==========================================
 * ëŒè€ÇÃDexÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_dex(struct block_list *bl)
{
	int dex = 0;
	nullpo_retr(0, bl);

	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->paramc[4];
	else {
		if(bl->type == BL_MOB) {
			dex = ((struct mob_data *)bl)->db->dex;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				dex += ((struct mob_data *)bl)->level - ((struct mob_data *)bl)->db->lv;
			if(((struct mob_data*)bl)->special_state.size==1) // change for sized monsters [Valaris]
				dex/=2;
			else if(((struct mob_data*)bl)->special_state.size==2)
				dex*=2;
		} else if(bl->type == BL_PET) {	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				dex = ((struct pet_data *)bl)->status->dex;
			else
				dex = ((struct pet_data *)bl)->db->dex;
		}

		dex = status_calc_dex(bl,dex);
	}
	if(dex < 0) dex = 0;
	return dex;
}
/*==========================================
 * ëŒè€ÇÃLukÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_luk(struct block_list *bl)
{
	int luk = 0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->paramc[5];
	else {
		if(bl->type == BL_MOB) {
			luk = ((struct mob_data *)bl)->db->luk;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				luk += ((struct mob_data *)bl)->level - ((struct mob_data *)bl)->db->lv;
			if(((struct mob_data*)bl)->special_state.size==1) // change for sized monsters [Valaris]
				luk/=2;
			else if(((struct mob_data*)bl)->special_state.size==2)
				luk*=2;
		} else if(bl->type == BL_PET) {	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				luk = ((struct pet_data *)bl)->status->luk;
			else
				luk = ((struct pet_data *)bl)->db->luk;
		}

		luk = status_calc_luk(bl,luk);
	}
	if(luk < 0) luk = 0;
	return luk;
}

/*==========================================
 * ëŒè€ÇÃFleeÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈1à»è„
 *------------------------------------------
 */
int status_get_flee(struct block_list *bl)
{
	int flee = 1;
	nullpo_retr(1, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->flee;
	
	flee = status_calc_flee(bl,status_get_agi(bl)+status_get_lv(bl));
	if(flee < 1) flee = 1;
	return flee;
}
/*==========================================
 * ëŒè€ÇÃHitÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈1à»è„
 *------------------------------------------
 */
int status_get_hit(struct block_list *bl)
{
	int hit = 1;
	nullpo_retr(1, bl);
	if (bl->type == BL_PC)
		return ((struct map_session_data *)bl)->hit;
	
	hit = status_calc_hit(bl,status_get_dex(bl)+status_get_lv(bl));
	if(hit < 1) hit = 1;
	return hit;
}
/*==========================================
 * ëŒè€ÇÃäÆëSâÒîÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈1à»è„
 *------------------------------------------
 */
int status_get_flee2(struct block_list *bl)
{
	int flee2 = 1;
	nullpo_retr(1, bl);

	if (bl->type == BL_PC) 
		return ((struct map_session_data *)bl)->flee2;

	flee2 = status_calc_flee2(bl,status_get_luk(bl)+10);
	if (flee2 < 1) flee2 = 1;
	return flee2;
}
/*==========================================
 * ëŒè€ÇÃÉNÉäÉeÉBÉJÉãÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈1à»è„
 *------------------------------------------
 */
int status_get_critical(struct block_list *bl)
{
	int critical = 1;
	nullpo_retr(1, bl);

	if (bl->type == BL_PC)
		return ((struct map_session_data *)bl)->critical;

	critical = status_get_luk(bl)*3+10;
	if(battle_config.enemy_critical_rate != 100)
		critical = critical*battle_config.enemy_critical_rate/100;
	critical = status_calc_critical(bl,critical);
	if (critical < 1) critical = 1;
	return critical;
}
/*==========================================
 * base_atkÇÃéÊìæ
 * ñﬂÇËÇÕêÆêîÇ≈1à»è„
 *------------------------------------------
 */
int status_get_batk(struct block_list *bl)
{
	int batk = 1;
	nullpo_retr(1, bl);
	
	if(bl->type==BL_PC) {
		batk = ((struct map_session_data *)bl)->base_atk;
		if (((struct map_session_data *)bl)->status.weapon < 16)
			batk += ((struct map_session_data *)bl)->weapon_atk[((struct map_session_data *)bl)->status.weapon];
	} else {
		int str,dstr;
		str = status_get_str(bl); //STR
		dstr = str/10;
		batk = dstr*dstr + str; //base_atkÇåvéZÇ∑ÇÈ

		if(bl->type == BL_MOB && ((struct mob_data *)bl)->guardian_data)
			batk += batk * 10*((struct mob_data *)bl)->guardian_data->guardup_lv/100; // Strengthen Guardians - custom value +10% ATK / lv

		batk = status_calc_batk(bl,batk);
	}
	if(batk < 1) batk = 1; //base_atkÇÕç≈í·Ç≈Ç‡1
	return batk;
}
/*==========================================
 * ëŒè€ÇÃAtkÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_atk(struct block_list *bl)
{
	int atk=0;
	nullpo_retr(0, bl);
	switch (bl->type) {
		case BL_PC:
			return ((struct map_session_data*)bl)->right_weapon.watk;
		case BL_MOB:
			atk = ((struct mob_data*)bl)->db->atk1;
			if(((struct mob_data *)bl)->guardian_data)
				atk += atk * 10*((struct mob_data *)bl)->guardian_data->guardup_lv/100; // Strengthen Guardians - custom value +10% ATK / lv
		break;
		case BL_PET:	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				atk = ((struct pet_data *)bl)->status->atk1;
			else
				atk = ((struct pet_data*)bl)->db->atk1;
		break;
	}
	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	atk = status_calc_watk(bl,atk);
	if(atk < 0) atk = 0;
	return atk;
}
/*==========================================
 * ëŒè€ÇÃç∂éËAtkÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_atk_(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC){
		return ((struct map_session_data*)bl)->left_weapon.watk;
	}
	return 0;
}
/*==========================================
 * ëŒè€ÇÃAtk2Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_atk2(struct block_list *bl)
{
	int atk2=0;
	nullpo_retr(0, bl);

	switch (bl->type) {
		case BL_PC:
			return ((struct map_session_data*)bl)->right_weapon.watk2;
		case BL_MOB:
			atk2 = ((struct mob_data*)bl)->db->atk2;
			
			if(((struct mob_data *)bl)->guardian_data)
				atk2 += atk2 * 10*((struct mob_data *)bl)->guardian_data->guardup_lv/100; // Strengthen Guardians - custom value +10% ATK / lv
		break;
		case BL_PET:	//<Skotlex> Use pet's stats
			if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
				atk2 = ((struct pet_data *)bl)->status->atk2;
			else
				atk2 = ((struct pet_data*)bl)->db->atk2;
		break;
	}		  

	// Absolute, then relative modifiers from status changes (shared between PC and NPC)
	atk2 = status_calc_watk(bl,atk2);

	if(atk2 < 0) atk2 = 0;
		return atk2;
}
/*==========================================
 * ëŒè€ÇÃç∂éËAtk2Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_atk_2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC)
		return ((struct map_session_data*)bl)->left_weapon.watk2;
	return 0;
}
/*==========================================
 * ëŒè€ÇÃMAtk1Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_matk1(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->matk1;
	else {
		int matk = 0;
		int int_ = status_get_int(bl);
		matk = status_calc_matk(bl,int_+(int_/5)*(int_/5));
 		return matk;
	}
}
/*==========================================
 * ëŒè€ÇÃMAtk2Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_matk2(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->matk2;
	else {
        int matk = 0;
		int int_ = status_get_int(bl);
		matk = status_calc_matk(bl,int_+(int_/7)*(int_/7));
		return matk;
	}
}
/*==========================================
 * ëŒè€ÇÃDefÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_def(struct block_list *bl)
{
	int def=0;
	nullpo_retr(0, bl);

	if(bl->type==BL_PC){
		def = ((struct map_session_data *)bl)->def;
		if(((struct map_session_data *)bl)->skilltimer != -1)
			def -= def * skill_get_castdef(((struct map_session_data *)bl)->skillid)/100;
	} else if(bl->type==BL_MOB) {
		def = ((struct mob_data *)bl)->db->def;
		def -= def * skill_get_castdef(((struct mob_data *)bl)->skillid)/100;
	} else if(bl->type==BL_PET)
		def = ((struct pet_data *)bl)->db->def;

	def = status_calc_def(bl,def);
	if(def < 0) def = 0;

	return def;
}
/*==========================================
 * ëŒè€ÇÃDef2Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈1à»è„
 *------------------------------------------
 */
int status_get_def2(struct block_list *bl)
{
	int def2 = 1;
	nullpo_retr(1, bl);
	
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->def2;
	else if(bl->type==BL_MOB)
		def2 = ((struct mob_data *)bl)->db->vit;
	else if(bl->type==BL_PET) {	//<Skotlex> Use pet's stats
		if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
			def2 = ((struct pet_data *)bl)->status->vit;
		else
			def2 = ((struct pet_data *)bl)->db->vit;
	}

	def2 = status_calc_def2(bl,def2);
	if(def2 < 1) def2 = 1;
	
	return def2;
}
/*==========================================
 * ëŒè€ÇÃMDefÇï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_mdef(struct block_list *bl)
{
	int mdef=0;
	nullpo_retr(0, bl);

	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->mdef;
	else if(bl->type==BL_MOB)
		mdef = ((struct mob_data *)bl)->db->mdef;
	else if(bl->type==BL_PET)
		mdef = ((struct pet_data *)bl)->db->mdef;

	mdef = status_calc_mdef(bl,mdef);
	if(mdef < 0) mdef = 0;

 	return mdef;
}
/*==========================================
 * ëŒè€ÇÃMDef2Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈0à»è„
 *------------------------------------------
 */
int status_get_mdef2(struct block_list *bl)
{
	int mdef2=0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->mdef2 + (((struct map_session_data *)bl)->paramc[2]>>1);
	else if(bl->type == BL_MOB)
		mdef2 = ((struct mob_data *)bl)->db->int_ + (((struct mob_data *)bl)->db->vit>>1);
	else if(bl->type == BL_PET) {	//<Skotlex> Use pet's stats
		if (battle_config.pet_lv_rate && ((struct pet_data *)bl)->status)
			mdef2 = ((struct pet_data *)bl)->status->int_ +(((struct pet_data *)bl)->status->vit>>1);
		else
			mdef2 = ((struct pet_data *)bl)->db->int_ + (((struct pet_data *)bl)->db->vit>>1);
	}

	mdef2 = status_calc_mdef2(bl,mdef2);
	if(mdef2 < 0) mdef2 = 0;
	
	return mdef2;
}
/*==========================================
 * ëŒè€ÇÃSpeed(à⁄ìÆë¨ìx)Çï‘Ç∑(îƒóp)
 * ñﬂÇËÇÕêÆêîÇ≈1à»è„
 * SpeedÇÕè¨Ç≥Ç¢ÇŸÇ§Ç™à⁄ìÆë¨ìxÇ™ë¨Ç¢
 *------------------------------------------
 */
int status_get_speed(struct block_list *bl)
{
	int speed = 1000;
	nullpo_retr(1000, bl);
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->speed;
	else if(bl->type==BL_MOB) {
		speed = ((struct mob_data *)bl)->speed;
		if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
			speed-=((struct mob_data *)bl)->level - ((struct mob_data *)bl)->db->lv;
	}
	else if(bl->type==BL_PET)
		speed = ((struct pet_data *)bl)->msd->petDB->speed;
	else if(bl->type==BL_NPC)	//Added BL_NPC (Skotlex)
		speed = ((struct npc_data *)bl)->speed;

	speed = status_calc_speed(bl,speed);

	if(speed < 1) speed = 1;
	return speed;
}
/*==========================================
 * ëŒè€ÇÃaDelay(çUåÇéûÉfÉBÉåÉC)Çï‘Ç∑(îƒóp)
 * aDelayÇÕè¨Ç≥Ç¢ÇŸÇ§Ç™çUåÇë¨ìxÇ™ë¨Ç¢
 *------------------------------------------
 */
int status_get_adelay(struct block_list *bl)
{
	int adelay,aspd_rate;
	nullpo_retr(4000, bl);
	switch (bl->type) {
		case BL_PC:
			return (((struct map_session_data *)bl)->aspd<<1);
		case BL_MOB:
			adelay = ((struct mob_data *)bl)->db->adelay;
			if(((struct mob_data *)bl)->guardian_data)
				aspd_rate = 100 - 10*((struct mob_data *)bl)->guardian_data->guardup_lv; // Strengthen Guardians - custom value +10% ASPD / lv
			else
				aspd_rate = 100;
			break;
		case BL_PET:
			adelay = ((struct pet_data *)bl)->db->adelay;
			aspd_rate = 100;
			break;
		default:
			adelay=4000;
			aspd_rate = 100;
			break;
	}
	aspd_rate = status_calc_aspd_rate(bl,aspd_rate);

	if(aspd_rate != 100)
		adelay = adelay*aspd_rate/100;
	if(adelay < battle_config.monster_max_aspd<<1) adelay = battle_config.monster_max_aspd<<1;
		return adelay;
}
int status_get_amotion(struct block_list *bl)
{
	nullpo_retr(2000, bl);
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->amotion;
	else {
		int amotion=2000,aspd_rate = 100;
		if(bl->type==BL_MOB) {
			amotion = ((struct mob_data *)bl)->db->amotion;

			if(((struct mob_data *)bl)->guardian_data)
				aspd_rate -= aspd_rate * 10*((struct mob_data *)bl)->guardian_data->guardup_lv/100; // Strengthen Guardians - custom value +10% ASPD / lv
		} else if(bl->type==BL_PET)
			amotion = ((struct pet_data *)bl)->db->amotion;

		aspd_rate = status_calc_aspd_rate(bl,aspd_rate);

		if(aspd_rate != 100)
			amotion = amotion*aspd_rate/100;
		if(amotion < battle_config.monster_max_aspd) amotion = battle_config.monster_max_aspd;
		return amotion;
	}
	return 2000;
}
int status_get_dmotion(struct block_list *bl)
{
	int ret;
	struct status_change *sc_data;

	nullpo_retr(0, bl);
	sc_data = status_get_sc_data(bl);
	if(bl->type==BL_MOB){
		ret=((struct mob_data *)bl)->db->dmotion;
		if(battle_config.monster_damage_delay_rate != 100)
			ret = ret*battle_config.monster_damage_delay_rate/100;
	}
	else if(bl->type==BL_PC){
		ret=((struct map_session_data *)bl)->dmotion;
		if(battle_config.pc_damage_delay_rate != 100)
			ret = ret*battle_config.pc_damage_delay_rate/100;
	}
	else if(bl->type==BL_PET)
		ret=((struct pet_data *)bl)->db->dmotion;
	else
		return 2000;

	if(sc_data && (sc_data[SC_ENDURE].timer!=-1 || sc_data[SC_CONCENTRATION].timer!=-1 || sc_data[SC_BERSERK].timer!=-1))
		if (!map_flag_gvg(bl->m)) //Only works on non-gvg grounds. [Skotlex]
			return 0;

	return ret;
}
int status_get_element(struct block_list *bl)
{
	// removed redundant variable ret [zzo]
	struct status_change *sc_data = status_get_sc_data(bl);

	nullpo_retr(20, bl);

	if(sc_data) {
		if( sc_data[SC_BENEDICTIO].timer!=-1 )	// êπëÃç~ïü
			return 26;
		if( sc_data[SC_FREEZE].timer!=-1 )	// ìÄåã
			return 21;
		if( sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2==0)
			return 22;
	}
	if(bl->type==BL_MOB)	// 10ÇÃà ÅÅLv*2ÅAÇPÇÃà ÅÅëÆê´
		return ((struct mob_data *)bl)->def_ele;
	if(bl->type==BL_PC)
		return 20+((struct map_session_data *)bl)->def_ele;	// ñhå‰ëÆê´Lv1
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->element;

	return 20;
}
//Retrieves the object's element acquired by status changes only.
int status_get_attack_sc_element(struct block_list *bl)
{
	struct status_change *sc_data=status_get_sc_data(bl);
	if(sc_data) {
		if( sc_data[SC_WATERWEAPON].timer!=-1)	// ÉtÉçÉXÉgÉEÉFÉ|Éì
			return 1;
		if( sc_data[SC_EARTHWEAPON].timer!=-1)	// ÉTÉCÉYÉ~ÉbÉNÉEÉFÉ|Éì
			return 2;
		if( sc_data[SC_FIREWEAPON].timer!=-1)	// ÉtÉåÅ[ÉÄÉâÉìÉ`ÉÉÅ[
			return 3;
		if( sc_data[SC_WINDWEAPON].timer!=-1)	// ÉâÉCÉgÉjÉìÉOÉçÅ[É_Å[
			return 4;
		if( sc_data[SC_ENCPOISON].timer!=-1)	// ÉGÉìÉ`ÉÉÉìÉgÉ|ÉCÉYÉì
			return 5;
		if( sc_data[SC_ASPERSIO].timer!=-1)		// ÉAÉXÉyÉãÉVÉI
			return 6;
		if( sc_data[SC_SHADOWWEAPON].timer!=-1)
			return 7;
		if( sc_data[SC_GHOSTWEAPON].timer!=-1)
			return 8;
	}
	return 0;
}


int status_get_attack_element(struct block_list *bl)
{
	int ret = status_get_attack_sc_element(bl);

	nullpo_retr(0, bl);
	
	if (ret)	return ret;
	
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return 0;
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->right_weapon.atk_ele;
	if(bl->type==BL_PET && (struct pet_data *)bl)
		return 0;

	return 0;
}
int status_get_attack_element2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC) {
		// removed redundant var, speeded up a bit [zzo]
		int ret = status_get_attack_sc_element(bl);

		if(ret) return ret;
		return ((struct map_session_data *)bl)->left_weapon.atk_ele;
	}
	return 0;
}
int status_get_party_id(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->status.party_id;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->msd->status.party_id;
	if(bl->type==BL_MOB){
		struct mob_data *md=(struct mob_data *)bl;
		if( md->master_id>0 )
		{
			struct map_session_data *msd;
			if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
				return msd->status.party_id;
			return -md->master_id;
		}
		return 0; //No party.
	}
	if(bl->type==BL_SKILL)
		return ((struct skill_unit *)bl)->group->party_id;
	return 0;
}

int status_get_guild_id(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->status.guild_id;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->msd->status.guild_id;
	if(bl->type==BL_MOB)
	{
		struct map_session_data *msd;
		struct mob_data *md = (struct mob_data *)bl;
		if (md->guardian_data)	//Guardian's guild [Skotlex]
			return md->guardian_data->guild_id;
		if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
			return msd->status.guild_id; //Alchemist's mobs [Skotlex]
		return 0; //No guild.
	}
	if(bl->type==BL_SKILL)
		return ((struct skill_unit *)bl)->group->guild_id;
	return 0;
}
int status_get_race(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->db->race;
	if(bl->type==BL_PC)
		return 7;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->race;
	return 0;
}
int status_get_size(struct block_list *bl)
{
	nullpo_retr(1, bl);
	switch (bl->type) {
		case BL_MOB:
			if (((struct mob_data *)bl)->sc_data[SC_SWOO].timer != -1) // [marquis007]
				return 0;
			return ((struct mob_data *)bl)->db->size;
		case BL_PET:	
			return ((struct pet_data *)bl)->db->size;
		case BL_PC:
		{
			struct map_session_data *sd = (struct map_session_data *)bl;
			if (sd->sc_data[SC_SWOO].timer != -1)
				return 0;
			if (sd->class_&JOBL_BABY) //[Lupus]
				return (pc_isriding(sd) && battle_config.character_size&2); //Baby Class Peco Rider + enabled option -> size = 1, else 0
			return 1+(pc_isriding(sd) && battle_config.character_size&1);	//Peco Rider + enabled option -> size = 2, else 1
		}
	}
	return 1;
}
int status_get_mode(struct block_list *bl)
{
	nullpo_retr(MD_CANMOVE, bl);
	if(bl->type==BL_MOB)
	{
		if (((struct mob_data *)bl)->mode)
			return ((struct mob_data *)bl)->mode;
		return ((struct mob_data *)bl)->db->mode;
	}
	if(bl->type==BL_PC)
		return (MD_CANMOVE|MD_LOOTER|MD_CANATTACK);
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->mode;
	if (bl->type==BL_SKILL)
		return (MD_CANATTACK|MD_CANMOVE);	//Default mode for skills: Can attack, can move (think dances).
	//Default universal mode, can move
	return MD_CANMOVE;	// Ç∆ÇËÇ†Ç¶Ç∏ìÆÇ≠Ç∆Ç¢Ç§Ç±Ç∆Ç≈1
}

int status_get_mexp(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->db->mexp;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->mexp;
	return 0;
}
int status_get_race2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type == BL_MOB)
		return ((struct mob_data *)bl)->db->race2;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->race2;
	return 0;
}
int status_isdead(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type == BL_MOB)
		return ((struct mob_data *)bl)->state.state == MS_DEAD;
	if(bl->type==BL_PC)
		return pc_isdead((struct map_session_data *)bl);
	return 0;
}
int status_isimmune(struct block_list *bl)
{
	struct map_session_data *sd = (struct map_session_data *)bl;
	
	nullpo_retr(0, bl);
	if (bl->type == BL_PC) {
		if (sd->special_state.no_magic_damage)
			return 1;
		if (sd->sc_count && sd->sc_data[SC_HERMODE].timer != -1)
			return 1;
	}	
	return 0;
}

// StatusChangeånÇÃèäìæ
struct status_change *status_get_sc_data(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data*)bl)->sc_data;
	if(bl->type==BL_PC)
		return ((struct map_session_data*)bl)->sc_data;
	return NULL;
}
short *status_get_sc_count(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	if(bl->type==BL_MOB)
		return &((struct mob_data*)bl)->sc_count;
	if(bl->type==BL_PC)
		return &((struct map_session_data*)bl)->sc_count;
	return NULL;
}
short *status_get_opt1(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return &((struct mob_data*)bl)->opt1;
	if(bl->type==BL_PC)
		return &((struct map_session_data*)bl)->opt1;
	if(bl->type==BL_NPC)
		return &((struct npc_data*)bl)->opt1;
	return 0;
}
short *status_get_opt2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return &((struct mob_data*)bl)->opt2;
	if(bl->type==BL_PC)
		return &((struct map_session_data*)bl)->opt2;
	if(bl->type==BL_NPC)
		return &((struct npc_data*)bl)->opt2;
	return 0;
}
short *status_get_opt3(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return &((struct mob_data*)bl)->opt3;
	if(bl->type==BL_PC)
		return &((struct map_session_data*)bl)->opt3;
	if(bl->type==BL_NPC)
		return &((struct npc_data*)bl)->opt3;
	return 0;
}
short *status_get_option(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB)
		return &((struct mob_data*)bl)->option;
	if(bl->type==BL_PC)
		return &((struct map_session_data*)bl)->status.option;
	if(bl->type==BL_NPC)
		return &((struct npc_data*)bl)->option;
	return 0;
}

int status_get_sc_def(struct block_list *bl, int type)
{
	int sc_def;
	nullpo_retr(0, bl);
	
	switch (type)
	{
	case SP_MDEF1:	// mdef
		sc_def = 100 - (3 + status_get_mdef(bl) + status_get_luk(bl)/3);
		break;
	case SP_MDEF2:	// int
		sc_def = 100 - (3 + status_get_int(bl) + status_get_luk(bl)/3);
		break;
	case SP_DEF1:	// def
		sc_def = 100 - (3 + status_get_def(bl) + status_get_luk(bl)/3);
		break;
	case SP_DEF2:	// vit
		sc_def = 100 - (3 + status_get_vit(bl) + status_get_luk(bl)/3);
		break;
	case SP_LUK:	// luck
		sc_def = 100 - (3 + status_get_luk(bl));
		break;

	case SC_STONE:
	case SC_FREEZE:
		sc_def = 100 - (3 + status_get_mdef(bl) + status_get_luk(bl)/3);
		break;
	case SC_STAN:
	case SC_POISON:
	case SC_SILENCE:
		sc_def = 100 - (3 + status_get_vit(bl) + status_get_luk(bl)/3);
		break;	
	case SC_SLEEP:
	case SC_CONFUSION:
		sc_def = 100 - (3 + status_get_int(bl) + status_get_luk(bl)/3);
		break;
	case SC_BLIND:
		sc_def = 100 - (3 + status_get_int(bl) + status_get_vit(bl)/3);
		break;
	case SC_CURSE:
		sc_def = 100 - (3 + status_get_luk(bl) + status_get_vit(bl)/3);
		break;	

	default:
		sc_def = 100;
		break;
	}

	if(bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)bl;
		if (md->class_ == MOBID_EMPERIUM)
			return 0;
		if (sc_def < 50)
			sc_def = 50;
	} else if(bl->type == BL_PC) {
		struct status_change* sc_data = status_get_sc_data(bl);
		if (sc_data)
		{
			if (sc_data[SC_SCRESIST].timer != -1)
				sc_def -= sc_data[SC_SCRESIST].val1; //Status resist
			else if (sc_data[SC_SIEGFRIED].timer != -1)
				sc_def -= sc_data[SC_SIEGFRIED].val2; //Status resistance.
		}
	}
	return (sc_def < 0) ? 0 : sc_def;
}

/*==========================================
 * Starts a status change.
 * type = type, val1~4 depend on the type.
 * Tick is base duration
 * flag:
 * &1: Cannot be avoided (it has to start)
 * &2: Tick should not be reduced (by vit, luk, lv, etc)
 * &4: sc_data loaded, no value has to be altered.
 *------------------------------------------
 */
int status_change_start(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag)
{
	struct map_session_data *sd = NULL;
	struct status_change* sc_data;
	short *sc_count, *option, *opt1, *opt2, *opt3;
	int opt_flag = 0, calc_flag = 0,updateflag = 0, save_flag = 0, race, mode, elem, undead_flag;
	int scdef = 0;

	nullpo_retr(0, bl);
	switch (bl->type)
	{
		case BL_PC:
			sd=(struct map_session_data *)bl;
			if (status_isdead(bl))
				return 0;
			break;
		case BL_MOB:
			if (((struct mob_data*)bl)->class_ == MOBID_EMPERIUM && type != SC_SAFETYWALL)
				return 0; //Emperium can't be afflicted by status changes.
			if (status_isdead(bl))
				return 0;
			break;
		case BL_PET: //Because pets can't have status changes.
		case BL_SKILL: //These may happen by attacking traps or the like. [Skotlex]
			return 0;
		default:
			if(battle_config.error_log)
				ShowError("status_change_start: invalid source type (%d)!\n", bl->type);
			return 0;
	}
	if(type < 0 || type >= SC_MAX) {
		if(battle_config.error_log)
			ShowError("status_change_start: invalid status change (%d)!\n", type);
		return 0;
	}
	sc_data=status_get_sc_data(bl);
	sc_count=status_get_sc_count(bl);
	option=status_get_option(bl);
	opt1=status_get_opt1(bl);
	opt2=status_get_opt2(bl);
	opt3=status_get_opt3(bl);

	race=status_get_race(bl);
	mode=status_get_mode(bl);
	elem=status_get_elem_type(bl);
	undead_flag=battle_check_undead(race,elem);

	if(type == SC_AETERNA && (sc_data[SC_STONE].timer != -1 || sc_data[SC_FREEZE].timer != -1) )
		return 0;
	if(type == SC_OVERTHRUST && sc_data[SC_MAXOVERTHRUST].timer != -1)
		return 0; //Overthrust can't take effect if under Max Overthrust. [Skotlex]
	switch(type){
		case SC_STONE:
		case SC_FREEZE:
			scdef=3+status_get_mdef(bl)+status_get_luk(bl)/3;
			break;
		case SC_STAN:
		case SC_SILENCE:
		case SC_POISON:
		case SC_DPOISON:
			scdef=3+status_get_vit(bl)+status_get_luk(bl)/3;
			break;
		case SC_SLEEP:
		case SC_BLIND:
			scdef=3+status_get_int(bl)+status_get_luk(bl)/3;
			break;
		case SC_CURSE:
			scdef=3+status_get_luk(bl);
			break;
		default:
			scdef=0;
	}
	if(scdef>=100)
		return 0;
	if(sd){
		if(type == SC_ADRENALINE && !(skill_get_weapontype(BS_ADRENALINE)&(1<<sd->status.weapon)))
			return 0;
		if( sd && type == SC_ADRENALINE2 && !(skill_get_weapontype(BS_ADRENALINE2)&(1<<sd->status.weapon)))
			return 0;

		if(SC_COMMON_MIN<=type && type<=SC_COMMON_MAX && !(flag&1)){
			if(sd->reseff[type-SC_COMMON_MIN] > 0 && rand()%10000<sd->reseff[type-SC_COMMON_MIN]){
				if(battle_config.battle_log)
					ShowInfo("PC %d skill_sc_start: status change %d blocked by reseff card (AID: %d).\n",type,bl->id);
				return 0;
			}
		}
	}

	if((type==SC_FREEZE || type==SC_STONE) && undead_flag && !(flag&1))
	//I've been informed that undead chars are inmune to stone curse too. [Skotlex]
		return 0;


	if (type==SC_BLESSING && (bl->type==BL_PC || (!undead_flag && race!=6))) {
		if (sc_data[SC_CURSE].timer!=-1)
			status_change_end(bl,SC_CURSE,-1);
		if (sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2==0)
			status_change_end(bl,SC_STONE,-1);
	}

	if((type == SC_ADRENALINE || type==SC_ADRENALINE2 || type == SC_WEAPONPERFECTION || type == SC_OVERTHRUST) &&
		sc_data[type].timer != -1 && sc_data[type].val2 && !val2)
		return 0;

	if(mode & MD_BOSS && !(flag&1) && ( (type>=SC_COMMON_MIN && type <= SC_COMMON_MAX)
		|| type==SC_QUAGMIRE || type==SC_DECREASEAGI || type==SC_SIGNUMCRUCIS || type==SC_PROVOKE || type==SC_ROKISWEIL
		|| type==SC_COMA
		|| (type == SC_BLESSING && (undead_flag || race == 6)))){
		/* É{ÉXÇ…ÇÕ?Ç©Ç»Ç¢(ÇΩÇæÇµÉJ?ÉhÇ…ÇÊÇÈ?â ÇÕìKópÇ≥ÇÍÇÈ) */
		return 0;
	}

	if(sc_data[type].timer != -1){	/* Ç∑Ç≈Ç…ìØÇ∂àŸèÌÇ…Ç»Ç¡ÇƒÇ¢ÇÈèÍçáÉ^ÉCÉ}âèú */
		if(sc_data[type].val1 > val1 && type != SC_COMBO && type != SC_DANCING && type != SC_DEVOTION &&
			type != SC_ASPDPOTION0 && type != SC_ASPDPOTION1 && type != SC_ASPDPOTION2 && type != SC_ASPDPOTION3
			&& type != SC_ATKPOTION && type != SC_MATKPOTION // added atk and matk potions [Valaris]
		)
			return 0;

		if ((type >=SC_STAN && type <= SC_BLIND) || type == SC_DPOISON)
			return 0;/* ?Ç¨ë´ÇµÇ™Ç≈Ç´Ç»Ç¢?ë‘àŸèÌÇ≈Ç†ÇÈéûÇÕ?ë‘àŸèÌÇçsÇÌÇ»Ç¢ */

		if (type == SC_GOSPEL && sc_data[type].val4 == BCT_SELF) //Must not override a casting gospel char.
			return 0;
		
		(*sc_count)--;
		delete_timer(sc_data[type].timer, status_change_timer);
		sc_data[type].timer = -1;
	}

	if(type==SC_FREEZE || type==SC_STAN || type==SC_SLEEP || type==SC_STOP || type == SC_CONFUSION ||
		type==SC_CLOSECONFINE || type==SC_CLOSECONFINE2)
		battle_stopwalking(bl,1);

	// ÉNÉAÉOÉ}ÉCÉA/éÑÇñYÇÍÇ»Ç¢Ç≈íÜÇÕñ≥å¯Ç»ÉXÉLÉã
	if ((sc_data[SC_QUAGMIRE].timer!=-1 || sc_data[SC_DONTFORGETME].timer!=-1) &&
		(type==SC_CONCENTRATE || type==SC_INCREASEAGI ||
		type==SC_TWOHANDQUICKEN || type==SC_SPEARSQUICKEN ||
		type==SC_ADRENALINE || type==SC_ADRENALINE2 ||
		type==SC_TRUESIGHT || type==SC_WINDWALK ||
		type==SC_CARTBOOST || type==SC_ASSNCROS || 
		type==SC_ONEHAND))
	return 0;

	switch(type){	/* àŸèÌÇÃéÌóﬁÇ≤Ç∆ÇÃ?óù */
		case SC_PROVOKE:			/* ÉvÉçÉ{ÉbÉN */
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	/* (ÉI?ÉgÉo?ÉT?ÉN) */
			break;
		case SC_ENDURE:				/* ÉCÉìÉfÉÖÉA */
			if(tick <= 0) tick = 1000 * 60;
			calc_flag = 1; // for updating mdef
			val2 = 7; // [Celest]
			break;
		case SC_AUTOBERSERK:
			{
				if (!(flag&4))
					tick = 60*1000;
				if (bl->type == BL_PC && sd->status.hp<sd->status.max_hp>>2 &&
					(sc_data[SC_PROVOKE].timer==-1 || sc_data[SC_PROVOKE].val2==0))
					status_change_start(bl,SC_PROVOKE,10,1,0,0,0,0);
			}
			break;
		
		case SC_INCREASEAGI:		/* ë¨ìxè„è∏ */
			calc_flag = 1;
			if(sc_data[SC_DECREASEAGI].timer!=-1 )
				status_change_end(bl,SC_DECREASEAGI,-1);
			break;
		case SC_DECREASEAGI:		/* ë¨ìxå∏è≠ */
			if (bl->type == BL_PC && !(tick&2))	// Celest
				tick>>=1;
			calc_flag = 1;
			if(sc_data[SC_INCREASEAGI].timer!=-1 )
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_ADRENALINE2].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc_data[SC_CARTBOOST].timer!=-1 )
				status_change_end(bl,SC_CARTBOOST,-1);
			if(sc_data[SC_ONEHAND].timer!=-1 )
				status_change_end(bl,SC_ONEHAND,-1);
			break;
		case SC_SIGNUMCRUCIS:		/* ÉVÉOÉiÉÄÉNÉãÉVÉX */
			calc_flag = 1;
			val2 = 10 + val1*2;
			if (!(flag&4))
				tick = 600*1000;
			clif_emotion(bl,4);
			break;
		case SC_ONEHAND: //Removes the Aspd potion effect, as reported by Vicious. [Skotlex]
			if(sc_data[SC_ASPDPOTION0].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION0,-1);
			if(sc_data[SC_ASPDPOTION1].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION1,-1);
			if(sc_data[SC_ASPDPOTION2].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION2,-1);
			if(sc_data[SC_ASPDPOTION3].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION3,-1);
		case SC_TWOHANDQUICKEN:		/* 2HQ */
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				return 0;
			*opt3 |= 1;
			calc_flag = 1;
			break;
		case SC_ADRENALINE2:
		case SC_ADRENALINE:			/* ÉAÉhÉåÉiÉäÉìÉâÉbÉVÉÖ */
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				return 0;
			if(bl->type == BL_PC && !(flag&2))
				if(pc_checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			calc_flag = 1;
			break;
		case SC_WEAPONPERFECTION:	/* ÉEÉFÉ|ÉìÉp?ÉtÉFÉNÉVÉáÉì */
			if(bl->type == BL_PC && !(flag&2))
				if(pc_checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			break;
		case SC_OVERTHRUST:			/* ÉI?Éo?ÉXÉâÉXÉg */
			if(bl->type == BL_PC && !(flag&2))
				if(pc_checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			*opt3 |= 2;
			break;
		case SC_MAXOVERTHRUST: //Cancels Normal Overthrust. [Skotlex]
			if (sc_data[SC_OVERTHRUST].timer != -1)
				status_change_end(bl, SC_OVERTHRUST, -1);
			break;
		case SC_MAXIMIZEPOWER:		/* É}ÉLÉVÉ}ÉCÉYÉpÉè?(SPÇ™1å∏ÇÈéûä‘,val2Ç…Ç‡) */
			if (!(flag&4))
			{
				if(bl->type != BL_PC)
					tick = 5000;
				val2 = tick;
			}
			break;
		case SC_ENCPOISON:			/* ÉGÉìÉ`ÉÉÉìÉgÉ|ÉCÉYÉì */
			calc_flag = 1;
			val2=(((val1 - 1) / 2) + 3)*100;	/* ì≈ït?ämó¶ */
			skill_enchant_elemental_end(bl,SC_ENCPOISON);
			break;
		case SC_EDP:	// [Celest]
			val2 = val1 + 2;			/* ñ“ì≈ït?ämó¶(%) */
			calc_flag = 1;
			break;
		case SC_POISONREACT:	/* É|ÉCÉYÉìÉäÉAÉNÉg */
			if (!(flag&4))
				val2=val1/2 + val1%2; // [Celest]
			break;
		case SC_ENERGYCOAT:			/* ÉGÉiÉW?ÉR?Ég */
			*opt3 |= 4;
			break;
		case SC_MAGICROD:
			val2 = val1*20;
			break;
		case SC_KYRIE:				/* ÉLÉäÉGÉGÉåÉCÉ\Éì */
			if (!(flag&4))
			{
				val2 = status_get_max_hp(bl) * (val1 * 2 + 10) / 100;/* ëœãvìx */
				val3 = (val1 / 2 + 5);	/* âÒ? */
			}
// -- moonsoul (added to undo assumptio status if target has it)
			if(sc_data[SC_ASSUMPTIO].timer!=-1 )
				status_change_end(bl,SC_ASSUMPTIO,-1);
			break;
		case SC_MINDBREAKER:
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	/* (ÉI?ÉgÉo?ÉT?ÉN) */
		case SC_TRICKDEAD:			/* éÄÇÒÇæÇ”ÇË */
			if (bl->type == BL_PC) {
				pc_stopattack(sd);
			}
			break;
		case SC_QUAGMIRE:			/* ÉNÉ@ÉOÉ}ÉCÉA */
			calc_flag = 1;
			if(sc_data[SC_CONCENTRATE].timer!=-1 )	/* èWíÜóÕå¸è„âèú */
				status_change_end(bl,SC_CONCENTRATE,-1);
			if(sc_data[SC_INCREASEAGI].timer!=-1 )	/* ë¨ìxè„è∏âèú */
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc_data[SC_ONEHAND].timer!=-1 )
				status_change_end(bl,SC_ONEHAND,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_ADRENALINE2].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc_data[SC_TRUESIGHT].timer!=-1 )	/* ÉgÉDÉã?ÉTÉCÉg */
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc_data[SC_WINDWALK].timer!=-1 )	/* ÉEÉCÉìÉhÉEÉH?ÉN */
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc_data[SC_CARTBOOST].timer!=-1 )	/* ÉJ?ÉgÉu?ÉXÉg */
				status_change_end(bl,SC_CARTBOOST,-1);
			break;
		case SC_MAGICPOWER:
			calc_flag = 1;
			val2 = 1;
			break;
		case SC_SACRIFICE:
			if (!(flag&4))
				val2 = 5;
			break;
		case SC_ASPERSIO:			/* ÉAÉXÉyÉãÉVÉI */
			skill_enchant_elemental_end(bl,SC_ASPERSIO);
			break;
		case SC_FIREWEAPON:		/* ÉtÉå?ÉÄÉâÉìÉ`ÉÉ? */
			skill_enchant_elemental_end(bl,SC_FIREWEAPON);
			break;
		case SC_WATERWEAPON:		/* ÉtÉçÉXÉgÉEÉFÉ|Éì */
			skill_enchant_elemental_end(bl,SC_WATERWEAPON);
			break;
		case SC_WINDWEAPON:	/* ÉâÉCÉgÉjÉìÉOÉç?É_? */
			skill_enchant_elemental_end(bl,SC_WINDWEAPON);
			break;
		case SC_EARTHWEAPON:		/* ÉTÉCÉYÉ~ÉbÉNÉEÉFÉ|Éì */
			skill_enchant_elemental_end(bl,SC_EARTHWEAPON);
			break;
		case SC_SHADOWWEAPON:
			skill_enchant_elemental_end(bl,SC_SHADOWWEAPON);
			break;
		case SC_GHOSTWEAPON:
			skill_enchant_elemental_end(bl,SC_GHOSTWEAPON);
			break;
		case SC_PROVIDENCE:			/* ÉvÉçÉîÉBÉfÉìÉX */
			calc_flag = 1;
			val2=val1*5;
			break;
		case SC_REFLECTSHIELD:
			val2=10+val1*3;
			break;
		case SC_STRIPWEAPON:
			if (val2==0) val2=90;
			break;
		case SC_STRIPSHIELD:
			if (val2==0) val2=85;
			break;

		case SC_AUTOSPELL:			/* ÉI?ÉgÉXÉyÉã */
			val4 = 5 + val1*2;
			break;

		case SC_VOLCANO:
			calc_flag = 1;
			val3 = val1*10;
			break;
		case SC_DELUGE:
			calc_flag = 1;
			if (sc_data[SC_FOGWALL].timer != -1 && sc_data[SC_BLIND].timer != -1)
				status_change_end(bl,SC_BLIND,-1);
			break;
		case SC_VIOLENTGALE:
			calc_flag = 1;
			val3 = val1*3;
			break;

		case SC_SPEARSQUICKEN:		/* ÉXÉsÉAÉNÉCÉbÉPÉì */
			calc_flag = 1;
			val2 = 20+val1;
			*opt3 |= 1;
			break;

		case SC_BLADESTOP:		/* îíênéÊÇË */
			if(val2==2) clif_bladestop((struct block_list *)val3,(struct block_list *)val4,1);
			*opt3 |= 32;
			break;

		case SC_LULLABY:			/* éqéÁâS */
		case SC_RICHMANKIM:
		case SC_ROKISWEIL:			/* ÉçÉLÇÃã©Ç— */
		case SC_INTOABYSS:			/* ê[ï£ÇÃíÜÇ… */
		case SC_POEMBRAGI:			/* ÉuÉâÉMÇÃéç */
		case SC_UGLYDANCE:			/* é©ï™èüéËÇ»É_ÉìÉX */
			break;
		case SC_ETERNALCHAOS:		/* ÉGÉ^?ÉiÉãÉJÉIÉX */
		case SC_DRUMBATTLE:			/* ?ëæå€ÇÃãøÇ´ */
		case SC_NIBELUNGEN:			/* Éj?ÉxÉãÉìÉOÇÃéwó÷ */
		case SC_SIEGFRIED:			/* ïséÄêgÇÃÉW?ÉNÉtÉä?Éh */
		case SC_WHISTLE:			/* å˚ìJ */
		case SC_ASSNCROS:			/* ó[ózÇÃÉAÉTÉVÉìÉNÉçÉX */
		case SC_APPLEIDUN:			/* ÉCÉhÉDÉìÇÃó—åÁ */
		case SC_HUMMING:			/* ÉnÉ~ÉìÉO */
		case SC_ATKPOTION: // Valaris
		case SC_MATKPOTION:
		case SC_FORTUNE:			/* çKâ^ÇÃÉLÉX */
		case SC_SERVICE4U:			/* ÉT?ÉrÉXÉtÉH?ÉÜ? */
			calc_flag = 1;
			break;
		case SC_DONTFORGETME:		/* éÑÇñYÇÍÇ»Ç¢Ç≈ */
			calc_flag = 1;
			if(sc_data[SC_INCREASEAGI].timer!=-1 )	/* ë¨ìxè„è∏âèú */
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc_data[SC_ONEHAND].timer!=-1 )
				status_change_end(bl,SC_ONEHAND,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_ADRENALINE2].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc_data[SC_ASSNCROS].timer!=-1 )
				status_change_end(bl,SC_ASSNCROS,-1);
			if(sc_data[SC_TRUESIGHT].timer!=-1 )	/* ÉgÉDÉã?ÉTÉCÉg */
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc_data[SC_WINDWALK].timer!=-1 )	/* ÉEÉCÉìÉhÉEÉH?ÉN */
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc_data[SC_CARTBOOST].timer!=-1 )	/* ÉJ?ÉgÉu?ÉXÉg */
				status_change_end(bl,SC_CARTBOOST,-1);
			break;
		case SC_MOONLIT:
			val2 = bl->id;
			skill_setmapcell(bl,CG_MOONLIT, val1, CELL_SETMOONLIT);
			break;
		case SC_DANCING:			/* É_ÉìÉX/ââëtíÜ */
			calc_flag = 1;
			if (!(flag&4))
			{
				val3= tick / 1000;
				tick = 1000;
			}
			break;

		case SC_EXPLOSIONSPIRITS:	// îöóÙîgìÆ
			calc_flag = 1;
			val2 = 75 + 25*val1;
			*opt3 |= 8;
			break;
		case SC_STEELBODY:			// ã‡çÑ
		case SC_SKA:
			calc_flag = 1;
			*opt3 |= 16;
			break;
		case SC_AUTOCOUNTER:
			val3 = val4 = 0;
			break;

		case SC_ASPDPOTION0:		/* ?ë¨É|?ÉVÉáÉì */
		case SC_ASPDPOTION1:
		case SC_ASPDPOTION2:
		case SC_ASPDPOTION3:
			calc_flag = 1;
			if (!(flag&4))
				val2 = 5*(2+type-SC_ASPDPOTION0);
			break;

		case SC_XMAS: // Xmas Suit [Valaris]
		case SC_WEDDING:	//åãç•óp(åãç•àﬂè÷Ç…Ç»Ç¡Çƒ?Ç≠ÇÃÇ™?Ç¢Ç∆Ç©)
			{
				struct map_session_data *sd;
				if (bl->type == BL_PC && (sd= (struct map_session_data *)bl))
				{	//Change look.
					if(type==SC_WEDDING)
						sd->view_class = JOB_WEDDING;
					else if(type==SC_XMAS)
						sd->view_class = JOB_XMAS;
					clif_changelook(&sd->bl,LOOK_BASE,sd->view_class);
#if PACKETVER < 4
					clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
					clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
#else
					clif_changelook(&sd->bl,LOOK_WEAPON,0);
#endif
					if(battle_config.save_clothcolor && sd->status.clothes_color > 0 && 
						((type==SC_WEDDING && !battle_config.wedding_ignorepalette) || 
							(type==SC_XMAS && !battle_config.xmas_ignorepalette)))
						clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->status.clothes_color);
				}
			}
			break;
		case SC_NOCHAT:	//É`ÉÉÉbÉgã÷é~?ë‘
			{
				if(!battle_config.muting_players)
					return 0;
				
				if (!(flag&4))
					tick = 60000;
				updateflag = SP_MANNER;
				save_flag = 1; // celest
			}
			break;

		/* option1 */
		case SC_STONE:				/* êŒâª */
			if(!(flag&2)) {
				int sc_def = status_get_mdef(bl)*200;
				tick = tick - sc_def;
			}
			if (!(flag&4))
				val3 = tick/1000;
			if(val3 < 1) val3 = 1;
			if (!(flag&4))
				tick = 5000;
			val2 = 1;
			break;
		case SC_SLEEP:				/* êáñ∞ */
			if(!(flag&4)) {
				tick = 30000;//êáñ∞ÇÕÉXÉe?É^ÉXëœê´Ç…?ÇÌÇÁÇ∏30ïb
			}
			break;
		case SC_FREEZE:				/* ìÄåã */
			if(!(flag&2)) {
				int sc_def = 100 - status_get_mdef(bl);
				tick = tick * sc_def / 100;
			}
			break;
		case SC_STAN:				/* ÉXÉ^ÉìÅival2Ç…É~ÉäïbÉZÉbÉgÅj */
			if(!(flag&2)) {
				int sc_def = status_get_sc_def_vit(bl);
				tick = tick * sc_def / 100;
			}
			break;

			/* option2 */
		case SC_DPOISON:			/* ñ“ì≈ */
		{
			int hp = status_get_hp(bl);
			int mhp = status_get_max_hp(bl);

			// MHP?1/4????????
			if (hp > mhp>>2) {
				if(bl->type == BL_PC) {
					int diff = mhp*10/100;
					if (hp - diff < mhp>>2)
						diff = hp - (mhp>>2);
					pc_heal((struct map_session_data *)bl, -diff, 0);
				} else if(bl->type == BL_MOB) {
					struct mob_data *md = (struct mob_data *)bl;
					hp -= mhp*15/100;
					if (hp > mhp>>2)
						md->hp = hp;
					else
						md->hp = mhp>>2;
				}
			}
		}	// fall through
		case SC_POISON:				/* ì≈ */
		{
			int mhp;

			calc_flag = 1;
			if (flag&4)
				break;
			if(!(flag&2)) {
				int sc_def = 100 - (status_get_vit(bl) + status_get_luk(bl)/5);
				tick = tick * sc_def / 100;
			}
			val3 = tick/1000;
			if(val3 < 1) val3 = 1;
			tick = 1000;
			mhp = status_get_max_hp(bl);
			if (bl->type == BL_PC)
				val4 = (type == SC_DPOISON) ? 3 + mhp/50 : 3 + mhp*3/200;
			else
				val4 = (type == SC_DPOISON) ? 3 + mhp/100 : 3 + mhp/200;
		
		}
		break;
		case SC_SILENCE:			/* íæ?ÅiÉåÉbÉNÉXÉfÉr?ÉiÅj */
			if (sc_data && sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4 == BCT_SELF) { //Clear Gospel [Skotlex]
					status_change_end(bl,SC_GOSPEL,-1);
				}
				break;
			}
			if(!(flag&2)) {
				int sc_def = 100 - status_get_vit(bl);
				tick = tick * sc_def / 100;
			}
			break;
		case SC_CONFUSION:
			clif_emotion(bl,1);
			break;
		case SC_BLIND:				/* à√? */
			calc_flag = 1;
			if(!(flag&4) && tick < 1000)
				tick = 30000;
			if(!(flag&2)) {
				int sc_def = 100 - (status_get_lv(bl)/10 + status_get_int(bl)/15);
				tick = tick*sc_def/100;
				if (tick < 5000) //Minimum 5 secs?
					tick = 5000;
			}
			break;
		case SC_CURSE:
			calc_flag = 1;
			if(!(flag&2)) {
				int sc_def = 100 - status_get_vit(bl);
				tick = tick * sc_def / 100;
			}
			break;

		case SC_BLEEDING:
			if(!(flag&2)) {
				int sc_def = 100 - (status_get_lv(bl)/5 +status_get_vit(bl));
				tick = tick * sc_def / 100;
			}
			if(!(flag&4) && tick < 10000) //Minimum bleed time is 10 secs or this sc does nothing! [Skotlex]
				tick = 10000;
			val4 = tick;
			tick = 10000;
			break;

		/* option */
		case SC_HIDING:		/* ÉnÉCÉfÉBÉìÉO */
			calc_flag = 1;
			if(bl->type == BL_PC && !(flag&4)) {
				val2 = tick / 1000;		/* éù?éûä‘ */
				tick = 1000;
			}
			break;
		case SC_CHASEWALK:
		case SC_CLOAKING:		/* ÉNÉç?ÉLÉìÉO */
			if (flag&4)
				break;
			if(bl->type != BL_PC)
				tick = 5000*val1;
			calc_flag = 1; // [Celest]
			val2 = tick;
			val3 = type==SC_CLOAKING ? 130-val1*3 : 135-val1*5;
			break;
		case SC_SIGHT:			/* ÉTÉCÉg/ÉãÉAÉt */
		case SC_RUWACH:
		case SC_SIGHTBLASTER:
			if (flag&4)
				break;
			val2 = tick/250;
			tick = 10;
			break;

		case SC_WEIGHT50:
		case SC_WEIGHT90:
		case SC_BROKENWEAPON:
		case SC_BROKENARMOR:
		case SC_READYSTORM: // Taekwon stances SCs [Dralnu]
		case SC_READYDOWN:
		case SC_READYCOUNTER:
		case SC_READYTURN:
		case SC_DODGE:
			if (flag&4)
				break;
			tick = 600*1000;
			break;

		case SC_AUTOGUARD:
			if (!flag)
			{
				struct map_session_data *tsd;
				int i,t;
				for(i=val2=0;i<val1;i++) {
					t = 5-(i>>1);
					val2 += (t < 0)? 1:t;
				}
				if (sd)
					for (i = 0; i < 5; i++)
					{	//Pass the status to the other affected chars. [Skotlex]
						if (sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])))
							status_change_start(&tsd->bl,SC_AUTOGUARD,val1,val2,0,0,tick,1);
					}
			}
			break;

		case SC_DEFENDER:
			calc_flag = 1;
			if (!flag)
			{	
				struct map_session_data *tsd;
				int i;
				val2 = 5 + val1*15;
				if (sd)
					for (i = 0; i < 5; i++)
					{	//See if there are devoted characters, and pass the status to them. [Skotlex]
						if (sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])))
							status_change_start(&tsd->bl,SC_DEFENDER,val1,val2,0,0,tick,1);
					}
			}
			break;

		case SC_CONCENTRATION:	/* ÉRÉìÉZÉìÉgÉå?ÉVÉáÉì */
			*opt3 |= 1;
			calc_flag = 1;
			break;

		case SC_TENSIONRELAX:	/* ÉeÉìÉVÉáÉìÉäÉâÉbÉNÉX */
			if (flag&4)
				break;
			if(bl->type == BL_PC) {
				tick = 10000;
			} else return 0;
			break;

		case SC_PARRYING:		/* ÉpÉäÉCÉìÉO */
		    val2 = 20 + val1*3;
			break;

		case SC_WINDWALK:		/* ÉEÉCÉìÉhÉEÉH?ÉN */
			calc_flag = 1;
			val2 = (val1+1)/2; // Flee bonus is 1/1/2/2/3/3/4/4/5/5, movement speed % increase is 4 times that
			break;

		case SC_JOINTBEAT: // Random break [DracoRPG]
			calc_flag = 1;
			val2 = rand()%6;
			if (val2 == 5) status_change_start(bl,SC_BLEEDING,val1,0,0,0,skill_get_time2(type,val1),0);
			break;

		case SC_BERSERK:		/* Éo?ÉT?ÉN */
			if(sd && !(flag&4)){
				sd->status.hp = sd->status.max_hp * 3;
				sd->status.sp = 0;
				clif_updatestatus(sd,SP_HP);
				clif_updatestatus(sd,SP_SP);
				sd->canregen_tick = gettick() + 300000;
			}
			*opt3 |= 128;
			if (!(flag&4))
				tick = 10000;
			calc_flag = 1;
			break;

		case SC_ASSUMPTIO:		/* ÉAÉXÉÄÉvÉeÉBÉI */
			*opt3 |= 2048;
			if(sc_data[SC_KYRIE].timer!=-1)
				status_change_end(bl,SC_KYRIE,-1);
			break;

		case SC_WARM: //SG skills [Komurka]
			if (!(flag&4)) {
				val2 = tick/1000;
				tick = 1000;
			}
			*opt3 |= 4096;
			opt_flag = 1;
			break;

		case SC_GOSPEL:
			if (val4 == BCT_SELF) {	// self effect
				if (flag&4)
					break;
				val2 = tick;
				tick = 1000;
				status_change_clear_buffs(bl);
				status_change_clear_debuffs(bl); //Gospel clears both types.
			} else
				calc_flag = 1;
			break;

		case SC_MARIONETTE:		/* É}ÉäÉIÉlÉbÉgÉRÉìÉgÉç?Éã */
		case SC_MARIONETTE2:
			if (flag&4)
				break;
			val2 = tick;
			if (!val3)
				return 0;
			tick = 1000;
			calc_flag = 1;
			*opt3 |= 1024;
			break;

		case SC_REJECTSWORD:	/* ÉäÉWÉFÉNÉgÉ\?Éh */
			val2 = 3; //3âÒçU?ÇíµÇÀï‘Ç∑
			break;

		case SC_MEMORIZE:		/* ÉÅÉÇÉâÉCÉY */
			val2 = 5; //âÒârè•Ç1/3Ç…Ç∑ÇÈ
			break;

		case SC_GRAVITATION:
			if (sd) {
				if (val3 == BCT_SELF) {
					sd->canmove_tick += tick;
					sd->canact_tick += tick;
				} else calc_flag = 1;
			}
			break;

		case SC_HERMODE:
			status_change_clear_buffs(bl);
			break;

		case SC_REGENERATION:
			val1 = 2;
		case SC_BATTLEORDERS:
			if (!(flag&4))
				tick = 60000; // 1 minute
			calc_flag = 1;
			break;
		case SC_GUILDAURA:
			calc_flag = 1;
			if (!(flag&4))
				tick = 1000;
			break;

		case SC_DEVOTION:			/* ÉfÉBÉ{?ÉVÉáÉì */
		{
			struct map_session_data *src;
			if ((src = map_id2sd(val1)) && src->sc_count)
			{	//Try to inherit the status from the Crusader [Skotlex]
			//Ideally, we should calculate the remaining time and use that, but we'll trust that
			//once the Crusader's status changes, it will reflect on the others. 
				if (src->sc_data[SC_AUTOGUARD].timer != -1)
					status_change_start(bl,SC_AUTOGUARD,src->sc_data[SC_AUTOGUARD].val1,0,0,0,
						skill_get_time(CR_AUTOGUARD,src->sc_data[SC_AUTOGUARD].val1),0);
				if (src->sc_data[SC_DEFENDER].timer != -1)
					status_change_start(bl,SC_DEFENDER,src->sc_data[SC_DEFENDER].val1,0,0,0,
						skill_get_time(CR_DEFENDER,src->sc_data[SC_DEFENDER].val1),0);
			}
			break;
		}

		case SC_COMA: //Coma. Sends a char to 1HP/SP
			battle_damage(NULL, bl, status_get_hp(bl)-1, 0);
			if (sd) pc_heal(sd,0,-sd->status.sp+1);
			return 0;

		case SC_CARTBOOST:		/* ÉJ?ÉgÉu?ÉXÉg */
			if(sc_data[SC_DECREASEAGI].timer!=-1 )
			{	//Cancel Decrease Agi, but take no further effect [Skotlex]
				status_change_end(bl,SC_DECREASEAGI,-1);
				return 0;
			}
			calc_flag = 1;
			break;

		case SC_CLOSECONFINE2:
			{
				struct block_list *src = val2?map_id2bl(val2):NULL;
				struct status_change *sc_data2 = src?status_get_sc_data(src):NULL;
				if (src && sc_data2) {
					if (sc_data2[SC_CLOSECONFINE].timer == -1) //Start lock on caster.
						status_change_start(src,SC_CLOSECONFINE,1,0,0,0,tick+1000,0);
					else { //Increase count of locked enemies and refresh time.
						sc_data2[SC_CLOSECONFINE].val1++;
						delete_timer(sc_data2[SC_CLOSECONFINE].timer, status_change_timer);
						sc_data2[SC_CLOSECONFINE].timer = add_timer(gettick()+tick+1000, status_change_timer, src->id, SC_CLOSECONFINE);
					}
				}
			}
			break;
		case SC_KAITE:
			val2 = 1+val1/5; //Number of bounces: 1 + skilllv/5
			break;
		case SC_KAUPE:
			if (flag&4)
				break; //Do nothing when loading.
			switch (val1) {
				case 3: //33*3 + 1 -> 100%
					val2++;
				case 1:
				case 2: //33, 66%
					val2 += 33*val1;
					val3 = 1; //Dodge 1 attack total.
					break;
				default: //Custom. For high level mob usage, higher level means more blocks. [Skotlex]
					val2 = 100;
					val3 = val1-2;
					break;
			}
			break;
		case SC_COMBO:
			switch (val1) { //Val1 contains the skill id
				case TK_STORMKICK:
					clif_skill_nodamage(bl,bl,TK_READYSTORM,1,1);
					if (sd) sd->attackabletime = gettick()+tick;
					break;
				case TK_DOWNKICK:
					clif_skill_nodamage(bl,bl,TK_READYDOWN,1,1);
					if (sd) sd->attackabletime = gettick()+tick;
					break;
				case TK_TURNKICK:
					clif_skill_nodamage(bl,bl,TK_READYTURN,1,1);
					if (sd) sd->attackabletime = gettick()+tick;
					break;
				case TK_COUNTER:
					clif_skill_nodamage(bl,bl,TK_READYCOUNTER,1,1);
					if (sd) sd->attackabletime = gettick()+tick;
					break;
			}
			break;
		case SC_SWOO: // [marquis007]
			if (!(flag&4) && status_get_mode(bl)&MD_BOSS)
				tick /= 4; //Reduce skill's duration. But for how long?
//			*opt3 |= 8192; //We haven't figured out this value yet...
			opt_flag = 1;
			calc_flag = 1;
			break;
		case SC_TKDORI:
			val2 = 11-val1; //Chance to consume: 11-skilllv%
			break;
		case SC_RUN:
			if (!(flag&4))
				val4 = gettick(); //Store time at which you started running.
			calc_flag = 1;
			break;

		case SC_CONCENTRATE:		/* èWíÜóÕå¸è„ */
		case SC_BLESSING:			/* ÉuÉåÉbÉVÉìÉO */
		case SC_ANGELUS:			/* ÉAÉìÉ[ÉãÉX */
		case SC_IMPOSITIO:			/* ÉCÉìÉ|ÉVÉeÉBÉIÉ}ÉkÉX */
		case SC_GLORIA:				/* ÉOÉçÉäÉA */
		case SC_LOUD:				/* ÉâÉEÉhÉ{ÉCÉX */
		case SC_KEEPING:
		case SC_BARRIER:
		case SC_MELTDOWN:		/* ÉÅÉãÉgÉ_ÉEÉì */
		case SC_TRUESIGHT:		/* ÉgÉDÉã?ÉTÉCÉg */
		case SC_SPIDERWEB:		/* ÉXÉpÉCÉ_?ÉEÉFÉbÉu */
		case SC_SLOWDOWN:
		case SC_SPEEDUP0:
		case SC_SPEEDUP1:
		case SC_INCALLSTATUS:
		case SC_INCHIT:			/* HITè„è∏ */
		case SC_INCHITRATE:		/* HIT%è„è∏ */
		case SC_INCFLEE:		/* FLEEè„è∏ */
		case SC_INCFLEERATE:		/* FLEE%è„è∏ */
		case SC_INCMHPRATE:		/* MHP%è„è∏ */
		case SC_INCMSPRATE:		/* MSP%è„è∏ */
		case SC_INCATKRATE:		/* ATK%è„è∏ */
		case SC_INCMATKRATE:
		case SC_INCDEFRATE:
		case SC_INCSTR:
		case SC_INCAGI:
		case SC_INCVIT:
		case SC_INCINT:
		case SC_INCDEX:
		case SC_INCLUK:
		case SC_STRFOOD:
		case SC_AGIFOOD:
		case SC_VITFOOD:
		case SC_INTFOOD:
		case SC_DEXFOOD:
		case SC_LUKFOOD:
		case SC_FLEEFOOD:
		case SC_HITFOOD:
		case SC_BATKFOOD:
		case SC_WATKFOOD:
		case SC_MATKFOOD:
		case SC_SPURT:
		case SC_SPIRIT:
		case SC_SUN_COMFORT:
		case SC_MOON_COMFORT:
		case SC_STAR_COMFORT:
		case SC_FUSION:
		case SC_SKE:
			calc_flag = 1;
			break;

		case SC_SAFETYWALL:
		case SC_SLOWPOISON: //Slow potion can be activated even if not poisoned.
		case SC_SUFFRAGIUM:			/* ÉTÉtÉâÉMÉÄ */
		case SC_BENEDICTIO:			/* êπ? */
		case SC_MAGNIFICAT:			/* É}ÉOÉjÉtÉBÉJ?Ég */
		case SC_AETERNA:			/* ÉG?ÉeÉãÉi */
  		case SC_STRIPARMOR:
		case SC_STRIPHELM:
		case SC_CP_WEAPON:
		case SC_CP_SHIELD:
		case SC_CP_ARMOR:
		case SC_CP_HELM:
		case SC_EXTREMITYFIST:		/* à¢èCóÖîeôÄåù */
		case SC_ANKLE:	/* ÉAÉìÉNÉã */
		case SC_BLADESTOP_WAIT:		/* îíênéÊÇË(ë“Çø) */
		case SC_HALLUCINATION:
		case SC_SPLASHER:		/* ÉxÉiÉÄÉXÉvÉâÉbÉVÉÉ? */
		case SC_FOGWALL:
		case SC_PRESERVE:
		case SC_DOUBLECAST:
		case SC_AURABLADE:		/* ÉI?ÉâÉuÉå?Éh */
		case SC_BABY:
		case SC_WATK_ELEMENT:
		case SC_ARMOR_ELEMENT:
		case SC_LONGING:
		case SC_ORCISH:
		case SC_SHRINK:
		case SC_WINKCHARM:
		case SC_SCRESIST:
		case SC_STOP:
		case SC_CLOSECONFINE:
		case SC_SKILLRATE_UP:
		case SC_KAIZEL:
		case SC_KAAHI:
		case SC_INTRAVISION:
		case SC_BASILICA:
			break;

		default:
			if(battle_config.error_log)
				ShowError("UnknownStatusChange [%d]\n", type);
			return 0;
	}

	if (bl->type == BL_PC && (battle_config.display_hallucination || type != SC_HALLUCINATION))
	{
		if (flag&4)
			clif_status_load(bl,StatusIconChangeTable[type],1); //Sending to owner since they aren't in the map yet. [Skotlex]
		clif_status_change(bl,StatusIconChangeTable[type],1);
	}

	/* optionÇÃ?çX */
	switch(type){
		case SC_STONE:
		case SC_FREEZE:
		case SC_STAN:
		case SC_SLEEP:

			// Cancel cast when get status [LuzZza]
			if (bl->type == BL_PC) {
				struct map_session_data *sd = (struct map_session_data *)bl; //Only Pressure is uninterruptable.
				if (sd->skilltimer != -1 && sd->skillid != PA_PRESSURE) skill_castcancel(bl, 0);
			} else
			if (bl->type == BL_MOB) {
				if (((struct mob_data *)bl)->skilltimer != -1) skill_castcancel(bl, 0);
			}	    		

			battle_stopattack(bl);	/* çU?í‚é~ */
			skill_stop_dancing(bl);	/* ââët/É_ÉìÉXÇÃíÜ? */
			{	/* ìØéûÇ…ä|Ç©ÇÁÇ»Ç¢ÉXÉe?É^ÉXàŸèÌÇâèú */
				int i;
				for(i = SC_STONE; i <= SC_SLEEP; i++){
					if(sc_data[i].timer != -1){
						(*sc_count)--;
						delete_timer(sc_data[i].timer, status_change_timer);
						sc_data[i].timer = -1;
					}
				}
			}
			if(type == SC_STONE)
				*opt1 = OPT1_STONEWAIT;
			else
				*opt1 = OPT1_STONE + (type - SC_STONE);
			opt_flag = 1;
			break;
		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			*opt2 |= 1<<(type-SC_POISON);
			opt_flag = 1;
			break;
		case SC_DPOISON:	// ébíËÇ≈ì≈ÇÃÉGÉtÉFÉNÉgÇégóp
			*opt2 |= OPT2_DPOISON;
			opt_flag = 1;
			break;
		case SC_SIGNUMCRUCIS:
			*opt2 |= OPT2_SIGNUMCRUCIS;
			opt_flag = 1;
			break;
		case SC_HIDING:
		case SC_CLOAKING:
			battle_stopattack(bl);	/* çU?í‚é~ */
			*option |= ((type==SC_HIDING)?OPTION_HIDE:OPTION_CLOAK);
			opt_flag =1 ;
			break;
		case SC_CHASEWALK:
			battle_stopattack(bl);	/* çU?í‚é~ */
			*option |= OPTION_CHASEWALK|OPTION_CLOAK;
			opt_flag =1 ;
			break;
		case SC_SIGHT:
			*option |= OPTION_SIGHT;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			*option |= OPTION_RUWACH;
			opt_flag = 1;
			break;
		case SC_WEDDING:
			*option |= OPTION_WEDDING;
			opt_flag = 1;
			break;
		case SC_ORCISH:
			*option |= OPTION_ORCISH;
			opt_flag = 1;
			break;
		case SC_SIGHTTRASHER:
			*option |= OPTION_SIGHTTRASHER;
			opt_flag = 1;
			break;
		case SC_FUSION:
			*option |= OPTION_FLYING;
			opt_flag = 1;
			break;
	}

	if(opt_flag)	/* optionÇÃ?çX */
		clif_changeoption(bl);

	(*sc_count)++;	/* ÉXÉe?É^ÉXàŸèÌÇÃ? */

	sc_data[type].val1 = val1;
	sc_data[type].val2 = val2;
	sc_data[type].val3 = val3;
	sc_data[type].val4 = val4;
	/* É^ÉCÉ}?ê›íË */
	sc_data[type].timer = add_timer(
		gettick() + tick, status_change_timer, bl->id, type);

	if(bl->type==BL_PC && calc_flag)
		status_calc_pc(sd,0);	/* ÉXÉe?É^ÉXçƒåvéZ */

	if(bl->type==BL_PC && save_flag)
		chrif_save(sd,0); // save the player status

	if(bl->type==BL_PC && updateflag)
		clif_updatestatus(sd,updateflag);	/* ÉXÉe?É^ÉXÇÉNÉâÉCÉAÉìÉgÇ…ëóÇÈ */

	if (bl->type==BL_PC && sd->pd)
		pet_sc_check(sd, type); //Skotlex: Pet Status Effect Healing

	if(type==SC_RUN && bl->type==BL_PC)
		pc_run(sd,val1,val2);
	
	return 0;
}
/*==========================================
 * ÉXÉeÅ[É^ÉXàŸèÌëSâèú
 *------------------------------------------
 */
int status_change_clear(struct block_list *bl,int type)
{
	struct status_change* sc_data;
	short *sc_count, *option, *opt1, *opt2, *opt3;
	int i;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc_data = status_get_sc_data(bl));
	nullpo_retr(0, sc_count = status_get_sc_count(bl));
	nullpo_retr(0, option = status_get_option(bl));
	nullpo_retr(0, opt1 = status_get_opt1(bl));
	nullpo_retr(0, opt2 = status_get_opt2(bl));
	nullpo_retr(0, opt3 = status_get_opt3(bl));

	if (*sc_count == 0)
		return 0;
	for(i = 0; i < SC_MAX; i++)
	{
		//Type 0: PC killed -> EDP and Meltdown must not be dispelled. [Skotlex]
		// Do not reset Xmas status when killed. [Valaris]
		if(sc_data[i].timer == -1 ||
			(type == 0 && (i == SC_EDP || i == SC_MELTDOWN || i == SC_XMAS)))
			continue;

		status_change_end(bl, i, -1);

		if (type == 1 && sc_data[i].timer != -1)
		{	//If for some reason status_change_end decides to still keep the status when quitting. [Skotlex]
			(*sc_count)--;
			delete_timer(sc_data[i].timer, status_change_timer);
			sc_data[i].timer = -1;
		}
	}
	//We can't assume the count is 0, some status don't end even when dead! [Skotlex]
	//(*sc_count) = 0;
	*opt1 = 0;
	*opt2 = 0;
	*opt3 = 0;
	*option &= OPTION_MASK;

	if(!type || type&2)
		clif_changeoption(bl);

	return 0;
}

/*==========================================
 * ÉXÉeÅ[É^ÉXàŸèÌèIóπ
 *------------------------------------------
 */
int status_change_end( struct block_list* bl , int type,int tid )
{
	struct map_session_data *sd;
	struct status_change* sc_data;
	int opt_flag=0, calc_flag = 0;
	short *sc_count, *option, *opt1, *opt2, *opt3;

	nullpo_retr(0, bl);
	if(bl->type!=BL_PC && bl->type!=BL_MOB) {
		if(battle_config.error_log)
			ShowError("status_change_end: neither MOB nor PC !\n");
		return 0;
	}

	if(type < 0 || type >= SC_MAX)
		return 0;

	sd = bl->type==BL_PC?(struct map_session_data *)bl:NULL;
	
	sc_data = status_get_sc_data(bl);
	sc_count = status_get_sc_count(bl);
	option = status_get_option(bl);
	opt1 = status_get_opt1(bl);
	opt2 = status_get_opt2(bl);
	opt3 = status_get_opt3(bl);

	if (sc_data[type].timer != -1 && (sc_data[type].timer == tid || tid == -1)) {

		if (tid == -1)	// É^ÉCÉ}Ç©ÇÁåƒÇŒÇÍÇƒÇ¢Ç»Ç¢Ç»ÇÁÉ^ÉCÉ}çÌèúÇÇ∑ÇÈ
			delete_timer(sc_data[type].timer,status_change_timer);

		/* äY?ÇÃàŸèÌÇê≥èÌÇ…?Ç∑ */
		sc_data[type].timer=-1;
		(*sc_count)--;

		switch(type){	/* àŸèÌÇÃéÌóﬁÇ≤Ç∆ÇÃ?óù */
			case SC_PROVOKE:			/* ÉvÉçÉ{ÉbÉN */
			case SC_ENDURE: // celest
			case SC_CONCENTRATE:		/* èWíÜóÕå¸è„ */
			case SC_BLESSING:			/* ÉuÉåÉbÉVÉìÉO */
			case SC_ANGELUS:			/* ÉAÉìÉ[ÉãÉX */
			case SC_INCREASEAGI:		/* ë¨ìxè„è∏ */
			case SC_DECREASEAGI:		/* ë¨ìxå∏è≠ */
			case SC_SIGNUMCRUCIS:		/* ÉVÉOÉiÉÄÉNÉãÉVÉX */
			case SC_HIDING:
			case SC_ONEHAND:
			case SC_TWOHANDQUICKEN:		/* 2HQ */
			case SC_ADRENALINE2:
			case SC_ADRENALINE:			/* ÉAÉhÉåÉiÉäÉìÉâÉbÉVÉÖ */
			case SC_ENCPOISON:			/* ÉGÉìÉ`ÉÉÉìÉgÉ|ÉCÉYÉì */
			case SC_IMPOSITIO:			/* ÉCÉìÉ|ÉVÉeÉBÉIÉ}ÉkÉX */
			case SC_GLORIA:				/* ÉOÉçÉäÉA */
			case SC_LOUD:				/* ÉâÉEÉhÉ{ÉCÉX */
			case SC_QUAGMIRE:			/* ÉNÉ@ÉOÉ}ÉCÉA */
			case SC_PROVIDENCE:			/* ÉvÉçÉîÉBÉfÉìÉX */
			case SC_SPEARSQUICKEN:		/* ÉXÉsÉAÉNÉCÉbÉPÉì */
			case SC_VOLCANO:
			case SC_DELUGE:
			case SC_VIOLENTGALE:
			case SC_ETERNALCHAOS:		/* ÉGÉ^?ÉiÉãÉJÉIÉX */
			case SC_DRUMBATTLE:			/* ?ëæå€ÇÃãøÇ´ */
			case SC_NIBELUNGEN:			/* Éj?ÉxÉãÉìÉOÇÃéwó÷ */
			case SC_SIEGFRIED:			/* ïséÄêgÇÃÉW?ÉNÉtÉä?Éh */
			case SC_WHISTLE:			/* å˚ìJ */
			case SC_ASSNCROS:			/* ó[ózÇÃÉAÉTÉVÉìÉNÉçÉX */
			case SC_HUMMING:			/* ÉnÉ~ÉìÉO */
			case SC_DONTFORGETME:		/* éÑÇñYÇÍÇ»Ç¢Ç≈ */
			case SC_FORTUNE:			/* çKâ^ÇÃÉLÉX */
			case SC_SERVICE4U:			/* ÉT?ÉrÉXÉtÉH?ÉÜ? */
			case SC_EXPLOSIONSPIRITS:	// îöóÙîgìÆ
			case SC_STEELBODY:			// ã‡çÑ
			case SC_APPLEIDUN:			/* ÉCÉhÉDÉìÇÃó—åÁ */
			case SC_BLADESTOP_WAIT:
			case SC_CONCENTRATION:		/* ÉRÉìÉZÉìÉgÉå?ÉVÉáÉì */
			case SC_ASSUMPTIO:			/* ÉAÉVÉÉÉìÉvÉeÉBÉI */
			case SC_WINDWALK:		/* ÉEÉCÉìÉhÉEÉH?ÉN */
			case SC_TRUESIGHT:		/* ÉgÉDÉã?ÉTÉCÉg */
			case SC_SPIDERWEB:		/* ÉXÉpÉCÉ_?ÉEÉFÉbÉu */
			case SC_MAGICPOWER:		/* ñÇñ@óÕ?ïù */
			case SC_CHASEWALK:
			case SC_ATKPOTION:		// [Valaris]
			case SC_MATKPOTION:		// [Valaris]
			case SC_MELTDOWN:		/* ÉÅÉãÉgÉ_ÉEÉì */
			case SC_CARTBOOST:
			case SC_MINDBREAKER:		/* É}ÉCÉìÉhÉuÉåÅ[ÉJÅ[ */
			case SC_EDP:	// Celest
			case SC_SLOWDOWN:
			case SC_ASPDPOTION0:		/* ?ë¨É|?ÉVÉáÉì */
			case SC_ASPDPOTION1:
			case SC_ASPDPOTION2:
			case SC_ASPDPOTION3:
			case SC_SPEEDUP0:
			case SC_SPEEDUP1:
			case SC_INCALLSTATUS:
			case SC_INCHIT:			/* HITè„è∏ */
			case SC_INCHITRATE:		/* HIT%è„è∏ */
			case SC_INCFLEE:		/* FLEEè„è∏ */
			case SC_INCFLEERATE:		/* FLEE%è„è∏ */
			case SC_INCMHPRATE:		/* MHP%è„è∏ */
			case SC_INCMSPRATE:		/* MSP%è„è∏ */
			case SC_INCATKRATE:		/* ATK%è„è∏ */
			case SC_INCMATKRATE:
			case SC_INCDEFRATE:
			case SC_INCSTR:
			case SC_INCAGI:
			case SC_INCVIT:
			case SC_INCINT:
			case SC_INCDEX:
			case SC_INCLUK:
			case SC_STRFOOD:
			case SC_AGIFOOD:
			case SC_VITFOOD:
			case SC_INTFOOD:
			case SC_DEXFOOD:
			case SC_LUKFOOD:
			case SC_FLEEFOOD:
			case SC_HITFOOD:
			case SC_BATKFOOD:
			case SC_WATKFOOD:
			case SC_MATKFOOD:
			case SC_BATTLEORDERS:
			case SC_REGENERATION:
			case SC_GUILDAURA:
			case SC_SPURT:
			case SC_SPIRIT: 
			case SC_SUN_COMFORT:
			case SC_MOON_COMFORT:
			case SC_STAR_COMFORT:
			case SC_FUSION:
			case SC_SKE:
			case SC_SWOO: // [marquis007]
			case SC_SKA: // [marquis007]
				calc_flag = 1;
				break;

			case SC_XMAS: // Xmas Suit [Valaris]
			case SC_WEDDING:	//åãç•óp(åãç•àﬂè÷Ç…Ç»Ç¡Çƒ?Ç≠ÇÃÇ™?Ç¢Ç∆Ç©)
			if (sd) {
				//Restore look
				sd->view_class = sd->status.class_;
				clif_changelook(&sd->bl,LOOK_BASE,sd->view_class);
#if PACKETVER < 4
				clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
				clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
#else
				clif_changelook(&sd->bl,LOOK_WEAPON,0);
#endif
				if(battle_config.save_clothcolor && sd->status.clothes_color > 0)
					clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->status.clothes_color);
			}
			break;
			case SC_RUN://ãÏÇØë´
				if (sd && sd->walktimer != -1)
						pc_stop_walking(sd,1);
				if (sc_data[type].val1 >= 7 &&
					DIFF_TICK(gettick(), sc_data[type].val4) <= 1000 &&
					(!sd || (sd->weapontype1 == 0 && sd->weapontype2 == 0))
				)
					status_change_start(bl,SC_SPURT,sc_data[type].val1,0,0,0,skill_get_time2(TK_RUN, sc_data[type].val1),0);
				calc_flag = 1;
			break;
			case SC_AUTOBERSERK:
				if (sc_data[SC_PROVOKE].timer != -1 && sc_data[SC_PROVOKE].val2 == 1)
					status_change_end(bl,SC_PROVOKE,-1);
				break;

			case SC_DEFENDER:
				calc_flag = 1;
			case SC_AUTOGUARD:
			if (sd) {
				struct map_session_data *tsd;
				int i;
				for (i = 0; i < 5; i++)
				{	//Clear the status from the others too [Skotlex]
					if (sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) && tsd->sc_data[type].timer != -1)
						status_change_end(&tsd->bl,type,-1);
				}
			}
			break;
			case SC_DEVOTION:		/* ÉfÉBÉ{?ÉVÉáÉì */
				{
					struct map_session_data *md = map_id2sd(sc_data[type].val1);
					//The status could have changed because the Crusader left the game. [Skotlex]
					if (md)
					{
						md->devotion[sc_data[type].val2] = 0;
						clif_devotion(md);
					}
					//Remove AutoGuard and Defender [Skotlex]
					if (sc_data[SC_AUTOGUARD].timer != -1)
						status_change_end(bl,SC_AUTOGUARD,-1);
					if (sc_data[SC_DEFENDER].timer != -1)
						status_change_end(bl,SC_DEFENDER,-1);
				}
				break;
			case SC_BLADESTOP:
				{
					struct status_change *t_sc_data = status_get_sc_data((struct block_list *)sc_data[type].val4);
					//ï–ï˚Ç™êÿÇÍÇΩÇÃÇ≈ëäéËÇÃîíên?ë‘Ç™êÿÇÍÇƒÇ»Ç¢ÇÃÇ»ÇÁâèú
					if(t_sc_data && t_sc_data[SC_BLADESTOP].timer!=-1)
						status_change_end((struct block_list *)sc_data[type].val4,SC_BLADESTOP,-1);

					if(sc_data[type].val2==2)
						clif_bladestop((struct block_list *)sc_data[type].val3,(struct block_list *)sc_data[type].val4,0);
				}
				break;
			case SC_DANCING:
				{
					struct map_session_data *dsd;
					struct status_change *d_sc_data;
					if(sc_data[type].val2)
					{
						skill_delunitgroup((struct skill_unit_group *)sc_data[type].val2);
						sc_data[type].val2 = 0;
					}
					if(sc_data[type].val4 && sc_data[type].val4 != BCT_SELF && (dsd=map_id2sd(sc_data[type].val4))){
						d_sc_data = dsd->sc_data;
						//çáëtÇ≈ëäéËÇ™Ç¢ÇÈèÍçáëäéËÇÃval4Ç0Ç…Ç∑ÇÈ
						if(d_sc_data && d_sc_data[type].timer!=-1)
						{
							d_sc_data[type].val2 = d_sc_data[type].val4 = 0; //This will prevent recursive loops. 
							status_change_end(&dsd->bl, type, -1);
						}
					}
					if(sc_data[type].val1 == CG_MOONLIT) //Only dance that doesn't has ground tiles... [Skotlex]
						status_change_end(bl, SC_MOONLIT, -1);
				}
				if (sc_data[SC_LONGING].timer!=-1)
					status_change_end(bl,SC_LONGING,-1);				
				calc_flag = 1;
				break;
			case SC_NOCHAT:	//É`ÉÉÉbÉgã÷é~?ë‘
				if (sd) {
					if(battle_config.manner_system){
						if (sd->status.manner >= 0) // weeee ^^ [celest]
							sd->status.manner = 0;
						clif_updatestatus(sd,SP_MANNER);
					}
				}
				break;
			case SC_SPLASHER:		/* ÉxÉiÉÄÉXÉvÉâÉbÉVÉÉ? */
				{
					struct block_list *src=map_id2bl(sc_data[type].val3);
					if(src && tid!=-1){
						//é©ï™Ç…É_ÉÅ?ÉWÅïé¸?3*3Ç…É_ÉÅ?ÉW
						skill_castend_damage_id(src, bl,sc_data[type].val2,sc_data[type].val1,gettick(),0 );
					}
				}
				break;
			case SC_CLOSECONFINE2:
				{
					struct block_list *src = sc_data[type].val2?map_id2bl(sc_data[type].val2):NULL;
					struct status_change *sc_data2 = src?status_get_sc_data(src):NULL;
					if (src && sc_data2) {
						if (sc_data2[SC_CLOSECONFINE].timer != -1) //If status was already ended, do nothing.
						{ //Decrease count
							if (--sc_data2[SC_CLOSECONFINE].val1 <= 0) //No more holds, free him up.
								status_change_end(src, SC_CLOSECONFINE, -1);
						}
					}
				}
				break;
			case SC_CLOSECONFINE:
				if (sc_data[type].val1 > 0) { //Caster has been unlocked... nearby chars need to be unlocked.
					int range = 2*skill_get_range2(bl, RG_CLOSECONFINE, 1);
					map_foreachinarea(status_change_timer_sub, 
						bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,bl,type,gettick());
				}
				break;
		/* option1 */
			case SC_FREEZE:
				sc_data[type].val3 = 0;
				break;

		/* option2 */
			case SC_POISON:				/* ì≈ */
			case SC_BLIND:				/* à√? */
			case SC_CURSE:
				calc_flag = 1;
				break;

			case SC_MARIONETTE:		/* É}ÉäÉIÉlÉbÉgÉRÉìÉgÉç?Éã */
			case SC_MARIONETTE2:	/// Marionette target
				{
					// check for partner and end their marionette status as well
					int type2 = (type == SC_MARIONETTE) ? SC_MARIONETTE2 : SC_MARIONETTE;
					struct block_list *pbl = map_id2bl(sc_data[type].val3);
					if (pbl) {
						struct status_change* sc_data;
						if ((sc_data = status_get_sc_data(pbl)) && sc_data[type2].timer != -1)
							status_change_end(pbl, type2, -1);
					}
					if (type == SC_MARIONETTE)
						clif_marionette(bl, 0); 
					calc_flag = 1;
				}
				break;

			case SC_BERSERK: //val4 indicates if the skill was dispelled. [Skotlex]
				if (sd && sd->status.hp > 100 && !sc_data[type].val4) {
					sd->status.hp = 100;
					clif_updatestatus(sd,SP_HP);
				}
				calc_flag = 1;
				break;
				
			case SC_GRAVITATION:
				if (sd) {
					if (sc_data[type].val3 == BCT_SELF) {
						unsigned int tick = gettick();
						sd->canmove_tick = tick;
						sd->canact_tick = tick;
					} else calc_flag = 1;
				}
				break;
			
			case SC_GOSPEL: //Clear the buffs from other chars.
				if(sc_data[type].val4 != BCT_SELF)
					calc_flag = 1;
				else if (sc_data[type].val3) { //Clear the group.
					struct skill_unit_group *group = (struct skill_unit_group *)sc_data[type].val3;
					sc_data[type].val3 = 0;
					skill_delunitgroup(group);
				}
				break;
			case SC_HERMODE: 
			case SC_BASILICA: //Clear the skill area. [Skotlex]
				if(sc_data[type].val3 == BCT_SELF)
					skill_clear_unitgroup(bl);
				break;
			case SC_MOONLIT: //Clear the unit effect. [Skotlex]
				skill_setmapcell(bl,CG_MOONLIT, sc_data[SC_MOONLIT].val1, CELL_CLRMOONLIT);
				break;
			}


		if (sd && (battle_config.display_hallucination || type != SC_HALLUCINATION))
			clif_status_change(bl,StatusIconChangeTable[type],0);

		switch(type){	/* ê≥èÌÇ…?ÇÈÇ∆Ç´Ç»Ç…Ç©?óùÇ™ïKóv */
		case SC_STONE:
		case SC_FREEZE:
		case SC_STAN:
		case SC_SLEEP:
			*opt1 = 0;
			opt_flag = 1;
			break;

		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			*opt2 &= ~(1<<(type-SC_POISON));
			opt_flag = 1;
			break;
		case SC_DPOISON:
			*opt2 &= ~OPT2_DPOISON;	// ì≈?ë‘âèú
			opt_flag = 1;
			break;
		case SC_SIGNUMCRUCIS:
			*opt2 &= ~OPT2_SIGNUMCRUCIS;
			opt_flag = 1;
			break;

		case SC_HIDING:
			*option &= ~OPTION_HIDE;
			opt_flag = 1 ;
			break;
		case SC_CLOAKING:
			*option &= ~OPTION_CLOAK;
			calc_flag = 1;	// orn
			opt_flag = 1 ;
			break;
		case SC_CHASEWALK:
			*option &= ~(OPTION_CHASEWALK|OPTION_CLOAK);
			opt_flag = 1 ;
			break;
		case SC_SIGHT:
			*option &= ~OPTION_SIGHT;
			opt_flag = 1;
			break;
		case SC_WEDDING:	//åãç•óp(åãç•àﬂè÷Ç…Ç»Ç¡Çƒ?Ç≠ÇÃÇ™?Ç¢Ç∆Ç©)
			*option &= ~OPTION_WEDDING;
			opt_flag = 1;
			break;
		case SC_ORCISH:
			*option &= ~OPTION_ORCISH;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			*option &= ~OPTION_RUWACH;
			opt_flag = 1;
			break;
		case SC_SIGHTTRASHER:
			*option &= ~OPTION_SIGHTTRASHER;
			opt_flag = 1;
			break;
		case SC_FUSION:
			*option &= ~OPTION_FLYING;
			opt_flag = 1;
			break;
		//opt3
		case SC_TWOHANDQUICKEN:		/* 2HQ */
		case SC_ONEHAND:		/* 1HQ */
		case SC_SPEARSQUICKEN:		/* ÉXÉsÉAÉNÉCÉbÉPÉì */
		case SC_CONCENTRATION:		/* ÉRÉìÉZÉìÉgÉå?ÉVÉáÉì */
			*opt3 &= ~1;
			break;
		case SC_OVERTHRUST:			/* ÉI?Éo?ÉXÉâÉXÉg */
			*opt3 &= ~2;
			break;
		case SC_ENERGYCOAT:			/* ÉGÉiÉW?ÉR?Ég */
			*opt3 &= ~4;
			break;
		case SC_EXPLOSIONSPIRITS:	// îöóÙîgìÆ
			*opt3 &= ~8;
			break;
		case SC_STEELBODY:			// ã‡çÑ
		case SC_SKA:
			*opt3 &= ~16;
			break;
		case SC_BLADESTOP:		/* îíênéÊÇË */
			*opt3 &= ~32;
			break;
		case SC_BERSERK:		/* Éo?ÉT?ÉN */
			*opt3 &= ~128;
			break;
		case SC_MARIONETTE:		/* É}ÉäÉIÉlÉbÉgÉRÉìÉgÉç?Éã */
		case SC_MARIONETTE2:
			*opt3 &= ~1024;
			break;
		case SC_ASSUMPTIO:		/* ÉAÉXÉÄÉvÉeÉBÉI */
			*opt3 &= ~2048;
			break;
		case SC_WARM: //SG skills [Komurka]
			*opt3 &= ~4096;
			opt_flag = 1;
			break;
		}

		if(opt_flag)	/* optionÇÃ?çXÇ?Ç¶ÇÈ */
			clif_changeoption(bl);

		if (sd && calc_flag)
			status_calc_pc((struct map_session_data *)bl,0);	/* ÉXÉe?É^ÉXçƒåvéZ */
	}

	return 0;
}


/*==========================================
 * ÉXÉeÅ[É^ÉXàŸèÌèIóπÉ^ÉCÉ}Å[
 *------------------------------------------
 */
int status_change_timer(int tid, unsigned int tick, int id, int data)
{
	int type = data;
	struct block_list *bl;
	struct map_session_data *sd=NULL;
	struct status_change *sc_data;
	//short *sc_count; //égÇ¡ÇƒÇ»Ç¢ÅH

// security system to prevent forgetting timer removal
	int temp_timerid;

	bl=map_id2bl(id);
#ifndef _WIN32
	nullpo_retr_f(0, bl, "id=%d data=%d",id,data);
#endif
	nullpo_retr(0, sc_data=status_get_sc_data(bl));

	if(bl->type==BL_PC)
		nullpo_retr(0, sd=(struct map_session_data *)bl);

	//sc_count=status_get_sc_count(bl); //égÇ¡ÇƒÇ»Ç¢ÅH

	if(sc_data[type].timer != tid) {
		if(battle_config.error_log)
			ShowError("status_change_timer %d != %d\n",tid,sc_data[type].timer);
		return 0;
	}

	// security system to prevent forgetting timer removal
	// you shouldn't be that careless inside the switch here
	temp_timerid = sc_data[type].timer;
	sc_data[type].timer = -1;

	switch(type){	/* ì¡éÍÇ»?óùÇ…Ç»ÇÈèÍçá */
	case SC_MAXIMIZEPOWER:	/* É}ÉLÉVÉ}ÉCÉYÉpÉè? */
	case SC_CLOAKING:
		if(!sd || sd->status.sp > 0)
		{
			if (sd)
			{
				sd->status.sp--;
				clif_updatestatus(sd,SP_SP);
			}
			sc_data[type].timer=add_timer( /* É^ÉCÉ}?çƒê›íË */
			sc_data[type].val2+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_CHASEWALK:
		if(sd){
			int sp = 10+sc_data[SC_CHASEWALK].val1*2;
			if (map_flag_gvg(sd->bl.m)) sp *= 5;
			if (sd->status.sp > sp){
				sd->status.sp -= sp; // update sp cost [Celest]
				clif_updatestatus(sd,SP_SP);
				if ((++sc_data[SC_CHASEWALK].val4) == 1) {
					status_change_start(bl, SC_INCSTR, 1<<(sc_data[SC_CHASEWALK].val1-1), 0, 0, 0,
						(sc_data[SC_SPIRIT].timer != -1 && sc_data[SC_SPIRIT].val2 == SL_ROGUE?10:1) //SL bonus -> x10 duration
						*skill_get_time2(ST_CHASEWALK,sc_data[SC_CHASEWALK].val1), 0);
				}
				sc_data[type].timer = add_timer( /* É^ÉCÉ}?çƒê›íË */
					sc_data[type].val2+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
	break;

	case SC_HIDING:		/* ÉnÉCÉfÉBÉìÉO */
		if(sd){		/* SPÇ™Ç†Ç¡ÇƒÅAéûä‘êßå¿ÇÃä‘ÇÕéù? */
			if( sd->status.sp > 0 && (--sc_data[type].val2)>0 ){
				if(sc_data[type].val2 % (sc_data[type].val1+3) ==0 ){
					sd->status.sp--;
					clif_updatestatus(sd,SP_SP);
				}
				sc_data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
					1000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
	break;

	case SC_SIGHT:	/* ÉTÉCÉg */
	case SC_RUWACH:	/* ÉãÉAÉt */
	case SC_SIGHTBLASTER:
		{
			int range = skill_get_range2(bl, type==SC_SIGHT?MG_SIGHT:(type==SC_RUWACH?AL_RUWACH:WZ_SIGHTBLASTER), sc_data[type].val1);
			map_foreachinarea( status_change_timer_sub,
				bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,
				bl,type,tick);

			if( (--sc_data[type].val2)>0 ){
				sc_data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
					250+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SIGNUMCRUCIS:		/* ÉVÉOÉiÉÄÉNÉãÉVÉX */
		{
			int race = status_get_race(bl);
			if(race == 6 || battle_check_undead(race,status_get_elem_type(bl))) {
				sc_data[type].timer=add_timer(1000*600+tick,status_change_timer, bl->id, data );
				return 0;
			}
		}
		break;

	case SC_WARM: //SG skills [Komurka]
		if( (--sc_data[type].val2)>0){
			map_foreachinarea( status_change_timer_sub,
				bl->m, bl->x-sc_data[type].val4, bl->y-sc_data[type].val4, bl->x+sc_data[type].val4,bl->y+sc_data[type].val4,BL_CHAR,
				bl,type,tick);
			sc_data[type].timer=add_timer(tick+1000, status_change_timer,bl->id, data);
			return 0;
		}
		break;

	case SC_PROVOKE:	/* ÉvÉçÉ{ÉbÉN/ÉI?ÉgÉo?ÉT?ÉN */
		if(sc_data[type].val2!=0){	/* ÉI?ÉgÉo?ÉT?ÉNÅiÇPïbÇ≤Ç∆Ç…HPÉ`ÉFÉbÉNÅj */
			if(sd && sd->status.hp>sd->status.max_hp>>2)	/* í‚é~ */
				break;
			sc_data[type].timer=add_timer( 1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_AUTOBERSERK: //Auto Berserk continues until triggered off manually. [Skotlex]
			sc_data[type].timer=add_timer( 1000*60+tick,status_change_timer, bl->id, data );
			return 0;
			
	case SC_ENDURE:	/* ÉCÉìÉfÉÖÉA */
		if(sd && sd->special_state.infinite_endure) {
			sc_data[type].timer=add_timer( 1000*60+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_STONE:
		if(sc_data[type].val2 != 0) {
			short *opt1 = status_get_opt1(bl);
			sc_data[type].val2 = 0;
			sc_data[type].val4 = 0;
			battle_stopwalking(bl,1);
			if(opt1) {
				*opt1 = OPT1_STONE;
				clif_changeoption(bl);
			}
			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		else if( (--sc_data[type].val3) > 0) {
			int hp = status_get_max_hp(bl);
			if((++sc_data[type].val4)%5 == 0 && status_get_hp(bl) > hp>>2) {
				hp = hp/100;
				if(hp < 1) hp = 1;
				if(sd)
					pc_heal(sd,-hp,0);
				else if(bl->type == BL_MOB){
					struct mob_data *md;
					if((md=((struct mob_data *)bl)) == NULL)
						break;
					md->hp -= hp;
				}
			}
			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_POISON:
		if (status_get_hp(bl) <= status_get_max_hp(bl)>>2) //Stop damaging after 25% HP left.
			break;
	case SC_DPOISON:
		if ((--sc_data[type].val3) > 0 && sc_data[SC_SLOWPOISON].timer == -1) {
			if(sd) {
				pc_heal(sd, -sc_data[type].val4, 0);
			} else if (bl->type == BL_MOB) {
				((struct mob_data*)bl)->hp -= sc_data[type].val4;
				if (battle_config.show_mob_hp)
					clif_charnameack (0, bl);
			} else 
				battle_heal(NULL, bl, -sc_data[type].val4, 0, 1);
		}
		if (sc_data[type].val3 > 0 && !status_isdead(bl))
		{
			sc_data[type].timer = add_timer (1000 + tick, status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_TENSIONRELAX:	/* ÉeÉìÉVÉáÉìÉäÉâÉbÉNÉX */
		if(sd){		/* SPÇ™Ç†Ç¡ÇƒÅAHPÇ™?É^ÉìÇ≈Ç»ÇØÇÍÇŒ?? */
			if( sd->status.sp > 12 && sd->status.max_hp > sd->status.hp ){
				sc_data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
					10000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
			if(sd->status.max_hp <= sd->status.hp)
			{
				status_change_end(&sd->bl,SC_TENSIONRELAX,-1);
				return 0;
			}
		}
		break;
	case SC_BLEEDING:	// [celest]
		// i hope i haven't interpreted it wrong.. which i might ^^;
		// Source:
		// - 10ı©™¥™»™ÀHP™¨ ı·¥
		// - ıÛ˙Ï™Œ™ﬁ™ﬁ´µ?´–Ïπ‘—™‰´Í´Ì´∞™∑™∆™‚?Õ˝™œ·º™®™ ™§
		// To-do: bleeding effect increases damage taken?
		if ((sc_data[type].val4 -= 10000) >= 0) {
			int hp = rand()%300 + 400;
			if(sd) {
				pc_heal(sd,-hp,0);
			} else if(bl->type == BL_MOB) {
				struct mob_data *md = (struct mob_data *)bl;
				if (md) md->hp -= hp;
			}
			if (!status_isdead(bl)) {
				// walking and casting effect is lost
				battle_stopwalking (bl, 1);
				skill_castcancel (bl, 0);
				sc_data[type].timer = add_timer(10000 + tick, status_change_timer, bl->id, data );
			}
			return 0;
		}
		break;

	// Status changes that don't have a time limit
	case SC_AETERNA:
	case SC_TRICKDEAD:
	case SC_WEIGHT50:
	case SC_WEIGHT90:
	case SC_MAGICPOWER:
	case SC_REJECTSWORD:
	case SC_MEMORIZE:
	case SC_BROKENWEAPON:
	case SC_BROKENARMOR:
	case SC_SACRIFICE:
	case SC_READYSTORM:
	case SC_READYDOWN:
	case SC_READYTURN:
	case SC_READYCOUNTER:
	case SC_RUN:
	case SC_DODGE:
		sc_data[type].timer=add_timer( 1000*600+tick,status_change_timer, bl->id, data );
		return 0;

	case SC_DANCING: //É_ÉìÉXÉXÉLÉãÇÃéûä‘SPè¡îÔ
		{
			int s = 0;
			int sp = 1;
			if(sd && (--sc_data[type].val3) > 0) {
				switch(sc_data[type].val1){
				case BD_RICHMANKIM:				/* ÉjÉàÉãÉhÇÃâÉ 3ïbÇ…SP1 */
				case BD_DRUMBATTLEFIELD:		/* ?ëæå€ÇÃãøÇ´ 3ïbÇ…SP1 */
				case BD_RINGNIBELUNGEN:			/* Éj?ÉxÉãÉìÉOÇÃéwó÷ 3ïbÇ…SP1 */
				case BD_SIEGFRIED:				/* ïséÄêgÇÃÉW?ÉNÉtÉä?Éh 3ïbÇ…SP1 */
				case BA_DISSONANCE:				/* ïsã¶òaâπ 3ïbÇ≈SP1 */
				case BA_ASSASSINCROSS:			/* ó[ózÇÃÉAÉTÉVÉìÉNÉçÉX 3ïbÇ≈SP1 */
				case DC_UGLYDANCE:				/* é©ï™èüéËÇ»É_ÉìÉX 3ïbÇ≈SP1 */
					s=3;
					break;
				case BD_LULLABY:				/* éqéÁâÃ 4ïbÇ…SP1 */
				case BD_ETERNALCHAOS:			/* âiâìÇÃç¨ì◊ 4ïbÇ…SP1 */
				case BD_ROKISWEIL:				/* ÉçÉLÇÃã©Ç— 4ïbÇ…SP1 */
				case DC_FORTUNEKISS:			/* çKâ^ÇÃÉLÉX 4ïbÇ≈SP1 */
					s=4;
					break;
				case CG_HERMODE:				// Wand of Hermod
					sp=5;	//Upkeep = 5
				case BD_INTOABYSS:				/* ê[ï£ÇÃíÜÇ… 5ïbÇ…SP1 */
				case BA_WHISTLE:				/* å˚ìJ 5ïbÇ≈SP1 */
				case DC_HUMMING:				/* ÉnÉ~ÉìÉO 5ïbÇ≈SP1 */
				case BA_POEMBRAGI:				/* ÉuÉâÉMÇÃéç 5ïbÇ≈SP1 */
				case DC_SERVICEFORYOU:			/* ÉT?ÉrÉXÉtÉH?ÉÜ? 5ïbÇ≈SP1 */
					s=5;
					break;
				case BA_APPLEIDUN:				/* ÉCÉhÉDÉìÇÃó—åÁ 6ïbÇ≈SP1 */
					s=6;
					break;
				case CG_MOONLIT:				/* åéñæÇËÇÃêÚÇ…óéÇøÇÈâ‘Ç—ÇÁ 10ïbÇ≈SP1ÅH */
					sp= 4*sc_data[type].val2; //Moonlit's cost is 4sp*skill_lv [Skotlex]
					//Upkeep is also every 10 secs.
				case DC_DONTFORGETME:			/* éÑÇñYÇÍÇ»Ç¢Ç≈Åc 10ïbÇ≈SP1 */
					s=10;
					break;
				}
				if (s && ((sc_data[type].val3 % s) == 0)) {
					if (sc_data[SC_LONGING].timer != -1)
						sp = s;
					if (sp > sd->status.sp)
						sp = sd->status.sp;
					sd->status.sp -= sp;
					clif_updatestatus(sd,SP_SP);
					if (sd->status.sp <= 0)
						break;
				}
				sc_data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
					1000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;

	case SC_DEVOTION:
		{	//Check range and timeleft to preserve status [Skotlex]
			//This implementation won't work for mobs because of map_id2sd, but it's a small cost in exchange of the speed of map_id2sd over map_id2sd
			struct map_session_data *md = map_id2sd(sc_data[type].val1);
			if (md && battle_check_range(bl, &md->bl, sc_data[type].val3) && (sc_data[type].val4-=1000)>0)
			{
				sc_data[type].timer = add_timer(1000+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;
		
	case SC_BERSERK:		/* Éo?ÉT?ÉN */
		if(sd){		/* HPÇ™100à»è„Ç»ÇÁ?? */
			if( (sd->status.hp - sd->status.max_hp*5/100) > 100 ){	// 5% every 10 seconds [DracoRPG]
				sd->status.hp -= sd->status.max_hp*5/100;	// changed to max hp [celest]
				clif_updatestatus(sd,SP_HP);
				sc_data[type].timer = add_timer(	/* É^ÉCÉ}?çƒê›íË */
					10000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
			else
				sd->canregen_tick = gettick() + 300000;
		}
		break;
	case SC_NOCHAT:	//É`ÉÉÉbÉgã÷é~?ë‘
		if(sd && battle_config.manner_system){
			sd->status.manner++;
			clif_updatestatus(sd,SP_MANNER);
			if (sd->status.manner < 0)
			{	//Every 60 seconds your manner goes up by 1 until it gets back to 0.
				sc_data[type].timer=add_timer(60000+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SPLASHER:
		if (sc_data[type].val4 % 1000 == 0) {
			char timer[2];
			sprintf (timer, "%d", sc_data[type].val4/1000);
			clif_message(bl, timer);
		}
		if((sc_data[type].val4 -= 500) > 0) {
			sc_data[type].timer = add_timer(
				500 + tick, status_change_timer,
				bl->id, data);
				return 0;
		}
		break;

	case SC_MARIONETTE:		/* É}ÉäÉIÉlÉbÉgÉRÉìÉgÉç?Éã */
	case SC_MARIONETTE2:
		{
			struct block_list *pbl = map_id2bl(sc_data[type].val3);
			if (pbl && battle_check_range(bl, pbl, 7) &&
				(sc_data[type].val2 -= 1000)>0) {
				sc_data[type].timer = add_timer(
					1000 + tick, status_change_timer,
					bl->id, data);
					return 0;
			}
		}
		break;

	case SC_GOSPEL:
		if(sc_data[type].val4 == BCT_SELF){
			int hp, sp;
			hp = (sc_data[type].val1 > 5) ? 45 : 30;
			sp = (sc_data[type].val1 > 5) ? 35 : 20;
			if(status_get_hp(bl) - hp > 0 &&
				(sd == NULL || sd->status.sp - sp> 0))
			{
				if (sd)
					pc_heal(sd,-hp,-sp);
				else if (bl->type == BL_MOB)
					mob_heal((struct mob_data *)bl,-hp);
					
				if ((sc_data[type].val2 -= 10000) > 0) {
					sc_data[type].timer = add_timer(
					10000+tick, status_change_timer,
						bl->id, data);
					return 0;
				}
			}
		}
		break;
		
	case SC_GUILDAURA:
		{
			struct block_list *tbl = map_id2bl(sc_data[type].val2);
			
			if (tbl && battle_check_range(bl, tbl, 2)){
				sc_data[type].timer = add_timer(
					1000 + tick, status_change_timer,
					bl->id, data);
					return 0;
			}
		}
		break;
	}

	// default for all non-handled control paths
	// security system to prevent forgetting timer removal

	// if we reach this point we need the timer for the next call, 
	// so restore it to have status_change_end handle a valid timer
	sc_data[type].timer = temp_timerid; 

	return status_change_end( bl,type,tid );
}

/*==========================================
 * ÉXÉeÅ[É^ÉXàŸèÌÉ^ÉCÉ}Å[îÕàÕèàóù
 *------------------------------------------
 */
int status_change_timer_sub(struct block_list *bl, va_list ap )
{
	struct block_list *src;
	struct map_session_data* sd=NULL;
	struct map_session_data* tsd=NULL;

	int type;
	unsigned int tick;

	src=va_arg(ap,struct block_list*);
	type=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);

	if (status_isdead(bl))
		return 0;
	if (src->type==BL_PC) sd= (struct map_session_data*)src;
	if (bl->type==BL_PC) tsd= (struct map_session_data*)bl;

	switch( type ){
	case SC_SIGHT:	/* ÉTÉCÉg */
	case SC_CONCENTRATE:
		if( (*status_get_option(bl))&(OPTION_HIDE|OPTION_CLOAK) ){
			status_change_end( bl, SC_HIDING, -1);
			status_change_end( bl, SC_CLOAKING, -1);
		}
		break;
	case SC_RUWACH:	/* ÉãÉAÉt */
		if( (*status_get_option(bl))&(OPTION_HIDE|OPTION_CLOAK) ){
			struct status_change *sc_data = status_get_sc_data(bl);	// check whether the target is hiding/cloaking [celest]
			if (sc_data && (sc_data[SC_HIDING].timer != -1 ||	// if the target is using a special hiding, i.e not using normal hiding/cloaking, don't bother
				sc_data[SC_CLOAKING].timer != -1)) {
				status_change_end( bl, SC_HIDING, -1);
				status_change_end( bl, SC_CLOAKING, -1);
				if(battle_check_target( src, bl, BCT_ENEMY ) > 0)
					skill_attack(BF_MAGIC,src,src,bl,AL_RUWACH,1,tick,0);
			}
		}
		break;
	case SC_SIGHTBLASTER:
		{
			struct status_change *sc_data = status_get_sc_data(src);
			if (sc_data && sc_data[type].val2 > 0 && battle_check_target( src, bl, BCT_ENEMY ) > 0)
			{	//sc_ check prevents a single round of Sight Blaster hitting multiple opponents. [Skotlex]
				skill_attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,1,tick,0);
				sc_data[type].val2 = 0; //This signals it to end.
			}
		}
		break;
	case SC_WARM: //SG skills [Komurka]
		{
			if(battle_check_target( src,bl, BCT_ENEMY ) > 0) {
				struct status_change *sc_data = status_get_sc_data(src);
				if(sd){
					if(sd->status.sp<2) {
						sd->sc_data[type].val2 = 0; //Makes it end on the next tick.
						break;
					}
					sd->status.sp -= 2;
					clif_updatestatus(sd,SP_SP);	
				}
				skill_attack(BF_WEAPON,src,src,bl,sc_data?sc_data[type].val3:SG_SUN_WARM,1,tick,0);
			}
		}
		break;
	case SC_CLOSECONFINE:
		{	//Lock char has released the hold on everyone...
			struct status_change *sc_data = status_get_sc_data(bl);
			if (sc_data && sc_data[SC_CLOSECONFINE2].timer != -1 && sc_data[SC_CLOSECONFINE2].val2 == src->id) {
				sc_data[SC_CLOSECONFINE2].val2 = 0;
				status_change_end(bl, SC_CLOSECONFINE2, -1);
			}
		}
		break;
	}
	return 0;
}

int status_change_clear_buffs (struct block_list *bl)
{
	int i;
	struct status_change *sc_data = status_get_sc_data(bl);
	if (!sc_data)
		return 0;		
	for (i = 20; i < SC_MAX; i++) {
		if(i==SC_HALLUCINATION || i==SC_WEIGHT50 || i==SC_WEIGHT90
			|| i == SC_QUAGMIRE || i == SC_SIGNUMCRUCIS || i == SC_DECREASEAGI 
			|| i == SC_SLOWDOWN || i == SC_ANKLE|| i == SC_BLADESTOP
			|| i == SC_MINDBREAKER || i == SC_WINKCHARM 
			|| i == SC_STOP || i == SC_NOCHAT || i == SC_ORCISH
			|| i == SC_STRIPWEAPON || i == SC_STRIPSHIELD || i == SC_STRIPARMOR || i == SC_STRIPHELM
			|| i == SC_COMBO || i == SC_DANCING || i == SC_GUILDAURA
			)
			continue;
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	return 0;
}
int status_change_clear_debuffs (struct block_list *bl)
{
	int i;
	struct status_change *sc_data = status_get_sc_data(bl);
	if (!sc_data)
		return 0;
	for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	//Other ailments not in the common range.
	if(sc_data[SC_HALLUCINATION].timer != -1)
		status_change_end(bl,SC_HALLUCINATION,-1);
	if(sc_data[SC_QUAGMIRE].timer != -1)
		status_change_end(bl,SC_QUAGMIRE,-1);
	if(sc_data[SC_SIGNUMCRUCIS].timer != -1)
		status_change_end(bl,SC_SIGNUMCRUCIS,-1);
	if(sc_data[SC_DECREASEAGI].timer != -1)
		status_change_end(bl,SC_DECREASEAGI,-1);
	if(sc_data[SC_SLOWDOWN].timer != -1)
		status_change_end(bl,SC_SLOWDOWN,-1);
	if(sc_data[SC_MINDBREAKER].timer != -1)
		status_change_end(bl,SC_MINDBREAKER,-1);
	if(sc_data[SC_WINKCHARM].timer != -1)
		status_change_end(bl,SC_WINKCHARM,-1);
	if(sc_data[SC_STOP].timer != -1)
		status_change_end(bl,SC_STOP,-1);
	if(sc_data[SC_ORCISH].timer != -1)
		status_change_end(bl,SC_ORCISH,-1);
	if(sc_data[SC_STRIPWEAPON].timer != -1)
		status_change_end(bl,SC_STRIPWEAPON,-1);
	if(sc_data[SC_STRIPSHIELD].timer != -1)
		status_change_end(bl,SC_STRIPSHIELD,-1);
	if(sc_data[SC_STRIPARMOR].timer != -1)
		status_change_end(bl,SC_STRIPARMOR,-1);
	if(sc_data[SC_STRIPHELM].timer != -1)
		status_change_end(bl,SC_STRIPHELM,-1);
	return 0;
}

static int status_calc_sigma(void)
{
	int i,j,k;

	for(i=0;i<MAX_PC_CLASS;i++) {
		memset(hp_sigma_val[i],0,sizeof(hp_sigma_val[i]));
		for(k=0,j=2;j<=MAX_LEVEL;j++) {
			k += hp_coefficient[i]*j + 50;
			k -= k%100;
			hp_sigma_val[i][j-1] = k;
		}
	}
	return 0;
}

int status_readdb(void) {
	int i,j;
	FILE *fp;
	char line[1024], path[1024],*p;

	sprintf(path, "%s/job_db1.txt", db_path);
	fp=fopen(path,"r"); // Job-specific values (weight, HP, SP, ASPD)
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	while(fgets(line, sizeof(line)-1, fp)){
		char *split[23];
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<22 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j<22)
			continue;
       	if(atoi(split[0])>=MAX_PC_CLASS)
		    continue;
		max_weight_base[atoi(split[0])]=atoi(split[1]);
		hp_coefficient[atoi(split[0])]=atoi(split[2]);
		hp_coefficient2[atoi(split[0])]=atoi(split[3]);
		sp_coefficient[atoi(split[0])]=atoi(split[4]);
		for(j=0;j<17;j++)
			aspd_base[atoi(split[0])][j]=atoi(split[j+5]);
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","job_db1.txt");

	memset(job_bonus,0,sizeof(job_bonus)); // Job-specific stats bonus
	sprintf(path, "%s/job_db2.txt", db_path);
	fp=fopen(path,"r");
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	while(fgets(line, sizeof(line)-1, fp)){
       	char *split[MAX_LEVEL+1]; //Job Level is limited to MAX_LEVEL, so the bonuses should likewise be limited to it. [Skotlex]
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<MAX_LEVEL+1 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(atoi(split[0])>=MAX_PC_CLASS)
		    continue;
		for(i=1;i<j && split[i];i++)
			job_bonus[atoi(split[0])][i-1]=atoi(split[i]);
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",path);

	// ÉTÉCÉYï‚ê≥Ée?ÉuÉã
	for(i=0;i<3;i++)
		for(j=0;j<20;j++)
			atkmods[i][j]=100;
	sprintf(path, "%s/size_fix.txt", db_path);
	fp=fopen(path,"r");
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		char *split[20];
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<20 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		for(j=0;j<20 && split[j];j++)
			atkmods[i][j]=atoi(split[j]);
		i++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",path);

	// ê∏?Éf?É^Ée?ÉuÉã
	for(i=0;i<5;i++){
		for(j=0;j<MAX_REFINE; j++)
			percentrefinery[i][j]=100;
		percentrefinery[i][j]=0; //Slot MAX+1 always has 0% success chance [Skotlex]
		refinebonus[i][0]=0;
		refinebonus[i][1]=0;
		refinebonus[i][2]=10;
	}

	sprintf(path, "%s/refine_db.txt", db_path);
	fp=fopen(path,"r");
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		char *split[16];
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<16 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		refinebonus[i][0]=atoi(split[0]);	// ê∏?É{?ÉiÉX
		refinebonus[i][1]=atoi(split[1]);	// âﬂ?ê∏?É{?ÉiÉX
		refinebonus[i][2]=atoi(split[2]);	// à¿ëSê∏?å¿äE
		for(j=0;j<MAX_REFINE && split[j];j++)
			percentrefinery[i][j]=atoi(split[j+3]);
		i++;
	}
	fclose(fp); //Lupus. close this file!!!
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",path);

	return 0;
}

/*==========================================
 * ÉXÉLÉãä÷åWèâä˙âªèàóù
 *------------------------------------------
 */
int do_init_status(void)
{
	if (SC_MAX > MAX_STATUSCHANGE)
	{
		ShowDebug("status.h defines %d status changes, but the MAX_STATUSCHANGE is %d! Fix it.\n", SC_MAX, MAX_STATUSCHANGE);
		exit(1);
	}
	add_timer_func_list(status_change_timer,"status_change_timer");
	initStatusIconChangeTable();
	status_readdb();
	status_calc_sigma();
	return 0;
}
