// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
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

// portability layer 
#ifdef _WIN32
	typedef int socklen_t;

	#define s_errno WSAGetLastError()
	#define S_ENOTSOCK WSAENOTSOCK
	#define S_EWOULDBLOCK WSAEWOULDBLOCK
	#define S_ECONNABORTED WSAECONNABORTED

	#define SHUT_RD   SD_RECEIVE
	#define SHUT_WR   SD_SEND
	#define SHUT_RDWR SD_BOTH
#else
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	#define ioctlsocket ioctl
	#define closesocket close

	#define s_errno errno
	#define S_ENOTSOCK EBADF
	#define S_EWOULDBLOCK EAGAIN
	#define S_ECONNABORTED ECONNABORTED
#endif

#include <fcntl.h>
#include <string.h>

#include "../common/socket.h"
#include "../common/mmo.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"

fd_set readfds;
int fd_max;
time_t last_tick;
time_t stall_time = 60;
int ip_rules = 1;

uint32 addr_[16];   // ip addresses of local host (host byte order)
int naddr_ = 0;   // # of ip addresses

#ifndef TCP_FRAME_LEN
#define TCP_FRAME_LEN	1024
#endif

static int mode_neg=1;
static size_t frame_size=TCP_FRAME_LEN;

// values derived from freya
// a player that send more than 2k is probably a hacker without be parsed
// biggest known packet: S 0153 <len>.w <emblem data>.?B -> 24x24 256 color .bmp (0153 + len.w + 1618/1654/1756 bytes)
size_t rfifo_size = (16*1024);
size_t wfifo_size = (16*1024);

#define CONVIP(ip) ip&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,ip>>24

struct socket_data *session[FD_SETSIZE];

int create_session(int fd, RecvFunc func_recv, SendFunc func_send, ParseFunc func_parse);

#ifndef MINICORE
static int connect_check(unsigned int ip);
#else
	#define connect_check(n)	1
#endif

/*======================================
 *	CORE : Default processing functions
 *--------------------------------------*/
int null_recv(int fd);
int null_send(int fd);
int null_parse(int fd);

int null_recv(int fd)
{
	return 0;
}

int null_send(int fd)
{
	return 0;
}

int null_parse(int fd)
{
	//ShowMessage("null_parse : %d\n",fd);
	session[fd]->rdata_pos = session[fd]->rdata_size; //RFIFOSKIP(fd, RFIFOREST(fd)); simplify calculation
	return 0;
}

int (*default_func_parse)(int fd) = null_parse;

void set_defaultparse(int (*defaultparse)(int fd))
{
	default_func_parse = defaultparse;
}


/*======================================
 *	CORE : Socket options
 *--------------------------------------*/
void set_nonblocking(int fd, int yes)
{
	// TCP_NODELAY BOOL Disables the Nagle algorithm for send coalescing.
	if(mode_neg)
		setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof yes);
	
	// FIONBIO Use with a nonzero argp parameter to enable the nonblocking mode of socket s. 
	// The argp parameter is zero if nonblocking is to be disabled. 
	if (ioctlsocket(fd, FIONBIO, &yes) != 0)
		ShowError("Couldn't set the socket to non-blocking mode (code %d)!\n", s_errno);
}

static void setsocketopts(int fd)
{
	int yes = 1; // reuse fix
#ifndef WIN32
    // set SO_REAUSEADDR to true, unix only. on windows this option causes
    // the previous owner of the socket to give up, which is not desirable
    // in most cases, neither compatible with unix.
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof(yes));
#ifdef SO_REUSEPORT
	setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof(yes));
#endif
#endif
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof(yes));
//	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &wfifo_size , sizeof(rfifo_size ));
//	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &rfifo_size , sizeof(rfifo_size ));

	// force the socket into no-wait, graceful-close mode (should be the default, but better make sure)
	//(http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/closesocket_2.asp)
	{
	struct linger opt;
	opt.l_onoff = 0; // SO_DONTLINGER
	opt.l_linger = 0; // Do not care
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt)))
		ShowWarning("setsocketopts: Unable to set SO_LINGER mode for connection %d!\n",fd);
	}
}

/*======================================
 *	CORE : Socket Sub Function
 *--------------------------------------
 */
static void set_eof(int fd)
{	//Marks a connection eof and invokes the parse_function to disconnect it right away. [Skotlex]
	if (session_isActive(fd))
		session[fd]->eof = 1;
}

static int recv_to_fifo(int fd)
{
	int len;

	if( !session_isActive(fd) )
		return -1;

	len = recv(fd, (char *) session[fd]->rdata + session[fd]->rdata_size, RFIFOSPACE(fd), 0);

	if (len == SOCKET_ERROR) {
		if (s_errno == S_ECONNABORTED) {
			ShowWarning("recv_to_fifo: Software caused connection abort on session #%d\n", fd);
			FD_CLR(fd, &readfds); //Remove the socket so the select() won't hang on it.
		}
		if (s_errno != S_EWOULDBLOCK) {
			//ShowDebug("recv_to_fifo: error %d, ending connection #%d\n", s_errno, fd);
			set_eof(fd);
		}
		return 0;
	}

	if (len <= 0) {	//Normal connection end.
		set_eof(fd);
		return 0;
	}

	session[fd]->rdata_size += len;
	session[fd]->rdata_tick = last_tick;
	return 0;
}

static int send_from_fifo(int fd)
{
	int len;

	if( !session_isValid(fd) )
		return -1;

	if (session[fd]->wdata_size == 0)
		return 0;

	len = send(fd, (const char *) session[fd]->wdata, session[fd]->wdata_size, 0);

	if (len == SOCKET_ERROR) {
		if (s_errno == S_ECONNABORTED) {
			ShowWarning("send_from_fifo: Software caused connection abort on session #%d\n", fd);
			FD_CLR(fd, &readfds); //Remove the socket so the select() won't hang on it.
		}
		if (s_errno != S_EWOULDBLOCK) {
			//ShowDebug("send_from_fifo: error %d, ending connection #%d\n", s_errno, fd);
			session[fd]->wdata_size = 0; //Clear the send queue as we can't send anymore. [Skotlex]
			set_eof(fd);
		}
		return 0;
	}

	//{ int i; ShowMessage("send %d : ",fd);  for(i=0;i<len;i++){ ShowMessage("%02x ",session[fd]->wdata[i]); } ShowMessage("\n");}
	if(len > 0) {
		if((size_t)len < session[fd]->wdata_size)
			memmove(session[fd]->wdata, session[fd]->wdata + len, session[fd]->wdata_size - len);

		session[fd]->wdata_size -= len;
	}

	return 0;
}

/// Best effort - there's no warranty that the data will be sent.
void flush_fifo(int fd)
{
	if(session[fd] != NULL && session[fd]->func_send == send_from_fifo)
		send_from_fifo(fd);
}

void flush_fifos(void)
{
	int i;
	for(i = 1; i < fd_max; i++)
		if(session[i] != NULL && session[i]->func_send == send_from_fifo)
			send_from_fifo(i);
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

	fd = accept(listen_fd,(struct sockaddr*)&client_address,&len);
	if ( fd == INVALID_SOCKET ) {
		ShowError("accept failed (code %i)!\n", s_errno);
		return -1;
	}

	setsocketopts(fd);
	set_nonblocking(fd, 1);

	if( ip_rules && !connect_check(*(uint32*)(&client_address.sin_addr)) ){
		do_close(fd);
		return -1;
	} else
		FD_SET(fd,&readfds);

	if( fd_max <= fd )
		fd_max = fd + 1;

	create_session(fd, recv_to_fifo, send_from_fifo, default_func_parse);
	session[fd]->client_addr = client_address;
	session[fd]->rdata_tick = last_tick;

	return fd;
}

int make_listen_bind(long ip,int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = (int)socket( AF_INET, SOCK_STREAM, 0 );

	if (fd == INVALID_SOCKET) {
		ShowError("socket() creation failed (code %d)!\n", s_errno);
		exit(1);
	}

	setsocketopts(fd);
	set_nonblocking(fd, 1);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port        = htons((unsigned short)port);

	result = bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
	if( result == SOCKET_ERROR ) {
		ShowError("bind failed (socket %d, code %d)!\n", fd, s_errno);
		exit(1);
	}
	result = listen( fd, 5 );
	if( result == SOCKET_ERROR ) {
		ShowError("listen failed (socket %d, code %d)!\n", fd, s_errno);
		exit(1);
	}
	if ( fd < 0 || fd > FD_SETSIZE ) 
	{ //Crazy error that can happen in Windows? (info from Freya)
		ShowFatalError("listen() returned invalid fd %d!\n",fd);
		exit(1);
	}

	if(fd_max <= fd) fd_max = fd + 1;
	FD_SET(fd, &readfds );

	create_session(fd, connect_client, null_send, null_parse);

	return fd;
}

int make_listen_port(int port)
{
	return make_listen_bind(INADDR_ANY,port);
}

int make_connection(long ip, int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = (int)socket( AF_INET, SOCK_STREAM, 0 );

	if (fd == INVALID_SOCKET) {
		ShowError("socket() creation failed (code %d)!\n", fd, s_errno);
		return -1;
	}

	setsocketopts(fd);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port = htons((unsigned short)port);

	ShowStatus("Connecting to %d.%d.%d.%d:%i\n",
		(ip)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);

	result = connect(fd, (struct sockaddr *)(&server_address), sizeof(struct sockaddr_in));
	if( result == SOCKET_ERROR ) {
		ShowError("connect failed (socket %d, code %d)!\n", fd, s_errno);
		do_close(fd);
		return -1;
	}
	//Now the socket can be made non-blocking. [Skotlex]
	set_nonblocking(fd, 1);

	if (fd_max <= fd)
		fd_max = fd + 1;
	FD_SET(fd,&readfds);

	create_session(fd, recv_to_fifo, send_from_fifo, default_func_parse);
	session[fd]->rdata_tick = last_tick;

	return fd;
}

int create_session(int fd, RecvFunc func_recv, SendFunc func_send, ParseFunc func_parse)
{
	CREATE(session[fd], struct socket_data, 1);
	CREATE(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE(session[fd]->wdata, unsigned char, wfifo_size);
	session[fd]->max_rdata  = rfifo_size;
	session[fd]->max_wdata  = wfifo_size;
	session[fd]->func_recv  = func_recv;
	session[fd]->func_send  = func_send;
	session[fd]->func_parse = func_parse;
	return 0;
}

int delete_session(int fd)
{
	if (fd <= 0 || fd >= FD_SETSIZE)
		return -1;
	FD_CLR(fd, &readfds);
	if (session[fd]) {
		aFree(session[fd]->rdata);
		aFree(session[fd]->wdata);
		aFree(session[fd]->session_data);
		aFree(session[fd]);
		session[fd] = NULL;
	}
	return 0;
}

int realloc_fifo(int fd,unsigned int rfifo_size,unsigned int wfifo_size)
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
	{	// grow rule; grow in multiples of wfifo_size
		newsize = wfifo_size;
		while( session[fd]->wdata_size + addition > newsize ) newsize += newsize;
	}
	else if( session[fd]->max_wdata>=FIFOSIZE_SERVERLINK) {
		//Inter-server adjust. [Skotlex]
		if ((session[fd]->wdata_size+addition)*4 < session[fd]->max_wdata)
			newsize = session[fd]->max_wdata/2;
		else
			return 0; //No change
	} else if( session[fd]->max_wdata > wfifo_size && (session[fd]->wdata_size+addition)*4 < session[fd]->max_wdata )
	{	// shrink rule, shrink by 2 when only a quater of the fifo is used, don't shrink below 4*addition
		newsize = session[fd]->max_wdata / 2;
	}
	else // no change
		return 0;

	RECREATE(session[fd]->wdata, unsigned char, newsize);
	session[fd]->max_wdata  = newsize;

	return 0;
}

int RFIFOSKIP(int fd,int len)
{
    struct socket_data *s;

	if ( !session_isActive(fd) )
		return 0;

	s = session[fd];

	if ( s->rdata_size < s->rdata_pos + len ) {
		//fprintf(stderr,"too many skip\n");
		//exit(1);
		//better than a COMPLETE program abort // TEST! :)
		ShowError("too many skip (%d) now skipped: %d (FD: %d)\n", len, RFIFOREST(fd), fd);
		len = RFIFOREST(fd);
	}
	s->rdata_pos = s->rdata_pos + len;
	return 0;
}

int WFIFOSET(int fd, int len)
{
	size_t newreserve;
	struct socket_data* s = session[fd];

	if( !session_isValid(fd) || s->wdata == NULL )
		return 0;

	// we have written len bytes to the buffer already before calling WFIFOSET
	if(s->wdata_size+len > s->max_wdata)
	{	// actually there was a buffer overflow already
		unsigned char *sin_addr = (unsigned char *)&s->client_addr.sin_addr;
		ShowFatalError("socket: Buffer Overflow. Connection %d (%d.%d.%d.%d) has written %d bytes on a %d/%d bytes buffer.\n", fd,
			sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], len, s->wdata_size, s->max_wdata);
		ShowDebug("Likely command that caused it: 0x%x\n",
			(*(unsigned short*)(s->wdata + s->wdata_size)));
		// no other chance, make a better fifo model
		exit(1);
	}

	s->wdata_size += len;
	// always keep a wfifo_size reserve in the buffer
	// For inter-server connections, let the reserve be 1/4th of the link size.
	newreserve = s->wdata_size + (s->max_wdata >= FIFOSIZE_SERVERLINK ? FIFOSIZE_SERVERLINK / 4 : wfifo_size);

	if(s->wdata_size >= frame_size)
		send_from_fifo(fd);

	// realloc after sending
	// readfifo does not need to be realloced at all
	// Even the inter-server buffer may need reallocating! [Skotlex]
	realloc_writefifo(fd, newreserve);

	return 0;
}

int do_sendrecv(int next)
{
	fd_set rfd;
	struct sockaddr_in	addr_check;
	struct timeval timeout;
	int ret,i,size;

	last_tick = time(0);

	//PRESEND Need to do this to ensure that the clients get something to do
	//which hopefully will cause them to send packets. [Meruru]
	for (i = 1; i < fd_max; i++)
	{
		if(!session[i])
			continue;

		if(session[i]->wdata_size)
			session[i]->func_send(i);
	}

	timeout.tv_sec  = next/1000;
	timeout.tv_usec = next%1000*1000;

	for(memcpy(&rfd, &readfds, sizeof(rfd));
		(ret = select(fd_max, &rfd, NULL, NULL, &timeout))<0;
		memcpy(&rfd, &readfds, sizeof(rfd)))
	{
		if(s_errno != S_ENOTSOCK)
			return 0;

		//Well then the error is due to a bad socket. Lets find and remove it
		//and try again
		for(i = 1; i < fd_max; i++)
		{
			if(!session[i])
			{
				if (FD_ISSET(i, &readfds)) {
					ShowError("Deleting non-cleared session %d\n", i);
					FD_CLR(i, &readfds);
				}
				continue;
			}

			//check the validity of the socket. Does what the last thing did
			//just alot faster [Meruru]
			size = sizeof(struct sockaddr);
			if(getsockname(i,(struct sockaddr*)&addr_check,&size)<0)
				if(s_errno == S_ENOTSOCK)
				{
					ShowError("Deleting invalid session %d\n", i);
				  	//So the code can react accordingly
					session[i]->eof = 1;
					session[i]->func_parse(i);
					delete_session(i); //free the bad session
					continue;
				}

			if (!FD_ISSET(i, &readfds))
				FD_SET(i,&readfds);
			ret = i;
		}
		fd_max = ret;
	}

#ifdef _WIN32
	// on windows, enumerating all members of the fd_set is way faster if we access the internals
	for(i=0;i<(int)rfd.fd_count;i++)
	{
		if(session[rfd.fd_array[i]])
			session[rfd.fd_array[i]]->func_recv(rfd.fd_array[i]);
	}

	for (i = 1; i < fd_max; i++)
	{
		if(!session[i])
			continue;

		if(session[i]->wdata_size)
			session[i]->func_send(i);

		if(session[i]->eof) //func_send can't free a session, this is safe.
		{	//Finally, even if there is no data to parse, connections signalled eof should be closed, so we call parse_func [Skotlex]
			session[i]->func_parse(i); //This should close the session inmediately.
		}
	}

#else
	// otherwise assume that the fd_set is a bit-array and enumerate it in a standard way
	for (i = 1; i < fd_max; i++)
	{
		if(!session[i])
			continue;

		if(FD_ISSET(i,&rfd)){
			//ShowMessage("read:%d\n",i);
			session[i]->func_recv(i);
		}

		if(session[i]->wdata_size)
			session[i]->func_send(i);
	
		if(session[i]->eof)
		{	//Finally, even if there is no data to parse, connections signalled eof should be closed, so we call parse_func [Skotlex]
			session[i]->func_parse(i); //This should close the session inmediately.
		}
	}
#endif

	return 0;
}

int do_parsepacket(void)
{
	int i;
	struct socket_data *sd;
	for(i = 1; i < fd_max; i++)
	{
		sd = session[i];
		if(!sd)
			continue;
		if (sd->rdata_tick && DIFF_TICK(last_tick,sd->rdata_tick) > stall_time) {
			ShowInfo ("Session #%d timed out\n", i);
			sd->eof = 1;
		}
		if(sd->rdata_size == 0 && sd->eof == 0)
			continue;

		sd->func_parse(i);

		if(!session[i])
			continue;
		/* after parse, check client's RFIFO size to know if there is an invalid packet (too big and not parsed) */
		if (session[i]->rdata_size == rfifo_size && session[i]->max_rdata == rfifo_size) {
			session[i]->eof = 1;
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
static int ddos_count     = 10;
static int ddos_interval  = 3*1000;
static int ddos_autoreset = 10*60*1000;
/// Connection history, an array of linked lists.
/// The array's index for any ip is ip&0xFFFF
static ConnectHistory* connect_history[0x10000];

static int connect_check_(uint32 ip);

/// Verifies if the IP can connect. (with debug info)
/// @see connect_check_()
static int connect_check(uint32 ip)
{
	int result = connect_check_(ip);
	if( access_debug ){
		ShowMessage("connect_check: Connection from %d.%d.%d.%d %s\n",
			CONVIP(ip),result ? "allowed." : "denied!");
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

	if( strcmp(str,"all") == 0 ){
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
		ShowMessage("access_ipmask: Loaded IP:%d.%d.%d.%d mask:%d.%d.%d.%d\n",
			CONVIP(ip), CONVIP(mask));
	}
	acc->ip   = ip;
	acc->mask = mask;
	return 1;
}
//////////////////////////////
#endif
//////////////////////////////

int socket_config_read(const char *cfgName) {
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=fopen(cfgName, "r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"stall_time")==0){
			stall_time = atoi(w2);
	#ifndef MINICORE
		} else if( strcmpi(w1,"enable_ip_rules") == 0 ){
			if( strcmpi(w2,"yes") == 0 )
				ip_rules = 1;
			else if( strcmpi(w2,"no") == 0 )
				ip_rules = 0;
			else
				ip_rules = atoi(w2);
		} else if( strcmpi(w1,"order") == 0 ){
			access_order = atoi(w2);
			if( strcmpi(w2,"deny,allow") == 0 )
				access_order = ACO_DENY_ALLOW;
			else if( strcmpi(w2,"allow,deny") == 0 )
				access_order=ACO_ALLOW_DENY;
			else if( strcmpi(w2,"mutual-failure") == 0 )
				access_order=ACO_MUTUAL_FAILURE;
		} else if( strcmpi(w1,"allow") == 0 ){
			RECREATE(access_allow, AccessControl, access_allownum+1);
			if( access_ipmask(w2,&access_allow[access_allownum]) )
				++access_allownum;
			else
				ShowError("socket_config_read: Invalid ip or ip range '%s'!\n", line);
		} else if( strcmpi(w1,"deny") == 0 ){
			RECREATE(access_deny, AccessControl, access_denynum+1);
			if( access_ipmask(w2,&access_deny[access_denynum]) )
				++access_denynum;
			else
				ShowError("socket_config_read: Invalid ip or ip range '%s'!\n", line);
		} else if( strcmpi(w1,"ddos_interval") == 0){
			ddos_interval = atoi(w2);
		} else if( strcmpi(w1,"ddos_count") == 0){
			ddos_count = atoi(w2);
		} else if( strcmpi(w1,"ddos_autoreset") == 0){
			ddos_autoreset = atoi(w2);
		} else if( strcmpi(w1,"debug") == 0){
			if( strcmpi(w2,"yes") == 0 )
				access_debug = 1;
			else if( strcmpi(w2,"no") == 0 )
				access_debug = 0;
			else
				access_debug = atoi(w2);
	#endif
		} else if (strcmpi(w1, "mode_neg") == 0)
		{
			if(strcmpi(w2,"yes")==0)
				mode_neg = 1;
			else if(strcmpi(w2,"no")==0)
				mode_neg = 0;
			else mode_neg = atoi(w2);
		} else if (strcmpi(w1, "frame_size") == 0)
			frame_size = (size_t)strtoul(w2, NULL, 10);
		else if (strcmpi(w1, "import") == 0)
			socket_config_read(w2);
	}
	fclose(fp);
	return 0;
}


void socket_final (void)
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
	shutdown(fd, SHUT_RDWR); // Disallow further reads/writes
	closesocket(fd); // We don't really care if these closing functions return an error, we are just shutting down and not reusing this socket.
	if (session[fd]) delete_session(fd);
}

/// Retrieve local ips in host byte order.
/// Uses loopback is no address is found.
int socket_getips(uint32 *ips, int max)
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
				ShowError("socket_getips: Cannot resolve our own hostname to a IP address\n");
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

		fd = socket(AF_INET, SOCK_STREAM, 0);

		// The ioctl call will fail with Invalid Argument if there are more
		// interfaces than will fit in the buffer
		ic.ifc_len = sizeof(buf);
		ic.ifc_buf = buf;
		if( ioctl(fd, SIOCGIFCONF, &ic) == -1 )
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
		closesocket(fd);
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

	FD_ZERO(&readfds);

	socket_config_read(SOCKET_CONF_FILENAME);

	// initialise last send-receive tick
	last_tick = time(0);

	// session[0] is now currently used for disconnected sessions of the map server, and as such,
	// should hold enough buffer (it is a vacuum so to speak) as it is never flushed. [Skotlex]
	// ##TODO "flush" this session periodically O.O [FlavioJS]
	create_session(0, null_recv, null_send, null_parse);

#ifndef MINICORE
	// Delete old connection history every 5 minutes
	memset(connect_history, 0, sizeof(connect_history));
	add_timer_func_list(connect_check_clear, "connect_check_clear");
	add_timer_interval(gettick()+1000, connect_check_clear, 0, 0, 5*60*1000);
#endif
}


int session_isValid(int fd)
{
	return ( (fd > 0) && (fd < FD_SETSIZE) && (session[fd] != NULL) );
}

int session_isActive(int fd)
{
	return ( session_isValid(fd) && !session[fd]->eof );
}

in_addr_t resolve_hostbyname(char* hostname, unsigned char *ip, char *ip_str)
{
	struct hostent *h = gethostbyname(hostname);
	char ip_buf[16];
	unsigned char ip2[4];
	if (!h) return 0;
	if (ip == NULL) ip = ip2;
	ip[0] = (unsigned char) h->h_addr[0];
	ip[1] = (unsigned char) h->h_addr[1];
	ip[2] = (unsigned char) h->h_addr[2];
	ip[3] = (unsigned char) h->h_addr[3];
	if (ip_str == NULL) ip_str = ip_buf;
	sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return inet_addr(ip_str);
}
