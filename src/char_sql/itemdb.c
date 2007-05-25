// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "itemdb.h"
#include "db.h"
#include "inter.h"
#include "char.h"
#include "utils.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"

#define MAX_RANDITEM	2000

// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

char item_db_db[256]="item_db"; // added to specify item_db sql table [Valaris]
char item_db2_db[256]="item_db2";

static struct dbt* item_db;

static void* create_item(DBKey key, va_list args) {
	struct item_data *id;
	int nameid = key.i;
	CREATE(id, struct item_data, 1);
	id->nameid = nameid;
	id->type = IT_ETC;
	return id;
}
/*==========================================
 * DBの検索
 *------------------------------------------*/
struct item_data* itemdb_search(int nameid)
{
	return idb_ensure(item_db,nameid,create_item);
}

/*==========================================
 *
 *------------------------------------------*/
int itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	if(type==IT_HEALING || type==IT_USABLE || type==IT_ETC || type==IT_CARD || type==IT_AMMO)
		return 0;
	return 1;
}
/*==========================================
 *
 *------------------------------------------*/
int itemdb_isequip2(struct item_data *data)
{
	if(data) {
		int type=data->type;
		if(type==IT_HEALING || type==IT_USABLE || type==IT_ETC || type==IT_CARD || type==IT_AMMO)
			return 0;
		else
			return 1;
	}
	return 0;
}



/*==========================================
 * アイテムデータベースの読み込み
 *------------------------------------------*/
static int itemdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0,lines=0;
	int nameid,j;
	char *str[32],*p,*np;
	struct item_data *id;
	int i=0;
	char *filename[]={ "item_db.txt","item_db2.txt" };

	for(i=0;i<2;i++){
		sprintf(line, "%s/%s", db_path, filename[i]);
		fp=fopen(line,"r");
		if(fp==NULL){
			if(i>0)
				continue;
			ShowFatalError("can't read %s\n",line);
			exit(1);
		}

		lines=0;
		while(fgets(line, sizeof(line), fp))
		{
			lines++;
			if(line[0]=='/' && line[1]=='/')
				continue;
			memset(str,0,sizeof(str));
			for(j=0,np=p=line;j<4 && p;j++){
				str[j]=p;
				p=strchr(p,',');
				if(p){ *p++=0; np=p; }
			}
			if(str[0]==NULL)
				continue;

			nameid=atoi(str[0]);
			if(nameid<=0)
				continue;
			if (j < 4)
			{	//Crash-fix on broken item lines. [Skotlex]
				ShowWarning("Reading %s: Insufficient fields for item with id %d, skipping.\n", filename[i], nameid);
				continue;
			}
			ln++;

			//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Job Upper,Gender,Loc,wLV,eLV,refineable,View
			id=itemdb_search(nameid);
			strncpy(id->name, str[1], ITEM_NAME_LENGTH-1);
			strncpy(id->jname, str[2], ITEM_NAME_LENGTH-1);
			id->type=atoi(str[3]);
			if (id->type == IT_DELAYCONSUME)
				id->type = IT_USABLE;
		}
		fclose(fp);
		if (ln > 0) {
			ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,filename[i]);
		}
		ln=0;	// reset to 0
	}
	return 0;
}

static int itemdb_read_sqldb(void) // sql item_db read, shortened version of map-server item_db read [Valaris]
{
	unsigned short nameid;
	struct item_data *id;
	char *item_db_name[] = { item_db_db, item_db2_db };
	long unsigned int ln = 0;
	int i;	

	// ----------

	for (i = 0; i < 2; i++) {
		sprintf(tmp_sql, "SELECT * FROM `%s`", item_db_name[i]);

		// Execute the query; if the query execution succeeded...
		if (mysql_query(&mysql_handle, tmp_sql) == 0) {
			sql_res = mysql_store_result(&mysql_handle);

			// If the storage of the query result succeeded...
			if (sql_res) {
				// Parse each row in the query result into sql_row
				while ((sql_row = mysql_fetch_row(sql_res)))
				{	/*Table structure is:
					00  id
					01  name_english
					02  name_japanese
					03  type
					...
					*/
					nameid = atoi(sql_row[0]);

					// If the identifier is not within the valid range, process the next row
					if (nameid == 0)
						continue;

					ln++;

					// ----------
         		id=itemdb_search(nameid);
					
					strncpy(id->name, sql_row[1], ITEM_NAME_LENGTH-1);
					strncpy(id->jname, sql_row[2], ITEM_NAME_LENGTH-1);

					id->type = atoi(sql_row[3]);
					if (id->type == IT_DELAYCONSUME)
						id->type = IT_USABLE;
				}
				ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, item_db_name[i]);
				ln = 0;
			} else {
				ShowSQL("DB error (%s) - %s\n",item_db_name[i], mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}

			// Free the query result
			mysql_free_result(sql_res);
		} else {
			ShowSQL("DB error (%s) - %s\n",item_db_name[i], mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
void do_final_itemdb(void)
{
	if(item_db){
		item_db->destroy(item_db,NULL);
		item_db=NULL;
	}
}
int do_init_itemdb(void)
{
	item_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_RELEASE_DATA,sizeof(int));

	if (db_use_sqldbs) // it db_use_sqldbs in inter config are yes, will read from item_db for char server display [Valaris]
		itemdb_read_sqldb();
	else
		itemdb_readdb();
	return 0;
}
