// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "showmsg.h"

#ifdef _WIN32
	#ifdef DEBUGLOGMAP
		#define DEBUGLOGPATH "log\\map-server.log"
	#else
		#ifdef DEBUGLOGCHAR
			#define DEBUGLOGPATH "log\\char-server.log"
		#else
			#ifdef DEBUGLOGLOGIN
				#define DEBUGLOGPATH "log\\login-server.log"
			#endif
		#endif
	#endif
#else
	#ifdef DEBUGLOGMAP
		#define DEBUGLOGPATH "log/map-server.log"
	#else
		#ifdef DEBUGLOGCHAR
			#define DEBUGLOGPATH "log/char-server.log"
		#else
			#ifdef DEBUGLOGLOGIN
				#define DEBUGLOGPATH "log/login-server.log"
			#endif
		#endif
	#endif
#endif

int msg_silent; //Specifies how silent the console is.
char tmp_output[1024] = {"\0"};
char timestamp_format[20] = ""; //For displaying Timestamps
// by MC Cameri
int _vShowMessage(enum msg_type flag, const char *string, va_list ap)
{
	// _ShowMessage MUST be used instead of printf as of 10/24/2004.
	// Return: 0 = Successful, 1 = Failed.
//	int ret = 0;
	char prefix[100];
#if defined(DEBUGLOGMAP) || defined(DEBUGLOGCHAR) || defined(DEBUGLOGLOGIN)
	FILE *fp;
#endif
	
	if (!string || strlen(string) <= 0) {
		ShowError("Empty string passed to _vShowMessage().\n");
		return 1;
	}

	if (timestamp_format[0])
	{	//Display time format. [Skotlex]
		time_t t = time(NULL);
		strftime(prefix, 80, timestamp_format, localtime(&t));
	} else prefix[0]='\0';


	switch (flag) {
		case MSG_NONE: // direct printf replacement
			break;
		case MSG_STATUS: //Bright Green (To inform about good things)
			strcat(prefix,CL_GREEN"[Status]"CL_RESET":");
			break;
		case MSG_SQL: //Bright Violet (For dumping out anything related with SQL) <- Actually, this is mostly used for SQL errors with the database, as successes can as well just be anything else... [Skotlex]
			strcat(prefix,CL_MAGENTA"[SQL]"CL_RESET":");
			break;
		case MSG_INFORMATION: //Bright White (Variable information)
			strcat(prefix,CL_WHITE"[Info]"CL_RESET":");
			break;
		case MSG_NOTICE: //Bright White (Less than a warning)
			strcat(prefix,CL_WHITE"[Notice]"CL_RESET":");
			break;
		case MSG_WARNING: //Bright Yellow
			strcat(prefix,CL_YELLOW"[Warning]"CL_RESET":");
			break;
		case MSG_DEBUG: //Bright Cyan, important stuff!
			strcat(prefix,CL_CYAN"[Debug]"CL_RESET":");
			break;
		case MSG_ERROR: //Bright Red  (Regular errors)
			strcat(prefix,CL_RED"[Error]"CL_RESET":");
			break;
		case MSG_FATALERROR: //Bright Red (Fatal errors, abort(); if possible)
			strcat(prefix,CL_RED"[Fatal Error]"CL_RESET":");
			break;
		default:
			ShowError("In function _vShowMessage() -> Invalid flag passed.\n");
			return 1;
	}

	if ((flag == MSG_DEBUG && !SHOW_DEBUG_MSG) ||
		(flag == MSG_INFORMATION && msg_silent&1) ||
		(flag == MSG_STATUS && msg_silent&2) ||
		(flag == MSG_NOTICE && msg_silent&4) ||
		(flag == MSG_WARNING && msg_silent&8) ||
		(flag == MSG_ERROR && msg_silent&16) ||
		(flag == MSG_SQL && msg_silent&16)
		) ; //Do not print it.
	else {
		if (flag == MSG_ERROR || flag == MSG_FATALERROR || flag == MSG_SQL)
		{	//Send Errors to StdErr [Skotlex]
			fprintf (stderr, "%s ", prefix);
			vfprintf (stderr, string, ap);
			fflush (stderr);
		} else {
			if (flag != MSG_NONE)
				printf ("%s ", prefix);
			vprintf (string, ap);
			fflush (stdout);
		}
	}

#if defined(DEBUGLOGMAP) || defined(DEBUGLOGCHAR) || defined(DEBUGLOGLOGIN)
	if(strlen(DEBUGLOGPATH) > 0) {
		fp=fopen(DEBUGLOGPATH,"a");
		if (fp == NULL)	{
			printf(CL_RED"[ERROR]"CL_RESET": Could not open '"CL_WHITE"%s"CL_RESET"', access denied.\n",DEBUGLOGPATH);
			fflush(stdout);
			return 0;
		}
		fprintf(fp,"%s ", prefix);
		vfprintf(fp,string,ap);
		fclose(fp);
	} else {
		printf(CL_RED"[ERROR]"CL_RESET": DEBUGLOGPATH not defined!\n");
	}
#endif

	va_end(ap);
/*
	if ((core_config.debug_output_level > -1) && (flag >= core_config.debug_output_level)) {
		FILE *fp;
		fp=fopen(OUTPUT_MESSAGES_LOG,"a");
		if (fp == NULL)	{
			ShowError("Could not open '"CL_WHITE"%s"CL_RESET"', file not found.\n",OUTPUT_MESSAGES_LOG);
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

void ClearScreen(void)
{
#ifndef _WIN32
	ShowMessage(CL_CLS);	// to prevent empty string passed messages
#endif
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
