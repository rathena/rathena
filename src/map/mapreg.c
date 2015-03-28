// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/timer.h"

#include "map.h" // mmysql_handle
#include "script.h"

#include <stdlib.h>

static DBMap* mapreg_db = NULL; // int var_id -> int value
static DBMap* mapregstr_db = NULL; // int var_id -> char* value

static char mapreg_table[32] = "mapreg";
static bool mapreg_dirty = false;
#define MAPREG_AUTOSAVE_INTERVAL (300*1000)


/// Looks up the value of an integer variable using its uid.
int mapreg_readreg(int uid)
{
	return idb_iget(mapreg_db, uid);
}

/// Looks up the value of a string variable using its uid.
char* mapreg_readregstr(int uid)
{
	return idb_get(mapregstr_db, uid);
}

/// Modifies the value of an integer variable.
bool mapreg_setreg(int uid, int val)
{
	int num = (uid & 0x00ffffff);
	int i   = (uid & 0xff000000) >> 24;
	const char* name = get_str(num);

	if( val != 0 )
	{
		if( idb_iput(mapreg_db,uid,val) )
			mapreg_dirty = true; // already exists, delay write
		else if(name[1] != '@')
		{// write new variable to database
			char tmp_str[32*2+1];
			Sql_EscapeStringLen(mmysql_handle, tmp_str, name, strnlen(name, 32));
			if( SQL_ERROR == Sql_Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%d','%d')", mapreg_table, tmp_str, i, val) )
				Sql_ShowDebug(mmysql_handle);
		}
	}
	else // val == 0
	{
		idb_remove(mapreg_db,uid);

		if( name[1] != '@' )
		{// Remove from database because it is unused.
			if( SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `varname`='%s' AND `index`='%d'", mapreg_table, name, i) )
				Sql_ShowDebug(mmysql_handle);
		}
	}

	return true;
}

/// Modifies the value of a string variable.
bool mapreg_setregstr(int uid, const char* str)
{
	int num = (uid & 0x00ffffff);
	int i   = (uid & 0xff000000) >> 24;
	const char* name = get_str(num);

	if( str == NULL || *str == 0 )
	{
		if(name[1] != '@') {
			if( SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `varname`='%s' AND `index`='%d'", mapreg_table, name, i) )
				Sql_ShowDebug(mmysql_handle);
		}
		idb_remove(mapregstr_db,uid);
	}
	else
	{
		if (idb_put(mapregstr_db,uid, aStrdup(str)))
			mapreg_dirty = true;
		else if(name[1] != '@') { //put returned null, so we must insert.
			// Someone is causing a database size infinite increase here without name[1] != '@' [Lance]
			char tmp_str[32*2+1];
			char tmp_str2[255*2+1];
			Sql_EscapeStringLen(mmysql_handle, tmp_str, name, strnlen(name, 32));
			Sql_EscapeStringLen(mmysql_handle, tmp_str2, str, strnlen(str, 255));
			if( SQL_ERROR == Sql_Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%d','%s')", mapreg_table, tmp_str, i, tmp_str2) )
				Sql_ShowDebug(mmysql_handle);
		}
	}

	return true;
}

/// Loads permanent variables from database
static void script_load_mapreg(void)
{
	/*
	        0        1       2
	   +-------------------------+
	   | varname | index | value |
	   +-------------------------+
	                                */
	SqlStmt* stmt = SqlStmt_Malloc(mmysql_handle);
	char varname[32+1];
	int index;
	char value[255+1];
	uint32 length;

	if ( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `varname`, `index`, `value` FROM `%s`", mapreg_table)
	  || SQL_ERROR == SqlStmt_Execute(stmt)
	  ) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &varname[0], sizeof(varname), &length, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_INT, &index, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &value[0], sizeof(value), NULL, NULL);
	
	while ( SQL_SUCCESS == SqlStmt_NextRow(stmt) )
	{
		int s = add_str(varname);
		int i = index;

		if( varname[length-1] == '$' )
			idb_put(mapregstr_db, (i<<24)|s, aStrdup(value));
		else
			idb_iput(mapreg_db, (i<<24)|s, atoi(value));
	}
	SqlStmt_Free(stmt);

	mapreg_dirty = false;
}

/// Saves permanent variables to database
static void script_save_mapreg(void)
{
	DBIterator* iter;
	DBData *data;
	DBKey key;

	iter = db_iterator(mapreg_db);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int num = (key.i & 0x00ffffff);
		int i   = (key.i & 0xff000000) >> 24;
		const char* name = get_str(num);

		if( name[1] == '@' )
			continue;

		if( SQL_ERROR == Sql_Query(mmysql_handle, "UPDATE `%s` SET `value`='%d' WHERE `varname`='%s' AND `index`='%d'", mapreg_table, db_data2i(data), name, i) )
			Sql_ShowDebug(mmysql_handle);
	}
	dbi_destroy(iter);

	iter = db_iterator(mapregstr_db);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int num = (key.i & 0x00ffffff);
		int i   = (key.i & 0xff000000) >> 24;
		const char* name = get_str(num);
		char tmp_str2[2*255+1];

		if( name[1] == '@' )
			continue;

		Sql_EscapeStringLen(mmysql_handle, tmp_str2, db_data2ptr(data), safestrnlen(db_data2ptr(data), 255));
		if( SQL_ERROR == Sql_Query(mmysql_handle, "UPDATE `%s` SET `value`='%s' WHERE `varname`='%s' AND `index`='%d'", mapreg_table, tmp_str2, name, i) )
			Sql_ShowDebug(mmysql_handle);
	}
	dbi_destroy(iter);

	mapreg_dirty = false;
}

static int script_autosave_mapreg(int tid, unsigned int tick, int id, intptr_t data)
{
	if( mapreg_dirty )
		script_save_mapreg();

	return 0;
}


void mapreg_reload(void)
{
	if( mapreg_dirty )
		script_save_mapreg();

	db_clear(mapreg_db);
	db_clear(mapregstr_db);

	script_load_mapreg();
}

void mapreg_final(void)
{
	if( mapreg_dirty )
		script_save_mapreg();

	db_destroy(mapreg_db);
	db_destroy(mapregstr_db);
}

void mapreg_init(void)
{
	mapreg_db = idb_alloc(DB_OPT_BASE);
	mapregstr_db = idb_alloc(DB_OPT_RELEASE_DATA);

	script_load_mapreg();

	add_timer_func_list(script_autosave_mapreg, "script_autosave_mapreg");
	add_timer_interval(gettick() + MAPREG_AUTOSAVE_INTERVAL, script_autosave_mapreg, 0, 0, MAPREG_AUTOSAVE_INTERVAL);
}

bool mapreg_config_read(const char* w1, const char* w2)
{
	if(!strcmpi(w1, "mapreg_db"))
		safestrncpy(mapreg_table, w2, sizeof(mapreg_table));
	else
		return false;

	return true;
}
