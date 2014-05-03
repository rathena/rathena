// Copyright (c) rAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifdef WIN32
#include "../common/winapi.h"
#else
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#endif

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/timer.h"
#include "../common/mutex.h"

struct ramutex{
#ifdef WIN32
	CRITICAL_SECTION hMutex;
#else
	pthread_mutex_t hMutex;
#endif
};


struct racond{
#ifdef WIN32
	HANDLE events[2];
	ra_align(8) volatile LONG nWaiters;
	CRITICAL_SECTION waiters_lock;

#define EVENT_COND_SIGNAL 0
#define EVENT_COND_BROADCAST 1

#else
	pthread_cond_t hCond;
#endif
};


////////////////////
// Mutex
//
// Implementation:
//


ramutex ramutex_create(){
	struct ramutex *m;
	
	m = (struct ramutex*)aMalloc( sizeof(struct ramutex) );
	if(m == NULL){
		ShowFatalError("ramutex_create: OOM while allocating %u bytes.\n", sizeof(struct ramutex));
		return NULL;
	}
	
#ifdef WIN32
	InitializeCriticalSection(&m->hMutex);
#else
	pthread_mutex_init(&m->hMutex, NULL);
#endif
		
	return m;
}//end: ramutex_create()


void ramutex_destroy( ramutex m ){

#ifdef WIN32
	DeleteCriticalSection(&m->hMutex);
#else
	pthread_mutex_destroy(&m->hMutex);
#endif

	aFree(m);

}//end: ramutex_destroy()


void ramutex_lock( ramutex m ){

#ifdef WIN32
	EnterCriticalSection(&m->hMutex);
#else
	pthread_mutex_lock(&m->hMutex);
#endif
}//end: ramutex_lock


bool ramutex_trylock( ramutex m ){
#ifdef WIN32
	if(TryEnterCriticalSection(&m->hMutex) == TRUE)
		return true;

	return false;
#else
	if(pthread_mutex_trylock(&m->hMutex) == 0)
		return true;
	
	return false;
#endif
}//end: ramutex_trylock()


void ramutex_unlock( ramutex m ){
#ifdef WIN32
	LeaveCriticalSection(&m->hMutex);
#else
	pthread_mutex_unlock(&m->hMutex);
#endif

}//end: ramutex_unlock()



///////////////
// Condition Variables
// 
// Implementation:
//

racond racond_create(){
	struct racond *c;
	
	c = (struct racond*)aMalloc( sizeof(struct racond) );
	if(c == NULL){
		ShowFatalError("racond_create: OOM while allocating %u bytes\n", sizeof(struct racond));
		return NULL;
	}

#ifdef WIN32
	c->nWaiters = 0;
	c->events[ EVENT_COND_SIGNAL ]		= CreateEvent( NULL,  FALSE,  FALSE,  NULL );
	c->events[ EVENT_COND_BROADCAST ]	= CreateEvent( NULL,  TRUE,   FALSE,  NULL );
	InitializeCriticalSection( &c->waiters_lock );
#else
	pthread_cond_init(&c->hCond, NULL);
#endif
	
	return c;
}//end: racond_create()


void racond_destroy( racond c ){
#ifdef WIN32
	CloseHandle( c->events[ EVENT_COND_SIGNAL ] );
	CloseHandle( c->events[ EVENT_COND_BROADCAST ] );
	DeleteCriticalSection( &c->waiters_lock );
#else
	pthread_cond_destroy(&c->hCond);
#endif

	aFree(c);
}//end: racond_destroy()


void racond_wait( racond c,  ramutex m,  sysint timeout_ticks){
#ifdef WIN32
	register DWORD ms;
	int result;
	bool is_last = false;


	EnterCriticalSection(&c->waiters_lock);
	c->nWaiters++;
	LeaveCriticalSection(&c->waiters_lock);

	if(timeout_ticks < 0)
		ms = INFINITE;
	else
		ms = (timeout_ticks > MAXDWORD) ? (MAXDWORD - 1) : (DWORD)timeout_ticks;
		
	
	// we can release the mutex (m) here, cause win's
	// manual reset events maintain state when used with
	// SetEvent()
	ramutex_unlock(m);

	result = WaitForMultipleObjects(2, c->events, FALSE, ms);
	
	
	EnterCriticalSection(&c->waiters_lock);
	c->nWaiters--;
	if( (result == WAIT_OBJECT_0 + EVENT_COND_BROADCAST) && (c->nWaiters == 0) )
		is_last = true; // Broadcast called!
	LeaveCriticalSection(&c->waiters_lock);

	

	// we are the last waiter that has to be notified, or to stop waiting
	// so we have to do a manual reset
	if(is_last == true)
		ResetEvent( c->events[EVENT_COND_BROADCAST] );


	ramutex_lock(m);

#else
	if(timeout_ticks < 0){
		pthread_cond_wait( &c->hCond,  &m->hMutex );
	}else{
		struct timespec wtime;
		int64 exact_timeout = gettick() + timeout_ticks;
	
		wtime.tv_sec = exact_timeout/1000;
		wtime.tv_nsec = (exact_timeout%1000)*1000000;
		
		pthread_cond_timedwait( &c->hCond,  &m->hMutex,  &wtime);
	}

#endif
}//end: racond_wait()


void racond_signal( racond c ){
#ifdef WIN32
//	bool has_waiters = false;
//	EnterCriticalSection(&c->waiters_lock);
//	if(c->nWaiters > 0)
//			has_waiters = true;
//	LeaveCriticalSection(&c->waiters_lock);
	
//	if(has_waiters == true)
		SetEvent( c->events[ EVENT_COND_SIGNAL ] );
#else
	pthread_cond_signal(&c->hCond);
#endif
}//end: racond_signal()


void racond_broadcast( racond c ){
#ifdef WIN32
//	bool has_waiters = false;
//	EnterCriticalSection(&c->waiters_lock);
//	if(c->nWaiters > 0)
//			has_waiters = true;
//	LeaveCriticalSection(&c->waiters_lock);
	
//	if(has_waiters == true)
		SetEvent( c->events[ EVENT_COND_BROADCAST ] );
#else
	pthread_cond_broadcast(&c->hCond);
#endif
}//end: racond_broadcast()


