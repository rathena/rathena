// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SOCKET_HPP
#define SOCKET_HPP

#ifdef WIN32
	#include "winapi.hpp"
	typedef long in_addr_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif
#include <time.h>

#include "cbasetypes.hpp"
#include "timer.hpp" // t_tick

#define FIFOSIZE_SERVERLINK 256*1024

// (^~_~^) Gepard Shield Start

#define  LICENSE_ID  1769879070
#define  CODE_VERSION  2016122701

#define  SESSION_CONST_1  0xB645CB7C
#define  SESSION_CONST_2  0xD9EAECA7

#define  ALGO_KEY_1  0x93
#define  ALGO_KEY_2  0x11
#define  ALGO_KEY_3  0xF7

#define  DATA_HASH_CONST_1  0x93AD0C0C
#define  DATA_HASH_CONST_2  0x935E5590

// (^~_~^) Gepard Shield End

// (^~_~^) Gepard Shield Start

extern long long start_tick;
extern bool is_gepard_active;
extern unsigned int min_allowed_license_version;

#define MATRIX_SIZE     2048
#define KEY_SIZE        32

#define GEPARD_REASON_LENGTH 99
#define GEPARD_TIME_STR_LENGTH 24
#define GEPARD_RESULT_STR_LENGTH 100

struct gepard_crypt_unit
{
	unsigned int flag;
	unsigned char pos_1;
	unsigned char pos_2;
	unsigned char pos_3;
	unsigned char key[256];
};

struct gepard_info_data
{
	
	long long sync_tick;
	unsigned int unique_id;
	unsigned int license_version;
	unsigned char mac_address[6];
	unsigned int is_init_ack_received;
	unsigned short cs_packet_request_move;
	unsigned short cs_packet_action_request;
	unsigned short cs_packet_use_skill_to_id;
	unsigned short cs_packet_use_skill_to_ground;
};

enum gepard_server_types
{
	GEPARD_MAP      = 0xCCCC,
	GEPARD_LOGIN    = 0xAAAA,
};

enum gepard_info_type
{
	GEPARD_INFO_MESSAGE,
	GEPARD_INFO_MESSAGE_EXIT,
	GEPARD_INFO_INVALID_INIT_ACK,
	GEPARD_INFO_BANNED,
	GEPARD_INFO_OLD_LICENSE_VERSION,
};

enum gepard_packets
{
	CS_LOAD_END_ACK        = 0x007D,
	CS_WHISPER_TO          = 0x0096,

	CS_LOGIN_PACKET_1      = 0x0064,
	CS_LOGIN_PACKET_2      = 0x0277,
	CS_LOGIN_PACKET_3      = 0x02b0,
	CS_LOGIN_PACKET_4      = 0x01dd,
	CS_LOGIN_PACKET_5      = 0x01fa,
	CS_LOGIN_PACKET_6      = 0x027c,
	CS_LOGIN_PACKET_7      = 0x0825,

	SC_SET_UNIT_WALKING_1  = 0x07F7,
	SC_SET_UNIT_WALKING_2  = 0x0856,
	SC_SET_UNIT_WALKING_3  = 0x0914,
	SC_SET_UNIT_WALKING_4  = 0x09DB,
	SC_SET_UNIT_WALKING_5  = 0x09FD,

	SC_SET_UNIT_IDLE_1     = 0x07F9,
	SC_SET_UNIT_IDLE_2     = 0x0857,
	SC_SET_UNIT_IDLE_3     = 0x0915,
	SC_SET_UNIT_IDLE_4     = 0x09DD,
	SC_SET_UNIT_IDLE_5     = 0x09FF,

	SC_NOTIFY_TIME         = 0x007F,
	SC_STATE_CHANGE        = 0x0229,

	SC_MSG_STATE_CHANGE_1  = 0x0196,
	SC_MSG_STATE_CHANGE_2  = 0x043F,
	SC_MSG_STATE_CHANGE_3  = 0x0983,

	SC_WHISPER_FROM        = 0x0097,
	SC_WHISPER_SEND_ACK    = 0x0098,

	CS_GEPARD_SYNC         = 0x8285,
	CS_GEPARD_INIT_ACK     = 0xC392,

	SC_GEPARD_INIT         = 0x4753,
	SC_GEPARD_INFO         = 0xBCDE,
	SC_GEPARD_SETTINGS     = 0x5395,
};

enum gepard_internal_packets
{
	GEPARD_M2C_BLOCK_REQ   = 0x5000,
	GEPARD_C2M_BLOCK_ACK   = 0x5001,
	GEPARD_M2C_UNBLOCK_REQ = 0x5002,
	GEPARD_C2M_UNBLOCK_ACK = 0x5003,
};

void gepard_set_eof(int fd);
long long gepard_get_tick(void);
unsigned int gepard_get_unique_id(int fd);
struct socket_data* gepard_get_socket_data(int fd);
void gepard_init(struct socket_data* s, int fd, uint16 server_type);
void gepard_send_info(int fd, unsigned short info_type, const char* message);
bool gepard_process_cs_packet(int fd, struct socket_data* s, size_t packet_size);
void gepard_process_sc_packet(int fd, struct socket_data* s, size_t packet_size);

// (^~_~^) Gepard Shield End

// socket I/O macros
#define RFIFOHEAD(fd)
#define WFIFOHEAD(fd, size) do{ if((fd) && session[fd]->wdata_size + (size) > session[fd]->max_wdata ) realloc_writefifo(fd, size); }while(0)
#define RFIFOP(fd,pos) (session[fd]->rdata + session[fd]->rdata_pos + (pos))
#define WFIFOP(fd,pos) (session[fd]->wdata + session[fd]->wdata_size + (pos))

#define RFIFOCP(fd,pos) ((char*)RFIFOP(fd,pos))
#define WFIFOCP(fd,pos) ((char*)WFIFOP(fd,pos))
#define RFIFOB(fd,pos) (*(uint8*)RFIFOP(fd,pos))
#define WFIFOB(fd,pos) (*(uint8*)WFIFOP(fd,pos))
#define RFIFOW(fd,pos) (*(uint16*)RFIFOP(fd,pos))
#define WFIFOW(fd,pos) (*(uint16*)WFIFOP(fd,pos))
#define RFIFOL(fd,pos) (*(uint32*)RFIFOP(fd,pos))
#define WFIFOL(fd,pos) (*(uint32*)WFIFOP(fd,pos))
#define RFIFOF(fd,pos) (*(float*)RFIFOP(fd,pos))
#define WFIFOF(fd,pos) (*(float*)WFIFOP(fd,pos))
#define RFIFOQ(fd,pos) (*(uint64*)RFIFOP(fd,pos))
#define WFIFOQ(fd,pos) (*(uint64*)WFIFOP(fd,pos))
#define RFIFOSPACE(fd) (session[fd]->max_rdata - session[fd]->rdata_size)
#define WFIFOSPACE(fd) (session[fd]->max_wdata - session[fd]->wdata_size)

#define RFIFOREST(fd)  (session[fd]->flag.eof ? 0 : session[fd]->rdata_size - session[fd]->rdata_pos)
#define RFIFOFLUSH(fd) \
	do { \
		if(session[fd]->rdata_size == session[fd]->rdata_pos){ \
			session[fd]->rdata_size = session[fd]->rdata_pos = 0; \
		} else { \
			session[fd]->rdata_size -= session[fd]->rdata_pos; \
			memmove(session[fd]->rdata, session[fd]->rdata+session[fd]->rdata_pos, session[fd]->rdata_size); \
			session[fd]->rdata_pos = 0; \
		} \
	} while(0)

// buffer I/O macros
#define RBUFP(p,pos) (((uint8*)(p)) + (pos))
#define RBUFCP(p,pos) ((char*)RBUFP((p),(pos)))
#define RBUFB(p,pos) (*(uint8*)RBUFP((p),(pos)))
#define RBUFW(p,pos) (*(uint16*)RBUFP((p),(pos)))
#define RBUFL(p,pos) (*(uint32*)RBUFP((p),(pos)))
#define RBUFQ(p,pos) (*(uint64*)RBUFP((p),(pos)))

#define WBUFP(p,pos) (((uint8*)(p)) + (pos))
#define WBUFCP(p,pos) ((char*)WBUFP((p),(pos)))
#define WBUFB(p,pos) (*(uint8*)WBUFP((p),(pos)))
#define WBUFW(p,pos) (*(uint16*)WBUFP((p),(pos)))
#define WBUFL(p,pos) (*(uint32*)WBUFP((p),(pos)))
#define WBUFQ(p,pos) (*(uint64*)WBUFP((p),(pos)))

#define TOB(n) ((uint8)((n)&UINT8_MAX))
#define TOW(n) ((uint16)((n)&UINT16_MAX))
#define TOL(n) ((uint32)((n)&UINT32_MAX))


// Struct declaration
typedef int (*RecvFunc)(int fd);
typedef int (*SendFunc)(int fd);
typedef int (*ParseFunc)(int fd);

struct socket_data
{
	struct {
		unsigned char eof : 1;
		unsigned char server : 1;
		unsigned char ping : 2;
	} flag;

	uint32 client_addr; // remote client address

	uint8 *rdata, *wdata;
	size_t max_rdata, max_wdata;
	size_t rdata_size, wdata_size;
	size_t rdata_pos;
	time_t rdata_tick; // time of last recv (for detecting timeouts); zero when timeout is disabled

	RecvFunc func_recv;
	SendFunc func_send;
	ParseFunc func_parse;

	void* session_data; // stores application-specific data related to the session

// (^~_~^) Gepard Shield Start
	struct gepard_info_data gepard_info;
	struct gepard_crypt_unit send_crypt;
	struct gepard_crypt_unit recv_crypt;
	struct gepard_crypt_unit sync_crypt;
// (^~_~^) Gepard Shield End

};


// Data prototype declaration

extern struct socket_data* session[FD_SETSIZE];

extern int fd_max;

extern time_t last_tick;
extern time_t stall_time;

//////////////////////////////////
// some checking on sockets
extern bool session_isValid(int fd);
extern bool session_isActive(int fd);
//////////////////////////////////

// Function prototype declaration

int make_listen_bind(uint32 ip, uint16 port);
int make_connection(uint32 ip, uint16 port, bool silent, int timeout);
int realloc_fifo(int fd, unsigned int rfifo_size, unsigned int wfifo_size);
int realloc_writefifo(int fd, size_t addition);
int WFIFOSET(int fd, size_t len);
int RFIFOSKIP(int fd, size_t len);

int do_sockets(t_tick next);
void do_close(int fd);
void socket_init(void);
void socket_final(void);

extern void flush_fifo(int fd);
extern void flush_fifos(void);
extern void set_nonblocking(int fd, unsigned long yes);

void set_defaultparse(ParseFunc defaultparse);


/// Server operation request
enum chrif_req_op {
	// Char-server <-> login-server oepration
	CHRIF_OP_LOGIN_BLOCK = 1,
	CHRIF_OP_LOGIN_BAN,
	CHRIF_OP_LOGIN_UNBLOCK,
	CHRIF_OP_LOGIN_UNBAN,
	CHRIF_OP_LOGIN_CHANGESEX,
	CHRIF_OP_LOGIN_VIP,

	// Char-server operation
	CHRIF_OP_BAN,
	CHRIF_OP_UNBAN,
	CHRIF_OP_CHANGECHARSEX,
};


// hostname/ip conversion functions
uint32 host2ip(const char* hostname);
const char* ip2str(uint32 ip, char ip_str[16]);
uint32 str2ip(const char* ip_str);
#define CONVIP(ip) ((ip)>>24)&0xFF,((ip)>>16)&0xFF,((ip)>>8)&0xFF,((ip)>>0)&0xFF
#define MAKEIP(a,b,c,d) (uint32)( ( ( (a)&0xFF ) << 24 ) | ( ( (b)&0xFF ) << 16 ) | ( ( (c)&0xFF ) << 8 ) | ( ( (d)&0xFF ) << 0 ) )
uint16 ntows(uint16 netshort);

int socket_getips(uint32* ips, int max);

extern uint32 addr_[16];   // ip addresses of local host (host byte order)
extern int naddr_;   // # of ip addresses

void set_eof(int fd);

/// Use a shortlist of sockets instead of iterating all sessions for sockets
/// that have data to send or need eof handling.
/// Adapted to use a static array instead of a linked list.
///
/// @author Buuyo-tama
#define SEND_SHORTLIST

#ifdef SEND_SHORTLIST
// Add a fd to the shortlist so that it'll be recognized as a fd that needs
// sending done on it.
void send_shortlist_add_fd(int fd);
// Do pending network sends (and eof handling) from the shortlist.
void send_shortlist_do_sends();
#endif

#endif /* SOCKET_HPP */
