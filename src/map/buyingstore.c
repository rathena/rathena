// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"  // ARR_FIND
#include "../common/malloc.h" // aMalloc, aFree
#include "../common/showmsg.h"  // ShowWarning
#include "../common/socket.h"  // RBUF*
#include "../common/strlib.h"  // safestrncpy
#include "atcommand.h"  // msg_txt
#include "battle.h"  // battle_config.*
#include "buyingstore.h"  // struct s_buyingstore
#include "clif.h"  // clif_buyingstore_*
#include "log.h"  // log_pick_pc, log_zeny
#include "pc.h"  // struct map_session_data
#include "chrif.h"

#include <stdlib.h> // atoi

/// Struct for buyingstore entry of autotrader
struct s_autotrade_entry {
	uint16 amount;
	int price;
	uint16 item_id;
};

/// Struct of autotrader
struct s_autotrade {
	int account_id;
	int char_id;
	int buyer_id;
	int m;
	uint16 x, y;
	unsigned char sex;
	char title[MESSAGE_SIZE];
	int limit;
	uint16 count;
	struct s_autotrade_entry **entries;
	struct map_session_data *sd;
};

//Autotrader
static struct s_autotrade **autotraders; ///Autotraders Storage
static uint16 autotrader_count; ///Autotrader count
static void do_final_buyingstore_autotrade(void);

/// constants (client-side restrictions)
#define BUYINGSTORE_MAX_PRICE 99990000
#define BUYINGSTORE_MAX_AMOUNT 9999


/// failure constants for clif functions
enum e_buyingstore_failure
{
	BUYINGSTORE_CREATE               = 1,  // "Failed to open buying store."
	BUYINGSTORE_CREATE_OVERWEIGHT    = 2,  // "Total amount of then possessed items exceeds the weight limit by %d. Please re-enter."
	BUYINGSTORE_TRADE_BUYER_ZENY     = 3,  // "All items within the buy limit were purchased."
	BUYINGSTORE_TRADE_BUYER_NO_ITEMS = 4,  // "All items were purchased."
	BUYINGSTORE_TRADE_SELLER_FAILED  = 5,  // "The deal has failed."
	BUYINGSTORE_TRADE_SELLER_COUNT   = 6,  // "The trade failed, because the entered amount of item %s is higher, than the buyer is willing to buy."
	BUYINGSTORE_TRADE_SELLER_ZENY    = 7,  // "The trade failed, because the buyer is lacking required balance."
	BUYINGSTORE_CREATE_NO_INFO       = 8,  // "No sale (purchase) information available."
};


static unsigned int buyingstore_nextid = 0;
static const short buyingstore_blankslots[MAX_SLOTS] = { 0 };  // used when checking whether or not an item's card slots are blank


/// Returns unique buying store id
static unsigned int buyingstore_getuid(void)
{
	return ++buyingstore_nextid;
}


bool buyingstore_setup(struct map_session_data* sd, unsigned char slots){
	if (!battle_config.feature_buying_store || sd->state.vending || sd->state.buyingstore || sd->state.trading || slots == 0) {
		return false;
	}

	if( sd->sc.data[SC_NOCHAT] && (sd->sc.data[SC_NOCHAT]->val1&MANNER_NOROOM) )
	{// custom: mute limitation
		return false;
	}

	if( map[sd->bl.m].flag.novending )
	{// custom: no vending maps
		clif_displaymessage(sd->fd, msg_txt(sd,276)); // "You can't open a shop on this map"
		return false;
	}

	if( map_getcell(sd->bl.m, sd->bl.x, sd->bl.y, CELL_CHKNOVENDING) )
	{// custom: no vending cells
		clif_displaymessage(sd->fd, msg_txt(sd,204)); // "You can't open a shop on this cell."
		return false;
	}

	if( slots > MAX_BUYINGSTORE_SLOTS )
	{
		ShowWarning("buyingstore_setup: Requested %d slots, but server supports only %d slots.\n", (int)slots, MAX_BUYINGSTORE_SLOTS);
		slots = MAX_BUYINGSTORE_SLOTS;
	}

	sd->buyingstore.slots = slots;
	clif_buyingstore_open(sd);

	return true;
}


bool buyingstore_create(struct map_session_data* sd, int zenylimit, unsigned char result, const char* storename, const uint8* itemlist, unsigned int count)
{
	unsigned int i, weight, listidx;
	char message_sql[MESSAGE_SIZE*2];

	if( !result || count == 0 )
	{// canceled, or no items
		return false;
	}

	if( !battle_config.feature_buying_store || pc_istrading(sd) || sd->buyingstore.slots == 0 || count > sd->buyingstore.slots || zenylimit <= 0 || zenylimit > sd->status.zeny || !storename[0] )
	{// disabled or invalid input
		sd->buyingstore.slots = 0;
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return false;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to buy (give zeny)
		sd->buyingstore.slots = 0;
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return false;
	}

	if( sd->sc.data[SC_NOCHAT] && (sd->sc.data[SC_NOCHAT]->val1&MANNER_NOROOM) )
	{// custom: mute limitation
		return false;
	}

	if( map[sd->bl.m].flag.novending )
	{// custom: no vending maps
		clif_displaymessage(sd->fd, msg_txt(sd,276)); // "You can't open a shop on this map"
		return false;
	}

	if( map_getcell(sd->bl.m, sd->bl.x, sd->bl.y, CELL_CHKNOVENDING) )
	{// custom: no vending cells
		clif_displaymessage(sd->fd, msg_txt(sd,204)); // "You can't open a shop on this cell."
		return false;
	}

	weight = sd->weight;

	// check item list
	for( i = 0; i < count; i++ )
	{// itemlist: <name id>.W <amount>.W <price>.L
		unsigned short nameid, amount;
		int price, idx;
		struct item_data* id;

		nameid = RBUFW(itemlist,i*8+0);
		amount = RBUFW(itemlist,i*8+2);
		price  = RBUFL(itemlist,i*8+4);

		if( ( id = itemdb_exists(nameid) ) == NULL || amount == 0 )
		{// invalid input
			break;
		}

		if( price <= 0 || price > BUYINGSTORE_MAX_PRICE )
		{// invalid price: unlike vending, items cannot be bought at 0 Zeny
			break;
		}

		if( !id->flag.buyingstore || !itemdb_cantrade_sub(id, pc_get_group_level(sd), pc_get_group_level(sd)) || ( idx = pc_search_inventory(sd, nameid) ) == -1 )
		{// restrictions: allowed, no character-bound items and at least one must be owned
			break;
		}

		if( sd->status.inventory[idx].amount+amount > BUYINGSTORE_MAX_AMOUNT )
		{// too many items of same kind
			break;
		}

		if( i )
		{// duplicate check. as the client does this too, only malicious intent should be caught here
			ARR_FIND( 0, i, listidx, sd->buyingstore.items[listidx].nameid == nameid );
			if( listidx != i )
			{// duplicate
				ShowWarning("buyingstore_create: Found duplicate item on buying list (nameid=%hu, amount=%hu, account_id=%d, char_id=%d).\n", nameid, amount, sd->status.account_id, sd->status.char_id);
				break;
			}
		}

		weight+= id->weight*amount;
		sd->buyingstore.items[i].nameid = nameid;
		sd->buyingstore.items[i].amount = amount;
		sd->buyingstore.items[i].price  = price;
	}

	if( i != count )
	{// invalid item/amount/price
		sd->buyingstore.slots = 0;
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return false;
	}

	if( (sd->max_weight*90)/100 < weight )
	{// not able to carry all wanted items without getting overweight (90%)
		sd->buyingstore.slots = 0;
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE_OVERWEIGHT, weight);
		return false;
	}

	// success
	sd->state.buyingstore = true;
	sd->buyer_id = buyingstore_getuid();
	sd->buyingstore.zenylimit = zenylimit;
	sd->buyingstore.slots = i;  // store actual amount of items
	safestrncpy(sd->message, storename, sizeof(sd->message));

	Sql_EscapeString( mmysql_handle, message_sql, sd->message );

	if( Sql_Query( mmysql_handle, "INSERT INTO `%s`(`id`,`account_id`,`char_id`,`sex`,`map`,`x`,`y`,`title`,`limit`,`autotrade`) VALUES( %d, %d, %d, '%c', '%s', %d, %d, '%s', %d, %d );", buyingstore_db, sd->buyer_id, sd->status.account_id, sd->status.char_id, sd->status.sex == 0 ? 'F' : 'M', map[sd->bl.m].name, sd->bl.x, sd->bl.y, message_sql, sd->buyingstore.zenylimit, sd->state.autotrade ) != SQL_SUCCESS ){
		Sql_ShowDebug(mmysql_handle);
	}

	for( i = 0; i < sd->buyingstore.slots; i++ ){
		if( Sql_Query( mmysql_handle, "INSERT INTO `%s`(`buyingstore_id`,`index`,`item_id`,`amount`,`price`) VALUES( %d, %d, %d, %d, %d );", buyingstore_items_db, sd->buyer_id, i, sd->buyingstore.items[i].nameid, sd->buyingstore.items[i].amount, sd->buyingstore.items[i].price ) != SQL_SUCCESS ){
			Sql_ShowDebug(mmysql_handle);
		}
	}

	clif_buyingstore_myitemlist(sd);
	clif_buyingstore_entry(sd);

	return true;
}


void buyingstore_close(struct map_session_data* sd)
{
	if( sd->state.buyingstore )
	{
		if( !sd->state.autotrade ){
			if( 
				Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE buyingstore_id = %d;", buyingstore_items_db, sd->buyer_id ) != SQL_SUCCESS ||
				Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `id` = %d;", buyingstore_db, sd->buyer_id ) != SQL_SUCCESS
			){
				Sql_ShowDebug(mmysql_handle);
			}
		}

		// invalidate data
		sd->state.buyingstore = false;
		memset(&sd->buyingstore, 0, sizeof(sd->buyingstore));

		// notify other players
		clif_buyingstore_disappear_entry(sd);
	}
}


void buyingstore_open(struct map_session_data* sd, int account_id)
{
	struct map_session_data* pl_sd;

	if( !battle_config.feature_buying_store || pc_istrading(sd) )
	{// not allowed to sell
		return;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to sell
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return;
	}

	if( ( pl_sd = map_id2sd(account_id) ) == NULL || !pl_sd->state.buyingstore )
	{// not online or not buying
		return;
	}

	if( !searchstore_queryremote(sd, account_id) && ( sd->bl.m != pl_sd->bl.m || !check_distance_bl(&sd->bl, &pl_sd->bl, AREA_SIZE) ) )
	{// out of view range
		return;
	}

	// success
	clif_buyingstore_itemlist(sd, pl_sd);
}


void buyingstore_trade(struct map_session_data* sd, int account_id, unsigned int buyer_id, const uint8* itemlist, unsigned int count)
{
	int zeny = 0;
	unsigned int i, weight, listidx, k;
	struct map_session_data* pl_sd;

	if( count == 0 )
	{// nothing to do
		return;
	}

	if( !battle_config.feature_buying_store || pc_istrading(sd) )
	{// not allowed to sell
		clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to sell
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	if( ( pl_sd = map_id2sd(account_id) ) == NULL || !pl_sd->state.buyingstore || pl_sd->buyer_id != buyer_id )
	{// not online, not buying or not same store
		clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	if( !searchstore_queryremote(sd, account_id) && ( sd->bl.m != pl_sd->bl.m || !check_distance_bl(&sd->bl, &pl_sd->bl, AREA_SIZE) ) )
	{// out of view range
		clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	searchstore_clearremote(sd);

	if( pl_sd->status.zeny < pl_sd->buyingstore.zenylimit )
	{// buyer lost zeny in the mean time? fix the limit
		pl_sd->buyingstore.zenylimit = pl_sd->status.zeny;
	}
	weight = pl_sd->weight;

	// check item list
	for( i = 0; i < count; i++ )
	{// itemlist: <index>.W <name id>.W <amount>.W
		unsigned short nameid, amount;
		int index;

		index  = RBUFW(itemlist,i*6+0)-2;
		nameid = RBUFW(itemlist,i*6+2);
		amount = RBUFW(itemlist,i*6+4);

		if( i )
		{// duplicate check. as the client does this too, only malicious intent should be caught here
			ARR_FIND( 0, i, k, RBUFW(itemlist,k*6+0)-2 == index );
			if( k != i )
			{// duplicate
				ShowWarning("buyingstore_trade: Found duplicate item on selling list (prevnameid=%hu, prevamount=%hu, nameid=%hu, amount=%hu, account_id=%d, char_id=%d).\n",
					RBUFW(itemlist,k*6+2), RBUFW(itemlist,k*6+4), nameid, amount, sd->status.account_id, sd->status.char_id);
				clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
				return;
			}
		}

		if( index < 0 || index >= ARRAYLENGTH(sd->status.inventory) || sd->inventory_data[index] == NULL || sd->status.inventory[index].nameid != nameid || sd->status.inventory[index].amount < amount )
		{// invalid input
			clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		if( sd->status.inventory[index].expire_time || (sd->status.inventory[index].bound && !pc_can_give_bounded_items(sd)) || !itemdb_cantrade(&sd->status.inventory[index], pc_get_group_level(sd), pc_get_group_level(pl_sd)) || memcmp(sd->status.inventory[index].card, buyingstore_blankslots, sizeof(buyingstore_blankslots)) )
		{// non-tradable item
			clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		ARR_FIND( 0, pl_sd->buyingstore.slots, listidx, pl_sd->buyingstore.items[listidx].nameid == nameid );
		if( listidx == pl_sd->buyingstore.slots || pl_sd->buyingstore.items[listidx].amount == 0 )
		{// there is no such item or the buyer has already bought all of them
			clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		if( pl_sd->buyingstore.items[listidx].amount < amount )
		{// buyer does not need that much of the item
			clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_COUNT, nameid);
			return;
		}

		if( pc_checkadditem(pl_sd, nameid, amount) == CHKADDITEM_OVERAMOUNT )
		{// buyer does not have enough space for this item
			clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		if( amount*(unsigned int)sd->inventory_data[index]->weight > pl_sd->max_weight-weight )
		{// normally this is not supposed to happen, as the total weight is
		 // checked upon creation, but the buyer could have gained items
			clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}
		weight+= amount*sd->inventory_data[index]->weight;

		if( amount*pl_sd->buyingstore.items[listidx].price > pl_sd->buyingstore.zenylimit-zeny )
		{// buyer does not have enough zeny
			clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_ZENY, nameid);
			return;
		}
		zeny+= amount*pl_sd->buyingstore.items[listidx].price;
	}

	// process item list
	for( i = 0; i < count; i++ )
	{// itemlist: <index>.W <name id>.W <amount>.W
		unsigned short nameid, amount;
		int index;

		index  = RBUFW(itemlist,i*6+0)-2;
		nameid = RBUFW(itemlist,i*6+2);
		amount = RBUFW(itemlist,i*6+4);

		ARR_FIND( 0, pl_sd->buyingstore.slots, listidx, pl_sd->buyingstore.items[listidx].nameid == nameid );
		zeny = amount*pl_sd->buyingstore.items[listidx].price;

		// move item
		pc_additem(pl_sd, &sd->status.inventory[index], amount, LOG_TYPE_BUYING_STORE);
		pc_delitem(sd, index, amount, 1, 0, LOG_TYPE_BUYING_STORE);
		pl_sd->buyingstore.items[listidx].amount-= amount;

		if( pl_sd->buyingstore.items[listidx].amount > 0 ){
			if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `amount` = %d WHERE `buyingstore_id` = %d AND `index` = %d;", buyingstore_items_db, pl_sd->buyingstore.items[listidx].amount, pl_sd->buyer_id, listidx ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
			}
		}else{
			if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `buyingstore_id` = %d AND `index` = %d;", buyingstore_items_db, pl_sd->buyer_id, listidx ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
			}
		}

		// pay up
		pc_payzeny(pl_sd, zeny, LOG_TYPE_BUYING_STORE, sd);
		pc_getzeny(sd, zeny, LOG_TYPE_BUYING_STORE, pl_sd);
		pl_sd->buyingstore.zenylimit-= zeny;

		// notify clients
		clif_buyingstore_delete_item(sd, index, amount, pl_sd->buyingstore.items[listidx].price);
		clif_buyingstore_update_item(pl_sd, nameid, amount);
	}

	if( save_settings&128 ) {
		chrif_save(sd, 0);
		chrif_save(pl_sd, 0);
	}
	
	// check whether or not there is still something to buy
	ARR_FIND( 0, pl_sd->buyingstore.slots, i, pl_sd->buyingstore.items[i].amount != 0 );
	if( i == pl_sd->buyingstore.slots )
	{// everything was bought
		clif_buyingstore_trade_failed_buyer(pl_sd, BUYINGSTORE_TRADE_BUYER_NO_ITEMS);
	}
	else if( pl_sd->buyingstore.zenylimit == 0 )
	{// zeny limit reached
		clif_buyingstore_trade_failed_buyer(pl_sd, BUYINGSTORE_TRADE_BUYER_ZENY);
	}
	else
	{// continue buying
		if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `limit` = %d WHERE `id` = %d;", buyingstore_db, pl_sd->buyingstore.zenylimit, pl_sd->buyer_id ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
		}

		return;
	}

	// cannot continue buying
	buyingstore_close(pl_sd);

	// remove auto-trader
	if( pl_sd->state.autotrade )
	{
		map_quit(pl_sd);
	}
}


/// Checks if an item is being bought in given player's buying store.
bool buyingstore_search(struct map_session_data* sd, unsigned short nameid)
{
	unsigned int i;

	if( !sd->state.buyingstore )
	{// not buying
		return false;
	}

	ARR_FIND( 0, sd->buyingstore.slots, i, sd->buyingstore.items[i].nameid == nameid && sd->buyingstore.items[i].amount );
	if( i == sd->buyingstore.slots )
	{// not found
		return false;
	}

	return true;
}


/// Searches for all items in a buyingstore, that match given ids, price and possible cards.
/// @return Whether or not the search should be continued.
bool buyingstore_searchall(struct map_session_data* sd, const struct s_search_store_search* s)
{
	unsigned int i, idx;
	struct s_buyingstore_item* it;

	if( !sd->state.buyingstore )
	{// not buying
		return true;
	}

	for( idx = 0; idx < s->item_count; idx++ )
	{
		ARR_FIND( 0, sd->buyingstore.slots, i, sd->buyingstore.items[i].nameid == s->itemlist[idx] && sd->buyingstore.items[i].amount );
		if( i == sd->buyingstore.slots )
		{// not found
			continue;
		}
		it = &sd->buyingstore.items[i];

		if( s->min_price && s->min_price > (unsigned int)it->price )
		{// too low price
			continue;
		}

		if( s->max_price && s->max_price < (unsigned int)it->price )
		{// too high price
			continue;
		}

		if( s->card_count )
		{// ignore cards, as there cannot be any
			;
		}

		if( !searchstore_result(s->search_sd, sd->buyer_id, sd->status.account_id, sd->message, it->nameid, it->amount, it->price, buyingstore_blankslots, 0) )
		{// result set full
			return false;
		}
	}

	return true;
}

/** Open buyingstore for Autotrader
* @param sd Player as autotrader
*/
void buyingstore_reopen( struct map_session_data* sd ){
	// Ready to open buyingstore for this char
	if ( sd && autotrader_count > 0 && autotraders){
		uint16 i;
		uint8 *data, *p;
		uint16 j, count;

		ARR_FIND(0,autotrader_count,i,autotraders[i] && autotraders[i]->char_id == sd->status.char_id);
		if (i >= autotrader_count) {
			return;
		}
		
		// Init buyingstore data for autotrader
		CREATE(data, uint8, autotraders[i]->count * 8);

		for (j = 0, p = data, count = autotraders[i]->count; j < autotraders[i]->count; j++) {
			struct s_autotrade_entry *entry = autotraders[i]->entries[j];
			uint16 *item_id = (uint16*)(p + 0);
			uint16 *amount = (uint16*)(p + 2);
			uint32 *price = (uint32*)(p + 4);

			*item_id = entry->item_id;
			*amount = entry->amount;
			*price = entry->price;

			p += 8;
		}

		// Open the buyingstore again
		if( buyingstore_setup( sd, (unsigned char)autotraders[i]->count ) &&
			buyingstore_create( sd, autotraders[i]->limit, 1, autotraders[i]->title, data, autotraders[i]->count ) )
		{
			ShowInfo("Loaded autotrade buyingstore data for '"CL_WHITE"%s"CL_RESET"' with '"CL_WHITE"%d"CL_RESET"' items at "CL_WHITE"%s (%d,%d)"CL_RESET"\n",
				sd->status.name,count,mapindex_id2name(sd->mapindex),sd->bl.x,sd->bl.y);

			// Set him to autotrade
			if (Sql_Query( mmysql_handle, "UPDATE `%s` SET `autotrade` = 1 WHERE `id` = %d;",
				buyingstore_db, sd->buyer_id ) != SQL_SUCCESS )
			{
				Sql_ShowDebug( mmysql_handle );
			}

			// Make him look perfect
			unit_setdir(&sd->bl,battle_config.feature_autotrade_direction);

			if( battle_config.feature_autotrade_sit )
				pc_setsit(sd);
		}else{
			// Failed to open the buyingstore, set him offline
			ShowWarning("Failed to load autotrade buyingstore data for '"CL_WHITE"%s"CL_RESET"' with '"CL_WHITE"%d"CL_RESET"' items\n", sd->status.name, count );

			map_quit( sd );
		}

		aFree(data);

		//If the last autotrade is loaded, clear autotraders [Cydh]
		if (i+1 >= autotrader_count)
			do_final_buyingstore_autotrade();
	}
}

/**
* Initializing autotraders from table
*/
void do_init_buyingstore_autotrade( void ) {
	if(battle_config.feature_autotrade) {
		uint16 i, items = 0;
		autotrader_count = 0;

		// Get autotrader from table. `map`, `x`, and `y`, aren't used here
		// Just read player that has data at buyingstore_items [Cydh]
		if (Sql_Query(mmysql_handle,
			"SELECT `id`, `account_id`, `char_id`, `sex`, `title`, `limit` "
			"FROM `%s` "
			"WHERE `autotrade` = 1 AND `limit` > 0 AND (SELECT COUNT(`buyingstore_id`) FROM `%s` WHERE `buyingstore_id` = `id`) > 0;",
			buyingstore_db, buyingstore_items_db ) != SQL_SUCCESS )
		{
			Sql_ShowDebug(mmysql_handle);
			return;
		}

		if( (autotrader_count = (uint32)Sql_NumRows(mmysql_handle)) > 0 ){
			// Init autotraders
			CREATE(autotraders, struct s_autotrade *, autotrader_count);

			if (autotraders == NULL) { //This is shouldn't happen [Cydh]
				ShowError("Failed to initialize buyingstore autotraders!\n");
				Sql_FreeResult(mmysql_handle);
				return;
			}

			// Init each autotrader data
			i = 0;
			while (SQL_SUCCESS == Sql_NextRow(mmysql_handle) && i < autotrader_count) {
				size_t len;
				char* data;

				CREATE(autotraders[i], struct s_autotrade, 1);

				Sql_GetData(mmysql_handle, 0, &data, NULL); autotraders[i]->buyer_id = atoi(data);
				Sql_GetData(mmysql_handle, 1, &data, NULL); autotraders[i]->account_id = atoi(data);
				Sql_GetData(mmysql_handle, 2, &data, NULL); autotraders[i]->char_id = atoi(data);
				Sql_GetData(mmysql_handle, 3, &data, NULL); autotraders[i]->sex = (data[0] == 'F') ? 0 : 1;
				Sql_GetData(mmysql_handle, 4, &data, &len); safestrncpy(autotraders[i]->title, data, min(len + 1, MESSAGE_SIZE));
				Sql_GetData(mmysql_handle, 5, &data, NULL); autotraders[i]->limit = atoi(data);
				autotraders[i]->count = 0;

				// initialize player
				CREATE(autotraders[i]->sd, struct map_session_data, 1);
			
				pc_setnewpc(autotraders[i]->sd, autotraders[i]->account_id, autotraders[i]->char_id, 0, gettick(), autotraders[i]->sex, 0);
			
				autotraders[i]->sd->state.autotrade = 1;
				chrif_authreq(autotraders[i]->sd, true);
				i++;
			}
			Sql_FreeResult(mmysql_handle);

			//Init items on vending list each autotrader
			for (i = 0; i < autotrader_count; i++){
				struct s_autotrade *at = NULL;
				uint16 j;

				if (autotraders[i] == NULL)
					continue;
				at = autotraders[i];

				if (SQL_ERROR == Sql_Query(mmysql_handle,
					"SELECT `item_id`, `amount`, `price` "
					"FROM `%s` "
					"WHERE `buyingstore_id` = %d "
					"ORDER BY `index` ASC;", buyingstore_items_db, at->buyer_id ) )
				{
					Sql_ShowDebug(mmysql_handle);
					continue;
				}

				if (!(at->count = (uint32)Sql_NumRows(mmysql_handle))) {
					map_quit(at->sd);
					continue;
				}
			
				//Init the list
				CREATE(at->entries, struct s_autotrade_entry *,at->count);

				//Add the item into list
				j = 0;
				while (SQL_SUCCESS == Sql_NextRow(mmysql_handle) && j < at->count) {
					char* data;
					CREATE(at->entries[j], struct s_autotrade_entry, 1);

					Sql_GetData(mmysql_handle, 0, &data, NULL); at->entries[j]->item_id = atoi(data);
					Sql_GetData(mmysql_handle, 1, &data, NULL); at->entries[j]->amount = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, NULL); at->entries[j]->price = atoi(data);
					j++;
				}
				items += j;
				Sql_FreeResult(mmysql_handle);
			}

			ShowStatus("Done loading '"CL_WHITE"%d"CL_RESET"' autotraders with '"CL_WHITE"%d"CL_RESET"' items.\n", autotrader_count, items);
		}
	}

	// Everything is loaded fine, their entries will be reinserted once they are loaded
	if (Sql_Query( mmysql_handle, "DELETE FROM `%s`;", buyingstore_db ) != SQL_SUCCESS ||
		Sql_Query( mmysql_handle, "DELETE FROM `%s`;", buyingstore_items_db ) != SQL_SUCCESS)
	{
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
* Clear all autotraders
* @author [Cydh]
*/
void do_final_buyingstore_autotrade(void) {
	if (autotrader_count && autotraders){
		uint16 i = 0;
		while (i < autotrader_count) { //Free the autotrader
			if (autotraders[i] == NULL)
				continue;
			if (autotraders[i]->count) {
				uint16 j = 0;
				while (j < autotraders[i]->count) { //Free the autotrade entries
					if (autotraders[i]->entries == NULL)
						continue;
					if (autotraders[i]->entries[j])
						aFree(autotraders[i]->entries[j]);
					j++;
				}
				aFree(autotraders[i]->entries);
			}
			aFree(autotraders[i]);
			i++;
		}
		aFree(autotraders);
		autotrader_count = 0;
	}
}
