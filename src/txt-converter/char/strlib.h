#ifndef _J_STR_H_
#define _J_STR_H_
#define J_MAX_MALLOC_SIZE 65535
//string functions.
//code by Jioh L. Jung
unsigned char* jstrescape (unsigned char* pt);
unsigned char* jstrescapecpy (unsigned char* pt,unsigned char* spt);
int jmemescapecpy (unsigned char* pt,unsigned char* spt, int size);
#endif
