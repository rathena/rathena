// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CASHSHOP_H_
#define _CASHSHOP_H_

#include "../common/cbasetypes.h" // uint16, uint32
#include "pc.h" // struct map_session_data

void do_init_cashshop( void );
void do_final_cashshop( void );
void cashshop_reloaddb( void );
bool cashshop_buylist( struct map_session_data* sd, uint32 kafrapoints, int n, uint16* item_list );

// Taken from AEGIS
enum CASH_SHOP_TAB_CODE
{
	CASHSHOP_TAB_NEW =  0x0,
	CASHSHOP_TAB_POPULAR =  0x1,
	CASHSHOP_TAB_LIMITED =  0x2,
	CASHSHOP_TAB_RENTAL =  0x3,
	CASHSHOP_TAB_PERPETUITY =  0x4,
	CASHSHOP_TAB_BUFF =  0x5,
	CASHSHOP_TAB_RECOVERY =  0x6,
	CASHSHOP_TAB_ETC =  0x7,
	CASHSHOP_TAB_SEARCH =  0x8
};

// PACKET_ZC_SE_PC_BUY_CASHITEM_RESULT
enum CASHSHOP_BUY_RESULT
{
	CASHSHOP_RESULT_SUCCESS =  0x0,
	CASHSHOP_RESULT_ERROR_SYSTEM =  0x1,
	CASHSHOP_RESULT_ERROR_SHORTTAGE_CASH =  0x2,
	CASHSHOP_RESULT_ERROR_UNKONWN_ITEM =  0x3,
	CASHSHOP_RESULT_ERROR_INVENTORY_WEIGHT =  0x4,
	CASHSHOP_RESULT_ERROR_INVENTORY_ITEMCNT =  0x5,
	CASHSHOP_RESULT_ERROR_PC_STATE =  0x6,
	CASHSHOP_RESULT_ERROR_OVER_PRODUCT_TOTAL_CNT =  0x7,
	CASHSHOP_RESULT_ERROR_SOME_BUY_FAILURE =  0x8,
	CASHSHOP_RESULT_ERROR_RUNE_OVERCOUNT =  0x9,
	CASHSHOP_RESULT_ERROR_EACHITEM_OVERCOUNT =  0xa,
	CASHSHOP_RESULT_ERROR_UNKNOWN =  0xb,
	CASHSHOP_RESULT_ERROR_BUSY =  0xc,
};

struct cash_item_data{
	unsigned short nameid;
	uint32 price;
};

struct cash_item_db{
	struct cash_item_data** item;
	uint32 count;
};

extern struct cash_item_db cash_shop_items[CASHSHOP_TAB_SEARCH];
extern bool cash_shop_defined;

#endif /* _CASHSHOP_H_ */
