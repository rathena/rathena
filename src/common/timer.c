// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/db.h"
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
#define TIMER_MAX_INTERVAL 1000

// timers (array)
static struct TimerData* timer_data	= NULL;
static int timer_data_max			= 0;
static int timer_data_num			= 0;

// free timers (array)
static int* free_timer_list		= NULL;
static int free_timer_list_max	= 0;
static int free_timer_list_num	= 0;

// timer heap (binary min heap)
static int* timer_heap		= NULL;
static int timer_heap_max	= 0;
static int timer_heap_num	= 0;

// server startup time
time_t start_time;


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

// root at index 0
#define BHEAP_PARENT(pos) ( ((pos) - 1)/2 )
#define BHEAP_LEFT(pos)   ( (pos)*2 + 1   )
#define BHEAP_RIGHT(pos)  ( (pos)*2 + 2   )

/// Adds a timer to the timer_heap
static
void push_timer_heap(int tid)
{
	int pos;

	// check available space
	if( timer_heap_num >= timer_heap_max )
	{
		timer_heap_max += 256;
		if( timer_heap )
			RECREATE(timer_heap, int, timer_heap_max);
		else
			CREATE(timer_heap, int, timer_heap_max);
		memset(timer_heap + (timer_heap_max - 256), 0, sizeof(int)*256);
	}

	// add the timer at the end
	pos = timer_heap_num++;
	timer_heap[pos] = tid;
	// restore binary heap properties
	while( pos > 0 )
	{
		int parent = BHEAP_PARENT(pos);
		if( DIFF_TICK(timer_data[tid].tick, timer_data[timer_heap[parent]].tick) > 0 )
			break;// done
		swap(timer_heap[pos], timer_heap[parent]);
		pos = parent;
	}
}

/// Removes a timer from the timer_heap
static
bool pop_timer_heap(int tid)
{
	int pos;

	// find the timer
#if 1
	// trying a simple array search
	ARR_FIND(0, timer_heap_num, pos, timer_heap[pos] == tid);
#else
	pos = 0;
	while( pos < timer_heap_num )
	{// search in the order current-left-right
		int left = BHEAP_LEFT(pos);
		int right = BHEAP_RIGHT(pos);

		if( timer_heap[pos] == tid )
			break;// found the timer
		if( left < timer_heap_num && DIFF_TICK(timer_data[tid].tick, timer_data[timer_heap[left]].tick) >= 0 )
		{// try left child
			pos = left;
			continue;
		}
		if( right < timer_heap_num && DIFF_TICK(timer_data[tid].tick, timer_data[timer_heap[right]].tick) >= 0 )
		{// try right child
			pos = right;
			continue;
		}

		// back and right
		while( true )
		{
			int parent;
			if( pos == 0 )
				return false;// not found
			parent = BHEAP_PARENT(pos);
			right = BHEAP_RIGHT(parent);
			if( pos != right && right < timer_heap_num && DIFF_TICK(timer_data[tid].tick, timer_data[timer_heap[right]].tick) >= 0 )
				break;// try this right
			pos = parent;
		}
		pos = right;
	}
#endif
	if( pos >= timer_heap_num )
		return false;// not found

	// remove timer (replace with last one)
	timer_heap[pos] = timer_heap[--timer_heap_num];
	// restore binary heap properties
	while( pos < timer_heap_num )
	{
		int left = BHEAP_LEFT(pos);
		int right = BHEAP_RIGHT(pos);
		if( left < timer_heap_num && DIFF_TICK(timer_data[timer_heap[pos]].tick, timer_data[timer_heap[left]].tick) > 0 )
		{// left exists and has smaller tick
			if( right < timer_heap_num && DIFF_TICK(timer_data[timer_heap[left]].tick, timer_data[timer_heap[right]].tick) > 0 )	
			{// right exists and has even smaller tick
				swap(timer_heap[pos], timer_heap[right]);
				pos = right;
			}
			else
			{
				swap(timer_heap[pos], timer_heap[left]);
				pos = left;
			}
		}
		else if( right < timer_heap_num && DIFF_TICK(timer_data[timer_heap[pos]].tick, timer_data[timer_heap[right]].tick) > 0 )
		{// right exists and has smaller tick
			swap(timer_heap[pos], timer_heap[right]);
			pos = right;
		}
		else
		{
			break;// done
		}
	}
	return true;
}

/*==========================
 * 	Timer Management
 *--------------------------*/

// diff_tick limits (2*24*60*60*1000 is 2 days ; 2*60*60*1000 is 2 hours)
#define FUTURE_DIFF_TICK ( INT_MIN + 2*24*60*60*1000 )
#define MAX_DIFF_TICK ( INT_MAX - 2*60*60*1000 )
#define MIN_DIFF_TICK ( -2*60*60*1000 )
#define PAST_DIFF_TICK ( -2*24*60*60*1000 )

/// Adjusts the tick value to a valid tick_diff range.
/// Returns false if the tick is invalid.
static
bool adjust_tick(unsigned int* tick)
{
	int diff;
	
	if( tick == NULL )
		return false;

	diff = DIFF_TICK(*tick, gettick());
	if( diff <= FUTURE_DIFF_TICK || diff > MAX_DIFF_TICK )
	{
		ShowWarning("adjust_tick: tick diff too far in the future %d, adjusting to the maximum %d\n", diff, MAX_DIFF_TICK);
		*tick -= (diff - MAX_DIFF_TICK);
	}
	else if( diff < PAST_DIFF_TICK )
	{
		return false;
	}
	else if( diff < MIN_DIFF_TICK )
	{
		ShowWarning("adjust_tick: tick diff too far in the past %d, adjusting to the minimm %d\n", diff, MIN_DIFF_TICK);
		*tick += (diff - MAX_DIFF_TICK);
	}
	return true;
}

/// Releases a timer.
static
void release_timer(int tid)
{
	if( timer_data[tid].type == 0 )
		return;// already released

	memset(&timer_data[tid], 0, sizeof(struct TimerData));
	if( free_timer_list_num >= free_timer_list_max )
	{
		free_timer_list_max += 256;
		if( free_timer_list )
			RECREATE(free_timer_list, int, free_timer_list_max);
		else
			CREATE(free_timer_list, int, free_timer_list_max);
		memset(free_timer_list + (free_timer_list_max - 256), 0, sizeof(int)*256);
	}
	free_timer_list[free_timer_list_num++] = tid;
}

/// Returns a free timer id.
static int acquire_timer(void)
{
	int tid;

	// select a free timer
	tid = timer_data_num;
	while( free_timer_list_num )
	{
		int pos = --free_timer_list_num;
		if( free_timer_list[pos] < timer_data_num && timer_data[free_timer_list[pos]].type == 0 )
		{// freed and released
			tid = free_timer_list[pos];
			break;
		}
	}

	// check available space
	if( tid >= timer_data_max )
	{
		timer_data_max += 256;
		if( timer_data )
			RECREATE(timer_data, struct TimerData, timer_data_max);
		else
			CREATE(timer_data, struct TimerData, timer_data_max);
		memset(timer_data + (timer_data_max - 256), 0, sizeof(struct TimerData)*256);
	}

	if( tid >= timer_data_num )
		timer_data_num = tid + 1;

	return tid;
}

/// Starts a new timer that is deleted once it expires (single-use).
/// Returns the timer's id.
int add_timer(unsigned int tick, TimerFunc func, int id, intptr data)
{
	int tid;

	if( !adjust_tick(&tick) )
	{
		ShowError("add_timer: tick out of range (tick=%u %08x[%s] id=%d data=%d diff_tick=%d)\n", tick, (int)func, search_timer_func_list(func), id, data, DIFF_TICK(tick, gettick()));
		return INVALID_TIMER;
	}
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

	if( interval < 1 )
	{
		ShowError("add_timer_interval: invalid interval (tick=%u %08x[%s] id=%d data=%d diff_tick=%d)\n", tick, (int)func, search_timer_func_list(func), id, data, DIFF_TICK(tick, gettick()));
		return INVALID_TIMER;
	}
	if( !adjust_tick(&tick) )
	{
		ShowError("add_timer_interval: tick out of range (tick=%u %08x[%s] id=%d data=%d diff_tick=%d)\n", tick, (int)func, search_timer_func_list(func), id, data, DIFF_TICK(tick, gettick()));
		return INVALID_TIMER;
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
const struct TimerData* get_timer(int tid)
{
	if( tid >= 0 && tid < timer_data_num && timer_data[tid].type != 0 )
		return &timer_data[tid];
	return NULL;
}

/// Marks a timer specified by 'id' for immediate deletion once it expires.
/// Param 'func' is used for debug/verification purposes.
/// Returns 0 on success, < 0 on failure.
int delete_timer(int tid, TimerFunc func)
{
	if( tid < 0 || tid >= timer_data_num || timer_data[tid].type == 0 )
	{
		ShowError("delete_timer error : no such timer %d (%08x(%s))\n", tid, (int)func, search_timer_func_list(func));
		return -1;
	}
	if( timer_data[tid].func != func )
	{
		ShowError("delete_timer error : function mismatch %08x(%s) != %08x(%s)\n", (int)timer_data[tid].func, search_timer_func_list(timer_data[tid].func), (int)func, search_timer_func_list(func));
		return -2;
	}

	if( timer_data[tid].type&TIMER_REMOVE_HEAP )
		// timer func being executed, make sure it's marked for removal when it ends
		timer_data[tid].type = TIMER_FORCE_REMOVE|TIMER_REMOVE_HEAP;
	else if( pop_timer_heap(tid) )
		release_timer(tid);
	else if( (timer_data[tid].type|TIMER_REMOVE_HEAP) == 0 )
		ShowDebug("delete_timer: failed to remove timer %d (%08x(%s), type=%d)\n", tid, (int)func, search_timer_func_list(func), timer_data[tid].type);
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
	if( tid < 0 || tid >= timer_data_num || timer_data[tid].type == 0 )
	{
		ShowError("settick_timer error : no such timer %d\n", tid);
		return -1;
	}
	if( timer_data[tid].tick == tick )
		return tick;

	if( !adjust_tick(&tick) )
	{
		ShowError("settick_timer: tick out of range, leaving timer unmodified (tid=%d tick=%u %08x[%s] diff_tick=%d)\n", tid, tick, (int)timer_data[tid].func, search_timer_func_list(timer_data[tid].func), DIFF_TICK(tick, gettick()));
		return -1;
	}
	pop_timer_heap(tid);
	if( tick == -1 )
		tick = 0;// -1 is reserved for error
	timer_data[tid].type &= ~TIMER_REMOVE_HEAP;
	timer_data[tid].tick = tick;
	push_timer_heap(tid);
	return tick;
}

/// Executes all expired timers.
/// Returns the value of the smallest non-expired timer (or 1 second if there aren't any).
int do_timer(unsigned int tick)
{
	int diff = 1000; // return value

	// process all timers one by one
	while( timer_heap_num )
	{
		int tid = timer_heap[0]; // first element in heap (smallest tick)

		diff = DIFF_TICK(timer_data[tid].tick, tick);
		if( diff > 0 )
			break; // no more expired timers to process

		pop_timer_heap(tid);

		// mark timer as 'to be removed'
		timer_data[tid].type |= TIMER_REMOVE_HEAP;

		if( timer_data[tid].func )
		{
			if( diff < -1000 )
				// 1秒以上の大幅な遅延が発生しているので、
				// timer処理タイミングを現在値とする事で
				// 呼び出し時タイミング(引数のtick)相対で処理してる
				// timer関数の次回処理タイミングを遅らせる
				timer_data[tid].func(tid, tick, timer_data[tid].id, timer_data[tid].data);
			else
				timer_data[tid].func(tid, timer_data[tid].tick, timer_data[tid].id, timer_data[tid].data);
		}

		// in the case the function didn't change anything...
		if( timer_data[tid].type & TIMER_REMOVE_HEAP || timer_data[tid].type == TIMER_FORCE_REMOVE )
		{
			timer_data[tid].type &= ~TIMER_REMOVE_HEAP;

			switch( timer_data[tid].type )
			{
			case TIMER_FORCE_REMOVE:
			case TIMER_ONCE_AUTODEL:
				release_timer(tid);
				break;
			case TIMER_INTERVAL:
				if( DIFF_TICK(timer_data[tid].tick, tick) < -1000 )
					timer_data[tid].tick = tick + timer_data[tid].interval;
				else
					timer_data[tid].tick += timer_data[tid].interval;
				push_timer_heap(tid);
				break;
			}
		}
	}

	if( diff < TIMER_MIN_INTERVAL )
		return TIMER_MIN_INTERVAL;
	if( diff > TIMER_MAX_INTERVAL )
		return TIMER_MAX_INTERVAL;
	return diff;
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
