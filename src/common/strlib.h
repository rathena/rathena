#ifndef _J_STR_LIB_H_
#define _J_STR_LIB_H_
#define J_MAX_MALLOC_SIZE 65535
// String function library.
// code by Jioh L. Jung (ziozzang@4wish.net)
// This code is under license "BSD"
char* jstrescape (char* pt);
char* jstrescapecpy (char* pt,char* spt);
int jmemescapecpy (char* pt,char* spt, int size);

// custom functions
int remove_control_chars(unsigned char *);
#endif
