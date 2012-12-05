// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_TIMER_H_
#define	_TIMER_H_

#include "../common/cbasetypes.h"

#define DIFF_TICK(a,b) ((int)((a)-(b)))

#define INVALID_TIMER -1

// timer flags
enum {
	TIMER_ONCE_AUTODEL = 0x01,
	TIMER_INTERVAL = 0x02,
	TIMER_REMOVE_HEAP = 0x10,
};

// Struct declaration

typedef int (*TimerFunc)(int tid, unsigned int tick, int id, intptr_t data);

struct TimerData {
	unsigned int tick;
	TimerFunc func;
	int type;
	int interval;
	int heap_pos;

	// general-purpose storage
	int id; 
	intptr_t data;
};

// Function prototype declaration

unsigned int gettick(void);
unsigned int gettick_nocache(void);

int add_timer(unsigned int tick, TimerFunc func, int id, intptr_t data);
int add_timer_interval(unsigned int tick, TimerFunc func, int id, intptr_t data, int interval);
const struct TimerData* get_timer(int tid);
int delete_timer(int tid, TimerFunc func);

int addtick_timer(int tid, unsigned int tick);
int settick_timer(int tid, unsigned int tick);

int add_timer_func_list(TimerFunc func, char* name);

unsigned long get_uptime(void);

int do_timer(unsigned int tick);
void timer_init(void);
void timer_final(void);

#endif /* _TIMER_H_ */
