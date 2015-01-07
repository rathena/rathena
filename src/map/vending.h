// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_VENDING_H_
#define	_VENDING_H_

#include "../common/cbasetypes.h"
//#include "map.h"
struct map_session_data;
struct s_search_store_search;

struct s_vending {
	short index; /// cart index (return item data)
	short amount; ///amout of the item for vending
	unsigned int value; ///at wich price
};

/// Struct for vending entry of autotrader
struct s_autotrade_entry {
	uint16 cartinventory_id; ///< Item entry id/cartinventory_id in cart_inventory table (for vending)
	uint16 amount; ///< Amount
	uint32 price; ///< Price
	uint16 index; ///< Item index in cart
	uint32 item_id; ///< Item ID (for buyingstore)
};

/// Struct of autotrader
struct s_autotrader {
	uint16 id; ///< vendor/buyer id
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

DBMap * vending_getdb();
void do_final_vending(void);
void do_init_vending(void);
void do_init_vending_autotrade( void );
 
void vending_reopen( struct map_session_data* sd );
void vending_closevending(struct map_session_data* sd);
int8 vending_openvending(struct map_session_data* sd, const char* message, const uint8* data, int count, struct s_autotrader *at);
void vending_vendinglistreq(struct map_session_data* sd, int id);
void vending_purchasereq(struct map_session_data* sd, int aid, int uid, const uint8* data, int count);
bool vending_search(struct map_session_data* sd, unsigned short nameid);
bool vending_searchall(struct map_session_data* sd, const struct s_search_store_search* s);

#endif /* _VENDING_H_ */
