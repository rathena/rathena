// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "char_logif.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>

#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "char.hpp"
#include "char_clif.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"
#include "int_guild.hpp"

using namespace rathena;

//early declaration
void chlogif_on_ready(void);
void chlogif_on_disconnect(void);

#if PACKETVER_SUPPORTS_PINCODE
void chlogif_pincode_notifyLoginPinError( uint32 account_id ){
	if ( chlogif_isconnected() ){
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2739;
		WFIFOL(login_fd,2) = account_id;
		WFIFOSET(login_fd,6);
	}
}

void chlogif_pincode_notifyLoginPinUpdate( uint32 account_id, char* pin ){
	if ( chlogif_isconnected() ){
		int size = 8 + PINCODE_LENGTH+1;
		WFIFOHEAD(login_fd,size);
		WFIFOW(login_fd,0) = 0x2738;
		WFIFOW(login_fd,2) = size;
		WFIFOL(login_fd,4) = account_id;
		strncpy( WFIFOCP(login_fd,8), pin, PINCODE_LENGTH+1 );
		WFIFOSET(login_fd,size);
	}
}

void chlogif_pincode_start(int fd, struct char_session_data* sd){
	if( charserv_config.pincode_config.pincode_enabled ){
		//ShowInfo("Asking to start pincode to AID: %d\n", sd->account_id);
		// PIN code system enabled
		if( sd->pincode[0] == '\0' ){
			// No PIN code has been set yet
			if( charserv_config.pincode_config.pincode_force ){
				chclif_pincode_sendstate( fd, sd, PINCODE_NEW );
			}else{
				chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );
			}
		}else{
			if( !(charserv_config.pincode_config.pincode_changetime)
			|| ( sd->pincode_change + charserv_config.pincode_config.pincode_changetime ) > time(nullptr) ){
				std::shared_ptr<struct online_char_data> node = util::umap_find( char_get_onlinedb(), sd->account_id );

				if( node != nullptr && node->pincode_success ){
					// User has already passed the check
					chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );
				}else{
					// Ask user for his PIN code
					chclif_pincode_sendstate( fd, sd, PINCODE_ASK );
				}
			}else{
				// User hasnt changed his PIN code too long
				chclif_pincode_sendstate( fd, sd, PINCODE_EXPIRED );
			}
		}
	}else{
		// PIN code system disabled
		//ShowInfo("Pincode is disabled.\n");
		chclif_pincode_sendstate( fd, sd, PINCODE_OK );
	}
}
#endif

/**
 * Timered function to send all account_id connected to login-serv
 * @param tid : Timer id
 * @param tick : Scheduled tick
 * @param id : GID linked to that timered call
 * @param data : data transmited for delayed function
 * @return
 */
TIMER_FUNC(chlogif_send_acc_tologin){
	if ( chlogif_isconnected() ){
		// send account list to login server
		size_t users = char_get_onlinedb().size();
		int i = 0;

		WFIFOHEAD(login_fd,8+users*4);
		WFIFOW(login_fd,0) = 0x272d;
		for( const auto& pair : char_get_onlinedb() ){
			std::shared_ptr<struct online_char_data> character = pair.second;

			if( character->server > -1 ){
				WFIFOL( login_fd, 8 + i * 4 ) = character->account_id;
				i++;
			}
		}
		WFIFOW(login_fd,2) = 8+ i*4;
		WFIFOL(login_fd,4) = i;
		WFIFOSET(login_fd,WFIFOW(login_fd,2));
		return 1;
	}
	return 0;
}

void chlogif_send_usercount(int users){
	if (!chlogif_isconnected())
		return;
	// send number of user to login server
	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x2714;
	WFIFOL(login_fd,2) = users;
	WFIFOSET(login_fd,6);
}


TIMER_FUNC(chlogif_broadcast_user_count){
	uint8 buf[6];
	int users = char_count_users();

	// only send an update when needed
	static int prev_users = 0;
	if( prev_users == users )
		return 0;
	prev_users = users;

	if( chlogif_isconnected() )
	{
		// send number of user to login server
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2714;
		WFIFOL(login_fd,2) = users;
		WFIFOSET(login_fd,6);
	}

	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	chmapif_sendall(buf,6);

	return 0;
}

void chlogif_upd_global_accreg(uint32 account_id, uint32 char_id) {
	if ( chlogif_isconnected() ){
		WFIFOHEAD(login_fd, 60000 + 300);
		WFIFOW(login_fd, 0) = 0x2728;
		WFIFOW(login_fd, 2) = 14;
		WFIFOL(login_fd, 4) = account_id;
		WFIFOL(login_fd, 8) = char_id;
		WFIFOW(login_fd, 12) = 0; // count
	}
}

void chlogif_prepsend_global_accreg(void) {
	if ( chlogif_isconnected() ){
		WFIFOSET(login_fd, WFIFOW(login_fd,2));
	}
}

void chlogif_send_global_accreg(const char *key, unsigned int index, int64 int_value, const char* string_value, bool is_string) {
	if (!chlogif_isconnected())
		return;

	int16 nlen = WFIFOW( login_fd, 2 );
	size_t len;

	len = strlen(key)+1;

	WFIFOB(login_fd, nlen) = (unsigned char)len; // won't be higher; the column size is 32
	nlen += 1;

	safestrncpy(WFIFOCP(login_fd,nlen), key, len);
	nlen += static_cast<decltype(nlen)>( len );

	WFIFOL(login_fd, nlen) = index;
	nlen += 4;

	if( is_string ) {
		WFIFOB(login_fd, nlen) = string_value ? 2 : 3;
		nlen += 1;

		if( string_value ) {
			len = strlen(string_value)+1;

			WFIFOB(login_fd, nlen) = (unsigned char)len; // won't be higher; the column size is 254
			nlen += 1;

			safestrncpy(WFIFOCP(login_fd,nlen), string_value, len);
			nlen += static_cast<decltype(nlen)>( len );
		}
	} else {
		WFIFOB(login_fd, nlen) = int_value ? 0 : 1;
		nlen += 1;

		if( int_value ) {
			WFIFOQ(login_fd, nlen) = int_value;
			nlen += 8;
		}
	}

	WFIFOW(login_fd,12) += 1;
	WFIFOW(login_fd, 2) = nlen;

	if( WFIFOW(login_fd, 2) > 60000 ) {
		int account_id = WFIFOL(login_fd,4), char_id = WFIFOL(login_fd,8);

		chlogif_prepsend_global_accreg();
		chlogif_upd_global_accreg(account_id, char_id); // prepare next
	}
}

void chlogif_request_accreg2(uint32 account_id, uint32 char_id){
	if (!chlogif_isconnected())
		return;
	WFIFOHEAD(login_fd,10);
	WFIFOW(login_fd,0) = 0x272e;
	WFIFOL(login_fd,2) = account_id;
	WFIFOL(login_fd,6) = char_id;
	WFIFOSET(login_fd,10);
}

void chlogif_send_reqaccdata(int fd, struct char_session_data *sd){
	if (!chlogif_isconnected())
		return;
	WFIFOHEAD(fd,6);
	WFIFOW(fd,0) = 0x2716;
	WFIFOL(fd,2) = sd->account_id;
	WFIFOSET(fd,6);
}

void chlogif_send_setacconline(int aid){
	if (!chlogif_isconnected())
		return;
	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x272b;
	WFIFOL(login_fd,2) = aid;
	WFIFOSET(login_fd,6);
}

void chlogif_send_setallaccoffline(int fd){
	if (!chlogif_isconnected())
		return;
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2737;
	WFIFOSET(fd,2);
}

void chlogif_send_setaccoffline(int fd, int aid){
	if (!chlogif_isconnected())
		return;
	WFIFOHEAD(fd,6);
	WFIFOW(fd,0) = 0x272c;
	WFIFOL(fd,2) = aid;
	WFIFOSET(fd,6);
}

int chlogif_parse_ackconnect(int fd){
	if (RFIFOREST(fd) < 3)
		return 0;

	if (RFIFOB(fd,2)) {
		//printf("connect login server error : %d\n", RFIFOB(fd,2));
		ShowError("Can not connect to login-server.\n");
		ShowError("The server communication passwords (default s1/p1) are probably invalid.\n");
		ShowError("Also, please make sure your login db has the correct communication username/passwords and the gender of the account is S.\n");
		ShowError("The communication passwords are set in map_athena.conf and char_athena.conf\n");
		set_eof(fd);
		return 0;
	} else {
		ShowStatus("Connected to login-server (connection #%d).\n", fd);
		chlogif_on_ready();
	}
	RFIFOSKIP(fd,3);
	return 1;
}

int chlogif_parse_ackaccreq(int fd){
	if (RFIFOREST(fd) < 21)
		return 0;
	{
		struct char_session_data* sd;
		uint32 account_id = RFIFOL(fd,2);
		uint32 login_id1 = RFIFOL(fd,6);
		uint32 login_id2 = RFIFOL(fd,10);
		uint8 sex = RFIFOB(fd,14);
		uint8 result = RFIFOB(fd,15);
		int request_id = RFIFOL(fd,16);
		uint8 clienttype = RFIFOB(fd,20);
		RFIFOSKIP(fd,21);

		if( session_isActive(request_id) && (sd=(struct char_session_data*)session[request_id]->session_data) &&
			!sd->auth && sd->account_id == account_id && sd->login_id1 == login_id1 && sd->login_id2 == login_id2 && sd->sex == sex )
		{
			int client_fd = request_id;
			sd->clienttype = clienttype;

			switch( result )
			{
			case 0:// ok
				char_auth_ok(client_fd, sd);
				break;
			case 1:// auth failed
				chclif_reject(client_fd,0); // rejected from server
				break;
			}
		}
	}
	return 1;
}

/**
 * Receive account data from login-server
 * AH 0x2717 <aid>.L <email>.40B <expiration_time>.L <group_id>.B <birthdate>.11B <pincode>.5B <pincode_change>.L <isvip>.B <char_vip>.B <char_billing>.B
 **/
int chlogif_parse_reqaccdata(int fd){
	if (RFIFOREST(fd) < 75)
		return 0;
	int u_fd; //user fd
	struct char_session_data* sd;

	// find the authenticated session with this account id
	ARR_FIND( 0, fd_max, u_fd, session[u_fd] && (sd = (struct char_session_data*)session[u_fd]->session_data) && sd->auth && sd->account_id == RFIFOL(fd,2) );
	if( u_fd < fd_max )
	{
		memcpy(sd->email, RFIFOP(fd,6), 40);
		sd->expiration_time = (time_t)RFIFOL(fd,46);
		sd->group_id = RFIFOB(fd,50);
		sd->char_slots = RFIFOB(fd,51);
		if( sd->char_slots > MAX_CHARS ) {
			ShowError("Account '%d' `character_slots` column is higher than supported MAX_CHARS (%d), update MAX_CHARS in mmo.hpp! capping to MAX_CHARS...\n",sd->account_id,sd->char_slots);
			sd->char_slots = MAX_CHARS;/* cap to maximum */
		} else if ( !sd->char_slots )/* no value aka 0 in sql */
			sd->char_slots = MIN_CHARS;/* cap to minimum */
		safestrncpy(sd->birthdate, RFIFOCP(fd,52), sizeof(sd->birthdate));
		safestrncpy(sd->pincode, RFIFOCP(fd,63), sizeof(sd->pincode));
		sd->pincode_change = (time_t)RFIFOL(fd,68);
		sd->isvip = RFIFOB(fd,72);
		sd->chars_vip = RFIFOB(fd,73);
		sd->chars_billing = RFIFOB(fd,74);
		// continued from char_auth_ok...
		if(((charserv_config.max_connect_user == 0 || charserv_config.char_maintenance == 1) ||
			(charserv_config.max_connect_user > 0 && char_count_users() >= charserv_config.max_connect_user)) &&
			sd->group_id < charserv_config.gm_allow_group) {
			// refuse connection (over populated)
			chclif_reject(u_fd,0);
		} else {
			// send characters to player
			chclif_mmo_char_send(u_fd, sd);
#if PACKETVER_SUPPORTS_PINCODE
			chlogif_pincode_start(u_fd,sd);
#endif
		}
	}
	RFIFOSKIP(fd,75);
	return 1;
}

int chlogif_parse_keepalive(int fd){
	if (RFIFOREST(fd) < 2)
		return 0;
	RFIFOSKIP(fd,2);
	session[fd]->flag.ping = 0;
	return 1;
}

/**
 * Performs the necessary operations when changing a character's sex, such as
 * correcting the job class and unequipping items, and propagating the
 * information to the guild data.
 *
 * @param sex      The new sex (SEX_MALE or SEX_FEMALE).
 * @param acc      The character's account ID.
 * @param char_id  The character ID.
 * @param class_   The character's current job class.
 * @param guild_id The character's guild ID.
 */
void chlogif_parse_change_sex_sub(int sex, int acc, int char_id, int class_, int guild_id)
{
	// job modification
	switch (class_)
	{
	case JOB_BARD:
		class_ = JOB_DANCER;
		break;
	case JOB_DANCER:
		class_ = JOB_BARD;
		break;
	case JOB_CLOWN:
		class_ = JOB_GYPSY;
		break;
	case JOB_GYPSY:
		class_ = JOB_CLOWN;
		break;
	case JOB_BABY_BARD:
		class_ = JOB_BABY_DANCER;
		break;
	case JOB_BABY_DANCER:
		class_ = JOB_BABY_BARD;
		break;
	case JOB_MINSTREL:
		class_ = JOB_WANDERER;
		break;
	case JOB_WANDERER:
		class_ = JOB_MINSTREL;
		break;
	case JOB_MINSTREL_T:
		class_ = JOB_WANDERER_T;
		break;
	case JOB_WANDERER_T:
		class_ = JOB_MINSTREL_T;
		break;
	case JOB_BABY_MINSTREL:
		class_ = JOB_BABY_WANDERER;
		break;
	case JOB_BABY_WANDERER:
		class_ = JOB_BABY_MINSTREL;
		break;
	case JOB_KAGEROU:
		class_ = JOB_OBORO;
		break;
	case JOB_OBORO:
		class_ = JOB_KAGEROU;
		break;
	case JOB_BABY_KAGEROU:
		class_ = JOB_BABY_OBORO;
		break;
	case JOB_BABY_OBORO:
		class_ = JOB_BABY_KAGEROU;
		break;
	case JOB_TROUBADOUR:
		class_ = JOB_TROUVERE;
		break;
	case JOB_TROUVERE:
		class_ = JOB_TROUBADOUR;
		break;
	case JOB_SHINKIRO:
		class_ = JOB_SHIRANUI;
		break;
	case JOB_SHIRANUI:
		class_ = JOB_SHINKIRO;
		break;
	}

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `equip` = '0' WHERE `char_id` = '%d'", schema_config.inventory_db, char_id))
		Sql_ShowDebug(sql_handle);

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class` = '%d', `weapon` = '0', `shield` = '0', `head_top` = '0', `head_mid` = '0', `head_bottom` = '0', `sex` = '%c' WHERE `char_id` = '%d'", schema_config.char_db, class_, sex == SEX_MALE ? 'M' : 'F', char_id))
		Sql_ShowDebug(sql_handle);
	if (guild_id) // If there is a guild, update the guild_member data [Skotlex]
		inter_guild_sex_changed(guild_id, acc, char_id, sex);
}

int chlogif_parse_ackchangesex(int fd)
{
	if (RFIFOREST(fd) < 7)
		return 0;
	else {
		unsigned char buf[7];
		uint32 acc = RFIFOL(fd,2);
		int sex = RFIFOB(fd,6);
		RFIFOSKIP(fd,7);

		if (acc > 0) { // TODO: Is this even possible?
			unsigned char i;
			int char_id = 0, class_ = 0, guild_id = 0;
			std::shared_ptr<struct auth_node> node = util::umap_find( char_get_authdb(), acc );
			SqlStmt *stmt;

			if (node != nullptr)
				node->sex = sex;

			// get characters
			stmt = SqlStmt_Malloc(sql_handle);
			if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `char_id`, `class`, `guild_id` FROM `%s` WHERE `account_id` = '%d'", schema_config.char_db, acc) || SqlStmt_Execute(stmt)) {
				SqlStmt_ShowDebug(stmt);
				SqlStmt_Free(stmt);
			}

			SqlStmt_BindColumn(stmt, 0, SQLDT_INT,   &char_id,  0, nullptr, nullptr);
			SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT, &class_,   0, nullptr, nullptr);
			SqlStmt_BindColumn(stmt, 2, SQLDT_INT,   &guild_id, 0, nullptr, nullptr);

			for (i = 0; i < MAX_CHARS && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i) {
				chlogif_parse_change_sex_sub(sex, acc, char_id, class_, guild_id);
			}
			SqlStmt_Free(stmt);
		}

		// notify all mapservers about this change
		WBUFW(buf,0) = 0x2b0d;
		WBUFL(buf,2) = acc;
		WBUFB(buf,6) = sex;
		chmapif_sendall(buf, 7);
	}
	return 1;
}

/**
 * Changes a character's sex.
 * The information is updated on database, and the character is kicked if it
 * currently is online.
 *
 * @param char_id The character's ID.
 * @param sex     The new sex.
 * @retval 0 in case of success.
 * @retval 1 in case of failure.
 */
int chlogif_parse_ackchangecharsex(int char_id, int sex)
{
	int class_ = 0, guild_id = 0, account_id = 0;
	unsigned char buf[7];
	char *data;

	// get character data
	if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`,`class`,`guild_id` FROM `%s` WHERE `char_id` = '%d'", schema_config.char_db, char_id)) {
		Sql_ShowDebug(sql_handle);
		return 1;
	}
	if (Sql_NumRows(sql_handle) != 1 || SQL_ERROR == Sql_NextRow(sql_handle)) {
		Sql_FreeResult(sql_handle);
		return 1;
	}

	Sql_GetData(sql_handle, 0, &data, nullptr); account_id = atoi(data);
	Sql_GetData(sql_handle, 1, &data, nullptr); class_ = atoi(data);
	Sql_GetData(sql_handle, 2, &data, nullptr); guild_id = atoi(data);
	Sql_FreeResult(sql_handle);

	chlogif_parse_change_sex_sub(sex, account_id, char_id, class_, guild_id);

	// disconnect player if online on char-server
	char_disconnect_player(account_id);

	// notify all mapservers about this change
	WBUFW(buf,0) = 0x2b0d;
	WBUFL(buf,2) = account_id;
	WBUFB(buf,6) = sex;
	chmapif_sendall(buf, 7);
	return 0;
}

int chlogif_parse_ack_global_accreg(int fd){
	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;
	else { //Receive account_reg2 registry, forward to map servers.
		RFIFOW(fd,0) = 0x3804;
		chmapif_sendall(RFIFOP(fd,0), RFIFOW(fd,2));
		RFIFOSKIP(fd, RFIFOW(fd,2));
	}
	return 1;
}

int chlogif_parse_accbannotification(int fd){
	if (RFIFOREST(fd) < 11)
		return 0;
	else { // send to all map-servers to disconnect the player
		unsigned char buf[11];
		WBUFW(buf,0) = 0x2b14;
		WBUFL(buf,2) = RFIFOL(fd,2);
		WBUFB(buf,6) = RFIFOB(fd,6); // 0: change of statut, 1: ban
		WBUFL(buf,7) = RFIFOL(fd,7); // status or final date of a banishment
		chmapif_sendall(buf, 11);
	}
	// disconnect player if online on char-server
	char_disconnect_player(RFIFOL(fd,2));
	RFIFOSKIP(fd,11);
	return 1;
}

int chlogif_parse_askkick(int fd){
	if (RFIFOREST(fd) < 6)
		return 0;
	else {
		uint32 aid = RFIFOL(fd,2);
		RFIFOSKIP(fd,6);

		std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), aid );

		// account is already marked as online!
		if( character != nullptr ){
			if( character->server > -1 )
			{	//Kick it from the map server it is on.
				mapif_disconnectplayer(map_server[character->server].fd, character->account_id, character->char_id, 2);
				if (character->waiting_disconnect == INVALID_TIMER)
					character->waiting_disconnect = add_timer(gettick()+AUTH_TIMEOUT, char_chardb_waiting_disconnect, character->account_id, 0);
			}
			else
			{// Manual kick from char server.
				struct char_session_data *tsd;
				int i;
				ARR_FIND( 0, fd_max, i, session[i] && (tsd = (struct char_session_data*)session[i]->session_data) && tsd->account_id == aid );
				if( i < fd_max )
				{
					chclif_send_auth_result(i,2);  //Send "Someone has already logged in with this id"
					set_eof(i);
				}
				else // still moving to the map-server
					char_set_char_offline(-1, aid);
			}
		}

		// reject auth attempts from map-server
		char_get_authdb().erase( aid );
	}
	return 1;
}

int chlogif_parse_updip(int fd){
	unsigned char buf[2];
	uint32 new_ip = 0;

	/**
	 * !CHECKME: This is intended? Just tell if there's IP sync request
	 * without sending current char IP (if changed) to map-server?
	 **/
	WBUFW(buf,0) = 0x2b1e;
	chmapif_sendall(buf, 2);

	new_ip = host2ip(charserv_config.login_ip_str);
	if (new_ip && new_ip != charserv_config.login_ip)
		charserv_config.login_ip = new_ip; //Update login ip, too.

	new_ip = host2ip(charserv_config.char_ip_str);
	if (new_ip && new_ip != charserv_config.char_ip) { // Char-server IP is updated.
		charserv_config.char_ip = new_ip;
		ShowInfo("Updating IP for [%s].\n", charserv_config.char_ip_str);
		// notify login server about the change
		WFIFOHEAD(fd,6);
		WFIFOW(fd,0) = 0x2736;
		WFIFOL(fd,2) = htonl(charserv_config.char_ip);
		WFIFOSET(fd,6);
	}

	RFIFOSKIP(fd,2);
	return 1;
}

/*
 * AH 0x2743
 * We received the info from login-serv, transmit it to map
 */
int chlogif_parse_vipack(int fd) {
#ifdef VIP_ENABLE
	if (RFIFOREST(fd) < 19)
		return 0;
	else {
		uint32 aid = RFIFOL(fd,2); //aid
		uint32 vip_time = RFIFOL(fd,6); //vip_time
		uint8 flag = RFIFOB(fd,10);
		uint32 groupid = RFIFOL(fd,11); //new group id
		int mapfd = RFIFOL(fd,15); //link to mapserv for ack
		RFIFOSKIP(fd,19);
		chmapif_vipack(mapfd,aid,vip_time,groupid,flag);
	}
#endif
	return 1;
}

/**
 * HA 0x2742
 * Request vip data to loginserv
 * @param aid : account_id to request the vip data
 * @param flag : 0x1 Select info and update old_groupid, 0x2 VIP duration is changed by atcommand or script, 0x8 First request on player login
 * @param add_vip_time : tick to add to vip timestamp
 * @param mapfd: link to mapserv for ack
 * @return 0 if success
 */
int chlogif_reqvipdata(uint32 aid, uint8 flag, int32 timediff, int mapfd) {
	loginif_check(-1);
#ifdef VIP_ENABLE
	WFIFOHEAD(login_fd,15);
	WFIFOW(login_fd,0) = 0x2742;
	WFIFOL(login_fd,2) = aid; //aid
	WFIFOB(login_fd,6) = flag; //flag
	WFIFOL(login_fd,7) = timediff; //req_inc_duration
	WFIFOL(login_fd,11) = mapfd; //req_inc_duration
	WFIFOSET(login_fd,15);
#endif
	return 0;
}

/**
* HA 0x2720
* Request account info to login-server
*/
int chlogif_req_accinfo(int fd, int u_fd, int u_aid, int account_id, int8 type) {
	loginif_check(-1);
	//ShowInfo("%d request account info for %d (type %d)\n", u_aid, account_id, type);
	WFIFOHEAD(login_fd,19);
	WFIFOW(login_fd,0) = 0x2720;
	WFIFOL(login_fd,2) = fd;
	WFIFOL(login_fd,6) = u_fd;
	WFIFOL(login_fd,10) = u_aid;
	WFIFOL(login_fd,14) = account_id;
	WFIFOB(login_fd,18) = type;
	WFIFOSET(login_fd,19);
	return 1;
}

/**
 * AH 0x2721
 * Retrieve account info from login-server, ask inter-server to tell player
 */
int chlogif_parse_AccInfoAck(int fd) {
	if (RFIFOREST(fd) < 19)
		return 0;
	else {
		int8 type = RFIFOB(fd, 18);
		if (type == 0 || RFIFOREST(fd) < 122+NAME_LENGTH) {
			mapif_accinfo_ack(false, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), 0, -1, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr);
			RFIFOSKIP(fd,19);
			return 1;
		}
		type>>=1;
		mapif_accinfo_ack(true, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), type, RFIFOL(fd,19), RFIFOL(fd,23), RFIFOL(fd,27),
			RFIFOCP(fd,31), RFIFOCP(fd,71), RFIFOCP(fd,87), RFIFOCP(fd,111),
			RFIFOCP(fd,122));
		RFIFOSKIP(fd,122+NAME_LENGTH);
	}
	return 1;
}


int chlogif_parse(int fd) {

	// only process data from the login-server
	if( fd != login_fd ) {
		ShowDebug("parse_fromlogin: Disconnecting invalid session #%d (is not the login-server)\n", fd);
		do_close(fd);
		return 0;
	}

	if( session[fd]->flag.eof ) {
		do_close(fd);
		login_fd = -1;
		chlogif_on_disconnect();
		return 0;
	} else if ( session[fd]->flag.ping ) {/* we've reached stall time */
		if( DIFF_TICK(last_tick, session[fd]->rdata_tick) > (stall_time * 2) ) {/* we can't wait any longer */
			set_eof(fd);
			return 0;
		} else if( session[fd]->flag.ping != 2 ) { /* we haven't sent ping out yet */
			WFIFOHEAD(fd,2);// sends a ping packet to login server (will receive pong 0x2718)
			WFIFOW(fd,0) = 0x2719;
			WFIFOSET(fd,2);

			session[fd]->flag.ping = 2;
		}
	}

	while(RFIFOREST(fd) >= 2) {
		// -1: Login server is not connected
		//  0: Avoid processing followup packets (prev was probably incomplete) packet
		//  1: Continue parsing
		int next = 1;
		uint16 command = RFIFOW(fd,0);
		switch( command ) {
			case 0x2711: next = chlogif_parse_ackconnect(fd); break;
			case 0x2713: next = chlogif_parse_ackaccreq(fd); break;
			case 0x2717: next = chlogif_parse_reqaccdata(fd); break;
			case 0x2718: next = chlogif_parse_keepalive(fd); break;
			case 0x2721: next = chlogif_parse_AccInfoAck(fd); break;
			case 0x2723: next = chlogif_parse_ackchangesex(fd); break;
			case 0x2726: next = chlogif_parse_ack_global_accreg(fd); break;
			case 0x2731: next = chlogif_parse_accbannotification(fd); break;
			case 0x2734: next = chlogif_parse_askkick(fd); break;
			case 0x2735: next = chlogif_parse_updip(fd); break;
			case 0x2743: next = chlogif_parse_vipack(fd); break;
			default:
				ShowError("Unknown packet 0x%04x received from login-server, disconnecting.\n", command);
				set_eof(fd);
				return 0;
		}
		if (next == 0)
			return 0; //do not parse next data
	}

	RFIFOFLUSH(fd);
	return 0;
}

TIMER_FUNC(chlogif_check_connect_logserver){
	if (chlogif_isconnected())
		return 0;

	ShowInfo("Attempt to connect to login-server...\n");
	login_fd = make_connection(charserv_config.login_ip, charserv_config.login_port, false,10);
	if (login_fd == -1)
	{	//Try again later. [Skotlex]
		login_fd = 0;
		return 0;
	}
	session[login_fd]->func_parse = chlogif_parse;
	session[login_fd]->flag.server = 1;
	realloc_fifo(login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

	WFIFOHEAD(login_fd,86);
	WFIFOW(login_fd,0) = 0x2710;
	memcpy(WFIFOP(login_fd,2), charserv_config.userid, 24);
	memcpy(WFIFOP(login_fd,26), charserv_config.passwd, 24);
	WFIFOL(login_fd,50) = 0;
	WFIFOL(login_fd,54) = htonl(charserv_config.char_ip);
	WFIFOW(login_fd,58) = htons(charserv_config.char_port);
	memcpy(WFIFOP(login_fd,60), charserv_config.server_name, 20);
	WFIFOW(login_fd,80) = 0;
	WFIFOW(login_fd,82) = charserv_config.char_maintenance;
	WFIFOW(login_fd,84) = charserv_config.char_new_display; //only display (New) if they want to [Kevin]
	WFIFOSET(login_fd,86);

	return 1;
}



int chlogif_isconnected(){
	return session_isActive(login_fd);
}

void do_init_chlogif(void) {
	// establish char-login connection if not present
	add_timer_func_list(chlogif_check_connect_logserver, "check_connect_login_server");
	add_timer_interval(gettick() + 1000, chlogif_check_connect_logserver, 0, 0, 10 * 1000);

	// send a list of all online account IDs to login server
	add_timer_func_list(chlogif_send_acc_tologin, "send_accounts_tologin");
	add_timer_interval(gettick() + 1000, chlogif_send_acc_tologin, 0, 0, 3600 * 1000); //Sync online accounts every hour
}

/// Resets all the data.
void chlogif_reset(void){
	int id;
	// TODO kick everyone out and reset everything or wait for connect and try to reaquire locks [FlavioJS]
	for( id = 0; id < ARRAYLENGTH(map_server); ++id )
		chmapif_server_reset(id);
	flush_fifos();
	exit(EXIT_FAILURE);
}

/// Called when the connection to Login Server is disconnected.
void chlogif_on_disconnect(void){
	ShowWarning("Connection to Login Server lost.\n\n");
}

/// Called when all the connection steps are completed.
void chlogif_on_ready(void)
{
	int i;

	//Send online accounts to login server.
	chlogif_send_acc_tologin(INVALID_TIMER, gettick(), 0, 0);

	// if no map-server already connected, display a message...
	ARR_FIND( 0, ARRAYLENGTH(map_server), i, session_isValid(map_server[i].fd) && !map_server[i].maps.empty() );
	if( i == ARRAYLENGTH(map_server) )
		ShowStatus("Awaiting maps from map-server.\n");
}

void do_final_chlogif(void)
{
	if( login_fd != -1 )
	{
		do_close(login_fd);
		login_fd = -1;
	}
}
