// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_GUILD_H_
#define _INT_GUILD_H_

struct guild;
struct guild_castle;

int inter_guild_init(void);
void inter_guild_final(void);
int inter_guild_save(void);
int inter_guild_parse_frommap(int fd);
struct guild *inter_guild_search(int guild_id);
int inter_guild_mapif_init(int fd);
int inter_guild_leave(int guild_id,int account_id,int char_id);
int inter_guild_sex_changed(int guild_id,int account_id,int char_id, short gender);

extern char guild_txt[1024];
extern char castle_txt[1024];

//For the TXT->SQL converter
int inter_guild_fromstr(char *str, struct guild *g);
int inter_guildcastle_fromstr(char *str, struct guild_castle *gc);

#endif /* _INT_GUILD_H_ */
