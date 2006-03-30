// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _J_STR_LIB_H_
#define _J_STR_LIB_H_
#define J_MAX_MALLOC_SIZE 65535
// String function library.
// code by Jioh L. Jung (ziozzang@4wish.net)
// This code is under license "BSD"
char* jstrescape (char* pt);
char* jstrescapecpy (char* pt,char* spt);
int jmemescapecpy (char* pt,char* spt, int size);

#if !defined(HAVE_mit_thread) && !defined(HAVE_STRTOK_R)
#define strtok_r(s,delim,save_ptr) athena_strtok_r((s),(delim),(save_ptr))
char *athena_strtok_r(char *s1, const char *s2, char **lasts);
#endif

// custom functions
int remove_control_chars(unsigned char *);
char *trim(char *str, const char *delim);
#endif
