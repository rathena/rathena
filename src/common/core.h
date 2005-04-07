// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_CORE_H_
#define	_CORE_H_

extern char *argp;
extern int runflag;
extern unsigned long ticks;
extern char SERVER_TYPE;

enum {
	SERVER_NONE,
	SERVER_LOGIN,
	SERVER_CHAR,
	SERVER_MAP,
};

int do_init(int,char**);
void set_termfunc(void (*termfunc)(void));

#endif	// _CORE_H_
