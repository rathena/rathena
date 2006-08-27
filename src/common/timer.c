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

#include "timer.h"
#include "../common/malloc.h"
#include "showmsg.h"

// タイマー間隔の最小値。モンスターの大量召還時、多数のクライアント接続時に
// サーバーが反応しなくなる場合は、TIMER_MIN_INTERVAL を増やしてください。

// If the server shows no reaction when processing thousands of monsters
// or connected by many clients, please increase TIMER_MIN_INTERVAL.

#define TIMER_MIN_INTERVAL 50

static struct TimerData* timer_data	= NULL;
static int timer_data_max	= 0;
static int timer_data_num	= 0;

static int* free_timer_list		= NULL;
static int free_timer_list_max	= 0;
static int free_timer_list_pos	= 0;

static int timer_heap_num = 0;
static int timer_heap_max = 0;
static int* timer_heap = NULL;

static int fix_heap_flag =0; //Flag for fixing the stack only once per tick loop. May not be the best way, but it's all I can think of currently :X [Skotlex]

// for debug
struct timer_func_list {
	int (*func)(int,unsigned int,int,int);
	struct timer_func_list* next;
	char* name;
};
static struct timer_func_list* tfl_root;

time_t start_time;

#ifdef __WIN32
/* Modified struct timezone to void - we pass NULL anyway */
void gettimeofday (struct timeval *t, void *dummy)
{
	DWORD millisec = GetTickCount();

	t->tv_sec = (int) (millisec / 1000);
	t->tv_usec = (millisec % 1000) * 1000;
}
#endif

//
int add_timer_func_list(int (*func)(int,unsigned int,int,int), char* name)
{
	struct timer_func_list* tfl;

	if (name) {
		tfl = (struct timer_func_list*) aCalloc (sizeof(struct timer_func_list), 1);
		tfl->name = (char *) aMalloc (strlen(name) + 1);

		tfl->next = tfl_root;
		tfl->func = func;
		strcpy(tfl->name, name);
		tfl_root = tfl;
	}
	return 0;
}

char* search_timer_func_list(int (*func)(int,unsigned int,int,int))
{
	struct timer_func_list* tfl = tfl_root;
	while (tfl) {
		if (func == tfl->func)
			return tfl->name;
		tfl = tfl->next;
	}

	return "unknown timer function";
}

/*----------------------------
 * 	Get tick time
 *----------------------------*/
static unsigned int gettick_cache;
static int gettick_count;

unsigned int gettick_nocache(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);
	gettick_count = 256;

	return gettick_cache = tval.tv_sec * 1000 + tval.tv_usec / 1000;
}

unsigned int gettick(void)
{
	gettick_count--;
	if (gettick_count < 0)
		return gettick_nocache();

	return gettick_cache;
}

/*======================================
 * 	CORE : Timer Heap
 *--------------------------------------
 */
static void push_timer_heap(int index)
{
	int i, j;
	int min, max, pivot; // for sorting

	// check number of element
	if (timer_heap_num >= timer_heap_max) {
		if (timer_heap_max == 0) {
			timer_heap_max = 256;
			timer_heap = (int *) aCalloc( sizeof(int) , 256);
		} else {
			timer_heap_max += 256;
			timer_heap = (int *) aRealloc( timer_heap, sizeof(int) * timer_heap_max);
			malloc_tsetdword(timer_heap + (timer_heap_max - 256), 0, sizeof(int) * 256);
		}
	}

	// do a sorting from higher to lower
	j = timer_data[index].tick; // speed up
	// with less than 4 values, it's speeder to use simple loop
	if (timer_heap_num < 4) {
		for(i = timer_heap_num; i > 0; i--)
//			if (j < timer_data[timer_heap[i - 1]].tick) //Plain comparisons break on bound looping timers. [Skotlex]
			if (DIFF_TICK(j, timer_data[timer_heap[i - 1]].tick) < 0)
				break;
			else
				timer_heap[i] = timer_heap[i - 1];
		timer_heap[i] = index;
	// searching by dichotomie
	} else {
		// if lower actual item is higher than new
//		if (j < timer_data[timer_heap[timer_heap_num - 1]].tick) //Plain comparisons break on bound looping timers. [Skotlex]
		if (DIFF_TICK(j, timer_data[timer_heap[timer_heap_num - 1]].tick) < 0)
			timer_heap[timer_heap_num] = index;
		else {
			// searching position
			min = 0;
			max = timer_heap_num - 1;
			while (min < max) {
				pivot = (min + max) / 2;
//				if (j < timer_data[timer_heap[pivot]].tick) //Plain comparisons break on bound looping timers. [Skotlex]
				if (DIFF_TICK(j, timer_data[timer_heap[pivot]].tick) < 0)
					min = pivot + 1;
				else
					max = pivot;
			}
			// move elements - do loop if there are a little number of elements to move
			if (timer_heap_num - min < 5) {
				for(i = timer_heap_num; i > min; i--)
					timer_heap[i] = timer_heap[i - 1];
			// move elements - else use memmove (speeder for a lot of elements)
			} else
				memmove(&timer_heap[min + 1], &timer_heap[min], sizeof(int) * (timer_heap_num - min));
			// save new element
			timer_heap[min] = index;
		}
	}

	timer_heap_num++;
}

/*==========================
 * 	Timer Management
 *--------------------------
 */

int acquire_timer (void)
{
	int i;

	if (free_timer_list_pos) {
		do {
			i = free_timer_list[--free_timer_list_pos];
		} while(i >= timer_data_num && free_timer_list_pos > 0);
	} else
		i = timer_data_num;

	if (i >= timer_data_num)
		for (i = timer_data_num; i < timer_data_max && timer_data[i].type; i++);
	if (i >= timer_data_num && i >= timer_data_max) {
		if (timer_data_max == 0) {
			timer_data_max = 256;
			timer_data = (struct TimerData*) aCalloc( sizeof(struct TimerData) , timer_data_max);
		} else {
			timer_data_max += 256;
			timer_data = (struct TimerData *) aRealloc( timer_data, sizeof(struct TimerData) * timer_data_max);
			malloc_tsetdword(timer_data + (timer_data_max - 256), 0, sizeof(struct TimerData) * 256);
		}
	}

	return i;
}

int add_timer(unsigned int tick,int (*func)(int,unsigned int,int,int), int id, int data)
{
	int tid = acquire_timer();

	timer_data[tid].tick	= tick;
	timer_data[tid].func	= func;
	timer_data[tid].id		= id;
	timer_data[tid].data	= data;
	timer_data[tid].type	= TIMER_ONCE_AUTODEL;
	timer_data[tid].interval = 1000;
	push_timer_heap(tid);

	if (tid >= timer_data_num)
		timer_data_num = tid + 1;

	return tid;
}

int add_timer_interval(unsigned int tick, int (*func)(int,unsigned int,int,int), int id, int data, int interval)
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

	if (tid >= timer_data_num)
		timer_data_num = tid + 1;

	return tid;
}

int delete_timer(int id, int (*func)(int,unsigned int,int,int))
{
	if (id <= 0 || id >= timer_data_num) {
		ShowError("delete_timer error : no such timer %d\n", id);
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
	return timer_data[tid].tick += tick;
}

//Sets the tick at which the timer triggers directly (meant as a replacement of delete_timer + add_timer) [Skotlex]
//FIXME: DON'T use this function yet, it is not correctly reorganizing the timer stack causing unexpected problems later on!
int settick_timer(int tid, unsigned int tick)
{
	int i,j;
	if (timer_data[tid].tick == tick)
		return tick;

	//FIXME: This search is not all that effective... there doesn't seems to be a better way to locate an element in the heap.
	for(i = timer_heap_num-1; i >= 0 && timer_heap[i] != tid; i--);

	if (i < 0)
		return -1; //Sort of impossible, isn't it?
	if (DIFF_TICK(timer_data[tid].tick, tick) > 0)
	{	//Timer is accelerated, shift timer near the end of the heap.
		if (i == timer_heap_num-1) //Nothing to shift.
			j = timer_heap_num-1;
		else {
			for (j = i+1; j < timer_heap_num && DIFF_TICK(timer_data[j].tick, tick) > 0; j++);
			j--;
			memmove(&timer_heap[i], &timer_heap[i+1], (j-i)*sizeof(int));
		}
	} else {	//Timer is delayed, shift timer near the beginning of the heap.
		if (i == 0) //Nothing to shift.
			j = 0;
		else {
			for (j = i-1; j >= 0 && DIFF_TICK(timer_data[j].tick, tick) < 0; j--);
			j++;
			memmove(&timer_heap[j+1], &timer_heap[j], (i-j)*sizeof(int));
		}
	}
	timer_heap[j] = tid;
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
		memmove(&tmp_heap[0], &timer_heap[0], i*sizeof(int));
		memmove(&timer_heap[0], &timer_heap[i], (timer_heap_num-i)*sizeof(int));
		memmove(&timer_heap[timer_heap_num-i], &tmp_heap[0], i*sizeof(int));
		aFree(tmp_heap);
	}
}

int do_timer(unsigned int tick)
{
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
		if (timer_heap_num > 0) // suppress the actual element from the table
			timer_heap_num--;
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
					free_timer_list = (int *) aRealloc(free_timer_list, sizeof(int) * free_timer_list_max);
					malloc_tsetdword(free_timer_list + (free_timer_list_max - 256), 0, 256 * sizeof(int));
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

	if ((unsigned int)(tick + nextmin) < tick) //Tick will loop, rearrange the heap on the next iteration.
		fix_heap_flag = 1;
	return nextmin;
}

unsigned long get_uptime (void)
{
	return (unsigned long) difftime (time(NULL), start_time);
}

void timer_init(void)
{
	time(&start_time);
}

void timer_final(void)
{
	struct timer_func_list* tfl = tfl_root, *tfl2;

	while (tfl) {
		tfl2 = tfl->next;	// copy next pointer
		aFree(tfl->name);	// free structures
		aFree(tfl);
		tfl = tfl2;			// use copied pointer for next cycle
	}
	
	if (timer_data) aFree(timer_data);
	if (timer_heap) aFree(timer_heap);
	if (free_timer_list) aFree(free_timer_list);
}

