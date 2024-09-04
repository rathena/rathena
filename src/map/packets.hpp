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

struct PACKET_ZC_NOTIFY_POSITION_TO_GUILDM {
	int16 packetType;
	uint32 aid;
	int16 xPos;
	int16 yPos;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_POSITION_TO_GUILDM, 0x1eb)

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
	uint16 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PAR_CHANGE_USER, 0x1ab)

struct PACKET_ZC_EQUIP_ARROW {
	int16 packetType;
	uint16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIP_ARROW, 0x13c)

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

struct PACKET_ZC_SKILL_UPDATE {
	int16 packetType;
	uint32 GID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILL_UPDATE, 0x1ac);

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

struct PACKET_ZC_EL_PAR_CHANGE {
	int16 packetType;
	uint16 type;
	uint32 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EL_PAR_CHANGE, 0x81e);

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
DEFINE_PACKET_HEADER(ZC_REMOVE_EFFECT, 0x0b0d)
DEFINE_PACKET_HEADER(ZC_FEED_MER, 0x22f)
DEFINE_PACKET_HEADER(ZC_FEED_PET, 0x1a3)

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
