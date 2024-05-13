// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <ctime>

#include <config/core.hpp>

#ifdef WIN32
	#include "winapi.hpp"
	typedef long in_addr_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif

#include "cbasetypes.hpp"
#include "malloc.hpp"
#include "timer.hpp" // t_tick

#ifndef MAXCONN
	#define MAXCONN FD_SETSIZE
#endif

#define FIFOSIZE_SERVERLINK 256*1024

// socket I/O macros
#define RFIFOHEAD(fd)
#define WFIFOHEAD( fd, size ) \
	do{ \
		if( ( fd ) && session[( fd )]->wdata_size + ( size ) > session[( fd )]->max_wdata ){ \
			_realloc_writefifo( ( fd ), ( size ), ALC_MARK ); \
		} \
	}while( false )
#define RFIFOP(fd,pos) (session[fd]->rdata + session[fd]->rdata_pos + (pos))
#define WFIFOP(fd,pos) (session[fd]->wdata + session[fd]->wdata_size + (pos))

#define RFIFOCP(fd,pos) ((char*)RFIFOP(fd,pos))
#define WFIFOCP(fd,pos) ((char*)WFIFOP(fd,pos))
#define RFIFOB(fd,pos) (*(uint8*)RFIFOP(fd,pos))
#define WFIFOB(fd,pos) (*(uint8*)WFIFOP(fd,pos))
#define RFIFOW(fd,pos) (*(uint16*)RFIFOP(fd,pos))
#define WFIFOW(fd,pos) (*(uint16*)WFIFOP(fd,pos))
#define RFIFOL(fd,pos) (*(uint32*)RFIFOP(fd,pos))
#define WFIFOL(fd,pos) (*(uint32*)WFIFOP(fd,pos))
#define RFIFOF(fd,pos) (*(float*)RFIFOP(fd,pos))
#define WFIFOF(fd,pos) (*(float*)WFIFOP(fd,pos))
#define RFIFOQ(fd,pos) (*(uint64*)RFIFOP(fd,pos))
#define WFIFOQ(fd,pos) (*(uint64*)WFIFOP(fd,pos))
#define RFIFOSPACE(fd) (session[fd]->max_rdata - session[fd]->rdata_size)
#define WFIFOSPACE(fd) (session[fd]->max_wdata - session[fd]->wdata_size)

#define RFIFOREST(fd)  (session[fd]->flag.eof ? 0 : session[fd]->rdata_size - session[fd]->rdata_pos)
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
#define RBUFCP(p,pos) ((char*)RBUFP((p),(pos)))
#define RBUFB(p,pos) (*(uint8*)RBUFP((p),(pos)))
#define RBUFW(p,pos) (*(uint16*)RBUFP((p),(pos)))
#define RBUFL(p,pos) (*(uint32*)RBUFP((p),(pos)))
#define RBUFQ(p,pos) (*(uint64*)RBUFP((p),(pos)))

#define WBUFP(p,pos) (((uint8*)(p)) + (pos))
#define WBUFCP(p,pos) ((char*)WBUFP((p),(pos)))
#define WBUFB(p,pos) (*(uint8*)WBUFP((p),(pos)))
#define WBUFW(p,pos) (*(uint16*)WBUFP((p),(pos)))
#define WBUFL(p,pos) (*(uint32*)WBUFP((p),(pos)))
#define WBUFQ(p,pos) (*(uint64*)WBUFP((p),(pos)))

#define TOB(n) ((uint8)((n)&UINT8_MAX))
#define TOW(n) ((uint16)((n)&UINT16_MAX))
#define TOL(n) ((uint32)((n)&UINT32_MAX))


// Struct declaration
typedef int (*RecvFunc)(int fd);
typedef int (*SendFunc)(int fd);
typedef int (*ParseFunc)(int fd);

struct socket_data
{
	struct {
		unsigned char eof : 1;
		unsigned char server : 1;
		unsigned char ping : 2;
	} flag;

	uint32 client_addr; // remote client address

	uint8 *rdata, *wdata;
	size_t max_rdata, max_wdata;
	size_t rdata_size, wdata_size;
	size_t rdata_pos;
	time_t rdata_tick; // time of last recv (for detecting timeouts); zero when timeout is disabled
	time_t wdata_tick; // time of last send (for detecting timeouts);

	RecvFunc func_recv;
	SendFunc func_send;
	ParseFunc func_parse;

	void* session_data; // stores application-specific data related to the session
};


// Data prototype declaration

extern struct socket_data* session[MAXCONN];

extern int fd_max;

extern time_t last_tick;
extern time_t stall_time;

//////////////////////////////////
// some checking on sockets
extern bool session_isValid(int fd);
extern bool session_isActive(int fd);
//////////////////////////////////

// Function prototype declaration

int make_listen_bind(uint32 ip, uint16 port);
int make_connection(uint32 ip, uint16 port, bool silent, int timeout);
#define realloc_fifo( fd, rfifo_size, wfifo_size ) _realloc_fifo( ( fd ), ( rfifo_size ), ( wfifo_size ), ALC_MARK )
#define realloc_writefifo( fd, addition ) _realloc_writefifo( ( fd ), ( addition ), ALC_MARK )
int _realloc_fifo( int fd, unsigned int rfifo_size, unsigned int wfifo_size, const char* file, int line, const char* func );
int _realloc_writefifo( int fd, size_t addition, const char* file, int line, const char* func );
int WFIFOSET(int fd, size_t len);
int RFIFOSKIP(int fd, size_t len);

int do_sockets(t_tick next);
void do_close(int fd);
void socket_init(void);
void socket_final(void);

extern void flush_fifo(int fd);
extern void flush_fifos(void);
extern void set_nonblocking(int fd, unsigned long yes);

void set_defaultparse(ParseFunc defaultparse);


/// Server operation request
enum chrif_req_op {
	// Char-server <-> login-server oepration
	CHRIF_OP_LOGIN_BLOCK = 1,
	CHRIF_OP_LOGIN_BAN,
	CHRIF_OP_LOGIN_UNBLOCK,
	CHRIF_OP_LOGIN_UNBAN,
	CHRIF_OP_LOGIN_CHANGESEX,
	CHRIF_OP_LOGIN_VIP,

	// Char-server operation
	CHRIF_OP_BAN,
	CHRIF_OP_UNBAN,
	CHRIF_OP_CHANGECHARSEX,
};


// hostname/ip conversion functions
uint32 host2ip(const char* hostname);
const char* ip2str(uint32 ip, char ip_str[16]);
uint32 str2ip(const char* ip_str);
#define CONVIP(ip) ((ip)>>24)&0xFF,((ip)>>16)&0xFF,((ip)>>8)&0xFF,((ip)>>0)&0xFF
#define MAKEIP(a,b,c,d) (uint32)( ( ( (a)&0xFF ) << 24 ) | ( ( (b)&0xFF ) << 16 ) | ( ( (c)&0xFF ) << 8 ) | ( ( (d)&0xFF ) << 0 ) )
uint16 ntows(uint16 netshort);

int socket_getips(uint32* ips, int max);

extern uint32 addr_[16];   // ip addresses of local host (host byte order)
extern int naddr_;   // # of ip addresses

void set_eof(int fd);

/// Use a shortlist of sockets instead of iterating all sessions for sockets
/// that have data to send or need eof handling.
/// Adapted to use a static array instead of a linked list.
///
/// @author Buuyo-tama
#define SEND_SHORTLIST

#ifdef SEND_SHORTLIST
// Add a fd to the shortlist so that it'll be recognized as a fd that needs
// sending done on it.
void send_shortlist_add_fd(int fd);
// Do pending network sends (and eof handling) from the shortlist.
void send_shortlist_do_sends();
#endif

// Reuseable global packet buffer to prevent too many allocations
// Take socket.cpp::socket_max_client_packet into consideration
extern int8 packet_buffer[UINT16_MAX];

template <typename P>
bool socket_send( int fd, P& packet ){
	if( !session_isActive( fd ) ){
		return false;
	}

	WFIFOHEAD( fd, sizeof( P ) );
	memcpy( WFIFOP( fd, 0 ), &packet, sizeof( P ) );
	WFIFOSET( fd, sizeof( P ) );

	return true;
}

template <typename P>
bool socket_send( int fd, P* packet ){
	if( !session_isActive( fd ) ){
		return false;
	}

	WFIFOHEAD( fd, packet->packetLength );
	memcpy( WFIFOP( fd, 0 ), packet, packet->packetLength );
	WFIFOSET( fd, packet->packetLength );

	return true;
}

#endif /* SOCKET_HPP */
