#include <stdio.h>
#include <stdlib.h>
#include "malloc.h"

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
