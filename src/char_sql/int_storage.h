#ifndef _INT_STORAGE_H_
#define _INT_STORAGE_H_

int inter_storage_sql_init();
void inter_storage_sql_final();
int inter_storage_delete(int account_id);
int inter_guild_storage_delete(int guild_id);

int inter_storage_parse_frommap(int fd);


//extern char storage_txt[256];

#endif
