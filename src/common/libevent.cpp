
#ifdef WIN32
#include "winapi.hpp"
typedef long in_addr_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "libevent.hpp"
#include "showmsg.hpp"

#ifdef SOCKET_LIBEVENT
// this is library for windows, compiled with VS2019, in linux you need to install libevent2 then compile the source with -levent parameter
#ifdef _WIN64
#pragma comment(lib, "../../3rdparty/libevent/lib/libevent_x64.lib")
#pragma comment(lib, "../../3rdparty/libevent/lib/libevent_core_x64.lib")
#else
#ifdef _WIN32
#pragma comment(lib, "../libevent/lib/libevent.lib")
#pragma comment(lib, "../libevent/lib/libevent_core.lib")
#endif
#endif

event_base* le_base;
event* le_timeout;
timeval le_tv;

uint32 le_bind_ip;
uint16 le_bind_port;

#ifdef SOCKET_LIBEVENT_USE_WIN_CRIT
CRITICAL_SECTION crit;
#endif

int le_start()
{
	int fd, err = 0;
	struct evconnlistener* listener;
	struct sockaddr_in sin;

#ifdef WIN32
	event_config* pConfig = event_config_new();
	event_config_set_flag(pConfig, EVENT_BASE_FLAG_STARTUP_IOCP); // force backend to IOCP
	event_config_set_num_cpus_hint(pConfig, 4); // lets try 4 worker threads, or comment this line to automatically set no. of workers depend on CPU cores.
	evthread_use_windows_threads();
	le_base = event_base_new_with_config(pConfig);
	event_config_free(pConfig);
#else// this will use EPOLL in linux
	evthread_use_pthreads();
	le_base = event_base_new();
#endif
	if (!le_base) {
		ShowError("Could not initialize libevent!\n");
		return 0;
	}
	else
	{
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(le_bind_ip);
		sin.sin_port = htons(le_bind_port);

		listener = evconnlistener_new_bind(le_base, le_listener_cb, le_base,
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
			(struct sockaddr*)&sin, sizeof(sin));

		if (!listener) {
			if (le_base)event_base_free(le_base);
			le_base = 0;
			shutdown_callback();
			ShowError("Could not create a listener!\n");
			return 0;
		}

		fd = evconnlistener_get_fd(listener);

		if (fd == -1)
		{
			if (le_base)event_base_free(le_base);
			le_base = 0;
			shutdown_callback();
			ShowError("evconnlistener_new_bind: socket creation failed\n");
			return 0;
		}

		if (fd == 0)
		{// reserved
			if (le_base)event_base_free(le_base);
			le_base = 0;
			shutdown_callback();
			ShowError("evconnlistener_new_bind: Socket #0 is reserved - Please report this!!!\n");
			return 0;
		}

		if (fd >= FD_SETSIZE)
		{
			if (le_base)event_base_free(le_base);
			le_base = 0;
			shutdown_callback();
			ShowError("fd >= FD_SETSIZE, fd : %d\n", fd);
			return 0;
		}

		le_create_session(fd, null_recv, null_send, default_func_parse);
		session[fd]->client_addr = 0; // just listens
		session[fd]->rdata_tick = 0; // disable timeouts on this socket
		session[fd]->le_rdata = 0;
		if (fd_max <= fd) fd_max = fd + 1;

		le_timeout = event_new(le_base, -1, 0, le_timercb, 0);
		evutil_timerclear(&le_tv);
		le_tv.tv_sec = 1;
		event_add(le_timeout, &le_tv); // add a recurring event with le_timercb, start with 1 second delay..
		last_tick = time(0);

		ShowStatus("Libevent started.\n");
		event_base_dispatch(le_base); // start libevent, this will keep on waiting for events and pass it to callbacks

		ShowStatus("Libevent is closing...\n");
		// dispatch brake is called, start cleanup now...
		evconnlistener_free(listener);
	}
	return 1;
}


void le_timercb(intptr fd, short event, void* arg)
{
	int next;
	next = do_timer(gettick_nocache());

	evutil_timerclear(&le_tv);
	le_tv.tv_sec = next / 1000;
	le_tv.tv_usec = next % 1000 * 1000;
	event_add(le_timeout, &le_tv);
	le_tryendsession(0);
}

void le_tryendsession(int flag)
{
	last_tick = time(NULL);

	for (int i = 1; i < fd_max; i += 1)
	{
		if (session[i])
		{
			if (session[i]->le_close_tick > 0 && (flag == 1 || last_tick > session[i]->le_close_tick))
			{
				le_delete_session(i);
			}
			else if (session[i]->rdata_tick && DIFF_TICK(last_tick, session[i]->rdata_tick) > stall_time) {
				if (session[i]->flag.server) {/* server is special */
					if (session[i]->flag.ping != 2)/* only update if necessary otherwise it'd resend the ping unnecessarily */
						session[i]->flag.ping = 1;
				}
				else {
					ShowInfo("Session #%d timed out\n", i);
					set_eof(i);
					session[i]->func_parse(i);
				}
			}
		}
	}
}

void le_conn_eventcb(struct bufferevent* bev, short events, void* user_data)
{
	intptr fd = (intptr)user_data;

	if (events & BEV_EVENT_EOF){
		set_eof(fd);
		session[fd]->func_parse(fd);
		session[fd]->le_close_tick = time(0) + 1;
	}
	else if (events & BEV_EVENT_ERROR){
		set_eof(fd);
		session[fd]->func_parse(fd);
		session[fd]->le_close_tick = time(0) + 1;
	}
	else if (events & BEV_EVENT_CONNECTED){
		session[fd]->func_parse(fd);
		session[fd]->func_send(fd);
		if (session[fd]->flag.eof){
			session[fd]->func_parse(fd);
		}
	}
}

void le_conn_writecb(struct bufferevent* bev, void* ptr)
{
	struct evbuffer* evbuf = bufferevent_get_output(bev);
	size_t len = evbuffer_get_length(evbuf);
	ShowStatus("conn_writecb - write buf len : %d\n", len);
}


void le_conn_readcb(struct bufferevent* bev, void* ptr)
{
	int len;// , i;
	intptr fd = (intptr)ptr;

	if (!session_isValid(fd)){
		return;
	}

	if (session[fd]->le_close_tick > 0){
		return;
	}

	len = bufferevent_read(bev, (char*)session[fd]->rdata + session[fd]->rdata_size, (int)RFIFOSPACE(fd));

	if (len == 0){
		return;
	}

	session[fd]->rdata_size += len;
	session[fd]->rdata_tick = last_tick;
	session[fd]->func_parse(fd);
	session[fd]->func_send(fd);

	RFIFOFLUSH(fd);
}


void le_listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int socklen, void* user_data)
{
	struct bufferevent* bev;
	struct sockaddr_in* client_address = (sockaddr_in*)sa;

	bev = bufferevent_socket_new(le_base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);

	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(le_base);
		return;
	}

	fd = bufferevent_getfd(bev);

	if (fd >= FD_SETSIZE){
		ShowError("bufferevent_socket_new: fd out of bound - %d! \n", fd);
		bufferevent_free(bev);
		return;
	}

	if (fd == -1) {
		ShowError("bufferevent_socket_new: accept failed!\n");
		bufferevent_free(bev);
		return;
	}

	if (fd == 0){
		ShowError("bufferevent_socket_new: Socket #0 is reserved - Please report this!!!\n");
		bufferevent_free(bev);
		return;
	}

	if (ip_rules && !le_connect_check(ntohl(client_address->sin_addr.s_addr))) {
		do_close(fd);
		return;
	}

	if (fd_max <= fd) fd_max = fd + 1;

	le_create_session(fd, null_recv, send_from_fifo, default_func_parse);
	session[fd]->client_addr = ntohl(client_address->sin_addr.s_addr);
	session[fd]->le_rdata = bev;

	bufferevent_setcb(bev, le_conn_readcb, NULL, le_conn_eventcb, (void*)fd);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_enable(bev, EV_READ);
}

int le_makeconnect(uint32 ip, uint16 port)
{
	struct bufferevent* bev;
	struct sockaddr_in remote_address;
	int fd;
	int result;

	if (!le_base)
	{
		ShowError("Bufferevent base is not valid!\n");
		return -1;
	}

	bev = bufferevent_socket_new(le_base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);

	if (!bev)
	{
		ShowError("bufferevent_socket_new: Failed to create bufferevent! \n");
		return -1;
	}

	remote_address.sin_family = AF_INET;
	remote_address.sin_addr.s_addr = htonl(ip);
	remote_address.sin_port = htons(port);

	ShowStatus("Connecting to %d.%d.%d.%d:%i\n", CONVIP(ip), port);

	result = bufferevent_socket_connect(bev, (struct sockaddr*)&remote_address, sizeof(remote_address));
	fd = bufferevent_getfd(bev);

	ShowStatus("result:%d fd:%d\n", result, fd);

	if (result == SOCKET_ERROR) {
		ShowError("make_connection: connect failed (socket #%d)!\n", fd);
		do_close(fd);
		return -1;
	}

	if (fd >= FD_SETSIZE)
	{
		ShowError("bufferevent_socket_new: fd out of bound, %d! \n", fd);
		bufferevent_free(bev);
		return -1;
	}

	if (fd == -1) {
		ShowError("bufferevent_socket_new: socket creation failed, %d !\n", fd);
		return -1;
	}

	if (fd == 0){
		ShowError("bufferevent_socket_new: Socket #0 is reserved - Please report this!!!\n");
		return -1;
	}

	if (fd_max <= fd) fd_max = fd + 1;

	le_create_session(fd, null_recv, send_from_fifo, default_func_parse);
	session[fd]->client_addr = ntohl(remote_address.sin_addr.s_addr);
	session[fd]->le_close_tick = 0;
	session[fd]->le_rdata = bev;

	bufferevent_setcb(bev, le_conn_readcb, NULL, le_conn_eventcb, (void*)fd);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_enable(bev, EV_READ);

	return fd;
}

void le_eventstop()
{
	if (le_base){
		struct timeval delay = { 2, 0 };
		event_base_loopexit(le_base, &delay);
	}
}
#endif

