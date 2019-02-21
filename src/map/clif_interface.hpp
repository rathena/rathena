// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLIF_INTERFACE_HPP
#define CLIF_INTERFACE_HPP

#include <vector>

#include <stdarg.h>

#include "../common/cbasetypes.hpp"
#include "../common/db.hpp" //dbmap
#include "../common/mmo.hpp"
#include "../common/timer.hpp" // t_tick

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
struct battleground_data;
struct quest;
struct party_booking_ad_info;
struct sale_item_data;
struct mail_message;
struct achievement;
struct guild_log_entry;
enum e_guild_storage_log : uint16;

enum e_PacketDBVersion { // packet DB
	MIN_PACKET_DB  = 0x064,
	MAX_PACKET_DB  = 0xAFF,
	MAX_PACKET_POS = 20,
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
{// map_property
	MAPPROPERTY_NOTHING       = 0,
	MAPPROPERTY_FREEPVPZONE   = 1,
	MAPPROPERTY_EVENTPVPZONE  = 2,
	MAPPROPERTY_AGITZONE      = 3,
	MAPPROPERTY_PKSERVERZONE  = 4, // message "You are in a PK area. Please beware of sudden attacks." in color 0x9B9BFF (light red)
	MAPPROPERTY_PVPSERVERZONE = 5,
	MAPPROPERTY_DENYSKILLZONE = 6,
};

enum map_type : uint8_t 
{// map_type
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
{// skill_fail
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
	// cart_additem_ack flags
	ADDITEM_TO_CART_FAIL_WEIGHT = 0x0,
	ADDITEM_TO_CART_FAIL_COUNT = 0x1,

	// equipitemack flags
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
	DMG_SKILL,				/// (skill?)
	DMG_REPEAT,				/// (repeat damage?)
	DMG_MULTI_HIT,			/// multi-hit damage
	DMG_MULTI_HIT_ENDURE,	/// multi-hit damage (endure)
	DMG_CRITICAL,			/// critical hit
	DMG_LUCY_DODGE,			/// lucky dodge
	DMG_TOUCH,				/// (touch skill?)
};

enum e_config_type : uint32 {
	CONFIG_OPEN_EQUIPMENT_WINDOW = 0,
	// Unknown
	CONFIG_PET_AUTOFEED = 2,
	CONFIG_HOMUNCULUS_AUTOFEED
};

/// Attendance System
enum in_ui_type : int8 {
	IN_UI_ATTENDANCE = 5
};

enum out_ui_type : int8 {
	OUT_UI_ATTENDANCE = 7
};

enum mail_send_result : uint8_t {
	WRITE_MAIL_SUCCESS = 0x0,
	WRITE_MAIL_FAILED = 0x1,
	WRITE_MAIL_FAILED_CNT = 0x2,
	WRITE_MAIL_FAILED_ITEM = 0x3,
	WRITE_MAIL_FAILED_CHECK_CHARACTER_NAME = 0x4,
	WRITE_MAIL_FAILED_WHISPEREXREGISTER = 0x5,
};

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

class Clif_Interface {
	public:
		virtual ~Clif_Interface() = default;
		virtual int setip(const char* ip) = 0;
		virtual void setbindip(const char* ip)= 0;
		virtual void setport(uint16 port)= 0;

		virtual uint32 getip(void)= 0;
		virtual uint32 refresh_ip(void)= 0;
		virtual uint16 getport(void)= 0;

		virtual void authok(struct map_session_data *sd)= 0;
		virtual void authrefuse(int fd, uint8 error_code)= 0;
		virtual void authfail_fd(int fd, int type)= 0;
		virtual void charselectok(int id, uint8 ok)= 0;
		virtual void dropflooritem(struct flooritem_data* fitem, bool canShowEffect)= 0;
		virtual void clearflooritem(struct flooritem_data *fitem, int fd)= 0;

		virtual void clearunit_single(int id, clr_type type, int fd)= 0;
		virtual void clearunit_area(struct block_list* bl, clr_type type)= 0;
		virtual void clearunit_delayed(struct block_list* bl, clr_type type, t_tick tick)= 0;
		virtual int spawn(struct block_list *bl)	 = 0; //area
		virtual void walkok(struct map_session_data *sd)	 = 0; //self
		virtual void move(struct unit_data *ud)  = 0; //area
		virtual void changemap(struct map_session_data *sd, short m, int x, int y)	 = 0; //self
		virtual void changemapserver(struct map_session_data* sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port)	 = 0; //self
		virtual void blown(struct block_list *bl)  = 0; //area
		virtual void slide(struct block_list *bl, int x, int y)  = 0; //area
		virtual void fixpos(struct block_list *bl)	 = 0; //area
virtual void npcbuysell(struct map_session_data* sd, int id)	 = 0; //self
virtual void buylist(struct map_session_data *sd, struct npc_data *nd)	 = 0; //self
virtual void selllist(struct map_session_data *sd)	 = 0; //self
virtual void npc_market_open(struct map_session_data *sd, struct npc_data *nd)= 0;
virtual void scriptmes(struct map_session_data *sd, int npcid, const char *mes)	 = 0; //self
virtual void scriptnext(struct map_session_data *sd,int npcid)	 = 0; //self
virtual void scriptclose(struct map_session_data *sd, int npcid)	 = 0; //self
virtual void scriptclear(struct map_session_data *sd, int npcid)	 = 0; //self
virtual void scriptmenu(struct map_session_data* sd, int npcid, const char* mes)	 = 0; //self
virtual void scriptinput(struct map_session_data *sd, int npcid)	 = 0; //self
virtual void scriptinputstr(struct map_session_data *sd, int npcid)	 = 0; //self
virtual void cutin(struct map_session_data* sd, const char* image, int type)	 = 0; //self
virtual void viewpoint(struct map_session_data *sd, int npc_id, int type, int x, int y, int id, int color)	 = 0; //self
virtual void additem(struct map_session_data *sd, int n, int amount, unsigned char fail)  = 0; //self
virtual void dropitem(struct map_session_data *sd,int n,int amount)	 = 0; //self
virtual void delitem(struct map_session_data *sd,int n,int amount, short reason)  = 0; //self
virtual void updatestatus(struct map_session_data *sd,int type)	 = 0; //self
virtual void changestatus(struct map_session_data* sd,int type,int val)	 = 0; //area
virtual int damage(struct block_list* src, struct block_list* dst, t_tick tick, int sdelay, int ddelay, int64 sdamage, int div, enum e_damage_type type, int64 sdamage2, bool spdamage)	 = 0; //area
virtual void takeitem(struct block_list* src, struct block_list* dst)= 0;
virtual void sitting(struct block_list* bl)= 0;
virtual void standing(struct block_list* bl)= 0;
virtual void sprite_change(struct block_list *bl, int id, int type, int val, int val2, enum send_target target)= 0;
virtual void changelook(struct block_list *bl,int type,int val)	 = 0; //area
virtual void changetraplook(struct block_list *bl,int val)  = 0; //area
virtual void refreshlook(struct block_list *bl,int id,int type,int val,enum send_target target)= 0; //area specified in 'target'
virtual void arrowequip(struct map_session_data *sd,int val)  = 0; //self
virtual void arrow_fail(struct map_session_data *sd,int type)  = 0; //self
virtual void arrow_create_list(struct map_session_data *sd)	 = 0; //self
virtual void statusupack(struct map_session_data *sd,int type,int ok,int val)	 = 0; //self
virtual void equipitemack(struct map_session_data *sd,int n,int pos,uint8 flag)	 = 0; //self
virtual void unequipitemack(struct map_session_data *sd,int n,int pos,int ok)	 = 0; //self
virtual void misceffect(struct block_list* bl,int type)	 = 0; //area
virtual void changeoption(struct block_list* bl)	 = 0; //area
virtual void changeoption2(struct block_list* bl)	 = 0; //area
virtual void useitemack(struct map_session_data *sd,int index,int amount,bool ok)	 = 0; //self
virtual void GlobalMessage(struct block_list* bl, const char* message,enum send_target target)= 0;
virtual void createchat(struct map_session_data* sd, int flag)	 = 0; //self
virtual void dispchat(struct chat_data* cd, int fd)	 = 0; //area or fd
virtual void joinchatfail(struct map_session_data *sd,int flag)	 = 0; //self
virtual void joinchatok(struct map_session_data *sd,struct chat_data* cd)	 = 0; //self
virtual void addchat(struct chat_data* cd,struct map_session_data *sd)	 = 0; //chat
virtual void changechatowner(struct chat_data* cd, struct map_session_data* sd)	 = 0; //chat
virtual void clearchat(struct chat_data *cd,int fd)	 = 0; //area or fd
virtual void leavechat(struct chat_data* cd, struct map_session_data* sd, bool flag)	 = 0; //chat
virtual void changechatstatus(struct chat_data* cd)	 = 0; //chat
virtual void refresh_storagewindow(struct map_session_data *sd)= 0;
virtual void refresh(struct map_session_data *sd)	 = 0; //self

virtual void emotion(struct block_list *bl,int type)= 0;
virtual void talkiebox(struct block_list* bl, const char* talkie)= 0;
virtual void wedding_effect(struct block_list *bl)= 0;
virtual void divorced(struct map_session_data* sd, const char* name)= 0;
virtual void callpartner(struct map_session_data *sd)= 0;
virtual void playBGM(struct map_session_data* sd, const char* name)= 0;
virtual void soundeffect(struct map_session_data* sd, struct block_list* bl, const char* name, int type)= 0;
virtual void soundeffectall(struct block_list* bl, const char* name, int type, enum send_target coverage)= 0;
virtual void parse_ActionRequest_sub(struct map_session_data *sd, int action_type, int target_id, t_tick tick)= 0;
virtual void hotkeys_send(struct map_session_data *sd)= 0;

// trade= 0;
virtual void traderequest(struct map_session_data* sd, const char* name)= 0;
virtual void tradestart(struct map_session_data* sd, uint8 type)= 0;
virtual void tradeadditem(struct map_session_data* sd, struct map_session_data* tsd, int index, int amount)= 0;
virtual void tradeitemok(struct map_session_data* sd, int index, int fail)= 0;
virtual void tradedeal_lock(struct map_session_data* sd, int fail)= 0;
virtual void tradecancelled(struct map_session_data* sd)= 0;
virtual void tradecompleted(struct map_session_data* sd, int fail)= 0;
virtual void tradeundo(struct map_session_data* sd)= 0;

// storage= 0;
virtual void storagelist(struct map_session_data* sd, struct item* items, int items_length, const char *storename)= 0;
virtual void updatestorageamount(struct map_session_data* sd, int amount, int max_amount)= 0;
virtual void storageitemadded(struct map_session_data* sd, struct item* i, int index, int amount)= 0;
virtual void storageitemremoved(struct map_session_data* sd, int index, int amount)= 0;
virtual void storageclose(struct map_session_data* sd)= 0;

virtual void skillinfoblock(struct map_session_data *sd)= 0;
virtual void skillup(struct map_session_data *sd, uint16 skill_id, int lv, int range, int upgradable)= 0;
virtual void skillinfo(struct map_session_data *sd,int skill_id, int inf)= 0;
virtual void addskill(struct map_session_data *sd, int skill_id)= 0;
virtual void deleteskill(struct map_session_data *sd, int skill_id)= 0;

virtual void skillcasting(struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, int property, int casttime)= 0;
virtual void skillcastcancel(struct block_list* bl)= 0;
virtual void skill_fail(struct map_session_data *sd,uint16 skill_id,enum useskill_fail_cause cause,int btype)= 0;
virtual void skill_cooldown(struct map_session_data *sd, uint16 skill_id, t_tick tick)= 0;
virtual int skill_damage(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int64 sdamage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type)= 0;
//int skill_damage2(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int damage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type)= 0;
virtual bool skill_nodamage(struct block_list *src,struct block_list *dst,uint16 skill_id,int heal,t_tick tick)= 0;
virtual void skill_poseffect(struct block_list *src,uint16 skill_id,int val,int x,int y,t_tick tick)= 0;
virtual void skill_estimation(struct map_session_data *sd,struct block_list *dst)= 0;
virtual void skill_warppoint(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, unsigned short map1, unsigned short map2, unsigned short map3, unsigned short map4)= 0;
virtual void skill_memomessage(struct map_session_data* sd, int type)= 0;
virtual void skill_teleportmessage(struct map_session_data *sd, int type)= 0;
virtual void skill_produce_mix_list(struct map_session_data *sd, int skill_id, int trigger)= 0;
virtual void cooking_list(struct map_session_data *sd, int trigger, uint16 skill_id, int qty, int list_type)= 0;

virtual void produceeffect(struct map_session_data* sd,int flag, unsigned short nameid)= 0;

virtual void getareachar_skillunit(struct block_list *bl, struct skill_unit *unit, enum send_target target, bool visible)= 0;
virtual void skill_delunit(struct skill_unit *unit)= 0;
virtual void skillunit_update(struct block_list* bl)= 0;

virtual void autospell(struct map_session_data *sd,uint16 skill_lv)= 0;
virtual void devotion(struct block_list *src, struct map_session_data *tsd)= 0;
virtual void spiritball(struct block_list *bl)= 0;
virtual void combo_delay(struct block_list *bl,t_tick wait)= 0;
virtual void bladestop(struct block_list *src, int dst_id, int active)= 0;
virtual void changemapcell(int fd, int16 m, int x, int y, int type, enum send_target target)= 0;

virtual void status_change(struct block_list *bl, int type, int flag, t_tick tick, int val1, int val2, int val3)= 0;
virtual void efst_status_change(struct block_list *bl, int tid, enum send_target target, int type, t_tick tick, int val1, int val2, int val3)= 0;
virtual void efst_status_change_sub(struct block_list *tbl, struct block_list *bl, enum send_target target)= 0;

virtual void wis_message(struct map_session_data* sd, const char* nick, const char* mes, int mes_len, int gmlvl)= 0;
virtual void wis_end(int fd, int result)= 0;

virtual void solved_charname(int fd, int charid, const char* name)= 0;
virtual void name( struct block_list* src, struct block_list *bl, send_target target )= 0;

virtual void use_card(struct map_session_data *sd,int idx)= 0;
virtual void insert_card(struct map_session_data *sd,int idx_equip,int idx_card,int flag)= 0;

virtual void inventorylist(struct map_session_data *sd)= 0;
virtual void equiplist(struct map_session_data *sd)= 0;

virtual void cart_additem(struct map_session_data *sd,int n,int amount,int fail)= 0;
virtual void cart_additem_ack(struct map_session_data *sd, uint8 flag)= 0;
virtual void cart_delitem(struct map_session_data *sd,int n,int amount)= 0;
virtual void cartlist(struct map_session_data *sd)= 0;
virtual void clearcart(int fd)= 0;

virtual void item_identify_list(struct map_session_data *sd)= 0;
virtual void item_identified(struct map_session_data *sd,int idx,int flag)= 0;
virtual void item_repair_list(struct map_session_data *sd, struct map_session_data *dstsd, int lv)= 0;
virtual void item_repaireffect(struct map_session_data *sd, int idx, int flag)= 0;
virtual void item_damaged(struct map_session_data* sd, unsigned short position)= 0;
virtual void item_refine_list(struct map_session_data *sd)= 0;
virtual void hat_effects( struct map_session_data* sd, struct block_list* bl, enum send_target target )= 0;
virtual void hat_effect_single( struct map_session_data* sd, uint16 effectId, bool enable )= 0;

virtual void item_skill(struct map_session_data *sd,uint16 skill_id,uint16 skill_lv)= 0;

virtual void mvp_effect(struct map_session_data *sd)= 0;
virtual void mvp_item(struct map_session_data *sd, unsigned short nameid)= 0;
virtual void mvp_exp(struct map_session_data *sd, unsigned int exp)= 0;
virtual void mvp_noitem(struct map_session_data* sd)= 0;
virtual void changed_dir(struct block_list *bl, enum send_target target)= 0;

// vending= 0;
virtual void openvendingreq(struct map_session_data* sd, int num)= 0;
virtual void showvendingboard(struct block_list* bl, const char* message, int fd)= 0;
virtual void closevendingboard(struct block_list* bl, int fd)= 0;
virtual void vendinglist(struct map_session_data* sd, int id, struct s_vending* vending)= 0;
virtual void buyvending(struct map_session_data* sd, int index, int amount, int fail)= 0;
virtual void openvending(struct map_session_data* sd, int id, struct s_vending* vending)= 0;
virtual void vendingreport(struct map_session_data* sd, int index, int amount, uint32 char_id, int zeny)= 0;

virtual void movetoattack(struct map_session_data *sd,struct block_list *bl)= 0;

// party= 0;
virtual void party_created(struct map_session_data *sd,int result)= 0;
virtual void party_member_info(struct party_data *p, struct map_session_data *sd)= 0;
virtual void party_info(struct party_data* p, struct map_session_data *sd)= 0;
virtual void party_invite(struct map_session_data *sd,struct map_session_data *tsd)= 0;
virtual void party_invite_reply(struct map_session_data* sd, const char* nick, enum e_party_invite_reply reply)= 0;
virtual void party_option(struct party_data *p,struct map_session_data *sd,int flag)= 0;
virtual void party_withdraw(struct map_session_data *sd, uint32 account_id, const char* name, enum e_party_member_withdraw result, enum send_target target)= 0;
virtual void party_message(struct party_data* p, uint32 account_id, const char* mes, int len)= 0;
virtual void party_xy(struct map_session_data *sd)= 0;
virtual void party_xy_single(int fd, struct map_session_data *sd)= 0;
virtual void party_hp(struct map_session_data *sd)= 0;
virtual void hpmeter_single(int fd, int id, unsigned int hp, unsigned int maxhp)= 0;
virtual void party_job_and_level(struct map_session_data *sd)= 0;
virtual void party_dead( struct map_session_data *sd )= 0;

// guild= 0;
virtual void guild_created(struct map_session_data *sd,int flag)= 0;
virtual void guild_belonginfo(struct map_session_data *sd)= 0;
virtual void guild_masterormember(struct map_session_data *sd)= 0;
virtual void guild_basicinfo(struct map_session_data *sd)= 0;
virtual void guild_allianceinfo(struct map_session_data *sd)= 0;
virtual void guild_memberlist(struct map_session_data *sd)= 0;
virtual void guild_skillinfo(struct map_session_data* sd)= 0;
virtual void guild_send_onlineinfo(struct map_session_data *sd) = 0;//[LuzZza]
virtual void guild_memberlogin_notice(struct guild *g,int idx,int flag)= 0;
virtual void guild_invite(struct map_session_data *sd,struct guild *g)= 0;
virtual void guild_inviteack(struct map_session_data *sd,int flag)= 0;
virtual void guild_leave(struct map_session_data *sd,const char *name,const char *mes)= 0;
virtual void guild_expulsion(struct map_session_data* sd, const char* name, const char* mes, uint32 account_id)= 0;
virtual void guild_positionchanged(struct guild *g,int idx)= 0;
virtual void guild_memberpositionchanged(struct guild *g,int idx)= 0;
virtual void guild_emblem(struct map_session_data *sd,struct guild *g)= 0;
virtual void guild_emblem_area(struct block_list* bl)= 0;
virtual void guild_notice(struct map_session_data* sd)= 0;
virtual void guild_message(struct guild *g,uint32 account_id,const char *mes,int len)= 0;
virtual void guild_reqalliance(struct map_session_data *sd,uint32 account_id,const char *name)= 0;
virtual void guild_allianceack(struct map_session_data *sd,int flag)= 0;
virtual void guild_delalliance(struct map_session_data *sd,int guild_id,int flag)= 0;
virtual void guild_oppositionack(struct map_session_data *sd,int flag)= 0;
virtual void guild_broken(struct map_session_data *sd,int flag)= 0;
virtual void guild_xy(struct map_session_data *sd)= 0;
virtual void guild_xy_single(int fd, struct map_session_data *sd)= 0;
virtual void guild_xy_remove(struct map_session_data *sd)= 0;

// Battleground= 0;
virtual void bg_hp(struct map_session_data *sd)= 0;
virtual void bg_xy(struct map_session_data *sd)= 0;
virtual void bg_xy_remove(struct map_session_data *sd)= 0;
virtual void bg_message(struct battleground_data *bg, int src_id, const char *name, const char *mes, int len)= 0;
virtual void bg_updatescore(int16 m)= 0;
virtual void bg_updatescore_single(struct map_session_data *sd)= 0;
virtual void sendbgemblem_area(struct map_session_data *sd)= 0;
virtual void sendbgemblem_single(int fd, struct map_session_data *sd)= 0;

// Instancing= 0;
virtual void instance_create(unsigned short instance_id, int num)= 0;
virtual void instance_changewait(unsigned short instance_id, int num)= 0;
virtual void instance_status(unsigned short instance_id, unsigned int limit1, unsigned int limit2)= 0;
virtual void instance_changestatus(unsigned int instance_id, int type, unsigned int limit)= 0;

// Custom Fonts= 0;
virtual void font(struct map_session_data *sd)= 0;

// atcommand= 0;
virtual void displaymessage(const int fd, const char* mes)= 0;
virtual void disp_message(struct block_list* src, const char* mes, int len, enum send_target target)= 0;
virtual void broadcast(struct block_list* bl, const char* mes, int len, int type, enum send_target target)= 0;
virtual void broadcast2(struct block_list* bl, const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target)= 0;
virtual void heal(int fd,int type,int val)= 0;
virtual void resurrection(struct block_list *bl,int type)= 0;
virtual void map_property(struct block_list *bl, enum map_property property, enum send_target t)= 0;
virtual void pvpset(struct map_session_data *sd, int pvprank, int pvpnum,int type)= 0;
virtual void map_property_mapall(int map, enum map_property property)= 0;
virtual void refine(int fd, int fail, int index, int val)= 0;
virtual void upgrademessage(int fd, int result, unsigned short item_id)= 0;

//petsystem= 0;
virtual void catch_process(struct map_session_data *sd)= 0;
virtual void pet_roulette(struct map_session_data *sd,int data)= 0;
virtual void sendegg(struct map_session_data *sd)= 0;
virtual void send_petstatus(struct map_session_data *sd)= 0;
virtual void pet_emotion(struct pet_data *pd,int param)= 0;
virtual void pet_food(struct map_session_data *sd,int foodid,int fail)= 0;

//friends list= 0;
virtual void friendslist_send(struct map_session_data *sd)= 0;

virtual void weather(int16 m) = 0;// [Valaris];
virtual void specialeffect(struct block_list* bl, int type, enum send_target target) = 0; // special effects [Valaris]
virtual void specialeffect_single(struct block_list* bl, int type, int fd)= 0;
virtual void messagecolor_target(struct block_list *bl, unsigned long color, const char *msg, bool rgb2bgr, enum send_target type, struct map_session_data *sd)= 0;
virtual void specialeffect_value(struct block_list* bl, int effect_id, int num, send_target target)= 0;

virtual void GM_kickack(struct map_session_data *sd, int id)= 0;
virtual void GM_kick(struct map_session_data *sd,struct map_session_data *tsd)= 0;
virtual void manner_message(struct map_session_data* sd, uint32 type)= 0;
virtual void GM_silence(struct map_session_data* sd, struct map_session_data* tsd, uint8 type)= 0;

virtual void disp_overhead_(struct block_list *bl, const char* mes, enum send_target flag)= 0;

virtual void get_weapon_view(struct map_session_data* sd, unsigned short *rhand, unsigned short *lhand)= 0;

virtual void party_xy_remove(struct map_session_data *sd) = 0;//Fix for minimap [Kevin]
virtual void gospel_info(struct map_session_data *sd, int type)= 0;
virtual void feel_req(int fd, struct map_session_data *sd, uint16 skill_lv)= 0;
virtual void starskill(struct map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result)= 0;
virtual void feel_info(struct map_session_data* sd, unsigned char feel_level, unsigned char type)= 0;
virtual void hate_info(struct map_session_data *sd, unsigned char hate_level,int class_, unsigned char type)= 0;
virtual void mission_info(struct map_session_data *sd, int mob_id, unsigned char progress)= 0;
virtual void feel_hate_reset(struct map_session_data *sd)= 0;

// [blackhole89]= 0;
virtual void hominfo(struct map_session_data *sd, struct homun_data *hd, int flag)= 0;
virtual int homskillinfoblock(struct map_session_data *sd)= 0;
virtual void homskillup(struct map_session_data *sd, uint16 skill_id)= 0;	//[orn]
virtual int hom_food(struct map_session_data *sd,int foodid,int fail)= 0;	//[orn]
virtual void send_homdata(struct map_session_data *sd, int state, int param) = 0;	//[orn]

virtual void configuration( struct map_session_data* sd, enum e_config_type type, bool enabled )= 0;
virtual void partytickack(struct map_session_data* sd, bool flag)= 0;
virtual void viewequip_ack(struct map_session_data* sd, struct map_session_data* tsd)= 0;
virtual void equipcheckbox(struct map_session_data* sd)= 0;

virtual void msg(struct map_session_data* sd, unsigned short id)= 0;
virtual void msg_value(struct map_session_data* sd, unsigned short id, int value)= 0;
virtual void msg_skill(struct map_session_data* sd, uint16 skill_id, int msg_id)= 0;

//quest system [Kevin] [Inkfish]= 0;
virtual void quest_send_list(struct map_session_data * sd)= 0;
virtual void quest_send_mission(struct map_session_data * sd)= 0;
virtual void quest_add(struct map_session_data * sd, struct quest * qd)= 0;
virtual void quest_delete(struct map_session_data * sd, int quest_id)= 0;
virtual void quest_update_status(struct map_session_data * sd, int quest_id, bool active)= 0;
virtual void quest_update_objective(struct map_session_data * sd, struct quest * qd, int mobid)= 0;
virtual void quest_show_event(struct map_session_data *sd, struct block_list *bl, short state, short color)= 0;
virtual void displayexp(struct map_session_data *sd, unsigned int exp, char type, bool quest, bool lost)= 0;

virtual int send(const uint8* buf, int len, struct block_list* bl, enum send_target type)= 0;

// MAIL SYSTEM= 0;

virtual void Mail_window(int fd, int flag)= 0;
virtual void Mail_read(struct map_session_data *sd, int mail_id)= 0;
virtual void mail_delete(struct map_session_data* sd, struct mail_message *msg, bool success)= 0;
virtual void Mail_return(int fd, int mail_id, short fail)= 0;
virtual void Mail_send(struct map_session_data* sd, enum mail_send_result result)= 0;
virtual void Mail_new(struct map_session_data* sd, int mail_id, const char *sender, const char *title)= 0;
virtual void Mail_refreshinbox(struct map_session_data *sd,enum mail_inbox_type type,int64 mailID)= 0;
virtual void mail_getattachment(struct map_session_data* sd, struct mail_message *msg, uint8 result, enum mail_attachment_type type)= 0;
virtual void Mail_Receiver_Ack(struct map_session_data* sd, uint32 char_id, short class_, uint32 level, const char* name)= 0;
virtual void mail_removeitem(struct map_session_data* sd, bool success, int index, int amount)= 0;

// AUCTION SYSTEM= 0;
virtual void Auction_openwindow(struct map_session_data *sd)= 0;
virtual void Auction_results(struct map_session_data *sd, short count, short pages, uint8 *buf)= 0;
virtual void Auction_message(int fd, unsigned char flag)= 0;
virtual void Auction_close(int fd, unsigned char flag)= 0;

virtual void bossmapinfo(struct map_session_data *sd, struct mob_data *md, enum e_bossmap_info flag)= 0;
virtual void cashshop_show(struct map_session_data *sd, struct npc_data *nd)= 0;

// ADOPTION= 0;
virtual void Adopt_reply(struct map_session_data *sd, int type)= 0;
virtual void Adopt_request(struct map_session_data *sd, struct map_session_data *src, int p_id)= 0;

// MERCENARIES= 0;
virtual void mercenary_info(struct map_session_data *sd)= 0;
virtual void mercenary_skillblock(struct map_session_data *sd)= 0;
virtual void mercenary_message(struct map_session_data* sd, int message)= 0;
virtual void mercenary_updatestatus(struct map_session_data *sd, int type)= 0;

// RENTAL SYSTEM= 0;
virtual void rental_time(int fd, unsigned short nameid, int seconds)= 0;
virtual void rental_expired(int fd, int index, unsigned short nameid)= 0;

// BOOK READING= 0;
virtual void readbook(int fd, int book_id, int page)= 0;

// Show Picker= 0;
virtual void party_show_picker(struct map_session_data * sd, struct item * item_data)= 0;

// Progress Bar [Inkfish]= 0;
virtual void progressbar(struct map_session_data * sd, unsigned long color, unsigned int second)= 0;
virtual void progressbar_abort(struct map_session_data * sd)= 0;
virtual void progressbar_npc(struct npc_data *nd, struct map_session_data* sd)= 0;

virtual void PartyBookingRegisterAck(struct map_session_data *sd, int flag)= 0;
virtual void PartyBookingDeleteAck(struct map_session_data* sd, int flag)= 0;
virtual void PartyBookingSearchAck(int fd, struct party_booking_ad_info** results, int count, bool more_result)= 0;
virtual void PartyBookingUpdateNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad)= 0;
virtual void PartyBookingDeleteNotify(struct map_session_data* sd, int index)= 0;
virtual void PartyBookingInsertNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad)= 0;

/* Bank System [Yommy/Hercules] */;
virtual void bank_deposit (struct map_session_data *sd, enum e_BANKING_DEPOSIT_ACK reason)= 0;
virtual void bank_withdraw (struct map_session_data *sd,enum e_BANKING_WITHDRAW_ACK reason)= 0;

virtual void showdigit(struct map_session_data* sd, unsigned char type, int value)= 0;

/// Buying Store System= 0;
virtual void buyingstore_open(struct map_session_data* sd)= 0;
virtual void buyingstore_open_failed(struct map_session_data* sd, unsigned short result, unsigned int weight)= 0;
virtual void buyingstore_myitemlist(struct map_session_data* sd)= 0;
virtual void buyingstore_entry(struct map_session_data* sd)= 0;
virtual void buyingstore_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd)= 0;
virtual void buyingstore_disappear_entry(struct map_session_data* sd)= 0;
virtual void buyingstore_disappear_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd)= 0;
virtual void buyingstore_itemlist(struct map_session_data* sd, struct map_session_data* pl_sd)= 0;
virtual void buyingstore_trade_failed_buyer(struct map_session_data* sd, short result)= 0;
virtual void buyingstore_update_item(struct map_session_data* sd, unsigned short nameid, unsigned short amount, uint32 char_id, int zeny)= 0;
virtual void buyingstore_delete_item(struct map_session_data* sd, short index, unsigned short amount, int price)= 0;
virtual void buyingstore_trade_failed_seller(struct map_session_data* sd, short result, unsigned short nameid)= 0;

/// Search Store System= 0;
virtual void search_store_info_ack(struct map_session_data* sd)= 0;
virtual void search_store_info_failed(struct map_session_data* sd, unsigned char reason)= 0;
virtual void open_search_store_info(struct map_session_data* sd)= 0;
virtual void search_store_info_click_ack(struct map_session_data* sd, short x, short y)= 0;

/// Cash Shop= 0;
virtual void cashshop_result( struct map_session_data* sd, unsigned short item_id, uint16 result )= 0;
virtual void cashshop_open( struct map_session_data* sd )= 0;
#ifdef VIP_ENABLE
virtual void display_pinfo(struct map_session_data *sd, int type)= 0;
#endif
/// Roulette= 0;
virtual void roulette_open(struct map_session_data* sd)= 0;

virtual int elementalconverter_list(struct map_session_data *sd)= 0;

virtual void millenniumshield(struct block_list *bl, short shields)= 0;

virtual int spellbook_list(struct map_session_data *sd)= 0;

virtual int magicdecoy_list(struct map_session_data *sd, uint16 skill_lv, short x, short y)= 0;

virtual int poison_list(struct map_session_data *sd, uint16 skill_lv)= 0;

virtual int autoshadowspell_list(struct map_session_data *sd)= 0;

virtual int skill_itemlistwindow( struct map_session_data *sd, uint16 skill_id, uint16 skill_lv )= 0;
virtual void elemental_info(struct map_session_data *sd)= 0;
virtual void elemental_updatestatus(struct map_session_data *sd, int type)= 0;

virtual void spiritcharm(struct map_session_data *sd)= 0;

virtual void snap( struct block_list *bl, short x, short y )= 0;
virtual void monster_hp_bar( struct mob_data* md, int fd )= 0;

// Clan System= 0;
virtual void clan_basicinfo( struct map_session_data *sd )= 0;
virtual void clan_message(struct clan *clan,const char *mes,int len)= 0;
virtual void clan_onlinecount( struct clan* clan )= 0;
virtual void clan_leave( struct map_session_data* sd )= 0;

// Bargain Tool= 0;
virtual void sale_start(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target)= 0;
virtual void sale_end(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target)= 0;
virtual void sale_amount(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target)= 0;
virtual void sale_open(struct map_session_data* sd)= 0;

virtual void channel_msg(struct Channel *channel, const char *msg, unsigned long color)= 0;

virtual void ranklist(struct map_session_data *sd, int16 rankingType)= 0;
virtual void update_rankingpoint(struct map_session_data *sd, int rankingtype, int point)= 0;

virtual void crimson_marker(struct map_session_data *sd, struct block_list *bl, bool remove)= 0;

virtual void showscript(struct block_list* bl, const char* message, enum send_target flag)= 0;
virtual void party_leaderchanged(struct map_session_data *sd, int prev_leader_aid, int new_leader_aid)= 0;

virtual void account_name(int fd, uint32 account_id, const char* accname)= 0;
virtual void notify_bindOnEquip(struct map_session_data *sd, int n)= 0;

virtual void merge_item_open(struct map_session_data *sd)= 0;

virtual void broadcast_obtain_special_item(const char *char_name, unsigned short nameid, unsigned short container, enum BROADCASTING_SPECIAL_ITEM_OBTAIN type)= 0;

virtual void dressing_room(struct map_session_data *sd, int flag)= 0;
virtual void navigateTo(struct map_session_data *sd, const char* mapname, uint16 x, uint16 y, uint8 flag, bool hideWindow, uint16 mob_id )= 0;
virtual void SelectCart(struct map_session_data *sd)= 0;

/// Achievement System= 0;
virtual void achievement_list_all(struct map_session_data *sd)= 0;
virtual void achievement_update(struct map_session_data *sd, struct achievement *ach, int count)= 0;
virtual void achievement_reward_ack(int fd, unsigned char result, int ach_id)= 0;

virtual void ui_open( struct map_session_data *sd, enum out_ui_type ui_type, int32 data )= 0;
virtual void attendence_response( struct map_session_data *sd, int32 data )= 0;

virtual void weight_limit( struct map_session_data* sd )= 0;

virtual void ccamerainfo( struct map_session_data* sd, bool show, float range = 0.0f, float rotation = 0.0f, float latitude = 0.0f )= 0;

/// Equip Switch System= 0;
virtual void equipswitch_list( struct map_session_data* sd )= 0;
virtual void equipswitch_add( struct map_session_data* sd,uint16 index, uint32 pos, bool failed )= 0;
virtual void equipswitch_remove( struct map_session_data* sd, uint16 index, uint32 pos, bool failed )= 0;
virtual void equipswitch_reply( struct map_session_data* sd, bool failed )= 0;
};

/* following functions are currently not in class (not mocked) because they are used in macros */
void do_init_clif(void);
void do_final_clif(void);
int clif_insight(struct block_list *bl,va_list ap); //map_forallinmovearea callback
int clif_outsight(struct block_list *bl,va_list ap); //map_forallinmovearea callback
int clif_friendslist_toggle_sub(struct map_session_data *sd,va_list ap);
void parse_BankDeposit (int fd, struct map_session_data *sd);
void parse_BankWithdraw (int fd, struct map_session_data *sd);
void parse_BankCheck (int fd, struct map_session_data *sd);
void parse_BankOpen (int fd, struct map_session_data *sd);
void parse_BankClose (int fd, struct map_session_data *sd);
void clif_parse_NPCMarketClosed(int fd, struct map_session_data *sd);
void clif_parse_NPCMarketPurchase(int fd, struct map_session_data *sd);

void clif_guild_storage_log( struct map_session_data* sd, std::vector<struct guild_log_entry>& log, enum e_guild_storage_log result );
void clif_parse_roulette_open(int fd, struct map_session_data *sd);
void clif_parse_roulette_info(int fd, struct map_session_data *sd);
void clif_parse_roulette_close(int fd, struct map_session_data *sd);
void clif_parse_roulette_generate(int fd, struct map_session_data *sd);
void clif_parse_roulette_item(int fd, struct map_session_data *sd);
void clif_parse_Auction_cancelreg(int fd, struct map_session_data *sd);
void clif_friendslist_reqack(struct map_session_data *sd, struct map_session_data *f_sd, int type);
void clif_parse_LoadEndAck(int fd,struct map_session_data *sd);
void clif_send_petdata(struct map_session_data* sd, struct pet_data* pd, int type, int param);
void class_change_target(struct block_list *bl,int class_, int type, enum send_target target, struct map_session_data *sd);

#define class_change(bl, class_, type) class_change_target(bl, class_, type, AREA, NULL)
#define clif_mob_class_change(md, class_) class_change(&md->bl, class_, 1)
#define clif_menuskill_clear(sd) (sd)->menuskill_id = (sd)->menuskill_val = (sd)->menuskill_val2 = 0;
#define progressbar_npc_area(nd) progressbar_npc((nd),NULL)
#define disp_overhead(bl, mes) disp_overhead_(bl, mes, AREA)
#define clif_pet_equip(sd, pd) clif_send_petdata(sd, pd, 3, (pd)->vd.head_bottom)
#define clif_pet_equip_area(pd) clif_send_petdata(NULL, pd, 3, (pd)->vd.head_bottom)
#define name_self(bl) name( (bl), (bl), SELF )
#define name_area(bl) name( (bl), (bl), AREA )
#define clif_pet_performance(pd, param) clif_send_petdata(NULL, pd, 4, param)
#define messagecolor(bl, color, msg, rgb2bgr, type) messagecolor_target(bl, color, msg, rgb2bgr, type, NULL) // Mob/Npc color talk [SnakeDrak]
#define status_load(bl, type, flag) status_change((bl), (type), (flag), 0, 0, 0, 0)

#endif /* CLIF_INTERFACE_HPP */
