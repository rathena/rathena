/*****************************************************************************\
 *  Copyright (c) rAthena Dev Teams - Licensed under GNU GPL                 *
 *  For more information, see LICENCE in the main folder                     *
 *                                                                           *
 *  <H1>Entry Reusage System</H1>                                            *
 *                                                                           *
 *  There are several root entry managers, each with a different entry size. *
 *  Each manager will keep track of how many instances have been 'created'.  *
 *  They will only automatically destroy themselves after the last instance  *
 *  is destroyed.                                                            *
 *                                                                           *
 *  Entries can be allocated from the managers.                              *
 *  If it has reusable entries (freed entry), it uses one.                   *
 *  So no assumption should be made about the data of the entry.             *
 *  Entries should be freed in the manager they where allocated from.        *
 *  Failure to do so can lead to unexpected behaviors.                      *
 *                                                                           *
 *  <H2>Advantages:</H2>                                                     *
 *  - The same manager is used for entries of the same size.                 *
 *    So entries freed in one instance of the manager can be used by other   *
 *    instances of the manager.                                              *
 *  - Much less memory allocation/deallocation - program will be faster.     *
 *  - Avoids memory fragmentation - program will run better for longer.       *
 *                                                                           *
 *  <H2>Disadvantages:</H2>                                                   *
 *  - Unused entries are almost inevitable - memory being wasted.            *
 *  - A  manager will only auto-destroy when all of its instances are        *
 *    destroyed so memory will usually only be recovered near the end.       *
 *  - Always wastes space for entries smaller than a pointer.                *
 *                                                                           *
 *  WARNING: The system is not thread-safe at the moment.                    *
 *                                                                           *
 *  HISTORY:                                                                 *
 *    0.1 - Initial version                                                  *
 *    1.0 - ERS Rework                                                       *
 *                                                                           *
 * @version 1.0 - ERS Rework                                                 *
 * @author GreenBox @ rAthena Project                                        *
 * @encoding US-ASCII                                                        *
 * @see common#ers.hpp                                                         *
\*****************************************************************************/

#include "ers.hpp"

#include <cstdlib>
#include <cstring>

#include "cbasetypes.hpp"
#include "malloc.hpp" // CREATE, RECREATE, aMalloc, aFree
#include "nullpo.hpp"
#include "showmsg.hpp" // ShowMessage, ShowError, ShowFatalError, CL_BOLD, CL_NORMAL

#ifndef DISABLE_ERS

#define ERS_BLOCK_ENTRIES 2048

struct ers_list
{
	struct ers_list *Next;
};

struct ers_instance_t;

typedef struct ers_cache
{
	// Allocated object size, including ers_list size
	uint32 ObjectSize;

	// Number of ers_instances referencing this
	int32 ReferenceCount;

	// Reuse linked list
	struct ers_list *ReuseList;

	// Memory blocks array
	unsigned char **Blocks;

	// Max number of blocks
	uint32 Max;

	// Free objects count
	uint32 Free;

	// Used blocks count
	uint32 Used;

	// Objects in-use count
	uint32 UsedObjs;

	// Default = ERS_BLOCK_ENTRIES, can be adjusted for performance for individual cache sizes.
	uint32 ChunkSize;

	// Misc options, some options are shared from the instance
	enum ERSOptions Options;

	// Linked list
	struct ers_cache *Next, *Prev;
} ers_cache_t;

struct ers_instance_t {
	// Interface to ERS
	struct eri VTable;

	// Name, used for debugging purposes
	char *Name;

	// Misc options
	enum ERSOptions Options;

	// Our cache
	ers_cache_t *Cache;

	// Count of objects in use, used for detecting memory leaks
	uint32 Count;

	struct ers_instance_t *Next, *Prev;
};


// Array containing a pointer for all ers_cache structures
static ers_cache_t *CacheList = nullptr;
static struct ers_instance_t *InstanceList = nullptr;

/**
 * @param Options the options from the instance seeking a cache, we use it to give it a cache with matching configuration
 **/
static ers_cache_t *ers_find_cache(uint32 size, enum ERSOptions Options) {
	ers_cache_t *cache;

	for (cache = CacheList; cache; cache = cache->Next)
		if ( cache->ObjectSize == size && cache->Options == ( Options & ERS_CACHE_OPTIONS ) )
			return cache;

	CREATE(cache, ers_cache_t, 1);
	cache->ObjectSize = size;
	cache->ReferenceCount = 0;
	cache->ReuseList = nullptr;
	cache->Blocks = nullptr;
	cache->Free = 0;
	cache->Used = 0;
	cache->UsedObjs = 0;
	cache->Max = 0;
	cache->ChunkSize = ERS_BLOCK_ENTRIES;
	cache->Options = (enum ERSOptions)(Options & ERS_CACHE_OPTIONS);

	if (CacheList == nullptr)
	{
		CacheList = cache;
	}
	else
	{
		cache->Next = CacheList;
		cache->Next->Prev = cache;
		CacheList = cache;
		CacheList->Prev = nullptr;
	}

	return cache;
}

static void ers_free_cache(ers_cache_t *cache, bool remove)
{
	uint32 i;

	for (i = 0; i < cache->Used; i++)
		aFree(cache->Blocks[i]);

	if (cache->Next)
		cache->Next->Prev = cache->Prev;

	if (cache->Prev)
		cache->Prev->Next = cache->Next;
	else
		CacheList = cache->Next;

	aFree(cache->Blocks);

	aFree(cache);
}

static void *ers_obj_alloc_entry(ERS *self)
{
	struct ers_instance_t *instance = (struct ers_instance_t *)self;
	void *ret;

	if (instance == nullptr) {
		ShowError("ers_obj_alloc_entry: nullptr object, aborting entry freeing.\n");
		return nullptr;
	}

	if (instance->Cache->ReuseList != nullptr) {
		ret = (void *)((unsigned char *)instance->Cache->ReuseList + sizeof(struct ers_list));
		instance->Cache->ReuseList = instance->Cache->ReuseList->Next;
	} else if (instance->Cache->Free > 0) {
		instance->Cache->Free--;
		ret = &instance->Cache->Blocks[instance->Cache->Used - 1][static_cast<size_t>( instance->Cache->Free ) * static_cast<size_t>( instance->Cache->ObjectSize ) + sizeof( struct ers_list )];
	} else {
		if (instance->Cache->Used == instance->Cache->Max) {
			instance->Cache->Max = (instance->Cache->Max * 4) + 3;
			RECREATE(instance->Cache->Blocks, unsigned char *, instance->Cache->Max);
		}

		CREATE(instance->Cache->Blocks[instance->Cache->Used], unsigned char, instance->Cache->ObjectSize * instance->Cache->ChunkSize);
		instance->Cache->Used++;

		instance->Cache->Free = instance->Cache->ChunkSize -1;
		ret = &instance->Cache->Blocks[instance->Cache->Used - 1][static_cast<size_t>( instance->Cache->Free ) * static_cast<size_t>( instance->Cache->ObjectSize ) + sizeof( struct ers_list )];
	}

	instance->Count++;
	instance->Cache->UsedObjs++;

	return ret;
}

static void ers_obj_free_entry(ERS *self, void *entry)
{
	struct ers_instance_t *instance = (struct ers_instance_t *)self;
	struct ers_list *reuse = (struct ers_list *)((unsigned char *)entry - sizeof(struct ers_list));

	if (instance == nullptr) {
		ShowError("ers_obj_free_entry: nullptr object, aborting entry freeing.\n");
		return;
	} else if (entry == nullptr) {
		ShowError("ers_obj_free_entry: nullptr entry, nothing to free.\n");
		return;
	}

	if( instance->Cache->Options & ERS_OPT_CLEAN )
		memset((unsigned char*)reuse + sizeof(struct ers_list), 0, instance->Cache->ObjectSize - sizeof(struct ers_list));

	reuse->Next = instance->Cache->ReuseList;
	instance->Cache->ReuseList = reuse;
	instance->Count--;
	instance->Cache->UsedObjs--;
}

static size_t ers_obj_entry_size(ERS *self)
{
	struct ers_instance_t *instance = (struct ers_instance_t *)self;

	if (instance == nullptr) {
		ShowError("ers_obj_entry_size: nullptr object, aborting entry freeing.\n");
		return 0;
	}

	return instance->Cache->ObjectSize;
}

static void ers_obj_destroy(ERS *self)
{
	struct ers_instance_t *instance = (struct ers_instance_t *)self;

	if (instance == nullptr) {
		ShowError("ers_obj_destroy: nullptr object, aborting entry freeing.\n");
		return;
	}

	if (instance->Count > 0)
		if (!(instance->Options & ERS_OPT_CLEAR))
			ShowWarning("Memory leak detected at ERS '%s', %d objects not freed.\n", instance->Name, instance->Count);

	if (--instance->Cache->ReferenceCount <= 0)
		ers_free_cache(instance->Cache, true);

	if (instance->Next)
		instance->Next->Prev = instance->Prev;

	if (instance->Prev)
		instance->Prev->Next = instance->Next;
	else
		InstanceList = instance->Next;

	if( instance->Options & ERS_OPT_FREE_NAME )
		aFree(instance->Name);

	aFree(instance);
}

void ers_cache_size(ERS *self, uint32 new_size) {
	struct ers_instance_t *instance = (struct ers_instance_t *)self;

	nullpo_retv(instance);

	if( !(instance->Cache->Options&ERS_OPT_FLEX_CHUNK) ) {
		ShowWarning("ers_cache_size: '%s' has adjusted its chunk size to '%d', however ERS_OPT_FLEX_CHUNK is missing!\n",instance->Name,new_size);
	}

	instance->Cache->ChunkSize = new_size;
}


ERS *ers_new(uint32 size, const char *name, enum ERSOptions options)
{
	struct ers_instance_t *instance;
	CREATE(instance,struct ers_instance_t, 1);

	size += sizeof(struct ers_list);

#if ERS_ALIGNED > 1 // If it's aligned to 1-byte boundaries, no need to bother.
	if (size % ERS_ALIGNED)
		size += ERS_ALIGNED - size % ERS_ALIGNED;
#endif

	instance->VTable.alloc = ers_obj_alloc_entry;
	instance->VTable.free = ers_obj_free_entry;
	instance->VTable.entry_size = ers_obj_entry_size;
	instance->VTable.destroy = ers_obj_destroy;
	instance->VTable.chunk_size = ers_cache_size;

	instance->Name = ( options & ERS_OPT_FREE_NAME ) ? (char *)aStrdup(name) : (char *)name;
	instance->Options = options;

	instance->Cache = ers_find_cache(size,instance->Options);

	instance->Cache->ReferenceCount++;

	if (InstanceList == nullptr) {
		InstanceList = instance;
	} else {
		instance->Next = InstanceList;
		instance->Next->Prev = instance;
		InstanceList = instance;
		InstanceList->Prev = nullptr;
	}

	instance->Count = 0;

	return &instance->VTable;
}

void ers_report(void) {
	ers_cache_t *cache;
	uint32 cache_c = 0, blocks_u = 0, blocks_a = 0, memory_b = 0, memory_t = 0;

	for (cache = CacheList; cache; cache = cache->Next) {
		cache_c++;
		ShowMessage(CL_BOLD"[ERS Cache of size '" CL_NORMAL "" CL_WHITE "%u" CL_NORMAL "" CL_BOLD "' report]\n" CL_NORMAL, cache->ObjectSize);
		ShowMessage("\tinstances          : %u\n", cache->ReferenceCount);
		ShowMessage("\tblocks in use      : %u/%u\n", cache->UsedObjs, cache->UsedObjs+cache->Free);
		ShowMessage("\tblocks unused      : %u\n", cache->Free);
		ShowMessage("\tmemory in use      : %.2f MB\n", cache->UsedObjs == 0 ? 0. : (double)((cache->UsedObjs * cache->ObjectSize)/1024)/1024);
		ShowMessage("\tmemory allocated   : %.2f MB\n", (cache->Free+cache->UsedObjs) == 0 ? 0. : (double)(((cache->UsedObjs+cache->Free) * cache->ObjectSize)/1024)/1024);
		blocks_u += cache->UsedObjs;
		blocks_a += cache->UsedObjs + cache->Free;
		memory_b += cache->UsedObjs * cache->ObjectSize;
		memory_t += (cache->UsedObjs+cache->Free) * cache->ObjectSize;
	}
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' caches in use\n",cache_c);
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' blocks in use, consuming '" CL_WHITE "%.2f MB" CL_NORMAL "'\n",blocks_u,(double)((memory_b)/1024)/1024);
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' blocks total, consuming '" CL_WHITE "%.2f MB" CL_NORMAL "' \n",blocks_a,(double)((memory_t)/1024)/1024);
}

/**
 * Call on shutdown to clear remaining entries
 **/
void ers_final(void) {
	struct ers_instance_t *instance = InstanceList, *next;

	while( instance ) {
		next = instance->Next;
		ers_obj_destroy((ERS*)instance);
		instance = next;
	}
}

#endif
