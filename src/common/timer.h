// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_TIMER_H_
#define	_TIMER_H_

#ifdef __WIN32
/* We need winsock lib to have timeval struct - windows is weirdo */
#define __USE_W32_SOCKETS
#include <windows.h>
#endif

#define BASE_TICK 5

#define TIMER_ONCE_AUTODEL 0x1
#define TIMER_INTERVAL 0x2
#define TIMER_REMOVE_HEAP 0x10

#define DIFF_TICK(a,b) ((int)((a)-(b)))

// Struct declaration

typedef int (*TimerFunc)(int,unsigned int,int,int);

struct TimerData {
	unsigned int tick;
	TimerFunc func;
	int id;
	int data;
	int type;
	int interval;
	int heap_pos;
};

// Function prototype declaration

unsigned int gettick_nocache(void);
unsigned int gettick(void);

int add_timer(unsigned int,TimerFunc f,int,int);
int add_timer_interval(unsigned int,TimerFunc f,int,int,int);
int delete_timer(int,TimerFunc f);

int addtick_timer(int tid,unsigned int tick);
int settick_timer(int tid,unsigned int tick);
struct TimerData *get_timer(int tid);

int do_timer(unsigned int tick);

int add_timer_func_list(TimerFunc func, char* name);
char* search_timer_func_list(TimerFunc f);

unsigned long get_uptime(void);

void timer_init(void);
void timer_final(void);

#endif	// _TIMER_H_
