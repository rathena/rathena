// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/utils.h"
#include "../common/strlib.h"
#include "inter.h"
#include "int_guild.h"
#include "char.h"
#include "char_clif.h"
#include "char_mapif.h"
#include "char_logif.h"

#include <stdlib.h>

//early declaration
void chlogif_on_ready(void);
void chlogif_on_disconnect(void);

int chlogif_pincode_notifyLoginPinError( uint32 account_id ){
	if (login_fd > 0 && session[login_fd] && !session[login_fd]->flag.eof){
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2739;
		WFIFOL(login_fd,2) = account_id;
		WFIFOSET(login_fd,6);
		return 1;
	}
	return 0;
}

int chlogif_pincode_notifyLoginPinUpdate( uint32 account_id, char* pin ){
	if (login_fd > 0 && session[login_fd] && !session[login_fd]->flag.eof){
		int size = 8 + PINCODE_LENGTH+1;
		WFIFOHEAD(login_fd,size);
		WFIFOW(login_fd,0) = 0x2738;
		WFIFOW(login_fd,2) = size;
		WFIFOL(login_fd,4) = account_id;
		strncpy( (char*)WFIFOP(login_fd,8), pin, PINCODE_LENGTH+1 );
		WFIFOSET(login_fd,size);
		return 1;
	}
	return 0;
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
			|| ( sd->pincode_change + charserv_config.pincode_config.pincode_changetime ) > time(NULL) ){
				DBMap*  online_char_db = char_get_onlinedb();
				struct online_char_data* node = (struct online_char_data*)idb_get( online_char_db, sd->account_id );

				if( node != NULL && node->pincode_success ){
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

/**
 * Load this character's account id into the 'online accounts' packet
 * @see DBApply
 */
static int chlogif_send_acc_tologin_sub(DBKey key, DBData *data, va_list ap) {
	struct online_char_data* character = db_data2ptr(data);
	int* i = va_arg(ap, int*);
	if(character->server > -1) {
		WFIFOL(login_fd,8+(*i)*4) = character->account_id;
		(*i)++;
		return 1;
	}
	return 0;
}

/**
 * Timered function to send all account_id connected to login-serv
 * @param tid : Timer id
 * @param tick : Scheduled tick
 * @param id : GID linked to that timered call
 * @param data : data transmited for delayed function
 * @return 
 */
int chlogif_send_acc_tologin(int tid, unsigned int tick, int id, intptr_t data) {
	if ( chlogif_isconnected() ){
		DBMap*  online_char_db = char_get_onlinedb();
		// send account list to login server
		int users = online_char_db->size(online_char_db);
		int i = 0;

		WFIFOHEAD(login_fd,8+users*4);
		WFIFOW(login_fd,0) = 0x272d;
		online_char_db->foreach(online_char_db, chlogif_send_acc_tologin_sub, &i, users);
		WFIFOW(login_fd,2) = 8+ i*4;
		WFIFOL(login_fd,4) = i;
		WFIFOSET(login_fd,WFIFOW(login_fd,2));
		return 1;
	}
	return 0;
}

int chlogif_send_usercount(int users){
	if( login_fd > 0 && session[login_fd] )
	{
		// send number of user to login server
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2714;
		WFIFOL(login_fd,2) = users;
		WFIFOSET(login_fd,6);
		return 1;
	}
	return 0;
}


int chlogif_broadcast_user_count(int tid, unsigned int tick, int id, intptr_t data)
{
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

//Send packet forward to login-server for account saving
int chlogif_save_accreg2(unsigned char* buf, int len){
	if (login_fd > 0) {
		WFIFOHEAD(login_fd,len+4);
		memcpy(WFIFOP(login_fd,4), buf, len);
		WFIFOW(login_fd,0) = 0x2728;
		WFIFOW(login_fd,2) = len+4;
		WFIFOSET(login_fd,len+4);
		return 1;
	}
	return 0;
}

int chlogif_request_accreg2(uint32 account_id, uint32 char_id){
	if (login_fd > 0) {
		WFIFOHEAD(login_fd,10);
		WFIFOW(login_fd,0) = 0x272e;
		WFIFOL(login_fd,2) = account_id;
		WFIFOL(login_fd,6) = char_id;
		WFIFOSET(login_fd,10);
		return 1;
	}
	return 0;
}

int chlogif_send_reqaccdata(int fd, struct char_session_data *sd){
        //loginif_isconnected
	if (login_fd > 0) { // request account data
		// request account data
		WFIFOHEAD(fd,6);
		WFIFOW(fd,0) = 0x2716;
		WFIFOL(fd,2) = sd->account_id;
		WFIFOSET(fd,6);
		return 1;
	}
	return 0;
}

int chlogif_send_setacconline(int aid){
        //loginif_isconnected
	if (login_fd > 0 && !session[login_fd]->flag.eof){
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x272b;
		WFIFOL(login_fd,2) = aid;
		WFIFOSET(login_fd,6);
		return 1;
	}
	return 0;
}

void chlogif_send_setallaccoffline(int fd){
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2737;
	WFIFOSET(fd,2);
}

int chlogif_send_setaccoffline(int fd, int aid){
	if (chlogif_isconnected()){
		WFIFOHEAD(fd,6);
		WFIFOW(fd,0) = 0x272c;
		WFIFOL(fd,2) = aid;
		WFIFOSET(fd,6);
		return 1;
	}
	return 0;
}

int chlogif_parse_ackconnect(int fd, struct char_session_data* sd){
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

int chlogif_parse_ackaccreq(int fd, struct char_session_data* sd){
	if (RFIFOREST(fd) < 25)
		return 0;
	{
		uint32 account_id = RFIFOL(fd,2);
		uint32 login_id1 = RFIFOL(fd,6);
		uint32 login_id2 = RFIFOL(fd,10);
		uint8 sex = RFIFOB(fd,14);
		uint8 result = RFIFOB(fd,15);
		int request_id = RFIFOL(fd,16);
		uint32 version = RFIFOL(fd,20);
		uint8 clienttype = RFIFOB(fd,24);
		RFIFOSKIP(fd,25);

		if( session_isActive(request_id) && (sd=(struct char_session_data*)session[request_id]->session_data) &&
			!sd->auth && sd->account_id == account_id && sd->login_id1 == login_id1 && sd->login_id2 == login_id2 && sd->sex == sex )
		{
			int client_fd = request_id;
			sd->version = version;
			sd->clienttype = clienttype;
			if(sd->version != date2version(PACKETVER))
				ShowWarning("s aid=%d has an incorect version=%d in clientinfo. Server compiled for %d\n",
					sd->account_id,sd->version,date2version(PACKETVER));

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

int chlogif_parse_reqaccdata(int fd, struct char_session_data* sd){
	int u_fd; //user fd
	if (RFIFOREST(fd) < 75)
		return 0;

	// find the authenticated session with this account id
	ARR_FIND( 0, fd_max, u_fd, session[u_fd] && (sd = (struct char_session_data*)session[u_fd]->session_data) && sd->auth && sd->account_id == RFIFOL(fd,2) );
	if( u_fd < fd_max )
	{
		int server_id;
		memcpy(sd->email, RFIFOP(fd,6), 40);
		sd->expiration_time = (time_t)RFIFOL(fd,46);
		sd->group_id = RFIFOB(fd,50);
		sd->char_slots = RFIFOB(fd,51);
		if( sd->char_slots > MAX_CHARS ) {
			ShowError("Account '%d' `character_slots` column is higher than supported MAX_CHARS (%d), update MAX_CHARS in mmo.h! capping to MAX_CHARS...\n",sd->account_id,sd->char_slots);
			sd->char_slots = MAX_CHARS;/* cap to maximum */
		} else if ( !sd->char_slots )/* no value aka 0 in sql */
			sd->char_slots = MIN_CHARS;/* cap to minimum */
		safestrncpy(sd->birthdate, (const char*)RFIFOP(fd,52), sizeof(sd->birthdate));
                safestrncpy(sd->pincode, (const char*)RFIFOP(fd,63), sizeof(sd->pincode));
                sd->pincode_change = (time_t)RFIFOL(fd,68);
                sd->isvip = RFIFOB(fd,72);
                sd->chars_vip = RFIFOB(fd,73);
                sd->chars_billing = RFIFOB(fd,74);
		ARR_FIND( 0, ARRAYLENGTH(map_server), server_id, map_server[server_id].fd > 0 && map_server[server_id].map[0] );
		// continued from char_auth_ok...
		if( server_id == ARRAYLENGTH(map_server) || //server not online, bugreport:2359
			(charserv_config.max_connect_user == 0 && sd->group_id != charserv_config.gm_allow_group) ||
			( charserv_config.max_connect_user > 0 && char_count_users() >= charserv_config.max_connect_user && sd->group_id != charserv_config.gm_allow_group ) ) {
			// refuse connection (over populated)
			chclif_reject(u_fd,0);
		} else {
			// send characters to player
			chclif_mmo_char_send(u_fd, sd);
			if(sd->version >= date2version(20110309))
				chlogif_pincode_start(u_fd,sd);
		}
	}
	RFIFOSKIP(fd,75);
	return 1;
}

int chlogif_parse_keepalive(int fd, struct char_session_data* sd){
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
	if (class_ == JOB_BARD || class_ == JOB_DANCER)
		class_ = (sex == SEX_MALE ? JOB_BARD : JOB_DANCER);
	else if (class_ == JOB_CLOWN || class_ == JOB_GYPSY)
		class_ = (sex == SEX_MALE ? JOB_CLOWN : JOB_GYPSY);
	else if (class_ == JOB_BABY_BARD || class_ == JOB_BABY_DANCER)
		class_ = (sex == SEX_MALE ? JOB_BABY_BARD : JOB_BABY_DANCER);
	else if (class_ == JOB_MINSTREL || class_ == JOB_WANDERER)
		class_ = (sex == SEX_MALE ? JOB_MINSTREL : JOB_WANDERER);
	else if (class_ == JOB_MINSTREL_T || class_ == JOB_WANDERER_T)
		class_ = (sex == SEX_MALE ? JOB_MINSTREL_T : JOB_WANDERER_T);
	else if (class_ == JOB_BABY_MINSTREL || class_ == JOB_BABY_WANDERER)
		class_ = (sex == SEX_MALE ? JOB_BABY_MINSTREL : JOB_BABY_WANDERER);
	else if (class_ == JOB_KAGEROU || class_ == JOB_OBORO)
		class_ = (sex == SEX_MALE ? JOB_KAGEROU : JOB_OBORO);

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `equip` = '0' WHERE `char_id` = '%d'", schema_config.inventory_db, char_id))
		Sql_ShowDebug(sql_handle);

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class` = '%d', `weapon` = '0', `shield` = '0', `head_top` = '0', `head_mid` = '0', `head_bottom` = '0' WHERE `char_id` = '%d'", schema_config.char_db, class_, char_id))
		Sql_ShowDebug(sql_handle);
	if (guild_id) // If there is a guild, update the guild_member data [Skotlex]
		inter_guild_sex_changed(guild_id, acc, char_id, sex);
}

int chlogif_parse_ackchangesex(int fd, struct char_session_data* sd)
{
	if (RFIFOREST(fd) < 7)
		return 0;
	{
		unsigned char buf[7];
		int acc = RFIFOL(fd,2);
		int sex = RFIFOB(fd,6);
		RFIFOSKIP(fd,7);

		if (acc > 0) { // TODO: Is this even possible?
			unsigned char i;
			int char_id = 0, class_ = 0, guild_id = 0;
			DBMap* auth_db = char_get_authdb();
			struct auth_node* node = (struct auth_node*)idb_get(auth_db, acc);
			SqlStmt *stmt;

			if (node != NULL)
				node->sex = sex;

			// get characters
			stmt = SqlStmt_Malloc(sql_handle);
			if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `char_id`, `class`, `guild_id` FROM `%s` WHERE `account_id` = '%d'", schema_config.char_db, acc) || SqlStmt_Execute(stmt)) {
				SqlStmt_ShowDebug(stmt);
				SqlStmt_Free(stmt);
			}

			SqlStmt_BindColumn(stmt, 0, SQLDT_INT,   &char_id,  0, NULL, NULL);
			SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT, &class_,   0, NULL, NULL);
			SqlStmt_BindColumn(stmt, 2, SQLDT_INT,   &guild_id, 0, NULL, NULL);

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

	Sql_GetData(sql_handle, 0, &data, NULL); account_id = atoi(data);
	Sql_GetData(sql_handle, 1, &data, NULL); class_ = atoi(data);
	Sql_GetData(sql_handle, 2, &data, NULL); guild_id = atoi(data);
	Sql_FreeResult(sql_handle);

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `sex` = '%c' WHERE `char_id` = '%d'", schema_config.char_db, sex == SEX_MALE ? 'M' : 'F', char_id)) {
		Sql_ShowDebug(sql_handle);
		return 1;
	}
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

int chlogif_parse_ackacc2req(int fd, struct char_session_data* sd){
	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;

	{	//Receive account_reg2 registry, forward to map servers.
		unsigned char buf[13+ACCOUNT_REG2_NUM*sizeof(struct global_reg)];
		memcpy(buf,RFIFOP(fd,0), RFIFOW(fd,2));
		WBUFW(buf,0) = 0x3804; //Map server can now receive all kinds of reg values with the same packet. [Skotlex]
		chmapif_sendall(buf, WBUFW(buf,2));
		RFIFOSKIP(fd, RFIFOW(fd,2));
	}
	return 1;
}

int chlogif_parse_accbannotification(int fd, struct char_session_data* sd){
	if (RFIFOREST(fd) < 11)
		return 0;

	{	// send to all map-servers to disconnect the player
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

int chlogif_parse_askkick(int fd, struct char_session_data* sd){
	if (RFIFOREST(fd) < 6)
		return 0;
	{
		DBMap*  online_char_db = char_get_onlinedb();
		DBMap*  auth_db = char_get_authdb();
		int aid = RFIFOL(fd,2);
		struct online_char_data* character = (struct online_char_data*)idb_get(online_char_db, aid);
		RFIFOSKIP(fd,6);
		if( character != NULL )
		{// account is already marked as online!
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
		idb_remove(auth_db, aid);// reject auth attempts from map-server
	}
	return 1;
}

int chlogif_parse_updip(int fd, struct char_session_data* sd){
	unsigned char buf[2];
	uint32 new_ip = 0;

	WBUFW(buf,0) = 0x2b1e;
	chmapif_sendall(buf, 2);

	new_ip = host2ip(charserv_config.login_ip_str);
	if (new_ip && new_ip != charserv_config.login_ip)
		charserv_config.login_ip = new_ip; //Update login ip, too.

	new_ip = host2ip(charserv_config.char_ip_str);
	if (new_ip && new_ip != charserv_config.char_ip)
	{	//Update ip.
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
	if (RFIFOREST(fd) < 20)
		return 0;
	else {
		uint32 aid = RFIFOL(fd,2); //aid
		uint32 vip_time = RFIFOL(fd,6); //vip_time
		uint8 isvip = RFIFOB(fd,10); //isvip
		uint32 groupid = RFIFOL(fd,11); //new group id
		uint8 isgm = RFIFOB(fd,15); //isgm
		int mapfd = RFIFOL(fd,16); //link to mapserv for ack
		RFIFOSKIP(fd,20);
		chmapif_vipack(mapfd,aid,vip_time,isvip,isgm,groupid);
	}
#endif
	return 1;
}

/**
 * HZ 0x2b2b
 * Request vip data from loginserv
 * @param aid : account_id to request the vip data
 * @param type : &2 define new duration, &1 load info
 * @param add_vip_time : tick to add to vip timestamp
 * @param mapfd: link to mapserv for ack
 * @return 0 if success
 */
int chlogif_reqvipdata(uint32 aid, uint8 type, int32 timediff, int mapfd) {
	loginif_check(-1);
#ifdef VIP_ENABLE
	WFIFOHEAD(login_fd,15);
	WFIFOW(login_fd,0) = 0x2742;
	WFIFOL(login_fd,2) = aid; //aid
	WFIFOB(login_fd,6) = type; //type
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
int chlogif_req_accinfo(int fd, int u_fd, int u_aid, int u_group, int account_id, int8 type) {
	loginif_check(-1);
	//ShowInfo("%d request account info for %d (type %d)\n", u_aid, account_id, type);
	WFIFOHEAD(login_fd,23);
	WFIFOW(login_fd,0) = 0x2720;
	WFIFOL(login_fd,2) = fd;
	WFIFOL(login_fd,6) = u_fd;
	WFIFOL(login_fd,10) = u_aid;
	WFIFOL(login_fd,14) = u_group;
	WFIFOL(login_fd,18) = account_id;
	WFIFOB(login_fd,22) = type;
	WFIFOSET(login_fd,23);
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
		if (type == 0 || RFIFOREST(fd) < 155+PINCODE_LENGTH+NAME_LENGTH) {
			mapif_accinfo_ack(false, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), 0, -1, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			RFIFOSKIP(fd,19);
			return 1;
		}
		type>>=1;
		mapif_accinfo_ack(true, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), type, RFIFOL(fd,19), RFIFOL(fd,23), RFIFOL(fd,27),
			(char*)RFIFOP(fd,31), (char*)RFIFOP(fd,71), (char*)RFIFOP(fd,87), (char*)RFIFOP(fd,111),
			(char*)RFIFOP(fd,122), (char*)RFIFOP(fd,155), (char*)RFIFOP(fd,155+PINCODE_LENGTH));
		RFIFOSKIP(fd,155+PINCODE_LENGTH+NAME_LENGTH);
	}
	return 1;
}


int chlogif_parse(int fd) {
	struct char_session_data* sd = NULL;

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

	sd = (struct char_session_data*)session[fd]->session_data;

	while(RFIFOREST(fd) >= 2) {
		int next=1;
		uint16 command = RFIFOW(fd,0);
		switch( command )
		{
			case 0x2743: next = chlogif_parse_vipack(fd); break;
			// acknowledgement of connect-to-loginserver request
			case 0x2711: next = chlogif_parse_ackconnect(fd,sd); break;
			// acknowledgement of account authentication request
			case 0x2713: next = chlogif_parse_ackaccreq(fd, sd); break;
			// account data
			case 0x2717: next = chlogif_parse_reqaccdata(fd, sd); break;
			// login-server alive packet
			case 0x2718: next = chlogif_parse_keepalive(fd, sd); break;
			// changesex reply
			case 0x2723: next = chlogif_parse_ackchangesex(fd, sd); break;
			// reply to an account_reg2 registry request
			case 0x2729: next = chlogif_parse_ackacc2req(fd, sd); break;
			// State change of account/ban notification (from login-server)
			case 0x2731: next = chlogif_parse_accbannotification(fd, sd); break;
			// Login server request to kick a character out. [Skotlex]
			case 0x2734: next = chlogif_parse_askkick(fd,sd); break;
			// ip address update signal from login server
			case 0x2735: next = chlogif_parse_updip(fd,sd); break;
			// @accinfo result
			case 0x2721: next = chlogif_parse_AccInfoAck(fd); break;
			default:
				ShowError("Unknown packet 0x%04x received from login-server, disconnecting.\n", command);
				set_eof(fd);
				return 0;
		}
		if(next==0) return 0; //do not parse next data
	}

	RFIFOFLUSH(fd);
	return 0;
}

int chlogif_check_connect_logserver(int tid, unsigned int tick, int id, intptr_t data) {
	if (login_fd > 0 && session[login_fd] != NULL)
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
	return (login_fd > 0 && session[login_fd] && !session[login_fd]->flag.eof);
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

/// Checks the conditions for the server to stop.
/// Releases the cookie when all characters are saved.
/// If all the conditions are met, it stops the core loop.
void chlogif_check_shutdown(void)
{
	if( runflag != CHARSERVER_ST_SHUTDOWN )
		return;
	runflag = CORE_ST_STOP;
}

/// Called when the connection to Login Server is disconnected.
void chlogif_on_disconnect(void){
	ShowWarning("Connection to Login Server lost.\n\n");
}

/// Called when all the connection steps are completed.
void chlogif_on_ready(void)
{
	int i;

	chlogif_check_shutdown();

	//Send online accounts to login server.
	chlogif_send_acc_tologin(INVALID_TIMER, gettick(), 0, 0);

	// if no map-server already connected, display a message...
	ARR_FIND( 0, ARRAYLENGTH(map_server), i, map_server[i].fd > 0 && map_server[i].map[0] );
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
