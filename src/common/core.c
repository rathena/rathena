// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mmo.h"
#include "showmsg.h"
#include "malloc.h"
#include "core.h"
#include "strlib.h"
#ifndef MINICORE
#include "socket.h"
#include "timer.h"
#include "thread.h"
#include "mempool.h"
#include "sql.h"
#endif
#include <stdlib.h>
#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include "../common/winapi.h" // Console close event handling
#include <direct.h> // _chdir
#endif


/// Called when a terminate signal is received.
void (*shutdown_callback)(void) = NULL;

#if defined(BUILDBOT)
	int buildbotflag = 0;
#endif

int runflag = CORE_ST_RUN;
char db_path[12] = "db"; /// relative path for db from server

char *SERVER_NAME = NULL;
char SERVER_TYPE = ATHENA_SERVER_NONE;

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
sigfunc *compat_signal(int signo, sigfunc *func) {
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
 *	CORE : Console events for Windows
 *--------------------------------------*/
#ifdef _WIN32
static BOOL WINAPI console_handler(DWORD c_event) {
    switch(c_event) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		if( shutdown_callback != NULL )
			shutdown_callback();
		else
			runflag = CORE_ST_STOP;// auto-shutdown
        break;
	default:
		return FALSE;
    }
    return TRUE;
}

static void cevents_init() {
	if (SetConsoleCtrlHandler(console_handler,TRUE)==FALSE)
		ShowWarning ("Unable to install the console handler!\n");
}
#endif

/*======================================
 *	CORE : Signal Sub Function
 *--------------------------------------*/
static void sig_proc(int sn) {
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

void signals_init (void) {
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
const char *get_svn_revision(void) {
		return EXPAND_AND_QUOTE(SVNVERSION);
	}
#else// not SVNVERSION
const char* get_svn_revision(void) {
	static char svn_version_buffer[16] = "";
	FILE *fp;

	if( svn_version_buffer[0] != '\0' )
		return svn_version_buffer;

	// subversion 1.7 uses a sqlite3 database
	// FIXME this is hackish at best...
	// - ignores database file structure
	// - assumes the data in NODES.dav_cache column ends with "!svn/ver/<revision>/<path>)"
	// - since it's a cache column, the data might not even exist
	if( (fp = fopen(".svn"PATHSEP_STR"wc.db", "rb")) != NULL || (fp = fopen(".."PATHSEP_STR".svn"PATHSEP_STR"wc.db", "rb")) != NULL )
	{
	#ifndef SVNNODEPATH
		//not sure how to handle branches, so i'll leave this overridable define until a better solution comes up
		#define SVNNODEPATH trunk
	#endif
		const char* prefix = "!svn/ver/";
		const char* postfix = "/"EXPAND_AND_QUOTE(SVNNODEPATH)")"; // there should exist only 1 entry like this
		size_t prefix_len = strlen(prefix);
		size_t postfix_len = strlen(postfix);
		size_t i,j,len;
		char* buffer;

		// read file to buffer
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		buffer = (char*)aMalloc(len + 1);
		fseek(fp, 0, SEEK_SET);
		len = fread(buffer, 1, len, fp);
		buffer[len] = '\0';
		fclose(fp);

		// parse buffer
		for( i = prefix_len + 1; i + postfix_len <= len; ++i ) {
			if( buffer[i] != postfix[0] || memcmp(buffer + i, postfix, postfix_len) != 0 )
				continue; // postfix missmatch
			for( j = i; j > 0; --j ) {// skip digits
				if( !ISDIGIT(buffer[j - 1]) )
					break;
			}
			if( memcmp(buffer + j - prefix_len, prefix, prefix_len) != 0 )
				continue; // prefix missmatch
			// done
			snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", atoi(buffer + j));
			break;
		}
		aFree(buffer);

		if( svn_version_buffer[0] != '\0' )
			return svn_version_buffer;
	}

	// subversion 1.6 and older?
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
					snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", rev);
				}
			}
			else
			{
				// Bin File format
				if ( fgets(line, sizeof(line), fp) == NULL ) { printf("Can't get bin name\n"); } // Get the name
				if ( fgets(line, sizeof(line), fp) == NULL ) { printf("Can't get entries kind\n"); } // Get the entries kind
				if(fgets(line, sizeof(line), fp)) // Get the rev numver
				{
					snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", atoi(line));
				}
			}
		}
		fclose(fp);

		if( svn_version_buffer[0] != '\0' )
			return svn_version_buffer;
	}

	// fallback
	svn_version_buffer[0] = UNKNOWN_VERSION;
	return svn_version_buffer;
}
#endif

// GIT path
#define GIT_ORIGIN "refs/remotes/origin/master"

// Grabs the hash from the last time the user updated their working copy (last pull)
const char *get_git_hash (void) {
	static char GitHash[41] = ""; //Sha(40) + 1
	FILE *fp;

	if( GitHash[0] != '\0' )
		return GitHash;

	if( (fp = fopen(".git/"GIT_ORIGIN, "r")) != NULL ) {
		char line[64];
		char *rev = malloc(sizeof(char) * 50);

		if( fgets(line, sizeof(line), fp) && sscanf(line, "%40s", rev) )
			snprintf(GitHash, sizeof(GitHash), "%s", rev);

		free(rev);
		fclose(fp);
	} else {
		GitHash[0] = UNKNOWN_VERSION;
	}

	if ( !(*GitHash) ) {
		GitHash[0] = UNKNOWN_VERSION;
	}

	return GitHash;
}

/*======================================
 *	CORE : Display title
 *  ASCII By CalciumKid 1/12/2011
 *--------------------------------------*/
static void display_title(void) {
	const char* svn = get_svn_revision();
	const char* git = get_git_hash();

	ShowMessage("\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"                                                                 "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"       "CL_BT_WHITE"            rAthena Development Team presents                  "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"                 ___   __  __                                    "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"           _____/   | / /_/ /_  ___  ____  ____ _                "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"          / ___/ /| |/ __/ __ \\/ _ \\/ __ \\/ __ `/                "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"         / /  / ___ / /_/ / / /  __/ / / / /_/ /                 "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"        /_/  /_/  |_\\__/_/ /_/\\___/_/ /_/\\__,_/                  "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"                                                                 "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"       "CL_GREEN"              http://rathena.org/board/                        "CL_PASS""CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_PASS"     "CL_BOLD"                                                                 "CL_PASS""CL_CLL""CL_NORMAL"\n");

	if( svn[0] != UNKNOWN_VERSION )
		ShowInfo("SVN Revision: '"CL_WHITE"%s"CL_RESET"'\n", svn);
	else if( git[0] != UNKNOWN_VERSION )
		ShowInfo("Git Hash: '"CL_WHITE"%s"CL_RESET"'\n", git);
}

// Warning if executed as superuser (root)
void usercheck(void)
{
#ifndef _WIN32
    if (geteuid() == 0) {
		ShowWarning ("You are running rAthena with root privileges, it is not necessary.\n");
    }
#endif
}

/*======================================
 *	CORE : MAINROUTINE
 *--------------------------------------*/
int main (int argc, char **argv)
{
	{// initialize program arguments
		char *p1;
		if((p1 = strrchr(argv[0], '/')) != NULL ||  (p1 = strrchr(argv[0], '\\')) != NULL ){
			char *pwd = NULL; //path working directory
			int n=0;
			SERVER_NAME = ++p1;
			n = p1-argv[0]; //calc dir name len
			pwd = safestrncpy(malloc(n + 1), argv[0], n);
			if(chdir(pwd) != 0)
				ShowError("Couldn't change working directory to %s for %s, runtime will probably fail",pwd,SERVER_NAME);
			free(pwd);
		}
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

	Sql_Init();
	rathread_init();
	mempool_init();
	db_init();
	signals_init();

#ifdef _WIN32
	cevents_init();
#endif

	timer_init();
	socket_init();

	do_init(argc,argv);

	// Main runtime cycle
	while (runflag != CORE_ST_STOP) { 
		int next = do_timer(gettick_nocache());
		do_sockets(next);
	}

	do_final();

	timer_final();
	socket_final();
	db_final();
	mempool_final();
	rathread_final();
#endif

	malloc_final();

	return 0;
}
