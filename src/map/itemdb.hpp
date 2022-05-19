// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ITEMDB_HPP
#define ITEMDB_HPP

#include <map>
#include <vector>

#include "../common/database.hpp"
#include "../common/db.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp" // ITEM_NAME_LENGTH

#include "script.hpp"
#include "status.hpp"

enum e_ammo_type : uint8;

///Use apple for unknown items.
const t_itemid UNKNOWN_ITEM_ID = 512;
/// The maximum number of item delays
#define MAX_ITEMDELAYS	10
///Designed for search functions, species max number of matches to display.
#define MAX_SEARCH	5

#define MAX_ROULETTE_LEVEL 7 /** client-defined value **/
#define MAX_ROULETTE_COLUMNS 9 /** client-defined value **/

const t_itemid CARD0_FORGE = 0x00FF;
const t_itemid CARD0_CREATE = 0x00FE;
const t_itemid CARD0_PET = 0x0100;

///Marks if the card0 given is "special" (non-item id used to mark pets/created items. [Skotlex]
#define itemdb_isspecial(i) (i == CARD0_FORGE || i == CARD0_CREATE || i == CARD0_PET)

///Enum of item id (for hardcoded purpose)
enum item_itemid : t_itemid
{
	ITEMID_DUMMY						= 499,
	ITEMID_RED_POTION					= 501,
	ITEMID_YELLOW_POTION				= 503,
	ITEMID_WHITE_POTION					= 504,
	ITEMID_BLUE_POTION					= 505,
	ITEMID_APPLE						= 512,
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
	ITEMID_NEW_INSURANCE				= 6413,
	ITEMID_STRANGE_EMBRYO				= 6415,
	ITEMID_BLACKSMITH_BLESSING			= 6635,
	ITEMID_STONE						= 7049,
	ITEMID_FIRE_BOTTLE					= 7135,
	ITEMID_ACID_BOTTLE					= 7136,
	ITEMID_MAN_EATER_BOTTLE				= 7137,
	ITEMID_MINI_BOTTLE					= 7138,
	ITEMID_COATING_BOTTLE				= 7139,
	ITEMID_FRAGMENT_OF_CRYSTAL			= 7321,
	ITEMID_SKULL_						= 7420,
	ITEMID_TRAP_ALLOY					= 7940,
	ITEMID_COOKIE_BAT					= 11605,
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
	ITEMID_PURIFICATION_BULLET			= 13220,
	ITEMID_SILVER_BULLET_				= 13221,
	ITEMID_DUN_TELE_SCROLL1				= 14527,
	ITEMID_DUN_TELE_SCROLL2				= 14581,
	ITEMID_WOB_RUNE						= 14582,
	ITEMID_WOB_SCHWALTZ					= 14583,
	ITEMID_WOB_RACHEL					= 14584,
	ITEMID_WOB_LOCAL					= 14585,
	ITEMID_SIEGE_TELEPORT_SCROLL		= 14591,
	ITEMID_INVENTORY_EX_EVT				= 25791,
	ITEMID_INVENTORY_EX_DIS				= 25792,
	ITEMID_INVENTORY_EX					= 25793,
	ITEMID_WL_MB_SG						= 100065,
	ITEMID_HOMUNCULUS_SUPPLEMENT		= 100371,
};

///Rune Knight
enum rune_item_list : t_itemid
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
enum mechanic_item_list : t_itemid
{
	ITEMID_ACCELERATOR				= 2800,
	ITEMID_SUICIDAL_DEVICE				= 2802,
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
enum genetic_item_list : t_itemid
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
	ITEMID_HP_INC_POTS_TO_THROW			= 13275,
	ITEMID_HP_INC_POTM_TO_THROW,
	ITEMID_HP_INC_POTL_TO_THROW,
	ITEMID_SP_INC_POTS_TO_THROW,
	ITEMID_SP_INC_POTM_TO_THROW,
	ITEMID_SP_INC_POTL_TO_THROW,
};

///Guillotine Cross
enum poison_item_list : t_itemid
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

///Item job
enum e_item_job : uint16
{
	ITEMJ_NONE        = 0x00,
	ITEMJ_NORMAL      = 0x01,
	ITEMJ_UPPER       = 0x02,
	ITEMJ_BABY        = 0x04,
	ITEMJ_THIRD       = 0x08,
	ITEMJ_THIRD_UPPER = 0x10,
	ITEMJ_THIRD_BABY  = 0x20,
	ITEMJ_FOURTH      = 0x40,
	ITEMJ_MAX         = 0xFF,

	ITEMJ_ALL_UPPER = ITEMJ_UPPER | ITEMJ_THIRD_UPPER | ITEMJ_FOURTH,
	ITEMJ_ALL_BABY = ITEMJ_BABY | ITEMJ_THIRD_BABY,
	ITEMJ_ALL_THIRD = ITEMJ_THIRD | ITEMJ_THIRD_UPPER | ITEMJ_THIRD_BABY,

#ifdef RENEWAL
	ITEMJ_ALL = ITEMJ_NORMAL | ITEMJ_UPPER | ITEMJ_BABY | ITEMJ_THIRD | ITEMJ_THIRD_UPPER | ITEMJ_THIRD_BABY | ITEMJ_FOURTH,
#else
	ITEMJ_ALL = ITEMJ_NORMAL | ITEMJ_UPPER | ITEMJ_BABY,
#endif
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
	IG_SHINING_EGG,
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
	IG_PRIVATE_AIRSHIP,
	IG_TOKEN_OF_SIEGFRIED,
	IG_ENCHANT_STONE_BOX,
	IG_ENCHANT_STONE_BOX2,
	IG_ENCHANT_STONE_BOX3,
	IG_ENCHANT_STONE_BOX4,
	IG_ENCHANT_STONE_BOX5,
	IG_ENCHANT_STONE_BOX6,
	IG_ENCHANT_STONE_BOX7,
	IG_ENCHANT_STONE_BOX8,
	IG_ENCHANT_STONE_BOX9,
	IG_ENCHANT_STONE_BOX10,
	IG_ENCHANT_STONE_BOX11,
	IG_ENCHANT_STONE_BOX12,
	IG_ENCHANT_STONE_BOX13,
	IG_ENCHANT_STONE_BOX14,
	IG_ENCHANT_STONE_BOX15,
	IG_ENCHANT_STONE_BOX16,
	IG_ENCHANT_STONE_BOX17,
	IG_ENCHANT_STONE_BOX18,
	IG_ENCHANT_STONE_BOX19,
	IG_ENCHANT_STONE_BOX20,
	IG_ENCHANT_STONE_BOX21,
	IG_XMAS_PACKAGE_14,
	IG_EASTER_EGG,
	IG_PITAPAT_BOX,
	IG_HAPPY_BOX_J,
	IG_CLASS_SHADOW_CUBE,
	IG_SEALED_SCROLL,
	IG_SQUAD_PRIZE1,
	IG_SQUAD_PRIZE2,
	IG_LI_NYANGVINE_BOX1_26,
	IG_LI_NYANGVINE_BOX2_26,
	IG_LI_NYANGVINE_BOX3_26,
	IG_ENCHANT_STONE_BOX26,
	IG_THIRD_JOB_STONE_GARMENT_BOX,
	IG_THIRD_JOB_STONE_GARMENT_BOX2,
	IG_THIRD_JOB_STONE_TOP_BOX,
	IG_THIRD_JOB_STONE_TOP_BOX2,
	IG_THIRD_JOB_STONE_MIDDLE_BOX,
	IG_THIRD_JOB_STONE_MIDDLE_BOX2,
	IG_THIRD_JOB_STONE_BOTTOM_BOX,
	IG_THIRD_JOB_STONE_BOTTOM_BOX2,
	IG_SHADOW_EXCHANGE_BOX,
	IG_GUNSLINGER_ENCHANT,
	IG_ENCHANTSTONE_RECIPE,
	IG_PET_EGG_BOX,
	IG_COSTUME_EXCHANGE_BOX,
	IG_FAN_UPGRADE_KIT,
	IG_SUIT_UPGRADE_KIT,
	IG_SCROLL_OF_FALLEN_ANGEL_WINGS,
	IG_CLASS_SHADOW_BOX_WEAPON,
	IG_CLASS_SHADOW_BOX_ARMOR,
	IG_CLASS_SHADOW_BOX_SHOES,
	IG_CLASS_SHADOW_BOX_SHIELD,
	IG_CLASS_SHADOW_BOX_PENDANT,
	IG_CLASS_SHADOW_BOX_EARRING,
	IG_STATUSSHADOW_MIX,
	IG_GEMSTONESHADOW_MIX,
	IG_BEARERSSHADOW_MIX,
	IG_COMPOSESHADOW_MIX,
	IG_RACESHADOW_MIX,
	IG_CANDY_BAG_SCROLL_MELEE,
	IG_CANDY_BAG_SCROLL_RANGE,
	IG_CANDY_BAG_SCROLL_MAGIC,
	IG_BOOSTER_AMPLIFIER,
	IG_MAGICAL_CAT_HAND,
	IG_INFINITYSHADOW_MIX,
	IG_SILVER_STATUE,
	IG_PHYSICALMAGICAL_MIX,
	IG_IMMUNEDATHENA_MIX,
	IG_HARDCHAMPTION_MIX,
	IG_KINGBIRDANCIENT_MIX,
	IG_CRITICALHIT_MIX,
	IG_BS_ITEM_M_S_2,
	IG_BS_ITEM_M_S_8,
	IG_BS_ITEM_M_S_10,
	IG_BS_ITEM_M_S_11,
	IG_BS_ITEM_M_S_34,
	IG_BS_ITEM_M_S_41,
	IG_BS_ITEM_M_S_42,
	IG_BS_ITEM_M_S_43,
	IG_BS_ITEM_M_S_44,
	IG_BS_SHA_M_S_1,
	IG_BS_SHA_M_S_17,
	IG_BS_SHA_M_S_18,
	IG_BS_SHA_M_S_19,
	IG_BS_SHA_M_S_20,
	IG_BS_ITEM_M_S_4,
	IG_BS_ITEM_M_S_6,
	IG_BS_ITEM_M_S_7,
	IG_BS_ITEM_M_S_12,
	IG_BS_ITEM_M_S_13,
	IG_BS_ITEM_M_S_15,
	IG_BS_ITEM_M_S_28,
	IG_BS_ITEM_M_S_29,
	IG_BS_ITEM_M_S_31,
	IG_BS_ITEM_M_S_32,
	IG_BS_ITEM_M_S_33,
	IG_BS_ITEM_M_S_36,
	IG_BS_ITEM_M_S_37,
	IG_BS_ITEM_M_S_38,
	IG_BS_ITEM_M_S_39,
	IG_BS_ITEM_M_S_40,
	IG_BS_ITEM_M_S_45,
	IG_BS_ITEM_M_S_46,
	IG_BS_ITEM_M_S_47,
	IG_BS_ITEM_M_S_48,
	IG_BS_ITEM_M_S_49,
	IG_BS_ITEM_M_S_50,
	IG_BS_SHA_M_S_5,
	IG_BS_SHA_M_S_6,
	IG_BS_SHA_M_S_7,
	IG_BS_SHA_M_S_8,
	IG_BS_SHA_M_S_13,
	IG_BS_SHA_M_S_15,
	IG_BS_SHA_M_S_16,
	IG_BS_SHA_M_S_23,
	IG_BS_ITEM_M_S_5,
	IG_BS_ITEM_M_S_9,
	IG_BS_ITEM_M_S_14,
	IG_BS_ITEM_M_S_16,
	IG_BS_ITEM_M_S_17,
	IG_BS_ITEM_M_S_19,
	IG_BS_ITEM_M_S_27,
	IG_BS_ITEM_M_S_35,
	IG_BS_SHA_M_S_9,
	IG_BS_SHA_M_S_10,
	IG_BS_SHA_M_S_11,
	IG_BS_SHA_M_S_21,
	IG_BS_ITEM_M_S_1,
	IG_BS_ITEM_M_S_3,
	IG_BS_ITEM_M_S_18,
	IG_BS_ITEM_M_S_20,
	IG_BS_ITEM_M_S_21,
	IG_BS_ITEM_M_S_22,
	IG_BS_ITEM_M_S_23,
	IG_BS_ITEM_M_S_24,
	IG_BS_ITEM_M_S_25,
	IG_BS_ITEM_M_S_26,
	IG_BS_ITEM_M_S_30,
	IG_BS_SHA_M_S_3,
	IG_BS_SHA_M_S_4,
	IG_BS_SHA_M_S_12,
	IG_BS_SHA_M_S_14,
	IG_BS_SHA_M_S_24,
	IG_BS_SHA_M_S_25,
	IG_BS_ITEM_M_S_51,
	IG_ENCHANTSTONE_RECIPE_9M,
	IG_IDTEST_SPECIAL,
	IG_PERFECTSIZE_MIX,
	IG_MAGICPIERCING_MIX,
	IG_PIERCING_MIX,
	IG_HASTY_MIX,
	IG_ENCHANTSTONE_RECIPE_4M,
	IG_SHADOW_CUBE,
	IG_SHADOW_CUBE_PENDANT,
	IG_SHADOW_CUBE_EARING,
	IG_ANGELPORING_BOX,
	IG_HELM_OF_FAITH_BOX,
	IG_2022_LUNARNEWYEARS_BOX,
	IG_2020_REWARD_BOX,
	IG_COSTUME_MILE_PACK_26_1,
	IG_COSTUME_MILE_PACK_26_2,
	IG_COSTUME_MILE_PACK_26_3,
	IG_EP17_1_SPC01,
	IG_EP17_1_SPC02,
	IG_EP17_1_SPC03,
	IG_EP17_1_SPC04,
	IG_STABILITYSHADOW_MIX,
	IG_BS_SHA_M_S_2,
	IG_BS_SHA_M_S_22,
	IG_SLD_CARD_RECIPE,
	IG_R_BEARERSSHADOW_MIX,
	IG_M_BLITZSHADOW_MIX,
	IG_RELOADSHADOW_MIX,
	IG_SPELLCASTERSHADOW_MIX,
	IG_MAGICALSHADOW_MIX,
	IG_PHYSICALSHADOW_MIX,
	IG_MAJORAUTOSPELL_MIX,
	IG_ABSORBSHADOW_MIX,
	IG_TRUE_GEMSHADOW_MIX,
	IG_MAMMOTH_MIX,
	IG_FULLTEMPSHADOW_MIX,
	IG_FULLPENESHADOW_MIX,
	IG_REMODEL_HERO_BOOTS,
	IG_ORIENTAL_SWORD_CUBE,
	IG_DRAGONIC_SLAYER_CUBE,
	IG_SHIVER_KATAR_K_CUBE,
	IG_BLADE_KATAR_CUBE,
	IG_SWORD_OF_BLUEFIRE_CUBE,
	IG_SLATE_SWORD_CUBE,
	IG_NARCIS_BOW_CUBE,
	IG_TRUMPET_SHELL_K_CUBE,
	IG_BARB_WIRE_K_CUBE,
	IG_AVENGER_CUBE,
	IG_METEOR_STRIKER_CUBE,
	IG_MAGIC_SWORD_CUBE,
	IG_FATALIST_CUBE,
	IG_ROYAL_BOW_K_CUBE,
	IG_SCALET_DRAGON_L_CUBE,
	IG_SHADOW_STAFF_K_CUBE,
	IG_FREEZING_ROD_CUBE,
	IG_IRON_NAIL_K_CUBE,
	IG_RAY_KNUCKLE_CUBE,
	IG_UNDINE_SPEAR_K_CUBE,
	IG_LIGHT_BLADE_CUBE,
	IG_IRON_STAFF_CUBE,
	IG_BLUE_CRYSTAL_STAFF_CUBE,
	IG_DEMON_HUNT_BIBLE_CUBE,
	IG_SAINT_HALL_CUBE,
	IG_MEAWFOXTAIL_CUBE,
	IG_FOG_DEW_SWORD_CUBE,
	IG_HUMMA_CLEAR_CUBE,
	IG_THOUSAND_SUN_CUBE,
	IG_SPIRIT_PENDULUM_CUBE,
	IG_CRIMSON_ROSE_CUBE,
	IG_MASTER_SOUL_RIFLE_CUBE,
	IG_GOLDEN_LORD_LAUNCHER_CUBE,
	IG_THE_BLACK_CUBE,
	IG_DEMON_SLAYER_SHOT_CUBE,

	IG_MAX,
};

/// Enum for bound/sell restricted selling
enum e_itemshop_restrictions {
	ISR_NONE = 0x0,
	ISR_BOUND = 0x1,
	ISR_SELLABLE = 0x2,
	ISR_BOUND_SELLABLE = 0x4,
	ISR_BOUND_GUILDLEADER_ONLY = 0x8,
};

/// Enum for item drop effects
enum e_item_drop_effect : uint16 {
	DROPEFFECT_NONE = 0,
	DROPEFFECT_CLIENT,
#if PACKETVER < 20200304
	DROPEFFECT_WHITE_PILLAR,
#endif
	DROPEFFECT_BLUE_PILLAR,
	DROPEFFECT_YELLOW_PILLAR,
	DROPEFFECT_PURPLE_PILLAR,
#if PACKETVER < 20200304
	DROPEFFECT_ORANGE_PILLAR,
#else
	DROPEFFECT_GREEN_PILLAR,
#endif
#if PACKETVER >= 20200304
	DROPEFFECT_RED_PILLAR,
#endif
	DROPEFFECT_MAX,
#if PACKETVER >= 20200304
	// White was removed in 2020-03-04
	DROPEFFECT_WHITE_PILLAR,
	// Orange was replaced by green in 2020-03-04
	DROPEFFECT_ORANGE_PILLAR,
#else
	// Not supported before 2020-03-04
	DROPEFFECT_GREEN_PILLAR,
	DROPEFFECT_RED_PILLAR,
#endif
};

/// Enum for items with delayed consumption
enum e_delay_consume : uint8 {
	DELAYCONSUME_NONE = 0x0,
	DELAYCONSUME_TEMP = 0x1, // Items that are not consumed immediately upon double-click
	DELAYCONSUME_NOCONSUME = 0x2, // Items that are not removed upon double-click
};

/// Item combo struct
struct s_item_combo {
	std::vector<t_itemid> nameid;
	script_code *script;
	uint16 id;

	~s_item_combo() {
		if (this->script) {
			script_free_code(this->script);
			this->script = nullptr;
		}

		this->nameid.clear();
	}
};

class ComboDatabase : public TypesafeYamlDatabase<uint16, s_item_combo> {
private:
	uint16 combo_num;
	uint16 find_combo_id( const std::vector<t_itemid>& items );

public:
	ComboDatabase() : TypesafeYamlDatabase("COMBO_DB", 1) {

	}

	void clear() override{
		TypesafeYamlDatabase::clear();
		this->combo_num = 0;
	}
	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void loadingFinished() override;
};

extern ComboDatabase itemdb_combo;

// Struct for item random option [Secret]
struct s_random_opt_data
{
	uint16 id;
	std::string name;
	script_code *script;

	~s_random_opt_data() {
		if (script)
			script_free_code(script);
	}
};

/// Struct for random option group entry
struct s_random_opt_group_entry {
	uint16 id;
	int16 min_value, max_value;
	int8 param;
	uint16 chance;
};

/// Struct for Random Option Group
struct s_random_opt_group {
	uint16 id;
	std::string name;
	std::map<uint16, std::vector<std::shared_ptr<s_random_opt_group_entry>>> slots;
	uint16 max_random;
	std::vector<std::shared_ptr<s_random_opt_group_entry>> random_options;

public:
	void apply( struct item& item );
};

class RandomOptionDatabase : public TypesafeYamlDatabase<uint16, s_random_opt_data> {
public:
	RandomOptionDatabase() : TypesafeYamlDatabase("RANDOM_OPTION_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void loadingFinished() override;

	// Additional
	bool option_exists(std::string name);
	bool option_get_id(std::string name, uint16 &id);
};

extern RandomOptionDatabase random_option_db;

class RandomOptionGroupDatabase : public TypesafeYamlDatabase<uint16, s_random_opt_group> {
public:
	RandomOptionGroupDatabase() : TypesafeYamlDatabase("RANDOM_OPTION_GROUP", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;

	// Additional
	bool add_option(const ryml::NodeRef& node, std::shared_ptr<s_random_opt_group_entry> &entry);
	bool option_exists(std::string name);
	bool option_get_id(std::string name, uint16 &id);
};

extern RandomOptionGroupDatabase random_option_group;

/// Struct of item group entry
struct s_item_group_entry
{
	t_itemid nameid; /// Item ID
	uint16 rate;
	uint16 duration, /// Duration if item as rental item (in minutes)
		amount; /// Amount of item will be obtained
	bool isAnnounced, /// Broadcast if player get this item
		GUID, /// Gives Unique ID for items in each box opened
		isStacked, /// Whether stackable items are given stacked
		isNamed; /// Named the item (if possible)
	uint8 bound; /// Makes the item as bound item (according to bound type)
	std::shared_ptr<s_random_opt_group> randomOptionGroup;
	uint16 refineMinimum;
	uint16 refineMaximum;
};

/// Struct of random group
struct s_item_group_random
{
	uint32 total_rate;
	std::unordered_map<t_itemid, std::shared_ptr<s_item_group_entry>> data; /// item ID, s_item_group_entry

	std::shared_ptr<s_item_group_entry> get_random_itemsubgroup();
};

/// Struct of item group that will be used for db
struct s_item_group_db
{
	uint16 id; /// Item Group ID
	std::unordered_map<uint16, std::shared_ptr<s_item_group_random>> random;	/// group ID, s_item_group_random
};

/// Struct of Roulette db
struct s_roulette_db {
	t_itemid *nameid[MAX_ROULETTE_LEVEL]; /// Item ID
	unsigned short *qty[MAX_ROULETTE_LEVEL]; /// Amount of Item ID
	int *flag[MAX_ROULETTE_LEVEL]; /// Whether the item is for loss or win
	int items[MAX_ROULETTE_LEVEL]; /// Number of items in the list for each
};
extern struct s_roulette_db rd;

///Main item data struct
struct item_data
{
	t_itemid nameid;
	std::string name, ename;

	uint32 value_buy;
	uint32 value_sell;
	item_types type;
	uint8 subtype;
	int maxchance; //For logs, for external game info, for scripts: Max drop chance of this item (e.g. 0.01% , etc.. if it = 0, then monsters don't drop it, -1 denotes items sold in shops only) [Lupus]
	uint8 sex;
	uint32 equip;
	uint32 weight;
	uint32 atk;
	uint32 def;
	uint16 range;
	uint16 slots;
	uint32 look;
	uint16 elv;
	uint16 weapon_level;
	uint16 armor_level;
	t_itemid view_id;
	uint16 elvmax; ///< Maximum level for this item
#ifdef RENEWAL
	uint32 matk;
#endif

//Lupus: I rearranged order of these fields due to compatibility with ITEMINFO script command
//		some script commands should be revised as well...
	uint64 class_base[3];	//Specifies if the base can wear this item (split in 3 indexes per type: 1-1, 2-1, 2-2)
	uint16 class_upper; //Specifies if the class-type can equip it (See e_item_job)
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
		unsigned delay_consume;	// [Skotlex]
		struct {
			bool drop, trade, trade_partner, sell, cart, storage, guild_storage, mail, auction;
		} trade_restriction;	//Item restrictions mask [Skotlex]
		unsigned autoequip: 1;
		bool buyingstore;
		bool dead_branch; // As dead branch item. Logged at `branchlog` table and cannot be used at 'nobranch' mapflag [Cydh]
		bool group; // As item group container [Cydh]
		unsigned guid : 1; // This item always be attached with GUID and make it as bound item! [Cydh]
		bool broadcast; ///< Will be broadcasted if someone obtain the item [Cydh]
		bool bindOnEquip; ///< Set item as bound when equipped
		e_item_drop_effect dropEffect; ///< Drop Effect Mode
	} flag;
	struct {// item stacking limitation
		uint16 amount;
		bool inventory, cart, storage, guild_storage;
	} stack;
	struct {
		uint16 override;
		bool sitting;
	} item_usage;
	short gm_lv_trade_override;	//GM-level to override trade_restriction
	std::vector<std::shared_ptr<s_item_combo>> combos;
	struct {
		uint32 duration;
		sc_type sc; ///< Use delay group if any instead using player's item_delay data [Cydh]
	} delay;

	~item_data() {
		if (this->script){
			script_free_code(this->script);
			this->script = nullptr;
		}

		if (this->equip_script){
			script_free_code(this->equip_script);
			this->equip_script = nullptr;
		}

		if (this->unequip_script){
			script_free_code(this->unequip_script);
			this->unequip_script = nullptr;
		}

		this->combos.clear();
	}

	bool isStackable();
	int inventorySlotNeeded(int quantity);
};

class ItemDatabase : public TypesafeCachedYamlDatabase<t_itemid, item_data> {
private:
	std::unordered_map<std::string, std::shared_ptr<item_data>> nameToItemDataMap;
	std::unordered_map<std::string, std::shared_ptr<item_data>> aegisNameToItemDataMap;

	e_sex defaultGender( const ryml::NodeRef& node, std::shared_ptr<item_data> id );

public:
	ItemDatabase() : TypesafeCachedYamlDatabase("ITEM_DB", 2, 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void loadingFinished() override;
	void clear() override{
		TypesafeCachedYamlDatabase::clear();

		this->nameToItemDataMap.clear();
		this->aegisNameToItemDataMap.clear();
	}

	// Additional
	std::shared_ptr<item_data> searchname( const char* name );
	std::shared_ptr<item_data> search_aegisname( const char *name );
};

extern ItemDatabase item_db;

class ItemGroupDatabase : public TypesafeCachedYamlDatabase<uint16, s_item_group_db> {
public:
	ItemGroupDatabase() : TypesafeCachedYamlDatabase("ITEM_GROUP_DB", 2, 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void loadingFinished() override;

	// Additional
	bool item_exists(uint16 group_id, t_itemid nameid);
	int16 item_exists_pc(map_session_data *sd, uint16 group_id);
	t_itemid get_random_item_id(uint16 group_id, uint8 sub_group);
	std::shared_ptr<s_item_group_entry> get_random_entry(uint16 group_id, uint8 sub_group);
	uint8 pc_get_itemgroup(uint16 group_id, bool identify, map_session_data *sd);
};

extern ItemGroupDatabase itemdb_group;

struct s_laphine_synthesis_requirement{
	t_itemid item_id;
	uint16 amount;
};

struct s_laphine_synthesis{
	t_itemid item_id;
	uint16 minimumRefine;
	uint16 maximumRefine;
	uint16 requiredRequirements;
	std::unordered_map<t_itemid, std::shared_ptr<s_laphine_synthesis_requirement>> requirements;
	uint16 rewardGroupId;
};

class LaphineSynthesisDatabase : public TypesafeYamlDatabase<t_itemid, s_laphine_synthesis>{
public:
	LaphineSynthesisDatabase() : TypesafeYamlDatabase( "LAPHINE_SYNTHESIS_DB", 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode( const ryml::NodeRef& node );
};

extern LaphineSynthesisDatabase laphine_synthesis_db;

struct s_laphine_upgrade{
	t_itemid item_id;
	std::vector<t_itemid> target_item_ids;
	uint16 minimumRefine;
	uint16 maximumRefine;
	uint16 requiredRandomOptions;
	bool cardsAllowed;
	std::shared_ptr<s_random_opt_group> randomOptionGroup;
	uint16 resultRefine;
	uint16 resultRefineMinimum;
	uint16 resultRefineMaximum;
};

class LaphineUpgradeDatabase : public TypesafeYamlDatabase<t_itemid, s_laphine_upgrade>{
public:
	LaphineUpgradeDatabase() : TypesafeYamlDatabase( "LAPHINE_UPGRADE_DB", 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode( const ryml::NodeRef& node );
};

extern LaphineUpgradeDatabase laphine_upgrade_db;

int itemdb_searchname_array(struct item_data** data, int size, const char *str);
struct item_data* itemdb_search(t_itemid nameid);
struct item_data* itemdb_exists(t_itemid nameid);
#define itemdb_name(n) itemdb_search(n)->name.c_str()
#define itemdb_ename(n) itemdb_search(n)->ename.c_str()
#define itemdb_type(n) itemdb_search(n)->type
#define itemdb_subtype(n) itemdb_search(n)->subtype
#define itemdb_atk(n) itemdb_search(n)->atk
#define itemdb_def(n) itemdb_search(n)->def
#define itemdb_look(n) itemdb_search(n)->look
#define itemdb_weight(n) itemdb_search(n)->weight
#define itemdb_equip(n) itemdb_search(n)->equip
#define itemdb_usescript(n) itemdb_search(n)->script
#define itemdb_equipscript(n) itemdb_search(n)->script
#define itemdb_wlv(n) itemdb_search(n)->weapon_level
#define itemdb_range(n) itemdb_search(n)->range
#define itemdb_slots(n) itemdb_search(n)->slots
#define itemdb_available(n) (itemdb_search(n)->flag.available)
#define itemdb_traderight(n) (itemdb_search(n)->flag.trade_restriction)
#define itemdb_viewid(n) (itemdb_search(n)->view_id)
#define itemdb_autoequip(n) (itemdb_search(n)->flag.autoequip)
#define itemdb_dropeffect(n) (itemdb_search(n)->flag.dropEffect)
const char* itemdb_typename(enum item_types type);
const char *itemdb_typename_ammo (e_ammo_type ammo);

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
bool itemdb_ishatched_egg(struct item* item);
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
char itemdb_isidentified(t_itemid nameid);
bool itemdb_isstackable2(struct item_data *id);
#define itemdb_isstackable(nameid) itemdb_isstackable2(itemdb_search(nameid))
bool itemdb_isNoEquip(struct item_data *id, uint16 m);

bool itemdb_parse_roulette_db(void);

void itemdb_reload(void);

void do_final_itemdb(void);
void do_init_itemdb(void);

#endif /* ITEMDB_HPP */
