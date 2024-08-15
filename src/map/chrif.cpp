// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chrif.hpp"

#include <cstdlib>
#include <cstring>

#include <common/cbasetypes.hpp>
#include <common/ers.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "clan.hpp"
#include "clif.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "npc.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"
#include "script.hpp" // script_config
#include "storage.hpp"

static TIMER_FUNC(check_connect_char_server);

static struct eri *auth_db_ers; //For reutilizing player login structures.
static DBMap* auth_db; // int id -> struct auth_node*
static bool char_init_done = false; //server already initialized? Used for InterInitOnce and vending loadings

static const int packet_len_table[0x3d] = { // U - used, F - free
	60, 3,-1,-1,10,-1, 6,-1,	// 2af8-2aff: U->2af8, U->2af9, U->2afa, U->2afb, U->2afc, U->2afd, U->2afe, U->2aff
	 6,-1,18, 7,-1, -1, 28 + MAP_NAME_LENGTH_EXT, 10,	// 2b00-2b07: U->2b00, U->2b01, U->2b02, U->2b03, U->2b04, U->2b05, U->2b06, U->2b07
	 6,30, 10, -1,86, 7,44,34,	// 2b08-2b0f: U->2b08, U->2b09, U->2b0a, U->2b0b, U->2b0c, U->2b0d, U->2b0e, U->2b0f
	11,10,10, 0,11, -1, 0,10,	// 2b10-2b17: U->2b10, U->2b11, U->2b12, F->2b13, U->2b14, U->2b15, F->2b16, U->2b17
	 2,10, 2,-1,-1,-1, 2, 7,	// 2b18-2b1f: U->2b18, U->2b19, U->2b1a, U->2b1b, U->2b1c, U->2b1d, U->2b1e, U->2b1f
	-1,10, 8, 2, 2,14,19,19,	// 2b20-2b27: U->2b20, U->2b21, U->2b22, U->2b23, U->2b24, U->2b25, U->2b26, U->2b27
	-1, 0, 6,15, 0, 6,-1,-1,	// 2b28-2b2f: U->2b28, F->2b29, U->2b2a, U->2b2b, F->2b2c, U->2b2d, U->2b2e, U->2b2f
 };

//Used Packets:
//2af8: Outgoing, chrif_connect -> 'connect to charserver / auth @ charserver'
//2af9: Incoming, chrif_connectack -> 'answer of the 2af8 login(ok / fail)'
//2afa: Outgoing, chrif_sendmap -> 'sending our maps'
//2afb: Incoming, chrif_sendmapack -> 'Maps received successfully / or not .. also received server name'
//2afc: Outgoing, chrif_scdata_request -> request sc_data for pc_authok'ed char. <- new command reuses previous one.
//2afd: Incoming, chrif_authok -> 'client authentication ok'
//2afe: Outgoing, send_usercount_tochar -> 'sends player count of this map server to charserver'
//2aff: Outgoing, send_users_tochar -> 'sends all actual connected character ids to charserver'
//2b00: Incoming, map_setusers -> 'set the actual usercount? PACKET.2B COUNT.L.. ?' (not sure)
//2b01: Outgoing, chrif_save -> 'charsave of char XY account XY (complete struct)'
//2b02: Outgoing, chrif_charselectreq -> 'player returns from ingame to charserver to select another char.., this packets includes sessid etc' ? (not 100% sure)
//2b03: Incoming, clif_charselectok -> '' (i think its the packet after enterworld?) (not sure)
//2b04: Incoming, chrif_recvmap -> 'getting maps from charserver of other mapserver's'
//2b05: Outgoing, chrif_changemapserver -> 'Tell the charserver the mapchange / quest for ok...'
//2b06: Incoming, chrif_changemapserverack -> 'awnser of 2b05, ok/fail, data: dunno^^'
//2b07: Outgoing, chrif_removefriend -> 'Tell charserver to remove friend_id from char_id friend list'
//2b08: Outgoing, chrif_searchcharid -> '...'
//2b09: Incoming, map_addchariddb -> 'Adds a name to the nick db'
//2b0a: Outgoing, chrif_skillcooldown_request -> requesting the list of skillcooldown for char
//2b0b: Incoming, chrif_skillcooldown_load -> received the list of cooldown for char
//2b0c: Outgoing, chrif_changeemail -> 'change mail address ...'
//2b0d: Incoming, chrif_changedsex -> 'Change sex of acc XY' (or char)
//2b0e: Outgoing, chrif_req_login_operation -> 'Do some operations (change sex, ban / unban etc)'
//2b0f: Incoming, chrif_ack_login_req -> 'answer of the 2b0e'
//2b10: Outgoing, chrif_updatefamelist -> 'Update the fame ranking lists and send them'
//2b11: Outgoing, chrif_divorce -> 'tell the charserver to do divorce'
//2b12: Incoming, chrif_divorceack -> 'divorce chars
//2b13: Outgoing, chrif_update_ip -> 'tell the change of map-server IP'
//2b14: Incoming, chrif_accountban -> 'not sure: kick the player with message XY'
//2b15: Outgoing, chrif_skillcooldown_save -> request to save skillcooldown
//2b17: Outgoing, chrif_char_offline -> 'tell the charserver that the char is now offline'
//2b18: Outgoing, chrif_char_reset_offline -> 'set all players OFF!'
//2b19: Outgoing, chrif_char_online -> 'tell the charserver that the char .. is online'
//2b1a: Outgoing, chrif_buildfamelist -> 'Build the fame ranking lists and send them'
//2b1b: Incoming, chrif_recvfamelist -> 'Receive fame ranking lists'
//2b1c: Outgoing, chrif_save_scdata -> 'Send sc_data of player for saving.'
//2b1d: Incoming, chrif_load_scdata -> 'received sc_data of player for loading.'
//2b1e: Incoming, chrif_update_ip -> 'Reqest forwarded from char-server for interserver IP sync.' [Lance]
//2b1f: Incoming, chrif_disconnectplayer -> 'disconnects a player (aid X) with the message XY ... 0x81 ..' [Sirius]
//2b20: Incoming, chrif_removemap -> 'remove maps of a server (sample: its going offline)' [Sirius]
//2b21: Incoming, chrif_save_ack. Returned after a character has been "final saved" on the char-server. [Skotlex]
//2b22: Incoming, chrif_updatefamelist_ack. Updated one position in the fame list.
//2b23: Outgoing, chrif_keepalive. charserver ping.
//2b24: Incoming, chrif_keepalive_ack. charserver ping reply.
//2b25: Incoming, chrif_deadopt -> 'Removes baby from Father ID and Mother ID'
//2b26: Outgoing, chrif_authreq -> 'client authentication request'
//2b27: Incoming, chrif_authfail -> 'client authentication failed'
//2b28: Outgoing, chrif_req_charban -> 'ban a specific char '
//2b29: FREE
//2b2a: Outgoing, chrif_req_charunban -> 'unban a specific char '
//2b2b: Incoming, chrif_parse_ack_vipActive -> vip info result
//2b2c: FREE
//2b2d: Outgoing, chrif_bsdata_request -> request bonus_script for pc_authok'ed char.
//2b2e: Outgoing, chrif_bsdata_save -> Send bonus_script of player for saving.
//2b2f: Incoming, chrif_bsdata_received -> received bonus_script of player for loading.

int chrif_connected = 0;
int char_fd = -1;
static char char_ip_str[128];
static uint32 char_ip = 0;
static uint16 char_port = 6121;
static char userid[NAME_LENGTH], passwd[NAME_LENGTH];
static int chrif_state = 0;
int other_mapserver_count=0; //Holds count of how many other map servers are online (apart of this instance) [Skotlex]
char charserver_name[NAME_LENGTH];

//Interval at which map server updates online listing. [Valaris]
#define CHECK_INTERVAL 3600000
//Interval at which map server sends number of connected users. [Skotlex]
#define UPDATE_INTERVAL 10000
//This define should spare writing the check in every function. [Skotlex]
#define chrif_check(a) { if(!chrif_isconnected()) return a; }

struct auth_node* chrif_search(uint32 account_id) {
	return (struct auth_node*)idb_get(auth_db, account_id);
}

struct auth_node* chrif_auth_check(uint32 account_id, uint32 char_id, enum sd_state state) {
	struct auth_node *node = chrif_search(account_id);

	return ( node && node->char_id == char_id && node->state == state ) ? node : nullptr;
}

bool chrif_auth_delete(uint32 account_id, uint32 char_id, enum sd_state state) {
	struct auth_node *node;

	if ( (node = chrif_auth_check(account_id, char_id, state) ) ) {
		int fd = node->sd ? node->sd->fd : node->fd;

		if ( session[fd] && session[fd]->session_data == node->sd )
			session[fd]->session_data = nullptr;

		if ( node->char_dat )
			aFree(node->char_dat);

		if ( node->sd ) {
			if (node->sd->regs.vars)
				node->sd->regs.vars->destroy(node->sd->regs.vars, script_reg_destroy);

			if (node->sd->regs.arrays)
				node->sd->regs.arrays->destroy(node->sd->regs.arrays, script_free_array_db);

			node->sd->~map_session_data();
			aFree(node->sd);
		}

		ers_free(auth_db_ers, node);
		idb_remove(auth_db,account_id);

		return true;
	}
	return false;
}

//Moves the sd character to the auth_db structure.
static bool chrif_sd_to_auth(TBL_PC* sd, enum sd_state state) {
	struct auth_node *node;

	if ( chrif_search(sd->status.account_id) )
		return false; //Already exists?

	node = ers_alloc(auth_db_ers, struct auth_node);

	memset(node, 0, sizeof(struct auth_node));

	node->account_id = sd->status.account_id;
	node->char_id = sd->status.char_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->status.sex;
	node->fd = sd->fd;
	node->sd = sd;	//Data from logged on char.
	node->node_created = gettick(); //timestamp for node timeouts
	node->state = state;

	sd->state.active = 0;

	idb_put(auth_db, node->account_id, node);

	return true;
}

static bool chrif_auth_logout(TBL_PC* sd, enum sd_state state) {

	if(sd->fd && state == ST_LOGOUT) { //Disassociate player, and free it after saving ack returns. [Skotlex]
		//fd info must not be lost for ST_MAPCHANGE as a final packet needs to be sent to the player.
		if ( session[sd->fd] )
			session[sd->fd]->session_data = nullptr;
		sd->fd = 0;
	}

	return chrif_sd_to_auth(sd, state);
}

bool chrif_auth_finished(map_session_data* sd) {
	struct auth_node *node= chrif_search(sd->status.account_id);

	if ( node && node->sd == sd && node->state == ST_LOGIN ) {
		node->sd = nullptr;

		return chrif_auth_delete(node->account_id, node->char_id, ST_LOGIN);
	}

	return false;
}
// sets char-server's user id
void chrif_setuserid(char *id) {
	safestrncpy(userid, id, NAME_LENGTH);
}

// sets char-server's password
void chrif_setpasswd(char *pwd) {
	safestrncpy(passwd, pwd, NAME_LENGTH);
}

// security check, prints warning if using default password
void chrif_checkdefaultlogin(void) {
#if !defined(BUILDBOT)
	if( strcmp( userid, "s1" ) == 0 && strcmp( passwd, "p1" ) == 0 ){
		ShowWarning("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your 'login' table to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("and then edit your user/password in conf/map_athena.conf (or conf/import/map_conf.txt)\n");
	}
#endif
}

// sets char-server's ip address
int chrif_setip(const char* ip) {
	char ip_str[16];

	if ( !( char_ip = host2ip(ip) ) ) {
		ShowWarning("Failed to Resolve Char Server Address! (%s)\n", ip);

		return 0;
	}

	safestrncpy(char_ip_str, ip, sizeof(char_ip_str));

	ShowInfo("Char Server IP Address : '" CL_WHITE "%s" CL_RESET "' -> '" CL_WHITE "%s" CL_RESET "'.\n", ip, ip2str(char_ip, ip_str));

	return 1;
}

// sets char-server's port number
void chrif_setport(uint16 port) {
	char_port = port;
}

// says whether the char-server is connected or not
int chrif_isconnected(void) {
	return (session_isValid(char_fd) && chrif_state == 2);
}

/**
 * Saves character data.
 * @param sd: Player data
 * @param flag: Save flag types:
 *  CSAVE_NORMAL: Normal save
 *  CSAVE_QUIT: Character is quitting
 *  CSAVE_CHANGE_MAPSERV: Character is changing map-servers
 *  CSAVE_AUTOTRADE: Character used @autotrade
 *  CSAVE_INVENTORY: Character changed inventory data
 *  CSAVE_CART: Character changed cart data
 */
int chrif_save(map_session_data *sd, int flag) {
	uint16 mmo_charstatus_len = 0;

	nullpo_retr(-1, sd);

	pc_makesavestatus(sd);

	if ( (flag&CSAVE_QUITTING) && sd->state.active) { //Store player data which is quitting
		if (chrif_isconnected()) {
			chrif_save_scdata(sd);
			chrif_skillcooldown_save(sd);
		}
		if ( !(flag&CSAVE_AUTOTRADE) && !chrif_auth_logout(sd, (flag&CSAVE_QUIT) ? ST_LOGOUT : ST_MAPCHANGE) )
			ShowError("chrif_save: Failed to set up player %d:%d for proper quitting!\n", sd->status.account_id, sd->status.char_id);
	}

	chrif_check(-1); //Character is saved on reconnect.

	chrif_bsdata_save(sd, ((flag&CSAVE_QUITTING) && !(flag&CSAVE_AUTOTRADE)));

	if (sd->storage.dirty)
		storage_storagesave(sd);
	if (flag&CSAVE_INVENTORY)
		intif_storage_save(sd,&sd->inventory);
	if (flag&CSAVE_CART)
		intif_storage_save(sd,&sd->cart);

	//For data sync
	if (sd->state.storage_flag == 2)
		storage_guild_storagesave(sd->status.account_id, sd->status.guild_id, flag);
	if (sd->premiumStorage.dirty)
		storage_premiumStorage_save(sd);

	if (flag&CSAVE_QUITTING)
		sd->state.storage_flag = 0; //Force close it.

	//Saving of registry values.
	if (sd->vars_dirty)
		intif_saveregistry(sd);

	mmo_charstatus_len = sizeof(sd->status) + 13;
	WFIFOHEAD(char_fd, mmo_charstatus_len);
	WFIFOW(char_fd,0) = 0x2b01;
	WFIFOW(char_fd,2) = mmo_charstatus_len;
	WFIFOL(char_fd,4) = sd->status.account_id;
	WFIFOL(char_fd,8) = sd->status.char_id;
	WFIFOB(char_fd,12) = (flag&CSAVE_QUIT) ? 1 : 0; //Flag to tell char-server this character is quitting.

	// Copy the whole status into the packet
	memcpy( WFIFOP( char_fd, 13 ), &sd->status, sizeof( struct mmo_charstatus ) );

	WFIFOSET(char_fd, WFIFOW(char_fd,2));

	if( sd->status.pet_id > 0 && sd->pd )
		intif_save_petdata(sd->status.account_id,&sd->pd->pet);
	if( hom_is_active(sd->hd) )
		hom_save(sd->hd);
	if( sd->md && mercenary_get_lifetime(sd->md) > 0 )
		mercenary_save(sd->md);
	if( sd->ed && elemental_get_lifetime(sd->ed) > 0 )
		elemental_save(sd->ed);
	if( sd->save_quest )
		intif_quest_save(sd);
	if (sd->achievement_data.save)
		intif_achievement_save(sd);

	return 0;
}

// connects to char-server (plaintext)
/**
 * Map-serv request to login into char-serv
 * @param fd : char-serv fd to log into
 * @return 0:request sent
 */
int chrif_connect(int fd) {
	ShowStatus("Logging in to char server...\n", char_fd);
	WFIFOHEAD(fd,60);
	WFIFOW(fd,0) = 0x2af8;
	memcpy(WFIFOP(fd,2), userid, NAME_LENGTH);
	memcpy(WFIFOP(fd,26), passwd, NAME_LENGTH);
	WFIFOL(fd,50) = 0;
	WFIFOL(fd,54) = htonl(clif_getip());
	WFIFOW(fd,58) = htons(clif_getport());
	WFIFOSET(fd,60);

	return 0;
}

// sends maps to char-server
int chrif_sendmap(int fd) {
	ShowStatus("Sending maps to char server...\n");

	// Sending normal maps, not instances
	WFIFOHEAD( fd, 4 + instance_start * MAP_NAME_LENGTH_EXT );
	WFIFOW(fd,0) = 0x2afa;
	for (int i = 0; i < instance_start; i++)
		safestrncpy( WFIFOCP( fd, 4 + i * MAP_NAME_LENGTH_EXT ), map[i].name, MAP_NAME_LENGTH_EXT );
	WFIFOW( fd, 2 ) = 4 + instance_start * MAP_NAME_LENGTH_EXT;
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

// receive maps from some other map-server (relayed via char-server)
int chrif_recvmap(int fd) {
	int i, j;
	uint32 ip = ntohl(RFIFOL(fd,4));
	uint16 port = ntohs(RFIFOW(fd,8));

	for( i = 10, j = 0; i < RFIFOW( fd, 2 ); i += MAP_NAME_LENGTH_EXT, j++ ){
		map_setipport( mapindex_name2id( RFIFOCP( fd, i ) ), ip, port );
	}

	if (battle_config.etc_log)
		ShowStatus("Received maps from %d.%d.%d.%d:%d (%d maps)\n", CONVIP(ip), port, j);

	other_mapserver_count++;

	return 0;
}

// remove specified maps (used when some other map-server disconnects)
int chrif_removemap(int fd) {
	int i, j;
	uint32 ip =  RFIFOL(fd,4);
	uint16 port = RFIFOW(fd,8);

	for( i = 10, j = 0; i < RFIFOW( fd, 2 ); i += MAP_NAME_LENGTH_EXT, j++ ){
		map_eraseipport( mapindex_name2id( RFIFOCP( fd, i ) ), ip, port );
	}

	other_mapserver_count--;

	if(battle_config.etc_log)
		ShowStatus("remove map of server %d.%d.%d.%d:%d (%d maps)\n", CONVIP(ip), port, j);

	return 0;
}

// received after a character has been "final saved" on the char-server
static void chrif_save_ack(int fd) {
	chrif_auth_delete(RFIFOL(fd,2), RFIFOL(fd,6), ST_LOGOUT);
}

// request to move a character between mapservers
int chrif_changemapserver(map_session_data* sd, uint32 ip, uint16 port) {
	nullpo_retr(-1, sd);

	if (other_mapserver_count < 1) {//No other map servers are online!
		clif_authfail_fd(sd->fd, 0);
		return -1;
	}

	chrif_check(-1);

	WFIFOHEAD( char_fd, 37 + MAP_NAME_LENGTH_EXT );
	WFIFOW(char_fd, 0) = 0x2b05;
	WFIFOL(char_fd, 2) = sd->bl.id;
	WFIFOL(char_fd, 6) = sd->login_id1;
	WFIFOL(char_fd,10) = sd->login_id2;
	WFIFOL(char_fd,14) = sd->status.char_id;
	safestrncpy( WFIFOCP( char_fd, 18 ), mapindex_id2name( sd->mapindex ), MAP_NAME_LENGTH_EXT );
	int offset = 18 + MAP_NAME_LENGTH_EXT;
	WFIFOW( char_fd, offset + 0 ) = sd->bl.x;
	WFIFOW( char_fd, offset + 2 ) = sd->bl.y;
	WFIFOL( char_fd, offset + 4 ) = htonl( ip );
	WFIFOW( char_fd, offset + 8 ) = htons( port );
	WFIFOB( char_fd, offset + 10 ) = sd->status.sex;
	WFIFOL( char_fd, offset + 11 ) = htonl( session[sd->fd]->client_addr );
	WFIFOL( char_fd, offset + 15 ) = sd->group_id;
	WFIFOSET( char_fd, 37 + MAP_NAME_LENGTH_EXT );

	return 0;
}

/// map-server change (mapserv) request acknowledgement (positive or negative)
/// R 2b06 <account_id>.L <login_id1>.L <login_id2>.L <char_id>.L <map>.16B <x>.W <y>.W <ip>.L <port>.W
int chrif_changemapserverack(uint32 account_id, int login_id1, int login_id2, uint32 char_id, const char* map, short x, short y, uint32 ip, uint16 port) {
	struct auth_node *node;

	if ( !( node = chrif_auth_check(account_id, char_id, ST_MAPCHANGE) ) )
		return -1;

	if ( !login_id1 ) {
		ShowError("map server change failed.\n");
		clif_authfail_fd(node->fd, 0);
		chrif_char_offline(node->sd);
	} else
		clif_changemapserver( *node->sd, map, x, y, ntohl(ip), ntohs(port) );

	//Player has been saved already, remove him from memory. [Skotlex]
	chrif_auth_delete(account_id, char_id, ST_MAPCHANGE);

	return 0;
}

/**
 * Does the char_serv have validate our connection to him ?
 * If yes then 
 *  - Send all our mapname to charserv
 *  - Retrieve guild castle
 *  - Do OnInterIfInit and OnInterIfInitOnce on all npc 
 * 0x2af9 <errCode>B
 */
int chrif_connectack(int fd) {
	if (RFIFOB(fd,2)) {
		ShowFatalError("Connection to char-server failed %d, please check conf/import/map_conf userid and passwd.\n", RFIFOB(fd,2));
		exit(EXIT_FAILURE);
	}

	ShowStatus("Successfully logged on to Char Server (Connection: '" CL_WHITE "%d" CL_RESET "').\n",fd);
	chrif_state = 1;
	chrif_connected = 1;

	chrif_sendmap(fd);

	npc_event_runall(script_config.inter_init_event_name);
	if( !char_init_done ) {
		npc_event_runall(script_config.inter_init_once_event_name);
		guild_castle_map_init();
		intif_clan_requestclans();
	}

	return 0;
}

/**
 * @see DBApply
 */
static int chrif_reconnect(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = (struct auth_node *)db_data2ptr(data);

	switch (node->state) {
		case ST_LOGIN:
			if ( node->sd && node->char_dat == nullptr ) {//Since there is no way to request the char auth, make it fail.
				pc_authfail(node->sd);
				chrif_char_offline(node->sd);
				chrif_auth_delete(node->account_id, node->char_id, ST_LOGIN);
			}
			break;
		case ST_LOGOUT:
			//Re-send final save
			chrif_save(node->sd, CSAVE_QUIT|CSAVE_INVENTORY|CSAVE_CART);
			break;
		case ST_MAPCHANGE: { //Re-send map-change request.
			map_session_data *sd = node->sd;
			uint32 ip;
			uint16 port;

			if( map_mapname2ipport(sd->mapindex,&ip,&port) == 0 )
				chrif_changemapserver(sd, ip, port);
			else //too much lag/timeout is the closest explanation for this error.
				clif_authfail_fd(sd->fd, 3);

			break;
			}
	}

	return 0;
}


/// Called when all the connection steps are completed.
void chrif_on_ready(void) {
	ShowStatus("Map Server is now online.\n");

	chrif_state = 2;

	//If there are players online, send them to the char-server. [Skotlex]
	send_users_tochar();

	//Auth db reconnect handling
	auth_db->foreach(auth_db,chrif_reconnect);

	//Re-save any storages that were modified in the disconnection time. [Skotlex]
	do_reconnect_storage();

	//Re-save any guild castles that were modified in the disconnection time.
	guild_castle_reconnect(-1, CD_NONE, 0);
	
	// Charserver is ready for loading autotrader
	if (!char_init_done)
	{
		do_init_buyingstore_autotrade();
		do_init_vending_autotrade();
		char_init_done = true;
	}
}


/**
 * Maps are sent, then received misc info from char-server
 * - Server name
 * HZ 0x2afb
 **/
int chrif_sendmapack(int fd) {
	uint16 offs = 5;

	if (RFIFOB(fd,4)) {
		ShowFatalError("chrif : send map list to char server failed %d\n", RFIFOB(fd,2));
		exit(EXIT_FAILURE);
	}

	// Whisper name
	safestrncpy( wisp_server_name, RFIFOCP( fd, offs ), NAME_LENGTH );

	// Server name
	safestrncpy( charserver_name, RFIFOCP( fd, offs + NAME_LENGTH ), NAME_LENGTH );

	ShowStatus( "Map-server connected to char-server '" CL_WHITE "%s" CL_RESET "' (whispername: %s).\n", charserver_name, wisp_server_name );

	chrif_on_ready();

	return 0;
}

/*==========================================
 * Request sc_data from charserver [Skotlex]
 *------------------------------------------*/
int chrif_scdata_request(uint32 account_id, uint32 char_id) {

#ifdef ENABLE_SC_SAVING
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2afc;
	WFIFOL(char_fd,2) = account_id;
	WFIFOL(char_fd,6) = char_id;
	WFIFOSET(char_fd,10);
#endif
	return 0;
}

/*==========================================
 * Request skillcooldown from charserver
 *------------------------------------------*/
int chrif_skillcooldown_request(uint32 account_id, uint32 char_id) {
	chrif_check(-1);
	WFIFOHEAD(char_fd, 10);
	WFIFOW(char_fd, 0) = 0x2b0a;
	WFIFOL(char_fd, 2) = account_id;
	WFIFOL(char_fd, 6) = char_id;
	WFIFOSET(char_fd, 10);
	return 0;
}

/*==========================================
 * Request auth confirmation
 *------------------------------------------*/
void chrif_authreq(map_session_data *sd, bool autotrade) {
	struct auth_node *node= chrif_search(sd->bl.id);

	if( node != nullptr || !chrif_isconnected() ) {
		set_eof(sd->fd);
		return;
	}

	WFIFOHEAD(char_fd,20);
	WFIFOW(char_fd,0) = 0x2b26;
	WFIFOL(char_fd,2) = sd->status.account_id;
	WFIFOL(char_fd,6) = sd->status.char_id;
	WFIFOL(char_fd,10) = sd->login_id1;
	WFIFOB(char_fd,14) = sd->status.sex;
	WFIFOL(char_fd,15) = htonl(session[sd->fd]->client_addr);
	WFIFOB(char_fd,19) = autotrade;
	WFIFOSET(char_fd,20);
	chrif_sd_to_auth(sd, ST_LOGIN);
}

/*==========================================
 * Auth confirmation ack
 *------------------------------------------*/
void chrif_authok(int fd) {
	uint32 account_id, group_id, char_id;
	uint32 login_id1,login_id2;
	time_t expiration_time;
	struct mmo_charstatus* status;
	struct auth_node *node;
	bool changing_mapservers;
	TBL_PC* sd;

	//Check if both servers agree on the struct's size
	if( RFIFOW(fd,2) - 25 != sizeof(struct mmo_charstatus) ) {
		ShowError("chrif_authok: Data size mismatch! %d != %" PRIuPTR "\n", RFIFOW(fd,2) - 25, sizeof(struct mmo_charstatus));
		return;
	}

	account_id = RFIFOL(fd,4);
	login_id1 = RFIFOL(fd,8);
	login_id2 = RFIFOL(fd,12);
	expiration_time = (time_t)(int32)RFIFOL(fd,16);
	group_id = RFIFOL(fd,20);
	changing_mapservers = (RFIFOB(fd,24)) > 0;
	status = (struct mmo_charstatus*)RFIFOP(fd,25);
	char_id = status->char_id;

	//Check if we don't already have player data in our server
	//Causes problems if the currently connected player tries to quit or this data belongs to an already connected player which is trying to re-auth.
	if ( ( sd = map_id2sd(account_id) ) != nullptr )
		return;

	if ( ( node = chrif_search(account_id) ) == nullptr )
		return; // should not happen

	if ( node->state != ST_LOGIN )
		return; //character in logout phase, do not touch that data.

	if ( node->sd == nullptr ) {
		/*
		//When we receive double login info and the client has not connected yet,
		//discard the older one and keep the new one.
		chrif_auth_delete(node->account_id, node->char_id, ST_LOGIN);
		*/
		return; // should not happen
	}

	sd = node->sd;

	if( global_core->is_running() &&
		node->char_dat == nullptr &&
		node->account_id == account_id &&
		node->char_id == char_id &&
		node->login_id1 == login_id1 )
	{ //Auth Ok
		if (pc_authok(sd, login_id2, expiration_time, group_id, status, changing_mapservers))
			return;
	} else { //Auth Failed
		pc_authfail(sd);
	}

	chrif_char_offline(sd); //Set him offline, the char server likely has it set as online already.
	chrif_auth_delete(account_id, char_id, ST_LOGIN);
}

// client authentication failed
void chrif_authfail(int fd) {/* HELLO WORLD. ip in RFIFOL 15 is not being used (but is available) */
	uint32 account_id, char_id;
	uint32 login_id1;
	char sex;
	struct auth_node* node;

	account_id = RFIFOL(fd,2);
	char_id    = RFIFOL(fd,6);
	login_id1  = RFIFOL(fd,10);
	sex        = RFIFOB(fd,14);

	node = chrif_search(account_id);

	if( node != nullptr &&
		node->account_id == account_id &&
		node->char_id == char_id &&
		node->login_id1 == login_id1 &&
		node->sex == sex &&
		node->state == ST_LOGIN )
	{// found a match
		clif_authfail_fd(node->fd, 0);
		chrif_auth_delete(account_id, char_id, ST_LOGIN);
	}
}


/**
 * This can still happen (client times out while waiting for char to confirm auth data)
 * @see DBApply
 */
int auth_db_cleanup_sub(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = (struct auth_node *)db_data2ptr(data);
	
	if(DIFF_TICK(gettick(),node->node_created)>60000) {
		const char* states[] = { "Login", "Logout", "Map change" };
		switch (node->state) {
			case ST_LOGOUT:
				//Re-save attempt (->sd should never be null here).
				node->node_created = gettick(); //Refresh tick (avoid char-server load if connection is really bad)
				chrif_save(node->sd, CSAVE_QUIT|CSAVE_INVENTORY|CSAVE_CART);
				break;
			default:
				//Clear data. any connected players should have timed out by now.
				ShowInfo("auth_db: Node (state %s) timed out for %d:%d\n", states[node->state], node->account_id, node->char_id);
				chrif_char_offline_nsd(node->account_id, node->char_id);
				chrif_auth_delete(node->account_id, node->char_id, node->state);
				break;
		}
		return 1;
	}
	return 0;
}

TIMER_FUNC(auth_db_cleanup){
	chrif_check(0);
	auth_db->foreach(auth_db, auth_db_cleanup_sub);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int chrif_charselectreq(map_session_data* sd, uint32 s_ip) {
	nullpo_retr(-1, sd);

	if( !sd || !sd->bl.id || !sd->login_id1 )
		return -1;

	chrif_check(-1);

	WFIFOHEAD(char_fd,18);
	WFIFOW(char_fd, 0) = 0x2b02;
	WFIFOL(char_fd, 2) = sd->bl.id;
	WFIFOL(char_fd, 6) = sd->login_id1;
	WFIFOL(char_fd,10) = sd->login_id2;
	WFIFOL(char_fd,14) = htonl(s_ip);
	WFIFOSET(char_fd,18);

	return 0;
}

/*==========================================
 * Search Char trough id on char serv
 *------------------------------------------*/
int chrif_searchcharid(uint32 char_id) {

	if( !char_id )
		return -1;

	chrif_check(-1);

	WFIFOHEAD(char_fd,6);
	WFIFOW(char_fd,0) = 0x2b08;
	WFIFOL(char_fd,2) = char_id;
	WFIFOSET(char_fd,6);

	return 0;
}

/*==========================================
 * Change Email
 *------------------------------------------*/
int chrif_changeemail(int id, const char *actual_email, const char *new_email) {

	if (battle_config.etc_log)
		ShowInfo("chrif_changeemail: account: %d, actual_email: '%s', new_email: '%s'.\n", id, actual_email, new_email);

	chrif_check(-1);

	WFIFOHEAD(char_fd,86);
	WFIFOW(char_fd,0) = 0x2b0c;
	WFIFOL(char_fd,2) = id;
	memcpy(WFIFOP(char_fd,6), actual_email, 40);
	memcpy(WFIFOP(char_fd,46), new_email, 40);
	WFIFOSET(char_fd,86);

	return 0;
}

/**
 * S 2b0e <accid>.l <name>.24B <operation_type>.w <timediff>L <val1>L <val2>L
 * Send an account modification request to the login server (via char server).
 * @aid : Player requesting operation account id
 * @character_name : Target of operation Player name
 * @operation_type : see chrif_req_op
 * @timediff : tick to add or remove to unixtimestamp
 * @val1 : extra data value to transfer for operation
 *		   CHRIF_OP_LOGIN_VIP: 0x1 : Select info and update old_groupid
 *							   0x2 : VIP duration is changed by atcommand or script
 *							   0x4 : Show status reply by char-server through 0x2b0f
 *							   0x8 : First request on player login
 * @val2 : extra data value to transfer for operation
 */
int chrif_req_login_operation(int aid, const char* character_name, enum chrif_req_op operation_type, int32 timediff, int val1, int val2) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,44);
	WFIFOW(char_fd,0) = 0x2b0e;
	WFIFOL(char_fd,2) = aid;
	safestrncpy(WFIFOCP(char_fd,6), character_name, NAME_LENGTH);
	WFIFOW(char_fd,30) = operation_type;

	if ( operation_type == CHRIF_OP_LOGIN_BAN || operation_type == CHRIF_OP_LOGIN_VIP)
		WFIFOL(char_fd,32) = timediff;
	WFIFOL(char_fd,36) = val1;
	WFIFOL(char_fd,40) = val2;
	WFIFOSET(char_fd,44);
	return 0;
}

/**
 * S 2b0e <accid>.l <name>.24B <operation_type>.w <timediff>L <val1>L <val2>L
 * Send a sex change (for account or character) request to the login server (via char server).
 * @sd : Player requesting operation
 */
int chrif_changesex(map_session_data *sd, bool change_account) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,44);
	WFIFOW(char_fd,0) = 0x2b0e;
	WFIFOL(char_fd,2) = sd->status.account_id;
	safestrncpy(WFIFOCP(char_fd,6), sd->status.name, NAME_LENGTH);
	WFIFOW(char_fd,30) = (change_account ? CHRIF_OP_LOGIN_CHANGESEX : CHRIF_OP_CHANGECHARSEX);
	if (!change_account)
		WFIFOB(char_fd,32) = sd->status.sex == SEX_MALE ? SEX_FEMALE : SEX_MALE;
	WFIFOSET(char_fd,44);

	clif_displaymessage(sd->fd, msg_txt(sd,408)); //"Need disconnection to perform change-sex request..."

	if (sd->fd)
		clif_authfail_fd(sd->fd, 15);
	else
		map_quit(sd);
	return 0;
}

/**
 * R 2b0f <accid>.l <name>.24B <type>.w <answer>.w
 * Processing a reply to chrif_req_login_operation() (request to modify an account).
 * NB: That ack is received just after the char has sent the request to login and therefore didn't have login reply yet
 * @param aid : player account id the request was concerning
 * @param player_name : name the request was concerning
 * @param type : code of operation done. See enum chrif_req_op
 * @param answer : type of answer
 *   0: login-server request done
 *   1: player not found
 *   2: gm level too low
 *   3: login-server offline
 */
static void chrif_ack_login_req(int aid, const char* player_name, uint16 type, uint16 answer) {
	map_session_data* sd;
	char action[25];
	char output[256];

	sd = map_id2sd(aid);

	if( aid < 0 || sd == nullptr ) {
		ShowError("chrif_ack_login_req failed - player not online.\n");
		return;
	}

	switch (type) {
		case CHRIF_OP_LOGIN_CHANGESEX:
		case CHRIF_OP_CHANGECHARSEX:
			type = CHRIF_OP_LOGIN_CHANGESEX; // So we don't have to create a new msgstring.
			[[fallthrough]];
		case CHRIF_OP_LOGIN_BLOCK:
		case CHRIF_OP_LOGIN_BAN:
		case CHRIF_OP_LOGIN_UNBLOCK:
		case CHRIF_OP_LOGIN_UNBAN:
			snprintf(action,25,"%s",msg_txt(sd,427+type)); //block|ban|unblock|unban|change the sex of
			break;
		case CHRIF_OP_LOGIN_VIP:
			if (!battle_config.disp_servervip_msg)
				return;
			snprintf(action,25,"%s",msg_txt(sd,436)); //VIP
			break;
		default:
			snprintf(action,25,"???");
			break;
	}

	switch (answer) {
		case 0: sprintf(output, msg_txt(sd,424), action, NAME_LENGTH, player_name); break; //Login-serv has been asked to %s '%.*s'.
		case 1: sprintf(output, msg_txt(sd,425), NAME_LENGTH, player_name); break;
		case 2: sprintf(output, msg_txt(sd,426), action, NAME_LENGTH, player_name); break;
		case 3: sprintf(output, msg_txt(sd,427), action, NAME_LENGTH, player_name); break;
		case 4: sprintf(output, msg_txt(sd,424), action, NAME_LENGTH, player_name); break;
		default: output[0] = '\0'; break;
	}
	clif_displaymessage(sd->fd, output);
}

/*==========================================
 * Request char server to change sex of char (modified by Yor)
 *------------------------------------------*/
int chrif_changedsex(int fd) {
	int acc, sex;
	map_session_data *sd;

	acc = RFIFOL(fd,2);
	sex = RFIFOL(fd,6);

	if ( battle_config.etc_log )
		ShowNotice("chrif_changedsex %d.\n", acc);

	sd = map_id2sd(acc);
	if ( sd ) { //Normally there should not be a char logged on right now!
		if ( sd->status.sex == sex )
			return 0; //Do nothing? Likely safe.
		sd->status.sex = !sd->status.sex;

		// Reset skills of gender split jobs.
		if ((sd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER || (sd->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO) {
			const static struct {
				e_skill start;
				e_skill end;
			} ranges[] = {
				// Bard class exclusive skills
				{ BA_MUSICALLESSON, BA_APPLEIDUN },
				// Dancer class exclusive skills
				{ DC_DANCINGLESSON, DC_SERVICEFORYOU },
				// Minstrel class exclusive skills
				{ MI_RUSH_WINDMILL, MI_HARMONIZE },
				// Wanderer class exclusive skills
				{ WA_SWING_DANCE, WA_MOONLIT_SERENADE },
				// Kagerou class exclusive skills
				{ KG_KAGEHUMI, KG_KAGEMUSYA },
				// Oboro class exclusive skills
				{ OB_ZANGETSU, OB_AKAITSUKI },
			};

			for( const auto& range : ranges ){
				for( uint16 skill_id = range.start; skill_id <= range.end; skill_id++ ){
					uint16 sk_idx = skill_get_index( skill_id );

					if( sd->status.skill[sk_idx].id > 0 && sd->status.skill[sk_idx].flag == SKILL_FLAG_PERMANENT ){
						sd->status.skill_point += sd->status.skill[sk_idx].lv;
						sd->status.skill[sk_idx].id = 0;
						sd->status.skill[sk_idx].lv = 0;
					}
				}
			}

			clif_updatestatus(*sd, SP_SKILLPOINT);
			// Change to other gender version of the job if needed.
			if (sd->status.sex)// Changed from female version of job.
				sd->status.class_ -= 1;
			else// Changed from male version of job.
				sd->status.class_ += 1;
			//sd->class_ Does not need to be updated as both versions of the job are the same.
		}
		// save character
		sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
							  // do same modify in login-server for the account, but no in char-server (it ask again login_id1 to login, and don't remember it)
		clif_displaymessage(sd->fd, msg_txt(sd,409)); //"Your sex has been changed (need disconnection by the server)..."
		set_eof(sd->fd); // forced to disconnect for the change
		map_quit(sd); // Remove leftovers (e.g. autotrading) [Paradox924X]
	}
	return 0;
}
/*==========================================
 * Request Char Server to Divorce Players
 *------------------------------------------*/
int chrif_divorce(int partner_id1, int partner_id2) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b11;
	WFIFOL(char_fd,2) = partner_id1;
	WFIFOL(char_fd,6) = partner_id2;
	WFIFOSET(char_fd,10);

	return 0;
}

/*==========================================
 * Divorce players
 * only used if 'partner_id' is offline
 *------------------------------------------*/
int chrif_divorceack(uint32 char_id, int partner_id) {
	map_session_data* sd;
	int i;

	if( !char_id || !partner_id )
		return 0;

	if( ( sd = map_charid2sd(char_id) ) != nullptr && sd->status.partner_id == partner_id ) {
		sd->status.partner_id = 0;
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_M || sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_F)
				pc_delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
	}

	if( ( sd = map_charid2sd(partner_id) ) != nullptr && sd->status.partner_id == char_id ) {
		sd->status.partner_id = 0;
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_M || sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_F)
				pc_delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
	}

	return 0;
}
/*==========================================
 * Removes Baby from parents
 *------------------------------------------*/
int chrif_deadopt(uint32 father_id, uint32 mother_id, uint32 child_id) {
	map_session_data* sd;
	uint16 idx = skill_get_index(WE_CALLBABY);

	if( father_id && ( sd = map_charid2sd(father_id) ) != nullptr && sd->status.child == child_id ) {
		sd->status.child = 0;
		sd->status.skill[idx].id = 0;
		sd->status.skill[idx].lv = 0;
		sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
		clif_deleteskill(sd,WE_CALLBABY);
	}

	if( mother_id && ( sd = map_charid2sd(mother_id) ) != nullptr && sd->status.child == child_id ) {
		sd->status.child = 0;
		sd->status.skill[idx].id = 0;
		sd->status.skill[idx].lv = 0;
		sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
		clif_deleteskill(sd,WE_CALLBABY);
	}

	return 0;
}

/*==========================================
 * Disconnection of a player (account has been banned of has a status, from login/char-server) by [Yor]
 *------------------------------------------*/
int chrif_ban(int fd) {
	int id, res=0;
	map_session_data *sd;

	id = RFIFOL(fd,2);
	res = RFIFOB(fd,6); // 0: change of statut, 1: ban, 2 charban

	if ( battle_config.etc_log )
		ShowNotice("chrif_ban %d.type = %s \n", id, res==1?"account":"char");

	if(res==2)  sd = map_charid2sd(id);
	else sd = map_id2sd(id);

	if ( id < 0 || sd == nullptr ) {
		//nothing to do on map if player not connected
		return 0;
	}

	sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
	if (res == 0) { 
		int ret_status = RFIFOL(fd,7); // status or final date of a banishment
		if(0<ret_status && ret_status<=9)
			clif_displaymessage(sd->fd, msg_txt(sd,411+ret_status));
		else if(ret_status==100)
			clif_displaymessage(sd->fd, msg_txt(sd,421));
		 else
			clif_displaymessage(sd->fd, msg_txt(sd,420)); //"Your account has not more authorised."
	} else if (res == 1 || res == 2) {
		time_t timestamp;
		char tmpstr[256];
		char strtime[25];
		timestamp = (time_t)RFIFOL(fd,7); // status or final date of a banishment
		strftime(strtime, 24, "%d-%m-%Y %H:%M:%S", localtime(&timestamp));
		safesnprintf(tmpstr,sizeof(tmpstr),msg_txt(sd,423),res==2?"char":"account",strtime); //"Your %s has been banished until %s "
		clif_displaymessage(sd->fd, tmpstr);
	}

	set_eof(sd->fd); // forced to disconnect for the change
	map_quit(sd); // Remove leftovers (e.g. autotrading) [Paradox924X]
	return 0;
}

int chrif_req_charban(int aid, const char* character_name, int32 timediff){
	chrif_check(-1);
	
	WFIFOHEAD(char_fd,10+NAME_LENGTH);
	WFIFOW(char_fd,0) = 0x2b28;
	WFIFOL(char_fd,2) = aid;
	WFIFOL(char_fd,6) = timediff;
	safestrncpy(WFIFOCP(char_fd,10), character_name, NAME_LENGTH);
	WFIFOSET(char_fd,10+NAME_LENGTH); //default 34
	return 0;
}

int chrif_req_charunban(int aid, const char* character_name){
	chrif_check(-1);
	
	WFIFOHEAD(char_fd,6+NAME_LENGTH);
	WFIFOW(char_fd,0) = 0x2b2a;
	WFIFOL(char_fd,2) = aid;
	safestrncpy(WFIFOCP(char_fd,6), character_name, NAME_LENGTH);
	WFIFOSET(char_fd,6+NAME_LENGTH);
	return 0;
}

//Disconnect the player out of the game, simple packet
//packet.w AID.L WHY.B 2+4+1 = 7byte
int chrif_disconnectplayer(int fd) {
	map_session_data* sd;
	uint32 account_id = RFIFOL(fd, 2);

	sd = map_id2sd(account_id);
	if( sd == nullptr ) {
		struct auth_node* auth = chrif_search(account_id);

		if( auth != nullptr && chrif_auth_delete(account_id, auth->char_id, ST_LOGIN) )
			return 0;

		return -1;
	}

	if (!sd->fd) {
		if (sd->state.autotrade)
			map_quit(sd);
		//Else we don't remove it because the char should have a timer to remove the player because it force-quit before,
		//and we don't want them kicking their previous instance before the 10 secs penalty time passes. [Skotlex]
		return 0;
	}

	switch(RFIFOB(fd, 6)) {
		case 1: clif_authfail_fd(sd->fd, 1); break; //server closed
		case 2: clif_authfail_fd(sd->fd, 2); break; //someone else logged in
		case 3: clif_authfail_fd(sd->fd, 4); break; //server overpopulated
		case 4: clif_authfail_fd(sd->fd, 10); break; //out of available time paid for
		case 5: clif_authfail_fd(sd->fd, 15); break; //forced to dc by gm
	}
	return 0;
}

/*==========================================
 * Request/Receive top 10 Fame character list
 *------------------------------------------*/
int chrif_updatefamelist(map_session_data &sd, e_rank ranktype) {
	chrif_check(-1);

	WFIFOHEAD(char_fd, 11);
	WFIFOW(char_fd,0) = 0x2b10;
	WFIFOL(char_fd,2) = sd.status.char_id;
	WFIFOL(char_fd,6) = sd.status.fame;
	WFIFOB(char_fd,10) = ranktype;
	WFIFOSET(char_fd,11);

	return 0;
}

int chrif_buildfamelist(void) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,2);
	WFIFOW(char_fd,0) = 0x2b1a;
	WFIFOSET(char_fd,2);

	return 0;
}

int chrif_recvfamelist(int fd) {
	int num, size;
	int total = 0, len = 8;

	memset (smith_fame_list, 0, sizeof(smith_fame_list));
	memset (chemist_fame_list, 0, sizeof(chemist_fame_list));
	memset (taekwon_fame_list, 0, sizeof(taekwon_fame_list));

	size = RFIFOW(fd, 6); //Blacksmith block size

	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&smith_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}

	total += num;

	size = RFIFOW(fd, 4); //Alchemist block size

	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&chemist_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}

	total += num;

	size = RFIFOW(fd, 2); //Total packet length

	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&taekwon_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}

	total += num;

	ShowInfo("Received Fame List of '" CL_WHITE "%d" CL_RESET "' characters.\n", total);

	return 0;
}

/// fame ranking update confirmation
/// R 2b22 <table>.B <index>.B <value>.L
int chrif_updatefamelist_ack(int fd) {
	struct fame_list* list;
	uint8 index;

	switch (RFIFOB(fd,2)) {
		case RANK_BLACKSMITH:	list = smith_fame_list;   break;
		case RANK_ALCHEMIST:	list = chemist_fame_list; break;
		case RANK_TAEKWON:		list = taekwon_fame_list; break;
		default: return 0;
	}

	index = RFIFOB(fd, 3);

	if (index >= MAX_FAME_LIST)
		return 0;

	list[index].fame = RFIFOL(fd,4);

	return 1;
}

int chrif_save_scdata(map_session_data *sd) { //parses the sc_data of the player and sends it to the char-server for saving. [Skotlex]
#ifdef ENABLE_SC_SAVING
	int i, count=0;
	t_tick tick;
	struct status_change_data data;
	status_change *sc = &sd->sc;
	const struct TimerData *timer;

	chrif_check(-1);
	tick = gettick();

	WFIFOHEAD(char_fd, 14 + SC_MAX*sizeof(struct status_change_data));
	WFIFOW(char_fd,0) = 0x2b1c;
	WFIFOL(char_fd,4) = sd->status.account_id;
	WFIFOL(char_fd,8) = sd->status.char_id;

	for (i = 0; i < SC_MAX; i++) {
		auto sce = sc->getSCE(static_cast<sc_type>(i));
		if (!sce)
			continue;
		if (sce->timer != INVALID_TIMER) {
			timer = get_timer(sce->timer);
			if (timer == nullptr || timer->func != status_change_timer)
				continue;
			if (DIFF_TICK(timer->tick,tick) > 0)
				data.tick = DIFF_TICK(timer->tick,tick); //Duration that is left before ending.
			else
				data.tick = 0; //Negative tick does not necessarily mean that sc has expired
		} else
			data.tick = INFINITE_TICK; //Infinite duration
		data.type = i;
		data.val1 = sce->val1;
		data.val2 = sce->val2;
		data.val3 = sce->val3;
		data.val4 = sce->val4;
		memcpy(WFIFOP(char_fd,14 +count*sizeof(struct status_change_data)),
			&data, sizeof(struct status_change_data));
		count++;
	}

	WFIFOW(char_fd,12) = count;
	// Total packet size
	WFIFOW( char_fd, 2 ) = static_cast<int16>( 14 + count * sizeof( struct status_change_data ) );
	WFIFOSET(char_fd,WFIFOW(char_fd,2));
#endif
	return 0;
}

int chrif_skillcooldown_save(map_session_data *sd) {
	int i, count = 0;
	struct skill_cooldown_data data;
	t_tick tick;
	const struct TimerData *timer;

	chrif_check(-1);
	tick = gettick();

	WFIFOHEAD(char_fd, 14 + MAX_SKILLCOOLDOWN * sizeof (struct skill_cooldown_data));
	WFIFOW(char_fd, 0) = 0x2b15;
	WFIFOL(char_fd, 4) = sd->status.account_id;
	WFIFOL(char_fd, 8) = sd->status.char_id;
	for (i = 0; i < MAX_SKILLCOOLDOWN; i++) {
		if (!sd->scd[i])
			continue;

		if (battle_config.guild_skill_relog_type == 1 && SKILL_CHK_GUILD(sd->scd[i]->skill_id))
			continue;

		timer = get_timer(sd->scd[i]->timer);
		if (timer == nullptr || timer->func != skill_blockpc_end || DIFF_TICK(timer->tick, tick) < 0)
			continue;

		data.tick = DIFF_TICK(timer->tick, tick);
		data.skill_id = sd->scd[i]->skill_id;
		memcpy(WFIFOP(char_fd, 14 + count * sizeof (struct skill_cooldown_data)), &data, sizeof (struct skill_cooldown_data));
		count++;
	}
	if (count == 0)
		return 0;

	WFIFOW(char_fd, 12) = count;
	WFIFOW( char_fd, 2 ) = static_cast<int16>( 14 + count * sizeof( struct skill_cooldown_data ) );
	WFIFOSET(char_fd, WFIFOW(char_fd, 2));

	return 0;
}

//Retrieve and load sc_data for a player. [Skotlex]
int chrif_load_scdata(int fd) {

#ifdef ENABLE_SC_SAVING
	map_session_data *sd;
	int aid, cid, i, count;

	aid = RFIFOL(fd,4); //Player Account ID
	cid = RFIFOL(fd,8); //Player Char ID

	sd = map_id2sd(aid);

	if ( !sd ) {
		ShowError("chrif_load_scdata: Player of AID %d not found!\n", aid);
		return -1;
	}

	if ( sd->status.char_id != cid ) {
		ShowError("chrif_load_scdata: Receiving data for account %d, char id does not matches (%d != %d)!\n", aid, sd->status.char_id, cid);
		return -1;
	}

	count = RFIFOW(fd,12); //sc_count

	for (i = 0; i < count; i++) {
		struct status_change_data *data = (struct status_change_data*)RFIFOP(fd,14 + i*sizeof(struct status_change_data));

		status_change_start(nullptr,&sd->bl, (sc_type)data->type, 10000, data->val1, data->val2, data->val3, data->val4, data->tick, SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_LOADED|SCSTART_NORATEDEF);
	}

	pc_scdata_received(sd);
#endif
	return 0;
}

//Retrieve and load skillcooldown for a player

int chrif_skillcooldown_load(int fd) {
	map_session_data *sd;
	int aid, cid, i, count;

	aid = RFIFOL(fd, 4);
	cid = RFIFOL(fd, 8);

	sd = map_id2sd(aid);
	if (!sd) {
		ShowError("chrif_skillcooldown_load: Player of AID %d not found!\n", aid);
		return -1;
	}
	if (sd->status.char_id != cid) {
		ShowError("chrif_skillcooldown_load: Receiving data for account %d, char id does not matches (%d != %d)!\n", aid, sd->status.char_id, cid);
		return -1;
	}
	count = RFIFOW(fd, 12); //sc_count
	for (i = 0; i < count; i++) {
		struct skill_cooldown_data *data = (struct skill_cooldown_data*) RFIFOP(fd, 14 + i * sizeof (struct skill_cooldown_data));
			skill_blockpc_start(sd, data->skill_id, data->tick);
	}
	return 0;
}

/*=========================================
 * Tell char-server charcter disconnected [Wizputer]
 *-----------------------------------------*/
int chrif_char_offline(map_session_data *sd) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b17;
	WFIFOL(char_fd,2) = sd->status.char_id;
	WFIFOL(char_fd,6) = sd->status.account_id;
	WFIFOSET(char_fd,10);

	return 0;
}
int chrif_char_offline_nsd(uint32 account_id, uint32 char_id) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b17;
	WFIFOL(char_fd,2) = char_id;
	WFIFOL(char_fd,6) = account_id;
	WFIFOSET(char_fd,10);

	return 0;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------*/
int chrif_flush_fifo(void) {
	chrif_check(-1);

	set_nonblocking(char_fd, 0);
	flush_fifos();
	set_nonblocking(char_fd, 1);

	return 0;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------*/
int chrif_char_reset_offline(void) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,2);
	WFIFOW(char_fd,0) = 0x2b18;
	WFIFOSET(char_fd,2);

	return 0;
}

/*=========================================
 * Tell char-server charcter is online [Wizputer]
 *-----------------------------------------*/

int chrif_char_online(map_session_data *sd) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b19;
	WFIFOL(char_fd,2) = sd->status.char_id;
	WFIFOL(char_fd,6) = sd->status.account_id;
	WFIFOSET(char_fd,10);

	return 0;
}


/// Called when the connection to Char Server is disconnected.
void chrif_on_disconnect(void) {
	if( chrif_connected != 1 )
		ShowWarning("Connection to Char Server lost.\n\n");
	chrif_connected = 0;

	other_mapserver_count = 0; //Reset counter. We receive ALL maps from all map-servers on reconnect.
	map_eraseallipport();

	//Attempt to reconnect in a second. [Skotlex]
	add_timer(gettick() + 1000, check_connect_char_server, 0, 0);
}

/**
 * !CHECKME: This is intended?
 * On sync request received, map-server only send its own IP
 * without change the char IP (if any)?
 * Since no IP info sent by 0x2b1e (chlogif_parse_updip)
 **/
void chrif_update_ip(int fd) {
	uint32 new_ip;

	WFIFOHEAD(fd,6);

	new_ip = host2ip(char_ip_str);

	if (new_ip && new_ip != char_ip)
		char_ip = new_ip; //Update char_ip

	new_ip = clif_refresh_ip();

	if (!new_ip)
		return; //No change

	WFIFOW(fd,0) = 0x2b13;
	WFIFOL(fd,2) = htonl(new_ip);
	WFIFOSET(fd,6);
}

// pings the charserver
void chrif_keepalive(int fd) {
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2b23;
	WFIFOSET(fd,2);
}

void chrif_keepalive_ack(int fd) {
	session[fd]->flag.ping = 0;/* reset ping state, we received a packet */
}

/**
 * Received vip-data from char-serv, fill map-serv data
 * @param fd : char-serv file descriptor (link to char-serv)
 */
void chrif_parse_ack_vipActive(int fd) {
#ifdef VIP_ENABLE
	int aid = RFIFOL(fd,2);
	uint32 vip_time = RFIFOL(fd,6);
	uint32 groupid = RFIFOL(fd,10);
	uint8 flag = RFIFOB(fd,14);
	TBL_PC *sd = map_id2sd(aid);
	bool changed = false;

	if(sd == nullptr) return;

	sd->group_id = groupid;
	pc_group_pc_load(sd);

	if ((flag&0x2)) //isgm
		clif_displaymessage(sd->fd,msg_txt(sd,437));
	else {
		changed = (sd->vip.enabled != (flag&0x1));
		if((flag&0x1)) { //isvip
			sd->vip.enabled = 1;
			sd->vip.time = vip_time;
			// Increase storage size for VIP.
			sd->storage.max_amount = battle_config.vip_storage_increase + MIN_STORAGE;
			if (sd->storage.max_amount > MAX_STORAGE) {
				ShowError("intif_parse_ack_vipActive: Storage size for player %s (%d:%d) is larger than MAX_STORAGE. Storage size has been set to MAX_STORAGE.\n", sd->status.name, sd->status.account_id, sd->status.char_id);
				sd->storage.max_amount = MAX_STORAGE;
			}
		} else if (sd->vip.enabled) {
			sd->vip.enabled = 0;
			sd->vip.time = 0;
			sd->storage.max_amount = MIN_STORAGE;
			sd->special_state.no_gemstone = 0;
			clif_displaymessage(sd->fd,msg_txt(sd,438));
		}
	}
	// Show info if status changed
	if (((flag&0x4) || changed) && !sd->vip.disableshowrate) {
		clif_display_pinfo( *sd );
	}
#endif
}


/**
 * ZA 0x2b2d
 * <cmd>.W <char_id>.L
 * Requets bonus_script datas
 * @param char_id
 * @author [Cydh]
 **/
int chrif_bsdata_request(uint32 char_id) {
	chrif_check(-1);
	WFIFOHEAD(char_fd,6);
	WFIFOW(char_fd,0) = 0x2b2d;
	WFIFOL(char_fd,2) = char_id;
	WFIFOSET(char_fd,6);
	return 0;
}

/**
 * ZA 0x2b2e
 * <cmd>.W <len>.W <char_id>.L <count>.B { <bonus_script>.?B }
 * Stores bonus_script data(s) to the table
 * @param sd
 * @author [Cydh]
 **/
int chrif_bsdata_save(map_session_data *sd, bool quit) {
	uint8 i = 0;

	chrif_check(-1);

	if (!sd)
		return 0;

	// Removing...
	if (quit && sd->bonus_script.head) {
		uint32 flag = BSF_REM_ON_LOGOUT; //Remove bonus when logout

		if (battle_config.debuff_on_logout&1) //Remove negative buffs
			flag |= BSF_REM_DEBUFF;
		if (battle_config.debuff_on_logout&2) //Remove positive buffs
			flag |= BSF_REM_BUFF;
		pc_bonus_script_clear(sd, flag);
	}

	//ShowInfo("Saving %d bonus script for CID=%d\n", sd->bonus_script.count, sd->status.char_id);

	WFIFOHEAD(char_fd, 9 + sd->bonus_script.count * sizeof(struct bonus_script_data));
	WFIFOW(char_fd, 0) = 0x2b2e;
	WFIFOL(char_fd, 4) = sd->status.char_id;

	if (sd->bonus_script.count) {
		t_tick tick = gettick();
		struct linkdb_node *node = nullptr;

		for (node = sd->bonus_script.head; node && i < MAX_PC_BONUS_SCRIPT; node = node->next) {
			const struct TimerData *timer = nullptr;
			struct bonus_script_data bs;
			struct s_bonus_script_entry *entry = (struct s_bonus_script_entry *)node->data;

			if (!entry || !(timer = get_timer(entry->tid)) || DIFF_TICK(timer->tick,tick) < 0)
				continue;

			memset(&bs, 0, sizeof(bs));
			safestrncpy(bs.script_str, StringBuf_Value(entry->script_buf), StringBuf_Length(entry->script_buf)+1);
			bs.tick = DIFF_TICK(timer->tick, tick);
			bs.flag = entry->flag;
			bs.type = entry->type;
			bs.icon = entry->icon;
			memcpy(WFIFOP(char_fd, 9 + i * sizeof(struct bonus_script_data)), &bs, sizeof(struct bonus_script_data));
			i++;
		}

		if (i != sd->bonus_script.count && sd->bonus_script.count > MAX_PC_BONUS_SCRIPT)
			ShowWarning("Only allowed to save %d (mmo.hpp::MAX_PC_BONUS_SCRIPT) bonus script each player.\n", MAX_PC_BONUS_SCRIPT);
	}

	WFIFOB(char_fd, 8) = i;
	WFIFOW(char_fd, 2) = 9 + sd->bonus_script.count * sizeof(struct bonus_script_data);
	WFIFOSET(char_fd, WFIFOW(char_fd, 2));

	return 0;
}

/**
 * AZ 0x2b2f
 * <cmd>.W <len>.W <cid>.L <count>.B { <bonus_script_data>.?B }
 * Bonus script received, set to player
 * @param fd
 * @author [Cydh]
 **/
int chrif_bsdata_received(int fd) {
	map_session_data *sd;
	uint32 cid = RFIFOL(fd,4);
	uint8 count = 0;

	sd = map_charid2sd(cid);

	if (!sd) {
		ShowError("chrif_bsdata_received: Player with CID %d not found!\n",cid);
		return -1;
	}

	if ((count = RFIFOB(fd,8))) {
		uint8 i = 0;

		//ShowInfo("Loaded %d bonus script for CID=%d\n", count, sd->status.char_id);

		for (i = 0; i < count; i++) {
			struct bonus_script_data *bs = (struct bonus_script_data*)RFIFOP(fd,9 + i*sizeof(struct bonus_script_data));
			struct s_bonus_script_entry *entry = nullptr;

			if (bs->script_str[0] == '\0' || !bs->tick)
				continue;

			if (!(entry = pc_bonus_script_add(sd, bs->script_str, bs->tick, (enum efst_type)bs->icon, bs->flag, bs->type)))
				continue;

			linkdb_insert(&sd->bonus_script.head, (void *)((intptr_t)entry), entry);
		}

		if (sd->bonus_script.head)
			status_calc_pc(sd,SCO_NONE);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int chrif_parse(int fd) {
	int packet_len;

	// only process data from the char-server
	if ( fd != char_fd ) {
		ShowDebug("chrif_parse: Disconnecting invalid session #%d (is not the char-server)\n", fd);
		do_close(fd);
		return 0;
	}

	if ( session[fd]->flag.eof ) {
		do_close(fd);
		char_fd = -1;
		chrif_on_disconnect();
		return 0;
	} else if ( session[fd]->flag.ping ) {/* we've reached stall time */
		if( DIFF_TICK(last_tick, session[fd]->rdata_tick) > (stall_time * 2) ) {/* we can't wait any longer */
			set_eof(fd);
			return 0;
		} else if( session[fd]->flag.ping != 2 ) { /* we haven't sent ping out yet */
			chrif_keepalive(fd);
			session[fd]->flag.ping = 2;
		}
	}

	while ( RFIFOREST(fd) >= 2 ) {
		int cmd = RFIFOW(fd,0);
		if (cmd < 0x2af8 || cmd >= 0x2af8 + ARRAYLENGTH(packet_len_table) || packet_len_table[cmd-0x2af8] == 0) {
			int r = intif_parse(fd); // Passed on to the intif

			if (r == 1) continue;	// Treated in intif
			if (r == 2) return 0;	// Didn't have enough data (len==-1)

			ShowWarning("chrif_parse: session #%d, intif_parse failed (unrecognized command 0x%.4x).\n", fd, cmd);
			set_eof(fd);
			return 0;
		}

		if ( ( packet_len = packet_len_table[cmd-0x2af8] ) == -1) { // dynamic-length packet, second WORD holds the length
			if (RFIFOREST(fd) < 4)
				return 0;
			packet_len = RFIFOW(fd,2);
		}

		if ((int)RFIFOREST(fd) < packet_len)
			return 0;

		//ShowDebug("Received packet 0x%4x (%d bytes) from char-server (connection %d)\n", RFIFOW(fd,0), packet_len, fd);

		switch(cmd) {
			case 0x2af9: chrif_connectack(fd); break;
			case 0x2afb: chrif_sendmapack(fd); break;
			case 0x2afd: chrif_authok(fd); break;
			case 0x2b00: map_setusers(RFIFOL(fd,2)); chrif_keepalive(fd); break;
			case 0x2b03: clif_charselectok(RFIFOL(fd,2), RFIFOB(fd,6)); break;
			case 0x2b04: chrif_recvmap(fd); break;
			case 0x2b06: chrif_changemapserverack( RFIFOL( fd, 2 ), RFIFOL( fd, 6 ), RFIFOL( fd, 10 ), RFIFOL( fd, 14 ), RFIFOCP( fd, 18 ), RFIFOW( fd, 18 + MAP_NAME_LENGTH_EXT ), RFIFOW( fd, 18 + MAP_NAME_LENGTH_EXT + 2 ), RFIFOL( fd, 18 + MAP_NAME_LENGTH_EXT + 4 ), RFIFOW( fd, 18 + MAP_NAME_LENGTH_EXT + 8 ) ); break;
			case 0x2b09: map_addnickdb(RFIFOL(fd,2), RFIFOCP(fd,6)); break;
			case 0x2b0b: chrif_skillcooldown_load(fd); break;
			case 0x2b0d: chrif_changedsex(fd); break;
			case 0x2b0f: chrif_ack_login_req(RFIFOL(fd,2), RFIFOCP(fd,6), RFIFOW(fd,30), RFIFOW(fd,32)); break;
			case 0x2b12: chrif_divorceack(RFIFOL(fd,2), RFIFOL(fd,6)); break;
			case 0x2b14: chrif_ban(fd); break;
			case 0x2b1b: chrif_recvfamelist(fd); break;
			case 0x2b1d: chrif_load_scdata(fd); break;
			case 0x2b1e: chrif_update_ip(fd); break;
			case 0x2b1f: chrif_disconnectplayer(fd); break;
			case 0x2b20: chrif_removemap(fd); break;
			case 0x2b21: chrif_save_ack(fd); break;
			case 0x2b22: chrif_updatefamelist_ack(fd); break;
			case 0x2b24: chrif_keepalive_ack(fd); break;
			case 0x2b25: chrif_deadopt(RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
			case 0x2b27: chrif_authfail(fd); break;
			case 0x2b2b: chrif_parse_ack_vipActive(fd); break;
			case 0x2b2f: chrif_bsdata_received(fd); break;
			default:
				ShowError("chrif_parse : unknown packet (session #%d): 0x%x. Disconnecting.\n", fd, cmd);
				set_eof(fd);
				return 0;
		}
		if ( fd == char_fd ) //There's the slight chance we lost the connection during parse, in which case this would segfault if not checked [Skotlex]
			RFIFOSKIP(fd, packet_len);
	}

	return 0;
}

// unused
TIMER_FUNC(send_usercount_tochar){
	chrif_check(-1);

	WFIFOHEAD(char_fd,4);
	WFIFOW(char_fd,0) = 0x2afe;
	WFIFOW(char_fd,2) = map_usercount();
	WFIFOSET(char_fd,4);
	return 0;
}

/*==========================================
 * timerFunction
 * Send to char the number of client connected to map
 *------------------------------------------*/
int send_users_tochar(void) {
	int users = 0, i = 0;
	map_session_data* sd;
	struct s_mapiterator* iter;

	chrif_check(-1);

	users = map_usercount();

	WFIFOHEAD(char_fd, 6+8*users);
	WFIFOW(char_fd,0) = 0x2aff;

	iter = mapit_getallusers();

	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) ) {
		WFIFOL(char_fd,6+8*i) = sd->status.account_id;
		WFIFOL(char_fd,6+8*i+4) = sd->status.char_id;
		i++;
	}

	mapit_free(iter);

	WFIFOW(char_fd,2) = 6 + 8*users;
	WFIFOW(char_fd,4) = users;
	WFIFOSET(char_fd, 6+8*users);

	return 0;
}

/*==========================================
 * timerFunction
  * Chk the connection to char server, (if it down)
 *------------------------------------------*/
static TIMER_FUNC(check_connect_char_server){
	static int displayed = 0;
	if ( char_fd <= 0 || session[char_fd] == nullptr ) {
		if ( !displayed ) {
			ShowStatus("Attempting to connect to Char Server. Please wait.\n");
			displayed = 1;
		}

		chrif_state = 0;
		char_fd = make_connection(char_ip, char_port,false,10);

		if (char_fd == -1)//Attempt to connect later. [Skotlex]
			return 0;

		session[char_fd]->func_parse = chrif_parse;
		session[char_fd]->flag.server = 1;
		realloc_fifo(char_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

		chrif_connect(char_fd);
		chrif_connected = (chrif_state == 2);
	}
	if ( chrif_isconnected() )
		displayed = 0;
	return 0;
}

/*==========================================
 * Asks char server to remove friend_id from the friend list of char_id
 *------------------------------------------*/
int chrif_removefriend(uint32 char_id, int friend_id) {

	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b07;
	WFIFOL(char_fd,2) = char_id;
	WFIFOL(char_fd,6) = friend_id;
	WFIFOSET(char_fd,10);

	return 0;
}

/**
 * @see DBApply
 */
int auth_db_final(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = (struct auth_node *)db_data2ptr(data);

	if (node->char_dat)
		aFree(node->char_dat);

	if (node->sd) {
		if (node->sd->regs.vars)
			node->sd->regs.vars->destroy(node->sd->regs.vars, script_reg_destroy);

		if (node->sd->regs.arrays)
			node->sd->regs.arrays->destroy(node->sd->regs.arrays, script_free_array_db);

		node->sd->~map_session_data();
		aFree(node->sd);
	}

	ers_free(auth_db_ers, node);

	return 0;
}

/*==========================================
 * Destructor
 *------------------------------------------*/
void do_final_chrif(void) {

	if( char_fd != -1 ) {
		do_close(char_fd);
		char_fd = -1;
	}

	auth_db->destroy(auth_db, auth_db_final);

	ers_destroy(auth_db_ers);
}

/*==========================================
 *
 *------------------------------------------*/
void do_init_chrif(void) {
	if(sizeof(struct mmo_charstatus) > 0xFFFF){
		ShowError("mmo_charstatus size = %" PRIuPTR " is too big to be transmitted. (must be below 0xFFFF)\n",
			sizeof(struct mmo_charstatus));
		exit(EXIT_FAILURE);
	}

	if (sizeof(struct s_storage) > 0xFFFF) {
		ShowError("s_storage size = %" PRIuPTR " is too big to be transmitted. (must be below 0xFFFF)\n", sizeof(struct s_storage));
		exit(EXIT_FAILURE);
	}

	if((sizeof(struct bonus_script_data) * MAX_PC_BONUS_SCRIPT) > 0xFFFF){
		ShowError("bonus_script_data size = %d is too big, please reduce MAX_PC_BONUS_SCRIPT (%d) size. (must be below 0xFFFF).\n",
			(sizeof(struct bonus_script_data) * MAX_PC_BONUS_SCRIPT), MAX_PC_BONUS_SCRIPT);
		exit(EXIT_FAILURE);
	}

	auth_db = idb_alloc(DB_OPT_BASE);
	auth_db_ers = ers_new(sizeof(struct auth_node),"chrif.cpp::auth_db_ers",ERS_OPT_NONE);

	add_timer_func_list(check_connect_char_server, "check_connect_char_server");
	add_timer_func_list(auth_db_cleanup, "auth_db_cleanup");

	// establish map-char connection if not present
	add_timer_interval(gettick() + 1000, check_connect_char_server, 0, 0, 10 * 1000);

	// wipe stale data for timed-out client connection requests
	add_timer_interval(gettick() + 1000, auth_db_cleanup, 0, 0, 30 * 1000);

	// send the user count every 10 seconds, to hide the charserver's online counting problem
	add_timer_interval(gettick() + 1000, send_usercount_tochar, 0, 0, UPDATE_INTERVAL);
}
