// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "../common/cbasetypes.h"

// Q: What are the 'a'-variant allocation functions?
// A: They allocate memory from the stack, which is automatically 
//    freed when the invoking function returns.
//    But it's not portable (http://c-faq.com/malloc/alloca.html)
//    and I have doubts our implementation works.
//    -> They should NOT be used, period.

#define ALC_MARK __FILE__, __LINE__, __func__

// disable built-in memory manager when using another manager
#if defined(MEMWATCH) || defined(DMALLOC) || defined(GCOLLECT) || defined(BCHECK)
#if !defined(NO_MEMMGR)
#define NO_MEMMGR
#endif
#endif

// Use built-in memory manager by default
#if !defined(NO_MEMMGR) && !defined(USE_MEMMGR)
#define USE_MEMMGR
#endif


//////////////////////////////////////////////////////////////////////
// Athena's built-in Memory Manager
#ifdef USE_MEMMGR

// Enable memory manager logging by default
#define LOG_MEMMGR

#	define aMalloc(n)		_mmalloc(n,ALC_MARK)
#	define aMallocA(n)		_mmalloc(n,ALC_MARK)
#	define aCalloc(m,n)		_mcalloc(m,n,ALC_MARK)
#	define aCallocA(m,n)	_mcalloc(m,n,ALC_MARK)
#	define aRealloc(p,n)	_mrealloc(p,n,ALC_MARK)
#	define aStrdup(p)		_mstrdup(p,ALC_MARK)
#	define aFree(p)			_mfree(p,ALC_MARK)

	void* _mmalloc	(size_t size, const char *file, int line, const char *func);
	void* _mcalloc	(size_t num, size_t size, const char *file, int line, const char *func);
	void* _mrealloc	(void *p, size_t size, const char *file, int line, const char *func);
	char* _mstrdup	(const char *p, const char *file, int line, const char *func);
	void  _mfree	(void *p, const char *file, int line, const char *func);

#else

#	define aMalloc(n)		aMalloc_((n),ALC_MARK)
#	define aMallocA(n)		aMallocA_((n),ALC_MARK)
#	define aCalloc(m,n)		aCalloc_((m),(n),ALC_MARK)
#	define aCallocA(m,n)	aCallocA_(m,n,ALC_MARK)
#	define aRealloc(p,n)	aRealloc_(p,n,ALC_MARK)
#	define aStrdup(p)		aStrdup_(p,ALC_MARK)
#	define aFree(p)			aFree_(p,ALC_MARK)

	void* aMalloc_	(size_t size, const char *file, int line, const char *func);
	void* aMallocA_	(size_t size, const char *file, int line, const char *func);
	void* aCalloc_	(size_t num, size_t size, const char *file, int line, const char *func);
	void* aCallocA_	(size_t num, size_t size, const char *file, int line, const char *func);
	void* aRealloc_	(void *p, size_t size, const char *file, int line, const char *func);
	char* aStrdup_	(const char *p, const char *file, int line, const char *func);
	void  aFree_	(void *p, const char *file, int line, const char *func);

#endif

////////////// Memory Managers //////////////////

#if defined(MEMWATCH)

#	include "memwatch.h"
#	define MALLOC(n,file,line,func)	mwMalloc((n),(file),(line))
#	define MALLOCA(n,file,line,func)	mwMalloc((n),(file),(line))
#	define CALLOC(m,n,file,line,func)	mwCalloc((m),(n),(file),(line))
#	define CALLOCA(m,n,file,line,func)	mwCalloc((m),(n),(file),(line))
#	define REALLOC(p,n,file,line,func)	mwRealloc((p),(n),(file),(line))
#	define STRDUP(p,file,line,func)	mwStrdup((p),(file),(line))
#	define FREE(p,file,line,func)		mwFree((p),(file),(line))

#elif defined(DMALLOC)

#	include "dmalloc.h"
#	define MALLOC(n,file,line,func)	dmalloc_malloc((file),(line),(n),DMALLOC_FUNC_MALLOC,0,0)
#	define MALLOCA(n,file,line,func)	dmalloc_malloc((file),(line),(n),DMALLOC_FUNC_MALLOC,0,0)
#	define CALLOC(m,n,file,line,func)	dmalloc_malloc((file),(line),(m)*(n),DMALLOC_FUNC_CALLOC,0,0)
#	define CALLOCA(m,n,file,line,func)	dmalloc_malloc((file),(line),(m)*(n),DMALLOC_FUNC_CALLOC,0,0)
#	define REALLOC(p,n,file,line,func)	dmalloc_realloc((file),(line),(p),(n),DMALLOC_FUNC_REALLOC,0)
#	define STRDUP(p,file,line,func)	strdup(p)
#	define FREE(p,file,line,func)		free(p)

#elif defined(GCOLLECT)

#	include "gc.h"
#	define MALLOC(n,file,line,func)	GC_MALLOC(n)
#	define MALLOCA(n,file,line,func)	GC_MALLOC_ATOMIC(n)
#	define CALLOC(m,n,file,line,func)	_bcalloc((m),(n))
#	define CALLOCA(m,n,file,line,func)	_bcallocA((m),(n))
#	define REALLOC(p,n,file,line,func)	GC_REALLOC((p),(n))
#	define STRDUP(p,file,line,func)	_bstrdup(p)
#	define FREE(p,file,line,func)		GC_FREE(p)

	void * _bcalloc(size_t, size_t);
	void * _bcallocA(size_t, size_t);
	char * _bstrdup(const char *);

#elif defined(BCHECK)

#	define MALLOC(n,file,line,func)	malloc(n)
#	define MALLOCA(n,file,line,func)	malloc(n)
#	define CALLOC(m,n,file,line,func)	calloc((m),(n))
#	define CALLOCA(m,n,file,line,func)	calloc((m),(n))
#	define REALLOC(p,n,file,line,func)	realloc((p),(n))
#	define STRDUP(p,file,line,func)	strdup(p)
#	define FREE(p,file,line,func)		free(p)

#else

#	define MALLOC(n,file,line,func)	malloc(n)
#	define MALLOCA(n,file,line,func)	malloc(n)
#	define CALLOC(m,n,file,line,func)	calloc((m),(n))
#	define CALLOCA(m,n,file,line,func)	calloc((m),(n))
#	define REALLOC(p,n,file,line,func)	realloc((p),(n))
#	define STRDUP(p,file,line,func)	strdup(p)
#	define FREE(p,file,line,func)		free(p)

#endif

/////////////// Buffer Creation /////////////////
// Full credit for this goes to Shinomori [Ajarn]

#ifdef __GNUC__ // GCC has variable length arrays

	#define CREATE_BUFFER(name, type, size) type name[size]
	#define DELETE_BUFFER(name)

#else // others don't, so we emulate them

	#define CREATE_BUFFER(name, type, size) type *name = (type *) aCalloc (size, sizeof(type))
	#define DELETE_BUFFER(name) aFree(name)

#endif

////////////// Others //////////////////////////
// should be merged with any of above later
#define CREATE(result, type, number) (result) = (type *) aCalloc ((number), sizeof(type))

#define CREATE_A(result, type, number) (result) = (type *) aCallocA ((number), sizeof(type))

#define RECREATE(result, type, number) (result) = (type *) aRealloc ((result), sizeof(type) * (number))

////////////////////////////////////////////////

bool malloc_verify(void* ptr);
size_t malloc_usage (void);
void malloc_init (void);
void malloc_final (void);

#endif /* _MALLOC_H_ */
