#ifndef _LOCK_H_
#define _LOCK_H_

FILE* lock_fopen(const char* filename,int *info);
int   lock_fclose(FILE *fp,const char* filename,int *info);

#endif

