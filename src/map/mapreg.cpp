// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mapreg.hpp"

#include <stdlib.h>

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

bool skip_insert = false;

static char mapreg_table[32] = "mapreg";
static bool mapreg_dirty = false; // Whether there are modified regs to be saved

// Put in a namespace to avoid conflicts with others variables
namespace mapreg {
	struct reg_db regs;
}

static auto& regs = mapreg::regs;

#define MAPREG_AUTOSAVE_INTERVAL (300*1000)


/**
 * Looks up the value of an integer variable using its uid.
 *
 * @param uid: variable's unique identifier.
 * @return: variable's integer value
 */
int64 mapreg_readreg(int64 uid)
{
	return regs.vars->getnum(uid);
}

/**
 * Looks up the value of a string variable using its uid.
 *
 * @param uid: variable's unique identifier
 * @return: variable's string value
 */
std::string mapreg_readregstr(int64 uid)
{
	return regs.vars->getstr(uid);
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
	int num = script_getvarid(uid);
	uint32 i = script_getvaridx(uid);
	const char* name = get_str(num);

	if (val != 0) {
		if (regs.vars->exists(uid)) {
			auto& m = regs.vars->get(uid);
			m = val;

			if (name[1] != '@') {
				mapreg_dirty = true;
				m.save = true;
			}
		} else {
			reg_db::entry m = val;
			m.uid = uid;

			if (name[1] != '@' && !skip_insert) {// write new variable to database
				char esc_name[32 * 2 + 1];
				Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
				if (SQL_ERROR == Sql_Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%" PRIu32 "','%" PRId64 "')", mapreg_table, esc_name, i, val))
					Sql_ShowDebug(mmysql_handle);
			}

			regs.vars->move(uid, m);
		}
	} else { // val == 0
		regs.vars->erase(uid);

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
bool mapreg_setregstr(int64 uid, std::string str)
{
	int num = script_getvarid(uid);
	uint32 i = script_getvaridx(uid);
	const char* name = get_str(num);

	if (str.empty()) {
		if (name[1] != '@') {
			char esc_name[32 * 2 + 1];
			Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
			if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `varname`='%s' AND `index`='%" PRIu32 "'", mapreg_table, esc_name, i))
				Sql_ShowDebug(mmysql_handle);
		}
		regs.vars->erase(uid);
	} else {
		if (regs.vars->exists(uid)) {
			auto& m = regs.vars->get(uid);
			m = str;

			if (name[1] != '@') {
				mapreg_dirty = true;
				m.save = true;
			}
		} else {
			reg_db::entry m = str;

			m.uid = uid;

			if (name[1] != '@' && !skip_insert) { //put returned null, so we must insert.
				char esc_name[32 * 2 + 1];
				char esc_str[255 * 2 + 1];
				Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
				Sql_EscapeStringLen(mmysql_handle, esc_str, str.c_str(), min(str.size(), 255));
				if (SQL_ERROR == Sql_Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%" PRIu32 "','%s')", mapreg_table, esc_name, i, esc_str))
					Sql_ShowDebug(mmysql_handle);
			}

			regs.vars->move(uid, m);
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

	SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &varname[0], sizeof(varname), &length, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_UINT32, &index, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &value[0], sizeof(value), NULL, NULL);

	while ( SQL_SUCCESS == SqlStmt_NextRow(stmt) ) {
		int s = add_str(varname);
		int64 uid = reference_uid(s, index);

		if( regs.vars->exists(uid) ) {
			ShowWarning("load_mapreg: duplicate! '%s' => '%s' skipping...\n",varname,value);
			continue;
		}
		if( varname[length-1] == '$' ) {
			mapreg_setregstr(uid, value);
		} else {
			mapreg_setreg(uid, strtoll(value,NULL,10));
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
		for (auto& [uid, reg] : *regs.vars) {
			auto num = reg_db::GetID(uid);
			auto index = reg_db::GetIndex(uid);

			if (reg.save) {
				const char* name = get_str(num);
				if (!reg.is_string) {
					char esc_name[32 * 2 + 1];
					Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
					if (SQL_ERROR == Sql_Query(mmysql_handle, "UPDATE `%s` SET `value`='%" PRId64 "' WHERE `varname`='%s' AND `index`='%" PRIu32 "' LIMIT 1", mapreg_table, reg.i64_value, esc_name, index))
						Sql_ShowDebug(mmysql_handle);
				} else {
					char esc_str[2 * 255 + 1];
					char esc_name[32 * 2 + 1];
					Sql_EscapeStringLen(mmysql_handle, esc_name, name, strnlen(name, 32));
					Sql_EscapeStringLen(mmysql_handle, esc_str, reg.str_value.c_str(), min(reg.str_value.size(), 255));
					if (SQL_ERROR == Sql_Query(mmysql_handle, "UPDATE `%s` SET `value`='%s' WHERE `varname`='%s' AND `index`='%" PRIu32 "' LIMIT 1", mapreg_table, esc_str, esc_name, index))
						Sql_ShowDebug(mmysql_handle);
				}
				reg.save = false;
			}
		}
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
 * Reloads mapregs, saving to database beforehand.
 *
 * This has the effect of clearing the temporary variables, and
 * reloading the permanent ones.
 */
void mapreg_reload(void)
{
	script_save_mapreg();

	script_free_vars(regs.vars);
	regs.vars = reg_db::create();

	script_load_mapreg();
}

/**
 * Finalizer.
 */
void mapreg_final(void)
{
	script_save_mapreg();

	script_free_vars(regs.vars);
}

/**
 * Initializer.
 */
void mapreg_init(void)
{
	regs.vars = reg_db::create();

	skip_insert = false;

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
