// original : core.c 2003/02/26 18:03:12 Rev 1.7

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <io.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifndef SIOCGIFCONF
#include <sys/sockio.h> // SIOCGIFCONF on Solaris, maybe others? [Shinomori]
#endif

#endif

#include <fcntl.h>
#include <string.h>

#include "socket.h"
#include "../common/dll.h"
#include "../common/mmo.h"	// [Valaris] thanks to fov
#include "../common/timer.h"
#include "../common/utils.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

fd_set readfds;
int fd_max;
time_t tick_;
time_t stall_time_ = 60;
int ip_rules = 1;

// #define UPNP

#ifdef UPNP
#if defined(CYGWIN) || defined(_WIN32)
DLL upnp_dll;
int (*upnp_init)();
int (*upnp_final)();
int (*firewall_addport)(char *desc, int port);
int (*upnp_addport)(char *desc, char *ip, int port);
extern char *argp;

int release_mappings = 1;
int close_ports = 1;
#else
#error This doesnt work with non-Windows yet
#endif
#endif

int rfifo_size = 65536;
int wfifo_size = 65536;

#ifndef TCP_FRAME_LEN
#define TCP_FRAME_LEN 1053
#endif

#define CONVIP(ip) ip&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,ip>>24

struct socket_data *session[FD_SETSIZE];

static int null_parse(int fd);
static int (*default_func_parse)(int) = null_parse;

static int null_console_parse(char *buf);
static int (*default_console_parse)(char*) = null_console_parse;
static int connect_check(unsigned int ip);

/*======================================
 *	CORE : Set function
 *--------------------------------------
 */
void set_defaultparse(int (*defaultparse)(int))
{
	default_func_parse = defaultparse;
}

void set_nonblocking(int fd, int yes) {
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);
}

static void setsocketopts(int fd)
{
	int yes = 1; // reuse fix

	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof yes);
#endif
	set_nonblocking(fd, yes);

	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &wfifo_size , sizeof(rfifo_size ));
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &rfifo_size , sizeof(rfifo_size ));
}

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
		session[fd]->rdata_tick = tick_;
	} else if(len<=0){
		// value of connection is not necessary the same
//		printf("set eof : connection #%d\n", fd);
		session[fd]->eof=1;
	}
	return 0;
}

static int send_from_fifo(int fd)
{
	int len;

	//printf("send_from_fifo : %d\n",fd);
	if(session[fd]->eof || session[fd]->wdata == 0)
		return -1;
	if (session[fd]->wdata_size == 0)
		return 0;

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
	} else if (errno != EAGAIN) {
//		printf("set eof :%d\n",fd);
		session[fd]->eof=1;
	}
	return 0;
}

void flush_fifos()
{
	int i;
	for(i=0;i<fd_max;i++)
		if(session[i] != NULL &&
		   session[i]->func_send == send_from_fifo)
			send_from_fifo(i);
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
#ifndef _WIN32
	int result;
#endif

	//printf("connect_client : %d\n",listen_fd);

	len=sizeof(client_address);

	fd = accept(listen_fd,(struct sockaddr*)&client_address,(socklen_t*)&len);
	if(fd_max<=fd) fd_max=fd+1;

	setsocketopts(fd);

	if(fd==-1) {
		perror("accept");
		return -1;
	} else if (ip_rules && !connect_check(*(unsigned int*)(&client_address.sin_addr))) {
		close(fd);
		return -1;
	} else
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
	CREATE_A(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE_A(session[fd]->wdata, unsigned char, wfifo_size);

	session[fd]->max_rdata   = rfifo_size;
	session[fd]->max_wdata   = wfifo_size;
	session[fd]->func_recv   = recv_to_fifo;
	session[fd]->func_send   = send_from_fifo;
	session[fd]->func_parse  = default_func_parse;
	session[fd]->client_addr = client_address;
	session[fd]->rdata_tick = tick_;

  //printf("new_session : %d %d\n",fd,session[fd]->eof);
	return fd;
}

int make_listen_port(int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

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

	setsocketopts(fd);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = htonl( INADDR_ANY );
	server_address.sin_port        = htons((unsigned short)port);

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

int make_listen_bind(long ip,int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

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

	setsocketopts(fd);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port        = htons((unsigned short)port);

#ifdef UPNP
	if (upnp_dll) {
		int localaddr = ntohl(addr_[0]);
		unsigned char *natip = (unsigned char *)&localaddr;
		char buf[16];
		sprintf(buf, "%d.%d.%d.%d", natip[0], natip[1], natip[2], natip[3]);
		//printf("natip=%d.%d.%d.%d\n", natip[0], natip[1], natip[2], natip[3]);
		if (firewall_addport(argp, port))
			printf ("Firewall port %d successfully opened\n", port);
		if (natip[0] == 192 && natip[1] == 168) {
			if (upnp_addport(argp, natip, port))
				printf ("Upnp mappings successfull\n");
			else printf ("Upnp mapping failed\n");
		}
	}
#endif

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
		printf("out of memory : make_listen_bind\n");
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

	CREATE_A(buf, char , 64);

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

	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(fd_max<=fd)
		fd_max=fd+1;

	setsocketopts(fd);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port = htons((unsigned short)port);

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
	CREATE_A(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE_A(session[fd]->wdata, unsigned char, wfifo_size);

	session[fd]->max_rdata  = rfifo_size;
	session[fd]->max_wdata  = wfifo_size;
	session[fd]->func_recv  = recv_to_fifo;
	session[fd]->func_send  = send_from_fifo;
	session[fd]->func_parse = default_func_parse;
	session[fd]->rdata_tick = tick_;

	return fd;
}

int delete_session(int fd)
{
	if(fd<=0 || fd>=FD_SETSIZE)
		return -1;
	FD_CLR(fd,&readfds);
	if(session[fd]){
		if(session[fd]->rdata)
			aFree(session[fd]->rdata);
		if(session[fd]->wdata)
			aFree(session[fd]->wdata);
		if(session[fd]->session_data)
			aFree(session[fd]->session_data);
		aFree(session[fd]);
	}
	session[fd]=NULL;
	//printf("delete_session:%d\n",fd);
	return 0;
}

int realloc_fifo(int fd,int rfifo_size,int wfifo_size)
{
	struct socket_data *s;

	if (fd <= 0) return 0;
	s = session[fd];
	if( s->max_rdata != rfifo_size && s->rdata_size < rfifo_size){
		RECREATE(s->rdata, unsigned char, rfifo_size);
		s->max_rdata  = rfifo_size;
	}
	if( s->max_wdata != wfifo_size && s->wdata_size < wfifo_size){
		RECREATE(s->wdata, unsigned char, wfifo_size);
		s->max_wdata  = wfifo_size;
	}
	return 0;
}

int WFIFOSET(int fd,int len)
{
	struct socket_data *s;

	if (fd <= 0) return 0;
	s = session[fd];
	if (s == NULL  || s->wdata == NULL)
		return 0;
	if( s->wdata_size+len+16384 > s->max_wdata ){
		unsigned char *sin_addr = (unsigned char *)&s->client_addr.sin_addr;
		realloc_fifo(fd,s->max_rdata, s->max_wdata <<1 );
		printf("socket: %d (%d.%d.%d.%d) wdata expanded to %d bytes.\n",fd, sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], s->max_wdata);
	}
	s->wdata_size=(s->wdata_size+(len)+2048 < s->max_wdata) ?
		 s->wdata_size+len : (printf("socket: %d wdata lost !!\n",fd),s->wdata_size);
	if (s->wdata_size > (TCP_FRAME_LEN))
		send_from_fifo(fd);
	return 0;
}

int do_sendrecv(int next)
{
	fd_set rfd,wfd;
	struct timeval timeout;
	int ret,i;

	tick_ = time(0);

	memcpy(&rfd, &readfds, sizeof(rfd));

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
				session[i]->func_send(i);
		}
		if(FD_ISSET(i,&rfd)){
			//printf("read:%d\n",i);
			if(session[i]->func_recv)
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
		if ((session[i]->rdata_tick != 0) && ((tick_ - session[i]->rdata_tick) > stall_time_))
			session[i]->eof = 1;
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

/* DDoS 攻撃対策 */

enum {
	ACO_DENY_ALLOW=0,
	ACO_ALLOW_DENY,
	ACO_MUTUAL_FAILTURE,
};

struct _access_control {
	unsigned int ip;
	unsigned int mask;
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
		printf("connect_check: Connection from %d.%d.%d.%d %s\n",
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
				printf("connect_check: Found match from allow list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
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
				printf("connect_check: Found match from deny list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
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
					printf("connect_check: DDOS Attack detected from %d.%d.%d.%d!\n",
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
			if(
				(DIFF_TICK(tick,hist->tick) > ddos_interval * 3 && !hist->status) ||
				(DIFF_TICK(tick,hist->tick) > ddos_autoreset && hist->status)
			) {
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
				list++;
			}
		}
	}
	if(access_debug) {
		printf("connect_check_clear: Cleared %d of %d from IP list.\n", clear, clear+list);
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
			printf("access_ipmask: Unknown format %s!\n",str);
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
		printf("access_ipmask: Loaded IP:%d.%d.%d.%d mask:%d.%d.%d.%d\n",
			CONVIP(ip), CONVIP(mask));
	}
	acc->ip   = ip;
	acc->mask = mask;
	return 1;
}

int socket_config_read(const char *cfgName) {
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=fopen(cfgName, "r");
	if(fp==NULL){
		printf("File not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"stall_time")==0){
			stall_time_ = atoi(w2);
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
	#ifdef UPNP
		} else if(!strcmpi(w1,"release_mappings")){
			if(strcmpi(w2,"yes")==0)
				release_mappings = 1;
			else if(strcmpi(w2,"no")==0)
				release_mappings = 0;
			else release_mappings = atoi(w2);
		} else if(!strcmpi(w1,"close_ports")){
			if(strcmpi(w2,"yes")==0)
				close_ports = 1;
			else if(strcmpi(w2,"no")==0)
				close_ports = 0;
			else close_ports = atoi(w2);
	#endif
		} else if (strcmpi(w1, "import") == 0)
			socket_config_read(w2);
	}
	fclose(fp);
	return 0;
}

int RFIFOSKIP(int fd,int len)
{
	struct socket_data *s;

	if (fd <= 0) return 0;
	s = session[fd];

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

#ifdef UPNP
// not implemented yet ^^;
void do_init_upnp(void)
{
	int *_release_mappings;
	int *_close_ports;

	upnp_dll = DLL_OPEN ("upnp.dll");
	if (!upnp_dll) {
		printf ("Cannot open upnp.dll: %s\n", dlerror());
		return;
	}
	DLL_SYM (upnp_init, upnp_dll, "do_init");
	DLL_SYM (upnp_final, upnp_dll, "do_final");
	DLL_SYM (firewall_addport, upnp_dll, "Firewall_AddPort");
	DLL_SYM (upnp_addport, upnp_dll, "UPNP_AddPort");
	if (!upnp_init || !upnp_final || !firewall_addport || !upnp_addport) {
		printf ("Cannot load symbol: %s\n", dlerror());
		DLL_CLOSE (upnp_dll);
		upnp_dll = NULL;
		return;
	}

	DLL_SYM (_release_mappings, upnp_dll, "release_mappings");
	DLL_SYM (_close_ports, upnp_dll, "close_ports");
	if (release_mappings && _release_mappings)
		*_release_mappings = release_mappings;
	if (close_ports && _close_ports)
		*_close_ports = close_ports;

	if (upnp_init() == 0) {
		printf ("Error initialising upnp.dll, unloading...\n");
		DLL_CLOSE (upnp_dll);
		upnp_dll = NULL;
	}
	return;
}
#endif

void do_final_socket(void)
{
	int i;
	struct _connect_history *hist , *hist2;
	for(i=0; i<fd_max; i++) {
		if(session[i]) {
			delete_session(i);
		}
	}
	for(i=0; i<0x10000; i++) {
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

	// session[0] のダミーデータを削除
	aFree(session[0]->rdata);
	aFree(session[0]->wdata);
	aFree(session[0]);

#ifdef UPNP
	if (upnp_dll) {
		upnp_final();
		DLL_CLOSE(upnp_dll);
	}
#endif
}

void do_socket(void)
{
	char *SOCKET_CONF_FILENAME = "conf/packet_athena.conf";

	FD_ZERO(&readfds);

	atexit(do_final_socket);
	socket_config_read(SOCKET_CONF_FILENAME);

	// session[0] にダミーデータを確保する
	CREATE(session[0], struct socket_data, 1);
	CREATE_A(session[0]->rdata, unsigned char, rfifo_size);
	CREATE_A(session[0]->wdata, unsigned char, wfifo_size);
	session[0]->max_rdata   = rfifo_size;
	session[0]->max_wdata   = wfifo_size;

	// とりあえず５分ごとに不要なデータを削除する
	add_timer_interval(gettick()+1000,connect_check_clear,0,0,300*1000);

#ifdef UPNP
	do_init_upnp();
#endif
}
