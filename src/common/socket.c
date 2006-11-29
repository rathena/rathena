// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#ifdef __WIN32
#define __USE_W32_SOCKETS
#include <windows.h>
#include <winsock.h>
#include <io.h>

typedef int socklen_t;
#else
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

#ifdef _WIN32
#define SEBADF	WSAENOTSOCK
#define serrno	WSAGetLastError()
#else
#define SEBADF EBADF
#define serrno errno
#endif

#include <fcntl.h>
#include <string.h>

#include "../common/socket.h"
#include "../common/mmo.h"	// [Valaris] thanks to fov
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"

fd_set readfds;
int fd_max;
time_t last_tick;
time_t stall_time = 60;
int ip_rules = 1;

// reuse port
#ifndef SO_REUSEPORT
	#define SO_REUSEPORT 15
#endif

#ifndef TCP_FRAME_LEN
#define TCP_FRAME_LEN	1024
#endif

static int mode_neg=1;
static size_t frame_size=TCP_FRAME_LEN;

#ifndef MINICORE
enum {
	ACO_DENY_ALLOW=0,
	ACO_ALLOW_DENY,
	ACO_MUTUAL_FAILTURE,
};

static struct _access_control *access_allow;
static struct _access_control *access_deny;
static int access_order=ACO_DENY_ALLOW;
static int access_allownum=0;
static int access_denynum=0;
static int access_debug=0;
static int ddos_count     = 10;
static int ddos_interval  = 3000;
static int ddos_autoreset = 600*1000;
#endif


// values derived from freya
// a player that send more than 2k is probably a hacker without be parsed
// biggest known packet: S 0153 <len>.w <emblem data>.?B -> 24x24 256 color .bmp (0153 + len.w + 1618/1654/1756 bytes)
size_t rfifo_size = (16*1024);
size_t wfifo_size = (16*1024);

#define CONVIP(ip) ip&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,ip>>24

struct socket_data *session[FD_SETSIZE];

static int null_parse(int fd);
static int (*default_func_parse)(int) = null_parse;

static int null_console_parse(char *buf);
static int (*default_console_parse)(char*) = null_console_parse;
#ifndef MINICORE
static int connect_check(unsigned int ip);
#else
	#define connect_check(n)	1
#endif

/*======================================
 *	CORE : Set function
 *--------------------------------------
 */
void set_defaultparse(int (*defaultparse)(int))
{
	default_func_parse = defaultparse;
}

void set_nonblocking(int fd, int yes) {
	// I don't think we need this
	// TCP_NODELAY BOOL Disables the Nagle algorithm for send coalescing.
	if(mode_neg)
		setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);
	
	// FIONBIO Use with a nonzero argp parameter to enable the nonblocking mode of socket s. 
	// The argp parameter is zero if nonblocking is to be disabled. 
#ifdef __WIN32
	ioctlsocket(fd, FIONBIO, &yes);
#else
	ioctl(fd,FIONBIO,&yes); 
#endif
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
#ifdef __WIN32
{	//set SO_LINGER option (from Freya)
	//(http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/closesocket_2.asp)
	struct linger opt;
	opt.l_onoff = 1;
	opt.l_linger = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt)))
		ShowWarning("setsocketopts: Unable to set SO_LINGER mode for connection %d!\n",fd);
}
#endif
}

/*======================================
 *	CORE : Socket Sub Function
 *--------------------------------------
 */
static void set_eof(int fd)
{	//Marks a connection eof and invokes the parse_function to disconnect it right away. [Skotlex]
	if (session_isActive(fd))
		session[fd]->eof=1;
}

static int recv_to_fifo(int fd)
{
	int len;

	if( (fd<0) || (fd>=FD_SETSIZE) || (NULL==session[fd]) )
		return -1;

	if(session[fd]->eof)
		return -1;

#ifdef __WIN32
	len=recv(fd,(char *)session[fd]->rdata+session[fd]->rdata_size, RFIFOSPACE(fd), 0);
	if (len == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAECONNABORTED) {
			ShowWarning("recv_to_fifo: Software caused connection abort on session #%d\n", fd);
			FD_CLR(fd, &readfds); //Remove the socket so the select() won't hang on it.
//			exit(1);	//Windows can't really recover from this one. [Skotlex]
		}
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
//			ShowDebug("recv_to_fifo: error %d, ending connection #%d\n", WSAGetLastError(), fd);
			set_eof(fd);
		}
		return 0;
	}
#else
	len=read(fd,session[fd]->rdata+session[fd]->rdata_size, RFIFOSPACE(fd));
	if (len == -1)
	{
		if (errno == ECONNABORTED)
		{
			ShowFatalError("recv_to_fifo: Network broken (Software caused connection abort on session #%d)\n", fd);
//			exit(1); //Temporal debug, maybe this can be fixed.
		}
		if (errno != EAGAIN) {	//Connection error.
//			perror("closing session: recv_to_fifo");
			set_eof(fd);
		}
		return 0;
	}
#endif	
	if (len <= 0) {	//Normal connection end.
		set_eof(fd);
		return 0;
	}

	session[fd]->rdata_size+=len;
	session[fd]->rdata_tick = last_tick;
	return 0;
}

static int send_from_fifo(int fd)
{
	int len;
	if( !session_isValid(fd) )
		return -1;

//	if (s->eof) // if we close connection, we can not send last information (you're been disconnected, etc...) [Yor]
//		return -1;
/*
	// clear write buffer if not connected <- I really like more the idea of sending the last information. [Skotlex]
	if( session[fd]->eof )
	{
		session[fd]->wdata_size = 0;
		return -1;
	}
*/
	
	if (session[fd]->wdata_size == 0)
		return 0;

#ifdef __WIN32
	len=send(fd, (const char *)session[fd]->wdata,session[fd]->wdata_size, 0);
	if (len == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAECONNABORTED) {
			ShowWarning("send_from_fifo: Software caused connection abort on session #%d\n", fd);
			session[fd]->wdata_size = 0; //Clear the send queue as we can't send anymore. [Skotlex]
			set_eof(fd);
			FD_CLR(fd, &readfds); //Remove the socket so the select() won't hang on it.
		}
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
//			ShowDebug("send_from_fifo: error %d, ending connection #%d\n", WSAGetLastError(), fd);
			session[fd]->wdata_size = 0; //Clear the send queue as we can't send anymore. [Skotlex]
			set_eof(fd);
		}
		return 0;
	}
#else
	len=send(fd, session[fd]->wdata, session[fd]->wdata_size, MSG_NOSIGNAL);
	if (len == -1)
	{
		if (errno == ECONNABORTED)
		{
			ShowWarning("send_from_fifo: Network broken (Software caused connection abort on session #%d)\n", fd);
			session[fd]->wdata_size = 0; //Clear the send queue as we can't send anymore. [Skotlex]
			set_eof(fd);
		}
		if (errno != EAGAIN) {
//			perror("closing session: send_from_fifo");
			session[fd]->wdata_size = 0; //Clear the send queue as we can't send anymore. [Skotlex]
			set_eof(fd);
		}
		return 0;
	}
#endif

	//{ int i; ShowMessage("send %d : ",fd);  for(i=0;i<len;i++){ ShowMessage("%02x ",session[fd]->wdata[i]); } ShowMessage("\n");}
	if(len>0){
		if((size_t)len<session[fd]->wdata_size){
			memmove(session[fd]->wdata,session[fd]->wdata+len,session[fd]->wdata_size-len);
			session[fd]->wdata_size-=len;
		} else {
			session[fd]->wdata_size=0;
		}
	}
	return 0;
}

void flush_fifo(int fd)
{
	if(session[fd] != NULL && session[fd]->func_send == send_from_fifo)
	{
		set_nonblocking(fd, 1);
		send_from_fifo(fd);
		set_nonblocking(fd, 0);
	}
}

void flush_fifos(void)
{
	int i;
	for(i=1;i<fd_max;i++)
		if(session[i] != NULL &&
		   session[i]->func_send == send_from_fifo)
			send_from_fifo(i);
}

static int null_parse(int fd)
{
	ShowMessage("null_parse : %d\n",fd);
	session[fd]->rdata_pos = session[fd]->rdata_size; //RFIFOSKIP(fd, RFIFOREST(fd)); simplify calculation
	return 0;
}

/*======================================
 *	CORE : Socket Function
 *--------------------------------------
 */

static int connect_client(int listen_fd)
{
	int fd;
	struct sockaddr_in client_address;
#ifdef __WIN32
	int len;
#else
	socklen_t len;
#endif
	//ShowMessage("connect_client : %d\n",listen_fd);

	len=sizeof(client_address);

	fd = accept(listen_fd,(struct sockaddr*)&client_address,&len);
#ifdef __WIN32
	if ( fd == INVALID_SOCKET ) {
		ShowError("accept failed (code %i)!\n", WSAGetLastError());
		return -1;
	}
#else                                                        
	if(fd==-1) {
		perror("accept");
		return -1;
	}
#endif
	
	if(fd_max<=fd) fd_max=fd+1;

	setsocketopts(fd);

#ifdef __WIN32
	{
		unsigned long val = 1;
		if (ioctlsocket(fd, FIONBIO, &val) != 0)
			ShowError("Couldn't set the socket to non-blocking mode (code %d)!\n", WSAGetLastError());
	}
#else
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		perror("connect_client (set nonblock)");
#endif

	if (ip_rules && !connect_check(*(unsigned int*)(&client_address.sin_addr))) {
		do_close(fd);
		return -1;
	} else
		FD_SET(fd,&readfds);

	CREATE(session[fd], struct socket_data, 1);
	CREATE(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE(session[fd]->wdata, unsigned char, wfifo_size);

	session[fd]->max_rdata   = rfifo_size;
	session[fd]->max_wdata   = wfifo_size;
	session[fd]->func_recv   = recv_to_fifo;
	session[fd]->func_send   = send_from_fifo;
	if(!session[listen_fd]->func_parse)
		session[fd]->func_parse = default_func_parse;
	else
		session[fd]->func_parse = session[listen_fd]->func_parse;
	session[fd]->client_addr = client_address;
	session[fd]->rdata_tick  = last_tick;
	session[fd]->type        = SESSION_UNKNOWN;	// undefined type

  //ShowMessage("new_session : %d %d\n",fd,session[fd]->eof);
	return fd;
}

int make_listen_bind(long ip,int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = (int)socket( AF_INET, SOCK_STREAM, 0 );

#ifdef __WIN32
	if (fd == INVALID_SOCKET) {
		ShowError("socket() creation failed (code %d)!\n", fd, WSAGetLastError());
		exit(1);
	}
#else
	if (fd == -1) {
		perror("make_listen_port:socket()");
		exit(1);
	}
#endif

#ifdef __WIN32
	{
	  	unsigned long val = 1;
		if (ioctlsocket(fd, FIONBIO, &val) != 0)
			ShowError("Couldn't set the socket to non-blocking mode (code %d)!\n", WSAGetLastError());
	}
#else
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		perror("make_listen_bind (set nonblock)");
#endif

	setsocketopts(fd);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port        = htons((unsigned short)port);

	result = bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
#ifdef __WIN32
	if( result == SOCKET_ERROR ) {
		ShowError("bind failed (socket %d, code %d)!\n", fd, WSAGetLastError());
		exit(1);
	}
#else
	if( result == -1 ) {
		perror("bind");
		exit(1);
	}
#endif
	result = listen( fd, 5 );
#ifdef __WIN32
	if( result == SOCKET_ERROR ) {
		ShowError("listen failed (socket %d, code %d)!\n", fd, WSAGetLastError());
		exit(1);
	}
#else
	if( result != 0) { /* error */
		perror("listen");
		exit(1);
	}
#endif
	if ( fd < 0 || fd > FD_SETSIZE ) 
	{ //Crazy error that can happen in Windows? (info from Freya)
		ShowFatalError("listen() returned invalid fd %d!\n",fd);
		exit(1);
	}

	if(fd_max<=fd) fd_max=fd+1;
	FD_SET(fd, &readfds );

	CREATE(session[fd], struct socket_data, 1);

	malloc_set(session[fd],0,sizeof(*session[fd]));
	session[fd]->func_recv = connect_client;

	return fd;
}

int make_listen_port(int port)
{
	return make_listen_bind(INADDR_ANY,port);
}

// Console Reciever [Wizputer]
int console_recieve(int i) {
	int n;
	char *buf;

	CREATE(buf, char, 64);
	malloc_tsetdword(buf,0,sizeof(64));

	n = read(0, buf , 64);
	if ( n < 0 )
		ShowError("Console input read error\n");
	else
	{
		ShowNotice ("Sorry, the console is currently non-functional.\n");
//		session[0]->func_console(buf);
	}

	aFree(buf);
	return 0;
}

void set_defaultconsoleparse(int (*defaultparse)(char*))
{
	default_console_parse = defaultparse;
}

static int null_console_parse(char *buf)
{
	ShowMessage("null_console_parse : %s\n",buf);
	return 0;
}

// function parse table
// To-do: -- use dynamic arrays
//        -- add a register_parse_func();
struct func_parse_table func_parse_table[SESSION_MAX];

int default_func_check (struct socket_data *sd) { return 1; }

void func_parse_check (struct socket_data *sd)
{
	int i;
	for (i = SESSION_HTTP; i < SESSION_MAX; i++) {
		if (func_parse_table[i].func &&
			func_parse_table[i].check &&
			func_parse_table[i].check(sd) != 0)
		{
			sd->type = i;
			sd->func_parse = func_parse_table[i].func;
			return;
		}
	}

	// undefined -- treat as raw socket (using default parse)
	sd->type = SESSION_RAW;
}

// Console Input [Wizputer]
int start_console(void) {

	//Until a better plan is came up with... can't be using session[0] anymore! [Skotlex]
	ShowNotice("The console is currently nonfunctional.\n");
	return 0;
	
	FD_SET(0,&readfds);

	if (!session[0]) {	// dummy socket already uses fd 0
		CREATE(session[0], struct socket_data, 1);
	}
	malloc_set(session[0],0,sizeof(*session[0]));

	session[0]->func_recv = console_recieve;
	session[0]->func_console = default_console_parse;

	return 0;
}

int make_connection(long ip,int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = (int)socket( AF_INET, SOCK_STREAM, 0 );

#ifdef __WIN32
	if (fd == INVALID_SOCKET) {
		ShowError("socket() creation failed (code %d)!\n", fd, WSAGetLastError());
		return -1;
	}
#else
	if (fd == -1) {
		perror("make_connection:socket()");
		return -1;
	}
#endif

	setsocketopts(fd);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port = htons((unsigned short)port);

	ShowStatus("Connecting to %d.%d.%d.%d:%i\n",
		(ip)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);

	result = connect(fd, (struct sockaddr *)(&server_address), sizeof(struct sockaddr_in));
#ifdef __WIN32
	if( result == SOCKET_ERROR ) {
		ShowError("connect failed (socket %d, code %d)!\n", fd, WSAGetLastError());
		do_close(fd);
		return -1;
	}
#else
	if (result < 0) { //This is only used when the map/char server try to connect to each other, so it can be handled. [Skotlex]
		perror("make_connection");
		do_close(fd);
		return -1;
	}
#endif
//Now the socket can be made non-blocking. [Skotlex]
#ifdef __WIN32
	{
		unsigned long val = 1;
		if (ioctlsocket(fd, FIONBIO, &val) != 0)
			ShowError("Couldn't set the socket to non-blocking mode (code %d)!\n", WSAGetLastError());
	}
#else
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		perror("make_connection (set nonblock)");
#endif

	if (fd_max <= fd)
		fd_max = fd + 1;
	FD_SET(fd,&readfds);

	CREATE(session[fd], struct socket_data, 1);
	CREATE(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE(session[fd]->wdata, unsigned char, wfifo_size);

	session[fd]->max_rdata  = rfifo_size;
	session[fd]->max_wdata  = wfifo_size;
	session[fd]->func_recv  = recv_to_fifo;
	session[fd]->func_send  = send_from_fifo;
	session[fd]->func_parse = default_func_parse;
	session[fd]->rdata_tick = last_tick;

	return fd;
}

void free_session_mem(int fd)
{
	if (session[fd]){
		if (session[fd]->rdata)
			aFree(session[fd]->rdata);
		if (session[fd]->wdata)
			aFree(session[fd]->wdata);
		if (session[fd]->session_data)
			aFree(session[fd]->session_data);
		aFree(session[fd]);
		session[fd] = NULL;
	}
}

int delete_session(int fd)
{
	if (fd <= 0 || fd >= FD_SETSIZE)
		return -1;
	FD_CLR(fd, &readfds);
	free_session_mem(fd);
	//ShowMessage("delete_session:%d\n",fd);
	return 0;
}

int realloc_fifo(int fd,unsigned int rfifo_size,unsigned int wfifo_size)
{
	if( !session_isValid(fd) )
		return 0;

	if( session[fd]->max_rdata != rfifo_size && session[fd]->rdata_size < rfifo_size){
		RECREATE(session[fd]->rdata, unsigned char, rfifo_size);
		session[fd]->max_rdata  = rfifo_size;
	}

	if( session[fd]->max_wdata != wfifo_size && session[fd]->wdata_size < wfifo_size){
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
	} else if( session[fd]->max_wdata>wfifo_size &&
	  	(session[fd]->wdata_size+addition)*4 < session[fd]->max_wdata )
	{	// shrink rule, shrink by 2 when only a quater of the fifo is used, don't shrink below 4*addition
		newsize = session[fd]->max_wdata/2;
	}
	else // no change
		return 0;

	RECREATE(session[fd]->wdata, unsigned char, newsize);
	session[fd]->max_wdata  = newsize;

	return 0;
}

int WFIFOSET(int fd,int len)
{
	size_t newreserve;
	struct socket_data *s = session[fd];

	if( !session_isValid(fd) || s->wdata == NULL )
		return 0;

	// we have written len bytes to the buffer already before calling WFIFOSET
	if(s->wdata_size+len > s->max_wdata)
	{	// actually there was a buffer overflow already
		unsigned char *sin_addr = (unsigned char *)&s->client_addr.sin_addr;
		ShowFatalError("socket: Buffer Overflow. Connection %d (%d.%d.%d.%d) has written %d byteson a %d/%d bytes buffer.\n", fd,
			sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], len, s->wdata_size, s->max_wdata);
		ShowDebug("Likely command that caused it: 0x%x\n",
			(*(unsigned short*)(s->wdata+s->wdata_size)));
		// no other chance, make a better fifo model
		exit(1);
	}

	s->wdata_size += len;
	// always keep a wfifo_size reserve in the buffer
	// For inter-server connections, let the reserve be 1/8th of the link size.
	newreserve = s->wdata_size + (s->max_wdata>=FIFOSIZE_SERVERLINK?FIFOSIZE_SERVERLINK<<3:wfifo_size);

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
	fd_set rfd,efd; //Added the Error Set so that such sockets can be made eof. They are the same as the rfd for now. [Skotlex]
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

		if(session[i]->wdata_size && session[i]->func_send)
			session[i]->func_send(i);
	}

	timeout.tv_sec  = next/1000;
	timeout.tv_usec = next%1000*1000;

	for(memcpy(&rfd, &readfds, sizeof(rfd)),
		memcpy(&efd, &readfds, sizeof(efd));
		(ret = select(fd_max, &rfd, NULL, &efd, &timeout))<0;
		memcpy(&rfd, &readfds, sizeof(rfd)),
		memcpy(&efd, &readfds, sizeof(efd)))
	{
		if(serrno != SEBADF)
			return 0;

		//Well then the error is due to a bad socket. Lets find and remove it
		//and try again
		for(i = 1; i < fd_max; i++)
		{
			if(!session[i])
				continue;

			//check the validity of the socket. Does what the last thing did
			//just alot faster [Meruru]
			size = sizeof(struct sockaddr);
			if(getsockname(i,(struct sockaddr*)&addr_check,&size)<0)
				if(serrno == SEBADF) //See the #defines at the top
				{
					free_session_mem(i); //free the bad session
					continue;
				}

			FD_SET(i,&readfds);
			ret = i;
		}

		fd_max = ret;
	}

	//ok under windows to use FD_ISSET is FUCKING stupid
	//because windows uses an array so lets do them part by part [Meruru]
#ifdef _WIN32
		//Do the socket sets. Unlike linux which uses a bit mask windows uses
		//a array. So calls to FS_ISSET are SLOW AS SHIT. So we have to do
		//a special case for them which actually turns out ok [Meruru]
	for(i=0;i<(int)rfd.fd_count;i++)
	{
		if(session[rfd.fd_array[i]] &&
			session[rfd.fd_array[i]]->func_recv)
			session[rfd.fd_array[i]]->func_recv(rfd.fd_array[i]);
	}
	for(i=0;i<(int)efd.fd_count;i++)
		set_eof(efd.fd_array[i]);

	for (i = 1; i < fd_max; i++)
	{
		if(!session[i])
			continue;

		//POSTSEND: Does write EVER BLOCK? NO!! not unless WE ARE CURRENTLY SENDING SOMETHING
		//Or just have opened a connection and dont know if its ready
		//And since eA isn't multi threaded and all the sockets are non blocking THIS ISNT A PROBLEM! [Meruru]

		if(session[i]->wdata_size && session[i]->func_send)
			session[i]->func_send(i);

		if(session[i]->eof) //The session check is for when the connection ended in func_parse
		{	//Finally, even if there is no data to parse, connections signalled eof should be closed, so we call parse_func [Skotlex]
			if (session[i]->func_parse)
				session[i]->func_parse(i); //This should close the session inmediately.
		}
	}

#else //where under linux its just a bit check so its smart [Meruru]

	for (i = 1; i < fd_max; i++){
		if(!session[i])
			continue;

		if(FD_ISSET(i,&efd)){
			//ShowMessage("error:%d\n",i);
			ShowDebug("do_sendrecv: Connection error on Session %d.\n", i);
			set_eof(i);
			continue;
		}


		if(FD_ISSET(i,&rfd)){
			//ShowMessage("read:%d\n",i);
			if(session[i]->func_recv)
				session[i]->func_recv(i);
		}

		//Does write EVER BLOCK. NO not unless WE ARE CURRENTALLY SENDING SOMETHING
		//And sence eA isnt multi threaded THIS ISNT A PROBLEM!
		if(session[i]->wdata_size && session[i]->func_send)
			session[i]->func_send(i);
	
		if(session[i] && session[i]->eof) //The session check is for when the connection ended in func_parse
		{	//Finally, even if there is no data to parse, connections signalled eof should be closed, so we call parse_func [Skotlex]
			if (session[i]->func_parse)
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
	for(i = 1; i < fd_max; i++){
		sd = session[i];
		if(!sd)
			continue;
		if (sd->rdata_tick && DIFF_TICK(last_tick,sd->rdata_tick) > stall_time) {
			ShowInfo ("Session #%d timed out\n", i);
			sd->eof = 1;
		}
		if(sd->rdata_size == 0 && sd->eof == 0)
			continue;
		if(sd->func_parse){
			if(sd->type == SESSION_UNKNOWN)
				func_parse_check(sd);
			if(sd->type != SESSION_UNKNOWN)
				sd->func_parse(i);
			if(!session[i])
				continue;
			/* after parse, check client's RFIFO size to know if there is an invalid packet (too big and not parsed) */
			if (session[i]->rdata_size == rfifo_size && session[i]->max_rdata == rfifo_size) {
				session[i]->eof = 1;
				continue;
			}
		}
		RFIFOFLUSH(i);
	}
	return 0;
}

/* DDoS 攻撃対策 */
#ifndef MINICORE
struct _access_control {
	unsigned int ip;
	unsigned int mask;
};

struct _connect_history {
	struct _connect_history *next;
	struct _connect_history *prev;
	int    status;
	int    count;
	unsigned int ip;
	unsigned int tick;
};
static struct _connect_history *connect_history[0x10000];
static int connect_check_(unsigned int ip);

// 接続できるかどうかの確認
//   false : 接続OK
//   true  : 接続NG
static int connect_check(unsigned int ip) {
	int result = connect_check_(ip);
	if(access_debug) {
		ShowMessage("connect_check: Connection from %d.%d.%d.%d %s\n",
			CONVIP(ip),result ? "allowed." : "denied!");
	}
	return result;
}

static int connect_check_(unsigned int ip) {
	struct _connect_history *hist     = connect_history[ip & 0xFFFF];
	struct _connect_history *hist_new;
	int    i,is_allowip = 0,is_denyip = 0,connect_ok = 0;

	// allow , deny リストに入っているか確認
	for(i = 0;i < access_allownum; i++) {
		if((ip & access_allow[i].mask) == (access_allow[i].ip & access_allow[i].mask)) {
			if(access_debug) {
				ShowMessage("connect_check: Found match from allow list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
					CONVIP(ip),
					CONVIP(access_allow[i].ip),
					CONVIP(access_allow[i].mask));
			}
			is_allowip = 1;
			break;
		}
	}
	for(i = 0;i < access_denynum; i++) {
		if((ip & access_deny[i].mask) == (access_deny[i].ip & access_deny[i].mask)) {
			if(access_debug) {
				ShowMessage("connect_check: Found match from deny list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
					CONVIP(ip),
					CONVIP(access_deny[i].ip),
					CONVIP(access_deny[i].mask));
			}
			is_denyip = 1;
			break;
		}
	}
	// コネクト出来るかどうか確認
	// connect_ok
	//   0 : 無条件に拒否
	//   1 : 田代砲チェックの結果次第
	//   2 : 無条件に許可
	switch(access_order) {
	case ACO_DENY_ALLOW:
	default:
		if(is_allowip) {
			connect_ok = 2;
		} else if(is_denyip) {
			connect_ok = 0;
		} else {
			connect_ok = 1;
		}
		break;
	case ACO_ALLOW_DENY:
		if(is_denyip) {
			connect_ok = 0;
		} else if(is_allowip) {
			connect_ok = 2;
		} else {
			connect_ok = 1;
		}
		break;
	case ACO_MUTUAL_FAILTURE:
		if(is_allowip) {
			connect_ok = 2;
		} else {
			connect_ok = 0;
		}
		break;
	}

	// 接続履歴を調べる
	while(hist) {
		if(ip == hist->ip) {
			// 同じIP発見
			if(hist->status) {
				// ban フラグが立ってる
				return (connect_ok == 2 ? 1 : 0);
			} else if(DIFF_TICK(gettick(),hist->tick) < ddos_interval) {
				// ddos_interval秒以内にリクエスト有り
				hist->tick = gettick();
				if(hist->count++ >= ddos_count) {
					// ddos 攻撃を検出
					hist->status = 1;
					ShowWarning("connect_check: DDOS Attack detected from %d.%d.%d.%d!\n",
						CONVIP(ip));
					return (connect_ok == 2 ? 1 : 0);
				} else {
					return connect_ok;
				}
			} else {
				// ddos_interval秒以内にリクエスト無いのでタイマークリア
				hist->tick  = gettick();
				hist->count = 0;
				return connect_ok;
			}
		}
		hist = hist->next;
	}
	// IPリストに無いので新規作成
	hist_new = (struct _connect_history *) aCalloc(1,sizeof(struct _connect_history));
	hist_new->ip   = ip;
	hist_new->tick = gettick();
	if(connect_history[ip & 0xFFFF] != NULL) {
		hist = connect_history[ip & 0xFFFF];
		hist->prev = hist_new;
		hist_new->next = hist;
	}
	connect_history[ip & 0xFFFF] = hist_new;
	return connect_ok;
}

static int connect_check_clear(int tid,unsigned int tick,int id,int data) {
	int i;
	int clear = 0;
	int list  = 0;
	struct _connect_history *hist , *hist2;
	for(i = 0;i < 0x10000 ; i++) {
		hist = connect_history[i];
		while(hist) {
			if ((DIFF_TICK(tick,hist->tick) > ddos_interval * 3 && !hist->status) ||
				(DIFF_TICK(tick,hist->tick) > ddos_autoreset && hist->status)) {
				// clear data
				hist2 = hist->next;
				if(hist->prev) {
					hist->prev->next = hist->next;
				} else {
					connect_history[i] = hist->next;
				}
				if(hist->next) {
					hist->next->prev = hist->prev;
				}
				aFree(hist);
				hist = hist2;
				clear++;
			} else {
				hist = hist->next;
			}
			list++;
		}
	}
	if(access_debug) {
		ShowMessage("connect_check_clear: Cleared %d of %d from IP list.\n", clear, list);
	}
	return list;
}

// IPマスクチェック
int access_ipmask(const char *str,struct _access_control* acc)
{
	unsigned int mask=0,i=0,m,ip, a0,a1,a2,a3;
	if( !strcmp(str,"all") ) {
		ip   = 0;
		mask = 0;
	} else {
		if( sscanf(str,"%d.%d.%d.%d%n",&a0,&a1,&a2,&a3,&i)!=4 || i==0) {
			ShowError("access_ipmask: Unknown format %s!\n",str);
			return 0;
		}
		ip = (a3 << 24) | (a2 << 16) | (a1 << 8) | a0;

		if(sscanf(str+i,"/%d.%d.%d.%d",&a0,&a1,&a2,&a3)==4 ){
			mask = (a3 << 24) | (a2 << 16) | (a1 << 8) | a0;
		} else if(sscanf(str+i,"/%d",&m) == 1) {
			for(i=0;i<m;i++) {
				mask = (mask >> 1) | 0x80000000;
			}
			mask = ntohl(mask);
		} else {
			mask = 0xFFFFFFFF;
		}
	}
	if(access_debug) {
		ShowMessage("access_ipmask: Loaded IP:%d.%d.%d.%d mask:%d.%d.%d.%d\n",
			CONVIP(ip), CONVIP(mask));
	}
	acc->ip   = ip;
	acc->mask = mask;
	return 1;
}
#endif

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
		} else if(strcmpi(w1,"enable_ip_rules")==0){
			if(strcmpi(w2,"yes")==0)
				ip_rules = 1;
			else if(strcmpi(w2,"no")==0)
				ip_rules = 0;
			else ip_rules = atoi(w2);
		} else if(strcmpi(w1,"order")==0){
			access_order=atoi(w2);
			if(strcmpi(w2,"deny,allow")==0) access_order=ACO_DENY_ALLOW;
			if(strcmpi(w2,"allow,deny")==0) access_order=ACO_ALLOW_DENY;
			if(strcmpi(w2,"mutual-failure")==0) access_order=ACO_MUTUAL_FAILTURE;
		} else if(strcmpi(w1,"allow")==0){
			access_allow = (struct _access_control *) aRealloc(access_allow,(access_allownum+1)*sizeof(struct _access_control));
			if(access_ipmask(w2,&access_allow[access_allownum])) {
				access_allownum++;
			}
		} else if(strcmpi(w1,"deny")==0){
			access_deny = (struct _access_control *) aRealloc(access_deny,(access_denynum+1)*sizeof(struct _access_control));
			if(access_ipmask(w2,&access_deny[access_denynum])) {
				access_denynum++;
			}
		} else if(!strcmpi(w1,"ddos_interval")){
			ddos_interval = atoi(w2);
		} else if(!strcmpi(w1,"ddos_count")){
			ddos_count = atoi(w2);
		} else if(!strcmpi(w1,"ddos_autoreset")){
			ddos_autoreset = atoi(w2);
		} else if(!strcmpi(w1,"debug")){
			if(strcmpi(w2,"yes")==0)
				access_debug = 1;
			else if(strcmpi(w2,"no")==0)
				access_debug = 0;
			else access_debug = atoi(w2);
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

int RFIFOSKIP(int fd,int len)
{
    struct socket_data *s;

	if ( !session_isActive(fd) ) //Nullpo error here[Kevin]
		return 0;

	s = session[fd];

	if ( s->rdata_size - s->rdata_pos - len < 0 ) {
		//fprintf(stderr,"too many skip\n");
		//exit(1);
		//better than a COMPLETE program abort // TEST! :)
		ShowError("too many skip (%d) now skipped: %d (FD: %d)\n", len, RFIFOREST(fd), fd);
		len = RFIFOREST(fd);
	}
	s->rdata_pos = s->rdata_pos+len;
	return 0;
}


unsigned int addr_[16];   // ip addresses of local host (host byte order)
unsigned int naddr_ = 0;   // # of ip addresses

void socket_final (void)
{
	int i;
#ifndef MINICORE
	struct _connect_history *hist , *hist2;
	for(i = 0; i < 0x10000; i++) {
		hist = connect_history[i];
		while(hist) {
			hist2 = hist->next;
			aFree(hist);
			hist = hist2;
		}
	}
	if (access_allow)
		aFree(access_allow);
	if (access_deny)
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

//Closes a socket.
//Needed to simplify shutdown code as well as manage the subtle differences in socket management from Windows and *nix.
void do_close(int fd)
{
//We don't really care if these closing functions return an error, we are just shutting down and not reusing this socket.
#ifdef __WIN32
//	shutdown(fd, SD_BOTH); //FIXME: Shutdown requires winsock2.h! What would be the proper shutting down method for winsock1?
	if (session[fd] && session[fd]->func_send == send_from_fifo)
		session[fd]->func_send(fd); //Flush the data as it is gonna be closed down, but it may not succeed as it is a nonblocking socket! [Skotlex]
	closesocket(fd);
#else
	if (close(fd))
		perror("do_close: close");
#endif
	if (session[fd])
		delete_session(fd);
}

void socket_init (void)
{
	char *SOCKET_CONF_FILENAME = "conf/packet_athena.conf";
#ifdef __WIN32
	char** a;
	unsigned int i;
	char fullhost[255];
	struct hostent* hent;

		/* Start up the windows networking */
	WORD version_wanted = MAKEWORD(1, 1); //Demand at least WinSocket version 1.1 (from Freya)
	WSADATA wsaData;

	if ( WSAStartup(version_wanted, &wsaData) != 0 ) {
		ShowFatalError("SYSERR: WinSock not available!\n");
		exit(1);
	}

	if(gethostname(fullhost, sizeof(fullhost)) == SOCKET_ERROR) {
		ShowError("Ugg.. no hostname defined!\n");
		return;
	}

	// XXX This should look up the local IP addresses in the registry
	// instead of calling gethostbyname. However, the way IP addresses
	// are stored in the registry is annoyingly complex, so I'll leave
	// this as T.B.D.
	hent = gethostbyname(fullhost);
	if (hent == NULL) {
		ShowError("Cannot resolve our own hostname to a IP address");
		return;
	}

	a = hent->h_addr_list;
	for(i = 0; a[i] != 0 && i < 16; ++i) {
		unsigned long addr1 = ntohl(*(unsigned long*) a[i]);
		addr_[i] = addr1;
	}
	naddr_ = i;
#else
	int pos;
	int fdes = socket(AF_INET, SOCK_STREAM, 0);
	char buf[16 * sizeof(struct ifreq)];
	struct ifconf ic;

	// The ioctl call will fail with Invalid Argument if there are more
	// interfaces than will fit in the buffer
	ic.ifc_len = sizeof(buf);
	ic.ifc_buf = buf;
	if(ioctl(fdes, SIOCGIFCONF, &ic) == -1) {
		ShowError("SIOCGIFCONF failed!\n");
		return;
	}

	for(pos = 0; pos < ic.ifc_len;)
	{
		struct ifreq * ir = (struct ifreq *) (ic.ifc_buf + pos);

		struct sockaddr_in * a = (struct sockaddr_in *) &(ir->ifr_addr);

		if(a->sin_family == AF_INET) {
			u_long ad = ntohl(a->sin_addr.s_addr);
			if(ad != INADDR_LOOPBACK && ad != INADDR_ANY) {
				addr_[naddr_ ++] = ad;
				if(naddr_ == 16)
					break;
			}
		}

	#if defined(_AIX) || defined(__APPLE__)
		pos += ir->ifr_addr.sa_len;  // For when we port athena to run on Mac's :)
		pos += sizeof(ir->ifr_name);
	#else
		pos += sizeof(struct ifreq);
	#endif
	}
#endif

	FD_ZERO(&readfds);

	socket_config_read(SOCKET_CONF_FILENAME);

	// initialise last send-receive tick
	last_tick = time(0);

	// session[0] Was for the console (whatever that was?), but is now currently used for disconnected sessions of the map
	// server, and as such, should hold enough buffer (it is a vacuum so to speak) as it is never flushed. [Skotlex]
	CREATE(session[0], struct socket_data, 1);
	CREATE(session[0]->rdata, unsigned char, 2*rfifo_size);
	CREATE(session[0]->wdata, unsigned char, 2*wfifo_size);
	session[0]->max_rdata   = 2*rfifo_size;
	session[0]->max_wdata   = 2*wfifo_size;

	malloc_set(func_parse_table, 0, sizeof(func_parse_table));
	func_parse_table[SESSION_RAW].check = default_func_check;
	func_parse_table[SESSION_RAW].func = default_func_parse;

#ifndef MINICORE
	// とりあえず５分ごとに不要なデータを削除する
	add_timer_func_list(connect_check_clear, "connect_check_clear");
	add_timer_interval(gettick()+1000,connect_check_clear,0,0,300*1000);
#endif
}


bool session_isValid(int fd)
{	//End of Exam has pointed out that fd==0 is actually an unconnected session! [Skotlex]
	//But this is not so true, it is used... for... something. The console uses it, would this not cause problems? [Skotlex]
	return ( (fd>0) && (fd<FD_SETSIZE) && (NULL!=session[fd]) );
}

bool session_isActive(int fd)
{
	return ( session_isValid(fd) && !session[fd]->eof );
}

in_addr_t resolve_hostbyname(char* hostname, unsigned char *ip, char *ip_str) {
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
