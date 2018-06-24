// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "loginchrif.hpp"

#include <stdlib.h>
#include <string.h>

#include "../common/showmsg.hpp" //show notice
#include "../common/socket.hpp" //wfifo session
#include "../common/strlib.hpp" //safeprint
#include "../common/timer.hpp" //difftick

#include "account.hpp"
#include "login.hpp"
#include "loginlog.hpp"

//early declaration
void logchrif_on_disconnect(int id);

/**
 * Packet send to all char-servers, except one. (wos: without our self)
 * @param sfd: fd to discard sending to
 * @param buf: packet to send in form of an array buffer
 * @param len: size of packet
 * @return : the number of char-serv the packet was sent to
 */
int logchrif_sendallwos(int sfd, uint8* buf, size_t len) {
	int i, c;
	for( i = 0, c = 0; i < ARRAYLENGTH(ch_server); ++i ) {
		int fd = ch_server[i].fd;
		if( session_isValid(fd) && fd != sfd ){
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			++c;
		}
	}
	return c;
}

/**
 * Timered function to synchronize ip addresses.
 *  Requesting all char to update their registered ip and transmit their new ip.
 *  Performed each ip_sync_interval.
 * @param tid: timer id
 * @param tick: tick of execution
 * @param id: unused
 * @param data: unused
 * @return 0
 */
int logchrif_sync_ip_addresses(int tid, unsigned int tick, int id, intptr_t data) {
	uint8 buf[2];
	ShowInfo("IP Sync in progress...\n");
	WBUFW(buf,0) = 0x2735;
	logchrif_sendallwos(-1, buf, 2);
	return 0;
}




/// Parsing handlers

/**
 * Request from char-server to authenticate an account.
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_reqauth(int fd, int id,char* ip){
	if( RFIFOREST(fd) < 23 )
		return 0;
	else{
		struct auth_node* node;
		uint32 account_id = RFIFOL(fd,2);
		uint32 login_id1 = RFIFOL(fd,6);
		uint32 login_id2 = RFIFOL(fd,10);
		uint8 sex = RFIFOB(fd,14);
		//uint32 ip_ = ntohl(RFIFOL(fd,15));
		int request_id = RFIFOL(fd,19);
		RFIFOSKIP(fd,23);

		node = (struct auth_node*)idb_get(auth_db, account_id);
		if( runflag == LOGINSERVER_ST_RUNNING &&
			node != NULL &&
			node->account_id == account_id &&
			node->login_id1  == login_id1 &&
			node->login_id2  == login_id2 &&
			node->sex        == sex_num2str(sex) /*&&
			node->ip         == ip_*/ ){// found
			//ShowStatus("Char-server '%s': authentication of the account %d accepted (ip: %s).\n", server[id].name, account_id, ip);

			// send ack
			WFIFOHEAD(fd,21);
			WFIFOW(fd,0) = 0x2713;
			WFIFOL(fd,2) = account_id;
			WFIFOL(fd,6) = login_id1;
			WFIFOL(fd,10) = login_id2;
			WFIFOB(fd,14) = sex;
			WFIFOB(fd,15) = 0;// ok
			WFIFOL(fd,16) = request_id;
			WFIFOB(fd,20) = node->clienttype;
			WFIFOSET(fd,21);

			// each auth entry can only be used once
			idb_remove(auth_db, account_id);
		}else{// authentication not found
			ShowStatus("Char-server '%s': authentication of the account %d REFUSED (ip: %s).\n", ch_server[id].name, account_id, ip);
			WFIFOHEAD(fd,21);
			WFIFOW(fd,0) = 0x2713;
			WFIFOL(fd,2) = account_id;
			WFIFOL(fd,6) = login_id1;
			WFIFOL(fd,10) = login_id2;
			WFIFOB(fd,14) = sex;
			WFIFOB(fd,15) = 1;// auth failed
			WFIFOL(fd,16) = request_id;
			WFIFOB(fd,20) = 0;
			WFIFOSET(fd,21);
		}
	}
	return 1;
}

/**
 * Receive a request to update user count for char-server identified by id.
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_ackusercount(int fd, int id){
	if( RFIFOREST(fd) < 6 )
		return 0;
	else{
		int users = RFIFOL(fd,2);
		RFIFOSKIP(fd,6);
		// how many users on world? (update)
		if( ch_server[id].users != users ){
			ShowStatus("set users %s : %d\n", ch_server[id].name, users);
			ch_server[id].users = users;
		}
	}
	return 1;
}

/**
 * Transmit account data to char_server
 * S 2717 aid.W email.40B exp_time.L group_id.B char_slot.B birthdate.11B pincode.5B pincode_change.L
 *  isvip.1B char_vip.1B max_billing.1B (tot 75)  
 * @return -1 : account not found, 1:sucess
 */
int logchrif_send_accdata(int fd, uint32 aid) {
	struct mmo_account acc;
	time_t expiration_time = 0;
	char email[40] = "";
	int group_id = 0;
	char birthdate[10+1] = "";
	char pincode[PINCODE_LENGTH+1];
	char isvip = false;
	uint8 char_slots = MIN_CHARS, char_vip = 0, char_billing = 0;
	AccountDB* accounts = login_get_accounts_db();

	memset(pincode,0,PINCODE_LENGTH+1);
	if( !accounts->load_num(accounts, &acc, aid) )
		return -1;
	else {
		safestrncpy(email, acc.email, sizeof(email));
		expiration_time = acc.expiration_time;
		group_id = acc.group_id;

		safestrncpy(birthdate, acc.birthdate, sizeof(birthdate));
		safestrncpy(pincode, acc.pincode, sizeof(pincode));
#ifdef VIP_ENABLE
		char_vip = login_config.vip_sys.char_increase;
		if( acc.vip_time > time(NULL) ) {
			isvip = true;
			char_slots = login_config.char_per_account + char_vip;
		} else
			char_slots = login_config.char_per_account;
		char_billing = MAX_CHAR_BILLING; //TODO create a config for this
#endif
	}

	WFIFOHEAD(fd,75);
	WFIFOW(fd,0) = 0x2717;
	WFIFOL(fd,2) = aid;
	safestrncpy(WFIFOCP(fd,6), email, 40);
	WFIFOL(fd,46) = (uint32)expiration_time;
	WFIFOB(fd,50) = (unsigned char)group_id;
	WFIFOB(fd,51) = char_slots;
	safestrncpy(WFIFOCP(fd,52), birthdate, 10+1);
	safestrncpy(WFIFOCP(fd,63), pincode, 4+1 );
	WFIFOL(fd,68) = (uint32)acc.pincode_change;
	WFIFOB(fd,72) = isvip;
	WFIFOB(fd,73) = char_vip;
	WFIFOB(fd,74) = char_billing;
	WFIFOSET(fd,75);
	return 1;
}

/**
 * Transmit vip specific data to char-serv (will be transfered to mapserv)
 * @param fd
 * @param acc
 * @param flag 0x1: VIP, 0x2: GM, 0x4: Show rates on player
 * @param mapfd
 */
int logchrif_sendvipdata(int fd, struct mmo_account acc, unsigned char flag, int mapfd) {
#ifdef VIP_ENABLE
	WFIFOHEAD(fd,19);
	WFIFOW(fd,0) = 0x2743;
	WFIFOL(fd,2) = acc.account_id;
	WFIFOL(fd,6) = (int)acc.vip_time;
	WFIFOB(fd,10) = flag;
	WFIFOL(fd,11) = acc.group_id; //new group id
	WFIFOL(fd,15) = mapfd; //link to mapserv
	WFIFOSET(fd,19);
	logchrif_send_accdata(fd,acc.account_id); //refresh char with new setting
#endif
	return 1;
}

/**
 * Receive a request for account data reply by sending all mmo_account information.
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_reqaccdata(int fd, int id, char *ip){
	if( RFIFOREST(fd) < 6 )
		return 0;
	else {
		uint32 aid = RFIFOL(fd,2);
		RFIFOSKIP(fd,6);
		if( logchrif_send_accdata(fd,aid) < 0 )
			ShowNotice("Char-server '%s': account %d NOT found (ip: %s).\n", ch_server[id].name, aid, ip);
	}
	return 1;
}

/**
 * Ping request from char-server to send a reply.
 * @param fd: fd to parse from (char-serv)
 * @return 1 success
 */
int logchrif_parse_keepalive(int fd){
	RFIFOSKIP(fd,2);
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2718;
	WFIFOSET(fd,2);
	return 1;
}

/**
 * Map server send information to change an email of an account via char-server.
 * 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_reqchangemail(int fd, int id, char* ip){
	if (RFIFOREST(fd) < 86)
		return 0;
	else{
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();
		char actual_email[40];
		char new_email[40];

		uint32 account_id = RFIFOL(fd,2);
		safestrncpy(actual_email, RFIFOCP(fd,6), 40);
		safestrncpy(new_email, RFIFOCP(fd,46), 40);
		RFIFOSKIP(fd, 86);

		if( e_mail_check(actual_email) == 0 )
			ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)\n", ch_server[id].name, account_id, ip);
		else if( e_mail_check(new_email) == 0 )
			ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)\n", ch_server[id].name, account_id, ip);
		else if( strcmpi(new_email, "a@a.com") == 0 )
			ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)\n", ch_server[id].name, account_id, ip);
		else if( !accounts->load_num(accounts, &acc, account_id) )
			ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but account doesn't exist (account: %d, ip: %s).\n", ch_server[id].name, account_id, ip);
		else if( strcmpi(acc.email, actual_email) != 0 )
			ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual e-mail is incorrect (account: %d (%s), actual e-mail: %s, proposed e-mail: %s, ip: %s).\n", ch_server[id].name, account_id, acc.userid, acc.email, actual_email, ip);
		else{
			safestrncpy(acc.email, new_email, 40);
			ShowNotice("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s).\n", ch_server[id].name, account_id, acc.userid, new_email, ip);
			// Save
			accounts->save(accounts, &acc);
		}
	}
	return 1;
}

/**
 * Receiving an account state update request from a map-server (relayed via char-server).
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 * TODO seems pretty damn close to logchrif_parse_reqbanacc
 */
int logchrif_parse_requpdaccstate(int fd, int id, char* ip){
	if (RFIFOREST(fd) < 10)
		return 0;
	else{
		struct mmo_account acc;

		uint32 account_id = RFIFOL(fd,2);
		unsigned int state = RFIFOL(fd,6);
		AccountDB* accounts = login_get_accounts_db();

		RFIFOSKIP(fd,10);

		if( !accounts->load_num(accounts, &acc, account_id) )
			ShowNotice("Char-server '%s': Error of Status change (account: %d not found, suggested status %d, ip: %s).\n", ch_server[id].name, account_id, state, ip);
		else if( acc.state == state )
			ShowNotice("Char-server '%s':  Error of Status change - actual status is already the good status (account: %d, status %d, ip: %s).\n", ch_server[id].name, account_id, state, ip);
		else{
			ShowNotice("Char-server '%s': Status change (account: %d, new status %d, ip: %s).\n", ch_server[id].name, account_id, state, ip);

			acc.state = state;
			// Save
			accounts->save(accounts, &acc);

			// notify other servers
			if (state != 0){
				uint8 buf[11];
				WBUFW(buf,0) = 0x2731;
				WBUFL(buf,2) = account_id;
				WBUFB(buf,6) = 0; // 0: change of state, 1: ban
				WBUFL(buf,7) = state; // status or final date of a banishment
				logchrif_sendallwos(-1, buf, 11);
			}
		}
	}
	return 1;
}

/**
 * Receiving a ban request from map-server via char-server.
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 * TODO check logchrif_parse_requpdaccstate for possible merge
 */
int logchrif_parse_reqbanacc(int fd, int id, char* ip){
	if (RFIFOREST(fd) < 10)
		return 0;
	else{
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();

		uint32 account_id = RFIFOL(fd,2);
		int timediff = RFIFOL(fd,6);
		RFIFOSKIP(fd,10);

		if( !accounts->load_num(accounts, &acc, account_id) )
			ShowNotice("Char-server '%s': Error of ban request (account: %d not found, ip: %s).\n", ch_server[id].name, account_id, ip);
		else{
			time_t timestamp;
			if (acc.unban_time == 0 || acc.unban_time < time(NULL))
				timestamp = time(NULL); // new ban
			else
				timestamp = acc.unban_time; // add to existing ban
			timestamp += timediff;
			if (timestamp == -1)
				ShowNotice("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s).\n", ch_server[id].name, account_id, ip);
			else if( timestamp <= time(NULL) || timestamp == 0 )
				ShowNotice("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s).\n", ch_server[id].name, account_id, ip);
			else{
				uint8 buf[11];
				char tmpstr[24];
				timestamp2string(tmpstr, sizeof(tmpstr), timestamp, login_config.date_format);
				ShowNotice("Char-server '%s': Ban request (account: %d, new final date of banishment: %d (%s), ip: %s).\n", ch_server[id].name, account_id, timestamp, tmpstr, ip);

				acc.unban_time = timestamp;

				// Save
				accounts->save(accounts, &acc);

				WBUFW(buf,0) = 0x2731;
				WBUFL(buf,2) = account_id;
				WBUFB(buf,6) = 1; // 0: change of status, 1: ban
				WBUFL(buf,7) = (uint32)timestamp; // status or final date of a banishment
				logchrif_sendallwos(-1, buf, 11);
			}
		}
	}
	return 1;
}

/**
 * Receiving a sex change request (sex is reversed).
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_reqchgsex(int fd, int id, char* ip){
	if( RFIFOREST(fd) < 6 )
		return 0;
	else{
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();

		uint32 account_id = RFIFOL(fd,2);
		RFIFOSKIP(fd,6);

		if( !accounts->load_num(accounts, &acc, account_id) )
			ShowNotice("Char-server '%s': Error of sex change (account: %d not found, ip: %s).\n", ch_server[id].name, account_id, ip);
		else if( acc.sex == 'S' )
			ShowNotice("Char-server '%s': Error of sex change - account to change is a Server account (account: %d, ip: %s).\n", ch_server[id].name, account_id, ip);
		else{
			unsigned char buf[7];
			char sex = ( acc.sex == 'M' ) ? 'F' : 'M'; //Change gender

			ShowNotice("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s).\n", ch_server[id].name, account_id, sex, ip);

			acc.sex = sex;
			// Save
			accounts->save(accounts, &acc);

			// announce to other servers
			WBUFW(buf,0) = 0x2723;
			WBUFL(buf,2) = account_id;
			WBUFB(buf,6) = sex_str2num(sex);
			logchrif_sendallwos(-1, buf, 7);
		}
	}
	return 1;
}

/**
 * We receive account_reg2 from a char-server, and we send them to other char-servers.
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_upd_global_accreg(int fd, int id, char* ip){
	if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
		return 0;
	else{
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();
		uint32 account_id = RFIFOL(fd,4);

		if( !accounts->load_num(accounts, &acc, account_id) )
			ShowStatus("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s).\n", ch_server[id].name, account_id, ip);
		else
			mmo_save_global_accreg(accounts,fd,account_id,RFIFOL(fd, 8));
		RFIFOSKIP(fd,RFIFOW(fd,2));
	}
	return 1;
}

/**
 * Receiving an unban request from map-server via char-server.
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @param ip: char-serv ip (used for info)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_requnbanacc(int fd, int id, char* ip){
	if( RFIFOREST(fd) < 6 )
		return 0;
	else{
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();

		uint32 account_id = RFIFOL(fd,2);
		RFIFOSKIP(fd,6);

		if( !accounts->load_num(accounts, &acc, account_id) )
			ShowNotice("Char-server '%s': Error of UnBan request (account: %d not found, ip: %s).\n", ch_server[id].name, account_id, ip);
		else if( acc.unban_time == 0 )
			ShowNotice("Char-server '%s': Error of UnBan request (account: %d, no change for unban date, ip: %s).\n", ch_server[id].name, account_id, ip);
		else{
			ShowNotice("Char-server '%s': UnBan request (account: %d, ip: %s).\n", ch_server[id].name, account_id, ip);
			acc.unban_time = 0;
			accounts->save(accounts, &acc);
		}
	}
	return 1;
}

/**
 * Set account_id to online.
 * @author [Wizputer]
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_setacconline(int fd, int id){
	if( RFIFOREST(fd) < 6 )
		return 0;
	login_add_online_user(id, RFIFOL(fd,2));
	RFIFOSKIP(fd,6);
	return 1;
}

/**
 * Set account_id to offline.
 * @author  [Wizputer]
 * @param fd: fd to parse from (char-serv)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_setaccoffline(int fd){
	if( RFIFOREST(fd) < 6 )
		return 0;
	login_remove_online_user(RFIFOL(fd,2));
	RFIFOSKIP(fd,6);
	return 1;
}

/**
 * Receive list of all online accounts.
 * @author  [Skotlex]
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_updonlinedb(int fd, int id){
	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;
	else{
		uint32 i, users;
		online_db->foreach(online_db, login_online_db_setoffline, id); //Set all chars from this char-server offline first
		users = RFIFOW(fd,4);
		for (i = 0; i < users; i++) {
			int aid = RFIFOL(fd,6+i*4);
			struct online_login_data *p = (struct online_login_data*)idb_ensure(online_db, aid, login_create_online_user);
			p->char_server = id;
			if (p->waiting_disconnect != INVALID_TIMER){
				delete_timer(p->waiting_disconnect, login_waiting_disconnect_timer);
				p->waiting_disconnect = INVALID_TIMER;
			}
		}
		RFIFOSKIP(fd,RFIFOW(fd,2));
	}
	return 1;
}

/**
 * Request account_reg2 for a character.
 * @param fd: fd to parse from (char-serv)
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_req_global_accreg(int fd){
	if (RFIFOREST(fd) < 10)
		return 0;
	else{
		AccountDB* accounts = login_get_accounts_db();
		uint32 account_id = RFIFOL(fd,2);
		uint32 char_id = RFIFOL(fd,6);
		RFIFOSKIP(fd,10);

		mmo_send_global_accreg(accounts,fd,account_id,char_id);
	}
	return 1;
}

/**
 * Received new charip from char-serv, update information.
 * @param fd: char-serv file descriptor
 * @param id: char-serv id
 * @return 0 not enough info transmitted, 1 success
 */
int logchrif_parse_updcharip(int fd, int id){
	if( RFIFOREST(fd) < 6 )
		return 0;
	ch_server[id].ip = ntohl(RFIFOL(fd,2));
	ShowInfo("Updated IP of Server #%d to %d.%d.%d.%d.\n",id, CONVIP(ch_server[id].ip));
	RFIFOSKIP(fd,6);
	return 1;
}

/**
 * Request to set all accounts offline.
 * @param fd: fd to parse from (char-serv)
 * @param id: id of char-serv (char-serv)
 * @return 1 success
 */
int logchrif_parse_setalloffline(int fd, int id){
	ShowInfo("Setting accounts from char-server %d offline.\n", id);
	online_db->foreach(online_db, login_online_db_setoffline, id);
	RFIFOSKIP(fd,2);
	return 1;
}

#if PACKETVER_SUPPORTS_PINCODE
/**
 * Request to change PIN Code for an account.
 * @param fd: fd to parse from (char-serv)
 * @return 0 fail (packet does not have enough data), 1 success
 */
int logchrif_parse_updpincode(int fd){
	if( RFIFOREST(fd) < 8 + PINCODE_LENGTH+1 )
		return 0;
	else{
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();

		if( accounts->load_num(accounts, &acc, RFIFOL(fd,4) ) ){
			strncpy( acc.pincode, RFIFOCP(fd,8), PINCODE_LENGTH+1 );
			acc.pincode_change = time( NULL );
			accounts->save(accounts, &acc);
		}
		RFIFOSKIP(fd,8 + PINCODE_LENGTH+1);
	}
	return 1;
}

/**
 * PIN Code was incorrectly entered too many times.
 * @param fd: fd to parse from (char-serv)
 * @return 0 fail (packet does not have enough data), 1 success (continue parsing)
 */
int logchrif_parse_pincode_authfail(int fd){
	if( RFIFOREST(fd) < 6 )
		return 0;
	else{
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();
		if( accounts->load_num(accounts, &acc, RFIFOL(fd,2) ) ){
			struct online_login_data* ld;

			ld = (struct online_login_data*)idb_get(online_db,acc.account_id);

			if( ld == NULL )
				return 0;

			login_log( host2ip(acc.last_ip), acc.userid, 100, "PIN Code check failed" );
		}
		login_remove_online_user(acc.account_id);
		RFIFOSKIP(fd,6);
	}
	return 1;
}
#endif

/**
 * Received a vip data reqest from char
 * type is the query to perform
 *  0x1 : Select info and update old_groupid
 *  0x2 : VIP duration is changed by atcommand or script
 *  0x8 : First request on player login
 * @param fd link to charserv
 * @return 0 missing data, 1 succeeded
 */
int logchrif_parse_reqvipdata(int fd) {
#ifdef VIP_ENABLE
	if( RFIFOREST(fd) < 15 )
		return 0;
	else { //request vip info
		struct mmo_account acc;
		AccountDB* accounts = login_get_accounts_db();
		int aid = RFIFOL(fd,2);
		int8 flag = RFIFOB(fd,6);
		int32 timediff = RFIFOL(fd,7);
		int mapfd = RFIFOL(fd,11);
		RFIFOSKIP(fd,15);
		
		if( accounts->load_num(accounts, &acc, aid ) ) {
			time_t now = time(NULL);
			time_t vip_time = acc.vip_time;
			bool isvip = false;

			if( acc.group_id > login_config.vip_sys.group ) { //Don't change group if it's higher.
				logchrif_sendvipdata(fd,acc,0x2|((flag&0x8)?0x4:0),mapfd);
				return 1;
			}
			if( flag&2 ) {
				if(!vip_time)
					vip_time = now; //new entry
				vip_time += timediff; // set new duration
			}
			if( now < vip_time ) { //isvip
				if(acc.group_id != login_config.vip_sys.group) //only upd this if we're not vip already
					acc.old_group = acc.group_id;
				acc.group_id = login_config.vip_sys.group;
				acc.char_slots = login_config.char_per_account + login_config.vip_sys.char_increase;
				isvip = true;
			} else { //expired or @vip -xx
				vip_time = 0;
				if(acc.group_id == login_config.vip_sys.group) //prevent alteration in case account wasn't registered as vip yet
					acc.group_id = acc.old_group;
				acc.old_group = 0;
				acc.char_slots = login_config.char_per_account;
			}
			acc.vip_time = vip_time;
			accounts->save(accounts,&acc);
			if( flag&1 )
				logchrif_sendvipdata(fd,acc,((isvip)?0x1:0)|((flag&0x8)?0x4:0),mapfd);
		}
	}
#endif
	return 1;
}

/**
* IA 0x2720
* Get account info that asked by inter/char-server
*/
int logchrif_parse_accinfo(int fd) {
	if( RFIFOREST(fd) < 23 )
		return 0;
	else {
		int map_fd = RFIFOL(fd, 2), u_fd = RFIFOL(fd, 6), u_aid = RFIFOL(fd, 10), account_id = RFIFOL(fd, 14);
		int8 type = RFIFOB(fd, 18);
		AccountDB* accounts = login_get_accounts_db();
		struct mmo_account acc;
		RFIFOSKIP(fd,19);

		// Send back the result to char-server
		if (accounts->load_num(accounts, &acc, account_id)) {
			int len = 122 + NAME_LENGTH;
			//ShowInfo("Found account info for %d, requested by %d\n", account_id, u_aid);
			WFIFOHEAD(fd, len);
			WFIFOW(fd, 0) = 0x2721;
			WFIFOL(fd, 2) = map_fd;
			WFIFOL(fd, 6) = u_fd;
			WFIFOL(fd, 10) = u_aid;
			WFIFOL(fd, 14) = account_id;
			WFIFOB(fd, 18) = (1<<type); // success
			WFIFOL(fd, 19) = acc.group_id;
			WFIFOL(fd, 23) = acc.logincount;
			WFIFOL(fd, 27) = acc.state;
			safestrncpy(WFIFOCP(fd, 31), acc.email, 40);
			safestrncpy(WFIFOCP(fd, 71), acc.last_ip, 16);
			safestrncpy(WFIFOCP(fd, 87), acc.lastlogin, 24);
			safestrncpy(WFIFOCP(fd, 111), acc.birthdate, 11);
			safestrncpy(WFIFOCP(fd, 122), acc.userid, NAME_LENGTH);
			WFIFOSET(fd, len);
		}
		else {
			//ShowInfo("Cannot found account info for %d, requested by %d\n", account_id, u_aid);
			WFIFOHEAD(fd, 19);
			WFIFOW(fd, 0) = 0x2721;
			WFIFOL(fd, 2) = map_fd;
			WFIFOL(fd, 6) = u_fd;
			WFIFOL(fd, 10) = u_aid;
			WFIFOL(fd, 14) = account_id;
			WFIFOB(fd, 18) = 0; // failed
			WFIFOSET(fd, 19);
		}
	}
	return 1;
}

/**
 * Entry point from char-server to log-server.
 * Function that checks incoming command, then splits it to the correct handler.
 * @param fd: file descriptor to parse, (link to char-serv)
 * @return 0=invalid server,marked for disconnection,unknow packet; 1=success
 */
int logchrif_parse(int fd){
	int cid; //char-serv id
	uint32 ipl;
	char ip[16];

	ARR_FIND( 0, ARRAYLENGTH(ch_server), cid, ch_server[cid].fd == fd );
	if( cid == ARRAYLENGTH(ch_server) ){// not a char server
		ShowDebug("logchrif_parse: Disconnecting invalid session #%d (is not a char-server)\n", fd);
		set_eof(fd);
		do_close(fd);
		return 0;
	}

	if( session[fd]->flag.eof ){
		do_close(fd);
		ch_server[cid].fd = -1;
		logchrif_on_disconnect(cid);
		return 0;
	}

	ipl = ch_server[cid].ip;
	ip2str(ipl, ip);

	while( RFIFOREST(fd) >= 2 ){
		int next = 1; // 0: avoid processing followup packets (prev was probably incomplete) packet, 1: Continue parsing
		uint16 command = RFIFOW(fd,0);
		switch( command ){
			case 0x2712: next = logchrif_parse_reqauth(fd, cid, ip); break;
			case 0x2714: next = logchrif_parse_ackusercount(fd, cid); break;
			case 0x2716: next = logchrif_parse_reqaccdata(fd, cid, ip); break;
			case 0x2719: next = logchrif_parse_keepalive(fd); break;
			case 0x2720: next = logchrif_parse_accinfo(fd); break; //@accinfo from inter-server
			case 0x2722: next = logchrif_parse_reqchangemail(fd,cid,ip); break;
			case 0x2724: next = logchrif_parse_requpdaccstate(fd,cid,ip); break;
			case 0x2725: next = logchrif_parse_reqbanacc(fd,cid,ip); break;
			case 0x2727: next = logchrif_parse_reqchgsex(fd,cid,ip); break;
			case 0x2728: next = logchrif_parse_upd_global_accreg(fd,cid,ip); break;
			case 0x272a: next = logchrif_parse_requnbanacc(fd,cid,ip); break;
			case 0x272b: next = logchrif_parse_setacconline(fd,cid); break;
			case 0x272c: next = logchrif_parse_setaccoffline(fd); break;
			case 0x272d: next = logchrif_parse_updonlinedb(fd,cid); break;
			case 0x272e: next = logchrif_parse_req_global_accreg(fd); break;
			case 0x2736: next = logchrif_parse_updcharip(fd,cid); break;
			case 0x2737: next = logchrif_parse_setalloffline(fd,cid); break;
#if PACKETVER_SUPPORTS_PINCODE
			case 0x2738: next = logchrif_parse_updpincode(fd); break;
			case 0x2739: next = logchrif_parse_pincode_authfail(fd); break;
#endif
			case 0x2742: next = logchrif_parse_reqvipdata(fd); break; //Vip sys
			default:
				ShowError("logchrif_parse: Unknown packet 0x%x from a char-server! Disconnecting!\n", command);
				set_eof(fd);
				return 0;
		} // switch
		if (next == 0)
			return 0;
	} // while
	return 1; //or 0
}




/// Constructor destructor and signal handlers

/**
 * Initializes a server structure.
 * @param id: id of char-serv (should be >0, FIXME)
 */
void logchrif_server_init(int id) {
	memset(&ch_server[id], 0, sizeof(ch_server[id]));
	ch_server[id].fd = -1;
}

/**
 * Destroys a server structure.
 * @param id: id of char-serv (should be >0, FIXME)
 */
void logchrif_server_destroy(int id){
	if( ch_server[id].fd != -1 ) {
		do_close(ch_server[id].fd);
		ch_server[id].fd = -1;
	}
}

/**
 * Resets all the data related to a server.
 *  Actually destroys then recreates the struct.
 * @param id: id of char-serv (should be >0, FIXME)
 */
void logchrif_server_reset(int id) {
	online_db->foreach(online_db, login_online_db_setoffline, id); //Set all chars from this char server to offline.
	logchrif_server_destroy(id);
	logchrif_server_init(id);
}

/**
 * Called when the connection to Char Server is disconnected.
 * @param id: id of char-serv (should be >0, FIXME)
 */
void logchrif_on_disconnect(int id) {
	ShowStatus("Char-server '%s' has disconnected.\n", ch_server[id].name);
	logchrif_server_reset(id);
}

/**
 * loginchrif constructor
 *  Initialisation, function called at start of the login-serv.
 */
void do_init_loginchrif(void){
	int i;
	for( i = 0; i < ARRAYLENGTH(ch_server); ++i )
		logchrif_server_init(i);

	// add timer to detect ip address change and perform update
	if (login_config.ip_sync_interval) {
		add_timer_func_list(logchrif_sync_ip_addresses, "sync_ip_addresses");
		add_timer_interval(gettick() + login_config.ip_sync_interval, logchrif_sync_ip_addresses, 0, 0, login_config.ip_sync_interval);
	}
}

/**
 * Signal handler
 *  This function attempts to properly close the server when an interrupt signal is received.
 *  current signal catch : SIGTERM, SIGINT
 */
void do_shutdown_loginchrif(void){
	int id;
	for( id = 0; id < ARRAYLENGTH(ch_server); ++id )
		logchrif_server_reset(id);
}

/**
 * loginchrif destructor
 *  dealloc..., function called at exit of the login-serv
 */
void do_final_loginchrif(void){
	int i;
	for( i = 0; i < ARRAYLENGTH(ch_server); ++i )
		logchrif_server_destroy(i);
}
