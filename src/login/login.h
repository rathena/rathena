// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_H_
#define _LOGIN_H_

#include "../common/mmo.h"

#define MAX_SERVERS 30

#define LOGIN_CONF_NAME "conf/login_athena.conf"
#define LAN_CONF_NAME "conf/subnet_athena.conf"
#define PASSWORDENC 3	// A definition is given when making an encryption password correspond.
                     	// It is 1 at the time of passwordencrypt.
                     	// It is made into 2 at the time of passwordencrypt2.
                     	// When it is made 3, it corresponds to both.

extern int login_port;
struct mmo_account {
	int version;	//Added for version check [Sirius]
	char userid[NAME_LENGTH];
	char passwd[NAME_LENGTH];
	int passwdenc;

	long account_id;
	long login_id1;
	long login_id2;
	long char_id;
	char lastlogin[24];
	int sex;
};

struct mmo_char_server {
	char name[21];
	long ip;
	short port;
	int users;
	int maintenance;
	int new_;
};

extern struct mmo_char_server server[MAX_SERVERS];
extern int server_fd[MAX_SERVERS];
#endif
