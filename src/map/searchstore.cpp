// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "searchstore.hpp"  // struct s_search_store_info

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"  // aMalloc, aRealloc, aFree
#include "../common/showmsg.hpp"  // ShowError, ShowWarning
#include "../common/strlib.hpp"  // safestrncpy

#include "battle.hpp"  // battle_config.*
#include "clif.hpp"  // clif_open_search_store_info, clif_search_store_info_*
#include "pc.hpp"  // struct map_session_data

/// Failure constants for clif functions
enum e_searchstore_failure
{
	SSI_FAILED_NOTHING_SEARCH_ITEM         = 0,  // "No matching stores were found."
	SSI_FAILED_OVER_MAXCOUNT               = 1,  // "There are too many results. Please enter more detailed search term."
	SSI_FAILED_SEARCH_CNT                  = 2,  // "You cannot search anymore."
	SSI_FAILED_LIMIT_SEARCH_TIME           = 3,  // "You cannot search yet."
	SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE = 4,  // "No sale (purchase) information available."
};

/// Search type constants
enum e_searchstore_searchtype
{
	SEARCHTYPE_VENDING      = 0,
	SEARCHTYPE_BUYING_STORE = 1,
};

/// Search effect constants
enum e_searchstore_effecttype
{
	EFFECTTYPE_NORMAL = 0,
	EFFECTTYPE_CASH   = 1,
	EFFECTTYPE_MAX
};

/// Type for shop search function
typedef bool (*searchstore_search_t)(struct map_session_data* sd, t_itemid nameid);
typedef bool (*searchstore_searchall_t)(struct map_session_data* sd, const struct s_search_store_search* s);

/**
 * Retrieves search function by type.
 * @param type : type of search to conduct
 * @return : search type
 */
static searchstore_search_t searchstore_getsearchfunc(unsigned char type)
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return &vending_search;
		case SEARCHTYPE_BUYING_STORE: return &buyingstore_search;
	}

	return NULL;
}

/**
 * Retrieves search-all function by type.
 * @param type : type of search to conduct
 * @return : search type
 */
static searchstore_searchall_t searchstore_getsearchallfunc(unsigned char type)
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return &vending_searchall;
		case SEARCHTYPE_BUYING_STORE: return &buyingstore_searchall;
	}

	return NULL;
}

/**
 * Checks if the player has a store by type.
 * @param sd : player requesting
 * @param type : type of search to conduct
 * @return : store type
 */
static bool searchstore_hasstore(struct map_session_data* sd, unsigned char type)
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return sd->state.vending;
		case SEARCHTYPE_BUYING_STORE: return sd->state.buyingstore;
	}

	return false;
}

/**
 * Returns player's store ID by type.
 * @param sd : player requesting
 * @param type : type of search to conduct
 * @return : store ID
 */
static int searchstore_getstoreid(struct map_session_data* sd, unsigned char type)
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return sd->vender_id;
		case SEARCHTYPE_BUYING_STORE: return sd->buyer_id;
	}

	return 0;
}

/**
 * Send request to open Search Store.
 * @param sd : player requesting
 * @param uses : uses left to open
 * @param effect : shop type
 * @return : true : opened, false : failed to open
 */
bool searchstore_open(struct map_session_data* sd, unsigned int uses, unsigned short effect)
{
	if( !battle_config.feature_search_stores || sd->searchstore.open )
		return false;

	if( !uses || effect >= EFFECTTYPE_MAX ) // invalid input
		return false;

	sd->searchstore.open   = true;
	sd->searchstore.uses   = uses;
	sd->searchstore.effect = effect;

	clif_open_search_store_info(sd);

	return true;
}

/**
 * Query and present the results for the item.
 * @param sd : player requesting
 * @param type : shop type
 * @param min_price : minimum zeny price
 * @param max_price : maximum zeny price
 * @param itemlist : list with stored item results
 * @param item_count : amount of items in itemlist
 * @param cardlist : list with stored cards (cards attached to items)
 * @param card_count : amount of items in cardlist
 */
void searchstore_query(struct map_session_data* sd, unsigned char type, unsigned int min_price, unsigned int max_price, const struct PACKET_CZ_SEARCH_STORE_INFO_item* itemlist, unsigned int item_count, const struct PACKET_CZ_SEARCH_STORE_INFO_item* cardlist, unsigned int card_count)
{
	unsigned int i;
	struct map_session_data* pl_sd;
	struct DBIterator *iter;
	struct s_search_store_search s;
	searchstore_searchall_t store_searchall;
	time_t querytime;

	if( !battle_config.feature_search_stores )
		return;

	if( !sd->searchstore.open )
		return;

	if( ( store_searchall = searchstore_getsearchallfunc(type) ) == NULL ) {
		ShowError("searchstore_query: Unknown search type %u (account_id=%d).\n", (unsigned int)type, sd->bl.id);
		return;
	}

	time(&querytime);

	if( sd->searchstore.nextquerytime > querytime ) {
		clif_search_store_info_failed(sd, SSI_FAILED_LIMIT_SEARCH_TIME);
		return;
	}

	if( !sd->searchstore.uses ) {
		clif_search_store_info_failed(sd, SSI_FAILED_SEARCH_CNT);
		return;
	}

	// validate lists
	for( i = 0; i < item_count; i++ ) {
		if( !itemdb_exists(itemlist[i].itemId) ) {
			ShowWarning("searchstore_query: Client resolved item %u is not known.\n", itemlist[i].itemId);
			clif_search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);
			return;
		}
	}
	for( i = 0; i < card_count; i++ ) {
		if( !itemdb_exists(cardlist[i].itemId) ) {
			ShowWarning("searchstore_query: Client resolved card %u is not known.\n", cardlist[i].itemId);
			clif_search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);
			return;
		}
	}

	if( max_price < min_price )
		SWAP(min_price, max_price);

	sd->searchstore.uses--;
	sd->searchstore.type = type;
	sd->searchstore.nextquerytime = querytime+battle_config.searchstore_querydelay;

	// drop previous results
	searchstore_clear(sd);

	// search
	s.search_sd  = sd;
	s.itemlist   = itemlist;
	s.cardlist   = cardlist;
	s.item_count = item_count;
	s.card_count = card_count;
	s.min_price  = min_price;
	s.max_price  = max_price;
	iter         = db_iterator((type == SEARCHTYPE_VENDING) ? vending_getdb() : buyingstore_getdb());

	for( pl_sd = (struct map_session_data*)dbi_first(iter); dbi_exists(iter);  pl_sd = (struct map_session_data*)dbi_next(iter) ) {
		if( sd == pl_sd ) // skip own shop, if any
			continue;

		if( !store_searchall(pl_sd, &s) ) { // exceeded result size
			clif_search_store_info_failed(sd, SSI_FAILED_OVER_MAXCOUNT);
			break;
		}
	}

	dbi_destroy(iter);

	if( !sd->searchstore.items.empty() ) {
		// present results
		clif_search_store_info_ack(sd);

		// one page displayed
		sd->searchstore.pages++;
	} else {
		// cleanup
		searchstore_clear(sd);

		// update uses
		clif_search_store_info_ack(sd);

		// notify of failure
		clif_search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);
	}
}

/**
 * Checks whether or not more results are available for the client.
 * @param sd : player requesting
 * @return : true : more items to search, false : no more items
 */
bool searchstore_querynext(struct map_session_data* sd)
{
	if( !sd->searchstore.items.empty() && ( sd->searchstore.items.size()-1 )/SEARCHSTORE_RESULTS_PER_PAGE > sd->searchstore.pages )
		return true;

	return false;
}

/**
 * Get and display the results for the next page.
 * @param sd : player requesting
 */
void searchstore_next(struct map_session_data* sd)
{
	if( !battle_config.feature_search_stores || !sd->searchstore.open || sd->searchstore.items.size() <= sd->searchstore.pages*SEARCHSTORE_RESULTS_PER_PAGE ) // nothing (more) to display
		return;

	// present results
	clif_search_store_info_ack(sd);

	// one more page displayed
	sd->searchstore.pages++;
}

/**
 * Prepare to clear information for closing of window.
 * @param sd : player requesting
 */
void searchstore_clear(struct map_session_data* sd)
{
	searchstore_clearremote(sd);

	sd->searchstore.items.clear();
	sd->searchstore.pages = 0;
}

/**
 * Close the Search Store window.
 * @param sd : player requesting
 */
void searchstore_close(struct map_session_data* sd)
{
	if( sd->searchstore.open ) {
		searchstore_clear(sd);

		sd->searchstore.uses = 0;
		sd->searchstore.open = false;
	}
}

/**
 * Process the actions (click) for the Search Store window.
 * @param sd : player requesting
 * @param account_id : account ID of owner's shop
 * @param store_id : store ID created by client
 * @param nameid : item being searched
 */
void searchstore_click(struct map_session_data* sd, uint32 account_id, int store_id, t_itemid nameid)
{
	unsigned int i;
	struct map_session_data* pl_sd;
	searchstore_search_t store_search;

	if( !battle_config.feature_search_stores || !sd->searchstore.open || sd->searchstore.items.empty() )
		return;

	searchstore_clearremote(sd);

	ARR_FIND( 0, sd->searchstore.items.size(), i, sd->searchstore.items[i]->store_id == store_id && sd->searchstore.items[i]->account_id == account_id && sd->searchstore.items[i]->nameid == nameid );
	if( i == sd->searchstore.items.size() ) { // no such result, crafted
		ShowWarning("searchstore_click: Received request with item %u of account %d, which is not part of current result set (account_id=%d, char_id=%d).\n", nameid, account_id, sd->bl.id, sd->status.char_id);
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	if( ( pl_sd = map_id2sd(account_id) ) == NULL ) { // no longer online
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	if( !searchstore_hasstore(pl_sd, sd->searchstore.type) || searchstore_getstoreid(pl_sd, sd->searchstore.type) != store_id ) { // no longer vending/buying or not same shop
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	store_search = searchstore_getsearchfunc(sd->searchstore.type);

	if( !store_search(pl_sd, nameid) ) {// item no longer being sold/bought
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	switch( sd->searchstore.effect ) {
		case EFFECTTYPE_NORMAL:
			// display coords
			if( sd->bl.m != pl_sd->bl.m ) // not on same map, wipe previous marker
				clif_search_store_info_click_ack(sd, -1, -1);
			else
				clif_search_store_info_click_ack(sd, pl_sd->bl.x, pl_sd->bl.y);
			break;
		case EFFECTTYPE_CASH:
			// open remotely
			// to bypass range checks
			sd->searchstore.remote_id = account_id;

			switch( sd->searchstore.type ) {
				case SEARCHTYPE_VENDING:      vending_vendinglistreq(sd, account_id); break;
				case SEARCHTYPE_BUYING_STORE: buyingstore_open(sd, account_id);       break;
			}
			break;
		default:
			// unknown
			ShowError("searchstore_click: Unknown search store effect %u (account_id=%d).\n", (unsigned int)sd->searchstore.effect, sd->bl.id);
	}
}

/**
 * Checks whether or not sd has opened account_id's shop remotely.
 * @param sd : player requesting
 * @param account_id : account ID of owner's shop
 * @return : true : shop opened, false : shop not opened
 */
bool searchstore_queryremote(struct map_session_data* sd, uint32 account_id)
{
	return (bool)( sd->searchstore.open && !sd->searchstore.items.empty() && sd->searchstore.remote_id == account_id );
}

/**
 * Removes range-check bypassing for remotely opened stores.
 * @param sd : player requesting
 */
void searchstore_clearremote(struct map_session_data* sd)
{
	sd->searchstore.remote_id = 0;
}
