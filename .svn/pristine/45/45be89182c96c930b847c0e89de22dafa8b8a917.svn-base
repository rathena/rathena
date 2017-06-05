#ifndef _rA_NETWORK_H_
#define _rA_NETWORK_H_

#include <netinet/in.h>
#include "cbasetypes.h"
#include "netbuffer.h" 
#include "evdp.h"

#ifndef MAXCONN
#define MAXCONN 16384
#endif


typedef struct SESSION{
	EVDP_DATA	evdp_data;	// Must be always the frist member! (some evdp's may rely on this fact)

	// Connection Type	
	enum{ NST_FREE=0, NST_LISTENER = 1, NST_CLIENT=2, NST_OUTGOING=3}	type;

	// Flags / Settings.
	bool v6; // is v6?
	bool disconnect_in_progress;	// To prevent stack overflows / recursive calls.
	
	
	union{ // union to save memory.
		struct sockaddr_in	v4;
		struct sockaddr_in6	v6;
	}addr;
	

	// "lowlevel" Handlers
	// (Implemented by the protocol specific parser)
	//
	bool (*onRecv)(int32 fd);	// return false = disconnect
	bool (*onSend)(int32 fd);	// return false = disconnect

	// Event Handlers for LISTENER type sockets 
	//
	// onConnect  gets Called when a connection has been 
	//	successfully accepted.
	//	Session entry is available in this Handler!
	//	A returncode of false will reejct the connection (disconnect)
	//	Note: When rejecting a connection in onConnect by returning false
	//		  The onDisconnect handler wont get called!
	//	Note: the onConnect Handler is also responsible for setting
	//		  the appropriate netparser (which implements onRecv/onSend..) [protocol specific]
	//	
	// onDisconnect  gets called when a connection gets disconnected 
	//				 (by peer as well as by core)
	//
	bool (*onConnect)(int32 fd);	// return false = disconnect (wont accept)
	void (*onDisconnect)(int32 fd);


	// 
	// Parser specific data
	//
	void *netparser_data;	// incase of RO Packet Parser, pointer to packet len table (uint16array)
	void (*onPacketComplete)(int32 fd, uint16 op, uint16 len, netbuf buf);
	

	//
	// Buffers
	// 
	struct{
		enum NETREADSTATE { NRS_WAITOP = 0,	NRS_WAITLEN = 1,	NRS_WAITDATA = 2}	state;
		
		uint32	head_left;
		uint16	head[2];
		
		netbuf	buf;
	} read;

	struct{
		uint32	max_outstanding;
		uint32	n_outstanding;
		
		uint32	dataPos;
				
		netbuf	buf, buf_last;				
	} write;
	
	// Application Level data Pointer
	// (required for backward compatibility with previous athena socket system.)
	void *data;
				
} SESSION;


/**
 * Subsystem Initialization / Finalization.
 *
 */
void network_init();
void network_final();


/**
 * Will do the net work :) ..
 */
void network_do();


/** 
 * Adds a new listner.
 *
 * @param v6	v6 listner? 
 * @param *addr	the address to listen on.
 * @param port	port to listen on
 *
 * @return -1 on error  otherwise  the identifier of the new listener.
 */
int32 network_addlistener(bool v6,  const char *addr,  uint16 port);


/**
 * Tries to establish an outgoing connection.
 *
 * @param v6		operate with IPv6 addresses?
 * @param addr		the address to connect to
 * @param port		the port to connect to
 * @param from_addr	the address to connect from (local source / optional if auto  -> NULL)
 * @param from_port the port to connect from (local source / optional if auto  -> 0)
 * @param onConnectionEstablishedHandler	callback that gets called when the connection is established.
 * @param onConnectionLooseHandler			callback that gets called when the connection gets disconnected (or the connection couldnt be established)
 *
 * @return -1 on error  otherwise  the identifier of the new connection
 */
int32 network_connect(bool v6,
						const char *addr,
						uint16 port,
						const char *from_addr,
						uint16 from_port,
						bool (*onConnectionEstablishedHandler)(int32 fd),
						void (*onConnectionLooseHandler)(int32 fd)
);

						

/**
 * Disconnects the given connection
 *
 * @param fd  connection identifier.
 *
 * @Note:
 * 	- onDisconnect callback gets called! 
 *	- cleares (returns) all assigned buffers
 *
 */
void network_disconnect(int32 fd);


/** 
 * Attach's a netbuffer at the end of sending queue to the given connection
 *
 * @param fd	connection identifier
 * @param buf	netbuffer to attach.
 */
void network_send(int32 fd,  netbuf buf);


/**
 * Sets the parser to RO Protocol like Packet Parser.
 * 
 * @param fd				connection identifier
 * @param *packetlentable	pointer to array of uint16 in size of UINT16_MAX,
 * @param onComplteProc		callback for packet completion.
 *
 * @note:
 * 	PacketLen Table Fromat:
 *	each element's offsets represents th ro opcode.
 *	value is length.
 *	a length of 0  means the packet is dynamic.
 *	a length of UINT16_MAX means the packet is unknown.
 *
 *	Static Packets must contain their hader in len so (0x64 ==  55 ..)
 *
 */
void network_parser_set_ro(int32 fd,
							int16 *packetlentable,
							void (*onPacketCompleteProc)(int32 fd,  uint16 op,  uint16 len,  netbuf buf) 
							);
#define ROPACKET_UNKNOWN UINT16_MAX
#define ROPACKET_DYNLEN 0




#endif
