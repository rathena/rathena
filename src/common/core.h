// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_CORE_H_
#define	_CORE_H_

//#define SQL_DEBUG //uncomment for debug_mysql_query instead of mysql_real_query
	
extern int arg_c;
extern char **arg_v;

extern int runflag;
extern char *SERVER_NAME;
extern char SERVER_TYPE;

extern int parse_console(char* buf);
extern const char *get_svn_revision(void);
extern int do_init(int,char**);
extern void set_server_type(void);
extern void set_termfunc(void (*termfunc)(void));
extern void do_abort(void);
extern void do_final(void);

#endif /* _CORE_H_ */
