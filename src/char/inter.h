// $Id: inter.h,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#ifndef _INTER_H_
#define _INTER_H_

int inter_init(const char *file);
void inter_final();
int inter_save();
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);

int inter_check_length(int fd,int length);

int inter_log(char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

extern int party_share_level;
extern char inter_log_filename[1024];
extern int log_inter;

#endif
