/**
 * @file loginclif.c
 * Module purpose is to handle incoming and outgoing requests with client.
 * Licensed under GNU GPL.
 *  For more information, see LICENCE in the main folder.
 * @author Athena Dev Teams originally in login.c
 * @author rAthena Dev Team
 */

#include "../common/timer.h" //difftick
#include "../common/strlib.h" //safeprint
#include "../common/showmsg.h" //show notice
#include "../common/socket.h" //wfifo session
#include "../common/malloc.h"
#include "../common/utils.h"
#include "../common/md5calc.h"
#include "../common/random.h"
#include "account.h"
#include "ipban.h" //ipban_check
#include "login.h"
#include "loginlog.h"
#include "loginclif.h"
#include "loginchrif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x81;
	WFIFOB(fd,2) = result;
	WFIFOSET(fd,3);
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
	struct auth_node* node;
	int i;

	if( runflag != LOGINSERVER_ST_RUNNING ){
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
		struct online_login_data* data = (struct online_login_data*)idb_get(online_db, sd->account_id);
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
				idb_remove(auth_db, sd->account_id);
				login_remove_online_user(sd->account_id);
				data = NULL;
			}
		}
	}

	login_log(ip, sd->userid, 100, "login ok");
	ShowStatus("Connection of the account '%s' accepted.\n", sd->userid);

	WFIFOHEAD(fd,47+32*server_num);
	WFIFOW(fd,0) = 0x69;
	WFIFOW(fd,2) = 47+32*server_num;
	WFIFOL(fd,4) = sd->login_id1;
	WFIFOL(fd,8) = sd->account_id;
	WFIFOL(fd,12) = sd->login_id2;
	WFIFOL(fd,16) = 0; // in old version, that was for ip (not more used)
	//memcpy(WFIFOP(fd,20), sd->lastlogin, 24); // in old version, that was for name (not more used)
	memset(WFIFOP(fd,20), 0, 24);
	WFIFOW(fd,44) = 0; // unknown
	WFIFOB(fd,46) = sex_str2num(sd->sex);
	for( i = 0, n = 0; i < ARRAYLENGTH(ch_server); ++i ) {
		if( !session_isValid(ch_server[i].fd) )
			continue;
		subnet_char_ip = lan_subnetcheck(ip); // Advanced subnet check [LuzZza]
		WFIFOL(fd,47+n*32) = htonl((subnet_char_ip) ? subnet_char_ip : ch_server[i].ip);
		WFIFOW(fd,47+n*32+4) = ntows(htons(ch_server[i].port)); // [!] LE byte order here [!]
		memcpy(WFIFOP(fd,47+n*32+6), ch_server[i].name, 20);
		WFIFOW(fd,47+n*32+26) = ch_server[i].users;
		WFIFOW(fd,47+n*32+28) = ch_server[i].type;
		WFIFOW(fd,47+n*32+30) = ch_server[i].new_;
		n++;
	}
	WFIFOSET(fd,47+32*server_num);

	// create temporary auth entry
	CREATE(node, struct auth_node, 1);
	node->account_id = sd->account_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->sex;
	node->ip = ip;
	node->version = sd->version;
	node->clienttype = sd->clienttype;
	idb_put(auth_db, sd->account_id, node);
	{
		struct online_login_data* data;
		// mark client as 'online'
		data = login_add_online_user(-1, sd->account_id);
		// schedule deletion of this node
		data->waiting_disconnect = add_timer(gettick()+AUTH_TIMEOUT, login_waiting_disconnect_timer, sd->account_id, 0);
	}
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
    6 = Your are Prohibited to log in until %s
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

	if( result == 1 && login_config.dynamic_pass_failure_ban )
		ipban_log(ip); // log failed password attempt

//#if PACKETVER >= 20120000 /* not sure when this started */
	if( sd->version >= date2version(20120000) ){ /* not sure when this started */
		WFIFOHEAD(fd,26);
		WFIFOW(fd,0) = 0x83e;
		WFIFOL(fd,2) = result;
		if( result != 6 )
			memset(WFIFOP(fd,6), '\0', 20);
		else { // 6 = Your are Prohibited to log in until %s
			struct mmo_account acc;
			AccountDB* accounts = login_get_accounts_db();
			time_t unban_time = ( accounts->load_str(accounts, &acc, sd->userid) ) ? acc.unban_time : 0;
			timestamp2string((char*)WFIFOP(fd,6), 20, unban_time, login_config.date_format);
		}
		WFIFOSET(fd,26);
	}
//#else	
	else {
		WFIFOHEAD(fd,23);
		WFIFOW(fd,0) = 0x6a;
		WFIFOB(fd,2) = (uint8)result;
		if( result != 6 )
			memset(WFIFOP(fd,3), '\0', 20);
		else { // 6 = Your are Prohibited to log in until %s
			struct mmo_account acc;
			AccountDB* accounts = login_get_accounts_db();
			time_t unban_time = ( accounts->load_str(accounts, &acc, sd->userid) ) ? acc.unban_time : 0;
			timestamp2string((char*)WFIFOP(fd,3), 20, unban_time, login_config.date_format);
		}
		WFIFOSET(fd,23);
	}
//#endif	
}

/**
 * Received a keepalive packet to maintain connection.
 * 0x200 <account.userid>.24B.
 * @param fd: fd to parse from (client fd)
 * @return 0 not enough info transmitted, 1 success
 */
static int logclif_parse_keepalive(int fd){
	if (RFIFOREST(fd) < 26)
		return 0;
	RFIFOSKIP(fd,26);
	return 1;
}

/**
 * Received a keepalive packet to maintain connection.
 * S 0204 <md5 hash>.16B (kRO 2004-05-31aSakexe langtype 0 and 6)
 * @param fd: fd to parse from (client fd)
 * @return 0 not enough info transmitted, 1 success
 */
static int logclif_parse_updclhash(int fd, struct login_session_data *sd){
	if (RFIFOREST(fd) < 18)
		return 0;
	sd->has_client_hash = 1;
	memcpy(sd->client_hash, RFIFOP(fd, 2), 16);
	RFIFOSKIP(fd,18);
	return 1;
}

/**
 * Received a connection request.
 * @param fd: file descriptor to parse from (client)
 * @param sd: client session
 * @param command: packet type sent
 * @param ip: ipv4 address (client)
 *  S 0064 <version>.L <username>.24B <password>.24B <clienttype>.B
 *  S 0277 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B
 *  S 02b0 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B <g_isGravityID>.B
 *  S 01dd <version>.L <username>.24B <password hash>.16B <clienttype>.B
 *  S 01fa <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.B(index of the connection in the clientinfo file (+10 if the command-line contains "pc"))
 *  S 027c <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.13B(junk)
 *  S 0825 <packetsize>.W <version>.L <clienttype>.B <userid>.24B <password>.27B <mac>.17B <ip>.15B <token>.(packetsize - 0x5C)B
 * @param fd: fd to parse from (client fd)
 * @return 0 failure, 1 success
 */
static int logclif_parse_reqauth(int fd, struct login_session_data *sd, int command, char* ip){
	size_t packet_len = RFIFOREST(fd);

	if( (command == 0x0064 && packet_len < 55)
	||  (command == 0x0277 && packet_len < 84)
	||  (command == 0x02b0 && packet_len < 85)
	||  (command == 0x01dd && packet_len < 47)
	||  (command == 0x01fa && packet_len < 48)
	||  (command == 0x027c && packet_len < 60)
	||  (command == 0x0825 && (packet_len < 4 || packet_len < RFIFOW(fd, 2))) )
		return 0;
	else {
		int result;
		uint32 version;
		char username[NAME_LENGTH];
		char password[PASSWD_LENGTH];
		unsigned char passhash[16];
		uint8 clienttype;
		bool israwpass = (command==0x0064 || command==0x0277 || command==0x02b0 || command == 0x0825);

		// Shinryo: For the time being, just use token as password.
		if(command == 0x0825) {
			char *accname = (char *)RFIFOP(fd, 9);
			char *token = (char *)RFIFOP(fd, 0x5C);
			size_t uAccLen = strlen(accname);
			size_t uTokenLen = RFIFOREST(fd) - 0x5C;

			version = RFIFOL(fd,4);

			if(uAccLen > NAME_LENGTH - 1 || uAccLen == 0 || uTokenLen > NAME_LENGTH - 1  || uTokenLen == 0)
			{
				logclif_auth_failed(sd, 3);
				return 0;
			}

			safestrncpy(username, accname, uAccLen + 1);
			safestrncpy(password, token, uTokenLen + 1);
			clienttype = RFIFOB(fd, 8);
		}
		else
		{
			version = RFIFOL(fd,2);
			safestrncpy(username, (const char*)RFIFOP(fd,6), NAME_LENGTH);
			if( israwpass )
			{
				safestrncpy(password, (const char*)RFIFOP(fd,30), PASSWD_LENGTH);
				clienttype = RFIFOB(fd,54);
			}
			else
			{
				memcpy(passhash, RFIFOP(fd,30), 16);
				clienttype = RFIFOB(fd,46);
			}
		}
		RFIFOSKIP(fd,RFIFOREST(fd)); // assume no other packet was sent

		sd->clienttype = clienttype;
		sd->version = version;
		safestrncpy(sd->userid, username, NAME_LENGTH);
		if( israwpass )
		{
			ShowStatus("Request for connection of %s (ip: %s) version=%d\n", sd->userid, ip,sd->version);
			safestrncpy(sd->passwd, password, NAME_LENGTH);
			if( login_config.use_md5_passwds )
				MD5_String(sd->passwd, sd->passwd);
			sd->passwdenc = 0;
		}
		else
		{
			ShowStatus("Request for connection (passwdenc mode) of %s (ip: %s) version=%d\n", sd->userid, ip,sd->version);
			bin2hex(sd->passwd, passhash, 16); // raw binary data here!
			sd->passwdenc = PASSWORDENC;
		}

		if( sd->passwdenc != 0 && login_config.use_md5_passwds )
		{
			logclif_auth_failed(sd, 3); // send "rejected from server"
			return 0;
		}

		result = login_mmo_auth(sd, false);

		if( result == -1 )
			logclif_auth_ok(sd);
		else
			logclif_auth_failed(sd, result);
	}
	return 1;
}

/**
 * Client requests an md5key for his session: keys will be generated and sent back.
 * @param fd: file descriptor to parse from (client)
 * @param sd: client session
 * @return 1 success
 */
static int logclif_parse_reqkey(int fd, struct login_session_data *sd){
	RFIFOSKIP(fd,2);
	{
		memset(sd->md5key, '\0', sizeof(sd->md5key));
		sd->md5keylen = (uint16)(12 + rnd() % 4);
		MD5_Salt(sd->md5keylen, sd->md5key);

		WFIFOHEAD(fd,4 + sd->md5keylen);
		WFIFOW(fd,0) = 0x01dc;
		WFIFOW(fd,2) = 4 + sd->md5keylen;
		memcpy(WFIFOP(fd,4), sd->md5key, sd->md5keylen);
		WFIFOSET(fd,WFIFOW(fd,2));
	}
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
	{
		int result;
		char server_name[20];
		char message[256];
		uint32 server_ip;
		uint16 server_port;
		uint16 type;
		uint16 new_;

		safestrncpy(sd->userid, (char*)RFIFOP(fd,2), NAME_LENGTH);
		safestrncpy(sd->passwd, (char*)RFIFOP(fd,26), NAME_LENGTH);
		if( login_config.use_md5_passwds )
			MD5_String(sd->passwd, sd->passwd);
		sd->passwdenc = 0;
		sd->version = login_config.client_version_to_connect; // hack to skip version check
		server_ip = ntohl(RFIFOL(fd,54));
		server_port = ntohs(RFIFOW(fd,58));
		safestrncpy(server_name, (char*)RFIFOP(fd,60), 20);
		type = RFIFOW(fd,82);
		new_ = RFIFOW(fd,84);
		RFIFOSKIP(fd,86);

		ShowInfo("Connection request of the char-server '%s' @ %u.%u.%u.%u:%u (account: '%s', pass: '%s', ip: '%s')\n", server_name, CONVIP(server_ip), server_port, sd->userid, sd->passwd, ip);
		sprintf(message, "charserver - %s@%u.%u.%u.%u:%u", server_name, CONVIP(server_ip), server_port);
		login_log(session[fd]->client_addr, sd->userid, 100, message);

		result = login_mmo_auth(sd, true);
		if( runflag == LOGINSERVER_ST_RUNNING &&
			result == -1 &&
			sd->sex == 'S' &&
			sd->account_id >= 0 && sd->account_id < ARRAYLENGTH(ch_server) &&
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
		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", ip);
		do_close(fd);
		return 0;
	}

	if( sd == NULL )
	{
		// Perform ip-ban check
		if( login_config.ipban && ipban_check(ipl) )
		{
			ShowStatus("Connection refused: IP isn't authorised (deny/allow, ip: %s).\n", ip);
			login_log(ipl, "unknown", -3, "ip banned");
			WFIFOHEAD(fd,23);
			WFIFOW(fd,0) = 0x6a;
			WFIFOB(fd,2) = 3; // 3 = Rejected from Server
			WFIFOSET(fd,23);
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
		int next=1;

		switch( command )
		{
		// New alive packet: used to verify if client is always alive.
		case 0x0200: next = logclif_parse_keepalive(fd); break;
		// client md5 hash (binary)
		case 0x0204: next = logclif_parse_updclhash(fd,sd); break;
		// request client login (raw password)
		case 0x0064: // S 0064 <version>.L <username>.24B <password>.24B <clienttype>.B
		case 0x0277: // S 0277 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B
		case 0x02b0: // S 02b0 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B <g_isGravityID>.B
		// request client login (md5-hashed password)
		case 0x01dd: // S 01dd <version>.L <username>.24B <password hash>.16B <clienttype>.B
		case 0x01fa: // S 01fa <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.B(index of the connection in the clientinfo file (+10 if the command-line contains "pc"))
		case 0x027c: // S 027c <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.13B(junk)
		case 0x0825: // S 0825 <packetsize>.W <version>.L <clienttype>.B <userid>.24B <password>.27B <mac>.17B <ip>.15B <token>.(packetsize - 0x5C)B
			next = logclif_parse_reqauth(fd,  sd, command, ip); 
			break;
		// Sending request of the coding key
		case 0x01db: next = logclif_parse_reqkey(fd, sd); break;
		// Connection request of a char-server
		case 0x2710: logclif_parse_reqcharconnec(fd,sd, ip); return 0; // processing will continue elsewhere
		default:
			ShowNotice("Abnormal end of connection (ip: %s): Unknown packet 0x%x\n", ip, command);
			set_eof(fd);
			return 0;
		}
		if(next==0) return 0; // avoid processing of followup packets (prev was probably incomplete)
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
