// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma warning(disable:4800) //forcing value to bool
#include "int_storage.hpp"

#include <stdlib.h>
#include <string.h>

#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp" // StringBuf

#include "char.hpp"
#include "inter.hpp"
#include "int_guild.hpp"

/**
 * Get max storage amount
 * @param id: Storage ID
 * @return Max amount
 */
int inter_premiumStorage_getMax(uint8 id) {
	std::shared_ptr<s_storage_table> storage = interServerDb.find( id );

	if( storage != nullptr ){
		return storage->max_num;
	}

	return MAX_STORAGE;
}

/**
 * Get table name of storage
 * @param id: Storage ID
 * @return Table name
 */
const char *inter_premiumStorage_getTableName(uint8 id) {
	std::shared_ptr<s_storage_table> storage = interServerDb.find( id );

	if( storage != nullptr ){
		return storage->table;
	}

	return schema_config.storage_db;
}

/**
 * Get printable name of storage
 * @param id: Storage ID
 * @return printable name
 */
const char *inter_premiumStorage_getPrintableName(uint8 id) {
	std::shared_ptr<s_storage_table> storage = interServerDb.find( id );

	if( storage != nullptr ){
		return storage->name;
	}

	return "Storage";
}

/**
 * Save inventory entries to SQL
 * @param char_id: Character ID to save
 * @param p: Inventory entries
 * @return 0 if success, or error count
 */
int inventory_tosql(uint32 char_id, struct s_storage* p)
{
	return char_memitemdata_to_sql(p->u.items_inventory, MAX_INVENTORY, char_id, TABLE_INVENTORY, p->stor_id);
}

/**
 * Save storage entries to SQL
 * @param char_id: Character ID to save
 * @param p: Storage entries
 * @return 0 if success, or error count
 */
int storage_tosql(uint32 account_id, struct s_storage* p)
{
	return char_memitemdata_to_sql(p->u.items_storage, MAX_STORAGE, account_id, TABLE_STORAGE, p->stor_id);
}

/**
 * Save cart entries to SQL
 * @param char_id: Character ID to save
 * @param p: Cart entries
 * @return 0 if success, or error count
 */
int cart_tosql(uint32 char_id, struct s_storage* p)
{
	return char_memitemdata_to_sql(p->u.items_cart, MAX_CART, char_id, TABLE_CART, p->stor_id);
}

/**
 * Fetch inventory entries from table
 * @param char_id: Character ID to fetch
 * @param p: Inventory entries
 * @return True if success, False if failed
 */
bool inventory_fromsql(uint32 char_id, struct s_storage* p)
{
	return char_memitemdata_from_sql( p, MAX_INVENTORY, char_id, TABLE_INVENTORY, p->stor_id );
}

/**
 * Fetch cart entries from table
 * @param char_id: Character ID to fetch
 * @param p: Cart entries
 * @return True if success, False if failed
 */
bool cart_fromsql(uint32 char_id, struct s_storage* p)
{
	return char_memitemdata_from_sql( p, MAX_CART, char_id, TABLE_CART, p->stor_id );
}

/**
 * Fetch storage entries from table
 * @param account_id: Account ID to fetch
 * @param p: Storage entries
 * @param stor_id: Storage ID
 * @return True if success, False if failed
 */
bool storage_fromsql(uint32 account_id, struct s_storage* p)
{
	return char_memitemdata_from_sql( p, MAX_STORAGE, account_id, TABLE_STORAGE, p->stor_id );
}

/**
 * Save guild_storage data to sql
 * @param guild_id: Guild ID to save
 * @param p: Guild Storage entries
 * @return True if success, False if failed
 */
bool guild_storage_tosql(int guild_id, struct s_storage* p)
{
	//ShowInfo("Guild Storage has been saved (GID: %d)\n", guild_id);
	return char_memitemdata_to_sql(p->u.items_guild, inter_guild_storagemax(guild_id), guild_id, TABLE_GUILD_STORAGE, p->stor_id);
}

/**
 * Fetch guild_storage entries from table
 * @param guild_id: Guild ID to fetch
 * @param p: Guild Storage entries
 * @return True if success, False if failed
 */
bool guild_storage_fromsql(int guild_id, struct s_storage* p)
{
	return char_memitemdata_from_sql( p, inter_guild_storagemax(guild_id), guild_id, TABLE_GUILD_STORAGE, p->stor_id );
}

void inter_storage_checkDB(void) {
	// Checking storage tables
	for( auto storage_table : interServerDb ){
		if (SQL_ERROR == Sql_Query(sql_handle, "SELECT  `id`,`account_id`,`nameid`,`amount`,`equip`,`identify`,`refine`,"
			"`attribute`,`card0`,`card1`,`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,"
			"`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,"
			"`expire_time`,`bound`,`unique_id`"
			" FROM `%s` LIMIT 1;", storage_table.second->table)) {
			Sql_ShowDebug(sql_handle);
		}else{
			Sql_FreeResult(sql_handle);
		}
	}
}

//---------------------------------------------------------
// storage data initialize
void inter_storage_sql_init(void)
{
	inter_storage_checkDB();
	return;
}

// storage data finalize
void inter_storage_sql_final(void)
{
	return;
}

//---------------------------------------------------------
// packet from map server

/**
 * Send guild storage data to the map server
 * @param fd: Map server's fd
 * @param account_id: Account ID requesting
 * @param guild_id: Guild ID requesting
 * @param flag: Additional parameters
 * @return True on success or false on failure
 */
bool mapif_load_guild_storage(int fd,uint32 account_id,int guild_id, char flag)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `guild_id` FROM `%s` WHERE `guild_id`='%d'", schema_config.guild_db, guild_id) )
		Sql_ShowDebug(sql_handle);
	else if( Sql_NumRows(sql_handle) > 0 )
	{// guild exists
		WFIFOHEAD(fd, sizeof(struct s_storage)+13);
		WFIFOW(fd,0) = 0x3818;
		WFIFOW(fd,2) = sizeof(struct s_storage)+13;
		WFIFOL(fd,4) = account_id;
		WFIFOL(fd,8) = guild_id;
		WFIFOB(fd,12) = flag; //1 open storage, 0 don't open
		guild_storage_fromsql(guild_id, (struct s_storage*)WFIFOP(fd,13));
		WFIFOSET(fd, WFIFOW(fd,2));
		return true;
	}
	// guild does not exist
	Sql_FreeResult(sql_handle);
	WFIFOHEAD(fd, 12);
	WFIFOW(fd,0) = 0x3818;
	WFIFOW(fd,2) = 12;
	WFIFOL(fd,4) = account_id;
	WFIFOL(fd,8) = 0;
	WFIFOSET(fd, 12);
	return false;
}

void mapif_save_guild_storage_ack(int fd,uint32 account_id,int guild_id,int fail)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0)=0x3819;
	WFIFOL(fd,2)=account_id;
	WFIFOL(fd,6)=guild_id;
	WFIFOB(fd,10)=fail;
	WFIFOSET(fd,11);
}

//---------------------------------------------------------
// packet from map server

void mapif_parse_LoadGuildStorage(int fd)
{
	RFIFOHEAD(fd);
	mapif_load_guild_storage(fd,RFIFOL(fd,2),RFIFOL(fd,6),1);
}

/**
 * Save guild storage data from map server
 * @param fd: Map server's fd
 * @return True on success or false on failure
 */
bool mapif_parse_SaveGuildStorage(int fd)
{
	int guild_id;
	int len;

	RFIFOHEAD(fd);
	guild_id = RFIFOL(fd,8);
	len = RFIFOW(fd,2);

	if( sizeof(struct s_storage) != len - 12 )
	{
		ShowError("inter storage: data size error %" PRIuPTR " != %d\n", sizeof(struct s_storage), len - 12);
	}
	else
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `guild_id` FROM `%s` WHERE `guild_id`='%d'", schema_config.guild_db, guild_id) )
			Sql_ShowDebug(sql_handle);
		else if( Sql_NumRows(sql_handle) > 0 )
		{// guild exists
			Sql_FreeResult(sql_handle);
			guild_storage_tosql(guild_id, (struct s_storage*)RFIFOP(fd,12));
			mapif_save_guild_storage_ack(fd, RFIFOL(fd,4), guild_id, 0);
			return false;
		}
		Sql_FreeResult(sql_handle);
	}
	mapif_save_guild_storage_ack(fd, RFIFOL(fd,4), guild_id, 1);
	return true;
}

#ifdef BOUND_ITEMS
/**
 * IZ 0x3856 <account_id>.L <guild_id>.W
 * Tells map-server if the process if complete, unlock the guild storage
 */
void mapif_itembound_ack(int fd, int account_id, int guild_id)
{
	WFIFOHEAD(fd,8);
	WFIFOW(fd,0) = 0x3856;
	WFIFOL(fd,2) = account_id;
	WFIFOW(fd,6) = guild_id;
	WFIFOSET(fd,8);
	char_unset_session_flag(account_id, 1);
}

/**
 * IZ 0x3857 <size>.W <count>.W <guild_id>.W { <item>.?B }.*MAX_INVENTORY
 * Send the retrieved guild bound items to map-server, store them to guild storage.
 * By using this method, stackable items will looks how it should be, and overflowed
 * item's stack won't disturbs the guild storage table and the leftover items (when
 * storage is full) will be discarded.
 * @param fd
 * @param guild_id
 * @param items[]
 * @param count
 * @author [Cydh]
 */
void mapif_itembound_store2gstorage(int fd, int guild_id, struct item items[], unsigned short count) {
	int size = 8 + sizeof(struct item) * MAX_INVENTORY, i;

	WFIFOHEAD(fd, size);
	WFIFOW(fd, 0) = 0x3857;
	WFIFOW(fd, 2) = size;
	WFIFOW(fd, 6) = guild_id;
	for (i = 0; i < count && i < MAX_INVENTORY; i++) {
		if (!&items[i])
			continue;
		memcpy(WFIFOP(fd, 8 + (i * sizeof(struct item))), &items[i], sizeof(struct item));
	}
	WFIFOW(fd, 4) = i;
	WFIFOSET(fd, size);
}

/**
 * ZI 0x3056 <char_id>.L <account_id>.L <guild_id>.W
 * Pulls guild bound items for offline characters
 * @author [Akinari]
 */
bool mapif_parse_itembound_retrieve(int fd)
{
	StringBuf buf;
	SqlStmt* stmt;
	unsigned short i = 0, count = 0;
	struct item item, items[MAX_INVENTORY];
	int j, guild_id = RFIFOW(fd,10);
	uint32 char_id = RFIFOL(fd,2), account_id = RFIFOL(fd,6);

	StringBuf_Init(&buf);

	// Get bound items from player's inventory
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `char_id`='%d' AND `bound` = %d", schema_config.inventory_db,char_id, BOUND_GUILD);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf)) ||
		SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		mapif_itembound_ack(fd,account_id,guild_id);
		return true;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,       &item.id,          0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_USHORT,    &item.nameid,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,     &item.amount,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT,      &item.equip,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,      &item.identify,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,      &item.refine,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,      &item.attribute,   0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,      &item.expire_time, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_UINT,      &item.bound,       0, NULL, NULL);
	for( j = 0; j < MAX_SLOTS; ++j )
		SqlStmt_BindColumn(stmt, 9+j, SQLDT_USHORT, &item.card[j], 0, NULL, NULL);
	for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
		SqlStmt_BindColumn(stmt, 9+MAX_SLOTS+j*3, SQLDT_SHORT, &item.option[j].id, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 10+MAX_SLOTS+j*3, SQLDT_SHORT, &item.option[j].value, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+j*3, SQLDT_CHAR, &item.option[j].param, 0, NULL, NULL);
	}
	memset(&items, 0, sizeof(items));
	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) )
		memcpy(&items[count++], &item, sizeof(struct item));
	Sql_FreeResult(sql_handle);

	ShowInfo("Found '" CL_WHITE "%d" CL_RESET "' guild bound item(s) from CID = " CL_WHITE "%d" CL_RESET ", AID = %d, Guild ID = " CL_WHITE "%d" CL_RESET ".\n", count, char_id, account_id, guild_id);
	if (!count) { //No items found - No need to continue
		StringBuf_Destroy(&buf);
		SqlStmt_Free(stmt);
		mapif_itembound_ack(fd,account_id,guild_id);
		return true;
	}

	char_set_session_flag(account_id, 1);

	// Delete bound items from player's inventory
	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "DELETE FROM `%s` WHERE `char_id` = %d AND `bound` = %d",schema_config.inventory_db, char_id, BOUND_GUILD);
	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf)) ||
		SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		mapif_itembound_ack(fd,account_id,guild_id);
		return true;
	}

	// Send the deleted items to map-server to store them in guild storage [Cydh]
	mapif_itembound_store2gstorage(fd, guild_id, items, count);

	// Verifies equip bitmasks (see item.equip) and handles the sql statement
#define CHECK_REMOVE(var,mask,token,num) {\
	if ((var)&(mask) && !(j&(num))) {\
		if (j)\
			StringBuf_AppendStr(&buf, ",");\
		StringBuf_AppendStr(&buf, "`"#token"`='0'");\
		j |= (1<<num);\
	}\
}

	StringBuf_Clear(&buf);
	j = 0;
	for (i = 0; i < count && i < MAX_INVENTORY; i++) {
		if (!&items[i] || !items[i].equip)
			continue;
		// Equips can be at more than one slot at the same time
		CHECK_REMOVE(items[i].equip, EQP_HAND_R, weapon, 0);
		CHECK_REMOVE(items[i].equip, EQP_HAND_L, shield, 1);
		CHECK_REMOVE(items[i].equip, EQP_HEAD_TOP|EQP_COSTUME_HEAD_TOP, head_top, 2);
		CHECK_REMOVE(items[i].equip, EQP_HEAD_MID|EQP_COSTUME_HEAD_MID, head_mid, 3);
		CHECK_REMOVE(items[i].equip, EQP_HEAD_LOW|EQP_COSTUME_HEAD_LOW, head_bottom, 4);
		CHECK_REMOVE(items[i].equip, EQP_GARMENT|EQP_COSTUME_GARMENT, robe, 5);
	}

#undef CHECK_REMOVE

	// Update player's view
	if (j) {
		StringBuf buf2;
		StringBuf_Init(&buf2);
		StringBuf_Printf(&buf2, "UPDATE `%s` SET %s WHERE `char_id`='%d'", schema_config.char_db, StringBuf_Value(&buf), char_id);

		if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf)) ||
			SQL_ERROR == SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			StringBuf_Destroy(&buf);
			StringBuf_Destroy(&buf2);
			mapif_itembound_ack(fd,account_id,guild_id);
			return true;
		}
		StringBuf_Destroy(&buf2);
	}

	StringBuf_Destroy(&buf);
	SqlStmt_Free(stmt);

	char_unset_session_flag(account_id, 1);
	return false;
}
#endif

/*==========================================
 * Storages (Inventory/Storage/Cart)
 *------------------------------------------*/

/**
 * Sending inventory/cart/storage data to player
 * IZ 0x388a <size>.W <type>.B <account_id>.L <result>.B <inventory>.?B
 * @param fd
 * @param account_id
 * @param type
 * @param entries Inventory/cart/storage entries
 * @param result
 */
void mapif_storage_data_loaded(int fd, uint32 account_id, char type, struct s_storage* entries, bool result) {
	uint16 size = sizeof(struct s_storage) + 10;
	
	WFIFOHEAD(fd, size);
	WFIFOW(fd, 0) = 0x388a;
	WFIFOW(fd, 2) = size;
	WFIFOB(fd, 4) = type;
	WFIFOL(fd, 5) = account_id;
	WFIFOB(fd, 9) = result;
	memcpy(WFIFOP(fd, 10), entries, sizeof(struct s_storage));
	WFIFOSET(fd, size);
}

/**
 * Tells player if inventory/cart/storage is saved
 * IZ 0x388b <account_id>.L <result>.B <type>.B
 * @param fd
 * @param account_id
 * @param char_id
 * @param success
 * @param type
 * @param stor_id
 */
void mapif_storage_saved(int fd, uint32 account_id, uint32 char_id, bool success, char type, uint8 stor_id) {
	WFIFOHEAD(fd,9);
	WFIFOW(fd, 0) = 0x388b;
	WFIFOL(fd, 2) = account_id;
	WFIFOB(fd, 6) = success;
	WFIFOB(fd, 7) = type;
	WFIFOB(fd, 8) = stor_id;
	WFIFOSET(fd,9);
}

/**
 * Requested inventory/cart/storage data for a player
 * ZI 0x308a <type>.B <account_id>.L <char_id>.L <storage_id>.B <mode>.B
 * @param fd
 */
bool mapif_parse_StorageLoad(int fd) {
	uint32 aid, cid;
	int type;
	uint8 stor_id, mode;
	struct s_storage stor;
	bool res = true;

	type = RFIFOB(fd,2);
	aid = RFIFOL(fd,3);
	cid = RFIFOL(fd,7);
	stor_id = RFIFOB(fd,11);

	memset(&stor, 0, sizeof(struct s_storage));
	stor.stor_id = stor_id;

	//ShowInfo("Loading storage for AID=%d.\n", aid);
	switch (type) {
		case TABLE_INVENTORY: res = inventory_fromsql(cid, &stor); break;
		case TABLE_STORAGE:
			if( !interServerDb.exists( stor_id ) ){
				ShowError( "Invalid storage with id %d\n", stor_id );
				return false;
			}

			res = storage_fromsql(aid, &stor);
			break;
		case TABLE_CART:      res = cart_fromsql(cid, &stor);      break;
		default: return false;
	}

	mode = RFIFOB(fd, 12);
	stor.state.put = (mode&STOR_MODE_PUT) ? 1 : 0;
	stor.state.get = (mode&STOR_MODE_GET) ? 1 : 0;

	mapif_storage_data_loaded(fd, aid, type, &stor, res);
	return true;
}

/**
 * Asking to save player's inventory/cart/storage data
 * ZI 0x308b <size>.W <type>.B <account_id>.L <char_id>.L <entries>.?B
 * @param fd
 */
bool mapif_parse_StorageSave(int fd) {
	int aid, cid, type;
	struct s_storage stor;

	RFIFOHEAD(fd);
	type = RFIFOB(fd, 4);
	aid = RFIFOL(fd, 5);
	cid = RFIFOL(fd, 9);
	
	memset(&stor, 0, sizeof(struct s_storage));
	memcpy(&stor, RFIFOP(fd, 13), sizeof(struct s_storage));

	//ShowInfo("Saving storage data for AID=%d.\n", aid);
	switch(type){
		case TABLE_INVENTORY:	inventory_tosql(cid, &stor); break;
		case TABLE_STORAGE:
			if( !interServerDb.exists( stor.stor_id ) ){
				ShowError( "Invalid storage with id %d\n", stor.stor_id );
				return false;
			}

			storage_tosql(aid, &stor);
			break;
		case TABLE_CART:	cart_tosql(cid, &stor); break;
		default: return false;
	}
	mapif_storage_saved(fd, aid, cid, true, type, stor.stor_id);
	return false;
}


/*==========================================
 * Parse packet from map-server
 *------------------------------------------*/
bool inter_storage_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)){
		case 0x3018: mapif_parse_LoadGuildStorage(fd); break;
		case 0x3019: mapif_parse_SaveGuildStorage(fd); break;
#ifdef BOUND_ITEMS
		case 0x3056: mapif_parse_itembound_retrieve(fd); break;
#endif
		case 0x308a: mapif_parse_StorageLoad(fd); break;
		case 0x308b: mapif_parse_StorageSave(fd); break;
		default:
			return false;
	}
	return true;
}
