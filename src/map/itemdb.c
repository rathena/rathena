// $Id: itemdb.c,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "grfio.h"
#include "nullpo.h"
#include "malloc.h"
#include "map.h"
#include "battle.h"
#include "itemdb.h"
#include "script.h"
#include "pc.h"
#include "showmsg.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define MAX_RANDITEM	2000

// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

static struct dbt* item_db;

static struct random_item_data blue_box[MAX_RANDITEM], violet_box[MAX_RANDITEM], card_album[MAX_RANDITEM], gift_box[MAX_RANDITEM], scroll[MAX_RANDITEM], finding_ore[MAX_RANDITEM];
static int blue_box_count=0, violet_box_count=0, card_album_count=0, gift_box_count=0, scroll_count=0, finding_ore_count = 0;
static int blue_box_default=0, violet_box_default=0, card_album_default=0, gift_box_default=0, scroll_default=0, finding_ore_default = 0;

// Function declarations

static void itemdb_read(void);
static int itemdb_readdb(void);
#ifndef TXT_ONLY
static int itemdb_read_sqldb(void);
#endif /* not TXT_ONLY */
static int itemdb_read_randomitem();
static int itemdb_read_itemavail(void);
static int itemdb_read_itemnametable(void);
static int itemdb_read_itemslottable(void);
static int itemdb_read_itemslotcounttable(void);
static int itemdb_read_cardillustnametable(void);
static int itemdb_read_noequip(void);
static int itemdb_read_norefine(void);
void itemdb_reload(void);

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
// name = item alias, so we should find items aliases first. if not found then look for "jname" (full name)
int itemdb_searchname_sub(void *key,void *data,va_list ap)
{
	struct item_data *item=(struct item_data *)data,**dst;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
//	if( strcmpi(item->name,str)==0 || strcmp(item->jname,str)==0 ||
//		memcmp(item->name,str,24)==0 || memcmp(item->jname,str,24)==0 )
	if( strcmpi(item->name,str)==0 ) //by lupus
		*dst=item;
	return 0;
}

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
int itemdb_searchjname_sub(void *key,void *data,va_list ap)
{
	struct item_data *item=(struct item_data *)data,**dst;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
	if( strcmpi(item->jname,str)==0 )
		*dst=item;
	return 0;
}
/*==========================================
 * 名前で検索
 *------------------------------------------
 */
struct item_data* itemdb_searchname(const char *str)
{
	struct item_data *item=NULL;
	numdb_foreach(item_db,itemdb_searchname_sub,str,&item);
	return item;
}

/*==========================================
 * 箱系アイテム検索
 *------------------------------------------
 */
int itemdb_searchrandomid(int flags)
{
	int nameid=0,i,index,count;
	struct random_item_data *list=NULL;

	struct {
		int nameid,count;
		struct random_item_data *list;
	} data[7];

	// for BCC32 compile error
	data[0].nameid = 0;						data[0].count = 0; 					data[0].list = NULL;
	data[1].nameid = blue_box_default;		data[1].count = blue_box_count;		data[1].list = blue_box;
	data[2].nameid = violet_box_default;	data[2].count = violet_box_count;	data[2].list = violet_box;
	data[3].nameid = card_album_default;	data[3].count = card_album_count;	data[3].list = card_album;
	data[4].nameid = gift_box_default;		data[4].count = gift_box_count;		data[4].list = gift_box;
	data[5].nameid = scroll_default;		data[5].count = scroll_count;		data[5].list = scroll;
	data[6].nameid = finding_ore_default;	data[6].count = finding_ore_count;	data[6].list = finding_ore;

	if(flags>=1 && flags<=6){
		nameid=data[flags].nameid;
		count=data[flags].count;
		list=data[flags].list;

		if(count > 0) {
			for(i=0;i<1000;i++) {
				index = rand()%count;
				if(	rand()%1000000 < list[index].per) {
					nameid = list[index].nameid;
					break;
				}
			}
		}
	}
	return nameid;
}

/*==========================================
 * DBの存在確認
 *------------------------------------------
 */
struct item_data* itemdb_exists(int nameid)
{
	return (struct item_data *) numdb_search(item_db,nameid);
}
/*==========================================
 * DBの検索
 *------------------------------------------
 */
struct item_data* itemdb_search(int nameid)
{
	struct item_data *id;

	id=(struct item_data *) numdb_search(item_db,nameid);
	if(id) return id;

	id=(struct item_data *)aCalloc(1,sizeof(struct item_data));
	numdb_insert(item_db,nameid,id);

	id->nameid=nameid;
	id->value_buy=10;
	id->value_sell=id->value_buy/2;
	id->weight=10;
	id->sex=2;
	id->elv=0;
	id->class_=0xffffffff;
	id->flag.available=0;
	id->flag.value_notdc=0;  //一応・・・
	id->flag.value_notoc=0;
	id->flag.no_equip=0;
	id->view_id=0;

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
 *
 *------------------------------------------
 */
int itemdb_isequip3(int nameid)
{
	int type=itemdb_type(nameid);
	if(type==4 || type==5 || type == 8)
		return 1;
	return 0;
}

/*==========================================
 * 捨てられるアイテムは1、そうでないアイテムは0
 *------------------------------------------
 */
int itemdb_isdropable(int nameid)
{
	//結婚指輪は捨てられない
	switch(nameid){
	case 2634: //結婚指輪
	case 2635: //結婚指輪
		return 0;
	}

	return 1;
}

/*====================================
 * Removed item_value_db, don't re-add
 *------------------------------------
 */
static void itemdb_read(void)
{
	#ifndef TXT_ONLY
		if (db_use_sqldbs)
		{
			itemdb_read_sqldb();
		}
		else
		{
			itemdb_readdb();
		}
	/* not TXT_ONLY */
	#else
		itemdb_readdb();
	#endif /* TXT_ONLY */

	itemdb_read_randomitem();
	itemdb_read_itemavail();
	itemdb_read_noequip();
	itemdb_read_norefine();
	if (battle_config.cardillust_read_grffile)
		itemdb_read_cardillustnametable();
	if (battle_config.item_equip_override_grffile)
		itemdb_read_itemslottable();
	if (battle_config.item_slots_override_grffile)
		itemdb_read_itemslotcounttable();
	if (battle_config.item_name_override_grffile)
		itemdb_read_itemnametable();
}

/*==========================================
 * アイテムデータベースの読み込み
 *------------------------------------------
 */
static int itemdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0,lines=0;
	int nameid,j;
	char *str[32],*p,*np;
	struct item_data *id;
	int i=0;
	char *filename[]={ "db/item_db.txt","db/item_db2.txt" };

	for(i=0;i<2;i++){

		fp=fopen(filename[i],"r");
		if(fp==NULL){
			if(i>0)
				continue;
			printf("can't read %s\n",filename[i]);
			exit(1);
		}

		lines=0;
		while(fgets(line,1020,fp)){
			lines++;
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

			{
				int buy = atoi(str[4]), sell = atoi(str[5]);
				// if buying price > selling price * 2 consider it valid and don't change it [celest]
				if (buy && sell && buy > sell*2){
					id->value_buy = buy;
					id->value_sell = sell;
				} else {
					// buy≠sell*2 は item_value_db.txt で指定してください。
					if (sell) {		// sell値を優先とする
						id->value_buy = sell*2;
						id->value_sell = sell;
					} else {
						id->value_buy = buy;
						id->value_sell = buy/2;
					}
				}
				// check for bad prices that can possibly cause exploits
				if (id->value_buy*75/100 < id->value_sell*124/100) {
					sprintf (tmp_output, "Item %s [%d] buying:%d < selling:%d\n",
						id->name, id->nameid, id->value_buy*75/100, id->value_sell*124/100);
					ShowWarning (tmp_output);
				}
			}
			id->weight=atoi(str[6]);
			id->atk=atoi(str[7]);
			id->def=atoi(str[8]);
			id->range=atoi(str[9]);
			id->slot=atoi(str[10]);
			id->class_=atoi(str[11]);
			id->sex=atoi(str[12]);
			if(id->equip != atoi(str[13])){
				id->equip=atoi(str[13]);
			}
			id->wlv=atoi(str[14]);
			id->elv=atoi(str[15]);
			id->look=atoi(str[16]);
			id->flag.available=1;
			id->flag.value_notdc=0;
			id->flag.value_notoc=0;
			id->view_id=0;

			id->use_script=NULL;
			id->equip_script=NULL;

			if((p=strchr(np,'{'))==NULL)
				continue;
			id->use_script = parse_script((unsigned char *) p,lines);
			if((p=strchr(p+1,'{'))==NULL)
				continue;
			id->equip_script = parse_script((unsigned char *) p,lines);
		}
		fclose(fp);
		if (ln > 0) {
			sprintf(tmp_output,"Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,filename[i]);
			ShowStatus(tmp_output);
		}
		ln=0;	// reset to 0
	}
	return 0;
}

// Removed item_value_db, don't re-add!

/*==========================================
 * ランダムアイテム出現データの読み込み
 *------------------------------------------
 */
static int itemdb_read_randomitem()
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,i,j;
	char *str[10],*p;

	const struct {
		char filename[64];
		struct random_item_data *pdata;
		int *pcount,*pdefault;
	} data[] = {
		{"db/item_bluebox.txt",		blue_box,	&blue_box_count, &blue_box_default		},
		{"db/item_violetbox.txt",	violet_box,	&violet_box_count, &violet_box_default	},
		{"db/item_cardalbum.txt",	card_album,	&card_album_count, &card_album_default	},
		{"db/item_giftbox.txt",		gift_box,	&gift_box_count, &gift_box_default	},
		{"db/item_scroll.txt",		scroll,		&scroll_count, &scroll_default	},
		{"db/item_findingore.txt",	finding_ore,&finding_ore_count, &finding_ore_default	},
	};

	for(i=0;i<sizeof(data)/sizeof(data[0]);i++){
		struct random_item_data *pd=data[i].pdata;
		int *pc=data[i].pcount;
		int *pdefault=data[i].pdefault;
		char *fn=(char *) data[i].filename;

		*pdefault = 0;
		if( (fp=fopen(fn,"r"))==NULL ){
			printf("can't read %s\n",fn);
			continue;
		}

		while(fgets(line,1020,fp)){
			if(line[0]=='/' && line[1]=='/')
				continue;
			memset(str,0,sizeof(str));
			for(j=0,p=line;j<3 && p;j++){
				str[j]=p;
				p=strchr(p,',');
				if(p) *p++=0;
			}

			if(str[0]==NULL)
				continue;

			nameid=atoi(str[0]);
			if(nameid<0 || nameid>=20000)
				continue;
			if(nameid == 0) {
				if(str[2])
					*pdefault = atoi(str[2]);
				continue;
			}

			if(str[2]){
				pd[ *pc   ].nameid = nameid;
				pd[(*pc)++].per = atoi(str[2]);
			}

			if(ln >= MAX_RANDITEM)
				break;
			ln++;
		}
		fclose(fp);
		if (*pc > 0) {
			sprintf(tmp_output,"Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",*pc,fn);
			ShowStatus(tmp_output);
		}
	}

	return 0;
}
/*==========================================
 * アイテム使用可能フラグのオーバーライド
 *------------------------------------------
 */
static int itemdb_read_itemavail(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j,k;
	char *str[10],*p;

	if( (fp=fopen("db/item_avail.txt","r"))==NULL ){
		printf("can't read db/item_avail.txt\n");
		return -1;
	}

	while(fgets(line,1020,fp)){
		struct item_data *id;
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}

		if(str[0]==NULL)
			continue;

		nameid=atoi(str[0]);
		if(nameid<0 || nameid>=20000 || !(id=itemdb_exists(nameid)) )
			continue;
		k=atoi(str[1]);
		if(k > 0) {
			id->flag.available = 1;
			id->view_id = k;
		}
		else
			id->flag.available = 0;
		ln++;
	}
	fclose(fp);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"db/item_avail.txt");
	ShowStatus(tmp_output);
	return 0;
}

/*==========================================
 * アイテムの名前テーブルを読み込む
 *------------------------------------------
 */
static int itemdb_read_itemnametable(void)
{
	char *buf,*p;
	int s;

	buf=(char *) grfio_reads("data\\idnum2itemdisplaynametable.txt",&s);

	if(buf==NULL)
		return -1;

	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid;
		char buf2[64];

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 ){

#ifdef ITEMDB_OVERRIDE_NAME_VERBOSE
			if( itemdb_exists(nameid) &&
				strncmp(itemdb_search(nameid)->jname,buf2,24)!=0 ){
				printf("[override] %d %s => %s\n",nameid
					,itemdb_search(nameid)->jname,buf2);
			}
#endif

			memcpy(itemdb_search(nameid)->jname,buf2,24);
		}

		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\idnum2itemdisplaynametable.txt");
	ShowStatus(tmp_output);

	return 0;
}

/*==========================================
 * カードイラストのリソース名前テーブルを読み込む
 *------------------------------------------
 */
static int itemdb_read_cardillustnametable(void)
{
	char *buf,*p;
	int s;

	buf=(char *) grfio_reads("data\\num2cardillustnametable.txt",&s);

	if(buf==NULL)
		return -1;

	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid;
		char buf2[64];

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 ){
			strcat(buf2,".bmp");
			memcpy(itemdb_search(nameid)->cardillustname,buf2,64);
//			printf("%d %s\n",nameid,itemdb_search(nameid)->cardillustname);
		}
		
		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\num2cardillustnametable.txt");
	ShowStatus(tmp_output);

	return 0;
}

//
// 初期化
//
/*==========================================
 *
 *------------------------------------------
 */
static int itemdb_read_itemslottable(void)
{
	char *buf,*p;
	int s;

	buf=(char *) grfio_read("data\\itemslottable.txt");
	if(buf==NULL)
		return -1;
	s=grfio_size("data\\itemslottable.txt");
	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid,equip;
		struct item_data* item;
		sscanf(p,"%d#%d#",&nameid,&equip);
		item = itemdb_search(nameid);
		if (item && itemdb_isequip2(item))			
			item->equip=equip;
		p=strchr(p,10);
		if(!p) break;
		p++;
		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\itemslottable.txt");
	ShowStatus(tmp_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static int itemdb_read_itemslotcounttable(void)
{
	char *buf,*p;
	int s;

	buf=(char *) grfio_read("data\\itemslotcounttable.txt");
	if(buf==NULL)
		return -1;
	s=grfio_size("data\\itemslotcounttable.txt");
	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid,slot;
		sscanf(p,"%d#%d#",&nameid,&slot);
		itemdb_search(nameid)->slot=slot;
		p=strchr(p,10);
		if(!p) break;
		p++;
		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\itemslotcounttable.txt");
	ShowStatus(tmp_output);

	return 0;
}

/*==========================================
 * 装備制限ファイル読み出し
 *------------------------------------------
 */
static int itemdb_read_noequip(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j;
	char *str[32],*p;
	struct item_data *id;

	if( (fp=fopen("db/item_noequip.txt","r"))==NULL ){
		printf("can't read db/item_noequip.txt\n");
		return -1;
	}
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		nameid=atoi(str[0]);
		if(nameid<=0 || nameid>=20000 || !(id=itemdb_exists(nameid)))
			continue;

		id->flag.no_equip=atoi(str[1]);

		ln++;

	}
	fclose(fp);
	if (ln > 0) {
		sprintf(tmp_output,"Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"db/item_noequip.txt");
		ShowStatus(tmp_output);
	}	
	return 0;
}

/*================================================
 * Whether the item can be refined or not [Celest]
 *------------------------------------------------
 */
static int itemdb_read_norefine(void)
{
	int i, nameid;
	struct item_data *id;
	// To-do: let it read from a text file later
	int cant_refine[] = {
		1243, 1530, 2110, 2112, 2201, 2202, 2203, 2204, 2205, 2210,
		2212, 2218, 2219, 2237, 2238, 2239, 2240, 2241, 2242, 2243,
		2250, 2253, 2260, 2262, 2263, 2264, 2265, 2266, 2267, 2268,
		2269, 2270, 2271, 2276, 2278, 2279, 2281, 2282, 2286, 2288,
		2289, 2290, 2291, 2292, 2293, 2295, 2296, 2297, 2298, 2352,
		2410, 2413, 2414, 2509, 2510, 2601, 2602, 2603, 2604, 2605,
		2607, 2608, 2609, 2610, 2611, 2612, 2613, 2614, 2615, 2616,
		2617, 2618, 2619, 2620, 2621, 2622, 2623, 2624, 2625, 2626,
		2627, 2628, 2629, 2630, 2631, 2634, 2635, 2636, 2637, 2638,
		2639, 2640, 5004, 5005, 5006, 5008, 5014, 5015, 5037, 5039,
		5040, 5043, 5046, 5049, 5050, 5051, 5053, 5054, 5055, 5058,
		5068, 5074, 5085, 5086, 5087, 5088, 5089, 5090, 5096, 5098, 0
	};

	for (i=0; i < (int)(sizeof(cant_refine) / sizeof(cant_refine[0])); i++) {
		nameid = cant_refine[i];
		if(nameid<=0 || nameid>=20000 || !(id=itemdb_exists(nameid)))
			continue;
		id->flag.no_refine = 1;
	}

	return 1;
}

#ifndef TXT_ONLY

/*======================================
* SQL
*===================================
*/
static int itemdb_read_sqldb(void)
{
	unsigned short	nameid;
	struct			item_data *id;
	char			script[65535 + 2 + 1]; // Maximum length of MySQL TEXT type (65535) + 2 bytes for curly brackets + 1 byte for terminator

	// ----------

	sprintf(tmp_sql, "SELECT * FROM `%s`", item_db_db);

	// Execute the query; if the query execution succeeded...
	if (mysql_query(&mmysql_handle, tmp_sql) == 0)
	{
		sql_res = mysql_store_result(&mmysql_handle);

		// If the storage of the query result succeeded...
		if (sql_res)
		{
			// Parse each row in the query result into sql_row
			while ((sql_row = mysql_fetch_row(sql_res)))
			{
				/* +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+
				   |  0 |            1 |             2 |    3 |         4 |          5 |      6 |      7 |       8 |     9 |    10 |         11 |            12 |              13 |           14 |          15 |   16 |         17 |           18 |
				   +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+
				   | id | name_english | name_japanese | type | price_buy | price_sell | weight | attack | defence | range | slots | equip_jobs | equip_genders | equip_locations | weapon_level | equip_level | view | script_use | script_equip |
				   +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+ */

				nameid = atoi(sql_row[0]);

				// If the identifier is not within the valid range, process the next row
				if (nameid == 0 || nameid >= 20000)
				{
					continue;
				}

				// Insert a new row into the item database

				/*id = aCalloc(sizeof(struct item_data), 1);

				if (id == NULL)
				{
					printf("out of memory : itemdb_read_sqldb\n");
					exit(1);
				}

				memset(id, 0, sizeof(struct item_data));
				numdb_insert(item_db, (int) nameid, id);*/

				// ----------
				id=itemdb_search(nameid);
				
				memcpy(id->name, sql_row[1], 25);
				memcpy(id->jname, sql_row[2], 25);

				id->type = atoi(sql_row[3]);

				// If price_buy is not NULL and price_sell is not NULL...
				if ((sql_row[4] != NULL) && (sql_row[5] != NULL))
				{
					id->value_buy = atoi(sql_row[4]);
					id->value_sell = atoi(sql_row[5]);
				}
				// If price_buy is not NULL and price_sell is NULL...
				else if ((sql_row[4] != NULL) && (sql_row[5] == NULL))
				{
					id->value_buy = atoi(sql_row[4]);
					id->value_sell = atoi(sql_row[4]) / 2;
				}
				// If price_buy is NULL and price_sell is not NULL...
				else if ((sql_row[4] == NULL) && (sql_row[5] != NULL))
				{
					id->value_buy = atoi(sql_row[5]) * 2;
					id->value_sell = atoi(sql_row[5]);
				}
				// If price_buy is NULL and price_sell is NULL...
				if ((sql_row[4] == NULL) && (sql_row[5] == NULL))
				{
					id->value_buy = 0;
					id->value_sell = 0;
				}

				id->weight	= atoi(sql_row[6]);

				id->atk		= (sql_row[7] != NULL)		? atoi(sql_row[7])	: 0;
				id->def		= (sql_row[8] != NULL)		? atoi(sql_row[8])	: 0;
				id->range	= (sql_row[9] != NULL)		? atoi(sql_row[9])	: 0;
				id->slot	= (sql_row[10] != NULL)		? atoi(sql_row[10])	: 0;
				id->class_	= (sql_row[11] != NULL)		? atoi(sql_row[11])	: 0;
				id->sex		= (sql_row[12] != NULL)		? atoi(sql_row[12])	: 0;
				id->equip	= (sql_row[13] != NULL)		? atoi(sql_row[13])	: 0;
				id->wlv		= (sql_row[14] != NULL)		? atoi(sql_row[14])	: 0;
				id->elv		= (sql_row[15] != NULL)		? atoi(sql_row[15])	: 0;
				id->look	= (sql_row[16] != NULL)		? atoi(sql_row[16])	: 0;

				id->view_id	= 0;

				// ----------

				if (sql_row[17] != NULL)
				{
                                        if (sql_row[17][0] == '{')
					  id->use_script = parse_script((unsigned char *) sql_row[17], 0);
                                        else {
					  sprintf(script, "{%s}", sql_row[17]);
					  id->use_script = parse_script((unsigned char *) script, 0);
                                        }
				}
				else
				{
					id->use_script = NULL;
				}

				if (sql_row[18] != NULL)
				{
                                        if (sql_row[18][0] == '{')
					  id->equip_script = parse_script((unsigned char *) sql_row[18], 0);
                                        else {
					  sprintf(script, "{%s}", sql_row[18]);
					  id->equip_script = parse_script((unsigned char *) script, 0);
                                        }
				}
				else
				{
					id->equip_script = NULL;
				}

				// ----------

				id->flag.available		= 1;
				id->flag.value_notdc	= 0;
				id->flag.value_notoc	= 0;
			}

			// If the retrieval failed, output an error
			if (mysql_errno(&mmysql_handle))
			{
				printf("Database server error (retrieving rows from %s): %s\n", item_db_db, mysql_error(&mmysql_handle));
			}
			sprintf(tmp_output,"Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",(unsigned long) mysql_num_rows(sql_res),item_db_db);
			ShowStatus(tmp_output);
		}
		else
		{
			printf("MySQL error (storing query result for %s): %s\n", item_db_db, mysql_error(&mmysql_handle));
		}

		// Free the query result
		mysql_free_result(sql_res);
	}
	else
	{
		printf("Database server error (executing query for %s): %s\n", item_db_db, mysql_error(&mmysql_handle));
	}

	return 0;
}

#endif /* not TXT_ONLY */
/*==========================================
 *
 *------------------------------------------
 */
static int itemdb_final(void *key,void *data,va_list ap)
{
	struct item_data *id;

	nullpo_retr(0, id= (struct item_data *) data);

	if(id->use_script)
		aFree(id->use_script);
	if(id->equip_script)
		aFree(id->equip_script);
	aFree(id);

	return 0;
}

void itemdb_reload(void)
{
	numdb_final(item_db,itemdb_final);
	do_init_itemdb();
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

/*
static FILE *dfp;
static int itemdebug(void *key,void *data,va_list ap){
//	struct item_data *id=(struct item_data *)data;
	fprintf(dfp,"%6d",(int)key);
	return 0;
}
void itemdebugtxt()
{
	dfp=fopen("itemdebug.txt","wt");
	numdb_foreach(item_db,itemdebug);
	fclose(dfp);
}
*/
/*==========================================
 *
 *------------------------------------------
 */
int do_init_itemdb(void)
{
	item_db = numdb_init();

	itemdb_read();

	return 0;
}
