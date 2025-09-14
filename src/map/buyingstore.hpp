// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef BUYINGSTORE_HPP
#define BUYINGSTORE_HPP

#include <common/cbasetypes.hpp>

#include "map.hpp" //MESSAGE_SIZE

struct s_search_store_search;
class map_session_data;

#define MAX_BUYINGSTORE_SLOTS 5

struct s_buyingstore_item
{
	int32 price;
	uint16 amount;
	t_itemid nameid;
};

struct s_buyingstore
{
	struct s_buyingstore_item items[MAX_BUYINGSTORE_SLOTS];
	int32 zenylimit;
	unsigned char slots;
};

/// Autotrade entry
struct s_autotrade_entry {
	uint32 cartinventory_id; ///< Item entry id/cartinventory_id in cart_inventory table (for vending)
	uint16 amount; ///< Amount
	uint32 price; ///< Price
	uint16 index; ///< Item index in cart
	t_itemid item_id; ///< Item ID (for buyingstore)
};

/// Struct of autotrader
struct s_autotrader {
	uint32 id; ///< vendor/buyer id
	uint32 account_id; ///< Account ID
	uint32 char_id; ///< Char ID
	int32 m; ///< Map location
	uint16 x, ///< X location
		y; ///< Y location
	unsigned char sex, ///< Autotrader's sex
		dir, ///< Body direction
		head_dir, ///< Head direction
		sit; ///< Is sitting?
	char title[MESSAGE_SIZE]; ///< Store name
	uint32 limit; ///< Maximum zeny to be spent (for buyingstore)
	uint16 count; ///< Number of item in store
	struct s_autotrade_entry **entries; ///< Store details
	map_session_data *sd;
};

int8 buyingstore_setup(map_session_data* sd, unsigned char slots);
int8 buyingstore_create(map_session_data* sd, int32 zenylimit, unsigned char result, const char* storename, const struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub* itemlist, uint32 count, struct s_autotrader *at);
void buyingstore_close(map_session_data* sd);
void buyingstore_open(map_session_data* sd, uint32 account_id);
void buyingstore_trade(map_session_data* sd, uint32 account_id, uint32 buyer_id, const struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub* itemlist, uint32 count);
bool buyingstore_search(map_session_data* sd, t_itemid nameid);
bool buyingstore_searchall(map_session_data* sd, const struct s_search_store_search* s);
DBMap *buyingstore_getdb(void);
void do_final_buyingstore(void);
void do_init_buyingstore(void);

void do_init_buyingstore_autotrade( void );
void buyingstore_reopen( map_session_data* sd );
void buyingstore_update(map_session_data &sd);

#endif /* BUYINGSTORE_HPP */
