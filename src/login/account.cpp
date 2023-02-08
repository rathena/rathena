// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "account.hpp"

#include <algorithm> //min / max
#include <memory>
#include <stdlib.h>
#include <string.h>

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"

#include "login.hpp" // login_config


/**
 * Establish the database connection.
 */
bool AccountDB::init() {
	accounts_ = Sql_Malloc();
	auto * sql_handle = get_handle();

	if (SQL_ERROR == Sql_Connect(sql_handle, db_username_.c_str(), db_password_.c_str(), db_hostname_.c_str(), db_port_, db_database_.c_str()))
	{
		ShowError("Couldn't connect with uname='%s',host='%s',port='%d',database='%s'\n",
				db_username_.c_str(), db_hostname_.c_str(), db_port_, db_database_.c_str());
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		accounts_ = nullptr;
		return false;
	}

	if (!codepage_.empty() && SQL_ERROR == Sql_SetEncoding(sql_handle, codepage_.c_str()))
		Sql_ShowDebug(sql_handle);

	remove_webtokens();

	return true;
}

/**
 * Destroy the database and close the connection to it.
 */
AccountDB::~AccountDB() {
	auto * handle = get_handle();
	remove_webtokens();
	Sql_Free(handle);
	accounts_ = nullptr;
}

/**
 * Get configuration information into buf.
 *  If the option is supported, adjust the internal state.
 * @param key: config keyword
 * @param buf: value set of the keyword
 * @param buflen: size of buffer to avoid out of bound
 * @return true if successful, false if something has failed
 */
bool AccountDB::get_property(const char* key, char* buf, size_t buflen)
{
	const char* signature = "login_server_";

	if( strncmpi(key, signature, strlen(signature)) == 0 ) {
		key += strlen(signature);
		if( strcmpi(key, "ip") == 0 )
			safesnprintf(buf, buflen, "%s", db_hostname_);
		else
		if( strcmpi(key, "port") == 0 )
			safesnprintf(buf, buflen, "%d", db_port_);
		else
		if( strcmpi(key, "id") == 0 )
			safesnprintf(buf, buflen, "%s", db_username_);
		else
		if(	strcmpi(key, "pw") == 0 )
			safesnprintf(buf, buflen, "%s", db_password_);
		else
		if( strcmpi(key, "db") == 0 )
			safesnprintf(buf, buflen, "%s", db_database_);
		else
		if( strcmpi(key, "account_db") == 0 )
			safesnprintf(buf, buflen, "%s", account_db_.c_str());
		else
		if( strcmpi(key, "global_acc_reg_str_table") == 0 )
			safesnprintf(buf, buflen, "%s", global_acc_reg_str_table_.c_str());
		else
		if( strcmpi(key, "global_acc_reg_num_table") == 0 )
			safesnprintf(buf, buflen, "%s", global_acc_reg_num_table_.c_str());
		else
			return false;// not found
		return true;
	}

	signature = "login_";
	if( strncmpi(key, signature, strlen(signature)) == 0 ) {
		key += strlen(signature);
		if( strcmpi(key, "codepage") == 0 )
			safesnprintf(buf, buflen, "%s", codepage_);
		else
		if( strcmpi(key, "case_sensitive") == 0 )
			safesnprintf(buf, buflen, "%d", (case_sensitive_ ? 1 : 0));
		else
			return false;// not found
		return true;
	}

	return false;// not found
}

/**
 * Read and set configuration.
 *  If the option is supported, adjust the internal state.
 * @param key: config keyword
 * @param value: config value for keyword
 * @return true if successful, false if something has failed
 */
bool AccountDB::set_property(const char* key, const char* value) {
	const char* signature;

	signature = "login_server_";
	if (strncmp(key, signature, strlen(signature)) == 0) {
		key += strlen(signature);
		if (strcmpi(key, "ip") == 0)
			db_hostname_ = value;
		else
		if (strcmpi(key, "port") == 0)
			db_port_ = (uint16)strtoul(value, NULL, 10);
		else
		if (strcmpi(key, "id") == 0)
			db_username_ = value;
		else
		if (strcmpi(key, "pw") == 0)
			db_password_ = value;
		else
		if (strcmpi(key, "db") == 0)
			db_database_ = value;
		else
		if (strcmpi(key, "account_db") == 0)
			account_db_ = value;
		else
		if (strcmpi(key, "global_acc_reg_str_table") == 0)
			global_acc_reg_str_table_ = value;
		else
		if (strcmpi(key, "global_acc_reg_num_table") == 0)
			global_acc_reg_num_table_ = value;
		else
			return false;// not found
		return true;
	}

	signature = "login_";
	if (strncmpi(key, signature, strlen(signature)) == 0) {
		key += strlen(signature);
		if (strcmpi(key, "codepage") == 0)
			codepage_ = value;
		else
		if (strcmpi(key, "case_sensitive") == 0)
			case_sensitive_ = (config_switch(value) == 1);
		else
			return false;// not found
		return true;
	}

	return false;// not found
}

/**
 * Create a new account entry.
 *  If acc->account_id is -1, the account id will be auto-generated,
 *  and its value will be written to acc->account_id if everything succeeds.
 * @param acc: pointer of mmo_account to save
 * @return true if successful, false if something has failed
 */
bool AccountDB::create(struct mmo_account& acc) {
	// decide on the account id to assign
	uint32 account_id;
	if (acc.account_id != -1) {
		// caller specifies it manually
		account_id = acc.account_id;
	} else {// ask the database
		char* data;
		size_t len;

		auto * sql_handle = get_handle();

		if (SQL_SUCCESS != Sql_Query(sql_handle, "SELECT MAX(`account_id`)+1 FROM `%s`", account_db_.c_str()))
		{
			Sql_ShowDebug(sql_handle);
			return false;
		}
		if (SQL_SUCCESS != Sql_NextRow(sql_handle))
		{
			Sql_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
			return false;
		}

		Sql_GetData(sql_handle, 0, &data, &len);
		account_id = (data != NULL) ? atoi(data) : 0;
		Sql_FreeResult(sql_handle);
		account_id = std::max<uint32>(START_ACCOUNT_NUM, account_id);
	}

	// zero value is prohibited
	if( account_id == 0 )
		return false;

	// absolute maximum
	if( account_id > END_ACCOUNT_NUM )
		return false;

	// insert the data into the database
	acc.account_id = account_id;
	return to_sql(acc, true, false);
}

/**
 * Delete an existing account entry and its regs.
 * @param account_id: id of user account
 * @return true if successful, false if something has failed
 */
bool AccountDB::remove(const uint32 account_id) {
	auto * sql_handle = get_handle();
	bool result = false;

	if (SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION")
		|| SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", account_db_.c_str(), account_id)
		|| SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", global_acc_reg_num_table_.c_str(), account_id)
		|| SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", global_acc_reg_str_table_.c_str(), account_id))
		Sql_ShowDebug(sql_handle);
	else
		result = true;

	result &= (SQL_SUCCESS == Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK"));

	return result;
}

/**
 * Update an existing account with the new data provided (both account and regs).
 * @param acc: pointer of mmo_account to save
 * @return true if successful, false if something has failed
 */
bool AccountDB::save(const struct mmo_account& acc, bool refresh_token) {
	return to_sql(acc, false, refresh_token);
}

/**
 * Retrieve data from db and store it in the provided data structure.
 *  Filled data structure is done by delegation to mmo_auth_fromsql.
 * @param acc: pointer of mmo_account to fill
 * @param account_id: id of user account
 * @return true if successful, false if something has failed
 */
bool AccountDB::load_num(struct mmo_account& acc, const uint32 account_id) {
	return from_sql(acc, account_id);
}

/**
 * Retrieve data from db and store it in the provided data structure.
 *  Doesn't actually retrieve data yet: escapes and checks userid, then transforms it to accid for fetching.
 *  Filled data structure is done by delegation to load_num.
 * @param acc: pointer of mmo_account to fill
 * @param userid: name of user account
 * @return true if successful, false if something has failed
 */
bool AccountDB::load_str(struct mmo_account& acc, const char* userid) {
	auto * sql_handle = get_handle();
	char esc_userid[2*NAME_LENGTH+1];

	Sql_EscapeString(sql_handle, esc_userid, userid);

	// get the list of account IDs for this user ID
	if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id` FROM `%s` WHERE `userid`= %s '%s'",
		account_db_.c_str(), (case_sensitive_ ? "BINARY" : ""), esc_userid)) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if (Sql_NumRows(sql_handle) > 1) {
		// serious problem - duplicit account
		ShowError("account_db_sql_load_str: multiple accounts found when retrieving data for account '%s'!\n", userid);
		Sql_FreeResult(sql_handle);
		return false;
	}

	if (SQL_SUCCESS != Sql_NextRow(sql_handle)) {
		// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	char* data;
	Sql_GetData(sql_handle, 0, &data, NULL);
	uint32 account_id = atoi(data);

	return load_num(acc, account_id);
}

/**
 * Fetch a struct mmo_account from sql, excluding web_auth_token.
 * @param db: pointer to db
 * @param acc: pointer of mmo_account to fill
 * @param account_id: id of user account to take data from
 * @return true if successful, false if something has failed
 */
bool AccountDB::from_sql(struct mmo_account& acc, uint32 account_id) {
	auto * sql_handle = get_handle();
	char* data;

	// retrieve login entry for the specified account
	if (SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `account_id`,`userid`,`user_pass`,`sex`,`email`,`group_id`,`state`,`unban_time`,`expiration_time`,`logincount`,`lastlogin`,`last_ip`,`birthdate`,`character_slots`,`pincode`, `pincode_change`, `vip_time`, `old_group` FROM `%s` WHERE `account_id` = %d",
		account_db_.c_str(), account_id)) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if (SQL_SUCCESS != Sql_NextRow(sql_handle)) {
		// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); acc.account_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); safestrncpy(acc.userid, data, sizeof(acc.userid));
	Sql_GetData(sql_handle,  2, &data, NULL); safestrncpy(acc.pass, data, sizeof(acc.pass));
	Sql_GetData(sql_handle,  3, &data, NULL); acc.sex = data[0];
	Sql_GetData(sql_handle,  4, &data, NULL); safestrncpy(acc.email, data, sizeof(acc.email));
	Sql_GetData(sql_handle,  5, &data, NULL); acc.group_id = (unsigned int) atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); acc.state = (unsigned int) strtoul(data, NULL, 10);
	Sql_GetData(sql_handle,  7, &data, NULL); acc.unban_time = atol(data);
	Sql_GetData(sql_handle,  8, &data, NULL); acc.expiration_time = atol(data);
	Sql_GetData(sql_handle,  9, &data, NULL); acc.logincount = (unsigned int) strtoul(data, NULL, 10);
	Sql_GetData(sql_handle, 10, &data, NULL); safestrncpy(acc.lastlogin, data==NULL?"":data, sizeof(acc.lastlogin));
	Sql_GetData(sql_handle, 11, &data, NULL); safestrncpy(acc.last_ip, data, sizeof(acc.last_ip));
	Sql_GetData(sql_handle, 12, &data, NULL); safestrncpy(acc.birthdate, data==NULL?"":data, sizeof(acc.birthdate));
	Sql_GetData(sql_handle, 13, &data, NULL); acc.char_slots = (uint8) atoi(data);
	Sql_GetData(sql_handle, 14, &data, NULL); safestrncpy(acc.pincode, data, sizeof(acc.pincode));
	Sql_GetData(sql_handle, 15, &data, NULL); acc.pincode_change = atol(data);
#ifdef VIP_ENABLE
	Sql_GetData(sql_handle, 16, &data, NULL); acc.vip_time = atol(data);
	Sql_GetData(sql_handle, 17, &data, NULL); acc.old_group = atoi(data);
#endif
	Sql_FreeResult(sql_handle);
	acc.web_auth_token[0] = '\0';

	if (acc.char_slots > MAX_CHARS) {
		ShowError( "Account %s (AID=%u) exceeds MAX_CHARS. Capping...\n", acc.userid, acc.account_id );
		acc.char_slots = MAX_CHARS;
	}

	return true;
}

/**
 * Save a struct mmo_account in sql.
 * @param db: pointer to db
 * @param acc: pointer of mmo_account to save
 * @param is_new: if it's a new entry or should we update
 * @return true if successful, false if something has failed
 */
bool AccountDB::to_sql(const struct mmo_account& acc, bool is_new, bool refresh_token) {
	auto * sql_handle = get_handle();
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
	bool result = false;

	// try
	do {

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	if( is_new )
	{// insert into account table
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt,
			"INSERT INTO `%s` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `group_id`, `state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, `last_ip`, `birthdate`, `character_slots`, `pincode`, `pincode_change`, `vip_time`, `old_group` ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
			account_db_.c_str())
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  0, SQLDT_INT,       (void*)&acc.account_id,      sizeof(acc.account_id))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  1, SQLDT_STRING,    (void*)acc.userid,           strlen(acc.userid))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  2, SQLDT_STRING,    (void*)acc.pass,             strlen(acc.pass))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  3, SQLDT_ENUM,      (void*)&acc.sex,             sizeof(acc.sex))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  4, SQLDT_STRING,    (void*)&acc.email,           strlen(acc.email))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  5, SQLDT_INT,       (void*)&acc.group_id,        sizeof(acc.group_id))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  6, SQLDT_UINT,      (void*)&acc.state,           sizeof(acc.state))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  7, SQLDT_LONG,      (void*)&acc.unban_time,      sizeof(acc.unban_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  8, SQLDT_INT,       (void*)&acc.expiration_time, sizeof(acc.expiration_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  9, SQLDT_UINT,      (void*)&acc.logincount,      sizeof(acc.logincount))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, acc.lastlogin[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc.lastlogin,       strlen(acc.lastlogin))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, SQLDT_STRING,    (void*)&acc.last_ip,         strlen(acc.last_ip))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 12, acc.birthdate[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc.birthdate,       strlen(acc.birthdate))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 13, SQLDT_UCHAR,     (void*)&acc.char_slots,      sizeof(acc.char_slots))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 14, SQLDT_STRING,    (void*)&acc.pincode,         strlen(acc.pincode))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 15, SQLDT_LONG,      (void*)&acc.pincode_change,  sizeof(acc.pincode_change))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 16, SQLDT_LONG,       (void*)&acc.vip_time,         sizeof(acc.vip_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 17, SQLDT_INT,        (void*)&acc.old_group,        sizeof(acc.old_group))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
			SqlStmt_ShowDebug(stmt);
			break;
		}
	}
	else
	{// update account table
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, 
#ifdef VIP_ENABLE
			"UPDATE `%s` SET `userid`=?,`user_pass`=?,`sex`=?,`email`=?,`group_id`=?,`state`=?,`unban_time`=?,`expiration_time`=?,`logincount`=?,`lastlogin`=?,`last_ip`=?,`birthdate`=?,`character_slots`=?,`pincode`=?, `pincode_change`=?, `vip_time`=?, `old_group`=? WHERE `account_id` = '%d'",
#else
			"UPDATE `%s` SET `userid`=?,`user_pass`=?,`sex`=?,`email`=?,`group_id`=?,`state`=?,`unban_time`=?,`expiration_time`=?,`logincount`=?,`lastlogin`=?,`last_ip`=?,`birthdate`=?,`character_slots`=?,`pincode`=?, `pincode_change`=? WHERE `account_id` = '%d'",
#endif
			account_db_.c_str(), acc.account_id)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  0, SQLDT_STRING,    (void*)acc.userid,           strlen(acc.userid))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  1, SQLDT_STRING,    (void*)acc.pass,             strlen(acc.pass))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  2, SQLDT_ENUM,      (void*)&acc.sex,             sizeof(acc.sex))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  3, SQLDT_STRING,    (void*)acc.email,            strlen(acc.email))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  4, SQLDT_INT,       (void*)&acc.group_id,        sizeof(acc.group_id))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  5, SQLDT_UINT,      (void*)&acc.state,           sizeof(acc.state))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  6, SQLDT_LONG,      (void*)&acc.unban_time,      sizeof(acc.unban_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  7, SQLDT_LONG,      (void*)&acc.expiration_time, sizeof(acc.expiration_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  8, SQLDT_UINT,      (void*)&acc.logincount,      sizeof(acc.logincount))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  9, acc.lastlogin[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc.lastlogin,       strlen(acc.lastlogin))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, SQLDT_STRING,    (void*)&acc.last_ip,         strlen(acc.last_ip))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, acc.birthdate[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc.birthdate,       strlen(acc.birthdate))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 12, SQLDT_UCHAR,     (void*)&acc.char_slots,      sizeof(acc.char_slots))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 13, SQLDT_STRING,    (void*)&acc.pincode,         strlen(acc.pincode))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 14, SQLDT_LONG,      (void*)&acc.pincode_change,  sizeof(acc.pincode_change))
#ifdef VIP_ENABLE
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 15, SQLDT_LONG,      (void*)&acc.vip_time,        sizeof(acc.vip_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 16, SQLDT_INT,       (void*)&acc.old_group,       sizeof(acc.old_group))
#endif
		||  SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
			SqlStmt_ShowDebug(stmt);
			break;
		}
	}

	if( acc.sex != 'S' && login_config.use_web_auth_token && refresh_token ){
		static bool initialized = false;
		static const char* query;

		// Pseudo Scope to break out
		while( !initialized ){
			if( SQL_SUCCESS == Sql_Query( sql_handle, "SELECT SHA2( 'test', 256 )" ) ){
				query = "UPDATE `%s` SET `web_auth_token` = LEFT( SHA2( CONCAT( UUID(), RAND() ), 256 ), %d ), `web_auth_token_enabled` = '1' WHERE `account_id` = '%d'";
				initialized = true;
				break;
			}

			if( SQL_SUCCESS == Sql_Query( sql_handle, "SELECT MD5( 'test' )" ) ){
				query = "UPDATE `%s` SET `web_auth_token` = LEFT( MD5( CONCAT( UUID(), RAND() ) ), %d ), `web_auth_token_enabled` = '1' WHERE `account_id` = '%d'";
				initialized = true;
				break;
			}

			ShowWarning( "Your MySQL does not support SHA2 and MD5 - no hashing will be used for login token creation.\n" );
			ShowWarning( "If you are using an old version of MySQL consider upgrading to a newer release.\n" );
			query = "UPDATE `%s` SET `web_auth_token` = LEFT( CONCAT( UUID(), RAND() ), %d ), `web_auth_token_enabled` = '1' WHERE `account_id` = '%d'";
			initialized = true;
			break;
		}

		const int MAX_RETRIES = 20;
		int i = 0;
		bool success = false;

		// Retry it for a maximum number of retries
		do{
			if( SQL_SUCCESS == Sql_Query( sql_handle, query, account_db_.c_str(), WEB_AUTH_TOKEN_LENGTH - 1, acc.account_id ) ){
				success = true;
				break;
			}
		}while( i < MAX_RETRIES && Sql_GetError( sql_handle ) == 1062 );

		if( !success ){
			if( i == MAX_RETRIES ){
				ShowError( "Failed to generate a unique web_auth_token with %d retries...\n", i );
			}else{
				Sql_ShowDebug( sql_handle );
			}

			break;
		}

		char* data;
		size_t len;

		if( SQL_SUCCESS != Sql_Query( sql_handle, "SELECT `web_auth_token` from `%s` WHERE `account_id` = '%d'", account_db_.c_str(), acc.account_id ) ||
			SQL_SUCCESS != Sql_NextRow( sql_handle ) ||
			SQL_SUCCESS != Sql_GetData( sql_handle, 0, &data, &len )
			){
			Sql_ShowDebug( sql_handle );
			break;
		}

		safestrncpy( (char *)&acc.web_auth_token, data, sizeof( acc.web_auth_token ) );

		Sql_FreeResult( sql_handle );
	}

	// if we got this far, everything was successful
	result = true;

	} while(0);
	// finally

	result &= ( SQL_SUCCESS == Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") );
	SqlStmt_Free(stmt);

	return result;
}

void AccountDB::save_global_accreg(int fd, uint32 account_id, uint32 char_id) {
	auto * sql_handle = get_handle();
	uint16 count = RFIFOW(fd, 12);

	if (count) {
		int cursor = 14, i;
		char key[32], sval[254], esc_key[32*2+1], esc_sval[254*2+1];

		for (i = 0; i < count; i++) {
			uint32 index;
			safestrncpy(key, RFIFOCP(fd, cursor + 1), RFIFOB(fd, cursor));
			Sql_EscapeString(sql_handle, esc_key, key);
			cursor += RFIFOB(fd, cursor) + 1;

			index = RFIFOL(fd, cursor);
			cursor += 4;

			switch (RFIFOB(fd, cursor++)) {
				// int
				case 0:
					if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%" PRIu32 "','%s','%" PRIu32 "','%" PRId64 "')", global_acc_reg_num_table_.c_str(), account_id, esc_key, index, RFIFOQ(fd, cursor)) )
						Sql_ShowDebug(sql_handle);
					cursor += 8;
					break;
				case 1:
					if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%" PRIu32 "' AND `key` = '%s' AND `index` = '%" PRIu32 "' LIMIT 1", global_acc_reg_num_table_.c_str(), account_id, esc_key, index) )
						Sql_ShowDebug(sql_handle);
					break;
				// str
				case 2:
					safestrncpy(sval, RFIFOCP(fd, cursor + 1), RFIFOB(fd, cursor));
					cursor += RFIFOB(fd, cursor) + 1;
					Sql_EscapeString(sql_handle, esc_sval, sval);
					if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%" PRIu32 "','%s','%" PRIu32 "','%s')", global_acc_reg_str_table_.c_str(), account_id, esc_key, index, esc_sval) )
						Sql_ShowDebug(sql_handle);
					break;
				case 3:
					if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%" PRIu32 "' AND `key` = '%s' AND `index` = '%" PRIu32 "' LIMIT 1", global_acc_reg_str_table_.c_str(), account_id, esc_key, index) )
						Sql_ShowDebug(sql_handle);
					break;
				default:
					ShowError("mmo_save_global_accreg: unknown type %d\n",RFIFOB(fd, cursor - 1));
					return;
			}
		}
	}
}

void AccountDB::send_global_accreg(int fd, uint32 account_id, uint32 char_id) {
	auto * sql_handle = get_handle();
	char* data;
	int plen = 0;
	size_t len;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%" PRIu32 "'", global_acc_reg_str_table_.c_str(), account_id) )
		Sql_ShowDebug(sql_handle);

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x2726;
	// 0x2 = length, set prior to being sent
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0; // var type (only set when all vars have been sent, regardless of type)
	WFIFOB(fd, 13) = 1; // is string type
	WFIFOW(fd, 14) = 0; // count
	plen = 16;

	/**
	 * Vessel!
	 *
	 * str type
	 * { keyLength(B), key(<keyLength>), index(L), valLength(B), val(<valLength>) }
	 **/
	while ( SQL_SUCCESS == Sql_NextRow(sql_handle) ) {
		Sql_GetData(sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 32
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += len;

		Sql_GetData(sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (uint32)atol(data);
		plen += 4;

		Sql_GetData(sql_handle, 2, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 254
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += len;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			// prepare follow up
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x2726;
			// 0x2 = length, set prior to being sent
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0; // var type (only set when all vars have been sent, regardless of type)
			WFIFOB(fd, 13) = 1; // is string type
			WFIFOW(fd, 14) = 0; // count
			plen = 16;
		}
	}

	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	Sql_FreeResult(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%" PRIu32 "'", global_acc_reg_num_table_.c_str(), account_id) )
		Sql_ShowDebug(sql_handle);

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x2726;
	// 0x2 = length, set prior to being sent
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0; // var type (only set when all vars have been sent, regardless of type)
	WFIFOB(fd, 13) = 0; // is int type
	WFIFOW(fd, 14) = 0; // count
	plen = 16;

	/**
	 * Vessel!
	 *
	 * int type
	 * { keyLength(B), key(<keyLength>), index(L), value(L) }
	 **/
	while ( SQL_SUCCESS == Sql_NextRow(sql_handle) ) {
		Sql_GetData(sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 32
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += len;

		Sql_GetData(sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (uint32)atol(data);
		plen += 4;

		Sql_GetData(sql_handle, 2, &data, NULL);

		WFIFOQ(fd, plen) = strtoll(data,NULL,10);
		plen += 8;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			// prepare follow up
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x2726;
			// 0x2 = length, set prior to being sent
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0; // var type (only set when all vars have been sent, regardless of type)
			WFIFOB(fd, 13) = 0; // is int type
			WFIFOW(fd, 14) = 0; // count

			plen = 16;
		}
	}

	WFIFOB(fd, 12) = 1;
	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	Sql_FreeResult(sql_handle);
}

bool AccountDB::enable_webtoken(const uint32 account_id) {
	auto * sql_handle = get_handle();
	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `web_auth_token_enabled` = '1' WHERE `account_id` = '%u'", account_db_.c_str(), account_id)) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Timered function to disable webtoken for user
 * If the user is online, then they must have logged since we started the timer.
 * In that case, do nothing. The new authtoken must be valid.
 * @param tid: timer id
 * @param tick: tick of execution
 * @param id: user account id
 * @param data: AccountDB // because we don't use singleton???
 * @return :0
 */
TIMER_FUNC(account_disable_webtoken_timer){
	const struct online_login_data* p = login_get_online_user(id);
	AccountDB* db = reinterpret_cast<AccountDB*>(data);

	return db->disable_webtoken(id);
}


bool AccountDB::disable_webtoken_timer(const uint32 account_id) {
	add_timer(gettick() + login_config.disable_webtoken_delay, account_disable_webtoken_timer, account_id, reinterpret_cast<intptr_t>(this));
	return true;
}

int AccountDB::disable_webtoken(const uint32 account_id) {
	const struct online_login_data* p = login_get_online_user(account_id);

	if (p == nullptr) {
		ShowInfo("Web Auth Token for account %d was disabled\n", account_id);
		auto * sql_handle = get_handle();
		if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `web_auth_token_enabled` = '0' WHERE `account_id` = '%u'", account_db_.c_str(), account_id)) {
			Sql_ShowDebug(sql_handle);
			return 0;
		}
	}

	return 0;
}


bool AccountDB::remove_webtokens() {
	auto * sql_handle = get_handle();
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `web_auth_token` = NULL, `web_auth_token_enabled` = '0'", account_db_.c_str())) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


Sql * AccountDB::get_handle() {
	return accounts_;
}