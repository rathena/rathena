// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// global defines
#define ACCOUNT_TXT_DB_VERSION 20080409
#define AUTHS_BEFORE_SAVE 10 // flush every 10 saves
#define AUTH_SAVING_INTERVAL 60000 // flush every 10 minutes

/// internal structure
typedef struct AccountDB_TXT
{
	AccountDB vtable;      // public interface

	DBMap* accounts;       // in-memory accounts storage
	int next_account_id;   // auto_increment
	int auths_before_save; // prevents writing to disk too often
	int save_timer;        // save timer id

	char account_db[1024]; // account data storage file
	bool case_sensitive;   // how to look up usernames

} AccountDB_TXT;

/// internal structure
typedef struct AccountDBIterator_TXT
{
	AccountDBIterator vtable;      // public interface

	DBIterator* iter;
} AccountDBIterator_TXT;

/// internal functions
static bool account_db_txt_init(AccountDB* self);
static void account_db_txt_destroy(AccountDB* self);
static bool account_db_txt_get_property(AccountDB* self, const char* key, char* buf, size_t buflen);
static bool account_db_txt_set_property(AccountDB* self, const char* option, const char* value);
static bool account_db_txt_create(AccountDB* self, struct mmo_account* acc);
static bool account_db_txt_remove(AccountDB* self, const int account_id);
static bool account_db_txt_save(AccountDB* self, const struct mmo_account* acc);
static bool account_db_txt_load_num(AccountDB* self, struct mmo_account* acc, const int account_id);
static bool account_db_txt_load_str(AccountDB* self, struct mmo_account* acc, const char* userid);
static AccountDBIterator* account_db_txt_iterator(AccountDB* self);
static void account_db_txt_iter_destroy(AccountDBIterator* self);
static bool account_db_txt_iter_next(AccountDBIterator* self, struct mmo_account* acc);

static bool mmo_auth_fromstr(struct mmo_account* acc, char* str, unsigned int version);
static bool mmo_auth_tostr(const struct mmo_account* acc, char* str);
static void mmo_auth_sync(AccountDB_TXT* self);
static int mmo_auth_sync_timer(int tid, unsigned int tick, int id, intptr data);

/// public constructor
AccountDB* account_db_txt(void)
{
	AccountDB_TXT* db = (AccountDB_TXT*)aCalloc(1, sizeof(AccountDB_TXT));

	// set up the vtable
	db->vtable.init         = &account_db_txt_init;
	db->vtable.destroy      = &account_db_txt_destroy;
	db->vtable.get_property = &account_db_txt_get_property;
	db->vtable.set_property = &account_db_txt_set_property;
	db->vtable.save         = &account_db_txt_save;
	db->vtable.create       = &account_db_txt_create;
	db->vtable.remove       = &account_db_txt_remove;
	db->vtable.load_num     = &account_db_txt_load_num;
	db->vtable.load_str     = &account_db_txt_load_str;
	db->vtable.iterator     = &account_db_txt_iterator;

	// initialize to default values
	db->accounts = NULL;
	db->next_account_id = START_ACCOUNT_NUM;
	db->auths_before_save = AUTHS_BEFORE_SAVE;
	db->save_timer = INVALID_TIMER;
	safestrncpy(db->account_db, "save/account.txt", sizeof(db->account_db));
	db->case_sensitive = false;

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


/// opens accounts file, loads it, and starts a periodic saving timer
static bool account_db_txt_init(AccountDB* self)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts;
	FILE* fp;
	char line[2048];
	unsigned int version = 0;

	// create accounts database
	db->accounts = idb_alloc(DB_OPT_RELEASE_DATA);
	accounts = db->accounts;

	// open data file
	fp = fopen(db->account_db, "r");
	if( fp == NULL )
	{
		// no account file -> no account -> no login, including char-server (ERROR)
		ShowError(CL_RED"account_db_txt_init: Accounts file [%s] not found."CL_RESET"\n", db->account_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) != NULL )
	{
		int account_id, n;
		unsigned int v;
		struct mmo_account acc;
		struct mmo_account* tmp;
		struct DBIterator* iter;
		int (*compare)(const char* str1, const char* str2) = ( db->case_sensitive ) ? strcmp : stricmp;

		if( line[0] == '/' && line[1] == '/' )
			continue;

		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		if( sscanf(line, "%d\t%%newid%%%n", &account_id, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( account_id > db->next_account_id )
				db->next_account_id = account_id;
			continue;
		}

		if( !mmo_auth_fromstr(&acc, line, version) )
		{
			ShowError("account_db_txt_init: skipping invalid data: %s", line);
			continue;
		}

		// apply constraints & checks here
		if( acc.sex != 'S' && (acc.account_id < START_ACCOUNT_NUM || acc.account_id > END_ACCOUNT_NUM) )
			ShowWarning("account_db_txt_init: account %d:'%s' has ID outside of the defined range for accounts (min:%d max:%d)!\n", acc.account_id, acc.userid, START_ACCOUNT_NUM, END_ACCOUNT_NUM);

		iter = accounts->iterator(accounts);
		for( tmp = (struct mmo_account*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct mmo_account*)iter->next(iter,NULL) )
			if( compare(acc.userid, tmp->userid) == 0 )
					break;
		iter->destroy(iter);

		if( tmp != NULL )
		{// entry with identical username
			ShowWarning("account_db_txt_init: account %d:'%s' has same username as account %d. The account will be inaccessible!\n", acc.account_id, acc.userid, tmp->account_id);
		}

		if( idb_get(accounts, acc.account_id) != NULL )
		{// account id already occupied
			ShowError("account_db_txt_init: ID collision for account id %d! Discarding data for account '%s'...\n", acc.account_id, acc.userid);
			continue;
		}

		// record entry in db
		tmp = (struct mmo_account*)aMalloc(sizeof(struct mmo_account));
		memcpy(tmp, &acc, sizeof(struct mmo_account));
		idb_put(accounts, acc.account_id, tmp);

		if( db->next_account_id < acc.account_id)
			db->next_account_id = acc.account_id + 1;
	}

	// close data file
	fclose(fp);

	// initialize data saving timer
	add_timer_func_list(mmo_auth_sync_timer, "mmo_auth_sync_timer");
	db->save_timer = add_timer_interval(gettick() + AUTH_SAVING_INTERVAL, mmo_auth_sync_timer, 0, (int)db, AUTH_SAVING_INTERVAL);

	return true;
}

/// flush accounts db, close savefile and deallocate structures
static void account_db_txt_destroy(AccountDB* self)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts = db->accounts;

	// stop saving timer
	delete_timer(db->save_timer, mmo_auth_sync_timer);

	// write data
	mmo_auth_sync(db);

	// delete accounts database
	accounts->destroy(accounts, NULL);
	db->accounts = NULL;

	// delete entire structure
	aFree(db);
}

/// Gets a property from this database.
static bool account_db_txt_get_property(AccountDB* self, const char* key, char* buf, size_t buflen)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	const char* signature = "account.txt.";

	if( strcmp(key, "engine.name") == 0 )
	{
		safesnprintf(buf, buflen, "txt");
		return true;
	}
	if( strcmp(key, "engine.version") == 0 )
	{
		safesnprintf(buf, buflen, "%d", ACCOUNT_TXT_DB_VERSION);
		return true;
	}
	if( strcmp(key, "engine.comment") == 0 )
	{
		safesnprintf(buf, buflen, "TXT Account Database %d", ACCOUNT_TXT_DB_VERSION);
		return true;
	}

	if( strncmp(key, signature, strlen(signature)) != 0 )
		return false;

	key += strlen(signature);

	if( strcmpi(key, "account_db") == 0 )
		safesnprintf(buf, buflen, "%s", db->account_db);
	else if( strcmpi(key, "case_sensitive") == 0 )
		safesnprintf(buf, buflen, "%d", (db->case_sensitive ? 1 : 0));
	else
		return false;// not found

	return true;
}

/// Sets a property in this database.
static bool account_db_txt_set_property(AccountDB* self, const char* key, const char* value)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	const char* signature = "account.txt.";

	if( strncmp(key, signature, strlen(signature)) != 0 )
		return false;

	key += strlen(signature);

	if( strcmpi(key, "account_db") == 0 )
		safestrncpy(db->account_db, value, sizeof(db->account_db));
	else if( strcmpi(key, "case_sensitive") == 0 )
		db->case_sensitive = config_switch(value);
	else // no match
		return false;

	return true;
}

/// Add a new entry for this account to the account db and save it.
/// If acc->account_id is -1, the account id will be auto-generated,
/// and its value will be written to acc->account_id if everything succeeds.
static bool account_db_txt_create(AccountDB* self, struct mmo_account* acc)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts = db->accounts;
	struct mmo_account* tmp;

	// decide on the account id to assign
	int account_id = ( acc->account_id != -1 ) ? acc->account_id : db->next_account_id;

	// absolute maximum
	if( account_id > END_ACCOUNT_NUM )
		return false;

	// check if the account_id is free
	tmp = idb_get(accounts, account_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("account_db_txt_create: cannot create account %d:'%s', this id is already occupied by %d:'%s'!\n", account_id, acc->userid, account_id, tmp->userid);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct mmo_account, 1);
	memcpy(tmp, acc, sizeof(struct mmo_account));
	tmp->account_id = account_id;
	idb_put(accounts, account_id, tmp);

	// increment the auto_increment value
	if( account_id >= db->next_account_id )
		db->next_account_id = account_id + 1;

	// flush data
	mmo_auth_sync(db);

	// write output
	acc->account_id = account_id;

	return true;
}

/// find an existing entry for this account id and delete it
static bool account_db_txt_remove(AccountDB* self, const int account_id)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts = db->accounts;

	//TODO: find out if this really works
	struct mmo_account* tmp = idb_remove(accounts, account_id);
	if( tmp == NULL )
	{// error condition - entry not present
		ShowError("account_db_txt_remove: no such account with id %d\n", account_id);
		return false;
	}

	// flush data
	mmo_auth_sync(db);

	return true;
}

/// rewrite the data stored in the account_db with the one provided
static bool account_db_txt_save(AccountDB* self, const struct mmo_account* acc)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts = db->accounts;
	int account_id = acc->account_id;

	// retrieve previous data
	struct mmo_acount* tmp = idb_get(accounts, account_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, acc, sizeof(struct mmo_account));

	// modify save counter and save if needed
	if( --db->auths_before_save == 0 )
		mmo_auth_sync(db);

	return true;
}

/// retrieve data from db and store it in the provided data structure
static bool account_db_txt_load_num(AccountDB* self, struct mmo_account* acc, const int account_id)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts = db->accounts;

	// retrieve data
	struct mmo_account* tmp = idb_get(accounts, account_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(acc, tmp, sizeof(struct mmo_account));

	return true;
}

/// retrieve data from db and store it in the provided data structure
static bool account_db_txt_load_str(AccountDB* self, struct mmo_account* acc, const char* userid)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts = db->accounts;

	// retrieve data
	struct DBIterator* iter = accounts->iterator(accounts);
	struct mmo_account* tmp;
	int (*compare)(const char* str1, const char* str2) = ( db->case_sensitive ) ? strcmp : stricmp;

	for( tmp = (struct mmo_account*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct mmo_account*)iter->next(iter,NULL) )
		if( compare(userid, tmp->userid) == 0 )
			break;
	iter->destroy(iter);

	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(acc, tmp, sizeof(struct mmo_account));

	return true;
}


/// Returns a new forward iterator.
static AccountDBIterator* account_db_txt_iterator(AccountDB* self)
{
	AccountDB_TXT* db = (AccountDB_TXT*)self;
	DBMap* accounts = db->accounts;
	AccountDBIterator_TXT* iter = (AccountDBIterator_TXT*)aCalloc(1, sizeof(AccountDBIterator_TXT));

	// set up the vtable
	iter->vtable.destroy = &account_db_txt_iter_destroy;
	iter->vtable.next    = &account_db_txt_iter_next;

	// fill data
	iter->iter = db_iterator(accounts);

	return &iter->vtable;
}


/// Destroys this iterator, releasing all allocated memory (including itself).
static void account_db_txt_iter_destroy(AccountDBIterator* self)
{
	AccountDBIterator_TXT* iter = (AccountDBIterator_TXT*)self;
	dbi_destroy(iter->iter);
	aFree(iter);
}


/// Fetches the next account in the database.
static bool account_db_txt_iter_next(AccountDBIterator* self, struct mmo_account* acc)
{
	AccountDBIterator_TXT* iter = (AccountDBIterator_TXT*)self;
	struct mmo_account* tmp = (struct mmo_account*)dbi_next(iter->iter);
	if( dbi_exists(iter->iter) )
	{
		memcpy(acc, tmp, sizeof(struct mmo_account));
		return true;
	}
	return false;
}


/// parse input string into the provided account data structure
static bool mmo_auth_fromstr(struct mmo_account* a, char* str, unsigned int version)
{
	char* fields[32];
	int count;
	char* regs;
	int i, n;

	// zero out the destination first
	memset(a, 0x00, sizeof(struct mmo_account));

	// extract tab-separated columns from line
	count = sv_split(str, strlen(str), 0, '\t', fields, ARRAYLENGTH(fields), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));

	if( version == ACCOUNT_TXT_DB_VERSION && count == 13 )
	{
		a->account_id = strtol(fields[1], NULL, 10);
		safestrncpy(a->userid, fields[2], sizeof(a->userid));
		safestrncpy(a->pass, fields[3], sizeof(a->pass));
		a->sex = fields[4][0];
		safestrncpy(a->email, fields[5], sizeof(a->email));
		a->level = strtoul(fields[6], NULL, 10);
		a->state = strtoul(fields[7], NULL, 10);
		a->unban_time = strtol(fields[8], NULL, 10);
		a->expiration_time = strtol(fields[9], NULL, 10);
		a->logincount = strtoul(fields[10], NULL, 10);
		safestrncpy(a->lastlogin, fields[11], sizeof(a->lastlogin));
		safestrncpy(a->last_ip, fields[12], sizeof(a->last_ip));
		regs = fields[13];
	}
	else
	if( version == 0 && count == 14 )
	{
		a->account_id = strtol(fields[1], NULL, 10);
		safestrncpy(a->userid, fields[2], sizeof(a->userid));
		safestrncpy(a->pass, fields[3], sizeof(a->pass));
		safestrncpy(a->lastlogin, fields[4], sizeof(a->lastlogin));
		a->sex = fields[5][0];
		a->logincount = strtoul(fields[6], NULL, 10);
		a->state = strtoul(fields[7], NULL, 10);
		safestrncpy(a->email, fields[8], sizeof(a->email));
		//safestrncpy(a->error_message, fields[9], sizeof(a->error_message));
		a->expiration_time = strtol(fields[10], NULL, 10);
		safestrncpy(a->last_ip, fields[11], sizeof(a->last_ip));
		//safestrncpy(a->memo, fields[12], sizeof(a->memo));
		a->unban_time = strtol(fields[13], NULL, 10);
		regs = fields[14];
	}
	else
	if( version == 0 && count == 13 )
	{
		a->account_id = strtol(fields[1], NULL, 10);
		safestrncpy(a->userid, fields[2], sizeof(a->userid));
		safestrncpy(a->pass, fields[3], sizeof(a->pass));
		safestrncpy(a->lastlogin, fields[4], sizeof(a->lastlogin));
		a->sex = fields[5][0];
		a->logincount = strtoul(fields[6], NULL, 10);
		a->state = strtoul(fields[7], NULL, 10);
		safestrncpy(a->email, fields[8], sizeof(a->email));
		//safestrncpy(a->error_message, fields[9], sizeof(a->error_message));
		a->expiration_time = strtol(fields[10], NULL, 10);
		safestrncpy(a->last_ip, fields[11], sizeof(a->last_ip));
		//safestrncpy(a->memo, fields[12], sizeof(a->memo));
		regs = fields[13];
	}
	else
	if( version == 0 && count == 8 )
	{
		a->account_id = strtol(fields[1], NULL, 10);
		safestrncpy(a->userid, fields[2], sizeof(a->userid));
		safestrncpy(a->pass, fields[3], sizeof(a->pass));
		safestrncpy(a->lastlogin, fields[4], sizeof(a->lastlogin));
		a->sex = fields[5][0];
		a->logincount = strtoul(fields[6], NULL, 10);
		a->state = strtoul(fields[7], NULL, 10);
		regs = fields[8];
	}
	else
	{// unmatched row
		return false;
	}

	// extract account regs
	// {reg name<COMMA>reg value<SPACE>}*
	n = 0;
	for( i = 0; i < ACCOUNT_REG2_NUM; ++i )
	{
		char key[32];
		char value[256];
	
		regs += n;

		if (sscanf(regs, "%31[^\t,],%255[^\t ] %n", key, value, &n) != 2)
		{
			// We must check if a str is void. If it's, we can continue to read other REG2.
			// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
			if (regs[0] == ',' && sscanf(regs, ",%[^\t ] %n", value, &n) == 1) { 
				i--;
				continue;
			} else
				break;
		}
		
		safestrncpy(a->account_reg2[i].str, key, 32);
		safestrncpy(a->account_reg2[i].value, value, 256);
	}
	a->account_reg2_num = i;

	return true;
}

/// dump the contents of the account data structure into the provided string buffer
static bool mmo_auth_tostr(const struct mmo_account* a, char* str)
{
	int i;
	char* str_p = str;

	str_p += sprintf(str_p, "%d\t%s\t%s\t%c\t%s\t%u\t%u\t%ld\t%ld\t%u\t%s\t%s\t",
	                 a->account_id, a->userid, a->pass, a->sex, a->email, a->level,
	                 a->state, (long)a->unban_time, (long)a->expiration_time,
	                 a->logincount, a->lastlogin, a->last_ip);

	for( i = 0; i < a->account_reg2_num; ++i )
		if( a->account_reg2[i].str[0] )
			str_p += sprintf(str_p, "%s,%s ", a->account_reg2[i].str, a->account_reg2[i].value);

	return true;
}

/// dump the entire account db to disk
static void mmo_auth_sync(AccountDB_TXT* db)
{
	int lock;
	FILE *fp;
	struct DBIterator* iter;
	struct mmo_account* acc;

	fp = lock_fopen(db->account_db, &lock);
	if( fp == NULL )
	{
		return;
	}

	fprintf(fp, "%d\n", ACCOUNT_TXT_DB_VERSION); // savefile version

	fprintf(fp, "// Accounts file: here are saved all information about the accounts.\n");
	fprintf(fp, "// Structure: account ID, username, password, sex, email, level, state, unban time, expiration time, # of logins, last login time, last (accepted) login ip, repeated(register key, register value)\n");
	fprintf(fp, "// where:\n");
	fprintf(fp, "//   sex             : M or F for normal accounts, S for server accounts\n");
	fprintf(fp, "//   level           : this account's gm level\n");
	fprintf(fp, "//   state           : 0: account is ok, 1 to 256: error code of packet 0x006a + 1\n");
	fprintf(fp, "//   unban time      : 0: no ban, <other value>: banned until the date (unix timestamp)\n");
	fprintf(fp, "//   expiration time : 0: unlimited account, <other value>: account expires on the date (unix timestamp)\n");

	//TODO: sort?

	iter = db->accounts->iterator(db->accounts);
	for( acc = (struct mmo_account*)iter->first(iter,NULL); iter->exists(iter); acc = (struct mmo_account*)iter->next(iter,NULL) )
	{
		char buf[2048]; // ought to be big enough ^^
		mmo_auth_tostr(acc, buf);
		fprintf(fp, "%s\n", buf);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_account_id);
	iter->destroy(iter);

	lock_fclose(fp, db->account_db, &lock);

	// reset save counter
	db->auths_before_save = AUTHS_BEFORE_SAVE;
}

static int mmo_auth_sync_timer(int tid, unsigned int tick, int id, intptr data)
{
	AccountDB_TXT* db = (AccountDB_TXT*)data;

	if( db->auths_before_save < AUTHS_BEFORE_SAVE )
		mmo_auth_sync(db); // db was modified, flush it

	return 0;
}
