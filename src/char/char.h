// $Id: char.h,v 1.1.1.1 2004/09/10 17:26:50 MagicalTux Exp $
#ifndef _CHAR_H_
#define _CHAR_H_

#define MAX_MAP_SERVERS 30

#define CHAR_CONF_NAME	"conf/char_athena.conf"

#define LOGIN_LAN_CONF_NAME	"conf/lan_support.conf"

#define DEFAULT_AUTOSAVE_INTERVAL 300*1000

struct mmo_map_server{
	long ip;
	short port;
	int users;
	char map[MAX_MAP_PER_SERVER][16];
};

int search_character_index(char* character_name);
char * search_character_name(int index);

int mapif_sendall(unsigned char *buf, unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf, unsigned int len);
int mapif_send(int fd,unsigned char *buf, unsigned int len);

int char_log(char *fmt, ...);

extern int autosave_interval;
extern char db_path[];

#endif
