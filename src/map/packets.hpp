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
#define MAX_ITEMLIST MAX_INVENTORY
#define MAX_ACHIEVEMENT_DB MAX_ACHIEVEMENT_OBJECTIVES

#define DEFINE_PACKET_HEADER(name, id) const uint16 HEADER_##name = id;

#include "packets_struct.hpp"

#undef MAX_ITEM_OPTIONS
#undef UNAVAILABLE_STRUCT
#undef MAX_ITEMLIST
#undef MAX_ACHIEVEMENT_DB
#undef MAX_PACKET_POS

#undef DEFINE_PACKET_HEADER

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

#pragma warning( pop )

#endif /* PACKETS_HPP */
