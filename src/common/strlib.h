#ifndef _J_STR_LIB_H_
#define _J_STR_LIB_H_
#define J_MAX_MALLOC_SIZE 65535
// String function library.
// code by Jioh L. Jung (ziozzang@4wish.net)
// This code is under license "BSD"
unsigned char* jstrescape (unsigned char* pt);
unsigned char* jstrescapecpy (unsigned char* pt,unsigned char* spt);
int jmemescapecpy (unsigned char* pt,unsigned char* spt, int size);
#endif
