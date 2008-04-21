// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // GetTickCount()
#else
#include <unistd.h>
#include <sys/time.h> // struct timeval, gettimeofday()
#endif

// If the server can't handle processing thousands of monsters
// or many connected clients, please increase TIMER_MIN_INTERVAL.
#define TIMER_MIN_INTERVAL 50

// timers
static struct TimerData* timer_data	= NULL;
static int timer_data_max	= 0;
static int timer_data_num	= 0;

// free timers
static int* free_timer_list		= NULL;
static int free_timer_list_max	= 0;
static int free_timer_list_pos	= 0;

//NOTE: using a binary heap should improve performance [FlavioJS]
// timer heap (ordered array of tid's)
static int timer_heap_num = 0;
static int timer_heap_max = 0;
static int* timer_heap = NULL;

// server startup time
time_t start_time;

unsigned int calc_times(void)
{
	time_t temp = time(NULL);

	return (unsigned int)mktime(localtime(&temp));
}

/*----------------------------
 * 	Timer debugging
 *----------------------------*/
struct timer_func_list {
	struct timer_func_list* next;
	TimerFunc func;
	char* name;
} *tfl_root = NULL;

/// Sets the name of a timer function.
int add_timer_func_list(TimerFunc func, char* name)
{
	struct timer_func_list* tfl;

	if (name) {
		for( tfl=tfl_root; tfl != NULL; tfl=tfl->next )
		{// check suspicious cases
			if( func == tfl->func )
				ShowWarning("add_timer_func_list: duplicating function %08x(%s) as %s.\n",(int)tfl->func,tfl->name,name);
			else if( strcmp(name,tfl->name) == 0 )
				ShowWarning("add_timer_func_list: function %08X has the same name as %08X(%s)\n",(int)func,(int)tfl->func,tfl->name);
		}
		CREATE(tfl,struct timer_func_list,1);
		tfl->next = tfl_root;
		tfl->func = func;
		tfl->name = aStrdup(name);
		tfl_root = tfl;
	}
	return 0;
}

/// Returns the name of the timer function.
char* search_timer_func_list(TimerFunc func)
{
	struct timer_func_list* tfl;

	for( tfl=tfl_root; tfl != NULL; tfl=tfl->next )
		if (func == tfl->func)
			return tfl->name;

	return "unknown timer function";
}

/*----------------------------
 * 	Get tick time
 *----------------------------*/

/// platform-abstracted tick retrieval
static unsigned int tick(void)
{
#if defined(WIN32)
	return GetTickCount();
#elif (defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK) /* posix compliant */) || (defined(__FreeBSD_cc_version) && __FreeBSD_cc_version >= 500005 /* FreeBSD >= 5.1.0 */)
	struct timespec tval;
	clock_gettime(CLOCK_MONOTONIC, &tval);
	return tval.tv_sec * 1000 + tval.tv_nsec / 1000000;
#else
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return tval.tv_sec * 1000 + tval.tv_usec / 1000;
#endif
}

//////////////////////////////////////////////////////////////////////////
#if defined(TICK_CACHE) && TICK_CACHE > 1
//////////////////////////////////////////////////////////////////////////
// tick is cached for TICK_CACHE calls
static unsigned int gettick_cache;
static int gettick_count = 1;

unsigned int gettick_nocache(void)
{
	gettick_count = TICK_CACHE;
	gettick_cache = tick();
	return gettick_cache;
}

unsigned int gettick(void)
{
	return ( --gettick_count == 0 ) ? gettick_nocache() : gettick_cache;
}
//////////////////////////////
#else
//////////////////////////////
// tick doesn't get cached
unsigned int gettick_nocache(void)
{
	return tick();
}

unsigned int gettick(void)
{
	return tick();
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////

/*======================================
 * 	CORE : Timer Heap
 *--------------------------------------*/

// searches for the target tick's position and stores it in pos (binary search)
#define HEAP_SEARCH(target,from,to,pos) \
	do { \
		int max,pivot; \
		max = to; \
		pos = from; \
		while (pos < max) { \
			pivot = (pos + max) / 2; \
			if (DIFF_TICK(target, timer_data[timer_heap[pivot]].tick) < 0) \
				pos = pivot + 1; \
			else \
				max = pivot; \
		} \
	} while(0)

/// Adds a timer to the timer_heap
static void push_timer_heap(int tid)
{
	int pos;

	// check number of element
	if (timer_heap_num >= timer_heap_max) {
		if (timer_heap_max == 0) {
			timer_heap_max = 256;
			CREATE(timer_heap, int, 256);
		} else {
			timer_heap_max += 256;
			RECREATE(timer_heap, int, timer_heap_max);
			memset(timer_heap + (timer_heap_max - 256), 0, sizeof(int) * 256);
		}
	}

	// do a sorting from higher to lower
	if( timer_heap_num == 0 || DIFF_TICK(timer_data[tid].tick, timer_data[timer_heap[timer_heap_num - 1]].tick) < 0 )
		timer_heap[timer_heap_num] = tid; // if lower actual item is higher than new
	else
	{
		// searching position
		HEAP_SEARCH(timer_data[tid].tick,0,timer_heap_num-1,pos);
		// move elements
		memmove(&timer_heap[pos + 1], &timer_heap[pos], sizeof(int) * (timer_heap_num - pos));
		// save new element
		timer_heap[pos] = tid;
	}

	timer_heap_num++;
}

/*==========================
 * 	Timer Management
 *--------------------------*/

/// Returns a free timer id.
static int acquire_timer(void)
{
	int tid;

	if (free_timer_list_pos) {
		do {
			tid = free_timer_list[--free_timer_list_pos];
		} while(tid >= timer_data_num && free_timer_list_pos > 0);
	} else
		tid = timer_data_num;

	if (tid >= timer_data_num)
		for (tid = timer_data_num; tid < timer_data_max && timer_data[tid].type; tid++);
	if (tid >= timer_data_num && tid >= timer_data_max)
	{// expand timer array
		if (timer_data_max == 0)
		{// create timer data (1st time)
			timer_data_max = 256;
			CREATE(timer_data, struct TimerData, timer_data_max);
		} else
		{// add more timers
			timer_data_max += 256;
			RECREATE(timer_data, struct TimerData, timer_data_max);
			memset(timer_data + (timer_data_max - 256), 0, sizeof(struct TimerData) * 256);
		}
	}

	if (tid >= timer_data_num)
		timer_data_num = tid + 1;

	return tid;
}

/// Starts a new timer that is deleted once it expires (single-use).
/// Returns the timer's id.
int add_timer(unsigned int tick, TimerFunc func, int id, intptr data)
{
	int tid;
	
	tid = acquire_timer();
	timer_data[tid].tick     = tick;
	timer_data[tid].func     = func;
	timer_data[tid].id       = id;
	timer_data[tid].data     = data;
	timer_data[tid].type     = TIMER_ONCE_AUTODEL;
	timer_data[tid].interval = 1000;
	push_timer_heap(tid);

	return tid;
}

/// Starts a new timer that automatically restarts itself (infinite loop until manually removed).
/// Returns the timer's id, or -1 if it fails.
int add_timer_interval(unsigned int tick, TimerFunc func, int id, intptr data, int interval)
{
	int tid;

	if( interval < 1 ) {
		ShowError("add_timer_interval : function %08x(%s) has invalid interval %d!\n", (int)func, search_timer_func_list(func), interval);
		return -1;
	}
	
	tid = acquire_timer();
	timer_data[tid].tick     = tick;
	timer_data[tid].func     = func;
	timer_data[tid].id       = id;
	timer_data[tid].data     = data;
	timer_data[tid].type     = TIMER_INTERVAL;
	timer_data[tid].interval = interval;
	push_timer_heap(tid);

	return tid;
}

/// Retrieves internal timer data
//FIXME: for safety, the return value should be 'const'
struct TimerData* get_timer(int tid)
{
	return &timer_data[tid];
}

/// Marks a timer specified by 'id' for immediate deletion once it expires.
/// Param 'func' is used for debug/verification purposes.
/// Returns 0 on success, < 0 on failure.
int delete_timer(int tid, TimerFunc func)
{
	if( tid <= 0 || tid >= timer_data_num ) {
		ShowError("delete_timer error : no such timer %d (%08x(%s))\n", tid, (int)func, search_timer_func_list(func));
		return -1;
	}
	if( timer_data[tid].func != func ) {
		ShowError("delete_timer error : function mismatch %08x(%s) != %08x(%s)\n", (int)timer_data[tid].func, search_timer_func_list(timer_data[tid].func), (int)func, search_timer_func_list(func));
		return -2;
	}

	timer_data[tid].func = NULL;
	timer_data[tid].type = TIMER_ONCE_AUTODEL;

	return 0;
}

/// Adjusts a timer's expiration time.
/// Returns the new tick value, or -1 if it fails.
int addtick_timer(int tid, unsigned int tick)
{
	return settick_timer(tid, timer_data[tid].tick+tick);
}

/// Modifies a timer's expiration time (an alternative to deleting a timer and starting a new one).
/// Returns the new tick value, or -1 if it fails.
int settick_timer(int tid, unsigned int tick)
{
	int old_pos,pos;
	unsigned int old_tick;
	
	old_tick = timer_data[tid].tick;
	if( old_tick == tick )
		return tick;

	// search old_tick position
	HEAP_SEARCH(old_tick,0,timer_heap_num-1,old_pos);
	while( timer_heap[old_pos] != tid )
	{// skip timers with the same tick
		if( old_tick != timer_data[timer_heap[old_pos]].tick )
		{
			ShowError("settick_timer: no such timer %d (%08x(%s))\n", tid, (int)timer_data[tid].func, search_timer_func_list(timer_data[tid].func));
			return -1;
		}
		++old_pos;
	}

	if( DIFF_TICK(tick,timer_data[tid].tick) < 0 )
	{// Timer is accelerated, shift timer near the end of the heap.
		if (old_pos == timer_heap_num-1) //Nothing to shift.
			pos = old_pos;
		else {
			HEAP_SEARCH(tick,old_pos+1,timer_heap_num-1,pos);
			--pos;
			if (pos != old_pos)
				memmove(&timer_heap[old_pos], &timer_heap[old_pos+1], (pos-old_pos)*sizeof(int));
		}
	} else
	{// Timer is delayed, shift timer near the beginning of the heap.
		if (old_pos == 0) //Nothing to shift.
			pos = old_pos;
		else {
			HEAP_SEARCH(tick,0,old_pos-1,pos);
			++pos;
			if (pos != old_pos)
				memmove(&timer_heap[pos+1], &timer_heap[pos], (old_pos-pos)*sizeof(int));
		}
	}

	timer_heap[pos] = tid;
	timer_data[tid].tick = tick;

	return tick;
}

/// Executes all expired timers.
/// Returns the value of the smallest non-expired timer (or 1 second if there aren't any).
int do_timer(unsigned int tick)
{
	int nextmin = 1000; // return value
	int i;

	// process all timers one by one
	while( timer_heap_num )
	{
		i = timer_heap[timer_heap_num - 1]; // last element in heap (=>smallest)
		if( (nextmin = DIFF_TICK(timer_data[i].tick, tick)) > 0 )
			break; // no more expired timers to process

		--timer_heap_num; // suppress the actual element from the table

		// mark timer as 'to be removed'
		timer_data[i].type |= TIMER_REMOVE_HEAP;

		if( timer_data[i].func )
		{
			if( nextmin < -1000 )
				// 1秒以上の大幅な遅延が発生しているので、
				// timer処理タイミングを現在値とする事で
				// 呼び出し時タイミング(引数のtick)相対で処理してる
				// timer関数の次回処理タイミングを遅らせる
				timer_data[i].func(i, tick, timer_data[i].id, timer_data[i].data);
			else
				timer_data[i].func(i, timer_data[i].tick, timer_data[i].id, timer_data[i].data);
		}

		// in the case the function didn't change anything...
		if( timer_data[i].type & TIMER_REMOVE_HEAP )
		{
			timer_data[i].type &= ~TIMER_REMOVE_HEAP;

			switch( timer_data[i].type )
			{
			case TIMER_ONCE_AUTODEL:
				timer_data[i].type = 0;
				if (free_timer_list_pos >= free_timer_list_max) {
					free_timer_list_max += 256;
					RECREATE(free_timer_list,int,free_timer_list_max);
					memset(free_timer_list + (free_timer_list_max - 256), 0, 256 * sizeof(int));
				}
				free_timer_list[free_timer_list_pos++] = i;
			break;
			case TIMER_INTERVAL:
				if (DIFF_TICK(timer_data[i].tick , tick) < -1000) {
					timer_data[i].tick = tick + timer_data[i].interval;
				} else {
					timer_data[i].tick += timer_data[i].interval;
				}
				timer_data[i].type &= ~TIMER_REMOVE_HEAP;
				push_timer_heap(i);
			break;
			}
		}
	}

	if( nextmin < TIMER_MIN_INTERVAL )
		nextmin = TIMER_MIN_INTERVAL;

	return nextmin;
}

unsigned long get_uptime(void)
{
	return (unsigned long)difftime(time(NULL), start_time);
}

void timer_init(void)
{
	time(&start_time);
}

void timer_final(void)
{
	struct timer_func_list *tfl;
	struct timer_func_list *next;

	for( tfl=tfl_root; tfl != NULL; tfl = next ) {
		next = tfl->next;	// copy next pointer
		aFree(tfl->name);	// free structures
		aFree(tfl);
	}

	if (timer_data) aFree(timer_data);
	if (timer_heap) aFree(timer_heap);
	if (free_timer_list) aFree(free_timer_list);
}
