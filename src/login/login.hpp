// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef LOGIN_HPP
#define LOGIN_HPP

#include <memory>

#include <common/cbasetypes.hpp>
#include <common/core.hpp>
#include <common/mmo.hpp> // NAME_LENGTH,SEX_*
#include <common/timer.hpp>
#include <config/core.hpp>

#include "account.hpp"

using rathena::server_core::Core;
using rathena::server_core::e_core_type;

namespace rathena{
	namespace server_login{
		class LoginServer : public Core{
			protected:
				bool initialize( int argc, char* argv[] ) override;
				void finalize() override;
				void handle_shutdown() override;

			public:
				LoginServer() : Core( e_core_type::LOGIN ){

				}
		};
	}
}

/// supported encryption types: 1- passwordencrypt, 2- passwordencrypt2, 3- both
#define PASSWORDENC 3

///Struct of 1 client connected to login-serv
struct login_session_data {
	uint32 account_id;			///also GID
	long login_id1;
	long login_id2;
	char sex;			/// 'F','M','S'

	char userid[NAME_LENGTH];	/// account name
	char passwd[PASSWD_LENGTH]; // 23+1 for plaintext, 32+1 for md5-ed passwords
	int passwdenc;			/// was the passwd transmited encrypted or clear ?
	char md5key[20];		/// md5 key of session (each connection could be encrypted with a md5 key)
	uint16 md5keylen;		/// len of the md5 key

	char lastlogin[24];		///date when last logged, Y-M-D HH:MM:SS
	uint8 group_id;			///groupid of account
	uint8 clienttype;		/// ???

	uint8 client_hash[16];		///hash of client
	int has_client_hash;		///client ha sent an hash

	int fd;				///socket of client

	char web_auth_token[WEB_AUTH_TOKEN_LENGTH]; /// web authentication token
};

#define MAX_SERVERS 5 //max number of mapserv that could be attach
///Struct describing 1 char-serv attach to us
struct mmo_char_server {
	char name[20];	///char-serv name
	int fd;			///char-serv socket (well actually file descriptor)
	uint32 ip;		///char-serv IP
	uint16 port;	///char-serv rt
	uint16 users;	/// user count on this server
	uint16 type;	/// 0=normal, 1=maintenance, 2=over 18, 3=paying, 4=P2P
	uint16 new_;	/// should display as 'new'?
};
extern struct mmo_char_server ch_server[MAX_SERVERS];

struct client_hash_node {
	unsigned int group_id;			//inferior or egal group to apply restriction
	uint8 hash[16];					///hash required for that groupid or below
	struct client_hash_node *next;	///next entry
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

	bool ipban;                                     /// perform IP blocking (via contents of `ipbanlist`) ?
	bool dynamic_pass_failure_ban;                  /// automatic IP blocking due to failed login attempts ?
	unsigned int dynamic_pass_failure_ban_interval; /// how far to scan the loginlog for password failures in minutes
	unsigned int dynamic_pass_failure_ban_limit;    /// number of failures needed to trigger the ipban
	unsigned int dynamic_pass_failure_ban_duration; /// duration of the ipban in minutes
	bool use_dnsbl;                                 /// dns blacklist blocking ?
	char dnsbl_servs[1024];                         /// comma-separated list of dnsbl servers

	int allowed_regs;								/// max number of registration
	int time_allowed;								/// registration interval in seconds

	int client_hash_check;							/// flags for checking client md5
	struct client_hash_node *client_hash_nodes;		/// linked list containing md5 hash for each gm group
	char loginconf_name[256];						/// name of main config file
	char msgconf_name[256];							/// name of msg_conf config file
	char lanconf_name[256];							/// name of lan config file

	bool usercount_disable;							/// Disable colorization and description in general?
	int usercount_low;								/// Amount of users that will display in green
	int usercount_medium;							/// Amount of users that will display in yellow
	int usercount_high;								/// Amount of users that will display in red

	int char_per_account;							/// number of characters an account can have
#ifdef VIP_ENABLE
	struct {
		unsigned int group;							/// VIP group ID
		unsigned int char_increase;					/// number of char-slot to increase in VIP state
	} vip_sys;
#endif
	bool use_web_auth_token;						/// Enable web authentication token system
	int disable_webtoken_delay;						/// delay disabling web token after char logs off in milliseconds
};
extern struct Login_Config login_config;

#define sex_num2str(num) ( (num ==  SEX_FEMALE  ) ? 'F' : (num ==  SEX_MALE  ) ? 'M' : 'S' )
#define sex_str2num(str) ( (str == 'F' ) ?  SEX_FEMALE  : (str == 'M' ) ?  SEX_MALE  :  SEX_SERVER  )

#define msg_config_read(cfgName) login_msg_config_read(cfgName)
#define msg_txt(msg_number) login_msg_txt(msg_number)
#define do_final_msg() login_do_final_msg()
int login_msg_config_read(char *cfgName);
const char* login_msg_txt(int msg_number);
void login_do_final_msg(void);
bool login_config_read(const char* cfgName, bool normal);

/// Online User Database [Wizputer]
struct online_login_data {
	uint32 account_id;
	int waiting_disconnect;
	int char_server;
};

/// Auth database
#define AUTH_TIMEOUT 30000
struct auth_node {
	uint32 account_id;
	uint32 login_id1;
	uint32 login_id2;
	uint32 ip;
	char sex;
	uint8 clienttype;
};

///Accessors
AccountDB* login_get_accounts_db(void);

struct online_login_data* login_get_online_user( uint32 account_id );

/**
 * Function to add a user in online_db.
 *  Checking if the user is already registered in the db.
 *  Stop disconnection timer if set.
 * @param char_server: id of char-serv on wich the player is
 * @param account_id: the account identifier
 * @return the new|registered online data
 */
struct online_login_data* login_add_online_user(int char_server, uint32 account_id);

/**
 * Function to remove a user from online_db.
 *  Checking if user was already scheduled for deletion, and remove that timer if found.
 * @param account_id: the account identifier
 */
void login_remove_online_user(uint32 account_id);

struct auth_node* login_get_auth_node( uint32 account_id );

struct auth_node* login_add_auth_node( struct login_session_data* sd, uint32 ip );

void login_remove_auth_node( uint32 account_id );

/**
 * Timered function to disconnect a user from login.
 *  This is done either after auth_ok or kicked by char-server.
 *  Removing user from auth_db and online_db.
 *  Delay is AUTH_TIMEOUT by default.
 * @param tid: timer id
 * @param tick: tick of execution
 * @param id: user account id
 * @param data: unused
 * @return :0
 */
TIMER_FUNC(login_waiting_disconnect_timer);

void login_online_db_setoffline( int char_server );

/**
 * Test to determine if an IP come from LAN or WAN.
 * @param ip: ip to check if in auth network
 * @return 0 if from wan, or subnet_char_ip if lan
 */
int lan_subnetcheck(uint32 ip);


/**
 * Create a new account and save it in db/sql.
 * @param userid: string for user login
 * @param pass: string for user pass
 * @param sex: should be M|F|S (todo make an enum ?)
 * @param last_ip:
 * @return :
 *	-1: success
 *	0: unregistered id (wrong sex fail to create in db);
 *	1: incorrect pass or userid (userid|pass too short or already exist);
 *	3: registration limit exceeded;
 */
int login_mmo_auth_new(const char* userid, const char* pass, const char sex, const char* last_ip);

/**
 * Check/authentication of a connection.
 * @param sd: string (atm:md5key or dbpass)
 * @param isServer: string (atm:md5key or dbpass)
 * @return :
 *	-1: success
 *	0: unregistered id;
 *	1: incorrect pass;
 *	2: expired id
 *	3: blacklisted (or registration limit exceeded if new acc);
 *	5: invalid client_version|hash;
 *	6: banned
 *	x: acc state (TODO document me deeper)
 */
int login_mmo_auth(struct login_session_data* sd, bool isServer);

int login_get_usercount( int users );

#endif /* LOGIN_HPP */
