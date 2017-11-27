// Copyright (c) rAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once 
#ifndef _rA_THREAD_H_
#define _rA_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cbasetypes.h"

typedef struct rAthread *prAthread;
typedef void* (*rAthreadProc)(void*);

typedef enum RATHREAD_PRIO {
	RAT_PRIO_LOW = 0,
	RAT_PRIO_NORMAL,
	RAT_PRIO_HIGH	
} RATHREAD_PRIO;

#define RA_INVALID_THID -1

/**
 * Get the handle of a specific thread 
 *
 * @param idx  -  The index of the thread
 * @return not NULL if success
 */
prAthread rathenat_getThread( int idx );

/**
 * Creates a new Thread
 *
 * @param entyPoint  - entryProc,
 * @param param - general purpose parameter, would be given as parameter to the thread's entrypoint.
 * 
 * @return not NULL if success
 */
prAthread rathread_create( rAthreadProc entryPoint,  void *param );


/** 
 * Creates a new Thread (with more creation options)
 *
 * @param entyPoint  - entryProc,
 * @param param - general purpose parameter, would be given as parameter to the thread's entrypoint
 * @param szStack - stack Size in bytes 
 * @param prio - Priority of the Thread @ OS Scheduler..
 *
 * @return not NULL if success
 */
prAthread rathread_createEx( rAthreadProc entryPoint,  void *param,  size_t szStack,  RATHREAD_PRIO prio );


/**
 * Destroys the given Thread immediatly
 *
 * @note The Handle gets invalid after call! dont use it afterwards. 
 *
 * @param handle - thread to destroy.
 */
void rathread_destroy ( prAthread handle );


/** 
 * Returns the thread handle of the thread calling this function
 * 
 * @note this wont work @ programms main thread
 * @note the underlying implementation might not perform very well, cache the value received! 
 * 
 * @return not NULL if success
 */
prAthread rathread_self( );


/**
 * Returns own thrad id (TID) 
 *
 * @note this is an unique identifier for the calling thread, and 
 *        depends on platfrom / compiler, and may not be the systems Thread ID! 
 *
 * @return -1 when fails, otherwise >= 0
 */
int rathread_get_tid();


/**
 * Waits for the given thread to terminate 
 *
 * @param handle - thread to wait (join) for
 * @param out_Exitcode - [OPTIONAL] - if given => Exitcode (value) of the given thread - if it's terminated
 * 
 * @return true - if the given thread has been terminated.
 */
bool rathread_wait( prAthread handle,  void* *out_exitCode );


/** 
 * Sets the given PRIORITY @ OS Task Scheduler
 * 
 * @param handle - thread to set prio for
 * @param rio - the priority (RAT_PRIO_LOW ... )
 */
void rathread_prio_set( prAthread handle, RATHREAD_PRIO prio );


/** 
 * Gets the current Prio of the given trhead
 *
 * @param handle - the thread to get the prio for.
 */
RATHREAD_PRIO rathread_prio_get( prAthread handle);


/**
 * Tells the OS scheduler to yield the execution of the calling thread
 * 
 * @note: this will not "pause" the thread,
 *			it just allows the OS to spent the remaining time 
 *			of the slice to another thread.
 */
void rathread_yield();



void rathread_init();
void rathread_final();

#ifdef __cplusplus
}
#endif

#endif
