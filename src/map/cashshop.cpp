// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cashshop.hpp"

#include <stdlib.h> // atoi
#include <string.h> // memset

#include "../common/cbasetypes.hpp" // uint16, uint32
#include "../common/malloc.hpp" // CREATE, RECREATE, aFree
#include "../common/showmsg.hpp" // ShowWarning, ShowStatus

#include "clif.hpp"
#include "log.hpp"
#include "pc.hpp" // s_map_session_data
#include "pet.hpp" // pet_create_egg

struct cash_item_db cash_shop_items[CASHSHOP_TAB_MAX];
#if PACKETVER_SUPPORTS_SALES
struct sale_item_db sale_items;
#endif
bool cash_shop_defined = false;

extern char item_cash_table[32];
extern char item_cash2_table[32];
extern char sales_table[32];

/*
 * Reads one line from database and assigns it to RAM.
 * return
 *  0 = failure
 *  1 = success
 */
static bool cashshop_parse_dbrow(char* fields[], int columns, int current) {
	uint16 tab = atoi(fields[0]);
	unsigned short nameid = atoi(fields[1]);
	uint32 price = atoi(fields[2]);
	int j;
	struct cash_item_data* cid;

	if( !itemdb_exists( nameid ) ){
		ShowWarning( "cashshop_parse_dbrow: Invalid ID %hu in line '%d', skipping...\n", nameid, current );
		return 0;
	}

	if( tab >= CASHSHOP_TAB_MAX ){
		ShowWarning( "cashshop_parse_dbrow: Invalid tab %d in line '%d', skipping...\n", tab, current );
		return 0;
	}else if( price < 1 ){
		ShowWarning( "cashshop_parse_dbrow: Invalid price %d in line '%d', skipping...\n", price, current );
		return 0;
	}

	ARR_FIND( 0, cash_shop_items[tab].count, j, nameid == cash_shop_items[tab].item[j]->nameid );

	if( j == cash_shop_items[tab].count ){
		RECREATE( cash_shop_items[tab].item, struct cash_item_data *, ++cash_shop_items[tab].count );
		CREATE( cash_shop_items[tab].item[ cash_shop_items[tab].count - 1], struct cash_item_data, 1 );
		cid = cash_shop_items[tab].item[ cash_shop_items[tab].count - 1];
	}else{
		cid = cash_shop_items[tab].item[j];
	}

	cid->nameid = nameid;
	cid->price = price;
	cash_shop_defined = true;

	return 1;
}

/*
 * Reads database from TXT format,
 * parses lines and sends them to parse_dbrow.
 */
static void cashshop_read_db_txt( void ){
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
	};
	int fi;

	for( fi = 0; fi < ARRAYLENGTH( dbsubpath ); ++fi ){
		uint8 n1 = (uint8)(strlen(db_path)+strlen(dbsubpath[fi])+1);
		uint8 n2 = (uint8)(strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[fi])+1);
		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);

		if(fi==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[fi]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[fi]);
		}
		else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[fi]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[fi]);
		}

		sv_readdb(dbsubpath2, "item_cash_db.txt", ',', 3, 3, -1, &cashshop_parse_dbrow, fi > 0);

		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}
}

/*
 * Reads database from SQL format,
 * parses line and sends them to parse_dbrow.
 */
static int cashshop_read_db_sql( void ){
	const char* cash_db_name[] = { item_cash_table, item_cash2_table };
	int fi;

	for( fi = 0; fi < ARRAYLENGTH( cash_db_name ); ++fi ){
		uint32 lines = 0, count = 0;

		if( SQL_ERROR == Sql_Query( mmysql_handle, "SELECT `tab`, `item_id`, `price` FROM `%s`", cash_db_name[fi] ) ){
			Sql_ShowDebug( mmysql_handle );
			continue;
		}


		while( SQL_SUCCESS == Sql_NextRow( mmysql_handle ) ){
			char* str[3];
			char dummy[256] = "";
			int i;

			++lines;

			for( i = 0; i < 3; ++i ){
				Sql_GetData( mmysql_handle, i, &str[i], NULL );

				if( str[i] == NULL ){
					str[i] = dummy;
				}
			}

			if( !cashshop_parse_dbrow( str, 3, lines ) ) {
				ShowError("cashshop_read_db_sql: Cannot process table '%s' at line '%d', skipping...\n", cash_db_name[fi], lines);
				continue;
			}

			++count;
		}

		Sql_FreeResult( mmysql_handle );

		ShowStatus( "Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, cash_db_name[fi] );
	}

	return 0;
}

#if PACKETVER_SUPPORTS_SALES
static bool sale_parse_dbrow( char* fields[], int columns, int current ){
	unsigned short nameid = atoi(fields[0]);
	int start = atoi(fields[1]), end = atoi(fields[2]), amount = atoi(fields[3]), i;
	time_t now = time(NULL);
	struct sale_item_data* sale_item = NULL;

	if( !itemdb_exists(nameid) ){
		ShowWarning( "sale_parse_dbrow: Invalid ID %hu in line '%d', skipping...\n", nameid, current );
		return false;
	}

	ARR_FIND( 0, cash_shop_items[CASHSHOP_TAB_SALE].count, i, cash_shop_items[CASHSHOP_TAB_SALE].item[i]->nameid == nameid );

	if( i == cash_shop_items[CASHSHOP_TAB_SALE].count ){
		ShowWarning( "sale_parse_dbrow: ID %hu is not registered in the limited tab in line '%d', skipping...\n", nameid, current );
		return false;
	}

	// Check if the end is after the start
	if( start >= end ){
		ShowWarning( "sale_parse_dbrow: Sale for item %hu was ignored, because the timespan was not correct.\n", nameid );
		return false;
	}

	// Check if it is already in the past
	if( end < now ){
		ShowWarning( "sale_parse_dbrow: An outdated sale for item %hu was ignored.\n", nameid );
		return false;
	}

	// Check if there is already an entry
	sale_item = sale_find_item(nameid,false);

	if( sale_item == NULL ){
		RECREATE(sale_items.item, struct sale_item_data *, ++sale_items.count);
		CREATE(sale_items.item[sale_items.count - 1], struct sale_item_data, 1);
		sale_item = sale_items.item[sale_items.count - 1];
	}

	sale_item->nameid = nameid;
	sale_item->start = start;
	sale_item->end = end;
	sale_item->amount = amount;
	sale_item->timer_start = INVALID_TIMER;
	sale_item->timer_end = INVALID_TIMER;

	return true;
}

static void sale_read_db_sql( void ){
	uint32 lines = 0, count = 0;

	if( SQL_ERROR == Sql_Query( mmysql_handle, "SELECT `nameid`, UNIX_TIMESTAMP(`start`), UNIX_TIMESTAMP(`end`), `amount` FROM `%s` WHERE `end` > now()", sales_table ) ){
		Sql_ShowDebug(mmysql_handle);
		return;
	}

	while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ){
		char* str[4];
		char dummy[256] = "";
		int i;

		lines++;

		for( i = 0; i < 4; i++ ){
			Sql_GetData( mmysql_handle, i, &str[i], NULL );

			if( str[i] == NULL ){
				str[i] = dummy;
			}
		}

		if( !sale_parse_dbrow( str, 4, lines ) ){
			ShowError( "sale_read_db_sql: Cannot process table '%s' at line '%d', skipping...\n", sales_table, lines );
			continue;
		}

		count++;
	}

	Sql_FreeResult(mmysql_handle);

	ShowStatus( "Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, sales_table );
}

static TIMER_FUNC(sale_end_timer){
	struct sale_item_data* sale_item = (struct sale_item_data*)data;

	// Remove the timer so the sale end is not sent out again
	delete_timer( sale_item->timer_end, sale_end_timer );
	sale_item->timer_end = INVALID_TIMER;
	
	clif_sale_end( sale_item, NULL, ALL_CLIENT );

	sale_remove_item( sale_item->nameid );

	return 1;
}

static TIMER_FUNC(sale_start_timer){
	struct sale_item_data* sale_item = (struct sale_item_data*)data;

	clif_sale_start( sale_item, NULL, ALL_CLIENT );
	clif_sale_amount( sale_item, NULL, ALL_CLIENT );

	// Clear the start timer
	if( sale_item->timer_start != INVALID_TIMER ){
		delete_timer( sale_item->timer_start, sale_start_timer );
		sale_item->timer_start = INVALID_TIMER;
	}

	// Init sale end
	sale_item->timer_end = add_timer( gettick() + (unsigned int)( sale_item->end - time(NULL) ) * 1000, sale_end_timer, 0, (intptr_t)sale_item );

	return 1;
}

enum e_sale_add_result sale_add_item( uint16 nameid, int32 count, time_t from, time_t to ){
	int i;
	struct sale_item_data* sale_item;

	// Check if the item exists in the sales tab
	ARR_FIND( 0, cash_shop_items[CASHSHOP_TAB_SALE].count, i, cash_shop_items[CASHSHOP_TAB_SALE].item[i]->nameid == nameid );

	// Item does not exist in the sales tab
	if( i == cash_shop_items[CASHSHOP_TAB_SALE].count ){
		return SALE_ADD_FAILED;
	}

	// Adding a sale in the past is not possible
	if( from < time(NULL) ){
		return SALE_ADD_FAILED;
	}

	// The end has to be after the start
	if( from >= to ){
		return SALE_ADD_FAILED;
	}

	// Amount has to be positive - this should be limited from the client too
	if( count == 0 ){
		return SALE_ADD_FAILED;
	}

	// Check if a sale of this item already exists
	if( sale_find_item(nameid, false) ){
		return SALE_ADD_DUPLICATE;
	}
	
	if( SQL_ERROR == Sql_Query(mmysql_handle, "INSERT INTO `%s`(`nameid`,`start`,`end`,`amount`) VALUES ( '%d', FROM_UNIXTIME(%d), FROM_UNIXTIME(%d), '%d' )", sales_table, nameid, (uint32)from, (uint32)to, count) ){
		Sql_ShowDebug(mmysql_handle);
		return SALE_ADD_FAILED;
	}

	RECREATE(sale_items.item, struct sale_item_data *, ++sale_items.count);
	CREATE(sale_items.item[sale_items.count - 1], struct sale_item_data, 1);
	sale_item = sale_items.item[sale_items.count - 1];

	sale_item->nameid = nameid;
	sale_item->start = from;
	sale_item->end = to;
	sale_item->amount = count;
	sale_item->timer_start = add_timer( gettick() + (unsigned int)(from - time(NULL)) * 1000, sale_start_timer, 0, (intptr_t)sale_item );
	sale_item->timer_end = INVALID_TIMER;

	return SALE_ADD_SUCCESS;
}

bool sale_remove_item( uint16 nameid ){
	struct sale_item_data* sale_item;
	int i;

	// Check if there is an entry for this item id
	sale_item = sale_find_item(nameid, false);

	if( sale_item == NULL ){
		return false;
	}

	// Delete it from the database
	if( SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `nameid` = '%d'", sales_table, nameid ) ){
		Sql_ShowDebug(mmysql_handle);
		return false;
	}

	if( sale_item->timer_start != INVALID_TIMER ){
		delete_timer(sale_item->timer_start, sale_start_timer);
		sale_item->timer_start = INVALID_TIMER;
	}

	// Check if the sale is currently running
	if( sale_item->timer_end != INVALID_TIMER ){
		delete_timer(sale_item->timer_end, sale_end_timer);
		sale_item->timer_end = INVALID_TIMER;

		// Notify all clients that the sale has ended
		clif_sale_end(sale_item, NULL, ALL_CLIENT);
	}

	// Find the original pointer in the array
	ARR_FIND( 0, sale_items.count, i, sale_items.item[i] == sale_item );

	// Is there still any entry left?
	if( --sale_items.count > 0 ){
		// fill the hole by moving the rest
		for( ; i < sale_items.count; i++ ){
			memcpy( sale_items.item[i], sale_items.item[i + 1], sizeof(struct sale_item_data) );
		}

		aFree(sale_items.item[i]);

		RECREATE(sale_items.item, struct sale_item_data *, sale_items.count);
	}else{
		aFree(sale_items.item[0]);
		aFree(sale_items.item);
		sale_items.item = NULL;
	}

	return true;
}

struct sale_item_data* sale_find_item( uint16 nameid, bool onsale ){
	int i;
	struct sale_item_data* sale_item;
	time_t now = time(NULL);

	ARR_FIND( 0, sale_items.count, i, sale_items.item[i]->nameid == nameid );

	// No item with the specified item id was found
	if( i == sale_items.count ){
		return NULL;
	}

	sale_item = sale_items.item[i];

	// No need to check any further
	if( !onsale ){
		return sale_item;
	}

	// The sale is in the future
	if( sale_items.item[i]->start > now ){
		return NULL;
	}

	// The sale was in the past
	if( sale_items.item[i]->end < now ){
		return NULL;
	}

	// The amount has been used up already
	if( sale_items.item[i]->amount == 0 ){
		return NULL;
	}

	// Return the sale item
	return sale_items.item[i];
}

void sale_notify_login( struct map_session_data* sd ){
	int i;

	for( i = 0; i < sale_items.count; i++ ){
		if( sale_items.item[i]->timer_end != INVALID_TIMER ){
			clif_sale_start( sale_items.item[i], &sd->bl, SELF );
			clif_sale_amount( sale_items.item[i], &sd->bl, SELF );
		}
	}
}
#endif

/*
 * Determines whether to read TXT or SQL database
 * based on 'db_use_sqldbs' in conf/map_athena.conf.
 */
static void cashshop_read_db( void ){
#if PACKETVER_SUPPORTS_SALES
	int i;
	time_t now = time(NULL);
#endif

	if( db_use_sqldbs ){
		cashshop_read_db_sql();
	} else {
		cashshop_read_db_txt();
	}

#if PACKETVER_SUPPORTS_SALES
	sale_read_db_sql();

	// Clean outdated sales
	if( SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `end` < FROM_UNIXTIME(%d)", sales_table, (uint32)now ) ){
		Sql_ShowDebug(mmysql_handle);
	}

	// Init next sale start, if there is any
	for( i = 0; i < sale_items.count; i++ ){
		struct sale_item_data* it = sale_items.item[i];

		if( it->start > now ){
			it->timer_start = add_timer( gettick() + (unsigned int)( it->start - time(NULL) ) * 1000, sale_start_timer, 0, (intptr_t)it );
		}else{
			sale_start_timer( 0, gettick(), 0, (intptr_t)it );
		}
	}
#endif
}

/** Attempts to purchase a cashshop item from the list.
 * Checks if the transaction is valid and if the user has enough inventory space to receive the item.
 * If yes, take cashpoints and give items; else return clif_error.
 * @param sd Player that request to buy item(s)
 * @param kafrapoints
 * @param n Count of item list
 * @param item_list Array of item ID
 * @return true: success, false: fail
 */
bool cashshop_buylist( struct map_session_data* sd, uint32 kafrapoints, int n, uint16* item_list ){
	uint32 totalcash = 0;
	uint32 totalweight = 0;
	int i,new_;
	item_data *id;
#if PACKETVER_SUPPORTS_SALES
	struct sale_item_data* sale = NULL;
#endif

	if( sd == NULL || item_list == NULL || !cash_shop_defined){
		clif_cashshop_result( sd, 0, CASHSHOP_RESULT_ERROR_UNKNOWN );
		return false;
	}else if( sd->state.trading ){
		clif_cashshop_result( sd, 0, CASHSHOP_RESULT_ERROR_PC_STATE );
		return false;
	}

	new_ = 0;

	for( i = 0; i < n; ++i ){
		unsigned short nameid = *( item_list + i * 5 );
		uint32 quantity = *( item_list + i * 5 + 2 );
		uint8 tab = (uint8)*( item_list + i * 5 + 4 );
		int j;

		if( tab >= CASHSHOP_TAB_MAX ){
			clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKNOWN );
			return false;
		}

		ARR_FIND( 0, cash_shop_items[tab].count, j, nameid == cash_shop_items[tab].item[j]->nameid || nameid == itemdb_viewid(cash_shop_items[tab].item[j]->nameid) );

		if( j == cash_shop_items[tab].count ){
			clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKONWN_ITEM );
			return false;
		}

		nameid = *( item_list + i * 5 ) = cash_shop_items[tab].item[j]->nameid; //item_avail replacement
		id = itemdb_exists(nameid);

		if( !id ){
			clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKONWN_ITEM );
			return false;
		}

		if( quantity > 99 ){
			// Client blocks buying more than 99 items of the same type at the same time, this means someone forged a packet with a higher quantity
			clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKNOWN );
			return false;
		}

#if PACKETVER_SUPPORTS_SALES
		if( tab == CASHSHOP_TAB_SALE ){
			sale = sale_find_item( nameid, true );

			if( sale == NULL ){
				// Client tried to buy an item from sale that was not even on sale
				clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKNOWN );
				return false;
			}

			if( sale->amount < quantity ){
				// Client tried to buy a higher quantity than is available
				clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKNOWN );
				// Maybe he did not get refreshed in time -> do it now
				clif_sale_amount( sale, &sd->bl, SELF );
				return false;
			}
		}
#endif

		switch( pc_checkadditem( sd, nameid, quantity ) ){
			case CHKADDITEM_EXIST:
				break;

			case CHKADDITEM_NEW:
				new_ += id->inventorySlotNeeded(quantity);
				break;

			case CHKADDITEM_OVERAMOUNT:
				clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_OVER_PRODUCT_TOTAL_CNT );
				return false;
		}

		totalcash += cash_shop_items[tab].item[j]->price * quantity;
		totalweight += itemdb_weight( nameid ) * quantity;
	}

	if( ( totalweight + sd->weight ) > sd->max_weight ){
		clif_cashshop_result( sd, 0, CASHSHOP_RESULT_ERROR_INVENTORY_WEIGHT );
		return false;
	}else if( pc_inventoryblank( sd ) < new_ ){
		clif_cashshop_result( sd, 0, CASHSHOP_RESULT_ERROR_INVENTORY_ITEMCNT );
		return false;
	}

	if(pc_paycash( sd, totalcash, kafrapoints, LOG_TYPE_CASH ) <= 0){
		clif_cashshop_result( sd, 0, CASHSHOP_RESULT_ERROR_SHORTTAGE_CASH );
		return false;
	}

	for( i = 0; i < n; ++i ){
		unsigned short nameid = *( item_list + i * 5 );
		uint32 quantity = *( item_list + i * 5 + 2 );
#if PACKETVER_SUPPORTS_SALES
		uint16 tab = *(item_list + i * 5 + 4);
#endif
		struct item_data *id = itemdb_search(nameid);

		if (!id)
			continue;

		if (!pet_create_egg(sd, nameid)) {
			unsigned short get_amt = quantity, j;

			if (id->flag.guid || !itemdb_isstackable2(id))
				get_amt = 1;

			for (j = 0; j < quantity; j += get_amt) {
				struct item item_tmp = { 0 };

				item_tmp.nameid = nameid;
				item_tmp.identify = 1;

				switch( pc_additem( sd, &item_tmp, get_amt, LOG_TYPE_CASH ) ){
					case ADDITEM_OVERWEIGHT:
						clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_INVENTORY_WEIGHT );
						return false;
					case ADDITEM_OVERITEM:
						clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_INVENTORY_ITEMCNT );
						return false;
					case ADDITEM_OVERAMOUNT:
						clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_OVER_PRODUCT_TOTAL_CNT );
						return false;
					case ADDITEM_STACKLIMIT:
						clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_RUNE_OVERCOUNT );
						return false;
				}

#if PACKETVER_SUPPORTS_SALES
				if( tab == CASHSHOP_TAB_SALE ){
					uint32 new_amount = sale->amount - get_amt;

					if( new_amount == 0 ){
						sale_remove_item(sale->nameid);
					}else{
						if( SQL_ERROR == Sql_Query( mmysql_handle, "UPDATE `%s` SET `amount` = '%d' WHERE `nameid` = '%d'", sales_table, new_amount, nameid ) ){
							Sql_ShowDebug(mmysql_handle);
						}

						sale->amount = new_amount;

						clif_sale_amount(sale, NULL, ALL_CLIENT);
					}
				}
#endif
			}
		}
	}

	clif_cashshop_result( sd, 0, CASHSHOP_RESULT_SUCCESS ); //Doesn't show any message?
	return true;
}

/*
 * Reloads cashshop database by destroying it and reading it again.
 */
void cashshop_reloaddb( void ){
	do_final_cashshop();
	do_init_cashshop();
}

/*
 * Destroys cashshop class.
 * Closes all and cleanup.
 */
void do_final_cashshop( void ){
	int tab, i;

	for( tab = CASHSHOP_TAB_NEW; tab < CASHSHOP_TAB_MAX; tab++ ){
		for( i = 0; i < cash_shop_items[tab].count; i++ ){
			aFree( cash_shop_items[tab].item[i] );
		}
		aFree( cash_shop_items[tab].item );
	}
	memset( cash_shop_items, 0, sizeof( cash_shop_items ) );

#if PACKETVER_SUPPORTS_SALES
	if( sale_items.count > 0 ){
		for( i = 0; i < sale_items.count; i++ ){
			struct sale_item_data* it = sale_items.item[i];

			if( it->timer_start != INVALID_TIMER ){
				delete_timer( it->timer_start, sale_start_timer );
				it->timer_start = INVALID_TIMER;
			}

			if( it->timer_end != INVALID_TIMER ){
				delete_timer( it->timer_end, sale_end_timer );
				it->timer_end = INVALID_TIMER;
			}

			aFree(it);
		}

		aFree(sale_items.item);

		sale_items.item = NULL;
		sale_items.count = 0;
	}
#endif
}

/*
 * Initializes cashshop class.
 * return
 *  0 : success
 */
void do_init_cashshop( void ){
	cash_shop_defined = false;
	cashshop_read_db();
}
