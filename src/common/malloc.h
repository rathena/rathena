// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MALLOC_H_
#define _MALLOC_H_

//#define MEMSET_TURBO

#ifndef __NETBSD__
#if __STDC_VERSION__ < 199901L
#	if __GNUC__ >= 2
#		define __func__ __FUNCTION__
#	else
#		define __func__ ""
#	endif
#endif
#endif
#define ALC_MARK __FILE__, __LINE__, __func__

//////////////////////////////////////////////////////////////////////
// Whether to use Athena's built-in Memory Manager (enabled by default)
// To disable just comment the following line
#if !defined(DMALLOC) && !defined(BCHECK)
	#define USE_MEMMGR
#endif
// Whether to enable Memory Manager's logging
#define LOG_MEMMGR

#ifdef USE_MEMMGR

#	define aMalloc(n)		_mmalloc(n,ALC_MARK)
#	define aMallocA(n)		_mmalloc(n,ALC_MARK)
#	define aCalloc(m,n)		_mcalloc(m,n,ALC_MARK)
#	define aCallocA(m,n)	_mcalloc(m,n,ALC_MARK)
#	define aRealloc(p,n)	_mrealloc(p,n,ALC_MARK)
#	define aStrdup(p)		_mstrdup(p,ALC_MARK)
#	define aFree(p)			_mfree(p,ALC_MARK)

	void* _mmalloc	(size_t, const char *, int, const char *);
	void* _mcalloc	(size_t, size_t, const char *, int, const char *);
	void* _mrealloc	(void *, size_t, const char *, int, const char *);
	char* _mstrdup	(const char *, const char *, int, const char *);
	void  _mfree	(void *, const char *, int, const char *);	

#else

#	define aMalloc(n)		aMalloc_(n,ALC_MARK)
#	define aMallocA(n)		aMallocA_(n,ALC_MARK)
#	define aCalloc(m,n)		aCalloc_(m,n,ALC_MARK)
#	define aCallocA(m,n)	aCallocA_(m,n,ALC_MARK)
#	define aRealloc(p,n)	aRealloc_(p,n,ALC_MARK)
#	define aStrdup(p)		aStrdup_(p,ALC_MARK)
#	define aFree(p)			aFree_(p,ALC_MARK)

	void* aMalloc_	(size_t, const char *, int, const char *);
	void* aMallocA_	(size_t, const char *, int, const char *);
	void* aCalloc_	(size_t, size_t, const char *, int, const char *);
	void* aCallocA_	(size_t, size_t, const char *, int, const char *);
	void* aRealloc_	(void *, size_t, const char *, int, const char *);
	char* aStrdup_	(const char *, const char *, int, const char *);
	void  aFree_	(void *, const char *, int, const char *);

#endif

////////////// Memory Managers //////////////////

#ifdef MEMWATCH

#	include "memwatch.h"
#	define MALLOC(n)	mwMalloc(n,__FILE__, __LINE__)
#	define MALLOCA(n)	mwMalloc(n,__FILE__, __LINE__)
#	define CALLOC(m,n)	mwCalloc(m,n,__FILE__, __LINE__)
#	define CALLOCA(m,n)	mwCalloc(m,n,__FILE__, __LINE__)
#	define REALLOC(p,n)	mwRealloc(p,n,__FILE__, __LINE__)
#	define STRDUP(p)	mwStrdup(p,__FILE__, __LINE__)
#	define FREE(p)		mwFree(p,__FILE__, __LINE__)

#elif defined(DMALLOC)

#	include "dmalloc.h"
#	define MALLOC(n)	dmalloc_malloc(__FILE__, __LINE__, (n), DMALLOC_FUNC_MALLOC, 0, 0)
#	define MALLOCA(n)	dmalloc_malloc(__FILE__, __LINE__, (n), DMALLOC_FUNC_MALLOC, 0, 0)
#	define CALLOC(m,n)	dmalloc_malloc(__FILE__, __LINE__, (m)*(n), DMALLOC_FUNC_CALLOC, 0, 0)
#	define CALLOCA(m,n)	dmalloc_malloc(__FILE__, __LINE__, (m)*(n), DMALLOC_FUNC_CALLOC, 0, 0)
#	define REALLOC(p,n)	dmalloc_realloc(__FILE__, __LINE__, (p), (n), DMALLOC_FUNC_REALLOC, 0)
#	define STRDUP(p)	strdup(p)
#	define FREE(p)		free(p)

#elif defined(GCOLLECT)

#	include "gc.h"
#	define MALLOC(n)	GC_MALLOC(n)
#	define MALLOCA(n)	GC_MALLOC_ATOMIC(n)
#	define CALLOC(m,n)	_bcalloc(m,n)
#	define CALLOCA(m,n)	_bcallocA(m,n)
#	define REALLOC(p,n)	GC_REALLOC(p,n)
#	define STRDUP(p)	_bstrdup(p)
#	define FREE(p)		GC_FREE(p)

	void * _bcalloc(size_t, size_t);
	void * _bcallocA(size_t, size_t);
	char * _bstrdup(const char *);

#elif defined(BCHECK)

#	define MALLOC(n)	malloc(n)
#	define MALLOCA(n)	malloc(n)
#	define CALLOC(m,n)	calloc(m,n)
#	define CALLOCA(m,n)	calloc(m,n)
#	define REALLOC(p,n)	realloc(p,n)
#	define STRDUP(p)	strdup(p)
#	define FREE(p)		free(p)

#else

#	define MALLOC(n)	malloc(n)
#	define MALLOCA(n)	malloc(n)
#	define CALLOC(m,n)	calloc(m,n)
#	define CALLOCA(m,n)	calloc(m,n)
#	define REALLOC(p,n)	realloc(p,n)
#	define STRDUP(p)	strdup(p)
#	define FREE(p)		free(p)

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
#define CREATE(result, type, number) (result) = (type *) aCalloc ((number), sizeof(type));

#define CREATE_A(result, type, number) (result) = (type *) aCallocA ((number), sizeof(type));

#define RECREATE(result, type, number) (result) = (type *) aRealloc ((result), sizeof(type) * (number)); 	

////////////////////////////////////////////////

unsigned int malloc_usage (void);
#ifndef INLINE
	#ifdef _WIN32
		#define INLINE 
	#else
		#define INLINE inline
	#endif
#endif
#ifdef MEMSET_TURBO
	INLINE void malloc_tsetdword(void *dest, int value, int count);
	INLINE void malloc_tsetword(void *dest, short value, int count);
	INLINE void malloc_set(void *dest, int value, int size);
#else
	#define malloc_tsetdword(x,y,z) memset(x,y,z)
	#define malloc_tsetword(x,y,z) memset(x,y,z)
	#define malloc_set(x,y,z) memset(x,y,z)
#endif
void malloc_init (void);
void malloc_final (void);

#endif
