// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef TIMER_HPP
#define TIMER_HPP

#include <time.h>

#include "cbasetypes.hpp"

#define tick_t int64
#define PRtf PRId64

static inline tick_t tick_diff( tick_t a, tick_t b ){
	return a - b;
}

// Convenience for now
#define DIFF_TICK(a,b) tick_diff(a,b)

const tick_t INFINITE_TICK = -1;

#define INVALID_TIMER -1
#define CLIF_WALK_TIMER -2

// timer flags
enum {
	TIMER_ONCE_AUTODEL = 0x01,
	TIMER_INTERVAL = 0x02,
	TIMER_REMOVE_HEAP = 0x10,
};

#define TIMER_FUNC(x) int x ( int tid, tick_t tick, int id, intptr_t data )

// Struct declaration
typedef TIMER_FUNC((*TimerFunc));

struct TimerData {
	tick_t tick;
	TimerFunc func;
	unsigned int type;
	int interval;

	// general-purpose storage
	int id;
	intptr_t data;
};

// Function prototype declaration

tick_t gettick(void);
tick_t gettick_nocache(void);

int add_timer(tick_t tick, TimerFunc func, int id, intptr_t data);
int add_timer_interval(tick_t tick, TimerFunc func, int id, intptr_t data, int interval);
const struct TimerData* get_timer(int tid);
int delete_timer(int tid, TimerFunc func);

tick_t addtick_timer(int tid, tick_t tick);
tick_t settick_timer(int tid, tick_t tick);

int add_timer_func_list(TimerFunc func, const char* name);

unsigned long get_uptime(void);

//transform a timestamp to string
const char* timestamp2string(char* str, size_t size, time_t timestamp, const char* format);
void split_time(int time, int* year, int* month, int* day, int* hour, int* minute, int* second);
double solve_time(char* modif_p);

tick_t do_timer(tick_t tick);
void timer_init(void);
void timer_final(void);

#endif /* TIMER_HPP */
