// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"
#include "../common/utils.h"
#include "../common/ers.h"

#include "map.h"
#include "path.h"
#include "pc.h"
#include "pet.h"
#include "npc.h"
#include "mob.h"
#include "clif.h"
#include "guild.h"
#include "skill.h"
#include "itemdb.h"
#include "battle.h"
#include "chrif.h"
#include "skill.h"
#include "status.h"
#include "script.h"
#include "unit.h"
#include "homunculus.h"
#include "mercenary.h"
#include "vending.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>


//Regen related flags.
#define RGN_HP	0x01
#define RGN_SP	0x02
#define RGN_SHP	0x04
#define RGN_SSP	0x08

static int max_weight_base[CLASS_COUNT];
static int hp_coefficient[CLASS_COUNT];
static int hp_coefficient2[CLASS_COUNT];
static int hp_sigma_val[CLASS_COUNT][MAX_LEVEL+1];
static int sp_coefficient[CLASS_COUNT];
static int aspd_base[CLASS_COUNT][MAX_WEAPON_TYPE];	//[blackhole89]
static int refinebonus[MAX_REFINE_BONUS][3];	// 精錬ボーナステーブル(refine_db.txt)
int percentrefinery[5][MAX_REFINE+1];	// 精錬成功率(refine_db.txt)
static int atkmods[3][MAX_WEAPON_TYPE];	// 武器ATKサイズ修正(size_fix.txt)
static char job_bonus[CLASS_COUNT][MAX_LEVEL];

static struct eri *sc_data_ers; //For sc_data entries
static struct status_data dummy_status;

int current_equip_item_index; //Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
int current_equip_card_id; //To prevent card-stacking (from jA) [Skotlex]
//we need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only
//to avoid cards exploits

static sc_type SkillStatusChangeTable[MAX_SKILL]; // skill  -> status
static int StatusIconChangeTable[SC_MAX];         // status -> icon
unsigned long StatusChangeFlagTable[SC_MAX];      // status -> flags
static int StatusSkillChangeTable[SC_MAX];        // status -> skill

sc_type status_skill2sc(int skill)
{
	int sk = skill_get_index(skill);
	if( sk == 0 ) {
		ShowError("status_skill2sc: Unsupported skill id %d\n", skill);
		return SC_NONE;
	}
	return SkillStatusChangeTable[sk];
}

int status_sc2skill(sc_type sc)
{
	if( sc < 0 || sc >= SC_MAX ) {
		ShowError("status_skill2sc: Unsupported status change id %d\n", sc);
		return 0;
	}

	return StatusSkillChangeTable[sc];
}

#define add_sc(skill,sc) set_sc(skill,sc,SI_BLANK,SCB_NONE)

static void set_sc(int skill, sc_type sc, int icon, unsigned int flag)
{
	int sk = skill_get_index(skill);
	if( sk == 0 ) {
		ShowError("set_sc: Unsupported skill id %d\n", skill);
		return;
	}
	if( sc < 0 || sc >= SC_MAX ) {
		ShowError("set_sc: Unsupported status change id %d\n", sc);
		return;
	}

	if( StatusSkillChangeTable[sc] == 0 )
  		StatusSkillChangeTable[sc] = skill;
	if( StatusIconChangeTable[sc] == SI_BLANK )
  		StatusIconChangeTable[sc] = icon;
	StatusChangeFlagTable[sc] |= flag;

	if( SkillStatusChangeTable[sk] == SC_NONE )
		SkillStatusChangeTable[sk] = sc;
}

void initChangeTables(void)
{
	int i;
	for (i = 0; i < SC_MAX; i++)
		StatusIconChangeTable[i] = SI_BLANK;
	for (i = 0; i < MAX_SKILL; i++)
		SkillStatusChangeTable[i] = SC_NONE;

	memset(StatusSkillChangeTable, 0, sizeof(StatusSkillChangeTable));
	memset(StatusChangeFlagTable, 0, sizeof(StatusChangeFlagTable));

	//First we define the skill for common ailments. These are used in skill_additional_effect through sc cards. [Skotlex]
	set_sc( NPC_PETRIFYATTACK , SC_STONE     , SI_BLANK    , SCB_DEF_ELE|SCB_DEF|SCB_MDEF );
	set_sc( NPC_WIDEFREEZE    , SC_FREEZE    , SI_BLANK    , SCB_DEF_ELE|SCB_DEF|SCB_MDEF );
	set_sc( NPC_STUNATTACK    , SC_STUN      , SI_BLANK    , SCB_NONE );
	set_sc( NPC_SLEEPATTACK   , SC_SLEEP     , SI_BLANK    , SCB_NONE );
	set_sc( NPC_POISON        , SC_POISON    , SI_BLANK    , SCB_DEF2|SCB_REGEN );
	set_sc( NPC_CURSEATTACK   , SC_CURSE     , SI_BLANK    , SCB_LUK|SCB_BATK|SCB_WATK|SCB_SPEED );
	set_sc( NPC_SILENCEATTACK , SC_SILENCE   , SI_BLANK    , SCB_NONE );
	set_sc( NPC_WIDECONFUSE   , SC_CONFUSION , SI_BLANK    , SCB_NONE );
	set_sc( NPC_BLINDATTACK   , SC_BLIND     , SI_BLANK    , SCB_HIT|SCB_FLEE );
	set_sc( NPC_BLEEDING      , SC_BLEEDING  , SI_BLEEDING , SCB_REGEN );
	set_sc( NPC_POISON        , SC_DPOISON   , SI_BLANK    , SCB_DEF2|SCB_REGEN );

	//The main status definitions
	add_sc( SM_BASH              , SC_STUN            );
	set_sc( SM_PROVOKE           , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	add_sc( SM_MAGNUM            , SC_WATK_ELEMENT    );
	set_sc( SM_ENDURE            , SC_ENDURE          , SI_ENDURE          , SCB_MDEF|SCB_DSPD );
	add_sc( MG_SIGHT             , SC_SIGHT           );
	add_sc( MG_SAFETYWALL        , SC_SAFETYWALL      );
	add_sc( MG_FROSTDIVER        , SC_FREEZE          );
	add_sc( MG_STONECURSE        , SC_STONE           );
	add_sc( AL_RUWACH            , SC_RUWACH          );
	add_sc( AL_PNEUMA            , SC_PNEUMA          );
	set_sc( AL_INCAGI            , SC_INCREASEAGI     , SI_INCREASEAGI     , SCB_AGI|SCB_SPEED );
	set_sc( AL_DECAGI            , SC_DECREASEAGI     , SI_DECREASEAGI     , SCB_AGI|SCB_SPEED );
	set_sc( AL_CRUCIS            , SC_SIGNUMCRUCIS    , SI_SIGNUMCRUCIS    , SCB_DEF );
	set_sc( AL_ANGELUS           , SC_ANGELUS         , SI_ANGELUS         , SCB_DEF2 );
	set_sc( AL_BLESSING          , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( AC_CONCENTRATION     , SC_CONCENTRATE     , SI_CONCENTRATE     , SCB_AGI|SCB_DEX );
	set_sc( TF_HIDING            , SC_HIDING          , SI_HIDING          , SCB_SPEED );
	add_sc( TF_POISON            , SC_POISON          );
	set_sc( KN_TWOHANDQUICKEN    , SC_TWOHANDQUICKEN  , SI_TWOHANDQUICKEN  , SCB_ASPD );
	add_sc( KN_AUTOCOUNTER       , SC_AUTOCOUNTER     );
	set_sc( PR_IMPOSITIO         , SC_IMPOSITIO       , SI_IMPOSITIO       , SCB_WATK );
	set_sc( PR_SUFFRAGIUM        , SC_SUFFRAGIUM      , SI_SUFFRAGIUM      , SCB_NONE );
	set_sc( PR_ASPERSIO          , SC_ASPERSIO        , SI_ASPERSIO        , SCB_ATK_ELE );
	set_sc( PR_BENEDICTIO        , SC_BENEDICTIO      , SI_BENEDICTIO      , SCB_DEF_ELE );
	set_sc( PR_SLOWPOISON        , SC_SLOWPOISON      , SI_SLOWPOISON      , SCB_REGEN );
	set_sc( PR_KYRIE             , SC_KYRIE           , SI_KYRIE           , SCB_NONE );
	set_sc( PR_MAGNIFICAT        , SC_MAGNIFICAT      , SI_MAGNIFICAT      , SCB_REGEN );
	set_sc( PR_GLORIA            , SC_GLORIA          , SI_GLORIA          , SCB_LUK );
	add_sc( PR_LEXDIVINA         , SC_SILENCE         );
	set_sc( PR_LEXAETERNA        , SC_AETERNA         , SI_AETERNA         , SCB_NONE );
	add_sc( WZ_METEOR            , SC_STUN            );
	add_sc( WZ_VERMILION         , SC_BLIND           );
	add_sc( WZ_FROSTNOVA         , SC_FREEZE          );
	add_sc( WZ_STORMGUST         , SC_FREEZE          );
	set_sc( WZ_QUAGMIRE          , SC_QUAGMIRE        , SI_QUAGMIRE        , SCB_AGI|SCB_DEX|SCB_ASPD|SCB_SPEED );
	set_sc( BS_ADRENALINE        , SC_ADRENALINE      , SI_ADRENALINE      , SCB_ASPD );
	set_sc( BS_WEAPONPERFECT     , SC_WEAPONPERFECTION, SI_WEAPONPERFECTION, SCB_NONE );
	set_sc( BS_OVERTHRUST        , SC_OVERTHRUST      , SI_OVERTHRUST      , SCB_NONE );
	set_sc( BS_MAXIMIZE          , SC_MAXIMIZEPOWER   , SI_MAXIMIZEPOWER   , SCB_NONE );
	add_sc( HT_LANDMINE          , SC_STUN            );
	add_sc( HT_ANKLESNARE        , SC_ANKLE           );
	add_sc( HT_SANDMAN           , SC_SLEEP           );
	add_sc( HT_FLASHER           , SC_BLIND           );
	add_sc( HT_FREEZINGTRAP      , SC_FREEZE          );
	set_sc( AS_CLOAKING          , SC_CLOAKING        , SI_CLOAKING        , SCB_CRI|SCB_SPEED );
	add_sc( AS_SONICBLOW         , SC_STUN            );
	set_sc( AS_ENCHANTPOISON     , SC_ENCPOISON       , SI_ENCPOISON       , SCB_ATK_ELE );
	set_sc( AS_POISONREACT       , SC_POISONREACT     , SI_POISONREACT     , SCB_NONE );
	add_sc( AS_VENOMDUST         , SC_POISON          );
	add_sc( AS_SPLASHER          , SC_SPLASHER        );
	set_sc( NV_TRICKDEAD         , SC_TRICKDEAD       , SI_TRICKDEAD       , SCB_REGEN );
	set_sc( SM_AUTOBERSERK       , SC_AUTOBERSERK     , SI_AUTOBERSERK     , SCB_NONE );
	add_sc( TF_SPRINKLESAND      , SC_BLIND           );
	add_sc( TF_THROWSTONE        , SC_STUN            );
	set_sc( MC_LOUD              , SC_LOUD            , SI_LOUD            , SCB_STR );
	set_sc( MG_ENERGYCOAT        , SC_ENERGYCOAT      , SI_ENERGYCOAT      , SCB_NONE );
	set_sc( NPC_EMOTION          , SC_MODECHANGE      , SI_BLANK           , SCB_MODE );
	add_sc( NPC_EMOTION_ON       , SC_MODECHANGE      );
	set_sc( NPC_ATTRICHANGE      , SC_ELEMENTALCHANGE , SI_ARMOR_PROPERTY  , SCB_DEF_ELE );
	add_sc( NPC_CHANGEWATER      , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEGROUND     , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEFIRE       , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEWIND       , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEPOISON     , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEHOLY       , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEDARKNESS   , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGETELEKINESIS, SC_ELEMENTALCHANGE );
	add_sc( NPC_POISON           , SC_POISON          );
	add_sc( NPC_BLINDATTACK      , SC_BLIND           );
	add_sc( NPC_SILENCEATTACK    , SC_SILENCE         );
	add_sc( NPC_STUNATTACK       , SC_STUN            );
	add_sc( NPC_PETRIFYATTACK    , SC_STONE           );
	add_sc( NPC_CURSEATTACK      , SC_CURSE           );
	add_sc( NPC_SLEEPATTACK      , SC_SLEEP           );
	add_sc( NPC_MAGICALATTACK    , SC_MAGICALATTACK   );
	set_sc( NPC_KEEPING          , SC_KEEPING         , SI_BLANK           , SCB_DEF );
	add_sc( NPC_DARKBLESSING     , SC_COMA            );
	set_sc( NPC_BARRIER          , SC_BARRIER         , SI_BLANK           , SCB_MDEF|SCB_DEF );
	add_sc( NPC_DEFENDER         , SC_ARMOR           );
	add_sc( NPC_LICK             , SC_STUN            );
	set_sc( NPC_HALLUCINATION    , SC_HALLUCINATION   , SI_HALLUCINATION   , SCB_NONE );
	add_sc( NPC_REBIRTH          , SC_REBIRTH         );
	add_sc( RG_RAID              , SC_STUN            );
	set_sc( RG_STRIPWEAPON       , SC_STRIPWEAPON     , SI_STRIPWEAPON     , SCB_WATK );
	set_sc( RG_STRIPSHIELD       , SC_STRIPSHIELD     , SI_STRIPSHIELD     , SCB_DEF );
	set_sc( RG_STRIPARMOR        , SC_STRIPARMOR      , SI_STRIPARMOR      , SCB_VIT );
	set_sc( RG_STRIPHELM         , SC_STRIPHELM       , SI_STRIPHELM       , SCB_INT );
	add_sc( AM_ACIDTERROR        , SC_BLEEDING        );
	set_sc( AM_CP_WEAPON         , SC_CP_WEAPON       , SI_CP_WEAPON       , SCB_NONE );
	set_sc( AM_CP_SHIELD         , SC_CP_SHIELD       , SI_CP_SHIELD       , SCB_NONE );
	set_sc( AM_CP_ARMOR          , SC_CP_ARMOR        , SI_CP_ARMOR        , SCB_NONE );
	set_sc( AM_CP_HELM           , SC_CP_HELM         , SI_CP_HELM         , SCB_NONE );
	set_sc( CR_AUTOGUARD         , SC_AUTOGUARD       , SI_AUTOGUARD       , SCB_NONE );
	add_sc( CR_SHIELDCHARGE      , SC_STUN            );
	set_sc( CR_REFLECTSHIELD     , SC_REFLECTSHIELD   , SI_REFLECTSHIELD   , SCB_NONE );
	add_sc( CR_HOLYCROSS         , SC_BLIND           );
	add_sc( CR_GRANDCROSS        , SC_BLIND           );
	add_sc( CR_DEVOTION          , SC_DEVOTION        );
	set_sc( CR_PROVIDENCE        , SC_PROVIDENCE      , SI_PROVIDENCE      , SCB_ALL );
	set_sc( CR_DEFENDER          , SC_DEFENDER        , SI_DEFENDER        , SCB_SPEED|SCB_ASPD );
	set_sc( CR_SPEARQUICKEN      , SC_SPEARQUICKEN    , SI_SPEARQUICKEN    , SCB_ASPD );
	set_sc( MO_STEELBODY         , SC_STEELBODY       , SI_STEELBODY       , SCB_DEF|SCB_MDEF|SCB_ASPD|SCB_SPEED );
	add_sc( MO_BLADESTOP         , SC_BLADESTOP_WAIT  );
	add_sc( MO_BLADESTOP         , SC_BLADESTOP       );
	set_sc( MO_EXPLOSIONSPIRITS  , SC_EXPLOSIONSPIRITS, SI_EXPLOSIONSPIRITS, SCB_CRI|SCB_REGEN );
	set_sc( MO_EXTREMITYFIST     , SC_EXTREMITYFIST   , SI_BLANK           , SCB_REGEN );
	add_sc( SA_MAGICROD          , SC_MAGICROD        );
	set_sc( SA_AUTOSPELL         , SC_AUTOSPELL       , SI_AUTOSPELL       , SCB_NONE );
	set_sc( SA_FLAMELAUNCHER     , SC_FIREWEAPON      , SI_FIREWEAPON      , SCB_ATK_ELE );
	set_sc( SA_FROSTWEAPON       , SC_WATERWEAPON     , SI_WATERWEAPON     , SCB_ATK_ELE );
	set_sc( SA_LIGHTNINGLOADER   , SC_WINDWEAPON      , SI_WINDWEAPON      , SCB_ATK_ELE );
	set_sc( SA_SEISMICWEAPON     , SC_EARTHWEAPON     , SI_EARTHWEAPON     , SCB_ATK_ELE );
	set_sc( SA_VOLCANO           , SC_VOLCANO         , SI_LANDENDOW       , SCB_WATK );
	set_sc( SA_DELUGE            , SC_DELUGE          , SI_LANDENDOW       , SCB_MAXHP );
	set_sc( SA_VIOLENTGALE       , SC_VIOLENTGALE     , SI_LANDENDOW       , SCB_FLEE );
	add_sc( SA_REVERSEORCISH     , SC_ORCISH          );
	add_sc( SA_COMA              , SC_COMA            );
	set_sc( BD_ENCORE            , SC_DANCING         , SI_BLANK           , SCB_SPEED|SCB_REGEN );
	add_sc( BD_RICHMANKIM        , SC_RICHMANKIM      );
	set_sc( BD_ETERNALCHAOS      , SC_ETERNALCHAOS    , SI_BLANK           , SCB_DEF2 );
	set_sc( BD_DRUMBATTLEFIELD   , SC_DRUMBATTLE      , SI_BLANK           , SCB_WATK|SCB_DEF );
	set_sc( BD_RINGNIBELUNGEN    , SC_NIBELUNGEN      , SI_BLANK           , SCB_WATK );
	add_sc( BD_ROKISWEIL         , SC_ROKISWEIL       );
	add_sc( BD_INTOABYSS         , SC_INTOABYSS       );
	set_sc( BD_SIEGFRIED         , SC_SIEGFRIED       , SI_BLANK           , SCB_ALL );
	add_sc( BA_FROSTJOKER        , SC_FREEZE          );
	set_sc( BA_WHISTLE           , SC_WHISTLE         , SI_BLANK           , SCB_FLEE|SCB_FLEE2 );
	set_sc( BA_ASSASSINCROSS     , SC_ASSNCROS        , SI_BLANK           , SCB_ASPD );
	add_sc( BA_POEMBRAGI         , SC_POEMBRAGI       );
	set_sc( BA_APPLEIDUN         , SC_APPLEIDUN       , SI_BLANK           , SCB_MAXHP );
	add_sc( DC_SCREAM            , SC_STUN            );
	set_sc( DC_HUMMING           , SC_HUMMING         , SI_BLANK           , SCB_HIT );
	set_sc( DC_DONTFORGETME      , SC_DONTFORGETME    , SI_BLANK           , SCB_SPEED|SCB_ASPD );
	set_sc( DC_FORTUNEKISS       , SC_FORTUNE         , SI_BLANK           , SCB_CRI );
	set_sc( DC_SERVICEFORYOU     , SC_SERVICE4U       , SI_BLANK           , SCB_ALL );
	add_sc( NPC_DARKCROSS        , SC_BLIND           );
	add_sc( NPC_GRANDDARKNESS    , SC_BLIND           );
	set_sc( NPC_STOP             , SC_STOP            , SI_STOP            , SCB_NONE );
	set_sc( NPC_WEAPONBRAKER     , SC_BROKENWEAPON    , SI_BROKENWEAPON    , SCB_NONE );
	set_sc( NPC_ARMORBRAKE       , SC_BROKENARMOR     , SI_BROKENARMOR     , SCB_NONE );
	set_sc( NPC_CHANGEUNDEAD     , SC_CHANGEUNDEAD    , SI_UNDEAD          , SCB_DEF_ELE );
	set_sc( NPC_POWERUP          , SC_INCHITRATE      , SI_BLANK           , SCB_HIT );
	set_sc( NPC_AGIUP            , SC_INCFLEERATE     , SI_BLANK           , SCB_FLEE );
	add_sc( NPC_INVISIBLE        , SC_CLOAKING        );
	set_sc( LK_AURABLADE         , SC_AURABLADE       , SI_AURABLADE       , SCB_NONE );
	set_sc( LK_PARRYING          , SC_PARRYING        , SI_PARRYING        , SCB_NONE );
	set_sc( LK_CONCENTRATION     , SC_CONCENTRATION   , SI_CONCENTRATION   , SCB_BATK|SCB_WATK|SCB_HIT|SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_DSPD );
	set_sc( LK_TENSIONRELAX      , SC_TENSIONRELAX    , SI_TENSIONRELAX    , SCB_REGEN );
	set_sc( LK_BERSERK           , SC_BERSERK         , SI_BERSERK         , SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_MDEF2|SCB_FLEE|SCB_SPEED|SCB_ASPD|SCB_MAXHP|SCB_REGEN );
	set_sc( HP_ASSUMPTIO         , SC_ASSUMPTIO       , SI_ASSUMPTIO       , SCB_NONE );
	add_sc( HP_BASILICA          , SC_BASILICA        );
	set_sc( HW_MAGICPOWER        , SC_MAGICPOWER      , SI_MAGICPOWER      , SCB_MATK );
	add_sc( PA_SACRIFICE         , SC_SACRIFICE       );
	set_sc( PA_GOSPEL            , SC_GOSPEL          , SI_BLANK           , SCB_SPEED|SCB_ASPD );
	add_sc( PA_GOSPEL            , SC_SCRESIST        );
	add_sc( CH_TIGERFIST         , SC_STOP            );
	set_sc( ASC_EDP              , SC_EDP             , SI_EDP             , SCB_NONE );
	set_sc( SN_SIGHT             , SC_TRUESIGHT       , SI_TRUESIGHT       , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK|SCB_CRI|SCB_HIT );
	set_sc( SN_WINDWALK          , SC_WINDWALK        , SI_WINDWALK        , SCB_FLEE|SCB_SPEED );
	set_sc( WS_MELTDOWN          , SC_MELTDOWN        , SI_MELTDOWN        , SCB_NONE );
	set_sc( WS_CARTBOOST         , SC_CARTBOOST       , SI_CARTBOOST       , SCB_SPEED );
	set_sc( ST_CHASEWALK         , SC_CHASEWALK       , SI_BLANK           , SCB_SPEED );
	set_sc( ST_REJECTSWORD       , SC_REJECTSWORD     , SI_REJECTSWORD     , SCB_NONE );
	add_sc( ST_REJECTSWORD       , SC_AUTOCOUNTER     );
	set_sc( CG_MARIONETTE        , SC_MARIONETTE      , SI_MARIONETTE      , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	set_sc( CG_MARIONETTE        , SC_MARIONETTE2     , SI_MARIONETTE2     , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	add_sc( LK_SPIRALPIERCE      , SC_STOP            );
	add_sc( LK_HEADCRUSH         , SC_BLEEDING        );
	set_sc( LK_JOINTBEAT         , SC_JOINTBEAT       , SI_JOINTBEAT       , SCB_BATK|SCB_DEF2|SCB_SPEED|SCB_ASPD );
	add_sc( HW_NAPALMVULCAN      , SC_CURSE           );
	set_sc( PF_MINDBREAKER       , SC_MINDBREAKER     , SI_BLANK           , SCB_MATK|SCB_MDEF2 );
	add_sc( PF_MEMORIZE          , SC_MEMORIZE        );
	add_sc( PF_FOGWALL           , SC_FOGWALL         );
	set_sc( PF_SPIDERWEB         , SC_SPIDERWEB       , SI_BLANK           , SCB_FLEE );
	set_sc( WE_BABY              , SC_BABY            , SI_BABY            , SCB_NONE );
	set_sc( TK_RUN               , SC_RUN             , SI_RUN             , SCB_SPEED|SCB_DSPD );
	set_sc( TK_RUN               , SC_SPURT           , SI_SPURT           , SCB_STR );
	set_sc( TK_READYSTORM        , SC_READYSTORM      , SI_READYSTORM      , SCB_NONE );
	set_sc( TK_READYDOWN         , SC_READYDOWN       , SI_READYDOWN       , SCB_NONE );
	add_sc( TK_DOWNKICK          , SC_STUN            );
	set_sc( TK_READYTURN         , SC_READYTURN       , SI_READYTURN       , SCB_NONE );
	set_sc( TK_READYCOUNTER      , SC_READYCOUNTER    , SI_READYCOUNTER    , SCB_NONE );
	set_sc( TK_DODGE             , SC_DODGE           , SI_DODGE           , SCB_NONE );
	set_sc( TK_SPTIME            , SC_EARTHSCROLL     , SI_EARTHSCROLL     , SCB_NONE );
	add_sc( TK_SEVENWIND         , SC_SEVENWIND ); 
	set_sc( TK_SEVENWIND         , SC_GHOSTWEAPON     , SI_GHOSTWEAPON     , SCB_ATK_ELE ); 
	set_sc( TK_SEVENWIND         , SC_SHADOWWEAPON    , SI_SHADOWWEAPON    , SCB_ATK_ELE ); 
	set_sc( SG_SUN_WARM          , SC_WARM            , SI_WARM            , SCB_NONE );
	add_sc( SG_MOON_WARM         , SC_WARM            );
	add_sc( SG_STAR_WARM         , SC_WARM            );
	set_sc( SG_SUN_COMFORT       , SC_SUN_COMFORT     , SI_SUN_COMFORT     , SCB_DEF2 );
	set_sc( SG_MOON_COMFORT      , SC_MOON_COMFORT    , SI_MOON_COMFORT    , SCB_FLEE );
	set_sc( SG_STAR_COMFORT      , SC_STAR_COMFORT    , SI_STAR_COMFORT    , SCB_ASPD );
	add_sc( SG_FRIEND            , SC_SKILLRATE_UP    );
	set_sc( SG_KNOWLEDGE         , SC_KNOWLEDGE       , SI_BLANK           , SCB_ALL );
	set_sc( SG_FUSION            , SC_FUSION          , SI_BLANK           , SCB_SPEED );
	set_sc( BS_ADRENALINE2       , SC_ADRENALINE2     , SI_ADRENALINE2     , SCB_ASPD );
	set_sc( SL_KAIZEL            , SC_KAIZEL          , SI_KAIZEL          , SCB_NONE );
	set_sc( SL_KAAHI             , SC_KAAHI           , SI_KAAHI           , SCB_NONE );
	set_sc( SL_KAUPE             , SC_KAUPE           , SI_KAUPE           , SCB_NONE );
	set_sc( SL_KAITE             , SC_KAITE           , SI_KAITE           , SCB_NONE );
	add_sc( SL_STUN              , SC_STUN            );
	set_sc( SL_SWOO              , SC_SWOO            , SI_BLANK           , SCB_SPEED );
	set_sc( SL_SKE               , SC_SKE             , SI_BLANK           , SCB_BATK|SCB_WATK|SCB_DEF|SCB_DEF2 );
	set_sc( SL_SKA               , SC_SKA             , SI_BLANK           , SCB_DEF|SCB_MDEF|SCB_ASPD );
	set_sc( SL_SMA               , SC_SMA             , SI_SMA             , SCB_NONE );
	set_sc( SM_SELFPROVOKE       , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	set_sc( ST_PRESERVE          , SC_PRESERVE        , SI_PRESERVE        , SCB_NONE );
	set_sc( PF_DOUBLECASTING     , SC_DOUBLECAST      , SI_DOUBLECAST      , SCB_NONE );
	set_sc( HW_GRAVITATION       , SC_GRAVITATION     , SI_BLANK           , SCB_ASPD );
	add_sc( WS_CARTTERMINATION   , SC_STUN            );
	set_sc( WS_OVERTHRUSTMAX     , SC_MAXOVERTHRUST   , SI_MAXOVERTHRUST   , SCB_NONE );
	set_sc( CG_LONGINGFREEDOM    , SC_LONGING         , SI_BLANK           , SCB_SPEED|SCB_ASPD );
	add_sc( CG_HERMODE           , SC_HERMODE         );
	set_sc( ITEM_ENCHANTARMS     , SC_ENCHANTARMS     , SI_BLANK           , SCB_ATK_ELE );
	set_sc( SL_HIGH              , SC_SPIRIT          , SI_SPIRIT          , SCB_ALL );
	set_sc( KN_ONEHAND           , SC_ONEHAND         , SI_ONEHAND         , SCB_ASPD );
	set_sc( GS_FLING             , SC_FLING           , SI_BLANK           , SCB_DEF|SCB_DEF2 );
	add_sc( GS_CRACKER           , SC_STUN            );
	add_sc( GS_DISARM            , SC_STRIPWEAPON     );
	add_sc( GS_PIERCINGSHOT      , SC_BLEEDING        );
	set_sc( GS_MADNESSCANCEL     , SC_MADNESSCANCEL   , SI_MADNESSCANCEL   , SCB_BATK|SCB_ASPD );
	set_sc( GS_ADJUSTMENT        , SC_ADJUSTMENT      , SI_ADJUSTMENT      , SCB_HIT|SCB_FLEE );
	set_sc( GS_INCREASING        , SC_INCREASING      , SI_ACCURACY        , SCB_AGI|SCB_DEX|SCB_HIT );
	set_sc( GS_GATLINGFEVER      , SC_GATLINGFEVER    , SI_GATLINGFEVER    , SCB_BATK|SCB_FLEE|SCB_SPEED|SCB_ASPD );
	set_sc( NJ_TATAMIGAESHI      , SC_TATAMIGAESHI    , SI_BLANK           , SCB_NONE );
	set_sc( NJ_SUITON            , SC_SUITON          , SI_BLANK           , SCB_AGI|SCB_SPEED );
	add_sc( NJ_HYOUSYOURAKU      , SC_FREEZE          );
	set_sc( NJ_NEN               , SC_NEN             , SI_NEN             , SCB_STR|SCB_INT );
	set_sc( NJ_UTSUSEMI          , SC_UTSUSEMI        , SI_UTSUSEMI        , SCB_NONE );
	set_sc( NJ_BUNSINJYUTSU      , SC_BUNSINJYUTSU    , SI_BUNSINJYUTSU    , SCB_DYE );

	add_sc( NPC_ICEBREATH        , SC_FREEZE          );
	add_sc( NPC_ACIDBREATH       , SC_POISON          );
	add_sc( NPC_HELLJUDGEMENT    , SC_CURSE           );
	add_sc( NPC_WIDESILENCE      , SC_SILENCE         );
	add_sc( NPC_WIDEFREEZE       , SC_FREEZE          );
	add_sc( NPC_WIDEBLEEDING     , SC_BLEEDING        );
	add_sc( NPC_WIDESTONE        , SC_STONE           );
	add_sc( NPC_WIDECONFUSE      , SC_CONFUSION       );
	add_sc( NPC_WIDESLEEP        , SC_SLEEP           );
	add_sc( NPC_WIDESIGHT        , SC_SIGHT           );
	add_sc( NPC_EVILLAND         , SC_BLIND           );
	add_sc( NPC_MAGICMIRROR      , SC_MAGICMIRROR     );
	set_sc( NPC_SLOWCAST         , SC_SLOWCAST        , SI_SLOWCAST        , SCB_NONE );
	set_sc( NPC_CRITICALWOUND    , SC_CRITICALWOUND   , SI_CRITICALWOUND   , SCB_NONE );
	set_sc( NPC_STONESKIN        , SC_ARMORCHANGE     , SI_BLANK           , SCB_DEF|SCB_MDEF );
	add_sc( NPC_ANTIMAGIC        , SC_ARMORCHANGE     );
	add_sc( NPC_WIDECURSE        , SC_CURSE           );
	add_sc( NPC_WIDESTUN         , SC_STUN            );

	set_sc( NPC_HELLPOWER        , SC_HELLPOWER       , SI_HELLPOWER       , SCB_NONE );
	set_sc( NPC_WIDEHELLDIGNITY  , SC_HELLPOWER       , SI_HELLPOWER       , SCB_NONE );
	set_sc( NPC_INVINCIBLE       , SC_INVINCIBLE      , SI_INVINCIBLE      , SCB_SPEED );
	set_sc( NPC_INVINCIBLEOFF    , SC_INVINCIBLEOFF   , SI_BLANK           , SCB_SPEED );

	set_sc( CASH_BLESSING        , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( CASH_INCAGI          , SC_INCREASEAGI     , SI_INCREASEAGI     , SCB_AGI|SCB_SPEED );
	set_sc( CASH_ASSUMPTIO       , SC_ASSUMPTIO       , SI_ASSUMPTIO       , SCB_NONE );

	//set_sc( ALL_PARTYFLEE        , SC_INCFLEE         , SI_PARTYFLEE       , SCB_NONE );

	set_sc( CR_SHRINK            , SC_SHRINK          , SI_SHRINK          , SCB_NONE );
	set_sc( RG_CLOSECONFINE      , SC_CLOSECONFINE2   , SI_CLOSECONFINE2   , SCB_NONE );
	set_sc( RG_CLOSECONFINE      , SC_CLOSECONFINE    , SI_CLOSECONFINE    , SCB_FLEE );
	set_sc( WZ_SIGHTBLASTER      , SC_SIGHTBLASTER    , SI_SIGHTBLASTER    , SCB_NONE );
	set_sc( DC_WINKCHARM         , SC_WINKCHARM       , SI_WINKCHARM       , SCB_NONE );
	add_sc( MO_BALKYOUNG         , SC_STUN            );
	add_sc( SA_ELEMENTWATER      , SC_ELEMENTALCHANGE );
	add_sc( SA_ELEMENTFIRE       , SC_ELEMENTALCHANGE );
	add_sc( SA_ELEMENTGROUND     , SC_ELEMENTALCHANGE );
	add_sc( SA_ELEMENTWIND       , SC_ELEMENTALCHANGE );

	set_sc( HLIF_AVOID           , SC_AVOID           , SI_BLANK           , SCB_SPEED );
	set_sc( HLIF_CHANGE          , SC_CHANGE          , SI_BLANK           , SCB_VIT|SCB_INT );
	set_sc( HFLI_FLEET           , SC_FLEET           , SI_BLANK           , SCB_ASPD|SCB_BATK|SCB_WATK );
	set_sc( HFLI_SPEED           , SC_SPEED           , SI_BLANK           , SCB_FLEE );
	set_sc( HAMI_DEFENCE         , SC_DEFENCE         , SI_BLANK           , SCB_DEF );
	set_sc( HAMI_BLOODLUST       , SC_BLOODLUST       , SI_BLANK           , SCB_BATK|SCB_WATK );

	add_sc( MER_CRASH            , SC_STUN            );
	set_sc( MER_PROVOKE          , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	add_sc( MS_MAGNUM            , SC_WATK_ELEMENT    );
	add_sc( MER_SIGHT            , SC_SIGHT           );
	set_sc( MER_DECAGI           , SC_DECREASEAGI     , SI_DECREASEAGI     , SCB_AGI|SCB_SPEED );
	set_sc( MER_MAGNIFICAT       , SC_MAGNIFICAT      , SI_MAGNIFICAT      , SCB_REGEN );
	add_sc( MER_LEXDIVINA        , SC_SILENCE         );
	add_sc( MA_LANDMINE          , SC_STUN            );
	add_sc( MA_SANDMAN           , SC_SLEEP           );
	add_sc( MA_FREEZINGTRAP      , SC_FREEZE          );
	set_sc( MER_AUTOBERSERK      , SC_AUTOBERSERK     , SI_AUTOBERSERK     , SCB_NONE );
	set_sc( ML_AUTOGUARD         , SC_AUTOGUARD       , SI_AUTOGUARD       , SCB_NONE );
	set_sc( MS_REFLECTSHIELD     , SC_REFLECTSHIELD   , SI_REFLECTSHIELD   , SCB_NONE );
	set_sc( ML_DEFENDER          , SC_DEFENDER        , SI_DEFENDER        , SCB_SPEED|SCB_ASPD );
	set_sc( MS_PARRYING          , SC_PARRYING        , SI_PARRYING        , SCB_NONE );
	set_sc( MS_BERSERK           , SC_BERSERK         , SI_BERSERK         , SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_MDEF2|SCB_FLEE|SCB_SPEED|SCB_ASPD|SCB_MAXHP|SCB_REGEN );
	add_sc( ML_SPIRALPIERCE      , SC_STOP            );
	set_sc( MER_QUICKEN          , SC_MERC_QUICKEN    , SI_BLANK           , SCB_ASPD );
	add_sc( ML_DEVOTION          , SC_DEVOTION        );
	set_sc( MER_KYRIE            , SC_KYRIE           , SI_KYRIE           , SCB_NONE );
	set_sc( MER_BLESSING         , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( MER_INCAGI           , SC_INCREASEAGI     , SI_INCREASEAGI     , SCB_AGI|SCB_SPEED );

	set_sc( GD_LEADERSHIP        , SC_GUILDAURA       , SI_BLANK           , SCB_STR|SCB_AGI|SCB_VIT|SCB_DEX );
	set_sc( GD_BATTLEORDER       , SC_BATTLEORDERS    , SI_BLANK           , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( GD_REGENERATION      , SC_REGENERATION    , SI_BLANK           , SCB_REGEN );

	// Storing the target job rather than simply SC_SPIRIT simplifies code later on.
	SkillStatusChangeTable[SL_ALCHEMIST]   = (sc_type)MAPID_ALCHEMIST,
	SkillStatusChangeTable[SL_MONK]        = (sc_type)MAPID_MONK,
	SkillStatusChangeTable[SL_STAR]        = (sc_type)MAPID_STAR_GLADIATOR,
	SkillStatusChangeTable[SL_SAGE]        = (sc_type)MAPID_SAGE,
	SkillStatusChangeTable[SL_CRUSADER]    = (sc_type)MAPID_CRUSADER,
	SkillStatusChangeTable[SL_SUPERNOVICE] = (sc_type)MAPID_SUPER_NOVICE,
	SkillStatusChangeTable[SL_KNIGHT]      = (sc_type)MAPID_KNIGHT,
	SkillStatusChangeTable[SL_WIZARD]      = (sc_type)MAPID_WIZARD,
	SkillStatusChangeTable[SL_PRIEST]      = (sc_type)MAPID_PRIEST,
	SkillStatusChangeTable[SL_BARDDANCER]  = (sc_type)MAPID_BARDDANCER,
	SkillStatusChangeTable[SL_ROGUE]       = (sc_type)MAPID_ROGUE,
	SkillStatusChangeTable[SL_ASSASIN]     = (sc_type)MAPID_ASSASSIN,
	SkillStatusChangeTable[SL_BLACKSMITH]  = (sc_type)MAPID_BLACKSMITH,
	SkillStatusChangeTable[SL_HUNTER]      = (sc_type)MAPID_HUNTER,
	SkillStatusChangeTable[SL_SOULLINKER]  = (sc_type)MAPID_SOUL_LINKER,

	//Status that don't have a skill associated.
	StatusIconChangeTable[SC_WEIGHT50] = SI_WEIGHT50;
	StatusIconChangeTable[SC_WEIGHT90] = SI_WEIGHT90;
	StatusIconChangeTable[SC_ASPDPOTION0] = SI_ASPDPOTION0;
	StatusIconChangeTable[SC_ASPDPOTION1] = SI_ASPDPOTION1;
	StatusIconChangeTable[SC_ASPDPOTION2] = SI_ASPDPOTION2;
	StatusIconChangeTable[SC_ASPDPOTION3] = SI_ASPDPOTIONINFINITY;
	StatusIconChangeTable[SC_SPEEDUP0] = SI_MOVHASTE_HORSE;
	StatusIconChangeTable[SC_SPEEDUP1] = SI_SPEEDPOTION1;
	StatusIconChangeTable[SC_INCSTR] = SI_INCSTR;
	StatusIconChangeTable[SC_MIRACLE] = SI_SPIRIT;
	StatusIconChangeTable[SC_INTRAVISION] = SI_INTRAVISION;
	StatusIconChangeTable[SC_STRFOOD] = SI_FOODSTR;
	StatusIconChangeTable[SC_AGIFOOD] = SI_FOODAGI;
	StatusIconChangeTable[SC_VITFOOD] = SI_FOODVIT;
	StatusIconChangeTable[SC_INTFOOD] = SI_FOODINT;
	StatusIconChangeTable[SC_DEXFOOD] = SI_FOODDEX;
	StatusIconChangeTable[SC_LUKFOOD] = SI_FOODLUK;
	StatusIconChangeTable[SC_FLEEFOOD]= SI_FOODFLEE;
	StatusIconChangeTable[SC_HITFOOD] = SI_FOODHIT;
	StatusIconChangeTable[SC_MANU_ATK] = SI_MANU_ATK;
	StatusIconChangeTable[SC_MANU_DEF] = SI_MANU_DEF;
	StatusIconChangeTable[SC_SPL_ATK] = SI_SPL_ATK;
	StatusIconChangeTable[SC_SPL_DEF] = SI_SPL_DEF;
	StatusIconChangeTable[SC_MANU_MATK] = SI_MANU_MATK;
	StatusIconChangeTable[SC_SPL_MATK] = SI_SPL_MATK;
	//Cash Items
	StatusIconChangeTable[SC_FOOD_STR_CASH] = SI_FOOD_STR_CASH;
	StatusIconChangeTable[SC_FOOD_AGI_CASH] = SI_FOOD_AGI_CASH;
	StatusIconChangeTable[SC_FOOD_VIT_CASH] = SI_FOOD_VIT_CASH;
	StatusIconChangeTable[SC_FOOD_DEX_CASH] = SI_FOOD_DEX_CASH;
	StatusIconChangeTable[SC_FOOD_INT_CASH] = SI_FOOD_INT_CASH;
	StatusIconChangeTable[SC_FOOD_LUK_CASH] = SI_FOOD_LUK_CASH;
	StatusIconChangeTable[SC_EXPBOOST] = SI_EXPBOOST;
	StatusIconChangeTable[SC_ITEMBOOST] = SI_ITEMBOOST;
	StatusIconChangeTable[SC_JEXPBOOST] = SI_CASH_PLUSONLYJOBEXP;
	StatusIconChangeTable[SC_LIFEINSURANCE] = SI_LIFEINSURANCE;
	StatusIconChangeTable[SC_BOSSMAPINFO] = SI_BOSSMAPINFO;
	StatusIconChangeTable[SC_DEF_RATE] = SI_DEF_RATE;
	StatusIconChangeTable[SC_MDEF_RATE] = SI_MDEF_RATE;
	StatusIconChangeTable[SC_INCCRI] = SI_INCCRI;
	StatusIconChangeTable[SC_INCFLEE2] = SI_PLUSAVOIDVALUE;
	StatusIconChangeTable[SC_INCHEALRATE] = SI_INCHEALRATE;
	StatusIconChangeTable[SC_S_LIFEPOTION] = SI_S_LIFEPOTION;
	StatusIconChangeTable[SC_L_LIFEPOTION] = SI_L_LIFEPOTION;
	StatusIconChangeTable[SC_SPCOST_RATE] = SI_ATKER_BLOOD;
	StatusIconChangeTable[SC_COMMONSC_RESIST] = SI_TARGET_BLOOD;
	// Mercenary Bonus Effects
	StatusIconChangeTable[SC_MERC_FLEEUP] = SI_MERC_FLEEUP;
	StatusIconChangeTable[SC_MERC_ATKUP] = SI_MERC_ATKUP;
	StatusIconChangeTable[SC_MERC_HPUP] = SI_MERC_HPUP;
	StatusIconChangeTable[SC_MERC_SPUP] = SI_MERC_SPUP;
	StatusIconChangeTable[SC_MERC_HITUP] = SI_MERC_HITUP;

	//Other SC which are not necessarily associated to skills.
	StatusChangeFlagTable[SC_ASPDPOTION0] = SCB_ASPD;
	StatusChangeFlagTable[SC_ASPDPOTION1] = SCB_ASPD;
	StatusChangeFlagTable[SC_ASPDPOTION2] = SCB_ASPD;
	StatusChangeFlagTable[SC_ASPDPOTION3] = SCB_ASPD;
	StatusChangeFlagTable[SC_SPEEDUP0] = SCB_SPEED;
	StatusChangeFlagTable[SC_SPEEDUP1] = SCB_SPEED;
	StatusChangeFlagTable[SC_ATKPOTION] = SCB_BATK;
	StatusChangeFlagTable[SC_MATKPOTION] = SCB_MATK;
	StatusChangeFlagTable[SC_INCALLSTATUS] |= SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK;
	StatusChangeFlagTable[SC_INCSTR] |= SCB_STR;
	StatusChangeFlagTable[SC_INCAGI] |= SCB_AGI;
	StatusChangeFlagTable[SC_INCVIT] |= SCB_VIT;
	StatusChangeFlagTable[SC_INCINT] |= SCB_INT;
	StatusChangeFlagTable[SC_INCDEX] |= SCB_DEX;
	StatusChangeFlagTable[SC_INCLUK] |= SCB_LUK;
	StatusChangeFlagTable[SC_INCHIT] |= SCB_HIT;
	StatusChangeFlagTable[SC_INCHITRATE] |= SCB_HIT;
	StatusChangeFlagTable[SC_INCFLEE] |= SCB_FLEE;
	StatusChangeFlagTable[SC_INCFLEERATE] |= SCB_FLEE;
	StatusChangeFlagTable[SC_INCCRI] |= SCB_CRI;
	StatusChangeFlagTable[SC_INCFLEE2] |= SCB_FLEE2;
	StatusChangeFlagTable[SC_INCMHPRATE] |= SCB_MAXHP;
	StatusChangeFlagTable[SC_INCMSPRATE] |= SCB_MAXSP;
	StatusChangeFlagTable[SC_INCATKRATE] |= SCB_BATK|SCB_WATK;
	StatusChangeFlagTable[SC_INCMATKRATE] |= SCB_MATK;
	StatusChangeFlagTable[SC_INCDEFRATE] |= SCB_DEF;
	StatusChangeFlagTable[SC_STRFOOD] |= SCB_STR;
	StatusChangeFlagTable[SC_AGIFOOD] |= SCB_AGI;
	StatusChangeFlagTable[SC_VITFOOD] |= SCB_VIT;
	StatusChangeFlagTable[SC_INTFOOD] |= SCB_INT;
	StatusChangeFlagTable[SC_DEXFOOD] |= SCB_DEX;
	StatusChangeFlagTable[SC_LUKFOOD] |= SCB_LUK;
	StatusChangeFlagTable[SC_HITFOOD] |= SCB_HIT;
	StatusChangeFlagTable[SC_FLEEFOOD] |= SCB_FLEE;
	StatusChangeFlagTable[SC_BATKFOOD] |= SCB_BATK;
	StatusChangeFlagTable[SC_WATKFOOD] |= SCB_WATK;
	StatusChangeFlagTable[SC_MATKFOOD] |= SCB_MATK;
	StatusChangeFlagTable[SC_ARMOR_ELEMENT] |= SCB_ALL;
	StatusChangeFlagTable[SC_ARMOR_RESIST] |= SCB_ALL;
	StatusChangeFlagTable[SC_SPCOST_RATE] |= SCB_ALL;
	StatusChangeFlagTable[SC_WALKSPEED] |= SCB_SPEED;
	StatusChangeFlagTable[SC_ITEMSCRIPT] |= SCB_ALL;
	// Cash Items
	StatusChangeFlagTable[SC_FOOD_STR_CASH] = SCB_STR;
	StatusChangeFlagTable[SC_FOOD_AGI_CASH] = SCB_AGI;
	StatusChangeFlagTable[SC_FOOD_VIT_CASH] = SCB_VIT;
	StatusChangeFlagTable[SC_FOOD_DEX_CASH] = SCB_DEX;
	StatusChangeFlagTable[SC_FOOD_INT_CASH] = SCB_INT;
	StatusChangeFlagTable[SC_FOOD_LUK_CASH] = SCB_LUK;
	// Mercenary Bonus Effects
	StatusChangeFlagTable[SC_MERC_FLEEUP] |= SCB_FLEE;
	StatusChangeFlagTable[SC_MERC_ATKUP] |= SCB_WATK;
	StatusChangeFlagTable[SC_MERC_HPUP] |= SCB_MAXHP;
	StatusChangeFlagTable[SC_MERC_SPUP] |= SCB_MAXSP;
	StatusChangeFlagTable[SC_MERC_HITUP] |= SCB_HIT;

	if( !battle_config.display_hallucination ) //Disable Hallucination.
		StatusIconChangeTable[SC_HALLUCINATION] = SI_BLANK;
}

static void initDummyData(void)
{
	memset(&dummy_status, 0, sizeof(dummy_status));
	dummy_status.hp = 
	dummy_status.max_hp = 
	dummy_status.max_sp = 
	dummy_status.str =
	dummy_status.agi =
	dummy_status.vit =
	dummy_status.int_ =
	dummy_status.dex =
	dummy_status.luk =
	dummy_status.hit = 1;
	dummy_status.speed = 2000;
	dummy_status.adelay = 4000;
	dummy_status.amotion = 2000;
	dummy_status.dmotion = 2000;
	dummy_status.ele_lv = 1; //Min elemental level.
	dummy_status.mode = MD_CANMOVE;
}

/*==========================================
 * 精錬ボーナス
 *------------------------------------------*/
int status_getrefinebonus(int lv,int type)
{
	if (lv >= 0 && lv < 5 && type >= 0 && type < 3)
		return refinebonus[lv][type];
	return 0;
}

//Sets HP to given value. Flag is the flag passed to status_heal in case
//final value is higher than current (use 2 to make a healing effect display 
//on players) It will always succeed (overrides Berserk block), but it can't kill.
int status_set_hp(struct block_list *bl, unsigned int hp, int flag)
{
	struct status_data *status;
	if (hp < 1) return 0;
	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (hp > status->max_hp) hp = status->max_hp;
	if (hp == status->hp) return 0;
	if (hp > status->hp)
		return status_heal(bl, hp - status->hp, 0, 1|flag);
	return status_zap(bl, status->hp - hp, 0);
}

//Sets SP to given value. Flag is the flag passed to status_heal in case
//final value is higher than current (use 2 to make a healing effect display 
//on players)
int status_set_sp(struct block_list *bl, unsigned int sp, int flag)
{
	struct status_data *status;

	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (sp > status->max_sp) sp = status->max_sp;
	if (sp == status->sp) return 0;
	if (sp > status->sp)
		return status_heal(bl, 0, sp - status->sp, 1|flag);
	return status_zap(bl, 0, status->sp - sp);
}

int status_charge(struct block_list* bl, int hp, int sp)
{
	if(!(bl->type&BL_CONSUME))
		return hp+sp; //Assume all was charged so there are no 'not enough' fails.
	return status_damage(NULL, bl, hp, sp, 0, 3);
}

//Inflicts damage on the target with the according walkdelay.
//If flag&1, damage is passive and does not triggers cancelling status changes.
//If flag&2, fail if target does not has enough to substract.
//If flag&4, if killed, mob must not give exp/loot.
//If flag&8, sp loss on dead target.
int status_damage(struct block_list *src,struct block_list *target,int hp, int sp, int walkdelay, int flag)
{
	struct status_data *status;
	struct status_change *sc;

	if(sp && !(target->type&BL_CONSUME))
		sp = 0; //Not a valid SP target.

	if (hp < 0) { //Assume absorbed damage.
		status_heal(target, cap_value(-hp, INT_MIN, INT_MAX), 0, 1);
		hp = 0;
	}

	if (sp < 0) {
		status_heal(target, 0, cap_value(-sp, INT_MIN, INT_MAX), 1);
		sp = 0;
	}

	if (!hp && !sp)
		return 0;

	if (target->type == BL_SKILL)
		return skill_unit_ondamaged((struct skill_unit *)target, src, hp, gettick());

	status = status_get_status_data(target);
	if( status == &dummy_status )
		return 0;

	if( !status->hp )
		flag |= 8;

// Let through. battle.c/skill.c have the whole logic of when it's possible or
// not to hurt someone (and this check breaks pet catching) [Skotlex]
//	if (!target->prev && !(flag&2))
//		return 0; //Cannot damage a bl not on a map, except when "charging" hp/sp

	sc = status_get_sc(target);
	if( battle_config.invincible_nodamage && src && sc && sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
		hp = 1;

	if( hp && !(flag&(1|8)) ) {
		if( sc ) {
			struct status_change_entry *sce;
			if (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
				status_change_end(target,SC_STONE,-1);
			status_change_end(target,SC_FREEZE,-1);
			status_change_end(target,SC_SLEEP,-1);
			status_change_end(target,SC_WINKCHARM,-1);
			status_change_end(target,SC_CONFUSION,-1);
			status_change_end(target,SC_TRICKDEAD,-1);
			status_change_end(target,SC_HIDING,-1);
			status_change_end(target,SC_CLOAKING,-1);
			status_change_end(target,SC_CHASEWALK,-1);
			if ((sce=sc->data[SC_ENDURE]) && !sce->val4) {
				//Endure count is only reduced by non-players on non-gvg maps.
				//val4 signals infinite endure. [Skotlex]
				if (src && src->type != BL_PC && !map_flag_gvg(target->m) && !map[target->m].flag.battleground && --(sce->val2) < 0)
					status_change_end(target, SC_ENDURE, -1);
			}
			if ((sce=sc->data[SC_GRAVITATION]) && sce->val3 == BCT_SELF)
			{
				struct skill_unit_group* sg = skill_id2group(sce->val4);
				if (sg) {
					skill_delunitgroup(sg);
					sce->val4 = 0;
					status_change_end(target, SC_GRAVITATION, -1);
				}
			}
			if(sc->data[SC_DANCING] && (unsigned int)hp > status->max_hp>>2)
				status_change_end(target, SC_DANCING, -1);
		}
		unit_skillcastcancel(target, 2);
	}

	if ((unsigned int)hp >= status->hp) {
		if (flag&2) return 0;
		hp = status->hp;
	}

	if ((unsigned int)sp > status->sp) {
		if (flag&2) return 0;
		sp = status->sp;
	}

	status->hp-= hp;
	status->sp-= sp;

	if (sc && hp && status->hp) {
		if (sc->data[SC_AUTOBERSERK] &&
			(!sc->data[SC_PROVOKE] || !sc->data[SC_PROVOKE]->val2) &&
			status->hp < status->max_hp>>2)
			sc_start4(target,SC_PROVOKE,100,10,1,0,0,0);
		if (sc->data[SC_BERSERK] && status->hp <= 100)
			status_change_end(target, SC_BERSERK, -1);
	}

	switch (target->type)
	{
		case BL_PC:  pc_damage((TBL_PC*)target,src,hp,sp); break;
		case BL_MOB: mob_damage((TBL_MOB*)target, src, hp); break;
		case BL_HOM: merc_damage((TBL_HOM*)target,src,hp,sp); break;
		case BL_MER: mercenary_damage((TBL_MER*)target,src,hp,sp); break;
	}

	if( status->hp || flag&8 )
  	{	//Still lives or has been dead before this damage.
		if (walkdelay)
			unit_set_walkdelay(target, gettick(), walkdelay, 0);
		return hp+sp;
	}

	status->hp = 1; //To let the dead function cast skills and all that.
	//NOTE: These dead functions should return: [Skotlex]
	//0: Death cancelled, auto-revived.
	//Non-zero: Standard death. Clear status, cancel move/attack, etc
	//&2: Also remove object from map.
	//&4: Also delete object from memory.
	switch (target->type) {
		case BL_PC:  flag = pc_dead((TBL_PC*)target,src); break;
		case BL_MOB: flag = mob_dead((TBL_MOB*)target, src, flag&4?3:0); break;
		case BL_HOM: flag = merc_hom_dead((TBL_HOM*)target,src); break;
		case BL_MER: flag = mercenary_dead((TBL_MER*)target,src); break;
		default:	//Unhandled case, do nothing to object.
			flag = 0;
			break;
	}

	if(!flag) //Death cancelled.
		return hp+sp;

	//Normal death
	status->hp = 0;
	if (battle_config.clear_unit_ondeath &&
		battle_config.clear_unit_ondeath&target->type)
		skill_clear_unitgroup(target);

	if(target->type&BL_REGEN)
	{	//Reset regen ticks.
		struct regen_data *regen = status_get_regen_data(target);
		if (regen) {
			memset(&regen->tick, 0, sizeof(regen->tick));
			if (regen->sregen)
				memset(&regen->sregen->tick, 0, sizeof(regen->sregen->tick));
			if (regen->ssregen)
				memset(&regen->ssregen->tick, 0, sizeof(regen->ssregen->tick));
		}
	}
   
	if( !(flag&8) && sc && sc->data[SC_KAIZEL] )
	{ //flag&8 = disable Kaizel
		int time = skill_get_time2(SL_KAIZEL,sc->data[SC_KAIZEL]->val1);
		//Look for Osiris Card's bonus effect on the character and revive 100% or revive normally
		if ( target->type == BL_PC && BL_CAST(BL_PC,target)->special_state.restart_full_recover == 1 )
			status_revive(target, 100, 100);
		else
			status_revive(target, sc->data[SC_KAIZEL]->val2, 0);
		status_change_clear(target,0);
		clif_skill_nodamage(target,target,ALL_RESURRECTION,1,1);
		sc_start(target,status_skill2sc(PR_KYRIE),100,10,time);

		if( target->type == BL_MOB ) 
			((TBL_MOB*)target)->state.rebirth = 1;

		return hp+sp;
	}

	if( target->type == BL_MOB && sc && sc->data[SC_REBIRTH] && !((TBL_MOB*)target)->state.rebirth )
	{// Ensure the monster has not already rebirthed before doing so.
		status_revive(target, sc->data[SC_REBIRTH]->val2, 0);
		status_change_clear(target,0);
		((TBL_MOB*)target)->state.rebirth = 1;

		return hp+sp;
	}

	status_change_clear(target,0);

	if(flag&4) //Delete from memory. (also invokes map removal code)
		unit_free(target,1);
	else
	if(flag&2) //remove from map
		unit_remove_map(target,1);
	else
	{ //Some death states that would normally be handled by unit_remove_map
		unit_stop_attack(target);
		unit_stop_walking(target,1);
		unit_skillcastcancel(target,0);
		clif_clearunit_area(target,1);
		skill_unit_move(target,gettick(),4);
		skill_cleartimerskill(target);
	}

	return hp+sp;
}

//Heals a character. If flag&1, this is forced healing (otherwise stuff like Berserk can block it)
//If flag&2, when the player is healed, show the HP/SP heal effect.
int status_heal(struct block_list *bl,int hp,int sp, int flag)
{
	struct status_data *status;
	struct status_change *sc;

	status = status_get_status_data(bl);

	if (status == &dummy_status || !status->hp)
		return 0;

	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;

	if (hp < 0) {
		status_damage(NULL, bl, cap_value(-hp, INT_MIN, INT_MAX), 0, 0, 1);
		hp = 0;
	}

	if(hp) {
		if (!(flag&1) && sc && sc->data[SC_BERSERK])
			hp = 0;

		if((unsigned int)hp > status->max_hp - status->hp)
			hp = status->max_hp - status->hp;
	}

	if(sp < 0) {
		status_damage(NULL, bl, 0, cap_value(-sp, INT_MIN, INT_MAX), 0, 1);
		sp = 0;
	}

	if(sp) {
		if((unsigned int)sp > status->max_sp - status->sp)
			sp = status->max_sp - status->sp;
	}

	if(!sp && !hp) return 0;

	status->hp+= hp;
	status->sp+= sp;

	if(hp && sc &&
		sc->data[SC_AUTOBERSERK] &&
		sc->data[SC_PROVOKE] &&
		sc->data[SC_PROVOKE]->val2==1 &&
		status->hp>=status->max_hp>>2
	)	//End auto berserk.
		status_change_end(bl,SC_PROVOKE,-1);

	// send hp update to client
	switch(bl->type) {
	case BL_PC:  pc_heal((TBL_PC*)bl,hp,sp,flag&2?1:0); break;
	case BL_MOB: mob_heal((TBL_MOB*)bl,hp); break;
	case BL_HOM: merc_hom_heal((TBL_HOM*)bl,hp,sp); break;
	case BL_MER: mercenary_heal((TBL_MER*)bl,hp,sp); break;
	}

	return hp+sp;
}

//Does percentual non-flinching damage/heal. If mob is killed this way,
//no exp/drops will be awarded if there is no src (or src is target)
//If rates are > 0, percent is of current HP/SP
//If rates are < 0, percent is of max HP/SP
//If !flag, this is heal, otherwise it is damage.
//Furthermore, if flag==2, then the target must not die from the substraction.
int status_percent_change(struct block_list *src,struct block_list *target,signed char hp_rate, signed char sp_rate, int flag)
{
	struct status_data *status;
	unsigned int hp  =0, sp = 0;

	status = status_get_status_data(target);

	//Change the equation when the values are high enough to discard the
	//imprecision in exchange of overflow protection [Skotlex]
	//Also add 100% checks since those are the most used cases where we don't 
	//want aproximation errors.
	if (hp_rate > 99)
		hp = status->hp;
	else if (hp_rate > 0)
		hp = status->hp>10000?
			hp_rate*(status->hp/100):
			(hp_rate*status->hp)/100;
	else if (hp_rate < -99)
		hp = status->max_hp;
	else if (hp_rate < 0)
		hp = status->max_hp>10000?
			(-hp_rate)*(status->max_hp/100):
			(-hp_rate*status->max_hp)/100;
	if (hp_rate && !hp)
		hp = 1;

	if (flag == 2 && hp >= status->hp)
		hp = status->hp-1; //Must not kill target.

	//Should be safe to not do overflow protection here, noone should have
	//millions upon millions of SP
	if (sp_rate > 99)
		sp = status->sp;
	else if (sp_rate > 0)
		sp = (sp_rate*status->sp)/100;
	else if (sp_rate < -99)
		sp = status->max_sp;
	else if (sp_rate < 0)
		sp = (-sp_rate)*status->max_sp/100;
	if (sp_rate && !sp)
		sp = 1;

	//Ugly check in case damage dealt is too much for the received args of
	//status_heal / status_damage. [Skotlex]
	if (hp > INT_MAX) {
	  	hp -= INT_MAX;
		if (flag)
			status_damage(src, target, INT_MAX, 0, 0, (!src||src==target?5:1));
		else
		  	status_heal(target, INT_MAX, 0, 0);
	}
  	if (sp > INT_MAX) {
		sp -= INT_MAX;
		if (flag)
			status_damage(src, target, 0, INT_MAX, 0, (!src||src==target?5:1));
		else
		  	status_heal(target, 0, INT_MAX, 0);
	}	
	if (flag)
		return status_damage(src, target, hp, sp, 0, (!src||src==target?5:1));
	return status_heal(target, hp, sp, 0);
}

int status_revive(struct block_list *bl, unsigned char per_hp, unsigned char per_sp)
{
	struct status_data *status;
	unsigned int hp, sp;
	if (!status_isdead(bl)) return 0;

	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0; //Invalid target.

	hp = status->max_hp * per_hp/100;
	sp = status->max_sp * per_sp/100;

	if(hp > status->max_hp - status->hp)
		hp = status->max_hp - status->hp;
	else if (per_hp && !hp)
		hp = 1;
		
	if(sp > status->max_sp - status->sp)
		sp = status->max_sp - status->sp;
	else if (per_sp && !sp)
		sp = 1;

	status->hp += hp;
	status->sp += sp;

	if (bl->prev) //Animation only if character is already on a map.
		clif_resurrection(bl, 1);
	switch (bl->type) {
		case BL_PC:  pc_revive((TBL_PC*)bl, hp, sp); break;
		case BL_MOB: mob_revive((TBL_MOB*)bl, hp); break;
		case BL_HOM: merc_hom_revive((TBL_HOM*)bl, hp, sp); break;
	}
	return 1;
}

/*==========================================
 * Checks whether the src can use the skill on the target,
 * taking into account status/option of both source/target. [Skotlex]
 * flag:
 * 	0 - Trying to use skill on target.
 * 	1 - Cast bar is done.
 * 	2 - Skill already pulled off, check is due to ground-based skills or splash-damage ones.
 * src MAY be null to indicate we shouldn't check it, this is a ground-based skill attack.
 * target MAY Be null, in which case the checks are only to see 
 * whether the source can cast or not the skill on the ground.
 *------------------------------------------*/
int status_check_skilluse(struct block_list *src, struct block_list *target, int skill_num, int flag)
{
	struct status_data *status;
	struct status_change *sc=NULL, *tsc;
	int hide_flag;

	status = src?status_get_status_data(src):&dummy_status;

	if (src && status_isdead(src))
		return 0;

	if (!skill_num) { //Normal attack checks.
		if (!(status->mode&MD_CANATTACK))
			return 0; //This mode is only needed for melee attacking.
		//Dead state is not checked for skills as some skills can be used 
		//on dead characters, said checks are left to skill.c [Skotlex]
		if (target && status_isdead(target))
			return 0;
	}

	if (skill_num == PA_PRESSURE && flag && target) {
		//Gloria Avoids pretty much everything....
		tsc = status_get_sc(target);
		if(tsc && tsc->option&OPTION_HIDE)
			return 0;
		return 1;
	}

	//Should fail when used on top of Land Protector [Skotlex]
	if (src && skill_num == AL_TELEPORT && map_getcell(src->m, src->x, src->y, CELL_CHKLANDPROTECTOR)
		&& !(status->mode&MD_BOSS)
		&& (src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_num))
		return 0;

	if (src) sc = status_get_sc(src);

	if(sc && sc->count)
	{
		if(sc->opt1 >0)
		{	//Stuned/Frozen/etc
			if (flag != 1) //Can't cast, casted stuff can't damage. 
				return 0;
			if (!(skill_get_inf(skill_num)&INF_GROUND_SKILL))
				return 0; //Targetted spells can't come off.
		}

		if (
			(sc->data[SC_TRICKDEAD] && skill_num != NV_TRICKDEAD)
			|| (sc->data[SC_AUTOCOUNTER] && !flag)
			|| (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF && skill_num != PA_GOSPEL)
			|| (sc->data[SC_GRAVITATION] && sc->data[SC_GRAVITATION]->val3 == BCT_SELF && flag != 2)
		)
			return 0;

		if (sc->data[SC_WINKCHARM] && target && !flag)
		{	//Prevents skill usage
			clif_emotion(src, 3);
			return 0;
		}

		if (sc->data[SC_BLADESTOP]) {
			switch (sc->data[SC_BLADESTOP]->val1)
			{
				case 5: if (skill_num == MO_EXTREMITYFIST) break;
				case 4: if (skill_num == MO_CHAINCOMBO) break;
				case 3: if (skill_num == MO_INVESTIGATE) break;
				case 2: if (skill_num == MO_FINGEROFFENSIVE) break;
				default: return 0;
			}
		}

		if (sc->data[SC_DANCING] && flag!=2)
		{
			if(sc->data[SC_LONGING])
			{	//Allow everything except dancing/re-dancing. [Skotlex]
				if (skill_num == BD_ENCORE ||
					skill_get_inf2(skill_num)&(INF2_SONG_DANCE|INF2_ENSEMBLE_SKILL)
				)
					return 0;
			} else
			switch (skill_num) {
			case BD_ADAPTATION:
			case CG_LONGINGFREEDOM:
			case BA_MUSICALSTRIKE:
			case DC_THROWARROW:
				break;
			default:
				return 0;
			}
			if ((sc->data[SC_DANCING]->val1&0xFFFF) == CG_HERMODE && skill_num == BD_ADAPTATION)
				return 0;	//Can't amp out of Wand of Hermode :/ [Skotlex]
		}

		if (skill_num && //Do not block item-casted skills.
			(src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_num)
		) {	//Skills blocked through status changes...
			if (!flag && ( //Blocked only from using the skill (stuff like autospell may still go through
				sc->data[SC_SILENCE] ||
				(sc->data[SC_MARIONETTE] && skill_num != CG_MARIONETTE) || //Only skill you can use is marionette again to cancel it
				(sc->data[SC_MARIONETTE2] && skill_num == CG_MARIONETTE) || //Cannot use marionette if you are being buffed by another
				sc->data[SC_STEELBODY] ||
				sc->data[SC_BERSERK]
			))
				return 0;

			//Skill blocking.
			if (
				(sc->data[SC_VOLCANO] && skill_num == WZ_ICEWALL) ||
				(sc->data[SC_ROKISWEIL] && skill_num != BD_ADAPTATION) ||
				(sc->data[SC_HERMODE] && skill_get_inf(skill_num) & INF_SUPPORT_SKILL) ||
				(sc->data[SC_NOCHAT] && sc->data[SC_NOCHAT]->val1&MANNER_NOSKILL)
			)
				return 0;

		}
	}

	if (sc && sc->option)
	{
		if (sc->option&OPTION_HIDE)
		switch (skill_num) { //Usable skills while hiding.
			case TF_HIDING:
			case AS_GRIMTOOTH:
			case RG_BACKSTAP:
			case RG_RAID:
			case NJ_SHADOWJUMP:
			case NJ_KIRIKAGE:
				break;
			default:
				//Non players can use all skills while hidden.
				if (!skill_num || src->type == BL_PC)
					return 0;
		}
		if (sc->option&OPTION_CHASEWALK && skill_num != ST_CHASEWALK)
			return 0;
	}
	if (target == NULL || target == src) //No further checking needed.
		return 1;

	tsc = status_get_sc(target);
	
	if(tsc && tsc->count)
	{	
		if(!skill_num && !(status->mode&MD_BOSS) && tsc->data[SC_TRICKDEAD])
			return 0;
		if((skill_num == WZ_STORMGUST || skill_num == WZ_FROSTNOVA || skill_num == NJ_HYOUSYOURAKU)
			&& tsc->data[SC_FREEZE])
			return 0;
		if(skill_num == PR_LEXAETERNA && (tsc->data[SC_FREEZE] || (tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE)))
			return 0;
	}

	//If targetting, cloak+hide protect you, otherwise only hiding does.
	hide_flag = flag?OPTION_HIDE:(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK);
		
 	//You cannot hide from ground skills.
	if( skill_get_ele(skill_num,1) == ELE_EARTH ) //TODO: Need Skill Lv here :/
		hide_flag &= ~OPTION_HIDE;

	switch( target->type )
	{
	case BL_PC:
		{
			struct map_session_data *sd = (TBL_PC*) target;
			if (pc_isinvisible(sd))
				return 0;
			if (tsc->option&hide_flag && !(status->mode&MD_BOSS) &&
				(sd->special_state.perfect_hiding || !(status->mode&MD_DETECTOR)))
				return 0;
		}
		break;
	case BL_ITEM:	//Allow targetting of items to pick'em up (or in the case of mobs, to loot them).
		//TODO: Would be nice if this could be used to judge whether the player can or not pick up the item it targets. [Skotlex]
		if (status->mode&MD_LOOTER)
			return 1;
		return 0;
	case BL_HOM: 
	case BL_MER:
		if( target->type == BL_HOM && skill_num && battle_config.hom_setting&0x1 && skill_get_inf(skill_num)&INF_SUPPORT_SKILL && battle_get_master(target) != src )
			return 0; // Can't use support skills on Homunculus (only Master/Self)
		if( target->type == BL_MER && (skill_num == PR_ASPERSIO || (skill_num >= SA_FLAMELAUNCHER && skill_num <= SA_SEISMICWEAPON)) && battle_get_master(target) != src )
			return 0; // Can't use Weapon endow skills on Mercenary (only Master)
		if( target->type == BL_MER && skill_num == AM_POTIONPITCHER )
			return 0; // Can't use Potion Pitcher on Mercenaries
	default:
		//Check for chase-walk/hiding/cloaking opponents.
		if (tsc && tsc->option&hide_flag && !(status->mode&(MD_BOSS|MD_DETECTOR)))
			return 0;
	}
	return 1;
}

//Checks whether the source can see and chase target.
int status_check_visibility(struct block_list *src, struct block_list *target)
{
	int view_range;
	struct status_data* status = status_get_status_data(src);
	struct status_change* tsc = status_get_sc(target);
	switch (src->type) {
	case BL_MOB:
		view_range = ((TBL_MOB*)src)->min_chase;
		break;
	case BL_PET:
		view_range = ((TBL_PET*)src)->db->range2;
		break;
	default:
		view_range = AREA_SIZE;
	}

	if (src->m != target->m || !check_distance_bl(src, target, view_range))
		return 0;

	switch (target->type)
	{	//Check for chase-walk/hiding/cloaking opponents.
	case BL_PC:
		if(tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) &&
			!(status->mode&MD_BOSS) &&
			(
				((TBL_PC*)target)->special_state.perfect_hiding ||
				!(status->mode&MD_DETECTOR)
			))
			return 0;
		break;
	default:
		if (tsc && tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) &&
			!(status->mode&(MD_BOSS|MD_DETECTOR)))
				return 0;
	}

	return 1;
}

// Basic ASPD value
int status_base_amotion_pc(struct map_session_data* sd, struct status_data* status)
{
	int amotion;
	
	// base weapon delay
	amotion = (sd->status.weapon < MAX_WEAPON_TYPE)
	 ? (aspd_base[pc_class2idx(sd->status.class_)][sd->status.weapon]) // single weapon
	 : (aspd_base[pc_class2idx(sd->status.class_)][sd->weapontype1] + aspd_base[pc_class2idx(sd->status.class_)][sd->weapontype2])*7/10; // dual-wield
	
	// percentual delay reduction from stats
	amotion-= amotion * (4*status->agi + status->dex)/1000;
	
	// raw delay adjustment from bAspd bonus
	amotion+= sd->aspd_add;
	
 	return amotion;
}

static unsigned short status_base_atk(const struct block_list *bl, const struct status_data *status)
{
	int flag = 0, str, dex, dstr;

	if(!(bl->type&battle_config.enable_baseatk))
		return 0;

	if (bl->type == BL_PC)
	switch(((TBL_PC*)bl)->status.weapon){
		case W_BOW:
		case W_MUSICAL: 
		case W_WHIP:
		case W_REVOLVER:
		case W_RIFLE:
		case W_GATLING:
		case W_SHOTGUN:
		case W_GRENADE:
			flag = 1;
	}
	if (flag) {
		str = status->dex;
		dex = status->str;
	} else {
		str = status->str;
		dex = status->dex;
	}
	//Normally only players have base-atk, but homunc have a different batk
	// equation, hinting that perhaps non-players should use this for batk.
	// [Skotlex]
	dstr = str/10;
	str += dstr*dstr;
	if (bl->type == BL_PC)
		str+= dex/5 + status->luk/5;
	return cap_value(str, 0, USHRT_MAX);
}

#define status_base_matk_max(status) (status->int_+(status->int_/5)*(status->int_/5))
#define status_base_matk_min(status) (status->int_+(status->int_/7)*(status->int_/7))

//Fills in the misc data that can be calculated from the other status info (except for level)
void status_calc_misc(struct block_list *bl, struct status_data *status, int level)
{
	//Non players get the value set, players need to stack with previous bonuses.
	if( bl->type != BL_PC )
		status->batk = 
		status->hit = status->flee =
		status->def2 = status->mdef2 =
		status->cri = status->flee2 = 0;

	status->matk_min = status_base_matk_min(status);
	status->matk_max = status_base_matk_max(status);

	status->hit += level + status->dex;
	status->flee += level + status->agi;
	status->def2 += status->vit;
	status->mdef2 += status->int_ + (status->vit>>1);

	if( bl->type&battle_config.enable_critical )
		status->cri += status->luk*3 + 10;
	else
		status->cri = 0;

	if (bl->type&battle_config.enable_perfect_flee)
		status->flee2 += status->luk + 10;
	else
		status->flee2 = 0;

	if (status->batk) {
		int temp = status->batk + status_base_atk(bl, status);
		status->batk = cap_value(temp, 0, USHRT_MAX);
	} else
		status->batk = status_base_atk(bl, status);
	if (status->cri)
	switch (bl->type) {
	case BL_MOB:
		if(battle_config.mob_critical_rate != 100)
			status->cri = status->cri*battle_config.mob_critical_rate/100;
		if(!status->cri && battle_config.mob_critical_rate)
		  	status->cri = 10;
		break;
	case BL_PC:
		//Players don't have a critical adjustment setting as of yet.
		break;
	default:
		if(battle_config.critical_rate != 100)
			status->cri = status->cri*battle_config.critical_rate/100;
		if (!status->cri && battle_config.critical_rate)
			status->cri = 10;
	}
	if(bl->type&BL_REGEN)
		status_calc_regen(bl, status, status_get_regen_data(bl));
}

//Skotlex: Calculates the initial status for the given mob
//first will only be false when the mob leveled up or got a GuardUp level.
int status_calc_mob_(struct mob_data* md, bool first)
{
	struct status_data *status;
	struct block_list *mbl = NULL;
	int flag=0;

	if(first)
	{	//Set basic level on respawn.
		md->level = md->db->lv;
	}

	//Check if we need custom base-status
	if (battle_config.mobs_level_up && md->level > md->db->lv)
		flag|=1;
	
	if (md->special_state.size)
		flag|=2;

	if (md->guardian_data && md->guardian_data->guardup_lv)
		flag|=4;
	if (md->class_ == MOBID_EMPERIUM)
		flag|=4;

	if (battle_config.slaves_inherit_speed && md->master_id)
		flag|=8;

	if (md->master_id && md->special_state.ai>1)
		flag|=16;

	if (!flag)
	{ //No special status required.
		if (md->base_status) {
			aFree(md->base_status);
			md->base_status = NULL;
		}
		if(first)
			memcpy(&md->status, &md->db->status, sizeof(struct status_data));
		return 0;
	}
	if (!md->base_status)
		md->base_status = (struct status_data*)aCalloc(1, sizeof(struct status_data));

	status = md->base_status;
	memcpy(status, &md->db->status, sizeof(struct status_data));

	if (flag&(8|16))
		mbl = map_id2bl(md->master_id);

	if (flag&8 && mbl) {
		struct status_data *mstatus = status_get_base_status(mbl);
		if (mstatus &&
			battle_config.slaves_inherit_speed&(mstatus->mode&MD_CANMOVE?1:2))
			status->speed = mstatus->speed;
	}

	if (flag&16 && mbl)
	{	//Max HP setting from Summon Flora/marine Sphere
		struct unit_data *ud = unit_bl2ud(mbl);
		//Remove special AI when this is used by regular mobs.
		if (mbl->type == BL_MOB && !((TBL_MOB*)mbl)->special_state.ai)
			md->special_state.ai = 0;
		if (ud)
		{	// different levels of HP according to skill level
			if (ud->skillid == AM_SPHEREMINE) {
				status->max_hp = 2000 + 400*ud->skilllv;
			} else { //AM_CANNIBALIZE
				status->max_hp = 1500 + 200*ud->skilllv + 10*status_get_lv(mbl);
				status->mode|= MD_CANATTACK|MD_AGGRESSIVE;
			}
			status->hp = status->max_hp;
		}
	}

	if (flag&1)
	{	// increase from mobs leveling up [Valaris]
		int diff = md->level - md->db->lv;
		status->str+= diff;
		status->agi+= diff;
		status->vit+= diff;
		status->int_+= diff;
		status->dex+= diff;
		status->luk+= diff;
		status->max_hp += diff*status->vit;
		status->max_sp += diff*status->int_;
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		status->speed -= diff;
	}


	if (flag&2)
	{	// change for sized monsters [Valaris]
		if (md->special_state.size==1) {
			status->max_hp>>=1;
			status->max_sp>>=1;
			if (!status->max_hp) status->max_hp = 1;
			if (!status->max_sp) status->max_sp = 1;
			status->hp=status->max_hp;
			status->sp=status->max_sp;
			status->str>>=1;
			status->agi>>=1;
			status->vit>>=1;
			status->int_>>=1;
			status->dex>>=1;
			status->luk>>=1;
			if (!status->str) status->str = 1;
			if (!status->agi) status->agi = 1;
			if (!status->vit) status->vit = 1;
			if (!status->int_) status->int_ = 1;
			if (!status->dex) status->dex = 1;
			if (!status->luk) status->luk = 1;
		} else if (md->special_state.size==2) {
			status->max_hp<<=1;
			status->max_sp<<=1;
			status->hp=status->max_hp;
			status->sp=status->max_sp;
			status->str<<=1;
			status->agi<<=1;
			status->vit<<=1;
			status->int_<<=1;
			status->dex<<=1;
			status->luk<<=1;
		}
	}

	status_calc_misc(&md->bl, status, md->level);

	if(flag&4)
	{	// Strengthen Guardians - custom value +10% / lv
		struct guild_castle *gc;
		gc=guild_mapname2gc(map[md->bl.m].name);
		if (!gc)
			ShowError("status_calc_mob: No castle set at map %s\n", map[md->bl.m].name);
		else {
			if(gc->castle_id > 23) {
				if(md->class_ == MOBID_EMPERIUM) {
					status->max_hp += 1000 * gc->defense;
					status->max_sp += 200 * gc->defense;
					status->hp = status->max_hp;
					status->sp = status->max_sp;
					status->def += (gc->defense+2)/3;
					status->mdef += (gc->defense+2)/3;
				}
			}else{
				status->max_hp += 1000 * gc->defense;
				status->max_sp += 200 * gc->defense;
				status->hp = status->max_hp;
				status->sp = status->max_sp;
				status->def += (gc->defense+2)/3;
				status->mdef += (gc->defense+2)/3;
			}
		}
		if(md->class_ != MOBID_EMPERIUM) {
			status->batk += status->batk * 10*md->guardian_data->guardup_lv/100;
			status->rhw.atk += status->rhw.atk * 10*md->guardian_data->guardup_lv/100;
			status->rhw.atk2 += status->rhw.atk2 * 10*md->guardian_data->guardup_lv/100;
			status->aspd_rate -= 100*md->guardian_data->guardup_lv;
		}
	}

	if( first ) //Initial battle status
		memcpy(&md->status, status, sizeof(struct status_data));

	return 1;
}

//Skotlex: Calculates the stats of the given pet.
int status_calc_pet_(struct pet_data *pd, bool first)
{
	nullpo_ret(pd);

	if (first) {
		memcpy(&pd->status, &pd->db->status, sizeof(struct status_data));
		pd->status.mode|= MD_CANMOVE; //so they can chase their master!
		pd->status.speed = pd->petDB->speed;
	}

	if (battle_config.pet_lv_rate && pd->msd)
	{
		struct map_session_data *sd = pd->msd;
		int lv;

		lv =sd->status.base_level*battle_config.pet_lv_rate/100;
		if (lv < 0)
			lv = 1;
		if (lv != pd->pet.level || first)
		{
			struct status_data *bstat = &pd->db->status, *status = &pd->status;
			pd->pet.level = lv;
			if (!first) //Lv Up animation
				clif_misceffect(&pd->bl, 0);
			status->rhw.atk = (bstat->rhw.atk*lv)/pd->db->lv;
			status->rhw.atk2 = (bstat->rhw.atk2*lv)/pd->db->lv;
			status->str = (bstat->str*lv)/pd->db->lv;
			status->agi = (bstat->agi*lv)/pd->db->lv;
			status->vit = (bstat->vit*lv)/pd->db->lv;
			status->int_ = (bstat->int_*lv)/pd->db->lv;
			status->dex = (bstat->dex*lv)/pd->db->lv;
			status->luk = (bstat->luk*lv)/pd->db->lv;

			status->rhw.atk = cap_value(status->rhw.atk, 1, battle_config.pet_max_atk1);
			status->rhw.atk2 = cap_value(status->rhw.atk2, 2, battle_config.pet_max_atk2);
			status->str = cap_value(status->str,1,battle_config.pet_max_stats);
			status->agi = cap_value(status->agi,1,battle_config.pet_max_stats);
			status->vit = cap_value(status->vit,1,battle_config.pet_max_stats);
			status->int_= cap_value(status->int_,1,battle_config.pet_max_stats);
			status->dex = cap_value(status->dex,1,battle_config.pet_max_stats);
			status->luk = cap_value(status->luk,1,battle_config.pet_max_stats);

			status_calc_misc(&pd->bl, &pd->status, lv);

			if (!first)	//Not done the first time because the pet is not visible yet
				clif_send_petstatus(sd);
		}
	} else if (first) {
		status_calc_misc(&pd->bl, &pd->status, pd->db->lv);
		if (!battle_config.pet_lv_rate && pd->pet.level != pd->db->lv)
			pd->pet.level = pd->db->lv;
	}
	
	//Support rate modifier (1000 = 100%)
	pd->rate_fix = 1000*(pd->pet.intimate - battle_config.pet_support_min_friendly)/(1000- battle_config.pet_support_min_friendly) +500;
	if(battle_config.pet_support_rate != 100)
		pd->rate_fix = pd->rate_fix*battle_config.pet_support_rate/100;

	return 1;
}	

/// Helper function for status_base_pc_maxhp(), used to pre-calculate the hp_sigma_val[] array
static void status_calc_sigma(void)
{
	int i,j;

	for(i = 0; i < CLASS_COUNT; i++)
	{
		unsigned int k = 0;
		hp_sigma_val[i][0] = hp_sigma_val[i][1] = 0;
		for(j = 2; j <= MAX_LEVEL; j++)
		{
			k += (hp_coefficient[i]*j + 50) / 100;
			hp_sigma_val[i][j] = k;
			if (k >= INT_MAX)
				break; //Overflow protection. [Skotlex]
		}
		for(; j <= MAX_LEVEL; j++)
			hp_sigma_val[i][j] = INT_MAX;
	}
}

/// Calculates base MaxHP value according to class and base level
/// The recursive equation used to calculate level bonus is (using integer operations)
///    f(0) = 35 | f(x+1) = f(x) + A + (x + B)*C/D
/// which reduces to something close to
///    f(x) = 35 + x*(A + B*C/D) + sum(i=2..x){ i*C/D }
static unsigned int status_base_pc_maxhp(struct map_session_data* sd, struct status_data* status)
{
	unsigned int val = pc_class2idx(sd->status.class_);
	val = 35 + sd->status.base_level*hp_coefficient2[val]/100 + hp_sigma_val[val][sd->status.base_level];

	if((sd->class_&MAPID_UPPERMASK) == MAPID_NINJA || (sd->class_&MAPID_UPPERMASK) == MAPID_GUNSLINGER)
		val += 100; //Since their HP can't be approximated well enough without this.
	if((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && pc_famerank(sd->status.char_id, MAPID_TAEKWON))
		val *= 3; //Triple max HP for top ranking Taekwons over level 90.
	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 99)
		val += 2000; //Supernovice lvl99 hp bonus.

	val += val * status->vit/100; // +1% per each point of VIT

	if (sd->class_&JOBL_UPPER)
		val += val * 25/100; //Trans classes get a 25% hp bonus
	else if (sd->class_&JOBL_BABY)
		val -= val * 30/100; //Baby classes get a 30% hp penalty
	return val;
}

static unsigned int status_base_pc_maxsp(struct map_session_data* sd, struct status_data *status)
{
	unsigned int val;

	val = 10 + sd->status.base_level*sp_coefficient[pc_class2idx(sd->status.class_)]/100;
	val += val * status->int_/100;

	if (sd->class_&JOBL_UPPER)
		val += val * 25/100;
	else if (sd->class_&JOBL_BABY)
		val -= val * 30/100;
	if ((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && pc_famerank(sd->status.char_id, MAPID_TAEKWON))
		val *= 3; //Triple max SP for top ranking Taekwons over level 90.

	return val;
}


//Calculates player data from scratch without counting SC adjustments.
//Should be invoked whenever players raise stats, learn passive skills or change equipment.
int status_calc_pc_(struct map_session_data* sd, bool first)
{
	static int calculating = 0; //Check for recursive call preemption. [Skotlex]
	struct status_data *status; // pointer to the player's base status
	const struct status_change *sc = &sd->sc;
	struct s_skill b_skill[MAX_SKILL]; // previous skill tree
	int b_weight, b_max_weight; // previous weight
	int i,index;
	int skill,refinedef=0;

	if (++calculating > 10) //Too many recursive calls!
		return -1;

	// remember player-specific values that are currently being shown to the client (for refresh purposes)
	memcpy(b_skill, &sd->status.skill, sizeof(b_skill));
	b_weight = sd->weight;
	b_max_weight = sd->max_weight;

	pc_calc_skilltree(sd);	// スキルツリ?の計算

	sd->max_weight = max_weight_base[pc_class2idx(sd->status.class_)]+sd->status.str*300;

	if(first) {
		//Load Hp/SP from char-received data.
		sd->battle_status.hp = sd->status.hp;
		sd->battle_status.sp = sd->status.sp;
		sd->regen.sregen = &sd->sregen;
		sd->regen.ssregen = &sd->ssregen;
		sd->weight=0;
		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid==0 || sd->inventory_data[i] == NULL)
				continue;
			sd->weight += sd->inventory_data[i]->weight*sd->status.inventory[i].amount;
		}
		sd->cart_weight=0;
		sd->cart_num=0;
		for(i=0;i<MAX_CART;i++){
			if(sd->status.cart[i].nameid==0)
				continue;
			sd->cart_weight+=itemdb_weight(sd->status.cart[i].nameid)*sd->status.cart[i].amount;
			sd->cart_num++;
		}
	}

	status = &sd->base_status;
	// these are not zeroed. [zzo]
	sd->hprate=100;
	sd->sprate=100;
	sd->castrate=100;
	sd->delayrate=100;
	sd->dsprate=100;
	sd->hprecov_rate = 100;
	sd->sprecov_rate = 100;
	sd->matk_rate = 100;
	sd->critical_rate = sd->hit_rate = sd->flee_rate = sd->flee2_rate = 100;
	sd->def_rate = sd->def2_rate = sd->mdef_rate = sd->mdef2_rate = 100;
	sd->regen.state.block = 0;

	// zeroed arrays, order follows the order in pc.h.
	// add new arrays to the end of zeroed area in pc.h (see comments) and size here. [zzo]
	memset (sd->param_bonus, 0, sizeof(sd->param_bonus)
		+ sizeof(sd->param_equip)
		+ sizeof(sd->subele)
		+ sizeof(sd->subrace)
		+ sizeof(sd->subrace2)
		+ sizeof(sd->subsize)
		+ sizeof(sd->reseff)
		+ sizeof(sd->weapon_coma_ele)
		+ sizeof(sd->weapon_coma_race)
		+ sizeof(sd->weapon_atk)
		+ sizeof(sd->weapon_atk_rate)
		+ sizeof(sd->arrow_addele) 
		+ sizeof(sd->arrow_addrace)
		+ sizeof(sd->arrow_addsize)
		+ sizeof(sd->magic_addele)
		+ sizeof(sd->magic_addrace)
		+ sizeof(sd->magic_addsize)
		+ sizeof(sd->critaddrace)
		+ sizeof(sd->expaddrace)
		+ sizeof(sd->ignore_mdef)
		+ sizeof(sd->ignore_def)
		+ sizeof(sd->itemgrouphealrate)
		+ sizeof(sd->sp_gain_race)
		);

	memset (&sd->right_weapon.overrefine, 0, sizeof(sd->right_weapon) - sizeof(sd->right_weapon.atkmods));
	memset (&sd->left_weapon.overrefine, 0, sizeof(sd->left_weapon) - sizeof(sd->left_weapon.atkmods));

	if (sd->special_state.intravision) //Clear status change.
		clif_status_load(&sd->bl, SI_INTRAVISION, 0);

	memset(&sd->special_state,0,sizeof(sd->special_state));
	memset(&status->max_hp, 0, sizeof(struct status_data)-(sizeof(status->hp)+sizeof(status->sp)));

	//FIXME: Most of these stuff should be calculated once, but how do I fix the memset above to do that? [Skotlex]
	status->speed = DEFAULT_WALK_SPEED;
	//Give them all modes except these (useful for clones)
	status->mode = MD_MASK&~(MD_BOSS|MD_PLANT|MD_DETECTOR|MD_ANGRY|MD_TARGETWEAK);

	status->size = (sd->class_&JOBL_BABY)?0:1;
	if (battle_config.character_size && pc_isriding(sd)) { //[Lupus]
		if (sd->class_&JOBL_BABY) {
			if (battle_config.character_size&2)
				status->size++;
		} else
		if(battle_config.character_size&1)
			status->size++;
	}
	status->aspd_rate = 1000;
	status->ele_lv = 1;
	status->race = RC_DEMIHUMAN;

	//zero up structures...
	memset(&sd->autospell,0,sizeof(sd->autospell)
		+ sizeof(sd->autospell2)
		+ sizeof(sd->autospell3)
		+ sizeof(sd->addeff)
		+ sizeof(sd->addeff2)
		+ sizeof(sd->addeff3)
		+ sizeof(sd->skillatk)
		+ sizeof(sd->skillheal)
		+ sizeof(sd->skillheal2)
		+ sizeof(sd->hp_loss)
		+ sizeof(sd->sp_loss)
		+ sizeof(sd->hp_regen)
		+ sizeof(sd->sp_regen)
		+ sizeof(sd->skillblown)
		+ sizeof(sd->skillcast)
		+ sizeof(sd->add_def)
		+ sizeof(sd->add_mdef)
		+ sizeof(sd->add_mdmg)
		+ sizeof(sd->add_drop)
		+ sizeof(sd->itemhealrate)
		+ sizeof(sd->subele2)
	);
	
	// vars zeroing. ints, shorts, chars. in that order.
	memset (&sd->atk_rate, 0,sizeof(sd->atk_rate)
		+ sizeof(sd->arrow_atk)
		+ sizeof(sd->arrow_ele)
		+ sizeof(sd->arrow_cri)
		+ sizeof(sd->arrow_hit)
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
		+ sizeof(sd->classchange)
		+ sizeof(sd->speed_rate)
		+ sizeof(sd->speed_add_rate)
		+ sizeof(sd->aspd_add)
		+ sizeof(sd->setitem_hash)
		+ sizeof(sd->setitem_hash2)
		// shorts
		+ sizeof(sd->splash_range)
		+ sizeof(sd->splash_add_range)
		+ sizeof(sd->add_steal_rate)
		+ sizeof(sd->add_heal_rate)
		+ sizeof(sd->add_heal2_rate)
		+ sizeof(sd->hp_gain_value)
		+ sizeof(sd->sp_gain_value)
		+ sizeof(sd->magic_hp_gain_value)
		+ sizeof(sd->magic_sp_gain_value)
		+ sizeof(sd->sp_vanish_rate)
		+ sizeof(sd->sp_vanish_per)
		+ sizeof(sd->unbreakable)
		+ sizeof(sd->unbreakable_equip)
		+ sizeof(sd->unstripable_equip)
		);

	// Autobonus
	pc_delautobonus(sd,sd->autobonus,ARRAYLENGTH(sd->autobonus),true);
	pc_delautobonus(sd,sd->autobonus2,ARRAYLENGTH(sd->autobonus),true);
	pc_delautobonus(sd,sd->autobonus3,ARRAYLENGTH(sd->autobonus),true);

	// Parse equipment.
	for(i=0;i<EQI_MAX-1;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index)
			continue;
		if(i == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index)
			continue;
		if(i == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index))
			continue;
		if(!sd->inventory_data[index])
			continue;

		status->def += sd->inventory_data[index]->def;

		if(first && sd->inventory_data[index]->equip_script)
	  	{	//Execute equip-script on login
			run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
			if (!calculating)
				return 1;
		}

		if(sd->inventory_data[index]->type == IT_WEAPON) {
			int r,wlv = sd->inventory_data[index]->wlv;
			struct weapon_data *wd;
			struct weapon_atk *wa;
			
			if (wlv >= MAX_REFINE_BONUS) 
				wlv = MAX_REFINE_BONUS - 1;
			if(i == EQI_HAND_L && sd->status.inventory[index].equip == EQP_HAND_L) {
				wd = &sd->left_weapon; // Left-hand weapon
				wa = &status->lhw;
			} else {
				wd = &sd->right_weapon;
				wa = &status->rhw;
			}
			wa->atk += sd->inventory_data[index]->atk;
			wa->atk2 = (r=sd->status.inventory[index].refine)*refinebonus[wlv][0];
			if((r-=refinebonus[wlv][2])>0) //Overrefine bonus.
				wd->overrefine = r*refinebonus[wlv][1];

			wa->range += sd->inventory_data[index]->range;
			if(sd->inventory_data[index]->script) {
				if (wd == &sd->left_weapon) {
					sd->state.lr_flag = 1;
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if (!calculating) //Abort, run_script retriggered this. [Skotlex]
					return 1;
			}

			if(sd->status.inventory[index].card[0]==CARD0_FORGE)
			{	// Forged weapon
				wd->star += (sd->status.inventory[index].card[1]>>8);
				if(wd->star >= 15) wd->star = 40; // 3 Star Crumbs now give +40 dmg
				if(pc_famerank(MakeDWord(sd->status.inventory[index].card[2],sd->status.inventory[index].card[3]) ,MAPID_BLACKSMITH))
					wd->star += 10;
				
				if (!wa->ele) //Do not overwrite element from previous bonuses.
					wa->ele = (sd->status.inventory[index].card[1]&0x0f);
			}
		}
		else if(sd->inventory_data[index]->type == IT_ARMOR) {
			refinedef += sd->status.inventory[index].refine*refinebonus[0][0];
			if(sd->inventory_data[index]->script) {
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if (!calculating) //Abort, run_script retriggered this. [Skotlex]
					return 1;
			}
		}
	}

	if(sd->equip_index[EQI_AMMO] >= 0){
		index = sd->equip_index[EQI_AMMO];
		if(sd->inventory_data[index]){		// Arrows
			sd->arrow_atk += sd->inventory_data[index]->atk;
			sd->state.lr_flag = 2;
			run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
				return 1;
		}
	}

	//Store equipment script bonuses 
	memcpy(sd->param_equip,sd->param_bonus,sizeof(sd->param_equip));
	memset(sd->param_bonus, 0, sizeof(sd->param_bonus));

	status->def += (refinedef+50)/100;

	//Parse Cards
	for(i=0;i<EQI_MAX-1;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index)
			continue;
		if(i == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index)
			continue;
		if(i == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index))
			continue;

		if(sd->inventory_data[index]) {
			int j,c;
			struct item_data *data;

			//Card script execution.
			if(itemdb_isspecial(sd->status.inventory[index].card[0]))
				continue;
			for(j=0;j<MAX_SLOTS;j++){ // Uses MAX_SLOTS to support Soul Bound system [Inkfish]
				current_equip_card_id= c= sd->status.inventory[index].card[j];
				if(!c)
					continue;
				data = itemdb_exists(c);
				if(!data)
					continue;
				if(first && data->equip_script)
			  	{	//Execute equip-script on login
					run_script(data->equip_script,0,sd->bl.id,0);
					if (!calculating)
						return 1;
				}
				if(!data->script)
					continue;
				if(data->flag.no_equip) { //Card restriction checks.
					if(map[sd->bl.m].flag.restricted && data->flag.no_equip&map[sd->bl.m].zone)
						continue;
					if(map[sd->bl.m].flag.pvp && data->flag.no_equip&1)
						continue;
					if(map_flag_gvg(sd->bl.m) && data->flag.no_equip&2) 
						continue;
				}
				if(i == EQI_HAND_L && sd->status.inventory[index].equip == EQP_HAND_L)
				{	//Left hand status.
					sd->state.lr_flag = 1;
					run_script(data->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(data->script,0,sd->bl.id,0);
				if (!calculating) //Abort, run_script his function. [Skotlex]
					return 1;
			}
		}
	}

	if( sc->count && sc->data[SC_ITEMSCRIPT] )
	{
		struct item_data *data = itemdb_exists(sc->data[SC_ITEMSCRIPT]->val1);
		if( data && data->script )
			run_script(data->script,0,sd->bl.id,0);
	}

	if( sd->pd )
	{ // Pet Bonus
		struct pet_data *pd = sd->pd;
		if( pd && pd->petDB && pd->petDB->equip_script && pd->pet.intimate >= battle_config.pet_equip_min_friendly )
			run_script(pd->petDB->equip_script,0,sd->bl.id,0);
		if( pd && pd->pet.intimate > 0 && (!battle_config.pet_equip_required || pd->pet.equip > 0) && pd->state.skillbonus == 1 && pd->bonus )
			pc_bonus(sd,pd->bonus->type, pd->bonus->val);
	}

	//param_bonus now holds card bonuses.
	if(status->rhw.range < 1) status->rhw.range = 1;
	if(status->lhw.range < 1) status->lhw.range = 1;
	if(status->rhw.range < status->lhw.range)
		status->rhw.range = status->lhw.range;

	sd->double_rate += sd->double_add_rate;
	sd->perfect_hit += sd->perfect_hit_add;
	sd->splash_range += sd->splash_add_range;

	// Damage modifiers from weapon type
	sd->right_weapon.atkmods[0] = atkmods[0][sd->weapontype1];
	sd->right_weapon.atkmods[1] = atkmods[1][sd->weapontype1];
	sd->right_weapon.atkmods[2] = atkmods[2][sd->weapontype1];
	sd->left_weapon.atkmods[0] = atkmods[0][sd->weapontype2];
	sd->left_weapon.atkmods[1] = atkmods[1][sd->weapontype2];
	sd->left_weapon.atkmods[2] = atkmods[2][sd->weapontype2];

	if(pc_isriding(sd) &&
		(sd->status.weapon==W_1HSPEAR || sd->status.weapon==W_2HSPEAR))
	{	//When Riding with spear, damage modifier to mid-class becomes 
		//same as versus large size.
		sd->right_weapon.atkmods[1] = sd->right_weapon.atkmods[2];
		sd->left_weapon.atkmods[1] = sd->left_weapon.atkmods[2];
	}

// ----- STATS CALCULATION -----

	// Job bonuses
	index = pc_class2idx(sd->status.class_);
	for(i=0;i<(int)sd->status.job_level && i<MAX_LEVEL;i++){
		if(!job_bonus[index][i])
			continue;
		switch(job_bonus[index][i]) {
			case 1: status->str++; break;
			case 2: status->agi++; break;
			case 3: status->vit++; break;
			case 4: status->int_++; break;
			case 5: status->dex++; break;
			case 6: status->luk++; break;
		}
	}

	// If a Super Novice has never died and is at least joblv 70, he gets all stats +10
	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->die_counter == 0 && sd->status.job_level >= 70){
		status->str += 10;
		status->agi += 10;
		status->vit += 10;
		status->int_+= 10;
		status->dex += 10;
		status->luk += 10;
	}

	// Absolute modifiers from passive skills
	if(pc_checkskill(sd,BS_HILTBINDING)>0)
		status->str++;
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0)
		status->int_ += (skill+1)/2; // +1 INT / 2 lv
	if((skill=pc_checkskill(sd,AC_OWL))>0)
		status->dex += skill;

	// Bonuses from cards and equipment as well as base stat, remember to avoid overflows.
	i = status->str + sd->status.str + sd->param_bonus[0] + sd->param_equip[0];
	status->str = cap_value(i,0,USHRT_MAX);
	i = status->agi + sd->status.agi + sd->param_bonus[1] + sd->param_equip[1];
	status->agi = cap_value(i,0,USHRT_MAX);
	i = status->vit + sd->status.vit + sd->param_bonus[2] + sd->param_equip[2];
	status->vit = cap_value(i,0,USHRT_MAX);
	i = status->int_+ sd->status.int_+ sd->param_bonus[3] + sd->param_equip[3];
	status->int_ = cap_value(i,0,USHRT_MAX);
	i = status->dex + sd->status.dex + sd->param_bonus[4] + sd->param_equip[4];
	status->dex = cap_value(i,0,USHRT_MAX);
	i = status->luk + sd->status.luk + sd->param_bonus[5] + sd->param_equip[5];
	status->luk = cap_value(i,0,USHRT_MAX);

// ------ BASE ATTACK CALCULATION ------

	// Base batk value is set on status_calc_misc
	// weapon-type bonus (FIXME: Why is the weapon_atk bonus applied to base attack?)
	if (sd->status.weapon < MAX_WEAPON_TYPE && sd->weapon_atk[sd->status.weapon])
		status->batk += sd->weapon_atk[sd->status.weapon];
	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,BS_HILTBINDING))>0)
		status->batk += 4;

// ----- HP MAX CALCULATION -----

	// Basic MaxHP value
	//We hold the standard Max HP here to make it faster to recalculate on vit changes.
	sd->status.max_hp = status_base_pc_maxhp(sd,status);
	//This is done to handle underflows from negative Max HP bonuses
	i = sd->status.max_hp + (int)status->max_hp;
	status->max_hp = cap_value(i, 0, INT_MAX);

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,CR_TRUST))>0)
		status->max_hp += skill*200;

	// Apply relative modifiers from equipment
	if(sd->hprate < 0)
		sd->hprate = 0;
	if(sd->hprate!=100)
		status->max_hp = status->max_hp * sd->hprate/100;
	if(battle_config.hp_rate != 100)
		status->max_hp = status->max_hp * battle_config.hp_rate/100;

	if(status->max_hp > (unsigned int)battle_config.max_hp)
		status->max_hp = battle_config.max_hp;
	else if(!status->max_hp)
		status->max_hp = 1;

// ----- SP MAX CALCULATION -----

	// Basic MaxSP value
	sd->status.max_sp = status_base_pc_maxsp(sd,status);
	//This is done to handle underflows from negative Max SP bonuses
	i = sd->status.max_sp + (int)status->max_sp;
	status->max_sp = cap_value(i, 0, INT_MAX);

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,SL_KAINA))>0)
		status->max_sp += 30*skill;
	if((skill=pc_checkskill(sd,HP_MEDITATIO))>0)
		status->max_sp += status->max_sp * skill/100;
	if((skill=pc_checkskill(sd,HW_SOULDRAIN))>0)
		status->max_sp += status->max_sp * 2*skill/100;

	// Apply relative modifiers from equipment
	if(sd->sprate < 0)
		sd->sprate = 0;
	if(sd->sprate!=100)
		status->max_sp = status->max_sp * sd->sprate/100;
	if(battle_config.sp_rate != 100)
		status->max_sp = status->max_sp * battle_config.sp_rate/100;

	if(status->max_sp > (unsigned int)battle_config.max_sp)
		status->max_sp = battle_config.max_sp;
	else if(!status->max_sp)
		status->max_sp = 1;

// ----- RESPAWN HP/SP -----
// 
	//Calc respawn hp and store it on base_status
	if (sd->special_state.restart_full_recover)
	{
		status->hp = status->max_hp;
		status->sp = status->max_sp;
	} else {
		if((sd->class_&MAPID_BASEMASK) == MAPID_NOVICE && !(sd->class_&JOBL_2) 
			&& battle_config.restart_hp_rate < 50) 
			status->hp=status->max_hp>>1;
		else 
			status->hp=status->max_hp * battle_config.restart_hp_rate/100;
		if(!status->hp)
			status->hp = 1;

		status->sp = status->max_sp * battle_config.restart_sp_rate /100;
	}

// ----- MISC CALCULATION -----
	status_calc_misc(&sd->bl, status, sd->status.base_level);

	//Equipment modifiers for misc settings
	if(sd->matk_rate < 0)
		sd->matk_rate = 0;
	if(sd->matk_rate != 100){
		status->matk_max = status->matk_max * sd->matk_rate/100;
		status->matk_min = status->matk_min * sd->matk_rate/100;
	}

	if(sd->hit_rate < 0)
		sd->hit_rate = 0;
	if(sd->hit_rate != 100)
		status->hit = status->hit * sd->hit_rate/100;

	if(sd->flee_rate < 0)
		sd->flee_rate = 0;
	if(sd->flee_rate != 100)
		status->flee = status->flee * sd->flee_rate/100;

	if(sd->def2_rate < 0)
		sd->def2_rate = 0;
	if(sd->def2_rate != 100)
		status->def2 = status->def2 * sd->def2_rate/100;

	if(sd->mdef2_rate < 0)
		sd->mdef2_rate = 0;
	if(sd->mdef2_rate != 100)
		status->mdef2 = status->mdef2 * sd->mdef2_rate/100;

	if(sd->critical_rate < 0) 
		sd->critical_rate = 0;
	if(sd->critical_rate != 100)
		status->cri = status->cri * sd->critical_rate/100;

	if(sd->flee2_rate < 0)
		sd->flee2_rate = 0;
	if(sd->flee2_rate != 100)
		status->flee2 = status->flee2 * sd->flee2_rate/100;

// ----- HIT CALCULATION -----

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,BS_WEAPONRESEARCH))>0)
		status->hit += skill*2;
	if((skill=pc_checkskill(sd,AC_VULTURE))>0){
		status->hit += skill;
		if(sd->status.weapon == W_BOW)
			status->rhw.range += skill;
	}
	if(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE)
  	{
		if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0)
			status->hit += 2*skill;
		if((skill=pc_checkskill(sd,GS_SNAKEEYE))>0) {
			status->hit += skill;
			status->rhw.range += skill;
		}
	}

// ----- FLEE CALCULATION -----

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,TF_MISS))>0)
		status->flee += skill*(sd->class_&JOBL_2 && (sd->class_&MAPID_BASEMASK) == MAPID_THIEF? 4 : 3);
	if((skill=pc_checkskill(sd,MO_DODGE))>0)
		status->flee += (skill*3)>>1;

// ----- EQUIPMENT-DEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->def_rate < 0)
		sd->def_rate = 0;
	if(sd->def_rate != 100) {
		i =  status->def * sd->def_rate/100;
		status->def = cap_value(i, CHAR_MIN, CHAR_MAX);
	}

	if (!battle_config.weapon_defense_type && status->def > battle_config.max_def)
	{
		status->def2 += battle_config.over_def_bonus*(status->def -battle_config.max_def);
		status->def = (unsigned char)battle_config.max_def;
	}

// ----- EQUIPMENT-MDEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->mdef_rate < 0)
		sd->mdef_rate = 0;
	if(sd->mdef_rate != 100) {
		i =  status->mdef * sd->mdef_rate/100;
		status->mdef = cap_value(i, CHAR_MIN, CHAR_MAX);
	}

	if (!battle_config.magic_defense_type && status->mdef > battle_config.max_def)
	{
		status->mdef2 += battle_config.over_def_bonus*(status->mdef -battle_config.max_def);
		status->mdef = (signed char)battle_config.max_def;
	}

// ----- ASPD CALCULATION -----
// Unlike other stats, ASPD rate modifiers from skills/SCs/items/etc are first all added together, then the final modifier is applied

	// Basic ASPD value
	i = status_base_amotion_pc(sd,status);
	status->amotion = cap_value(i,battle_config.max_aspd,2000);

	// Relative modifiers from passive skills
	if((skill=pc_checkskill(sd,SA_ADVANCEDBOOK))>0 && sd->status.weapon == W_BOOK)
		status->aspd_rate -= 5*skill;
	if((skill = pc_checkskill(sd,SG_DEVIL)) > 0 && !pc_nextjobexp(sd))
		status->aspd_rate -= 30*skill;
	if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0 &&
		(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		status->aspd_rate -= ((skill+1)/2) * 10;
	if(pc_isriding(sd))
		status->aspd_rate += 500-100*pc_checkskill(sd,KN_CAVALIERMASTERY);

	status->adelay = 2*status->amotion;


// ----- DMOTION -----
//
	i =  800-status->agi*4;
	status->dmotion = cap_value(i, 400, 800);
	if(battle_config.pc_damage_delay_rate != 100)
		status->dmotion = status->dmotion*battle_config.pc_damage_delay_rate/100;

// ----- MISC CALCULATIONS -----

	// Weight
	if((skill=pc_checkskill(sd,MC_INCCARRY))>0)
		sd->max_weight += 2000*skill;
	if(pc_isriding(sd) && pc_checkskill(sd,KN_RIDING)>0)
		sd->max_weight += 10000;
	if(sc->data[SC_KNOWLEDGE])
		sd->max_weight += sd->max_weight*sc->data[SC_KNOWLEDGE]->val1/10;
	if((skill=pc_checkskill(sd,ALL_INCCARRY))>0)
		sd->max_weight += 2000*skill;

	if (pc_checkskill(sd,SM_MOVINGRECOVERY)>0)
		sd->regen.state.walk = 1;
	else
		sd->regen.state.walk = 0;

	// Skill SP cost
	if((skill=pc_checkskill(sd,HP_MANARECHARGE))>0 )
		sd->dsprate -= 4*skill;

	if(sc->data[SC_SERVICE4U])
		sd->dsprate -= sc->data[SC_SERVICE4U]->val3;

	if(sc->data[SC_SPCOST_RATE])
		sd->dsprate -= sc->data[SC_SPCOST_RATE]->val1;

	//Underflow protections.
	if(sd->dsprate < 0)
		sd->dsprate = 0;
	if(sd->castrate < 0)
		sd->castrate = 0;
	if(sd->delayrate < 0)
		sd->delayrate = 0;
	if(sd->hprecov_rate < 0)
		sd->hprecov_rate = 0;
	if(sd->sprecov_rate < 0)
		sd->sprecov_rate = 0;

	// Anti-element and anti-race
	if((skill=pc_checkskill(sd,CR_TRUST))>0)
		sd->subele[ELE_HOLY] += skill*5;
	if((skill=pc_checkskill(sd,BS_SKINTEMPER))>0) {
		sd->subele[ELE_NEUTRAL] += skill;
		sd->subele[ELE_FIRE] += skill*4;
	}
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0 ){
		skill = skill*4;
		sd->right_weapon.addrace[RC_DRAGON]+=skill;
		sd->left_weapon.addrace[RC_DRAGON]+=skill;
		sd->magic_addrace[RC_DRAGON]+=skill;
		sd->subrace[RC_DRAGON]+=skill;
	}

	if(sc->count){
     	if(sc->data[SC_CONCENTRATE])
		{	//Update the card-bonus data
			sc->data[SC_CONCENTRATE]->val3 = sd->param_bonus[1]; //Agi
			sc->data[SC_CONCENTRATE]->val4 = sd->param_bonus[4]; //Dex
		}
     	if(sc->data[SC_SIEGFRIED]){
			i = sc->data[SC_SIEGFRIED]->val2;
			sd->subele[ELE_WATER] += i;
			sd->subele[ELE_EARTH] += i;
			sd->subele[ELE_FIRE] += i;
			sd->subele[ELE_WIND] += i;
			sd->subele[ELE_POISON] += i;
			sd->subele[ELE_HOLY] += i;
			sd->subele[ELE_DARK] += i;
			sd->subele[ELE_GHOST] += i;
			sd->subele[ELE_UNDEAD] += i;
		}
		if(sc->data[SC_PROVIDENCE]){
			sd->subele[ELE_HOLY] += sc->data[SC_PROVIDENCE]->val2;
			sd->subrace[RC_DEMON] += sc->data[SC_PROVIDENCE]->val2;
		}
		if(sc->data[SC_ARMOR_ELEMENT])
		{	//This status change should grant card-type elemental resist.
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_ELEMENT]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_ELEMENT]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_ELEMENT]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_ELEMENT]->val4;
		}
		if(sc->data[SC_ARMOR_RESIST])
		{ // Undead Scroll
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_RESIST]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_RESIST]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_RESIST]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_RESIST]->val4;
		}
	}

	status_cpy(&sd->battle_status, status);

// ----- CLIENT-SIDE REFRESH -----
	if(memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill)))
		clif_skillinfoblock(sd);
	if(b_weight != sd->weight)
		clif_updatestatus(sd,SP_WEIGHT);
	if(b_max_weight != sd->max_weight) {
		clif_updatestatus(sd,SP_MAXWEIGHT);
		pc_updateweightstatus(sd);
	}

	calculating = 0;

	return 0;
}

int status_calc_mercenary_(struct mercenary_data *md, bool first)
{
	struct status_data *status = &md->base_status;
	struct s_mercenary *merc = &md->mercenary;

	if( first )
	{
		memcpy(status, &md->db->status, sizeof(struct status_data));
		status->mode = MD_CANMOVE|MD_CANATTACK;
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		md->battle_status.hp = merc->hp;
		md->battle_status.sp = merc->sp;
	}

	status_calc_misc(&md->bl, status, md->db->lv);
	status_cpy(&md->battle_status, status);

	return 0;
}

int status_calc_homunculus_(struct homun_data *hd, bool first)
{
	struct status_data *status = &hd->base_status;
	struct s_homunculus *hom = &hd->homunculus;
	int skill;
	int amotion;

	status->str = hom->str / 10;
	status->agi = hom->agi / 10;
	status->vit = hom->vit / 10;
	status->dex = hom->dex / 10;
	status->int_ = hom->int_ / 10;
	status->luk = hom->luk / 10;

	if (first) {	//[orn]
		const struct s_homunculus_db *db = hd->homunculusDB;
		status->def_ele =  db->element;
		status->ele_lv = 1;
		status->race = db->race;
		status->size = (hom->class_ == db->evo_class)?db->evo_size:db->base_size;
		status->rhw.range = 1 + status->size;
		status->mode = MD_CANMOVE|MD_CANATTACK;
		status->speed = DEFAULT_WALK_SPEED;
		if (battle_config.hom_setting&0x8 && hd->master)
			status->speed = status_get_speed(&hd->master->bl);

		status->hp = 1;
		status->sp = 1;
	}
	skill = hom->level/10 + status->vit/5;
	status->def = cap_value(skill, 0, 99);

	skill = hom->level/10 + status->int_/5;
	status->mdef = cap_value(skill, 0, 99);

	status->max_hp = hom->max_hp ;
	status->max_sp = hom->max_sp ;

	merc_hom_calc_skilltree(hd);

	if((skill=merc_hom_checkskill(hd,HAMI_SKIN)) > 0)
		status->def +=	skill * 4;

	if((skill = merc_hom_checkskill(hd,HVAN_INSTRUCT)) > 0)
	{
		status->int_ += 1 +skill/2 +skill/4 +skill/5;
		status->str  += 1 +skill/3 +skill/3 +skill/4;
	}

	if((skill=merc_hom_checkskill(hd,HAMI_SKIN)) > 0)
		status->max_hp += skill * 2 * status->max_hp / 100;

	if((skill = merc_hom_checkskill(hd,HLIF_BRAIN)) > 0)
		status->max_sp += (1 +skill/2 -skill/4 +skill/5) * status->max_sp / 100 ;

	if (first) {
		hd->battle_status.hp = hom->hp ;
		hd->battle_status.sp = hom->sp ;
	}

	status->rhw.atk = status->dex;
	status->rhw.atk2 = status->str + hom->level;

	status->aspd_rate = 1000;

	amotion = (1000 -4*status->agi -status->dex) * hd->homunculusDB->baseASPD/1000;
	status->amotion = cap_value(amotion,battle_config.max_aspd,2000);
	status->adelay = status->amotion; //It seems adelay = amotion for Homunculus.

	status_calc_misc(&hd->bl, status, hom->level);
	status_cpy(&hd->battle_status, status);

	return 1;
}

static unsigned short status_calc_str(struct block_list *,struct status_change *,int);
static unsigned short status_calc_agi(struct block_list *,struct status_change *,int);
static unsigned short status_calc_vit(struct block_list *,struct status_change *,int);
static unsigned short status_calc_int(struct block_list *,struct status_change *,int);
static unsigned short status_calc_dex(struct block_list *,struct status_change *,int);
static unsigned short status_calc_luk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_batk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_watk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_matk(struct block_list *,struct status_change *,int);
static signed short status_calc_hit(struct block_list *,struct status_change *,int);
static signed short status_calc_critical(struct block_list *,struct status_change *,int);
static signed short status_calc_flee(struct block_list *,struct status_change *,int);
static signed short status_calc_flee2(struct block_list *,struct status_change *,int);
static signed char status_calc_def(struct block_list *,struct status_change *,int);
static signed short status_calc_def2(struct block_list *,struct status_change *,int);
static signed char status_calc_mdef(struct block_list *,struct status_change *,int);
static signed short status_calc_mdef2(struct block_list *,struct status_change *,int);
static unsigned short status_calc_speed(struct block_list *,struct status_change *,int);
static short status_calc_aspd_rate(struct block_list *,struct status_change *,int);
static unsigned short status_calc_dmotion(struct block_list *bl, struct status_change *sc, int dmotion);
static unsigned int status_calc_maxhp(struct block_list *,struct status_change *,unsigned int);
static unsigned int status_calc_maxsp(struct block_list *,struct status_change *,unsigned int);
static unsigned char status_calc_element(struct block_list *bl, struct status_change *sc, int element);
static unsigned char status_calc_element_lv(struct block_list *bl, struct status_change *sc, int lv);
static unsigned short status_calc_mode(struct block_list *bl, struct status_change *sc, int mode);

//Calculates base regen values.
void status_calc_regen(struct block_list *bl, struct status_data *status, struct regen_data *regen)
{
	struct map_session_data *sd;
	int val, skill;

	if( !(bl->type&BL_REGEN) || !regen )
		return;

	sd = BL_CAST(BL_PC,bl);
	val = 1 + (status->vit/5) + (status->max_hp/200);

	if( sd && sd->hprecov_rate != 100 )
		val = val*sd->hprecov_rate/100;

	regen->hp = cap_value(val, 1, SHRT_MAX);

	val = 1 + (status->int_/6) + (status->max_sp/100);
	if( status->int_ >= 120 )
		val += ((status->int_-120)>>1) + 4;

	if( sd && sd->sprecov_rate != 100 )
		val = val*sd->sprecov_rate/100;

	regen->sp = cap_value(val, 1, SHRT_MAX);

	if( sd )
	{
		struct regen_data_sub *sregen;
		if( (skill=pc_checkskill(sd,HP_MEDITATIO)) > 0 )
		{
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
		//Only players have skill/sitting skill regen for now.
		sregen = regen->sregen;

		val = 0;
		if( (skill=pc_checkskill(sd,SM_RECOVERY)) > 0 )
			val += skill*5 + skill*status->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=pc_checkskill(sd,MG_SRECOVERY)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		if( (skill=pc_checkskill(sd,NJ_NINPOU)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		sregen->sp = cap_value(val, 0, SHRT_MAX);

		// Skill-related recovery (only when sit)
		sregen = regen->ssregen;

		val = 0;
		if( (skill=pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*4 + skill*status->max_hp/500;

		if( (skill=pc_checkskill(sd,TK_HPTIME)) > 0 && sd->state.rest )
			val += skill*30 + skill*status->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=pc_checkskill(sd,TK_SPTIME)) > 0 && sd->state.rest )
		{
			val += skill*3 + skill*status->max_sp/500;
			if ((skill=pc_checkskill(sd,SL_KAINA)) > 0) //Power up Enjoyable Rest
				val += (30+10*skill)*val/100;
		}
		if( (skill=pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*2 + skill*status->max_sp/500;
		sregen->sp = cap_value(val, 0, SHRT_MAX);
	}

	if( bl->type == BL_HOM )
	{
		struct homun_data *hd = (TBL_HOM*)bl;
		if( (skill = merc_hom_checkskill(hd,HAMI_SKIN)) > 0 )
		{
			val = regen->hp*(100+5*skill)/100;
			regen->hp = cap_value(val, 1, SHRT_MAX);
		}
		if( (skill = merc_hom_checkskill(hd,HLIF_BRAIN)) > 0 )
		{
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
	}
	else if( bl->type == BL_MER )
	{
		val = (status->max_hp * status->vit / 10000 + 1) * 6;
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = (status->max_sp * (status->int_ + 10) / 750) + 1;
		regen->sp = cap_value(val, 1, SHRT_MAX);
	}
}

//Calculates SC related regen rates.
void status_calc_regen_rate(struct block_list *bl, struct regen_data *regen, struct status_change *sc)
{
	if (!(bl->type&BL_REGEN) || !regen)
		return;

	regen->flag = RGN_HP|RGN_SP;
	if(regen->sregen)
	{
		if (regen->sregen->hp)
			regen->flag|=RGN_SHP;

		if (regen->sregen->sp)
			regen->flag|=RGN_SSP;
		regen->sregen->rate.hp = regen->sregen->rate.sp = 1;
	}
	if (regen->ssregen)
	{
		if (regen->ssregen->hp)
			regen->flag|=RGN_SHP;

		if (regen->ssregen->sp)
			regen->flag|=RGN_SSP;
		regen->ssregen->rate.hp = regen->ssregen->rate.sp = 1;
	}
	regen->rate.hp = regen->rate.sp = 1;

	if (!sc || !sc->count)
		return;

	if (
		(sc->data[SC_POISON] && !sc->data[SC_SLOWPOISON])
		|| (sc->data[SC_DPOISON] && !sc->data[SC_SLOWPOISON])
		|| sc->data[SC_BERSERK]
		|| sc->data[SC_TRICKDEAD]
		|| sc->data[SC_BLEEDING]
	)	//No regen
		regen->flag = 0;

	if (
		sc->data[SC_DANCING]
		|| (
			(((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK &&
			(sc->data[SC_EXTREMITYFIST] || (sc->data[SC_EXPLOSIONSPIRITS] && (!sc->data[SC_SPIRIT] || sc->data[SC_SPIRIT]->val2 != SL_MONK)))
			)
	)	//No natural SP regen
		regen->flag &=~RGN_SP;

	if(
		sc->data[SC_TENSIONRELAX]
	  ) {
		regen->rate.hp += 2;
		if (regen->sregen)
			regen->sregen->rate.hp += 3;
	}
	if (sc->data[SC_MAGNIFICAT])
	{
		regen->rate.hp += 1;
		regen->rate.sp += 1;
	}
	if (sc->data[SC_REGENERATION])
	{
		const struct status_change_entry *sce = sc->data[SC_REGENERATION];
		if (!sce->val4)
		{
			regen->rate.hp += sce->val2;
			regen->rate.sp += sce->val3;
		} else
			regen->flag&=~sce->val4; //Remove regen as specified by val4
	}
}

/// Recalculates parts of an object's battle status according to the specified flags.
/// @param flag bitfield of values from enum scb_flag
void status_calc_bl_main(struct block_list *bl, enum scb_flag flag)
{
	const struct status_data *b_status = status_get_base_status(bl);
	struct status_data *status = status_get_status_data(bl);
	struct status_change *sc = status_get_sc(bl);
	TBL_PC *sd = BL_CAST(BL_PC,bl);
	int temp;

	if (!b_status || !status)
		return;

	if((!(bl->type&BL_REGEN)) && (!sc || !sc->count)) { //No difference.
		status_cpy(status, b_status);
		return;
	}

	if(flag&SCB_STR) {
		status->str = status_calc_str(bl, sc, b_status->str);
		flag|=SCB_BATK;
		if( bl->type&BL_HOM )
			flag |= SCB_WATK;
	}

	if(flag&SCB_AGI) {
		status->agi = status_calc_agi(bl, sc, b_status->agi);
		flag|=SCB_FLEE;
		if( bl->type&(BL_PC|BL_HOM) )
			flag |= SCB_ASPD|SCB_DSPD;
	}

	if(flag&SCB_VIT) {
		status->vit = status_calc_vit(bl, sc, b_status->vit);
		flag|=SCB_DEF2|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER) )
			flag |= SCB_MAXHP;
		if( bl->type&BL_HOM )
			flag |= SCB_DEF;
	}

	if(flag&SCB_INT) {
		status->int_ = status_calc_int(bl, sc, b_status->int_);
		flag|=SCB_MATK|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER) )
			flag |= SCB_MAXSP;
		if( bl->type&BL_HOM )
			flag |= SCB_MDEF;
	}

	if(flag&SCB_DEX) {
		status->dex = status_calc_dex(bl, sc, b_status->dex);
		flag|=SCB_BATK|SCB_HIT;
		if( bl->type&(BL_PC|BL_HOM) )
			flag |= SCB_ASPD;
		if( bl->type&BL_HOM )
			flag |= SCB_WATK;
	}

	if(flag&SCB_LUK) {
		status->luk = status_calc_luk(bl, sc, b_status->luk);
		flag|=SCB_BATK|SCB_CRI|SCB_FLEE2;
	}

	if(flag&SCB_BATK && b_status->batk) {
		status->batk = status_base_atk(bl,status);
		temp = b_status->batk - status_base_atk(bl,b_status);
		if (temp)
		{
			temp += status->batk;
			status->batk = cap_value(temp, 0, USHRT_MAX);
		}
		status->batk = status_calc_batk(bl, sc, status->batk);
	}

	if(flag&SCB_WATK) {

		status->rhw.atk = status_calc_watk(bl, sc, b_status->rhw.atk);
		if (!sd) //Should not affect weapon refine bonus
			status->rhw.atk2 = status_calc_watk(bl, sc, b_status->rhw.atk2);

		if(b_status->lhw.atk) {
			if (sd) {
				sd->state.lr_flag = 1;
				status->lhw.atk = status_calc_watk(bl, sc, b_status->lhw.atk);
				sd->state.lr_flag = 0;
			} else {
				status->lhw.atk = status_calc_watk(bl, sc, b_status->lhw.atk);
				status->lhw.atk2= status_calc_watk(bl, sc, b_status->lhw.atk2);
			}
		}

		if( bl->type&BL_HOM )
		{
			status->rhw.atk += (status->dex - b_status->dex);
			status->rhw.atk2 += (status->str - b_status->str);
			if( status->rhw.atk2 < status->rhw.atk )
				status->rhw.atk2 = status->rhw.atk;
		}
	}

	if(flag&SCB_HIT) {
		if (status->dex == b_status->dex)
			status->hit = status_calc_hit(bl, sc, b_status->hit);
		else
			status->hit = status_calc_hit(bl, sc, b_status->hit +(status->dex - b_status->dex));
	}

	if(flag&SCB_FLEE) {
		if (status->agi == b_status->agi)
			status->flee = status_calc_flee(bl, sc, b_status->flee);
		else
			status->flee = status_calc_flee(bl, sc, b_status->flee +(status->agi - b_status->agi));
	}

	if(flag&SCB_DEF)
	{
		status->def = status_calc_def(bl, sc, b_status->def);

		if( bl->type&BL_HOM )
			status->def += (status->vit/5 - b_status->vit/5);
	}

	if(flag&SCB_DEF2) {
		if (status->vit == b_status->vit)
			status->def2 = status_calc_def2(bl, sc, b_status->def2);
		else
			status->def2 = status_calc_def2(bl, sc, b_status->def2 + (status->vit - b_status->vit));
	}

	if(flag&SCB_MDEF)
	{
		status->mdef = status_calc_mdef(bl, sc, b_status->mdef);
	
		if( bl->type&BL_HOM )
			status->mdef += (status->int_/5 - b_status->int_/5);
	}
		
	if(flag&SCB_MDEF2) {
		if (status->int_ == b_status->int_ && status->vit == b_status->vit)
			status->mdef2 = status_calc_mdef2(bl, sc, b_status->mdef2);
		else
			status->mdef2 = status_calc_mdef2(bl, sc, b_status->mdef2 +(status->int_ - b_status->int_) +((status->vit - b_status->vit)>>1));
	}

	if(flag&SCB_SPEED) {
		struct unit_data *ud = unit_bl2ud(bl);
		status->speed = status_calc_speed(bl, sc, b_status->speed);

		//Re-walk to adjust speed (we do not check if walktimer != -1
		//because if you step on something while walking, the moment this
		//piece of code triggers the walk-timer is set on -1) [Skotlex]
	  	if (ud)
			ud->state.change_walk_target = ud->state.speed_changed = 1;

		if( bl->type&BL_PC && status->speed < battle_config.max_walk_speed )
			status->speed = battle_config.max_walk_speed;

		if( bl->type&BL_HOM && battle_config.hom_setting&0x8 && ((TBL_HOM*)bl)->master)
			status->speed = status_get_speed(&((TBL_HOM*)bl)->master->bl);


	}

	if(flag&SCB_CRI && b_status->cri) {
		if (status->luk == b_status->luk)
			status->cri = status_calc_critical(bl, sc, b_status->cri);
		else
			status->cri = status_calc_critical(bl, sc, b_status->cri + 3*(status->luk - b_status->luk));
	}

	if(flag&SCB_FLEE2 && b_status->flee2) {
		if (status->luk == b_status->luk)
			status->flee2 = status_calc_flee2(bl, sc, b_status->flee2);
		else
			status->flee2 = status_calc_flee2(bl, sc, b_status->flee2 +(status->luk - b_status->luk));
	}

	if(flag&SCB_ATK_ELE) {
		status->rhw.ele = status_calc_attack_element(bl, sc, b_status->rhw.ele);
		if (sd) sd->state.lr_flag = 1;
		status->lhw.ele = status_calc_attack_element(bl, sc, b_status->lhw.ele);
		if (sd) sd->state.lr_flag = 0;
	}

	if(flag&SCB_DEF_ELE) {
		status->def_ele = status_calc_element(bl, sc, b_status->def_ele);
		status->ele_lv = status_calc_element_lv(bl, sc, b_status->ele_lv);
	}

	if(flag&SCB_MODE)
	{
		status->mode = status_calc_mode(bl, sc, b_status->mode);
		//Since mode changed, reset their state.
		if (!(status->mode&MD_CANATTACK))
			unit_stop_attack(bl);
		if (!(status->mode&MD_CANMOVE))
			unit_stop_walking(bl,1);
	}

// No status changes alter these yet.
//	if(flag&SCB_SIZE)
// if(flag&SCB_RACE)
// if(flag&SCB_RANGE)

	if(flag&SCB_MAXHP) {
		if( bl->type&BL_PC )
		{
			status->max_hp = status_base_pc_maxhp(sd,status);
			status->max_hp += b_status->max_hp - sd->status.max_hp;

			status->max_hp = status_calc_maxhp(bl, sc, status->max_hp);

			if( status->max_hp > (unsigned int)battle_config.max_hp )
				status->max_hp = (unsigned int)battle_config.max_hp;
		}
		else
		{
			status->max_hp = status_calc_maxhp(bl, sc, b_status->max_hp);
		}

		if( status->hp > status->max_hp ) //FIXME: Should perhaps a status_zap should be issued?
		{
			status->hp = status->max_hp;
			if( sd ) clif_updatestatus(sd,SP_HP);
		}
	}

	if(flag&SCB_MAXSP) {
		if( bl->type&BL_PC )
		{
			status->max_sp = status_base_pc_maxsp(sd,status);
			status->max_sp += b_status->max_sp - sd->status.max_sp;

			status->max_sp = status_calc_maxsp(&sd->bl, &sd->sc, status->max_sp);

			if( status->max_sp > (unsigned int)battle_config.max_sp )
				status->max_sp = (unsigned int)battle_config.max_sp;
		}
		else
		{
			status->max_sp = status_calc_maxsp(bl, sc, b_status->max_sp);
		}

		if( status->sp > status->max_sp )
		{
			status->sp = status->max_sp;
			if( sd ) clif_updatestatus(sd,SP_SP);
		}
	}

	if(flag&SCB_MATK) {
		//New matk
		status->matk_min = status_base_matk_min(status);
		status->matk_max = status_base_matk_max(status);

		if( bl->type&BL_PC && sd->matk_rate != 100 )
		{
			//Bonuses from previous matk
			status->matk_max = status->matk_max * sd->matk_rate/100;
			status->matk_min = status->matk_min * sd->matk_rate/100;
		}
			
		status->matk_min = status_calc_matk(bl, sc, status->matk_min);
		status->matk_max = status_calc_matk(bl, sc, status->matk_max);

		if(sc->data[SC_MAGICPOWER]) { //Store current matk values
			sc->mp_matk_min = status->matk_min;
			sc->mp_matk_max = status->matk_max;
		}

		if( bl->type&BL_HOM && battle_config.hom_setting&0x20 ) //Hom Min Matk is always the same as Max Matk
			status->matk_min = status->matk_max;

	}

	if(flag&SCB_ASPD) {
		int amotion;
		if( bl->type&BL_PC )
		{
			amotion = status_base_amotion_pc(sd,status);
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);
			
			if(status->aspd_rate != 1000)
				amotion = amotion*status->aspd_rate/1000;
			
			status->amotion = cap_value(amotion,battle_config.max_aspd,2000);
			
			status->adelay = 2*status->amotion;
		}
		else
		if( bl->type&BL_HOM )
		{
			amotion = (1000 -4*status->agi -status->dex) * ((TBL_HOM*)bl)->homunculusDB->baseASPD/1000;			
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);
			
			if(status->aspd_rate != 1000)
				amotion = amotion*status->aspd_rate/1000;
			
			status->amotion = cap_value(amotion,battle_config.max_aspd,2000);
			
			status->adelay = status->amotion;
		}
		else // mercenary and mobs
		{
			amotion = b_status->amotion;
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);
			
			if(status->aspd_rate != 1000)
				amotion = amotion*status->aspd_rate/1000;
			
			status->amotion = cap_value(amotion, battle_config.monster_max_aspd, 2000);	
			
			temp = b_status->adelay*status->aspd_rate/1000;
			status->adelay = cap_value(temp, battle_config.monster_max_aspd*2, 4000);
		}
	}

	if(flag&SCB_DSPD) {
		int dmotion;
		if( bl->type&BL_PC )
		{
			if (b_status->agi == status->agi)
				status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
			else {
				dmotion = 800-status->agi*4;
				status->dmotion = cap_value(dmotion, 400, 800);
				if(battle_config.pc_damage_delay_rate != 100)
					status->dmotion = status->dmotion*battle_config.pc_damage_delay_rate/100;
				//It's safe to ignore b_status->dmotion since no bonus affects it.
				status->dmotion = status_calc_dmotion(bl, sc, status->dmotion);
			}
		}
		else
		if( bl->type&BL_HOM )
		{
			dmotion = 800-status->agi*4;
			status->dmotion = cap_value(dmotion, 400, 800);
			status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
		}
		else // mercenary and mobs
		{
			status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
		}
	}

	if(flag&(SCB_VIT|SCB_MAXHP|SCB_INT|SCB_MAXSP) && bl->type&BL_REGEN)
		status_calc_regen(bl, status, status_get_regen_data(bl));

	if(flag&SCB_REGEN && bl->type&BL_REGEN)
		status_calc_regen_rate(bl, status_get_regen_data(bl), sc);
}

/// Recalculates parts of an object's base status and battle status according to the specified flags.
/// Also sends updates to the client wherever applicable.
/// @param flag bitfield of values from enum scb_flag
/// @param first if true, will cause status_calc_* functions to run their base status initialization code
void status_calc_bl_(struct block_list* bl, enum scb_flag flag, bool first)
{
	struct status_data b_status; // previous battle status
	struct status_data* status; // pointer to current battle status

	// remember previous values
	status = status_get_status_data(bl);
	memcpy(&b_status, status, sizeof(struct status_data));

	if( flag&SCB_BASE )
	{// calculate the object's base status too
		switch( bl->type )
		{
		case BL_PC:  status_calc_pc_(BL_CAST(BL_PC,bl), first);          break;
		case BL_MOB: status_calc_mob_(BL_CAST(BL_MOB,bl), first);        break;
		case BL_PET: status_calc_pet_(BL_CAST(BL_PET,bl), first);        break;
		case BL_HOM: status_calc_homunculus_(BL_CAST(BL_HOM,bl), first); break;
		case BL_MER: status_calc_mercenary_(BL_CAST(BL_MER,bl), first);  break;
		}
	}

	if( first && bl->type == BL_MOB )
		return; // assume there will be no statuses active

	status_calc_bl_main(bl, flag);

	if( first && bl->type == BL_HOM )
		return; // client update handled by caller

	// compare against new values and send client updates
	if( bl->type == BL_PC )
	{
		TBL_PC* sd = BL_CAST(BL_PC, bl);
		if(b_status.str != status->str)
			clif_updatestatus(sd,SP_STR);
		if(b_status.agi != status->agi)
			clif_updatestatus(sd,SP_AGI);
		if(b_status.vit != status->vit)
			clif_updatestatus(sd,SP_VIT);
		if(b_status.int_ != status->int_)
			clif_updatestatus(sd,SP_INT);
		if(b_status.dex != status->dex)
			clif_updatestatus(sd,SP_DEX);
		if(b_status.luk != status->luk)
			clif_updatestatus(sd,SP_LUK);
		if(b_status.hit != status->hit)
			clif_updatestatus(sd,SP_HIT);
		if(b_status.flee != status->flee)
			clif_updatestatus(sd,SP_FLEE1);
		if(b_status.amotion != status->amotion)
			clif_updatestatus(sd,SP_ASPD);
		if(b_status.speed != status->speed)
			clif_updatestatus(sd,SP_SPEED);
		if(b_status.rhw.atk != status->rhw.atk || b_status.lhw.atk != status->lhw.atk || b_status.batk != status->batk)
			clif_updatestatus(sd,SP_ATK1);
		if(b_status.def != status->def)
			clif_updatestatus(sd,SP_DEF1);
		if(b_status.rhw.atk2 != status->rhw.atk2 || b_status.lhw.atk2 != status->lhw.atk2)
			clif_updatestatus(sd,SP_ATK2);
		if(b_status.def2 != status->def2)
			clif_updatestatus(sd,SP_DEF2);
		if(b_status.flee2 != status->flee2)
			clif_updatestatus(sd,SP_FLEE2);
		if(b_status.cri != status->cri)
			clif_updatestatus(sd,SP_CRITICAL);
		if(b_status.matk_max != status->matk_max)
			clif_updatestatus(sd,SP_MATK1);
		if(b_status.matk_min != status->matk_min)
			clif_updatestatus(sd,SP_MATK2);
		if(b_status.mdef != status->mdef)
			clif_updatestatus(sd,SP_MDEF1);
		if(b_status.mdef2 != status->mdef2)
			clif_updatestatus(sd,SP_MDEF2);
		if(b_status.rhw.range != status->rhw.range)
			clif_updatestatus(sd,SP_ATTACKRANGE);
		if(b_status.max_hp != status->max_hp)
			clif_updatestatus(sd,SP_MAXHP);
		if(b_status.max_sp != status->max_sp)
			clif_updatestatus(sd,SP_MAXSP);
		if(b_status.hp != status->hp)
			clif_updatestatus(sd,SP_HP);
		if(b_status.sp != status->sp)
			clif_updatestatus(sd,SP_SP);
	}
	else
	if( bl->type == BL_HOM )
	{
		TBL_HOM* hd = BL_CAST(BL_HOM, bl);
		if( hd->master && memcmp(&b_status, status, sizeof(struct status_data)) != 0 )
			clif_hominfo(hd->master,hd,0);
	}
	else
	if( bl->type == BL_MER )
	{
		TBL_MER* md = BL_CAST(BL_MER, bl);
		if( b_status.rhw.atk != status->rhw.atk || b_status.rhw.atk2 != status->rhw.atk2 )
			clif_mercenary_updatestatus(md->master, SP_ATK1);
		if( b_status.matk_max != status->matk_max )
			clif_mercenary_updatestatus(md->master, SP_MATK1);
		if( b_status.hit != status->hit )
			clif_mercenary_updatestatus(md->master, SP_HIT);
		if( b_status.cri != status->cri )
			clif_mercenary_updatestatus(md->master, SP_CRITICAL);
		if( b_status.def != status->def )
			clif_mercenary_updatestatus(md->master, SP_DEF1);
		if( b_status.mdef != status->mdef )
			clif_mercenary_updatestatus(md->master, SP_MDEF1);
		if( b_status.flee != status->flee )
			clif_mercenary_updatestatus(md->master, SP_MERCFLEE);
		if( b_status.amotion != status->amotion )
			clif_mercenary_updatestatus(md->master, SP_ASPD);
		if( b_status.max_hp != status->max_hp )
			clif_mercenary_updatestatus(md->master, SP_MAXHP);
		if( b_status.max_sp != status->max_sp )
			clif_mercenary_updatestatus(md->master, SP_MAXSP);
		if( b_status.hp != status->hp )
			clif_mercenary_updatestatus(md->master, SP_HP);
		if( b_status.sp != status->sp )
			clif_mercenary_updatestatus(md->master, SP_SP);
	}
}

/*==========================================
 * Apply shared stat mods from status changes [DracoRPG]
 *------------------------------------------*/
static unsigned short status_calc_str(struct block_list *bl, struct status_change *sc, int str)
{
	if(!sc || !sc->count)
		return cap_value(str,0,USHRT_MAX);

	if(sc->data[SC_INCALLSTATUS])
		str += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCSTR])
		str += sc->data[SC_INCSTR]->val1;
	if(sc->data[SC_STRFOOD])
		str += sc->data[SC_STRFOOD]->val1;
	if(sc->data[SC_FOOD_STR_CASH])
		str += sc->data[SC_FOOD_STR_CASH]->val1;
	if(sc->data[SC_BATTLEORDERS])
		str += 5;
	if(sc->data[SC_GUILDAURA] && sc->data[SC_GUILDAURA]->val3>>16)
		str += (sc->data[SC_GUILDAURA]->val3)>>16;
	if(sc->data[SC_LOUD])
		str += 4;
	if(sc->data[SC_TRUESIGHT])
		str += 5;
	if(sc->data[SC_SPURT])
		str += 10;
	if(sc->data[SC_NEN])
		str += sc->data[SC_NEN]->val1;
	if(sc->data[SC_BLESSING]){
		if(sc->data[SC_BLESSING]->val2)
			str += sc->data[SC_BLESSING]->val2;
		else
			str >>= 1;
	}
	if(sc->data[SC_MARIONETTE])
		str -= ((sc->data[SC_MARIONETTE]->val3)>>16)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		str += ((sc->data[SC_MARIONETTE2]->val3)>>16)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && str < 50)
		str = 50;

	return (unsigned short)cap_value(str,0,USHRT_MAX);
}

static unsigned short status_calc_agi(struct block_list *bl, struct status_change *sc, int agi)
{
	if(!sc || !sc->count)
		return cap_value(agi,0,USHRT_MAX);

	if(sc->data[SC_CONCENTRATE] && !sc->data[SC_QUAGMIRE])
		agi += (agi-sc->data[SC_CONCENTRATE]->val3)*sc->data[SC_CONCENTRATE]->val2/100;
	if(sc->data[SC_INCALLSTATUS])
		agi += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCAGI])
		agi += sc->data[SC_INCAGI]->val1;
	if(sc->data[SC_AGIFOOD])
		agi += sc->data[SC_AGIFOOD]->val1;
	if(sc->data[SC_FOOD_AGI_CASH])
		agi += sc->data[SC_FOOD_AGI_CASH]->val1;
	if(sc->data[SC_GUILDAURA] && (sc->data[SC_GUILDAURA]->val4)>>16)
		agi += (sc->data[SC_GUILDAURA]->val4)>>16;
	if(sc->data[SC_TRUESIGHT])
		agi += 5;
	if(sc->data[SC_INCREASEAGI])
		agi += sc->data[SC_INCREASEAGI]->val2;
	if(sc->data[SC_INCREASING])
		agi += 4;	// added based on skill updates [Reddozen]
	if(sc->data[SC_DECREASEAGI])
		agi -= sc->data[SC_DECREASEAGI]->val2;
	if(sc->data[SC_QUAGMIRE])
		agi -= sc->data[SC_QUAGMIRE]->val2;
	if(sc->data[SC_SUITON] && sc->data[SC_SUITON]->val3)
		agi -= sc->data[SC_SUITON]->val2;
	if(sc->data[SC_MARIONETTE])
		agi -= ((sc->data[SC_MARIONETTE]->val3)>>8)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		agi += ((sc->data[SC_MARIONETTE2]->val3)>>8)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && agi < 50)
		agi = 50;

	return (unsigned short)cap_value(agi,0,USHRT_MAX);
}

static unsigned short status_calc_vit(struct block_list *bl, struct status_change *sc, int vit)
{
	if(!sc || !sc->count)
		return cap_value(vit,0,USHRT_MAX);

	if(sc->data[SC_INCALLSTATUS])
		vit += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCVIT])
		vit += sc->data[SC_INCVIT]->val1;
	if(sc->data[SC_VITFOOD])
		vit += sc->data[SC_VITFOOD]->val1;
	if(sc->data[SC_FOOD_VIT_CASH])
		vit += sc->data[SC_FOOD_VIT_CASH]->val1;
	if(sc->data[SC_CHANGE])
		vit += sc->data[SC_CHANGE]->val2;
	if(sc->data[SC_GUILDAURA] && sc->data[SC_GUILDAURA]->val3&0xFFFF)
		vit += sc->data[SC_GUILDAURA]->val3&0xFFFF;
	if(sc->data[SC_TRUESIGHT])
		vit += 5;
	if(sc->data[SC_STRIPARMOR])
		vit -= vit * sc->data[SC_STRIPARMOR]->val2/100;
	if(sc->data[SC_MARIONETTE])
		vit -= sc->data[SC_MARIONETTE]->val3&0xFF;
	if(sc->data[SC_MARIONETTE2])
		vit += sc->data[SC_MARIONETTE2]->val3&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && vit < 50)
		vit = 50;

	return (unsigned short)cap_value(vit,0,USHRT_MAX);
}

static unsigned short status_calc_int(struct block_list *bl, struct status_change *sc, int int_)
{
	if(!sc || !sc->count)
		return cap_value(int_,0,USHRT_MAX);

	if(sc->data[SC_INCALLSTATUS])
		int_ += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCINT])
		int_ += sc->data[SC_INCINT]->val1;
	if(sc->data[SC_INTFOOD])
		int_ += sc->data[SC_INTFOOD]->val1;
	if(sc->data[SC_FOOD_INT_CASH])
		int_ += sc->data[SC_FOOD_INT_CASH]->val1;
	if(sc->data[SC_CHANGE])
		int_ += sc->data[SC_CHANGE]->val3;
	if(sc->data[SC_BATTLEORDERS])
		int_ += 5;
	if(sc->data[SC_TRUESIGHT])
		int_ += 5;
	if(sc->data[SC_BLESSING]){
		if (sc->data[SC_BLESSING]->val2)
			int_ += sc->data[SC_BLESSING]->val2;
		else
			int_ >>= 1;
	}
	if(sc->data[SC_STRIPHELM])
		int_ -= int_ * sc->data[SC_STRIPHELM]->val2/100;
	if(sc->data[SC_NEN])
		int_ += sc->data[SC_NEN]->val1;
	if(sc->data[SC_MARIONETTE])
		int_ -= ((sc->data[SC_MARIONETTE]->val4)>>16)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		int_ += ((sc->data[SC_MARIONETTE2]->val4)>>16)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && int_ < 50)
		int_ = 50;

	return (unsigned short)cap_value(int_,0,USHRT_MAX);
}

static unsigned short status_calc_dex(struct block_list *bl, struct status_change *sc, int dex)
{
	if(!sc || !sc->count)
		return cap_value(dex,0,USHRT_MAX);

	if(sc->data[SC_CONCENTRATE] && !sc->data[SC_QUAGMIRE])
		dex += (dex-sc->data[SC_CONCENTRATE]->val4)*sc->data[SC_CONCENTRATE]->val2/100;

	if(sc->data[SC_INCALLSTATUS])
		dex += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCDEX])
		dex += sc->data[SC_INCDEX]->val1;
	if(sc->data[SC_DEXFOOD])
		dex += sc->data[SC_DEXFOOD]->val1;
	if(sc->data[SC_FOOD_DEX_CASH])
		dex += sc->data[SC_FOOD_DEX_CASH]->val1;
	if(sc->data[SC_BATTLEORDERS])
		dex += 5;
	if(sc->data[SC_GUILDAURA] && sc->data[SC_GUILDAURA]->val4&0xFFFF)
		dex += sc->data[SC_GUILDAURA]->val4&0xFFFF;
	if(sc->data[SC_TRUESIGHT])
		dex += 5;
	if(sc->data[SC_QUAGMIRE])
		dex -= sc->data[SC_QUAGMIRE]->val2;
	if(sc->data[SC_BLESSING]){
		if (sc->data[SC_BLESSING]->val2)
			dex += sc->data[SC_BLESSING]->val2;
		else
			dex >>= 1;
	}
	if(sc->data[SC_INCREASING])
		dex += 4;	// added based on skill updates [Reddozen]
	if(sc->data[SC_MARIONETTE])
		dex -= ((sc->data[SC_MARIONETTE]->val4)>>8)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		dex += ((sc->data[SC_MARIONETTE2]->val4)>>8)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && dex < 50)
		dex  = 50;

	return (unsigned short)cap_value(dex,0,USHRT_MAX);
}

static unsigned short status_calc_luk(struct block_list *bl, struct status_change *sc, int luk)
{
	if(!sc || !sc->count)
		return cap_value(luk,0,USHRT_MAX);

	if(sc->data[SC_CURSE])
		return 0;
	if(sc->data[SC_INCALLSTATUS])
		luk += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCLUK])
		luk += sc->data[SC_INCLUK]->val1;
	if(sc->data[SC_LUKFOOD])
		luk += sc->data[SC_LUKFOOD]->val1;
	if(sc->data[SC_FOOD_LUK_CASH])
		luk += sc->data[SC_FOOD_LUK_CASH]->val1;
	if(sc->data[SC_TRUESIGHT])
		luk += 5;
	if(sc->data[SC_GLORIA])
		luk += 30;
	if(sc->data[SC_MARIONETTE])
		luk -= sc->data[SC_MARIONETTE]->val4&0xFF;
	if(sc->data[SC_MARIONETTE2])
		luk += sc->data[SC_MARIONETTE2]->val4&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && luk < 50)
		luk = 50;

	return (unsigned short)cap_value(luk,0,USHRT_MAX);
}

static unsigned short status_calc_batk(struct block_list *bl, struct status_change *sc, int batk)
{
	if(!sc || !sc->count)
		return cap_value(batk,0,USHRT_MAX);

	if(sc->data[SC_ATKPOTION])
		batk += sc->data[SC_ATKPOTION]->val1;
	if(sc->data[SC_BATKFOOD])
		batk += sc->data[SC_BATKFOOD]->val1;
	if(sc->data[SC_INCATKRATE])
		batk += batk * sc->data[SC_INCATKRATE]->val1/100;
	if(sc->data[SC_PROVOKE])
		batk += batk * sc->data[SC_PROVOKE]->val3/100;
	if(sc->data[SC_CONCENTRATION])
		batk += batk * sc->data[SC_CONCENTRATION]->val2/100;
	if(sc->data[SC_SKE])
		batk += batk * 3;
	if(sc->data[SC_BLOODLUST])
		batk += batk * sc->data[SC_BLOODLUST]->val2/100;
	if(sc->data[SC_JOINTBEAT] && sc->data[SC_JOINTBEAT]->val2&BREAK_WAIST)
		batk -= batk * 25/100;
	if(sc->data[SC_CURSE])
		batk -= batk * 25/100;
//Curse shouldn't effect on this?  <- Curse OR Bleeding??
//	if(sc->data[SC_BLEEDING])
//		batk -= batk * 25/100;
	if(sc->data[SC_FLEET])
		batk += batk * sc->data[SC_FLEET]->val3/100;
	if(sc->data[SC_GATLINGFEVER])
		batk += sc->data[SC_GATLINGFEVER]->val3;
	if(sc->data[SC_MADNESSCANCEL])
		batk += 100;
	return (unsigned short)cap_value(batk,0,USHRT_MAX);
}

static unsigned short status_calc_watk(struct block_list *bl, struct status_change *sc, int watk)
{
	if(!sc || !sc->count)
		return cap_value(watk,0,USHRT_MAX);

	if(sc->data[SC_IMPOSITIO])
		watk += sc->data[SC_IMPOSITIO]->val2;
	if(sc->data[SC_WATKFOOD])
		watk += sc->data[SC_WATKFOOD]->val1;
	if(sc->data[SC_DRUMBATTLE])
		watk += sc->data[SC_DRUMBATTLE]->val2;
	if(sc->data[SC_VOLCANO])
		watk += sc->data[SC_VOLCANO]->val2;
	if(sc->data[SC_INCATKRATE])
		watk += watk * sc->data[SC_INCATKRATE]->val1/100;
	if(sc->data[SC_PROVOKE])
		watk += watk * sc->data[SC_PROVOKE]->val3/100;
	if(sc->data[SC_CONCENTRATION])
		watk += watk * sc->data[SC_CONCENTRATION]->val2/100;
	if(sc->data[SC_SKE])
		watk += watk * 3;
	if(sc->data[SC_NIBELUNGEN]) {
		if (bl->type != BL_PC)
			watk += sc->data[SC_NIBELUNGEN]->val2;
		else {
			TBL_PC *sd = (TBL_PC*)bl;
			int index = sd->equip_index[sd->state.lr_flag?EQI_HAND_L:EQI_HAND_R];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				watk += sc->data[SC_NIBELUNGEN]->val2;
		}
	}
	if(sc->data[SC_BLOODLUST])
		watk += watk * sc->data[SC_BLOODLUST]->val2/100;
	if(sc->data[SC_FLEET])
		watk += watk * sc->data[SC_FLEET]->val3/100;
	if(sc->data[SC_CURSE])
		watk -= watk * 25/100;
	if(sc->data[SC_STRIPWEAPON])
		watk -= watk * sc->data[SC_STRIPWEAPON]->val2/100;
	if(sc->data[SC_MERC_ATKUP])
		watk += sc->data[SC_MERC_ATKUP]->val2;

	return (unsigned short)cap_value(watk,0,USHRT_MAX);
}

static unsigned short status_calc_matk(struct block_list *bl, struct status_change *sc, int matk)
{
	if(!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);

	if(sc->data[SC_MATKPOTION])
		matk += sc->data[SC_MATKPOTION]->val1;
	if(sc->data[SC_MATKFOOD])
		matk += sc->data[SC_MATKFOOD]->val1;
	if(sc->data[SC_MAGICPOWER])
		matk += matk * sc->data[SC_MAGICPOWER]->val3/100;
	if(sc->data[SC_MINDBREAKER])
		matk += matk * sc->data[SC_MINDBREAKER]->val2/100;
	if(sc->data[SC_INCMATKRATE])
		matk += matk * sc->data[SC_INCMATKRATE]->val1/100;

	return (unsigned short)cap_value(matk,0,USHRT_MAX);
}

static signed short status_calc_critical(struct block_list *bl, struct status_change *sc, int critical)
{
	if(!sc || !sc->count)
		return cap_value(critical,10,SHRT_MAX);

	if (sc->data[SC_INCCRI])
		critical += sc->data[SC_INCCRI]->val2;
	if (sc->data[SC_EXPLOSIONSPIRITS])
		critical += sc->data[SC_EXPLOSIONSPIRITS]->val2;
	if (sc->data[SC_FORTUNE])
		critical += sc->data[SC_FORTUNE]->val2;
	if (sc->data[SC_TRUESIGHT])
		critical += sc->data[SC_TRUESIGHT]->val2;
	if(sc->data[SC_CLOAKING])
		critical += critical;

	return (short)cap_value(critical,10,SHRT_MAX);
}

static signed short status_calc_hit(struct block_list *bl, struct status_change *sc, int hit)
{
	
	if(!sc || !sc->count)
		return cap_value(hit,1,SHRT_MAX);

	if(sc->data[SC_INCHIT])
		hit += sc->data[SC_INCHIT]->val1;
	if(sc->data[SC_HITFOOD])
		hit += sc->data[SC_HITFOOD]->val1;
	if(sc->data[SC_TRUESIGHT])
		hit += sc->data[SC_TRUESIGHT]->val3;
	if(sc->data[SC_HUMMING])
		hit += sc->data[SC_HUMMING]->val2;
	if(sc->data[SC_CONCENTRATION])
		hit += sc->data[SC_CONCENTRATION]->val3;
	if(sc->data[SC_INCHITRATE])
		hit += hit * sc->data[SC_INCHITRATE]->val1/100;
	if(sc->data[SC_BLIND])
		hit -= hit * 25/100;
	if(sc->data[SC_ADJUSTMENT])
		hit -= 30;
	if(sc->data[SC_INCREASING])
		hit += 20; // RockmanEXE; changed based on updated [Reddozen]
	if(sc->data[SC_MERC_HITUP])
		hit += sc->data[SC_MERC_HITUP]->val2;

	return (short)cap_value(hit,1,SHRT_MAX);
}

static signed short status_calc_flee(struct block_list *bl, struct status_change *sc, int flee)
{
	if( bl->type == BL_PC )
	{
		if( map_flag_gvg(bl->m) )
			flee -= flee * battle_config.gvg_flee_penalty/100;
		else if( map[bl->m].flag.battleground )
			flee -= flee * battle_config.bg_flee_penalty/100;
	}

	if(!sc || !sc->count)
		return cap_value(flee,1,SHRT_MAX);

	if(sc->data[SC_INCFLEE])
		flee += sc->data[SC_INCFLEE]->val1;
	if(sc->data[SC_FLEEFOOD])
		flee += sc->data[SC_FLEEFOOD]->val1;
	if(sc->data[SC_WHISTLE])
		flee += sc->data[SC_WHISTLE]->val2;
	if(sc->data[SC_WINDWALK])
		flee += sc->data[SC_WINDWALK]->val2;
	if(sc->data[SC_INCFLEERATE])
		flee += flee * sc->data[SC_INCFLEERATE]->val1/100;
	if(sc->data[SC_VIOLENTGALE])
		flee += sc->data[SC_VIOLENTGALE]->val2;
	if(sc->data[SC_MOON_COMFORT]) //SG skill [Komurka]
		flee += sc->data[SC_MOON_COMFORT]->val2;
	if(sc->data[SC_CLOSECONFINE])
		flee += 10;
	if(sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1)
		flee -= flee * 50/100;
	if(sc->data[SC_BERSERK])
		flee -= flee * 50/100;
	if(sc->data[SC_BLIND])
		flee -= flee * 25/100;
	if(sc->data[SC_ADJUSTMENT])
		flee += 30;
	if(sc->data[SC_GATLINGFEVER])
		flee -= sc->data[SC_GATLINGFEVER]->val4;
	if(sc->data[SC_SPEED])
		flee += 10 + sc->data[SC_SPEED]->val1 * 10;
	if(sc->data[SC_MERC_FLEEUP])
		flee += sc->data[SC_MERC_FLEEUP]->val2;

	return (short)cap_value(flee,1,SHRT_MAX);
}

static signed short status_calc_flee2(struct block_list *bl, struct status_change *sc, int flee2)
{
	if(!sc || !sc->count)
		return cap_value(flee2,10,SHRT_MAX);

	if(sc->data[SC_INCFLEE2])
		flee2 += sc->data[SC_INCFLEE2]->val2;
	if(sc->data[SC_WHISTLE])
		flee2 += sc->data[SC_WHISTLE]->val3*10;

	return (short)cap_value(flee2,10,SHRT_MAX);
}

static signed char status_calc_def(struct block_list *bl, struct status_change *sc, int def)
{
	if(!sc || !sc->count)
		return (signed char)cap_value(def,CHAR_MIN,CHAR_MAX);

	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_SKA])
		return sc->data[SC_SKA]->val3;
	if(sc->data[SC_BARRIER])
		return 100;
	if(sc->data[SC_KEEPING])
		return 90;
	if(sc->data[SC_STEELBODY])
		return 90;
	if(sc->data[SC_ARMORCHANGE])
		def += sc->data[SC_ARMORCHANGE]->val2;
	if(sc->data[SC_DRUMBATTLE])
		def += sc->data[SC_DRUMBATTLE]->val3;
	if(sc->data[SC_DEFENCE])	//[orn]
		def += sc->data[SC_DEFENCE]->val2 ;
	if(sc->data[SC_INCDEFRATE])
		def += def * sc->data[SC_INCDEFRATE]->val1/100;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		def >>=1;
	if(sc->data[SC_FREEZE])
		def >>=1;
	if(sc->data[SC_SIGNUMCRUCIS])
		def -= def * sc->data[SC_SIGNUMCRUCIS]->val2/100;
	if(sc->data[SC_CONCENTRATION])
		def -= def * sc->data[SC_CONCENTRATION]->val4/100;
	if(sc->data[SC_SKE])
		def >>=1;
	if(sc->data[SC_PROVOKE] && bl->type != BL_PC) // Provoke doesn't alter player defense->
		def -= def * sc->data[SC_PROVOKE]->val4/100;
	if(sc->data[SC_STRIPSHIELD])
		def -= def * sc->data[SC_STRIPSHIELD]->val2/100;
	if (sc->data[SC_FLING])
		def -= def * (sc->data[SC_FLING]->val2)/100;

	return (signed char)cap_value(def,CHAR_MIN,CHAR_MAX);
}

static signed short status_calc_def2(struct block_list *bl, struct status_change *sc, int def2)
{
	if(!sc || !sc->count)
		return cap_value(def2,1,SHRT_MAX);
	
	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_ETERNALCHAOS])
		return 0;
	if(sc->data[SC_SUN_COMFORT])
		def2 += sc->data[SC_SUN_COMFORT]->val2;
	if(sc->data[SC_ANGELUS])
		def2 += def2 * sc->data[SC_ANGELUS]->val2/100;
	if(sc->data[SC_CONCENTRATION])
		def2 -= def2 * sc->data[SC_CONCENTRATION]->val4/100;
	if(sc->data[SC_POISON])
		def2 -= def2 * 25/100;
	if(sc->data[SC_DPOISON])
		def2 -= def2 * 25/100;
	if(sc->data[SC_SKE])
		def2 -= def2 * 50/100;
	if(sc->data[SC_PROVOKE])
		def2 -= def2 * sc->data[SC_PROVOKE]->val4/100;
	if(sc->data[SC_JOINTBEAT])
		def2 -= def2 * ( sc->data[SC_JOINTBEAT]->val2&BREAK_SHOULDER ? 50 : 0 ) / 100
			  + def2 * ( sc->data[SC_JOINTBEAT]->val2&BREAK_WAIST ? 25 : 0 ) / 100;
	if(sc->data[SC_FLING])
		def2 -= def2 * (sc->data[SC_FLING]->val3)/100;

	return (short)cap_value(def2,1,SHRT_MAX);
}

static signed char status_calc_mdef(struct block_list *bl, struct status_change *sc, int mdef)
{
	if(!sc || !sc->count)
		return (signed char)cap_value(mdef,CHAR_MIN,CHAR_MAX);

	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_BARRIER])
		return 100;
	if(sc->data[SC_STEELBODY])
		return 90;
	if(sc->data[SC_SKA])
		return 90;
	if(sc->data[SC_ARMORCHANGE])
		mdef += sc->data[SC_ARMORCHANGE]->val3;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		mdef += 25*mdef/100;
	if(sc->data[SC_FREEZE])
		mdef += 25*mdef/100;
	if(sc->data[SC_ENDURE] && sc->data[SC_ENDURE]->val4 == 0)
		mdef += sc->data[SC_ENDURE]->val1;
	if(sc->data[SC_CONCENTRATION])
		mdef += 1; //Skill info says it adds a fixed 1 Mdef point.

	return (signed char)cap_value(mdef,CHAR_MIN,CHAR_MAX);
}

static signed short status_calc_mdef2(struct block_list *bl, struct status_change *sc, int mdef2)
{
	if(!sc || !sc->count)
		return cap_value(mdef2,1,SHRT_MAX);

	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_MINDBREAKER])
		mdef2 -= mdef2 * sc->data[SC_MINDBREAKER]->val3/100;

	return (short)cap_value(mdef2,1,SHRT_MAX);
}

static unsigned short status_calc_speed(struct block_list *bl, struct status_change *sc, int speed)
{
	TBL_PC* sd = BL_CAST(BL_PC, bl);
	int speed_rate;

	if( sc == NULL )
		return cap_value(speed,10,USHRT_MAX);

	if( sd && sd->ud.skilltimer != -1 && pc_checkskill(sd,SA_FREECAST) > 0 )
	{
		speed_rate = 175 - 5 * pc_checkskill(sd,SA_FREECAST);
	}
	else
	{
		speed_rate = 100;

		//GetMoveHasteValue2()
		{
			int val = 0;

			if( sc->data[SC_FUSION] )
				val = 25;
			else
			if( sd && pc_isriding(sd) )
				val = 25;

			speed_rate -= val;
		}

		//GetMoveSlowValue()
		{
			int val = 0;

			if( sd && sc->data[SC_HIDING] && pc_checkskill(sd,RG_TUNNELDRIVE) > 0 )
				val = 120 - 6 * pc_checkskill(sd,RG_TUNNELDRIVE);
			else
			if( sd && sc->data[SC_CHASEWALK] && sc->data[SC_CHASEWALK]->val3 < 0 )
				val = sc->data[SC_CHASEWALK]->val3;
			else
			{
				// Longing for Freedom cancels song/dance penalty
				if( sc->data[SC_LONGING] )
					val = max( val, 50 - 10 * sc->data[SC_LONGING]->val1 );
				else
				if( sd && sc->data[SC_DANCING] )
					val = max( val, 500 - (40 + 10 * (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_BARDDANCER)) * pc_checkskill(sd,(sd->status.sex?BA_MUSICALLESSON:DC_DANCINGLESSON)) );

				if( sc->data[SC_DECREASEAGI] )
					val = max( val, 25 );
				if( sc->data[SC_QUAGMIRE] )
					val = max( val, 50 );
				if( sc->data[SC_DONTFORGETME] )
					val = max( val, sc->data[SC_DONTFORGETME]->val3 );
				if( sc->data[SC_CURSE] )
					val = max( val, 300 );
				if( sc->data[SC_CHASEWALK] )
					val = max( val, sc->data[SC_CHASEWALK]->val3 );
				if( sc->data[SC_WEDDING] )
					val = max( val, 100 );
				if( sc->data[SC_JOINTBEAT] && sc->data[SC_JOINTBEAT]->val2&(BREAK_ANKLE|BREAK_KNEE) )
					val = max( val, (sc->data[SC_JOINTBEAT]->val2&BREAK_ANKLE ? 50 : 0) + (sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE ? 30 : 0) );
				if( sc->data[SC_CLOAKING] && (sc->data[SC_CLOAKING]->val4&1) == 0 )
					val = max( val, sc->data[SC_CLOAKING]->val1 < 3 ? 300 : 30 - 3 * sc->data[SC_CLOAKING]->val1 );
				if( sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY )
					val = max( val, 75 );
				if( sc->data[SC_SLOWDOWN] ) // Slow Potion
					val = max( val, 100 );
				if( sc->data[SC_GATLINGFEVER] )
					val = max( val, 100 );
				if( sc->data[SC_SUITON] )
					val = max( val, sc->data[SC_SUITON]->val3 );
				if( sc->data[SC_SWOO] )
					val = max( val, 300 );

				if( sd && sd->speed_rate + sd->speed_add_rate > 0 ) // permanent item-based speedup
					val = max( val, sd->speed_rate + sd->speed_add_rate );
			}

			speed_rate += val;
		}

		//GetMoveHasteValue1()
		{
			int val = 0;

			if( sc->data[SC_SPEEDUP1] ) //FIXME: used both by NPC_AGIUP and Speed Potion script
				val = max( val, 50 );
			if( sc->data[SC_INCREASEAGI] )
				val = max( val, 25 );
			if( sc->data[SC_WINDWALK] )
				val = max( val, 2 * sc->data[SC_WINDWALK]->val1 );
			if( sc->data[SC_CARTBOOST] )
				val = max( val, 20 );
			if( sd && (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN && pc_checkskill(sd,TF_MISS) > 0 )
				val = max( val, 1 * pc_checkskill(sd,TF_MISS) );
			if( sc->data[SC_CLOAKING] && (sc->data[SC_CLOAKING]->val4&1) == 1 )
				val = max( val, sc->data[SC_CLOAKING]->val1 >= 10 ? 25 : 3 * sc->data[SC_CLOAKING]->val1 - 3 );
			if( sc->data[SC_BERSERK] )
				val = max( val, 25 );
			if( sc->data[SC_RUN] )
				val = max( val, 55 );
			if( sc->data[SC_AVOID] )
				val = max( val, 10 * sc->data[SC_AVOID]->val1 );
			if( sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
				val = max( val, 75 );

			//FIXME: official items use a single bonus for this [ultramage]
			if( sc->data[SC_SPEEDUP0] ) // temporary item-based speedup
				val = max( val, 25 );
			if( sd && sd->speed_rate + sd->speed_add_rate < 0 ) // permanent item-based speedup
				val = max( val, -(sd->speed_rate + sd->speed_add_rate) );

			speed_rate -= val;
		}

		if( speed_rate < 40 )
			speed_rate = 40;
	}

	//GetSpeed()
	{
		if( sd && pc_iscarton(sd) )
			speed += speed * (50 - 5 * pc_checkskill(sd,MC_PUSHCART)) / 100;
		if( speed_rate != 100 )
			speed = speed * speed_rate / 100;
		if( sc->data[SC_STEELBODY] )
			speed = 200;
		if( sc->data[SC_DEFENDER] )
			speed = max(speed, 200);
		if( sc->data[SC_WALKSPEED] && sc->data[SC_WALKSPEED]->val1 > 0 ) // ChangeSpeed
			speed = speed * 100 / sc->data[SC_WALKSPEED]->val1;
	}

	return (short)cap_value(speed,10,USHRT_MAX);
}

/// Calculates an object's ASPD modifier (alters the base amotion value).
/// Note that the scale of aspd_rate is 1000 = 100%.
static short status_calc_aspd_rate(struct block_list *bl, struct status_change *sc, int aspd_rate)
{
	int i;
	if(!sc || !sc->count)
		return cap_value(aspd_rate,0,SHRT_MAX);

	if(!sc->data[SC_QUAGMIRE])
	{
		int max = 0;
		if(sc->data[SC_STAR_COMFORT])
			max = sc->data[SC_STAR_COMFORT]->val2;

		if(sc->data[SC_TWOHANDQUICKEN] &&
			max < sc->data[SC_TWOHANDQUICKEN]->val2)
			max = sc->data[SC_TWOHANDQUICKEN]->val2;

		if(sc->data[SC_ONEHAND] &&
			max < sc->data[SC_ONEHAND]->val2)
			max = sc->data[SC_ONEHAND]->val2;

		if(sc->data[SC_MERC_QUICKEN] &&
			max < sc->data[SC_MERC_QUICKEN]->val2)
			max = sc->data[SC_MERC_QUICKEN]->val2;

		if(sc->data[SC_ADRENALINE2] &&
			max < sc->data[SC_ADRENALINE2]->val3)
			max = sc->data[SC_ADRENALINE2]->val3;
		
		if(sc->data[SC_ADRENALINE] &&
			max < sc->data[SC_ADRENALINE]->val3)
			max = sc->data[SC_ADRENALINE]->val3;
		
		if(sc->data[SC_SPEARQUICKEN] &&
			max < sc->data[SC_SPEARQUICKEN]->val2)
			max = sc->data[SC_SPEARQUICKEN]->val2;

		if(sc->data[SC_GATLINGFEVER] &&
			max < sc->data[SC_GATLINGFEVER]->val2)
			max = sc->data[SC_GATLINGFEVER]->val2;
		
		if(sc->data[SC_FLEET] &&
			max < sc->data[SC_FLEET]->val2)
			max = sc->data[SC_FLEET]->val2;

		if(sc->data[SC_ASSNCROS] &&
			max < sc->data[SC_ASSNCROS]->val2)
		{
			if (bl->type!=BL_PC)
				max = sc->data[SC_ASSNCROS]->val2;
			else
			switch(((TBL_PC*)bl)->status.weapon)
			{
				case W_BOW:
				case W_REVOLVER:
				case W_RIFLE:
				case W_GATLING:
				case W_SHOTGUN:
				case W_GRENADE:
					break;
				default:
					max = sc->data[SC_ASSNCROS]->val2;
			}
		}
		aspd_rate -= max;

	  	//These stack with the rest of bonuses.
		if(sc->data[SC_BERSERK])
			aspd_rate -= 300;
		else if(sc->data[SC_MADNESSCANCEL])
			aspd_rate -= 200;
	}

	if(sc->data[i=SC_ASPDPOTION3] ||
		sc->data[i=SC_ASPDPOTION2] ||
		sc->data[i=SC_ASPDPOTION1] ||
		sc->data[i=SC_ASPDPOTION0])
		aspd_rate -= sc->data[i]->val2;
	if(sc->data[SC_DONTFORGETME])
		aspd_rate += 10 * sc->data[SC_DONTFORGETME]->val2;
	if(sc->data[SC_LONGING])
		aspd_rate += sc->data[SC_LONGING]->val2;
	if(sc->data[SC_STEELBODY])
		aspd_rate += 250;
	if(sc->data[SC_SKA])
		aspd_rate += 250;
	if(sc->data[SC_DEFENDER])
		aspd_rate += sc->data[SC_DEFENDER]->val4;
	if(sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY)
		aspd_rate += 250;
	if(sc->data[SC_GRAVITATION])
		aspd_rate += sc->data[SC_GRAVITATION]->val2;
	if(sc->data[SC_JOINTBEAT]) {
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_WRIST )
			aspd_rate += 250;
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE )
			aspd_rate += 100;
	}

	return (short)cap_value(aspd_rate,0,SHRT_MAX);
}

static unsigned short status_calc_dmotion(struct block_list *bl, struct status_change *sc, int dmotion)
{
	if( !sc || !sc->count || map_flag_gvg(bl->m) || map[bl->m].flag.battleground )
		return cap_value(dmotion,0,USHRT_MAX);
		
	if( sc->data[SC_ENDURE] )
		return 0;
	if( sc->data[SC_CONCENTRATION] )
		return 0;
	if( sc->data[SC_RUN] )
		return 0;

	return (unsigned short)cap_value(dmotion,0,USHRT_MAX);
}

static unsigned int status_calc_maxhp(struct block_list *bl, struct status_change *sc, unsigned int maxhp)
{
	if(!sc || !sc->count)
		return cap_value(maxhp,1,UINT_MAX);

	if(sc->data[SC_INCMHPRATE])
		maxhp += maxhp * sc->data[SC_INCMHPRATE]->val1/100;
	if(sc->data[SC_APPLEIDUN])
		maxhp += maxhp * sc->data[SC_APPLEIDUN]->val2/100;
	if(sc->data[SC_DELUGE])
		maxhp += maxhp * sc->data[SC_DELUGE]->val2/100;
	if(sc->data[SC_BERSERK])
		maxhp += maxhp * 2;
	if(sc->data[SC_MARIONETTE])
		maxhp -= 1000;

	if(sc->data[SC_MERC_HPUP])
		maxhp += maxhp * sc->data[SC_MERC_HPUP]->val2/100;

	return cap_value(maxhp,1,UINT_MAX);
}

static unsigned int status_calc_maxsp(struct block_list *bl, struct status_change *sc, unsigned int maxsp)
{
	if(!sc || !sc->count)
		return cap_value(maxsp,1,UINT_MAX);

	if(sc->data[SC_INCMSPRATE])
		maxsp += maxsp * sc->data[SC_INCMSPRATE]->val1/100;
	if(sc->data[SC_SERVICE4U])
		maxsp += maxsp * sc->data[SC_SERVICE4U]->val2/100;
	if(sc->data[SC_MERC_SPUP])
		maxsp += maxsp * sc->data[SC_MERC_SPUP]->val2/100;

	return cap_value(maxsp,1,UINT_MAX);
}

static unsigned char status_calc_element(struct block_list *bl, struct status_change *sc, int element)
{
	if(!sc || !sc->count)
		return element;

	if(sc->data[SC_FREEZE])
		return ELE_WATER;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return ELE_EARTH;
	if(sc->data[SC_BENEDICTIO])
		return ELE_HOLY;
	if(sc->data[SC_CHANGEUNDEAD])
		return ELE_UNDEAD;
	if(sc->data[SC_ELEMENTALCHANGE])
		return sc->data[SC_ELEMENTALCHANGE]->val2;
	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

static unsigned char status_calc_element_lv(struct block_list *bl, struct status_change *sc, int lv)
{
	if(!sc || !sc->count)
		return lv;

	if(sc->data[SC_FREEZE])	
		return 1;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return 1;
	if(sc->data[SC_BENEDICTIO])
		return 1;
	if(sc->data[SC_CHANGEUNDEAD])
		return 1;
	if(sc->data[SC_ELEMENTALCHANGE])
		return sc->data[SC_ELEMENTALCHANGE]->val1;

	return (unsigned char)cap_value(lv,1,4);
}


unsigned char status_calc_attack_element(struct block_list *bl, struct status_change *sc, int element)
{
	if(!sc || !sc->count)
		return element;
	if(sc->data[SC_ENCHANTARMS])
		return sc->data[SC_ENCHANTARMS]->val2;
	if(sc->data[SC_WATERWEAPON])
		return ELE_WATER;
	if(sc->data[SC_EARTHWEAPON])
		return ELE_EARTH;
	if(sc->data[SC_FIREWEAPON])
		return ELE_FIRE;
	if(sc->data[SC_WINDWEAPON])
		return ELE_WIND;
	if(sc->data[SC_ENCPOISON])
		return ELE_POISON;
	if(sc->data[SC_ASPERSIO])
		return ELE_HOLY;
	if(sc->data[SC_SHADOWWEAPON])
		return ELE_DARK;
	if(sc->data[SC_GHOSTWEAPON])
		return ELE_GHOST;
	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

static unsigned short status_calc_mode(struct block_list *bl, struct status_change *sc, int mode)
{
	if(!sc || !sc->count)
		return mode;
	if(sc->data[SC_MODECHANGE]) {
		if (sc->data[SC_MODECHANGE]->val2)
			mode = sc->data[SC_MODECHANGE]->val2; //Set mode
		if (sc->data[SC_MODECHANGE]->val3)
			mode|= sc->data[SC_MODECHANGE]->val3; //Add mode
		if (sc->data[SC_MODECHANGE]->val4)
			mode&=~sc->data[SC_MODECHANGE]->val4; //Del mode
	}
	return cap_value(mode,0,USHRT_MAX);
}

const char* status_get_name(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:  return ((TBL_PC *)bl)->fakename[0] != '\0' ? ((TBL_PC*)bl)->fakename : ((TBL_PC*)bl)->status.name;
	case BL_MOB: return ((TBL_MOB*)bl)->name;
	case BL_PET: return ((TBL_PET*)bl)->pet.name;
	case BL_HOM: return ((TBL_HOM*)bl)->homunculus.name;
	case BL_NPC: return ((TBL_NPC*)bl)->name;
	}
	return "Unknown";
}

/*==========================================
 * 対象のClassを返す(汎用)
 * 戻りは整数で0以上
 *------------------------------------------*/
int status_get_class(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type )
	{
	case BL_PC:  return ((TBL_PC*)bl)->status.class_;
	case BL_MOB: return ((TBL_MOB*)bl)->vd->class_; //Class used on all code should be the view class of the mob.
	case BL_PET: return ((TBL_PET*)bl)->pet.class_;
	case BL_HOM: return ((TBL_HOM*)bl)->homunculus.class_;
	case BL_MER: return ((TBL_MER*)bl)->mercenary.class_;
	case BL_NPC: return ((TBL_NPC*)bl)->class_;
	}
	return 0;
}
/*==========================================
 * 対象のレベルを返す(汎用)
 * 戻りは整数で0以上
 *------------------------------------------*/
int status_get_lv(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:  return ((TBL_PC*)bl)->status.base_level;
		case BL_MOB: return ((TBL_MOB*)bl)->level;
		case BL_PET: return ((TBL_PET*)bl)->pet.level;
		case BL_HOM: return ((TBL_HOM*)bl)->homunculus.level;
		case BL_MER: return ((TBL_MER*)bl)->db->lv;
	}
	return 1;
}

struct regen_data *status_get_regen_data(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->regen;
		case BL_HOM: return &((TBL_HOM*)bl)->regen;
		case BL_MER: return &((TBL_MER*)bl)->regen;
		default:
			return NULL;
	}
}

struct status_data *status_get_status_data(struct block_list *bl)
{
	nullpo_retr(&dummy_status, bl);

	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->battle_status;
		case BL_MOB: return &((TBL_MOB*)bl)->status;
		case BL_PET: return &((TBL_PET*)bl)->status;
		case BL_HOM: return &((TBL_HOM*)bl)->battle_status;
		case BL_MER: return &((TBL_MER*)bl)->battle_status;
		default:
			return &dummy_status;
	}
}

struct status_data *status_get_base_status(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->base_status;
		case BL_MOB: return ((TBL_MOB*)bl)->base_status ? ((TBL_MOB*)bl)->base_status : &((TBL_MOB*)bl)->db->status;
		case BL_PET: return &((TBL_PET*)bl)->db->status;
		case BL_HOM: return &((TBL_HOM*)bl)->base_status;
		case BL_MER: return &((TBL_MER*)bl)->base_status;
		default:
			return NULL;
	}
}

signed char status_get_def(struct block_list *bl)
{
	struct unit_data *ud;
	struct status_data *status = status_get_status_data(bl);
	int def = status?status->def:0;
	ud = unit_bl2ud(bl);
	if (ud && ud->skilltimer != -1)
		def -= def * skill_get_castdef(ud->skillid)/100;
	return cap_value(def, CHAR_MIN, CHAR_MAX);
}

unsigned short status_get_speed(struct block_list *bl)
{
	if(bl->type==BL_NPC)//Only BL with speed data but no status_data [Skotlex]
		return ((struct npc_data *)bl)->speed;
	return status_get_status_data(bl)->speed;
}

int status_get_party_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:
		return ((TBL_PC*)bl)->status.party_id;
	case BL_PET:
		if (((TBL_PET*)bl)->msd)
			return ((TBL_PET*)bl)->msd->status.party_id;
		break;
	case BL_MOB:
	{
		struct mob_data *md=(TBL_MOB*)bl;
		if( md->master_id>0 )
		{
			struct map_session_data *msd;
			if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
				return msd->status.party_id;
			return -md->master_id;
		}
	}
		break;
	case BL_HOM:
		if (((TBL_HOM*)bl)->master)
			return ((TBL_HOM*)bl)->master->status.party_id;
		break;
	case BL_MER:
		if (((TBL_MER*)bl)->master)
			return ((TBL_MER*)bl)->master->status.party_id;
		break;
	case BL_SKILL:
		return ((TBL_SKILL*)bl)->group->party_id;
	}
	return 0;
}

int status_get_guild_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:
		return ((TBL_PC*)bl)->status.guild_id;
	case BL_PET:
		if (((TBL_PET*)bl)->msd)
			return ((TBL_PET*)bl)->msd->status.guild_id;
		break;
	case BL_MOB:
	{
		struct map_session_data *msd;
		struct mob_data *md = (struct mob_data *)bl;
		if (md->guardian_data)	//Guardian's guild [Skotlex]
			return md->guardian_data->guild_id;
		if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
			return msd->status.guild_id; //Alchemist's mobs [Skotlex]
	}
		break;
	case BL_HOM:
	  	if (((TBL_HOM*)bl)->master)
			return ((TBL_HOM*)bl)->master->status.guild_id;
		break;
	case BL_MER:
		if (((TBL_MER*)bl)->master)
			return ((TBL_MER*)bl)->master->status.guild_id;
		break;
	case BL_NPC:
	  	if (((TBL_NPC*)bl)->subtype == SCRIPT)
			return ((TBL_NPC*)bl)->u.scr.guild_id;
		break;
	case BL_SKILL:
		return ((TBL_SKILL*)bl)->group->guild_id;
	}
	return 0;
}

int status_get_emblem_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:
		return ((TBL_PC*)bl)->guild_emblem_id;
	case BL_PET:
		if (((TBL_PET*)bl)->msd)
			return ((TBL_PET*)bl)->msd->guild_emblem_id;
		break;
	case BL_MOB:
	{
		struct map_session_data *msd;
		struct mob_data *md = (struct mob_data *)bl;
		if (md->guardian_data)	//Guardian's guild [Skotlex]
			return md->guardian_data->emblem_id;
		if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
			return msd->guild_emblem_id; //Alchemist's mobs [Skotlex]
	}
		break;
	case BL_HOM:
	  	if (((TBL_HOM*)bl)->master)
			return ((TBL_HOM*)bl)->master->guild_emblem_id;
		break;
	case BL_MER:
	  	if (((TBL_MER*)bl)->master)
			return ((TBL_MER*)bl)->master->guild_emblem_id;
		break;
	case BL_NPC:
		if (((TBL_NPC*)bl)->subtype == SCRIPT && ((TBL_NPC*)bl)->u.scr.guild_id > 0) {
			struct guild *g = guild_search(((TBL_NPC*)bl)->u.scr.guild_id);
			if (g)
				return g->emblem_id;
		}
		break;
	}
	return 0;
}

int status_get_mexp(struct block_list *bl)
{
	nullpo_ret(bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->db->mexp;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->mexp;
	return 0;
}
int status_get_race2(struct block_list *bl)
{
	nullpo_ret(bl);
	if(bl->type == BL_MOB)
		return ((struct mob_data *)bl)->db->race2;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->race2;
	return 0;
}

int status_isdead(struct block_list *bl)
{
	nullpo_ret(bl);
	return status_get_status_data(bl)->hp == 0;
}

int status_isimmune(struct block_list *bl)
{
	struct status_change *sc =status_get_sc(bl);
	if (sc && sc->data[SC_HERMODE])
		return 100;

	if (bl->type == BL_PC &&
		((TBL_PC*)bl)->special_state.no_magic_damage >= battle_config.gtb_sc_immunity)
		return ((TBL_PC*)bl)->special_state.no_magic_damage;
	return 0;
}

struct view_data* status_get_viewdata(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->vd;
		case BL_MOB: return ((TBL_MOB*)bl)->vd;
		case BL_PET: return &((TBL_PET*)bl)->vd;
		case BL_NPC: return ((TBL_NPC*)bl)->vd;
		case BL_HOM: return ((TBL_HOM*)bl)->vd;
		case BL_MER: return ((TBL_MER*)bl)->vd;
	}
	return NULL;
}

void status_set_viewdata(struct block_list *bl, int class_)
{
	struct view_data* vd;
	nullpo_retv(bl);
	if (mobdb_checkid(class_) || mob_is_clone(class_))
		vd = mob_get_viewdata(class_);
	else if (npcdb_checkid(class_) || (bl->type == BL_NPC && class_ == WARP_CLASS))
		vd = npc_get_viewdata(class_);
	else if (homdb_checkid(class_))
		vd = merc_get_hom_viewdata(class_);
	else if (merc_class(class_))
		vd = merc_get_viewdata(class_);
	else
		vd = NULL;

	switch (bl->type) {
	case BL_PC:
		{
			TBL_PC* sd = (TBL_PC*)bl;
			if (pcdb_checkid(class_)) {
				if (sd->sc.option&OPTION_WEDDING)
					class_ = JOB_WEDDING;
				else
				if (sd->sc.option&OPTION_SUMMER)
					class_ = JOB_SUMMER;
				else
				if (sd->sc.option&OPTION_XMAS)
					class_ = JOB_XMAS;
				else
				if (sd->sc.option&OPTION_RIDING)
				switch (class_)
				{	//Adapt class to a Mounted one.
				case JOB_KNIGHT:
					class_ = JOB_KNIGHT2;
					break;
				case JOB_CRUSADER:
					class_ = JOB_CRUSADER2;
					break;
				case JOB_LORD_KNIGHT:
					class_ = JOB_LORD_KNIGHT2;
					break;
				case JOB_PALADIN:
					class_ = JOB_PALADIN2;
					break;
				case JOB_BABY_KNIGHT:
					class_ = JOB_BABY_KNIGHT2;
					break;
				case JOB_BABY_CRUSADER:
					class_ = JOB_BABY_CRUSADER2;
					break;
				}
				sd->vd.class_ = class_;
				clif_get_weapon_view(sd, &sd->vd.weapon, &sd->vd.shield);
				sd->vd.head_top = sd->status.head_top;
				sd->vd.head_mid = sd->status.head_mid;
				sd->vd.head_bottom = sd->status.head_bottom;
				sd->vd.hair_style = sd->status.hair;
				sd->vd.hair_color = sd->status.hair_color;
				sd->vd.cloth_color = sd->status.clothes_color;
				sd->vd.sex = sd->status.sex;
			} else if (vd)
				memcpy(&sd->vd, vd, sizeof(struct view_data));
			else
				ShowError("status_set_viewdata (PC): No view data for class %d\n", class_);
		}
	break;
	case BL_MOB:
		{
			TBL_MOB* md = (TBL_MOB*)bl;
			if (vd)
				md->vd = vd;
			else
				ShowError("status_set_viewdata (MOB): No view data for class %d\n", class_);
		}
	break;
	case BL_PET:
		{
			TBL_PET* pd = (TBL_PET*)bl;
			if (vd) {
				memcpy(&pd->vd, vd, sizeof(struct view_data));
				if (!pcdb_checkid(vd->class_)) {
					pd->vd.hair_style = battle_config.pet_hair_style;
					if(pd->pet.equip) {
						pd->vd.head_bottom = itemdb_viewid(pd->pet.equip);
						if (!pd->vd.head_bottom)
							pd->vd.head_bottom = pd->pet.equip;
					}
				}
			} else
				ShowError("status_set_viewdata (PET): No view data for class %d\n", class_);
		}
	break;
	case BL_NPC:
		{
			TBL_NPC* nd = (TBL_NPC*)bl;
			if (vd)
				nd->vd = vd;
			else
				ShowError("status_set_viewdata (NPC): No view data for class %d\n", class_);
		}
	break;
	case BL_HOM:		//[blackhole89]
		{
			struct homun_data *hd = (struct homun_data*)bl;
			if (vd)
				hd->vd = vd;
			else
				ShowError("status_set_viewdata (HOMUNCULUS): No view data for class %d\n", class_);
		}
		break;
	case BL_MER:
		{
			struct mercenary_data *md = (struct mercenary_data*)bl;
			if (vd)
				md->vd = vd;
			else
				ShowError("status_set_viewdata (MERCENARY): No view data for class %d\n", class_);
		}
		break;
	}
	vd = status_get_viewdata(bl);
	if (vd && vd->cloth_color && (
		(vd->class_==JOB_WEDDING && battle_config.wedding_ignorepalette)
		|| (vd->class_==JOB_XMAS && battle_config.xmas_ignorepalette)
		|| (vd->class_==JOB_SUMMER && battle_config.summer_ignorepalette)
	))
		vd->cloth_color = 0;
}

/// Returns the status_change data of bl or NULL if it doesn't exist.
struct status_change *status_get_sc(struct block_list *bl)
{
	if( bl )
	switch (bl->type) {
	case BL_PC:  return &((TBL_PC*)bl)->sc;
	case BL_MOB: return &((TBL_MOB*)bl)->sc;
	case BL_NPC: return &((TBL_NPC*)bl)->sc;
	case BL_HOM: return &((TBL_HOM*)bl)->sc;
	case BL_MER: return &((TBL_MER*)bl)->sc;
	}
	return NULL;
}

void status_change_init(struct block_list *bl)
{
	struct status_change *sc = status_get_sc(bl);
	nullpo_retv(sc);
	memset(sc, 0, sizeof (struct status_change));
}

//Applies SC defense to a given status change.
//Returns the adjusted duration based on flag values.
//the flag values are the same as in status_change_start.
int status_get_sc_def(struct block_list *bl, enum sc_type type, int rate, int tick, int flag)
{
	int sc_def, tick_def = 0;
	struct status_data* status;
	struct status_change* sc;
	struct map_session_data *sd;

	nullpo_ret(bl);

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
	case SC_SUITON:
		return 0;
	}
	
	sd = BL_CAST(BL_PC,bl);
	status = status_get_status_data(bl);
	switch (type)
	{
	case SC_STUN:
	case SC_POISON:
	case SC_DPOISON:
	case SC_SILENCE:
	case SC_BLEEDING:
		sc_def = 3 +status->vit;
		break;
	case SC_SLEEP:
		sc_def = 3 +status->int_;
		break;
	case SC_DECREASEAGI:
		if (sd) tick>>=1; //Half duration for players.
	case SC_STONE:
	case SC_FREEZE:
		sc_def = 3 +status->mdef;
		break;
	case SC_CURSE:
		//Special property: inmunity when luk is greater than level or zero
		if (status->luk > status_get_lv(bl) || status->luk == 0)
			return 0;
		else
			sc_def = 3 +status->luk;
		tick_def = status->vit;
		break;
	case SC_BLIND:
		sc_def = 3 +(status->vit + status->int_)/2;
		break;
	case SC_CONFUSION:
		sc_def = 3 +(status->str + status->int_)/2;
		break;
	case SC_ANKLE:
		if(status->mode&MD_BOSS) // Lasts 5 times less on bosses
			tick /= 5;
		sc_def = status->agi / 2;
		break;
	case SC_MAGICMIRROR:
	case SC_ARMORCHANGE:
		if (sd) //Duration greatly reduced for players.
			tick /= 15;
		//No defense against it (buff).
	default:
		//Effect that cannot be reduced? Likely a buff.
		if (!(rand()%10000 < rate))
			return 0;
		return tick?tick:1;
	}
	
	if (sd) {

		if (battle_config.pc_sc_def_rate != 100)
			sc_def = sc_def*battle_config.pc_sc_def_rate/100;

		if (sc_def < battle_config.pc_max_sc_def)
			sc_def += (battle_config.pc_max_sc_def - sc_def)*
				status->luk/battle_config.pc_luk_sc_def;
		else
			sc_def = battle_config.pc_max_sc_def;

		if (tick_def) {
			if (battle_config.pc_sc_def_rate != 100)
				tick_def = tick_def*battle_config.pc_sc_def_rate/100;
		}

	} else {

		if (battle_config.mob_sc_def_rate != 100)
			sc_def = sc_def*battle_config.mob_sc_def_rate/100;

		if (sc_def < battle_config.mob_max_sc_def)
			sc_def += (battle_config.mob_max_sc_def - sc_def)*
				status->luk/battle_config.mob_luk_sc_def;
		else
			sc_def = battle_config.mob_max_sc_def;

		if (tick_def) {
			if (battle_config.mob_sc_def_rate != 100)
				tick_def = tick_def*battle_config.mob_sc_def_rate/100;
		}
	}
	
	sc = status_get_sc(bl);
	if (sc && sc->count)
	{
		if (sc->data[SC_SCRESIST])
			sc_def += sc->data[SC_SCRESIST]->val1; //Status resist
		else if (sc->data[SC_SIEGFRIED])
			sc_def += sc->data[SC_SIEGFRIED]->val3; //Status resistance.
	}

	//When no tick def, reduction is the same for both.
	if( !tick_def && type != SC_STONE ) //Recent tests show duration of petrify isn't reduced by MDEF. [Inkfish]
		tick_def = sc_def;

	//Natural resistance
	if (!(flag&8)) {
		rate -= rate*sc_def/100;

		//Item resistance (only applies to rate%)
		if(sd && SC_COMMON_MIN <= type && type <= SC_COMMON_MAX)
		{
			if( sd->reseff[type-SC_COMMON_MIN] > 0 )
				rate -= rate*sd->reseff[type-SC_COMMON_MIN]/10000;
			if( sd->sc.data[SC_COMMONSC_RESIST] )
				rate -= rate*sd->sc.data[SC_COMMONSC_RESIST]->val1/100;
		}
	}
	if (!(rand()%10000 < rate))
		return 0;

	//Why would a status start with no duration? Presume it has 
	//duration defined elsewhere.
	if (!tick) return 1;

	//Rate reduction
 	if (flag&2)
		return tick;

	tick -= tick*tick_def/100;
	// Changed to 5 seconds according to recent tests [Playtester]
	if (type == SC_ANKLE && tick < 5000)
		tick = 5000;
	return tick<=0?0:tick;
}

/*==========================================
 * Starts a status change.
 * 'type' = type, 'val1~4' depend on the type.
 * 'rate' = base success rate. 10000 = 100%
 * 'tick' is base duration
 * 'flag':
 * &1: Cannot be avoided (it has to start)
 * &2: Tick should not be reduced (by vit, luk, lv, etc)
 * &4: sc_data loaded, no value has to be altered.
 * &8: rate should not be reduced
 *------------------------------------------*/
int status_change_start(struct block_list* bl,enum sc_type type,int rate,int val1,int val2,int val3,int val4,int tick,int flag)
{
	struct map_session_data *sd = NULL;
	struct status_change* sc;
	struct status_change_entry* sce;
	struct status_data *status;
	struct view_data *vd;
	int opt_flag, calc_flag, undead_flag;

	nullpo_ret(bl);
	sc = status_get_sc(bl);
	status = status_get_status_data(bl);

	if( type <= SC_NONE || type >= SC_MAX )
	{
		ShowError("status_change_start: invalid status change (%d)!\n", type);
		return 0;
	}

	if( !sc )
		return 0; //Unable to receive status changes
	
	if( status_isdead(bl) )
		return 0;

	if( bl->type == BL_MOB && type != SC_SAFETYWALL && type != SC_PNEUMA )
	{
		struct mob_data *md = BL_CAST(BL_MOB,bl);
		if( md->class_ == MOBID_EMPERIUM || mob_is_battleground(md) )
			return 0; //Emperium/BG Monsters can't be afflicted by status changes
	}

	sd = BL_CAST(BL_PC, bl);

	//Adjust tick according to status resistances
	if( !(flag&(1|4)) )
	{
		tick = status_get_sc_def(bl, type, rate, tick, flag);
		if( !tick ) return 0;
	}

	undead_flag = battle_check_undead(status->race,status->def_ele);
	//Check for inmunities / sc fails
	switch (type)
	{
	case SC_FREEZE:
	case SC_STONE:
		//Undead are immune to Freeze/Stone
		if (undead_flag && !(flag&1))
			return 0;
	case SC_SLEEP:
	case SC_STUN:
		if (sc->opt1)
			return 0; //Cannot override other opt1 status changes. [Skotlex]
	break;
	case SC_SIGNUMCRUCIS:
		//Only affects demons and undead element (but not players)
		if((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC)
			return 0;
	break;
	case SC_AETERNA:
		if( (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE) || sc->data[SC_FREEZE] )
			return 0;
	break;
	case SC_KYRIE:
		if (bl->type == BL_MOB)
			return 0;
	break;
	case SC_OVERTHRUST:
		if (sc->data[SC_MAXOVERTHRUST])
			return 0; //Overthrust can't take effect if under Max Overthrust. [Skotlex]
	break;
	case SC_ADRENALINE:
		if(sd && !pc_check_weapontype(sd,skill_get_weapontype(BS_ADRENALINE)))
			return 0;
		if (sc->data[SC_QUAGMIRE] ||
			sc->data[SC_DECREASEAGI]
		)
			return 0;
	break;
	case SC_ADRENALINE2:
		if(sd && !pc_check_weapontype(sd,skill_get_weapontype(BS_ADRENALINE2)))
			return 0;
		if (sc->data[SC_QUAGMIRE] ||
			sc->data[SC_DECREASEAGI]
		)
			return 0;
	break;
	case SC_ONEHAND:
	case SC_MERC_QUICKEN:
	case SC_TWOHANDQUICKEN:
		if(sc->data[SC_DECREASEAGI])
			return 0;
	case SC_CONCENTRATE:
	case SC_INCREASEAGI:
	case SC_SPEARQUICKEN:
	case SC_TRUESIGHT:
	case SC_WINDWALK:
	case SC_CARTBOOST:
	case SC_ASSNCROS:
		if (sc->data[SC_QUAGMIRE])
			return 0;
	break;
	case SC_CLOAKING:
		//Avoid cloaking with no wall and low skill level. [Skotlex]
		//Due to the cloaking card, we have to check the wall versus to known
		//skill level rather than the used one. [Skotlex]
		//if (sd && val1 < 3 && skill_check_cloaking(bl,NULL))
		if( sd && pc_checkskill(sd, AS_CLOAKING) < 3 && !skill_check_cloaking(bl,NULL) )
			return 0;
	break;
	case SC_MODECHANGE:
	{
		int mode;
		struct status_data *bstatus = status_get_base_status(bl);
		if (!bstatus) return 0;
		if (sc->data[type])
		{	//Pile up with previous values.
			if(!val2) val2 = sc->data[type]->val2;
			val3 |= sc->data[type]->val3;
			val4 |= sc->data[type]->val4;
		}
		mode = val2?val2:bstatus->mode; //Base mode
		if (val4) mode&=~val4; //Del mode
		if (val3) mode|= val3; //Add mode
		if (mode == bstatus->mode) { //No change.
			if (sc->data[type]) //Abort previous status
				return status_change_end(bl, type, -1);
			return 0;
		}
	}
	break;
	//Strip skills, need to divest something or it fails.
	case SC_STRIPWEAPON:
		if (sd) {
			int i;
			opt_flag = 0; //Reuse to check success condition.
			if(sd->unstripable_equip&EQP_WEAPON)
				return 0;
			i = sd->equip_index[EQI_HAND_L];
			if (i>=0 && sd->inventory_data[i] &&
				sd->inventory_data[i]->type == IT_WEAPON)
			{
				opt_flag|=1;
				pc_unequipitem(sd,i,3); //L-hand weapon
			}

			i = sd->equip_index[EQI_HAND_R];
			if (i>=0 && sd->inventory_data[i] &&
				sd->inventory_data[i]->type == IT_WEAPON)
			{
				opt_flag|=2;
				pc_unequipitem(sd,i,3);
			}
			if (!opt_flag) return 0;
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_STRIPSHIELD:
		if( val2 == 1 ) val2 = 0; //GX effect. Do not take shield off..		
		else
		if (sd) {
			int i;
			if(sd->unstripable_equip&EQP_SHIELD)
				return 0;
			i = sd->equip_index[EQI_HAND_L];
			if (i<0 || !sd->inventory_data[i] ||
				sd->inventory_data[i]->type != IT_ARMOR)
				return 0;
			pc_unequipitem(sd,i,3);
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_STRIPARMOR:
		if (sd) {
			int i;
			if(sd->unstripable_equip&EQP_ARMOR)
				return 0;
			i = sd->equip_index[EQI_ARMOR];
			if (i<0 || !sd->inventory_data[i])
				return 0;
			pc_unequipitem(sd,i,3);
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_STRIPHELM:
		if (sd) {
			int i;
			if(sd->unstripable_equip&EQP_HELM)
				return 0;
			i = sd->equip_index[EQI_HEAD_TOP];
			if (i<0 || !sd->inventory_data[i])
				return 0;
			pc_unequipitem(sd,i,3);
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_MERC_FLEEUP:
	case SC_MERC_ATKUP:
	case SC_MERC_HPUP:
	case SC_MERC_SPUP:
	case SC_MERC_HITUP:
		if( bl->type != BL_MER )
			return 0; // Stats only for Mercenaries
	break;
	case SC_STRFOOD:
		if (sc->data[SC_FOOD_STR_CASH] && sc->data[SC_FOOD_STR_CASH]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_AGIFOOD:
		if (sc->data[SC_FOOD_AGI_CASH] && sc->data[SC_FOOD_AGI_CASH]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_VITFOOD:
		if (sc->data[SC_FOOD_VIT_CASH] && sc->data[SC_FOOD_VIT_CASH]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_INTFOOD:
		if (sc->data[SC_FOOD_INT_CASH] && sc->data[SC_FOOD_INT_CASH]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_DEXFOOD:
		if (sc->data[SC_FOOD_DEX_CASH] && sc->data[SC_FOOD_DEX_CASH]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_LUKFOOD:
		if (sc->data[SC_FOOD_LUK_CASH] && sc->data[SC_FOOD_LUK_CASH]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_FOOD_STR_CASH:
		if (sc->data[SC_STRFOOD] && sc->data[SC_STRFOOD]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_FOOD_AGI_CASH:
		if (sc->data[SC_AGIFOOD] && sc->data[SC_AGIFOOD]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_FOOD_VIT_CASH:
		if (sc->data[SC_VITFOOD] && sc->data[SC_VITFOOD]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_FOOD_INT_CASH:
		if (sc->data[SC_INTFOOD] && sc->data[SC_INTFOOD]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_FOOD_DEX_CASH:
		if (sc->data[SC_DEXFOOD] && sc->data[SC_DEXFOOD]->val1 > sc->data[type]->val1)
			return 0;
	break;
	case SC_FOOD_LUK_CASH:
		if (sc->data[SC_LUKFOOD] && sc->data[SC_LUKFOOD]->val1 > sc->data[type]->val1)
			return 0;
	break;
	}

	//Check for BOSS resistances
	if(status->mode&MD_BOSS && !(flag&1)) {
		 if (type>=SC_COMMON_MIN && type <= SC_COMMON_MAX)
			 return 0;
		 switch (type) {
			case SC_BLESSING:
			  if (!undead_flag && status->race!=RC_DEMON)
				  break;
			case SC_DECREASEAGI:
			case SC_PROVOKE:
			case SC_COMA:
			case SC_GRAVITATION:
			case SC_SUITON:
			case SC_RICHMANKIM:
			case SC_ROKISWEIL:
			case SC_FOGWALL:
				return 0;
		}
	}

	//Before overlapping fail, one must check for status cured.
	switch (type)
	{
	case SC_BLESSING:
		//TO-DO Blessing and Agi up should do 1 damage against players on Undead Status, even on PvM
		//but cannot be plagiarized (this requires aegis investigation on packets and official behavior) [Brainstorm]
		if ((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC) {
			if (sc->data[SC_CURSE])
				status_change_end(bl,SC_CURSE,-1);
			if (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
				status_change_end(bl,SC_STONE,-1);
		}
		break;
	case SC_INCREASEAGI:
		status_change_end(bl,SC_DECREASEAGI,-1);
		break;
	case SC_QUAGMIRE:
		status_change_end(bl,SC_CONCENTRATE,-1);
		status_change_end(bl,SC_TRUESIGHT,-1);
		status_change_end(bl,SC_WINDWALK,-1);
		//Also blocks the ones below...
	case SC_DECREASEAGI:
		status_change_end(bl,SC_CARTBOOST,-1);
		//Also blocks the ones below...
	case SC_DONTFORGETME:
		status_change_end(bl,SC_INCREASEAGI,-1);
		status_change_end(bl,SC_ADRENALINE,-1);
		status_change_end(bl,SC_ADRENALINE2,-1);
		status_change_end(bl,SC_SPEARQUICKEN,-1);
		status_change_end(bl,SC_TWOHANDQUICKEN,-1);
		status_change_end(bl,SC_ONEHAND,-1);
		status_change_end(bl,SC_MERC_QUICKEN,-1);
		break;
	case SC_ONEHAND:
	  	//Removes the Aspd potion effect, as reported by Vicious. [Skotlex]
		status_change_end(bl,SC_ASPDPOTION0,-1);
		status_change_end(bl,SC_ASPDPOTION1,-1);
		status_change_end(bl,SC_ASPDPOTION2,-1);
		status_change_end(bl,SC_ASPDPOTION3,-1);
		break;
	case SC_MAXOVERTHRUST:
	  	//Cancels Normal Overthrust. [Skotlex]
		status_change_end(bl, SC_OVERTHRUST, -1);
		break;
	case SC_KYRIE:
		//Cancels Assumptio
		status_change_end(bl,SC_ASSUMPTIO,-1);
		break;
	case SC_DELUGE:
		if (sc->data[SC_FOGWALL] && sc->data[SC_BLIND])
			status_change_end(bl,SC_BLIND,-1);
		break;
	case SC_SILENCE:
		if (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF)
			status_change_end(bl,SC_GOSPEL,-1);
		break;
	case SC_HIDING:
		status_change_end(bl, SC_CLOSECONFINE, -1);
		status_change_end(bl, SC_CLOSECONFINE2, -1);
		break;
	case SC_BERSERK:
		if(battle_config.berserk_cancels_buffs)
		{
			status_change_end(bl,SC_ONEHAND,-1);
			status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			status_change_end(bl,SC_CONCENTRATION,-1);
			status_change_end(bl,SC_PARRYING,-1);
			status_change_end(bl,SC_AURABLADE,-1);
			status_change_end(bl,SC_MERC_QUICKEN,-1);
		}
		break;
	case SC_ASSUMPTIO:
		status_change_end(bl,SC_KYRIE,-1);
		status_change_end(bl,SC_KAITE,-1);
		break;
	case SC_KAITE:
		status_change_end(bl,SC_ASSUMPTIO,-1);
		break;
	case SC_CARTBOOST:
		if(sc->data[SC_DECREASEAGI])
		{	//Cancel Decrease Agi, but take no further effect [Skotlex]
			status_change_end(bl,SC_DECREASEAGI,-1);
			return 0;
		}
		break;
	case SC_FUSION:
		status_change_end(bl,SC_SPIRIT,-1);
		break;
	case SC_ADJUSTMENT:
		status_change_end(bl,SC_MADNESSCANCEL,-1);
		break;
	case SC_MADNESSCANCEL:
		status_change_end(bl,SC_ADJUSTMENT,-1);
		break;
	//NPC_CHANGEUNDEAD will debuff Blessing and Agi Up
	case SC_CHANGEUNDEAD:
		status_change_end(bl,SC_BLESSING,-1);
		status_change_end(bl,SC_INCREASEAGI,-1);
		break;
	case SC_STRFOOD:
		if (sc->data[SC_FOOD_STR_CASH] && sc->data[SC_FOOD_STR_CASH]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_FOOD_STR_CASH,-1);
		break;
	case SC_AGIFOOD:
		if (sc->data[SC_FOOD_AGI_CASH] && sc->data[SC_FOOD_AGI_CASH]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_FOOD_AGI_CASH,-1);
		break;
	case SC_VITFOOD:
		if (sc->data[SC_FOOD_VIT_CASH] && sc->data[SC_FOOD_VIT_CASH]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_FOOD_VIT_CASH,-1);
		break;
	case SC_INTFOOD:
		if (sc->data[SC_FOOD_INT_CASH] && sc->data[SC_FOOD_INT_CASH]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_FOOD_INT_CASH,-1);
		break;
	case SC_DEXFOOD:
		if (sc->data[SC_FOOD_DEX_CASH] && sc->data[SC_FOOD_DEX_CASH]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_FOOD_DEX_CASH,-1);
		break;
	case SC_LUKFOOD:
		if (sc->data[SC_FOOD_LUK_CASH] && sc->data[SC_FOOD_LUK_CASH]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_FOOD_LUK_CASH,-1);
		break;
	case SC_FOOD_STR_CASH:
		if (sc->data[SC_STRFOOD] && sc->data[SC_STRFOOD]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_STRFOOD,-1);
		break;
	case SC_FOOD_AGI_CASH:
		if (sc->data[SC_AGIFOOD] && sc->data[SC_AGIFOOD]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_AGIFOOD,-1);
		break;
	case SC_FOOD_VIT_CASH:
		if (sc->data[SC_VITFOOD] && sc->data[SC_VITFOOD]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_VITFOOD,-1);
		break;
	case SC_FOOD_INT_CASH:
		if (sc->data[SC_INTFOOD] && sc->data[SC_INTFOOD]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_INTFOOD,-1);
		break;
	case SC_FOOD_DEX_CASH:
		if (sc->data[SC_DEXFOOD] && sc->data[SC_DEXFOOD]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_DEXFOOD,-1);
		break;
	case SC_FOOD_LUK_CASH:
		if (sc->data[SC_LUKFOOD] && sc->data[SC_LUKFOOD]->val1 <= sc->data[type]->val1)
			status_change_end(bl,SC_LUKFOOD,-1);
		break;
	}

	//Check for overlapping fails
	if( (sce = sc->data[type]) )
	{
		switch( type )
		{
			case SC_MERC_FLEEUP:
			case SC_MERC_ATKUP:
			case SC_MERC_HPUP:
			case SC_MERC_SPUP:
			case SC_MERC_HITUP:
				if( sce->val1 > val1 )
					val1 = sce->val1;
				break;
			case SC_ADRENALINE:
			case SC_ADRENALINE2:
			case SC_WEAPONPERFECTION:
			case SC_OVERTHRUST:
				if (sce->val2 > val2)
					return 0;
				break;
			case SC_S_LIFEPOTION:
			case SC_L_LIFEPOTION:
			case SC_BOSSMAPINFO:
			case SC_STUN:
			case SC_SLEEP:
			case SC_POISON:
			case SC_CURSE:
			case SC_SILENCE:
			case SC_CONFUSION:
			case SC_BLIND:
			case SC_BLEEDING:
			case SC_DPOISON:
			case SC_CLOSECONFINE2: //Can't be re-closed in.
			case SC_MARIONETTE:
			case SC_MARIONETTE2:
			case SC_NOCHAT:
			case SC_CHANGE: //Otherwise your Hp/Sp would get refilled while still within effect of the last invocation.
				return 0;
			case SC_COMBO: 
			case SC_DANCING:
			case SC_DEVOTION:
			case SC_ASPDPOTION0:
			case SC_ASPDPOTION1:
			case SC_ASPDPOTION2:
			case SC_ASPDPOTION3:
			case SC_ATKPOTION:
			case SC_MATKPOTION:
			case SC_ENCHANTARMS:
			case SC_ARMOR_ELEMENT:
			case SC_ARMOR_RESIST:
				break;
			case SC_GOSPEL:
				 //Must not override a casting gospel char.
				if(sce->val4 == BCT_SELF)
					return 0;
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_ENDURE:
				if(sce->val4 && !val4)
					return 1; //Don't let you override infinite endure.
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_KAAHI:
				//Kaahi overwrites previous level regardless of existing level.
				//Delete timer if it exists.
				if (sce->val4 != -1) {
					delete_timer(sce->val4,kaahi_heal_timer);
					sce->val4=-1;
				}
				break;
			case SC_JAILED:
				//When a player is already jailed, do not edit the jail data.
				val2 = sce->val2;
				val3 = sce->val3;
				val4 = sce->val4;
				break;
			case SC_JOINTBEAT:
				val2 |= sce->val2; // stackable ailments
			default:
				if(sce->val1 > val1)
					return 1; //Return true to not mess up skill animations. [Skotlex]
		}
	}

	vd = status_get_viewdata(bl);
	calc_flag = StatusChangeFlagTable[type];
	if(!(flag&4)) //&4 - Do not parse val settings when loading SCs
	switch(type)
	{
		case SC_DECREASEAGI:
		case SC_INCREASEAGI:
			val2 = 2 + val1; //Agi change
			break;
		case SC_ENDURE:
			val2 = 7; // Hit-count [Celest]
			if( !(flag&1) && (bl->type&(BL_PC|BL_MER)) && !map_flag_gvg(bl->m) && !map[bl->m].flag.battleground && !val4 )
			{
				struct map_session_data *tsd;
				if( sd )
				{
					int i;
					for( i = 0; i < 5; i++ )
					{
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
							status_change_start(&tsd->bl, type, 10000, val1, val2, val3, val4, tick, 1);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
					status_change_start(&tsd->bl, type, 10000, val1, val2, val3, val4, tick, 1);
			}
			//val4 signals infinite endure (if val4 == 2 it is infinite endure from Berserk)
			if( val4 )
				tick = -1;
			break;
		case SC_AUTOBERSERK:
			if (status->hp < status->max_hp>>2 &&
				(!sc->data[SC_PROVOKE] || sc->data[SC_PROVOKE]->val2==0))
					sc_start4(bl,SC_PROVOKE,100,10,1,0,0,60000);
			tick = -1;
			break;
		case SC_SIGNUMCRUCIS:
			val2 = 10 + 4*val1; //Def reduction
			tick = -1;
			clif_emotion(bl,4);
			break;
		case SC_MAXIMIZEPOWER:
			val2 = tick>0?tick:60000;
			break;
		case SC_EDP:	// [Celest]
			val2 = val1 + 2; //Chance to Poison enemies.
			val3 = 50*(val1+1); //Damage increase (+50 +50*lv%)
			break;
		case SC_POISONREACT:
			val2=(val1+1)/2 + val1/10; // Number of counters [Skotlex]
			val3=50; // + 5*val1; //Chance to counter. [Skotlex]
			break;
		case SC_MAGICROD:
			val2 = val1*20; //SP gained
			break;
		case SC_KYRIE:
			val2 = status->max_hp * (val1 * 2 + 10) / 100; //%Max HP to absorb
			val3 = (val1 / 2 + 5); //Hits
			break;
		case SC_MAGICPOWER:
			//val1: Skill lv
			val2 = 1; //Lasts 1 invocation
			val3 = 5*val1; //Matk% increase
			break;
		case SC_SACRIFICE:
			val2 = 5; //Lasts 5 hits
			tick = -1;
			break;
		case SC_ENCPOISON:
			val2= 250+50*val1;	//Poisoning Chance (2.5+0.5%) in 1/10000 rate
		case SC_ASPERSIO:
		case SC_FIREWEAPON:
		case SC_WATERWEAPON:
		case SC_WINDWEAPON:
		case SC_EARTHWEAPON:
		case SC_SHADOWWEAPON:
		case SC_GHOSTWEAPON:
			skill_enchant_elemental_end(bl,type);
			break;
		case SC_ELEMENTALCHANGE:
			// val1 : Element Lvl (if called by skill lvl 1, takes random value between 1 and 4)
			// val2 : Element (When no element, random one is picked)
			// val3 : 0 = called by skill 1 = called by script (fixed level)
			if( !val2 ) val2 = rand()%ELE_MAX;

			if( val1 == 1 && val3 == 0 )
				val1 = 1 + rand()%4;
			else if( val1 > 4 )
				val1 = 4; // Max Level
			val3 = 0; // Not need to keep this info.
			break;
		case SC_PROVIDENCE:
			val2=val1*5; //Race/Ele resist
			break;
		case SC_REFLECTSHIELD:
			val2=10+val1*3; // %Dmg reflected
			if( !(flag&1) && (bl->type&(BL_PC|BL_MER)) )
			{
				struct map_session_data *tsd;
				if( sd )
				{
					int i;
					for( i = 0; i < 5; i++ )
					{
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
							status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
					status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
			}
			break;
		case SC_STRIPWEAPON:
			if (!sd) //Watk reduction
				val2 = 25;
			break;
		case SC_STRIPSHIELD:
			if (!sd) //Def reduction
				val2 = 15;
			break;
		case SC_STRIPARMOR:
			if (!sd) //Vit reduction
				val2 = 40;
			break;
		case SC_STRIPHELM:
			if (!sd) //Int reduction
				val2 = 40;
			break;
		case SC_AUTOSPELL:
			//Val1 Skill LV of Autospell
			//Val2 Skill ID to cast
			//Val3 Max Lv to cast
			val4 = 5 + val1*2; //Chance of casting
			break;
		case SC_VOLCANO:
			if (status->def_ele == ELE_FIRE)
				val2 = val1*10; //Watk increase
			else
				val2 = 0;
			break;
		case SC_VIOLENTGALE:
			if (status->def_ele == ELE_WIND)
				val2 = val1*3; //Flee increase
			else
				val2 = 0;
			break;
		case SC_DELUGE:
			if(status->def_ele == ELE_WATER)
				val2 = deluge_eff[val1-1]; //HP increase
			else
				val2 = 0;
			break;
		case SC_SUITON:
			if (!val2 || (sd && (sd->class_&MAPID_UPPERMASK) == MAPID_NINJA)) {
				//No penalties.
				val2 = 0; //Agi penalty
				val3 = 0; //Walk speed penalty
				break;
			}
			val3 = 50;
			val2 = 3*((val1+1)/3);
			if (val1 > 4) val2--;
			break;
		case SC_ONEHAND:
		case SC_TWOHANDQUICKEN:
			val2 = 300;
			if (val1 > 10) //For boss casted skills [Skotlex]
				val2 += 20*(val1-10);
			break;
		case SC_MERC_QUICKEN:
			val2 = 300;
			break;

		case SC_SPEARQUICKEN:
			val2 = 200+10*val1;
			break;
		case SC_DANCING:
			//val1 : Skill ID + LV
			//val2 : Skill Group of the Dance.
			//val3 : Brings the skilllv (merged into val1 here)
			//val4 : Partner
			if (val1 == CG_MOONLIT)
				clif_status_change(bl,SI_MOONLIT,1,tick);
			val1|= (val3<<16);
			val3 = tick/1000; //Tick duration
			tick = 1000;
			break;
		case SC_LONGING:
			val2 = 500-100*val1; //Aspd penalty.
			break;
		case SC_EXPLOSIONSPIRITS:
			val2 = 75 + 25*val1; //Cri bonus
			break;
		case SC_ASPDPOTION0:
		case SC_ASPDPOTION1:
		case SC_ASPDPOTION2:
		case SC_ASPDPOTION3:
			val2 = 50*(2+type-SC_ASPDPOTION0);
			break;

		case SC_WEDDING:
		case SC_XMAS:
		case SC_SUMMER:
			if (!vd) return 0;
			//Store previous values as they could be removed.
			val1 = vd->class_;
			val2 = vd->weapon;
			val3 = vd->shield;
			val4 = vd->cloth_color;
			unit_stop_attack(bl);
			clif_changelook(bl,LOOK_WEAPON,0);
			clif_changelook(bl,LOOK_SHIELD,0);
			clif_changelook(bl,LOOK_BASE,type==SC_WEDDING?JOB_WEDDING:type==SC_XMAS?JOB_XMAS:JOB_SUMMER);
			clif_changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
			break;
		case SC_NOCHAT:
			tick = 60000;
			val1 = battle_config.manner_system; //Mute filters.
			if (sd)
			{
				clif_changestatus(&sd->bl,SP_MANNER,sd->status.manner);
				clif_updatestatus(sd,SP_MANNER);
			}
			break;

		case SC_STONE:
			val3 = tick/1000; //Petrified HP-damage iterations.
			if(val3 < 1) val3 = 1; 
			tick = val4; //Petrifying time.
			if (tick < 1000)
				tick = 1000; //Min time
			calc_flag = 0; //Actual status changes take effect on petrified state.
			break;

		case SC_DPOISON:
		//Lose 10/15% of your life as long as it doesn't brings life below 25%
		if (status->hp > status->max_hp>>2)
		{
			int diff = status->max_hp*(bl->type==BL_PC?10:15)/100;
			if (status->hp - diff < status->max_hp>>2)
				diff = status->hp - (status->max_hp>>2);
			status_zap(bl, diff, 0);
		}
		// fall through
		case SC_POISON:				/* 毒 */
		val3 = tick/1000; //Damage iterations
		if(val3 < 1) val3 = 1;
		tick = 1000;
		//val4: HP damage
		if (bl->type == BL_PC)
			val4 = (type == SC_DPOISON) ? 3 + status->max_hp/50 : 3 + status->max_hp*3/200;
		else
			val4 = (type == SC_DPOISON) ? 3 + status->max_hp/100 : 3 + status->max_hp/200;
		
		break;
		case SC_CONFUSION:
			clif_emotion(bl,1);
			break;
		case SC_BLEEDING:
			val4 = tick/10000;
			if (!val4) val4 = 1;
			tick = 10000;
			break;
		case SC_S_LIFEPOTION:
		case SC_L_LIFEPOTION:
			if( val1 == 0 ) return 0;
			// val1 = heal percent/amout
			// val2 = seconds between heals
			// val4 = total of heals
			if( val2 < 1 ) val2 = 1;
			if( (val4 = tick/(val2 * 1000)) < 1 )
				val4 = 1;
			tick = val2 * 1000; // val2 = Seconds between heals
			break;
		case SC_BOSSMAPINFO:
			if( sd != NULL )
			{
				struct mob_data *boss_md = map_getmob_boss(bl->m); // Search for Boss on this Map
				if( boss_md == NULL || boss_md->bl.prev == NULL )
				{ // No MVP on this map - MVP is dead
					clif_bossmapinfo(sd->fd, boss_md, 1);
					return 0; // No need to start SC
				}
				val1 = boss_md->bl.id;
				if( (val4 = tick/1000) < 1 )
					val4 = 1;
				tick = 1000;
			}
			break;
		case SC_HIDING:
			val2 = tick/1000;
			tick = 1000;
			val3 = 0; // unused, previously speed adjustment
			val4 = val1+3; //Seconds before SP substraction happen.
			break;
		case SC_CHASEWALK:
			val2 = tick>0?tick:10000; //Interval at which SP is drained.
			val3 = 35 - 5 * val1; //Speed adjustment.
			if (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_ROGUE)
				val3 -= 40;
			val4 = 10+val1*2; //SP cost.
			if (map_flag_gvg(bl->m) || map[bl->m].flag.battleground) val4 *= 5;
			break;
		case SC_CLOAKING:
			if (!sd) //Monsters should be able to walk with no penalties. [Skotlex]
				val1 = 10;
			val2 = tick>0?tick:60000; //SP consumption rate.
			val3 = 0; // unused, previously walk speed adjustment
			//val4&1 signals the presence of a wall.
			//val4&2 makes cloak not end on normal attacks [Skotlex]
			//val4&4 makes cloak not end on using skills
			if (bl->type == BL_PC)	//Standard cloaking.
				val4 |= battle_config.pc_cloak_check_type&7;
			else
				val4 |= battle_config.monster_cloak_check_type&7;
			break;
		case SC_SIGHT:			/* サイト/ルアフ */
		case SC_RUWACH:
		case SC_SIGHTBLASTER:
			val3 = skill_get_splash(val2, val1); //Val2 should bring the skill-id.
			val2 = tick/250;
			tick = 10;
			break;

		//Permanent effects.
		case SC_AETERNA:
		case SC_MODECHANGE:
		case SC_WEIGHT50:
		case SC_WEIGHT90:
		case SC_BROKENWEAPON:
		case SC_BROKENARMOR:
		case SC_READYSTORM:
		case SC_READYDOWN:
		case SC_READYCOUNTER:
		case SC_READYTURN:
		case SC_DODGE:
			tick = -1;
			break;

		case SC_AUTOGUARD:
			if( !(flag&1) )
			{
				struct map_session_data *tsd;
				int i,t;
				for( i = val2 = 0; i < val1; i++)
				{
					t = 5-(i>>1);
					val2 += (t < 0)? 1:t;
				}

				if( bl->type&(BL_PC|BL_MER) )
				{
					if( sd )
					{
						for( i = 0; i < 5; i++ )
						{
							if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
								status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
						}
					}
					else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
						status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
				}
			}
			break;

		case SC_DEFENDER:
			if (!(flag&1))
			{	
				struct map_session_data *tsd;
				int i;
				val2 = 5 + 15*val1; //Damage reduction
				val3 = 0; // unused, previously speed adjustment
				val4 = 250 - 50*val1; //Aspd adjustment 

				if (sd)
				for (i = 0; i < 5; i++)
				{	//See if there are devoted characters, and pass the status to them. [Skotlex]
					if (sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])))
						status_change_start(&tsd->bl,type,10000,val1,5+val1*5,val3,val4,tick,1);
				}
			}
			break;

		case SC_TENSIONRELAX:
			if (sd) {
				pc_setsit(sd);
				clif_sitting(&sd->bl);
			}
			val2 = 12; //SP cost
			val4 = 10000; //Decrease at 10secs intervals.
			val3 = tick/val4;
			tick = val4;
			break;
		case SC_PARRYING:
		    val2 = 20 + val1*3; //Block Chance
			break;

		case SC_WINDWALK:
			val2 = (val1+1)/2; // Flee bonus is 1/1/2/2/3/3/4/4/5/5
			break;

		case SC_JOINTBEAT:
			if( val2&BREAK_NECK )
				sc_start(bl,SC_BLEEDING,100,val1,skill_get_time2(status_sc2skill(type),val1));
			break;

		case SC_BERSERK:
			if (!sc->data[SC_ENDURE] || !sc->data[SC_ENDURE]->val4)
				sc_start4(bl, SC_ENDURE, 100,10,0,0,2, tick);
			//HP healing is performing after the calc_status call.
			//Val2 holds HP penalty
			if (!val4) val4 = skill_get_time2(status_sc2skill(type),val1);
			if (!val4) val4 = 10000; //Val4 holds damage interval
			val3 = tick/val4; //val3 holds skill duration
			tick = val4;
			break;

		case SC_GOSPEL:
			if(val4 == BCT_SELF) {	// self effect
				val2 = tick/10000;
				tick = 10000;
				status_change_clear_buffs(bl,3); //Remove buffs/debuffs
			}
			break;

		case SC_MARIONETTE:
		{
			int stat;

			val3 = 0;
			val4 = 0;
			stat = ( sd ? sd->status.str : status_get_base_status(bl)->str ) / 2; val3 |= cap_value(stat,0,0xFF)<<16;
			stat = ( sd ? sd->status.agi : status_get_base_status(bl)->agi ) / 2; val3 |= cap_value(stat,0,0xFF)<<8;
			stat = ( sd ? sd->status.vit : status_get_base_status(bl)->vit ) / 2; val3 |= cap_value(stat,0,0xFF);
			stat = ( sd ? sd->status.int_: status_get_base_status(bl)->int_) / 2; val4 |= cap_value(stat,0,0xFF)<<16;
			stat = ( sd ? sd->status.dex : status_get_base_status(bl)->dex ) / 2; val4 |= cap_value(stat,0,0xFF)<<8;
			stat = ( sd ? sd->status.luk : status_get_base_status(bl)->luk ) / 2; val4 |= cap_value(stat,0,0xFF);
			break;
		}
		case SC_MARIONETTE2:
		{
			int stat,max_stat;
			// fetch caster information
			struct block_list *pbl = map_id2bl(val1);
			struct status_change *psc = pbl?status_get_sc(pbl):NULL;
			struct status_change_entry *psce = psc?psc->data[SC_MARIONETTE]:NULL;
			// fetch target's stats
			struct status_data* status = status_get_status_data(bl); // battle status

			if (!psce)
				return 0;

			val3 = 0;
			val4 = 0;
			max_stat = battle_config.max_parameter; //Cap to 99 (default)
			stat = (psce->val3 >>16)&0xFF; stat = min(stat, max_stat - status->str ); val3 |= cap_value(stat,0,0xFF)<<16;
			stat = (psce->val3 >> 8)&0xFF; stat = min(stat, max_stat - status->agi ); val3 |= cap_value(stat,0,0xFF)<<8;
			stat = (psce->val3 >> 0)&0xFF; stat = min(stat, max_stat - status->vit ); val3 |= cap_value(stat,0,0xFF);
			stat = (psce->val4 >>16)&0xFF; stat = min(stat, max_stat - status->int_); val4 |= cap_value(stat,0,0xFF)<<16;
			stat = (psce->val4 >> 8)&0xFF; stat = min(stat, max_stat - status->dex ); val4 |= cap_value(stat,0,0xFF)<<8;
			stat = (psce->val4 >> 0)&0xFF; stat = min(stat, max_stat - status->luk ); val4 |= cap_value(stat,0,0xFF);
			break;
		}
		case SC_REJECTSWORD:
			val2 = 15*val1; //Reflect chance
			val3 = 3; //Reflections
			tick = -1;
			break;

		case SC_MEMORIZE:
			val2 = 5; //Memorized casts.
			tick = -1;
			break;

		case SC_GRAVITATION:
			val2 = 50*val1; //aspd reduction
			break;

		case SC_REGENERATION:
			if (val1 == 1)
				val2 = 2;
			else
				val2 = val1; //HP Regerenation rate: 200% 200% 300%
			val3 = val1; //SP Regeneration Rate: 100% 200% 300%
			//if val4 comes set, this blocks regen rather than increase it.
			break;

		case SC_DEVOTION:
		{
			struct block_list *d_bl;
			struct status_change *d_sc;

			if( (d_bl = map_id2bl(val1)) && (d_sc = status_get_sc(d_bl)) && d_sc->count )
			{ // Inherits Status From Source
				const enum sc_type types[] = { SC_AUTOGUARD, SC_DEFENDER, SC_REFLECTSHIELD, SC_ENDURE };
				enum sc_type type2;
				int i = (map_flag_gvg(bl->m) || map[bl->m].flag.battleground)?2:3;
				while( i >= 0 )
				{
					type2 = types[i];
					if( d_sc->data[type2] )
						sc_start(bl, type2, 100, d_sc->data[type2]->val1, skill_get_time(status_sc2skill(type2),d_sc->data[type2]->val1));
					i--;
				}
			}
			break;
		}

		case SC_COMA: //Coma. Sends a char to 1HP. If val2, do not zap sp
			status_zap(bl, status->hp-1, val2?0:status->sp);
			return 1;
			break;
		case SC_CLOSECONFINE2:
		{
			struct block_list *src = val2?map_id2bl(val2):NULL;
			struct status_change *sc2 = src?status_get_sc(src):NULL;
			struct status_change_entry *sce2 = sc2?sc2->data[SC_CLOSECONFINE]:NULL;
			if (src && sc2) {
				if (!sce2) //Start lock on caster.
					sc_start4(src,SC_CLOSECONFINE,100,val1,1,0,0,tick+1000);
				else { //Increase count of locked enemies and refresh time.
					(sce2->val2)++;
					delete_timer(sce2->timer, status_change_timer);
					sce2->timer = add_timer(gettick()+tick+1000, status_change_timer, src->id, SC_CLOSECONFINE);
				}
			} else //Status failed.
				return 0;
		}
			break;
		case SC_KAITE:
			val2 = 1+val1/5; //Number of bounces: 1 + skilllv/5
			break;
		case SC_KAUPE:
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
		{
			//val1: Skill ID
			//val2: When given, target (for autotargetting skills)
			//val3: When set, this combo time should NOT delay attack/movement
			//val4: Combo time
			struct unit_data *ud = unit_bl2ud(bl);
			switch (val1) {
				case TK_STORMKICK:
					clif_skill_nodamage(bl,bl,TK_READYSTORM,1,1);
					break;
				case TK_DOWNKICK:
					clif_skill_nodamage(bl,bl,TK_READYDOWN,1,1);
					break;
				case TK_TURNKICK:
					clif_skill_nodamage(bl,bl,TK_READYTURN,1,1);
					break;
				case TK_COUNTER:
					clif_skill_nodamage(bl,bl,TK_READYCOUNTER,1,1);
					break;
				case MO_COMBOFINISH:
				case CH_TIGERFIST:
				case CH_CHAINCRUSH:
					if( sd )
					{
						sd->state.combo = 1;
						clif_skillinfoblock(sd);
					}
					break;
				case TK_JUMPKICK:
					if( sd )
					{
						sd->state.combo = 2;
						clif_skillinfoblock(sd);
					}
					break;		
			}
			if (ud && !val3) 
			{
				ud->attackabletime = gettick()+tick;
				unit_set_walkdelay(bl, gettick(), tick, 1);
			}
			val4 = tick; //Store combo-time in val4.
		}
			break;
		case SC_EARTHSCROLL:
			val2 = 11-val1; //Chance to consume: 11-skilllv%
			break;
		case SC_RUN:
			val4 = gettick(); //Store time at which you started running.
			tick = -1;
			break;
		case SC_KAAHI:
			val2 = 200*val1; //HP heal
			val3 = 5*val1; //SP cost 
			val4 = -1;	//Kaahi Timer.
			break;
		case SC_BLESSING:
			if ((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC)
				val2 = val1;
			else
				val2 = 0; //0 -> Half stat.
			break;
		case SC_TRICKDEAD:
			if (vd) vd->dead_sit = 1;
			tick = -1;
			break;
		case SC_CONCENTRATE:
			val2 = 2 + val1;
			if (sd) { //Store the card-bonus data that should not count in the %
				val3 = sd->param_bonus[1]; //Agi
				val4 = sd->param_bonus[4]; //Dex
			} else {
				val3 = val4 = 0;
			}
			break;
		case SC_MAXOVERTHRUST:
			val2 = 20*val1; //Power increase
			break;
		case SC_OVERTHRUST:
			//val2 holds if it was casted on self, or is bonus received from others
			val3 = 5*val1; //Power increase
			if(sd && pc_checkskill(sd,BS_HILTBINDING)>0)
				tick += tick / 10;
			break;
		case SC_ADRENALINE2:
		case SC_ADRENALINE:
			val3 = (val2) ? 300 : 200; // aspd increase
		case SC_WEAPONPERFECTION:
			if(sd && pc_checkskill(sd,BS_HILTBINDING)>0)
				tick += tick / 10;
			break;
		case SC_CONCENTRATION:
			val2 = 5*val1; //Batk/Watk Increase
			val3 = 10*val1; //Hit Increase
			val4 = 5*val1; //Def reduction
			break;
		case SC_ANGELUS:
			val2 = 5*val1; //def increase
			break;
		case SC_IMPOSITIO:
			val2 = 5*val1; //watk increase
			break;
		case SC_MELTDOWN:
			val2 = 100*val1; //Chance to break weapon
			val3 = 70*val1; //Change to break armor
			break;
		case SC_TRUESIGHT:
			val2 = 10*val1; //Critical increase
			val3 = 3*val1; //Hit increase
			break;
		case SC_SUN_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk)/2; //def increase
			break;
		case SC_MOON_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk)/10; //flee increase
			break;
		case SC_STAR_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk); //Aspd increase
			break;
		case SC_QUAGMIRE:
			val2 = (sd?5:10)*val1; //Agi/Dex decrease.
			break;

		// gs_something1 [Vicious]
		case SC_GATLINGFEVER:
			val2 = 20*val1; //Aspd increase
			val3 = 20+10*val1; //Batk increase
			val4 = 5*val1; //Flee decrease
			break;

		case SC_FLING:
			if (bl->type == BL_PC)
				val2 = 0; //No armor reduction to players.
			else
				val2 = 5*val1; //Def reduction
			val3 = 5*val1; //Def2 reduction
			break;
		case SC_PROVOKE:
			//val2 signals autoprovoke.
			val3 = 2+3*val1; //Atk increase
			val4 = 5+5*val1; //Def reduction.
			break;
		case SC_AVOID:
			//val2 = 10*val1; //Speed change rate.
			break;
		case SC_DEFENCE:
			val2 = 2*val1; //Def bonus
			break;
		case SC_BLOODLUST:
			val2 = 20+10*val1; //Atk rate change.
			val3 = 3*val1; //Leech chance
			val4 = 20; //Leech percent
			break;
		case SC_FLEET:
			val2 = 30*val1; //Aspd change
			val3 = 5+5*val1; //bAtk/wAtk rate change
			break;
		case SC_MINDBREAKER:
			val2 = 20*val1; //matk increase.
			val3 = 12*val1; //mdef2 reduction.
			break;
		case SC_SKA:  
			val2 = tick/1000;  
			val3 = rand()%100; //Def changes randomly every second...  
			tick = 1000;  
			break;  
		case SC_JAILED:
			//Val1 is duration in minutes. Use INT_MAX to specify 'unlimited' time.
			tick = val1>0?1000:250;
			if (sd)
			{
				if (sd->mapindex != val2)
				{
					int pos =  (bl->x&0xFFFF)|(bl->y<<16), //Current Coordinates
					map =  sd->mapindex; //Current Map
					//1. Place in Jail (val2 -> Jail Map, val3 -> x, val4 -> y
					pc_setpos(sd,(unsigned short)val2,val3,val4, 3);
					//2. Set restore point (val3 -> return map, val4 return coords
					val3 = map;
					val4 = pos;
				} else if (!val3 || val3 == sd->mapindex) { //Use save point.
					val3 = sd->status.save_point.map;
					val4 = (sd->status.save_point.x&0xFFFF)
						|(sd->status.save_point.y<<16);
				}
			}
			break;
		case SC_UTSUSEMI:
			val2=(val1+1)/2; // number of hits blocked
			val3=skill_get_blewcount(NJ_UTSUSEMI, val1); //knockback value.
			break;
		case SC_BUNSINJYUTSU:
			val2=(val1+1)/2; // number of hits blocked
			break;
		case SC_CHANGE:
			val2= 30*val1; //Vit increase
			val3= 20*val1; //Int increase
			break;
		case SC_SWOO:
			if(status->mode&MD_BOSS)
				tick /= 5; //TODO: Reduce skill's duration. But for how long?
			break;
		case SC_SPIDERWEB:
			if( bl->type == BL_PC )
				tick /= 2;
			break;
		case SC_ARMOR:
			//NPC_DEFENDER:
			val2 = 80; //Damage reduction
			//Attack requirements to be blocked:
			val3 = BF_LONG; //Range
			val4 = BF_WEAPON|BF_MISC; //Type
			break;
		case SC_ENCHANTARMS:
			//end previous enchants
			skill_enchant_elemental_end(bl,type);
			//Make sure the received element is valid.
			if (val2 >= ELE_MAX)
				val2 = val2%ELE_MAX;
			else if (val2 < 0)
				val2 = rand()%ELE_MAX;
			break;
		case SC_CRITICALWOUND:
			val2 = 20*val1; //Heal effectiveness decrease
			break;
		case SC_MAGICMIRROR:
		case SC_SLOWCAST:
			val2 = 20*val1; //Magic reflection/cast rate
			break;

		case SC_ARMORCHANGE:
			if (val2 == NPC_ANTIMAGIC)
			{	//Boost mdef
				val2 =-20;
				val3 = 20;
			} else { //Boost def
				val2 = 20;
				val3 =-20;
			}
			val2*=val1; //20% per level
			val3*=val1;
			break;
		case SC_EXPBOOST:
		case SC_JEXPBOOST:
			if (val1 < 0)
				val1 = 0;
			break;
		case SC_INCFLEE2:
		case SC_INCCRI:
			val2 = val1*10; //Actual boost (since 100% = 1000)
			break;
		case SC_SUFFRAGIUM:
			val2 = 15 * val1; //Speed cast decrease
			break;
		case SC_INCHEALRATE:
			if (val1 < 1)
				val1 = 1;
			break;
		case SC_HALLUCINATION:
			val2 = 5+val1; //Factor by which displayed damage is increased by
			break;
		case SC_DOUBLECAST:
			val2 = 30+10*val1; //Trigger rate
			break;
		case SC_KAIZEL:
			val2 = 10*val1; //% of life to be revived with
			break;
		// case SC_ARMOR_ELEMENT:
		// case SC_ARMOR_RESIST:
			// Mod your resistance against elements:
			// val1 = water | val2 = earth | val3 = fire | val4 = wind
			// break;
		//case ????:
			//Place here SCs that have no SCB_* data, no skill associated, no ICON
			//associated, and yet are not wrong/unknown. [Skotlex]
			//break;

		case SC_MERC_FLEEUP:
		case SC_MERC_ATKUP:
		case SC_MERC_HITUP:
			val2 = 15 * val1;
			break;
		case SC_MERC_HPUP:
		case SC_MERC_SPUP:
			val2 = 5 * val1;
			break;
		case SC_REBIRTH:
			val2 = 20*val1; //% of life to be revived with
			break;

		case SC_MANU_DEF:
		case SC_MANU_ATK:
		case SC_MANU_MATK:
			val2 = 1; // Manuk group
			break;
		case SC_SPL_DEF:
		case SC_SPL_ATK:
		case SC_SPL_MATK:
			val2 = 2; // Splendide group
			break;

		default:
			if( calc_flag == SCB_NONE && StatusSkillChangeTable[type] == 0 && StatusIconChangeTable[type] == 0 )
			{	//Status change with no calc, no icon, and no skill associated...? 
				ShowError("UnknownStatusChange [%d]\n", type);
				return 0;
			}
	}
	else //Special considerations when loading SC data.
	switch( type )
	{
		case SC_WEDDING:
		case SC_XMAS:
		case SC_SUMMER:
			clif_changelook(bl,LOOK_WEAPON,0);
			clif_changelook(bl,LOOK_SHIELD,0);
			clif_changelook(bl,LOOK_BASE,type==SC_WEDDING?JOB_WEDDING:type==SC_XMAS?JOB_XMAS:JOB_SUMMER);
			clif_changelook(bl,LOOK_CLOTHES_COLOR,val4);
			break;	
		case SC_KAAHI:
			val4 = -1;
			break;
	}

	//Those that make you stop attacking/walking....
	switch (type)
	{
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
		case SC_STONE:
			if (sd && pc_issit(sd)) //Avoid sprite sync problems.
				pc_setstand(sd);
		case SC_TRICKDEAD:
			unit_stop_attack(bl);
			status_change_end(bl, SC_DANCING, -1);
			// Cancel cast when get status [LuzZza]
			if (battle_config.sc_castcancel&bl->type)
				unit_skillcastcancel(bl, 0);
		case SC_STOP:
		case SC_CONFUSION:
		case SC_CLOSECONFINE:
		case SC_CLOSECONFINE2:
		case SC_ANKLE:
		case SC_SPIDERWEB:
			unit_stop_walking(bl,1);
		break;
		case SC_HIDING:
		case SC_CLOAKING:
		case SC_CHASEWALK:
		case SC_WEIGHT90:
			unit_stop_attack(bl);
		break;
		case SC_SILENCE:
			if (battle_config.sc_castcancel&bl->type)
				unit_skillcastcancel(bl, 0);
		break;
	}

	// Set option as needed.
	opt_flag = 1;
	switch(type)
	{
		//OPT1
		case SC_STONE:  sc->opt1 = OPT1_STONEWAIT; break;
		case SC_FREEZE: sc->opt1 = OPT1_FREEZE;    break;
		case SC_STUN:   sc->opt1 = OPT1_STUN;      break;
		case SC_SLEEP:  sc->opt1 = OPT1_SLEEP;     break;
		//OPT2
		case SC_POISON:       sc->opt2 |= OPT2_POISON;       break;
		case SC_CURSE:        sc->opt2 |= OPT2_CURSE;        break;
		case SC_SILENCE:      sc->opt2 |= OPT2_SILENCE;      break;
		case SC_SIGNUMCRUCIS: sc->opt2 |= OPT2_SIGNUMCRUCIS; break;
		case SC_BLIND:        sc->opt2 |= OPT2_BLIND;        break;
		case SC_ANGELUS:      sc->opt2 |= OPT2_ANGELUS;      break;
		case SC_BLEEDING:     sc->opt2 |= OPT2_BLEEDING;     break;
		case SC_DPOISON:      sc->opt2 |= OPT2_DPOISON;      break;
		//OPT3
		case SC_TWOHANDQUICKEN:
		case SC_ONEHAND:
		case SC_SPEARQUICKEN:
		case SC_CONCENTRATION:
		case SC_MERC_QUICKEN:
			sc->opt3 |= OPT3_QUICKEN;
			opt_flag = 0;
			break;
		case SC_MAXOVERTHRUST:
		case SC_OVERTHRUST:
		case SC_SWOO:	//Why does it shares the same opt as Overthrust? Perhaps we'll never know...
			sc->opt3 |= OPT3_OVERTHRUST;
			opt_flag = 0;
			break;
		case SC_ENERGYCOAT:
		case SC_SKE:
			sc->opt3 |= OPT3_ENERGYCOAT;
			opt_flag = 0;
			break;
		case SC_INCATKRATE:
			//Simulate Explosion Spirits effect for NPC_POWERUP [Skotlex]
			if (bl->type != BL_MOB) {
				opt_flag = 0;
				break;
			}
		case SC_EXPLOSIONSPIRITS:
			sc->opt3 |= OPT3_EXPLOSIONSPIRITS;
			opt_flag = 0;
			break;
		case SC_STEELBODY:
		case SC_SKA:
			sc->opt3 |= OPT3_STEELBODY;
			opt_flag = 0;
			break;
		case SC_BLADESTOP:
			sc->opt3 |= OPT3_BLADESTOP;
			opt_flag = 0;
			break;
		case SC_AURABLADE:
			sc->opt3 |= OPT3_AURABLADE;
			opt_flag = 0;
			break;
		case SC_BERSERK:
			sc->opt3 |= OPT3_BERSERK;
			opt_flag = 0;
			break;
//		case ???: // doesn't seem to do anything
//			sc->opt3 |= OPT3_LIGHTBLADE;
//			opt_flag = 0;
//			break;
		case SC_DANCING:
			if ((val1&0xFFFF) == CG_MOONLIT)
				sc->opt3 |= OPT3_MOONLIT;
			opt_flag = 0;
			break;
		case SC_MARIONETTE:
		case SC_MARIONETTE2:
			sc->opt3 |= OPT3_MARIONETTE;
			opt_flag = 0;
			break;
		case SC_ASSUMPTIO:
			sc->opt3 |= OPT3_ASSUMPTIO;
			opt_flag = 0;
			break;
		case SC_WARM: //SG skills [Komurka]
			sc->opt3 |= OPT3_WARM;
			opt_flag = 0;
			break;
		case SC_KAITE:
			sc->opt3 |= OPT3_KAITE;
			opt_flag = 0;
			break;
		case SC_BUNSINJYUTSU:
			sc->opt3 |= OPT3_BUNSIN;
			opt_flag = 0;
			break;
		case SC_SPIRIT:
			sc->opt3 |= OPT3_SOULLINK;
			opt_flag = 0;
			break;
		case SC_CHANGEUNDEAD:
			sc->opt3 |= OPT3_UNDEAD;
			opt_flag = 0;
			break;
//		case ???: // from DA_CONTRACT (looks like biolab mobs aura)
//			sc->opt3 |= OPT3_CONTRACT;
//			opt_flag = 0;
//			break;
		//OPTION
		case SC_HIDING:
			sc->option |= OPTION_HIDE;
			opt_flag = 2;
			break;
		case SC_CLOAKING:
			sc->option |= OPTION_CLOAK;
			opt_flag = 2;
			break;
		case SC_CHASEWALK:
			sc->option |= OPTION_CHASEWALK|OPTION_CLOAK;
			opt_flag = 2;
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
		case SC_XMAS:
			sc->option |= OPTION_XMAS;
			break;
		case SC_SUMMER:
			sc->option |= OPTION_SUMMER;
			break;
		case SC_ORCISH:
			sc->option |= OPTION_ORCISH;
			break;
		case SC_FUSION:
			sc->option |= OPTION_FLYING;
			break;
		default:
			opt_flag = 0;
	}

	//On Aegis, when turning on a status change, first goes the option packet, then the sc packet.
	if(opt_flag)
		clif_changeoption(bl);

	if (calc_flag&SCB_DYE)
	{	//Reset DYE color
		if (vd && vd->cloth_color)
		{
			val4 = vd->cloth_color;
			clif_changelook(bl,LOOK_CLOTHES_COLOR,0);
		}
		calc_flag&=~SCB_DYE;
	}

	if( vd && (pcdb_checkid(vd->class_) || bl->type == BL_MER ) ) //Only for players sprites, client crashes if they receive this for a mob o.O [Skotlex]
		clif_status_change(bl,StatusIconChangeTable[type],1,tick);
	else if( sd ) //Send packet to self otherwise (disguised player?)
		clif_status_load(bl,StatusIconChangeTable[type],1);

	//Don't trust the previous sce assignment, in case the SC ended somewhere between there and here.
	if((sce=sc->data[type]))
	{// reuse old sc
		if( sce->timer != INVALID_TIMER )
			delete_timer(sce->timer, status_change_timer);
	}
	else
	{// new sc
		++(sc->count);
		sce = sc->data[type] = ers_alloc(sc_data_ers, struct status_change_entry);
	}
	sce->val1 = val1;
	sce->val2 = val2;
	sce->val3 = val3;
	sce->val4 = val4;
	if (tick >= 0)
		sce->timer = add_timer(gettick() + tick, status_change_timer, bl->id, type);
	else
		sce->timer = INVALID_TIMER; //Infinite duration

	if (calc_flag)
		status_calc_bl(bl,calc_flag);
	
	if(sd && sd->pd)
		pet_sc_check(sd, type); //Skotlex: Pet Status Effect Healing

	switch( type )
	{
		case SC_BERSERK:
			sce->val2 = 5*status->max_hp/100;
			status_heal(bl, status->max_hp, 0, 1); //Do not use percent_heal as this healing must override BERSERK's block.
			status_set_sp(bl, 0, 0); //Damage all SP
			break;
		case SC_CHANGE:
			status_percent_heal(bl, 100, 100);
			break;
		case SC_RUN:
			{
				struct unit_data *ud = unit_bl2ud(bl);
				if( ud )
					ud->state.running = unit_run(bl);
			}
			break;
		case SC_BOSSMAPINFO:
			clif_bossmapinfo(sd->fd, map_id2boss(sce->val1), 0); // First Message
			break;
		case SC_MERC_HPUP:
			status_percent_heal(bl, 100, 0); // Recover Full HP
			break;
		case SC_MERC_SPUP:
			status_percent_heal(bl, 0, 100); // Recover Full SP
			break;
	}

	if( opt_flag&2 && sd && sd->touching_id )
		npc_touchnext_areanpc(sd,false); // run OnTouch_ on next char in range

	return 1;
}
/*==========================================
 * ステータス異常全解除
 * type:
 * 0 - ???
 * 1 - ???
 * 2 - ???
 *------------------------------------------*/
int status_change_clear(struct block_list* bl, int type)
{
	struct status_change* sc;
	int i;

	sc = status_get_sc(bl);

	if (!sc || !sc->count)
		return 0;

	for(i = 0; i < SC_MAX; i++)
	{
		if(!sc->data[i])
		  continue;

		if(type == 0)
		switch (i)
		{	//Type 0: PC killed -> Place here statuses that do not dispel on death.
		case SC_WEIGHT50:
		case SC_WEIGHT90:
		case SC_EDP:
		case SC_MELTDOWN:
		case SC_XMAS:
		case SC_SUMMER:
		case SC_NOCHAT:
		case SC_FUSION:
		case SC_EARTHSCROLL:
		case SC_READYSTORM:
		case SC_READYDOWN:
		case SC_READYCOUNTER:
		case SC_READYTURN:
		case SC_DODGE:
		case SC_JAILED:
		case SC_EXPBOOST:
		case SC_ITEMBOOST:
		case SC_HELLPOWER:
		case SC_JEXPBOOST:
		case SC_AUTOTRADE:
		case SC_FOOD_STR_CASH:
		case SC_FOOD_AGI_CASH:
		case SC_FOOD_VIT_CASH:
		case SC_FOOD_DEX_CASH:
		case SC_FOOD_INT_CASH:
		case SC_FOOD_LUK_CASH:
			continue;
		}

		status_change_end(bl, (sc_type)i, INVALID_TIMER);

		if( type == 1 && sc->data[i] )
		{	//If for some reason status_change_end decides to still keep the status when quitting. [Skotlex]
			(sc->count)--;
			if (sc->data[i]->timer != INVALID_TIMER)
				delete_timer(sc->data[i]->timer, status_change_timer);
			ers_free(sc_data_ers, sc->data[i]);
			sc->data[i] = NULL;
		}
	}

	sc->opt1 = 0;
	sc->opt2 = 0;
	sc->opt3 = 0;
	sc->option &= OPTION_MASK;

	if( type == 0 || type == 2 )
		clif_changeoption(bl);

	return 1;
}

/*==========================================
 * ステータス異常終了
 *------------------------------------------*/
int status_change_end(struct block_list* bl, enum sc_type type, int tid)
{
	struct map_session_data *sd;
	struct status_change *sc;
	struct status_change_entry *sce;
	struct status_data *status;
	struct view_data *vd;
	int opt_flag=0, calc_flag;

	nullpo_ret(bl);
	
	sc = status_get_sc(bl);
	status = status_get_status_data(bl);

	if(type < 0 || type >= SC_MAX || !sc || !(sce = sc->data[type]))
		return 0;

	sd = BL_CAST(BL_PC,bl);

	if (sce->timer != tid && tid != -1)
		return 0;

	if (tid == -1) {
		if (type == SC_ENDURE && sce->val4)
			//Do not end infinite endure.
			return 0;
		if (sce->timer != -1) //Could be a SC with infinite duration
			delete_timer(sce->timer,status_change_timer);
		if (sc->opt1)
		switch (type) {
			//"Ugly workaround"  [Skotlex]
			//delays status change ending so that a skill that sets opt1 fails to 
			//trigger when it also removed one
			case SC_STONE:
				sce->val3 = 0; //Petrify time counter.
			case SC_FREEZE:
			case SC_STUN:
			case SC_SLEEP:
			if (sce->val1) {
			  	//Removing the 'level' shouldn't affect anything in the code
				//since these SC are not affected by it, and it lets us know
				//if we have already delayed this attack or not.
				sce->val1 = 0;
				sce->timer = add_timer(gettick()+10, status_change_timer, bl->id, type);
				return 1;
			}
		}
	}

	sc->data[type] = NULL;
	(sc->count)--;

	vd = status_get_viewdata(bl);
	calc_flag = StatusChangeFlagTable[type];
	switch(type){
		case SC_WEDDING:
		case SC_XMAS:
		case SC_SUMMER:
			if (!vd) break;
			if (sd)
			{	//Load data from sd->status.* as the stored values could have changed.
				//Must remove OPTION to prevent class being rechanged.
				sc->option &= type==SC_WEDDING?~OPTION_WEDDING:type==SC_XMAS?~OPTION_XMAS:~OPTION_SUMMER;
				clif_changeoption(&sd->bl);
				status_set_viewdata(bl, sd->status.class_);
			} else {
				vd->class_ = sce->val1;
				vd->weapon = sce->val2;
				vd->shield = sce->val3;
				vd->cloth_color = sce->val4;
			}
			clif_changelook(bl,LOOK_BASE,vd->class_);
			clif_changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
			clif_changelook(bl,LOOK_WEAPON,vd->weapon);
			clif_changelook(bl,LOOK_SHIELD,vd->shield);
			if(sd) clif_skillinfoblock(sd);
		break;
		case SC_RUN:
		{
			struct unit_data *ud = unit_bl2ud(bl);
			bool begin_spurt = true;
			if (ud) {
				if(!ud->state.running)
					begin_spurt = false;
				ud->state.running = 0;
				if (ud->walktimer != -1)
					unit_stop_walking(bl,1);
			}
			if (begin_spurt && sce->val1 >= 7 &&
				DIFF_TICK(gettick(), sce->val4) <= 1000 &&
				(!sd || (sd->weapontype1 == 0 && sd->weapontype2 == 0))
			)
				sc_start(bl,SC_SPURT,100,sce->val1,skill_get_time2(status_sc2skill(type), sce->val1));
		}
		break;
		case SC_AUTOBERSERK:
			if (sc->data[SC_PROVOKE] && sc->data[SC_PROVOKE]->val2 == 1)
				status_change_end(bl,SC_PROVOKE,-1);
			break;

		case SC_ENDURE:
		case SC_DEFENDER:
		case SC_REFLECTSHIELD:
		case SC_AUTOGUARD:
			{
				struct map_session_data *tsd;
				if( bl->type == BL_PC )
				{ // Clear Status from others
					int i;
					for( i = 0; i < 5; i++ )
					{
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) && tsd->sc.data[type] )
							status_change_end(&tsd->bl, type, -1);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag )
				{ // Clear Status from Master
					tsd = ((TBL_MER*)bl)->master;
					if( tsd && tsd->sc.data[type] )
						status_change_end(&tsd->bl, type, -1);
				}
			}
			break;
		case SC_DEVOTION:
			{
				struct block_list *d_bl = map_id2bl(sce->val1);
				if( d_bl )
				{
					if( d_bl->type == BL_PC )
						((TBL_PC*)d_bl)->devotion[sce->val2] = 0;
					else if( d_bl->type == BL_MER )
						((TBL_MER*)d_bl)->devotion_flag = 0;
					clif_devotion(d_bl, NULL);
				}

				status_change_end(bl,SC_AUTOGUARD,-1);
				status_change_end(bl,SC_DEFENDER,-1);
				status_change_end(bl,SC_REFLECTSHIELD,-1);
				status_change_end(bl,SC_ENDURE,-1);
			}
			break;

		case SC_BLADESTOP:
			if(sce->val4)
			{
				int tid = sce->val4;
				struct block_list *tbl = map_id2bl(tid);
				struct status_change *tsc = status_get_sc(tbl);
				sce->val4 = 0;
				if(tbl && tsc && tsc->data[SC_BLADESTOP])
				{
					tsc->data[SC_BLADESTOP]->val4 = 0;
					status_change_end(tbl,SC_BLADESTOP,-1);
				}
				clif_bladestop(bl, tid, 0);
			}
			break;
		case SC_DANCING:
			{
				struct map_session_data *dsd;
				struct status_change_entry *dsc;
				struct skill_unit_group *group;

				if(sce->val4 && sce->val4 != BCT_SELF && (dsd=map_id2sd(sce->val4)))
				{// end status on partner as well
					dsc = dsd->sc.data[SC_DANCING];
					if(dsc)
					{	//This will prevent recursive loops. 
						dsc->val2 = dsc->val4 = 0;
						status_change_end(&dsd->bl, SC_DANCING, -1);
					}
				}

				if(sce->val2)
				{// erase associated land skill
					group = skill_id2group(sce->val2);
					sce->val2 = 0;
					skill_delunitgroup(group);
				}

				if((sce->val1&0xFFFF) == CG_MOONLIT)
					clif_status_change(bl,SI_MOONLIT,0,0);

				status_change_end(bl,SC_LONGING,-1);
			}
			break;
		case SC_NOCHAT:
			if (sd && sd->status.manner < 0 && tid != -1)
				sd->status.manner = 0;
			if (sd)
			{
				clif_changestatus(&sd->bl,SP_MANNER,sd->status.manner);
				clif_updatestatus(sd,SP_MANNER);
			}
			break;
		case SC_SPLASHER:	
			{
				struct block_list *src=map_id2bl(sce->val3);
				if(src && tid!=-1)
					skill_castend_damage_id(src, bl, sce->val2, sce->val1, gettick(), SD_LEVEL );
			}
			break;
		case SC_CLOSECONFINE2:
			{
				struct block_list *src = sce->val2?map_id2bl(sce->val2):NULL;
				struct status_change *sc2 = src?status_get_sc(src):NULL;
				if (src && sc2 && sc2->data[SC_CLOSECONFINE]) {
					//If status was already ended, do nothing.
					//Decrease count
					if (--(sc2->data[SC_CLOSECONFINE]->val1) <= 0) //No more holds, free him up.
						status_change_end(src, SC_CLOSECONFINE, -1);
				}
			}
		case SC_CLOSECONFINE:
			if (sce->val2 > 0) {
				//Caster has been unlocked... nearby chars need to be unlocked.
				int range = 1
					+skill_get_range2(bl, status_sc2skill(type), sce->val1)
					+skill_get_range2(bl, TF_BACKSLIDING, 1); //Since most people use this to escape the hold....
				map_foreachinarea(status_change_timer_sub, 
					bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,bl,sce,type,gettick());
			}
			break;
		case SC_COMBO: //Clear last used skill when it is part of a combo.
			if( sd )
			{
				if( sd->state.combo )
				{
					sd->state.combo = 0;
					clif_skillinfoblock(sd);
				}
				if( sd->skillid_old == sce->val1 )
					sd->skillid_old = sd->skilllv_old = 0;
			}
			break;

		case SC_MARIONETTE:
		case SC_MARIONETTE2:	/// Marionette target
			if (sce->val1)
			{	// check for partner and end their marionette status as well
				enum sc_type type2 = (type == SC_MARIONETTE) ? SC_MARIONETTE2 : SC_MARIONETTE;
				struct block_list *pbl = map_id2bl(sce->val1);
				struct status_change* sc2 = pbl?status_get_sc(pbl):NULL;
				
				if (sc2 && sc2->data[type2])
				{
					sc2->data[type2]->val1 = 0;
					status_change_end(pbl, type2, -1);
				}
			}
			break;

		case SC_BERSERK:
			//If val2 is removed, no HP penalty (dispelled?) [Skotlex]
			if(status->hp > 100 && sce->val2)
				status_set_hp(bl, 100, 0); 
			if(sc->data[SC_ENDURE] && sc->data[SC_ENDURE]->val4 == 2)
			{
				sc->data[SC_ENDURE]->val4 = 0;
				status_change_end(bl, SC_ENDURE, -1);
			}
			sc_start4(bl, SC_REGENERATION, 100, 10,0,0,(RGN_HP|RGN_SP), skill_get_time(LK_BERSERK, sce->val1));
			break;
		case SC_GOSPEL:
			if (sce->val3) { //Clear the group.
				struct skill_unit_group* group = skill_id2group(sce->val3);
				sce->val3 = 0;
				skill_delunitgroup(group);
			}
			break;
		case SC_HERMODE: 
			if(sce->val3 == BCT_SELF)
				skill_clear_unitgroup(bl);
			break;
		case SC_BASILICA: //Clear the skill area. [Skotlex]
				skill_clear_unitgroup(bl);
				break;
		case SC_TRICKDEAD:
			if (vd) vd->dead_sit = 0;
			break;
		case SC_WARM:
			if (sce->val4) { //Clear the group.
				struct skill_unit_group* group = skill_id2group(sce->val4);
				sce->val4 = 0;
				skill_delunitgroup(group);
			}
			break;
		case SC_KAAHI:
			//Delete timer if it exists.
			if (sce->val4 != -1)
				delete_timer(sce->val4,kaahi_heal_timer);
			break;
		case SC_JAILED:
			if(tid == -1)
				break;
		  	//natural expiration.
			if(sd && sd->mapindex == sce->val2)
				pc_setpos(sd,(unsigned short)sce->val3,sce->val4&0xFFFF, sce->val4>>16, 3);
			break; //guess hes not in jail :P
		case SC_CHANGE:
			if (tid == -1)
		 		break;
			// "lose almost all their HP and SP" on natural expiration.
			status_set_hp(bl, 10, 0);
			status_set_sp(bl, 10, 0);
			break;
		case SC_AUTOTRADE:
			if (tid == -1)
				break;
			vending_closevending(sd);
			map_quit(sd);
			// Because map_quit calls status_change_end with tid -1
			// from here it's not neccesary to continue
			return 1;
			break;
		case SC_STOP:
			if( sce->val2 )
			{
				struct block_list* tbl = map_id2bl(sce->val2);
				sce->val2 = 0;
				if( tbl && (sc = status_get_sc(tbl)) && sc->data[SC_STOP] && sc->data[SC_STOP]->val2 == bl->id )
					status_change_end(tbl, SC_STOP, -1);
			}
			break;
		}

	opt_flag = 1;
	switch(type){
	case SC_STONE:
	case SC_FREEZE:
	case SC_STUN:
	case SC_SLEEP:
		sc->opt1 = 0;
		break;

	case SC_POISON:
	case SC_CURSE:
	case SC_SILENCE:
	case SC_BLIND:
		sc->opt2 &= ~(1<<(type-SC_POISON));
		break;
	case SC_DPOISON:
		sc->opt2 &= ~OPT2_DPOISON;
		break;
	case SC_SIGNUMCRUCIS:
		sc->opt2 &= ~OPT2_SIGNUMCRUCIS;
		break;

	case SC_HIDING:
		sc->option &= ~OPTION_HIDE;
		opt_flag|= 2|4; //Check for warp trigger + AoE trigger
		break;
	case SC_CLOAKING:
		sc->option &= ~OPTION_CLOAK;
		opt_flag|= 2;
		break;
	case SC_CHASEWALK:
		sc->option &= ~(OPTION_CHASEWALK|OPTION_CLOAK);
		opt_flag|= 2;
		break;
	case SC_SIGHT:
		sc->option &= ~OPTION_SIGHT;
		break;
	case SC_WEDDING:	
		sc->option &= ~OPTION_WEDDING;
		break;
	case SC_XMAS:	
		sc->option &= ~OPTION_XMAS;
		break;
	case SC_SUMMER:
		sc->option &= ~OPTION_SUMMER;
		break;
	case SC_ORCISH:
		sc->option &= ~OPTION_ORCISH;
		break;
	case SC_RUWACH:
		sc->option &= ~OPTION_RUWACH;
		break;
	case SC_FUSION:
		sc->option &= ~OPTION_FLYING;
		break;
	//opt3
	case SC_TWOHANDQUICKEN:
	case SC_ONEHAND:
	case SC_SPEARQUICKEN:
	case SC_CONCENTRATION:
	case SC_MERC_QUICKEN:
		sc->opt3 &= ~OPT3_QUICKEN;
		opt_flag = 0;
		break;
	case SC_OVERTHRUST:
	case SC_MAXOVERTHRUST:
	case SC_SWOO:
		sc->opt3 &= ~OPT3_OVERTHRUST;
		opt_flag = 0;
		break;
	case SC_ENERGYCOAT:
	case SC_SKE:
		sc->opt3 &= ~OPT3_ENERGYCOAT;
		opt_flag = 0;
		break;
	case SC_INCATKRATE: //Simulated Explosion spirits effect.
		if (bl->type != BL_MOB)
		{
			opt_flag = 0;
			break;
		}
	case SC_EXPLOSIONSPIRITS:
		sc->opt3 &= ~OPT3_EXPLOSIONSPIRITS;
		opt_flag = 0;
		break;
	case SC_STEELBODY:
	case SC_SKA:
		sc->opt3 &= ~OPT3_STEELBODY;
		opt_flag = 0;
		break;
	case SC_BLADESTOP:
		sc->opt3 &= ~OPT3_BLADESTOP;
		opt_flag = 0;
		break;
	case SC_AURABLADE:
		sc->opt3 &= ~OPT3_AURABLADE;
		opt_flag = 0;
		break;
	case SC_BERSERK:
		sc->opt3 &= ~OPT3_BERSERK;
		opt_flag = 0;
		break;
//	case ???: // doesn't seem to do anything
//		sc->opt3 &= ~OPT3_LIGHTBLADE;
//		opt_flag = 0;
//		break;
	case SC_DANCING:
		if ((sce->val1&0xFFFF) == CG_MOONLIT)
			sc->opt3 &= ~OPT3_MOONLIT;
		opt_flag = 0;
		break;
	case SC_MARIONETTE:
	case SC_MARIONETTE2:
		sc->opt3 &= ~OPT3_MARIONETTE;
		opt_flag = 0;
		break;
	case SC_ASSUMPTIO:
		sc->opt3 &= ~OPT3_ASSUMPTIO;
		opt_flag = 0;
		break;
	case SC_WARM: //SG skills [Komurka]
		sc->opt3 &= ~OPT3_WARM;
		opt_flag = 0;
		break;
	case SC_KAITE:
		sc->opt3 &= ~OPT3_KAITE;
		opt_flag = 0;
		break;
	case SC_BUNSINJYUTSU:
		sc->opt3 &= ~OPT3_BUNSIN;
		opt_flag = 0;
		break;
	case SC_SPIRIT:
		sc->opt3 &= ~OPT3_SOULLINK;
		opt_flag = 0;
		break;
	case SC_CHANGEUNDEAD:
		sc->opt3 &= ~OPT3_UNDEAD;
		opt_flag = 0;
		break;
//	case ???: // from DA_CONTRACT (looks like biolab mobs aura)
//		sc->opt3 &= ~OPT3_CONTRACT;
//		opt_flag = 0;
//		break;
	default:
		opt_flag = 0;
	}

	if (calc_flag&SCB_DYE)
	{	//Restore DYE color
		if (vd && !vd->cloth_color && sce->val4)
			clif_changelook(bl,LOOK_CLOTHES_COLOR,sce->val4);
		calc_flag&=~SCB_DYE;
	}

	//On Aegis, when turning off a status change, first goes the sc packet, then the option packet.
	if( vd && (pcdb_checkid(vd->class_) || bl->type == BL_MER ) )
		clif_status_change(bl,StatusIconChangeTable[type],0,0);
	else if (sd)
		clif_status_load(bl,StatusIconChangeTable[type],0);

	if(opt_flag)
		clif_changeoption(bl);

	if (calc_flag)
		status_calc_bl(bl,calc_flag);

	if(opt_flag&4) //Out of hiding, invoke on place.
		skill_unit_move(bl,gettick(),1);

	if(opt_flag&2 && sd && map_getcell(bl->m,bl->x,bl->y,CELL_CHKNPC))
		npc_touch_areanpc(sd,bl->m,bl->x,bl->y); //Trigger on-touch event.

	ers_free(sc_data_ers, sce);
	return 1;
}

int kaahi_heal_timer(int tid, unsigned int tick, int id, intptr data)
{
	struct block_list *bl;
	struct status_change *sc;
	struct status_change_entry *sce;
	struct status_data *status;
	int hp;

	if(!((bl=map_id2bl(id))&&
		(sc=status_get_sc(bl)) &&
		(sce = sc->data[SC_KAAHI])))
		return 0;

	if(sce->val4 != tid) {
		ShowError("kaahi_heal_timer: Timer mismatch: %d != %d\n", tid, sce->val4);
		sce->val4=-1;
		return 0;
	}

	status=status_get_status_data(bl);
	if(!status_charge(bl, 0, sce->val3)) {
		sce->val4=-1;
		return 0;
	}

	hp = status->max_hp - status->hp;
	if (hp > sce->val2)
		hp = sce->val2;
	if (hp)
		status_heal(bl, hp, 0, 2);
	sce->val4=-1;
	return 1;
}

/*==========================================
 * ステータス異常終了タイマー
 *------------------------------------------*/
int status_change_timer(int tid, unsigned int tick, int id, intptr data)
{
	enum sc_type type = (sc_type)data;
	struct block_list *bl;
	struct map_session_data *sd;
	struct status_data *status;
	struct status_change *sc;
	struct status_change_entry *sce;

	bl = map_id2bl(id);
	if(!bl)
	{
		ShowDebug("status_change_timer: Null pointer id: %d data: %d\n", id, data);
		return 0;
	}
	sc = status_get_sc(bl);
	status = status_get_status_data(bl);
	
	if(!(sc && (sce = sc->data[type])))
	{
		ShowDebug("status_change_timer: Null pointer id: %d data: %d bl-type: %d\n", id, data, bl->type);
		return 0;
	}

	if( sce->timer != tid )
	{
		ShowError("status_change_timer: Mismatch for type %d: %d != %d (bl id %d)\n",type,tid,sce->timer, bl->id);
		return 0;
	}

	sd = BL_CAST(BL_PC, bl);

// set the next timer of the sce (don't assume the status still exists)
#define sc_timer_next(t,f,i,d) \
	if( (sce=sc->data[type]) ) \
		sce->timer = add_timer(t,f,i,d); \
	else \
		ShowError("status_change_timer: Unexpected NULL status change id: %d data: %d\n", id, data)

	switch(type)
	{
	case SC_MAXIMIZEPOWER:
	case SC_CLOAKING:
		if(!status_charge(bl, 0, 1))
			break; //Not enough SP to continue.
		sc_timer_next(sce->val2+tick, status_change_timer, bl->id, data);
		return 0;

	case SC_CHASEWALK:
		if(!status_charge(bl, 0, sce->val4))
			break; //Not enough SP to continue.
			
		if (!sc->data[SC_INCSTR]) {
			sc_start(bl, SC_INCSTR,100,1<<(sce->val1-1),
				(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_ROGUE?10:1) //SL bonus -> x10 duration
				*skill_get_time2(status_sc2skill(type),sce->val1));
		}
		sc_timer_next(sce->val2+tick, status_change_timer, bl->id, data);
		return 0;
	break;

	case SC_SKA:  
		if(--(sce->val2)>0){  
			sce->val3 = rand()%100; //Random defense.  
			sc_timer_next(1000+tick, status_change_timer,bl->id, data);  
			return 0;  
		}  
		break;

	case SC_HIDING:
		if(--(sce->val2)>0){
			
			if(sce->val2 % sce->val4 == 0 && !status_charge(bl, 0, 1))
				break; //Fail if it's time to substract SP and there isn't.
		
			sc_timer_next(1000+tick, status_change_timer,bl->id, data);
			return 0;
		}
	break;

	case SC_SIGHT:
	case SC_RUWACH:
	case SC_SIGHTBLASTER:
		map_foreachinrange( status_change_timer_sub, bl, sce->val3, BL_CHAR, bl, sce, type, tick);

		if( --(sce->val2)>0 ){
			sc_timer_next(250+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
		
	case SC_PROVOKE:
		if(sce->val2) { //Auto-provoke (it is ended in status_heal)
			sc_timer_next(1000*60+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_STONE:
		if(sc->opt1 == OPT1_STONEWAIT && sce->val3) {
			sce->val4 = 0;
			unit_stop_walking(bl,1);
			sc->opt1 = OPT1_STONE;
			clif_changeoption(bl);
			sc_timer_next(1000+tick,status_change_timer, bl->id, data );
			status_calc_bl(bl, StatusChangeFlagTable[type]);
			return 0;
		}
		if(--(sce->val3) > 0) {
			if(++(sce->val4)%5 == 0 && status->hp > status->max_hp/4)
				status_percent_damage(NULL, bl, 1, 0, false);
			sc_timer_next(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_POISON:
		if(status->hp <= max(status->max_hp>>2, sce->val4)) //Stop damaging after 25% HP left.
			break;
	case SC_DPOISON:
		if (--(sce->val3) > 0) {
			if (!sc->data[SC_SLOWPOISON]) {
				bool flag;
				map_freeblock_lock();
				status_zap(bl, sce->val4, 0);
				flag = !sc->data[type]; //We check for this rather than 'killed' since the target could have revived with kaizel.
				map_freeblock_unlock();
				if (flag) return 0; //target died, SC cancelled already.
			}
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_TENSIONRELAX:
		if(status->max_hp > status->hp && --(sce->val3) > 0){
			sc_timer_next(sce->val4+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_KNOWLEDGE:
		if (!sd) break;
		if(bl->m == sd->feel_map[0].m ||
			bl->m == sd->feel_map[1].m ||
			bl->m == sd->feel_map[2].m)
		{	//Timeout will be handled by pc_setpos
			sce->timer = INVALID_TIMER;
			return 0;
		}
		break;

	case SC_BLEEDING:
		if (--(sce->val4) >= 0) {
			int flag, hp =  rand()%600 + 200;
			map_freeblock_lock();
			status_fix_damage(NULL, bl, sd||hp<status->hp?hp:status->hp-1, 0);
			flag = !sc->data[type];
			map_freeblock_unlock();
			if( !flag ) {
				if( status->hp == 1 ) break;
				sc_timer_next(10000 + tick, status_change_timer, bl->id, data);
			}
			return 0;
		}
		break;

	case SC_S_LIFEPOTION:
	case SC_L_LIFEPOTION:
		if( sd && --(sce->val4) >= 0 )
		{
			// val1 < 0 = per max% | val1 > 0 = exact amount
			int hp = 0;
			if( status->hp < status->max_hp )
				hp = (sce->val1 < 0) ? (int)(sd->status.max_hp * -1 * sce->val1 / 100.) : sce->val1 ;
			status_heal(bl, hp, 0, 2);
			sc_timer_next((sce->val2 * 1000) + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_BOSSMAPINFO:
		if( sd && --(sce->val4) >= 0 )
		{
			struct mob_data *boss_md = map_id2boss(sce->val1);
			if( boss_md && sd->bl.m == boss_md->bl.m )
			{
				clif_bossmapinfo(sd->fd, boss_md, 1); // Update X - Y on minimap
				if (boss_md->bl.prev != NULL) {
					sc_timer_next(5000 + tick, status_change_timer, bl->id, data);
					return 0;
				}
			}
		}
		break;

	case SC_DANCING: //ダンススキルの時間SP消費
		{
			int s = 0;
			int sp = 1;
			if (--sce->val3 <= 0)
				break;
			switch(sce->val1&0xFFFF){
				case BD_RICHMANKIM:
				case BD_DRUMBATTLEFIELD:
				case BD_RINGNIBELUNGEN:
				case BD_SIEGFRIED:
				case BA_DISSONANCE:
				case BA_ASSASSINCROSS:
				case DC_UGLYDANCE:
					s=3;
					break;
				case BD_LULLABY:
				case BD_ETERNALCHAOS:
				case BD_ROKISWEIL:
				case DC_FORTUNEKISS:
					s=4;
					break;
				case CG_HERMODE:
				case BD_INTOABYSS:
				case BA_WHISTLE:
				case DC_HUMMING:
				case BA_POEMBRAGI:
				case DC_SERVICEFORYOU:
					s=5;
					break;
				case BA_APPLEIDUN:
					s=6;
					break;
				case CG_MOONLIT:
					//Moonlit's cost is 4sp*skill_lv [Skotlex]
					sp= 4*(sce->val1>>16);
					//Upkeep is also every 10 secs.
				case DC_DONTFORGETME:
					s=10;
					break;
			}
			if( s != 0 && sce->val3 % s == 0 )
			{
				if (sc->data[SC_LONGING])
					sp*= 3;
				if (!status_charge(bl, 0, sp))
					break;
			}
			sc_timer_next(1000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_DEVOTION:
		//FIXME: use normal status duration instead of a looping timer
		if( (sce->val4 -= 1000) > 0 )
		{
			sc_timer_next(1000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
		
	case SC_BERSERK:
		// 5% every 10 seconds [DracoRPG]
		if(--(sce->val3)>0 && status_charge(bl, sce->val2, 0))
		{
			sc_timer_next(sce->val4+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_NOCHAT:
		if(sd){
			sd->status.manner++;
			clif_changestatus(bl,SP_MANNER,sd->status.manner);
			clif_updatestatus(sd,SP_MANNER);
			if (sd->status.manner < 0)
			{	//Every 60 seconds your manner goes up by 1 until it gets back to 0.
				sc_timer_next(60000+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SPLASHER:
		// custom Venom Splasher countdown timer
		//if (sce->val4 % 1000 == 0) {
		//	char timer[10];
		//	snprintf (timer, 10, "%d", sce->val4/1000);
		//	clif_message(bl, timer);
		//}
		if((sce->val4 -= 500) > 0) {
			sc_timer_next(500 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_MARIONETTE:
	case SC_MARIONETTE2:
		{
			struct block_list *pbl = map_id2bl(sce->val1);
			if( pbl && check_distance_bl(bl, pbl, 7) )
			{
				sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_GOSPEL:
		if(sce->val4 == BCT_SELF && --(sce->val2) > 0)
		{
			int hp, sp;
			hp = (sce->val1 > 5) ? 45 : 30;
			sp = (sce->val1 > 5) ? 35 : 20;
			if(!status_charge(bl, hp, sp))
				break;
			sc_timer_next(10000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
		
	case SC_GUILDAURA:
		{
			struct block_list *tbl = map_id2bl(sce->val2);
			
			if (tbl && battle_check_range(bl, tbl, 2)){
				sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_JAILED:
		if(sce->val1 == INT_MAX || --(sce->val1) > 0)
		{
			sc_timer_next(60000+tick, status_change_timer, bl->id,data);
			return 0;
		}
		break;

	case SC_BLIND:
		if(sc->data[SC_FOGWALL]) 
		{	//Blind lasts forever while you are standing on the fog.
			sc_timer_next(5000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	}

	// default for all non-handled control paths is to end the status
	return status_change_end( bl,type,tid );	
#undef sc_timer_next
}

/*==========================================
 * ステータス異常タイマー範囲処理
 *------------------------------------------*/
int status_change_timer_sub(struct block_list* bl, va_list ap)
{
	struct status_change* tsc;

	struct block_list* src = va_arg(ap,struct block_list*);
	struct status_change_entry* sce = va_arg(ap,struct status_change_entry*);
	enum sc_type type = (sc_type)va_arg(ap,int); //gcc: enum args get promoted to int
	unsigned int tick = va_arg(ap,unsigned int);

	if (status_isdead(bl))
		return 0;

	tsc = status_get_sc(bl);

	switch( type )
	{
	case SC_SIGHT:	/* サイト */
	case SC_CONCENTRATE:
		status_change_end(bl, SC_HIDING, -1);
		status_change_end(bl, SC_CLOAKING, -1);
		break;
	case SC_RUWACH:	/* ルアフ */
		if (tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING])) {
			status_change_end(bl, SC_HIDING, -1);
			status_change_end(bl, SC_CLOAKING, -1);
			if(battle_check_target( src, bl, BCT_ENEMY ) > 0)
				skill_attack(BF_MAGIC,src,src,bl,AL_RUWACH,1,tick,0);
		}
		break;
	case SC_SIGHTBLASTER:
		if (battle_check_target( src, bl, BCT_ENEMY ) > 0 &&
			status_check_skilluse(src, bl, WZ_SIGHTBLASTER, 2))
		{
			skill_attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,1,tick,0);
			if (sce) sce->val2 = 0; //This signals it to end.
		}
		break;
	case SC_CLOSECONFINE:
		//Lock char has released the hold on everyone...
		if (tsc && tsc->data[SC_CLOSECONFINE2] && tsc->data[SC_CLOSECONFINE2]->val2 == src->id) {
			tsc->data[SC_CLOSECONFINE2]->val2 = 0;
			status_change_end(bl, SC_CLOSECONFINE2, -1);
		}
		break;
	}
	return 0;
}

/*==========================================
 * Clears buffs/debuffs of a character.
 * type&1 -> buffs, type&2 -> debuffs
 *------------------------------------------*/
int status_change_clear_buffs (struct block_list* bl, int type)
{
	int i;
	struct status_change *sc= status_get_sc(bl);

	if (!sc || !sc->count)
		return 0;

	if (type&2) //Debuffs
	for( i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++ )
	{
		if(sc->data[i])
			status_change_end(bl,(sc_type)i,-1);
	}

	for( i = SC_COMMON_MAX+1; i < SC_MAX; i++ )
	{
		if(!sc->data[i])
			continue;
		
		switch (i) {
			//Stuff that cannot be removed
			case SC_WEIGHT50:
			case SC_WEIGHT90:
			case SC_COMBO:
			case SC_SMA:
			case SC_DANCING:
			case SC_GUILDAURA:
			case SC_SAFETYWALL:
			case SC_PNEUMA:
			case SC_NOCHAT:
			case SC_JAILED:
			case SC_ANKLE:
			case SC_BLADESTOP:
			case SC_CP_WEAPON:
			case SC_CP_SHIELD:
			case SC_CP_ARMOR:
			case SC_CP_HELM:
			case SC_STRFOOD:
			case SC_AGIFOOD:
			case SC_VITFOOD:
			case SC_INTFOOD:
			case SC_DEXFOOD:
			case SC_LUKFOOD:
			case SC_HITFOOD:
			case SC_FLEEFOOD:
			case SC_BATKFOOD:
			case SC_WATKFOOD:
			case SC_MATKFOOD:
			case SC_FOOD_STR_CASH:
			case SC_FOOD_AGI_CASH:
			case SC_FOOD_VIT_CASH:
			case SC_FOOD_DEX_CASH:
			case SC_FOOD_INT_CASH:
			case SC_FOOD_LUK_CASH:
				continue;
				
			//Debuffs that can be removed.
			case SC_HALLUCINATION:
			case SC_QUAGMIRE:
			case SC_SIGNUMCRUCIS:
			case SC_DECREASEAGI:
			case SC_SLOWDOWN:
			case SC_MINDBREAKER:
			case SC_WINKCHARM:
			case SC_STOP:
			case SC_ORCISH:
			case SC_STRIPWEAPON:
			case SC_STRIPSHIELD:
			case SC_STRIPARMOR:
			case SC_STRIPHELM:
				if (!(type&2))
					continue;
				break;
			//The rest are buffs that can be removed.
			case SC_BERSERK:
				if (!(type&1))
					continue;
			  	sc->data[i]->val2 = 0;
				break;
			default:
				if (!(type&1))
					continue;
				break;
		}
		status_change_end(bl,(sc_type)i,-1);
	}
	return 0;
}

//Natural regen related stuff.
static unsigned int natural_heal_prev_tick,natural_heal_diff_tick;
static int status_natural_heal(struct block_list* bl, va_list args)
{
	struct regen_data *regen;
	struct status_data *status;
	struct status_change *sc;
	struct unit_data *ud;
	struct view_data *vd = NULL;
	struct regen_data_sub *sregen;
	struct map_session_data *sd;
	int val,rate,bonus = 0,flag;

	regen = status_get_regen_data(bl);
	if (!regen) return 0;
	status = status_get_status_data(bl);
	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;
	sd = BL_CAST(BL_PC,bl);

	flag = regen->flag;
	if (flag&RGN_HP && (status->hp >= status->max_hp || regen->state.block&1))
		flag&=~(RGN_HP|RGN_SHP);
	if (flag&RGN_SP && (status->sp >= status->max_sp || regen->state.block&2))
		flag&=~(RGN_SP|RGN_SSP);

	if (flag && (
		status_isdead(bl) ||
		(sc && sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK))
	))
		flag=0;

	if (sd) {
		if (sd->hp_loss.value || sd->sp_loss.value)
			pc_bleeding(sd, natural_heal_diff_tick);
		if (sd->hp_regen.value || sd->sp_regen.value)
			pc_regen(sd, natural_heal_diff_tick);
	}

	if(flag&(RGN_SHP|RGN_SSP) && regen->ssregen &&
		(vd = status_get_viewdata(bl)) && vd->dead_sit == 2)
	{	//Apply sitting regen bonus.
		sregen = regen->ssregen;
		if(flag&(RGN_SHP))
		{	//Sitting HP regen
			val = natural_heal_diff_tick * sregen->rate.hp;
			if (regen->state.overweight)
				val>>=1; //Half as fast when overweight.
			sregen->tick.hp += val;
			while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval)
			{
				sregen->tick.hp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp)
				{	//Full
					flag&=~(RGN_HP|RGN_SHP);
					break;
				}
			}
		}
		if(flag&(RGN_SSP))
		{	//Sitting SP regen
			val = natural_heal_diff_tick * sregen->rate.sp;
			if (regen->state.overweight)
				val>>=1; //Half as fast when overweight.
			sregen->tick.sp += val;
			while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval)
			{
				sregen->tick.sp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, 0, sregen->sp, 3) < sregen->sp)
				{	//Full
					flag&=~(RGN_SP|RGN_SSP);
					break;
				}
			}
		}
	}

	if (flag && regen->state.overweight)
		flag=0;

	ud = unit_bl2ud(bl);

	if (flag&(RGN_HP|RGN_SHP|RGN_SSP) && ud && ud->walktimer != -1)
	{
		flag&=~(RGN_SHP|RGN_SSP);
		if(!regen->state.walk)
			flag&=~RGN_HP;
	}

	if (!flag)
		return 0;

	if (flag&(RGN_HP|RGN_SP))
	{
		if(!vd) vd = status_get_viewdata(bl);
		if(vd && vd->dead_sit == 2)
			bonus++;
		if(regen->state.gc)
			bonus++;
	}

	//Natural Hp regen
	if (flag&RGN_HP)
	{
		rate = natural_heal_diff_tick*(regen->rate.hp+bonus);
		if (ud && ud->walktimer != -1)
			rate/=2;
		// Homun HP regen fix (they should regen as if they were sitting (twice as fast) 
		if(bl->type==BL_HOM) rate *=2;

		regen->tick.hp += rate;
		
		if(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval)
		{
			val = 0;
			do {
				val += regen->hp;
				regen->tick.hp -= battle_config.natural_healhp_interval;
			} while(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval);
			if (status_heal(bl, val, 0, 1) < val)
				flag&=~RGN_SHP; //full.
		}
	}

	//Natural SP regen
	if(flag&RGN_SP)
	{
		rate = natural_heal_diff_tick*(regen->rate.sp+bonus);
		// Homun SP regen fix (they should regen as if they were sitting (twice as fast) 
		if(bl->type==BL_HOM) rate *=2;

		regen->tick.sp += rate;
		
		if(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval)
		{
			val = 0;
			do {
				val += regen->sp;
				regen->tick.sp -= battle_config.natural_healsp_interval;
			} while(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval);
			if (status_heal(bl, 0, val, 1) < val)
				flag&=~RGN_SSP; //full.
		}
	}

	if (!regen->sregen)
		return flag;

	//Skill regen
	sregen = regen->sregen;

	if(flag&RGN_SHP)
	{	//Skill HP regen
		sregen->tick.hp += natural_heal_diff_tick * sregen->rate.hp;
		
		while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval)
		{
			sregen->tick.hp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp)
				break; //Full
		}
	}
	if(flag&RGN_SSP)
	{	//Skill SP regen
		sregen->tick.sp += natural_heal_diff_tick * sregen->rate.sp;
		while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval)
		{
			val = sregen->sp;
			if (sd && sd->state.doridori) {
				val*=2;
				sd->state.doridori = 0;
				if ((rate = pc_checkskill(sd,TK_SPTIME)))
					sc_start(bl,status_skill2sc(TK_SPTIME),
						100,rate,skill_get_time(TK_SPTIME, rate));
				if (
					(sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR &&
					rand()%10000 < battle_config.sg_angel_skill_ratio
				) { //Angel of the Sun/Moon/Star
					clif_feel_hate_reset(sd);
					pc_resethate(sd);
					pc_resetfeel(sd);
				}
			}
			sregen->tick.sp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, 0, val, 3) < val)
				break; //Full
		}
	}
	return flag;
}

//Natural heal main timer.
static int status_natural_heal_timer(int tid, unsigned int tick, int id, intptr data)
{
	natural_heal_diff_tick = DIFF_TICK(tick,natural_heal_prev_tick);
	map_foreachregen(status_natural_heal);
	natural_heal_prev_tick = tick;
	return 0;
}

int status_readdb(void)
{
	int i,j,class_;
	FILE *fp;
	char line[1024], path[1024],*p;

	sprintf(path, "%s/job_db1.txt", db_path);
	fp=fopen(path,"r"); // Job-specific values (weight, HP, SP, ASPD)
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	i = 0;
	while(fgets(line, sizeof(line), fp))
	{
		//NOTE: entry MAX_WEAPON_TYPE is not counted
		char* split[5 + MAX_WEAPON_TYPE];
		i++;
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line; j < 5 + MAX_WEAPON_TYPE && p; j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j < 5 + MAX_WEAPON_TYPE)
		{	//Weapon #.MAX_WEAPON_TYPE is constantly not load. Fix to that: replace < with <= [blackhole89]
			ShowDebug("%s: Not enough columns at line %d\n", path, i);
			continue;
		}
		class_ = atoi(split[0]);
		if(!pcdb_checkid(class_))
			continue;
		class_ = pc_class2idx(class_);
		max_weight_base[class_]=atoi(split[1]);
		hp_coefficient[class_]=atoi(split[2]);
		hp_coefficient2[class_]=atoi(split[3]);
		sp_coefficient[class_]=atoi(split[4]);
		for(j=0;j<MAX_WEAPON_TYPE;j++)
			aspd_base[class_][j]=atoi(split[j+5]);
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",path);

	memset(job_bonus,0,sizeof(job_bonus)); // Job-specific stats bonus
	sprintf(path, "%s/job_db2.txt", db_path);
	fp=fopen(path,"r");
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	while(fgets(line, sizeof(line), fp))
	{
		char *split[MAX_LEVEL+1]; //Job Level is limited to MAX_LEVEL, so the bonuses should likewise be limited to it. [Skotlex]
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<MAX_LEVEL+1 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		class_ = atoi(split[0]);
		if(!pcdb_checkid(class_))
		    continue;
		class_ = pc_class2idx(class_);
		for(i=1;i<j && split[i];i++)
			job_bonus[class_][i-1]=atoi(split[i]);
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",path);

	// サイズ補正テ?ブル
	for(i=0;i<3;i++)
		for(j=0;j<MAX_WEAPON_TYPE;j++)
			atkmods[i][j]=100;
	sprintf(path, "%s/size_fix.txt", db_path);
	fp=fopen(path,"r");
	if(fp==NULL){
		ShowError("can't read %s\n", path);
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line), fp))
	{
		char *split[MAX_WEAPON_TYPE];
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<MAX_WEAPON_TYPE && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
			atkmods[i][j]=atoi(split[j]);
		}
		i++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",path);

	// 精?デ?タテ?ブル
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
	while(fgets(line, sizeof(line), fp))
	{
		char *split[MAX_REFINE+4];
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<MAX_REFINE+4 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		refinebonus[i][0]=atoi(split[0]);	// 精?ボ?ナス
		refinebonus[i][1]=atoi(split[1]);	// 過?精?ボ?ナス
		refinebonus[i][2]=atoi(split[2]);	// 安全精?限界
		for(j=0;j<MAX_REFINE && split[j+3];j++)
			percentrefinery[i][j]=atoi(split[j+3]);
		i++;
	}
	fclose(fp); //Lupus. close this file!!!
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",path);

	return 0;
}

/*==========================================
 * スキル関係初期化処理
 *------------------------------------------*/
int do_init_status(void)
{
	add_timer_func_list(status_change_timer,"status_change_timer");
	add_timer_func_list(kaahi_heal_timer,"kaahi_heal_timer");
	add_timer_func_list(status_natural_heal_timer,"status_natural_heal_timer");
	initChangeTables();
	initDummyData();
	status_readdb();
	status_calc_sigma();
	natural_heal_prev_tick = gettick();
	sc_data_ers = ers_new(sizeof(struct status_change_entry));
	add_timer_interval(natural_heal_prev_tick + NATURAL_HEAL_INTERVAL, status_natural_heal_timer, 0, 0, NATURAL_HEAL_INTERVAL);
	return 0;
}
void do_final_status(void)
{
	ers_destroy(sc_data_ers);
}
