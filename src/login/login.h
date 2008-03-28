// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_H_
#define _LOGIN_H_

#include "../common/mmo.h" // NAME_LENGTH

#define LOGIN_CONF_NAME "conf/login_athena.conf"
#define LAN_CONF_NAME "conf/subnet_athena.conf"

// supported encryption types: 1- passwordencrypt, 2- passwordencrypt2, 3- both
#define PASSWORDENC 3

struct mmo_account {
	int version;
	char userid[NAME_LENGTH];
	char passwd[NAME_LENGTH];
	int passwdenc;

	int account_id;
	long login_id1;
	long login_id2;
	char lastlogin[24];
	char sex;
	uint8 level;
};

struct mmo_char_server {
	char name[20];
	int fd;
	uint32 ip;
	uint16 port;
	uint16 users;		// user count on this server
	uint16 maintenance;	// in maintenance mode?
	uint16 new_;		// allows creating new chars?
};

extern struct Login_Config {

	uint32 login_ip;                                // the address to bind to
	uint16 login_port;                              // the port to bind to
	unsigned int ip_sync_interval;                  // interval (in minutes) to execute a DNS/IP update (for dynamic IPs)
	bool log_login;                                 // whether to log login server actions or not
	char date_format[32];                           // date format used in messages
	bool console;                                   // console input system enabled?
	bool new_account_flag;                          // autoregistration via _M/_F ?
	int start_limited_time;                         // new account expiration time (-1: unlimited)
//	bool case_sensitive;                            // are logins case sensitive ?
	bool use_md5_passwds;                           // work with password hashes instead of plaintext passwords?
//	bool login_gm_read;                             // should the login server handle info about gm accounts?
	int min_level_to_connect;                       // minimum level of player/GM (0: player, 1-99: GM) to connect
	bool online_check;                              // reject incoming players that are already registered as online ?
	bool check_client_version;                      // check the clientversion set in the clientinfo ?
	int client_version_to_connect;                  // the client version needed to connect (if checking is enabled)

//	bool ipban;                                     // perform IP blocking (via contents of `ipbanlist`) ?
//	bool dynamic_pass_failure_ban;                  // automatic IP blocking due to failed login attemps ?
//	unsigned int dynamic_pass_failure_ban_interval; // how far to scan the loginlog for password failures
//	unsigned int dynamic_pass_failure_ban_limit;    // number of failures needed to trigger the ipban
//	unsigned int dynamic_pass_failure_ban_duration; // duration of the ipban
	bool use_dnsbl;                                 // dns blacklist blocking ?
	char dnsbl_servs[1024];                         // comma-separated list of dnsbl servers

} login_config;

// TXT-specific account database
// holds info about all existing accounts (entire contents of account.txt)
extern struct auth_data {
	int account_id;
	uint8 sex; // 0, 1, 2
	char userid[24];
	char pass[32+1]; // 23+1 for normal, 32+1 for md5-ed passwords
	char lastlogin[24];
	int logincount;
	uint32 state; // packet 0x006a value + 1 (0: compte OK)
	char email[40]; // e-mail (by default: a@a.com)
	char error_message[20]; // Message of error code #6 = Your are Prohibited to log in until %s (packet 0x006a)
	time_t ban_until_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	char last_ip[16]; // save of last IP of connection
	char memo[255]; // a memo field
	int account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM]; // account script variables (stored on login server)
} *auth_dat;

#endif /* _LOGIN_H_ */
