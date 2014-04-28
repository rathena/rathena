//
// Network Subsystem (previously known as socket system)
//
// Author: Florian Wilkemeyer <fw@f-ws.de>
//
// Copyright (c) rAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
//
//#ifdef HAVE_ACCETP4
#define _GNU_SOURCE
//#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


#include "../common/cbasetypes.h"
#include "../common/showmsg.h"
#include "../common/timer.h"
#include "../common/evdp.h"
#include "../common/netbuffer.h"

#include "../common/network.h"

#define ENABLE_IPV6
#define HAVE_ACCEPT4
#define EVENTS_PER_CYCLE 10
#define PARANOID_CHECKS

// Local Vars (settings..)
static int l_ListenBacklog = 64;

//
// Global Session Array (previously exported as session[]
//
SESSION g_Session[MAXCONN];


//
static bool onSend(int32 fd);


#define _network_free_netbuf_async( buf ) add_timer( 0, _network_async_free_netbuf_proc, 0,  (intptr_t) buf)
static int _network_async_free_netbuf_proc(int tid, unsigned int tick, int id, intptr_t data){
	// netbuf is in data
	netbuffer_put( (netbuf)data );

	return 0;
}//end: _network_async_free_netbuf_proc()



void network_init(){
	SESSION *s;
	int32 i;
	
	memset(g_Session, 0x00, (sizeof(SESSION) * MAXCONN) );
	
	for(i = 0; i < MAXCONN; i++){
		s = &g_Session[i];

		s->type = NST_FREE;
		s->disconnect_in_progress = false;
	}
	
	// Initialize the corresponding event dispatcher
	evdp_init();

	//
	add_timer_func_list(_network_async_free_netbuf_proc, "_network_async_free_netbuf_proc");
		
}//end: network_init()


void network_final(){

	// @TODO:
	// .. disconnect and cleanup everything!
	
	evdp_final();

}//end: network_final()


void network_do(){
	struct EVDP_EVENT l_events[EVENTS_PER_CYCLE];
	register struct EVDP_EVENT *ev;
	register int n, nfds;
	register SESSION *s;
	
	nfds = evdp_wait( l_events,	EVENTS_PER_CYCLE, 1000); // @TODO: timer_getnext()
	
	for(n = 0; n < nfds; n++){
		ev = &l_events[n];
		s = &g_Session[ ev->fd ];
		
		if(ev->events & EVDP_EVENT_HUP){
			network_disconnect( ev->fd );	
			continue; // no further event processing.
		}// endif vent is HUP (disconnect)
		
		
		if(ev->events & EVDP_EVENT_IN){
			
			if(s->onRecv != NULL){
				if( false == s->onRecv(ev->fd) ){
					network_disconnect(ev->fd);
					continue; // ..
				}
			}else{
				ShowError("network_do: fd #%u has no onRecv proc set. - disconnecting\n", ev->fd);
				network_disconnect(ev->fd);
				continue;
			}	
						
		}// endif event is IN (recv)
		
		
		if(ev->events & EVDP_EVENT_OUT){
			if(s->onSend != NULL){
				if( false == s->onSend(ev->fd) ){
					network_disconnect(ev->fd);
					continue;
				}
			}else{
				ShowError("network_do: fd #%u has no onSend proc set. - disconnecting\n", ev->fd);
				network_disconnect(ev->fd);
				continue;
			}
		}// endif event is OUT (send)
		
	}//endfor
			
}//end: network_do()


static bool _setnonblock(int32 fd){
	int flags = fcntl(fd, F_GETFL, 0);
	if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0)
		return false;

	return true;
}//end: _setnonblock()


static bool _network_accept(int32 fd){
	SESSION *listener = &g_Session[fd];
	SESSION *s; 
	union{
		struct sockaddr_in v4;
#ifdef ENABLE_IPV6
		struct sockaddr_in6 v6;
#endif
	} _addr;
	int newfd;
	socklen_t addrlen;
	struct sockaddr *addr;

	// Accept until OS returns - nothing to accept anymore
	// - this is required due to our EVDP abstraction. (which handles on listening sockets similar to epoll's EPOLLET flag.)
	while(1){
#ifdef ENABLE_IPV6
		if(listener->v6 == true){
			addrlen = sizeof(_addr.v6);
			addr = (struct sockaddr*)&_addr.v6;
		}else{
#endif
			addrlen = sizeof(_addr.v4);
			addr = (struct sockaddr*)&_addr.v4;		
#ifdef ENABLE_IPV6
		}
#endif

#ifdef HAVE_ACCEPT4
		newfd = accept4(fd, addr, &addrlen, SOCK_NONBLOCK);
#else
		newfd = accept(fd, addr, &addrlen);
#endif

		if(newfd == -1){
			if(errno == EAGAIN || errno == EWOULDBLOCK)
				break; // this is fully valid & whished., se explaination on top of while(1)
			
			// Otherwis .. we have serious problems :( seems tahat our listner has gone away..
			// @TODO handle this .. 
			ShowError("_network_accept: accept() returned error. closing listener. (errno: %u / %s)\n", errno, strerror(errno));

			return false; // will call disconnect after return.
			//break;
		}

#ifndef HAVE_ACCEPT4 // no accept4 means, we have to set nonblock by ourself. ..
		if(_setnonblock(newfd) == false){
			ShowError("_network_accept: failed to set newly accepted connection nonblocking (errno: %u / %s). - disconnecting.\n", errno, strerror(errno));
			close(newfd);
			continue; 
		}
#endif

		// Check connection limits.
		if(newfd >= MAXCONN){
			ShowError("_network_accept: failed to accept connection - MAXCONN (%u) exceeded.\n", MAXCONN);
			close(newfd);
			continue; // we have to loop over the events (and disconnect them too ..) but otherwise we would leak event notifications.
		}


		// Create new Session.
		s = &g_Session[newfd];
		s->type = NST_CLIENT;
		
		// The new connection inherits listenr's handlers.
		s->onDisconnect = listener->onDisconnect;
		s->onConnect = listener->onConnect; // maybe useless but .. fear the future .. :~ 
	
		// Register the new connection @ EVDP
		if( evdp_addclient(newfd, &s->evdp_data) == false){
			ShowError("_network_accept: failed to accept connection - event subsystem returned an error.\n");
			close(newfd);
			s->type = NST_FREE;
		}
		
		// Call the onConnect handler on the listener.
		if( listener->onConnect(newfd) == false ){
			// Resfused by onConnect handler..
			evdp_remove(newfd, &s->evdp_data);
			
			close(newfd);
			s->type = NST_FREE;
			
			s->data = NULL; // be on the safe side ~ !
			continue;
		}
		
		
	}

	return true;
}//end: _network_accept()


void network_disconnect(int32 fd){
	SESSION *s = &g_Session[fd];
	netbuf b, bn;
	
	// Prevent recursive calls 
	// by wrong implemented on disconnect handlers.. and such..
	if(s->disconnect_in_progress == true)
		return; 	
		
	s->disconnect_in_progress = true;
	
	
	// Disconnect Todo:
	//	- Call onDisconnect Handler
	//	- Release all Assigned buffers.
	//	- remove from event system (notifications)
	//	- cleanup session structure
	//	- close connection. 
	//
	
	if(s->onDisconnect != NULL && 
		s->type != NST_LISTENER){
		
		s->onDisconnect( fd );
	}

	// Read Buffer 
	if(s->read.buf != NULL){
		netbuffer_put(s->read.buf);
		s->read.buf = NULL;
	}

	// Write Buffer(s)
	b = s->write.buf;
	while(1){
		if(b == NULL) break;

		bn = b->next;
		
		netbuffer_put(b);
		
		b = bn;
	}
	s->write.buf = NULL;
	s->write.buf_last = NULL;
	
	s->write.n_outstanding = 0;
	s->write.max_outstanding = 0;
	
		
	// Remove from event system.
	evdp_remove(fd, &s->evdp_data);
	
	// Cleanup Session Structure.
	s->type = NST_FREE;
	s->data = NULL; // no application level data assigned
	s->disconnect_in_progress = false;


	// Close connection	
	close(fd);	
	
}//end: network_disconnect()


int32 network_addlistener(bool v6,  const char *addr,  uint16 port){
	SESSION *s;
	int fd;
#ifdef SO_REUSEADDR
	int optval;
#endif

#if !defined(ENABLE_IPV6)
	if(v6 == true){
		 ShowError("network_addlistener(%c, '%s', %u):  this release has no IPV6 support.\n", (v6==true?'t':'f'), addr, port);
		 return -1;
	}
#endif

#ifdef ENABLE_IPV6
	if(v6 == true)
		fd = socket(AF_INET6, SOCK_STREAM, 0);
	else
#endif
		fd = socket(AF_INET, SOCK_STREAM, 0);

	// Error?
	if(fd == -1){
		ShowError("network_addlistener(%c, '%s', %u):  socket() failed (errno: %u / %s)\n",  (v6==true?'t':'f'), addr, port, errno, strerror(errno));
		return -1;
	}
	
	// Too many connections?
	if(fd >= MAXCONN){
		ShowError("network_addlistener(%c, '%s', %u):  cannot create listener, exceeds more than supported connections (%u).\n", (v6==true?'t':'f'),  addr, port, MAXCONN);
		close(fd);
		return -1;
	}

	s = &g_Session[fd];
	if(s->type != NST_FREE){ // additional checks.. :)
			ShowError("network_addlistener(%c, '%s', %u): failed, got fd #%u which is already in use in local session table?!\n", (v6==true?'t':'f'), addr, port, fd);
			close(fd);
			return -1;
	}

	// Fill ip addr structs
#ifdef ENABLE_IPV6
	if(v6 == true){
		memset(&s->addr.v6, 0x00, sizeof(s->addr.v6));
		s->addr.v6.sin6_family = AF_INET6;
		s->addr.v6.sin6_port = htons(port);
		if(inet_pton(AF_INET6, addr, &s->addr.v6.sin6_addr) != 1){
			ShowError("network_addlistener(%c, '%s', %u): failed to parse the given IPV6 address.\n", (v6==true?'t':'f'), addr, port);
			close(fd);
			return -1;
		}
	}else{
#endif
		memset(&s->addr.v4, 0x00, sizeof(s->addr.v4));
		s->addr.v4.sin_family = AF_INET;
		s->addr.v4.sin_port = htons(port);
		s->addr.v4.sin_addr.s_addr = inet_addr(addr);
#ifdef ENABLE_IPV6
	}
#endif

	// if OS has support for SO_REUSEADDR, apply the flag
	// so the address could be used when there're still time_wait sockets outstanding from previous application run.
#ifdef SO_REUSEADDR
	optval = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#endif

	// Bind
#ifdef ENABLE_IPV6
	if(v6 == true){
		if( bind(fd, (struct sockaddr*)&s->addr.v6,  sizeof(s->addr.v6)) == -1) {
			ShowError("network_addlistener(%c, '%s', %u): bind failed (errno: %u / %s)\n", (v6==true?'t':'f'), addr, port, errno, strerror(errno));
			close(fd);
			return -1;
		}
	}else{
#endif
		if( bind(fd, (struct sockaddr*)&s->addr.v4,  sizeof(s->addr.v4)) == -1) {
            ShowError("network_addlistener(%c, '%s', %u): bind failed (errno: %u / %s)\n", (v6==true?'t':'f'), addr, port, errno, strerror(errno));
			close(fd);
			return -1;
		}		
#ifdef ENABLE_IPV6
	}
#endif

	if( listen(fd, l_ListenBacklog) == -1){
		ShowError("network_addlistener(%c, '%s', %u): listen failed (errno: %u / %s)\n", (v6==true?'t':'f'), addr, port, errno, strerror(errno));
		close(fd);
		return -1;
	}
		

	// Set to nonblock!
	if(_setnonblock(fd) == false){
		ShowError("network_addlistener(%c, '%s', %u): cannot set to nonblock (errno: %u / %s)\n", (v6==true?'t':'f'), addr, port, errno, strerror(errno));
		close(fd);
		return -1;
	}	

	// Rgister @ evdp.
	if( evdp_addlistener(fd, &s->evdp_data) != true){
		ShowError("network_addlistener(%c, '%s', %u): eventdispatcher subsystem returned an error.\n", (v6==true?'t':'f'), addr, port);
		close(fd);
		return -1;
	}

	// Apply flags on Session array for this conneciton.
	if(v6 == true)	s->v6 = true;
	else			s->v6 = false;
	
	s->type = NST_LISTENER;
	s->onRecv = _network_accept;

	ShowStatus("Added Listener on '%s':%u\n", addr, port, (v6==true ? "(ipv6)":"(ipv4)") );

	return fd;
}//end: network_addlistener()


static bool _network_connect_establishedHandler(int32 fd){
	register SESSION *s = &g_Session[fd];
	int val;
	socklen_t val_len;
	
	if(s->type == NST_FREE)
		return true;	// due to multiple non coalesced event notifications
						// this can happen .. when a previous handled event has already disconnected the connection
						// within the same cycle..
	
	val = -1;
	val_len = sizeof(val);
	getsockopt(fd, SOL_SOCKET, SO_ERROR, &val, &val_len);
	
	if(val != 0){
		// :( .. cleanup session..
		s->type = NST_FREE;
		s->onSend = NULL;
		s->onConnect = NULL;
		s->onDisconnect = NULL;

		evdp_remove(fd, &s->evdp_data);
		close(fd);
		
		return true; // we CANT return false,
					 // becuase the normal disconnect procedure would execute the ondisconnect handler, which we dont want .. in this case.
	}else{
		// ok 
		if(s->onConnect(fd) == false) {
			// onConnect handler has refused the connection .. 
			// cleanup .. and ok
			s->type = NST_FREE;
			s->onSend = NULL;
			s->onConnect = NULL;
			s->onDisconnect = NULL;
			
			evdp_remove(fd, &s->evdp_data);
			close(fd);
			
			return true; // we dnot want the ondisconnect handler to be executed, so its okay to handle this by ourself.
		}
		
		// connection established ! 
		// 
		if( evdp_outgoingconnection_established(fd, &s->evdp_data) == false ){
			return false; // we want the normal disconnect procedure.. with call to ondisconnect handler.
		}
		
		s->onSend = NULL;
		
		ShowStatus("#%u connection successfull!\n", fd);	
	}

	return true;	
}//end: _network_connect_establishedHandler()


int32 network_connect(bool v6,
                        const char *addr,
                        uint16 port,
                        const char *from_addr,
                        uint16 from_port,
                        bool (*onConnectionEstablishedHandler)(int32 fd),
                        void (*onConnectionLooseHandler)(int32 fd)
){
	register SESSION *s;
	int32 fd, ret;
	struct sockaddr_in ip4;
#ifdef SO_REUSEADDR
	int optval;
#endif
#ifdef ENABLE_IPV6
	struct sockaddr_in6 ip6;
#endif

#ifdef ENABLE_IPV6
	if(v6 == true)
		fd = socket(AF_INET6, SOCK_STREAM, 0);
	else
#endif
		fd = socket(AF_INET, SOCK_STREAM, 0);

#ifndef ENABLE_IPV6
	// check..
	if(v6 == true){
		ShowError("network_connect(%c, '%s', %u...): tried to create an ipv6 connection, IPV6 is not supported in this release.\n", (v6==true?'t':'f'), addr, port);
		return -1;
	}
#endif

	// check connection limits.
	if(fd >= MAXCONN){
		ShowError("network_connect(%c, '%s', %u...): cannot create new connection, exceeeds more than supported connections (%u)\n", (v6==true?'t':'f'), addr, port );
		close(fd);
		return -1;
	}

	// Originating IP/Port pair given ?
	if(from_addr != NULL && *from_addr != 0){
		//.. 
		#ifdef SO_REUSEADDR
	    optval=1;
	    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		#endif

		#ifdef ENABLE_IPV6
		if(v6 == true){
			memset(&ip6, 0x00, sizeof(ip6));
			ip6.sin6_family = AF_INET6;
			ip6.sin6_port = htons(from_port);
			
			if(inet_pton(AF_INET6, from_addr, &ip6.sin6_addr) != 1){
				ShowError("network_connect(%c, '%s', %u...): cannot parse originating (from) IPV6 address (errno: %u / %s)\n", (v6==true?'t':'f'), addr, port, errno, strerror(errno));
				close(fd);
				return -1;
 			}

 			ret = bind(fd, (struct sockaddr*)&ip6, sizeof(ip6));
		}else{
		#endif
			memset(&ip4, 0x00, sizeof(ip4));

			ip4.sin_family = AF_INET;
			ip4.sin_port = htons(from_port);
			ip4.sin_addr.s_addr = inet_addr(from_addr);
			ret = bind(fd, (struct sockaddr*)&ip4, sizeof(ip4));
		#ifdef ENABLE_IPV6
		}
		#endif
		
	}

	// Set non block
	if(_setnonblock(fd) == false){
		ShowError("network_connect(%c, '%s', %u...): cannot set socket to nonblocking (errno: %u / %s)\n", (v6==true?'t':'f'), addr, port, errno, strerror(errno));
		close(fd);
		return -1;
	}


	// Create ip addr block to connect to .. 
#ifdef ENABLE_IPV6
	if(v6 == true){
		memset(&ip6, 0x00, sizeof(ip6));
		ip6.sin6_family = AF_INET6;
		ip6.sin6_port = htons(port);
		
		if(inet_pton(AF_INET6, addr, &ip6.sin6_addr) != 1){
			 ShowError("network_connect(%c, '%s', %u...): cannot parse destination IPV6 address (errno: %u / %s)\n", (v6==true?'t':'f'), addr, port, errno, strerror(errno));
			 close(fd);
			 return -1;
		}
	
	}else{
#endif
	memset(&ip4, 0x00, sizeof(ip4));
	
	ip4.sin_family = AF_INET;
	ip4.sin_port = htons(port);
	ip4.sin_addr.s_addr = inet_addr(addr);	
#ifdef ENABLE_IPV6
	}
#endif

	// Assign Session..
	s = &g_Session[fd];
	s->type = NST_OUTGOING;
	s->v6 = v6;
	s->onConnect = onConnectionEstablishedHandler;
	s->onDisconnect = onConnectionLooseHandler;
	s->onRecv = NULL;
	s->onSend = _network_connect_establishedHandler;
#ifdef ENABLE_IPV6
	if(v6 == true)
		memcpy(&s->addr.v6, &ip6, sizeof(ip6));
	else
#endif
		memcpy(&s->addr.v4, &ip4, sizeof(ip4));
	
	// Register @ EVDP. as outgoing (see doc of the function)
	if(evdp_addconnecting(fd, &s->evdp_data) == false){
		ShowError("network_connect(%c, '%s', %u...): eventdispatcher subsystem returned an error.\n", (v6==true?'t':'f'), addr, port);
		
		// cleanup session x.x.. 
		s->type = NST_FREE;
		s->onConnect = NULL;
		s->onDisconnect = NULL;
		s->onSend = NULL;
		
		// close, return error code.
		close(fd);
		return -1;
	}

#ifdef ENABLE_IPV6
	if(v6 == true)
		ret = connect(fd, (struct sockaddr*)&ip6, sizeof(ip6));
	else
#endif
		ret = connect(fd, (struct sockaddr*)&ip4, sizeof(ip4));


	// 
	if(ret != 0 && errno != EINPROGRESS){
		ShowWarning("network_connect(%c, '%s', %u...): connection failed (errno: %u / %s)\n", (v6==true?'t':'f'),  addr, port, errno, strerror(errno));
		
		// Cleanup session ..
		s->type = NST_FREE;
		s->onConnect = NULL;
		s->onDisconnect = NULL;
		s->onSend = NULL;
		
		// .. remove from evdp and close fd.	
		evdp_remove(fd, &s->evdp_data);
		close(fd);
		return -1;
	}

	// ! The Info Message :~D
	ShowStatus("network_connect fd#%u (%s:%u) in progress.. \n", fd, addr, port);

return fd;	
}//end: network_connect()


static bool _onSend(int32 fd){
	register SESSION *s = &g_Session[fd];
	register netbuf buf, buf_next;
	register uint32 szNeeded;
	register int wLen;

	if(s->type == NST_FREE)
		return true; 	// Possible due to multipl non coalsced event notifications 
						// so onSend gets called after disconnect caused by an previous vent. 
						// we can ignore the call to onSend, then. 
	
	buf = s->write.buf;
	while(1){
		if(buf == NULL)
			break;
		
		buf_next = buf->next;
		
		
		szNeeded = (buf->dataLen - s->write.dataPos);	// using th session-local .dataPos member, due to shared write buffer support.
		
		// try to write.
		wLen = write(fd, &buf->buf[s->write.dataPos],  szNeeded);
		if(wLen == 0){
			return false; // eof.
		}else if(wLen == -1){
			if(errno == EAGAIN || errno == EWOULDBLOCK)
				return true; // dont disconnect / try again later.
			
			// all other errors. . 
			return false;
		}
		
		// Wrote data.. =>
		szNeeded -= wLen;
		if(szNeeded > 0){
			// still data left .. 
			// 
			s->write.dataPos += wLen; // fix offset.
			return true;
		}else{
			// this buffer has been written successfully
			// could be returned to pool.
			netbuffer_put(buf);
			s->write.n_outstanding--; // When threadsafe -> Interlocked here.
			s->write.dataPos = 0; 
		}
			
			
		buf = buf_next;
	}

	// okay,
	// reaching this part means:
	// while interrupted by break - 
	// which means all buffers are written, nothing left
	//
	
	s->write.buf_last = NULL;
	s->write.buf = NULL;
	s->write.n_outstanding = 0;
	s->write.dataPos = 0;
	
	// Remove from event dispatcher (write notification)
	//
	evdp_writable_remove(fd, &s->evdp_data);
			
	return true;	
}//end: _onSend()


static bool _onRORecv(int32 fd){
	register SESSION *s = &g_Session[fd];
	register uint32	szNeeded;
	register char *p;
	register int rLen;
	
	if(s->type == NST_FREE)
		return true;	// Possible due to multiple non coalesced events by evdp.
						//  simply ignore this call returning positive result. 
	
	// Initialize p and szNeeded depending on change
	// 
	switch(s->read.state){
		case NRS_WAITOP:
			szNeeded = s->read.head_left;
			p = ((char*)&s->read.head[0]) + (2-szNeeded);
		break;

		case NRS_WAITLEN:
			szNeeded = s->read.head_left;
			p = ((char*)&s->read.head[1]) + (2-szNeeded);
		break;
		
		case NRS_WAITDATA:{
			register netbuf buf = s->read.buf;
			
			szNeeded = (buf->dataLen - buf->dataPos);
			p = (char*)&buf->buf[ buf->dataPos ];
		}
		break;	
		
		default: 
			// .. the impossible gets possible .. 
			ShowError("_onRORecv: fd #%u has unknown read.state (%d) - disconnecting\n", fd, s->read.state);
			return false;
		break;
	}
	
	
	// 
	
	rLen = read(fd, p, szNeeded);
	if(rLen == 0){
		// eof..
		return false;
	}else if(rLen == -1){
		
		if(errno == EAGAIN || errno == EWOULDBLOCK){
			// try again later .. (this case shouldnt happen, because we're event trigered.. but .. sometimes it happens :)
			return true;
		}
		
		// an additional interesting case would be 
		// EINTR, this 'could' be handled .. but:
		//	posix says that its possible that data gets currupted during irq
		//	or data gor read and not reported.., so we'd have a data loss..
		//	(which shouldnt happen with stream based protocols such as tcp)
		// its better to disonnect the client in that case.
		
		return false;
	}
	
	//
	// Got Data:
	//  next action also depends on current state .. 
	// 
	szNeeded -= rLen;
	switch(s->read.state){
		case NRS_WAITOP:

			if(szNeeded > 0){
				// still data missing .. 
				s->read.head_left = szNeeded;
				return true; // wait for completion.
			}else{
				// complete .. 
				//  next state depends on packet type.
				
				s->read.head[1] = ((uint16*)s->netparser_data)[ s->read.head[0] ];  // store lenght of packet by opcode head[0] to head[1]
				
				if(s->read.head[1] == ROPACKET_UNKNOWN){
					// unknown packet - disconnect
					ShowWarning("_onRORecv: fd #%u got unlnown packet 0x%04x - disconnecting.\n", fd, s->read.head[0]);
					return false;
				}
				else if(s->read.head[1] == ROPACKET_DYNLEN){
					// dynamic length
					// next state: requrie len.
					s->read.state = NRS_WAITLEN;
					s->read.head_left = 2;
					return true; //
				}
				else if(s->read.head[1] == 2){ 
					// packet has no data (only opcode)
					register netbuf buf = netbuffer_get(2); // :D whoohoo its giant! 
					
					NBUFW(buf, 0) = s->read.head[0]; // store opcode @ packet begin.
					buf->dataPos = 2;
					buf->dataLen = 2;
					buf->next = NULL;
					
					// Back to initial state -> Need opcode.
					s->read.state = NRS_WAITOP;
					s->read.head_left = 2;
					s->read.buf = NULL;
					
					// Call completion routine here.					
					s->onPacketComplete(fd,  s->read.head[0],  2,  buf); 
					
					return true; // done :)
				}
				else{
					// paket needs .. data ..
					register netbuf buf = netbuffer_get( s->read.head[1] );
					
					NBUFW(buf, 0) = s->read.head[0]; // store opcode @ packet begin.
					buf->dataPos = 2;
					buf->dataLen = s->read.head[1];
					buf->next = NULL;
					
					// attach buffer.
					s->read.buf = buf;
					
					// set state:
					s->read.state = NRS_WAITDATA;	
					
					return true;
				}
				
			}//endif: szNeeded > 0 (opcode read completed?)
			
		break;
		
		
		case NRS_WAITLEN:
			
			if(szNeeded > 0){
				// incomplete .. 
				s->read.head_left = szNeeded;
				return true;
			}else{
				
				if(s->read.head[1] == 4){
					// packet has no data (only opcode + length)
					register netbuf buf = netbuffer_get( 4 );
					
					NBUFL(buf, 0) = *((uint32*)&s->read.head[0]); // copy  Opcode + length to netbuffer using MOVL	
					buf->dataPos = 4;
					buf->dataLen = 4;
					buf->next = NULL;
					
					// set initial state (need opcode)
					s->read.state = NRS_WAITOP;
					s->read.head_left = 2; 
					s->read.buf = NULL;
					
					// call completion routine.
					s->onPacketComplete(fd,  s->read.head[0],  4,  buf);

					return true;					
				}
				else if(s->read.head[1] < 4){
					// invalid header.
					ShowWarning("_onRORecv: fd #%u invalid header - got packet 0x%04x, reported length < 4 - INVALID - disconnecting\n", fd, s->read.head[0]);
					return false;
				}
				else{
					// Data needed
					// next state -> waitdata!
					register netbuf buf = netbuffer_get( s->read.head[1] );
					
					NBUFL(buf, 0) = *((uint32*)&s->read.head[0]); // copy  Opcode + length to netbuffer using MOVL
					buf->dataPos = 4;
					buf->dataLen = s->read.head[1];
					buf->next = NULL;
					
					// attach to session:
					s->read.buf = buf;
					s->read.state = NRS_WAITDATA;
					
					return true;
				}
				
			}//endif: szNeeded > 0 (length read complete?)
			
		break;
		
		
		case NRS_WAITDATA:

			if(szNeeded == 0){
				// Packet finished!
				// compltion.
				register netbuf buf = s->read.buf;
				
				// set initial state.
				s->read.state = NRS_WAITOP;
				s->read.head_left = 2;
				s->read.buf = NULL;
				
				// Call completion routine.
				s->onPacketComplete(fd,  NBUFW(buf, 0),  buf->dataLen,  buf);
				
				return true;
			}else{
				// still data needed 
				s->read.buf->dataPos += rLen; 
				
				return true;				
			}
		break;
		
		
		//
		default:
			ShowError("_onRORecv: fd #%u has unknown read.state (%d) [2] - disconnecting\n", fd, s->read.state);
			return false;
		break;
	}
	
		
	return false;
}//end: _onRORecv()


void network_send(int32 fd,  netbuf buf){
	register SESSION *s = &g_Session[fd];
	
#ifdef PARANOID_CHECKS
	if(fd >= MAXCONN){
		ShowError("network_send: tried to attach buffer to connection idientifer #%u which is out of bounds.\n", fd);
		_network_free_netbuf_async(buf);
		return;
	}
#endif


	if(s->type == NST_FREE)
		return;
	
	// Check Max Outstanding buffers limit.
	if( (s->write.max_outstanding > 0)	&&
		(s->write.n_outstanding >= s->write.max_outstanding) ){
		
		ShowWarning("network_send: fd #%u max Outstanding buffers exceeded. - disconnecting.\n", fd);
		network_disconnect(fd);
		//
		_network_free_netbuf_async(buf);
		return;
	}
	

	// Attach to the end:
	buf->next = NULL;
	if(s->write.buf_last != NULL){
		s->write.buf_last->next = buf; 
		s->write.buf_last = buf;

	}else{
		// currently no buffer attached.
		s->write.buf = s->write.buf_last = buf;
		
		// register @ evdp for writable notification.
		evdp_writable_add(fd, &s->evdp_data); // 
	}
	
	
	//
	s->write.n_outstanding++;
	
}//end: network_send()


void network_parser_set_ro(int32 fd,
							int16 *packetlentable,
							void (*onPacketCompleteProc)(int32 fd,  uint16 op,  uint16 len,  netbuf buf) 
							){
	register SESSION *s = &g_Session[fd];
	register netbuf b, nb; // used for potential free attached buffers.
	
	if(s->type == NST_FREE)
		return;
	
	s->onPacketComplete = onPacketCompleteProc;
	
	s->onRecv = _onRORecv; // ..
	s->onSend = _onSend; // Using the normal generic netbuf based send function.
		
	s->netparser_data = packetlentable;
	
	// Initial State -> Need Packet OPCode.
	s->read.state = NRS_WAITOP;
	s->read.head_left = 2; 

	
	// Detach (if..) all buffers.
	if(s->read.buf != NULL){
		_network_free_netbuf_async(s->read.buf); //
		s->read.buf = NULL;	
	}
	
	if(s->write.buf != NULL){
		b = s->write.buf;
		while(1){
			nb = b->next;
			
			_network_free_netbuf_async(b);
			
			b = nb;
		}
	
		s->write.buf = NULL;
		s->write.buf_last = NULL;
		s->write.n_outstanding = 0;
	}
	
	// not changing any limits on outstanding ..
	//
	
}//end: network_parser_set_ro()
