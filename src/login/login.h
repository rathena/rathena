// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_H_
#define _LOGIN_H_

#include "../common/mmo.h" // NAME_LENGTH,SEX_*
#include "../common/core.h" // CORE_ST_LAST
#include "../config/core.h"

enum E_LOGINSERVER_ST
{
	LOGINSERVER_ST_RUNNING = CORE_ST_LAST,
	LOGINSERVER_ST_STARTING,
	LOGINSERVER_ST_SHUTDOWN,
	LOGINSERVER_ST_LAST
};

/// supported encryption types: 1- passwordencrypt, 2- passwordencrypt2, 3- both
#define PASSWORDENC 3

///Struct of 1 client connected to login-serv
struct login_session_data {
	int account_id;			///also GID
	long login_id1;
	long login_id2;
	char sex;			/// 'F','M','S'

	char userid[NAME_LENGTH];	/// account name
	char passwd[32+1];		/// 23+1 for plaintext, 32+1 for md5-ed passwords
	int passwdenc;			/// was the passwd transmited encrypted or clear ?
	char md5key[20];		/// md5 key of session (each connection could be encrypted with a md5 key)
	uint16 md5keylen;		/// len of the md5 key

	char lastlogin[24];		///date when last logged, Y-M-D HH:MM:SS
	uint8 group_id;			///groupid of account
	uint8 clienttype;		/// ???
	uint32 version;			///version contained in clientinfo

	uint8 client_hash[16];		///hash of client
	int has_client_hash;		///client ha sent an hash

	int fd;				///socket of client
};

///Struct describing 1 char-serv attach to us
struct mmo_char_server {
	char name[20];	///char-serv name
	int fd;		///char-serv socket (well actually file descriptor)
	uint32 ip;	///char-serv IP
	uint16 port;	///char-serv rt
	uint16 users;	/// user count on this server
	uint16 type;	/// 0=normal, 1=maintenance, 2=over 18, 3=paying, 4=P2P
	uint16 new_;	/// should display as 'new'?
};

struct client_hash_node {
	int group_id;		///group
	uint8 hash[16];		///hash required for that groupid or below
	struct client_hash_node *next; ///next entry
};

struct Login_Config {
	uint32 login_ip;                                /// the address to bind to
	uint16 login_port;                              /// the port to bind to
	unsigned int ipban_cleanup_interval;            /// interval (in seconds) to clean up expired IP bans
	unsigned int ip_sync_interval;                  /// interval (in minutes) to execute a DNS/IP update (for dynamic IPs)
	bool log_login;                                 /// whether to log login server actions or not
	char date_format[32];                           /// date format used in messages
	bool console;                                   /// console input system enabled?
	bool new_account_flag,new_acc_length_limit;     /// autoregistration via _M/_F ? / if yes minimum length is 4?
	int start_limited_time;                         /// new account expiration time (-1: unlimited)
	bool use_md5_passwds;                           /// work with password hashes instead of plaintext passwords?
	int group_id_to_connect;                        /// required group id to connect
	int min_group_id_to_connect;                    /// minimum group id to connect
	bool check_client_version;                      /// check the clientversion set in the clientinfo ?
	uint32 client_version_to_connect;               /// the client version needed to connect (if checking is enabled)

	bool ipban;                                     /// perform IP blocking (via contents of `ipbanlist`) ?
	bool dynamic_pass_failure_ban;                  /// automatic IP blocking due to failed login attemps ?
	unsigned int dynamic_pass_failure_ban_interval; /// how far to scan the loginlog for password failures
	unsigned int dynamic_pass_failure_ban_limit;    /// number of failures needed to trigger the ipban
	unsigned int dynamic_pass_failure_ban_duration; /// duration of the ipban
	bool use_dnsbl;                                 /// dns blacklist blocking ?
	char dnsbl_servs[1024];                         /// comma-separated list of dnsbl servers

	char account_engine[256];                       /// name of the engine to use (defaults to auto, for the first available engine)

	int client_hash_check;                          /// flags for checking client md5
	struct client_hash_node *client_hash_nodes;     /// linked list containg md5 hash for each gm group
	int char_per_account;                           /// number of characters an account can have
#ifdef VIP_ENABLE
	struct {
		unsigned int group;                     /// Vip groupid
		unsigned int char_increase;             /// number of char-slot to increase in vip state
	} vip_sys;
#endif
};

#define sex_num2str(num) ( (num ==  SEX_FEMALE  ) ? 'F' : (num ==  SEX_MALE  ) ? 'M' : 'S' )
#define sex_str2num(str) ( (str == 'F' ) ?  SEX_FEMALE  : (str == 'M' ) ?  SEX_MALE  :  SEX_SERVER  )

#define msg_config_read(cfgName) login_msg_config_read(cfgName)
#define msg_txt(msg_number) login_msg_txt(msg_number)
#define do_final_msg() login_do_final_msg()

int login_msg_config_read(char *cfgName);
const char* login_msg_txt(int msg_number);
void login_do_final_msg(void);


#define MAX_SERVERS 30					///number of charserv loginserv can handle
extern struct mmo_char_server server[MAX_SERVERS];	///array of char-servs data
extern struct Login_Config login_config;		///config of login serv


#endif /* _LOGIN_H_ */
