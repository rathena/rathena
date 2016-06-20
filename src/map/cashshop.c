// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h" // uint16, uint32
#include "../common/malloc.h" // CREATE, RECREATE, aFree
#include "../common/showmsg.h" // ShowWarning, ShowStatus

#include "cashshop.h"
#include "pet.h" // pet_create_egg

#include <string.h> // memset
#include <stdlib.h> // atoi

struct cash_item_db cash_shop_items[CASHSHOP_TAB_SEARCH];
bool cash_shop_defined = false;

extern char item_cash_db_db[32];
extern char item_cash_db2_db[32];

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

	if( tab > CASHSHOP_TAB_SEARCH ){
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
		"/"DBIMPORT,
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

		sv_readdb(dbsubpath2, "item_cash_db.txt",          ',', 3, 3, -1, &cashshop_parse_dbrow, fi);

		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}
}

/*
 * Reads database from SQL format,
 * parses line and sends them to parse_dbrow.
 */
static int cashshop_read_db_sql( void ){
	const char* cash_db_name[] = { item_cash_db_db, item_cash_db2_db };
	int fi;

	for( fi = 0; fi < ARRAYLENGTH( cash_db_name ); ++fi ){
		uint32 lines = 0, count = 0;

		if( SQL_ERROR == Sql_Query( mmysql_handle, "SELECT `tab`, `item_id`, `price` FROM `%s`", cash_db_name[fi] ) ){
			Sql_ShowDebug( mmysql_handle );
			continue;
		}

		while( SQL_SUCCESS == Sql_NextRow( mmysql_handle ) ){
			char* str[3];
			int i;

			++lines;

			for( i = 0; i < 3; ++i ){
				Sql_GetData( mmysql_handle, i, &str[i], NULL );

				if( str[i] == NULL ){
					str[i] = "";
				}
			}

			if( !cashshop_parse_dbrow( str, 3, lines ) ) {
				ShowError("cashshop_read_db_sql: Cannot process table '%s' at line '%d', skipping...\n", cash_db_name[fi], lines);
				continue;
			}

			++count;
		}

		Sql_FreeResult( mmysql_handle );

		ShowStatus( "Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, cash_db_name[fi] );
	}

	return 0;
}

/*
 * Determines whether to read TXT or SQL database
 * based on 'db_use_sqldbs' in conf/map_athena.conf.
 */
static void cashshop_read_db( void ){
	if( db_use_sqldbs ){
		cashshop_read_db_sql();
	} else {
		cashshop_read_db_txt();
	}
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
		uint16 tab = *( item_list + i * 5 + 4 );
		int j;

		if( tab > CASHSHOP_TAB_SEARCH ){
			clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKNOWN );
			return false;
		}

		ARR_FIND( 0, cash_shop_items[tab].count, j, nameid == cash_shop_items[tab].item[j]->nameid || nameid == itemdb_viewid(cash_shop_items[tab].item[j]->nameid) );

		if( j == cash_shop_items[tab].count ){
			clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKONWN_ITEM );
			return false;
		}

		nameid = *( item_list + i * 5 ) = cash_shop_items[tab].item[j]->nameid; //item_avail replacement

		if( !itemdb_exists( nameid ) ){
			clif_cashshop_result( sd, nameid, CASHSHOP_RESULT_ERROR_UNKONWN_ITEM );
			return false;
		}else if( !itemdb_isstackable( nameid ) && quantity > 1 ){
			/* ShowWarning( "Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable cash item %hu!\n", sd->status.name, sd->status.account_id, sd->status.char_id, quantity, nameid ); */
			quantity = *( item_list + i * 5 + 2 ) = 1;
		}

		switch( pc_checkadditem( sd, nameid, quantity ) ){
			case CHKADDITEM_EXIST:
				break;

			case CHKADDITEM_NEW:
				new_++;
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

	if(pc_paycash( sd, totalcash, kafrapoints, LOG_TYPE_CASH ) < 0){
		clif_cashshop_result( sd, 0, CASHSHOP_RESULT_ERROR_SHORTTAGE_CASH );
		return false;
	}

	for( i = 0; i < n; ++i ){
		unsigned short nameid = *( item_list + i * 5 );
		uint32 quantity = *( item_list + i * 5 + 2 );
		struct item_data *id = itemdb_search(nameid);

		if (!id)
			continue;

		if (!itemdb_isstackable2(id) && quantity > 1)
			quantity = 1;

		if (!pet_create_egg(sd, nameid)) {
			unsigned short get_amt = quantity, j;

			if (id->flag.guid)
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

	for( tab = CASHSHOP_TAB_NEW; tab < CASHSHOP_TAB_SEARCH; tab++ ){
		for( i = 0; i < cash_shop_items[tab].count; i++ ){
			aFree( cash_shop_items[tab].item[i] );
		}
		aFree( cash_shop_items[tab].item );
	}
	memset( cash_shop_items, 0, sizeof( cash_shop_items ) );
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
