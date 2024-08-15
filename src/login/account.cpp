// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "account.hpp"

#include <algorithm> //min / max
#include <cstdlib>
#include <cstring>

#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>

#include "login.hpp" // login_config

/// global defines

/// internal structure
typedef struct AccountDB_SQL {
	AccountDB vtable;    // public interface
	Sql* accounts;       // SQL handle accounts storage
	std::string db_hostname = "127.0.0.1";
	uint16 db_port = 3306;
	std::string db_username = "ragnarok";
	std::string db_password = "";
	std::string db_database = "ragnarok";
	std::string codepage = "";
	// other settings
	bool case_sensitive;
	//table name
	char account_db[32];
	char global_acc_reg_num_table[32];
	char global_acc_reg_str_table[32];

} AccountDB_SQL;

/// internal structure
typedef struct AccountDBIterator_SQL {
	AccountDBIterator vtable;    // public interface
	AccountDB_SQL* db;
	int last_account_id;
} AccountDBIterator_SQL;

/// internal functions
static bool account_db_sql_init(AccountDB* self);
static void account_db_sql_destroy(AccountDB* self);
static bool account_db_sql_get_property(AccountDB* self, const char* key, char* buf, size_t buflen);
static bool account_db_sql_set_property(AccountDB* self, const char* option, const char* value);
static bool account_db_sql_create(AccountDB* self, struct mmo_account* acc);
static bool account_db_sql_remove(AccountDB* self, const uint32 account_id);
static bool account_db_sql_enable_webtoken( AccountDB* self, const uint32 account_id );
static bool account_db_sql_disable_webtoken( AccountDB* self, const uint32 account_id );
static bool account_db_sql_remove_webtokens( AccountDB* self );
static bool account_db_sql_save(AccountDB* self, const struct mmo_account* acc, bool refresh_token);
static bool account_db_sql_load_num(AccountDB* self, struct mmo_account* acc, const uint32 account_id);
static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid);
static AccountDBIterator* account_db_sql_iterator(AccountDB* self);
static void account_db_sql_iter_destroy(AccountDBIterator* self);
static bool account_db_sql_iter_next(AccountDBIterator* self, struct mmo_account* acc);

static bool mmo_auth_fromsql(AccountDB_SQL* db, struct mmo_account* acc, uint32 account_id);
static bool mmo_auth_tosql(AccountDB_SQL* db, const struct mmo_account* acc, bool is_new, bool refresh_token);

/// public constructor
AccountDB* account_db_sql(void) {
	AccountDB_SQL* db = (AccountDB_SQL*)aCalloc(1, sizeof(AccountDB_SQL));
	new(db) AccountDB_SQL();

	// set up the vtable
	db->vtable.init         = &account_db_sql_init;
	db->vtable.destroy      = &account_db_sql_destroy;
	db->vtable.get_property = &account_db_sql_get_property;
	db->vtable.set_property = &account_db_sql_set_property;
	db->vtable.save         = &account_db_sql_save;
	db->vtable.create       = &account_db_sql_create;
	db->vtable.remove       = &account_db_sql_remove;
	db->vtable.enable_webtoken = &account_db_sql_enable_webtoken;
	db->vtable.disable_webtoken = &account_db_sql_disable_webtoken;
	db->vtable.remove_webtokens = &account_db_sql_remove_webtokens;
	db->vtable.load_num     = &account_db_sql_load_num;
	db->vtable.load_str     = &account_db_sql_load_str;
	db->vtable.iterator     = &account_db_sql_iterator;

	// initialize to default values
	db->accounts = nullptr;
	// other settings
	db->case_sensitive = false;
	safestrncpy(db->account_db, "login", sizeof(db->account_db));
	safestrncpy(db->global_acc_reg_num_table, "global_acc_reg_num", sizeof(db->global_acc_reg_num_table));
	safestrncpy(db->global_acc_reg_str_table, "global_acc_reg_str", sizeof(db->global_acc_reg_str_table));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


/**
 * Establish the database connection.
 * @param self: pointer to db
 */
static bool account_db_sql_init(AccountDB* self) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle;

	db->accounts = Sql_Malloc();
	sql_handle = db->accounts;

	if( SQL_ERROR == Sql_Connect(sql_handle, db->db_username.c_str(), db->db_password.c_str(), db->db_hostname.c_str(), db->db_port, db->db_database.c_str()) )
	{
		ShowError("Couldn't connect with uname='%s',host='%s',port='%hu',database='%s'\n",
			db->db_username.c_str(), db->db_hostname.c_str(), db->db_port, db->db_database.c_str());
		Sql_ShowDebug(sql_handle);
		Sql_Free(db->accounts);
		db->accounts = nullptr;
		return false;
	}

	if( !db->codepage.empty() && SQL_ERROR == Sql_SetEncoding(sql_handle, db->codepage.c_str()) )
		Sql_ShowDebug(sql_handle);

	self->remove_webtokens( self );

	return true;
}

/**
 * Destroy the database and close the connection to it.
 * @param self: pointer to db
 */
static void account_db_sql_destroy(AccountDB* self){
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	if( SQL_ERROR == Sql_Query( db->accounts, "UPDATE `%s` SET `web_auth_token` = NULL", db->account_db ) ){
		Sql_ShowDebug( db->accounts );
	}

	Sql_Free(db->accounts);
	db->accounts = nullptr;

	db->~AccountDB_SQL();
	aFree(db);
}

/**
 * Get configuration information into buf.
 *  If the option is supported, adjust the internal state.
 * @param self: pointer to db
 * @param key: config keyword
 * @param buf: value set of the keyword
 * @param buflen: size of buffer to avoid out of bound
 * @return true if successful, false if something has failed
 */
static bool account_db_sql_get_property(AccountDB* self, const char* key, char* buf, size_t buflen)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	const char* signature;

	signature = "login_server_";
	if( strncmpi(key, signature, strlen(signature)) == 0 ) {
		key += strlen(signature);
		if( strcmpi(key, "ip") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_hostname.c_str());
		else
		if( strcmpi(key, "port") == 0 )
			safesnprintf(buf, buflen, "%hu", db->db_port);
		else
		if( strcmpi(key, "id") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_username.c_str());
		else
		if(	strcmpi(key, "pw") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_password.c_str());
		else
		if( strcmpi(key, "db") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_database.c_str());
		else
		if( strcmpi(key, "account_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->account_db);
		else
		if( strcmpi(key, "global_acc_reg_str_table") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_acc_reg_str_table);
		else
		if( strcmpi(key, "global_acc_reg_num_table") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_acc_reg_num_table);
		else
			return false;// not found
		return true;
	}

	signature = "login_";
	if( strncmpi(key, signature, strlen(signature)) == 0 ) {
		key += strlen(signature);
		if( strcmpi(key, "codepage") == 0 )
			safesnprintf(buf, buflen, "%s", db->codepage.c_str());
		else
		if( strcmpi(key, "case_sensitive") == 0 )
			safesnprintf(buf, buflen, "%d", (db->case_sensitive ? 1 : 0));
		else
			return false;// not found
		return true;
	}

	return false;// not found
}

/**
 * Read and set configuration.
 *  If the option is supported, adjust the internal state.
 * @param self: pointer to db
 * @param key: config keyword
 * @param value: config value for keyword
 * @return true if successful, false if something has failed
 */
static bool account_db_sql_set_property(AccountDB* self, const char* key, const char* value) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	const char* signature;

	signature = "login_server_";
	if( strncmp(key, signature, strlen(signature)) == 0 ) {
		key += strlen(signature);
		if( strcmpi(key, "ip") == 0 )
			db->db_hostname = value;
		else
		if( strcmpi(key, "port") == 0 )
			db->db_port = (uint16)strtoul( value, nullptr, 10 );
		else
		if( strcmpi(key, "id") == 0 )
			db->db_username = value;
		else
		if( strcmpi(key, "pw") == 0 )
			db->db_password = value;
		else
		if( strcmpi(key, "db") == 0 )
			db->db_database = value;
		else
		if( strcmpi(key, "account_db") == 0 )
			safestrncpy(db->account_db, value, sizeof(db->account_db));
		else
		if( strcmpi(key, "global_acc_reg_str_table") == 0 )
			safestrncpy(db->global_acc_reg_str_table, value, sizeof(db->global_acc_reg_str_table));
		else
		if( strcmpi(key, "global_acc_reg_num_table") == 0 )
			safestrncpy(db->global_acc_reg_num_table, value, sizeof(db->global_acc_reg_num_table));
		else
			return false;// not found
		return true;
	}

	signature = "login_";
	if( strncmpi(key, signature, strlen(signature)) == 0 ) {
		key += strlen(signature);
		if( strcmpi(key, "codepage") == 0 )
			db->codepage = value;
		else
		if( strcmpi(key, "case_sensitive") == 0 )
			db->case_sensitive = (config_switch(value)==1);
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
 * @param self: pointer to db
 * @param acc: pointer of mmo_account to save
 * @return true if successful, false if something has failed
 */
static bool account_db_sql_create(AccountDB* self, struct mmo_account* acc) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle = db->accounts;

	// decide on the account id to assign
	uint32 account_id;
	if( acc->account_id != -1 )
	{// caller specifies it manually
		account_id = acc->account_id;
	}
	else
	{// ask the database
		char* data;
		size_t len;

		if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT MAX(`account_id`)+1 FROM `%s`", db->account_db) )
		{
			Sql_ShowDebug(sql_handle);
			return false;
		}
		if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
		{
			Sql_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
			return false;
		}

		Sql_GetData(sql_handle, 0, &data, &len);
		account_id = ( data != nullptr ) ? atoi(data) : 0;
		Sql_FreeResult(sql_handle);
		account_id = max((uint32_t) START_ACCOUNT_NUM, account_id);
	}

	// zero value is prohibited
	if( account_id == 0 )
		return false;

	// absolute maximum
	if( account_id > END_ACCOUNT_NUM )
		return false;

	// insert the data into the database
	acc->account_id = account_id;
	return mmo_auth_tosql(db, acc, true, false);
}

/**
 * Delete an existing account entry and its regs.
 * @param self: pointer to db
 * @param account_id: id of user account
 * @return true if successful, false if something has failed
 */
static bool account_db_sql_remove(AccountDB* self, const uint32 account_id) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle = db->accounts;
	bool result = false;

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION")
	||  SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", db->account_db, account_id)
	||  SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", db->global_acc_reg_num_table, account_id)
	||  SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", db->global_acc_reg_str_table, account_id) )
		Sql_ShowDebug(sql_handle);
	else
		result = true;

	result &= ( SQL_SUCCESS == Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") );

	return result;
}

/**
 * Update an existing account with the new data provided (both account and regs).
 * @param self: pointer to db
 * @param acc: pointer of mmo_account to save
 * @return true if successful, false if something has failed
 */
static bool account_db_sql_save(AccountDB* self, const struct mmo_account* acc, bool refresh_token) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	return mmo_auth_tosql(db, acc, false, refresh_token);
}

/**
 * Retrieve data from db and store it in the provided data structure.
 *  Filled data structure is done by delegation to mmo_auth_fromsql.
 * @param self: pointer to db
 * @param acc: pointer of mmo_account to fill
 * @param account_id: id of user account
 * @return true if successful, false if something has failed
 */
static bool account_db_sql_load_num(AccountDB* self, struct mmo_account* acc, const uint32 account_id) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	return mmo_auth_fromsql(db, acc, account_id);
}

/**
 * Retrieve data from db and store it in the provided data structure.
 *  Doesn't actually retrieve data yet: escapes and checks userid, then transforms it to accid for fetching.
 *  Filled data structure is done by delegation to account_db_sql_load_num.
 * @param self: pointer to db
 * @param acc: pointer of mmo_account to fill
 * @param userid: name of user account
 * @return true if successful, false if something has failed
 */
static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle = db->accounts;
	char esc_userid[2*NAME_LENGTH+1];
	uint32 account_id;
	char* data;

	Sql_EscapeString(sql_handle, esc_userid, userid);

	// get the list of account IDs for this user ID
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id` FROM `%s` WHERE `userid`= %s '%s'",
		db->account_db, (db->case_sensitive ? "BINARY" : ""), esc_userid) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( Sql_NumRows(sql_handle) > 1 )
	{// serious problem - duplicit account
		ShowError("account_db_sql_load_str: multiple accounts found when retrieving data for account '%s'!\n", userid);
		Sql_FreeResult(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, nullptr);
	account_id = atoi(data);

	return account_db_sql_load_num(self, acc, account_id);
}

/**
 * Create a new forward iterator.
 * @param self: pointer to db iterator
 * @return a new db iterator
 */
static AccountDBIterator* account_db_sql_iterator(AccountDB* self) {
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	AccountDBIterator_SQL* iter = (AccountDBIterator_SQL*)aCalloc(1, sizeof(AccountDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &account_db_sql_iter_destroy;
	iter->vtable.next    = &account_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->last_account_id = -1;

	return &iter->vtable;
}

/**
 * Destroys this iterator, releasing all allocated memory (including itself).
 * @param self: pointer to db iterator
 */
static void account_db_sql_iter_destroy(AccountDBIterator* self) {
	AccountDBIterator_SQL* iter = (AccountDBIterator_SQL*)self;
	aFree(iter);
}

/**
 * Fetches the next account in the database.
 * @param self: pointer to db iterator
 * @param acc: pointer of mmo_account to fill
 * @return true if next account found and filled, false if something has failed
 */
static bool account_db_sql_iter_next(AccountDBIterator* self, struct mmo_account* acc) {
	AccountDBIterator_SQL* iter = (AccountDBIterator_SQL*)self;
	AccountDB_SQL* db = iter->db;
	Sql* sql_handle = db->accounts;
	char* data;

	// get next account ID
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id` FROM `%s` WHERE `account_id` > '%d' ORDER BY `account_id` ASC LIMIT 1",
		db->account_db, iter->last_account_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) &&
		SQL_SUCCESS == Sql_GetData(sql_handle, 0, &data, nullptr) &&
		data != nullptr )
	{// get account data
		uint32 account_id;
		account_id = atoi(data);
		if( mmo_auth_fromsql(db, acc, account_id) )
		{
			iter->last_account_id = account_id;
			Sql_FreeResult(sql_handle);
			return true;
		}
	}
	Sql_FreeResult(sql_handle);
	return false;
}

/**
 * Fetch a struct mmo_account from sql, excluding web_auth_token.
 * @param db: pointer to db
 * @param acc: pointer of mmo_account to fill
 * @param account_id: id of user account to take data from
 * @return true if successful, false if something has failed
 */
static bool mmo_auth_fromsql(AccountDB_SQL* db, struct mmo_account* acc, uint32 account_id) {
	Sql* sql_handle = db->accounts;
	char* data;

	// retrieve login entry for the specified account
	if( SQL_ERROR == Sql_Query(sql_handle,
#ifdef VIP_ENABLE
		"SELECT `account_id`,`userid`,`user_pass`,`sex`,`email`,`group_id`,`state`,`unban_time`,`expiration_time`,`logincount`,`lastlogin`,`last_ip`,`birthdate`,`character_slots`,`pincode`, `pincode_change`, `vip_time`, `old_group` FROM `%s` WHERE `account_id` = %d",
#else
		"SELECT `account_id`,`userid`,`user_pass`,`sex`,`email`,`group_id`,`state`,`unban_time`,`expiration_time`,`logincount`,`lastlogin`,`last_ip`,`birthdate`,`character_slots`,`pincode`, `pincode_change` FROM `%s` WHERE `account_id` = %d",
#endif
		db->account_db, account_id )
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, nullptr); acc->account_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, nullptr); safestrncpy(acc->userid, data, sizeof(acc->userid));
	Sql_GetData(sql_handle,  2, &data, nullptr); safestrncpy(acc->pass, data, sizeof(acc->pass));
	Sql_GetData(sql_handle,  3, &data, nullptr); acc->sex = data[0];
	Sql_GetData(sql_handle,  4, &data, nullptr); safestrncpy(acc->email, data, sizeof(acc->email));
	Sql_GetData(sql_handle,  5, &data, nullptr); acc->group_id = (unsigned int) atoi(data);
	Sql_GetData(sql_handle,  6, &data, nullptr); acc->state = (unsigned int) strtoul(data, nullptr, 10);
	Sql_GetData(sql_handle,  7, &data, nullptr); acc->unban_time = atol(data);
	Sql_GetData(sql_handle,  8, &data, nullptr); acc->expiration_time = atol(data);
	Sql_GetData(sql_handle,  9, &data, nullptr); acc->logincount = (unsigned int) strtoul(data, nullptr, 10);
	Sql_GetData(sql_handle, 10, &data, nullptr); safestrncpy(acc->lastlogin, data==nullptr?"":data, sizeof(acc->lastlogin));
	Sql_GetData(sql_handle, 11, &data, nullptr); safestrncpy(acc->last_ip, data, sizeof(acc->last_ip));
	Sql_GetData(sql_handle, 12, &data, nullptr); safestrncpy(acc->birthdate, data==nullptr?"":data, sizeof(acc->birthdate));
	Sql_GetData(sql_handle, 13, &data, nullptr); acc->char_slots = (uint8) atoi(data);
	Sql_GetData(sql_handle, 14, &data, nullptr); safestrncpy(acc->pincode, data, sizeof(acc->pincode));
	Sql_GetData(sql_handle, 15, &data, nullptr); acc->pincode_change = atol(data);
#ifdef VIP_ENABLE
	Sql_GetData(sql_handle, 16, &data, nullptr); acc->vip_time = atol(data);
	Sql_GetData(sql_handle, 17, &data, nullptr); acc->old_group = atoi(data);
#endif
	Sql_FreeResult(sql_handle);
	acc->web_auth_token[0] = '\0';

	if( acc->char_slots > MAX_CHARS ){
		ShowError( "Account %s (AID=%u) exceeds MAX_CHARS. Capping...\n", acc->userid, acc->account_id );
		acc->char_slots = MAX_CHARS;
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
static bool mmo_auth_tosql(AccountDB_SQL* db, const struct mmo_account* acc, bool is_new, bool refresh_token) {
	Sql* sql_handle = db->accounts;
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
	bool result = false;

	// try
	do
	{

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	if( is_new )
	{// insert into account table
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt,
#ifdef VIP_ENABLE
			"INSERT INTO `%s` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `group_id`, `state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, `last_ip`, `birthdate`, `character_slots`, `pincode`, `pincode_change`, `vip_time`, `old_group` ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
#else
			"INSERT INTO `%s` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `group_id`, `state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, `last_ip`, `birthdate`, `character_slots`, `pincode`, `pincode_change`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
#endif
			db->account_db)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  0, SQLDT_INT,       (void*)&acc->account_id,      sizeof(acc->account_id))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  1, SQLDT_STRING,    (void*)acc->userid,           strlen(acc->userid))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  2, SQLDT_STRING,    (void*)acc->pass,             strlen(acc->pass))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  3, SQLDT_ENUM,      (void*)&acc->sex,             sizeof(acc->sex))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  4, SQLDT_STRING,    (void*)&acc->email,           strlen(acc->email))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  5, SQLDT_INT,       (void*)&acc->group_id,        sizeof(acc->group_id))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  6, SQLDT_UINT,      (void*)&acc->state,           sizeof(acc->state))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  7, SQLDT_LONG,      (void*)&acc->unban_time,      sizeof(acc->unban_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  8, SQLDT_INT,       (void*)&acc->expiration_time, sizeof(acc->expiration_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  9, SQLDT_UINT,      (void*)&acc->logincount,      sizeof(acc->logincount))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, acc->lastlogin[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc->lastlogin,       strlen(acc->lastlogin))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, SQLDT_STRING,    (void*)&acc->last_ip,         strlen(acc->last_ip))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 12, acc->birthdate[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc->birthdate,       strlen(acc->birthdate))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 13, SQLDT_UCHAR,     (void*)&acc->char_slots,      sizeof(acc->char_slots))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 14, SQLDT_STRING,    (void*)&acc->pincode,         strlen(acc->pincode))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 15, SQLDT_LONG,      (void*)&acc->pincode_change,  sizeof(acc->pincode_change))
#ifdef VIP_ENABLE
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 16, SQLDT_LONG,       (void*)&acc->vip_time,         sizeof(acc->vip_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 17, SQLDT_INT,        (void*)&acc->old_group,        sizeof(acc->old_group))
#endif
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
			db->account_db, acc->account_id)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  0, SQLDT_STRING,    (void*)acc->userid,           strlen(acc->userid))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  1, SQLDT_STRING,    (void*)acc->pass,             strlen(acc->pass))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  2, SQLDT_ENUM,      (void*)&acc->sex,             sizeof(acc->sex))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  3, SQLDT_STRING,    (void*)acc->email,            strlen(acc->email))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  4, SQLDT_INT,       (void*)&acc->group_id,        sizeof(acc->group_id))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  5, SQLDT_UINT,      (void*)&acc->state,           sizeof(acc->state))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  6, SQLDT_LONG,      (void*)&acc->unban_time,      sizeof(acc->unban_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  7, SQLDT_LONG,      (void*)&acc->expiration_time, sizeof(acc->expiration_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  8, SQLDT_UINT,      (void*)&acc->logincount,      sizeof(acc->logincount))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  9, acc->lastlogin[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc->lastlogin,       strlen(acc->lastlogin))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, SQLDT_STRING,    (void*)&acc->last_ip,         strlen(acc->last_ip))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, acc->birthdate[0]?SQLDT_STRING:SQLDT_NULL,    (void*)&acc->birthdate,       strlen(acc->birthdate))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 12, SQLDT_UCHAR,     (void*)&acc->char_slots,      sizeof(acc->char_slots))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 13, SQLDT_STRING,    (void*)&acc->pincode,         strlen(acc->pincode))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 14, SQLDT_LONG,      (void*)&acc->pincode_change,  sizeof(acc->pincode_change))
#ifdef VIP_ENABLE
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 15, SQLDT_LONG,      (void*)&acc->vip_time,        sizeof(acc->vip_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 16, SQLDT_INT,       (void*)&acc->old_group,       sizeof(acc->old_group))
#endif
		||  SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
			SqlStmt_ShowDebug(stmt);
			break;
		}
	}

	if( acc->sex != 'S' && login_config.use_web_auth_token && refresh_token ){
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
			if( SQL_SUCCESS == Sql_Query( sql_handle, query, db->account_db, WEB_AUTH_TOKEN_LENGTH - 1, acc->account_id ) ){
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

		if( SQL_SUCCESS != Sql_Query( sql_handle, "SELECT `web_auth_token` from `%s` WHERE `account_id` = '%d'", db->account_db, acc->account_id ) ||
			SQL_SUCCESS != Sql_NextRow( sql_handle ) ||
			SQL_SUCCESS != Sql_GetData( sql_handle, 0, &data, &len )
			){
			Sql_ShowDebug( sql_handle );
			break;
		}

		safestrncpy( (char *)&acc->web_auth_token, data, sizeof( acc->web_auth_token ) );

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

void mmo_save_global_accreg(AccountDB* self, int fd, uint32 account_id, uint32 char_id) {
	Sql* sql_handle = ((AccountDB_SQL*)self)->accounts;
	AccountDB_SQL* db = (AccountDB_SQL*)self;
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
					if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%" PRIu32 "','%s','%" PRIu32 "','%" PRId64 "')", db->global_acc_reg_num_table, account_id, esc_key, index, RFIFOQ(fd, cursor)) )
						Sql_ShowDebug(sql_handle);
					cursor += 8;
					break;
				case 1:
					if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%" PRIu32 "' AND `key` = '%s' AND `index` = '%" PRIu32 "' LIMIT 1", db->global_acc_reg_num_table, account_id, esc_key, index) )
						Sql_ShowDebug(sql_handle);
					break;
				// str
				case 2:
					safestrncpy(sval, RFIFOCP(fd, cursor + 1), RFIFOB(fd, cursor));
					cursor += RFIFOB(fd, cursor) + 1;
					Sql_EscapeString(sql_handle, esc_sval, sval);
					if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%" PRIu32 "','%s','%" PRIu32 "','%s')", db->global_acc_reg_str_table, account_id, esc_key, index, esc_sval) )
						Sql_ShowDebug(sql_handle);
					break;
				case 3:
					if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%" PRIu32 "' AND `key` = '%s' AND `index` = '%" PRIu32 "' LIMIT 1", db->global_acc_reg_str_table, account_id, esc_key, index) )
						Sql_ShowDebug(sql_handle);
					break;
				default:
					ShowError("mmo_save_global_accreg: unknown type %d\n",RFIFOB(fd, cursor - 1));
					return;
			}
		}
	}
}

void mmo_send_global_accreg(AccountDB* self, int fd, uint32 account_id, uint32 char_id) {
	Sql* sql_handle = ((AccountDB_SQL*)self)->accounts;
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	char* data;
	int16 plen = 0;
	size_t len;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%" PRIu32 "'", db->global_acc_reg_str_table, account_id) )
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
		Sql_GetData(sql_handle, 0, &data, nullptr);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 32
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += static_cast<decltype(plen)>( len );

		Sql_GetData(sql_handle, 1, &data, nullptr);

		WFIFOL(fd, plen) = (uint32)atol(data);
		plen += 4;

		Sql_GetData(sql_handle, 2, &data, nullptr);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 254
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += static_cast<decltype(plen)>( len );

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

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%" PRIu32 "'", db->global_acc_reg_num_table, account_id) )
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
		Sql_GetData(sql_handle, 0, &data, nullptr);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 32
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += static_cast<decltype(plen)>( len );

		Sql_GetData(sql_handle, 1, &data, nullptr);

		WFIFOL(fd, plen) = (uint32)atol(data);
		plen += 4;

		Sql_GetData(sql_handle, 2, &data, nullptr);

		WFIFOQ(fd, plen) = strtoll(data,nullptr,10);
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

bool account_db_sql_enable_webtoken( AccountDB* self, const uint32 account_id ){
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	if( SQL_ERROR == Sql_Query( db->accounts, "UPDATE `%s` SET `web_auth_token_enabled` = '1' WHERE `account_id` = '%u'", db->account_db, account_id ) ){
		Sql_ShowDebug( db->accounts );
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
	AccountDB_SQL* db = reinterpret_cast<AccountDB_SQL*>(data);

	if (p == nullptr) {
		ShowInfo("Web Auth Token for account %d was disabled\n", id);
		if( SQL_ERROR == Sql_Query( db->accounts, "UPDATE `%s` SET `web_auth_token_enabled` = '0' WHERE `account_id` = '%u'", db->account_db, id ) ){
			Sql_ShowDebug( db->accounts );
			return 0;
		}
	}

	return 0;
}



bool account_db_sql_disable_webtoken( AccountDB* self, const uint32 account_id ){
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	add_timer(gettick() + login_config.disable_webtoken_delay, account_disable_webtoken_timer, account_id, reinterpret_cast<intptr_t>(db));

	return true;
}

bool account_db_sql_remove_webtokens( AccountDB* self ){
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	if( SQL_ERROR == Sql_Query( db->accounts, "UPDATE `%s` SET `web_auth_token` = NULL, `web_auth_token_enabled` = '0'", db->account_db ) ){
		Sql_ShowDebug( db->accounts );
		return false;
	}

	return true;
}
