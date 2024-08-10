// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLIF_HPP
#define CLIF_HPP

#include <cstdarg>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/db.hpp> //dbmap
#include <common/mmo.hpp>
#include <common/timer.hpp> // t_tick

#include "packets.hpp"
#include "script.hpp"

struct Channel;
struct clan;
struct item;
struct s_storage;
//#include "map.hpp"
struct block_list;
struct unit_data;
class map_session_data;
struct homun_data;
struct pet_data;
struct mob_data;
struct npc_data;
struct chat_data;
struct flooritem_data;
struct skill_unit;
struct s_vending;
struct party;
struct party_data;
struct mmo_guild;
struct s_battleground_data;
struct quest;
struct party_booking_ad_info;
struct sale_item_data;
struct mail_message;
struct achievement;
struct guild_log_entry;
enum e_guild_storage_log : uint16;
enum e_bg_queue_apply_ack : uint16;
enum e_instance_notify : uint8;
struct s_laphine_synthesis;
struct s_laphine_upgrade;
struct s_captcha_data;
enum e_macro_detect_status : uint8;
enum e_macro_report_status : uint8;
enum e_hom_state2 : uint8;
enum _sp;
enum e_searchstore_failure : uint16;

enum e_PacketDBVersion { // packet DB
	MIN_PACKET_DB  = 0x064,
	MAX_PACKET_DB  = 0xBFF,
#if !defined(MAX_PACKET_POS)
	MAX_PACKET_POS = 20,
#endif
};

struct s_packet_db {
	short len;
	void (*func)(int, map_session_data *);
	short pos[MAX_PACKET_POS];
};

#ifdef PACKET_OBFUSCATION
/// Keys based on packet versions
struct s_packet_keys {
	unsigned int keys[3]; ///< 3-Keys
};
#endif

enum e_CASHSHOP_ACK : uint8_t{
	ERROR_TYPE_NONE             = 0, ///< The deal has successfully completed.
	ERROR_TYPE_NPC              = 1, ///< The Purchase has failed because the NPC does not exist.
	ERROR_TYPE_SYSTEM           = 2, ///< The Purchase has failed because the Kafra Shop System is not working correctly.
	ERROR_TYPE_INVENTORY_WEIGHT = 3, ///< You are over your Weight Limit.
	ERROR_TYPE_EXCHANGE         = 4, ///< You cannot purchase items while you are in a trade.
	ERROR_TYPE_ITEM_ID          = 5, ///< The Purchase has failed because the Item Information was incorrect.
	ERROR_TYPE_MONEY            = 6, ///< You do not have enough Kafra Credit Points.
	ERROR_TYPE_AMOUNT           = 7, ///< You can purchase up to 10 items.
	ERROR_TYPE_PURCHASE_FAIL    = 8, ///< Some items could not be purchased.
};

enum e_BANKING_DEPOSIT_ACK : uint8_t {
	BDA_SUCCESS  = 0x0,
	BDA_ERROR    = 0x1,
	BDA_NO_MONEY = 0x2,
	BDA_OVERFLOW = 0x3,
};

enum e_BANKING_WITHDRAW_ACK : uint8_t {
	BWA_SUCCESS       = 0x0,
	BWA_NO_MONEY      = 0x1,
	BWA_UNKNOWN_ERROR = 0x2,
};

enum RECV_ROULETTE_ITEM_REQ : uint8_t {
	RECV_ITEM_SUCCESS    = 0x0,
	RECV_ITEM_FAILED     = 0x1,
	RECV_ITEM_OVERCOUNT  = 0x2,
	RECV_ITEM_OVERWEIGHT = 0x3,
};

enum RECV_ROULETTE_ITEM_ACK : uint8_t {
	RECV_ITEM_NORMAL =  0x0,
	RECV_ITEM_LOSING =  0x1,
};

enum GENERATE_ROULETTE_ACK : uint8_t {
	GENERATE_ROULETTE_SUCCESS         = 0x0,
	GENERATE_ROULETTE_FAILED          = 0x1,
	GENERATE_ROULETTE_NO_ENOUGH_POINT = 0x2,
	GENERATE_ROULETTE_LOSING          = 0x3,
};

enum OPEN_ROULETTE_ACK : uint8_t {
	OPEN_ROULETTE_SUCCESS = 0x0,
	OPEN_ROULETTE_FAILED  = 0x1,
};

enum CLOSE_ROULETTE_ACK : uint8_t {
	CLOSE_ROULETTE_SUCCESS = 0x0,
	CLOSE_ROULETTE_FAILED  = 0x1,
};

enum MERGE_ITEM_ACK : uint8_t {
	MERGE_ITEM_SUCCESS = 0x0,
	MERGE_ITEM_FAILED_NOT_MERGE = 0x1,
	MERGE_ITEM_FAILED_MAX_COUNT = 0x2,
};

enum BROADCASTING_SPECIAL_ITEM_OBTAIN : uint8_t {
	ITEMOBTAIN_TYPE_BOXITEM =  0x0,
	ITEMOBTAIN_TYPE_MONSTER_ITEM =  0x1,
	ITEMOBTAIN_TYPE_NPC =  0x2,
};

enum e_adopt_reply : uint8_t {
	ADOPT_REPLY_MORE_CHILDREN = 0,
	ADOPT_REPLY_LEVEL_70,
	ADOPT_REPLY_MARRIED,
};

enum e_wip_block : uint8_t {
	WIP_DISABLE_NONE = 0x0,
	WIP_DISABLE_SKILLITEM = 0x1,
	WIP_DISABLE_NPC = 0x2,
	WIP_DISABLE_ALL = 0x3,
};

enum e_party_invite_reply {
	PARTY_REPLY_JOIN_OTHER_PARTY = 0,   ///< result=0 : "The Character already joined another party." -> MsgStringTable[80]
	PARTY_REPLY_REJECTED,			    ///< result=1 : "Request for party rejected." -> MsgStringTable[81]
	PARTY_REPLY_ACCEPTED,			    ///< result=2 : "Request for party accepted." -> MsgStringTable[82]
	PARTY_REPLY_FULL,				    ///< result=3 : "Party Capacity exceeded." -> MsgStringTable[83]
	PARTY_REPLY_DUAL,				    ///< result=4 : "Character in the same account already joined." -> MsgStringTable[608]
	PARTY_REPLY_JOINMSG_REFUSE,		    ///< result=5 : "The character blocked the party invitation." -> MsgStringTable[1324] (since 20070904)
	PARTY_REPLY_UNKNOWN_ERROR,		    ///< result=6 : ??
	PARTY_REPLY_OFFLINE,			    ///< result=7 : "The Character is not currently online or does not exist." -> MsgStringTable[71] (since 20070904)
	PARTY_REPLY_INVALID_MAPPROPERTY,    ///< result=8 : !TODO "Unable to organize a party in this map" -> MsgStringTable[1388] (since 20080527)
	PARTY_REPLY_INVALID_MAPPROPERTY_ME, ///< return=9 : !TODO "Cannot join a party in this map" -> MsgStringTable[1871] (since 20110205)
	PARTY_REPLY_MEMORIALDUNGEON,	    ///< return=10: "You cannot invite or withdraw while in memorial dungeon" -> MsgStringTable[3027] (since 20161130)
};

/// Enum for Convex Mirror (SC_BOSSMAPINFO)
enum e_bossmap_info {
	BOSS_INFO_NOT = 0,
	BOSS_INFO_ALIVE,
	BOSS_INFO_ALIVE_WITHMSG,
	BOSS_INFO_DEAD,
};

enum class e_purchase_result : uint8{
	PURCHASE_SUCCEED = 0x0,
	PURCHASE_FAIL_MONEY,
	PURCHASE_FAIL_WEIGHT,
	PURCHASE_FAIL_COUNT,
	PURCHASE_FAIL_STOCK,
	PURCHASE_FAIL_ITEM_EXCHANGING,
	PURCHASE_FAIL_INVALID_MCSTORE,
	PURCHASE_FAIL_OPEN_MCSTORE_ITEMLIST,
	PURCHASE_FAIL_GIVE_MONEY,
	PURCHASE_FAIL_EACHITEM_COUNT,
	// Unknown names
	PURCHASE_FAIL_RODEX,
	PURCHASE_FAIL_EXCHANGE_FAILED,
	PURCHASE_FAIL_EXCHANGE_DONE,
	PURCHASE_FAIL_STOCK_EMPTY,
	PURCHASE_FAIL_GOODS,
	// End unknown names
	PURCHASE_FAIL_ADD = 0xff,
};

#define packet_len(cmd) packet_db[cmd].len
extern struct s_packet_db packet_db[MAX_PACKET_DB+1];

// local define
enum send_target : uint8_t {
	ALL_CLIENT = 0,
	ALL_SAMEMAP,
	AREA,				// area
	AREA_WOS,			// area, without self
	AREA_WOC,			// area, without chatrooms
	AREA_WOSC,			// area, without own chatroom
	AREA_CHAT_WOC,		// hearable area, without chatrooms
	CHAT,				// current chatroom
	CHAT_WOS,			// current chatroom, without self
	PARTY,
	PARTY_WOS,
	PARTY_SAMEMAP,
	PARTY_SAMEMAP_WOS,
	PARTY_AREA,
	PARTY_AREA_WOS,
	GUILD,
	GUILD_WOS,
	GUILD_SAMEMAP,
	GUILD_SAMEMAP_WOS,
	GUILD_AREA,
	GUILD_AREA_WOS,
	GUILD_NOBG,
	DUEL,
	DUEL_WOS,
	SELF,

	BG,					// BattleGround System
	BG_WOS,
	BG_SAMEMAP,
	BG_SAMEMAP_WOS,
	BG_AREA,
	BG_AREA_WOS,

	CLAN,				// Clan System
};

enum broadcast_flags : uint8_t {
	BC_ALL			= 0,
	BC_MAP			= 1,
	BC_AREA			= 2,
	BC_SELF			= 3,
	BC_TARGET_MASK	= 0x07,

	BC_PC			= 0x00,
	BC_NPC			= 0x08,
	BC_SOURCE_MASK	= 0x08, // BC_PC|BC_NPC

	BC_YELLOW		= 0x00,
	BC_BLUE			= 0x10,
	BC_WOE			= 0x20,
	BC_COLOR_MASK	= 0x30, // BC_YELLOW|BC_BLUE|BC_WOE

	BC_DEFAULT		= BC_ALL|BC_PC|BC_YELLOW
};

enum emotion_type {
	ET_SURPRISE = 0,	// /!
	ET_QUESTION,		// /?
	ET_DELIGHT,
	ET_THROB,
	ET_SWEAT,
	ET_AHA,
	ET_FRET,
	ET_ANGER,
	ET_MONEY,			// /$
	ET_THINK,			// /...
	ET_SCISSOR,     	// /gawi --- 10
	ET_ROCK,        	// /bawi
	ET_WRAP,       		// /bo
	ET_FLAG,
	ET_BIGTHROB,
	ET_THANKS,
	ET_KEK,
	ET_SORRY,
	ET_SMILE,
	ET_PROFUSELY_SWEAT,
	ET_SCRATCH,			// --- 20
	ET_BEST,
	ET_STARE_ABOUT,		// /??
	ET_HUK,
	ET_O,
	ET_X,
	ET_HELP,
	ET_GO,
	ET_CRY,
	ET_KIK,
	ET_CHUP,			// --- 30
	ET_CHUPCHUP,
	ET_HNG,
	ET_OK,
	ET_CHAT_PROHIBIT,	// red /... used for muted characters
	ET_INDONESIA_FLAG,
	ET_STARE,			// /bzz, /stare
	ET_HUNGRY,
	ET_COOL,			// /awsm, /cool
	ET_MERONG,
	ET_SHY,				// --- 40
	ET_GOODBOY,			// /pat, /goodboy
	ET_SPTIME,			// /mp, /sptime
	ET_SEXY,
	ET_COMEON,			// /com, /comeon
	ET_SLEEPY,			// /yawn, /sleepy
	ET_CONGRATULATION,	// /grat, /congrats
	ET_HPTIME,			// /hp, /hptime
	ET_PH_FLAG,
	ET_MY_FLAG,
	ET_SI_FLAG,			// --- 50
	ET_BR_FLAG,
	ET_SPARK,			// /fsh
	ET_CONFUSE,			// /spin
	ET_OHNO,
	ET_HUM,				// /dum
	ET_BLABLA,			// /crwd
	ET_OTL,				// /otl, /desp
	ET_DICE1,
	ET_DICE2,
	ET_DICE3,			// --- 60
	ET_DICE4,
	ET_DICE5,
	ET_DICE6,
	ET_INDIA_FLAG,
	ET_LUV,				// /love
	ET_FLAG8,
	ET_FLAG9,
	ET_MOBILE,
	ET_MAIL,
	ET_ANTENNA0,		// --- 70
	ET_ANTENNA1,
	ET_ANTENNA2,
	ET_ANTENNA3,
	ET_HUM2,
	ET_ABS,
	ET_OOPS,
	ET_SPIT,
	ET_ENE,
	ET_PANIC,
	ET_WHISP,			// --- 80
	ET_YUT1,
	ET_YUT2,
	ET_YUT3,
	ET_YUT4,
	ET_YUT5,
	ET_YUT6,
	ET_YUT7,
	//
	ET_MAX
};

enum clr_type : uint8_t 
{
	CLR_OUTSIGHT = 0,
	CLR_DEAD,
	CLR_RESPAWN,
	CLR_TELEPORT,
	CLR_TRICKDEAD,
};

enum map_property : uint8_t 
{// clif_map_property
	MAPPROPERTY_NOTHING       = 0,
	MAPPROPERTY_FREEPVPZONE   = 1,
	MAPPROPERTY_EVENTPVPZONE  = 2,
	MAPPROPERTY_AGITZONE      = 3,
	MAPPROPERTY_PKSERVERZONE  = 4, // message "You are in a PK area. Please beware of sudden attacks." in color 0x9B9BFF (light red)
	MAPPROPERTY_PVPSERVERZONE = 5,
	MAPPROPERTY_DENYSKILLZONE = 6,
};

enum e_map_type : uint8_t 
{// clif_map_type
	MAPTYPE_VILLAGE              = 0,
	MAPTYPE_VILLAGE_IN           = 1,
	MAPTYPE_FIELD                = 2,
	MAPTYPE_DUNGEON              = 3,
	MAPTYPE_ARENA                = 4,
	MAPTYPE_PENALTY_FREEPKZONE   = 5,
	MAPTYPE_NOPENALTY_FREEPKZONE = 6,
	MAPTYPE_EVENT_GUILDWAR       = 7,
	MAPTYPE_AGIT                 = 8,
	MAPTYPE_DUNGEON2             = 9,
	MAPTYPE_DUNGEON3             = 10,
	MAPTYPE_PKSERVER             = 11,
	MAPTYPE_PVPSERVER            = 12,
	MAPTYPE_DENYSKILL            = 13,
	MAPTYPE_TURBOTRACK           = 14,
	MAPTYPE_JAIL                 = 15,
	MAPTYPE_MONSTERTRACK         = 16,
	MAPTYPE_PORINGBATTLE         = 17,
	MAPTYPE_AGIT_SIEGEV15        = 18,
	MAPTYPE_BATTLEFIELD          = 19,
	MAPTYPE_PVP_TOURNAMENT       = 20,
	//Map types 21 - 24 not used.
	MAPTYPE_SIEGE_LOWLEVEL       = 25,
	//Map types 26 - 28 remains opens for future types.
	MAPTYPE_UNUSED               = 29,
};

enum useskill_fail_cause : uint8_t 
{// clif_skill_fail
	USESKILL_FAIL_LEVEL = 0,
	USESKILL_FAIL_SP_INSUFFICIENT = 1,
	USESKILL_FAIL_HP_INSUFFICIENT = 2,
	USESKILL_FAIL_STUFF_INSUFFICIENT = 3,
	USESKILL_FAIL_SKILLINTERVAL = 4,
	USESKILL_FAIL_MONEY = 5,
	USESKILL_FAIL_THIS_WEAPON = 6,
	USESKILL_FAIL_REDJAMSTONE = 7,
	USESKILL_FAIL_BLUEJAMSTONE = 8,
	USESKILL_FAIL_WEIGHTOVER = 9,
	USESKILL_FAIL = 10,
	USESKILL_FAIL_TOTARGET = 11,
	USESKILL_FAIL_ANCILLA_NUMOVER = 12,
	USESKILL_FAIL_HOLYWATER = 13,
	USESKILL_FAIL_ANCILLA = 14,
	USESKILL_FAIL_DUPLICATE_RANGEIN = 15,
	USESKILL_FAIL_NEED_OTHER_SKILL = 16,
	USESKILL_FAIL_NEED_HELPER = 17,
	USESKILL_FAIL_INVALID_DIR = 18,
	USESKILL_FAIL_SUMMON = 19,
	USESKILL_FAIL_SUMMON_NONE = 20,
	USESKILL_FAIL_IMITATION_SKILL_NONE = 21,
	USESKILL_FAIL_DUPLICATE = 22,
	USESKILL_FAIL_CONDITION = 23,
	USESKILL_FAIL_PAINTBRUSH = 24,
	USESKILL_FAIL_DRAGON = 25,
	USESKILL_FAIL_POS = 26,
	USESKILL_FAIL_HELPER_SP_INSUFFICIENT = 27,
	USESKILL_FAIL_NEER_WALL = 28,
	USESKILL_FAIL_NEED_EXP_1PERCENT = 29,
	USESKILL_FAIL_CHORUS_SP_INSUFFICIENT = 30,
	USESKILL_FAIL_GC_WEAPONBLOCKING = 31,
	USESKILL_FAIL_GC_POISONINGWEAPON = 32,
	USESKILL_FAIL_MADOGEAR = 33,
	USESKILL_FAIL_NEED_EQUIPMENT_KUNAI = 34,
	USESKILL_FAIL_TOTARGET_PLAYER = 35,
	USESKILL_FAIL_SIZE = 36,
	USESKILL_FAIL_CANONBALL = 37,
	//XXX_USESKILL_FAIL_II_MADOGEAR_ACCELERATION = 38,
	//XXX_USESKILL_FAIL_II_MADOGEAR_HOVERING_BOOSTER = 39,
	USESKILL_FAIL_MADOGEAR_HOVERING = 40,
	//XXX_USESKILL_FAIL_II_MADOGEAR_SELFDESTRUCTION_DEVICE = 41,
	//XXX_USESKILL_FAIL_II_MADOGEAR_SHAPESHIFTER = 42,
	USESKILL_FAIL_GUILLONTINE_POISON = 43,
	//XXX_USESKILL_FAIL_II_MADOGEAR_COOLING_DEVICE = 44,
	//XXX_USESKILL_FAIL_II_MADOGEAR_MAGNETICFIELD_GENERATOR = 45,
	//XXX_USESKILL_FAIL_II_MADOGEAR_BARRIER_GENERATOR = 46,
	//XXX_USESKILL_FAIL_II_MADOGEAR_OPTICALCAMOUFLAGE_GENERATOR = 47,
	//XXX_USESKILL_FAIL_II_MADOGEAR_REPAIRKIT = 48,
	//XXX_USESKILL_FAIL_II_MONKEY_SPANNER = 49,
	USESKILL_FAIL_MADOGEAR_RIDE = 50,
	USESKILL_FAIL_SPELLBOOK = 51,
	USESKILL_FAIL_SPELLBOOK_DIFFICULT_SLEEP = 52,
	USESKILL_FAIL_SPELLBOOK_PRESERVATION_POINT = 53,
	USESKILL_FAIL_SPELLBOOK_READING = 54,
	//XXX_USESKILL_FAIL_II_FACE_PAINTS = 55,
	//XXX_USESKILL_FAIL_II_MAKEUP_BRUSH = 56,
	USESKILL_FAIL_CART = 57,
	//XXX_USESKILL_FAIL_II_THORNS_SEED = 58,
	//XXX_USESKILL_FAIL_II_BLOOD_SUCKER_SEED = 59,
	USESKILL_FAIL_NO_MORE_SPELL = 60,
	//XXX_USESKILL_FAIL_II_BOMB_MUSHROOM_SPORE = 61,
	//XXX_USESKILL_FAIL_II_GASOLINE_BOOMB = 62,
	//XXX_USESKILL_FAIL_II_OIL_BOTTLE = 63,
	//XXX_USESKILL_FAIL_II_EXPLOSION_POWDER = 64,
	//XXX_USESKILL_FAIL_II_SMOKE_POWDER = 65,
	//XXX_USESKILL_FAIL_II_TEAR_GAS = 66,
	//XXX_USESKILL_FAIL_II_HYDROCHLORIC_ACID_BOTTLE = 67,
	//XXX_USESKILL_FAIL_II_HELLS_PLANT_BOTTLE = 68,
	//XXX_USESKILL_FAIL_II_MANDRAGORA_FLOWERPOT = 69,
	USESKILL_FAIL_MANUAL_NOTIFY = 70,
	// CAUTION: client uses unidentified display name for displaying the required item. Still broken on 2017-05-31 [Lemongrass]
	USESKILL_FAIL_NEED_ITEM = 71,
	USESKILL_FAIL_NEED_EQUIPMENT = 72,
	USESKILL_FAIL_COMBOSKILL = 73,
	USESKILL_FAIL_SPIRITS = 74,
	USESKILL_FAIL_EXPLOSIONSPIRITS = 75,
	USESKILL_FAIL_HP_TOOMANY = 76,
	USESKILL_FAIL_NEED_ROYAL_GUARD_BANDING = 77,
	USESKILL_FAIL_NEED_EQUIPPED_WEAPON_CLASS = 78,
	USESKILL_FAIL_EL_SUMMON = 79,
	USESKILL_FAIL_RELATIONGRADE = 80,
	USESKILL_FAIL_STYLE_CHANGE_FIGHTER = 81,
	USESKILL_FAIL_STYLE_CHANGE_GRAPPLER = 82,
	USESKILL_FAIL_THERE_ARE_NPC_AROUND = 83,
	USESKILL_FAIL_NEED_MORE_BULLET = 84,
	USESKILL_FAIL_COINS = 85,
	// 86-99 unknown
	USESKILL_FAIL_AP_INSUFFICIENT = 100,
	USESKILL_FAIL_MAX
};

enum clif_equipitemack_flag : uint8_t {
#if PACKETVER_MAIN_NUM >= 20121205 || PACKETVER_RE_NUM >= 20121107 || defined(PACKETVER_ZERO)
	ITEM_EQUIP_ACK_OK = 0,
	ITEM_EQUIP_ACK_FAIL = 2,
	ITEM_EQUIP_ACK_FAILLEVEL = 1,
#else
	ITEM_EQUIP_ACK_OK = 1,
	ITEM_EQUIP_ACK_FAIL = 0,
	ITEM_EQUIP_ACK_FAILLEVEL = 0,
#endif
};

//! NOTE: These values below need client version validation
// These values correspond to the msgstringtable line number minus 1
enum clif_messages : uint16_t {

	// You cannot carry more items because you are overweight.
	MSI_CANT_GET_ITEM_BECAUSE_WEIGHT = 52,

	// You can't have this item because you will exceed the weight limit.
	MSI_CANT_GET_ITEM_BECAUSE_COUNT = 220,

	// You can't put this item on.
	MSI_CAN_NOT_EQUIP_ITEM = 372,

	// You cannot use this item while sitting.
	MSI_CANT_USE_WHEN_SITDOWN = 663,

	// The party member was not summoned because you are not the party leader.
	MSI_CANNOT_PARTYCALL = 1221,

	// There is no party member to summon in the current map.
	MSI_NO_PARTYMEM_ON_THISMAP = 1222,

	// The mercenary contract has expired.
	MSI_MER_FINISH = 1266,

	// The mercenary has died.
	MSI_MER_DIE = 1267,

	// This skill cannot be used within this area.
	MSI_IMPOSSIBLE_SKILL_AREA = 1334,

	// This item cannot be used within this area.
	MSI_IMPOSSIBLE_USEITEM_AREA = 1335,

	// This character's equipment information is not open to the public.
	MSI_OPEN_EQUIPEDITEM_REFUSED = 1357,

	// Item can only be used when Mado Gear is mounted.
	MSI_USESKILL_FAIL_MADOGEAR = 1435,

	// Only available when cart is mounted.
	MSI_USESKILL_FAIL_CART = 1519,

	// Cannot create rune stone more than the maximum amount.
	MSI_RUNESTONE_MAKEERROR_OVERCOUNT = 1563,

	// Combination of item is not possible in conversion.
	MSI_SKILL_RECIPE_NOTEXIST = 1570,

	// Please ensure an extra space in your inventory.
	MSI_SKILL_INVENTORY_KINDCNT_OVER = 1572,

	// Successful.
	MSI_SKILL_SUCCESS = 1574,

	// Failed.
	MSI_SKILL_FAIL = 1575,

	// Items cannot be used in materials cannot be emotional.
	MSI_SKILL_FAIL_MATERIAL_IDENTITY = 1581,

	// [Bow] must be equipped.
	MSI_FAIL_NEED_EQUIPPED_BOW = 1691,							

#if (PACKETVER >= 20130807 && PACKETVER <= 20130814) && !defined(PACKETVER_ZERO)
	// %d seconds left until you can use
	MSI_ITEM_REUSE_LIMIT_SECOND = 1863,

	// Any work in progress (NPC dialog, manufacturing ...) quit and try again.
	MSI_BUSY = 1924,

	// While boarding reins is not available for items.
	MSI_FAIELD_RIDING_OVERLAPPED = 1932,

	// It is only possible to change the party leader while on the same map.
	MSI_PARTY_MASTER_CHANGE_SAME_MAP = 2095,

	// Merge items available does not exist.
	MSI_NOT_EXIST_MERGE_ITEM = 2184,

	// This bullet is not suitable for the weapon you are equipping.
	MSI_WRONG_BULLET = 2494,
#else
	// %d seconds left until you can use
	MSI_ITEM_REUSE_LIMIT_SECOND = 1862,

	// Any work in progress (NPC dialog, manufacturing ...) quit and try again.
	MSI_BUSY = 1923,

	// While boarding reins is not available for items.
	MSI_FAIELD_RIDING_OVERLAPPED = 1931,

	// It is only possible to change the party leader while on the same map.
	MSI_PARTY_MASTER_CHANGE_SAME_MAP = 2094,

	// Merge items available does not exist.
	MSI_NOT_EXIST_MERGE_ITEM = 2183,

	// This bullet is not suitable for the weapon you are equipping.
	MSI_WRONG_BULLET = 2493,
#endif

	// [Gatling Gun] weapon class must be equipped.
	MSI_FAIL_NEED_EQUIPPED_GUN_GATLING = 2554,

	// [Shotgun] weapon class must be equipped.
	MSI_FAIL_NEED_EQUIPPED_GUN_SHOTGUN = 2555,

	// [Rifle] weapon class must be equipped.
	MSI_FAIL_NEED_EQUIPPED_GUN_RIFLE = 2556,

	// [Revolver] weapon class must be equipped.
	MSI_FAIL_NEED_EQUIPPED_GUN_HANDGUN = 2557,

	// [Silver Bullet] must be equipped.
	MSI_FAIL_NEED_EQUIPPED_PROPERTY_SAINT_BULLET = 2558,

	// [Holy Water] must be equipped.
	MSI_FAIL_NEED_EQUIPPED_GUN_GRANADE = 2561,

	// [Grenade Launcher] weapon class must be equipped.
	MSI_IMPOSSIBLE_CHANGE_GUILD_MASTER_IN_SIEGE_TIME = 2963,

	// You have to wait for one day before delegating a new Guild leader
	MSI_IMPOSSIBLE_CHANGE_GUILD_MASTER_NOT_TIME = 2964,

	// This item has been set for Swap Equipment.
	MSI_SWAP_EQUIPITEM_UNREGISTER_FIRST = 3015,

	// Possession limit is over 70%, or you have less than 10 free inventory space.
	MSI_PICKUP_FAILED_ITEMCREATE = 3022,

	// Currently there is no attendance check event.
	MSI_CHECK_ATTENDANCE_NOT_EVENT = 3474,

	// The total amount of items to sell exceeds the amount of Zeny you can have. \nPlease modify the quantity and price.
	MSI_MERCHANTSHOP_TOTA_LOVER_ZENY_ERR = 3826,

	// It weighs more than 70%. Decrease the Weight and try again.
	MSI_ENCHANT_FAILED_OVER_WEIGHT = 3837,

	// Enchantment successful!
	MSI_ENCHANT_SUCCESS = 3857,

	// Enchantment failed!
	MSI_ENCHANT_FAILED = 3858,
};

enum e_personalinfo : uint8_t {
	PINFO_BASIC = 0,
	PINFO_PREMIUM,
	PINFO_SERVER,
	PINFO_CAFE,
	PINFO_MAX,
};

enum e_damage_type : uint8_t {
	DMG_NORMAL = 0,			/// damage [ damage: total damage, div: amount of hits, damage2: assassin dual-wield damage ]
	DMG_PICKUP_ITEM,		/// pick up item
	DMG_SIT_DOWN,			/// sit down
	DMG_STAND_UP,			/// stand up
	DMG_ENDURE,				/// damage (endure)
	DMG_SPLASH,				/// (splash?)
	DMG_SINGLE,				/// (skill?)
	DMG_REPEAT,				/// (repeat damage?)
	DMG_MULTI_HIT,			/// multi-hit damage
	DMG_MULTI_HIT_ENDURE,	/// multi-hit damage (endure)
	DMG_CRITICAL,			/// critical hit
	DMG_LUCY_DODGE,			/// lucky dodge
	DMG_TOUCH,				/// (touch skill?)
	DMG_MULTI_HIT_CRITICAL  /// multi-hit with critical
};

enum class e_pet_evolution_result : uint32 {
	FAIL_UNKNOWN = 0x0,
	FAIL_NOTEXIST_CALLPET = 0x1,
	FAIL_NOT_PETEGG = 0x2,
	FAIL_RECIPE = 0x3,
	FAIL_MATERIAL = 0x4,
	FAIL_RG_FAMILIAR = 0x5,
	SUCCESS = 0x6
};

enum e_config_type : uint32 {
	CONFIG_OPEN_EQUIPMENT_WINDOW = 0,
	CONFIG_CALL,
	CONFIG_PET_AUTOFEED,
	CONFIG_HOMUNCULUS_AUTOFEED
};

enum e_memorial_dungeon_command : uint16 {
	COMMAND_MEMORIALDUNGEON_DESTROY_FORCE = 0x3,
};

enum e_exitem_add_result : uint8 {
	EXITEM_ADD_SUCCEED,
	EXITEM_ADD_FAILED_OVERWEIGHT,
	EXITEM_ADD_FAILED_CLOSED,
	EXITEM_ADD_FAILED_OVERCOUNT,
	EXITEM_ADD_FAILED_EACHITEM_OVERCOUNT,
};

enum e_dynamicnpc_result : int32{
	DYNAMICNPC_RESULT_SUCCESS,
	DYNAMICNPC_RESULT_UNKNOWN,
	DYNAMICNPC_RESULT_UNKNOWNNPC,
	DYNAMICNPC_RESULT_DUPLICATE,
	DYNAMICNPC_RESULT_OUTOFTIME
};

enum e_siege_teleport_result : uint8 {
	SIEGE_TP_SUCCESS = 0,
	SIEGE_TP_NOT_ENOUGH_ZENY = 1,
	SIEGE_TP_INVALID_MODE = 2
};

enum e_ack_remember_warppoint_result : uint8 {
	WARPPOINT_SUCCESS = 0,
	WARPPOINT_LOW_LEVEL = 1,
	WARPPOINT_NOT_LEARNED = 2
};

enum e_notify_mapinfo_result : uint8 {
	NOTIFY_MAPINFO_CANT_TP = 0,
	NOTIFY_MAPINFO_CANT_MEMO = 1,
	NOTIFY_MAPINFO_CANT_USE_SKILL = 2,
	NOTIFY_MAPINFO_CANT_USE_ITEM = 3,
};

enum e_ack_itemrefining : uint8 {
	ITEMREFINING_SUCCESS = 0,
	ITEMREFINING_FAILURE = 1,
	ITEMREFINING_DOWNGRADE = 2,
	ITEMREFINING_FAILURE2 = 3
};

enum e_refuse_enter_room : uint8 {
	ENTERROOM_FULL = 0,
	ENTERROOM_WRONG_PASSWORD = 1,
	ENTERROOM_KICKED = 2,
	ENTERROOM_SUCCESS = 3,
	ENTERROOM_NO_ZENY = 4,
	ENTERROOM_TOO_LOW_LEVEL = 5,
	ENTERROOM_TOO_HIGH_LEVEL = 6,
	ENTERROOM_UNSUITABLE_JOB = 7
};

enum e_create_chatroom : uint8 {
	CREATEROOM_SUCCESS = 0,
	CREATEROOM_LIMIT_EXCEEDED = 1,
	CREATEROOM_ALREADY_EXISTS = 2
};

enum e_action_failure : uint8 {
	ARROWFAIL_NO_AMMO = 0,
	ARROWFAIL_WEIGHT_LIMIT = 1,
	ARROWFAIL_WEIGHT_LIMIT2 = 2,
	ARROWFAIL_SUCCESS = 3
};

enum e_notify_effect : uint8 {
	NOTIFYEFFECT_BASE_LEVEL_UP = 0,
	NOTIFYEFFECT_JOB_LEVEL_UP = 1,
	NOTIFYEFFECT_REFINE_FAILURE = 2,
	NOTIFYEFFECT_REFINE_SUCCESS = 3,
	NOTIFYEFFECT_GAME_OVER = 4,
	NOTIFYEFFECT_PHARMACY_SUCCESS = 5,
	NOTIFYEFFECT_PHARMACY_FAILURE = 6,
	NOTIFYEFFECT_SUPER_NOVICE_BASE_LEVEL_UP = 7,
	NOTIFYEFFECT_SUPER_NOVICE_JOB_LEVEL_UP = 8,
	NOTIFYEFFECT_TAEKWON_BASE_LEVEL_UP = 9,
};

enum e_pc_purchase_result_frommc : uint8 {
	PURCHASEMC_SUCCESS = 0,
	PURCHASEMC_NO_ZENY = 1,
	PURCHASEMC_OVERWEIGHT = 2,
	PURCHASEMC_OUT_OF_STOCK = 4,
	PURCHASEMC_TRADING = 5,
	PURCHASEMC_STORE_INCORRECT = 6,
	PURCHASEMC_NO_SALES_INFO = 7,
};

enum e_ack_openstore2 : uint8 {
	// Success
	OPENSTORE2_SUCCESS = 0,

	// (Pop-up) Failed to open stalls. (MSI_MERCHANTSHOP_MAKING_FAIL / 2639)
	OPENSTORE2_FAILED = 1,

	// 2 is unused

#if PACKETVER >= 20170419
	// Unable to open a shop at the current location. (MSI_MERCHANTSHOP_FAIL_POSITION / 3229)
	OPENSTORE2_NOVENDING = 3,
#endif
};

enum e_ack_whisper : uint8 {
	ACKWHISPER_SUCCESS = 0,
	ACKWHISPER_TARGET_OFFLINE = 1,
	ACKWHISPER_IGNORED = 2,
	ACKWHISPER_ALL_IGNORED = 3
};

enum e_ack_additem_to_cart : uint8 {
	ADDITEM_TO_CART_FAIL_WEIGHT = 0,
	ADDITEM_TO_CART_FAIL_COUNT = 1
};

int clif_setip(const char* ip);
void clif_setbindip(const char* ip);
void clif_setport(uint16 port);

uint32 clif_getip(void);
uint32 clif_refresh_ip(void);
uint16 clif_getport(void);

void clif_authok(map_session_data *sd);
void clif_authrefuse(int fd, uint8 error_code);
void clif_authfail_fd(int fd, int type);
void clif_charselectok(int id, uint8 ok);
void clif_dropflooritem(struct flooritem_data* fitem, bool canShowEffect);
void clif_clearflooritem( flooritem_data& fitem, map_session_data* tsd = nullptr );

void clif_clearunit_single( uint32 GID, clr_type type, map_session_data& tsd );
void clif_clearunit_area( block_list& bl, clr_type type );
void clif_clearunit_delayed(struct block_list* bl, clr_type type, t_tick tick);
int clif_spawn(struct block_list *bl, bool walking = false);	//area
void clif_walkok( map_session_data& sd );
void clif_move( struct unit_data& ud ); //area
void clif_changemap( map_session_data& sd, short m, uint16 x, uint16 y );
void clif_changemapserver( map_session_data& sd, const char* map, uint16 x, uint16 y, uint32 ip, uint16 port );
void clif_blown(struct block_list *bl); // area
void clif_slide(struct block_list *bl, int x, int y); // area
void clif_fixpos( block_list& bl );
void clif_npcbuysell( map_session_data& sd, npc_data& nd );
void clif_buylist( map_session_data& sd, npc_data& nd );
void clif_selllist( map_session_data& sd );
void clif_npc_market_open( map_session_data& sd, npc_data& nd );
void clif_parse_NPCMarketClosed(int fd, map_session_data *sd);
void clif_parse_NPCMarketPurchase(int fd, map_session_data *sd);
void clif_scriptmes( map_session_data& sd, uint32 npcid, const char *mes );
void clif_scriptnext( map_session_data& sd, uint32 npcid );
void clif_scriptclose( map_session_data& sd, uint32 npcid );
void clif_scriptclear( map_session_data& sd, int npcid ); //self
void clif_scriptmenu( map_session_data& sd, uint32 npcid, const char* mes );
void clif_scriptinput( map_session_data& sd, uint32 npcid );
void clif_scriptinputstr( map_session_data& sd, uint32 npcid );
void clif_cutin( map_session_data& sd, const char* image, int type );
void clif_viewpoint( map_session_data& sd, uint32 npc_id, int type, uint16 x, uint16 y, int id, uint32 color );
void clif_additem(map_session_data *sd, int n, int amount, unsigned char fail); // self
void clif_dropitem( map_session_data& sd, int index, int amount );
void clif_delitem( map_session_data& sd, int index, int amount, short reason );
void clif_update_hp(map_session_data &sd);
void clif_updatestatus( map_session_data& sd, _sp type );
void clif_changemanner( map_session_data& sd );
int clif_damage(struct block_list* src, struct block_list* dst, t_tick tick, int sdelay, int ddelay, int64 sdamage, int div, enum e_damage_type type, int64 sdamage2, bool spdamage);	// area
void clif_takeitem(struct block_list* src, struct block_list* dst);
void clif_sitting(struct block_list* bl);
void clif_standing(struct block_list* bl);
void clif_sprite_change(struct block_list *bl, int id, int type, int val, int val2, enum send_target target);
void clif_changelook(struct block_list *bl,int type,int val);	// area
void clif_changetraplook(struct block_list *bl,int val); // area
void clif_refreshlook(struct block_list *bl,int id,int type,int val,enum send_target target); //area specified in 'target'
void clif_arrowequip( map_session_data& sd );
void clif_arrow_fail( map_session_data& sd, e_action_failure type );
void clif_arrow_create_list( map_session_data& sd );
void clif_statusupack( map_session_data& sd, int32 type, bool success, int32 val = 0 );
void clif_equipitemack( map_session_data& sd, uint8 flag, int index, int pos = 0 ); // self
void clif_unequipitemack(map_session_data *sd,int n,int pos,int ok);	// self
void clif_misceffect( block_list& bl, e_notify_effect type );
void clif_changeoption_target(struct block_list* bl, struct block_list* target);
#define clif_changeoption(bl) clif_changeoption_target(bl, nullptr)	// area
void clif_changeoption2( block_list& bl );
void clif_useitemack(map_session_data *sd,int index,int amount,bool ok);	// self
void clif_GlobalMessage( block_list& bl, const char* message, enum send_target target );
void clif_createchat( map_session_data& sd, e_create_chatroom flag );
void clif_dispchat(struct chat_data* cd, int fd);	// area or fd
void clif_joinchatfail( map_session_data& sd, e_refuse_enter_room result );
void clif_joinchatok(map_session_data *sd,struct chat_data* cd);	// self
void clif_addchat(struct chat_data* cd,map_session_data *sd);	// chat
void clif_changechatowner(struct chat_data* cd, map_session_data* sd);	// chat
void clif_clearchat(struct chat_data *cd,int fd);	// area or fd
void clif_leavechat(struct chat_data* cd, map_session_data* sd, bool flag);	// chat
void clif_changechatstatus(struct chat_data* cd);	// chat
void clif_refresh_storagewindow(map_session_data *sd);
void clif_refresh(map_session_data *sd);	// self

void clif_emotion(struct block_list *bl,int type);
void clif_talkiebox(struct block_list* bl, const char* talkie);
void clif_wedding_effect(struct block_list *bl);
void clif_divorced(map_session_data* sd, const char* name);
void clif_callpartner(map_session_data& sd);
void clif_playBGM( map_session_data& sd, const char* name );
void clif_soundeffect( struct block_list& bl, const char* name, int type, enum send_target target );
void clif_parse_ActionRequest_sub( map_session_data& sd, int action_type, int target_id, t_tick tick );
void clif_parse_LoadEndAck(int fd,map_session_data *sd);
void clif_hotkeys_send(map_session_data *sd, int tab);

// trade
void clif_traderequest(map_session_data* sd, const char* name);
void clif_tradestart(map_session_data* sd, uint8 type);
void clif_tradeadditem(map_session_data* sd, map_session_data* tsd, int index, int amount);
void clif_tradeitemok(map_session_data& sd, int index, e_exitem_add_result result);
void clif_tradedeal_lock( map_session_data& sd, bool who );
void clif_tradecancelled( map_session_data& sd );
void clif_tradecompleted( map_session_data& sd );
void clif_tradeundo( map_session_data& sd );

// storage
void clif_storagelist(map_session_data* sd, struct item* items, int items_length, const char *storename);
void clif_updatestorageamount( map_session_data& sd, uint16 amount, uint16 max_amount );
void clif_storageitemadded(map_session_data* sd, struct item* i, int index, int amount);
void clif_storageitemremoved( map_session_data& sd, uint16 index, uint32 amount );
void clif_storageclose( map_session_data& sd );

int clif_insight(struct block_list *bl,va_list ap);	// map_forallinmovearea callback
int clif_outsight(struct block_list *bl,va_list ap);	// map_forallinmovearea callback

void clif_class_change_target(struct block_list *bl,int class_, int type, enum send_target target, map_session_data *sd);
#define clif_class_change(bl, class_, type) clif_class_change_target(bl, class_, type, AREA, nullptr)
#define clif_mob_class_change(md, class_) clif_class_change(&md->bl, class_, 1)

void clif_skillinfoblock(map_session_data *sd);
void clif_skillup(map_session_data *sd, uint16 skill_id, int lv, int range, int upgradable);
void clif_skillinfo(map_session_data *sd,int skill_id, int inf);
void clif_addskill(map_session_data *sd, int skill_id);
void clif_deleteskill(map_session_data *sd, int skill_id, bool skip_infoblock = false);

void clif_skillcasting(struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, uint16 skill_lv, int property, int casttime);
void clif_skillcastcancel( block_list& bl );
void clif_skill_fail( map_session_data& sd, uint16 skill_id, enum useskill_fail_cause cause = USESKILL_FAIL_LEVEL, int btype = 0, t_itemid itemId = 0 );
void clif_skill_cooldown( map_session_data &sd, uint16 skill_id, t_tick tick );
int clif_skill_damage(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int64 sdamage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type);
//int clif_skill_damage2(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int damage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type);
bool clif_skill_nodamage(struct block_list *src,struct block_list *dst,uint16 skill_id,int heal,t_tick tick);
void clif_skill_poseffect(struct block_list *src,uint16 skill_id,int val,int x,int y,t_tick tick);
void clif_skill_estimation(map_session_data *sd,struct block_list *dst);
void clif_skill_warppoint( map_session_data* sd, uint16 skill_id, uint16 skill_lv, const char* map1, const char* map2 = "", const char* map3 = "", const char* map4 = "" );
void clif_skill_memomessage( map_session_data& sd, e_ack_remember_warppoint_result result );
void clif_skill_teleportmessage( map_session_data& sd, e_notify_mapinfo_result result );
void clif_skill_produce_mix_list( map_session_data& sd, int skill_id, int trigger );
void clif_cooking_list( map_session_data& sd, int trigger, uint16 skill_id, int qty, int list_type );

void clif_produceeffect(map_session_data* sd,int flag, t_itemid nameid);

void clif_getareachar_skillunit(struct block_list *bl, struct skill_unit *unit, enum send_target target, bool visible);
void clif_skill_delunit( skill_unit& unit );
void clif_skillunit_update( block_list& bl);

void clif_skill_unit_test(struct block_list *bl, short x, short y, int unit_id, short range, short skill_lv);

void clif_autospell( map_session_data& sd, uint16 skill_lv );
void clif_devotion(struct block_list *src, map_session_data *tsd);
void clif_spiritball( struct block_list *bl, struct block_list* target = nullptr, enum send_target send_target = AREA );
void clif_soulball( map_session_data *sd, struct block_list* target = nullptr, enum send_target send_target = AREA );
void clif_servantball( map_session_data& sd, struct block_list* target = nullptr, enum send_target send_target = AREA );
void clif_abyssball( map_session_data& sd, struct block_list* target = nullptr, enum send_target send_target = AREA );
void clif_combo_delay(struct block_list *bl,t_tick wait);
void clif_bladestop(struct block_list *src, int dst_id, int active);
void clif_changemapcell(int fd, int16 m, int x, int y, int type, enum send_target target);

#define clif_status_load(bl, type, flag) clif_status_change((bl), (type), (flag), 0, 0, 0, 0)
void clif_status_change(struct block_list *bl, int type, int flag, t_tick tick, int val1, int val2, int val3);
void clif_efst_status_change(struct block_list *bl, int tid, enum send_target target, int type, t_tick tick, int val1, int val2, int val3);
void clif_efst_status_change_sub(struct block_list *tbl, struct block_list *bl, enum send_target target);

void clif_wis_message(map_session_data* sd, const char* nick, const char* mes, size_t mes_len, int gmlvl);
void clif_wis_end( map_session_data& sd, e_ack_whisper result );

void clif_solved_charname(int fd, int charid, const char* name);
void clif_name( struct block_list* src, struct block_list *bl, send_target target );
#define clif_name_self(bl) clif_name( (bl), (bl), SELF )
#define clif_name_area(bl) clif_name( (bl), (bl), AREA )

void clif_use_card(map_session_data *sd,int idx);
void clif_insert_card( map_session_data& sd, int32 idx_equip, int32 idx_card, bool failure );

void clif_inventorylist(map_session_data *sd);
void clif_equiplist(map_session_data *sd);

void clif_cart_additem(map_session_data *sd,int n,int amount);
void clif_cart_additem_ack( map_session_data& sd, e_ack_additem_to_cart flag );
void clif_cart_delitem( map_session_data& sd, int32 index, int32 amount );
void clif_cartlist(map_session_data *sd);
void clif_clearcart(int fd);

void clif_item_identify_list(map_session_data *sd);
void clif_item_identified( map_session_data& sd, int32 idx, bool failure );
void clif_item_repair_list( map_session_data& sd, map_session_data& dstsd, uint16 lv );
void clif_item_repaireffect( map_session_data& sd, int32 idx, bool failure );
void clif_item_damaged( map_session_data& sd, uint16 position );
void clif_item_refine_list( map_session_data& sd );
void clif_hat_effects( map_session_data& sd, block_list& bl, enum send_target target );
void clif_hat_effect_single( map_session_data& sd, uint16 effectId, bool enable );

void clif_item_skill(map_session_data *sd,uint16 skill_id,uint16 skill_lv);

void clif_mvp_effect(map_session_data *sd);
void clif_mvp_item(map_session_data *sd, t_itemid nameid);
void clif_mvp_exp(map_session_data *sd, t_exp exp);
void clif_mvp_noitem(map_session_data* sd);
void clif_changed_dir(struct block_list *bl, enum send_target target);

// vending
void clif_openvendingreq( map_session_data& sd, uint16 num );
void clif_showvendingboard( map_session_data& sd, enum send_target target = AREA_WOS, struct block_list* tbl = nullptr );
void clif_closevendingboard(struct block_list* bl, int fd);
void clif_vendinglist( map_session_data& sd, map_session_data& vsd );
void clif_buyvending( map_session_data& sd, uint16 index, uint16 amount, e_pc_purchase_result_frommc result );
void clif_openvending( map_session_data& sd );
void clif_openvending_ack(map_session_data& sd, e_ack_openstore2 result);
void clif_vendingreport( map_session_data& sd, uint16 index, uint16 amount, uint32 char_id, int32 zeny );

void clif_movetoattack( map_session_data& sd, block_list& bl );

// party
void clif_party_created( map_session_data& sd, int result );
void clif_party_member_info( struct party_data& party, map_session_data& sd );
void clif_party_info( struct party_data& party, map_session_data *sd = nullptr );
void clif_party_invite( map_session_data& sd, map_session_data& tsd );
void clif_party_invite_reply( map_session_data& sd, const char* nick, enum e_party_invite_reply reply );
void clif_party_option(struct party_data *p,map_session_data *sd,int flag);
void clif_party_withdraw( map_session_data& sd, uint32 account_id, const char* name, enum e_party_member_withdraw result, enum send_target target );
void clif_party_message( struct party_data& party, uint32 account_id, const char* mes, size_t len );
void clif_party_xy( map_session_data& sd );
void clif_party_xy_single( map_session_data& sd, map_session_data& tsd );
void clif_party_hp( map_session_data& sd );
void clif_hpmeter_single( map_session_data& sd, uint32 id, uint32 hp, uint32 maxhp );
void clif_party_job_and_level( map_session_data& sd );
void clif_party_dead( map_session_data& sd );

// guild
void clif_guild_created( map_session_data& sd, int flag );
void clif_guild_belonginfo( map_session_data& sd );
void clif_guild_masterormember(map_session_data *sd);
void clif_guild_basicinfo( map_session_data& sd );
void clif_guild_allianceinfo(map_session_data *sd);
void clif_guild_memberlist( map_session_data& sd );
void clif_guild_skillinfo( map_session_data& sd );
void clif_guild_send_onlineinfo(map_session_data *sd); //[LuzZza]
void clif_guild_memberlogin_notice(const struct mmo_guild &g,int idx,int flag);
void clif_guild_invite( map_session_data& sd, const struct mmo_guild& g );
void clif_guild_inviteack( map_session_data& sd, int flag );
void clif_guild_leave( map_session_data& sd, const char* name, uint32 char_id, const char* mes );
void clif_guild_expulsion( map_session_data& sd, const char* name, uint32 char_id, const char* mes );
void clif_guild_positionchanged(const struct mmo_guild &g,int idx);
void clif_guild_memberpositionchanged(const struct mmo_guild &g,int idx);
void clif_guild_emblem(const map_session_data &sd, const struct mmo_guild &g);
void clif_guild_emblem_area(struct block_list* bl);
void clif_guild_notice( map_session_data& sd );
void clif_guild_message( const struct mmo_guild& g, uint32 account_id, const char* mes, size_t len );
void clif_guild_reqalliance(map_session_data *sd,uint32 account_id,const char *name);
void clif_guild_allianceack(map_session_data *sd,int flag);
void clif_guild_delalliance(map_session_data *sd,int guild_id,int flag);
void clif_guild_oppositionack(map_session_data *sd,int flag);
void clif_guild_broken( map_session_data& sd, int flag );
void clif_guild_xy( map_session_data& sd );
void clif_guild_xy_single( map_session_data& sd, map_session_data& tsd );
void clif_guild_xy_remove( map_session_data& sd );
void clif_guild_castle_list(map_session_data& sd);
void clif_guild_castle_teleport_res(map_session_data& sd, enum e_siege_teleport_result result);

// Battleground
void clif_bg_hp(map_session_data *sd);
void clif_bg_xy(map_session_data *sd);
void clif_bg_xy_remove(map_session_data *sd);
void clif_bg_message(struct s_battleground_data *bg, int src_id, const char *name, const char *mes, size_t len);
void clif_bg_updatescore(int16 m);
void clif_bg_updatescore_single(map_session_data *sd);
void clif_sendbgemblem_area(map_session_data *sd);
void clif_sendbgemblem_single(int fd, map_session_data *sd);

// Battleground Queue
void clif_bg_queue_apply_result(e_bg_queue_apply_ack result, const char *name, map_session_data *sd);
void clif_bg_queue_apply_notify(const char *name, map_session_data *sd);
void clif_bg_queue_entry_init(map_session_data *sd);
void clif_bg_queue_lobby_notify(const char *name, map_session_data *sd);
void clif_bg_queue_ack_lobby(bool result, const char *name, const char *lobbyname, map_session_data *sd);

// Instancing
void clif_instance_create( int instance_id, size_t num );
void clif_instance_changewait(int instance_id, int num);
void clif_instance_status(int instance_id, unsigned int limit1, unsigned int limit2);
void clif_instance_changestatus(int instance_id, e_instance_notify type, unsigned int limit);
void clif_parse_MemorialDungeonCommand(int fd, map_session_data *sd);
void clif_instance_info( map_session_data& sd );

// Custom Fonts
void clif_font(map_session_data *sd);

// atcommand
void clif_displaymessage(const int fd, const char* mes);
void clif_disp_message(struct block_list* src, const char* mes, size_t len, enum send_target target);
void clif_broadcast(struct block_list* bl, const char* mes, size_t len, int type, enum send_target target);
void clif_broadcast2(struct block_list* bl, const char* mes, size_t len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target);
void clif_heal( map_session_data& sd, int32 type, uint32 val );
void clif_resurrection( block_list& bl );
void clif_map_property(struct block_list *bl, enum map_property property, enum send_target t);
void clif_pvpset(map_session_data *sd, int pvprank, int pvpnum,int type);
void clif_map_property_mapall(int map, enum map_property property);
void clif_refine( map_session_data& sd, uint16 index, e_ack_itemrefining result );
void clif_upgrademessage( map_session_data* sd, int result, t_itemid item_id );

//petsystem
void clif_catch_process( map_session_data& sd );
void clif_pet_roulette( map_session_data& sd, bool success );
void clif_sendegg(map_session_data *sd);
void clif_send_petstatus(map_session_data *sd);
void clif_send_petdata(map_session_data* sd, struct pet_data* pd, int type, int param);
#define clif_pet_equip(sd, pd) clif_send_petdata(sd, pd, 3, (pd)->vd.head_bottom)
#define clif_pet_equip_area(pd) clif_send_petdata(nullptr, pd, 3, (pd)->vd.head_bottom)
#define clif_pet_performance(pd, param) clif_send_petdata(nullptr, pd, 4, param)
void clif_pet_emotion(struct pet_data *pd,int param);
void clif_pet_food( map_session_data& sd, int32 foodid, bool success );
void clif_pet_autofeed_status(map_session_data* sd, bool force);

//friends list
int clif_friendslist_toggle_sub(map_session_data *sd,va_list ap);
void clif_friendslist_send( map_session_data& sd );
void clif_friendslist_reqack(map_session_data *sd, map_session_data *f_sd, int type);

void clif_weather(int16 m); // [Valaris]
void clif_specialeffect(struct block_list* bl, int type, enum send_target target); // special effects [Valaris]
void clif_specialeffect_single(struct block_list* bl, int type, int fd);
void clif_specialeffect_remove(struct block_list* bl_src, int effect, enum send_target e_target, struct block_list* bl_target);
void clif_messagecolor_target(struct block_list *bl, unsigned long color, const char *msg, bool rgb2bgr, enum send_target type, map_session_data *sd);
#define clif_messagecolor(bl, color, msg, rgb2bgr, type) clif_messagecolor_target(bl, color, msg, rgb2bgr, type, nullptr) // Mob/Npc color talk [SnakeDrak]
void clif_specialeffect_value(struct block_list* bl, int effect_id, int num, send_target target);

void clif_GM_kickack(map_session_data *sd, int id);
void clif_GM_kick(map_session_data *sd,map_session_data *tsd);
void clif_manner_message(map_session_data* sd, uint32 type);
void clif_GM_silence(map_session_data* sd, map_session_data* tsd, uint8 type);

void clif_disp_overhead_(struct block_list *bl, const char* mes, enum send_target flag);
#define clif_disp_overhead(bl, mes) clif_disp_overhead_(bl, mes, AREA)

void clif_get_weapon_view(map_session_data* sd, t_itemid *rhand, t_itemid *lhand);

void clif_party_xy_remove(map_session_data *sd); //Fix for minimap [Kevin]
void clif_gospel_info(map_session_data *sd, int type);
void clif_feel_req(int fd, map_session_data *sd, uint16 skill_lv);
void clif_starskill(map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result);
void clif_feel_info(map_session_data* sd, unsigned char feel_level, unsigned char type);
void clif_hate_info(map_session_data *sd, unsigned char hate_level,int class_, unsigned char type);
void clif_mission_info(map_session_data *sd, int mob_id, unsigned char progress);
void clif_feel_hate_reset(map_session_data *sd);

// [blackhole89]
void clif_hominfo(map_session_data *sd, struct homun_data *hd, int flag);
void clif_homskillinfoblock( homun_data& hd );
void clif_homskillup( homun_data& hd, uint16 skill_id );
void clif_hom_food( map_session_data& sd, int32 foodid, bool success );
void clif_send_homdata( homun_data& hd, e_hom_state2 state );

void clif_configuration( map_session_data* sd, enum e_config_type type, bool enabled );
void clif_viewequip_ack( map_session_data& sd, map_session_data& tsd );
void clif_equipcheckbox(map_session_data* sd);

void clif_msg(map_session_data* sd, unsigned short id);
void clif_msg_value(map_session_data* sd, unsigned short id, int value);
void clif_msg_skill(map_session_data* sd, uint16 skill_id, int msg_id);
void clif_msg_color( map_session_data* sd, uint16 msg_id, uint32 color );

//quest system [Kevin] [Inkfish]
void clif_quest_send_list(map_session_data * sd);
void clif_quest_send_mission(map_session_data * sd);
void clif_quest_add(map_session_data * sd, struct quest * qd);
void clif_quest_delete(map_session_data * sd, int quest_id);
void clif_quest_update_status(map_session_data * sd, int quest_id, bool active);
void clif_quest_update_objective(map_session_data * sd, struct quest * qd);
void clif_quest_show_event(map_session_data *sd, struct block_list *bl, e_questinfo_types effect, e_questinfo_markcolor color);
void clif_displayexp(map_session_data *sd, t_exp exp, char type, bool quest, bool lost);

int clif_send(const void* buf, int len, struct block_list* bl, enum send_target type);
void do_init_clif(void);
void do_final_clif(void);

// MAIL SYSTEM
enum mail_send_result : uint8_t {
	WRITE_MAIL_SUCCESS = 0x0,
	WRITE_MAIL_FAILED = 0x1,
	WRITE_MAIL_FAILED_CNT = 0x2,
	WRITE_MAIL_FAILED_ITEM = 0x3,
	WRITE_MAIL_FAILED_CHECK_CHARACTER_NAME = 0x4,
	WRITE_MAIL_FAILED_WHISPEREXREGISTER = 0x5,
};

void clif_Mail_window(int fd, int flag);
void clif_Mail_read(map_session_data *sd, int mail_id);
void clif_mail_delete(map_session_data* sd, struct mail_message *msg, bool success);
void clif_Mail_return(int fd, int mail_id, short fail);
void clif_Mail_send(map_session_data* sd, enum mail_send_result result);
void clif_Mail_new(map_session_data* sd, int mail_id, const char *sender, const char *title);
void clif_Mail_refreshinbox(map_session_data *sd,enum mail_inbox_type type,int64 mailID);
void clif_mail_getattachment(map_session_data* sd, struct mail_message *msg, uint8 result, enum mail_attachment_type type);
void clif_Mail_Receiver_Ack(map_session_data* sd, uint32 char_id, short class_, uint32 level, const char* name);
void clif_mail_removeitem(map_session_data* sd, bool success, int index, int amount);
// AUCTION SYSTEM
void clif_Auction_openwindow(map_session_data *sd);
void clif_Auction_results(map_session_data *sd, short count, short pages, uint8 *buf);
void clif_Auction_message(int fd, unsigned char flag);
void clif_Auction_close(int fd, unsigned char flag);
void clif_parse_Auction_cancelreg(int fd, map_session_data *sd);

void clif_bossmapinfo( map_session_data& sd, mob_data* md, e_bossmap_info flag );
void clif_cashshop_show( map_session_data& sd, npc_data& nd );

// ADOPTION
void clif_Adopt_reply(map_session_data *sd, int type);
void clif_Adopt_request(map_session_data *sd, map_session_data *src, int p_id);

// MERCENARIES
void clif_mercenary_info(map_session_data *sd);
void clif_mercenary_skillblock(map_session_data *sd);
void clif_mercenary_updatestatus(map_session_data *sd, int type);

// RENTAL SYSTEM
void clif_rental_time( map_session_data* sd, t_itemid nameid, int seconds );
void clif_rental_expired( map_session_data* sd, int index, t_itemid nameid );

// BOOK READING
void clif_readbook(int fd, int book_id, int page);

// Show Picker
void clif_party_show_picker(map_session_data * sd, struct item * item_data);

// Progress Bar [Inkfish]
void clif_progressbar(map_session_data * sd, unsigned long color, unsigned int second);
void clif_progressbar_abort(map_session_data * sd);
void clif_progressbar_npc(struct npc_data *nd, map_session_data* sd);
#define clif_progressbar_npc_area(nd) clif_progressbar_npc((nd),nullptr)

void clif_PartyBookingRegisterAck(map_session_data *sd, int flag);
void clif_PartyBookingDeleteAck(map_session_data* sd, int flag);
void clif_PartyBookingSearchAck(int fd, struct party_booking_ad_info** results, int count, bool more_result);
void clif_PartyBookingUpdateNotify(map_session_data* sd, struct party_booking_ad_info* pb_ad);
void clif_PartyBookingDeleteNotify(map_session_data* sd, int index);
void clif_PartyBookingInsertNotify(map_session_data* sd, struct party_booking_ad_info* pb_ad);

/* Bank System [Yommy/Hercules] */
void clif_parse_BankDeposit (int fd, map_session_data *sd);
void clif_parse_BankWithdraw (int fd, map_session_data *sd);
void clif_parse_BankCheck (int fd, map_session_data *sd);
void clif_parse_BankOpen (int fd, map_session_data *sd);
void clif_parse_BankClose (int fd, map_session_data *sd);

void clif_showdigit(map_session_data* sd, unsigned char type, int value);

/// Buying Store System
void clif_buyingstore_open(map_session_data* sd);
void clif_buyingstore_open_failed(map_session_data* sd, unsigned short result, unsigned int weight);
void clif_buyingstore_myitemlist( map_session_data& sd );
void clif_buyingstore_entry( map_session_data& sd, struct block_list* tbl = nullptr );
void clif_buyingstore_disappear_entry( map_session_data& sd, struct block_list* tbl = nullptr );
void clif_buyingstore_itemlist( map_session_data& sd, map_session_data& pl_sd );
void clif_buyingstore_trade_failed_buyer(map_session_data* sd, short result);
void clif_buyingstore_update_item(map_session_data* sd, t_itemid nameid, unsigned short amount, uint32 char_id, int zeny);
void clif_buyingstore_delete_item(map_session_data* sd, short index, unsigned short amount, int price);
void clif_buyingstore_trade_failed_seller(map_session_data* sd, short result, t_itemid nameid);

/// Search Store System
void clif_search_store_info_ack( map_session_data& sd );
void clif_search_store_info_failed(map_session_data& sd, e_searchstore_failure reason);
void clif_open_search_store_info(map_session_data& sd);
void clif_search_store_info_click_ack(map_session_data& sd, int16 x, int16 y);

/// Cash Shop
void clif_cashshop_result( map_session_data* sd, t_itemid item_id, uint16 result );
void clif_cashshop_open( map_session_data* sd, int tab );

void clif_display_pinfo( map_session_data& sd );

/// Roulette
void clif_roulette_open(map_session_data* sd);
void clif_parse_roulette_open(int fd, map_session_data *sd);
void clif_parse_roulette_info(int fd, map_session_data *sd);
void clif_parse_roulette_close(int fd, map_session_data *sd);
void clif_parse_roulette_generate(int fd, map_session_data *sd);
void clif_parse_roulette_item(int fd, map_session_data *sd);

void clif_elementalconverter_list( map_session_data& sd );

void clif_millenniumshield( block_list& bl, int16 shields );

void clif_magicdecoy_list( map_session_data& sd, uint16 skill_lv, short x, short y );

void clif_poison_list( map_session_data& sd, uint16 skill_lv );

void clif_autoshadowspell_list( map_session_data& sd );

int clif_skill_itemlistwindow( map_session_data *sd, uint16 skill_id, uint16 skill_lv );
void clif_elemental_info(map_session_data *sd);
void clif_elemental_updatestatus(map_session_data& sd, _sp type);

void clif_spiritcharm( map_session_data& sd );

void clif_snap( struct block_list *bl, short x, short y );
void clif_monster_hp_bar( struct mob_data* md, int fd );

// Clan System
void clif_clan_basicinfo( map_session_data& sd );
void clif_clan_message( struct clan &clan, const char *mes, size_t len );
void clif_clan_onlinecount( struct clan& clan );
void clif_clan_leave( map_session_data& sd );

// Bargain Tool
void clif_sale_start(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void clif_sale_end(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void clif_sale_amount(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void clif_sale_open(map_session_data* sd);

// Refine UI
void clif_refineui_open( map_session_data* sd );

/**
 * Color Table
 **/
enum clif_colors {
	COLOR_DEFAULT,
	COLOR_RED,
	COLOR_WHITE,
	COLOR_YELLOW,
	COLOR_CYAN,
	COLOR_LIGHT_GREEN,
	COLOR_LIGHT_YELLOW,
	COLOR_MAX
};
extern unsigned long color_table[COLOR_MAX];

void clif_channel_msg(struct Channel *channel, const char *msg, unsigned long color);

#define clif_menuskill_clear(sd) (sd)->menuskill_id = (sd)->menuskill_val = (sd)->menuskill_val2 = 0;

void clif_ranklist(map_session_data *sd, int16 rankingType);
void clif_update_rankingpoint(map_session_data &sd, int rankingtype, int point);

void clif_crimson_marker( map_session_data& sd, struct block_list& bl, bool remove );

void clif_showscript(struct block_list* bl, const char* message, enum send_target flag);
void clif_party_leaderchanged(map_session_data *sd, int prev_leader_aid, int new_leader_aid);

void clif_account_name(int fd, uint32 account_id, const char* accname);
void clif_notify_bindOnEquip( map_session_data& sd, int16 index );

void clif_merge_item_open( map_session_data& sd );

void clif_broadcast_obtain_special_item(const char *char_name, t_itemid nameid, t_itemid container, enum BROADCASTING_SPECIAL_ITEM_OBTAIN type);

void clif_dressing_room(map_session_data *sd, int flag);
void clif_navigateTo(map_session_data *sd, const char* mapname, uint16 x, uint16 y, uint8 flag, bool hideWindow, uint16 mob_id );
void clif_SelectCart(map_session_data *sd);

/// Achievement System
void clif_achievement_list_all(map_session_data *sd);
void clif_achievement_update(map_session_data *sd, struct achievement *ach, int count);
void clif_achievement_reward_ack(int fd, unsigned char result, int ach_id);

/// Attendance System
enum in_ui_type : int8 {
	IN_UI_MACRO_REGISTER = 2,
	IN_UI_MACRO_DETECTOR,
	IN_UI_ATTENDANCE = 5
};

enum out_ui_type : int8 {
	OUT_UI_BANK = 0,
	OUT_UI_STYLIST,
	OUT_UI_CAPTCHA,
	OUT_UI_MACRO,
	OUT_UI_TIP = 5,
	OUT_UI_QUEST,
	OUT_UI_ATTENDANCE,
	OUT_UI_ENCHANTGRADE,
	OUT_UI_ENCHANT = 10,
};

void clif_ui_open( map_session_data& sd, enum out_ui_type ui_type, int32 data );
void clif_attendence_response( map_session_data *sd, int32 data );

void clif_weight_limit( map_session_data* sd );

void clif_guild_storage_log( map_session_data& sd, std::vector<struct guild_log_entry>& log, enum e_guild_storage_log result );

void clif_camerainfo( map_session_data* sd, bool show, float range = 0.0f, float rotation = 0.0f, float latitude = 0.0f );

/// Equip Switch System
void clif_equipswitch_list( map_session_data* sd );
void clif_equipswitch_add( map_session_data* sd,uint16 index, uint32 pos, uint8 flag );
void clif_equipswitch_remove( map_session_data* sd, uint16 index, uint32 pos, bool failed );
void clif_equipswitch_reply( map_session_data* sd, bool failed );

/// Pet evolution
void clif_pet_evolution_result( map_session_data* sd, e_pet_evolution_result result );

void clif_parse_skill_toid( map_session_data* sd, uint16 skill_id, uint16 skill_lv, int target_id );

void clif_inventory_expansion_info( map_session_data* sd );

// Barter System
void clif_barter_open( map_session_data& sd, struct npc_data& nd );
void clif_barter_extended_open( map_session_data& sd, struct npc_data& nd );

void clif_summon_init(struct mob_data& md);
void clif_summon_hp_bar(struct mob_data& md);

// Laphine System
void clif_laphine_synthesis_open( map_session_data *sd, std::shared_ptr<s_laphine_synthesis> synthesis );
void clif_laphine_upgrade_open( map_session_data* sd, std::shared_ptr<s_laphine_upgrade> upgrade );

// Reputation System
void clif_reputation_type( map_session_data& sd, int64 type, int64 points );
void clif_reputation_list( map_session_data& sd );

// Item Reform UI
void clif_item_reform_open( map_session_data& sd, t_itemid item );

// Item Enchant UI
void clif_enchantwindow_open( map_session_data& sd, uint64 clientLuaIndex );

// Enchanting Shadow / Shadow Scar Spirit
void clif_enchantingshadow_spirit(unit_data &ud);

void clif_broadcast_refine_result(map_session_data& sd, t_itemid itemId, int8 level, bool success);

// Captcha Register
void clif_captcha_upload_request(map_session_data &sd);
void clif_captcha_upload_end(map_session_data &sd);

// Captcha Preview
void clif_captcha_preview_response(map_session_data &sd, std::shared_ptr<s_captcha_data> cd);

// Macro Detector
void clif_macro_detector_request(map_session_data &sd);
void clif_macro_detector_request_show(map_session_data &sd);
void clif_macro_detector_status(map_session_data &sd, e_macro_detect_status stype);

// Macro Reporter
void clif_macro_reporter_select(map_session_data &sd, const std::vector<uint32> &aid_list);
void clif_macro_reporter_status(map_session_data &sd, e_macro_report_status stype);

void clif_dynamicnpc_result( map_session_data& sd, e_dynamicnpc_result result );

void clif_set_dialog_align(map_session_data& sd, int npcid, e_say_dialog_align align);
void clif_set_npc_window_size(map_session_data& sd, int width, int height);
void clif_set_npc_window_pos(map_session_data& sd, int x, int y);
void clif_set_npc_window_pos_percent(map_session_data& sd, int x, int y);

void clif_noask_sub( map_session_data& sd, map_session_data& tsd, int type );

void clif_specialpopup(map_session_data& sd, int32 id);

#endif /* CLIF_HPP */
