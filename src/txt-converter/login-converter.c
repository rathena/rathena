// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#define WITH_TXT
#define WITH_SQL

#include "../common/cbasetypes.h"
#include "../common/mmo.h" // struct mmo_account
#include "../common/core.h"
#include "../common/showmsg.h"
#include "../login/account.h"
#include <stdio.h>
#include <string.h>

#define LOGIN_CONF_NAME "conf/login_athena.conf"

AccountDB* txtdb = NULL;
AccountDB* sqldb = NULL;

//--------------------------------------------------------

int convert_login(void)
{
	AccountDBIterator* iter;
	struct mmo_account acc;

	if( !txtdb->init(txtdb) || !sqldb->init(sqldb) )
	{
		ShowFatalError("Initialization failed, unable to start conversion.\n");
		return 0;
	}

	ShowStatus("Conversion started...\n");
	//TODO: do some counting & statistics

	iter = txtdb->iterator(txtdb);
	while( iter->next(iter, &acc) )
	{
		ShowInfo("Converting user (id: %d, name: %s, gm level: %d)...", acc.account_id, acc.userid, acc.level);
		if( sqldb->create(sqldb, &acc) )
			ShowMessage(CL_GREEN "success.\n");
		else
			ShowMessage(CL_RED "failed!\n");
	}
	iter->destroy(iter);

	ShowStatus("Conversion finished.\n");

	return 0;
}

int login_config_read(const char* cfgName)
{
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	ShowStatus("Start reading login server configuration: %s\n", cfgName);

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

		txtdb->set_property(txtdb, w1, w2);
		sqldb->set_property(sqldb, w1, w2);

		//support the import command, just like any other config
		if( strcmpi(w1,"import") == 0 )
			login_config_read(w2);
	}

	fclose(fp);
	ShowStatus("End reading login server configuration...\n");
	return 0;
}

int do_init(int argc, char** argv)
{
	int input;

	txtdb = account_db_txt();
	sqldb = account_db_sql();

	login_config_read( (argc > 1) ? argv[1] : LOGIN_CONF_NAME );

	ShowInfo("\nWarning : Make sure you backup your databases before continuing!\n");
	ShowInfo("\nDo you wish to convert your Login Database to SQL? (y/n) : ");
	input = getchar();

	if(input == 'y' || input == 'Y')
		convert_login();

	return 0;
}

void do_final(void)
{
	txtdb->destroy(txtdb);
	sqldb->destroy(sqldb);	
}
