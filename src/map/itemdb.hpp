// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ITEMDB_HPP_
#define _ITEMDB_HPP_

#include "../common/db.h"
#include "../common/mmo.h" // ITEM_NAME_LENGTH

///Maximum allowed Item ID (range: 1 ~ 65,534)
#define MAX_ITEMID USHRT_MAX
///Use apple for unknown items.
#define UNKNOWN_ITEM_ID 512
/// The maximum number of item delays
#define MAX_ITEMDELAYS	10
///Designed for search functions, species max number of matches to display.
#define MAX_SEARCH	5
///Maximum amount of items a combo may require
#define MAX_ITEMS_PER_COMBO 6

#define MAX_ITEMGROUP_RANDGROUP 4	///Max group for random item (increase this when needed). TODO: Remove this limit and use dynamic size if needed

#define MAX_ROULETTE_LEVEL 7 /** client-defined value **/
#define MAX_ROULETTE_COLUMNS 9 /** client-defined value **/

#define CARD0_FORGE 0x00FF
#define CARD0_CREATE 0x00FE
#define CARD0_PET 0x0100

///Marks if the card0 given is "special" (non-item id used to mark pets/created items. [Skotlex]
#define itemdb_isspecial(i) (i == CARD0_FORGE || i == CARD0_CREATE || i == CARD0_PET)

///Enum of item id (for hardcoded purpose)
enum item_itemid
{
	ITEMID_RED_POTION					= 501,
	ITEMID_YELLOW_POTION				= 503,
	ITEMID_WHITE_POTION					= 504,
	ITEMID_BLUE_POTION					= 505,
	ITEMID_APPLE						= 512,
	ITEMID_CARROT						= 515,
	ITEMID_HOLY_WATER					= 523,
	ITEMID_PUMPKIN						= 535,
	ITEMID_RED_SLIM_POTION				= 545,
	ITEMID_YELLOW_SLIM_POTION			= 546,
	ITEMID_WHITE_SLIM_POTION			= 547,
	ITEMID_WING_OF_FLY					= 601,
	ITEMID_WING_OF_BUTTERFLY			= 602,
	ITEMID_ANODYNE						= 605,
	ITEMID_ALOEBERA						= 606,
	ITEMID_MAGNIFIER					= 611,
	ITEMID_POISON_BOTTLE				= 678,
	ITEMID_EMPTY_BOTTLE					= 713,
	ITEMID_EMPERIUM						= 714,
	ITEMID_RED_GEMSTONE					= 716,
	ITEMID_BLUE_GEMSTONE				= 717,
	ITEMID_ORIDECON_STONE				= 756,
	ITEMID_ALCOHOL						= 970,
	ITEMID_ORIDECON						= 984,
	ITEMID_ANVIL						= 986,
	ITEMID_ORIDECON_ANVIL				= 987,
	ITEMID_GOLDEN_ANVIL					= 988,
	ITEMID_EMPERIUM_ANVIL				= 989,
	ITEMID_BLOODY_RED					= 990,
	ITEMID_CRYSTAL_BLUE					= 991,
	ITEMID_WIND_OF_VERDURE				= 992,
	ITEMID_YELLOW_LIVE					= 993,
	ITEMID_FLAME_HEART					= 994,
	ITEMID_MISTIC_FROZEN				= 995,
	ITEMID_ROUGH_WIND					= 996,
	ITEMID_GREAT_NATURE					= 997,
	ITEMID_IRON							= 998,
	ITEMID_STEEL						= 999,
	ITEMID_STAR_CRUMB					= 1000,
	ITEMID_IRON_ORE						= 1002,
	ITEMID_PHRACON						= 1010,
	ITEMID_EMVERETARCON					= 1011,
	ITEMID_TRAP							= 1065,
	ITEMID_ANGRA_MANYU					= 1599,
	ITEMID_PAINT_BRUSH					= 6122,
	ITEMID_MAGIC_GEAR_FUEL				= 6146,
	ITEMID_STRANGE_EMBRYO				= 6415,
	ITEMID_STONE						= 7049,
	ITEMID_FIRE_BOTTLE					= 7135,
	ITEMID_ACID_BOTTLE					= 7136,
	ITEMID_MAN_EATER_BOTTLE				= 7137,
	ITEMID_MINI_BOTTLE					= 7138,
	ITEMID_COATING_BOTTLE				= 7139,
	ITEMID_FRAGMENT_OF_CRYSTAL			= 7321,
	ITEMID_SKULL_						= 7420,
	ITEMID_TOKEN_OF_SIEGFRIED			= 7621,
	ITEMID_TRAP_ALLOY					= 7940,
	ITEMID_CATNIP_FRUIT					= 11602,
	ITEMID_MERCENARY_RED_POTION			= 12184,
	ITEMID_MERCENARY_BLUE_POTION		= 12185,
	ITEMID_GIANT_FLY_WING				= 12212,
	ITEMID_NEURALIZER					= 12213,
	ITEMID_M_CENTER_POTION				= 12241,
	ITEMID_M_AWAKENING_POTION			= 12242,
	ITEMID_M_BERSERK_POTION				= 12243,
	ITEMID_N_FLY_WING					= 12323,
	ITEMID_N_BUTTERFLY_WING				= 12324,
	ITEMID_NOVICE_MAGNIFIER				= 12325,
	ITEMID_ANCILLA						= 12333,
	ITEMID_DUN_TELE_SCROLL3				= 12352,
	ITEMID_REINS_OF_MOUNT				= 12622,
	ITEMID_NOBLE_NAMEPLATE				= 12705,
	ITEMID_SILVER_BULLET				= 13201,
	ITEMID_SLUG_AMMUNITION_L			= 13210,
	ITEMID_SLUG_AMMUNITION_M			= 13211,
	ITEMID_SLUG_AMMUNITION_H			= 13212,
	ITEMID_SLUG_AMMUNITION_SH			= 13213,
	ITEMID_SLUG_AMMUNITION_XH			= 13214,
	ITEMID_PURIFICATION_BULLET			= 13220,
	ITEMID_SILVER_BULLET_				= 13221,
	ITEMID_DUN_TELE_SCROLL1				= 14527,
	ITEMID_DUN_TELE_SCROLL2				= 14581,
	ITEMID_WOB_RUNE						= 14582,
	ITEMID_WOB_SCHWALTZ					= 14583,
	ITEMID_WOB_RACHEL					= 14584,
	ITEMID_WOB_LOCAL					= 14585,
	ITEMID_SIEGE_TELEPORT_SCROLL		= 14591,
};

///Rune Knight
enum rune_item_list
{
	ITEMID_NAUTHIZ		= 12725,
	ITEMID_RAIDO,
	ITEMID_BERKANA,
	ITEMID_ISA,
	ITEMID_OTHILA,
	ITEMID_URUZ,
	ITEMID_THURISAZ,
	ITEMID_WYRD,
	ITEMID_HAGALAZ,
	ITEMID_LUX_ANIMA	= 22540,
};

///Mechanic
enum mechanic_item_list
{
	ITEMID_ACCELERATOR				= 2800,
	ITEMID_HOVERING_BOOSTER,
	ITEMID_SUICIDAL_DEVICE,
	ITEMID_SHAPE_SHIFTER,
	ITEMID_COOLING_DEVICE,
	ITEMID_MAGNETIC_FIELD_GENERATOR,
	ITEMID_BARRIER_BUILDER,
	ITEMID_REPAIR_KIT,
	ITEMID_CAMOUFLAGE_GENERATOR,
	ITEMID_HIGH_QUALITY_COOLER,
	ITEMID_SPECIAL_COOLER,
	ITEMID_SCARLET_PTS				= 6360,
	ITEMID_INDIGO_PTS,
	ITEMID_YELLOW_WISH_PTS,
	ITEMID_LIME_GREEN_PTS,
	ITEMID_REPAIR_A              = 12392,
	ITEMID_REPAIR_B,
	ITEMID_REPAIR_C,
};

///Genetic
enum genetic_item_list
{
	ITEMID_SEED_OF_HORNY_PLANT			= 6210,
	ITEMID_BLOODSUCK_PLANT_SEED,
	ITEMID_BOMB_MUSHROOM_SPORE,
	ITEMID_HP_INCREASE_POTION_SMALL		= 12422,
	ITEMID_HP_INCREASE_POTION_MEDIUM,
	ITEMID_HP_INCREASE_POTION_LARGE,
	ITEMID_SP_INCREASE_POTION_SMALL,
	ITEMID_SP_INCREASE_POTION_MEDIUM,
	ITEMID_SP_INCREASE_POTION_LARGE,
	ITEMID_CONCENTRATED_WHITE_POTION_Z,
	ITEMID_SAVAGE_FULL_ROAST,
	ITEMID_COCKTAIL_WARG_BLOOD,
	ITEMID_MINOR_STEW,
	ITEMID_SIROMA_ICED_TEA,
	ITEMID_DROSERA_HERB_SALAD,
	ITEMID_PETITE_TAIL_NOODLES,
	ITEMID_BLACK_MASS,
	ITEMID_VITATA500,
	ITEMID_CONCENTRATED_CEROMAIN_SOUP,
	ITEMID_CURE_FREE					= 12475,
	ITEMID_APPLE_BOMB					= 13260,
	ITEMID_COCONUT_BOMB,
	ITEMID_MELON_BOMB,
	ITEMID_PINEAPPLE_BOMB,
	ITEMID_BANANA_BOMB,
	ITEMID_BLACK_LUMP,
	ITEMID_BLACK_HARD_LUMP,
	ITEMID_VERY_HARD_LUMP,
	ITEMID_MYSTERIOUS_POWDER,
};

///Guillotine Cross
enum poison_item_list
{
	ITEMID_PARALYSE = 12717,
	ITEMID_LEECHESEND,
	ITEMID_OBLIVIONCURSE,
	ITEMID_DEATHHURT,
	ITEMID_TOXIN,
	ITEMID_PYREXIA,
	ITEMID_MAGICMUSHROOM,
	ITEMID_VENOMBLEED,
};

///Item No Use List
enum item_nouse_list
{
	NOUSE_SITTING = 0x01,
};

///Item job
enum e_item_job
{
	ITEMJ_NORMAL      = 0x01,
	ITEMJ_UPPER       = 0x02,
	ITEMJ_BABY        = 0x04,
	ITEMJ_THIRD       = 0x08,
	ITEMJ_THIRD_TRANS = 0x10,
	ITEMJ_THIRD_BABY  = 0x20,
};

enum e_item_ammo
{
	AMMO_ARROW = 1,
	AMMO_THROWABLE_DAGGER,
	AMMO_BULLET,
	AMMO_SHELL,
	AMMO_GRENADE,
	AMMO_SHURIKEN,
	AMMO_KUNAI,
	AMMO_CANNONBALL,
	AMMO_THROWABLE_ITEM, ///Sling items

	MAX_AMMO_TYPE,
};

#define AMMO_TYPE_ALL ((1<<MAX_AMMO_TYPE)-1)

enum e_random_item_group {
	IG_BLUEBOX = 1,
	IG_VIOLETBOX,
	IG_CARDALBUM,
	IG_GIFTBOX,
	IG_SCROLLBOX,
	IG_FINDINGORE,
	IG_COOKIEBAG,
	IG_FIRSTAID,
	IG_HERB,
	IG_FRUIT,
	IG_MEAT,
	IG_CANDY,
	IG_JUICE,
	IG_FISH,
	IG_BOX,
	IG_GEMSTONE,
	IG_RESIST,
	IG_ORE,
	IG_FOOD,
	IG_RECOVERY,
	IG_MINERAL,
	IG_TAMING,
	IG_SCROLL,
	IG_QUIVER,
	IG_MASK,
	IG_ACCESORY,
	IG_JEWEL,
	IG_GIFTBOX_1,
	IG_GIFTBOX_2,
	IG_GIFTBOX_3,
	IG_GIFTBOX_4,
	IG_EGGBOY,
	IG_EGGGIRL,
	IG_GIFTBOXCHINA,
	IG_LOTTOBOX,
	IG_FOODBAG,
	IG_POTION,
	IG_REDBOX_2,
	IG_BLEUBOX,
	IG_REDBOX,
	IG_GREENBOX,
	IG_YELLOWBOX,
	IG_OLDGIFTBOX,
	IG_MAGICCARDALBUM,
	IG_HOMETOWNGIFT,
	IG_MASQUERADE,
	IG_TRESURE_BOX_WOE,
	IG_MASQUERADE_2,
	IG_EASTER_SCROLL,
	IG_PIERRE_TREASUREBOX,
	IG_CHERISH_BOX,
	IG_CHERISH_BOX_ORI,
	IG_LOUISE_COSTUME_BOX,
	IG_XMAS_GIFT,
	IG_FRUIT_BASKET,
	IG_IMPROVED_COIN_BAG,
	IG_INTERMEDIATE_COIN_BAG,
	IG_MINOR_COIN_BAG,
	IG_S_GRADE_COIN_BAG,
	IG_A_GRADE_COIN_BAG,
	IG_ADVANCED_WEAPONS_BOX,
	IG_SPLENDID_BOX,
	IG_CARDALBUM_ARMOR,
	IG_CARDALBUM_HELM,
	IG_CARDALBUM_ACC,
	IG_CARDALBUM_SHOES,
	IG_CARDALBUM_SHIELD,
	IG_CARDALBUM_WEAPON,
	IG_CARDALBUM_GARMENT,
	IG_FLAMEL_CARD,
	IG_SPECIAL_BOX,
	IG_TRESURE_BOX_WOE_,
	IG_RWC_PARTI_BOX,
	IG_RWC_FINAL_COMP_BOX,
	IG_GIFT_BUNDLE,
	IG_CARACAS_RING_BOX,
	IG_CRUMPLED_PAPER,
	IG_SOLO_GIFT_BASKET,
	IG_COUPLE_EVENT_BASKET,
	IG_GM_WARP_BOX,
	IG_FORTUNE_COOKIE1,
	IG_FORTUNE_COOKIE2,
	IG_FORTUNE_COOKIE3,
	IG_NEW_GIFT_ENVELOPE,
	IG_PASSION_FB_HAT_BOX,
	IG_COOL_FB_HAT_BOX,
	IG_VICTORY_FB_HAT_BOX,
	IG_GLORY_FB_HAT_BOX,
	IG_PASSION_HAT_BOX2,
	IG_COOL_HAT_BOX2,
	IG_VICTORY_HAT_BOX2,
	IG_ASPERSIO_5_SCROLL_BOX,
	IG_PET_EGG_SCROLL_BOX1,
	IG_PET_EGG_SCROLL_BOX2,
	IG_PET_EGG_SCROLL1,
	IG_PET_EGG_SCROLL2,
	IG_PET_EGG_SCROLL_BOX3,
	IG_PET_EGG_SCROLL_BOX4,
	IG_PET_EGG_SCROLL_BOX5,
	IG_PET_EGG_SCROLL3,
	IG_PET_EGG_SCROLL4,
	IG_PET_EGG_SCROLL5,
	IG_INFILTRATOR_BOX,
	IG_MURAMASA_BOX,
	IG_EXCALIBUR_BOX,
	IG_COMBAT_KNIFE_BOX,
	IG_COUNTER_DAGGER_BOX,
	IG_KAISER_KNUCKLE_BOX,
	IG_POLE_AXE_BOX,
	IG_MIGHTY_STAFF_BOX,
	IG_RIGHT_EPSILON_BOX,
	IG_BALISTAR_BOX,
	IG_DIARY_OF_GREAT_SAGE_BOX,
	IG_ASURA_BOX,
	IG_APPLE_OF_ARCHER_BOX,
	IG_BUNNY_BAND_BOX,
	IG_SAHKKAT_BOX,
	IG_LORD_CIRCLET_BOX,
	IG_ELVEN_EARS_BOX,
	IG_STEEL_FLOWER_BOX,
	IG_CRITICAL_RING_BOX,
	IG_EARRING_BOX,
	IG_RING_BOX,
	IG_NECKLACE_BOX,
	IG_GLOVE_BOX,
	IG_BROOCH_BOX,
	IG_ROSARY_BOX,
	IG_SAFETY_RING_BOX,
	IG_VESPER_CORE01_BOX,
	IG_VESPER_CORE02_BOX,
	IG_VESPER_CORE03_BOX,
	IG_VESPER_CORE04_BOX,
	IG_PET_EGG_SCROLL_BOX6,
	IG_PET_EGG_SCROLL_BOX7,
	IG_PET_EGG_SCROLL_BOX8,
	IG_PET_EGG_SCROLL_BOX9,
	IG_PET_EGG_SCROLL_BOX10,
	IG_PET_EGG_SCROLL_BOX11,
	IG_PET_EGG_SCROLL6,
	IG_PET_EGG_SCROLL7,
	IG_PET_EGG_SCROLL8,
	IG_PET_EGG_SCROLL9,
	IG_PET_EGG_SCROLL10,
	IG_PET_EGG_SCROLL11,
	IG_CP_HELM_SCROLL_BOX,
	IG_CP_SHIELD_SCROLL_BOX,
	IG_CP_ARMOR_SCROLL_BOX,
	IG_CP_WEAPON_SCROLL_BOX,
	IG_REPAIR_SCROLL_BOX,
	IG_SUPER_PET_EGG1,
	IG_SUPER_PET_EGG2,
	IG_SUPER_PET_EGG3,
	IG_SUPER_PET_EGG4,
	IG_SUPER_CARD_PET_EGG1,
	IG_SUPER_CARD_PET_EGG2,
	IG_SUPER_CARD_PET_EGG3,
	IG_SUPER_CARD_PET_EGG4,
	IG_VIGORGRA_PACKAGE1,
	IG_VIGORGRA_PACKAGE2,
	IG_VIGORGRA_PACKAGE3,
	IG_VIGORGRA_PACKAGE4,
	IG_VIGORGRA_PACKAGE5,
	IG_VIGORGRA_PACKAGE6,
	IG_VIGORGRA_PACKAGE7,
	IG_VIGORGRA_PACKAGE8,
	IG_VIGORGRA_PACKAGE9,
	IG_VIGORGRA_PACKAGE10,
	IG_VIGORGRA_PACKAGE11,
	IG_VIGORGRA_PACKAGE12,
	IG_PET_EGG_SCROLL12,
	IG_PET_EGG_SCROLL13,
	IG_PET_EGG_SCROLL14,
	IG_SUPER_PET_EGG5,
	IG_SUPER_PET_EGG6,
	IG_SUPER_PET_EGG7,
	IG_SUPER_PET_EGG8,
	IG_PET_EGG_SCROLL_E,
	IG_RAMEN_HAT_BOX,
	IG_MYSTERIOUS_TRAVEL_SACK1,
	IG_MYSTERIOUS_TRAVEL_SACK2,
	IG_MYSTERIOUS_TRAVEL_SACK3,
	IG_MYSTERIOUS_TRAVEL_SACK4,
	IG_MAGICIAN_CARD_BOX,
	IG_ACOLYTE_CARD_BOX,
	IG_ARCHER_CARD_BOX,
	IG_SWORDMAN_CARD_BOX,
	IG_THIEF_CARD_BOX,
	IG_MERCHANT_CARD_BOX,
	IG_HARD_CORE_SET_BOX,
	IG_KITTY_SET_BOX,
	IG_SOFT_CORE_SET_BOX,
	IG_DEVIRUCHI_SET_BOX,
	IG_MVP_HUNT_BOX,
	IG_BREWING_BOX,
	IG_XMAS_PET_SCROLL,
	IG_LUCKY_SCROLL08,
	IG_BR_SWORDPACKAGE,
	IG_BR_MAGEPACKAGE,
	IG_BR_ACOLPACKAGE,
	IG_BR_ARCHERPACKAGE,
	IG_BR_MERPACKAGE,
	IG_BR_THIEFPACKAGE,
	IG_ACIDBOMB_10_BOX,
	IG_BASIC_SIEGE_SUPPLY_BOX,
	IG_ADV_SIEGE_SUPPLY_BOX,
	IG_ELITE_SIEGE_SUPPLY_BOX,
	IG_SAKURA_SCROLL,
	IG_BEHOLDER_RING_BOX,
	IG_HALLOW_RING_BOX,
	IG_CLAMOROUS_RING_BOX,
	IG_CHEMICAL_RING_BOX,
	IG_INSECTICIDE_RING_BOX,
	IG_FISHER_RING_BOX,
	IG_DECUSSATE_RING_BOX,
	IG_BLOODY_RING_BOX,
	IG_SATANIC_RING_BOX,
	IG_DRAGOON_RING_BOX,
	IG_ANGEL_SCROLL,
	IG_DEVIL_SCROLL,
	IG_SURPRISE_SCROLL,
	IG_JULY7_SCROLL,
	IG_BACSOJIN_SCROLL,
	IG_ANIMAL_SCROLL,
	IG_HEART_SCROLL,
	IG_NEW_YEAR_SCROLL,
	IG_VALENTINE_PLEDGE_BOX,
	IG_OX_TAIL_SCROLL,
	IG_BUDDAH_SCROLL,
	IG_EVIL_INCARNATION,
	IG_F_CLOVER_BOX_MOUTH,
	IG_MOUTH_BUBBLE_GUM_BOX,
	IG_F_CLOVER_BOX_MOUTH2,
	IG_F_CLOVER_BOX_MOUTH4,
	IG_BGUM_BOX_IN_MOUTH2,
	IG_BGUM_BOX_IN_MOUTH4,
	IG_TW_OCTOBER_SCROLL,
	IG_MY_SCROLL1,
	IG_TW_NOV_SCROLL,
	IG_MY_SCROLL2,
	IG_PR_RESET_STONE_BOX,
	IG_FPR_RESET_STONE_BOX,
	IG_MAJESTIC_DEVIL_SCROLL,
	IG_LIFE_RIBBON_BOX,
	IG_LIFE_RIBBON_BOX2,
	IG_LIFE_RIBBON_BOX3,
	IG_MAGIC_CANDY_BOX10,
	IG_RWC2010_SUITCASEA,
	IG_RWC2010_SUITCASEB,
	IG_SAGITTARIUS_SCROLL,
	IG_SAGITTARIUS_SCR_BOX,
	IG_SAGITTAR_DIADEM_SCROLL,
	IG_SAGITTAR_DI_SCROLL_BOX,
	IG_CAPRI_CROWN_SCROLL,
	IG_CAPRI_CROWN_SCROLL_BOX,
	IG_CAPRICON_DI_SCROLL,
	IG_CAPRICON_DI_SCROLL_BOX,
	IG_AQUARIUS_DIADEM_SCROLL,
	IG_AQUARIUS_DI_SCROLL_BOX,
	IG_LOVELY_AQUARIUS_SCROLL,
	IG_LOVELY_AQUARIUS_BOX,
	IG_PISCES_DIADEM_SCROLL,
	IG_PISCES_DIADEM_BOX,
	IG_ENERGETIC_PISCES_SCROLL,
	IG_ENERGETIC_PISCES_BOX,
	IG_ARIES_SCROLL,
	IG_ARIES_SCROLL_BOX,
	IG_BOARDING_HALTER_BOX,
	IG_TAURUS_DIADEM_SCROLL,
	IG_TAURUS_DI_SCROLL_BOX,
	IG_UMBALA_SPIRIT_BOX2,
	IG_F_UMBALA_SPIRIT_BOX2,
	IG_TAURUS_CROWN_SCROLL,
	IG_TAURUS_CROWN_SCROLL_BOX,
	IG_GEMI_DIADEM_SCROLL,
	IG_GEMI_DIADEM_SCROLL_BOX,
	IG_SUPER_PET_EGG1_2,
	IG_SUPER_PET_EGG4_2,
	IG_FIRE_BRAND_BOX,
	IG_BR_INDEPENDENCE_SCROLL,
	IG_ALL_IN_ONE_RING_BOX,
	IG_GEMI_CROWN_SCROLL,
	IG_GEMI_CROWN_SCROLL_BOX,
	IG_RWC_SPECIAL_SCROLL,
	IG_RWC_LIMITED_SCROLL,
	IG_ASGARD_SCROLL,
	IG_MS_CANCER_SCROLL,
	IG_RWC_SUPER_SCROLL,
	IG_LEO_SCROLL,
	IG_MS_VIRGO_SCROLL,
	IG_LUCKY_EGG_C6,
	IG_LIBRA_SCROLL,
	IG_HALLO_SCROLL,
	IG_MS_SCORPIO_SCROLL,
	IG_TCG_CARD_SCROLL,
	IG_BOITATA_SCROLL,
	IG_LUCKY_EGG_C2,
	IG_LUCKY_EGG_C6_,
	IG_LUCKY_EGG_C9,
	IG_LUCKY_EGG_C7,
	IG_LUCKY_EGG_C8,
	IG_LUCKY_EGG_C10,
	IG_WIND_TYPE_SCROLL,
	IG_LUCKY_EGG_C3,
	IG_LUCKY_EGG_C4,
	IG_LUCKY_EGG_C5,
	IG_WEATHER_REPORT_BOX,
	IG_COMIN_ACTOR_BOX,
	IG_HEN_SET_BOX,
	IG_LUCKY_EGG_C,
	IG_WATER_TYPE_SCROLL,
	IG_EARTH_TYPE_SCROLL,
	IG_EARTH_TYPE_SCROLL_,
	IG_SPLASH_SCROLL,
	IG_VOCATION_SCROLL,
	IG_WISDOM_SCROLL,
	IG_PATRON_SCROLL,
	IG_HEAVEN_SCROLL,
	IG_TW_AUG_SCROLL,
	IG_TW_NOV_SCROLL2,
	IG_ILLUSION_NOTHING,
	IG_TW_SEP_SCROLL,
	IG_FLAME_LIGHT,
	IG_TW_RAINBOW_SCROLL,
	IG_TW_RED_SCROLL,
	IG_TW_ORANGE_SCROLL,
	IG_TW_YELLOW_SCROLL,
	IG_SCROLL_OF_DEATH,
	IG_SCROLL_OF_LIFE,
	IG_SCROLL_OF_MAGIC,
	IG_SCROLL_OF_THEWS,
	IG_SCROLL_OF_DARKNESS,
	IG_SCROLL_OF_HOLINESS,
	IG_HORNED_SCROLL,
	IG_MERCURY_SCROLL,
	IG_CHALLENGE_KIT,
	IG_TW_APRIL_SCROLL,
	IG_TW_OCTOBER_SCROLL_,
	IG_SUMMER_SCROLL3,
	IG_C_WING_OF_FLY_3DAY_BOX,
	IG_RWC_2012_SET_BOX,
	IG_EX_DEF_POTION_BOX,
	IG_RWC_SCROLL_2012,
	IG_OLD_COIN_POCKET,
	IG_HIGH_COIN_POCKET,
	IG_MID_COIN_POCKET,
	IG_LOW_COIN_POCKET,
	IG_SGRADE_POCKET,
	IG_AGRADE_POCKET,
	IG_BGRADE_POCKET,
	IG_CGRADE_POCKET,
	IG_DGRADE_POCKET,
	IG_EGRADE_POCKET,
	IG_PTOTECTION_SEAGOD_BOX,
	IG_HAIRTAIL_BOX1,
	IG_HAIRTAIL_BOX2,
	IG_SPEARFISH_BOX1,
	IG_SPEARFISH_BOX2,
	IG_SAUREL_BOX1,
	IG_SAUREL_BOX2,
	IG_TUNA_BOX1,
	IG_TUNA_BOX2,
	IG_MALANG_CRAB_BOX1,
	IG_MALANG_CRAB_BOX2,
	IG_BRINDLE_EEL_BOX1,
	IG_BRINDLE_EEL_BOX2,
	IG_PTOTECTION_SEAGOD_BOX2,
	IG_PTOTECTION_SEAGOD_BOX3,
	IG_OCTO_HSTICK_BOX,
	IG_OCTO_HSTICK_BOX2,
	IG_OCTO_HSTICK_BOX3,
	IG_SILVERVINE_FRUIT_BOX10,
	IG_SILVERVINE_FRUIT_BOX40,
	IG_SILVERVINE_FRUIT_BOX4,
	IG_MALANG_WOE_ENCARD_BOX,
	IG_XMAS_BLESS,
	IG_FIRE_TYPE_SCROLL,
	IG_BLUE_SCROLL,
	IG_GOOD_STUDENT_GIFT_BOX,
	IG_BAD_STUDENT_GIFT_BOX,
	IG_INDIGO_SCROLL,
	IG_VIOLET_SCROLL,
	IG_BI_HWANG_SCROLL,
	IG_JUNG_BI_SCROLL,
	IG_JE_UN_SCROLL,
	IG_YONG_KWANG_SCROLL,
	IG_HALLOWEEN_G_BOX,
	IG_SOLO_CHRISTMAS_GIFT,
	IG_SG_WEAPON_SUPPLY_BOX,
	IG_CANDY_HOLDER,
	IG_LUCKY_BAG,
	IG_HOLY_EGG_2,
	IG_ADVENTURER_RETURNS_SUPPORT_BOX,
	IG_SUPPORT_PACKAGE,
	IG_SUPPORT_PACKAGE_10,
	IG_EVENT_ALMIGHTY_BOX,
	IG_EVENT_ALMIGHTY_BOX_100,
	IG_LOTTOBOX1,
	IG_LOTTOBOX2,
	IG_LOTTOBOX3,
	IG_LOTTOBOX4,
	IG_LOTTOBOX5,
	IG_SUPPORT_PACKAGE_III,
	IG_SUPPORT_PACKAGE_III_10,
	IG_UNLIMITED_BOX,
	IG_UNLIMITED_BOX_10,
	IG_UNLIMITED_BOX_II,
	IG_UNLIMITED_BOX_II_10,
	IG_THREE_MASTER_PACKAGE_III,
	IG_THREE_MASTER_PACKAGE_III_10,
	IG_2013_RWC_SCROLL,
	IG_SUPPORT_PACKAGE_II,
	IG_SUPPORT_PACKAGE_II_10,
	IG_LEVEL_UP_BOX100,
	IG_LEVEL_UP_BOX120,
	IG_LEVEL_UP_BOX130,
	IG_LEVEL_UP_BOX140,
	IG_LEVEL_UP_BOX150,
	IG_LEVEL_UP_BOX160,
	IG_GIFT_BUFF_SET,
	IG_LUCKY_SILVERVINE_FRUIT_BOX_III10,
	IG_LUCKY_SILVERVINE_FRUIT_BOX_III110,
	IG_OLD_ORE_BOX,
	IG_BLESSING_LUCKY_SCROLL,
	IG_SOGRAT_LUCKY_SCROLL,
	IG_GARNET_LUCKY_SCROLL,
	IG_AMORA_LUCKY_SCROLL,
	IG_VENUS_LUCKY_SCROLL,
	IG_ERZULIE_LUCKY_SCROLL,
	IG_MAJESTIC_LUCKY_SCROLL,
	IG_EPIC_HEROES_LUCKY_EGG,
	IG_HERO_MIDGARD_EGG,
	IG_IMORTAL_MIDGARD_SCROLL,
	IG_TW_13Y_LUCKY_EGG_06,
	IG_HAPPY_TIME_SCROLL,
	IG_TIME_TRAVEL_SCROLL,
	IG_SOLARIS_FESTIVAL_SCROLL,
	IG_MIDGARD_FES_SCROLL,
	IG_MIDGARD_SCROLL,
	IG_SWEET_MIDGARD_SCROLL,
	IG_WINTER_MIDGARD_SCROLL,
	IG_SPRING_FESTIVAL_SCROLL,
	IG_IDRO10TH_SCROLL,
	IG_REQUIEM_SCROLL,
	IG_HOLY_SPIRIT_SCROLL,
	IG_GARUDA_SCROLL,
	IG_THANKS_GIVING_SCROLL,
	IG_IDN_LEGEND_HERO_SCROLL,
	IG_BLESSING_MIDGARD_SCROLL,
	IG_CHRONOSIAN_LUCKY_SCROLL,
	IG_SANCTUARY_LUCKY_SCROLL,
	IG_CYBORG_LUCKY_SCROLL,
	IG_UNDINE_LUCKY_SCROLL,
	IG_GOD_MATERIAL_BOX,
	IG_SEALED_MIND_BOX,
	IG_COSTAMA_EGG18,
	IG_COSTAMA_EGG19,
	IG_FLOWER_BLOSSOM_SCROLL,
	IG_COSTAMA_EGG24,
	IG_SMITHY_LUCKY_SCROLL,
	IG_GANYMEDE_LUCKY_SCROLL,
	IG_LASTANGEL_LUCKYSCROLL,
	IG_VALKYRIE_LUCKY_SCROLL,
	IG_SPLASH_RAINBOW_LUCKY_SCROLL,
	IG_SHAPESHIFTER_COSTUME,
	IG_JULY_LUCKY_SCROLL,
	IG_COSTAMA_EGG23,
	IG_COSTAMA_EGG28,
	IG_MIDGARD_LUCKY_SCROLL,
	IG_BLESSING_SCARLET_SCROLL,
	IG_COSTAMA_EGG29,
	IG_INK_BALL,
	IG_SOMETHING_CANDY_HOLDER,
	IG_MYSTERIOUS_EGG,
	IG_AGUST_LUCKY_SCROLL,
	IG_ELEMENT,
	IG_POISON,
	IG_CASH_FOOD,
	IG_BOMB,
	IG_THROWABLE,
	IG_MERCENARY,
	IG_NOIVE_BOX,
	IG_VALERIAN_SCROLL,
	IG_IMMORTAL_EGG,
	IG_SAPPHIRE_EGG,
	IG_IDN_HEART_SCROLL,
	IG_IDN_WISDOM_EGG,
	IG_CHRISTMAS_BOX,
	IG_SPECIAL_CHRISTMAS_BOX,
	IG_SANTA_GIFT,
	IG_PRIZEOFHERO,
};

/// Enum for bound/sell restricted selling
enum e_itemshop_restrictions {
	ISR_NONE = 0x0,
	ISR_BOUND = 0x1,
	ISR_SELLABLE = 0x2,
	ISR_ALL = 0x3,
};

///Item combo struct
struct item_combo
{
	struct script_code *script;
	unsigned short *nameid;/* nameid array */
	unsigned char count;
	unsigned short id;/* id of this combo */
	bool isRef;/* whether this struct is a reference or the master */
};


/// Struct of item group entry
struct s_item_group_entry
{
	unsigned short nameid, /// Item ID
		duration, /// Duration if item as rental item (in minutes)
		amount; /// Amount of item will be obtained
	bool isAnnounced, /// Broadcast if player get this item
		GUID, /// Gives Unique ID for items in each box opened
		isNamed; /// Named the item (if possible)
	char bound; /// Makes the item as bound item (according to bound type)
};

/// Struct of random group
struct s_item_group_random
{
	struct s_item_group_entry *data; /// Random group entry
	unsigned short data_qty; /// Number of item in random group
};

/// Struct of item group that will be used for db
struct s_item_group_db
{
	unsigned short id, /// Item Group ID
		must_qty; /// Number of must item at this group
	struct s_item_group_entry *must; /// Must item entry
	struct s_item_group_random random[MAX_ITEMGROUP_RANDGROUP]; //! TODO: Move this fixed array to dynamic size if needed.
};

/// Struct of Roulette db
struct s_roulette_db {
	unsigned short *nameid[MAX_ROULETTE_LEVEL], /// Item ID
		           *qty[MAX_ROULETTE_LEVEL]; /// Amount of Item ID
	int *flag[MAX_ROULETTE_LEVEL]; /// Whether the item is for loss or win
	int items[MAX_ROULETTE_LEVEL]; /// Number of items in the list for each
};
extern struct s_roulette_db rd;

///Main item data struct
struct item_data
{
	unsigned short nameid;
	char name[ITEM_NAME_LENGTH],jname[ITEM_NAME_LENGTH];

	//Do not add stuff between value_buy and view_id (see how getiteminfo works)
	int value_buy;
	int value_sell;
	int type;
	int maxchance; //For logs, for external game info, for scripts: Max drop chance of this item (e.g. 0.01% , etc.. if it = 0, then monsters don't drop it, -1 denotes items sold in shops only) [Lupus]
	int sex;
	int equip;
	int weight;
	int atk;
	int def;
	int range;
	int slot;
	int look;
	int elv;
	int wlv;
	int view_id;
	int elvmax; ///< Maximum level for this item
#ifdef RENEWAL
	int matk;
#endif

	int delay;
//Lupus: I rearranged order of these fields due to compatibility with ITEMINFO script command
//		some script commands should be revised as well...
	uint64 class_base[3];	//Specifies if the base can wear this item (split in 3 indexes per type: 1-1, 2-1, 2-2)
	unsigned class_upper : 6; //Specifies if the class-type can equip it (0x01: normal, 0x02: trans, 0x04: baby, 0x08:third, 0x10:trans-third, 0x20-third-baby)
	struct {
		int chance;
		int id;
	} mob[MAX_SEARCH]; //Holds the mobs that have the highest drop rate for this item. [Skotlex]
	struct script_code *script;	//Default script for everything.
	struct script_code *equip_script;	//Script executed once when equipping.
	struct script_code *unequip_script;//Script executed once when unequipping.
	struct {
		unsigned available : 1;
		uint32 no_equip;
		unsigned no_refine : 1;	// [celest]
		unsigned delay_consume : 2;	// 1 - Signifies items that are not consumed immediately upon double-click; 2 - Signifies items that are not removed on consumption [Skotlex]
		unsigned trade_restriction : 9;	//Item restrictions mask [Skotlex]
		unsigned autoequip: 1;
		unsigned buyingstore : 1;
		unsigned dead_branch : 1; // As dead branch item. Logged at `branchlog` table and cannot be used at 'nobranch' mapflag [Cydh]
		unsigned group : 1; // As item group container [Cydh]
		unsigned guid : 1; // This item always be attached with GUID and make it as bound item! [Cydh]
		unsigned broadcast : 1; ///< Will be broadcasted if someone obtain the item [Cydh]
		bool bindOnEquip; ///< Set item as bound when equipped
	} flag;
	struct {// item stacking limitation
		unsigned short amount;
		unsigned int inventory:1;
		unsigned int cart:1;
		unsigned int storage:1;
		unsigned int guildstorage:1;
	} stack;
	struct {// used by item_nouse.txt
		unsigned int flag;
		unsigned short override;
	} item_usage;
	short gm_lv_trade_override;	//GM-level to override trade_restriction
	/* bugreport:309 */
	struct item_combo **combos;
	unsigned char combos_count;
	short delay_sc; ///< Use delay group if any instead using player's item_delay data [Cydh]
};

// Struct for item random option [Secret]
struct s_random_opt_data
{
	unsigned short id;
	struct script_code *script;
};

/// Enum for Random Option Groups
enum Random_Option_Group {
	RDMOPTG_None = 0,
	RDMOPTG_Crimson_Weapon,
};

/// Struct for random option group entry
struct s_random_opt_group_entry {
	struct s_item_randomoption option[MAX_ITEM_RDM_OPT];
};

/// Struct for Random Option Group
struct s_random_opt_group {
	uint8 id;
	struct s_random_opt_group_entry *entries;
	uint16 total;
};

struct item_data* itemdb_searchname(const char *name);
int itemdb_searchname_array(struct item_data** data, int size, const char *str);
struct item_data* itemdb_search(unsigned short nameid);
struct item_data* itemdb_exists(unsigned short nameid);
#define itemdb_name(n) itemdb_search(n)->name
#define itemdb_jname(n) itemdb_search(n)->jname
#define itemdb_type(n) itemdb_search(n)->type
#define itemdb_atk(n) itemdb_search(n)->atk
#define itemdb_def(n) itemdb_search(n)->def
#define itemdb_look(n) itemdb_search(n)->look
#define itemdb_weight(n) itemdb_search(n)->weight
#define itemdb_equip(n) itemdb_search(n)->equip
#define itemdb_usescript(n) itemdb_search(n)->script
#define itemdb_equipscript(n) itemdb_search(n)->script
#define itemdb_wlv(n) itemdb_search(n)->wlv
#define itemdb_range(n) itemdb_search(n)->range
#define itemdb_slot(n) itemdb_search(n)->slot
#define itemdb_available(n) (itemdb_search(n)->flag.available)
#define itemdb_traderight(n) (itemdb_search(n)->flag.trade_restriction)
#define itemdb_viewid(n) (itemdb_search(n)->view_id)
#define itemdb_autoequip(n) (itemdb_search(n)->flag.autoequip)
const char* itemdb_typename(enum item_types type);
const char *itemdb_typename_ammo (enum e_item_ammo ammo);
bool itemdb_is_spellbook2(unsigned short nameid);

struct s_item_group_entry *itemdb_get_randgroupitem(uint16 group_id, uint8 sub_group);
unsigned short itemdb_searchrandomid(uint16 group_id, uint8 sub_group);

#define itemdb_value_buy(n) itemdb_search(n)->value_buy
#define itemdb_value_sell(n) itemdb_search(n)->value_sell
#define itemdb_canrefine(n) (!itemdb_search(n)->flag.no_refine)
//Item trade restrictions [Skotlex]
bool itemdb_isdropable_sub(struct item_data *itd, int gmlv, int unused);
bool itemdb_cantrade_sub(struct item_data *itd, int gmlv, int gmlv2);
bool itemdb_canpartnertrade_sub(struct item_data *itd, int gmlv, int gmlv2);
bool itemdb_cansell_sub(struct item_data *itd, int gmlv, int unused);
bool itemdb_cancartstore_sub(struct item_data *itd, int gmlv, int unused);
bool itemdb_canstore_sub(struct item_data *itd, int gmlv, int unused);
bool itemdb_canguildstore_sub(struct item_data *itd, int gmlv, int unused);
bool itemdb_canmail_sub(struct item_data *itd, int gmlv, int unused);
bool itemdb_canauction_sub(struct item_data *itd, int gmlv, int unused);
bool itemdb_isrestricted(struct item* item, int gmlv, int gmlv2, bool (*func)(struct item_data*, int, int));
#define itemdb_isdropable(item, gmlv) itemdb_isrestricted(item, gmlv, 0, itemdb_isdropable_sub)
#define itemdb_cantrade(item, gmlv, gmlv2) itemdb_isrestricted(item, gmlv, gmlv2, itemdb_cantrade_sub)
#define itemdb_canpartnertrade(item, gmlv, gmlv2) itemdb_isrestricted(item, gmlv, gmlv2, itemdb_canpartnertrade_sub)
#define itemdb_cansell(item, gmlv) itemdb_isrestricted(item, gmlv, 0, itemdb_cansell_sub)
#define itemdb_cancartstore(item, gmlv)  itemdb_isrestricted(item, gmlv, 0, itemdb_cancartstore_sub)
#define itemdb_canstore(item, gmlv) itemdb_isrestricted(item, gmlv, 0, itemdb_canstore_sub)
#define itemdb_canguildstore(item, gmlv) itemdb_isrestricted(item , gmlv, 0, itemdb_canguildstore_sub)
#define itemdb_canmail(item, gmlv) itemdb_isrestricted(item , gmlv, 0, itemdb_canmail_sub)
#define itemdb_canauction(item, gmlv) itemdb_isrestricted(item , gmlv, 0, itemdb_canauction_sub)

bool itemdb_isequip2(struct item_data *id);
#define itemdb_isequip(nameid) itemdb_isequip2(itemdb_search(nameid))
char itemdb_isidentified(unsigned short nameid);
bool itemdb_isstackable2(struct item_data *id);
#define itemdb_isstackable(nameid) itemdb_isstackable2(itemdb_search(nameid))
bool itemdb_isNoEquip(struct item_data *id, uint16 m);

struct item_combo *itemdb_combo_exists(unsigned short combo_id);

struct s_item_group_db *itemdb_group_exists(unsigned short group_id);
bool itemdb_group_item_exists(unsigned short group_id, unsigned short nameid);
char itemdb_pc_get_itemgroup(uint16 group_id, bool identify, struct map_session_data *sd);

bool itemdb_parse_roulette_db(void);

struct s_random_opt_data *itemdb_randomopt_exists(short id);
struct s_random_opt_group *itemdb_randomopt_group_exists(int id);

void itemdb_reload(void);

void do_final_itemdb(void);
void do_init_itemdb(void);

#endif /* _ITEMDB_HPP_ */
