// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/md5calc.h"
#include "../common/lock.h"
#include "account.h"
#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // for stat/lstat/fstat

#define MAX_SERVERS 30
extern struct mmo_char_server server[MAX_SERVERS];
extern AccountDB* accounts;

int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len);
bool check_password(const char* md5key, int passwdenc, const char* passwd, const char* refpass);
int mmo_auth_new(const char* userid, const char* pass, const char sex, const char* last_ip);
int parse_admin(int fd);


bool ladmin_auth(struct login_session_data* sd, const char* ip)
{
	bool result = false;

	if( str2ip(ip) != host2ip(login_config.admin_allowed_host) )
		ShowNotice("'ladmin'-login: Connection in administration mode REFUSED - IP isn't authorised (ip: %s).\n", ip);
	else
	if( !login_config.admin_state )
		ShowNotice("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (ip: %s)\n", ip);
	else
	if( !check_password(sd->md5key, sd->passwdenc, sd->passwd, login_config.admin_pass) )
		ShowNotice("'ladmin'-login: Connection in administration mode REFUSED - invalid password (ip: %s)\n", ip);
	else
	{
		ShowNotice("'ladmin'-login: Connection in administration mode accepted (ip: %s)\n", ip);
		session[sd->fd]->func_parse = parse_admin;
		result = true;
	}

	return result;
}

//---------------------------------------
// Packet parsing for administation login
//
// List of supported operations:
// 0x7530 - request server version (response: 0x7531)
// 0x7938 - request server list (response: 0x7939)
// 0x7920 - request entire list of accounts (response: 0x7921)
// 0x794e - request message broadcast (response: 0x794f + 0x2726)

// 0x7930 - request account creation (response: 0x7931)
// 0x7932 - request account deletion (response: 0x7933 + 0x2730)

// 0x7934 - request account password modification (response: 0x7935)
// 0x7936 - request account state modification (response: 0x7937 + 0x2731)
// 0x793a - request password check (response: 0x793b)
// 0x793c - request account sex modification (response: 0x793d + 0x2723)
// 0x793e - request account gm-level modification (response: 0x793f)
// 0x7940 - request account email modification (response: 0x7941)
// 0x7942 - request account memo modification (response: 0x7943)
// 0x7948 - request account expiration-time modification - absolute (response: 0x7949)
// 0x7950 - request account expiration-time modification - relative (response: 0x7951)
// 0x794a - request account unban-time modification - absolute (response: 0x794b + 0x2731)
// 0x794c - request account unban-time modification - relative (response: 0x794d + 0x2731)

// 0x7944 - request account id lookup by name (response: 0x7945)
// 0x7946 - request account name lookup by id (response: 0x7947)
// 0x7952 - request account information lookup by name (response: 0x7953)
// 0x7954 - request account information lookup by id (response: 0x7953)
//---------------------------------------
int parse_admin(int fd)
{
	unsigned int i, j;
	char* account_name;
	struct mmo_account acc;

	uint32 ipl = session[fd]->client_addr;
	char ip[16];
	ip2str(ipl, ip);

	if( session[fd]->flag.eof )
	{
		do_close(fd);
		ShowInfo("Remote administration has disconnected (session #%d).\n", fd);
		return 0;
	}

	while( RFIFOREST(fd) >= 2 )
	{
		uint16 command = RFIFOW(fd,0);

		switch( command )
		{
		
		case 0x7530:	// Request of the server version
			ShowStatus("'ladmin': Sending of the server version (ip: %s)\n", ip);
			WFIFOHEAD(fd,10);
			WFIFOW(fd,0) = 0x7531;
			WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd,4) = ATHENA_REVISION;
			WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7) = ATHENA_SERVER_LOGIN;
			WFIFOW(fd,8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);
			RFIFOSKIP(fd,2);
			break;
/*
		case 0x7920:	// Request of an accounts list
			if (RFIFOREST(fd) < 10)
				return 0;
			{
				int st, ed;
				uint16 len;
				CREATE_BUFFER(id, int, auth_num);
				st = RFIFOL(fd,2);
				ed = RFIFOL(fd,6);
				RFIFOSKIP(fd,10);
				WFIFOW(fd,0) = 0x7921;
				if (st < 0)
					st = 0;
				if (ed > END_ACCOUNT_NUM || ed < st || ed <= 0)
					ed = END_ACCOUNT_NUM;
				ShowStatus("'ladmin': Sending an accounts list (ask: from %d to %d, ip: %s)\n", st, ed, ip);
				// Sort before send
				for(i = 0; i < auth_num; i++) {
					unsigned int k;
					id[i] = i;
					for(j = 0; j < i; j++) {
						if (auth_dat[id[i]].account_id < auth_dat[id[j]].account_id) {
							for(k = i; k > j; k--) {
								id[k] = id[k-1];
							}
							id[j] = i; // id[i]
							break;
						}
					}
				}
				// Sending accounts information
				len = 4;
				for(i = 0; i < auth_num && len < 30000; i++) {
					int account_id = auth_dat[id[i]].account_id; // use sorted index
					if (account_id >= st && account_id <= ed) {
						j = id[i];
						WFIFOL(fd,len) = account_id;
						WFIFOB(fd,len+4) = (unsigned char)isGM(account_id);
						memcpy(WFIFOP(fd,len+5), auth_dat[j].userid, 24);
						WFIFOB(fd,len+29) = auth_dat[j].sex;
						WFIFOL(fd,len+30) = auth_dat[j].logincount;
						if (auth_dat[j].state == 0 && auth_dat[j].unban_time != 0) // if no state and banished
							WFIFOL(fd,len+34) = 7; // 6 = Your are Prohibited to log in until %s
						else
							WFIFOL(fd,len+34) = auth_dat[j].state;
						len += 38;
					}
				}
				WFIFOW(fd,2) = len;
				WFIFOSET(fd,len);
				//if (id) free(id);
				DELETE_BUFFER(id);
			}
			break;
*/
		case 0x7930:	// Request for an account creation
			if (RFIFOREST(fd) < 91)
				return 0;
		{
			struct mmo_account ma;
			safestrncpy(ma.userid, (char*)RFIFOP(fd, 2), sizeof(ma.userid));
			safestrncpy(ma.pass, (char*)RFIFOP(fd,26), sizeof(ma.pass));
			ma.sex = RFIFOB(fd,50);
			safestrncpy(ma.email, (char*)RFIFOP(fd,51), sizeof(ma.email));
			safestrncpy(ma.lastlogin, "-", sizeof(ma.lastlogin));

			ShowNotice("'ladmin': Account creation request (account: %s pass: %s, sex: %c, email: %s, ip: %s)\n", ma.userid, ma.pass, ma.sex, ma.email, ip);

			WFIFOW(fd,0) = 0x7931;
			WFIFOL(fd,2) = mmo_auth_new(ma.userid, ma.pass, ma.sex, ip);
			safestrncpy((char*)WFIFOP(fd,6), ma.userid, 24);
			WFIFOSET(fd,30);
		}
			RFIFOSKIP(fd,91);
			break;
/*
		case 0x7932:	// Request for an account deletion
			if (RFIFOREST(fd) < 26)
				return 0;
		{
			struct mmo_account acc;

			char* account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';

			WFIFOW(fd,0) = 0x7933;

			if( accounts->load_str(accounts, &acc, account_name) )
			{
				// Char-server is notified of deletion (for characters deletion).
				unsigned char buf[65535];
				WBUFW(buf,0) = 0x2730;
				WBUFL(buf,2) = acc.account_id;
				charif_sendallwos(-1, buf, 6);

				// send answer
				memcpy(WFIFOP(fd,6), acc.userid, 24);
				WFIFOL(fd,2) = acc.account_id;

				// delete account
				memset(acc.userid, '\0', sizeof(acc.userid));
				auth_dat[i].account_id = -1;
				mmo_auth_sync();
			} else {
				WFIFOL(fd,2) = -1;
				memcpy(WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': Attempt to delete an unknown account (account: %s, ip: %s)\n", account_name, ip);
			}
			WFIFOSET(fd,30);
		}
			RFIFOSKIP(fd,26);
			break;
*/
		case 0x7934:	// Request to change a password
			if (RFIFOREST(fd) < 50)
				return 0;
		{
			struct mmo_account acc;

			char* account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';

			WFIFOW(fd,0) = 0x7935;

			if( accounts->load_str(accounts, &acc, account_name) )
			{
				WFIFOL(fd,2) = acc.account_id;
				safestrncpy((char*)WFIFOP(fd,6), acc.userid, 24);
				safestrncpy(acc.pass, (char*)RFIFOP(fd,26), 24);
				ShowNotice("'ladmin': Modification of a password (account: %s, new password: %s, ip: %s)\n", acc.userid, acc.pass, ip);

				accounts->save(accounts, &acc);
			}
			else
			{
				WFIFOL(fd,2) = -1;
				safestrncpy((char*)WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': Attempt to modify the password of an unknown account (account: %s, ip: %s)\n", account_name, ip);
			}

			WFIFOSET(fd,30);
		}
			RFIFOSKIP(fd,50);
			break;

		case 0x7936:	// Request to modify a state
			if (RFIFOREST(fd) < 50)
				return 0;
		{
			struct mmo_account acc;

			char* account_name = (char*)RFIFOP(fd,2);
			uint32 state = RFIFOL(fd,26);
			account_name[23] = '\0';

			WFIFOW(fd,0) = 0x7937;

			if( accounts->load_str(accounts, &acc, account_name) )
			{
				memcpy(WFIFOP(fd,6), acc.userid, 24);
				WFIFOL(fd,2) = acc.account_id;

				if (acc.state == state)
					ShowNotice("'ladmin': Modification of a state, but the state of the account already has this value (account: %s, received state: %d, ip: %s)\n", account_name, state, ip);
				else
				{
					ShowNotice("'ladmin': Modification of a state (account: %s, new state: %d, ip: %s)\n", acc.userid, state, ip);

					if (acc.state == 0) {
						unsigned char buf[16];
						WBUFW(buf,0) = 0x2731;
						WBUFL(buf,2) = acc.account_id;
						WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
						WBUFL(buf,7) = state; // status or final date of a banishment
						charif_sendallwos(-1, buf, 11);
					}
					acc.state = state;
					accounts->save(accounts, &acc);
				}
			}
			else
			{
				ShowNotice("'ladmin': Attempt to modify the state of an unknown account (account: %s, received state: %d, ip: %s)\n", account_name, state, ip);
				WFIFOL(fd,2) = -1;
				memcpy(WFIFOP(fd,6), account_name, 24);
			}

			WFIFOL(fd,30) = state;
			WFIFOSET(fd,34);
		}
			RFIFOSKIP(fd,50);
			break;
/*
		case 0x7938:	// Request for servers list and # of online players
		{		
			uint8 server_num = 0;
			ShowStatus("'ladmin': Sending of servers list (ip: %s)\n", ip);
			for(i = 0; i < MAX_SERVERS; i++) {
				if (server[i].fd >= 0) {
					WFIFOL(fd,4+server_num*32) = htonl(server[i].ip);
					WFIFOW(fd,4+server_num*32+4) = htons(server[i].port);
					memcpy(WFIFOP(fd,4+server_num*32+6), server[i].name, 20);
					WFIFOW(fd,4+server_num*32+26) = server[i].users;
					WFIFOW(fd,4+server_num*32+28) = server[i].maintenance;
					WFIFOW(fd,4+server_num*32+30) = server[i].new_;
					server_num++;
				}
			}
			WFIFOW(fd,0) = 0x7939;
			WFIFOW(fd,2) = 4 + 32 * server_num;
			WFIFOSET(fd,4+32*server_num);
			RFIFOSKIP(fd,2);
			break;
		}

		case 0x793a:	// Request to password check
			if (RFIFOREST(fd) < 50)
				return 0;
			WFIFOW(fd,0) = 0x793b;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			if( accounts->load_str(accounts, &acc, account_name) )
			{
				char pass[25];
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memcpy(pass, RFIFOP(fd,26), 24);
				pass[24] = '\0';
				remove_control_chars(pass);
				if (strcmp(acc.pass, pass) == 0) {
					WFIFOL(fd,2) = acc.account_id;
					ShowNotice("'ladmin': Check of password OK (account: %s, password: %s, ip: %s)\n", acc.userid, acc.pass, ip);
				} else {
					ShowNotice("'ladmin': Failure of password check (account: %s, proposed pass: %s, ip: %s)\n", acc.userid, pass, ip);
				}
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': Attempt to check the password of an unknown account (account: %s, ip: %s)\n", account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;

		case 0x793c:	// Request to modify sex
			if (RFIFOREST(fd) < 27)
				return 0;
			WFIFOW(fd,0) = 0x793d;
			WFIFOL(fd,2) = 0xFFFFFFFF; // -1
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char sex;
				sex = RFIFOB(fd,26);
				if (sex != 'F' && sex != 'M') {
					if (sex > 31)
						ShowNotice("'ladmin': Attempt to give an invalid sex (account: %s, received sex: %c, ip: %s)\n", account_name, sex, ip);
					else
						ShowNotice("'ladmin': Attempt to give an invalid sex (account: %s, received sex: 'control char', ip: %s)\n", account_name, ip);
				} else {
					if( accounts->load_str(accounts, &acc, account_name) )
					{
						memcpy(WFIFOP(fd,6), acc.userid, 24);
						if (acc.sex != sex)
						{
							unsigned char buf[16];
							ShowNotice("'ladmin': Modification of a sex (account: %s, new sex: %c, ip: %s)\n", acc.userid, sex, ip);

							WFIFOL(fd,2) = acc.account_id;
							acc.sex = sex;
							accounts->save(accounts, &acc);

							// send to all char-server the change
							WBUFW(buf,0) = 0x2723;
							WBUFL(buf,2) = acc.account_id;
							WBUFB(buf,6) = acc.sex;
							charif_sendallwos(-1, buf, 7);
						} else {
							ShowNotice("'ladmin': Modification of a sex, but the sex is already the good sex (account: %s, sex: %c, ip: %s)\n", acc.userid, sex, ip);
						}
					} else {
						ShowNotice("'ladmin': Attempt to modify the sex of an unknown account (account: %s, received sex: %c, ip: %s)\n", account_name, sex, ip);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;

		case 0x793e:	// Request to modify GM level
			if (RFIFOREST(fd) < 27)
				return 0;
			WFIFOW(fd,0) = 0x793f;
			WFIFOL(fd,2) = 0xFFFFFFFF; // -1
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
		{
			char new_gm_level;
			new_gm_level = RFIFOB(fd,26);
			if( new_gm_level < 0 || new_gm_level > 99 )
				ShowNotice("'ladmin': Attempt to give an invalid GM level (account: %s, received GM level: %d, ip: %s)\n", account_name, (int)new_gm_level, ip);
			else
			if( !accounts->load_str(accounts, &acc, account_name) )
				ShowNotice("'ladmin': Attempt to modify the GM level of an unknown account (account: %s, received GM level: %d, ip: %s)\n", account_name, (int)new_gm_level, ip);
			else
			{
				memcpy(WFIFOP(fd,6), acc.userid, 24);

				if (isGM(acc.account_id) == new_gm_level)
					ShowNotice("'ladmin': Attempt to modify of a GM level, but the GM level is already the good GM level (account: %s (%d), GM level: %d, ip: %s)\n", acc.userid, acc.account_id, (int)new_gm_level, ip);
				else
				{
					//TODO: change level
				}
			}
		}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;

		case 0x7940:	// Request to modify e-mail
			if (RFIFOREST(fd) < 66)
				return 0;
			WFIFOW(fd,0) = 0x7941;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char email[40];
				memcpy(email, RFIFOP(fd,26), 40);
				if (e_mail_check(email) == 0) {
					ShowNotice("'ladmin': Attempt to give an invalid e-mail (account: %s, ip: %s)\n", account_name, ip);
				} else {
					remove_control_chars(email);
					i = search_account_index(account_name);
					if (i != -1) {
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						memcpy(auth_dat[i].email, email, 40);
						WFIFOL(fd,2) = auth_dat[i].account_id;
						ShowNotice("'ladmin': Modification of an email (account: %s, new e-mail: %s, ip: %s)\n", auth_dat[i].userid, email, ip);
						mmo_auth_sync();
					} else {
						ShowNotice("'ladmin': Attempt to modify the e-mail of an unknown account (account: %s, received e-mail: %s, ip: %s)\n", account_name, email, ip);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,66);
			break;

		case 0x7942:	// Request to modify memo field
			if ((int)RFIFOREST(fd) < 28 || (int)RFIFOREST(fd) < (28 + RFIFOW(fd,26)))
				return 0;
			WFIFOW(fd,0) = 0x7943;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				int size_of_memo = sizeof(auth_dat[i].memo);
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memset(auth_dat[i].memo, '\0', size_of_memo);
				if (RFIFOW(fd,26) == 0) {
					strncpy(auth_dat[i].memo, "-", size_of_memo);
				} else if (RFIFOW(fd,26) > size_of_memo - 1) {
					memcpy(auth_dat[i].memo, RFIFOP(fd,28), size_of_memo - 1);
				} else {
					memcpy(auth_dat[i].memo, RFIFOP(fd,28), RFIFOW(fd,26));
				}
				auth_dat[i].memo[size_of_memo - 1] = '\0';
				remove_control_chars(auth_dat[i].memo);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				ShowNotice("'ladmin': Modification of a memo field (account: %s, new memo: %s, ip: %s)\n", auth_dat[i].userid, auth_dat[i].memo, ip);
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': Attempt to modify the memo field of an unknown account (account: %s, ip: %s)\n", account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,28 + RFIFOW(fd,26));
			break;

		case 0x7944:	// Request to found an account id
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7945;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				ShowNotice("'ladmin': Request (by the name) of an account id (account: %s, id: %d, ip: %s)\n", auth_dat[i].userid, auth_dat[i].account_id, ip);
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': ID request (by the name) of an unknown account (account: %s, ip: %s)\n", account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;

		case 0x7946:	// Request to found an account name
			if (RFIFOREST(fd) < 6)
				return 0;
			WFIFOW(fd,0) = 0x7947;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,6), '\0', 24);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == (int)RFIFOL(fd,2)) {
					strncpy((char*)WFIFOP(fd,6), auth_dat[i].userid, 24);
					ShowNotice("'ladmin': Request (by id) of an account name (account: %s, id: %d, ip: %s)\n", auth_dat[i].userid, RFIFOL(fd,2), ip);
					break;
				}
			}
			if (i == auth_num) {
				ShowNotice("'ladmin': Name request (by id) of an unknown account (id: %d, ip: %s)\n", RFIFOL(fd,2), ip);
				strncpy((char*)WFIFOP(fd,6), "", 24);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,6);
			break;

		case 0x7948:	// Request to change the validity limit (timestamp) (absolute value)
			if (RFIFOREST(fd) < 30)
				return 0;
			{
				time_t timestamp;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x7949;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				timestamp = (time_t)RFIFOL(fd,26);
				strftime(tmpstr, 24, login_config.date_format, localtime(&timestamp));
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					ShowNotice("'ladmin': Change of a validity limit (account: %s, new validity: %d (%s), ip: %s)\n", auth_dat[i].userid, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip);
					auth_dat[i].expiration_time = timestamp;
					WFIFOL(fd,2) = auth_dat[i].account_id;
					mmo_auth_sync();
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					ShowNotice("'ladmin': Attempt to change the validity limit of an unknown account (account: %s, received validity: %d (%s), ip: %s)\n", account_name, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip);
				}
				WFIFOL(fd,30) = (unsigned int)timestamp;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;

		case 0x794a:	// Request to change the final date of a banishment (timestamp) (absolute value)
			if (RFIFOREST(fd) < 30)
				return 0;
			{
				time_t timestamp;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x794b;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				timestamp = (time_t)RFIFOL(fd,26);
				if (timestamp <= time(NULL))
					timestamp = 0;
				strftime(tmpstr, 24, login_config.date_format, localtime(&timestamp));
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					WFIFOL(fd,2) = auth_dat[i].account_id;
					ShowNotice("'ladmin': Change of the final date of a banishment (account: %s, new final date of banishment: %d (%s), ip: %s)\n", auth_dat[i].userid, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
					if (auth_dat[i].unban_time != timestamp) {
						if (timestamp != 0) {
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
							WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
						}
						auth_dat[i].unban_time = timestamp;
						mmo_auth_sync();
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					ShowNotice("'ladmin': Attempt to change the final date of a banishment of an unknown account (account: %s, received final date of banishment: %d (%s), ip: %s)\n", account_name, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
				}
				WFIFOL(fd,30) = (unsigned int)timestamp;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;

		case 0x794c:	// Request to change the final date of a banishment (timestamp) (relative change)
			if (RFIFOREST(fd) < 38)
				return 0;
			{
				time_t timestamp;
				struct tm *tmtime;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x794d;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				i = search_account_index(account_name);
				if (i != -1) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					if (auth_dat[i].unban_time == 0 || auth_dat[i].unban_time < time(NULL))
						timestamp = time(NULL);
					else
						timestamp = auth_dat[i].unban_time;
					tmtime = localtime(&timestamp);
					tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,26);
					tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,28);
					tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,30);
					tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,32);
					tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,34);
					tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,36);
					timestamp = mktime(tmtime);
					if (timestamp != -1) {
						if (timestamp <= time(NULL))
							timestamp = 0;
						strftime(tmpstr, 24, login_config.date_format, localtime(&timestamp));
						ShowNotice("'ladmin': Adjustment of a final date of a banishment (account: %s, (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)\n", auth_dat[i].userid, (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
						if (auth_dat[i].unban_time != timestamp) {
							if (timestamp != 0) {
								unsigned char buf[16];
								WBUFW(buf,0) = 0x2731;
								WBUFL(buf,2) = auth_dat[i].account_id;
								WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
								WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
								charif_sendallwos(-1, buf, 11);
							}
							auth_dat[i].unban_time = timestamp;
							mmo_auth_sync();
						}
					} else {
						strftime(tmpstr, 24, login_config.date_format, localtime(&auth_dat[i].unban_time));
						ShowNotice("'ladmin': Impossible to adjust the final date of a banishment (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)\n", auth_dat[i].userid, auth_dat[i].unban_time, (auth_dat[i].unban_time == 0 ? "no banishment" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip);
					}
					WFIFOL(fd,30) = (unsigned long)auth_dat[i].unban_time;
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					ShowNotice("'ladmin': Attempt to adjust the final date of a banishment of an unknown account (account: %s, ip: %s)\n", account_name, ip);
					WFIFOL(fd,30) = 0;
				}
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;

		case 0x794e:	// Request to send a broadcast message
			if (RFIFOREST(fd) < 8 || RFIFOREST(fd) < (8 + RFIFOL(fd,4)))
				return 0;
			WFIFOW(fd,0) = 0x794f;
			WFIFOW(fd,2) = 0xFFFF; // WTF???
			if (RFIFOL(fd,4) < 1) {
				ShowNotice("'ladmin': Receiving a message for broadcast, but message is void (ip: %s)\n", ip);
			} else {
				// at least 1 char-server
				for(i = 0; i < MAX_SERVERS; i++)
					if (server[i].fd >= 0)
						break;
				if (i == MAX_SERVERS) {
					ShowNotice("'ladmin': Receiving a message for broadcast, but no char-server is online (ip: %s)\n", ip);
				} else {
					unsigned char buf[32000];
					char message[32000];
					WFIFOW(fd,2) = 0;
					memset(message, '\0', sizeof(message));
					memcpy(message, RFIFOP(fd,8), RFIFOL(fd,4));
					message[sizeof(message)-1] = '\0';
					remove_control_chars(message);
					if (RFIFOW(fd,2) == 0)
						ShowNotice("'ladmin': Receiving a message for broadcast (message (in yellow): %s, ip: %s)\n", message, ip);
					else
						ShowNotice("'ladmin': Receiving a message for broadcast (message (in blue): %s, ip: %s)\n", message, ip);
					// send same message to all char-servers (no answer)
					memcpy(WBUFP(buf,0), RFIFOP(fd,0), 8 + RFIFOL(fd,4));
					WBUFW(buf,0) = 0x2726;
					charif_sendallwos(-1, buf, 8 + RFIFOL(fd,4));
				}
			}
			WFIFOSET(fd,4);
			RFIFOSKIP(fd,8 + RFIFOL(fd,4));
			break;

		case 0x7950:	// Request to change the validity limite (timestamp) (relative change)
			if (RFIFOREST(fd) < 38)
				return 0;
		{
			time_t timestamp;
			struct tm *tmtime;
			char tmpstr[2048];
			char tmpstr2[2048];
			WFIFOW(fd,0) = 0x7951;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				WFIFOL(fd,2) = auth_dat[i].account_id;
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				timestamp = auth_dat[i].expiration_time;
				if (timestamp == 0 || timestamp < time(NULL))
					timestamp = time(NULL);
				tmtime = localtime(&timestamp);
				tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,26);
				tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,28);
				tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,30);
				tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,32);
				tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,34);
				tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,36);
				timestamp = mktime(tmtime);
				if (timestamp != -1) {
					strftime(tmpstr, 24, login_config.date_format, localtime(&auth_dat[i].expiration_time));
					strftime(tmpstr2, 24, login_config.date_format, localtime(&timestamp));
					ShowNotice("'ladmin': Adjustment of a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)\n", auth_dat[i].userid, auth_dat[i].expiration_time, (auth_dat[i].expiration_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "unlimited" : tmpstr2), ip);
					auth_dat[i].expiration_time = timestamp;
					mmo_auth_sync();
					WFIFOL(fd,30) = (unsigned long)auth_dat[i].expiration_time;
				} else {
					strftime(tmpstr, 24, login_config.date_format, localtime(&auth_dat[i].expiration_time));
					ShowNotice("'ladmin': Impossible to adjust a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)\n", auth_dat[i].userid, auth_dat[i].expiration_time, (auth_dat[i].expiration_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip);
					WFIFOL(fd,30) = 0;
				}
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': Attempt to adjust the validity limit of an unknown account (account: %s, ip: %s)\n", account_name, ip);
				WFIFOL(fd,30) = 0;
			}

			WFIFOSET(fd,34);
		}
			RFIFOSKIP(fd,38);
			break;
*/
		case 0x7952:	// Request about informations of an account (by account name)
			if (RFIFOREST(fd) < 26)
				return 0;
		{
			struct mmo_account acc;

			WFIFOW(fd,0) = 0x7953;

			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';

			if( accounts->load_str(accounts, &acc, account_name) )
			{
				ShowNotice("'ladmin': Sending information of an account (request by the name; account: %s, id: %d, ip: %s)\n", acc.userid, acc.account_id, ip);
				WFIFOL(fd,2) = acc.account_id;
				WFIFOB(fd,6) = acc.level;
				safestrncpy((char*)WFIFOP(fd,7), acc.userid, 24);
				WFIFOB(fd,31) = acc.sex;
				WFIFOL(fd,32) = acc.logincount;
				WFIFOL(fd,36) = acc.state;
				safestrncpy((char*)WFIFOP(fd,40), "-", 20); // error message (removed)
				safestrncpy((char*)WFIFOP(fd,60), acc.lastlogin, 24);
				safestrncpy((char*)WFIFOP(fd,84), acc.last_ip, 16);
				safestrncpy((char*)WFIFOP(fd,100), acc.email, 40);
				WFIFOL(fd,140) = (unsigned long)acc.expiration_time;
				WFIFOL(fd,144) = (unsigned long)acc.unban_time;
				WFIFOW(fd,148) = 0; // previously, this was strlen(memo), and memo went afterwards
			}
			else
			{
				ShowNotice("'ladmin': Attempt to obtain information (by the name) of an unknown account (account: %s, ip: %s)\n", account_name, ip);
				WFIFOL(fd,2) = -1;
				safestrncpy((char*)WFIFOP(fd,7), account_name, 24); // not found
			}

			WFIFOSET(fd,150);
		}
			RFIFOSKIP(fd,26);
			break;

		case 0x7954:	// Request about information of an account (by account id)
			if (RFIFOREST(fd) < 6)
				return 0;
		{
			struct mmo_account acc;

			int account_id = RFIFOL(fd,2);

			WFIFOHEAD(fd,150);
			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = account_id;

			if( accounts->load_num(accounts, &acc, account_id) )
			{
				ShowNotice("'ladmin': Sending information of an account (request by the id; account: %s, id: %d, ip: %s)\n", acc.userid, account_id, ip);
				WFIFOB(fd,6) = acc.level;
				safestrncpy((char*)WFIFOP(fd,7), acc.userid, 24);
				WFIFOB(fd,31) = acc.sex;
				WFIFOL(fd,32) = acc.logincount;
				WFIFOL(fd,36) = acc.state;
				safestrncpy((char*)WFIFOP(fd,40), "-", 20); // error message (removed)
				safestrncpy((char*)WFIFOP(fd,60), acc.lastlogin, 24);
				safestrncpy((char*)WFIFOP(fd,84), acc.last_ip, 16);
				safestrncpy((char*)WFIFOP(fd,100), acc.email, 40);
				WFIFOL(fd,140) = (unsigned long)acc.expiration_time;
				WFIFOL(fd,144) = (unsigned long)acc.unban_time;
				WFIFOW(fd,148) = 0; // previously, this was strlen(memo), and memo went afterwards
			}
			else
			{
				ShowNotice("'ladmin': Attempt to obtain information (by the id) of an unknown account (id: %d, ip: %s)\n", account_id, ip);
				safestrncpy((char*)WFIFOP(fd,7), "", 24); // not found
			}

			WFIFOSET(fd,150);
		}
			RFIFOSKIP(fd,6);
			break;

		default:
			ShowStatus("'ladmin': End of connection, unknown packet (ip: %s)\n", ip);
			set_eof(fd);
			return 0;
		}
	}
	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}
