/*****************************************************************************\
 *  Copyright (c) Athena Dev Teams - Licensed under GNU GPL                  *
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
 *  Failure to do so can lead to unexpected behaviours.                      *
 *                                                                           *
 *  <H2>Advantages:</H2>                                                     *
 *  - The same manager is used for entries of the same size.                 *
 *    So entries freed in one instance of the manager can be used by other   *
 *    instances of the manager.                                              *
 *  - Much less memory allocation/deallocation - program will be faster.     *
 *  - Avoids memory fragmentaion - program will run better for longer.       *
 *                                                                           *
 *  <H2>Disavantages:</H2>                                                   *
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
 * @see common#ers.h                                                         *
\*****************************************************************************/
#include <stdlib.h>

#include "../common/cbasetypes.h"
#include "../common/malloc.h" // CREATE, RECREATE, aMalloc, aFree
#include "../common/showmsg.h" // ShowMessage, ShowError, ShowFatalError, CL_BOLD, CL_NORMAL
#include "ers.h"

#ifndef DISABLE_ERS

#define ERS_ROOT_SIZE 256
#define ERS_BLOCK_ENTRIES 4096

struct ers_list
{
	struct ers_list *Next;
};

typedef struct ers_cache
{
	// Allocated object size, including ers_list size
	unsigned int ObjectSize;

	// Number of ers_instances referencing this
	int ReferenceCount;

	// Reuse linked list
	struct ers_list *ReuseList;

	// Memory blocks array
	unsigned char **Blocks;

	// Max number of blocks 
	unsigned int Max;

	// Free objects count
	unsigned int Free;
	
	// Used objects count
	unsigned int Used;

	// Linked list
	struct ers_cache *Next, *Prev;
} ers_cache_t;

typedef struct
{
	// Interface to ERS
	struct eri VTable;
	
	// Name, used for debbuging purpouses
	char *Name;

	// Misc options
	enum ERSOptions Options;

	// Our cache
	ers_cache_t *Cache;

	// Count of objects in use, used for detecting memory leaks
	unsigned int Count;
} ers_instance_t;


// Array containing a pointer for all ers_cache structures
static ers_cache_t *CacheList;

static ers_cache_t *ers_find_cache(unsigned int size)
{
	ers_cache_t *cache;

	for (cache = CacheList; cache; cache = cache->Next)
		if (cache->ObjectSize == size)
			return cache;

	CREATE(cache, ers_cache_t, 1);
	cache->ObjectSize = size;
	cache->ReferenceCount = 0;
	cache->ReuseList = NULL;
	cache->Blocks = NULL;
	cache->Free = 0;
	cache->Used = 0;
	cache->Max = 0;
	
	if (CacheList == NULL)
	{
		CacheList = cache;
	}
	else
	{
		cache->Next = CacheList;
		cache->Next->Prev = cache;
		CacheList = cache;
		CacheList->Prev = NULL;
	}

	return cache;
}

static void ers_free_cache(ers_cache_t *cache, bool remove)
{
	unsigned int i;

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

static void *ers_obj_alloc_entry(ERS self)
{
	ers_instance_t *instance = (ers_instance_t *)self;
	void *ret;

	if (instance == NULL) 
	{
		ShowError("ers_obj_alloc_entry: NULL object, aborting entry freeing.\n");
		return NULL;
	}

	if (instance->Cache->ReuseList != NULL)
	{
		ret = (void *)((unsigned char *)instance->Cache->ReuseList + sizeof(struct ers_list));
		instance->Cache->ReuseList = instance->Cache->ReuseList->Next;
	} 
	else if (instance->Cache->Free > 0) 
	{
		instance->Cache->Free--;
		ret = &instance->Cache->Blocks[instance->Cache->Used - 1][instance->Cache->Free * instance->Cache->ObjectSize + sizeof(struct ers_list)];
	} 
	else 
	{
		if (instance->Cache->Used == instance->Cache->Max) 
		{
			instance->Cache->Max = (instance->Cache->Max * 4) + 3;
			RECREATE(instance->Cache->Blocks, unsigned char *, instance->Cache->Max);
		}

		CREATE(instance->Cache->Blocks[instance->Cache->Used], unsigned char, instance->Cache->ObjectSize * ERS_BLOCK_ENTRIES);
		instance->Cache->Used++;

		instance->Cache->Free = ERS_BLOCK_ENTRIES -1;
		ret = &instance->Cache->Blocks[instance->Cache->Used - 1][instance->Cache->Free * instance->Cache->ObjectSize + sizeof(struct ers_list)];
	}

	instance->Count++;

	return ret;
}

static void ers_obj_free_entry(ERS self, void *entry)
{
	ers_instance_t *instance = (ers_instance_t *)self;
	struct ers_list *reuse = (struct ers_list *)((unsigned char *)entry - sizeof(struct ers_list));

	if (instance == NULL) 
	{
		ShowError("ers_obj_free_entry: NULL object, aborting entry freeing.\n");
		return;
	} 
	else if (entry == NULL) 
	{
		ShowError("ers_obj_free_entry: NULL entry, nothing to free.\n");
		return;
	}

	reuse->Next = instance->Cache->ReuseList;
	instance->Cache->ReuseList = reuse;
	instance->Count--;
}

static size_t ers_obj_entry_size(ERS self)
{
	ers_instance_t *instance = (ers_instance_t *)self;

	if (instance == NULL) 
	{
		ShowError("ers_obj_entry_size: NULL object, aborting entry freeing.\n");
		return 0;
	} 

	return instance->Cache->ObjectSize;
}

static void ers_obj_destroy(ERS self)
{
	ers_instance_t *instance = (ers_instance_t *)self;

	if (instance == NULL) 
	{
		ShowError("ers_obj_destroy: NULL object, aborting entry freeing.\n");
		return;
	}

	if (instance->Count > 0)
		if (!(instance->Options & ERS_OPT_CLEAR))
			ShowWarning("Memory leak detected at ERS '%s', %d objects not freed.\n", instance->Name, instance->Count);

	if (--instance->Cache->ReferenceCount <= 0)
		ers_free_cache(instance->Cache, true);

	aFree(instance);
}

ERS ers_new(uint32 size, char *name, enum ERSOptions options)
{
	ers_instance_t *instance;
	CREATE(instance, ers_instance_t, 1);

	size += sizeof(struct ers_list);
	if (size % ERS_ALIGNED)
		size += ERS_ALIGNED - size % ERS_ALIGNED;

	instance->VTable.alloc = ers_obj_alloc_entry;
	instance->VTable.free = ers_obj_free_entry;
	instance->VTable.entry_size = ers_obj_entry_size;
	instance->VTable.destroy = ers_obj_destroy;

	instance->Name = name;
	instance->Options = options;

	instance->Cache = ers_find_cache(size);
	instance->Cache->ReferenceCount++;

	instance->Count = 0;

	return &instance->VTable;
}

void ers_report(void)
{
	ers_cache_t *cache;
	int i = 0;
	for (cache = CacheList; cache; cache = cache->Next) {
		ShowMessage(CL_BOLD"[Entry manager #%u report]\n"CL_NORMAL, ++i);
		ShowMessage("\tinstances          : %u\n", cache->ReferenceCount);
		ShowMessage("\tblock array size   : %u\n", cache->ObjectSize);
		ShowMessage("\tallocated blocks   : %u\n", cache->Free+cache->Used);
		ShowMessage("\tentries being used : %u\n", cache->Used);
		ShowMessage("\tunused entries     : %u\n", cache->Free);
	}
}

void ers_force_destroy_all(void)
{
	ers_cache_t *cache;
	
	for (cache = CacheList; cache; cache = cache->Next)
			ers_free_cache(cache, false);
}

#endif
