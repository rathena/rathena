#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include "login.h"
#include "char_int.h"

//----------------------
// Client requesting login [Edit: Wizputer]
//----------------------
void client_request_login(int fd,unsigned char *p ) {
    struct mmo_account account;
   	char t_uid[32];  
   	int server_num = 0,result,i;
    
    #ifdef DEBUG
    printf("client connection request %s from %d.%d.%d.%d\n", RFIFOP(fd, 6), p[0], p[1], p[2], p[3]);
    #endif

	account.userid = RFIFOP(fd, 6);
	account.passwd = RFIFOP(fd, 30);
#ifdef PASSWORDENC
	account.passwdenc= (RFIFOW(fd,0)==0x64)?0:PASSWORDENC;
#else
	account.passwdenc=0;
#endif
	result=mmo_auth(&account, fd);

	jstrescapecpy(t_uid,RFIFOP(fd, 6));
	if(result==-1){
	    unsigned char gm_level = isGM(account.account_id);
        if (min_level_to_connect > gm_level || !servers_connected) {
			WFIFOW(fd,0) = 0x81;
			WFIFOL(fd,2) = 1; // 01 = Server closed
			WFIFOSET(fd,3);
	    } else {
            if (p[0] != 127) {
                sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s','100', 'login ok')", loginlog_db, p[0], p[1], p[2], p[3], t_uid);
                sql_query(tmpsql,"client_request_login");
            }

            if (gm_level) {
                #ifdef DEBUG            
			    printf("Connection of the GM (level:%d) account '%s' accepted.\n", gm_level, account.userid);
			    #endif
			} else {
			    #ifdef DEBUG
				printf("Connection of the account '%s' accepted.\n", account.userid);
				#endif
			}
   			
		    for(i = 0; i < MAX_SERVERS || server_num < servers_connected ; i++)
		        if (server_fd[i] >= 0) {
		            //Lan check added by Kashy
		            if (lan_ip_check(p))
					    WFIFOL(fd,47+server_num*32) = inet_addr(lan_char_ip);
	                else
                        WFIFOL(fd,47+server_num*32) = server[i].ip;

                    WFIFOW(fd,47+server_num*32+4) = server[i].port;
                    memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20);
                    WFIFOW(fd,47+server_num*32+26) = server[i].users;
                    WFIFOW(fd,47+server_num*32+28) = server[i].maintenance;
                    WFIFOW(fd,47+server_num*32+30) = server[i].new;
                    server_num++;
                }   
            
                WFIFOW(fd,0)=0x69;
                WFIFOW(fd,2)=47+32*server_num;
                WFIFOL(fd,4)=account.login_id1;
                WFIFOL(fd,8)=account.account_id;
                WFIFOL(fd,12)=account.login_id2;
                WFIFOL(fd,16)=0;
                memcpy(WFIFOP(fd,20),account.lastlogin,24);
                WFIFOB(fd,46)=account.sex;
                WFIFOSET(fd,47+32*server_num);
                
                if(auth_fifo_pos>=AUTH_FIFO_SIZE)
                    auth_fifo_pos=0;
                
                auth_fifo[auth_fifo_pos].account_id=account.account_id;
                auth_fifo[auth_fifo_pos].login_id1=account.login_id1;
                auth_fifo[auth_fifo_pos].login_id2=account.login_id2;
                auth_fifo[auth_fifo_pos].sex=account.sex;
                auth_fifo[auth_fifo_pos].delflag=0;
                auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr.sin_addr.s_addr;
                auth_fifo_pos++;
        }
    } else {
		char error[32];
		
		sprintf(tmpsql,"SELECT `error` FROM `errors` WHERE `result`='%d'",result);
		sql_query(tmpsql,"client_request_login");
		
		if ((sql_res = mysql_store_result(&mysql_handle))) {
                if ((sql_row = mysql_fetch_row(sql_res))) {
                    sprintf(error,sql_row[0]);
                } else {
                    sprintf(error,"No Error!");
                }
        }        
		
        
        sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s', '%d','login failed : %s')", loginlog_db, p[0], p[1], p[2], p[3], t_uid, result, error);
        
        		
        sql_query(tmpsql,"client_request_login");

		if ((result == 1) && (dynamic_pass_failure_ban != 0)){	// failed password
			sprintf(tmpsql,"SELECT count(*) FROM `%s` WHERE `ip` = '%d.%d.%d.%d' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
			        loginlog_db, p[0], p[1], p[2], p[3], dynamic_pass_failure_ban_time);	//how many times filed account? in one ip.
			sql_query(tmpsql,"client_request_login");
			
			if ((sql_res = mysql_store_result(&mysql_handle))) {
                if ((sql_row = mysql_fetch_row(sql_res))) {
                    if (atoi(sql_row[0]) >= dynamic_pass_failure_ban_how_many ) {
                        sprintf(tmpsql,"INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%d.%d.%d.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban: %s')", p[0], p[1], p[2], dynamic_pass_failure_ban_how_long, t_uid);
                        sql_query(tmpsql,"client_request_login");
                    }
                }
            }        

			
			mysql_free_result(sql_res);
			
		}
		
		//cannot connect login failed
		memset(WFIFOP(fd,0),'\0',23);
		WFIFOW(fd,0)=0x6a;
		WFIFOB(fd,2)=result-1;
		if (result == 6) { // 6 = Your are Prohibited to log in until %s
			char tmpstr[256];
			strftime(tmpstr, 20, date_format, localtime(&account.ban_until_time));
			tmpstr[19] = '\0';
			memcpy(WFIFOP(fd,3), tmpstr, 20);
		} else { // we send error message
			memcpy(WFIFOP(fd,3), error, 20);
		}
	}

	WFIFOSET(fd,23);

	RFIFOSKIP(fd,(RFIFOW(fd,0)==0x64)?55:47);
}

//------------------------------------------------------
// MD5 Key requested for encypted login [Edit: Wizputer
//------------------------------------------------------
void md5_key_request(int fd) {
    #ifdef DEBUG
    printf("Request Password key -%s\n",md5key);
    #endif
    
	RFIFOSKIP(fd,2);
	WFIFOW(fd,0)=0x01dc;
	WFIFOW(fd,2)=4+md5keylen;
	memcpy(WFIFOP(fd,4),md5key,md5keylen);
	WFIFOSET(fd,WFIFOW(fd,2));
}

//----------------------------------------------------
// Char-server requesting connection [Edit: Wizputer]
//-----------------------------------------------------
void char_request_login(int fd, unsigned char *p) {
    struct mmo_account account;
	unsigned char* server_name;
   	char t_uid[32];  
	int result;
	
	sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s@%s','100', 'charserver - %s@%d.%d.%d.%d:%d')", loginlog_db, p[0], p[1], p[2], p[3], RFIFOP(fd, 2),RFIFOP(fd, 60),RFIFOP(fd, 60), RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57), RFIFOW(fd, 58));
	sql_query(tmpsql,"char_request_login");
	
	#ifdef DEBUG
	printf("server connection request %s @ %d.%d.%d.%d:%d (%d.%d.%d.%d)\n",
			RFIFOP(fd, 60), RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57), RFIFOW(fd, 58),
			p[0], p[1], p[2], p[3]);
	#endif

	account.userid = RFIFOP(fd, 2);
	account.passwd = RFIFOP(fd, 26);
	account.passwdenc = 0;
	server_name = RFIFOP(fd,60);
	result = mmo_auth(&account, fd);

	#ifdef DEBUG
	printf("Result: %d - Sex: %d - Account ID: %d\n",result,account.sex,(int) account.account_id);
	#endif

	if(result == -1 && account.sex==2 && account.account_id<MAX_SERVERS && server_fd[account.account_id]==-1){
	    printf("Connection of the char-server '%s' accepted.\n", server_name);

        memset(&server[account.account_id], 0, sizeof(struct mmo_char_server));
        
        server[account.account_id].ip=RFIFOL(fd,54);
		server[account.account_id].port=RFIFOW(fd,58);
		memcpy(server[account.account_id].name,RFIFOP(fd,60),20);
		server[account.account_id].users=0;
		server[account.account_id].maintenance=RFIFOW(fd,82);
		server[account.account_id].new=RFIFOW(fd,84);
		server_fd[account.account_id]=fd;

		if(anti_freeze_enable)
			server_freezeflag[account.account_id] = 5; // Char-server anti-freeze system. Counter. 5 ok, 4...0 freezed

		jstrescapecpy(t_uid,server[account.account_id].name);
		
		sprintf(tmpsql,"REPLACE DELAYED INTO `sstatus`(`index`,`name`,`user`) VALUES ( '%ld', '%s', '%d')",
				account.account_id, server[account.account_id].name,0);
		sql_query(tmpsql,"char_request_login");
		
        WFIFOW(fd,0)=0x2711;
		WFIFOB(fd,2)=0;
		WFIFOSET(fd,3);
		session[fd]->func_parse=parse_fromchar;
		realloc_fifo(fd,FIFOSIZE_SERVERLINK,FIFOSIZE_SERVERLINK);
		
		servers_connected++;
	} else {
		WFIFOW(fd, 0) =0x2711;
		WFIFOB(fd, 2)=3;
		WFIFOSET(fd, 3);
	}

	RFIFOSKIP(fd, 86);
}

//---------------------------------------------
// Athena Version Info Request [Edit: Wizputer]
//---------------------------------------------
void request_athena_info(int fd) {
    #ifdef DEBUG
	printf ("Athena version check...\n");
	#endif
	
	WFIFOW(fd,0)=0x7531;
	WFIFOB(fd,2)=ATHENA_MAJOR_VERSION;
	WFIFOB(fd,3)=ATHENA_MINOR_VERSION;
	WFIFOB(fd,4)=ATHENA_REVISION;
	WFIFOB(fd,5)=ATHENA_RELEASE_FLAG;
	WFIFOB(fd,6)=ATHENA_OFFICIAL_FLAG;
	WFIFOB(fd,7)=ATHENA_SERVER_LOGIN;
	WFIFOW(fd,8)=ATHENA_MOD_VERSION;
	WFIFOSET(fd,10);
	RFIFOSKIP(fd,2);
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd) {
	char ip[16];
 	
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	sprintf(ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

	if (ipban > 0) {
		//ip ban
		//p[0], p[1], p[2], p[3]
		//request DB connection
		//check
		sprintf(tmpsql, "SELECT count(*) FROM `ipbanlist` WHERE `list` = '%d.*.*.*' OR `list` = '%d.%d.*.*' OR `list` = '%d.%d.%d.*' OR `list` = '%d.%d.%d.%d'",
		  p[0], p[0], p[1], p[0], p[1], p[2], p[0], p[1], p[2], p[3]);
		sql_query(tmpsql,"parse_login");

		if((sql_res = mysql_store_result(&mysql_handle))) {
            if((sql_row = mysql_fetch_row(sql_res))) {//row fetching
                if (atoi(sql_row[0]) >0) {
                    // ip ban ok.
                    printf ("packet from banned ip : %d.%d.%d.%d" RETCODE, p[0], p[1], p[2], p[3]);
                
                    sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', 'unknown','-3', 'ip banned')", loginlog_db, p[0], p[1], p[2], p[3]);
                    sql_query(tmpsql,"parse_login");
                
                    #ifdef DEBUG
                    printf ("close session connection...\n");
                    #endif

                    // close connection
                    session[fd]->eof = 1;
                } else {
                    #ifdef DEBUG
                    printf ("packet from ip (ban check ok) : %d.%d.%d.%d" RETCODE, p[0], p[1], p[2], p[3]);
                    #endif
                }
            }    
        }
        
        mysql_free_result(sql_res);
    }
    
	if (session[fd]->eof) {
	    int i;
		for(i = 0; i < MAX_SERVERS && i < servers_connected; i++)
			if (server_fd[i] == fd) {
				server_fd[i] = -1;
				servers_connected--;
			}			
		close(fd);
		delete_session(fd);
		return 0;
	}

	while(RFIFOREST(fd)>=2){
	    #ifdef DEBUG_PACKETS
		printf("parse_login : %d %d packet case=%x\n", fd, RFIFOREST(fd), RFIFOW(fd,0));
		#endif

		switch(RFIFOW(fd,0)){
		case 0x200:		// New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
			if (RFIFOREST(fd) < 26)
				return 0;
			RFIFOSKIP(fd,26);
			break;

		case 0x204:		// New alive packet: structure: 0x204 <encrypted.account.userid>.16B. (new ragexe from 22 june 2004)
			if (RFIFOREST(fd) < 18)
				return 0;
			RFIFOSKIP(fd,18);
			break;

		case 0x64:
		case 0x01dd:
			if(RFIFOREST(fd)< ((RFIFOW(fd, 0) ==0x64)?55:47))
				return 0;
			client_request_login(fd, p);	
			break;
		case 0x01db:
		    if (session[fd]->session_data) {
		          #ifdef DEBUG
		          printf("login: abnormal request of MD5 key (already opened session).\n");
		          #endif
		          session[fd]->eof = 1;
		          return 0;
            }
            md5_key_request(fd);
            break;

        case 0x2710:
			if(RFIFOREST(fd)<86)
				return 0;
			char_request_login(fd,p);
			break;

		case 0x7530:
		    request_athena_info(fd);
			break;

		case 0x7532:
		default:
		    #ifdef DEBUG
			printf ("End of connection (ip: %s)" RETCODE, ip);
			#endif
			session[fd]->eof = 1;
			return 0;
		}
	}

	return 0;
}

