// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_CORE_HPP_
#define	_CORE_HPP_

/* so that developers with --enable-debug can raise signals from any section of the code they'd like */
#ifdef DEBUG
	#include <signal.h>
#endif

extern int arg_c;
extern char **arg_v;

#if defined(BUILDBOT)
	extern int buildbotflag;
#endif

#define UNKNOWN_VERSION '\x02'

/// @see E_CORE_ST
extern int runflag;
extern char *SERVER_NAME;
extern char db_path[12]; /// relative path for db from servers

enum {
	ATHENA_SERVER_NONE = 0,	// not defined
	ATHENA_SERVER_LOGIN	= 1,	// login server
	ATHENA_SERVER_CHAR = 2,	// char server
	ATHENA_SERVER_INTER	= 4,	// inter server
	ATHENA_SERVER_MAP = 8,	// map server
};

extern char SERVER_TYPE;

extern int parse_console(const char* buf);
const char *get_svn_revision(void);
const char *get_git_hash(void);
extern int do_init(int,char**);
extern void set_server_type(void);
extern void do_abort(void);
extern void do_final(void);

/// The main loop continues until runflag is CORE_ST_STOP
enum E_CORE_ST
{
	CORE_ST_STOP = 0,
	CORE_ST_RUN,
	CORE_ST_LAST
};

/// Called when a terminate signal is received. (Ctrl+C pressed)
/// If NULL, runflag is set to CORE_ST_STOP instead.
extern void (*shutdown_callback)(void);

#endif /* _CORE_HPP_ */
