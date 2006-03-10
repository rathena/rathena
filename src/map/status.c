// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <limits.h>

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

int SkillStatusChangeTable[MAX_SKILL]; //Stores the status that should be associated to this skill.
int StatusIconChangeTable[SC_MAX]; //Stores the icon that should be associated to this status change.
int StatusSkillChangeTable[SC_MAX]; //Stores the skill that should be considered associated to this status change. 

static int max_weight_base[MAX_PC_CLASS];
static int hp_coefficient[MAX_PC_CLASS];
static int hp_coefficient2[MAX_PC_CLASS];
static int hp_sigma_val[MAX_PC_CLASS][MAX_LEVEL];
static int sp_coefficient[MAX_PC_CLASS];
static int aspd_base[MAX_PC_CLASS][MAX_WEAPON_TYPE+1];	//[blackhole89]
#define MAX_REFINE_BONUS 5
static int refinebonus[MAX_REFINE_BONUS][3];	// ê∏òBÉ{Å[ÉiÉXÉeÅ[ÉuÉã(refine_db.txt)
int percentrefinery[5][MAX_REFINE+1];	// ê∏òBê¨å˜ó¶(refine_db.txt)
static int atkmods[3][MAX_WEAPON_TYPE+1];	// ïêäÌATKÉTÉCÉYèCê≥(size_fix.txt)
static char job_bonus[MAX_PC_CLASS][MAX_LEVEL];

int current_equip_item_index; //Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
int current_equip_card_id; //To prevent card-stacking (from jA) [Skotlex]
//we need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only
//to avoid cards exploits

//Initializes the StatusIconChangeTable variable. May seem somewhat slower than directly defining the array,
//but it is much less prone to errors. [Skotlex]
void initChangeTables(void) {
	int i;
	for (i = 0; i < SC_MAX; i++)
		StatusIconChangeTable[i] = SI_BLANK;
	for (i = 0; i < MAX_SKILL; i++)
		SkillStatusChangeTable[i] = -1;
	memset(StatusSkillChangeTable, 0, sizeof(StatusSkillChangeTable));

	//First we define the skill for common ailments. These are used in 
	//skill_additional_effect through sc cards. [Skotlex]
	StatusSkillChangeTable[SC_STONE] =     MG_STONECURSE;
	StatusSkillChangeTable[SC_FREEZE] =    MG_FROSTDIVER;
	StatusSkillChangeTable[SC_STUN] =      NPC_STUNATTACK;
	StatusSkillChangeTable[SC_SLEEP] =     NPC_SLEEPATTACK;
	StatusSkillChangeTable[SC_POISON] =    NPC_POISON;
	StatusSkillChangeTable[SC_CURSE] =     NPC_CURSEATTACK;
	StatusSkillChangeTable[SC_SILENCE] =   NPC_SILENCEATTACK;
	StatusSkillChangeTable[SC_CONFUSION] = DC_WINKCHARM;
	StatusSkillChangeTable[SC_BLIND] =     NPC_BLINDATTACK;
	StatusSkillChangeTable[SC_BLEEDING] =  LK_HEADCRUSH;
	StatusSkillChangeTable[SC_DPOISON] =   NPC_POISON;

#define set_sc(skill, sc, icon) \
	if (SkillStatusChangeTable[skill]==-1) SkillStatusChangeTable[skill] = sc; \
	if (StatusSkillChangeTable[sc]==0) StatusSkillChangeTable[sc] = skill; \
	if (StatusIconChangeTable[sc]==SI_BLANK) StatusIconChangeTable[sc] = icon; 
	
	set_sc(SM_BASH,                 SC_STUN,                SI_BLANK);
	set_sc(SM_PROVOKE,              SC_PROVOKE,             SI_PROVOKE);
	set_sc(SM_MAGNUM,               SC_WATK_ELEMENT,        SI_BLANK);
	set_sc(SM_ENDURE,               SC_ENDURE,              SI_ENDURE);
	set_sc(MG_SIGHT,                SC_SIGHT,               SI_BLANK);
	set_sc(MG_SAFETYWALL,           SC_SAFETYWALL,          SI_BLANK);
	set_sc(MG_FROSTDIVER,           SC_FREEZE,              SI_BLANK);
	set_sc(MG_STONECURSE,           SC_STONE,               SI_BLANK);
	set_sc(AL_RUWACH,               SC_RUWACH,              SI_BLANK);
	set_sc(AL_INCAGI,               SC_INCREASEAGI,         SI_INCREASEAGI);
	set_sc(AL_DECAGI,               SC_DECREASEAGI,         SI_DECREASEAGI);
	set_sc(AL_CRUCIS,               SC_SIGNUMCRUCIS,        SI_SIGNUMCRUCIS);
	set_sc(AL_ANGELUS,              SC_ANGELUS,             SI_ANGELUS);
	set_sc(AL_BLESSING,             SC_BLESSING,            SI_BLESSING);
	set_sc(AC_CONCENTRATION,        SC_CONCENTRATE,         SI_CONCENTRATE);
	set_sc(TF_HIDING,               SC_HIDING,              SI_HIDING);
	set_sc(TF_POISON,               SC_POISON,              SI_BLANK);
	set_sc(KN_TWOHANDQUICKEN,       SC_TWOHANDQUICKEN,      SI_TWOHANDQUICKEN);
	set_sc(KN_AUTOCOUNTER,          SC_AUTOCOUNTER,         SI_BLANK);
	set_sc(PR_IMPOSITIO,            SC_IMPOSITIO,           SI_IMPOSITIO);
	set_sc(PR_SUFFRAGIUM,           SC_SUFFRAGIUM,          SI_SUFFRAGIUM);
	set_sc(PR_ASPERSIO,             SC_ASPERSIO,            SI_ASPERSIO);
	set_sc(PR_BENEDICTIO,           SC_BENEDICTIO,          SI_BENEDICTIO);
	set_sc(PR_SLOWPOISON,           SC_SLOWPOISON,          SI_SLOWPOISON);
	set_sc(PR_KYRIE,                SC_KYRIE,	              SI_KYRIE);
	set_sc(PR_MAGNIFICAT,           SC_MAGNIFICAT,          SI_MAGNIFICAT);
	set_sc(PR_GLORIA,               SC_GLORIA,              SI_GLORIA);
	set_sc(PR_LEXDIVINA,            SC_SILENCE,	           SI_BLANK);
	set_sc(PR_LEXAETERNA,           SC_AETERNA,             SI_AETERNA);
	set_sc(WZ_METEOR,               SC_STUN,                SI_BLANK);
	set_sc(WZ_VERMILION,            SC_BLIND,               SI_BLANK);
	set_sc(WZ_FROSTNOVA,            SC_FREEZE,              SI_BLANK);
	set_sc(WZ_STORMGUST,            SC_FREEZE,              SI_BLANK);
	set_sc(WZ_QUAGMIRE,             SC_QUAGMIRE,            SI_QUAGMIRE);
	set_sc(BS_ADRENALINE,           SC_ADRENALINE,          SI_ADRENALINE);
	set_sc(BS_WEAPONPERFECT,        SC_WEAPONPERFECTION,    SI_WEAPONPERFECTION);
	set_sc(BS_OVERTHRUST,           SC_OVERTHRUST,          SI_OVERTHRUST);
	set_sc(BS_MAXIMIZE,             SC_MAXIMIZEPOWER,       SI_MAXIMIZEPOWER);
	set_sc(HT_LANDMINE,             SC_STUN,                SI_BLANK);
	set_sc(HT_ANKLESNARE,           SC_ANKLE,               SI_BLANK);
	set_sc(HT_SANDMAN,              SC_SLEEP,               SI_BLANK);
	set_sc(HT_FLASHER,              SC_BLIND,               SI_BLANK);
	set_sc(HT_FREEZINGTRAP,         SC_FREEZE,              SI_BLANK);
	set_sc(AS_CLOAKING,             SC_CLOAKING,	           SI_CLOAKING);
	set_sc(AS_SONICBLOW,            SC_STUN,	              SI_BLANK);
	set_sc(AS_GRIMTOOTH,            SC_SLOWDOWN,            SI_BLANK);
	set_sc(AS_ENCHANTPOISON,        SC_ENCPOISON,	        SI_ENCPOISON);
	set_sc(AS_POISONREACT,          SC_POISONREACT,         SI_POISONREACT);
	set_sc(AS_VENOMDUST,            SC_POISON,              SI_BLANK);
	set_sc(AS_SPLASHER,             SC_SPLASHER,            SI_BLANK);
	set_sc(NV_TRICKDEAD,            SC_TRICKDEAD,           SI_TRICKDEAD);
	set_sc(SM_AUTOBERSERK,          SC_AUTOBERSERK,         SI_BLANK);
	set_sc(TF_SPRINKLESAND,         SC_BLIND,               SI_BLANK);
	set_sc(TF_THROWSTONE,           SC_STUN,                SI_BLANK);
	set_sc(MC_LOUD,                 SC_LOUD,                SI_LOUD);
	set_sc(MG_ENERGYCOAT,           SC_ENERGYCOAT,          SI_ENERGYCOAT);
	set_sc(NPC_POISON,              SC_POISON,              SI_BLANK);
	set_sc(NPC_BLINDATTACK,         SC_BLIND,               SI_BLANK);
	set_sc(NPC_SILENCEATTACK,       SC_SILENCE,             SI_BLANK);
	set_sc(NPC_STUNATTACK,          SC_STUN,                SI_BLANK);
	set_sc(NPC_PETRIFYATTACK,       SC_STONE,               SI_BLANK);
	set_sc(NPC_CURSEATTACK,         SC_CURSE,               SI_BLANK);
	set_sc(NPC_SLEEPATTACK,         SC_SLEEP,               SI_BLANK);
	set_sc(NPC_KEEPING,             SC_KEEPING,             SI_BLANK);
	set_sc(NPC_DARKBLESSING,        SC_COMA,                SI_BLANK);
	set_sc(NPC_BARRIER,             SC_BARRIER,             SI_BLANK);
	set_sc(NPC_LICK,                SC_STUN,                SI_BLANK);
	set_sc(NPC_HALLUCINATION,       SC_HALLUCINATION,       SI_HALLUCINATION);
	set_sc(NPC_REBIRTH,             SC_KAIZEL,              SI_KAIZEL);
	set_sc(RG_RAID,                 SC_STUN,                SI_BLANK);
	set_sc(RG_STRIPWEAPON,          SC_STRIPWEAPON,         SI_STRIPWEAPON);
	set_sc(RG_STRIPSHIELD,          SC_STRIPSHIELD,         SI_STRIPSHIELD);
	set_sc(RG_STRIPARMOR,           SC_STRIPARMOR,          SI_STRIPARMOR);
	set_sc(RG_STRIPHELM,            SC_STRIPHELM,           SI_STRIPHELM);
	set_sc(AM_ACIDTERROR,           SC_BLEEDING,            SI_BLEEDING);
	set_sc(AM_CP_WEAPON,            SC_CP_WEAPON,           SI_CP_WEAPON);
	set_sc(AM_CP_SHIELD,            SC_CP_SHIELD,           SI_CP_SHIELD);
	set_sc(AM_CP_ARMOR,             SC_CP_ARMOR,            SI_CP_ARMOR);
	set_sc(AM_CP_HELM,              SC_CP_HELM,             SI_CP_HELM);
	set_sc(CR_AUTOGUARD,            SC_AUTOGUARD,           SI_AUTOGUARD);
	set_sc(CR_SHIELDCHARGE,         SC_STUN,                SI_AUTOGUARD);
	set_sc(CR_REFLECTSHIELD,        SC_REFLECTSHIELD,       SI_REFLECTSHIELD);
	set_sc(CR_HOLYCROSS,            SC_BLIND,               SI_BLANK);
	set_sc(CR_GRANDCROSS,           SC_BLIND,               SI_BLANK);
	set_sc(CR_DEVOTION,             SC_DEVOTION,            SI_DEVOTION);
	set_sc(CR_PROVIDENCE,           SC_PROVIDENCE,          SI_PROVIDENCE);
	set_sc(CR_DEFENDER,             SC_DEFENDER,            SI_DEFENDER);
	set_sc(CR_SPEARQUICKEN,         SC_SPEARSQUICKEN,       SI_SPEARQUICKEN);
	set_sc(MO_STEELBODY,            SC_STEELBODY,           SI_STEELBODY);
	set_sc(MO_BLADESTOP,            SC_BLADESTOP_WAIT,      SI_BLANK);
	set_sc(MO_EXPLOSIONSPIRITS,     SC_EXPLOSIONSPIRITS,    SI_EXPLOSIONSPIRITS);
	set_sc(MO_EXTREMITYFIST,        SC_EXTREMITYFIST,       SI_BLANK);
	set_sc(SA_MAGICROD,             SC_MAGICROD,            SI_BLANK);
	set_sc(SA_AUTOSPELL,            SC_AUTOSPELL,           SI_AUTOSPELL);
	set_sc(SA_FLAMELAUNCHER,        SC_FIREWEAPON,          SI_FIREWEAPON);
	set_sc(SA_FROSTWEAPON,          SC_WATERWEAPON,         SI_WATERWEAPON);
	set_sc(SA_LIGHTNINGLOADER,      SC_WINDWEAPON,          SI_WINDWEAPON);
	set_sc(SA_SEISMICWEAPON,        SC_EARTHWEAPON,         SI_EARTHWEAPON);
	set_sc(SA_VOLCANO,              SC_VOLCANO,             SI_BLANK);
	set_sc(SA_DELUGE,               SC_DELUGE,              SI_BLANK);
	set_sc(SA_VIOLENTGALE,          SC_VIOLENTGALE,         SI_BLANK);
	set_sc(SA_LANDPROTECTOR,        SC_LANDPROTECTOR,       SI_BLANK);
	set_sc(SA_REVERSEORCISH,        SC_ORCISH,              SI_BLANK);
	set_sc(SA_COMA,                 SC_COMA,                SI_BLANK);
	set_sc(BD_LULLABY,              SC_LULLABY,             SI_BLANK);
	set_sc(BD_RICHMANKIM,           SC_RICHMANKIM,          SI_BLANK);
	set_sc(BD_ETERNALCHAOS,         SC_ETERNALCHAOS,        SI_BLANK);
	set_sc(BD_DRUMBATTLEFIELD,      SC_DRUMBATTLE,          SI_BLANK);
	set_sc(BD_RINGNIBELUNGEN,       SC_NIBELUNGEN,          SI_BLANK);
	set_sc(BD_ROKISWEIL,            SC_ROKISWEIL,           SI_BLANK);
	set_sc(BD_INTOABYSS,            SC_INTOABYSS,           SI_BLANK);
	set_sc(BD_SIEGFRIED,            SC_SIEGFRIED,           SI_BLANK);
	set_sc(BA_FROSTJOKE,            SC_FREEZE,              SI_BLANK);
	set_sc(BA_WHISTLE,              SC_WHISTLE,             SI_BLANK);
	set_sc(BA_ASSASSINCROSS,        SC_ASSNCROS,            SI_BLANK);
	set_sc(BA_POEMBRAGI,            SC_POEMBRAGI,           SI_BLANK);
	set_sc(BA_APPLEIDUN,            SC_APPLEIDUN,           SI_BLANK);
	set_sc(DC_UGLYDANCE,            SC_UGLYDANCE,           SI_BLANK);
	set_sc(DC_SCREAM,               SC_STUN,                SI_BLANK);
	set_sc(DC_HUMMING,              SC_HUMMING,             SI_BLANK);
	set_sc(DC_DONTFORGETME,         SC_DONTFORGETME,        SI_BLANK);
	set_sc(DC_FORTUNEKISS,          SC_FORTUNE,             SI_BLANK);
	set_sc(DC_SERVICEFORYOU,        SC_SERVICE4U,           SI_BLANK);
	set_sc(NPC_DARKCROSS,           SC_BLIND,               SI_BLANK);
	set_sc(NPC_GRANDDARKNESS,       SC_BLIND,               SI_BLANK);
	set_sc(NPC_STOP,                SC_STOP,                SI_BLANK);
	set_sc(NPC_BREAKWEAPON,         SC_BROKENWEAPON,        SI_BROKENWEAPON);
	set_sc(NPC_BREAKARMOR,          SC_BROKENARMOR,         SI_BROKENARMOR);
	set_sc(LK_AURABLADE,            SC_AURABLADE,           SI_AURABLADE);
	set_sc(LK_PARRYING,             SC_PARRYING,            SI_PARRYING);
	set_sc(LK_CONCENTRATION,        SC_CONCENTRATION,       SI_CONCENTRATION);
	set_sc(LK_TENSIONRELAX,         SC_TENSIONRELAX,        SI_TENSIONRELAX);
	set_sc(LK_BERSERK,              SC_BERSERK,             SI_BERSERK);
	set_sc(LK_FURY,                 SC_FURY,                SI_FURY);
	set_sc(HP_ASSUMPTIO,            SC_ASSUMPTIO,           SI_ASSUMPTIO);
	set_sc(HP_BASILICA,             SC_BASILICA,            SI_BLANK);
	set_sc(HW_MAGICPOWER,           SC_MAGICPOWER,          SI_MAGICPOWER);
	set_sc(PA_SACRIFICE,            SC_SACRIFICE,           SI_BLANK);
	set_sc(PA_GOSPEL,               SC_GOSPEL,              SI_BLANK);
	set_sc(CH_TIGERFIST,            SC_STOP,                SI_BLANK);
	set_sc(ASC_EDP,                 SC_EDP,                 SI_EDP);
	set_sc(SN_SIGHT,                SC_TRUESIGHT,           SI_TRUESIGHT);
	set_sc(SN_WINDWALK,             SC_WINDWALK,            SI_WINDWALK);
	set_sc(WS_MELTDOWN,             SC_MELTDOWN,            SI_MELTDOWN);
	set_sc(WS_CARTBOOST,            SC_CARTBOOST,           SI_CARTBOOST);
	set_sc(ST_CHASEWALK,            SC_CHASEWALK,           SI_CHASEWALK);
	set_sc(ST_REJECTSWORD,          SC_REJECTSWORD,         SI_REJECTSWORD);
	set_sc(ST_REJECTSWORD,          SC_AUTOCOUNTER,         SI_BLANK);
	set_sc(CG_MOONLIT,              SC_MOONLIT,             SI_MOONLIT);
	set_sc(CG_MARIONETTE,           SC_MARIONETTE,          SI_MARIONETTE);
	set_sc(CG_MARIONETTE,           SC_MARIONETTE2,         SI_MARIONETTE2);
	set_sc(LK_SPIRALPIERCE,         SC_STOP,                SI_BLANK);
	set_sc(LK_HEADCRUSH,            SC_BLEEDING,            SI_BLEEDING);
	set_sc(LK_JOINTBEAT,            SC_JOINTBEAT,           SI_BLANK);
	set_sc(HW_NAPALMVULCAN,         SC_CURSE,               SI_BLANK);
	set_sc(PF_MINDBREAKER,          SC_MINDBREAKER,         SI_BLANK);
	set_sc(PF_MEMORIZE,             SC_MEMORIZE,            SI_BLANK);
	set_sc(PF_FOGWALL,              SC_FOGWALL,             SI_BLANK);
	set_sc(PF_SPIDERWEB,            SC_SPIDERWEB,           SI_BLANK);
	set_sc(WE_BABY,                 SC_BABY,                SI_BLANK);
	set_sc(TK_RUN,                  SC_RUN,                 SI_BLANK);
	set_sc(TK_RUN,                  SC_SPURT,               SI_SPURT);
	set_sc(TK_READYSTORM,           SC_READYSTORM,          SI_READYSTORM);
	set_sc(TK_READYDOWN,            SC_READYDOWN,           SI_READYDOWN);
	set_sc(TK_DOWNKICK,             SC_STUN,                SI_BLANK);
	set_sc(TK_READYTURN,            SC_READYTURN,           SI_READYTURN);
	set_sc(TK_READYCOUNTER,         SC_READYCOUNTER,        SI_READYCOUNTER);
	set_sc(TK_DODGE,                SC_DODGE,               SI_DODGE);
	set_sc(TK_SPTIME,               SC_TKDORI,              SI_BLANK);
	set_sc(TK_SEVENWIND,            SC_GHOSTWEAPON,         SI_GHOSTWEAPON);
	set_sc(TK_SEVENWIND,            SC_SHADOWWEAPON,        SI_SHADOWWEAPON);
	set_sc(SG_SUN_WARM,             SC_WARM,                SI_WARM);
	set_sc(SG_MOON_WARM,            SC_WARM,                SI_WARM);
	set_sc(SG_STAR_WARM,            SC_WARM,                SI_WARM);
	set_sc(SG_SUN_COMFORT,          SC_SUN_COMFORT,         SI_SUN_COMFORT);
	set_sc(SG_MOON_COMFORT,         SC_MOON_COMFORT,        SI_MOON_COMFORT);
	set_sc(SG_STAR_COMFORT,         SC_STAR_COMFORT,        SI_STAR_COMFORT);
	set_sc(SG_FUSION,               SC_FUSION,              SI_BLANK);
	set_sc(BS_ADRENALINE2,          SC_ADRENALINE2,         SI_ADRENALINE2);
	set_sc(SL_KAIZEL,               SC_KAIZEL,              SI_KAIZEL);
	set_sc(SL_KAAHI,                SC_KAAHI,               SI_KAAHI);
	set_sc(SL_KAUPE,                SC_KAUPE,               SI_KAUPE);
	set_sc(SL_KAITE,                SC_KAITE,               SI_KAITE);
	set_sc(SL_STUN,                 SC_STUN,                SI_BLANK);
	set_sc(SL_SWOO,                 SC_SWOO,                SI_BLANK);
	set_sc(SL_SKE,                  SC_SKE,                 SI_BLANK);
	set_sc(SL_SKA,                  SC_SKA,                 SI_BLANK);
	set_sc(ST_PRESERVE,             SC_PRESERVE,            SI_PRESERVE);
	set_sc(PF_DOUBLECASTING,        SC_DOUBLECAST,          SI_DOUBLECAST);
	set_sc(HW_GRAVITATION,          SC_GRAVITATION,         SI_BLANK);
	set_sc(WS_CARTTERMINATION,      SC_STUN,                SI_BLANK);
	set_sc(WS_OVERTHRUSTMAX,        SC_MAXOVERTHRUST,       SI_MAXOVERTHRUST);
	set_sc(CG_LONGINGFREEDOM,       SC_LONGING,             SI_BLANK);
	set_sc(CG_HERMODE,              SC_HERMODE,             SI_BLANK);
	set_sc(SL_HIGH,                 SC_SPIRIT,              SI_SPIRIT);
	set_sc(KN_ONEHAND,              SC_ONEHAND,             SI_ONEHAND);
	set_sc(CR_SHRINK,               SC_SHRINK,              SI_SHRINK);
	set_sc(RG_CLOSECONFINE,         SC_CLOSECONFINE2,       SI_CLOSECONFINE2);
	set_sc(RG_CLOSECONFINE,         SC_CLOSECONFINE,        SI_CLOSECONFINE);
	set_sc(WZ_SIGHTBLASTER,         SC_SIGHTBLASTER,        SI_SIGHTBLASTER);
	set_sc(DC_WINKCHARM,            SC_WINKCHARM,           SI_WINKCHARM);
	set_sc(MO_BALKYOUNG,            SC_STUN,                SI_BLANK);
	//Until they're at right position - gs_set_sc- [Vicious] / some of these don't seem to have a status icon adequate [blackhole89]
	set_sc(GS_MADNESSCANCEL,        SC_MADNESSCANCEL,       SI_MADNESSCANCEL);
	set_sc(GS_ADJUSTMENT,           SC_ADJUSTMENT,          SI_ADJUSTMENT);
	set_sc(GS_INCREASING,           SC_INCREASING,          SI_ACCURACY);
	set_sc(GS_GATLINGFEVER,         SC_GATLINGFEVER,        SI_GATLINGFEVER);
	set_sc(NJ_TATAMIGAESHI,         SC_TATAMIGAESHI,        SI_BLANK);
	set_sc(NJ_UTSUSEMI,             SC_UTSUSEMI,            SI_MAEMI);
	set_sc(NJ_KAENSIN,              SC_KAENSIN,             SI_BLANK);
	set_sc(NJ_SUITON,               SC_SUITON,              SI_BLANK);
	set_sc(NJ_NEN,                  SC_NEN,                 SI_NEN);

	// Storing the target job rather than simply SC_SPIRIT simplifies code later on.
	SkillStatusChangeTable[SL_ALCHEMIST] =   MAPID_ALCHEMIST,
	SkillStatusChangeTable[SL_MONK] =        MAPID_MONK,
	SkillStatusChangeTable[SL_STAR] =        MAPID_STAR_GLADIATOR,
	SkillStatusChangeTable[SL_SAGE] =        MAPID_SAGE,
	SkillStatusChangeTable[SL_CRUSADER] =    MAPID_CRUSADER,
	SkillStatusChangeTable[SL_SUPERNOVICE] = MAPID_SUPER_NOVICE,
	SkillStatusChangeTable[SL_KNIGHT] =      MAPID_KNIGHT,
	SkillStatusChangeTable[SL_WIZARD] =      MAPID_WIZARD,
	SkillStatusChangeTable[SL_PRIEST] =      MAPID_PRIEST,
	SkillStatusChangeTable[SL_BARDDANCER] =  MAPID_BARDDANCER,
	SkillStatusChangeTable[SL_ROGUE] =       MAPID_ROGUE,
	SkillStatusChangeTable[SL_ASSASIN] =     MAPID_ASSASSIN,
	SkillStatusChangeTable[SL_BLACKSMITH] =  MAPID_BLACKSMITH,
	SkillStatusChangeTable[SL_HUNTER] =      MAPID_HUNTER,
	SkillStatusChangeTable[SL_SOULLINKER] =  MAPID_SOUL_LINKER,

	//Status that don't have a skill associated.
	StatusIconChangeTable[SC_WEIGHT50 ] =   SI_WEIGHT50;
	StatusIconChangeTable[SC_WEIGHT90] =    SI_WEIGHT90;
	StatusIconChangeTable[SC_ASPDPOTION0] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_ASPDPOTION1] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_ASPDPOTION2] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_ASPDPOTION3] = SI_ASPDPOTION;
	StatusIconChangeTable[SC_SPEEDUP0] =    SI_SPEEDPOTION;
	StatusIconChangeTable[SC_SPEEDUP1] =    SI_SPEEDPOTION;
	StatusIconChangeTable[SC_MIRACLE] =    SI_SPIRIT;
	
	//Guild skills don't fit due to their range being beyond MAX_SKILL
	StatusIconChangeTable[SC_GUILDAURA] =    SI_GUILDAURA;
	StatusIconChangeTable[SC_BATTLEORDERS] = SI_BATTLEORDERS;
#undef set_sc

	if (!battle_config.display_hallucination) //Disable Hallucination.
		StatusIconChangeTable[SC_HALLUCINATION] = SI_BLANK;
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
 * flag:
 * 	0 - Trying to use skill on target.
 * 	1 - Cast bar is done.
 * 	2- Skill already pulled off, check is due to ground-based skills or splash-damage ones.
 * src MAY be null to indicate we shouldn't check it, this is a ground-based skill attack.
 * target MAY Be null, in which case the checks are only to see 
 * whether the source can cast or not the skill on the ground.
 *------------------------------------------
 */
int status_check_skilluse(struct block_list *src, struct block_list *target, int skill_num, int flag)
{
	int mode, race, hide_flag;
	struct status_change *sc=NULL, *tsc;

	if (src && status_isdead(src))
		return 0;
	if (target && status_isdead(target) && skill_num != ALL_RESURRECTION && skill_num != PR_REDEMPTIO)
		return 0;
	
	mode = src?status_get_mode(src):MD_CANATTACK;
	
	if (!skill_num && !(mode&MD_CANATTACK))
		return 0; //This mode is only needed for melee attacking.

	if (skill_num == PA_PRESSURE && flag) {
	//Gloria Avoids pretty much everythng....
		tsc = target?status_get_sc(target):NULL;
		if(tsc) {
			if (tsc->option&OPTION_HIDE)
				return 0;
			if (tsc->count && tsc->data[SC_TRICKDEAD].timer != -1)
				return 0;
		}
		return 1;
	}

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

	if (src) sc = status_get_sc(src);
	
	if(sc && sc->opt1 >0 && (battle_config.sc_castcancel || flag != 1))
		//When sc do not cancel casting, the spell should come out.
		return 0;
	
	if(sc && sc->count)
	{
		if (
			(sc->data[SC_TRICKDEAD].timer != -1 && skill_num != NV_TRICKDEAD)
			|| (sc->data[SC_AUTOCOUNTER].timer != -1 && skill_num != KN_AUTOCOUNTER)
			|| (sc->data[SC_GOSPEL].timer != -1 && sc->data[SC_GOSPEL].val4 == BCT_SELF && skill_num != PA_GOSPEL)
			|| (sc->data[SC_GRAVITATION].timer != -1 && sc->data[SC_GRAVITATION].val3 == BCT_SELF)
		)
			return 0;

		if (sc->data[SC_BLADESTOP].timer != -1) {
			switch (sc->data[SC_BLADESTOP].val1)
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
			if (!flag && ( //Blocked only from using the skill (stuff like autospell may still go through
				(sc->data[SC_MARIONETTE].timer != -1 && skill_num != CG_MARIONETTE) ||
				(sc->data[SC_MARIONETTE2].timer != -1 && skill_num == CG_MARIONETTE) ||
				sc->data[SC_SILENCE].timer != -1 || 
				sc->data[SC_STEELBODY].timer != -1 ||
				sc->data[SC_BERSERK].timer != -1 ||
				sc->data[SC_SKA].timer != -1
			))
				return 0;
			//Skill blocking.
			if (
				(sc->data[SC_VOLCANO].timer != -1 && skill_num == WZ_ICEWALL) ||
				(sc->data[SC_ROKISWEIL].timer != -1 && skill_num != BD_ADAPTATION && !(mode&MD_BOSS)) ||
				(sc->data[SC_HERMODE].timer != -1 && skill_get_inf(skill_num) & INF_SUPPORT_SKILL) ||
				sc->data[SC_NOCHAT].timer != -1
			)
				return 0;

			if (sc->data[SC_DANCING].timer != -1)
			{
				if (skill_num != BD_ADAPTATION && skill_num != CG_LONGINGFREEDOM
					&& skill_num != BA_MUSICALSTRIKE && skill_num != DC_THROWARROW)
					return 0;
				if (sc->data[SC_DANCING].val1 == CG_HERMODE && skill_num == BD_ADAPTATION)
					return 0;	//Can't amp out of Wand of Hermode :/ [Skotlex]
			}
		}
	}

	if (sc && sc->option)
	{
		if (sc->option&OPTION_HIDE && skill_num != TF_HIDING && skill_num != AS_GRIMTOOTH
			&& skill_num != RG_BACKSTAP && skill_num != RG_RAID)
			return 0;
//		if (sc->option&OPTION_CLOAK && skill_num == TF_HIDING)
//			return 0; //Latest reports indicate Hiding is usable while Cloaking. [Skotlex]
		if (sc->option&OPTION_CHASEWALK && skill_num != ST_CHASEWALK)
			return 0;
	}
	if (target == NULL || target == src) //No further checking needed.
		return 1;

	tsc = status_get_sc(target);
	if(tsc && tsc->count)
	{	
		if (!(mode & MD_BOSS) && tsc->data[SC_TRICKDEAD].timer != -1)
			return 0;
		if(skill_num == WZ_STORMGUST && tsc->data[SC_FREEZE].timer != -1)
			return 0;
		if(skill_num == PR_LEXAETERNA && (tsc->data[SC_FREEZE].timer != -1 || (tsc->data[SC_STONE].timer != -1 && tsc->data[SC_STONE].val2 == 0)))
			return 0;
	}

	race = src?status_get_race(src):0; 
	//If targetting, cloak+hide protect you, otherwise only hiding does.
	hide_flag = flag?OPTION_HIDE:(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK);
		
 	//You cannot hide from ground skills.
	if(skill_get_pl(skill_num) == 2)
		hide_flag &= ~OPTION_HIDE;
	
	switch (target->type)
	{
	case BL_PC:
		{
			struct map_session_data *sd = (struct map_session_data*) target;
			if (pc_isinvisible(sd))
				return 0;
			if (tsc->option&hide_flag
				&& (sd->state.perfect_hiding || !(race == 4 || race == 6 || mode&MD_DETECTOR))
				&& !(mode&MD_BOSS))
				return 0;
		}
		break;
	case BL_ITEM:	//Allow targetting of items to pick'em up (or in the case of mobs, to loot them).
		//TODO: Would be nice if this could be used to judge whether the player can or not pick up the item it targets. [Skotlex]
		if (mode&MD_LOOTER)
			return 1;
		else
			return 0;
	default:
		//Check for chase-walk/hiding/cloaking opponents.
		if (tsc && !(mode&MD_BOSS))
		{
			if (tsc->option&hide_flag && !(race == 4 || race == 6 || mode&MD_DETECTOR))
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
	int skill,refinedef=0;
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
	b_watk = sd->right_weapon.watk + sd->left_weapon.watk;
	b_def = sd->def;
	b_watk2 = sd->right_weapon.watk2 + sd->left_weapon.watk2;
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
	sd->hprate=100;
	sd->sprate=100;
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
			int j,c;
			//Card script execution.
			if(sd->status.inventory[index].card[0]==0x00ff ||
				sd->status.inventory[index].card[0]==0x00fe ||
				sd->status.inventory[index].card[0]==(short)0xff00)
				continue;
			for(j=0;j<sd->inventory_data[index]->slot;j++){	// ÉJ?Éh
				current_equip_card_id= c= sd->status.inventory[index].card[j];
				if(c>0){
					if(i == 8 && sd->status.inventory[index].equip == 0x20)
					{	//Left hand status.
						sd->state.lr_flag = 1;
						run_script(itemdb_equipscript(c),0,sd->bl.id,0);
						sd->state.lr_flag = 0;
					} else
						run_script(itemdb_equipscript(c),0,sd->bl.id,0);
					if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
						return 1;
				}
			}
		}
	}
	
	if(sd->status.pet_id > 0 && battle_config.pet_status_support && sd->pet.intimate > 0)
	{ // Pet
		struct pet_data *pd=sd->pd;
		if(pd && (!battle_config.pet_equip_required || pd->equip > 0) &&
			pd->state.skillbonus == 1 && pd->bonus) //Skotlex: Readjusted for pets
			pc_bonus(sd,pd->bonus->type, pd->bonus->val);
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
		if(!sd->inventory_data[index])
			continue;
		
		sd->def += sd->inventory_data[index]->def;
		if(sd->inventory_data[index]->type == 4) {
			int r,wlv = sd->inventory_data[index]->wlv;
			struct weapon_data *wd;
			if (wlv >= MAX_REFINE_BONUS) 
				wlv = MAX_REFINE_BONUS - 1;
			if(i == 8 && sd->status.inventory[index].equip == 0x20)
				wd = &sd->left_weapon; // Left-hand weapon
			else
				wd = &sd->right_weapon;
			wd->watk += sd->inventory_data[index]->atk;
			wd->watk2 = (r=sd->status.inventory[index].refine)*refinebonus[wlv][0];
			if((r-=refinebonus[wlv][2])>0) //Overrefine bonus.
				wd->overrefine = r*refinebonus[wlv][1];

			if (wd == &sd->left_weapon) {
				sd->attackrange_ += sd->inventory_data[index]->range;
				sd->state.lr_flag = 1;
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				sd->state.lr_flag = 0;
			} else {
				sd->attackrange += sd->inventory_data[index]->range;
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
			}
			if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
				return 1;

			if(sd->status.inventory[index].card[0]==0x00ff)
			{	// Forged weapon
				wd->star += (sd->status.inventory[index].card[1]>>8);
				if(wd->star >= 15) wd->star = 40; // 3 Star Crumbs now give +40 dmg
				if(pc_istop10fame( MakeDWord(sd->status.inventory[index].card[2],sd->status.inventory[index].card[3]) ,MAPID_BLACKSMITH))
					wd->star += 10;
				
				if (!wd->atk_ele) //Do not overwrite element from previous bonuses.
					wd->atk_ele = (sd->status.inventory[index].card[1]&0x0f);

			}
		}
		else if(sd->inventory_data[index]->type == 5) {
			refinedef += sd->status.inventory[index].refine*refinebonus[0][0];
			run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
			if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
				return 1;
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
	if(sd->sc.count && sd->sc.data[SC_CONCENTRATE].timer!=-1 && sd->sc.data[SC_QUAGMIRE].timer == -1){
		sd->paramb[1] += (sd->status.agi+sd->paramb[1]+sd->parame[1]-sd->paramcard[1])*(2+sd->sc.data[SC_CONCENTRATE].val1)/100;
		sd->paramb[4] += (sd->status.dex+sd->paramb[4]+sd->parame[4]-sd->paramcard[4])*(2+sd->sc.data[SC_CONCENTRATE].val1)/100;
	}

	// Absolute modifiers from status changes (only for PC)
	if(sd->sc.count){
		if (sd->sc.data[SC_BATTLEORDERS].timer != -1) {
			sd->paramb[0] += 5;
			sd->paramb[3] += 5;
			sd->paramb[4] += 5;
		}
		if (sd->sc.data[SC_GUILDAURA].timer != -1) {
			int guildflag = sd->sc.data[SC_GUILDAURA].val4;
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
	if(sd->sc.count){
		if(sd->sc.data[SC_MARIONETTE].timer!=-1){
			sd->paramb[0]-= sd->status.str/2;
			sd->paramb[1]-= sd->status.agi/2;
			sd->paramb[2]-= sd->status.vit/2;
			sd->paramb[3]-= sd->status.int_/2;
			sd->paramb[4]-= sd->status.dex/2;
			sd->paramb[5]-= sd->status.luk/2;
		}
		else if(sd->sc.data[SC_MARIONETTE2].timer!=-1){
			struct map_session_data *psd = map_id2sd(sd->sc.data[SC_MARIONETTE2].val3);
			if (psd) {	// if partner is found
				bl = pc_maxparameter(sd); //Cap to max parameter. [Skotlex]
				if (sd->status.str < bl)
					sd->paramb[0] += sd->status.str+psd->status.str/2 > bl ?
						bl-sd->status.str : psd->status.str/2;
				if (sd->status.agi < bl)
					sd->paramb[1] += sd->status.agi+psd->status.agi/2 > bl ?
						bl-sd->status.agi : psd->status.agi/2;
				if (sd->status.vit < bl)
					sd->paramb[2] += sd->status.vit+psd->status.vit/2 > bl ?
						bl-sd->status.vit : psd->status.vit/2;
				if (sd->status.int_ < bl)
					sd->paramb[3] += sd->status.int_+psd->status.int_/2 > bl ?
						bl-sd->status.int_ : psd->status.int_/2;
				if (sd->status.dex < bl)
					sd->paramb[4] += sd->status.dex+psd->status.dex/2 > bl ?
						bl-sd->status.dex : psd->status.dex/2;
				if (sd->status.luk < bl)
					sd->paramb[5] += sd->status.luk+psd->status.luk/2 > bl ?
						bl-sd->status.luk : psd->status.luk/2;
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

	if(sd->sc.count){
		if(sd->sc.data[SC_SPIRIT].timer!=-1 && sd->sc.data[SC_SPIRIT].val2 == SL_HIGH)
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

	if (sd->sc.count && sd->sc.data[SC_CURSE].timer!=-1)
		sd->paramc[5] = 0;

// ------ BASE ATTACK CALCULATION ------

	// Basic Base ATK value
	switch(sd->status.weapon){
		case 11: // Bows
		case 13: // Musical Instruments
		case 14: // Whips
		case 17: // Revolver
		case 18: // Rifle
		case 19: // Shotgun
		case 20: //Gatling Gun
		case 21: //Grenade Launcher
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
	if(sd->sc.count){
		if(sd->sc.data[SC_NIBELUNGEN].timer!=-1){
			index = sd->equip_index[9];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				sd->right_weapon.watk2 += sd->sc.data[SC_NIBELUNGEN].val2;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				sd->left_weapon.watk2 += sd->sc.data[SC_NIBELUNGEN].val2;
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
	if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0 && (sd->status.weapon == 17 || sd->status.weapon == 18
		|| sd->status.weapon == 19 || sd->status.weapon == 20 || sd->status.weapon == 21))
		sd->hit += 2*skill;
	if((skill=pc_checkskill(sd,GS_SNAKEEYE))>0 && (sd->status.weapon == 17 || sd->status.weapon == 18
		|| sd->status.weapon == 19 || sd->status.weapon == 20 || sd->status.weapon == 21)) {
		sd->hit += skill;
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
	if(sd->sc.count && sd->sc.data[SC_BERSERK].timer!=-1)
		sd->def2 = 0;
	else if(sd->sc.count && sd->sc.data[SC_ETERNALCHAOS].timer!=-1)
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
	if(sd->sc.count && sd->sc.data[SC_BERSERK].timer!=-1)
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
	if((skill=pc_checkskill(sd,TF_MISS))>0 && (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN && sd->sc.data[SC_CLOAKING].timer==-1)
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
	if(sd->sc.count && sd->sc.data[SC_DANCING].timer!=-1){
			int s_rate = 500-40*pc_checkskill(sd,(sd->status.sex?BA_MUSICALLESSON:DC_DANCINGLESSON));
			if (sd->sc.data[SC_SPIRIT].timer != -1 && sd->sc.data[SC_SPIRIT].val2 == SL_BARDDANCER)
				s_rate -= 40; //TODO: Figure out real bonus rate.
			if (sd->sc.data[SC_LONGING].timer!=-1)
				s_rate -= 20 * sd->sc.data[SC_LONGING].val1;
			sd->speed += sd->speed * s_rate/100;
	}
	if(sd->sc.data[SC_FUSION].timer != -1) //Additional movement speed from SG_FUSION [Komurka]
		sd->speed -= sd->speed * 25/100;

	// Apply relative modifiers from equipment
	if(sd->speed_rate != 100)
		sd->speed = sd->speed*sd->speed_rate/100;

	if(sd->speed < battle_config.max_walk_speed)
		sd->speed = battle_config.max_walk_speed;

// ----- ASPD CALCULATION -----
// Unlike other stats, ASPD rate modifiers from skills/SCs/items/etc are first all added together, then the final modifier is applied

	// Basic ASPD value
	if (sd->status.weapon <= MAX_WEAPON_TYPE)
		sd->aspd += aspd_base[sd->status.class_][sd->status.weapon]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[sd->status.class_][sd->status.weapon]/1000;
	else
		sd->aspd += (
			(aspd_base[sd->status.class_][sd->weapontype1]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[sd->status.class_][sd->weapontype1]/1000) +
			(aspd_base[sd->status.class_][sd->weapontype2]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[sd->status.class_][sd->weapontype2]/1000)
			) *2/3; //From what I read in rodatazone, 2/3 should be more accurate than 0.7 -> 140 / 200; [Skotlex]

	// Relative modifiers from passive skills
	if((skill=pc_checkskill(sd,SA_ADVANCEDBOOK))>0)
		sd->aspd_rate -= (skill/2);
	if((skill = pc_checkskill(sd,SG_DEVIL)) > 0 && !pc_nextjobexp(sd))
		sd->aspd_rate -= (skill*3);

	if(pc_isriding(sd))
		sd->aspd_rate += 50-10*pc_checkskill(sd,KN_CAVALIERMASTERY);
		
	if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0 && (sd->status.weapon == 17 || sd->status.weapon == 18
		|| sd->status.weapon == 19 || sd->status.weapon == 20 || sd->status.weapon == 21))
		sd->aspd_rate -= (int)(skill / 2);

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
	if(battle_config.hp_rate != 100)
		sd->status.max_hp = sd->status.max_hp * battle_config.hp_rate/100;
	
	if (sd->status.max_hp < 0) //HP overflow??
		sd->status.max_hp = battle_config.max_hp;
	else if(sd->status.max_hp > battle_config.max_hp)
		sd->status.max_hp = battle_config.max_hp;
	else if(sd->status.max_hp == 0)
		sd->status.max_hp = 1;
	
	if(sd->status.hp>sd->status.max_hp)
		sd->status.hp=sd->status.max_hp;

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
	if(battle_config.sp_rate != 100)
		sd->status.max_sp = sd->status.max_sp * battle_config.sp_rate/100;
	
	if(sd->status.max_sp > battle_config.max_sp)
		sd->status.max_sp = battle_config.max_sp;
	else if(sd->status.max_sp <= 0)
	  	sd->status.max_sp = 1;
	if(sd->status.sp>sd->status.max_sp)
		sd->status.sp=sd->status.max_sp;

	if(sd->sc.data[SC_DANCING].timer==-1){
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
		if((skill=pc_checkskill(sd,NJ_NINPOU)) > 0)
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

	if(sd->sc.count){
		if(sd->sc.data[SC_SERVICE4U].timer!=-1)
			sd->dsprate -= sd->sc.data[SC_SERVICE4U].val3;
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

	if(sd->sc.count){
       	if(sd->sc.data[SC_SIEGFRIED].timer!=-1){
			sd->subele[1] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[2] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[3] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[4] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[5] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[6] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[7] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[8] += sd->sc.data[SC_SIEGFRIED].val2;
			sd->subele[9] += sd->sc.data[SC_SIEGFRIED].val2;
		}
		if(sd->sc.data[SC_PROVIDENCE].timer!=-1){
			sd->subele[6] += sd->sc.data[SC_PROVIDENCE].val2;
			sd->subrace[6] += sd->sc.data[SC_PROVIDENCE].val2;
		}
	}

// ----- CLIENT-SIDE REFRESH -----
	if(first&1) { //Since this is the initial loading, the Falcon and Peco icons must be loaded. [Skotlex]
		if (sd->sc.option&OPTION_FALCON)
			clif_status_load(&sd->bl, SI_FALCON, 1);
		if (sd->sc.option&OPTION_RIDING)
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

	
	if(sd->sc.data[SC_WEDDING].timer != -1 && sd->view_class != JOB_WEDDING)
		sd->view_class=JOB_WEDDING;
	
	if(sd->sc.data[SC_XMAS].timer != -1 && sd->view_class != JOB_XMAS)
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

	if(memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill)))
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
	if(b_watk != sd->right_weapon.watk + sd->left_weapon.watk || b_base_atk != sd->base_atk)
		clif_updatestatus(sd,SP_ATK1);
	if(b_def != sd->def)
		clif_updatestatus(sd,SP_DEF1);
	if(b_watk2 != sd->right_weapon.watk2 + sd->left_weapon.watk2)
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

	calculating = 0;
	return 0;
}

/*==========================================
 * Apply shared stat mods from status changes [DracoRPG]
 *------------------------------------------
 */
int status_calc_str(struct block_list *bl, int str)
{
	struct status_change *sc;
	nullpo_retr(str,bl);
	sc= status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCALLSTATUS].timer!=-1)
			str += sc->data[SC_INCALLSTATUS].val1;
		if(sc->data[SC_INCSTR].timer!=-1)
			str += sc->data[SC_INCSTR].val1;
		if(sc->data[SC_STRFOOD].timer!=-1)
			str += sc->data[SC_STRFOOD].val1;
		if(sc->data[SC_LOUD].timer!=-1)
			str += 4;
  		if(sc->data[SC_TRUESIGHT].timer!=-1)
			str += 5;
		if(sc->data[SC_SPURT].timer!=-1)
			str += 10; //Bonus is +!0 regardless of skill level
		if(sc->data[SC_BLESSING].timer != -1){
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6)
				str >>= 1;
			else str += sc->data[SC_BLESSING].val1;
		}
	}

	return str;
}

int status_calc_agi(struct block_list *bl, int agi)
{
	struct status_change *sc;
	nullpo_retr(agi,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCALLSTATUS].timer!=-1)
			agi += sc->data[SC_INCALLSTATUS].val1;
		if(sc->data[SC_INCAGI].timer!=-1)
			agi += sc->data[SC_INCAGI].val1;
		if(sc->data[SC_AGIFOOD].timer!=-1)
			agi += sc->data[SC_AGIFOOD].val1;
  		if(sc->data[SC_TRUESIGHT].timer!=-1)
			agi += 5;
		if(sc->data[SC_INCREASEAGI].timer!=-1)
			agi += 2 + sc->data[SC_INCREASEAGI].val1;
		if(sc->data[SC_DECREASEAGI].timer!=-1)
			agi -= 2 + sc->data[SC_DECREASEAGI].val1;
		if(sc->data[SC_QUAGMIRE].timer!=-1)
			agi -= sc->data[SC_QUAGMIRE].val1*(bl->type==BL_PC?5:10);
		if(sc->data[SC_SUITON].timer!=-1 || status_get_class(bl) != JOB_NINJA)
			agi -= (((sc->data[SC_SUITON].val1 - 1) / 3) + 1) * 3;
	}

	return agi;
}

int status_calc_vit(struct block_list *bl, int vit)
{
	struct status_change *sc;
	nullpo_retr(vit,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCALLSTATUS].timer!=-1)
			vit += sc->data[SC_INCALLSTATUS].val1;
		if(sc->data[SC_INCVIT].timer!=-1)
			vit += sc->data[SC_INCVIT].val1;
		if(sc->data[SC_VITFOOD].timer!=-1)
			vit += sc->data[SC_VITFOOD].val1;
  		if(sc->data[SC_TRUESIGHT].timer!=-1)
			vit += 5;
		if(sc->data[SC_STRIPARMOR].timer!=-1 && bl->type != BL_PC)
			vit -= vit * 8*sc->data[SC_STRIPARMOR].val1/100;
	}

	return vit;
}

int status_calc_int(struct block_list *bl, int int_)
{
	struct status_change *sc;
	nullpo_retr(int_,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCALLSTATUS].timer!=-1)
			int_ += sc->data[SC_INCALLSTATUS].val1;
		if(sc->data[SC_INCINT].timer!=-1)
			int_ += sc->data[SC_INCINT].val1;
		if(sc->data[SC_INTFOOD].timer!=-1)
			int_ += sc->data[SC_INTFOOD].val1;
  		if(sc->data[SC_TRUESIGHT].timer!=-1)
			int_ += 5;
		if(sc->data[SC_BLESSING].timer != -1){
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6)
				int_ >>= 1;
			else int_ += sc->data[SC_BLESSING].val1;
		}
		if(sc->data[SC_STRIPHELM].timer!=-1 && bl->type != BL_PC)
			int_ -= int_ * 8*sc->data[SC_STRIPHELM].val1/100;
	}

	return int_;
}

int status_calc_dex(struct block_list *bl, int dex)
{
	struct status_change *sc;
	nullpo_retr(dex,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCALLSTATUS].timer!=-1)
			dex += sc->data[SC_INCALLSTATUS].val1;
		if(sc->data[SC_INCDEX].timer!=-1)
			dex += sc->data[SC_INCDEX].val1;
		if(sc->data[SC_DEXFOOD].timer!=-1)
			dex += sc->data[SC_DEXFOOD].val1;
  		if(sc->data[SC_TRUESIGHT].timer!=-1)
			dex += 5;
		if(sc->data[SC_QUAGMIRE].timer!=-1)
			dex -= sc->data[SC_QUAGMIRE].val1*(bl->type==BL_PC?5:10);
		if(sc->data[SC_BLESSING].timer != -1){
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6)
				dex >>= 1;
			else dex += sc->data[SC_BLESSING].val1;
		}
	}

	return dex;
}

int status_calc_luk(struct block_list *bl, int luk)
{
	struct status_change *sc;
	nullpo_retr(luk,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCALLSTATUS].timer!=-1)
			luk += sc->data[SC_INCALLSTATUS].val1;
		if(sc->data[SC_INCLUK].timer!=-1)
			luk += sc->data[SC_INCLUK].val1;
		if(sc->data[SC_LUKFOOD].timer!=-1)
			luk += sc->data[SC_LUKFOOD].val1;
  		if(sc->data[SC_TRUESIGHT].timer!=-1)
			luk += 5;
  		if(sc->data[SC_GLORIA].timer!=-1)
			luk += 30;
	}

	return luk;
}

int status_calc_batk(struct block_list *bl, int batk)
{
	struct status_change *sc;
	nullpo_retr(batk,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_ATKPOTION].timer!=-1)
			batk += sc->data[SC_ATKPOTION].val1;
		if(sc->data[SC_BATKFOOD].timer!=-1)
			batk += sc->data[SC_BATKFOOD].val1;
		if(sc->data[SC_INCATKRATE].timer!=-1)
			batk += batk * sc->data[SC_INCATKRATE].val1/100;
		if(sc->data[SC_PROVOKE].timer!=-1)
			batk += batk * (2+3*sc->data[SC_PROVOKE].val1)/100;
		if(sc->data[SC_CONCENTRATION].timer!=-1)
			batk += batk * 5*sc->data[SC_CONCENTRATION].val1/100;
		if(sc->data[SC_SKE].timer!=-1)
			batk += batk * 3;
		if(sc->data[SC_JOINTBEAT].timer!=-1 && sc->data[SC_JOINTBEAT].val2==4)
			batk -= batk * 25/100;
		if(sc->data[SC_CURSE].timer!=-1)
			batk -= batk * 25/100;
//Curse shouldn't effect on this? 
//		if(sc->data[SC_BLEEDING].timer != -1)
//			batk -= batk * 25/100;
		if(sc->data[SC_MADNESSCANCEL].timer!=-1)
			batk += 100;
	}

	return batk;
}

int status_calc_watk(struct block_list *bl, int watk)
{
	struct status_change *sc;
	nullpo_retr(watk,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_IMPOSITIO].timer!=-1)
			watk += 5*sc->data[SC_IMPOSITIO].val1;
		if(sc->data[SC_WATKFOOD].timer!=-1)
			watk += sc->data[SC_WATKFOOD].val1;
		if(sc->data[SC_DRUMBATTLE].timer!=-1)
			watk += sc->data[SC_DRUMBATTLE].val2;
		if(sc->data[SC_VOLCANO].timer!=-1 && status_get_elem_type(bl)==3)
			watk += sc->data[SC_VOLCANO].val3;
		if(sc->data[SC_INCATKRATE].timer!=-1)
			watk += watk * sc->data[SC_INCATKRATE].val1/100;
		if(sc->data[SC_PROVOKE].timer!=-1)
			watk += watk * (2+3*sc->data[SC_PROVOKE].val1)/100;
		if(sc->data[SC_CONCENTRATION].timer!=-1)
			watk += watk * 5*sc->data[SC_CONCENTRATION].val1/100;
		if(sc->data[SC_SKE].timer!=-1)
			watk += watk * 3;
		if(sc->data[SC_NIBELUNGEN].timer!=-1 && bl->type != BL_PC && (status_get_element(bl)/10)>=8)
			watk += sc->data[SC_NIBELUNGEN].val2;
		if(sc->data[SC_CURSE].timer!=-1)
			watk -= watk * 25/100;
		if(sc->data[SC_STRIPWEAPON].timer!=-1 && bl->type != BL_PC)
			watk -= watk * 5*sc->data[SC_STRIPWEAPON].val1/100;
	}
	return watk;
}

int status_calc_matk(struct block_list *bl, int matk)
{
	struct status_change *sc;
	nullpo_retr(matk,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_MATKPOTION].timer!=-1)
			matk += sc->data[SC_MATKPOTION].val1;
		if(sc->data[SC_MATKFOOD].timer!=-1)
			matk += sc->data[SC_MATKFOOD].val1;
		if(sc->data[SC_MAGICPOWER].timer!=-1)
			matk += matk * 5*sc->data[SC_MAGICPOWER].val1/100;
		if(sc->data[SC_MINDBREAKER].timer!=-1)
			matk += matk * 20*sc->data[SC_MINDBREAKER].val1/100;
		if(sc->data[SC_INCMATKRATE].timer!=-1)
			matk += matk * sc->data[SC_INCMATKRATE].val1/100;
	}

	return matk;
}

int status_calc_critical(struct block_list *bl, int critical)
{
	struct status_change *sc;
	nullpo_retr(critical,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if (sc->data[SC_EXPLOSIONSPIRITS].timer!=-1)
			critical += sc->data[SC_EXPLOSIONSPIRITS].val2;
		if (sc->data[SC_FORTUNE].timer!=-1)
			critical += sc->data[SC_FORTUNE].val2*10;
		if (sc->data[SC_TRUESIGHT].timer!=-1)
			critical += sc->data[SC_TRUESIGHT].val1*10;
		if(sc->data[SC_CLOAKING].timer!=-1)
			critical += critical;
	}

	return critical;
}

int status_calc_hit(struct block_list *bl, int hit)
{
	struct status_change *sc;
	nullpo_retr(hit,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCHIT].timer != -1)
			hit += sc->data[SC_INCHIT].val1;
		if(sc->data[SC_HITFOOD].timer!=-1)
			hit += sc->data[SC_HITFOOD].val1;
		if(sc->data[SC_TRUESIGHT].timer != -1)
			hit += 3*sc->data[SC_TRUESIGHT].val1;
		if(sc->data[SC_HUMMING].timer!=-1)
			hit += sc->data[SC_HUMMING].val2;
		if(sc->data[SC_CONCENTRATION].timer != -1)
			hit += 10*sc->data[SC_CONCENTRATION].val1;
		if(sc->data[SC_INCHITRATE].timer != -1)
			hit += hit * sc->data[SC_INCHITRATE].val1/100;
		if(sc->data[SC_BLIND].timer != -1)
			hit -= hit * 25 / 100;
	}

	return hit;
}

int status_calc_flee(struct block_list *bl, int flee)
{
	struct status_change *sc;
	nullpo_retr(flee,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_INCFLEE].timer!=-1)
			flee += sc->data[SC_INCFLEE].val1;
		if(sc->data[SC_FLEEFOOD].timer!=-1)
			flee += sc->data[SC_FLEEFOOD].val1;
		if(sc->data[SC_WHISTLE].timer!=-1)
			flee += sc->data[SC_WHISTLE].val2;
		if(sc->data[SC_WINDWALK].timer!=-1)
			flee += sc->data[SC_WINDWALK].val2;
		if(sc->data[SC_INCFLEERATE].timer!=-1)
			flee += flee * sc->data[SC_INCFLEERATE].val1/100;
		if(sc->data[SC_VIOLENTGALE].timer!=-1 && status_get_elem_type(bl)==4)
			flee += flee * sc->data[SC_VIOLENTGALE].val3/100;
		if(sc->data[SC_MOON_COMFORT].timer!=-1) //SG skill [Komurka]
			flee += (status_get_lv(bl) + status_get_dex(bl) + status_get_luk(bl))/10;
		if(sc->data[SC_CLOSECONFINE].timer!=-1)
			flee += 10;
		if(sc->data[SC_SPIDERWEB].timer!=-1)
			flee -= flee * 50/100;
		if(sc->data[SC_BERSERK].timer!=-1)
			flee -= flee * 50/100;
		if(sc->data[SC_BLIND].timer!=-1)
			flee -= flee * 25/100;
		if(sc->data[SC_ADJUSTMENT].timer!=-1)
			flee += 30;
		if(sc->data[SC_GATLINGFEVER].timer!=-1)
			flee -= sc->data[SC_GATLINGFEVER].val1*5;
	}

	if (bl->type == BL_PC && map_flag_gvg(bl->m)) //GVG grounds flee penalty, placed here because it's "like" a status change. [Skotlex]
		flee -= flee * battle_config.gvg_flee_penalty/100;
	return flee;
}

int status_calc_flee2(struct block_list *bl, int flee2)
{
	struct status_change *sc;
	nullpo_retr(flee2,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_WHISTLE].timer!=-1)
			flee2 += sc->data[SC_WHISTLE].val3*10;
	}

	return flee2;
}

int status_calc_def(struct block_list *bl, int def)
{
	struct status_change *sc;
	nullpo_retr(def,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc->data[SC_KEEPING].timer!=-1)
			return 100;
		if(sc->data[SC_STEELBODY].timer!=-1)
			return 90;
		if(sc->data[SC_SKA].timer != -1) // [marquis007]
			return 90;
		if(sc->data[SC_DRUMBATTLE].timer!=-1)
			def += sc->data[SC_DRUMBATTLE].val3;
		if(sc->data[SC_INCDEFRATE].timer!=-1)
			def += def * sc->data[SC_INCDEFRATE].val1/100;
		if(sc->data[SC_SIGNUMCRUCIS].timer!=-1)
			def -= def * sc->data[SC_SIGNUMCRUCIS].val2/100;
		if(sc->data[SC_CONCENTRATION].timer!=-1)
			def -= def * 5*sc->data[SC_CONCENTRATION].val1/100;
		if(sc->data[SC_SKE].timer!=-1)
			def -= def  * 50/100;
		if(sc->data[SC_PROVOKE].timer!=-1 && bl->type != BL_PC) // Provoke doesn't alter player defense.
			def -= def * (5+5*sc->data[SC_PROVOKE].val1)/100;
		if(sc->data[SC_STRIPSHIELD].timer!=-1 && bl->type != BL_PC)
			def -= def * 3*sc->data[SC_STRIPSHIELD].val1/100;
	}

	return def;
}

int status_calc_def2(struct block_list *bl, int def2)
{
	struct status_change *sc;
	nullpo_retr(def2,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc->data[SC_ETERNALCHAOS].timer!=-1)
			return 0;
		if(sc->data[SC_SUN_COMFORT].timer!=-1)
			def2 += (status_get_lv(bl) + status_get_dex(bl) + status_get_luk(bl))/2;
		if(sc->data[SC_ANGELUS].timer!=-1)
			def2 += def2 * (10+5*sc->data[SC_ANGELUS].val1)/100;
		if(sc->data[SC_CONCENTRATION].timer!=-1)
			def2 -= def2 * 5*sc->data[SC_CONCENTRATION].val1/100;
		if(sc->data[SC_POISON].timer!=-1)
			def2 -= def2 * 25/100;
		if(sc->data[SC_SKE].timer!=-1)
			def2 -= def2 * 50/100;
		if(sc->data[SC_PROVOKE].timer!=-1)
			def2 -= def2 * (5+5*sc->data[SC_PROVOKE].val1)/100;
		if(sc->data[SC_JOINTBEAT].timer!=-1){
			if(sc->data[SC_JOINTBEAT].val2==3)
				def2 -= def2 * 50/100;
			else if(sc->data[SC_JOINTBEAT].val2==4)
				def2 -= def2 * 25/100;
		}
	}

	return def2;
}

int status_calc_mdef(struct block_list *bl, int mdef)
{
	struct status_change *sc;
	nullpo_retr(mdef,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc->data[SC_BARRIER].timer!=-1)
			return 100;
		if(sc->data[SC_STEELBODY].timer!=-1)
			return 90;
		if(sc->data[SC_SKA].timer != -1) // [marquis007]
			return 90; // should it up mdef too?
		if(sc->data[SC_ENDURE].timer!=-1)
			mdef += sc->data[SC_ENDURE].val1;
	}

	return mdef;
}

int status_calc_mdef2(struct block_list *bl, int mdef2)
{
	struct status_change *sc;
	nullpo_retr(mdef2,bl);
	sc = status_get_sc(bl);

	if(sc && sc->count){
		if(sc->data[SC_BERSERK].timer!=-1)
			return 0;
		if(sc->data[SC_MINDBREAKER].timer!=-1)
			mdef2 -= mdef2 * 12*sc->data[SC_MINDBREAKER].val1/100;
	}

	return mdef2;
}

int status_calc_speed(struct block_list *bl, int speed)
{
	struct status_change *sc;
	sc = status_get_sc(bl);

	if(sc && sc->count) {
		if(sc->data[SC_CURSE].timer!=-1)
			speed += 450;
		if(sc->data[SC_SWOO].timer != -1) // [marquis007]
			speed += 450; //Let's use Curse's slow down momentarily (exact value unknown)
		if(sc->data[SC_SPEEDUP1].timer!=-1)
			speed -= speed*50/100;
		else if(sc->data[SC_SPEEDUP0].timer!=-1)
			speed -= speed*25/100;
		else if(sc->data[SC_INCREASEAGI].timer!=-1)
			speed -= speed * 25/100;
		else if(sc->data[SC_CARTBOOST].timer!=-1)
			speed -= speed * 20/100;
		else if(sc->data[SC_BERSERK].timer!=-1)
			speed -= speed * 20/100;
		else if(sc->data[SC_WINDWALK].timer!=-1)
			speed -= speed * 4*sc->data[SC_WINDWALK].val2/100;
		if(sc->data[SC_WEDDING].timer!=-1)
			speed += speed * 50/100;
		if(sc->data[SC_SLOWDOWN].timer!=-1)
			speed += speed * 50/100;
		if(sc->data[SC_DECREASEAGI].timer!=-1)
			speed += speed * 25/100;
		if(sc->data[SC_STEELBODY].timer!=-1)
			speed += speed * 25/100;
		if(sc->data[SC_SKA].timer!=-1)
			speed += speed * 25/100;
		if(sc->data[SC_QUAGMIRE].timer!=-1)
			speed += speed * 50/100;
		if(sc->data[SC_DONTFORGETME].timer!=-1)
			speed += speed * sc->data[SC_DONTFORGETME].val3/100;
		if(sc->data[SC_DEFENDER].timer!=-1)
			speed += speed * (55-5*sc->data[SC_DEFENDER].val1)/100;
		if(sc->data[SC_GOSPEL].timer!=-1 && sc->data[SC_GOSPEL].val4 == BCT_ENEMY)
			speed += speed * 25/100;
		if(sc->data[SC_JOINTBEAT].timer!=-1) {
			if (sc->data[SC_JOINTBEAT].val2 == 0)
				speed += speed * 50/100;
			else if (sc->data[SC_JOINTBEAT].val2 == 2)
				speed += speed * 30/100;
		}
		if(sc->data[SC_CLOAKING].timer!=-1)
			speed = speed * (sc->data[SC_CLOAKING].val3-3*sc->data[SC_CLOAKING].val1) /100;
		if(sc->data[SC_CHASEWALK].timer!=-1)
			speed = speed * sc->data[SC_CHASEWALK].val3/100;
		if(sc->data[SC_RUN].timer!=-1)/*ãÏÇØë´Ç…ÇÊÇÈë¨ìxïœâª*/
			speed -= speed * 25/100;
		if(sc->data[SC_GATLINGFEVER].timer!=-1)
			speed += speed * 25/100;
	}

	return speed;
}

int status_calc_aspd_rate(struct block_list *bl, int aspd_rate)
{
	struct status_change *sc;
	sc = status_get_sc(bl);

	if(sc && sc->count) {
		int i;
		if(sc->data[SC_QUAGMIRE].timer==-1 && sc->data[SC_DONTFORGETME].timer==-1){
			if(sc->data[SC_TWOHANDQUICKEN].timer!=-1)
				aspd_rate -= 30;
			else if(sc->data[SC_ONEHAND].timer!=-1)
				aspd_rate -= 30;
			else if(sc->data[SC_ADRENALINE2].timer!=-1)
				aspd_rate -= (sc->data[SC_ADRENALINE2].val2 || !battle_config.party_skill_penalty)?30:20;
			else if(sc->data[SC_ADRENALINE].timer!=-1)
				aspd_rate -= (sc->data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)?30:20;
			else if(sc->data[SC_SPEARSQUICKEN].timer!=-1)
				aspd_rate -= sc->data[SC_SPEARSQUICKEN].val2;
			else if(sc->data[SC_ASSNCROS].timer!=-1 && (bl->type!=BL_PC || ((struct map_session_data*)bl)->status.weapon != 11))
				aspd_rate -= sc->data[SC_ASSNCROS].val2;
		}
		if(sc->data[SC_BERSERK].timer!=-1)
			aspd_rate -= 30;
		if(sc->data[i=SC_ASPDPOTION3].timer!=-1 || sc->data[i=SC_ASPDPOTION2].timer!=-1 || sc->data[i=SC_ASPDPOTION1].timer!=-1 || sc->data[i=SC_ASPDPOTION0].timer!=-1)
			aspd_rate -= sc->data[i].val2;
		if(sc->data[SC_DONTFORGETME].timer!=-1)
			aspd_rate += sc->data[SC_DONTFORGETME].val2;
		if(sc->data[SC_STEELBODY].timer!=-1)
			aspd_rate += 25;
		if(sc->data[SC_SKA].timer!=-1)
			aspd_rate += 25;
		if(sc->data[SC_DEFENDER].timer != -1)
			aspd_rate += 25 -sc->data[SC_DEFENDER].val1*5;
		if(sc->data[SC_GOSPEL].timer!=-1 && sc->data[SC_GOSPEL].val4 == BCT_ENEMY)
			aspd_rate += 25;
		if(sc->data[SC_GRAVITATION].timer!=-1)
			aspd_rate += sc->data[SC_GRAVITATION].val2;
//Curse shouldn't effect on this?
//		if(sc->data[SC_BLEEDING].timer != -1)
//			aspd_rate += 25;
		if(sc->data[SC_JOINTBEAT].timer!=-1) {
			if (sc->data[SC_JOINTBEAT].val2 == 1)
				aspd_rate += 25;
			else if (sc->data[SC_JOINTBEAT].val2 == 2)
				aspd_rate += 10;
		}
		if(sc->data[SC_STAR_COMFORT].timer!=-1)
			aspd_rate -= (status_get_lv(bl) + status_get_dex(bl) + status_get_luk(bl))/10;
		if(sc->data[SC_MADNESSCANCEL].timer!=-1)
			aspd_rate -= 20;
		if(sc->data[SC_GATLINGFEVER].timer!=-1)
			aspd_rate -= sc->data[SC_GATLINGFEVER].val1*2;
	}

	return aspd_rate;
}

int status_calc_maxhp(struct block_list *bl, int maxhp)
{
	struct status_change *sc;
	sc = status_get_sc(bl);

	if(sc && sc->count) {
		if(sc->data[SC_INCMHPRATE].timer!=-1)
			maxhp += maxhp * sc->data[SC_INCMHPRATE].val1/100;
		if(sc->data[SC_APPLEIDUN].timer!=-1)
			maxhp += maxhp * sc->data[SC_APPLEIDUN].val2/100;
		if(sc->data[SC_DELUGE].timer!=-1 && status_get_elem_type(bl)==1)
			maxhp += maxhp * deluge_eff[sc->data[SC_DELUGE].val1-1]/100;
		if(sc->data[SC_BERSERK].timer!=-1)
			maxhp += maxhp * 2;
	}

	return maxhp;
}

int status_calc_maxsp(struct block_list *bl, int maxsp)
{
	struct status_change *sc;
	sc = status_get_sc(bl);

	if(sc && sc->count) {
		if(sc->data[SC_INCMSPRATE].timer!=-1)
			maxsp += maxsp * sc->data[SC_INCMSPRATE].val1/100;
		if(sc->data[SC_SERVICE4U].timer!=-1)
			maxsp += maxsp * sc->data[SC_SERVICE4U].val2/100;
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
			if (start && sd->sc.data[SC_CLOAKING].timer != -1)
			{	//There shouldn't be an "stop" case here.
				//If the previous upgrade was 
				//SPEED_ADD_RATE(3*sd->sc.data[SC_CLOAKING].val1 -sd->sc.data[SC_CLOAKING].val3);
				//Then just changing val3 should be a net difference of....
				if (3*sd->sc.data[SC_CLOAKING].val1 != sd->sc.data[SC_CLOAKING].val3)	//This reverts the previous value.
					sd->speed = sd->speed * 100 /(sd->sc.data[SC_CLOAKING].val3-3*sd->sc.data[SC_CLOAKING].val1);
				sd->sc.data[SC_CLOAKING].val3 = skill_lv;
					sd->speed = sd->speed * (sd->sc.data[SC_CLOAKING].val3-sd->sc.data[SC_CLOAKING].val1*3) /100;
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
		if (((struct map_session_data *)bl)->status.weapon <= MAX_WEAPON_TYPE)
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
	struct status_change *sc;

	nullpo_retr(0, bl);
	sc = status_get_sc(bl);
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

	if(sc && sc->count && (sc->data[SC_ENDURE].timer!=-1 || sc->data[SC_CONCENTRATION].timer!=-1 || sc->data[SC_BERSERK].timer!=-1))
		if (!map_flag_gvg(bl->m)) //Only works on non-gvg grounds. [Skotlex]
			return 0;

	return ret;
}
int status_get_element(struct block_list *bl)
{
	// removed redundant variable ret [zzo]
	struct status_change *sc= status_get_sc(bl);

	nullpo_retr(20, bl);

	if(sc && sc->count) {
		if( sc->data[SC_FREEZE].timer!=-1 )	// ìÄåã
			return 21;
		if( sc->data[SC_STONE].timer!=-1 && sc->data[SC_STONE].val2==0)
			return 22;
		if( sc->data[SC_BENEDICTIO].timer!=-1 )	// êπëÃç~ïü
			return 26;
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
	struct status_change *sc =status_get_sc(bl);
	if(sc && sc->count) {
		if( sc->data[SC_WATERWEAPON].timer!=-1)	// ÉtÉçÉXÉgÉEÉFÉ|Éì
			return 1;
		if( sc->data[SC_EARTHWEAPON].timer!=-1)	// ÉTÉCÉYÉ~ÉbÉNÉEÉFÉ|Éì
			return 2;
		if( sc->data[SC_FIREWEAPON].timer!=-1)	// ÉtÉåÅ[ÉÄÉâÉìÉ`ÉÉÅ[
			return 3;
		if( sc->data[SC_WINDWEAPON].timer!=-1)	// ÉâÉCÉgÉjÉìÉOÉçÅ[É_Å[
			return 4;
		if( sc->data[SC_ENCPOISON].timer!=-1)	// ÉGÉìÉ`ÉÉÉìÉgÉ|ÉCÉYÉì
			return 5;
		if( sc->data[SC_ASPERSIO].timer!=-1)		// ÉAÉXÉyÉãÉVÉI
			return 6;
		if( sc->data[SC_SHADOWWEAPON].timer!=-1)
			return 7;
		if( sc->data[SC_GHOSTWEAPON].timer!=-1)
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
			if (((struct mob_data *)bl)->sc.data[SC_SWOO].timer != -1) // [marquis007]
				return 0;
			return ((struct mob_data *)bl)->db->size;
		case BL_PET:	
			return ((struct pet_data *)bl)->db->size;
		case BL_PC:
		{
			struct map_session_data *sd = (struct map_session_data *)bl;
			if (sd->sc.data[SC_SWOO].timer != -1)
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
	struct status_change *sc =status_get_sc(bl);
	if (bl->type == BL_PC &&
		((struct map_session_data *)bl)->special_state.no_magic_damage)
		return 1;
	if (sc && sc->count && sc->data[SC_HERMODE].timer != -1)
		return 1;
	return 0;
}

struct status_change *status_get_sc(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	if(bl->type==BL_MOB)
		return &((struct mob_data*)bl)->sc;
	if(bl->type==BL_PC)
		return &((struct map_session_data*)bl)->sc;
	return NULL;
}

//Returns defense against the specified status change.
//Return range is 0 (no resist) to 10000 (inmunity)
int status_get_sc_def(struct block_list *bl, int type)
{
	int sc_def;
	struct status_change* sc;
	nullpo_retr(0, bl);

	//Status that are blocked by Golden Thief Bug card or Wand of Hermod
	if (status_isimmune(bl))
	switch (type)
	{
	case SC_DECREASEAGI:
	case SC_SILENCE:
	case SC_COMA:
	case SC_INCREASEAGI:
	case SC_BLESSING:
	case SC_SLOWPOISON:
	case SC_IMPOSITIO:
	case SC_AETERNA:
	case SC_SUFFRAGIUM:
	case SC_BENEDICTIO:
	case SC_PROVIDENCE:
	case SC_KYRIE:
	case SC_ASSUMPTIO:
	case SC_ANGELUS:
	case SC_MAGNIFICAT:
	case SC_GLORIA:
	case SC_WINDWALK:
	case SC_MAGICROD:
	case SC_HALLUCINATION:
	case SC_STONE:
	case SC_QUAGMIRE:
		return 10000;
	}
	
	switch (type)
	{
	//Note that stats that are *100/3 were simplified to *33
	case SC_STONE:
	case SC_FREEZE:
	case SC_DECREASEAGI:
	case SC_COMA:
		sc_def = 300 +100*status_get_mdef(bl) +33*status_get_luk(bl);
		break;
	case SC_SLEEP:
	case SC_CONFUSION:
		sc_def = 300 +100*status_get_int(bl) +33*status_get_luk(bl);
		break;
// Removed since it collides with normal sc.
//	case SP_DEF1:	// def
//		sc_def = 300 +100*status_get_def(bl) +33*status_get_luk(bl);
//		break;
	case SC_STUN:
	case SC_POISON:
	case SC_DPOISON:
	case SC_SILENCE:
	case SC_BLEEDING:
	case SC_STOP:
		sc_def = 300 +100*status_get_vit(bl) +33*status_get_luk(bl);
		break;
	case SC_BLIND:
		sc_def = 300 +100*status_get_int(bl) +33*status_get_vit(bl);
		break;
	case SC_CURSE:
		sc_def = 300 +100*status_get_luk(bl) +33*status_get_vit(bl);
		break;
	default:
		return 0; //Effect that cannot be reduced? Likely a buff.
	}

	if (bl->type == BL_PC) {
		if (battle_config.pc_sc_def_rate != 100)
			sc_def = sc_def*battle_config.pc_sc_def_rate/100;
	} else
	if (battle_config.mob_sc_def_rate != 100)
		sc_def = sc_def*battle_config.mob_sc_def_rate/100;
	
	sc = status_get_sc(bl);
	if (sc && sc->count)
	{
		if (sc->data[SC_SCRESIST].timer != -1)
			sc_def += 100*sc->data[SC_SCRESIST].val1; //Status resist
		else if (sc->data[SC_SIEGFRIED].timer != -1)
			sc_def += 100*sc->data[SC_SIEGFRIED].val2; //Status resistance.
	}

	if(bl->type == BL_PC) {
		if (sc_def > battle_config.pc_max_sc_def)
			sc_def = battle_config.pc_max_sc_def;
	} else if (sc_def > battle_config.mob_max_sc_def)
		sc_def = battle_config.mob_max_sc_def;
	
	return sc_def;
}

//Reduces tick delay based on type and character defenses.
int status_get_sc_tick(struct block_list *bl, int type, int tick)
{
	struct map_session_data *sd;
	int rate=0, min=0;
	//If rate is positive, it is a % reduction (10000 -> 100%)
	//if it is negative, it is an absolute reduction in ms.
	sd = bl->type == BL_PC?(struct map_session_data *)bl:NULL;
	switch (type) {
		case SC_DECREASEAGI:		/* ë¨ìxå∏è≠ */
			if (sd)	// Celest
				tick>>=1;
		break;
		case SC_ADRENALINE:			/* ÉAÉhÉåÉiÉäÉìÉâÉbÉVÉÖ */
		case SC_ADRENALINE2:
		case SC_WEAPONPERFECTION:	/* ÉEÉFÉ|ÉìÉp?ÉtÉFÉNÉVÉáÉì */
		case SC_OVERTHRUST:			/* ÉI?Éo?ÉXÉâÉXÉg */
			if(sd && pc_checkskill(sd,BS_HILTBINDING)>0)
				tick += tick / 10;
		break;
		case SC_STONE:				/* êŒâª */
			rate = -200*status_get_mdef(bl);
		break;
		case SC_FREEZE:				/* ìÄåã */
			rate = 100*status_get_mdef(bl);
		break;
		case SC_STUN:	//Reduction in duration is the same as reduction in rate.
			rate = status_get_sc_def(bl, type);
		break;
		case SC_DPOISON:			/* ñ“ì≈ */
		case SC_POISON:				/* ì≈ */
			rate = 100*status_get_vit(bl) + 20*status_get_luk(bl);
		break;
		case SC_SILENCE:			/* íæ?ÅiÉåÉbÉNÉXÉfÉr?ÉiÅj */
		case SC_CONFUSION:
		case SC_CURSE:
			rate = 100*status_get_vit(bl);
		break;
		case SC_BLIND:				/* à√? */
			rate = 10*status_get_lv(bl) + 7*status_get_int(bl);
			min = 5000; //Minimum 5 secs?
		break;
		case SC_BLEEDING:
			rate = 20*status_get_lv(bl) +100*status_get_vit(bl);
			min = 10000; //Need a min of 10 secs for it to hurt at least once.
		break;
		case SC_SWOO:
			if (status_get_mode(bl)&MD_BOSS)
				tick /= 5; //TODO: Reduce skill's duration. But for how long?
		break;
		case SC_ANKLE:
			if(status_get_mode(bl)&MD_BOSS) // Lasts 5 times less on bosses
				tick /= 5;
			rate = -100*status_get_agi(bl);
		// Minimum trap time of 3+0.03*skilllv seconds [celest]
		// Changed to 3 secs and moved from skill.c [Skotlex]
			min = 3000;
		break;
		case SC_SPIDERWEB:
			if (map[bl->m].flag.pvp)
				tick /=2;
		break;
		case SC_STOP:
		// Unsure of this... but I get a feeling that agi reduces this
		// (it was on Tiger Fist Code, but at -1 ms per 10 agi....
			rate = -100*status_get_agi(bl);
		break;
	}
	if (rate) {
		if (bl->type == BL_PC) {
			if (battle_config.pc_sc_def_rate != 100)
				rate = rate*battle_config.pc_sc_def_rate/100;
			if (battle_config.pc_max_sc_def != 10000)
				min = tick*(10000-battle_config.pc_max_sc_def)/10000;
		} else {
			if (battle_config.mob_sc_def_rate != 100)
				rate = rate*battle_config.mob_sc_def_rate/100;
			if (battle_config.mob_max_sc_def != 10000)
				min = tick*(10000-battle_config.mob_max_sc_def)/10000;
		}
		
		if (rate >0)
			tick -= tick*rate/10000;
		else
			tick -= rate;
	}
	return tick<min?min:tick;
}
/*==========================================
 * Starts a status change.
 * type = type, val1~4 depend on the type.
 * rate = base success rate. 10000 = 100%
 * Tick is base duration
 * flag:
 * &1: Cannot be avoided (it has to start)
 * &2: Tick should not be reduced (by vit, luk, lv, etc)
 * &4: sc_data loaded, no value has to be altered.
 * &8: rate should not be reduced
 *------------------------------------------
 */
int status_change_start(struct block_list *bl,int type,int rate,int val1,int val2,int val3,int val4,int tick,int flag)
{
	struct map_session_data *sd = NULL;
	struct status_change* sc;
	int opt_flag , calc_flag = 0,updateflag = 0, save_flag = 0, race, mode, elem, undead_flag;

	nullpo_retr(0, bl);
	sc=status_get_sc(bl);

	if (!sc || status_isdead(bl))
		return 0;
	
	switch (bl->type)
	{
		case BL_PC:
			sd=(struct map_session_data *)bl;
			break;
		case BL_MOB:
			if (((struct mob_data*)bl)->class_ == MOBID_EMPERIUM && type != SC_SAFETYWALL)
				return 0; //Emperium can't be afflicted by status changes.
			break;
	}

	if(type < 0 || type >= SC_MAX) {
		if(battle_config.error_log)
			ShowError("status_change_start: invalid status change (%d)!\n", type);
		return 0;
	}

	//Check rate
	if (!(flag&(4|1))) {
		if (rate > 10000) //Shouldn't let this go above 100%
			rate = 10000;
		race = flag&8?0:status_get_sc_def(bl, type); //recycling race to store the sc_def value.
		//sd resistance applies even if the flag is &8
		if(sd && SC_COMMON_MIN<=type && type<=SC_COMMON_MAX && sd->reseff[type-SC_COMMON_MIN] > 0)
			race+= sd->reseff[type-SC_COMMON_MIN];

		if (race)
			rate -= rate*race/10000;

		if (!(rand()%10000 < rate))
			return 0;
	}
	
	race=status_get_race(bl);
	mode=status_get_mode(bl);
	elem=status_get_elem_type(bl);
	undead_flag=battle_check_undead(race,elem);

	//Check for inmunities / sc fails
	switch (type) {
		case SC_FREEZE:
		case SC_STONE:
			//Undead are inmune to Freeze/Stone
			if (undead_flag && !(flag&1))
				return 0;
		case SC_SLEEP:
		case SC_STUN:
			if (sc->opt1)
				return 0; //Cannot override other opt1 status changes. [Skotlex]
		break;
		case SC_CURSE:
			//Dark Elementals are inmune to curse.
			if (elem == 7 && !(flag&1))
				return 0;
		break;
		case SC_COMA:
			//Dark elementals and Demons are inmune to coma.
			if((elem == 7 || race == 6) && !(flag&1))
				return 0;
		break;
		case SC_SIGNUMCRUCIS:
			//Only affects demons and undead.
			if(race != 6 && !undead_flag)
				return 0;
			break;
		case SC_AETERNA:
		  if (sc->data[SC_STONE].timer != -1 || sc->data[SC_FREEZE].timer != -1)
			  return 0;
		break;
		case SC_OVERTHRUST:
			if (sc->data[SC_MAXOVERTHRUST].timer != -1)
				return 0; //Overthrust can't take effect if under Max Overthrust. [Skotlex]
		break;
		case SC_ADRENALINE:
		 	if (sd && !(skill_get_weapontype(BS_ADRENALINE)&(1<<sd->status.weapon)))
				return 0;
			if (sc->data[SC_QUAGMIRE].timer!=-1 ||
				sc->data[SC_DONTFORGETME].timer!=-1 ||
				sc->data[SC_DECREASEAGI].timer!=-1
			)
				return 0;
		break;
		case SC_ADRENALINE2:
			if (sd && !(skill_get_weapontype(BS_ADRENALINE2)&(1<<sd->status.weapon)))
				return 0;
			if (sc->data[SC_QUAGMIRE].timer!=-1 ||
				sc->data[SC_DONTFORGETME].timer!=-1 ||
				sc->data[SC_DECREASEAGI].timer!=-1
			)
				return 0;
		break;
		case SC_ONEHAND:
		case SC_TWOHANDQUICKEN:
			if(sc->data[SC_DECREASEAGI].timer!=-1)
				return 0;
		case SC_CONCENTRATE:
		case SC_INCREASEAGI:
		case SC_SPEARSQUICKEN:
		case SC_TRUESIGHT:
		case SC_WINDWALK:
		case SC_CARTBOOST:
		case SC_ASSNCROS:
			if (sc->data[SC_QUAGMIRE].timer!=-1 || sc->data[SC_DONTFORGETME].timer!=-1)
				return 0;
		break;
		case SC_CLOAKING:
		//Avoid cloaking with no wall and low skill level. [Skotlex]
		//Due to the cloaking card, we have to check the wall versus to known skill level rather than the used one. [Skotlex]
//			if (sd && skilllv < 3 && skill_check_cloaking(bl))
			if (sd && pc_checkskill(sd, AS_CLOAKING)< 3 && skill_check_cloaking(bl))
				return 0;
		break;
	}

	//Check for BOSS resistances
	if(mode & MD_BOSS && !(flag&1)) {
		 if (type>=SC_COMMON_MIN && type <= SC_COMMON_MAX)
			 return 0;
		 switch (type) {
			case SC_BLESSING:
			  if (!undead_flag || race != 6)
				  break;
			case SC_QUAGMIRE:
			case SC_DECREASEAGI:
			case SC_SIGNUMCRUCIS:
			case SC_PROVOKE:
			case SC_ROKISWEIL:
			case SC_COMA:
			case SC_GRAVITATION:
				return 0;
		}
	}

	//Check for overlapping fails
	if(sc->data[type].timer != -1){
		switch (type) {
			case SC_ADRENALINE:
			case SC_ADRENALINE2:
			case SC_WEAPONPERFECTION:
			case SC_OVERTHRUST:
				if (sc->data[type].val2 && !val2)
					return 0;
			break;
			case SC_GOSPEL:
				 //Must not override a casting gospel char.
				if (sc->data[type].val4 == BCT_SELF)
					return 0;
			case SC_STUN:
			case SC_SLEEP:
			case SC_POISON:
			case SC_CURSE:
			case SC_SILENCE:
			case SC_CONFUSION:
			case SC_BLIND:
			case SC_BLEEDING:
			case SC_DPOISON:
			case SC_COMBO: //You aren't supposed to change the combo (and it gets turned off when you trigger it)
			case SC_CLOSECONFINE2: //Can't be re-closed in.
				return 0;
			case SC_DANCING:
			case SC_DEVOTION:
			case SC_ASPDPOTION0:
			case SC_ASPDPOTION1:
			case SC_ASPDPOTION2:
			case SC_ASPDPOTION3:
			case SC_ATKPOTION:
			case SC_MATKPOTION:
				break;
			default:
				if(sc->data[type].val1 > val1)
					return 0;
			}
		(sc->count)--;
		delete_timer(sc->data[type].timer, status_change_timer);
		sc->data[type].timer = -1;
	}

	//SC duration reduction.
	if(!(flag&(2|4)) && tick) {
		tick = status_get_sc_tick(bl, type, tick);
		if (tick < 0)
			return 0;
	}

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
				if (sd && sd->status.hp<sd->status.max_hp>>2 &&
					(sc->data[SC_PROVOKE].timer==-1 || sc->data[SC_PROVOKE].val2==0))
					sc_start4(bl,SC_PROVOKE,100,10,1,0,0,0);
			}
			break;
		
		case SC_INCREASEAGI:		/* ë¨ìxè„è∏ */
			calc_flag = 1;
			if(sc->data[SC_DECREASEAGI].timer!=-1 )
				status_change_end(bl,SC_DECREASEAGI,-1);
			break;
		case SC_DECREASEAGI:		/* ë¨ìxå∏è≠ */
			calc_flag = 1;
			if(sc->data[SC_INCREASEAGI].timer!=-1 )
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc->data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc->data[SC_ADRENALINE2].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc->data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc->data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc->data[SC_CARTBOOST].timer!=-1 )
				status_change_end(bl,SC_CARTBOOST,-1);
			if(sc->data[SC_ONEHAND].timer!=-1 )
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
			if(sc->data[SC_ASPDPOTION0].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION0,-1);
			if(sc->data[SC_ASPDPOTION1].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION1,-1);
			if(sc->data[SC_ASPDPOTION2].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION2,-1);
			if(sc->data[SC_ASPDPOTION3].timer!=-1)
				status_change_end(bl,SC_ASPDPOTION3,-1);
			calc_flag = 1;
			break;
		case SC_MAXOVERTHRUST: //Cancels Normal Overthrust. [Skotlex]
			if (sc->data[SC_OVERTHRUST].timer != -1)
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
		case SC_EDP:	// [Celest]
			val2 = val1 + 2;			/* ñ“ì≈ït?ämó¶(%) */
			calc_flag = 1;
			break;
		case SC_POISONREACT:	/* É|ÉCÉYÉìÉäÉAÉNÉg */
			if (!(flag&4))
				val2=val1/2 + val1%2; // [Celest]
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
			if(sc->data[SC_ASSUMPTIO].timer!=-1 )
				status_change_end(bl,SC_ASSUMPTIO,-1);
			break;
		case SC_MINDBREAKER:
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	/* (ÉI?ÉgÉo?ÉT?ÉN) */
			break;
		case SC_QUAGMIRE:			/* ÉNÉ@ÉOÉ}ÉCÉA */
			calc_flag = 1;
			if(sc->data[SC_CONCENTRATE].timer!=-1 )	/* èWíÜóÕå¸è„âèú */
				status_change_end(bl,SC_CONCENTRATE,-1);
			if(sc->data[SC_INCREASEAGI].timer!=-1 )	/* ë¨ìxè„è∏âèú */
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc->data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc->data[SC_ONEHAND].timer!=-1 )
				status_change_end(bl,SC_ONEHAND,-1);
			if(sc->data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc->data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc->data[SC_ADRENALINE2].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc->data[SC_TRUESIGHT].timer!=-1 )	/* ÉgÉDÉã?ÉTÉCÉg */
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc->data[SC_WINDWALK].timer!=-1 )	/* ÉEÉCÉìÉhÉEÉH?ÉN */
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc->data[SC_CARTBOOST].timer!=-1 )	/* ÉJ?ÉgÉu?ÉXÉg */
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
		case SC_ENCPOISON:			/* ÉGÉìÉ`ÉÉÉìÉgÉ|ÉCÉYÉì */
			calc_flag = 1;
			val2=(((val1 - 1) / 2) + 3)*100;	/* ì≈ït?ämó¶ */
		case SC_ASPERSIO:			/* ÉAÉXÉyÉãÉVÉI */
		case SC_FIREWEAPON:		/* ÉtÉå?ÉÄÉâÉìÉ`ÉÉ? */
		case SC_WATERWEAPON:		/* ÉtÉçÉXÉgÉEÉFÉ|Éì */
		case SC_WINDWEAPON:	/* ÉâÉCÉgÉjÉìÉOÉç?É_? */
		case SC_EARTHWEAPON:		/* ÉTÉCÉYÉ~ÉbÉNÉEÉFÉ|Éì */
		case SC_SHADOWWEAPON:
		case SC_GHOSTWEAPON:
			skill_enchant_elemental_end(bl,type);
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
			if (sc->data[SC_FOGWALL].timer != -1 && sc->data[SC_BLIND].timer != -1)
				status_change_end(bl,SC_BLIND,-1);
			break;
		case SC_VIOLENTGALE:
			calc_flag = 1;
			val3 = val1*3;
			break;
		case SC_SUITON:
			calc_flag = 1;
			break;

		case SC_SPEARSQUICKEN:		/* ÉXÉsÉAÉNÉCÉbÉPÉì */
			calc_flag = 1;
			val2 = 20+val1;
			break;

		case SC_BLADESTOP:		/* îíênéÊÇË */
			if(val2==2) clif_bladestop((struct block_list *)val3,(struct block_list *)val4,1);
			break;

		case SC_DONTFORGETME:		/* éÑÇñYÇÍÇ»Ç¢Ç≈ */
			calc_flag = 1;
			if(sc->data[SC_INCREASEAGI].timer!=-1 )	/* ë¨ìxè„è∏âèú */
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc->data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc->data[SC_ONEHAND].timer!=-1 )
				status_change_end(bl,SC_ONEHAND,-1);
			if(sc->data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc->data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc->data[SC_ADRENALINE2].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc->data[SC_ASSNCROS].timer!=-1 )
				status_change_end(bl,SC_ASSNCROS,-1);
			if(sc->data[SC_TRUESIGHT].timer!=-1 )	/* ÉgÉDÉã?ÉTÉCÉg */
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc->data[SC_WINDWALK].timer!=-1 )	/* ÉEÉCÉìÉhÉEÉH?ÉN */
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc->data[SC_CARTBOOST].timer!=-1 )	/* ÉJ?ÉgÉu?ÉXÉg */
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
		if (sd)
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
			if (flag&4)
				break;
			val2 = 1;
			val3 = tick/1000;
			if(val3 < 1) val3 = 1;
			tick = 5000;
			break;
		case SC_SLEEP:				/* êáñ∞ */
			if(!(flag&4))
				tick = 30000;//êáñ∞ÇÕÉXÉe?É^ÉXëœê´Ç…?ÇÌÇÁÇ∏30ïb
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
			if (sc->data[SC_GOSPEL].timer!=-1) {
				if (sc->data[SC_GOSPEL].val4 == BCT_SELF) { //Clear Gospel [Skotlex]
					status_change_end(bl,SC_GOSPEL,-1);
				}
				break;
			}
			break;
		case SC_CONFUSION:
			clif_emotion(bl,1);
			break;
		case SC_BLEEDING:
			val4 = tick;
			tick = 10000;
			break;
		/* option */
		case SC_HIDING:		/* ÉnÉCÉfÉBÉìÉO */
			calc_flag = 1;
			if(sc->data[SC_CLOSECONFINE].timer != -1)
				status_change_end(bl, SC_CLOSECONFINE, -1);
			if(sc->data[SC_CLOSECONFINE2].timer != -1)
				status_change_end(bl, SC_CLOSECONFINE2, -1);
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
							status_change_start(&tsd->bl,SC_AUTOGUARD,10000,val1,val2,0,0,tick,1);
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
							status_change_start(&tsd->bl,SC_DEFENDER,10000,val1,val2,0,0,tick,1);
					}
			}
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
			if (val2 == 5) sc_start(bl,SC_BLEEDING,100,val1,skill_get_time2(StatusSkillChangeTable[type],val1));
			break;

		case SC_BERSERK:		/* Éo?ÉT?ÉN */
			if(battle_config.berserk_cancels_buffs)
			{
				if (sc->data[SC_ONEHAND].timer != -1)
					status_change_end(bl,SC_ONEHAND,-1);
				if (sc->data[SC_TWOHANDQUICKEN].timer != -1)
					status_change_end(bl,SC_TWOHANDQUICKEN,-1);
				if (sc->data[SC_CONCENTRATION].timer != -1)
					status_change_end(bl,SC_CONCENTRATION,-1);
				if (sc->data[SC_PARRYING].timer != -1)
					status_change_end(bl,SC_PARRYING,-1);
				if (sc->data[SC_ENDURE].timer != -1)
					status_change_end(bl,SC_ENDURE,-1);
				if (sc->data[SC_AURABLADE].timer != -1)
					status_change_end(bl,SC_AURABLADE,-1);
			}
			if(sd && !(flag&4)){
				sd->status.hp = sd->status.max_hp * 3;
				sd->status.sp = 0;
				clif_updatestatus(sd,SP_HP);
				clif_updatestatus(sd,SP_SP);
				sd->canregen_tick = gettick() + 300000;
			}
			if (!(flag&4))
				tick = 10000;
			calc_flag = 1;
			break;

		case SC_ASSUMPTIO:		/* ÉAÉXÉÄÉvÉeÉBÉI */
			if(sc->data[SC_KYRIE].timer!=-1)
				status_change_end(bl,SC_KYRIE,-1);
			break;

		case SC_WARM: //SG skills [Komurka]
			if (!(flag&4)) {
				val2 = tick/100;
				tick = 100;
			}
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
			if ((src = map_id2sd(val1)) && src->sc.count)
			{	//Try to inherit the status from the Crusader [Skotlex]
			//Ideally, we should calculate the remaining time and use that, but we'll trust that
			//once the Crusader's status changes, it will reflect on the others. 
				int type2 = SC_AUTOGUARD;
				if (src->sc.data[type2].timer != -1)
					sc_start(bl,type2,100,src->sc.data[type2].val1,skill_get_time(StatusSkillChangeTable[type2],src->sc.data[type2].val1));
				type2 = SC_DEFENDER;
				if (src->sc.data[type2].timer != -1)
					sc_start(bl,type2,100,src->sc.data[type2].val1,skill_get_time(StatusSkillChangeTable[type2],src->sc.data[type2].val1));
			}
			break;
		}

		case SC_COMA: //Coma. Sends a char to 1HP
			battle_damage(NULL, bl, status_get_hp(bl)-1, 0);
			return 1;

		case SC_CARTBOOST:		/* ÉJ?ÉgÉu?ÉXÉg */
			if(sc->data[SC_DECREASEAGI].timer!=-1 )
			{	//Cancel Decrease Agi, but take no further effect [Skotlex]
				status_change_end(bl,SC_DECREASEAGI,-1);
				return 0;
			}
			calc_flag = 1;
			break;

		case SC_CLOSECONFINE2:
			{
				struct block_list *src = val2?map_id2bl(val2):NULL;
				struct status_change *sc2 = src?status_get_sc(src):NULL;
				if (src && sc2 && sc2->count) {
					if (sc2->data[SC_CLOSECONFINE].timer == -1) //Start lock on caster.
						sc_start4(src,SC_CLOSECONFINE,100,sc->data[type].val1,1,0,0,tick+1000);
					else { //Increase count of locked enemies and refresh time.
						sc2->data[SC_CLOSECONFINE].val2++;
						delete_timer(sc2->data[SC_CLOSECONFINE].timer, status_change_timer);
						sc2->data[SC_CLOSECONFINE].timer = add_timer(gettick()+tick+1000, status_change_timer, src->id, SC_CLOSECONFINE);
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
		case SC_TKDORI:
			val2 = 11-val1; //Chance to consume: 11-skilllv%
			break;
		case SC_RUN:
			if (!(flag&4))
				val4 = gettick(); //Store time at which you started running.
			calc_flag = 1;
			break;
		case SC_BLESSING:
			if (bl->type==BL_PC || (!undead_flag && race!=6)) {
				if (sc->data[SC_CURSE].timer!=-1)
					status_change_end(bl,SC_CURSE,-1);
				if (sc->data[SC_STONE].timer!=-1 && sc->data[SC_STONE].val2==0)
					status_change_end(bl,SC_STONE,-1);
			}
		case SC_CONCENTRATION:	/* ÉRÉìÉZÉìÉgÉå?ÉVÉáÉì */case SC_ETERNALCHAOS:		/* ÉGÉ^?ÉiÉãÉJÉIÉX */
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
		case SC_ADRENALINE2:
		case SC_ADRENALINE:			/* ÉAÉhÉåÉiÉäÉìÉâÉbÉVÉÖ */
		case SC_BLIND:				/* à√? */
		case SC_CURSE:
		case SC_CONCENTRATE:		/* èWíÜóÕå¸è„ */
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
		case SC_SWOO: // [marquis007]
		case SC_STEELBODY:			// ã‡çÑ
		case SC_SKA:
		case SC_TWOHANDQUICKEN:		/* 2HQ */
		case SC_MIRACLE:
			calc_flag = 1;
			break;

		case SC_LULLABY:			/* éqéÁâS */
		case SC_RICHMANKIM:
		case SC_ROKISWEIL:			/* ÉçÉLÇÃã©Ç— */
		case SC_INTOABYSS:			/* ê[ï£ÇÃíÜÇ… */
		case SC_POEMBRAGI:			/* ÉuÉâÉMÇÃéç */
		case SC_UGLYDANCE:			/* é©ï™èüéËÇ»É_ÉìÉX */
		case SC_WEAPONPERFECTION:	/* ÉEÉFÉ|ÉìÉp?ÉtÉFÉNÉVÉáÉì */
		case SC_TRICKDEAD:			/* éÄÇÒÇæÇ”ÇË */
		case SC_FREEZE:				/* ìÄåã */
		case SC_STUN:				/* ÉXÉ^ÉìÅival2Ç…É~ÉäïbÉZÉbÉgÅj */
		case SC_ENERGYCOAT:			/* ÉGÉiÉW?ÉR?Ég */
		case SC_SAFETYWALL:
		case SC_OVERTHRUST:			/* ÉI?Éo?ÉXÉâÉXÉg */
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
		// gs_something1 [Vicious]
		case SC_MADNESSCANCEL:
		case SC_ADJUSTMENT:
		case SC_INCREASING:
		case SC_GATLINGFEVER:
		case SC_TATAMIGAESHI:
		case SC_KAENSIN:
			calc_flag = 1;
			break;
		case SC_UTSUSEMI:
		case SC_NEN:
			break;
	

		default:
			if(battle_config.error_log)
				ShowError("UnknownStatusChange [%d]\n", type);
			return 0;
	}

	//Those that make you stop attacking/walking....
	switch (type) {
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
		case SC_STONE:
			if (sd && pc_issit(sd)) //Avoid sprite sync problems.
				pc_setstand(sd);
		case SC_TRICKDEAD:
			battle_stopattack(bl);
			skill_stop_dancing(bl);	/* ââët/É_ÉìÉXÇÃíÜ? */
			// Cancel cast when get status [LuzZza]
			if (battle_config.sc_castcancel)
				skill_castcancel(bl, 0);
		case SC_STOP:
		case SC_CONFUSION:
		case SC_CLOSECONFINE:
		case SC_CLOSECONFINE2:
		case SC_ANKLE:
		case SC_SPIDERWEB:
		case SC_MADNESSCANCEL:
			battle_stopwalking(bl,1);
		break;
		case SC_HIDING:
		case SC_CLOAKING:
		case SC_CHASEWALK:
			battle_stopattack(bl);	/* çU?í‚é~ */
		break;
	}


	if (bl->type == BL_PC)
	{
		if (flag&4)
			clif_status_load(bl,StatusIconChangeTable[type],1); //Sending to owner since they aren't in the map yet. [Skotlex]
		clif_status_change(bl,StatusIconChangeTable[type],1);
	}

	// Set option as needed.
	opt_flag = 1;
	switch(type){
		//OPT1
		case SC_STONE:
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
			if(type == SC_STONE)
				sc->opt1 = OPT1_STONEWAIT;
			else
				sc->opt1 = OPT1_STONE + (type - SC_STONE);
			break;
		//OPT2
		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			sc->opt2 |= 1<<(type-SC_POISON);
			break;
		case SC_DPOISON:	// ébíËÇ≈ì≈ÇÃÉGÉtÉFÉNÉgÇégóp
			sc->opt2 |= OPT2_DPOISON;
			break;
		case SC_SIGNUMCRUCIS:
			sc->opt2 |= OPT2_SIGNUMCRUCIS;
			break;
		//OPT3
		case SC_TWOHANDQUICKEN:		/* 2HQ */
		case SC_SPEARSQUICKEN:		/* ÉXÉsÉAÉNÉCÉbÉPÉì */
		case SC_CONCENTRATION:	/* ÉRÉìÉZÉìÉgÉå?ÉVÉáÉì */
			sc->opt3 |= 1;
			opt_flag = 0;
			break;
		case SC_MAXOVERTHRUST:
		case SC_OVERTHRUST:			/* ÉI?Éo?ÉXÉâÉXÉg */
			sc->opt3 |= 2;
			opt_flag = 0;
			break;
		case SC_ENERGYCOAT:			/* ÉGÉiÉW?ÉR?Ég */
			sc->opt3 |= 4;
			opt_flag = 0;
			break;
		case SC_INCATKRATE:		/* ATK%è„è∏ */
			//Simulate Explosion Spirits effect for NPC_POWERUP [Skotlex]
			if (bl->type != BL_MOB) {
				opt_flag = 0;
				break;
			}
		case SC_EXPLOSIONSPIRITS:	// îöóÙîgìÆ
			sc->opt3 |= 8;
			opt_flag = 0;
			break;
		case SC_STEELBODY:			// ã‡çÑ
		case SC_SKA:
			sc->opt3 |= 16;
			opt_flag = 0;
			break;
		case SC_BLADESTOP:		/* îíênéÊÇË */
			sc->opt3 |= 32;
			opt_flag = 0;
			break;
		case SC_BERSERK:		/* Éo?ÉT?ÉN */
			sc->opt3 |= 128;
			opt_flag = 0;
			break;
		case SC_MARIONETTE:		/* É}ÉäÉIÉlÉbÉgÉRÉìÉgÉç?Éã */
		case SC_MARIONETTE2:
			sc->opt3 |= 1024;
			opt_flag = 0;
			break;
		case SC_ASSUMPTIO:		/* ÉAÉXÉÄÉvÉeÉBÉI */
			sc->opt3 |= 2048;
			opt_flag = 0;
			break;
		case SC_WARM: //SG skills [Komurka]
			sc->opt3 |= 4096;
			opt_flag = 0;
			break;
//		case SC_SWOO: // [marquis007]
//			sc->opt3 |= 8192; //We haven't figured out this value yet...
//			break;
			
		//OPTION
		case SC_HIDING:
			sc->option |= OPTION_HIDE;
			break;
		case SC_CLOAKING:
			sc->option |= OPTION_CLOAK;
			break;
		case SC_CHASEWALK:
			sc->option |= OPTION_CHASEWALK|OPTION_CLOAK;
			break;
		case SC_SIGHT:
			sc->option |= OPTION_SIGHT;
			break;
		case SC_RUWACH:
			sc->option |= OPTION_RUWACH;
			break;
		case SC_WEDDING:
			sc->option |= OPTION_WEDDING;
			break;
		case SC_ORCISH:
			sc->option |= OPTION_ORCISH;
			break;
		case SC_SIGHTTRASHER:
			sc->option |= OPTION_SIGHTTRASHER;
			break;
		case SC_FUSION:
			sc->option |= OPTION_FLYING;
			break;
		default:
			opt_flag = 0;
	}

	if(opt_flag)	/* optionÇÃ?çX */
		clif_changeoption(bl);

	(sc->count)++;	/* ÉXÉe?É^ÉXàŸèÌÇÃ? */

	sc->data[type].val1 = val1;
	sc->data[type].val2 = val2;
	sc->data[type].val3 = val3;
	sc->data[type].val4 = val4;
	/* É^ÉCÉ}?ê›íË */
	sc->data[type].timer = add_timer(
		gettick() + tick, status_change_timer, bl->id, type);

	if(sd) {
		if (calc_flag)
			status_calc_pc(sd,0);	/* ÉXÉe?É^ÉXçƒåvéZ */
		if(save_flag)
			chrif_save(sd,0); // save the player status
		if(updateflag)
			clif_updatestatus(sd,updateflag);	/* ÉXÉe?É^ÉXÇÉNÉâÉCÉAÉìÉgÇ…ëóÇÈ */
		if (sd->pd)
			pet_sc_check(sd, type); //Skotlex: Pet Status Effect Healing
		if (type==SC_RUN)
			pc_run(sd,val1,val2);
	}
	return 1;
}
/*==========================================
 * ÉXÉeÅ[É^ÉXàŸèÌëSâèú
 *------------------------------------------
 */
int status_change_clear(struct block_list *bl,int type)
{
	struct status_change* sc;
	int i;

	nullpo_retr(0, sc = status_get_sc(bl));

	if (sc->count == 0)
		return 0;
	for(i = 0; i < SC_MAX; i++)
	{
		//Type 0: PC killed -> EDP and Meltdown must not be dispelled. [Skotlex]
		// Do not reset Xmas status when killed. [Valaris]
		if(sc->data[i].timer == -1 ||
			(type == 0 &&
			(i == SC_EDP || i == SC_MELTDOWN || i == SC_XMAS || i == SC_NOCHAT)))
			continue;

		status_change_end(bl, i, -1);

		if (type == 1 && sc->data[i].timer != -1)
		{	//If for some reason status_change_end decides to still keep the status when quitting. [Skotlex]
			(sc->count)--;
			delete_timer(sc->data[i].timer, status_change_timer);
			sc->data[i].timer = -1;
		}
	}
	sc->opt1 = 0;
	sc->opt2 = 0;
	sc->opt3 = 0;
	sc->option &= OPTION_MASK;

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
	struct status_change *sc;
	int opt_flag=0, calc_flag = 0;

	nullpo_retr(0, bl);
	
	sc = status_get_sc(bl);
	if(!sc) {
		if(battle_config.error_log)
			ShowError("status_change_end: BL type %d doesn't has sc data!\n", bl->type);
		return 0;
	}

	if(type < 0 || type >= SC_MAX)
		return 0;

	sd = bl->type==BL_PC?(struct map_session_data *)bl:NULL;

	if (sc->data[type].timer != -1 && (sc->data[type].timer == tid || tid == -1)) {

		if (tid == -1)	// É^ÉCÉ}Ç©ÇÁåƒÇŒÇÍÇƒÇ¢Ç»Ç¢Ç»ÇÁÉ^ÉCÉ}çÌèúÇÇ∑ÇÈ
			delete_timer(sc->data[type].timer,status_change_timer);

		/* äY?ÇÃàŸèÌÇê≥èÌÇ…?Ç∑ */
		sc->data[type].timer=-1;
		(sc->count)--;

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
				if (sc->data[type].val1 >= 7 &&
					DIFF_TICK(gettick(), sc->data[type].val4) <= 1000 &&
					(!sd || (sd->weapontype1 == 0 && sd->weapontype2 == 0))
				)
					sc_start(bl,SC_SPURT,100,sc->data[type].val1,skill_get_time2(StatusSkillChangeTable[type], sc->data[type].val1));
				calc_flag = 1;
			break;
			case SC_AUTOBERSERK:
				if (sc->data[SC_PROVOKE].timer != -1 && sc->data[SC_PROVOKE].val2 == 1)
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
					if (sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) && tsd->sc.data[type].timer != -1)
						status_change_end(&tsd->bl,type,-1);
				}
			}
			break;
			case SC_DEVOTION:		/* ÉfÉBÉ{?ÉVÉáÉì */
				{
					struct map_session_data *md = map_id2sd(sc->data[type].val1);
					//The status could have changed because the Crusader left the game. [Skotlex]
					if (md)
					{
						md->devotion[sc->data[type].val2] = 0;
						clif_devotion(md);
					}
					//Remove AutoGuard and Defender [Skotlex]
					if (sc->data[SC_AUTOGUARD].timer != -1)
						status_change_end(bl,SC_AUTOGUARD,-1);
					if (sc->data[SC_DEFENDER].timer != -1)
						status_change_end(bl,SC_DEFENDER,-1);
				}
				break;
			case SC_BLADESTOP:
				{
					struct status_change *tsc = status_get_sc((struct block_list *)sc->data[type].val4);
					//ï–ï˚Ç™êÿÇÍÇΩÇÃÇ≈ëäéËÇÃîíên?ë‘Ç™êÿÇÍÇƒÇ»Ç¢ÇÃÇ»ÇÁâèú
					if(tsc && tsc->data[SC_BLADESTOP].timer!=-1)
						status_change_end((struct block_list *)sc->data[type].val4,SC_BLADESTOP,-1);

					if(sc->data[type].val2==2)
						clif_bladestop((struct block_list *)sc->data[type].val3,(struct block_list *)sc->data[type].val4,0);
				}
				break;
			case SC_DANCING:
				{
					struct map_session_data *dsd;
					struct status_change *dsc;
					if(sc->data[type].val2)
					{
						skill_delunitgroup((struct skill_unit_group *)sc->data[type].val2);
						sc->data[type].val2 = 0;
					}
					if(sc->data[type].val4 && sc->data[type].val4 != BCT_SELF && (dsd=map_id2sd(sc->data[type].val4))){
						dsc = &dsd->sc;
						//çáëtÇ≈ëäéËÇ™Ç¢ÇÈèÍçáëäéËÇÃval4Ç0Ç…Ç∑ÇÈ
						if(dsc && dsc->data[type].timer!=-1)
						{
							dsc->data[type].val2 = dsc->data[type].val4 = 0; //This will prevent recursive loops. 
							status_change_end(&dsd->bl, type, -1);
						}
					}
					if(sc->data[type].val1 == CG_MOONLIT) //Only dance that doesn't has ground tiles... [Skotlex]
						status_change_end(bl, SC_MOONLIT, -1);
				}
				if (sc->data[SC_LONGING].timer!=-1)
					status_change_end(bl,SC_LONGING,-1);				
				calc_flag = 1;
				break;
			case SC_NOCHAT:	//É`ÉÉÉbÉgã÷é~?ë‘
				if (sd) {
					if(battle_config.manner_system){
						//Why set it to 0? Can't we use good manners for something? [Skotlex]
//						if (sd->status.manner >= 0) // weeee ^^ [celest]
//							sd->status.manner = 0;
						clif_updatestatus(sd,SP_MANNER);
					}
				}
				break;
			case SC_SPLASHER:		/* ÉxÉiÉÄÉXÉvÉâÉbÉVÉÉ? */
				{
					struct block_list *src=map_id2bl(sc->data[type].val3);
					if(src && tid!=-1){
						//é©ï™Ç…É_ÉÅ?ÉWÅïé¸?3*3Ç…É_ÉÅ?ÉW
						skill_castend_damage_id(src, bl,sc->data[type].val2,sc->data[type].val1,gettick(),0 );
					}
				}
				break;
			case SC_CLOSECONFINE2:
				{
					struct block_list *src = sc->data[type].val2?map_id2bl(sc->data[type].val2):NULL;
					struct status_change *sc2 = src?status_get_sc(src):NULL;
					if (src && sc2 && sc2->count) {
						if (sc2->data[SC_CLOSECONFINE].timer != -1) //If status was already ended, do nothing.
						{ //Decrease count
							if (--sc2->data[SC_CLOSECONFINE].val1 <= 0) //No more holds, free him up.
								status_change_end(src, SC_CLOSECONFINE, -1);
						}
					}
				}
				break;
			case SC_CLOSECONFINE:
				if (sc->data[type].val2 > 0) { //Caster has been unlocked... nearby chars need to be unlocked.
					int range = 2*skill_get_range2(bl, StatusSkillChangeTable[type], sc->data[type].val1);
					map_foreachinarea(status_change_timer_sub, 
						bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,bl,sc,type,gettick());
				}
				break;
		/* option1 */
			case SC_FREEZE:
				sc->data[type].val3 = 0;
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
					struct block_list *pbl = map_id2bl(sc->data[type].val3);
					struct status_change* sc2 = pbl?status_get_sc(pbl):NULL;
					if (pbl && sc2 && sc2->count && sc2->data[type2].timer != -1)
							status_change_end(pbl, type2, -1);
					if (type == SC_MARIONETTE)
						clif_marionette(bl, 0); 
					calc_flag = 1;
				}
				break;

			case SC_BERSERK: //val4 indicates if the skill was dispelled. [Skotlex]
				if (sd && sd->status.hp > 100 && !sc->data[type].val4) {
					sd->status.hp = 100;
					clif_updatestatus(sd,SP_HP);
				}
				calc_flag = 1;
				break;
				
			case SC_GRAVITATION:
				if (sd) {
					if (sc->data[type].val3 == BCT_SELF) {
						unsigned int tick = gettick();
						sd->canmove_tick = tick;
						sd->canact_tick = tick;
					} else calc_flag = 1;
				}
				break;
			
			case SC_GOSPEL: //Clear the buffs from other chars.
				if(sc->data[type].val4 != BCT_SELF)
					calc_flag = 1;
				else if (sc->data[type].val3) { //Clear the group.
					struct skill_unit_group *group = (struct skill_unit_group *)sc->data[type].val3;
					sc->data[type].val3 = 0;
					skill_delunitgroup(group);
				}
				break;
			case SC_HERMODE: 
			case SC_BASILICA: //Clear the skill area. [Skotlex]
				if(sc->data[type].val3 == BCT_SELF)
					skill_clear_unitgroup(bl);
				break;
			case SC_MOONLIT: //Clear the unit effect. [Skotlex]
				skill_setmapcell(bl,CG_MOONLIT, sc->data[SC_MOONLIT].val1, CELL_CLRMOONLIT);
				break;
			//gs_something2 [Vicious]
			case SC_MADNESSCANCEL:
			case SC_ADJUSTMENT:
			case SC_INCREASING:
			case SC_GATLINGFEVER:
			case SC_TATAMIGAESHI:
			case SC_KAENSIN:
			case SC_SUITON:
				calc_flag = 1;
				break;
			case SC_UTSUSEMI:
			case SC_NEN:
				break;
			}


		if (sd)
			clif_status_change(bl,StatusIconChangeTable[type],0);

		switch(type){	/* ê≥èÌÇ…?ÇÈÇ∆Ç´Ç»Ç…Ç©?óùÇ™ïKóv */
		case SC_STONE:
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
			sc->opt1 = 0;
			opt_flag = 1;
			break;

		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			sc->opt2 &= ~(1<<(type-SC_POISON));
			opt_flag = 1;
			break;
		case SC_DPOISON:
			sc->opt2 &= ~OPT2_DPOISON;	// ì≈?ë‘âèú
			opt_flag = 1;
			break;
		case SC_SIGNUMCRUCIS:
			sc->opt2 &= ~OPT2_SIGNUMCRUCIS;
			opt_flag = 1;
			break;

		case SC_HIDING:
			sc->option &= ~OPTION_HIDE;
			opt_flag = 1 ;
			break;
		case SC_CLOAKING:
			sc->option &= ~OPTION_CLOAK;
			calc_flag = 1;	// orn
			opt_flag = 1 ;
			break;
		case SC_CHASEWALK:
			sc->option &= ~(OPTION_CHASEWALK|OPTION_CLOAK);
			opt_flag = 1 ;
			break;
		case SC_SIGHT:
			sc->option &= ~OPTION_SIGHT;
			opt_flag = 1;
			break;
		case SC_WEDDING:	//åãç•óp(åãç•àﬂè÷Ç…Ç»Ç¡Çƒ?Ç≠ÇÃÇ™?Ç¢Ç∆Ç©)
			sc->option &= ~OPTION_WEDDING;
			opt_flag = 1;
			break;
		case SC_ORCISH:
			sc->option &= ~OPTION_ORCISH;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			sc->option &= ~OPTION_RUWACH;
			opt_flag = 1;
			break;
		case SC_SIGHTTRASHER:
			sc->option &= ~OPTION_SIGHTTRASHER;
			opt_flag = 1;
			break;
		case SC_FUSION:
			sc->option &= ~OPTION_FLYING;
			opt_flag = 1;
			break;
		//opt3
		case SC_TWOHANDQUICKEN:		/* 2HQ */
		case SC_ONEHAND:		/* 1HQ */
		case SC_SPEARSQUICKEN:		/* ÉXÉsÉAÉNÉCÉbÉPÉì */
		case SC_CONCENTRATION:		/* ÉRÉìÉZÉìÉgÉå?ÉVÉáÉì */
			sc->opt3 &= ~1;
			break;
		case SC_OVERTHRUST:			/* ÉI?Éo?ÉXÉâÉXÉg */
			sc->opt3 &= ~2;
			break;
		case SC_ENERGYCOAT:			/* ÉGÉiÉW?ÉR?Ég */
			sc->opt3 &= ~4;
			break;
		case SC_INCATKRATE: //Simulated Explosion spirits effect.
			if (bl->type != BL_MOB)
				break;
		case SC_EXPLOSIONSPIRITS:	// îöóÙîgìÆ
			sc->opt3 &= ~8;
			break;
		case SC_STEELBODY:			// ã‡çÑ
		case SC_SKA:
			sc->opt3 &= ~16;
			break;
		case SC_BLADESTOP:		/* îíênéÊÇË */
			sc->opt3 &= ~32;
			break;
		case SC_BERSERK:		/* Éo?ÉT?ÉN */
			sc->opt3 &= ~128;
			break;
		case SC_MARIONETTE:		/* É}ÉäÉIÉlÉbÉgÉRÉìÉgÉç?Éã */
		case SC_MARIONETTE2:
			sc->opt3 &= ~1024;
			break;
		case SC_ASSUMPTIO:		/* ÉAÉXÉÄÉvÉeÉBÉI */
			sc->opt3 &= ~2048;
			break;
		case SC_WARM: //SG skills [Komurka]
			sc->opt3 &= ~4096;
			break;
		}

		if(opt_flag)	/* optionÇÃ?çXÇ?Ç¶ÇÈ */
			clif_changeoption(bl);

		if (sd && calc_flag)
			status_calc_pc((struct map_session_data *)bl,0);	/* ÉXÉe?É^ÉXçƒåvéZ */
	}

	return 1;
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
	struct status_change *sc;

// security system to prevent forgetting timer removal
	int temp_timerid;

	bl=map_id2bl(id);
#ifndef _WIN32
	nullpo_retr_f(0, bl, "id=%d data=%d",id,data);
#endif
	nullpo_retr(0, sc=status_get_sc(bl));

	if(bl->type==BL_PC)
		sd=(struct map_session_data *)bl;

	if(sc->data[type].timer != tid) {
		if(battle_config.error_log)
			ShowError("status_change_timer %d != %d\n",tid,sc->data[type].timer);
		return 0;
	}

	// security system to prevent forgetting timer removal
	// you shouldn't be that careless inside the switch here
	temp_timerid = sc->data[type].timer;
	sc->data[type].timer = -1;

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
			sc->data[type].timer=add_timer( /* É^ÉCÉ}?çƒê›íË */
				sc->data[type].val2+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_CHASEWALK:
		if(sd){
			int sp = 10+sc->data[type].val1*2;
			if (map_flag_gvg(sd->bl.m)) sp *= 5;
			if (pc_damage_sp(sd, sp, 0) > 0) {
				if ((++sc->data[type].val4) == 1) {
					sc_start(bl, SC_INCSTR,100,1<<(sc->data[type].val1-1),
						(sc->data[SC_SPIRIT].timer != -1 && sc->data[SC_SPIRIT].val2 == SL_ROGUE?10:1) //SL bonus -> x10 duration
						*skill_get_time2(StatusSkillChangeTable[type],sc->data[type].val1));
				}
				sc->data[type].timer = add_timer( /* É^ÉCÉ}?çƒê›íË */
					sc->data[type].val2+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
	break;

	case SC_HIDING:		/* ÉnÉCÉfÉBÉìÉO */
		if(sd){		/* SPÇ™Ç†Ç¡ÇƒÅAéûä‘êßå¿ÇÃä‘ÇÕéù? */
			if( sd->status.sp > 0 && (--sc->data[type].val2)>0 ){
				if(sc->data[type].val2 % (sc->data[type].val1+3) ==0 ){
					sd->status.sp--;
					clif_updatestatus(sd,SP_SP);
				}
				sc->data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
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
			map_foreachinrange( status_change_timer_sub, bl, 
				skill_get_splash(StatusSkillChangeTable[type], sc->data[type].val1),
				BL_CHAR, bl,sc,type,tick);

			if( (--sc->data[type].val2)>0 ){
				sc->data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
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
				sc->data[type].timer=add_timer(1000*600+tick,status_change_timer, bl->id, data );
				return 0;
			}
		}
		break;

	case SC_WARM: //SG skills [Komurka]
		if( (--sc->data[type].val2)>0){
			map_foreachinrange( status_change_timer_sub, bl,
				sc->data[type].val4,BL_CHAR,
				bl,sc,type,tick);
			sc->data[type].timer=add_timer(tick+100, status_change_timer,bl->id, data);
			return 0;
		}
		break;

	case SC_PROVOKE:	/* ÉvÉçÉ{ÉbÉN/ÉI?ÉgÉo?ÉT?ÉN */
		if(sc->data[type].val2!=0){	/* ÉI?ÉgÉo?ÉT?ÉNÅiÇPïbÇ≤Ç∆Ç…HPÉ`ÉFÉbÉNÅj */
			if(sd && sd->status.hp>sd->status.max_hp>>2)	/* í‚é~ */
				break;
			sc->data[type].timer=add_timer( 1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_ENDURE:	/* ÉCÉìÉfÉÖÉA */
		if(sd && sd->special_state.infinite_endure) {
			sc->data[type].timer=add_timer( 1000*60+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_STONE:
		if(sc->data[type].val2 != 0) {
			sc->data[type].val2 = 0;
			sc->data[type].val4 = 0;
			battle_stopwalking(bl,1);
			sc->opt1 = OPT1_STONE;
			clif_changeoption(bl);
			sc->data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		else if( (--sc->data[type].val3) > 0) {
			int hp = status_get_max_hp(bl);
			if((++sc->data[type].val4)%5 == 0 && status_get_hp(bl) > hp>>2) {
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
			sc->data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_POISON:
		if (status_get_hp(bl) <= status_get_max_hp(bl)>>2) //Stop damaging after 25% HP left.
			break;
	case SC_DPOISON:
		if ((--sc->data[type].val3) > 0 && sc->data[SC_SLOWPOISON].timer == -1) {
			if(sd) {
				pc_heal(sd, -sc->data[type].val4, 0);
			} else if (bl->type == BL_MOB) {
				((struct mob_data*)bl)->hp -= sc->data[type].val4;
				if (battle_config.show_mob_hp)
					clif_charnameack (0, bl);
			} else 
				battle_heal(NULL, bl, -sc->data[type].val4, 0, 1);
		}
		if (sc->data[type].val3 > 0 && !status_isdead(bl))
		{
			sc->data[type].timer = add_timer (1000 + tick, status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_TENSIONRELAX:	/* ÉeÉìÉVÉáÉìÉäÉâÉbÉNÉX */
		if(sd){		/* SPÇ™Ç†Ç¡ÇƒÅAHPÇ™?É^ÉìÇ≈Ç»ÇØÇÍÇŒ?? */
			if( sd->status.sp > 12 && sd->status.max_hp > sd->status.hp ){
				sc->data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
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
		if ((sc->data[type].val4 -= 10000) >= 0) {
			int hp = rand()%600 + 200;
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
				sc->data[type].timer = add_timer(10000 + tick, status_change_timer, bl->id, data );
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
	case SC_AUTOBERSERK: //continues until triggered off manually. [Skotlex]
		sc->data[type].timer=add_timer( 1000*600+tick,status_change_timer, bl->id, data );
		return 0;

	case SC_DANCING: //É_ÉìÉXÉXÉLÉãÇÃéûä‘SPè¡îÔ
		{
			int s = 0;
			int sp = 1;
			if (--sc->data[type].val3 <= 0)
				break;
			if(sd) {
				switch(sc->data[type].val1){
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
					sp= 4*sc->data[type].val2; //Moonlit's cost is 4sp*skill_lv [Skotlex]
					//Upkeep is also every 10 secs.
				case DC_DONTFORGETME:			/* éÑÇñYÇÍÇ»Ç¢Ç≈Åc 10ïbÇ≈SP1 */
					s=10;
					break;
				}
				if (s && ((sc->data[type].val3 % s) == 0)) {
					if (sc->data[SC_LONGING].timer != -1)
						sp = s;
					if (pc_damage_sp(sd, sp, 0) <= 0)
						break;
				}
			}
			sc->data[type].timer=add_timer(	/* É^ÉCÉ}?çƒê›íË */
				1000+tick, status_change_timer,
				bl->id, data);
			return 0;
		}
		break;

	case SC_DEVOTION:
		{	//Check range and timeleft to preserve status [Skotlex]
			//This implementation won't work for mobs because of map_id2sd, but it's a small cost in exchange of the speed of map_id2sd over map_id2sd
			struct map_session_data *md = map_id2sd(sc->data[type].val1);
			if (md && battle_check_range(bl, &md->bl, sc->data[type].val3) && (sc->data[type].val4-=1000)>0)
			{
				sc->data[type].timer = add_timer(1000+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;
		
	case SC_BERSERK:		/* Éo?ÉT?ÉN */
		if(sd){		/* HPÇ™100à»è„Ç»ÇÁ?? */
			if( (sd->status.hp - sd->status.max_hp*5/100) > 100 ){	// 5% every 10 seconds [DracoRPG]
				sd->status.hp -= sd->status.max_hp*5/100;	// changed to max hp [celest]
				clif_updatestatus(sd,SP_HP);
				sc->data[type].timer = add_timer(	/* É^ÉCÉ}?çƒê›íË */
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
				sc->data[type].timer=add_timer(60000+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SPLASHER:
		if (sc->data[type].val4 % 1000 == 0) {
			char timer[2];
			sprintf (timer, "%d", sc->data[type].val4/1000);
			clif_message(bl, timer);
		}
		if((sc->data[type].val4 -= 500) > 0) {
			sc->data[type].timer = add_timer(
				500 + tick, status_change_timer,
				bl->id, data);
				return 0;
		}
		break;

	case SC_MARIONETTE:		/* É}ÉäÉIÉlÉbÉgÉRÉìÉgÉç?Éã */
	case SC_MARIONETTE2:
		{
			struct block_list *pbl = map_id2bl(sc->data[type].val3);
			if (pbl && battle_check_range(bl, pbl, 7) &&
				(sc->data[type].val2 -= 1000)>0) {
				sc->data[type].timer = add_timer(
					1000 + tick, status_change_timer,
					bl->id, data);
					return 0;
			}
		}
		break;

	case SC_GOSPEL:
		if(sc->data[type].val4 == BCT_SELF){
			int hp, sp;
			hp = (sc->data[type].val1 > 5) ? 45 : 30;
			sp = (sc->data[type].val1 > 5) ? 35 : 20;
			if(status_get_hp(bl) - hp > 0 &&
				(sd == NULL || sd->status.sp - sp> 0))
			{
				if (sd)
					pc_heal(sd,-hp,-sp);
				else if (bl->type == BL_MOB)
					mob_heal((struct mob_data *)bl,-hp);
					
				if ((sc->data[type].val2 -= 10000) > 0) {
					sc->data[type].timer = add_timer(
					10000+tick, status_change_timer,
						bl->id, data);
					return 0;
				}
			}
		}
		break;
		
	case SC_GUILDAURA:
		{
			struct block_list *tbl = map_id2bl(sc->data[type].val2);
			
			if (tbl && battle_check_range(bl, tbl, 2)){
				sc->data[type].timer = add_timer(
					1000 + tick, status_change_timer,
					bl->id, data);
					return 0;
			}
		}
		break;
	// gs_status_change_timer [Vicious]
	case SC_NEN:
		sc->data[type].timer=add_timer( 1000*600+tick,status_change_timer, bl->id, data );
		return 0;
	}

	// default for all non-handled control paths
	// security system to prevent forgetting timer removal

	// if we reach this point we need the timer for the next call, 
	// so restore it to have status_change_end handle a valid timer
	sc->data[type].timer = temp_timerid; 

	return status_change_end( bl,type,tid );
}

/*==========================================
 * ÉXÉeÅ[É^ÉXàŸèÌÉ^ÉCÉ}Å[îÕàÕèàóù
 *------------------------------------------
 */
int status_change_timer_sub(struct block_list *bl, va_list ap )
{
	struct block_list *src;
	struct status_change *sc, *tsc;
	struct map_session_data* sd=NULL;
	struct map_session_data* tsd=NULL;

	int type;
	unsigned int tick;

	src=va_arg(ap,struct block_list*);
	sc=va_arg(ap,struct status_change*);
	type=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);
	tsc=status_get_sc(bl);
	
	if (status_isdead(bl))
		return 0;
	if (src->type==BL_PC) sd= (struct map_session_data*)src;
	if (bl->type==BL_PC) tsd= (struct map_session_data*)bl;

	switch( type ){
	case SC_SIGHT:	/* ÉTÉCÉg */
	case SC_CONCENTRATE:
		if (tsc && tsc->count) {
			if (tsc->data[SC_HIDING].timer != -1)
				status_change_end( bl, SC_HIDING, -1);
			if (tsc->data[SC_CLOAKING].timer != -1)
				status_change_end( bl, SC_CLOAKING, -1);
		}
		break;
	case SC_RUWACH:	/* ÉãÉAÉt */
		if (tsc && tsc->count && (tsc->data[SC_HIDING].timer != -1 ||	// if the target is using a special hiding, i.e not using normal hiding/cloaking, don't bother
			tsc->data[SC_CLOAKING].timer != -1)) {
			status_change_end( bl, SC_HIDING, -1);
			status_change_end( bl, SC_CLOAKING, -1);
			if(battle_check_target( src, bl, BCT_ENEMY ) > 0)
				skill_attack(BF_MAGIC,src,src,bl,AL_RUWACH,1,tick,0);
		}
		break;
	case SC_SIGHTBLASTER:
		{
			if (sc && sc->count && sc->data[type].val2 > 0 && battle_check_target( src, bl, BCT_ENEMY ) > 0)
			{	//sc_ check prevents a single round of Sight Blaster hitting multiple opponents. [Skotlex]
				skill_attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,1,tick,0);
				sc->data[type].val2 = 0; //This signals it to end.
			}
		}
		break;
	case SC_WARM: //SG skills [Komurka]
		if(sc && sc->data[type].val2 &&
			battle_check_target( src,bl, BCT_ENEMY ) > 0)
	 	{
			if(tsd)
				//Only damage SP [Skotlex]
				pc_damage_sp(tsd, 60, 0);
			else { //Otherwise, Knockback attack.
				if(sd && pc_damage_sp(sd, 2, 0) <= 0)
					sd->sc.data[type].val2 = 0; //Makes it end on the next tick.
				skill_attack(BF_WEAPON,src,src,bl,sc->data[type].val3,sc->data[type].val1,tick,0);
			}
		}
		break;
	case SC_CLOSECONFINE:
		//Lock char has released the hold on everyone...
		if (tsc && tsc->count && tsc->data[SC_CLOSECONFINE2].timer != -1 && tsc->data[SC_CLOSECONFINE2].val2 == src->id) {
			tsc->data[SC_CLOSECONFINE2].val2 = 0;
			status_change_end(bl, SC_CLOSECONFINE2, -1);
		}
		break;
	}
	return 0;
}

int status_change_clear_buffs (struct block_list *bl)
{
	int i;
	struct status_change *sc= status_get_sc(bl);
	if (!sc || !sc->count)
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
		if(sc->data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	return 0;
}
int status_change_clear_debuffs (struct block_list *bl)
{
	int i;
	struct status_change *sc = status_get_sc(bl);
	if (!sc || !sc->count)
		return 0;
	for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++) {
		if(sc->data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	//Other ailments not in the common range.
	if(sc->data[SC_HALLUCINATION].timer != -1)
		status_change_end(bl,SC_HALLUCINATION,-1);
	if(sc->data[SC_QUAGMIRE].timer != -1)
		status_change_end(bl,SC_QUAGMIRE,-1);
	if(sc->data[SC_SIGNUMCRUCIS].timer != -1)
		status_change_end(bl,SC_SIGNUMCRUCIS,-1);
	if(sc->data[SC_DECREASEAGI].timer != -1)
		status_change_end(bl,SC_DECREASEAGI,-1);
	if(sc->data[SC_SLOWDOWN].timer != -1)
		status_change_end(bl,SC_SLOWDOWN,-1);
	if(sc->data[SC_MINDBREAKER].timer != -1)
		status_change_end(bl,SC_MINDBREAKER,-1);
	if(sc->data[SC_WINKCHARM].timer != -1)
		status_change_end(bl,SC_WINKCHARM,-1);
	if(sc->data[SC_STOP].timer != -1)
		status_change_end(bl,SC_STOP,-1);
	if(sc->data[SC_ORCISH].timer != -1)
		status_change_end(bl,SC_ORCISH,-1);
	if(sc->data[SC_STRIPWEAPON].timer != -1)
		status_change_end(bl,SC_STRIPWEAPON,-1);
	if(sc->data[SC_STRIPSHIELD].timer != -1)
		status_change_end(bl,SC_STRIPSHIELD,-1);
	if(sc->data[SC_STRIPARMOR].timer != -1)
		status_change_end(bl,SC_STRIPARMOR,-1);
	if(sc->data[SC_STRIPHELM].timer != -1)
		status_change_end(bl,SC_STRIPHELM,-1);
	return 0;
}

static int status_calc_sigma(void)
{
	int i,j;
	unsigned int k;

	for(i=0;i<MAX_PC_CLASS;i++) {
		memset(hp_sigma_val[i],0,sizeof(hp_sigma_val[i]));
		for(k=0,j=2;j<=MAX_LEVEL;j++) {
			k += hp_coefficient[i]*j + 50;
			k -= k%100;
			hp_sigma_val[i][j-1] = k;
			if (k >= INT_MAX)
				break; //Overflow protection. [Skotlex]
		}
		for(;j<=MAX_LEVEL;j++)
			hp_sigma_val[i][j-1] = INT_MAX;
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
		char *split[MAX_WEAPON_TYPE + 6];
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j< (MAX_WEAPON_TYPE + 6) && p;j++){	//not 22 anymore [blackhole89]
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j < MAX_WEAPON_TYPE + 6)	//Weapon #.MAX_WEAPON_TYPE is constantly not load. Fix to that: replace < with <= [blackhole89]
			continue;
		if(atoi(split[0])>=MAX_PC_CLASS)
			continue;
		max_weight_base[atoi(split[0])]=atoi(split[1]);
		hp_coefficient[atoi(split[0])]=atoi(split[2]);
		hp_coefficient2[atoi(split[0])]=atoi(split[3]);
		sp_coefficient[atoi(split[0])]=atoi(split[4]);
		for(j=0;j<=MAX_WEAPON_TYPE;j++)
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
		for(j=0;j<=MAX_WEAPON_TYPE;j++)
			atkmods[i][j]=100;
	sprintf(path, "%s/size_fix.txt", db_path);
	fp=fopen(path,"r");
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		char *split[MAX_WEAPON_TYPE+1];
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<=MAX_WEAPON_TYPE && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		for(j=0;j<=MAX_WEAPON_TYPE && split[j];j++)
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
	initChangeTables();
	status_readdb();
	status_calc_sigma();
	return 0;
}
