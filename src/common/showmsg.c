#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "showmsg.h"

char tmp_output[1024] = {"\0"};

// by MC Cameri
int _vShowMessage(enum msg_type flag, const char *string, va_list ap)
{
	// _ShowMessage MUST be used instead of printf as of 10/24/2004.
	// Return: 0 = Successful, 1 = Failed.
//	int ret = 0;
	char prefix[40];
	
	if (!string || strlen(string) <= 0) {
		printf("Empty string passed to _ShowMessage().\n");
		return 1;
	}
	switch (flag) {
		case MSG_NONE: // direct printf replacement
			break;
		case MSG_STATUS: //Bright Green (To inform about good things)
			strcpy(prefix,CL_GREEN"[Status]"CL_RESET":");
			break;
		case MSG_SQL: //Bright Violet (For dumping out anything related with SQL)
			strcpy(prefix,CL_MAGENTA"[SQL]"CL_RESET":");
			break;
		case MSG_INFORMATION: //Bright White (Variable information)
			strcpy(prefix,CL_WHITE"[Info]"CL_RESET":");
			break;
		case MSG_NOTICE: //Bright White (Less than a warning)
			strcpy(prefix,CL_WHITE"[Notice]"CL_RESET":");
			break;
		case MSG_WARNING: //Bright Yellow
			strcpy(prefix,CL_YELLOW"[Warning]"CL_RESET":");
			break;
		case MSG_DEBUG: //Bright Cyan, important stuff!
			strcpy(prefix,CL_CYAN"[Debug]"CL_RESET":");
			break;
		case MSG_ERROR: //Bright Red  (Regular errors)
			strcpy(prefix,CL_RED"[Error]"CL_RESET":");
			break;
		case MSG_FATALERROR: //Bright Red (Fatal errors, abort(); if possible)
			strcpy(prefix,CL_RED"[Fatal Error]"CL_RESET":");
			break;
		default:
			printf("In function _ShowMessage() -> Invalid flag passed.\n");
			return 1;
	}

	if (!(flag == MSG_DEBUG && !SHOW_DEBUG_MSG)) {
		if (flag != MSG_NONE)
			printf ("%s ", prefix);
		vprintf (string, ap);
		fflush (stdout);
	}

	va_end(ap);
/*
	if ((core_config.debug_output_level > -1) && (flag >= core_config.debug_output_level)) {
		FILE *fp;
		fp=fopen(OUTPUT_MESSAGES_LOG,"a");
		if (fp == NULL)	{
			printf(CL_RED"[Error]"CL_RESET": Could not open '"CL_WHITE"%s"CL_RESET"', file not found.\n",OUTPUT_MESSAGES_LOG);
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

int _ShowMessage(enum msg_type flag, const char *string, ...)
{
  	va_list ap;
	
	va_start(ap, string);
        return _vShowMessage(flag, string, ap);
}

// direct printf replacement
int ShowMessage(const char *string, ...) {
	va_list ap;

	va_start(ap, string);
		return _vShowMessage(MSG_NONE, string, ap);
}
int ShowStatus(const char *string, ...) {
	va_list ap;
	
	va_start(ap, string);
		return _vShowMessage(MSG_STATUS, string, ap);
}
int ShowSQL(const char *string, ...) {
	va_list ap;
	
	va_start(ap, string);
		return _vShowMessage(MSG_SQL, string, ap);
}
int ShowInfo(const char *string, ...) {
	va_list ap;
	
	va_start(ap, string);
		return _vShowMessage(MSG_INFORMATION, string, ap);
}
int ShowNotice(const char *string, ...) {
	va_list ap;
	
	va_start(ap, string);
		return _vShowMessage(MSG_NOTICE, string, ap);
}
int ShowWarning(const char *string, ...) {
	va_list ap;
	
	va_start(ap, string);
		return _vShowMessage(MSG_WARNING, string, ap);
}
int ShowDebug(const char *string, ...) {
	va_list ap;
	
	va_start(ap, string);
		return _vShowMessage(MSG_DEBUG, string, ap);
}
int ShowError(const char *string, ...) { 
	va_list ap;
	
	va_start(ap, string);
		return _vShowMessage(MSG_ERROR, string, ap);
}
int ShowFatalError(const char *string, ...) {
	va_list ap;

	va_start(ap, string);
		return _vShowMessage(MSG_FATALERROR, string, ap);
}
