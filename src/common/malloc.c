// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/core.h"
#include "../common/showmsg.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

////////////// Memory Libraries //////////////////

#if defined(MEMWATCH)

#	include <string.h> 
#	include "memwatch.h"
#	define MALLOC(n,file,line,func)	mwMalloc((n),(file),(line))
#	define CALLOC(m,n,file,line,func)	mwCalloc((m),(n),(file),(line))
#	define REALLOC(p,n,file,line,func)	mwRealloc((p),(n),(file),(line))
#	define STRDUP(p,file,line,func)	mwStrdup((p),(file),(line))
#	define FREE(p,file,line,func)		mwFree((p),(file),(line))
#	define MEMORY_USAGE()	0
#	define MEMORY_VERIFY(ptr)	mwIsSafeAddr(ptr, 1)
#	define MEMORY_CHECK() CHECK()

#elif defined(DMALLOC)

#	include <string.h>
#	include <stdlib.h>
#	include "dmalloc.h"
#	define MALLOC(n,file,line,func)	dmalloc_malloc((file),(line),(n),DMALLOC_FUNC_MALLOC,0,0)
#	define CALLOC(m,n,file,line,func)	dmalloc_malloc((file),(line),(m)*(n),DMALLOC_FUNC_CALLOC,0,0)
#	define REALLOC(p,n,file,line,func)	dmalloc_realloc((file),(line),(p),(n),DMALLOC_FUNC_REALLOC,0)
#	define STRDUP(p,file,line,func)	strdup(p)
#	define FREE(p,file,line,func)		free(p)
#	define MEMORY_USAGE()	dmalloc_memory_allocated()
#	define MEMORY_VERIFY(ptr)	(dmalloc_verify(ptr) == DMALLOC_VERIFY_NOERROR)
#	define MEMORY_CHECK()	dmalloc_log_stats(); dmalloc_log_unfreed()

#elif defined(GCOLLECT)

#	include "gc.h"
#	ifdef GC_ADD_CALLER
#		define RETURN_ADDR 0,
#	else
#		define RETURN_ADDR
#	endif
#	define MALLOC(n,file,line,func)	GC_debug_malloc((n), RETURN_ADDR (file),(line))
#	define CALLOC(m,n,file,line,func)	GC_debug_malloc((m)*(n), RETURN_ADDR (file),(line))
#	define REALLOC(p,n,file,line,func)	GC_debug_realloc((p),(n), RETURN_ADDR (file),(line))
#	define STRDUP(p,file,line,func)	GC_debug_strdup((p), RETURN_ADDR (file),(line))
#	define FREE(p,file,line,func)		GC_debug_free(p)
#	define MEMORY_USAGE()	GC_get_heap_size()
#	define MEMORY_VERIFY(ptr)	(GC_base(ptr) != NULL)
#	define MEMORY_CHECK()	GC_gcollect()

#else

#	define MALLOC(n,file,line,func)	malloc(n)
#	define CALLOC(m,n,file,line,func)	calloc((m),(n))
#	define REALLOC(p,n,file,line,func)	realloc((p),(n))
#	define STRDUP(p,file,line,func)	strdup(p)
#	define FREE(p,file,line,func)		free(p)
#	define MEMORY_USAGE()	0
#	define MEMORY_VERIFY(ptr)	true
#	define MEMORY_CHECK()

#endif

void* aMalloc_(size_t size, const char *file, int line, const char *func)
{
	void *ret = MALLOC(size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aMalloc %d\n",file,line,func,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aMalloc error out of memory!\n",file,line,func);
		exit(EXIT_FAILURE);
	}

	return ret;
}
void* aCalloc_(size_t num, size_t size, const char *file, int line, const char *func)
{
	void *ret = CALLOC(num, size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aCalloc %d %d\n",file,line,func,num,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aCalloc error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
void* aRealloc_(void *p, size_t size, const char *file, int line, const char *func)
{
	void *ret = REALLOC(p, size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aRealloc %p %d\n",file,line,func,p,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aRealloc error out of memory!\n",file,line,func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
char* aStrdup_(const char *p, const char *file, int line, const char *func)
{
	char *ret = STRDUP(p, file, line, func);
	// ShowMessage("%s:%d: in func %s: aStrdup %p\n",file,line,func,p);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aStrdup error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
void aFree_(void *p, const char *file, int line, const char *func)
{
	// ShowMessage("%s:%d: in func %s: aFree %p\n",file,line,func,p);
	if (p)
		FREE(p, file, line, func);
}


#ifdef USE_MEMMGR

#if defined(DEBUG)
#define DEBUG_MEMMGR
#endif

/* USE_MEMMGR */

/*
 * Memory manager
 *     able to handle malloc and free efficiently
 *     Since the complex processing, I might be slightly heavier.
 *
 * (I'm sorry for the poor description ^ ^;) such as data structures
 *		Divided into "blocks" of a plurality of memory, "unit" of a plurality of blocks further
 *      I have to divide. Size of the unit, a plurality of distribution equal to the capacity of one block
 *      That's what you have. For example, if one unit of 32KB, one block 1 Yu 32Byte
 *      Knit, or are able to gather 1024, gathered 512 units 64Byte
 *      I can be or have. (Excluding padding, the unit_head)
 *
 *     Lead-linked list (block_prev, block_next) in each other is the same size block
 *       Linked list (hash_prev, hash_nect) even among such one in the block with the figure
 *       I like to have. Thus, reuse of memory no longer needed can be performed efficiently.
 */

/* Alignment of the block */
#define BLOCK_ALIGNMENT1	16
#define BLOCK_ALIGNMENT2	64

/* Amount of data entering a block */
#define BLOCK_DATA_COUNT1	128
#define BLOCK_DATA_COUNT2	608

/* The size of the block: 16*128 + 64*576 = 40KB */
#define BLOCK_DATA_SIZE1	( BLOCK_ALIGNMENT1 * BLOCK_DATA_COUNT1 )
#define BLOCK_DATA_SIZE2	( BLOCK_ALIGNMENT2 * BLOCK_DATA_COUNT2 )
#define BLOCK_DATA_SIZE		( BLOCK_DATA_SIZE1 + BLOCK_DATA_SIZE2 )

/* The number of blocks to be allocated at a time. */
#define BLOCK_ALLOC		104

/* block */
struct block {
	struct block* block_next;		/* Then the allocated area */
	struct block* unfill_prev;		/* The previous area not filled */
	struct block* unfill_next;		/* The next area not filled */
	unsigned short unit_size;		/* The size of the unit */
	unsigned short unit_hash;		/* The hash of the unit */
	unsigned short unit_count;		/* The number of units */
	unsigned short unit_used;		/* The number of used units */
	unsigned short unit_unfill;		/* The number of unused units */
	unsigned short unit_maxused;	/* The maximum value of units used */
	char   data[ BLOCK_DATA_SIZE ];
};

struct unit_head {
	struct block   *block;
	const  char*   file;
	unsigned short line;
	unsigned short size;
	long           checksum;
};

static struct block* hash_unfill[BLOCK_DATA_COUNT1 + BLOCK_DATA_COUNT2 + 1];
static struct block* block_first, *block_last, block_head;

/* Data for areas that do not use the memory be turned */
struct unit_head_large {
	size_t                  size;
	struct unit_head_large* prev;
	struct unit_head_large* next;
	struct unit_head        unit_head;
};

static struct unit_head_large *unit_head_large_first = NULL;

static struct block* block_malloc(unsigned short hash);
static void          block_free(struct block* p);
static size_t        memmgr_usage_bytes;

#define block2unit(p, n) ((struct unit_head*)(&(p)->data[ p->unit_size * (n) ]))
#define memmgr_assert(v) do { if(!(v)) { ShowError("Memory manager: assertion '" #v "' failed!\n"); } } while(0)

static unsigned short size2hash( size_t size )
{
	if( size <= BLOCK_DATA_SIZE1 ) {
		return (unsigned short)(size + BLOCK_ALIGNMENT1 - 1) / BLOCK_ALIGNMENT1;
	} else if( size <= BLOCK_DATA_SIZE ){
		return (unsigned short)(size - BLOCK_DATA_SIZE1 + BLOCK_ALIGNMENT2 - 1) / BLOCK_ALIGNMENT2
				+ BLOCK_DATA_COUNT1;
	} else {
		return 0xffff;	// If it exceeds the block length hash I do not
	}
}

static size_t hash2size( unsigned short hash )
{
	if( hash <= BLOCK_DATA_COUNT1) {
		return hash * BLOCK_ALIGNMENT1;
	} else {
		return (hash - BLOCK_DATA_COUNT1) * BLOCK_ALIGNMENT2 + BLOCK_DATA_SIZE1;
	}
}

void* _mmalloc(size_t size, const char *file, int line, const char *func )
{
	struct block *block;
	short size_hash = size2hash( size );
	struct unit_head *head;

	if (((long) size) < 0) {
		ShowError("_mmalloc: %d\n", size);
		return NULL;
	}
	
	if(size == 0) {
		return NULL;
	}
	memmgr_usage_bytes += size;

	/* To ensure the area that exceeds the length of the block, using malloc () to */
	/* At that time, the distinction by assigning NULL to unit_head.block */
	if(hash2size(size_hash) > BLOCK_DATA_SIZE - sizeof(struct unit_head)) {
		struct unit_head_large* p = (struct unit_head_large*)MALLOC(sizeof(struct unit_head_large)+size,file,line,func);
		if(p != NULL) {
			p->size            = size;
			p->unit_head.block = NULL;
			p->unit_head.size  = 0;
			p->unit_head.file  = file;
			p->unit_head.line  = line;
			p->prev = NULL;
			if (unit_head_large_first == NULL)
				p->next = NULL;
			else {
				unit_head_large_first->prev = p;
				p->next = unit_head_large_first;
			}
			unit_head_large_first = p;
			*(long*)((char*)p + sizeof(struct unit_head_large) - sizeof(long) + size) = 0xdeadbeaf;
			return (char *)p + sizeof(struct unit_head_large) - sizeof(long);
		} else {
			ShowFatalError("Memory manager::memmgr_alloc failed (allocating %d+%d bytes at %s:%d).\n", sizeof(struct unit_head_large), size, file, line);
			exit(EXIT_FAILURE);
		}
	}

	/* When a block of the same size is not ensured, to ensure a new */
	if(hash_unfill[size_hash]) {
		block = hash_unfill[size_hash];
	} else {
		block = block_malloc(size_hash);
	}

	if( block->unit_unfill == 0xFFFF ) {
		// there are no more free space that
		memmgr_assert(block->unit_used <  block->unit_count);
		memmgr_assert(block->unit_used == block->unit_maxused);
		head = block2unit(block, block->unit_maxused);
		block->unit_used++;
		block->unit_maxused++;
	} else {
		head = block2unit(block, block->unit_unfill);
		block->unit_unfill = head->size;
		block->unit_used++;
	}

	if( block->unit_unfill == 0xFFFF && block->unit_maxused >= block->unit_count) {
		// Since I ran out of the unit, removed from the list unfill
		if( block->unfill_prev == &block_head) {
			hash_unfill[ size_hash ] = block->unfill_next;
		} else {
			block->unfill_prev->unfill_next = block->unfill_next;
		}
		if( block->unfill_next ) {
			block->unfill_next->unfill_prev = block->unfill_prev;
		}
		block->unfill_prev = NULL;
	}

#ifdef DEBUG_MEMMGR
	{
		size_t i, sz = hash2size( size_hash );
		for( i=0; i<sz; i++ )
		{
			if( ((unsigned char*)head)[ sizeof(struct unit_head) - sizeof(long) + i] != 0xfd )
			{
				if( head->line != 0xfdfd )
				{
					ShowError("Memory manager: freed-data is changed. (freed in %s line %d)\n", head->file,head->line);
				}
				else
				{
					ShowError("Memory manager: not-allocated-data is changed.\n");
				}
				break;
			}
		}
		memset( (char *)head + sizeof(struct unit_head) - sizeof(long), 0xcd, sz );
	}
#endif

	head->block = block;
	head->file  = file;
	head->line  = line;
	head->size  = (unsigned short)size;
	*(long*)((char*)head + sizeof(struct unit_head) - sizeof(long) + size) = 0xdeadbeaf;
	return (char *)head + sizeof(struct unit_head) - sizeof(long);
}

void* _mcalloc(size_t num, size_t size, const char *file, int line, const char *func )
{
	void *p = _mmalloc(num * size,file,line,func);
	memset(p,0,num * size);
	return p;
}

void* _mrealloc(void *memblock, size_t size, const char *file, int line, const char *func )
{
	size_t old_size;
	if(memblock == NULL) {
		return _mmalloc(size,file,line,func);
	}

	old_size = ((struct unit_head *)((char *)memblock - sizeof(struct unit_head) + sizeof(long)))->size;
	if( old_size == 0 ) {
		old_size = ((struct unit_head_large *)((char *)memblock - sizeof(struct unit_head_large) + sizeof(long)))->size;
	}
	if(old_size > size) {
		// Size reduction - return> as it is (negligence)
		return memblock;
	}  else {
		// Size Large
		void *p = _mmalloc(size,file,line,func);
		if(p != NULL) {
			memcpy(p,memblock,old_size);
		}
		_mfree(memblock,file,line,func);
		return p;
	}
}

char* _mstrdup(const char *p, const char *file, int line, const char *func )
{
	if(p == NULL) {
		return NULL;
	} else {
		size_t len = strlen(p);
		char *string  = (char *)_mmalloc(len + 1,file,line,func);
		memcpy(string,p,len+1);
		return string;
	}
}

void _mfree(void *ptr, const char *file, int line, const char *func )
{
	struct unit_head *head;

	if (ptr == NULL)
		return; 

	head = (struct unit_head *)((char *)ptr - sizeof(struct unit_head) + sizeof(long));
	if(head->size == 0) {
		/* area that is directly secured by malloc () */
		struct unit_head_large *head_large = (struct unit_head_large *)((char *)ptr - sizeof(struct unit_head_large) + sizeof(long));
		if(
			*(long*)((char*)head_large + sizeof(struct unit_head_large) - sizeof(long) + head_large->size)
			!= 0xdeadbeaf)
		{
			ShowError("Memory manager: args of aFree 0x%p is overflowed pointer %s line %d\n", ptr, file, line);
		} else {
			head->size = 0xFFFF;
			if(head_large->prev) {
				head_large->prev->next = head_large->next;
			} else {
				unit_head_large_first  = head_large->next;
			}
			if(head_large->next) {
				head_large->next->prev = head_large->prev;
			}
			memmgr_usage_bytes -= head_large->size;
#ifdef DEBUG_MEMMGR
			// set freed memory to 0xfd
			memset(ptr, 0xfd, head_large->size);
#endif
			FREE(head_large,file,line,func);
		}
	} else {
		/* Release unit */
		struct block *block = head->block;
		if( (char*)head - (char*)block > sizeof(struct block) ) {
			ShowError("Memory manager: args of aFree 0x%p is invalid pointer %s line %d\n", ptr, file, line);
		} else if(head->block == NULL) {
			ShowError("Memory manager: args of aFree 0x%p is freed pointer %s:%d@%s\n", ptr, file, line, func);
		} else if(*(long*)((char*)head + sizeof(struct unit_head) - sizeof(long) + head->size) != 0xdeadbeaf) {
			ShowError("Memory manager: args of aFree 0x%p is overflowed pointer %s line %d\n", ptr, file, line);
		} else {
			memmgr_usage_bytes -= head->size;
			head->block         = NULL;
#ifdef DEBUG_MEMMGR
			memset(ptr, 0xfd, block->unit_size - sizeof(struct unit_head) + sizeof(long) );
			head->file = file;
			head->line = line;
#endif
			memmgr_assert( block->unit_used > 0 );
			if(--block->unit_used == 0) {
				/* Release of the block */
				block_free(block);
			} else {
				if( block->unfill_prev == NULL) {
					// add to unfill list
					if( hash_unfill[ block->unit_hash ] ) {
						hash_unfill[ block->unit_hash ]->unfill_prev = block;
					}
					block->unfill_prev = &block_head;
					block->unfill_next = hash_unfill[ block->unit_hash ];
					hash_unfill[ block->unit_hash ] = block;
				}
				head->size     = block->unit_unfill;
				block->unit_unfill = (unsigned short)(((uintptr_t)head - (uintptr_t)block->data) / block->unit_size);
			}
		}
	}
}

/* Allocating blocks */
static struct block* block_malloc(unsigned short hash)
{
	struct block *p;
	if(hash_unfill[0] != NULL) {
		/* Space for the block has already been secured */
		p = hash_unfill[0];
		hash_unfill[0] = hash_unfill[0]->unfill_next;
	} else {
		int i;
		/* Newly allocated space for the block */
		p = (struct block*)MALLOC(sizeof(struct block) * (BLOCK_ALLOC), __FILE__, __LINE__, __func__ );
		if(p == NULL) {
			ShowFatalError("Memory manager::block_alloc failed.\n");
			exit(EXIT_FAILURE);
		}

		if(block_first == NULL) {
			/* First ensure */
			block_first = p;
		} else {
			block_last->block_next = p;
		}
		block_last = &p[BLOCK_ALLOC - 1];
		block_last->block_next = NULL;
		/* Linking the block */
		for(i=0;i<BLOCK_ALLOC;i++) {
			if(i != 0) {
				// I do not add the link p [0], so we will use
				p[i].unfill_next = hash_unfill[0];
				hash_unfill[0]   = &p[i];
				p[i].unfill_prev = NULL;
				p[i].unit_used = 0;
			}
			if(i != BLOCK_ALLOC -1) {
				p[i].block_next = &p[i+1];
			}
		}
	}

	// Add to unfill
	memmgr_assert(hash_unfill[ hash ] == NULL);
	hash_unfill[ hash ] = p;
	p->unfill_prev  = &block_head;
	p->unfill_next  = NULL;
	p->unit_size    = (unsigned short)(hash2size( hash ) + sizeof(struct unit_head));
	p->unit_hash    = hash;
	p->unit_count   = BLOCK_DATA_SIZE / p->unit_size;
	p->unit_used    = 0;
	p->unit_unfill  = 0xFFFF;
	p->unit_maxused = 0;
#ifdef DEBUG_MEMMGR
	memset( p->data, 0xfd, sizeof(p->data) );
#endif
	return p;
}

static void block_free(struct block* p)
{
	if( p->unfill_prev ) {
		if( p->unfill_prev == &block_head) {
			hash_unfill[ p->unit_hash ] = p->unfill_next;
		} else {
			p->unfill_prev->unfill_next = p->unfill_next;
		}
		if( p->unfill_next ) {
			p->unfill_next->unfill_prev = p->unfill_prev;
		}
		p->unfill_prev = NULL;
	}

	p->unfill_next = hash_unfill[0];
	hash_unfill[0] = p;
}

size_t memmgr_usage (void)
{
	return memmgr_usage_bytes / 1024;
}

#ifdef LOG_MEMMGR
static char memmer_logfile[128];
static FILE *log_fp;

static void memmgr_log (char *buf)
{
	if( !log_fp )
	{
		time_t raw;
		struct tm* t;

		log_fp = fopen(memmer_logfile,"at");
		if (!log_fp) log_fp = stdout;

		time(&raw);
		t = localtime(&raw);
		fprintf(log_fp, "\nMemory manager: Memory leaks found at %d/%02d/%02d %02dh%02dm%02ds (Revision %s).\n",
			(t->tm_year+1900), (t->tm_mon+1), t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, get_svn_revision());
	}
	fprintf(log_fp, "%s", buf);
	return;
}
#endif /* LOG_MEMMGR */

/// Returns true if the memory location is active.
/// Active means it is allocated and points to a usable part.
///
/// @param ptr Pointer to the memory
/// @return true if the memory is active
bool memmgr_verify(void* ptr)
{
	struct block* block = block_first;
	struct unit_head_large* large = unit_head_large_first;

	if( ptr == NULL )
		return false;// never valid

	// search small blocks
	while( block )
	{
		if( (char*)ptr >= (char*)block && (char*)ptr < ((char*)block) + sizeof(struct block) )
		{// found memory block
			if( block->unit_used && (char*)ptr >= block->data )
			{// memory block is being used and ptr points to a sub-unit
				size_t i = (size_t)((char*)ptr - block->data)/block->unit_size;
				struct unit_head* head = block2unit(block, i);
				if( i < block->unit_maxused && head->block != NULL )
				{// memory unit is allocated, check if ptr points to the usable part
					return ( (char*)ptr >= ((char*)head) + sizeof(struct unit_head) - sizeof(long)
						&& (char*)ptr < ((char*)head) + sizeof(struct unit_head) - sizeof(long) + head->size );
				}
			}
			return false;
		}
		block = block->block_next;
	}

	// search large blocks
	while( large )
	{
		if( (char*)ptr >= (char*)large && (char*)ptr < ((char*)large) + large->size )
		{// found memory block, check if ptr points to the usable part
			return ( (char*)ptr >= ((char*)large) + sizeof(struct unit_head_large) - sizeof(long)
				&& (char*)ptr < ((char*)large) + sizeof(struct unit_head_large) - sizeof(long) + large->size );
		}
		large = large->next;
	}
	return false;
}

static void memmgr_final (void)
{
	struct block *block = block_first;
	struct unit_head_large *large = unit_head_large_first;

#ifdef LOG_MEMMGR
	int count = 0;
#endif /* LOG_MEMMGR */

	while (block) {
		if (block->unit_used) {
			int i;
			for (i = 0; i < block->unit_maxused; i++) {
				struct unit_head *head = block2unit(block, i);
				if(head->block != NULL) {
					char* ptr = (char *)head + sizeof(struct unit_head) - sizeof(long);
#ifdef LOG_MEMMGR
					char buf[1024];
					sprintf (buf,
						"%04d : %s line %d size %lu address 0x%p\n", ++count,
						head->file, head->line, (unsigned long)head->size, ptr);
					memmgr_log (buf);
#endif /* LOG_MEMMGR */
					// get block pointer and free it [celest]
					_mfree(ptr, ALC_MARK);
				}
			}
		}
		block = block->block_next;
	}

	while(large) {
		struct unit_head_large *large2;
#ifdef LOG_MEMMGR
		char buf[1024];
		sprintf (buf,
			"%04d : %s line %d size %lu address 0x%p\n", ++count,
			large->unit_head.file, large->unit_head.line, (unsigned long)large->size, &large->unit_head.checksum);
		memmgr_log (buf);
#endif /* LOG_MEMMGR */
		large2 = large->next;
		FREE(large,file,line,func);
		large = large2;
	}
#ifdef LOG_MEMMGR
	if(count == 0) {
		ShowInfo("Memory manager: No memory leaks found.\n");
	} else {
		ShowWarning("Memory manager: Memory leaks found and fixed.\n");
		fclose(log_fp);
	}
#endif /* LOG_MEMMGR */
}

static void memmgr_init (void)
{
#ifdef LOG_MEMMGR
	sprintf(memmer_logfile, "log/%s.leaks", SERVER_NAME);
	ShowStatus("Memory manager initialised: "CL_WHITE"%s"CL_RESET"\n", memmer_logfile);
	memset(hash_unfill, 0, sizeof(hash_unfill));
#endif /* LOG_MEMMGR */
}
#endif /* USE_MEMMGR */


/*======================================
 * Initialise
 *--------------------------------------
 */


/// Tests the memory for errors and memory leaks.
void malloc_memory_check(void)
{
	MEMORY_CHECK();
}


/// Returns true if a pointer is valid.
/// The check is best-effort, false positives are possible.
bool malloc_verify_ptr(void* ptr)
{
#ifdef USE_MEMMGR
	return memmgr_verify(ptr) && MEMORY_VERIFY(ptr);
#else
	return MEMORY_VERIFY(ptr);
#endif
}


size_t malloc_usage (void)
{
#ifdef USE_MEMMGR
	return memmgr_usage ();
#else
	return MEMORY_USAGE();
#endif
}

void malloc_final (void)
{
#ifdef USE_MEMMGR
	memmgr_final ();
#endif
	MEMORY_CHECK();
}

void malloc_init (void)
{
#if defined(DMALLOC) && defined(CYGWIN)
	// http://dmalloc.com/docs/latest/online/dmalloc_19.html
	dmalloc_debug_setup(getenv("DMALLOC_OPTIONS"));
#endif
#ifdef GCOLLECT
	// don't garbage collect, only report inaccessible memory that was not deallocated
	GC_find_leak = 1;
	GC_INIT();
#endif
#ifdef USE_MEMMGR
	memmgr_init ();
#endif
}
