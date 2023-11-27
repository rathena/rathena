// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PACKETS_HPP
#define PACKETS_HPP

#pragma warning( push )
#pragma warning( disable : 4200 )

// Required for MESSAGE_SIZE, TALKBOX_MESSAGE_SIZE
#include "map.hpp"

#define MAX_ITEM_OPTIONS MAX_ITEM_RDM_OPT
#define UNAVAILABLE_STRUCT int8 _____unavailable
/* packet size constant for itemlist */
#if MAX_INVENTORY > MAX_STORAGE && MAX_INVENTORY > MAX_CART
	#define MAX_ITEMLIST MAX_INVENTORY
#elif MAX_CART > MAX_INVENTORY && MAX_CART > MAX_STORAGE
	#define MAX_ITEMLIST MAX_CART
#else
	#define MAX_ITEMLIST MAX_STORAGE
#endif
#define MAX_ACHIEVEMENT_DB MAX_ACHIEVEMENT_OBJECTIVES

#define DEFINE_PACKET_HEADER(name, id) const int16 HEADER_##name = id;
#define DEFINE_PACKET_ID(name, id) DEFINE_PACKET_HEADER(name, id)

#include "packets_struct.hpp"

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( push, 1 )
#endif

struct PACKET_ZC_PC_PURCHASE_RESULT{
	int16 packetType;
	uint8 result;
} __attribute__((packed));

struct PACKET_CZ_REQ_MAKINGARROW{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_SE_PC_BUY_CASHITEM_LIST_sub{
	uint32 itemId;
	uint32 amount;
	uint16 tab;
} __attribute__((packed));

struct PACKET_CZ_SE_PC_BUY_CASHITEM_LIST{
	int16 packetType;
	int16 packetLength;
	uint16 count;
	uint32 kafraPoints;
	struct PACKET_CZ_SE_PC_BUY_CASHITEM_LIST_sub items[];
} __attribute__((packed));

struct PACKET_CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO{
	int16 packetType;
	int16 packetLength;
	uint32 AID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO{
	int16 packetType;
	uint16 result;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 price;
} __attribute__((packed));

struct PACKET_CZ_REQ_APPLY_BARGAIN_SALE_ITEM{
	int16 packetType;
	uint32 AID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 amount;
	uint32 startTime;
#if PACKETVER >= 20150520
	uint16 hours;
#else
	uint8 hours;
#endif
} __attribute__((packed));

struct PACKET_CZ_REQ_REMOVE_BARGAIN_SALE_ITEM{
	int16 packetType;
	uint32 AID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_BARGAIN_SALE_SELLING{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 remainingTime;
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_BARGAIN_SALE_CLOSE{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_COUNT_BARGAIN_SALE_ITEM{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 amount;
} __attribute__((packed));

struct PACKET_ZC_ACK_GUILDSTORAGE_LOG_sub{
	uint32 id;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int32 amount;
	uint8 action;
	int32 refine;
	int64 uniqueId;
	uint8 IsIdentified;
	uint16 itemType;
	struct EQUIPSLOTINFO slot;
	char name[NAME_LENGTH];
	char time[NAME_LENGTH];
	uint8 attribute;
} __attribute__((packed));

struct PACKET_ZC_ACK_GUILDSTORAGE_LOG{
	int16 packetType;
	int16 PacketLength;
	uint16 result;
	uint16 amount;
	struct PACKET_ZC_ACK_GUILDSTORAGE_LOG_sub items[];
} __attribute__((packed));

struct PACKET_CZ_UNCONFIRMED_TSTATUS_UP{
	int16 packetType;
	int16 type;
	int16 amount;
} __attribute__((packed));

struct PACKET_CZ_GUILD_EMBLEM_CHANGE2 {
	int16 packetType;
	uint32 guild_id;
	uint32 version;
} __attribute__((packed));

struct PACKET_ZC_CHANGE_GUILD {
	int16 packetType;
#if PACKETVER < 20190724
	uint32 aid;
	uint32 guild_id;
	uint16 emblem_id;
#else
	uint32 guild_id;
	uint32 emblem_id;
	uint32 unknown;
#endif
} __attribute__((packed));

struct PACKET_ZC_BROADCAST{
	int16 packetType;
	int16 PacketLength;
	char message[];
} __attribute__((packed));

struct PACKET_ZC_BROADCAST2{
	int16 packetType;
	int16 PacketLength;
	uint32 fontColor;
	int16 fontType;
	int16 fontSize;
	int16 fontAlign;
	int16 fontY;
	char message[];
} __attribute__((packed));

struct PACKET_ZC_SOULENERGY{
	int16 PacketType;
	uint32 AID;
	uint16 num;
} __attribute__((packed));

struct PACKET_ZC_ENTRY_QUEUE_INIT {
	int16 packetType;
} __attribute__((packed));

struct PACKET_CZ_UNCONFIRMED_RODEX_RETURN{
	int16 packetType;
	uint32 msgId;
} __attribute__((packed));

struct PACKET_CZ_REQ_STYLE_CLOSE{
	int16 packetType;
} __attribute__((packed));

struct PACKET_ZC_SUMMON_HP_INIT {
	int16 PacketType;
	uint32 summonAID;
	uint32 CurrentHP;
	uint32 MaxHP;
} __attribute__((packed));

struct PACKET_ZC_SUMMON_HP_UPDATE {
	int16 PacketType;
	uint32 summonAID;
	uint16 VarId;
	uint32 Value;
} __attribute__((packed));

struct PACKET_ZC_REPUTE_INFO_sub{
	uint64 type;
	int64 points;
} __attribute__((packed));

struct PACKET_ZC_REPUTE_INFO{
	int16 packetType;
	int16 packetLength;
	uint8 success;
	struct PACKET_ZC_REPUTE_INFO_sub list[];
} __attribute__((packed));

struct PACKET_ZC_OPEN_REFORM_UI{
	int16 packetType;
	uint32 itemId;
} __attribute__((packed));

struct PACKET_CZ_CLOSE_REFORM_UI{
	int16 packetType;
} __attribute__((packed));

struct PACKET_CZ_ITEM_REFORM{
	int16 packetType;
	uint32 itemId;
	uint16 index;
} __attribute__((packed));

struct PACKET_ZC_ITEM_REFORM_ACK{
	int16 packetType;
	uint16 index;
	uint8 result;
} __attribute__((packed));

struct PACKET_ZC_UI_OPEN_V3{
	int16 packetType;
	uint8 type;
	uint64 data;
} __attribute__((packed));

struct PACKET_CZ_REQUEST_RANDOM_ENCHANT{
	int16 packetType;
	uint64 clientLuaIndex;
	uint16 index;
} __attribute__((packed));

struct PACKET_CZ_REQUEST_PERFECT_ENCHANT{
	int16 packetType;
	uint64 clientLuaIndex;
	uint16 index;
	uint32 itemId;
} __attribute__((packed));

struct PACKET_CZ_REQUEST_UPGRADE_ENCHANT{
	int16 packetType;
	uint64 clientLuaIndex;
	uint16 index;
	uint16 slot;
} __attribute__((packed));

struct PACKET_CZ_REQUEST_RESET_ENCHANT{
	int16 packetType;
	uint64 clientLuaIndex;
	uint16 index;
} __attribute__((packed));

struct PACKET_ZC_RESPONSE_ENCHANT{
	int16 packetType;
	uint32 messageId;
	uint32 enchantItemId;
} __attribute__((packed));

struct PACKET_CZ_CLOSE_UI_ENCHANT{
	int16 packetType;
} __attribute__((packed));

struct PACKET_ZC_TARGET_SPIRITS {
	int16 packetType;
	uint32 GID;
	uint32 unknown_val;
	uint16 amount;
} __attribute__((packed));

struct PACKET_CZ_USE_PACKAGEITEM{
	int16 PacketType;
	uint16 index;
	uint32 AID;
	uint32 itemID;
	uint32 BoxIndex;
} __attribute__((packed));

struct PACKET_ZC_FRIENDS_LIST_sub{
	uint32 AID;
	uint32 CID;
#if !( PACKETVER_MAIN_NUM >= 20180307 || PACKETVER_RE_NUM >= 20180221 || PACKETVER_ZERO_NUM >= 20180328 ) || PACKETVER >= 20200902
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct PACKET_ZC_FRIENDS_LIST{
	int16 packetType;
	int16 PacketLength;
	struct PACKET_ZC_FRIENDS_LIST_sub friends[];
} __attribute__((packed));

struct PACKET_CZ_PC_SELL_ITEMLIST_sub {
	uint16 index;
	uint16 amount;
} __attribute__((packed));

struct PACKET_CZ_PC_SELL_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	PACKET_CZ_PC_SELL_ITEMLIST_sub sellList[];
} __attribute__((packed));

struct PACKET_CZ_REQ_CHANGE_MEMBERPOS_sub{
	uint32 AID;
	uint32 CID;
	int32 position;
} __attribute__((packed));

struct PACKET_CZ_REQ_CHANGE_MEMBERPOS{
	int16 packetType;
	int16 packetLength;
	struct PACKET_CZ_REQ_CHANGE_MEMBERPOS_sub list[];
} __attribute__((packed));

struct PACKET_ZC_CLEAR_DIALOG{
	int16 packetType;
	uint32 GID;
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_BIND_ON_EQUIP{
	int16 packetType;
	int16 index;
} __attribute__((packed));

struct PACKET_ZC_BANKING_CHECK{
	int16 packetType;
	int64 money;
	int16 reason;
} __attribute__((packed));

struct PACKET_ZC_ACK_BANKING_WITHDRAW{
	int16 packetType;
	int16 reason;
	int64 money;
	int32 zeny;
} __attribute__((packed));

struct PACKET_ZC_ACK_BANKING_DEPOSIT{
	int16 packetType;
	int16 reason;
	int64 money;
	int32 zeny;
} __attribute__((packed));

struct PACKET_ZC_ACK_CLOSE_BANKING{
	int16 packetType;
	int16 unknown;
} __attribute__((packed));

struct PACKET_ZC_ACK_OPEN_BANKING{
	int16 packetType;
	int16 unknown;
} __attribute__((packed));

struct PACKET_ZC_ACK_ADD_EXCHANGE_ITEM {
	int16 packetType;
	uint16 index;
	uint8 result;
} __attribute__((packed));

struct PACKET_ZC_COUPLENAME {
	int16 packetType;
	char name[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_DYNAMICNPC_CREATE_RESULT{
	int16 packetType;
	int32 result;
} __attribute__((packed));

struct PACKET_CZ_PARTY_REQ_MASTER_TO_JOIN{
	int16 packetType;
	uint32 CID;
	uint32 AID;
} __attribute__((packed));

struct PACKET_ZC_PARTY_REQ_MASTER_TO_JOIN{
	int16 packetType;
	uint32 CID;
	uint32 AID;
	char name[NAME_LENGTH];
	uint16 x;
	uint16 y;
} __attribute__((packed));

struct PACKET_CZ_PARTY_REQ_ACK_MASTER_TO_JOIN{
	int16 packetType;
	uint32 CID;
	uint32 AID;
	uint8 accept;
} __attribute__((packed));

struct PACKET_ZC_PARTY_JOIN_REQ_ACK_FROM_MASTER{
	int16 packetType;
	char player_name[NAME_LENGTH];
	char party_name[NAME_LENGTH];
	uint32 AID;
	uint32 refused;
} __attribute__((packed));

struct PACKET_CZ_REQ_SE_CASH_TAB_CODE{
	int16 packetType;
	int16 tab;
} __attribute__((packed));

struct PACKET_ZC_ACK_SE_CASH_ITEM_LIST2_sub{
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int32 price;
} __attribute__((packed));

struct PACKET_ZC_ACK_SE_CASH_ITEM_LIST2{
	int16 packetType;
	int16 packetLength;
	uint32 tab;
	int16 count;
	struct PACKET_ZC_ACK_SE_CASH_ITEM_LIST2_sub items[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20140508 || PACKETVER_RE_NUM >= 20140508 || defined(PACKETVER_ZERO)
struct PACKET_ZC_GOLDPCCAFE_POINT{
	int16 packetType;
	int8 active;
	int8 unitPoint;
	int32 point;
	int32 accumulatePlaySecond;
} __attribute__((packed));
#elif PACKETVER_MAIN_NUM >= 20140430 || PACKETVER_RE_NUM >= 20140430
	// TODO: find difference (1byte) priority low...
#endif

struct PACKET_CZ_DYNAMICNPC_CREATE_REQUEST{
	int16 packetType;
	char nickname[NAME_LENGTH];
} __attribute__((packed));

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( pop )
#endif

DEFINE_PACKET_HEADER(ZC_NOTIFY_CHAT, 0x8d)
DEFINE_PACKET_HEADER(ZC_BROADCAST, 0x9a)
DEFINE_PACKET_HEADER(ZC_ITEM_ENTRY, 0x9d)
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_RESULT, 0xca)
DEFINE_PACKET_HEADER(ZC_ACK_ADD_EXCHANGE_ITEM, 0xea)
DEFINE_PACKET_HEADER(ZC_MVP_GETTING_ITEM, 0x10a)
DEFINE_PACKET_HEADER(CZ_REQ_CHANGE_MEMBERPOS, 0x155)
DEFINE_PACKET_HEADER(CZ_REQMAKINGITEM, 0x18e)
DEFINE_PACKET_HEADER(ZC_ACK_REQMAKINGITEM, 0x18f)
DEFINE_PACKET_HEADER(CZ_REQ_MAKINGARROW, 0x1ae)
DEFINE_PACKET_HEADER(ZC_BROADCAST2, 0x1c3)
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	DEFINE_PACKET_HEADER(CZ_REQ_ITEMREPAIR, 0xb66)
#else
	DEFINE_PACKET_HEADER(CZ_REQ_ITEMREPAIR, 0x1fd)
#endif
#if PACKETVER >= 20190724
	DEFINE_PACKET_HEADER(ZC_CHANGE_GUILD, 0x0b47)
#else
	DEFINE_PACKET_HEADER(ZC_CHANGE_GUILD, 0x1b4)
#endif
DEFINE_PACKET_HEADER(ZC_COUPLENAME, 0x1e6);
DEFINE_PACKET_HEADER(ZC_FRIENDS_LIST, 0x201)
DEFINE_PACKET_HEADER(ZC_NOTIFY_WEAPONITEMLIST, 0x221)
DEFINE_PACKET_HEADER(ZC_ACK_WEAPONREFINE, 0x223)
DEFINE_PACKET_HEADER(CZ_REQ_MAKINGITEM, 0x25b)
DEFINE_PACKET_HEADER(ZC_PC_CASH_POINT_ITEMLIST, 0x287)
DEFINE_PACKET_HEADER(ZC_CASH_TIME_COUNTER, 0x298)
DEFINE_PACKET_HEADER(ZC_CASH_ITEM_DELETE, 0x299)
DEFINE_PACKET_HEADER(ZC_NOTIFY_BIND_ON_EQUIP, 0x2d3)
DEFINE_PACKET_HEADER(ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER, 0x824)
DEFINE_PACKET_HEADER(CZ_SSILIST_ITEM_CLICK, 0x83c)
DEFINE_PACKET_HEADER(CZ_REQ_SE_CASH_TAB_CODE, 0x846)
DEFINE_PACKET_HEADER(ZC_ACK_SE_CASH_ITEM_LIST2, 0x8c0)
DEFINE_PACKET_HEADER(ZC_ACK_SCHEDULER_CASHITEM, 0x8ca)
DEFINE_PACKET_HEADER(ZC_CLEAR_DIALOG, 0x8d6)
DEFINE_PACKET_HEADER(ZC_ENTRY_QUEUE_INIT, 0x90e);
DEFINE_PACKET_HEADER(ZC_BANKING_CHECK, 0x9a6)
DEFINE_PACKET_HEADER(ZC_ACK_BANKING_DEPOSIT, 0x9a8)
DEFINE_PACKET_HEADER(ZC_ACK_BANKING_WITHDRAW, 0x9aa)
DEFINE_PACKET_HEADER(CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO, 0x9ac)
DEFINE_PACKET_HEADER(ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO, 0x9ad)
DEFINE_PACKET_HEADER(CZ_REQ_APPLY_BARGAIN_SALE_ITEM, 0x9ae)
DEFINE_PACKET_HEADER(CZ_REQ_REMOVE_BARGAIN_SALE_ITEM, 0x9b0)
DEFINE_PACKET_HEADER(ZC_NOTIFY_BARGAIN_SALE_SELLING, 0x9b2)
DEFINE_PACKET_HEADER(ZC_NOTIFY_BARGAIN_SALE_CLOSE, 0x9b3)
DEFINE_PACKET_HEADER(ZC_ACK_OPEN_BANKING, 0x9b7)
DEFINE_PACKET_HEADER(ZC_ACK_CLOSE_BANKING, 0x9b9)
DEFINE_PACKET_HEADER(ZC_ACK_COUNT_BARGAIN_SALE_ITEM, 0x9c4)
DEFINE_PACKET_HEADER(ZC_ACK_GUILDSTORAGE_LOG, 0x9da)
DEFINE_PACKET_HEADER(ZC_GOLDPCCAFE_POINT, 0xa15)
DEFINE_PACKET_HEADER(CZ_DYNAMICNPC_CREATE_REQUEST, 0xa16)
DEFINE_PACKET_HEADER(ZC_DYNAMICNPC_CREATE_RESULT, 0xa17)
DEFINE_PACKET_HEADER(CZ_REQ_APPLY_BARGAIN_SALE_ITEM2, 0xa3d)
DEFINE_PACKET_HEADER(CZ_REQ_STYLE_CHANGE, 0xa46)
DEFINE_PACKET_HEADER(ZC_STYLE_CHANGE_RES, 0xa47)
DEFINE_PACKET_HEADER(CZ_REQ_STYLE_CLOSE, 0xa48)
DEFINE_PACKET_HEADER(ZC_GROUP_ISALIVE, 0xab2)
DEFINE_PACKET_HEADER(CZ_PARTY_REQ_MASTER_TO_JOIN, 0x0ae6)
DEFINE_PACKET_HEADER(ZC_PARTY_REQ_MASTER_TO_JOIN, 0x0ae7)
DEFINE_PACKET_HEADER(CZ_PARTY_REQ_ACK_MASTER_TO_JOIN, 0x0af8)
DEFINE_PACKET_HEADER(ZC_PARTY_JOIN_REQ_ACK_FROM_MASTER, 0x0afa)
DEFINE_PACKET_HEADER(CZ_REQ_STYLE_CHANGE2, 0xafc)
DEFINE_PACKET_HEADER(ZC_REMOVE_EFFECT, 0x0b0d)
DEFINE_PACKET_HEADER(CZ_UNCONFIRMED_TSTATUS_UP, 0x0b24)
DEFINE_PACKET_HEADER(CZ_GUILD_EMBLEM_CHANGE2, 0x0b46)
DEFINE_PACKET_HEADER(ZC_TARGET_SPIRITS, 0xb68)
DEFINE_PACKET_HEADER(ZC_SOULENERGY, 0xb73)
DEFINE_PACKET_HEADER(CZ_UNCONFIRMED_RODEX_RETURN, 0xb98)
DEFINE_PACKET_HEADER(ZC_SUMMON_HP_INIT, 0xb6b)
DEFINE_PACKET_HEADER(ZC_SUMMON_HP_UPDATE, 0xb6c)
DEFINE_PACKET_HEADER(ZC_REPUTE_INFO, 0x0b8d)
DEFINE_PACKET_HEADER(ZC_OPEN_REFORM_UI, 0x0b8f)
DEFINE_PACKET_HEADER(CZ_CLOSE_REFORM_UI, 0x0b90)
DEFINE_PACKET_HEADER(CZ_ITEM_REFORM, 0x0b91)
DEFINE_PACKET_HEADER(ZC_ITEM_REFORM_ACK, 0x0b92)
DEFINE_PACKET_HEADER(ZC_UI_OPEN_V3, 0x0b9a)
DEFINE_PACKET_HEADER(CZ_REQUEST_RANDOM_ENCHANT, 0x0b9b)
DEFINE_PACKET_HEADER(CZ_REQUEST_PERFECT_ENCHANT, 0x0b9c)
DEFINE_PACKET_HEADER(CZ_REQUEST_UPGRADE_ENCHANT, 0x0b9d)
DEFINE_PACKET_HEADER(CZ_REQUEST_RESET_ENCHANT, 0x0b9e)
DEFINE_PACKET_HEADER(ZC_RESPONSE_ENCHANT, 0x0b9f)
DEFINE_PACKET_HEADER(CZ_CLOSE_UI_ENCHANT, 0x0ba0)
DEFINE_PACKET_HEADER(CZ_USE_PACKAGEITEM, 0x0baf)
DEFINE_PACKET_HEADER(CZ_PC_SELL_ITEMLIST, 0x00c9)

const int16 MAX_INVENTORY_ITEM_PACKET_NORMAL = ( ( INT16_MAX - ( sizeof( struct packet_itemlist_normal ) - ( sizeof( struct NORMALITEM_INFO ) * MAX_ITEMLIST) ) ) / sizeof( struct NORMALITEM_INFO ) );
const int16 MAX_INVENTORY_ITEM_PACKET_EQUIP = ( ( INT16_MAX - ( sizeof( struct packet_itemlist_equip ) - ( sizeof( struct EQUIPITEM_INFO ) * MAX_ITEMLIST ) ) ) / sizeof( struct EQUIPITEM_INFO ) );

const int16 MAX_STORAGE_ITEM_PACKET_NORMAL = ( ( INT16_MAX - ( sizeof( struct ZC_STORE_ITEMLIST_NORMAL ) - ( sizeof( struct NORMALITEM_INFO ) * MAX_ITEMLIST) ) ) / sizeof( struct NORMALITEM_INFO ) );
const int16 MAX_STORAGE_ITEM_PACKET_EQUIP = ( ( INT16_MAX - ( sizeof( struct ZC_STORE_ITEMLIST_EQUIP ) - ( sizeof( struct EQUIPITEM_INFO ) * MAX_ITEMLIST ) ) ) / sizeof( struct EQUIPITEM_INFO ) );

const int16 MAX_GUILD_STORAGE_LOG_PACKET = ( ( INT16_MAX - sizeof( struct PACKET_ZC_ACK_GUILDSTORAGE_LOG ) ) / sizeof( struct PACKET_ZC_ACK_GUILDSTORAGE_LOG_sub ) );

#undef MAX_ITEM_OPTIONS
#undef UNAVAILABLE_STRUCT
#undef MAX_ITEMLIST
#undef MAX_ACHIEVEMENT_DB
#undef MAX_PACKET_POS
#undef DEFINE_PACKET_HEADER

#pragma warning( pop )

#endif /* PACKETS_HPP */
