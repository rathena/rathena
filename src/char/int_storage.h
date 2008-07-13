// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STORAGE_H_
#define _INT_STORAGE_H_

struct storage_data;
struct guild_storage;

int inter_storage_init(void);
void inter_storage_final(void);
int inter_storage_save(void);
int inter_guild_storage_save(void);
int inter_storage_delete(int account_id);
int inter_guild_storage_delete(int guild_id);
int inter_storage_parse_frommap(int fd);

extern char storage_txt[1024];
extern char guild_storage_txt[1024];

//Exported for use in the TXT-SQL converter.
int storage_fromstr(char *str,struct storage_data *p);
int guild_storage_fromstr(char *str,struct guild_storage *p);

bool storage_load(int account_id, struct storage_data* storage);
bool storage_save(int account_id, struct storage_data* storage);

#endif /* _INT_STORAGE_H_ */
