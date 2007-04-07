// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_SOCKET_H_
#define _SOCKET_H_

#ifdef WIN32
	#include <windows.h>
	typedef long in_addr_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif

#include "../common/cbasetypes.h"
#include <time.h>


// socket I/O macros
#ifdef TURBO
#define RFIFOVAR(fd) rbPtr ## fd
#define WFIFOVAR(fd) wbPtr ## fd
#define RFIFOHEAD(fd) uint8 *RFIFOVAR(fd) = session[fd]->rdata+session[fd]->rdata_pos
#define WFIFOHEAD(fd, x) uint8 *WFIFOVAR(fd) = ( (fd) > 0 && session[fd] ? session[fd]->wdata+session[fd]->wdata_size : NULL )
#define RFIFOP(fd,pos) ( &RFIFOVAR(fd) + (pos) )
#define WFIFOP(fd,pos) ( &WFIFOVAR(fd) + (pos) )
#else
#define RFIFOHEAD(fd)
#define WFIFOHEAD(fd, size) do{ if((fd) && session[fd]->wdata_size + (size) > session[fd]->max_wdata ) realloc_writefifo(fd, size); }while(0)
#define RFIFOP(fd,pos) (session[fd]->rdata + session[fd]->rdata_pos + (pos))
#define WFIFOP(fd,pos) (session[fd]->wdata + session[fd]->wdata_size + (pos))
#endif

#define RFIFOB(fd,pos) (*(uint8*)RFIFOP(fd,pos))
#define WFIFOB(fd,pos) (*(uint8*)WFIFOP(fd,pos))
#define RFIFOW(fd,pos) (*(uint16*)RFIFOP(fd,pos))
#define WFIFOW(fd,pos) (*(uint16*)WFIFOP(fd,pos))
#define RFIFOL(fd,pos) (*(uint32*)RFIFOP(fd,pos))
#define WFIFOL(fd,pos) (*(uint32*)WFIFOP(fd,pos))
#define RFIFOSPACE(fd) (session[fd]->max_rdata - session[fd]->rdata_size)
#define WFIFOSPACE(fd) (session[fd]->max_wdata - session[fd]->wdata_size)

#define RFIFOREST(fd)  (session[fd]->rdata_size - session[fd]->rdata_pos)
#define RFIFOFLUSH(fd) \
	do { \
		if(session[fd]->rdata_size == session[fd]->rdata_pos){ \
			session[fd]->rdata_size = session[fd]->rdata_pos = 0; \
		} else { \
			session[fd]->rdata_size -= session[fd]->rdata_pos; \
			memmove(session[fd]->rdata, session[fd]->rdata+session[fd]->rdata_pos, session[fd]->rdata_size); \
			session[fd]->rdata_pos = 0; \
		} \
	} while(0)

// buffer I/O macros
#define RBUFP(p,pos) (((uint8*)(p)) + (pos))
#define RBUFB(p,pos) (*(uint8*)RBUFP((p),(pos)))
#define RBUFW(p,pos) (*(uint16*)RBUFP((p),(pos)))
#define RBUFL(p,pos) (*(uint32*)RBUFP((p),(pos)))

#define WBUFP(p,pos) (((uint8*)(p)) + (pos))
#define WBUFB(p,pos) (*(uint8*)WBUFP((p),(pos)))
#define WBUFW(p,pos) (*(uint16*)WBUFP((p),(pos)))
#define WBUFL(p,pos) (*(uint32*)WBUFP((p),(pos)))

#define TOB(n) ((uint8)((n)&UINT8_MAX))
#define TOW(n) ((uint16)((n)&UINT16_MAX))
#define TOL(n) ((uint32)((n)&UINT32_MAX))


// Struct declaration
typedef int (*RecvFunc)(int fd);
typedef int (*SendFunc)(int fd);
typedef int (*ParseFunc)(int fd);

struct socket_data {
	unsigned char eof;
	unsigned char *rdata, *wdata;
	size_t max_rdata, max_wdata;
	size_t rdata_size, wdata_size;
	size_t rdata_pos;
	time_t rdata_tick; // time of last receive (for detecting timeouts)
	uint32 client_addr; // remote client address (zero for s2s connections)
	void* session_data;
	RecvFunc func_recv;
	SendFunc func_send;
	ParseFunc func_parse;
};


// Data prototype declaration

extern struct socket_data* session[FD_SETSIZE];

extern int fd_max;

extern time_t last_tick;
extern time_t stall_time;

//////////////////////////////////
// some checking on sockets
extern int session_isValid(int fd);
extern int session_isActive(int fd);
//////////////////////////////////

// Function prototype declaration

int make_listen_bind(uint32 ip, uint16 port);
int make_connection(uint32 ip, uint16 port);
int realloc_fifo(int fd, unsigned int rfifo_size, unsigned int wfifo_size);
int realloc_writefifo(int fd, size_t addition);
int WFIFOSET(int fd, int len);
int RFIFOSKIP(int fd, int len);

int do_sendrecv(int next);
int do_parsepacket(void);
void do_close(int fd);
void socket_init(void);
void socket_final(void);

extern void flush_fifo(int fd);
extern void flush_fifos(void);
extern void set_nonblocking(int fd, int yes);

void set_defaultparse(ParseFunc defaultparse);

// hostname/ip conversion functions
uint32 host2ip(const char* hostname);
const char* ip2str(uint32 ip, char ip_str[16]);
uint32 str2ip(const char* ip_str);
#define CONVIP(ip) (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip>>0)&0xFF

int socket_getips(uint32* ips, int max);

extern uint32 addr_[16];   // ip addresses of local host (host byte order)
extern int naddr_;   // # of ip addresses

#endif /* _SOCKET_H_ */
