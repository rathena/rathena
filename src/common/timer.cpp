// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "timer.hpp"

#include <cstdlib>
#include <cstring>
#include <utility>

#include "cbasetypes.hpp"
#include "db.hpp"
#include "malloc.hpp"
#include "nullpo.hpp"
#include "showmsg.hpp"
#include "utils.hpp"
#ifdef WIN32
#include "winapi.hpp" // GetTickCount()
#endif

// If the server can't handle processing thousands of monsters
// or many connected clients, please increase TIMER_MIN_INTERVAL.
// The official interval of 20ms is however strongly recommended,
// as it is needed for perfect server-client syncing.
const t_tick TIMER_MIN_INTERVAL = 20;
const t_tick TIMER_MAX_INTERVAL = 1000;

// timers (array)
static struct TimerData* timer_data = nullptr;
static int timer_data_max = 0;
static int timer_data_num = 0;

// free timers (array)
static int* free_timer_list = nullptr;
static int free_timer_list_max = 0;
static int free_timer_list_pos = 0;


/// Comparator for the timer heap. (minimum tick at top)
/// Returns negative if tid1's tick is smaller, positive if tid2's tick is smaller, 0 if equal.
///
/// @param tid1 First timer
/// @param tid2 Second timer
/// @return negative if tid1 is top, positive if tid2 is top, 0 if equal
#define DIFFTICK_MINTOPCMP(tid1,tid2) DIFF_TICK(timer_data[tid1].tick,timer_data[tid2].tick)

// timer heap (binary heap of tid's)
static BHEAP_VAR(int, timer_heap);


// server startup time
time_t start_time;


/*----------------------------
 * 	Timer debugging
 *----------------------------*/
struct timer_func_list {
	struct timer_func_list* next;
	TimerFunc func;
	char* name;
} *tfl_root = nullptr;

/// Sets the name of a timer function.
int add_timer_func_list(TimerFunc func, const char* name)
{
	struct timer_func_list* tfl;

	if (name) {
		for( tfl=tfl_root; tfl != nullptr; tfl=tfl->next )
		{// check suspicious cases
			if( func == tfl->func )
				ShowWarning("add_timer_func_list: duplicating function %p(%s) as %s.\n",tfl->func,tfl->name,name);
			else if( strcmp(name,tfl->name) == 0 )
				ShowWarning("add_timer_func_list: function %p has the same name as %p(%s)\n",func,tfl->func,tfl->name);
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
const char* search_timer_func_list(TimerFunc func)
{
	struct timer_func_list* tfl;

	for( tfl=tfl_root; tfl != nullptr; tfl=tfl->next )
		if (func == tfl->func)
			return tfl->name;

	return "unknown timer function";
}

/*----------------------------
 * 	Get tick time
 *----------------------------*/

#if defined(ENABLE_RDTSC)
static uint64 RDTSC_BEGINTICK = 0,   RDTSC_CLOCK = 0;

static __inline uint64 _rdtsc(){
	register union{
		uint64	qw;
		uint32 	dw[2];
	} t;

	asm volatile("rdtsc":"=a"(t.dw[0]), "=d"(t.dw[1]) );

	return t.qw;
}

static void rdtsc_calibrate(){
	uint64 t1, t2;
	int32 i;

	ShowStatus("Calibrating Timer Source, please wait... ");

	RDTSC_CLOCK = 0;

	for(i = 0; i < 5; i++){
		t1 = _rdtsc();
		usleep(1000000); //1000 MS
		t2 = _rdtsc();
		RDTSC_CLOCK += (t2 - t1) / 1000;
	}
	RDTSC_CLOCK /= 5;

	RDTSC_BEGINTICK = _rdtsc();

	ShowMessage(" done. (Frequency: %u Mhz)\n", (uint32)(RDTSC_CLOCK/1000) );
}

#endif

/// platform-abstracted tick retrieval
static t_tick tick(void)
{
#if defined(WIN32)
#ifdef DEPRECATED_WINDOWS_SUPPORT
	return GetTickCount();
#else
	return GetTickCount64();
#endif
#elif defined(ENABLE_RDTSC)
	//
		return (unsigned int)((_rdtsc() - RDTSC_BEGINTICK) / RDTSC_CLOCK);
	//
#elif defined(HAVE_MONOTONIC_CLOCK)
	struct timespec tval;
	clock_gettime(CLOCK_MONOTONIC, &tval);
	return tval.tv_sec * 1000 + tval.tv_nsec / 1000000;
#else
	struct timeval tval;
	gettimeofday(&tval, nullptr);
	return tval.tv_sec * 1000 + tval.tv_usec / 1000;
#endif
}

//////////////////////////////////////////////////////////////////////////
#if defined(TICK_CACHE) && TICK_CACHE > 1
//////////////////////////////////////////////////////////////////////////
// tick is cached for TICK_CACHE calls
static t_tick gettick_cache;
static int gettick_count = 1;

unsigned int gettick_nocache(void)
{
	gettick_count = TICK_CACHE;
	gettick_cache = tick();
	return gettick_cache;
}

t_tick gettick(void)
{
	return ( --gettick_count == 0 ) ? gettick_nocache() : gettick_cache;
}
//////////////////////////////
#else
//////////////////////////////
// tick doesn't get cached
t_tick gettick_nocache(void)
{
	return tick();
}

t_tick gettick(void)
{
	return tick();
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////

/*======================================
 * 	CORE : Timer Heap
 *--------------------------------------*/

/// Adds a timer to the timer_heap
static void push_timer_heap(int tid)
{
	BHEAP_ENSURE(timer_heap, 1, 256);
	BHEAP_PUSH(timer_heap, tid, DIFFTICK_MINTOPCMP);
}

/*==========================
 * 	Timer Management
 *--------------------------*/

/// Returns a free timer id.
static int acquire_timer(void)
{
	int tid;

	// select a free timer
	if (free_timer_list_pos) {
		do {
			tid = free_timer_list[--free_timer_list_pos];
		} while(tid >= timer_data_num && free_timer_list_pos > 0);
	} else
		tid = timer_data_num;

	// check available space
	if( tid >= timer_data_num )
		for (tid = timer_data_num; tid < timer_data_max && timer_data[tid].type; tid++);
	if (tid >= timer_data_num && tid >= timer_data_max)
	{// expand timer array
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
int add_timer(t_tick tick, TimerFunc func, int id, intptr_t data)
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
/// Returns the timer's id, or INVALID_TIMER if it fails.
int add_timer_interval(t_tick tick, TimerFunc func, int id, intptr_t data, int interval)
{
	int tid;

	if( interval < 1 )
	{
		ShowError("add_timer_interval: invalid interval (tick=%" PRtf " %p[%s] id=%d data=%" PRIdPTR " diff_tick=%d)\n", tick, func, search_timer_func_list(func), id, data, DIFF_TICK(tick, gettick()));
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
	return ( tid >= 0 && tid < timer_data_num ) ? &timer_data[tid] : nullptr;
}

/// Marks a timer specified by 'id' for immediate deletion once it expires.
/// Param 'func' is used for debug/verification purposes.
/// Returns 0 on success, < 0 on failure.
int delete_timer(int tid, TimerFunc func)
{
	if( tid < 0 || tid >= timer_data_num )
	{
		ShowError("delete_timer error : no such timer %d (%p(%s))\n", tid, func, search_timer_func_list(func));
		return -1;
	}
	if( timer_data[tid].func != func )
	{
		ShowError("delete_timer error : function mismatch %p(%s) != %p(%s)\n", timer_data[tid].func, search_timer_func_list(timer_data[tid].func), func, search_timer_func_list(func));
		return -2;
	}

	timer_data[tid].func = nullptr;
	timer_data[tid].type = TIMER_ONCE_AUTODEL;

	return 0;
}

/// Adjusts a timer's expiration time.
/// Returns the new tick value, or -1 if it fails.
t_tick addtick_timer(int tid, t_tick tick)
{
	return settick_timer(tid, timer_data[tid].tick+tick);
}

/// Modifies a timer's expiration time (an alternative to deleting a timer and starting a new one).
/// Returns the new tick value, or -1 if it fails.
t_tick settick_timer(int tid, t_tick tick)
{
	size_t i;

	// search timer position
	ARR_FIND(0, BHEAP_LENGTH(timer_heap), i, BHEAP_DATA(timer_heap)[i] == tid);
	if( i == BHEAP_LENGTH(timer_heap) )
	{
		ShowError("settick_timer: no such timer %d (%p(%s))\n", tid, timer_data[tid].func, search_timer_func_list(timer_data[tid].func));
		return -1;
	}

	if( tick == -1 )
		tick = 0;// add 1ms to avoid the error value -1

	if( timer_data[tid].tick == tick )
		return tick;// nothing to do, already in propper position

	// pop and push adjusted timer
	BHEAP_POPINDEX(timer_heap, i, DIFFTICK_MINTOPCMP);
	timer_data[tid].tick = tick;
	BHEAP_PUSH(timer_heap, tid, DIFFTICK_MINTOPCMP);
	return tick;
}

/// Executes all expired timers.
/// Returns the value of the smallest non-expired timer (or 1 second if there aren't any).
t_tick do_timer(t_tick tick)
{
	t_tick diff = TIMER_MAX_INTERVAL; // return value

	// process all timers one by one
	while( BHEAP_LENGTH(timer_heap) )
	{
		int tid = BHEAP_PEEK(timer_heap);// top element in heap (smallest tick)

		diff = DIFF_TICK(timer_data[tid].tick, tick);
		if( diff > 0 )
			break; // no more expired timers to process

		// remove timer
		BHEAP_POP(timer_heap, DIFFTICK_MINTOPCMP);
		timer_data[tid].type |= TIMER_REMOVE_HEAP;

		if( timer_data[tid].func )
		{
			if( diff < -1000 )
				// timer was delayed for more than 1 second, use current tick instead
				timer_data[tid].func(tid, tick, timer_data[tid].id, timer_data[tid].data);
			else
				timer_data[tid].func(tid, timer_data[tid].tick, timer_data[tid].id, timer_data[tid].data);
		}

		// in the case the function didn't change anything...
		if( timer_data[tid].type & TIMER_REMOVE_HEAP )
		{
			timer_data[tid].type &= ~TIMER_REMOVE_HEAP;

			switch( timer_data[tid].type )
			{
			default:
			case TIMER_ONCE_AUTODEL:
				timer_data[tid].type = 0;
				if (free_timer_list_pos >= free_timer_list_max) {
					free_timer_list_max += 256;
					RECREATE(free_timer_list,int,free_timer_list_max);
					memset(free_timer_list + (free_timer_list_max - 256), 0, 256 * sizeof(int));
				}
				free_timer_list[free_timer_list_pos++] = tid;
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

	return cap_value(diff, TIMER_MIN_INTERVAL, TIMER_MAX_INTERVAL);
}

unsigned long get_uptime(void)
{
	return (unsigned long)difftime(time(nullptr), start_time);
}

/**
 * Converting a timestamp is a srintf according to format
 * safefr then strftime as it ensure \0 at end of string
 * @param str, pointer to the destination string
 * @param size, max length of the string
 * @param timestamp, see unix epoch
 * @param format, format to convert timestamp on, see strftime format
 * @return the string of timestamp
 */
const char* timestamp2string(char* str, size_t size, time_t timestamp, const char* format){
	size_t len = strftime(str, size, format, localtime(&timestamp));
	memset(str + len, '\0', size - len);
	return str;
}

/*
 * Split given timein into year, month, day, hour, minute, second
 */
void split_time(int timein, int* year, int* month, int* day, int* hour, int* minute, int *second) {
	const int factor_min = 60;
	const int factor_hour = factor_min*60;
	const int factor_day = factor_hour*24;
	const int factor_month = 2629743; // Approx  (30.44 days) 
	const int factor_year = 31556926; // Approx (365.24 days)

	*year = timein/factor_year;
	timein -= *year*factor_year;
	*month = timein/factor_month;
	timein -= *month*factor_month;
	*day = timein/factor_day;
	timein -= *day*factor_day;
	*hour = timein/factor_hour;
	timein -= *hour*factor_hour;
	*minute = timein/factor_min;
	timein -= *minute*factor_min;
	*second = timein;

	*year = max(0,*year);
	*month = max(0,*month);
	*day = max(0,*day);
	*hour = max(0,*hour);
	*minute = max(0,*minute);
	*second = max(0,*second);
}

/*
 * Create a "timestamp" with the given argument
 */
double solve_time(char* modif_p) {
	double totaltime = 0;
	struct tm then_tm;
	time_t now = time(nullptr);
	time_t then = now;
	then_tm = *localtime(&then);
	
	nullpo_retr(0,modif_p);

	while (modif_p[0] != '\0') {
		int value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 's') {
				then_tm.tm_sec += value;
				modif_p++;
			} else if (modif_p[0] == 'n') {
				then_tm.tm_min += value;
				modif_p++;
			} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
				then_tm.tm_min += value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				then_tm.tm_hour += value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				then_tm.tm_mday += value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				then_tm.tm_mon += value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				then_tm.tm_year += value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}
	then = mktime(&then_tm);
	totaltime = difftime(then,now);

	return totaltime;
}

void timer_init(void)
{
#if defined(ENABLE_RDTSC)
	rdtsc_calibrate();
#endif

	time(&start_time);
}

void timer_final(void)
{
	struct timer_func_list *tfl;
	struct timer_func_list *next;

	for( tfl=tfl_root; tfl != nullptr; tfl = next ) {
		next = tfl->next;	// copy next pointer
		aFree(tfl->name);	// free structures
		aFree(tfl);
	}

	if (timer_data) aFree(timer_data);
	BHEAP_CLEAR(timer_heap);
	if (free_timer_list) aFree(free_timer_list);
}
