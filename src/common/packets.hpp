// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PACKETS_HPP
#define PACKETS_HPP

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

#pragma warning( push )
#pragma warning( disable : 4200 )

#define DEFINE_PACKET_HEADER( name, id ) const int16 HEADER_##name = id
#define DEFINE_PACKET_ID( name, id ) DEFINE_PACKET_HEADER( name, id )

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( push, 1 )
#endif

struct PACKET_CA_LOGIN{
	int16 packetType;
	uint32 version;
	char username[NAME_LENGTH];
	char password[NAME_LENGTH];
	uint8 clienttype;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_LOGIN, 0x64 );

#if PACKETVER >= 20170315
struct PACKET_AC_ACCEPT_LOGIN_sub{
	uint32 ip;
	uint16 port;
	char name[20];
	uint16 users;
	uint16 type;
	uint16 new_;
	uint8 unknown[128];
} __attribute__((packed));

struct PACKET_AC_ACCEPT_LOGIN{
	int16 packetType;
	int16 packetLength;
	uint32 login_id1;
	uint32 AID;
	uint32 login_id2;
	uint32 last_ip;
	char last_login[26];
	uint8 sex;
	char token[WEB_AUTH_TOKEN_LENGTH];
	PACKET_AC_ACCEPT_LOGIN_sub char_servers[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( AC_ACCEPT_LOGIN, 0xac4 );
#else
struct PACKET_AC_ACCEPT_LOGIN_sub{
	uint32 ip;
	uint16 port;
	char name[20];
	uint16 users;
	uint16 type;
	uint16 new_;
} __attribute__((packed));

struct PACKET_AC_ACCEPT_LOGIN{
	int16 packetType;
	int16 packetLength;
	uint32 login_id1;
	uint32 AID;
	uint32 login_id2;
	uint32 last_ip;
	char last_login[26];
	uint8 sex;
	PACKET_AC_ACCEPT_LOGIN_sub char_servers[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( AC_ACCEPT_LOGIN, 0x69 );
#endif

// not sure when this started
#if PACKETVER >= 20120000
struct PACKET_AC_REFUSE_LOGIN{
	int16 packetType;
	uint32 error;
	char unblock_time[20];
} __attribute__((packed));
DEFINE_PACKET_HEADER( AC_REFUSE_LOGIN, 0x83e );
#else
struct PACKET_AC_REFUSE_LOGIN{
	int16 packetType;
	uint8 error;
	char unblock_time[20];
} __attribute__((packed));
DEFINE_PACKET_HEADER( AC_REFUSE_LOGIN, 0x6a );
#endif

struct PACKET_SC_NOTIFY_BAN{
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( SC_NOTIFY_BAN, 0x81 );

struct PACKET_CA_REQ_HASH{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_REQ_HASH, 0x1db );

struct PACKET_AC_ACK_HASH{
	int16 packetType;
	int16 packetLength;
	char salt[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( AC_ACK_HASH, 0x1dc );

struct PACKET_CA_LOGIN2{
	int16 packetType;
	uint32 version;
	char username[NAME_LENGTH];
	uint8 passwordMD5[16];
	uint8 clienttype;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_LOGIN2, 0x1dd );

struct PACKET_CA_LOGIN3{
	int16 packetType;
	uint32 version;
	char username[NAME_LENGTH];
	uint8 passwordMD5[16];
	uint8 clienttype;
	uint8 clientinfo;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_LOGIN3, 0x1fa );

struct PACKET_CA_CONNECT_INFO_CHANGED{
	int16 packetType;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_CONNECT_INFO_CHANGED, 0x200 );

struct PACKET_CA_EXE_HASHCHECK{
	int16 packetType;
	char hash[16];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_EXE_HASHCHECK, 0x204 );

struct PACKET_CA_LOGIN_PCBANG{
	int16 packetType;
	uint32 version;
	char username[NAME_LENGTH];
	char password[NAME_LENGTH];
	uint8 clienttype;
	char ip[16];
	char mac[13];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_LOGIN_PCBANG, 0x277 );

struct PACKET_CA_LOGIN4{
	int16 packetType;
	uint32 version;
	char username[NAME_LENGTH];
	uint8 passwordMD5[16];
	uint8 clienttype;
	char mac[13];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_LOGIN4, 0x27c );

struct PACKET_CA_LOGIN_CHANNEL{
	int16 packetType;
	uint32 version;
	char username[NAME_LENGTH];
	char password[NAME_LENGTH];
	uint8 clienttype;
	char ip[16];
	char mac[13];
	uint8 is_gravity;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_LOGIN_CHANNEL, 0x2b0 );

struct PACKET_CA_SSO_LOGIN_REQ{
	int16 packetType;
	int16 packetLength;
	uint32 version;
	uint8 clienttype;
	char username[NAME_LENGTH];
	char password[27];
	char mac[17];
	char ip[15];
	char token[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_SSO_LOGIN_REQ, 0x825 );

struct PACKET_CT_AUTH{
	int16 packetType;
	uint8 unknown[66];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CT_AUTH, 0xacf );

struct PACKET_TC_RESULT{
	int16 packetType;
	int16 packetLength;
	uint32 type;
	char unknown1[20];
	char unknown2[6];
} __attribute__((packed));
DEFINE_PACKET_HEADER( TC_RESULT, 0xae3 );

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( pop )
#endif

#pragma warning( pop )

#endif /* PACKETS_HPP */
