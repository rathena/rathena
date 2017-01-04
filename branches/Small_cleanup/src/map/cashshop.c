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
static int cashshop_parse_dbrow( char** str, const char* source, int line ){
	unsigned short nameid = atoi( str[1] );

	if( itemdb_exists( nameid ) ){
		uint16 tab = atoi( str[0] );
		uint32 price = atoi( str[2] );
		struct cash_item_data* cid;
		int j;

		if( tab > CASHSHOP_TAB_SEARCH ){
			ShowWarning( "cashshop_parse_dbrow: Invalid tab %d in line %d of \"%s\", skipping...\n", tab, line, source );
			return 0;
		}else if( price < 1 ){
			ShowWarning( "cashshop_parse_dbrow: Invalid price %d in line %d of \"%s\", skipping...\n", price, line, source );
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
	}else{
		ShowWarning( "cashshop_parse_dbrow: Invalid ID %hu in line %d of \"%s\", skipping...\n", nameid, line, source );
	}

	return 0;
}

/*
 * Reads database from TXT format,
 * parses lines and sends them to parse_dbrow.
 * TODO: Change to sv_readdb
 */
static void cashshop_read_db_txt( void ){
	const char* filename[] = { DBPATH"item_cash_db.txt", DBIMPORT"/item_cash_db.txt" };
	int fi;

	for( fi = 0; fi < ARRAYLENGTH( filename ); ++fi ){
		uint32 lines = 0, count = 0;
		char line[1024];

		char path[256];
		FILE* fp;

		sprintf( path, "%s/%s", db_path, filename[fi] );
		fp = fopen( path, "r" );
		if( fp == NULL ) {
			ShowWarning( "itemdb_readdb: File not found \"%s\", skipping.\n", path );
			continue;
		}

		while( fgets( line, sizeof( line ), fp ) ){
			char *str[3], *p;
			int i;
			lines++;

			if( line[0] == '/' && line[1] == '/' )
				continue;

			memset( str, 0, sizeof( str ) );

			p = line;
			while( ISSPACE( *p ) )
				++p;
			if( *p == '\0' )
				continue;

			for( i = 0; i < 2; ++i ){
				str[i] = p;
				p = strchr( p, ',' );

				if( p == NULL )
					break;

				*p = '\0';
				++p;
			}

			str[2] = p;
			while( !ISSPACE( *p ) && *p != '\0' && *p != '/' )
				++p;

			if( p == NULL ){
				ShowError("cashshop_read_db_txt: Insufficient columns in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi( str[0] ) );
				continue;
			}

			if( !cashshop_parse_dbrow( str, path, lines ) )
				continue;

			count++;
		}

		fclose(fp);

		ShowStatus( "Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, path );
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

			if( !cashshop_parse_dbrow( str, cash_db_name[fi], lines ) )
				continue;

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

		nameid = *( item_list + i * 5 ) = cash_shop_items[tab].item[j]->nameid; //item_avail replacement

		if( j == cash_shop_items[tab].count || !itemdb_exists( nameid ) ){
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
