// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/malloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char login_account_id[256]="account_id";
char login_userid[256]="userid";
char login_user_pass[256]="user_pass";
char login_db[256]="login";
char globalreg_db[256]="global_reg_value";

static DBMap* gm_account_db=NULL; // int account_id -> struct gm_account*

int db_server_port = 3306;
char db_server_ip[32] = "127.0.0.1";
char db_server_id[32] = "ragnarok";
char db_server_pw[32] = "ragnarok";
char db_server_logindb[32] = "ragnarok";

#define INTER_CONF_NAME "conf/inter_athena.conf"
#define GM_ACCOUNT_NAME "conf/GM_account.txt"
#define ACCOUNT_TXT_NAME "save/account.txt"
//--------------------------------------------------------

int isGM(int account_id)
{
	struct gm_account* p = (struct gm_account*)idb_get(gm_account_db,account_id);
	return( p != NULL ) ? p->level : 0;
}

int read_gm_account()
{
	char line[8192];
	struct gm_account *p;
	FILE *fp;
	int line_counter = 0, gm_counter = 0;
	
	ShowStatus("Starting reading gm_account\n");
	
	if( (fp = fopen(GM_ACCOUNT_NAME,"r")) == NULL )
		return 1;
	
	gm_account_db = idb_alloc(DB_OPT_RELEASE_DATA);
	
	while(fgets(line,sizeof(line),fp))
	{
		line_counter++;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		
		p = (struct gm_account*)aMalloc(sizeof(struct gm_account));
		if(p==NULL){
			ShowFatalError("gm_account: out of memory!\n");
			exit(EXIT_FAILURE);
		}
		
		if(sscanf(line,"%d %d",&p->account_id,&p->level) != 2 || p->level <= 0) {
			ShowWarning("gm_account: unsupported data format [conf/GM_account.txt] on line %d\n", line_counter);
			continue;
		}
		else {
			if(p->level > 99)
				p->level = 99;
			p = (struct gm_account*)idb_put(gm_account_db,p->account_id,p);
			if( p )
				aFree(p);// old entry replaced
			gm_counter++;
			ShowInfo("GM ID: %d Level: %d\n",p->account_id,p->level);
		}
	}

	fclose(fp);
	ShowStatus("%d ID of gm_accounts read.\n", gm_counter);
	return 0;
}

int convert_login(void)
{
	Sql* mysql_handle;
	SqlStmt* stmt;
	int line_counter = 0;
	FILE *fp;
	int account_id, logincount, user_level, state, n, i;
	char line[2048], userid[2048], pass[2048], lastlogin[2048], sex, email[2048], error_message[2048], last_ip[2048], memo[2048];
	int ban_until_time, connect_until_time;
	char dummy[2048];

	mysql_handle = Sql_Malloc();
	if ( SQL_ERROR == Sql_Connect(mysql_handle, db_server_id, db_server_pw, db_server_ip, db_server_port, db_server_logindb) )
	{
		Sql_ShowDebug(mysql_handle);
		Sql_Free(mysql_handle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("Connect: Success!\n");
	
	ShowStatus("Convert start...\n");
	fp = fopen(ACCOUNT_TXT_NAME,"r");
	if(fp == NULL)
		return 0;

	while(fgets(line,sizeof(line),fp) != NULL)
	{
		line_counter++;
		if(line[0]=='/' && line[1]=='/')
			continue;

		i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\t]\t%d\t%[^\r\n]%n",
			&account_id, userid, pass, lastlogin, &sex, &logincount, &state,
			email, error_message, &connect_until_time, last_ip, memo, &ban_until_time, dummy, &n);

		if (i < 13) {
			ShowWarning("Skipping incompatible data on line %d\n", line_counter);
			continue;
 		}

		if (i > 13)
			ShowWarning("Reading login account variables is not implemented, data will be lost! (line %d)\n", line_counter);

		user_level = isGM(account_id);
		ShowInfo("Converting user (id: %d, name: %s, gm level: %d)\n", account_id, userid, user_level);
		
		stmt = SqlStmt_Malloc(mysql_handle);
		if( SQL_ERROR == SqlStmt_Prepare(stmt, 
			"REPLACE INTO `login` "
			"(`account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level`, `error_message`, `connect_until`, `last_ip`, `memo`, `ban_until`, `state`) "
			"VALUES "
			"(%d, ?, ?, '%s', '%c', %d, '%s', %d, '%s', %d, '%s', '%s', %d, %d)",
			account_id, lastlogin, sex, logincount, email, user_level, error_message, connect_until_time, last_ip, memo, ban_until_time, state)
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_STRING, userid, strnlen(userid, 255))
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 1, SQLDT_STRING, pass, strnlen(pass, 32))
		||	SQL_ERROR == SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
		}
		SqlStmt_Free(stmt);
	
		//TODO: parse the rest of the line to read the login-stored account variables, and import them to `global_reg_value`
		//      then remove the 'dummy' buffer
	}
	fclose(fp);
	Sql_Free(mysql_handle);

	ShowStatus("Convert end...\n");

	return 0;
}

int login_config_read(const char* cfgName)
{
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	ShowStatus("Start reading interserver configuration: %s\n", cfgName);

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
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
			ShowStatus("set db_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_port")==0){
			db_server_port=atoi(w2);
			ShowStatus("set db_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_id")==0){
			strcpy(db_server_id, w2);
			ShowStatus("set db_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_pw")==0){
			strcpy(db_server_pw, w2);
			ShowStatus("set db_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_logindb")==0){
			strcpy(db_server_logindb, w2);
			ShowStatus("set db_server_logindb : %s\n",w2);
		}
		//support the import command, just like any other config
		else if(strcmpi(w1,"import")==0){
			login_config_read(w2);
		}
	}
	fclose(fp);
	ShowStatus("End reading interserver configuration...\n");
	return 0;
}

int do_init(int argc, char** argv)
{
	int input;
	login_config_read( (argc > 1) ? argv[1] : INTER_CONF_NAME );
	read_gm_account();

	ShowInfo("\nWarning : Make sure you backup your databases before continuing!\n");
	ShowInfo("\nDo you wish to convert your Login Database to SQL? (y/n) : ");
	input = getchar();
	if(input == 'y' || input == 'Y')
		convert_login();
	return 0;
}

void do_final(void)
{
	if( gm_account_db )
	{
		db_destroy(gm_account_db);
		gm_account_db = NULL;
	}
}
