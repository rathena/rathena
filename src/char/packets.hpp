// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PACKETS_HPP
#define PACKETS_HPP

#include "../common/mmo.hpp"

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
#if PACKETVER_RE_NUM >= 20211103
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

#define DEFINE_PACKET_HEADER(name, id) const int16 HEADER_##name = id;

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
	DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARACTER_SLOT, 0xb70 )
#else
	DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARACTER_SLOT, 0x8d5 )
#endif
#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
	DEFINE_PACKET_HEADER( HC_ACK_CHARINFO_PER_PAGE, 0xb72 )
#else
	DEFINE_PACKET_HEADER( HC_ACK_CHARINFO_PER_PAGE, 0x99d )
#endif
#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
	DEFINE_PACKET_HEADER( HC_ACCEPT_MAKECHAR, 0xb6f )
#else
	DEFINE_PACKET_HEADER( HC_ACCEPT_MAKECHAR, 0x6d )
#endif

#undef DEFINE_PACKET_HEADER

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( pop )
#endif

#pragma warning( pop )

#endif /* PACKETS_HPP */
