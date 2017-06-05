#ifndef _rA_EVDP_H_
#define _rA_EVDP_H_

#include "cbasetypes.h"

typedef struct EVDP_DATA EVDP_DATA;


//#idef EVDP_EPOLL
#include <sys/epoll.h>
struct EVDP_DATA{
	struct epoll_event ev_data;
	bool ev_added;
};
//#endif


enum EVDP_EVENTFLAGS{
	EVDP_EVENT_IN = 1,	// Incomming data  
	EVDP_EVENT_OUT = 2,	// Connection accepts writing.
	EVDP_EVENT_HUP = 4	// Connection Closed.
};

typedef struct EVDP_EVENT{
	int32	events;	// due to performance reasons, this should be the first member.
	int32	fd;	// Connection Identifier
} EVDP_EVENT;



/** 
 * Network Event Dispatcher Initialization / Finalization routines
 */
void evdp_init();
void evdp_final();


/**
 * Will Wait for events.
 *
 * @param *out_ev 		pointer to array in size at least of max_events.
 * @param max_events	max no of events to report with this call (coalesc)
 * @param timeout_ticks	max time to wait in ticks (milliseconds) 
 *
 * @Note:
 * 	The function will block until an event has occured on one of the monitored connections
 *	or the timeout of timeout_ticks has passed by.
 *	Upon successfull call (changed connections) this function will write the connection
 *	Identifier & event  to the out_fds array. 
 *
 * @return 	0 -> Timeout, 	> 0 no of changed connections.
 */
int32 evdp_wait(EVDP_EVENT *out_fds,	int32 max_events, 	int32 timeout_ticks);


/** 
 * Applys the given mask on the given connection.
 * 
 * @param fd	connection identifier
 * @param *ep	event data pointer for the connection
 * @param mask	new event mask we're monitoring for.
 */
//void evdp_apply(int32 fd,  EVDP_DATA *ep,	int32 mask);


/** 
 * Adds a connection (listner) to the event notification system.
 *
 * @param fd 	connection identifier
 * @param *ep	event data pointer for the connection 
 *
 * @note: 
 *	Listener type sockets are edge triggered, (see epoll manual for more information)
 *  - This basicaly means that youll receive one event, adn you have to accept until accept returns an error (nothing to accept)
 *
 * MONITORS by default:   IN
 * 
 * @return success indicator.
 */ 
bool evdp_addlistener(int32 fd, EVDP_DATA *ep);

/**
 * Adds a connection (client connectioN) to the event notification system
 *
 * @param fd	connection identifier
 * @param *ep	event data pointr for the connection
 * 
 * @note:
 * 
 * MONITORS by default:	IN, HUP
 *
 * @return success indicator.
 */
bool evdp_addclient(int32 fd, EVDP_DATA *ep);

/**
 * Adds a connection (pending / outgoing connection!) to the event notification system.
 *
 * @param fd	connection identifier
 * @param *ep	event data pointer for the conneciton.
 *
 * @note:
 *	Outgoing connection type sockets are getting monitored for connection established
 *	successfull
 *	- if the connection has been established - we're generitng a writable notification .. (send) 
 * 		this is typical for BSD / posix conform network stacks.
 *	- Additinionally its edge triggered.
 *
 * @see evdp_outgoingconnection_established  
 *
 *
 * @return success indicator
 */
bool evdp_addconnecting(int32 fd, EVDP_DATA *ep);

/**
 * Adds an outgoing connection to the normal event notification system after it has been successfully established. 
 *
 * @param fd	connection identifier
 * @param *ep	event data pointer for the conneciton.
 
 * @note 
 * 	after this call, its handled like a normal "client" connection (incomming)
 * 
 * @rturn success indicator
 */
bool evdp_outgoingconnection_established(int32 fd, EVDP_DATA *ep);

/**
 * Marks a connection to be monitored for writable.
 *
 * @param fd	connection identifier
 * @param *ep	event data pointer for the connection
 *
 * @note:
 *	the connection must be already added (as client or listener)
 * 
 *
 * @return success indicator
 */
bool evdp_writable_add(int32 fd, EVDP_DATA *ep);

/** 
 * Removes the connection from writable notification monitoring
 *
 * @param fd	connection identifier
 * @param *ep	event data pointr for the connection
 *
 */ 
void evdp_writable_remove(int32 fd, EVDP_DATA *ep);

/**
 * Removes an connectio from the event notification system.
 *
 * @param fd  connection iditentfir
 * @param *ep  event data pointer for th connection
 *
 *
 * @note:
 * 	this will also clear the given EVENT_DATA block 
 *	so the connection slot is in an "initial" blank status / ready to get reused.
 *
 */
void evdp_remove(int32 fd, 	EVDP_DATA *ep);



#endif
