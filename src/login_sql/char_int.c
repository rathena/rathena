#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include "login.h"

//--------------------------------
// Send to char
//--------------------------------
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c;
	int fd;

	c = 0;
	for(i = 0; i < MAX_SERVERS && i < servers_connected; i++) {
		if ((fd = server_fd[i]) > 0 && fd != sfd) {
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

//--------------------------------
// Char-server anti-freeze system
//--------------------------------
int char_anti_freeze_system(int tid, unsigned int tick, int id, int data) {
	int i;

	for(i = 0; i < MAX_SERVERS && i < servers_connected; i++) {
		if (server_fd[i] >= 0) {// if char-server is online
//			printf("char_anti_freeze_system: server #%d '%s', flag: %d.\n", i, server[i].name, server_freezeflag[i]);
			if (server_freezeflag[i]-- < 1) {// Char-server anti-freeze system. Counter. 5 ok, 4...0 freezed
				session[server_fd[i]]->eof = 1;
			}
		}
	}

	return 0;
}

//-------------------------------------------
// Request for account reg from char-server [Edit: Wizputer]
//-------------------------------------------
void send_account_reg(int fd) {
    int account_id = RFIFOL(fd,2);
    int i;
			
    for(i=0;i<AUTH_FIFO_SIZE;i++){
        if (auth_fifo[i].account_id == account_id &&
		    auth_fifo[i].login_id1 == RFIFOL(fd,6) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
		    auth_fifo[i].login_id2 == RFIFOL(fd,10) && // relate to the versions higher than 18 
#endif
		    auth_fifo[i].sex == RFIFOB(fd,14) &&
#if CMP_AUTHFIFO_IP != 0
		    auth_fifo[i].ip == RFIFOL(fd,15) &&
#endif
		    !auth_fifo[i].delflag) {
				auth_fifo[i].delflag = 1;
				#ifdef DEBUG
				printf("Client: [%d] Auth Number: [%d]\n",fd, i);
				#endif
				break;
		}
	}

	if (i != AUTH_FIFO_SIZE) { // send account_reg
		int p;
		time_t connect_until_time = 0;
		char email[40] = "";

		sprintf(tmpsql, "SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%d'", login_db, login_db_account_id, account_id);
		sql_query(tmpsql,"send_account_reg");
		
		
        if ((sql_res = mysql_store_result(&mysql_handle))) {
			if((sql_row = mysql_fetch_row(sql_res))) {
			    connect_until_time = atol(sql_row[1]);
			    strcpy(email, sql_row[0]);
			}    
		}
		mysql_free_result(sql_res);
		
		if (account_id > 0) {
			sprintf(tmpsql, "SELECT `str`,`value` FROM `global_reg_value` WHERE `type`='1' AND `account_id`='%d'",account_id);
			sql_query(tmpsql,"send_account_reg");
			
            if ((sql_res = mysql_store_result(&mysql_handle))) {
				WFIFOW(fd,0) = 0x2729;
				WFIFOL(fd,4) = account_id;
				for(p = 8; (sql_row = mysql_fetch_row(sql_res));p+=36){
					memcpy(WFIFOP(fd,p), sql_row[0], 32);
					WFIFOL(fd,p+32) = atoi(sql_row[1]);
				}
				WFIFOW(fd,2) = p;
				WFIFOSET(fd,p);
				#ifdef DEBUG
				printf("account_reg2 send : login->char (auth fifo)\n");
				#endif
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 0;
				memcpy(WFIFOP(fd, 7), email, 40);
				WFIFOL(fd,47) = (unsigned long) connect_until_time;
				WFIFOSET(fd,51);
			}
			mysql_free_result(sql_res);
		}
	} else {
		WFIFOW(fd,0) = 0x2713;
		WFIFOL(fd,2) = account_id;
		WFIFOB(fd,6) = 1;
		WFIFOSET(fd,51);
	}
	RFIFOSKIP(fd,19);
}

//----------------------------------------------------------
// Number of users in the world (connected char-server(s)) [Edit: Wizputer]
//----------------------------------------------------------
void number_world_users(int fd, int id) {
	#ifdef DEBUG
    if (server[id].users != RFIFOL(fd,2))
		printf("set number users %s : %d\n", server[id].name, RFIFOL(fd,2));
	#endif
		
	server[id].users = RFIFOL(fd,2);
	if(anti_freeze_enable)
		server_freezeflag[id] = 5; // Char anti-freeze system. Counter. 5 ok, 4...0 freezed
		
	sprintf(tmpsql,"UPDATE `sstatus` SET `user` = '%d' WHERE `index` = '%d'", server[id].users, id);
	sql_query(tmpsql,"number_world_users");
	
	RFIFOSKIP(fd,6);
}

//-----------------------------------------
// Email and Time request from char-server [Edit: Wizputer]
//-----------------------------------------
void email_time_request(int fd, int id) {
	int account_id=RFIFOL(fd,2);
	time_t connect_until_time = 0;
	char email[40] = "";
	
	sprintf(tmpsql,"SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%d'",login_db, login_db_account_id, account_id);
	sql_query(tmpsql,"email_time_request");

	if ((sql_res = mysql_store_result(&mysql_handle))) {
		if((sql_row = mysql_fetch_row(sql_res))) {
		    connect_until_time = atol(sql_row[1]);
		    strcpy(email, sql_row[0]);
		}    
	}
	mysql_free_result(sql_res);
	
	#ifdef DEBUG
	printf("parse_fromchar: E-mail/limited time request from '%s' server (concerned account: %d)\n", server[id].name, account_id);
	#endif
	
	WFIFOW(fd,0) = 0x2717;
	WFIFOL(fd,2) = account_id;
	memcpy(WFIFOP(fd, 6), email, 40);
	WFIFOL(fd,46) = (unsigned long) connect_until_time;
	WFIFOSET(fd,50);

	RFIFOSKIP(fd,6);
}

//--------------------------------
// Request to change email [Edit: Wizputer]
//--------------------------------
void change_account_email(int fd, int id, char ip[16]) {
	int acc = RFIFOL(fd,2);
	char actual_email[40], new_email[40];

	memcpy(actual_email, RFIFOP(fd,6), 40);
	memcpy(new_email, RFIFOP(fd,46), 40);

	if (e_mail_check(actual_email) == 0) {
	    #ifdef DEBUG
		printf("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)" RETCODE,
		       server[id].name, acc, ip);
        #endif
	} else if (e_mail_check(new_email) == 0) {
	    #ifdef DEBUG
	    printf("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)" RETCODE,
		       server[id].name, acc, ip);
        #endif     
	} else if (strcmpi(new_email, "athena@athena.com") == 0) {
	    #ifdef DEBUG
		printf("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)" RETCODE,
               server[id].name, acc, ip);
        #endif               
	} else {
		sprintf(tmpsql, "SELECT `%s`,`email` FROM `%s` WHERE `%s` = '%d'", login_db_userid, login_db, login_db_account_id, acc);
		sql_query(tmpsql,"change_account_email");

		if ((sql_res = mysql_store_result(&mysql_handle))) {
			if((sql_row = mysql_fetch_row(sql_res))) {	//row fetching
			    if (strcmpi(sql_row[1], actual_email) == 0) {
				    sprintf(tmpsql, "UPDATE `%s` SET `email` = '%s' WHERE `%s` = '%d'", login_db, new_email, login_db_account_id, acc);
				    sql_query(tmpsql,"change_account_email");
				    
                    #ifdef DEBUG
                    printf("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s)." RETCODE,
                            server[id].name, acc, sql_row[0], actual_email, ip);
                    #endif
                }
            }    
		}
				
    }
    
    RFIFOSKIP(fd, 86);
}

//-----------------------------------------------
// State change request from map server (By Yor) [Edit: Wizputer]
//-----------------------------------------------
void status_change_request(int fd) {
	int acc = RFIFOL(fd,2), status = RFIFOL(fd,6);

	sprintf(tmpsql, "SELECT `state` FROM `%s` WHERE `%s` = '%d'", login_db, login_db_account_id, acc);
	sql_query(tmpsql,"status_change_request");
	
	if ((sql_res = mysql_store_result(&mysql_handle))) {
		if((sql_row = mysql_fetch_row(sql_res))) { // row fetching
		    if (atoi(sql_row[0]) != status && status != 0) {
		        unsigned char buf[16];
		        WBUFW(buf,0) = 0x2731;
		        WBUFL(buf,2) = acc;
		        WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
		        WBUFL(buf,7) = status; // status or final date of a banishment
		        charif_sendallwos(-1, buf, 11);
            }
        }    
			
		sprintf(tmpsql,"UPDATE `%s` SET `state` = '%d' WHERE `%s` = '%d'", login_db, status,login_db_account_id,acc);
		sql_query(tmpsql,"status_change_request");
    }    
	
    RFIFOSKIP(fd,10);
}
//--------------------------------------
// Ban request from map-server (By Yor) [Edit: Wizputer]
//--------------------------------------
void ban_request(int fd) {
	int acc=RFIFOL(fd,2);
	struct tm *tmtime;
	time_t timestamp, tmptime;

	sprintf(tmpsql, "SELECT `ban_until` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);
	sql_query(tmpsql,"ban_request");
	
	if ((sql_res = mysql_store_result(&mysql_handle)) && (sql_row = mysql_fetch_row(sql_res))) {
		tmptime = atol(sql_row[0]);

		if (tmptime == 0 || tmptime < time(NULL))
			timestamp = time(NULL);
		else
			timestamp = tmptime;

		tmtime = localtime(&timestamp);
		tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,6);
		tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,8);
		tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,10);
		tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,12);
		tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,14);
		tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,16);

		timestamp = mktime(tmtime);

		if (timestamp != -1) {
			if (timestamp <= time(NULL))
				timestamp = 0;
			if (tmptime != timestamp) {
				if (timestamp != 0) {
					unsigned char buf[16];
					WBUFW(buf,0) = 0x2731;
					WBUFL(buf,2) = acc;
					WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
					WBUFL(buf,7) = timestamp; // status or final date of a banishment
					charif_sendallwos(-1, buf, 11);
				}
				#ifdef DEBUG
				printf("Account: [%d] Banned until: [%ld]\n", acc, timestamp); 
				#endif
				
                sprintf(tmpsql, "UPDATE `%s` SET `ban_until` = '%ld', `state`='7' WHERE `%s` = '%d'", login_db, timestamp, login_db_account_id, acc);
				sql_query(tmpsql,"ban_request");
			}
		}
    }
    
	RFIFOSKIP(fd,18);
}

//-----------------------------
// Change sex [Edit: Wizputer]
//-----------------------------
void change_sex(int fd) {
	int sex,acc=RFIFOL(fd,4);
	unsigned char buf[16];
	
	sprintf(tmpsql,"SELECT `sex` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);
    sql_query(tmpsql,"change_sex");
        
    if ((sql_res = mysql_store_result(&mysql_handle)) && (sql_row = mysql_fetch_row(sql_res))) {
        if (strcmpi(sql_row[0], "M") == 0)
            sex = 1;
        else
            sex = 0;
        
        sprintf(tmpsql,"UPDATE `%s` SET `sex` = '%c' WHERE `%s` = '%d'", login_db, (sex==0?'M':'F'), login_db_account_id, acc);
		sql_query(tmpsql,"change_sex");
		
		WBUFW(buf,0) = 0x2723;
		WBUFL(buf,2) = acc;
		WBUFB(buf,6) = sex;
		charif_sendallwos(-1, buf, 7);
    }	
	
    RFIFOSKIP(fd,6);
}

//-------------------------------
// Save Account Reg [Edit: Wizputer]
//-------------------------------
void save_account_reg(int fd){
	int p,j,value,acc=RFIFOL(fd,4);
	char str[32];
	char temp_str[32];

	if (acc>0){
		unsigned char buf[RFIFOW(fd,2)+1];
		for(p=8,j=0;p<RFIFOW(fd,2) && j<ACCOUNT_REG2_NUM;p+=36,j++){
			memcpy(str,RFIFOP(fd,p),32);
			value=RFIFOL(fd,p+32);
			
            sprintf(tmpsql,"REPLACE INTO `global_reg_value` (`type`, `account_id`, `str`, `value`) VALUES ( 1 , '%d' , '%s' , '%d');",  acc, jstrescapecpy(temp_str,str), value);
			sql_query(tmpsql,"save_account_reg");
		}

		// Send to char
		memcpy(WBUFP(buf,0),RFIFOP(fd,0),RFIFOW(fd,2));
		WBUFW(buf,0)=0x2729;
		charif_sendallwos(fd,buf,WBUFW(buf,2));
	}

	RFIFOSKIP(fd,RFIFOW(fd,2));
	
	#ifdef DEBUG
	printf("login: save account_reg (from char)\n");
	#endif
}

//------------------------------------------------
// Recieve unban request from map-server (by Yor) [Edit: Wizputer]
//------------------------------------------------
void unban_request(int fd) {
	int acc = RFIFOL(fd,2);
				
    sprintf(tmpsql,"UPDATE `%s` SET `ban_until` = '0', `state`='0' WHERE `%s` = '%d' AND `state`='6'", login_db,login_db_account_id,acc);
    sql_query(tmpsql,"unban_request");

	RFIFOSKIP(fd,6);
}

//-----------------------------------------------------
// char-server packet parse [Edit: Wizputer]
//-----------------------------------------------------
int parse_fromchar(int fd){
	int id;

	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	char ip[16];

	sprintf(ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

	for(id = 0; id < MAX_SERVERS && id < servers_connected; id++)
		if (server_fd[id] == fd)
			break;

	if (id == MAX_SERVERS)
		session[fd]->eof = 1;
		
	if(session[fd]->eof) {
		if (id < MAX_SERVERS) {
			printf("Char-server '%s' has disconnected.\n", server[id].name);
			server_fd[id] = -1;
			memset(&server[id], 0, sizeof(struct mmo_char_server));
			servers_connected--;
			// server delete
			sprintf(tmpsql, "DELETE FROM `sstatus` WHERE `index`='%d'", id);
    		sql_query(tmpsql,"parse_fromchar");
		}
		close(fd);
		delete_session(fd);
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
	    #ifdef DEBUG_PACKETS
		printf("char_parse: %d %d packet case=%x\n", fd, RFIFOREST(fd), RFIFOW(fd, 0));
		#endif
		
		switch (RFIFOW(fd,0)) {
		case 0x2712:
			if (RFIFOREST(fd) < 19)
				return 0;
			send_account_reg(fd);
			break;

		case 0x2714:
			if (RFIFOREST(fd) < 6)
				return 0;
			number_world_users(fd,id);
			break;

		case 0x2716:
			if (RFIFOREST(fd) < 6)
				return 0;
			email_time_request(fd, id);
			break;
	
		case 0x2722:
			if (RFIFOREST(fd) < 86)
				return 0;
			change_account_email(fd, id, ip);
			break;

		case 0x2724:
			if (RFIFOREST(fd) < 10)
				return 0;
			status_change_request(fd);
			break;

		case 0x2725:
			if (RFIFOREST(fd) < 18)
				return 0;
			ban_request(fd);
			break;

		case 0x2727:
			if (RFIFOREST(fd) < 6)
				return 0;
			change_sex(fd);
			break;

		case 0x2728:
            if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
                return 0;
            save_account_reg(fd);	
            break;

        case 0x272a:
			if (RFIFOREST(fd) < 6)
				return 0;
			unban_request(fd);
			return 0;
			
        case 0x272b:    // Set account_id to online [Wizputer]
            if (RFIFOREST(fd) < 6)
                return 0;
            add_online_user(RFIFOL(fd,2));
            RFIFOSKIP(fd,6);
            break;
    
        case 0x272c:   // Set account_id to offline [Wizputer]
            if (RFIFOREST(fd) < 6)
                return 0;
            remove_online_user(RFIFOL(fd,2));
            RFIFOSKIP(fd,6);
            break;
			
		default:
		    #ifdef DEBUG
		    printf("login: unknown packet %x! (from char).\n", RFIFOW(fd,0));
		    #endif
		    session[fd]->eof = 1;
        }
	}

	return 0;
}
