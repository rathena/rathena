// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "int_status.h"

#include <stdio.h>

// Contains all the status change data in-memory. [Skotlex]
static DBMap* scdata_db = NULL; // int char_id -> struct scdata*
char scdata_txt[1024]="save/scdata.txt"; //By [Skotlex]

#ifdef ENABLE_SC_SAVING
static void* create_scdata(DBKey key, va_list args)
{
	struct scdata *data;
	data = (struct scdata*)aCalloc(1, sizeof(struct scdata));
	data->account_id = va_arg(args, int);
	data->char_id = key.i;
	return data;
}

/*==========================================
 * Loads status change data of the player given. [Skotlex]
 *------------------------------------------*/
struct scdata* status_search_scdata(int aid, int cid)
{
	return (struct scdata*)scdata_db->ensure(scdata_db, i2key(cid), create_scdata, aid);
}

/*==========================================
 * Deletes status change data of the player given. [Skotlex]
 * Should be invoked after the data of said player was successfully loaded.
 *------------------------------------------*/
void status_delete_scdata(int aid, int cid)
{
	struct scdata* scdata = (struct scdata*)idb_remove(scdata_db, cid);
	if (scdata)
	{
		if (scdata->data)
			aFree(scdata->data);
		aFree(scdata);
	}
}


static void inter_status_tostr(char* line, struct scdata *sc_data)
{
	int i, len;

	len = sprintf(line, "%d,%d,%d\t", sc_data->account_id, sc_data->char_id, sc_data->count);
	for(i = 0; i < sc_data->count; i++) {
		len += sprintf(line + len, "%d,%d,%d,%d,%d,%d\t", sc_data->data[i].type, sc_data->data[i].tick,
			sc_data->data[i].val1, sc_data->data[i].val2, sc_data->data[i].val3, sc_data->data[i].val4);
	}
	return;
}

static int inter_scdata_fromstr(char *line, struct scdata *sc_data)
{
	int i, len, next;
	
	if (sscanf(line,"%d,%d,%d\t%n",&sc_data->account_id, &sc_data->char_id, &sc_data->count, &next) < 3)
		return 0;

	if (sc_data->count < 1)
		return 0;
	
	sc_data->data = (struct status_change_data*)aCalloc(sc_data->count, sizeof (struct status_change_data));

	for (i = 0; i < sc_data->count; i++)
	{
		if (sscanf(line + next, "%hu,%d,%d,%d,%d,%d\t%n", &sc_data->data[i].type, &sc_data->data[i].tick,
			&sc_data->data[i].val1, &sc_data->data[i].val2, &sc_data->data[i].val3, &sc_data->data[i].val4, &len) < 6)
		{
			aFree(sc_data->data);
			return 0;
		}
		next+=len;
	}
	return 1;
}
/*==========================================
 * Loads all scdata from the given filename.
 *------------------------------------------*/
void status_load_scdata(const char* filename)
{
	FILE *fp;
	int sd_count=0, sc_count=0;
	char line[8192];
	struct scdata *sc;

	if ((fp = fopen(filename, "r")) == NULL) {
		ShowError("status_load_scdata: Cannot open file %s!\n", filename);
		return;
	}

	while(fgets(line, sizeof(line), fp))
	{
		sc = (struct scdata*)aCalloc(1, sizeof(struct scdata));
		if (inter_scdata_fromstr(line, sc)) {
			sd_count++;
			sc_count+= sc->count;
			sc = (struct scdata*)idb_put(scdata_db, sc->char_id, sc);
			if (sc) {
				ShowError("Duplicate entry in %s for character %d\n", filename, sc->char_id);
				if (sc->data) aFree(sc->data);
				aFree(sc);
			}
		} else {
			ShowError("status_load_scdata: Broken line data: %s\n", line);
			aFree(sc);
		}
	}
	fclose(fp);
	ShowStatus("Loaded %d saved status changes for %d characters.\n", sc_count, sd_count);
}

static int inter_status_save_sub(DBKey key, void *data, va_list ap) {
	char line[8192];
	struct scdata * sc_data;
	FILE *fp;

	sc_data = (struct scdata *)data;
	if (sc_data->count < 1)
		return 0;
	
	fp = va_arg(ap, FILE *);
	inter_status_tostr(line, sc_data);
	fprintf(fp, "%s\n", line);
	return 0;
}

/*==========================================
 * Saves all scdata to the given filename.
 *------------------------------------------*/
void inter_status_save()
{
	FILE *fp;
	int lock;

	if ((fp = lock_fopen(scdata_txt, &lock)) == NULL) {
		ShowError("int_status: can't write [%s] !!! data is lost !!!\n", scdata_txt);
		return;
	}
	scdata_db->foreach(scdata_db, inter_status_save_sub, fp);
	lock_fclose(fp,scdata_txt, &lock);
}

/*==========================================
 * Initializes db.
 *------------------------------------------*/
void status_init()
{
	scdata_db = idb_alloc(DB_OPT_BASE);
	status_load_scdata(scdata_txt);
}

/*==========================================
 * Frees up memory.
 *------------------------------------------*/
static int scdata_db_final(DBKey k,void *d,va_list ap)
{
	struct scdata *data = (struct scdata*)d;
	if (data->data)
		aFree(data->data);
	aFree(data);
	return 0;
}

/*==========================================
 * Final cleanup.
 *------------------------------------------*/
void status_final(void)
{
	scdata_db->destroy(scdata_db, scdata_db_final);
}

#endif //ENABLE_SC_SAVING
