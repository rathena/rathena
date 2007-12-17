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
#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // for stat/lstat/fstat

#define MAX_SERVERS 30
extern struct mmo_char_server server[MAX_SERVERS];
extern uint32 auth_num;
extern int account_id_count;
extern char GM_account_filename[1024];
extern char login_log_unknown_packets_filename[1024];

int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len);
int search_account_index(char* account_name);
int mmo_auth_new(struct mmo_account* account, char sex, char* email);
void mmo_auth_sync(void);
int mmo_auth_tostr(char* str, struct auth_data* p);
int read_gm_account(void);
void send_GM_accounts(int fd);
int isGM(int account_id);

//---------------------------------------
// Packet parsing for administation login
//---------------------------------------
int parse_admin(int fd)
{
	unsigned int i, j;
	char* account_name;

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

		case 0x7532:	// Request of end of connection
			ShowStatus("'ladmin': End of connection (ip: %s)\n", ip);
			RFIFOSKIP(fd,2);
			set_eof(fd);
			break;

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
						if (auth_dat[j].state == 0 && auth_dat[j].ban_until_time != 0) // if no state and banished
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

		case 0x7930:	// Request for an account creation
			if (RFIFOREST(fd) < 91)
				return 0;
			{
				struct mmo_account ma;
				memcpy(ma.userid,RFIFOP(fd, 2),NAME_LENGTH);
				ma.userid[23] = '\0';
				memcpy(ma.passwd, RFIFOP(fd, 26), NAME_LENGTH);
				ma.passwd[23] = '\0';
				memcpy(ma.lastlogin, "-", 2);
				ma.sex = RFIFOB(fd,50);
				WFIFOW(fd,0) = 0x7931;
				WFIFOL(fd,2) = 0xffffffff;
				memcpy(WFIFOP(fd,6), RFIFOP(fd,2), 24);
				if (strlen(ma.userid) < 4 || strlen(ma.passwd) < 4) {
					ShowNotice("'ladmin': Attempt to create an invalid account (account or pass is too short, ip: %s)\n", ip);
				} else if (ma.sex != 'F' && ma.sex != 'M') {
					ShowNotice("'ladmin': Attempt to create an invalid account (account: %s, received pass: %s, invalid sex, ip: %s)\n", ma.userid, ma.passwd, ip);
				} else if (account_id_count > END_ACCOUNT_NUM) {
					ShowNotice("'ladmin': Attempt to create an account, but there is no more available id number (account: %s, pass: %s, sex: %c, ip: %s)\n", ma.userid, ma.passwd, ma.sex, ip);
				} else {
					remove_control_chars(ma.userid);
					remove_control_chars(ma.passwd);
					for(i = 0; i < auth_num; i++) {
						if (strncmp(auth_dat[i].userid, ma.userid, 24) == 0) {
							ShowNotice("'ladmin': Attempt to create an already existing account (account: %s, pass: %s, received pass: %s, ip: %s)\n", auth_dat[i].userid, auth_dat[i].pass, ma.passwd, ip);
							break;
						}
					}
					if (i == auth_num) {
						int new_id;
						char email[40];
						memcpy(email, RFIFOP(fd,51), 40);
						email[39] = '\0';
						remove_control_chars(email);
						new_id = mmo_auth_new(&ma, ma.sex, email);
						ShowNotice("'ladmin': Account creation (account: %s (id: %d), pass: %s, sex: %c, email: %s, ip: %s)\n", ma.userid, new_id, ma.passwd, ma.sex, auth_dat[i].email, ip);
						WFIFOL(fd,2) = new_id;
						mmo_auth_sync();
					}
				}
				WFIFOSET(fd,30);
				RFIFOSKIP(fd,91);
			}
			break;

		case 0x7932:	// Request for an account deletion
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7933;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				// Char-server is notified of deletion (for characters deletion).
				unsigned char buf[65535];
				WBUFW(buf,0) = 0x2730;
				WBUFL(buf,2) = auth_dat[i].account_id;
				charif_sendallwos(-1, buf, 6);
				// send answer
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				// save deleted account in log file
				ShowNotice("'ladmin': Account deletion (account: %s, id: %d, ip: %s) - saved in next line:\n", auth_dat[i].userid, auth_dat[i].account_id, ip);
				mmo_auth_tostr((char*)buf, &auth_dat[i]);
				ShowNotice("%s\n", buf);
				// delete account
				memset(auth_dat[i].userid, '\0', sizeof(auth_dat[i].userid));
				auth_dat[i].account_id = -1;
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': Attempt to delete an unknown account (account: %s, ip: %s)\n", account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;

		case 0x7934:	// Request to change a password
			if (RFIFOREST(fd) < 50)
				return 0;
			WFIFOW(fd,0) = 0x7935;
			WFIFOL(fd,2) = 0xFFFFFFFF; /// WTF??? an unsigned being set to a -1
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memcpy(auth_dat[i].pass, RFIFOP(fd,26), 24);
				auth_dat[i].pass[23] = '\0';
				remove_control_chars(auth_dat[i].pass);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				ShowNotice("'ladmin': Modification of a password (account: %s, new password: %s, ip: %s)\n", auth_dat[i].userid, auth_dat[i].pass, ip);
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				ShowNotice("'ladmin': Attempt to modify the password of an unknown account (account: %s, ip: %s)\n", account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;

		case 0x7936:	// Request to modify a state
			if (RFIFOREST(fd) < 50)
				return 0;
			{
				char error_message[20];
				uint32 statut;
				WFIFOW(fd,0) = 0x7937;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				statut = RFIFOL(fd,26);
				memcpy(error_message, RFIFOP(fd,30), 20);
				error_message[19] = '\0';
				remove_control_chars(error_message);
				if (statut != 7 || error_message[0] == '\0') { // 7: // 6 = Your are Prohibited to log in until %s
					strcpy(error_message, "-");
				}
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					WFIFOL(fd,2) = auth_dat[i].account_id;
					if (auth_dat[i].state == statut && strcmp(auth_dat[i].error_message, error_message) == 0)
						ShowNotice("'ladmin': Modification of a state, but the state of the account is already the good state (account: %s, received state: %d, ip: %s)\n", account_name, statut, ip);
					else {
						if (statut == 7)
							ShowNotice("'ladmin': Modification of a state (account: %s, new state: %d - prohibited to login until '%s', ip: %s)\n", auth_dat[i].userid, statut, error_message, ip);
						else
							ShowNotice("'ladmin': Modification of a state (account: %s, new state: %d, ip: %s)\n", auth_dat[i].userid, statut, ip);
						if (auth_dat[i].state == 0) {
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
							WBUFL(buf,7) = statut; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentication)
						}
						auth_dat[i].state = statut;
						memcpy(auth_dat[i].error_message, error_message, 20);
						mmo_auth_sync();
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					ShowNotice("'ladmin': Attempt to modify the state of an unknown account (account: %s, received state: %d, ip: %s)\n", account_name, statut, ip);
				}
				WFIFOL(fd,30) = statut;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,50);
			break;

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
			i = search_account_index(account_name);
			if (i != -1) {
				char pass[25];
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memcpy(pass, RFIFOP(fd,26), 24);
				pass[24] = '\0';
				remove_control_chars(pass);
				if (strcmp(auth_dat[i].pass, pass) == 0) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					ShowNotice("'ladmin': Check of password OK (account: %s, password: %s, ip: %s)\n", auth_dat[i].userid, auth_dat[i].pass, ip);
				} else {
					ShowNotice("'ladmin': Failure of password check (account: %s, proposed pass: %s, ip: %s)\n", auth_dat[i].userid, pass, ip);
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
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
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
					i = search_account_index(account_name);
					if (i != -1) {
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						if (auth_dat[i].sex != ((sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm'))) {
							unsigned char buf[16];
							WFIFOL(fd,2) = auth_dat[i].account_id;
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentication)
							auth_dat[i].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');
							ShowNotice("'ladmin': Modification of a sex (account: %s, new sex: %c, ip: %s)\n", auth_dat[i].userid, sex, ip);
							mmo_auth_sync();
							// send to all char-server the change
							WBUFW(buf,0) = 0x2723;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = auth_dat[i].sex;
							charif_sendallwos(-1, buf, 7);
						} else {
							ShowNotice("'ladmin': Modification of a sex, but the sex is already the good sex (account: %s, sex: %c, ip: %s)\n", auth_dat[i].userid, sex, ip);
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
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char new_gm_level;
				new_gm_level = RFIFOB(fd,26);
				if (new_gm_level < 0 || new_gm_level > 99) {
					ShowNotice("'ladmin': Attempt to give an invalid GM level (account: %s, received GM level: %d, ip: %s)\n", account_name, (int)new_gm_level, ip);
				} else {
					i = search_account_index(account_name);
					if (i != -1) {
						int acc = auth_dat[i].account_id;
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						if (isGM(acc) != new_gm_level) {
							// modification of the file
							FILE *fp, *fp2;
							int lock;
							char line[512];
							int GM_account, GM_level;
							int modify_flag;
							char tmpstr[24];
							time_t raw_time;
							if ((fp2 = lock_fopen(GM_account_filename, &lock)) != NULL) {
								if ((fp = fopen(GM_account_filename, "r")) != NULL) {
									time(&raw_time);
									strftime(tmpstr, 23, login_config.date_format, localtime(&raw_time));
									modify_flag = 0;
									// read/write GM file
									while(fgets(line, sizeof(line), fp))
									{
										while(line[0] != '\0' && (line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r'))
											line[strlen(line)-1] = '\0'; // TODO: remove this
										if ((line[0] == '/' && line[1] == '/') || line[0] == '\0')
											fprintf(fp2, "%s\n", line);
										else {
											if (sscanf(line, "%d %d", &GM_account, &GM_level) != 2 && sscanf(line, "%d: %d", &GM_account, &GM_level) != 2)
												fprintf(fp2, "%s\n", line);
											else if (GM_account != acc)
												fprintf(fp2, "%s\n", line);
											else if (new_gm_level < 1) {
												fprintf(fp2, "// %s: 'ladmin' GM level removed on account %d '%s' (previous level: %d)\n//%d %d\n", tmpstr, acc, auth_dat[i].userid, GM_level, acc, new_gm_level);
												modify_flag = 1;
											} else {
												fprintf(fp2, "// %s: 'ladmin' GM level on account %d '%s' (previous level: %d)\n%d %d\n", tmpstr, acc, auth_dat[i].userid, GM_level, acc, new_gm_level);
												modify_flag = 1;
											}
										}
									}
									if (modify_flag == 0)
										fprintf(fp2, "// %s: 'ladmin' GM level on account %d '%s' (previous level: 0)\n%d %d\n", tmpstr, acc, auth_dat[i].userid, acc, new_gm_level);
									fclose(fp);
								} else {
									ShowNotice("'ladmin': Attempt to modify of a GM level - impossible to read GM accounts file (account: %s (%d), received GM level: %d, ip: %s)\n", auth_dat[i].userid, acc, (int)new_gm_level, ip);
								}
								if (lock_fclose(fp2, GM_account_filename, &lock) == 0) {
									WFIFOL(fd,2) = acc;
									ShowNotice("'ladmin': Modification of a GM level (account: %s (%d), new GM level: %d, ip: %s)\n", auth_dat[i].userid, acc, (int)new_gm_level, ip);
									// read and send new GM informations
									read_gm_account();
									send_GM_accounts(-1);
								} else {
									ShowNotice("'ladmin': Attempt to modify of a GM level - impossible to write GM accounts file (account: %s (%d), received GM level: %d, ip: %s)\n", auth_dat[i].userid, acc, (int)new_gm_level, ip);
								}
							} else {
								ShowNotice("'ladmin': Attempt to modify of a GM level - impossible to write GM accounts file (account: %s (%d), received GM level: %d, ip: %s)\n", auth_dat[i].userid, acc, (int)new_gm_level, ip);
							}
						} else {
							ShowNotice("'ladmin': Attempt to modify of a GM level, but the GM level is already the good GM level (account: %s (%d), GM level: %d, ip: %s)\n", auth_dat[i].userid, acc, (int)new_gm_level, ip);
						}
					} else {
						ShowNotice("'ladmin': Attempt to modify the GM level of an unknown account (account: %s, received GM level: %d, ip: %s)\n", account_name, (int)new_gm_level, ip);
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
					auth_dat[i].connect_until_time = timestamp;
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
					if (auth_dat[i].ban_until_time != timestamp) {
						if (timestamp != 0) {
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
							WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentication)
						}
						auth_dat[i].ban_until_time = timestamp;
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
					if (auth_dat[i].ban_until_time == 0 || auth_dat[i].ban_until_time < time(NULL))
						timestamp = time(NULL);
					else
						timestamp = auth_dat[i].ban_until_time;
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
						if (auth_dat[i].ban_until_time != timestamp) {
							if (timestamp != 0) {
								unsigned char buf[16];
								WBUFW(buf,0) = 0x2731;
								WBUFL(buf,2) = auth_dat[i].account_id;
								WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
								WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
								charif_sendallwos(-1, buf, 11);
								for(j = 0; j < AUTH_FIFO_SIZE; j++)
									if (auth_fifo[j].account_id == auth_dat[i].account_id) {
										auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentication)
										break;
									}
							}
							auth_dat[i].ban_until_time = timestamp;
							mmo_auth_sync();
						}
					} else {
						strftime(tmpstr, 24, login_config.date_format, localtime(&auth_dat[i].ban_until_time));
						ShowNotice("'ladmin': Impossible to adjust the final date of a banishment (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)\n", auth_dat[i].userid, auth_dat[i].ban_until_time, (auth_dat[i].ban_until_time == 0 ? "no banishment" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip);
					}
					WFIFOL(fd,30) = (unsigned long)auth_dat[i].ban_until_time;
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
					timestamp = auth_dat[i].connect_until_time;
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
						strftime(tmpstr, 24, login_config.date_format, localtime(&auth_dat[i].connect_until_time));
						strftime(tmpstr2, 24, login_config.date_format, localtime(&timestamp));
						ShowNotice("'ladmin': Adjustment of a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)\n", auth_dat[i].userid, auth_dat[i].connect_until_time, (auth_dat[i].connect_until_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "unlimited" : tmpstr2), ip);
						auth_dat[i].connect_until_time = timestamp;
						mmo_auth_sync();
						WFIFOL(fd,30) = (unsigned long)auth_dat[i].connect_until_time;
					} else {
						strftime(tmpstr, 24, login_config.date_format, localtime(&auth_dat[i].connect_until_time));
						ShowNotice("'ladmin': Impossible to adjust a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)\n", auth_dat[i].userid, auth_dat[i].connect_until_time, (auth_dat[i].connect_until_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip);
						WFIFOL(fd,30) = 0;
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					ShowNotice("'ladmin': Attempt to adjust the validity limit of an unknown account (account: %s, ip: %s)\n", account_name, ip);
					WFIFOL(fd,30) = 0;
				}
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;

		case 0x7952:	// Request about informations of an account (by account name)
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				WFIFOL(fd,2) = auth_dat[i].account_id;
				WFIFOB(fd,6) = (unsigned char)isGM(auth_dat[i].account_id);
				memcpy(WFIFOP(fd,7), auth_dat[i].userid, 24);
				WFIFOB(fd,31) = auth_dat[i].sex;
				WFIFOL(fd,32) = auth_dat[i].logincount;
				WFIFOL(fd,36) = auth_dat[i].state;
				memcpy(WFIFOP(fd,40), auth_dat[i].error_message, 20);
				memcpy(WFIFOP(fd,60), auth_dat[i].lastlogin, 24);
				memcpy(WFIFOP(fd,84), auth_dat[i].last_ip, 16);
				memcpy(WFIFOP(fd,100), auth_dat[i].email, 40);
				WFIFOL(fd,140) = (unsigned long)auth_dat[i].connect_until_time;
				WFIFOL(fd,144) = (unsigned long)auth_dat[i].ban_until_time;
				WFIFOW(fd,148) = (uint16)strlen(auth_dat[i].memo);
				if (auth_dat[i].memo[0]) {
					memcpy(WFIFOP(fd,150), auth_dat[i].memo, strlen(auth_dat[i].memo));
				}
				ShowNotice("'ladmin': Sending information of an account (request by the name; account: %s, id: %d, ip: %s)\n", auth_dat[i].userid, auth_dat[i].account_id, ip);
				WFIFOSET(fd,150+strlen(auth_dat[i].memo));
			} else {
				memcpy(WFIFOP(fd,7), account_name, 24);
				WFIFOW(fd,148) = 0;
				ShowNotice("'ladmin': Attempt to obtain information (by the name) of an unknown account (account: %s, ip: %s)\n", account_name, ip);
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,26);
			break;

		case 0x7954:	// Request about information of an account (by account id)
			if (RFIFOREST(fd) < 6)
				return 0;
			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,7), '\0', 24);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == (int)RFIFOL(fd,2)) {
					ShowNotice("'ladmin': Sending information of an account (request by the id; account: %s, id: %d, ip: %s)\n", auth_dat[i].userid, RFIFOL(fd,2), ip);
					WFIFOB(fd,6) = (unsigned char)isGM(auth_dat[i].account_id);
					memcpy(WFIFOP(fd,7), auth_dat[i].userid, 24);
					WFIFOB(fd,31) = auth_dat[i].sex;
					WFIFOL(fd,32) = auth_dat[i].logincount;
					WFIFOL(fd,36) = auth_dat[i].state;
					memcpy(WFIFOP(fd,40), auth_dat[i].error_message, 20);
					memcpy(WFIFOP(fd,60), auth_dat[i].lastlogin, 24);
					memcpy(WFIFOP(fd,84), auth_dat[i].last_ip, 16);
					memcpy(WFIFOP(fd,100), auth_dat[i].email, 40);
					WFIFOL(fd,140) = (unsigned long)auth_dat[i].connect_until_time;
					WFIFOL(fd,144) = (unsigned long)auth_dat[i].ban_until_time;
					WFIFOW(fd,148) = (uint16)strlen(auth_dat[i].memo);
					if (auth_dat[i].memo[0]) {
						memcpy(WFIFOP(fd,150), auth_dat[i].memo, strlen(auth_dat[i].memo));
					}
					WFIFOSET(fd,150+strlen(auth_dat[i].memo));
					break;
				}
			}
			if (i == auth_num) {
				ShowNotice("'ladmin': Attempt to obtain information (by the id) of an unknown account (id: %d, ip: %s)\n", RFIFOL(fd,2), ip);
				strncpy((char*)WFIFOP(fd,7), "", 24);
				WFIFOW(fd,148) = 0;
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,6);
			break;

		case 0x7955:	// Request to reload GM file (no answer)
			ShowStatus("'ladmin': Request to re-load GM configuration file (ip: %s).\n", ip);
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts(-1);
			RFIFOSKIP(fd,2);
			break;

		default:
			{
				FILE *logfp;
				char tmpstr[24];
				time_t raw_time;
				logfp = fopen(login_log_unknown_packets_filename, "a");
				if (logfp) {
					time(&raw_time);
					strftime(tmpstr, 23, login_config.date_format, localtime(&raw_time));
					fprintf(logfp, "%s: receiving of an unknown packet -> disconnection\n", tmpstr);
					fprintf(logfp, "parse_admin: connection #%d (ip: %s), packet: 0x%x (with being read: %lu).\n", fd, ip, command, (unsigned long)RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):\n");
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F\n");
					memset(tmpstr, '\0', sizeof(tmpstr));
					for(i = 0; i < RFIFOREST(fd); i++) {
						if ((i & 15) == 0)
							fprintf(logfp, "%04X ",i);
						fprintf(logfp, "%02x ", RFIFOB(fd,i));
						if (RFIFOB(fd,i) > 0x1f)
							tmpstr[i % 16] = RFIFOB(fd,i);
						else
							tmpstr[i % 16] = '.';
						if ((i - 7) % 16 == 0) // -8 + 1
							fprintf(logfp, " ");
						else if ((i + 1) % 16 == 0) {
							fprintf(logfp, " %s\n", tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s\n", tmpstr);
					}
					fprintf(logfp, "\n");
					fclose(logfp);
				}
			}
			ShowStatus("'ladmin': End of connection, unknown packet (ip: %s)\n", ip);
			set_eof(fd);
			return 0;
		}
	}
	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}
