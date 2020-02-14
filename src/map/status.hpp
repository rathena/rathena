// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef STATUS_HPP
#define STATUS_HPP

#include "../common/mmo.hpp"
#include "../common/timer.hpp"

enum e_race2 : uint8;
struct block_list;
struct mob_data;
struct pet_data;
struct homun_data;
struct mercenary_data;
struct elemental_data;
struct npc_data;
struct status_change;

/**
 * Max Refine available to your server
 * Changing this limit requires edits to refine_db.txt
 **/
#ifdef RENEWAL
#	define MAX_REFINE 20
#else
#	define MAX_REFINE 10
#endif

/// Refine type
enum refine_type {
	REFINE_TYPE_ARMOR	= 0,
	REFINE_TYPE_WEAPON1	= 1,
	REFINE_TYPE_WEAPON2	= 2,
	REFINE_TYPE_WEAPON3	= 3,
	REFINE_TYPE_WEAPON4	= 4,
	REFINE_TYPE_SHADOW	= 5,
	REFINE_TYPE_MAX		= 6
};

/// Refine cost type
enum refine_cost_type {
	REFINE_COST_NORMAL = 0,
	REFINE_COST_OVER10,
	REFINE_COST_HD,
	REFINE_COST_ENRICHED,
	REFINE_COST_OVER10_HD,
	REFINE_COST_HOLINK,
	REFINE_COST_WAGJAK,
	REFINE_COST_MAX
};

struct refine_cost {
	unsigned short nameid;
	int zeny;
};

/// Get refine chance
int status_get_refine_chance(enum refine_type wlv, int refine, bool enriched);
int status_get_refine_cost(int weapon_lv, int type, bool what);

/// Status changes listing. These code are for use by the server.
enum sc_type : int16 {
	SC_NONE = -1,

	//First we enumerate common status ailments which are often used around.
	SC_STONE = 0,
	SC_COMMON_MIN = 0, // begin
	SC_FREEZE,
	SC_STUN,
	SC_SLEEP,
	SC_POISON,
	SC_CURSE,
	SC_SILENCE,
	SC_CONFUSION,
	SC_BLIND,
	SC_BLEEDING,
	SC_DPOISON, //10
	SC_COMMON_MAX = 10, // end

	//Next up, we continue on 20, to leave enough room for additional "common" ailments in the future.
	SC_PROVOKE = 20,
	SC_ENDURE,
	SC_TWOHANDQUICKEN,
	SC_CONCENTRATE,
	SC_HIDING,
	SC_CLOAKING,
	SC_ENCPOISON,
	SC_POISONREACT,
	SC_QUAGMIRE,
	SC_ANGELUS,
	SC_BLESSING, //30
	SC_SIGNUMCRUCIS,
	SC_INCREASEAGI,
	SC_DECREASEAGI,
	SC_SLOWPOISON,
	SC_IMPOSITIO  ,
	SC_SUFFRAGIUM,
	SC_ASPERSIO,
	SC_BENEDICTIO,
	SC_KYRIE,
	SC_MAGNIFICAT, //40
	SC_GLORIA,
	SC_AETERNA,
	SC_ADRENALINE,
	SC_WEAPONPERFECTION,
	SC_OVERTHRUST,
	SC_MAXIMIZEPOWER,
	SC_TRICKDEAD,
	SC_LOUD,
	SC_ENERGYCOAT,
	SC_BROKENARMOR, //50 - NOTE: These two aren't used anywhere, and they have an icon...
	SC_BROKENWEAPON,
	SC_HALLUCINATION,
	SC_WEIGHT50,
	SC_WEIGHT90,
	SC_ASPDPOTION0,
	SC_ASPDPOTION1,
	SC_ASPDPOTION2,
	SC_ASPDPOTION3,
	SC_SPEEDUP0,
	SC_SPEEDUP1, //60
	SC_ATKPOTION,
	SC_MATKPOTION,
	SC_WEDDING,
	SC_SLOWDOWN,
	SC_ANKLE,
	SC_KEEPING,
	SC_BARRIER,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR, //70
	SC_STRIPHELM,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	SC_AUTOGUARD,
	SC_REFLECTSHIELD,
	SC_SPLASHER,
	SC_PROVIDENCE,
	SC_DEFENDER, //80
	SC_MAGICROD,
	SC_SPELLBREAKER,
	SC_AUTOSPELL,
	SC_SIGHTTRASHER,
	SC_AUTOBERSERK,
	SC_SPEARQUICKEN,
	SC_AUTOCOUNTER,
	SC_SIGHT,
	SC_SAFETYWALL,
	SC_RUWACH, //90
	SC_EXTREMITYFIST,
	SC_EXPLOSIONSPIRITS,
	SC_COMBO,
	SC_BLADESTOP_WAIT,
	SC_BLADESTOP,
	SC_FIREWEAPON,
	SC_WATERWEAPON,
	SC_WINDWEAPON,
	SC_EARTHWEAPON,
	SC_VOLCANO, //100,
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_WATK_ELEMENT,
	SC_ARMOR,
	SC_ARMOR_ELEMENT_WATER,
	SC_NOCHAT,
	SC_BABY,
	SC_AURABLADE,
	SC_PARRYING,
	SC_CONCENTRATION, //110
	SC_TENSIONRELAX,
	SC_BERSERK,
	SC_FURY,
	SC_GOSPEL,
	SC_ASSUMPTIO,
	SC_BASILICA,
	SC_GUILDAURA,
	SC_MAGICPOWER,
	SC_EDP,
	SC_TRUESIGHT, //120
	SC_WINDWALK,
	SC_MELTDOWN,
	SC_CARTBOOST,
	SC_CHASEWALK,
	SC_REJECTSWORD,
	SC_MARIONETTE,
	SC_MARIONETTE2,
	SC_CHANGEUNDEAD,
	SC_JOINTBEAT,
	SC_MINDBREAKER, //130
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	SC_DEVOTION,
	SC_SACRIFICE,
	SC_STEELBODY,
	SC_ORCISH,
	SC_READYSTORM,
	SC_READYDOWN,
	SC_READYTURN, //140
	SC_READYCOUNTER,
	SC_DODGE,
	SC_RUN,
	SC_SHADOWWEAPON,
	SC_ADRENALINE2,
	SC_GHOSTWEAPON,
	SC_KAIZEL,
	SC_KAAHI,
	SC_KAUPE,
	SC_ONEHAND, //150
	SC_PRESERVE,
	SC_BATTLEORDERS,
	SC_REGENERATION,
	SC_DOUBLECAST,
	SC_GRAVITATION,
	SC_MAXOVERTHRUST,
	SC_LONGING,
	SC_HERMODE,
	SC_SHRINK,
	SC_SIGHTBLASTER, //160
	SC_WINKCHARM,
	SC_CLOSECONFINE,
	SC_CLOSECONFINE2,
	SC_DANCING,
	SC_ELEMENTALCHANGE,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
	SC_NIBELUNGEN,
	SC_ROKISWEIL, //170
	SC_INTOABYSS,
	SC_SIEGFRIED,
	SC_WHISTLE,
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	SC_MODECHANGE,
	SC_HUMMING,
	SC_DONTFORGETME,
	SC_FORTUNE, //180
	SC_SERVICE4U,
	SC_STOP,	//Prevents inflicted chars from walking. [Skotlex]
	SC_SPURT,
	SC_SPIRIT,
	SC_COMA, //Not a real SC_, it makes a char's HP/SP hit 1.
	SC_INTRAVISION,
	SC_INCALLSTATUS,
	SC_INCSTR,
	SC_INCAGI,
	SC_INCVIT, //190
	SC_INCINT,
	SC_INCDEX,
	SC_INCLUK,
	SC_INCHIT,
	SC_INCHITRATE,
	SC_INCFLEE,
	SC_INCFLEERATE,
	SC_INCMHPRATE,
	SC_INCMSPRATE,
	SC_INCATKRATE, //200
	SC_INCMATKRATE,
	SC_INCDEFRATE,
	SC_STRFOOD,
	SC_AGIFOOD,
	SC_VITFOOD,
	SC_INTFOOD,
	SC_DEXFOOD,
	SC_LUKFOOD,
	SC_HITFOOD,
	SC_FLEEFOOD, //210
	SC_BATKFOOD,
	SC_WATKFOOD,
	SC_MATKFOOD,
	SC_SCRESIST, //Increases resistance to status changes.
	SC_XMAS, // Xmas Suit [Valaris]
	SC_WARM, //SG skills [Komurka]
	SC_SUN_COMFORT,
	SC_MOON_COMFORT,
	SC_STAR_COMFORT,
	SC_FUSION, //220
	SC_SKILLRATE_UP,
	SC_SKE,
	SC_KAITE,
	SC_SWOO, // [marquis007]
	SC_SKA, // [marquis007]
	SC_EARTHSCROLL,
	SC_MIRACLE, //SG 'hidden' skill [Komurka]
	SC_MADNESSCANCEL,
	SC_ADJUSTMENT,
	SC_INCREASING,  //230
	SC_GATLINGFEVER,
	SC_TATAMIGAESHI,
	SC_UTSUSEMI,
	SC_BUNSINJYUTSU,
	SC_KAENSIN,
	SC_SUITON,
	SC_NEN,
	SC_KNOWLEDGE,
	SC_SMA,
	SC_FLING,	//240
	SC_AVOID,
	SC_CHANGE,
	SC_BLOODLUST,
	SC_FLEET,
	SC_SPEED,
	SC_DEFENCE,
	SC_INCASPDRATE,
	SC_INCFLEE2 = 248,
	SC_JAILED,
	SC_ENCHANTARMS,	//250
	SC_MAGICALATTACK,
	SC_ARMORCHANGE,
	SC_CRITICALWOUND,
	SC_MAGICMIRROR,
	SC_SLOWCAST,
	SC_SUMMER,
	SC_EXPBOOST,
	SC_ITEMBOOST,
	SC_BOSSMAPINFO,
	SC_LIFEINSURANCE, //260
	SC_INCCRI,
	//SC_INCDEF,
	//SC_INCBASEATK = 263,
	//SC_FASTCAST,
	SC_MDEF_RATE = 265,
	//SC_HPREGEN,
	SC_INCHEALRATE = 267,
	SC_PNEUMA,
	SC_AUTOTRADE,
	SC_KSPROTECTED, //270
	SC_ARMOR_RESIST = 271,
	SC_SPCOST_RATE,
	SC_COMMONSC_RESIST,
	SC_SEVENWIND,
	SC_DEF_RATE,
	//SC_SPREGEN,
	SC_WALKSPEED = 277,

	// Mercenary Only Bonus Effects
	SC_MERC_FLEEUP,
	SC_MERC_ATKUP,
	SC_MERC_HPUP, //280
	SC_MERC_SPUP,
	SC_MERC_HITUP,
	SC_MERC_QUICKEN,

	SC_REBIRTH,
	//SC_SKILLCASTRATE, //285
	//SC_DEFRATIOATK,
	//SC_HPDRAIN,
	//SC_SKILLATKBONUS,
	SC_ITEMSCRIPT = 289,
	SC_S_LIFEPOTION, //290
	SC_L_LIFEPOTION,
	SC_JEXPBOOST,
	//SC_IGNOREDEF,
	SC_HELLPOWER = 294,
	SC_INVINCIBLE, //295
	SC_INVINCIBLEOFF,
	SC_MANU_ATK,
	SC_MANU_DEF,
	SC_SPL_ATK,
	SC_SPL_DEF, //300
	SC_MANU_MATK,
	SC_SPL_MATK,
	SC_FOOD_STR_CASH,
	SC_FOOD_AGI_CASH,
	SC_FOOD_VIT_CASH,
	SC_FOOD_DEX_CASH,
	SC_FOOD_INT_CASH,
	SC_FOOD_LUK_CASH,//308
	/**
	 * 3rd
	 **/
	SC_FEAR,//309
	SC_BURNING,//310
	SC_FREEZING,//311
	/**
	 * Rune Knight
	 **/
	SC_ENCHANTBLADE,//312
	SC_DEATHBOUND,//313
	SC_MILLENNIUMSHIELD,
	SC_CRUSHSTRIKE,//315
	SC_REFRESH,
	SC_REUSE_REFRESH,
	SC_GIANTGROWTH,
	SC_STONEHARDSKIN,
	SC_VITALITYACTIVATION,//320
	SC_STORMBLAST,
	SC_FIGHTINGSPIRIT,
	SC_ABUNDANCE,
	/**
	 * Arch Bishop
	**/
	SC_ADORAMUS,
	SC_EPICLESIS,//325
	SC_ORATIO,
	SC_LAUDAAGNUS,
	SC_LAUDARAMUS,
	SC_RENOVATIO,
	SC_EXPIATIO,//330
	SC_DUPLELIGHT,
	SC_SECRAMENT,
	/**
	 * Warlock
	 **/
	SC_WHITEIMPRISON,
	SC_MARSHOFABYSS,
	SC_RECOGNIZEDSPELL,//335
	SC_STASIS,
	SC_SPHERE_1,
	SC_SPHERE_2,
	SC_SPHERE_3,
	SC_SPHERE_4,//340
	SC_SPHERE_5,
	SC_READING_SB,
	SC_FREEZE_SP,
	/**
	 * Ranger
	 **/
	SC_FEARBREEZE,
	SC_ELECTRICSHOCKER,//345
	SC_WUGDASH,
	SC_BITE,
	SC_CAMOUFLAGE,
	/**
	 * Mechanic
	 **/
	SC_ACCELERATION,
	SC_HOVERING,//350
	SC_SHAPESHIFT,
	SC_INFRAREDSCAN,
	SC_ANALYZE,
	SC_MAGNETICFIELD,
	SC_NEUTRALBARRIER,//355
	SC_NEUTRALBARRIER_MASTER,
	SC_STEALTHFIELD,
	SC_STEALTHFIELD_MASTER,
	SC_OVERHEAT,
	SC_OVERHEAT_LIMITPOINT,//360
	/**
	 * Guillotine Cross
	 **/
	SC_VENOMIMPRESS,
	SC_POISONINGWEAPON,
	SC_WEAPONBLOCKING,
	SC_CLOAKINGEXCEED,
	SC_HALLUCINATIONWALK,//365
	SC_HALLUCINATIONWALK_POSTDELAY,
	SC_ROLLINGCUTTER,
	SC_TOXIN,
	SC_PARALYSE,
	SC_VENOMBLEED,//370
	SC_MAGICMUSHROOM,
	SC_DEATHHURT,
	SC_PYREXIA,
	SC_OBLIVIONCURSE,
	SC_LEECHESEND,//375
	/**
	 * Royal Guard
	 **/
	SC_REFLECTDAMAGE,
	SC_FORCEOFVANGUARD,
	SC_SHIELDSPELL_DEF,
	SC_SHIELDSPELL_MDEF,
	SC_SHIELDSPELL_REF,//380
	SC_EXEEDBREAK,
	SC_PRESTIGE,
	SC_BANDING,
	SC_BANDING_DEFENCE,
	SC_EARTHDRIVE,//385
	SC_INSPIRATION,
	/**
	 * Sorcerer
	 **/
	SC_SPELLFIST,
	SC_CRYSTALIZE,
	SC_STRIKING,
	SC_WARMER,//390
	SC_VACUUM_EXTREME,
	SC_PROPERTYWALK,
	/**
	 * Minstrel / Wanderer
	 **/
	SC_SWINGDANCE,
	SC_SYMPHONYOFLOVER,
	SC_MOONLITSERENADE,//395
	SC_RUSHWINDMILL,
	SC_ECHOSONG,
	SC_HARMONIZE,
	SC_VOICEOFSIREN,
	SC_DEEPSLEEP,//400
	SC_SIRCLEOFNATURE,
	SC_GLOOMYDAY,
	SC_GLOOMYDAY_SK,
	SC_SONGOFMANA,
	SC_DANCEWITHWUG,//405
	SC_SATURDAYNIGHTFEVER,
	SC_LERADSDEW,
	SC_MELODYOFSINK,
	SC_BEYONDOFWARCRY,
	SC_UNLIMITEDHUMMINGVOICE,//410
	SC_SITDOWN_FORCE,
	SC_NETHERWORLD,
	/**
	 * Sura
	 **/
	SC_CRESCENTELBOW,
	SC_CURSEDCIRCLE_ATKER,
	SC_CURSEDCIRCLE_TARGET,
	SC_LIGHTNINGWALK,//416
	SC_RAISINGDRAGON,
	SC_GT_ENERGYGAIN,
	SC_GT_CHANGE,
	SC_GT_REVITALIZE,
	/**
	 * Genetic
	 **/
	SC_GN_CARTBOOST,//421
	SC_THORNSTRAP,
	SC_BLOODSUCKER,
	SC_SMOKEPOWDER,
	SC_TEARGAS,
	SC_MANDRAGORA,//426
	SC_STOMACHACHE,
	SC_MYSTERIOUS_POWDER,
	SC_MELON_BOMB,
	SC_BANANA_BOMB,
	SC_BANANA_BOMB_SITDOWN,//431
	SC_SAVAGE_STEAK,
	SC_COCKTAIL_WARG_BLOOD,
	SC_MINOR_BBQ,
	SC_SIROMA_ICE_TEA,
	SC_DROCERA_HERB_STEAMED,//436
	SC_PUTTI_TAILS_NOODLES,
	SC_BOOST500,
	SC_FULL_SWING_K,
	SC_MANA_PLUS,
	SC_MUSTLE_M,//441
	SC_LIFE_FORCE_F,
	SC_EXTRACT_WHITE_POTION_Z,
	SC_VITATA_500,
	SC_EXTRACT_SALAMINE_JUICE,
	/**
	 * Shadow Chaser
	 **/
	SC__REPRODUCE,//446
	SC__AUTOSHADOWSPELL,
	SC__SHADOWFORM,
	SC__BODYPAINT,
	SC__INVISIBILITY,
	SC__DEADLYINFECT,//451
	SC__ENERVATION,
	SC__GROOMY,
	SC__IGNORANCE,
	SC__LAZINESS,
	SC__UNLUCKY,//456
	SC__WEAKNESS,
	SC__STRIPACCESSORY,
	SC__MANHOLE,
	SC__BLOODYLUST,//460
	/**
	 * Elemental Spirits
	 **/
	SC_CIRCLE_OF_FIRE,
	SC_CIRCLE_OF_FIRE_OPTION,
	SC_FIRE_CLOAK,
	SC_FIRE_CLOAK_OPTION,
	SC_WATER_SCREEN,//465
	SC_WATER_SCREEN_OPTION,
	SC_WATER_DROP,
	SC_WATER_DROP_OPTION,
	SC_WATER_BARRIER,
	SC_WIND_STEP,//470
	SC_WIND_STEP_OPTION,
	SC_WIND_CURTAIN,
	SC_WIND_CURTAIN_OPTION,
	SC_ZEPHYR,
	SC_SOLID_SKIN,//475
	SC_SOLID_SKIN_OPTION,
	SC_STONE_SHIELD,
	SC_STONE_SHIELD_OPTION,
	SC_POWER_OF_GAIA,
	SC_PYROTECHNIC,//480
	SC_PYROTECHNIC_OPTION,
	SC_HEATER,
	SC_HEATER_OPTION,
	SC_TROPIC,
	SC_TROPIC_OPTION,//485
	SC_AQUAPLAY,
	SC_AQUAPLAY_OPTION,
	SC_COOLER,
	SC_COOLER_OPTION,
	SC_CHILLY_AIR,//490
	SC_CHILLY_AIR_OPTION,
	SC_GUST,
	SC_GUST_OPTION,
	SC_BLAST,
	SC_BLAST_OPTION,//495
	SC_WILD_STORM,
	SC_WILD_STORM_OPTION,
	SC_PETROLOGY,
	SC_PETROLOGY_OPTION,
	SC_CURSED_SOIL,//500
	SC_CURSED_SOIL_OPTION,
	SC_UPHEAVAL,
	SC_UPHEAVAL_OPTION,
	SC_TIDAL_WEAPON,
	SC_TIDAL_WEAPON_OPTION,//505
	SC_ROCK_CRUSHER,
	SC_ROCK_CRUSHER_ATK,
	/* Guild Aura */
	SC_LEADERSHIP,
	SC_GLORYWOUNDS,
	SC_SOULCOLD, //508
	SC_HAWKEYES,
	/* ... */
	SC_ODINS_POWER,
	SC_RAID,
	/* Sorcerer .extra */
	SC_FIRE_INSIGNIA,
	SC_WATER_INSIGNIA,
	SC_WIND_INSIGNIA, //516
	SC_EARTH_INSIGNIA,
	/* new pushcart */
	SC_PUSH_CART,
	/* Warlock Spell books */
	SC_SPELLBOOK1,
	SC_SPELLBOOK2,
	SC_SPELLBOOK3,
	SC_SPELLBOOK4,
	SC_SPELLBOOK5,
	SC_SPELLBOOK6,
/**
 * In official server there are only 7 maximum number of spell books that can be memorized
 * To increase the maximum value just add another status type before SC_MAXSPELLBOOK (ex. SC_SPELLBOOK7, SC_SPELLBOOK8 and so on)
 **/
	SC_MAXSPELLBOOK,
	/* Max HP & SP */
	SC_INCMHP,
	SC_INCMSP,
	SC_PARTYFLEE, // 531
	/**
	* Kagerou & Oboro [malufett]
	**/
	SC_MEIKYOUSISUI,
	SC_JYUMONJIKIRI,
	SC_KYOUGAKU,
	SC_IZAYOI,
	SC_ZENKAI,
	SC_KAGEHUMI,
	SC_KYOMU,
	SC_KAGEMUSYA,
	SC_ZANGETSU,
	SC_GENSOU,
	SC_AKAITSUKI,

	//homon S
	SC_STYLE_CHANGE,
	SC_TINDER_BREAKER,
	SC_TINDER_BREAKER2,
	SC_CBC,
	SC_EQC,
	SC_GOLDENE_FERSE,
	SC_ANGRIFFS_MODUS,
	SC_OVERED_BOOST,
	SC_LIGHT_OF_REGENE,
	SC_ASH,
	SC_GRANITIC_ARMOR,
	SC_MAGMA_FLOW,
	SC_PYROCLASTIC,
	SC_PARALYSIS,
	SC_PAIN_KILLER,
	SC_HANBOK,
	//Vellum Weapon reductions
	SC_DEFSET,
	SC_MDEFSET,
	SC_DARKCROW,
	SC_FULL_THROTTLE,
	SC_REBOUND,
	SC_UNLIMIT,
	SC_KINGS_GRACE,
	SC_TELEKINESIS_INTENSE,
	SC_OFFERTORIUM,
	SC_FRIGG_SONG,
	SC_MONSTER_TRANSFORM,
	SC_ANGEL_PROTECT,
	SC_ILLUSIONDOPING,
	SC_FLASHCOMBO,
	SC_MOONSTAR,
	SC_SUPER_STAR,

	/**
	 * Rebellion [Cydh]
	 **/
	SC_HEAT_BARREL,
	SC_MAGICALBULLET,
	SC_P_ALTER,
	SC_E_CHAIN,
	SC_C_MARKER,
	SC_ANTI_M_BLAST,
	SC_B_TRAP,
	SC_H_MINE,
	SC_QD_SHOT_READY,

	SC_MTF_ASPD,
	SC_MTF_RANGEATK,
	SC_MTF_MATK,
	SC_MTF_MLEATKED,
	SC_MTF_CRIDAMAGE,

	SC_OKTOBERFEST,
	SC_STRANGELIGHTS,
	SC_DECORATION_OF_MUSIC,

	SC_QUEST_BUFF1,
	SC_QUEST_BUFF2,
	SC_QUEST_BUFF3,

	SC_ALL_RIDING,

	SC_TEARGAS_SOB,
	SC__FEINTBOMB,
	SC__CHAOS,
	SC_CHASEWALK2,
	SC_VACUUM_EXTREME_POSTDELAY,

	SC_MTF_ASPD2,
	SC_MTF_RANGEATK2,
	SC_MTF_MATK2,
	SC_2011RWC_SCROLL,
	SC_JP_EVENT04,

	// 2014 Halloween Event
	SC_MTF_MHP,
	SC_MTF_MSP,
	SC_MTF_PUMPKIN,
	SC_MTF_HITFLEE,

	SC_CRIFOOD,
	SC_ATTHASTE_CASH,

	// Item Reuse Limits
	SC_REUSE_LIMIT_A,
	SC_REUSE_LIMIT_B,
	SC_REUSE_LIMIT_C,
	SC_REUSE_LIMIT_D,
	SC_REUSE_LIMIT_E,
	SC_REUSE_LIMIT_F,
	SC_REUSE_LIMIT_G,
	SC_REUSE_LIMIT_H,
	SC_REUSE_LIMIT_MTF,
	SC_REUSE_LIMIT_ASPD_POTION,
	SC_REUSE_MILLENNIUMSHIELD,
	SC_REUSE_CRUSHSTRIKE,
	SC_REUSE_STORMBLAST,
	SC_ALL_RIDING_REUSE_LIMIT,
	SC_REUSE_LIMIT_ECL,
	SC_REUSE_LIMIT_RECALL,

	SC_PROMOTE_HEALTH_RESERCH,
	SC_ENERGY_DRINK_RESERCH,
	SC_NORECOVER_STATE,

	/**
	 * Summoner
	 */
	SC_SUHIDE,
	SC_SU_STOOP,
	SC_SPRITEMABLE,
	SC_CATNIPPOWDER,
	SC_SV_ROOTTWIST,
	SC_BITESCAR,
	SC_ARCLOUSEDASH,
	SC_TUNAPARTY,
	SC_SHRIMP,
	SC_FRESHSHRIMP,

	SC_ACTIVE_MONSTER_TRANSFORM,

	SC_CLOUD_KILL, // Deprecated

	SC_LJOSALFAR,
	SC_MERMAID_LONGING,
	SC_HAT_EFFECT,
	SC_FLOWERSMOKE,
	SC_FSTONE,
	SC_HAPPINESS_STAR,
	SC_MAPLE_FALLS,
	SC_TIME_ACCESSORY,
	SC_MAGICAL_FEATHER,
	SC_GVG_GIANT,
	SC_GVG_GOLEM,
	SC_GVG_STUN,
	SC_GVG_STONE,
	SC_GVG_FREEZ,
	SC_GVG_SLEEP,
	SC_GVG_CURSE,
	SC_GVG_SILENCE,
	SC_GVG_BLIND,

	SC_CLAN_INFO,
	SC_SWORDCLAN,
	SC_ARCWANDCLAN,
	SC_GOLDENMACECLAN,
	SC_CROSSBOWCLAN,
	SC_JUMPINGCLAN,

	SC_TAROTCARD,

	// Geffen Magic Tournament Buffs
	SC_GEFFEN_MAGIC1,
	SC_GEFFEN_MAGIC2,
	SC_GEFFEN_MAGIC3,

	SC_MAXPAIN,
	SC_ARMOR_ELEMENT_EARTH,
	SC_ARMOR_ELEMENT_FIRE,
	SC_ARMOR_ELEMENT_WIND,

	SC_DAILYSENDMAILCNT,

	SC_DORAM_BUF_01,
	SC_DORAM_BUF_02,

	/**
	 * Summoner - Extended
	 */
	SC_HISS,
	SC_NYANGGRASS,
	SC_GROOMING,
	SC_SHRIMPBLESSING,
	SC_CHATTERING,
	SC_DORAM_WALKSPEED,
	SC_DORAM_MATK,
	SC_DORAM_FLEE2,
	SC_DORAM_SVSP,

	SC_FALLEN_ANGEL,

	SC_CHEERUP,
	SC_DRESSUP,

	// Old Glast Heim Buffs
	SC_GLASTHEIM_ATK,
	SC_GLASTHEIM_DEF,
	SC_GLASTHEIM_HEAL,
	SC_GLASTHEIM_HIDDEN,
	SC_GLASTHEIM_STATE,
	SC_GLASTHEIM_ITEMDEF,
	SC_GLASTHEIM_HPSP,

	// Nightmare Biolab Buffs
	SC_LHZ_DUN_N1,
	SC_LHZ_DUN_N2,
	SC_LHZ_DUN_N3,
	SC_LHZ_DUN_N4,

	SC_ANCILLA,
	SC_EARTHSHAKER,
	SC_WEAPONBLOCK_ON,
	SC_SPORE_EXPLOSION,

	SC_ENTRY_QUEUE_APPLY_DELAY,
	SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT,

#ifdef RENEWAL
	SC_EXTREMITYFIST2, //! NOTE: This SC should be right before SC_MAX, so it doesn't disturb if RENEWAL is disabled
#endif
	SC_MAX, //Automatically updated max, used in for's to check we are within bounds.
};

/// Official status change ids, used to display status icons on the client.
enum efst_types : short{
/// Do not modify code below this, until the end of the API hook, since it will be automatically generated again
/// @APIHOOK_START(EFST_ENUM)
	EFST_BLANK = -1,

/// @APIHOOK_END
/// Do not modify code above this, since it will be automatically generated by the API again
	EFST_MAX,
};

/// JOINTBEAT stackable ailments
enum e_joint_break : uint8 {
	BREAK_ANKLE = 0x01,		///< MoveSpeed reduced by 50%
	BREAK_WRIST = 0x02,		///< ASPD reduced by 25%
	BREAK_KNEE = 0x04,		///< MoveSpeed reduced by 30%, ASPD reduced by 10%
	BREAK_SHOULDER = 0x08,	///< DEF reduced by 50%
	BREAK_WAIST = 0x10,		///< DEF reduced by 25%, ATK reduced by 25%
	BREAK_NECK = 0x20,		///< Current attack does 2x damage, inflicts 'bleeding' for 30 seconds
	BREAK_FLAGS = BREAK_ANKLE | BREAK_WRIST | BREAK_KNEE | BREAK_SHOULDER | BREAK_WAIST | BREAK_NECK,
};

extern short current_equip_item_index;
extern unsigned int current_equip_combo_pos;
extern int current_equip_card_id;
extern bool running_npc_stat_calc_event;
extern short current_equip_opt_index;

//Status change option definitions (options are what makes status changes visible to chars
//who were not on your field of sight when it happened)

///opt1: (BODYSTATE_*) Non stackable status changes.
enum sc_opt1 {
	OPT1_STONE = 1, //Petrified
	OPT1_FREEZE,
	OPT1_STUN,
	OPT1_SLEEP,
	//Aegis uses OPT1 = 5 to identify undead enemies (which also grants them immunity to the other opt1 changes)
	OPT1_STONEWAIT = 6, //Petrifying
	OPT1_BURNING,
	OPT1_IMPRISON,
};

///opt2: (HEALTHSTATE_*) Stackable status changes.
enum sc_opt2 {
	OPT2_POISON		= 0x0001,
	OPT2_CURSE		= 0x0002,
	OPT2_SILENCE		= 0x0004,
	OPT2_SIGNUMCRUCIS	= 0x0008, //Confusion
	OPT2_BLIND		= 0x0010,
	OPT2_ANGELUS		= 0x0020,
	OPT2_BLEEDING		= 0x0040,
	OPT2_DPOISON		= 0x0080,
	OPT2_FEAR		= 0x0100,
};

///opt3: (SHOW_EFST_*)
enum sc_opt3 {
	OPT3_NORMAL		= 0x00000000,
	OPT3_QUICKEN		= 0x00000001,
	OPT3_OVERTHRUST		= 0x00000002,
	OPT3_ENERGYCOAT		= 0x00000004,
	OPT3_EXPLOSIONSPIRITS	= 0x00000008,
	OPT3_STEELBODY		= 0x00000010,
	OPT3_BLADESTOP		= 0x00000020,
	OPT3_AURABLADE		= 0x00000040,
	OPT3_BERSERK		= 0x00000080,
	OPT3_LIGHTBLADE		= 0x00000100,
	OPT3_MOONLIT		= 0x00000200,
	OPT3_MARIONETTE		= 0x00000400,
	OPT3_ASSUMPTIO		= 0x00000800,
	OPT3_WARM		= 0x00001000,
	OPT3_KAITE		= 0x00002000,
	OPT3_BUNSIN		= 0x00004000,
	OPT3_SOULLINK		= 0x00008000,
	OPT3_UNDEAD		= 0x00010000,
	OPT3_CONTRACT		= 0x00020000,
};

///Option (EFFECTSTATE_*)
enum e_option {
	OPTION_NOTHING		= 0x00000000,
	OPTION_SIGHT		= 0x00000001,
	OPTION_HIDE			= 0x00000002,
	OPTION_CLOAK		= 0x00000004,
	OPTION_CART1		= 0x00000008,
	OPTION_FALCON		= 0x00000010,
	OPTION_RIDING		= 0x00000020,
	OPTION_INVISIBLE	= 0x00000040,
	OPTION_CART2		= 0x00000080,
	OPTION_CART3		= 0x00000100,
	OPTION_CART4		= 0x00000200,
	OPTION_CART5		= 0x00000400,
	OPTION_ORCISH		= 0x00000800,
	OPTION_WEDDING		= 0x00001000,
	OPTION_RUWACH		= 0x00002000,
	OPTION_CHASEWALK	= 0x00004000,
	OPTION_FLYING		= 0x00008000, //! NOTE: That clientside Flying and Xmas are 0x8000 for clients prior to 2007.
	OPTION_XMAS			= 0x00010000,
	OPTION_TRANSFORM	= 0x00020000,
	OPTION_SUMMER		= 0x00040000,
	OPTION_DRAGON1		= 0x00080000,
	OPTION_WUG			= 0x00100000,
	OPTION_WUGRIDER		= 0x00200000,
	OPTION_MADOGEAR		= 0x00400000,
	OPTION_DRAGON2		= 0x00800000,
	OPTION_DRAGON3		= 0x01000000,
	OPTION_DRAGON4		= 0x02000000,
	OPTION_DRAGON5		= 0x04000000,
	OPTION_HANBOK		= 0x08000000,
	OPTION_OKTOBERFEST	= 0x10000000,
	OPTION_SUMMER2		= 0x20000000,

	// compound constant for older carts
	OPTION_CART	= OPTION_CART1|OPTION_CART2|OPTION_CART3|OPTION_CART4|OPTION_CART5,

	// compound constants
	OPTION_DRAGON	= OPTION_DRAGON1|OPTION_DRAGON2|OPTION_DRAGON3|OPTION_DRAGON4|OPTION_DRAGON5,
	OPTION_COSTUME	= OPTION_WEDDING|OPTION_XMAS|OPTION_SUMMER|OPTION_HANBOK|OPTION_OKTOBERFEST|OPTION_SUMMER2,
};

///Defines for the manner system [Skotlex]
enum manner_flags
{
	MANNER_NOCHAT		= 0x01,
	MANNER_NOSKILL		= 0x02,
	MANNER_NOCOMMAND	= 0x04,
	MANNER_NOITEM		= 0x08,
	MANNER_NOROOM		= 0x10,
};

/// Status Change State Flags
enum scs_flag {
	SCS_NOMOVECOND		= 0x00000001, ///< cond flag for nomove
	SCS_NOMOVE			= 0x00000002, ///< unit unable to move
	SCS_NOPICKITEMCOND	= 0x00000004, ///< cond flag for nopickitem
	SCS_NOPICKITEM		= 0x00000008, ///< player unable to pick up items
	SCS_NODROPITEMCOND	= 0x00000010, ///< cond flag for nodropitem
	SCS_NODROPITEM		= 0x00000020, ///< player unable to drop items
	SCS_NOCASTCOND		= 0x00000040, ///< cond flag for nocast
	SCS_NOCAST			= 0x00000080, ///< unit unable to cast skills
	SCS_NOCHAT			= 0x00000100, ///< unit can't talk
	SCS_NOCHATCOND		= 0x00000200, ///< cond flag for notalk
};

///Define flags for the status_calc_bl function. [Skotlex]
enum scb_flag
{
	SCB_NONE	= 0x00000000,
	SCB_BASE	= 0x00000001,
	SCB_MAXHP	= 0x00000002,
	SCB_MAXSP	= 0x00000004,
	SCB_STR		= 0x00000008,
	SCB_AGI		= 0x00000010,
	SCB_VIT		= 0x00000020,
	SCB_INT		= 0x00000040,
	SCB_DEX		= 0x00000080,
	SCB_LUK		= 0x00000100,
	SCB_BATK	= 0x00000200,
	SCB_WATK	= 0x00000400,
	SCB_MATK	= 0x00000800,
	SCB_HIT		= 0x00001000,
	SCB_FLEE	= 0x00002000,
	SCB_DEF		= 0x00004000,
	SCB_DEF2	= 0x00008000,
	SCB_MDEF	= 0x00010000,
	SCB_MDEF2	= 0x00020000,
	SCB_SPEED	= 0x00040000,
	SCB_ASPD	= 0x00080000,
	SCB_DSPD	= 0x00100000,
	SCB_CRI		= 0x00200000,
	SCB_FLEE2	= 0x00400000,
	SCB_ATK_ELE	= 0x00800000,
	SCB_DEF_ELE	= 0x01000000,
	SCB_MODE	= 0x02000000,
	SCB_SIZE	= 0x04000000,
	SCB_RACE	= 0x08000000,
	SCB_RANGE	= 0x10000000,
	SCB_REGEN	= 0x20000000,
	SCB_DYE		= 0x40000000, // force cloth-dye change to 0 to avoid client crashes.

	SCB_BATTLE	= 0x3FFFFFFE,
	SCB_ALL		= 0x3FFFFFFF
};

enum e_status_calc_opt {
	SCO_NONE  = 0x0,
	SCO_FIRST = 0x1, ///< Trigger the calculations that should take place only onspawn/once, process base status initialization code
	SCO_FORCE = 0x2, ///< Only relevant to BL_PC types, ensures call bypasses the queue caused by delayed damage
};

/// Flags for status_change_start and status_get_sc_def
enum e_status_change_start_flags {
	SCSTART_NONE       = 0x00,
	SCSTART_NOAVOID    = 0x01, /// Cannot be avoided (it has to start)
	SCSTART_NOTICKDEF  = 0x02, /// Tick should not be reduced (by statuses or bonuses)
	SCSTART_LOADED     = 0x04, /// When sc_data loaded (fetched from table), no values (val1 ~ val4) have to be altered/recalculate
	SCSTART_NORATEDEF  = 0x08, /// Rate should not be reduced (by statuses or bonuses)
	SCSTART_NOICON     = 0x10, /// Status icon won't be sent to client
};

/// Enum for status_change_clear_buffs
enum e_status_change_clear_buffs_flags {
	SCCB_BUFFS        = 0x01,
	SCCB_DEBUFFS      = 0x02,
	SCCB_REFRESH      = 0x04,
	SCCB_CHEM_PROTECT = 0x08,
	SCCB_LUXANIMA     = 0x10,
};

///Enum for bonus_script's flag [Cydh]
enum e_bonus_script_flags {
	BSF_REM_ON_DEAD				= 0x001, ///< Removed when dead
	BSF_REM_ON_DISPELL			= 0x002, ///< Removed by Dispell
	BSF_REM_ON_CLEARANCE		= 0x004, ///< Removed by Clearance
	BSF_REM_ON_LOGOUT			= 0x008, ///< Removed when player logged out
	BSF_REM_ON_BANISHING_BUSTER	= 0x010, ///< Removed by Banishing Buster
	BSF_REM_ON_REFRESH			= 0x020, ///< Removed by Refresh
	BSF_REM_ON_LUXANIMA			= 0x040, ///< Removed by Luxanima
	BSF_REM_ON_MADOGEAR			= 0x080, ///< Removed when Madogear is activated or deactivated
	BSF_REM_ON_DAMAGED			= 0x100, ///< Removed when receive damage
	BSF_PERMANENT				= 0x200, ///< Cannot be removed by sc_end SC_ALL

	// These flags cannot be stacked, BSF_FORCE_REPLACE has highest priority to check if YOU force to add both
	BSF_FORCE_REPLACE			= 0x400, ///< Force to replace duplicated script by expanding the duration
	BSF_FORCE_DUPLICATE			= 0x800, ///< Force to add duplicated script

	// These flags aren't part of 'bonus_script' scripting flags
	BSF_REM_ALL		= 0x0,		///< Remove all bonus script
	BSF_REM_BUFF	= 0x4000,	///< Remove positive buff if battle_config.debuff_on_logout&1
	BSF_REM_DEBUFF	= 0x8000,	///< Remove negative buff if battle_config.debuff_on_logout&2
};

///Enum for status_get_hpbonus and status_get_spbonus
enum e_status_bonus {
	STATUS_BONUS_FIX = 0,
	STATUS_BONUS_RATE = 1,
};

/// Enum for status_calc_weight and status_calc_cart_weight
enum e_status_calc_weight_opt {
	CALCWT_NONE = 0x0,
	CALCWT_ITEM = 0x1,		///< Recalculate item weight
	CALCWT_MAXBONUS = 0x2,	///< Recalculate max weight based on skill/status/configuration bonuses
	CALCWT_CARTSTATE = 0x4,	///< Whether to check for cart state
};

// Enum for refine chance types
enum e_refine_chance_type {
	REFINE_CHANCE_NORMAL = 0,
	REFINE_CHANCE_ENRICHED,
	REFINE_CHANCE_EVENT_NORMAL,
	REFINE_CHANCE_EVENT_ENRICHED,
	REFINE_CHANCE_TYPE_MAX
};

///Define to determine who gets HP/SP consumed on doing skills/etc. [Skotlex]
#define BL_CONSUME (BL_PC|BL_HOM|BL_MER|BL_ELEM)
///Define to determine who has regen
#define BL_REGEN (BL_PC|BL_HOM|BL_MER|BL_ELEM)
///Define to determine who will receive a clif_status_change packet for effects that require one to display correctly
#define BL_SCEFFECT (BL_PC|BL_HOM|BL_MER|BL_MOB|BL_ELEM)

/** Basic damage info of a weapon
* Required because players have two of these, one in status_data
* and another for their left hand weapon. */
struct weapon_atk {
	unsigned short atk, atk2;
	unsigned short range;
	unsigned char ele;
#ifdef RENEWAL
	unsigned short matk;
	unsigned char wlv;
#endif
};

extern sc_type SkillStatusChangeTable[MAX_SKILL];   /// skill  -> status
extern int StatusIconChangeTable[SC_MAX];           /// status -> "icon" (icon is a bit of a misnomer, since there exist values with no icon associated)
extern unsigned int StatusChangeFlagTable[SC_MAX];  /// status -> flags
extern int StatusSkillChangeTable[SC_MAX];          /// status -> skill
extern int StatusRelevantBLTypes[EFST_MAX];           /// "icon" -> enum bl_type (for clif->status_change to identify for which bl types to send packets)
extern unsigned int StatusChangeStateTable[SC_MAX]; /// status -> flags
extern unsigned int StatusDisplayType[SC_MAX];

///For holding basic status (which can be modified by status changes)
struct status_data {
	unsigned int
		hp, sp,  // see status_cpy before adding members before hp and sp
		max_hp, max_sp;
	short
		str, agi, vit, int_, dex, luk,
		eatk;
	unsigned short
		batk,
#ifdef RENEWAL
		watk,
		watk2,
#endif
		matk_min, matk_max,
		speed,
		amotion, adelay, dmotion;
	enum e_mode mode;
	short
		hit, flee, cri, flee2,
		def2, mdef2,
#ifdef RENEWAL_ASPD
		aspd_rate2,
#endif
		aspd_rate;
	/**
	 * defType is RENEWAL dependent and defined in src/config/const.hpp
	 **/
	defType def,mdef;

	unsigned char
		def_ele, ele_lv,
		size,
		race, /// see enum e_race
		class_; /// see enum e_classAE

	struct weapon_atk rhw, lhw; //Right Hand/Left Hand Weapon.
};

///Additional regen data that only players have.
struct regen_data_sub {
	unsigned short
		hp,sp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp;
	} tick;

	//Regen rates. n/100
	struct {
		unsigned short hp,sp;
	} rate;
};

///Regen data
struct regen_data {
	unsigned char flag; //Marks what stuff you may heal or not.
	unsigned short hp,sp,shp,ssp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp,shp,ssp;
	} tick;

	//Regen rates. n/100
	struct {
		unsigned short hp, sp, shp, ssp;
	} rate;

	struct {
		unsigned walk:1; //Can you regen even when walking?
		unsigned gc:1;	//Tags when you should have double regen due to GVG castle
		unsigned overweight :2; //overweight state (1: 50%, 2: 90%)
		unsigned block :2; //Block regen flag (1: Hp, 2: Sp)
	} state;

	//skill-regen, sitting-skill-regen (since not all chars with regen need it)
	struct regen_data_sub *sregen, *ssregen;
};

///Status display entry
struct sc_display_entry {
	enum sc_type type;
	int val1, val2, val3;
};

///Status change entry
struct status_change_entry {
	int timer;
	int val1,val2,val3,val4;
};

///Status change
struct status_change {
	unsigned int option;// effect state (bitfield)
	unsigned int opt3;// skill state (bitfield)
	unsigned short opt1;// body state
	unsigned short opt2;// health state (bitfield)
	unsigned char count;
	//! TODO: See if it is possible to implement the following SC's without requiring extra parameters while the SC is inactive.
	struct {
		unsigned char move;
		unsigned char pickup;
		unsigned char drop;
		unsigned char cast;
		unsigned char chat;
	} cant;/* status change state flags */
	//int sg_id; //ID of the previous Storm gust that hit you
	short comet_x, comet_y; // Point where src casted Comet - required to calculate damage from this point
/**
 * The Storm Gust counter was dropped in renewal
 **/
#ifndef RENEWAL
	unsigned char sg_counter; //Storm gust counter (previous hits from storm gust)
#endif
	unsigned char bs_counter; // Blood Sucker counter
	struct status_change_entry *data[SC_MAX];
};

// for looking up associated data
sc_type status_skill2sc(int skill);
int status_sc2skill(sc_type sc);
unsigned int status_sc2scb_flag(sc_type sc);
int status_type2relevant_bl_types(int type);

int status_damage(struct block_list *src,struct block_list *target,int64 dhp,int64 dsp, t_tick walkdelay, int flag);
//Define for standard HP damage attacks.
#define status_fix_damage(src, target, hp, walkdelay) status_damage(src, target, hp, 0, walkdelay, 0)
//Define for standard SP damage attacks.
#define status_fix_spdamage(src, target, sp, walkdelay) status_damage(src, target, 0, sp, walkdelay, 0)
//Define for standard HP/SP damage triggers.
#define status_zap(bl, hp, sp) status_damage(NULL, bl, hp, sp, 0, 1)
//Define for standard HP/SP skill-related cost triggers (mobs require no HP/SP to use skills)
int64 status_charge(struct block_list* bl, int64 hp, int64 sp);
int status_percent_change(struct block_list *src, struct block_list *target, int8 hp_rate, int8 sp_rate, uint8 flag);
//Easier handling of status_percent_change
#define status_percent_heal(bl, hp_rate, sp_rate) status_percent_change(NULL, bl, -(hp_rate), -(sp_rate), 0)
/// Deals % damage from 'src' to 'target'. If rate is > 0 is % of current HP/SP, < 0 % of MaxHP/MaxSP
#define status_percent_damage(src, target, hp_rate, sp_rate, kill) status_percent_change(src, target, hp_rate, sp_rate, (kill)?1:2)
//Instant kill with no drops/exp/etc
#define status_kill(bl) status_percent_damage(NULL, bl, 100, 0, true)
//Used to set the hp/sp of an object to an absolute value (can't kill)
int status_set_hp(struct block_list *bl, unsigned int hp, int flag);
int status_set_maxhp(struct block_list *bl, unsigned int hp, int flag);
int status_set_sp(struct block_list *bl, unsigned int sp, int flag);
int status_set_maxsp(struct block_list *bl, unsigned int hp, int flag);
int status_heal(struct block_list *bl,int64 hhp,int64 hsp, int flag);
int status_revive(struct block_list *bl, unsigned char per_hp, unsigned char per_sp);

struct regen_data *status_get_regen_data(struct block_list *bl);
struct status_data *status_get_status_data(struct block_list *bl);
struct status_data *status_get_base_status(struct block_list *bl);
const char * status_get_name(struct block_list *bl);
int status_get_class(struct block_list *bl);
int status_get_lv(struct block_list *bl);
#define status_get_range(bl) status_get_status_data(bl)->rhw.range
#define status_get_hp(bl) status_get_status_data(bl)->hp
#define status_get_max_hp(bl) status_get_status_data(bl)->max_hp
#define status_get_sp(bl) status_get_status_data(bl)->sp
#define status_get_max_sp(bl) status_get_status_data(bl)->max_sp
#define status_get_str(bl) status_get_status_data(bl)->str
#define status_get_agi(bl) status_get_status_data(bl)->agi
#define status_get_vit(bl) status_get_status_data(bl)->vit
#define status_get_int(bl) status_get_status_data(bl)->int_
#define status_get_dex(bl) status_get_status_data(bl)->dex
#define status_get_luk(bl) status_get_status_data(bl)->luk
#define status_get_hit(bl) status_get_status_data(bl)->hit
#define status_get_flee(bl) status_get_status_data(bl)->flee
defType status_get_def(struct block_list *bl);
#define status_get_mdef(bl) status_get_status_data(bl)->mdef
#define status_get_flee2(bl) status_get_status_data(bl)->flee2
#define status_get_def2(bl) status_get_status_data(bl)->def2
#define status_get_mdef2(bl) status_get_status_data(bl)->mdef2
#define status_get_critical(bl)  status_get_status_data(bl)->cri
#define status_get_batk(bl) status_get_status_data(bl)->batk
#define status_get_watk(bl) status_get_status_data(bl)->rhw.atk
#define status_get_watk2(bl) status_get_status_data(bl)->rhw.atk2
#define status_get_matk_max(bl) status_get_status_data(bl)->matk_max
#define status_get_matk_min(bl) status_get_status_data(bl)->matk_min
#define status_get_lwatk(bl) status_get_status_data(bl)->lhw.atk
#define status_get_lwatk2(bl) status_get_status_data(bl)->lhw.atk2
unsigned short status_get_speed(struct block_list *bl);
#define status_get_adelay(bl) status_get_status_data(bl)->adelay
#define status_get_amotion(bl) status_get_status_data(bl)->amotion
#define status_get_dmotion(bl) status_get_status_data(bl)->dmotion
#define status_get_element(bl) status_get_status_data(bl)->def_ele
#define status_get_element_level(bl) status_get_status_data(bl)->ele_lv
unsigned char status_calc_attack_element(struct block_list *bl, struct status_change *sc, int element);
#define status_get_attack_sc_element(bl, sc) status_calc_attack_element(bl, sc, 0)
#define status_get_attack_element(bl) status_get_status_data(bl)->rhw.ele
#define status_get_attack_lelement(bl) status_get_status_data(bl)->lhw.ele
#define status_get_race(bl) status_get_status_data(bl)->race
#define status_get_class_(bl) status_get_status_data(bl)->class_
#define status_get_size(bl) status_get_status_data(bl)->size
#define status_get_mode(bl) status_get_status_data(bl)->mode
#define status_has_mode(status,md) (((status)->mode&(md)) == (md))
#define status_bl_has_mode(bl,md) status_has_mode(status_get_status_data((bl)),(md))

#define status_get_homstr(bl) (status->str + ((TBL_HOM*)bl)->homunculus.str_value)
#define status_get_homagi(bl) (status->agi + ((TBL_HOM*)bl)->homunculus.agi_value)
#define status_get_homvit(bl) (status->vit + ((TBL_HOM*)bl)->homunculus.vit_value)
#define status_get_homint(bl) (status->int_ + ((TBL_HOM*)bl)->homunculus.int_value)
#define status_get_homdex(bl) (status->dex + ((TBL_HOM*)bl)->homunculus.dex_value)
#define status_get_homluk(bl) (status->luk + ((TBL_HOM*)bl)->homunculus.luk_value)

int status_get_party_id(struct block_list *bl);
int status_get_guild_id(struct block_list *bl);
int status_get_emblem_id(struct block_list *bl);
enum e_race2 status_get_race2(struct block_list *bl);

struct view_data *status_get_viewdata(struct block_list *bl);
void status_set_viewdata(struct block_list *bl, int class_);
void status_change_init(struct block_list *bl);
struct status_change *status_get_sc(struct block_list *bl);

int status_isdead(struct block_list *bl);
int status_isimmune(struct block_list *bl);

t_tick status_get_sc_def(struct block_list *src,struct block_list *bl, enum sc_type type, int rate, t_tick tick, unsigned char flag);
//Short version, receives rate in 1->100 range, and does not uses a flag setting.
#define sc_start(src, bl, type, rate, val1, tick) status_change_start(src,bl,type,100*(rate),val1,0,0,0,tick,SCSTART_NONE)
#define sc_start2(src, bl, type, rate, val1, val2, tick) status_change_start(src,bl,type,100*(rate),val1,val2,0,0,tick,SCSTART_NONE)
#define sc_start4(src, bl, type, rate, val1, val2, val3, val4, tick) status_change_start(src,bl,type,100*(rate),val1,val2,val3,val4,tick,SCSTART_NONE)

int status_change_start(struct block_list* src, struct block_list* bl,enum sc_type type,int rate,int val1,int val2,int val3,int val4,t_tick duration,unsigned char flag);
int status_change_end_(struct block_list* bl, enum sc_type type, int tid, const char* file, int line);
#define status_change_end(bl,type,tid) status_change_end_(bl,type,tid,__FILE__,__LINE__)
TIMER_FUNC(status_change_timer);
int status_change_timer_sub(struct block_list* bl, va_list ap);
int status_change_clear(struct block_list* bl, int type);
void status_change_clear_buffs(struct block_list* bl, uint8 type);
void status_change_clear_onChangeMap(struct block_list *bl, struct status_change *sc);

#define status_calc_bl(bl, flag) status_calc_bl_(bl, (enum scb_flag)(flag), SCO_NONE)
#define status_calc_mob(md, opt) status_calc_bl_(&(md)->bl, SCB_ALL, opt)
#define status_calc_pet(pd, opt) status_calc_bl_(&(pd)->bl, SCB_ALL, opt)
#define status_calc_pc(sd, opt) status_calc_bl_(&(sd)->bl, SCB_ALL, opt)
#define status_calc_homunculus(hd, opt) status_calc_bl_(&(hd)->bl, SCB_ALL, opt)
#define status_calc_mercenary(md, opt) status_calc_bl_(&(md)->bl, SCB_ALL, opt)
#define status_calc_elemental(ed, opt) status_calc_bl_(&(ed)->bl, SCB_ALL, opt)
#define status_calc_npc(nd, opt) status_calc_bl_(&(nd)->bl, SCB_ALL, opt)

bool status_calc_weight(struct map_session_data *sd, enum e_status_calc_weight_opt flag);
bool status_calc_cart_weight(struct map_session_data *sd, enum e_status_calc_weight_opt flag);
void status_calc_bl_(struct block_list *bl, enum scb_flag flag, enum e_status_calc_opt opt);
int status_calc_mob_(struct mob_data* md, enum e_status_calc_opt opt);
void status_calc_pet_(struct pet_data* pd, enum e_status_calc_opt opt);
int status_calc_pc_(struct map_session_data* sd, enum e_status_calc_opt opt);
int status_calc_homunculus_(struct homun_data *hd, enum e_status_calc_opt opt);
int status_calc_mercenary_(struct mercenary_data *md, enum e_status_calc_opt opt);
int status_calc_elemental_(struct elemental_data *ed, enum e_status_calc_opt opt);
int status_calc_npc_(struct npc_data *nd, enum e_status_calc_opt opt);

void status_calc_misc(struct block_list *bl, struct status_data *status, int level);
void status_calc_regen(struct block_list *bl, struct status_data *status, struct regen_data *regen);
void status_calc_regen_rate(struct block_list *bl, struct regen_data *regen, struct status_change *sc);

void status_calc_slave_mode(struct mob_data *md, struct mob_data *mmd);

bool status_check_skilluse(struct block_list *src, struct block_list *target, uint16 skill_id, int flag);
int status_check_visibility(struct block_list *src, struct block_list *target);

int status_change_spread(struct block_list *src, struct block_list *bl, bool type);

#ifndef RENEWAL
unsigned short status_base_matk_min(const struct status_data* status);
unsigned short status_base_matk_max(const struct status_data* status);
#else
unsigned int status_weapon_atk(struct weapon_atk wa, struct map_session_data *sd);
unsigned short status_base_atk_min(struct block_list *bl, const struct status_data* status, int level);
unsigned short status_base_atk_max(struct block_list *bl, const struct status_data* status, int level);
unsigned short status_base_matk_min(struct block_list *bl, const struct status_data* status, int level);
unsigned short status_base_matk_max(struct block_list *bl, const struct status_data* status, int level);
#endif

unsigned short status_base_atk(const struct block_list *bl, const struct status_data *status, int level);

void initChangeTables(void);
int status_readdb(void);
int do_init_status(void);
void do_final_status(void);

#endif /* STATUS_HPP */
