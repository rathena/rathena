// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_SQL_H_
#define _LOGIN_SQL_H_

#define MAX_SERVERS 30

#define LOGIN_CONF_NAME	"conf/login_athena.conf"
#define SQL_CONF_NAME "conf/inter_athena.conf"
#define LAN_CONF_NAME "conf/subnet_athena.conf"

// supported encryption types: 1- passwordencrypt, 2- passwordencrypt2, 3- both
#define PASSWORDENC 3

struct mmo_account {
	int version;
	char userid[NAME_LENGTH];
	char passwd[NAME_LENGTH];
	int passwdenc;
	
	int account_id;
	int char_id;
	long login_id1;
	long login_id2;
	char lastlogin[24];
	int sex;
	int level;
};

struct mmo_char_server {
	char name[20];
	uint32 ip;
	uint16 port;
	int users;
	int maintenance;
	int new_;
};

#endif /* _LOGIN_SQL_H_ */
