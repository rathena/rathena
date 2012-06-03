
#include "../common/core.h"
#include "../common/atomic.h"
#include "../common/thread.h"
#include "../common/spinlock.h"
#include "../common/showmsg.h"

#include <stdio.h>
#include <stdlib.h>

// 
// Simple test for the spinlock implementation to see if it works properly..  
//



#define THRC 32 //thread Count 
#define PERINC 100000
#define LOOPS 47


static SPIN_LOCK lock;
static int val = 0;
static volatile int32 done_threads = 0;

static  void *worker(void *p){
	register int i;
	
	for(i = 0; i < PERINC; i++){
		EnterSpinLock(&lock);
		EnterSpinLock(&lock);
		
		val++;
		
		LeaveSpinLock(&lock);
		LeaveSpinLock(&lock);
	}
	
	InterlockedIncrement(&done_threads);
	
	return NULL;
}//end: worker()


int do_init(int argc, char **argv){
	rAthread t[THRC];
	int j, i;
	int ok;
	
	ShowStatus("==========\n");
	ShowStatus("TEST: %u Runs,  (%u Threads)\n", LOOPS, THRC);
	ShowStatus("This can take a while\n");
	ShowStatus("\n\n");
	
	ok =0;
	for(j = 0; j < LOOPS; j++){
		val = 0;
		done_threads = 0;
		
		InitializeSpinLock(&lock);
		

		for(i =0; i < THRC; i++){
			t[i] = rathread_createEx( worker,  NULL,  1024*512,  RAT_PRIO_NORMAL);
		}
		
		
		while(1){
			if(InterlockedCompareExchange(&done_threads, THRC, THRC) == THRC) 
				break;
			
			rathread_yield();
		}
		
		FinalizeSpinLock(&lock);
		
		// Everything fine?
		if(val != (THRC*PERINC) ){
			printf("FAILED! (Result: %u, Expected: %u)\n",  val,  (THRC*PERINC) );
		}else{
			printf("OK! (Result: %u, Expected: %u)\n", val, (THRC*PERINC) );
			ok++;
		}

	}
	

	if(ok != LOOPS){
		ShowFatalError("Test failed.\n");		
		exit(1);
	}else{
		ShowStatus("Test passed.\n");
		exit(0);
	}


return 0;
}//end: do_init()


void do_abort(){
}//end: do_abort()


void set_server_type(){
	SERVER_TYPE = ATHENA_SERVER_NONE;
}//end: set_server_type()


void do_final(){
}//end: do_final()


int parse_console(const char* command){
	return 0;
}//end: parse_console

