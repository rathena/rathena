// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PACKETS_HPP
#define PACKETS_HPP

#include <common/mmo.hpp>

#pragma warning( push )
#pragma warning( disable : 4200 )

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( push, 1 )
#endif

struct CHARACTER_INFO{
	uint32 GID;
#if PACKETVER >= 20170830
	int64 exp;
#else
	int32 exp;
#endif
	int32 money;
#if PACKETVER >= 20170830
	int64 jobexp;
#else
	int32 jobexp;
#endif
	int32 joblevel;
	int32 bodystate;
	int32 healthstate;
	int32 effectstate;
	int32 virtue;
	int32 honor;
	int16 jobpoint;
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
	int64 hp;
	int64 maxhp;
	int64 sp;
	int64 maxsp;
#else
	int32 hp;
	int32 maxhp;
	int16 sp;
	int16 maxsp;
#endif
	int16 speed;
	int16 job;
	int16 head;
#if PACKETVER >= 20141022
	int16 body;
#endif
	int16 weapon;
	int16 level;
	int16 sppoint;
	int16 accessory;
	int16 shield;
	int16 accessory2;
	int16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	char name[24];
	uint8 Str;
	uint8 Agi;
	uint8 Vit;
	uint8 Int;
	uint8 Dex;
	uint8 Luk;
	uint8 CharNum;
	uint8 hairColor;
	int16 bIsChangedCharName;
#if (PACKETVER >= 20100720 && PACKETVER <= 20100727) || PACKETVER >= 20100803
	char mapName[16];
#endif
#if PACKETVER >= 20100803
	int32 DelRevDate;
#endif
#if PACKETVER >= 20110111
	int32 robePalette;
#endif
#if PACKETVER >= 20110928
	int32 chr_slot_changeCnt;
#endif
#if PACKETVER >= 20111025
	int32 chr_name_changeCnt;
#endif
#if PACKETVER >= 20141016
	uint8 sex;
#endif
} __attribute__((packed));

struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME_sub{
	int32 status;
	char map[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));

struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME{
	int16 packetType;
	int16 packetLength;
	struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME_sub maps[];
} __attribute__((packed));

struct PACKET_CH_SELECT_ACCESSIBLE_MAPNAME{
	int16 packetType;
	int8 slot;
	int8 mapnumber;
} __attribute__((packed));

#define DEFINE_PACKET_HEADER(name, id) const int16 HEADER_##name = id;

// Additional packet struct definitions from char_clif.cpp

// Outgoing packets
DEFINE_PACKET_HEADER( HC_NOTIFY_ACCESSIBLE_MAPNAME, 0x840 )
struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME_sub{
	int32 status;
	char map[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));

struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME{
	int16 packetType;
	int16 packetLength;
	struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME_sub maps[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
	DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARACTER_SLOT, 0xb70 )
#else
	DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARACTER_SLOT, 0x8d5 )
#endif
struct PACKET_HC_ACK_CHANGE_CHARACTER_SLOT {
	int16 packetType;
	int16 packetLength;
	int16 reason;
	int16 charMoves;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
	DEFINE_PACKET_HEADER( HC_ACK_CHARINFO_PER_PAGE, 0xb72 )
#else
	DEFINE_PACKET_HEADER( HC_ACK_CHARINFO_PER_PAGE, 0x99d )
#endif
struct PACKET_HC_ACK_CHARINFO_PER_PAGE {
	int16 packetType;
	int16 packetLength;
	// Dynamic size - contents would depend on character data
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
	DEFINE_PACKET_HEADER( HC_ACCEPT_MAKECHAR, 0xb6f )
#else
	DEFINE_PACKET_HEADER( HC_ACCEPT_MAKECHAR, 0x6d )
#endif
struct PACKET_HC_ACCEPT_MAKECHAR {
	int16 packetType;
	int16 packetLength;
	// Character data would follow - using characterinfo structure
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_REFUSE_ENTER, 0x6c )
struct PACKET_HC_REFUSE_ENTER {
	int16 packetType;
	uint8 errorCode;
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_NOTIFY_BAN, 0x81 )
struct PACKET_HC_NOTIFY_BAN {
	int16 packetType;
	uint8 result;
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_CHARLIST_NOTIFY, 0x9a0 )
struct PACKET_HC_CHARLIST_NOTIFY {
	int16 packetType;
	int32 totalPages;
#if defined(PACKETVER_RE) && PACKETVER >= 20151001 && PACKETVER < 20180103
	int32 charSlots;
#endif
} __attribute__((packed));

struct PACKET_HC_BLOCK_CHARACTER_sub {
	uint32 GID;
	char szExpireDate[20];
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_BLOCK_CHARACTER, 0x20d )
struct PACKET_HC_BLOCK_CHARACTER {
	int16 packetType;
	int16 packetLength;
	// Dynamic size - array of PACKET_HC_BLOCK_CHARACTER_sub entries
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_CHAR_DELETE2_ACK, 0x828 )
struct PACKET_HC_CHAR_DELETE2_ACK {
	int16 packetType;
	uint32 charId;
	uint32 result;
	uint32 deleteDate;
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_CHAR_DELETE2_ACCEPT_ACK, 0x82a )
struct PACKET_HC_CHAR_DELETE2_ACCEPT_ACK {
	int16 packetType;
	uint32 charId;
	uint32 result;
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_CHAR_DELETE2_CANCEL_ACK, 0x82c )
struct PACKET_HC_CHAR_DELETE2_CANCEL_ACK {
	int16 packetType;
	uint32 charId;
	uint32 result;
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_ACK_IS_VALID_CHARNAME, 0x28e )
struct PACKET_HC_ACK_IS_VALID_CHARNAME {
	int16 packetType;
	uint16 result;
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARNAME, 0x290 )
struct PACKET_HC_ACK_CHANGE_CHARNAME {
	int16 packetType;
	uint16 result;
} __attribute__((packed));

DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARACTERNAME, 0x8fd )
struct PACKET_HC_ACK_CHANGE_CHARACTERNAME {
	int16 packetType;
	uint32 result;
} __attribute__((packed));

struct PACKET_HC_SEND_MAP_DATA {
	int16 packetType;
	uint32 charId;
	char mapName[16];
	uint32 ip;
	uint16 port;
#if PACKETVER >= 20170315
	uint8 unknown[128]; // Unknown data
#endif
} __attribute__((packed));

// Incoming packets
DEFINE_PACKET_HEADER( CH_SELECT_ACCESSIBLE_MAPNAME, 0x841 )

DEFINE_PACKET_HEADER( CH_REQ_TO_CONNECT, 0x65 )
struct PACKET_CH_REQ_TO_CONNECT {
	int16 packetType;
	uint32 account_id;
	uint32 login_id1;
	uint32 login_id2;
	uint8 sex;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_SELECT_CHAR, 0x66 )
struct PACKET_CH_SELECT_CHAR {
	int16 packetType;
	uint8 slot;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_MAKE_NEW_CHAR_2015, 0xa39 )
DEFINE_PACKET_HEADER( CH_MAKE_NEW_CHAR_2012, 0x970 )
DEFINE_PACKET_HEADER( CH_MAKE_NEW_CHAR, 0x67 )
struct PACKET_CH_MAKE_NEW_CHAR {
	int16 packetType;
	char name[24]; // NAME_LENGTH
#if PACKETVER >= 20151001
	uint8 slot;
	uint16 hairColor;
	uint16 hairStyle;
	uint16 startingJob;
	uint16 unknown; // Unknown field
	uint8 sex;
#elif PACKETVER >= 20120307
	uint8 slot;
	uint16 hairColor;
	uint16 hairStyle;
#else
	uint8 str;
	uint8 agi;
	uint8 vit;
	uint8 int_;
	uint8 dex;
	uint8 luk;
	uint8 slot;
	uint16 hairColor;
	uint16 hairStyle;
#endif
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_DELETE_CHAR, 0x68 )
DEFINE_PACKET_HEADER( CH_DELETE_CHAR_NEW, 0x1fb )
struct PACKET_CH_DELETE_CHAR {
	int16 packetType;
	uint32 charId;
	char email[40]; // Email field size
} __attribute__((packed));

struct PACKET_CH_DELETE_CHAR_NEW {
	int16 packetType;
	uint32 charId;
	char email[40]; // Additional fields in newer version
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_KEEP_ALIVE, 0x187 )
struct PACKET_CH_KEEP_ALIVE {
	int16 packetType;
	uint32 accountId;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_IS_VALID_CHARNAME, 0x28d )
struct PACKET_CH_REQ_IS_VALID_CHARNAME {
	int16 packetType;
	uint32 accountId;
	uint32 charId;
	char newName[24]; // NAME_LENGTH
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_CHANGE_CHARNAME, 0x28f )
struct PACKET_CH_REQ_CHANGE_CHARNAME {
	int16 packetType;
	uint32 charId;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_CHANGE_CHARACTERNAME, 0x8fc )
struct PACKET_CH_REQ_CHANGE_CHARACTERNAME {
	int16 packetType;
	uint32 charId;
	char newName[24]; // NAME_LENGTH
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_CAPTCHA, 0x7e5 )
struct PACKET_CH_REQ_CAPTCHA {
	int16 packetType;
	uint16 unknown;
	uint32 accountId;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_CHECK_CAPTCHA, 0x7e7 )
struct PACKET_CH_CHECK_CAPTCHA {
	int16 packetType;
	uint16 length;
	uint32 accountId;
	uint8 code[10]; // Captcha code
	uint8 unknown[14]; // Unknown data
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_CHAR_DELETE2, 0x827 )
struct PACKET_CH_REQ_CHAR_DELETE2 {
	int16 packetType;
	uint32 charId;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_CHAR_DELETE2_ACCEPT, 0x829 )
struct PACKET_CH_REQ_CHAR_DELETE2_ACCEPT {
	int16 packetType;
	uint32 charId;
	uint8 birthDate[6]; // YY MM DD
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_CHAR_DELETE2_CANCEL, 0x82b )
struct PACKET_CH_REQ_CHAR_DELETE2_CANCEL {
	int16 packetType;
	uint32 charId;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_CHARLIST, 0x9a1 )
struct PACKET_CH_REQ_CHARLIST {
	int16 packetType;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_PINCODE_CHECK, 0x8b8 )
struct PACKET_CH_PINCODE_CHECK {
	int16 packetType;
	uint32 accountId;
	uint32 seed;
	char pinCode[4]; // Assuming PINCODE_LENGTH is 4
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_PINCODE_CHANGE, 0x8be )
struct PACKET_CH_PINCODE_CHANGE {
	int16 packetType;
	uint32 accountId;
	char oldPin[4]; // Assuming PINCODE_LENGTH is 4
	char newPin[4]; // Assuming PINCODE_LENGTH is 4
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_PINCODE_SETNEW, 0x8ba )
struct PACKET_CH_PINCODE_SETNEW {
	int16 packetType;
	uint32 accountId;
	char newPin[4]; // Assuming PINCODE_LENGTH is 4
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_REQ_PINCODE_WINDOW, 0x8c5 )
struct PACKET_CH_REQ_PINCODE_WINDOW {
	int16 packetType;
	uint32 accountId;
} __attribute__((packed));

DEFINE_PACKET_HEADER( CH_MOVE_CHAR_SLOT, 0x8d4 )
struct PACKET_CH_MOVE_CHAR_SLOT {
	int16 packetType;
	uint16 from;
	uint16 to;
} __attribute__((packed));

#undef DEFINE_PACKET_HEADER

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( pop )
#endif

#pragma warning( pop )

#endif /* PACKETS_HPP */
