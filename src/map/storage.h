// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STORAGE_H_
#define _STORAGE_H_

//#include "../common/mmo.h"
struct s_storage;
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

struct s_storage* guild2storage(int guild_id);
struct s_storage* guild2storage2(int guild_id);
void storage_guild_delete(int guild_id);
char storage_guild_storageopen(struct map_session_data *sd);
bool storage_guild_additem(struct map_session_data *sd,struct s_storage *stor,struct item *item_data,int amount);
bool storage_guild_additem2(struct s_storage* stor, struct item* item, int amount);
bool storage_guild_delitem(struct map_session_data *sd,struct s_storage *stor,int n,int amount);
void storage_guild_storageadd(struct map_session_data *sd,int index,int amount);
void storage_guild_storageget(struct map_session_data *sd,int index,int amount);
void storage_guild_storageaddfromcart(struct map_session_data *sd,int index,int amount);
void storage_guild_storagegettocart(struct map_session_data *sd,int index,int amount);
void storage_guild_storageclose(struct map_session_data *sd);
void storage_guild_storage_quit(struct map_session_data *sd,int flag);
bool storage_guild_storagesave(uint32 account_id, int guild_id, int flag);
void storage_guild_storagesaved(int guild_id); //Ack from char server that guild store was saved.

int compare_item(struct item *a, struct item *b);

#endif /* _STORAGE_H_ */
