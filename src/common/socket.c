// original : core.c 2003/02/26 18:03:12 Rev 1.7

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#include <fcntl.h>
#include <string.h>

#include "mmo.h"	// [Valaris] thanks to fov
#include "socket.h"
#include "utils.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

fd_set readfds;
int fd_max;

int rfifo_size = 65536;
int wfifo_size = 65536;

#ifndef TCP_FRAME_LEN
#define TCP_FRAME_LEN 1053
#endif

struct socket_data *session[FD_SETSIZE];

static int null_parse(int fd);
static int (*default_func_parse)(int) = null_parse;

static int null_console_parse(char *buf);
static int (*default_console_parse)(char*) = null_console_parse;

/*======================================
 *	CORE : Set function
 *--------------------------------------
 */
void set_defaultparse(int (*defaultparse)(int))
{
	default_func_parse = defaultparse;
}

#ifdef NSOCKET
static void setsocketopts(int fd)
{
	int yes = 1; // reuse fix

	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof yes);
#endif
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);

	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &wfifo_size , sizeof(rfifo_size ));
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &rfifo_size , sizeof(rfifo_size ));
}

#endif /* NSOCKET */
/*======================================
 *	CORE : Socket Sub Function
 *--------------------------------------
 */

static int recv_to_fifo(int fd)
{
	int len;

	//printf("recv_to_fifo : %d %d\n",fd,session[fd]->eof);
	if(session[fd]->eof)
		return -1;


#ifdef _WIN32
	len=recv(fd,session[fd]->rdata+session[fd]->rdata_size, RFIFOSPACE(fd), 0);
#else
	len=read(fd,session[fd]->rdata+session[fd]->rdata_size,RFIFOSPACE(fd));
#endif

//	printf (":::RECEIVE:::\n");
//	dump(session[fd]->rdata, len); printf ("\n");

	//{ int i; printf("recv %d : ",fd); for(i=0;i<len;i++){ printf("%02x ",RFIFOB(fd,session[fd]->rdata_size+i)); } printf("\n");}
	if(len>0){
		session[fd]->rdata_size+=len;
	} else if(len<=0){
		// value of connection is not necessary the same
//		if (fd == 4)			// Removed [Yor]
//			printf("Char-Server Has Disconnected.\n");
//		else if (fd == 5)		// Removed [Yor]
//			printf("Attempt To Log In Successful.\n");
//		else if (fd == 7)		// Removed [Yor]
//			printf("Char-Server Has Disconnected.\n");
//		else if (fd == 8)		// Removed [Valaris]
//			printf("%s has logged off your server.\n",RFIFOP(fd,6));	// Removed [Valaris]

//		else if (fd != 8)	// [Valaris]
		printf("set eof : connection #%d\n", fd);
		session[fd]->eof=1;
	}
	return 0;
}

static int send_from_fifo(int fd)
{
	int len;

	//printf("send_from_fifo : %d\n",fd);
	if(session[fd]->eof)
		return -1;

#ifdef _WIN32
	len=send(fd, session[fd]->wdata,session[fd]->wdata_size, 0);
#else
	len=write(fd,session[fd]->wdata,session[fd]->wdata_size);
#endif

//	printf (":::SEND:::\n");
//	dump(session[fd]->wdata, len); printf ("\n");

	//{ int i; printf("send %d : ",fd);  for(i=0;i<len;i++){ printf("%02x ",session[fd]->wdata[i]); } printf("\n");}
	if(len>0){
		if(len<session[fd]->wdata_size){
			memmove(session[fd]->wdata,session[fd]->wdata+len,session[fd]->wdata_size-len);
			session[fd]->wdata_size-=len;
		} else {
			session[fd]->wdata_size=0;
		}
	} else {
		printf("set eof :%d\n",fd);
		session[fd]->eof=1;
	}
	return 0;
}

static int null_parse(int fd)
{
	printf("null_parse : %d\n",fd);
	RFIFOSKIP(fd,RFIFOREST(fd));
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
	int len;
	int result;
#ifndef NSOCKET
	int yes = 1; // reuse fix
#endif /* not NSOCKET */

	//printf("connect_client : %d\n",listen_fd);

	len=sizeof(client_address);

	fd=accept(listen_fd,(struct sockaddr*)&client_address,&len);
	if(fd_max<=fd) fd_max=fd+1;

#ifndef NSOCKET
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof yes);
#endif
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);
#else /* NSOCKET */
	setsocketopts(fd);
#endif /* NSOCKET */

	if(fd==-1)
		perror("accept");
	else 
		FD_SET(fd,&readfds);

#ifdef _WIN32
        {
	  	unsigned long val = 1;
  		ioctlsocket(fd, FIONBIO, &val);
        }
#else
	result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	CREATE(session[fd], struct socket_data, 1);
	CREATE(session[fd]->rdata, char, rfifo_size);
	CREATE(session[fd]->wdata, char, wfifo_size);

	session[fd]->max_rdata   = rfifo_size;
	session[fd]->max_wdata   = wfifo_size;
	session[fd]->func_recv   = recv_to_fifo;
	session[fd]->func_send   = send_from_fifo;
	session[fd]->func_parse  = default_func_parse;
	session[fd]->client_addr = client_address;

  //printf("new_session : %d %d\n",fd,session[fd]->eof);
	return fd;
}

int make_listen_port(int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;
#ifndef NSOCKET
	int yes = 1; // reuse fix
#endif /* not NSOCKET */

	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(fd_max<=fd) fd_max=fd+1;

#ifdef _WIN32
        {
	  	unsigned long val = 1;
  		ioctlsocket(fd, FIONBIO, &val);
        }
#else
	result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

#ifndef NSOCKET
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof yes);
#endif
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);
#else /* NSOCKET */
	setsocketopts(fd);
#endif /* NSOCKET */

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = htonl( INADDR_ANY );
	server_address.sin_port        = htons(port);

	result = bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
	if( result == -1 ) {
		perror("bind");
		exit(1);
	}
	result = listen( fd, 5 );
	if( result == -1 ) { /* error */
		perror("listen");
		exit(1);
	}

	FD_SET(fd, &readfds );

	CREATE(session[fd], struct socket_data, 1);

	if(session[fd]==NULL){
		printf("out of memory : make_listen_port\n");
		exit(1);
	}
	memset(session[fd],0,sizeof(*session[fd]));
	session[fd]->func_recv = connect_client;

	return fd;
}

// Console Reciever [Wizputer]
int console_recieve(int i) {
	int n;
	char *buf;
	
	CREATE(buf, char , 64);
	
	memset(buf,0,sizeof(64));

	n = read(0, buf , 64);
	if ( n < 0 )
		printf("Console input read error\n");
	else
		session[0]->func_console(buf);
	return 0;
}

void set_defaultconsoleparse(int (*defaultparse)(char*))
{
	default_console_parse = defaultparse;
}

static int null_console_parse(char *buf)
{
	printf("null_console_parse : %s\n",buf);
	return 0;
}

// Console Input [Wizputer]
int start_console(void) {
	FD_SET(0,&readfds);
    
	CREATE(session[0], struct socket_data, 1);
	if(session[0]==NULL){
		printf("out of memory : start_console\n");
		exit(1);
	}
	
	memset(session[0],0,sizeof(*session[0]));
	
	session[0]->func_recv = console_recieve;
	session[0]->func_console = default_console_parse;
	
	return 0;
}   
    
int make_connection(long ip,int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;
#ifndef NSOCKET
	int yes = 1; // reuse fix
#endif /* not NSOCKET */

	fd = socket( AF_INET, SOCK_STREAM, 0 );
#ifndef NSOCKET
	if(fd_max<=fd) fd_max=fd+1;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof yes);
#endif
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);
#else /* NSOCKET */
	if(fd_max<=fd) 
		fd_max=fd+1;

	setsocketopts(fd);
#endif /* NSOCKET */

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port = htons(port);

#ifdef _WIN32
        {
            unsigned long val = 1;
            ioctlsocket(fd, FIONBIO, &val);
        }
#else
        result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	result = connect(fd, (struct sockaddr *)(&server_address),sizeof(struct sockaddr_in));

	FD_SET(fd,&readfds);

	CREATE(session[fd], struct socket_data, 1);
	CREATE(session[fd]->rdata, char, rfifo_size);
	CREATE(session[fd]->wdata, char, wfifo_size);

	session[fd]->max_rdata  = rfifo_size;
	session[fd]->max_wdata  = wfifo_size;
	session[fd]->func_recv  = recv_to_fifo;
	session[fd]->func_send  = send_from_fifo;
	session[fd]->func_parse = default_func_parse;

	return fd;
}

int delete_session(int fd)
{
	if(fd<0 || fd>=FD_SETSIZE)
		return -1;
	FD_CLR(fd,&readfds);
	if(session[fd]){
		if(session[fd]->rdata)
			free(session[fd]->rdata);
		if(session[fd]->wdata)
			free(session[fd]->wdata);
		if(session[fd]->session_data)
			free(session[fd]->session_data);
		free(session[fd]);
	}
	session[fd]=NULL;
	//printf("delete_session:%d\n",fd);
	return 0;
}

int realloc_fifo(int fd,int rfifo_size,int wfifo_size)
{
	struct socket_data *s=session[fd];
	if( s->max_rdata != rfifo_size && s->rdata_size < rfifo_size){
		RECREATE(s->rdata, char, rfifo_size);
		s->max_rdata  = rfifo_size;
	}
	if( s->max_wdata != wfifo_size && s->wdata_size < wfifo_size){
		RECREATE(s->wdata, char, wfifo_size);
		s->max_wdata  = wfifo_size;
	}
	return 0;
}

int WFIFOSET(int fd,int len)
{
	struct socket_data *s=session[fd];
#ifdef NSOCKET
	if (s == NULL  || s->wdata == NULL)
		return 0;
#endif /* NSOCKET */
	if( s->wdata_size+len+16384 > s->max_wdata ){
		unsigned char *sin_addr = (unsigned char *)&s->client_addr.sin_addr;
		realloc_fifo(fd,s->max_rdata, s->max_wdata <<1 );
		printf("socket: %d (%d.%d.%d.%d) wdata expanded to %d bytes.\n",fd, sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], s->max_wdata);
	}
	s->wdata_size=(s->wdata_size+(len)+2048 < s->max_wdata) ?
		 s->wdata_size+len : (printf("socket: %d wdata lost !!\n",fd),s->wdata_size);
#ifdef NSOCKET
	if (s->wdata_size > (TCP_FRAME_LEN)) 
		send_from_fifo(fd);
#endif /* NSOCKET */
	return 0;
}

int do_sendrecv(int next)
{
	fd_set rfd,wfd;
	struct timeval timeout;
	int ret,i;

	rfd=readfds;
	FD_ZERO(&wfd);
	for(i=0;i<fd_max;i++){
		if(!session[i] && FD_ISSET(i,&readfds)){
			printf("force clr fds %d\n",i);
			FD_CLR(i,&readfds);
			continue;
		}
		if(!session[i])
			continue;
		if(session[i]->wdata_size)
			FD_SET(i,&wfd);
	}
	timeout.tv_sec  = next/1000;
	timeout.tv_usec = next%1000*1000;
	ret = select(fd_max,&rfd,&wfd,NULL,&timeout);
	if(ret<=0)
		return 0;
	for(i=0;i<fd_max;i++){
		if(!session[i])
			continue;
		if(FD_ISSET(i,&wfd)){
			//printf("write:%d\n",i);
			if(session[i]->func_send)
			    //send_from_fifo(i);
				session[i]->func_send(i);
		}
		if(FD_ISSET(i,&rfd)){
			//printf("read:%d\n",i);
			if(session[i]->func_recv)
			    //recv_to_fifo(i);
				session[i]->func_recv(i);
		}
	}
	return 0;
}

int do_parsepacket(void)
{
	int i;
	for(i=0;i<fd_max;i++){
		if(!session[i])
			continue;
		if(session[i]->rdata_size==0 && session[i]->eof==0)
			continue;
		if(session[i]->func_parse){
			session[i]->func_parse(i);
			if(!session[i])
				continue;
		}
		RFIFOFLUSH(i);
	}
	return 0;
}

void do_socket(void)
{
	FD_ZERO(&readfds);
}

int RFIFOSKIP(int fd,int len)
{
	struct socket_data *s=session[fd];

	if (s->rdata_size-s->rdata_pos-len<0) {
		fprintf(stderr,"too many skip\n");
		exit(1);
	}

	s->rdata_pos = s->rdata_pos+len;

	return 0;
}


unsigned int addr_[16];   // ip addresses of local host (host byte order)
unsigned int naddr_ = 0;   // # of ip addresses

int  Net_Init(void)
{
#ifdef _WIN32
    char** a;
    unsigned int i;
    char fullhost[255];
    struct hostent* hent;
    
        /* Start up the windows networking */
    WSADATA wsaData;

    if ( WSAStartup(WINSOCK_VERSION, &wsaData) != 0 ) {
        printf("SYSERR: WinSock not available!\n");
        exit(1);
    }

    if(gethostname(fullhost, sizeof(fullhost)) == SOCKET_ERROR) {
        printf("Ugg.. no hostname defined!\n");
        return 0;
    } 

    // XXX This should look up the local IP addresses in the registry
    // instead of calling gethostbyname. However, the way IP addresses
    // are stored in the registry is annoyingly complex, so I'll leave
    // this as T.B.D.
    hent = gethostbyname(fullhost);
    if (hent == NULL) {
        printf("Cannot resolve our own hostname to a IP address");
        return 0;
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
    printf("SIOCGIFCONF failed!\n");
    return 0;
  }

  for(pos = 0; pos < ic.ifc_len;) 
  {
    struct ifreq * ir = (struct ifreq *) (ic.ifc_buf + pos);

    struct sockaddr_in * a = (struct sockaddr_in *) &(ir->ifr_addr);

    if(a->sin_family == AF_INET) {
      u_long ad = ntohl(a->sin_addr.s_addr);
      if(ad != INADDR_LOOPBACK) {
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

  return(0);
}
