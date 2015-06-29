// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STORAGE_H_
#define _STORAGE_H_

//#include "../common/mmo.h"
struct storage_data;
struct guild_storage;
struct item;
//#include "map.h"
struct map_session_data;

int storage_delitem(struct map_session_data* sd, int n, int amount);
int storage_storageopen(struct map_session_data *sd);
void storage_storageadd(struct map_session_data *sd,int index,int amount);
void storage_storageget(struct map_session_data *sd,int index,int amount);
void storage_storageaddfromcart(struct map_session_data *sd,int index,int amount);
void storage_storagegettocart(struct map_session_data *sd,int index,int amount);
void storage_storageclose(struct map_session_data *sd);
void storage_sortitem(struct item* items, unsigned int size);
void do_init_storage(void);
void do_final_storage(void);
void do_reconnect_storage(void);
void storage_storage_quit(struct map_session_data *sd, int flag);

struct guild_storage* gstorage_guild2storage(int guild_id);
struct guild_storage *gstorage_get_storage(int guild_id);
void gstorage_delete(int guild_id);
char gstorage_storageopen(struct map_session_data *sd);
bool gstorage_additem(struct map_session_data *sd,struct guild_storage *stor,struct item *item,int amount);
bool gstorage_additem2(struct guild_storage *stor, struct item *item, int amount);
bool gstorage_delitem(struct map_session_data *sd,struct guild_storage *stor,int n,int amount);
void gstorage_storageadd(struct map_session_data *sd,int index,int amount);
void gstorage_storageget(struct map_session_data *sd,int index,int amount);
void gstorage_storageaddfromcart(struct map_session_data *sd,int index,int amount);
void gstorage_storagegettocart(struct map_session_data *sd,int index,int amount);
void gstorage_storageclose(struct map_session_data *sd);
void gstorage_storage_quit(struct map_session_data *sd,int flag);
bool gstorage_storagesave(uint32 account_id, int guild_id, int flag);
void gstorage_storagesaved(int guild_id);

int compare_item(struct item *a, struct item *b);

#endif /* _STORAGE_H_ */
