// $Id: timer.c,v 1.1.1.1 2004/09/10 17:44:49 Yor Exp $
// original : core.c 2003/02/26 18:03:12 Rev 1.7

//#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef __WIN32
#define __USE_W32_SOCKETS
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#endif

#include "timer.h"
#include "malloc.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

static struct TimerData* timer_data;
static int timer_data_max, timer_data_num;
static int* free_timer_list;
static int free_timer_list_max, free_timer_list_pos;

static int timer_heap_num = 0, timer_heap_max = 0;
static int* timer_heap = NULL;

// for debug
struct timer_func_list {
	int (*func)(int,unsigned int,int,int);
	struct timer_func_list* next;
	char* name;
};
static struct timer_func_list* tfl_root;

#ifdef __WIN32
/* Modified struct timezone to void - we pass NULL anyway */
void gettimeofday(struct timeval *t, void *dummy) {
	DWORD millisec = GetTickCount();

	t->tv_sec = (int) (millisec / 1000);
	t->tv_usec = (millisec % 1000) * 1000;
}
#endif

//
int add_timer_func_list(int (*func)(int,unsigned int,int,int), char* name) {
	struct timer_func_list* tfl;

	//CALLOC(tfl, struct timer_func_list, 1);
	tfl = (struct timer_func_list*) aCalloc( sizeof(struct timer_func_list) , 1);
	//MALLOC(tfl->name, char, strlen(name) + 1);
	tfl->name = (char *) aMalloc( strlen(name) + 1 );

	tfl->next = tfl_root;
	tfl->func = func;
	strcpy(tfl->name, name);
	tfl_root = tfl;

	return 0;
}

char* search_timer_func_list(int (*func)(int,unsigned int,int,int)) {
	struct timer_func_list* tfl;

	for(tfl = tfl_root; tfl; tfl = tfl->next) {
		if (func == tfl->func)
			return tfl->name;
	}
	return "???";
}

/*----------------------------
 * 	Get tick time
 *----------------------------*/
static unsigned int gettick_cache;
static int gettick_count;

unsigned int gettick_nocache(void) {
	struct timeval tval;

	gettimeofday(&tval, NULL);
	gettick_count = 256;

	return gettick_cache = tval.tv_sec * 1000 + tval.tv_usec / 1000;
}

unsigned int gettick(void) {
	gettick_count--;
	if (gettick_count < 0)
		return gettick_nocache();

	return gettick_cache;
}

/*======================================
 * 	CORE : Timer Heap
 *--------------------------------------
 */
static void push_timer_heap(int index) {
	int i, j;
	int min, max, pivot; // for sorting

	// check number of element
	if (timer_heap_num >= timer_heap_max) {
		if (timer_heap_max == 0) {
			timer_heap_max = 256;
			//CALLOC(timer_heap, int, 256);
			timer_heap = (int *) aCalloc( sizeof(int) , 256);
		} else {
			timer_heap_max += 256;
			//REALLOC(timer_heap, int, timer_heap_max);
			timer_heap = (int *) aRealloc( timer_heap, sizeof(int) * timer_heap_max);
			memset(timer_heap + (timer_heap_max - 256), 0, sizeof(int) * 256);
		}
	}

	// do a sorting from higher to lower
	j = timer_data[index].tick; // speed up
	// with less than 4 values, it's speeder to use simple loop
	if (timer_heap_num < 4) {
		for(i = timer_heap_num; i > 0; i--)
			if (j < timer_data[timer_heap[i - 1]].tick)
				break;
			else
				timer_heap[i] = timer_heap[i - 1];
		timer_heap[i] = index;
	// searching by dichotomie
	} else {
		// if lower actual item is higher than new
		if (j < timer_data[timer_heap[timer_heap_num - 1]].tick)
			timer_heap[timer_heap_num] = index;
		else {
			// searching position
			min = 0;
			max = timer_heap_num - 1;
			while (min < max) {
				pivot = (min + max) / 2;
				if (j < timer_data[timer_heap[pivot]].tick)
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

int add_timer(unsigned int tick,int (*func)(int,unsigned int,int,int),int id,int data) {
	struct TimerData* td;
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
			//CALLOC(timer_data, struct TimerData, timer_data_max);
			timer_data = (struct TimerData*) aCalloc( sizeof(struct TimerData) , timer_data_max);
		} else {
			timer_data_max += 256;
			//REALLOC(timer_data, struct TimerData, timer_data_max);
			timer_data = (struct TimerData *) aRealloc( timer_data, sizeof(struct TimerData) * timer_data_max);
			memset(timer_data + (timer_data_max - 256), 0, sizeof(struct TimerData) * 256);
		}
	}
	td = &timer_data[i];
	td->tick = tick;
	td->func = func;
	td->id = id;
	td->data = data;
	td->type = TIMER_ONCE_AUTODEL;
	td->interval = 1000;
	push_timer_heap(i);
	if (i >= timer_data_num)
		timer_data_num = i + 1;

	return i;
}

int add_timer_interval(unsigned int tick,int (*func)(int,unsigned int,int,int),int id,int data,int interval) {
	int tid;

	tid = add_timer(tick,func,id,data);
	timer_data[tid].type = TIMER_INTERVAL;
	timer_data[tid].interval = interval;

	return tid;
}

int delete_timer(int id,int (*func)(int,unsigned int,int,int)) {
	if (id <= 0 || id >= timer_data_num) {
		printf("delete_timer error : no such timer %d\n", id);
		return -1;
	}
	if (timer_data[id].func != func) {
		printf("delete_timer error : function dismatch %08x(%s) != %08x(%s)\n",
			 (int)timer_data[id].func, search_timer_func_list(timer_data[id].func),
			 (int)func, search_timer_func_list(func));
		return -2;
	}
	// そのうち消えるにまかせる
	timer_data[id].func = NULL;
	timer_data[id].type = TIMER_ONCE_AUTODEL;
//	timer_data[id].tick -= 60 * 60 * 1000;

	return 0;
}

int addtick_timer(int tid,unsigned int tick) {
	return timer_data[tid].tick += tick;
}

struct TimerData* get_timer(int tid) {
	return &timer_data[tid];
}

int do_timer(unsigned int tick) {
	int i, nextmin = 1000;

	while(timer_heap_num) {
		i = timer_heap[timer_heap_num - 1]; // next shorter element
		if (DIFF_TICK(timer_data[i].tick, tick) > 0) {
			nextmin = DIFF_TICK(timer_data[i].tick, tick);
			break;
		}
		if (timer_heap_num > 0) // suppress the actual element from the table
			timer_heap_num--;
		timer_data[i].type |= TIMER_REMOVE_HEAP;
		if (timer_data[i].func) {
			if (DIFF_TICK(timer_data[i].tick, tick) < -1000) {
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
					//REALLOC(free_timer_list, int, free_timer_list_max);
					free_timer_list = (int *) aRealloc(free_timer_list, sizeof(int) * free_timer_list_max);
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

	if (nextmin < 10)
		nextmin = 10;

	return nextmin;
}

void timer_final() {
	struct timer_func_list* tfl = tfl_root, *tfl2;

//	while (tfl) {
//		tfl2 = tfl;
//		aFree(tfl->name);
//		aFree(tfl);
//		tfl = tfl2->next; // access on already freed memory
//	}

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

