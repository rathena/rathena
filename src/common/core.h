// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_CORE_H_
#define	_CORE_H_

extern int runflag;

int do_init(int,char**);

void set_termfunc(void (*termfunc)(void));

#endif	// _CORE_H_
