// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CASHSHOP_HPP_
#define _CASHSHOP_HPP_

#include "../common/cbasetypes.hpp" // uint16, uint32
#include "../common/timer.hpp" // ShowWarning, ShowStatus
#include "../config/core.hpp"

struct map_session_data;

void do_init_cashshop( void );
void do_final_cashshop( void );
void cashshop_reloaddb( void );
bool cashshop_buylist( struct map_session_data* sd, uint32 kafrapoints, int n, uint16* item_list );

// Taken from AEGIS
enum CASH_SHOP_TAB_CODE
{
	CASHSHOP_TAB_NEW =  0x0,
	CASHSHOP_TAB_POPULAR,
	CASHSHOP_TAB_LIMITED,
	CASHSHOP_TAB_RENTAL,
	CASHSHOP_TAB_PERPETUITY,
	CASHSHOP_TAB_BUFF,
	CASHSHOP_TAB_RECOVERY,
	CASHSHOP_TAB_ETC,
#if PACKETVER_SUPPORTS_SALES
	CASHSHOP_TAB_SALE,
#endif
	CASHSHOP_TAB_MAX
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

extern struct cash_item_db cash_shop_items[CASHSHOP_TAB_MAX];
extern bool cash_shop_defined;

enum e_sale_add_result {
	SALE_ADD_SUCCESS = 0,
	SALE_ADD_FAILED = 1,
	SALE_ADD_DUPLICATE = 2
};

struct sale_item_data{
	// Data
	uint16 nameid;
	time_t start;
	time_t end;
	uint32 amount;

	// Timers
	int timer_start;
	int timer_end;
};

struct sale_item_db{
	struct sale_item_data** item;
	uint32 count;
};

#if PACKETVER_SUPPORTS_SALES
extern struct sale_item_db sale_items;

struct sale_item_data* sale_find_item(uint16 nameid, bool onsale);
enum e_sale_add_result sale_add_item(uint16 nameid, int32 count, time_t from, time_t to);
bool sale_remove_item(uint16 nameid);
void sale_notify_login( struct map_session_data* sd );
#endif

#endif /* _CASHSHOP_HPP_ */
