// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cbasetypes.h"
#ifdef __WIN32
#include <my_global.h>
#include <my_sys.h>
#endif
#include <mysql.h>

#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/db.h"

struct auth_dat_ {
	int account_id, sex;
	char userid[24], pass[24], lastlogin[24];
	int logincount;
	int state; // packet 0x006a value + 1 (0: compte OK)
	char email[40]; // e-mail (by default: a@a.com)
	char error_message[20]; // Message of error code #6 = You are Prohibited to log in until %s (packet 0x006a)
	time_t ban_until_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	char last_ip[16]; // save of last IP of connection
	char memo[255]; // a memo field
	int account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];
} *auth_dat;

char login_account_id[256]="account_id";
char login_userid[256]="userid";
char login_user_pass[256]="user_pass";
char login_db[256]="login";

static struct dbt *gm_account_db;

int db_server_port = 3306;
char db_server_ip[32] = "127.0.0.1";
char db_server_id[32] = "ragnarok";
char db_server_pw[32] = "ragnarok";
char db_server_logindb[32] = "ragnarok";

#define INTER_CONF_NAME "conf/inter_athena.conf"

int isGM(int account_id)
{
	struct gm_account *p;
	p = idb_get(gm_account_db,account_id);
	if( p == NULL)
		return 0;
	return p->level;
}

int read_gm_account()
{
	char line[8192];
	struct gm_account *p;
	FILE *fp;
	int c=0;

	gm_account_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_BASE,sizeof(int));

	printf("Starting reading gm_account\n");

	if( (fp=fopen("conf/GM_account.txt","r"))==NULL )
		return 1;
	while(fgets(line,sizeof(line),fp))
	{
		if(line[0] == '/' || line[1] == '/' || line[2] == '/')
			continue;

		p = (struct gm_account*)malloc(sizeof(struct gm_account));
		if(p==NULL){
			printf("gm_account: out of memory!\n");
			exit(0);
		}

		if(sscanf(line,"%d %d",&p->account_id,&p->level) != 2 || p->level <= 0) {
			printf("gm_account: broken data [conf/GM_account.txt] line %d\n",c);
			continue;
		}
		else {
			if(p->level > 99)
				p->level = 99;
			idb_put(gm_account_db,p->account_id,p);
			c++;
			printf("GM ID: %d Level: %d\n",p->account_id,p->level);
		}
	}
	fclose(fp);
	printf("%d ID of gm_accounts read.\n",c);
	return 0;
}

int mmo_auth_init(void)
{
	MYSQL mysql_handle;
	char tmpsql[1024];
	MYSQL_RES* 	sql_res ;
	MYSQL_ROW	sql_row ;
	FILE *fp;
	int account_id, logincount, user_level, state, n, i;
	char line[2048], userid[2048], pass[2048], lastlogin[2048], sex, email[2048], error_message[2048], last_ip[2048], memo[2048];
	time_t ban_until_time;
	time_t connect_until_time;
	char t_uid[256];

	mysql_init(&mysql_handle);
	if(!mysql_real_connect(&mysql_handle, db_server_ip, db_server_id, db_server_pw,
		db_server_logindb ,db_server_port, (char *)NULL, 0)) {
			//pointer check
			printf("%s\n",mysql_error(&mysql_handle));
			exit(1);
	}
	else {
		printf ("Connect: Success!\n");
	}
	printf ("Convert start...\n");


	fp=fopen("save/account.txt","r");
	auth_dat = (struct auth_dat_*)malloc(sizeof(auth_dat[0])*256);
	if(fp==NULL)
		return 0;
	while(fgets(line,1023,fp)!=NULL)
	{
		if(line[0]=='/' && line[1]=='/')
			continue;

		i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t"
			"%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]\t%ld%n",
			&account_id, userid, pass, lastlogin, &sex, &logincount, &state,
			email, error_message, &connect_until_time, last_ip, memo, &ban_until_time, &n);

		sprintf(tmpsql, "SELECT `%s`,`%s`,`%s`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`"
			" FROM `%s` WHERE `%s`='%s'", login_account_id, login_userid, login_user_pass, login_db, login_userid, t_uid);

		if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		}
		user_level = isGM(account_id);
		printf ("userlevel: %s (%d)- %d\n",userid, account_id, user_level);
		sql_res = mysql_store_result(&mysql_handle) ;
		sql_row = mysql_fetch_row(sql_res);	//row fetching
		if (!sql_row) //no row -> insert
			sprintf(tmpsql, "INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level`) VALUES (%d, '%s', '%s', '%s', '%c', %d, 'user@athena', %d);",account_id , userid, pass,lastlogin,sex,logincount, user_level);
		else //row reside -> updating
			sprintf(tmpsql, "UPDATE `login` SET `account_id`='%d', `userid`='%s', `user_pass`='%s', `lastlogin`='%s', `sex`='%c', `logincount`='%d', `email`='user@athena', `level`='%d'\nWHERE `account_id`='%d';",account_id , userid, pass,lastlogin,sex,logincount, user_level, account_id);
		printf ("Query: %s\n",tmpsql);
		mysql_free_result(sql_res) ; //resource free
		if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		}
	}
	fclose(fp);

	printf ("Convert end...\n");

	return 0;
}

int login_config_read(const char *cfgName)
{
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	printf ("Start reading interserver configuration: %s\n",cfgName);

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		printf("File not found: %s\n", cfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;

		i=sscanf(line,"%[^:]:%s", w1, w2);
		if(i!=2)
			continue;

		//add for DB connection
		if(strcmpi(w1,"db_server_ip")==0){
			strcpy(db_server_ip, w2);
			printf ("set db_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_port")==0){
			db_server_port=atoi(w2);
			printf ("set db_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_id")==0){
			strcpy(db_server_id, w2);
			printf ("set db_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_pw")==0){
			strcpy(db_server_pw, w2);
			printf ("set db_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_logindb")==0){
			strcpy(db_server_logindb, w2);
			printf ("set db_server_logindb : %s\n",w2);
		}
		//support the import command, just like any other config
		else if(strcmpi(w1,"import")==0){
			login_config_read(w2);
		}
	}
	fclose(fp);
	printf ("End reading interserver configuration...\n");
	return 0;
}

int do_init(int argc,char **argv)
{
	char input;
	login_config_read( (argc>1)?argv[1]:INTER_CONF_NAME );
	read_gm_account();

	printf("\nWarning : Make sure you backup your databases before continuing!\n");
	printf("\nDo you wish to convert your Login Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y')
		mmo_auth_init();
	printf ("Everything's been converted!\n");
	exit (0);
}

void do_abort(void) {}

void do_final(void) {}
