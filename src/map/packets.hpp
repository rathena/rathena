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

struct PACKET_ZC_USER_COUNT{
	uint16 packetType;
	int32 playersCount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_USER_COUNT, 0xc2)

struct PACKET_ZC_PC_PURCHASE_RESULT{
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_RESULT, 0xca)

struct PACKET_CZ_REQ_MAKINGARROW{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_MAKINGARROW, 0x1ae)

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
DEFINE_PACKET_HEADER(CZ_SE_PC_BUY_CASHITEM_LIST, 0x848)

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
DEFINE_PACKET_HEADER(CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO, 0x9ac)

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
DEFINE_PACKET_HEADER(ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO, 0x9ad)

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
DEFINE_PACKET_HEADER(CZ_REQ_APPLY_BARGAIN_SALE_ITEM, 0x9ae)

struct PACKET_CZ_REQ_REMOVE_BARGAIN_SALE_ITEM{
	int16 packetType;
	uint32 AID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_REMOVE_BARGAIN_SALE_ITEM, 0x9b0)

struct PACKET_ZC_NOTIFY_BARGAIN_SALE_SELLING{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 remainingTime;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_BARGAIN_SALE_SELLING, 0x9b2)

struct PACKET_ZC_NOTIFY_BARGAIN_SALE_CLOSE{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_BARGAIN_SALE_CLOSE, 0x9b3)

struct PACKET_ZC_ACK_COUNT_BARGAIN_SALE_ITEM{
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_COUNT_BARGAIN_SALE_ITEM, 0x9c4)

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
DEFINE_PACKET_HEADER(ZC_ACK_GUILDSTORAGE_LOG, 0x9da)

struct PACKET_CZ_ADVANCED_STATUS_CHANGE{
	int16 packetType;
	int16 type;
	int16 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ADVANCED_STATUS_CHANGE, 0x0b24)

struct PACKET_CZ_REQ_ADD_NEW_EMBLEM {
	int16 packetType;
	uint32 guild_id;
	uint32 version;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_ADD_NEW_EMBLEM, 0x0b46)

struct PACKET_ZC_BROADCAST{
	int16 packetType;
	int16 PacketLength;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BROADCAST, 0x9a)

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
DEFINE_PACKET_HEADER(ZC_BROADCAST2, 0x1c3)

struct PACKET_ZC_ENTRY_QUEUE_INIT {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ENTRY_QUEUE_INIT, 0x90e)

struct PACKET_CZ_RODEX_RETURN{
	int16 packetType;
	uint32 msgId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_RODEX_RETURN, 0xb98)

struct PACKET_CZ_REQ_STYLE_CLOSE{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_STYLE_CLOSE, 0xa48)

struct PACKET_ZC_SUMMON_HP_INIT {
	int16 PacketType;
	uint32 summonAID;
	uint32 CurrentHP;
	uint32 MaxHP;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SUMMON_HP_INIT, 0xb6b)

struct PACKET_ZC_SUMMON_HP_UPDATE {
	int16 PacketType;
	uint32 summonAID;
	uint16 VarId;
	uint32 Value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SUMMON_HP_UPDATE, 0xb6c)

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
DEFINE_PACKET_HEADER(ZC_REPUTE_INFO, 0x0b8d)

struct PACKET_ZC_UI_OPEN_V3{
	int16 packetType;
	uint8 type;
	uint64 data;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UI_OPEN_V3, 0x0b9a)

struct PACKET_ZC_TARGET_SPIRITS {
	int16 packetType;
	uint32 GID;
	uint32 unknown_val;
	uint16 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_TARGET_SPIRITS, 0xb68)

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
DEFINE_PACKET_HEADER(ZC_FRIENDS_LIST, 0x201)

struct PACKET_CZ_PC_SELL_ITEMLIST_sub {
	uint16 index;
	uint16 amount;
} __attribute__((packed));

struct PACKET_CZ_PC_SELL_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	PACKET_CZ_PC_SELL_ITEMLIST_sub sellList[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PC_SELL_ITEMLIST, 0x00c9)

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
DEFINE_PACKET_HEADER(CZ_REQ_CHANGE_MEMBERPOS, 0x155)

struct PACKET_ZC_CLEAR_DIALOG{
	int16 packetType;
	uint32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CLEAR_DIALOG, 0x8d6)

struct PACKET_ZC_NOTIFY_BIND_ON_EQUIP{
	int16 packetType;
	int16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_BIND_ON_EQUIP, 0x2d3)

struct PACKET_ZC_BANKING_CHECK{
	int16 packetType;
	int64 money;
	int16 reason;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BANKING_CHECK, 0x9a6)

struct PACKET_ZC_ACK_BANKING_WITHDRAW{
	int16 packetType;
	int16 reason;
	int64 money;
	int32 zeny;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_BANKING_WITHDRAW, 0x9aa)

struct PACKET_ZC_ACK_BANKING_DEPOSIT{
	int16 packetType;
	int16 reason;
	int64 money;
	int32 zeny;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_BANKING_DEPOSIT, 0x9a8)

struct PACKET_ZC_ACK_CLOSE_BANKING{
	int16 packetType;
	int16 unknown;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_CLOSE_BANKING, 0x9b9)

struct PACKET_ZC_ACK_OPEN_BANKING{
	int16 packetType;
	int16 unknown;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_OPEN_BANKING, 0x9b7)

#if PACKETVER > 6
struct PACKET_ZC_REQ_EXCHANGE_ITEM{
	uint16 packetType;
	char requesterName[NAME_LENGTH];
	uint32 targetId;
	uint16 targetLv;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_EXCHANGE_ITEM, 0x1f4)
#else
struct PACKET_ZC_REQ_EXCHANGE_ITEM{
	uint16 packetType;
	char requesterName[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_EXCHANGE_ITEM, 0x9a)
#endif

#if PACKETVER > 6
struct PACKET_ZC_ACK_EXCHANGE_ITEM{
	uint16 packetType;
	uint8 result;
	uint32 targetId;
	uint16 targetLv;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_EXCHANGE_ITEM, 0x1f5)
#else
struct PACKET_ZC_ACK_EXCHANGE_ITEM{
	uint16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_EXCHANGE_ITEM, 0xe7)
#endif

struct PACKET_ZC_ACK_ADD_EXCHANGE_ITEM {
	int16 packetType;
	uint16 index;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ADD_EXCHANGE_ITEM, 0xea)

struct PACKET_ZC_COUPLENAME {
	int16 packetType;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_COUPLENAME, 0x1e6);

struct PACKET_CZ_PARTY_REQ_MASTER_TO_JOIN{
	int16 packetType;
	uint32 CID;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PARTY_REQ_MASTER_TO_JOIN, 0x0ae6)

struct PACKET_ZC_PARTY_REQ_MASTER_TO_JOIN{
	int16 packetType;
	uint32 CID;
	uint32 AID;
	char name[NAME_LENGTH];
	uint16 x;
	uint16 y;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PARTY_REQ_MASTER_TO_JOIN, 0x0ae7)

struct PACKET_CZ_PARTY_REQ_ACK_MASTER_TO_JOIN{
	int16 packetType;
	uint32 CID;
	uint32 AID;
	uint8 accept;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PARTY_REQ_ACK_MASTER_TO_JOIN, 0x0af8)

struct PACKET_ZC_PARTY_JOIN_REQ_ACK_FROM_MASTER{
	int16 packetType;
	char player_name[NAME_LENGTH];
	char party_name[NAME_LENGTH];
	uint32 AID;
	uint32 refused;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PARTY_JOIN_REQ_ACK_FROM_MASTER, 0x0afa)

struct PACKET_CZ_REQ_SE_CASH_TAB_CODE{
	int16 packetType;
	int16 tab;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_SE_CASH_TAB_CODE, 0x846)

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
DEFINE_PACKET_HEADER(ZC_ACK_SE_CASH_ITEM_LIST2, 0x8c0)

struct PACKET_CZ_REQ_MERGE_ITEM{
	int16 packetType;
	int16 packetLength;
	uint16 indices[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_MERGE_ITEM, 0x96e)

struct PACKET_CZ_RESET_SKILL{
	int16 packetType;
	uint8 unknown;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_RESET_SKILL, 0x0bb1)

struct PACKET_ZC_SKILLINFO_UPDATE{
	uint16 packetType;
	uint16 skillId;
	uint16 level;
	uint16 sp;
	uint16 range2;
	bool upFlag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILLINFO_UPDATE, 0x10e);

struct PACKET_ZC_BOSS_INFO{
	int16 packetType;
	uint8 type;
	uint32 x;
	uint32 y;
	uint16 minHours;
	uint16 minMinutes;
	uint16 maxHours;
	uint16 maxMinutes;
	char name[51];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOSS_INFO, 0x293)

struct PACKET_CZ_INVENTORY_TAB{
	int16 packetType;
	int16 index;
	bool favorite;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_INVENTORY_TAB, 0x907)

struct PACKET_ZC_INVENTORY_TAB{
	int16 packetType;
	int16 index;
	bool favorite;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_INVENTORY_TAB, 0x908)

struct PACKET_CZ_REQ_OPEN_BANKING{
	int16 packetType;
	uint32 AID;
} __attribute__((packed));

struct PACKET_CZ_REQ_CLOSE_BANKING{
	int16 packetType;
	uint32 AID;
} __attribute__((packed));

struct PACKET_CZ_REQ_BANKING_CHECK{
	int16 packetType;
	uint32 AID;
} __attribute__((packed));

struct PACKET_CZ_REQ_BANKING_DEPOSIT{
	int16 packetType;
	uint32 AID;
	int32 zeny;
} __attribute__((packed));

struct PACKET_CZ_REQ_BANKING_WITHDRAW{
	int16 packetType;
	uint32 AID;
	int32 zeny;
} __attribute__((packed));

#if PACKETVER < 20080102
struct PACKET_ZC_ACCEPT_ENTER {
	int16 packetType;
	uint32 startTime;
	uint8 posDir[3];
	uint8 xSize;
	uint8 ySize;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACCEPT_ENTER, 0x73)
#elif PACKETVER < 20141022 || PACKETVER >= 20160330
struct PACKET_ZC_ACCEPT_ENTER {
	int16 packetType;
	uint32 startTime;
	uint8 posDir[3];
	uint8 xSize;
	uint8 ySize;
	uint16 font;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACCEPT_ENTER, 0x2eb)
#else
struct PACKET_ZC_ACCEPT_ENTER {
	int16 packetType;
	uint32 startTime;
	uint8 posDir[3];
	uint8 xSize;
	uint8 ySize;
	uint16 font;
	uint8 sex;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACCEPT_ENTER, 0xa18)
#endif

struct PACKET_ZC_REFUSE_ENTER {
	int16 packetType;
	uint8 errorCode;
} __attribute__((packed));
static_assert(sizeof(PACKET_ZC_REFUSE_ENTER) == 3);
DEFINE_PACKET_HEADER(ZC_REFUSE_ENTER, 0x74)

struct PACKET_SC_NOTIFY_BAN {
	int16 packetType;
	uint8 errorCode;
} __attribute__((packed));
DEFINE_PACKET_HEADER(SC_NOTIFY_BAN, 0x81)

struct PACKET_ZC_RESTART_ACK {
	int16 packetType;
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RESTART_ACK, 0xb3)

struct PACKET_ZC_NOTIFY_VANISH {
	int16 packetType;
	uint32 gid;
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_VANISH, 0x80)

struct PACKET_ZC_ITEM_DISAPPEAR {
	int16 packetType;
	uint32 itemAid;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ITEM_DISAPPEAR, 0xa1)

struct PACKET_ZC_MILLENNIUMSHIELD {
	int16 packetType;
	uint32 aid;
	int16 num;
	int16 state;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MILLENNIUMSHIELD, 0x440)

struct PACKET_ZC_SPIRITS_ATTRIBUTE {
	int16 packetType;
	uint32 aid;
	int16 spiritsType;
	int16 num;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SPIRITS_ATTRIBUTE, 0x8cf)

struct PACKET_ZC_CHANGESTATE_MER {
	int16 packetType;
	uint8 type;
	uint8 state;
	uint32 gid;
	uint32 data;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGESTATE_MER, 0x230)

struct PACKET_ZC_HOSKILLINFO_LIST_sub {
	uint16 id;
	uint16 inf;
	uint16 unknown;
	uint16 level;
	uint16 sp;
	uint16 range;
	char name[NAME_LENGTH];
	uint8 upgradable;
} __attribute__((packed));

struct PACKET_ZC_HOSKILLINFO_LIST {
	int16 packetType;
	int16 packetLength;
	PACKET_ZC_HOSKILLINFO_LIST_sub skills[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_HOSKILLINFO_LIST, 0x235)

struct PACKET_ZC_HOSKILLINFO_UPDATE {
	int16 packetType;
	uint16 skill_id;
	int16 Level;
	int16 SP;
	int16 AttackRange;
	bool upgradable;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_HOSKILLINFO_UPDATE, 0x239)

// Unused packet (alpha?)
/*
struct PACKET_ZC_NOTIFY_MOVE {
	int16 packetType;
	uint32 gid;
	uint8 moveData[6];
	uint32 moveStartTime;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_MOVE, 0x86)
*/

struct PACKET_ZC_NOTIFY_PLAYERMOVE {
	int16 packetType;
	uint32 moveStartTime;
	uint8 moveData[6];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_PLAYERMOVE, 0x87);

struct PACKET_ZC_CHANGE_DIRECTION{
	int16 packetType;
	uint32 srcId;
	uint16 headDir;
	uint8 dir;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_DIRECTION, 0x9c)

struct PACKET_ZC_NPCACK_MAPMOVE {
	int16 packetType;
	char mapName[MAP_NAME_LENGTH_EXT];
	uint16 xPos;
	uint16 yPos;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPCACK_MAPMOVE, 0x91)

#if PACKETVER >= 20170315
// Actually ZC_NPCACK_SERVERMOVE_DOMAIN
struct PACKET_ZC_NPCACK_SERVERMOVE {
	int16 packetType;
	char mapName[MAP_NAME_LENGTH_EXT];
	uint16 xPos;
	uint16 yPos;
	uint32 ip;
	uint16 port;
	char domain[128];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPCACK_SERVERMOVE, 0xac7)
#else
struct PACKET_ZC_NPCACK_SERVERMOVE {
	int16 packetType;
	char mapName[MAP_NAME_LENGTH_EXT];
	uint16 xPos;
	uint16 yPos;
	uint32 ip;
	uint16 port;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPCACK_SERVERMOVE, 0x92)
#endif

struct PACKET_ZC_STOPMOVE {
	int16 packetType;
	uint32	AID;
	uint16	xPos;
	uint16	yPos;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_STOPMOVE, 0x88)

struct PACKET_ZC_SELECT_DEALTYPE {
	int16 packetType;
	uint32 npcId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SELECT_DEALTYPE, 0xc4)

struct PACKET_ZC_PC_SELL_ITEMLIST_sub {
	uint16 index;
	uint32 price;
	uint32 overcharge;
} __attribute__((packed));

struct PACKET_ZC_PC_SELL_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	PACKET_ZC_PC_SELL_ITEMLIST_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_SELL_ITEMLIST, 0xc7)

struct PACKET_ZC_CLOSE_DIALOG {
	int16 packetType;
	uint32 npcId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CLOSE_DIALOG, 0xb6)

struct PACKET_ZC_MENU_LIST {
	int16 packetType;
	int16 packetLength;
	uint32 npcId;
	char menu[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MENU_LIST, 0xb7)

struct PACKET_ZC_OPEN_EDITDLG {
	int16 packetType;
	uint32 npcId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_OPEN_EDITDLG, 0x142)

struct PACKET_ZC_OPEN_EDITDLGSTR {
	int16 packetType;
	uint32 npcId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_OPEN_EDITDLGSTR, 0x1d4)

struct PACKET_ZC_ACK_REQ_ALLY_GUILD {
	int16 packetType;
	uint8 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQ_ALLY_GUILD, 0x173)
  
struct PACKET_ZC_DELETE_RELATED_GUILD {
	int16 packetType;
	uint32 allyID;
	uint32 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DELETE_RELATED_GUILD, 0x184)

struct PACKET_ZC_REQ_ALLY_GUILD {
	int16 packetType;
	uint32 inviterId;
	char inviterGuildName[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_ALLY_GUILD, 0x171)

struct PACKET_ZC_ACK_REQ_HOSTILE_GUILD {
	int16 packetType;
	uint8 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQ_HOSTILE_GUILD, 0x181)

struct PACKET_ZC_COMPASS {
	int16 packetType;
	uint32 npcId;
	uint32 type;
	uint32 xPos;
	uint32 yPos;
	uint8 id;
	uint32 color;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_COMPASS, 0x144)

struct PACKET_ZC_ITEM_THROW_ACK {
	int16 packetType;
	uint16 index;
	uint16 count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ITEM_THROW_ACK, 0xaf)

struct PACKET_ZC_DELETE_ITEM_FROM_BODY {
	int16 packetType;
	int16 deleteType;
	uint16 index;
	int16 count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DELETE_ITEM_FROM_BODY, 0x7fa)

struct PACKET_ZC_CARTOFF {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CARTOFF, 0x12b)

struct PACKET_ZC_ACK_GUILD_MENUINTERFACE {
	int16 packetType;
	int32 menuFlag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_GUILD_MENUINTERFACE, 0x014e)

struct PACKET_ZC_ACK_CHANGE_GUILD_POSITIONINFO_sub {
	int32 positionID;
	int32 mode;
	int32 ranking;
	int32 payRate;
	char posName[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_ACK_CHANGE_GUILD_POSITIONINFO {
	int16 packetType;
	int16 packetLength;
	PACKET_ZC_ACK_CHANGE_GUILD_POSITIONINFO_sub posInfo[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_CHANGE_GUILD_POSITIONINFO, 0x174);

struct PACKET_ZC_NOTIFY_POSITION_TO_GUILDM {
	int16 packetType;
	uint32 aid;
	int16 xPos;
	int16 yPos;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_POSITION_TO_GUILDM, 0x1eb)

struct PACKET_ZC_GUILD_CHAT {
	int16 packetType;
	int16 packetLength;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_CHAT, 0x17f)

// TODO: no idea when it changed
#if defined(PACKETVER)
struct PACKET_ZC_UPDATE_CHARSTAT {
	uint16 packetType;
	uint32 aid;
	uint32 cid;
	uint32 status;
	uint16 gender;
	uint16 hairStyle;
	uint16 hairColor;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UPDATE_CHARSTAT, 0x01f2)
#else
struct PACKET_ZC_UPDATE_CHARSTAT {
	uint16 packetType;
	uint32 aid;
	uint32 cid;
	uint32 status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UPDATE_CHARSTAT, 0x016d)
#endif

struct PACKET_ZC_ACK_REQ_CHANGE_MEMBERS_sub {
	uint32 accId;
	uint32 charId;
	int32 positionID;
} __attribute__((packed));

struct PACKET_ZC_ACK_REQ_CHANGE_MEMBERS {
	int16 packetType;
	int16 packetLength;
	PACKET_ZC_ACK_REQ_CHANGE_MEMBERS_sub members[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQ_CHANGE_MEMBERS, 0x156);

struct PACKET_ZC_STATUS {
	int16 packetType;
	uint16 point;
	uint8 str;
	uint8 standardStr;
	uint8 agi;
	uint8 standardAgi;
	uint8 vit;
	uint8 standardVit;
	uint8 int_;
	uint8 standardInt;
	uint8 dex;
	uint8 standardDex;
	uint8 luk;
	uint8 standardLuk;
	int16 attPower;
	int16 refiningPower;
	int16 max_mattPower;
	int16 min_mattPower;
	int16 itemdefPower;
	int16 plusdefPower;
	int16 mdefPower;
	int16 plusmdefPower;
	int16 hitSuccessValue;
	int16 avoidSuccessValue;
	int16 plusAvoidSuccessValue;
	int16 criticalSuccessValue;
	int16 ASPD;
	int16 plusASPD;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_STATUS, 0xbd)

struct PACKET_ZC_NOTIFY_MAPINFO {
	int16 packetType;
	int16 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_MAPINFO, 0x189)

struct PACKET_ZC_ACK_REMEMBER_WARPPOINT {
	int16 packetType;
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REMEMBER_WARPPOINT, 0x11e)

struct PACKET_ZC_DISPEL {
	int16 packetType;
	uint32 gid;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DISPEL, 0x1b9)

struct PACKET_ZC_RESURRECTION {
	int16 packetType;
	uint32 gid;
	int16 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RESURRECTION, 0x148)

struct PACKET_ZC_NOTIFY_MAPPROPERTY2 {
	int16 packetType;
	int16 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_MAPPROPERTY2, 0x1d6)

struct PACKET_ZC_ACK_ITEMREFINING {
	int16 packetType;
	uint16 result;
	uint16 index;
	uint16 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ITEMREFINING, 0x188)

struct PACKET_ZC_PAR_CHANGE_USER {
	int16 packetType;
	uint32 gid;
	int16 type;
	uint32 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PAR_CHANGE_USER, 0x1ab)

struct PACKET_ZC_CHANGE_CHATROOM{
	uint16 packetType;
	uint16 packetSize;
	uint32 ownerId;
	uint32 chatId;
	uint16 limit;
	uint16 users;
	uint8 flag;
	char title[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_CHATROOM, 0xdf);

struct PACKET_ZC_EQUIP_ARROW {
	int16 packetType;
	uint16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIP_ARROW, 0x13c)

#if PACKETVER >= 20130000
struct PACKET_ZC_REQ_TAKEOFF_EQUIP_ACK{
	uint16 packetType;
	uint16 index;
	uint32 wearLocation;
	uint8 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_TAKEOFF_EQUIP_ACK, 0x99a)
#elif PACKETVER >= 20110824
struct PACKET_ZC_REQ_TAKEOFF_EQUIP_ACK{
	uint16 packetType;
	uint16 index;
	uint16 wearLocation;
	uint8 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_TAKEOFF_EQUIP_ACK, 0x8d1)
#else
struct PACKET_ZC_REQ_TAKEOFF_EQUIP_ACK{
	uint16 packetType;
	uint16 index;
	uint16 wearLocation;
	bool flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_TAKEOFF_EQUIP_ACK, 0xac)
#endif

struct PACKET_ZC_CLOSE_STORE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CLOSE_STORE, 0xf8);

struct PACKET_ZC_DELETE_ITEM_FROM_STORE {
	int16 packetType;
	uint16 index;
	uint32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DELETE_ITEM_FROM_STORE, 0xf6);

struct PACKET_ZC_NOTIFY_STOREITEM_COUNTINFO {
	int16 packetType;
	uint16 amount;
	uint16 max_amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_STOREITEM_COUNTINFO, 0xf2);

struct PACKET_ZC_EXCHANGEITEM_UNDO {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EXCHANGEITEM_UNDO, 0xf1);

struct PACKET_ZC_EXEC_EXCHANGE_ITEM {
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EXEC_EXCHANGE_ITEM, 0xf0);

struct PACKET_ZC_CANCEL_EXCHANGE_ITEM {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CANCEL_EXCHANGE_ITEM, 0xee);

struct PACKET_ZC_CONCLUDE_EXCHANGE_ITEM {
	int16 packetType;
	uint8 who;
} __attribute__((packed));

struct PACKET_ZC_ACK_CREATE_CHATROOM {
	int16 packetType;
	uint8 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_CREATE_CHATROOM, 0xd6);

DEFINE_PACKET_HEADER(ZC_CONCLUDE_EXCHANGE_ITEM, 0xec);

struct PACKET_ZC_REFUSE_ENTER_ROOM {
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REFUSE_ENTER_ROOM, 0xda);

struct PACKET_ZC_DESTROY_ROOM {
	int16 packetType;
	uint32 chatId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DESTROY_ROOM, 0xd8);

struct PACKET_ZC_ENTER_ROOM_sub{
	uint32 flag;
	char name[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_ENTER_ROOM{
	uint16 packetType;
	uint16 packetSize;
	uint32 chatId;
	PACKET_ZC_ENTER_ROOM_sub members[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ENTER_ROOM, 0xdb);

struct PACKET_ZC_NPC_SHOWEFST_UPDATE {
	int16 packetType;
	uint32 gid;
	uint32 effectState;
	int32 level;
	uint32 showEFST;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPC_SHOWEFST_UPDATE, 0x28a);

struct PACKET_ZC_ACTION_FAILURE {
	int16 packetType;
	uint16 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACTION_FAILURE, 0x13b)

struct PACKET_ZC_NOTIFY_EFFECT {
	int16 packetType;
	uint32 aid;
	uint32 effectId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_EFFECT, 0x19b);

struct PACKET_ZC_ACK_ITEMCOMPOSITION {
	int16 packetType;
	uint16 equipIndex;
	uint16 cardIndex;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ITEMCOMPOSITION, 0x17d);

struct PACKET_ZC_ACK_ITEMIDENTIFY {
	int16 packetType;
	uint16 index;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ITEMIDENTIFY, 0x179);

struct PACKET_ZC_ACK_ITEMREPAIR {
	int16 packetType;
	uint16 index;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ITEMREPAIR, 0x1fe);

struct PACKET_ZC_EQUIPITEM_DAMAGED {
	int16 packetType;
	uint16 equipLocation;
	uint32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPITEM_DAMAGED, 0x2bb);

struct PACKET_ZC_DELETE_ITEM_FROM_CART {
	int16 packetType;
	uint16 index;
	int32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DELETE_ITEM_FROM_CART, 0x125);

struct PACKET_ZC_OPENSTORE {
	int16 packetType;
	uint16 num;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_OPENSTORE, 0x12d);

struct PACKET_ZC_PC_PURCHASE_RESULT_FROMMC {
	int16 packetType;
	uint16 index;
	uint16 amount;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_RESULT_FROMMC, 0x135);

struct PACKET_ZC_ACK_OPENSTORE2 {
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_OPENSTORE2, 0xa28 );

struct PACKET_ZC_SKILL_DISAPPEAR {
	int16 packetType;
	uint32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILL_DISAPPEAR, 0x120);

struct PACKET_ZC_SKILLINFO_DELETE {
	uint16 packetType;
	uint16 skillID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILLINFO_DELETE, 0x441);

struct PACKET_ZC_SKILL_UPDATE {
	int16 packetType;
	uint32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILL_UPDATE, 0x1ac);

struct PACKET_ZC_HIGHJUMP{
	uint16 packetType;
	uint32 srcId;
	uint16 x;
	uint16 y;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_HIGHJUMP, 0x01ff);

#if PACKETVER >= 20141022
struct PACKET_ZC_RECOVERY {
	int16 packetType;
	uint16 type;
	int32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RECOVERY, 0xa27);
#else
struct PACKET_ZC_RECOVERY {
	int16 packetType;
	uint16 type;
	int16 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RECOVERY, 0x13d);
#endif

#if PACKETVER >= 20131223
struct PACKET_ZC_ACK_WHISPER {
	int16 packetType;
	uint8 result;
	uint32 CID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_WHISPER, 0x9df);
#else
struct PACKET_ZC_ACK_WHISPER {
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_WHISPER, 0x98);
#endif

struct PACKET_ZC_ACK_ADDITEM_TO_CART {
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ADDITEM_TO_CART, 0x12c);

// TODO : not sure for client date [Napster]
#if PACKETVER >= 20141016
struct PACKET_ZC_DELETEITEM_FROM_MCSTORE {
	int16 packetType;
	uint16 index;
	uint16 amount;
	uint32 buyerCID;
	uint32 date;
	int32 zeny;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DELETEITEM_FROM_MCSTORE, 0x9e5);
#else
struct PACKET_ZC_DELETEITEM_FROM_MCSTORE {
	int16 packetType;
	uint16 index;
	uint16 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DELETEITEM_FROM_MCSTORE, 0x137);
#endif

struct PACKET_CZ_REQ_BAN_GUILD{
	int16 packetType;
	uint32 guild_id;
	uint32 AID;
	uint32 CID;
	char message[40];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_BAN_GUILD, 0x15b);

struct PACKET_CZ_REQ_LEAVE_GUILD{
	int16 packetType;
	uint32 guild_id;
	uint32 AID;
	uint32 CID;
	char message[40];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_LEAVE_GUILD, 0x159);

struct PACKET_CZ_REQ_DISORGANIZE_GUILD{
	int16 packetType;
	char key[40];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_DISORGANIZE_GUILD, 0x15d);

struct PACKET_ZC_ACK_DISORGANIZE_GUILD_RESULT{
	int16 packetType;
	int32 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_DISORGANIZE_GUILD_RESULT, 0x15e);

struct PACKET_ZC_RESULT_MAKE_GUILD{
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RESULT_MAKE_GUILD, 0x167);

struct PACKET_CZ_REQ_JOIN_GUILD{
	int16 packetType;
	uint32 AID;
	uint32 inviter_AID;
	uint32 inviter_CID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_JOIN_GUILD, 0x168);

struct PACKET_ZC_ACK_REQ_JOIN_GUILD{
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQ_JOIN_GUILD, 0x169);

struct PACKET_ZC_REQ_JOIN_GUILD{
	int16 packetType;
	uint32 guild_id;
	char guild_name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_JOIN_GUILD, 0x16a);

struct PACKET_CZ_JOIN_GUILD{
	int16 packetType;
	uint32 guild_id;
	int32 answer;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_JOIN_GUILD, 0x16b);

struct PACKET_ZC_GUILD_NOTICE{
	int16 packetType;
	char subject[60];
	char notice[120];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_NOTICE, 0x16f);

struct PACKET_CZ_REQ_JOIN_GUILD2{
	int16 packetType;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_JOIN_GUILD2, 0x916);

struct PACKET_CZ_REQ_JOIN_GROUP{
	int16 packetType;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_JOIN_GROUP, 0xfc);

struct PACKET_CZ_JOIN_GROUP{
	int16 packetType;
	uint32 party_id;
	int32 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_JOIN_GROUP, 0xff);

struct PACKET_CZ_REQ_LEAVE_GROUP{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_LEAVE_GROUP, 0x100);

struct PACKET_CZ_REQ_EXPEL_GROUP_MEMBER{
	int16 packetType;
	uint32 AID;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_EXPEL_GROUP_MEMBER, 0x103);

struct PACKET_CZ_PARTY_JOIN_REQ{
	int16 packetType;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PARTY_JOIN_REQ, 0x2c4);

struct PACKET_CZ_PARTY_JOIN_REQ_ACK{
	int16 packetType;
	uint32 party_id;
	uint8 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PARTY_JOIN_REQ_ACK, 0x2c7);

#if PACKETVER_MAIN_NUM >= 20210000 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20210217
// PACKET_ZC_HO_PAR_CHANGE2
struct PACKET_ZC_HO_PAR_CHANGE {
	int16 packetType;
	uint16 type;
	uint64 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_HO_PAR_CHANGE, 0xba5);
#else
struct PACKET_ZC_HO_PAR_CHANGE {
	int16 packetType;
	uint16 type;
	int32 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_HO_PAR_CHANGE, 0x7db);
#endif

struct PACKET_ZC_EL_PAR_CHANGE {
	int16 packetType;
	uint16 type;
	uint32 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EL_PAR_CHANGE, 0x81e);


struct PACKET_CZ_REQ_EMOTION {
	int16 packetType;
	uint8 emotion_type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_EMOTION, 0xbf);

#if PACKETVER >= 20131223
struct PACKET_ZC_NOTIFY_ACT{
	int16 packetType;
	int32 srcID;
	int32 targetID;
	int32 serverTick;
	int32 srcSpeed;
	int32 dmgSpeed;
	int32 damage;
	int8 isSPDamage;
	uint16 div;
	uint8 type;
	int32 damage2;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_ACT, 0x8c8);
#elif PACKETVER >= 20071113
struct PACKET_ZC_NOTIFY_ACT{
	int16 packetType;
	int32 srcID;
	int32 targetID;
	int32 serverTick;
	int32 srcSpeed;
	int32 dmgSpeed;
	int32 damage;
	uint16 div;
	uint8 type;
	int32 damage2;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_ACT, 0x2e1);
#else
struct PACKET_ZC_NOTIFY_ACT{
	int16 packetType;
	int32 srcID;
	int32 targetID;
	int32 serverTick;
	int32 srcSpeed;
	int32 dmgSpeed;
	int16 damage;
	uint16 div;
	uint8 type;
	int16 damage2;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_ACT, 0x8a);
#endif

struct PACKET_CZ_REQUEST_MOVENPC{
	int16 packetType;
	uint32 GID;
	uint8 PosDir[3];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQUEST_MOVENPC, 0x232);

struct PACKET_ZC_NPCSPRITE_CHANGE{
	int16 packetType;
	uint32 GID;
	uint8 type;
	uint32 class_;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPCSPRITE_CHANGE, 0x1b0);

struct PACKET_ZC_MEMBER_NEWENTRY{
	int16 packetType;
	uint16 count;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MEMBER_NEWENTRY, 0xdc);

struct PACKET_ZC_MEMBER_EXIT{
	int16 packetType;
	uint16 count;
	char name[NAME_LENGTH];
	uint8 kicked;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MEMBER_EXIT, 0xdd);

struct PACKET_CZ_MOVETO_MAP{
	int16 packetType;
	char map[MAP_NAME_LENGTH_EXT];
	uint16 x;
	uint16 y;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MOVETO_MAP, 0x140);

struct PACKET_CZ_BROADCAST{
	int16 packetType;
	uint16 packetSize;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BROADCAST, 0x99);

#if PACKETVER >= 20120925
// CZ_REQ_WEAR_EQUIP_V5
struct PACKET_CZ_REQ_WEAR_EQUIP{
	int16 packetType;
	uint16 index;
	uint32 position;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_WEAR_EQUIP, 0x998);
#else
struct PACKET_CZ_REQ_WEAR_EQUIP{
	int16 packetType;
	uint16 index;
	uint16 position;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_WEAR_EQUIP, 0xa9);
#endif

struct PACKET_CZ_ACK_SELECT_DEALTYPE{
	int16 packetType;
	uint32 GID;
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ACK_SELECT_DEALTYPE, 0xc5);

struct PACKET_CZ_CREATE_CHATROOM{
	int16 packetType;
	uint16 packetSize;
	uint16 limit;
	uint8 type;
	char password[8];
	char title[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CREATE_CHATROOM, 0xd5);

struct PACKET_CZ_BLACKSMITH_RANK{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BLACKSMITH_RANK, 0x217);

struct PACKET_CZ_ALCHEMIST_RANK{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ALCHEMIST_RANK, 0x218);

struct RANKLIST{
	char names[10][NAME_LENGTH];
	uint32 points[10];
} __attribute__((packed));

struct PACKET_ZC_BLACKSMITH_RANK{
	int16 packetType;
	RANKLIST list;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BLACKSMITH_RANK, 0x219);

struct PACKET_ZC_ALCHEMIST_RANK{
	int16 packetType;
	RANKLIST list;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ALCHEMIST_RANK, 0x21a);

struct PACKET_ZC_BLACKSMITH_POINT{
	int16 packetType;
	uint32 points;
	uint32 points_total;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BLACKSMITH_POINT, 0x21b);

struct PACKET_ZC_ALCHEMIST_POINT{
	int16 packetType;
	uint32 points;
	uint32 points_total;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ALCHEMIST_POINT, 0x21c);

struct PACKET_ZC_TAEKWON_POINT{
	int16 packetType;
	uint32 points;
	uint32 points_total;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_TAEKWON_POINT, 0x224);

struct PACKET_CZ_TAEKWON_RANK{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_TAEKWON_RANK, 0x225);

struct PACKET_ZC_TAEKWON_RANK{
	int16 packetType;
	RANKLIST list;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_TAEKWON_RANK, 0x226);

struct PACKET_ZC_KILLER_POINT{
	int16 packetType;
	uint32 points;
	uint32 points_total;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_KILLER_POINT, 0x236);

struct PACKET_CZ_KILLER_RANK{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_KILLER_RANK, 0x237);

struct PACKET_ZC_KILLER_RANK{
	int16 packetType;
	RANKLIST list;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_KILLER_RANK, 0x238);

struct PACKET_CZ_REQ_RANKING{
	int16 packetType;
	uint16 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_RANKING, 0x97c);

struct PACKET_ZC_ACK_RANKING_sub{
	char name[NAME_LENGTH];
	uint32 points;
} __attribute__((packed));

struct PACKET_ZC_ACK_RANKING{
	int16 packetType;
	uint16 type;
	RANKLIST list;
	uint32 mypoints;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_RANKING, 0x97d);

struct PACKET_ZC_UPDATE_RANKING_POINT{
	int16 packetType;
	uint16 type;
	uint32 points;
	uint32 points_total;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UPDATE_RANKING_POINT, 0x97e);

struct PACKET_ZC_ACK_RANKING2{
	int16 packetType;
	uint16 type;
	uint32 CIDs[10];
	uint32 points[10];
	uint32 mypoints;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_RANKING2, 0xaf6);

struct PACKET_CZ_LESSEFFECT{
	int16 packetType;
	int32 state;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_LESSEFFECT, 0x21d);

struct PACKET_CZ_ACTIVE_QUEST{
	int16 packetType;
	uint32 quest_id;
	uint8 active;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ACTIVE_QUEST, 0x2b6);

struct PACKET_CZ_JOIN_BABY{
	int16 packetType;
	uint32 father_AID;
	uint32 mother_AID;
	uint32 accepted;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_JOIN_BABY, 0x1f7);

struct PACKET_CZ_AUCTION_ITEM_SEARCH{
	int16 packetType;
	uint16 type;
	uint32 auction_id;
	char text[24];
	uint16 page;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_AUCTION_ITEM_SEARCH, 0x251);

struct PACKET_CZ_AUCTION_BUY{
	int16 packetType;
	uint32 auction_id;
	uint32 money;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_AUCTION_BUY, 0x24f);

struct PACKET_CZ_AUCTION_ADD{
	int16 packetType;
	uint32 now_money;
	uint32 max_money;
	uint16 hours;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_AUCTION_ADD, 0x24d);

struct PACKET_ZC_DRESSROOM_OPEN{
	int16 packetType;
	uint16 view;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DRESSROOM_OPEN, 0xa02);

struct PACKET_ZC_ROOM_NEWENTRY {
	int16 packetType;
	uint16 packetSize; 
	int32 owner;
	int32 id;
	uint16 limit;
	uint16 users;
	uint8 type;
	char title[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ROOM_NEWENTRY, 0xd7);

struct PACKET_ZC_MONSTER_INFO {
	int16 packetType;
	uint16 class_;
	uint16 level;
	uint16 size;
	uint32 hp;
	int16 def;
	uint16 race;
	int16 mdef;
	uint16 element;
	uint8 water;
	uint8 earth;
	uint8 fire;
	uint8 wind;
	uint8 poison;
	uint8 holy;
	uint8 shadow;
	uint8 ghost;
	uint8 undead;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MONSTER_INFO, 0x18c);

#if PACKETVER >= 20180221
struct PACKET_ZC_ACK_REQNAME_BYGID {
	int16 packetType;
	uint16 flag;
	uint32 CID;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQNAME_BYGID, 0xaf7);
#else
struct PACKET_ZC_ACK_REQNAME_BYGID {
	int16 packetType;
	uint32 CID;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQNAME_BYGID, 0x194);
#endif

struct PACKET_ZC_PET_ACT {
	int16 packetType;
	uint32 GID;
	int32 data;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PET_ACT, 0x1aa);

struct PACKET_ZC_COMBODELAY {
	int16 packetType;
	uint32 AID;
	uint32 delay;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_COMBODELAY, 0x1d2);

struct PACKET_ZC_BLADESTOP {
	int16 packetType;
	uint32 srcId;
	uint32 targetId;
	uint32 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BLADESTOP, 0x1d1);

struct PACKET_ZC_MVP {
	int16 packetType;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MVP, 0x10c);

struct PACKET_ZC_MVP_GETTING_SPECIAL_EXP {
	int16 packetType;
	uint32 exp;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MVP_GETTING_SPECIAL_EXP, 0x10b);

struct PACKET_ZC_THROW_MVPITEM {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_THROW_MVPITEM, 0x10d);

struct PACKET_ZC_UPDATE_MAPINFO{
	int16 packetType;
	int16 x;
	int16 y;
	int16 type;
	char mapname[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UPDATE_MAPINFO, 0x192);

struct PACKET_CZ_REQ_ENTER_ROOM{
	int16 packetType;
	uint32 chat_id;
	char password[8];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_ENTER_ROOM, 0xd9);

struct PACKET_CZ_CHANGE_CHATROOM{
	int16 packetType;
	uint16 packetSize;
	uint16 limit;
	uint8 type;
	char password[8];
	char title[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CHANGE_CHATROOM, 0xde);

struct PACKET_CZ_ADD_EXCHANGE_ITEM{
	int16 packetType;
	uint16 index;
	int32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ADD_EXCHANGE_ITEM, 0xe8);

struct PACKET_CZ_MOVE_ITEM_FROM_CART_TO_BODY{
	int16 packetType;
	uint16 index;
	int32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MOVE_ITEM_FROM_CART_TO_BODY, 0x127);

struct PACKET_CZ_SELECT_WARPPOINT{
	int16 packetType;
	int16 skill_id;
	char mapname[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SELECT_WARPPOINT, 0x11b);

struct PACKET_CZ_INPUT_EDITDLG{
	int16 packetType;
	uint32 GID;
	int32 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_INPUT_EDITDLG, 0x143);

struct PACKET_CZ_INPUT_EDITDLGSTR{
	int16 packetType;
	uint16 packetSize;
	int32 GID;
	char value[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_INPUT_EDITDLGSTR, 0x1d5);

struct PACKET_CZ_CLOSE_DIALOG{
	int16 packetType;
	uint32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CLOSE_DIALOG, 0x146);

struct PACKET_CZ_RESET{
	int16 packetType;
	uint16 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_RESET, 0x197);

struct PACKET_CZ_MOVE_ITEM_FROM_STORE_TO_CART{
	int16 packetType;
	uint16 index;
	int32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MOVE_ITEM_FROM_STORE_TO_CART, 0x128);

struct PACKET_CZ_REQ_ITEMIDENTIFY{
	int16 packetType;
	uint16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_ITEMIDENTIFY, 0x178);

struct PACKET_CZ_SELECTAUTOSPELL{
	int16 packetType;
	uint32 skill_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SELECTAUTOSPELL, 0x1ce);

struct PACKET_CZ_REQ_ITEMCOMPOSITION_LIST{
	int16 packetType;
	uint16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_ITEMCOMPOSITION_LIST, 0x17a);

struct PACKET_CZ_REQ_ITEMCOMPOSITION{
	int16 packetType;
	uint16 index_card;
	uint16 index_equip;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_ITEMCOMPOSITION, 0x17c);

struct PACKET_CZ_LOCALBROADCAST{
	int16 packetType;
	uint16 packetSize;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_LOCALBROADCAST, 0x19c);

struct PACKET_CZ_MOVE_ITEM_FROM_CART_TO_STORE{
	int16 packetType;
	uint16 index;
	int32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MOVE_ITEM_FROM_CART_TO_STORE, 0x129);

#if PACKETVER_MAIN_NUM >= 20180307 || PACKETVER_RE_NUM >= 20180221 || PACKETVER_ZERO_NUM >= 20180328
struct PACKET_ZC_FRIENDS_STATE{
	int16 packetType;
	uint32 AID;
	uint32 CID;
	uint8 offline;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_FRIENDS_STATE, 0x206);
#else
struct PACKET_ZC_FRIENDS_STATE{
	int16 packetType;
	uint32 AID;
	uint32 CID;
	uint8 offline;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_FRIENDS_STATE, 0x206);
#endif

struct PACKET_CZ_MAKE_GROUP{
	int16 packetType;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MAKE_GROUP, 0xf9);

struct PACKET_CZ_MAKE_GROUP2{
	int16 packetType;
	char name[NAME_LENGTH];
	uint8 item_pickup;
	uint8 item_share;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MAKE_GROUP2, 0x1e8);

struct PACKET_CZ_GM_CHECKER{
	int16 packetType;
	char mapname[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_GM_CHECKER, 0xc0b);

struct PACKET_ZC_GM_CHECKER{
	int16 packetType;
	int16 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GM_CHECKER, 0xc0c);

struct PACKET_ZC_CONFIG_NOTIFY {
	int16 packetType;
	uint8 flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CONFIG_NOTIFY, 0x2da);

struct PACKET_ZC_CONGRATULATION {
	int16 packetType;
	int32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CONGRATULATION, 0x1ea);

struct PACKET_ZC_DIVORCE{
	int16 packetType;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DIVORCE, 0x205);

struct PACKET_ZC_EMOTION {
	int16 packetType;
	int32 GID;
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EMOTION, 0xc0);

struct PACKET_ZC_MSG {
	int16 packetType;
	uint16 msgId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MSG, 0x291);

struct PACKET_ZC_MSG_SKILL {
	int16 packetType;
	uint16 skillId;
	int32 msgId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MSG_SKILL, 0x7e6);

struct PACKET_ZC_MSG_VALUE {
	int16 packetType;
	uint16 message;
	int32 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MSG_VALUE, 0x7e2);

struct PACKET_ZC_NOTIFY_MANNER_POINT_GIVEN {
	int16 packetType;
	uint8 type;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_MANNER_POINT_GIVEN, 0x14b);

struct PACKET_ZC_NOTIFY_TIME {
	int16 packetType;
	uint32 time;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_TIME, 0x7f);

struct PACKET_ZC_SETTING_WHISPER_PC {
	int16 packetType;
	uint8 type;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SETTING_WHISPER_PC, 0xd1);

struct PACKET_ZC_SETTING_WHISPER_STATE {
	int16 packetType;
	uint8 type;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SETTING_WHISPER_STATE, 0xd2);

struct PACKET_ZC_SKILLMSG {
	int16 packetType;
	int32 msgId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILLMSG, 0x215);

struct PACKET_CZ_REQ_EMOTION_EXPANSION{
	int16 packetType;
	uint8 unknown[4];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_EMOTION_EXPANSION, 0xbe9);

struct PACKET_ZC_DISAPPEAR_ENTRY{
	int16 packetType;
	int32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DISAPPEAR_ENTRY, 0x132);

#if PACKETVER >= 20120618
struct PACKET_ZC_EFST_SET_ENTER{
	uint16 packetType;
	uint32 targetID;
	uint16 type;
	uint32 duration;
	uint32 duration2;
	int32 val1;
	int32 val2;
	int32 val3;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EFST_SET_ENTER, 0x984)
#else
struct PACKET_ZC_EFST_SET_ENTER{
	uint16 packetType;
	uint32 targetID;
	uint16 type;
	uint32 duration;
	int32 val1;
	int32 val2;
	int32 val3;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EFST_SET_ENTER, 0x8ff)
#endif

struct PACKET_CZ_SETTING_WHISPER_STATE{
	int16 packetType;
	int8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SETTING_WHISPER_STATE, 0xd0);

struct PACKET_ZC_WHISPER_LIST_sub{
	char name[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_WHISPER_LIST{
	int16 packetType;
	uint16 packetSize;
	PACKET_ZC_WHISPER_LIST_sub names[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_WHISPER_LIST, 0xd4);

struct PACKET_CZ_ALLY_CHAT{
	int16 packetType;
	int16 packetLength;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ALLY_CHAT, 0xbdd);

struct PACKET_CZ_REQ_REPORT_USER{
	int16 packetType;
	uint8 unknown[135];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_REPORT_USER, 0xbe2);

struct PACKET_CZ_QUEST_STATUS_REQ{
	int16 packetType;
	int16 packetLength;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_QUEST_STATUS_REQ, 0xbf3);

struct PACKET_CZ_MOVE_ITEM_TO_PERSONAL{
	int16 packetType;
	uint32 unknown;
	uint16 index;
	uint32 amount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MOVE_ITEM_TO_PERSONAL, 0xc22);

#if PACKETVER_MAIN_NUM >= 20250402
struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2 {
	uint32 nameid;
	uint16 refine_level;
	uint32 amount;
	uint16 type;
} __attribute__((packed));

struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub {
	uint32 nameid;
	uint16 type;
	uint32 amount;
	uint32 weight;
	uint32 index;
	uint32 zeny;
	uint16 viewSprite;
	uint32 location;
	uint32 currency_count;
	uint32 refine_level;
	// Workaround: this should be currencies[], but compilers do not support multiple layers of incomplete types. See error C2233
	struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2 currencies[1];
} __attribute__((packed));

struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO {
	int16 packetType;
	int16 packetLength;
	int32 items_count;
	struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub items[];
} __attribute__((packed));

DEFINE_PACKET_HEADER(ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO, 0x0b79);
#elif PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2 {
	uint32 nameid;
	uint16 refine_level;
	uint32 amount;
	uint16 type;
} __attribute__((packed));

struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub {
	uint32 nameid;
	uint16 type;
	uint32 amount;
	uint32 weight;
	uint32 index;
	uint32 zeny;
	uint16 viewSprite;
	uint32 location;
	uint32 currency_count;
	// Workaround: this should be currencies[], but compilers do not support multiple layers of incomplete types. See error C2233
	struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2 currencies[1];
} __attribute__((packed));

struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO {
	int16 packetType;
	int16 packetLength;
	int32 items_count;
	struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub items[];
} __attribute__((packed));

DEFINE_PACKET_HEADER(ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO, 0x0b79);
#elif PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2 {
	uint32 nameid;
	uint16 refine_level;
	uint32 amount;
	uint16 type;
} __attribute__((packed));

struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub {
	uint32 nameid;
	uint16 type;
	uint32 amount;
	uint32 weight;
	uint32 index;
	uint32 zeny;
	uint32 currency_count;
	// Workaround: this should be currencies[], but compilers do not support multiple layers of incomplete types. See error C2233
	struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2 currencies[1];
} __attribute__((packed));

struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO {
	int16 packetType;
	int16 packetLength;
	int32 items_count;
	struct PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub items[];
} __attribute__((packed));

DEFINE_PACKET_HEADER(ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO, 0x0b56);
#endif  // PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( pop )
#endif

// Other packets without struct defined in this file
DEFINE_PACKET_HEADER(ZC_NOTIFY_CHAT, 0x8d)
DEFINE_PACKET_HEADER(ZC_ITEM_ENTRY, 0x9d)
DEFINE_PACKET_HEADER(ZC_MVP_GETTING_ITEM, 0x10a)
DEFINE_PACKET_HEADER(ZC_MAKABLEITEMLIST, 0x18d)
DEFINE_PACKET_HEADER(CZ_REQMAKINGITEM, 0x18e)
DEFINE_PACKET_HEADER(ZC_ACK_REQMAKINGITEM, 0x18f)
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	DEFINE_PACKET_HEADER(CZ_REQ_ITEMREPAIR, 0xb66)
#else
	DEFINE_PACKET_HEADER(CZ_REQ_ITEMREPAIR, 0x1fd)
#endif
DEFINE_PACKET_HEADER(ZC_NOTIFY_WEAPONITEMLIST, 0x221)
DEFINE_PACKET_HEADER(ZC_ACK_WEAPONREFINE, 0x223)
DEFINE_PACKET_HEADER(CZ_REQ_MAKINGITEM, 0x25b)
DEFINE_PACKET_HEADER(ZC_PC_CASH_POINT_ITEMLIST, 0x287)
DEFINE_PACKET_HEADER(ZC_CASH_TIME_COUNTER, 0x298)
DEFINE_PACKET_HEADER(ZC_CASH_ITEM_DELETE, 0x299)
DEFINE_PACKET_HEADER(CZ_SKILL_SELECT_RESPONSE, 0x443)
DEFINE_PACKET_HEADER(ZC_MYITEMLIST_BUYING_STORE, 0x813)
DEFINE_PACKET_HEADER(ZC_ACK_ITEMLIST_BUYING_STORE, 0x818)
DEFINE_PACKET_HEADER(ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER, 0x824)
DEFINE_PACKET_HEADER(CZ_SSILIST_ITEM_CLICK, 0x83c)
DEFINE_PACKET_HEADER(ZC_ACK_SCHEDULER_CASHITEM, 0x8ca)
DEFINE_PACKET_HEADER(ZC_NOTIFY_CLAN_CONNECTINFO, 0x988)
DEFINE_PACKET_HEADER(ZC_ACK_CLAN_LEAVE, 0x989)
DEFINE_PACKET_HEADER(ZC_NOTIFY_CLAN_CHAT, 0x98e)
DEFINE_PACKET_HEADER(CZ_REQ_BANKING_DEPOSIT, 0x9a7)
DEFINE_PACKET_HEADER(CZ_REQ_BANKING_WITHDRAW, 0x9a9)
DEFINE_PACKET_HEADER(CZ_REQ_BANKING_CHECK, 0x9ab)
DEFINE_PACKET_HEADER(CZ_REQ_OPEN_BANKING, 0x9b6)
DEFINE_PACKET_HEADER(CZ_REQ_CLOSE_BANKING, 0x9b8)
DEFINE_PACKET_HEADER(CZ_REQ_APPLY_BARGAIN_SALE_ITEM2, 0xa3d)
DEFINE_PACKET_HEADER(CZ_REQ_STYLE_CHANGE, 0xa46)
DEFINE_PACKET_HEADER(ZC_STYLE_CHANGE_RES, 0xa47)
DEFINE_PACKET_HEADER(ZC_GROUP_ISALIVE, 0xab2)
DEFINE_PACKET_HEADER(CZ_REQ_STYLE_CHANGE2, 0xafc)
DEFINE_PACKET_HEADER(ZC_GUILD_POSITION, 0x0afd)
DEFINE_PACKET_HEADER(ZC_REMOVE_EFFECT, 0x0b0d)
DEFINE_PACKET_HEADER(ZC_FEED_MER, 0x22f)
DEFINE_PACKET_HEADER(ZC_FEED_PET, 0x1a3)
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
DEFINE_PACKET_HEADER(ZC_WARPLIST, 0xabe)
#else
DEFINE_PACKET_HEADER(ZC_WARPLIST, 0x11c)
#endif
DEFINE_PACKET_HEADER(ZC_MSG_COLOR, 0x9cd);


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
