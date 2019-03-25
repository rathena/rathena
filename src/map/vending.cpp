// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vending.hpp"

#include <stdlib.h> // atoi

#include "../common/malloc.hpp" // aMalloc, aFree
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp" // ShowInfo
#include "../common/strlib.hpp"
#include "../common/timer.hpp"  // DIFF_TICK

#include "achievement.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "buyingstore.hpp"
#include "buyingstore.hpp" // struct s_autotrade_entry, struct s_autotrader
#include "chrif.hpp"
#include "clif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "npc.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"

static uint32 vending_nextid = 0; ///Vending_id counter
static DBMap *vending_db; ///DB holder the vender : charid -> map_session_data

//Autotrader
static DBMap *vending_autotrader_db; /// Holds autotrader info: char_id -> struct s_autotrader
static void vending_autotrader_remove(struct s_autotrader *at, bool remove);
static int vending_autotrader_free(DBKey key, DBData *data, va_list ap);

/**
 * Lookup to get the vending_db outside module
 * @return the vending_db
 */
DBMap * vending_getdb()
{
	return vending_db;
}

/**
 * Create an unique vending shop id.
 * @return the next vending_id
 */
static int vending_getuid(void)
{
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
		if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE vending_id = %d;", vending_items_table, sd->vender_id ) != SQL_SUCCESS ||
			Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `id` = %d;", vendings_table, sd->vender_id ) != SQL_SUCCESS ) {
				Sql_ShowDebug(mmysql_handle);
		}

		sd->state.vending = false;
		sd->vender_id = 0;
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

	if (!pc_can_give_items(sd) || !pc_can_give_items(vsd)) { //check if both GMs are allowed to trade
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return;
	}

	sd->vended_id = vsd->vender_id;  // register vending uid

	clif_vendinglist(sd, id, vsd->vending);
}

/**
 * Calculates taxes for vending
 * @param sd: Vender
 * @param zeny: Total amount to tax
 * @return Total amount after taxes
 */
static double vending_calc_tax(struct map_session_data *sd, double zeny)
{
	if (battle_config.vending_tax && zeny >= battle_config.vending_tax_min)
		zeny -= zeny * (battle_config.vending_tax / 10000.);

	return zeny;
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
		if( z > (double)sd->status.zeny || z < 0. || z > (double)MAX_ZENY ) {
			clif_buyvending(sd, idx, amount, 1); // you don't have enough zeny
			return;
		}
		if( z + (double)vsd->status.zeny > (double)MAX_ZENY && !battle_config.vending_over_max ) {
			clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // too much zeny = overflow
			return;

		}
		w += itemdb_weight(vsd->cart.u.items_cart[idx].nameid) * amount;
		if( w + sd->weight > sd->max_weight ) {
			clif_buyvending(sd, idx, amount, 2); // you can not buy, because overweight
			return;
		}

		//Check to see if cart/vend info is in sync.
		if( vending[j].amount > vsd->cart.u.items_cart[idx].amount )
			vending[j].amount = vsd->cart.u.items_cart[idx].amount;

		// if they try to add packets (example: get twice or more 2 apples if marchand has only 3 apples).
		// here, we check cumulative amounts
		if( vending[j].amount < amount ) {
			// send more quantity is not a hack (an other player can have buy items just before)
			clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // not enough quantity
			return;
		}

		vending[j].amount -= amount;

		switch( pc_checkadditem(sd, vsd->cart.u.items_cart[idx].nameid, amount) ) {
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
	achievement_update_objective(sd, AG_SPEND_ZENY, 1, (int)z);
	z = vending_calc_tax(sd, z);
	pc_getzeny(vsd, (int)z, LOG_TYPE_VENDING, sd);

	for( i = 0; i < count; i++ ) {
		short amount = *(uint16*)(data + 4*i + 0);
		short idx    = *(uint16*)(data + 4*i + 2);
		idx -= 2;
		z = 0.; // zeny counter

		// vending item
		pc_additem(sd, &vsd->cart.u.items_cart[idx], amount, LOG_TYPE_VENDING);
		vsd->vending[vend_list[i]].amount -= amount;
		z += ((double)vsd->vending[i].value * (double)amount);

		if( vsd->vending[vend_list[i]].amount ) {
			if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `amount` = %d WHERE `vending_id` = %d and `cartinventory_id` = %d", vending_items_table, vsd->vending[vend_list[i]].amount, vsd->vender_id, vsd->cart.u.items_cart[idx].id ) != SQL_SUCCESS ) {
				Sql_ShowDebug( mmysql_handle );
			}
		} else {
			if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `vending_id` = %d and `cartinventory_id` = %d", vending_items_table, vsd->vender_id, vsd->cart.u.items_cart[idx].id ) != SQL_SUCCESS ) {
				Sql_ShowDebug( mmysql_handle );
			}
		}

		pc_cart_delitem(vsd, idx, amount, 0, LOG_TYPE_VENDING);
		z = vending_calc_tax(sd, z);
		clif_vendingreport(vsd, idx, amount, sd->status.char_id, (int)z);

		//print buyer's name
		if( battle_config.buyer_name ) {
			char temp[256];
			sprintf(temp, msg_txt(sd,265), sd->status.name);
			clif_messagecolor(&vsd->bl, color_table[COLOR_LIGHT_GREEN], temp, false, SELF);
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

	//Always save BOTH: customer (buyer) and vender
	if( save_settings&CHARSAVE_VENDING ) {
		chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
		chrif_save(vsd, CSAVE_INVENTORY|CSAVE_CART);
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
 * @param data : itemlist data
 *	data := {<index>.w <amount>.w <value>.l}[count]
 * @param count : number of different items
 * @param at Autotrader info, or NULL if requetsed not from autotrade persistance
 * @return 0 If success, 1 - Cannot open (die, not state.prevend, trading), 2 - No cart, 3 - Count issue, 4 - Cart data isn't saved yet, 5 - No valid item found
 */
int8 vending_openvending(struct map_session_data* sd, const char* message, const uint8* data, int count, struct s_autotrader *at)
{
	int i, j;
	int vending_skill_lvl;
	char message_sql[MESSAGE_SIZE*2];
	StringBuf buf;
	
	nullpo_retr(false,sd);

	if ( pc_isdead(sd) || !sd->state.prevend || pc_istrading(sd)) {
		return 1; // can't open vendings lying dead || didn't use via the skill (wpe/hack) || can't have 2 shops at once
	}

	vending_skill_lvl = pc_checkskill(sd, MC_VENDING);
	
	// skill level and cart check
	if( !vending_skill_lvl || !pc_iscarton(sd) ) {
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0);
		return 2;
	}

	// check number of items in shop
	if( count < 1 || count > MAX_VENDING || count > 2 + vending_skill_lvl ) { // invalid item count
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0);
		return 3;
	}

	if (save_settings&CHARSAVE_VENDING) // Avoid invalid data from saving
		chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);

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
		||  !sd->cart.u.items_cart[index].identify // unidentified item
		||  sd->cart.u.items_cart[index].attribute == 1 // broken item
		||  sd->cart.u.items_cart[index].expire_time // It should not be in the cart but just in case
		||  (sd->cart.u.items_cart[index].bound && !pc_can_give_bounded_items(sd)) // can't trade account bound items and has no permission
		||  !itemdb_cantrade(&sd->cart.u.items_cart[index], pc_get_group_level(sd), pc_get_group_level(sd)) ) // untradeable item
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
		return 5;
	}

	sd->state.prevend = 0;
	sd->state.vending = true;
	sd->state.workinprogress = WIP_DISABLE_NONE;
	sd->vender_id = vending_getuid();
	sd->vend_num = i;
	safestrncpy(sd->message, message, MESSAGE_SIZE);
	
	Sql_EscapeString( mmysql_handle, message_sql, sd->message );

	if( Sql_Query( mmysql_handle, "INSERT INTO `%s`(`id`, `account_id`, `char_id`, `sex`, `map`, `x`, `y`, `title`, `autotrade`, `body_direction`, `head_direction`, `sit`) "
		"VALUES( %d, %d, %d, '%c', '%s', %d, %d, '%s', %d, '%d', '%d', '%d' );",
		vendings_table, sd->vender_id, sd->status.account_id, sd->status.char_id, sd->status.sex == SEX_FEMALE ? 'F' : 'M', map_getmapdata(sd->bl.m)->name, sd->bl.x, sd->bl.y, message_sql, sd->state.autotrade, at ? at->dir : sd->ud.dir, at ? at->head_dir : sd->head_dir, at ? at->sit : pc_issit(sd) ) != SQL_SUCCESS ) {
		Sql_ShowDebug(mmysql_handle);
	}

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`vending_id`,`index`,`cartinventory_id`,`amount`,`price`) VALUES", vending_items_table);
	for (j = 0; j < i; j++) {
		StringBuf_Printf(&buf, "(%d,%d,%d,%d,%d)", sd->vender_id, j, sd->cart.u.items_cart[sd->vending[j].index].id, sd->vending[j].amount, sd->vending[j].value);
		if (j < i-1)
			StringBuf_AppendStr(&buf, ",");
	}
	if (SQL_ERROR == Sql_QueryStr(mmysql_handle, StringBuf_Value(&buf)))
		Sql_ShowDebug(mmysql_handle);
	StringBuf_Destroy(&buf);

	clif_openvending(sd,sd->bl.id,sd->vending);
	clif_showvendingboard(&sd->bl,message,0);

	idb_put(vending_db, sd->status.char_id, sd);

	return 0;
}

/**
 * Checks if an item is being sold in given player's vending.
 * @param sd : vender session (player)
 * @param nameid : item id
 * @return 0:not selling it, 1: yes
 */
bool vending_search(struct map_session_data* sd, unsigned short nameid)
{
	int i;

	if( !sd->state.vending ) { // not vending
		return false;
	}

	ARR_FIND( 0, sd->vend_num, i, sd->cart.u.items_cart[sd->vending[i].index].nameid == (short)nameid );
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
bool vending_searchall(struct map_session_data* sd, const struct s_search_store_search* s)
{
	int i, c, slot;
	unsigned int idx, cidx;
	struct item* it;

	if( !sd->state.vending ) // not vending
		return true;

	for( idx = 0; idx < s->item_count; idx++ ) {
		ARR_FIND( 0, sd->vend_num, i, sd->cart.u.items_cart[sd->vending[i].index].nameid == s->itemlist[idx] );
		if( i == sd->vend_num ) { // not found
			continue;
		}
		it = &sd->cart.u.items_cart[sd->vending[i].index];

		if( s->min_price && s->min_price > sd->vending[i].value ) { // too low price
			continue;
		}

		if( s->max_price && s->max_price < sd->vending[i].value ) { // too high price
			continue;
		}

		if( s->card_count ) { // check cards
			if( itemdb_isspecial(it->card[0]) ) { // something, that is not a carded
				continue;
			}
			slot = itemdb_slot(it->nameid);

			for( c = 0; c < slot && it->card[c]; c ++ ) {
				ARR_FIND( 0, s->card_count, cidx, s->cardlist[cidx] == it->card[c] );
				if( cidx != s->card_count ) { // found
					break;
				}
			}

			if( c == slot || !it->card[c] ) { // no card match
				continue;
			}
		}

		if( !searchstore_result(s->search_sd, sd->vender_id, sd->status.account_id, sd->message, it->nameid, sd->vending[i].amount, sd->vending[i].value, it->card, it->refine) ) { // result set full
			return false;
		}
	}

	return true;
}

/**
* Open vending for Autotrader
* @param sd Player as autotrader
*/
void vending_reopen( struct map_session_data* sd )
{
	struct s_autotrader *at = NULL;
	int8 fail = -1;

	nullpo_retv(sd);

	// Open vending for this autotrader
	if ((at = (struct s_autotrader *)uidb_get(vending_autotrader_db, sd->status.char_id)) && at->count && at->entries) {
		uint8 *data, *p;
		uint16 j, count;

		// Init vending data for autotrader
		CREATE(data, uint8, at->count * 8);

		for (j = 0, p = data, count = at->count; j < at->count; j++) {
			struct s_autotrade_entry *entry = at->entries[j];
			uint16 *index = (uint16*)(p + 0);
			uint16 *amount = (uint16*)(p + 2);
			uint32 *value = (uint32*)(p + 4);

			// Find item position in cart
			ARR_FIND(0, MAX_CART, entry->index, sd->cart.u.items_cart[entry->index].id == entry->cartinventory_id);

			if (entry->index == MAX_CART) {
				count--;
				continue;
			}

			*index = entry->index + 2;
			*amount = itemdb_isstackable(sd->cart.u.items_cart[entry->index].nameid) ? entry->amount : 1;
			*value = entry->price;

			p += 8;
		}

		sd->state.prevend = 1; // Set him into a hacked prevend state
		sd->state.autotrade = 1;

		// Make sure abort all NPCs
		npc_event_dequeue(sd);
		pc_cleareventtimer(sd);

		// Open the vending again
		if( (fail = vending_openvending(sd, at->title, data, count, at)) == 0 ) {
			// Make vendor look perfect
			pc_setdir(sd, at->dir, at->head_dir);
			clif_changed_dir(&sd->bl, AREA_WOS);
			if( at->sit ) {
				pc_setsit(sd);
				skill_sit(sd, 1);
				clif_sitting(&sd->bl);
			}

			// Immediate save
			chrif_save(sd, CSAVE_AUTOTRADE);

			ShowInfo("Vending loaded for '" CL_WHITE "%s" CL_RESET "' with '" CL_WHITE "%d" CL_RESET "' items at " CL_WHITE "%s (%d,%d)" CL_RESET "\n",
				sd->status.name, count, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y);
		}
		aFree(data);
	}

	if (at) {
		vending_autotrader_remove(at, true);
		if (db_size(vending_autotrader_db) == 0)
			vending_autotrader_db->clear(vending_autotrader_db, vending_autotrader_free);
	}

	if (fail != 0) {
		ShowError("vending_reopen: (Error:%d) Load failed for autotrader '" CL_WHITE "%s" CL_RESET "' (CID=%d/AID=%d)\n", fail, sd->status.name, sd->status.char_id, sd->status.account_id);
		map_quit(sd);
	}
}

/**
* Initializing autotraders from table
*/
void do_init_vending_autotrade(void)
{
	if (battle_config.feature_autotrade) {
		if (Sql_Query(mmysql_handle,
			"SELECT `id`, `account_id`, `char_id`, `sex`, `title`, `body_direction`, `head_direction`, `sit` "
			"FROM `%s` "
			"WHERE `autotrade` = 1 AND (SELECT COUNT(`vending_id`) FROM `%s` WHERE `vending_id` = `id`) > 0 "
			"ORDER BY `id`;",
			vendings_table, vending_items_table ) != SQL_SUCCESS )
		{
			Sql_ShowDebug(mmysql_handle);
			return;
		}

		if( Sql_NumRows(mmysql_handle) > 0 ) {
			uint16 items = 0;
			DBIterator *iter = NULL;
			struct s_autotrader *at = NULL;

			// Init each autotrader data
			while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
				size_t len;
				char* data;

				at = NULL;
				CREATE(at, struct s_autotrader, 1);
				Sql_GetData(mmysql_handle, 0, &data, NULL); at->id = atoi(data);
				Sql_GetData(mmysql_handle, 1, &data, NULL); at->account_id = atoi(data);
				Sql_GetData(mmysql_handle, 2, &data, NULL); at->char_id = atoi(data);
				Sql_GetData(mmysql_handle, 3, &data, NULL); at->sex = (data[0] == 'F') ? SEX_FEMALE : SEX_MALE;
				Sql_GetData(mmysql_handle, 4, &data, &len); safestrncpy(at->title, data, zmin(len + 1, MESSAGE_SIZE));
				Sql_GetData(mmysql_handle, 5, &data, NULL); at->dir = atoi(data);
				Sql_GetData(mmysql_handle, 6, &data, NULL); at->head_dir = atoi(data);
				Sql_GetData(mmysql_handle, 7, &data, NULL); at->sit = atoi(data);
				at->count = 0;

				if (battle_config.feature_autotrade_direction >= 0)
					at->dir = battle_config.feature_autotrade_direction;
				if (battle_config.feature_autotrade_head_direction >= 0)
					at->head_dir = battle_config.feature_autotrade_head_direction;
				if (battle_config.feature_autotrade_sit >= 0)
					at->sit = battle_config.feature_autotrade_sit;

				// initialize player
				CREATE(at->sd, struct map_session_data, 1);
				pc_setnewpc(at->sd, at->account_id, at->char_id, 0, gettick(), at->sex, 0);
				at->sd->state.autotrade = 1|2;
				at->sd->state.monster_ignore = (battle_config.autotrade_monsterignore);
				chrif_authreq(at->sd, true);
				uidb_put(vending_autotrader_db, at->char_id, at);
			}
			Sql_FreeResult(mmysql_handle);

			// Init items for each autotraders
			iter = db_iterator(vending_autotrader_db);
			for (at = (struct s_autotrader *)dbi_first(iter); dbi_exists(iter); at = (struct s_autotrader *)dbi_next(iter)) {
				uint16 j = 0;

				if (SQL_ERROR == Sql_Query(mmysql_handle,
					"SELECT `cartinventory_id`, `amount`, `price` "
					"FROM `%s` "
					"WHERE `vending_id` = %d "
					"ORDER BY `index` ASC;",
					vending_items_table, at->id ) )
				{
					Sql_ShowDebug(mmysql_handle);
					continue;
				}

				if (!(at->count = (uint16)Sql_NumRows(mmysql_handle))) {
					map_quit(at->sd);
					vending_autotrader_remove(at, true);
					continue;
				}

				//Init the list
				CREATE(at->entries, struct s_autotrade_entry *, at->count);

				//Add the item into list
				j = 0;
				while (SQL_SUCCESS == Sql_NextRow(mmysql_handle) && j < at->count) {
					char *data;
					CREATE(at->entries[j], struct s_autotrade_entry, 1);
					Sql_GetData(mmysql_handle, 0, &data, NULL); at->entries[j]->cartinventory_id = atoi(data);
					Sql_GetData(mmysql_handle, 1, &data, NULL); at->entries[j]->amount = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, NULL); at->entries[j]->price = atoi(data);
					j++;
				}
				items += j;
				Sql_FreeResult(mmysql_handle);
			}
			dbi_destroy(iter);

			ShowStatus("Done loading '" CL_WHITE "%d" CL_RESET "' vending autotraders with '" CL_WHITE "%d" CL_RESET "' items.\n", db_size(vending_autotrader_db), items);
		}
	}

	// Everything is loaded fine, their entries will be reinserted once they are loaded
	if (Sql_Query( mmysql_handle, "DELETE FROM `%s`;", vendings_table ) != SQL_SUCCESS ||
		Sql_Query( mmysql_handle, "DELETE FROM `%s`;", vending_items_table ) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
 * Remove an autotrader's data
 * @param at Autotrader
 * @param remove If true will removes from vending_autotrader_db
 **/
static void vending_autotrader_remove(struct s_autotrader *at, bool remove) {
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
		uidb_remove(vending_autotrader_db, at->char_id);
	aFree(at);
}

/**
* Clear all autotraders
* @author [Cydh]
*/
static int vending_autotrader_free(DBKey key, DBData *data, va_list ap) {
	struct s_autotrader *at = (struct s_autotrader *)db_data2ptr(data);
	if (at)
		vending_autotrader_remove(at, false);
	return 0;
}

/**
 * Initialise the vending module
 * called in map::do_init
 */
void do_final_vending(void)
{
	db_destroy(vending_db);
	vending_autotrader_db->destroy(vending_autotrader_db, vending_autotrader_free);
}

/**
 * Destory the vending module
 * called in map::do_final
 */
void do_init_vending(void)
{
	vending_db = idb_alloc(DB_OPT_BASE);
	vending_autotrader_db = uidb_alloc(DB_OPT_BASE);
	vending_nextid = 0;
}
