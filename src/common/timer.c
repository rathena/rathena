// $Id: timer.c,v 1.1.1.1 2004/09/10 17:44:49 MagicalTux Exp $
// original : core.c 2003/02/26 18:03:12 Rev 1.7

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#endif

#include "timer.h"
#include "utils.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

// If the server shows no reaction when processing thousands of monsters

// or connected by many clients, please increase TIMER_MIN_INTERVEL.

#define TIMER_MIN_INTERVEL 50

static struct TimerData* timer_data;
static int timer_data_max,timer_data_num;
static int* free_timer_list;
static int free_timer_list_max, free_timer_list_pos;

static int timer_heap_max=0; //fixed Shinomori from eA forums
static int* timer_heap = NULL;

// for debug
struct timer_func_list {
	int (*func)(int,unsigned int,int,int);
	struct timer_func_list* next;
	char* name;
};
static struct timer_func_list* tfl_root;

#if defined(_WIN32)
void gettimeofday(struct timeval *t, struct timezone *dummy)
{
  DWORD millisec = GetTickCount();

  t->tv_sec = (int) (millisec / 1000);
  t->tv_usec = (millisec % 1000) * 1000;
}

#endif


//
int add_timer_func_list(int (*func)(int,unsigned int,int,int),char* name)
{
	struct timer_func_list* tfl;

	CREATE(tfl, struct timer_func_list, 1);
	CREATE(tfl->name, char, strlen(name) + 1);

	tfl->next = tfl_root;
	tfl->func = func;
	strcpy(tfl->name,name);
	tfl_root = tfl;

	return 0;
}

char* search_timer_func_list(int (*func)(int,unsigned int,int,int))
{
	struct timer_func_list* tfl;
	for(tfl = tfl_root;tfl;tfl = tfl->next) {
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
unsigned int gettick_nocache(void)
{
	struct timeval tval;
	gettimeofday(&tval,NULL);
	gettick_count = 256;
	return gettick_cache = tval.tv_sec * 1000 + tval.tv_usec/1000;
}

unsigned int gettick(void)
{
	gettick_count--;
	if (gettick_count<0)
		return gettick_nocache();
	return gettick_cache;
}

/*======================================
 * 	CORE : Timer Heap
 *--------------------------------------
 */
static void push_timer_heap(int index)
{
	int i, h;

	if (timer_heap == NULL || timer_heap[0] + 1 >= timer_heap_max) {
		int first = timer_heap == NULL;

		timer_heap_max += 256;
		RECREATE(timer_heap, int, timer_heap_max);
		memset(timer_heap + (timer_heap_max - 256), 0, sizeof(int) * 256);
		if (first)
			timer_heap[0] = 0;
	}

	timer_heap[0]++;

	for (h = timer_heap[0]-1, i = (h - 1) / 2;
		 h > 0 && DIFF_TICK(timer_data[index].tick,
			timer_data[timer_heap[i + 1]].tick) < 0;
		 i = (h - 1) / 2) {
		timer_heap[h + 1] = timer_heap[i + 1];
		h = i;
	}
	timer_heap[h + 1] = index;
}

static int top_timer_heap()
{
	if (timer_heap == NULL || timer_heap[0] <= 0)
		return -1;

	return timer_heap[1];
}

static int pop_timer_heap()
{
	int i,h,k;
	int ret,last;

	if (timer_heap == NULL || timer_heap[0] <= 0)
		return -1;
	ret = timer_heap[1];
	last = timer_heap[timer_heap[0]];
	timer_heap[0]--;

	for(h = 0,k = 2;k<timer_heap[0];k = k * 2 + 2) {
		if (DIFF_TICK(timer_data[timer_heap[k + 1]].tick , timer_data[timer_heap[k]].tick)>0)
			k--;
		timer_heap[h + 1] = timer_heap[k + 1], h = k;
	}
	if (k == timer_heap[0])
		timer_heap[h + 1] = timer_heap[k], h = k-1;

	for(i = (h-1)/2;
		h>0 && DIFF_TICK(timer_data[timer_heap[i + 1]].tick , timer_data[last].tick)>0;
		i = (h-1)/2) {
		timer_heap[h + 1] = timer_heap[i + 1],h = i;
	}
	timer_heap[h + 1] = last;

	return ret;
}

int add_timer(unsigned int tick,int (*func)(int,unsigned int,int,int),int id,int data)
{
	struct TimerData* td;
	int i;

	if (free_timer_list_pos) {
		do {
			i = free_timer_list[--free_timer_list_pos];
		} while(i >= timer_data_num && free_timer_list_pos > 0);
	} else
		i = timer_data_num;
	if (i >= timer_data_num)
		for (i = timer_data_num;i<timer_data_max && timer_data[i].type; i++);
	if (i >= timer_data_num && i >= timer_data_max) {
		int j;
		if (timer_data_max == 0) {
			timer_data_max = 256;
			CREATE(timer_data, struct TimerData, timer_data_max);
		} else {
			timer_data_max += 256;
			RECREATE(timer_data, struct TimerData, timer_data_max);
			if (timer_data == NULL) {
				printf("out of memory : add_timer timer_data\n");
				exit(1);
			}
			memset(timer_data + (timer_data_max - 256), 0,
				sizeof(struct TimerData) * 256);
		}
		for(j = timer_data_max-256;j<timer_data_max; j++)
			timer_data[j].type = 0;
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

int add_timer_interval(unsigned int tick,int (*func)(int,unsigned int,int,int),int id,int data,int interval)
{
	int tid;
	tid = add_timer(tick,func,id,data);
	timer_data[tid].type = TIMER_INTERVAL;
	timer_data[tid].interval = interval;
	return tid;
}

int delete_timer(int id,int (*func)(int,unsigned int,int,int))
{
	if (id <= 0 || id >= timer_data_num) {
		printf("delete_timer error : no such timer %d\n", id);
		return -1;
	}
	if (timer_data[id].func != func) {
		printf("delete_timer error : function dismatch %08x(%s) != %08x(%s)\n",
			 (int)timer_data[id].func,
				search_timer_func_list(timer_data[id].func),
			 (int)func,
				search_timer_func_list(func));
		return -2;
	}
	// そのうち消えるにまかせる
	timer_data[id].func = NULL;
	timer_data[id].type = TIMER_ONCE_AUTODEL;
	timer_data[id].tick -= 60 * 60 * 1000;
	return 0;
}

int addtick_timer(int tid,unsigned int tick)
{
	return timer_data[tid].tick += tick;
}
struct TimerData* get_timer(int tid)
{
	return &timer_data[tid];
}


int do_timer(unsigned int tick)
{
	int i,nextmin = 1000;

#if 0
	static int disp_tick = 0;
	if (DIFF_TICK(disp_tick,tick)<-5000 || DIFF_TICK(disp_tick,tick)>5000) {
		printf("timer %d(%d + %d)\n",timer_data_num,timer_heap[0],free_timer_list_pos);
		disp_tick = tick;
	}
#endif

	while((i = top_timer_heap()) >= 0) {
		if (DIFF_TICK(timer_data[i].tick , tick)>0) {
			nextmin = DIFF_TICK(timer_data[i].tick , tick);
			break;
		}
		pop_timer_heap();
		timer_data[i].type |= TIMER_REMOVE_HEAP;
		if (timer_data[i].func) {
			if (DIFF_TICK(timer_data[i].tick , tick) < -1000) {
				// 1秒以上の大幅な遅延が発生しているので、
				// timer処理タイミングを現在値とする事で
				// 呼び出し時タイミング(引数のtick)相対で処理してる
				// timer関数の次回処理タイミングを遅らせる
				timer_data[i].func(i,tick,timer_data[i].id,timer_data[i].data);
			} else {
				timer_data[i].func(i,timer_data[i].tick,timer_data[i].id,timer_data[i].data);
			}
		}
		if (timer_data[i].type&TIMER_REMOVE_HEAP) {
			switch(timer_data[i].type & ~TIMER_REMOVE_HEAP) {
			case TIMER_ONCE_AUTODEL:
				timer_data[i].type = 0;
				if (free_timer_list_pos >= free_timer_list_max) {
					free_timer_list_max += 256;
					RECREATE(free_timer_list, int, free_timer_list_max);
					memset(free_timer_list + (free_timer_list_max - 256), 0,
						256 * sizeof(free_timer_list[0]));
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

	if(nextmin < TIMER_MIN_INTERVEL)

		nextmin = TIMER_MIN_INTERVEL;
	return nextmin;
}

void timer_final() 
{
    free(timer_data);
}
