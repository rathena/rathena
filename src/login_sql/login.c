// $Id: login.c,v 1.6 2004/09/19 21:12:07 Valaris Exp $
// original : login2.c 2003/01/28 02:29:17 Rev.1.1.1.1
// txt version 1.100

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include "timer.h"

#include "login.h"
#include "login_int.h"
#include "char_int.h"

#ifdef PASSWORDENC
#include "md5calc.h"
#endif

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define J_MAX_MALLOC_SIZE 65535
// Login Listening Port
int login_port = 6900;

struct auth_fifo auth_fifo[AUTH_FIFO_SIZE];

int auth_fifo_pos;

// MySQL Query
char tmpsql[65535], prev_query[65535];

// MySQL Connection Handle
MYSQL mysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;

// It's to check IP of a player between login-server and char-server (part of anti-hacking system)
int check_ip_flag; 

// Login's FD
int login_fd;

// LAN IP of char-server and subnet mask(Kashy)
char lan_char_ip[16];
int subnetmaski[4];

// Char-server data
struct mmo_char_server server[MAX_SERVERS];
int server_fd[MAX_SERVERS];
unsigned char servers_connected = 0;

// Anti-freeze Data
int server_freezeflag[MAX_SERVERS];
int anti_freeze_enable = 0;
int ANTI_FREEZE_INTERVAL = 15;

// MD5 Key Data for encrypted login
char md5key[20];
int md5keylen = 16;

// Auth FIFO Position
int auth_fifo_pos = 0;

//Added for Mugendai's I'm Alive mod
int imalive_on=0;
int imalive_time=60;

//Added by Mugendai for GUI
int flush_on=1;
int flush_time=100;

// Date format for bans
char date_format[32] = "%Y-%m-%d %H:%M:%S";

// minimum level of player/GM (0: player, 1-99: gm) to connect on the server
int min_level_to_connect = 0; 

// It's to check IP of a player between login-server and char-server (part of anti-hacking system)
int check_ip_flag = 1; 

// Dynamic IP Ban config
int ipban = 1;
int dynamic_account_ban = 1;
int dynamic_account_ban_class = 0;
int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 60;

// MySQL Config
int login_server_port = 3306;
char login_server_ip[16] = "127.0.0.1";
char login_server_id[16] = "ragnarok";
char login_server_pw[16] = "ragnarok";
char login_server_db[16] = "ragnarok";
int use_md5_passwds = 0;

// MySQL custom table and column names 
char login_db[32] = "login";
char loginlog_db[32] = "loginlog";
char login_db_account_id[32] = "account_id";
char login_db_userid[32] = "userid";
char login_db_user_pass[32] = "user_pass";
char login_db_level[32] = "level";

// Console interface on/off
int console = 0;

// Online User DB
struct dbt *online_db;

// GM Database
struct dbt *gm_db;

//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------

void add_online_user(int account_id) {
    int *p;
    p = malloc(sizeof(int));
    if (p == NULL) {
		printf("add_online_user: memory allocation failure (malloc)!\n");
		exit(0);
	}
	p = &account_id;
    numdb_insert(online_db, account_id, p);
}

int is_user_online(int account_id) {
    int *p;

	p = numdb_search(online_db, account_id);
	if (p == NULL)
		return 0;
	
	#ifdef DEBUG
	printf("Acccount [%d] Online\n",*p);
	#endif
	return 1;
}

void remove_online_user(int account_id) {
    int *p;
    p = numdb_erase(online_db,account_id);
    free(p);
}

//----------------------------------------------
//SQL Commands ( Original by Clownisius ) [Edit: Wizputer]
//----------------------------------------------

void sql_query(char* query,char function[32]) {
	if(mysql_query(&mysql_handle, query)){
		printf("---------- SQL error report ----------\n");
		printf("MySQL Server Error: %s\n", mysql_error(&mysql_handle));
		printf("Query: %s\n", query);
		printf("In function: %s \n", function);
		printf("\nPrevious query: %s\n", prev_query);
//		if (strcmp(mysql_error(&mysql_handle),"CR_COMMANDS_OUT_OF_SYNC") !=0) printf(" - =  Shutting down Char Server  = - \n\n");
		printf("-------- End SQL Error Report --------\n");
//		printf("Uncontrolled param: %s",&mysql_handle);
//		if (strcmp(mysql_error(&mysql_handle),"CR_COMMANDS_OUT_OF_SYNC") !=0) exit(1);
	}
	
	strcpy(prev_query,query);

}

//-----------------------------------------------------
// check user level [Wizputer]
//-----------------------------------------------------

unsigned char isGM(int account_id) {
    unsigned char *level;
    
   	level = numdb_search(gm_db, account_id);
	if (level == NULL)
		return 0;

	return *level;
}

static int gmdb_final(void *key,void *data,va_list ap) {
	unsigned char *level;

	nullpo_retr(0, level=data);

	free(level);

	return 0;
}

void do_final_gmdb(void) {
	if(gm_db){
		numdb_final(gm_db,gmdb_final);
		gm_db=NULL;
	}
}

void read_GMs(int fd) {
    unsigned char *level;
    int i=0;
        
    if(gm_db)
        do_final_gmdb();
        
    gm_db = numdb_init();
    
    sprintf(tmpsql,"SELECT `%s`,`%s` FROM `%s` WHERE `%s` > '%d'", login_db_account_id, login_db_level, login_db,login_db_level,lowest_gm_level);
    sql_query(tmpsql,"read_GMs");
    
    WFIFOW(fd, 0) = 0x2732;
    
    if ((sql_res = mysql_store_result(&mysql_handle))) {
        for(i=0;(sql_row = mysql_fetch_row(sql_res));i++) {
            level = malloc(sizeof(unsigned char));
            
            if( (*level = atoi(sql_row[1])) > 99 )
                *level = 99;

            numdb_insert(gm_db, atoi(sql_row[0]), level);
            
            WFIFOL(fd,6+5*i) = atoi(sql_row[0]);
            WFIFOB(fd,10+5*i) = *level;
        }
        
        WFIFOW(fd,2) = i;
    }
    
    WFIFOSET(fd,6+5*i);
    
    mysql_free_result(sql_res);
}    
    

//---------------------------------------------------
// E-mail check: return 0 (not correct) or 1 (valid).
//---------------------------------------------------
int e_mail_check(unsigned char *email) {
	char ch;
	unsigned char* last_arobas;

	// athena limits
	if (strlen(email) < 3 || strlen(email) > 39)
		return 0;

	// part of RFC limits (official reference of e-mail description)
	if (strchr(email, '@') == NULL || email[strlen(email)-1] == '@')
		return 0;

	if (email[strlen(email)-1] == '.')
		return 0;

	last_arobas = strrchr(email, '@');

	if (strstr(last_arobas, "@.") != NULL ||
	    strstr(last_arobas, "..") != NULL)
		return 0;

	for(ch = 1; ch < 32; ch++) {
		if (strchr(last_arobas, ch) != NULL) {
			return 0;
			break;
		}
	}

	if (strchr(last_arobas, ' ') != NULL ||
	    strchr(last_arobas, ';') != NULL)
		return 0;

	// all correct
	return 1;
}

//-----------------------------------------------------
// Connect to MySQL
//-----------------------------------------------------
int mmo_auth_sqldb_init(void) {

	printf("Login-server starting...\n");

	// memory initialize
	#ifdef DEBUG
	printf("memory initialize....\n");
	#endif

	mysql_init(&mysql_handle);

	// DB connection start
	printf("Connecting to Login Database Server...\n");
	if (!mysql_real_connect(&mysql_handle, login_server_ip, login_server_id, login_server_pw,
	    login_server_db, login_server_port, (char *)NULL, 0)) {
		// pointer check
		printf("%s\n", mysql_error(&mysql_handle));
		exit(1);
	} else {
		printf("Connected to MySQL Server\n");
	}
	
	//delete all server status
	sprintf(tmpsql,"TRUNCATE TABLE `sstatus`");
	sql_query(tmpsql,"mmo_db_close");

	sprintf(tmpsql, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver', '100','login server started')", loginlog_db);
	sql_query(tmpsql,"mmo_auth_sqldb_init");

	return 0;
}

//-----------------------------------------------------
// Close MySQL and Close all sessions
//-----------------------------------------------------
void mmo_db_close(void) {
	int i, fd;

	//set log.
	sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')", loginlog_db);
	sql_query(tmpsql,"mmo_db_close");

	//delete all server status
	sprintf(tmpsql,"TRUNCATE TABLE `sstatus`");
	sql_query(tmpsql,"mmo_db_close");

	mysql_close(&mysql_handle);
	printf("MySQL Connection closed\n");

	for (i = 0; i < MAX_SERVERS; i++) {
		if ((fd = server_fd[i]) >= 0)
			delete_session(fd);
	}
	delete_session(login_fd);
}

//-----------------------------------------------------
// Auth account
//-----------------------------------------------------
int mmo_auth( struct mmo_account* account , int fd){
	struct timeval tv;
	char tmpstr[256];
	char t_uid[32], t_pass[32];
    char user_password[32];
	
	char ip[16];
	
	int encpasswdok = 0;
	int state;
	
	#ifdef PASSWORDENC
	char logbuf[1024], *p = logbuf;
    char md5str[64],md5bin[32];
	int j;
	#endif

	unsigned char *sin_addr = (unsigned char *)&session[fd]->client_addr.sin_addr;
	sprintf(ip, "%d.%d.%d.%d", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3]);
	
	#ifdef DEBUG
	printf ("Starting auth for [%s]...\n",ip);
	#endif

	// auth start : time seed
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%Y-%m-%d %H:%M:%S",localtime(&(tv.tv_sec)));
	sprintf(tmpstr+19, ".%03d", (int)tv.tv_usec/1000);

	jstrescapecpy(t_uid,account->userid);
	jstrescapecpy(t_pass, account->passwd);

	// make query
	sprintf(tmpsql, "SELECT `%s`,`%s`,`%s`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`,`%s`"
	                " FROM `%s` WHERE `%s`='%s'", login_db_account_id, login_db_userid, login_db_user_pass, login_db_level, login_db, login_db_userid, t_uid);
	//login {0-account_id/1-userid/2-user_pass/3-lastlogin/4-logincount/5-sex/6-connect_untl/7-last_ip/8-ban_until/9-state}

	sql_query(tmpsql,"mmo_auth");

	if ((sql_res = mysql_store_result(&mysql_handle))) {
		if(!(sql_row = mysql_fetch_row(sql_res))) {
		    #ifdef DEBUG
			printf ("Auth failed: No Account Time: [%s] Username: [%s] Password: [%s]\n", tmpstr, account->userid, account->passwd);
			#endif
			mysql_free_result(sql_res);
			return 0;
		}
	} else {
	    #ifdef DEBUG
		printf("mmo_auth DB result error\n");
		#endif
		return 0;
	}

	account->ban_until_time = atol(sql_row[8]);
	state = atoi(sql_row[9]);

	if (state) {
		switch(state) { // packet 0x006a value + 1
		case 1:   // 0 = Unregistered ID
		case 2:   // 1 = Incorrect Password
		case 3:   // 2 = This ID is expired
		case 4:   // 3 = Rejected from Server
		case 5:   // 4 = You have been blocked by the GM Team
		case 6:   // 5 = Your Game's EXE file is not the latest version
		case 8:   // 7 = Server is jammed due to over populated
		case 9:   // 8 = No MSG (actually, all states after 9 except 99 are No MSG, use only this)
		case 100: // 99 = This ID has been totally erased
		    #ifdef DEBUG
			printf("Auth Error #%d\n", state);
			#endif
			mysql_free_result(sql_res);
			return state;
			break;
		case 7:   // 6 = Your are Prohibited to log in until %s
		    strftime(tmpstr, 20, date_format, localtime(&account->ban_until_time));
		    tmpstr[19] = '\0';
		    if (account->ban_until_time > time(NULL)) { // always banned
		        mysql_free_result(sql_res);  
		        return 7;
            } else { // ban is finished
			    // reset the ban time
			    sprintf(tmpsql, "UPDATE `%s` SET `ban_until`='0',`state`='0' WHERE BINARY `%s`='%s'", login_db, login_db_userid, t_uid);
			    sql_query(tmpsql,"mmo_auth");
		    }
		    break;
		default:
			return 100; // 99 = ID has been totally erased
			break;
		}
	}
	
	if (atol(sql_row[6]) != 0 && atol(sql_row[6]) < time(NULL)) {
		return 2; // 2 = This ID is expired
	}

    if ( is_user_online(atol(sql_row[0])) ) {
        printf("User [%s] is already online - Rejected.\n",sql_row[1]);
	    return 3; // Rejected
    }

	if (use_md5_passwds) {
		MD5_String(account->passwd,user_password);
	} else {
		jstrescapecpy(user_password, account->passwd);
	}

	#ifdef DEBUG
	printf("Account [ok] Pass Encode Value: [%d]\n",account->passwdenc);
	#endif

#ifdef PASSWORDENC
	if (account->passwdenc > 0) {
		j = account->passwdenc;


		#ifdef DEBUG
		printf ("Starting md5calc..\n");
		#endif
		
		if (j > 2)
			j = 1;
		
		do {
			if (j == 1) {
				sprintf(md5str, "%s%s", md5key,sql_row[2]);
			} else if (j == 2) {
				sprintf(md5str, "%s%s", sql_row[2], md5key);
			} else
				md5str[0] = 0;
			#ifdef DEBUG	
			printf("j: [%d] mdstr: [%s]\n", j, md5str);
			#endif
			
			MD5_String2binary(md5str, md5bin);
			encpasswdok = (memcmp(user_password, md5bin, 16) == 0);
		} while (j < 2 && !encpasswdok && (j++) != account->passwdenc);
		
		#ifdef DEBUG
        printf("key [%s] md5 [%s] ", md5key, md5str);
		printf("client [%s] accountpass [%s]\n", user_password, sql_row[2]);
		printf ("end md5calc..\n");
		#endif
	}
#endif
	if ((strcmp(user_password, sql_row[2]) && !encpasswdok)) {
		if (account->passwdenc == 0) {
		    #ifdef DEBUG
			printf ("auth failed pass error %s %s %s" RETCODE, tmpstr, account->userid, user_password);
			#endif
#ifdef PASSWORDENC
		} else {			
			p += sprintf(p, "auth failed pass error %s %s recv-md5[", tmpstr, account->userid);
			
            for(j = 0; j < 16; j++)
				p += sprintf(p, "%02x", ((unsigned char *)user_password)[j]);
			
			p += sprintf(p, "] calc-md5[");
			
            for(j = 0; j < 16; j++)
				p += sprintf(p, "%02x", ((unsigned char *)md5bin)[j]);
			
			p += sprintf(p, "] md5key[");
			
            for(j = 0; j < md5keylen; j++)
				p += sprintf(p, "%02x", ((unsigned char *)md5key)[j]);
			
			p += sprintf(p, "]" RETCODE);
			
			#ifdef DEBUG
			printf("%s\n", p);
			#endif
#endif
		}
		return 2;
	}

	#ifdef DEBUG
	printf("Auth ok: Time: [%s] Username: [%s]\n" RETCODE, tmpstr, account->userid);
	#endif

	account->account_id = atoi(sql_row[0]);
	account->login_id1 = rand();
	account->login_id2 = rand();
	memcpy(tmpstr, sql_row[3], 19);
	memcpy(account->lastlogin, tmpstr, 24);
	account->sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';

	sprintf(tmpsql, "UPDATE `%s` SET `lastlogin` = NOW(), `logincount`=`logincount` +1, `last_ip`='%s'  WHERE BINARY  `%s` = '%s'",
	        login_db, ip, login_db_userid, sql_row[1]);
	sql_query(tmpsql,"mmo_auth");
  
	mysql_free_result(sql_res) ; //resource free

	return -1;
}

//-----------------------------------------
// Lan ip check ( added by Kashy )
//-----------------------------------------
int lan_ip_check(unsigned char *p) {
	int y;
	int lancheck = 1;
	int lancharip[4];

	unsigned int k0, k1, k2, k3;
	
	sscanf(lan_char_ip, "%d.%d.%d.%d", &k0, &k1, &k2, &k3);
	lancharip[0] = k0; lancharip[1] = k1; lancharip[2] = k2; lancharip[3] = k3;

	for(y = 0; y < 4; y++) {
		if ((lancharip[y] & subnetmaski[y])!= (p[y]))
		lancheck = 0;
		break; }

	#ifdef DEBUG
	printf("LAN check: %s.\n", (lancheck) ? "\033[1;32mLAN\033[0m" : "\033[1;31mWAN\033[0m");
	#endif
	
	return lancheck;
}


//-----------------------------------------------------
// BANNED IP CHECK.
//-----------------------------------------------------
int ip_ban_check(int tid, unsigned int tick, int id, int data){

	//query
	if(mysql_query(&mysql_handle, "DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()")) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
	}

	return 0;
}

//------------------------------------
// Console Command Parser [Wizputer]
//------------------------------------
int parse_console(char *buf) {
    char *type,*command;
    
    type = (char *)malloc(64);
    command = (char *)malloc(64);
    
    memset(type,0,64);
    memset(command,0,64);
    
    printf("Console: %s\n",buf);
    
    if ( sscanf(buf, "%[^:]:%[^\n]", type , command ) < 2 )
        sscanf(buf,"%[^\n]",type);
    
    printf("Type of command: %s || Command: %s \n",type,command);
    
    if(buf) free(buf);
    if(type) free(type);
    if(command) free(command);
    
    return 0;
}

//-------------------------------------------------
// Return numerical value of a switch configuration
// on/off, english, français, deutsch, español
//-------------------------------------------------
int config_switch(const char *str) {
	if (strcmpi(str, "on") == 0 || strcmpi(str, "yes") == 0 || strcmpi(str, "oui") == 0 || strcmpi(str, "ja") == 0 || strcmpi(str, "si") == 0)
		return 1;
	if (strcmpi(str, "off") == 0 || strcmpi(str, "no") == 0 || strcmpi(str, "non") == 0 || strcmpi(str, "nein") == 0)
		return 0;

	return atoi(str);
}


//-------------------------------
// LAN Support Config (Kashy)
//-------------------------------
int login_lan_config_read(const char *lancfgName){
	int i;
	char subnetmask[128];
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp=fopen(lancfgName, "r");

	if (fp == NULL) {
		printf("file not found: %s\n", lancfgName);
		return 1;
	}
	printf("Start reading of Lan Support configuration file\n");
	while(fgets(line, sizeof(line)-1, fp)){
		if (line[0] == '/' && line[1] == '/')
			continue;

		i = sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		else if(strcmpi(w1,"lan_char_ip")==0){
			strcpy(lan_char_ip, w2);
			printf ("set Lan_Char_IP : %s\n",w2);
			}

		else if(strcmpi(w1,"subnetmask")==0){
			unsigned int k0, k1, k2, k3;

			strcpy(subnetmask, w2);
			sscanf(subnetmask, "%d.%d.%d.%d", &k0, &k1, &k2, &k3);
			subnetmaski[0] = k0; subnetmaski[1] = k1; subnetmaski[2] = k2; subnetmaski[3] = k3;
			printf ("set subnetmask : %s\n",w2);
			}
		}
	fclose(fp);

	{
		unsigned int a0, a1, a2, a3;
		unsigned char p[4];
		sscanf(lan_char_ip, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
		p[0] = a0; p[1] = a1; p[2] = a2; p[3] = a3;
		printf("LAN test of LAN IP of the char-server: ");
		if (lan_ip_check(p) == 0) {
			printf("\033[1;31m***ERROR: LAN IP of the char-server doesn't belong to the specified Sub-network\033[0m\n");
		}
	}

	printf("End reading of Lan Support configuration file\n");

	return 0;
}

//-----------------------------------------------------
// Login configuration
//-----------------------------------------------------
int login_config_read(const char *cfgName){
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");

	if(fp==NULL){
		printf("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	printf("Start reading login configuration: %s\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;

		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		else if(strcmpi(w1,"login_port")==0){
			login_port=atoi(w2);
			printf ("set login_port : %s\n",w2);
		}
		else if(strcmpi(w1,"ipban")==0){
			ipban=atoi(w2);
			printf ("set ipban : %d\n",ipban);
		}
		//account ban -> ip ban
		else if(strcmpi(w1,"dynamic_account_ban")==0){
			dynamic_account_ban=atoi(w2);
			printf ("set dynamic_account_ban : %d\n",dynamic_account_ban);
		}
		else if(strcmpi(w1,"dynamic_account_ban_class")==0){
			dynamic_account_ban_class=atoi(w2);
			printf ("set dynamic_account_ban_class : %d\n",dynamic_account_ban_class);
		}
		//dynamic password error ban
		else if(strcmpi(w1,"dynamic_pass_failure_ban")==0){
			dynamic_pass_failure_ban=atoi(w2);
			printf ("set dynamic_pass_failure_ban : %d\n",dynamic_pass_failure_ban);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_time")==0){
			dynamic_pass_failure_ban_time=atoi(w2);
			printf ("set dynamic_pass_failure_ban_time : %d\n",dynamic_pass_failure_ban_time);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_how_many")==0){
			dynamic_pass_failure_ban_how_many=atoi(w2);
			printf ("set dynamic_pass_failure_ban_how_many : %d\n",dynamic_pass_failure_ban_how_many);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_how_long")==0){
			dynamic_pass_failure_ban_how_long=atoi(w2);
			printf ("set dynamic_pass_failure_ban_how_long : %d\n",dynamic_pass_failure_ban_how_long);
		} 
		else if(strcmpi(w1,"anti_freeze_enable")==0){
			anti_freeze_enable = config_switch(w2);
		} 
		else if (strcmpi(w1, "anti_freeze_interval") == 0) {
			ANTI_FREEZE_INTERVAL = atoi(w2);
			if (ANTI_FREEZE_INTERVAL < 5)
				ANTI_FREEZE_INTERVAL = 5; // minimum 5 seconds
		} 
		else if (strcmpi(w1, "import") == 0) {
			login_config_read(w2);
		} else if(strcmpi(w1,"imalive_on")==0) {		//Added by Mugendai for I'm Alive mod
			imalive_on = atoi(w2);			//Added by Mugendai for I'm Alive mod
		} else if(strcmpi(w1,"imalive_time")==0) {	//Added by Mugendai for I'm Alive mod
			imalive_time = atoi(w2);		//Added by Mugendai for I'm Alive mod
		} else if(strcmpi(w1,"flush_on")==0) {		//Added by Mugendai for GUI
			flush_on = atoi(w2);			//Added by Mugendai for GUI
		} else if(strcmpi(w1,"flush_time")==0) {	//Added by Mugendai for GUI
			flush_time = atoi(w2);			//Added by Mugendai for GUI
		}
		else if(strcmpi(w1,"use_MD5_passwords")==0){
			if (!strcmpi(w2,"yes")) {
				use_md5_passwds=1;
			} else if (!strcmpi(w2,"no")){
				use_md5_passwds=0;
			}
			printf ("Using MD5 Passwords: %s \n",w2);
		}
        else if (strcmpi(w1, "date_format") == 0) { // note: never have more than 19 char for the date!
				switch (atoi(w2)) {
				case 0:
					strcpy(date_format, "%d-%m-%Y %H:%M:%S"); // 31-12-2004 23:59:59
					break;
				case 1:
					strcpy(date_format, "%m-%d-%Y %H:%M:%S"); // 12-31-2004 23:59:59
					break;
				case 2:
					strcpy(date_format, "%Y-%d-%m %H:%M:%S"); // 2004-31-12 23:59:59
					break;
				case 3:
					strcpy(date_format, "%Y-%m-%d %H:%M:%S"); // 2004-12-31 23:59:59
					break;
				}
		}
        else if (strcmpi(w1, "min_level_to_connect") == 0) {
				min_level_to_connect = atoi(w2);
		}
        else if (strcmpi(w1, "check_ip_flag") == 0) {
                check_ip_flag = config_switch(w2);
    	}
    	else if (strcmpi(w1, "console") == 0) {
			    if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
			        console = 1;
        }
 	}
	fclose(fp);
	printf ("End reading login configuration...\n");
	return 0;
}

//-----------------------------------------------------
// SQL configuration
//-----------------------------------------------------
void sql_config_read(const char *cfgName){ /* Kalaspuff, to get login_db */
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp=fopen(cfgName,"r");
	if(fp==NULL){
		printf("file not found: %s\n",cfgName);
		exit(1);
	}
	printf("Start reading SQL configuration: %s\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if (strcmpi(w1, "login_db") == 0) {
			strcpy(login_db, w2);
		}
		//add for DB connection
		else if(strcmpi(w1,"login_server_ip")==0){
			strcpy(login_server_ip, w2);
			printf ("set login_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_port")==0){
			login_server_port=atoi(w2);
			printf ("set login_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_id")==0){
			strcpy(login_server_id, w2);
			printf ("set login_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_pw")==0){
			strcpy(login_server_pw, w2);
			printf ("set login_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_db")==0){
			strcpy(login_server_db, w2);
			printf ("set login_server_db : %s\n",w2);
		}
		//added for custom column names for custom login table
		else if(strcmpi(w1,"login_db_account_id")==0){
			strcpy(login_db_account_id, w2);
		}
		else if(strcmpi(w1,"login_db_userid")==0){
			strcpy(login_db_userid, w2);
		}
		else if(strcmpi(w1,"login_db_user_pass")==0){
			strcpy(login_db_user_pass, w2);
		}
		else if(strcmpi(w1,"login_db_level")==0){
			strcpy(login_db_level, w2);
		}
		//end of custom table config
		else if (strcmpi(w1, "loginlog_db") == 0) {
			strcpy(loginlog_db, w2);
		}
    }
    fclose(fp);
    printf("reading SQL configuration done.....\n");
}


//-----------------------------------------------------
//I'm Alive Alert
//Used to output 'I'm Alive' every few seconds
//Intended to let frontends know if the app froze
//-----------------------------------------------------
int imalive_timer(int tid, unsigned int tick, int id, int data){
	printf("I'm Alive\n");
	return 0;
}

//-----------------------------------------------------
//Flush stdout
//stdout buffer needs flushed to be seen in GUI
//-----------------------------------------------------
int flush_timer(int tid, unsigned int tick, int id, int data){
	fflush(stdout);
	return 0;
}

int do_init(int argc,char **argv){
	int i;

	//read login configuration
	login_config_read( (argc>1)?argv[1]:LOGIN_CONF_NAME );
	
	//read SQL configuration
	sql_config_read(SQL_CONF_NAME);
	
	//read LAN support configuation
	login_lan_config_read((argc > 1) ? argv[1] : LAN_CONF_NAME);
	
	//Generate Passworded Key.
	#ifdef DEBUG
	printf ("memset value: [0] var: [md5key] \n");
	#endif
	
	memset(md5key, 0, sizeof(md5key));
	
	#ifdef DEBUG
	printf ("memset var: [md5key] complete\n");
	printf ("Set MD5 key length\n");
	#endif

	md5keylen=rand()%4+12;
	for(i=0;i<md5keylen;i++)
		md5key[i]=rand()%255+1;
		
	#ifdef DEBUG
	printf ("Set MD5 key length complete\n");
	printf ("Set Auth FIFO Size\n");
	#endif
	
	for(i=0;i<AUTH_FIFO_SIZE;i++)
		auth_fifo[i].delflag=1;
	
	#ifdef DEBUG
	printf ("Set Auth FIFO Size complete\n");
	printf ("Set max number servers\n");
	#endif
	
	for(i=0;i<MAX_SERVERS;i++)
		server_fd[i]=-1;
	
	#ifdef DEBUG
	printf ("Set max number servers complete\n");
	#endif

	//server port open & binding
	login_fd=make_listen_port(login_port);


	printf ("Initializing SQL DB\n");
	mmo_auth_sqldb_init();
	printf ("SQL DB Initialized\n");

	// Close connection to SQL DB at termiantion
	set_termfunc(mmo_db_close);

	//set default parser as parse_login function
	set_defaultparse(parse_login);

	//Added for Mugendais I'm Alive mod
	if(imalive_on)
		add_timer_interval(gettick()+10, imalive_timer,0,0,imalive_time*1000);

	//Added by Mugendai for GUI support
	if(flush_on)
		add_timer_interval(gettick()+10, flush_timer,0,0,flush_time);

	if(anti_freeze_enable > 0) {
		add_timer_func_list(char_anti_freeze_system, "char_anti_freeze_system");
		i = add_timer_interval(gettick()+1000, char_anti_freeze_system, 0, 0, ANTI_FREEZE_INTERVAL * 1000);
	}

	// ban deleter timer - 1 minute term
	#ifdef DEBUG
	printf("add interval tic (ip_ban_check)...\n");
	#endif
	i=add_timer_interval(gettick()+10, ip_ban_check,0,0,60*1000);

	if (console) {
		set_defaultconsoleparse(parse_console);
		start_console();
	}
	
	// Online user database init
    free(online_db);
	online_db = numdb_init();
	
	// Read GMs from table
	read_GMs();
	
	printf("The login-server is \033[1;32mready\033[0m (Server is listening on the port %d).\n\n", login_port);

	return 0;
}
