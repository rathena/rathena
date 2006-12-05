// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_H_
#define _LOGIN_H_

#define MAX_SERVERS 30

#define LOGIN_CONF_NAME	"conf/login_athena.conf"
#define SQL_CONF_NAME "conf/inter_athena.conf"
#define LAN_CONF_NAME "conf/subnet_athena.conf"

#ifndef SQL_DEBUG

#define mysql_query(_x, _y) mysql_real_query(_x, _y, strlen(_y)) //supports ' in names and runs faster [Kevin]

#else 

#define mysql_query(_x, _y)  debug_mysql_query(__FILE__, __LINE__, _x, _y)

#endif

#define PASSWORDENC		3	// A definition is given when making an encryption password correspond.
							// It is 1 at the time of passwordencrypt.
							// It is made into 2 at the time of passwordencrypt2.
							// When it is made 3, it corresponds to both.

#define START_ACCOUNT_NUM	  2000000
#define END_ACCOUNT_NUM		100000000

struct mmo_account {
	int version;	//Added by sirius for versioncheck
	char userid[NAME_LENGTH];
	char passwd[NAME_LENGTH];
	int passwdenc;
	
	
	long account_id;
	long login_id1;
	long login_id2;
	long char_id;
	char lastlogin[24];
	int sex;
	int level; // added [zzo]
};

struct mmo_char_server {
	char name[20];
	long ip;
	short port;
	int users;
	int maintenance;
	int new_;
};


#endif
