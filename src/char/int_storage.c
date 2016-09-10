// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h" // StringBuf
#include "../common/sql.h"
#include "char.h"
#include "inter.h"

#include <stdlib.h>


#define STORAGE_MEMINC	16

/**
 * Save inventory entries to SQL
 * @param char_id: Character ID to save
 * @param p: Inventory entries
 * @return 0 if success, or error count
 */
static int inventory_tosql(uint32 char_id, struct s_storage* p)
{
	return char_memitemdata_to_sql(p->u.items_inventory, MAX_INVENTORY, char_id, TABLE_INVENTORY);
}

/**
 * Save storage entries to SQL
 * @param char_id: Character ID to save
 * @param p: Storage entries
 * @return 0 if success, or error count
 */
static int storage_tosql(uint32 account_id, struct s_storage* p)
{
	return char_memitemdata_to_sql(p->u.items_storage, MAX_STORAGE, account_id, TABLE_STORAGE);
}

/**
 * Save cart entries to SQL
 * @param char_id: Character ID to save
 * @param p: Cart entries
 * @return 0 if success, or error count
 */
static int cart_tosql(uint32 char_id, struct s_storage* p)
{
	return char_memitemdata_to_sql(p->u.items_cart, MAX_CART, char_id, TABLE_CART);
}

/**
 * Fetch inventory entries from table
 * @param char_id: Character ID to fetch
 * @param p: Inventory list to save the entries
 * @return True if success, False if failed
 */
static bool inventory_fromsql(uint32 char_id, struct s_storage* p)
{
	int i;
	StringBuf buf;
	SqlStmt* stmt;
	struct item tmp_item;

	memset(p, 0, sizeof(struct s_storage)); //clean up memory
	p->id = char_id;
	p->type = TABLE_INVENTORY;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return false;
	}

	// storage {`account_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`expire_time`/`favorite`/`bound`/`unique_id`/`card0`/`card1`/`card2`/`card3`/`option_id0`/`option_val0`/`option_parm0`/`option_id1`/`option_val1`/`option_parm1`/`option_id2`/`option_val2`/`option_parm2`/`option_id3`/`option_val3`/`option_parm3`/`option_id4`/`option_val4`/`option_parm4`}
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `favorite`, `bound`, `unique_id`");
	for( i = 0; i < MAX_SLOTS; ++i )
		StringBuf_Printf(&buf, ", `card%d`", i);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		StringBuf_Printf(&buf, ", `option_id%d`", i);
		StringBuf_Printf(&buf, ", `option_val%d`", i);
		StringBuf_Printf(&buf, ", `option_parm%d`", i);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `char_id`=? LIMIT %d", schema_config.inventory_db, MAX_INVENTORY);

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
		||	SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return false;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,       &tmp_item.id,          0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_USHORT,    &tmp_item.nameid,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,     &tmp_item.amount,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT,      &tmp_item.equip,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,      &tmp_item.identify,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,      &tmp_item.refine,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,      &tmp_item.attribute,   0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,      &tmp_item.expire_time, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_CHAR,      &tmp_item.favorite,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_CHAR,      &tmp_item.bound,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt,10, SQLDT_ULONGLONG, &tmp_item.unique_id,   0, NULL, NULL);
	for( i = 0; i < MAX_SLOTS; ++i )
		SqlStmt_BindColumn(stmt, 11+i, SQLDT_USHORT, &tmp_item.card[i], 0, NULL, NULL);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].id, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 12+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].value, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 13+MAX_SLOTS+i*3, SQLDT_CHAR, &tmp_item.option[i].param, 0, NULL, NULL);
 	}

	for( i = 0; i < MAX_INVENTORY && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->u.items_inventory[i], &tmp_item, sizeof(tmp_item));

	p->amount = i;
	ShowInfo("Loaded inventory data from DB - CID: %d (total: %d)\n", char_id, p->amount);

	SqlStmt_FreeResult(stmt);
	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
	return true;
}

/**
 * Fetch cart entries from table
 * @param char_id: Character ID to fetch
 * @param p: Cart list to save the entries
 * @return True if success, False if failed
 */
static bool cart_fromsql(uint32 char_id, struct s_storage* p)
{
	int i,j;
	StringBuf buf;
	SqlStmt* stmt;
	struct item tmp_item;

	memset(p, 0, sizeof(struct s_storage)); //clean up memory
	p->id = char_id;
	p->type = TABLE_CART;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return false;
	}

	// storage {`char_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`expire_time`/`bound`/`unique_id`/`card0`/`card1`/`card2`/`card3`/`option_id0`/`option_val0`/`option_parm0`/`option_id1`/`option_val1`/`option_parm1`/`option_id2`/`option_val2`/`option_parm2`/`option_id3`/`option_val3`/`option_parm3`/`option_id4`/`option_val4`/`option_parm4`}
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ",`card%d`", j);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		StringBuf_Printf(&buf, ", `option_id%d`", i);
		StringBuf_Printf(&buf, ", `option_val%d`", i);
		StringBuf_Printf(&buf, ", `option_parm%d`", i);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `char_id`=? ORDER BY `id` LIMIT %d", schema_config.cart_db, MAX_CART);

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
		||	SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return false;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,         &tmp_item.id,          0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_USHORT,      &tmp_item.nameid,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,       &tmp_item.amount,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT,        &tmp_item.equip,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,        &tmp_item.identify,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,        &tmp_item.refine,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,        &tmp_item.attribute,   0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,        &tmp_item.expire_time, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_CHAR,        &tmp_item.bound,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_ULONGLONG,   &tmp_item.unique_id,   0, NULL, NULL);
	for( i = 0; i < MAX_SLOTS; ++i )
		SqlStmt_BindColumn(stmt, 10+i, SQLDT_USHORT, &tmp_item.card[i], 0, NULL, NULL);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		SqlStmt_BindColumn(stmt, 10+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].id, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].value, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 12+MAX_SLOTS+i*3, SQLDT_CHAR, &tmp_item.option[i].param, 0, NULL, NULL);
 	}

	for( i = 0; i < MAX_CART && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->u.items_cart[i], &tmp_item, sizeof(tmp_item));

	p->amount = i;
	ShowInfo("Loaded Cart data from DB - CID: %d (total: %d)\n", char_id, p->amount);

	SqlStmt_FreeResult(stmt);
	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
	return true;
}

/**
 * Fetch storage entries from table
 * @param char_id: Character ID to fetch
 * @param p: Storage list to save the entries
 * @return True if success, False if failed
 */
static bool storage_fromsql(uint32 account_id, struct s_storage* p)
{
	int i, j;
	StringBuf buf;
	SqlStmt* stmt;
	struct item tmp_item;

	memset(p, 0, sizeof(struct s_storage)); //clean up memory
	p->id = account_id;
	p->type = TABLE_STORAGE;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return false;
	}

	// storage {`account_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`/`option_id0`/`option_val0`/`option_parm0`/`option_id1`/`option_val1`/`option_parm1`/`option_id2`/`option_val2`/`option_parm2`/`option_id3`/`option_val3`/`option_parm3`/`option_id4`/`option_val4`/`option_parm4`}
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		StringBuf_Printf(&buf, ", `option_id%d`", i);
		StringBuf_Printf(&buf, ", `option_val%d`", i);
		StringBuf_Printf(&buf, ", `option_parm%d`", i);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `account_id`=? ORDER BY `nameid` LIMIT %d", schema_config.storage_db, account_id, MAX_STORAGE);

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &account_id, 0)
		||	SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return false;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,           &tmp_item.id,          0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_USHORT,        &tmp_item.nameid,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,         &tmp_item.amount,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT,          &tmp_item.equip,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,          &tmp_item.identify,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,          &tmp_item.refine,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,          &tmp_item.attribute,   0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,          &tmp_item.expire_time, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_CHAR,          &tmp_item.bound,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_ULONGLONG,     &tmp_item.unique_id,   0, NULL, NULL);
	for( i = 0; i < MAX_SLOTS; ++i )
		SqlStmt_BindColumn(stmt, 10+i, SQLDT_USHORT, &tmp_item.card[i],     0, NULL, NULL);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		SqlStmt_BindColumn(stmt, 10+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].id, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].value, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 12+MAX_SLOTS+i*3, SQLDT_CHAR, &tmp_item.option[i].param, 0, NULL, NULL);
 	}

	for( i = 0; i < MAX_STORAGE && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->u.items_storage[i], &tmp_item, sizeof(tmp_item));

	p->amount = i;
	ShowInfo("Loaded Storage data from DB - AID: %d (total: %d)\n", account_id, p->amount);

	SqlStmt_FreeResult(stmt);
	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
	return true;
}

/**
 * Save guild_storage data to sql
 * @param guild_id: Character ID to save
 * @param p: Guild Storage list to save the entries
 * @return 0 if success, or error count
 */
bool guild_storage_tosql(int guild_id, struct s_storage* p)
{
	//ShowInfo("Guild Storage has been saved (GID: %d)\n", guild_id);
	return char_memitemdata_to_sql(p->u.items_guild, MAX_GUILD_STORAGE, guild_id, TABLE_GUILD_STORAGE);
}

/**
 * Fetch guild_storage entries from table
 * @param char_id: Character ID to fetch
 * @param p: Storage list to save the entries
 * @return True if success, False if failed
 */
bool guild_storage_fromsql(int guild_id, struct s_storage* p)
{
	int i,j;
	StringBuf buf;
	SqlStmt* stmt;
	struct item tmp_item;

	memset(p, 0, sizeof(struct s_storage)); //clean up memory
	p->id = guild_id;
	p->type = TABLE_GUILD_STORAGE;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return false;
	}

	// storage {`guild_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`expire_time`/`bound`/`unique_id`/`card0`/`card1`/`card2`/`card3`/`option_id0`/`option_val0`/`option_parm0`/`option_id1`/`option_val1`/`option_parm1`/`option_id2`/`option_val2`/`option_parm2`/`option_id3`/`option_val3`/`option_parm3`/`option_id4`/`option_val4`/`option_parm4`}
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`expire_time`,`bound`,`unique_id`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ",`card%d`", j);
	for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `guild_id`='%d' ORDER BY `nameid`", schema_config.guild_storage_db, guild_id);

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &guild_id, 0)
		||	SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return false;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,          &tmp_item.id,        0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_USHORT,       &tmp_item.nameid,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,        &tmp_item.amount,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT,         &tmp_item.equip,     0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,         &tmp_item.identify,  0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,         &tmp_item.refine,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,         &tmp_item.attribute, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,         &tmp_item.expire_time, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_CHAR,         &tmp_item.bound,     0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_ULONGLONG,    &tmp_item.unique_id, 0, NULL, NULL);
	for( i = 0; i < MAX_SLOTS; ++i )
		SqlStmt_BindColumn(stmt, 10+i, SQLDT_USHORT, &tmp_item.card[i],   0, NULL, NULL);
 	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		SqlStmt_BindColumn(stmt, 10+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].id, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 11+MAX_SLOTS+i*3, SQLDT_SHORT, &tmp_item.option[i].value, 0, NULL, NULL);
		SqlStmt_BindColumn(stmt, 12+MAX_SLOTS+i*3, SQLDT_CHAR, &tmp_item.option[i].param, 0, NULL, NULL);
 	}

	for( i = 0; i < MAX_GUILD_STORAGE && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->u.items_guild[i], &tmp_item, sizeof(tmp_item));

	p->amount = i;
	ShowInfo("Loaded Guild Storage data from DB - GID: %d (total: %d)\n", guild_id, p->amount);

	SqlStmt_FreeResult(stmt);
	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
	return true;
}

//---------------------------------------------------------
// storage data initialize
void inter_storage_sql_init(void)
{
	return;
}

// storage data finalize
void inter_storage_sql_final(void)
{
	return;
}

// Delete char storage
void inter_storage_delete(uint32 account_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id`='%d'", schema_config.storage_db, account_id) )
		Sql_ShowDebug(sql_handle);
}

void inter_guild_storage_delete(int guild_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id`='%d'", schema_config.guild_storage_db, guild_id) )
		Sql_ShowDebug(sql_handle);
}

//---------------------------------------------------------
// packet from map server

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
		return false;
	}
	// guild does not exist
	Sql_FreeResult(sql_handle);
	WFIFOHEAD(fd, 12);
	WFIFOW(fd,0) = 0x3818;
	WFIFOW(fd,2) = 12;
	WFIFOL(fd,4) = account_id;
	WFIFOL(fd,8) = 0;
	WFIFOSET(fd, 12);
	return true;
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

bool mapif_parse_SaveGuildStorage(int fd)
{
	int guild_id;
	int len;

	RFIFOHEAD(fd);
	guild_id = RFIFOL(fd,8);
	len = RFIFOW(fd,2);

	if( sizeof(struct s_storage) != len - 12 )
	{
		ShowError("inter storage: data size error %d != %d\n", sizeof(struct s_storage), len - 12);
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
static void mapif_itembound_ack(int fd, int account_id, int guild_id)
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
static void mapif_itembound_store2gstorage(int fd, int guild_id, struct item items[], unsigned short count) {
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
	SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT,    &item.equip,       0, NULL, NULL);
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

	ShowInfo("Found '"CL_WHITE"%d"CL_RESET"' guild bound item(s) from CID = "CL_WHITE"%d"CL_RESET", AID = %d, Guild ID = "CL_WHITE"%d"CL_RESET".\n", count, char_id, account_id, guild_id);
	if (!count) { //No items found - No need to continue
		StringBuf_Destroy(&buf);
		SqlStmt_Free(stmt);
		mapif_itembound_ack(fd,account_id,guild_id);
		return true;
	}

	char_set_session_flag(account_id, 1);

	// Delete bound items from player's inventory
	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "DELETE FROM `%s` WHERE `bound` = %d",schema_config.inventory_db, BOUND_GUILD);
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
static void mapif_storage_data_loaded(int fd, uint32 account_id, char type, struct s_storage entries, bool result) {
	uint16 size = sizeof(struct s_storage) + 10;
	
	WFIFOHEAD(fd, size);
	WFIFOW(fd, 0) = 0x388a;
	WFIFOW(fd, 2) = size;
	WFIFOB(fd, 4) = type;
	WFIFOL(fd, 5) = account_id;
	WFIFOB(fd, 9) = result;
	memcpy(WFIFOP(fd, 10), &entries, sizeof(struct s_storage));
	WFIFOSET(fd, size);
}

/**
 * Tells player if inventory/cart/storage is saved
 * IZ 0x388b <account_id>.L <result>.B <type>.B
 * @param fd
 * @param account_id
 * @param char_id
 * @param type
 */
void mapif_storage_saved(int fd, uint32 account_id, uint32 char_id, bool sucess, char type) {
	WFIFOHEAD(fd,8);
	WFIFOW(fd, 0) = 0x388b;
	WFIFOL(fd, 2) = account_id;
	WFIFOB(fd, 6) = sucess;
	WFIFOB(fd, 7) = type;
	WFIFOSET(fd,8);

	if (type == TABLE_CART_) {
		struct s_storage stor;
		memset(&stor, 0, sizeof(struct s_storage));
		mapif_storage_data_loaded(fd, account_id, type, stor, cart_fromsql(char_id, &stor));
	}
}

/**
 * Requested inventory/cart/storage data for a player
 * ZI 0x308a <type>.B <account_id>.L <char_id>.L
 * @param fd
 */
bool mapif_parse_StorageLoad(int fd) {
	uint32 aid, cid;
	int type;
	struct s_storage stor;
	bool res = true;

	RFIFOHEAD(fd);
	type = RFIFOB(fd,2);
	aid = RFIFOL(fd,3);
	cid = RFIFOL(fd,7);

	memset(&stor, 0, sizeof(struct s_storage));

	//ShowInfo("Loading storage for AID=%d.\n", aid);
	switch (type) {
		case TABLE_INVENTORY: res = inventory_fromsql(cid, &stor); break;
		case TABLE_STORAGE:   res = storage_fromsql(aid, &stor);   break;
		case TABLE_CART:      res = cart_fromsql(cid, &stor);      break;
		default: return false;
	}
	mapif_storage_data_loaded(fd, aid, type, stor, res);
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
		case TABLE_INVENTORY: inventory_tosql(cid, &stor); break;
		case TABLE_STORAGE:   storage_tosql(aid, &stor);   break;
		case TABLE_CART:
		case TABLE_CART_:
			cart_tosql(cid, &stor);
			break;
		default: return false;
	}
	mapif_storage_saved(fd, aid, cid, true, type);
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
