// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mapreg.hpp"

#include <cstdlib>

#include <common/cbasetypes.hpp>
#include <common/db.hpp>
#include <common/ers.hpp>
#include <common/malloc.hpp>
#include <common/showmsg.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "map.hpp" // mmysql_handle
#include "script.hpp"

static struct eri *mapreg_ers;

bool skip_insert = false;

static char mapreg_table[32] = "mapreg";
static bool mapreg_dirty = false; // Whether there are modified regs to be saved
struct reg_db regs;

#define MAPREG_AUTOSAVE_INTERVAL (300*1000)


/**
 * Looks up the value of an integer variable using its uid.
 *
 * @param uid: variable's unique identifier.
 * @return: variable's integer value
 */
int64 mapreg_readreg(int64 uid)
{
	struct mapreg_save *m = (struct mapreg_save *)i64db_get(regs.vars, uid);
	return m ? m->u.i : 0;
}

/**
 * Looks up the value of a string variable using its uid.
 *
 * @param uid: variable's unique identifier
 * @return: variable's string value
 */
char* mapreg_readregstr(int64 uid)
{
	struct mapreg_save *m = (struct mapreg_save *)i64db_get(regs.vars, uid);
	return m ? m->u.str : nullptr;
}

/**
 * Modifies the value of an integer variable.
 *
 * @param uid: variable's unique identifier
 * @param val new value
 * @return: true value was successfully set
 */
bool mapreg_setreg(int64 uid, int64 val)
{
	struct mapreg_save *m;
	int num = script_getvarid(uid);
	uint32 i = script_getvaridx(uid);
	const char* name = get_str(num);

	if (val != 0) {
		if ((m = static_cast<mapreg_save *>(i64db_get(regs.vars, uid)))) {
			m->u.i = val;
			if (name[1] != '@') {
				m->save = true;
				mapreg_dirty = true;
			}
		} else {
			if (i)
				script_array_update(&regs, uid, false);

			m = ers_alloc(mapreg_ers, struct mapreg_save);

			m->u.i = val;
			m->uid = uid;
			m->save = false;
			m->is_string = false;

			if (name[1] != '@' && !skip_insert) {// write new variable to database
				char esc_name[32 * 2 + 1];
				Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
				if (SQL_ERROR == Sql_Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%" PRIu32 "','%" PRId64 "')", mapreg_table, esc_name, i, val))
					Sql_ShowDebug(mmysql_handle);
			}
			i64db_put(regs.vars, uid, m);
		}
	} else { // val == 0
		if (i)
			script_array_update(&regs, uid, true);
		if ((m = static_cast<mapreg_save *>(i64db_get(regs.vars, uid)))) {
			ers_free(mapreg_ers, m);
		}
		i64db_remove(regs.vars, uid);

		if (name[1] != '@') {// Remove from database because it is unused.
			char esc_name[32 * 2 + 1];
			Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
			if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `varname`='%s' AND `index`='%" PRIu32 "'", mapreg_table, esc_name, i))
				Sql_ShowDebug(mmysql_handle);
		}
	}

	return true;
}

/**
 * Modifies the value of a string variable.
 *
 * @param uid: variable's unique identifier
 * @param str: new value
 * @return: true value was successfully set
 */
bool mapreg_setregstr(int64 uid, const char* str)
{
	struct mapreg_save *m;
	int num = script_getvarid(uid);
	uint32 i = script_getvaridx(uid);
	const char* name = get_str(num);

	if (str == nullptr || *str == 0) {
		if (i)
			script_array_update(&regs, uid, true);
		if (name[1] != '@') {
			char esc_name[32 * 2 + 1];
			Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
			if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `varname`='%s' AND `index`='%" PRIu32 "'", mapreg_table, esc_name, i))
				Sql_ShowDebug(mmysql_handle);
		}
		if ((m = static_cast<mapreg_save *>(i64db_get(regs.vars, uid)))) {
			if (m->u.str != nullptr)
				aFree(m->u.str);
			ers_free(mapreg_ers, m);
		}
		i64db_remove(regs.vars, uid);
	} else {
		if ((m = static_cast<mapreg_save *>(i64db_get(regs.vars, uid)))) {
			if (m->u.str != nullptr)
				aFree(m->u.str);
			m->u.str = aStrdup(str);
			if (name[1] != '@') {
				mapreg_dirty = true;
				m->save = true;
			}
		} else {
			if (i)
				script_array_update(&regs, uid, false);

			m = ers_alloc(mapreg_ers, struct mapreg_save);

			m->uid = uid;
			m->u.str = aStrdup(str);
			m->save = false;
			m->is_string = true;

			if (name[1] != '@' && !skip_insert) { //put returned null, so we must insert.
				char esc_name[32 * 2 + 1];
				char esc_str[255 * 2 + 1];
				Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
				Sql_EscapeStringLen(mmysql_handle, esc_str, str, strnlen(str, 255));
				if (SQL_ERROR == Sql_Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%" PRIu32 "','%s')", mapreg_table, esc_name, i, esc_str))
					Sql_ShowDebug(mmysql_handle);
			}
			i64db_put(regs.vars, uid, m);
		}
	}

	return true;
}

/**
 * Loads permanent variables from database.
 */
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
	uint32 index;
	char value[255+1];
	uint32 length;

	if ( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `varname`, `index`, `value` FROM `%s`", mapreg_table)
	  || SQL_ERROR == SqlStmt_Execute(stmt)
	  ) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return;
	}

	skip_insert = true;

	SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &varname[0], sizeof(varname), &length, nullptr);
	SqlStmt_BindColumn(stmt, 1, SQLDT_UINT32, &index, 0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &value[0], sizeof(value), nullptr, nullptr);

	while ( SQL_SUCCESS == SqlStmt_NextRow(stmt) ) {
		int s = add_str(varname);
		int64 uid = reference_uid(s, index);

		if( i64db_exists(regs.vars, uid) ) {
			ShowWarning("load_mapreg: duplicate! '%s' => '%s' skipping...\n",varname,value);
			continue;
		}
		if( varname[length-1] == '$' ) {
			mapreg_setregstr(uid, value);
		} else {
			mapreg_setreg(uid, strtoll(value,nullptr,10));
		}
	}

	SqlStmt_Free(stmt);

	skip_insert = false;
	mapreg_dirty = false;
}

/**
 * Saves permanent variables to database.
 */
static void script_save_mapreg(void)
{
	if (mapreg_dirty) {
		DBIterator *iter = db_iterator(regs.vars);
		struct mapreg_save *m;
		for (m = static_cast<mapreg_save *>(dbi_first(iter)); dbi_exists(iter); m = static_cast<mapreg_save *>(dbi_next(iter))) {
			if (m->save) {
				int num = script_getvarid(m->uid);
				uint32 i = script_getvaridx(m->uid);
				const char* name = get_str(num);
				if (!m->is_string) {
					char esc_name[32 * 2 + 1];
					Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
					if (SQL_ERROR == Sql_Query(mmysql_handle, "UPDATE `%s` SET `value`='%" PRId64 "' WHERE `varname`='%s' AND `index`='%" PRIu32 "' LIMIT 1", mapreg_table, m->u.i, esc_name, i))
						Sql_ShowDebug(mmysql_handle);
				} else {
					char esc_str[2 * 255 + 1];
					char esc_name[32 * 2 + 1];
					Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
					Sql_EscapeStringLen(mmysql_handle, esc_str, m->u.str, safestrnlen(m->u.str, 255));
					if (SQL_ERROR == Sql_Query(mmysql_handle, "UPDATE `%s` SET `value`='%s' WHERE `varname`='%s' AND `index`='%" PRIu32 "' LIMIT 1", mapreg_table, esc_str, esc_name, i))
						Sql_ShowDebug(mmysql_handle);
				}
				m->save = false;
			}
		}
		dbi_destroy(iter);
		mapreg_dirty = false;
	}
}

/**
 * Timer event to auto-save permanent variables.
 */
static TIMER_FUNC(script_autosave_mapreg){
	script_save_mapreg();
	return 0;
}

/**
 * Destroys a mapreg_save structure, freeing the contained string, if any.
 *
 * @see DBApply
 */
int mapreg_destroyreg(DBKey key, DBData *data, va_list ap)
{
	struct mapreg_save *m = nullptr;

	if (data->type != DB_DATA_PTR) // Sanity check
		return 0;

	m = static_cast<mapreg_save *>(db_data2ptr(data));

	if (m->is_string) {
		if (m->u.str)
			aFree(m->u.str);
	}
	ers_free(mapreg_ers, m);

	return 0;
}

/**
 * Reloads mapregs, saving to database beforehand.
 *
 * This has the effect of clearing the temporary variables, and
 * reloading the permanent ones.
 */
void mapreg_reload(void)
{
	script_save_mapreg();

	regs.vars->clear(regs.vars, mapreg_destroyreg);

	if (regs.arrays) {
		regs.arrays->destroy(regs.arrays, script_free_array_db);
		regs.arrays = nullptr;
	}

	script_load_mapreg();
}

/**
 * Finalizer.
 */
void mapreg_final(void)
{
	script_save_mapreg();

	regs.vars->destroy(regs.vars, mapreg_destroyreg);

	ers_destroy(mapreg_ers);

	if (regs.arrays)
		regs.arrays->destroy(regs.arrays, script_free_array_db);
}

/**
 * Initializer.
 */
void mapreg_init(void)
{
	regs.vars = i64db_alloc(DB_OPT_BASE);
	mapreg_ers = ers_new(sizeof(struct mapreg_save), "mapreg.cpp:mapreg_ers", ERS_OPT_CLEAN);

	skip_insert = false;
	regs.arrays = nullptr;

	script_load_mapreg();

	add_timer_func_list(script_autosave_mapreg, "script_autosave_mapreg");
	add_timer_interval(gettick() + MAPREG_AUTOSAVE_INTERVAL, script_autosave_mapreg, 0, 0, MAPREG_AUTOSAVE_INTERVAL);
}

/**
 * Loads the mapreg configuration file.
 */
bool mapreg_config_read(const char* w1, const char* w2)
{
	if(!strcmpi(w1, "mapreg_table"))
		safestrncpy(mapreg_table, w2, sizeof(mapreg_table));
	else
		return false;

	return true;
}
