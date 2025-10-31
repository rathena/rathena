// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_HPP
#define CHAR_HPP

#include <memory>
#include <unordered_map>
#include <vector>

#include <common/core.hpp> // CORE_ST_LAST
#include <common/mmo.hpp>
#include <common/msg_conf.hpp>
#include <common/timer.hpp>
#include <config/core.hpp>

#include "packets.hpp"

using rathena::server_core::Core;
using rathena::server_core::e_core_type;

namespace rathena::server_character {
class CharacterServer : public Core {
	protected:
		bool initialize( int32 argc, char* argv[] ) override;
		void finalize() override;
		void handle_shutdown() override;

	public:
		CharacterServer() : Core( e_core_type::CHARACTER ){

		}
};

}

extern int32 login_fd; //login file descriptor
extern int32 char_fd; //char file descriptor

#define MAX_STARTPOINT 5
#define MAX_STARTITEM 32

enum e_char_delete {
	CHAR_DEL_EMAIL = 1,
	CHAR_DEL_BIRTHDATE
};

enum e_char_delete_restriction {
	CHAR_DEL_RESTRICT_PARTY = 1,
	CHAR_DEL_RESTRICT_GUILD,
	CHAR_DEL_RESTRICT_ALL
};

enum e_char_del_response {
	CHAR_DELETE_OK = 0,
	CHAR_DELETE_DATABASE,
	CHAR_DELETE_NOTFOUND,
	CHAR_DELETE_BASELEVEL,
	CHAR_DELETE_GUILD,
	CHAR_DELETE_PARTY,
	CHAR_DELETE_TIME,
};

struct Schema_Config {
	int32 db_use_sqldbs;
	char db_path[1024];
	char char_db[DB_NAME_LEN];
	char scdata_db[DB_NAME_LEN];
	char skillcooldown_db[DB_NAME_LEN];
	char cart_db[DB_NAME_LEN];
	char inventory_db[DB_NAME_LEN];
	char charlog_db[DB_NAME_LEN];
	char storage_db[DB_NAME_LEN];
	char interlog_db[DB_NAME_LEN];
	char skill_db[DB_NAME_LEN];
	char memo_db[DB_NAME_LEN];
	char guild_db[DB_NAME_LEN];
	char guild_alliance_db[DB_NAME_LEN];
	char guild_castle_db[DB_NAME_LEN];
	char guild_expulsion_db[DB_NAME_LEN];
	char guild_member_db[DB_NAME_LEN];
	char guild_position_db[DB_NAME_LEN];
	char guild_skill_db[DB_NAME_LEN];
	char guild_storage_db[DB_NAME_LEN];
	char party_db[DB_NAME_LEN];
	char pet_db[DB_NAME_LEN];
	char mail_db[DB_NAME_LEN]; // MAIL SYSTEM
	char mail_attachment_db[DB_NAME_LEN];
	char auction_db[DB_NAME_LEN]; // Auctions System
	char friend_db[DB_NAME_LEN];
	char hotkey_db[DB_NAME_LEN];
	char quest_db[DB_NAME_LEN];
	char homunculus_db[DB_NAME_LEN];
	char skill_homunculus_db[DB_NAME_LEN];
	char skillcooldown_homunculus_db[DB_NAME_LEN];
	char mercenary_db[DB_NAME_LEN];
	char mercenary_owner_db[DB_NAME_LEN];
	char skillcooldown_mercenary_db[DB_NAME_LEN];
	char elemental_db[DB_NAME_LEN];
	char bonus_script_db[DB_NAME_LEN];
	char acc_reg_num_table[DB_NAME_LEN];
	char acc_reg_str_table[DB_NAME_LEN];
	char char_reg_str_table[DB_NAME_LEN];
	char char_reg_num_table[DB_NAME_LEN];
	char clan_table[DB_NAME_LEN];
	char clan_alliance_table[DB_NAME_LEN];
	char achievement_table[DB_NAME_LEN];
};
extern struct Schema_Config schema_config;

#if PACKETVER_SUPPORTS_PINCODE
/// Pincode system
enum pincode_state : uint8 {
	PINCODE_OK		= 0,
	PINCODE_ASK		= 1,
	PINCODE_NOTSET	= 2,
	PINCODE_EXPIRED	= 3,
	PINCODE_NEW		= 4,
	PINCODE_ILLEGAL = 5,
#if 0
	PINCODE_KSSN	= 6, // Not supported since we do not store KSSN
#endif
#if PACKETVER >= 20180124
	// The button for pin code access was removed
	PINCODE_PASSED  = PINCODE_OK,
#else
	PINCODE_PASSED	= 7,
#endif
	PINCODE_WRONG	= 8,
	PINCODE_MAXSTATE
};
struct Pincode_Config {
	bool pincode_enabled;
	int32 pincode_changetime;
	int32 pincode_maxtry;
	bool pincode_force;
	bool pincode_allow_repeated;
	bool pincode_allow_sequential;
};
#endif

struct CharMove_Config {
	bool char_move_enabled;
	bool char_movetoused;
	bool char_moves_unlimited;
};
struct Char_Config {
	int32 char_per_account; //Maximum chars per account (default unlimited) [Sirius]
	int32 char_del_level; //From which level u can delete character [Lupus]
	int32 char_del_delay; //minimum delay before effectly do the deletion
	bool name_ignoring_case; // Allow or not identical name for characters but with a different case by [Yor]
	char unknown_char_name[NAME_LENGTH]; // Name to use when the requested name cannot be determined
	char char_name_letters[1024]; // list of letters/symbols allowed (or not) in a character name. by [Yor]
	int32 char_name_option; // Option to know which letters/symbols are authorised in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters) by [Yor]
	uint8 char_name_min_length; // Minimum character name length (default: 4)
	int32 char_del_option;	// Character deletion type, email = 1, birthdate = 2 (default)
	int32 char_del_restriction;	// Character deletion restriction (0: none, 1: if the character is in a party, 2: if the character is in a guild, 3: if the character is in a party or a guild)
	bool char_rename_party;	// Character renaming in a party
	bool char_rename_guild;	// Character renaming in a guild
};

#define TRIM_CHARS "\255\xA0\032\t\x0A\x0D " //The following characters are trimmed regardless because they cause confusion and problems on the servers. [Skotlex]
struct CharServ_Config {
	char userid[24];
	char passwd[24];
	char server_name[20];
	char wisp_server_name[NAME_LENGTH];
	char login_ip_str[128];
	uint32 login_ip;
	uint16 login_port;
	char char_ip_str[128];
	uint32 char_ip;
	char bind_ip_str[128];
	uint32 bind_ip;
	uint16 char_port;
	int32 char_maintenance;
	bool char_new;
	int32 char_new_display;

	struct CharMove_Config charmove_config;
	struct Char_Config char_config;
#if PACKETVER_SUPPORTS_PINCODE
	struct Pincode_Config pincode_config;
#endif

	int32 save_log; // show loading/saving messages
	int32 log_char;	// loggin char or not [devil]
	int32 log_inter;	// loggin inter or not [devil]
	int32 char_check_db;	///cheking sql-table at begining ?

	struct s_point_str start_point[MAX_STARTPOINT], start_point_doram[MAX_STARTPOINT]; // Initial position the player will spawn on the server
	int16 start_point_count, start_point_count_doram; // Number of positions read
	struct startitem start_items[MAX_STARTITEM], start_items_doram[MAX_STARTITEM]; // Initial items the player with spawn with on the server
	uint32 start_status_points;
	int32 console;
	int32 max_connect_user;
	int32 gm_allow_group;
	int32 autosave_interval;
	int32 start_zeny;
	int32 guild_exp_rate;

	int32 clan_remove_inactive_days;
	int32 mail_return_days;
	int32 mail_delete_days;
	int32 mail_retrieve;
	int32 mail_return_empty;

	int32 allowed_job_flag;
	int32 clear_parties;
};
extern struct CharServ_Config charserv_config;

#define MAX_MAP_SERVERS 2 //how many mapserver a char server can handle
struct mmo_map_server {
	int32 fd;
	uint32 ip;
	uint16 port;
	int32 users;
	std::vector<std::string> maps;
};
extern struct mmo_map_server map_server[MAX_MAP_SERVERS];

#define AUTH_TIMEOUT 30000
struct auth_node {
	uint32 account_id;
	uint32 char_id;
	uint32 login_id1;
	uint32 login_id2;
	uint32 ip;
	int32 sex;
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int32 group_id;
	unsigned changing_mapservers : 1;
	uint8 version;
};

std::unordered_map<uint32, std::shared_ptr<struct auth_node>>& char_get_authdb();

struct online_char_data {
	uint32 account_id;
	uint32 char_id;
	int32 fd;
	int32 waiting_disconnect;
	int16 server; // -2: unknown server, -1: not connected, 0+: id of server
	bool pincode_success;

public: 
	online_char_data( uint32 account_id );
};

std::unordered_map<uint32, std::shared_ptr<struct online_char_data>>& char_get_onlinedb();

struct char_session_data {
	bool auth; // whether the session is authed or not
	uint32 account_id, login_id1, login_id2, sex;
	int32 found_char[MAX_CHARS]; // ids of chars on this account
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int32 group_id; // permission
	uint8 char_slots; // total number of characters that can be created
	uint8 chars_vip;
	uint8 chars_billing;
	uint8 clienttype;
	char new_name[NAME_LENGTH];
	char birthdate[10+1];  // YYYY-MM-DD
	// Pincode system
	char pincode[PINCODE_LENGTH+1];
	uint32 pincode_seed;
	time_t pincode_change;
	uint16 pincode_try;
	bool pincode_correct;
	// Addon system
	uint32 char_moves[MAX_CHARS]; // character moves left
	uint8 isvip;
	time_t unban_time[MAX_CHARS];
	int32 charblock_timer;
	uint8 flag; // &1 - Retrieving guild bound items
};

std::unordered_map<uint32, std::shared_ptr<struct mmo_charstatus>>& char_get_chardb();

//Custom limits for the fame lists. [Skotlex]
extern int32 fame_list_size_chemist;
extern int32 fame_list_size_smith;
extern int32 fame_list_size_taekwon;
// Char-server-side stored fame lists [DracoRPG]
extern struct fame_list smith_fame_list[MAX_FAME_LIST];
extern struct fame_list chemist_fame_list[MAX_FAME_LIST];
extern struct fame_list taekwon_fame_list[MAX_FAME_LIST];

#define DEFAULT_AUTOSAVE_INTERVAL 300*1000
#define MAX_CHAR_BUF sizeof( struct CHARACTER_INFO ) //Max size (for WFIFOHEAD calls)

int32 char_search_mapserver( const std::string& map, uint32 ip, uint16 port );
int32 char_lan_subnetcheck(uint32 ip);

int32 char_count_users(void);
void char_db_setoffline( std::shared_ptr<struct online_char_data> character, int32 server );
void char_set_char_online(int32 map_id, uint32 char_id, uint32 account_id);
void char_set_char_offline(uint32 char_id, uint32 account_id);
void char_set_all_offline(int32 id);
void char_disconnect_player(uint32 account_id);
TIMER_FUNC(char_chardb_waiting_disconnect);

int32 char_mmo_gender(const struct char_session_data *sd, const struct mmo_charstatus *p, char sex);
int32 char_mmo_char_tobuf(uint8* buffer, struct mmo_charstatus* p);
int32 char_mmo_char_tosql(uint32 char_id, struct mmo_charstatus* p);
int32 char_mmo_char_fromsql(uint32 char_id, struct mmo_charstatus* p, bool load_everything);
int32 char_mmo_chars_fromsql(struct char_session_data* sd, uint8* buf, uint8* count = nullptr);
enum e_char_del_response char_delete(struct char_session_data* sd, uint32 char_id);
int32 char_rename_char_sql(struct char_session_data *sd, uint32 char_id);
int32 char_divorce_char_sql(int32 partner_id1, int32 partner_id2);
int32 char_memitemdata_to_sql(const struct item items[], int32 max, int32 id, enum storage_type tableswitch, uint8 stor_id);
bool char_memitemdata_from_sql(struct s_storage* p, int32 max, int32 id, enum storage_type tableswitch, uint8 stor_id);

int32 char_married(int32 pl1,int32 pl2);
int32 char_child(int32 parent_id, int32 child_id);
int32 char_family(int32 pl1,int32 pl2,int32 pl3);

//extern bool char_gm_read;
int32 char_loadName(uint32 char_id, char* name);
int32 char_check_char_name(char * name, char * esc_name);

bool char_pincode_decrypt( uint32 userSeed, char* pin );
int32 char_pincode_compare( int32 fd, struct char_session_data* sd, char* pin );
void char_auth_ok(int32 fd, struct char_session_data *sd);
void char_set_charselect(uint32 account_id);
void char_read_fame_list(void);

int32 char_make_new_char( struct char_session_data* sd, char* name_, int32 str, int32 agi, int32 vit, int32 int_, int32 dex, int32 luk, int32 slot, int32 hair_color, int32 hair_style, int16 start_job, int32 sex );

void char_set_session_flag_(int32 account_id, int32 val, bool set);
#define char_set_session_flag(account_id, val)   ( char_set_session_flag_((account_id), (val), true)  )
#define char_unset_session_flag(account_id, val) ( char_set_session_flag_((account_id), (val), false) )

//For use in packets that depend on an sd being present [Skotlex]
#define FIFOSD_CHECK(rest) { if (RFIFOREST(fd) < rest) return 0; if (sd == nullptr || !sd->auth) { RFIFOSKIP(fd, rest); return 0; } }

#define msg_config_read(cfgName) char_msg_config_read(cfgName)
#define msg_txt(msg_number) char_msg_txt(msg_number)
#define do_final_msg() char_do_final_msg()
int32 char_msg_config_read(const char *cfgName);
const char* char_msg_txt(int32 msg_number);
void char_do_final_msg(void);
bool char_config_read(const char* cfgName, bool normal);

#endif /* CHAR_HPP */
