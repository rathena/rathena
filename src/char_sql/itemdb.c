// $Id: itemdb.c,v 1.1.1.1 2004/09/10 17:44:48 MagicalTux Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "itemdb.h"
#include "db.h"
#include "inter.h"
#include "char.h"
#include "utils.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define MAX_RANDITEM	2000

// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

char item_db_db[256]="item_db"; // added to specify item_db sql table [Valaris]

static struct dbt* item_db;

/*==========================================
 * DBの検索
 *------------------------------------------
 */
struct item_data* itemdb_search(int nameid)
{
	struct item_data *id;

	id=numdb_search(item_db,nameid);
	if(id) return id;

	CREATE(id, struct item_data, 1);

	numdb_insert(item_db,nameid,id);


	if(nameid>500 && nameid<600)
		id->type=0;   //heal item
	else if(nameid>600 && nameid<700)
		id->type=2;   //use item
	else if((nameid>700 && nameid<1100) ||
			(nameid>7000 && nameid<8000))
		id->type=3;   //correction
	else if(nameid>=1750 && nameid<1771)
		id->type=10;  //arrow
	else if(nameid>1100 && nameid<2000)
		id->type=4;   //weapon
	else if((nameid>2100 && nameid<3000) ||
			(nameid>5000 && nameid<6000))
		id->type=5;   //armor
	else if(nameid>4000 && nameid<5000)
		id->type=6;   //card
	else if(nameid>9000 && nameid<10000)
		id->type=7;   //egg
	else if(nameid>10000)
		id->type=8;   //petequip

	return id;
}

/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	if(type==0 || type==2 || type==3 || type==6 || type==10)
		return 0;
	return 1;
}
/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip2(struct item_data *data)
{
	if(data) {
		int type=data->type;
		if(type==0 || type==2 || type==3 || type==6 || type==10)
			return 0;
		else
			return 1;
	}
	return 0;
}



/*==========================================
 * アイテムデータベースの読み込み
 *------------------------------------------
 */
static int itemdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j;
	char *str[32],*p,*np;
	struct item_data *id;

	fp=fopen("db/item_db.txt","r");
	if(fp==NULL){
		printf("can't read db/item_db.txt\n");
		exit(1);
	}
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,np=p=line;j<17 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p){ *p++=0; np=p; }
		}
		if(str[0]==NULL)
			continue;
		
		nameid=atoi(str[0]);
		if(nameid<=0 || nameid>=20000)
			continue;
		ln++;

		//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Gender,Loc,wLV,eLV,View
		id=itemdb_search(nameid);
		memcpy(id->name,str[1],24);
		memcpy(id->jname,str[2],24);
		id->type=atoi(str[3]);

	}
	fclose(fp);
	printf("read db/item_db.txt done (count=%d)\n",ln);
	return 0;
}

static int itemdb_read_sqldb(void) // sql item_db read, shortened version of map-server item_db read [Valaris]
{
	unsigned int nameid; 	// Type should be "unsigned short int", but currently isn't for compatibility with numdb_insert()
	struct item_data *id;

	// ----------

	// Output query to retrieve all rows from the item database table
	sprintf(tmp_sql, "SELECT * FROM `%s`", item_db_db);

	// Execute the query; if the query execution fails, output an error
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("Database server error (executing query for %s): %s\n", item_db_db, mysql_error(&mysql_handle));
	}

	// Store the query result
	sql_res = mysql_store_result(&mysql_handle);

	// If the storage of the query result succeeded
	if (sql_res) {
		// Parse each row in the query result into sql_row
		while ((sql_row = mysql_fetch_row(sql_res))) {
			nameid = atoi(sql_row[0]);

			// If the identifier is not within the valid range, process the next row
			if (nameid == 0 || nameid >= 20000)	{	// Should ">= 20000" be "> 20000"?
				continue;
			}

			// ----------

			// Insert a new row into the item database
/*
			id = calloc(sizeof(struct item_data), 1);

			if (id == NULL) {
				printf("out of memory : itemdb_read_sqldb\n");
				exit(1);
			}

			memset(id, 0, sizeof(struct item_data));
			numdb_insert(item_db, nameid, id);

			// ----------
*/
            id=itemdb_search(nameid);
             
			memcpy(id->name, sql_row[1], 24);
			memcpy(id->jname, sql_row[2], 24);

			id->type = atoi(sql_row[3]);
		}

		// If the retrieval failed, output an error
		if (mysql_errno(&mysql_handle)) {
			printf("Database server error (retrieving rows from %s): %s\n", item_db_db, mysql_error(&mysql_handle));
		}

		printf("read %s done (count = %lu)\n", item_db_db, (unsigned long) mysql_num_rows(sql_res));

		// Free the query result
		mysql_free_result(sql_res);
	} else {
		printf("MySQL error (storing query result for %s): %s\n", item_db_db, mysql_error(&mysql_handle));
	}

	return 0;
}

static int itemdb_final(void *key,void *data,va_list ap)
{
	struct item_data *id;

	id=data;
	if(id->use_script)
		free(id->use_script);
	if(id->equip_script)
		free(id->equip_script);
	free(id);

	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
void do_final_itemdb(void)
{
	if(item_db){
		numdb_final(item_db,itemdb_final);
		item_db=NULL;
	}
}
int do_init_itemdb(void)
{
	item_db = numdb_init();

	if (db_use_sqldbs) // it db_use_sqldbs in inter config are yes, will read from item_db for char server display [Valaris]
		itemdb_read_sqldb();
	else
	itemdb_readdb();
	return 0;
}

