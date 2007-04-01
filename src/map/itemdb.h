// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ITEMDB_H_
#define _ITEMDB_H_

#include "mmo.h" // ITEM_NAME_LENGTH

#define MAX_RANDITEM	10000

#define MAX_SEARCH	5  //Designed for search functions, species max number of matches to display.


enum item_types {
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
};

enum item_group_list {
	IG_BLUEBOX=1,	//1
	IG_VIOLETBOX,	//2
	IG_CARDALBUM,	//3
	IG_GIFTBOX,	//4
	IG_SCROLLBOX,	//5
	IG_FINDINGORE,	//6
	IG_COOKIEBAG,	//7
	IG_FIRSTAID,	//8
	IG_HERB,	//9
	IG_FRUIT,	//10
	IG_MEAT,	//11
	IG_CANDY,	//12
	IG_JUICE,	//13
	IG_FISH,	//14
	IG_BOX,	//15
	IG_GEMSTONE,	//16
	IG_RESIST,	//17
	IG_ORE,	//18
	IG_FOOD,	//19
	IG_RECOVERY,	//20
	IG_MINERAL,	//21
	IG_TAMING,	//22
	IG_SCROLL,	//23
	IG_QUIVER,	//24
	IG_MASK,	//25
	IG_ACCESORY,	//26
	IG_JEWEL,	//27
	IG_GIFTBOX_1,	//28
	IG_GIFTBOX_2,	//29
	IG_GIFTBOX_3,	//30
	IG_GIFTBOX_4,	//31
	IG_EGGBOY,	//32
	IG_EGGGIRL,	//33
	IG_GIFTBOXCHINA,	//34
	IG_LOTTOBOX,	//35
	IG_FOODBAG,	//36
	IG_POTION,	//37
	MAX_ITEMGROUP,
};

#define CARD0_FORGE 0x00FF
#define CARD0_CREATE 0x00FE
#define CARD0_PET ((short)0xFF00)

//Marks if the card0 given is "special" (non-item id used to mark pets/created items. [Skotlex]
#define itemdb_isspecial(i) (i == CARD0_FORGE || i == CARD0_CREATE || i == CARD0_PET)

//Use apple for unknown items.
#define UNKNOWN_ITEM_ID 512

struct item_data {
	int nameid;
	char name[ITEM_NAME_LENGTH],jname[ITEM_NAME_LENGTH];
	//Do not add stuff between value_buy and wlv (see how getiteminfo works)
	int value_buy;
	int value_sell;
	int type;
	int maxchance; //For logs, for external game info, for scripts: Max drop chance of this item (e.g. 0.01% , etc.. if it = 0, then monsters don't drop it) [Lupus]
	int sex;
	int equip;
	int weight;
	int atk;
	int def;
	int range;
	int slot;
	int look;
	int elv;
	int wlv;
	int view_id;
//Lupus: I rearranged order of these fields due to compatibility with ITEMINFO script command
//		some script commands should be revised as well...
	unsigned int class_base[3];	//Specifies if the base can wear this item (split in 3 indexes per type: 1-1, 2-1, 2-2)
	unsigned class_upper : 3; //Specifies if the upper-type can equip it (1: normal, 2: upper, 3: baby)
	struct {
		unsigned short chance;
		int id;
	} mob[MAX_SEARCH]; //Holds the mobs that have the highest drop rate for this item. [Skotlex]
	struct script_code *script;	//Default script for everything.
	struct script_code *equip_script;	//Script executed once when equipping.
	struct script_code *unequip_script;//Script executed once when unequipping.
	struct {
		unsigned available : 1;
		unsigned value_notdc : 1;
		unsigned value_notoc : 1;
		short no_equip;
		unsigned no_refine : 1;	// [celest]
		unsigned delay_consume : 1;	// Signifies items that are not consumed inmediately upon double-click [Skotlex]
		unsigned trade_restriction : 7;	//Item restrictions mask [Skotlex]
		unsigned autoequip: 1;
	} flag;
	short gm_lv_trade_override;	//GM-level to override trade_restriction
};

struct item_group {
	int nameid[MAX_RANDITEM];
	int qty; //Counts amount of items in the group.
};

struct item_data* itemdb_searchname(const char *name);
int itemdb_searchname_array(struct item_data** data, int size, const char *str);
struct item_data* itemdb_load(int nameid);
struct item_data* itemdb_search(int nameid);
struct item_data* itemdb_exists(int nameid);
#define itemdb_name(n) itemdb_search(n)->name
#define itemdb_jname(n) itemdb_search(n)->jname
#define itemdb_type(n) itemdb_search(n)->type
#define itemdb_atk(n) itemdb_search(n)->atk
#define itemdb_def(n) itemdb_search(n)->def
#define itemdb_look(n) itemdb_search(n)->look
#define itemdb_weight(n) itemdb_search(n)->weight
#define itemdb_equip(n) itemdb_search(n)->equip
#define itemdb_usescript(n) itemdb_search(n)->script
#define itemdb_equipscript(n) itemdb_search(n)->script
#define itemdb_wlv(n) itemdb_search(n)->wlv
#define itemdb_range(n) itemdb_search(n)->range
#define itemdb_slot(n) itemdb_search(n)->slot
#define itemdb_available(n) (itemdb_exists(n) && itemdb_search(n)->flag.available)
#define itemdb_viewid(n) (itemdb_search(n)->view_id)
#define itemdb_autoequip(n) (itemdb_search(n)->flag.autoequip)
int itemdb_group_bonus(const int itemgrouphealrate[MAX_ITEMGROUP], int itemid);

int itemdb_searchrandomid(int flags);

#define itemdb_value_buy(n) itemdb_search(n)->value_buy
#define itemdb_value_sell(n) itemdb_search(n)->value_sell
#define itemdb_value_notdc(n) itemdb_search(n)->flag.value_notdc
#define itemdb_value_notoc(n) itemdb_search(n)->flag.value_notoc
#define itemdb_canrefine(n) itemdb_search(n)->flag.no_refine
//Item trade restrictions [Skotlex]
int itemdb_isdropable_sub(struct item_data *, int, int);
int itemdb_cantrade_sub(struct item_data*, int, int);
int itemdb_canpartnertrade_sub(struct item_data*, int, int);
int itemdb_cansell_sub(struct item_data*,int, int);
int itemdb_cancartstore_sub(struct item_data*, int, int);
int itemdb_canstore_sub(struct item_data*, int, int);
int itemdb_canguildstore_sub(struct item_data*, int, int);
int itemdb_isrestricted(struct item* item, int gmlv, int gmlv2, int (*func)(struct item_data*, int, int));
#define itemdb_isdropable(item, gmlv) itemdb_isrestricted(item, gmlv, 0, itemdb_isdropable_sub)
#define itemdb_cantrade(item, gmlv, gmlv2) itemdb_isrestricted(item, gmlv, gmlv2, itemdb_cantrade_sub)
#define itemdb_canpartnertrade(item, gmlv, gmlv2) itemdb_isrestricted(item, gmlv, gmlv2, itemdb_canpartnertrade_sub)
#define itemdb_cansell(item, gmlv) itemdb_isrestricted(item, gmlv, 0, itemdb_cansell_sub)
#define itemdb_cancartstore(item, gmlv)  itemdb_isrestricted(item, gmlv, 0, itemdb_cancartstore_sub)
#define itemdb_canstore(item, gmlv) itemdb_isrestricted(item, gmlv, 0, itemdb_canstore_sub) 
#define itemdb_canguildstore(item, gmlv) itemdb_isrestricted(item , gmlv, 0, itemdb_canguildstore_sub) 

int itemdb_isequip(int);
int itemdb_isequip2(struct item_data *);
int itemdb_isidentified(int);
int itemdb_isstackable(int);
int itemdb_isstackable2(struct item_data *);

void itemdb_reload(void);

void do_final_itemdb(void);
int do_init_itemdb(void);

#endif /* _ITEMDB_H_ */
