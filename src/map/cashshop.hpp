// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CASHSHOP_HPP
#define CASHSHOP_HPP

#include <memory> // std::shared_ptr
#include <vector> // std::vector

#include <config/core.hpp>

#include <common/cbasetypes.hpp> // uint16, uint32
#include <common/database.hpp> // TypesafeYamlDatabase
#include <common/mmo.hpp> // t_itemid
#include <common/timer.hpp> // ShowWarning, ShowStatus

#include "clif.hpp"

class map_session_data;

void do_init_cashshop( void );
void do_final_cashshop( void );
void cashshop_reloaddb( void );
bool cashshop_buylist( map_session_data* sd, uint32 kafrapoints, int n, const PACKET_CZ_SE_PC_BUY_CASHITEM_LIST_sub* item_list );

// Taken from AEGIS (CASH_SHOP_TAB_CODE)
enum e_cash_shop_tab : uint16{
	CASHSHOP_TAB_NEW =  0x0,
	CASHSHOP_TAB_HOT,
	CASHSHOP_TAB_LIMITED,
	CASHSHOP_TAB_RENTAL,
	CASHSHOP_TAB_PERMANENT,
	CASHSHOP_TAB_SCROLLS,
	CASHSHOP_TAB_CONSUMABLES,
	CASHSHOP_TAB_OTHER,
	CASHSHOP_TAB_SALE,
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

struct s_cash_item{
	t_itemid nameid;
	uint32 price;
};

struct s_cash_item_tab{
	e_cash_shop_tab tab;
	std::vector<std::shared_ptr<s_cash_item>> items;
};

class CashShopDatabase : public TypesafeYamlDatabase<uint16, s_cash_item_tab>{
public:
	CashShopDatabase() : TypesafeYamlDatabase( "ITEM_CASH_DB", 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode( const ryml::NodeRef& node );

	// Additional
	std::shared_ptr<s_cash_item> findItemInTab( e_cash_shop_tab tab, t_itemid nameid );
};

extern CashShopDatabase cash_shop_db;

enum e_sale_add_result {
	SALE_ADD_SUCCESS = 0,
	SALE_ADD_FAILED = 1,
	SALE_ADD_DUPLICATE = 2
};

struct sale_item_data{
	// Data
	t_itemid nameid;
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

struct sale_item_data* sale_find_item(t_itemid nameid, bool onsale);
enum e_sale_add_result sale_add_item(t_itemid nameid, int32 count, time_t from, time_t to);
bool sale_remove_item(t_itemid nameid);
void sale_notify_login( map_session_data* sd );
#endif

#endif /* CASHSHOP_HPP */
