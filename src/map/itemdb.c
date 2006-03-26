// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "map.h"
#include "grfio.h"
#include "battle.h"
#include "itemdb.h"
#include "script.h"
#include "pc.h"

#define MAX_RANDITEM	10000
#define MAX_ITEMGROUP	32
// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

static struct dbt* item_db;

static struct random_item_data blue_box[MAX_RANDITEM], violet_box[MAX_RANDITEM], card_album[MAX_RANDITEM], gift_box[MAX_RANDITEM], scroll[MAX_RANDITEM], finding_ore[MAX_RANDITEM], cookie_bag[MAX_RANDITEM];
static int blue_box_count=0, violet_box_count=0, card_album_count=0, gift_box_count=0, scroll_count=0, finding_ore_count = 0, cookie_bag_count=0;
static int blue_box_default=0, violet_box_default=0, card_album_default=0, gift_box_default=0, scroll_default=0, finding_ore_default = 0, cookie_bag_default=0;

static struct item_group itemgroup_db[MAX_ITEMGROUP];

struct item_data *dummy_item=NULL; //This is the default dummy item used for non-existant items. [Skotlex]

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
// name = item alias, so we should find items aliases first. if not found then look for "jname" (full name)
int itemdb_searchname_sub(DBKey key,void *data,va_list ap)
{
	struct item_data *item=(struct item_data *)data,**dst;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
	if(item == dummy_item) return 0;
	if( strcmpi(item->name,str)==0 ) //by lupus
		*dst=item;
	return 0;
}

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
int itemdb_searchjname_sub(int key,void *data,va_list ap)
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
	item_db->foreach(item_db,itemdb_searchname_sub,str,&item);
	return item;
}

static int itemdb_searchname_array_sub(DBKey key,void * data,va_list ap)
{
	struct item_data *item=(struct item_data *)data;
	char *str;
	str=va_arg(ap,char *);
	if (item == dummy_item)
		return 1; //Invalid item.
	if(strstr(item->jname,str))
		return 0;
	if(strstr(item->name,str))
		return 0;
	return strcmpi(item->jname,str);
}

/*==========================================
 * Founds up to N matches. Returns number of matches [Skotlex]
 *------------------------------------------
 */
int itemdb_searchname_array(struct item_data** data, int size, const char *str)
{
	return item_db->getall(item_db,(void**)data,size,itemdb_searchname_array_sub,str);
}


/*==========================================
 * 箱系アイテム検索
 *------------------------------------------
 */
int itemdb_searchrandomid(int flags)
{
	int nameid=0,i,index,count;
	struct random_item_data *list=NULL;

	static struct {
		int nameid,count;
		struct random_item_data *list;
	} data[8];

	if (flags == 0) { //Initialize.
		memset(data, 0, sizeof(data));
		data[1].nameid = blue_box_default;
	  	data[1].count = blue_box_count;
	  	data[1].list = blue_box;
		data[2].nameid = violet_box_default;
	  	data[2].count = violet_box_count;
	  	data[2].list = violet_box;
		data[3].nameid = card_album_default;
	  	data[3].count = card_album_count;
	  	data[3].list = card_album;
		data[4].nameid = gift_box_default;
	  	data[4].count = gift_box_count;
	  	data[4].list = gift_box;
		data[5].nameid = scroll_default;
	  	data[5].count = scroll_count;
		data[5].list = scroll;
		data[6].nameid = finding_ore_default;
		data[6].count = finding_ore_count;
		data[6].list = finding_ore;
		data[7].nameid = cookie_bag_default;
		data[7].count = cookie_bag_count;
		data[7].list = cookie_bag;
	}
	if(flags>=1 && flags<=7){
		nameid=data[flags].nameid;
		count=data[flags].count;
		list=data[flags].list;

		if(count > 0) {
			for(i=0;i<1000;i++) {
				index = rand()%count;
				if(rand()%1000000 < list[index].per) {
					nameid = list[index].nameid;
					break;
				}
			}
		}
	}
	return nameid;
}

/*==========================================
 *
 *------------------------------------------
 */
int itemdb_group (int nameid)
{
	int i, j;
	for (i=0; i < MAX_ITEMGROUP; i++) {
		for (j=0; j < itemgroup_db[i].qty && itemgroup_db[i].id[j]; j++) {
			if (itemgroup_db[i].id[j] == nameid)
				return i;
		}
	}
	return -1;
}
int itemdb_searchrandomgroup (int groupid)
{
	if (groupid < 0 || groupid >= MAX_ITEMGROUP ||
		itemgroup_db[groupid].qty == 0 || itemgroup_db[groupid].id[0] == 0)
		return 0;
	
	return itemgroup_db[groupid].id[ rand()%itemgroup_db[groupid].qty ];
}

/*==========================================
 * DBの存在確認
 *------------------------------------------
 */
struct item_data* itemdb_exists(int nameid)
{
	struct item_data* id = idb_get(item_db,nameid);
	if (id == dummy_item) return NULL;
	return id;
}

/*==========================================
 * Converts the jobid from the format in itemdb 
 * to the format used by the map server. [Skotlex]
 *------------------------------------------
 */
static void itemdb_jobid2mapid(unsigned short *bclass, int jobmask)
{
	int i;
	bclass[0]= bclass[1]= bclass[2]= 0;
	//Base classes
	for (i = JOB_NOVICE; i <= JOB_THIEF; i++)
	{
		if (jobmask & 1<<i)
			bclass[0] |= 1<<(MAPID_NOVICE+i);
	}
	//2-1 classes
	if (jobmask & 1<<JOB_KNIGHT)
		bclass[1] |= 1<<MAPID_SWORDMAN;
	if (jobmask & 1<<JOB_PRIEST)
		bclass[1] |= 1<<MAPID_ACOLYTE;
	if (jobmask & 1<<JOB_WIZARD)
		bclass[1] |= 1<<MAPID_MAGE;
	if (jobmask & 1<<JOB_BLACKSMITH)
		bclass[1] |= 1<<MAPID_MERCHANT;
	if (jobmask & 1<<JOB_HUNTER)
		bclass[1] |= 1<<MAPID_ARCHER;
	if (jobmask & 1<<JOB_ASSASSIN)
		bclass[1] |= 1<<MAPID_THIEF;
	//2-2 classes
	if (jobmask & 1<<JOB_CRUSADER)
		bclass[2] |= 1<<MAPID_SWORDMAN;
	if (jobmask & 1<<JOB_MONK)
		bclass[2] |= 1<<MAPID_ACOLYTE;
	if (jobmask & 1<<JOB_SAGE)
		bclass[2] |= 1<<MAPID_MAGE;
	if (jobmask & 1<<JOB_ALCHEMIST)
		bclass[2] |= 1<<MAPID_MERCHANT;
	if (jobmask & 1<<JOB_BARD)
		bclass[2] |= 1<<MAPID_ARCHER;
	if (jobmask & 1<<JOB_DANCER)
		bclass[2] |= 1<<MAPID_ARCHER;
	if (jobmask & 1<<JOB_ROGUE)
		bclass[2] |= 1<<MAPID_THIEF;
	//Special classes that don't fit above.
	if (jobmask & 1<<JOB_SUPER_NOVICE)
		bclass[1] |= 1<<MAPID_NOVICE;
	if (jobmask & 1<<24) //Taekwon boy
		bclass[0] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<25) //Star Gladiator
		bclass[1] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<26) //Soul Linker
		bclass[2] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<27) //Gunslinger
		bclass[0] |= 1<<MAPID_GUNSLINGER;
	if (jobmask & 1<<28) //Ninja
		bclass[0] |= 1<<MAPID_NINJA;
}

static void create_dummy_data(void) {
	if (dummy_item)
		aFree(dummy_item);
	
	dummy_item=(struct item_data *)aCalloc(1,sizeof(struct item_data));
	dummy_item->nameid=500;
	dummy_item->weight=1;
	dummy_item->type=3; //Etc item
	strncpy(dummy_item->name,"UNKNOWN_ITEM",ITEM_NAME_LENGTH-1);
	strncpy(dummy_item->jname,"UNKNOWN_ITEM",ITEM_NAME_LENGTH-1);
	dummy_item->view_id = 512; //Use apple sprite.
}

static void* create_item_data(DBKey key, va_list args) {
	struct item_data *id;
	id=(struct item_data *)aCalloc(1,sizeof(struct item_data));
	id->nameid = key.i;
	id->weight=1;
	id->type=3; //Etc item
	return id;
}

/*==========================================
 * Loads (and creates if not found) an item from the db.
 *------------------------------------------
 */
struct item_data* itemdb_load(int nameid)
{
	return idb_ensure(item_db,nameid,create_item_data);
}

static void* return_dummy_data(DBKey key, va_list args) {
	if (battle_config.error_log)
		ShowWarning("itemdb_search: Item ID %d does not exists in the item_db. Using dummy data.\n", key.i);
	return dummy_item;
}

/*==========================================
 * Loads an item from the db. If not found, it will return the dummy item.
 *------------------------------------------
 */
struct item_data* itemdb_search(int nameid)
{
	return idb_ensure(item_db,nameid,return_dummy_data);
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
 * Trade Restriction functions [Skotlex]
 *------------------------------------------
 */
int itemdb_isdropable(int nameid, int gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&1) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cantrade(int nameid, int gmlv, int gmlv2)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&2) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_canpartnertrade(int nameid, int gmlv, int gmlv2)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&(2|4)) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_cansell(int nameid, int gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&8) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cancartstore(int nameid, int gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&16) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canstore(int nameid, int gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&32) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canguildstore(int nameid, int gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&64) || gmlv >= item->gm_lv_trade_override));
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
 * ランダムアイテム出現データの読み込み
 *------------------------------------------
 */
static int itemdb_read_randomitem(void)
{
	FILE *fp;
	char line[1024];
	int nameid,i,j;
	char *str[10],*p;

	const struct {
		char filename[64];
		struct random_item_data *pdata;
		int *pcount,*pdefault;
	} data[] = {
		{"item_bluebox.txt",		blue_box,	&blue_box_count, &blue_box_default		},
		{"item_violetbox.txt",	violet_box,	&violet_box_count, &violet_box_default	},
		{"item_cardalbum.txt",	card_album,	&card_album_count, &card_album_default	},
		{"item_giftbox.txt",		gift_box,	&gift_box_count, &gift_box_default	},
		{"item_scroll.txt",		scroll,		&scroll_count, &scroll_default	},
		{"item_findingore.txt",	finding_ore,&finding_ore_count, &finding_ore_default	},
		{"item_cookie_bag.txt",	cookie_bag,&cookie_bag_count, &cookie_bag_default	},
	};

	for(i=0;i<sizeof(data)/sizeof(data[0]);i++){
		struct random_item_data *pd=data[i].pdata;
		int *pc=data[i].pcount;
		int *pdefault=data[i].pdefault;
		char *fn=(char *) data[i].filename;

		*pdefault = 0;
		*pc = 0; //zero the count in case we are reloading. [Skotlex]
		
		sprintf(line, "%s/%s", db_path, fn);
		if( (fp=fopen(line,"r"))==NULL ){
			ShowError("can't read %s\n",line);
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

			if(*pc >= MAX_RANDITEM)
			{
				if (battle_config.error_log)
					ShowWarning("Reached limit of random items [%d] in file [%s]\n", MAX_RANDITEM, data[i].filename);
				break;
			}
		}
		fclose(fp);
		if (*pc > 0) {
			ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",*pc,fn);
		}
	}

	itemdb_searchrandomid(0); //Initialize values.
	return 0;
}

/*==========================================
 * アイテム使用可能フラグのオーバーライド
 *------------------------------------------
 */
static int itemdb_read_itemavail (void)
{
	FILE *fp;
	int nameid, j, k, ln = 0;
	char line[1024], *str[10], *p;
	struct item_data *id;

	sprintf(line, "%s/item_avail.txt", db_path);
	if ((fp = fopen(line,"r")) == NULL) {
		ShowError("can't read %s\n", line);
		return -1;
	}

	while (fgets(line, sizeof(line) - 1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		memset(str, 0, sizeof(str));
		for (j = 0, p = line; j < 2 && p; j++) {
			str[j] = p;
			p = strchr(p, ',');
			if(p) *p++ = 0;
		}

		if (j < 2 || str[0] == NULL ||
			(nameid = atoi(str[0])) < 0 || nameid >= 20000 || !(id = itemdb_exists(nameid)))
			continue;

		k = atoi(str[1]);
		if (k > 0) {
			id->flag.available = 1;
			id->view_id = k;
		} else
			id->flag.available = 0;
		ln++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, "item_avail.txt");

	return 0;
}

/*==========================================
 * read item group data
 *------------------------------------------
 */
static int itemdb_read_itemgroup(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int groupid,j,k;
	char *str[31],*p;

	sprintf(line, "%s/item_group_db.txt", db_path);
	if( (fp=fopen(line,"r"))==NULL ){
		ShowError("can't read %s\n", line);
		return -1;
	}

	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<31 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		groupid = atoi(str[0]);
		if (groupid < 0 || groupid >= MAX_ITEMGROUP)
			continue;

		for (j=1; j<=30; j++) {
			if (!str[j])
				break;
			k=atoi(str[j]);
			if (k < 0 || k >= 20000 || !itemdb_exists(k))
				continue;
			//printf ("%d[%d] = %d\n", groupid, j-1, k);
			itemgroup_db[groupid].id[j-1] = k;
			itemgroup_db[groupid].qty=j;
		}
		for (j=1; j<30; j++) { //Cleanup the contents. [Skotlex]
			if (itemgroup_db[groupid].id[j-1] == 0 &&
				itemgroup_db[groupid].id[j] != 0) 
			{
				itemgroup_db[groupid].id[j-1] = itemgroup_db[groupid].id[j];
				itemgroup_db[groupid].id[j] = 0;
				itemgroup_db[groupid].qty = j;
			}
		}
		ln++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"item_group_db.txt");
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
		char buf2[64]; //Why 64? What's this for, other than holding an item's name? [Skotlex]

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 ){

#ifdef ITEMDB_OVERRIDE_NAME_VERBOSE
			if( itemdb_exists(nameid) &&
				strncmp(itemdb_search(nameid)->jname,buf2,ITEM_NAME_LENGTH)!=0 ){
				ShowNotice("[override] %d %s => %s\n",nameid
					,itemdb_search(nameid)->jname,buf2);
			}
#endif

			strncpy(itemdb_search(nameid)->jname,buf2,ITEM_NAME_LENGTH-1);
		}

		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\idnum2itemdisplaynametable.txt");

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
		}
		
		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\num2cardillustnametable.txt");

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
	char *buf, *p;
	int s;

	buf = (char *)grfio_reads("data\\itemslottable.txt", &s);
	if (buf == NULL)
		return -1;
	buf[s] = 0;
	for (p = buf; p - buf < s; ) {
		int nameid, equip;
		struct item_data* item;
		sscanf(p, "%d#%d#", &nameid, &equip);
		item = itemdb_search(nameid);
		if (item && itemdb_isequip2(item))			
			item->equip = equip;
		p = strchr(p, 10);
		if(!p) break;
		p++;
		p=strchr(p, 10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\itemslottable.txt");

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static int itemdb_read_itemslotcounttable(void)
{
	char *buf, *p;
	int s;

	buf = (char *)grfio_reads("data\\itemslotcounttable.txt", &s);
	if (buf == NULL)
		return -1;
	buf[s] = 0;
	for (p = buf; p - buf < s;){
		int nameid, slot;
		sscanf(p, "%d#%d#", &nameid, &slot);
		if (slot > MAX_SLOTS)
		{
			ShowWarning("itemdb_read_itemslotcounttable: Item %d specifies %d slots, but the server only supports up to %d\n", nameid, slot, MAX_SLOTS);
			slot = MAX_SLOTS;
		}
		itemdb_slot(nameid) = slot;
		p = strchr(p,10);
		if(!p) break;
		p++;
		p = strchr(p,10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n", "data\\itemslotcounttable.txt");

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

	sprintf(line, "%s/item_noequip.txt", db_path);
	if( (fp=fopen(line,"r"))==NULL ){
		ShowError("can't read %s\n", line);
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
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"item_noequip.txt");
	}	
	return 0;
}

/*==========================================
 * Reads item trade restrictions [Skotlex]
 *------------------------------------------
 */
static int itemdb_read_itemtrade(void)
{
	FILE *fp;
	int nameid, j, flag, gmlv, ln = 0;
	char line[1024], *str[10], *p;
	struct item_data *id;

	sprintf(line, "%s/item_trade.txt", db_path);
	if ((fp = fopen(line,"r")) == NULL) {
		ShowError("can't read %s\n", line);
		return -1;
	}

	while (fgets(line, sizeof(line) - 1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		memset(str, 0, sizeof(str));
		for (j = 0, p = line; j < 3 && p; j++) {
			str[j] = p;
			p = strchr(p, ',');
			if(p) *p++ = 0;
		}

		if (j < 3 || str[0] == NULL ||
			(nameid = atoi(str[0])) < 0 || nameid >= 20000 || !(id = itemdb_exists(nameid)))
			continue;

		flag = atoi(str[1]);
		gmlv = atoi(str[2]);
		
		if (flag > 0 && flag < 128 && gmlv > 0) { //Check range
			id->flag.trade_restriction = flag;
			id->gm_lv_trade_override = gmlv;
			ln++;
		}
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, "item_trade.txt");

	return 0;
}

/*======================================
 * Applies gender restrictions according to settings. [Skotlex]
 *======================================
 */
static int itemdb_gendercheck(struct item_data *id)
{
	if (id->nameid == WEDDING_RING_M) //Grom Ring
		return 1;
	if (id->nameid == WEDDING_RING_F) //Bride Ring
		return 0;
	if (id->look == 13 && id->type == 4) //Musical instruments are always male-only
		return 1;
	if (id->look == 14 && id->type == 4) //Whips are always female-only
		return 0;

	return (battle_config.ignore_items_gender?2:id->sex);
}
#ifndef TXT_ONLY

/*======================================
* SQL
*===================================
*/
static int itemdb_read_sqldb(void)
{
	unsigned short nameid;
	struct item_data *id;
	char script[65535 + 2 + 1]; // Maximum length of MySQL TEXT type (65535) + 2 bytes for curly brackets + 1 byte for terminator
	char *item_db_name[] = { item_db_db, item_db2_db };
	long unsigned int ln = 0;
	int i;	

	// ----------

	for (i = 0; i < 2; i++) {
		sprintf(tmp_sql, "SELECT * FROM `%s`", item_db_name[i]);

		// Execute the query; if the query execution succeeded...
		if (mysql_query(&mmysql_handle, tmp_sql) == 0) {
			sql_res = mysql_store_result(&mmysql_handle);

			// If the storage of the query result succeeded...
			if (sql_res) {
				// Parse each row in the query result into sql_row
				while ((sql_row = mysql_fetch_row(sql_res)))
				{
					/* +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+
					   |  0 |            1 |             2 |    3 |         4 |          5 |      6 |      7 |       8 |     9 |    10 |         11 |          12 |            13 |              14 |           15 |   16        |         17 |   18 |     19 |
					   +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+
					   | id | name_english | name_japanese | type | price_buy | price_sell | weight | attack | defence | range | slots | equip_jobs | equip_upper | equip_genders | equip_locations | weapon_level | equip_level | refineable | view | script |
					   +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+ */

					nameid = atoi(sql_row[0]);

					// If the identifier is not within the valid range, process the next row
					if (nameid == 0 || nameid >= 20000)
						continue;

					ln++;

					// ----------
					id = itemdb_load(nameid);
					
					strncpy(id->name, sql_row[1], ITEM_NAME_LENGTH-1);
					strncpy(id->jname, sql_row[2], ITEM_NAME_LENGTH-1);

					id->type = atoi(sql_row[3]);
					if (id->type == 11)
					{	//Items that are consumed upon target confirmation
						//(yggdrasil leaf, spells & pet lures) [Skotlex]
						id->type = 2;
						id->flag.delay_consume=1;
					}

					// If price_buy is not NULL and price_sell is not NULL...
					if ((sql_row[4] != NULL) && (sql_row[5] != NULL)) {
						id->value_buy = atoi(sql_row[4]);
						id->value_sell = atoi(sql_row[5]);
					}
					// If price_buy is not NULL and price_sell is NULL...
					else if ((sql_row[4] != NULL) && (sql_row[5] == NULL)) {
						id->value_buy = atoi(sql_row[4]);
						id->value_sell = atoi(sql_row[4]) / 2;
					}
					// If price_buy is NULL and price_sell is not NULL...
					else if ((sql_row[4] == NULL) && (sql_row[5] != NULL)) {
						id->value_buy = atoi(sql_row[5]) * 2;
						id->value_sell = atoi(sql_row[5]);
					}
					// If price_buy is NULL and price_sell is NULL...
					if ((sql_row[4] == NULL) && (sql_row[5] == NULL)) {
						id->value_buy = 0;
						id->value_sell = 0;
					}

					id->weight	= atoi(sql_row[6]);
					id->atk		= (sql_row[7] != NULL) ? atoi(sql_row[7]) : 0;
					id->def		= (sql_row[8] != NULL) ? atoi(sql_row[8]) : 0;
					id->range	= (sql_row[9] != NULL) ? atoi(sql_row[9]) : 0;
					id->slot	= (sql_row[10] != NULL) ? atoi(sql_row[10]) : 0;
					if (id->slot > MAX_SLOTS)
					{
						ShowWarning("itemdb_read_sqldb: Item %d (%s) specifies %d slots, but the server only supports up to %d\n", nameid, id->jname, id->slot, MAX_SLOTS);
						id->slot = MAX_SLOTS;
					}
					itemdb_jobid2mapid(id->class_base, (sql_row[11] != NULL) ? atoi(sql_row[11]) : 0);
					id->class_upper= (sql_row[12] != NULL) ? atoi(sql_row[12]) : 0;
					id->sex		= (sql_row[13] != NULL) ? atoi(sql_row[13]) : 0;
					id->equip	= (sql_row[14] != NULL) ? atoi(sql_row[14]) : 0;
					id->wlv		= (sql_row[15] != NULL) ? atoi(sql_row[15]) : 0;
					id->elv		= (sql_row[16] != NULL)	? atoi(sql_row[16]) : 0;
					id->flag.no_refine = (sql_row[17] == NULL || atoi(sql_row[17]) == 1)?0:1;
					id->look	= (sql_row[18] != NULL) ? atoi(sql_row[18]) : 0;
					id->view_id	= 0;
					id->sex = itemdb_gendercheck(id); //Apply gender filtering.

					// ----------
					
					if (id->script)
						aFree(id->script);
					if (sql_row[19] != NULL) {
						if (sql_row[19][0] == '{')
							id->script = parse_script((unsigned char *) sql_row[19], 0);
						else {
							sprintf(script, "{%s}", sql_row[19]);
							id->script = parse_script((unsigned char *) script, 0);
						}
					} else id->script = NULL;

					// ----------

					id->flag.available	= 1;
					id->flag.value_notdc	= 0;
					id->flag.value_notoc	= 0;
				}

				ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, item_db_name[i]);
				ln = 0;
			} else {
				ShowSQL("DB error (%s) - %s\n",item_db_name[i], mysql_error(&mmysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}

			// Free the query result
			mysql_free_result(sql_res);
		} else {
			ShowSQL("DB error (%s) - %s\n",item_db_name[i], mysql_error(&mmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	}

	return 0;
}
#endif /* not TXT_ONLY */

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
		while(fgets(line,1020,fp)){
			lines++;
			if(line[0]=='/' && line[1]=='/')
				continue;
			memset(str,0,sizeof(str));
			for(j=0,np=p=line;j<19 && p;j++){
				str[j]=p;
				p=strchr(p,',');
				if(p){ *p++=0; np=p; }
			}
			if(str[0]==NULL)
				continue;

			nameid=atoi(str[0]);
			if(nameid<=0 || nameid>=20000)
				continue;
			if (j < 19)
			{	//Crash-fix on broken item lines. [Skotlex]
				ShowWarning("Reading %s: Insufficient fields for item with id %d, skipping.\n", filename[i], nameid);
				continue;
			}
			ln++;

			//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Job Upper,Gender,Loc,wLV,eLV,refineable,View
			id=itemdb_load(nameid);
			strncpy(id->name, str[1], ITEM_NAME_LENGTH-1);
			strncpy(id->jname, str[2], ITEM_NAME_LENGTH-1);
			id->type=atoi(str[3]);
			if (id->type == 11)
			{	//Items that are consumed upon target confirmation
				//(yggdrasil leaf, spells & pet lures) [Skotlex]
				id->type = 2;
				id->flag.delay_consume=1;
			}

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
					ShowWarning ("Item %s [%d] buying:%d < selling:%d\n",
						id->name, id->nameid, id->value_buy*75/100, id->value_sell*124/100);
				}
			}
			id->weight=atoi(str[6]);
			id->atk=atoi(str[7]);
			id->def=atoi(str[8]);
			id->range=atoi(str[9]);
			id->slot=atoi(str[10]);
			if (id->slot > MAX_SLOTS)
			{
				ShowWarning("itemdb_readdb: Item %d (%s) specifies %d slots, but the server only supports up to %d\n", nameid, id->jname, id->slot, MAX_SLOTS);
				id->slot = MAX_SLOTS;
			}
			itemdb_jobid2mapid(id->class_base, atoi(str[11]));
			id->class_upper = atoi(str[12]);
			id->sex	= atoi(str[13]);
			if(id->equip != atoi(str[14])){
				id->equip=atoi(str[14]);
			}
			id->wlv=atoi(str[15]);
			id->elv=atoi(str[16]);
			id->flag.no_refine = atoi(str[17])?0:1;	//If the refine column is 1, no_refine is 0
			id->look=atoi(str[18]);
			id->flag.available=1;
			id->flag.value_notdc=0;
			id->flag.value_notoc=0;
			id->view_id=0;
			id->sex = itemdb_gendercheck(id); //Apply gender filtering.

			if (id->script) {
				aFree(id->script);
				id->script=NULL;
			}
			if((p=strchr(np,'{'))==NULL)
				continue;
			id->script = parse_script((unsigned char *) p,lines);
		}
		fclose(fp);
		if (ln > 0) {
			ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,filename[i]);
		}
		ln=0;	// reset to 0
	}
	return 0;
}

/*====================================
 * Removed item_value_db, don't re-add
 *------------------------------------
 */
static void itemdb_read(void)
{
#ifndef TXT_ONLY
	if (db_use_sqldbs)
		itemdb_read_sqldb();
	else
#endif
		itemdb_readdb();

	itemdb_read_itemgroup();
	itemdb_read_randomitem();
	itemdb_read_itemavail();
	itemdb_read_noequip();
	itemdb_read_itemtrade();
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
 * Initialize / Finalize
 *------------------------------------------
 */
static int itemdb_final_sub (DBKey key,void *data,va_list ap)
{
	int flag;
	struct item_data *id = (struct item_data *)data;

	flag = va_arg(ap, int);
	if (id->script)
	{
		aFree(id->script);
		id->script = NULL;
	}
	// Whether to clear the item data (exception: do not clear the dummy item data
	if (flag && id != dummy_item) 
		aFree(id);

	return 0;
}

void itemdb_reload(void)
{
	// free up all item scripts first
	item_db->foreach(item_db, itemdb_final_sub, 0);
	itemdb_read();
}

void do_final_itemdb(void)
{
	item_db->destroy(item_db, itemdb_final_sub, 1);
	if (dummy_item) {
		if (dummy_item->script)
			aFree(dummy_item->script);
		aFree(dummy_item);
		dummy_item = NULL;
	}
}

int do_init_itemdb(void)
{
	item_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_BASE,sizeof(int)); 
	create_dummy_data(); //Dummy data item.
	itemdb_read();

	return 0;
}
