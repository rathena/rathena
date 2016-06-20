// Copyright (c) rAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _rA_NETBUFFER_H_
#define _rA_NETBUFFER_H_

#include "cbasetypes.h"

typedef struct netbuf{
	sysint	pool;				// The pool ID this buffer belongs to,
								// is set to -1 if its an emergency allocated buffer 
	
	struct netbuf *next;		// Used by Network system.

	volatile int32 refcnt;		// Internal Refcount, it gets lowered every call to netbuffer_put, 
								// if its getting zero, the buffer will returned back to the pool
								// and can be reused.

	int32	dataPos;	// Current Offset
						// Used only for Reading (recv job)
						// write cases are using the sessions local datapos member due to
						// shared write buffer support.
	
	int32	dataLen;	// read buffer case:
						//	The length expected to read to.
						//	when this->dataPos == dateLen, read job has been completed.
						// write buffer case:
						//	The lngth of data in te buffer
						//	when s->dataPos == dataLen, write job has been completed
						//
						// Note:
						//	leftBytes = (dateLen - dataPos)
						//
						//	Due to shared buffer support
						//	dataPos gets not used in write case (each connection has its local offset)
						//

	// The Bufferspace itself.
	char buf[32];
} *netbuf;


void netbuffer_init();
void netbuffer_final();

/**
 * Gets a netbuffer that has atleast (sz) byes space.
 *
 * @note: The netbuffer system guarantees that youll always recevie a buffer.
 *			no check for null is required!
 *
 * @param sz - minimum size needed.
 *
 * @return pointer to netbuf struct
 */
netbuf netbuffer_get( sysint sz );


/** 
 * Returns the given netbuffer (decreases refcount, if its 0 - the buffer will get returned to the pool)
 *
 * @param buf - the buffer to return 
 */
void netbuffer_put( netbuf buf );


/** 
 * Increases the Refcount on the given buffer 
 * (used for areasends .. etc)
 *
 */
void netbuffer_incref( netbuf buf );


// Some Useful macros
#define NBUFP(netbuf,pos) (((uint8*)(netbuf->buf)) + (pos))
#define NBUFB(netbuf,pos) (*(uint8*)((netbuf->buf) + (pos)))
#define NBUFW(netbuf,pos) (*(uint16*)((netbuf->buf) + (pos)))
#define NBUFL(netbuf,pos) (*(uint32*)((netbuf->buf) + (pos)))



#endif
