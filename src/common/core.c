// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/version.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"
#include "core.h"
#ifndef MINICORE
#include "../common/db.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/plugins.h"
#endif
#ifndef _WIN32
#include "svnversion.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif


/// Called when a terminate signal is received.
void (*shutdown_callback)(void) = NULL;


int runflag = CORE_ST_RUN;
int arg_c = 0;
char **arg_v = NULL;

char *SERVER_NAME = NULL;
char SERVER_TYPE = ATHENA_SERVER_NONE;
#ifndef SVNVERSION
	static char eA_svn_version[10] = "";
#endif

#ifndef MINICORE	// minimalist Core
// Added by Gabuzomeu
//
// This is an implementation of signal() using sigaction() for portability.
// (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
// Programming in the UNIX Environment_.
//
#ifdef WIN32	// windows don't have SIGPIPE
#define SIGPIPE SIGINT
#endif

#ifndef POSIX
#define compat_signal(signo, func) signal(signo, func)
#else
sigfunc *compat_signal(int signo, sigfunc *func)
{
	struct sigaction sact, oact;

	sact.sa_handler = func;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
#ifdef SA_INTERRUPT
	sact.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif

	if (sigaction(signo, &sact, &oact) < 0)
		return (SIG_ERR);

	return (oact.sa_handler);
}
#endif

/*======================================
 *	CORE : Signal Sub Function
 *--------------------------------------*/
static void sig_proc(int sn)
{
	static int is_called = 0;

	switch (sn) {
	case SIGINT:
	case SIGTERM:
		if (++is_called > 3)
			exit(EXIT_SUCCESS);
		if( shutdown_callback != NULL )
			shutdown_callback();
		else
			runflag = CORE_ST_STOP;// auto-shutdown
		break;
	case SIGSEGV:
	case SIGFPE:
		do_abort();
		// Pass the signal to the system's default handler
		compat_signal(sn, SIG_DFL);
		raise(sn);
		break;
#ifndef _WIN32
	case SIGXFSZ:
		// ignore and allow it to set errno to EFBIG
		ShowWarning ("Max file size reached!\n");
		//run_flag = 0;	// should we quit?
		break;
	case SIGPIPE:
		//ShowInfo ("Broken pipe found... closing socket\n");	// set to eof in socket.c
		break;	// does nothing here
#endif
	}
}

void signals_init (void)
{
	compat_signal(SIGTERM, sig_proc);
	compat_signal(SIGINT, sig_proc);
#ifndef _DEBUG // need unhandled exceptions to debug on Windows
	compat_signal(SIGSEGV, sig_proc);
	compat_signal(SIGFPE, sig_proc);
#endif
#ifndef _WIN32
	compat_signal(SIGILL, SIG_DFL);
	compat_signal(SIGXFSZ, sig_proc);
	compat_signal(SIGPIPE, sig_proc);
	compat_signal(SIGBUS, SIG_DFL);
	compat_signal(SIGTRAP, SIG_DFL);
#endif
}
#endif

#ifdef SVNVERSION
	#define xstringify(x) stringify(x)
	#define stringify(x) #x
	const char *get_svn_revision(void)
	{
		return xstringify(SVNVERSION);
	}
#else// not SVNVERSION
const char* get_svn_revision(void)
{
	FILE *fp;

	if(*eA_svn_version)
		return eA_svn_version;

	if ((fp = fopen(".svn/entries", "r")) != NULL)
	{
		char line[1024];
		int rev;
		// Check the version
		if (fgets(line, sizeof(line), fp))
		{
			if(!ISDIGIT(line[0]))
			{
				// XML File format
				while (fgets(line,sizeof(line),fp))
					if (strstr(line,"revision=")) break;
				if (sscanf(line," %*[^\"]\"%d%*[^\n]", &rev) == 1) {
					snprintf(eA_svn_version, sizeof(eA_svn_version), "%d", rev);
				}
			}
			else
			{
				// Bin File format
				fgets(line, sizeof(line), fp); // Get the name
				fgets(line, sizeof(line), fp); // Get the entries kind
				if(fgets(line, sizeof(line), fp)) // Get the rev numver
				{
					snprintf(eA_svn_version, sizeof(eA_svn_version), "%d", atoi(line));
				}
			}
		}
		fclose(fp);
	}

	if(!(*eA_svn_version))
		snprintf(eA_svn_version, sizeof(eA_svn_version), "Unknown");

	return eA_svn_version;
}
#endif

/*======================================
 *	CORE : Display title
 *  ASCII By CalciumKid 1/12/2011
 *--------------------------------------*/
static void display_title(void)
{
	//ClearScreen(); // clear screen and go up/left (0, 0 position in text)
	ShowMessage("\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"                                                              "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"          "CL_BT_WHITE"            RAthena Development Team presents            "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"              ____  ___   __  __                              "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"             / __ \\/   | / /_/ /_  ___  ____  ____ _          "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"            / /_/ / /| |/ __/ __ \\/ _ \\/ __ \\/ __ `/          "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"           / _, _/ ___ / /_/ / / /  __/ / / / /_/ /           "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"          /_/ |_/_/  |_\\__/_/ /_/\\___/_/ /_/\\___,_/           "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"                                                              "CL_PASS""CL_CLL""CL_NORMAL"\n");  
	ShowMessage(""CL_PASS"          "CL_GREEN"                http://rathena.org/board/                "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"        "CL_BOLD"                                                              "CL_PASS""CL_CLL""CL_NORMAL"\n"); 

	ShowInfo("SVN Revision: '"CL_WHITE"%s"CL_RESET"'.\n", get_svn_revision());
}

// Warning if logged in as superuser (root)
void usercheck(void)
{
#ifndef _WIN32
    if ((getuid() == 0) && (getgid() == 0)) {
	ShowWarning ("You are running eAthena as the root superuser.\n");
	ShowWarning ("It is unnecessary and unsafe to run eAthena with root privileges.\n");
	sleep(3);
    }
#endif
}

/*======================================
 *	CORE : MAINROUTINE
 *--------------------------------------*/
int main (int argc, char **argv)
{
	{// initialize program arguments
		char *p1 = SERVER_NAME = argv[0];
		char *p2 = p1;
		while ((p1 = strchr(p2, '/')) != NULL || (p1 = strchr(p2, '\\')) != NULL)
		{
			SERVER_NAME = ++p1;
			p2 = p1;
		}
		arg_c = argc;
		arg_v = argv;
	}

	malloc_init();// needed for Show* in display_title() [FlavioJS]

#ifdef MINICORE // minimalist Core
	display_title();
	usercheck();
	do_init(argc,argv);
	do_final();
#else// not MINICORE
	set_server_type();
	display_title();
	usercheck();

	db_init();
	signals_init();

	timer_init();
	socket_init();
	plugins_init();

	do_init(argc,argv);
	plugin_event_trigger(EVENT_ATHENA_INIT);

	{// Main runtime cycle
		int next;
		while (runflag != CORE_ST_STOP) {
			next = do_timer(gettick_nocache());
			do_sockets(next);
		}
	}

	plugin_event_trigger(EVENT_ATHENA_FINAL);
	do_final();

	timer_final();
	plugins_final();
	socket_final();
	db_final();
#endif

	malloc_final();

	return 0;
}
