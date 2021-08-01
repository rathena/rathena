#pragma once

#ifdef SOCKET_LIBEVENT
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <event2/util.h>

#include "core.hpp"
#include "socket.hpp"

int le_start();
void le_listener_cb(struct evconnlistener*, evutil_socket_t, struct sockaddr*, int socklen, void*);
int le_makeconnect(uint32 ip, uint16 port);
void le_timercb(intptr fd, short event, void* arg);
void le_tryendsession(int flag);
void le_eventstop();

extern struct event_base* le_base;
extern struct event* le_timeout;
extern struct timeval le_tv;
extern uint32 le_bind_ip;
extern uint16 le_bind_port;

#ifdef SOCKET_LIBEVENT_USE_WIN_CRIT
extern CRITICAL_SECTION crit;
#endif

// imported orig rathena functions
int le_create_session(int fd, RecvFunc func_recv, SendFunc func_send, ParseFunc func_parse);
void le_delete_session(int fd);

extern ParseFunc default_func_parse;

int null_recv(int fd);
int null_send(int fd);
int null_parse(int fd);

extern int ip_rules;
int le_connect_check(uint32 ip);
int send_from_fifo(int fd);
#endif
