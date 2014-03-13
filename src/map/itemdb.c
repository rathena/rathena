// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "itemdb.h"
#include "map.h"
#include "battle.h" // struct battle_config
#include "cashshop.h"
#include "script.h" // item script processing
#include "pc.h"     // W_MUSICAL, W_WHIP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct item_data* itemdb_array[MAX_ITEMDB];
static DBMap*            itemdb_other;// int nameid -> struct item_data*

static struct s_item_group_db itemgroup_db[MAX_ITEMGROUP];

struct item_data dummy_item; //This is the default dummy item used for non-existant items. [Skotlex]

static DBMap *itemdb_combo;

DBMap * itemdb_get_combodb(){
	return itemdb_combo;
}

/**
 * Search for item name
 * name = item alias, so we should find items aliases first. if not found then look for "jname" (full name)
 * @see DBApply
 */
static int itemdb_searchname_sub(DBKey key, DBData *data, va_list ap)
{
	struct item_data *item = db_data2ptr(data), **dst, **dst2;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
	dst2=va_arg(ap,struct item_data **);
	if(item == &dummy_item) return 0;

	//Absolute priority to Aegis code name.
	if (*dst != NULL) return 0;
	if( strcmpi(item->name,str)==0 )
		*dst=item;

	//Second priority to Client displayed name.
	if (*dst2 != NULL) return 0;
	if( strcmpi(item->jname,str)==0 )
		*dst2=item;
	return 0;
}

/*==========================================
 * Return item data from item name. (lookup)
 *------------------------------------------*/
struct item_data* itemdb_searchname(const char *str)
{
	struct item_data* item;
	struct item_data* item2=NULL;
	int i;

	for( i = 0; i < ARRAYLENGTH(itemdb_array); ++i )
	{
		item = itemdb_array[i];
		if( item == NULL )
			continue;

		// Absolute priority to Aegis code name.
		if( strcasecmp(item->name,str) == 0 )
			return item;

		//Second priority to Client displayed name.
		if( strcasecmp(item->jname,str) == 0 )
			item2 = item;
	}

	item = NULL;
	itemdb_other->foreach(itemdb_other,itemdb_searchname_sub,str,&item,&item2);
	return item?item:item2;
}

/**
 * @see DBMatcher
 */
static int itemdb_searchname_array_sub(DBKey key, DBData data, va_list ap)
{
	struct item_data *item = db_data2ptr(&data);
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
 *------------------------------------------*/
int itemdb_searchname_array(struct item_data** data, int size, const char *str)
{
	struct item_data* item;
	int i;
	int count=0;

	// Search in the array
	for( i = 0; i < ARRAYLENGTH(itemdb_array); ++i )
	{
		item = itemdb_array[i];
		if( item == NULL )
			continue;

		if( stristr(item->jname,str) || stristr(item->name,str) )
		{
			if( count < size )
				data[count] = item;
			++count;
		}
	}

	// search in the db
	if( count < size )
	{
		DBData *db_data[MAX_SEARCH];
		int db_count = 0;
		size -= count;
		db_count = itemdb_other->getall(itemdb_other, (DBData**)&db_data, size, itemdb_searchname_array_sub, str);
		for (i = 0; i < db_count; i++)
			data[count++] = db_data2ptr(db_data[i]);
		count += db_count;
	}
	return count;
}

/**
* Return a random item id from group. (takes into account % chance giving/tot group)
* NOTE: Sub group 0 will be set to default 1, since 0 isn't random group
* @param group_id
* @param sub_group: Default is 1
* @return nameid
*/
unsigned short itemdb_searchrandomid(uint16 group_id, uint8 sub_group)
{
	if (sub_group)
		sub_group -= 1;
	if (!group_id || group_id >= MAX_ITEMGROUP || !&itemgroup_db[group_id]) {
		ShowError("itemdb_searchrandomid: Invalid group id %d\n", group_id);
		return UNKNOWN_ITEM_ID;
	}
	if (sub_group > MAX_ITEMGROUP_RANDGROUP) {
		ShowError("itemdb_searchrandomid: Invalid sub_group %d\n", sub_group+1);
		return UNKNOWN_ITEM_ID;
	}
	if (&itemgroup_db[group_id].random[sub_group] && itemgroup_db[group_id].random[sub_group].data_qty)
		return itemgroup_db[group_id].random[sub_group].data[rand()%itemgroup_db[group_id].random[sub_group].data_qty].nameid;

	ShowError("itemdb_searchrandomid: No item entries for group id %d and sub group %d\n", group_id, sub_group+1);
	return UNKNOWN_ITEM_ID;
}

/** [Cydh]
* Return a number of item's amount that will be obtained for 'getrandgroupitem id,1;'
* NOTE: Sub group 0 will be set to default 1, since 0 isn't random group
* @param group_id
* @param sub_group
* @param nameid: The target item will be found
* @return amount
*/
uint16 itemdb_get_randgroupitem_count(uint16 group_id, uint8 sub_group, uint16 nameid) {
	uint16 i, amt = 1;
	if (sub_group)
		sub_group -= 1;
	if (!group_id || group_id >= MAX_ITEMGROUP || !&itemgroup_db[group_id]) {
		ShowError("itemdb_get_randgroupitem_count: Invalid group id %d\n", group_id);
		return amt;
	}
	if (sub_group > MAX_ITEMGROUP_RANDGROUP) {
		ShowError("itemdb_get_randgroupitem_count: Invalid sub_group id %d\n", group_id+1);
		return amt;
	}
	if (!(&itemgroup_db[group_id].random[sub_group]) || !itemgroup_db[group_id].random[sub_group].data_qty)
		return amt;
	ARR_FIND(0,itemgroup_db[group_id].random[sub_group].data_qty,i,itemgroup_db[group_id].random[sub_group].data[i].nameid == nameid);
	if (i < itemgroup_db[group_id].random[sub_group].data_qty)
		amt = itemgroup_db[group_id].random[sub_group].data[i].amount;
	return amt;
}

/** [Cydh]
* Gives item(s) to the player based on item group
* @param sd: Player that obtains item from item group
* @param group_id: The group ID of item that obtained by player
* @param *group: struct s_item_group from itemgroup_db[group_id].random[idx] or itemgroup_db[group_id].must[sub_group][idx]
*/
static void itemdb_pc_get_itemgroup_sub(struct map_session_data *sd, uint16 group_id, struct s_item_group *group) {
	uint16 i;
	struct item tmp;

	nullpo_retv(group);

	memset(&tmp,0,sizeof(tmp));

	tmp.nameid = group->nameid;
	tmp.amount = (itemdb_isstackable(group->nameid)) ? group->amount : 1;
	tmp.bound = group->bound;
	tmp.identify = 1;
	tmp.expire_time = (group->duration) ? (unsigned int)(time(NULL) + group->duration*60) : 0;
	if (group->isNamed) {
		tmp.card[0] = itemdb_isequip(group->nameid) ? CARD0_FORGE : CARD0_CREATE;
		tmp.card[1] = 0;
		tmp.card[2] = GetWord(sd->status.char_id, 0);
		tmp.card[3] = GetWord(sd->status.char_id, 1);
	}
	//Do loop for non-stackable item
	for (i = 0; i < group->amount; i++) {
		int flag;
		if ((flag = pc_additem(sd,&tmp,tmp.amount,LOG_TYPE_SCRIPT)))
			clif_additem(sd,0,0,flag);
		else if (!flag && group->isAnnounced) { ///TODO: Move this broadcast to proper behavior (it should on at different packet)
			char output[CHAT_SIZE_MAX];
			sprintf(output,msg_txt(NULL,717),sd->status.name,itemdb_jname(group->nameid),itemdb_jname(sd->itemid));
			clif_broadcast(&sd->bl,output,strlen(output),0,ALL_CLIENT);
			//clif_broadcast_obtain_special_item();
		}
		if (itemdb_isstackable(group->nameid))
			break;
	}
}

/** [Cydh]
* Find item(s) that will be obtained by player based on Item Group
* @param group_id: The group ID that will be gained by player
* @param nameid: The item that trigger this item group
* @return val: 0:success, 1:no sd, 2:invalid item group
*/
char itemdb_pc_get_itemgroup(uint16 group_id, struct map_session_data *sd) {
	uint16 i = 0;

	nullpo_retr(1,sd);
	
	if (!group_id || group_id >= MAX_ITEMGROUP) {
		ShowError("itemdb_pc_get_itemgroup: Invalid group id '%d' specified.",group_id);
		return 2;
	}
	
	//Get the 'must' item(s)
	if (itemgroup_db[group_id].must_qty)
		for (i = 0; i < itemgroup_db[group_id].must_qty; i++)
			if (&itemgroup_db[group_id].must[i] && itemdb_exists(itemgroup_db[group_id].must[i].nameid))
				itemdb_pc_get_itemgroup_sub(sd,group_id,&itemgroup_db[group_id].must[i]);

	//Get the 'random' item each random group
	for (i = 0; i < MAX_ITEMGROUP_RANDGROUP; i++) {
		uint16 rand;
		if (!(&itemgroup_db[group_id].random[i]) || !itemgroup_db[group_id].random[i].data_qty) //Skip empty random group
			continue;
		rand = rnd()%itemgroup_db[group_id].random[i].data_qty;
		//Woops, why is the data empty? Every check should be done when load the item group! So this is bad day for the player :P
		if (!&itemgroup_db[group_id].random[i].data[rand] || !itemgroup_db[group_id].random[i].data[rand].nameid) {
			continue;
		}
		if (itemdb_exists(itemgroup_db[group_id].random[i].data[rand].nameid))
			itemdb_pc_get_itemgroup_sub(sd,group_id,&itemgroup_db[group_id].random[i].data[rand]);
	}

	return 0;
}

/*==========================================
 * Calculates total item-group related bonuses for the given item
 *------------------------------------------*/
int itemdb_group_bonus(struct map_session_data* sd, int itemid)
{
	int bonus = 0, i, j;
	for (i=0; i < MAX_ITEMGROUP; i++) {
		if (!sd->itemgrouphealrate[i])
			continue;
		ARR_FIND(0,itemgroup_db[i].random[0].data_qty,j,itemgroup_db[i].random[0].data[j].nameid == itemid );
		if( j < itemgroup_db[i].random[0].data_qty )
			bonus += sd->itemgrouphealrate[i];
	}
	return bonus;
}

/// Searches for the item_data.
/// Returns the item_data or NULL if it does not exist.
struct item_data* itemdb_exists(int nameid)
{
	struct item_data* item;

	if( nameid >= 0 && nameid < ARRAYLENGTH(itemdb_array) )
		return itemdb_array[nameid];
	item = (struct item_data*)idb_get(itemdb_other,nameid);
	if( item == &dummy_item )
		return NULL;// dummy data, doesn't exist
	return item;
}

/// Returns name type of ammunition [Cydh]
const char *itemdb_typename_ammo (enum e_item_ammo ammo) {
	switch (ammo) {
		case AMMO_ARROW:			return "Arrow";
		case AMMO_THROWABLE_DAGGER:	return "Throwable Dagger";
		case AMMO_BULLET:			return "Bullet";
		case AMMO_SHELL:			return "Shell";
		case AMMO_GRENADE:			return "Grenade";
		case AMMO_SHURIKEN:			return "Shuriken";
		case AMMO_KUNAI:			return "Kunai";
		case AMMO_CANNONBALL:		return "Cannonball";
		case AMMO_THROWABLE_ITEM:	return "Throwable Item/Sling Item";
	}
	return "Ammunition";
}

/// Returns human readable name for given item type.
/// @param type Type id to retrieve name for ( IT_* ).
const char* itemdb_typename(enum item_types type)
{
	switch(type)
	{
		case IT_HEALING:        return "Potion/Food";
		case IT_USABLE:         return "Usable";
		case IT_ETC:            return "Etc.";
		case IT_WEAPON:         return "Weapon";
		case IT_ARMOR:          return "Armor";
		case IT_CARD:           return "Card";
		case IT_PETEGG:         return "Pet Egg";
		case IT_PETARMOR:       return "Pet Accessory";
		case IT_AMMO:           return "Arrow/Ammunition";
		case IT_DELAYCONSUME:   return "Delay-Consume Usable";
		case IT_SHADOWGEAR:     return "Shadow Equipment";
		case IT_CASH:           return "Cash Usable";
	}
	return "Unknown Type";
}

/*==========================================
 * Converts the jobid from the format in itemdb
 * to the format used by the map server. [Skotlex]
 *------------------------------------------*/
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
//	Bard/Dancer share the same slot now.
//	if (jobmask & 1<<JOB_DANCER)
//		bclass[2] |= 1<<MAPID_ARCHER;
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
	if (jobmask & 1<<JOB_NINJA) { //Kagerou/Oboro jobs can equip Ninja equips. [Rytech]
		bclass[0] |= 1<<MAPID_NINJA;
		bclass[1] |= 1<<MAPID_NINJA;
	}
	if (jobmask & 1<<26) //Bongun/Munak
		bclass[0] |= 1<<MAPID_GANGSI;
	if (jobmask & 1<<27) //Death Knight
		bclass[1] |= 1<<MAPID_GANGSI;
	if (jobmask & 1<<28) //Dark Collector
		bclass[2] |= 1<<MAPID_GANGSI;
	if (jobmask & 1<<29) //Kagerou / Oboro
		bclass[1] |= 1<<MAPID_NINJA;
	if (jobmask & 1<<30) //Rebellion
		bclass[1] |= 1<<MAPID_GUNSLINGER;
}

static void create_dummy_data(void)
{
	memset(&dummy_item, 0, sizeof(struct item_data));
	dummy_item.nameid=500;
	dummy_item.weight=1;
	dummy_item.value_sell=1;
	dummy_item.type=IT_ETC; //Etc item
	safestrncpy(dummy_item.name,"UNKNOWN_ITEM",sizeof(dummy_item.name));
	safestrncpy(dummy_item.jname,"UNKNOWN_ITEM",sizeof(dummy_item.jname));
	dummy_item.view_id=UNKNOWN_ITEM_ID;
}

static struct item_data* create_item_data(int nameid)
{
	struct item_data *id;
	CREATE(id, struct item_data, 1);
	id->nameid = nameid;
	id->weight = 1;
	id->type = IT_ETC;
	return id;
}

/*==========================================
 * Loads (and creates if not found) an item from the db.
 *------------------------------------------*/
struct item_data* itemdb_load(int nameid)
{
	struct item_data *id;

	if( nameid >= 0 && nameid < ARRAYLENGTH(itemdb_array) )
	{
		id = itemdb_array[nameid];
		if( id == NULL || id == &dummy_item )
			id = itemdb_array[nameid] = create_item_data(nameid);
		return id;
	}

	id = (struct item_data*)idb_get(itemdb_other, nameid);
	if( id == NULL || id == &dummy_item )
	{
		id = create_item_data(nameid);
		idb_put(itemdb_other, nameid, id);
	}
	return id;
}

/*==========================================
 * Loads an item from the db. If not found, it will return the dummy item.
 *------------------------------------------*/
struct item_data* itemdb_search(int nameid)
{
	struct item_data* id;
	if( nameid >= 0 && nameid < ARRAYLENGTH(itemdb_array) )
		id = itemdb_array[nameid];
	else
		id = (struct item_data*)idb_get(itemdb_other, nameid);

	if( id == NULL )
	{
		ShowWarning("itemdb_search: Item ID %d does not exists in the item_db. Using dummy data.\n", nameid);
		id = &dummy_item;
		dummy_item.nameid = nameid;
	}
	return id;
}

/*==========================================
 * Returns if given item is a player-equippable piece.
 *------------------------------------------*/
bool itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	switch (type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_AMMO:
		case IT_SHADOWGEAR:
			return true;
		default:
			return false;
	}
}

/*==========================================
 * Alternate version of itemdb_isequip
 *------------------------------------------*/
bool itemdb_isequip2(struct item_data *data)
{
	nullpo_ret(data);
	switch(data->type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_AMMO:
		case IT_SHADOWGEAR:
			return true;
		default:
			return false;
	}
}

/*==========================================
 * Returns if given item's type is stackable.
 *------------------------------------------*/
bool itemdb_isstackable(uint16 nameid)
{
  uint8 type = itemdb_type(nameid);
  switch(type) {
	  case IT_WEAPON:
	  case IT_ARMOR:
	  case IT_PETEGG:
	  case IT_PETARMOR:
	  case IT_SHADOWGEAR:
		  return false;
	  default:
		  return true;
  }
}

/*==========================================
 * Alternate version of itemdb_isstackable
 *------------------------------------------*/
bool itemdb_isstackable2(struct item_data *data)
{
  nullpo_ret(data);
  switch(data->type) {
	  case IT_WEAPON:
	  case IT_ARMOR:
	  case IT_PETEGG:
	  case IT_PETARMOR:
	  case IT_SHADOWGEAR:
		  return false;
	  default:
		  return true;
  }
}


/*==========================================
 * Trade Restriction functions [Skotlex]
 *------------------------------------------*/
int itemdb_isdropable_sub(struct item_data *item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&1) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cantrade_sub(struct item_data* item, int gmlv, int gmlv2) {
	return (item && (!(item->flag.trade_restriction&2) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_canpartnertrade_sub(struct item_data* item, int gmlv, int gmlv2) {
	return (item && (item->flag.trade_restriction&4 || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_cansell_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&8) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cancartstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&16) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&32) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canguildstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&64) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canmail_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&128) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canauction_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&256) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_isrestricted(struct item* item, int gmlv, int gmlv2, int (*func)(struct item_data*, int, int))
{
	struct item_data* item_data = itemdb_search(item->nameid);
	int i;

	if (!func(item_data, gmlv, gmlv2))
		return false;

	if(item_data->slot == 0 || itemdb_isspecial(item->card[0]))
		return true;

	for(i = 0; i < item_data->slot; i++) {
		if (!item->card[i]) continue;
		if (!func(itemdb_search(item->card[i]), gmlv, gmlv2))
			return false;
	}
	return true;
}

/** Specifies if item-type should drop unidentified.
* @param nameid ID of item
*/
char itemdb_isidentified(int nameid) {
	int type=itemdb_type(nameid);
	switch (type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETARMOR:
		case IT_SHADOWGEAR:
			return 0;
		default:
			return 1;
	}
}

/** Search by name for the override flags available items (Give item another sprite)
* Structure: <nameid>,<sprite>
*/
static bool itemdb_read_itemavail(char* str[], int columns, int current) {
	int nameid, sprite;
	struct item_data *id;

	nameid = atoi(str[0]);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		ShowWarning("itemdb_read_itemavail: Invalid item id %d.\n", nameid);
		return false;
	}

	sprite = atoi(str[1]);

	if( sprite > 0 )
	{
		id->flag.available = 1;
		id->view_id = sprite;
	}
	else
	{
		id->flag.available = 0;
	}

	return true;
}

/** Read item group data
* Structure: GroupID,ItemID,Rate{,Amount,isMust,isAnnounced,Duration,isNamed,isBound}
*/
static void itemdb_read_itemgroup_sub(const char* filename, bool silent)
{
	FILE *fp;
	int ln=0, entries=0;
	char line[1024];

	if ((fp=fopen(filename,"r")) == NULL) {
		if(silent == 0) ShowError("Can't read %s\n", filename);
		return;
	}
	
	while (fgets(line,sizeof(line),fp)) {
		uint16 nameid;
		int j, group_id, prob = 1, amt = 1, rand_group = 1, announced = 0, dur = 0, named = 0, bound = 0;
		char *str[3], *p, w1[1024], w2[1024];
		bool found = false;
		struct s_item_group_random *random;

		ln++;
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (strstr(line,"import")) {
			if (sscanf(line,"%[^:]: %[^\r\n]",w1,w2) == 2 &&
				strcmpi(w1,"import") == 0)
			{
				itemdb_read_itemgroup_sub(w2, 0);
				continue;
			}
		}
		memset(str,0,sizeof(str));
		for (j = 0, p = line; j < 3 && p;j++) {
			str[j] = p;
			if (j == 2)
				sscanf(str[j],"%d,%d,%d,%d,%d,%d,%d",&prob,&amt,&rand_group,&announced,&dur,&named,&bound);
			p = strchr(p,',');
			if (p) *p++=0;
		}
		if (str[0] == NULL)
			continue;
		if (j < 3) {
			if (j > 1) //Or else it barks on blank lines...
				ShowWarning("itemdb_read_itemgroup: Insufficient fields for entry at %s:%d\n", filename, ln);
			continue;
		}

		//Checking group_id
		trim(str[0]);
		if (ISDIGIT(str[0][0]))
			group_id = atoi(str[0]);
		else //Try reads group id by const
			script_get_constant(trim(str[0]),&group_id);
		if (!group_id || group_id >= MAX_ITEMGROUP) {
			ShowWarning("itemdb_read_itemgroup: Cannot save '%s' because invalid group id or group db is overflow in %s:%d\n", str[0], filename, ln);
			continue;
		}

		//Checking sub group
		if (rand_group < 0 || rand_group > MAX_ITEMGROUP_RANDGROUP) {
			ShowWarning("itemdb_read_itemgroup: Invalid sub group %d for group '%s' in %s:%d\n", rand_group, str[0], filename, ln);
			continue;
		}

		if (rand_group && prob < 1) {
			ShowWarning("itemdb_read_itemgroup: Invalid probaility for group '%s' sub: %d in %s:%d\n", str[0], rand_group, filename, ln);
			continue;
		}

		//Checking item
		trim(str[1]);
		if (ISDIGIT(str[1][0]) && itemdb_exists((nameid = atoi(str[1]))))
			found = true;
		else if (itemdb_searchname(str[1])) {
			found = true;
			nameid = itemdb_searchname(str[1])->nameid;
		}
		if (!found) {
			ShowWarning("itemdb_read_itemgroup: Non-existant item '%s' in %s:%d\n", str[1], filename, ln);
			continue;
		}

		amt = cap_value(amt,1,MAX_AMOUNT);
		dur = cap_value(dur,0,UINT16_MAX);
		bound = cap_value(bound,0,4);

		//Must item (rand_group == 0), place it here
		if (!rand_group) {
			uint16 idx = itemgroup_db[group_id].must_qty;
			if (!idx)
				CREATE(itemgroup_db[group_id].must,struct s_item_group,1);
			else
				RECREATE(itemgroup_db[group_id].must,struct s_item_group,idx+1);

			itemgroup_db[group_id].must[idx].nameid = nameid;
			itemgroup_db[group_id].must[idx].amount = amt;
			itemgroup_db[group_id].must[idx].isAnnounced = announced;
			itemgroup_db[group_id].must[idx].duration = dur;
			itemgroup_db[group_id].must[idx].isNamed = named;
			itemgroup_db[group_id].must[idx].bound = bound;
			itemgroup_db[group_id].must_qty++;
			//If 'must' item isn't set as random item, skip the next process
			if (!prob) {
				entries++;
				continue;
			}
			rand_group = 0;
		}
		else
			rand_group -= 1;

		random = &itemgroup_db[group_id].random[rand_group];

		//Check, if the entry for this random group already created or not
		if (!random->data_qty) {
			CREATE(random->data,struct s_item_group,prob);
			random->data_qty = 0;
		}
		else
			RECREATE(random->data,struct s_item_group,random->data_qty+prob);
		//Now put the entry to its rand_group
		for (j = random->data_qty; j < random->data_qty+prob; j++) {
			random->data[j].nameid = nameid;
			random->data[j].amount = amt;
			random->data[j].isAnnounced = announced;
			random->data[j].duration = dur;
			random->data[j].isNamed = named;
			random->data[j].bound = bound;
		}
		random->data_qty += prob;
		entries++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", entries, filename);
	return;
}

static void itemdb_read_itemgroup(const char* basedir, bool silent) {
	char filepath[256];
	sprintf(filepath, "%s/%s", basedir, "item_group_db.txt");
	itemdb_read_itemgroup_sub(filepath, silent);
	return;
}

/** Read item forbidden by mapflag (can't equip item)
* Structure: <nameid>,<mode>
*/
static bool itemdb_read_noequip(char* str[], int columns, int current) {
	int nameid;
	struct item_data *id;

	nameid = atoi(str[0]);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		ShowWarning("itemdb_read_noequip: Invalid item id %d.\n", nameid);
		return false;
	}

	id->flag.no_equip |= atoi(str[1]);

	return true;
}

/** Reads item trade restrictions [Skotlex]
* Structure: <nameid>,<mask>,<gm level>
*/
static bool itemdb_read_itemtrade(char* str[], int columns, int current) {
	int nameid, flag, gmlv;
	struct item_data *id;

	nameid = atoi(str[0]);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		//ShowWarning("itemdb_read_itemtrade: Invalid item id %d.\n", nameid);
		//return false;
		// FIXME: item_trade.txt contains items, which are commented in item database.
		return true;
	}

	flag = atoi(str[1]);
	gmlv = atoi(str[2]);

	if( flag < 0 || flag > 511 ) {//Check range
		ShowWarning("itemdb_read_itemtrade: Invalid trading mask %d for item id %d.\n", flag, nameid);
		return false;
	}
	if( gmlv < 1 )
	{
		ShowWarning("itemdb_read_itemtrade: Invalid override GM level %d for item id %d.\n", gmlv, nameid);
		return false;
	}

	id->flag.trade_restriction = flag;
	id->gm_lv_trade_override = gmlv;

	return true;
}

/** Reads item delay amounts [Paradox924X]
* Structure: <nameid>,<delay>
*/
static bool itemdb_read_itemdelay(char* str[], int columns, int current) {
	int nameid, delay;
	struct item_data *id;

	nameid = atoi(str[0]);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		ShowWarning("itemdb_read_itemdelay: Invalid item id %d.\n", nameid);
		return false;
	}

	delay = atoi(str[1]);

	if( delay < 0 )
	{
		ShowWarning("itemdb_read_itemdelay: Invalid delay %d for item id %d.\n", id->delay, nameid);
		return false;
	}

	id->delay = delay;

	return true;
}

/** Reads item stacking restrictions
* Structure: <item id>,<stack limit amount>,<type>
*/
static bool itemdb_read_stack(char* fields[], int columns, int current) {
	unsigned short nameid, amount;
	unsigned int type;
	struct item_data* id;

	nameid = (unsigned short)strtoul(fields[0], NULL, 10);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		ShowWarning("itemdb_read_stack: Unknown item id '%hu'.\n", nameid);
		return false;
	}

	if( !itemdb_isstackable2(id) )
	{
		ShowWarning("itemdb_read_stack: Item id '%hu' is not stackable.\n", nameid);
		return false;
	}

	amount = (unsigned short)strtoul(fields[1], NULL, 10);
	type = strtoul(fields[2], NULL, 10);

	if( !amount )
	{// ignore
		return true;
	}

	id->stack.amount       = amount;
	id->stack.inventory    = (type&1)!=0;
	id->stack.cart         = (type&2)!=0;
	id->stack.storage      = (type&4)!=0;
	id->stack.guildstorage = (type&8)!=0;

	return true;
}

/** Reads items allowed to be sold in buying stores
* <nameid>
*/
static bool itemdb_read_buyingstore(char* fields[], int columns, int current) {
	int nameid;
	struct item_data* id;

	nameid = atoi(fields[0]);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		ShowWarning("itemdb_read_buyingstore: Invalid item id %d.\n", nameid);
		return false;
	}

	if( !itemdb_isstackable2(id) )
	{
		ShowWarning("itemdb_read_buyingstore: Non-stackable item id %d cannot be enabled for buying store.\n", nameid);
		return false;
	}

	id->flag.buyingstore = true;

	return true;
}

/** Item usage restriction (item_nouse.txt)
* <nameid>,<flag>,<override>
*/
static bool itemdb_read_nouse(char* fields[], int columns, int current) {
	int nameid, flag, override;
	struct item_data* id;

	nameid = atoi(fields[0]);

	if( ( id = itemdb_exists(nameid) ) == NULL ) {
		ShowWarning("itemdb_read_nouse: Invalid item id %d.\n", nameid);
		return false;
	}

	flag = atoi(fields[1]);
	override = atoi(fields[2]);

	id->item_usage.flag = flag;
	id->item_usage.override = override;

	return true;
}

/**
 * @return: amount of retrieved entries.
 **/
static int itemdb_combo_split_atoi (char *str, int *val) {
	int i;

	for (i=0; i<MAX_ITEMS_PER_COMBO; i++) {
		if (!str) break;

		val[i] = atoi(str);

		str = strchr(str,':');

		if (str)
			*str++=0;
	}

	if( i == 0 ) //No data found.
		return 0;

	return i;
}
/**
 * <combo{:combo{:combo:{..}}}>,<{ script }>
 **/
static void itemdb_read_combos(const char* basedir, bool silent) {
	uint32 lines = 0, count = 0;
	char line[1024];

	char path[256];
	FILE* fp;

	sprintf(path, "%s/%s", basedir, "item_combo_db.txt");

	if ((fp = fopen(path, "r")) == NULL) {
		if(silent==0) ShowError("itemdb_read_combos: File not found \"%s\".\n", path);
		return;
	}

	// process rows one by one
	while(fgets(line, sizeof(line), fp)) {
		char *str[2], *p;

		lines++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		memset(str, 0, sizeof(str));

		p = line;

		p = trim(p);

		if (*p == '\0')
			continue;// empty line

		if (!strchr(p,','))
		{
			/* is there even a single column? */
			ShowError("itemdb_read_combos: Insufficient columns in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}

		str[0] = p;
		p = strchr(p,',');
		*p = '\0';
		p++;

		str[1] = p;
		p = strchr(p,',');
		p++;

		if (str[1][0] != '{') {
			ShowError("itemdb_read_combos(#1): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}
		/* no ending key anywhere (missing \}\) */
		if ( str[1][strlen(str[1])-1] != '}' ) {
			ShowError("itemdb_read_combos(#2): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		} else {
			int items[MAX_ITEMS_PER_COMBO];
			int v = 0, retcount = 0;
			struct item_data * id = NULL;
			int idx = 0;
			if((retcount = itemdb_combo_split_atoi(str[0], items)) < 2) {
				ShowError("itemdb_read_combos: line %d of \"%s\" doesn't have enough items to make for a combo (min:2), skipping.\n", lines, path);
				continue;
			}
			/* validate */
			for(v = 0; v < retcount; v++) {
				if( !itemdb_exists(items[v]) ) {
					ShowError("itemdb_read_combos: line %d of \"%s\" contains unknown item ID %d, skipping.\n", lines, path,items[v]);
					break;
				}
			}
			/* failed at some item */
			if( v < retcount )
				continue;
			id = itemdb_exists(items[0]);
			idx = id->combos_count;
			/* first entry, create */
			if( id->combos == NULL ) {
				CREATE(id->combos, struct item_combo*, 1);
				id->combos_count = 1;
			} else {
				RECREATE(id->combos, struct item_combo*, ++id->combos_count);
			}
			CREATE(id->combos[idx],struct item_combo,1);
			id->combos[idx]->nameid = aMalloc( retcount * sizeof(unsigned short) );
			id->combos[idx]->count = retcount;
			id->combos[idx]->script = parse_script(str[1], path, lines, 0);
			id->combos[idx]->id = count;
			id->combos[idx]->isRef = false;
			/* populate ->nameid field */
			for( v = 0; v < retcount; v++ ) {
				id->combos[idx]->nameid[v] = items[v];
			}

			/* populate the children to refer to this combo */
			for( v = 1; v < retcount; v++ ) {
				struct item_data * it;
				int index;
				it = itemdb_exists(items[v]);
				index = it->combos_count;
				if( it->combos == NULL ) {
					CREATE(it->combos, struct item_combo*, 1);
					it->combos_count = 1;
				} else {
					RECREATE(it->combos, struct item_combo*, ++it->combos_count);
				}
				CREATE(it->combos[index],struct item_combo,1);
				/* we copy previously alloc'd pointers and just set it to reference */
				memcpy(it->combos[index],id->combos[idx],sizeof(struct item_combo));
				/* we flag this way to ensure we don't double-dealloc same data */
				it->combos[index]->isRef = true;
			}
			idb_put(itemdb_combo,id->combos[idx]->id,id->combos[idx]);
		}
		count++;
	}
	fclose(fp);

	ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",count,path);

	return;
}



/*======================================
 * Applies gender restrictions according to settings. [Skotlex]
 *======================================*/
static char itemdb_gendercheck(struct item_data *id)
{
	if (id->nameid == WEDDING_RING_M) //Grom Ring
		return 1;
	if (id->nameid == WEDDING_RING_F) //Bride Ring
		return 0;
	if (id->look == W_MUSICAL && id->type == IT_WEAPON) //Musical instruments are always male-only
		return 1;
	if (id->look == W_WHIP && id->type == IT_WEAPON) //Whips are always female-only
		return 0;

	return (battle_config.ignore_items_gender) ? 2 : id->sex;
}
/**
 * [RRInd]
 * For backwards compatibility, in Renewal mode, MATK from weapons comes from the atk slot
 * We use a ':' delimiter which, if not found, assumes the weapon does not provide any matk.
 **/
#ifdef RENEWAL
static void itemdb_re_split_atoi(char *str, int *atk, int *matk) {
	int i, val[2];

	for (i=0; i<2; i++) {
		if (!str) break;
		val[i] = atoi(str);
		str = strchr(str,':');
		if (str)
			*str++=0;
	}
	if( i == 0 ) {
		*atk = *matk = 0;
		return;//no data found
	}
	if( i == 1 ) {//Single Value, we assume it's the ATK
		*atk = val[0];
		*matk = 0;
		return;
	}
	//We assume we have 2 values.
	*atk = val[0];
	*matk = val[1];
	return;
}
#endif
/**
* Processes one itemdb entry
*/
static bool itemdb_parse_dbrow(char** str, const char* source, int line, int scriptopt) {
	/*
		+----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+-------------+---------------+-----------------+--------------+-------------+------------+------+--------+--------------+----------------+
		| 00 |      01      |       02      |  03  |     04    |     05     |   06   |   07   |    08   |   09  |   10  |     11     |      12     |       13      |        14       |      15      |      16     |     17     |  18  |   19   |      20      |        21      |
		+----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+-------------+---------------+-----------------+--------------+-------------+------------+------+--------+--------------+----------------+
		| id | name_english | name_japanese | type | price_buy | price_sell | weight | attack | defence | range | slots | equip_jobs | equip_upper | equip_genders | equip_locations | weapon_level | equip_level | refineable | view | script | equip_script | unequip_script |
		+----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+-------------+---------------+-----------------+--------------+-------------+------------+------+--------+--------------+----------------+
	*/
	int nameid;
	struct item_data* id;

	nameid = atoi(str[0]);
	if( nameid <= 0 )
	{
		ShowWarning("itemdb_parse_dbrow: Invalid id %d in line %d of \"%s\", skipping.\n", nameid, line, source);
		return false;
	}

	//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Job Upper,Gender,Loc,wLV,eLV,refineable,View
	id = itemdb_load(nameid);
	safestrncpy(id->name, str[1], sizeof(id->name));
	safestrncpy(id->jname, str[2], sizeof(id->jname));

	id->type = atoi(str[3]);

	if( id->type < 0 || id->type == IT_UNKNOWN || id->type == IT_UNKNOWN2 || ( id->type > IT_SHADOWGEAR && id->type < IT_CASH ) || id->type >= IT_MAX )
	{// catch invalid item types
		ShowWarning("itemdb_parse_dbrow: Invalid item type %d for item %d. IT_ETC will be used.\n", id->type, nameid);
		id->type = IT_ETC;
	}

	if (id->type == IT_DELAYCONSUME)
	{	//Items that are consumed only after target confirmation
		id->type = IT_USABLE;
		id->flag.delay_consume = 1;
	} else //In case of an itemdb reload and the item type changed.
		id->flag.delay_consume = 0;

	//When a particular price is not given, we should base it off the other one
	//(it is important to make a distinction between 'no price' and 0z)
	if ( str[4][0] )
		id->value_buy = atoi(str[4]);
	else
		id->value_buy = atoi(str[5]) * 2;

	if ( str[5][0] )
		id->value_sell = atoi(str[5]);
	else
		id->value_sell = id->value_buy / 2;
	/*
	if ( !str[4][0] && !str[5][0])
	{
		ShowWarning("itemdb_parse_dbrow: No buying/selling price defined for item %d (%s), using 20/10z\n",       nameid, id->jname);
		id->value_buy = 20;
		id->value_sell = 10;
	} else
	*/
	if (id->value_buy/124. < id->value_sell/75.)
		ShowWarning("itemdb_parse_dbrow: Buying/Selling [%d/%d] price of item %d (%s) allows Zeny making exploit  through buying/selling at discounted/overcharged prices!\n",
			id->value_buy, id->value_sell, nameid, id->jname);

	id->weight = atoi(str[6]);
#ifdef RENEWAL
	itemdb_re_split_atoi(str[7],&id->atk,&id->matk);
#else
	id->atk = atoi(str[7]);
#endif
	id->def = atoi(str[8]);
	id->range = atoi(str[9]);
	id->slot = atoi(str[10]);

	if (id->slot > MAX_SLOTS)
	{
		ShowWarning("itemdb_parse_dbrow: Item %d (%s) specifies %d slots, but the server only supports up to %d. Using %d slots.\n", nameid, id->jname, id->slot, MAX_SLOTS, MAX_SLOTS);
		id->slot = MAX_SLOTS;
	}

	itemdb_jobid2mapid(id->class_base, (unsigned int)strtoul(str[11],NULL,0));
	id->class_upper = atoi(str[12]);
	id->sex	= atoi(str[13]);
	id->equip = atoi(str[14]);

	if (!id->equip && itemdb_isequip2(id))
	{
		ShowWarning("Item %d (%s) is an equipment with no equip-field! Making it an etc item.\n", nameid, id->jname);
		id->type = IT_ETC;
	}

	if( id->type != IT_SHADOWGEAR && id->equip&EQP_SHADOW_GEAR )
	{
		ShowWarning("Item %d (%s) have invalid equipment slot! Making it an etc item.\n", nameid, id->jname);
		id->type = IT_ETC;
	}

	id->wlv = cap_value(atoi(str[15]), REFINE_TYPE_ARMOR, REFINE_TYPE_MAX);
#ifdef RENEWAL
	itemdb_re_split_atoi(str[16],&id->elv,&id->elvmax);
#else
	id->elv = atoi(str[16]);
#endif
	id->flag.no_refine = atoi(str[17]) ? 0 : 1; //FIXME: verify this
	id->look = atoi(str[18]);

	id->flag.available = 1;
	id->view_id = 0;
	id->sex = itemdb_gendercheck(id); //Apply gender filtering.

	if (id->script) {
		script_free_code(id->script);
		id->script = NULL;
	}
	if (id->equip_script) {
		script_free_code(id->equip_script);
		id->equip_script = NULL;
	}
	if (id->unequip_script) {
		script_free_code(id->unequip_script);
		id->unequip_script = NULL;
	}

	if (*str[19])
		id->script = parse_script(str[19], source, line, scriptopt);
	if (*str[20])
		id->equip_script = parse_script(str[20], source, line, scriptopt);
	if (*str[21])
		id->unequip_script = parse_script(str[21], source, line, scriptopt);

	return true;
}

/**
* Read item from item db
* item_db2 overwriting item_db
*/
static int itemdb_readdb(void){
	const char* filename[] = {
		DBPATH"item_db.txt",
		DBIMPORT"/item_db.txt" 
	};

	int fi;

	for( fi = 0; fi < ARRAYLENGTH(filename); ++fi ) {
		uint32 lines = 0, count = 0;
		char line[1024];

		char path[256];
		FILE* fp;

		sprintf(path, "%s/%s", db_path, filename[fi]);
		fp = fopen(path, "r");
		if( fp == NULL ) {
			ShowWarning("itemdb_readdb: File not found \"%s\", skipping.\n", path);
			continue;
		}

		// process rows one by one
		while(fgets(line, sizeof(line), fp))
		{
			char *str[32], *p;
			int i;
			lines++;
			if(line[0] == '/' && line[1] == '/')
				continue;
			memset(str, 0, sizeof(str));

			p = line;
			while( ISSPACE(*p) )
				++p;
			if( *p == '\0' )
				continue;// empty line
			for( i = 0; i < 19; ++i )
			{
				str[i] = p;
				p = strchr(p,',');
				if( p == NULL )
					break;// comma not found
				*p = '\0';
				++p;
			}

			if( p == NULL )
			{
				ShowError("itemdb_readdb: Insufficient columns in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}

			// Script
			if( *p != '{' )
			{
				ShowError("itemdb_readdb: Invalid format (Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
			str[19] = p;
			p = strstr(p+1,"},");
			if( p == NULL )
			{
				ShowError("itemdb_readdb: Invalid format (Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
			p[1] = '\0';
			p += 2;

			// OnEquip_Script
			if( *p != '{' )
			{
				ShowError("itemdb_readdb: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
			str[20] = p;
			p = strstr(p+1,"},");
			if( p == NULL )
			{
				ShowError("itemdb_readdb: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
			p[1] = '\0';
			p += 2;

			// OnUnequip_Script (last column)
			if( *p != '{' )
			{
				ShowError("itemdb_readdb: Invalid format (OnUnequip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
			str[21] = p;

			if ( str[21][strlen(str[21])-2] != '}' ) {
				/* lets count to ensure it's not something silly e.g. a extra space at line ending */
				int v, lcurly = 0, rcurly = 0;

				for( v = 0; v < strlen(str[21]); v++ ) {
					if( str[21][v] == '{' )
						lcurly++;
					else if ( str[21][v] == '}' )
						rcurly++;
				}

				if( lcurly != rcurly ) {
					ShowError("itemdb_readdb: Mismatching curly braces in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
					continue;
				}
			}

			if (!itemdb_parse_dbrow(str, path, lines, 0))
				continue;

			count++;
		}

		fclose(fp);

		ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, path);
	}

	return 0;
}

/**
* Read item_db table
*/
static int itemdb_read_sqldb(void) {

	const char* item_db_name[] = {
#ifdef RENEWAL
		item_db_re_db,
#else
		item_db_db,
#endif
		item_db2_db
	};
	int fi;

	for( fi = 0; fi < ARRAYLENGTH(item_db_name); ++fi ) {
		uint32 lines = 0, count = 0;

		// retrieve all rows from the item database
		if( SQL_ERROR == Sql_Query(mmysql_handle, "SELECT * FROM `%s`", item_db_name[fi]) ) {
			Sql_ShowDebug(mmysql_handle);
			continue;
		}

		// process rows one by one
		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {// wrap the result into a TXT-compatible format
			char* str[22];
			char* dummy = "";
			int i;
			++lines;
			for( i = 0; i < 22; ++i ) {
				Sql_GetData(mmysql_handle, i, &str[i], NULL);
				if( str[i] == NULL )
					str[i] = dummy; // get rid of NULL columns
			}

			if (!itemdb_parse_dbrow(str, item_db_name[fi], lines, SCRIPT_IGNORE_EXTERNAL_BRACKETS))
				continue;
			++count;
		}

		// free the query result
		Sql_FreeResult(mmysql_handle);

		ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, item_db_name[fi]);
	}

	return 0;
}

/** Unique item ID function
* Only one operation by once
* @param flag
* 0 return new id
* 1 set new value, checked with current value
* 2 set new value bypassing anything
* 3/other
* @param value
* @return last value
*------------------------------------------*/
uint64 itemdb_unique_id(int8 flag, int64 value) {
	static uint64 item_uid = 0;

	if(flag)
	{
		if(flag == 1)
		{	if(item_uid < value)
				return (item_uid = value);
		}else if(flag == 2)
			return (item_uid = value);

		return item_uid;
	}

	return ++item_uid;
}

/**
* Load Unique ID for Item
*/
static void itemdb_uid_load(void){

	char * uid;
	if (SQL_ERROR == Sql_Query(mmysql_handle, "SELECT `value` FROM `interreg` WHERE `varname`='unique_id'"))
		Sql_ShowDebug(mmysql_handle);

	if( SQL_SUCCESS != Sql_NextRow(mmysql_handle) )
	{
		ShowError("itemdb_uid_load: Unable to fetch unique_id data\n");
		Sql_FreeResult(mmysql_handle);
		return;
	}

	Sql_GetData(mmysql_handle, 0, &uid, NULL);
	itemdb_unique_id(1, (uint64)strtoull(uid, NULL, 10));
	Sql_FreeResult(mmysql_handle);
}

/** Check if the item is restricted by item_noequip.txt
* @param id Item that will be checked
* @param m Map ID
* @return true: can't be used; false: can be used
*/
bool itemdb_isNoEquip(struct item_data *id, uint16 m) {
	if (!id->flag.no_equip)
		return false;
	/* on restricted maps the item is consumed but the effect is not used */
	if ((!map_flag_vs(m) && id->flag.no_equip&1) || // Normal
		(map[m].flag.pvp && id->flag.no_equip&2) || // PVP
		(map_flag_gvg(m) && id->flag.no_equip&4) || // GVG
		(map[m].flag.battleground && id->flag.no_equip&8) || // Battleground
		(map[m].flag.restricted && id->flag.no_equip&(8*map[m].zone)) // Zone restriction
		)
		return true;
	return false;
}

/**
* Read all item-related databases
*/
static void itemdb_read(void) {
	int i;
	const char* dbsubpath[] = {
		"",
		"/"DBIMPORT,
	};
	
	if (db_use_sqldbs)
		itemdb_read_sqldb();
	else
		itemdb_readdb();

	memset(&itemgroup_db, 0, sizeof(itemgroup_db));
	
	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		uint8 n1 = strlen(db_path)+strlen(dbsubpath[i])+1;
		uint8 n2 = strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1;
		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);
		

		if(i==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		}
		else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}
		
		sv_readdb(dbsubpath1, "item_avail.txt",         ',', 2, 2, -1, &itemdb_read_itemavail, i);
		sv_readdb(dbsubpath1, "item_stack.txt",         ',', 3, 3, -1, &itemdb_read_stack, i);
		sv_readdb(dbsubpath1, "item_nouse.txt",         ',', 3, 3, -1, &itemdb_read_nouse, i);
		
		itemdb_read_itemgroup(dbsubpath2, i);
		itemdb_read_combos(dbsubpath2,i); //TODO change this to sv_read ? id#script ?
		sv_readdb(dbsubpath2, "item_noequip.txt",       ',', 2, 2, -1, &itemdb_read_noequip, i);
		sv_readdb(dbsubpath2, "item_trade.txt",         ',', 3, 3, -1, &itemdb_read_itemtrade, i);
		sv_readdb(dbsubpath2, "item_delay.txt",         ',', 2, 2, -1, &itemdb_read_itemdelay, i);
		sv_readdb(dbsubpath2, "item_buyingstore.txt",   ',', 1, 1, -1, &itemdb_read_buyingstore, i);
		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}
	itemdb_uid_load();
}

/*==========================================
 * Initialize / Finalize
 *------------------------------------------*/

/**
* Destroys the item_data.
*/
static void destroy_item_data(struct item_data* self, bool free_self) {
	if( self == NULL )
		return;
	// free scripts
	if( self->script )
		script_free_code(self->script);
	if( self->equip_script )
		script_free_code(self->equip_script);
	if( self->unequip_script )
		script_free_code(self->unequip_script);
	if( self->combos_count ) {
		int i;
		for( i = 0; i < self->combos_count; i++ ) {
			if( !self->combos[i]->isRef ) {
				aFree(self->combos[i]->nameid);
				script_free_code(self->combos[i]->script);
			}
			aFree(self->combos[i]);
		}
		aFree(self->combos);
	}
#if defined(DEBUG)
	// trash item
	memset(self, 0xDD, sizeof(struct item_data));
#endif
	// free self
	if( free_self )
		aFree(self);
}

/**
 * @see DBApply
 */
static int itemdb_final_sub(DBKey key, DBData *data, va_list ap)
{
	struct item_data *id = db_data2ptr(data);

	if( id != &dummy_item )
		destroy_item_data(id, true);

	return 0;
}

/**
* Reload Item DB
*/
void itemdb_reload(void) {
	struct s_mapiterator* iter;
	struct map_session_data* sd;

	int i,d,k;

	// clear the previous itemdb data
	for( i = 0; i < ARRAYLENGTH(itemdb_array); ++i )
		if( itemdb_array[i] )
			destroy_item_data(itemdb_array[i], true);

	for (i = 0; i < MAX_ITEMGROUP; i++) {
		uint8 j;
		if (!(&itemgroup_db[i]))
			continue;
		if (itemgroup_db[i].must_qty)
			aFree(itemgroup_db[i].must);
		for (j = 0; j < MAX_ITEMGROUP_RANDGROUP; j++) {
			if (!(&itemgroup_db[i].random[j]) || !itemgroup_db[i].random[j].data_qty)
				continue;
			aFree(itemgroup_db[i].random[j].data);
		}
	}

	itemdb_other->clear(itemdb_other, itemdb_final_sub);
	db_clear(itemdb_combo);

	memset(itemdb_array, 0, sizeof(itemdb_array));

	// read new data
	itemdb_read();
	cashshop_reloaddb();

	//Epoque's awesome @reloaditemdb fix - thanks! [Ind]
	//- Fixes the need of a @reloadmobdb after a @reloaditemdb to re-link monster drop data
	for( i = 0; i < MAX_MOB_DB; i++ ) {
		struct mob_db *entry;
		if( !((i < 1324 || i > 1363) && (i < 1938 || i > 1946)) )
			continue;
		entry = mob_db(i);
		for(d = 0; d < MAX_MOB_DROP; d++) {
			struct item_data *id;
			if( !entry->dropitem[d].nameid )
				continue;
			id = itemdb_search(entry->dropitem[d].nameid);

			for (k = 0; k < MAX_SEARCH; k++) {
				if (id->mob[k].chance <= entry->dropitem[d].p)
					break;
			}

			if (k == MAX_SEARCH)
				continue;

			if (id->mob[k].id != i)
				memmove(&id->mob[k+1], &id->mob[k], (MAX_SEARCH-k-1)*sizeof(id->mob[0]));
			id->mob[k].chance = entry->dropitem[d].p;
			id->mob[k].id = i;
		}
	}

	// readjust itemdb pointer cache for each player
	iter = mapit_geteachpc();
	for( sd = (struct map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (struct map_session_data*)mapit_next(iter) ) {
		memset(sd->item_delay, 0, sizeof(sd->item_delay));  // reset item delays
		pc_setinventorydata(sd);
		pc_check_available_item(sd); // Check for invalid(ated) items.
		/* clear combo bonuses */
		if( sd->combos.count ) {
			aFree(sd->combos.bonus);
			aFree(sd->combos.id);
			sd->combos.bonus = NULL;
			sd->combos.id = NULL;
			sd->combos.count = 0;
			if( pc_load_combo(sd) > 0 )
				status_calc_pc(sd,0);
		}

	}
	mapit_free(iter);
}


/**
* Finalizing Item DB
*/
void do_final_itemdb(void) {
	int i;

	for( i = 0; i < ARRAYLENGTH(itemdb_array); ++i )
		if( itemdb_array[i] )
			destroy_item_data(itemdb_array[i], true);

	for (i = 0; i < MAX_ITEMGROUP; i++) {
		uint8 j;
		if (!(&itemgroup_db[i]))
			continue;
		if (itemgroup_db[i].must_qty)
			aFree(itemgroup_db[i].must);
		for (j = 0; j < MAX_ITEMGROUP_RANDGROUP; j++) {
			if (!(&itemgroup_db[i].random[j]) || !itemgroup_db[i].random[j].data_qty)
				continue;
			aFree(itemgroup_db[i].random[j].data);
		}
	}

	itemdb_other->destroy(itemdb_other, itemdb_final_sub);
	destroy_item_data(&dummy_item, false);
	db_destroy(itemdb_combo);
}

/**
* Initializing Item DB
*/
void do_init_itemdb(void) {
	memset(itemdb_array, 0, sizeof(itemdb_array));
	itemdb_other = idb_alloc(DB_OPT_BASE);
	itemdb_combo = idb_alloc(DB_OPT_BASE);
	create_dummy_data(); //Dummy data item.
	
	itemdb_read();
}
