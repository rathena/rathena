// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PACKETS_HPP
#define PACKETS_HPP

#include <functional>
#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/utilities.hpp>

#pragma warning( push )
#pragma warning( disable : 4200 )

#define DEFINE_PACKET_HEADER( name, id ) const int16 HEADER_##name = id

// NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#if !defined( sun ) && ( !defined( __NETBSD__ ) || __NetBSD_Version__ >= 600000000 )
	#pragma pack( push, 1 )
#endif

struct PACKET{
	int16 packetType;
	int16 packetLength;
} __attribute__((packed));

struct CHARACTER_INFO{
	uint32 GID;
#if PACKETVER >= 20170830
	int64 exp;
#else
	int32 exp;
#endif
	int32 money;
#if PACKETVER >= 20170830
	int64 jobexp;
#else
	int32 jobexp;
#endif
	int32 joblevel;
	int32 bodystate;
	int32 healthstate;
	int32 effectstate;
	int32 virtue;
	int32 honor;
	int16 jobpoint;
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
	int64 hp;
	int64 maxhp;
	int64 sp;
	int64 maxsp;
#else
	int32 hp;
	int32 maxhp;
	int16 sp;
	int16 maxsp;
#endif
	int16 speed;
	int16 job;
	int16 head;
#if PACKETVER >= 20141022
	int16 body;
#endif
	int16 weapon;
	int16 level;
	int16 sppoint;
	int16 accessory;
	int16 shield;
	int16 accessory2;
	int16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	char name[24];
	uint8 Str;
	uint8 Agi;
	uint8 Vit;
	uint8 Int;
	uint8 Dex;
	uint8 Luk;
	uint8 CharNum;
	uint8 hairColor;
	int16 bIsChangedCharName;
#if (PACKETVER >= 20100720 && PACKETVER <= 20100727) || PACKETVER >= 20100803
	char mapName[16];
#endif
#if PACKETVER >= 20100803
	int32 DelRevDate;
#endif
#if PACKETVER >= 20110111
	int32 robePalette;
#endif
#if PACKETVER >= 20110928
	int32 chr_slot_changeCnt;
#endif
#if PACKETVER >= 20111025
	int32 chr_name_changeCnt;
#endif
#if PACKETVER >= 20141016
	uint8 sex;
#endif
} __attribute__((packed));

struct PACKET_CA_LOGIN{
	int16 packetType;
	uint32 version;
	char username[NAME_LENGTH];
	char password[NAME_LENGTH];
	uint8 clienttype;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CA_LOGIN, 0x64 );

struct PACKET_CH_SELECT_CHAR{
	int16 packetType;
	uint8 slot;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_SELECT_CHAR, 0x66 );

#if PACKETVER >= 20151001
struct PACKET_CH_MAKE_CHAR{
	int16 packetType;
	char name[NAME_LENGTH];
	uint8 slot;
	uint16 hair_color;
	uint16 hair_style;
	uint32 job;
	uint8 sex;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_MAKE_CHAR, 0xa39 );
#elif PACKETVER >= 20120307
struct PACKET_CH_MAKE_CHAR{
	int16 packetType;
	char name[NAME_LENGTH];
	uint8 slot;
	uint16 hair_color;
	uint16 hair_style;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_MAKE_CHAR, 0x970 );
#else
struct PACKET_CH_MAKE_CHAR{
	int16 packetType;
	char name[NAME_LENGTH];
	uint8 str;
	uint8 agi;
	uint8 vit;
	uint8 int_;
	uint8 dex;
	uint8 luk;
	uint8 slot;
	uint16 hair_color;
	uint16 hair_style;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_MAKE_CHAR, 0x67 );
#endif

#if PACKETVER >= 20040419
struct PACKET_CH_DELETE_CHAR{
	int16 packetType;
	uint32 CID;
	char key[50];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_DELETE_CHAR, 0x1fb );
#else
struct PACKET_CH_DELETE_CHAR{
	int16 packetType;
	uint32 CID;
	char key[40];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_DELETE_CHAR, 0x68 );
#endif

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

struct PACKET_HC_ACCEPT_ENTER{
	int16 packetType;
	int16 packetLength;
#if PACKETVER >= 20100413
	uint8 total;
	uint8 premium_start;
	uint8 premium_end;
#endif
	char extension[20];
	CHARACTER_INFO characters[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACCEPT_ENTER, 0x6b );

struct PACKET_HC_REFUSE_ENTER{
	int16 packetType;
	uint8 error;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_REFUSE_ENTER, 0x6c );

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
struct PACKET_HC_ACCEPT_MAKECHAR{
	int16 packetType;
	CHARACTER_INFO character;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACCEPT_MAKECHAR, 0xb6f );
#else
struct PACKET_HC_ACCEPT_MAKECHAR{
	int16 packetType;
	CHARACTER_INFO character;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACCEPT_MAKECHAR, 0x6d );
#endif

struct PACKET_HC_REFUSE_MAKECHAR{
	int16 packetType;
	uint8 error;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_REFUSE_MAKECHAR, 0x6e );

struct PACKET_HC_ACCEPT_DELETECHAR{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACCEPT_DELETECHAR, 0x6f );

struct PACKET_HC_REFUSE_DELETECHAR{
	int16 packetType;
	uint8 error;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_REFUSE_DELETECHAR, 0x70 );

#if PACKETVER >= 20170315
struct PACKET_HC_NOTIFY_ZONESVR{
	int16 packetType;
	uint32 CID;
	char mapname[MAP_NAME_LENGTH_EXT];
	uint32 ip;
	uint16 port;
	char domain[128];
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_NOTIFY_ZONESVR, 0xac5 );
#else
struct PACKET_HC_NOTIFY_ZONESVR{
	int16 packetType;
	uint32 CID;
	char mapname[MAP_NAME_LENGTH_EXT];
	uint32 ip;
	uint16 port;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_NOTIFY_ZONESVR, 0x81 );
#endif

struct PACKET_SC_NOTIFY_BAN{
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( SC_NOTIFY_BAN, 0x81 );

struct PACKET_PING{
	int16 packetType;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER( PING, 0x187 );

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

struct PACKET_HC_BLOCK_CHARACTER_sub{
	uint32 CID;
	char unblock_time[20];
} __attribute__((packed));

struct PACKET_HC_BLOCK_CHARACTER{
	int16 packetType;
	int16 packetLength;
	PACKET_HC_BLOCK_CHARACTER_sub characters[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_BLOCK_CHARACTER, 0x20d );

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

struct PACKET_CH_REQ_IS_VALID_CHARNAME{
	int16 packetType;
	uint32 AID;
	uint32 CID;
	char new_name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_REQ_IS_VALID_CHARNAME, 0x28d );

struct PACKET_HC_ACK_IS_VALID_CHARNAME{
	int16 packetType;
	uint16 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACK_IS_VALID_CHARNAME, 0x28e );

#if PACKETVER >= 20111101
struct PACKET_CH_REQ_CHANGE_CHARNAME{
	int16 packetType;
	uint32 CID;
	char new_name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_REQ_CHANGE_CHARNAME, 0x8fc );
#else
struct PACKET_CH_REQ_CHANGE_CHARNAME{
	int16 packetType;
	uint32 CID;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_REQ_CHANGE_CHARNAME, 0x28f );
#endif

#if PACKETVER >= 20111101
struct PACKET_HC_ACK_CHANGE_CHARNAME{
	int16 packetType;
	uint32 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARNAME, 0x8fd );
#else
struct PACKET_HC_ACK_CHANGE_CHARNAME{
	int16 packetType;
	uint16 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARNAME, 0x290 );
#endif

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

struct PACKET_CH_DELETE_CHAR3_RESERVED{
	int16 packetType;
	uint32 CID;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_DELETE_CHAR3_RESERVED, 0x827 );

struct PACKET_HC_DELETE_CHAR3_RESERVED{
	int16 packetType;
	uint32 CID;
	int32 result;
	uint32 date;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_DELETE_CHAR3_RESERVED, 0x828 );

struct PACKET_CH_DELETE_CHAR3{
	int16 packetType;
	uint32 CID;
	char birthdate[6];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_DELETE_CHAR3, 0x829 );

struct PACKET_HC_DELETE_CHAR3{
	int16 packetType;
	uint32 CID;
	int32 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_DELETE_CHAR3, 0x82a );

struct PACKET_CH_DELETE_CHAR3_CANCEL{
	int16 packetType;
	uint32 CID;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_DELETE_CHAR3_CANCEL, 0x82b );

struct PACKET_HC_DELETE_CHAR3_CANCEL{
	int16 packetType;
	uint32 CID;
	int32 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_DELETE_CHAR3_CANCEL, 0x82c );

struct PACKET_HC_ACCEPT_ENTER2{
	int16 packetType;
	int16 packetLength;
	uint8 normal;
	uint8 premium;
	uint8 billing;
	uint8 producible;
	uint8 total;
	char extension[20];
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACCEPT_ENTER2, 0x82d );

struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME_sub{
	int32 status;
	char map[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));

struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME{
	int16 packetType;
	int16 packetLength;
	struct PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME_sub maps[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_NOTIFY_ACCESSIBLE_MAPNAME, 0x840 );

struct PACKET_CH_SELECT_ACCESSIBLE_MAPNAME{
	int16 packetType;
	int8 slot;
	int8 mapnumber;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_SELECT_ACCESSIBLE_MAPNAME, 0x841 );

struct PACKET_CH_SECOND_PASSWD_ACK{
	int16 packetType;
	uint32 AID;
	char pin[4];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_SECOND_PASSWD_ACK, 0x8b8 );

struct PACKET_HC_SECOND_PASSWD_LOGIN{
	int16 packetType;
	uint32 seed;
	uint32 AID;
	uint16 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_SECOND_PASSWD_LOGIN, 0x8b9 );

struct PACKET_CH_MAKE_SECOND_PASSWD{
	int16 packetType;
	uint32 AID;
	char pin[4];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_MAKE_SECOND_PASSWD, 0x8ba );

struct PACKET_CH_EDIT_SECOND_PASSWD{
	int16 packetType;
	uint32 AID;
	char old_pin[4];
	char new_pin[4];
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_EDIT_SECOND_PASSWD, 0x8be );

struct PACKET_CH_AVAILABLE_SECOND_PASSWD{
	int16 packetType;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_AVAILABLE_SECOND_PASSWD, 0x8c5 );

struct PACKET_CH_REQ_CHANGE_CHARACTER_SLOT{
	int16 packetType;
	uint16 slot_before;
	uint16 slot_after;
	uint16 remaining;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_REQ_CHANGE_CHARACTER_SLOT, 0x8d4 );

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
struct PACKET_HC_ACK_CHANGE_CHARACTER_SLOT{
	int16 packetType;
	int16 packetLength;
	uint16 reason;
	uint16 moves;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARACTER_SLOT, 0xb70 );
#else
struct PACKET_HC_ACK_CHANGE_CHARACTER_SLOT{
	int16 packetType;
	int16 packetLength;
	uint16 reason;
	uint16 moves;
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACK_CHANGE_CHARACTER_SLOT, 0x8d5 );
#endif

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103
struct PACKET_HC_ACK_CHARINFO_PER_PAGE{
	int16 packetType;
	int16 packetLength;
	CHARACTER_INFO characters[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACK_CHARINFO_PER_PAGE, 0xb72 );
#else
struct PACKET_HC_ACK_CHARINFO_PER_PAGE{
	int16 packetType;
	int16 packetLength;
	CHARACTER_INFO characters[];
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_ACK_CHARINFO_PER_PAGE, 0x99d );
#endif

struct PACKET_HC_CHARLIST_NOTIFY{
	int16 packetType;
	uint32 total;
#if PACKETVER_RE_NUM >= 20151001 && PACKETVER_RE_NUM < 20180103
	uint32 slots;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER( HC_CHARLIST_NOTIFY, 0x9a0 );

struct PACKET_CH_CHARLIST_REQ{
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER( CH_CHARLIST_REQ, 0x9a1 );

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

template <typename sessiontype> class PacketDatabase{
private:
	struct s_packet_info{
		bool fixed;
		int16 size;
		std::function<bool ( int32 fd, sessiontype& sd )> func;
	};

	std::unordered_map<int16, s_packet_info> infos;

public:
	void add( int16 packetType, bool fixed, int16 size, std::function<bool ( int32 fd, sessiontype& sd )> func ){
		if( fixed ){
			if( size < 2 ){
				ShowError( "Definition for packet 0x%04x is invalid. Minimum size for a fixed length packet is 2 bytes.\n", packetType );
				return;
			}
		}else{
			if( size < 4 ){
				ShowError( "Definition for packet 0x%04x is invalid. Minimum size for a dynamic length packet is 2 bytes.\n", packetType );
				return;
			}
		}

		if( rathena::util::umap_find( this->infos, packetType ) != nullptr ){
			ShowError( "Definition for packet 0x%04x is done at least twice.\n", packetType );
			return;
		}

		s_packet_info& info = infos[packetType];

		info.fixed = fixed;
		info.size = size;
		info.func = func;
	}

	bool handle( int32 fd, sessiontype& sd ){
		int16 remaining =  static_cast<int16>( RFIFOREST( fd ) );

		if( remaining < 2 ){
			ShowError( "Did not receive enough bytes to process a packet\n" );
			set_eof( fd );
			return false;
		}

		PACKET* p = (PACKET*)RFIFOP( fd, 0 );

		s_packet_info* info = rathena::util::umap_find( this->infos, p->packetType );

		if( info == nullptr ){
			ShowError( "Received unknown packet 0x%04x\n", p->packetType );
			set_eof( fd );
			return false;
		}

		if( info->fixed ){
			if( remaining < info->size ){
				ShowError( "Invalid size %hd for packet 0x%04x with fixed size of %hd\n", remaining, p->packetType, info->size );
				set_eof( fd );
				return false;
			}

			bool ret = info->func( fd, sd );

			RFIFOSKIP( fd, info->size );

			return ret;
		}else{
			if( remaining < info->size ){
				ShowError( "Invalid size %hd for packet 0x%04x with dynamic minimum size of %hd\n", remaining, p->packetType, info->size );
				set_eof( fd );
				return false;
			}

			if( remaining < p->packetLength ){
				ShowError( "Invalid size %hd for packet 0x%04x with dynamic size of %hd\n", remaining, p->packetType, p->packetLength );
				set_eof( fd );
				return false;
			}

			bool ret = info->func( fd, sd );

			RFIFOSKIP( fd, p->packetLength );

			return ret;
		}
	}
};

#endif /* PACKETS_HPP */
