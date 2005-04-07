#ifndef _BUFFER_H_
#define _BUFFER_H_

// Full credit for this goes to Shinomori [Ajarn]

#ifdef __GNUC__ // GCC has variable length arrays

#define CREATE_BUFFER(name, type, size) type name[size]
#define DELETE_BUFFER(name)

#else // others don't, so we emulate them

#define CREATE_BUFFER(name, type, size) type *name=(type*)aCalloc(size,sizeof(type))
#define DELETE_BUFFER(name) aFree(name);name=NULL

#endif

#endif
