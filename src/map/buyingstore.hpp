// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _BUYINGSTORE_HPP_
#define _BUYINGSTORE_HPP_

#include "../common/cbasetypes.hpp"

#include "map.hpp" //MESSAGE_SIZE

struct s_search_store_search;
struct map_session_data;

#define MAX_BUYINGSTORE_SLOTS 5

struct s_buyingstore_item
{
	int price;
	unsigned short amount;
	unsigned short nameid;
};

struct s_buyingstore
{
	struct s_buyingstore_item items[MAX_BUYINGSTORE_SLOTS];
	int zenylimit;
	unsigned char slots;
};

/// Autotrade entry
struct s_autotrade_entry {
	uint32 cartinventory_id; ///< Item entry id/cartinventory_id in cart_inventory table (for vending)
	uint16 amount; ///< Amount
	uint32 price; ///< Price
	uint16 index; ///< Item index in cart
	uint32 item_id; ///< Item ID (for buyingstore)
};

/// Struct of autotrader
struct s_autotrader {
	uint32 id; ///< vendor/buyer id
	uint32 account_id; ///< Account ID
	uint32 char_id; ///< Char ID
	int m; ///< Map location
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
	struct map_session_data *sd;
};

int8 buyingstore_setup(struct map_session_data* sd, unsigned char slots);
int8 buyingstore_create(struct map_session_data* sd, int zenylimit, unsigned char result, const char* storename, const uint8* itemlist, unsigned int count, struct s_autotrader *at);
void buyingstore_close(struct map_session_data* sd);
void buyingstore_open(struct map_session_data* sd, uint32 account_id);
void buyingstore_trade(struct map_session_data* sd, uint32 account_id, unsigned int buyer_id, const uint8* itemlist, unsigned int count);
bool buyingstore_search(struct map_session_data* sd, unsigned short nameid);
bool buyingstore_searchall(struct map_session_data* sd, const struct s_search_store_search* s);
DBMap *buyingstore_getdb(void);
void do_final_buyingstore(void);
void do_init_buyingstore(void);

void do_init_buyingstore_autotrade( void );
void buyingstore_reopen( struct map_session_data* sd );

#endif /* _BUYINGSTORE_HPP_ */
