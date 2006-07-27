// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/grfio.h"
#include "../common/strlib.h"
#include "map.h"
#include "battle.h"
#include "itemdb.h"
#include "script.h"
#include "pc.h"

// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

static struct dbt* item_db;

static struct item_group itemgroup_db[MAX_ITEMGROUP];

struct item_data dummy_item; //This is the default dummy item used for non-existant items. [Skotlex]

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
	if(item == &dummy_item) return 0;
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
	if (item == &dummy_item)
		return 1; //Invalid item.
	if(stristr(item->jname,str))
		return 0;
	if(stristr(item->name,str))
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
int itemdb_searchrandomid(int group)
{
	if(group<1 || group>=MAX_ITEMGROUP) {
		if (battle_config.error_log)
			ShowError("itemdb_searchrandomid: Invalid group id %d\n", group);
		return UNKNOWN_ITEM_ID;
	}
	if (itemgroup_db[group].qty)
		return itemgroup_db[group].nameid[rand()%itemgroup_db[group].qty];
	
	if (battle_config.error_log)
		ShowError("itemdb_searchrandomid: No item entries for group id %d\n", group);
	return UNKNOWN_ITEM_ID;
}

/*==========================================
 * Returns the group this item belongs to.
 * Skips general random item givers (gift/blue/violet box)
 *------------------------------------------
 */
int itemdb_group (int nameid)
{
	int i, j;
	for (i=0; i < MAX_ITEMGROUP; i++) {
		switch (i) {
			case IG_BLUEBOX:
			case IG_VIOLETBOX:
			case IG_CARDALBUM:
			case IG_GIFTBOX:
			case IG_COOKIEBAG:
			case IG_GIFTBOX_1:
			case IG_GIFTBOX_2:
			case IG_GIFTBOX_3:
			case IG_GIFTBOX_4:
			case IG_GIFTBOXCHINA:
				continue;
		}

		for (j=0; j < itemgroup_db[i].qty; j++) {
			if (itemgroup_db[i].id[j] == nameid)
				return i;
		}
	}
	return -1;
}

/*==========================================
 * DBの存在確認
 *------------------------------------------
 */
struct item_data* itemdb_exists(int nameid)
{
	struct item_data* id;
	if (!nameid) return NULL;
	id = idb_get(item_db,nameid);
//	if (id == &dummy_item) return NULL; //Let dummy items go through... technically they "exist" because someone already has them...
	return id;
}

/*==========================================
 * Converts the jobid from the format in itemdb 
 * to the format used by the map server. [Skotlex]
 *------------------------------------------
 */
static void itemdb_jobid2mapid(unsigned int *bclass, unsigned int jobmask)
{
	int i;
	bclass[0]= bclass[1]= bclass[2]= 0;
	//Base classes
	if (jobmask & 1<<JOB_NOVICE)
	{	//Both Novice/Super-Novice are counted with the same ID
		bclass[0] |= 1<<MAPID_NOVICE;
		bclass[1] |= 1<<MAPID_NOVICE;
	}
	for (i = JOB_NOVICE+1; i <= JOB_THIEF; i++)
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
	if (jobmask & 1<<21) //Taekwon boy
		bclass[0] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<22) //Star Gladiator
		bclass[1] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<23) //Soul Linker
		bclass[2] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<JOB_GUNSLINGER)
		bclass[0] |= 1<<MAPID_GUNSLINGER;
	if (jobmask & 1<<JOB_NINJA)
		bclass[0] |= 1<<MAPID_NINJA;
}

static void create_dummy_data(void) {
	memset(&dummy_item, 0, sizeof(struct item_data));
	dummy_item.nameid=500;
	dummy_item.weight=1;
	dummy_item.value_sell = 1;
	dummy_item.type=3; //Etc item
	strncpy(dummy_item.name,"UNKNOWN_ITEM",ITEM_NAME_LENGTH-1);
	strncpy(dummy_item.jname,"UNKNOWN_ITEM",ITEM_NAME_LENGTH-1);
	dummy_item.view_id = UNKNOWN_ITEM_ID;
}

static void* create_item_data(DBKey key, va_list args) {
	struct item_data *id;
	id=(struct item_data *)aCalloc(1,sizeof(struct item_data));
	id->nameid = key.i;
	id->weight=1;
	id->type=IT_ETC;
	return id;
}

/*==========================================
 * Loads (and creates if not found) an item from the db.
 *------------------------------------------
 */
struct item_data* itemdb_load(int nameid)
{
	struct item_data *id = idb_ensure(item_db,nameid,create_item_data);
	if (id == &dummy_item)
  	{	//Remove dummy_item, replace by real data.
		DBKey key;
		key.i = nameid;
		idb_remove(item_db,nameid);
		id = create_item_data(key, NULL);
		idb_put(item_db,nameid,id);
	}
	return id;
}

static void* return_dummy_data(DBKey key, va_list args) {
	if (battle_config.error_log)
		ShowWarning("itemdb_search: Item ID %d does not exists in the item_db. Using dummy data.\n", key.i);
	return &dummy_item;
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
 * Returns if given item is a player-equippable piece.
 *------------------------------------------
 */
int itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	switch (type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_AMMO:
			return 1;
		default:
			return 0;
	}
}

/*==========================================
 * Alternate version of itemdb_isequip
 *------------------------------------------
 */
int itemdb_isequip2(struct item_data *data)
{ 
	nullpo_retr(0, data);
	switch(data->type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_AMMO:
			return 1;
		default:
			return 0;
	}
}

/*==========================================
* Returns if given item's type is stackable.
*------------------------------------------
*/
int itemdb_isstackable(int nameid)
{
  int type=itemdb_type(nameid);
  switch(type) {
	  case IT_WEAPON:
	  case IT_ARMOR:
	  case IT_PETEGG:
	  case IT_PETARMOR:
		  return 0;
	  default:
		  return 1;
  }
}

/*==========================================
* Alternate version of itemdb_isstackable
*------------------------------------------
*/
int itemdb_isstackable2(struct item_data *data)
{
  nullpo_retr(0, data);
  switch(data->type) {
	  case IT_WEAPON:
	  case IT_ARMOR:
	  case IT_PETEGG:
	  case IT_PETARMOR:
		  return 0;
	  default:
		  return 1;
  }
}


/*==========================================
 * Trade Restriction functions [Skotlex]
 *------------------------------------------
 */
int itemdb_isdropable_sub(struct item_data *item, int gmlv, int unused)
{
	return (item && (!(item->flag.trade_restriction&1) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cantrade_sub(struct item_data* item, int gmlv, int gmlv2)
{
	return (item && (!(item->flag.trade_restriction&2) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_canpartnertrade_sub(struct item_data* item, int gmlv, int gmlv2)
{
	return (item && (item->flag.trade_restriction&4 || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_cansell_sub(struct item_data* item, int gmlv, int unused)
{
	return (item && (!(item->flag.trade_restriction&8) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cancartstore_sub(struct item_data* item, int gmlv, int unused)
{	
	return (item && (!(item->flag.trade_restriction&16) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canstore_sub(struct item_data* item, int gmlv, int unused)
{	
	return (item && (!(item->flag.trade_restriction&32) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canguildstore_sub(struct item_data* item, int gmlv, int unused)
{	
	return (item && (!(item->flag.trade_restriction&64) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_isrestricted(struct item* item, int gmlv, int gmlv2, int (*func)(struct item_data*, int, int))
{
	struct item_data* item_data = itemdb_search(item->nameid);
	int i;

	if (!func(item_data, gmlv, gmlv2))
		return 0;
	
	if(item_data->slot == 0 || itemdb_isspecial(item->card[0]))
		return 1;
	
	for(i = 0; i < item_data->slot; i++) {
		if (!item->card[i]) continue;
		if (!func(itemdb_search(item->card[i]), gmlv, gmlv2))
			return 0;
	}
	return 1;
}

/*==========================================
 *	Specifies if item-type should drop unidentified.
 *------------------------------------------
 */
int itemdb_isidentified(int nameid)
{
	int type=itemdb_type(nameid);
	switch (type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETARMOR:
			return 0;
		default:
			return 1;
	}
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
			(nameid = atoi(str[0])) < 0 || !(id = itemdb_exists(nameid)))
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
static void itemdb_read_itemgroup_sub(const char* filename)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int groupid,j,k,nameid;
	char *str[3],*p;
	char w1[1024], w2[1024];
	
	if( (fp=fopen(filename,"r"))==NULL ){
		ShowError("can't read %s\n", line);
		return;
	}

	while(fgets(line,1020,fp)){
		ln++;
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(strstr(line,"import")) {
			if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2 &&
				strcmpi(w1, "import") == 0) {
				itemdb_read_itemgroup_sub(w2);
				continue;
			}
		}
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<3 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;
		if (j<3)
			continue;
		groupid = atoi(str[0]);
		if (groupid < 0 || groupid >= MAX_ITEMGROUP) {
			ShowWarning("itemdb_read_itemgroup: Invalid group %d in %s:%d\n", groupid, filename, ln);
			continue;
		}
		nameid = atoi(str[1]);
		if (!itemdb_exists(nameid)) {
			ShowWarning("itemdb_read_itemgroup: Non-existant item %d in %s:%d\n", nameid, filename, ln);
			continue;
		}
		k = atoi(str[2]);
		if (itemgroup_db[groupid].qty+k > MAX_RANDITEM) {
			ShowWarning("itemdb_read_itemgroup: Group %d is full (%d entries) in %s:%d\n", groupid, MAX_RANDITEM, filename, ln);
			continue;
		}
		for(j=0;j<k;j++)
			itemgroup_db[groupid].nameid[itemgroup_db[groupid].qty++] = nameid;
	}
	fclose(fp);
	return;
}

static void itemdb_read_itemgroup(void)
{
	char path[256];
	int i;
	const char* groups[] = {
		"Blue Box",
		"Violet Box",
		"Card Album",
		"Gift Box",
		"Scroll Box",
		"Finding Ore",
		"Cookie Bag",
		"Potion",
		"Herbs",
		"Fruits",
		"Meat",
		"Candy",
		"Juice",
		"Fish",
		"Boxes",
		"Gemstone",
		"Jellopy",
		"Ore",
		"Food",
		"Recovery",
		"Minerals",
		"Taming",
		"Scrolls",
		"Quivers",
		"Masks",
		"Accesory",
		"Jewels",
		"Gift Box 1",
		"Gift Box 2",
		"Gift Box 3",
		"Gift Box 4",
		"Egg Boy",
		"Egg Girl",
		"Gift Box China",
		"Lotto Box",
	};
	memset(&itemgroup_db, 0, sizeof(itemgroup_db));
	snprintf(path, 255, "%s/item_group_db.txt", db_path);
	itemdb_read_itemgroup_sub(path);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","item_group_db.txt");
	if (battle_config.etc_log) {
		for (i = 1; i < MAX_ITEMGROUP; i++)
			ShowInfo("Group %s: %d entries.\n", groups[i-1], itemgroup_db[i].qty);
	}
	return;
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
		if (equip && item && itemdb_isequip2(item))
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
		if(nameid<=0 || !(id=itemdb_exists(nameid)))
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
			(nameid = atoi(str[0])) < 0 || !(id = itemdb_exists(nameid)))
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
	if (id->look == W_MUSICAL && id->type == IT_WEAPON) //Musical instruments are always male-only
		return 1;
	if (id->look == W_WHIP && id->type == IT_WEAPON) //Whips are always female-only
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
				{	/*Table structure is:
					00  id
					01  name_english
					02  name_japanese
					03  type
					04  price_buy
					05  price_sell
					06  weight
					07  attack
					08  defence
					09  range
					10  slots
					11  equip_jobs
					12  equip_upper
					13  equip_genders
					14  equip_locations
					15  weapon_level
					16  equip_level
					17  refineable
					18  view
					19  script
					20  equip_script
					21  unequip_script
					*/
					nameid = atoi(sql_row[0]);

					// If the identifier is not within the valid range, process the next row
					if (nameid == 0)
						continue;

					ln++;

					// ----------
					id = itemdb_load(nameid);
					
					strncpy(id->name, sql_row[1], ITEM_NAME_LENGTH-1);
					strncpy(id->jname, sql_row[2], ITEM_NAME_LENGTH-1);

					id->type = atoi(sql_row[3]);
					if (id->type == IT_DELAYCONSUME)
					{	//Items that are consumed upon target confirmation
						//(yggdrasil leaf, spells & pet lures) [Skotlex]
						id->type = IT_USABLE;
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
					itemdb_jobid2mapid(id->class_base, (sql_row[11] != NULL) ? (unsigned int)strtoul(sql_row[11], NULL, 0) : 0);
					id->class_upper= (sql_row[12] != NULL) ? atoi(sql_row[12]) : 0;
					id->sex		= (sql_row[13] != NULL) ? atoi(sql_row[13]) : 0;
					id->equip	= (sql_row[14] != NULL) ? atoi(sql_row[14]) : 0;
					if (!id->equip && itemdb_isequip2(id))
					{
						ShowWarning("Item %d (%s) is an equipment with no equip-field! Making it an etc item.\n", nameid, id->jname);
						id->type = 3;
					}
					id->wlv		= (sql_row[15] != NULL) ? atoi(sql_row[15]) : 0;
					id->elv		= (sql_row[16] != NULL)	? atoi(sql_row[16]) : 0;
					id->flag.no_refine = (sql_row[17] == NULL || atoi(sql_row[17]) == 1)?0:1;
					id->look	= (sql_row[18] != NULL) ? atoi(sql_row[18]) : 0;
					id->view_id	= 0;
					id->sex = itemdb_gendercheck(id); //Apply gender filtering.

					// ----------
					
					if (id->script)
						script_free_code(id->script);
					if (sql_row[19] != NULL) {
						if (sql_row[19][0] == '{')
							id->script = parse_script((unsigned char *) sql_row[19],item_db_name[i], 0);
						else {
							sprintf(script, "{%s}", sql_row[19]);
							id->script = parse_script((unsigned char *) script, item_db_name[i], 0);
						}
					} else id->script = NULL;
	
					if (id->equip_script)
						script_free_code(id->equip_script);
					if (sql_row[20] != NULL) {
						if (sql_row[20][0] == '{')
							id->equip_script = parse_script((unsigned char *) sql_row[20], item_db_name[i], 0);
						else {
							sprintf(script, "{%s}", sql_row[20]);
							id->equip_script = parse_script((unsigned char *) script, item_db_name[i], 0);
						}
					} else id->equip_script = NULL;
	
					if (id->unequip_script)
						script_free_code(id->unequip_script);
					if (sql_row[21] != NULL) {
						if (sql_row[21][0] == '{')
							id->unequip_script = parse_script((unsigned char *) sql_row[21],item_db_name[i], 0);
						else {
							sprintf(script, "{%s}", sql_row[21]);
							id->unequip_script = parse_script((unsigned char *) script, item_db_name[i], 0);
						}
					} else id->unequip_script = NULL;
				
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
			if(nameid<=0)
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
			if (id->type == IT_DELAYCONSUME)
			{	//Items that are consumed upon target confirmation
				//(yggdrasil leaf, spells & pet lures) [Skotlex]
				id->type = IT_USABLE;
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
			itemdb_jobid2mapid(id->class_base, (unsigned int)strtoul(str[11],NULL,0));
			id->class_upper = atoi(str[12]);
			id->sex	= atoi(str[13]);
			if(id->equip != atoi(str[14])){
				id->equip=atoi(str[14]);
			}
			if (!id->equip && itemdb_isequip2(id))
			{
				ShowWarning("Item %d (%s) is an equipment with no equip-field! Making it an etc item.\n", nameid, id->jname);
				id->type = 3;
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
				script_free_code(id->script);
				id->script=NULL;
			}
			if (id->equip_script) {
				script_free_code(id->equip_script);
				id->equip_script=NULL;
			}
			if (id->unequip_script) {
				script_free_code(id->unequip_script);
				id->unequip_script=NULL;
			}

			if((p=strchr(np,'{'))==NULL)
				continue;
			
			str[19] = p; //Script
			np = strchr(p,'}');
			
			while (np && np[1] && np[1] != ',')
				np = strchr(np+1,'}'); //Jump close brackets until the next field is found.
			if (!np || !np[1]) {
				//Couldn't find the end of the script field.
				id->script = parse_script((unsigned char *) str[19],filename[i],lines);
				continue;
			}
			np[1] = '\0'; //Set end of script
			id->script = parse_script((unsigned char *) str[19],filename[i],lines);
			np+=2; //Skip to next field
			
			if(!np || (p=strchr(np,'{'))==NULL)
				continue;
			
			str[20] = p; //Equip Script
			np = strchr(p,'}');
			
			while (np && np[1] && np[1] != ',')
				np = strchr(np+1,'}'); //Jump close brackets until the next field is found.
			if (!np || !np[1]) {
				//Couldn't find the end of the script field.
				id->equip_script = parse_script((unsigned char *) str[20],filename[i],lines);
				continue;
			}
			
			np[1] = '\0'; //Set end of script
			id->equip_script = parse_script((unsigned char *) str[20],filename[i],lines);
			np+=2; //Skip comma, to next field
			
			if(!np || (p=strchr(np,'{'))==NULL)
				continue;
			//Unequip script, last column.
			id->unequip_script = parse_script((unsigned char *) p,filename[i],lines);
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
		script_free_code(id->script);
		id->script = NULL;
	}
	if (id->equip_script)
	{
		script_free_code(id->equip_script);
		id->equip_script = NULL;
	}
	if (id->unequip_script)
	{
		script_free_code(id->unequip_script);
		id->unequip_script = NULL;
	}
	// Whether to clear the item data (exception: do not clear the dummy item data
	if (flag && id != &dummy_item) 
		aFree(id);

	return 0;
}

void itemdb_reload(void)
{
	//Just read, the function takes care of freeing scripts.
	itemdb_read();
}

void do_final_itemdb(void)
{
	item_db->destroy(item_db, itemdb_final_sub, 1);
	if (dummy_item.script) {
		script_free_code(dummy_item.script);
		dummy_item.script = NULL;
	}
	if (dummy_item.equip_script) {
		script_free_code(dummy_item.equip_script);
		dummy_item.equip_script = NULL;
	}
	if (dummy_item.unequip_script) {
		script_free_code(dummy_item.unequip_script);
		dummy_item.unequip_script = NULL;
	}
}

int do_init_itemdb(void)
{
	item_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_BASE,sizeof(int)); 
	create_dummy_data(); //Dummy data item.
	itemdb_read();

	return 0;
}
