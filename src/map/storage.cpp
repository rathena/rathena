// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "storage.hpp"

#include <map>

#include <stdlib.h>
#include <string.h>

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/utilities.hpp"

#include "battle.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "guild.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp" // struct map_session_data
#include "pc.hpp"
#include "pc_groups.hpp"

using namespace rathena;

///Databases of guild_storage : int guild_id -> struct guild_storage
std::map<int, struct s_storage> guild_storage_db;

struct s_storage_table *storage_db;
int storage_count;

/**
 * Get storage name
 * @param id Storage ID
 * @return Storage name or "Storage" if not found
 * @author [Cydh]
 **/
const char *storage_getName(uint8 id) {
	if (storage_db && storage_count) {
		int i;
		for (i = 0; i < storage_count; i++) {
			if (&storage_db[i] && storage_db[i].id == id && storage_db[i].name[0])
				return storage_db[i].name;
		}
	}
	return "Storage";
}

/**
 * Check if sotrage ID is valid
 * @param id Storage ID
 * @return True:Valid, False:Invalid
 **/
bool storage_exists(uint8 id) {
	if (storage_db && storage_count) {
		int i;
		for (i = 0; i < storage_count; i++) {
			if (storage_db[i].id == id)
				return true;
		}
	}
	return false;
}

/**
 * Storage item comparator (for qsort)
 * sort by itemid and amount
 * @param _i1 : item a
 * @param _i2 : item b
 * @return i1<=>i2
 */
static int storage_comp_item(const void *_i1, const void *_i2)
{
	struct item *i1 = (struct item *)_i1;
	struct item *i2 = (struct item *)_i2;

	if (i1->nameid == i2->nameid)
		return 0;
	else if (!(i1->nameid) || !(i1->amount))
		return 1;
	else if (!(i2->nameid) || !(i2->amount))
		return -1;

	return i1->nameid - i2->nameid;
}

/**
 * Sort item by storage_comp_item (nameid)
 * used when we open up our storage or guild_storage
 * @param items : list of items to sort
 * @param size : number of item in list
 */
void storage_sortitem(struct item* items, unsigned int size)
{
	nullpo_retv(items);

	if( battle_config.client_sort_storage )
		qsort(items, size, sizeof(struct item), storage_comp_item);
}

/**
 * Initiate storage module
 * Called from map.cpp::do_init()
 */
void do_init_storage(void)
{
	storage_db = NULL;
	storage_count = 0;
}

/**
 * Destroy storage module
 * @author : [MC Cameri]
 * Called from map.cpp::do_final()
 */
void do_final_storage(void)
{
	guild_storage_db.clear();
	if (storage_db)
		aFree(storage_db);
	storage_db = NULL;
	storage_count = 0;
}

/**
 * Function to be invoked upon server reconnection to char. To save all 'dirty' storages
 * @author [Skotlex]
 */
void do_reconnect_storage(void){
	for( auto entry : guild_storage_db ){
		struct s_storage stor = entry.second;

		// Save closed storages.
		if( stor.dirty && stor.status == 0 ){
			storage_guild_storagesave(0, stor.id, 0);
		}
	}
}

/**
 * Player attempt tp open his storage.
 * @param sd : player
 * @return  0:success, 1:fail
 */
int storage_storageopen(struct map_session_data *sd)
{
	nullpo_ret(sd);

	if(sd->state.storage_flag)
		return 1; //Already open?

	if( !pc_can_give_items(sd) ) { // check is this GM level is allowed to put items to storage
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return 1;
	}

	sd->state.storage_flag = 1;
	storage_sortitem(sd->storage.u.items_storage, sd->storage.max_amount);
	clif_storagelist(sd, sd->storage.u.items_storage, sd->storage.max_amount, storage_getName(0));
	clif_updatestorageamount(sd, sd->storage.amount, sd->storage.max_amount);

	return 0;
}

/**
 * Check if 2 item a and b have same values
 * @param a : item 1
 * @param b : item 2
 * @return 1:same, 0:are different
 */
int compare_item(struct item *a, struct item *b)
{
	if( a->nameid == b->nameid &&
		a->identify == b->identify &&
		a->refine == b->refine &&
		a->attribute == b->attribute &&
		a->expire_time == b->expire_time &&
		a->bound == b->bound &&
		a->unique_id == b->unique_id
		)
	{
		int i;

		for (i = 0; i < MAX_SLOTS && (a->card[i] == b->card[i]); i++);

		return (i == MAX_SLOTS);
	}

	return 0;
}

/**
 * Check if item can be added to storage
 * @param stor Storage data
 * @param idx Index item from inventory/cart
 * @param items List of items from inventory/cart
 * @param amount Amount of item will be added
 * @param max_num Max inventory/cart
 * @return @see enum e_storage_add
 **/
static enum e_storage_add storage_canAddItem(struct s_storage *stor, int idx, struct item items[], int amount, int max_num) {
	if (idx < 0 || idx >= max_num)
		return STORAGE_ADD_INVALID;

	if (items[idx].nameid <= 0)
		return STORAGE_ADD_INVALID; // No item on that spot

	if (amount < 1 || amount > items[idx].amount)
		return STORAGE_ADD_INVALID;

	if (itemdb_ishatched_egg(&items[idx]))
		return STORAGE_ADD_INVALID;

	if (!stor->state.put)
		return STORAGE_ADD_NOACCESS;

	return STORAGE_ADD_OK;
}

/**
 * Check if item can be moved from storage
 * @param stor Storage data
 * @param idx Index from storage
 * @param amount Number of item
 * @return @see enum e_storage_add
 **/
static enum e_storage_add storage_canGetItem(struct s_storage *stor, int idx, int amount) {
	// If last index check is sd->storage_size, if player isn't VIP anymore but there's item, player can't take it
	// Example the storage size when not VIP anymore is 350/300, player still can take the 301st~349th item.
	if( idx < 0 || idx >= ARRAYLENGTH(stor->u.items_storage) )
		return STORAGE_ADD_INVALID;

	if( stor->u.items_storage[idx].nameid <= 0 )
		return STORAGE_ADD_INVALID; //Nothing there

	if( amount < 1 || amount > stor->u.items_storage[idx].amount )
		return STORAGE_ADD_INVALID;

	if (!stor->state.get)
		return STORAGE_ADD_NOACCESS;

	return STORAGE_ADD_OK;
}

/**
 * Make a player add an item to his storage
 * @param sd : player
 * @param stor : Storage data
 * @param item_data : item to add
 * @param amount : quantity of items
 * @return 0:success, 1:failed, 2:failed because of room or stack checks
 */
static int storage_additem(struct map_session_data* sd, struct s_storage *stor, struct item *it, int amount)
{
	struct item_data *data;
	int i;

	if( it->nameid == 0 || amount <= 0 )
		return 1;

	data = itemdb_search(it->nameid);

	if( data->stack.storage && amount > data->stack.amount ) // item stack limitation
		return 2;

	if( !itemdb_canstore(it, pc_get_group_level(sd)) ) { // Check if item is storable. [Skotlex]
		clif_displaymessage (sd->fd, msg_txt(sd,264));
		return 1;
	}

	if( (it->bound > BOUND_ACCOUNT) && !pc_can_give_bounded_items(sd) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,294));
		return 1;
	}

	if( itemdb_isstackable2(data) ) { // Stackable
		for( i = 0; i < stor->max_amount; i++ ) {
			if( compare_item(&stor->u.items_storage[i], it) ) { // existing items found, stack them
				if( amount > MAX_AMOUNT - stor->u.items_storage[i].amount || ( data->stack.storage && amount > data->stack.amount - stor->u.items_storage[i].amount ) )
					return 2;

				stor->u.items_storage[i].amount += amount;
				stor->dirty = true;
				clif_storageitemadded(sd,&stor->u.items_storage[i],i,amount);

				return 0;
			}
		}
	}

	if( stor->amount >= stor->max_amount )
		return 2;

	// find free slot
	ARR_FIND( 0, stor->max_amount, i, stor->u.items_storage[i].nameid == 0 );
	if( i >= stor->max_amount )
		return 2;

	// add item to slot
	memcpy(&stor->u.items_storage[i],it,sizeof(stor->u.items_storage[0]));
	stor->amount++;
	stor->u.items_storage[i].amount = amount;
	stor->dirty = true;
	clif_storageitemadded(sd,&stor->u.items_storage[i],i,amount);
	clif_updatestorageamount(sd, stor->amount, stor->max_amount);

	return 0;
}

/**
 * Make a player delete an item from his storage
 * @param sd : player
 * @param n : idx on storage to remove the item from
 * @param amount :number of item to remove
 * @return 0:sucess, 1:fail
 */
int storage_delitem(struct map_session_data* sd, struct s_storage *stor, int index, int amount)
{
	if( stor->u.items_storage[index].nameid == 0 || stor->u.items_storage[index].amount < amount )
		return 1;

	stor->u.items_storage[index].amount -= amount;
	stor->dirty = true;

	if( stor->u.items_storage[index].amount == 0 ) {
		memset(&stor->u.items_storage[index],0,sizeof(stor->u.items_storage[0]));
		stor->amount--;
		if( sd->state.storage_flag == 1 || sd->state.storage_flag == 3 )
			clif_updatestorageamount(sd, stor->amount, stor->max_amount);
	}

	if( sd->state.storage_flag == 1 || sd->state.storage_flag == 3 )
		clif_storageitemremoved(sd,index,amount);

	return 0;
}

/**
 * Add an item to the storage from the inventory.
 * @param sd : player
 * @param stor : Storage data
 * @param index : inventory index to take the item from
 * @param amount : number of item to take
 * @return 0:fail, 1:success
 */
void storage_storageadd(struct map_session_data* sd, struct s_storage *stor, int index, int amount)
{
	enum e_storage_add result;

	nullpo_retv(sd);

	result = storage_canAddItem(stor, index, sd->inventory.u.items_inventory, amount, MAX_INVENTORY);
	if (result == STORAGE_ADD_INVALID)
		return;
	else if (result == STORAGE_ADD_OK) {
		switch( storage_additem(sd, stor, &sd->inventory.u.items_inventory[index], amount) ){
			case 0:
				pc_delitem(sd,index,amount,0,4,LOG_TYPE_STORAGE);
				return;
			case 1:
				break;
			case 2:
				result = STORAGE_ADD_NOROOM;
				break;
		}
	}

	clif_storageitemremoved(sd,index,0);
	clif_dropitem(sd,index,0);
}

/**
 * Retrieve an item from the storage into inventory
 * @param sd : player
 * @param index : storage index to take the item from
 * @param amount : number of item to take
 * @return 0:fail, 1:success
 */
void storage_storageget(struct map_session_data *sd, struct s_storage *stor, int index, int amount)
{
	unsigned char flag = 0;
	enum e_storage_add result;

	nullpo_retv(sd);

	result = storage_canGetItem(stor, index, amount);
	if (result != STORAGE_ADD_OK)
		return;

	if ((flag = pc_additem(sd,&stor->u.items_storage[index],amount,LOG_TYPE_STORAGE)) == ADDITEM_SUCCESS)
		storage_delitem(sd,stor,index,amount);
	else {
		clif_storageitemremoved(sd,index,0);
		clif_additem(sd,0,0,flag);
	}
}

/**
 * Move an item from cart to storage.
 * @param sd : player
 * @param stor : Storage data
 * @param index : cart index to take the item from
 * @param amount : number of item to take
 * @return 0:fail, 1:success
 */
void storage_storageaddfromcart(struct map_session_data *sd, struct s_storage *stor, int index, int amount)
{
	enum e_storage_add result;
	nullpo_retv(sd);

	if (sd->state.prevend) {
		return;
	}

	result = storage_canAddItem(stor, index, sd->cart.u.items_inventory, amount, MAX_CART);
	if (result == STORAGE_ADD_INVALID)
		return;
	else if (result == STORAGE_ADD_OK) {
		switch( storage_additem(sd, stor, &sd->cart.u.items_cart[index], amount) ){
			case 0:
				pc_cart_delitem(sd,index,amount,0,LOG_TYPE_STORAGE);
				return;
			case 1:
				break;
			case 2:
				result = STORAGE_ADD_NOROOM;
				break;
		}
	}

	clif_storageitemremoved(sd,index,0);
	clif_dropitem(sd,index,0);
}

/**
 * Get from Storage to the Cart inventory
 * @param sd : player
 * @param stor : Storage data
 * @param index : storage index to take the item from
 * @param amount : number of item to take
 * @return 0:fail, 1:success
 */
void storage_storagegettocart(struct map_session_data* sd, struct s_storage *stor, int index, int amount)
{
	unsigned char flag = 0;
	enum e_storage_add result;

	nullpo_retv(sd);

	if (sd->state.prevend) {
		return;
	}

	result = storage_canGetItem(stor, index, amount);
	if (result != STORAGE_ADD_OK)
		return;

	if ((flag = pc_cart_additem(sd,&stor->u.items_storage[index],amount,LOG_TYPE_STORAGE)) == 0)
		storage_delitem(sd,stor,index,amount);
	else {
		clif_storageitemremoved(sd,index,0);
		clif_cart_additem_ack(sd,(flag==1)?ADDITEM_TO_CART_FAIL_WEIGHT:ADDITEM_TO_CART_FAIL_COUNT);
	}
}

/**
 * Request to save storage
 * @param sd: Player who has the storage
 */
void storage_storagesave(struct map_session_data *sd)
{
	nullpo_retv(sd);

	intif_storage_save(sd, &sd->storage);
}

/**
 * Ack of storage has been saved
 * @param sd: Player who has the storage
 */
void storage_storagesaved(struct map_session_data *sd)
{
	if (!sd)
		return;

	sd->storage.dirty = false;

	if (sd->state.storage_flag == 1) {
		sd->state.storage_flag = 0;
		clif_storageclose(sd);
	}
}

/**
 * Make player close his storage
 * @param sd: Player who has the storage
 * @author [massdriller] / modified by [Valaris]
 */
void storage_storageclose(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (sd->storage.dirty) {
		if (save_settings&CHARSAVE_STORAGE)
			chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
		else
			storage_storagesave(sd);
		if (sd->state.storage_flag == 1) {
			sd->state.storage_flag = 0;
			clif_storageclose(sd);
		}
	} else
		storage_storagesaved(sd);
}

/**
 * Force closing the storage for player without displaying result
 * (example when quitting the game)
 * @param sd : player to close storage
 * @param flag :
 *  1: Character is quitting
 *	2(x): Character is changing map-servers 
 */
void storage_storage_quit(struct map_session_data* sd, int flag)
{
	nullpo_retv(sd);

	if (save_settings&CHARSAVE_STORAGE)
		chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
	else
		storage_storagesave(sd);
}

/**
 * Retrieve the guild_storage of a guild
 * will create a new storage if none found for the guild
 * @param guild_id : id of the guild
 * @return s_storage
 */
struct s_storage *guild2storage(int guild_id)
{
	struct s_storage *gs;

	if (guild_search(guild_id) == nullptr)
		return nullptr;

	gs = guild2storage2(guild_id);
	
	if( gs == nullptr ){
		gs = &guild_storage_db[guild_id];
		gs->id = guild_id;
		gs->type = TABLE_GUILD_STORAGE;
	}

	return gs;
}

/**
 * See if the guild_storage exist in db and fetch it if it's the case
 * @author : [Skotlex]
 * @param guild_id : guild_id to search the storage
 * @return s_storage or nullptr
 */
struct s_storage *guild2storage2(int guild_id){
	return util::map_find( guild_storage_db, guild_id );
}

/**
 * Delete a guild_storage and remove it from db
 * @param guild_id : guild to remove the storage from
 */
void storage_guild_delete(int guild_id)
{
	guild_storage_db.erase(guild_id);
}

/**
 * Attempt to open guild storage for player
 * @param sd : player
 * @return 0 : success, 1 : fail, 2 : no guild found
 */
char storage_guild_storageopen(struct map_session_data* sd)
{
	struct s_storage *gstor;

	nullpo_ret(sd);

	if(sd->status.guild_id <= 0)
		return GSTORAGE_NO_GUILD;

#ifdef OFFICIAL_GUILD_STORAGE
	if (!guild_checkskill(sd->guild, GD_GUILD_STORAGE))
		return GSTORAGE_NO_STORAGE; // Can't open storage if the guild has not learned the skill
#endif

	if (sd->state.storage_flag == 2)
		return GSTORAGE_ALREADY_OPEN; // Guild storage already open.
	else if (sd->state.storage_flag)
		return GSTORAGE_STORAGE_ALREADY_OPEN; // Can't open both storages at a time.

#if PACKETVER >= 20140205
	int pos;

	if ((pos = guild_getposition(sd)) < 0 || !(sd->guild->position[pos].mode&GUILD_PERM_STORAGE))
		return GSTORAGE_NO_PERMISSION; // Guild member doesn't have permission
#endif

	if( !pc_can_give_items(sd) ) { //check is this GM level can open guild storage and store items [Lupus]
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return GSTORAGE_ALREADY_OPEN;
	}

	if((gstor = guild2storage2(sd->status.guild_id)) == nullptr
#ifdef OFFICIAL_GUILD_STORAGE
		|| (gstor->max_amount != guild_checkskill(sd->guild, GD_GUILD_STORAGE) * 100)
#endif
	) {
		intif_request_guild_storage(sd->status.account_id,sd->status.guild_id);
		return GSTORAGE_OPEN;
	}

	if(gstor->status)
		return GSTORAGE_ALREADY_OPEN;

	if( gstor->lock )
		return GSTORAGE_ALREADY_OPEN;

	gstor->status = true;
	sd->state.storage_flag = 2;
	storage_sortitem(gstor->u.items_guild, ARRAYLENGTH(gstor->u.items_guild));
	clif_storagelist(sd, gstor->u.items_guild, ARRAYLENGTH(gstor->u.items_guild), "Guild Storage");
	clif_updatestorageamount(sd, gstor->amount, gstor->max_amount);

	return GSTORAGE_OPEN;
}

void storage_guild_log( struct map_session_data* sd, struct item* item, int16 amount ){
	int i;
	SqlStmt* stmt = SqlStmt_Malloc(mmysql_handle);
	StringBuf buf;
	StringBuf_Init(&buf);

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`time`, `guild_id`, `char_id`, `name`, `nameid`, `amount`, `identify`, `refine`, `attribute`, `unique_id`, `bound`", guild_storage_log_table);
	for (i = 0; i < MAX_SLOTS; ++i)
		StringBuf_Printf(&buf, ", `card%d`", i);
	for (i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
		StringBuf_Printf(&buf, ", `option_id%d`", i);
		StringBuf_Printf(&buf, ", `option_val%d`", i);
		StringBuf_Printf(&buf, ", `option_parm%d`", i);
	}
	StringBuf_Printf(&buf, ") VALUES(NOW(),'%u','%u', '%s', '%d', '%d','%d','%d','%d','%" PRIu64 "','%d'",
		sd->status.guild_id, sd->status.char_id, sd->status.name, item->nameid, amount, item->identify, item->refine,item->attribute, item->unique_id, item->bound);

	for (i = 0; i < MAX_SLOTS; i++)
		StringBuf_Printf(&buf, ",'%d'", item->card[i]);
	for (i = 0; i < MAX_ITEM_RDM_OPT; i++)
		StringBuf_Printf(&buf, ",'%d','%d','%d'", item->option[i].id, item->option[i].value, item->option[i].param);
	StringBuf_Printf(&buf, ")");

	if (SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf)) || SQL_SUCCESS != SqlStmt_Execute(stmt))
		SqlStmt_ShowDebug(stmt);

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
}

enum e_guild_storage_log storage_guild_log_read_sub( struct map_session_data* sd, std::vector<struct guild_log_entry>& log, uint32 max ){
	StringBuf buf;
	int j;

	StringBuf_Init(&buf);

	StringBuf_AppendStr(&buf, "SELECT `id`, `time`, `name`, `amount`");
	StringBuf_AppendStr(&buf, " , `nameid`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`");
	for (j = 0; j < MAX_SLOTS; ++j)
		StringBuf_Printf(&buf, ", `card%d`", j);
	for (j = 0; j < MAX_ITEM_RDM_OPT; ++j) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `guild_id`='%u'", guild_storage_log_table, sd->status.guild_id );
	StringBuf_Printf(&buf, " ORDER BY `time` DESC LIMIT %u", max);

	SqlStmt* stmt = SqlStmt_Malloc(mmysql_handle);
	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf)) ||
		SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);

		return GUILDSTORAGE_LOG_FAILED;
	}

	struct guild_log_entry entry;

	// General data
	SqlStmt_BindColumn(stmt, 0, SQLDT_UINT,      &entry.id,               0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_STRING,    &entry.time, sizeof(entry.time), NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING,    &entry.name, sizeof(entry.name), NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_SHORT,     &entry.amount,           0, NULL, NULL);

	// Item data
	SqlStmt_BindColumn(stmt, 4, SQLDT_USHORT,    &entry.item.nameid,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,      &entry.item.identify,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,      &entry.item.refine,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_CHAR,      &entry.item.attribute,   0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_UINT,      &entry.item.expire_time, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_UINT,      &entry.item.bound,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 10, SQLDT_UINT64,    &entry.item.unique_id,   0, NULL, NULL);
	for( j = 0; j < MAX_SLOTS; ++j )
		SqlStmt_BindColumn(stmt, 11+j, SQLDT_USHORT, &entry.item.card[j], 0, NULL, NULL);
	for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+j*3, SQLDT_SHORT, &entry.item.option[j].id, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+j*3+1, SQLDT_SHORT, &entry.item.option[j].value, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+j*3+2, SQLDT_CHAR, &entry.item.option[j].param, 0, NULL, NULL);
	}

	log.reserve(max);

	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) ){
		log.push_back( entry );
	}

	Sql_FreeResult(mmysql_handle);
	StringBuf_Destroy(&buf);
	SqlStmt_Free(stmt);

	if( log.empty() ){
		return GUILDSTORAGE_LOG_EMPTY;
	}

	return GUILDSTORAGE_LOG_FINAL_SUCCESS;
}

enum e_guild_storage_log storage_guild_log_read( struct map_session_data* sd ){
	std::vector<struct guild_log_entry> log;

	// ( 65535(maximum packet size) - 8(header) ) / 83 (entry size) = 789 (-1 for safety)
	enum e_guild_storage_log ret = storage_guild_log_read_sub( sd, log, 788 );

	clif_guild_storage_log( sd, log, ret );

	return ret;
}

/**
 * Attempt to add an item in guild storage, then refresh it
 * @param sd : player attempting to open the guild_storage
 * @param stor : guild_storage
 * @param item : item to add
 * @param amount : number of item to add
 * @return True : success, False : fail
 */
bool storage_guild_additem(struct map_session_data* sd, struct s_storage* stor, struct item* item_data, int amount)
{
	struct item_data *id;
	int i;

	nullpo_ret(sd);
	nullpo_ret(stor);
	nullpo_ret(item_data);

	if(item_data->nameid == 0 || amount <= 0)
		return false;

	id = itemdb_search(item_data->nameid);

	if( id->stack.guildstorage && amount > id->stack.amount ) // item stack limitation
		return false;

	if (!itemdb_canguildstore(item_data, pc_get_group_level(sd)) || item_data->expire_time) { // Check if item is storable. [Skotlex]
		clif_displaymessage (sd->fd, msg_txt(sd,264));
		return false;
	}

	if ((item_data->bound == BOUND_ACCOUNT || item_data->bound > BOUND_GUILD) && !pc_can_give_bounded_items(sd)) {
		clif_displaymessage(sd->fd, msg_txt(sd,294));
		return false;
	}

	if(itemdb_isstackable2(id)) { //Stackable
		for(i = 0; i < stor->max_amount; i++) {
			if(compare_item(&stor->u.items_guild[i], item_data)) {
				if( amount > MAX_AMOUNT - stor->u.items_guild[i].amount || ( id->stack.guildstorage && amount > id->stack.amount - stor->u.items_guild[i].amount ) )
					return false;

				stor->u.items_guild[i].amount += amount;
				clif_storageitemadded(sd,&stor->u.items_guild[i],i,amount);
				stor->dirty = true;

				storage_guild_log( sd, &stor->u.items_guild[i], amount );

				return true;
			}
		}
	}

	//Add item
	for(i = 0; i < stor->max_amount && stor->u.items_guild[i].nameid; i++);
	if(i >= stor->max_amount)
		return false;

	memcpy(&stor->u.items_guild[i],item_data,sizeof(stor->u.items_guild[0]));
	stor->u.items_guild[i].amount = amount;
	stor->amount++;
	clif_storageitemadded(sd,&stor->u.items_guild[i],i,amount);
	clif_updatestorageamount(sd, stor->amount, stor->max_amount);
	stor->dirty = true;

	storage_guild_log( sd, &stor->u.items_guild[i], amount );

	return true;
}

/**
 * Attempt to add an item in guild storage, then refresh i
 * @param stor : guild_storage
 * @param item : item to add
 * @param amount : number of item to add
 * @return True : success, False : fail
 */
bool storage_guild_additem2(struct s_storage* stor, struct item* item, int amount) {
	struct item_data *id;
	int i;

	nullpo_ret(stor);
	nullpo_ret(item);

	if (item->nameid == 0 || amount <= 0 || !(id = itemdb_exists(item->nameid)))
		return false;

	if (item->expire_time)
		return false;

	if (itemdb_isstackable2(id)) { // Stackable
		for (i = 0; i < stor->max_amount; i++) {
			if (compare_item(&stor->u.items_guild[i], item)) {
				// Set the amount, make it fit with max amount
				amount = min(amount, ((id->stack.guildstorage) ? id->stack.amount : MAX_AMOUNT) - stor->u.items_guild[i].amount);
				if (amount != item->amount)
					ShowWarning("storage_guild_additem2: Stack limit reached! Altered amount of item \"" CL_WHITE "%s" CL_RESET "\" (%d). '" CL_WHITE "%d" CL_RESET "' -> '" CL_WHITE"%d" CL_RESET "'.\n", id->name, id->nameid, item->amount, amount);
				stor->u.items_guild[i].amount += amount;
				stor->dirty = true;
				return true;
			}
		}
	}

	// Add the item
	for (i = 0; i < stor->max_amount && stor->u.items_guild[i].nameid; i++);
	if (i >= stor->max_amount)
		return false;

	memcpy(&stor->u.items_guild[i], item, sizeof(stor->u.items_guild[0]));
	stor->u.items_guild[i].amount = amount;
	stor->amount++;
	stor->dirty = true;
	return true;
}

/**
 * Attempt to delete an item in guild storage, then refresh it
 * @param sd : player
 * @param stor : guild_storage
 * @param n : index of item in guild storage
 * @param amount : number of item to delete
 * @return True : success, False : fail
 */
bool storage_guild_delitem(struct map_session_data* sd, struct s_storage* stor, int n, int amount)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, stor);

	if(!stor->u.items_guild[n].nameid || stor->u.items_guild[n].amount < amount)
		return false;

	// Log before removing it
	storage_guild_log( sd, &stor->u.items_guild[n], -amount );

	stor->u.items_guild[n].amount -= amount;

	if(!stor->u.items_guild[n].amount) {
		memset(&stor->u.items_guild[n],0,sizeof(stor->u.items_guild[0]));
		stor->amount--;
		clif_updatestorageamount(sd, stor->amount, stor->max_amount);
	}

	clif_storageitemremoved(sd,n,amount);
	stor->dirty = true;
	return true;
}

/**
 * Attempt to add an item in guild storage from inventory, then refresh it
 * @param sd : player
 * @param amount : number of item to delete
 */
void storage_guild_storageadd(struct map_session_data* sd, int index, int amount)
{
	struct s_storage *stor;

	nullpo_retv(sd);
	nullpo_retv(stor = guild2storage2(sd->status.guild_id));

	if( !stor->status || stor->amount > stor->max_amount )
		return;

	if( index < 0 || index >= MAX_INVENTORY )
		return;

	if( sd->inventory.u.items_inventory[index].nameid == 0 )
		return;

	if( amount < 1 || amount > sd->inventory.u.items_inventory[index].amount )
		return;

	if( stor->lock ) {
		storage_guild_storageclose(sd);
		return;
	}

	if(storage_guild_additem(sd,stor,&sd->inventory.u.items_inventory[index],amount))
		pc_delitem(sd,index,amount,0,4,LOG_TYPE_GSTORAGE);
	else {
		clif_storageitemremoved(sd,index,0);
		clif_dropitem(sd,index,0);
	}
}

/**
 * Attempt to retrieve an item from guild storage to inventory, then refresh it
 * @param sd : player
 * @param index : index of item in storage
 * @param amount : number of item to get
 * @return 1:success, 0:fail
 */
void storage_guild_storageget(struct map_session_data* sd, int index, int amount)
{
	struct s_storage *stor;
	unsigned char flag = 0;

	nullpo_retv(sd);
	nullpo_retv(stor = guild2storage2(sd->status.guild_id));

	if(!stor->status)
		return;

	if(index < 0 || index >= stor->max_amount)
		return;

	if(stor->u.items_guild[index].nameid == 0)
		return;

	if(amount < 1 || amount > stor->u.items_guild[index].amount)
		return;

	if( stor->lock ) {
		storage_guild_storageclose(sd);
		return;
	}

	if((flag = pc_additem(sd,&stor->u.items_guild[index],amount,LOG_TYPE_GSTORAGE)) == 0)
		storage_guild_delitem(sd,stor,index,amount);
	else { // inform fail
		clif_storageitemremoved(sd,index,0);
		clif_additem(sd,0,0,flag);
	}
}

/**
 * Attempt to add an item in guild storage from cart, then refresh it
 * @param sd : player
 * @param index : index of item in cart
 * @param amount : number of item to transfer
 */
void storage_guild_storageaddfromcart(struct map_session_data* sd, int index, int amount)
{
	struct s_storage *stor;

	nullpo_retv(sd);
	nullpo_retv(stor = guild2storage2(sd->status.guild_id));

	if( !stor->status || stor->amount > stor->max_amount )
		return;

	if( index < 0 || index >= MAX_CART )
		return;

	if( sd->cart.u.items_cart[index].nameid == 0 )
		return;

	if( amount < 1 || amount > sd->cart.u.items_cart[index].amount )
		return;

	if(storage_guild_additem(sd,stor,&sd->cart.u.items_cart[index],amount))
		pc_cart_delitem(sd,index,amount,0,LOG_TYPE_GSTORAGE);
	else {
		clif_storageitemremoved(sd,index,0);
		clif_dropitem(sd,index,0);
	}
}

/**
 * Attempt to retrieve an item from guild storage to cart, then refresh it
 * @param sd : player
 * @param index : index of item in storage
 * @param amount : number of item to transfer
 * @return 1:fail, 0:success
 */
void storage_guild_storagegettocart(struct map_session_data* sd, int index, int amount)
{
	short flag;
	struct s_storage *stor;

	nullpo_retv(sd);
	nullpo_retv(stor = guild2storage2(sd->status.guild_id));

	if(!stor->status)
		return;

	if(index < 0 || index >= stor->max_amount)
		return;

	if(stor->u.items_guild[index].nameid == 0)
		return;

	if(amount < 1 || amount > stor->u.items_guild[index].amount)
		return;

	if((flag = pc_cart_additem(sd,&stor->u.items_guild[index],amount,LOG_TYPE_GSTORAGE)) == 0)
		storage_guild_delitem(sd,stor,index,amount);
	else {
		clif_storageitemremoved(sd,index,0);
		clif_cart_additem_ack(sd,(flag == 1) ? ADDITEM_TO_CART_FAIL_WEIGHT:ADDITEM_TO_CART_FAIL_COUNT);
	}
}

/**
 * Request to save guild storage
 * @param account_id : account requesting the save
 * @param guild_id : guild to take the guild_storage
 * @param flag : 1=char quitting, close the storage
 * @return False : fail (no storage), True : success (requested)
 */
bool storage_guild_storagesave(uint32 account_id, int guild_id, int flag)
{
	struct s_storage *stor = guild2storage2(guild_id);

	if (stor) {
		if (flag&CSAVE_QUIT) //Char quitting, close it.
			stor->status = false;

		if (stor->dirty)
			intif_send_guild_storage(account_id,stor);

		return true;
	}

	return false;
}

/**
 * ACK save of guild storage
 * @param guild_id : guild to use the storage
 */
void storage_guild_storagesaved(int guild_id)
{
	struct s_storage *stor;

	if ((stor = guild2storage2(guild_id)) != NULL) {
		if (stor->dirty && !stor->status) // Storage has been correctly saved.
			stor->dirty = false;
	}
}

/**
 * Close storage for player then save it
 * @param sd : player
 */
void storage_guild_storageclose(struct map_session_data* sd)
{
	struct s_storage *stor;

	nullpo_retv(sd);
	nullpo_retv(stor = guild2storage2(sd->status.guild_id));

	clif_storageclose(sd);
	if (stor->status) {
		if (save_settings&CHARSAVE_STORAGE)
			chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART); //This one also saves the storage. [Skotlex]
		else
			storage_guild_storagesave(sd->status.account_id, sd->status.guild_id,0);

		stor->status = false;
	}

	sd->state.storage_flag = 0;
}

/**
 * Close storage for player then save it
 * @param sd
 * @param flag
 */
void storage_guild_storage_quit(struct map_session_data* sd, int flag)
{
	struct s_storage *stor;

	nullpo_retv(sd);
	nullpo_retv(stor = guild2storage2(sd->status.guild_id));

	if (flag) {	//Only during a guild break flag is 1 (don't save storage)
		clif_storageclose(sd);

		if (save_settings&CHARSAVE_STORAGE)
			chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);

		sd->state.storage_flag = 0;
		stor->status = false;
		return;
	}

	if (stor->status) {
		if (save_settings&CHARSAVE_STORAGE)
			chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
		else
			storage_guild_storagesave(sd->status.account_id,sd->status.guild_id,1);
	}

	sd->state.storage_flag = 0;
	stor->status = false;
}

/**
 * Open premium storage
 * @param sd Player
 **/
void storage_premiumStorage_open(struct map_session_data *sd) {
	nullpo_retv(sd);

	sd->state.storage_flag = 3;
	storage_sortitem(sd->premiumStorage.u.items_storage, sd->premiumStorage.max_amount);
	clif_storagelist(sd, sd->premiumStorage.u.items_storage, sd->premiumStorage.max_amount, storage_getName(sd->premiumStorage.stor_id));
	clif_updatestorageamount(sd, sd->premiumStorage.amount, sd->premiumStorage.max_amount);
}

/**
 * Request to open premium storage
 * @param sd Player who request
 * @param num Storage number
 * @param mode Storage mode @see enum e_storage_mode
 * @return 1:Success to request, 0:Failed
 * @author [Cydh]
 **/
bool storage_premiumStorage_load(struct map_session_data *sd, uint8 num, uint8 mode) {
	nullpo_ret(sd);

	if (sd->state.storage_flag)
		return 0;

	if (sd->state.vending || sd->state.buyingstore || sd->state.prevend || sd->state.autotrade)
		return 0;

	if (sd->state.banking || sd->state.callshop)
		return 0;

	if (!pc_can_give_items(sd)) { // check is this GM level is allowed to put items to storage
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return 0;
	}

	if (sd->premiumStorage.stor_id != num)
		return intif_storage_request(sd, TABLE_STORAGE, num, mode);
	else {
		sd->premiumStorage.state.put = (mode&STOR_MODE_PUT) ? 1 : 0;
		sd->premiumStorage.state.get = (mode&STOR_MODE_GET) ? 1 : 0;
		storage_premiumStorage_open(sd);
	}
	return 1;
}

/**
 * Request to save premium storage
 * @param sd Player who has the storage
 * @author [Cydh]
 **/
void storage_premiumStorage_save(struct map_session_data *sd) {
	nullpo_retv(sd);

	intif_storage_save(sd, &sd->premiumStorage);
}

/**
 * Ack of secondary premium has been saved
 * @param sd Player who has the storage
 **/
void storage_premiumStorage_saved(struct map_session_data *sd) {
	if (!sd)
		return;

	sd->premiumStorage.dirty = 0;

	if (sd->state.storage_flag == 3) {
		sd->state.storage_flag = 0;
		clif_storageclose(sd);
	}
}

/**
 * Request to close premium storage
 * @param sd Player who has the storage
 * @author [Cydh]
 **/
void storage_premiumStorage_close(struct map_session_data *sd) {
	nullpo_retv(sd);

	if (sd->premiumStorage.dirty) {
		if (save_settings&CHARSAVE_STORAGE)
			chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
		else
			storage_premiumStorage_save(sd);
		if (sd->state.storage_flag == 3) {
			sd->state.storage_flag = 0;
			clif_storageclose(sd);
		}
	}
	else 
		storage_premiumStorage_saved(sd);
}

/**
 * Force save then close the premium storage
 * @param sd Player who has the storage
 * @author [Cydh]
 **/
void storage_premiumStorage_quit(struct map_session_data *sd) {
	nullpo_retv(sd);

	if (save_settings&CHARSAVE_STORAGE)
		chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
	else
		storage_premiumStorage_save(sd);
}
