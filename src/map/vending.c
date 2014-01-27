// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/nullpo.h"
#include "../common/malloc.h" // aMalloc, aFree
#include "../common/showmsg.h" // ShowInfo
#include "../common/strlib.h"
#include "../common/utils.h"
#include "clif.h"
#include "itemdb.h"
#include "atcommand.h"
#include "map.h"
#include "path.h"
#include "chrif.h"
#include "vending.h"
#include "pc.h"
#include "npc.h"
#include "skill.h"
#include "battle.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h> // atoi
#include <string.h>

struct vending_entry{
	int cartinventory_id;
	int amount;
	int price;
	int index;
};

struct vending{
	int account_id;
	int char_id;
	int vendor_id;
	int m;
	int x;
	int y;
	unsigned char sex;
	char title[MESSAGE_SIZE];
	uint32 count;
	struct vending_entry** entries;
	struct map_session_data *sd;
};

static int vending_nextid = 0; ///Vending_id counter
static DBMap *vending_db; ///Db holder the vender : charid -> map_session_data
static DBMap *autotrade_db;

/**
 * Lookup to get the vending_db outside module
 * @return the vending_db
 */
DBMap * vending_getdb(){
	return vending_db;
}

/**
 * Create an unique vending shop id.
 * @return the next vending_id
 */
static int vending_getuid(void){
	return ++vending_nextid;
}

/**
 * Make a player close his shop
 * @param sd : player session
 */
void vending_closevending(struct map_session_data* sd)
{
	nullpo_retv(sd);

	if( sd->state.vending ) {
		if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE vending_id = %d;", vending_items_db, sd->vender_id ) != SQL_SUCCESS ||
			Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `id` = %d;", vendings_db, sd->vender_id ) != SQL_SUCCESS ){
				Sql_ShowDebug(mmysql_handle);
		}
		
		sd->state.vending = false;
		clif_closevendingboard(&sd->bl, 0);
		idb_remove(vending_db, sd->status.char_id);
	}
}

/**
 * Player request a shop's item list (a player shop)
 * @param sd : player requestion the list
 * @param id : vender account id (gid)
 */
void vending_vendinglistreq(struct map_session_data* sd, int id)
{
	struct map_session_data* vsd;
	nullpo_retv(sd);

	if( (vsd = map_id2sd(id)) == NULL )
		return;
	if( !vsd->state.vending )
		return; // not vending

	if (!pc_can_give_items(sd) || !pc_can_give_items(vsd)) //check if both GMs are allowed to trade
	{	// GM is not allowed to trade
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return;
	}

	sd->vended_id = vsd->vender_id;  // register vending uid

	clif_vendinglist(sd, id, vsd->vending);
}

/**
 * Purchase item(s) from a shop
 * @param sd : buyer player session
 * @param aid : account id of vender
 * @param uid : shop unique id
 * @param data : items data who would like to purchase \n
 *	data := {<index>.w <amount>.w }[count]
 * @param count : number of different items he's trying to buy
 */
void vending_purchasereq(struct map_session_data* sd, int aid, int uid, const uint8* data, int count)
{
	int i, j, cursor, w, new_ = 0, blank, vend_list[MAX_VENDING];
	double z;
	struct s_vending vending[MAX_VENDING]; // against duplicate packets
	struct map_session_data* vsd = map_id2sd(aid);

	nullpo_retv(sd);
	if( vsd == NULL || !vsd->state.vending || vsd->bl.id == sd->bl.id )
		return; // invalid shop

	if( vsd->vender_id != uid ) { // shop has changed
		clif_buyvending(sd, 0, 0, 6);  // store information was incorrect
		return;
	}

	if( !searchstore_queryremote(sd, aid) && ( sd->bl.m != vsd->bl.m || !check_distance_bl(&sd->bl, &vsd->bl, AREA_SIZE) ) )
		return; // shop too far away

	searchstore_clearremote(sd);

	if( count < 1 || count > MAX_VENDING || count > vsd->vend_num )
		return; // invalid amount of purchased items

	blank = pc_inventoryblank(sd); //number of free cells in the buyer's inventory

	// duplicate item in vending to check hacker with multiple packets
	memcpy(&vending, &vsd->vending, sizeof(vsd->vending)); // copy vending list

	// some checks
	z = 0.; // zeny counter
	w = 0;  // weight counter
	for( i = 0; i < count; i++ ) {
		short amount = *(uint16*)(data + 4*i + 0);
		short idx    = *(uint16*)(data + 4*i + 2);
		idx -= 2;

		if( amount <= 0 )
			return;

		// check of item index in the cart
		if( idx < 0 || idx >= MAX_CART )
			return;

		ARR_FIND( 0, vsd->vend_num, j, vsd->vending[j].index == idx );
		if( j == vsd->vend_num )
			return; //picked non-existing item
		else
			vend_list[i] = j;

		z += ((double)vsd->vending[j].value * (double)amount);
		if( z > (double)sd->status.zeny || z < 0. || z > (double)MAX_ZENY )
		{
			clif_buyvending(sd, idx, amount, 1); // you don't have enough zeny
			return;
		}
		if( z + (double)vsd->status.zeny > (double)MAX_ZENY && !battle_config.vending_over_max )
		{
			clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // too much zeny = overflow
			return;

		}
		w += itemdb_weight(vsd->status.cart[idx].nameid) * amount;
		if( w + sd->weight > sd->max_weight ) {
			clif_buyvending(sd, idx, amount, 2); // you can not buy, because overweight
			return;
		}

		//Check to see if cart/vend info is in sync.
		if( vending[j].amount > vsd->status.cart[idx].amount )
			vending[j].amount = vsd->status.cart[idx].amount;

		// if they try to add packets (example: get twice or more 2 apples if marchand has only 3 apples).
		// here, we check cumulative amounts
		if( vending[j].amount < amount )
		{
			// send more quantity is not a hack (an other player can have buy items just before)
			clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // not enough quantity
			return;
		}

		vending[j].amount -= amount;

		switch( pc_checkadditem(sd, vsd->status.cart[idx].nameid, amount) ) {
		case CHKADDITEM_EXIST:
			break;	//We'd add this item to the existing one (in buyers inventory)
		case CHKADDITEM_NEW:
			new_++;
			if (new_ > blank)
				return; //Buyer has no space in his inventory
			break;
		case CHKADDITEM_OVERAMOUNT:
			return; //too many items
		}
	}

	pc_payzeny(sd, (int)z, LOG_TYPE_VENDING, vsd);
	if( battle_config.vending_tax )
		z -= z * (battle_config.vending_tax/10000.);
	pc_getzeny(vsd, (int)z, LOG_TYPE_VENDING, sd);

	for( i = 0; i < count; i++ ) {
		short amount = *(uint16*)(data + 4*i + 0);
		short idx    = *(uint16*)(data + 4*i + 2);
		idx -= 2;

		// vending item
		pc_additem(sd, &vsd->status.cart[idx], amount, LOG_TYPE_VENDING);
		vsd->vending[vend_list[i]].amount -= amount;

		if( vsd->vending[vend_list[i]].amount ){
			if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `amount` = %d WHERE `vending_id` = %d and `cartinventory_id` = %d", vending_items_db, vsd->vending[vend_list[i]].amount, vsd->vender_id, vsd->status.cart[idx].id ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
			}
		}else{
			if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `vending_id` = %d and `cartinventory_id` = %d", vending_items_db, vsd->vender_id, vsd->status.cart[idx].id ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
			}
		}

		pc_cart_delitem(vsd, idx, amount, 0, LOG_TYPE_VENDING);
		clif_vendingreport(vsd, idx, amount);

		//print buyer's name
		if( battle_config.buyer_name ) {
			char temp[256];
			sprintf(temp, msg_txt(sd,265), sd->status.name);
			clif_disp_onlyself(vsd,temp,strlen(temp));
		}
	}

	// compact the vending list
	for( i = 0, cursor = 0; i < vsd->vend_num; i++ ) {
		if( vsd->vending[i].amount == 0 )
			continue;

		if( cursor != i ) { // speedup
			vsd->vending[cursor].index = vsd->vending[i].index;
			vsd->vending[cursor].amount = vsd->vending[i].amount;
			vsd->vending[cursor].value = vsd->vending[i].value;
		}

		cursor++;
	}
	vsd->vend_num = cursor;

	//Always save BOTH: buyer and customer
	if( save_settings&2 ) {
		chrif_save(sd,0);
		chrif_save(vsd,0);
	}

	//check for @AUTOTRADE users [durf]
	if( vsd->state.autotrade ) {
		//see if there is anything left in the shop
		ARR_FIND( 0, vsd->vend_num, i, vsd->vending[i].amount > 0 );
		if( i == vsd->vend_num ) {
			//Close Vending (this was automatically done by the client, we have to do it manually for autovenders) [Skotlex]
			vending_closevending(vsd);
			map_quit(vsd);	//They have no reason to stay around anymore, do they?
		}
	}
}

/**
 * Player setup a new shop
 * @param sd : player opening the shop
 * @param message : shop title
 * @param data : itemlist data \n
 *	data := {<index>.w <amount>.w <value>.l}[count]
 * @param count : number of different items
 */
void vending_openvending(struct map_session_data* sd, const char* message, const uint8* data, int count) {
	int i, j;
	int vending_skill_lvl;
	char message_sql[MESSAGE_SIZE*2];
	
	nullpo_retv(sd);

	if ( pc_isdead(sd) || !sd->state.prevend || pc_istrading(sd))
		return; // can't open vendings lying dead || didn't use via the skill (wpe/hack) || can't have 2 shops at once

	vending_skill_lvl = pc_checkskill(sd, MC_VENDING);
	
	// skill level and cart check
	if( !vending_skill_lvl || !pc_iscarton(sd) ) {
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0);
		return;
	}

	// check number of items in shop
	if( count < 1 || count > MAX_VENDING || count > 2 + vending_skill_lvl )
	{	// invalid item count
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0);
		return;
	}

	// filter out invalid items
	i = 0;
	for( j = 0; j < count; j++ ) {
		short index        = *(uint16*)(data + 8*j + 0);
		short amount       = *(uint16*)(data + 8*j + 2);
		unsigned int value = *(uint32*)(data + 8*j + 4);

		index -= 2; // offset adjustment (client says that the first cart position is 2)

		if( index < 0 || index >= MAX_CART // invalid position
		||  pc_cartitem_amount(sd, index, amount) < 0 // invalid item or insufficient quantity
		//NOTE: official server does not do any of the following checks!
		||  !sd->status.cart[index].identify // unidentified item
		||  sd->status.cart[index].attribute == 1 // broken item
		||  sd->status.cart[index].expire_time // It should not be in the cart but just in case
		||  (sd->status.cart[index].bound && !pc_can_give_bounded_items(sd)) // can't trade account bound items and has no permission
		||  !itemdb_cantrade(&sd->status.cart[index], pc_get_group_level(sd), pc_get_group_level(sd)) ) // untradeable item
			continue;

		sd->vending[i].index = index;
		sd->vending[i].amount = amount;
		sd->vending[i].value = min(value, (unsigned int)battle_config.vending_max_value);

		i++; // item successfully added
	}

	if( i != j )
		clif_displaymessage (sd->fd, msg_txt(sd,266)); //"Some of your items cannot be vended and were removed from the shop."

	if( i == 0 ) { // no valid item found
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0); // custom reply packet
		return;
	}
	sd->state.prevend = 0;
	sd->state.vending = true;
	sd->vender_id = vending_getuid();
	sd->vend_num = i;
	safestrncpy(sd->message, message, MESSAGE_SIZE);
	
	Sql_EscapeString( mmysql_handle, message_sql, sd->message );

	if( Sql_Query( mmysql_handle, "INSERT INTO `%s`(`id`,`account_id`,`char_id`,`sex`,`map`,`x`,`y`,`title`,`autotrade`) VALUES( %d, %d, %d, '%c', '%s', %d, %d, '%s', %d );", vendings_db, sd->vender_id, sd->status.account_id, sd->status.char_id, sd->status.sex == 0 ? 'F' : 'M', map[sd->bl.m].name, sd->bl.x, sd->bl.y, message_sql, sd->state.autotrade ) != SQL_SUCCESS ){
		Sql_ShowDebug(mmysql_handle);
	}

	for( i = 0; i < count; i++ ){
		if( Sql_Query( mmysql_handle, "INSERT INTO `%s`(`vending_id`,`index`,`cartinventory_id`,`amount`,`price`) VALUES( %d, %d, %d, %d, %d );", vending_items_db, sd->vender_id, i, sd->status.cart[sd->vending[i].index].id, sd->vending[i].amount, sd->vending[i].value ) != SQL_SUCCESS ){
			Sql_ShowDebug(mmysql_handle);
		}
	}

	clif_openvending(sd,sd->bl.id,sd->vending);
	clif_showvendingboard(&sd->bl,message,0);

	idb_put(vending_db, sd->status.char_id, sd);
}

/**
 * Checks if an item is being sold in given player's vending.
 * @param sd : vender session (player)
 * @param nameid : item id
 * @return 0:not selling it, 1: yes
 */
bool vending_search(struct map_session_data* sd, unsigned short nameid) {
	int i;

	if( !sd->state.vending ) { // not vending
		return false;
	}

	ARR_FIND( 0, sd->vend_num, i, sd->status.cart[sd->vending[i].index].nameid == (short)nameid );
	if( i == sd->vend_num ) { // not found
		return false;
	}

	return true;
}



/**
 * Searches for all items in a vending, that match given ids, price and possible cards.
 * @param sd : The vender session to search into
 * @param s : parameter of the search (see s_search_store_search)
 * @return Whether or not the search should be continued.
 */
bool vending_searchall(struct map_session_data* sd, const struct s_search_store_search* s) {
	int i, c, slot;
	unsigned int idx, cidx;
	struct item* it;

	if( !sd->state.vending ) // not vending
		return true;

	for( idx = 0; idx < s->item_count; idx++ ) {
		ARR_FIND( 0, sd->vend_num, i, sd->status.cart[sd->vending[i].index].nameid == (short)s->itemlist[idx] );
		if( i == sd->vend_num ) {// not found
			continue;
		}
		it = &sd->status.cart[sd->vending[i].index];

		if( s->min_price && s->min_price > sd->vending[i].value ) {// too low price
			continue;
		}

		if( s->max_price && s->max_price < sd->vending[i].value ) {// too high price
			continue;
		}

		if( s->card_count ) {// check cards
			if( itemdb_isspecial(it->card[0]) ) {// something, that is not a carded
				continue;
			}
			slot = itemdb_slot(it->nameid);

			for( c = 0; c < slot && it->card[c]; c ++ ) {
				ARR_FIND( 0, s->card_count, cidx, s->cardlist[cidx] == it->card[c] );
				if( cidx != s->card_count )
				{// found
					break;
				}
			}

			if( c == slot || !it->card[c] ) {// no card match
				continue;
			}
		}

		if( !searchstore_result(s->search_sd, sd->vender_id, sd->status.account_id, sd->message, it->nameid, sd->vending[i].amount, sd->vending[i].value, it->card, it->refine) )
		{// result set full
			return false;
		}
	}

	return true;
}

void vending_reopen( struct map_session_data* sd ){
	int i, count;
	uint8 *data, *p;
	uint16 *index, *amount;
	uint32 *value;
	struct vending *vending;
	struct vending_entry *entry;

	vending = (struct vending*)idb_get( autotrade_db, sd->status.char_id );

	if( !vending ){
		map_quit(sd);
		return;
	}

	if( vending->count > 0 ){
		data = (uint8*)aMalloc( vending->count * 8 );

		for( i = 0, p = data, count = vending->count; i < vending->count; i++ ){
			entry = vending->entries[i];

			index = (uint16*)(p + 0);
			amount = (uint16*)(p + 2);
			value = (uint32*)(p + 4);

			ARR_FIND( 0, MAX_CART, entry->index, sd->status.cart[entry->index].id == entry->cartinventory_id );

			if( entry->index == MAX_CART ){
				count--;
				continue;
			}

			*index = entry->index + 2;
			*amount = entry->amount;
			*value = entry->price;

			p += 8;
		}

		// Set him into a hacked prevend state
		sd->state.prevend = 1;

		// Open the shop again
		vending_openvending( sd, vending->title, data, count );

		// Set him to autotrade
		if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `autotrade` = 1 WHERE `id` = %d;", vendings_db, sd->vender_id ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
		}

		// Make him look perfect
		unit_setdir(&sd->bl,battle_config.feature_autotrade_direction);

		if( battle_config.feature_autotrade_sit ){
			pc_setsit(sd);
		}
	}

	aFree(data);

	idb_remove( autotrade_db, sd->status.char_id );

	for( i = 0; i < vending->count; i++ ){
		aFree( vending->entries[i] );
	}

	aFree(vending->entries);
	aFree(vending);

	if( !count ){
		map_quit(sd);
	}
}

void do_init_vending_autotrade( void ){
	if( battle_config.feature_autotrade ){
		struct vending **autotraders;
		struct vending *vending;
		struct vending_entry *entry;
		uint32 count;
		int i, j;

		if( Sql_Query(mmysql_handle, 
			"SELECT `id`, `account_id`, `char_id`, `sex`, `map`, `x`, `y`, `title`"
			"FROM `%s`"
			"WHERE `autotrade` = 1;", vendings_db ) != SQL_SUCCESS ) {
			Sql_ShowDebug(mmysql_handle);
			return;
		}

		count = (uint32)Sql_NumRows(mmysql_handle);

		if( count <= 0 ){
			return;
		}

		autotraders = (struct vending**)aMalloc( sizeof( struct vending* ) * count );
		i = 0;

		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
			size_t len;
			char* data;

			vending = autotraders[i] = (struct vending *)aMalloc( sizeof( struct vending ) );

			Sql_GetData( mmysql_handle, 0, &data, NULL ); vending->vendor_id = atoi(data);
			Sql_GetData( mmysql_handle, 1, &data, NULL ); vending->account_id = atoi(data);
			Sql_GetData( mmysql_handle, 2, &data, NULL ); vending->char_id = atoi(data);
			Sql_GetData( mmysql_handle, 3, &data, NULL ); vending->sex = data[0];
			Sql_GetData( mmysql_handle, 4, &data, NULL ); vending->m = map_mapname2mapid( data );
			Sql_GetData( mmysql_handle, 5, &data, NULL ); vending->x = atoi(data);
			Sql_GetData( mmysql_handle, 6, &data, NULL ); vending->y = atoi(data);
			Sql_GetData( mmysql_handle, 7, &data, &len ); safestrncpy( vending->title, data, min( len + 1, MESSAGE_SIZE ) );

			vending->count = 0;

			idb_put( autotrade_db, vending->char_id, vending );
			
			// initialize player
			CREATE(vending->sd, TBL_PC, 1);

			pc_setnewpc( vending->sd, vending->account_id, vending->char_id, 0, gettick(), vending->sex == 'F' ? 0 : 1, 0 );

			vending->sd->state.autotrade = 1;

			chrif_authreq( vending->sd, true );

			i++;
		}

		Sql_FreeResult( mmysql_handle );

		for( i = 0; i < count; i++ ){
			vending = autotraders[i];

			if( SQL_ERROR == Sql_Query(mmysql_handle, 
				"SELECT `cartinventory_id`, `amount`, `price`"
				"FROM `%s`"
				"WHERE `vending_id` = %d "
				"ORDER BY `index` ASC;", vending_items_db, vending->vendor_id ) ) {
				Sql_ShowDebug(mmysql_handle);
				continue;
			}

			vending->count = (uint32)Sql_NumRows(mmysql_handle);

			if( vending->count <= 0 ){
				// Player was not correctly deleted, must not be set online
				idb_remove( autotrade_db, vending->char_id );
				map_quit(vending->sd);
				aFree(vending);
				continue;
			}
		
			vending->entries = (struct vending_entry**)aMalloc( sizeof( struct vending_entry* ) * vending->count );
			j = 0;

			while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
				char* data;

				entry = vending->entries[j] = (struct vending_entry*)aMalloc( sizeof( struct vending_entry ) );
			
				Sql_GetData( mmysql_handle, 0, &data, NULL ); entry->cartinventory_id = atoi(data);
				Sql_GetData( mmysql_handle, 1, &data, NULL ); entry->amount = atoi(data);
				Sql_GetData( mmysql_handle, 2, &data, NULL ); entry->price = atoi(data);

				j++;
			}

			Sql_FreeResult( mmysql_handle );
		}

		aFree(autotraders);

		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' autotraders.\n",count);
	}
	
	// Everything is loaded fine, their entries will be reinserted once they are loaded
	if( Sql_Query( mmysql_handle, "DELETE FROM `%s`;", vendings_db ) != SQL_SUCCESS ||
		Sql_Query( mmysql_handle, "DELETE FROM `%s`;", vending_items_db ) != SQL_SUCCESS ){
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
 * Initialise the vending module
 * called in map::do_init
 */
void do_final_vending(void) {
	db_destroy(vending_db);
	db_destroy(autotrade_db);
}

/**
 * Destory the vending module
 * called in map::do_final
 */
void do_init_vending(void) {
	vending_db = idb_alloc(DB_OPT_BASE);
	autotrade_db = idb_alloc(DB_OPT_BASE);
	vending_nextid = 0;
}
