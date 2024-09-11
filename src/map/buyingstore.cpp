// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "buyingstore.hpp"  // struct s_buyingstore

#include <cstdlib> // atoi

#include <common/db.hpp>  // ARR_FIND
#include <common/malloc.hpp> // aMalloc, aFree
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>  // ShowWarning
#include <common/socket.hpp>  // RBUF*
#include <common/strlib.hpp>  // safestrncpy
#include <common/timer.hpp>  // gettick

#include "atcommand.hpp"  // msg_txt
#include "battle.hpp"  // battle_config.*
#include "chrif.hpp"
#include "clif.hpp"  // clif_buyingstore_*
#include "log.hpp"  // log_pick_pc, log_zeny
#include "npc.hpp"
#include "pc.hpp"  // map_session_data

//Autotrader
static DBMap *buyingstore_autotrader_db; /// Holds autotrader info: char_id -> struct s_autotrader
static void buyingstore_autotrader_remove(struct s_autotrader *at, bool remove);
static int buyingstore_autotrader_free(DBKey key, DBData *data, va_list ap);

/// constants (client-side restrictions)
#define BUYINGSTORE_MAX_PRICE 99990000
#define BUYINGSTORE_MAX_AMOUNT 9999

static DBMap *buyingstore_db;

DBMap *buyingstore_getdb(void) {
	return buyingstore_db;
}

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
static const t_itemid buyingstore_blankslots[MAX_SLOTS] = { 0 };  // used when checking whether or not an item's card slots are blank


/// Returns unique buying store id
static unsigned int buyingstore_getuid(void)
{
	return ++buyingstore_nextid;
}

/**
* Attempt to setup buying store fast check before create new one
* @param sd
* @param slots Number of item on the list
* @return 0 If success, 1 - Cannot open, 2 - Manner penalty, 3 - Mapflag restiction, 4 - Cell restriction
*/
int8 buyingstore_setup(map_session_data* sd, unsigned char slots){
	nullpo_retr(1, sd);

	if (!battle_config.feature_buying_store || sd->state.vending || sd->state.buyingstore || sd->state.trading || slots == 0) {
		return 1;
	}

	if( sd->sc.getSCE(SC_NOCHAT) && (sd->sc.getSCE(SC_NOCHAT)->val1&MANNER_NOROOM) )
	{// custom: mute limitation
		return 2;
	}

	if( map_getmapflag(sd->bl.m, MF_NOBUYINGSTORE) )
	{// custom: no buyingstore maps
		clif_displaymessage(sd->fd, msg_txt(sd,276)); // "You can't open a shop on this map"
		return 3;
	}

	if( map_getcell(sd->bl.m, sd->bl.x, sd->bl.y, CELL_CHKNOBUYINGSTORE) )
	{// custom: no buyingstore cells
		clif_displaymessage(sd->fd, msg_txt(sd,204)); // "You can't open a shop on this cell."
		return 4;
	}

	if( slots > MAX_BUYINGSTORE_SLOTS )
	{
		ShowWarning("buyingstore_setup: Requested %d slots, but server supports only %d slots.\n", (int)slots, MAX_BUYINGSTORE_SLOTS);
		slots = MAX_BUYINGSTORE_SLOTS;
	}

	sd->buyingstore.slots = slots;
	clif_buyingstore_open(sd);

	return 0;
}

/**
* Attempt to create new buying store
* @param sd
* @param zenylimit
* @param result
* @param storename
* @param *itemlist { <nameid>.W, <amount>.W, <price>.L }*
* @param count Number of item on the itemlist
* @param at Autotrader info, or nullptr if requetsed not from autotrade persistance
* @return 0 If success, 1 - Cannot open, 2 - Manner penalty, 3 - Mapflag restiction, 4 - Cell restriction, 5 - Invalid count/result, 6 - Cannot give item, 7 - Will be overweight
*/
int8 buyingstore_create( map_session_data* sd, int zenylimit, unsigned char result, const char* storename, const struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub* itemlist, unsigned int count, struct s_autotrader *at ){
	unsigned int i, weight, listidx;
	char message_sql[MESSAGE_SIZE*2];
	StringBuf buf;

	nullpo_retr(1, sd);

	if( !result || count == 0 )
	{// canceled, or no items
		return 5;
	}

	if( !battle_config.feature_buying_store || pc_istrading(sd) || sd->buyingstore.slots == 0 || count > sd->buyingstore.slots || zenylimit <= 0 || zenylimit > sd->status.zeny || !storename[0] )
	{// disabled or invalid input
		sd->buyingstore.slots = 0;
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return 1;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to buy (give zeny)
		sd->buyingstore.slots = 0;
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return 6;
	}

	if( sd->sc.getSCE(SC_NOCHAT) && (sd->sc.getSCE(SC_NOCHAT)->val1&MANNER_NOROOM) )
	{// custom: mute limitation
		return 2;
	}

	if( map_getmapflag(sd->bl.m, MF_NOBUYINGSTORE) )
	{// custom: no buyingstore maps
		clif_displaymessage(sd->fd, msg_txt(sd,276)); // "You can't open a shop on this map"
		return 3;
	}

	if( map_getcell(sd->bl.m, sd->bl.x, sd->bl.y, CELL_CHKNOBUYINGSTORE) )
	{// custom: no buyingstore cells
		clif_displaymessage(sd->fd, msg_txt(sd,204)); // "You can't open a shop on this cell."
		return 4;
	}

	weight = sd->weight;

	// check item list
	for( i = 0; i < count; i++ ){
		const struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub *item = &itemlist[i];
		std::shared_ptr<item_data> id = item_db.find(item->itemId);

		// invalid input
		if( id == nullptr || item->amount == 0 ){	
			break;
		}

		// invalid price: unlike vending, items cannot be bought at 0 Zeny
		if( item->price <= 0 || item->price > BUYINGSTORE_MAX_PRICE ){
			break;
		}

		// restrictions: allowed and no character-bound items
		if( !id->flag.buyingstore || !itemdb_cantrade_sub( id.get(), pc_get_group_level( sd ), pc_get_group_level( sd ) ) ){ 
			break;
		}

		int idx = pc_search_inventory( sd, item->itemId );

		// At least one must be owned
		if( idx < 0 ){
			break;
		}

		// too many items of same kind
		if( sd->inventory.u.items_inventory[idx].amount + item->amount > BUYINGSTORE_MAX_AMOUNT ){
			break;
		}

		// duplicate check. as the client does this too, only malicious intent should be caught here
		if( i ){
			ARR_FIND( 0, i, listidx, sd->buyingstore.items[listidx].nameid == item->itemId );

			// duplicate
			if( listidx != i ){
				ShowWarning( "buyingstore_create: Found duplicate item on buying list (nameid=%u, amount=%hu, account_id=%d, char_id=%d).\n", item->itemId, item->amount, sd->status.account_id, sd->status.char_id );
				break;
			}
		}

		weight+= id->weight*item->amount;
		sd->buyingstore.items[i].nameid = item->itemId;
		sd->buyingstore.items[i].amount = item->amount;
		sd->buyingstore.items[i].price  = item->price;
	}

	if( i != count )
	{// invalid item/amount/price
		sd->buyingstore.slots = 0;
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return 5;
	}

	if( (sd->max_weight*90)/100 < weight )
	{// not able to carry all wanted items without getting overweight (90%)
		sd->buyingstore.slots = 0;
		clif_buyingstore_open_failed(sd, BUYINGSTORE_CREATE_OVERWEIGHT, weight);
		return 7;
	}

	// success
	sd->state.buyingstore = true;
	sd->buyer_id = buyingstore_getuid();
	sd->buyingstore.zenylimit = zenylimit;
	sd->buyingstore.slots = i;  // store actual amount of items
	safestrncpy(sd->message, storename, sizeof(sd->message));

	Sql_EscapeString( mmysql_handle, message_sql, sd->message );

	if( Sql_Query( mmysql_handle, "INSERT INTO `%s`(`id`, `account_id`, `char_id`, `sex`, `map`, `x`, `y`, `title`, `limit`, `autotrade`, `body_direction`, `head_direction`, `sit`) "
		"VALUES( %d, %d, %d, '%c', '%s', %d, %d, '%s', %d, %d, '%d', '%d', '%d' );",
		buyingstores_table, sd->buyer_id, sd->status.account_id, sd->status.char_id, sd->status.sex == 0 ? 'F' : 'M', map_getmapdata(sd->bl.m)->name, sd->bl.x, sd->bl.y, message_sql, sd->buyingstore.zenylimit, sd->state.autotrade, at ? at->dir : sd->ud.dir, at ? at->head_dir : sd->head_dir, at ? at->sit : pc_issit(sd) ) != SQL_SUCCESS ){
		Sql_ShowDebug(mmysql_handle);
	}

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`buyingstore_id`,`index`,`item_id`,`amount`,`price`) VALUES", buyingstore_items_table);
	for (i = 0; i < sd->buyingstore.slots; i++){
		StringBuf_Printf(&buf, "(%d,%d,%u,%d,%d)", sd->buyer_id, i, sd->buyingstore.items[i].nameid, sd->buyingstore.items[i].amount, sd->buyingstore.items[i].price);
		if (i < sd->buyingstore.slots-1)
			StringBuf_AppendStr(&buf, ",");
	}
	if (SQL_ERROR == Sql_QueryStr(mmysql_handle, StringBuf_Value(&buf)))
		Sql_ShowDebug(mmysql_handle);
	StringBuf_Destroy(&buf);

	clif_buyingstore_myitemlist( *sd );
	clif_buyingstore_entry( *sd );
	idb_put(buyingstore_db, sd->status.char_id, sd);

	return 0;
}

/**
* Close buying store and clear buying store data from tables
* @param sd
*/
void buyingstore_close(map_session_data* sd) {
	nullpo_retv(sd);

	if( sd->state.buyingstore ) {
		if( 
			Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE buyingstore_id = %d;", buyingstore_items_table, sd->buyer_id ) != SQL_SUCCESS ||
			Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `id` = %d;", buyingstores_table, sd->buyer_id ) != SQL_SUCCESS
		) {
			Sql_ShowDebug(mmysql_handle);
		}

		sd->state.buyingstore = false;
		sd->buyer_id = 0;
		memset(&sd->buyingstore, 0, sizeof(sd->buyingstore));
		idb_remove(buyingstore_db, sd->status.char_id);

		// notify other players
		clif_buyingstore_disappear_entry( *sd );
	}
}

/**
* Open buying store from buyer
* @param sd Player
* @param account_id Buyer account ID
*/
void buyingstore_open(map_session_data* sd, uint32 account_id)
{
	map_session_data* pl_sd;

	nullpo_retv(sd);

	if( !battle_config.feature_buying_store || pc_istrading(sd) )
	{// not allowed to sell
		return;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to sell
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return;
	}

	if( ( pl_sd = map_id2sd(account_id) ) == nullptr || !pl_sd->state.buyingstore )
	{// not online or not buying
		return;
	}

	if( !searchstore_queryremote(*sd, account_id) && ( sd->bl.m != pl_sd->bl.m || !check_distance_bl(&sd->bl, &pl_sd->bl, AREA_SIZE) ) )
	{// out of view range
		return;
	}

	// success
	clif_buyingstore_itemlist( *sd, *pl_sd );
}

/**
* Start transaction
* @param sd Player/Seller
* @param account_id Buyer account ID
* @param *itemlist List of sold items { <index>.W, <nameid>.W, <amount>.W }*
* @param count Number of item on the itemlist
*/
void buyingstore_trade( map_session_data* sd, uint32 account_id, unsigned int buyer_id, const struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub* itemlist, unsigned int count ){
	int zeny = 0;
	unsigned int weight;
	map_session_data* pl_sd;

	nullpo_retv(sd);

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

	if( ( pl_sd = map_id2sd(account_id) ) == nullptr || !pl_sd->state.buyingstore || pl_sd->buyer_id != buyer_id )
	{// not online, not buying or not same store
		clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	if( !searchstore_queryremote(*sd, account_id) && ( sd->bl.m != pl_sd->bl.m || !check_distance_bl(&sd->bl, &pl_sd->bl, AREA_SIZE) ) )
	{// out of view range
		clif_buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	searchstore_clearremote(*sd);

	// buyer lost zeny in the mean time? fix the limit
	if( pl_sd->status.zeny < pl_sd->buyingstore.zenylimit ){
		pl_sd->buyingstore.zenylimit = pl_sd->status.zeny;
	}
	weight = pl_sd->weight;

	// check item list
	for( int i = 0; i < count; i++ ){
		const struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub* item = &itemlist[i];

		// duplicate check. as the client does this too, only malicious intent should be caught here
		for( int k = 0; k < i; k++ ){
			// duplicate
			if( itemlist[k].index == item->index && k != i ){
				ShowWarning( "buyingstore_trade: Found duplicate item on selling list (prevnameid=%u, prevamount=%hu, nameid=%u, amount=%hu, account_id=%d, char_id=%d).\n", itemlist[k].itemId, itemlist[k].amount, item->itemId, item->amount, sd->status.account_id, sd->status.char_id );
				clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_FAILED, item->itemId );
				return;
			}
		}

		int index = item->index - 2; // TODO: clif::server_index

		// invalid input
		if( index < 0 || index >= ARRAYLENGTH( sd->inventory.u.items_inventory ) || sd->inventory_data[index] == nullptr || sd->inventory.u.items_inventory[index].nameid != item->itemId || sd->inventory.u.items_inventory[index].amount < item->amount ){
			clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_FAILED, item->itemId );
			return;
		}

		// non-tradable item
		if( sd->inventory.u.items_inventory[index].expire_time || ( sd->inventory.u.items_inventory[index].bound && !pc_can_give_bounded_items( sd ) ) || !itemdb_cantrade( &sd->inventory.u.items_inventory[index], pc_get_group_level( sd ), pc_get_group_level( pl_sd ) ) || memcmp( sd->inventory.u.items_inventory[index].card, buyingstore_blankslots, sizeof( buyingstore_blankslots ) ) ){
			clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_FAILED, item->itemId );
			return;
		}

		int listidx;

		ARR_FIND( 0, pl_sd->buyingstore.slots, listidx, pl_sd->buyingstore.items[listidx].nameid == item->itemId );

		// there is no such item or the buyer has already bought all of them
		if( listidx == pl_sd->buyingstore.slots || pl_sd->buyingstore.items[listidx].amount == 0 ){
			clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_FAILED, item->itemId );
			return;
		}

		// buyer does not need that much of the item
		if( pl_sd->buyingstore.items[listidx].amount < item->amount ){
			clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_COUNT, item->itemId );
			return;
		}

		// buyer does not have enough space for this item
		if( pc_checkadditem( pl_sd, item->itemId, item->amount ) == CHKADDITEM_OVERAMOUNT ){
			clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_FAILED, item->itemId );
			return;
		}

		// normally this is not supposed to happen, as the total weight is
		// checked upon creation, but the buyer could have gained items
		if( item->amount * (unsigned int)sd->inventory_data[index]->weight > pl_sd->max_weight - weight ){
			clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_FAILED, item->itemId );
			return;
		}

		weight += item->amount * sd->inventory_data[index]->weight;

		// buyer does not have enough zeny
		if( item->amount * pl_sd->buyingstore.items[listidx].price > pl_sd->buyingstore.zenylimit - zeny ){
			clif_buyingstore_trade_failed_seller( sd, BUYINGSTORE_TRADE_SELLER_ZENY, item->itemId );
			return;
		}

		zeny += item->amount * pl_sd->buyingstore.items[listidx].price;
	}

	// process item list
	for( int i = 0; i < count; i++ ){
		const struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub* item = &itemlist[i];
		int listidx;

		ARR_FIND( 0, pl_sd->buyingstore.slots, listidx, pl_sd->buyingstore.items[listidx].nameid == item->itemId );
		zeny = item->amount * pl_sd->buyingstore.items[listidx].price;

		int index = item->index - 2; // TODO: clif::server_index

		// move item
		pc_additem(pl_sd, &sd->inventory.u.items_inventory[index], item->amount, LOG_TYPE_BUYING_STORE);
		pc_delitem(sd, index, item->amount, 1, 0, LOG_TYPE_BUYING_STORE);
		pl_sd->buyingstore.items[listidx].amount -= item->amount;

		if( pl_sd->buyingstore.items[listidx].amount > 0 ){
			if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `amount` = %d WHERE `buyingstore_id` = %d AND `index` = %d;", buyingstore_items_table, pl_sd->buyingstore.items[listidx].amount, pl_sd->buyer_id, listidx ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
			}
		}else{
			if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `buyingstore_id` = %d AND `index` = %d;", buyingstore_items_table, pl_sd->buyer_id, listidx ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
			}
		}

		// pay up
		pc_payzeny(pl_sd, zeny, LOG_TYPE_BUYING_STORE, sd->status.char_id);
		pc_getzeny(sd, zeny, LOG_TYPE_BUYING_STORE, pl_sd->status.char_id);
		pl_sd->buyingstore.zenylimit-= zeny;

		// notify clients
		clif_buyingstore_delete_item(sd, index, item->amount, pl_sd->buyingstore.items[listidx].price);
		clif_buyingstore_update_item(pl_sd, item->itemId, item->amount, sd->status.char_id, zeny);
	}

	if( save_settings&CHARSAVE_VENDING ) {
		chrif_save(sd, CSAVE_NORMAL|CSAVE_INVENTORY);
		chrif_save(pl_sd, CSAVE_NORMAL|CSAVE_INVENTORY);
	}
	
	// check whether or not there is still something to buy
	int i;
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
		if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `limit` = %d WHERE `id` = %d;", buyingstores_table, pl_sd->buyingstore.zenylimit, pl_sd->buyer_id ) != SQL_SUCCESS ){
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
bool buyingstore_search(map_session_data* sd, t_itemid nameid)
{
	unsigned int i;

	nullpo_ret(sd);

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
bool buyingstore_searchall(map_session_data* sd, const struct s_search_store_search* s)
{
	unsigned int i, idx;
	struct s_buyingstore_item* it;

	nullpo_ret(sd);

	if( !sd->state.buyingstore )
	{// not buying
		return true;
	}

	for( idx = 0; idx < s->item_count; idx++ )
	{
		ARR_FIND( 0, sd->buyingstore.slots, i, sd->buyingstore.items[i].nameid == s->itemlist[idx].itemId && sd->buyingstore.items[i].amount );
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

		// Check if the result set is full
		if( s->search_sd->searchstore.items.size() >= (unsigned int)battle_config.searchstore_maxresults ){
			return false;
		}

		std::shared_ptr<s_search_store_info_item> ssitem = std::make_shared<s_search_store_info_item>();

		ssitem->store_id = sd->buyer_id;
		ssitem->account_id = sd->status.account_id;
		safestrncpy( ssitem->store_name, sd->message, sizeof( ssitem->store_name ) );
		ssitem->nameid = it->nameid;
		ssitem->amount = it->amount;
		ssitem->price = it->price;
		for( int j = 0; j < MAX_SLOTS; j++ ){
			ssitem->card[j] = 0;
		}
		ssitem->refine = 0;
		ssitem->enchantgrade = 0;

		s->search_sd->searchstore.items.push_back( ssitem );
	}

	return true;
}

/**
* Open buyingstore for Autotrader
* @param sd Player as autotrader
*/
void buyingstore_reopen( map_session_data* sd ){
	struct s_autotrader *at = nullptr;
	int8 fail = -1;

	nullpo_retv(sd);

	// Ready to open buyingstore for this char
	if ((at = (struct s_autotrader *)uidb_get(buyingstore_autotrader_db, sd->status.char_id)) && at->count && at->entries) {
		struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub* data;

		// Init buyingstore data for autotrader
		CREATE(data, struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub, at->count );

		for( int j = 0; j < at->count; j++) {
			data[j].itemId = at->entries[j]->item_id;
			data[j].amount = at->entries[j]->amount;
			data[j].price = at->entries[j]->price;
		}

		sd->state.autotrade = 1;

		// Make sure abort all NPCs
		npc_event_dequeue(sd);
		pc_cleareventtimer(sd);

		// Open the buyingstore again
		if( (fail = buyingstore_setup( sd, (unsigned char)at->count )) == 0 &&
			(fail = buyingstore_create( sd, at->limit, 1, at->title, data, at->count, at )) == 0 )
		{
			// Make buyer look perfect
			pc_setdir(sd, at->dir, at->head_dir);
			clif_changed_dir(sd->bl, AREA_WOS);
			if( at->sit ) {
				pc_setsit(sd);
				skill_sit(sd, 1);
				clif_sitting(sd->bl);
			}

			// Immediate save
			chrif_save(sd, CSAVE_AUTOTRADE);

			ShowInfo("Buyingstore loaded for '" CL_WHITE "%s" CL_RESET "' with '" CL_WHITE "%d" CL_RESET "' items at " CL_WHITE "%s (%d,%d)" CL_RESET "\n",
				sd->status.name, at->count, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y);
		}
		aFree(data);
	}

	if (at) {
		buyingstore_autotrader_remove(at, true);
		if (db_size(buyingstore_autotrader_db) == 0)
			buyingstore_autotrader_db->clear(buyingstore_autotrader_db, buyingstore_autotrader_free);
	}

	if (fail != 0) {
		ShowError("buyingstore_reopen: (Error:%d) Load failed for autotrader '" CL_WHITE "%s" CL_RESET "' (CID=%/AID=%d)\n", fail, sd->status.name, sd->status.char_id, sd->status.account_id);
		map_quit(sd);
	}
}

/**
* Initializing autotraders from table
*/
void do_init_buyingstore_autotrade( void ) {
	if(battle_config.feature_autotrade) {
		if (Sql_Query(mmysql_handle,
			"SELECT `id`, `account_id`, `char_id`, `sex`, `title`, `limit`, `body_direction`, `head_direction`, `sit` "
			"FROM `%s` "
			"WHERE `autotrade` = 1 AND `limit` > 0 AND (SELECT COUNT(`buyingstore_id`) FROM `%s` WHERE `buyingstore_id` = `id`) > 0 "
			"ORDER BY `id`;",
			buyingstores_table, buyingstore_items_table ) != SQL_SUCCESS )
		{
			Sql_ShowDebug(mmysql_handle);
			return;
		}

		if( Sql_NumRows(mmysql_handle) > 0 ) {
			uint16 items = 0;
			DBIterator *iter = nullptr;
			struct s_autotrader *at = nullptr;

			// Init each autotrader data
			while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
				size_t len;
				char* data;

				at = nullptr;
				CREATE(at, struct s_autotrader, 1);
				Sql_GetData(mmysql_handle, 0, &data, nullptr); at->id = atoi(data);
				Sql_GetData(mmysql_handle, 1, &data, nullptr); at->account_id = atoi(data);
				Sql_GetData(mmysql_handle, 2, &data, nullptr); at->char_id = atoi(data);
				Sql_GetData(mmysql_handle, 3, &data, nullptr); at->sex = (data[0] == 'F') ? SEX_FEMALE : SEX_MALE;
				Sql_GetData(mmysql_handle, 4, &data, &len); safestrncpy(at->title, data, zmin(len + 1, MESSAGE_SIZE));
				Sql_GetData(mmysql_handle, 5, &data, nullptr); at->limit = atoi(data);
				Sql_GetData(mmysql_handle, 6, &data, nullptr); at->dir = atoi(data);
				Sql_GetData(mmysql_handle, 7, &data, nullptr); at->head_dir = atoi(data);
				Sql_GetData(mmysql_handle, 8, &data, nullptr); at->sit = atoi(data);
				at->count = 0;

				if (battle_config.feature_autotrade_direction >= 0)
					at->dir = battle_config.feature_autotrade_direction;
				if (battle_config.feature_autotrade_head_direction >= 0)
					at->head_dir = battle_config.feature_autotrade_head_direction;
				if (battle_config.feature_autotrade_sit >= 0)
					at->sit = battle_config.feature_autotrade_sit;

				// initialize player
				CREATE(at->sd, map_session_data, 1); // TODO: Dont use Memory Manager allocation anymore and rely on the C++ container
				new (at->sd) map_session_data();
				pc_setnewpc(at->sd, at->account_id, at->char_id, 0, gettick(), at->sex, 0);
				at->sd->state.autotrade = 1|4;
				if (battle_config.autotrade_monsterignore)
					at->sd->state.block_action |= PCBLOCK_IMMUNE;
				else
					at->sd->state.block_action &= ~PCBLOCK_IMMUNE;
				chrif_authreq(at->sd, true);
				uidb_put(buyingstore_autotrader_db, at->char_id, at);
			}
			Sql_FreeResult(mmysql_handle);
			
			// Init items for each autotraders
			iter = db_iterator(buyingstore_autotrader_db);
			for (at = (struct s_autotrader *)dbi_first(iter); dbi_exists(iter); at = (struct s_autotrader *)dbi_next(iter)) {
				uint16 j = 0;

				if (SQL_ERROR == Sql_Query(mmysql_handle,
					"SELECT `item_id`, `amount`, `price` "
					"FROM `%s` "
					"WHERE `buyingstore_id` = %d "
					"ORDER BY `index` ASC;",
					buyingstore_items_table, at->id ) )
				{
					Sql_ShowDebug(mmysql_handle);
					continue;
				}

				if (!(at->count = (uint16)Sql_NumRows(mmysql_handle))) {
					map_quit(at->sd);
					buyingstore_autotrader_remove(at, true);
					continue;
				}
			
				//Init the list
				CREATE(at->entries, struct s_autotrade_entry *,at->count);

				//Add the item into list
				j = 0;
				while (SQL_SUCCESS == Sql_NextRow(mmysql_handle) && j < at->count) {
					char *data;
					CREATE(at->entries[j], struct s_autotrade_entry, 1);
					Sql_GetData(mmysql_handle, 0, &data, nullptr); at->entries[j]->item_id = strtoul(data, nullptr, 10);
					Sql_GetData(mmysql_handle, 1, &data, nullptr); at->entries[j]->amount = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, nullptr); at->entries[j]->price = atoi(data);
					j++;
				}
				items += j;
				Sql_FreeResult(mmysql_handle);
			}
			dbi_destroy(iter);

			ShowStatus("Done loading '" CL_WHITE "%d" CL_RESET "' buyingstore autotraders with '" CL_WHITE "%d" CL_RESET "' items.\n", db_size(buyingstore_autotrader_db), items);
		}
	}

	// Everything is loaded fine, their entries will be reinserted once they are loaded
	if (Sql_Query( mmysql_handle, "DELETE FROM `%s`;", buyingstores_table ) != SQL_SUCCESS ||
		Sql_Query( mmysql_handle, "DELETE FROM `%s`;", buyingstore_items_table ) != SQL_SUCCESS)
	{
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
 * Remove an autotrader's data
 * @param at Autotrader
 * @param remove If true will removes from buyingstore_autotrader_db
 **/
static void buyingstore_autotrader_remove(struct s_autotrader *at, bool remove) {
	nullpo_retv(at);
	if (at->count && at->entries) {
		uint16 i = 0;
		for (i = 0; i < at->count; i++) {
			if (at->entries[i])
				aFree(at->entries[i]);
		}
		aFree(at->entries);
	}
	if (remove)
		uidb_remove(buyingstore_autotrader_db, at->char_id);
	aFree(at);
}

/**
* Clear all autotraders
* @author [Cydh]
*/
static int buyingstore_autotrader_free(DBKey key, DBData *data, va_list ap) {
	struct s_autotrader *at = (struct s_autotrader *)db_data2ptr(data);
	if (at)
		buyingstore_autotrader_remove(at, false);
	return 0;
}

/**
* Update buyer location
* @param sd: Player's session data
*/
void buyingstore_update(map_session_data &sd)
{
	if (Sql_Query(mmysql_handle, "UPDATE `%s` SET `map` = '%s', `x` = '%d', `y` = '%d', `body_direction` = '%d', `head_direction` = '%d', `sit` = '%d', `autotrade` = '%d' WHERE `id` = '%d'",
		buyingstores_table, map_getmapdata(sd.bl.m)->name, sd.bl.x, sd.bl.y, sd.ud.dir, sd.head_dir, pc_issit(&sd), sd.state.autotrade,
		sd.buyer_id
	) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
 * Initialise the buyingstore module
 * called in map::do_init
 */
void do_final_buyingstore(void) {
	db_destroy(buyingstore_db);
	buyingstore_autotrader_db->destroy(buyingstore_autotrader_db, buyingstore_autotrader_free);
}

/**
 * Destory the buyingstore module
 * called in map::do_final
 */
void do_init_buyingstore(void) {
	buyingstore_db = idb_alloc(DB_OPT_BASE);
	buyingstore_autotrader_db = uidb_alloc(DB_OPT_BASE);
	buyingstore_nextid = 0;
}
