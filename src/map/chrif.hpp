// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHRIF_HPP
#define CHRIF_HPP

#include <ctime>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp> // NAME_LENGTH
#include <common/timer.hpp> // t_tick
#include <common/socket.hpp> // enum chrif_req_op

//fwd declaration
class map_session_data;

enum sd_state { ST_LOGIN, ST_LOGOUT, ST_MAPCHANGE };

enum e_chrif_save_opt {
	CSAVE_NORMAL = 0x00,		/// Normal
	CSAVE_QUIT = 0x01,				/// Character quitting
	CSAVE_CHANGE_MAPSERV = 0x02,	/// Character changing map server
	CSAVE_AUTOTRADE = 0x04,		/// Character entering autotrade state
	CSAVE_INVENTORY = 0x08,		/// Inventory data changed
	CSAVE_CART = 0x10,				/// Cart data changed
	CSAVE_QUITTING = CSAVE_QUIT|CSAVE_CHANGE_MAPSERV|CSAVE_AUTOTRADE,
};

struct auth_node {
	uint32 account_id, char_id;
	int32 login_id1, login_id2, sex, fd;
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	map_session_data *sd;	//Data from logged on char.
	struct mmo_charstatus *char_dat;	//Data from char server.
	t_tick node_created; //timestamp for node timeouts
	enum sd_state state; //To track whether player was login in/out or changing maps.
};

void chrif_setuserid(char* id);
void chrif_setpasswd(char* pwd);
void chrif_checkdefaultlogin(void);
int32 chrif_setip(const char* ip);
void chrif_setport(uint16 port);

int32 chrif_isconnected(void);

extern int32 chrif_connected;
extern int32 other_mapserver_count;
extern char charserver_name[NAME_LENGTH];

struct auth_node* chrif_search(uint32 account_id);
struct auth_node* chrif_auth_check(uint32 account_id, uint32 char_id, enum sd_state state);
bool chrif_auth_delete(uint32 account_id, uint32 char_id, enum sd_state state);
bool chrif_auth_finished(map_session_data* sd);

void chrif_authreq(map_session_data* sd, bool autotrade);
void chrif_authok(int32 fd);
int32 chrif_scdata_request(uint32 account_id, uint32 char_id);
int32 chrif_skillcooldown_request(uint32 account_id, uint32 char_id);
int32 chrif_skillcooldown_save(map_session_data &sd);
int32 chrif_skillcooldown_load(int32 fd);

int32 chrif_save(map_session_data* sd, int32 flag);
int32 chrif_charselectreq(map_session_data* sd, uint32 s_ip);
int32 chrif_changemapserver(map_session_data* sd, uint32 ip, uint16 port);

int32 chrif_searchcharid(uint32 char_id);
int32 chrif_changeemail(int32 id, const char *actual_email, const char *new_email);
int32 chrif_req_login_operation(int32 aid, const char* character_name, enum chrif_req_op operation_type, int32 timediff, int32 val1, int32 val2);
int32 chrif_updatefamelist(map_session_data &sd, e_rank ranktype);
int32 chrif_buildfamelist(void);
int32 chrif_save_scdata(map_session_data *sd);
int32 chrif_char_offline(map_session_data *sd);
int32 chrif_char_offline_nsd(uint32 account_id, uint32 char_id);
int32 chrif_char_reset_offline(void);
int32 send_users_tochar(void);
int32 chrif_char_online(map_session_data *sd);
int32 chrif_changesex(map_session_data *sd, bool change_account);
int32 chrif_divorce(int32 partner_id1, int32 partner_id2);

int32 chrif_removefriend(uint32 char_id, int32 friend_id);

void chrif_parse_ack_vipActive(int32 fd);

int32 chrif_req_charban(int32 aid, const char* character_name, int32 timediff);
int32 chrif_req_charunban(int32 aid, const char* character_name);

int32 chrif_bsdata_request(uint32 char_id);
int32 chrif_bsdata_save(map_session_data *sd, bool quit);

void do_final_chrif(void);
void do_init_chrif(void);

int32 chrif_flush_fifo(void);

#endif /* CHRIF_HPP */
