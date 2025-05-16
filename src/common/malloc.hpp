// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MALLOC_HPP
#define MALLOC_HPP

#include <config/core.hpp>

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

#	define aMalloc2(n,file,line,func)		_mmalloc(n,file,line,func)
#	define aCalloc2(m,n,file,line,func)		_mcalloc(m,n,file,line,func)
#	define aRealloc2(p,n,file,line,func)	_mrealloc(p,n,file,line,func)
#	define aStrdup2(p,file,line,func)		_mstrdup(p,file,line,func)
#	define aFree2(p,file,line,func)			_mfree(p,file,line,func)

	void* _mmalloc	(size_t size, const char *file, int32 line, const char *func);
	void* _mcalloc	(size_t num, size_t size, const char *file, int32 line, const char *func);
	void* _mrealloc	(void *p, size_t size, const char *file, int32 line, const char *func);
	char* _mstrdup	(const char *p, const char *file, int32 line, const char *func);
	void  _mfree	(void *p, const char *file, int32 line, const char *func);

#else

#	define aMalloc(n)		aMalloc_((n),ALC_MARK)
#	define aCalloc(m,n)		aCalloc_((m),(n),ALC_MARK)
#	define aRealloc(p,n)	aRealloc_(p,n,ALC_MARK)
#	define aStrdup(p)		aStrdup_(p,ALC_MARK)
#	define aFree(p)			aFree_(p,ALC_MARK)

#	define aMalloc2(n,file,line,func)		aMalloc_((n),file,line,func)
#	define aCalloc2(m,n,file,line,func)		aCalloc_((m),(n),file,line,func)
#	define aRealloc2(p,n,file,line,func)	aRealloc_(p,n,file,line,func)
#	define aStrdup2(p,file,line,func)		aStrdup_(p,file,line,func)
#	define aFree2(p,file,line,func)			aFree_(p,file,line,func)

	void* aMalloc_	(size_t size, const char *file, int32 line, const char *func);
	void* aCalloc_	(size_t num, size_t size, const char *file, int32 line, const char *func);
	void* aRealloc_	(void *p, size_t size, const char *file, int32 line, const char *func);
	char* aStrdup_	(const char *p, const char *file, int32 line, const char *func);
	void  aFree_	(void *p, const char *file, int32 line, const char *func);

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
#define CREATE2( result, type, number, file, line, func ) (result) = (type*)aCalloc2( ( number ), sizeof( type ), ( file ), ( line ), ( func ) )
#define RECREATE2( result, type, number, file, line, func ) (result) = (type*)aRealloc2( ( result ), sizeof( type ) * ( number ), ( file ), ( line ), ( func ) )

////////////////////////////////////////////////

void malloc_memory_check(void);
bool malloc_verify_ptr(void* ptr);
size_t malloc_usage (void);
void malloc_init (void);
void malloc_final (void);

#endif /* MALLOC_HPP */
