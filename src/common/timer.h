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

#define TIMER_ONCE_AUTODEL 1
#define TIMER_INTERVAL 2
#define TIMER_REMOVE_HEAP 16

#define DIFF_TICK(a,b) ((int)((a)-(b)))

// Struct declaration

struct TimerData {
	unsigned int tick;
	int (*func)(int,unsigned int,int,int);
	int id;
	int data;
	int type;
	int interval;
	int heap_pos;
};

// Function prototype declaration

#ifdef __WIN32
void gettimeofday(struct timeval *t, void *dummy);
#endif

unsigned int gettick_nocache(void);
unsigned int gettick(void);

int add_timer(unsigned int,int (*)(int,unsigned int,int,int),int,int);
int add_timer_interval(unsigned int,int (*)(int,unsigned int,int,int),int,int,int);
int delete_timer(int,int (*)(int,unsigned int,int,int));

int addtick_timer(int tid,unsigned int tick);
int settick_timer(int tid,unsigned int tick);
struct TimerData *get_timer(int tid);

int do_timer(unsigned int tick);

int add_timer_func_list(int (*)(int,unsigned int,int,int),char*);
char* search_timer_func_list(int (*)(int,unsigned int,int,int));

unsigned long get_uptime(void);

void timer_init(void);
void timer_final(void);

#endif	// _TIMER_H_
