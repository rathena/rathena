// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MALLOC_HPP_
#define _MALLOC_HPP_

#include "cbasetypes.hpp"

#define ALC_MARK __FILE__, __LINE__, __func__


// default use of the built-in memory manager
#if !defined(NO_MEMMGR) && !defined(USE_MEMMGR)
#if defined(MEMWATCH) || defined(DMALLOC) || defined(GCOLLECT)
// disable built-in memory manager when using another memory library
#define NO_MEMMGR
#else
// use built-in memory manager by default
#define USE_MEMMGR
#endif
#endif


//////////////////////////////////////////////////////////////////////
// Athena's built-in Memory Manager
#ifdef USE_MEMMGR

// Enable memory manager logging by default
#define LOG_MEMMGR

// no logging for minicore
#if defined(MINICORE) && defined(LOG_MEMMGR)
#undef LOG_MEMMGR
#endif

#	define aMalloc(n)		_mmalloc(n,ALC_MARK)
#	define aCalloc(m,n)		_mcalloc(m,n,ALC_MARK)
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
#	define aCalloc(m,n)		aCalloc_((m),(n),ALC_MARK)
#	define aRealloc(p,n)	aRealloc_(p,n,ALC_MARK)
#	define aStrdup(p)		aStrdup_(p,ALC_MARK)
#	define aFree(p)			aFree_(p,ALC_MARK)

	void* aMalloc_	(size_t size, const char *file, int line, const char *func);
	void* aCalloc_	(size_t num, size_t size, const char *file, int line, const char *func);
	void* aRealloc_	(void *p, size_t size, const char *file, int line, const char *func);
	char* aStrdup_	(const char *p, const char *file, int line, const char *func);
	void  aFree_	(void *p, const char *file, int line, const char *func);

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
#define RECREATE(result, type, number) (result) = (type *) aRealloc ((result), sizeof(type) * (number))

////////////////////////////////////////////////

void malloc_memory_check(void);
bool malloc_verify_ptr(void* ptr);
size_t malloc_usage (void);
void malloc_init (void);
void malloc_final (void);

#endif /* _MALLOC_HPP_ */
