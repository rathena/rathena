#ifndef _LOGIN_H_
#define _LOGIN_H_

#define MAX_SERVERS 30

#define LOGIN_CONF_NAME	"conf/login_athena.conf"
#define SQL_CONF_NAME "conf/inter_athena.conf"
#define LAN_CONF_NAME     "conf/lan_support.conf"

#define PASSWORDENC		3	// A definition is given when making an encryption password correspond.
							// It is 1 at the time of passwordencrypt.
							// It is made into 2 at the time of passwordencrypt2.
							// When it is made 3, it corresponds to both.

#define START_ACCOUNT_NUM	  2000000
#define END_ACCOUNT_NUM		100000000

//add include for DBMS(mysql)
#include <mysql.h>

#include "../common/socket.h"
#include "../common/core.h"
#include "../common/mmo.h"
#include "../common/version.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/nullpo.h"

#include "strlib.h"

#ifdef LCCWIN32
#include <winsock.h>
#pragma lib <libmysql.lib>
extern void gettimeofday(struct timeval *t, struct timezone *dummy);
#else
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <time.h>
void Gettimeofday(struct timeval *timenow)
{
	time_t t;
	t = clock();
	timenow->tv_usec = t;
	timenow->tv_sec = t / CLK_TCK;
	return;
}
#define gettimeofday(timenow, dummy) Gettimeofday(timenow)
#pragma comment(lib,"libmysql.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#endif

// Auth Data
#define AUTH_FIFO_SIZE 256
extern struct auth_fifo {
	int account_id,login_id1,login_id2;
	int ip,sex,delflag;
} auth_fifo[AUTH_FIFO_SIZE];

extern int auth_fifo_pos;

// MySQL Query
extern char tmpsql[65535], prev_query[65535];

// MySQL Connection Handle
extern MYSQL mysql_handle;
extern MYSQL_RES* 	sql_res ;
extern MYSQL_ROW	sql_row ;

// MySQL custom table and column names 
extern char login_db[32];
extern char loginlog_db[32];
extern char login_db_account_id[32];
extern char login_db_userid[32];
extern char login_db_user_pass[32];
extern char login_db_level[32];

// MD5 Key Data for encrypted login
extern int md5keylen;
extern char md5key[20];

// Dynamic IP Ban config
extern int ipban;
extern int dynamic_account_ban;
extern int dynamic_account_ban_class;
extern int dynamic_pass_failure_ban;
extern int dynamic_pass_failure_ban_time;
extern int dynamic_pass_failure_ban_how_many;
extern int dynamic_pass_failure_ban_how_long;

// Date format for bans
extern char date_format[32];

// minimum level of player/GM (0: player, 1-99: gm) to connect on the server
extern int min_level_to_connect; 

// It's to check IP of a player between login-server and char-server (part of anti-hacking system)
extern int check_ip_flag; 

// Login's FD
extern int login_fd;

// LAN IP of char-server and subnet mask(Kashy)
extern char lan_char_ip[16];
extern int subnetmaski[4];

// Char-server data
extern struct mmo_char_server server[MAX_SERVERS];
extern int server_fd[MAX_SERVERS];
extern unsigned char servers_connected;

// Anti-freeze Data
extern int server_freezeflag[MAX_SERVERS];
extern int anti_freeze_enable;
extern int ANTI_FREEZE_INTERVAL;

struct mmo_account {
	char* userid;
	char* passwd;
	int passwdenc;

	long account_id;
	long login_id1;
	long login_id2;
	long char_id;
	char lastlogin[24];
	int sex;

	time_t ban_until_time;
};

struct mmo_char_server {
	char name[20];
	long ip;
	short port;
	int users;
	int maintenance;
	int new;
};

void sql_query(char*,char*);
int e_mail_check(unsigned char*);
void add_online_user(int);
int is_user_online(int);
void remove_online_user(int);
int mmo_auth( struct mmo_account*, int);
unsigned char isGM(int);
int lan_ip_check(unsigned char *);


#endif
