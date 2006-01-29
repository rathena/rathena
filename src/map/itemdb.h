// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ITEMDB_H_
#define _ITEMDB_H_

#include "map.h"

struct item_data {
	int nameid;
	char name[ITEM_NAME_LENGTH],jname[ITEM_NAME_LENGTH];
	char prefix[NAME_LENGTH],suffix[NAME_LENGTH];
	char cardillustname[64];
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
//Lupus: I rearranged order of these fields due to compatibility with ITEMINFO script command
//		some script commands should be revised as well...
	unsigned short class_base[3];	//Specifies if the base can wear this item (split in 3 indexes per type: 1-1, 2-1, 2-2)
	unsigned class_upper : 3; //Specifies if the upper-type can equip it (1: normal, 2: upper, 3: baby)
	unsigned char *script;	// 攻撃,防御の属性設定もこの中で可能かな?
	struct {
		unsigned available : 1;
		unsigned value_notdc : 1;
		unsigned value_notoc : 1;
		unsigned no_equip : 3;
		unsigned no_use : 1;
		unsigned no_refine : 1;	// [celest]
		unsigned delay_consume : 1;	// Signifies items that are not consumed inmediately upon double-click [Skotlex]
		unsigned trade_restriction : 7;	//Item restrictions mask [Skotlex]
	} flag;
	short gm_lv_trade_override;	//GM-level to override trade_restriction
	int view_id;
};

struct random_item_data {
	int nameid;
	int per;
};

struct item_group {
	int qty; //Counts amount of items in the group.
	int id[30];	// 120 bytes
};

struct item_data* itemdb_searchname(const char *name);
struct item_data* itemdb_search(int nameid);
struct item_data* itemdb_exists(int nameid);
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
int itemdb_group(int nameid);

int itemdb_searchrandomid(int flags);
int itemdb_searchrandomgroup(int groupid);

#define itemdb_value_buy(n) itemdb_search(n)->value_buy
#define itemdb_value_sell(n) itemdb_search(n)->value_sell
#define itemdb_value_notdc(n) itemdb_search(n)->flag.value_notdc
#define itemdb_value_notoc(n) itemdb_search(n)->flag.value_notoc
#define itemdb_canrefine(n) itemdb_search(n)->flag.no_refine
//Item trade restrictions [Skotlex]
int itemdb_isdropable(int nameid, int gmlv);
int itemdb_cantrade(int nameid, int gmlv, int gmlv2);
int itemdb_cansell(int nameid, int gmlv);
int itemdb_canstore(int nameid, int gmlv);
int itemdb_canguildstore(int nameid, int gmlv);
int itemdb_cancartstore(int nameid, int gmlv);
int itemdb_canpartnertrade(int nameid, int gmlv, int gmlv2);

int itemdb_isequip(int);
int itemdb_isequip2(struct item_data *);
int itemdb_isequip3(int);

// itemdb_equipマクロとitemdb_equippointとの違いは
// 前者が鯖側dbで定義された値そのものを返すのに対し
// 後者はsessiondataを考慮した鞍側での装備可能場所
// すべての組み合わせを返す

void itemdb_reload(void);

void do_final_itemdb(void);
int do_init_itemdb(void);

#endif
