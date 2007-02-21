// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ITEMDB_H_
#define _ITEMDB_H_

#include "mmo.h"

//FIXME: Maybe it would be better to move this enum to mmo.h,
//instead of having it twice on the map server and here? [Skotlex]
enum {
	IT_HEALING = 0,
	IT_UNKNOWN, //1
	IT_USABLE,  //2
	IT_ETC,     //3
	IT_WEAPON,  //4
	IT_ARMOR,   //5
	IT_CARD,    //6
	IT_PETEGG,  //7
	IT_PETARMOR,//8
	IT_UNKNOWN2,//9
	IT_AMMO,    //10
	IT_DELAYCONSUME,//11
	IT_MAX 
} item_types;

struct item_data {
	int nameid;
	char name[ITEM_NAME_LENGTH],jname[ITEM_NAME_LENGTH];
	int type;
};

extern char item_db_db[256];
extern char item_db2_db[256];
struct item_data* itemdb_search(int nameid);
#define itemdb_type(n) itemdb_search(n)->type

int itemdb_isequip(int);
int itemdb_isequip2(struct item_data *);

void do_final_itemdb(void);
int do_init_itemdb(void);

#endif /* _ITEMDB_H_ */
