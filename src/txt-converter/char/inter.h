// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_H_
#define _INTER_H_

struct accreg;

int inter_init_txt(const char *file);
void inter_final(void);
int inter_save(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason);

int inter_check_length(int fd,int length);

int inter_log(char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

extern unsigned int party_share_level;
extern char inter_log_filename[1024];
extern char main_chat_nick[16];

//For TXT->SQL conversion
extern char accreg_txt[];
int inter_accreg_fromstr(const char *str, struct accreg *reg);

#endif /* _INTER_H_ */
