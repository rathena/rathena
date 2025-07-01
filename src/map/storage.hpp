// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <memory>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

struct s_storage;
struct item;
class map_session_data;

extern std::unordered_map<uint16, std::shared_ptr<struct s_storage_table>> storage_db;

enum e_storage_add {
	STORAGE_ADD_OK,
	STORAGE_ADD_NOROOM,
	STORAGE_ADD_NOACCESS,
	STORAGE_ADD_INVALID,
};

/// Guild storage flags
enum e_guild_storage_flags : uint8 {
	GSTORAGE_OPEN = 0,
	GSTORAGE_STORAGE_ALREADY_OPEN,
	GSTORAGE_ALREADY_OPEN,
	GSTORAGE_NO_GUILD,
	GSTORAGE_NO_STORAGE,
	GSTORAGE_NO_PERMISSION
};

enum e_guild_storage_log : uint16 {
	GUILDSTORAGE_LOG_SUCCESS,
	GUILDSTORAGE_LOG_FINAL_SUCCESS,
	GUILDSTORAGE_LOG_EMPTY,
	GUILDSTORAGE_LOG_FAILED,
};

struct guild_log_entry{
	uint32 id;
	char name[NAME_LENGTH];
	char time[NAME_LENGTH];
	struct item item;
	int16 amount;
};

const char *storage_getName(uint8 id);
bool storage_exists(uint8 id);

int32 storage_delitem(map_session_data* sd, struct s_storage *stor, int32 index, int32 amount);
int32 storage_storageopen(map_session_data *sd);
void storage_storageadd(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount);
void storage_storageget(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount, bool favorite=false);
void storage_storageaddfromcart(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount);
void storage_storagegettocart(map_session_data *sd, struct s_storage *stor, int32 index, int32 amount);
void storage_storagesave(map_session_data *sd);
void storage_storageclose(map_session_data *sd);
void storage_sortitem(struct item* items, uint32 size);
void do_init_storage(void);
void do_final_storage(void);
void do_reconnect_storage(void);
void storage_storage_quit(map_session_data *sd, int32 flag);

struct s_storage* guild2storage(int32 guild_id);
struct s_storage* guild2storage2(int32 guild_id);
void storage_guild_delete(int32 guild_id);
char storage_guild_storageopen(map_session_data *sd);
enum e_guild_storage_log storage_guild_log_read( map_session_data* sd );
bool storage_guild_additem(map_session_data *sd,struct s_storage *stor,struct item *item_data,int32 amount);
bool storage_guild_additem2(struct s_storage* stor, struct item* item, int32 amount);
bool storage_guild_delitem(map_session_data *sd,struct s_storage *stor,int32 n,int32 amount);
void storage_guild_storageadd(map_session_data *sd,int32 index,int32 amount);
void storage_guild_storageget(map_session_data *sd,int32 index,int32 amount, bool favorite=false);
void storage_guild_storageaddfromcart(map_session_data *sd,int32 index,int32 amount);
void storage_guild_storagegettocart(map_session_data *sd,int32 index,int32 amount);
void storage_guild_storageclose(map_session_data *sd);
void storage_guild_storage_quit(map_session_data *sd,int32 flag);
bool storage_guild_storagesave(uint32 account_id, int32 guild_id, int32 flag);
void storage_guild_storagesaved(int32 guild_id); //Ack from char server that guild store was saved.

// Premium Storage [Cydh]
void storage_premiumStorage_open(map_session_data *sd);
bool storage_premiumStorage_load(map_session_data *sd, uint8 num, uint8 mode);
void storage_premiumStorage_save(map_session_data *sd);
void storage_premiumStorage_close(map_session_data *sd);
void storage_premiumStorage_quit(map_session_data *sd);

int32 compare_item(struct item *a, struct item *b);

#endif /* STORAGE_HPP */
