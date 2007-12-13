// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef WIN32
	#include <winsock2.h>
	#include <io.h>
#else
	#include <errno.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <net/if.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <netdb.h>
	#include <arpa/inet.h>

	#ifndef SIOCGIFCONF
	#include <sys/sockio.h> // SIOCGIFCONF on Solaris, maybe others? [Shinomori]
	#endif
#endif

/////////////////////////////////////////////////////////////////////
#if defined(WIN32)
/////////////////////////////////////////////////////////////////////
// windows portability layer 

typedef int socklen_t;

#define sErrno WSAGetLastError()
#define S_ENOTSOCK WSAENOTSOCK
#define S_EWOULDBLOCK WSAEWOULDBLOCK
#define S_EINTR WSAEINTR
#define S_ECONNABORTED WSAECONNABORTED

#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH

// global array of sockets (emulating linux)
// fd is the position in the array
static SOCKET sock_arr[FD_SETSIZE];
static int sock_arr_len = 0;

/// Returns the socket associated with the target fd.
///
/// @param fd Target fd.
/// @return Socket
#define fd2sock(fd) sock_arr[fd]

/// Returns the first fd associated with the socket.
/// Returns -1 if the socket is not found.
///
/// @param s Socket
/// @return Fd or -1
int sock2fd(SOCKET s)
{
	int fd;

	// search for the socket
	for( fd = 1; fd < sock_arr_len; ++fd )
		if( sock_arr[fd] == s )
			break;// found the socket
	if( fd == sock_arr_len )
		return -1;// not found
	return fd;
}


/// Inserts the socket into the global array of sockets.
/// Returns a new fd associated with the socket.
/// If there are too many sockets it closes the socket, sets an error and 
//  returns -1 instead.
/// Since fd 0 is reserved, it returns values in the range [1,FD_SETSIZE[.
///
/// @param s Socket
/// @return New fd or -1
int sock2newfd(SOCKET s)
{
	int fd;

	// find an empty position
	for( fd = 1; fd < sock_arr_len; ++fd )
		if( sock_arr[fd] == INVALID_SOCKET )
			break;// empty position
	if( fd == ARRAYLENGTH(sock_arr) )
	{// too many sockets
		closesocket(s);
		WSASetLastError(WSAEMFILE);
		return -1;
	}
	sock_arr[fd] = s;
	if( sock_arr_len <= fd )
		sock_arr_len = fd+1;
	return fd;
}

int sAccept(int fd, struct sockaddr* addr, int* addrlen)
{
	SOCKET s;

	// accept connection
	s = accept(fd2sock(fd), addr, addrlen);
	if( s == INVALID_SOCKET )
		return -1;// error
	return sock2newfd(s);
}

int sClose(int fd)
{
	int ret = closesocket(fd2sock(fd));
	fd2sock(fd) = INVALID_SOCKET;
	return ret;
}

int sSocket(int af, int type, int protocol)
{
	SOCKET s;

	// create socket
	s = socket(af,type,protocol);
	if( s == INVALID_SOCKET )
		return -1;// error
	return sock2newfd(s);
}

#define sBind(fd,name,namelen) bind(fd2sock(fd),name,namelen)
#define sConnect(fd,name,namelen) connect(fd2sock(fd),name,namelen)
#define sIoctl(fd,cmd,argp) ioctlsocket(fd2sock(fd),cmd,argp)
#define sListen(fd,backlog) listen(fd2sock(fd),backlog)
#define sRecv(fd,buf,len,flags) recv(fd2sock(fd),buf,len,flags)
#define sSelect select
#define sSend(fd,buf,len,flags) send(fd2sock(fd),buf,len,flags)
#define sSetsockopt(fd,level,optname,optval,optlen) setsockopt(fd2sock(fd),level,optname,optval,optlen)
#define sShutdown(fd,how) shutdown(fd2sock(fd),how)
#define sFD_SET(fd,set) FD_SET(fd2sock(fd),set)
#define sFD_CLR(fd,set) FD_CLR(fd2sock(fd),set)
#define sFD_ISSET(fd,set) FD_ISSET(fd2sock(fd),set)
#define sFD_ZERO FD_ZERO

/////////////////////////////////////////////////////////////////////
#else
/////////////////////////////////////////////////////////////////////
// nix portability layer

#define SOCKET_ERROR (-1)

#define sErrno errno
#define S_ENOTSOCK EBADF
#define S_EWOULDBLOCK EAGAIN
#define S_EINTR EINTR
#define S_ECONNABORTED ECONNABORTED

#define sAccept accept
#define sClose close
#define sSocket socket

#define sBind bind
#define sConnect connect
#define sIoctl ioctl
#define sListen listen
#define sRecv recv
#define sSelect select
#define sSend send
#define sSetsockopt setsockopt
#define sShutdown shutdown
#define sFD_SET FD_SET
#define sFD_CLR FD_CLR
#define sFD_ISSET FD_ISSET
#define sFD_ZERO FD_ZERO

/////////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////////

fd_set readfds;
int fd_max;
time_t last_tick;
time_t stall_time = 60;

uint32 addr_[16];   // ip addresses of local host (host byte order)
int naddr_ = 0;   // # of ip addresses

// initial recv buffer size (this will also be the max. size)
// biggest known packet: S 0153 <len>.w <emblem data>.?B -> 24x24 256 color .bmp (0153 + len.w + 1618/1654/1756 bytes)
#define RFIFO_SIZE (2*1024)
// initial send buffer size (will be resized as needed)
#define WFIFO_SIZE (16*1024)

struct socket_data* session[FD_SETSIZE];

#ifdef SEND_SHORTLIST
int send_shortlist_array[FD_SETSIZE];// we only support FD_SETSIZE sockets, limit the array to that
int send_shortlist_count = 0;// how many fd's are in the shortlist
uint32 send_shortlist_set[(FD_SETSIZE+31)/32];// to know if specific fd's are already in the shortlist
#endif

static int create_session(int fd, RecvFunc func_recv, SendFunc func_send, ParseFunc func_parse);

#ifndef MINICORE
	int ip_rules = 1;
	static int connect_check(uint32 ip);
#endif


/*======================================
 *	CORE : Default processing functions
 *--------------------------------------*/
int null_recv(int fd) { return 0; }
int null_send(int fd) { return 0; }
int null_parse(int fd) { return 0; }

ParseFunc default_func_parse = null_parse;

void set_defaultparse(ParseFunc defaultparse)
{
	default_func_parse = defaultparse;
}


/*======================================
 *	CORE : Socket options
 *--------------------------------------*/
void set_nonblocking(int fd, unsigned long yes)
{
	// FIONBIO Use with a nonzero argp parameter to enable the nonblocking mode of socket s. 
	// The argp parameter is zero if nonblocking is to be disabled. 
	if( sIoctl(fd, FIONBIO, &yes) != 0 )
		ShowError("set_nonblocking: Failed to set socket #%d to non-blocking mode (code %d) - Please report this!!!\n", fd, sErrno);
}

void setsocketopts(int fd)
{
	int yes = 1; // reuse fix
#if !defined(WIN32)
	// set SO_REAUSEADDR to true, unix only. on windows this option causes
	// the previous owner of the socket to give up, which is not desirable
	// in most cases, neither compatible with unix.
	sSetsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof(yes));
#ifdef SO_REUSEPORT
	sSetsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof(yes));
#endif
#endif

	// Set the socket into no-delay mode; otherwise packets get delayed for up to 200ms, likely creating server-side lag.
	// The RO protocol is mainly single-packet request/response, plus the FIFO model already does packet grouping anyway.
	sSetsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));

	// force the socket into no-wait, graceful-close mode (should be the default, but better make sure)
	//(http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/closesocket_2.asp)
	{
	struct linger opt;
	opt.l_onoff = 0; // SO_DONTLINGER
	opt.l_linger = 0; // Do not care
	if( sSetsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt)) )
		ShowWarning("setsocketopts: Unable to set SO_LINGER mode for connection #%d!\n", fd);
	}
}

/*======================================
 *	CORE : Socket Sub Function
 *--------------------------------------*/
void set_eof(int fd)
{
	if( session_isActive(fd) )
	{
#ifdef SEND_SHORTLIST
		// Add this socket to the shortlist for eof handling.
		send_shortlist_add_fd(fd);
#endif
		session[fd]->eof = 1;
	}
}

int recv_to_fifo(int fd)
{
	int len;

	if( !session_isActive(fd) )
		return -1;

	len = sRecv(fd, (char *) session[fd]->rdata + session[fd]->rdata_size, (int)RFIFOSPACE(fd), 0);

	if( len == SOCKET_ERROR )
	{//An exception has occured
		if( sErrno != S_EWOULDBLOCK ) {
			//ShowDebug("recv_to_fifo: code %d, closing connection #%d\n", sErrno, fd);
			set_eof(fd);
		}
		return 0;
	}

	if( len == 0 )
	{//Normal connection end.
		set_eof(fd);
		return 0;
	}

	session[fd]->rdata_size += len;
	session[fd]->rdata_tick = last_tick;
	return 0;
}

int send_from_fifo(int fd)
{
	int len;

	if( !session_isValid(fd) )
		return -1;

	if( session[fd]->wdata_size == 0 )
		return 0; // nothing to send

	len = sSend(fd, (const char *) session[fd]->wdata, (int)session[fd]->wdata_size, 0);

	if( len == SOCKET_ERROR )
	{//An exception has occured
		if( sErrno != S_EWOULDBLOCK ) {
			//ShowDebug("send_from_fifo: error %d, ending connection #%d\n", sErrno, fd);
			session[fd]->wdata_size = 0; //Clear the send queue as we can't send anymore. [Skotlex]
			set_eof(fd);
		}
		return 0;
	}

	if( len > 0 )
	{
		// some data could not be transferred?
		// shift unsent data to the beginning of the queue
		if( (size_t)len < session[fd]->wdata_size )
			memmove(session[fd]->wdata, session[fd]->wdata + len, session[fd]->wdata_size - len);

		session[fd]->wdata_size -= len;
	}

	return 0;
}

/// Best effort - there's no warranty that the data will be sent.
void flush_fifo(int fd)
{
	if(session[fd] != NULL)
		session[fd]->func_send(fd);
}

void flush_fifos(void)
{
	int i;
	for(i = 1; i < fd_max; i++)
		flush_fifo(i);
}

/*======================================
 *	CORE : Connection functions
 *--------------------------------------*/
int connect_client(int listen_fd)
{
	int fd;
	struct sockaddr_in client_address;
	socklen_t len;

	len = sizeof(client_address);

	fd = sAccept(listen_fd, (struct sockaddr*)&client_address, &len);
	if ( fd == -1 ) {
		ShowError("connect_client: accept failed (code %d)!\n", sErrno);
		return -1;
	}
	if( fd == 0 )
	{// reserved
		ShowError("connect_client: Socket #0 is reserved - Please report this!!!\n");
		sClose(fd);
		return -1;
	}
	if( fd >= FD_SETSIZE )
	{// socket number too big
		ShowError("connect_client: New socket #%d is greater than can we handle! Increase the value of FD_SETSIZE (currently %d) for your OS to fix this!\n", fd, FD_SETSIZE);
		sClose(fd);
		return -1;
	}

	setsocketopts(fd);
	set_nonblocking(fd, 1);

#ifndef MINICORE
	if( ip_rules && !connect_check(ntohl(client_address.sin_addr.s_addr)) ) {
		do_close(fd);
		return -1;
	}
#endif

	if( fd_max <= fd ) fd_max = fd + 1;
	sFD_SET(fd,&readfds);

	create_session(fd, recv_to_fifo, send_from_fifo, default_func_parse);
	session[fd]->client_addr = ntohl(client_address.sin_addr.s_addr);

	return fd;
}

int make_listen_bind(uint32 ip, uint16 port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = sSocket(AF_INET, SOCK_STREAM, 0);

	if( fd == -1 )
	{
		ShowError("make_listen_bind: socket creation failed (code %d)!\n", sErrno);
		exit(EXIT_FAILURE);
	}
	if( fd == 0 )
	{// reserved
		ShowError("make_listen_bind: Socket #0 is reserved - Please report this!!!\n");
		sClose(fd);
		return -1;
	}
	if( fd >= FD_SETSIZE )
	{// socket number too big
		ShowError("make_listen_bind: New socket #%d is greater than can we handle! Increase the value of FD_SETSIZE (currently %d) for your OS to fix this!\n", fd, FD_SETSIZE);
		sClose(fd);
		return -1;
	}

	setsocketopts(fd);
	set_nonblocking(fd, 1);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = htonl(ip);
	server_address.sin_port        = htons(port);

	result = sBind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
	if( result == SOCKET_ERROR ) {
		ShowError("make_listen_bind: bind failed (socket #%d, code %d)!\n", fd, sErrno);
		exit(EXIT_FAILURE);
	}
	result = sListen(fd,5);
	if( result == SOCKET_ERROR ) {
		ShowError("make_listen_bind: listen failed (socket #%d, code %d)!\n", fd, sErrno);
		exit(EXIT_FAILURE);
	}

	if(fd_max <= fd) fd_max = fd + 1;
	sFD_SET(fd, &readfds);

	create_session(fd, connect_client, null_send, null_parse);
	session[fd]->rdata_tick = 0; // disable timeouts on this socket
	session[fd]->client_addr = 0;

	return fd;
}

int make_connection(uint32 ip, uint16 port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = sSocket(AF_INET, SOCK_STREAM, 0);

	if (fd == -1) {
		ShowError("make_connection: socket creation failed (code %d)!\n", sErrno);
		return -1;
	}
	if( fd == 0 )
	{// reserved
		ShowError("make_connection: Socket #0 is reserved - Please report this!!!\n");
		sClose(fd);
		return -1;
	}
	if( fd >= FD_SETSIZE )
	{// socket number too big
		ShowError("make_connection: New socket #%d is greater than can we handle! Increase the value of FD_SETSIZE (currently %d) for your OS to fix this!\n", fd, FD_SETSIZE);
		sClose(fd);
		return -1;
	}

	setsocketopts(fd);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = htonl(ip);
	server_address.sin_port        = htons(port);

	ShowStatus("Connecting to %d.%d.%d.%d:%i\n", CONVIP(ip), port);

	result = sConnect(fd, (struct sockaddr *)(&server_address), sizeof(struct sockaddr_in));
	if( result == SOCKET_ERROR ) {
		ShowError("make_connection: connect failed (socket #%d, code %d)!\n", fd, sErrno);
		do_close(fd);
		return -1;
	}
	//Now the socket can be made non-blocking. [Skotlex]
	set_nonblocking(fd, 1);

	if (fd_max <= fd) fd_max = fd + 1;
	sFD_SET(fd,&readfds);

	create_session(fd, recv_to_fifo, send_from_fifo, default_func_parse);
	session[fd]->client_addr = 0;

	return fd;
}

static int create_session(int fd, RecvFunc func_recv, SendFunc func_send, ParseFunc func_parse)
{
	CREATE(session[fd], struct socket_data, 1);
	CREATE(session[fd]->rdata, unsigned char, RFIFO_SIZE);
	CREATE(session[fd]->wdata, unsigned char, WFIFO_SIZE);
	session[fd]->max_rdata  = RFIFO_SIZE;
	session[fd]->max_wdata  = WFIFO_SIZE;
	session[fd]->func_recv  = func_recv;
	session[fd]->func_send  = func_send;
	session[fd]->func_parse = func_parse;
	session[fd]->rdata_tick = last_tick;
	return 0;
}

static int delete_session(int fd)
{
	if (fd <= 0 || fd >= FD_SETSIZE)
		return -1;
	if (session[fd]) {
		aFree(session[fd]->rdata);
		aFree(session[fd]->wdata);
		aFree(session[fd]->session_data);
		aFree(session[fd]);
		session[fd] = NULL;
	}
	return 0;
}

int realloc_fifo(int fd, unsigned int rfifo_size, unsigned int wfifo_size)
{
	if( !session_isValid(fd) )
		return 0;

	if( session[fd]->max_rdata != rfifo_size && session[fd]->rdata_size < rfifo_size) {
		RECREATE(session[fd]->rdata, unsigned char, rfifo_size);
		session[fd]->max_rdata  = rfifo_size;
	}

	if( session[fd]->max_wdata != wfifo_size && session[fd]->wdata_size < wfifo_size) {
		RECREATE(session[fd]->wdata, unsigned char, wfifo_size);
		session[fd]->max_wdata  = wfifo_size;
	}
	return 0;
}

int realloc_writefifo(int fd, size_t addition)
{
	size_t newsize;

	if( !session_isValid(fd) ) // might not happen
		return 0;

	if( session[fd]->wdata_size + addition  > session[fd]->max_wdata )
	{	// grow rule; grow in multiples of WFIFO_SIZE
		newsize = WFIFO_SIZE;
		while( session[fd]->wdata_size + addition > newsize ) newsize += newsize;
	}
	else
	if( session[fd]->max_wdata >= FIFOSIZE_SERVERLINK)
	{
		//Inter-server adjust. [Skotlex]
		if ((session[fd]->wdata_size+addition)*4 < session[fd]->max_wdata)
			newsize = session[fd]->max_wdata / 2;
		else
			return 0; //No change
	}
	else
	if( session[fd]->max_wdata > WFIFO_SIZE && (session[fd]->wdata_size+addition)*4 < session[fd]->max_wdata )
	{	// shrink rule, shrink by 2 when only a quater of the fifo is used, don't shrink below 4*addition
		newsize = session[fd]->max_wdata / 2;
	}
	else // no change
		return 0;

	// crash prevention for bugs that cause the send queue to fill up in an infinite loop
	if( newsize > 5*1024*1024 ) // 5 MB is way beyond reasonable
	{
		ShowError("realloc_writefifo: session #%d's send buffer was overloaded! Disconnecting...\n", fd);
		// drop all data (but the space will still be available)
		session[fd]->wdata_size = 0;
		// request disconnect
		set_eof(fd);
		return 0;
	}

	RECREATE(session[fd]->wdata, unsigned char, newsize);
	session[fd]->max_wdata  = newsize;

	return 0;
}

/// advance the RFIFO cursor (marking 'len' bytes as processed)
int RFIFOSKIP(int fd, size_t len)
{
    struct socket_data *s;

	if ( !session_isActive(fd) )
		return 0;

	s = session[fd];

	if ( s->rdata_size < s->rdata_pos + len ) {
		ShowError("RFIFOSKIP: skipped past end of read buffer! Adjusting from %d to %d (session #%d)\n", len, RFIFOREST(fd), fd);
		len = RFIFOREST(fd);
	}

	s->rdata_pos = s->rdata_pos + len;
	return 0;
}

/// advance the WFIFO cursor (marking 'len' bytes for sending)
int WFIFOSET(int fd, size_t len)
{
	size_t newreserve;
	struct socket_data* s = session[fd];

	if( !session_isValid(fd) || s->wdata == NULL )
		return 0;

	// we have written len bytes to the buffer already before calling WFIFOSET
	if(s->wdata_size+len > s->max_wdata)
	{	// actually there was a buffer overflow already
		uint32 ip = s->client_addr;
		ShowFatalError("WFIFOSET: Write Buffer Overflow. Connection %d (%d.%d.%d.%d) has written %d bytes on a %d/%d bytes buffer.\n", fd, CONVIP(ip), len, s->wdata_size, s->max_wdata);
		ShowDebug("Likely command that caused it: 0x%x\n", (*(unsigned short*)(s->wdata + s->wdata_size)));
		// no other chance, make a better fifo model
		exit(EXIT_FAILURE);
	}

	s->wdata_size += len;
	//If the interserver has 200% of its normal size full, flush the data.
	if(s->max_wdata >= FIFOSIZE_SERVERLINK &&
		s->wdata_size >= 2*FIFOSIZE_SERVERLINK)
		flush_fifo(fd);

	// always keep a WFIFO_SIZE reserve in the buffer
	// For inter-server connections, let the reserve be 1/4th of the link size.
	newreserve = s->wdata_size + (s->max_wdata >= FIFOSIZE_SERVERLINK ? FIFOSIZE_SERVERLINK / 4 : WFIFO_SIZE);

	// readjust the buffer to the newly chosen size
	realloc_writefifo(fd, newreserve);

#ifdef SEND_SHORTLIST
	send_shortlist_add_fd(fd);
#endif

	return 0;
}

int do_sockets(int next)
{
	fd_set rfd;
	struct timeval timeout;
	int ret,i;

	// PRESEND Timers are executed before do_sendrecv and can send packets and/or set sessions to eof.
	// Send remaining data and process client-side disconnects here.
#ifdef SEND_SHORTLIST
	send_shortlist_do_sends();
#else
	for (i = 1; i < fd_max; i++)
	{
		if(!session[i])
			continue;

		if(session[i]->wdata_size)
			session[i]->func_send(i);
	}
#endif

	// can timeout until the next tick
	timeout.tv_sec  = next/1000;
	timeout.tv_usec = next%1000*1000;

	memcpy(&rfd, &readfds, sizeof(rfd));
	ret = sSelect(fd_max, &rfd, NULL, NULL, &timeout);

	if( ret == SOCKET_ERROR )
	{
		if( sErrno != S_EINTR )
		{
			ShowFatalError("do_sockets: select() failed, error code %d!\n", sErrno);
			exit(EXIT_FAILURE);
		}
		return 0; // interrupted by a signal, just loop and try again
	}

	last_tick = time(NULL);

#if defined(WIN32)
	// on windows, enumerating all members of the fd_set is way faster if we access the internals
	for( i = 0; i < (int)rfd.fd_count; ++i )
	{
		int fd = sock2fd(rfd.fd_array[i]);
		if( session[fd] )
			session[fd]->func_recv(fd);
	}
#else
	// otherwise assume that the fd_set is a bit-array and enumerate it in a standard way
	//TODO: select() returns the number of readable sockets; use that to exit the fd_max loop faster
	for (i = 1; i < fd_max; i++)
	{
		if(sFD_ISSET(i,&rfd) && session[i])
			session[i]->func_recv(i);
	}
#endif

	// POSTSEND Send remaining data and handle eof sessions.
#ifdef SEND_SHORTLIST
	send_shortlist_do_sends();
#else
	for (i = 1; i < fd_max; i++)
	{
		if(!session[i])
			continue;

		if(session[i]->wdata_size)
			session[i]->func_send(i);

		if(session[i]->eof) //func_send can't free a session, this is safe.
		{	//Finally, even if there is no data to parse, connections signalled eof should be closed, so we call parse_func [Skotlex]
			session[i]->func_parse(i); //This should close the session immediately.
		}
	}
#endif

	// parse input data on each socket
	for(i = 1; i < fd_max; i++)
	{
		if(!session[i])
			continue;

		if (session[i]->rdata_tick && DIFF_TICK(last_tick, session[i]->rdata_tick) > stall_time) {
			ShowInfo("Session #%d timed out\n", i);
			set_eof(i);
		}

		session[i]->func_parse(i);

		if(!session[i])
			continue;

		// after parse, check client's RFIFO size to know if there is an invalid packet (too big and not parsed)
		if (session[i]->rdata_size == RFIFO_SIZE && session[i]->max_rdata == RFIFO_SIZE) {
			set_eof(i);
			continue;
		}
		RFIFOFLUSH(i);
	}

	return 0;
}

//////////////////////////////
#ifndef MINICORE
//////////////////////////////
// IP rules and DDoS protection

typedef struct _connect_history {
	struct _connect_history* next;
	uint32 ip;
	uint32 tick;
	int count;
	unsigned ddos : 1;
} ConnectHistory;

typedef struct _access_control {
	uint32 ip;
	uint32 mask;
} AccessControl;

enum _aco {
	ACO_DENY_ALLOW,
	ACO_ALLOW_DENY,
	ACO_MUTUAL_FAILURE
};

static AccessControl* access_allow = NULL;
static AccessControl* access_deny = NULL;
static int access_order    = ACO_DENY_ALLOW;
static int access_allownum = 0;
static int access_denynum  = 0;
static int access_debug    = 0;
static int ddos_count      = 10;
static int ddos_interval   = 3*1000;
static int ddos_autoreset  = 10*60*1000;
/// Connection history, an array of linked lists.
/// The array's index for any ip is ip&0xFFFF
static ConnectHistory* connect_history[0x10000];

static int connect_check_(uint32 ip);

/// Verifies if the IP can connect. (with debug info)
/// @see connect_check_()
static int connect_check(uint32 ip)
{
	int result = connect_check_(ip);
	if( access_debug ) {
		ShowMessage("connect_check: Connection from %d.%d.%d.%d %s\n", CONVIP(ip),result ? "allowed." : "denied!");
	}
	return result;
}

/// Verifies if the IP can connect.
///  0      : Connection Rejected
///  1 or 2 : Connection Accepted
static int connect_check_(uint32 ip)
{
	ConnectHistory* hist = connect_history[ip&0xFFFF];
	int i;
	int is_allowip = 0;
	int is_denyip = 0;
	int connect_ok = 0;

	// Search the allow list
	for( i=0; i < access_allownum; ++i ){
		if( (ip & access_allow[i].mask) == (access_allow[i].ip & access_allow[i].mask) ){
			if( access_debug ){
				ShowMessage("connect_check: Found match from allow list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
					CONVIP(ip),
					CONVIP(access_allow[i].ip),
					CONVIP(access_allow[i].mask));
			}
			is_allowip = 1;
			break;
		}
	}
	// Search the deny list
	for( i=0; i < access_denynum; ++i ){
		if( (ip & access_deny[i].mask) == (access_deny[i].ip & access_deny[i].mask) ){
			if( access_debug ){
				ShowMessage("connect_check: Found match from deny list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
					CONVIP(ip),
					CONVIP(access_deny[i].ip),
					CONVIP(access_deny[i].mask));
			}
			is_denyip = 1;
			break;
		}
	}
	// Decide connection status
	//  0 : Reject
	//  1 : Accept
	//  2 : Unconditional Accept (accepts even if flagged as DDoS)
	switch(access_order) {
	case ACO_DENY_ALLOW:
	default:
		if( is_denyip )
			connect_ok = 0; // Reject
		else if( is_allowip )
			connect_ok = 2; // Unconditional Accept
		else
			connect_ok = 1; // Accept
		break;
	case ACO_ALLOW_DENY:
		if( is_allowip )
			connect_ok = 2; // Unconditional Accept
		else if( is_denyip )
			connect_ok = 0; // Reject
		else
			connect_ok = 1; // Accept
		break;
	case ACO_MUTUAL_FAILURE:
		if( is_allowip && !is_denyip )
			connect_ok = 2; // Unconditional Accept
		else
			connect_ok = 0; // Reject
		break;
	}

	// Inspect connection history
	while( hist ) {
		if( ip == hist->ip )
		{// IP found
			if( hist->ddos )
			{// flagged as DDoS
				return (connect_ok == 2 ? 1 : 0);
			} else if( DIFF_TICK(gettick(),hist->tick) < ddos_interval )
			{// connection within ddos_interval
				hist->tick = gettick();
				if( hist->count++ >= ddos_count )
				{// DDoS attack detected
					hist->ddos = 1;
					ShowWarning("connect_check: DDoS Attack detected from %d.%d.%d.%d!\n", CONVIP(ip));
					return (connect_ok == 2 ? 1 : 0);
				}
				return connect_ok;
			} else
			{// not within ddos_interval, clear data
				hist->tick  = gettick();
				hist->count = 0;
				return connect_ok;
			}
		}
		hist = hist->next;
	}
	// IP not found, add to history
	CREATE(hist, ConnectHistory, 1);
	memset(hist, 0, sizeof(ConnectHistory));
	hist->ip   = ip;
	hist->tick = gettick();
	hist->next = connect_history[ip&0xFFFF];
	connect_history[ip&0xFFFF] = hist;
	return connect_ok;
}

/// Timer function.
/// Deletes old connection history records.
static int connect_check_clear(int tid, unsigned int tick, int id, int data)
{
	int i;
	int clear = 0;
	int list  = 0;
	ConnectHistory root;
	ConnectHistory* prev_hist;
	ConnectHistory* hist;

	for( i=0; i < 0x10000 ; ++i ){
		prev_hist = &root;
		root.next = hist = connect_history[i];
		while( hist ){
			if( (!hist->ddos && DIFF_TICK(tick,hist->tick) > ddos_interval*3) ||
					(hist->ddos && DIFF_TICK(tick,hist->tick) > ddos_autoreset) )
			{// Remove connection history
				prev_hist->next = hist->next;
				aFree(hist);
				hist = prev_hist->next;
				clear++;
			} else {
				prev_hist = hist;
				hist = hist->next;
			}
			list++;
		}
		connect_history[i] = root.next;
	}
	if( access_debug ){
		ShowMessage("connect_check_clear: Cleared %d of %d from IP list.\n", clear, list);
	}
	return list;
}

/// Parses the ip address and mask and puts it into acc.
/// Returns 1 is successful, 0 otherwise.
int access_ipmask(const char* str, AccessControl* acc)
{
	uint32 ip;
	uint32 mask;
	unsigned int a[4];
	unsigned int m[4];
	int n;

	if( strcmp(str,"all") == 0 ) {
		ip   = 0;
		mask = 0;
	} else {
		if( ((n=sscanf(str,"%u.%u.%u.%u/%u.%u.%u.%u",a,a+1,a+2,a+3,m,m+1,m+2,m+3)) != 8 && // not an ip + standard mask
				(n=sscanf(str,"%u.%u.%u.%u/%u",a,a+1,a+2,a+3,m)) != 5 && // not an ip + bit mask
				(n=sscanf(str,"%u.%u.%u.%u",a,a+1,a+2,a+3)) != 4 ) || // not an ip
				a[0] > 255 || a[1] > 255 || a[2] > 255 || a[3] > 255 || // invalid ip
				(n == 8 && (m[0] > 255 || m[1] > 255 || m[2] > 255 || m[3] > 255)) || // invalid standard mask
				(n == 5 && m[0] > 32) ){ // invalid bit mask
			return 0;
		}
		ip = (uint32)(a[0] | (a[1] << 8) | (a[2] << 16) | (a[3] << 24));
		if( n == 8 )
		{// standard mask
			mask = (uint32)(a[0] | (a[1] << 8) | (a[2] << 16) | (a[3] << 24));
		} else if( n == 5 )
		{// bit mask
			mask = 0;
			while( m[0] ){
				mask = (mask >> 1) | 0x80000000;
				--m[0];
			}
			mask = ntohl(mask);
		} else
		{// just this ip
			mask = 0xFFFFFFFF;
		}
	}
	if( access_debug ){
		ShowMessage("access_ipmask: Loaded IP:%d.%d.%d.%d mask:%d.%d.%d.%d\n", CONVIP(ip), CONVIP(mask));
	}
	acc->ip   = ip;
	acc->mask = mask;
	return 1;
}
//////////////////////////////
#endif
//////////////////////////////

int socket_config_read(const char* cfgName)
{
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp = fopen(cfgName, "r");
	if(fp == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		if(sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (!strcmpi(w1, "stall_time"))
			stall_time = atoi(w2);
#ifndef MINICORE
		else if (!strcmpi(w1, "enable_ip_rules")) {
			ip_rules = config_switch(w2);
		} else if (!strcmpi(w1, "order")) {
			if (!strcmpi(w2, "deny,allow"))
				access_order = ACO_DENY_ALLOW;
			else if (!strcmpi(w2, "allow,deny"))
				access_order = ACO_ALLOW_DENY;
			else if (!strcmpi(w2, "mutual-failure"))
				access_order = ACO_MUTUAL_FAILURE;
		} else if (!strcmpi(w1, "allow")) {
			RECREATE(access_allow, AccessControl, access_allownum+1);
			if (access_ipmask(w2, &access_allow[access_allownum]))
				++access_allownum;
			else
				ShowError("socket_config_read: Invalid ip or ip range '%s'!\n", line);
		} else if (!strcmpi(w1, "deny")) {
			RECREATE(access_deny, AccessControl, access_denynum+1);
			if (access_ipmask(w2, &access_deny[access_denynum]))
				++access_denynum;
			else
				ShowError("socket_config_read: Invalid ip or ip range '%s'!\n", line);
		}
		else if (!strcmpi(w1,"ddos_interval"))
			ddos_interval = atoi(w2);
		else if (!strcmpi(w1,"ddos_count"))
			ddos_count = atoi(w2);
		else if (!strcmpi(w1,"ddos_autoreset"))
			ddos_autoreset = atoi(w2);
		else if (!strcmpi(w1,"debug"))
			access_debug = config_switch(w2);
#endif
		else if (!strcmpi(w1, "import"))
			socket_config_read(w2);
	}

	fclose(fp);
	return 0;
}


void socket_final(void)
{
	int i;
#ifndef MINICORE
	ConnectHistory* hist;
	ConnectHistory* next_hist;

	for( i=0; i < 0x10000; ++i ){
		hist = connect_history[i];
		while( hist ){
			next_hist = hist->next;
			aFree(hist);
			hist = next_hist;
		}
	}
	if( access_allow )
		aFree(access_allow);
	if( access_deny )
		aFree(access_deny);
#endif

	for (i = 1; i < fd_max; i++) {
		if(session[i])
			delete_session(i);
	}

	// session[0] のダミーデータを削除
	aFree(session[0]->rdata);
	aFree(session[0]->wdata);
	aFree(session[0]);
}

/// Closes a socket.
void do_close(int fd)
{
	flush_fifo(fd); // Try to send what's left (although it might not succeed since it's a nonblocking socket)
	sFD_CLR(fd, &readfds);// this needs to be done before closing the socket
	sShutdown(fd, SHUT_RDWR); // Disallow further reads/writes
	sClose(fd); // We don't really care if these closing functions return an error, we are just shutting down and not reusing this socket.
	if (session[fd]) delete_session(fd);
}

/// Retrieve local ips in host byte order.
/// Uses loopback is no address is found.
int socket_getips(uint32* ips, int max)
{
	int num = 0;

	if( ips == NULL || max <= 0 )
		return 0;

#ifdef WIN32
	{
		char fullhost[255];
		u_long** a;
		struct hostent* hent;

		// XXX This should look up the local IP addresses in the registry
		// instead of calling gethostbyname. However, the way IP addresses
		// are stored in the registry is annoyingly complex, so I'll leave
		// this as T.B.D. [Meruru]
		if( gethostname(fullhost, sizeof(fullhost)) == SOCKET_ERROR )
		{
			ShowError("socket_getips: No hostname defined!\n");
			return 0;
		}
		else
		{
			hent = gethostbyname(fullhost);
			if( hent == NULL ){
				ShowError("socket_getips: Cannot resolve our own hostname to an IP address\n");
				return 0;
			}
			a = (u_long**)hent->h_addr_list;
			for( ; a[num] != NULL && num < max; ++num)
				ips[num] = (uint32)ntohl(*a[num]);
		}
	}
#else // not WIN32
	{
		int pos;
		int fd;
		char buf[2*16*sizeof(struct ifreq)];
		struct ifconf ic;
		struct ifreq* ir;
		struct sockaddr_in* a;
		u_long ad;

		fd = sSocket(AF_INET, SOCK_STREAM, 0);

		// The ioctl call will fail with Invalid Argument if there are more
		// interfaces than will fit in the buffer
		ic.ifc_len = sizeof(buf);
		ic.ifc_buf = buf;
		if( sIoctl(fd, SIOCGIFCONF, &ic) == -1 )
		{
			ShowError("socket_getips: SIOCGIFCONF failed!\n");
			return 0;
		}
		else
		{
			for( pos=0; pos < ic.ifc_len && num < max; )
			{
				ir = (struct ifreq*)(buf+pos);
				a = (struct sockaddr_in*) &(ir->ifr_addr);
				if( a->sin_family == AF_INET ){
					ad = ntohl(a->sin_addr.s_addr);
					if( ad != INADDR_LOOPBACK && ad != INADDR_ANY )
						ips[num++] = (uint32)ad;
				}
	#if (defined(BSD) && BSD >= 199103) || defined(_AIX) || defined(__APPLE__)
				pos += ir->ifr_addr.sa_len + sizeof(ir->ifr_name);
	#else// not AIX or APPLE
				pos += sizeof(struct ifreq);
	#endif//not AIX or APPLE
			}
		}
		sClose(fd);
	}
#endif // not W32

	// Use loopback if no ips are found
	if( num == 0 )
		ips[num++] = (uint32)INADDR_LOOPBACK;

	return num;
}

void socket_init(void)
{
	char *SOCKET_CONF_FILENAME = "conf/packet_athena.conf";

#ifdef WIN32
	{// Start up windows networking
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 0);
		if( WSAStartup(wVersionRequested, &wsaData) != 0 )
		{
			ShowError("socket_init: WinSock not available!\n");
			return;
		}
		if( LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0 )
		{
			printf("socket_init: WinSock version mismatch (2.0 or compatible required)!\n");
			return;
		}
	}
#endif

	// Get initial local ips
	naddr_ = socket_getips(addr_,16);

	sFD_ZERO(&readfds);
#if defined(SEND_SHORTLIST)
	memset(send_shortlist_set, 0, sizeof(send_shortlist_set));
#endif

	socket_config_read(SOCKET_CONF_FILENAME);

	// initialise last send-receive tick
	last_tick = time(NULL);

	// session[0] is now currently used for disconnected sessions of the map server, and as such,
	// should hold enough buffer (it is a vacuum so to speak) as it is never flushed. [Skotlex]
	create_session(0, null_recv, null_send, null_parse);

#ifndef MINICORE
	// Delete old connection history every 5 minutes
	memset(connect_history, 0, sizeof(connect_history));
	add_timer_func_list(connect_check_clear, "connect_check_clear");
	add_timer_interval(gettick()+1000, connect_check_clear, 0, 0, 5*60*1000);
#endif
}


bool session_isValid(int fd)
{
	return ( fd > 0 && fd < FD_SETSIZE && session[fd] != NULL );
}

bool session_isActive(int fd)
{
	return ( session_isValid(fd) && !session[fd]->eof );
}

// Resolves hostname into a numeric ip.
uint32 host2ip(const char* hostname)
{
	struct hostent* h = gethostbyname(hostname);
	return (h != NULL) ? ntohl(*(uint32*)h->h_addr) : 0;
}

// Converts a numeric ip into a dot-formatted string.
// Result is placed either into a user-provided buffer or a static system buffer.
const char* ip2str(uint32 ip, char ip_str[16])
{
	struct in_addr addr;
	addr.s_addr = htonl(ip);
	return (ip_str == NULL) ? inet_ntoa(addr) : strncpy(ip_str, inet_ntoa(addr), 16);
}

// Converts a dot-formatted ip string into a numeric ip.
uint32 str2ip(const char* ip_str)
{
	return ntohl(inet_addr(ip_str));
}

// Reorders bytes from network to little endian (Windows).
// Neccessary for sending port numbers to the RO client until Gravity notices that they forgot ntohs() calls.
uint16 ntows(uint16 netshort)
{
	return ((netshort & 0xFF) << 8) | ((netshort & 0xFF00) >> 8);
}

#ifdef SEND_SHORTLIST
// Add a fd to the shortlist so that it'll be recognized as a fd that needs
// sending or eof handling.
void send_shortlist_add_fd(int fd)
{
	int i;
	int bit;

	if( fd < 0 || fd >= FD_SETSIZE )
		return;// out of range
	i = fd/32;
	bit = fd%32;
	if( (send_shortlist_set[i]>>bit)&1 )
		return;// already in the list

	// set the bit
	send_shortlist_set[i] |= 1<<bit;
	// Add to the end of the shortlist array.
	send_shortlist_array[send_shortlist_count++] = fd;
}

// Do pending network sends and eof handling from the shortlist.
void send_shortlist_do_sends()
{
	int i = 0;

	// Assume all or most of the fd's don't remain in the shortlist
	memset(send_shortlist_set, 0, sizeof(send_shortlist_set));

	while( i < send_shortlist_count )
	{
		int fd = send_shortlist_array[i];

		// If this session still exists, perform send operations on it and
		// check for the eof state.
		if( session[fd] )
		{
			// Send data
			if( session[fd]->wdata_size )
				session[fd]->func_send(fd);

			// If it's been marked as eof, call the parse func on it so that
			// the socket will be immediately closed.
			if( session[fd]->eof )
				session[fd]->func_parse(fd);

			// If the session still exists, is not eof and has things left to
			// be sent from it we'll keep it in the shortlist.
			if( session[fd] && !session[fd]->eof && session[fd]->wdata_size )
			{
				send_shortlist_set[fd/32] |= 1<<(fd%32);
				++i;
				continue;
			}
		}

		// Remove fd from shortlist, move the last fd to the current position
		send_shortlist_array[i] = send_shortlist_array[--send_shortlist_count];
	}
}
#endif
