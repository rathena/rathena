#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "showmsg.h"

char tmp_output[1024] = {"\0"};

int _ShowMessage(const char *string, enum msg_type flag){ // by MC Cameri
	/* 
		_ShowMessage MUST be used instead of printf as of 10/24/2004.
		Return: 0 = Successful, 1 = Failed.
	*/
//	int ret = 0;
	char prefix[40];
	char *output;
	if (strlen(string) <= 0) {
		ShowError("Empty string passed to ShowMessage().\n");
		return 1;
	}
	switch (flag) {
		case MSG_STATUS: //Bright Green (To inform about good things)
			strcpy(prefix,"\033[1;32m[Status]\033[0;0m: ");
			break;
/*	//Do we really need this now? [MC Cameri]
		case MSG_SQL: //Bright Violet (For dumping out anything related with SQL)
			strcpy(prefix,"\033[1;35m[SQL]\033[0;0m: ");
			break;
*/
		case MSG_INFORMATION: //Bright Blue (Variable information)
			strcpy(prefix,"\033[1;34m[Info]\033[0;0m: ");
			break;
		case MSG_NOTICE: //Bright White (Less than a warning)
			strcpy(prefix,"\033[1;29m[Notice]\033[0;0m: ");
			break;
		case MSG_WARNING: //Bright Yellow
			strcpy(prefix,"\033[1;33m[Warning]\033[0;0m: ");
			break;
		case MSG_ERROR: //Bright Red  (Regular errors)
			strcpy(prefix,"\033[1;31m[Error]\033[0;0m: ");
			break;
		case MSG_FATALERROR: //Bright Red (Fatal errors, abort(); if possible)
			strcpy(prefix,"\033[1;31m[Fatal Error]\033[0;0m: ");
			break;
		default:
			ShowError("In function _ShowMessage() -> Invalid flag passed.\n");
			return 1;
	}
	output = (char*)malloc(sizeof(char)*(strlen(prefix)+strlen(string))+1);
	if (output == NULL) {
		return 1;
//		abort(); // Kill server?
	}
	strcpy(output,prefix);
	strcat(output,string);
	printf(output);
	fflush(stdout);
/*
	if ((core_config.debug_output_level > -1) && (flag >= core_config.debug_output_level)) {
		FILE *fp;
		fp=fopen(OUTPUT_MESSAGES_LOG,"a");
		if (fp == NULL)	{
			printf("\033[1;31m[Error]\033[0;0m: Could not open \033[1;29m%s\033[0;0m, file not found.\n",OUTPUT_MESSAGES_LOG);
			fflush(stdout);
			return;
		}
		StripColor(output);
		strcpy(output,"\r");
		fwrite(output,strlen(output),1,fp);
		fclose(fp);
	}
*/
	return 0;
}
