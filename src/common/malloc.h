#ifndef _MALLOC_H_
#define _MALLOC_H_

#include <stdlib.h>

#if defined(GCOLLECT)

#include "gc.h"
#define aMalloc(n) GC_MALLOC(n)
#define aCalloc(m,n) _bcalloc(m,n)
#define aRealloc(p,n) GC_REALLOC(p,n)

extern void * _bcalloc(size_t, size_t);

#elif defined(BCHECK)
#define aMalloc(n) malloc(n)
#define aCalloc(m,n) calloc(m,n)
#define aRealloc(p,n) realloc(p,n)
#else

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ ""
# endif
#endif

#define ALC_MARK __FILE__, __LINE__, __func__

void* aMalloc_( size_t size, const char *file, int line, const char *func );
void* aCalloc_( size_t num, size_t size, const char *file, int line, const char *func );
void* aRealloc_( void *p, size_t size, const char *file, int line, const char *func );

#define aMalloc(n) aMalloc_(n,ALC_MARK)
#define aCalloc(m,n) aCalloc_(m,n,ALC_MARK)
#define aRealloc(p,n) aRealloc_(p,n,ALC_MARK)

#endif

#endif
