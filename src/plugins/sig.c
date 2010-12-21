// $Id: sig.c 1 2005-6-13 3:17:17 PM Celestia $

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#define __USE_GNU  // required to enable strsignal on some platforms
#include <string.h>
#include <time.h>
#include "../common/plugin.h"
#include "../common/version.h"
#include "../common/showmsg.h"

PLUGIN_INFO = {
	"Signals",
	PLUGIN_CORE,
	"1.1",
	PLUGIN_VERSION,
	"Handles program signals"
};

PLUGIN_EVENTS_TABLE = {
	{ "sig_init", "Plugin_Init" },
	{ "sig_final", "Plugin_Final" },
	{ NULL, NULL }
};

//////////////////////////////////////

#if defined(_WIN32) || defined(MINGW)
	int sig_init()
	{
		ShowError("sig: This plugin is not supported - Enable 'exchndl' instead!\n");
		return 0;
	}
	int sig_final() { return 0; }
#elif defined (__NETBSD__) || defined (__FREEBSD__)
	int sig_init()
	{
		ShowError("sig: This plugin is not supported!\n");
		return 0;
	}
	int sig_final() { return 0; }
#else

//////////////////////////////////////

#if !defined(CYGWIN)
	#include <execinfo.h>
#endif

const char* (*getrevision)();
unsigned long (*getuptime)();
char *server_name;
int crash_flag = 0;

int sig_final ();

// by Gabuzomeu
// This is an implementation of signal() using sigaction() for portability.
// (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
// Programming in the UNIX Environment_.
//

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

/*=========================================
 *	Dumps the stack using glibc's backtrace
 *-----------------------------------------
 */
#ifdef CYGWIN
	#define FOPEN_ freopen
	extern void cygwin_stackdump();
#else
	#define FOPEN_(fn,m,s) fopen(fn,m)
#endif
void sig_dump(int sn)
{
	FILE *fp;
	char file[256];
	int no = 0;

	crash_flag = 1;	
	// search for a usable filename
	do {
		sprintf (file, "log/%s%04d.stackdump", server_name, ++no);
	} while((fp = fopen(file,"r")) && (fclose(fp), no < 9999));
	// dump the trace into the file

	if ((fp = FOPEN_(file, "w", stderr)) != NULL) {
		const char *revision;
	#ifndef CYGWIN
		void* array[20];
		char **stack;
		size_t size;
	#endif

		ShowNotice ("Dumping stack to '"CL_WHITE"%s"CL_RESET"'...\n", file);
		if ((revision = getrevision()) != NULL)
			fprintf(fp, "Version: svn%s \n", revision);
		else
			fprintf(fp, "Version: %2d.%02d.%02d mod%02d \n", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION, ATHENA_MOD_VERSION);
		fprintf(fp, "Exception: %s \n", strsignal(sn));
		fflush (fp);

	#ifdef CYGWIN
		cygwin_stackdump ();
	#else
		fprintf(fp, "Stack trace:\n");
		size = backtrace (array, 20);
		stack = backtrace_symbols (array, size);
		for (no = 0; no < size; no++) {
			fprintf(fp, "%s\n", stack[no]);
		}
		fprintf(fp,"End of stack trace\n");
		free(stack);
	#endif

		ShowNotice("%s Saved.\n", file);
		fflush(stdout);
		fclose(fp);
	}

	sig_final();	// Log our uptime
	// Pass the signal to the system's default handler
	compat_signal(sn, SIG_DFL);
	raise(sn);
}

/*=========================================
 *	Shutting down (Program did not crash ^^)
 *	- Log our current up time
 *-----------------------------------------
 */
int sig_final ()
{
	time_t curtime;
	char curtime2[24];	
	FILE *fp;
	long seconds = 0, day = 24*60*60, hour = 60*60,
		minute = 60, days = 0, hours = 0, minutes = 0;

	fp = fopen("log/uptime.log","a");
	if (fp) {
		time(&curtime);
		strftime(curtime2, 24, "%m/%d/%Y %H:%M:%S", localtime(&curtime));

		seconds = getuptime();
		days = seconds/day;
		seconds -= (seconds/day>0)?(seconds/day)*day:0;
		hours = seconds/hour;
		seconds -= (seconds/hour>0)?(seconds/hour)*hour:0;
		minutes = seconds/minute;
		seconds -= (seconds/minute>0)?(seconds/minute)*minute:0;

		fprintf(fp, "%s: %s %s - %ld days, %ld hours, %ld minutes, %ld seconds.\n",
			curtime2, server_name, (crash_flag ? "crashed" : "uptime"),
			days, hours, minutes, seconds);
		fclose(fp);
	}

	return 1;
}

/*=========================================
 *	Register the signal handlers
 *-----------------------------------------
 */
int sig_init ()
{
	void (*func) = sig_dump;
#ifdef CYGWIN	// test if dumper is enabled
	char *buf = getenv ("CYGWIN");
	if (buf && strstr(buf, "error_start") != NULL)
		func = SIG_DFL;
#endif

	IMPORT_SYMBOL(server_name, 1);
	IMPORT_SYMBOL(getrevision, 6);
	IMPORT_SYMBOL(getuptime, 11);

	compat_signal(SIGSEGV, func);
	compat_signal(SIGFPE, func);
	compat_signal(SIGILL, func);
	compat_signal(SIGBUS, func);

	return 1;
}
#endif
