#ifndef _MALLOC_H_
#define _MALLOC_H_

#include <stdlib.h>

#if defined(DMALLOC)

#	include "dmalloc.h"
#	define aMalloc(size) \
		dmalloc_malloc(__FILE__, __LINE__, (size), DMALLOC_FUNC_MALLOC, 0, 0)
#	define aMallocA(size) \
		dmalloc_malloc(__FILE__, __LINE__, (size), DMALLOC_FUNC_MALLOC, 0, 0)
#	define aCallocA(count,size) \
		dmalloc_malloc(__FILE__, __LINE__, (count)*(size), DMALLOC_FUNC_CALLOC, 0, 0)
#	define aCalloc(count,size) \
		dmalloc_malloc(__FILE__, __LINE__, (count)*(size), DMALLOC_FUNC_CALLOC, 0, 0)
#	define aRealloc(ptr,size) \
		dmalloc_realloc(__FILE__, __LINE__, (ptr), (size), DMALLOC_FUNC_REALLOC, 0)
#	define aFree(ptr) free(ptr)
#	define aStrdup(ptr) strdup(ptr)

#elif defined(GCOLLECT)

#	include "gc.h"
#	define aMalloc(n) GC_MALLOC(n)
#	define aMallocA(n) GC_MALLOC_ATOMIC(n)
#	define aCallocA(m,n) _bcallocA(m,n)
#	define aCalloc(m,n) _bcalloc(m,n)
#	define aRealloc(p,n) GC_REALLOC(p,n)
#	define aFree(n) GC_FREE(n)
#	define aStrdup(n) _bstrdup(n)

	extern void * _bcalloc(size_t, size_t);
	extern void * _bcallocA(size_t, size_t);
	extern char * _bstrdup(const char *);

#elif defined(BCHECK)

#	define aMalloc(n) malloc(n)
#	define aMallocA(n) malloc(n)
#	define aCalloc(m,n) calloc(m,n)
#	define aCallocA(m,n) calloc(m,n)
#	define aRealloc(p,n) realloc(p,n)
#	define aFree(n) free(n)
#	define aStrdup(n) strdup(n)

#else

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ ""
# endif
#endif

#	define ALC_MARK __FILE__, __LINE__, __func__

	void* aMalloc_( size_t size, const char *file, int line, const char *func );
	void* aCalloc_( size_t num, size_t size, const char *file, int line, const char *func );
	void* aRealloc_( void *p, size_t size, const char *file, int line, const char *func );
	void  aFree_( void *p, const char *file, int line, const char *func );
	char* aStrdup_( const void *p, const char *file, int line, const char *func );

#	define aMalloc(n) aMalloc_(n,ALC_MARK)
#	define aMallocA(n) aMalloc_(n,ALC_MARK)
#	define aCalloc(m,n) aCalloc_(m,n,ALC_MARK)
#	define aCallocA(m,n) aCalloc_(m,n,ALC_MARK)
#	define aRealloc(p,n) aRealloc_(p,n,ALC_MARK)
#	define aStrdup(p) aStrdup_(p,ALC_MARK)
#	define aFree(p) do { aFree_(p,ALC_MARK); if(p != NULL) { p = NULL; } } while(0)

#endif

int do_init_memmgr(const char* file);

#endif
