// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <sys/types.h>

#ifdef __WIN32
#define __USE_W32_SOCKETS
// Well, this won't last another 30++ years (where conversion will truncate).
//#define _USE_32BIT_TIME_T	// use 32 bit time variables on 64bit windows
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "timer.h"

// タイマー間隔の最小値。モンスターの大量召還時、多数のクライアント接続時に
// サーバーが反応しなくなる場合は、TIMER_MIN_INTERVAL を増やしてください。

// If the server shows no reaction when processing thousands of monsters
// or connected by many clients, please increase TIMER_MIN_INTERVAL.

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
// searches for the target tick's position and stores it in pos (binary search)
#define HEAP_SEARCH(target,from,to,pos) \
	do { \
		int max,pivot; \
		pos = from; \
		max = to; \
		while (pos < max) { \
			pivot = (pos + max) / 2; \
			if (DIFF_TICK(target, timer_data[timer_heap[pivot]].tick) < 0) \
				pos = pivot + 1; \
			else \
				max = pivot; \
		} \
	} while(0)

// for debug
struct timer_func_list {
	struct timer_func_list* next;
	TimerFunc func;
	char* name;
};
static struct timer_func_list* tfl_root = NULL;

time_t start_time;

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
static unsigned int gettick_cache;
static int gettick_count;

unsigned int gettick_nocache(void)
{
#ifdef _WIN32
	gettick_count = 256;
	return gettick_cache = GetTickCount();
#else
	struct timeval tval;

	gettimeofday(&tval, NULL);
	gettick_count = 256;

	return gettick_cache = tval.tv_sec * 1000 + tval.tv_usec / 1000;
#endif
}

unsigned int gettick(void)
{
	if (--gettick_count < 0)
		return gettick_nocache();

	return gettick_cache;
}

/*======================================
 * 	CORE : Timer Heap
 *--------------------------------------
 */
/// Adds a timer to the timer_heap
static void push_timer_heap(int tid)
{
	unsigned int tick;
	int pos;
	int i;

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
	tick = timer_data[tid].tick; // speed up
	// with less than 4 values, it's speeder to use simple loop
	if (timer_heap_num < 4) {
		for(i = timer_heap_num; i > 0; i--)
		{
//			if (j < timer_data[timer_heap[i - 1]].tick) //Plain comparisons break on bound looping timers. [Skotlex]
			if (DIFF_TICK(tick, timer_data[timer_heap[i - 1]].tick) < 0)
				break;
			else
				timer_heap[i] = timer_heap[i - 1];
		}
		timer_heap[i] = tid;
	// searching by dichotomy (binary search)
	} else {
		// if lower actual item is higher than new
//		if (j < timer_data[timer_heap[timer_heap_num - 1]].tick) //Plain comparisons break on bound looping timers. [Skotlex]
		if (DIFF_TICK(tick, timer_data[timer_heap[timer_heap_num - 1]].tick) < 0)
			timer_heap[timer_heap_num] = tid;
		else {
			// searching position
			HEAP_SEARCH(tick,0,timer_heap_num-1,pos);
			// move elements - do loop if there are a little number of elements to move
			if (timer_heap_num - pos < 5) {
				for(i = timer_heap_num; i > pos; i--)
					timer_heap[i] = timer_heap[i - 1];
			// move elements - else use memmove (speeder for a lot of elements)
			} else
				memmove(&timer_heap[pos + 1], &timer_heap[pos], sizeof(int) * (timer_heap_num - pos));
			// save new element
			timer_heap[pos] = tid;
		}
	}

	timer_heap_num++;
}

/*==========================
 * 	Timer Management
 *--------------------------
 */

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

int add_timer(unsigned int tick,TimerFunc func, int id, int data)
{
	int tid = acquire_timer();

	timer_data[tid].tick	= tick;
	timer_data[tid].func	= func;
	timer_data[tid].id		= id;
	timer_data[tid].data	= data;
	timer_data[tid].type	= TIMER_ONCE_AUTODEL;
	timer_data[tid].interval = 1000;
	push_timer_heap(tid);

	return tid;
}

int add_timer_interval(unsigned int tick, TimerFunc func, int id, int data, int interval)
{
	int tid;

	if (interval < 1) {
		ShowError("add_timer_interval : function %08x(%s) has invalid interval %d!\n",
			 (int)func, search_timer_func_list(func), interval);
		return -1;
	}
	
	tid = acquire_timer();
	timer_data[tid].tick	= tick;
	timer_data[tid].func	= func;
	timer_data[tid].id		= id;
	timer_data[tid].data	= data;
	timer_data[tid].type	= TIMER_INTERVAL;
	timer_data[tid].interval = interval;
	push_timer_heap(tid);

	return tid;
}

int delete_timer(int id, TimerFunc func)
{
	if (id <= 0 || id >= timer_data_num) {
		ShowError("delete_timer error : no such timer %d (%08x(%s))\n", id, (int)func, search_timer_func_list(func));
		return -1;
	}
	if (timer_data[id].func != func) {
		ShowError("delete_timer error : function mismatch %08x(%s) != %08x(%s)\n",
			 (int)timer_data[id].func, search_timer_func_list(timer_data[id].func),
			 (int)func, search_timer_func_list(func));
		return -2;
	}
	// そのうち消えるにまかせる
	timer_data[id].func = NULL;
	timer_data[id].type = TIMER_ONCE_AUTODEL;

	return 0;
}

int addtick_timer(int tid, unsigned int tick)
{
	// Doesn't adjust the timer position. Might be the root of the FIXME in settick_timer. [FlavioJS]
	//return timer_data[tid].tick += tick;
	return settick_timer(tid, timer_data[tid].tick+tick);
}

//Sets the tick at which the timer triggers directly (meant as a replacement of delete_timer + add_timer) [Skotlex]
//FIXME: DON'T use this function yet, it is not correctly reorganizing the timer stack causing unexpected problems later on!
int settick_timer(int tid, unsigned int tick)
{
	int old_pos,pos;
	unsigned int old_tick;
	
	old_tick = timer_data[tid].tick;
	if( old_tick == tick )
		return tick;

	//FIXME: This search is not all that effective... there doesn't seems to be a better way to locate an element in the heap.
	//for(i = timer_heap_num-1; i >= 0 && timer_heap[i] != tid; i--);

	// search old_tick position
	HEAP_SEARCH(old_tick,0,timer_heap_num-1,old_pos);
	while( timer_heap[old_pos] != tid )
	{// skip timers with the same tick
		if( DIFF_TICK(old_tick,timer_data[timer_heap[old_pos]].tick) != 0 )
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

struct TimerData* get_timer(int tid)
{
	return &timer_data[tid];
}

//Correcting the heap when the tick overflows is an idea taken from jA to
//prevent timer problems. Thanks to [End of Exam] for providing the required data. [Skotlex]
//This funtion will rearrange the heap and assign new tick values.
static void fix_timer_heap(unsigned int tick)
{
	if (timer_heap_num >= 0 && tick < 0x00010000 && timer_data[timer_heap[0]].tick > 0xf0000000)
	{	//The last timer is way too far into the future, and the current tick is too close to 0, overflow was very likely
		//(not perfect, but will work as long as the timer is not expected to happen 50 or so days into the future)
		int i;
		int *tmp_heap;
		for (i=0; i < timer_heap_num && timer_data[timer_heap[i]].tick > 0xf0000000; i++)
		{	//All functions with high tick value should had been executed already...
			timer_data[timer_heap[i]].tick = 0;
		}
		//Move elements to readjust the heap.
		tmp_heap = aCalloc(sizeof(int), i);
		memcpy(tmp_heap, timer_heap, i*sizeof(int));
		memmove(timer_heap, &timer_heap[i], (timer_heap_num-i)*sizeof(int));
		memmove(&timer_heap[timer_heap_num-i], tmp_heap, i*sizeof(int));
		aFree(tmp_heap);
	}
}

int do_timer(unsigned int tick)
{
	static int fix_heap_flag = 0; //Flag for fixing the stack only once per tick loop. May not be the best way, but it's all I can think of currently :X [Skotlex]
	int i, nextmin = 1000;

	if (tick < 0x010000 && fix_heap_flag)
	{
		fix_timer_heap(tick);
		fix_heap_flag = 0;
	}

	while(timer_heap_num) {
		i = timer_heap[timer_heap_num - 1]; // next shorter element
		if ((nextmin = DIFF_TICK(timer_data[i].tick, tick)) > 0)
			break;

		--timer_heap_num; // suppress the actual element from the table
		timer_data[i].type |= TIMER_REMOVE_HEAP;
		if (timer_data[i].func) {
			if (nextmin < -1000) {
				// 1秒以上の大幅な遅延が発生しているので、
				// timer処理タイミングを現在値とする事で
				// 呼び出し時タイミング(引数のtick)相対で処理してる
				// timer関数の次回処理タイミングを遅らせる
				timer_data[i].func(i, tick, timer_data[i].id, timer_data[i].data);
			} else {
				timer_data[i].func(i, timer_data[i].tick, timer_data[i].id, timer_data[i].data);
			}
		}
		if (timer_data[i].type & TIMER_REMOVE_HEAP) {
			switch(timer_data[i].type & ~TIMER_REMOVE_HEAP) {
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

	if (nextmin < TIMER_MIN_INTERVAL)
		nextmin = TIMER_MIN_INTERVAL;

	if (UINT_MAX - nextmin < tick) //Tick will loop, rearrange the heap on the next iteration.
		fix_heap_flag = 1;
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
