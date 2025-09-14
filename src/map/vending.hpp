// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_VENDING_HPP_
#define	_VENDING_HPP_

#include <common/cbasetypes.hpp>
#include <common/db.hpp>
#include <common/mmo.hpp>

class map_session_data;
struct s_search_store_search;
struct s_autotrader;

struct s_vending {
	int16 index; /// cart index (return item data)
	int16 amount; ///amout of the item for vending
	uint32 value; ///at which price
};

DBMap * vending_getdb();
void do_final_vending(void);
void do_init_vending(void);
void do_init_vending_autotrade( void );
 
void vending_reopen( map_session_data& sd );
void vending_closevending(map_session_data* sd);
int8 vending_openvending( map_session_data& sd, const char* message, const uint8* data, int32 count, struct s_autotrader *at );
void vending_vendinglistreq(map_session_data* sd, int32 id);
void vending_purchasereq(map_session_data* sd, int32 aid, int32 uid, const uint8* data, int32 count);
bool vending_search(map_session_data* sd, t_itemid nameid);
bool vending_searchall(map_session_data* sd, const struct s_search_store_search* s);
void vending_update(map_session_data &sd);

#endif /* _VENDING_HPP_ */
