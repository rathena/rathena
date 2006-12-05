// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STORAGE_SQL_H_
#define _INT_STORAGE_SQL_H_

int inter_storage_sql_init(void);
void inter_storage_sql_final(void);
int inter_storage_delete(int account_id);
int inter_guild_storage_delete(int guild_id);

int inter_storage_parse_frommap(int fd);

//Exported for use in the TXT-SQL converter.
int storage_tosql(int account_id,struct storage *p);
int guild_storage_tosql(int guild_id, struct guild_storage *p);
#endif
