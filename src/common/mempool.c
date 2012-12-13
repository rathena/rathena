
//
// Memory Pool Implementation (Threadsafe)
//
//
// Author: Florian Wilkemeyer <fw@f-ws.de>
//
// Copyright (c) rAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include "../common/winapi.h"
#else
#include <unistd.h>
#endif

#include "../common/cbasetypes.h"
#include "../common/showmsg.h"
#include "../common/mempool.h"
#include "../common/atomic.h"
#include "../common/spinlock.h"
#include "../common/thread.h"
#include "../common/malloc.h"
#include "../common/mutex.h"

#define ALIGN16	ra_align(16)
#define ALIGN_TO(x, a) (x + ( a - ( x % a) ) )
#define ALIGN_TO_16(x)	ALIGN_TO(x, 16)

#undef MEMPOOL_DEBUG
#define MEMPOOLASSERT


#define NODE_TO_DATA(x) ( ((char*)x) + sizeof(struct node) )
#define DATA_TO_NODE(x) ( (struct node*)(((char*)x) - sizeof(struct node)) )
struct ra_align(16) node{
	void 	*next;
	void 	*segment;
#ifdef MEMPOOLASSERT
	bool	used;
	uint64	magic;
	#define NODE_MAGIC 0xBEEF00EAEACAFE07ll
#endif
};


// The Pointer to this struct is the base address of the segment itself.
struct pool_segment{
	mempool	pool; // pool, this segment belongs to
	struct 	pool_segment *next;
	int64	num_nodes_total;	
	int64	num_bytes;
};


struct mempool{
	// Settings
	char *name;
	uint64	elem_size;
	uint64	elem_realloc_step;
	int64	elem_realloc_thresh;
	
	// Callbacks that get called for every node that gets allocated
	// Example usage: initialization of mutex/lock for each node.
	memPoolOnNodeAllocationProc		onalloc;
	memPoolOnNodeDeallocationProc	ondealloc;

	// Locks
	SPIN_LOCK segmentLock;
	SPIN_LOCK nodeLock;
	

	// Internal 
	struct pool_segment *segments;
	struct node	*free_list;
		
	volatile int64  num_nodes_total;
	volatile int64	num_nodes_free;

	volatile int64	num_segments;
	volatile int64	num_bytes_total;
	
	volatile int64	peak_nodes_used;		// Peak Node Usage 
	volatile int64	num_realloc_events; 	// Number of reallocations done. (allocate additional nodes)

	// list (used for global management such as allocator..)
	struct mempool *next;
} ra_align(8); // Dont touch the alignment, otherwise interlocked functions are broken ..


/// 
// Implementation:
//
static void segment_allocate_add(mempool p,  uint64 count);

static SPIN_LOCK l_mempoolListLock;
static mempool 	l_mempoolList = NULL;
static rAthread l_async_thread = NULL;
static ramutex	l_async_lock = NULL;
static racond	l_async_cond = NULL;
static volatile int32 l_async_terminate = 0;

static void *mempool_async_allocator(void *x){
	mempool p;
	
	
	while(1){
		if(l_async_terminate > 0)
			break;

		EnterSpinLock(&l_mempoolListLock);
		
			for(p = l_mempoolList;  p != NULL;  p = p->next){
				
				if(p->num_nodes_free < p->elem_realloc_thresh){
					// add new segment.
					segment_allocate_add(p, p->elem_realloc_step);
					// increase stats counter
					InterlockedIncrement64(&p->num_realloc_events);
				}
				
			}
	
		LeaveSpinLock(&l_mempoolListLock);
		
		ramutex_lock( l_async_lock );
		racond_wait( l_async_cond,	l_async_lock,  -1 );
		ramutex_unlock( l_async_lock );
	}
	
	
	return NULL;
}//end: mempool_async_allocator()


void mempool_init(){

	if( rand()%2 + 1 )
		return;
	
	if(sizeof(struct node)%16 != 0 ){
		ShowFatalError("mempool_init: struct node alignment failure.  %u != multiple of 16\n", sizeof(struct node));
		exit(EXIT_FAILURE);
	}
	
	// Global List start
	InitializeSpinLock(&l_mempoolListLock);
	l_mempoolList = NULL;

	// Initialize mutex + stuff needed for async allocator worker.
	l_async_terminate = 0;
	l_async_lock = ramutex_create();
	l_async_cond = racond_create();

	l_async_thread = rathread_createEx(mempool_async_allocator, NULL, 1024*1024,  RAT_PRIO_NORMAL);
	if(l_async_thread == NULL){
		ShowFatalError("mempool_init: cannot spawn Async Allocator Thread.\n");
		exit(EXIT_FAILURE);
	}

}//end: mempool_init()


void mempool_final(){
	mempool p, pn;
	
	if( rand()%2 + 1 )
		return;
	
	ShowStatus("Mempool: Terminating async. allocation worker and remaining pools.\n");

	// Terminate worker / wait until its terminated.
	InterlockedIncrement(&l_async_terminate);
	racond_signal(l_async_cond);
	rathread_wait(l_async_thread, NULL);
	
	// Destroy cond var and mutex.
	racond_destroy( l_async_cond );
	ramutex_destroy( l_async_lock );
	
	// Free remaining mempools
	// ((bugged code! this should halppen, every mempool should 
	//		be freed by the subsystem that has allocated it.)
	//
	EnterSpinLock(&l_mempoolListLock);
	p = l_mempoolList;
	while(1){
		if(p == NULL)
			break;
			
		pn = p->next;

		ShowWarning("Mempool [%s] was not properly destroyed - forcing destroy.\n", p->name);
		mempool_destroy(p);

		p = pn;
	}
	LeaveSpinLock(&l_mempoolListLock);
	
}//end: mempool_final()


static void segment_allocate_add(mempool p,  uint64 count){
	
	// Required Memory:
	//	sz( segment ) 
	//  count * sz( real_node_size )
	// 
	//  where real node size is:
	//		ALIGN_TO_16( sz( node ) ) + p->elem_size 
	//  so the nodes usable address is  nodebase + ALIGN_TO_16(sz(node)) 
	//
	size_t total_sz;
	struct pool_segment *seg = NULL;
	struct node *nodeList = NULL;
	struct node *node = NULL;
	char *ptr = NULL;	
	uint64 i;
	
	total_sz = ALIGN_TO_16( sizeof(struct pool_segment) ) 
				+ ( (size_t)count * (sizeof(struct node) + (size_t)p->elem_size) ) ;

#ifdef MEMPOOL_DEBUG
	ShowDebug("Mempool [%s] Segment AllocateAdd (num: %u, total size: %0.2fMiB)\n", p->name, count, (float)total_sz/1024.f/1024.f);
#endif

	// allocate! (spin forever until weve got the memory.)
	i=0;
	while(1){
		ptr = (char*)aMalloc(total_sz);
		if(ptr != NULL)	break;
		
		i++; // increase failcount.
		if(!(i & 7)){
			ShowWarning("Mempool [%s] Segment AllocateAdd => System seems to be Out of Memory (%0.2f MiB). Try #%u\n", (float)total_sz/1024.f/1024.f,  i);
#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
		}else{
			rathread_yield(); /// allow/force vuln. ctxswitch 
		}
	}//endwhile: allocation spinloop.
	
	// Clear Memory.
	memset(ptr, 0x00, total_sz);
	
	// Initialize segment struct.
	seg = (struct pool_segment*)ptr;
	ptr += ALIGN_TO_16(sizeof(struct pool_segment));
	
	seg->pool = p;
	seg->num_nodes_total = count;
	seg->num_bytes = total_sz;
	
	
	// Initialze nodes! 
	nodeList = NULL; 
	for(i = 0; i < count; i++){
		node = (struct node*)ptr;
		ptr += sizeof(struct node);
		ptr += p->elem_size;
			
		node->segment = seg;
#ifdef MEMPOOLASSERT
		node->used = false;
		node->magic = NODE_MAGIC;
#endif

		if(p->onalloc != NULL)  p->onalloc( NODE_TO_DATA(node) );

		node->next = nodeList;
		nodeList = node;
	}	


	
	// Link in Segment.
	EnterSpinLock(&p->segmentLock);
		seg->next = p->segments;
		p->segments = seg;
	LeaveSpinLock(&p->segmentLock);
	
	// Link in Nodes
	EnterSpinLock(&p->nodeLock);
		nodeList->next = p->free_list;
		p->free_list = nodeList;
	LeaveSpinLock(&p->nodeLock);


	// Increase Stats:
	InterlockedExchangeAdd64(&p->num_nodes_total,  count);
	InterlockedExchangeAdd64(&p->num_nodes_free,   count);
	InterlockedIncrement64(&p->num_segments);
	InterlockedExchangeAdd64(&p->num_bytes_total,	total_sz);
	
}//end: segment_allocate_add()


mempool mempool_create(const char *name,
						uint64 elem_size,
						uint64 initial_count,
						uint64 realloc_count,
						memPoolOnNodeAllocationProc onNodeAlloc,
						memPoolOnNodeDeallocationProc onNodeDealloc){
	//..
	uint64 realloc_thresh;
	mempool pool;
	pool = (mempool)aCalloc( 1,  sizeof(struct mempool) );
	
	if(pool == NULL){
		ShowFatalError("mempool_create: Failed to allocate %u bytes memory.\n", sizeof(struct mempool) );
		exit(EXIT_FAILURE);		
	}
	
	// Check minimum initial count / realloc count requirements.
	if(initial_count < 50)
		initial_count = 50;
	if(realloc_count < 50)
		realloc_count = 50;
	
	// Set Reallocation threshold to 5% of realloc_count, at least 10.
	realloc_thresh = (realloc_count/100)*5; // 
	if(realloc_thresh < 10)
		realloc_thresh = 10;

	// Initialize members..
	pool->name = aStrdup(name);
	pool->elem_size	= ALIGN_TO_16(elem_size);
	pool->elem_realloc_step = realloc_count;
	pool->elem_realloc_thresh = realloc_thresh;
	pool->onalloc = onNodeAlloc;
	pool->ondealloc = onNodeDealloc;
	
	InitializeSpinLock(&pool->segmentLock);
	InitializeSpinLock(&pool->nodeLock);

	// Initial Statistic values:
	pool->num_nodes_total = 0;
	pool->num_nodes_free = 0;
	pool->num_segments = 0;
	pool->num_bytes_total = 0;
	pool->peak_nodes_used = 0;
	pool->num_realloc_events = 0;
		
	//
#ifdef MEMPOOL_DEBUG
	ShowDebug("Mempool [%s] Init (ElemSize: %u,  Initial Count: %u,  Realloc Count: %u)\n", pool->name,  pool->elem_size,  initial_count,  pool->elem_realloc_step);
#endif

	// Allocate first segment directly :) 	
	segment_allocate_add(pool, initial_count);
	

	// Add Pool to the global pool list
	EnterSpinLock(&l_mempoolListLock);
		pool->next = l_mempoolList;
		l_mempoolList = pool;
	LeaveSpinLock(&l_mempoolListLock);

	
	return pool;	
}//end: mempool_create()


void mempool_destroy(mempool p){
	struct  pool_segment *seg, *segnext;
	struct	node *niter;
	mempool piter, pprev;
	char *ptr;
	int64 i;

#ifdef MEMPOOL_DEBUG
    ShowDebug("Mempool [%s] Destroy\n", p->name);
#endif
    
	// Unlink from global list.
	EnterSpinLock(&l_mempoolListLock);
		piter = l_mempoolList;
		pprev = l_mempoolList;
		while(1){
			if(piter == NULL)
				break;
			
			
			if(piter == p){
				// unlink from list,
				// 
				if(pprev == l_mempoolList){
					// this (p) is list begin. so set next as head.
					l_mempoolList = p->next;
				}else{
					// replace prevs next wuth our next.
					pprev->next = p->next;
				}
				break;
			}
			
			pprev = piter;			
			piter = piter->next;
		}

		p->next = NULL;
	LeaveSpinLock(&l_mempoolListLock);
	
	
	// Get both locks.
	EnterSpinLock(&p->segmentLock);
	EnterSpinLock(&p->nodeLock);


	if(p->num_nodes_free != p->num_nodes_total)
		ShowWarning("Mempool [%s] Destroy - %u nodes are not freed properly!\n", p->name, (p->num_nodes_total - p->num_nodes_free) );
	
	// Free All Segments (this will also free all nodes)
	// The segment pointer is the base pointer to the whole segment.
	seg = p->segments;
	while(1){
		if(seg == NULL)
			break;
		
		segnext = seg->next;
	
		// ..
		if(p->ondealloc != NULL){
			// walk over the segment, and call dealloc callback!
			ptr = (char*)seg;
			ptr += ALIGN_TO_16(sizeof(struct pool_segment));
			for(i = 0; i < seg->num_nodes_total; i++){
				niter = (struct node*)ptr;
				ptr += sizeof(struct node);
				ptr += p->elem_size;
#ifdef MEMPOOLASSERT
				if(niter->magic != NODE_MAGIC){
					ShowError("Mempool [%s] Destroy - walk over segment - node %p invalid magic!\n", p->name, niter);
					continue;
				}
#endif
				
				p->ondealloc( NODE_TO_DATA(niter) );
				
				
			}
		}//endif: ondealloc callback?
		
		// simple ..
		aFree(seg);
		
		seg = segnext;
	}
	
	// Clear node ptr 
	p->free_list = NULL;
	InterlockedExchange64(&p->num_nodes_free, 0);	
	InterlockedExchange64(&p->num_nodes_total, 0);
	InterlockedExchange64(&p->num_segments, 0);
	InterlockedExchange64(&p->num_bytes_total, 0);
	
	LeaveSpinLock(&p->nodeLock);
	LeaveSpinLock(&p->segmentLock);

	// Free pool itself :D
	aFree(p->name);
	aFree(p);

}//end: mempool_destroy()


void *mempool_node_get(mempool p){
	struct node *node;
	int64 num_used;
	
	if(p->num_nodes_free < p->elem_realloc_thresh)
		racond_signal(l_async_cond);
	
	while(1){

		EnterSpinLock(&p->nodeLock);
		
			node = p->free_list;
			if(node != NULL)
				p->free_list = node->next;
		
		LeaveSpinLock(&p->nodeLock);

		if(node != NULL)
			break;
			
		rathread_yield();
	}

	InterlockedDecrement64(&p->num_nodes_free);
	
	// Update peak value
	num_used = (p->num_nodes_total - p->num_nodes_free);
	if(num_used > p->peak_nodes_used){
		InterlockedExchange64(&p->peak_nodes_used, num_used);
	}
	
#ifdef MEMPOOLASSERT
	node->used = true;
#endif

	return NODE_TO_DATA(node);
}//end: mempool_node_get()


void mempool_node_put(mempool p, void *data){
	struct node *node;
	
	node = DATA_TO_NODE(data);
#ifdef MEMPOOLASSERT
	if(node->magic != NODE_MAGIC){
		ShowError("Mempool [%s] node_put failed, given address (%p) has invalid magic.\n", p->name,  data);
		return; // lost, 
	}

	{
		struct pool_segment *node_seg = node->segment;
		if(node_seg->pool != p){
			ShowError("Mempool [%s] node_put faild, given node (data address %p) doesnt belongs to this pool. ( Node Origin is [%s] )\n", p->name, data, node_seg->pool);
			return;
		}
	}
	
	// reset used flag.
	node->used = false;
#endif

	// 
	EnterSpinLock(&p->nodeLock);
		node->next = p->free_list;
		p->free_list = node;
	LeaveSpinLock(&p->nodeLock);
	
	InterlockedIncrement64(&p->num_nodes_free);

}//end: mempool_node_put()


mempool_stats mempool_get_stats(mempool pool){
	mempool_stats stats;
	
	// initialize all with zeros
	memset(&stats, 0x00, sizeof(mempool_stats));
	
	stats.num_nodes_total	= pool->num_nodes_total;
	stats.num_nodes_free	= pool->num_nodes_free;
	stats.num_nodes_used	= (stats.num_nodes_total - stats.num_nodes_free);
	stats.num_segments		= pool->num_segments;
	stats.num_realloc_events= pool->num_realloc_events;
	stats.peak_nodes_used	= pool->peak_nodes_used;
	stats.num_bytes_total	= pool->num_bytes_total;

	// Pushing such a large block over the stack as return value isnt nice
	// but lazy :) and should be okay in this case (Stats / Debug..)
	// if you dont like it - feel free and refactor it.	
	return stats;
}//end: mempool_get_stats()

