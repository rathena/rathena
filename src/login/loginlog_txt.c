// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "account.h"
#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char login_log_filename[1024] = "log/login.log";


// Returns the number of failed login attemps by the ip in the last minutes.
unsigned long loginlog_failedattempts(uint32 ip, unsigned int minutes)
{
	// XXX not implemented
	return 0;
}


/*=============================================
 * Records an event in the login log
 *---------------------------------------------*/
void login_log(uint32 ip, const char* username, int rcode, const char* message)
{
	FILE* log_fp;

	if( !login_config.log_login )
		return;
	
	log_fp = fopen(login_log_filename, "a");
	if( log_fp != NULL )
	{
		char esc_username[NAME_LENGTH*4+1];
		char esc_message[255*4+1];
		time_t raw_time;
		char str_time[24];

		sv_escape_c(esc_username, username, safestrnlen(username,NAME_LENGTH), NULL);
		sv_escape_c(esc_message, message, safestrnlen(message,255), NULL);

		time(&raw_time);
		strftime(str_time, 24, login_config.date_format, localtime(&raw_time));
		str_time[23] = '\0';

		fprintf(log_fp, "%s\t%s\t%s\t%d\t%s\n", str_time, ip2str(ip,NULL), esc_username, rcode, esc_message);

		fclose(log_fp);
	}
}


bool loginlog_config_read(const char* w1, const char* w2)
{
	if(!strcmpi(w1, "login_log_filename"))
		safestrncpy(login_log_filename, w2, sizeof(login_log_filename));
	else
		return false;

	return true;
}


bool loginlog_init(void)
{
	return true;
}


bool loginlog_final(void)
{
	return true;
}
