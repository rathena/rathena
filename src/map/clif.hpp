// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLIF_HPP
#define CLIF_HPP

#include <vector>

#include <stdarg.h>

#include "../common/cbasetypes.hpp"
#include "../common/db.hpp" //dbmap
#include "../common/mmo.hpp"
#include "../common/timer.hpp" // t_tick

#include "packets.hpp"
#include "script.hpp"

struct Channel;
struct clan;
struct item;
struct s_storage;
//#include "map.hpp"
struct block_list;
struct unit_data;
struct map_session_data;
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
struct guild;
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

enum e_PacketDBVersion { // packet DB
	MIN_PACKET_DB  = 0x064,
	MAX_PACKET_DB  = 0xBFF,
#if !defined(MAX_PACKET_POS)
	MAX_PACKET_POS = 20,
#endif
};

enum e_packet_ack : uint8_t{
	ZC_ACK_OPEN_BANKING = 0,
	ZC_ACK_CLOSE_BANKING,
	ZC_ACK_BANKING_DEPOSIT,
	ZC_ACK_BANKING_WITHDRAW,
	ZC_BANKING_CHECK,
	ZC_PERSONAL_INFOMATION,
	ZC_PERSONAL_INFOMATION_CHN,
	ZC_CLEAR_DIALOG,
	ZC_C_MARKERINFO,
	ZC_NOTIFY_BIND_ON_EQUIP,
	ZC_WEAR_EQUIP_ACK,
	ZC_MERGE_ITEM_OPEN,
	ZC_ACK_MERGE_ITEM,
	ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN,
	//add other here
	MAX_ACK_FUNC //auto upd len
};

struct s_packet_db {
	short len;
	void (*func)(int, struct map_session_data *);
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
	PARTY_REPLY_JOINMSG_REFUSE,		    ///< result=5 : !TODO "The character blocked the party invitation." -> MsgStringTable[1324] (since 20070904)
	PARTY_REPLY_UNKNOWN_ERROR,		    ///< result=6 : ??
	PARTY_REPLY_OFFLINE,			    ///< result=7 : "The Character is not currently online or does not exist." -> MsgStringTable[71] (since 20070904)
	PARTY_REPLY_INVALID_MAPPROPERTY,    ///< result=8 : !TODO "Unable to organize a party in this map" -> MsgStringTable[1388] (since 20080527)
	PARTY_REPLY_INVALID_MAPPROPERTY_ME, ///< return=9 : !TODO "Cannot join a party in this map" -> MsgStringTable[1871] (since 20110205)
};

/// Enum for Convex Mirror (SC_BOSSMAPINFO)
enum e_bossmap_info {
	BOSS_INFO_NOT = 0,
	BOSS_INFO_ALIVE,
	BOSS_INFO_ALIVE_WITHMSG,
	BOSS_INFO_DEAD,
};

#define packet_len(cmd) packet_db[cmd].len
extern struct s_packet_db packet_db[MAX_PACKET_DB+1];
extern int packet_db_ack[MAX_ACK_FUNC + 1];

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

enum map_type : uint8_t 
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

	USESKILL_FAIL_MAX
};

enum clif_messages : uint16_t {
	/* Constant values */
	// clif_cart_additem_ack flags
	ADDITEM_TO_CART_FAIL_WEIGHT = 0x0,
	ADDITEM_TO_CART_FAIL_COUNT = 0x1,

	// clif_equipitemack flags
	ITEM_EQUIP_ACK_OK = 0,
	ITEM_EQUIP_ACK_FAIL = 1,
	ITEM_EQUIP_ACK_FAILLEVEL = 2,
	/* -end- */

	//! NOTE: These values below need client version validation
	ITEM_CANT_OBTAIN_WEIGHT = 0x34, /* You cannot carry more items because you are overweight. */
	ITEM_NOUSE_SITTING = 0x297,
	ITEM_PARTY_MEMBER_NOT_SUMMONED = 0x4c5, ///< "The party member was not summoned because you are not the party leader."
	ITEM_PARTY_NO_MEMBER_IN_MAP = 0x4c6, ///< "There is no party member to summon in the current map."
	MERC_MSG_BASE = 0x4f2,
	SKILL_CANT_USE_AREA = 0x536,
	ITEM_CANT_USE_AREA = 0x537,
	VIEW_EQUIP_FAIL = 0x54d,
	ITEM_NEED_MADOGEAR = 0x59b,
	ITEM_NEED_CART = 0x5ef,
	RUNE_CANT_CREATE = 0x61b,
	ITEM_CANT_COMBINE = 0x623,
	INVENTORY_SPACE_FULL = 0x625,
	ITEM_PRODUCE_SUCCESS = 0x627,
	ITEM_PRODUCE_FAIL = 0x628,
	ITEM_UNIDENTIFIED = 0x62d,
	ITEM_NEED_BOW = 0x69b,
	ITEM_REUSE_LIMIT = 0x746,
	WORK_IN_PROGRESS = 0x783,
	NEED_REINS_OF_MOUNT = 0x78c,
	PARTY_MASTER_CHANGE_SAME_MAP = 0x82e, ///< "It is only possible to change the party leader while on the same map."
	MERGE_ITEM_NOT_AVAILABLE = 0x887,
	ITEM_BULLET_EQUIP_FAIL = 0x9bd,
	SKILL_NEED_GATLING = 0x9fa,
	SKILL_NEED_SHOTGUN = 0x9fb,
	SKILL_NEED_RIFLE = 0x9fc,
	SKILL_NEED_REVOLVER = 0x9fd,
	SKILL_NEED_HOLY_BULLET = 0x9fe,
	SKILL_NEED_GRENADE = 0xa01,
	GUILD_MASTER_WOE = 0xb93, /// <"Currently in WoE hours, unable to delegate Guild leader"
	GUILD_MASTER_DELAY = 0xb94, /// <"You have to wait for one day before delegating a new Guild leader"
	MSG_ATTENDANCE_DISABLED = 0xd92,

	// Unofficial names
	C_ITEM_EQUIP_SWITCH = 0xbc7, 
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
	// Unknown
	CONFIG_PET_AUTOFEED = 2,
	CONFIG_HOMUNCULUS_AUTOFEED
};

int clif_setip(const char* ip);
void clif_setbindip(const char* ip);
void clif_setport(uint16 port);

uint32 clif_getip(void);
uint32 clif_refresh_ip(void);
uint16 clif_getport(void);

void clif_authok(struct map_session_data *sd);
void clif_authrefuse(int fd, uint8 error_code);
void clif_authfail_fd(int fd, int type);
void clif_charselectok(int id, uint8 ok);
void clif_dropflooritem(struct flooritem_data* fitem, bool canShowEffect);
void clif_clearflooritem(struct flooritem_data *fitem, int fd);

void clif_clearunit_single(int id, clr_type type, int fd);
void clif_clearunit_area(struct block_list* bl, clr_type type);
void clif_clearunit_delayed(struct block_list* bl, clr_type type, t_tick tick);
int clif_spawn(struct block_list *bl, bool walking = false);	//area
void clif_walkok(struct map_session_data *sd);	// self
void clif_move(struct unit_data *ud); //area
void clif_changemap(struct map_session_data *sd, short m, int x, int y);	//self
void clif_changemapserver(struct map_session_data* sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port);	//self
void clif_blown(struct block_list *bl); // area
void clif_slide(struct block_list *bl, int x, int y); // area
void clif_fixpos(struct block_list *bl);	// area
void clif_npcbuysell(struct map_session_data* sd, int id);	//self
void clif_buylist(struct map_session_data *sd, struct npc_data *nd);	//self
void clif_selllist(struct map_session_data *sd);	//self
void clif_npc_market_open(struct map_session_data *sd, struct npc_data *nd);
void clif_parse_NPCMarketClosed(int fd, struct map_session_data *sd);
void clif_parse_NPCMarketPurchase(int fd, struct map_session_data *sd);
void clif_scriptmes(struct map_session_data *sd, int npcid, const char *mes);	//self
void clif_scriptnext(struct map_session_data *sd,int npcid);	//self
void clif_scriptclose(struct map_session_data *sd, int npcid);	//self
void clif_scriptclear(struct map_session_data *sd, int npcid);	//self
void clif_scriptmenu(struct map_session_data* sd, int npcid, const char* mes);	//self
void clif_scriptinput(struct map_session_data *sd, int npcid);	//self
void clif_scriptinputstr(struct map_session_data *sd, int npcid);	// self
void clif_cutin(struct map_session_data* sd, const char* image, int type);	//self
void clif_viewpoint(struct map_session_data *sd, int npc_id, int type, int x, int y, int id, int color);	//self
void clif_additem(struct map_session_data *sd, int n, int amount, unsigned char fail); // self
void clif_dropitem(struct map_session_data *sd,int n,int amount);	//self
void clif_delitem(struct map_session_data *sd,int n,int amount, short reason); //self
void clif_updatestatus(struct map_session_data *sd,int type);	//self
void clif_changestatus(struct map_session_data* sd,int type,int val);	//area
int clif_damage(struct block_list* src, struct block_list* dst, t_tick tick, int sdelay, int ddelay, int64 sdamage, int div, enum e_damage_type type, int64 sdamage2, bool spdamage);	// area
void clif_takeitem(struct block_list* src, struct block_list* dst);
void clif_sitting(struct block_list* bl);
void clif_standing(struct block_list* bl);
void clif_sprite_change(struct block_list *bl, int id, int type, int val, int val2, enum send_target target);
void clif_changelook(struct block_list *bl,int type,int val);	// area
void clif_changetraplook(struct block_list *bl,int val); // area
void clif_refreshlook(struct block_list *bl,int id,int type,int val,enum send_target target); //area specified in 'target'
void clif_arrowequip(struct map_session_data *sd,int val); //self
void clif_arrow_fail(struct map_session_data *sd,int type); //self
void clif_arrow_create_list(struct map_session_data *sd);	//self
void clif_statusupack(struct map_session_data *sd,int type,int ok,int val);	// self
void clif_equipitemack(struct map_session_data *sd,int n,int pos,uint8 flag);	// self
void clif_unequipitemack(struct map_session_data *sd,int n,int pos,int ok);	// self
void clif_misceffect(struct block_list* bl,int type);	// area
void clif_changeoption_target(struct block_list* bl, struct block_list* target);
#define clif_changeoption(bl) clif_changeoption_target(bl, NULL)	// area
void clif_changeoption2(struct block_list* bl);	// area
void clif_useitemack(struct map_session_data *sd,int index,int amount,bool ok);	// self
void clif_GlobalMessage(struct block_list* bl, const char* message,enum send_target target);
void clif_createchat(struct map_session_data* sd, int flag);	// self
void clif_dispchat(struct chat_data* cd, int fd);	// area or fd
void clif_joinchatfail(struct map_session_data *sd,int flag);	// self
void clif_joinchatok(struct map_session_data *sd,struct chat_data* cd);	// self
void clif_addchat(struct chat_data* cd,struct map_session_data *sd);	// chat
void clif_changechatowner(struct chat_data* cd, struct map_session_data* sd);	// chat
void clif_clearchat(struct chat_data *cd,int fd);	// area or fd
void clif_leavechat(struct chat_data* cd, struct map_session_data* sd, bool flag);	// chat
void clif_changechatstatus(struct chat_data* cd);	// chat
void clif_refresh_storagewindow(struct map_session_data *sd);
void clif_refresh(struct map_session_data *sd);	// self

void clif_emotion(struct block_list *bl,int type);
void clif_talkiebox(struct block_list* bl, const char* talkie);
void clif_wedding_effect(struct block_list *bl);
void clif_divorced(struct map_session_data* sd, const char* name);
void clif_callpartner(struct map_session_data *sd);
void clif_playBGM(struct map_session_data* sd, const char* name);
void clif_soundeffect(struct map_session_data* sd, struct block_list* bl, const char* name, int type);
void clif_soundeffectall(struct block_list* bl, const char* name, int type, enum send_target coverage);
void clif_parse_ActionRequest_sub(struct map_session_data *sd, int action_type, int target_id, t_tick tick);
void clif_parse_LoadEndAck(int fd,struct map_session_data *sd);
void clif_hotkeys_send(struct map_session_data *sd, int tab);

// trade
void clif_traderequest(struct map_session_data* sd, const char* name);
void clif_tradestart(struct map_session_data* sd, uint8 type);
void clif_tradeadditem(struct map_session_data* sd, struct map_session_data* tsd, int index, int amount);
void clif_tradeitemok(struct map_session_data* sd, int index, int fail);
void clif_tradedeal_lock(struct map_session_data* sd, int fail);
void clif_tradecancelled(struct map_session_data* sd);
void clif_tradecompleted(struct map_session_data* sd, int fail);
void clif_tradeundo(struct map_session_data* sd);

// storage
void clif_storagelist(struct map_session_data* sd, struct item* items, int items_length, const char *storename);
void clif_updatestorageamount(struct map_session_data* sd, int amount, int max_amount);
void clif_storageitemadded(struct map_session_data* sd, struct item* i, int index, int amount);
void clif_storageitemremoved(struct map_session_data* sd, int index, int amount);
void clif_storageclose(struct map_session_data* sd);

int clif_insight(struct block_list *bl,va_list ap);	// map_forallinmovearea callback
int clif_outsight(struct block_list *bl,va_list ap);	// map_forallinmovearea callback

void clif_class_change_target(struct block_list *bl,int class_, int type, enum send_target target, struct map_session_data *sd);
#define clif_class_change(bl, class_, type) clif_class_change_target(bl, class_, type, AREA, NULL)
#define clif_mob_class_change(md, class_) clif_class_change(&md->bl, class_, 1)

void clif_skillinfoblock(struct map_session_data *sd);
void clif_skillup(struct map_session_data *sd, uint16 skill_id, int lv, int range, int upgradable);
void clif_skillinfo(struct map_session_data *sd,int skill_id, int inf);
void clif_addskill(struct map_session_data *sd, int skill_id);
void clif_deleteskill(struct map_session_data *sd, int skill_id);

void clif_skillcasting(struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, uint16 skill_lv, int property, int casttime);
void clif_skillcastcancel(struct block_list* bl);
void clif_skill_fail(struct map_session_data *sd,uint16 skill_id,enum useskill_fail_cause cause,int btype, int itemId = 0);
void clif_skill_cooldown(struct map_session_data *sd, uint16 skill_id, t_tick tick);
int clif_skill_damage(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int64 sdamage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type);
//int clif_skill_damage2(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int damage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type);
bool clif_skill_nodamage(struct block_list *src,struct block_list *dst,uint16 skill_id,int heal,t_tick tick);
void clif_skill_poseffect(struct block_list *src,uint16 skill_id,int val,int x,int y,t_tick tick);
void clif_skill_estimation(struct map_session_data *sd,struct block_list *dst);
void clif_skill_warppoint(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, unsigned short map1, unsigned short map2, unsigned short map3, unsigned short map4);
void clif_skill_memomessage(struct map_session_data* sd, int type);
void clif_skill_teleportmessage(struct map_session_data *sd, int type);
void clif_skill_produce_mix_list(struct map_session_data *sd, int skill_id, int trigger);
void clif_cooking_list(struct map_session_data *sd, int trigger, uint16 skill_id, int qty, int list_type);

void clif_produceeffect(struct map_session_data* sd,int flag, unsigned short nameid);

void clif_getareachar_skillunit(struct block_list *bl, struct skill_unit *unit, enum send_target target, bool visible);
void clif_skill_delunit(struct skill_unit *unit);
void clif_skillunit_update(struct block_list* bl);

void clif_autospell(struct map_session_data *sd,uint16 skill_lv);
void clif_devotion(struct block_list *src, struct map_session_data *tsd);
void clif_spiritball(struct block_list *bl);
void clif_soulball(struct map_session_data *sd);
void clif_combo_delay(struct block_list *bl,t_tick wait);
void clif_bladestop(struct block_list *src, int dst_id, int active);
void clif_changemapcell(int fd, int16 m, int x, int y, int type, enum send_target target);

#define clif_status_load(bl, type, flag) clif_status_change((bl), (type), (flag), 0, 0, 0, 0)
void clif_status_change(struct block_list *bl, int type, int flag, t_tick tick, int val1, int val2, int val3);
void clif_efst_status_change(struct block_list *bl, int tid, enum send_target target, int type, t_tick tick, int val1, int val2, int val3);
void clif_efst_status_change_sub(struct block_list *tbl, struct block_list *bl, enum send_target target);

void clif_wis_message(struct map_session_data* sd, const char* nick, const char* mes, int mes_len, int gmlvl);
void clif_wis_end(int fd, int result);

void clif_solved_charname(int fd, int charid, const char* name);
void clif_name( struct block_list* src, struct block_list *bl, send_target target );
#define clif_name_self(bl) clif_name( (bl), (bl), SELF )
#define clif_name_area(bl) clif_name( (bl), (bl), AREA )

void clif_use_card(struct map_session_data *sd,int idx);
void clif_insert_card(struct map_session_data *sd,int idx_equip,int idx_card,int flag);

void clif_inventorylist(struct map_session_data *sd);
void clif_equiplist(struct map_session_data *sd);

void clif_cart_additem(struct map_session_data *sd,int n,int amount);
void clif_cart_additem_ack(struct map_session_data *sd, uint8 flag);
void clif_cart_delitem(struct map_session_data *sd,int n,int amount);
void clif_cartlist(struct map_session_data *sd);
void clif_clearcart(int fd);

void clif_item_identify_list(struct map_session_data *sd);
void clif_item_identified(struct map_session_data *sd,int idx,int flag);
void clif_item_repair_list(struct map_session_data *sd, struct map_session_data *dstsd, int lv);
void clif_item_repaireffect(struct map_session_data *sd, int idx, int flag);
void clif_item_damaged(struct map_session_data* sd, unsigned short position);
void clif_item_refine_list(struct map_session_data *sd);
void clif_hat_effects( struct map_session_data* sd, struct block_list* bl, enum send_target target );
void clif_hat_effect_single( struct map_session_data* sd, uint16 effectId, bool enable );

void clif_item_skill(struct map_session_data *sd,uint16 skill_id,uint16 skill_lv);

void clif_mvp_effect(struct map_session_data *sd);
void clif_mvp_item(struct map_session_data *sd, unsigned short nameid);
void clif_mvp_exp(struct map_session_data *sd, unsigned int exp);
void clif_mvp_noitem(struct map_session_data* sd);
void clif_changed_dir(struct block_list *bl, enum send_target target);

// vending
void clif_openvendingreq(struct map_session_data* sd, int num);
void clif_showvendingboard(struct block_list* bl, const char* message, int fd);
void clif_closevendingboard(struct block_list* bl, int fd);
void clif_vendinglist( struct map_session_data* sd, struct map_session_data* vsd );
void clif_buyvending(struct map_session_data* sd, int index, int amount, int fail);
void clif_openvending(struct map_session_data* sd, int id, struct s_vending* vending);
void clif_vendingreport(struct map_session_data* sd, int index, int amount, uint32 char_id, int zeny);

void clif_movetoattack(struct map_session_data *sd,struct block_list *bl);

// party
void clif_party_created(struct map_session_data *sd,int result);
void clif_party_member_info(struct party_data *p, struct map_session_data *sd);
void clif_party_info(struct party_data* p, struct map_session_data *sd);
void clif_party_invite(struct map_session_data *sd,struct map_session_data *tsd);
void clif_party_invite_reply(struct map_session_data* sd, const char* nick, enum e_party_invite_reply reply);
void clif_party_option(struct party_data *p,struct map_session_data *sd,int flag);
void clif_party_withdraw(struct map_session_data *sd, uint32 account_id, const char* name, enum e_party_member_withdraw result, enum send_target target);
void clif_party_message(struct party_data* p, uint32 account_id, const char* mes, int len);
void clif_party_xy(struct map_session_data *sd);
void clif_party_xy_single(int fd, struct map_session_data *sd);
void clif_party_hp(struct map_session_data *sd);
void clif_hpmeter_single(int fd, int id, unsigned int hp, unsigned int maxhp);
void clif_party_job_and_level(struct map_session_data *sd);
void clif_party_dead( struct map_session_data *sd );

// guild
void clif_guild_created(struct map_session_data *sd,int flag);
void clif_guild_belonginfo(struct map_session_data *sd);
void clif_guild_masterormember(struct map_session_data *sd);
void clif_guild_basicinfo(struct map_session_data *sd);
void clif_guild_allianceinfo(struct map_session_data *sd);
void clif_guild_memberlist(struct map_session_data *sd);
void clif_guild_skillinfo(struct map_session_data* sd);
void clif_guild_send_onlineinfo(struct map_session_data *sd); //[LuzZza]
void clif_guild_memberlogin_notice(struct guild *g,int idx,int flag);
void clif_guild_invite(struct map_session_data *sd,struct guild *g);
void clif_guild_inviteack(struct map_session_data *sd,int flag);
void clif_guild_leave(struct map_session_data *sd,const char *name,const char *mes);
void clif_guild_expulsion(struct map_session_data* sd, const char* name, const char* mes, uint32 account_id);
void clif_guild_positionchanged(struct guild *g,int idx);
void clif_guild_memberpositionchanged(struct guild *g,int idx);
void clif_guild_emblem(struct map_session_data *sd,struct guild *g);
void clif_guild_emblem_area(struct block_list* bl);
void clif_guild_notice(struct map_session_data* sd);
void clif_guild_message(struct guild *g,uint32 account_id,const char *mes,int len);
void clif_guild_reqalliance(struct map_session_data *sd,uint32 account_id,const char *name);
void clif_guild_allianceack(struct map_session_data *sd,int flag);
void clif_guild_delalliance(struct map_session_data *sd,int guild_id,int flag);
void clif_guild_oppositionack(struct map_session_data *sd,int flag);
void clif_guild_broken(struct map_session_data *sd,int flag);
void clif_guild_xy(struct map_session_data *sd);
void clif_guild_xy_single(int fd, struct map_session_data *sd);
void clif_guild_xy_remove(struct map_session_data *sd);

// Battleground
void clif_bg_hp(struct map_session_data *sd);
void clif_bg_xy(struct map_session_data *sd);
void clif_bg_xy_remove(struct map_session_data *sd);
void clif_bg_message(struct s_battleground_data *bg, int src_id, const char *name, const char *mes, int len);
void clif_bg_updatescore(int16 m);
void clif_bg_updatescore_single(struct map_session_data *sd);
void clif_sendbgemblem_area(struct map_session_data *sd);
void clif_sendbgemblem_single(int fd, struct map_session_data *sd);

// Battleground Queue
void clif_bg_queue_apply_result(e_bg_queue_apply_ack result, const char *name, struct map_session_data *sd);
void clif_bg_queue_apply_notify(const char *name, struct map_session_data *sd);
void clif_bg_queue_entry_init(struct map_session_data *sd);
void clif_bg_queue_lobby_notify(const char *name, struct map_session_data *sd);
void clif_bg_queue_ack_lobby(bool result, const char *name, const char *lobbyname, struct map_session_data *sd);

// Instancing
void clif_instance_create(int instance_id, int num);
void clif_instance_changewait(int instance_id, int num);
void clif_instance_status(int instance_id, unsigned int limit1, unsigned int limit2);
void clif_instance_changestatus(int instance_id, e_instance_notify type, unsigned int limit);

// Custom Fonts
void clif_font(struct map_session_data *sd);

// atcommand
void clif_displaymessage(const int fd, const char* mes);
void clif_disp_message(struct block_list* src, const char* mes, int len, enum send_target target);
void clif_broadcast(struct block_list* bl, const char* mes, int len, int type, enum send_target target);
void clif_broadcast2(struct block_list* bl, const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target);
void clif_heal(int fd,int type,int val);
void clif_resurrection(struct block_list *bl,int type);
void clif_map_property(struct block_list *bl, enum map_property property, enum send_target t);
void clif_pvpset(struct map_session_data *sd, int pvprank, int pvpnum,int type);
void clif_map_property_mapall(int map, enum map_property property);
void clif_refine(int fd, int fail, int index, int val);
void clif_upgrademessage( struct map_session_data* sd, int result, unsigned short item_id );

//petsystem
void clif_catch_process(struct map_session_data *sd);
void clif_pet_roulette(struct map_session_data *sd,int data);
void clif_sendegg(struct map_session_data *sd);
void clif_send_petstatus(struct map_session_data *sd);
void clif_send_petdata(struct map_session_data* sd, struct pet_data* pd, int type, int param);
#define clif_pet_equip(sd, pd) clif_send_petdata(sd, pd, 3, (pd)->vd.head_bottom)
#define clif_pet_equip_area(pd) clif_send_petdata(NULL, pd, 3, (pd)->vd.head_bottom)
#define clif_pet_performance(pd, param) clif_send_petdata(NULL, pd, 4, param)
void clif_pet_emotion(struct pet_data *pd,int param);
void clif_pet_food(struct map_session_data *sd,int foodid,int fail);

//friends list
int clif_friendslist_toggle_sub(struct map_session_data *sd,va_list ap);
void clif_friendslist_send(struct map_session_data *sd);
void clif_friendslist_reqack(struct map_session_data *sd, struct map_session_data *f_sd, int type);

void clif_weather(int16 m); // [Valaris]
void clif_specialeffect(struct block_list* bl, int type, enum send_target target); // special effects [Valaris]
void clif_specialeffect_single(struct block_list* bl, int type, int fd);
void clif_messagecolor_target(struct block_list *bl, unsigned long color, const char *msg, bool rgb2bgr, enum send_target type, struct map_session_data *sd);
#define clif_messagecolor(bl, color, msg, rgb2bgr, type) clif_messagecolor_target(bl, color, msg, rgb2bgr, type, NULL) // Mob/Npc color talk [SnakeDrak]
void clif_specialeffect_value(struct block_list* bl, int effect_id, int num, send_target target);

void clif_GM_kickack(struct map_session_data *sd, int id);
void clif_GM_kick(struct map_session_data *sd,struct map_session_data *tsd);
void clif_manner_message(struct map_session_data* sd, uint32 type);
void clif_GM_silence(struct map_session_data* sd, struct map_session_data* tsd, uint8 type);

void clif_disp_overhead_(struct block_list *bl, const char* mes, enum send_target flag);
#define clif_disp_overhead(bl, mes) clif_disp_overhead_(bl, mes, AREA)

void clif_get_weapon_view(struct map_session_data* sd, unsigned short *rhand, unsigned short *lhand);

void clif_party_xy_remove(struct map_session_data *sd); //Fix for minimap [Kevin]
void clif_gospel_info(struct map_session_data *sd, int type);
void clif_feel_req(int fd, struct map_session_data *sd, uint16 skill_lv);
void clif_starskill(struct map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result);
void clif_feel_info(struct map_session_data* sd, unsigned char feel_level, unsigned char type);
void clif_hate_info(struct map_session_data *sd, unsigned char hate_level,int class_, unsigned char type);
void clif_mission_info(struct map_session_data *sd, int mob_id, unsigned char progress);
void clif_feel_hate_reset(struct map_session_data *sd);

// [blackhole89]
void clif_hominfo(struct map_session_data *sd, struct homun_data *hd, int flag);
int clif_homskillinfoblock(struct map_session_data *sd);
void clif_homskillup(struct map_session_data *sd, uint16 skill_id);	//[orn]
void clif_hom_food(struct map_session_data *sd,int foodid,int fail);	//[orn]
void clif_send_homdata(struct map_session_data *sd, int state, int param);	//[orn]

void clif_configuration( struct map_session_data* sd, enum e_config_type type, bool enabled );
void clif_partytickack(struct map_session_data* sd, bool flag);
void clif_viewequip_ack(struct map_session_data* sd, struct map_session_data* tsd);
void clif_equipcheckbox(struct map_session_data* sd);

void clif_msg(struct map_session_data* sd, unsigned short id);
void clif_msg_value(struct map_session_data* sd, unsigned short id, int value);
void clif_msg_skill(struct map_session_data* sd, uint16 skill_id, int msg_id);

//quest system [Kevin] [Inkfish]
void clif_quest_send_list(struct map_session_data * sd);
void clif_quest_send_mission(struct map_session_data * sd);
void clif_quest_add(struct map_session_data * sd, struct quest * qd);
void clif_quest_delete(struct map_session_data * sd, int quest_id);
void clif_quest_update_status(struct map_session_data * sd, int quest_id, bool active);
void clif_quest_update_objective(struct map_session_data * sd, struct quest * qd, int mobid);
void clif_quest_show_event(struct map_session_data *sd, struct block_list *bl, e_questinfo_types effect, e_questinfo_markcolor color);
void clif_displayexp(struct map_session_data *sd, unsigned int exp, char type, bool quest, bool lost);

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
void clif_Mail_read(struct map_session_data *sd, int mail_id);
void clif_mail_delete(struct map_session_data* sd, struct mail_message *msg, bool success);
void clif_Mail_return(int fd, int mail_id, short fail);
void clif_Mail_send(struct map_session_data* sd, enum mail_send_result result);
void clif_Mail_new(struct map_session_data* sd, int mail_id, const char *sender, const char *title);
void clif_Mail_refreshinbox(struct map_session_data *sd,enum mail_inbox_type type,int64 mailID);
void clif_mail_getattachment(struct map_session_data* sd, struct mail_message *msg, uint8 result, enum mail_attachment_type type);
void clif_Mail_Receiver_Ack(struct map_session_data* sd, uint32 char_id, short class_, uint32 level, const char* name);
void clif_mail_removeitem(struct map_session_data* sd, bool success, int index, int amount);
// AUCTION SYSTEM
void clif_Auction_openwindow(struct map_session_data *sd);
void clif_Auction_results(struct map_session_data *sd, short count, short pages, uint8 *buf);
void clif_Auction_message(int fd, unsigned char flag);
void clif_Auction_close(int fd, unsigned char flag);
void clif_parse_Auction_cancelreg(int fd, struct map_session_data *sd);

void clif_bossmapinfo(struct map_session_data *sd, struct mob_data *md, enum e_bossmap_info flag);
void clif_cashshop_show(struct map_session_data *sd, struct npc_data *nd);

// ADOPTION
void clif_Adopt_reply(struct map_session_data *sd, int type);
void clif_Adopt_request(struct map_session_data *sd, struct map_session_data *src, int p_id);

// MERCENARIES
void clif_mercenary_info(struct map_session_data *sd);
void clif_mercenary_skillblock(struct map_session_data *sd);
void clif_mercenary_message(struct map_session_data* sd, int message);
void clif_mercenary_updatestatus(struct map_session_data *sd, int type);

// RENTAL SYSTEM
void clif_rental_time( struct map_session_data* sd, unsigned short nameid, int seconds );
void clif_rental_expired( struct map_session_data* sd, int index, unsigned short nameid );

// BOOK READING
void clif_readbook(int fd, int book_id, int page);

// Show Picker
void clif_party_show_picker(struct map_session_data * sd, struct item * item_data);

// Progress Bar [Inkfish]
void clif_progressbar(struct map_session_data * sd, unsigned long color, unsigned int second);
void clif_progressbar_abort(struct map_session_data * sd);
void clif_progressbar_npc(struct npc_data *nd, struct map_session_data* sd);
#define clif_progressbar_npc_area(nd) clif_progressbar_npc((nd),NULL)

void clif_PartyBookingRegisterAck(struct map_session_data *sd, int flag);
void clif_PartyBookingDeleteAck(struct map_session_data* sd, int flag);
void clif_PartyBookingSearchAck(int fd, struct party_booking_ad_info** results, int count, bool more_result);
void clif_PartyBookingUpdateNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad);
void clif_PartyBookingDeleteNotify(struct map_session_data* sd, int index);
void clif_PartyBookingInsertNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad);

/* Bank System [Yommy/Hercules] */
void clif_bank_deposit (struct map_session_data *sd, enum e_BANKING_DEPOSIT_ACK reason);
void clif_bank_withdraw (struct map_session_data *sd,enum e_BANKING_WITHDRAW_ACK reason);
void clif_parse_BankDeposit (int fd, struct map_session_data *sd);
void clif_parse_BankWithdraw (int fd, struct map_session_data *sd);
void clif_parse_BankCheck (int fd, struct map_session_data *sd);
void clif_parse_BankOpen (int fd, struct map_session_data *sd);
void clif_parse_BankClose (int fd, struct map_session_data *sd);

void clif_showdigit(struct map_session_data* sd, unsigned char type, int value);

/// Buying Store System
void clif_buyingstore_open(struct map_session_data* sd);
void clif_buyingstore_open_failed(struct map_session_data* sd, unsigned short result, unsigned int weight);
void clif_buyingstore_myitemlist(struct map_session_data* sd);
void clif_buyingstore_entry(struct map_session_data* sd);
void clif_buyingstore_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd);
void clif_buyingstore_disappear_entry(struct map_session_data* sd);
void clif_buyingstore_disappear_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd);
void clif_buyingstore_itemlist(struct map_session_data* sd, struct map_session_data* pl_sd);
void clif_buyingstore_trade_failed_buyer(struct map_session_data* sd, short result);
void clif_buyingstore_update_item(struct map_session_data* sd, unsigned short nameid, unsigned short amount, uint32 char_id, int zeny);
void clif_buyingstore_delete_item(struct map_session_data* sd, short index, unsigned short amount, int price);
void clif_buyingstore_trade_failed_seller(struct map_session_data* sd, short result, unsigned short nameid);

/// Search Store System
void clif_search_store_info_ack(struct map_session_data* sd);
void clif_search_store_info_failed(struct map_session_data* sd, unsigned char reason);
void clif_open_search_store_info(struct map_session_data* sd);
void clif_search_store_info_click_ack(struct map_session_data* sd, short x, short y);

/// Cash Shop
void clif_cashshop_result( struct map_session_data* sd, unsigned short item_id, uint16 result );
void clif_cashshop_open( struct map_session_data* sd, int tab );

void clif_display_pinfo(struct map_session_data *sd, int type);

/// Roulette
void clif_roulette_open(struct map_session_data* sd);
void clif_parse_roulette_open(int fd, struct map_session_data *sd);
void clif_parse_roulette_info(int fd, struct map_session_data *sd);
void clif_parse_roulette_close(int fd, struct map_session_data *sd);
void clif_parse_roulette_generate(int fd, struct map_session_data *sd);
void clif_parse_roulette_item(int fd, struct map_session_data *sd);

void clif_elementalconverter_list(struct map_session_data *sd);

void clif_millenniumshield(struct block_list *bl, short shields);

void clif_spellbook_list(struct map_session_data *sd);

void clif_magicdecoy_list(struct map_session_data *sd, uint16 skill_lv, short x, short y);

void clif_poison_list(struct map_session_data *sd, uint16 skill_lv);

int clif_autoshadowspell_list(struct map_session_data *sd);

int clif_skill_itemlistwindow( struct map_session_data *sd, uint16 skill_id, uint16 skill_lv );
void clif_elemental_info(struct map_session_data *sd);
void clif_elemental_updatestatus(struct map_session_data *sd, int type);

void clif_spiritcharm(struct map_session_data *sd);

void clif_snap( struct block_list *bl, short x, short y );
void clif_monster_hp_bar( struct mob_data* md, int fd );

// Clan System
void clif_clan_basicinfo( struct map_session_data *sd );
void clif_clan_message(struct clan *clan,const char *mes,int len);
void clif_clan_onlinecount( struct clan* clan );
void clif_clan_leave( struct map_session_data* sd );

// Bargain Tool
void clif_sale_start(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void clif_sale_end(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void clif_sale_amount(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void clif_sale_open(struct map_session_data* sd);

// Refine UI
void clif_refineui_open( struct map_session_data* sd );

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
	COLOR_MAX
};
extern unsigned long color_table[COLOR_MAX];

void clif_channel_msg(struct Channel *channel, const char *msg, unsigned long color);

#define clif_menuskill_clear(sd) (sd)->menuskill_id = (sd)->menuskill_val = (sd)->menuskill_val2 = 0;

void clif_ranklist(struct map_session_data *sd, int16 rankingType);
void clif_update_rankingpoint(struct map_session_data *sd, int rankingtype, int point);

void clif_crimson_marker(struct map_session_data *sd, struct block_list *bl, bool remove);

void clif_showscript(struct block_list* bl, const char* message, enum send_target flag);
void clif_party_leaderchanged(struct map_session_data *sd, int prev_leader_aid, int new_leader_aid);

void clif_account_name(int fd, uint32 account_id, const char* accname);
void clif_notify_bindOnEquip(struct map_session_data *sd, int n);

void clif_merge_item_open(struct map_session_data *sd);

void clif_broadcast_obtain_special_item(const char *char_name, unsigned short nameid, unsigned short container, enum BROADCASTING_SPECIAL_ITEM_OBTAIN type);

void clif_dressing_room(struct map_session_data *sd, int flag);
void clif_navigateTo(struct map_session_data *sd, const char* mapname, uint16 x, uint16 y, uint8 flag, bool hideWindow, uint16 mob_id );
void clif_SelectCart(struct map_session_data *sd);

/// Achievement System
void clif_achievement_list_all(struct map_session_data *sd);
void clif_achievement_update(struct map_session_data *sd, struct achievement *ach, int count);
void clif_achievement_reward_ack(int fd, unsigned char result, int ach_id);

/// Attendance System
enum in_ui_type : int8 {
	IN_UI_ATTENDANCE = 5
};

enum out_ui_type : int8 {
	OUT_UI_ATTENDANCE = 7
};

void clif_ui_open( struct map_session_data *sd, enum out_ui_type ui_type, int32 data );
void clif_attendence_response( struct map_session_data *sd, int32 data );

void clif_weight_limit( struct map_session_data* sd );

void clif_guild_storage_log( struct map_session_data* sd, std::vector<struct guild_log_entry>& log, enum e_guild_storage_log result );

void clif_camerainfo( struct map_session_data* sd, bool show, float range = 0.0f, float rotation = 0.0f, float latitude = 0.0f );

/// Equip Switch System
void clif_equipswitch_list( struct map_session_data* sd );
void clif_equipswitch_add( struct map_session_data* sd,uint16 index, uint32 pos, uint8 flag );
void clif_equipswitch_remove( struct map_session_data* sd, uint16 index, uint32 pos, bool failed );
void clif_equipswitch_reply( struct map_session_data* sd, bool failed );

/// Pet evolution
void clif_pet_evolution_result( struct map_session_data* sd, e_pet_evolution_result result );

void clif_parse_skill_toid( struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, int target_id );

#endif /* CLIF_HPP */
