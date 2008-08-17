// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "script.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DBMap* mapreg_db = NULL; // int var_id -> int value
static DBMap* mapregstr_db = NULL; // int var_id -> char* value

static char mapreg_txt[256] = "save/mapreg.txt";
static bool mapreg_dirty = false;
#define MAPREG_AUTOSAVE_INTERVAL (300*1000)


/// Looks up the value of an integer variable using its uid.
int mapreg_readreg(int uid)
{
	return (int)idb_get(mapreg_db, uid);
}

/// Looks up the value of a string variable using its uid.
char* mapreg_readregstr(int uid)
{
	return (char*)idb_get(mapregstr_db, uid);
}

/// Modifies the value of an integer variable.
bool mapreg_setreg(int uid, int val)
{
	if( val != 0 )
		idb_put(mapreg_db,uid,(void*)val);
	else
		idb_remove(mapreg_db,uid);

	mapreg_dirty = true;
	return true;
}

/// Modifies the value of a string variable.
bool mapreg_setregstr(int uid, const char* str)
{
	if( str == NULL || *str == 0 )
		idb_remove(mapregstr_db,uid);
	else
		idb_put(mapregstr_db,uid,aStrdup(str));

	mapreg_dirty = true;
	return true;
}

/// Loads permanent variables from savefile
static void script_load_mapreg(void)
{
	FILE* fp;
	char line[1024];

	fp = fopen(mapreg_txt,"rt");
	if( fp == NULL )
		return;

	while( fgets(line,sizeof(line),fp) )
	{
		char varname[32+1];
		char value[255+1];
		int n,s,i;

		// read name and index
		if( sscanf(line, "%255[^,],%d\t%n", varname,&i,&n) != 2 &&
			(i = 0, sscanf(line,"%[^\t]\t%n", varname,&n) != 1) )
			continue;

		// read value
		if( sscanf(line + n, "%[^\n\r]", value) != 1 )
		{
			ShowError("%s: %s broken data !\n", mapreg_txt, varname);
			continue;
		}

		s = add_str(varname);
		if( varname[strlen(varname)-1] == '$' )
			idb_put(mapregstr_db, (i<<24)|s, aStrdup(value));
		else
			idb_put(mapreg_db, (i<<24)|s, (void*)atoi(value));
	}
	fclose(fp);

	mapreg_dirty = false;
}

/// Saves permanent variables to savefile
static void script_save_mapreg(void)
{
	FILE *fp;
	int lock;
	DBIterator* iter;
	void* data;
	DBKey key;

	fp = lock_fopen(mapreg_txt,&lock);
	if( fp == NULL )
	{
		ShowError("script_save_mapreg: Unable to lock-open file [%s]!\n", mapreg_txt);
		return;
	}

	iter = mapreg_db->iterator(mapreg_db);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int num = (key.i & 0x00ffffff);
		int i   = (key.i & 0xff000000) >> 24;
		const char* name = get_str(num);

		if( name[1] == '@' )
			continue;

		if( i == 0 )
			fprintf(fp, "%s\t%d\n", name, (int)data);
		else
			fprintf(fp, "%s,%d\t%d\n", name, i, (int)data);
	}
	iter->destroy(iter);

	iter = mapregstr_db->iterator(mapregstr_db);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int num = (key.i & 0x00ffffff);
		int i   = (key.i & 0xff000000) >> 24;
		const char* name = get_str(num);

		if( name[1] == '@' )
			continue;

		if( i == 0 )
			fprintf(fp, "%s\t%s\n", name, (char *)data);
		else
			fprintf(fp, "%s,%d\t%s\n", name, i, (char *)data);
	}
	iter->destroy(iter);

	lock_fclose(fp,mapreg_txt,&lock);

	mapreg_dirty = false;
}

static int script_autosave_mapreg(int tid, unsigned int tick, int id, intptr data)
{
	if( mapreg_dirty )
		script_save_mapreg();

	return 0;
}


void mapreg_reload(void)
{
	if( mapreg_dirty )
		script_save_mapreg();

	mapreg_db->clear(mapreg_db, NULL);
	mapregstr_db->clear(mapregstr_db, NULL);

	script_load_mapreg();
}

void mapreg_final(void)
{
	if( mapreg_dirty )
		script_save_mapreg();

	mapreg_db->destroy(mapreg_db,NULL);
	mapregstr_db->destroy(mapregstr_db,NULL);
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
	if(!strcmpi(w1, "mapreg_txt"))
		safestrncpy(mapreg_txt, w2, sizeof(mapreg_txt));
	else
		return false;

	return true;
}
