#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malloc.h"

#if !defined(DMALLOC) && !defined(GCOLLECT) && !defined(BCHECK)

void* aMalloc_( size_t size, const char *file, int line, const char *func )
{
	void *ret;

//	printf("%s:%d: in func %s: malloc %d\n",file,line,func,size);
	ret=malloc(size);
	if(ret==NULL){
		printf("%s:%d: in func %s: malloc error out of memory!\n",file,line,func);
		exit(1);

	}
	return ret;
}
void* aCalloc_( size_t num, size_t size, const char *file, int line, const char *func )
{
	void *ret;

//	printf("%s:%d: in func %s: calloc %d %d\n",file,line,func,num,size);
	ret=calloc(num,size);
	if(ret==NULL){
		printf("%s:%d: in func %s: calloc error out of memory!\n",file,line,func);
		exit(1);

	}
	return ret;
}

void* aRealloc_( void *p, size_t size, const char *file, int line, const char *func )
{
	void *ret;

//	printf("%s:%d: in func %s: realloc %p %d\n",file,line,func,p,size);
	ret=realloc(p,size);
	if(ret==NULL){
		printf("%s:%d: in func %s: realloc error out of memory!\n",file,line,func);
		exit(1);

	}
	return ret;
}

#endif


#if defined(GCOLLECT)

void * _bcallocA(size_t size, size_t cnt) {
	void *ret = aMallocA(size * cnt);
	memset(ret, 0, size * cnt);
	return ret;
}

void * _bcalloc(size_t size, size_t cnt) {
	void *ret = aMalloc(size * cnt);
	memset(ret, 0, size * cnt);
	return ret;
}
#endif

char * _bstrdup(const char *chr) {
	int len = strlen(chr);
	char *ret = (char*)aMalloc(len + 1);
	strcpy(ret, chr);
	return ret;
}
