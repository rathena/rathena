// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ITEMDB_H_
#define _ITEMDB_H_

#include "../common/db.h"
#include "../common/mmo.h" // ITEM_NAME_LENGTH
#include "map.h"

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

//The only item group required by the code to be known. See const.txt for the full list.
#define IG_FINDINGORE 6
#define IG_POTION 37

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
	ITEMID_HOLY_WATER					= 523,
	ITEMID_PUMPKIN						= 535,
	ITEMID_RED_SLIM_POTION				= 545,
	ITEMID_YELLOW_SLIM_POTION			= 546,
	ITEMID_WHITE_SLIM_POTION			= 547,
	ITEMID_WING_OF_FLY					= 601,
	ITEMID_WING_OF_BUTTERFLY			= 602,
	ITEMID_ANODYNE						= 605,
	ITEMID_ALOEBERA						= 606,
	ITEMID_POISON_BOTTLE				= 678,
	ITEMID_EMPTY_BOTTLE					= 713,
	ITEMID_EMPERIUM						= 714,
	ITEMID_YELLOW_GEMSTONE				= 715,
	ITEMID_RED_GEMSTONE					= 716,
	ITEMID_BLUE_GEMSTONE				= 717,
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
	ITEMID_PHRACON						= 1010,
	ITEMID_EMVERETARCON					= 1011,
	ITEMID_TRAP							= 1065,
	ITEMID_PAINT_BRUSH					= 6122,
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
	ITEMID_MERCENARY_RED_POTION			= 12184,
	ITEMID_MERCENARY_BLUE_POTION		= 12185,
	ITEMID_BATTLE_MANUAL				= 12208,
	ITEMID_BUBBLE_GUM					= 12210,
	ITEMID_GIANT_FLY_WING				= 12212,
	ITEMID_NEURALIZER					= 12213,
	ITEMID_M_CENTER_POTION				= 12241,
	ITEMID_M_AWAKENING_POTION			= 12242,
	ITEMID_M_BERSERK_POTION				= 12243,
	ITEMID_COMP_BATTLE_MANUAL			= 12263,
	ITEMID_LOVE_ANGEL					= 12287,
	ITEMID_SQUIRREL						= 12288,
	ITEMID_GOGO							= 12289,
	ITEMID_PICTURE_DIARY				= 12304,
	ITEMID_MINI_HEART					= 12305,
	ITEMID_NEWCOMER						= 12306,
	ITEMID_KID							= 12307,
	ITEMID_MAGIC_CASTLE					= 12308,
	ITEMID_BULGING_HEAD					= 12309,
	ITEMID_THICK_BATTLE_MANUAL			= 12312,
	ITEMID_ANCILLA						= 12333,
	ITEMID_DUN_TELE_SCROLL3				= 12352,
	ITEMID_REINS_OF_MOUNT				= 12622,
	ITEMID_COMP_BUBBLE_GUM				= 12264,
	ITEMID_NOBLE_NAMEPLATE				= 12705,
	ITEMID_DUN_TELE_SCROLL1				= 14527,
	ITEMID_BATTLE_MANUAL25				= 14532,
	ITEMID_BATTLE_MANUAL100				= 14533,
	ITEMID_BATTLE_MANUAL300				= 14545,
	ITEMID_DUN_TELE_SCROLL2				= 14581,
	ITEMID_WOB_RUNE						= 14582,
	ITEMID_WOB_SCHWALTZ					= 14583,
	ITEMID_WOB_RACHEL					= 14584,
	ITEMID_WOB_LOCAL					= 14585,
	ITEMID_SIEGE_TELEPORT_SCROLL		= 14591,
	ITEMID_JOB_MANUAL50					= 14592,
};

///Mercenary Scrolls
enum mercenary_scroll_item_list
{
	ITEMID_BOW_MERCENARY_SCROLL1 = 12153,
	ITEMID_BOW_MERCENARY_SCROLL2,
	ITEMID_BOW_MERCENARY_SCROLL3,
	ITEMID_BOW_MERCENARY_SCROLL4,
	ITEMID_BOW_MERCENARY_SCROLL5,
	ITEMID_BOW_MERCENARY_SCROLL6,
	ITEMID_BOW_MERCENARY_SCROLL7,
	ITEMID_BOW_MERCENARY_SCROLL8,
	ITEMID_BOW_MERCENARY_SCROLL9,
	ITEMID_BOW_MERCENARY_SCROLL10,
	ITEMID_SWORDMERCENARY_SCROLL1,
	ITEMID_SWORDMERCENARY_SCROLL2,
	ITEMID_SWORDMERCENARY_SCROLL3,
	ITEMID_SWORDMERCENARY_SCROLL4,
	ITEMID_SWORDMERCENARY_SCROLL5,
	ITEMID_SWORDMERCENARY_SCROLL6,
	ITEMID_SWORDMERCENARY_SCROLL7,
	ITEMID_SWORDMERCENARY_SCROLL8,
	ITEMID_SWORDMERCENARY_SCROLL9,
	ITEMID_SWORDMERCENARY_SCROLL10,
	ITEMID_SPEARMERCENARY_SCROLL1,
	ITEMID_SPEARMERCENARY_SCROLL2,
	ITEMID_SPEARMERCENARY_SCROLL3,
	ITEMID_SPEARMERCENARY_SCROLL4,
	ITEMID_SPEARMERCENARY_SCROLL5,
	ITEMID_SPEARMERCENARY_SCROLL6,
	ITEMID_SPEARMERCENARY_SCROLL7,
	ITEMID_SPEARMERCENARY_SCROLL8,
	ITEMID_SPEARMERCENARY_SCROLL9,
	ITEMID_SPEARMERCENARY_SCROLL10,
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
	ITEMID_BOOST500_TO_THROW,
	ITEMID_FULL_SWINGK_TO_THROW,
	ITEMID_MANA_PLUS_TO_THROW,
	ITEMID_CURE_FREE_TO_THROW,
	ITEMID_STAMINA_UP_M_TO_THROW,
	ITEMID_DIGESTIVE_F_TO_THROW,
	ITEMID_HP_INC_POTS_TO_THROW,
	ITEMID_HP_INC_POTM_TO_THROW,
	ITEMID_HP_INC_POTL_TO_THROW,
	ITEMID_SP_INC_POTS_TO_THROW,
	ITEMID_SP_INC_POTM_TO_THROW,
	ITEMID_SP_INC_POTL_TO_THROW,
	ITEMID_EN_WHITE_POTZ_TO_THROW,
	ITEMID_VITATA500_TO_THROW,
	ITEMID_EN_CEL_JUICE_TO_THROW,
	ITEMID_SAVAGE_BBQ_TO_THROW,
	ITEMID_WUG_COCKTAIL_TO_THROW,
	ITEMID_M_BRISKET_TO_THROW,
	ITEMID_SIROMA_ICETEA_TO_THROW,
	ITEMID_DROCERA_STEW_TO_THROW,
	ITEMID_PETTI_NOODLE_TO_THROW,
	ITEMID_BLACK_THING_TO_THROW,
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

///Spell Books
enum spell_book_item_list
{
	ITEMID_MAGIC_BOOK_FB = 6189,
	ITEMID_MAGIC_BOOK_CB,
	ITEMID_MAGIC_BOOK_LB,
	ITEMID_MAGIC_BOOK_SG,
	ITEMID_MAGIC_BOOK_LOV,
	ITEMID_MAGIC_BOOK_MS,
	ITEMID_MAGIC_BOOK_CM,
	ITEMID_MAGIC_BOOK_TV,
	ITEMID_MAGIC_BOOK_TS,
	ITEMID_MAGIC_BOOK_JT,
	ITEMID_MAGIC_BOOK_WB,
	ITEMID_MAGIC_BOOK_HD,
	ITEMID_MAGIC_BOOK_ES,
	ITEMID_MAGIC_BOOK_ES_,
	ITEMID_MAGIC_BOOK_CL,
	ITEMID_MAGIC_BOOK_CR,
	ITEMID_MAGIC_BOOK_DL,
};

///Cash Food
enum cash_food_item_list
{
	ITEMID_STR_DISH10_  = 12202,
	ITEMID_AGI_DISH10_,
	ITEMID_INT_DISH10_,
	ITEMID_DEX_DISH10_,
	ITEMID_LUK_DISH10_,
	ITEMID_VIT_DISH10_,
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
} rd;

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
	unsigned int class_base[3];	//Specifies if the base can wear this item (split in 3 indexes per type: 1-1, 2-1, 2-2)
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
		unsigned delay_consume : 1;	// Signifies items that are not consumed immediately upon double-click [Skotlex]
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
#define itemdb_is_rune(n) ((n >= ITEMID_NAUTHIZ && n <= ITEMID_HAGALAZ) || n == ITEMID_LUX_ANIMA)
#define itemdb_is_element(n) (n >= ITEMID_SCARLET_PTS && n <= ITEMID_LIME_GREEN_PTS)
#define itemdb_is_spellbook(n) (n >= ITEMID_MAGIC_BOOK_FB && n <= ITEMID_MAGIC_BOOK_DL)
#define itemdb_is_poison(n) (n >= ITEMID_PARALYSE && n <= ITEMID_VENOMBLEED)
#define itemid_isgemstone(id) ( (id) >= ITEMID_YELLOW_GEMSTONE && (id) <= ITEMID_BLUE_GEMSTONE )
#define itemdb_iscashfood(id) ( (id) >= ITEMID_STR_DISH10_ && (id) <= ITEMID_VIT_DISH10_ )
#define itemdb_is_GNbomb(n) (n >= ITEMID_APPLE_BOMB && n <= ITEMID_VERY_HARD_LUMP)
#define itemdb_is_GNthrowable(n) (n >= ITEMID_MYSTERIOUS_POWDER && n <= ITEMID_BLACK_THING_TO_THROW)
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
char itemdb_pc_get_itemgroup(uint16 group_id, struct map_session_data *sd);
uint16 itemdb_get_randgroupitem_count(uint16 group_id, uint8 sub_group, unsigned short nameid);

bool itemdb_parse_roulette_db(void);

void itemdb_reload(void);

void do_final_itemdb(void);
void do_init_itemdb(void);

#endif /* _ITEMDB_H_ */
