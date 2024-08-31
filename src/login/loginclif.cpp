// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "loginclif.hpp"

#include <cstdlib>
#include <cstring>

#include <common/malloc.hpp>
#include <common/md5calc.hpp>
#include <common/packets.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp> //show notice
#include <common/socket.hpp> //wfifo session
#include <common/strlib.hpp> //safeprint
#include <common/timer.hpp> //difftick
#include <common/utils.hpp>

#include "account.hpp"
#include "ipban.hpp" //ipban_check
#include "login.hpp"
#include "loginchrif.hpp"
#include "loginlog.hpp"

/**
 * Transmit auth result to client.
 * @param fd: client file desciptor link
 * @param result: result to transmit to client, see below
 *  1 : Server closed
 *  2 : Someone has already logged in with this id
 *  8 : already online
 * <result>.B (SC_NOTIFY_BAN)
 */
static void logclif_sent_auth_result(int fd,char result){
	PACKET_SC_NOTIFY_BAN p = {};

	p.packetType = HEADER_SC_NOTIFY_BAN;
	p.result = result;

	socket_send( fd, p );
}

/**
 * Auth successful, inform client and create a temp auth_node.
 * @param sd: player session
 */
static void logclif_auth_ok(struct login_session_data* sd) {
	int fd = sd->fd;
	uint32 ip = session[fd]->client_addr;

	uint8 server_num, n;
	uint32 subnet_char_ip;
	int i;

	if( !global_core->is_running() ){
		// players can only login while running
		logclif_sent_auth_result(fd,1); // server closed
		return;
	}

	if( login_config.group_id_to_connect >= 0 && sd->group_id != login_config.group_id_to_connect ) {
		ShowStatus("Connection refused: the required group id for connection is %d (account: %s, group: %d).\n", login_config.group_id_to_connect, sd->userid, sd->group_id);
		logclif_sent_auth_result(fd,1); // server closed
		return;
	} else if( login_config.min_group_id_to_connect >= 0 && login_config.group_id_to_connect == -1 && sd->group_id < login_config.min_group_id_to_connect ) {
		ShowStatus("Connection refused: the minimum group id required for connection is %d (account: %s, group: %d).\n", login_config.min_group_id_to_connect, sd->userid, sd->group_id);
		logclif_sent_auth_result(fd,1); // server closed
		return;
	}

	server_num = 0;
	for( i = 0; i < ARRAYLENGTH(ch_server); ++i )
		if( session_isActive(ch_server[i].fd) )
			server_num++;

	if( server_num == 0 )
	{// if no char-server, don't send void list of servers, just disconnect the player with proper message
		ShowStatus("Connection refused: there is no char-server online (account: %s).\n", sd->userid);
		logclif_sent_auth_result(fd,1); // server closed
		return;
	}

	{
		struct online_login_data* data = login_get_online_user( sd->account_id );

		if( data )
		{// account is already marked as online!
			if( data->char_server > -1 )
			{// Request char servers to kick this account out. [Skotlex]
				uint8 buf[6];
				ShowNotice("User '%s' is already online - Rejected.\n", sd->userid);
				WBUFW(buf,0) = 0x2734;
				WBUFL(buf,2) = sd->account_id;
				logchrif_sendallwos(-1, buf, 6);
				if( data->waiting_disconnect == INVALID_TIMER )
					data->waiting_disconnect = add_timer(gettick()+AUTH_TIMEOUT, login_waiting_disconnect_timer, sd->account_id, 0);
				logclif_sent_auth_result(fd,8); // 08 = Server still recognizes your last login
				return;
			}
			else
			if( data->char_server == -1 )
			{// client has authed but did not access char-server yet
				// wipe previous session
				login_remove_auth_node(sd->account_id);
				login_remove_online_user(sd->account_id);
				data = nullptr;
			}
		}
	}

	login_log(ip, sd->userid, 100, "login ok");
	ShowStatus("Connection of the account '%s' accepted.\n", sd->userid);

	PACKET_AC_ACCEPT_LOGIN* p = reinterpret_cast<PACKET_AC_ACCEPT_LOGIN*>( packet_buffer );

	p->packetType = HEADER_AC_ACCEPT_LOGIN;
	p->packetLength = sizeof( *p );
	p->login_id1 = sd->login_id1;
	p->AID = sd->account_id;
	p->login_id2 = sd->login_id2;
	// in old version, that was for ip (not more used)
	p->last_ip = 0;
	// in old version, that was for last login time (not more used)
	safestrncpy( p->last_login, "", sizeof( p->last_login ) );
	p->sex = sex_str2num( sd->sex );
#if PACKETVER >= 20170315
	safestrncpy( p->token, sd->web_auth_token, WEB_AUTH_TOKEN_LENGTH ); // web authentication token
#endif

	for( i = 0, n = 0; i < ARRAYLENGTH(ch_server); ++i ) {
		if( !session_isValid(ch_server[i].fd) )
			continue;
		subnet_char_ip = lan_subnetcheck(ip); // Advanced subnet check [LuzZza]

		PACKET_AC_ACCEPT_LOGIN_sub& char_server = p->char_servers[n];

		char_server.ip = htonl( ( subnet_char_ip ) ? subnet_char_ip : ch_server[i].ip );
		char_server.port = ntows( htons( ch_server[i].port ) ); // [!] LE byte order here [!]
		safestrncpy( char_server.name, ch_server[i].name, sizeof( char_server.name ) );
		char_server.users = login_get_usercount( ch_server[i].users );
		char_server.type = ch_server[i].type;
		char_server.new_ = ch_server[i].new_;
#if PACKETVER >= 20170315
		memset( &char_server.unknown, 0, sizeof( char_server.unknown ) );
#endif

#ifdef DEBUG
		ShowDebug(
			"Sending the client (%d %d.%d.%d.%d) to char-server %s with ip %d.%d.%d.%d and port "
			"%hu\n",
			sd->account_id, CONVIP(ip), ch_server[i].name,
			CONVIP((subnet_char_ip) ? subnet_char_ip : ch_server[i].ip), ch_server[i].port);
#endif

		n++;
		p->packetLength += sizeof( char_server );
	}

	socket_send( fd, p );

	// create temporary auth entry
	login_add_auth_node( sd, ip );

	// mark client as 'online'
	struct online_login_data* data = login_add_online_user(-1, sd->account_id);
	// schedule deletion of this node
	data->waiting_disconnect = add_timer(gettick()+AUTH_TIMEOUT, login_waiting_disconnect_timer, sd->account_id, 0);
}

static void logclif_auth_failed( int fd, int result, const char* unblock_time = "" ){
	PACKET_AC_REFUSE_LOGIN p = {};

	p.packetType = HEADER_AC_REFUSE_LOGIN;
	p.error = result;
	safestrncpy( p.unblock_time, "", sizeof( p.unblock_time ) );

	socket_send( fd, p );
}

/**
 * Inform client that auth has failed.
 * @param sd: player session
 * @param result: nb (msg define in conf)
    0 = Unregistered ID
    1 = Incorrect Password
    2 = This ID is expired
    3 = Rejected from Server
    4 = You have been blocked by the GM Team
    5 = Your Game's EXE file is not the latest version
    6 = You are prohibited to log in until %s
    7 = Server is jammed due to over populated
    8 = No more accounts may be connected from this company
    9 = MSI_REFUSE_BAN_BY_DBA
    10 = MSI_REFUSE_EMAIL_NOT_CONFIRMED
    11 = MSI_REFUSE_BAN_BY_GM
    12 = MSI_REFUSE_TEMP_BAN_FOR_DBWORK
    13 = MSI_REFUSE_SELF_LOCK
    14 = MSI_REFUSE_NOT_PERMITTED_GROUP
    15 = MSI_REFUSE_NOT_PERMITTED_GROUP
    99 = This ID has been totally erased
    100 = Login information remains at %s
    101 = Account has been locked for a hacking investigation. Please contact the GM Team for more information
    102 = This account has been temporarily prohibited from login due to a bug-related investigation
    103 = This character is being deleted. Login is temporarily unavailable for the time being
    104 = This character is being deleted. Login is temporarily unavailable for the time being
     default = Unknown Error.
 */
static void logclif_auth_failed(struct login_session_data* sd, int result) {
	int fd = sd->fd;
	uint32 ip = session[fd]->client_addr;

	if (login_config.log_login)
	{
		if(result >= 0 && result <= 15)
		    login_log(ip, sd->userid, result, msg_txt(result));
		else if(result >= 99 && result <= 104)
		    login_log(ip, sd->userid, result, msg_txt(result-83)); //-83 offset
		else
		    login_log(ip, sd->userid, result, msg_txt(22)); //unknow error
	}

	if( (result == 0 || result == 1) && login_config.dynamic_pass_failure_ban )
		ipban_log(ip); // log failed password attempt

	// 6 = You are prohibited to log in until %s
	if( result == 6 ){
		char unblock_time[20];
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();
		time_t unban_time = ( accounts->load_str( accounts, &acc, sd->userid ) ) ? acc.unban_time : 0;
		timestamp2string( unblock_time, sizeof( unblock_time ), unban_time, login_config.date_format );

		logclif_auth_failed( fd, result, unblock_time );
	}else{
		logclif_auth_failed( fd, result );
	}
}

/**
 * Received a keepalive packet to maintain connection.
 * 0x200 <account.userid>.24B.
 * @param fd: fd to parse from (client fd)
 * @return 0 not enough info transmitted, 1 success
 */
static bool logclif_parse_keepalive( int fd, struct login_session_data& ){
	// Do nothing
	return true;
}

/**
 * Received a keepalive packet to maintain connection.
 * S 0204 <md5 hash>.16B (kRO 2004-05-31aSakexe langtype 0 and 6)
 * @param fd: fd to parse from (client fd)
 * @return 0 not enough info transmitted, 1 success
 */
static bool logclif_parse_updclhash( int fd, struct login_session_data& sd ){
	PACKET_CA_EXE_HASHCHECK* p = (PACKET_CA_EXE_HASHCHECK*)RFIFOP( fd, 0 );

	sd.has_client_hash = 1;
	memcpy( sd.client_hash, p->hash, sizeof( sd.client_hash ) );

	return true;
}

template <typename P>
static bool logclif_parse_reqauth_raw( int fd, login_session_data& sd ){
	P* p = (P*)RFIFOP( fd, 0 );

	char ip[16];
	uint32 ipl = session[fd]->client_addr;
	ip2str( ipl, ip );

	safestrncpy( sd.userid, p->username, sizeof( sd.userid ) );
	sd.clienttype = p->clienttype;

	ShowStatus( "Request for connection of %s (ip: %s)\n", sd.userid, ip );
	safestrncpy( sd.passwd, p->password, PASSWD_LENGTH );

	if( login_config.use_md5_passwds ){
		MD5_String( sd.passwd, sd.passwd );
	}

	sd.passwdenc = 0;

	int result = login_mmo_auth( &sd, false );

	if( result == -1 ){
		logclif_auth_ok( &sd );
	}else{
		logclif_auth_failed( &sd, result );
	}

	return true;
}

template <typename P>
static bool logclif_parse_reqauth_md5( int fd, login_session_data& sd ){
	P* p = (P*)RFIFOP( fd, 0 );

	char ip[16];
	uint32 ipl = session[fd]->client_addr;
	ip2str( ipl, ip );

	safestrncpy( sd.userid, p->username, sizeof( sd.userid ) );
	sd.clienttype = p->clienttype;

	ShowStatus( "Request for connection (passwdenc mode) of %s (ip: %s)\n", sd.userid, ip );
	bin2hex( sd.passwd, p->passwordMD5, sizeof( p->passwordMD5 ) ); // raw binary data here!

	sd.passwdenc = PASSWORDENC;

	if( login_config.use_md5_passwds ){
		logclif_auth_failed( &sd, 3 ); // send "rejected from server"
		return false;
	}

	int result = login_mmo_auth( &sd, false );

	if( result == -1 ){
		logclif_auth_ok( &sd );
	}else{
		logclif_auth_failed( &sd, result );
	}

	return true;
}

template <typename P>
static bool logclif_parse_reqauth_sso( int fd, login_session_data& sd ){
	P* p = (P*)RFIFOP( fd, 0 );

	char ip[16];
	uint32 ipl = session[fd]->client_addr;
	ip2str( ipl, ip );

	size_t token_length = p->packetLength - sizeof( *p );

	safestrncpy( sd.userid, p->username, sizeof( sd.userid ) );
	sd.clienttype = p->clienttype;

	ShowStatus( "Request for connection (SSO mode) of %s (ip: %s)\n", sd.userid, ip );
	// Shinryo: For the time being, just use token as password.
	safestrncpy( sd.passwd, p->token, token_length + 1 );

	if( login_config.use_md5_passwds ){
		MD5_String( sd.passwd, sd.passwd );
	}

	sd.passwdenc = 0;

	int result = login_mmo_auth( &sd, false );

	if( result == -1 ){
		logclif_auth_ok( &sd );
	}else{
		logclif_auth_failed( &sd, result );
	}

	return true;
}

static void logclif_reqkey_result( int fd, struct login_session_data& sd ){
	PACKET_AC_ACK_HASH* p = (PACKET_AC_ACK_HASH*)packet_buffer;

	p->packetType = HEADER_AC_ACK_HASH;
	p->packetLength = sizeof( *p ) + sd.md5keylen;
	strncpy( p->salt, sd.md5key, sd.md5keylen );

	socket_send( fd, p );
}

/**
 * Client requests an md5key for his session: keys will be generated and sent back.
 * @param fd: file descriptor to parse from (client)
 * @param sd: client session
 * @return 1 success
 */
static bool logclif_parse_reqkey( int fd, struct login_session_data& sd ){
	PACKET_CA_REQ_HASH* p_in = (PACKET_CA_REQ_HASH*)RFIFOP( fd, 0 );

	sd.md5keylen = sizeof( sd.md5key );
	MD5_Salt( sd.md5keylen, sd.md5key );

	logclif_reqkey_result( fd, sd );

	return 1;
}

/**
 * Char-server request to connect to the login-server.
 * This is needed to exchange packets.
 * @param fd: file descriptor to parse from (client)
 * @param sd: client session
 * @param ip: ipv4 address (client)
 * @return 0 packet received too shirt, 1 success
 */
static int logclif_parse_reqcharconnec(int fd, struct login_session_data *sd, char* ip){
	if (RFIFOREST(fd) < 86)
		return 0;
	else {
		int result;
		char server_name[20];
		char message[256];
		uint32 server_ip;
		uint16 server_port;
		uint16 type;
		uint16 new_;

		safestrncpy(sd->userid, RFIFOCP(fd,2), NAME_LENGTH);
		safestrncpy(sd->passwd, RFIFOCP(fd,26), NAME_LENGTH);
		if( login_config.use_md5_passwds )
			MD5_String(sd->passwd, sd->passwd);
		sd->passwdenc = 0;
		server_ip = ntohl(RFIFOL(fd,54));
		server_port = ntohs(RFIFOW(fd,58));
		safestrncpy(server_name, RFIFOCP(fd,60), 20);
		type = RFIFOW(fd,82);
		new_ = RFIFOW(fd,84);
		RFIFOSKIP(fd,86);

		ShowInfo("Connection request of the char-server '%s' @ %u.%u.%u.%u:%u (account: '%s', ip: '%s')\n", server_name, CONVIP(server_ip), server_port, sd->userid, ip);
		sprintf(message, "charserver - %s@%u.%u.%u.%u:%u", server_name, CONVIP(server_ip), server_port);
		login_log(session[fd]->client_addr, sd->userid, 100, message);

		result = login_mmo_auth(sd, true);
		if( global_core->is_running() &&
			result == -1 &&
			sd->sex == 'S' &&
			sd->account_id < ARRAYLENGTH(ch_server) &&
			!session_isValid(ch_server[sd->account_id].fd) )
		{
			ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
			safestrncpy(ch_server[sd->account_id].name, server_name, sizeof(ch_server[sd->account_id].name));
			ch_server[sd->account_id].fd = fd;
			ch_server[sd->account_id].ip = server_ip;
			ch_server[sd->account_id].port = server_port;
			ch_server[sd->account_id].users = 0;
			ch_server[sd->account_id].type = type;
			ch_server[sd->account_id].new_ = new_;

			session[fd]->func_parse = logchrif_parse;
			session[fd]->flag.server = 1;
			realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

			// send connection success
			WFIFOHEAD(fd,3);
			WFIFOW(fd,0) = 0x2711;
			WFIFOB(fd,2) = 0;
			WFIFOSET(fd,3);
		}
		else
		{
			ShowNotice("Connection of the char-server '%s' REFUSED.\n", server_name);
			WFIFOHEAD(fd,3);
			WFIFOW(fd,0) = 0x2711;
			WFIFOB(fd,2) = 3;
			WFIFOSET(fd,3);
		}
	}
	return 1;
}

static void logclif_otp_result( int fd ){
	PACKET_TC_RESULT p = {};

	p.packetType = HEADER_TC_RESULT;
	p.packetLength = sizeof( p );
	p.type = 0; // normal login
	safestrncpy( p.unknown1, "S1000", sizeof( p.unknown1 ) );
	safestrncpy( p.unknown2, "token", sizeof( p.unknown2 ) );

	socket_send( fd, p );
}

static bool logclif_parse_otp_login( int fd, struct login_session_data& ){
	PACKET_CT_AUTH* p = (PACKET_CT_AUTH*)RFIFOP( fd, 0 );

	logclif_otp_result( fd );

	return 1;
}

class LoginPacketDatabase : public PacketDatabase<login_session_data>{
public:
	LoginPacketDatabase(){
		this->add( HEADER_CA_CONNECT_INFO_CHANGED, true, sizeof( PACKET_CA_CONNECT_INFO_CHANGED ), logclif_parse_keepalive );
		this->add( HEADER_CA_EXE_HASHCHECK, true, sizeof( PACKET_CA_EXE_HASHCHECK ), logclif_parse_updclhash );
		this->add( HEADER_CA_LOGIN, true, sizeof( PACKET_CA_LOGIN ), logclif_parse_reqauth_raw<PACKET_CA_LOGIN> );
		this->add( HEADER_CA_LOGIN_PCBANG, true, sizeof( PACKET_CA_LOGIN_PCBANG ), logclif_parse_reqauth_raw<PACKET_CA_LOGIN_PCBANG> );
		this->add( HEADER_CA_LOGIN_CHANNEL, true, sizeof( PACKET_CA_LOGIN_CHANNEL ), logclif_parse_reqauth_raw<PACKET_CA_LOGIN_CHANNEL> );
		this->add( HEADER_CA_LOGIN2, true, sizeof( PACKET_CA_LOGIN2 ), logclif_parse_reqauth_md5<PACKET_CA_LOGIN2> );
		this->add( HEADER_CA_LOGIN3, true, sizeof( PACKET_CA_LOGIN3 ), logclif_parse_reqauth_md5<PACKET_CA_LOGIN3> );
		this->add( HEADER_CA_LOGIN4, true, sizeof( PACKET_CA_LOGIN4 ), logclif_parse_reqauth_md5<PACKET_CA_LOGIN4> );
		this->add( HEADER_CA_SSO_LOGIN_REQ, false, sizeof( PACKET_CA_SSO_LOGIN_REQ ), logclif_parse_reqauth_sso<PACKET_CA_SSO_LOGIN_REQ> );
		this->add( HEADER_CA_REQ_HASH, true, sizeof( PACKET_CA_REQ_HASH ), logclif_parse_reqkey );
		this->add( HEADER_CT_AUTH, true, sizeof( PACKET_CT_AUTH ), logclif_parse_otp_login );
	}
} login_packet_db;

/**
 * Entry point from client to log-server.
 * Function that checks incoming command, then splits it to the correct handler.
 * @param fd: file descriptor to parse, (link to client)
 * @return 0=invalid session,marked for disconnection,unknow packet, banned..; 1=success
 */
int logclif_parse(int fd) {
	struct login_session_data* sd = (struct login_session_data*)session[fd]->session_data;

	char ip[16];
	uint32 ipl = session[fd]->client_addr;
	ip2str(ipl, ip);

	if( session[fd]->flag.eof )
	{
		ShowInfo("Closed connection from '" CL_WHITE "%s" CL_RESET "'.\n", ip);
		do_close(fd);
		return 0;
	}

	if( sd == nullptr ){
		// Perform ip-ban check
		if( login_config.ipban && ipban_check(ipl) )
		{
			ShowStatus("Connection refused: IP isn't authorised (deny/allow, ip: %s).\n", ip);
			login_log(ipl, "unknown", -3, "ip banned");

			logclif_auth_failed( fd, 3 ); // 3 = Rejected from Server

			set_eof(fd);
			return 0;
		}
		// create a session for this new connection
		CREATE(session[fd]->session_data, struct login_session_data, 1);
		sd = (struct login_session_data*)session[fd]->session_data;
		sd->fd = fd;
	}

	while( RFIFOREST(fd) >= 2 )
	{
		uint16 command = RFIFOW(fd,0);

		switch( command ){
			// Connection request of a char-server
			case 0x2710: logclif_parse_reqcharconnec(fd,sd, ip); return 0; // processing will continue elsewhere
			default:
				if( !login_packet_db.handle( fd, *sd ) ){
					return 0;
				}
				break;
		}
	}

	return 0;
}

/// Constructor destructor

/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 */
void do_init_loginclif(void){
	return;
}

/**
 * loginclif destructor
 *  dealloc..., function called at exit of the login-serv
 */
void do_final_loginclif(void){
	return;
}
