// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Copyright (c) Hercules Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "clif.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unordered_set>

#include <common/cbasetypes.hpp>
#include <common/conf.hpp>
#include <common/ers.hpp>
#include <common/grfio.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "achievement.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "cashshop.hpp"
#include "channel.hpp"
#include "chat.hpp"
#include "chrif.hpp"
#include "clan.hpp"
#include "clif.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "mail.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"
#include "quest.hpp"
#include "script.hpp"
#include "skill.hpp"
#include "status.hpp"
#include "storage.hpp"
#include "trade.hpp"
#include "unit.hpp"
#include "vending.hpp"

using namespace rathena;

static inline uint32 client_tick( t_tick tick ){
	return (uint32)tick;
}

#if PACKETVER >= 20170830
static inline int64 client_exp(t_exp exp) {
	return (int64)u64min(exp, MAX_EXP);
}
#else
static inline int32 client_exp(t_exp exp) {
	return (int32)u64min(exp, MAX_EXP);
}
#endif

/* for clif_clearunit_delayed */
static struct eri *delay_clearunit_ers;

struct s_packet_db packet_db[MAX_PACKET_DB + 1];
unsigned long color_table[COLOR_MAX];

#include "clif_obfuscation.hpp"
static bool clif_session_isValid(map_session_data *sd);
static void clif_loadConfirm( map_session_data *sd );
static void clif_favorite_item( map_session_data& sd, uint16 index );

#if PACKETVER >= 20150513
enum mail_type {
	MAIL_TYPE_TEXT = 0x0,
	MAIL_TYPE_ZENY = 0x2,
	MAIL_TYPE_ITEM = 0x4,
	MAIL_TYPE_NPC = 0x8
};
#endif

enum e_inventory_type{
	INVTYPE_INVENTORY = 0,
	INVTYPE_CART = 1,
	INVTYPE_STORAGE = 2,
	INVTYPE_GUILD_STORAGE = 3,
};

/** Converts item type to display it on client if necessary.
* @param nameid: Item ID
* @return item type. For IT_PETEGG will be displayed as IT_ARMOR. If Shadow Weapon of IT_SHADOWGEAR as IT_WEAPON and else as IT_ARMOR
*/
static inline int itemtype(t_itemid nameid) {
	struct item_data* id = itemdb_search(nameid); //Use itemdb_search, so non-existance item will use dummy data and won't crash the server. bugreport:8468
	int type = id->type;
	if( type == IT_SHADOWGEAR ) {
		if( id->equip&EQP_SHADOW_WEAPON )
			return IT_WEAPON;
		else
			return IT_ARMOR;
	}
	return ( type == IT_PETEGG ) ? IT_ARMOR : type;
}

// TODO: doc
static inline uint16 client_index( uint16 server_index ){
	return server_index + 2;
}

static inline uint16 server_index( uint16 client_index ){
	return client_index - 2;
}

static inline uint16 client_storage_index( uint16 server_index ){
	return server_index + 1;
}

static inline uint16 server_storage_index( uint16 client_index ){
	return client_index - 1;
}

static inline uint32 disguised_bl_id( uint32 bl_id ){
	// Casting to prevent a compiler warning
	return -((int32)bl_id);
}

#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
static inline uint32 client_nameid( t_itemid server_nameid ){
	t_itemid view = itemdb_viewid( server_nameid );

	if( view > 0 ){
		return view;
	}else{
		return server_nameid;
	}
}
#else
static inline uint16 client_nameid( t_itemid server_nameid ){
	t_itemid view = itemdb_viewid( server_nameid );

	if( view > 0 ){
		if( view > UINT16_MAX ){
			return (uint16)UNKNOWN_ITEM_ID;
		}

		return (uint16)view;
	}else{
		if( server_nameid > UINT16_MAX ){
			return (uint16)UNKNOWN_ITEM_ID;
		}

		return (uint16)server_nameid;
	}
}
#endif

static inline void WBUFPOS(uint8* p, unsigned short pos, short x, short y, unsigned char dir) {
	p += pos;
	p[0] = (uint8)(x>>2);
	p[1] = (uint8)((x<<6) | ((y>>4)&0x3f));
	p[2] = (uint8)((y<<4) | (dir&0xf));
}


// client-side: x0+=sx0*0.0625-0.5 and y0+=sy0*0.0625-0.5
static inline void WBUFPOS2(uint8* p, unsigned short pos, short x0, short y0, short x1, short y1, uint8 sx0, uint8 sy0) {
	p += pos;
	p[0] = (uint8)(x0>>2);
	p[1] = (uint8)((x0<<6) | ((y0>>4)&0x3f));
	p[2] = (uint8)((y0<<4) | ((x1>>6)&0x0f));
	p[3] = (uint8)((x1<<2) | ((y1>>8)&0x03));
	p[4] = (uint8)y1;
	p[5] = (uint8)((sx0<<4) | (sy0&0x0f));
}


static inline void WFIFOPOS(int fd, unsigned short pos, short x, short y, unsigned char dir) {
	WBUFPOS(WFIFOP(fd,pos), 0, x, y, dir);
}

static inline void RBUFPOS(const uint8* p, unsigned short pos, short* x, short* y, unsigned char* dir) {
	p += pos;

	if( x ) {
		x[0] = ( ( p[0] & 0xff ) << 2 ) | ( p[1] >> 6 );
	}

	if( y ) {
		y[0] = ( ( p[1] & 0x3f ) << 4 ) | ( p[2] >> 4 );
	}

	if( dir ) {
		dir[0] = ( p[2] & 0x0f );
	}
}


static inline void RBUFPOS2(const uint8* p, unsigned short pos, short* x0, short* y0, short* x1, short* y1, unsigned char* sx0, unsigned char* sy0) {
	p += pos;

	if( x0 ) {
		x0[0] = ( ( p[0] & 0xff ) << 2 ) | ( p[1] >> 6 );
	}

	if( y0 ) {
		y0[0] = ( ( p[1] & 0x3f ) << 4 ) | ( p[2] >> 4 );
	}

	if( x1 ) {
		x1[0] = ( ( p[2] & 0x0f ) << 6 ) | ( p[3] >> 2 );
	}

	if( y1 ) {
		y1[0] = ( ( p[3] & 0x03 ) << 8 ) | ( p[4] >> 0 );
	}

	if( sx0 ) {
		sx0[0] = ( p[5] & 0xf0 ) >> 4;
	}

	if( sy0 ) {
		sy0[0] = ( p[5] & 0x0f ) >> 0;
	}
}


static inline void RFIFOPOS(int fd, unsigned short pos, short* x, short* y, unsigned char* dir) {
	RBUFPOS(RFIFOP(fd,pos), 0, x, y, dir);
}


static inline void RFIFOPOS2(int fd, unsigned short pos, short* x0, short* y0, short* x1, short* y1, unsigned char* sx0, unsigned char* sy0) {
	RBUFPOS2(WFIFOP(fd,pos), 0, x0, y0, x1, y1, sx0, sy0);
}

//To idenfity disguised characters.
static inline bool disguised(struct block_list* bl) {
	return (bool)( bl->type == BL_PC && ((TBL_PC*)bl)->disguise );
}


//Guarantees that the given string does not exceeds the allowed size, as well as making sure it's null terminated. [Skotlex]
static inline unsigned int mes_len_check(char* mes, unsigned int len, unsigned int max) {
	if( len > max )
		len = max;

	mes[len-1] = '\0';

	return len;
}


static char map_ip_str[128];
static uint32 map_ip;
static uint32 bind_ip = INADDR_ANY;
static uint16 map_port = 5121;
static bool clif_ally_only = false;
int map_fd;

static int clif_parse (int fd);

/*==========================================
 * Ip setting of map-server
 *------------------------------------------*/
int clif_setip(const char* ip) {
	char ip_str[16];
	map_ip = host2ip(ip);
	if (!map_ip) {
		ShowWarning("Failed to Resolve Map Server Address! (%s)\n", ip);
		return 0;
	}

	safestrncpy(map_ip_str, ip, sizeof(map_ip_str));
	ShowInfo("Map Server IP Address : '" CL_WHITE "%s" CL_RESET "' -> '" CL_WHITE "%s" CL_RESET "'.\n", ip, ip2str(map_ip, ip_str));
	return 1;
}

void clif_setbindip(const char* ip)
{
	bind_ip = host2ip(ip);
	if (bind_ip) {
		char ip_str[16];
		ShowInfo("Map Server Bind IP Address : '" CL_WHITE "%s" CL_RESET "' -> '" CL_WHITE "%s" CL_RESET "'.\n", ip, ip2str(bind_ip, ip_str));
	} else {
		ShowWarning("Failed to Resolve Map Server Address! (%s)\n", ip);
	}
}

/*==========================================
 * Sets map port to 'port'
 * is run from map.cpp upon loading map server configuration
 *------------------------------------------*/
void clif_setport(uint16 port)
{
	map_port = port;
}

/*==========================================
 * Returns map server IP
 *------------------------------------------*/
uint32 clif_getip(void)
{
	return map_ip;
}

//Refreshes map_server ip, returns the new ip if the ip changed, otherwise it returns 0.
uint32 clif_refresh_ip(void)
{
	uint32 new_ip;

	new_ip = host2ip(map_ip_str);
	if (new_ip && new_ip != map_ip) {
		map_ip = new_ip;
		ShowInfo("Updating IP resolution of [%s].\n", map_ip_str);
		return map_ip;
	}
	return 0;
}

/*==========================================
 * Returns map port which is set by clif_setport()
 *------------------------------------------*/
uint16 clif_getport(void)
{
	return map_port;
}

#if PACKETVER >= 20071106
static inline unsigned char clif_bl_type(struct block_list *bl, bool walking) {
	switch (bl->type) {
	case BL_PC:    return (disguised(bl) && !pcdb_checkid(status_get_viewdata(bl)->class_))? 0x1:0x0; //PC_TYPE
	case BL_ITEM:  return 0x2; //ITEM_TYPE
	case BL_SKILL: return 0x3; //SKILL_TYPE
	case BL_CHAT:  return 0x4; //UNKNOWN_TYPE
	case BL_MOB:
		if( pcdb_checkid( status_get_viewdata( bl )->class_ ) ){
			return 0x0; //PC_TYPE
		}else{
			switch( ( (mob_data*)bl )->special_state.ai ){
				case AI_ABR:
					return 0xd; //NPC_ABR_TYPE
				case AI_BIONIC:
					return 0xe; //NPC_BIONIC_TYPE
				default:
					return 0x5; //NPC_MOB_TYPE
			}
		}
	case BL_NPC:
// From 2017-07-26 on NPC type units can also use player sprites.
// There is one exception and this is if they are walking.
// Since walking NPCs are not supported on official servers, the client does not know how to handle it.
#if PACKETVER >= 20170726
		if (pcdb_checkid( status_get_viewdata( bl )->class_ ) && walking)
			return 0x0;
		else if (mobdb_checkid( status_get_viewdata( bl )->class_ ))	// FIXME: categorize NPCs able to walk
			return 0xC;
		else
			return 0x6;
#else
		return pcdb_checkid(status_get_viewdata(bl)->class_) ? 0x0 : 0x6; //NPC_EVT_TYPE
#endif
	case BL_PET:   return pcdb_checkid(status_get_viewdata(bl)->class_)?0x0:0x7; //NPC_PET_TYPE
	case BL_HOM:   return 0x8; //NPC_HOM_TYPE
	case BL_MER:   return 0x9; //NPC_MERSOL_TYPE
	case BL_ELEM:  return 0xa; //NPC_ELEMENTAL_TYPE
	default:       return 0x1; //NPC_TYPE
	}
}
#endif

static bool clif_session_isValid(map_session_data *sd) {
	return ( sd != nullptr && session_isActive(sd->fd) );
}

/*==========================================
 * sub process of clif_send
 * Called from a map_foreachinallarea (grabs all players in specific area and subjects them to this function)
 * In order to send area-wise packets, such as:
 * - AREA : everyone nearby your area
 * - AREA_WOSC (AREA WITHOUT SAME CHAT) : Not run for people in the same chat as yours
 * - AREA_WOC (AREA WITHOUT CHAT) : Not run for people inside a chat
 * - AREA_WOS (AREA WITHOUT SELF) : Not run for self
 * - AREA_CHAT_WOC : Everyone in the area of your chat without a chat
 *------------------------------------------*/
static int clif_send_sub(struct block_list *bl, va_list ap)
{
	struct block_list *src_bl;
	map_session_data *sd;
	unsigned char *buf;
	int len, type, fd;

	nullpo_ret(bl);
	nullpo_ret(sd = (map_session_data *)bl);

	// Don't send to disconnected clients.
	if( !session_isActive( fd = sd->fd ) ){
		return 0;
	}

	buf = va_arg(ap,unsigned char*);
	len = va_arg(ap,int);
	nullpo_ret(src_bl = va_arg(ap,struct block_list*));
	type = va_arg(ap,int);

	switch(type) {
	case AREA_WOS:
		if (bl == src_bl)
			return 0;
	break;
	case AREA_WOC:
		if (sd->chatID || bl == src_bl)
			return 0;
	break;
	case AREA_WOSC:
	{
		if(src_bl->type == BL_PC) {
			map_session_data *ssd = (map_session_data *)src_bl;
			if (ssd && sd->chatID && (sd->chatID == ssd->chatID))
			return 0;
		}
		else if(src_bl->type == BL_NPC) {
			struct npc_data *nd = (struct npc_data *)src_bl;
			if (nd && sd->chatID && (sd->chatID == nd->chat_id))
			return 0;
		}
	}
	break;
	}

	if( src_bl->type == BL_NPC && npc_is_hidden_dynamicnpc( *( (struct npc_data*)src_bl ), *sd ) ){
		// Do not send anything
		return 0;
	}

	/* unless visible, hold it here */
	if (!battle_config.update_enemy_position && clif_ally_only && !sd->special_state.intravision &&
		!sd->sc.getSCE(SC_INTRAVISION) && battle_check_target(src_bl,&sd->bl,BCT_ENEMY) > 0)
		return 0;

	WFIFOHEAD(fd, len);
	if (WFIFOP(fd,0) == buf) {
		ShowError("WARNING: Invalid use of clif_send function\n");
		ShowError("         Packet x%4x use a WFIFO of a player instead of to use a buffer.\n", WBUFW(buf,0));
		ShowError("         Please correct your code.\n");
		// don't send to not move the pointer of the packet for next sessions in the loop
		//WFIFOSET(fd,0);//## TODO is this ok?
		//NO. It is not ok. There is the chance WFIFOSET actually sends the buffer data, and shifts elements around, which will corrupt the buffer.
		return 0;
	}

	memcpy(WFIFOP(fd,0), buf, len);
	WFIFOSET(fd,len);

	return 0;
}

/*==========================================
 * Packet Delegation (called on all packets that require data to be sent to more than one client)
 * functions that are sent solely to one use whose ID it posses use WFIFOSET
 *------------------------------------------*/
int clif_send(const void* buf, int len, struct block_list* bl, enum send_target type)
{
	int i;
	map_session_data *sd, *tsd;
	struct party_data *p = nullptr;
	std::shared_ptr<s_battleground_data> bg;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0, fd;
	struct s_mapiterator* iter;

	if( type != ALL_CLIENT )
		nullpo_ret(bl);

	sd = BL_CAST(BL_PC, bl);

	switch(type) {

	case ALL_CLIENT: //All player clients.
		iter = mapit_getallusers();
		while( ( tsd = (map_session_data*)mapit_next( iter ) ) != nullptr ){
			if( session_isActive( fd = tsd->fd ) ){
				WFIFOHEAD( fd, len );
				memcpy( WFIFOP( fd, 0 ), buf, len );
				WFIFOSET( fd, len );
			}
		}
		mapit_free(iter);
		break;

	case ALL_SAMEMAP: //All players on the same map
		iter = mapit_getallusers();
		while( ( tsd = (map_session_data*)mapit_next( iter ) ) != nullptr ){
			if( bl->m == tsd->bl.m && session_isActive( fd = tsd->fd ) ){
				WFIFOHEAD( fd, len );
				memcpy( WFIFOP( fd, 0 ), buf, len );
				WFIFOSET( fd, len );
			}
		}
		mapit_free(iter);
		break;

	case AREA:
	case AREA_WOSC:
		if (sd && bl->prev == nullptr) //Otherwise source misses the packet.[Skotlex]
			clif_send (buf, len, bl, SELF);
		[[fallthrough]];
	case AREA_WOC:
	case AREA_WOS:
		map_foreachinallarea(clif_send_sub, bl->m, bl->x-AREA_SIZE, bl->y-AREA_SIZE, bl->x+AREA_SIZE, bl->y+AREA_SIZE,
			BL_PC, buf, len, bl, type);
		break;
	case AREA_CHAT_WOC:
		map_foreachinallarea(clif_send_sub, bl->m, bl->x-(AREA_SIZE-5), bl->y-(AREA_SIZE-5),
			bl->x+(AREA_SIZE-5), bl->y+(AREA_SIZE-5), BL_PC, buf, len, bl, AREA_WOC);
		break;

	case CHAT:
	case CHAT_WOS:
		{
			struct chat_data *cd;
			if (sd) {
				cd = (struct chat_data*)map_id2bl(sd->chatID);
			} else if (bl->type == BL_CHAT) {
				cd = (struct chat_data*)bl;
			} else break;
			if (cd == nullptr)
				break;
			for(i = 0; i < cd->users; i++) {
				if (type == CHAT_WOS && cd->usersd[i] == sd)
					continue;
				if( session_isActive( fd = cd->usersd[i]->fd ) ){
					WFIFOHEAD(fd,len);
					memcpy(WFIFOP(fd,0), buf, len);
					WFIFOSET(fd,len);
				}
			}
		}
		break;

	case PARTY_AREA:
	case PARTY_AREA_WOS:
		x0 = bl->x - AREA_SIZE;
		y0 = bl->y - AREA_SIZE;
		x1 = bl->x + AREA_SIZE;
		y1 = bl->y + AREA_SIZE;
		[[fallthrough]];
	case PARTY:
	case PARTY_WOS:
	case PARTY_SAMEMAP:
	case PARTY_SAMEMAP_WOS:
		if (sd && sd->status.party_id)
			p = party_search(sd->status.party_id);

		if (p) {
			for(i=0;i<MAX_PARTY;i++){
				if( (sd = p->data[i].sd) == nullptr )
					continue;

				if( !session_isActive( fd = sd->fd ) )
					continue;

				if( sd->bl.id == bl->id && (type == PARTY_WOS || type == PARTY_SAMEMAP_WOS || type == PARTY_AREA_WOS) )
					continue;

				if( type != PARTY && type != PARTY_WOS && bl->m != sd->bl.m )
					continue;

				if( (type == PARTY_AREA || type == PARTY_AREA_WOS) && (sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1) )
					continue;

				WFIFOHEAD(fd, len);
				memcpy(WFIFOP(fd, 0), buf, len);
				WFIFOSET(fd, len);
			}
			if (!enable_spy) //Skip unnecessary parsing. [Skotlex]
				break;

			iter = mapit_getallusers();
			while( ( tsd = (map_session_data*)mapit_next( iter ) ) != nullptr ){
				if( tsd->partyspy == p->party.party_id && session_isActive( fd = tsd->fd ) ){
					WFIFOHEAD( fd, len );
					memcpy( WFIFOP( fd, 0 ), buf, len );
					WFIFOSET( tsd->fd, len );
				}
			}
			mapit_free(iter);
		}
		break;

	case DUEL:
	case DUEL_WOS:
		if (!sd || !sd->duel_group) break; //Invalid usage.

		iter = mapit_getallusers();
		while( ( tsd = (map_session_data*)mapit_next( iter ) ) != nullptr ){
			if( type == DUEL_WOS && bl->id == tsd->bl.id )
				continue;
			if( sd->duel_group == tsd->duel_group && session_isActive( fd = tsd->fd ) ){
				WFIFOHEAD( fd, len );
				memcpy( WFIFOP( fd, 0 ), buf, len );
				WFIFOSET( fd, len );
			}
		}
		mapit_free(iter);
		break;

	case SELF:
		if( clif_session_isValid(sd) ){
			fd = sd->fd;
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
		}
		break;

	// New definitions for guilds [Valaris] - Cleaned up and reorganized by [Skotlex]
	case GUILD_AREA:
	case GUILD_AREA_WOS:
		x0 = bl->x - AREA_SIZE;
		y0 = bl->y - AREA_SIZE;
		x1 = bl->x + AREA_SIZE;
		y1 = bl->y + AREA_SIZE;
		[[fallthrough]];
	case GUILD_SAMEMAP:
	case GUILD_SAMEMAP_WOS:
	case GUILD:
	case GUILD_WOS:
	case GUILD_NOBG: {
		if (!sd || !sd->status.guild_id || !sd->guild)
			break;

		const auto &g = sd->guild->guild;
		for(i = 0; i < g.max_member; i++) {
			if( (sd = g.member[i].sd) != nullptr ){
				if( !session_isActive( fd = sd->fd ) )
					continue;

				if( type == GUILD_NOBG && sd->bg_id )
					continue;

				if( sd->bl.id == bl->id && (type == GUILD_WOS || type == GUILD_SAMEMAP_WOS || type == GUILD_AREA_WOS) )
					continue;

				if( type != GUILD && type != GUILD_NOBG && type != GUILD_WOS && sd->bl.m != bl->m )
					continue;

				if( (type == GUILD_AREA || type == GUILD_AREA_WOS) && (sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1) )
					continue;

				WFIFOHEAD(fd,len);
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
			}
		}
		if (!enable_spy) //Skip unnecessary parsing. [Skotlex]
			break;

		iter = mapit_getallusers();
		while( ( tsd = (map_session_data*)mapit_next( iter ) ) != nullptr ){
			if( tsd->guildspy == g.guild_id && session_isActive( fd = tsd->fd ) ){
				WFIFOHEAD( fd, len );
				memcpy( WFIFOP( fd, 0 ), buf, len );
				WFIFOSET( fd, len );
			}
		}
		mapit_free(iter);
		break;
	}
	case BG_AREA:
	case BG_AREA_WOS:
		x0 = bl->x - AREA_SIZE;
		y0 = bl->y - AREA_SIZE;
		x1 = bl->x + AREA_SIZE;
		y1 = bl->y + AREA_SIZE;
		[[fallthrough]];
	case BG_SAMEMAP:
	case BG_SAMEMAP_WOS:
	case BG:
	case BG_WOS:
		if( sd && sd->bg_id > 0 && (bg = util::umap_find(bg_team_db, sd->bg_id)))
		{
			for (const auto &member : bg->members) {
				if( ( sd = member.sd ) == nullptr || !session_isActive( fd = sd->fd ) )
					continue;
				if(sd->bl.id == bl->id && (type == BG_WOS || type == BG_SAMEMAP_WOS || type == BG_AREA_WOS) )
					continue;
				if( type != BG && type != BG_WOS && sd->bl.m != bl->m )
					continue;
				if( (type == BG_AREA || type == BG_AREA_WOS) && (sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1) )
					continue;
				WFIFOHEAD(fd,len);
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
			}
		}
		break;
	case CLAN:
		if( sd && sd->clan ){
			struct clan* clan = sd->clan;

			for( i = 0; i < clan->max_member; i++ ){
				if( ( sd = clan->members[i] ) == nullptr || !session_isActive( fd = sd->fd ) ){
					continue;
				}

				WFIFOHEAD(fd,len);
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
			}

			if (!enable_spy) //Skip unnecessary parsing. [Skotlex]
				break;

			iter = mapit_getallusers();
			while( ( tsd = (map_session_data*)mapit_next( iter ) ) != nullptr ){
				if( tsd->clanspy == clan->id && session_isActive( fd = tsd->fd ) ){
					WFIFOHEAD(fd, len);
					memcpy(WFIFOP(fd, 0), buf, len);
					WFIFOSET(fd, len);
				}
			}
			mapit_free(iter);
		}
		break;

	default:
		ShowError("clif_send: Unrecognized type %d\n",type);
		return -1;
	}

	return 0;
}


/// Notifies the client, that it's connection attempt was accepted.
/// 0073 <start time>.L <position>.3B <x size>.B <y size>.B (ZC_ACCEPT_ENTER)
/// 02eb <start time>.L <position>.3B <x size>.B <y size>.B <font>.W (ZC_ACCEPT_ENTER2)
/// 0a18 <start time>.L <position>.3B <x size>.B <y size>.B <font>.W <sex>.B (ZC_ACCEPT_ENTER3)
void clif_authok(map_session_data *sd)
{
	PACKET_ZC_ACCEPT_ENTER packet{};

	packet.packetType = HEADER_ZC_ACCEPT_ENTER;
	packet.startTime = client_tick(gettick());
	WBUFPOS(packet.posDir, 0, sd->bl.x, sd->bl.y, sd->ud.dir);
	packet.xSize = 5; // ignored
	packet.ySize = 5; // ignored
#if PACKETVER >= 20080102
	packet.font = sd->status.font;
#endif
#if PACKETVER >= 20141016 && PACKETVER < 20160330
	packet.sex = sd->status.sex;
#endif

	clif_send(&packet, sizeof(packet), &sd->bl, SELF);
}


/// Notifies the client, that it's connection attempt was refused.
/// 0074 <error code>.B (ZC_REFUSE_ENTER)
/// error code:
///     0 = client type mismatch
///     1 = ID mismatch
///     2 = mobile - out of available time
///     3 = mobile - already logged in
///     4 = mobile - waiting state
void clif_authrefuse(int fd, uint8 error_code)
{
	PACKET_ZC_REFUSE_ENTER packet{};

	packet.packetType = HEADER_ZC_REFUSE_ENTER;
	packet.errorCode = error_code;

	socket_send<PACKET_ZC_REFUSE_ENTER>(fd, packet);
}


/// Notifies the client of a ban or forced disconnect.
/// 0081 <error code>.B (SC_NOTIFY_BAN)
/// error code:
///     0 = BAN_UNFAIR
///     1 = server closed -> MsgStringTable[4]
///     2 = ID already logged in -> MsgStringTable[5]
///     3 = timeout/too much lag -> MsgStringTable[241]
///     4 = server full -> MsgStringTable[264]
///     5 = underaged -> MsgStringTable[305]
///     8 = Server sill recognizes last connection -> MsgStringTable[441]
///     9 = too many connections from this ip -> MsgStringTable[529]
///     10 = out of available time paid for -> MsgStringTable[530]
///     11 = BAN_PAY_SUSPEND
///     12 = BAN_PAY_CHANGE
///     13 = BAN_PAY_WRONGIP
///     14 = BAN_PAY_PNGAMEROOM
///     15 = disconnected by a GM -> if( servicetype == taiwan ) MsgStringTable[579]
///     16 = BAN_JAPAN_REFUSE1
///     17 = BAN_JAPAN_REFUSE2
///     18 = BAN_INFORMATION_REMAINED_ANOTHER_ACCOUNT
///     100 = BAN_PC_IP_UNFAIR
///     101 = BAN_PC_IP_COUNT_ALL
///     102 = BAN_PC_IP_COUNT
///     103 = BAN_GRAVITY_MEM_AGREE
///     104 = BAN_GAME_MEM_AGREE
///     105 = BAN_HAN_VALID
///     106 = BAN_PC_IP_LIMIT_ACCESS
///     107 = BAN_OVER_CHARACTER_LIST
///     108 = BAN_IP_BLOCK
///     109 = BAN_INVALID_PWD_CNT
///     110 = BAN_NOT_ALLOWED_JOBCLASS
///     ? = disconnected -> MsgStringTable[3]
void clif_authfail_fd(int fd, int type)
{
	if (!session_isValid(fd) || session[fd]->func_parse != clif_parse) //clif_authfail should only be invoked on players!
		return;
	
	PACKET_SC_NOTIFY_BAN packet{};

	packet.packetType = HEADER_SC_NOTIFY_BAN;
	packet.errorCode = static_cast<uint8>(type);

	socket_send<PACKET_SC_NOTIFY_BAN>(fd, packet);

	set_eof(fd);
}


/// Notifies the client, whether it can disconnect and change servers.
/// 00b3 <type>.B (ZC_RESTART_ACK)
/// type:
///     1 = disconnect, char-select
///     ? = nothing
void clif_charselectok(int id, uint8 ok)
{
	map_session_data* sd = map_id2sd(id);

	if (sd == nullptr)
		return;

	PACKET_ZC_RESTART_ACK packet{};

	packet.packetType = HEADER_ZC_RESTART_ACK;
	packet.type = ok;

	clif_send( &packet, sizeof( packet ), &sd->bl, SELF );
}

/// Makes an item appear on the ground.
/// 009E <id>.L <name id>.W <identified>.B <x>.W <y>.W <subX>.B <subY>.B <amount>.W (ZC_ITEM_FALL_ENTRY)
/// 084B <id>.L <name id>.W <type>.W <identified>.B <x>.W <y>.W <subX>.B <subY>.B <amount>.W (ZC_ITEM_FALL_ENTRY4)
/// 0ADD <id>.L <name id>.W <type>.W <identified>.B <x>.W <y>.W <subX>.B <subY>.B <amount>.W <show drop effect>.B <drop effect mode>.W (ZC_ITEM_FALL_ENTRY5)
void clif_dropflooritem( struct flooritem_data* fitem, bool canShowEffect ){
	nullpo_retv(fitem);

	if( fitem->item.nameid == 0 ){
		return;
	}

	struct packet_dropflooritem p;

	p.PacketType = dropflooritemType;
	p.ITAID = fitem->bl.id;
	p.ITID = client_nameid( fitem->item.nameid );
#if PACKETVER >= 20130000 /* not sure date */
	p.type = itemtype( fitem->item.nameid );
#endif
	p.IsIdentified = fitem->item.identify ? 1 : 0;
	p.xPos = fitem->bl.x;
	p.yPos = fitem->bl.y;
	p.subX = fitem->subx;
	p.subY = fitem->suby;
	p.count = fitem->item.amount;
#if defined(PACKETVER_ZERO) || PACKETVER >= 20180418
	if( canShowEffect ){
		uint8 dropEffect = itemdb_dropeffect( fitem->item.nameid );

		if( dropEffect > 0 ){
			p.showdropeffect = 1;
			p.dropeffectmode = dropEffect - 1;
		}else if (battle_config.rndopt_drop_pillar != 0){
			uint8 optionCount = 0;

			for (uint8 i = 0; i < MAX_ITEM_RDM_OPT; i++) {
				if (fitem->item.option[i].id != 0) {
					optionCount++;
				}
			}

			if (optionCount > 0) {
				p.showdropeffect = 1;
				if (optionCount == 1)
					p.dropeffectmode = DROPEFFECT_BLUE_PILLAR - 1;
				else if (optionCount == 2)
					p.dropeffectmode = DROPEFFECT_YELLOW_PILLAR - 1;
				else
					p.dropeffectmode = DROPEFFECT_PURPLE_PILLAR - 1;
			} else {
				p.showdropeffect = 0;
				p.dropeffectmode = DROPEFFECT_NONE;
			}
		} else {
			p.showdropeffect = 0;
			p.dropeffectmode = DROPEFFECT_NONE;
		}
	}else{
		p.showdropeffect = 0;
		p.dropeffectmode = DROPEFFECT_NONE;
	}
#endif
	clif_send( &p, sizeof(p), &fitem->bl, AREA );
}



/// Makes an item disappear from the ground.
/// 00a1 <id>.L (ZC_ITEM_DISAPPEAR)
void clif_clearflooritem( flooritem_data& fitem, map_session_data* tsd ){
	PACKET_ZC_ITEM_DISAPPEAR packet{};

	packet.packetType = HEADER_ZC_ITEM_DISAPPEAR;
	packet.itemAid = fitem.bl.id;

	if( tsd == nullptr ){
		clif_send(&packet, sizeof(PACKET_ZC_ITEM_DISAPPEAR), &fitem.bl, AREA);
	} else {
		clif_send(&packet, sizeof(PACKET_ZC_ITEM_DISAPPEAR), &tsd->bl, SELF );
	}
}


/// Makes a unit (char, npc, mob, homun) disappear to one client.
/// 0080 <id>.L <type>.B (ZC_NOTIFY_VANISH)
/// type:
///     0 = out of sight
///     1 = died
///     2 = logged out
///     3 = teleport
///     4 = trickdead
void clif_clearunit_single( uint32 GID, clr_type type, map_session_data& tsd ){
	PACKET_ZC_NOTIFY_VANISH packet{};

	packet.packetType = HEADER_ZC_NOTIFY_VANISH;
	packet.gid = GID;
	packet.type = static_cast<decltype(packet.type)>(type);

	clif_send( &packet, sizeof( packet ), &tsd.bl, SELF );
}

/// Makes a unit (char, npc, mob, homun) disappear to all clients in area.
/// 0080 <id>.L <type>.B (ZC_NOTIFY_VANISH)
/// type:
///     0 = out of sight
///     1 = died
///     2 = logged out
///     3 = teleport
///     4 = trickdead
void clif_clearunit_area( block_list& bl, clr_type type ){
	PACKET_ZC_NOTIFY_VANISH packet{};

	packet.packetType = HEADER_ZC_NOTIFY_VANISH;
	packet.gid = bl.id;
	packet.type = static_cast<uint8>(type);

	clif_send(&packet, sizeof(PACKET_ZC_NOTIFY_VANISH), &bl, type == CLR_DEAD ? AREA : AREA_WOS);

	if(disguised(&bl)) {
		packet.gid = disguised_bl_id( bl.id );
		clif_send(&packet, sizeof(PACKET_ZC_NOTIFY_VANISH), &bl, SELF);
	}
}


/// Used to make monsters with player-sprites disappear after dying
/// like normal monsters, because the client does not remove those
/// automatically.
static TIMER_FUNC(clif_clearunit_delayed_sub){
	struct block_list *bl = (struct block_list *)data;

	if( bl != nullptr ){
		clif_clearunit_area( *bl, (clr_type)id );
		ers_free( delay_clearunit_ers, bl );
	}

	return 0;
}
void clif_clearunit_delayed(struct block_list* bl, clr_type type, t_tick tick)
{
	struct block_list *tbl = ers_alloc(delay_clearunit_ers, struct block_list);
	tbl->next = nullptr;
	tbl->prev = nullptr;
	tbl->id = bl->id;
	tbl->m = bl->m;
	tbl->x = bl->x;
	tbl->y = bl->y;
	tbl->type = BL_NUL;
	add_timer(tick, clif_clearunit_delayed_sub, (int)type, (intptr_t)tbl);
}

void clif_get_weapon_view(map_session_data* sd, t_itemid *rhand, t_itemid *lhand)
{
	if(sd->sc.option&OPTION_COSTUME)
	{
		*rhand = *lhand = 0;
		return;
	}

#if PACKETVER < 4
	*rhand = sd->status.weapon;
	*lhand = sd->status.shield;
#else
	if (sd->equip_index[EQI_HAND_R] >= 0 &&
		sd->inventory_data[sd->equip_index[EQI_HAND_R]])
	{
		struct item_data* id = sd->inventory_data[sd->equip_index[EQI_HAND_R]];
		if (id->view_id > 0)
			*rhand = id->view_id;
		else
			*rhand = id->nameid;
	} else
		*rhand = 0;

	if (sd->equip_index[EQI_HAND_L] >= 0 &&
		sd->equip_index[EQI_HAND_L] != sd->equip_index[EQI_HAND_R] &&
		sd->inventory_data[sd->equip_index[EQI_HAND_L]])
	{
		struct item_data* id = sd->inventory_data[sd->equip_index[EQI_HAND_L]];
		if (id->view_id > 0)
			*lhand = id->view_id;
		else
			*lhand = id->nameid;
	} else
		*lhand = 0;
#endif
}

//To make the assignation of the level based on limits clearer/easier. [Skotlex]
static int clif_setlevel_sub(int lv) {
	if( lv < battle_config.max_lv ) {
		;
	} else if( lv < battle_config.aura_lv ) {
		lv = battle_config.max_lv - 1;
	} else {
		lv = battle_config.max_lv;
	}

	return lv;
}

static int clif_setlevel(struct block_list* bl) {
	int lv = status_get_lv(bl);
	if( battle_config.client_limit_unit_lv&bl->type )
		return clif_setlevel_sub(lv);
	switch( bl->type ) {
		case BL_NPC:
		case BL_PET:
			// npcs and pets do not have level
			return 0;
	}
	return lv;
}

/*==========================================
 * Prepares 'unit standing/spawning' packet
 *------------------------------------------*/
static void clif_set_unit_idle( struct block_list* bl, bool walking, send_target target, struct block_list* tbl ){
	nullpo_retv( bl );

	map_session_data* sd = BL_CAST( BL_PC, bl );
	status_change* sc = status_get_sc( bl );
	struct view_data* vd = status_get_viewdata( bl );
	int g_id = status_get_guild_id( bl );

#if PACKETVER < 20091103
	if( !pcdb_checkid( vd->class_ ) ){
		struct packet_idle_unit2 p;

		p.PacketType = idle_unit2Type;
#if PACKETVER >= 20071106
		p.objecttype = clif_bl_type( bl, walking );
#endif
		p.GID = bl->id;
		p.speed = status_get_speed( bl );
		p.bodyState = ( sc ) ? sc->opt1 : 0;
		p.healthState = ( sc ) ? sc->opt2 : 0;
		p.effectState = ( sc ) ? sc->option : 0;
		p.job = vd->class_;
		p.head = vd->hair_style;
		p.weapon = vd->weapon;
		p.accessory = vd->head_bottom;
		if( bl->type == BL_NPC && vd->class_ == JT_GUILD_FLAG ){
			// The hell, why flags work like this?
			p.shield = status_get_emblem_id( bl );
			p.accessory2 = GetWord( g_id, 1 );
			p.accessory3 = GetWord( g_id, 0 );
		}else{
			p.shield = vd->shield;
			p.accessory2 = vd->head_top;
			p.accessory3 = vd->head_mid;
		}
		p.headpalette = vd->hair_color;
		p.bodypalette = vd->cloth_color;
		p.headDir = ( sd )? sd->head_dir : 0;
		p.GUID = g_id;
		p.GEmblemVer = status_get_emblem_id( bl );
		p.honor = ( sd ) ? sd->status.manner : 0;
		p.virtue = ( sc ) ? sc->opt3 : 0;
		p.isPKModeON = ( sd && sd->status.karma ) ? 1 : 0;
		p.sex = vd->sex;
		WBUFPOS( &p.PosDir[0], 0, bl->x, bl->y, unit_getdir( bl ) );
		p.xSize = p.ySize = ( sd ) ? 5 : 0;
		p.state = vd->dead_sit;
		p.clevel = clif_setlevel( bl );

		clif_send( &p, sizeof( p ), tbl, target );

		return;
	}
#endif

	struct packet_idle_unit p;

	p.PacketType = idle_unitType;
#if PACKETVER >= 20091103
	p.PacketLength = sizeof(p);
	p.objecttype = clif_bl_type( bl, walking );
#endif
#if PACKETVER >= 20131223
	p.AID = bl->id;
	p.GID = (sd) ? sd->status.char_id : 0; // CCODE
#else
	p.GID = bl->id;
#endif
	p.speed = status_get_speed(bl);
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;

	// npc option changed?
	if( tbl && tbl->type == BL_PC && bl->type == BL_NPC ){
		map_session_data* sd = (map_session_data*)tbl;
		struct npc_data* nd = (struct npc_data*)bl;
		int option = (sc) ? sc->option : 0;

		if( !nd->vd.dead_sit ){
			if( std::find( sd->cloaked_npc.begin(), sd->cloaked_npc.end(), nd->bl.id ) != sd->cloaked_npc.end() ){
				option ^= OPTION_CLOAK;
			}
		}

		p.effectState = option;
	}else{
		p.effectState = (sc) ? sc->option : 0;
	}
	p.job = vd->class_;
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
#if PACKETVER < 7 || PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.shield = vd->shield;
#endif
	if( bl->type == BL_NPC && vd->class_ == JT_GUILD_FLAG ){
		// The hell, why flags work like this?
		p.accessory = status_get_emblem_id( bl );
		p.accessory2 = GetWord( g_id, 1 );
		p.accessory3 = GetWord( g_id, 0 );
	}else{
		p.accessory = vd->head_bottom;
		p.accessory2 = vd->head_top;
		p.accessory3 = vd->head_mid;
	}
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd)? sd->head_dir : 0;
#if PACKETVER >= 20101124
	p.robe = vd->robe;
#endif
	p.GUID = g_id;
	p.GEmblemVer = status_get_emblem_id( bl );
	p.honor = (sd) ? sd->status.manner : 0;
	p.virtue = (sc) ? sc->opt3 : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS( &p.PosDir[0], 0, bl->x, bl->y, unit_getdir( bl ) );
	p.xSize = p.ySize = (sd) ? 5 : 0;
	p.state = vd->dead_sit;
	p.clevel = clif_setlevel( bl );
#if PACKETVER >= 20080102
	p.font = (sd) ? sd->status.font : 0;
#endif
#if PACKETVER >= 20120221
	if( battle_config.monster_hp_bars_info && !map_getmapflag( bl->m, MF_HIDEMOBHPBAR ) && bl->type == BL_MOB && ( status_get_hp( bl ) < status_get_max_hp( bl ) ) ){
		p.maxHP = status_get_max_hp(bl);
		p.HP = status_get_hp(bl);
	}else{
		p.maxHP = -1;
		p.HP = -1;
	}

	if( bl->type == BL_MOB ){
		p.isBoss = ( (mob_data*)bl )->get_bosstype();
	}else if( bl->type == BL_PET ){
		p.isBoss = ( (pet_data*)bl )->db->get_bosstype();
	}else{
		p.isBoss = BOSSTYPE_NONE;
	}
#endif
#if PACKETVER >= 20150513
	p.body = vd->body_style;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	safestrncpy(p.name, status_get_name( bl ), NAME_LENGTH);
#endif

	clif_send( &p, sizeof( p ), tbl, target );
	// if disguised, send to self
	if( disguised( bl ) ){
#if PACKETVER >= 20091103
		p.objecttype = pcdb_checkid( status_get_viewdata( bl )->class_ ) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
#if PACKETVER >= 20131223
		p.AID = disguised_bl_id( bl->id );
#else
		p.GID = disguised_bl_id( bl->id );
#endif
#else
		p.GID = disguised_bl_id( bl->id );
#endif
		clif_send(&p, sizeof(p), bl, SELF);
	}
}

static void clif_spawn_unit( struct block_list *bl, enum send_target target ){
	nullpo_retv( bl );

	map_session_data* sd = BL_CAST( BL_PC, bl );
	status_change* sc = status_get_sc( bl );
	struct view_data* vd = status_get_viewdata( bl );
	int g_id = status_get_guild_id( bl );

#if PACKETVER < 20091103
	if( !pcdb_checkid( vd->class_ ) ){
		struct packet_spawn_unit2 p;

		p.PacketType = spawn_unit2Type;
#if PACKETVER >= 20071106
		p.objecttype = clif_bl_type( bl, false );
#endif
		p.GID = bl->id;
		p.speed = status_get_speed( bl );
		p.bodyState = ( sc ) ? sc->opt1 : 0;
		p.healthState = ( sc ) ? sc->opt2 : 0;
		p.effectState = ( sc ) ? sc->option : 0;
		p.head = vd->hair_style;
		p.weapon = vd->weapon;
		p.accessory = vd->head_bottom;
		p.job = vd->class_;
		if( bl->type == BL_NPC && vd->class_ == JT_GUILD_FLAG ){
			// The hell, why flags work like this?
			p.shield = status_get_emblem_id( bl );
			p.accessory2 = GetWord( g_id, 1 );
			p.accessory3 = GetWord( g_id, 0 );
		}else{
			p.shield = vd->shield;
			p.accessory2 = vd->head_top;
			p.accessory3 = vd->head_mid;
		}
		p.headpalette = vd->hair_color;
		p.bodypalette = vd->cloth_color;
		p.headDir = ( sd ) ? sd->head_dir : 0;
		p.isPKModeON = ( sd && sd->status.karma ) ? 1 : 0;
		p.sex = vd->sex;
		WBUFPOS( &p.PosDir[0], 0, bl->x, bl->y, unit_getdir( bl ) );
		p.xSize = p.ySize = ( sd ) ? 5 : 0;

		clif_send( &p, sizeof( p ), bl, target );
		return;
	}
#endif

	struct packet_spawn_unit p;

	p.PacketType = spawn_unitType;
#if PACKETVER >= 20091103
	p.PacketLength = sizeof(p);
	p.objecttype = clif_bl_type( bl, false );
#endif
#if PACKETVER >= 20131223
	p.AID = bl->id;
	p.GID = (sd) ? sd->status.char_id : 0; // CCODE
#else
	p.GID = bl->id;
#endif
	p.speed = status_get_speed( bl );
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;
	p.effectState = (sc) ? sc->option : 0;
	p.job = vd->class_;
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
#if PACKETVER < 7 || PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.shield = vd->shield;
#endif
	if( bl->type == BL_NPC && vd->class_ == JT_GUILD_FLAG ){
		// The hell, why flags work like this?
		p.accessory = status_get_emblem_id( bl );
		p.accessory2 = GetWord( g_id, 1 );
		p.accessory3 = GetWord( g_id, 0 );
	}else{
		p.accessory = vd->head_bottom;
		p.accessory2 = vd->head_top;
		p.accessory3 = vd->head_mid;
	}
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd)? sd->head_dir : 0;
#if PACKETVER >= 20101124
	p.robe = vd->robe;
#endif
	p.GUID = g_id;
	p.GEmblemVer = status_get_emblem_id( bl );
	p.honor = (sd) ? sd->status.manner : 0;
	p.virtue = (sc) ? sc->opt3 : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS( &p.PosDir[0], 0, bl->x, bl->y, unit_getdir( bl ) );
	p.xSize = p.ySize = (sd) ? 5 : 0;
	p.clevel = clif_setlevel( bl );
#if PACKETVER >= 20080102
	p.font = (sd) ? sd->status.font : 0;
#endif
#if PACKETVER >= 20120221
	if( battle_config.monster_hp_bars_info && bl->type == BL_MOB && !map_getmapflag( bl->m, MF_HIDEMOBHPBAR ) && ( status_get_hp( bl ) < status_get_max_hp( bl ) ) ){
		p.maxHP = status_get_max_hp( bl );
		p.HP = status_get_hp( bl );
	}else{
		p.maxHP = -1;
		p.HP = -1;
	}

	if( bl->type == BL_MOB ){
		p.isBoss = ( (mob_data*)bl )->get_bosstype();
	}else if( bl->type == BL_PET ){
		p.isBoss = ( (pet_data*)bl )->db->get_bosstype();
	}else{
		p.isBoss = BOSSTYPE_NONE;
	}
#endif
#if PACKETVER >= 20150513
	p.body = vd->body_style;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	safestrncpy( p.name, status_get_name( bl ), NAME_LENGTH );
#endif

	if( disguised( bl ) ){
		nullpo_retv( sd );

		if( sd->status.class_ != sd->disguise ){
			clif_send( &p, sizeof( p ), bl, target );
		}

#if PACKETVER >= 20091103
		p.objecttype = pcdb_checkid( status_get_viewdata(bl)->class_ ) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
#if PACKETVER >= 20131223
		p.AID = disguised_bl_id( bl->id );
#else
		p.GID = disguised_bl_id( bl->id );
#endif
#else
		p.GID = disguised_bl_id( bl->id );
#endif
		clif_send( &p, sizeof( p ), bl, SELF );
	}else{
		clif_send( &p, sizeof( p ), bl, target );
	}
}

/*==========================================
 * Prepares 'unit walking' packet
 *------------------------------------------*/
static void clif_set_unit_walking( struct block_list& bl, map_session_data* tsd, struct unit_data& ud, enum send_target target ){
	struct packet_unit_walking p;

	p.PacketType = unit_walkingType;
#if PACKETVER >= 20091103
	p.PacketLength = sizeof(p);
#endif
#if PACKETVER >= 20071106
	p.objecttype = clif_bl_type( &bl, true );
#endif
	map_session_data* sd = BL_CAST(BL_PC, &bl);
#if PACKETVER >= 20131223
	p.AID = bl.id;
	p.GID = (sd) ? sd->status.char_id : 0; // CCODE
#else
	p.GID = bl.id;
#endif
	p.speed = status_get_speed( &bl );
	status_change* sc = status_get_sc( &bl );
	p.bodyState = (sc) ? sc->opt1 : 0;
	p.healthState = (sc) ? sc->opt2 : 0;
	p.effectState = (sc) ? sc->option : 0;
	struct view_data* vd = status_get_viewdata( &bl );
	p.job = vd->class_;
	p.head = vd->hair_style;
	p.weapon = vd->weapon;
	p.accessory = vd->head_bottom;
	p.moveStartTime = client_tick(gettick());
#if PACKETVER < 7 || PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.shield = vd->shield;
#endif
	p.accessory2 = vd->head_top;
	p.accessory3 = vd->head_mid;
	p.headpalette = vd->hair_color;
	p.bodypalette = vd->cloth_color;
	p.headDir = (sd) ? sd->head_dir : 0;
#if PACKETVER >= 20101124
	p.robe = vd->robe;
#endif
	p.GUID = status_get_guild_id( &bl );
	p.GEmblemVer = status_get_emblem_id( &bl );
	p.honor = (sd) ? sd->status.manner : 0;
	p.virtue = (sc) ? sc->opt3 : 0;
	p.isPKModeON = (sd && sd->status.karma) ? 1 : 0;
	p.sex = vd->sex;
	WBUFPOS2(&p.MoveData[0], 0, bl.x, bl.y, ud.to_x, ud.to_y, ud.sx, ud.sy);
	p.xSize = p.ySize = (sd) ? 5 : 0;
	p.clevel = clif_setlevel( &bl );
#if PACKETVER >= 20080102
	p.font = (sd) ? sd->status.font : 0;
#endif
#if PACKETVER >= 20120221
	if( battle_config.monster_hp_bars_info && !map_getmapflag(bl.m, MF_HIDEMOBHPBAR) && bl.type == BL_MOB && (status_get_hp( &bl ) < status_get_max_hp( &bl ) ) ){
		p.maxHP = status_get_max_hp( &bl );
		p.HP = status_get_hp( &bl );
	} else {
		p.maxHP = -1;
		p.HP = -1;
	}

	if( bl.type == BL_MOB ){
		p.isBoss = reinterpret_cast<mob_data*>( &bl )->get_bosstype();
	}else if( bl.type == BL_PET ){
		p.isBoss = reinterpret_cast<pet_data*>( &bl )->db->get_bosstype();
	}else{
		p.isBoss = BOSSTYPE_NONE;
	}
#endif
#if PACKETVER >= 20150513
	p.body = vd->body_style;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	safestrncpy(p.name, status_get_name( &bl ), NAME_LENGTH);
#endif

	clif_send( &p, sizeof(p), tsd ? &tsd->bl : &bl, target );

	// if disguised, send the info to self
	if( disguised( &bl ) ){
#if PACKETVER >= 20091103
		p.objecttype = pcdb_checkid( status_get_viewdata( &bl )->class_ ) ? 0x0 : 0x5; //PC_TYPE : NPC_MOB_TYPE
#if PACKETVER >= 20131223
		p.AID = disguised_bl_id( bl.id );
#else
		p.GID = disguised_bl_id( bl.id );
#endif
#else
		p.GID = disguised_bl_id( bl.id );
#endif
		clif_send(&p, sizeof(p), &bl, SELF);
	}
}

/// Changes sprite of an NPC object (ZC_NPCSPRITE_CHANGE).
/// 01b0 <id>.L <type>.B <value>.L
/// type:
///     unused
void clif_class_change_target(struct block_list *bl,int class_,int type, enum send_target target, map_session_data *sd)
{
	nullpo_retv(bl);

	if(!pcdb_checkid(class_))
	{// player classes yield missing sprites
		unsigned char buf[16];
		WBUFW(buf,0)=0x1b0;
		WBUFL(buf,2)=bl->id;
		WBUFB(buf,6)=type;
		WBUFL(buf,7)=class_;
		clif_send(buf,packet_len(0x1b0),(sd == nullptr ? bl : &(sd->bl)),target);
	}
}

void clif_servantball( map_session_data& sd, struct block_list* target, enum send_target send_target ){
	PACKET_ZC_SPIRITS p = {};

	p.PacketType = HEADER_ZC_SPIRITS;
	p.AID = sd.bl.id;
	p.num = sd.servantball;

	if( target == nullptr ){
		target = &sd.bl;
	}

	clif_send( &p, sizeof( p ), target, send_target );
}

void clif_abyssball( map_session_data& sd, struct block_list* target, enum send_target send_target ){
	PACKET_ZC_SPIRITS p = {};

	p.PacketType = HEADER_ZC_SPIRITS;
	p.AID = sd.bl.id;
	p.num = sd.abyssball;

	if( target == nullptr ){
		target = &sd.bl;
	}

	clif_send( &p, sizeof( p ), target, send_target );
}

/// Notifies the client of an object's Millenium Shields.
static void clif_millenniumshield_single( map_session_data& sd, map_session_data& tsd ){
#if PACKETVER >= 20081126
	status_change_entry* sce = sd.sc.getSCE( SC_MILLENNIUMSHIELD );

	if( sce == nullptr ){
		return;
	}

	PACKET_ZC_MILLENNIUMSHIELD packet{};

	packet.packetType = HEADER_ZC_MILLENNIUMSHIELD;
	packet.aid = sd.bl.id;
	packet.num = sce->val2;
	packet.state = 0;

	clif_send( &packet, sizeof( packet ), &tsd.bl, SELF );
#endif
}

/*==========================================
 * Kagerou/Oboro amulet spirit
 *------------------------------------------*/
static void clif_spiritcharm_single( map_session_data& sd, map_session_data& tsd ){
#if PACKETVER >= 20111102
	if( sd.spiritcharm_type == CHARM_TYPE_NONE ){
		return;
	}

	if( sd.spiritcharm <= 0 ){
		return;
	}

	PACKET_ZC_SPIRITS_ATTRIBUTE packet{};

	packet.packetType = HEADER_ZC_SPIRITS_ATTRIBUTE;
	packet.aid = sd.bl.id;
	packet.spiritsType = static_cast<decltype(packet.spiritsType)>(sd.spiritcharm_type);
	packet.num = static_cast<decltype(packet.num)>(sd.spiritcharm);

	clif_send( &packet, sizeof( packet ), &tsd.bl, SELF );
#endif
}

/*==========================================
 * Enchanting Shadow / Shadow Scar Spirit
 *------------------------------------------*/
void clif_enchantingshadow_spirit(unit_data &ud) {
#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191120 || PACKETVER_ZERO_NUM >= 20191127
	PACKET_ZC_TARGET_SPIRITS p = {};

	p.packetType = HEADER_ZC_TARGET_SPIRITS;
	p.GID = ud.bl->id;
	p.unknown_val = 0;
	p.amount = static_cast<uint16>(ud.shadow_scar_timer.size());

	clif_send(&p, sizeof(p), ud.bl, AREA);
#endif
}

/*==========================================
 * Run when player changes map / refreshes
 * Tells its client to display all weather settings being used by this map
 *------------------------------------------*/
static void clif_weather_check(map_session_data *sd)
{
	int16 m = sd->bl.m;
	int fd = sd->fd;

	if (map_getmapflag(m, MF_SNOW)
		|| map_getmapflag(m, MF_CLOUDS)
		|| map_getmapflag(m, MF_FOG)
		|| map_getmapflag(m, MF_FIREWORKS)
		|| map_getmapflag(m, MF_SAKURA)
		|| map_getmapflag(m, MF_LEAVES)
		|| map_getmapflag(m, MF_CLOUDS2))
	{
		if (map_getmapflag(m, MF_SNOW))
			clif_specialeffect_single(&sd->bl, EF_SNOW, fd);
		if (map_getmapflag(m, MF_CLOUDS))
			clif_specialeffect_single(&sd->bl, EF_CLOUD3, fd);
		if (map_getmapflag(m, MF_CLOUDS2))
			clif_specialeffect_single(&sd->bl, EF_CLOUD5, fd);
		if (map_getmapflag(m, MF_FOG))
			clif_specialeffect_single(&sd->bl, EF_CLOUD4, fd);
		if (map_getmapflag(m, MF_FIREWORKS)) {
			clif_specialeffect_single(&sd->bl, EF_POKJUK, fd);
			clif_specialeffect_single(&sd->bl, EF_THROWITEM2, fd);
			clif_specialeffect_single(&sd->bl, EF_POKJUK_SOUND, fd);
		}
		if (map_getmapflag(m, MF_SAKURA))
			clif_specialeffect_single(&sd->bl, EF_SAKURA, fd);
		if (map_getmapflag(m, MF_LEAVES))
			clif_specialeffect_single(&sd->bl, EF_MAPLE, fd);
	}
}
/**
 * Run when the weather on a map changes, throws all players in map id 'm' to clif_weather_check function
 **/
void clif_weather(int16 m)
{
	struct s_mapiterator* iter;
	map_session_data *sd=nullptr;

	iter = mapit_getallusers();
	for( sd = (map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (map_session_data*)mapit_next(iter) )
	{
		if( sd->bl.m == m )
			clif_weather_check(sd);
	}
	mapit_free(iter);
}

/**
 * Hide a NPC from the effects of Maya Purple card.
 * @param bl: Block data
 * @return True if NPC is disabled or false otherwise
 */
static inline bool clif_npc_mayapurple(block_list *bl) {
	nullpo_retr(false, bl);

	if (bl->type == BL_NPC) {
		npc_data *nd = map_id2nd(bl->id);

		// TODO: Confirm if waitingroom cause any special cases
		if (/* nd->chat_id == 0 && */ nd->is_invisible)
			return true;
	}

	return false;
}

/**
 * Main function to spawn a unit on the client (player/mob/pet/etc)
 **/
int clif_spawn( struct block_list *bl, bool walking ){
	struct view_data *vd;

	vd = status_get_viewdata(bl);
	if( !vd || vd->class_ == JT_INVISIBLE )
		return 0;

	// Hide NPC from Maya Purple card
	if (clif_npc_mayapurple(bl))
		return 0;

	if( bl->type == BL_NPC && !vd->dead_sit ){
		clif_set_unit_idle( bl, walking, AREA_WOS, bl );
	}else{
		clif_spawn_unit( bl, AREA_WOS );
	}

	if (vd->cloth_color)
		clif_refreshlook(bl,bl->id,LOOK_CLOTHES_COLOR,vd->cloth_color,AREA_WOS);
	if (vd->body_style)
		clif_refreshlook(bl,bl->id,LOOK_BODY2,vd->body_style,AREA_WOS);

	switch (bl->type)
	{
	case BL_PC:
		{
			TBL_PC *sd = ((TBL_PC*)bl);

			if (sd->spiritball > 0)
				clif_spiritball(&sd->bl);
			if (sd->sc.getSCE(SC_MILLENNIUMSHIELD))
				clif_millenniumshield( sd->bl, sd->sc.getSCE( SC_MILLENNIUMSHIELD )->val2 );
			if (sd->soulball > 0)
				clif_soulball(sd);
			if (sd->servantball > 0)
				clif_servantball( *sd );
			if (sd->abyssball > 0)
				clif_abyssball( *sd );
			if(sd->state.size==SZ_BIG) // tiny/big players [Valaris]
				clif_specialeffect(bl,EF_GIANTBODY2,AREA);
			else if(sd->state.size==SZ_MEDIUM)
				clif_specialeffect(bl,EF_BABYBODY2,AREA);
			if( sd->bg_id && map_getmapflag(sd->bl.m, MF_BATTLEGROUND) )
				clif_sendbgemblem_area(sd);
			if (sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm > 0)
				clif_spiritcharm( *sd );
			if (sd->status.robe)
				clif_refreshlook(bl,bl->id,LOOK_ROBE,sd->status.robe,AREA);
			clif_efst_status_change_sub(bl, bl, AREA);
			clif_hat_effects( *sd, sd->bl, AREA );
		}
		break;
	case BL_MOB:
		{
			TBL_MOB *md = ((TBL_MOB*)bl);
			if(md->special_state.size==SZ_BIG) // tiny/big mobs [Valaris]
				clif_specialeffect(&md->bl,EF_GIANTBODY2,AREA);
			else if(md->special_state.size==SZ_MEDIUM)
				clif_specialeffect(&md->bl,EF_BABYBODY2,AREA);
			if ( md->special_state.ai == AI_ABR || md->special_state.ai == AI_BIONIC )
				clif_summon_init(*md);
		}
		break;
	case BL_NPC:
		{
			TBL_NPC *nd = ((TBL_NPC*)bl);
			if( nd->size == SZ_BIG )
				clif_specialeffect(&nd->bl,EF_GIANTBODY2,AREA);
			else if( nd->size == SZ_MEDIUM )
				clif_specialeffect(&nd->bl,EF_BABYBODY2,AREA);
			clif_efst_status_change_sub(bl, bl, AREA);
			clif_progressbar_npc_area(nd);
		}
		break;
	case BL_PET:
		if (vd->head_bottom)
			clif_pet_equip_area((TBL_PET*)bl); // needed to display pet equip properly
		break;
	}
	return 0;
}

/// Sends information about owned homunculus to the client . [orn]
/// 022e <name>.24B <modified>.B <level>.W <hunger>.W <intimacy>.W <equip id>.W <atk>.W <matk>.W <hit>.W <crit>.W <def>.W <mdef>.W <flee>.W <aspd>.W <hp>.W <max hp>.W <sp>.W <max sp>.W <exp>.L <max exp>.L <skill points>.W <atk range>.W	(ZC_PROPERTY_HOMUN)
/// 09f7 <name>.24B <modified>.B <level>.W <hunger>.W <intimacy>.W <equip id>.W <atk>.W <matk>.W <hit>.W <crit>.W <def>.W <mdef>.W <flee>.W <aspd>.W <hp>.L <max hp>.L <sp>.W <max sp>.W <exp>.L <max exp>.L <skill points>.W <atk range>.W (ZC_PROPERTY_HOMUN_2)
void clif_hominfo( map_session_data *sd, struct homun_data *hd, int flag ){
#if PACKETVER_MAIN_NUM >= 20101005 || PACKETVER_RE_NUM >= 20080827 || defined(PACKETVER_ZERO)
	nullpo_retv( sd );
	nullpo_retv( hd );

	struct status_data *status = &hd->battle_status;
	PACKET_ZC_PROPERTY_HOMUN p = {};

	p.packetType = HEADER_ZC_PROPERTY_HOMUN;
	safestrncpy( p.name, hd->homunculus.name, sizeof( p.name ) );
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	p.flags = ( !battle_config.hom_rename && hd->homunculus.rename_flag ? 0x1 : 0x0 ) | ( hd->homunculus.vaporize == HOM_ST_REST ? 0x2 : 0 ) | ( hd->homunculus.hp > 0 ? 0x4 : 0 );
	p.level = hd->homunculus.level;
	p.hunger = hd->homunculus.hunger;
	p.intimacy = hd->homunculus.intimacy / 100;
#if !(PACKETVER_MAIN_NUM >= 20190619 || PACKETVER_RE_NUM >= 20190605 || PACKETVER_ZERO_NUM >= 20190626)
	p.itemId = 0; // equip id
#endif
	p.atk2 = cap_value( status->rhw.atk2 + status->batk, 0, INT16_MAX );
	p.matk = i16min( status->matk_max, INT16_MAX ); //FIXME capping to INT16 here is too late
	p.hit = status->hit;
	if( battle_config.hom_setting&HOMSET_DISPLAY_LUK ){
		p.crit = status->luk / 3 + 1;	//crit is a +1 decimal value! Just display purpose.[Vicious]
	}else{
		p.crit = status->cri / 10;
	}
#ifdef RENEWAL
	p.def = status->def + status->def2;
	p.mdef = status->mdef + status->mdef2;
#else
	p.def = status->def + status->vit;
	p.mdef = status->mdef;
#endif
	p.flee = status->flee;
	p.amotion = (flag) ? 0 : status->amotion;
#if PACKETVER >= 20141016
	// Homunculus HP bar will screw up if the percentage calculation exceeds signed values
	// Tested maximum: 21474836(=INT32_MAX/100), any value above will screw up the HP bar
	if( status->max_hp > ( INT32_MAX / 100 ) ){
		p.hp = status->hp / ( status->max_hp / 100 );
		p.maxHp = 100;
	}else{
		p.hp = status->hp;
		p.maxHp = status->max_hp;
	}
#else
	if( status->max_hp > INT16_MAX ){
		p.hp = status->hp / ( status->max_hp / 100 );
		p.maxHp = 100;
	}else{
		p.hp = status->hp;
		p.maxHp = status->max_hp;
	}
#endif
	if( status->max_sp > INT16_MAX ){
		p.sp = status->sp / ( status->max_sp / 100 );
		p.maxSp = 100;
	}else{
		p.sp = status->sp;
		p.maxSp = status->max_sp;
	}
#if PACKETVER_MAIN_NUM >= 20210303 || PACKETVER_RE_NUM >= 20211103
	p.exp = hd->homunculus.exp;
	p.expNext = hd->exp_next;
#else
	p.exp = (uint32)hd->homunculus.exp;
	p.expNext = (uint32)hd->exp_next;
#endif
	switch( hom_class2type( hd->homunculus.class_ ) ){
		case HT_REG:
		case HT_EVO:
			if( hd->homunculus.level >= battle_config.hom_max_level ){
				p.expNext = 0;
			}
			break;
		case HT_S:
			if( hd->homunculus.level >= battle_config.hom_S_max_level ){
				p.expNext = 0;
			}
			break;
	}
	p.skillPoints = hd->homunculus.skillpts;
	p.range = status_get_range( &hd->bl );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}


/// Notification about a change in homunuculus' state.
/// 0230 <type>.B <state>.B <id>.L <data>.L (ZC_CHANGESTATE_MER)
/// type:
///     unused
/// state:
///     0 = pre-init
///     1 = intimacy
///     2 = hunger
///     3 = accessory?
///     ? = ignored
void clif_send_homdata( homun_data& hd, e_hom_state2 state ){
#if PACKETVER >= 20050523
	if( hd.master == nullptr ){
		return;
	}

	uint32 param;

	switch( state ){
		case SP_ACK:
			param = 0;
			break;
		case SP_INTIMATE:
			// TODO: Why is something like this here? [Lemongrass]
			if( hd.homunculus.class_ == hd.homunculusDB->evo_class && hom_intimacy_intimacy2grade( hd.homunculus.intimacy ) >= HOMGRADE_LOYAL ){
				hom_calc_skilltree( &hd );
			}

			param = hd.homunculus.intimacy / 100;
			break;
		case SP_HUNGRY:
			param = hd.homunculus.hunger;
			break;
		default:
			return;
	}
	
	PACKET_ZC_CHANGESTATE_MER packet{};

	packet.packetType = HEADER_ZC_CHANGESTATE_MER;
	packet.type = 0;
	packet.state = state;
	packet.gid = hd.bl.id;
	packet.data = param;

	clif_send( &packet, sizeof( packet ), &hd.master->bl, SELF );
#endif
}


void clif_homskillinfoblock( homun_data& hd ){
#if PACKETVER >= 20050530
	map_session_data* sd = hd.master;

	if( sd == nullptr ){
		return;
	}
	
	PACKET_ZC_HOSKILLINFO_LIST* packet = reinterpret_cast<PACKET_ZC_HOSKILLINFO_LIST*>(packet_buffer);

	packet->packetType = HEADER_ZC_HOSKILLINFO_LIST;
	packet->packetLength = sizeof( *packet );

	for(int i = 0, count = 0; i < MAX_HOMUNSKILL; i++) {
		int id = hd.homunculus.hskill[i].id;

		if (id != 0) {
			int combo = (hd.homunculus.hskill[i].flag)&SKILL_FLAG_TMP_COMBO;
			short idx = hom_skill_get_index(id);
			if (idx == -1)
				continue;
			packet->skills[count].id = id;
			packet->skills[count].inf = (combo) ? INF_SELF_SKILL : skill_get_inf(id);
			packet->skills[count].unknown = 0;
			packet->skills[count].level = hd.homunculus.hskill[idx].lv;
			packet->skills[count].sp = skill_get_sp(id,hd.homunculus.hskill[idx].lv);
			packet->skills[count].range = skill_get_range2(&hd.bl,id,hd.homunculus.hskill[idx].lv,false);
			safestrncpy(packet->skills[count].name, skill_get_name(id), NAME_LENGTH);
			packet->skills[count].upgradable = (hd.homunculus.level < hom_skill_get_min_level(hd.homunculus.class_, id) || hd.homunculus.hskill[idx].lv >= hom_skill_tree_get_max(id, hd.homunculus.class_)) ? 0 : 1;
			packet->packetLength += sizeof( packet->skills[0] );
			count++;
		}
	}

	clif_send( packet, packet->packetLength, &sd->bl, SELF );
#endif
}

void clif_homskillup( homun_data& hd, uint16 skill_id ){
#if PACKETVER >= 20050531
	short idx = hom_skill_get_index( skill_id );

	if( idx == -1 ){
		return;
	}

	map_session_data* sd = hd.master;

	if( sd == nullptr ){
		return;
	}

	PACKET_ZC_HOSKILLINFO_UPDATE packet{};

	packet.packetType = HEADER_ZC_HOSKILLINFO_UPDATE;
	packet.skill_id = skill_id;
	packet.Level = hd.homunculus.hskill[idx].lv;
	packet.SP = skill_get_sp(skill_id,hd.homunculus.hskill[idx].lv);
	packet.AttackRange = skill_get_range2(&hd.bl,skill_id,hd.homunculus.hskill[idx].lv,false);
	packet.upgradable = (hd.homunculus.level < hom_skill_get_min_level(hd.homunculus.class_, skill_id) || hd.homunculus.hskill[idx].lv >= hom_skill_tree_get_max(skill_id, hd.homunculus.class_)) ? 0 : 1;

	clif_send( &packet, sizeof( packet ), &sd->bl, SELF );
#endif
}

/// Result of request to feed a homun/merc.
/// 022f <result>.B <name id>.W (ZC_FEED_MER)
/// result:
///     0 = failure
///     1 = success
void clif_hom_food( map_session_data& sd, int32 foodid, bool success ){
	PACKET_ZC_FEED_MER packet{};

	packet.packetType = HEADER_ZC_FEED_MER;
	packet.result = success;
	packet.itemId = client_nameid( foodid );

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client, that it is walking.
/// 0087 <walk start time>.L <walk data>.6B (ZC_NOTIFY_PLAYERMOVE)
void clif_walkok( map_session_data& sd ){
	PACKET_ZC_NOTIFY_PLAYERMOVE packet{};

	packet.packetType = HEADER_ZC_NOTIFY_PLAYERMOVE;
	packet.moveStartTime = client_tick(gettick());
	WBUFPOS2(packet.moveData, 0, sd.bl.x, sd.bl.y, sd.ud.to_x, sd.ud.to_y, 8, 8);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies clients in an area, that an other visible object is walking.
/// Note: unit must not be self
void clif_move( struct unit_data& ud )
{
	struct block_list* bl = ud.bl;
	struct view_data* vd = status_get_viewdata(bl);

	if (bl == nullptr || vd == nullptr)
		return;

	// This performance check is needed to keep GM-hidden objects from being notified to bots.
	if (vd->class_ == JT_INVISIBLE)
		return;

	// Hide NPC from Maya Purple card
	if (clif_npc_mayapurple(bl))
		return;

	status_change* sc = nullptr;

	if ((sc = status_get_sc(bl)) && sc->option & (OPTION_HIDE | OPTION_CLOAK | OPTION_INVISIBLE | OPTION_CHASEWALK))
		clif_ally_only = true;

	clif_set_unit_walking( *bl, nullptr, ud, AREA_WOS );

	if (vd->cloth_color)
		clif_refreshlook(bl, bl->id, LOOK_CLOTHES_COLOR, vd->cloth_color, AREA_WOS);
	if (vd->body_style)
		clif_refreshlook(bl, bl->id, LOOK_BODY2, vd->body_style, AREA_WOS);

	switch (bl->type) {
	case BL_PC:
		{
			map_session_data* sd = reinterpret_cast<map_session_data*>( bl );
			if (sd->state.size == SZ_BIG) // tiny/big players [Valaris]
				clif_specialeffect(&sd->bl, EF_GIANTBODY2, AREA);
			else if (sd->state.size == SZ_MEDIUM)
				clif_specialeffect(&sd->bl, EF_BABYBODY2, AREA);
			if (sd->status.robe)
				clif_refreshlook(bl, bl->id, LOOK_ROBE, sd->status.robe, AREA);
		}
	break;
	case BL_MOB:
		{
			mob_data* md = reinterpret_cast<mob_data*>( bl );
			if (md->special_state.size == SZ_BIG) // tiny/big mobs [Valaris]
				clif_specialeffect(&md->bl, EF_GIANTBODY2, AREA);
			else if (md->special_state.size == SZ_MEDIUM)
				clif_specialeffect(&md->bl, EF_BABYBODY2, AREA);
		}
	break;
	case BL_PET:
		if (vd->head_bottom) // needed to display pet equip properly
			clif_pet_equip_area(BL_CAST(BL_PET, bl));
		break;
	}

	clif_ally_only = false;
}


/*==========================================
 * Delays the map_quit of a player after they are disconnected. [Skotlex]
 *------------------------------------------*/
static TIMER_FUNC(clif_delayquit){
	map_session_data *sd = nullptr;

	//Remove player from map server
	if ((sd = map_id2sd(id)) != nullptr && sd->fd == 0) //Should be a disconnected player.
		map_quit(sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
void clif_quitsave(int fd,map_session_data *sd) {
	if (!battle_config.prevent_logout ||
		sd->canlog_tick == 0 ||
		DIFF_TICK(gettick(), sd->canlog_tick) > battle_config.prevent_logout)
		map_quit(sd);
	else if (session_isValid(sd->fd)) {
		//Disassociate session from player (session is deleted after this function was called)
		//And set a timer to make him quit later.
		session[sd->fd]->session_data = nullptr;
		sd->fd = 0;
		add_timer(gettick() + 10000, clif_delayquit, sd->bl.id, 0);
	}
}

/// Notifies the client of a position change to coordinates on given map (ZC_NPCACK_MAPMOVE).
/// 0091 <map name>.16B <x>.W <y>.W
void clif_changemap( map_session_data& sd, short m, uint16 x, uint16 y ){
	PACKET_ZC_NPCACK_MAPMOVE packet{};

	packet.packetType = HEADER_ZC_NPCACK_MAPMOVE;
	mapindex_getmapname_ext(map_mapid2mapname(m), packet.mapName);
	packet.xPos = x;
	packet.yPos = y;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client of a position change to coordinates on given map, which is on another map-server.
/// 0092 <map name>.16B <x>.W <y>.W <ip>.L <port>.W (ZC_NPCACK_SERVERMOVE)
/// 0ac7 <map name>.16B <x>.W <y>.W <ip>.L <port>.W <unknown>.128B (ZC_NPCACK_SERVERMOVE_DOMAIN)
void clif_changemapserver( map_session_data& sd, const char* map, uint16 x, uint16 y, uint32 ip, uint16 port ){
	PACKET_ZC_NPCACK_SERVERMOVE packet{};

	packet.packetType = HEADER_ZC_NPCACK_SERVERMOVE;

	mapindex_getmapname_ext(map, packet.mapName);
	packet.xPos = x;
	packet.yPos = y;
	packet.ip = htonl(ip);
	packet.port = ntows(htons(port)); // [!] LE byte order here [!]
#if PACKETVER >= 20170315
	safestrncpy( packet.domain, "", sizeof( packet.domain ) );
#endif

#ifdef DEBUG
	ShowDebug( "Sending the client (%d %d.%d.%d.%d) to map-server with ip %d.%d.%d.%d and port %hu\n", sd.status.account_id, CONVIP(session[sd.fd]->client_addr), CONVIP(ip), port );
#endif

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// In many situations (knockback, backslide, etc.) Aegis sends both clif_slide and clif_fixpos
/// This function combines both calls and allows to simplify the calling code
void clif_blown(struct block_list *bl)
{
	clif_slide(bl, bl->x, bl->y);
	clif_fixpos( *bl );
}


/// Visually moves(slides) a character to x,y. If the target cell
/// isn't walkable, the char doesn't move at all. If the char is
/// sitting it will stand up.
/// 0088 <id>.L <x>.W <y>.W (ZC_STOPMOVE)
void clif_fixpos( block_list& bl ){	
	PACKET_ZC_STOPMOVE packet = {};

	packet.packetType = HEADER_ZC_STOPMOVE;
	packet.AID = bl.id;
	packet.xPos = bl.x;
	packet.yPos = bl.y;

	clif_send( &packet, sizeof( packet ), &bl, AREA);

	if( disguised( &bl ) ){
		packet.AID = disguised_bl_id( bl.id );
		clif_send( &packet, sizeof( packet ), &bl, SELF );
	}
}


/// Displays the buy/sell dialog of an NPC shop.
/// 00c4 <shop id>.L (ZC_SELECT_DEALTYPE)
void clif_npcbuysell( map_session_data& sd, npc_data& nd ){
	PACKET_ZC_SELECT_DEALTYPE packet{};

	packet.packetType = HEADER_ZC_SELECT_DEALTYPE;
	packet.npcId = nd.bl.id;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Presents list of items, that can be bought in an NPC shop.
/// 00c6 <packet len>.W { <price>.L <discount price>.L <item type>.B <name id>.W }* (ZC_PC_PURCHASE_ITEMLIST)
void clif_buylist( map_session_data& sd, npc_data& nd ){
	PACKET_ZC_PC_PURCHASE_ITEMLIST* p = reinterpret_cast<PACKET_ZC_PC_PURCHASE_ITEMLIST*>( packet_buffer );

	p->packetType = HEADER_ZC_PC_PURCHASE_ITEMLIST;
	p->packetLength = sizeof( *p );

	for( int i = 0, count = 0, discount = npc_shop_discount( &nd ); i < nd.u.shop.count; i++ ){
		int val = nd.u.shop.shop_item[i].value;

		p->items[count].price = val;
		p->items[count].discountPrice = ( discount ) ? pc_modifybuyvalue( &sd, val ) : val;
		p->items[count].itemType = itemtype( nd.u.shop.shop_item[i].nameid );
		p->items[count].itemId = client_nameid( nd.u.shop.shop_item[i].nameid );
#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103
		std::shared_ptr<item_data> id = item_db.find(nd.u.shop.shop_item[i].nameid);

		p->items[count].viewSprite = id->look;
		p->items[count].location = pc_equippoint_sub( &sd, id.get() );
#endif

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
		count++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
}


/// Presents list of items, that can be sold to an NPC shop.
/// 00c7 <packet len>.W { <index>.W <price>.L <overcharge price>.L }* (ZC_PC_SELL_ITEMLIST)
void clif_selllist( map_session_data& sd){
	if( !sd.npc_shopid ){
		return;
	}

	npc_data* nd = map_id2nd( sd.npc_shopid );

	if( nd == nullptr ){
		return;
	}

	PACKET_ZC_PC_SELL_ITEMLIST *packet = reinterpret_cast<PACKET_ZC_PC_SELL_ITEMLIST*>(packet_buffer);

	packet->packetType = HEADER_ZC_PC_SELL_ITEMLIST;
	packet->packetLength = sizeof( *packet );

	for( int i = 0, c = 0; i < MAX_INVENTORY; i++ ){
		if( sd.inventory.u.items_inventory[i].nameid <= 0 ){
			continue;
		}

		if( sd.inventory_data[i] == nullptr ){
			continue;
		}

		if( !pc_can_sell_item( &sd, &sd.inventory.u.items_inventory[i], nd->subtype ) ){
			continue;
		}

		int price;

		if( battle_config.rental_item_novalue && sd.inventory.u.items_inventory[i].expire_time ){
			price = 0;
		}else{
			price = sd.inventory_data[i]->value_sell;

			if( price < 0 ){
				continue;
			}
		}

		packet->items[c].index = client_index(i);
		packet->items[c].price = price;
		packet->items[c].overcharge = pc_modifysellvalue( &sd, price );

		packet->packetLength += sizeof( packet->items[0] );
		c++;
	}
	
	clif_send( packet, packet->packetLength, &sd.bl, SELF );
}

/// Closes shop (CZ_NPC_TRADE_QUIT).
/// 09d4
void clif_parse_NPCShopClosed(int fd, map_session_data *sd) {
	// TODO: State tracking?
	sd->npc_shopid = 0;
}

/**
 * Presents list of items, that can be sold to a Market shop.
 * @author: Ind and Yommy
 **/
/// 0x9d5 <packet len>.W { <name id>.W <type>.B <price>.L <qty>.L <weight>.W }* (ZC_NPC_MARKET_OPEN)
void clif_npc_market_open( map_session_data& sd, npc_data& nd ){
#if PACKETVER >= 20131223
	if( sd.state.trading ){
		return;
	}

	PACKET_ZC_NPC_MARKET_OPEN* p = reinterpret_cast<PACKET_ZC_NPC_MARKET_OPEN*>( packet_buffer );

	p->packetType = HEADER_ZC_NPC_MARKET_OPEN;
	p->packetLength = sizeof( *p );

	for( int i = 0, count = 0; i < nd.u.shop.count; i++ ){
		struct npc_item_list *item = &nd.u.shop.shop_item[i];

		if( !item->nameid ){
			continue;
		}

		std::shared_ptr<item_data> id = item_db.find(item->nameid);

		if( !id ){
			continue;
		}

		PACKET_ZC_NPC_MARKET_OPEN_sub& entry = p->list[count];

		entry.nameid = client_nameid( item->nameid );
		entry.type = itemtype( item->nameid );
		entry.price = item->value;
		entry.qty = item->qty;
		entry.weight = id->weight;
#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103
		entry.location = pc_equippoint_sub( &sd, id.get() );
#endif

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
		count++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );

	sd.state.trading = 1;
#endif
}


/// Closes the Market shop window.
void clif_parse_NPCMarketClosed(int fd, map_session_data *sd) {
	nullpo_retv(sd);
	sd->npc_shopid = 0;
	sd->state.trading = 0;
}


/// Purchase item from Market shop.
/// 0x9d7 <packet len>.W <count>.B { <name id>.W <qty>.W <price>.L }* (ZC_NPC_MARKET_PURCHASE_RESULT)
void clif_npc_market_purchase_ack( map_session_data& sd, e_purchase_result res, std::vector<s_npc_buy_list>& list ){
#if PACKETVER >= 20131223
	struct npc_data *nd = map_id2nd( sd.npc_shopid );

	if( nd == nullptr ){
		return;
	}

	PACKET_ZC_NPC_MARKET_PURCHASE_RESULT* p = reinterpret_cast<PACKET_ZC_NPC_MARKET_PURCHASE_RESULT*>( packet_buffer );

	p->PacketType = HEADER_ZC_NPC_MARKET_PURCHASE_RESULT;
	p->PacketLength = sizeof( *p );

#if PACKETVER_MAIN_NUM >= 20190807 || PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190814
	p->result = ( res == e_purchase_result::PURCHASE_SUCCEED ? 0 : -1 );
#else
	p->result = ( res == e_purchase_result::PURCHASE_SUCCEED ? 1 : 0 );
#endif

	if( res == e_purchase_result::PURCHASE_SUCCEED ){
		for( int i = 0, j, count = 0; i < list.size(); i++ ){
			ARR_FIND( 0, nd->u.shop.count, j, list[i].nameid == nd->u.shop.shop_item[j].nameid );

			// Not found
			if( j == nd->u.shop.count ){
				continue;
			}

			p->list[count].ITID = client_nameid( list[i].nameid );
			p->list[count].qty = list[i].qty;
			p->list[count].price = nd->u.shop.shop_item[j].value;
			p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof( p->list[0] ) );
			count++;
		}
	}

	clif_send( p, p->PacketLength, &sd.bl, SELF );
#endif
}


/// Purchase item from Market shop.
/// 0x9d6 <len>.W { <name id>.W <qty>.L } (CZ_NPC_MARKET_PURCHASE)
void clif_parse_NPCMarketPurchase(int fd, map_session_data *sd) {
#if PACKETVER >= 20131223
	if( sd == nullptr ){
		return;
	}

	if( !sd->npc_shopid ){
		return;
	}

	const PACKET_CZ_NPC_MARKET_PURCHASE* p = reinterpret_cast<PACKET_CZ_NPC_MARKET_PURCHASE*>( RFIFOP( fd, 0 ) );

	int count = ( p->PacketLength - sizeof( *p ) ) / sizeof( p->list[0] );

	std::vector<s_npc_buy_list> items;

	items.reserve( count );

	for( int i = 0; i < count; i++ ){
		s_npc_buy_list item = {};

		item.nameid = p->list[i].ITID;
		item.qty = p->list[i].qty;

		items.push_back( item );
	}

	e_purchase_result res = npc_buylist( sd, items );
	clif_npc_market_purchase_ack( *sd, res, items );
#endif
}


/// Displays an NPC dialog message.
/// 00b4 <packet len>.W <npc id>.L <message>.?B (ZC_SAY_DIALOG)
/// Client behavior (dialog window):
/// - disable mouse targeting
/// - open the dialog window
/// - set npcid of dialog window (0 by default)
/// - if set to clear on next mes, clear contents
/// - append this text
void clif_scriptmes( map_session_data& sd, uint32 npcid, const char *mes ){
	PACKET_ZC_SAY_DIALOG* p = reinterpret_cast<PACKET_ZC_SAY_DIALOG*>( packet_buffer );

	int16 length = (int16)( strlen( mes ) + 1 );

	p->PacketType = HEADER_ZC_SAY_DIALOG;
	p->PacketLength = sizeof( *p ) + length;
	p->NpcID = npcid;
	safestrncpy( p->message, mes, length );

	clif_send( p, p->PacketLength, &sd.bl, SELF );
}


/// Adds a 'next' button to an NPC dialog.
/// 00b5 <npc id>.L (ZC_WAIT_DIALOG)
/// Client behavior (dialog window):
/// - disable mouse targeting
/// - open the dialog window
/// - add 'next' button
/// When 'next' is pressed:
/// - 00B9 <npcid of dialog window>.L
/// - set to clear on next mes
/// - remove 'next' button
void clif_scriptnext( map_session_data& sd, uint32 npcid ){
	PACKET_ZC_WAIT_DIALOG p = {};

	p.PacketType = HEADER_ZC_WAIT_DIALOG;
	p.NpcID = npcid;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Adds a 'close' button to an NPC dialog.
/// 00b6 <npc id>.L (ZC_CLOSE_DIALOG)
/// Client behavior:
/// - if dialog window is open:
///   - remove 'next' button
///   - add 'close' button
/// - else:
///   - enable mouse targeting
///   - close the dialog window
///   - close the menu window
/// When 'close' is pressed:
/// - enable mouse targeting
/// - close the dialog window
/// - close the menu window
/// - 0146 <npcid of dialog window>.L
void clif_scriptclose( map_session_data& sd, uint32 npcid ){
	PACKET_ZC_CLOSE_DIALOG packet{};

	packet.packetType = HEADER_ZC_CLOSE_DIALOG;
	packet.npcId = npcid;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/**
 * Close script when player is idle
 * 08d6 <npc id>.L (ZC_CLEAR_DIALOG)
 * @author [Ind/Hercules]
 * @param sd : player pointer
 * @param npcid : npc gid to close
 */
void clif_scriptclear( map_session_data& sd, int npcid ){
	PACKET_ZC_CLEAR_DIALOG p = {};

	p.packetType = HEADER_ZC_CLEAR_DIALOG;
	p.GID = npcid;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
 }


void clif_sendfakenpc( map_session_data& sd, uint32 npcid ){
	sd.state.using_fake_npc = 1;

	// Code below is a copy of clif_set_unit_idle
	struct packet_idle_unit p;

	p.PacketType = idle_unitType;
#if PACKETVER >= 20091103
	p.PacketLength = sizeof(p);
	p.objecttype = 0x1; // NPC_TYPE
#endif
#if PACKETVER >= 20131223
	p.AID = npcid;
	p.GID = 0;
#else
	p.GID = npcid;
#endif
	p.speed = 0;
	p.bodyState = 0;
	p.healthState = 0;
	p.effectState = 0;
	p.job = JT_HIDDEN_NPC;
	p.head = 0;
	p.weapon = 0;
#if PACKETVER < 7 || PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	p.shield = 0;
#endif
	p.accessory = 0;
	p.accessory2 = 0;
	p.accessory3 = 0;
	p.headpalette = 0;
	p.bodypalette = 0;
	p.headDir = 0;
#if PACKETVER >= 20101124
	p.robe = 0;
#endif
	p.GUID = 0;
	p.GEmblemVer = 0;
	p.honor = 0;
	p.virtue = 0;
	p.isPKModeON = 0;
	p.sex = SEX_FEMALE;
	WBUFPOS( &p.PosDir[0], 0, sd.bl.x, sd.bl.y, sd.ud.dir );
	p.xSize = p.ySize = 5;
	p.state = 0;
	p.clevel = 0;
#if PACKETVER >= 20080102
	p.font = 0;
#endif
#if PACKETVER >= 20120221
	p.maxHP = -1;
	p.HP = -1;
#endif
#if PACKETVER >= 20150513
	p.body = 0;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	safestrncpy( p.name, "", NAME_LENGTH);
#endif

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Displays an NPC dialog menu.
/// 00b7 <packet len>.W <npc id>.L <menu items>.?B (ZC_MENU_LIST)
/// Client behavior:
/// - disable mouse targeting
/// - close the menu window
/// - open the menu window
/// - add options to the menu (separated in the text by ":")
/// - set npcid of menu window
/// - if dialog window is open:
///   - remove 'next' button
/// When 'ok' is pressed:
/// - 00B8 <npcid of menu window>.L <selected option>.B
/// - close the menu window
/// When 'cancel' is pressed:
/// - 00B8 <npcid of menu window>.L <-1>.B
/// - enable mouse targeting
/// - close a bunch of windows...
/// WARNING: the 'cancel' button closes other windows besides the dialog window and the menu window.
///    Which suggests their have intertwined behavior. (probably the mouse targeting)
/// TODO investigate behavior of other windows [FlavioJS]
void clif_scriptmenu( map_session_data& sd, uint32 npcid, const char* mes ){
	struct block_list *bl = nullptr;
	
	if (!sd.state.using_fake_npc && (npcid == fake_nd->bl.id || ((bl = map_id2bl(npcid)) && (bl->m!=sd.bl.m ||
	   bl->x<sd.bl.x-AREA_SIZE-1 || bl->x>sd.bl.x+AREA_SIZE+1 ||
	   bl->y<sd.bl.y-AREA_SIZE-1 || bl->y>sd.bl.y+AREA_SIZE+1))))
	   clif_sendfakenpc( sd, npcid );

	// String length + 1 byte for zero termination
	size_t mes_length = strlen( mes ) + 1;

	PACKET_ZC_MENU_LIST* packet = reinterpret_cast<PACKET_ZC_MENU_LIST*>( packet_buffer );

	packet->packetType = HEADER_ZC_MENU_LIST;
	packet->packetLength = sizeof( PACKET_ZC_MENU_LIST );
	packet->npcId = npcid;
	safestrncpy( packet->menu, mes, mes_length );
	packet->packetLength += static_cast<decltype(packet->packetLength)>( mes_length );

	clif_send( packet, packet->packetLength, &sd.bl, SELF );
}


/// Displays an NPC dialog input box for numbers.
/// 0142 <npc id>.L (ZC_OPEN_EDITDLG)
/// Client behavior (inputnum window):
/// - if npcid exists in the client:
///   - open the inputnum window
///   - set npcid of inputnum window
/// When 'ok' is pressed:
/// - if inputnum window has text:
///   - if npcid exists in the client:
///     - 0143 <npcid of inputnum window>.L <atoi(text)>.L
///   - close inputnum window
void clif_scriptinput( map_session_data& sd, uint32 npcid ){
	struct block_list *bl = nullptr;

	if (!sd.state.using_fake_npc && (npcid == fake_nd->bl.id || ((bl = map_id2bl(npcid)) && (bl->m!=sd.bl.m ||
	   bl->x<sd.bl.x-AREA_SIZE-1 || bl->x>sd.bl.x+AREA_SIZE+1 ||
	   bl->y<sd.bl.y-AREA_SIZE-1 || bl->y>sd.bl.y+AREA_SIZE+1))))
	   clif_sendfakenpc( sd, npcid );

	PACKET_ZC_OPEN_EDITDLG packet{};

	packet.packetType = HEADER_ZC_OPEN_EDITDLG;
	packet.npcId = npcid;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Displays an NPC dialog input box for strings.
/// 01d4 <npc id>.L (ZC_OPEN_EDITDLGSTR)
/// Client behavior (inputstr window):
/// - if npcid is 0 or npcid exists in the client:
///   - open the inputstr window
///   - set npcid of inputstr window
/// When 'ok' is pressed:
/// - if inputstr window has text and isn't an insult(manner.txt):
///   - if npcid is 0 or npcid exists in the client:
///     - 01d5 <packetlen>.W <npcid of inputstr window>.L <text>.?B
///   - close inputstr window
void clif_scriptinputstr( map_session_data& sd, uint32 npcid ){
	struct block_list *bl = nullptr;

	if (!sd.state.using_fake_npc && (npcid == fake_nd->bl.id || ((bl = map_id2bl(npcid)) && (bl->m!=sd.bl.m ||
	   bl->x<sd.bl.x-AREA_SIZE-1 || bl->x>sd.bl.x+AREA_SIZE+1 ||
	   bl->y<sd.bl.y-AREA_SIZE-1 || bl->y>sd.bl.y+AREA_SIZE+1))))
	   clif_sendfakenpc( sd, npcid );

	PACKET_ZC_OPEN_EDITDLGSTR packet{};

	packet.packetType = HEADER_ZC_OPEN_EDITDLGSTR;
	packet.npcId = npcid;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Marks a position on client's minimap.
/// 0144 <npc id>.L <type>.L <x>.L <y>.L <id>.B <color>.L (ZC_COMPASS)
/// npc id:
///     is ignored in the client
/// type:
///     0 = display mark for 15 seconds
///     1 = display mark until dead or teleported
///     2 = remove mark
/// color:
///     0x00RRGGBB
void clif_viewpoint( map_session_data& sd, uint32 npc_id, int type, uint16 x, uint16 y, int id, uint32 color ){
	PACKET_ZC_COMPASS packet{};

	packet.packetType = HEADER_ZC_COMPASS;
	packet.npcId = npc_id;
	packet.type = type;
	packet.xPos = x;
	packet.yPos = y;
	packet.id = id;
	packet.color = color;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Displays an illustration image.
/// 0145 <image name>.16B <type>.B (ZC_SHOW_IMAGE)
/// 01b3 <image name>.64B <type>.B (ZC_SHOW_IMAGE2)
/// type:
///     0 = bottom left corner
///     1 = bottom middle
///     2 = bottom right corner
///     3 = middle of screen, inside a movable window
///     4 = middle of screen, movable with a close button, chrome-less
///   255 = clear all displayed cutins
void clif_cutin( map_session_data& sd, const char* image, int type ){
	PACKET_ZC_SHOW_IMAGE packet{};

	packet.packetType = HEADER_ZC_SHOW_IMAGE;
	safestrncpy(packet.image, image, sizeof(packet.image));
	packet.type = type;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/*==========================================
 * Fills in card data from the given item and into the buffer. [Skotlex]
 *------------------------------------------*/
static void clif_addcards( struct EQUIPSLOTINFO* buf, struct item* item ){
	nullpo_retv( buf );

	// Blank data
	if( item == nullptr ){
		buf->card[0] = 0;
		buf->card[1] = 0;
		buf->card[2] = 0;
		buf->card[3] = 0;
		return;
	}

	// Pet eggs
	if( item->card[0] == CARD0_PET ){
		buf->card[0] = 0;
		buf->card[1] = 0;
		// Pet intimacy
		// No idea when this was added exactly, but older clients have no problem if we send it anyway
		buf->card[2] = item->card[3] >> 1;
		// Pet renamed flag
		buf->card[3] = item->card[3] & 1;
		return;
	}

	// Forged/created items
	if( item->card[0] == CARD0_FORGE || item->card[0] == CARD0_CREATE ){
		buf->card[0] = item->card[0];
		buf->card[1] = item->card[1];
		buf->card[2] = item->card[2];
		buf->card[3] = item->card[3];
		return;
	}

	int i = 0, j;

	// Client only receives four cards.. so randomly send them a set of cards. [Skotlex]
	if( MAX_SLOTS > 4 && ( j = itemdb_slots( item->nameid ) ) > 4 ){
		i = rnd() % ( j - 3 ); //eg: 6 slots, possible i values: 0->3, 1->4, 2->5 => i = rnd()%3;
	}

	// Normal items.
	for( int k = 0; k < 4; k++, i++ ){
		if( item->card[i] > 0 ){
			buf->card[k] = client_nameid( item->card[i] );
		}else{
			buf->card[k] = 0;
		}
	}
}

/// Fills in part of the item buffers that calls for variable bonuses data. [Napster]
/// A maximum of 5 random options can be supported.
static void clif_add_random_options( struct ItemOptions buf[MAX_ITEM_RDM_OPT], struct item& it ){
	for( int i = 0; i < MAX_ITEM_RDM_OPT; i++ ){
		if( it.option[i].id ){
			buf[i].index = it.option[i].id; // OptIndex
			buf[i].value = it.option[i].value; // Value
			buf[i].param = it.option[i].param; // Param1
		}else{
			buf[i].index = 0;
			buf[i].value = 0;
			buf[i].param = 0;
		}
	}
	
#if MAX_ITEM_RDM_OPT < 5
	for( ; i < 5; i++ ){
		buf[i].index = 0;	// OptIndex
		buf[i].value = 0;	// Value
		buf[i].param = 0;	// Param1
	}
#endif
}

/// Notifies the client, about a received inventory item or the result of a pick-up request.
/// 00a0 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B <result>.B (ZC_ITEM_PICKUP_ACK)
/// 029a <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B <result>.B <expire time>.L (ZC_ITEM_PICKUP_ACK2)
/// 02d4 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W (ZC_ITEM_PICKUP_ACK3)
/// 0990 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.L <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W (ZC_ITEM_PICKUP_ACK_V5)
/// 0a0c <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.L <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W { <option id>.W <option value>.W <option param>.B }*5 (ZC_ITEM_PICKUP_ACK_V6)
/// 0a37 <index>.W <amount>.W <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.L <item type>.B <result>.B <expire time>.L <bindOnEquipType>.W { <option id>.W <option value>.W <option param>.B }*5 <favorite>.B <view id>.W (ZC_ITEM_PICKUP_ACK_V7)
void clif_additem( map_session_data *sd, int n, int amount, unsigned char fail ){
	nullpo_retv( sd );

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	PACKET_ZC_ITEM_PICKUP_ACK p = {};

	if( fail ){
		p = {};
	}else{
		if( n < 0 || n >= MAX_INVENTORY || sd->inventory.u.items_inventory[n].nameid == 0 || sd->inventory_data[n] == nullptr ){
			return;
		}

		p.nameid = client_nameid( sd->inventory.u.items_inventory[n].nameid );
		p.IsIdentified = sd->inventory.u.items_inventory[n].identify ? 1 : 0;
		p.IsDamaged = ( sd->inventory.u.items_inventory[n].attribute ) != 0 ? 1 : 0;
		p.refiningLevel =sd->inventory.u.items_inventory[n].refine;
		clif_addcards( &p.slot, &sd->inventory.u.items_inventory[n] );
		p.location = pc_equippoint(sd,n);
		p.type = itemtype( sd->inventory.u.items_inventory[n].nameid );
#if PACKETVER >= 20061218
		p.HireExpireDate = sd->inventory.u.items_inventory[n].expire_time;
#if PACKETVER >= 20071002
		/* why restrict the flag to non-stackable? because this is the only packet allows stackable to,
		 * show the color, and therefore it'd be inconsistent with the rest (aka it'd show yellow, you relog/refresh and boom its gone)
		 */
		p.bindOnEquipType = sd->inventory.u.items_inventory[n].bound && !itemdb_isstackable2( sd->inventory_data[n] ) ? 2 : sd->inventory_data[n]->flag.bindOnEquip ? 1 : 0;
#if PACKETVER >= 20150226
		clif_add_random_options( p.option_data, sd->inventory.u.items_inventory[n] );
#if PACKETVER >= 20160921
		p.favorite = ( sd->inventory.u.items_inventory[n].favorite != 0 ) ? 1 : 0;
		p.look = sd->inventory_data[n]->look;
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
		p.grade = sd->inventory.u.items_inventory[n].enchantgrade;
#endif
#endif
#endif
#endif
#endif
	}

	p.PacketType = HEADER_ZC_ITEM_PICKUP_ACK;
	p.Index = client_index( n );
	p.count = amount;
	p.result = fail;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Notifies the client, that an inventory item was deleted or dropped.
/// 00af <index>.W <amount>.W (ZC_ITEM_THROW_ACK)
void clif_dropitem( map_session_data& sd, int index, int amount ){
	PACKET_ZC_ITEM_THROW_ACK packet{};

	packet.packetType = HEADER_ZC_ITEM_THROW_ACK;
	packet.index = client_index( index );
	packet.count = amount;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client, that an inventory item was deleted.
/// 07fa <delete type>.W <index>.W <amount>.W (ZC_DELETE_ITEM_FROM_BODY)
/// delete type:
///     0 = Normal
///     1 = Item used for a skill
///     2 = Refine failed
///     3 = Material changed
///     4 = Moved to storage
///     5 = Moved to cart
///     6 = Item sold
///     7 = Consumed by Four Spirit Analysis (SO_EL_ANALYSIS) skill
void clif_delitem( map_session_data& sd, int index, int amount, short reason ){
#if PACKETVER >= 20091117
	PACKET_ZC_DELETE_ITEM_FROM_BODY packet{};

	packet.packetType = HEADER_ZC_DELETE_ITEM_FROM_BODY;
	packet.deleteType = reason;
	packet.index = client_index( index );
	packet.count = amount;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
#else
	clif_dropitem( sd, index, amount );
#endif
}

static void clif_item_equip( short idx, struct EQUIPITEM_INFO *p, struct item *it, struct item_data *id, int eqp_pos ){
	nullpo_retv( p );
	nullpo_retv( it );
	nullpo_retv( id );

	p->index = idx;

	p->ITID = client_nameid( it->nameid );

	p->type = itemtype( it->nameid );

#if PACKETVER < 20120925
	p->IsIdentified = it->identify ? 1 : 0;
#endif

	p->location = eqp_pos;
	p->WearState = it->equip;
#if PACKETVER < 20120925
	p->IsDamaged = it->attribute != 0 ? 1 : 0;
#endif
	p->RefiningLevel = it->refine;

	clif_addcards( &p->slot, it );

#if PACKETVER >= 20071002
	p->HireExpireDate = it->expire_time;
#endif

#if PACKETVER >= 20080102
	p->bindOnEquipType = it->bound ? 2 : id->flag.bindOnEquip ? 1 : 0;
#endif

#if PACKETVER >= 20100629
	// TODO: WBUFW(buf,n+8) = (equip == -2 && id->equip == EQP_AMMO) ? id->equip : 0;
	p->wItemSpriteNumber = ( id->equip&EQP_VISIBLE ) ? id->look : 0;
#endif

#if PACKETVER >= 20120925
	p->Flag.IsIdentified = it->identify ? 1 : 0;
	p->Flag.IsDamaged = it->attribute ? 1 : 0;
	p->Flag.PlaceETCTab = it->favorite ? 1 : 0;
	p->Flag.SpareBits = 0;
#endif

#if PACKETVER >= 20150226
	clif_add_random_options( p->option_data, *it );
	// Client ingores the value anyway and recalculates it
	p->option_count = 0;
#endif

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	p->grade = it->enchantgrade;
#endif
}

static void clif_item_normal( short idx, struct NORMALITEM_INFO *p, struct item *i, struct item_data *id ){
	nullpo_retv(p);
	nullpo_retv(i);
	nullpo_retv(id);

	p->index = idx;

	p->ITID = client_nameid( i->nameid );

	p->type = itemtype( i->nameid );

#if PACKETVER < 20120925
	p->IsIdentified = i->identify ? 1 : 0;
#endif

	p->count = i->amount;
	p->WearState = id->equip;

#if PACKETVER >= 5
	clif_addcards( &p->slot, i );
#endif

#if PACKETVER >= 20080102
	p->HireExpireDate = i->expire_time;
#endif

#if PACKETVER >= 20120925
	p->Flag.IsIdentified = i->identify ? 1 : 0;
	p->Flag.PlaceETCTab  = ( i->favorite != 0 ) ? 1 : 0;
	p->Flag.SpareBits    = 0;
#endif
}

static void clif_inventoryStart( map_session_data*sd, e_inventory_type type, const char *name ){
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	nullpo_retv(sd);
	nullpo_retv(name);

	char buf[sizeof(PACKET_ZC_INVENTORY_START) + 24];
	memset(buf, 0, sizeof(buf));
	PACKET_ZC_INVENTORY_START *p = (PACKET_ZC_INVENTORY_START *)buf;
	p->packetType = HEADER_ZC_INVENTORY_START;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	p->invType = type;
#endif
#if PACKETVER_RE_NUM >= 20180919 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	int strLen = (int)safestrnlen(name, 24) + 1;
	if (strLen > 24)
		strLen = 24;
	const int len = sizeof(PACKET_ZC_INVENTORY_START) + strLen;
	p->packetLength = len;
	safestrncpy(p->name, name, strLen);
#else
	const int len = sizeof(PACKET_ZC_INVENTORY_START);
	safestrncpy(p->name, name, NAME_LENGTH);
#endif
	clif_send( p, len, &sd->bl, SELF );
#endif
}

static void clif_inventoryEnd( map_session_data *sd, e_inventory_type type ){
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	nullpo_retv(sd);

	PACKET_ZC_INVENTORY_END p = {};
	p.packetType = HEADER_ZC_INVENTORY_END;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	p.invType = type;
#endif
	p.flag = 0;
	clif_send( &p, sizeof(p), &sd->bl, SELF );
#endif
}

//Unified inventory function which sends all of the inventory (requires two packets, one for equipable items and one for stackable ones. [Skotlex]
void clif_inventorylist( map_session_data *sd ){
	nullpo_retv( sd );

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	e_inventory_type type = INVTYPE_INVENTORY;

	clif_inventoryStart( sd, type, "" );
#endif
	static struct packet_itemlist_normal itemlist_normal;
	static struct packet_itemlist_equip itemlist_equip;
	int equip = 0;
	int normal = 0;

	for( int i = 0; i < MAX_INVENTORY; i++ ){
		if( sd->inventory.u.items_inventory[i].nameid == 0 || sd->inventory_data[i] == nullptr ){
			continue;
		}

		// Non-stackable (Equippable)
		if( !itemdb_isstackable2( sd->inventory_data[i] ) ){
			clif_item_equip( client_index( i ), &itemlist_equip.list[equip++], &sd->inventory.u.items_inventory[i], sd->inventory_data[i], pc_equippoint( sd, i ) );

			if( equip == MAX_INVENTORY_ITEM_PACKET_NORMAL ){
				itemlist_equip.PacketType  = inventorylistequipType;
				itemlist_equip.PacketLength = static_cast<decltype(itemlist_equip.PacketLength)>( ( sizeof( itemlist_equip ) - sizeof( itemlist_equip.list ) ) + ( sizeof( struct EQUIPITEM_INFO ) * equip ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
				itemlist_equip.invType = type;
#endif

				clif_send( &itemlist_equip, itemlist_equip.PacketLength, &sd->bl, SELF );

				equip = 0;
			}
		// Stackable (Normal)
		}else{
			clif_item_normal( client_index( i ), &itemlist_normal.list[normal++], &sd->inventory.u.items_inventory[i], sd->inventory_data[i] );

			if( normal == MAX_INVENTORY_ITEM_PACKET_NORMAL ){
				itemlist_normal.PacketType = inventorylistnormalType;
				itemlist_normal.PacketLength = static_cast<decltype(itemlist_normal.PacketLength)>( ( sizeof( itemlist_normal ) - sizeof( itemlist_normal.list ) ) + ( sizeof( struct NORMALITEM_INFO ) * normal ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
				itemlist_normal.invType = type;
#endif

				clif_send( &itemlist_normal, itemlist_normal.PacketLength, &sd->bl, SELF );

				normal = 0;
			}
		}
	}

	if( normal ){
		itemlist_normal.PacketType = inventorylistnormalType;
		itemlist_normal.PacketLength = static_cast<decltype(itemlist_normal.PacketLength)>( ( sizeof( itemlist_normal ) - sizeof( itemlist_normal.list ) ) + ( sizeof( struct NORMALITEM_INFO ) * normal ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_normal.invType = type;
#endif

		clif_send( &itemlist_normal, itemlist_normal.PacketLength, &sd->bl, SELF );
	}

	if( sd->equip_index[EQI_AMMO] >= 0 )
		clif_arrowequip( *sd );

	if( equip ) {
		itemlist_equip.PacketType  = inventorylistequipType;
		itemlist_equip.PacketLength = static_cast<decltype(itemlist_equip.PacketLength)>( ( sizeof( itemlist_equip ) - sizeof( itemlist_equip.list ) ) + ( sizeof( struct EQUIPITEM_INFO ) * equip ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_equip.invType = type;
#endif

		clif_send( &itemlist_equip, itemlist_equip.PacketLength, &sd->bl, SELF );
	}

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif_inventoryEnd( sd, type );
#endif
/* on 20120925 onwards this is a field on clif_item_equip/normal */
#if PACKETVER >= 20111122 && PACKETVER < 20120925
	for( int i = 0; i < MAX_INVENTORY; i++ ){
		if( sd->inventory.u.items_inventory[i].nameid == 0 || sd->inventory_data[i] == nullptr )
			continue;

		if ( sd->inventory.u.items_inventory[i].favorite != 0 )
			clif_favorite_item( *sd, i );
	}
#endif
}

//Required when items break/get-repaired. Only sends equippable item list.
void clif_equiplist( map_session_data *sd ){
	// TODO: implement again => only send equip part
	clif_inventorylist( sd );
}

void clif_storagelist(map_session_data* sd, struct item* items, int items_length, const char *storename){
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	e_inventory_type type = INVTYPE_STORAGE;

	clif_inventoryStart( sd, type, storename );
#endif
	static struct ZC_STORE_ITEMLIST_NORMAL storage_itemlist_normal;
	static struct ZC_STORE_ITEMLIST_EQUIP storage_itemlist_equip;
	int equip = 0;
	int normal = 0;

	for( int i = 0; i < items_length; i++ ){
		if( items[i].nameid == 0 ){
			continue;
		}

		struct item_data* id = itemdb_search( items[i].nameid );

		// Non-stackable (Equippable)
		if( !itemdb_isstackable2( id ) ){
			clif_item_equip( client_storage_index( i ), &storage_itemlist_equip.list[equip++], &items[i], id, pc_equippoint_sub( sd, id ) );

			if( equip == MAX_STORAGE_ITEM_PACKET_EQUIP ){
				storage_itemlist_equip.PacketType  = storageListEquipType;
				storage_itemlist_equip.PacketLength = static_cast<decltype(storage_itemlist_equip.PacketLength)>( ( sizeof( storage_itemlist_equip ) - sizeof( storage_itemlist_equip.list ) ) + ( sizeof( struct EQUIPITEM_INFO ) * equip ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
				storage_itemlist_equip.invType = type;
#elif PACKETVER >= 20120925
				safestrncpy( storage_itemlist_equip.name, storename, NAME_LENGTH );
#endif

				clif_send( &storage_itemlist_equip, storage_itemlist_equip.PacketLength, &sd->bl, SELF );

				equip = 0;
			}
		// Stackable (Normal)
		}else{
			clif_item_normal( client_storage_index( i ), &storage_itemlist_normal.list[normal++], &items[i], id );

			if( normal == MAX_STORAGE_ITEM_PACKET_NORMAL ){
				storage_itemlist_normal.PacketType = storageListNormalType;
				storage_itemlist_normal.PacketLength = static_cast<decltype(storage_itemlist_normal.PacketLength)>( ( sizeof( storage_itemlist_normal ) - sizeof( storage_itemlist_normal.list ) ) + ( sizeof( struct NORMALITEM_INFO ) * normal ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
				storage_itemlist_normal.invType = type;
#elif PACKETVER >= 20120925
				safestrncpy( storage_itemlist_normal.name, storename, NAME_LENGTH );
#endif

				clif_send( &storage_itemlist_normal, storage_itemlist_normal.PacketLength, &sd->bl, SELF );

				normal = 0;
			}
		}
	}

	if( normal ){
		storage_itemlist_normal.PacketType = storageListNormalType;
		storage_itemlist_normal.PacketLength = static_cast<decltype(storage_itemlist_normal.PacketLength)>( ( sizeof( storage_itemlist_normal ) - sizeof( storage_itemlist_normal.list ) ) + ( sizeof( struct NORMALITEM_INFO ) * normal ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		storage_itemlist_normal.invType = type;
#elif PACKETVER >= 20120925
		safestrncpy( storage_itemlist_normal.name, storename, NAME_LENGTH );
#endif

		clif_send( &storage_itemlist_normal, storage_itemlist_normal.PacketLength, &sd->bl, SELF );
	}

	if( equip ) {
		storage_itemlist_equip.PacketType  = storageListEquipType;
		storage_itemlist_equip.PacketLength = static_cast<decltype(storage_itemlist_equip.PacketLength)>( ( sizeof( storage_itemlist_equip ) - sizeof( storage_itemlist_equip.list ) ) + ( sizeof( struct EQUIPITEM_INFO ) * equip ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		storage_itemlist_equip.invType = type;
#elif PACKETVER >= 20120925
		safestrncpy( storage_itemlist_equip.name, storename, NAME_LENGTH );
#endif

		clif_send( &storage_itemlist_equip, storage_itemlist_equip.PacketLength, &sd->bl, SELF );
	}

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif_inventoryEnd( sd, type );
#endif
}

void clif_cartlist( map_session_data *sd ){
	nullpo_retv( sd );

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	e_inventory_type type = INVTYPE_CART;

	clif_inventoryStart( sd, type, "" );
#endif
	static struct packet_itemlist_normal itemlist_normal;
	static struct packet_itemlist_equip itemlist_equip;
	int normal = 0;
	int equip = 0;

	for( int i = 0; i < MAX_CART; i++ ){
		if( sd->cart.u.items_cart[i].nameid == 0 ){
			continue;
		}

		struct item_data* id = itemdb_search( sd->cart.u.items_cart[i].nameid );

		// Non-stackable (Equippable)
		if( !itemdb_isstackable2(id) ){
			clif_item_equip( client_index( i ), &itemlist_equip.list[equip++], &sd->cart.u.items_cart[i], id, id->equip );
		 // Stackable (Normal)
		}else{
			clif_item_normal( client_index( i ), &itemlist_normal.list[normal++], &sd->cart.u.items_cart[i], id );
		}
	}

	if( normal ){
		itemlist_normal.PacketType = cartlistnormalType;
		itemlist_normal.PacketLength = static_cast<decltype(itemlist_normal.PacketLength)>( ( sizeof( itemlist_normal ) - sizeof( itemlist_normal.list ) ) + ( sizeof( struct NORMALITEM_INFO ) * normal ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_normal.invType = type;
#endif

		clif_send( &itemlist_normal, itemlist_normal.PacketLength, &sd->bl, SELF );
	}

	if( equip ){
		itemlist_equip.PacketType = cartlistequipType;
		itemlist_equip.PacketLength = static_cast<decltype(itemlist_equip.PacketLength)>( ( sizeof( itemlist_equip ) - sizeof( itemlist_equip.list ) ) + ( sizeof( struct EQUIPITEM_INFO ) * equip ) );
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
		itemlist_equip.invType = type;
#endif

		clif_send( &itemlist_equip, itemlist_equip.PacketLength, &sd->bl, SELF );
	}

#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	clif_inventoryEnd( sd, type );
#endif
}


/// Removes cart.
/// 012b (ZC_CARTOFF)
/// Client behaviour:
/// Closes the cart storage and removes all it's items from memory.
/// The Num & Weight values of the cart are left untouched and the cart is NOT removed.
void clif_clearcart(int fd)
{
	PACKET_ZC_CARTOFF packet{};

	packet.packetType = HEADER_ZC_CARTOFF;

	socket_send<PACKET_ZC_CARTOFF>(fd, packet);
}


/// Sends XY location to all other guild members
/// 01eb <account id>.L <x>.W <y>.W (ZC_NOTIFY_POSITION_TO_GUILDM)
void clif_guild_xy( map_session_data& sd ){
	PACKET_ZC_NOTIFY_POSITION_TO_GUILDM packet{};

	packet.packetType = HEADER_ZC_NOTIFY_POSITION_TO_GUILDM;
	packet.aid = sd.status.account_id;
	packet.xPos = sd.bl.x;
	packet.yPos = sd.bl.y;

	clif_send(&packet, sizeof(packet), &sd.bl, GUILD_SAMEMAP_WOS);
}

/// Sends XY location to a specific guild member
/// 01eb <account id>.L <x>.W <y>.W (ZC_NOTIFY_POSITION_TO_GUILDM)
void clif_guild_xy_single( map_session_data& sd, map_session_data& tsd ){
	if( sd.bg_id ){
		return;
	}

	PACKET_ZC_NOTIFY_POSITION_TO_GUILDM packet{};

	packet.packetType = HEADER_ZC_NOTIFY_POSITION_TO_GUILDM;
	packet.aid = sd.status.account_id;
	packet.xPos = sd.bl.x;
	packet.yPos = sd.bl.y;

	clif_send( &packet, sizeof( packet ), &tsd.bl, SELF );
}

/// Removes XY location from all other guild members
/// 01eb <account id>.L <x>.W <y>.W (ZC_NOTIFY_POSITION_TO_GUILDM)
void clif_guild_xy_remove( map_session_data& sd ){
	PACKET_ZC_NOTIFY_POSITION_TO_GUILDM packet{};

	packet.packetType = HEADER_ZC_NOTIFY_POSITION_TO_GUILDM;
	packet.aid = sd.status.account_id;
	packet.xPos = -1;
	packet.yPos = -1;

	clif_send( &packet, sizeof( packet ), &sd.bl, GUILD_SAMEMAP_WOS );
}

/*==========================================
 * Load castle list for guild UI. [Asheraf] / [Balfear]
 *------------------------------------------*/
void clif_guild_castle_list(map_session_data& sd){
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814
	auto &g = sd.guild;

	if (g == nullptr)
		return;

	int castle_count = guild_checkcastles(g->guild);

	if( castle_count <= 0 ){
		return;
	}

	PACKET_ZC_GUILD_AGIT_INFO* p = reinterpret_cast<PACKET_ZC_GUILD_AGIT_INFO*>( packet_buffer );

	p->packetType = HEADER_ZC_GUILD_AGIT_INFO;
	p->packetLength = sizeof( *p );

	int i = 0;
	for (const auto& gc : castle_db) {
		if (gc.second->guild_id == g->guild.guild_id && gc.second->client_id) {
			p->castle_list[i] = static_cast<int8>( gc.second->client_id );

			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->castle_list[0] ) );
			++i;
		}
	}

	clif_send(p, p->packetLength, &sd.bl, SELF);
#endif
}

/*==========================================
 * Send castle info Economy/Defence. [Asheraf] / [Balfear]
 *------------------------------------------*/
void clif_guild_castleinfo(map_session_data& sd, std::shared_ptr<guild_castle> castle ){
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814
	if( castle->client_id == 0 ){
		return;
	}

	PACKET_ZC_REQ_ACK_AGIT_INVESTMENT p = {};

	p.packetType = HEADER_ZC_REQ_ACK_AGIT_INVESTMENT;
	p.castle_id = static_cast<int8>( castle->client_id );
	p.economy = castle->economy;
	p.defense = castle->defense;

	clif_send(&p, sizeof(p), &sd.bl, SELF);
#endif
}

/*==========================================
 * Show teleport request result. [Asheraf] / [Balfear]
 *------------------------------------------*/
void clif_guild_castle_teleport_res(map_session_data& sd, enum e_siege_teleport_result result){
#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814
	PACKET_ZC_REQ_ACK_MOVE_GUILD_AGIT p = {};

	p.packetType = HEADER_ZC_REQ_ACK_MOVE_GUILD_AGIT;
	p.result = static_cast<int16>( result );

	clif_send(&p, sizeof(p), &sd.bl, SELF);
#endif
}

/*==========================================
 * Request castle info. [Asheraf] / [Balfear]
 *------------------------------------------*/
void clif_parse_guild_castle_info_request(int fd, map_session_data* sd){
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190522 || PACKETVER_ZERO_NUM >= 20190515
	if( sd == nullptr ){
		return;
	}

	auto &g = sd->guild;

	if (g == nullptr)
		return;

	const PACKET_CZ_REQ_AGIT_INVESTMENT* p = reinterpret_cast<PACKET_CZ_REQ_AGIT_INVESTMENT*>( RFIFOP( fd, 0 ) );

	std::shared_ptr<guild_castle> gc = castle_db.find_by_clientid( p->castle_id );

	if (gc == nullptr)
		return;
	if (gc->guild_id != g->guild.guild_id)
		return;

	clif_guild_castleinfo(*sd, gc);
#endif
}

/*==========================================
 * Teleport to castle. [Asheraf] / [Balfear]
 *------------------------------------------*/
void clif_parse_guild_castle_teleport_request(int fd, map_session_data* sd){
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190522 || PACKETVER_ZERO_NUM >= 20190515
	const PACKET_CZ_REQ_MOVE_GUILD_AGIT* p = reinterpret_cast<PACKET_CZ_REQ_MOVE_GUILD_AGIT*>RFIFOP( fd, 0 );
	auto &g = sd->guild;

	if (g == nullptr)
		return;

	std::shared_ptr<guild_castle> gc = castle_db.find_by_clientid( p->castle_id );

	if (gc == nullptr)
		return;
	if (!gc->warp_enabled)
		return;
	if (gc->guild_id != g->guild.guild_id)
		return;

	if (map_getmapflag(sd->bl.m, MF_GVG_CASTLE) 
		|| map_getmapflag(sd->bl.m, MF_NOTELEPORT)
		|| map_getmapflag(sd->bl.m, MF_NOWARP))
		return;

	uint32 zeny = gc->zeny;

	switch( gc->type ){
		case WOE_FIRST_EDITION:
			if( agit_flag ){
				zeny = gc->zeny_siege;
			}
			break;
		case WOE_SECOND_EDITION:
			if( agit2_flag ){
				zeny = gc->zeny_siege;
			}
			break;
		case WOE_THIRD_EDITION:
			if( agit3_flag ){
				zeny = gc->zeny_siege;
			}
			break;
	}

	if (zeny && pc_payzeny(sd, zeny, LOG_TYPE_OTHER)) {
		clif_guild_castle_teleport_res(*sd, SIEGE_TP_NOT_ENOUGH_ZENY);
		return;
	}

	pc_setpos(sd, gc->mapindex, gc->warp_x, gc->warp_y, CLR_OUTSIGHT);
#endif
}

/*==========================================
 *
 *------------------------------------------*/
static int clif_hpmeter_sub( struct block_list *bl, va_list ap ){
	map_session_data* sd = va_arg( ap, map_session_data* );
	map_session_data* tsd = BL_CAST( BL_PC, bl );

	nullpo_ret(sd);
	nullpo_ret(tsd);

	if( pc_has_permission( tsd, PC_PERM_VIEW_HPMETER ) ){
		 clif_hpmeter_single( *tsd, sd->status.account_id, sd->battle_status.hp, sd->battle_status.max_hp );
	}
	return 0;
}

/*==========================================
 * Server tells all players that are allowed to view HP bars
 * and are nearby 'sd' that 'sd' hp bar was updated.
 *------------------------------------------*/
static int clif_hpmeter(map_session_data *sd)
{
	nullpo_ret(sd);
	map_foreachinallarea(clif_hpmeter_sub, sd->bl.m, sd->bl.x-AREA_SIZE, sd->bl.y-AREA_SIZE, sd->bl.x+AREA_SIZE, sd->bl.y+AREA_SIZE, BL_PC, sd);
	return 0;
}

/**
 * Send HP bar update to others.
 * @param sd: Player invoking update
 */
void clif_update_hp(map_session_data &sd) {
	if (map_getmapdata(sd.bl.m)->hpmeter_visible)
		clif_hpmeter(&sd);
	if (!battle_config.party_hp_mode && sd.status.party_id)
		clif_party_hp( sd );
	if (sd.bg_id)
		clif_bg_hp(&sd);
}

/// Notifies client of a character parameter change.
/// 00b0 <var id>.W <value>.L (ZC_PAR_CHANGE)
static void clif_par_change(map_session_data& sd, uint16 varId, int count) {
	PACKET_ZC_PAR_CHANGE packet{};

	packet.PacketType = HEADER_ZC_PAR_CHANGE;
	packet.varID = varId;
	packet.count = count;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Notifies client of a character parameter change.
/// 00b1 <var id>.W <value>.L (ZC_LONGPAR_CHANGE)
static void clif_longpar_change(map_session_data& sd, uint16 varId, int amount) {
	PACKET_ZC_LONGPAR_CHANGE packet{};

	packet.PacketType = HEADER_ZC_LONGPAR_CHANGE;
	packet.varID = varId;
	packet.amount = amount;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Notifies client of a character parameter change.
/// 00be <status id>.W <value>.B (ZC_STATUS_CHANGE)
static void clif_zc_status_change(map_session_data& sd, uint16 status_id, uint8 value) {
	PACKET_ZC_STATUS_CHANGE packet{};

	packet.PacketType = HEADER_ZC_STATUS_CHANGE;
	packet.statusID = status_id;
	packet.value = value;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Notifies client of a character parameter change.
/// 0121 <current count>.W <max count>.W <current weight>.L <max weight>.L (ZC_NOTIFY_CARTITEM_COUNTINFO)
static void clif_cartcount( map_session_data& sd ){
	PACKET_ZC_NOTIFY_CARTITEM_COUNTINFO packet{};

	packet.PacketType = HEADER_ZC_NOTIFY_CARTITEM_COUNTINFO;
	packet.curCount = sd.cart_num;
	packet.maxCount = MAX_CART;
	packet.curWeight = sd.cart_weight;
	packet.maxWeight = sd.cart_weight_max;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Tells the client how far it is allowed to attack (weapon range)
/// 013a <atk range>.W (ZC_ATTACK_RANGE)
static void clif_attackrange( map_session_data& sd, int16 range ){
	PACKET_ZC_ATTACK_RANGE packet{};

	packet.PacketType = HEADER_ZC_ATTACK_RANGE;
	packet.currentAttRange = range;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Notifies client of a character parameter change.
/// 0141 <status id>.L <base status>.L <plus status>.L (ZC_COUPLESTATUS)
static void clif_couplestatus(map_session_data& sd, uint32 status_type, int32 defaultStatus, int32 plusStatus) {
	PACKET_ZC_COUPLESTATUS packet{};

	packet.PacketType = HEADER_ZC_COUPLESTATUS;
	packet.statusType = status_type;
	packet.defaultStatus = defaultStatus;
	packet.plusStatus = plusStatus;

	clif_send(&packet, sizeof(packet), &sd.bl, SELF);
}

/// Notifies client of a character parameter change.
/// 0acb <var id>.W <value>.Q (ZC_LONGLONGPAR_CHANGE)
static void clif_longlongpar_change(map_session_data& sd, uint16 varId, int64 amount) {
#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
	PACKET_ZC_LONGLONGPAR_CHANGE packet{};

	packet.PacketType = HEADER_ZC_LONGLONGPAR_CHANGE;
	packet.varID = varId;
	packet.amount = amount;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
#endif
}

/// Notifies client of a character parameter change.
void clif_updatestatus( map_session_data& sd, enum _sp type ){
	switch(type){
		case SP_WEIGHT:
			pc_updateweightstatus(&sd);
			clif_par_change(sd, type, sd.weight);
			break;
		case SP_MAXWEIGHT:
			clif_par_change(sd, type, sd.max_weight);
			break;
		case SP_SPEED:
			clif_par_change(sd, type, sd.battle_status.speed);
			break;
		case SP_BASELEVEL:
			clif_par_change(sd, type, sd.status.base_level);
			break;
		case SP_JOBLEVEL:
			clif_par_change(sd, type, sd.status.job_level);
			break;
		case SP_KARMA:
			// Adding this back, I wonder if the client intercepts this - [Lance]
			clif_par_change(sd, type, sd.status.karma);
			break;
		case SP_MANNER:
			clif_par_change(sd, type, sd.status.manner);
			break;
		case SP_STATUSPOINT:
			clif_par_change(sd, type, sd.status.status_point);
			break;
		case SP_SKILLPOINT:
			clif_par_change(sd, type, sd.status.skill_point);
			break;
		case SP_HIT:
			clif_par_change(sd, type, sd.battle_status.hit);
			break;
		case SP_FLEE1:
			clif_par_change(sd, type, sd.battle_status.flee);
			break;
		case SP_FLEE2:
			clif_par_change(sd, type, sd.battle_status.flee2/10);
			break;
		case SP_MAXHP:
			clif_par_change(sd, type, sd.battle_status.max_hp);
			break;
		case SP_MAXSP:
			clif_par_change(sd, type, sd.battle_status.max_sp);
			break;
		case SP_HP:
			// On officials the HP never go below 1, even if you die [Lemongrass]
			// On officials the HP Novice class never go below 50%, even if you die [Napster]
			if (sd.battle_status.hp == 0) {
				clif_par_change(sd, type, (sd.class_&MAPID_UPPERMASK) != MAPID_NOVICE ? 1 : sd.battle_status.max_hp/2);
			} else {
				clif_par_change(sd, type, sd.battle_status.hp);
			}
			break;
		case SP_SP:
			clif_par_change(sd, type, sd.battle_status.sp);
			break;
		case SP_ASPD:
			clif_par_change(sd, type, sd.battle_status.amotion);
			break;
		case SP_ATK1:
			clif_par_change(sd, type, pc_leftside_atk(&sd));
			break;
		case SP_DEF1:
			clif_par_change(sd, type, pc_leftside_def(&sd));
			break;
		case SP_MDEF1:
			clif_par_change(sd, type, pc_leftside_mdef(&sd));
			break;
		case SP_ATK2:
			clif_par_change(sd, type, pc_rightside_atk(&sd));
			break;
		case SP_DEF2:
			clif_par_change(sd, type, pc_rightside_def(&sd));
			break;
		case SP_MDEF2: {
				//negative check (in case you have something like Berserk active)
				int mdef2 = pc_rightside_mdef(&sd);

#ifndef RENEWAL
				clif_par_change(sd, type, mdef2 < 0 ? 0 : mdef2);
#else
				clif_par_change(sd, type, mdef2);
#endif
			}
			break;
		case SP_CRITICAL:
			clif_par_change(sd, type, sd.battle_status.cri/10);
			break;
		case SP_MATK1:
			clif_par_change(sd, type, pc_rightside_matk(&sd));
			break;
		case SP_MATK2:
			clif_par_change(sd, type, pc_leftside_matk(&sd));
			break;

		case SP_ZENY:
			clif_longpar_change(sd, type, sd.status.zeny);
			break;
#if PACKETVER >= 20170830
		case SP_BASEEXP:
			clif_longlongpar_change(sd, type, client_exp(sd.status.base_exp));
			break;
		case SP_JOBEXP:
			clif_longlongpar_change(sd, type, client_exp(sd.status.job_exp));
			break;
		case SP_NEXTBASEEXP:
			clif_longlongpar_change(sd, type, client_exp(pc_nextbaseexp(&sd)));
			break;
		case SP_NEXTJOBEXP:
			clif_longlongpar_change(sd, type, client_exp(pc_nextjobexp(&sd)));
			break;
#else
		case SP_BASEEXP:
			clif_longpar_change(sd, type, client_exp(sd.status.base_exp));
			break;
		case SP_JOBEXP:
			clif_longpar_change(sd, type, client_exp(sd.status.job_exp));
			break;
		case SP_NEXTBASEEXP:
			clif_longpar_change(sd, type, client_exp(pc_nextbaseexp(&sd)));
			break;
		case SP_NEXTJOBEXP:
			clif_longpar_change(sd, type, client_exp(pc_nextjobexp(&sd)));
			break;
#endif

		/**
		 * SP_U<STAT> are used to update the amount of points necessary to increase that stat
		 **/
		case SP_USTR:
		case SP_UAGI:
		case SP_UVIT:
		case SP_UINT:
		case SP_UDEX:
		case SP_ULUK:
			clif_zc_status_change(sd, static_cast<uint16>(type), static_cast<uint8>(pc_need_status_point(&sd, type-SP_USTR+SP_STR, 1)));
			break;

		case SP_ATTACKRANGE:
			clif_attackrange( sd, sd.battle_status.rhw.range );
			break;

		case SP_STR:
			clif_couplestatus(sd, type, sd.status.str, sd.battle_status.str - sd.status.str);
			break;
		case SP_AGI:
			clif_couplestatus(sd, type, sd.status.agi, sd.battle_status.agi - sd.status.agi);
			break;
		case SP_VIT:
			clif_couplestatus(sd, type, sd.status.vit, sd.battle_status.vit - sd.status.vit);
			break;
		case SP_INT:
			clif_couplestatus(sd, type, sd.status.int_, sd.battle_status.int_ - sd.status.int_);
			break;
		case SP_DEX:
			clif_couplestatus(sd, type, sd.status.dex, sd.battle_status.dex - sd.status.dex);
			break;
		case SP_LUK:
			clif_couplestatus(sd, type, sd.status.luk, sd.battle_status.luk - sd.status.luk);
			break;

		case SP_CARTINFO:
			clif_cartcount( sd );
			break;

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
		case SP_AP:
			clif_par_change( sd, SP_AP, sd.battle_status.ap );
			break;
		case SP_TRAITPOINT:
			clif_par_change( sd, SP_TRAITPOINT, sd.status.trait_point );
			break;
		case SP_MAXAP:
			clif_par_change( sd, SP_MAXAP, sd.battle_status.max_ap );
			break;

		case SP_POW:
			clif_couplestatus( sd, SP_POW, sd.status.pow, sd.battle_status.pow - sd.status.pow );
			break;
		case SP_STA:
			clif_couplestatus( sd, SP_STA, sd.status.sta, sd.battle_status.sta - sd.status.sta );
			break;
		case SP_WIS:
			clif_couplestatus( sd, SP_WIS, sd.status.wis, sd.battle_status.wis - sd.status.wis );
			break;
		case SP_SPL:
			clif_couplestatus( sd, SP_SPL, sd.status.spl, sd.battle_status.spl - sd.status.spl );
			break;
		case SP_CON:
			clif_couplestatus( sd, SP_CON, sd.status.con, sd.battle_status.con - sd.status.con );
			break;
		case SP_CRT:
			clif_couplestatus( sd, SP_CRT, sd.status.crt, sd.battle_status.crt - sd.status.crt );
			break;

		case SP_UPOW:
		case SP_USTA:
		case SP_UWIS:
		case SP_USPL:
		case SP_UCON:
		case SP_UCRT:
			clif_zc_status_change( sd, static_cast<uint16>( type ), static_cast<uint8>( pc_need_trait_point( &sd, type - SP_UPOW + SP_POW, 1 ) ) );
			break;

		case SP_PATK:
			clif_par_change( sd, SP_PATK, sd.battle_status.patk );
			break;
		case SP_SMATK:
			clif_par_change( sd, SP_SMATK, sd.battle_status.smatk );
			break;
		case SP_RES:
			clif_par_change( sd, SP_RES, sd.battle_status.res );
			break;
		case SP_MRES:
			clif_par_change( sd, SP_MRES, sd.battle_status.mres );
			break;
		case SP_HPLUS:
			clif_par_change( sd, SP_HPLUS, sd.battle_status.hplus );
			break;
		case SP_CRATE:
			clif_par_change( sd, SP_CRATE, sd.battle_status.crate );
			break;
#else
		case SP_AP:
		case SP_TRAITPOINT:
		case SP_MAXAP:
		case SP_POW:
		case SP_STA:
		case SP_WIS:
		case SP_SPL:
		case SP_CON:
		case SP_CRT:
		case SP_UPOW:
		case SP_USTA:
		case SP_UWIS:
		case SP_USPL:
		case SP_UCON:
		case SP_UCRT:
		case SP_PATK:
		case SP_SMATK:
		case SP_RES:
		case SP_MRES:
		case SP_HPLUS:
		case SP_CRATE:
			// 4th job status are not supported by older clients
			return;
#endif

	default:
		ShowError("clif_updatestatus : unrecognized type %d\n",type);
		return;
	}

	// Additional update packets that should be sent right after
	switch( type ){
		case SP_BASELEVEL:
			if( sd.status.party_id ){
				struct party_data* p;
				int i;
		
				if( ( p = party_search( sd.status.party_id ) ) != nullptr ){
					ARR_FIND(0, MAX_PARTY, i, p->party.member[i].char_id == sd.status.char_id);

					if( i < MAX_PARTY ){
						p->party.member[i].lv = sd.status.base_level;
						clif_party_job_and_level( sd );
					}
				}
			}
			break;
		case SP_HP:
			clif_update_hp( sd );
			break;
	}
}


/// Notifies client of a parameter change of an another player.
/// 01ab <account id>.L <var id>.W <value>.L (ZC_PAR_CHANGE_USER)
/// var id:
///     SP_MANNER
///     ?
void clif_changemanner( map_session_data& sd ) {
	PACKET_ZC_PAR_CHANGE_USER packet{};

	packet.packetType = HEADER_ZC_PAR_CHANGE_USER;
	packet.gid = sd.bl.id;
	packet.type = static_cast<decltype(packet.type)>(SP_MANNER);
	packet.value = sd.status.manner;

	clif_send( &packet, sizeof( packet ), &sd.bl, AREA_WOS );
}


/// 00c3 <id>.L <type>.B <value>.B (ZC_SPRITE_CHANGE)
/// 01d7 <id>.L <type>.B <value1>.W <value2>.W (ZC_SPRITE_CHANGE2)
void clif_sprite_change( struct block_list *bl, int id, int type, int val, int val2, enum send_target target ){
	PACKET_ZC_SPRITE_CHANGE p = {};

	p.packetType = sendLookType;
	p.AID = id;
	p.type = type;
	p.val = val;
#if PACKETVER >= 4
	p.val2 = val2;
#endif

	clif_send( &p, sizeof( p ), bl, target );
}


/// Updates sprite/style properties of an object.
void clif_changelook(struct block_list *bl, int type, int val) {
	map_session_data* sd;
	status_change* sc;
	struct view_data* vd;
	enum send_target target = AREA;
	int val2 = 0;
	nullpo_retv(bl);

	sd = BL_CAST(BL_PC, bl);
	sc = status_get_sc(bl);
	vd = status_get_viewdata(bl);

	if( vd ) //temp hack to let Warp Portal change appearance
		switch(type) {
			case LOOK_WEAPON:
				if (sd) {
					clif_get_weapon_view(sd, &vd->weapon, &vd->shield);
					val = vd->weapon;
				}
				else 
					vd->weapon = val;
				break;
			case LOOK_SHIELD:
				if (sd) {
					clif_get_weapon_view(sd, &vd->weapon, &vd->shield);
					val = vd->shield;
				}
				else 
					vd->shield = val;
				break;
			case LOOK_BASE:
				if (!sd) 
					break;

				if ( val == JT_INVISIBLE )
					return;

				if (sd->sc.option&OPTION_COSTUME)
					vd->weapon = vd->shield = 0;

				if (!vd->cloth_color)
					break;

				if (sd) {
					if (sd->sc.option&OPTION_WEDDING && battle_config.wedding_ignorepalette)
						vd->cloth_color = 0;
					if (sd->sc.option&OPTION_XMAS && battle_config.xmas_ignorepalette)
						vd->cloth_color = 0;
					if (sd->sc.option&(OPTION_SUMMER|OPTION_SUMMER2) && battle_config.summer_ignorepalette)
						vd->cloth_color = 0;
					if (sd->sc.option&OPTION_HANBOK && battle_config.hanbok_ignorepalette)
						vd->cloth_color = 0;
					if (sd->sc.option&OPTION_OKTOBERFEST && battle_config.oktoberfest_ignorepalette)
						vd->cloth_color = 0;
					if (vd->body_style && sd->sc.option&OPTION_COSTUME)
 						vd->body_style = 0;
				}
				break;
			case LOOK_HAIR:
				vd->hair_style = val;
				break;
			case LOOK_HEAD_BOTTOM:
				vd->head_bottom = val;
				break;
			case LOOK_HEAD_TOP:
				vd->head_top = val;
				break;
			case LOOK_HEAD_MID:
				vd->head_mid = val;
				break;
			case LOOK_HAIR_COLOR:
				vd->hair_color = val;
				break;
			case LOOK_CLOTHES_COLOR:
				if (val && sd) {
					if (sd->sc.option&OPTION_WEDDING && battle_config.wedding_ignorepalette)
						val = 0;
					if (sd->sc.option&OPTION_XMAS && battle_config.xmas_ignorepalette)
						val = 0;
					if (sd->sc.option&(OPTION_SUMMER|OPTION_SUMMER2) && battle_config.summer_ignorepalette)
						val = 0;
					if (sd->sc.option&OPTION_HANBOK && battle_config.hanbok_ignorepalette)
						val = 0;
					if (sd->sc.option&OPTION_OKTOBERFEST && battle_config.oktoberfest_ignorepalette)
						val = 0;
				}
				vd->cloth_color = val;
				break;
			case LOOK_SHOES:
#if PACKETVER > 3
				if (sd) {
					int n;
					if((n = sd->equip_index[EQI_SHOES]) >= 0 && sd->inventory_data[n]) {
						if(sd->inventory_data[n]->view_id > 0)
							val = sd->inventory_data[n]->view_id;
						else
							val = sd->inventory.u.items_inventory[n].nameid;
					} else
						val = 0;
				}
#endif
				//Shoes? No packet uses this....
				break;
			case LOOK_BODY:
				// unknown purpose
				break;
			case LOOK_ROBE:
#if PACKETVER < 20110111
				return;
#else
				vd->robe = val;
#endif
				break;
			case LOOK_BODY2:
#if PACKETVER < 20150513
				return;
#else
				if (val && sd && sd->sc.option&OPTION_COSTUME)
 					val = 0;
 				vd->body_style = val;
#endif
				break;
	}

	// prevent leaking the presence of GM-hidden objects
	if( sc && sc->option&OPTION_INVISIBLE )
		target = SELF;

#if PACKETVER < 4
	if (target != SELF && bl->type == BL_NPC && (((TBL_NPC*)bl)->is_invisible))
		target = SELF;
	clif_sprite_change(bl, bl->id, type, val, 0, target);
#else
	if (bl->type != BL_NPC) {
		if (type == LOOK_WEAPON || type == LOOK_SHIELD) {
			type = LOOK_WEAPON;
			val = (vd ? vd->weapon : 0);
			val2 = (vd ? vd->shield : 0);
		}
		if (disguised(bl)) {
			clif_sprite_change(bl, bl->id, type, val, val2, AREA_WOS);
			clif_sprite_change(bl, disguised_bl_id(bl->id), type, val, val2, SELF);
		} else
			clif_sprite_change(bl, bl->id, type, val, val2, target);
	} else
		unit_refresh(bl);
#endif
}


//Sends a change-base-look packet required for traps as they are triggered.
void clif_changetraplook(struct block_list *bl,int val) {
	clif_sprite_change(bl, bl->id, LOOK_BASE, val, 0, AREA);
}


/// For the stupid cloth-dye bug. Resends the given view data to the area specified by bl.
void clif_refreshlook(struct block_list *bl, int id, int type, int val, enum send_target target) {
	clif_sprite_change(bl, id, type, val, 0, target);
}

/// Character status (ZC_STATUS).
/// 00bd <stpoint>.W <str>.B <need str>.B <agi>.B <need agi>.B <vit>.B <need vit>.B
///     <int>.B <need int>.B <dex>.B <need dex>.B <luk>.B <need luk>.B <atk>.W <atk2>.W
///     <matk min>.W <matk max>.W <def>.W <def2>.W <mdef>.W <mdef2>.W <hit>.W
///     <flee>.W <flee2>.W <crit>.W <aspd>.W <aspd2>.W
void clif_initialstatus( map_session_data& sd ) {
	PACKET_ZC_STATUS packet{};

	packet.packetType = HEADER_ZC_STATUS;

	packet.point = min(sd.status.status_point, INT16_MAX);
	packet.str = min(sd.status.str, UINT8_MAX);
	packet.standardStr = pc_need_status_point( &sd, SP_STR, 1 );
	packet.agi = min(sd.status.agi, UINT8_MAX);
	packet.standardAgi = pc_need_status_point( &sd, SP_AGI,1 );
	packet.vit = min(sd.status.vit, UINT8_MAX);
	packet.standardVit = pc_need_status_point( &sd, SP_VIT,1 );
	packet.int_ = min(sd.status.int_, UINT8_MAX);
	packet.standardInt = pc_need_status_point( &sd, SP_INT,1 );
	packet.dex = min(sd.status.dex, UINT8_MAX);
	packet.standardDex = pc_need_status_point( &sd, SP_DEX,1 );
	packet.luk = min(sd.status.luk, UINT8_MAX);
	packet.standardLuk = pc_need_status_point( &sd, SP_LUK,1 );

	packet.attPower = pc_leftside_atk( &sd );
	packet.refiningPower = pc_rightside_atk( &sd );
	packet.max_mattPower = pc_rightside_matk( &sd );
	packet.min_mattPower = pc_leftside_matk( &sd );
	packet.itemdefPower = pc_leftside_def( &sd );
	packet.plusdefPower = pc_rightside_def( &sd );
	packet.mdefPower = pc_leftside_mdef( &sd );

#ifdef RENEWAL
	packet.plusmdefPower = pc_rightside_mdef( &sd );
#else
	// Negative check for Frenzy'ed characters.
	packet.plusmdefPower = std::max( pc_rightside_mdef( &sd ), 0 );
#endif

	packet.hitSuccessValue = sd.battle_status.hit;
	packet.avoidSuccessValue = sd.battle_status.flee;
	packet.plusAvoidSuccessValue = sd.battle_status.flee2 / 10;
	packet.criticalSuccessValue = sd.battle_status.cri / 10;
	packet.ASPD = sd.battle_status.amotion;
	packet.plusASPD = 0;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );

	clif_updatestatus(sd, SP_STR);
	clif_updatestatus(sd, SP_AGI);
	clif_updatestatus(sd, SP_VIT);
	clif_updatestatus(sd, SP_INT);
	clif_updatestatus(sd, SP_DEX);
	clif_updatestatus(sd, SP_LUK);

	clif_updatestatus(sd, SP_ATTACKRANGE);
	clif_updatestatus(sd, SP_ASPD);

#ifdef RENEWAL
	clif_updatestatus(sd, SP_POW);
	clif_updatestatus(sd, SP_STA);
	clif_updatestatus(sd, SP_WIS);
	clif_updatestatus(sd, SP_SPL);
	clif_updatestatus(sd, SP_CON);
	clif_updatestatus(sd, SP_CRT);
	clif_updatestatus(sd, SP_PATK);
	clif_updatestatus(sd, SP_SMATK);
	clif_updatestatus(sd, SP_RES);
	clif_updatestatus(sd, SP_MRES);
	clif_updatestatus(sd, SP_HPLUS);
	clif_updatestatus(sd, SP_CRATE);
	clif_updatestatus(sd, SP_TRAITPOINT);
	clif_updatestatus(sd, SP_AP);
	clif_updatestatus(sd, SP_MAXAP);
	clif_updatestatus(sd, SP_UPOW);
	clif_updatestatus(sd, SP_USTA);
	clif_updatestatus(sd, SP_UWIS);
	clif_updatestatus(sd, SP_USPL);
	clif_updatestatus(sd, SP_UCON);
	clif_updatestatus(sd, SP_UCRT);
#endif
}


/// Marks an ammunition item in inventory as equipped.
/// 013c <index>.W (ZC_EQUIP_ARROW)
void clif_arrowequip( map_session_data& sd ) {
#if PACKETVER >= 20121128
	clif_status_change(&sd.bl, EFST_CLIENT_ONLY_EQUIP_ARROW, 1, INFINITE_TICK, 0, 0, 0);
#endif

	PACKET_ZC_EQUIP_ARROW packet{};

	packet.packetType = HEADER_ZC_EQUIP_ARROW;

	// Inventory index of the arrow
	packet.index = client_index( sd.equip_index[EQI_AMMO] );

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Ammunition action message.
/// 013b <type>.W (ZC_ACTION_FAILURE)
/// type:
///     0 = MsgStringTable[242]="Please equip the proper ammunition first."
///     1 = MsgStringTable[243]="You can't Attack or use Skills because your Weight Limit has been exceeded."
///     2 = MsgStringTable[244]="You can't use Skills because Weight Limit has been exceeded."
///     3 = assassin, baby_assassin, assassin_cross => MsgStringTable[1040]="You have equipped throwing daggers."
///         gunslinger => MsgStringTable[1175]="Bullets have been equipped."
///         NOT ninja => MsgStringTable[245]="Ammunition has been equipped."
void clif_arrow_fail( map_session_data& sd, e_action_failure type ) {
	PACKET_ZC_ACTION_FAILURE packet{};

	packet.packetType = HEADER_ZC_ACTION_FAILURE;
	packet.type = static_cast<decltype(packet.type)>(type);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Presents a list of items, that can be processed by Arrow Crafting (ZC_MAKINGARROW_LIST).
/// 01ad <packet len>.W { <name id>.W }*
void clif_arrow_create_list( map_session_data& sd ){
	PACKET_ZC_MAKINGARROW_LIST* p = reinterpret_cast<PACKET_ZC_MAKINGARROW_LIST*>( packet_buffer );

	p->packetType = HEADER_ZC_MAKINGARROW_LIST;
	p->packetLength = sizeof( *p );

	int count = 0;

	for (const auto &it : skill_arrow_db) {
		t_itemid nameid = it.second->nameid;

		if( !item_db.exists( nameid ) ){
			continue;
		}

		int index = pc_search_inventory( &sd, nameid );

		if( index < 0 ){
			continue;
		}

		if( sd.inventory.u.items_inventory[index].equip ){
			continue;
		}

		if( !sd.inventory.u.items_inventory[index].identify ){
			continue;
		}

		p->items[count].itemId = client_nameid( nameid );
		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
		count++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );

	if( count > 0 ){
		sd.menuskill_id = AC_MAKINGARROW;
		sd.menuskill_val = count;
	}
}


/// Notifies the client, about the result of an status change request.
/// 00bc <status id>.W <result>.B <value>.B (ZC_STATUS_CHANGE_ACK)
/// status id:
///     SP_STR ~ SP_LUK and SP_POW ~ SP_CRT
/// result:
///     0 = failure
///     1 = success
void clif_statusupack( map_session_data& sd, int32 type, bool success, int32 val ) {
	PACKET_ZC_STATUS_CHANGE_ACK packet{};

	packet.packetType = HEADER_ZC_STATUS_CHANGE_ACK;
	packet.sp = static_cast<decltype(packet.sp)>(type);
	packet.ok = success;
	packet.value = cap_value(val, 0, UINT8_MAX);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client about the result of a request to equip an item.
/// 00aa <index>.W <equip location>.W <result>.B (ZC_REQ_WEAR_EQUIP_ACK)
/// 00aa <index>.W <equip location>.W <view id>.W <result>.B (PACKETVER >= 20100629)
/// 08d0 <index>.W <equip location>.W <view id>.W <result>.B (ZC_REQ_WEAR_EQUIP_ACK2)
/// 0999 <index>.W <equip location>.L <view id>.W <result>.B (ZC_ACK_WEAR_EQUIP_V5)
/// @ok: //inversed forv2 v5
///     0 = failure
///     1 = success
///     2 = failure due to low level
void clif_equipitemack( map_session_data& sd, uint8 flag, int index, int pos ){
	PACKET_ZC_REQ_WEAR_EQUIP_ACK p = {};

	p.PacketType = HEADER_ZC_REQ_WEAR_EQUIP_ACK;
	p.index = client_index( index );
	p.wearLocation = pos;
#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20100629
	if( flag == ITEM_EQUIP_ACK_OK && sd.inventory_data[index]->equip&EQP_VISIBLE ){
		p.wItemSpriteNumber = sd.inventory_data[index]->look;
	}else{
		p.wItemSpriteNumber = 0;
	}
#endif
	p.result = flag;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Notifies the client about the result of a request to take off an item.
/// 00ac <index>.W <equip location>.W <result>.B (ZC_REQ_TAKEOFF_EQUIP_ACK)
/// 08d1 <index>.W <equip location>.W <result>.B (ZC_REQ_TAKEOFF_EQUIP_ACK2)
/// 099a <index>.W <equip location>.L <result>.B (ZC_ACK_TAKEOFF_EQUIP_V5)
/// @ok : //inversed for v2 v5
///     0 = failure
///     1 = success
void clif_unequipitemack(map_session_data *sd,int n,int pos,int ok)
{
	int fd, header, offs = 0;
#if PACKETVER >= 20130000
	header = 0x99a;
	ok = ok ? 0 : 1;
#elif PACKETVER >= 20110824
	header = 0x8d1;
	ok = ok ? 0 : 1;
#else
	header = 0xac;
#endif
	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd, packet_len(header));
	WFIFOW(fd,offs+0) = header;
	WFIFOW(fd,offs+2) = n+2;
#if PACKETVER >= 20130000
	WFIFOL(fd,offs+4) = pos;
	offs += 2;
#else
	WFIFOW(fd,offs+4) = pos;
#endif
	WFIFOB(fd,offs+6) = ok;
	WFIFOSET(fd, packet_len(header));
}


/// Notifies clients in the area about an special/visual effect.
/// 019b <id>.L <effect id>.L (ZC_NOTIFY_EFFECT)
/// effect id:
///     0 = base level up
///     1 = job level up
///     2 = refine failure
///     3 = refine success
///     4 = game over
///     5 = pharmacy success
///     6 = pharmacy failure
///     7 = base level up (super novice)
///     8 = job level up (super novice)
///     9 = base level up (taekwon)
void clif_misceffect( block_list& bl, e_notify_effect type ){
	PACKET_ZC_NOTIFY_EFFECT packet{};

	packet.packetType = HEADER_ZC_NOTIFY_EFFECT;
	packet.aid = bl.id;
	packet.effectId = static_cast<decltype(packet.effectId)>(type);

	clif_send( &packet, sizeof( packet ), &bl, AREA );
}


/// Notifies clients in the area of a state change.
/// 0119 <id>.L <body state>.W <health state>.W <effect state>.W <pk mode>.B (ZC_STATE_CHANGE)
/// 0229 <id>.L <body state>.W <health state>.W <effect state>.L <pk mode>.B (ZC_STATE_CHANGE3)
void clif_changeoption_target( struct block_list* bl, struct block_list* target ){
	nullpo_retv( bl );

	status_change* sc = status_get_sc( bl );

	//How can an option change if there's no sc?
	if( sc == nullptr ){
		return;
	}

	map_session_data* sd = BL_CAST( BL_PC, bl );

	PACKET_ZC_STATE_CHANGE p = {};

	p.packetType = HEADER_ZC_STATE_CHANGE;
	p.AID = bl->id;
	p.bodyState = sc->opt1;
	p.healthState = sc->opt2;
	p.effectState = sc->option;
	p.isPKModeON = sd ? sd->status.karma : false;

	if( target == nullptr ){
		if( disguised( bl ) ){
			clif_send( &p, sizeof( p ), bl, AREA_WOS );
			p.AID = disguised_bl_id( p.AID );
			clif_send( &p, sizeof( p ), bl, SELF );
			p.AID = disguised_bl_id( p.AID );
			p.effectState = OPTION_INVISIBLE;
			clif_send( &p, sizeof( p ), bl, SELF );
		}else{
			clif_send( &p, sizeof( p ), bl, AREA );
		}

		//Whenever we send "changeoption" to the client, the provoke icon is lost
		//There is probably an option for the provoke icon, but as we don't know it, we have to do this for now
		if( sc->getSCE(SC_PROVOKE) ){
			const struct TimerData *td = get_timer( sc->getSCE(SC_PROVOKE)->timer );

			clif_status_change( bl, status_db.getIcon(SC_PROVOKE), 1, ( !td ? INFINITE_TICK : DIFF_TICK( td->tick, gettick() ) ), 0, 0, 0 );
		}
	}else{
		if( disguised( bl ) ){
			p.AID = disguised_bl_id( p.AID );
			clif_send( &p, sizeof( p ), target, SELF );
			p.AID = disguised_bl_id( p.AID );
			p.effectState = OPTION_INVISIBLE;
			clif_send( &p, sizeof( p ), target, SELF );
		}else{
			clif_send( &p, sizeof( p ), target, SELF );
		}
	}
}


/// Displays status change effects on NPCs/monsters.
/// 028a <id>.L <effect state>.L <level>.L <showEFST>.L (ZC_NPC_SHOWEFST_UPDATE)
void clif_changeoption2( block_list& bl ){
	status_change *sc = status_get_sc(&bl);
	if (!sc)
		return; //How can an option change if there's no sc?

	PACKET_ZC_NPC_SHOWEFST_UPDATE packet{};

	packet.packetType = HEADER_ZC_NPC_SHOWEFST_UPDATE;
	packet.gid = bl.id;
	packet.effectState = sc->option;
	packet.level = clif_setlevel(&bl);
	packet.showEFST = sc->opt3;

	if (disguised(&bl)) {
		clif_send( &packet, sizeof( packet ), &bl, AREA_WOS );
		
		packet.gid = disguised_bl_id( bl.id );
		clif_send( &packet, sizeof( packet ), &bl, SELF );
		
		packet.gid = bl.id;
		packet.effectState = OPTION_INVISIBLE;
		clif_send( &packet, sizeof( packet ), &bl, SELF );
	} else {
		clif_send( &packet, sizeof( packet ), &bl, AREA );
	}
}


/// Notifies the client about the result of an item use request.
/// 00a8 <index>.W <amount>.W <result>.B (ZC_USE_ITEM_ACK)
/// 01c8 <index>.W <name id>.W <id>.L <amount>.W <result>.B (ZC_USE_ITEM_ACK2)
void clif_useitemack( map_session_data *sd, int index, int amount, bool ok ){
	nullpo_retv( sd );

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	if( index < 0 || index >= MAX_INVENTORY || sd->inventory.u.items_inventory[index].nameid == 0 || sd->inventory_data[index] == nullptr ){
		return;
	}

	PACKET_ZC_USE_ITEM_ACK p = {};

	p.packetType = useItemAckType;
	p.index = index + 2;
#if PACKETVER > 3
	p.itemId = client_nameid( sd->inventory.u.items_inventory[index].nameid );
	p.AID = sd->bl.id;
#endif
	p.amount = amount;
	p.result = ok;

	if( !ok ){
		clif_send( &p, sizeof(p), &sd->bl, SELF );
	}else{
		clif_send( &p, sizeof(p), &sd->bl, AREA );
	}
}


/// Inform client whether chatroom creation was successful or not.
/// 00d6 <flag>.B (ZC_ACK_CREATE_CHATROOM)
/// flag:
///     0 = Room has been successfully created (opens chat room)
///     1 = Room limit exceeded
///     2 = Same room already exists
void clif_createchat( map_session_data& sd, e_create_chatroom flag ){
	PACKET_ZC_ACK_CREATE_CHATROOM packet{};

	packet.packetType = HEADER_ZC_ACK_CREATE_CHATROOM;
	packet.flag = static_cast<decltype(packet.flag)>(flag);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Display a chat above the owner (ZC_ROOM_NEWENTRY).
/// 00d7 <packet len>.W <owner id>.L <char id>.L <limit>.W <users>.W <type>.B <title>.?B
/// type:
///     0 = private (password protected)
///     1 = public
///     2 = arena (npc waiting room)
///     3 = PK zone (non-clickable)
void clif_dispchat(struct chat_data* cd, int fd)
{
	unsigned char buf[128];
	uint8 type;

	if( cd == nullptr || cd->owner == nullptr )
		return;

	type = (cd->owner->type == BL_PC ) ? (cd->pub) ? 1 : 0
	     : (cd->owner->type == BL_NPC) ? (cd->limit) ? 2 : 3
	     : 1;

	WBUFW(buf, 0) = 0xd7;
	WBUFW(buf, 2) = (uint16)(17 + strlen(cd->title));
	WBUFL(buf, 4) = cd->owner->id;
	WBUFL(buf, 8) = cd->bl.id;
	WBUFW(buf,12) = cd->limit;
	WBUFW(buf,14) = (cd->owner->type == BL_NPC) ? cd->users+1 : cd->users;
	WBUFB(buf,16) = type;
	memcpy(WBUFCP(buf,17), cd->title, strlen(cd->title)); // not zero-terminated

	if( session_isActive(fd) ) {
		WFIFOHEAD(fd,WBUFW(buf,2));
		memcpy(WFIFOP(fd,0),buf,WBUFW(buf,2));
		WFIFOSET(fd,WBUFW(buf,2));
	} else {
		clif_send(buf,WBUFW(buf,2),cd->owner,AREA_WOSC);
	}
}


/// Chatroom properties adjustment (ZC_CHANGE_CHATROOM).
/// 00df <packet len>.W <owner id>.L <chat id>.L <limit>.W <users>.W <type>.B <title>.?B
/// type:
///     0 = private (password protected)
///     1 = public
///     2 = arena (npc waiting room)
///     3 = PK zone (non-clickable)
void clif_changechatstatus(struct chat_data* cd)
{
	unsigned char buf[128];
	uint8 type;

	if( cd == nullptr || cd->usersd[0] == nullptr )
		return;

	type = (cd->owner->type == BL_PC ) ? (cd->pub) ? 1 : 0
	     : (cd->owner->type == BL_NPC) ? (cd->limit) ? 2 : 3
	     : 1;

	WBUFW(buf, 0) = 0xdf;
	WBUFW(buf, 2) = (uint16)(17 + strlen(cd->title));
	WBUFL(buf, 4) = cd->owner->id;
	WBUFL(buf, 8) = cd->bl.id;
	WBUFW(buf,12) = cd->limit;
	WBUFW(buf,14) = (cd->owner->type == BL_NPC) ? cd->users+1 : cd->users;
	WBUFB(buf,16) = type;
	memcpy(WBUFCP(buf,17), cd->title, strlen(cd->title)); // not zero-terminated

	clif_send(buf,WBUFW(buf,2),cd->owner,CHAT);
}


/// Removes the chatroom (ZC_DESTROY_ROOM).
/// 00d8 <chat id>.L
void clif_clearchat(struct chat_data *cd,int fd)
{
	unsigned char buf[32];

	nullpo_retv(cd);

	WBUFW(buf,0) = 0xd8;
	WBUFL(buf,2) = cd->bl.id;
	if( session_isActive(fd) ) {
		WFIFOHEAD(fd,packet_len(0xd8));
		memcpy(WFIFOP(fd,0),buf,packet_len(0xd8));
		WFIFOSET(fd,packet_len(0xd8));
	} else {
		clif_send(buf,packet_len(0xd8),cd->owner,AREA_WOSC);
	}
}


/// Displays messages regarding join chat failures.
/// 00da <result>.B (ZC_REFUSE_ENTER_ROOM)
/// result:
///     0 = room full
///     1 = wrong password
///     2 = kicked
///     3 = success (no message)
///     4 = no enough zeny
///     5 = too low level
///     6 = too high level
///     7 = unsuitable job class
void clif_joinchatfail( map_session_data& sd, e_refuse_enter_room result ){
	PACKET_ZC_REFUSE_ENTER_ROOM packet{};

	packet.packetType = HEADER_ZC_REFUSE_ENTER_ROOM;
	packet.result = static_cast<decltype(packet.result)>(result);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client about entering a chatroom (ZC_ENTER_ROOM).
/// 00db <packet len>.W <chat id>.L { <role>.L <name>.24B }*
/// role:
///     0 = owner (menu)
///     1 = normal
vvoid clif_joinchatok(map_session_data& sd, chat_data& cd){


	PACKET_ZC_ENTER_ROOM* p = reinterpret_cast<PACKET_ZC_ENTER_ROOM*>( packet_buffer );

	p->packetType = HEADER_ZC_ENTER_ROOM;
	p->packetSize = sizeof(*p);
	p->chatId = cd.bl.id;
	
	if(cd.owner->type == BL_NPC){
		PACKET_ZC_ENTER_ROOM_sub& owner = p->members[0];
		owner.flag = 0;
		safestrncpy(owner.name, reinterpret_cast<npc_data*>(cd.owner)->name, sizeof(owner.name));
		p->packetSize += static_cast<decltype(p->packetSize)>( sizeof( owner ) );

		for (size_t i = 0; i < cd.users; i++) {
			PACKET_ZC_ENTER_ROOM_sub& member = p->members[i + 1];

			member.flag = 1;
			safestrncpy(member.name, cd.usersd[i]->status.name, sizeof(member.name));

			p->packetSize += static_cast<decltype(p->packetSize)>( sizeof( member ) );
		}
	}else{
		for (size_t i = 0; i < cd.users; i++) {
			PACKET_ZC_ENTER_ROOM_sub& member = p->members[i];

			member.flag = i > 0;
			safestrncpy(member.name, cd.usersd[i]->status.name, sizeof(member.name));

			p->packetSize += static_cast<decltype(p->packetSize)>( sizeof( member ) );
		}
	}
	
	clif_send(p,p->packetSize,&sd.bl,SELF);

}


/// Notifies clients in a chat about a new member (ZC_MEMBER_NEWENTRY).
/// 00dc <users>.W <name>.24B
void clif_addchat(struct chat_data* cd,map_session_data *sd)
{
	unsigned char buf[29];

	nullpo_retv(sd);
	nullpo_retv(cd);

	WBUFW(buf, 0) = 0xdc;
	WBUFW(buf, 2) = cd->users;
	safestrncpy(WBUFCP(buf, 4),sd->status.name,NAME_LENGTH);
	clif_send(buf,packet_len(0xdc),&sd->bl,CHAT_WOS);
}


/// Announce the new owner (ZC_ROLE_CHANGE).
/// 00e1 <role>.L <nick>.24B
/// role:
///     0 = owner (menu)
///     1 = normal
void clif_changechatowner(struct chat_data* cd, map_session_data* sd)
{
	unsigned char buf[64];

	nullpo_retv(sd);
	nullpo_retv(cd);

	WBUFW(buf, 0) = 0xe1;
	WBUFL(buf, 2) = 1;
	safestrncpy(WBUFCP(buf,6),cd->usersd[0]->status.name,NAME_LENGTH);

	WBUFW(buf,30) = 0xe1;
	WBUFL(buf,32) = 0;
	safestrncpy(WBUFCP(buf,36),sd->status.name,NAME_LENGTH);

	clif_send(buf,packet_len(0xe1)*2,&sd->bl,CHAT);
}


/// Notify about user leaving the chatroom (ZC_MEMBER_EXIT).
/// 00dd <users>.W <nick>.24B <flag>.B
/// flag:
///     0 = left
///     1 = kicked
void clif_leavechat(struct chat_data* cd, map_session_data* sd, bool flag)
{
	unsigned char buf[32];

	nullpo_retv(sd);
	nullpo_retv(cd);

	WBUFW(buf, 0) = 0xdd;
	WBUFW(buf, 2) = cd->users-1;
	safestrncpy(WBUFCP(buf,4),sd->status.name,NAME_LENGTH);
	WBUFB(buf,28) = flag;

	clif_send(buf,packet_len(0xdd),&sd->bl,CHAT);
}


/// Opens a trade request window from char 'name'.
/// 00e5 <nick>.24B (ZC_REQ_EXCHANGE_ITEM)
/// 01f4 <nick>.24B <charid>.L <baselvl>.W (ZC_REQ_EXCHANGE_ITEM2)
void clif_traderequest(map_session_data* sd, const char* name)
{
	int fd = sd->fd;

#if PACKETVER < 6
	WFIFOHEAD(fd,packet_len(0xe5));
	WFIFOW(fd,0) = 0xe5;
	safestrncpy(WFIFOCP(fd,2), name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0xe5));
#else
	map_session_data* tsd = map_id2sd(sd->trade_partner);
	if( !tsd ) return;

	WFIFOHEAD(fd,packet_len(0x1f4));
	WFIFOW(fd,0) = 0x1f4;
	safestrncpy(WFIFOCP(fd,2), name, NAME_LENGTH);
	WFIFOL(fd,26) = tsd->status.char_id;
	WFIFOW(fd,30) = tsd->status.base_level;
	WFIFOSET(fd,packet_len(0x1f4));
#endif
}


/// Reply to a trade-request.
/// 00e7 <result>.B (ZC_ACK_EXCHANGE_ITEM)
/// 01f5 <result>.B <charid>.L <baselvl>.W (ZC_ACK_EXCHANGE_ITEM2)
/// result:
///     0 = Char is too far
///     1 = Character does not exist
///     2 = Trade failed
///     3 = Accept
///     4 = Cancel
///     5 = Busy
void clif_tradestart(map_session_data* sd, uint8 type)
{
	int fd = sd->fd;
	map_session_data* tsd = map_id2sd(sd->trade_partner);
	if( PACKETVER < 6 || !tsd ) {
		WFIFOHEAD(fd,packet_len(0xe7));
		WFIFOW(fd,0) = 0xe7;
		WFIFOB(fd,2) = type;
		WFIFOSET(fd,packet_len(0xe7));
	} else {
		WFIFOHEAD(fd,packet_len(0x1f5));
		WFIFOW(fd,0) = 0x1f5;
		WFIFOB(fd,2) = type;
		WFIFOL(fd,3) = tsd->status.char_id;
		WFIFOW(fd,7) = tsd->status.base_level;
		WFIFOSET(fd,packet_len(0x1f5));
	}
}


/// Notifies the client about an item from other player in current trade.
/// 00e9 <amount>.L <nameid>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_EXCHANGE_ITEM)
/// 080f <nameid>.W <item type>.B <amount>.L <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_EXCHANGE_ITEM2)
/// 0a09 <nameid>.W <item type>.B <amount>.L <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W { <option id>.W <option value>.W <option param>.B }*5 (ZC_ADD_EXCHANGE_ITEM3)
void clif_tradeadditem( map_session_data* sd, map_session_data* tsd, int index, int amount ){
	nullpo_retv( sd );
	nullpo_retv( tsd );

	int fd = tsd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	PACKET_ZC_ADD_EXCHANGE_ITEM p = {};

	if( index ){
		index = server_index( index );

		if( index >= MAX_INVENTORY || sd->inventory.u.items_inventory[index].nameid == 0 || sd->inventory_data[index] == nullptr ){
			return;
		}

		p.itemId = client_nameid( sd->inventory.u.items_inventory[index].nameid );
#if PACKETVER >= 20100223
		p.itemType = sd->inventory_data[index]->type;
#endif
		p.identified = sd->inventory.u.items_inventory[index].identify;
		p.damaged = sd->inventory.u.items_inventory[index].attribute;
		p.refine = sd->inventory.u.items_inventory[index].refine;
		clif_addcards( &p.slot, &sd->inventory.u.items_inventory[index] );
#if PACKETVER >= 20150226
		clif_add_random_options( p.option_data, sd->inventory.u.items_inventory[index] );
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
		p.location = pc_equippoint_sub( sd, sd->inventory_data[index] );
		p.look = sd->inventory_data[index]->look;
		p.grade = sd->inventory.u.items_inventory[index].enchantgrade;
#endif
#endif
	}else{
		p = {};
	}

	p.packetType = HEADER_ZC_ADD_EXCHANGE_ITEM;
	p.amount = amount;

	clif_send( &p, sizeof( p ), &tsd->bl, SELF );
}


/// Notifies the client about the result of request to add an item to the current trade (ZC_ACK_ADD_EXCHANGE_ITEM).
/// 00ea <index>.W <result>.B
/// result:
///     0 = success
///     1 = overweight
///     2 = trade canceled
void clif_tradeitemok(map_session_data& sd, int index, e_exitem_add_result result)
{
	PACKET_ZC_ACK_ADD_EXCHANGE_ITEM p = {};
	p.packetType = HEADER_ZC_ACK_ADD_EXCHANGE_ITEM;
	p.index = client_index(index);
	p.result = static_cast<uint8>(result);

	clif_send(&p, sizeof(p), &sd.bl, SELF);
}


/// Notifies the client about finishing one side of the current trade.
/// 00ec <who>.B (ZC_CONCLUDE_EXCHANGE_ITEM)
/// who:
///     0 = self
///     1 = other player
void clif_tradedeal_lock( map_session_data& sd, bool who ){
	PACKET_ZC_CONCLUDE_EXCHANGE_ITEM packet{};

	packet.packetType = HEADER_ZC_CONCLUDE_EXCHANGE_ITEM;
	packet.who = who;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client about the trade being canceled.
/// 00ee (ZC_CANCEL_EXCHANGE_ITEM)
void clif_tradecancelled( map_session_data& sd ){
	PACKET_ZC_CANCEL_EXCHANGE_ITEM packet{};

	packet.packetType = HEADER_ZC_CANCEL_EXCHANGE_ITEM;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Result of a trade.
/// 00f0 <result>.B (ZC_EXEC_EXCHANGE_ITEM)
/// result:
///     0 = success
///     1 = failure
void clif_tradecompleted( map_session_data& sd ){
	PACKET_ZC_EXEC_EXCHANGE_ITEM packet{};

	packet.packetType = HEADER_ZC_EXEC_EXCHANGE_ITEM;
	packet.result = 0;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Resets the trade window on the send side.
/// 00f1 (ZC_EXCHANGEITEM_UNDO)
/// NOTE: Unknown purpose. Items are not removed until the window is
///       refreshed (ex. by putting another item in there).
void clif_tradeundo( map_session_data& sd ){
	PACKET_ZC_EXCHANGEITEM_UNDO packet{};

	packet.packetType = HEADER_ZC_EXCHANGEITEM_UNDO;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Updates storage total amount.
/// 00f2 <current count>.W <max count>.W (ZC_NOTIFY_STOREITEM_COUNTINFO)
void clif_updatestorageamount( map_session_data& sd, uint16 amount, uint16 max_amount ){
	PACKET_ZC_NOTIFY_STOREITEM_COUNTINFO packet{};

	packet.packetType = HEADER_ZC_NOTIFY_STOREITEM_COUNTINFO;
	packet.amount = amount;
	packet.max_amount = max_amount;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client of an item being added to the storage.
/// 00f4 <index>.W <amount>.L <nameid>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_STORE)
/// 01c4 <index>.W <amount>.L <nameid>.W <type>.B <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_STORE2)
/// 0a0a <index>.W <amount>.L <nameid>.W <type>.B <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W { <option id>.W <option value>.W <option param>.B }*5 (ZC_ADD_ITEM_TO_STORE3)
void clif_storageitemadded( map_session_data* sd, struct item* i, int index, int amount ){
	nullpo_retv( sd );
	nullpo_retv( i );

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	PACKET_ZC_ADD_ITEM_TO_STORE p = {};

	p.packetType = HEADER_ZC_ADD_ITEM_TO_STORE; // Storage item added
	p.index = client_storage_index( index ); // index
	p.amount = amount; // amount
	p.itemId = client_nameid( i->nameid ); // id
#if PACKETVER >= 5
	p.itemType = itemtype( i->nameid ); //type
#endif
	p.identified = i->identify; //identify flag
	p.damaged = i->attribute; // attribute
	p.refine = i->refine; //refine
	clif_addcards( &p.slot, i );
#if PACKETVER_MAIN_NUM >= 20140813 || PACKETVER_RE_NUM >= 20140402 || defined(PACKETVER_ZERO)
	clif_add_random_options( p.option_data, *i );
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	p.grade = i->enchantgrade;
#endif
#endif

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Notifies the client of an item being deleted from the storage.
/// 00f6 <index>.W <amount>.L (ZC_DELETE_ITEM_FROM_STORE)
void clif_storageitemremoved( map_session_data& sd, uint16 index, uint32 amount ){
	PACKET_ZC_DELETE_ITEM_FROM_STORE packet{};

	packet.packetType = HEADER_ZC_DELETE_ITEM_FROM_STORE;
	packet.index = client_storage_index(index);
	packet.amount = amount;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Closes storage.
/// 00f8 (ZC_CLOSE_STORE)
void clif_storageclose( map_session_data& sd ){
	PACKET_ZC_CLOSE_STORE packet{};

	packet.packetType = HEADER_ZC_CLOSE_STORE;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies clients in an area of a player's souls.
/// 01d0 <id>.L <amount>.W (ZC_SPIRITS)
/// 01e1 <id>.L <amount>.W (ZC_SPIRITS2)
/// 0b73 <id>.L <amount>.W (ZC_SOULENERGY)
void clif_soulball( map_session_data *sd, struct block_list* target, enum send_target send_target ){
#if PACKETVER_MAIN_NUM >= 20200414 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20200506
	PACKET_ZC_SOULENERGY p = {};

	p.PacketType = HEADER_ZC_SOULENERGY;
#else
	PACKET_ZC_SPIRITS p = {};

	p.PacketType = HEADER_ZC_SPIRITS;
#endif

	p.AID = sd->bl.id;
	p.num = sd->soulball;

	clif_send( &p, sizeof( p ), target == nullptr ? &sd->bl : target, send_target );
}


/*==========================================
 * Server tells 'sd' player client the abouts of 'dstsd' player
 *------------------------------------------*/
static void clif_getareachar_pc(map_session_data* sd,map_session_data* dstsd)
{
	struct block_list *d_bl;
	int i;

	if( dstsd->chatID ) {
		struct chat_data *cd = nullptr;
		if( (cd = (struct chat_data*)map_id2bl(dstsd->chatID)) && cd->usersd[0]==dstsd)
			clif_dispchat(cd,sd->fd);
	} else if( dstsd->state.vending )
		clif_showvendingboard( *dstsd, SELF, &sd->bl );
	else if( dstsd->state.buyingstore )
		clif_buyingstore_entry( *dstsd, &sd->bl );

	if(dstsd->spiritball > 0)
		clif_spiritball( &dstsd->bl, &sd->bl, SELF );
	clif_millenniumshield_single( *dstsd, *sd );
	clif_spiritcharm_single( *dstsd, *sd );
	if (dstsd->soulball > 0)
		clif_soulball( dstsd, &sd->bl, SELF );
	if (dstsd->servantball > 0)
		clif_servantball( *dstsd, &sd->bl, SELF );
	if (dstsd->abyssball > 0)
		clif_abyssball( *dstsd, &sd->bl, SELF );
	if( (sd->status.party_id && dstsd->status.party_id == sd->status.party_id) || //Party-mate, or hpdisp setting.
		(sd->bg_id && sd->bg_id == dstsd->bg_id) || //BattleGround
		pc_has_permission(sd, PC_PERM_VIEW_HPMETER)
	)
	clif_hpmeter_single( *sd, dstsd->bl.id, dstsd->battle_status.hp, dstsd->battle_status.max_hp );

	// display link (sd - dstsd) to sd
	ARR_FIND( 0, MAX_DEVOTION, i, sd->devotion[i] == dstsd->bl.id );
	if( i < MAX_DEVOTION )
		clif_devotion(&sd->bl, sd);
	// display links (dstsd - devotees) to sd
	ARR_FIND( 0, MAX_DEVOTION, i, dstsd->devotion[i] > 0 );
	if( i < MAX_DEVOTION )
		clif_devotion(&dstsd->bl, sd);
	// display link (dstsd - crusader) to sd
	if( dstsd->sc.getSCE(SC_DEVOTION) && (d_bl = map_id2bl(dstsd->sc.getSCE(SC_DEVOTION)->val1)) != nullptr )
		clif_devotion(d_bl, sd);
}

void clif_getareachar_unit( map_session_data* sd,struct block_list *bl ){
	struct unit_data *ud;
	struct view_data *vd;
	bool option = false;
	unsigned int option_val = 0;

	vd = status_get_viewdata(bl);
	if (!vd || vd->class_ == JT_INVISIBLE)
		return;

	// Hide NPC from Maya Purple card
	if (clif_npc_mayapurple(bl))
		return;

	if( bl->type == BL_NPC && npc_is_hidden_dynamicnpc( *( (struct npc_data*)bl ), *sd ) ){
		// Do not send anything
		return;
	}

	ud = unit_bl2ud(bl);

	if( ud && ud->walktimer != INVALID_TIMER ){
		clif_set_unit_walking( *bl, sd, *ud, SELF );
	}else{
		clif_set_unit_idle( bl, false, SELF, &sd->bl );
	}

	if (vd->cloth_color)
		clif_refreshlook(&sd->bl,bl->id,LOOK_CLOTHES_COLOR,vd->cloth_color,SELF);
	if (vd->body_style)
		clif_refreshlook(&sd->bl,bl->id,LOOK_BODY2,vd->body_style,SELF);

	switch (bl->type)
	{
	case BL_PC:
		{
			TBL_PC* tsd = (TBL_PC*)bl;

			clif_getareachar_pc(sd, tsd);
			if(tsd->state.size==SZ_BIG) // tiny/big players [Valaris]
				clif_specialeffect_single(bl,EF_GIANTBODY2,sd->fd);
			else if(tsd->state.size==SZ_MEDIUM)
				clif_specialeffect_single(bl,EF_BABYBODY2,sd->fd);
			if( tsd->bg_id && map_getmapflag(tsd->bl.m, MF_BATTLEGROUND) )
				clif_sendbgemblem_single(sd->fd,tsd);
			if ( tsd->status.robe )
				clif_refreshlook(&sd->bl,bl->id,LOOK_ROBE,tsd->status.robe,SELF);
			clif_efst_status_change_sub(&sd->bl, bl, SELF);
			clif_hat_effects( *sd, tsd->bl, SELF );
		}
		break;
	case BL_MER: // Devotion Effects
		if( ((TBL_MER*)bl)->devotion_flag )
			clif_devotion(bl, sd);
		break;
	case BL_NPC:
		{
			TBL_NPC* nd = (TBL_NPC*)bl;
			if( nd->chat_id )
				clif_dispchat((struct chat_data*)map_id2bl(nd->chat_id),sd->fd);
			if( nd->size == SZ_BIG )
				clif_specialeffect_single(bl,EF_GIANTBODY2,sd->fd);
			else if( nd->size == SZ_MEDIUM )
				clif_specialeffect_single(bl,EF_BABYBODY2,sd->fd);
			clif_efst_status_change_sub(&sd->bl, bl, SELF);
			clif_progressbar_npc(nd, sd);
		}
		break;
	case BL_MOB:
		{
			TBL_MOB* md = (TBL_MOB*)bl;
			if(md->special_state.size==SZ_BIG) // tiny/big mobs [Valaris]
				clif_specialeffect_single(bl,EF_GIANTBODY2,sd->fd);
			else if(md->special_state.size==SZ_MEDIUM)
				clif_specialeffect_single(bl,EF_BABYBODY2,sd->fd);
#if PACKETVER >= 20120404
			if (battle_config.monster_hp_bars_info && !map_getmapflag(bl->m, MF_HIDEMOBHPBAR)) {
				int i;
				for(i = 0; i < DAMAGELOG_SIZE; i++)// must show hp bar to all char who already hit the mob.
					if( md->dmglog[i].id == sd->status.char_id )
						clif_monster_hp_bar(md, sd->fd);
			}
#endif
		}
		break;
	case BL_PET:
		if (vd->head_bottom)
			clif_pet_equip(sd, (TBL_PET*)bl); // needed to display pet equip properly
		break;
	}
}

//Modifies the type of damage according to status changes [Skotlex]
//Aegis data specifies that: 4 endure against single hit sources, 9 against multi-hit.
static enum e_damage_type clif_calc_delay(enum e_damage_type type, int div, int64 damage, int delay)
{
	return ( delay == 0 && damage > 0 ) ? ( div > 1 ? DMG_MULTI_HIT_ENDURE : DMG_ENDURE ) : type;
}

/*==========================================
 * Estimates walk delay based on the damage criteria. [Skotlex]
 *------------------------------------------*/
static int clif_calc_walkdelay(struct block_list *bl,int delay, char type, int64 damage, int div_)
{
	if (type == DMG_ENDURE || type == DMG_MULTI_HIT_ENDURE || damage <= 0)
		return 0;

	if (bl->type == BL_PC) {
		if (battle_config.pc_walk_delay_rate != 100)
			delay = delay*battle_config.pc_walk_delay_rate/100;
	} else
		if (battle_config.walk_delay_rate != 100)
			delay = delay*battle_config.walk_delay_rate/100;

	if (div_ > 1) //Multi-hit skills mean higher delays.
		delay += battle_config.multihit_delay*(div_-1);

	return (delay > 0) ? delay:1; //Return 1 to specify there should be no noticeable delay, but you should stop walking.
}

/*========================================== [Playtester]
* Returns hallucination damage the client displays
*------------------------------------------*/
static int clif_hallucination_damage()
{
	int digit = rnd() % 5 + 1;
	switch (digit)
	{
	case 1:
		return (rnd() % 10);
	case 2:
		return (rnd() % 100);
	case 3:
		return (rnd() % 1000);
	case 4:
		return (rnd() % 10000);
	}
	return (rnd() % 32767);
}

#define DEFAULT_ANIMATION_SPEED 432

/// Sends a 'damage' packet (src performs action on dst)
/// 008a <src ID>.L <dst ID>.L <server tick>.L <src speed>.L <dst speed>.L <damage>.W <div>.W <type>.B <damage2>.W (ZC_NOTIFY_ACT)
/// 02e1 <src ID>.L <dst ID>.L <server tick>.L <src speed>.L <dst speed>.L <damage>.L <div>.W <type>.B <damage2>.L (ZC_NOTIFY_ACT2)
/// 08c8 <src ID>.L <dst ID>.L <server tick>.L <src speed>.L <dst speed>.L <damage>.L <IsSPDamage>.B <div>.W <type>.B <damage2>.L (ZC_NOTIFY_ACT3)
/// type:
///     0 = damage [ damage: total damage, div: amount of hits, damage2: assassin dual-wield damage ]
///     1 = pick up item
///     2 = sit down
///     3 = stand up
///     4 = damage (endure)
///     5 = (splash?)
///     6 = (skill?)
///     7 = (repeat damage?)
///     8 = multi-hit damage
///     9 = multi-hit damage (endure)
///     10 = critical hit
///     11 = lucky dodge
///     12 = (touch skill?)
///     13 = multi-hit critical
int clif_damage(struct block_list* src, struct block_list* dst, t_tick tick, int sdelay, int ddelay, int64 sdamage, int div, enum e_damage_type type, int64 sdamage2, bool spdamage)
{
	unsigned char buf[34];
	status_change *sc;
	int damage = (int)cap_value(sdamage,INT_MIN,INT_MAX);
	int damage2 = (int)cap_value(sdamage2,INT_MIN,INT_MAX);
#if PACKETVER < 20071113
	const int cmd = 0x8a;
	int offset = 0;
#elif PACKETVER < 20131223
	const int cmd = 0x2e1;
	int offset = 2;
#else
	const int cmd = 0x8c8;
	int offset = 3;
#endif

	nullpo_ret(src);
	nullpo_ret(dst);

	if (type != DMG_MULTI_HIT_CRITICAL)
		type = clif_calc_delay(type,div,damage+damage2,ddelay);
	sc = status_get_sc(dst);
	if(sc && sc->count) {
		if(sc->getSCE(SC_HALLUCINATION)) {
			damage = clif_hallucination_damage();
			if(damage2) damage2 = clif_hallucination_damage();
		}
	}

	// Calculate what sdelay to send to the client so it applies damage at the same time as the server
	if (battle_config.synchronize_damage && src->type == BL_MOB) {
		// When a clif_damage packet is sent to the client it will also send "sdelay" (amotion) as value.
		// The client however does not interpret this value as AttackMotion but incorrectly as an inverted
		// animation speed modifier, with 432 standing for 1x animation speed.
		// The client will ignore all values above 432, but lower values will speed up the animation.
		// 216 for example means play the animation at double the speed. 108 is quadruple speed.
		// Each monster has an attack animation and may define the frame in the attack animation on which
		// it displays the damage and makes the target flinch / stop. If the damage frame is undefined,
		// it instead displays the damage / flinch / stop at the beginning of the second to last frame.
		// We define the time after which the damage frame shows at 1x speed as clientamotion.
		uint16 clientamotion = std::max((uint16)1, status_get_clientamotion(src));

		// Knowing when the damage frame happens in the animation allows us to synchronize the timing
		// between client and server using the formula below.
		sdelay = sdelay * DEFAULT_ANIMATION_SPEED / clientamotion;

		// Hint: If amotion is larger than clientamotion this results in a value above 432 which makes the
		// client display the attack at 1x speed. In this case we need to shorten the delay damage timer
		// on the server to clientamotion ms instead (see battle_delay_damage).
		sdelay = std::min(sdelay, DEFAULT_ANIMATION_SPEED);
	}

	WBUFW(buf,0) = cmd;
	WBUFL(buf,2) = src->id;
	WBUFL(buf,6) = dst->id;
	WBUFL(buf,10) = client_tick(tick);
	WBUFL(buf,14) = sdelay;
	WBUFL(buf,18) = ddelay;
	if (battle_config.hide_woe_damage && map_flag_gvg(src->m)) {
#if PACKETVER < 20071113
		WBUFW(buf,22) = damage ? div : 0;
		WBUFW(buf,27+offset) = damage2 ? div : 0;
#else
		WBUFL(buf, 22) = damage ? div : 0;
		WBUFL(buf, 27 + offset) = damage2 ? div : 0;
#endif
	} else {
#if PACKETVER < 20071113
		WBUFW(buf,22) = min(damage, INT16_MAX);
		WBUFW(buf,27+offset) = damage2;
#else
		WBUFL(buf,22) = damage;
		WBUFL(buf,27+offset) = damage2;
#endif
	}
#if PACKETVER >= 20131223
	WBUFB(buf,26) = (spdamage) ? 1 : 0; // IsSPDamage - Displays blue digits.
#endif
	WBUFW(buf,24+offset) = div;
	WBUFB(buf,26+offset) = type;

	if(disguised(dst)) {
		clif_send(buf, packet_len(cmd), dst, AREA_WOS);
		WBUFL(buf,6) = disguised_bl_id( dst->id );
		clif_send(buf, packet_len(cmd), dst, SELF);
	} else
		clif_send(buf, packet_len(cmd), dst, AREA);

	if(disguised(src)) {
		WBUFL(buf,2) = disguised_bl_id( src->id );
		if (disguised(dst))
			WBUFL(buf,6) = dst->id;
#if PACKETVER < 20071113
		if(damage > 0) WBUFW(buf,22) = -1;
		if(damage2 > 0) WBUFW(buf,27) = -1;
#else
		if(damage > 0) WBUFL(buf,22) = -1;
		if(damage2 > 0) WBUFL(buf,27+offset) = -1;
#endif
		clif_send(buf,packet_len(cmd),src,SELF);
	}

	if(src == dst) {
		unit_setdir(src, unit_getdir(src));
	}

	// In case this assignment is bypassed by DMG_MULTI_HIT_CRITICAL
	type = clif_calc_delay(type, div, damage + damage2, ddelay);
	//Return adjusted can't walk delay for further processing.
	return clif_calc_walkdelay(dst, ddelay, type, damage+damage2, div);
}

/*==========================================
 * src picks up dst
 *------------------------------------------*/
void clif_takeitem(struct block_list* src, struct block_list* dst)
{
	//clif_damage(src,dst,0,0,0,0,0,DMG_PICKUP_ITEM,0,false);
	unsigned char buf[32];

	nullpo_retv(src);
	nullpo_retv(dst);

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = src->id;
	WBUFL(buf, 6) = dst->id;
	WBUFB(buf,26) = 1;
	clif_send(buf, packet_len(0x8a), src, AREA);

}

/*==========================================
 * inform clients in area that `bl` is sitting
 *------------------------------------------*/
void clif_sitting(struct block_list* bl)
{
	unsigned char buf[32];
	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = bl->id;
	WBUFB(buf,26) = 2;
	clif_send(buf, packet_len(0x8a), bl, AREA);

	if(disguised(bl)) {
		WBUFL(buf, 2) = disguised_bl_id( bl->id );
		clif_send(buf, packet_len(0x8a), bl, SELF);
	}
}

/*==========================================
 * inform clients in area that `bl` is standing
 *------------------------------------------*/
void clif_standing(struct block_list* bl)
{
	unsigned char buf[32];
	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = bl->id;
	WBUFB(buf,26) = 3;
	clif_send(buf, packet_len(0x8a), bl, AREA);

	if(disguised(bl)) {
		WBUFL(buf, 2) = disguised_bl_id( bl->id );
		clif_send(buf, packet_len(0x8a), bl, SELF);
	}
}


/// Inform client(s) about a map-cell change (ZC_UPDATE_MAPINFO).
/// 0192 <x>.W <y>.W <type>.W <map name>.16B
void clif_changemapcell(int fd, int16 m, int x, int y, int type, enum send_target target)
{
	unsigned char buf[32];

	WBUFW(buf,0) = 0x192;
	WBUFW(buf,2) = x;
	WBUFW(buf,4) = y;
	WBUFW(buf,6) = type;
	mapindex_getmapname_ext(map_mapid2mapname(m),WBUFCP(buf,8));

	if( session_isActive(fd) )
	{
		WFIFOHEAD(fd,packet_len(0x192));
		memcpy(WFIFOP(fd,0), buf, packet_len(0x192));
		WFIFOSET(fd,packet_len(0x192));
	}
	else
	{
		struct block_list dummy_bl;
		dummy_bl.type = BL_NUL;
		dummy_bl.x = x;
		dummy_bl.y = y;
		dummy_bl.m = m;
		clif_send(buf,packet_len(0x192),&dummy_bl,target);
	}
}


/// Notifies the client about an item on floor (ZC_ITEM_ENTRY).
/// 009d <id>.L <name id>.W <identified>.B <x>.W <y>.W <amount>.W <subX>.B <subY>.B
void clif_getareachar_item( map_session_data* sd,struct flooritem_data* fitem ){
	nullpo_retv( sd );
	nullpo_retv( fitem );

	PACKET_ZC_ITEM_ENTRY p = {};

	p.packetType = HEADER_ZC_ITEM_ENTRY;
	p.AID = fitem->bl.id;
	p.itemId = client_nameid( fitem->item.nameid );
	p.identify = fitem->item.identify;
	p.x = fitem->bl.x;
	p.y = fitem->bl.y;
	p.amount = fitem->item.amount;
	p.subX = fitem->subx;
	p.subY = fitem->suby;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}

/// Notifes client about Graffiti
/// 01c9 <id>.L <creator id>.L <x>.W <y>.W <unit id>.B <visible>.B <has msg>.B <msg>.80B (ZC_SKILL_ENTRY2)
static void clif_graffiti(struct block_list *bl, struct skill_unit *unit, enum send_target target) {
	unsigned char buf[128];
	
	nullpo_retv(bl);
	nullpo_retv(unit);

	WBUFW(buf, 0) = 0x1c9;
	WBUFL(buf, 2) = unit->bl.id;
	WBUFL(buf, 6) = unit->group->src_id;
	WBUFW(buf,10) = unit->bl.x;
	WBUFW(buf,12) = unit->bl.y;
	WBUFB(buf,14) = unit->group->unit_id;
	WBUFB(buf,15) = 1;
	WBUFB(buf,16) = 1;
	safestrncpy(WBUFCP(buf,17),unit->group->valstr,MESSAGE_SIZE);
	clif_send(buf,packet_len(0x1c9),bl,target);
}

/// Notifies the client of a skill unit.
/// 011f <id>.L <creator id>.L <x>.W <y>.W <unit id>.B <visible>.B (ZC_SKILL_ENTRY)
/// 08c7 <lenght>.W <id> L <creator id>.L <x>.W <y>.W <unit id>.B <range>.W <visible>.B (ZC_SKILL_ENTRY3)
/// 099f <lenght>.W <id> L <creator id>.L <x>.W <y>.W <unit id>.L <range>.W <visible>.B (ZC_SKILL_ENTRY4)
/// 09ca <lenght>.W <id> L <creator id>.L <x>.W <y>.W <unit id>.L <range>.B <visible>.B <skill level>.B (ZC_SKILL_ENTRY5)
void clif_getareachar_skillunit(struct block_list *bl, struct skill_unit *unit, enum send_target target, bool visible) {
	int header = 0, unit_id = 0, pos = 0, fd = 0, len = -1;
	unsigned char buf[128];

	nullpo_retv(bl);
	nullpo_retv(unit);

	if (bl->type == BL_PC)
		fd = ((TBL_PC*)bl)->fd;

	if (unit->group->state.guildaura)
		return;

	if (unit->group->state.song_dance&0x1 && unit->val2&(1 << UF_ENSEMBLE))
		unit_id = unit->val2&(1 << UF_SONG) ? UNT_DISSONANCE : UNT_UGLYDANCE;
	else if (skill_get_unit_flag(unit->group->skill_id, UF_RANGEDSINGLEUNIT) && !(unit->val2 & (1 << UF_RANGEDSINGLEUNIT)))
		unit_id = UNT_DUMMYSKILL; // Use invisible unit id for other case of rangedsingle unit
	else
		unit_id = unit->group->unit_id;

	if (!visible)
		unit_id = UNT_DUMMYSKILL; // Hack to makes hidden trap really hidden!

#if PACKETVER >= 3
	if (unit_id == UNT_GRAFFITI) { // Graffiti [Valaris]
		clif_graffiti(bl, unit, target);
		return;
	}
#endif

#if PACKETVER <= 20120702
	header = 0x011f;
//#if PACKETVER < 20110718
//	header = 0x011f;
//#elif PACKETVER < 20121212
//	header = 0x08c7;
#elif PACKETVER < 20130731
	header = 0x099f;
#else
	header = 0x09ca;
#endif

	len = packet_len(header);
	WBUFW(buf,pos) = header;
	if (header != 0x011f) {
		WBUFW(buf, pos+2) = len;
		pos += 2;
	}
	WBUFL(buf,pos+2) = unit->bl.id;
	WBUFL(buf,pos+6) = unit->group->src_id;
	WBUFW(buf,pos+10) = unit->bl.x;
	WBUFW(buf,pos+12) = unit->bl.y;
	switch (header) {
		case 0x011f:
			WBUFB(buf,pos+14) = unit_id;
			WBUFB(buf,pos+15) = visible;
			break;
		case 0x08c7:
			WBUFB(buf,pos+14) = unit_id;
			WBUFW(buf,pos+15) = unit->range;
			WBUFB(buf,pos+17) = visible;
			break;
		case 0x099f:
			WBUFL(buf,pos+14) = unit_id;
			WBUFW(buf,pos+18) = unit->range;
			WBUFB(buf,pos+20) = visible;
			break;
		case 0x09ca:
			WBUFL(buf,pos+14) = unit_id;
			WBUFB(buf,pos+18) = (unsigned char)unit->range;
			WBUFB(buf,pos+19) = visible;
			WBUFB(buf,pos+20) = (unsigned char)unit->group->skill_lv;
			break;
	}
	clif_send(buf, len, bl, target);

	if (unit->group->skill_id == WZ_ICEWALL)
		clif_changemapcell(fd, unit->bl.m, unit->bl.x, unit->bl.y, 5, SELF);
}

/// 09ca <lenght>.W <id> L <creator id>.L <x>.W <y>.W <unit id>.L <range>.B <visible>.B <skill level>.B (ZC_SKILL_ENTRY5)
void clif_skill_unit_test(struct block_list *bl, short x, short y, int unit_id, short range, short skill_lv) {
	unsigned char buf[128];

	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x09ca;
	WBUFW(buf, 2) = packet_len(0x09ca);
	WBUFL(buf, 4) = 1000;
	WBUFL(buf, 8) = 2000;
	WBUFW(buf, 12) = x;
	WBUFW(buf, 14) = y;
	WBUFL(buf, 16) = unit_id;
	WBUFB(buf, 20) = (unsigned char)range;
	WBUFB(buf, 21) = 1;
	WBUFB(buf, 22) = (unsigned char)skill_lv;

	clif_send(buf, packet_len(0x09ca), bl, AREA);
}

/// Server tells client to remove unit of id 'unit->bl.id'
/// 0120 <id>.L (ZC_SKILL_DISAPPEAR)
static void clif_clearchar_skillunit( skill_unit& unit, map_session_data& sd ){
	PACKET_ZC_SKILL_DISAPPEAR packet{};

	packet.packetType = HEADER_ZC_SKILL_DISAPPEAR;
	packet.GID = unit.bl.id;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );

	if( unit.group && unit.group->skill_id == WZ_ICEWALL ){
		clif_changemapcell( sd.fd, unit.bl.m, unit.bl.x, unit.bl.y, unit.val2, SELF );
	}
}


/// Removes a skill unit.
/// 0120 <id>.L (ZC_SKILL_DISAPPEAR)
void clif_skill_delunit( skill_unit& unit ){
	PACKET_ZC_SKILL_DISAPPEAR packet{};

	packet.packetType = HEADER_ZC_SKILL_DISAPPEAR;
	packet.GID = unit.bl.id;

	clif_send( &packet, sizeof( packet ), &unit.bl, AREA );
}


/// Sent when an object gets ankle-snared.
/// 01ac <id>.L (ZC_SKILL_UPDATE)
/// Only affects units with class [139,153] client-side.
void clif_skillunit_update( block_list& bl ){
	PACKET_ZC_SKILL_UPDATE packet{};

	packet.packetType = HEADER_ZC_SKILL_UPDATE;
	packet.GID = bl.id;

	clif_send( &packet, sizeof( packet ), &bl, AREA );
}


/*==========================================
 *
 *------------------------------------------*/
static int clif_getareachar(struct block_list* bl,va_list ap)
{
	map_session_data *sd;

	nullpo_ret(bl);

	sd=va_arg(ap,map_session_data*);

	if (!clif_session_isValid(sd))
		return 0;

	switch(bl->type){
	case BL_ITEM:
		clif_getareachar_item(sd,(struct flooritem_data*) bl);
		break;
	case BL_SKILL:
		skill_getareachar_skillunit_visibilty_single((TBL_SKILL*)bl, &sd->bl);
		break;
	default:
		if(&sd->bl == bl)
			break;
		clif_getareachar_unit(sd,bl);
		break;
	}
	return 0;
}

/*==========================================
 * tbl has gone out of view-size of bl
 *------------------------------------------*/
int clif_outsight(struct block_list *bl,va_list ap)
{
	struct block_list *tbl;
	struct view_data *vd;
	TBL_PC *sd, *tsd;
	tbl=va_arg(ap,struct block_list*);
	if(bl == tbl) return 0;
	sd = BL_CAST(BL_PC, bl);
	tsd = BL_CAST(BL_PC, tbl);

	if (clif_session_isValid(tsd)) { //tsd has lost sight of the bl object.
		nullpo_ret(bl);
		switch(bl->type){
		case BL_PC:
			if(sd->vd.class_ != JT_INVISIBLE)
				clif_clearunit_single( bl->id, CLR_OUTSIGHT, *tsd );
			if(sd->chatID){
				struct chat_data *cd;
				cd=(struct chat_data*)map_id2bl(sd->chatID);
				if(cd->usersd[0]==sd)
					clif_dispchat(cd,tsd->fd);
			}
			if(sd->state.vending)
				clif_closevendingboard(bl,tsd->fd);
			if(sd->state.buyingstore)
				clif_buyingstore_disappear_entry( *sd, &tsd->bl );
			break;
		case BL_ITEM:
			clif_clearflooritem( *reinterpret_cast<flooritem_data*>( bl ), tsd );
			break;
		case BL_SKILL:
			clif_clearchar_skillunit( *((skill_unit *)bl), *tsd );
			break;
		case BL_NPC:
			if(!(((TBL_NPC*)bl)->is_invisible))
				clif_clearunit_single( bl->id, CLR_OUTSIGHT, *tsd );
			break;
		default:
			if((vd=status_get_viewdata(bl)) && vd->class_ != JT_INVISIBLE)
				clif_clearunit_single( bl->id, CLR_OUTSIGHT, *tsd );
			break;
		}
	}
	if (clif_session_isValid(sd)) { //sd is watching tbl go out of view.
		nullpo_ret(tbl);
		if(tbl->type == BL_SKILL) //Trap knocked out of sight
			clif_clearchar_skillunit( *((skill_unit *)tbl), *sd );
		else if(((vd=status_get_viewdata(tbl)) && vd->class_ != JT_INVISIBLE) &&
			!(tbl->type == BL_NPC && (((TBL_NPC*)tbl)->is_invisible)))
			clif_clearunit_single( tbl->id, CLR_OUTSIGHT, *sd );
	}
	return 0;
}

/*==========================================
 * tbl has come into view of bl
 *------------------------------------------*/
int clif_insight(struct block_list *bl,va_list ap)
{
	struct block_list *tbl;
	TBL_PC *sd, *tsd;
	tbl=va_arg(ap,struct block_list*);

	if (bl == tbl) return 0;

	sd = BL_CAST(BL_PC, bl);
	tsd = BL_CAST(BL_PC, tbl);

	if (clif_session_isValid(tsd)) { //Tell tsd that bl entered into his view
		switch(bl->type){
		case BL_ITEM:
			clif_getareachar_item(tsd,(struct flooritem_data*)bl);
			break;
		case BL_SKILL:
			skill_getareachar_skillunit_visibilty_single((TBL_SKILL*)bl, &tsd->bl);
			break;
		default:
			clif_getareachar_unit(tsd,bl);
			break;
		}
	}
	if (clif_session_isValid(sd)) { //Tell sd that tbl walked into his view
		clif_getareachar_unit(sd,tbl);
	}
	return 0;
}


/// Updates whole skill tree (ZC_SKILLINFO_LIST).
/// 010f <packet len>.W { <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <skill name>.24B <upgradable>.B }*
void clif_skillinfoblock(map_session_data *sd)
{
	int fd;
	int i,len,id;

	nullpo_retv(sd);

	fd = sd->fd;
	if (!session_isActive(fd))
		return;

	WFIFOHEAD(fd, MAX_SKILL * 37 + 4);
	WFIFOW(fd,0) = 0x10f;
	bool haveCallPartnerSkill = false;
	for ( i = 0, len = 4; i < MAX_SKILL; i++)
	{
		if( (id = sd->status.skill[i].id) != 0 )
		{
			// workaround for bugreport:5348
			if (len + 37 > 8192)
				break;
			
			// skip WE_CALLPARTNER and send it in special way
			if (id == WE_CALLPARTNER) {
				haveCallPartnerSkill = true;
				continue;
			}
			WFIFOW(fd,len)   = id;
			WFIFOL(fd,len+2) = skill_get_inf(id);
			WFIFOW(fd,len+6) = sd->status.skill[i].lv;
			WFIFOW(fd,len+8) = skill_get_sp(id,sd->status.skill[i].lv);
			WFIFOW(fd,len+10)= skill_get_range2(&sd->bl,id,sd->status.skill[i].lv,false);
			safestrncpy(WFIFOCP(fd,len+12), skill_get_name(id), NAME_LENGTH);
			if(sd->status.skill[i].flag == SKILL_FLAG_PERMANENT)
				WFIFOB(fd,len+36) = (sd->status.skill[i].lv < skill_tree_get_max(id, sd->status.class_))? 1:0;
			else
				WFIFOB(fd,len+36) = 0;
			len += 37;
		}
	}
	WFIFOW(fd,2)=len;
	WFIFOSET(fd,len);

	// adoption fix
	if (haveCallPartnerSkill) {
		clif_addskill(sd, WE_CALLPARTNER);
		clif_skillinfo(sd, WE_CALLPARTNER, 0);
	}

	// workaround for bugreport:5348; send the remaining skills one by one to bypass packet size limit
	for ( ; i < MAX_SKILL; i++)
	{
		if( (id = sd->status.skill[i].id) != 0 && ( id != WE_CALLPARTNER || !haveCallPartnerSkill ) )
		{
			clif_addskill(sd, id);
			clif_skillinfo(sd, id, 0);
		}
	}
}
/**
 * Server tells client 'sd' to add skill of id 'id' to it's skill tree (e.g. with Ice Falcion item)
 **/

/// Adds new skill to the skill tree (ZC_ADD_SKILL).
/// 0111 <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <skill name>.24B <upgradable>.B
void clif_addskill(map_session_data *sd, int skill_id)
{
	nullpo_retv(sd);

	int fd = sd->fd;
	uint16 idx = skill_get_index(skill_id);

	if (!session_isActive(fd) || !idx)
		return;

	if( sd->status.skill[idx].id <= 0 )
		return;

	WFIFOHEAD(fd, packet_len(0x111));
	WFIFOW(fd,0) = 0x111;
	WFIFOW(fd,2) = skill_id;
	WFIFOL(fd,4) = skill_get_inf(skill_id);
	WFIFOW(fd,8) = sd->status.skill[idx].lv;
	WFIFOW(fd,10) = skill_get_sp(skill_id,sd->status.skill[idx].lv);
	WFIFOW(fd,12)= skill_get_range2(&sd->bl,skill_id,sd->status.skill[idx].lv,false);
	safestrncpy(WFIFOCP(fd,14), skill_get_name(skill_id), NAME_LENGTH);
	if( sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT )
		WFIFOB(fd,38) = (sd->status.skill[idx].lv < skill_tree_get_max(skill_id, sd->status.class_))? 1:0;
	else
		WFIFOB(fd,38) = 0;
	WFIFOSET(fd,packet_len(0x111));
}


/// Deletes a skill from the skill tree (ZC_SKILLINFO_DELETE).
/// 0441 <skill id>.W
void clif_deleteskill(map_session_data *sd, int skill_id, bool skip_infoblock)
{
#if PACKETVER >= 20081217
	nullpo_retv(sd);

	int fd = sd->fd;
	uint16 idx = skill_get_index(skill_id);

	if (!session_isActive(fd) || !idx)
		return;

	WFIFOHEAD(fd,packet_len(0x441));
	WFIFOW(fd,0) = 0x441;
	WFIFOW(fd,2) = skill_id;
	WFIFOSET(fd,packet_len(0x441));
#endif
#if PACKETVER_MAIN_NUM >= 20190807 || PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
	if (!skip_infoblock)
#endif
		clif_skillinfoblock(sd);
}

/// Updates a skill in the skill tree (ZC_SKILLINFO_UPDATE).
/// 010e <skill id>.W <level>.W <sp cost>.W <attack range>.W <upgradable>.B
void clif_skillup(map_session_data *sd, uint16 skill_id, int lv, int range, int upgradable) {
	nullpo_retv(sd);

	int fd = sd->fd;
	uint16 idx = skill_get_index(skill_id);

	if (!session_isActive(fd) || !idx)
		return;
	
	WFIFOHEAD(fd, packet_len(0x10e));
	WFIFOW(fd, 0) = 0x10e;
	WFIFOW(fd, 2) = skill_id;
	WFIFOW(fd, 4) = lv;
	WFIFOW(fd, 6) = skill_get_sp(skill_id, lv);
	WFIFOW(fd, 8) = range;
	WFIFOB(fd, 10) = upgradable;
	WFIFOSET(fd, packet_len(0x10e));
}


/// Updates a skill in the skill tree (ZC_SKILLINFO_UPDATE2).
/// 07e1 <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <upgradable>.B
void clif_skillinfo(map_session_data *sd,int skill_id, int inf)
{
	nullpo_retv(sd);

	const int fd = sd->fd;
	uint16 idx = skill_get_index(skill_id);

	if (!session_isActive(fd) || !idx)
		return;

	WFIFOHEAD(fd,packet_len(0x7e1));
	WFIFOW(fd,0) = 0x7e1;
	WFIFOW(fd,2) = skill_id;
	WFIFOL(fd,4) = inf?inf:skill_get_inf(skill_id);
	WFIFOW(fd,8) = sd->status.skill[idx].lv;
	WFIFOW(fd,10) = skill_get_sp(skill_id,sd->status.skill[idx].lv);
	WFIFOW(fd,12) = skill_get_range2(&sd->bl,skill_id,sd->status.skill[idx].lv,false);
	if( sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT )
		WFIFOB(fd,14) = (sd->status.skill[idx].lv < skill_tree_get_max(skill_id, sd->status.class_))? 1:0;
	else
		WFIFOB(fd,14) = 0;
	WFIFOSET(fd,packet_len(0x7e1));
}

void clif_skill_scale( struct block_list *bl, int src_id, int x, int y, uint16 skill_id, uint16 skill_lv, int casttime ){
#if PACKETVER >= 20151223
	if( !battle_config.show_skill_scale ){
		return;
	}

	PACKET_ZC_SKILL_SCALE p = {};

	p.PacketType = skillscale;
	p.AID = src_id;
	p.skill_id = skill_id;
	p.skill_lv = skill_lv;
	p.x = x;
	p.y = y;
	p.casttime = casttime;

	if( disguised( bl ) ){
		clif_send( &p, sizeof( p ), bl, AREA_WOS );
		p.AID = disguised_bl_id( bl->id );
		clif_send( &p, sizeof( p ), bl, SELF );
	}else{
		clif_send( &p, sizeof( p ), bl, AREA );
	}
#endif
}

/// Notifies clients in area, that an object is about to use a skill.
/// 013e <src id>.L <dst id>.L <x>.W <y>.W <skill id>.W <property>.L <delaytime>.L (ZC_USESKILL_ACK)
/// 07fb <src id>.L <dst id>.L <x>.W <y>.W <skill id>.W <property>.L <delaytime>.L <is disposable>.B (ZC_USESKILL_ACK2)
/// property:
///     0 = Yellow cast aura
///     1 = Water elemental cast aura
///     2 = Earth elemental cast aura
///     3 = Fire elemental cast aura
///     4 = Wind elemental cast aura
///     5 = Poison elemental cast aura
///     6 = Holy elemental cast aura
///     ? = like 0
/// is disposable:
///     0 = yellow chat text "[src name] will use skill [skill name]."
///     1 = no text
void clif_skillcasting(struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, uint16 skill_lv, int property, int casttime)
{
#if PACKETVER < 20091124
	const int cmd = 0x13e;
#else
	const int cmd = 0x7fb;
#endif
	unsigned char buf[32];

	WBUFW(buf,0) = cmd;
	WBUFL(buf,2) = src_id;
	WBUFL(buf,6) = dst_id;
	WBUFW(buf,10) = dst_x;
	WBUFW(buf,12) = dst_y;
	WBUFW(buf,14) = skill_id;
	WBUFL(buf,16) = property<0?0:property; //Avoid sending negatives as element [Skotlex]
	WBUFL(buf,20) = casttime;
#if PACKETVER >= 20091124
	WBUFB(buf,24) = 0;  // isDisposable
#endif

	if (disguised(bl)) {
		clif_send(buf,packet_len(cmd), bl, AREA_WOS);
		WBUFL(buf,2) = disguised_bl_id( src_id );
		clif_send(buf,packet_len(cmd), bl, SELF);
	} else
		clif_send(buf,packet_len(cmd), bl, AREA);

	if( skill_get_inf2( skill_id, INF2_SHOWSCALE ) ){
		clif_skill_scale( bl, src_id, bl->x, bl->y, skill_id, skill_lv, casttime );
	}
}


/// Notifies clients in area, that an object canceled casting.
/// 01b9 <id>.L (ZC_DISPEL)
void clif_skillcastcancel( block_list& bl ){
	PACKET_ZC_DISPEL packet{};

	packet.packetType = HEADER_ZC_DISPEL;
	packet.gid = bl.id;

	clif_send( &packet, sizeof(PACKET_ZC_DISPEL), &bl, AREA );
}


/// Notifies the client about the result of a skill use request (ZC_ACK_TOUSESKILL).
/// 0110 <skill id>.W <num>.L <result>.B <cause>.B
/// num (only used when skill id = NV_BASIC and cause = 0):
///     0 = "skill failed" MsgStringTable[159]
///     1 = "no emotions" MsgStringTable[160]
///     2 = "no sit" MsgStringTable[161]
///     3 = "no chat" MsgStringTable[162]
///     4 = "no party" MsgStringTable[163]
///     5 = "no shout" MsgStringTable[164]
///     6 = "no PKing" MsgStringTable[165]
///     7 = "no alligning" MsgStringTable[383]
///     ? = ignored
/// cause:
///     0 = "not enough skill level" MsgStringTable[214] (AL_WARP)
///         "steal failed" MsgStringTable[205] (TF_STEAL)
///         "envenom failed" MsgStringTable[207] (TF_POISON)
///         "skill failed" MsgStringTable[204] (otherwise)
///   ... = @see enum useskill_fail_cause
///     ? = ignored
///
/// if(result!=0) doesn't display any of the previous messages
/// Note: when this packet is received an unknown flag is always set to 0,
/// suggesting this is an ACK packet for the UseSkill packets and should be sent on success too [FlavioJS]
void clif_skill_fail( map_session_data& sd, uint16 skill_id, enum useskill_fail_cause cause, int btype, t_itemid itemId ){
	if(battle_config.display_skill_fail&1)
		return; //Disable all skill failed messages

	if(cause==USESKILL_FAIL_SKILLINTERVAL && !sd.state.showdelay)
		return; //Disable delay failed messages

	if(skill_id == RG_SNATCHER && battle_config.display_skill_fail&4)
		return;

	if(skill_id == TF_POISON && battle_config.display_skill_fail&8)
		return;

	PACKET_ZC_ACK_TOUSESKILL p = {};

	p.packetType = HEADER_ZC_ACK_TOUSESKILL;
	p.skillId = skill_id;
	p.btype = btype;
	p.itemId = itemId;
	p.flag = 0; // 0 - failed
	p.cause = cause;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Skill cooldown display icon.
/// 043d <skill ID>.W <tick>.L (ZC_SKILL_POSTDELAY)
void clif_skill_cooldown( map_session_data &sd, uint16 skill_id, t_tick tick ){
#if PACKETVER_MAIN_NUM >= 20081112 || PACKETVER_RE_NUM >= 20081111 || defined(PACKETVER_ZERO)
	PACKET_ZC_SKILL_POSTDELAY packet{};

	packet.PacketType = HEADER_ZC_SKILL_POSTDELAY;
	packet.SKID = skill_id;
	packet.DelayTM = client_tick(tick);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
#endif
}

/// Skill attack effect and damage.
/// 0114 <skill id>.W <src id>.L <dst id>.L <tick>.L <src delay>.L <dst delay>.L <damage>.W <level>.W <div>.W <type>.B (ZC_NOTIFY_SKILL)
/// 01de <skill id>.W <src id>.L <dst id>.L <tick>.L <src delay>.L <dst delay>.L <damage>.L <level>.W <div>.W <type>.B (ZC_NOTIFY_SKILL2)
int clif_skill_damage(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int64 sdamage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type)
{
	unsigned char buf[64];
	status_change *sc;
	int damage = (int)cap_value(sdamage,INT_MIN,INT_MAX);

	nullpo_ret(src);
	nullpo_ret(dst);

	type = clif_calc_delay(type,div,damage,ddelay);

	if( ( sc = status_get_sc(dst) ) && sc->count ) {
		if(sc->getSCE(SC_HALLUCINATION) && damage)
			damage = clif_hallucination_damage();
	}

#if PACKETVER < 3
	WBUFW(buf,0)=0x114;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFL(buf,8)=dst->id;
	WBUFL(buf,12)=client_tick(tick);
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	if (battle_config.hide_woe_damage && map_flag_gvg(src->m)) {
		WBUFW(buf,24)=damage?div:0;
	} else {
		WBUFW(buf,24)=damage;
	}
	WBUFW(buf,26)=skill_lv;
	WBUFW(buf,28)=div;
	WBUFB(buf,30)=type;
	if (disguised(dst)) {
		clif_send(buf,packet_len(0x114),dst,AREA_WOS);
		WBUFL(buf,8)=disguised_bl_id(dst->id);
		clif_send(buf,packet_len(0x114),dst,SELF);
	} else
		clif_send(buf,packet_len(0x114),dst,AREA);

	if(disguised(src)) {
		WBUFL(buf,4)=disguised_bl_id(src->id);
		if (disguised(dst))
			WBUFL(buf,8)=dst->id;
		if(damage > 0)
			WBUFW(buf,24)=-1;
		clif_send(buf,packet_len(0x114),src,SELF);
	}
#else
	WBUFW(buf,0)=0x1de;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFL(buf,8)=dst->id;
	WBUFL(buf,12)=client_tick(tick);
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	if (battle_config.hide_woe_damage && map_flag_gvg(src->m)) {
		WBUFL(buf,24)=damage?div:0;
	} else {
		WBUFL(buf,24)=damage;
	}
	WBUFW(buf,28)=skill_lv;
	WBUFW(buf,30)=div;
	// For some reason, late 2013 and newer clients have
	// a issue that causes players and monsters to endure
	// type 6 (ACTION_SKILL) skills. So we have to do a small
	// hack to set all type 6 to be sent as type 8 ACTION_ATTACK_MULTIPLE
#if PACKETVER < 20131223
	WBUFB(buf,32)=type;
#else
	WBUFB(buf,32)=( type == DMG_SINGLE ) ? DMG_MULTI_HIT : type;
#endif
	if (disguised(dst)) {
		clif_send(buf,packet_len(0x1de),dst,AREA_WOS);
		WBUFL(buf,8)=disguised_bl_id(dst->id);
		clif_send(buf,packet_len(0x1de),dst,SELF);
	} else
		clif_send(buf,packet_len(0x1de),dst,AREA);

	if(disguised(src)) {
		WBUFL(buf,4)=disguised_bl_id(src->id);
		if (disguised(dst))
			WBUFL(buf,8)=dst->id;
		if(damage > 0)
			WBUFL(buf,24)=-1;
		clif_send(buf,packet_len(0x1de),src,SELF);
	}
#endif

	//Because the damage delay must be synced with the client, here is where the can-walk tick must be updated. [Skotlex]
	return clif_calc_walkdelay(dst,ddelay,type,damage,div);
}


/// Ground skill attack effect and damage (ZC_NOTIFY_SKILL_POSITION).
/// 0115 <skill id>.W <src id>.L <dst id>.L <tick>.L <src delay>.L <dst delay>.L <x>.W <y>.W <damage>.W <level>.W <div>.W <type>.B
/*
int clif_skill_damage2(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int damage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type)
{
	unsigned char buf[64];
	status_change *sc;

	nullpo_ret(src);
	nullpo_ret(dst);

	type = (type>DMG_NORMAL)?type:skill_get_hit(skill_id);
	type = clif_calc_delay(type,div,damage,ddelay);
	sc = status_get_sc(dst);

	if(sc && sc->count) {
		if(sc->getSCE(SC_HALLUCINATION) && damage)
			damage = clif_hallucination_damage();
	}

	WBUFW(buf,0)=0x115;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFL(buf,8)=dst->id;
	WBUFL(buf,12)=client_tick(tick);
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	WBUFW(buf,24)=dst->x;
	WBUFW(buf,26)=dst->y;
	if (battle_config.hide_woe_damage && map_flag_gvg(src->m)) {
		WBUFW(buf,28)=damage?div:0;
	} else {
		WBUFW(buf,28)=damage;
	}
	WBUFW(buf,30)=skill_lv;
	WBUFW(buf,32)=div;
	WBUFB(buf,34)=type;
	clif_send(buf,packet_len(0x115),src,AREA);
	if(disguised(src)) {
		WBUFL(buf,4)=disguised_bl_id(src->id);
		if(damage > 0)
			WBUFW(buf,28)=-1;
		clif_send(buf,packet_len(0x115),src,SELF);
	}
	if (disguised(dst)) {
		WBUFL(buf,8)=disguised_bl_id(dst->id);
		if (disguised(src))
			WBUFL(buf,4)=src->id;
		else if(damage > 0)
			WBUFW(buf,28)=-1;
		clif_send(buf,packet_len(0x115),dst,SELF);
	}

	//Because the damage delay must be synced with the client, here is where the can-walk tick must be updated. [Skotlex]
	return clif_calc_walkdelay(dst,ddelay,type,damage,div);
}
*/


/// Non-damaging skill effect
/// 011a <skill id>.W <heal>.W <dst id>.L <src id>.L <result>.B (ZC_USE_SKILL).
/// 09cb <skill id>.W <heal>.L <dst id>.L <src id>.L <result>.B (ZC_USE_SKILL2).
bool clif_skill_nodamage(struct block_list *src,struct block_list *dst, uint16 skill_id, int heal, t_tick tick)
{
	unsigned char buf[17];
#if PACKETVER < 20130731
	const int cmd = 0x11a;
#else
	const int cmd = 0x9cb;
#endif
	int offset = 0;
	bool success = ( tick != 0 );

	nullpo_ret(dst);

	WBUFW(buf,0) = cmd;
	WBUFW(buf,2) = skill_id;
#if PACKETVER < 20130731
	WBUFW(buf,4) = min(heal, INT16_MAX);
#else
	WBUFL(buf,4) = min(heal, INT32_MAX);
	offset += 2;
#endif
	WBUFL(buf,6+offset) = dst->id;
	WBUFL(buf,10+offset) = src ? src->id : 0;
	WBUFB(buf,14+offset) = success;

	if (disguised(dst)) {
		clif_send(buf, packet_len(cmd), dst, AREA_WOS);
		WBUFL(buf,6+offset) = disguised_bl_id(dst->id);
		clif_send(buf, packet_len(cmd), dst, SELF);
	} else
		clif_send(buf, packet_len(cmd), dst, AREA);

	if(src && disguised(src)) {
		WBUFL(buf,10+offset) = disguised_bl_id(src->id);
		if (disguised(dst))
			WBUFL(buf,6+offset) = dst->id;
		clif_send(buf, packet_len(cmd), src, SELF);
	}

	return success;
}


/// Non-damaging ground skill effect (ZC_NOTIFY_GROUNDSKILL).
/// 0117 <skill id>.W <src id>.L <level>.W <x>.W <y>.W <tick>.L
void clif_skill_poseffect(struct block_list *src,uint16 skill_id,int val,int x,int y,t_tick tick)
{
	unsigned char buf[32];

	nullpo_retv(src);

	WBUFW(buf,0)=0x117;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFW(buf,8)=val;
	WBUFW(buf,10)=x;
	WBUFW(buf,12)=y;
	WBUFL(buf,14)=client_tick(tick);
	if(disguised(src)) {
		clif_send(buf,packet_len(0x117),src,AREA_WOS);
		WBUFL(buf,4)=disguised_bl_id(src->id);
		clif_send(buf,packet_len(0x117),src,SELF);
	} else
		clif_send(buf,packet_len(0x117),src,AREA);
}

/// Presents a list of available warp destinations (ZC_WARPLIST).
/// 011c <skill id>.W { <map name>.16B }*4
void clif_skill_warppoint( map_session_data* sd, uint16 skill_id, uint16 skill_lv, const char* map1, const char* map2, const char* map3, const char* map4 ){
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x11c));
	WFIFOW(fd,0) = 0x11c;
	WFIFOW(fd,2) = skill_id;
	memset(WFIFOP(fd,4), 0x00, 4*MAP_NAME_LENGTH_EXT);
	if( strcmp( "", map1 ) != 0 ){
		mapindex_getmapname_ext( map1, WFIFOCP( fd, 4 ) );
	}
	if( strcmp( "", map2 ) != 0 ){
		mapindex_getmapname_ext( map2, WFIFOCP( fd, 20 ) );
	}
	if( strcmp( "", map3 ) != 0 ){
		mapindex_getmapname_ext( map3, WFIFOCP( fd, 36 ) );
	}
	if( strcmp( "", map4 ) != 0 ){
		mapindex_getmapname_ext( map4, WFIFOCP( fd, 52 ) );
	}
	WFIFOSET(fd,packet_len(0x11c));

	sd->menuskill_id = skill_id;
	if (skill_id == AL_WARP) {
		sd->menuskill_val = (sd->ud.skillx<<16)|sd->ud.skilly; //Store warp position here.
		sd->state.workinprogress = WIP_DISABLE_ALL;
	} else
		sd->menuskill_val = skill_lv;
}


/// Memo message.
/// 011e <type>.B (ZC_ACK_REMEMBER_WARPPOINT)
/// type:
///     0 = "Saved location as a Memo Point for Warp skill." in color 0xFFFF00 (cyan)
///     1 = "Skill Level is not high enough." in color 0x0000FF (red)
///     2 = "You haven't learned Warp." in color 0x0000FF (red)
///
/// @param sd Who receives the message
/// @param type What message
void clif_skill_memomessage( map_session_data& sd, e_ack_remember_warppoint_result result ){
	PACKET_ZC_ACK_REMEMBER_WARPPOINT packet{};

	packet.packetType = HEADER_ZC_ACK_REMEMBER_WARPPOINT;
	packet.type = static_cast<decltype(packet.type)>(result);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Teleport message.
/// 0189 <type>.W (ZC_NOTIFY_MAPINFO)
/// type:
///     0 = "Unable to Teleport in this area" in color 0xFFFF00 (cyan)
///     1 = "Saved point cannot be memorized." in color 0x0000FF (red)
///     2 = "This skill cannot be used in this area"
///     3 = "This item cannot be used in this area"
///
/// @param sd Who receives the message
/// @param type What message
void clif_skill_teleportmessage( map_session_data& sd, e_notify_mapinfo_result result ){
	PACKET_ZC_NOTIFY_MAPINFO packet{};

	packet.packetType = HEADER_ZC_NOTIFY_MAPINFO;
	packet.type = static_cast<decltype(packet.type)>(result);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Displays Sense (WZ_ESTIMATION) information window (ZC_MONSTER_INFO).
/// 018c <class>.W <level>.W <size>.W <hp>.L <def>.W <race>.W <mdef>.W <element>.W
///     <water%>.B <earth%>.B <fire%>.B <wind%>.B <poison%>.B <holy%>.B <shadow%>.B <ghost%>.B <undead%>.B
void clif_skill_estimation(map_session_data *sd,struct block_list *dst)
{
	struct status_data *status;
	unsigned char buf[64];
	int i, fix;

	nullpo_retv(sd);
	nullpo_retv(dst);

	if( dst->type != BL_MOB )
		return;

	status = status_get_status_data(dst);

	WBUFW(buf, 0)=0x18c;
	WBUFW(buf, 2)=status_get_class(dst);
	WBUFW(buf, 4)=status_get_lv(dst);
	WBUFW(buf, 6)=status->size;
	WBUFL(buf, 8)=status->hp;
	WBUFW(buf,12)= (battle_config.estimation_type&1?status->def:0)
		+(battle_config.estimation_type&2?status->def2:0);
	WBUFW(buf,14)=status->race;
	WBUFW(buf,16)= (battle_config.estimation_type&1?status->mdef:0)
		+(battle_config.estimation_type&2?status->mdef2:0);
	WBUFW(buf,18)= status->def_ele;
	for(i=0;i<9;i++)
//		The following caps negative attributes to 0 since the client displays them as 255-fix. [Skotlex]
		WBUFB(buf,20+i)= (unsigned char)((fix=elemental_attribute_db.getAttribute(status->ele_lv, i+1, status->def_ele))<0?0:fix);

	clif_send(buf,packet_len(0x18c),&sd->bl,sd->status.party_id>0?PARTY_SAMEMAP:SELF);
}


/// Presents a textual list of producable items.
/// 018d <packet len>.W { <name id>.W { <material id>.W }*3 }* (ZC_MAKABLEITEMLIST)
/// material id:
///     unused by the client
void clif_skill_produce_mix_list( map_session_data& sd, int skill_id, int trigger ){
	// Avoid resending the menu
	if( sd.menuskill_id == skill_id ){
		return;
	}

	if (skill_id == GC_CREATENEWPOISON)
		skill_id = GC_RESEARCHNEWPOISON;

	PACKET_ZC_MAKABLEITEMLIST* p = reinterpret_cast<PACKET_ZC_MAKABLEITEMLIST*>( packet_buffer );

	p->packetType = HEADER_ZC_MAKABLEITEMLIST;
	p->packetLength = sizeof( *p );

	int count = 0;
	for( int i = 0; i < MAX_SKILL_PRODUCE_DB; i++ ){
		if( !skill_can_produce_mix( &sd, skill_produce_db[i].nameid, trigger, 1 ) ){
			continue;
		}

		if( skill_id > 0 && skill_produce_db[i].req_skill != skill_id ){
			continue;
		}

		PACKET_ZC_MAKABLEITEMLIST_sub& entry = p->items[count];

		entry.itemId = client_nameid( skill_produce_db[i].nameid );
		entry.material[0] = 0;
		entry.material[1] = 0;
		entry.material[2] = 0;

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
		count++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );

	if( count > 0 ){
		sd.menuskill_id = skill_id;
		sd.menuskill_val = trigger;
	}
}


/// Present a list of producable items.
/// 025a <packet len>.W <mk type>.W { <name id>.W }* (ZC_MAKINGITEM_LIST)
/// mk type:
///     1 = cooking
///     2 = arrow
///     3 = elemental
///     4 = GN_MIX_COOKING
///     5 = GN_MAKEBOMB
///     6 = GN_S_PHARMACY
///     7 = MT_M_MACHINE
///     8 = BO_BIONIC_PHARMACY
void clif_cooking_list( map_session_data& sd, int trigger, uint16 skill_id, int qty, int list_type ){
#if PACKETVER >= 20051010
	// Avoid resending the menu
	if( sd.menuskill_id == skill_id ){
		return;
	}

	PACKET_ZC_MAKINGITEM_LIST* p = reinterpret_cast<PACKET_ZC_MAKINGITEM_LIST*>( packet_buffer );

	p->packetType = HEADER_ZC_MAKINGITEM_LIST;
	p->packetLength = sizeof( *p );
	p->makeItem = list_type;

	int count = 0;
	for( int i = 0; i < MAX_SKILL_PRODUCE_DB; i++ ){
		if( !skill_can_produce_mix( &sd, skill_produce_db[i].nameid, trigger, qty ) ){
			continue;
		}

		PACKET_ZC_MAKINGITEM_LIST_sub& entry = p->items[count];

		entry.itemId = client_nameid( skill_produce_db[i].nameid );

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
		count++;
	}

	if( count > 0 || skill_id == AM_PHARMACY ){
		clif_send( p, p->packetLength, &sd.bl, SELF );

		sd.menuskill_id = skill_id;
		sd.menuskill_val = trigger;
		sd.menuskill_val2 = qty; // amount.
	}else{
		clif_menuskill_clear( &sd );

#if PACKETVER >= 20090922
		clif_msg_skill( &sd, skill_id, MSI_SKILL_INVENTORY_KINDCNT_OVER );
#else
		clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
	}
#endif
}


/// Notifies clients of a status change.
/// 0196 <index>.W <id>.L <state>.B (ZC_MSG_STATE_CHANGE) [used for ending status changes and starting them on non-pc units (when needed)]
/// 043f <index>.W <id>.L <state>.B <remain msec>.L { <val>.L }*3 (ZC_MSG_STATE_CHANGE2) [used exclusively for starting statuses on pcs]
/// 0983 <index>.W <id>.L <state>.B <total msec>.L <remain msec>.L { <val>.L }*3 (ZC_MSG_STATE_CHANGE3) (PACKETVER >= 20120618)
/// @param bl Sends packet to clients around this object
/// @param id ID of object that has this effect
/// @param type Status icon see enum efst_type
/// @param flag 1:Active, 0:Deactive
/// @param tick Duration in ms
/// @param val1
/// @param val2
/// @param val3
void clif_status_change_sub(struct block_list *bl, int id, int type, int flag, t_tick tick, int val1, int val2, int val3, enum send_target target_type)
{
	unsigned char buf[32];

	if (type == EFST_BLANK)  //It shows nothing on the client...
		return;

	if (type == EFST_POSTDELAY && tick == 0)
		return;

	nullpo_retv(bl);

	// Statuses with an infinite duration, but still needs a duration sent to display properly.
	if (type == EFST_LUNARSTANCE || type == EFST_UNIVERSESTANCE || type == EFST_SUNSTANCE || type == EFST_STARSTANCE)
		tick = 200;

#if PACKETVER >= 20120618
	if (flag && battle_config.display_status_timers)
		WBUFW(buf,0) = 0x983;
	else
#elif PACKETVER >= 20090121
	if (flag && battle_config.display_status_timers)
		WBUFW(buf,0) = 0x43f;
	else
#endif
		WBUFW(buf,0) = 0x196;
	WBUFW(buf,2) = type;
	WBUFL(buf,4) = id;
	WBUFB(buf,8) = flag;
#if PACKETVER >= 20120618
	if (flag && battle_config.display_status_timers) {
		if (tick <= 0)
			tick = 9999; // this is indeed what official servers do

		WBUFL(buf,9) = client_tick(tick);/* at this stage remain and total are the same value I believe */
		WBUFL(buf,13) = client_tick(tick);
		WBUFL(buf,17) = val1;
		WBUFL(buf,21) = val2;
		WBUFL(buf,25) = val3;
	}
#elif PACKETVER >= 20090121
	if (flag && battle_config.display_status_timers) {
		if (tick <= 0)
			tick = 9999; // this is indeed what official servers do

		WBUFL(buf,9) = client_tick(tick);
		WBUFL(buf,13) = val1;
		WBUFL(buf,17) = val2;
		WBUFL(buf,21) = val3;
	}
#endif
	clif_send(buf, packet_len(WBUFW(buf,0)), bl, target_type);
}

/* Sends status effect to clients around the bl
 * @param bl Object that has the effect
 * @param type Status icon see enum efst_type
 * @param flag 1:Active, 0:Deactive
 * @param tick Duration in ms
 * @param val1
 * @param val2
 * @param val3
 */
void clif_status_change(struct block_list *bl, int type, int flag, t_tick tick, int val1, int val2, int val3) {
	map_session_data *sd = nullptr;

	if (type == EFST_BLANK)  //It shows nothing on the client...
		return;

	if (type == EFST_POSTDELAY && tick == 0)
		return;

	if (type == EFST_ILLUSION && !battle_config.display_hallucination) // Disable Hallucination.
		return;

#if !( PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 )
	// Older clients display normal riding icon.
	if (type == EFST_MADOGEAR_TYPE)
		type = EFST_RIDING;
#endif

	nullpo_retv(bl);

	sd = BL_CAST(BL_PC, bl);

	// Check if current bl type is in the returned bitmask and only send status changes that actually matter to the client
	if (!(status_efst_get_bl_type(static_cast<efst_type>(type)) & bl->type))
		return;

	clif_status_change_sub(bl, bl->id, type, flag, tick, val1, val2, val3, ((sd ? (pc_isinvisible(sd) ? SELF : AREA) : AREA_WOS)));
}

/**
 * Send any active EFST to those around.
 * @param tbl: Unit to send the packet to
 * @param bl: Objects walking into view
 * @param target: Client send type
 */
void clif_efst_status_change_sub(struct block_list *tbl, struct block_list *bl, enum send_target target) {
	unsigned char i;
	struct sc_display_entry **sc_display;
	unsigned char sc_display_count;
	bool spheres_sent;

	nullpo_retv(bl);

	switch( bl->type ){
		case BL_PC: {
			map_session_data* sd = (map_session_data*)bl;

			sc_display = sd->sc_display;
			sc_display_count = sd->sc_display_count;
			spheres_sent = !sd->state.connect_new;
			}
			break;
		case BL_NPC: {
			struct npc_data* nd = (struct npc_data*)bl;

			sc_display = nd->sc_display;
			sc_display_count = nd->sc_display_count;
			spheres_sent = true;
			}
			break;
		default:
			return;
	}

	for (i = 0; i < sc_display_count; i++) {
		enum sc_type type = sc_display[i]->type;
		status_change *sc = status_get_sc(bl);
		const struct TimerData *td = (sc && sc->getSCE(type) ? get_timer(sc->getSCE(type)->timer) : nullptr);
		t_tick tick = 0;

		if (td)
			tick = DIFF_TICK(td->tick, gettick());

		// Status changes that need special handling
		switch( type ){
			case SC_SPHERE_1:
			case SC_SPHERE_2:
			case SC_SPHERE_3:
			case SC_SPHERE_4:
			case SC_SPHERE_5:
				if( spheres_sent ){
					target = AREA_WOS;
				}
				break;
			case SC_HELLS_PLANT:
				if( sc && sc->getSCE(type) ){
					tick = sc->getSCE(type)->val4;
				}
				break;
		}

#if PACKETVER > 20120418
		clif_efst_status_change(tbl, bl->id, target, status_db.getIcon(type), tick, sc_display[i]->val1, sc_display[i]->val2, sc_display[i]->val3);
#else
		clif_status_change_sub(tbl, bl->id, status_db.getIcon(type), 1, tick, sc_display[i]->val1, sc_display[i]->val2, sc_display[i]->val3, target);
#endif
	}
}

/// Notifies the client when a player enters the screen with an active EFST.
/// 08ff <id>.L <index>.W <remain msec>.L { <val>.L }*3  (ZC_EFST_SET_ENTER) (PACKETVER >= 20111108)
/// 0984 <id>.L <index>.W <total msec>.L <remain msec>.L { <val>.L }*3 (ZC_EFST_SET_ENTER2) (PACKETVER >= 20120618)
void clif_efst_status_change(struct block_list *bl, int tid, enum send_target target, int type, t_tick tick, int val1, int val2, int val3) {
#if PACKETVER >= 20111108
	unsigned char buf[32];
#if PACKETVER >= 20120618
	const int cmd = 0x984;
#elif PACKETVER >= 20111108
	const int cmd = 0x8ff;
#endif
	int offset = 0;

	if (type == EFST_BLANK)
		return;

	nullpo_retv(bl);

	if (tick <= 0)
		tick = 9999;

	WBUFW(buf,offset + 0) = cmd;
	WBUFL(buf,offset + 2) = tid;
	WBUFW(buf,offset + 6) = type;
#if PACKETVER >= 20111108
	WBUFL(buf,offset + 8) = client_tick(tick); // Set remaining status duration [exneval]
#if PACKETVER >= 20120618
	WBUFL(buf,offset + 12) = client_tick(tick);
	offset += 4;
#endif
	WBUFL(buf,offset + 12) = val1;
	WBUFL(buf,offset + 16) = val2;
	WBUFL(buf,offset + 20) = val3;
#endif
	clif_send(buf,packet_len(cmd),bl,target);
#endif
}

/// Send message (modified by [Yor]) (ZC_NOTIFY_PLAYERCHAT).
/// 008e <packet len>.W <message>.?B
void clif_displaymessage(const int fd, const char* mes)
{
	nullpo_retv(mes);

	if( session_isActive( fd ) ){
		char *message, *line;

		message = aStrdup(mes);
		line = strtok(message, "\n");

		while(line != nullptr) {
#if PACKETVER == 20141022
		/** for some reason game client crashes depending on message pattern (only for this packet) **/
		/** so we redirect to ZC_NPC_CHAT **/
		//clif_messagecolor(&sd->bl, color_table[COLOR_DEFAULT], mes, false, SELF);
			unsigned long color = (color_table[COLOR_DEFAULT] & 0x0000FF) << 16 | (color_table[COLOR_DEFAULT] & 0x00FF00) | (color_table[COLOR_DEFAULT] & 0xFF0000) >> 16; // RGB to BGR
			unsigned short len = strnlen(line, CHAT_SIZE_MAX);

			if (len > 0) { 
				WFIFOHEAD(fd, 13 + len);
				WFIFOW(fd, 0) = 0x2C1;
				WFIFOW(fd, 2) = 13 + len;
				WFIFOL(fd, 4) = 0;
				WFIFOL(fd, 8) = color;
				safestrncpy(WFIFOCP(fd, 12), line, len + 1);
				WFIFOSET(fd, WFIFOW(fd, 2));
			}
#else
			// Limit message to 255+1 characters (otherwise it causes a buffer overflow in the client)
			int16 len;

			len = static_cast<decltype(len)>( strnlen(line, CHAT_SIZE_MAX) );

			if (len > 0) { // don't send a void message (it's not displaying on the client chat). @help can send void line.
				WFIFOHEAD(fd, 5 + len);
				WFIFOW(fd,0) = 0x8e;
				WFIFOW(fd,2) = 5 + len; // 4 + len + nullptr teminate
				safestrncpy(WFIFOCP(fd,4), line, len + 1);
				WFIFOSET(fd, 5 + len);
			}
#endif
			line = strtok(nullptr, "\n");
		}
		aFree(message);
	}
}

/// Send broadcast message in yellow or blue without font formatting.
/// 009a <packet len>.W <message>.?B (ZC_BROADCAST)
void clif_broadcast(struct block_list* bl, const char* mes, size_t len, int type, enum send_target target)
{
	nullpo_retv(mes);
	if (len < 2)
		return;

	PACKET_ZC_BROADCAST* p = reinterpret_cast<PACKET_ZC_BROADCAST*>( packet_buffer );

	p->packetType = HEADER_ZC_BROADCAST;
	p->PacketLength = sizeof( *p );

	if( ( type&BC_BLUE ) != 0 ){
		// If there's "blue" at the beginning of the message, game client will display it in blue instead of yellow.
		const char* color = "blue";
		size_t length = strlen( color );

		strcpy( p->message, color );
		p->PacketLength += static_cast<decltype(p->PacketLength)>( length );

		strncpy( &p->message[length], mes, len );
		p->PacketLength += static_cast<decltype(p->PacketLength)>( len );
	}else if( ( type&BC_WOE ) != 0 ){
		// If there's "ssss", game client will recognize message as 'WoE broadcast'.
		const char* color = "ssss";
		size_t length = strlen( color );

		strcpy( p->message, color );
		p->PacketLength += static_cast<decltype(p->PacketLength)>( length );

		strncpy( &p->message[length], mes, len );
		p->PacketLength += static_cast<decltype(p->PacketLength)>( len );
	}else{
		strncpy( p->message, mes, len );
		p->PacketLength += static_cast<decltype(p->PacketLength)>( len );
	}

	clif_send( p, p->PacketLength, bl, target );
}

/*==========================================
 * Displays a message on a 'bl' to all it's nearby clients
 * 008d <PacketLength>.W <GID>.L <message>.?B (ZC_NOTIFY_CHAT)
 *------------------------------------------*/
void clif_GlobalMessage( block_list& bl, const char* message, enum send_target target ){
	nullpo_retv(message);

	int16 len = (int16)( strlen( message ) + 1 );

	if( len > CHAT_SIZE_MAX ) {
		ShowWarning("clif_GlobalMessage: Truncating too long message '%s' (len=%" PRId16 ").\n", message, len);
		len = CHAT_SIZE_MAX;
	}

	PACKET_ZC_NOTIFY_CHAT* p = reinterpret_cast<PACKET_ZC_NOTIFY_CHAT*>( packet_buffer );

	p->PacketType = HEADER_ZC_NOTIFY_CHAT;
	p->PacketLength =  sizeof( *p );
	p->GID = bl.id;
	safestrncpy( p->Message, message, len );
	p->PacketLength += static_cast<decltype(p->PacketLength)>( len );

	clif_send( p, p->PacketLength, &bl, target );
}


/// Send broadcast message with font formatting.
/// 01c3 <packet len>.W <fontColor>.L <fontType>.W <fontSize>.W <fontAlign>.W <fontY>.W <message>.?B (ZC_BROADCAST2)
void clif_broadcast2(struct block_list* bl, const char* mes, size_t len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target)
{
	nullpo_retv(mes);
	if (len < 2)
		return;

	PACKET_ZC_BROADCAST2* p = reinterpret_cast<PACKET_ZC_BROADCAST2*>( packet_buffer );

	p->packetType = HEADER_ZC_BROADCAST2;
	p->PacketLength = sizeof( *p );
	p->fontColor = fontColor;
	p->fontType = fontType;
	p->fontSize = fontSize;
	p->fontAlign = fontAlign;
	p->fontY = fontY;
	strncpy( p->message, mes, len );
	p->PacketLength += static_cast<decltype(p->PacketLength)>( len );

	clif_send( p, p->PacketLength, bl, target );
}

/*
 * Display *msg to all *users in channel
 */
void clif_channel_msg(struct Channel *channel, const char *msg, unsigned long color) {
	DBIterator *iter;
	map_session_data *user;
	unsigned short msg_len = 0, len = 0;
	unsigned char buf[CHAT_SIZE_MAX];

	if (!channel || !msg)
		return;

	msg_len = (unsigned short)(strlen(msg) + 1);

	if( msg_len > sizeof(buf)-12 ) {
		ShowWarning("clif_channel_msg: Truncating too long message '%s' (len=%u).\n", msg, msg_len);
		msg_len = sizeof(buf)-12;
	}

	WBUFW(buf,0) = 0x2C1;
	WBUFW(buf,2) = (len = msg_len + 12);
	WBUFL(buf,4) = 0;
	WBUFL(buf,8) = color;
	safestrncpy(WBUFCP(buf,12), msg, msg_len);

	iter = db_iterator(channel->users);
	for( user = (map_session_data *)dbi_first(iter); dbi_exists(iter); user = (map_session_data *)dbi_next(iter) ) {
		clif_send(buf, len, &user->bl, SELF);
	}
	dbi_destroy(iter);
}

/// Displays heal effect.
/// 013d <var id>.W <amount>.W (ZC_RECOVERY)
/// 0a27 <var id>.W <amount>.L (ZC_RECOVERY2)
/// var id:
///     5 = HP (SP_HP)
///     7 = SP (SP_SP)
///     ? = ignored
void clif_heal( map_session_data& sd, int32 type, uint32 val ) {
	PACKET_ZC_RECOVERY packet{};

	packet.packetType = HEADER_ZC_RECOVERY;
	packet.type = static_cast<decltype(packet.type)>(type);
	packet.amount = std::min( static_cast<decltype(packet.amount)>( val ), std::numeric_limits<decltype(packet.amount)>::max() );

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Displays resurrection effect.
/// 0148 <id>.L <type>.W (ZC_RESURRECTION)
/// type:
///     ignored
void clif_resurrection( block_list& bl ){
	PACKET_ZC_RESURRECTION packet{};

	packet.packetType = HEADER_ZC_RESURRECTION;
	packet.gid = bl.id;
	packet.type = 0;

	clif_send( &packet, sizeof( packet ), &bl, AREA );

	if (disguised(&bl)) {
		clif_spawn(&bl);
	}
}


/// Sets the map property
/// 0199 <type>.W (ZC_NOTIFY_MAPPROPERTY)
/// 099b <type>.W <flags>.L (ZC_MAPPROPERTY_R2)
void clif_map_property(struct block_list *bl, enum map_property property, enum send_target t)
{
#if PACKETVER >= 20121010
	short cmd = 0x99b;
	unsigned char buf[8];
#else
	short cmd = 0x199;
	unsigned char buf[4];
#endif
	
	WBUFW(buf,0)=cmd;
	WBUFW(buf,2)=property;

#if PACKETVER >= 20121010
	struct map_data *mapdata = map_getmapdata(bl->m);
	map_session_data *sd = BL_CAST(BL_PC, bl);

	WBUFL(buf,4) = ((mapdata->getMapFlag(MF_PVP) || (sd && sd->duel_group > 0))<<0)| // PARTY - Show attack cursor on non-party members (PvP)
		((mapdata->getMapFlag(MF_BATTLEGROUND) || mapdata_flag_gvg2(mapdata))<<1)|// GUILD - Show attack cursor on non-guild members (GvG)
		((mapdata->getMapFlag(MF_BATTLEGROUND) || mapdata_flag_gvg2(mapdata))<<2)|// SIEGE - Show emblem over characters heads when in GvG (WoE castle)
		((mapdata->getMapFlag(MF_FORCEMINEFFECT) || mapdata_flag_gvg2(mapdata))<<3)| // USE_SIMPLE_EFFECT - Forces simpler skill effects, like /mineffect command
		((mapdata->getMapFlag(MF_NOLOCKON) || mapdata_flag_vs(mapdata) || (sd && sd->duel_group > 0))<<4)| // DISABLE_LOCKON - Only allow attacks on other players with shift key or /ns active
		((mapdata->getMapFlag(MF_PVP))<<5)| // COUNT_PK - Show the PvP counter
		((mapdata->getMapFlag(MF_PARTYLOCK))<<6)| // NO_PARTY_FORMATION - Prevents party creation/modification (Might be used for instance dungeons)
		((mapdata->getMapFlag(MF_BATTLEGROUND))<<7)| // BATTLEFIELD - Unknown (Does something for battlegrounds areas)
		((mapdata->getMapFlag(MF_NOCOSTUME))<<8)| // DISABLE_COSTUMEITEM - Disable costume sprites
		((!mapdata->getMapFlag(MF_NOUSECART))<<9)| // USECART - Allow opening cart inventory (Well force it to always allow it)
		((!mapdata->getMapFlag(MF_NOSUNMOONSTARMIRACLE))<<10); // SUNMOONSTAR_MIRACLE - Allows Star Gladiator's Miracle to activate
		//(1<<11); // Unused bits. 1 - 10 is 0x1 length and 11 is 0x15 length. May be used for future settings.
#endif
	
	clif_send(buf,packet_len(cmd),bl,t);
}

/// Set the map type.
/// 01d6 <type>.W (ZC_NOTIFY_MAPPROPERTY2)
void clif_map_type( map_session_data& sd, e_map_type type ){
	PACKET_ZC_NOTIFY_MAPPROPERTY2 packet{};

	packet.packetType = HEADER_ZC_NOTIFY_MAPPROPERTY2;
	packet.type = static_cast<decltype(packet.type)>(type);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Updates PvP ranking (ZC_NOTIFY_RANKING).
/// 019a <id>.L <ranking>.L <total>.L
void clif_pvpset(map_session_data *sd,int pvprank,int pvpnum,int type)
{
	if(type == 2) {
		int fd = sd->fd;
		WFIFOHEAD(fd,packet_len(0x19a));
		WFIFOW(fd,0) = 0x19a;
		WFIFOL(fd,2) = sd->bl.id;
		WFIFOL(fd,6) = pvprank;
		WFIFOL(fd,10) = pvpnum;
		WFIFOSET(fd,packet_len(0x19a));
	} else {
		unsigned char buf[32];
		WBUFW(buf,0) = 0x19a;
		WBUFL(buf,2) = sd->bl.id;
		if(sd->sc.option&(OPTION_HIDE|OPTION_CLOAK))
			WBUFL(buf,6) = UINT32_MAX; //On client displays as --
		else
			WBUFL(buf,6) = pvprank;
		WBUFL(buf,10) = pvpnum;
		if(pc_isinvisible(sd) || sd->disguise) //Causes crashes when a 'mob' with pvp info dies.
			clif_send(buf,packet_len(0x19a),&sd->bl,SELF);
		else if(!type)
			clif_send(buf,packet_len(0x19a),&sd->bl,AREA);
		else
			clif_send(buf,packet_len(0x19a),&sd->bl,ALL_SAMEMAP);
	}
}


/*==========================================
 *
 *------------------------------------------*/
void clif_map_property_mapall(int map_idx, enum map_property property)
{
	struct block_list bl;

	bl.id = 0;
	bl.type = BL_NUL;
	bl.m = map_idx;
	
	clif_map_property( &bl, property, ALL_SAMEMAP );
}

/// Notifies the client about the result of a refine attempt.
/// 0188 <result>.W <index>.W <refine>.W (ZC_ACK_ITEMREFINING)
/// result:
///     0 = success
///     1 = failure
///     2 = downgrade
///     3 = failure (?)
void clif_refine( map_session_data& sd, uint16 index, e_ack_itemrefining result ){
	PACKET_ZC_ACK_ITEMREFINING packet{};

	packet.packetType = HEADER_ZC_ACK_ITEMREFINING;
	packet.result = static_cast<decltype(packet.result)>(result);
	packet.index = client_index( index );
	packet.value = static_cast<decltype(packet.value)>( sd.inventory.u.items_inventory[index].refine );

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Notifies the client about the result of a weapon refine attempt (ZC_ACK_WEAPONREFINE).
/// 0223 <result>.L <nameid>.W
/// result:
///     0 = "weapon upgraded: %s" MsgStringTable[911] in rgb(0,255,255)
///     1 = "weapon upgraded: %s" MsgStringTable[912] in rgb(0,205,205)
///     2 = "cannot upgrade %s until you level up the upgrade weapon skill" MsgStringTable[913] in rgb(255,200,200)
///     3 = "you lack the item %s to upgrade the weapon" MsgStringTable[914] in rgb(255,200,200)
void clif_upgrademessage( map_session_data* sd, int result, t_itemid item_id ){
	PACKET_ZC_ACK_WEAPONREFINE p = {};

	p.packetType = HEADER_ZC_ACK_WEAPONREFINE;
	p.result = result;
	p.itemId = client_nameid( item_id );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Whisper is transmitted to the destination player (ZC_WHISPER).
/// 0097 <packet len>.W <nick>.24B <message>.?B
/// 0097 <packet len>.W <nick>.24B <isAdmin>.L <message>.?B (PACKETVER >= 20091104)
void clif_wis_message(map_session_data* sd, const char* nick, const char* mes, size_t mes_len, int gmlvl){
	PACKET_ZC_WHISPER* p = reinterpret_cast<PACKET_ZC_WHISPER*>( packet_buffer );

	p->PacketType = HEADER_ZC_WHISPER;
	p->PacketLength = sizeof( *p );
	safestrncpy( p->sender, nick, NAME_LENGTH );
	safestrncpy( p->message, mes, mes_len );
	p->PacketLength += static_cast<decltype( p->PacketLength )>( mes_len );

#if PACKETVER >= 20091104
	map_session_data* ssd = map_nick2sd(nick, false);

	// If it is not a message from the server or a player from another map-server
	if( ssd != nullptr ){
		gmlvl = pc_get_group_level(ssd);
	}

	p->isAdmin = (gmlvl == 99) ? 1 : 0;

#if PACKETVER_MAIN_NUM >= 20131204 || PACKETVER_RE_NUM >= 20131120 || defined(PACKETVER_ZERO)
	if( ssd != nullptr ){
		p->senderGID = ssd->bl.id;
	}else{
		p->senderGID = 0;
	}
#endif
#endif

	clif_send( p, p->PacketLength, &sd->bl, SELF );
}


/// Inform the player about the result of his whisper action 
/// 0098 <result>.B (ZC_ACK_WHISPER).
/// 09df <result>.B <CID>.L (ZC_ACK_WHISPER02).
/// result:
///     0 = success to send whisper
///     1 = target character is not loged in
///     2 = ignored by target
///     3 = everyone ignored by target
void clif_wis_end( map_session_data& sd, e_ack_whisper result ){
	PACKET_ZC_ACK_WHISPER packet{};

	packet.packetType = HEADER_ZC_ACK_WHISPER;
	packet.result = static_cast<decltype(packet.result)>(result);
#if PACKETVER >= 20131223
	packet.CID = sd.status.char_id;
#endif

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Returns character name requested by char_id (ZC_ACK_REQNAME_BYGID).
/// 0194 <char id>.L <name>.24B
/// 0af7 <flag>.W <char id>.L <name>.24B
void clif_solved_charname(int fd, int charid, const char* name)
{
#if PACKETVER >= 20180221
	WFIFOHEAD(fd,packet_len(0xaf7));
	WFIFOW(fd,0) = 0xaf7;
	WFIFOW(fd,2) = name[0] ? 3 : 2;
	WFIFOL(fd,4) = charid;
	safestrncpy(WFIFOCP(fd, 8), name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x0af7));
#else
	WFIFOHEAD(fd,packet_len(0x194));
	WFIFOW(fd,0)=0x194;
	WFIFOL(fd,2)=charid;
	safestrncpy(WFIFOCP(fd,6), name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x194));
#endif
}


/// Presents a list of items that can be carded/composed (ZC_ITEMCOMPOSITION_LIST).
/// 017b <packet len>.W { <name id>.W }*
void clif_use_card(map_session_data *sd,int idx)
{
	int i,c,ep;
	int fd=sd->fd;

	nullpo_retv(sd);
	if (idx < 0 || idx >= MAX_INVENTORY) //Crash-fix from bad packets.
		return;

	if (!sd->inventory_data[idx] || sd->inventory_data[idx]->type != IT_CARD)
		return; //Avoid parsing invalid item indexes (no card/no item)

	ep=sd->inventory_data[idx]->equip;
	WFIFOHEAD(fd,MAX_INVENTORY * 2 + 4);
	WFIFOW(fd,0)=0x17b;

	for(i=c=0;i<MAX_INVENTORY;i++){
		int j;

		if(sd->inventory_data[i] == nullptr)
			continue;
		if(sd->inventory_data[i]->type!=IT_WEAPON && sd->inventory_data[i]->type!=IT_ARMOR)
			continue;
		if(itemdb_isspecial(sd->inventory.u.items_inventory[i].card[0])) //Can't slot it
			continue;

		if(sd->inventory.u.items_inventory[i].identify==0 )	//Not identified
			continue;

		if((sd->inventory_data[i]->equip&ep)==0)	//Not equippable on this part.
			continue;

		if(sd->inventory_data[i]->type==IT_WEAPON && ep==EQP_SHIELD) //Shield card won't go on left weapon.
			continue;

		if(sd->inventory_data[i]->type == IT_ARMOR && (ep & EQP_ACC) && ((ep & EQP_ACC) != EQP_ACC) && ((sd->inventory_data[i]->equip & EQP_ACC) != (ep & EQP_ACC)) ) // specific accessory-card can only be inserted to specific accessory.
			continue;

		ARR_FIND( 0, sd->inventory_data[i]->slots, j, sd->inventory.u.items_inventory[i].card[j] == 0 );
		if( j == sd->inventory_data[i]->slots )	// No room
			continue;

		if( sd->inventory.u.items_inventory[i].equip > 0 )	// Do not check items that are already equipped
			continue;

		WFIFOW(fd,4+c*2)=i+2;
		c++;
	}

	if( !c ) return;	// no item is available for card insertion

	WFIFOW(fd,2)=4+c*2;
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Notifies the client about the result of item carding/composition.
/// 017d <equip index>.W <card index>.W <result>.B (ZC_ACK_ITEMCOMPOSITION)
/// result:
///     0 = success
///     1 = failure
void clif_insert_card( map_session_data& sd, int32 idx_equip, int32 idx_card, bool failure ){
	PACKET_ZC_ACK_ITEMCOMPOSITION packet{};

	packet.packetType = HEADER_ZC_ACK_ITEMCOMPOSITION;
	packet.equipIndex = client_index( idx_equip );
	packet.cardIndex = client_index( idx_card );
	packet.result = failure;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Presents a list of items that can be identified (ZC_ITEMIDENTIFY_LIST).
/// 0177 <packet len>.W { <index>.W }*
void clif_item_identify_list(map_session_data *sd)
{
	int i,c;
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;

	WFIFOHEAD(fd,MAX_INVENTORY * 2 + 4);
	WFIFOW(fd,0)=0x177;
	for(i=c=0;i<MAX_INVENTORY;i++){
		if(sd->inventory.u.items_inventory[i].nameid > 0 && !sd->inventory.u.items_inventory[i].identify){
			WFIFOW(fd,c*2+4)=client_index(i);
			c++;
		}
	}
	if(c > 0) {
		WFIFOW(fd,2)=c*2+4;
		WFIFOSET(fd,WFIFOW(fd,2));
		sd->menuskill_id = MC_IDENTIFY;
		sd->menuskill_val = c;
		sd->state.workinprogress = WIP_DISABLE_ALL;
	}
}


/// Notifies the client about the result of a item identify request.
/// 0179 <index>.W <result>.B (ZC_ACK_ITEMIDENTIFY)
/// result:
///     0 = success
///     1 = failure
void clif_item_identified( map_session_data& sd, int32 idx, bool failure ){
	PACKET_ZC_ACK_ITEMIDENTIFY packet{};

	packet.packetType = HEADER_ZC_ACK_ITEMIDENTIFY;
	packet.index = client_index( idx );
	packet.result = failure;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Presents a list of items that can be repaired.
/// 01fc <packet len>.W { <index>.W <name id>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }* (ZC_REPAIRITEMLIST)
void clif_item_repair_list( map_session_data& sd, map_session_data& dstsd, uint16 lv ){
	PACKET_ZC_REPAIRITEMLIST* p = reinterpret_cast<PACKET_ZC_REPAIRITEMLIST*>( packet_buffer );

	p->packetType = HEADER_ZC_REPAIRITEMLIST;
	p->packetLength = sizeof( *p );

	size_t c = 0;

	for( size_t i = 0; i < MAX_INVENTORY; i++ ){
		if( dstsd.inventory.u.items_inventory[i].nameid > 0 && dstsd.inventory.u.items_inventory[i].attribute != 0 && !itemdb_ishatched_egg( &dstsd.inventory.u.items_inventory[i] ) ){ // && skill_can_repair(sd,nameid)){
			p->items[c].index = static_cast<decltype( p->items[0].index )>( i );
			p->items[c].itemId = client_nameid( dstsd.inventory.u.items_inventory[i].nameid );
			p->items[c].refine = dstsd.inventory.u.items_inventory[i].refine;
			clif_addcards( &p->items[c].slot, &dstsd.inventory.u.items_inventory[i] );

			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
			c++;
		}
	}

	if( c > 0 ){
		clif_send( p, p->packetLength, &sd.bl, SELF );

		sd.menuskill_id = BS_REPAIRWEAPON;
		sd.menuskill_val = dstsd.bl.id;
		sd.menuskill_val2 = lv;
	}else{
		clif_skill_fail( sd, sd.ud.skill_id );
	}
}


/// Notifies the client about the result of a item repair request.
/// 01fe <index>.W <result>.B (ZC_ACK_ITEMREPAIR)
/// index:
///     ignored (inventory index)
/// result:
///     0 = Item repair success.
///     1 = Item repair failure.
void clif_item_repaireffect( map_session_data& sd, int32 idx, bool failure ){
	PACKET_ZC_ACK_ITEMREPAIR packet{};

	packet.packetType = HEADER_ZC_ACK_ITEMREPAIR;
	packet.index = client_index( idx );
	packet.result = failure;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Displays a message, that an equipment got damaged.
/// 02bb <equip location>.W <account id>.L (ZC_EQUIPITEM_DAMAGED)
void clif_item_damaged( map_session_data& sd, uint16 position ){
	PACKET_ZC_EQUIPITEM_DAMAGED packet{};

	packet.packetType = HEADER_ZC_EQUIPITEM_DAMAGED;
	packet.equipLocation = position;
	packet.GID = sd.bl.id;

	// TODO: the packet seems to be sent to other people as well, probably party and/or guild.
	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Presents a list of weapon items that can be refined [Taken from jAthena] (ZC_NOTIFY_WEAPONITEMLIST).
/// 0221 <packet len>.W { <index>.W <name id>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }*
void clif_item_refine_list( map_session_data& sd ){
	PACKET_ZC_NOTIFY_WEAPONITEMLIST* p = reinterpret_cast<PACKET_ZC_NOTIFY_WEAPONITEMLIST*>( packet_buffer );

	p->packetType = HEADER_ZC_NOTIFY_WEAPONITEMLIST;
	p->packetLength = sizeof( *p );

	uint16 skill_lv = pc_checkskill( &sd, WS_WEAPONREFINE );

	int refine_item[MAX_WEAPON_LEVEL];

	refine_item[0] = pc_search_inventory( &sd, ITEMID_PHRACON );
	refine_item[1] = pc_search_inventory( &sd, ITEMID_EMVERETARCON );
	refine_item[2] = refine_item[3] = pc_search_inventory( &sd, ITEMID_ORIDECON );
#ifdef RENEWAL
	refine_item[4] = -1;
#endif

	int count = 0;
	for( int i = 0; i < MAX_INVENTORY; i++ ){
		if( sd.inventory.u.items_inventory[i].nameid > 0 && sd.inventory.u.items_inventory[i].refine < skill_lv &&
			sd.inventory_data[i] != nullptr && sd.inventory_data[i]->type == IT_WEAPON &&
			sd.inventory.u.items_inventory[i].identify && sd.inventory_data[i]->weapon_level >= 1 &&
			refine_item[sd.inventory_data[i]->weapon_level - 1] != -1 && !( sd.inventory.u.items_inventory[i].equip & EQP_ARMS ) ){

			p->items[count].index = client_index( i );
			p->items[count].itemId = client_nameid( sd.inventory.u.items_inventory[i].nameid );
			p->items[count].refine = sd.inventory.u.items_inventory[i].refine;
			clif_addcards( &p->items[count].slot, &sd.inventory.u.items_inventory[i] );

			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
			count++;
		}
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );

	if( count > 0 ){
		sd.menuskill_id = WS_WEAPONREFINE;
		sd.menuskill_val = skill_lv;
	}
}


/// Notification of an auto-casted skill (ZC_AUTORUN_SKILL).
/// 0147 <skill id>.W <type>.L <level>.W <sp cost>.W <atk range>.W <skill name>.24B <upgradable>.B
void clif_item_skill( map_session_data *sd, uint16 skill_id, uint16 skill_lv ){
	nullpo_retv( sd );

	PACKET_ZC_AUTORUN_SKILL p = {};

	p.packetType = HEADER_ZC_AUTORUN_SKILL;
	p.skill_id = skill_id;
	p.skill_type = skill_get_inf( skill_id );
	p.skill_lv = skill_lv;
	p.skill_sp = skill_get_sp( skill_id, skill_lv );
	p.skill_range = skill_get_range2( &sd->bl, skill_id, skill_lv, false );
	safestrncpy( p.skill_name, skill_get_name( skill_id ), NAME_LENGTH );
	p.up_flag = false;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Adds an item to character's cart.
/// 0124 <index>.W <amount>.L <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_CART)
/// 01c5 <index>.W <amount>.L <name id>.W <type>.B <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W (ZC_ADD_ITEM_TO_CART2)
/// 0a0b <index>.W <amount>.L <name id>.W <type>.B <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W { <option id>.W <option value>.W <option param>.B }*5 (ZC_ADD_ITEM_TO_CART3)
void clif_cart_additem( map_session_data *sd, int n, int amount ){
	nullpo_retv( sd );

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	if( n < 0 || n >= MAX_CART || sd->cart.u.items_cart[n].nameid == 0 ){
		return;
	}

	PACKET_ZC_ADD_ITEM_TO_CART p = {};

	p.packetType = HEADER_ZC_ADD_ITEM_TO_CART;
	p.index = client_index( n );
	p.amount = amount;
	p.itemId = client_nameid( sd->cart.u.items_cart[n].nameid );
#if PACKETVER >= 5
	p.itemType = itemdb_type( sd->cart.u.items_cart[n].nameid );
#endif
	p.identified = sd->cart.u.items_cart[n].identify;
	p.damaged  = sd->cart.u.items_cart[n].attribute;
	p.refine = sd->cart.u.items_cart[n].refine;
	clif_addcards( &p.slot, &sd->cart.u.items_cart[n] );
#if PACKETVER >= 20150226
	clif_add_random_options( p.option_data, sd->cart.u.items_cart[n] );
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	p.grade = sd->cart.u.items_cart[n].enchantgrade;
#endif
#endif

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}

/// Acknowledge an item have been added to cart
/// 012c <result>.B (ZC_ACK_ADDITEM_TO_CART)
/// result:
/// 0 = ADDITEM_TO_CART_FAIL_WEIGHT
/// 1 = ADDITEM_TO_CART_FAIL_COUNT
void clif_cart_additem_ack( map_session_data& sd, e_ack_additem_to_cart flag ){
	PACKET_ZC_ACK_ADDITEM_TO_CART packet{};

	packet.packetType = HEADER_ZC_ACK_ADDITEM_TO_CART;
	packet.result = static_cast<decltype(packet.result)>(flag);
	
	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

// 09B7 <unknow data> (ZC_ACK_OPEN_BANKING)
void clif_bank_open( map_session_data& sd ){
#if PACKETVER >= 20130717
	PACKET_ZC_ACK_OPEN_BANKING p = {};

	p.packetType = HEADER_ZC_ACK_OPEN_BANKING;
	p.unknown = 0;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/*
 * Request to Open the banking system
 * 09B6 <aid>L (CZ_REQ_OPEN_BANKING)
 */
void clif_parse_BankOpen(int fd, map_session_data* sd) {
#if PACKETVER >= 20130717
	const PACKET_CZ_REQ_OPEN_BANKING* p = reinterpret_cast<PACKET_CZ_REQ_OPEN_BANKING*>( RFIFOP( fd, 0 ) );

	//TODO check if preventing trade or stuff like that
	//also mark something in case char ain't available for saving, should we check now ?
	nullpo_retv(sd);
	if( !battle_config.feature_banking ) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1496),false,SELF); //Banking is disabled
		return;
	}
	if( map_getmapflag( sd->bl.m, MF_NOBANK ) ){
		clif_displaymessage( sd->fd, msg_txt( sd, 831 ) ); // You cannot use the Bank on this map.
		return;
	}
	else {
		if(sd->status.account_id == p->AID){
			sd->state.banking = 1;
			//request save ?
			//chrif_bankdata_request(sd->status.account_id, sd->status.char_id);
			//on succes open bank ?
			clif_bank_open( *sd );
		}
	}
#endif
}

// 09B9 <unknow data> (ZC_ACK_CLOSE_BANKING)
void clif_bank_close( map_session_data& sd ){
#if PACKETVER >= 20130717
	PACKET_ZC_ACK_CLOSE_BANKING p = {};

	p.packetType = HEADER_ZC_ACK_CLOSE_BANKING;
	p.unknown = 0;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/*
 * Request to close the banking system
 * 09B8 <aid>L (CZ_REQ_CLOSE_BANKING)
 */
void clif_parse_BankClose(int fd, map_session_data* sd) {
#if PACKETVER >= 20130717
	const PACKET_CZ_REQ_CLOSE_BANKING* p = reinterpret_cast<PACKET_CZ_REQ_CLOSE_BANKING*>( RFIFOP( fd, 0 ) );

	nullpo_retv(sd);
	if( !battle_config.feature_banking ) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1496),false,SELF); //Banking is disabled
		//still allow to go trough to not stuck player if we have disable it while they was in
	}
	if(sd->status.account_id == p->AID){
		sd->state.banking = 0;
		clif_bank_close( *sd );
	}
#endif
}

/*
 * Display how much we got in bank (I suppose)
 * 09A6 <Bank_Vault>Q <Reason>W (ZC_BANKING_CHECK)
 */
void clif_Bank_Check( map_session_data& sd ){
#if PACKETVER >= 20130717
	PACKET_ZC_BANKING_CHECK p = {};

	p.packetType = HEADER_ZC_BANKING_CHECK;
	p.money = sd.bank_vault;
	p.reason = 0;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/*
 * Requesting the data in bank
 * 09AB <aid>L (CZ_REQ_BANKING_CHECK)
 */
void clif_parse_BankCheck(int fd, map_session_data* sd) {
#if PACKETVER >= 20130717
	const PACKET_CZ_REQ_BANKING_CHECK* p = reinterpret_cast<PACKET_CZ_REQ_BANKING_CHECK*>( RFIFOP( fd, 0 ) );

	nullpo_retv(sd);

	if( !battle_config.feature_banking ) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1496),false,SELF); //Banking is disabled
		return;
	}
	if( map_getmapflag( sd->bl.m, MF_NOBANK ) ){
		clif_displaymessage( sd->fd, msg_txt( sd, 831 ) ); // You cannot use the Bank on this map.
		return;
	}
	else {
		if(sd->status.account_id == p->AID) //since we have it let check it for extra security
			clif_Bank_Check( *sd );
	}
#endif
}

/*
 * Acknowledge of deposit some money in bank
  09A8 <Reason>W <Money>Q <balance>L (ZC_ACK_BANKING_DEPOSIT)
 */
void clif_bank_deposit( map_session_data& sd, enum e_BANKING_DEPOSIT_ACK reason ){
#if PACKETVER >= 20130717
	PACKET_ZC_ACK_BANKING_DEPOSIT p = {};

	p.packetType = HEADER_ZC_ACK_BANKING_DEPOSIT;
	p.money = sd.bank_vault;
	p.zeny = sd.status.zeny;
	p.reason = reason;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/*
 * Request saving some money in bank
 * @author : original [Yommy/Hercules]
 * 09A7 <AID>L <Money>L (CZ_REQ_BANKING_DEPOSIT)
 */
void clif_parse_BankDeposit(int fd, map_session_data* sd) {
#if PACKETVER >= 20130717
	const PACKET_CZ_REQ_BANKING_DEPOSIT* p = reinterpret_cast<PACKET_CZ_REQ_BANKING_DEPOSIT*>( RFIFOP( fd, 0 ) );

	nullpo_retv(sd);
	if( !battle_config.feature_banking ) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1496),false,SELF); //Banking is disabled
		return;
	}
	if( map_getmapflag( sd->bl.m, MF_NOBANK ) ){
		clif_displaymessage( sd->fd, msg_txt( sd, 831 ) ); // You cannot use the Bank on this map.
		return;
	}
	else {
		if(sd->status.account_id == p->AID){
			enum e_BANKING_DEPOSIT_ACK reason = pc_bank_deposit(sd,max(0,p->zeny));
			clif_bank_deposit( *sd, reason );
		}
	}
#endif
}

/*
 * Acknowledge of withdrawing some money from bank
 * 09AA <Reason>W <Money>Q <balance>L (ZC_ACK_BANKING_WITHDRAW)
 */
void clif_bank_withdraw( map_session_data& sd, enum e_BANKING_WITHDRAW_ACK reason ){
#if PACKETVER >= 20130717
	PACKET_ZC_ACK_BANKING_WITHDRAW p = {};

	p.packetType = HEADER_ZC_ACK_BANKING_WITHDRAW;
	p.reason = reason;
	p.money = sd.bank_vault;
	p.zeny = sd.status.zeny;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/*
 * Request Withdrawing some money from bank
 * 09A9 <AID>L <Money>L (CZ_REQ_BANKING_WITHDRAW)
 */
void clif_parse_BankWithdraw(int fd, map_session_data* sd) {
#if PACKETVER >= 20130717
	const PACKET_CZ_REQ_BANKING_WITHDRAW* p = reinterpret_cast<PACKET_CZ_REQ_BANKING_WITHDRAW*>( RFIFOP( fd, 0 ) );

	nullpo_retv(sd);
	if( !battle_config.feature_banking ) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1496),false,SELF); //Banking is disabled
		return;
	}
	if( map_getmapflag( sd->bl.m, MF_NOBANK ) ){
		clif_displaymessage( sd->fd, msg_txt( sd, 831 ) ); // You cannot use the Bank on this map.
		return;
	}
	else {
		if(sd->status.account_id == p->AID){
			enum e_BANKING_WITHDRAW_ACK reason = pc_bank_withdraw(sd,max(0,p->zeny));
			clif_bank_withdraw( *sd, reason );
		}
	}
#endif
}

/// Deletes an item from character's cart.
/// 0125 <index>.W <amount>.L (ZC_DELETE_ITEM_FROM_CART)
void clif_cart_delitem( map_session_data& sd, int32 index, int32 amount ){
	PACKET_ZC_DELETE_ITEM_FROM_CART packet{};

	packet.packetType = HEADER_ZC_DELETE_ITEM_FROM_CART;
	packet.index = client_index( index );
	packet.amount = amount;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Opens the shop creation menu.
/// 012d <num>.W (ZC_OPENSTORE)
/// num:
///     number of allowed item slots
void clif_openvendingreq( map_session_data& sd, uint16 num ){
	PACKET_ZC_OPENSTORE packet{};

	packet.packetType = HEADER_ZC_OPENSTORE;
	packet.num = num;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Displays a vending board to target/area (ZC_STORE_ENTRY).
/// 0131 <owner id>.L <message>.80B
void clif_showvendingboard( map_session_data& sd, enum send_target target, struct block_list* tbl ){
	if( tbl == nullptr ){
		tbl = &sd.bl;
		target = AREA_WOS;
	}

	PACKET_ZC_STORE_ENTRY p = {};

	p.packetType = HEADER_ZC_STORE_ENTRY;
	p.makerAID = sd.status.account_id;
	safestrncpy( p.storeName, sd.message, sizeof( p.storeName ) );

	clif_send( &p, sizeof( p ), tbl, target );
}


/// Removes a vending board from screen (ZC_DISAPPEAR_ENTRY).
/// 0132 <owner id>.L
void clif_closevendingboard(struct block_list* bl, int fd)
{
	unsigned char buf[16];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x132;
	WBUFL(buf,2) = bl->id;
	if( session_isActive(fd) ) {
		WFIFOHEAD(fd,packet_len(0x132));
		memcpy(WFIFOP(fd,0),buf,packet_len(0x132));
		WFIFOSET(fd,packet_len(0x132));
	} else {
		clif_send(buf,packet_len(0x132),bl,AREA_WOS);
	}
}


/// Sends a list of items in a shop.
/// R 0133 <packet len>.W <owner id>.L { <price>.L <amount>.W <index>.W <type>.B <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }* (ZC_PC_PURCHASE_ITEMLIST_FROMMC)
/// R 0800 <packet len>.W <owner id>.L <unique id>.L { <price>.L <amount>.W <index>.W <type>.B <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }* (ZC_PC_PURCHASE_ITEMLIST_FROMMC2)
void clif_vendinglist( map_session_data& sd, map_session_data& vsd ){
	PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC* p = reinterpret_cast<PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC*>( packet_buffer );

	p->packetType = HEADER_ZC_PC_PURCHASE_ITEMLIST_FROMMC;
	p->packetLength = sizeof( *p );
	p->AID = vsd.status.account_id;
#if PACKETVER >= 20100105
	p->venderId = vsd.vender_id;
#endif

	for( int i = 0, count = 0; i < vsd.vend_num; i++ ){
		PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub& entry = p->items[count];
		int index = vsd.vending[i].index;
		struct item_data* data = itemdb_search( vsd.cart.u.items_cart[index].nameid );

		if( data == nullptr ){
			continue;
		}

		entry.price = vsd.vending[i].value;
		entry.amount = vsd.vending[i].amount;
		entry.index = client_index( index );
		entry.itemType = itemtype( vsd.cart.u.items_cart[index].nameid );
		entry.itemId = client_nameid( vsd.cart.u.items_cart[index].nameid );
		entry.identified = vsd.cart.u.items_cart[index].identify;
		entry.damaged = vsd.cart.u.items_cart[index].attribute;
		entry.refine = vsd.cart.u.items_cart[index].refine;
		clif_addcards( &entry.slot, &vsd.cart.u.items_cart[index] );
#if PACKETVER >= 20150226
		clif_add_random_options( entry.option_data, vsd.cart.u.items_cart[index] );
#if PACKETVER >= 20160921
		entry.location = pc_equippoint_sub( &sd, data );
		entry.viewSprite = data->look;
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
		entry.grade = vsd.cart.u.items_cart[index].enchantgrade;
#endif
#endif
#endif

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
		count++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
}


/// Shop purchase failure.
/// 0135 <index>.W <amount>.W <result>.B (ZC_PC_PURCHASE_RESULT_FROMMC)
/// result:
///     0 = success
///     1 = not enough zeny
///     2 = overweight
///     4 = out of stock
///     5 = "cannot use an npc shop while in a trade"
///     6 = Because the store information was incorrect the item was not purchased.
///     7 = No sales information.
void clif_buyvending( map_session_data& sd, uint16 index, uint16 amount, e_pc_purchase_result_frommc result ){
	PACKET_ZC_PC_PURCHASE_RESULT_FROMMC packet{};

	packet.packetType = HEADER_ZC_PC_PURCHASE_RESULT_FROMMC;
	packet.index = client_index( index );
	packet.amount = amount;
	packet.result = static_cast<decltype(packet.result)>(result);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Show's vending player its list of items for sale.
/// 0a28 <result>.B (ZC_ACK_OPENSTORE2)
void clif_openvending_ack( map_session_data& sd, e_ack_openstore2 result ){
#if PACKETVER >= 20141022
	PACKET_ZC_ACK_OPENSTORE2 packet{};

	packet.packetType = HEADER_ZC_ACK_OPENSTORE2;
	packet.result = static_cast<decltype(packet.result)>(result);

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
#endif
}

/// Shop creation success.
/// 0136 <packet len>.W <owner id>.L { <price>.L <index>.W <amount>.W <type>.B <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }* (ZC_PC_PURCHASE_MYITEMLIST)
void clif_openvending( map_session_data& sd ){
	PACKET_ZC_PC_PURCHASE_MYITEMLIST* p = reinterpret_cast<PACKET_ZC_PC_PURCHASE_MYITEMLIST*>( packet_buffer );

	p->packetType = HEADER_ZC_PC_PURCHASE_MYITEMLIST;
	p->packetLength = sizeof( *p );
	p->AID = sd.bl.id;

	for( int i = 0; i < sd.vend_num; i++ ) {
		PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub& entry = p->items[i];
		int16 index = sd.vending[i].index;

		entry.price = sd.vending[i].value;
		entry.index = client_index( index );
		entry.amount = sd.vending[i].amount;
		entry.itemType = itemtype( sd.cart.u.items_cart[index].nameid );
		entry.itemId = client_nameid( sd.cart.u.items_cart[index].nameid );
		entry.identified = sd.cart.u.items_cart[index].identify;
		entry.damaged = sd.cart.u.items_cart[index].attribute;
		entry.refine = sd.cart.u.items_cart[index].refine;
		clif_addcards( &entry.slot, &sd.cart.u.items_cart[index] );
#if PACKETVER >= 20150226
		clif_add_random_options( entry.option_data, sd.cart.u.items_cart[index] );
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
		entry.grade = sd.cart.u.items_cart[index].enchantgrade;
#endif
#endif

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );

	clif_openvending_ack( sd, OPENSTORE2_SUCCESS );
}


/// Inform merchant that someone has bought an item.
/// 0137 <index>.W <amount>.W (ZC_DELETEITEM_FROM_MCSTORE)
/// 09e5 <index>.W <amount>.W <GID>.L <Date>.L <zeny>.L (ZC_DELETEITEM_FROM_MCSTORE2)
void clif_vendingreport( map_session_data& sd, uint16 index, uint16 amount, uint32 char_id, int32 zeny ){
	PACKET_ZC_DELETEITEM_FROM_MCSTORE packet{};

	packet.packetType = HEADER_ZC_DELETEITEM_FROM_MCSTORE;
	packet.index = client_index( index );
	packet.amount = amount;

// TODO : not sure for client date [Napster]
#if PACKETVER >= 20141016
	packet.buyerCID = char_id;
	packet.date = client_tick( time(nullptr) );
	packet.zeny = zeny;
#endif

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Result of organizing a party (ZC_ACK_MAKE_GROUP).
/// 00fa <result>.B
/// result:
///     0 = opens party window and shows MsgStringTable[77]="party successfully organized"
///     1 = MsgStringTable[78]="party name already exists"
///     2 = MsgStringTable[79]="already in a party"
///     3 = cannot organize parties on this map
///     ? = nothing
void clif_party_created( map_session_data& sd, int result ){
	PACKET_ZC_ACK_MAKE_GROUP p = {};

	p.PacketType = HEADER_ZC_ACK_MAKE_GROUP;
	p.result = result;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Adds new member to a party.
/// 0104 <account id>.L <role>.L <x>.W <y>.W <state>.B <party name>.24B <char name>.24B <map name>.16B (ZC_ADD_MEMBER_TO_GROUP)
/// 01e9 <account id>.L <role>.L <x>.W <y>.W <state>.B <party name>.24B <char name>.24B <map name>.16B <item pickup rule>.B <item share rule>.B (ZC_ADD_MEMBER_TO_GROUP2)
/// 0a43 <account id>.L <role>.L <class>.W <base level>.W <x>.W <y>.W <state>.B <party name>.24B <char name>.24B <map name>.16B <item pickup rule>.B <item share rule>.B (ZC_ADD_MEMBER_TO_GROUP3)
/// role:
///     0 = leader
///     1 = normal
/// state:
///     0 = connected
///     1 = disconnected
void clif_party_member_info( struct party_data& party, map_session_data& sd ){
	int id = party_getmemberid( &party, &sd );

	if( id < 0 ){
		return;
	}

	PACKET_ZC_ADD_MEMBER_TO_GROUP p = {};

	p.packetType = partymemberinfo;
	p.AID = sd.status.account_id;
#if PACKETVER >= 20171207
	p.GID = sd.status.char_id;
#endif
	p.leader = ( party.party.member[id].leader != 0 ) ? 0 : 1;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	p.class_ = sd.status.class_;
	p.baseLevel = sd.status.base_level;
#endif
	p.x = sd.bl.x;
	p.y = sd.bl.y;
	p.offline = ( party.party.member[id].online != 0 ) ? 0 : 1;
	safestrncpy( p.partyName, party.party.name, sizeof( p.partyName ) );
	safestrncpy( p.playerName, sd.status.name, sizeof( p.playerName ) );
	mapindex_getmapname_ext( map_mapid2mapname( sd.bl.m ), p.mapName );
	p.sharePickup = ( party.party.item&1 ) ? 1 : 0;
	p.shareLoot = ( party.party.item&2 )? 1 : 0;

	clif_send( &p, sizeof( p ), &sd.bl, PARTY );
}


/// Sends party information (ZC_GROUP_LIST).
/// 00fb <packet len>.W <party name>.24B { <account id>.L <nick>.24B <map name>.16B <role>.B <state>.B }*
/// role:
///     0 = leader
///     1 = normal
/// state:
///     0 = connected
///     1 = disconnected
/// 0a44 <packet len>.W <party name>.24B { <account id>.L <nick>.24B <map name>.16B <role>.B <state>.B <class>.W <base level>.W }* <item pickup rule>.B <item share rule>.B <unknown>.L
void clif_party_info( struct party_data& party, map_session_data* sd ){
	send_target target;

	if( sd == nullptr ){
		sd = party_getavailablesd( &party );

		if( sd == nullptr ){
			// Nobody is online to notify
			return;
		}

		// Send to whole party
		target = PARTY;
	}else{
		// Send only to self
		target = SELF;
	}

	PACKET_ZC_GROUP_LIST* p = reinterpret_cast<PACKET_ZC_GROUP_LIST*>( packet_buffer );

	p->packetType = partyinfo;
	p->packetLen = sizeof( *p );
	safestrncpy( p->partyName, party.party.name, sizeof( p->partyName ) );

	for( int i = 0, c = 0; i < MAX_PARTY; i++ ){
		struct party_member& m = party.party.member[i];

		if( m.account_id == 0 ){
			continue;
		}

		PACKET_ZC_GROUP_LIST_SUB& member = p->members[c];

		member.AID = m.account_id;
#if PACKETVER >= 20171207
		member.GID = m.char_id;
#endif
		safestrncpy( member.playerName, m.name, sizeof( member.playerName ) );
		mapindex_getmapname_ext( m.map, member.mapName );
		member.leader = ( m.leader ) ? 0 : 1;
		member.offline = ( m.online ) ? 0 : 1;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
		member.class_ = m.class_;
		member.baseLevel = m.lv;
#endif

		p->packetLen += static_cast<decltype(p->packetLen)>( sizeof( member ) );
		c++;
	}

	clif_send( p, p->packetLen, &sd->bl, target );
}


/// The player's 'party invite' state, sent during login (ZC_PARTY_CONFIG).
/// 02c9 <flag>.B
/// flag:
///     0 = allow party invites
///     1 = auto-deny party invites
void clif_partyinvitationstate( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20070911 || defined(PACKETVER_RE) || PACKETVER_AD_NUM >= 20070911 || PACKETVER_SAK_NUM >= 20070904 || defined(PACKETVER_ZERO)
	PACKET_ZC_PARTY_CONFIG p = {};

	p.packetType = HEADER_ZC_PARTY_CONFIG;
	p.denyPartyInvites = sd.status.disable_partyinvite;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}


/// Party invitation request.
/// 00fe <party id>.L <party name>.24B (ZC_REQ_JOIN_GROUP)
/// 02c6 <party id>.L <party name>.24B (ZC_PARTY_JOIN_REQ)
void clif_party_invite( map_session_data& sd, map_session_data& tsd ){
	struct party_data* party = party_search( sd.status.party_id );

	if( party == nullptr ){
		return;
	}

	PACKET_ZC_PARTY_JOIN_REQ p = {};

	p.PacketType = HEADER_ZC_PARTY_JOIN_REQ;
	p.GRID = party->party.party_id;
	safestrncpy( p.groupName, party->party.name, sizeof( p.groupName ) );

	clif_send( &p, sizeof( p ), &tsd.bl, SELF );
}


/// Party invite result.
/// 00fd <nick>.24S <result>.B (ZC_ACK_REQ_JOIN_GROUP)
/// 02c5 <nick>.24S <result>.L (ZC_PARTY_JOIN_REQ_ACK)
/// result=0  : char is already in a party -> MsgStringTable[80]
/// result=1  : party invite was rejected -> MsgStringTable[81]
/// result=2  : party invite was accepted -> MsgStringTable[82]
/// result=3  : party is full -> MsgStringTable[83]
/// result=4  : char of the same account already joined the party -> MsgStringTable[608]
/// result=5  : char blocked party invite -> MsgStringTable[1324] (since 20070904)
/// result=7  : char is not online or doesn't exist -> MsgStringTable[71] (since 20070904)
/// result=8  : (%s) are currently in restricted map to join a party. -> MsgStringTable[1388] (since 20080527)
/// result=9  : Cannot join a party in this map. -> MsgStringTable[1871] (since 20110215)
/// result=10 : You cannot invite or withdraw while in memorial dungeon -> message: MSG_ID_BD3 (since 20161130)
/// result=11 : The character is a level that can not join the party -> message: MSG_ID_C9A (since 20170412)
void clif_party_invite_reply( map_session_data& sd, const char* nick, enum e_party_invite_reply reply ){
#if PACKETVER < 20070904
	if( reply == PARTY_REPLY_OFFLINE ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 3 ) ); // Character not found.
		return;
	}
#endif

	PACKET_ZC_PARTY_JOIN_REQ_ACK p = {};

	p.PacketType = HEADER_ZC_PARTY_JOIN_REQ_ACK;
	safestrncpy( p.characterName, nick, sizeof( p.characterName ) );
	p.result = reply;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Updates party settings.
/// 0101 <exp option>.L (ZC_GROUPINFO_CHANGE)
/// 07d8 <exp option>.L <item pick rule>.B <item share rule>.B (ZC_REQ_GROUPINFO_CHANGE_V2)
/// exp option:
///     0 = exp sharing disabled
///     1 = exp sharing enabled
///     2 = cannot change exp sharing
///
/// flag:
///     0 = send to party
///     1 = send to sd
void clif_party_option(struct party_data *p,map_session_data *sd,int flag)
{
	unsigned char buf[16];
#if PACKETVER < 20090603
	const int cmd = 0x101;
#else
	const int cmd = 0x7d8;
#endif

	nullpo_retv(p);

	if(!sd && flag==0){
		int i;
		ARR_FIND(0,MAX_PARTY,i,p->data[i].sd);
		if (i < MAX_PARTY)
			sd = p->data[i].sd;
	}
	if(!sd) return;
	WBUFW(buf,0)=cmd;
	WBUFL(buf,2)=((flag&0x01)?2:p->party.exp);
#if PACKETVER >= 20090603
	WBUFB(buf,6)=(p->party.item&1)?1:0;
	WBUFB(buf,7)=(p->party.item&2)?1:0;
#endif
	if(flag==0)
		clif_send(buf,packet_len(cmd),&sd->bl,PARTY);
	else
		clif_send(buf,packet_len(cmd),&sd->bl,SELF);
}

/**
 * 0105 <account id>.L <char name>.24B <result>.B (ZC_DELETE_MEMBER_FROM_GROUP).
 * @param sd Send the notification for this player
 * @param account_id Account ID of kicked member
 * @param name Name of kicked member
 * @param result Party leave result @see PARTY_MEMBER_WITHDRAW
 * @param target Send target
 **/
void clif_party_withdraw( map_session_data& sd, uint32 account_id, const char* name, enum e_party_member_withdraw result, enum send_target target ){
	PACKET_ZC_DELETE_MEMBER_FROM_GROUP p = {};

	p.PacketType = HEADER_ZC_DELETE_MEMBER_FROM_GROUP;
	p.AID = account_id;
	safestrncpy( p.characterName, name, sizeof( p.characterName ) );
	p.result = result;

	clif_send( &p, sizeof( p ), &sd.bl, target );
}


/// Party chat message (ZC_NOTIFY_CHAT_PARTY).
/// 0109 <packet len>.W <account id>.L <message>.?B
void clif_party_message( struct party_data& party, uint32 account_id, const char* mes, size_t len ){
	map_session_data* sd = party_getavailablesd( &party );

	if( sd == nullptr ){
		return;
	}

	if (len > CHAT_SIZE_MAX) {
		ShowWarning( "clif_party_message: Truncated message '%s' (len=%d, max=%" PRIuPTR ", party_id=%d).\n", mes, len, CHAT_SIZE_MAX, party.party.party_id );
		len = CHAT_SIZE_MAX;
	}

	PACKET_ZC_NOTIFY_CHAT_PARTY* p = reinterpret_cast<PACKET_ZC_NOTIFY_CHAT_PARTY*>( packet_buffer );

	p->PacketType = HEADER_ZC_NOTIFY_CHAT_PARTY;
	p->PacketLength = sizeof( *p );
	p->AID = account_id;
	safestrncpy( p->chatMsg, mes, len );
	p->PacketLength += static_cast<decltype(p->PacketLength)>( len );

	clif_send( p, p->PacketLength, &sd->bl, PARTY );
}


/// Updates the position of a party member on the minimap (ZC_NOTIFY_POSITION_TO_GROUPM).
/// 0107 <account id>.L <x>.W <y>.W
void clif_party_xy( map_session_data& sd ){
	PACKET_ZC_NOTIFY_POSITION_TO_GROUPM p = {};

	p.PacketType = HEADER_ZC_NOTIFY_POSITION_TO_GROUPM;
	p.AID = sd.status.account_id;
	p.xPos = sd.bl.x;
	p.yPos = sd.bl.y;

	clif_send( &p, sizeof( p ), &sd.bl, PARTY_SAMEMAP_WOS );
}

/*==========================================
 * Sends x/y dot to a single fd. [Skotlex]
 *------------------------------------------*/
void clif_party_xy_single( map_session_data& sd, map_session_data& tsd ){
	PACKET_ZC_NOTIFY_POSITION_TO_GROUPM p = {};

	p.PacketType = HEADER_ZC_NOTIFY_POSITION_TO_GROUPM;
	p.AID = sd.status.account_id;
	p.xPos = sd.bl.x;
	p.yPos = sd.bl.y;

	clif_send( &p, sizeof( p ), &tsd.bl, SELF );
}

/// Updates HP bar of a party member.
/// 0106 <account id>.L <hp>.W <max hp>.W (ZC_NOTIFY_HP_TO_GROUPM)
/// 080e <account id>.L <hp>.L <max hp>.L (ZC_NOTIFY_HP_TO_GROUPM_R2)
void clif_party_hp( map_session_data& sd ){
	PACKET_ZC_NOTIFY_HP_TO_GROUPM p = {};

	p.PacketType = HEADER_ZC_NOTIFY_HP_TO_GROUPM;
	p.AID = sd.status.account_id;
#if PACKETVER < 20100119
	// To correctly display the %hp bar. [Skotlex]
	if( sd.battle_status.max_hp > INT16_MAX ){
		p.hp = sd.battle_status.hp / ( sd.battle_status.max_hp / 100 );
		p.maxhp = 100;
	}else{
		p.hp = sd.battle_status.hp;
		p.maxhp = sd.battle_status.max_hp;
	}
#else
	p.hp = sd.battle_status.hp;
	p.maxhp = sd.battle_status.max_hp;
#endif

	clif_send( &p, sizeof( p ), &sd.bl, PARTY_AREA_WOS );
}

/// Notifies the party members of a character's death or revival.
/// 0AB2 <GID>.L <dead>.B
void clif_party_dead( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	PACKET_ZC_GROUP_ISALIVE p = {};

	p.packetType = HEADER_ZC_GROUP_ISALIVE;
	p.AID = sd.status.account_id;
	p.isDead = pc_isdead( &sd );

	clif_send( &p, sizeof( p ), &sd.bl, PARTY );
#endif
}

/// Updates the job and level of a party member
/// 0abd <account id>.L <job>.W <level>.W
void clif_party_job_and_level( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	PACKET_ZC_NOTIFY_MEMBERINFO_TO_GROUPM p = {};

	p.PacketType = HEADER_ZC_NOTIFY_MEMBERINFO_TO_GROUPM;
	p.AID = sd.status.account_id;
	p.job = sd.status.class_;
	p.level = sd.status.base_level;

	clif_send( &p, sizeof( p ), &sd.bl, PARTY );
#endif
}

/*==========================================
 * Sends HP bar to a single fd. [Skotlex]
 *------------------------------------------*/
void clif_hpmeter_single( map_session_data& sd, uint32 id, uint32 hp, uint32 maxhp ){
	PACKET_ZC_NOTIFY_HP_TO_GROUPM p = {};

	p.PacketType = HEADER_ZC_NOTIFY_HP_TO_GROUPM;
	p.AID = id;
#if PACKETVER < 20100119
	// To correctly display the %hp bar. [Skotlex]
	if( maxhp > INT16_MAX ){
		p.hp = hp / ( maxhp / 100 );
		p.maxhp = 100;
	}else{
		p.hp = hp;
		p.maxhp = maxhp;
	}
#else
	p.hp = hp;
	p.maxhp = maxhp;
#endif

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Notifies the client, that it's attack target is too far.
/// 0139 <target id>.L <target x>.W <target y>.W <x>.W <y>.W <atk range>.W (ZC_ATTACK_FAILURE_FOR_DISTANCE)
void clif_movetoattack( map_session_data& sd, block_list& bl ){
	PACKET_ZC_ATTACK_FAILURE_FOR_DISTANCE packet{};

	packet.PacketType = HEADER_ZC_ATTACK_FAILURE_FOR_DISTANCE;
	packet.targetAID = bl.id;
	packet.targetXPos = bl.x;
	packet.targetYPos = bl.y;
	packet.xPos = sd.bl.x;
	packet.yPos = sd.bl.y;
	packet.currentAttRange = sd.battle_status.rhw.range;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Notifies the client about the result of an item produce request (ZC_ACK_REQMAKINGITEM).
/// 018f <result>.W <name id>.W
/// result:
///     0 = success
///     1 = failure
///     2 = success (alchemist)
///     3 = failure (alchemist)
void clif_produceeffect(map_session_data* sd,int flag, t_itemid nameid){
	nullpo_retv( sd );

	clif_solved_charname( sd->fd, sd->status.char_id, sd->status.name );

	PACKET_ZC_ACK_REQMAKINGITEM p = {};

	p.packetType = HEADER_ZC_ACK_REQMAKINGITEM;
	p.result = flag;
	p.itemId = client_nameid( nameid );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Initiates the pet taming process.
/// 019e (ZC_START_CAPTURE)
void clif_catch_process( map_session_data& sd ){
	PACKET_ZC_START_CAPTURE packet{};

	packet.PacketType = HEADER_ZC_START_CAPTURE;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Displays the result of a pet taming attempt.
/// 01a0 <result>.B (ZC_TRYCAPTURE_MONSTER)
///     0 = failure
///     1 = success
void clif_pet_roulette( map_session_data& sd, bool success ){
	PACKET_ZC_TRYCAPTURE_MONSTER packet{};

	packet.PacketType = HEADER_ZC_TRYCAPTURE_MONSTER;
	packet.result = success;

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}


/// Presents a list of pet eggs that can be hatched (ZC_PETEGG_LIST).
/// 01a6 <packet len>.W { <index>.W }*
void clif_sendegg(map_session_data *sd)
{
	int i,n=0,fd;

	nullpo_retv(sd);

	fd=sd->fd;
	if (battle_config.pet_no_gvg && map_flag_gvg2(sd->bl.m)) { //Disable pet hatching in GvG grounds during Guild Wars [Skotlex]
		clif_displaymessage(fd, msg_txt(sd,666));
		return;
	}
	WFIFOHEAD(fd, MAX_INVENTORY * 2 + 4);
	WFIFOW(fd,0)=0x1a6;
	for(i=0,n=0;i<MAX_INVENTORY;i++){
		if(sd->inventory.u.items_inventory[i].nameid == 0 || sd->inventory_data[i] == nullptr ||
		   sd->inventory_data[i]->type != IT_PETEGG ||
		   sd->inventory.u.items_inventory[i].amount <= 0)
			continue;
		WFIFOW(fd,n*2+4)=client_index(i);
		n++;
	}
	WFIFOW(fd,2)=4+n*2;
	WFIFOSET(fd,WFIFOW(fd,2));

	sd->menuskill_id = SA_TAMINGMONSTER;
	sd->menuskill_val = -1;
}


/// Sends a specific pet data update (ZC_CHANGESTATE_PET).
/// 01a4 <type>.B <id>.L <data>.L
/// type:
///     0 = pre-init (data = 0)
///     1 = intimacy (data = 0~4)
///     2 = hunger (data = 0~4)
///     3 = accessory
///     4 = performance (data = 1~3: normal, 4: special)
///     5 = hairstyle
///     6 = close egg selection ui and update egg in inventory (PACKETVER >= 20180704)
///
/// If sd is null, the update is sent to nearby objects, otherwise it is sent only to that player.
void clif_send_petdata(map_session_data* sd, struct pet_data* pd, int type, int param)
{
	uint8 buf[16];
	nullpo_retv(pd);

	WBUFW(buf,0) = 0x1a4;
	WBUFB(buf,2) = type;
	WBUFL(buf,3) = pd->bl.id;
	WBUFL(buf,7) = param;
	if (sd)
		clif_send(buf, packet_len(0x1a4), &sd->bl, SELF);
	else
		clif_send(buf, packet_len(0x1a4), &pd->bl, AREA);
}


/// Pet's base data (ZC_PROPERTY_PET).
/// 01a2 <name>.24B <renamed>.B <level>.W <hunger>.W <intimacy>.W <accessory id>.W <class>.W
void clif_send_petstatus(map_session_data *sd)
{
	int fd;
	struct s_pet *pet;

	nullpo_retv(sd);
	nullpo_retv(sd->pd);

	fd=sd->fd;
	pet = &sd->pd->pet;
	WFIFOHEAD(fd,packet_len(0x1a2));
	WFIFOW(fd,0)=0x1a2;
	safestrncpy(WFIFOCP(fd,2),pet->name,NAME_LENGTH);
	WFIFOB(fd,26)=battle_config.pet_rename?0:pet->rename_flag;
	WFIFOW(fd,27)=pet->level;
	WFIFOW(fd,29)=pet->hungry;
	WFIFOW(fd,31)=pet->intimate;
	WFIFOW(fd,33)=pet->equip;
#if PACKETVER >= 20081126
	WFIFOW(fd,35)=pet->class_;
#endif
	WFIFOSET(fd,packet_len(0x1a2));
}


/// Notification about a pet's emotion/talk (ZC_PET_ACT).
/// 01aa <id>.L <data>.L
/// data:
///     @see CZ_PET_ACT.
void clif_pet_emotion(struct pet_data *pd,int param)
{
	unsigned char buf[16];

	nullpo_retv(pd);

	memset(buf,0,packet_len(0x1aa));

	WBUFW(buf,0)=0x1aa;
	WBUFL(buf,2)=pd->bl.id;
	WBUFL(buf,6)=param;

	clif_send(buf,packet_len(0x1aa),&pd->bl,AREA);
}


/// Result of request to feed a pet.
/// 01a3 <result>.B <name id>.W (ZC_FEED_PET)
/// result:
///     0 = failure
///     1 = success
void clif_pet_food( map_session_data& sd, int32 foodid, bool success ){
	PACKET_ZC_FEED_PET packet{};

	packet.packetType = HEADER_ZC_FEED_PET;
	packet.result = success;
	packet.itemId = client_nameid( foodid );

	clif_send( &packet, sizeof( packet ), &sd.bl, SELF );
}

/// Send pet auto feed info.
void clif_pet_autofeed_status(map_session_data* sd, bool force) {
#if PACKETVER >= 20141008
	nullpo_retv(sd);

	if (force || battle_config.pet_autofeed_always) {
		// Always send ON or OFF
		if (sd->pd && battle_config.feature_pet_autofeed) {
			clif_configuration(sd, CONFIG_PET_AUTOFEED, sd->pd->pet.autofeed);
		}
		else {
			clif_configuration(sd, CONFIG_PET_AUTOFEED, false);
		}
	}
	else {
		// Only send when enabled
		if (sd->pd && battle_config.feature_pet_autofeed && sd->pd->pet.autofeed) {
			clif_configuration(sd, CONFIG_PET_AUTOFEED, true);
		}
	}
#endif
}

/// Presents a list of skills that can be auto-spelled.
/// 01cd { <skill id>.L }*7 (ZC_AUTOSPELLLIST)
void clif_autospell( map_session_data& sd, uint16 skill_lv ){
	struct s_autospell_requirement{
		uint16 skill_id;
		uint16 required_autospell_skill_lv;
	};

#ifdef RENEWAL
	 const std::vector<s_autospell_requirement> autospell_skills = {
		{ MG_FIREBOLT, 0 },
		{ MG_COLDBOLT, 0 },
		{ MG_LIGHTNINGBOLT, 0 },
		{ MG_SOULSTRIKE, 3 },
		{ MG_FIREBALL, 3 },
		{ WZ_EARTHSPIKE, 6 },
		{ MG_FROSTDIVER, 6 },
		{ MG_THUNDERSTORM, 9 },
		{ WZ_HEAVENDRIVE, 9 }
	};
#else
	const std::vector<s_autospell_requirement> autospell_skills = {
		{ MG_NAPALMBEAT, 0 },
		{ MG_COLDBOLT, 1 },
		{ MG_FIREBOLT, 1 },
		{ MG_LIGHTNINGBOLT, 1 },
		{ MG_SOULSTRIKE, 4 },
		{ MG_FIREBALL, 7 },
		{ MG_FROSTDIVER, 9 },
	};
#endif

#if PACKETVER_MAIN_NUM >= 20181128 || PACKETVER_RE_NUM >= 20181031
	PACKET_ZC_AUTOSPELLLIST* p = reinterpret_cast<PACKET_ZC_AUTOSPELLLIST*>( packet_buffer );

	p->packetType = HEADER_ZC_AUTOSPELLLIST;
	p->packetLength = sizeof( *p );

	size_t count = 0;
	for( const s_autospell_requirement& requirement : autospell_skills ){
		if( skill_lv > requirement.required_autospell_skill_lv && pc_checkskill( &sd, requirement.skill_id ) ){
			p->skills[count++] = requirement.skill_id;
			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->skills[0] ) );
		}
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
#elif PACKETVER_MAIN_NUM >= 20090406 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO) || PACKETVER_SAK_NUM >= 20080618
	PACKET_ZC_AUTOSPELLLIST p = {};

	p.packetType = HEADER_ZC_AUTOSPELLLIST;

	size_t count = 0;
	for( const s_autospell_requirement& requirement : autospell_skills ){
		if( count == ARRAYLENGTH( p.skills ) ){
			break;
		}

		if( skill_lv > requirement.required_autospell_skill_lv && pc_checkskill( &sd, requirement.skill_id ) ){
			p.skills[count++] = requirement.skill_id;
		}else{
			p.skills[count++] = 0;
		}
	}

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif

	sd.menuskill_id = SA_AUTOSPELL;
	sd.menuskill_val = skill_lv;
}


/// Devotion's visual effect (ZC_DEVOTIONLIST).
/// 01cf <devoter id>.L { <devotee id>.L }*5 <max distance>.W
void clif_devotion(struct block_list *src, map_session_data *tsd)
{
	unsigned char buf[56];

	nullpo_retv(src);
	memset(buf,0,packet_len(0x1cf));

	WBUFW(buf,0) = 0x1cf;
	WBUFL(buf,2) = src->id;
	if( src->type == BL_MER )
	{
		s_mercenary_data *md = BL_CAST(BL_MER,src);
		if( md && md->master && md->devotion_flag )
			WBUFL(buf,6) = md->master->bl.id;

		WBUFW(buf,26) = skill_get_range2(src, ML_DEVOTION, mercenary_checkskill(md, ML_DEVOTION), false);
	}
	else
	{
		int i;
		map_session_data *sd = BL_CAST(BL_PC,src);
		if( sd == nullptr )
			return;

		for( i = 0; i < 5 /*MAX_DEVOTION*/; i++ ) // Client only able show to 5 links
			WBUFL(buf,6+4*i) = sd->devotion[i];
		WBUFW(buf,26) = skill_get_range2(src, CR_DEVOTION, pc_checkskill(sd, CR_DEVOTION), false);
	}

	if( tsd )
		clif_send(buf, packet_len(0x1cf), &tsd->bl, SELF);
	else
		clif_send(buf, packet_len(0x1cf), src, AREA);
}

/// Notifies the client of an object's spirits.
/// 01d0 <id>.L <amount>.W (ZC_SPIRITS)
/// 01e1 <id>.L <amount>.W (ZC_SPIRITS2)
void clif_spiritball( struct block_list *bl, struct block_list* target, enum send_target send_target ){
	nullpo_retv( bl );

	PACKET_ZC_SPIRITS p = {};

	p.PacketType = HEADER_ZC_SPIRITS;
	p.AID = bl->id;

	switch( bl->type ){
		case BL_PC:
			p.num = ( (map_session_data*)bl )->spiritball;
			break;
		case BL_HOM:
			p.num = ( (struct homun_data*)bl )->homunculus.spiritball;
			break;
	}

	clif_send( &p, sizeof( p ), target == nullptr ? bl : target, send_target );
}

/// Notifies clients in area of a character's combo delay (ZC_COMBODELAY).
/// 01d2 <account id>.L <delay>.L
void clif_combo_delay(struct block_list *bl,t_tick wait)
{
	unsigned char buf[32];

	nullpo_retv(bl);

	WBUFW(buf,0)=0x1d2;
	WBUFL(buf,2)=bl->id;
	WBUFL(buf,6)=client_tick(wait);
	clif_send(buf,packet_len(0x1d2),bl,AREA);
}


/// Notifies clients in area that a character has blade-stopped another (ZC_BLADESTOP).
/// 01d1 <src id>.L <dst id>.L <flag>.L
/// flag:
///     0 = inactive
///     1 = active
void clif_bladestop(struct block_list *src, int dst_id, int active)
{
	unsigned char buf[32];

	nullpo_retv(src);

	WBUFW(buf,0)=0x1d1;
	WBUFL(buf,2)=src->id;
	WBUFL(buf,6)=dst_id;
	WBUFL(buf,10)=active;

	clif_send(buf,packet_len(0x1d1),src,AREA);
}


/// MVP effect (ZC_MVP).
/// 010c <account id>.L
void clif_mvp_effect(map_session_data *sd)
{
	unsigned char buf[16];

	nullpo_retv(sd);

	WBUFW(buf,0)=0x10c;
	WBUFL(buf,2)=sd->bl.id;
	clif_send(buf,packet_len(0x10c),&sd->bl,AREA);
}


/// MVP item reward message (ZC_MVP_GETTING_ITEM).
/// 010a <name id>.W
void clif_mvp_item( map_session_data *sd, t_itemid nameid ){
	PACKET_ZC_MVP_GETTING_ITEM p = {};

	p.packetType = HEADER_ZC_MVP_GETTING_ITEM;
	p.itemId = client_nameid( nameid );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// MVP EXP reward message (ZC_MVP_GETTING_SPECIAL_EXP).
/// 010b <exp>.L
void clif_mvp_exp(map_session_data *sd, t_exp exp) {
#if PACKETVER >= 20131223		// Kro remove this packet [Napster]
	if (battle_config.mvp_exp_reward_message) {
		char e_msg[CHAT_SIZE_MAX];
		sprintf(e_msg, msg_txt(sd, 717), exp);
		clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], e_msg, false, SELF);		// Congratulations! You are the MVP! Your reward EXP Points are %u !!
	}
#else
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd, packet_len(0x10b));
	WFIFOW(fd,0) = 0x10b;
	WFIFOL(fd,2) = (uint32)u64min( exp, MAX_EXP );
	WFIFOSET(fd, packet_len(0x10b));
#endif
}


/// Dropped MVP item reward message (ZC_THROW_MVPITEM).
/// 010d
///
/// "You are the MVP, but cannot obtain the reward because
///     you are overweight."
void clif_mvp_noitem(map_session_data* sd)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x10d));
	WFIFOW(fd,0) = 0x10d;
	WFIFOSET(fd,packet_len(0x10d));
}


/// Guild creation result.
/// 0167 <result>.B (ZC_RESULT_MAKE_GUILD)
/// result:
///     0 = "Guild has been created."
///     1 = "You are already in a Guild."
///     2 = "That Guild Name already exists."
///     3 = "You need the neccessary item to create a Guild."
void clif_guild_created( map_session_data& sd, int flag ){
	PACKET_ZC_RESULT_MAKE_GUILD p = {};

	p.packetType = HEADER_ZC_RESULT_MAKE_GUILD;
	p.result = static_cast<decltype(p.result)>( flag );

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Notifies the client that it is belonging to a guild (ZC_UPDATE_GDID).
/// 016c <guild id>.L <emblem id>.L <mode>.L <ismaster>.B <inter sid>.L <guild name>.24B
/// mode:
///     &0x01 = allow invite
///     &0x10 = allow expel
void clif_guild_belonginfo( map_session_data& sd ){
	if( sd.guild == nullptr ){
		return;
	}

	const auto &guild = sd.guild->guild;
	int position = guild_getposition(sd);

	if( position < 0 ){
		return;
	}

	PACKET_ZC_UPDATE_GDID p = {};

	p.PacketType = HEADER_ZC_UPDATE_GDID;
	p.guildId = guild.guild_id;
	p.emblemVersion = guild.emblem_id;
	p.mode = guild.position[position].mode;
	p.isMaster = ( sd.state.gmaster_flag == 1 );
	p.interSid = 0; // InterSID (unknown purpose)
	safestrncpy( p.guildName, guild.name, sizeof( p.guildName ) );
#if PACKETVER_MAIN_NUM >= 20220216
	p.masterGID = guild.member[0].char_id;
#endif

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Guild member login notice.
/// 016d <account id>.L <char id>.L <status>.L (ZC_UPDATE_CHARSTAT)
/// 01f2 <account id>.L <char id>.L <status>.L <gender>.W <hair style>.W <hair color>.W (ZC_UPDATE_CHARSTAT2)
/// status:
///     0 = offline
///     1 = online
void clif_guild_memberlogin_notice(const struct mmo_guild &g,int idx,int flag)
{
	unsigned char buf[64];
	map_session_data* sd;

	WBUFW(buf, 0)=0x1f2;
	WBUFL(buf, 2)=g.member[idx].account_id;
	WBUFL(buf, 6)=g.member[idx].char_id;
	WBUFL(buf,10)=flag;

	if( ( sd = g.member[idx].sd ) != nullptr )
	{
		WBUFW(buf,14) = sd->status.sex;
		WBUFW(buf,16) = sd->status.hair;
		WBUFW(buf,18) = sd->status.hair_color;
		clif_send(buf,packet_len(0x1f2),&sd->bl,GUILD_WOS);
	}
	else if( ( sd = guild_getavailablesd(g) ) != nullptr )
	{
		WBUFW(buf,14) = 0;
		WBUFW(buf,16) = 0;
		WBUFW(buf,18) = 0;
		clif_send(buf,packet_len(0x1f2),&sd->bl,GUILD);
	}
}

// Function `clif_guild_memberlogin_notice` sends info about
// logins and logouts of a guild member to the rest members.
// But at the 1st time (after a player login or map changing)
// the client won't show the message.
// So I suggest use this function for sending "first-time-info"
// to some player on entering the game or changing location.
// At next time the client would always show the message.
// The function sends all the statuses in the single packet
// to economize traffic. [LuzZza]
void clif_guild_send_onlineinfo(map_session_data *sd)
{
	unsigned char buf[14*128];
	int i, count=0, p_len;

	nullpo_retv(sd);

	p_len = packet_len(0x16d);

	auto &g = sd->guild;
	if (!g)
		return;

	for(i=0; i<g->guild.max_member; i++) {

		if(g->guild.member[i].account_id > 0 &&
			g->guild.member[i].account_id != sd->status.account_id) {

			WBUFW(buf,count*p_len) = 0x16d;
			WBUFL(buf,count*p_len+2) = g->guild.member[i].account_id;
			WBUFL(buf,count*p_len+6) = g->guild.member[i].char_id;
			WBUFL(buf,count*p_len+10) = g->guild.member[i].online;
			count++;
		}
	}

	clif_send(buf, p_len*count, &sd->bl, SELF);
}


/// Bitmask of enabled guild window tabs (ZC_ACK_GUILD_MENUINTERFACE).
/// 014e <menu flag>.L
/// menu flag:
///      0x00 = Basic Info (always on)
///     &0x01 = Member manager
///     &0x02 = Positions
///     &0x04 = Skills
///     &0x10 = Expulsion list
///     &0x40 = Unknown (GMENUFLAG_ALLGUILDLIST)
///     &0x80 = Notice
void clif_guild_masterormember(map_session_data *sd)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x14e));
	WFIFOW(fd,0) = 0x14e;
	WFIFOL(fd,2) = (sd->state.gmaster_flag) ? 0xd7 : 0x57;
	WFIFOSET(fd,packet_len(0x14e));
}


/// Guild basic information (Territories [Valaris])
/// 0150 <guild id>.L <level>.L <member num>.L <member max>.L <exp>.L <max exp>.L <points>.L <honor>.L <virtue>.L <emblem id>.L <name>.24B <master name>.24B <manage land>.16B (ZC_GUILD_INFO)
/// 01b6 <guild id>.L <level>.L <member num>.L <member max>.L <exp>.L <max exp>.L <points>.L <honor>.L <virtue>.L <emblem id>.L <name>.24B <master name>.24B <manage land>.16B <zeny>.L (ZC_GUILD_INFO2)
/// 0a84 <guild id>.L <level>.L <member num>.L <member max>.L <exp>.L <max exp>.L <points>.L <honor>.L <virtue>.L <emblem id>.L <name>.24B <manage land>.16B <zeny>.L <master char id>.L (ZC_GUILD_INFO3)
void clif_guild_basicinfo( map_session_data& sd ){
	if( sd.guild == nullptr ){
		return;
	}

	const auto &guild = sd.guild->guild;
	PACKET_ZC_GUILD_INFO p = {};

	p.PacketType = HEADER_ZC_GUILD_INFO;
	p.GDID = guild.guild_id;
	p.level = guild.guild_lv;
	p.userNum = guild.connect_member;
	p.maxUserNum = guild.max_member;
	p.userAverageLevel = guild.average_lv;
	p.exp = (uint32)cap_value( guild.exp, 0, MAX_GUILD_EXP );
	p.maxExp = (uint32)cap_value( guild.next_exp, 0, MAX_GUILD_EXP );
	p.point = 0; // Tax Points
	p.honor = 0; // Honor: (left) Vulgar [-100,100] Famed (right)
	p.virtue = 0; // Virtue: (down) Wicked [-100,100] Righteous (up)
	p.emblemVersion = guild.emblem_id;
	safestrncpy( p.guildname, guild.name, sizeof( p.guildname ) );
	safestrncpy( p.manageLand, msg_txt( &sd, 300 + guild_checkcastles( guild ) ), sizeof( p.manageLand ) );
	p.zeny = 0;
#if PACKETVER >= 20200902
	p.masterGID = guild.member[0].char_id; // leader
	safestrncpy( p.masterName, guild.master, sizeof( p.masterName ) );
#elif PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	p.masterGID = guild.member[0].char_id; // leader
#else
	safestrncpy( p.masterName, guild.master, sizeof( p.masterName ) );
#endif

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Guild alliance and opposition list (ZC_MYGUILD_BASIC_INFO).
/// 014c <packet len>.W { <relation>.L <guild id>.L <guild name>.24B }*
void clif_guild_allianceinfo(map_session_data *sd)
{
	int fd,i,c;

	nullpo_retv(sd);
	auto &g = sd->guild;
	if (!g)
		return;

	fd = sd->fd;
	WFIFOHEAD(fd, MAX_GUILDALLIANCE * 32 + 4);
	WFIFOW(fd, 0)=0x14c;
	for(i=c=0;i<MAX_GUILDALLIANCE;i++){
		struct guild_alliance *a=&g->guild.alliance[i];
		if(a->guild_id>0){
			WFIFOL(fd,c*32+4)=a->opposition;
			WFIFOL(fd,c*32+8)=a->guild_id;
			safestrncpy(WFIFOCP(fd,c*32+12),a->name,NAME_LENGTH);
			c++;
		}
	}
	WFIFOW(fd, 2)=c*32+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Guild member manager information
/// 0154 <packet len>.W { <account>.L <char id>.L <hair style>.W <hair color>.W <gender>.W <class>.W <level>.W <contrib exp>.L <state>.L <position>.L <memo>.50B <name>.24B }* (ZC_MEMBERMGR_INFO)
/// state:
///     0 = offline
///     1 = online
/// memo:
///     probably member's self-introduction (unused, no client UI/packets for editing it)
/// 0aa5 <packet len>.W { <account>.L <char id>.L <hair style>.W <hair color>.W <gender>.W <class>.W <level>.W <contrib exp>.L <state>.L <position>.L <lastlogin>.L }* (ZC_MEMBERMGR_INFO2)
void clif_guild_memberlist( map_session_data& sd ){
	if( sd.guild == nullptr ){
		return;
	}

	const auto &guild = sd.guild->guild;
	PACKET_ZC_MEMBERMGR_INFO* p = reinterpret_cast<PACKET_ZC_MEMBERMGR_INFO*>( packet_buffer );

	p->PacketType = HEADER_ZC_MEMBERMGR_INFO;
	p->packetLength = sizeof( *p );

	for( int i = 0, c = 0; i < guild.max_member; i++ ){
		const auto &member = guild.member[i];

		if( member.account_id == 0 ){
			continue;
		}

		GUILD_MEMBER_INFO& member_info = p->guildMemberInfo[c];

		member_info.AID = member.account_id;
		member_info.GID = member.char_id;
		member_info.head = member.hair;
		member_info.headPalette = member.hair_color;
		member_info.sex = member.gender;
		member_info.job = member.class_;
		member_info.level = member.lv;
		member_info.contributionExp = (uint32)cap_value( member.exp, 0, MAX_GUILD_EXP );
		member_info.currentState = member.online;
		member_info.positionID = member.position;
#if PACKETVER >= 20200902
		member_info.lastLoginTime = member.last_login;
		safestrncpy( member_info.char_name, member.name, sizeof( member_info.char_name ) );
#elif PACKETVER_MAIN_NUM >= 20161214 || PACKETVER_RE_NUM >= 20161130 || defined(PACKETVER_ZERO)
		member_info.lastLoginTime = member.last_login;
#else
		memset( member_info.intro, 0, sizeof( member_info.intro ) );  //[Ind] - This is displayed in the 'note' column but being you can't edit it it's sent empty.
		safestrncpy( member_info.char_name, member.name, sizeof( member_info.char_name ) );
#endif

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( member_info ) );
		c++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
}


/// Guild position name information (ZC_POSITION_ID_NAME_INFO).
/// 0166 <packet len>.W { <position id>.L <position name>.24B }*
void clif_guild_positionnamelist(map_session_data *sd)
{
	int i,fd;

	nullpo_retv(sd);
	auto &g = sd->guild;
	if (!g)
		return;

	fd = sd->fd;
	WFIFOHEAD(fd, MAX_GUILDPOSITION * 28 + 4);
	WFIFOW(fd, 0)=0x166;
	for(i=0;i<MAX_GUILDPOSITION;i++){
		WFIFOL(fd,i*28+4)=i;
		safestrncpy(WFIFOCP(fd,i*28+8),g->guild.position[i].name,NAME_LENGTH);
	}
	WFIFOW(fd,2)=i*28+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Guild position information (ZC_POSITION_INFO).
/// 0160 <packet len>.W { <position id>.L <mode>.L <ranking>.L <pay rate>.L }*
/// mode:
///     &0x01 = allow invite
///     &0x10 = allow expel
/// ranking:
///     TODO
void clif_guild_positioninfolist(map_session_data *sd)
{
	int i,fd;

	nullpo_retv(sd);
	auto &g = sd->guild;
	if (!g)
		return;

	fd = sd->fd;
	WFIFOHEAD(fd, MAX_GUILDPOSITION * 16 + 4);
	WFIFOW(fd, 0)=0x160;
	for(i=0;i<MAX_GUILDPOSITION;i++){
		struct guild_position *p=&g->guild.position[i];
		WFIFOL(fd,i*16+ 4)=i;
		WFIFOL(fd,i*16+ 8)=p->mode;
		WFIFOL(fd,i*16+12)=i;
		WFIFOL(fd,i*16+16)=p->exp_mode;
	}
	WFIFOW(fd, 2)=i*16+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Notifies clients in a guild about updated position information.
/// 0174 <packet len>.W { <position id>.L <mode>.L <ranking>.L <pay rate>.L <position name>.24B }* (ZC_ACK_CHANGE_GUILD_POSITIONINFO)
/// mode:
///     &0x01 = allow invite
///     &0x10 = allow expel
/// ranking:
///     TODO
void clif_guild_positionchanged(const struct mmo_guild &g,int idx)
{
	// FIXME: This packet is intended to update the clients after a
	// commit of position info changes, not sending one packet per
	// position.
	map_session_data *sd;
	unsigned char buf[128];

	WBUFW(buf, 0)=0x174;
	WBUFW(buf, 2)=44;  // packet len
	// GUILD_REG_POSITION_INFO{
	WBUFL(buf, 4)=idx;
	WBUFL(buf, 8)=g.position[idx].mode;
	WBUFL(buf,12)=idx;
	WBUFL(buf,16)=g.position[idx].exp_mode;
	safestrncpy(WBUFCP(buf,20),g.position[idx].name,NAME_LENGTH);
	// }*
	if( (sd=guild_getavailablesd(g))!=nullptr )
		clif_send(buf,WBUFW(buf,2),&sd->bl,GUILD);
}


/// Notifies clients in a guild about updated member position assignments.
/// 0156 <packet len>.W { <account id>.L <char id>.L <position id>.L }* (ZC_ACK_REQ_CHANGE_MEMBERS)
void clif_guild_memberpositionchanged(const struct mmo_guild &g, int idx)
{
	// FIXME: This packet is intended to update the clients after a
	// commit of member position assignment changes, not sending one
	// packet per position.
	map_session_data *sd;
	unsigned char buf[64];

	WBUFW(buf, 0)=0x156;
	WBUFW(buf, 2)=16;  // packet len
	// MEMBER_POSITION_INFO{
	WBUFL(buf, 4)=g.member[idx].account_id;
	WBUFL(buf, 8)=g.member[idx].char_id;
	WBUFL(buf,12)=g.member[idx].position;
	// }*
	if( (sd=guild_getavailablesd(g))!=nullptr )
		clif_send(buf,WBUFW(buf,2),&sd->bl,GUILD);
}


/// Sends emblems bitmap data to the client that requested it.
/// 0152 <packet len>.W <guild id>.L <emblem id>.L <emblem data>.?B (ZC_GUILD_EMBLEM_IMG)
void clif_guild_emblem(const map_session_data &sd, const struct mmo_guild &g)
{
	int fd = sd.fd;
	if( g.emblem_len <= 0 )
		return;

	WFIFOHEAD(fd,g.emblem_len+12);
	WFIFOW(fd,0)=0x152;
	WFIFOW(fd,2)=g.emblem_len+12;
	WFIFOL(fd,4)=g.guild_id;
	WFIFOL(fd,8)=g.emblem_id;
	memcpy(WFIFOP(fd,12),g.emblem_data,g.emblem_len);
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Sends update of the guild id/emblem id to everyone in the area.
/// 01b4 <id>.L <guild id>.L <emblem id>.W (ZC_CHANGE_GUILD)
/// 0b1f <guild id>.L <version>.L <AID>.L (ZC_NEW_EMBLEM_DOWNLOAD)
/// 0b47 <guild id>.L <version>.L <AID>.L (ZC_ACK_ADD_NEW_EMBLEM)
void clif_guild_emblem_area(struct block_list* bl)
{
	// TODO this packet doesn't force the update of ui components that have the emblem visible
	//      (emblem in the flag npcs and emblem over the head in agit maps) [FlavioJS]
	PACKET_ZC_CHANGE_GUILD p{};

	p.packetType = HEADER_ZC_CHANGE_GUILD;
	p.guild_id = status_get_guild_id(bl);
	p.emblem_id = status_get_emblem_id(bl);
	p.AID = bl->id;

	clif_send(&p, sizeof(p), bl, AREA_WOS);
}


/// Sends guild skills.
/// 0162 <packet len>.W <skill points>.W { <skill id>.W <type>.L <level>.W <sp cost>.W <atk range>.W <skill name>.24B <upgradable>.B }* (ZC_GUILD_SKILLINFO)
void clif_guild_skillinfo( map_session_data& sd ){
	auto& g = sd.guild;

	if( g == nullptr ){
		return;
	}

	PACKET_ZC_GUILD_SKILLINFO* p = reinterpret_cast<PACKET_ZC_GUILD_SKILLINFO*>( packet_buffer );

	p->PacketType = HEADER_ZC_GUILD_SKILLINFO;
	p->PacketLength = sizeof( *p );
	p->skillPoint = g->guild.skill_point;

	for( size_t i = 0, c = 0; i < MAX_GUILDSKILL; i++ ){
		if( g->guild.skill[i].id <= 0 ){
			continue;
		}

		if( !guild_check_skill_require( g->guild, g->guild.skill[i].id ) ){
			continue;
		}

		GUILD_SKILLDATA& gs = p->skillInfo[c];
		int skill_id = g->guild.skill[i].id;

		gs.id = skill_id;
		gs.inf = skill_get_inf(skill_id);
		gs.level = g->guild.skill[i].lv;
		if( g->guild.skill[i].lv > 0 ){
			gs.sp = skill_get_sp( skill_id, g->guild.skill[i].lv );
			gs.range2 = skill_get_range( skill_id, g->guild.skill[i].lv );
		}else{
			gs.sp = 0;
			gs.range2 = 0;
		}
		safestrncpy( gs.name, skill_get_name( skill_id ), sizeof( gs.name ) );
		gs.upFlag = ( g->guild.skill[i].lv < guild_skill_get_max( skill_id ) && &sd == g->guild.member[0].sd ) ? 1 : 0;

		p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof( gs ) );
		c++;
	}

	clif_send( p, p->PacketLength, &sd.bl, SELF );
}


/// Sends guild notice to client.
/// 016f <subject>.60B <notice>.120B (ZC_GUILD_NOTICE)
void clif_guild_notice( map_session_data& sd ){
	auto& g = sd.guild;

	if( g == nullptr ){
		return;
	}

	if( g->guild.mes1[0] == '\0' && g->guild.mes2[0] == '\0' ){
		return;
	}

	PACKET_ZC_GUILD_NOTICE p = {};

	p.packetType = HEADER_ZC_GUILD_NOTICE;
	safestrncpy( p.subject, g->guild.mes1, sizeof( p.subject ) );
	safestrncpy( p.notice, g->guild.mes2, sizeof( p.notice ) );

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Guild invite.
/// 016a <guild id>.L <guild name>.24B (ZC_REQ_JOIN_GUILD)
void clif_guild_invite( map_session_data& sd, const struct mmo_guild& g ){
	PACKET_ZC_REQ_JOIN_GUILD p = {};

	p.packetType = HEADER_ZC_REQ_JOIN_GUILD;
	p.guild_id = g.guild_id;
	safestrncpy( p.guild_name, g.name, sizeof( p.guild_name ) );

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Reply to invite request.
/// 0169 <answer>.B (ZC_ACK_REQ_JOIN_GUILD)
/// answer:
///     0 = Already in guild.
///     1 = Offer rejected.
///     2 = Offer accepted.
///     3 = Guild full.
void clif_guild_inviteack( map_session_data& sd, int flag ){
	PACKET_ZC_ACK_REQ_JOIN_GUILD p = {};

	p.packetType = HEADER_ZC_ACK_REQ_JOIN_GUILD;
	p.result = static_cast<decltype(p.result)>( flag );

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Notifies clients of a guild of a leaving member.
/// 015a <char name>.24B <reason>.40B (ZC_ACK_LEAVE_GUILD)
/// 0a83 <CID>.L <reason>.40B (ZC_ACK_LEAVE_GUILD_DELNAME)
void clif_guild_leave( map_session_data& sd, const char* name, uint32 char_id, const char* mes ){
	PACKET_ZC_ACK_LEAVE_GUILD p = {};

	p.packetType = guildLeave;
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	p.GID = char_id;
#else
	safestrncpy( p.name, name, sizeof( p.name ) );
#endif
	safestrncpy( p.reason, mes, sizeof( p.reason ) );

	clif_send( &p, sizeof( p ), &sd.bl, GUILD_NOBG );
}


/// Notifies clients of a guild of an expelled member.
/// 015c <char name>.24B <reason>.40B <account name>.24B (ZC_ACK_BAN_GUILD)
/// 0839 <char name>.24B <reason>.40B (ZC_ACK_BAN_GUILD_SSO)
/// 0a82 <CID>.L <reason>.40B (ZC_ACK_BAN_GUILD_DELNAME)
void clif_guild_expulsion( map_session_data& sd, const char* name, uint32 char_id, const char* mes ){
	PACKET_ZC_ACK_BAN_GUILD p = {};

	p.packetType = guildExpulsion;
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	p.GID = char_id;
#else
	safestrncpy( p.name, name, sizeof( p.name ) );
#endif
	safestrncpy( p.reason, mes, sizeof( p.reason ) );
#if PACKETVER < 20100803
	// account name is not sent for security reasons
	safestrncpy( p.account_name, "", sizeof( p.account_name ) );
#endif

	clif_send( &p, sizeof( p ), &sd.bl, GUILD_NOBG );
}


/// Guild expulsion list (ZC_BAN_LIST).
/// 0163 <packet len>.W { <char name>.24B <account name>.24B <reason>.40B }*
/// 0163 <packet len>.W { <char name>.24B <reason>.40B }* (PACKETVER >= 20100803)
void clif_guild_expulsionlist(map_session_data* sd)
{
#if PACKETVER < 20100803
	const int offset = NAME_LENGTH*2+40;
#else
	const int offset = NAME_LENGTH+40;
#endif
	int fd, i, c = 0;

	nullpo_retv(sd);

	auto &g = sd->guild;
	if (!g)
		return;

	fd = sd->fd;

	WFIFOHEAD(fd,4 + MAX_GUILDEXPULSION * offset);
	WFIFOW(fd,0) = 0x163;

	for( i = 0; i < MAX_GUILDEXPULSION; i++ )
	{
		struct guild_expulsion* e = &g->guild.expulsion[i];

		if( e->account_id > 0 )
		{
			safestrncpy(WFIFOCP(fd,4 + c*offset), e->name, NAME_LENGTH);
#if PACKETVER < 20100803
			memset(WFIFOP(fd,4 + c*offset+24), 0, NAME_LENGTH); // account name (not used for security reasons)
			memcpy(WFIFOP(fd,4 + c*offset+48), e->mes, 40);
#else
			memcpy(WFIFOP(fd,4 + c*offset+24), e->mes, 40);
#endif
			c++;
		}
	}
	WFIFOW(fd,2) = 4 + c*offset;
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Guild chat message (ZC_GUILD_CHAT).
/// 017f <packet len>.W <message>.?B
void clif_guild_message( const struct mmo_guild& g, uint32 account_id, const char* mes, size_t len ){
	// TODO: account_id is not used, candidate for deletion? [Ai4rei]
	map_session_data *sd;
	uint8 buf[256];

	if( len == 0 )
	{
		return;
	}
	else if( len > sizeof(buf)-5 )
	{
		ShowWarning("clif_guild_message: Truncated message '%s' (len=%d, max=%" PRIuPTR ", guild_id=%d).\n", mes, len, sizeof(buf)-5, g.guild_id);
		len = sizeof(buf)-5;
	}

	WBUFW(buf, 0) = 0x17f;
	WBUFW( buf, 2 ) = static_cast<int16>( len + 5 );
	safestrncpy(WBUFCP(buf,4), mes, len+1);

	if ((sd = guild_getavailablesd(g)) != nullptr)
		clif_send(buf, WBUFW(buf,2), &sd->bl, GUILD_NOBG);
}

/// Request for guild alliance (ZC_REQ_ALLY_GUILD).
/// 0171 <inviter account id>.L <guild name>.24B
void clif_guild_reqalliance(map_session_data *sd,uint32 account_id,const char *name)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x171));
	WFIFOW(fd,0)=0x171;
	WFIFOL(fd,2)=account_id;
	safestrncpy(WFIFOCP(fd,6),name,NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x171));
}


/// Notifies the client about the result of a alliance request (ZC_ACK_REQ_ALLY_GUILD).
/// 0173 <answer>.B
/// answer:
///     0 = Already allied.
///     1 = You rejected the offer.
///     2 = You accepted the offer.
///     3 = They have too any alliances.
///     4 = You have too many alliances.
///     5 = Alliances are disabled.
void clif_guild_allianceack(map_session_data *sd,int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x173));
	WFIFOW(fd,0)=0x173;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet_len(0x173));
}


/// Notifies the client that a alliance or opposition has been removed (ZC_DELETE_RELATED_GUILD).
/// 0184 <other guild id>.L <relation>.L
/// relation:
///     0 = Ally
///     1 = Enemy
void clif_guild_delalliance(map_session_data *sd,int guild_id,int flag)
{
	nullpo_retv(sd);

	int fd = sd->fd;

	if ( !session_isActive(fd) )
		return;

	WFIFOHEAD(fd,packet_len(0x184));
	WFIFOW(fd,0)=0x184;
	WFIFOL(fd,2)=guild_id;
	WFIFOL(fd,6)=flag;
	WFIFOSET(fd,packet_len(0x184));
}


/// Notifies the client about the result of a opposition request (ZC_ACK_REQ_HOSTILE_GUILD).
/// 0181 <result>.B
/// result:
///     0 = Antagonist has been set.
///     1 = Guild has too many Antagonists.
///     2 = Already set as an Antagonist.
///     3 = Antagonists are disabled.
void clif_guild_oppositionack(map_session_data *sd,int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x181));
	WFIFOW(fd,0)=0x181;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len(0x181));
}


/// Adds alliance or opposition (ZC_ADD_RELATED_GUILD).
/// 0185 <relation>.L <guild id>.L <guild name>.24B
/*
void clif_guild_allianceadded(struct guild *g,int idx)
{
	unsigned char buf[64];
	WBUFW(buf,0)=0x185;
	WBUFL(buf,2)=g->alliance[idx].opposition;
	WBUFL(buf,6)=g->alliance[idx].guild_id;
	safestrncpy(WBUFP(buf,10),g->alliance[idx].name,NAME_LENGTH);
	clif_send(buf,packet_len(0x185),guild_getavailablesd(g),GUILD);
}
*/


/// Notifies the client about the result of a guild break.
/// 015e <reason>.L (ZC_ACK_DISORGANIZE_GUILD_RESULT)
///     0 = success
///     1 = invalid key (guild name, @see clif_parse_GuildBreak)
///     2 = there are still members in the guild
void clif_guild_broken( map_session_data& sd, int flag ){
	PACKET_ZC_ACK_DISORGANIZE_GUILD_RESULT p = {};

	p.packetType = HEADER_ZC_ACK_DISORGANIZE_GUILD_RESULT;
	p.result = flag;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Displays emotion on an object (ZC_EMOTION).
/// 00c0 <id>.L <type>.B
/// type:
///     enum emotion_type
void clif_emotion(struct block_list *bl,int type)
{
	unsigned char buf[8];

	nullpo_retv(bl);

	WBUFW(buf,0)=0xc0;
	WBUFL(buf,2)=bl->id;
	WBUFB(buf,6)=type;
	clif_send(buf,packet_len(0xc0),bl,AREA);
}


/// Displays the contents of a talkiebox trap.
/// 0191 <id>.L <contents>.80B (ZC_TALKBOX_CHATCONTENTS)
void clif_talkiebox( struct block_list* bl, const char* talkie ){
	nullpo_retv( bl );
	nullpo_retv( talkie );

	PACKET_ZC_TALKBOX_CHATCONTENTS p = {};

	p.PacketType = HEADER_ZC_TALKBOX_CHATCONTENTS;
	p.aid = bl->id;
	safestrncpy( p.message, talkie, TALKBOX_MESSAGE_SIZE );

	clif_send( &p, sizeof( PACKET_ZC_TALKBOX_CHATCONTENTS ), bl, AREA );
}


/// Displays wedding effect centered on an object (ZC_CONGRATULATION).
/// 01ea <id>.L
void clif_wedding_effect(struct block_list *bl)
{
	unsigned char buf[6];

	nullpo_retv(bl);

	WBUFW(buf,0) = 0x1ea;
	WBUFL(buf,2) = bl->id;
	clif_send(buf, packet_len(0x1ea), bl, AREA);
}


/// Notifies the client of the name of the partner character (ZC_COUPLENAME).
/// 01e6 <partner name>.24B
void clif_callpartner(map_session_data& sd)
{
	PACKET_ZC_COUPLENAME p = { 0 };

	p.packetType = HEADER_ZC_COUPLENAME;

	if( sd.status.partner_id ) {
		map_session_data *p_sd = pc_get_partner(&sd);
		if (p_sd != nullptr && !p_sd->state.autotrade)
			safestrncpy(p.name, p_sd->status.name, NAME_LENGTH);
		else
			p.name[0] = 0;
	} else {// Send zero-length name if no partner, to initialize the client buffer.
		p.name[0] = 0;
	}

	clif_send(&p, sizeof(p), &sd.bl, AREA);
}


/// Initiates the partner "taming" process [DracoRPG] (ZC_START_COUPLE).
/// 01e4
/// This packet while still implemented by the client is no longer being officially used.
/*
void clif_marriage_process(map_session_data *sd)
{
	int fd;
	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x1e4));
	WFIFOW(fd,0)=0x1e4;
	WFIFOSET(fd,packet_len(0x1e4));
}
*/


/// Notice of divorce (ZC_DIVORCE).
/// 0205 <partner name>.24B
void clif_divorced(map_session_data* sd, const char* name)
{
	int fd;
	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x205));
	WFIFOW(fd,0)=0x205;
	safestrncpy(WFIFOCP(fd,2), name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x205));
}


/// Marriage proposal (ZC_REQ_COUPLE).
/// 01e2 <account id>.L <char id>.L <char name>.24B
/// This packet while still implemented by the client is no longer being officially used.
/*
void clif_marriage_proposal(int fd, map_session_data *sd, map_session_data* ssd)
{
	nullpo_retv(sd);

	WFIFOHEAD(fd,packet_len(0x1e2));
	WFIFOW(fd,0) = 0x1e2;
	WFIFOL(fd,2) = ssd->status.account_id;
	WFIFOL(fd,6) = ssd->status.char_id;
	safestrncpy(WFIFOCP(fd,10), ssd->status.name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x1e2));
}
*/


/*==========================================
 * Displays a message using the guild-chat colors to the specified targets. [Skotlex]
 *------------------------------------------*/
void clif_disp_message(struct block_list* src, const char* mes, size_t len, enum send_target target)
{
	unsigned char buf[256];

	if( len == 0 ) {
		return;
	} else if( len > sizeof(buf)-5 ) {
		ShowWarning("clif_disp_message: Truncated message '%s' (len=%d, max=%" PRIuPTR ", aid=%d).\n", mes, len, sizeof(buf)-5, src->id);
		len = sizeof(buf)-5;
	}

	WBUFW(buf, 0) = 0x17f;
	WBUFW(buf, 2) = static_cast<int16>( len + 5 );
	safestrncpy(WBUFCP(buf,4), mes, len+1);
	clif_send(buf, WBUFW(buf,2), src, target);
}


/// Notifies the client about the result of a request to disconnect another player (ZC_ACK_DISCONNECT_CHARACTER).
/// 00cd <result>.L (unknown packet version or invalid information at packet_len_table)
/// 00cd <result>.B
/// result:
///     0 = failure
///     1 = success
void clif_GM_kickack(map_session_data *sd, int id)
{
	int fd;

	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0xcd));
	WFIFOW(fd,0) = 0xcd;
	WFIFOB(fd,2) = id;  // FIXME: this is not account id
	WFIFOSET(fd, packet_len(0xcd));
}


void clif_GM_kick(map_session_data *sd, map_session_data *tsd)
{
	nullpo_retv(tsd);

	if (sd == nullptr)
		tsd->state.keepshop = true;

	if (session_isActive(tsd->fd))
		clif_authfail_fd(tsd->fd, 15);
	else
		map_quit(tsd);

	if (sd)
		clif_GM_kickack(sd, tsd->status.account_id);
}


/// Displays various manner-related status messages (ZC_ACK_GIVE_MANNER_POINT).
/// 014a <result>.L
/// result:
///     0 = "A manner point has been successfully aligned."
///     1 = MP_FAILURE_EXHAUST
///     2 = MP_FAILURE_ALREADY_GIVING
///     3 = "Chat Block has been applied by GM due to your ill-mannerous action."
///     4 = "Automated Chat Block has been applied due to Anti-Spam System."
///     5 = "You got a good point from %s."
void clif_manner_message(map_session_data* sd, uint32 type)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x14a));
	WFIFOW(fd,0) = 0x14a;
	WFIFOL(fd,2) = type;
	WFIFOSET(fd, packet_len(0x14a));
}


/// Followup to 0x14a type 3/5, informs who did the manner adjustment action (ZC_NOTIFY_MANNER_POINT_GIVEN).
/// 014b <type>.B <GM name>.24B
/// type:
///     0 = positive (unmute)
///     1 = negative (mute)
void clif_GM_silence(map_session_data* sd, map_session_data* tsd, uint8 type)
{
	int fd;
	nullpo_retv(sd);
	nullpo_retv(tsd);

	fd = tsd->fd;
	WFIFOHEAD(fd,packet_len(0x14b));
	WFIFOW(fd,0) = 0x14b;
	WFIFOB(fd,2) = type;
	safestrncpy(WFIFOCP(fd,3), sd->status.name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x14b));
}


/// Notifies the client about the result of a request to allow/deny whispers from a player (ZC_SETTING_WHISPER_PC).
/// 00d1 <type>.B <result>.B
/// type:
///     0 = /ex (deny)
///     1 = /in (allow)
/// result:
///     0 = success
///     1 = failure
///     2 = too many blocks
void clif_wisexin(map_session_data *sd,int type,int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xd1));
	WFIFOW(fd,0)=0xd1;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_len(0xd1));
}

/// Notifies the client about the result of a request to allow/deny whispers from anyone (ZC_SETTING_WHISPER_STATE).
/// 00d2 <type>.B <result>.B
/// type:
///     0 = /exall (deny)
///     1 = /inall (allow)
/// result:
///     0 = success
///     1 = failure
void clif_wisall(map_session_data *sd,int type,int flag)
{
	int fd;

	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0xd2));
	WFIFOW(fd,0)=0xd2;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_len(0xd2));
}

/// Play a BGM! [Rikter/Yommy] (ZC_PLAY_NPC_BGM).
/// 07fe <bgm>.24B
void clif_playBGM( map_session_data& sd, const char* name ){
#if PACKETVER_MAIN_NUM >= 20220504
	PACKET_ZC_PLAY_NPC_BGM* p = reinterpret_cast<PACKET_ZC_PLAY_NPC_BGM*>( packet_buffer );
	size_t length = strlen( name ) + 1;

	p->PacketType = HEADER_ZC_PLAY_NPC_BGM;
	p->PacketLength = sizeof( *p );
	p->playType = 0; // TODO: implement, send loop for the time being
	safestrncpy( p->bgm, name, length );
	p->PacketLength += static_cast<decltype(p->PacketLength)>( length );

	clif_send( p, p->PacketLength, &sd.bl, SELF );
#elif PACKETVER >= 20091201
	PACKET_ZC_PLAY_NPC_BGM p = {};

	p.PacketType = HEADER_ZC_PLAY_NPC_BGM;
	safestrncpy( p.bgm, name, sizeof( p.bgm ) );

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/// Plays/stops a wave sound (ZC_SOUND).
/// 01d3 <file name>.24B <act>.B <term>.L <npc id>.L
/// file name:
///     relative to data\wav
/// act:
///     0 = play (once)
///     1 = play (repeat, does not work)
///     2 = stops all sound instances of file name (does not work)
/// term:
///     unknown purpose, only relevant to act = 1
/// npc id:
///     The accustic direction of the sound is determined by the
///     relative position of the NPC to the player (3D sound).
void clif_soundeffect( struct block_list& bl, const char* name, int type, enum send_target target ){
	PACKET_ZC_SOUND p = {};

	p.PacketType = HEADER_ZC_SOUND;
	safestrncpy( p.name, name, sizeof( p.name ) );
	p.act = type;
	p.term = 0; // TODO: implement
	p.AID = bl.id;

	clif_send( &p, sizeof( p ), &bl, target );
}

/// Displays special effects (npcs, weather, etc) [Valaris] (ZC_NOTIFY_EFFECT2).
/// 01f3 <id>.L <effect id>.L
/// effect id:
///     @see doc/effect_list.txt
void clif_specialeffect(struct block_list* bl, int type, enum send_target target)
{
	unsigned char buf[24];

	nullpo_retv(bl);

	memset(buf, 0, packet_len(0x1f3));

	WBUFW(buf,0) = 0x1f3;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = type;

	clif_send(buf, packet_len(0x1f3), bl, target);

	if (disguised(bl)) {
		WBUFL(buf,2) = disguised_bl_id(bl->id);
		clif_send(buf, packet_len(0x1f3), bl, SELF);
	}
}

void clif_specialeffect_single(struct block_list* bl, int type, int fd)
{
	WFIFOHEAD(fd,10);
	WFIFOW(fd,0) = 0x1f3;
	WFIFOL(fd,2) = bl->id;
	WFIFOL(fd,6) = type;
	WFIFOSET(fd,10);
}


/// Notifies clients of an special/visual effect that accepts an value (ZC_NOTIFY_EFFECT3).
/// 0284 <id>.L <effect id>.L <num data>.L
/// effect id:
///     @see doc/effect_list.txt
/// num data:
///     effect-dependent value
void clif_specialeffect_value(struct block_list* bl, int effect_id, int num, send_target target)
{
	uint8 buf[14];

	WBUFW(buf,0) = 0x284;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = effect_id;
	WBUFL(buf,10) = num;

	clif_send(buf, packet_len(0x284), bl, target);

	if( disguised(bl) )
	{
		WBUFL(buf,2) = disguised_bl_id(bl->id);
		clif_send(buf, packet_len(0x284), bl, SELF);
	}
}

void clif_specialeffect_remove(struct block_list* bl_src, int effect, enum send_target e_target, struct block_list* bl_target)
{
#if PACKETVER >= 20181002
	nullpo_retv( bl_src );
	nullpo_retv( bl_target );

	PACKET_ZC_REMOVE_EFFECT p = {};

	p.packetType = HEADER_ZC_REMOVE_EFFECT;
	p.aid = bl_src->id;
	p.effectId = effect;

	clif_send( &p, sizeof( PACKET_ZC_REMOVE_EFFECT ), bl_target, e_target );

	if( disguised(bl_src) )
	{
		p.aid = disguised_bl_id( bl_src->id );
		clif_send( &p, sizeof( PACKET_ZC_REMOVE_EFFECT ), bl_src, SELF );
	}
#endif
}

/// Monster/NPC color chat [SnakeDrak] (ZC_NPC_CHAT).
/// 02c1 <packet len>.W <id>.L <color>.L <message>.?B
void clif_messagecolor_target(struct block_list *bl, unsigned long color, const char *msg, bool rgb2bgr, enum send_target type, map_session_data *sd) {
	unsigned short msg_len = (unsigned short)(strlen(msg) + 1);
	uint8 buf[CHAT_SIZE_MAX];

	nullpo_retv(bl);

	if (msg_len > sizeof(buf) - 12) {
		ShowWarning("clif_messagecolor: Truncating too long message '%s' (len=%u).\n", msg, msg_len);
		msg_len = sizeof(buf) - 12;
	}

	if (rgb2bgr)
		color = (color & 0x0000FF) << 16 | (color & 0x00FF00) | (color & 0xFF0000) >> 16; // RGB to BGR

	WBUFW(buf,0) = 0x2C1;
	WBUFW(buf,2) = msg_len + 12;
	WBUFL(buf,4) = bl->id;
	WBUFL(buf,8) = color;
	memcpy(WBUFCP(buf,12), msg, msg_len);

	clif_send(buf, WBUFW(buf,2), (sd == nullptr ? bl : &(sd->bl)), type);
}

/**
 * Notifies the client that the storage window is still open
 *
 * Should only be used in cases where the client closed the 
 * storage window without server's consent
 */
void clif_refresh_storagewindow(map_session_data *sd) {
	// Notify the client that the storage is open
	if( sd->state.storage_flag == 1 ) {
		storage_sortitem(sd->storage.u.items_storage, ARRAYLENGTH(sd->storage.u.items_storage));
		clif_storagelist(sd, sd->storage.u.items_storage, ARRAYLENGTH(sd->storage.u.items_storage), storage_getName(0));
		clif_updatestorageamount(*sd, sd->storage.amount, sd->storage.max_amount);
	}
	// Notify the client that the gstorage is open otherwise it will
	// remain locked forever and nobody will be able to access it
	if( sd->state.storage_flag == 2 ) {
		struct s_storage *gstor = guild2storage2(sd->status.guild_id);

		if( !gstor ) // Shouldn't happen. The information should already be at the map-server
			intif_request_guild_storage(sd->status.account_id, sd->status.guild_id);
		else {
			storage_sortitem(gstor->u.items_guild, ARRAYLENGTH(gstor->u.items_guild));
			clif_storagelist(sd, gstor->u.items_guild, ARRAYLENGTH(gstor->u.items_guild), "Guild Storage");
			clif_updatestorageamount(*sd, gstor->amount, gstor->max_amount);
		}
	}
	// Notify the client that the premium storage is open
	if (sd->state.storage_flag == 3) {
		storage_sortitem(sd->premiumStorage.u.items_storage, ARRAYLENGTH(sd->premiumStorage.u.items_storage));
		clif_storagelist(sd, sd->premiumStorage.u.items_storage, ARRAYLENGTH(sd->premiumStorage.u.items_storage), storage_getName(sd->premiumStorage.stor_id));
		clif_updatestorageamount(*sd, sd->premiumStorage.amount, sd->premiumStorage.max_amount);
	}
}

// refresh the client's screen, getting rid of any effects
void clif_refresh(map_session_data *sd)
{
	nullpo_retv(sd);

	clif_changemap( *sd, sd->bl.m, sd->bl.x, sd->bl.y );
	clif_inventorylist(sd);
	clif_equipswitch_list(sd);
	if(pc_iscarton(sd)) {
		clif_cartlist(sd);
		clif_updatestatus(*sd,SP_CARTINFO);
	}
	clif_updatestatus(*sd,SP_WEIGHT);
	clif_updatestatus(*sd,SP_MAXWEIGHT);
	clif_updatestatus(*sd,SP_STR);
	clif_updatestatus(*sd,SP_AGI);
	clif_updatestatus(*sd,SP_VIT);
	clif_updatestatus(*sd,SP_INT);
	clif_updatestatus(*sd,SP_DEX);
	clif_updatestatus(*sd,SP_LUK);
#ifdef RENEWAL
	clif_updatestatus(*sd,SP_POW);
	clif_updatestatus(*sd,SP_STA);
	clif_updatestatus(*sd,SP_WIS);
	clif_updatestatus(*sd,SP_SPL);
	clif_updatestatus(*sd,SP_CON);
	clif_updatestatus(*sd,SP_CRT);
#endif
	if (sd->spiritball)
		clif_spiritball( &sd->bl, &sd->bl, SELF );
	clif_millenniumshield_single( *sd, *sd );
	clif_spiritcharm_single( *sd, *sd );
	if (sd->soulball)
		clif_soulball( sd, &sd->bl, SELF );
	if (sd->servantball)
		clif_servantball( *sd, &sd->bl, SELF );
	if (sd->abyssball)
		clif_abyssball( *sd, &sd->bl, SELF );
	if (sd->vd.cloth_color)
		clif_refreshlook(&sd->bl,sd->bl.id,LOOK_CLOTHES_COLOR,sd->vd.cloth_color,SELF);
	if (sd->vd.body_style)
		clif_refreshlook(&sd->bl,sd->bl.id,LOOK_BODY2,sd->vd.body_style,SELF);
	if(hom_is_active(sd->hd))
		clif_send_homdata( *sd->hd, SP_ACK );
	if( sd->md ) {
		clif_mercenary_info(sd);
		clif_mercenary_skillblock(sd);
	}
	if( sd->ed )
		clif_elemental_info(sd);
	map_foreachinallrange(clif_getareachar,&sd->bl,AREA_SIZE,BL_ALL,sd);
	clif_weather_check(sd);
	if( sd->chatID )
		chat_leavechat(sd,0);
	if( sd->state.vending )
		clif_openvending( *sd );
	if( pc_issit(sd) )
		clif_sitting(&sd->bl); // FIXME: just send to self, not area
	if( pc_isdead(sd) ) // When you refresh, resend the death packet.
		clif_clearunit_single( sd->bl.id, CLR_DEAD, *sd );
	else
		clif_changed_dir(&sd->bl, SELF);
	clif_efst_status_change_sub(&sd->bl,&sd->bl,SELF);

	//Issue #2143
	//Cancel Trading State 
	if (sd->state.trading)
		trade_tradecancel(sd);
	//Cancel Buying/Selling State
	if (sd->state.buyingstore)
		buyingstore_close(sd);


	mail_clear(sd);

	clif_loadConfirm( sd );

	if( disguised(&sd->bl) ) {/* refresh-da */
		short disguise = sd->disguise;
		pc_disguise(sd, 0);
		pc_disguise(sd, disguise);
	}
	clif_refresh_storagewindow(sd);
}


/// Updates the object's (bl) name on client.
/// Used to update when a char leaves a party/guild. [Skotlex]
/// Needed because when you send a 0x95 packet, the client will not remove the cached party/guild info that is not sent.
/// 0095 <id>.L <char name>.24B (ZC_ACK_REQNAME)
/// 0195 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B (ZC_ACK_REQNAMEALL)
/// 0a30 <id>.L <char name>.24B <party name>.24B <guild name>.24B <position name>.24B <title ID>.L (ZC_ACK_REQNAMEALL2)
void clif_name( struct block_list* src, struct block_list *bl, send_target target ){
	nullpo_retv( src );
	nullpo_retv( bl );

	switch( bl->type ){
		case BL_PC: {
			PACKET_ZC_ACK_REQNAMEALL packet = { 0 };

			packet.packet_id = HEADER_ZC_ACK_REQNAMEALL;
			packet.gid = bl->id;

			map_session_data *sd = (map_session_data *)bl;

			//Requesting your own "shadow" name. [Skotlex]
			if( src == bl && target == SELF && sd->disguise ){
				packet.gid = -bl->id;
			}

			if( sd->fakename[0] ) {
				safestrncpy( packet.name, sd->fakename, NAME_LENGTH );
				clif_send( &packet, sizeof(packet), src, target );
				return;
			}

			safestrncpy( packet.name, sd->status.name, NAME_LENGTH );

			party_data *p = nullptr;

			if( sd->status.party_id ){
				p = party_search( sd->status.party_id );
			}

			// do not display party unless the player is also in a guild
			if( p && ( sd->guild || battle_config.display_party_name ) ){
				safestrncpy( packet.party_name, p->party.name, NAME_LENGTH );
			}

			if( sd->guild ){
				int position;

				// Will get the position of the guild the player is in
				position = guild_getposition(*sd);

				safestrncpy( packet.guild_name, sd->guild->guild.name, NAME_LENGTH );
				safestrncpy( packet.position_name, sd->guild->guild.position[position].name, NAME_LENGTH );
			}else if( sd->clan ){
				safestrncpy( packet.position_name, sd->clan->name, NAME_LENGTH );
			}

#if PACKETVER_MAIN_NUM >= 20150225 || PACKETVER_RE_NUM >= 20141126 || defined( PACKETVER_ZERO )
			packet.title_id = sd->status.title_id; // Title ID
#endif

			clif_send(&packet, sizeof(packet), src, target);
		}
			break;
		//[blackhole89]
		case BL_HOM:
		case BL_MER:
		case BL_PET:
		case BL_NPC:
		case BL_ELEM: {
			PACKET_ZC_ACK_REQNAMEALL_NPC packet = { 0 };

			packet.packet_id = HEADER_ZC_ACK_REQNAMEALL_NPC;
			packet.gid = bl->id;

			switch (bl->type) {
				case BL_HOM:
					memcpy(packet.name, ((TBL_HOM *)bl)->homunculus.name, NAME_LENGTH);
					break;
				case BL_MER:
					memcpy(packet.name, ((TBL_MER *)bl)->db->name.c_str(), NAME_LENGTH);
					break;
				case BL_PET:
					safestrncpy(packet.name, ((TBL_PET *)bl)->pet.name, NAME_LENGTH);
					break;
				case BL_NPC:
					safestrncpy(packet.name, ((TBL_NPC *)bl)->name, NAME_LENGTH);
					break;
				case BL_ELEM:
					safestrncpy(packet.name, ((TBL_ELEM *)bl)->db->name.c_str(), NAME_LENGTH);
					break;
			}

#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
			unit_data *ud = unit_bl2ud(bl);

			if (ud != nullptr) {
				memcpy(packet.title, ud->title, NAME_LENGTH);
				packet.groupId = ud->group_id;
			}
#endif

			clif_send(&packet, sizeof(packet), src, target);
		}
			break;
		case BL_MOB: {
			mob_data *md = (mob_data *)bl;

			if( md->guardian_data && md->guardian_data->guild_id ){
				PACKET_ZC_ACK_REQNAMEALL packet = { 0 };

				packet.packet_id = HEADER_ZC_ACK_REQNAMEALL;
				packet.gid = bl->id;
				safestrncpy( packet.name, md->name, NAME_LENGTH );
				safestrncpy( packet.guild_name, md->guardian_data->guild_name, NAME_LENGTH );
				safestrncpy( packet.position_name, md->guardian_data->castle->castle_name, NAME_LENGTH );

				clif_send(&packet, sizeof(packet), src, target);
			}else if( battle_config.show_mob_info ){
				PACKET_ZC_ACK_REQNAMEALL packet = { 0 };

				packet.packet_id = HEADER_ZC_ACK_REQNAMEALL;
				packet.gid = bl->id;
				safestrncpy( packet.name, md->name, NAME_LENGTH );

				char mobhp[50], *str_p = mobhp;

				if( battle_config.show_mob_info&4 ){
					str_p += sprintf( str_p, "Lv. %d | ", md->level );
				}

				if( battle_config.show_mob_info&1 ){
					str_p += sprintf( str_p, "HP: %u/%u | ", md->status.hp, md->status.max_hp );
				}

				if( battle_config.show_mob_info&2 ){
					str_p += sprintf( str_p, "HP: %u%% | ", get_percentage( md->status.hp, md->status.max_hp ) );
				}

				// Even thought mobhp ain't a name, we send it as one so the client can parse it. [Skotlex]
				if( str_p != mobhp ){
					*(str_p-3) = '\0'; //Remove trailing space + pipe.
					safestrncpy( packet.party_name, mobhp, NAME_LENGTH );
				}

				clif_send(&packet, sizeof(packet), src, target);
			} else {
				PACKET_ZC_ACK_REQNAMEALL_NPC packet = { 0 };

				packet.packet_id = HEADER_ZC_ACK_REQNAMEALL_NPC;
				packet.gid = bl->id;
				safestrncpy(packet.name, md->name, NAME_LENGTH);

#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
				unit_data *ud = unit_bl2ud(bl);

				if (ud != nullptr) {
					memcpy(packet.title, ud->title, NAME_LENGTH);
					packet.groupId = ud->group_id;
				}
#endif

				clif_send(&packet, sizeof(packet), src, target);
			}
		}
			break;
		case BL_CHAT:
		case BL_SKILL:
			// Newer clients request this, but do not need an answer
			return;
		default:
			ShowError("clif_name: bad type %d(%d)\n", bl->type, bl->id);
			return;
	}
}

/// Taekwon Jump (TK_HIGHJUMP) effect (ZC_HIGHJUMP).
/// 01ff <id>.L <x>.W <y>.W
///
/// Visually moves(instant) a character to x,y. The char moves even
/// when the target cell isn't walkable. If the char is sitting it
/// stays that way.
void clif_slide(struct block_list *bl, int x, int y)
{
	unsigned char buf[10];
	nullpo_retv(bl);

	WBUFW(buf, 0) = 0x01ff;
	WBUFL(buf, 2) = bl->id;
	WBUFW(buf, 6) = x;
	WBUFW(buf, 8) = y;
	clif_send(buf, packet_len(0x1ff), bl, AREA);

	if( disguised(bl) )
	{
		WBUFL(buf,2) = disguised_bl_id(bl->id);
		clif_send(buf, packet_len(0x1ff), bl, SELF);
	}
}


/// Public chat message (ZC_NOTIFY_CHAT). lordalfa/Skotlex - used by @me as well
/// 008d <packet len>.W <id>.L <message>.?B
void clif_disp_overhead_(struct block_list *bl, const char* mes, enum send_target flag)
{
	unsigned char buf[256]; //This should be more than sufficient, the theorical max is CHAT_SIZE + 8 (pads and extra inserted crap)
	int16 len_mes;

	len_mes = static_cast<decltype(len_mes)>( strlen(mes) + 1 ); //Account for \0

	if (len_mes > sizeof(buf)-8) {
		ShowError("clif_disp_overhead: Message too long (length %d)\n", len_mes);
		len_mes = sizeof(buf)-8; //Trunk it to avoid problems.
	}
	// send message to others
	if (flag == AREA) {
		WBUFW(buf,0) = 0x8d;
		WBUFW(buf,2) = len_mes + 8; // len of message + 8 (command+len+id)
		WBUFL(buf,4) = bl->id;
		safestrncpy(WBUFCP(buf,8), mes, len_mes);
		clif_send(buf, WBUFW(buf,2), bl, AREA_CHAT_WOC);
	}

	// send back message to the speaker
	if( bl->type == BL_PC ) {
		WBUFW(buf,0) = 0x8e;
		WBUFW(buf, 2) = len_mes + 4;
		safestrncpy(WBUFCP(buf,4), mes, len_mes);
		clif_send(buf, WBUFW(buf,2), bl, SELF);
	}
}

/*==========================
 * Minimap fix [Kevin]
 * Remove dot from minimap
 *--------------------------*/
void clif_party_xy_remove(map_session_data* sd)
{
	nullpo_retv(sd);

	PACKET_ZC_NOTIFY_POSITION_TO_GROUPM p{};
	p.PacketType = HEADER_ZC_NOTIFY_POSITION_TO_GROUPM;
	p.AID = sd->status.account_id;
	p.xPos = -1;
	p.yPos = -1;

	clif_send(&p, sizeof(p), &sd->bl, PARTY_SAMEMAP_WOS);
}


/// Displays a skill message (thanks to Rayce) (ZC_SKILLMSG).
/// 0215 <msg id>.L
/// msg id:
///     0x15 = End all negative status (PA_GOSPEL)
///     0x16 = Immunity to all status (PA_GOSPEL)
///     0x17 = MaxHP +100% (PA_GOSPEL)
///     0x18 = MaxSP +100% (PA_GOSPEL)
///     0x19 = All stats +20 (PA_GOSPEL)
///     0x1c = Enchant weapon with Holy element (PA_GOSPEL)
///     0x1d = Enchant armor with Holy element (PA_GOSPEL)
///     0x1e = DEF +25% (PA_GOSPEL)
///     0x1f = ATK +100% (PA_GOSPEL)
///     0x20 = HIT/Flee +50 (PA_GOSPEL)
///     0x28 = Full strip failed because of coating (ST_FULLSTRIP)
///     ? = nothing
void clif_gospel_info(map_session_data *sd, int type)
{
	int fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x215));
	WFIFOW(fd,0)=0x215;
	WFIFOL(fd,2)=type;
	WFIFOSET(fd, packet_len(0x215));

}


/// Multi-purpose mission information packet (ZC_STARSKILL).
/// 020e <mapname>.24B <monster_id>.L <star>.B <result>.B
/// result:
///      0 = Star Gladiator %s has designed <mapname>'s as the %s.
///      star:
///          0 = Place of the Sun
///          1 = Place of the Moon
///          2 = Place of the Stars
///      1 = Star Gladiator %s's %s: <mapname>
///      star:
///          0 = Place of the Sun
///          1 = Place of the Moon
///          2 = Place of the Stars
///      10 = Star Gladiator %s has designed <mapname>'s as the %s.
///      star:
///          0 = Target of the Sun
///          1 = Target of the Moon
///          2 = Target of the Stars
///      11 = Star Gladiator %s's %s: <mapname used as monster name>
///      star:
///          0 = Monster of the Sun
///          1 = Monster of the Moon
///          2 = Monster of the Stars
///      20 = [TaeKwon Mission] Target Monster : <mapname used as monster name> (<star>%)
///      21 = [Taming Mission] Target Monster : <mapname used as monster name>
///      22 = [Collector Rank] Target Item : <monster_id used as item id>
///      30 = [Sun, Moon and Stars Angel] Designed places and monsters have been reset.
///      40 = Target HP : <monster_id used as HP>
void clif_starskill(map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x20e));
	WFIFOW(fd,0) = 0x20e;
	safestrncpy(WFIFOCP(fd,2), mapname, NAME_LENGTH);
	WFIFOL(fd,26) = monster_id;
	WFIFOB(fd,30) = star;
	WFIFOB(fd,31) = result;
	WFIFOSET(fd,packet_len(0x20e));
}

/*==========================================
 * Info about Star Glaldiator save map [Komurka]
 * type: 1: Information, 0: Map registered
 *------------------------------------------*/
void clif_feel_info(map_session_data* sd, unsigned char feel_level, unsigned char type)
{
	char mapname[MAP_NAME_LENGTH_EXT];

	mapindex_getmapname_ext(mapindex_id2name(sd->feel_map[feel_level].index), mapname);
	clif_starskill(sd, mapname, 0, feel_level, type ? 1 : 0);
}

/*==========================================
 * Info about Star Glaldiator hate mob [Komurka]
 * type: 1: Register mob, 0: Information.
 *------------------------------------------*/
void clif_hate_info(map_session_data *sd, unsigned char hate_level,int class_, unsigned char type)
{
	if( pcdb_checkid(class_) ) {
		clif_starskill(sd, job_name(class_), class_, hate_level, type ? 10 : 11);
	} else if( mobdb_checkid(class_) ) {
		clif_starskill(sd, mob_db.find(class_)->jname.c_str(), class_, hate_level, type ? 10 : 11);
	} else {
		ShowWarning("clif_hate_info: Received invalid class %d for this packet (char_id=%d, hate_level=%u, type=%u).\n", class_, sd->status.char_id, (unsigned int)hate_level, (unsigned int)type);
	}
}

/*==========================================
 * Info about TaeKwon Do TK_MISSION mob [Skotlex]
 *------------------------------------------*/
void clif_mission_info(map_session_data *sd, int mob_id, unsigned char progress)
{
	clif_starskill(sd, mob_db.find(mob_id)->jname.c_str(), mob_id, progress, 20);
}

/*==========================================
 * Feel/Hate reset (thanks to Rayce) [Skotlex]
 *------------------------------------------*/
void clif_feel_hate_reset(map_session_data *sd)
{
	clif_starskill(sd, "", 0, 0, 30);
}


/// Send out reply to configuration change
/// 02d9 <type>.L <value>.L (ZC_CONFIG)
/// type: see enum e_config_type
/// value:
///		false = disabled
///		true = enabled
void clif_configuration( map_session_data* sd, enum e_config_type type, bool enabled ){
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x2d9));
	WFIFOW(fd, 0) = 0x2d9;
	WFIFOL(fd, 2) = type;
	WFIFOL(fd, 6) = enabled;
	WFIFOSET(fd, packet_len(0x2d9));
}


/// The player's 'view equip' state, sent during login (ZC_CONFIG_NOTIFY).
/// 02da <open equip window>.B
/// open equip window:
///     0 = disabled
///     1 = enabled
void clif_equipcheckbox(map_session_data* sd)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x2da));
	WFIFOW(fd, 0) = 0x2da;
	WFIFOB(fd, 2) = (sd->status.show_equip ? 1 : 0);
	WFIFOSET(fd, packet_len(0x2da));
}


/// Sends info about a player's equipped items.
/// 02d7 <packet len>.W <name>.24B <class>.W <hairstyle>.W <up-viewid>.W <mid-viewid>.W <low-viewid>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.26B* (ZC_EQUIPWIN_MICROSCOPE)
/// 02d7 <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.28B* (ZC_EQUIPWIN_MICROSCOPE, PACKETVER >= 20100629)
/// 0859 <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.28B* (ZC_EQUIPWIN_MICROSCOPE2, PACKETVER >= 20101124)
/// 0859 <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <robe>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.28B* (ZC_EQUIPWIN_MICROSCOPE2, PACKETVER >= 20110111)
/// 0997 <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <robe>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.31B* (ZC_EQUIPWIN_MICROSCOPE_V5, PACKETVER >= 20120925)
/// 0a2d <packet len>.W <name>.24B <class>.W <hairstyle>.W <bottom-viewid>.W <mid-viewid>.W <up-viewid>.W <robe>.W <haircolor>.W <cloth-dye>.W <gender>.B {equip item}.57B* (ZC_EQUIPWIN_MICROSCOPE_V6, PACKETVER >= 20150225)
void clif_viewequip_ack( map_session_data& sd, map_session_data& tsd ){
#if PACKETVER_AD_NUM >= 20071211 || PACKETVER_SAK_NUM >= 20071127 || PACKETVER_MAIN_NUM >= 20071211 || defined(PACKETVER_RE)
	PACKET_ZC_EQUIPWIN_MICROSCOPE* p = reinterpret_cast<PACKET_ZC_EQUIPWIN_MICROSCOPE*>( packet_buffer );

	p->PacketType = HEADER_ZC_EQUIPWIN_MICROSCOPE;
	p->PacketLength = sizeof( *p );

	safestrncpy( p->characterName, tsd.status.name, NAME_LENGTH );

	p->job = tsd.status.class_;
	p->head = tsd.vd.hair_style;
	p->accessory = tsd.vd.head_bottom;
	p->accessory2 = tsd.vd.head_mid;
	p->accessory3 = tsd.vd.head_top;
#if PACKETVER >= 20110111
	p->robe = tsd.vd.robe;
#endif
	p->headpalette = tsd.vd.hair_color;
	p->bodypalette = tsd.vd.cloth_color;
#if PACKETVER_MAIN_NUM >= 20180801 || PACKETVER_RE_NUM >= 20180801 || PACKETVER_ZERO_NUM >= 20180808
	p->body2 = tsd.vd.body_style;
#endif
	p->sex = tsd.vd.sex;

	for( int i = 0, equip = 0; i < EQI_MAX; i++ ){
		int k = tsd.equip_index[i];

		if( k >= 0 ){
			if( tsd.inventory.u.items_inventory[k].nameid == 0 || tsd.inventory_data[k] == nullptr ){ // Item doesn't exist
				continue;
			}

			if( !tsd.inventory.u.items_inventory[k].equip ){
				continue;
			}

			if( !itemdb_isequip2( tsd.inventory_data[k] ) ){ // Is not equippable
				continue;
			}

			clif_item_equip( client_index( k ), &p->list[equip], &tsd.inventory.u.items_inventory[k], tsd.inventory_data[k], pc_equippoint( &tsd, k ) );

			p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof( p->list[0] ) );
			equip++;
		}
	}

	clif_send( p, p->PacketLength, &sd.bl, SELF );
#endif
}


/// Display msgstringtable.txt string (ZC_MSG).
/// 0291 <message>.W
void clif_msg(map_session_data* sd, unsigned short id)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x291));
	WFIFOW(fd, 0) = 0x291;
	WFIFOW(fd, 2) = id;  // zero-based msgstringtable.txt index
	WFIFOSET(fd, packet_len(0x291));
}


/// Display msgstringtable.txt string and fill in a valid for %d format (ZC_MSG_VALUE).
/// 0x7e2 <message>.W <value>.L
void clif_msg_value(map_session_data* sd, unsigned short id, int value)
{
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x7e2));
	WFIFOW(fd,0) = 0x7e2;
	WFIFOW(fd,2) = id;
	WFIFOL(fd,4) = value;
	WFIFOSET(fd, packet_len(0x7e2));
}


/// Displays msgstringtable.txt string, prefixed with a skill name. (ZC_MSG_SKILL).
/// 07e6 <skill id>.W <msg id>.L
///
/// NOTE: Message has following format and is printed in color 0xCDCDFF (purple):
///       "[SkillName] Message"
void clif_msg_skill(map_session_data* sd, uint16 skill_id, int msg_id)
{
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x7e6));
	WFIFOW(fd,0) = 0x7e6;
	WFIFOW(fd,2) = skill_id;
	WFIFOL(fd,4) = msg_id;
	WFIFOSET(fd, packet_len(0x7e6));
}

/// Displays msgstringtable.txt string in a color. (ZC_MSG_COLOR).
/// 09cd <msg id>.W <color>.L
void clif_msg_color( map_session_data *sd, uint16 msg_id, uint32 color ){
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x9cd));
	WFIFOW(fd, 0) = 0x9cd;
	WFIFOW(fd, 2) = msg_id;
	WFIFOL(fd, 4) = color;

	WFIFOSET(fd, packet_len(0x9cd));
}

/// Validates one global/guild/party/whisper message packet and tries to recognize its components.
/// Returns true if the packet was parsed successfully.
/// Formats: false - <packet id>.w <packet len>.w (<name> : <message>).?B 00
///          true - <packet id>.w <packet len>.w <name>.24B <message>.?B 00
static bool clif_process_message(map_session_data* sd, bool whisperFormat, char* out_name, char* out_message, char* out_full_message ){
	const char* seperator = " : ";
	int fd;
	struct s_packet_db* info;
	uint16 packetLength, inputLength;
	const char *input, *name, *message;
	size_t nameLength, messageLength;

	fd = sd->fd;

	info = &packet_db[RFIFOW(fd,0)];

	packetLength = RFIFOW(fd,info->pos[0]);
	input = RFIFOCP(fd,info->pos[1]);

	// basic structure check for the 4-byte header
	if( packetLength < 4 ){
		ShowWarning("clif_process_message: Received malformed packet from player '%s' (no message data)!\n", sd->status.name);
		return false;
	}else{
		inputLength = packetLength - 4;
	}

	// process <name> part of the packet
	if( whisperFormat ) {	
		// name has fixed width
		if( inputLength < NAME_LENGTH + 1 ) {
			ShowWarning("clif_process_message: Received malformed packet from player '%s' (packet length is incorrect)!\n", sd->status.name);
			return false;
		}

		name = input;

		// validate name
		nameLength = strnlen( name, NAME_LENGTH - 1 );

		// only restriction is that the name must be zero-terminated
		if( name[nameLength] != '\0' ) {
			ShowWarning("clif_process_message: Player '%s' sent an unterminated name!\n", sd->status.name);
			return false;
		}

		message = input + NAME_LENGTH;
		messageLength = inputLength - NAME_LENGTH;		
	}else{
		// name and message are separated by ' : '
		size_t seperatorLength = strnlen( seperator, NAME_LENGTH );

		nameLength = strnlen( sd->status.name, NAME_LENGTH - 1 ); // name length (w/o zero byte)
		
		// check if there's enough data provided
		if( inputLength < nameLength + seperatorLength + 1 ){
			ShowWarning("clif_process_message: Received malformed packet from player '%s' (no username data)!\n", sd->status.name);
			return false;
		}

		name = input;

		// validate name
		if( strncmp( name, sd->status.name, nameLength ) || // the text must start with the speaker's name
			strncmp( name + nameLength, seperator, seperatorLength ) ) // followed by the seperator
		{
			//Hacked message, or infamous "client desynch" issue where they pick one char while loading another.
			ShowWarning("clif_process_message: Player '%s' sent a message using an incorrect name! Forcing a relog...\n", sd->status.name);
			set_eof(sd->fd); // Just kick them out to correct it.
			return false;
		}

		message = input + nameLength + seperatorLength;
		messageLength = inputLength - nameLength - seperatorLength;
	}

#if PACKETVER < 20151001
	// the declared length must match real length
	if( messageLength != strnlen(message, messageLength)+1 ) {
		ShowWarning("clif_process_message: Received malformed packet from player '%s' (length is incorrect)!\n", sd->status.name);
		return false;
	}

	// verify <message> part of the packet
	if( message[messageLength-1] != '\0' ) {	// message must be zero-terminated
		ShowWarning("clif_process_message: Player '%s' sent an unterminated message string!\n", sd->status.name);
		return false;
	}
#else
	// No zero termination anymore
	messageLength += 1;
#endif

	// messages mustn't be too long
	if( messageLength > CHAT_SIZE_MAX-1 ) {
		// Normally you can only enter CHATBOX_SIZE-1 letters into the chat box, but Frost Joke / Dazzler's text can be longer.
		// Also, the physical size of strings that use multibyte encoding can go multiple times over the chatbox capacity.
		// Neither the official client nor server place any restriction on the length of the data in the packet,
		// but we'll only allow reasonably long strings here. This also makes sure that they fit into the `chatlog` table.
		ShowWarning("clif_process_message: Player '%s' sent a message too long ('%.*s')!\n", sd->status.name, CHAT_SIZE_MAX-1, message);
		return false;
	}
	
	// If it is not a whisper message, set the name to the fakename of the player
	if( whisperFormat == false && sd->fakename[0] != '\0' ){
		strcpy( out_name, sd->fakename );
	}else{
		safestrncpy( out_name, name, nameLength + 1 );
	}
	safestrncpy( out_message, message, messageLength );

	if( whisperFormat ){
		sprintf( out_full_message, "%-24s%s", out_name, out_message );
		out_full_message[nameLength] = '\0';
	}else{
		sprintf( out_full_message, "%s%s%s", out_name, seperator, out_message );
	}

	if( is_atcommand( fd, sd, out_message, 1 )  )
		return false;

	if (sd->sc.cant.chat || (sd->state.block_action & PCBLOCK_CHAT))
		return false; //no "chatting" while muted.

	if( battle_config.min_chat_delay ) { //[Skotlex]
		if (DIFF_TICK(sd->cantalk_tick, gettick()) > 0)
			return false;
		sd->cantalk_tick = gettick() + battle_config.min_chat_delay;
	}

	if (battle_config.idletime_option&IDLE_CHAT)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_CHAT)
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_CHAT)
		sd->idletime_mer = last_tick;

	//achievement_update_objective(sd, AG_CHATTING, 1, 1); // !TODO: Confirm how this achievement is triggered

	return true;
}

/**
 * Displays a message if the player enters a PK Zone (during pk_mode)
 * @param sd: Player data
 */
inline void clif_pk_mode_message(map_session_data * sd)
{
	if (battle_config.pk_mode && battle_config.pk_mode_mes &&
		sd && map_getmapflag(sd->bl.m, MF_PVP)) {
		if( (int)sd->status.base_level < battle_config.pk_min_level ) {
			char output[CHAT_SIZE_MAX];
			// 1504: You've entered a PK Zone (safe until level %d).
			safesnprintf(output, CHAT_SIZE_MAX, msg_txt(sd,1504), 
						 battle_config.pk_min_level);
			clif_showscript(&sd->bl, output, SELF);
		} else {
			// 1503: You've entered a PK Zone.
			clif_showscript(&sd->bl, msg_txt(sd,1503), SELF);
		}
	}
	return;
}

// ---------------------
// clif_parse_wanttoconnect
// ---------------------
// Parses a WantToConnection packet to try to identify which is the packet version used. [Skotlex]
// error codes:
// 0 - Success
// 1 - Unknown packet_ver
// 2 - Invalid account_id
// 3 - Invalid char_id
// 4 - Invalid login_id1 (reserved)
// 5 - Invalid client_tick (reserved)
// 6 - Invalid sex
// Only the first 'invalid' error that appears is used.
static int clif_parse_WantToConnection_sub(int fd)
{
	int value; //Value is used to temporarily store account/char_id/sex

	//By default, start searching on the default one.
	uint16 cmd = RFIFOW(fd, 0);
	int16 packet_len;

	packet_len = static_cast<decltype(packet_len)>( RFIFOREST( fd ) );

	// FIXME: If the packet is not received at once, this will FAIL.
	// Figure out, when it happens, that only part of the packet is
	// received, or fix the function to be able to deal with that
	// case.
	if( packet_len != packet_db[cmd].len )
		return 1; /* wrong length */
	else if( (value=(int)RFIFOL(fd, packet_db[cmd].pos[0])) < START_ACCOUNT_NUM || value > END_ACCOUNT_NUM )
		return 2; /* invalid account_id */
	else if( (value=(int)RFIFOL(fd, packet_db[cmd].pos[1])) <= 0 )
		return 3; /* invalid char_id */
	/*                   RFIFOL(fd, packet_db[cmd].pos[2]) - don't care about login_id1 */
	/*                   RFIFOL(fd, packet_db[cmd].pos[3]) - don't care about client_tick */
	else if( (value=(int)RFIFOB(fd, packet_db[cmd].pos[4])) != 0 && value != 1 )
		return 6; /* invalid sex */
	else
		return 0;
}

/// Request to connect to map-server.
/// 0072 <account id>.L <char id>.L <auth code>.L <client time>.L <gender>.B (CZ_ENTER)
/// 0436 <account id>.L <char id>.L <auth code>.L <client time>.L <gender>.B (CZ_ENTER2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_WantToConnection(int fd, map_session_data* sd)
{
	struct block_list* bl;
	struct auth_node* node;
	int cmd, account_id, char_id, login_id1, sex, err;
	t_tick client_tick; //The client tick is a tick, therefore it needs be unsigned. [Skotlex]

	if (sd) {
		ShowError("clif_parse_WantToConnection : invalid request (character already logged in)\n");
		return;
	}

	cmd = RFIFOW(fd, 0);
	account_id = RFIFOL(fd, packet_db[cmd].pos[0]);
	char_id = RFIFOL(fd, packet_db[cmd].pos[1]);
	login_id1 = RFIFOL(fd, packet_db[cmd].pos[2]);
	client_tick = RFIFOL(fd, packet_db[cmd].pos[3]);
	sex = RFIFOB(fd, packet_db[cmd].pos[4]);

	err = clif_parse_WantToConnection_sub(fd);

	if( err ){ // connection rejected
		ShowInfo("clif_parse: Disconnecting session #%d with unknown connect packet 0x%04x(length:%d)%s\n", fd, cmd, RFIFOREST(fd), (
				err == 1 ? "." :
				err == 2 ? ", possibly for having an invalid account_id." :
				err == 3 ? ", possibly for having an invalid char_id." :
				/* Uncomment when checks are added in clif_parse_WantToConnection_sub. [FlavioJS]
				err == 4 ? ", possibly for having an invalid login_id1." :
				err == 5 ? ", possibly for having an invalid client_tick." :
				*/
				err == 6 ? ", possibly for having an invalid sex." :
				". ERROR invalid error code"));
		
		WFIFOHEAD(fd,packet_len(0x6a));
		WFIFOW(fd,0) = 0x6a;
		WFIFOB(fd,2) = err;
		WFIFOSET(fd,packet_len(0x6a));

#ifdef DUMP_INVALID_PACKET
		ShowDump(RFIFOP(fd, 0), RFIFOREST(fd));
#endif

		RFIFOSKIP(fd, RFIFOREST(fd));

		set_eof(fd);
		return;
	}

	if( !global_core->is_running() ){ // not allowed
		clif_authfail_fd(fd,1);// server closed
		return;
	}

	//Check for double login.
	bl = map_id2bl(account_id);
	if(bl && bl->type != BL_PC) {
		ShowError("clif_parse_WantToConnection: a non-player object already has id %d, please increase the starting account number\n", account_id);
		WFIFOHEAD(fd,packet_len(0x6a));
		WFIFOW(fd,0) = 0x6a;
		WFIFOB(fd,2) = 3; // Rejected by server
		WFIFOSET(fd,packet_len(0x6a));
		set_eof(fd);
		return;
	}

	if (bl ||
		((node=chrif_search(account_id)) && //An already existing node is valid only if it is for this login.
			!(node->account_id == account_id && node->char_id == char_id && node->state == ST_LOGIN)))
	{
		clif_authfail_fd(fd, 8); //Still recognizes last connection
		return;
	}

	CREATE(sd, TBL_PC, 1);
	new(sd) map_session_data();
	sd->fd = fd;
#ifdef PACKET_OBFUSCATION
	sd->cryptKey = (((((clif_cryptKey[0] * clif_cryptKey[1]) + clif_cryptKey[2]) & 0xFFFFFFFF) * clif_cryptKey[1]) + clif_cryptKey[2]) & 0xFFFFFFFF;
#endif
	session[fd]->session_data = sd;

	pc_setnewpc(sd, account_id, char_id, login_id1, client_tick, sex, fd);

#if PACKETVER < 20070521
	WFIFOHEAD(fd,4);
	WFIFOL(fd,0) = sd->bl.id;
	WFIFOSET(fd,4);
#else
	WFIFOHEAD(fd,packet_len(0x283));
	WFIFOW(fd,0) = 0x283;
	WFIFOL(fd,2) = sd->bl.id;
	WFIFOSET(fd,packet_len(0x283));
#endif

	chrif_authreq(sd,false);
}


/// Notification from the client, that it has finished map loading and is about to display player's character (CZ_NOTIFY_ACTORINIT).
/// 007d
void clif_parse_LoadEndAck(int fd,map_session_data *sd)
{
	bool guild_notice = false;

	if(sd->bl.prev != nullptr)
		return;

	// Autotraders should ignore this entirely, clif_parse_LoadEndAck is always invoked manually for them
	if (!sd->state.active || (!sd->state.autotrade && !sd->state.pc_loaded)) { //Character loading is not complete yet!
		//Let pc_reg_received or pc_scdata_received reinvoke this when ready.
		sd->state.connect_new = 0;
		return;
	}

	if (sd->state.rewarp) { //Rewarp player.
		sd->state.rewarp = 0;
		clif_changemap( *sd, sd->bl.m, sd->bl.x, sd->bl.y );
		return;
	}

	sd->state.warping = 0;

	// look
#if PACKETVER < 4
	clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
	clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
#else
	clif_changelook(&sd->bl,LOOK_WEAPON,0);
#endif
	pc_set_costume_view(sd);

	if(sd->vd.cloth_color)
		clif_refreshlook(&sd->bl,sd->bl.id,LOOK_CLOTHES_COLOR,sd->vd.cloth_color,SELF);
	if(sd->vd.body_style)
		clif_refreshlook(&sd->bl,sd->bl.id,LOOK_BODY2,sd->vd.body_style,SELF);

	// item
	clif_inventorylist(sd);  // inventory list first, otherwise deleted items in pc_checkitem show up as 'unknown item'
	pc_checkitem(sd);
	clif_equipswitch_list(sd);

	// cart
	if(pc_iscarton(sd)) {
		clif_cartlist(sd);
		clif_updatestatus(*sd,SP_CARTINFO);
	}

	// weight
	clif_updatestatus(*sd,SP_WEIGHT);
	clif_updatestatus(*sd,SP_MAXWEIGHT);

	// guild
	// (needs to go before clif_spawn() to show guild emblems correctly)
	if(sd->status.guild_id)
		guild_send_memberinfoshort(sd,1);

	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	if(battle_config.pc_invincible_time > 0) {
		if(mapdata_flag_gvg(mapdata))
			pc_setinvincibletimer(sd,battle_config.pc_invincible_time<<1);
		else
			pc_setinvincibletimer(sd,battle_config.pc_invincible_time);
	}

	if( mapdata->users++ == 0 && battle_config.dynamic_mobs )
		map_spawnmobs(sd->bl.m);
	if( !pc_isinvisible(sd) ) { // increment the number of pvp players on the map
		mapdata->users_pvp++;
	}
	sd->state.debug_remove_map = 0; // temporary state to track double remove_map's [FlavioJS]

	// reset the callshop flag if the player changes map
	sd->state.callshop = 0;

	if(map_addblock(&sd->bl))
		return;
	clif_spawn(&sd->bl);

	// Party
	// (needs to go after clif_spawn() to show hp bars correctly)
	if(sd->status.party_id) {
		party_send_movemap(sd);
		clif_party_hp( *sd ); // Show hp after displacement [LuzZza]
	}

	if( sd->bg_id ) clif_bg_hp(sd); // BattleGround System

	if(!pc_isinvisible(sd) && mapdata->getMapFlag(MF_PVP)) {
		if(!battle_config.pk_mode) { // remove pvp stuff for pk_mode [Valaris]
			if (!mapdata->getMapFlag(MF_PVP_NOCALCRANK))
				sd->pvp_timer = add_timer(gettick()+200, pc_calc_pvprank_timer, sd->bl.id, 0);
			sd->pvp_rank = 0;
			sd->pvp_lastusers = 0;
			sd->pvp_point = 5;
			sd->pvp_won = 0;
			sd->pvp_lost = 0;
		}
		clif_map_property(&sd->bl, MAPPROPERTY_FREEPVPZONE, SELF);
	} else if(sd->duel_group) // set flag, if it's a duel [LuzZza]
		clif_map_property(&sd->bl, MAPPROPERTY_FREEPVPZONE, SELF);
	else if (mapdata->getMapFlag(MF_GVG_DUNGEON))
		clif_map_property(&sd->bl, MAPPROPERTY_FREEPVPZONE, SELF); //TODO: Figure out the real packet to send here.
	else if( mapdata_flag_gvg(mapdata) )
		clif_map_property(&sd->bl, MAPPROPERTY_AGITZONE, SELF);
	else
		clif_map_property(&sd->bl, MAPPROPERTY_NOTHING, SELF);

	// info about nearby objects
	// must use foreachinarea (CIRCULAR_AREA interferes with foreachinrange)
	map_foreachinallarea(clif_getareachar, sd->bl.m, sd->bl.x-AREA_SIZE, sd->bl.y-AREA_SIZE, sd->bl.x+AREA_SIZE, sd->bl.y+AREA_SIZE, BL_ALL, sd);

	// pet
	if( sd->pd ) {
		if( battle_config.pet_no_gvg && mapdata_flag_gvg(mapdata) ) { //Return the pet to egg. [Skotlex]
			clif_displaymessage(sd->fd, msg_txt(sd,666));
			pet_return_egg( sd, sd->pd );
		} else {
			if(map_addblock(&sd->pd->bl))
				return;
			clif_spawn(&sd->pd->bl);
			clif_send_petdata(sd,sd->pd,0,0);
			clif_send_petstatus(sd);
//			skill_unit_move(&sd->pd->bl,gettick(),1);
		}
	}

	//homunculus [blackhole89]
	if( hom_is_active(sd->hd) ) {
		if(map_addblock(&sd->hd->bl))
			return;
		clif_spawn(&sd->hd->bl);
		clif_send_homdata( *sd->hd, SP_ACK );
		clif_hominfo(sd,sd->hd,1);
		clif_hominfo(sd,sd->hd,0); //for some reason, at least older clients want this sent twice
		clif_homskillinfoblock( *sd->hd );
		status_calc_bl(&sd->hd->bl, { SCB_SPEED });
		if( !(battle_config.hom_setting&HOMSET_NO_INSTANT_LAND_SKILL) )
			skill_unit_move(&sd->hd->bl,gettick(),1); // apply land skills immediately
	}

	if( sd->md ) {
		if(map_addblock(&sd->md->bl))
			return;
		clif_spawn(&sd->md->bl);
		clif_mercenary_info(sd);
		clif_mercenary_skillblock(sd);
		status_calc_bl(&sd->md->bl, { SCB_SPEED }); // Mercenary mimic their master's speed on each map change
	}

	if( sd->ed ) {
		if (map_addblock(&sd->ed->bl))
			return;
		clif_spawn(&sd->ed->bl);
		clif_elemental_info(sd);
		clif_elemental_updatestatus(*sd, SP_HP);
		clif_hpmeter_single( *sd, sd->ed->bl.id, sd->ed->battle_status.hp, sd->ed->battle_status.max_hp );
		clif_elemental_updatestatus(*sd, SP_SP);
		status_calc_bl(&sd->ed->bl, { SCB_SPEED }); //Elemental mimic their master's speed on each map change
	}

	if(sd->state.connect_new) {
		int lv;
		guild_notice = true;
		clif_skillinfoblock(sd);
		clif_hotkeys_send(sd,0);
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
		clif_hotkeys_send(sd,1);
#endif
		clif_updatestatus(*sd,SP_BASEEXP);
		clif_updatestatus(*sd,SP_NEXTBASEEXP);
		clif_updatestatus(*sd,SP_JOBEXP);
		clif_updatestatus(*sd,SP_NEXTJOBEXP);
		clif_updatestatus(*sd,SP_SKILLPOINT);
		clif_initialstatus( *sd );

		if (sd->sc.option&OPTION_FALCON)
			clif_status_load(&sd->bl, EFST_FALCON, 1);
		else if (sd->sc.option&(OPTION_RIDING|OPTION_DRAGON))
			clif_status_load(&sd->bl, EFST_RIDING, 1);
		else if (sd->sc.option&OPTION_WUGRIDER)
			clif_status_load(&sd->bl, EFST_WUGRIDER, 1);
		else if (sd->sc.getSCE(SC_ALL_RIDING))
			clif_status_load(&sd->bl, EFST_ALL_RIDING, 1);

		if(sd->status.manner < 0)
			sc_start(&sd->bl,&sd->bl,SC_NOCHAT,100,0,0);

		//Auron reported that This skill only triggers when you logon on the map o.O [Skotlex]
		if ((lv = pc_checkskill(sd,SG_KNOWLEDGE)) > 0) {
			if(sd->bl.m == sd->feel_map[0].m
				|| sd->bl.m == sd->feel_map[1].m
				|| sd->bl.m == sd->feel_map[2].m)
				sc_start(&sd->bl,&sd->bl, SC_KNOWLEDGE, 100, lv, skill_get_time(SG_KNOWLEDGE, lv));
		}

		if(sd->pd && sd->pd->pet.intimate > 900)
			clif_pet_emotion(sd->pd,(sd->pd->pet.class_ - 100)*100 + 50 + pet_hungry_val(sd->pd));

		if(hom_is_active(sd->hd))
			hom_init_timers(sd->hd);

		if (night_flag && mapdata->getMapFlag(MF_NIGHTENABLED)) {
			sd->state.night = 1;
			clif_status_load(&sd->bl, EFST_SKE, 1);
		}

		// Notify everyone that this char logged in [Skotlex].
		map_foreachpc(clif_friendslist_toggle_sub, sd->status.account_id, sd->status.char_id, 1);

		if (!sd->state.autotrade) { // Don't trigger NPC event or opening vending/buyingstore will be failed
			npc_script_event( *sd, NPCE_LOGIN );
		}

		// Set facing direction before check below to update client
		if (battle_config.spawn_direction)
			unit_setdir(&sd->bl, sd->status.body_direction, false);
	} else {
		//For some reason the client "loses" these on warp/map-change.
		clif_updatestatus(*sd,SP_STR);
		clif_updatestatus(*sd,SP_AGI);
		clif_updatestatus(*sd,SP_VIT);
		clif_updatestatus(*sd,SP_INT);
		clif_updatestatus(*sd,SP_DEX);
		clif_updatestatus(*sd,SP_LUK);
#ifdef RENEWAL
		clif_updatestatus(*sd,SP_POW);
		clif_updatestatus(*sd,SP_STA);
		clif_updatestatus(*sd,SP_WIS);
		clif_updatestatus(*sd,SP_SPL);
		clif_updatestatus(*sd,SP_CON);
		clif_updatestatus(*sd,SP_CRT);
#endif

		// abort currently running script
		sd->state.using_fake_npc = 0;
		sd->state.menu_or_input = 0;
		sd->npc_menu = 0;

		if(sd->npc_id)
			npc_event_dequeue(sd);
	}

	if( sd->state.changemap ) {// restore information that gets lost on map-change
		clif_partyinvitationstate( *sd );
#if PACKETVER >= 20070918
		clif_equipcheckbox(sd);
#endif
		clif_pet_autofeed_status(sd,false);
		clif_configuration( sd, CONFIG_CALL, sd->status.disable_call );
#if PACKETVER >= 20170920
		if( battle_config.homunculus_autofeed_always ){
			// Always send ON or OFF
			if( sd->hd && battle_config.feature_homunculus_autofeed ){
				clif_configuration( sd, CONFIG_HOMUNCULUS_AUTOFEED, sd->hd->homunculus.autofeed );
			}else{
				clif_configuration( sd, CONFIG_HOMUNCULUS_AUTOFEED, false );
			}
		}else{
			// Only send when enabled
			if( sd->hd && battle_config.feature_homunculus_autofeed && sd->hd->homunculus.autofeed ){
				clif_configuration( sd, CONFIG_HOMUNCULUS_AUTOFEED, true );
			}
		}
#endif
		clif_reputation_list( *sd );

		if (sd->guild && battle_config.guild_notice_changemap == 1){
			// Displays after VIP
			clif_guild_notice( *sd );
			guild_notice = false; // Do not display it twice
		}

		if (battle_config.bg_flee_penalty != 100 || battle_config.gvg_flee_penalty != 100) {
			struct map_data *pmap = map_getmapdata(sd->state.pmap);

			if ((pmap != nullptr && (mapdata_flag_gvg(pmap) || pmap->getMapFlag(MF_BATTLEGROUND))) || (mapdata != nullptr && (mapdata_flag_gvg(mapdata) || mapdata->getMapFlag(MF_BATTLEGROUND))))
				status_calc_bl(&sd->bl, { SCB_FLEE }); //Refresh flee penalty
		}

		if( night_flag && mapdata->getMapFlag(MF_NIGHTENABLED) )
		{	//Display night.
			if( !sd->state.night )
			{
				sd->state.night = 1;
				clif_status_load(&sd->bl, EFST_SKE, 1);
			}
		}
		else if( sd->state.night )
		{ //Clear night display.
			sd->state.night = 0;
			clif_status_load(&sd->bl, EFST_SKE, 0);
		}

		if( mapdata->getMapFlag(MF_BATTLEGROUND) )
		{
			clif_map_type( *sd, MAPTYPE_BATTLEFIELD ); // Battleground Mode
			if( map_getmapflag(sd->bl.m, MF_BATTLEGROUND) == 2 )
				clif_bg_updatescore_single(sd);
		}

		if( mapdata->getMapFlag(MF_ALLOWKS) && !mapdata_flag_ks(mapdata) )
		{
			char output[128];
			sprintf(output, "[ Kill Steal Protection Disable. KS is allowed in this map ]");
			clif_broadcast(&sd->bl, output, strlen(output) + 1, BC_BLUE, SELF);
		}

		if( pc_has_permission(sd,PC_PERM_VIEW_HPMETER) ) {
			mapdata->hpmeter_visible++;
			sd->state.hpmeter_visible = 1;
		}

		status_change_clear_onChangeMap(&sd->bl, &sd->sc);
		map_iwall_get(sd); // Updates Walls Info on this Map to Client
		status_calc_pc(sd, sd->state.autotrade ? SCO_FIRST : SCO_NONE); // Some conditions are map-dependent so we must recalculate

#ifdef VIP_ENABLE
		if (!sd->state.connect_new &&
			!sd->vip.disableshowrate &&
			sd->state.pmap != sd->bl.m &&
			map_getmapflag(sd->state.pmap, MF_BEXP) != mapdata->getMapFlag(MF_BEXP)
			)
		{
			clif_display_pinfo( *sd );
		}
#endif

		// Instances do not need their own channels
		if( channel_config.map_tmpl.name[0] && (channel_config.map_tmpl.opt&CHAN_OPT_AUTOJOIN) && !mapdata->instance_id && !mapdata->getMapFlag(MF_NOMAPCHANNELAUTOJOIN) )
			channel_mjoin(sd); //join new map

		clif_pk_mode_message(sd);
		
		// Update the client
		clif_goldpc_info( *sd );
	}
	
	if( sd->guild && ( battle_config.guild_notice_changemap == 2 || guild_notice ) ){
		// Displays at end
		clif_guild_notice( *sd );
	}

	mail_clear(sd);

	/* Guild Aura Init */
	if( sd->guild && sd->state.gmaster_flag ) {
		guild_guildaura_refresh(sd,GD_LEADERSHIP,guild_checkskill(sd->guild->guild,GD_LEADERSHIP));
		guild_guildaura_refresh(sd,GD_GLORYWOUNDS,guild_checkskill(sd->guild->guild,GD_GLORYWOUNDS));
		guild_guildaura_refresh(sd,GD_SOULCOLD,guild_checkskill(sd->guild->guild,GD_SOULCOLD));
		guild_guildaura_refresh(sd,GD_HAWKEYES,guild_checkskill(sd->guild->guild,GD_HAWKEYES));
	}

	if( sd->state.vending ) { /* show we have a vending */
		clif_openvending( *sd );
		clif_showvendingboard( *sd );
	}

	// Don't trigger NPC event or opening vending/buyingstore will be failed
	if(!sd->state.autotrade && mapdata->getMapFlag(MF_LOADEVENT)) // Lance
		npc_script_event( *sd, NPCE_LOADMAP );

	if (pc_checkskill(sd, SG_DEVIL) && ((sd->class_&MAPID_THIRDMASK) == MAPID_STAR_EMPEROR || pc_is_maxjoblv(sd)))
		clif_status_load(&sd->bl, EFST_DEVIL1, 1);  //blindness [Komurka]

	if (sd->sc.opt2) //Client loses these on warp.
		clif_changeoption(&sd->bl);

	if ((sd->sc.getSCE(SC_MONSTER_TRANSFORM) || sd->sc.getSCE(SC_ACTIVE_MONSTER_TRANSFORM)) && battle_config.mon_trans_disable_in_gvg && mapdata_flag_gvg2(mapdata)) {
		status_change_end(&sd->bl, SC_MONSTER_TRANSFORM);
		status_change_end(&sd->bl, SC_ACTIVE_MONSTER_TRANSFORM);
		clif_displaymessage(sd->fd, msg_txt(sd,731)); // Transforming into monster is not allowed in Guild Wars.
	}

	clif_weather_check(sd);

	// For automatic triggering of NPCs after map loading (so you don't need to walk 1 step first)
	if (map_getcell(sd->bl.m,sd->bl.x,sd->bl.y,CELL_CHKNPC))
		npc_touch_area_allnpc(sd,sd->bl.m,sd->bl.x,sd->bl.y);
	else
		sd->areanpc.clear();

	/* it broke at some point (e.g. during a crash), so we make it visibly dead again. */
	if( !sd->status.hp && !pc_isdead(sd) && status_isdead(&sd->bl) )
		pc_setdead(sd);

	// If player is dead, and is spawned (such as @refresh) send death packet. [Valaris]
	if(pc_isdead(sd))
		clif_clearunit_area( sd->bl, CLR_DEAD );
	else {
		skill_usave_trigger(sd);
		if (battle_config.spawn_direction)
			clif_changed_dir(&sd->bl, SELF);
	}

	// Trigger skill effects if you appear standing on them
	if(!battle_config.pc_invincible_time)
		skill_unit_move(&sd->bl,gettick(),1);

	pc_show_questinfo_reinit(sd);
	pc_show_questinfo(sd);

#if PACKETVER >= 20150513
	if( sd->mail.inbox.unread ){
		clif_Mail_new(sd, 0, nullptr, nullptr);
	}
#endif

	sd->state.connect_new = 0;
	sd->state.changemap = false;
}


/// Server's tick (ZC_NOTIFY_TIME).
/// 007f <time>.L
void clif_notify_time(map_session_data* sd, t_tick time)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x7f));
	WFIFOW(fd,0) = 0x7f;
	WFIFOL(fd,2) = client_tick(time);
	WFIFOSET(fd,packet_len(0x7f));
}


/// Request for server's tick.
/// 007e <client tick>.L (CZ_REQUEST_TIME)
/// 0360 <client tick>.L (CZ_REQUEST_TIME2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_TickSend(int fd, map_session_data *sd)
{
	sd->client_tick = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	clif_notify_time(sd, gettick());
}


/// Sends hotkey bar.
/// 02b9 { <is skill>.B <id>.L <count>.W }*27 (ZC_SHORTCUT_KEY_LIST)
/// 07d9 { <is skill>.B <id>.L <count>.W }*36 (ZC_SHORTCUT_KEY_LIST_V2, PACKETVER >= 20090603)
/// 07d9 { <is skill>.B <id>.L <count>.W }*38 (ZC_SHORTCUT_KEY_LIST_V2, PACKETVER >= 20090617)
/// 0a00 <rotate>.B { <is skill>.B <id>.L <count>.W }*38 (ZC_SHORTCUT_KEY_LIST_V3, PACKETVER >= 20141022)
void clif_hotkeys_send( map_session_data *sd, int tab ){
#if defined(HOTKEY_SAVING) && ( PACKETVER_MAIN_NUM >= 20070711 || PACKETVER_RE_NUM >= 20080827 || PACKETVER_AD_NUM >= 20070711 || PACKETVER_SAK_NUM >= 20070628 )
	nullpo_retv( sd );

	PACKET_ZC_SHORTCUT_KEY_LIST p = {};

	p.packetType = HEADER_ZC_SHORTCUT_KEY_LIST;

#if PACKETVER_MAIN_NUM >= 20141022 || PACKETVER_RE_NUM >= 20141015 || defined(PACKETVER_ZERO)
	if( tab == 0 ){
		p.rotate = sd->status.hotkey_rowshift;
	}else{
		p.rotate = sd->status.hotkey_rowshift2;
	}

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	p.tab = tab;
#endif
#endif

	for( int i = 0, offset = tab * MAX_HOTKEYS; i < MAX_HOTKEYS_PACKET; i++ ){
		p.hotkey[i].isSkill = sd->status.hotkeys[i + offset].type;
		p.hotkey[i].id = sd->status.hotkeys[i + offset].id;
		p.hotkey[i].count = sd->status.hotkeys[i + offset].lv;
	}

	clif_send( &p, sizeof(PACKET_ZC_SHORTCUT_KEY_LIST), &sd->bl, SELF );
#endif
}

/// Request to update a position on the hotkey row bar
void clif_parse_HotkeyRowShift( int fd, map_session_data *sd ){
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	const PACKET_CZ_SHORTCUTKEYBAR_ROTATE2* p = reinterpret_cast<PACKET_CZ_SHORTCUTKEYBAR_ROTATE2*>( RFIFOP( fd, 0 ) );

	if( p->tab == 0 ){
		sd->status.hotkey_rowshift = p->rowshift;
	}else{
		sd->status.hotkey_rowshift2 = p->rowshift;
	}
#elif PACKETVER_MAIN_NUM >= 20140129 || PACKETVER_RE_NUM >= 20140129 || defined(PACKETVER_ZERO)
	const PACKET_CZ_SHORTCUTKEYBAR_ROTATE1* p = reinterpret_cast<PACKET_CZ_SHORTCUTKEYBAR_ROTATE1*>( RFIFOP( fd, 0 ) );

	sd->status.hotkey_rowshift = p->rowshift;
#endif
}

/// Request to update a position on the hotkey bar.
/// 02ba <index>.W <is skill>.B <id>.L <count>.W (CZ_SHORTCUT_KEY_CHANGE)
void clif_parse_Hotkey(int fd, map_session_data *sd) {
#ifdef HOTKEY_SAVING
#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	const PACKET_CZ_SHORTCUT_KEY_CHANGE2* p = reinterpret_cast<PACKET_CZ_SHORTCUT_KEY_CHANGE2*>( RFIFOP( fd, 0 ) );
	const unsigned short idx = p->index + p->tab * MAX_HOTKEYS;

	if( idx >= MAX_HOTKEYS_DB ){
		return;
	}

	sd->status.hotkeys[idx].type = p->hotkey.isSkill;
	sd->status.hotkeys[idx].id = p->hotkey.id;
	sd->status.hotkeys[idx].lv = p->hotkey.count;
#elif PACKETVER_MAIN_NUM >= 20070618 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO) || PACKETVER_AD_NUM >= 20070618 || PACKETVER_SAK_NUM >= 20070618
	const PACKET_CZ_SHORTCUT_KEY_CHANGE1* p = reinterpret_cast<PACKET_CZ_SHORTCUT_KEY_CHANGE1*>( RFIFOP( fd, 0 ) );
	const unsigned short idx = p->index;

	if( idx >= MAX_HOTKEYS_DB ){
		return;
	}

	sd->status.hotkeys[idx].type = p->hotkey.isSkill;
	sd->status.hotkeys[idx].id = p->hotkey.id;
	sd->status.hotkeys[idx].lv = p->hotkey.count;
#endif
#endif
}


/// Displays cast-like progress bar (ZC_PROGRESS).
/// 02f0 <color>.L <time>.L
void clif_progressbar(map_session_data * sd, unsigned long color, unsigned int second)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x2f0));
	WFIFOW(fd,0) = 0x2f0;
	WFIFOL(fd,2) = color;
	WFIFOL(fd,6) = second;
	WFIFOSET(fd,packet_len(0x2f0));
}


/// Removes an ongoing progress bar (ZC_PROGRESS_CANCEL).
/// 02f2
void clif_progressbar_abort(map_session_data * sd)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x2f2));
	WFIFOW(fd,0) = 0x2f2;
	WFIFOSET(fd,packet_len(0x2f2));
}


/// Notification from the client, that the progress bar has reached 100%.
/// 02f1 (CZ_PROGRESS)
void clif_parse_progressbar(int fd, map_session_data * sd){
	// No progressbar active, ignore it
	if( !sd->progressbar.npc_id ){
		return;
	}

	int npc_id = sd->progressbar.npc_id;
	bool closing = false;

	// Check if the progress was canceled
	if( gettick() < sd->progressbar.timeout && sd->st ){
		closing = true;
		sd->st->state = CLOSE; // will result in END in npc_scriptcont

		// If a message window was open, offer a close button to the user
		if( sd->st->mes_active ){
			clif_scriptclose( *sd, npc_id );
		}
	}

	sd->progressbar.npc_id = 0;
	sd->progressbar.timeout = 0;
	sd->state.workinprogress = WIP_DISABLE_NONE;

	if( battle_config.idletime_option&IDLE_NPC_PROGRESS ){
		sd->idletime = last_tick;
	}

	npc_scriptcont(sd, npc_id, closing);
}

/// Displays cast-like progress bar on a NPC
/// 09d1 <id>.L <color>.L <time>.L (ZC_PROGRESS_ACTOR)
void clif_progressbar_npc( struct npc_data *nd, map_session_data* sd ){
#if PACKETVER >= 20130821
	unsigned char buf[14];

	if( nd->progressbar.timeout > 0 ){
		WBUFW(buf,0) = 0x9d1;
		WBUFL(buf,2) = nd->bl.id;
		WBUFL(buf,6) = nd->progressbar.color;
		WBUFL(buf,10) = client_tick( ( nd->progressbar.timeout - gettick() ) / 1000 );

		if( sd ){
			clif_send(buf, packet_len(0x9d1), &sd->bl, SELF);
		}else{
			clif_send(buf, packet_len(0x9d1), &nd->bl, AREA);
		}
	}
#endif
}

/// Request to walk to a certain position on the current map.
/// 0085 <dest>.3B (CZ_REQUEST_MOVE)
/// 035f <dest>.3B (CZ_REQUEST_MOVE2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_WalkToXY(int fd, map_session_data *sd)
{
	short x, y;

	if (pc_isdead(sd)) {
		clif_clearunit_area( sd->bl, CLR_DEAD );
		return;
	}

	if (sd->sc.opt1 && ( sd->sc.opt1 == OPT1_STONEWAIT || sd->sc.opt1 == OPT1_BURNING ))
		; //You CAN walk on this OPT1 value.
	else if (sd->progressbar.npc_id) {
		clif_progressbar_abort(sd);
		return; // First walk attempt cancels the progress bar
	} else if (pc_cant_act(sd))
		return;

	if(sd->sc.getSCE(SC_RUN) || sd->sc.getSCE(SC_WUGDASH))
		return;

	RFIFOPOS(fd, packet_db[RFIFOW(fd,0)].pos[0], &x, &y, nullptr);

	//A move command one cell west is only valid if the target cell is free
	if(battle_config.official_cell_stack_limit > 0
		&& sd->bl.x == x+1 && sd->bl.y == y && map_count_oncell(sd->bl.m, x, y, BL_CHAR|BL_NPC, 1))
		return;

	// Cloaking wall check is actually updated when you click to process next movement
	// not when you move each cell.  This is official behaviour.
	if (sd->sc.getSCE(SC_CLOAKING))
		skill_check_cloaking(&sd->bl, sd->sc.getSCE(SC_CLOAKING));
	status_change_end(&sd->bl, SC_ROLLINGCUTTER); // If you move, you lose your counters. [malufett]
	status_change_end(&sd->bl, SC_CRESCIVEBOLT);

	pc_delinvincibletimer(sd);

	//Set last idle time... [Skotlex]
	if (battle_config.idletime_option&IDLE_WALK)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_WALK)
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_WALK)
		sd->idletime_mer = last_tick;

	unit_walktoxy(&sd->bl, x, y, 4);
}


/// Notification about the result of a disconnect request (ZC_ACK_REQ_DISCONNECT).
/// 018b <result>.W
/// result:
///     0 = disconnect (quit)
///     1 = cannot disconnect (wait 10 seconds)
///     ? = ignored
void clif_disconnect_ack(map_session_data* sd, short result)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x18b));
	WFIFOW(fd,0) = 0x18b;
	WFIFOW(fd,2) = result;
	WFIFOSET(fd,packet_len(0x18b));
}


/// Request to disconnect from server (CZ_REQ_DISCONNECT).
/// 018a <type>.W
/// type:
///     0 = quit
void clif_parse_QuitGame(int fd, map_session_data *sd)
{
	/*	Rovert's prevent logout option fixed [Valaris]	*/
	if( !sd->sc.getSCE(SC_CLOAKING) && !sd->sc.getSCE(SC_HIDING) && !sd->sc.getSCE(SC_CHASEWALK) && !sd->sc.getSCE(SC_CLOAKINGEXCEED) && !sd->sc.getSCE(SC_SUHIDE) && !sd->sc.getSCE(SC_NEWMOON) &&
		(!battle_config.prevent_logout || sd->canlog_tick == 0 || DIFF_TICK(gettick(), sd->canlog_tick) > battle_config.prevent_logout) )
	{
		pc_damage_log_clear(sd,0);
		clif_disconnect_ack(sd, 0);
		flush_fifo( fd );
		if( battle_config.drop_connection_on_quit ){
			set_eof( fd );
		}
	} else {
		clif_disconnect_ack(sd, 1);
	}
}


/// Requesting unit's name.
/// 0094 <id>.L (CZ_REQNAME)
/// 0368 <id>.L (CZ_REQNAME2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_GetCharNameRequest(int fd, map_session_data *sd)
{
	int id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	struct block_list* bl;
	//status_change *sc;

	if( id < 0 && -id == sd->bl.id ) // for disguises [Valaris]
		id = sd->bl.id;

	bl = map_id2bl(id);
	if( bl == nullptr )
		return;	// Lagged clients could request names of already gone mobs/players. [Skotlex]

	if( sd->bl.m != bl->m || !check_distance_bl(&sd->bl, bl, AREA_SIZE) )
		return; // Block namerequests past view range

	// 'see people in GM hide' cheat detection
	/* disabled due to false positives (network lag + request name of char that's about to hide = race condition)
	sc = status_get_sc(bl);
	if (sc && sc->option&OPTION_INVISIBLE && !disguised(bl) &&
		bl->type != BL_NPC && //Skip hidden NPCs which can be seen using Maya Purple
		pc_get_group_level(sd) < battle_config.hack_info_GM_level
	) {
		char gm_msg[256];
		sprintf(gm_msg, "Hack on NameRequest: character '%s' (account: %d) requested the name of an invisible target (id: %d).\n", sd->status.name, sd->status.account_id, id);
		ShowWarning(gm_msg);
		// information is sent to all online GMs
		intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, gm_msg);
		return;
	}
	*/

	clif_name(&sd->bl,bl,SELF);
}


/// Validates and processes global messages
/// 008c <packet len>.W <text>.?B (<name> : <message>) 00 (CZ_REQUEST_CHAT)
/// There are various variants of this packet.
void clif_parse_GlobalMessage(int fd, map_session_data* sd)
{
	char name[NAME_LENGTH], message[CHAT_SIZE_MAX], output[CHAT_SIZE_MAX+NAME_LENGTH*2];
	size_t length;

	// validate packet and retrieve name and message
	if( !clif_process_message(sd, false, name, message, output ) )
		return;

	if( sd->gcbind && ((sd->gcbind->opt&CHAN_OPT_CAN_CHAT) || pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN)) ) {
		channel_send(sd->gcbind,sd,message);
		return;
	}

	// send message to others (using the send buffer for temp. storage)
	clif_GlobalMessage( sd->bl, output, sd->chatID ? CHAT_WOS : AREA_CHAT_WOC );

	length = strlen(output) + 1;

	// send back message to the speaker
	WFIFOHEAD(fd,4+length);
	WFIFOW(fd,0) = 0x8e;
	WFIFOW(fd,2) = (uint16)(4+length);
	safestrncpy(WFIFOCP(fd,4), output, length );
	WFIFOSET(fd, WFIFOW(fd,2));

#ifdef PCRE_SUPPORT
	// trigger listening npcs
	map_foreachinallrange(npc_chat_sub, &sd->bl, AREA_SIZE, BL_NPC, output, strlen(output), &sd->bl);
#endif

	// Chat logging type 'O' / Global Chat
	log_chat(LOG_CHAT_GLOBAL, 0, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, nullptr, message);
	//achievement_update_objective(sd, AG_CHAT, 1, sd->bl.m); //! TODO: What's the official use of this achievement type?
}


/// /mm /mapmove (as @rura GM command) (CZ_MOVETO_MAP).
/// Request to warp to a map on given coordinates.
/// 0140 <map name>.16B <x>.W <y>.W
void clif_parse_MapMove(int fd, map_session_data *sd)
{
	char command[MAP_NAME_LENGTH_EXT+25];
	char* map_name;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	map_name = RFIFOCP(fd,info->pos[0]);
	map_name[MAP_NAME_LENGTH_EXT-1]='\0';
	safesnprintf(command,sizeof(command),"%cmapmove %s %d %d", atcommand_symbol, map_name,
	    RFIFOW(fd,info->pos[1]), //x
	    RFIFOW(fd,info->pos[2])); //y
	is_atcommand(fd, sd, command, 1);
}


/// Updates body and head direction of an object (ZC_CHANGE_DIRECTION).
/// 009c <id>.L <head dir>.W <dir>.B
/// head dir:
///     0 = straight
///     1 = turned CW
///     2 = turned CCW
/// dir:
///     0 = north
///     1 = northwest
///     2 = west
///     3 = southwest
///     4 = south
///     5 = southeast
///     6 = east
///     7 = northeast
void clif_changed_dir(struct block_list *bl, enum send_target target)
{
	unsigned char buf[64];

	WBUFW(buf,0) = 0x9c;
	WBUFL(buf,2) = bl->id;
	WBUFW(buf,6) = bl->type==BL_PC?((TBL_PC*)bl)->head_dir:0;
	WBUFB(buf,8) = unit_getdir(bl);

	clif_send(buf, packet_len(0x9c), bl, target);

	if (disguised(bl)) {
		WBUFL(buf,2) = disguised_bl_id(bl->id);
		WBUFW(buf,6) = 0;
		clif_send(buf, packet_len(0x9c), bl, SELF);
	}
}


/// Request to change own body and head direction.
/// 009b <head dir>.W <dir>.B (CZ_CHANGE_DIRECTION)
/// 0361 <head dir>.W <dir>.B (CZ_CHANGE_DIRECTION2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_ChangeDir(int fd, map_session_data *sd)
{
	unsigned char headdir, dir;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	headdir = RFIFOB(fd,info->pos[0]);
	dir = RFIFOB(fd,info->pos[1]);
	pc_setdir(sd, dir, headdir);

	clif_changed_dir(&sd->bl, AREA_WOS);
}


/// Request to show an emotion (CZ_REQ_EMOTION).
/// 00bf <type>.B
/// type:
///     @see enum emotion_type
void clif_parse_Emotion(int fd, map_session_data *sd){
	if( sd == nullptr ){
		return;
	}

	int emoticon = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	if (battle_config.basic_skill_check == 0 || pc_checkskill(sd, NV_BASIC) >= 2 || pc_checkskill(sd, SU_BASIC_SKILL) >= 1) {
		if (emoticon == ET_CHAT_PROHIBIT) {// prevent use of the mute emote [Valaris]
			clif_skill_fail( *sd, 1, USESKILL_FAIL_LEVEL, 1 );
			return;
		}
		// fix flood of emotion icon (ro-proxy): flood only the hacker player
		if (sd->emotionlasttime + 1 >= time(nullptr)) { // not more than 1 per second
			sd->emotionlasttime = time(nullptr);
			clif_skill_fail( *sd, 1, USESKILL_FAIL_LEVEL, 1 );
			return;
		}
		sd->emotionlasttime = time(nullptr);

		if (battle_config.idletime_option&IDLE_EMOTION)
			sd->idletime = last_tick;
		if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_EMOTION)
			sd->idletime_hom = last_tick;
		if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_EMOTION)
			sd->idletime_mer = last_tick;

		if (sd->state.block_action & PCBLOCK_EMOTION) {
			clif_skill_fail( *sd, 1, USESKILL_FAIL_LEVEL, 1 );
			return;
		}

		if(battle_config.client_reshuffle_dice && emoticon>=ET_DICE1 && emoticon<=ET_DICE6) {// re-roll dice
			emoticon = rnd()%6+ET_DICE1;
		}

		clif_emotion(&sd->bl, emoticon);
	} else
		clif_skill_fail( *sd, 1, USESKILL_FAIL_LEVEL, 1 );
}


/// Amount of currently online players, reply to /w /who (ZC_USER_COUNT).
/// 00c2 <count>.L
void clif_user_count(map_session_data* sd, int count)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0xc2));
	WFIFOW(fd,0) = 0xc2;
	WFIFOL(fd,2) = count;
	WFIFOSET(fd,packet_len(0xc2));
}


/// /w /who (CZ_REQ_USER_COUNT).
/// Request to display amount of currently connected players.
/// 00c1
void clif_parse_HowManyConnections(int fd, map_session_data *sd)
{
	clif_user_count(sd, map_getusers());
}

void clif_parse_ActionRequest_sub( map_session_data& sd, int action_type, int target_id, t_tick tick ){
	if (pc_isdead(&sd)) {
		clif_clearunit_area(sd.bl, CLR_DEAD);
		return;
	}

	// Statuses that don't let the player sit / stand / talk with NPCs (targeted)
	if (action_type != 0x00 && action_type != 0x07) {
		if (sd.sc.cant.interact)
			return;
		pc_stop_walking(&sd, 1);
	}
	pc_stop_attack(&sd);

	if(target_id<0 && -target_id == sd.bl.id) // for disguises [Valaris]
		target_id = sd.bl.id;

	switch(action_type)
	{
	case 0x00: // once attack
	case 0x07: // continuous attack

		if( pc_cant_act(&sd) )
			return;

		if (!battle_config.sdelay_attack_enable && pc_checkskill(&sd, SA_FREECAST) <= 0) {
			if (DIFF_TICK(tick, sd.ud.canact_tick) < 0) {
				clif_skill_fail( sd, 1, USESKILL_FAIL_SKILLINTERVAL );
				return;
			}
		}

		pc_delinvincibletimer(&sd);
		if (battle_config.idletime_option&IDLE_ATTACK)
			sd.idletime = last_tick;
		if (battle_config.hom_idle_no_share && sd.hd && battle_config.idletime_hom_option&IDLE_ATTACK)
			sd.idletime_hom = last_tick;
		if (battle_config.mer_idle_no_share && sd.md && battle_config.idletime_mer_option&IDLE_ATTACK)
			sd.idletime_mer = last_tick;
		unit_attack(&sd.bl, target_id, action_type != 0);
		break;
	case 0x02: // sitdown
		if (battle_config.basic_skill_check && pc_checkskill(&sd, NV_BASIC) < 3 && pc_checkskill(&sd, SU_BASIC_SKILL) < 1) {
			clif_skill_fail( sd, 1, USESKILL_FAIL_LEVEL, 2 );
			break;
		}

		if(pc_issit(&sd)) {
			//Bugged client? Just refresh them.
			clif_sitting(&sd.bl);
			return;
		}

		if (sd.ud.skilltimer != INVALID_TIMER || (sd.sc.opt1 && sd.sc.opt1 != OPT1_STONEWAIT && sd.sc.opt1 != OPT1_BURNING))
			break;

		if (sd.sc.count && (
			sd.sc.getSCE(SC_DANCING) ||
			(sd.sc.getSCE(SC_GRAVITATION) && sd.sc.getSCE(SC_GRAVITATION)->val3 == BCT_SELF)
		)) //No sitting during these states either.
			break;

		if (sd.state.block_action & PCBLOCK_SITSTAND) {
			clif_displaymessage(sd.fd, msg_txt(&sd,794)); // This action is currently blocked.
			break;
		}

		if (battle_config.idletime_option&IDLE_SIT)
			sd.idletime = last_tick;
		if (battle_config.hom_idle_no_share && sd.hd && battle_config.idletime_hom_option&IDLE_SIT)
			sd.idletime_hom = last_tick;
		if (battle_config.mer_idle_no_share && sd.md && battle_config.idletime_mer_option&IDLE_SIT)
			sd.idletime_mer = last_tick;

		pc_setsit(&sd);
		skill_sit(&sd, true);
		clif_sitting(&sd.bl);
		break;
	case 0x03: // standup
		if (!pc_issit(&sd)) {
			//Bugged client? Just refresh them.
			clif_standing(&sd.bl);
			return;
		}

		if (sd.sc.opt1 && sd.sc.opt1 != OPT1_STONEWAIT && sd.sc.opt1 != OPT1_BURNING)
			break;

		if (sd.state.block_action & PCBLOCK_SITSTAND) {
			clif_displaymessage(sd.fd, msg_txt(&sd,794)); // This action is currently blocked.
			break;
		}

		if (pc_setstand(&sd, false)) {
			if (battle_config.idletime_option&IDLE_SIT)
				sd.idletime = last_tick;
			if (battle_config.hom_idle_no_share && sd.hd && battle_config.idletime_hom_option&IDLE_SIT)
				sd.idletime_hom = last_tick;
			if (battle_config.mer_idle_no_share && sd.md && battle_config.idletime_mer_option&IDLE_SIT)
				sd.idletime_mer = last_tick;
			skill_sit(&sd, false);
			clif_standing(&sd.bl);
		}
		break;
	}
}


/// Request for an action.
/// 0089 <target id>.L <action>.B (CZ_REQUEST_ACT)
/// 0437 <target id>.L <action>.B (CZ_REQUEST_ACT2)
/// action:
///     0 = attack
///     1 = pick up item
///     2 = sit down
///     3 = stand up
///     7 = continous attack
///     12 = (touch skill?)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_ActionRequest(int fd, map_session_data *sd)
{
	if( sd == nullptr ){
		return;
	}

	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	clif_parse_ActionRequest_sub( *sd,
		RFIFOB(fd,info->pos[1]),
		RFIFOL(fd,info->pos[0]),
		gettick()
	);
}


/// Response to the death/system menu (CZ_RESTART).
/// 00b2 <type>.B
/// type:
///     0 = restart (respawn)
///     1 = char-select (disconnect)
void clif_parse_Restart(int fd, map_session_data *sd)
{
	switch(RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0])) {
	case 0x00:
		pc_respawn(sd,CLR_OUTSIGHT);
		break;
	case 0x01:
		/*	Rovert's Prevent logout option - Fixed [Valaris]	*/
		if( !sd->sc.getSCE(SC_CLOAKING) && !sd->sc.getSCE(SC_HIDING) && !sd->sc.getSCE(SC_CHASEWALK) && !sd->sc.getSCE(SC_CLOAKINGEXCEED) && !sd->sc.getSCE(SC_SUHIDE) && !sd->sc.getSCE(SC_NEWMOON) &&
			(!battle_config.prevent_logout || sd->canlog_tick == 0 || DIFF_TICK(gettick(), sd->canlog_tick) > battle_config.prevent_logout) )
		{	//Send to char-server for character selection.
			pc_damage_log_clear(sd,0);
			chrif_charselectreq(sd, session[fd]->client_addr);
		} else {
			clif_disconnect_ack(sd, 1);
		}
		break;
	}
}


/// Validates and processes whispered messages (CZ_WHISPER).
/// 0096 <packet len>.W <nick>.24B <message>.?B
void clif_parse_WisMessage(int fd, map_session_data* sd)
{
	map_session_data* dstsd;
	int i;
	char target[NAME_LENGTH], message[CHAT_SIZE_MAX], output[CHAT_SIZE_MAX+NAME_LENGTH*2];

	// validate packet and retrieve name and message
	if( !clif_process_message( sd, true, target, message, output ) )
		return;

	// Chat logging type 'W' / Whisper
	log_chat(LOG_CHAT_WHISPER, 0, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, target, message);

	//-------------------------------------------------------//
	//   Lordalfa - Paperboy - To whisper NPC commands       //
	//-------------------------------------------------------//
	if (target[0] && (strncasecmp(target,"NPC:",4) == 0) && (strlen(target) > 4))
	{
		char* str = target+4; //Skip the NPC: string part.
		struct npc_data* npc;
		if ((npc = npc_name2id(str))) {
			char split_data[NUM_WHISPER_VAR][CHAT_SIZE_MAX];
			char *split;
			char event[EVENT_NAME_LENGTH];

			str = message;
			// skip codepage indicator, if detected
			if( str[0] == '|' && strlen(str) >= 4 )
				str += 3;
			for( i = 0; i < NUM_WHISPER_VAR; ++i ) {// Splits the message using '#' as separators
				split = strchr(str,'#');
				if( split == nullptr ) { // use the remaining string
					safestrncpy(split_data[i], str, ARRAYLENGTH(split_data[i]));
					for( ++i; i < NUM_WHISPER_VAR; ++i )
						split_data[i][0] = '\0';
					break;
				}
				*split = '\0';
				safestrncpy(split_data[i], str, ARRAYLENGTH(split_data[i]));
				str = split+1;
			}

			for( i = 0; i < NUM_WHISPER_VAR; ++i ) {
				char variablename[CHAT_SIZE_MAX];
				safesnprintf(variablename,sizeof(variablename),"@whispervar%d$", i);
				set_var_str( sd, variablename, split_data[i] );
			}

			safesnprintf(event,sizeof(event),"%s::%s", npc->exname,script_config.onwhisper_event_name);
			npc_event(sd,event,0); // Calls the NPC label

			return;
		}
	} else if( target[0] == '#' ) {
		struct Channel *channel = nullptr;
		char* chname = target;

		channel = channel_name2channel(chname,sd,3);
		if(channel && (pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) || ((channel->opt&CHAN_OPT_CAN_CHAT) && channel_pccheckgroup(channel,sd->group_id)))){
			if(channel_pc_haschan(sd,channel)>=0){ //we are in the chan
				channel_send(channel,sd,message);
			}
			else if( channel->pass[0] == '\0') { //no pass needed
				if (channel_join(channel,sd)==0)
					channel_send(channel,sd,message); //join success
			}
			else {
				clif_displaymessage(fd, msg_txt(sd,1402)); //You're not in that channel, type '@join <#channel_name>'
			}
			return;
		}
	}

	// searching destination character
	dstsd = map_nick2sd(target,false);

	if (dstsd == nullptr || strcmp(dstsd->status.name, target) != 0) {
		// player is not on this map-server
		// At this point, don't send wisp/page if it's not exactly the same name, because (example)
		// if there are 'Test' player on an other map-server and 'test' player on this map-server,
		// and if we ask for 'Test', we must not contact 'test' player
		// so, we send information to inter-server, which is the only one which decide (and copy correct name).
		intif_wis_message(sd, target, message, strlen(message) + 1);
		return;
	}

	// if player ignores everyone
	if (dstsd->state.ignoreAll && pc_get_group_level(sd) <= pc_get_group_level(dstsd)) {
		if (pc_isinvisible(dstsd) && pc_get_group_level(sd) < pc_get_group_level(dstsd))
			clif_wis_end( *sd, ACKWHISPER_TARGET_OFFLINE );
		else
			clif_wis_end( *sd, ACKWHISPER_ALL_IGNORED );
		return;
	}

	// if player is autotrading
	if (dstsd->state.autotrade == 1){
		safesnprintf(output,sizeof(output),"%s is in autotrade mode and cannot receive whispered messages.", dstsd->status.name);
		clif_wis_message(sd, wisp_server_name, output, strlen(output) + 1, 0);
		return;
	}

	if (pc_get_group_level(sd) <= pc_get_group_level(dstsd)) {
		// if player ignores the source character
		ARR_FIND(0, MAX_IGNORE_LIST, i, dstsd->ignore[i].name[0] == '\0' || strcmp(dstsd->ignore[i].name, sd->status.name) == 0);
		if(i < MAX_IGNORE_LIST && dstsd->ignore[i].name[0] != '\0') { // source char present in ignore list
			clif_wis_end( *sd, ACKWHISPER_IGNORED );
			return;
		}
	}

	// notify sender of success
	clif_wis_end( *sd, ACKWHISPER_SUCCESS );

	// Normal message
	clif_wis_message(dstsd, sd->status.name, message, strlen(message)+1, 0);
}


/// /b /nb (CZ_BROADCAST).
/// Request to broadcast a message on whole server.
/// 0099 <packet len>.W <text>.?B 00
void clif_parse_Broadcast(int fd, map_session_data* sd) {
	char command[CHAT_SIZE_MAX+11];
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	unsigned int len = RFIFOW(fd,info->pos[0])-4;
	char* msg = RFIFOCP(fd,info->pos[1]);

	// as the length varies depending on the command used, just block unreasonably long strings
	mes_len_check(msg, len, CHAT_SIZE_MAX);

	safesnprintf(command,sizeof(command),"%ckami %s", atcommand_symbol, msg);
	is_atcommand(fd, sd, command, 1);
}


/// Request to pick up an item.
/// 009f <id>.L (CZ_ITEM_PICKUP)
/// 0362 <id>.L (CZ_ITEM_PICKUP2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_TakeItem(int fd, map_session_data *sd)
{
	struct flooritem_data *fitem;
	int map_object_id;

	map_object_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	fitem = (struct flooritem_data*)map_id2bl(map_object_id);

	do {
		if (pc_isdead(sd)) {
			clif_clearunit_area( sd->bl, CLR_DEAD );
			break;
		}

		if (fitem == nullptr || fitem->bl.type != BL_ITEM || fitem->bl.m != sd->bl.m)
			break;

		if (pc_cant_act(sd))
			break;

		if (!pc_takeitem(sd, fitem))
			break;

		return;
	} while (0);
	// Client REQUIRES a fail packet or you can no longer pick items.
	clif_additem(sd,0,0,6);
}


/// Request to drop an item.
/// 00a2 <index>.W <amount>.W (CZ_ITEM_THROW)
/// 0363 <index>.W <amount>.W (CZ_ITEM_THROW2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_DropItem(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int item_index  = RFIFOW(fd,info->pos[0]) -2;
	int item_amount = RFIFOW(fd,info->pos[1]) ;

	for(;;) {
		if (pc_isdead(sd))
			break;

		if (pc_cant_act2(sd) || sd->npc_id)
			break;

		if (sd->sc.cant.drop)
			break;

		if (!pc_dropitem(sd, item_index, item_amount))
			break;

		if (battle_config.idletime_option&IDLE_DROPITEM)
			sd->idletime = last_tick;
		if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_DROPITEM)
			sd->idletime_hom = last_tick;
		if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_DROPITEM)
			sd->idletime_mer = last_tick;

		return;
	}

	//Because the client does not like being ignored.
	clif_dropitem( *sd, item_index, 0 );
}


/// Request to use an item.
/// 00a7 <index>.W <account id>.L (CZ_USE_ITEM)
/// 0439 <index>.W <account id>.L (CZ_USE_ITEM2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_UseItem(int fd, map_session_data *sd)
{
	int n;

	if (pc_isdead(sd)) {
		clif_clearunit_area( sd->bl, CLR_DEAD );
		return;
	}

	if ( (!sd->npc_id && pc_istrading(sd)) || sd->chatID || (sd->state.block_action & PCBLOCK_USEITEM) ) {
		clif_msg(sd, MSI_BUSY);
		return;
	}

	//Whether the item is used or not is irrelevant, the char ain't idle. [Skotlex]
	if (battle_config.idletime_option&IDLE_USEITEM)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_USEITEM)
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_USEITEM)
		sd->idletime_mer = last_tick;
	n = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-2;

	if(n <0 || n >= MAX_INVENTORY)
		return;
	if (!pc_useitem(sd,n))
		clif_useitemack(sd,n,0,false); //Send an empty ack packet or the client gets stuck.
}


/// Request to equip an item
/// 00a9 <index>.W <position>.W (CZ_REQ_WEAR_EQUIP).
/// 0998 <index>.W <position>.L (CZ_REQ_WEAR_EQUIP_V5)
void clif_parse_EquipItem(int fd,map_session_data *sd)
{
	int index;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	if(pc_isdead(sd)) {
		clif_clearunit_area( sd->bl, CLR_DEAD );
		return;
	}
	index = RFIFOW(fd,info->pos[0])-2;
	if (index < 0 || index >= MAX_INVENTORY)
		return; //Out of bounds check.

	if((sd->npc_id && !sd->npc_item_flag) || (sd->state.block_action & PCBLOCK_EQUIP)) {
		clif_msg_color( sd, MSI_CAN_NOT_EQUIP_ITEM, color_table[COLOR_RED] );
		return;
	} else if (sd->state.storage_flag || sd->sc.opt1)
		; //You can equip/unequip stuff while storage is open/under status changes
	else if (pc_cant_act2(sd))
		return;

	if(!sd->inventory.u.items_inventory[index].identify) {
		clif_equipitemack( *sd, ITEM_EQUIP_ACK_FAIL, index ); // fail
		return;
	}

	if(!sd->inventory_data[index])
		return;

	if(sd->inventory_data[index]->type == IT_PETARMOR){
		pet_equipitem(sd,index);
		return;
	}

	if (battle_config.idletime_option&IDLE_USEITEM)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_USEITEM)
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_USEITEM)
		sd->idletime_mer = last_tick;

	//Client doesn't send the position for ammo.
	if(sd->inventory_data[index]->type == IT_AMMO)
		pc_equipitem(sd,index,EQP_AMMO);
	else {
	int req_pos;

#if PACKETVER  >= 20120925
		req_pos = RFIFOL(fd,info->pos[1]);
#else
		req_pos = (int)RFIFOW(fd,info->pos[1]);
#endif
		pc_equipitem(sd,index,req_pos);
	}
}


/// Request to take off an equip (CZ_REQ_TAKEOFF_EQUIP).
/// 00ab <index>.W
void clif_parse_UnequipItem(int fd,map_session_data *sd)
{
	int index;

	if(pc_isdead(sd)) {
		clif_clearunit_area( sd->bl, CLR_DEAD );
		return;
	}

	if((sd->npc_id && !sd->npc_item_flag) || (sd->state.block_action & PCBLOCK_EQUIP)) {
		clif_msg_color( sd, MSI_CAN_NOT_EQUIP_ITEM, color_table[COLOR_RED] );
		return;
	} else if (sd->state.storage_flag || sd->sc.opt1)
		; //You can equip/unequip stuff while storage is open/under status changes
	else if (pc_cant_act2(sd))
		return;

	index = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-2;

	if (battle_config.idletime_option&IDLE_USEITEM)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_USEITEM)
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_USEITEM)
		sd->idletime_mer = last_tick;

	pc_unequipitem(sd,index,1);
}


/// Request to start a conversation with an NPC (CZ_CONTACTNPC).
/// 0090 <id>.L <type>.B
/// type:
///     1 = click
void clif_parse_NpcClicked(int fd,map_session_data *sd)
{
	if( sd == nullptr ){
		return;
	}

	struct block_list *bl;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	if(pc_isdead(sd)) {
		clif_clearunit_area( sd->bl, CLR_DEAD );
		return;
	}

	if( pc_cant_act2(sd) || sd->npc_id || pc_hasprogress( sd, WIP_DISABLE_NPC ) ){
#ifdef RENEWAL
		clif_msg( sd, MSI_BUSY);
#endif
		return;
	}

	if( sd->state.mail_writing )
		return;

	bl = map_id2bl(RFIFOL(fd,info->pos[0]));
	//type = RFIFOB(fd,info->pos[1]);
	if (!bl) return;
	switch (bl->type) {
		case BL_MOB:
		case BL_PC:
			clif_parse_ActionRequest_sub( *sd, 0x07, bl->id, gettick() );
			break;
		case BL_NPC:
#ifdef RENEWAL
			if (sd->ud.skill_id < RK_ENCHANTBLADE && sd->ud.skilltimer != INVALID_TIMER) { // Should only show an error message for non-3rd job skills with a running timer
				clif_msg(sd, MSI_BUSY);
				break;
			}
#endif
			if( bl->m != -1 ){ // the user can't click floating npcs directly (hack attempt)
				struct npc_data* nd = (struct npc_data*)bl;

				// Progressbar is running
				if( nd->progressbar.timeout > 0 ){
					return;
				}

				npc_click(sd,nd);
			}
			break;
	}
}


/// Selection between buy/sell was made (CZ_ACK_SELECT_DEALTYPE).
/// 00c5 <id>.L <type>.B
/// type:
///     0 = buy
///     1 = sell
void clif_parse_NpcBuySellSelected(int fd,map_session_data *sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	if (sd->state.trading)
		return;
	npc_buysellsel(sd,RFIFOL(fd,info->pos[0]),RFIFOB(fd,info->pos[1]));
}


/// Notification about the result of a purchase attempt from an NPC shop (ZC_PC_PURCHASE_RESULT).
/// 00ca <result>.B
/// result:
///     0 = "The deal has successfully completed."
///     1 = "You do not have enough zeny."
///     2 = "You are over your Weight Limit."
///     3 = "Out of the maximum capacity, you have too many items."
///     9 = "Amounts are exceeded the possession of the item is not available for purchase."
///    10 = "Props open-air store sales will be traded in RODEX"
///    11 = "The exchange failed."
///    12 = "The exchange was well done."
///    13 = "The item is already sold and out of stock."
///    14 = "There is not enough goods to exchange."
void clif_npc_buy_result( map_session_data* sd, e_purchase_result result ){
	PACKET_ZC_PC_PURCHASE_RESULT p = {};

	p.packetType = HEADER_ZC_PC_PURCHASE_RESULT;
	p.result = (uint8)result;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Request to buy chosen items from npc shop.
/// 00c8 <packet len>.W { <amount>.W <name id>.W }* (CZ_PC_PURCHASE_ITEMLIST)
void clif_parse_NpcBuyListSend( int fd, map_session_data* sd ){
	const PACKET_CZ_PC_PURCHASE_ITEMLIST *p = reinterpret_cast<PACKET_CZ_PC_PURCHASE_ITEMLIST*>( RFIFOP( fd, 0 ) );

	uint16 n = ( p->packetLength - sizeof( *p ) ) / sizeof( p->items[0] );
	e_purchase_result result;

	if( sd->state.trading || !sd->npc_shopid )
		result = e_purchase_result::PURCHASE_FAIL_MONEY;
	else{
		std::vector<s_npc_buy_list> items = {};

		items.reserve( n );

		for( uint16 i = 0; i < n; i++ ){
			s_npc_buy_list item = {};

			item.nameid = p->items[i].itemId;
			item.qty = p->items[i].amount;

			items.push_back( item );
		}

		result = npc_buylist( sd, items );
	}

	sd->npc_shopid = 0; //Clear shop data.
	clif_npc_buy_result(sd, result);
}


/// Notification about the result of a sell attempt to an NPC shop (ZC_PC_SELL_RESULT).
/// 00cb <result>.B
/// result:
///     0 = "The deal has successfully completed."
///     1 = "The deal has failed."
void clif_npc_sell_result(map_session_data* sd, unsigned char result)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0xcb));
	WFIFOW(fd,0) = 0xcb;
	WFIFOB(fd,2) = result;
	WFIFOSET(fd,packet_len(0xcb));
}


/// Request to sell chosen items to npc shop (CZ_PC_SELL_ITEMLIST).
/// 00c9 <packet len>.W { <index>.W <amount>.W }*
void clif_parse_NpcSellListSend(int fd,map_session_data *sd)
{
	int fail=0;
	const PACKET_CZ_PC_SELL_ITEMLIST* p = reinterpret_cast<PACKET_CZ_PC_SELL_ITEMLIST*>( RFIFOP( fd, 0 ) );

	int n = ( p->packetLength - sizeof( *p ) ) / sizeof( p->sellList[0] );
	
	if (sd->state.trading || !sd->npc_shopid)
		fail = 1;
	else
		fail = npc_selllist(sd, n, p->sellList);

	sd->npc_shopid = 0; //Clear shop data.
	clif_npc_sell_result(sd, fail);
}


/// Chatroom creation request (CZ_CREATE_CHATROOM).
/// 00d5 <packet len>.W <limit>.W <type>.B <passwd>.8B <title>.?B
/// type:
///     0 = private
///     1 = public
void clif_parse_CreateChatRoom(int fd, map_session_data* sd)
{
	if( sd == nullptr ){
		return;
	}

	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int len = RFIFOW(fd,info->pos[0])-15;
	int limit = RFIFOW(fd,info->pos[1]);
	bool pub = (RFIFOB(fd,info->pos[2]) != 0);
	const char* password = RFIFOCP(fd,info->pos[3]); //not zero-terminated
	const char* title = RFIFOCP(fd,info->pos[4]); // not zero-terminated
	char s_password[CHATROOM_PASS_SIZE];
	char s_title[CHATROOM_TITLE_SIZE];

	if (sd->sc.getSCE(SC_NOCHAT) && sd->sc.getSCE(SC_NOCHAT)->val1&MANNER_NOROOM)
		return;
	if(battle_config.basic_skill_check && pc_checkskill(sd,NV_BASIC) < 4 && pc_checkskill(sd, SU_BASIC_SKILL) < 1) {
		clif_skill_fail( *sd, 1, USESKILL_FAIL_LEVEL, 3 );
		return;
	}

	if( npc_isnear(&sd->bl) ) {
		// uncomment to send msg_txt.
		//char output[150];
		//sprintf(output, msg_txt(662), battle_config.min_npc_vendchat_distance);
		//clif_displaymessage(sd->fd, output);
		clif_skill_fail( *sd, 1, USESKILL_FAIL_THERE_ARE_NPC_AROUND );
		return;
	}

	if( len <= 0 )
		return; // invalid input

	safestrncpy(s_password, password, CHATROOM_PASS_SIZE);
	safestrncpy(s_title, title, min(len+1,CHATROOM_TITLE_SIZE)); //NOTE: assumes that safestrncpy will not access the len+1'th byte

	chat_createpcchat(sd, s_title, s_password, limit, pub);
}


/// Chatroom join request (CZ_REQ_ENTER_ROOM).
/// 00d9 <chat ID>.L <passwd>.8B
void clif_parse_ChatAddMember(int fd, map_session_data* sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int chatid = RFIFOL(fd,info->pos[0]);
	const char* password = RFIFOCP(fd,info->pos[1]); // not zero-terminated

	chat_joinchat(sd,chatid,password);
}


/// Chatroom properties adjustment request (CZ_CHANGE_CHATROOM).
/// 00de <packet len>.W <limit>.W <type>.B <passwd>.8B <title>.?B
/// type:
///     0 = private
///     1 = public
void clif_parse_ChatRoomStatusChange(int fd, map_session_data* sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int len = RFIFOW(fd,info->pos[0])-15;
	int limit = RFIFOW(fd,info->pos[1]);
	bool pub = (RFIFOB(fd,info->pos[2]) != 0);
	const char* password = RFIFOCP(fd,info->pos[3]); // not zero-terminated
	const char* title = RFIFOCP(fd,info->pos[4]); // not zero-terminated
	char s_password[CHATROOM_PASS_SIZE];
	char s_title[CHATROOM_TITLE_SIZE];

	if( len <= 0 )
		return; // invalid input

	safestrncpy(s_password, password, CHATROOM_PASS_SIZE);
	safestrncpy(s_title, title, min(len+1,CHATROOM_TITLE_SIZE)); //NOTE: assumes that safestrncpy will not access the len+1'th byte

	chat_changechatstatus(sd, s_title, s_password, limit, pub);
}


/// Request to change the chat room ownership (CZ_REQ_ROLE_CHANGE).
/// 00e0 <role>.L <nick>.24B
/// role:
///     0 = owner
///     1 = normal
void clif_parse_ChangeChatOwner(int fd, map_session_data* sd)
{
	//int role = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	chat_changechatowner(sd,RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[1]));
}


/// Request to expel a player from chat room (CZ_REQ_EXPEL_MEMBER).
/// 00e2 <name>.24B
void clif_parse_KickFromChat(int fd,map_session_data *sd)
{
	chat_kickchat(sd,RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// Request to leave the current chatroom (CZ_EXIT_ROOM).
/// 00e3
void clif_parse_ChatLeave(int fd, map_session_data* sd)
{
	chat_leavechat(sd,0);
}

// Handles notifying asker and rejecter of what has just ocurred.
// Type is used to determine the correct msg_txt to use
void clif_noask_sub( map_session_data& sd, map_session_data& tsd, int type ){
	char output[CHAT_SIZE_MAX];

	// Your request has been rejected by autoreject option.
	clif_messagecolor( &sd.bl, color_table[COLOR_LIGHT_GREEN], msg_txt( &sd, 392 ), false, SELF );

	// Notice that a request was rejected.
	safesnprintf( output, CHAT_SIZE_MAX, msg_txt( &tsd, type ), sd.status.name );
	clif_messagecolor( &tsd.bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
}

/// Request to begin a trade (CZ_REQ_EXCHANGE_ITEM).
/// 00e4 <account id>.L
void clif_parse_TradeRequest(int fd,map_session_data *sd)
{
	map_session_data *t_sd;

	t_sd = map_id2sd(RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));

	if(!sd->chatID && pc_cant_act(sd))
		return; //You can trade while in a chatroom.

	if(t_sd){
		// @noask [LuzZza]
		if(t_sd->state.noask) {
			clif_noask_sub( *sd, *t_sd, 393 ); // Autorejected trade request from %s.
			return;
		}

		if (t_sd->state.mail_writing) {
			int old = sd->trade_partner;

			// Fake trading
			sd->trade_partner = t_sd->status.account_id;
			clif_tradestart(sd, 5);
			// Restore old state
			sd->trade_partner = old;

			return;
		}
	}

	if( battle_config.basic_skill_check && pc_checkskill(sd,NV_BASIC) < 1 && pc_checkskill(sd, SU_BASIC_SKILL) < 1) {
		clif_skill_fail( *sd, 1 );
		return;
	}

	trade_traderequest(sd,t_sd);
}


/// Answer to a trade request (CZ_ACK_EXCHANGE_ITEM).
/// 00e6 <result>.B
/// result:
///     3 = accepted
///     4 = rejected
void clif_parse_TradeAck(int fd,map_session_data *sd)
{
	trade_tradeack(sd,RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// Request to add an item to current trade (CZ_ADD_EXCHANGE_ITEM).
/// 00e8 <index>.W <amount>.L
void clif_parse_TradeAddItem(int fd,map_session_data *sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	short index = RFIFOW(fd,info->pos[0]);
	int amount = RFIFOL(fd,info->pos[1]);

	if( index == 0 )
		trade_tradeaddzeny(sd, amount);
	else
		trade_tradeadditem(sd, server_index(index), (short)amount);
}


/// Request to lock items in current trade (CZ_CONCLUDE_EXCHANGE_ITEM).
/// 00eb
void clif_parse_TradeOk(int fd,map_session_data *sd)
{
	trade_tradeok(sd);
}


/// Request to cancel current trade (CZ_CANCEL_EXCHANGE_ITEM).
/// 00ed
void clif_parse_TradeCancel(int fd,map_session_data *sd)
{
	trade_tradecancel(sd);
}


/// Request to commit current trade (CZ_EXEC_EXCHANGE_ITEM).
/// 00ef
void clif_parse_TradeCommit(int fd,map_session_data *sd)
{
	trade_tradecommit(sd);
}


/// Request to stop chasing/attacking an unit (CZ_CANCEL_LOCKON).
/// 0118
void clif_parse_StopAttack(int fd,map_session_data *sd)
{
	pc_stop_attack(sd);
	if (sd) sd->ud.state.attack_continue = 0;
}


/// Request to move an item from inventory to cart.
/// 0126 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_BODY_TO_CART)
void clif_parse_PutItemToCart( int fd, map_session_data *sd ){
	if (pc_istrading(sd) || !pc_iscarton(sd) || pc_cant_act2(sd))
		return;
	if (map_getmapflag(sd->bl.m, MF_NOUSECART))
		return;

	const PACKET_CZ_MOVE_ITEM_FROM_BODY_TO_CART* p = reinterpret_cast<PACKET_CZ_MOVE_ITEM_FROM_BODY_TO_CART*>( RFIFOP( fd, 0 ) );

	pc_putitemtocart( sd, server_index( p->index ), p->count);
}


/// Request to move an item from cart to inventory (CZ_MOVE_ITEM_FROM_CART_TO_BODY).
/// 0127 <index>.W <amount>.L
void clif_parse_GetItemFromCart(int fd,map_session_data *sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	if (!pc_iscarton(sd) || pc_cant_act2(sd))
		return;
	if (map_getmapflag(sd->bl.m, MF_NOUSECART))
		return;
	pc_getitemfromcart(sd,RFIFOW(fd,info->pos[0])-2,RFIFOL(fd,info->pos[1]));
}


/// Request to remove cart/falcon/peco/dragon/mado (CZ_REQ_CARTOFF).
/// 012a
void clif_parse_RemoveOption(int fd,map_session_data *sd)
{
	if( !(sd->sc.option&(OPTION_RIDING|OPTION_FALCON|OPTION_DRAGON|OPTION_MADOGEAR))
#ifdef NEW_CARTS
		&& sd->sc.getSCE(SC_PUSH_CART) )
		pc_setcart(sd,0);
#else
		)
		pc_setoption(sd,sd->sc.option&~OPTION_CART);
#endif
	else  // priority to remove this option before we can clear cart
		pc_setoption(sd,sd->sc.option&~(OPTION_RIDING|OPTION_FALCON|OPTION_DRAGON|OPTION_MADOGEAR));
}


/// Request to select cart's visual look for new cart design (ZC_SELECTCART).
/// 097f <Length>.W <identity>.L <type>.B
void clif_SelectCart(map_session_data *sd) {
#if PACKETVER >= 20150826
	int i = 0, carts = 3;

	int fd = sd->fd;
	WFIFOHEAD(fd,8 + carts);
	WFIFOW(fd,0) = 0x97f;
	WFIFOW(fd,2) = 8 + carts;
	WFIFOL(fd,4) = sd->status.account_id;
	// Right now we have 10-12, tested it you can also enable selection for all cart styles here(1-12)
	for( i = 0; i < carts; i++ ) {
	WFIFOB(fd,8 + i) = 10 + i;
	}
	WFIFOSET(fd,8 + carts);
#endif
}


/// Request to select cart's visual look for new cart design (CZ_SELECTCART).
/// 0980 <identity>.L <type>.B
void clif_parse_SelectCart(int fd,map_session_data *sd) {
#if PACKETVER >= 20150826
	int type;

	// Check identity
	if( !sd || pc_checkskill(sd,MC_CARTDECORATE) < 1 || RFIFOL(fd,2) != sd->status.account_id )
	return;

	type = (int)RFIFOB(fd,6);

	// Check type
	if( type < 10 || type > 12 ) 
		return;

	pc_setcart(sd, type);
#endif
}


/// Request to change cart's visual look (CZ_REQ_CHANGECART).
/// 01af <num>.W
void clif_parse_ChangeCart(int fd,map_session_data *sd)
{// TODO: State tracking?
	int type;

	if( !sd || pc_checkskill(sd, MC_CHANGECART) < 1 )
		return;

#ifdef RENEWAL
	if (sd->npc_id || pc_hasprogress(sd, WIP_DISABLE_SKILLITEM)) {
		clif_msg(sd, MSI_BUSY);
		return;
	}
#endif

	type = (int)RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	if( 
#ifdef NEW_CARTS
#if PACKETVER >= 20191106
		(type == 13 && sd->status.base_level > 100) ||
#endif
		(type == 9 && sd->status.base_level > 130) ||
		(type == 8 && sd->status.base_level > 120) ||
		(type == 7 && sd->status.base_level > 110) ||
		(type == 6 && sd->status.base_level > 100) ||
#endif
		(type == 5 && sd->status.base_level > 90) ||
	    (type == 4 && sd->status.base_level > 80) ||
	    (type == 3 && sd->status.base_level > 65) ||
	    (type == 2 && sd->status.base_level > 40) ||
	    (type == 1))
		pc_setcart(sd, type);
}


/// Request to increase status (CZ_STATUS_CHANGE).
/// 00bb <status id>.W <amount>.B
/// status id:
///     SP_STR ~ SP_LUK
/// amount:
///     Old clients always send 1 for this, even when using /str+ and the like.
///     Newer clients (2013-12-23 and newer) send the correct amount.
void clif_parse_StatusUp(int fd,map_session_data *sd)
{
	int increase_amount = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[1]);

	if( increase_amount < 0 ) {
		ShowDebug("clif_parse_StatusUp: Negative 'increase' value sent by client! (fd: %d, value: %d)\n",
			fd, increase_amount);
	}
	pc_statusup(sd,RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]),increase_amount);
}


/// Request to increase trait status.
/// 0b24 <status id>.W <amount>.W (CZ_ADVANCED_STATUS_CHANGE)
/// status id:
///     SP_POW ~ SP_CON
/// amount:
///     The amount to increase the trait status
void clif_parse_traitstatus_up( int fd, map_session_data *sd ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	const PACKET_CZ_ADVANCED_STATUS_CHANGE* p = reinterpret_cast<PACKET_CZ_ADVANCED_STATUS_CHANGE*>( RFIFOP( fd, 0 ) );

	if( p->amount < 0 ){
		ShowDebug( "clif_parse_traitstatus_up: Negative 'increase' value sent by client! %s (AID: %d, CID: %d, value: %d)\n", sd->status.name, sd->status.account_id, sd->status.char_id, p->amount );
		return;
	}

	pc_traitstatusup( sd, p->type, p->amount );
#endif
}


/// Request to increase level of a skill (CZ_UPGRADE_SKILLLEVEL).
/// 0112 <skill id>.W
void clif_parse_SkillUp(int fd,map_session_data *sd)
{
	pc_skillup(sd,RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}

static void clif_parse_UseSkillToId_homun(struct homun_data *hd, map_session_data *sd, t_tick tick, uint16 skill_id, uint16 skill_lv, int target_id)
{
	int lv;

	if( !hd )
		return;
	if( skill_isNotOk_hom(hd, skill_id, skill_lv) ) {
		clif_emotion(&hd->bl, ET_THINK);
		return;
	}
	if( hd->bl.id != target_id && skill_get_inf(skill_id)&INF_SELF_SKILL )
		target_id = hd->bl.id;
	if( hd->ud.skilltimer != INVALID_TIMER ) {
		if( skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST ) return;
	} else if( DIFF_TICK(tick, hd->ud.canact_tick) < 0 ) {
		clif_emotion(&hd->bl, ET_THINK);
		if (hd->master)
			clif_skill_fail( *hd->master, skill_id, USESKILL_FAIL_SKILLINTERVAL );
		return;
	}

	lv = hom_checkskill(hd, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit_skilluse_id(&hd->bl, target_id, skill_id, skill_lv);
}

static void clif_parse_UseSkillToPos_homun(struct homun_data *hd, map_session_data *sd, t_tick tick, uint16 skill_id, uint16 skill_lv, short x, short y, int skillmoreinfo)
{
	int lv;
	if( !hd )
		return;
	if( skill_isNotOk_hom(hd, skill_id, skill_lv) ) {
		clif_emotion(&hd->bl, ET_THINK);
		return;
	}
	if( hd->ud.skilltimer != INVALID_TIMER ) {
		if( skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST ) return;
	} else if( DIFF_TICK(tick, hd->ud.canact_tick) < 0 ) {
		clif_emotion(&hd->bl, ET_THINK);
		if (hd->master)
			clif_skill_fail( *hd->master, skill_id, USESKILL_FAIL_SKILLINTERVAL );
		return;
	}

#ifdef RENEWAL
	if (hd->sc.getSCE(SC_BASILICA_CELL))
#else
	if (hd->sc.getSCE(SC_BASILICA))
#endif
		return;
	lv = hom_checkskill(hd, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit_skilluse_pos(&hd->bl, x, y, skill_id, skill_lv);
}

static void clif_parse_UseSkillToId_mercenary(s_mercenary_data *md, map_session_data *sd, t_tick tick, uint16 skill_id, uint16 skill_lv, int target_id)
{
	int lv;

	if( !md )
		return;
	if( skill_isNotOk_mercenary(skill_id, *md) )
		return;
	if( md->bl.id != target_id && skill_get_inf(skill_id)&INF_SELF_SKILL )
		target_id = md->bl.id;
	if( md->ud.skilltimer != INVALID_TIMER )
	{
		if( skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST ) return;
	}
	else if( DIFF_TICK(tick, md->ud.canact_tick) < 0 )
		return;

	lv = mercenary_checkskill(md, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit_skilluse_id(&md->bl, target_id, skill_id, skill_lv);
}

static void clif_parse_UseSkillToPos_mercenary(s_mercenary_data *md, map_session_data *sd, t_tick tick, uint16 skill_id, uint16 skill_lv, short x, short y, int skillmoreinfo)
{
	int lv;
	if( !md )
		return;
	if( skill_isNotOk_mercenary(skill_id, *md) )
		return;
	if( md->ud.skilltimer != INVALID_TIMER )
		return;
	if( DIFF_TICK(tick, md->ud.canact_tick) < 0 )
	{
		if (md->master)
			clif_skill_fail( *md->master, skill_id, USESKILL_FAIL_SKILLINTERVAL );
		return;
	}

#ifdef RENEWAL
	if (md->sc.getSCE(SC_BASILICA_CELL))
#else
	if (md->sc.getSCE(SC_BASILICA))
#endif
		return;
	lv = mercenary_checkskill(md, skill_id);
	if( skill_lv > lv )
		skill_lv = lv;
	if( skill_lv )
		unit_skilluse_pos(&md->bl, x, y, skill_id, skill_lv);
}

void clif_parse_skill_toid( map_session_data* sd, uint16 skill_id, uint16 skill_lv, int target_id ){
	if( sd == nullptr ){
		return;
	}

	t_tick tick = gettick();

	if( skill_lv < 1 ) skill_lv = 1; //No clue, I have seen the client do this with guild skills :/ [Skotlex]

	int inf = skill_get_inf(skill_id);
	if (inf&INF_GROUND_SKILL || !inf)
		return; //Using a ground/passive skill on a target? WRONG.

	if (sd->state.block_action & PCBLOCK_SKILL) {
		clif_msg(sd, MSI_BUSY);
		return;
	}

	if( SKILL_CHK_HOMUN(skill_id) ) {
		clif_parse_UseSkillToId_homun(sd->hd, sd, tick, skill_id, skill_lv, target_id);
		return;
	}

	if( SKILL_CHK_MERC(skill_id) ) {
		clif_parse_UseSkillToId_mercenary(sd->md, sd, tick, skill_id, skill_lv, target_id);
		return;
	}

	// Whether skill fails or not is irrelevant, the char ain't idle. [Skotlex]
	// This is done here, because homunculi and mercenaries can be triggered by AI and not by the player itself
	if (battle_config.idletime_option&IDLE_USESKILLTOID)
		sd->idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd->hd && battle_config.idletime_hom_option&IDLE_USESKILLTOID)
		sd->idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd->md && battle_config.idletime_mer_option&IDLE_USESKILLTOID)
		sd->idletime_mer = last_tick;

	if( sd->npc_id ){
		if( pc_hasprogress( sd, WIP_DISABLE_SKILLITEM ) || !sd->npc_item_flag || !( inf & INF_SELF_SKILL ) ){
#ifdef RENEWAL
			clif_msg( sd, MSI_BUSY);
#endif
			return;
		}
	}

	if ((pc_cant_act2(sd) || sd->chatID) && 
		skill_id != RK_REFRESH && 
		!( ( skill_id == SR_GENTLETOUCH_CURE || skill_id == SU_GROOMING ) && (sd->sc.opt1 == OPT1_STONE || sd->sc.opt1 == OPT1_FREEZE || sd->sc.opt1 == OPT1_STUN)) &&
		!(sd->state.storage_flag && (inf&INF_SELF_SKILL))) //SELF skills can be used with the storage open, issue: 8027
		return;

	if( pc_issit(sd) )
		return;

	if( skill_isNotOk(skill_id, *sd) )
		return;

	if( sd->bl.id != target_id && inf&INF_SELF_SKILL )
		target_id = sd->bl.id; // never trust the client

	if( target_id < 0 && -target_id == sd->bl.id ) // for disguises [Valaris]
		target_id = sd->bl.id;

	if( sd->ud.skilltimer != INVALID_TIMER ) {
		if( skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST )
			return;
	} else if( DIFF_TICK(tick, sd->ud.canact_tick) < 0 ) {
		if( sd->skillitem != skill_id ) {
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_SKILLINTERVAL );
			return;
		}
	}

	if( sd->sc.option&OPTION_COSTUME )
		return;

#ifndef RENEWAL
	if( sd->sc.getSCE(SC_BASILICA) && (skill_id != HP_BASILICA || sd->sc.getSCE(SC_BASILICA)->val4 != sd->bl.id) )
		return; // On basilica only caster can use Basilica again to stop it.
#endif

	if( sd->menuskill_id ) {
		if( sd->menuskill_id == SA_TAMINGMONSTER ) {
			clif_menuskill_clear(sd); //Cancel pet capture.
		}else if( sd->menuskill_id == SG_FEEL ){
			clif_menuskill_clear( sd ); // Cancel selection
		}else if( sd->menuskill_id != SA_AUTOSPELL )
			return; //Can't use skills while a menu is open.
	}

	if( sd->skillitem == skill_id ) {
		if( skill_lv != sd->skillitemlv )
			skill_lv = sd->skillitemlv;
		if( !(inf&INF_SELF_SKILL) )
			pc_delinvincibletimer(sd); // Target skills thru items cancel invincibility. [Inkfish]
		unit_skilluse_id(&sd->bl, target_id, skill_id, skill_lv);
		return;
	}
	sd->skillitem = sd->skillitemlv = 0;

	if( SKILL_CHK_GUILD(skill_id) ) {
		if (sd->guild && sd->state.gmaster_flag || skill_id == GD_CHARGESHOUT_BEATING)
			skill_lv = guild_checkskill(sd->guild->guild, skill_id);
		else
			skill_lv = 0;
	} else {
		if( skill_id != ALL_EQSWITCH ){
			skill_lv = min(pc_checkskill(sd, skill_id),skill_lv); //never trust client
		}
	}

	pc_delinvincibletimer(sd);

	if( skill_lv )
		unit_skilluse_id(&sd->bl, target_id, skill_id, skill_lv);
}


/// Request to use a targeted skill.
/// 0113 <skill lv>.W <skill id>.W <target id>.L (CZ_USE_SKILL)
/// 0438 <skill lv>.W <skill id>.W <target id>.L (CZ_USE_SKILL2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_UseSkillToId( int fd, map_session_data *sd ){
	struct s_packet_db* info = &packet_db[RFIFOW(fd, 0)];

	clif_parse_skill_toid( sd, RFIFOW(fd, info->pos[1]), RFIFOW(fd, info->pos[0]), RFIFOL(fd, info->pos[2]) );
}

/*==========================================
 * Client tells server he'd like to use AoE skill id 'skill_id' of level 'skill_lv' on 'x','y' location
 *------------------------------------------*/
static void clif_parse_UseSkillToPosSub( int fd, map_session_data& sd, uint16 skill_lv, uint16 skill_id, short x, short y, int skillmoreinfo ){
	t_tick tick = gettick();

	if( !(skill_get_inf(skill_id)&INF_GROUND_SKILL) )
		return; //Using a target skill on the ground? WRONG.

	if (sd.state.block_action & PCBLOCK_SKILL) {
		clif_msg(&sd, MSI_BUSY);
		return;
	}

	if( SKILL_CHK_HOMUN(skill_id) ) {
		clif_parse_UseSkillToPos_homun(sd.hd, &sd, tick, skill_id, skill_lv, x, y, skillmoreinfo);
		return;
	}

	if( SKILL_CHK_MERC(skill_id) ) {
		clif_parse_UseSkillToPos_mercenary(sd.md, &sd, tick, skill_id, skill_lv, x, y, skillmoreinfo);
		return;
	}

	if( pc_hasprogress( &sd, WIP_DISABLE_SKILLITEM ) ){
#ifdef RENEWAL
		clif_msg( &sd, MSI_BUSY);
#endif
		return;
	}

	//Whether skill fails or not is irrelevant, the char ain't idle. [Skotlex]
	if (battle_config.idletime_option&IDLE_USESKILLTOPOS)
		sd.idletime = last_tick;
	if (battle_config.hom_idle_no_share && sd.hd && battle_config.idletime_hom_option&IDLE_USESKILLTOPOS)
		sd.idletime_hom = last_tick;
	if (battle_config.mer_idle_no_share && sd.md && battle_config.idletime_mer_option&IDLE_USESKILLTOPOS)
		sd.idletime_mer = last_tick;

	if( skill_isNotOk(skill_id, sd) )
		return;
	if( skillmoreinfo != -1 ) {
		if( pc_issit(&sd) ) {
			clif_skill_fail( sd, skill_id );
			return;
		}
		//You can't use Graffiti/TalkieBox AND have a vending open, so this is safe.
		safestrncpy(sd.message, RFIFOCP(fd,skillmoreinfo), MESSAGE_SIZE);
	}

	if( sd.ud.skilltimer != INVALID_TIMER )
		return;

	if( DIFF_TICK(tick, sd.ud.canact_tick) < 0 ) {
		if( sd.skillitem != skill_id ) {
			clif_skill_fail( sd, skill_id, USESKILL_FAIL_SKILLINTERVAL );
			return;
		}
	}

	if( sd.sc.option&OPTION_COSTUME )
		return;

#ifndef RENEWAL
	if( sd.sc.getSCE(SC_BASILICA) && (skill_id != HP_BASILICA || sd.sc.getSCE(SC_BASILICA)->val4 != sd.bl.id) )
		return; // On basilica only caster can use Basilica again to stop it.
#endif

	if( sd.menuskill_id ) {
		if( sd.menuskill_id == SA_TAMINGMONSTER ) {
			clif_menuskill_clear(&sd); //Cancel pet capture.
		}else if( sd.menuskill_id == SG_FEEL ){
			clif_menuskill_clear( &sd ); // Cancel selection
		} else if( sd.menuskill_id != SA_AUTOSPELL )
			return; //Can't use skills while a menu is open.
	}

	pc_delinvincibletimer(&sd);

	if( sd.skillitem == skill_id ) {
		if( skill_lv != sd.skillitemlv )
			skill_lv = sd.skillitemlv;
		unit_skilluse_pos(&sd.bl, x, y, skill_id, skill_lv);
	} else {
		int lv;
		sd.skillitem = sd.skillitemlv = 0;
		if( (lv = pc_checkskill(&sd, skill_id)) > 0 ) {
			if( skill_lv > lv )
				skill_lv = lv;
			unit_skilluse_pos(&sd.bl, x, y, skill_id,skill_lv);
		}
	}
}


/// Request to use a ground skill.
/// 0116 <skill lv>.W <skill id>.W <x>.W <y>.W (CZ_USE_SKILL_TOGROUND)
/// 0366 <skill lv>.W <skill id>.W <x>.W <y>.W (CZ_USE_SKILL_TOGROUND2)
/// 0AF4 <skill lv>.W <skill id>.W <x>.W <y>.W <unknown>.B (CZ_USE_SKILL_TOGROUND3)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_UseSkillToPos(int fd, map_session_data *sd)
{
	if( sd == nullptr ){
		return;
	}

	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	if (pc_cant_act(sd))
		return;
	if (pc_issit(sd))
		return;

	clif_parse_UseSkillToPosSub(fd, *sd,
		RFIFOW(fd,info->pos[0]), //skill lv
		RFIFOW(fd,info->pos[1]), //skill num
		RFIFOW(fd,info->pos[2]), //pos x
		RFIFOW(fd,info->pos[3]), //pos y
		// TODO: find out what this is intended to do
		//RFIFOB(fd, info->pos[4])
		-1	//Skill more info.
	);
}


/// Request to use a ground skill with text.
/// 0190 <skill lv>.W <skill id>.W <x>.W <y>.W <contents>.80B (CZ_USE_SKILL_TOGROUND_WITHTALKBOX)
/// 0367 <skill lv>.W <skill id>.W <x>.W <y>.W <contents>.80B (CZ_USE_SKILL_TOGROUND_WITHTALKBOX2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_UseSkillToPosMoreInfo(int fd, map_session_data *sd)
{
	if( sd == nullptr ){
		return;
	}

	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	if (pc_cant_act(sd))
		return;
	if (pc_issit(sd))
		return;

	clif_parse_UseSkillToPosSub(fd, *sd,
		RFIFOW(fd,info->pos[0]), //Skill lv
		RFIFOW(fd,info->pos[1]), //Skill num
		RFIFOW(fd,info->pos[2]), //pos x
		RFIFOW(fd,info->pos[3]), //pos y
		info->pos[4] //skill more info
	);
}


/// Answer to map selection dialog (CZ_SELECT_WARPPOINT).
/// 011b <skill id>.W <map name>.16B
void clif_parse_UseSkillMap(int fd, map_session_data* sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	uint16 skill_id = RFIFOW(fd,info->pos[0]);
	char map_name[MAP_NAME_LENGTH];

	mapindex_getmapname(RFIFOCP(fd,info->pos[1]), map_name);
	sd->state.workinprogress = WIP_DISABLE_NONE;

	if(skill_id != sd->menuskill_id)
		return;

	//It is possible to use teleport with the storage window open bugreport:8027
	if (pc_cant_act(sd) && !sd->state.storage_flag && skill_id != AL_TELEPORT) {
		clif_menuskill_clear(sd);
		return;
	}

	pc_delinvincibletimer(sd);
	skill_castend_map(sd,skill_id,map_name);
}


/// Request to set a memo on current map (CZ_REMEMBER_WARPPOINT).
/// 011d
void clif_parse_RequestMemo(int fd,map_session_data *sd)
{
	if (!pc_isdead(sd))
		pc_memo(sd,-1);
}


/// Answer to pharmacy item selection dialog.
/// 018e <name id>.W { <material id>.W }*3 (CZ_REQMAKINGITEM)
void clif_parse_ProduceMix(int fd,map_session_data *sd){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQMAKINGITEM* p = reinterpret_cast<PACKET_CZ_REQMAKINGITEM*>( RFIFOP( fd, 0 ) );

	switch( sd->menuskill_id ) {
		case -1:
		case AM_PHARMACY:
		case RK_RUNEMASTERY:
		case GC_RESEARCHNEWPOISON:
			break;
		default:
			return;
	}
	if (pc_istrading(sd)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif_skill_fail( *sd, sd->ud.skill_id );
		clif_menuskill_clear(sd);
		return;
	}

	int produce_idx;

	if( (produce_idx = skill_can_produce_mix(sd,p->itemId,sd->menuskill_val, 1)) )
		skill_produce_mix(sd,0,p->itemId,p->material[0],p->material[1],p->material[2],1,produce_idx-1);
	clif_menuskill_clear(sd);
}


/// Answer to mixing item selection dialog.
/// 025b <mk type>.W <name id>.W (CZ_REQ_MAKINGITEM)
/// mk type:
///     1 = cooking
///     2 = arrow
///     3 = elemental
///     4 = GN_MIX_COOKING
///     5 = GN_MAKEBOMB
///     6 = GN_S_PHARMACY
///     7 = MT_M_MACHINE - Unconfirmed
///     8 = BO_BIONIC_PHARMACY - Unconfirmed
void clif_parse_Cooking(int fd,map_session_data *sd) {
#if PACKETVER >= 20051010
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_MAKINGITEM* p = reinterpret_cast<PACKET_CZ_REQ_MAKINGITEM*>( RFIFOP( fd, 0 ) );

	int amount = sd->menuskill_val2 ? sd->menuskill_val2 : 1;
	short food_idx = -1;

	if( p->type == 6 && sd->menuskill_id != GN_MIX_COOKING && sd->menuskill_id != GN_S_PHARMACY )
		return;

	if (pc_istrading(sd)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif_skill_fail( *sd, sd->ud.skill_id );
		clif_menuskill_clear(sd);
		return;
	}
	if( (food_idx = skill_can_produce_mix(sd,p->itemId,sd->menuskill_val, amount)) )
		skill_produce_mix(sd,(p->type>1?sd->menuskill_id:0),p->itemId,0,0,0,amount,food_idx-1);
	clif_menuskill_clear(sd);
#endif
}


/// Answer to repair weapon item selection dialog
/// 01fd <index> W (CZ_REQ_ITEMREPAIR)
/// 01fd <index>.W <name id>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W ???
void clif_parse_RepairItem( int fd, map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

// Hercules has wrong date -> use correct one here
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	const PACKET_CZ_REQ_ITEMREPAIR2* p = reinterpret_cast<PACKET_CZ_REQ_ITEMREPAIR2*>( RFIFOP( fd, 0 ) );
#else
	const PACKET_CZ_REQ_ITEMREPAIR1* p = reinterpret_cast<PACKET_CZ_REQ_ITEMREPAIR1*>( RFIFOP( fd, 0 ) );
#endif

	if (sd->menuskill_id != BS_REPAIRWEAPON)
		return;
	if (pc_istrading(sd)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif_skill_fail( *sd, sd->ud.skill_id );
		clif_menuskill_clear(sd);
		return;
	}
	skill_repairweapon( *sd, p->item.index );
	clif_menuskill_clear(sd);
}


/// Answer to refine weapon item selection dialog (CZ_REQ_WEAPONREFINE).
/// 0222 <index>.L
void clif_parse_WeaponRefine( int fd, map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

	if (sd->menuskill_id != WS_WEAPONREFINE) //Packet exploit?
		return;
	if (pc_istrading(sd)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif_skill_fail( *sd, sd->ud.skill_id );
		clif_menuskill_clear(sd);
		return;
	}
	skill_weaponrefine( *sd, server_index( RFIFOL( fd, 2 ) ) );
	clif_menuskill_clear(sd);
}


/// Answer to script menu dialog (CZ_CHOOSE_MENU).
/// 00b8 <npc id>.L <choice>.B
/// choice:
///     1~254 = menu item
///     255   = cancel
/// NOTE: If there were more than 254 items in the list, choice
///     overflows to choice%256.
void clif_parse_NpcSelectMenu(int fd,map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int npc_id = RFIFOL(fd,info->pos[0]);
	uint8 select = RFIFOB(fd,info->pos[1]);

#ifdef SECURE_NPCTIMEOUT
	if( sd->npc_idle_timer == INVALID_TIMER && !sd->state.ignoretimeout )
		return;
#endif

	if( (select > sd->npc_menu && select != 0xff) || select == 0 ) {
			TBL_NPC* nd = map_id2nd(npc_id);
			ShowWarning("Invalid menu selection on npc %d:'%s' - got %d, valid range is [%d..%d] (player AID:%d, CID:%d, name:'%s')!\n", npc_id, (nd)?nd->name:"invalid npc id", select, 1, sd->npc_menu, sd->bl.id, sd->status.char_id, sd->status.name);
			clif_GM_kick(nullptr,sd);
		return;
	}

	sd->npc_menu = select;

	if( battle_config.idletime_option&IDLE_NPC_MENU ){
		sd->idletime = last_tick;
	}

	npc_scriptcont(sd,npc_id, false);
}


/// NPC dialog 'next' click (CZ_REQ_NEXT_SCRIPT).
/// 00b9 <npc id>.L
void clif_parse_NpcNextClicked(int fd,map_session_data *sd)
{
	if( battle_config.idletime_option&IDLE_NPC_NEXT ){
		sd->idletime = last_tick;
	}

	npc_scriptcont(sd,RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]), false);
}


/// NPC numeric input dialog value (CZ_INPUT_EDITDLG).
/// 0143 <npc id>.L <value>.L
void clif_parse_NpcAmountInput(int fd,map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int npcid = RFIFOL(fd,info->pos[0]);
	int amount = (int)RFIFOL(fd,info->pos[1]);

	sd->npc_amount = amount;

	if( battle_config.idletime_option&IDLE_NPC_INPUT ){
		sd->idletime = last_tick;
	}

	npc_scriptcont(sd, npcid, false);
}


/// NPC text input dialog value (CZ_INPUT_EDITDLGSTR).
/// 01d5 <packet len>.W <npc id>.L <string>.?B
void clif_parse_NpcStringInput(int fd, map_session_data* sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int message_len = RFIFOW(fd,info->pos[0])-8;
	int npcid = RFIFOL(fd,info->pos[1]);
	const char* message = RFIFOCP(fd,info->pos[2]);

	if( message_len <= 0 )
		return; // invalid input

#if PACKETVER >= 20151001
	message_len++;
#endif

	safestrncpy(sd->npc_str, message, min(message_len,CHATBOX_SIZE));

	if( battle_config.idletime_option&IDLE_NPC_INPUT ){
		sd->idletime = last_tick;
	}

	npc_scriptcont(sd, npcid, false);
}


/// NPC dialog 'close' click (CZ_CLOSE_DIALOG).
/// 0146 <npc id>.L
void clif_parse_NpcCloseClicked(int fd,map_session_data *sd)
{
	if (!sd->npc_id) //Avoid parsing anything when the script was done with. [Skotlex]
		return;

	if( battle_config.idletime_option&IDLE_NPC_CLOSE ){
		sd->idletime = last_tick;
	}

	npc_scriptcont(sd, RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]), true);
}


/// Answer to identify item selection dialog (CZ_REQ_ITEMIDENTIFY).
/// 0178 <index>.W
/// index:
///     -1 = cancel
void clif_parse_ItemIdentify(int fd,map_session_data *sd) {
	short idx = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]) - 2;

	if (sd->menuskill_id != MC_IDENTIFY)
		return;

	// Ignore the request
	// - Invalid item index
	// - Invalid item ID or item doesn't exist
	// - Item is already identified
	if (idx < 0 || idx >= MAX_INVENTORY ||
		sd->inventory.u.items_inventory[idx].nameid == 0 || sd->inventory_data[idx] == nullptr ||
		sd->inventory.u.items_inventory[idx].identify) {// cancel pressed
			sd->state.workinprogress = WIP_DISABLE_NONE;
			clif_menuskill_clear(sd);
			return;
	}

	skill_identify(sd, idx);
	clif_menuskill_clear(sd);
}


/// Answer to arrow crafting item selection dialog.
/// 01ae <name id>.W (CZ_REQ_MAKINGARROW)
void clif_parse_SelectArrow(int fd,map_session_data *sd) {
	if( sd == nullptr ){
		return;
	}

	if (pc_istrading(sd)) {
		//Make it fail to avoid shop exploits where you sell something different than you see.
		clif_skill_fail( *sd, sd->ud.skill_id );
		clif_menuskill_clear(sd);
		return;
	}

	const PACKET_CZ_REQ_MAKINGARROW* p = reinterpret_cast<PACKET_CZ_REQ_MAKINGARROW*>( RFIFOP( fd, 0 ) );

	switch (sd->menuskill_id) {
		case AC_MAKINGARROW:
			skill_arrow_create(sd,p->itemId);
			break;
		case SA_CREATECON:
			skill_produce_mix(sd,SA_CREATECON,p->itemId,0,0,0,1,-1);
			break;
		case GC_POISONINGWEAPON:
			skill_poisoningweapon(*sd,p->itemId);
			break;
		case NC_MAGICDECOY:
			skill_magicdecoy(*sd,p->itemId);
			break;
	}

	clif_menuskill_clear(sd);
}


/// Answer to SA_AUTOSPELL skill selection dialog (CZ_SELECTAUTOSPELL).
/// 01ce <skill id>.L
void clif_parse_AutoSpell(int fd,map_session_data *sd)
{
	if (sd->menuskill_id != SA_AUTOSPELL)
		return;
	sd->state.workinprogress = WIP_DISABLE_NONE;
	skill_autospell(sd,RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));
	clif_menuskill_clear(sd);
}


/// Request to display item carding/composition list (CZ_REQ_ITEMCOMPOSITION_LIST).
/// 017a <card index>.W
void clif_parse_UseCard(int fd,map_session_data *sd)
{
	if (sd->state.trading != 0)
		return;
	clif_use_card(sd,RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-2);
}


/// Answer to carding/composing item selection dialog (CZ_REQ_ITEMCOMPOSITION).
/// 017c <card index>.W <equip index>.W
void clif_parse_InsertCard(int fd,map_session_data *sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	if (sd->state.trading != 0)
		return;
	pc_insert_card(sd,RFIFOW(fd,info->pos[0])-2,RFIFOW(fd,info->pos[1])-2);
}


/// Request of character's name by char ID.
/// 0193 <char id>.L (CZ_REQNAME_BYGID)
/// 0369 <char id>.L (CZ_REQNAME_BYGID2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_SolveCharName(int fd, map_session_data *sd)
{
	int charid;

	charid = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	map_reqnickdb(sd, charid);
}


/// /resetskill /resetstate (CZ_RESET).
/// Request to reset stats or skills.
/// 0197 <type>.W
/// type:
///     0 = state
///     1 = skill
void clif_parse_ResetChar(int fd, map_session_data *sd) {
	char cmd[15];

	if( RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]) )
		safesnprintf(cmd,sizeof(cmd),"%cresetskill",atcommand_symbol);
	else
		safesnprintf(cmd,sizeof(cmd),"%cresetstat",atcommand_symbol);

	is_atcommand(fd, sd, cmd, 1);
}


/// /lb /nlb (CZ_LOCALBROADCAST).
/// Request to broadcast a message on current map.
/// 019c <packet len>.W <text>.?B
void clif_parse_LocalBroadcast(int fd, map_session_data* sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	char command[CHAT_SIZE_MAX+16];
	unsigned int len = RFIFOW(fd,info->pos[0])-4;
	char* msg = RFIFOCP(fd,info->pos[1]);

	// as the length varies depending on the command used, just block unreasonably long strings
	mes_len_check(msg, len, CHAT_SIZE_MAX);

	safesnprintf(command,sizeof(command),"%clkami %s", atcommand_symbol, msg);
	is_atcommand(fd, sd, command, 1);
}


/// Request to move an item from inventory to storage.
/// 00f3 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_BODY_TO_STORE)
/// 0364 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_BODY_TO_STORE2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_MoveToKafra(int fd, map_session_data *sd)
{
	int item_index, item_amount;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	if (pc_istrading(sd))
		return;

	item_index = RFIFOW(fd,info->pos[0])-2;
	item_amount = RFIFOL(fd,info->pos[1]);
	if (item_index < 0 || item_index >= MAX_INVENTORY || item_amount < 1)
		return;
	if( sd->inventory.u.items_inventory[item_index].equipSwitch ){
		clif_msg( sd, MSI_SWAP_EQUIPITEM_UNREGISTER_FIRST );
		return;
	}

	if (sd->state.storage_flag == 1)
		storage_storageadd(sd, &sd->storage, item_index, item_amount);
	else
	if (sd->state.storage_flag == 2)
		storage_guild_storageadd(sd, item_index, item_amount);
	else if (sd->state.storage_flag == 3)
		storage_storageadd(sd, &sd->premiumStorage, item_index, item_amount);
}


/// Request to move an item from storage to inventory.
/// 00f5 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_STORE_TO_BODY)
/// 0365 <index>.W <amount>.L (CZ_MOVE_ITEM_FROM_STORE_TO_BODY2)
/// There are various variants of this packet, some of them have padding between fields.
void clif_parse_MoveFromKafra(int fd,map_session_data *sd)
{
	int item_index, item_amount;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	item_index = RFIFOW(fd,info->pos[0])-1;
	item_amount = RFIFOL(fd,info->pos[1]);

	if (sd->state.storage_flag == 1)
		storage_storageget(sd, &sd->storage, item_index, item_amount);
	else if(sd->state.storage_flag == 2)
		storage_guild_storageget(sd, item_index, item_amount);
	else if(sd->state.storage_flag == 3)
		storage_storageget(sd, &sd->premiumStorage, item_index, item_amount);
}


/// Request to move an item from cart to storage (CZ_MOVE_ITEM_FROM_CART_TO_STORE).
/// 0129 <index>.W <amount>.L
void clif_parse_MoveToKafraFromCart(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	int idx = RFIFOW(fd,info->pos[0]) - 2;
	int amount = RFIFOL(fd,info->pos[1]);

	if( sd->state.vending )
		return;
	if (!pc_iscarton(sd))
		return;
	if (map_getmapflag(sd->bl.m, MF_NOUSECART))
		return;

	if (idx < 0 || idx >= MAX_INVENTORY || amount < 1)
		return;
	if( sd->inventory.u.items_inventory[idx].equipSwitch ){
		clif_msg( sd, MSI_SWAP_EQUIPITEM_UNREGISTER_FIRST );
		return;
	}

	if (sd->state.storage_flag == 1)
		storage_storageaddfromcart(sd, &sd->storage, idx, amount);
	else if (sd->state.storage_flag == 2)
		storage_guild_storageaddfromcart(sd, idx, amount);
	else if (sd->state.storage_flag == 3)
		storage_storageaddfromcart(sd, &sd->premiumStorage, idx, amount);
}


/// Request to move an item from storage to cart (CZ_MOVE_ITEM_FROM_STORE_TO_CART).
/// 0128 <index>.W <amount>.L
void clif_parse_MoveFromKafraToCart(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int idx = RFIFOW(fd,info->pos[0]) - 1;
	int amount = RFIFOL(fd,info->pos[1]);

	if( sd->state.vending )
		return;
	if (!pc_iscarton(sd))
		return;
	if (map_getmapflag(sd->bl.m, MF_NOUSECART))
		return;

	if (sd->state.storage_flag == 1)
		storage_storagegettocart(sd, &sd->storage, idx, amount);
	else
	if (sd->state.storage_flag == 2)
		storage_guild_storagegettocart(sd, idx, amount);
	else if (sd->state.storage_flag == 3)
		storage_storagegettocart(sd, &sd->premiumStorage, idx, amount);
}


/// Request to close storage (CZ_CLOSE_STORE).
/// 00f7
void clif_parse_CloseKafra(int fd, map_session_data *sd)
{
	if( sd->state.storage_flag == 1 )
		storage_storageclose(sd);
	else
	if( sd->state.storage_flag == 2 )
		storage_guild_storageclose(sd);
	else if( sd->state.storage_flag == 3 )
		storage_premiumStorage_close(sd);
}


/// Displays kafra storage password dialog (ZC_REQ_STORE_PASSWORD).
/// 023a <info>.W
/// info:
///     0 = password has not been set yet
///     1 = storage is password-protected
///     8 = too many wrong passwords
///     ? = ignored
/// NOTE: This packet is only available on certain non-kRO clients.
void clif_storagepassword(map_session_data* sd, short info)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x23a));
	WFIFOW(fd,0) = 0x23a;
	WFIFOW(fd,2) = info;
	WFIFOSET(fd,packet_len(0x23a));
}


/// Answer to the kafra storage password dialog (CZ_ACK_STORE_PASSWORD).
/// 023b <type>.W <password>.16B <new password>.16B
/// type:
///     2 = change password
///     3 = check password
/// NOTE: This packet is only available on certain non-kRO clients.
void clif_parse_StoragePassword(int fd, map_session_data *sd){ //@TODO
//	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
//	int type = RFIFOW(fd,info->pos[0]);
//	char* password = RFIFOP(fd,info->pos[1]);
//	char* new_password = RFIFOP(fd,info->pos[2]);
//	storage_changepass(sd,password,new_password);

}


/// Result of kafra storage password validation (ZC_RESULT_STORE_PASSWORD).
/// 023c <result>.W <error count>.W
/// result:
///     4 = password change success
///     5 = password change failure
///     6 = password check success
///     7 = password check failure
///     8 = too many wrong passwords
///     ? = ignored
/// NOTE: This packet is only available on certain non-kRO clients.
void clif_storagepassword_result(map_session_data* sd, short result, short error_count)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x23c));
	WFIFOW(fd,0) = 0x23c;
	WFIFOW(fd,2) = result;
	WFIFOW(fd,4) = error_count;
	WFIFOSET(fd,packet_len(0x23c));
}


/// Party creation request
/// 00f9 <party name>.24B (CZ_MAKE_GROUP)
void clif_parse_CreateParty(int fd, map_session_data *sd){
	if( sd == nullptr ){
		return;
	}

	char* name = RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	name[NAME_LENGTH-1] = '\0';

	if( map_getmapflag(sd->bl.m, MF_PARTYLOCK) ) {// Party locked.
		clif_displaymessage(fd, msg_txt(sd,227));
		return;
	}
	if( battle_config.basic_skill_check && pc_checkskill(sd,NV_BASIC) < 7 && pc_checkskill(sd, SU_BASIC_SKILL) < 1 ) {
		clif_skill_fail( *sd, 1, USESKILL_FAIL_LEVEL, 4 );
		return;
	}

	party_create( *sd, name, 0, 0 );
}

/// 01e8 <party name>.24B <item pickup rule>.B <item share rule>.B (CZ_MAKE_GROUP2)
void clif_parse_CreateParty2(int fd, map_session_data *sd){
	if( sd == nullptr ){
		return;
	}

	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	char* name = RFIFOCP(fd,info->pos[0]);
	int item1 = RFIFOB(fd,info->pos[1]);
	int item2 = RFIFOB(fd,info->pos[2]);
	name[NAME_LENGTH-1] = '\0';

	if( map_getmapflag(sd->bl.m, MF_PARTYLOCK) ) {// Party locked.
		clif_displaymessage(fd, msg_txt(sd,227));
		return;
	}
	if( battle_config.basic_skill_check && pc_checkskill(sd,NV_BASIC) < 7 && pc_checkskill(sd, SU_BASIC_SKILL) < 1 ) {
		clif_skill_fail( *sd, 1, USESKILL_FAIL_LEVEL, 4 );
		return;
	}

	party_create( *sd, name, item1, item2 );
}


/// Party invitation request by account id
/// 00fc <account id>.L (CZ_REQ_JOIN_GROUP)
void clif_parse_PartyInvite( int fd, map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_JOIN_GROUP* p = reinterpret_cast<PACKET_CZ_REQ_JOIN_GROUP*>( RFIFOP( fd, 0 ) );

	party_invite( *sd, map_id2sd( p->AID ) );
}

/// Party invitation request by name
/// 02c4 <char name>.24B (CZ_PARTY_JOIN_REQ)
void clif_parse_PartyInvite2( int fd, map_session_data *sd ){
#if PACKETVER >= 20070227
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_PARTY_JOIN_REQ* p = reinterpret_cast<PACKET_CZ_PARTY_JOIN_REQ*>( RFIFOP( fd, 0 ) );

	char name[NAME_LENGTH];

	safestrncpy( name, p->name, sizeof( name ) );

	party_invite( *sd, map_nick2sd( name, false ) );
#endif
}


/// Party invitation reply
/// 00ff <party id>.L <flag>.L (CZ_JOIN_GROUP)
/// flag:
///     0 = reject
///     1 = accept
void clif_parse_ReplyPartyInvite( int fd, map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_JOIN_GROUP* p = reinterpret_cast<PACKET_CZ_JOIN_GROUP*>( RFIFOP( fd, 0 ) );

	party_reply_invite( *sd, p->party_id, p->flag );
}

/// Party invitation reply
/// 02c7 <party id>.L <flag>.B (CZ_PARTY_JOIN_REQ_ACK)
/// flag:
///     0 = reject
///     1 = accept
void clif_parse_ReplyPartyInvite2( int fd, map_session_data *sd ){
#if PACKETVER >= 20070227
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_PARTY_JOIN_REQ_ACK* p = reinterpret_cast<PACKET_CZ_PARTY_JOIN_REQ_ACK*>( RFIFOP( fd, 0 ) );

	party_reply_invite( *sd, p->party_id, p->flag );
#endif
}


/// Request to leave party.
/// 0100 (CZ_REQ_LEAVE_GROUP)
void clif_parse_LeaveParty( int fd, map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

	//const PACKET_CZ_REQ_LEAVE_GROUP* p = reinterpret_cast<PACKET_CZ_REQ_LEAVE_GROUP*>( RFIFOP( fd, 0 ) );

	party_leave( *sd, true );
}


/// Request to expel a party member.
/// 0103 <account id>.L <char name>.24B (CZ_REQ_EXPEL_GROUP_MEMBER)
void clif_parse_RemovePartyMember( int fd, map_session_data* sd ){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_EXPEL_GROUP_MEMBER* p = reinterpret_cast<PACKET_CZ_REQ_EXPEL_GROUP_MEMBER*>( RFIFOP( fd, 0 ) );

	party_removemember( *sd, p->AID, p->name );
}


/// Request to change party options.
/// 0102 <exp share rule>.L (CZ_CHANGE_GROUPEXPOPTION)
/// 07d7 <exp share rule>.L <item pickup rule>.B <item share rule>.B (CZ_GROUPINFO_CHANGE_V2)
void clif_parse_PartyChangeOption(int fd, map_session_data *sd)
{
	struct party_data *p;
	int i,expflag;
	int cmd = RFIFOW(fd,0);
	struct s_packet_db* info = &packet_db[cmd];

	if( !sd->status.party_id )
		return;

	p = party_search(sd->status.party_id);
	if( p == nullptr )
		return;
	ARR_FIND( 0, MAX_PARTY, i, p->data[i].sd == sd );
	if( i == MAX_PARTY )
		return; //Shouldn't happen
	if( !p->party.member[i].leader )
		return;

	expflag = RFIFOL(fd,info->pos[0]);
	if(cmd == 0x0102){ //Client can't change the item-field
		party_changeoption(sd, expflag, p->party.item);
	}
	else {
		int itemflag = (RFIFOB(fd,info->pos[1])?1:0)|(RFIFOB(fd,info->pos[2])?2:0);
		party_changeoption(sd, expflag, itemflag);
	}
}


/// Validates and processes party messages (CZ_REQUEST_CHAT_PARTY).
/// 0108 <packet len>.W <text>.?B (<name> : <message>) 00
void clif_parse_PartyMessage(int fd, map_session_data* sd){
	char name[NAME_LENGTH], message[CHAT_SIZE_MAX], output[CHAT_SIZE_MAX+NAME_LENGTH*2];

	// validate packet and retrieve name and message
	if( !clif_process_message( sd, false, name, message, output ) )
		return;

	party_send_message(sd, output, strlen(output) + 1 );
}


/// Changes Party Leader (CZ_CHANGE_GROUP_MASTER).
/// 07da <account id>.L
void clif_parse_PartyChangeLeader(int fd, map_session_data* sd){
	party_changeleader(sd, map_id2sd(RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0])),nullptr);
}


/// Party Booking in KRO [Spiria]
///

/// Request to register a party booking advertisment (CZ_PARTY_BOOKING_REQ_REGISTER).
/// 0802 <level>.W <map id>.W { <job>.W }*6
void clif_parse_PartyBookingRegisterReq(int fd, map_session_data* sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	short level = RFIFOW(fd,info->pos[0]);
	short mapid = RFIFOW(fd,info->pos[1]);
	int idxpbj = info->pos[2];
	short job[MAX_PARTY_BOOKING_JOBS];
	int i;

	for(i=0; i<MAX_PARTY_BOOKING_JOBS; i++)
		job[i] = RFIFOB(fd,idxpbj+i*2);

	party_booking_register(sd, level, mapid, job);
}


/// Result of request to register a party booking advertisment (ZC_PARTY_BOOKING_ACK_REGISTER).
/// 0803 <result>.W
/// result:
///     0 = success
///     1 = failure
///     2 = already registered
void clif_PartyBookingRegisterAck(map_session_data *sd, int flag)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x803));
	WFIFOW(fd,0) = 0x803;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x803));
}


/// Request to search for party booking advertisments (CZ_PARTY_BOOKING_REQ_SEARCH).
/// 0804 <level>.W <map id>.W <job>.W <last index>.L <result count>.W
void clif_parse_PartyBookingSearchReq(int fd, map_session_data* sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	short level = RFIFOW(fd,info->pos[0]);
	short mapid = RFIFOW(fd,info->pos[1]);
	short job = RFIFOW(fd,info->pos[2]);
	unsigned long lastindex = RFIFOL(fd,info->pos[3]);
	short resultcount = RFIFOW(fd,info->pos[4]);

	party_booking_search(sd, level, mapid, job, lastindex, resultcount);
}


/// Party booking search results (ZC_PARTY_BOOKING_ACK_SEARCH).
/// 0805 <packet len>.W <more results>.B { <index>.L <char name>.24B <expire time>.L <level>.W <map id>.W { <job>.W }*6 }*
/// more results:
///     0 = no
///     1 = yes
void clif_PartyBookingSearchAck(int fd, struct party_booking_ad_info** results, int count, bool more_result)
{
	int i, j;
	int size = sizeof(struct party_booking_ad_info); // structure size (48)
	struct party_booking_ad_info *pb_ad;
	WFIFOHEAD(fd,size*count + 5);
	WFIFOW(fd,0) = 0x805;
	WFIFOW(fd,2) = size*count + 5;
	WFIFOB(fd,4) = more_result;
	for(i=0; i<count; i++)
	{
		pb_ad = results[i];
		WFIFOL(fd,i*size+5) = pb_ad->index;
		safestrncpy(WFIFOCP(fd,i*size+9),pb_ad->charname,NAME_LENGTH);
		WFIFOL(fd,i*size+33) = pb_ad->starttime;  // FIXME: This is expire time
		WFIFOW(fd,i*size+37) = pb_ad->p_detail.level;
		WFIFOW(fd,i*size+39) = pb_ad->p_detail.mapid;
		for(j=0; j<MAX_PARTY_BOOKING_JOBS; j++)
			WFIFOW(fd,i*size+41+j*2) = pb_ad->p_detail.job[j];
	}
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Request to delete own party booking advertisment (CZ_PARTY_BOOKING_REQ_DELETE).
/// 0806
void clif_parse_PartyBookingDeleteReq(int fd, map_session_data* sd)
{
	if(party_booking_delete(sd))
		clif_PartyBookingDeleteAck(sd, 0);
}


/// Result of request to delete own party booking advertisment (ZC_PARTY_BOOKING_ACK_DELETE).
/// 0807 <result>.W
/// result:
///     0 = success
///     1 = success (auto-removed expired ad)
///     2 = failure
///     3 = nothing registered
void clif_PartyBookingDeleteAck(map_session_data* sd, int flag)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x807));
	WFIFOW(fd,0) = 0x807;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x807));
}


/// Request to update party booking advertisment (CZ_PARTY_BOOKING_REQ_UPDATE).
/// 0808 { <job>.W }*6
void clif_parse_PartyBookingUpdateReq(int fd, map_session_data* sd)
{
	short job[MAX_PARTY_BOOKING_JOBS];
	int i;
	int idxpbu = packet_db[RFIFOW(fd,0)].pos[0];

	for(i=0; i<MAX_PARTY_BOOKING_JOBS; i++)
		job[i] = RFIFOW(fd,idxpbu+i*2);

	party_booking_update(sd, job);
}


/// Notification about new party booking advertisment (ZC_PARTY_BOOKING_NOTIFY_INSERT).
/// 0809 <index>.L <char name>.24B <expire time>.L <level>.W <map id>.W { <job>.W }*6
void clif_PartyBookingInsertNotify(map_session_data* sd, struct party_booking_ad_info* pb_ad)
{
	int i;
	uint8 buf[38+MAX_PARTY_BOOKING_JOBS*2];

	if(pb_ad == nullptr) return;

	WBUFW(buf,0) = 0x809;
	WBUFL(buf,2) = pb_ad->index;
	safestrncpy(WBUFCP(buf,6),pb_ad->charname,NAME_LENGTH);
	WBUFL(buf,30) = pb_ad->starttime;  // FIXME: This is expire time
	WBUFW(buf,34) = pb_ad->p_detail.level;
	WBUFW(buf,36) = pb_ad->p_detail.mapid;
	for(i=0; i<MAX_PARTY_BOOKING_JOBS; i++)
		WBUFW(buf,38+i*2) = pb_ad->p_detail.job[i];

	clif_send(buf, packet_len(0x809), &sd->bl, ALL_CLIENT);
}


/// Notification about updated party booking advertisment (ZC_PARTY_BOOKING_NOTIFY_UPDATE).
/// 080a <index>.L { <job>.W }*6
void clif_PartyBookingUpdateNotify(map_session_data* sd, struct party_booking_ad_info* pb_ad)
{
	int i;
	uint8 buf[6+MAX_PARTY_BOOKING_JOBS*2];

	if(pb_ad == nullptr) return;

	WBUFW(buf,0) = 0x80a;
	WBUFL(buf,2) = pb_ad->index;
	for(i=0; i<MAX_PARTY_BOOKING_JOBS; i++)
		WBUFW(buf,6+i*2) = pb_ad->p_detail.job[i];
	clif_send(buf,packet_len(0x80a),&sd->bl,ALL_CLIENT); // Now UPDATE all client.
}


/// Notification about deleted party booking advertisment (ZC_PARTY_BOOKING_NOTIFY_DELETE).
/// 080b <index>.L
void clif_PartyBookingDeleteNotify(map_session_data* sd, int index)
{
	uint8 buf[6];

	WBUFW(buf,0) = 0x80b;
	WBUFL(buf,2) = index;

	clif_send(buf, packet_len(0x80b), &sd->bl, ALL_CLIENT); // Now UPDATE all client.
}


/// Request to close own vending (CZ_REQ_CLOSESTORE).
/// 012e
void clif_parse_CloseVending(int fd, map_session_data* sd)
{
	vending_closevending(sd);
}


/// Request to open a vending shop (CZ_REQ_BUY_FROMMC).
/// 0130 <account id>.L
void clif_parse_VendingListReq(int fd, map_session_data* sd)
{
	if( sd->npc_id ) {// using an NPC
		return;
	}
	vending_vendinglistreq(sd,RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// Shop item(s) purchase request.
/// 0134 <packet len>.W <account id>.L { <amount>.W <index>.W }* (CZ_PC_PURCHASE_ITEMLIST_FROMMC)
void clif_parse_PurchaseReq(int fd, map_session_data* sd){
	const PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC* p = reinterpret_cast<PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC*>( RFIFOP( fd, 0 ) );

	vending_purchasereq( sd, p->AID, sd->vended_id, (uint8*)p->list, ( p->packetLength - sizeof( *p ) ) / sizeof( p->list[0] ) );

	// whether it fails or not, the buy window is closed
	sd->vended_id = 0;
}


/// Shop item(s) purchase request.
/// 0801 <packet len>.W <account id>.L <unique id>.L { <amount>.W <index>.W }* (CZ_PC_PURCHASE_ITEMLIST_FROMMC2)
void clif_parse_PurchaseReq2(int fd, map_session_data* sd){
#if PACKETVER >= 20100105
	const PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC2* p = reinterpret_cast<PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC2*>( RFIFOP( fd, 0 ) );

	vending_purchasereq( sd, p->AID, p->UniqueID, (uint8*)p->list, ( p->packetLength - sizeof( *p ) ) / sizeof( p->list[0] ) );

	// whether it fails or not, the buy window is closed
	sd->vended_id = 0;
#endif
}


/// Confirm or cancel the shop preparation window.
/// 012f <packet len>.W <shop name>.80B { <index>.W <amount>.W <price>.L }* (CZ_REQ_OPENSTORE)
/// 01b2 <packet len>.W <shop name>.80B <result>.B { <index>.W <amount>.W <price>.L }* (CZ_REQ_OPENSTORE2)
/// result:
///     0 = canceled
///     1 = open
void clif_parse_OpenVending(int fd, map_session_data* sd){
	if( sd == nullptr ){
		return;
	}

	int cmd = RFIFOW(fd,0);
	struct s_packet_db* info = &packet_db[cmd];
	short len = (short)RFIFOW(fd,info->pos[0]);
	const char* message = RFIFOCP(fd,info->pos[1]);
	const uint8* data = (uint8*)RFIFOP(fd,info->pos[3]);

	if(cmd == 0x12f){ // (CZ_REQ_OPENSTORE)
		len -= 84;
	}
	else { //(CZ_REQ_OPENSTORE2)
		bool flag;

		len -= 85;
		flag = RFIFOB(fd,info->pos[2]) != 0;
		if (!flag) {
			sd->state.prevend = 0;
			sd->state.workinprogress = WIP_DISABLE_NONE;
		}
	}

	if( sd->sc.getSCE(SC_NOCHAT) && sd->sc.getSCE(SC_NOCHAT)->val1&MANNER_NOROOM )
		return;
	if( map_getmapflag(sd->bl.m, MF_NOVENDING) ) {
		clif_displaymessage (sd->fd, msg_txt(sd,276)); // "You can't open a shop on this map"
		return;
	}
	if( map_getcell(sd->bl.m,sd->bl.x,sd->bl.y,CELL_CHKNOVENDING) ) {
		clif_displaymessage (sd->fd, msg_txt(sd,204)); // "You can't open a shop on this cell."
		return;
	}

	if( message[0] == '\0' ) // invalid input
		return;

	vending_openvending(*sd, message, data, len/8, nullptr);
}


/// Guild creation request (CZ_REQ_MAKE_GUILD).
/// 0165 <char id>.L <guild name>.24B
void clif_parse_CreateGuild(int fd,map_session_data *sd){
	//int charid = RFIFOL(fd,packet_db[cmd].pos[0]);
	char* name = RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[1]);
	name[NAME_LENGTH-1] = '\0';

	if(map_getmapflag(sd->bl.m, MF_GUILDLOCK)) { //Guild locked.
		clif_displaymessage(fd, msg_txt(sd,228));
		return;
	}

	if(sd->clan){
		// Should display a clientside message "You are currently joined in Clan !!" so we ignore it
		return;
	}

	guild_create( *sd, name );
}


/// Request for guild window interface permissions (CZ_REQ_GUILD_MENUINTERFACE).
/// 014d
void clif_parse_GuildCheckMaster(int fd, map_session_data *sd)
{
	clif_guild_masterormember(sd);
}


/// Request for guild window information (CZ_REQ_GUILD_MENU).
/// 014f <type>.L
/// type:
///     0 = basic info
///     1 = member manager
///     2 = positions
///     3 = skills
///     4 = expulsion list
///     5 = unknown (GM_ALLGUILDLIST)
///     6 = notice
void clif_parse_GuildRequestInfo(int fd, map_session_data *sd)
{
	int type = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if( !sd->status.guild_id && !sd->bg_id )
		return;

	switch( type )
	{
	case 0:	// Basic Information Guild, hostile alliance information
		clif_guild_basicinfo( *sd );
		clif_guild_allianceinfo(sd);
		clif_guild_castle_list(*sd);
		break;
	case 1:	// Members list, list job title
		clif_guild_positionnamelist(sd);
		clif_guild_memberlist( *sd );
		break;
	case 2:	// List job title, title information list
		clif_guild_positionnamelist(sd);
		clif_guild_positioninfolist(sd);
		break;
	case 3:	// Skill list
		clif_guild_skillinfo( *sd );
		break;
	case 4:	// Expulsion list
		clif_guild_expulsionlist(sd);
		break;
	default:
		ShowError("clif: guild request info: unknown type %d\n", type);
		break;
	}
}


/// Request to update guild positions (CZ_REG_CHANGE_GUILD_POSITIONINFO).
/// 0161 <packet len>.W { <position id>.L <mode>.L <ranking>.L <pay rate>.L <name>.24B }*
void clif_parse_GuildChangePositionInfo(int fd, map_session_data *sd)
{
	int i;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int len = RFIFOW(fd,info->pos[0]);
	int idxgpos = info->pos[1];

	if(!sd->state.gmaster_flag)
		return;

	for(i = idxgpos; i < len; i += 40 ){
		guild_change_position(sd->status.guild_id, RFIFOL(fd,i), RFIFOL(fd,i+4), RFIFOL(fd,i+12), RFIFOCP(fd,i+16));
	}
}


/// Request to update the position of guild members.
/// 0155 <packet len>.W { <account id>.L <char id>.L <position id>.L }* (CZ_REQ_CHANGE_MEMBERPOS)
void clif_parse_GuildChangeMemberPosition( int fd, map_session_data *sd ){
	if(!sd->state.gmaster_flag)
		return;

	const PACKET_CZ_REQ_CHANGE_MEMBERPOS* p = reinterpret_cast<PACKET_CZ_REQ_CHANGE_MEMBERPOS*>( RFIFOP( fd, 0 ) );

	int16 entries = ( p->packetLength - sizeof( *p ) ) / sizeof( p->list[0] );

	for( int16 i = 0; i < entries; i++ ){
		const PACKET_CZ_REQ_CHANGE_MEMBERPOS_sub& entry = p->list[i];

		// Guild leadership change
		if( entry.position == 0 ){
			if( !battle_config.guild_leaderchange_woe && is_agit_start() ){
				clif_msg( sd, MSI_IMPOSSIBLE_CHANGE_GUILD_MASTER_IN_SIEGE_TIME );
				return;
			}

			if( battle_config.guild_leaderchange_delay && DIFF_TICK( time( nullptr ),sd->guild->guild.last_leader_change ) < battle_config.guild_leaderchange_delay ){
				clif_msg( sd, MSI_IMPOSSIBLE_CHANGE_GUILD_MASTER_NOT_TIME );
				return;
			}

			guild_gm_change( sd->status.guild_id, entry.CID );

			// No further entries will be processed - the requesting player lost his guild master status
			return;
		}else if( entry.position > 0 ){
			guild_change_memberposition( sd->status.guild_id, entry.AID, entry.CID, entry.position );
		}
	}
}


/// Request for guild emblem data (CZ_REQ_GUILD_EMBLEM_IMG).
/// 0151 <guild id>.L
void clif_parse_GuildRequestEmblem(int fd,map_session_data *sd)
{
	int guild_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	auto g = guild_search(guild_id);

	if (g)
		clif_guild_emblem(*sd, g->guild);
}


/// Validates data of a guild emblem (compressed bitmap)
enum e_result_validate_emblem {	// Used as Result for clif_validate_emblem
	EMBVALIDATE_SUCCESS = 0,
	EMBVALIDATE_ERR_RAW_FILEFORMAT,	// Invalid File Format (Error in zlib/decompression or malformed BMP header)
	EMBVALIDATE_ERR_TRANSPARENCY	// uploaded emblem does not met the requirements of inter_config.emblem_transparency_limit
};

static enum e_result_validate_emblem clif_validate_emblem(const uint8* emblem, unsigned long emblem_len)
{
	uint8 buf[1800];  // no well-formed emblem bitmap is larger than 1782 (24 bit) / 1654 (8 bit) bytes
	unsigned long buf_len = sizeof(buf);
	int offset = 0;

	if(!(( decode_zip(buf, &buf_len, emblem, emblem_len) == 0 && buf_len >= 18 )  // sizeof(BITMAPFILEHEADER) + sizeof(biSize) of the following info header struct
		&& RBUFW(buf,0) == 0x4d42   // BITMAPFILEHEADER.bfType (signature)
		&& RBUFL(buf,2) == buf_len  // BITMAPFILEHEADER.bfSize (file size)
		&& (offset = RBUFL(buf,10)) < buf_len  // BITMAPFILEHEADER.bfOffBits (offset to bitmap bits)
		))
		return EMBVALIDATE_ERR_RAW_FILEFORMAT;

	if(inter_config.emblem_transparency_limit != 100) {
		int i, transcount = 1, tmp[3];
		for(i = offset; i < buf_len-1; i++) {
			int j = i%3;
			tmp[j] = RBUFL(buf,i);
			if(j == 2 && (tmp[0] == 0xFFFF00FF) && (tmp[1] == 0xFFFF00) && (tmp[2] == 0xFF00FFFF)) //if pixel is transparent
				transcount++;
		}
		if(((transcount*300)/(buf_len-offset)) > inter_config.emblem_transparency_limit) //convert in % to chk
			return EMBVALIDATE_ERR_TRANSPARENCY;
	}

	return EMBVALIDATE_SUCCESS;
}


/// Request to update the guild emblem (CZ_REGISTER_GUILD_EMBLEM_IMG).
/// 0153 <packet len>.W <emblem data>.?B
void clif_parse_GuildChangeEmblem(int fd,map_session_data *sd){
	if( sd == nullptr ){
		return;
	}

	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	unsigned long emblem_len = RFIFOW(fd,info->pos[0])-4;
	const uint8* emblem = RFIFOP(fd,info->pos[1]);
	int emb_val=0;

	if( !emblem_len || !sd->state.gmaster_flag )
		return;

	if(!(inter_config.emblem_woe_change) && is_agit_start() ){
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,385),false,SELF); //"You not allowed to change emblem during woe"
		return;
	}
	emb_val = clif_validate_emblem(emblem, emblem_len);
	if(emb_val == EMBVALIDATE_ERR_RAW_FILEFORMAT){
		ShowWarning("clif_parse_GuildChangeEmblem: Rejected malformed guild emblem (size=%lu, accound_id=%d, char_id=%d, guild_id=%d).\n", emblem_len, sd->status.account_id, sd->status.char_id, sd->status.guild_id);
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,386),false,SELF); //"The chosen emblem was detected invalid\n"
		return;
	} else if(emb_val == EMBVALIDATE_ERR_TRANSPARENCY){
		char output[128];
		safesnprintf(output,sizeof(output),msg_txt(sd,387),inter_config.emblem_transparency_limit);
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],output,false,SELF); //"The chosen emblem was detected invalid as it contain too much transparency (limit=%d)\n"
		return;
	}

	guild_change_emblem( *sd, emblem_len, (const char*)emblem );
}

/// Request to update the guild emblem id
/// 0b46 <guild id>.L <version>.L
void clif_parse_GuildChangeEmblem2(int fd, map_session_data* sd) {
	if( sd == nullptr ){
		return;
	}

#if PACKETVER >= 20190724
	const PACKET_CZ_REQ_ADD_NEW_EMBLEM* p = reinterpret_cast<PACKET_CZ_REQ_ADD_NEW_EMBLEM*>( RFIFOP( fd, 0 ) );
	auto &g = sd->guild;

	if (!g || g->guild.guild_id != p->guild_id)
		return;

	if (!sd->state.gmaster_flag)
		return;

	if (!inter_config.emblem_woe_change && is_agit_start()) {
		clif_messagecolor(&sd->bl, color_table[COLOR_RED], msg_txt(sd, 385), false, SELF); //"You not allowed to change emblem during woe"
		return;
	}

	guild_change_emblem_version( *sd, p->version );
#endif
}

/// Guild notice update request (CZ_GUILD_NOTICE).
/// 016e <guild id>.L <msg1>.60B <msg2>.120B
void clif_parse_GuildChangeNotice(int fd, map_session_data* sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int guild_id = RFIFOL(fd,info->pos[0]);
	char* msg1 = RFIFOCP(fd,info->pos[1]);
	char* msg2 = RFIFOCP(fd,info->pos[2]);

	if(!sd->state.gmaster_flag)
		return;

	// compensate for some client defects when using multilanguage mode
	if (msg1[0] == '|' && msg1[3] == '|') msg1+= 3; // skip duplicate marker
	if (msg2[0] == '|' && msg2[3] == '|') msg2+= 3; // skip duplicate marker
	if (msg2[0] == '|') msg2[strnlen(msg2, MAX_GUILDMES2)-1] = '\0'; // delete extra space at the end of string

	guild_change_notice(sd, guild_id, msg1, msg2);
}

/// Guild invite request.
/// 0168 <account id>.L <inviter account id>.L <inviter char id>.L (CZ_REQ_JOIN_GUILD)
void clif_parse_GuildInvite( int fd,map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_JOIN_GUILD* p = reinterpret_cast<PACKET_CZ_REQ_JOIN_GUILD*>( RFIFOP( fd, 0 ) );

	guild_invite( *sd, map_id2sd( p->AID ) );
}

/// Guild invite request (/guildinvite)
/// 0916 <char name>.24B (CZ_REQ_JOIN_GUILD2)
void clif_parse_GuildInvite2( int fd, map_session_data *sd ){
#if PACKETVER >= 20120410
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_JOIN_GUILD2* p = reinterpret_cast<PACKET_CZ_REQ_JOIN_GUILD2*>( RFIFOP( fd, 0 ) );

	char nick[NAME_LENGTH] = {0};

	safestrncpy( nick, p->name, NAME_LENGTH );

	guild_invite( *sd, map_nick2sd( nick, false ) );
#endif
}

/// Answer to guild invitation.
/// 016b <guild id>.L <answer>.L (CZ_JOIN_GUILD)
/// answer:
///     0 = refuse
///     1 = accept
void clif_parse_GuildReplyInvite( int fd, map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_JOIN_GUILD* p = reinterpret_cast<PACKET_CZ_JOIN_GUILD*>( RFIFOP( fd, 0 ) );

	guild_reply_invite( *sd, p->guild_id, p->answer );
}

/// Request to leave guild.
/// 0159 <guild id>.L <account id>.L <char id>.L <reason>.40B (CZ_REQ_LEAVE_GUILD)
void clif_parse_GuildLeave(int fd,map_session_data *sd){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_LEAVE_GUILD* p = reinterpret_cast<PACKET_CZ_REQ_LEAVE_GUILD*>( RFIFOP( fd, 0 ) );

	guild_leave( *sd, p->guild_id, p->AID, p->CID, p->message );
}


/// Request to expel a member of a guild.
/// 015b <guild id>.L <account id>.L <char id>.L <reason>.40B (CZ_REQ_BAN_GUILD)
void clif_parse_GuildExpulsion(int fd,map_session_data *sd){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_BAN_GUILD* p = reinterpret_cast<PACKET_CZ_REQ_BAN_GUILD*>( RFIFOP( fd, 0 ) );

	guild_expulsion( *sd, p->guild_id, p->AID, p->CID, p->message );
}


/// Validates and processes guild messages (CZ_GUILD_CHAT).
/// 017e <packet len>.W <text>.?B (<name> : <message>) 00
void clif_parse_GuildMessage(int fd, map_session_data* sd){
	char name[NAME_LENGTH], message[CHAT_SIZE_MAX], output[CHAT_SIZE_MAX+NAME_LENGTH*2];

	// validate packet and retrieve name and message
	if( !clif_process_message( sd, false, name, message, output ) )
		return;

	if( sd->bg_id )
		bg_send_message(sd, output, strlen(output) );
	else
		guild_send_message(sd, output, strlen(output) );
}


/// Guild alliance request (CZ_REQ_ALLY_GUILD).
/// 0170 <account id>.L <inviter account id>.L <inviter char id>.L
void clif_parse_GuildRequestAlliance(int fd, map_session_data *sd)
{
	map_session_data *t_sd;

	if(!sd->state.gmaster_flag)
		return;

	if(map_getmapflag(sd->bl.m, MF_GUILDLOCK)) { //Guild locked.
		clif_displaymessage(fd, msg_txt(sd,228));
		return;
	}

	t_sd = map_id2sd(RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));
	//inv_aid = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[1]);
	//inv_cid = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[2]);

	// @noask [LuzZza]
	if(t_sd && t_sd->state.noask) {
		clif_noask_sub( *sd, *t_sd, 396 ); // Autorejected alliance request from %s.
		return;
	}

	guild_reqalliance(sd,t_sd);
}


/// Answer to a guild alliance request (CZ_ALLY_GUILD).
/// 0172 <inviter account id>.L <answer>.L
/// answer:
///     0 = refuse
///     1 = accept
void clif_parse_GuildReplyAlliance(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	guild_reply_reqalliance(sd,
	    RFIFOL(fd,info->pos[0]),
	    RFIFOL(fd,info->pos[1]));
}


/// Request to delete a guild alliance or opposition (CZ_REQ_DELETE_RELATED_GUILD).
/// 0183 <opponent guild id>.L <relation>.L
/// relation:
///     0 = Ally
///     1 = Enemy
void clif_parse_GuildDelAlliance(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	if(!sd->state.gmaster_flag)
		return;

	if(map_getmapflag(sd->bl.m, MF_GUILDLOCK)) { //Guild locked.
		clif_displaymessage(fd, msg_txt(sd,228));
		return;
	}
	guild_delalliance(sd,
	    RFIFOL(fd,info->pos[0]),
	    RFIFOL(fd,info->pos[1]));
}


/// Request to set a guild as opposition (CZ_REQ_HOSTILE_GUILD).
/// 0180 <account id>.L
void clif_parse_GuildOpposition(int fd, map_session_data *sd)
{
	map_session_data *t_sd;

	if(!sd->state.gmaster_flag)
		return;

	if(map_getmapflag(sd->bl.m, MF_GUILDLOCK)) { //Guild locked.
		clif_displaymessage(fd, msg_txt(sd,228));
		return;
	}

	t_sd = map_id2sd(RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));

	// @noask [LuzZza]
	if(t_sd && t_sd->state.noask) {
		clif_noask_sub( *sd, *t_sd, 397 ); // Autorejected opposition request from %s.
		return;
	}

	guild_opposition(sd,t_sd);
}


/// Request to delete own guild.
/// 015d <key>.40B (CZ_REQ_DISORGANIZE_GUILD)
/// key:
///     now guild name; might have been (intended) email, since the
///     field name and size is same as the one in CH_DELETE_CHAR.
void clif_parse_GuildBreak( int fd, map_session_data *sd ){
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_REQ_DISORGANIZE_GUILD* p = reinterpret_cast<PACKET_CZ_REQ_DISORGANIZE_GUILD*>( RFIFOP( fd, 0 ) );

	guild_break( *sd, p->key );
}


/// Pet
///

/// Request to invoke a pet menu action (CZ_COMMAND_PET).
/// 01a1 <type>.B
/// type:
///     0 = pet information
///     1 = feed
///     2 = performance
///     3 = return to egg
///     4 = unequip accessory
void clif_parse_PetMenu(int fd, map_session_data *sd){
	pet_menu(sd,RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// Attempt to tame a monster (CZ_TRYCAPTURE_MONSTER).
/// 019f <id>.L
void clif_parse_CatchPet(int fd, map_session_data *sd){
	pet_catch_process2(sd,RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// Answer to pet incubator egg selection dialog (CZ_SELECT_PETEGG).
/// 01a7 <index>.W
void clif_parse_SelectEgg(int fd, map_session_data *sd){
	if (sd->menuskill_id != SA_TAMINGMONSTER || sd->menuskill_val != -1)
		return;

	pet_select_egg(sd,RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0])-2);
	clif_menuskill_clear(sd);
}


/// Request to display pet's emotion/talk (CZ_PET_ACT).
/// 01a9 <data>.L
/// data:
///     is either emotion (@see enum emotion_type) or a compound value
///     (((mob id)-100)*100+(act id)*10+(hunger)) that describes an
///     entry (given in parentheses) in data\pettalktable.xml
///     act id:
///         0 = feeding
///         1 = hunting
///         2 = danger
///         3 = dead
///         4 = normal (stand)
///         5 = special performance (perfor_s)
///         6 = level up (levelup)
///         7 = performance 1 (perfor_1)
///         8 = performance 2 (perfor_2)
///         9 = performance 3 (perfor_3)
///        10 = log-in greeting (connect)
///     hungry value:
///         0 = very hungry (hungry)
///         1 = hungry (bit_hungry)
///         2 = satisfied (noting)
///         3 = stuffed (full)
///         4 = full (so_full)
void clif_parse_SendEmotion(int fd, map_session_data *sd)
{
	if(sd->pd)
		clif_pet_emotion(sd->pd,RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// Request to change pet's name (CZ_RENAME_PET).
/// 01a5 <name>.24B
void clif_parse_ChangePetName(int fd, map_session_data *sd)
{
	pet_change_name(sd,RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// /kill (CZ_DISCONNECT_CHARACTER).
/// Request to disconnect a character.
/// 00cc <account id>.L
/// NOTE: Also sent when using GM right click menu "(name) force to quit"
void clif_parse_GMKick(int fd, map_session_data *sd)
{
	struct block_list *target;
	int tid;

	tid = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	target = map_id2bl(tid);
	if (!target) {
		clif_GM_kickack(sd, 0);
		return;
	}

	switch (target->type) {
		case BL_PC:
		{
			char command[NAME_LENGTH+6];
			safesnprintf(command,sizeof(command),"%ckick %s", atcommand_symbol, status_get_name(target));
			is_atcommand(fd, sd, command, 1);
		}
		break;

		/**
		 * This one does not invoke any atcommand, so we need to check for permissions.
		 */
		case BL_MOB:
		{
			char command[100];
			if( !pc_can_use_command(sd, "killmonster", COMMAND_ATCOMMAND)) {
				clif_GM_kickack(sd, 0);
				return;
			}
			safesnprintf(command,sizeof(command),"/kick %s (%d)", status_get_name(target), status_get_class(target));
			log_atcommand(sd, command);
			status_percent_damage(&sd->bl, target, 100, 0, true); // can invalidate 'target'
		}
		break;

		case BL_NPC:
		{
			struct npc_data* nd = (struct npc_data *)target;
			if( pc_can_use_command(sd, "unloadnpc", COMMAND_ATCOMMAND)) {
				npc_unload_duplicates(nd);
				npc_unload(nd,true);
				npc_read_event_script();
			}
		}
		break;

		default:
			clif_GM_kickack(sd, 0);
	}
}


/// /killall (CZ_DISCONNECT_ALL_CHARACTER).
/// Request to disconnect all characters.
/// 00ce
void clif_parse_GMKickAll(int fd, map_session_data* sd) {
	char cmd[15];
	safesnprintf(cmd,sizeof(cmd),"%ckickall",atcommand_symbol);
	is_atcommand(fd, sd, cmd, 1);
}


/// /remove (CZ_REMOVE_AID).
/// Request to warp to a character with given login ID.
/// 01ba <account name>.24B

/// /shift (CZ_SHIFT).
/// Request to warp to a character with given name.
/// 01bb <char name>.24B
void clif_parse_GMShift(int fd, map_session_data *sd)
{// FIXME: remove is supposed to receive account name for clients prior 20100803RE
	char *player_name;
	char command[NAME_LENGTH+8];

	player_name = RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	player_name[NAME_LENGTH-1] = '\0';

	safesnprintf(command,sizeof(command),"%cjumpto %s", atcommand_symbol, player_name);
	is_atcommand(fd, sd, command, 1);
}


/// /remove (CZ_REMOVE_AID_SSO).
/// Request to warp to a character with given account ID.
/// 0843 <account id>.L
void clif_parse_GMRemove2(int fd, map_session_data* sd)
{
	uint32 account_id;
	map_session_data* pl_sd;

	account_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if( (pl_sd = map_id2sd(account_id)) != nullptr ) {
		char command[NAME_LENGTH+8];
		safesnprintf(command,sizeof(command),"%cjumpto %s", atcommand_symbol, pl_sd->status.name);
		is_atcommand(fd, sd, command, 1);
	}
}


/// /recall (CZ_RECALL).
/// Request to summon a player with given login ID to own position.
/// 01bc <account name>.24B

/// /summon (CZ_RECALL_GID).
/// Request to summon a player with given name to own position.
/// 01bd <char name>.24B
void clif_parse_GMRecall(int fd, map_session_data *sd)
{// FIXME: recall is supposed to receive account name for clients prior 20100803RE
	char *player_name;
	char command [NAME_LENGTH+8];

	player_name = RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	player_name[NAME_LENGTH-1] = '\0';

	safesnprintf(command,sizeof(command),"%crecall %s", atcommand_symbol, player_name);
	is_atcommand(fd, sd, command, 1);
}


/// /recall (CZ_RECALL_SSO).
/// Request to summon a player with given account ID to own position.
/// 0842 <account id>.L
void clif_parse_GMRecall2(int fd, map_session_data* sd)
{
	uint32 account_id;
	map_session_data* pl_sd;

	account_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if( (pl_sd = map_id2sd(account_id)) != nullptr ) {
		char command[NAME_LENGTH+8];
		safesnprintf(command,sizeof(command),"%crecall %s", atcommand_symbol, pl_sd->status.name);
		is_atcommand(fd, sd, command, 1);
	}
}


/// /item /monster (CZ_ITEM_CREATE).
/// Request to execute GM commands.
/// Usage:
/// /item n - summon n monster or acquire n item/s
/// /item money - grants 2147483647 zenies
/// /item whereisboss - locate boss mob in current map.(not yet implemented)
/// /item regenboss_n t - regenerate n boss monster by t millisecond.(not yet implemented)
/// /item onekillmonster - toggle an ability to kill mobs in one hit.(not yet implemented)
/// /item bossinfo - display the information of a boss monster in current map.(not yet implemented)
/// /item cap_n - capture n monster as pet.(not yet implemented)
/// /item agitinvest - reset current global agit investments.(not yet implemented)
/// 013f <item/mob name>.24B
/// 09ce <item/mob name>.100B [Ind/Yommy]
void clif_parse_GM_Item_Monster(int fd, map_session_data *sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int mob_id = 0;
	StringBuf command;
	char *str;
//#if PACKETVER >= 20131218
//	char str[100];
//#else
//	char str[24];
//#endif

	str = RFIFOCP(fd,info->pos[0]);
	if (!str || str[0] == '\0')
		return;
	if (strcmpi(str,"SPPOINT") == 0 || strcmpi(str,"JOBLEVEL") == 0) //! TODO /sp sends these values
		return;
	trim(str);

	// Zeny
	if( strcmpi(str, "money") == 0 ) {
		StringBuf_Init(&command);
		StringBuf_Printf(&command, "%czeny %d", atcommand_symbol, INT_MAX);
		is_atcommand(fd, sd, StringBuf_Value(&command), 1);
		StringBuf_Destroy(&command);
		return;
	}

	std::shared_ptr<item_data> id = item_db.searchname( str );

	// Item
	if( id ){
		StringBuf_Init(&command);
		if( !itemdb_isstackable2( id.get() ) ){ //Nonstackable
			StringBuf_Printf(&command, "%citem2 %u 1 0 0 0 0 0 0 0", atcommand_symbol, id->nameid);
		}else{
			if (id->flag.guid)
				StringBuf_Printf(&command, "%citem %u 1", atcommand_symbol, id->nameid);
			else
				StringBuf_Printf(&command, "%citem %u 20", atcommand_symbol, id->nameid);
		}
		is_atcommand(fd, sd, StringBuf_Value(&command), 1);
		StringBuf_Destroy(&command);
		return;
	}

	// Monster
	if ((mob_id = mobdb_searchname(str)) == 0)
		mob_id = mobdb_checkid(atoi(str));

	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);

	if( mob != nullptr ) {
		StringBuf_Init(&command);
		StringBuf_Printf(&command, "%cmonster %s", atcommand_symbol, mob->sprite.c_str());
		is_atcommand(fd, sd, StringBuf_Value(&command), 1);
		StringBuf_Destroy(&command);
		return;
	}
}


/// /hide (CZ_CHANGE_EFFECTSTATE).
/// 019d <effect state>.L
/// effect state:
///     TODO: Any OPTION_* ?
void clif_parse_GMHide(int fd, map_session_data *sd) {
	char cmd[6];
	//int eff_st = RFIFOL(packet_db[RFIFOW(fd,0)].pos[0]);

	safesnprintf(cmd,sizeof(cmd),"%chide",atcommand_symbol);
	is_atcommand(fd, sd, cmd, 1);
}


/// Request to adjust player's manner points (CZ_REQ_GIVE_MANNER_POINT).
/// 0149 <account id>.L <type>.B <value>.W
/// type:
///     0 = positive points
///     1 = negative points
///     2 = self mute (+10 minutes)
void clif_parse_GMReqNoChat(int fd,map_session_data *sd)
{
	int id, type, value;
	map_session_data *dstsd;
	char command[NAME_LENGTH+15];
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];


	id = RFIFOL(fd,info->pos[0]);
	type = RFIFOB(fd,info->pos[1]);
	value = RFIFOW(fd,info->pos[2]);

	if( type == 0 )
		value = -value;

	//If type is 2 and the ids don't match, this is a crafted hacked packet!
	//Disabled because clients keep self-muting when you give players public @ commands... [Skotlex]
	if (type == 2 /* && (pc_get_group_level(sd) > 0 || sd->bl.id != id)*/)
		return;

	dstsd = map_id2sd(id);
	if( dstsd == nullptr )
		return;

	safesnprintf(command,sizeof(command),"%cmute %d %s", atcommand_symbol, value, dstsd->status.name);
	is_atcommand(fd, sd, command, 1);
}


/// /rc (CZ_REQ_GIVE_MANNER_BYNAME).
/// GM adjustment of a player's manner value by -60.
/// 0212 <char name>.24B
void clif_parse_GMRc(int fd, map_session_data* sd)
{
	char command[NAME_LENGTH+15];
	char *name = RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	name[NAME_LENGTH-1] = '\0';
	safesnprintf(command,sizeof(command),"%cmute %d %s", atcommand_symbol, 60, name);
	is_atcommand(fd, sd, command, 1);
}


/// Result of request to resolve account name (ZC_ACK_ACCOUNTNAME).
/// 01e0 <account id>.L <account name>.24B
void clif_account_name(int fd, uint32 account_id, const char* accname)
{
	WFIFOHEAD(fd,packet_len(0x1e0));
	WFIFOW(fd,0) = 0x1e0;
	WFIFOL(fd,2) = account_id;
	safestrncpy(WFIFOCP(fd,6), accname, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x1e0));
}


/// GM requesting account name (for right-click gm menu) (CZ_REQ_ACCOUNTNAME).
/// 01df <account id>.L
//! TODO: Figure out how does this actually work
void clif_parse_GMReqAccountName(int fd, map_session_data *sd)
{
	uint32 account_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	/*
	char query[30];
	safesnprintf(query,sizeof(query),"%d", account_id);
	intif_request_accinfo(sd->fd, sd->bl.id, pc_get_group_level(sd), query, 1); //will call clif_account_name at return
	*/

	clif_account_name(fd, account_id, "");
}


/// /changemaptype <x> <y> <type> (CZ_CHANGE_MAPTYPE).
/// GM single cell type change request.
/// 0198 <x>.W <y>.W <type>.W
/// type:
///     0 = not walkable
///     1 = walkable
void clif_parse_GMChangeMapType(int fd, map_session_data *sd)
{
	int x,y,type;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	if(! pc_has_permission(sd, PC_PERM_USE_CHANGEMAPTYPE) )
		return;

	x = RFIFOW(fd,info->pos[0]);
	y = RFIFOW(fd,info->pos[1]);
	type = RFIFOW(fd,info->pos[2]);

	map_setgatcell(sd->bl.m,x,y,type);
	clif_changemapcell(0,sd->bl.m,x,y,type,ALL_SAMEMAP);
	//FIXME: once players leave the map, the client 'forgets' this information.
}


/// /in /ex (CZ_SETTING_WHISPER_PC).
/// Request to allow/deny whispers from a nick.
/// 00cf <nick>.24B <type>.B
/// type:
///     0 = (/ex nick) deny speech from nick
///     1 = (/in nick) allow speech from nick
void clif_parse_PMIgnore(int fd, map_session_data* sd)
{
	char* nick;
	uint8 type;
	int i;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	nick = RFIFOCP(fd,info->pos[0]);
	nick[NAME_LENGTH-1] = '\0'; // to be sure that the player name has at most 23 characters
	type = RFIFOB(fd,info->pos[1]);

	if( type == 0 ) { // Add name to ignore list (block)
		if (strcmp(wisp_server_name, nick) == 0) {
			clif_wisexin(sd, type, 1); // fail
			return;
		}

		// try to find a free spot, while checking for duplicates at the same time
		ARR_FIND( 0, MAX_IGNORE_LIST, i, sd->ignore[i].name[0] == '\0' || strcmp(sd->ignore[i].name, nick) == 0 );
		if( i == MAX_IGNORE_LIST ) {// no space for new entry
			clif_wisexin(sd, type, 2); // too many blocks
			return;
		}
		if( sd->ignore[i].name[0] != '\0' ) {// name already exists
			clif_wisexin(sd, type, 0); // Aegis reports success.
			return;
		}

		//Insert in position i
		safestrncpy(sd->ignore[i].name, nick, NAME_LENGTH);
	} else { // Remove name from ignore list (unblock)

		// find entry
		ARR_FIND( 0, MAX_IGNORE_LIST, i, sd->ignore[i].name[0] == '\0' || strcmp(sd->ignore[i].name, nick) == 0 );
		if( i == MAX_IGNORE_LIST || sd->ignore[i].name[0] == '\0' ) { //Not found
			clif_wisexin(sd, type, 1); // fail
			return;
		}
		// move everything one place down to overwrite removed entry
		memmove(sd->ignore[i].name, sd->ignore[i+1].name, (MAX_IGNORE_LIST-i-1)*sizeof(sd->ignore[0].name));
		// wipe last entry
		memset(sd->ignore[MAX_IGNORE_LIST-1].name, 0, sizeof(sd->ignore[0].name));
	}

	clif_wisexin(sd, type, 0); // success
}


/// /inall /exall (CZ_SETTING_WHISPER_STATE).
/// Request to allow/deny all whispers.
/// 00d0 <type>.B
/// type:
///     0 = (/exall) deny all speech
///     1 = (/inall) allow all speech
void clif_parse_PMIgnoreAll(int fd, map_session_data *sd)
{
	int type = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]), flag;

	if( type == 0 ) {// Deny all
		if( sd->state.ignoreAll ) {
			flag = 1; // fail
		} else {
			sd->state.ignoreAll = 1;
			flag = 0; // success
		}
	} else {//Unblock everyone
		if( sd->state.ignoreAll ) {
			sd->state.ignoreAll = 0;
			flag = 0; // success
		} else {
			if (sd->ignore[0].name[0] != '\0')
			{  //Wipe the ignore list.
				memset(sd->ignore, 0, sizeof(sd->ignore));
				flag = 0; // success
			} else {
				flag = 1; // fail
			}
		}
	}

	clif_wisall(sd, type, flag);
}


/// Whisper ignore list (ZC_WHISPER_LIST).
/// 00d4 <packet len>.W { <char name>.24B }*
void clif_PMIgnoreList(map_session_data* sd)
{
	int i, fd = sd->fd;

	WFIFOHEAD(fd,4+ARRAYLENGTH(sd->ignore)*NAME_LENGTH);
	WFIFOW(fd,0) = 0xd4;

	for( i = 0; i < ARRAYLENGTH(sd->ignore) && sd->ignore[i].name[0]; i++ ) {
		safestrncpy(WFIFOCP(fd,4+i*NAME_LENGTH), sd->ignore[i].name, NAME_LENGTH);
	}

	WFIFOW(fd,2) = 4+i*NAME_LENGTH;
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Whisper ignore list request (CZ_REQ_WHISPER_LIST).
/// 00d3
void clif_parse_PMIgnoreList(int fd,map_session_data *sd)
{
	clif_PMIgnoreList(sd);
}


/// Request to invoke the /doridori recovery bonus (CZ_DORIDORI).
/// 01e7
void clif_parse_NoviceDoriDori(int fd, map_session_data *sd)
{
	if (sd->state.doridori) return;

	switch (sd->class_&MAPID_UPPERMASK) {
		case MAPID_SOUL_LINKER:
		case MAPID_STAR_GLADIATOR:
		case MAPID_TAEKWON:
			if (!sd->state.rest)
				break;
			[[fallthrough]];
		case MAPID_SUPER_NOVICE:
		case MAPID_SUPER_BABY:
		case MAPID_SUPER_NOVICE_E:
		case MAPID_SUPER_BABY_E:
			sd->state.doridori=1;
			break;
	}
}


/// Request to invoke the effect of super novice's guardian angel prayer (CZ_CHOPOKGI).
/// 01ed
/// Note: This packet is caused by 7 lines of any text, followed by
///       the prayer and an another line of any text. The prayer is
///       defined by lines 790~793 in data\msgstringtable.txt
///       "Dear angel, can you hear my voice?"
///       "I am" (space separated player name) "Super Novice~"
///       "Help me out~ Please~ T_T"
void clif_parse_NoviceExplosionSpirits(int fd, map_session_data *sd)
{
	if( (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE ) {
		t_exp next = pc_nextbaseexp(sd);

		if( next ) {
			int percent = (int)( ( (double)sd->status.base_exp/(double)next )*1000. );

			if( percent && ( percent%100 ) == 0 ) {// 10.0%, 20.0%, ..., 90.0%
				sc_start(&sd->bl,&sd->bl, SC_EXPLOSIONSPIRITS, 100, 17, skill_get_time(MO_EXPLOSIONSPIRITS, 5)); //Lv17-> +50 critical (noted by Poki) [Skotlex]
				clif_skill_nodamage(&sd->bl, &sd->bl, MO_EXPLOSIONSPIRITS, 5, 1);  // prayer always shows successful Lv5 cast and disregards noskill restrictions
			}
		}
	}
}


/// Friends List
///

/// Toggles a single friend online/offline [Skotlex] (ZC_FRIENDS_STATE).
/// 0206 <account id>.L <char id>.L <state>.B
/// 0206 <account id>.L <char id>.L <state>.B <name>.24B >= 20180221
/// state:
///     0 = online
///     1 = offline
void clif_friendslist_toggle(map_session_data *sd,uint32 account_id, uint32 char_id, int online)
{
	int i, fd = sd->fd;

	//Seek friend.
	for (i = 0; i < MAX_FRIENDS && sd->status.friends[i].char_id &&
		(sd->status.friends[i].char_id != char_id || sd->status.friends[i].account_id != account_id); i++);

	if(i == MAX_FRIENDS || sd->status.friends[i].char_id == 0)
		return; //Not found

	WFIFOHEAD(fd,packet_len(0x206));
	WFIFOW(fd, 0) = 0x206;
	WFIFOL(fd, 2) = sd->status.friends[i].account_id;
	WFIFOL(fd, 6) = sd->status.friends[i].char_id;
	WFIFOB(fd,10) = !online; //Yeah, a 1 here means "logged off", go figure...
#if PACKETVER >= 20180221
	safestrncpy(WFIFOCP(fd, 11), sd->status.friends[i].name, NAME_LENGTH);
#endif
	WFIFOSET(fd, packet_len(0x206));
}


//Subfunction called from clif_foreachclient to toggle friends on/off [Skotlex]
int clif_friendslist_toggle_sub(map_session_data *sd,va_list ap)
{
	uint32 account_id, char_id, online;
	account_id = va_arg(ap, int);
	char_id = va_arg(ap, int);
	online = va_arg(ap, int);
	clif_friendslist_toggle(sd, account_id, char_id, online);
	return 0;
}


/// Sends the whole friends list (ZC_FRIENDS_LIST).
/// 0201 <packet len>.W { <account id>.L <char id>.L <name>.24B }*
/// 0201 <packet len>.W { <account id>.L <char id>.L }* >= 20180221
void clif_friendslist_send( map_session_data& sd ){
	PACKET_ZC_FRIENDS_LIST* p = reinterpret_cast<PACKET_ZC_FRIENDS_LIST*>( packet_buffer );

	p->packetType = HEADER_ZC_FRIENDS_LIST;
	p->PacketLength = sizeof( *p );

	for( int i = 0, c = 0; i < MAX_FRIENDS && sd.status.friends[i].char_id; i++ ){
		PACKET_ZC_FRIENDS_LIST_sub& f = p->friends[c];

		f.AID = sd.status.friends[i].account_id;
		f.CID = sd.status.friends[i].char_id;
#if !( PACKETVER_MAIN_NUM >= 20180307 || PACKETVER_RE_NUM >= 20180221 || PACKETVER_ZERO_NUM >= 20180328 ) || PACKETVER >= 20200902
		safestrncpy( f.name, sd.status.friends[i].name, sizeof( f.name ) );
#endif

		p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof( f ) );
		c++;
	}

	clif_send( p, p->PacketLength, &sd.bl, SELF );

	// Sending the online players
	for( int i = 0; i < MAX_FRIENDS && sd.status.friends[i].char_id; i++ ){
		if( map_charid2sd( sd.status.friends[i].char_id ) ){
			clif_friendslist_toggle( &sd, sd.status.friends[i].account_id, sd.status.friends[i].char_id, 1 );
		}
	}
}


/// Notification about the result of a friend add request (ZC_ADD_FRIENDS_LIST).
/// 0209 <result>.W <account id>.L <char id>.L <name>.24B
/// result:
///     0 = MsgStringTable[821]="You have become friends with (%s)."
///     1 = MsgStringTable[822]="(%s) does not want to be friends with you."
///     2 = MsgStringTable[819]="Your Friend List is full."
///     3 = MsgStringTable[820]="(%s)'s Friend List is full."
void clif_friendslist_reqack(map_session_data *sd, map_session_data *f_sd, int type)
{
	int fd;
	nullpo_retv(sd);

	fd = sd->fd;
	WFIFOHEAD(fd,packet_len(0x209));
	WFIFOW(fd,0) = 0x209;
	WFIFOW(fd,2) = type;
	if (f_sd) {
		WFIFOL(fd,4) = f_sd->status.account_id;
		WFIFOL(fd,8) = f_sd->status.char_id;
		safestrncpy(WFIFOCP(fd, 12), f_sd->status.name,NAME_LENGTH);
	}
	WFIFOSET(fd, packet_len(0x209));
}


/// Asks a player for permission to be added as friend (ZC_REQ_ADD_FRIENDS).
/// 0207 <req account id>.L <req char id>.L <req char name>.24B
void clif_friendlist_req(map_session_data* sd, uint32 account_id, uint32 char_id, const char* name)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x207));
	WFIFOW(fd,0) = 0x207;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = char_id;
	safestrncpy(WFIFOCP(fd,10), name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x207));
}


/// Request to add a player as friend (CZ_ADD_FRIENDS).
/// 0202 <name>.24B
void clif_parse_FriendsListAdd(int fd, map_session_data *sd)
{
	map_session_data *f_sd;
	int i;

	f_sd = map_nick2sd(RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]),false);

	// Friend doesn't exist (no player with this name)
	if (f_sd == nullptr) {
		clif_displaymessage(fd, msg_txt(sd,3));
		return;
	}

	if( sd->bl.id == f_sd->bl.id ) {// adding oneself as friend
		return;
	}

	// ensure that the request player's friend list is not full
	ARR_FIND(0, MAX_FRIENDS, i, sd->status.friends[i].char_id == 0);

	if( i == MAX_FRIENDS ){
		clif_friendslist_reqack(sd, f_sd, 2);
		return;
	}

	// @noask [LuzZza]
	if(f_sd->state.noask) {
		clif_noask_sub( *sd, *f_sd, 398 ); // Autorejected friend request from %s.
		return;
	}

	// Friend already exists
	for (i = 0; i < MAX_FRIENDS && sd->status.friends[i].char_id != 0; i++) {
		if (sd->status.friends[i].char_id == f_sd->status.char_id) {
			clif_displaymessage(fd, msg_txt(sd,671)); //"Friend already exists."
			return;
		}
	}

	f_sd->friend_req = sd->status.char_id;
	sd->friend_req   = f_sd->status.char_id;

	clif_friendlist_req(f_sd, sd->status.account_id, sd->status.char_id, sd->status.name);
}


/// Answer to a friend add request (CZ_ACK_REQ_ADD_FRIENDS).
/// 0208 <inviter account id>.L <inviter char id>.L <reply>.B
/// 0208 <inviter account id>.L <inviter char id>.L <reply>.L (PACKETVER >= 6)
/// reply:
///     0 = rejected
///     1 = accepted
void clif_parse_FriendsListReply(int fd, map_session_data *sd)
{
	map_session_data *f_sd;
	uint32 account_id;
	char reply;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	account_id = RFIFOL(fd,info->pos[0]);
	//char_id = RFIFOL(fd,info->pos[1]);
#if PACKETVER < 6
	reply = RFIFOB(fd,info->pos[2]);
#else
	reply = RFIFOL(fd,info->pos[2]);
#endif

	if( sd->bl.id == account_id ) {// adding oneself as friend
		return;
	}

	f_sd = map_id2sd(account_id); //The account id is the same as the bl.id of players.
	if (f_sd == nullptr)
		return;

	if (reply == 0 || !( sd->friend_req == f_sd->status.char_id && f_sd->friend_req == sd->status.char_id ) )
		clif_friendslist_reqack(f_sd, sd, 1);
	else {
		int i;
		// Find an empty slot
		for (i = 0; i < MAX_FRIENDS; i++)
			if (f_sd->status.friends[i].char_id == 0)
				break;
		if (i == MAX_FRIENDS) {
			clif_friendslist_reqack(f_sd, sd, 2);
			return;
		}

		f_sd->status.friends[i].account_id = sd->status.account_id;
		f_sd->status.friends[i].char_id = sd->status.char_id;
		safestrncpy(f_sd->status.friends[i].name, sd->status.name, NAME_LENGTH);
		clif_friendslist_reqack(f_sd, sd, 0);

		achievement_update_objective(f_sd, AG_ADD_FRIEND, 1, i + 1);

		if (battle_config.friend_auto_add) {
			// Also add f_sd to sd's friendlist.
			for (i = 0; i < MAX_FRIENDS; i++) {
				if (sd->status.friends[i].char_id == f_sd->status.char_id)
					return; //No need to add anything.
				if (sd->status.friends[i].char_id == 0)
					break;
			}
			if (i == MAX_FRIENDS) {
				clif_friendslist_reqack(sd, f_sd, 2);
				return;
			}

			sd->status.friends[i].account_id = f_sd->status.account_id;
			sd->status.friends[i].char_id = f_sd->status.char_id;
			safestrncpy(sd->status.friends[i].name, f_sd->status.name, NAME_LENGTH);
			clif_friendslist_reqack(sd, f_sd, 0);

			achievement_update_objective(sd, AG_ADD_FRIEND, 1, i + 1);
		}
	}
}


/// Request to delete a friend (CZ_DELETE_FRIENDS).
/// 0203 <account id>.L <char id>.L
void clif_parse_FriendsListRemove(int fd, map_session_data *sd)
{
	map_session_data *f_sd = nullptr;
	uint32 account_id, char_id;
	int i, j;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	account_id = RFIFOL(fd,info->pos[0]);
	char_id = RFIFOL(fd,info->pos[1]);

	// Search friend
	for (i = 0; i < MAX_FRIENDS &&
		(sd->status.friends[i].char_id != char_id || sd->status.friends[i].account_id != account_id); i++);

	if (i == MAX_FRIENDS) {
		clif_displaymessage(fd, msg_txt(sd,672)); //"Name not found in list."
		return;
	}

	//remove from friend's list first
	if( (f_sd = map_id2sd(account_id)) && f_sd->status.char_id == char_id) {
		for (i = 0; i < MAX_FRIENDS &&
			(f_sd->status.friends[i].char_id != sd->status.char_id || f_sd->status.friends[i].account_id != sd->status.account_id); i++);

		if (i != MAX_FRIENDS) {
			// move all chars up
			for(j = i + 1; j < MAX_FRIENDS; j++)
				memcpy(&f_sd->status.friends[j-1], &f_sd->status.friends[j], sizeof(f_sd->status.friends[0]));

			memset(&f_sd->status.friends[MAX_FRIENDS-1], 0, sizeof(f_sd->status.friends[MAX_FRIENDS-1]));
			//should the guy be notified of some message? we should add it here if so
			WFIFOHEAD(f_sd->fd,packet_len(0x20a));
			WFIFOW(f_sd->fd,0) = 0x20a;
			WFIFOL(f_sd->fd,2) = sd->status.account_id;
			WFIFOL(f_sd->fd,6) = sd->status.char_id;
			WFIFOSET(f_sd->fd, packet_len(0x20a));
		}

	} else { //friend not online -- ask char server to delete from his friendlist
		if(chrif_removefriend(char_id,sd->status.char_id)) { // char-server offline, abort
			clif_displaymessage(fd, msg_txt(sd,673)); //"This action can't be performed at the moment. Please try again later."
			return;
		}
	}

	// We can now delete from original requester
	for (i = 0; i < MAX_FRIENDS &&
		(sd->status.friends[i].char_id != char_id || sd->status.friends[i].account_id != account_id); i++);
	// move all chars up
	for(j = i + 1; j < MAX_FRIENDS; j++)
		memcpy(&sd->status.friends[j-1], &sd->status.friends[j], sizeof(sd->status.friends[0]));

	memset(&sd->status.friends[MAX_FRIENDS-1], 0, sizeof(sd->status.friends[MAX_FRIENDS-1]));
	clif_displaymessage(fd, msg_txt(sd,674)); //"Friend removed"

	WFIFOHEAD(fd,packet_len(0x20a));
	WFIFOW(fd,0) = 0x20a;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = char_id;
	WFIFOSET(fd, packet_len(0x20a));
}


/// /pvpinfo list (ZC_ACK_PVPPOINT).
/// 0210 <char id>.L <account id>.L <win point>.L <lose point>.L <point>.L
void clif_PVPInfo(map_session_data* sd)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x210));
	WFIFOW(fd,0) = 0x210;
	WFIFOL(fd,2) = sd->status.char_id;
	WFIFOL(fd,6) = sd->status.account_id;
	WFIFOL(fd,10) = sd->pvp_won;	// times won
	WFIFOL(fd,14) = sd->pvp_lost;	// times lost
	WFIFOL(fd,18) = sd->pvp_point;
	WFIFOSET(fd, packet_len(0x210));
}


/// /pvpinfo (CZ_REQ_PVPPOINT).
/// 020f <char id>.L <account id>.L
void clif_parse_PVPInfo(int fd,map_session_data *sd)
{
	// TODO: Is there a way to use this on an another player (char/acc id)?
	//int cid = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	//int aid = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[1]);
	clif_PVPInfo(sd);
}

/// SG Feel save OK [Komurka] (CZ_AGREE_STARPLACE).
/// 0254 <which>.B
/// which: (unused atm for security purpose)
///     0 = sun
///     1 = moon
///     2 = star
void clif_parse_FeelSaveOk(int fd,map_session_data *sd)
{
	int i;
	//int wich = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if (sd->menuskill_id != SG_FEEL)
		return;
	i = sd->menuskill_val-1;
	if (i<0 || i >= MAX_PC_FEELHATE) return; //Bug?

	sd->feel_map[i].index = map_id2index(sd->bl.m);
	sd->feel_map[i].m = sd->bl.m;
	pc_setglobalreg(sd, add_str(sg_info[i].feel_var), sd->feel_map[i].index);

//Are these really needed? Shouldn't they show up automatically from the feel save packet?
//	clif_misceffect2(&sd->bl, 0x1b0);
//	clif_misceffect2(&sd->bl, 0x21f);
	clif_feel_info(sd, i, 0);
	clif_menuskill_clear(sd);
}


/// Star Gladiator's Feeling map confirmation prompt (ZC_STARPLACE).
/// 0253 <which>.B
/// which:
///     0 = sun
///     1 = moon
///     2 = star
void clif_feel_req(int fd, map_session_data *sd, uint16 skill_lv)
{
	WFIFOHEAD(fd,packet_len(0x253));
	WFIFOW(fd,0)=0x253;
	WFIFOB(fd,2)=TOB(skill_lv-1);
	WFIFOSET(fd, packet_len(0x253));
	sd->menuskill_id = SG_FEEL;
	sd->menuskill_val = skill_lv;
}


/// Request to change homunculus' name (CZ_RENAME_MER).
/// 0231 <name>.24B
void clif_parse_ChangeHomunculusName(int fd, map_session_data *sd){
	hom_change_name(sd,RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]));
}


/// Request to warp/move homunculus/mercenary to it's owner (CZ_REQUEST_MOVETOOWNER).
/// 0234 <id>.L
void clif_parse_HomMoveToMaster(int fd, map_session_data *sd){
	int id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]); // Mercenary or Homunculus
	struct block_list *bl = nullptr;
	struct unit_data *ud = nullptr;

	if( sd->md && sd->md->bl.id == id )
		bl = &sd->md->bl;
	else if( hom_is_active(sd->hd) && sd->hd->bl.id == id )
		bl = &sd->hd->bl; // Moving Homunculus
	else
		return;

	unit_calc_pos(bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	ud = unit_bl2ud(bl);
	unit_walktoxy(bl, ud->to_x, ud->to_y, 4);
}


/// Request to move homunculus/mercenary (CZ_REQUEST_MOVENPC).
/// 0232 <id>.L <position data>.3B
void clif_parse_HomMoveTo(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int id = RFIFOL(fd,info->pos[0]); // Mercenary or Homunculus
	struct block_list *bl = nullptr;
	short x, y;

	RFIFOPOS(fd, info->pos[1], &x, &y, nullptr);

	if( sd->md && sd->md->bl.id == id )
		bl = &sd->md->bl; // Moving Mercenary
	else if( hom_is_active(sd->hd) && sd->hd->bl.id == id )
		bl = &sd->hd->bl; // Moving Homunculus
	else
		return;

	unit_walktoxy(bl, x, y, 4);
}


/// Request to do an action with homunculus/mercenary (CZ_REQUEST_ACTNPC).
/// 0233 <id>.L <target id>.L <action>.B
/// action:
///     always 0
void clif_parse_HomAttack(int fd,map_session_data *sd)
{
	struct block_list *bl = nullptr;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int id = RFIFOL(fd,info->pos[0]);
	int target_id = RFIFOL(fd,info->pos[1]);
	int action_type = RFIFOB(fd,info->pos[2]);

	if( hom_is_active(sd->hd) && sd->hd->bl.id == id )
		bl = &sd->hd->bl;
	else if( sd->md && sd->md->bl.id == id )
		bl = &sd->md->bl;
	else return;

	unit_stop_attack(bl);
	unit_attack(bl, target_id, action_type != 0);
}


/// Request to invoke a homunculus menu action (CZ_COMMAND_MER).
/// 022d <type>.W <command>.B
/// type:
///     always 0
/// command:
///     0 = homunculus information
///     1 = feed
///     2 = delete
void clif_parse_HomMenu(int fd, map_session_data *sd)
{	//[orn]
	int cmd = RFIFOW(fd,0);
	//int type = RFIFOW(fd,packet_db[cmd].pos[0]);
	if(!hom_is_active(sd->hd))
		return;

	hom_menu(sd,RFIFOB(fd,packet_db[cmd].pos[1]));
}


/// Request to resurrect oneself using Token of Siegfried (CZ_STANDING_RESURRECTION).
/// 0292
void clif_parse_AutoRevive(int fd, map_session_data *sd)
{
	pc_revive_item(sd);
}


/// Information about character's status values (ZC_ACK_STATUS_GM).
/// 0214 <str>.B <standardStr>.B <agi>.B <standardAgi>.B <vit>.B <standardVit>.B
///      <int>.B <standardInt>.B <dex>.B <standardDex>.B <luk>.B <standardLuk>.B
///      <attPower>.W <refiningPower>.W <max_mattPower>.W <min_mattPower>.W
///      <itemdefPower>.W <plusdefPower>.W <mdefPower>.W <plusmdefPower>.W
///      <hitSuccessValue>.W <avoidSuccessValue>.W <plusAvoidSuccessValue>.W
///      <criticalSuccessValue>.W <ASPD>.W <plusASPD>.W
void clif_check(int fd, map_session_data* pl_sd)
{
	WFIFOHEAD(fd,packet_len(0x214));
	WFIFOW(fd, 0) = 0x214;
	WFIFOB(fd, 2) = min(pl_sd->status.str, UINT8_MAX);
	WFIFOB(fd, 3) = pc_need_status_point(pl_sd, SP_STR, 1);
	WFIFOB(fd, 4) = min(pl_sd->status.agi, UINT8_MAX);
	WFIFOB(fd, 5) = pc_need_status_point(pl_sd, SP_AGI, 1);
	WFIFOB(fd, 6) = min(pl_sd->status.vit, UINT8_MAX);
	WFIFOB(fd, 7) = pc_need_status_point(pl_sd, SP_VIT, 1);
	WFIFOB(fd, 8) = min(pl_sd->status.int_, UINT8_MAX);
	WFIFOB(fd, 9) = pc_need_status_point(pl_sd, SP_INT, 1);
	WFIFOB(fd,10) = min(pl_sd->status.dex, UINT8_MAX);
	WFIFOB(fd,11) = pc_need_status_point(pl_sd, SP_DEX, 1);
	WFIFOB(fd,12) = min(pl_sd->status.luk, UINT8_MAX);
	WFIFOB(fd,13) = pc_need_status_point(pl_sd, SP_LUK, 1);
	WFIFOW(fd,14) = pl_sd->battle_status.batk+pl_sd->battle_status.rhw.atk+pl_sd->battle_status.lhw.atk;
	WFIFOW(fd,16) = pl_sd->battle_status.rhw.atk2+pl_sd->battle_status.lhw.atk2;
	WFIFOW(fd,18) = pl_sd->battle_status.matk_max;
	WFIFOW(fd,20) = pl_sd->battle_status.matk_min;
	WFIFOW(fd,22) = pl_sd->battle_status.def;
	WFIFOW(fd,24) = pl_sd->battle_status.def2;
	WFIFOW(fd,26) = pl_sd->battle_status.mdef;
	WFIFOW(fd,28) = pl_sd->battle_status.mdef2;
	WFIFOW(fd,30) = pl_sd->battle_status.hit;
	WFIFOW(fd,32) = pl_sd->battle_status.flee;
	WFIFOW(fd,34) = pl_sd->battle_status.flee2/10;
	WFIFOW(fd,36) = pl_sd->battle_status.cri/10;
	WFIFOW(fd,38) = (2000-pl_sd->battle_status.amotion)/10;  // aspd
	WFIFOW(fd,40) = 0;  // FIXME: What is 'plusASPD' supposed to be? Maybe adelay?
	WFIFOSET(fd,packet_len(0x214));
}


/// /check (CZ_REQ_STATUS_GM).
/// Request character's status values.
/// 0213 <char name>.24B
void clif_parse_Check(int fd, map_session_data *sd)
{
	char charname[NAME_LENGTH];
	map_session_data* pl_sd;

	if(!pc_has_permission(sd, PC_PERM_USE_CHECK))
		return;

	safestrncpy(charname, RFIFOCP(fd,packet_db[RFIFOW(fd,0)].pos[0]), sizeof(charname));

	if( ( pl_sd = map_nick2sd(charname,false) ) == nullptr || pc_get_group_level(sd) < pc_get_group_level(pl_sd) )
	{
		return;
	}

	clif_check(fd, pl_sd);
}



/// MAIL SYSTEM
/// By Zephyrus
///

/// Notification about the result of adding an item to mail
/// 0255 <index>.W <result>.B (ZC_ACK_MAIL_ADD_ITEM)
/// result:
///     0 = success
///     1 = failure
/// 0a05 <result>.B <index>.W <amount>.W <item id>.W <item type>.B <identified>.B <damaged>.B <refine>.B (ZC_ACK_ADD_ITEM_TO_MAIL)
///		{ <card id>.W }*4 { <option index>.W <option value>.W <option parameter>.B }*5
/// result:
///		0 = success
///		1 = over weight
///		2 = fatal error
void clif_Mail_setattachment( map_session_data* sd, int index, int amount, uint8 flag ){
	int fd = sd->fd;

#if PACKETVER < 20150513
	WFIFOHEAD(fd,packet_len(0x255));
	WFIFOW(fd,0) = 0x255;
	WFIFOW(fd,2) = index;
	WFIFOB(fd,4) = flag;
	WFIFOSET(fd,packet_len(0x255));
#else
	PACKET_ZC_ACK_ADD_ITEM_RODEX p = {};

	if( flag ){
		memset( &p, 0, sizeof( p ) );
	}else{
		struct item* item = &sd->inventory.u.items_inventory[server_index(index)];

		p.index = index;
		p.count = amount;
		p.itemId = client_nameid( item->nameid );
		p.type = itemtype( item->nameid );
		p.IsIdentified = item->identify ? 1 : 0;
		p.IsDamaged = item->attribute ? 1 : 0;
		p.refiningLevel = item->refine;
		clif_addcards( &p.slot, item );
		clif_add_random_options( p.optionData, *item );

		p.weight = 0;
		for( int i = 0; i < MAIL_MAX_ITEM; i++ ){
			if( sd->mail.item[i].nameid == 0 ){
				continue;
			}

			p.weight += sd->mail.item[i].amount * ( sd->inventory_data[sd->mail.item[i].index]->weight / 10 );
		}
		p.favorite = ( item->favorite != 0 ) ? 1 : 0;
		p.location = pc_equippoint( sd, server_index( index ) );
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
		p.grade = item->enchantgrade;
#endif
	}

	p.PacketType = HEADER_ZC_ACK_ADD_ITEM_RODEX;
	p.result = flag;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}


/// Notification about the result of retrieving a mail attachment
/// 0245 <result>.B (ZC_MAIL_REQ_GET_ITEM)
/// result:
///     0 = success
///     1 = failure
///     2 = too many items
/// 09f2 <mail id>.Q <mail tab>.B <result>.B (ZC_ACK_ZENY_FROM_MAIL)
/// 09f4 <mail id>.Q <mail tab>.B <result>.B (ZC_ACK_ITEM_FROM_MAIL)
void clif_mail_getattachment(map_session_data* sd, struct mail_message *msg, uint8 result, enum mail_attachment_type type) {
#if PACKETVER < 20150513
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x245));
	WFIFOW(fd,0) = 0x245;
	WFIFOB(fd,2) = result;
	WFIFOSET(fd,packet_len(0x245));
#else
	switch( type ){
		case MAIL_ATT_ITEM:
		case MAIL_ATT_ZENY:
			break;
		default:
			return;
	}

	PACKET_ZC_ACK_ITEM_FROM_MAIL p = { 0 };

	p.PacketType = type == MAIL_ATT_ZENY ? rodexgetzeny : rodexgetitem;
	p.MailID = msg->id;
	p.opentype = msg->type;
	p.result = result;
	clif_send(&p, sizeof(p), &sd->bl, SELF);

	clif_Mail_refreshinbox( sd, msg->type, 0 );
#endif
}


/// Notification about the result of sending a mail
/// 0249 <result>.B (ZC_MAIL_REQ_SEND)
/// result:
///     0 = success
///     1 = recipinent does not exist
/// 09ed <result>.B (ZC_ACK_WRITE_MAIL)
void clif_Mail_send(map_session_data* sd, enum mail_send_result result){
#if PACKETVER < 20150513
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x249));
	WFIFOW(fd,0) = 0x249;
	WFIFOB(fd,2) = result != WRITE_MAIL_SUCCESS;
	WFIFOSET(fd,packet_len(0x249));
#else
	PACKET_ZC_WRITE_MAIL_RESULT p = { 0 };

	p.PacketType = rodexwriteresult;
	p.result = result;
	clif_send(&p, sizeof(p), &sd->bl, SELF);
#endif
}


/// Notification about the result of deleting a mail.
/// 0257 <mail id>.L <result>.W (ZC_ACK_MAIL_DELETE)
/// result:
///     0 = success
///     1 = failure
// 09f6 <mail tab>.B <mail id>.Q (ZC_ACK_DELETE_MAIL)
void clif_mail_delete( map_session_data* sd, struct mail_message *msg, bool success ){
#if PACKETVER < 20150513
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x257));
	WFIFOW(fd,0) = 0x257;
	WFIFOL(fd,2) = msg->id;
	WFIFOW(fd,6) = !success;
	WFIFOSET(fd, packet_len(0x257));
#else
	// This is only a success notification
	if( success ){
		PACKET_ZC_ACK_DELETE_MAIL p = { 0 };

		p.PacketType = rodexdelete;
		p.opentype = msg->type;
		p.MailID = msg->id;
		clif_send(&p, sizeof(p), &sd->bl, SELF);
	}
#endif
}


/// Notification about the result of returning a mail (ZC_ACK_MAIL_RETURN).
/// 0274 <mail id>.L <result>.W
/// result:
///     0 = success
///     1 = failure
void clif_Mail_return(int fd, int mail_id, short fail)
{
	WFIFOHEAD(fd,packet_len(0x274));
	WFIFOW(fd,0) = 0x274;
	WFIFOL(fd,2) = mail_id;
	WFIFOW(fd,6) = fail;
	WFIFOSET(fd,packet_len(0x274));
}

/// Notification about new mail.
/// 024a <mail id>.L <title>.40B <sender>.24B (ZC_MAIL_RECEIVE)
/// 09e7 <result>.B (ZC_NOTIFY_UNREADMAIL)
void clif_Mail_new(map_session_data* sd, int mail_id, const char *sender, const char *title){
#if PACKETVER < 20150513
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x24a));
	WFIFOW(fd,0) = 0x24a;
	WFIFOL(fd,2) = mail_id;
	safestrncpy(WFIFOCP(fd,6), title, MAIL_TITLE_LENGTH);
	safestrncpy(WFIFOCP(fd,46), sender, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x24a));
#else
	PACKET_ZC_NOTIFY_UNREADMAIL p = { 0 };

	p.PacketType = rodexicon;
	p.result = sd->mail.inbox.unread > 0 || sd->mail.inbox.unchecked > 0; // unread
	clif_send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

/// Opens/closes the mail window (ZC_MAIL_WINDOWS).
/// 0260 <type>.L
/// type:
///     0 = open
///     1 = close
void clif_Mail_window(int fd, int flag)
{
	WFIFOHEAD(fd,packet_len(0x260));
	WFIFOW(fd,0) = 0x260;
	WFIFOL(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x260));
}

/// Lists mails stored in inbox.
/// 0240 <packet len>.W <amount>.L { <mail id>.L <title>.40B <read>.B <sender>.24B <time>.L }*amount (ZC_MAIL_REQ_GET_LIST)
/// read:
///     0 = unread
///     1 = read
/// 09f0 <packet len>.W <type>.B <amount>.B <last page>.B (ZC_ACK_MAIL_LIST)
///		{ <mail id>.Q <read>.B <type>.B <sender>.24B <received>.L <expires>.L <title length>.W <title>.?B }*
/// 0a7d <packet len>.W <type>.B <amount>.B <last page>.B (ZC_ACK_MAIL_LIST2)
///		{ <mail id>.Q <read>.B <type>.B <sender>.24B <received>.L <expires>.L <title length>.W <title>.?B }*
/// 0ac2 <packet len>.W <unknown>.B (ZC_ACK_MAIL_LIST3)
///		{ <type>.B <mail id>.Q <read>.B <type>.B <sender>.24B <expires>.L <title length>.W <title>.?B }*
void clif_Mail_refreshinbox(map_session_data *sd,enum mail_inbox_type type,int64 mailID){
#if PACKETVER < 20150513
	int fd = sd->fd;
	struct mail_data *md = &sd->mail.inbox;
	struct mail_message *msg;
	int len, i, j;

	len = 8 + (73 * md->amount);

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x240;
	WFIFOW(fd,2) = len;
	WFIFOL(fd,4) = md->amount;
	for( i = j = 0; i < MAIL_MAX_INBOX && j < md->amount; i++ )
	{
		msg = &md->msg[i];
		if (msg->id < 1)
			continue;

		WFIFOL(fd,8+73*j) = msg->id;
		memcpy(WFIFOP(fd,12+73*j), msg->title, MAIL_TITLE_LENGTH);
		WFIFOB(fd,52+73*j) = (msg->status != MAIL_UNREAD);
		safestrncpy(WFIFOCP(fd,53+73*j), msg->send_name, NAME_LENGTH);
		WFIFOL(fd,77+73*j) = (uint32)msg->timestamp;
		j++;
	}
	WFIFOSET(fd,len);

	if( md->full ) {// TODO: is this official?
		char output[100];
		safesnprintf(output,sizeof(output),"Inbox is full (Max %d). Delete some mails.", MAIL_MAX_INBOX);
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	}
#else
	int fd = sd->fd;
	struct mail_data *md = &sd->mail.inbox;
	struct mail_message *msg;
	int i, j, k, offset, titleLength;
	uint8 mailType, amount, remaining;
	uint32 now = (uint32)time(nullptr);
#if PACKETVER >= 20170419
	int cmd = 0xac2;
#elif PACKETVER >= 20160601
	int cmd = 0xa7d;
#else
	int cmd = 0x9f0;
#endif

	if( battle_config.mail_daily_count ){
		mail_refresh_remaining_amount(sd);
	}

#if PACKETVER >= 20170419
	// Always send all
	i = md->amount;
#else
	// If a starting mail id was sent
	if( mailID != 0 ){
		ARR_FIND( 0, md->amount, i, md->msg[i].id == mailID );

		// Unknown mail
		if( i == md->amount ){
			// Ignore the request for now
			return; // TODO: Should we just show the first page instead?
		}

		// It was actually the oldest/first mail, there is no further page
		if( i == 0 ){
			return;
		}

		// We actually want the next/older mail
		i -= 1;
	}else{
		i = md->amount;
	}
#endif
	
	// Count the remaining mails from the starting mail or the beginning
	// Only count mails of the target type(before 2017-04-19) and those that should not have been deleted already
	for( j = i, remaining = 0; j >= 0; j-- ){
		msg = &md->msg[j];

		if (msg->id < 1)
			continue;
#if PACKETVER < 20170419
		if (msg->type != type)
			continue;
#endif
		if (msg->scheduled_deletion > 0 && msg->scheduled_deletion <= now)
			continue;

		remaining++;
	}

#if PACKETVER >= 20170419
	// Always send all
	amount = remaining;
#else
	if( remaining > MAIL_PAGE_SIZE ){
		amount = MAIL_PAGE_SIZE;
	}else{
		amount = remaining;
	}
#endif

	WFIFOHEAD(fd, 7 + ((44 + MAIL_TITLE_LENGTH) * amount));
	WFIFOW(fd, 0) = cmd;
#if PACKETVER >= 20170419
	WFIFOB(fd, 4) = 1; // Unknown
	offset = 5;
#else
	WFIFOB(fd, 4) = type;
	WFIFOB(fd, 5) = amount;
	WFIFOB(fd, 6) = ( remaining < MAIL_PAGE_SIZE ); // last page
	offset = 7;
#endif

	for( amount = 0; i >= 0; i-- ){
		msg = &md->msg[i];

		if (msg->id < 1)
			continue;
#if PACKETVER < 20170419
		if (msg->type != type)
			continue;
#endif
		if (msg->scheduled_deletion > 0 && msg->scheduled_deletion <= now)
			continue;

#if PACKETVER >= 20170419
		WFIFOB(fd, offset) = msg->type;
		offset += 1;
#endif

		WFIFOQ(fd, offset + 0) = (uint64)msg->id;
		WFIFOB(fd, offset + 8) = (msg->status != MAIL_UNREAD);

		mailType = MAIL_TYPE_TEXT; // Normal letter

		if( msg->zeny > 0 ){
			mailType |= MAIL_TYPE_ZENY; // Mail contains zeny
		}

		for( k = 0; k < MAIL_MAX_ITEM; k++ ){
			if( msg->item[k].nameid > 0 ){
				mailType |= MAIL_TYPE_ITEM; // Mail contains items
				break;
			}
		}

#if PACKETVER >= 20170419
		// If it came from an npc?
		if( !msg->send_id ){
			mailType |= MAIL_TYPE_NPC;
		}
#endif

		WFIFOB(fd, offset + 9) = mailType;
		safestrncpy(WFIFOCP(fd, offset + 10), msg->send_name, NAME_LENGTH);

#if PACKETVER < 20170419
		// How much time has passed since you received the mail
		WFIFOL(fd, offset + 34 ) = now - (uint32)msg->timestamp;
		offset += 4;
#endif

		// If automatic return/deletion of mails is enabled, notify the client when it will kick in
		if( msg->scheduled_deletion > 0 ){
			WFIFOL(fd, offset + 34) = (uint32)msg->scheduled_deletion - now;
		}else{
			// Fake the scheduled deletion to one year in the future
			// Sadly the client always displays the scheduled deletion after 24 hours no matter how high this value gets [Lemongrass]
			WFIFOL(fd, offset + 34) = 365 * 24 * 60 * 60;
		}

		WFIFOW(fd, offset + 38) = titleLength = (int16)(strlen(msg->title) + 1);
		safestrncpy(WFIFOCP(fd, offset + 40), msg->title, titleLength);

		offset += 40 + titleLength;
	}
	WFIFOW(fd, 2) = (int16)offset;
	WFIFOSET(fd, offset);
#endif
}

/// Mail inbox list request.
/// 023f (CZ_MAIL_GET_LIST)
/// 09e8 <mail tab>.B <mail id>.Q (CZ_OPEN_MAILBOX)
/// 09ee <mail tab>.B <mail id>.Q (CZ_REQ_NEXT_MAIL_LIST)
/// 09ef <mail tab>.B <mail id>.Q (CZ_REQ_REFRESH_MAIL_LIST)
/// 0ac0 <mail id>.Q <unknown>.16B (CZ_OPEN_MAILBOX2)
/// 0ac1 <mail id>.Q <unknown>.16B (CZ_REQ_REFRESH_MAIL_LIST2)
void clif_parse_Mail_refreshinbox(int fd, map_session_data *sd){
	if( mail_invalid_operation( sd ) ){
		return;
	}

#if PACKETVER < 20150513
	struct mail_data* md = &sd->mail.inbox;

	if( md->amount < MAIL_MAX_INBOX && (md->full || sd->mail.changed) )
		intif_Mail_requestinbox(sd->status.char_id, 1, MAIL_INBOX_NORMAL);
	else
		clif_Mail_refreshinbox(sd, MAIL_INBOX_NORMAL,0);

	mail_removeitem(sd, 0, sd->mail.item[0].index, sd->mail.item[0].amount);
	mail_removezeny(sd, false);
#else
	int cmd = RFIFOW(fd, 0);
#if PACKETVER < 20170419
	uint8 openType = RFIFOB(fd, 2);
	uint64 mailId = RFIFOQ(fd, 3);
#else
	uint8 openType;
	uint64 mailId = RFIFOQ(fd, 2);
	int i;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mailId);

	if( i == MAIL_MAX_INBOX ){
		openType = MAIL_INBOX_NORMAL;
		mailId = 0;
	}else{
		openType = sd->mail.inbox.msg[i].type;
		mailId = 0;
	}
#endif

	switch( openType ){
		case MAIL_INBOX_NORMAL:
		case MAIL_INBOX_ACCOUNT:
		case MAIL_INBOX_RETURNED:
			break;
		default:
			// Unknown type: ignore request
			return;
	}

	if( sd->mail.changed || ( cmd == 0x9ef || cmd == 0xac1 ) ){
		intif_Mail_requestinbox(sd->status.char_id, 1, (enum mail_inbox_type)openType);
		return;
	}

	// If it is not a next page request
	if( cmd != 0x9ee ){
		mailId = 0;
	}

	clif_Mail_refreshinbox(sd,(enum mail_inbox_type)openType,mailId);
#endif
}

/// Opens a mail
/// 0242 <packet len>.W <mail id>.L <title>.40B <sender>.24B <time>.L <zeny>.L (ZC_MAIL_REQ_OPEN)
///     <amount>.L <name id>.W <item type>.W <identified>.B <damaged>.B <refine>.B
///     <card1>.W <card2>.W <card3>.W <card4>.W <message>.?B
/// 09eb <packet len>.W <type>.B <mail id>.Q <message length>.W <zeny>.Q <item count>.B (ZC_ACK_READ_MAIL)
///		{  }*n
// TODO: Packet description => for repeated block
void clif_Mail_read( map_session_data *sd, int mail_id ){
	int i, fd = sd->fd;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if( i == MAIL_MAX_INBOX ) {
		clif_Mail_return(sd->fd, mail_id, 1); // Mail doesn't exist
		ShowWarning("clif_parse_Mail_read: char '%s' trying to read a message not the inbox.\n", sd->status.name);
		return;
	} else {
		struct mail_message *msg = &sd->mail.inbox.msg[i];
		struct item *item;
		int16 msg_len;

		msg_len = static_cast<decltype(msg_len)>( strlen(msg->body) );

		if( msg_len == 0 ) {
			strcpy(msg->body, "(no message)"); // TODO: confirm for RODEX
			msg_len = static_cast<decltype(msg_len)>( strlen(msg->body) );
		}

#if PACKETVER < 20150513
		item = &msg->item[0];

		int16 len = 101 + msg_len;

		WFIFOHEAD(fd,len);
		WFIFOW(fd,0) = 0x242;
		WFIFOW(fd,2) = len;
		WFIFOL(fd,4) = msg->id;
		safestrncpy(WFIFOCP(fd,8), msg->title, MAIL_TITLE_LENGTH + 1);
		safestrncpy(WFIFOCP(fd,48), msg->send_name, NAME_LENGTH + 1);
		WFIFOL(fd,72) = 0;
		WFIFOL(fd,76) = msg->zeny;

		std::shared_ptr<item_data> data = item_db.find(item->nameid);

		if( item->nameid && data != nullptr ) {
			WFIFOL(fd,80) = item->amount;
			WFIFOW(fd,84) = client_nameid( item->nameid );
			WFIFOW(fd,86) = itemtype( item->nameid );
			WFIFOB(fd,88) = item->identify;
			WFIFOB(fd,89) = item->attribute;
			WFIFOB(fd,90) = item->refine;
			WFIFOW(fd,91) = item->card[0];
			WFIFOW(fd,93) = item->card[1];
			WFIFOW(fd,95) = item->card[2];
			WFIFOW(fd,97) = item->card[3];
		} else // no item, set all to zero
			memset(WFIFOP(fd,80), 0x00, 19);

		WFIFOB(fd,99) = (unsigned char)msg_len;
		safestrncpy(WFIFOCP(fd,100), msg->body, msg_len + 1);
		WFIFOSET(fd,len);
#else
		msg_len += 1; // Zero Termination

		PACKET_ZC_ACK_READ_RODEX* p = reinterpret_cast<PACKET_ZC_ACK_READ_RODEX*>( packet_buffer );

		p->PacketType = HEADER_ZC_ACK_READ_RODEX;
		p->PacketLength = sizeof( *p );
		p->opentype = msg->type;
		p->MailID = msg->id;
		p->TextcontentsLength = static_cast<decltype(p->TextcontentsLength)>( msg_len );
		p->zeny = msg->zeny;

		safestrncpy( WBUFCP( p, p->PacketLength ), msg->body, msg_len );
		p->PacketLength += p->TextcontentsLength;

		p->ItemCnt = 0;
		for( int j = 0; j < MAIL_MAX_ITEM; j++ ){
			item = &msg->item[j];

			std::shared_ptr<item_data> data = item_db.find(item->nameid);

			if( item->nameid > 0 && item->amount > 0 && data != nullptr ){
				PACKET_ZC_ACK_READ_RODEX_SUB* mailitem = reinterpret_cast<PACKET_ZC_ACK_READ_RODEX_SUB*>( WBUFP( p, p->PacketLength ) );

				mailitem->ITID = client_nameid( item->nameid );
				mailitem->count = item->amount;
				mailitem->type = itemtype( item->nameid );
				mailitem->IsIdentified = item->identify ? 1 : 0;
				mailitem->IsDamaged = item->attribute ? 1 : 0;
				mailitem->refiningLevel = item->refine;
				mailitem->location = pc_equippoint_sub( sd, data.get() );
				mailitem->viewSprite = data->look;
				mailitem->bindOnEquip = item->bound ? 2 : data->flag.bindOnEquip ? 1 : 0;
				clif_addcards( &mailitem->slot, item );
				clif_add_random_options( mailitem->option_data, *item );
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
				mailitem->grade = item->enchantgrade;
#endif

				p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof( *mailitem ) );
				p->ItemCnt++;
			}
		}

		clif_send( p, p->PacketLength, &sd->bl, SELF );
#endif

		if (msg->status == MAIL_UNREAD) {
			msg->status = MAIL_READ;
			intif_Mail_read(mail_id);
			clif_parse_Mail_refreshinbox(fd, sd);

			sd->mail.inbox.unread--;
			clif_Mail_new(sd, 0, msg->send_name, msg->title);
		}
	}
}


/// Request to open a mail.
/// 0241 <mail id>.L (CZ_MAIL_OPEN)
/// 09ea <mail tab>.B <mail id>.Q (CZ_REQ_READ_MAIL)
void clif_parse_Mail_read(int fd, map_session_data *sd){
#if PACKETVER < 20150513
	int mail_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
#else
	//uint8 openType = RFIFOB(fd, 2);
	int mail_id = (int)RFIFOQ(fd, 3);
#endif

	if( mail_id <= 0 )
		return;
	if( mail_invalid_operation(sd) )
		return;

	clif_Mail_read(sd, mail_id);
}

/// Allow a player to begin writing a mail
/// 0a12 <receiver>.24B <success>.B (ZC_ACK_OPEN_WRITE_MAIL)
void clif_send_Mail_beginwrite_ack( map_session_data *sd, char* name, bool success ){
	PACKET_ZC_ACK_OPEN_WRITE_MAIL p = { 0 };

	p.PacketType = rodexopenwrite;
	safestrncpy(p.receiveName, name, NAME_LENGTH);
	p.result = success;
	clif_send(&p, sizeof(p), &sd->bl, SELF);
}

/// Request to start writing a mail
/// 0a08 <receiver>.24B (CZ_REQ_OPEN_WRITE_MAIL)
void clif_parse_Mail_beginwrite( int fd, map_session_data *sd ){
	char name[NAME_LENGTH];

	safestrncpy(name, RFIFOCP(fd, 2), NAME_LENGTH);

	if( mail_invalid_operation( sd ) ){
		return;
	}

	if( sd->state.storage_flag || sd->state.mail_writing || sd->trade_partner ){
		clif_send_Mail_beginwrite_ack(sd, name, false);
		return;
	}

	mail_clear(sd);

	sd->state.mail_writing = true;
	clif_send_Mail_beginwrite_ack(sd, name,true);
}

/// Notification that the client cancelled writing a mail
/// 0a03 (CZ_REQ_CANCEL_WRITE_MAIL)
void clif_parse_Mail_cancelwrite( int fd, map_session_data *sd ){
	sd->state.mail_writing = false;
}

/// Give the client information about the recipient, if available
/// 0a14 <char id>.L <class>.W <base level>.W (ZC_CHECK_RECEIVE_CHARACTER_NAME)
/// 0a51 <char id>.L <class>.W <base level>.W <name>.24B (ZC_CHECK_RECEIVE_CHARACTER_NAME2)
void clif_Mail_Receiver_Ack( map_session_data* sd, uint32 char_id, short class_, uint32 level, const char* name ){
#if PACKETVER >= 20141119
	PACKET_ZC_CHECKNAME p = { 0 };

	p.PacketType = HEADER_ZC_CHECKNAME;
	p.CharId = char_id;
	p.Class = class_;
	p.BaseLevel = level;
#if PACKETVER >= 20160302
	strncpy(p.Name, name, NAME_LENGTH);
#endif
	clif_send(&p, sizeof(p), &sd->bl, SELF);
#endif
}

/// Request information about the recipient
/// 0a13 <name>.24B (CZ_CHECK_RECEIVE_CHARACTER_NAME)
void clif_parse_Mail_Receiver_Check(int fd, map_session_data *sd) {
#if PACKETVER >= 20140423
#if PACKETVER_MAIN_NUM >= 20201104 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20201118
	const PACKET_CZ_CHECKNAME2* p = reinterpret_cast<PACKET_CZ_CHECKNAME2*>( RFIFOP( fd, 0 ) );
#else
	const PACKET_CZ_CHECKNAME1* p = reinterpret_cast<PACKET_CZ_CHECKNAME1*>( RFIFOP( fd, 0 ) );
#endif
	static char name[NAME_LENGTH];

	if( mail_invalid_operation( sd ) ){
		return;
	}

	safestrncpy(name, p->Name, NAME_LENGTH);

	intif_mail_checkreceiver(sd, name);
#endif
}

/// Request to receive mail's attachment.
/// 0244 <mail id>.L (CZ_MAIL_GET_ITEM)
/// 09f1 <mail id>.Q <mail tab>.B (CZ_REQ_ZENY_FROM_MAIL)
/// 09f3 <mail id>.Q <mail tab>.B (CZ_REQ_ITEM_FROM_MAIL)
void clif_parse_Mail_getattach( int fd, map_session_data *sd ){
	int i;
	struct mail_message* msg;
#if PACKETVER < 20150513
	int mail_id = RFIFOL(fd, packet_db[RFIFOW(fd, 0)].pos[0]);
	int attachment = MAIL_ATT_ALL;
#else
	uint16 packet_id = RFIFOW(fd, 0);
	int mail_id = (int)RFIFOQ(fd, 2);
	//int openType = RFIFOB(fd, 10);
	int attachment = packet_id == 0x9f1 ? MAIL_ATT_ZENY : packet_id == 0x9f3 ? MAIL_ATT_ITEM : MAIL_ATT_NONE;
#endif

	if( !chrif_isconnected() )
		return;
	if( mail_id <= 0 )
		return;
	if( mail_invalid_operation(sd) )
		return;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if( i == MAIL_MAX_INBOX )
		return;

	msg = &sd->mail.inbox.msg[i];

	if( attachment&MAIL_ATT_ZENY && msg->zeny < 1 ){
		attachment &= ~MAIL_ATT_ZENY;
	}
	
	if( attachment&MAIL_ATT_ITEM ){
		ARR_FIND(0, MAIL_MAX_ITEM, i, msg->item[i].nameid > 0 || msg->item[i].amount > 0);

		// No items were found
		if( i == MAIL_MAX_ITEM ){
			attachment &= ~MAIL_ATT_ITEM;
		}
	}

	// Either no attachment requested at all or there are no zeny or items in the mail
	if( attachment == MAIL_ATT_NONE ){
		return;
	}

	if( attachment&MAIL_ATT_ZENY ){
		if( ( msg->zeny + sd->status.zeny + sd->mail.pending_zeny ) > MAX_ZENY ){
			clif_mail_getattachment(sd, msg, 1, MAIL_ATT_ZENY); //too many zeny
			return;
		}else{
			// Store the pending zeny (required for the "retrieve all" feature)
			sd->mail.pending_zeny += msg->zeny;

			// To make sure another request fails
			msg->zeny = 0;
		}
	}

	if( attachment&MAIL_ATT_ITEM ){
		int slots = 0, totalweight = 0;

		for( i = 0; i < MAIL_MAX_ITEM; i++ ){
			struct item* item = &msg->item[i];

			if( item->nameid > 0 && item->amount > 0 ){
				std::shared_ptr<item_data> data = item_db.find(item->nameid);

				if(data == nullptr)
					continue;

				switch( pc_checkadditem(sd, item->nameid, item->amount) ){
					case CHKADDITEM_NEW:
						slots += data->inventorySlotNeeded( item->amount );
						break;
					case CHKADDITEM_OVERAMOUNT:
						clif_mail_getattachment(sd, msg, 2, MAIL_ATT_ITEM);
						return;
				}

				totalweight += data->weight * item->amount;
			}
		}

		if( ( totalweight + sd->weight + sd->mail.pending_weight ) > sd->max_weight ){
			clif_mail_getattachment(sd, msg, 2, MAIL_ATT_ITEM);
			return;
		}else if( pc_inventoryblank(sd) < ( slots + sd->mail.pending_slots ) ){
			clif_mail_getattachment(sd, msg, 2, MAIL_ATT_ITEM);
			return;
		}

		// To make sure another request fails
		memset(msg->item, 0, MAIL_MAX_ITEM*sizeof(struct item));

		// Store the pending weight (required for the "retrieve all" feature)
		sd->mail.pending_weight += totalweight;
		// Store the pending slots (required for the "retrieve all" feature)
		sd->mail.pending_slots += slots;
	}

	intif_mail_getattach(sd,msg, (enum mail_attachment_type)attachment);
}


/// Request to delete a mail.
/// 0243 <mail id>.L (CZ_MAIL_DELETE)
/// 09f5 <mail tab>.B <mail id>.Q (CZ_REQ_DELETE_MAIL)
void clif_parse_Mail_delete(int fd, map_session_data *sd){
#if PACKETVER < 20150513
	int mail_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
#else
	//int openType = RFIFOB(fd, 2);
	int mail_id = (int)RFIFOQ(fd, 3);
#endif
	int i, j;

	if( !chrif_isconnected() )
		return;
	if( mail_id <= 0 )
		return;
	if( mail_invalid_operation(sd) )
		return;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if (i < MAIL_MAX_INBOX) {
		struct mail_message *msg = &sd->mail.inbox.msg[i];

		// can't delete mail without removing zeny first
		if( msg->zeny > 0 ){
			clif_mail_delete(sd, msg, false);
			return;
		}

		// can't delete mail without removing attachment(s) first
		for( j = 0; j < MAIL_MAX_ITEM; j++ ){
			if( msg->item[j].nameid > 0 && msg->item[j].amount > 0 ){
				clif_mail_delete(sd, msg, false);
				return;
			}
		}

		if( intif_Mail_delete(sd->status.char_id, mail_id) && msg->status == MAIL_UNREAD ){
			sd->mail.inbox.unread--;
			clif_Mail_new(sd,0,nullptr,nullptr);
		}
	}
}


/// Request to return a mail.
/// 0273 <mail id>.L <receive name>.24B (CZ_REQ_MAIL_RETURN)
void clif_parse_Mail_return(int fd, map_session_data *sd){
#if PACKETVER_MAIN_NUM >= 20201104 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20201118
	const PACKET_CZ_RODEX_RETURN* p = reinterpret_cast<PACKET_CZ_RODEX_RETURN*>( RFIFOP( fd, 0 ) );

	//ShowDump( p, sizeof( p ) );

	int mail_id = p->msgId;

	// not supported for now
	return;
#else
	int mail_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
#endif
	//char *rec_name = RFIFOP(fd,packet_db[RFIFOW(fd,0)].pos[1]);
	int i;

	if( mail_id <= 0 )
		return;
	if( mail_invalid_operation(sd) )
		return;

	ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
	if( i < MAIL_MAX_INBOX && sd->mail.inbox.msg[i].send_id != 0 )
		intif_Mail_return(sd->status.char_id, mail_id);
	else
		clif_Mail_return(sd->fd, mail_id, 1);
}


/// Request to add an item or Zeny to mail.
/// 0247 <index>.W <amount>.L (CZ_MAIL_ADD_ITEM)
/// 0a04 <index>.W <amount>.W (CZ_REQ_ADD_ITEM_TO_MAIL)
void clif_parse_Mail_setattach(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	uint16 idx = RFIFOW(fd,info->pos[0]);
#if PACKETVER < 20150513
	int amount = RFIFOL(fd,info->pos[1]);
#else
	int amount = RFIFOW(fd,info->pos[1]);
#endif
	unsigned char flag;

	if( !chrif_isconnected() )
		return;
	if (amount < 0 || server_index(idx) >= MAX_INVENTORY)
		return;

	if (sd->inventory_data[server_index(idx)] == nullptr)
		return;

	if( mail_invalid_operation( sd ) ){
		return;
	}

	flag = mail_setitem(sd, idx, amount);

	if( flag == MAIL_ATTACH_EQUIPSWITCH ){
		clif_msg( sd, MSI_SWAP_EQUIPITEM_UNREGISTER_FIRST );
	}else{
		clif_Mail_setattachment(sd,idx,amount,flag);
	}
}

/// Remove an item from a mail
/// 0a07 <result>.B <index>.W <amount>.W <weight>.W
void clif_mail_removeitem( map_session_data* sd, bool success, int index, int amount ){
	PACKET_ZC_ACK_REMOVE_ITEM_MAIL p = { 0 };

	p.PacketType = rodexremoveitem;
	p.result = success;
	p.index = index;
	p.cnt = amount;

	int total = 0;
	for( int i = 0; i < MAIL_MAX_ITEM; i++ ){
		if( sd->mail.item[i].nameid == 0 ){
			break;
		}

		total += sd->mail.item[i].amount * ( sd->inventory_data[sd->mail.item[i].index]->weight / 10 );
	}

	p.weight = total;
	clif_send(&p, sizeof(p), &sd->bl, SELF);
}

/// Request to reset mail item and/or Zeny
/// 0246 <type>.W (CZ_MAIL_RESET_ITEM)
/// type:
///     0 = reset all
///     1 = remove item
///     2 = remove zeny
/// 0a06 <index>.W <amount>.W (CZ_REQ_REMOVE_ITEM_MAIL)
void clif_parse_Mail_winopen(int fd, map_session_data *sd)
{
#if PACKETVER < 20150513
	int type = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	if (type == 0 || type == 1)
		mail_removeitem(sd, 0, sd->mail.item[0].index, sd->mail.item[0].amount);
	if (type == 0 || type == 2)
		mail_removezeny(sd, false);
#else
	uint16 index = RFIFOW(fd, 2);
	uint16 count = RFIFOW(fd, 4);

	mail_removeitem(sd,0,index,count);
#endif
}

/// Request to send mail
/// 0248 <packet len>.W <recipient>.24B <title>.40B <body len>.B <body>.?B (CZ_MAIL_SEND)
/// 09ec <packet len>.W <recipient>.24B <sender>.24B <zeny>.Q <title length>.W <body length>.W <title>.?B <body>.?B (CZ_REQ_WRITE_MAIL)
/// 0a6e <packet len>.W <recipient>.24B <sender>.24B <zeny>.Q <title length>.W <body length>.W <char id>.L <title>.?B <body>.?B (CZ_REQ_WRITE_MAIL2)
void clif_parse_Mail_send(int fd, map_session_data *sd){
#if PACKETVER < 20150513
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	if( !chrif_isconnected() )
		return;

	if( RFIFOW(fd,info->pos[0]) < 69 ) {
		ShowWarning("Invalid Msg Len from account %d.\n", sd->status.account_id);
		return;
	}

	mail_send(sd, RFIFOCP(fd,info->pos[1]), RFIFOCP(fd,info->pos[2]), RFIFOCP(fd,info->pos[4]), RFIFOB(fd,info->pos[3]));
#else
	uint16 length = RFIFOW(fd, 2);

	if( length < 0x3e ){
		ShowWarning("Too short...\n");
		clif_Mail_send(sd, WRITE_MAIL_FAILED);
		return;
	}

	if( mail_invalid_operation( sd ) ){
		return;
	}

	// Forged request without a begin writing packet?
	if( !sd->state.mail_writing ){
		return; // Ignore it
	}

	char receiver[NAME_LENGTH];

	safestrncpy(receiver, RFIFOCP(fd, 4), NAME_LENGTH);

//	char sender[NAME_LENGTH];

//	safestrncpy(sender, RFIFOCP(fd, 28), NAME_LENGTH);

	uint64 zeny = RFIFOQ(fd, 52);
	uint16 titleLength = RFIFOW(fd, 60);
	uint16 textLength = RFIFOW(fd, 62);
	uint16 realTitleLength = min(titleLength, MAIL_TITLE_LENGTH);
	uint16 realTextLength = min(textLength, MAIL_BODY_LENGTH);

	char title[MAIL_TITLE_LENGTH];
	char text[MAIL_BODY_LENGTH];

#if PACKETVER <= 20160330
	safestrncpy(title, RFIFOCP(fd, 64), realTitleLength);
	safestrncpy(text, RFIFOCP(fd, 64 + titleLength), realTextLength);
#else
	// 64 = <char id>.L
	safestrncpy(title, RFIFOCP(fd, 68), realTitleLength);
	safestrncpy(text, RFIFOCP(fd, 68 + titleLength), realTextLength);
#endif

	if( zeny > 0 ){
		if( mail_setitem(sd,0,(uint32)zeny) != MAIL_ATTACH_SUCCESS ){
			clif_Mail_send(sd,WRITE_MAIL_FAILED);
			return;
		}
	}

	mail_send(sd, receiver, title, text, realTextLength);
#endif
}


/// AUCTION SYSTEM
/// By Zephyrus
///

/// Opens/closes the auction window (ZC_AUCTION_WINDOWS).
/// 025f <type>.L
/// type:
///     0 = open
///     1 = close
void clif_Auction_openwindow(map_session_data *sd)
{
	int fd = sd->fd;

	if( sd->state.storage_flag || sd->state.vending || sd->state.buyingstore || sd->state.trading )
		return;

	if( !battle_config.feature_auction )
		return;

	WFIFOHEAD(fd,packet_len(0x25f));
	WFIFOW(fd,0) = 0x25f;
	WFIFOL(fd,2) = 0;
	WFIFOSET(fd,packet_len(0x25f));
}


/// Returns auction item search results (ZC_AUCTION_ITEM_REQ_SEARCH).
/// 0252 <packet len>.W <pages>.L <count>.L { <auction id>.L <seller name>.24B <name id>.W <type>.L <amount>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <now price>.L <max price>.L <buyer name>.24B <delete time>.L }*
void clif_Auction_results(map_session_data *sd, short count, short pages, uint8 *buf)
{
	int i, fd = sd->fd, len = sizeof(struct auction_data);
	struct auction_data auction;

	WFIFOHEAD(fd,12 + (count * 83));
	WFIFOW(fd,0) = 0x252;
	WFIFOW(fd,2) = 12 + (count * 83);
	WFIFOL(fd,4) = pages;
	WFIFOL(fd,8) = count;

	for( i = 0; i < count; i++ ) {
		int k = 12 + (i * 83);
		memcpy(&auction, RBUFP(buf,i * len), len);

		WFIFOL(fd,k) = auction.auction_id;
		safestrncpy(WFIFOCP(fd,4+k), auction.seller_name, NAME_LENGTH);			
		WFIFOW(fd,28+k) = client_nameid( auction.item.nameid );
		WFIFOL(fd,30+k) = auction.type;
		WFIFOW(fd,34+k) = auction.item.amount; // Always 1
		WFIFOB(fd,36+k) = auction.item.identify;
		WFIFOB(fd,37+k) = auction.item.attribute;
		WFIFOB(fd,38+k) = auction.item.refine;
		WFIFOW(fd,39+k) = auction.item.card[0];
		WFIFOW(fd,41+k) = auction.item.card[1];
		WFIFOW(fd,43+k) = auction.item.card[2];
		WFIFOW(fd,45+k) = auction.item.card[3];
		WFIFOL(fd,47+k) = auction.price;
		WFIFOL(fd,51+k) = auction.buynow;
		safestrncpy(WFIFOCP(fd,55+k), auction.buyer_name, NAME_LENGTH);
		WFIFOL(fd,79+k) = (uint32)auction.timestamp;
	}
	WFIFOSET(fd,WFIFOW(fd,2));
}


/// Result from request to add an item (ZC_ACK_AUCTION_ADD_ITEM).
/// 0256 <index>.W <result>.B
/// result:
///     0 = success
///     1 = failure
static void clif_Auction_setitem(int fd, int index, bool fail)
{
	WFIFOHEAD(fd,packet_len(0x256));
	WFIFOW(fd,0) = 0x256;
	WFIFOW(fd,2) = index;
	WFIFOB(fd,4) = fail;
	WFIFOSET(fd,packet_len(0x256));
}


/// Request to initialize 'new auction' data (CZ_AUCTION_CREATE).
/// 024b <type>.W
/// type:
///     0 = create (any other action in auction window)
///     1 = cancel (cancel pressed on register tab)
///     ? = junk, uninitialized value (ex. when switching between list filters)
void clif_parse_Auction_cancelreg(int fd, map_session_data *sd){
	//int type = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if( sd->auction.amount > 0 )
		clif_additem(sd, sd->auction.index, sd->auction.amount, 0);

	sd->auction.amount = 0;
}


/// Request to add an item to the action (CZ_AUCTION_ADD_ITEM).
/// 024c <index>.W <count>.L
void clif_parse_Auction_setitem(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int idx = RFIFOW(fd,info->pos[0]) - 2;
	int amount = RFIFOL(fd,info->pos[1]); // Always 1

	if( !battle_config.feature_auction )
		return;

	if( sd->auction.amount > 0 )
		sd->auction.amount = 0;

	if( idx < 0 || idx >= MAX_INVENTORY ) {
		ShowWarning("Character %s trying to set invalid item index in auctions.\n", sd->status.name);
		return;
	}

	if( amount != 1 || amount > sd->inventory.u.items_inventory[idx].amount ) { // By client, amount is always set to 1. Maybe this is a future implementation.
		ShowWarning("Character %s trying to set invalid amount in auctions.\n", sd->status.name);
		return;
	}

	std::shared_ptr<item_data> id = item_db.find(sd->inventory.u.items_inventory[idx].nameid);

	if( id != nullptr && !(id->type == IT_ARMOR || id->type == IT_PETARMOR || id->type == IT_WEAPON || id->type == IT_CARD || id->type == IT_ETC || id->type == IT_SHADOWGEAR) )
	{ // Consumable or pets are not allowed
		clif_Auction_setitem(sd->fd, idx, true);
		return;
	}

	if( !pc_can_give_items(sd) || sd->inventory.u.items_inventory[idx].expire_time ||
			!sd->inventory.u.items_inventory[idx].identify ||
			(sd->inventory.u.items_inventory[idx].bound && !pc_can_give_bounded_items(sd)) ||
			!itemdb_available(sd->inventory.u.items_inventory[idx].nameid) ||
			!itemdb_canauction(&sd->inventory.u.items_inventory[idx],pc_get_group_level(sd)) ) { // Quest Item or something else
		clif_Auction_setitem(sd->fd, idx, true);
		return;
	}

	sd->auction.index = idx;
	sd->auction.amount = amount;
	clif_Auction_setitem(fd, idx + 2, false);
}

/// Result from an auction action (ZC_AUCTION_RESULT).
/// 0250 <result>.B
/// result:
///     0 = You have failed to bid into the auction
///     1 = You have successfully bid in the auction
///     2 = The auction has been canceled
///     3 = An auction with at least one bidder cannot be canceled
///     4 = You cannot register more than 5 items in an auction at a time
///     5 = You do not have enough Zeny to pay the Auction Fee
///     6 = You have won the auction
///     7 = You have failed to win the auction
///     8 = You do not have enough Zeny
///     9 = You cannot place more than 5 bids at a time
void clif_Auction_message(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,packet_len(0x250));
	WFIFOW(fd,0) = 0x250;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x250));
}


/// Result of the auction close request (ZC_AUCTION_ACK_MY_SELL_STOP).
/// 025e <result>.W
/// result:
///     0 = You have ended the auction
///     1 = You cannot end the auction
///     2 = Auction ID is incorrect
void clif_Auction_close(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,packet_len(0x25e));
	WFIFOW(fd,0) = 0x25d;  // BUG: The client identifies this packet as 0x25d (CZ_AUCTION_REQ_MY_SELL_STOP)
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,packet_len(0x25e));
}


/// Request to add an auction (CZ_AUCTION_ADD).
/// 024d <now money>.L <max money>.L <delete hour>.W
void clif_parse_Auction_register(int fd, map_session_data *sd)
{
	struct auction_data auction;
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];

	if( !battle_config.feature_auction )
		return;

	auction.price = RFIFOL(fd,info->pos[0]);
	auction.buynow = RFIFOL(fd,info->pos[1]);
	auction.hours = RFIFOW(fd,info->pos[2]);

	// Invalid Situations...
	if( sd->auction.amount < 1 ) {
		ShowWarning("Character %s trying to register auction without item.\n", sd->status.name);
		return;
	}

	if( auction.price >= auction.buynow ) {
		ShowWarning("Character %s trying to alter auction prices.\n", sd->status.name);
		return;
	}

	if( auction.hours < 1 || auction.hours > 48 ) {
		ShowWarning("Character %s trying to enter an invalid time for auction.\n", sd->status.name);
		return;
	}

	// Auction checks...
	if( sd->inventory.u.items_inventory[sd->auction.index].bound && !pc_can_give_bounded_items(sd) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,293));
		clif_Auction_message(fd, 2); // The auction has been canceled
		return;
	}
	if( sd->status.zeny < (auction.hours * battle_config.auction_feeperhour) ) {
		clif_Auction_message(fd, 5); // You do not have enough zeny to pay the Auction Fee.
		return;
	}

	if( auction.buynow > battle_config.auction_maximumprice ) { // Zeny Limits
		auction.buynow = battle_config.auction_maximumprice;
		if( auction.price >= auction.buynow )
			auction.price = auction.buynow - 1;
	}

	auction.auction_id = 0;
	auction.seller_id = sd->status.char_id;
	safestrncpy(auction.seller_name, sd->status.name, sizeof(auction.seller_name));
	auction.buyer_id = 0;
	memset(auction.buyer_name, '\0', sizeof(auction.buyer_name));

	if( sd->inventory.u.items_inventory[sd->auction.index].nameid == 0 || sd->inventory.u.items_inventory[sd->auction.index].amount < sd->auction.amount )
	{
		clif_Auction_message(fd, 2); // The auction has been canceled
		return;
	}

	std::shared_ptr<item_data> id = item_db.find(sd->inventory.u.items_inventory[sd->auction.index].nameid);

	if( id == nullptr )
	{ // Just in case
		clif_Auction_message(fd, 2); // The auction has been canceled
		return;
	}

	safestrncpy(auction.item_name, id->ename.c_str(), sizeof(auction.item_name));
	auction.type = id->type;
	memcpy(&auction.item, &sd->inventory.u.items_inventory[sd->auction.index], sizeof(struct item));
	auction.item.amount = 1;
	auction.timestamp = 0;

	if( !intif_Auction_register(&auction) )
		clif_Auction_message(fd, 4); // No Char Server? lets say something to the client
	else
	{
		int zeny = auction.hours*battle_config.auction_feeperhour;

		pc_delitem(sd, sd->auction.index, sd->auction.amount, 1, 6, LOG_TYPE_AUCTION);
		sd->auction.amount = 0;

		pc_payzeny(sd, zeny, LOG_TYPE_AUCTION);
	}
}


/// Cancels an auction (CZ_AUCTION_ADD_CANCEL).
/// 024e <auction id>.L
void clif_parse_Auction_cancel(int fd, map_session_data *sd){
	unsigned int auction_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	intif_Auction_cancel(sd->status.char_id, auction_id);
}


/// Closes an auction (CZ_AUCTION_REQ_MY_SELL_STOP).
/// 025d <auction id>.L
void clif_parse_Auction_close(int fd, map_session_data *sd){
	unsigned int auction_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	intif_Auction_close(sd->status.char_id, auction_id);
}


/// Places a bid on an auction (CZ_AUCTION_BUY).
/// 024f <auction id>.L <money>.L
void clif_parse_Auction_bid(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	unsigned int auction_id = RFIFOL(fd,info->pos[0]);
	int bid = RFIFOL(fd,info->pos[1]);

	if( !pc_can_give_items(sd) ) { //They aren't supposed to give zeny [Inkfish]
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return;
	}

	if( bid <= 0 )
		clif_Auction_message(fd, 0); // You have failed to bid into the auction
	else if( bid > sd->status.zeny )
		clif_Auction_message(fd, 8); // You do not have enough zeny
	else if ( CheckForCharServer() ) // char server is down (bugreport:1138)
		clif_Auction_message(fd, 0); // You have failed to bid into the auction
	else {
		pc_payzeny(sd, bid, LOG_TYPE_AUCTION);
		intif_Auction_bid(sd->status.char_id, sd->status.name, auction_id, bid);
	}
}


/// Auction Search (CZ_AUCTION_ITEM_SEARCH).
/// 0251 <search type>.W <auction id>.L <search text>.24B <page number>.W
/// search type:
///     0 = armor
///     1 = weapon
///     2 = card
///     3 = misc
///     4 = name search
///     5 = auction id search
void clif_parse_Auction_search(int fd, map_session_data* sd){
	char search_text[NAME_LENGTH];
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	short type = RFIFOW(fd,info->pos[0]);
	int price = RFIFOL(fd,info->pos[1]);  // FIXME: bug #5071
	int page = RFIFOW(fd,info->pos[3]);

	if( !battle_config.feature_auction )
		return; 

	clif_parse_Auction_cancelreg(fd, sd);

	safestrncpy(search_text, RFIFOCP(fd,info->pos[2]), sizeof(search_text));
	intif_Auction_requestlist(sd->status.char_id, type, price, search_text, page);
}


/// Requests list of own currently active bids or auctions (CZ_AUCTION_REQ_MY_INFO).
/// 025c <type>.W
/// type:
///     0 = sell (own auctions)
///     1 = buy (own bids)
void clif_parse_Auction_buysell(int fd, map_session_data* sd)
{
	short type = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]) + 6;

	if( !battle_config.feature_auction )
		return;

	clif_parse_Auction_cancelreg(fd, sd);

	intif_Auction_requestlist(sd->status.char_id, type, 0, "", 1);
}


/// CASH/POINT SHOP
///

void clif_cashshop_open( map_session_data* sd, int tab ){
#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined(PACKETVER_ZERO)
	nullpo_retv( sd );

	PACKET_ZC_SE_CASHSHOP_OPEN p = {};

	p.packetType = HEADER_ZC_SE_CASHSHOP_OPEN;
	p.cashPoints = sd->cashPoints;
	p.kafraPoints = sd->kafraPoints;
#if PACKETVER_MAIN_NUM >= 20200129 || PACKETVER_RE_NUM >= 20200205 || PACKETVER_ZERO_NUM >= 20191224
	p.tab = tab;
#endif

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_cashshop_open_request( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined(PACKETVER_ZERO)
	nullpo_retv( sd );

	int tab = 0;

#if PACKETVER >= 20191224
	const PACKET_CZ_SE_CASHSHOP_OPEN2* p = reinterpret_cast<PACKET_CZ_SE_CASHSHOP_OPEN2*>( RFIFOP( fd, 0 ) );

	tab = p->tab;
#endif

	if (map_getmapflag(sd->bl.m, MF_NOCASHSHOP)) {
		clif_displaymessage(fd, msg_txt(sd, 451)); // Cash Shop is disabled on this map.
		return;
	}

	sd->state.cashshop_open = true;
	sd->npc_shopid = -1; // Set npc_shopid when using cash shop from "cash shop" button [Aelys|Susu] bugreport:96

	clif_cashshop_open( sd, tab );
#endif
}

void clif_parse_cashshop_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined(PACKETVER_ZERO)
	sd->state.cashshop_open = false;
	sd->npc_shopid = 0; // Reset npc_shopid when using cash shop from "cash shop" button [Aelys|Susu] bugreport:96
	// No need to do anything here
#endif
}

//0846 <tabid>.W (CZ_REQ_SE_CASH_TAB_CODE))
//08c0 <len>.W <openIdentity>.L <itemcount>.W (ZC_ACK_SE_CASH_ITEM_LIST2)
void clif_parse_CashShopReqTab(int fd, map_session_data *sd) {
#if PACKETVER_MAIN_NUM >= 20100824 || PACKETVER_RE_NUM >= 20100824 || defined(PACKETVER_ZERO)
	const PACKET_CZ_REQ_SE_CASH_TAB_CODE* p_in = reinterpret_cast<PACKET_CZ_REQ_SE_CASH_TAB_CODE*>( RFIFOP( fd, 0 ) );

// TODO: most likely wrong answer packet. Most likely HEADER_ZC_ACK_SE_CASH_ITEM_LIST (0x847) would be correct [Lemongrass]
// [4144] packet exists only in 2011 and was dropped after
#if PACKETVER >= 20110222 && PACKETVER < 20120000
	std::shared_ptr<s_cash_item_tab> tab = cash_shop_db.find( static_cast<e_cash_shop_tab>( p_in->tab ) );

	if( tab == nullptr ){
		return;
	}

	// Skip empty tabs, the client only expects filled ones
	if( tab->items.empty() ){
		return;
	}

#if !(PACKETVER_SUPPORTS_SALES)
	if( tab->tab == CASHSHOP_TAB_SALE ){
		return;
	}
#endif

	PACKET_ZC_ACK_SE_CASH_ITEM_LIST2* p = reinterpret_cast<PACKET_ZC_ACK_SE_CASH_ITEM_LIST2*>( packet_buffer );

	p->packetType = HEADER_ZC_ACK_SE_CASH_ITEM_LIST2;
	p->packetLength = sizeof( *p );
	p->tab = tab->tab;
	p->count = 0;

	for( std::shared_ptr<s_cash_item> item : tab->items ){
		p->items[p->count].itemId = client_nameid( item->nameid );
		p->items[p->count].price = item->price;

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
		p->count++;
	}

	clif_send( p, p->packetLength, &sd->bl, SELF );
#endif
#endif
}

//08ca <len>.W <itemcount> W <tabcode>.W (ZC_ACK_SCHEDULER_CASHITEM)
void clif_cashshop_list( map_session_data& sd ){
	for( const auto& pair : cash_shop_db ){
		std::shared_ptr<s_cash_item_tab> tab = pair.second;

		// Skip empty tabs, the client only expects filled ones
		if( tab->items.empty() ){
			continue;
		}

#if !(PACKETVER_SUPPORTS_SALES)
		if( tab->tab == CASHSHOP_TAB_SALE ){
			continue;
		}
#endif

		PACKET_ZC_ACK_SCHEDULER_CASHITEM* p = reinterpret_cast<PACKET_ZC_ACK_SCHEDULER_CASHITEM*>( packet_buffer );

		p->packetType = HEADER_ZC_ACK_SCHEDULER_CASHITEM;
		p->packetLength = sizeof( *p );
		p->count = 0;
		p->tabNum = tab->tab;

		for( std::shared_ptr<s_cash_item> item : tab->items ){
			p->items[p->count].itemId = client_nameid( item->nameid );
			p->items[p->count].price = item->price;
#ifdef ENABLE_CASHSHOP_PREVIEW_PATCH
			struct item_data* id = itemdb_search( item->nameid );

			if( id == nullptr ){
				p->items[p->count].location = 0;
				p->items[p->count].viewSprite = 0;
			}else{
				p->items[p->count].location = pc_equippoint_sub( &sd, id );
				p->items[p->count].viewSprite = id->look;
			}
#endif
			p->count++;
			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );

			if( ( static_cast<size_t>( p->packetLength ) + sizeof( p->items[0] ) ) >= INT16_MAX ){
				// Send current data
				clif_send( p, p->packetLength, &sd.bl, SELF );

				// Start a new packet
				p->count = 0;
				p->packetLength = sizeof( *p );
			}
		}

		clif_send( p, p->packetLength, &sd.bl, SELF );
	}
}

void clif_parse_cashshop_list_request( int fd, map_session_data* sd ){
	if( !sd->status.cashshop_sent ) {
		clif_cashshop_list( *sd );
#if PACKETVER_SUPPORTS_SALES
		sale_notify_login(sd);
#endif
		sd->status.cashshop_sent = true;
	}
}

/// List of items offered in a cash shop (ZC_PC_CASH_POINT_ITEMLIST).
/// 0287 <packet len>.W <cash point>.L { <sell price>.L <discount price>.L <item type>.B <name id>.W }*
/// 0287 <packet len>.W <cash point>.L <kafra point>.L { <sell price>.L <discount price>.L <item type>.B <name id>.W }* (PACKETVER >= 20070711)
void clif_cashshop_show( map_session_data& sd, npc_data& nd ){
	sd.npc_shopid = nd.bl.id;

	int cost[2] = { 0, 0 };

	npc_shop_currency_type( &sd, &nd, cost, true );

	PACKET_ZC_PC_CASH_POINT_ITEMLIST* p = reinterpret_cast<PACKET_ZC_PC_CASH_POINT_ITEMLIST*>( packet_buffer );

	p->packetType = HEADER_ZC_PC_CASH_POINT_ITEMLIST;
	p->packetLength = sizeof( *p );
	p->cashPoints = cost[0];
#if PACKETVER >= 20070711
	p->kafraPoints = cost[1];
#endif

	for( int i = 0; i < nd.u.shop.count; i++ ) {
		struct item_data* id = itemdb_search( nd.u.shop.shop_item[i].nameid );

		p->items[i].price = nd.u.shop.shop_item[i].value;
		p->items[i].discountPrice = nd.u.shop.shop_item[i].value; // Discount Price
		p->items[i].itemType = itemtype( id->nameid );
		p->items[i].itemId = client_nameid( id->nameid );
#ifdef ENABLE_OLD_CASHSHOP_PREVIEW_PATCH
		p->items[i].location = pc_equippoint_sub( &sd, id );
		p->items[i].viewSprite = id->look;
		memset( p->items[i].unused, 0, sizeof( p->items[i].unused ) );
#endif

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
}

/// Cashshop Buy Ack (ZC_PC_CASH_POINT_UPDATE).
/// 0289 <cash point>.L <error>.W
/// 0289 <cash point>.L <kafra point>.L <error>.W (PACKETVER >= 20070711)
/// error:
///     0 = The deal has successfully completed. (ERROR_TYPE_NONE)
///     1 = The Purchase has failed because the NPC does not exist. (ERROR_TYPE_NPC)
///     2 = The Purchase has failed because the Kafra Shop System is not working correctly. (ERROR_TYPE_SYSTEM)
///     3 = You are over your Weight Limit. (ERROR_TYPE_INVENTORY_WEIGHT)
///     4 = You cannot purchase items while you are in a trade. (ERROR_TYPE_EXCHANGE)
///     5 = The Purchase has failed because the Item Information was incorrect. (ERROR_TYPE_ITEM_ID)
///     6 = You do not have enough Kafra Credit Points. (ERROR_TYPE_MONEY)
///     7 = You can purchase up to 10 items. (ERROR_TYPE_AMOUNT)
///     8 = Some items could not be purchased. (ERROR_TYPE_PURCHASE_FAIL)
void clif_cashshop_ack(map_session_data* sd, int error)
{
	int fd, cost[2] = { 0, 0 };
	struct npc_data *nd;

	nullpo_retv(sd);

	fd = sd->fd;
	nd = map_id2nd(sd->npc_shopid);

	npc_shop_currency_type(sd, nd, cost, false);

	WFIFOHEAD(fd, packet_len(0x289));
	WFIFOW(fd,0) = 0x289;
	WFIFOL(fd,2) = cost[0];
#if PACKETVER < 20070711
	WFIFOW(fd,6) = TOW(error);
#else
	WFIFOL(fd,6) = cost[1];
	WFIFOW(fd,10) = TOW(error);
#endif
	WFIFOSET(fd, packet_len(0x289));
}

void clif_cashshop_result( map_session_data *sd, t_itemid item_id, uint16 result ){
#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined( PACKETVER_ZERO )
	nullpo_retv( sd );

	PACKET_ZC_SE_PC_BUY_CASHITEM_RESULT packet = {};

	packet.packetType = HEADER_ZC_SE_PC_BUY_CASHITEM_RESULT;
	if( item_id != 0 ){
		packet.itemId = client_nameid( item_id );
	}else{
		packet.itemId = 0;
	}
	packet.result = result;
	packet.cashPoints = sd->cashPoints;
	packet.kafraPoints = sd->kafraPoints;

	clif_send( &packet, sizeof( packet ), &sd->bl, SELF );
#endif
}

/// Request to buy item(s) from cash shop.
/// 0288 <name id>.W <amount>.W (CZ_PC_BUY_CASH_POINT_ITEM)
/// 0288 <name id>.W <amount>.W <kafra points>.L (PACKETVER >= 20070711)
/// 0288 <packet len>.W <kafra points>.L <count>.W { <amount>.W <name id>.W }.4B*count (PACKETVER >= 20100803)
void clif_parse_npccashshop_buy( int fd, map_session_data *sd ){
	nullpo_retv( sd );

	if( sd->state.trading || !sd->npc_shopid ){
		clif_cashshop_ack( sd, 1 );
		return;
	}

	const PACKET_CZ_PC_BUY_CASH_POINT_ITEM* p = reinterpret_cast<PACKET_CZ_PC_BUY_CASH_POINT_ITEM*>( RFIFOP( fd, 0 ) );

#if PACKETVER < 20070711
	clif_cashshop_ack( sd, npc_cashshop_buy( sd, p->itemId, p->amount, 0 ) );
#elif PACKETVER < 20101116
	clif_cashshop_ack( sd, npc_cashshop_buy( sd, p->itemId, p->amount, p->kafraPoints ) );
#else
	int s_itl = sizeof( p->items[0] );

	if( p->packetLength < sizeof( *p ) || p->packetLength != sizeof( *p ) + p->count * s_itl ){
		ShowWarning( "Player %u sent incorrect cash shop buy packet (len %u:%" PRIdPTR ")!\n", sd->status.char_id, p->packetLength, sizeof( *p ) + p->count * s_itl );
		return;
	}

	std::vector<s_npc_buy_list> item_list = {};

	item_list.reserve( p->count );

	for( int i = 0; i < p->count; i++ ){
		s_npc_buy_list item = {};

		item.nameid = p->items[i].itemId;
		item.qty = p->items[i].amount;

		item_list.push_back( item );
	}

	clif_cashshop_ack( sd, npc_cashshop_buylist( sd, p->kafraPoints, item_list ) );
#endif
}

/// Request to buy item(s) from cash shop.
/// 0848 <packet len>.W <count>.W <packet len>.W <kafra points>.L <count>.W { <amount>.W <name id>.W <tab>.W }.6B*count (CZ_SE_PC_BUY_CASHITEM_LIST)
void clif_parse_cashshop_buy( int fd, map_session_data *sd ){
	const PACKET_CZ_SE_PC_BUY_CASHITEM_LIST* p = reinterpret_cast<PACKET_CZ_SE_PC_BUY_CASHITEM_LIST*>( RFIFOP( fd, 0 ) );

	int s_itl = sizeof( p->items[0] );

	if( p->packetLength < sizeof( *p ) || p->packetLength != sizeof( *p ) + p->count * s_itl ){
		ShowWarning( "Player %u sent incorrect cash shop buy packet (len %u:%" PRIdPTR ")!\n", sd->status.char_id, p->packetLength, sizeof( *p ) + p->count * s_itl );
		return;
	}

	cashshop_buylist( sd, p->kafraPoints, p->count, p->items );
}

/// Adoption System
///

/// Adoption message (ZC_BABYMSG).
/// 0216 <msg>.L
/// msg:
///     ADOPT_REPLY_MORE_CHILDREN = "You cannot adopt more than 1 child."
///     ADOPT_REPLY_LEVEL_70 = "You must be at least character level 70 in order to adopt someone."
///     ADOPT_REPLY_MARRIED = "You cannot adopt a married person."
void clif_Adopt_reply(map_session_data *sd, int type)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,6);
	WFIFOW(fd,0) = 0x216;
	WFIFOL(fd,2) = type;
	WFIFOSET(fd,6);
}


/// Adoption confirmation (ZC_REQ_BABY).
/// 01f6 <account id>.L <char id>.L <name>.B
void clif_Adopt_request(map_session_data *sd, map_session_data *src, int p_id)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,34);
	WFIFOW(fd,0) = 0x1f6;
	WFIFOL(fd,2) = src->status.account_id;
	WFIFOL(fd,6) = p_id;
	safestrncpy(WFIFOCP(fd,10), src->status.name, NAME_LENGTH);
	WFIFOSET(fd,34);
}


/// Request to adopt a player (CZ_REQ_JOIN_BABY).
/// 01f9 <account id>.L
void clif_parse_Adopt_request(int fd, map_session_data *sd)
{
	TBL_PC *tsd = map_id2sd(RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]));
	TBL_PC *p_sd = map_charid2sd(sd->status.partner_id);

	if( pc_try_adopt(sd, p_sd, tsd) == ADOPT_ALLOWED )
	{
		tsd->adopt_invite = sd->status.account_id;
		clif_Adopt_request(tsd, sd, p_sd->status.account_id);
	}
}


/// Answer to adopt confirmation (CZ_JOIN_BABY).
/// 01f7 <account id>.L <char id>.L <answer>.L
/// answer:
///     0 = rejected
///     1 = accepted
void clif_parse_Adopt_reply(int fd, map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int p1_id = RFIFOL(fd,info->pos[0]);
	int p2_id = RFIFOL(fd,info->pos[1]);
	int result = RFIFOL(fd,info->pos[2]);
	map_session_data* p1_sd = map_id2sd(p1_id);
	map_session_data* p2_sd = map_id2sd(p2_id);

	int pid = sd->adopt_invite;
	sd->adopt_invite = 0;

	if( p1_sd == nullptr || p2_sd == nullptr )
		return; // Both players need to be online

	if( pid != p1_sd->status.account_id )
		return; // Incorrect values

	if( result == 0 )
		return; // Rejected

	pc_adoption(p1_sd, p2_sd, sd);
}


/// Convex Mirror
/// 0293 <infoType>.B <x>.L <y>.L <minHours>.W <minMinutes>.W <maxHours>.W <maxMinutes>.W <monster name>.51B (ZC_BOSS_INFO)
/// infoType:
///     BOSS_INFO_NOT = No boss on this map.
///     BOSS_INFO_ALIVE = Boss is alive (position update).
///     BOSS_INFO_ALIVE_WITHMSG = Boss is alive (initial announce).
///     BOSS_INFO_DEAD = Boss is dead.
void clif_bossmapinfo( map_session_data& sd, mob_data* md, e_bossmap_info flag ){
	PACKET_ZC_BOSS_INFO p = {};

	p.packetType = HEADER_ZC_BOSS_INFO;
	p.type = flag;

	switch (flag) {
		case BOSS_INFO_NOT:
			// No data required
			break;
		case BOSS_INFO_ALIVE_WITHMSG:
			[[fallthrough]];
		case BOSS_INFO_ALIVE:

			p.x = md->bl.x;
			p.y = md->bl.y;
			break;
		case BOSS_INFO_DEAD: {
			const struct TimerData * timer_data = get_timer(md->spawn_timer);
			unsigned int seconds;
			int hours, minutes;

			seconds = (unsigned int)(DIFF_TICK(timer_data->tick, gettick()) / 1000 + 60);
			hours = seconds / (60 * 60);
			seconds = seconds - (60 * 60 * hours);
			minutes = seconds / 60;


			p.minHours = hours;
			p.minMinutes = minutes;
			break;
		}
	}

	if( md != nullptr ){
		safestrncpy( p.name, md->db->jname.c_str(), sizeof( p.name ) );
	}

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
}


/// Requesting equip of a player (CZ_EQUIPWIN_MICROSCOPE).
/// 02d6 <account id>.L
void clif_parse_ViewPlayerEquip(int fd, map_session_data* sd)
{
	int aid = RFIFOL(fd, packet_db[RFIFOW(fd,0)].pos[0]);
	map_session_data* tsd = map_id2sd(aid);

	if (!tsd)
		return;

	if (sd->bl.m != tsd->bl.m)
		return;
	else if( tsd->status.show_equip || pc_has_permission(sd, PC_PERM_VIEW_EQUIPMENT) )
		clif_viewequip_ack( *sd, *tsd );
	else
		clif_msg(sd, MSI_OPEN_EQUIPEDITEM_REFUSED);
}


/// Request to a configuration.
/// 02d8 <type>.L <flag>.L (CZ_CONFIG)
/// type: see enum e_config_type
/// flag:
///     false = disabled
///     true = enabled
void clif_parse_configuration( int fd, map_session_data* sd ){
	int cmd = RFIFOW(fd,0);
	int type = RFIFOL(fd,packet_db[cmd].pos[0]);
	bool flag = RFIFOL(fd,packet_db[cmd].pos[1]) != 0;

	switch( type ){
		case CONFIG_OPEN_EQUIPMENT_WINDOW:
			sd->status.show_equip = flag;
			break;
		case CONFIG_CALL:
			sd->status.disable_call = flag;
			break;
		case CONFIG_PET_AUTOFEED:
			// Player can not click this if he does not have a pet
			if( sd->pd == nullptr || !battle_config.feature_pet_autofeed || !sd->pd->get_pet_db()->allow_autofeed ){
				return;
			}

			sd->pd->pet.autofeed = flag;
			break;
		case CONFIG_HOMUNCULUS_AUTOFEED:
			// Player can not click this if he does not have a homunculus or it is vaporized
			if( sd->hd == nullptr || sd->hd->homunculus.vaporize || !battle_config.feature_homunculus_autofeed ){
				return;
			}

			sd->hd->homunculus.autofeed = flag;
			break;
		default:
			ShowWarning( "clif_parse_configuration: received unknown configuration type '%d'...\n", type );
			return;
	}

	clif_configuration( sd, static_cast<e_config_type>(type), flag );
}

/// Request to change party invitation tick.
/// 02C8 <enabled>.B (CZ_PARTY_CONFIG)
/// value:
///	 0 = disabled (triggered by /accept)
///	 1 = enabled (triggered by /refuse)
void clif_parse_PartyTick( int fd, map_session_data* sd ){
	const PACKET_CZ_PARTY_CONFIG* p = reinterpret_cast<PACKET_CZ_PARTY_CONFIG*>( RFIFOP( fd, 0 ) );

	sd->status.disable_partyinvite = p->refuseInvite;

	clif_partyinvitationstate( *sd );
}

/// Questlog System [Kevin] [Inkfish]
///

/**
 * Safe check to init packet len & quest num for player.
 * @param def_len
 * @param info_len Length of each quest info.
 * @param avail_quests Avail quests that player has.
 * @param limit_out Limit for quest num, to make sure the quest num is still in allowed value.
 * @param len_out Packet length as initial set => def_len + limit_out * info_len.
 **/
static void clif_quest_len(int def_len, int info_len, int avail_quests, int *limit_out, int *len_out) {
	(*limit_out) = (0xFFFF - def_len) / info_len;
	(*limit_out) = (avail_quests > (*limit_out)) ? (*limit_out) : avail_quests;
	(*len_out) = ((*limit_out) * info_len) + def_len;
}

std::string clif_quest_string( std::shared_ptr<s_quest_objective> objective ){
	std::string race_name;

	switch( objective->race ){
		case RC_FORMLESS:	race_name = "Formless"; break;
		case RC_UNDEAD:		race_name = "Undead"; break;
		case RC_BRUTE:		race_name = "Brute"; break;
		case RC_PLANT:		race_name = "Plant"; break;
		case RC_INSECT:		race_name = "Insect"; break;
		case RC_FISH:		race_name = "Fish"; break;
		case RC_DEMON:		race_name = "Demon"; break;
		case RC_DEMIHUMAN:	race_name = "Demihuman"; break;
		case RC_ANGEL:		race_name = "Angel"; break;
		case RC_DRAGON:		race_name = "Dragon"; break;
		default:
			ShowWarning( "clif_quest_string: Unsupported race %d - using empty string...\n", objective->race );
			[[fallthrough]];
		case RC_ALL:		race_name = ""; break;
	}

	std::string size_name;

	switch( objective->size ){
		case SZ_SMALL:	size_name = "Small"; break;
		case SZ_MEDIUM:	size_name = "Medium"; break;
		case SZ_BIG:	size_name = "Large"; break;
		default:
			ShowWarning( "clif_quest_string: Unsupported size %d - using empty string...\n", objective->size );
			[[fallthrough]];
		case SZ_ALL:	size_name = ""; break;
	}

	std::string ele_name;

	switch( objective->element ){
		case ELE_NEUTRAL:	ele_name = "Neutral Element"; break;
		case ELE_WATER:		ele_name = "Water Element"; break;
		case ELE_EARTH:		ele_name = "Earth Element"; break;
		case ELE_FIRE:		ele_name = "Fire Element"; break;
		case ELE_WIND:		ele_name = "Wind Element"; break;
		case ELE_POISON:	ele_name = "Poison Element"; break;
		case ELE_HOLY:		ele_name = "Holy Element"; break;
		case ELE_DARK:		ele_name = "Shadow Element"; break;
		case ELE_GHOST:		ele_name = "Ghost Element"; break;
		case ELE_UNDEAD:	ele_name = "Undead Element"; break;
		default:
			ShowWarning( "clif_quest_string: Unsupported element %d - using empty string...\n", objective->element );
			[[fallthrough]];
		case ELE_ALL:		ele_name = ""; break;
	}

	std::string str;

	if( !objective->map_name.empty() ){
		str += objective->map_name;
	}

	if( !race_name.empty() ){
		if( !str.empty() ){
			str += ", ";
		}

		str += race_name;
	}

	if( !size_name.empty() ){
		if( !str.empty() ){
			str += ", ";
		}

		str += size_name;
	}

	if( !ele_name.empty() ){
		if( !str.empty() ){
			str += ", ";
		}

		str += ele_name;
	}
	
	return str;
}

/// Sends list of all quest states
/// 02b1 <packet len>.W <num>.L { <quest id>.L <active>.B }*num (ZC_ALL_QUEST_LIST)
/// 097a <packet len>.W <num>.L { <quest id>.L <active>.B <remaining time>.L <time>.L <count>.W { <mob_id>.L <killed>.W <total>.W <mob name>.24B }*count }*num (ZC_ALL_QUEST_LIST2)
/// 09f8 <packet len>.W <num>.L { <quest id>.L <active>.B <remaining time>.L <time>.L <count>.W { <hunt identification>.L <mob type>.L <mob_id>.L <min level>.W <max level>.W <killed>.W <total>.W <mob name>.24B }*count }*num  (ZC_ALL_QUEST_LIST3)
void clif_quest_send_list(map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;
	int i;
	int offset = 8;
	int limit = 0;

#if PACKETVER >= 20141022
#if PACKETVER >= 20150513
	int size = 22 + NAME_LENGTH;
#else
	int size = 10 + NAME_LENGTH;
#endif

	clif_quest_len(offset, 15 + ( size * MAX_QUEST_OBJECTIVES), sd->avail_quests, &limit, &i);
	WFIFOHEAD(fd,i);
#if PACKETVER >= 20150513
	WFIFOW(fd, 0) = 0x9f8;
#else
	WFIFOW(fd, 0) = 0x97a;
#endif
	WFIFOL(fd, 4) = limit;

	for (i = 0; i < limit; i++) {
		std::shared_ptr<s_quest_db> qi = quest_search(sd->quest_log[i].quest_id);

		WFIFOL(fd, offset) = sd->quest_log[i].quest_id;
		offset += 4;
		WFIFOB(fd, offset) = sd->quest_log[i].state;
		offset++;
		WFIFOL(fd, offset) = static_cast<uint32>(sd->quest_log[i].time - qi->time);
		offset += 4;
		WFIFOL(fd, offset) = static_cast<uint32>(sd->quest_log[i].time);
		offset += 4;
		WFIFOW(fd, offset) = static_cast<uint16>(qi->objectives.size());
		offset += 2;
		
		if (!qi->objectives.empty()) {
			std::shared_ptr<s_mob_db> mob;

			for (int j = 0; j < qi->objectives.size(); j++) {
				mob = mob_db.find(qi->objectives[j]->mob_id);

				e_race race = qi->objectives[j]->race;
				e_size size = qi->objectives[j]->size;
				e_element element = qi->objectives[j]->element;

#if PACKETVER >= 20150513
				WFIFOL(fd, offset) = sd->quest_log[i].quest_id * 1000 + j;
				offset += 4;
				WFIFOL(fd, offset) = (race ? race : (size ? size : (element ? element : 0)));
				offset += 4;
#endif
				WFIFOL(fd, offset) = ((mob && qi->objectives[j]->mob_id > 0) ? qi->objectives[j]->mob_id : MOBID_PORING);
				offset += 4;
#if PACKETVER >= 20150513
				WFIFOW(fd, offset) = qi->objectives[j]->min_level;
				offset += 2;
				WFIFOW(fd, offset) = qi->objectives[j]->max_level;
				offset += 2;
#endif
				WFIFOW(fd, offset) = sd->quest_log[i].count[j];
				offset += 2;
				WFIFOW(fd, offset) = qi->objectives[j]->count;
				offset += 2;
				if (mob && qi->objectives[j]->mob_id > 0)
					safestrncpy(WFIFOCP(fd,offset), mob->jname.c_str(), NAME_LENGTH);
				else
					safestrncpy(WFIFOCP(fd,offset), clif_quest_string(qi->objectives[j]).c_str(), NAME_LENGTH);
				offset += NAME_LENGTH;
			}
		}
	}

	WFIFOW(fd, 2) = offset;
	WFIFOSET(fd, offset);
#else
	clif_quest_len(offset, 5, sd->avail_quests, &limit, &i);
	WFIFOHEAD(fd,i);
	WFIFOW(fd, 0) = 0x2b1;
	WFIFOL(fd, 4) = limit;

	for (i = 0; i < limit; i++) {
		WFIFOL(fd, offset) = sd->quest_log[i].quest_id;
		offset += 4;
		WFIFOB(fd, offset) = sd->quest_log[i].state;
		offset += 1;
	}
	
	WFIFOW(fd, 2) = offset;
	WFIFOSET(fd, offset);
#endif
}

/// Sends list of all quest missions (ZC_ALL_QUEST_MISSION).
/// 02b2 <packet len>.W <num>.L { <quest id>.L <start time>.L <expire time>.L <mobs>.W { <mob id>.L <mob count>.W <mob name>.24B }*3 }*num
void clif_quest_send_mission(map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;
	int limit = 0;
	int len = sd->avail_quests*104+8;

	clif_quest_len(8, 14 + ((6 + NAME_LENGTH) * MAX_QUEST_OBJECTIVES), sd->avail_quests, &limit, &len);
	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = 0x2b2;
	WFIFOW(fd, 2) = len;
	WFIFOL(fd, 4) = limit;

	for (int i = 0; i < limit; i++) {
		std::shared_ptr<s_quest_db> qi = quest_search(sd->quest_log[i].quest_id);

		WFIFOL(fd, i*104+8) = sd->quest_log[i].quest_id;
		WFIFOL(fd, i*104+12) = static_cast<uint32>(sd->quest_log[i].time - qi->time);
		WFIFOL(fd, i*104+16) = static_cast<uint32>(sd->quest_log[i].time);
		WFIFOW(fd, i*104+20) = static_cast<uint16>(qi->objectives.size());

		for (int j = 0 ; j < qi->objectives.size(); j++) {
			std::shared_ptr<s_mob_db> mob = mob_db.find(qi->objectives[j]->mob_id);

			WFIFOL(fd, i*104+22+j*30) = (mob ? qi->objectives[j]->mob_id : MOBID_PORING);
			WFIFOW(fd, i*104+26+j*30) = sd->quest_log[i].count[j];
			if (mob && qi->objectives[j]->mob_id > 0)
				safestrncpy(WFIFOCP(fd, i*104+28+j*30), mob->jname.c_str(), NAME_LENGTH);
			else
				safestrncpy(WFIFOCP(fd, i*104+28+j*30), clif_quest_string(qi->objectives[j]).c_str(), NAME_LENGTH);
		}
	}

	WFIFOSET(fd, len);
}


/// Notification about a new quest
/// 02b3 <quest id>.L <active>.B <start time>.L <expire time>.L <mobs>.W { <mob id>.L <mob count>.W <mob name>.24B }*3 (ZC_ADD_QUEST)
/// 08fe <packet len>.W  { <quest id>.L <mob id>.L <total count>.W <current count>.W }*3 (ZC_HUNTING_QUEST_INFO)
/// 09f9 <quest id>.L <active>.B <start time>.L <expire time>.L <mobs>.W { <hunt identification>.L <mob type>.L <mob id>.L <min level>.W <max level>.W <mob count>.W <mob name>.24B }*3 (ZC_ADD_QUEST_EX)
void clif_quest_add(map_session_data *sd, struct quest *qd)
{
	nullpo_retv(sd);
	nullpo_retv(qd);

	int fd = sd->fd;
	std::shared_ptr<s_quest_db> qi = quest_search(qd->quest_id);
	if (!qi)
		return;
#if PACKETVER >= 20150513
	int cmd = 0x9f9;
#else
	int cmd = 0x2b3;
#endif

	WFIFOHEAD(fd, packet_len(cmd));
	WFIFOW(fd, 0) = cmd;
	WFIFOL(fd, 2) = qd->quest_id;
	WFIFOB(fd, 6) = qd->state;
	WFIFOB(fd, 7) = static_cast<uint8>(qd->time - qi->time);
	WFIFOL(fd, 11) = static_cast<uint32>(qd->time);
	WFIFOW(fd, 15) = static_cast<uint16>(qi->objectives.size());

	for (int i = 0, offset = 17; i < qi->objectives.size(); i++) {
		std::shared_ptr<s_mob_db> mob = mob_db.find(qi->objectives[i]->mob_id);
		e_race race = qi->objectives[i]->race;
		e_size size = qi->objectives[i]->size;
		e_element element = qi->objectives[i]->element;

#if PACKETVER >= 20150513
		WFIFOL(fd, offset) = qd->quest_id * 1000 + i;
		offset += 4;
		WFIFOL(fd, offset) = (race ? race : (size ? size : (element ? element : 0)));	// effect ?
		offset += 4;
#endif
		WFIFOL(fd, offset) = ((mob && qi->objectives[i]->mob_id > 0) ? qi->objectives[i]->mob_id : MOBID_PORING);	// 0 can't be used as it displays "Novice" job regardless of the clif_mobtype_name
		offset += 4;
#if PACKETVER >= 20150513
		WFIFOW(fd, offset) = qi->objectives[i]->min_level;
		offset += 2;
		WFIFOW(fd, offset) = qi->objectives[i]->max_level;
		offset += 2;
#endif
		WFIFOW(fd, offset) = qd->count[i];
		offset += 2;
		if (mob && qi->objectives[i]->mob_id > 0)
			safestrncpy(WFIFOCP(fd,offset), mob->jname.c_str(), NAME_LENGTH);
		else
			safestrncpy(WFIFOCP(fd,offset), clif_quest_string(qi->objectives[i]).c_str(), NAME_LENGTH);
		offset += NAME_LENGTH;
	}

	WFIFOSET(fd, packet_len(cmd));

#if PACKETVER >= 20150513
	int16 len;

	len = static_cast<decltype(len)>( 4 + qi->objectives.size() * 12 );

	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = 0x8fe;
	WFIFOW(fd, 2) = len;

	for (int i = 0, offset = 4; i < qi->objectives.size(); i++, offset += 12) {
		WFIFOL(fd, offset) = qd->quest_id * 1000 + i;
		WFIFOL(fd, offset + 4) = qi->objectives[i]->mob_id;
		WFIFOW(fd, offset + 8) = qi->objectives[i]->count;
		WFIFOW(fd, offset + 10) = qd->count[i];
	}

	WFIFOSET(fd, len);

#endif
}


/// Notification about a quest being removed (ZC_DEL_QUEST).
/// 02b4 <quest id>.L
void clif_quest_delete(map_session_data *sd, int quest_id)
{
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x2b4));
	WFIFOW(fd, 0) = 0x2b4;
	WFIFOL(fd, 2) = quest_id;
	WFIFOSET(fd, packet_len(0x2b4));
}


/// Notification of an update to the hunting mission counter
/// 02b5 <packet len>.W <mobs>.W { <quest id>.L <mob id>.L <total count>.W <current count>.W }*3 (ZC_UPDATE_MISSION_HUNT)
/// 09fa <packet len>.W <mobs>.W { <quest id>.L <hunt identification>.L <total count>.W <current count>.W }*3 (ZC_UPDATE_MISSION_HUNT_EX)
void clif_quest_update_objective(map_session_data *sd, struct quest *qd)
{
	int fd = sd->fd;
	int offset = 6;
	std::shared_ptr<s_quest_db> qi = quest_search(qd->quest_id);
	int16 len;
	len = static_cast<decltype(len)>( qi->objectives.size() * 12 + 6 );
#if PACKETVER >= 20150513
	int cmd = 0x9fa;
#else
	int cmd = 0x2b5;
#endif

	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = cmd;
	WFIFOW(fd, 4) = static_cast<uint16>(qi->objectives.size());

	for (int i = 0; i < qi->objectives.size(); i++) {
		WFIFOL(fd, offset) = qd->quest_id;
		offset += 4;
#if PACKETVER >= 20150513
		WFIFOL(fd, offset) = qd->quest_id * 1000 + i;
		offset += 4;
#else
		WFIFOL(fd, offset) = qi->objectives[i]->mob_id;
		offset += 4;
#endif
		WFIFOW(fd, offset) = qi->objectives[i]->count;
		offset += 2;
		WFIFOW(fd, offset) = qd->count[i];
		offset += 2;
	}

	WFIFOW(fd, 2) = offset;
	WFIFOSET(fd, offset);
}


/// Request to change the state of a quest (CZ_ACTIVE_QUEST).
/// 02b6 <quest id>.L <active>.B
void clif_parse_questStateAck(int fd, map_session_data *sd)
{
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	quest_update_status(sd, RFIFOL(fd,info->pos[0]),
	    RFIFOB(fd,info->pos[1])?Q_ACTIVE:Q_INACTIVE);
}


/// Notification about the change of a quest state (ZC_ACTIVE_QUEST).
/// 02b7 <quest id>.L <active>.B
void clif_quest_update_status(map_session_data *sd, int quest_id, bool active)
{
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x2b7));
	WFIFOW(fd, 0) = 0x2b7;
	WFIFOL(fd, 2) = quest_id;
	WFIFOB(fd, 6) = active;
	WFIFOSET(fd, packet_len(0x2b7));
}


/// Notification about an NPC's quest state (ZC_QUEST_NOTIFY_EFFECT).
/// 0446 <npc id>.L <x>.W <y>.W <effect>.W <color>.W
/// effect:
///     0 = none
///     1 = exclamation mark icon
///     2 = question mark icon
/// color:
///     0 = yellow
///     1 = orange
///     2 = green
///     3 = purple
void clif_quest_show_event(map_session_data *sd, struct block_list *bl, e_questinfo_types effect, e_questinfo_markcolor color)
{
#if PACKETVER >= 20090218
	nullpo_retv(sd);
	nullpo_retv(bl);

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x446));
	WFIFOW(fd, 0) = 0x446;
	WFIFOL(fd, 2) = bl->id;
	WFIFOW(fd, 6) = bl->x;
	WFIFOW(fd, 8) = bl->y;
	WFIFOW(fd, 10) = effect;
	WFIFOW(fd, 12) = color;
	WFIFOSET(fd, packet_len(0x446));
#endif
}


/// Mercenary System
///

/// Notification about a mercenary status parameter change (ZC_MER_PAR_CHANGE).
/// 02a2 <var id>.W <value>.L
void clif_mercenary_updatestatus(map_session_data *sd, int type)
{
	s_mercenary_data *md;
	struct status_data *status;
	int fd;
	if( !clif_session_isValid(sd) || (md = sd->md) == nullptr )
		return;

	fd = sd->fd;
	status = &md->battle_status;
	WFIFOHEAD(fd,packet_len(0x2a2));
	WFIFOW(fd,0) = 0x2a2;
	WFIFOW(fd,2) = type;
	switch( type ) {
		case SP_ATK1:
			{
				int atk = rnd()%(status->rhw.atk2 - status->rhw.atk + 1) + status->batk + status->rhw.atk;
				WFIFOL(fd,4) = cap_value(atk, 0, INT16_MAX);
			}
			break;
		case SP_MATK1:
			WFIFOL(fd,4) = min(status->matk_max, UINT16_MAX);
			break;
		case SP_HIT:
			WFIFOL(fd,4) = status->hit;
			break;
		case SP_CRITICAL:
			WFIFOL(fd,4) = status->cri/10;
			break;
		case SP_DEF1:
			WFIFOL(fd,4) = status->def;
			break;
		case SP_MDEF1:
			WFIFOL(fd,4) = status->mdef;
			break;
		case SP_MERCFLEE:
			WFIFOL(fd,4) = status->flee;
			break;
		case SP_ASPD:
			WFIFOL(fd,4) = status->amotion;
			break;
		case SP_HP:
			WFIFOL(fd,4) = status->hp;
			break;
		case SP_MAXHP:
			WFIFOL(fd,4) = status->max_hp;
			break;
		case SP_SP:
			WFIFOL(fd,4) = status->sp;
			break;
		case SP_MAXSP:
			WFIFOL(fd,4) = status->max_sp;
			break;
		case SP_MERCKILLS:
			WFIFOL(fd,4) = md->mercenary.kill_count;
			break;
		case SP_MERCFAITH:
			WFIFOL(fd,4) = mercenary_get_faith(md);
			break;
	}
	WFIFOSET(fd,packet_len(0x2a2));
}


/// Mercenary base status data (ZC_MER_INIT).
/// 029b <id>.L <atk>.W <matk>.W <hit>.W <crit>.W <def>.W <mdef>.W <flee>.W <aspd>.W
///     <name>.24B <level>.W <hp>.L <maxhp>.L <sp>.L <maxsp>.L <expire time>.L <faith>.W
///     <calls>.L <kills>.L <atk range>.W
void clif_mercenary_info(map_session_data *sd)
{
	int fd;
	s_mercenary_data *md;
	struct status_data *status;
	int atk;

	if( !clif_session_isValid(sd) || (md = sd->md) == nullptr )
		return;

	fd = sd->fd;
	status = &md->battle_status;

	WFIFOHEAD(fd,packet_len(0x29b));
	WFIFOW(fd,0) = 0x29b;
	WFIFOL(fd,2) = md->bl.id;

	// Mercenary shows ATK as a random value between ATK ~ ATK2
	atk = rnd()%(status->rhw.atk2 - status->rhw.atk + 1) + status->batk + status->rhw.atk;
	WFIFOW(fd,6) = cap_value(atk, 0, INT16_MAX);
	WFIFOW(fd,8) = min(status->matk_max, UINT16_MAX);
	WFIFOW(fd,10) = status->hit;
	WFIFOW(fd,12) = status->cri/10;
	WFIFOW(fd,14) = status->def;
	WFIFOW(fd,16) = status->mdef;
	WFIFOW(fd,18) = status->flee;
	WFIFOW(fd,20) = status->amotion;
	safestrncpy(WFIFOCP(fd,22), md->db->name.c_str(), NAME_LENGTH);
	WFIFOW(fd,46) = md->db->lv;
	WFIFOL(fd,48) = status->hp;
	WFIFOL(fd,52) = status->max_hp;
	WFIFOL(fd,56) = status->sp;
	WFIFOL(fd,60) = status->max_sp;
	WFIFOL(fd,64) = client_tick(time(nullptr) + (mercenary_get_lifetime(md) / 1000));
	WFIFOW(fd,68) = mercenary_get_faith(md);
	WFIFOL(fd,70) = mercenary_get_calls(md);
	WFIFOL(fd,74) = md->mercenary.kill_count;
	WFIFOW(fd,78) = md->battle_status.rhw.range;
	WFIFOSET(fd,packet_len(0x29b));
}


/// Mercenary skill tree (ZC_MER_SKILLINFO_LIST).
/// 029d <packet len>.W { <skill id>.W <type>.L <level>.W <sp cost>.W <attack range>.W <skill name>.24B <upgradable>.B }*
void clif_mercenary_skillblock(map_session_data *sd)
{
	s_mercenary_data *md;
	int fd, len = 4;

	if( sd == nullptr || (md = sd->md) == nullptr )
		return;

	fd = sd->fd;
	WFIFOHEAD(fd,4+37*MAX_MERCSKILL);
	WFIFOW(fd,0) = 0x29d;
	for (const auto &it : md->db->skill) {
		uint16 id = it.first;

		if (!SKILL_CHK_MERC(id))
			continue;

		uint16 lv = it.second;

		WFIFOW(fd,len) = id;
		WFIFOL(fd,len+2) = skill_get_inf(id);
		WFIFOW(fd,len+6) = lv;
		WFIFOW(fd,len+8) = skill_get_sp(id, lv);
		WFIFOW(fd,len+10) = skill_get_range2(&md->bl, id, lv, false);
		safestrncpy(WFIFOCP(fd,len+12), skill_get_name(id), NAME_LENGTH);
		WFIFOB(fd,len+36) = 0; // Skillable for Mercenary?
		len += 37;
	}

	WFIFOW(fd,2) = len;
	WFIFOSET(fd,len);
}


/// Request to invoke a mercenary menu action (CZ_MER_COMMAND).
/// 029f <command>.B
///     1 = mercenary information
///     2 = delete
void clif_parse_mercenary_action(int fd, map_session_data* sd)
{
	int option = RFIFOB(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	if( sd->md == nullptr )
		return;

	if( option == 2 ) mercenary_delete(sd->md, 2);
}


/// Notification about the remaining time of a rental item.
/// 0298 <name id>.W <seconds>.L (ZC_CASH_TIME_COUNTER)
void clif_rental_time( map_session_data* sd, t_itemid nameid, int seconds ){
	// '<ItemName>' item will disappear in <seconds/60> minutes.
	PACKET_ZC_CASH_TIME_COUNTER p = {};

	p.packetType = HEADER_ZC_CASH_TIME_COUNTER;
	p.itemId = client_nameid( nameid );
	p.seconds = seconds;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Deletes a rental item from client's inventory.
/// 0299 <index>.W <name id>.W (ZC_CASH_ITEM_DELETE)
void clif_rental_expired( map_session_data* sd, int index, t_itemid nameid ){
	// '<ItemName>' item has been deleted from the Inventory
	PACKET_ZC_CASH_ITEM_DELETE p = {};

	p.packetType = HEADER_ZC_CASH_ITEM_DELETE;
	p.index = client_index( index );
	p.itemId = client_nameid( nameid );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Book Reading (ZC_READ_BOOK).
/// 0294 <book id>.L <page>.L
void clif_readbook(int fd, int book_id, int page)
{
	WFIFOHEAD(fd,packet_len(0x294));
	WFIFOW(fd,0) = 0x294;
	WFIFOL(fd,2) = book_id;
	WFIFOL(fd,6) = page;
	WFIFOSET(fd,packet_len(0x294));
}


/// Battlegrounds
///

/// Updates HP bar of a camp member.
/// 02e0 <account id>.L <name>.24B <hp>.W <max hp>.W (ZC_BATTLEFIELD_NOTIFY_HP)
/// 0a0e <account id>.L <hp>.L <max hp>.L (ZC_BATTLEFIELD_NOTIFY_HP2)
void clif_bg_hp(map_session_data *sd) {
#if PACKETVER < 20140613
	unsigned char buf[34];
	const int cmd = 0x2e0;
#else
	unsigned char buf[14];
	const int cmd = 0xa0e;
#endif
	nullpo_retv(sd);

	WBUFW(buf,0) = cmd;
	WBUFL(buf,2) = sd->status.account_id;
#if PACKETVER < 20140613
    safestrncpy(WBUFCP(buf,6), sd->status.name, NAME_LENGTH);
    if( sd->battle_status.max_hp > INT16_MAX ) { // To correctly display the %hp bar. [Skotlex]
		WBUFW(buf,30) = sd->battle_status.hp/(sd->battle_status.max_hp/100);
        WBUFW(buf,32) = 100;
    } else {
		WBUFW(buf,30) = sd->battle_status.hp;
		WBUFW(buf,32) = sd->battle_status.max_hp;
    }
#else
    if( sd->battle_status.max_hp > INT32_MAX ) {
		WBUFL(buf,6) = sd->battle_status.hp/(sd->battle_status.max_hp/100);
		WBUFL(buf,10) = 100;
    } else {
		WBUFL(buf,6) = sd->battle_status.hp;
		WBUFL(buf,10) = sd->battle_status.max_hp;
    }
#endif
	clif_send(buf, packet_len(cmd), &sd->bl, BG_AREA_WOS);
}


/// Updates the position of a camp member on the minimap (ZC_BATTLEFIELD_NOTIFY_POSITION).
/// 02df <account id>.L <name>.24B <class>.W <x>.W <y>.W
void clif_bg_xy(map_session_data *sd)
{
	unsigned char buf[36];
	nullpo_retv(sd);

	WBUFW(buf,0)=0x2df;
	WBUFL(buf,2)=sd->status.account_id;
	safestrncpy(WBUFCP(buf,6), sd->status.name, NAME_LENGTH);
	WBUFW(buf,30)=sd->status.class_;
	WBUFW(buf,32)=sd->bl.x;
	WBUFW(buf,34)=sd->bl.y;

	clif_send(buf, packet_len(0x2df), &sd->bl, BG_SAMEMAP_WOS);
}

void clif_bg_xy_remove(map_session_data *sd)
{
	unsigned char buf[36];
	nullpo_retv(sd);

	WBUFW(buf,0)=0x2df;
	WBUFL(buf,2)=sd->status.account_id;
	memset(WBUFP(buf,6), 0, NAME_LENGTH);
	WBUFW(buf,30)=0;
	WBUFW(buf,32)=-1;
	WBUFW(buf,34)=-1;

	clif_send(buf, packet_len(0x2df), &sd->bl, BG_SAMEMAP_WOS);
}

/// Notifies clients of a battleground message.
/// 02DC <packet len>.W <account id>.L <name>.24B <message>.?B (ZC_BATTLEFIELD_CHAT)
void clif_bg_message( struct s_battleground_data *bg, int src_id, const char *name, const char *mes, size_t len ){
	map_session_data *sd = bg_getavailablesd( bg );

	if( sd == nullptr ){
		return;
	}

	// limit length
	len = zmin( len + 1, CHAT_SIZE_MAX );

	unsigned char buf[8 + NAME_LENGTH + CHAT_SIZE_MAX];

	WBUFW(buf,0) = 0x2dc;
	WBUFW(buf,2) = static_cast<int16>( len + NAME_LENGTH + 8 );
	WBUFL(buf,4) = src_id;
	safestrncpy(WBUFCP(buf,8), name, NAME_LENGTH);
	safestrncpy(WBUFCP(buf,8+NAME_LENGTH), mes, len );

	clif_send(buf,WBUFW(buf,2), &sd->bl, BG);
}

/// Validates and processes battlechat messages.
/// All messages that are sent after enabling battleground chat with /battlechat.
/// 02DB <packet len>.W <text>.?B (CZ_BATTLEFIELD_CHAT)
void clif_parse_BattleChat(int fd, map_session_data* sd){
	char name[NAME_LENGTH], message[CHAT_SIZE_MAX], output[CHAT_SIZE_MAX+NAME_LENGTH*2];

	if( !clif_process_message( sd, false, name, message, output ) )
		return;

	bg_send_message(sd, output, strlen(output) );
}


/// Notifies client of a battleground score change (ZC_BATTLEFIELD_NOTIFY_POINT).
/// 02de <camp A points>.W <camp B points>.W
void clif_bg_updatescore(int16 m)
{
	struct block_list bl;
	unsigned char buf[6];
	struct map_data *mapdata = map_getmapdata(m);

	bl.id = 0;
	bl.type = BL_NUL;
	bl.m = m;

	WBUFW(buf,0) = 0x2de;
	WBUFW(buf,2) = mapdata->bgscore_lion;
	WBUFW(buf,4) = mapdata->bgscore_eagle;
	clif_send(buf,packet_len(0x2de),&bl,ALL_SAMEMAP);
}

void clif_bg_updatescore_single(map_session_data *sd)
{
	int fd;
	nullpo_retv(sd);
	fd = sd->fd;

	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	WFIFOHEAD(fd,packet_len(0x2de));
	WFIFOW(fd,0) = 0x2de;
	WFIFOW(fd,2) = mapdata->bgscore_lion;
	WFIFOW(fd,4) = mapdata->bgscore_eagle;
	WFIFOSET(fd,packet_len(0x2de));
}


/// Battleground camp belong-information (ZC_BATTLEFIELD_NOTIFY_CAMPINFO).
/// 02dd <account id>.L <name>.24B <camp>.W
void clif_sendbgemblem_area(map_session_data *sd)
{
	unsigned char buf[33];
	nullpo_retv(sd);

	WBUFW(buf, 0) = 0x2dd;
	WBUFL(buf,2) = sd->bl.id;
	safestrncpy(WBUFCP(buf,6), sd->status.name, NAME_LENGTH); // name don't show in screen.
	WBUFW(buf,30) = sd->bg_id;
	clif_send(buf,packet_len(0x2dd), &sd->bl, AREA);
}

void clif_sendbgemblem_single(int fd, map_session_data *sd)
{
	nullpo_retv(sd);
	WFIFOHEAD(fd,32);
	WFIFOW(fd,0) = 0x2dd;
	WFIFOL(fd,2) = sd->bl.id;
	safestrncpy(WFIFOCP(fd,6), sd->status.name, NAME_LENGTH);
	WFIFOW(fd,30) = sd->bg_id;
	WFIFOSET(fd,packet_len(0x2dd));
}

/// Battlegrounds queue incoming apply request from client.
/// Queue types: 1 solo queue, 2 party queue, 4 guild queue.
/// 0x8d7 <queue type>.W <battleground name>.24B (CZ_REQ_ENTRY_QUEUE_APPLY)
void clif_parse_bg_queue_apply_request(int fd, map_session_data *sd)
{
	if (!battle_config.feature_bgqueue)
		return;

	nullpo_retv(sd);

	short type = RFIFOW(fd,2);
	char name[NAME_LENGTH];

	safestrncpy(name, RFIFOCP(fd, 4), NAME_LENGTH);

	if (sd->bg_queue_id > 0) {
		//ShowWarning("clif_parse_bg_queue_apply_request: Received duplicate queue application: %d from player %s (AID:%d CID:%d).\n", type, sd->status.name, sd->status.account_id, sd->status.char_id);
		clif_bg_queue_apply_result(BG_APPLY_DUPLICATE, name, sd); // Duplicate application warning
		return;
	} else if (type == 1) // Solo
		bg_queue_join_solo(name, sd);
	else if (type == 2) // Party
		bg_queue_join_party(name, sd);
	else if (type == 4) // Guild
		bg_queue_join_guild(name, sd);
	else {
		ShowWarning("clif_parse_bg_queue_apply_request: Received invalid queue type: %d from player %s (AID:%d CID:%d).\n", type, sd->status.name, sd->status.account_id, sd->status.char_id);
		clif_bg_queue_apply_result(BG_APPLY_INVALID_APP, name, sd); // Someone sent an invalid queue type packet
		return;
	}
}

/// Outgoing battlegrounds queue apply result.
/// Result types: @see e_bg_queue_apply_ack
/// 0x8d8 <result>.B <battleground name>.24B (ZC_ACK_ENTRY_QUEUE_APPLY)
void clif_bg_queue_apply_result(e_bg_queue_apply_ack result, const char *name, map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x8d8));
	WFIFOW(fd,0) = 0x8d8;
	WFIFOB(fd,2) = result;
	safestrncpy(WFIFOCP(fd,3), name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x8d8));
}

/// Outgoing battlegrounds queue apply notification.
/// Sends a notification at the start of the battlegrounds queue and is also used to update the queue number.
/// 0x8d9 <battleground name>.24B <queue number>.L (ZC_NOTIFY_ENTRY_QUEUE_APPLY)
void clif_bg_queue_apply_notify(const char *name, map_session_data *sd)
{
	nullpo_retv(sd);

	std::shared_ptr<s_battleground_queue> queue = bg_search_queue(sd->bg_queue_id);

	if (queue == nullptr) {
		ShowError("clif_bg_queue_apply_notify: Player is not in a battleground queue.\n");
		return;
	}

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x8d9));
	WFIFOW(fd,0) = 0x8d9;
	safestrncpy(WFIFOCP(fd,2), name, NAME_LENGTH);
	WFIFOL(fd,2+NAME_LENGTH) = static_cast<int32>( queue->teama_members.size() + queue->teamb_members.size() );
	WFIFOSET(fd, packet_len(0x8d9));
}

/// Battlegrounds queue outgoing cancel result.
/// 0x8db <result>.B <battleground name>.24B (ZC_ACK_ENTRY_QUEUE_CANCEL)
void clif_bg_queue_cancel_result(bool success, const char *name, map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x8d8));
	WFIFOW(fd,0) = 0x8db;
	WFIFOB(fd,2) = success;
	safestrncpy(WFIFOCP(fd,3), name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x8d8));
}

/// Battlegrounds queue incoming cancel request from client.
/// 0x8da <battleground name>.24B (CZ_REQ_ENTRY_QUEUE_CANCEL)
void clif_parse_bg_queue_cancel_request(int fd, map_session_data *sd)
{
	if (!battle_config.feature_bgqueue)
		return;

	nullpo_retv(sd);

	bool success;

	if (sd->bg_queue_id > 0) {
		std::shared_ptr<s_battleground_queue> queue = bg_search_queue(sd->bg_queue_id);

		if (queue && queue->state == QUEUE_STATE_SETUP_DELAY)
			return; // Make the cancel button do nothing if the entry window is open. Otherwise it'll crash the game when you click on both the queue status and entry status window.
		else
			success = bg_queue_leave(sd);
	} else {
		ShowWarning("clif_parse_bg_queue_cancel_request: Player trying to request leaving non-existent queue with name: %s (AID:%d CID:%d).\n", sd->status.name, sd->status.account_id, sd->status.char_id);
		success = false;
	}

	char name[NAME_LENGTH];

	safestrncpy( name, RFIFOCP( fd, 2 ), NAME_LENGTH );

	clif_bg_queue_cancel_result(success, name, sd);
}

/// Battleground is ready to be joined, send a window asking for players to accept or decline.
/// 0x8df <battleground name>.24B <lobby name>.24B (ZC_NOTIFY_LOBBY_ADMISSION)
void clif_bg_queue_lobby_notify(const char *name, map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x8df));
	WFIFOW(fd,0) = 0x8df;
	safestrncpy(WFIFOCP(fd,2), name, NAME_LENGTH);
	safestrncpy(WFIFOCP(fd,2+NAME_LENGTH), name, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x8df));
}

/// Incoming packet from client telling server whether player wants to enter battleground or cancel.
/// Result types: 1(Accept), 2(Decline).
/// 0x8e0 <result>.B <battleground name>.24B <lobby name>.24B (CZ_REPLY_LOBBY_ADMISSION)
void clif_parse_bg_queue_lobby_reply(int fd, map_session_data *sd)
{
	nullpo_retv(sd);

	if(sd->bg_queue_id > 0) {
		uint8 result = RFIFOB(fd, 2);

		if(result == 1) { // Accept
			bg_queue_on_accept_invite(sd);
		} else if(result == 2) { // Decline
			bg_queue_leave(sd);
			clif_bg_queue_entry_init(sd);
		}
	}
}

/// Plays a gong sound, signaling that someone has accepted the invite to enter a battleground.
/// 0x8e1 <result>.B <battleground name>.24B <lobby name>.24B (ZC_REPLY_ACK_LOBBY_ADMISSION)
void clif_bg_queue_ack_lobby(bool result, const char *name, const char *lobbyname, map_session_data *sd)
{
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x8e1));
	WFIFOW(fd,0) = 0x8e1;
	WFIFOB(fd,2) = result;
	safestrncpy(WFIFOCP(fd,3), name, NAME_LENGTH);
	safestrncpy(WFIFOCP(fd,3+NAME_LENGTH), lobbyname, NAME_LENGTH);
	WFIFOSET(fd, packet_len(0x8e1));
}

/// Battlegrounds queue incoming queue number request from client.
/// 0x90a <battleground name>.24B (CZ_REQ_ENTRY_QUEUE_RANKING)
void clif_parse_bg_queue_request_queue_number(int fd, map_session_data *sd)
{
	nullpo_retv(sd);

	char name[NAME_LENGTH];

	safestrncpy( name, RFIFOCP(fd, 2), NAME_LENGTH );

	clif_bg_queue_apply_notify(name, sd);
}

/// Silently removes all the battlegrounds stuff client side so that you will open the first BG window when you press battle on the interface.
/// Send this when a player joins a battleground so that it will remove all the queue stuff upon warping in.
/// 0x90e (ZC_ENTRY_QUEUE_INIT)
void clif_bg_queue_entry_init(map_session_data *sd)
{
	nullpo_retv(sd);

	PACKET_ZC_ENTRY_QUEUE_INIT p = {};

	p.packetType = HEADER_ZC_ENTRY_QUEUE_INIT;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}

/// Custom Fonts (ZC_NOTIFY_FONT).
/// 02ef <account_id>.L <font id>.W
void clif_font(map_session_data *sd)
{
#if PACKETVER >= 20080102
	unsigned char buf[8];
	nullpo_retv(sd);
	WBUFW(buf,0) = 0x2ef;
	WBUFL(buf,2) = sd->bl.id;
	WBUFW(buf,6) = sd->status.font;
	clif_send(buf, packet_len(0x2ef), &sd->bl, AREA);
#endif
}


/// Required to start the instancing information window on Client
/// This window re-appears each "refresh" of client automatically until the keep_limit reaches 0.
/// S 0x2cb <Instance name>.61B <Standby Position>.W
void clif_instance_create( int instance_id, size_t num ){
#if PACKETVER >= 20071128
	map_session_data *sd = nullptr;
	enum send_target target = PARTY;
	unsigned char buf[65];

	instance_getsd(instance_id, sd, &target);

	if (!sd)
		return;

	std::shared_ptr<s_instance_db> db = instance_db.find(util::umap_find(instances, instance_id)->id);

	if (!db)
		return;

	WBUFW(buf,0) = 0x2cb;
	safestrncpy(WBUFCP(buf,2), db->name.c_str(), INSTANCE_NAME_LENGTH);
	WBUFW( buf, 63 ) = static_cast<int16>( num );

	clif_send(buf,packet_len(0x2cb),&sd->bl,target);
#endif
}

/// To announce Instancing queue creation if no maps available
/// S 0x2cc <Standby Position>.W
void clif_instance_changewait(int instance_id, int num)
{
#if PACKETVER >= 20071128
	map_session_data *sd = nullptr;
	enum send_target target = PARTY;
	unsigned char buf[4];

	instance_getsd(instance_id, sd, &target);

	if (!sd)
		return;

	WBUFW(buf,0) = 0x2cc;
	WBUFW(buf,2) = num;
	clif_send(buf,packet_len(0x2cc),&sd->bl,target);
#endif

	return;
}

/// Notify the current status to members
/// S 0x2cd <Instance Name>.61B <Instance Remaining Time>.L <Instance Noplayers close time>.L
void clif_instance_status(int instance_id, unsigned int limit1, unsigned int limit2)
{
#if PACKETVER >= 20071128
	map_session_data *sd = nullptr;
	enum send_target target = PARTY;
	unsigned char buf[71];

	instance_getsd(instance_id, sd, &target);

	if (!sd)
		return;

	std::shared_ptr<s_instance_db> db = instance_db.find(util::umap_find(instances, instance_id)->id);

	if (!db)
		return;

	WBUFW(buf,0) = 0x2cd;
	safestrncpy(WBUFCP(buf,2), db->name.c_str(), INSTANCE_NAME_LENGTH);
	WBUFL(buf,63) = limit1;
	WBUFL(buf,67) = limit2;
	clif_send(buf,packet_len(0x2cd),&sd->bl,target);
#endif

	return;
}

/// Notify a status change to members
/// S 0x2ce <Message ID>.L
/// 0 = Notification (EnterLimitDate update?)
/// 1 = The Memorial Dungeon expired; it has been destroyed
/// 2 = The Memorial Dungeon's entry time limit expired; it has been destroyed
/// 3 = The Memorial Dungeon has been removed.
/// 4 = Create failure (removes the instance window)
void clif_instance_changestatus(int instance_id, e_instance_notify type, unsigned int limit)
{
#if PACKETVER >= 20071128
	map_session_data *sd = nullptr;
	enum send_target target = PARTY;
	unsigned char buf[10];

	instance_getsd(instance_id, sd, &target);

	if (!sd)
		return;

	WBUFW(buf,0) = 0x2ce;
	WBUFL(buf,2) = type;
	WBUFL(buf,6) = limit;
	clif_send(buf,packet_len(0x2ce),&sd->bl,target);
#endif

	return;
}

/// Destroy an instance from the status window
/// 02cf <command>.L (CZ_MEMORIALDUNGEON_COMMAND)
void clif_parse_MemorialDungeonCommand(int fd, map_session_data *sd)
{
	if (pc_istrading(sd) || pc_isdead(sd) || map_getmapdata(sd->bl.m)->instance_id > 0)
		return;

	const PACKET_CZ_MEMORIALDUNGEON_COMMAND* p = reinterpret_cast<PACKET_CZ_MEMORIALDUNGEON_COMMAND*>( RFIFOP( fd, 0 ) );

	switch (p->command) {
		case COMMAND_MEMORIALDUNGEON_DESTROY_FORCE:
			instance_destroy_command(sd);
			break;
	}
}

void clif_instance_info( map_session_data& sd ){
	if( sd.instance_id > 0 ){
		instance_reqinfo( &sd, sd.instance_id );
	}

	if( sd.status.party_id > 0 ){
		struct party_data* p = party_search( sd.status.party_id );

		if( p != nullptr && p->instance_id > 0 ){
			instance_reqinfo( &sd, p->instance_id );
		}
	}

	if( sd.guild != nullptr && sd.guild->instance_id > 0 ){
		instance_reqinfo( &sd, sd.guild->instance_id );
	}

	if( sd.clan != nullptr && sd.clan->instance_id > 0 ){
		instance_reqinfo( &sd, sd.clan->instance_id );
	}
}

/// Notifies clients about item picked up by a party member.
/// 02b8 <account id>.L <name id>.W <identified>.B <damaged>.B <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W <equip location>.W <item type>.B (ZC_ITEM_PICKUP_PARTY)
void clif_party_show_picker( map_session_data* sd, struct item* item_data ){
#if PACKETVER >= 20071002
	nullpo_retv( sd );
	nullpo_retv( item_data );

	struct item_data* id = itemdb_search( item_data->nameid );

	PACKET_ZC_ITEM_PICKUP_PARTY p = {};

	p.packetType = HEADER_ZC_ITEM_PICKUP_PARTY;
	p.AID = sd->status.account_id;
	p.itemId = client_nameid( item_data->nameid );
	p.identified = item_data->identify;
	p.damaged = item_data->attribute;
	p.refine = item_data->refine;
	clif_addcards( &p.slot, item_data );
	p.location = id->equip; // equip location
	p.itemType = itemtype( id->nameid ); // item type
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	p.grade = item_data->enchantgrade;
#endif

	clif_send( &p, sizeof( p ), &sd->bl, PARTY_SAMEMAP_WOS );
#endif
}

/** Display gained exp.
 * 07f6 <account id>.L <amount>.L <var id>.W <exp type>.W (ZC_NOTIFY_EXP)
 * 0acc <account id>.L <amount>.Q <var id>.W <exp type>.W (ZC_NOTIFY_EXP2)
 * amount: INT32_MIN ~ INT32_MAX
 * var id:
 *     SP_BASEEXP, SP_JOBEXP
 * exp type:
 *     0 = normal exp gained/lost
 *     1 = quest exp gained/lost
 * @param sd Player
 * @param exp EXP value gained/loss
 * @param type SP_BASEEXP, SP_JOBEXP
 * @param quest False:Normal EXP; True:Quest EXP (displayed in purple color)
 * @param lost True:if losing EXP
 */
void clif_displayexp(map_session_data *sd, t_exp exp, char type, bool quest, bool lost)
{
	int fd;
	int offset;
#if PACKETVER >= 20170830
	int cmd = 0xacc;
#else
	int cmd = 0x7f6;
#endif

	nullpo_retv(sd);

	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(cmd));
	WFIFOW(fd,0) = cmd;
	WFIFOL(fd,2) = sd->bl.id;
#if PACKETVER >= 20170830
	WFIFOQ(fd,6) = client_exp(exp) * (lost ? -1 : 1);
	offset = 4;
#else
	WFIFOL(fd,6) = client_exp(exp) * (lost ? -1 : 1);
	offset = 0;
#endif
	WFIFOW(fd,10+offset) = type;
	WFIFOW(fd,12+offset) = (quest && type != SP_JOBEXP) ? 1 : 0; // NOTE: Somehow JobEXP always in yellow color
	WFIFOSET(fd,packet_len(cmd));
}


/// Displays digital clock digits on top of the screen (ZC_SHOWDIGIT).
/// type:
///     0 = Displays 'value' for 5 seconds.
///     1 = Incremental counter (1 tick/second), negated 'value' specifies start value (e.g. using -10 lets the counter start at 10).
///     2 = Decremental counter (1 tick/second), negated 'value' specifies start value (does not stop when reaching 0, but overflows).
///     3 = Decremental counter (1 tick/second), 'value' specifies start value (stops when reaching 0, displays at most 2 digits).
/// value:
///     Except for type 3 it is interpreted as seconds for displaying as DD:HH:MM:SS, HH:MM:SS, MM:SS or SS (leftmost '00' is not displayed).
void clif_showdigit(map_session_data* sd, unsigned char type, int value)
{
	WFIFOHEAD(sd->fd, packet_len(0x1b1));
	WFIFOW(sd->fd,0) = 0x1b1;
	WFIFOB(sd->fd,2) = type;
	WFIFOL(sd->fd,3) = value;
	WFIFOSET(sd->fd, packet_len(0x1b1));
}


/// Notification of the state of client command /effect (CZ_LESSEFFECT).
/// 021d <state>.L
/// state:
///     0 = Full effects
///     1 = Reduced effects
///
/// NOTE:   The state is used on Aegis for sending skill unit packet
///         0x11f (ZC_SKILL_ENTRY) instead of 0x1c9 (ZC_SKILL_ENTRY2)
///         whenever possible. Due to the way the decision check is
///         constructed, this state tracking was rendered useless,
///         as the only skill unit, that is sent with 0x1c9 is
///         Graffiti.
void clif_parse_LessEffect(int fd, map_session_data* sd){
	int isLess = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);
	sd->state.lesseffect = ( isLess != 0 );
}

/// 07e4 <length>.w <option>.l <val>.l {<index>.w <amount>.w).4b* (CZ_ITEMLISTWIN_RES)
void clif_parse_ItemListWindowSelected(int fd, map_session_data* sd) {
	if( sd == nullptr ){
		return;
	}

	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int n = (RFIFOW(fd,info->pos[0])-12) / 4;
	int type = RFIFOL(fd,info->pos[1]);
	int flag = RFIFOL(fd,info->pos[2]); // Button clicked: 0 = Cancel, 1 = OK
	unsigned short* item_list = (unsigned short*)RFIFOP(fd,info->pos[3]);

	if( sd->state.trading || sd->npc_shopid )
		return;

	if( flag == 0 || n == 0) {
		clif_menuskill_clear(sd);
		return; // Canceled by player.
	}

	if( sd->menuskill_id != SO_EL_ANALYSIS && sd->menuskill_id != GN_CHANGEMATERIAL ) {
		clif_menuskill_clear(sd);
		return; // Prevent hacking.
	}

	switch( type ) {
		case 0: // Change Material
			skill_changematerial(sd,n,item_list);
			break;
		case 1:	// Level 1: Pure to Rough
		case 2:	// Level 2: Rough to Pure
			skill_elementalanalysis(*sd,n,type,item_list);
			break;
	}
	clif_menuskill_clear(sd);

	return;
}

/*==========================================
 * Elemental System
 *==========================================*/

/// Notifies client of a change in an elemental's status parameter.
/// 0x81e <type>.W <value>.L (ZC_EL_PAR_CHANGE)
void clif_elemental_updatestatus(map_session_data& sd, _sp type) {
#if PACKETVER >= 20100309
	if (sd.ed == nullptr)
		return;

	PACKET_ZC_EL_PAR_CHANGE p = {};

	p.packetType = HEADER_ZC_EL_PAR_CHANGE;
	p.type = static_cast<decltype(p.type)>(type);
	status_data* status = &sd.ed->battle_status;
	switch( type ) {
		case SP_HP:
			p.value = static_cast<decltype(p.value)>(status->hp);
			break;
		case SP_MAXHP:
			p.value = static_cast<decltype(p.value)>(status->max_hp);
			break;
		case SP_SP:
			p.value = static_cast<decltype(p.value)>(status->sp);
			break;
		case SP_MAXSP:
			p.value = static_cast<decltype(p.value)>(status->max_sp);
			break;
	}

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

void clif_elemental_info(map_session_data *sd) {
	int fd;
	s_elemental_data *ed;
	struct status_data *status;

	if( !clif_session_isValid(sd) || (ed = sd->ed) == nullptr )
		return;

	fd = sd->fd;
	status = &ed->battle_status;

	WFIFOHEAD(fd,22);
	WFIFOW(fd, 0) = 0x81d;
	WFIFOL(fd, 2) = ed->bl.id;
	WFIFOL(fd, 6) = status->hp;
	WFIFOL(fd,10) = status->max_hp;
	WFIFOL(fd,14) = status->sp;
	WFIFOL(fd,18) = status->max_sp;
	WFIFOSET(fd,22);
}


/// Buying Store System
///

/// Opens preparation window for buying store (ZC_OPEN_BUYING_STORE).
/// 0810 <slots>.B
void clif_buyingstore_open(map_session_data* sd)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x810));
	WFIFOW(fd,0) = 0x810;
	WFIFOB(fd,2) = sd->buyingstore.slots;
	WFIFOSET(fd,packet_len(0x810));
}


/// Request to create a buying store.
/// 0811 <packet len>.W <limit zeny>.L <result>.B <store name>.80B { <name id>.W <amount>.W <price>.L }* (CZ_REQ_OPEN_BUYING_STORE)
/// result:
///     0 = cancel
///     1 = open
static void clif_parse_ReqOpenBuyingStore( int fd, map_session_data* sd ){
	const PACKET_CZ_REQ_OPEN_BUYING_STORE* p = reinterpret_cast<PACKET_CZ_REQ_OPEN_BUYING_STORE*>( RFIFOP( fd, 0 ) );

	// TODO: Make this check global for all variable length packets.
	// minimum packet length
	if( p->packetLength < sizeof( *p ) ){
		ShowError( "clif_parse_ReqOpenBuyingStore: Malformed packet (expected length=%" PRIdPTR ", length=%u, account_id=%d).\n", sizeof( *p ), p->packetLength, sd->bl.id );
		return;
	}

	if (map_getmapflag(sd->bl.m, MF_NOBUYINGSTORE)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 276)); // "You can't open a shop on this map"
		return;
	}
	if (map_getcell(sd->bl.m, sd->bl.x, sd->bl.y, CELL_CHKNOBUYINGSTORE)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 204)); // "You can't open a shop on this cell."
		return;
	}

	char storename[MESSAGE_SIZE];

	safestrncpy( storename, p->storeName, sizeof( storename ) );

	// so that buyingstore_create knows, how many elements it has access to
	int packet_len = p->packetLength - sizeof( *p );

	if( packet_len % sizeof( p->items[0] ) ){
		ShowError( "clif_parse_ReqOpenBuyingStore: Unexpected item list size %u (account_id=%d, block size=%" PRIdPTR ")\n", packet_len, sd->bl.id, sizeof( p->items[0] ) );
		return;
	}

	buyingstore_create( sd, p->zenyLimit, p->result, storename, p->items, packet_len / sizeof( p->items[0] ), nullptr );
}


/// Notification, that the requested buying store could not be created (ZC_FAILED_OPEN_BUYING_STORE_TO_BUYER).
/// 0812 <result>.W <total weight>.L
/// result:
///     1 = "Failed to open buying store." (0x6cd, MSI_BUYINGSTORE_OPEN_FAILED)
///     2 = "Total amount of then possessed items exceeds the weight limit by <weight/10-maxweight*90%>. Please re-enter." (0x6ce, MSI_BUYINGSTORE_OVERWEIGHT)
///     8 = "No sale (purchase) information available." (0x705)
///     ? = nothing
void clif_buyingstore_open_failed(map_session_data* sd, unsigned short result, unsigned int weight)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x812));
	WFIFOW(fd,0) = 0x812;
	WFIFOW(fd,2) = result;
	WFIFOL(fd,4) = weight;
	WFIFOSET(fd,packet_len(0x812));
}


/// Notification, that the requested buying store was created.
/// 0813 <packet len>.W <account id>.L <limit zeny>.L { <price>.L <count>.W <type>.B <name id>.W }* (ZC_MYITEMLIST_BUYING_STORE)
void clif_buyingstore_myitemlist( map_session_data& sd ){
#if PACKETVER >= 20100309
	PACKET_ZC_MYITEMLIST_BUYING_STORE* p = reinterpret_cast<PACKET_ZC_MYITEMLIST_BUYING_STORE*>( packet_buffer );

	p->packetType = HEADER_ZC_MYITEMLIST_BUYING_STORE;
	p->packetLength = sizeof( *p );
	p->AID = sd.bl.id;
	p->zenyLimit = sd.buyingstore.zenylimit;

	for( int i = 0; i < sd.buyingstore.slots; i++ ){
		PACKET_ZC_MYITEMLIST_BUYING_STORE_sub& entry = p->items[i];

		entry.price = sd.buyingstore.items[i].price;
		entry.amount = sd.buyingstore.items[i].amount;
		entry.itemType = itemtype( sd.buyingstore.items[i].nameid );
		entry.itemId = client_nameid( sd.buyingstore.items[i].nameid );

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
}


/// Notifies clients in area of a buying store (ZC_BUYING_STORE_ENTRY).
/// 0814 <account id>.L <store name>.80B
void clif_buyingstore_entry( map_session_data& sd, struct block_list* tbl ){
#if PACKETVER >= 20100420
	enum send_target target;

	if( tbl == nullptr ){
		tbl = &sd.bl;
		target = AREA_WOS;
	}else{
		target = SELF;
	}

	PACKET_ZC_BUYING_STORE_ENTRY p = {};

	p.packetType = HEADER_ZC_BUYING_STORE_ENTRY;
	p.makerAID = sd.status.account_id;
	safestrncpy( p.storeName, sd.message, sizeof( p.storeName ) );

	clif_send( &p, sizeof( p ), tbl, target );
#endif
}

/// Request to close own buying store (CZ_REQ_CLOSE_BUYING_STORE).
/// 0815
static void clif_parse_ReqCloseBuyingStore(int fd, map_session_data* sd)
{
	buyingstore_close(sd);
}


/// Notifies clients in area that a buying store was closed (ZC_DISAPPEAR_BUYING_STORE_ENTRY).
/// 0816 <account id>.L
void clif_buyingstore_disappear_entry( map_session_data& sd, struct block_list* tbl ){
#if PACKETVER >= 20100309
	enum send_target target;

	if( tbl == nullptr ){
		tbl = &sd.bl;
		target = AREA_WOS;
	}else{
		target = SELF;
	}

	PACKET_ZC_DISAPPEAR_BUYING_STORE_ENTRY p = {};

	p.packetType = HEADER_ZC_DISAPPEAR_BUYING_STORE_ENTRY;
	p.makerAID = sd.status.account_id;

	clif_send( &p, sizeof( p ), tbl, target );
#endif
}

/// Request to open someone else's buying store (CZ_REQ_CLICK_TO_BUYING_STORE).
/// 0817 <account id>.L
static void clif_parse_ReqClickBuyingStore(int fd, map_session_data* sd)
{
	uint32 account_id;

	account_id = RFIFOL(fd,packet_db[RFIFOW(fd,0)].pos[0]);

	buyingstore_open(sd, account_id);
}


/// Sends buying store item list.
/// 0818 <packet len>.W <account id>.L <store id>.L <limit zeny>.L { <price>.L <amount>.W <type>.B <name id>.W }* (ZC_ACK_ITEMLIST_BUYING_STORE)
void clif_buyingstore_itemlist( map_session_data& sd, map_session_data& pl_sd ){
#if PACKETVER >= 20100309
	PACKET_ZC_ACK_ITEMLIST_BUYING_STORE* p = reinterpret_cast<PACKET_ZC_ACK_ITEMLIST_BUYING_STORE*>( packet_buffer );

	p->packetType = HEADER_ZC_ACK_ITEMLIST_BUYING_STORE;
	p->packetLength = sizeof( *p );
	p->AID = pl_sd.bl.id;
	p->storeId = pl_sd.buyer_id;
	p->zenyLimit = pl_sd.buyingstore.zenylimit;

	for( int i = 0; i < pl_sd.buyingstore.slots; i++ ){
		PACKET_ZC_ACK_ITEMLIST_BUYING_STORE_sub& entry = p->items[i];

		entry.price = pl_sd.buyingstore.items[i].price;
		entry.amount = pl_sd.buyingstore.items[i].amount;  // TODO: Figure out, if no longer needed items (amount == 0) are listed on official.
		entry.itemType = itemtype(pl_sd.buyingstore.items[i].nameid);
		entry.itemId = client_nameid( pl_sd.buyingstore.items[i].nameid );

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
}


/// Request to sell items to a buying store.
/// 0819 <packet len>.W <account id>.L <store id>.L { <index>.W <name id>.W <amount>.W }* (CZ_REQ_TRADE_BUYING_STORE)
static void clif_parse_ReqTradeBuyingStore( int fd, map_session_data* sd ){
	const PACKET_CZ_REQ_TRADE_BUYING_STORE* p = reinterpret_cast<PACKET_CZ_REQ_TRADE_BUYING_STORE*>( RFIFOP( fd, 0 ) );

	// minimum packet length
	if( p->packetLength < sizeof( *p ) ){
		ShowError( "clif_parse_ReqTradeBuyingStore: Malformed packet (expected length=%" PRIdPTR ", length=%u, account_id=%d).\n", sizeof( *p ), p->packetLength, sd->bl.id );
		return;
	}

	// so that buyingstore_trade knows, how many elements it has access to
	int packet_len = p->packetLength - sizeof( *p );

	if( packet_len % sizeof( p->items[0] ) ){
		ShowError( "clif_parse_ReqTradeBuyingStore: Unexpected item list size %u (account_id=%d, buyer_id=%d, block size=%" PRIdPTR ")\n", packet_len, sd->bl.id, p->AID, sizeof( p->items[0] ) );
		return;
	}

	buyingstore_trade( sd, p->AID, p->storeId, p->items, packet_len / sizeof( p->items[0] ) );
}


/// Notifies the buyer, that the buying store has been closed due to a post-trade condition (ZC_FAILED_TRADE_BUYING_STORE_TO_BUYER).
/// 081a <result>.W
/// result:
///     3 = "All items within the buy limit were purchased." (0x6cf, MSI_BUYINGSTORE_TRADE_OVERLIMITZENY)
///     4 = "All items were purchased." (0x6d0, MSI_BUYINGSTORE_TRADE_BUYCOMPLETE)
///     ? = nothing
void clif_buyingstore_trade_failed_buyer(map_session_data* sd, short result)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x81a));
	WFIFOW(fd,0) = 0x81a;
	WFIFOW(fd,2) = result;
	WFIFOSET(fd,packet_len(0x81a));
}


/// Updates the zeny limit and an item in the buying store item list.
/// 081b <name id>.W <amount>.W <limit zeny>.L (ZC_UPDATE_ITEM_FROM_BUYING_STORE)
/// 09e6 <name id>.W <amount>.W <zeny>.L <limit zeny>.L <GID>.L <Date>.L (ZC_UPDATE_ITEM_FROM_BUYING_STORE2)
void clif_buyingstore_update_item( map_session_data* sd, t_itemid nameid, unsigned short amount, uint32 char_id, int zeny ){
	PACKET_ZC_UPDATE_ITEM_FROM_BUYING_STORE p = {};

	p.packetType = buyingStoreUpdateItemType;
	p.itemId = client_nameid( nameid );
	p.amount = amount;
	p.zenyLimit = sd->buyingstore.zenylimit;
#if PACKETVER >= 20141016
	p.zeny = zeny;
	p.charId = char_id;  // GID
	p.updateTime = (int)time(nullptr);
#endif

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Deletes item from inventory, that was sold to a buying store (ZC_ITEM_DELETE_BUYING_STORE).
/// 081c <index>.W <amount>.W <price>.L
/// message:
///     "%s (%d) were sold at %dz." (0x6d2, MSI_BUYINGSTORE_TRADE_SELLCOMPLETE)
///
/// NOTE:   This function has to be called _instead_ of clif_delitem/clif_dropitem.
void clif_buyingstore_delete_item(map_session_data* sd, short index, unsigned short amount, int price)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x81c));
	WFIFOW(fd,0) = 0x81c;
	WFIFOW(fd,2) = client_index( index );
	WFIFOW(fd,4) = amount;
	WFIFOL(fd,6) = price;  // price per item, client calculates total Zeny by itself
	WFIFOSET(fd,packet_len(0x81c));
}


/// Notifies the seller, that a buying store trade failed.
/// 0824 <result>.W <name id>.W (ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER)
/// result:
///     5 = "The deal has failed." (0x39, MSI_DEAL_FAIL)
///     6 = "The trade failed, because the entered amount of item %s is higher, than the buyer is willing to buy." (0x6d3, MSI_BUYINGSTORE_TRADE_OVERCOUNT)
///     7 = "The trade failed, because the buyer is lacking required balance." (0x6d1, MSI_BUYINGSTORE_TRADE_LACKBUYERZENY)
///     ? = nothing
void clif_buyingstore_trade_failed_seller( map_session_data* sd, short result, t_itemid nameid ){
	PACKET_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER p = {};

	p.packetType = HEADER_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER;
	p.result = result;
	p.itemId = client_nameid( nameid );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}


/// Search Store Info System
///

/// Request to search for stores.
/// 0835 <packet len>.W <type>.B <max price>.L <min price>.L <name id count>.B <card count>.B { <name id>.W }* { <card>.W }* (CZ_SEARCH_STORE_INFO)
/// type:
///     0 = Vending
///     1 = Buying Store
///
/// NOTE:   The client determines the item ids by specifying a name and optionally,
///         amount of card slots. If the client does not know about the item it
///         cannot be searched.
static void clif_parse_SearchStoreInfo( int fd, map_session_data *sd ){
	if (!battle_config.feature_search_stores)
		return;

	const PACKET_CZ_SEARCH_STORE_INFO* p = reinterpret_cast<PACKET_CZ_SEARCH_STORE_INFO*>( RFIFOP( fd, 0 ) );

	// minimum packet length
	if( p->packetLength < sizeof( *p ) ){
		ShowError( "clif_parse_SearchStoreInfo: Malformed packet (expected length=%" PRIdPTR ", length=%u, account_id=%d).\n", sizeof( *p ), p->packetLength, sd->bl.id );
		return;
	}

	// check, if there is enough data for the claimed count of items
	int packet_len = p->packetLength - sizeof( *p );

	if( packet_len % sizeof( p->items[0] ) ){
		ShowError( "clif_parse_SearchStoreInfo: Unexpected item list size %u (account_id=%d, block size=%" PRIdPTR ")\n", packet_len, sd->bl.id, sizeof( p->items[0] ) );
		return;
	}

	int count = packet_len / sizeof( p->items[0] );

	if( count < p->itemsCount + p->cardsCount ){
		ShowError( "clif_parse_SearchStoreInfo: Malformed packet (expected count=%u, count=%u, account_id=%d).\n", p->itemsCount + p->cardsCount, count, sd->bl.id );
		return;
	}

	if ( p->searchType > SEARCHTYPE_BUYING_STORE ) {
		ShowError( "clif_parse_SearchStoreInfo: Invalid search type %u (account_id=%d).\n", p->searchType, sd->bl.id );
		return;
	}

	if ( p->minPrice > battle_config.vending_max_value ) {
		ShowError( "clif_parse_SearchStoreInfo: Invalid min price %u (account_id=%d).\n", p->minPrice, sd->bl.id );
		return;
	}

	if ( p->maxPrice > battle_config.vending_max_value ) {
		ShowError( "clif_parse_SearchStoreInfo: Invalid max price %u (account_id=%d).\n", p->maxPrice, sd->bl.id );
		return;
	}

	searchstore_query( *sd, static_cast<e_searchstore_searchtype>(p->searchType), p->minPrice, p->maxPrice, &p->items[0], p->itemsCount, &p->items[p->itemsCount], p->cardsCount );
}


/// Results for a store search request.
/// 0836 <packet len>.W <is first page>.B <is next page>.B <remaining uses>.B { <store id>.L <account id>.L <shop name>.80B <nameid>.W <item type>.B <price>.L <amount>.W <refine>.B <card1>.W <card2>.W <card3>.W <card4>.W }* (ZC_SEARCH_STORE_INFO_ACK)
/// is first page:
///     0 = appends to existing results
///     1 = clears previous results before displaying this result set
/// is next page:
///     0 = no "next" label
///     1 = "next" label to retrieve more results
void clif_search_store_info_ack( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20100817 || PACKETVER_RE_NUM >= 20100706 || defined(PACKETVER_ZERO)
	if (!battle_config.feature_search_stores)
		return;

	uint32 start = sd.searchstore.pages * SEARCHSTORE_RESULTS_PER_PAGE ;
	uint32 end = umin( static_cast<uint32>( sd.searchstore.items.size() ), start + SEARCHSTORE_RESULTS_PER_PAGE );

	PACKET_ZC_SEARCH_STORE_INFO_ACK* p = reinterpret_cast<PACKET_ZC_SEARCH_STORE_INFO_ACK*>( packet_buffer );

	p->packetType = HEADER_ZC_SEARCH_STORE_INFO_ACK;
	p->packetLength = sizeof( *p );
	p->firstPage = !sd.searchstore.pages;
	p->nextPage = searchstore_querynext( sd );
	p->usesCount = static_cast<decltype(p->usesCount)>( std::min<decltype(sd.searchstore.uses)>( sd.searchstore.uses, std::numeric_limits<decltype(p->usesCount)>::max() ) );

	for( int i = 0, count = 0; i < end - start; i++ ){
		std::shared_ptr<s_search_store_info_item> ssitem = sd.searchstore.items[start + i];

		if( ssitem == nullptr ){
			continue;
		}


		PACKET_ZC_SEARCH_STORE_INFO_ACK_sub& entry = p->items[count];

		entry.storeId = ssitem->store_id;
		entry.AID = ssitem->account_id;
		safestrncpy( entry.shopName, ssitem->store_name, MESSAGE_SIZE );
		entry.itemId = client_nameid( ssitem->nameid );
		entry.itemType = itemtype( ssitem->nameid );
		entry.price = ssitem->price;
		entry.amount = ssitem->amount;
		entry.refine = ssitem->refine;

		// make-up an item for clif_addcards
		struct item it = {};

		for( int j = 0; j < MAX_SLOTS; j++ ){
			it.card[j] = ssitem->card[j];
		}
		it.nameid = ssitem->nameid;
		it.amount = ssitem->amount;

		clif_addcards( &entry.slot, &it );
#if PACKETVER >= 20150226
		clif_add_random_options( entry.option_data, it );
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
		entry.grade = ssitem->enchantgrade;
#endif
#endif

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
		count++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
}


/// Notification of failure when searching for stores.
/// 0837 <reason>.B (ZC_SEARCH_STORE_INFO_FAILED)
void clif_search_store_info_failed(map_session_data& sd, e_searchstore_failure reason){
#if PACKETVER_MAIN_NUM >= 20100601 || PACKETVER_RE_NUM >= 20100601 || defined(PACKETVER_ZERO)
	if (!battle_config.feature_search_stores)
		return;

	PACKET_ZC_SEARCH_STORE_INFO_FAILED packet{};

	packet.packetType = HEADER_ZC_SEARCH_STORE_INFO_FAILED;
	packet.reason = static_cast<decltype(packet.reason)>(reason);

	clif_send(&packet, sizeof(packet), &sd.bl, SELF);
#endif
}


/// Request to display next page of results.
/// 0838  (CZ_SEARCH_STORE_INFO_NEXT_PAGE)
static void clif_parse_SearchStoreInfoNextPage(int fd, map_session_data* sd){
	if (!battle_config.feature_search_stores)
		return;

	searchstore_next(*sd);
}


/// Opens the search store window.
/// 083a <effect>.W <remaining uses>.B (ZC_OPEN_SEARCH_STORE_INFO)
void clif_open_search_store_info(map_session_data& sd){
#if PACKETVER_MAIN_NUM >= 20100701 || PACKETVER_RE_NUM >= 20100701 || defined(PACKETVER_ZERO)
	if (!battle_config.feature_search_stores)
		return;

	PACKET_ZC_OPEN_SEARCH_STORE_INFO packet{};

	packet.packetType = HEADER_ZC_OPEN_SEARCH_STORE_INFO;
	packet.effect = static_cast<decltype(packet.effect)>(sd.searchstore.effect);
#if PACKETVER_MAIN_NUM >= 20100701 || PACKETVER_RE_NUM >= 20100701 || defined(PACKETVER_ZERO)
	packet.remainingUses = static_cast<decltype(packet.remainingUses)>( std::min<decltype(sd.searchstore.uses)>( sd.searchstore.uses, std::numeric_limits<decltype(packet.remainingUses)>::max() ) );
#endif

	clif_send(&packet, sizeof(packet), &sd.bl, SELF);
#endif
}


/// Request to close the store search window.
/// 083b (CZ_CLOSE_SEARCH_STORE_INFO)
static void clif_parse_CloseSearchStoreInfo(int fd, map_session_data* sd){
	if (!battle_config.feature_search_stores)
		return;

	searchstore_close(*sd);
}


/// Request to invoke catalog effect on a store from search results.
/// 083c <account id>.L <store id>.L <nameid>.W (CZ_SSILIST_ITEM_CLICK)
static void clif_parse_SearchStoreInfoListItemClick( int fd, map_session_data* sd ){
	if (!battle_config.feature_search_stores)
		return;

	const PACKET_CZ_SSILIST_ITEM_CLICK* p = reinterpret_cast<PACKET_CZ_SSILIST_ITEM_CLICK*>( RFIFOP( fd, 0 ) );

	searchstore_click( *sd, p->AID, p->storeId, p->itemId );
}


/// Notification of the store position on current map.
/// 083d <xPos>.W <yPos>.W (ZC_SSILIST_ITEM_CLICK_ACK)
void clif_search_store_info_click_ack(map_session_data& sd, int16 x, int16 y){
#if PACKETVER_MAIN_NUM >= 20100608 || PACKETVER_RE_NUM >= 20100608 || defined(PACKETVER_ZERO)
	if (!battle_config.feature_search_stores)
		return;

	PACKET_ZC_SSILIST_ITEM_CLICK_ACK packet{};

	packet.packetType = HEADER_ZC_SSILIST_ITEM_CLICK_ACK;
	packet.x = x;
	packet.y = y;

	clif_send(&packet, sizeof(packet), &sd.bl, SELF);
#endif
}


/// Parse function for packet debugging.
void clif_parse_debug(int fd,map_session_data *sd)
{
	int16 packet_len;

	// clif_parse ensures, that there is at least 2 bytes of data
	uint16 cmd = RFIFOW(fd,0);

	if( sd ) {
		packet_len = packet_db[cmd].len;

		if( packet_len == 0 )
		{// unknown
			packet_len = static_cast<decltype(packet_len)>( RFIFOREST(fd) );
		}
		else if( packet_len == -1 )
		{// variable length
			packet_len = RFIFOW(fd,2);  // clif_parse ensures, that this amount of data is already received
		}
		ShowDebug("Packet debug of 0x%04X (length %d), %s session #%d, %d/%d (AID/CID)\n", cmd, packet_len, sd->state.active ? "authed" : "unauthed", fd, sd->status.account_id, sd->status.char_id);
	}
	else
	{
		packet_len = static_cast<decltype(packet_len)>( RFIFOREST(fd) );
		ShowDebug("Packet debug of 0x%04X (length %d), session #%d\n", cmd, packet_len, fd);
	}

	ShowDump(RFIFOP(fd,0), packet_len);
}
/*==========================================
 * Server tells client to display a window similar to Magnifier (item) one
 * Server populates the window with avilable elemental converter options according to player's inventory
 *------------------------------------------*/
void clif_elementalconverter_list( map_session_data& sd ){
	PACKET_ZC_MAKINGARROW_LIST* p = reinterpret_cast<PACKET_ZC_MAKINGARROW_LIST*>( packet_buffer );

	p->packetType = HEADER_ZC_MAKINGARROW_LIST;
	p->packetLength = sizeof( *p );

	int count = 0;
	for( int i = 0; i < MAX_SKILL_PRODUCE_DB; i++ ){
		if( skill_can_produce_mix( &sd, skill_produce_db[i].nameid, 23, 1 ) ){
			p->items[count].itemId = client_nameid( skill_produce_db[i].nameid );
			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
			count++;
		}
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );

	if( count > 0 ){
		sd.menuskill_id = SA_CREATECON;
		sd.menuskill_val = count;
	}
}

/**
 * Rune Knight
 **/
void clif_millenniumshield( block_list& bl, int16 shields ){
#if PACKETVER >= 20081126
	PACKET_ZC_MILLENNIUMSHIELD packet{};

	packet.packetType = HEADER_ZC_MILLENNIUMSHIELD;
	packet.aid = bl.id;
	packet.num = shields;
	packet.state = 0;

	clif_send( &packet, sizeof( packet ), &bl, AREA );
#endif
}

/**
 * Mechanic
 **/
/*==========================================
 * Magic Decoy Material List
 *------------------------------------------*/
void clif_magicdecoy_list( map_session_data& sd, uint16 skill_lv, short x, short y ){
	PACKET_ZC_MAKINGARROW_LIST* p = reinterpret_cast<PACKET_ZC_MAKINGARROW_LIST*>( packet_buffer );

	p->packetType = HEADER_ZC_MAKINGARROW_LIST;
	p->packetLength = sizeof( *p );

	size_t count = 0;
	for( size_t i = 0; i < MAX_INVENTORY; i++ ){
		if( itemdb_group.item_exists( IG_ELEMENT, sd.inventory.u.items_inventory[i].nameid ) ){
			p->items[count].itemId = client_nameid( sd.inventory.u.items_inventory[i].nameid );
			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
			count++;
		}
	}

	if( count > 0 ){
		clif_send( p, p->packetLength, &sd.bl, SELF );

		sd.menuskill_id = NC_MAGICDECOY;
		sd.menuskill_val = skill_lv;
		sd.sc.comet_x = x;
		sd.sc.comet_y = y;
	}else{
		clif_skill_fail( sd, NC_MAGICDECOY );
	}
}
/*==========================================
 * Guillotine Cross Poisons List
 *------------------------------------------*/
void clif_poison_list( map_session_data& sd, uint16 skill_lv ){
	PACKET_ZC_MAKINGARROW_LIST* p = reinterpret_cast<PACKET_ZC_MAKINGARROW_LIST*>( packet_buffer );

	p->packetType = HEADER_ZC_MAKINGARROW_LIST;
	p->packetLength = sizeof( *p );

	size_t count = 0;
	for( size_t i = 0; i < MAX_INVENTORY; i++ ){
		if( itemdb_group.item_exists( IG_POISON, sd.inventory.u.items_inventory[i].nameid ) ){
			p->items[count].itemId = client_nameid( sd.inventory.u.items_inventory[i].nameid );
			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->items[0] ) );
			count++;
		}
	}

	if( count > 0 ){
		clif_send( p, p->packetLength, &sd.bl, SELF );

		sd.menuskill_id = GC_POISONINGWEAPON;
		sd.menuskill_val = skill_lv;
	}else{
		clif_skill_fail( sd, GC_POISONINGWEAPON, USESKILL_FAIL_GUILLONTINE_POISON );
	}
}

/// 0442 <Length>.W <why>.L { <Skill_Id>.W }* (ZC_SKILL_SELECT_REQUEST).
void clif_autoshadowspell_list( map_session_data& sd ){
#if PACKETVER >= 20081210
	if( sd.menuskill_id == SC_AUTOSHADOWSPELL )
		return;

	PACKET_ZC_SKILL_SELECT_REQUEST* p = reinterpret_cast<PACKET_ZC_SKILL_SELECT_REQUEST*>( packet_buffer );

	p->packetType = HEADER_ZC_SKILL_SELECT_REQUEST;
	p->packetLength = sizeof( *p );
	p->flag = 1; // enum PACKET_ZC_SKILL_SELECT_REQUEST::enumWHY::WHY_SC_AUTOSHADOWSPELL =  0x1
	
	size_t count = 0;
	for( size_t i = 0; i < MAX_SKILL; i++ ){
		if( sd.status.skill[i].flag == SKILL_FLAG_PLAGIARIZED && sd.status.skill[i].id > 0 &&
			skill_get_inf2(sd.status.skill[i].id, INF2_ISAUTOSHADOWSPELL))
		{
			p->skillIds[count] = sd.status.skill[i].id;
			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->skillIds[0] ) );
			count++;
		}
	}

	if( count > 0 ) {
		clif_send( p, p->packetLength, &sd.bl, SELF );

		sd.menuskill_id = SC_AUTOSHADOWSPELL;
		sd.menuskill_val = static_cast<decltype(sd.menuskill_val)>( count );
	} else {
		status_change_end( &sd.bl, SC_STOP );
		clif_skill_fail( sd, SC_AUTOSHADOWSPELL, USESKILL_FAIL_IMITATION_SKILL_NONE );
	}
#endif
}

/*===========================================
 * Skill list for Four Elemental Analysis
 * and Change Material skills.
 * 07e3 <type>.L (ZC_ITEMLISTWIN_OPEN).
 *------------------------------------------*/
int clif_skill_itemlistwindow( map_session_data *sd, uint16 skill_id, uint16 skill_lv ) {
#if PACKETVER >= 20090922
	int fd = sd->fd;

	nullpo_ret(sd);

	sd->menuskill_id = skill_id; // To prevent hacking.
	sd->menuskill_val = skill_lv;

	if( skill_id == GN_CHANGEMATERIAL )
		skill_lv = 0; // Changematerial

	WFIFOHEAD(fd, packet_len(0x7e3));
	WFIFOW(fd,0) = 0x7e3;
	WFIFOL(fd,2) = skill_lv;
	WFIFOL(fd,4) = 0;
	WFIFOSET(fd, packet_len(0x7e3));
#endif
	return 1;
}

/*==========================================
 * Select a skill into a given list (used by SA_AUTOSPELL/SC_AUTOSHADOWSPELL)
 * 0443 <type>.L <skill_id>.W (CZ_SKILL_SELECT_RESPONSE)
 *------------------------------------------*/
void clif_parse_SkillSelectMenu(int fd, map_session_data *sd) {
#if PACKETVER >= 20081210
	if( sd == nullptr ){
		return;
	}

	const PACKET_CZ_SKILL_SELECT_RESPONSE* p = reinterpret_cast<PACKET_CZ_SKILL_SELECT_RESPONSE*>( RFIFOP( fd, 0 ) );

	if (sd->menuskill_id == SA_AUTOSPELL) {
		sd->state.workinprogress = WIP_DISABLE_NONE;
		skill_autospell(sd, p->selectedSkillId);
	} else if (sd->menuskill_id == SC_AUTOSHADOWSPELL) {
		if (pc_istrading(sd)) {
			clif_skill_fail( *sd, sd->ud.skill_id );
			clif_menuskill_clear(sd);
			return;
		}

		skill_select_menu(*sd, p->selectedSkillId);
	} else
		return;

	clif_menuskill_clear(sd);
#endif
}


/// Kagerou/Oboro amulet spirit.
/// 08cf <id>.L <type>.W <num>.W (ZC_SPIRITS_ATTRIBUTE)
void clif_spiritcharm( map_session_data& sd ){
#if PACKETVER >= 20111102
	PACKET_ZC_SPIRITS_ATTRIBUTE packet{};

	packet.packetType = HEADER_ZC_SPIRITS_ATTRIBUTE;
	packet.aid = sd.bl.id;
	packet.spiritsType = static_cast<decltype(packet.spiritsType)>(sd.spiritcharm_type);
	packet.num = static_cast<decltype(packet.num)>(sd.spiritcharm);

	clif_send( &packet, sizeof( packet ), &sd.bl, AREA );
#endif
}


/// Move Item from or to Personal Tab
/// 0907 <index>.W <type>.B (CZ_INVENTORY_TAB)
/// type:
/// 	0 = move item to personal tab
/// 	1 = move item to normal tab
void clif_parse_MoveItem( int fd, map_session_data* sd ){
// TODO: Check for correct packet version
#if PACKETVER >= 20120410
	/* can't move while dead. */
	if(pc_isdead(sd)) {
		return;
	}

	const PACKET_CZ_INVENTORY_TAB* p = reinterpret_cast<PACKET_CZ_INVENTORY_TAB*>( RFIFOP( fd, 0 ) );

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if ( sd->inventory.u.items_inventory[index].favorite != 0 && p->favorite == true )
		sd->inventory.u.items_inventory[index].favorite = 0;
	else if( p->favorite == false )
		sd->inventory.u.items_inventory[index].favorite = 1;
	else
		return;/* nothing to do. */

	clif_favorite_item( *sd, index );
#endif
}


/// Items that are in favorite tab of inventory.
/// 0908 <index>.W <favorite>.B (ZC_INVENTORY_TAB)
static void clif_favorite_item( map_session_data& sd, uint16 index ){
// TODO: Check for correct packet version
#if PACKETVER >= 20111122
	PACKET_ZC_INVENTORY_TAB p = {};

	p.packetType = HEADER_ZC_INVENTORY_TAB;
	p.index = client_index( index );
	p.favorite = ( sd.inventory.u.items_inventory[index].favorite == 1 ) ? false : true;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}


/// 08d2 <id>.L <Pos X>.W <Pos Y>.W (ZC_FASTMOVE).
void clif_snap( struct block_list *bl, short x, short y ) {
	unsigned char buf[10];

	WBUFW(buf,0) = 0x8d2;
	WBUFL(buf,2) = bl->id;
	WBUFW(buf,6) = x;
	WBUFW(buf,8) = y;

	if( disguised(bl) )
	{
		clif_send(buf, packet_len(0x8d2), bl, AREA_WOS);
		WBUFL(buf,2) = disguised_bl_id(bl->id);
		clif_send(buf, packet_len(0x8d2), bl, SELF);
	} else
		clif_send(buf,packet_len(0x8d2),bl, AREA);
}

/// 0977 <id>.L <HP>.L <maxHP>.L (ZC_HP_INFO).
void clif_monster_hp_bar( struct mob_data* md, int fd ) {
#if PACKETVER >= 20120404
	WFIFOHEAD(fd,packet_len(0x977));

	WFIFOW(fd,0)  = 0x977;
	WFIFOL(fd,2)  = md->bl.id;
	WFIFOL(fd,6)  = md->status.hp;
	WFIFOL(fd,10) = md->status.max_hp;

	WFIFOSET(fd,packet_len(0x977));
#endif
}

/* [Ind] placeholder for unsupported incoming packets (avoids server disconnecting client) */
void __attribute__ ((unused)) clif_parse_dull(int fd, map_session_data *sd) {
	return;
}

/// Ack world info (ZC_ACK_BEFORE_WORLD_INFO)
/// 0979 <world name>.24B <char name>.24B
void clif_ackworldinfo(map_session_data* sd) {
	int fd;

	nullpo_retv(sd);
	fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0x979));
	WFIFOW(fd,0)=0x979;
	//AID -> world name ?
	safestrncpy(WFIFOCP(fd,2), "" /* World name */, 24);
	safestrncpy(WFIFOCP(fd,26), sd->status.name, NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x979));
}

/// req world info (CZ_REQ_BEFORE_WORLD_INFO)
/// 0978 <AID>.L
void clif_parse_reqworldinfo(int fd,map_session_data *sd) {
	//uint32 aid = RFIFOL(fd,2); //should we trust client ?
	if(sd) 
		clif_ackworldinfo(sd);
}

static void clif_loadConfirm( map_session_data *sd ){
#if PACKETVER_MAIN_NUM >= 20190403 || PACKETVER_RE_NUM >= 20190320 || PACKETVER_ZERO_NUM >= 20190410
	nullpo_retv( sd );

	PACKET_ZC_NOTIFY_ACTORINIT p = {};

	p.packetType = HEADER_ZC_NOTIFY_ACTORINIT;

	clif_send( &p, sizeof(p), &sd->bl, SELF );

	clif_instance_info( *sd );
#endif
}

/// unknown usage (CZ_BLOCKING_PLAY_CANCEL)
/// 0447
void clif_parse_blocking_playcancel( int fd, map_session_data *sd ){
	clif_loadConfirm( sd );
	
	int32 mf = map_getmapflag(sd->bl.m, MF_SPECIALPOPUP);

	if (mf > 0) {
		clif_specialpopup(*sd, mf);
	}
}

/// req world info (CZ_CLIENT_VERSION)
/// 044A <version>.L
void clif_parse_client_version(int fd, map_session_data *sd) {
	//if(sd)
	;
}

/// Ranking list

/// ranking pointlist  { <name>.24B <point>.L }*10
void clif_sub_ranklist(unsigned char *buf,int idx,map_session_data* sd, enum e_rank rankingtype){
	struct fame_list* list;
	int i, size = MAX_FAME_LIST;

	switch(rankingtype) {
		case RANK_BLACKSMITH:	list = smith_fame_list; break;
		case RANK_ALCHEMIST:	list = chemist_fame_list; break;
		case RANK_TAEKWON:		list = taekwon_fame_list; break;
		// PK currently unsupported
		case RANK_KILLER:		list = nullptr; size = 0; break;
		default:
			ShowError( "clif_sub_ranklist: Unsupported ranking type '%d'. Please report this.\n", rankingtype );
			return;
	}

	//Packet size limits this list to 10 elements. [Skotlex]
	for (i = 0; i < min(10,size); i++) {
		if (list[i].id > 0) {
			const char* name;
			if (strcmp(list[i].name, "-") == 0 &&
				(name = map_charid2nick(list[i].id)) != nullptr)
			{
				safestrncpy(WBUFCP(buf,idx + NAME_LENGTH * i), name, NAME_LENGTH);
			} else {
				safestrncpy(WBUFCP(buf,idx + NAME_LENGTH * i), list[i].name, NAME_LENGTH);
			}
		} else {
			safestrncpy(WBUFCP(buf, idx + NAME_LENGTH * i), "None", NAME_LENGTH);
		}
		WBUFL(buf, idx+NAME_LENGTH*10 + i * 4) = list[i].fame; //points
	}

	for(;i < 10; i++) { //In case the MAX is less than 10.
		safestrncpy(WBUFCP(buf, idx + NAME_LENGTH * i), "Unavailable", NAME_LENGTH);
		WBUFL(buf, idx+NAME_LENGTH*10 + i * 4) = 0;
	}
}

/// Request for the blacksmith ranklist.
/// /blacksmith command sends this packet to the server.
/// 0217 (CZ_BLACKSMITH_RANK)
void clif_parse_Blacksmith( int fd, map_session_data *sd ){
	clif_ranklist(sd,RANK_BLACKSMITH);
}

/// Request for the alchemist ranklist.
/// /alchemist command sends this packet to the server.
/// 0218 (CZ_ALCHEMIST_RANK)
void clif_parse_Alchemist( int fd, map_session_data *sd ){
	clif_ranklist(sd,RANK_ALCHEMIST);
}

/// Request for the taekwon ranklist.
/// /taekwon command sends this packet to the server.
/// 0225 (CZ_TAEKWON_RANK)
void clif_parse_Taekwon( int fd, map_session_data *sd ){
	clif_ranklist(sd,RANK_TAEKWON);
}

/// Request for the killer ranklist.
/// /pk command sends this packet to the server.
/// 0237 (CZ_KILLER_RANK)
void clif_parse_RankingPk( int fd, map_session_data *sd ){
	clif_ranklist(sd,RANK_KILLER);
}

/// 0219 { <name>.24B }*10 { <point>.L }*10 (ZC_BLACKSMITH_RANK)
/// 021a { <name>.24B }*10 { <point>.L }*10 (ZC_ALCHEMIST_RANK)
/// 0226 { <name>.24B }*10 { <point>.L }*10 (ZC_TAEKWON_RANK)
/// 0238 { <name>.24B }*10 { <point>.L }*10 (ZC_KILLER_RANK)
/// 097d <RankingType>.W {<CharName>.24B <point>L}*10 <mypoint>L (ZC_ACK_RANKING)
void clif_ranklist(map_session_data *sd, int16 rankingType) {
	enum e_rank rank;

#if PACKETVER < 20130710
	unsigned char buf[MAX_FAME_LIST * sizeof(struct fame_list)+2];
	short cmd;

	switch( rankingType ){
		case RANK_BLACKSMITH:	cmd = 0x219; break;
		case RANK_ALCHEMIST:	cmd = 0x21a; break;
		case RANK_TAEKWON:		cmd = 0x226; break;
		case RANK_KILLER:		cmd = 0x238; break;
		default:
			ShowError( "clif_ranklist: Unsupported ranking type '%d'. Please report this.\n", rankingType );
			return;
	}

	// Can be safely casted here, since we validated it before
	rank = (enum e_rank)rankingType;

	WBUFW(buf,0) = cmd;
	clif_sub_ranklist(buf,2,sd,rank);

	clif_send(buf, packet_len(cmd), &sd->bl, SELF);
#else
	unsigned char buf[MAX_FAME_LIST * sizeof(struct fame_list)+8];
	int mypoint = 0;

	switch( rankingType ){
		case RANK_BLACKSMITH:
		case RANK_ALCHEMIST:
		case RANK_TAEKWON:
		case RANK_KILLER:
			break;
		default:
			ShowError( "clif_ranklist: Unsupported ranking type '%d'. Please report this.\n", rankingType );
			return;
	}

	// Can be safely casted here, since we validated it before
	rank = (enum e_rank)rankingType;

	WBUFW(buf,0) = 0x97d;
	WBUFW(buf,2) = rank;
	clif_sub_ranklist(buf,4,sd,rank);

	switch(sd->class_&MAPID_UPPERMASK){ //mypoint (checking if valid type)
		case MAPID_BLACKSMITH:
		case MAPID_ALCHEMIST:
		case MAPID_TAEKWON:
			mypoint = sd->status.fame;
	}
	WBUFL(buf,284) = mypoint; //mypoint
	clif_send(buf, 288, &sd->bl, SELF);
#endif
}

/*
 *  097c <type> (CZ_REQ_RANKING)
 * type
 *  0: /blacksmith
 *  1: /alchemist
 *  2: /taekwon
 *  3: /pk
 * */
void clif_parse_ranklist(int fd,map_session_data *sd) {
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	int16 rankingtype = RFIFOW(fd,info->pos[0]); //type

	clif_ranklist(sd,rankingtype);
}

/// Updates the fame rank points for the given ranking.
/// 021b <points>.L <total points>.L (ZC_BLACKSMITH_POINT)
/// 021c <points>.L <total points>.L (ZC_ALCHEMIST_POINT)
/// 0224 <points>.L <total points>.L (ZC_TAEKWON_POINT)
/// 097e <RankingType>.W <point>.L <TotalPoint>.L (ZC_UPDATE_RANKING_POINT)
void clif_update_rankingpoint(map_session_data &sd, int rankingtype, int point) {
	int fd = sd.fd;
#if PACKETVER < 20130710
	short cmd;
	switch(rankingtype){
		case RANK_BLACKSMITH:	cmd = 0x21b; break;
		case RANK_ALCHEMIST:	cmd = 0x21c; break;
		case RANK_TAEKWON:		cmd = 0x224; break;
		default:
			ShowError( "clif_update_rankingpoint: Unsupported ranking type '%d'. Please report this.\n", rankingtype );
			return;
	}

	WFIFOHEAD(fd,packet_len(cmd));
	WFIFOW(fd,0) = cmd;
	WFIFOL(fd,2) = point;
	WFIFOL(fd,6) = sd.status.fame;
	WFIFOSET(fd, packet_len(cmd));
#else
	WFIFOHEAD(fd,packet_len(0x97e));
	WFIFOW(fd,0) = 0x97e;
	WFIFOW(fd,2) = rankingtype;
	WFIFOL(fd,4) = point;
	WFIFOL(fd,8) = sd.status.fame;
	WFIFOSET(fd,packet_len(0x97e));
#endif
}

/**
 * Transmit personal information to player. (rates)
 * 0x08cb <packet len>.W <exp>.W <death>.W <drop>.W <DETAIL_EXP_INFO>7B (ZC_PERSONAL_INFOMATION)
 * <InfoType>.B <Exp>.W <Death>.W <Drop>.W (DETAIL_EXP_INFO 0x08cb)
 * 0x097b <packet len>.W <exp>.L <death>.L <drop>.L <DETAIL_EXP_INFO>13B (ZC_PERSONAL_INFOMATION2)
 * 0x0981 <packet len>.W <exp>.W <death>.W <drop>.W <activity rate>.W <DETAIL_EXP_INFO>13B (ZC_PERSONAL_INFOMATION_CHN)
 * <InfoType>.B <Exp>.L <Death>.L <Drop>.L (DETAIL_EXP_INFO 0x97b|0981)
 * FIXME!
 * - Find/decide for details of EXP, Drop, and Death penalty rates
 * - For now, we're assuming values for DETAIL_EXP_INFO are:
 *   0 - map adjustment (bexp mapflag), 1 - Premium/VIP adjustment, 2 - Server rate adjustment, 3 - None
*/
#ifdef VIP_ENABLE
void clif_display_pinfo( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20110627 || PACKETVER_RE_NUM >= 20110628 || defined(PACKETVER_ZERO)
	// TODO: Whoever wants to take the blame, fix this crappy logic below - and yes I know indentation is wrong, but CANT TOUCH THIS! *sings* [Lemongrass]
		int details_bexp[PINFO_MAX], details_drop[PINFO_MAX], details_penalty[PINFO_MAX];

		/**
		 * EXP
		 */
		//0:PCRoom
		details_bexp[PINFO_BASIC] = map_getmapflag( sd.bl.m, MF_BEXP );
		if (details_bexp[PINFO_BASIC] == 100 || !details_bexp[PINFO_BASIC])
			details_bexp[PINFO_BASIC] = 0;
		else {
			if (details_bexp[PINFO_BASIC] < 100) {
				details_bexp[PINFO_BASIC] = 100 - details_bexp[PINFO_BASIC];
				details_bexp[PINFO_BASIC] = 0 - details_bexp[PINFO_BASIC];
			} else
				details_bexp[PINFO_BASIC] = details_bexp[PINFO_BASIC] - 100;
		}

		//1:Premium
		if( pc_isvip( &sd ) ){
			details_bexp[PINFO_PREMIUM] = battle_config.vip_base_exp_increase * battle_config.base_exp_rate / 100;
			if (details_bexp[PINFO_PREMIUM] < 0)
				details_bexp[PINFO_PREMIUM] = 0 - details_bexp[PINFO_PREMIUM];
		} else
			details_bexp[PINFO_PREMIUM] = 0;

		//2:Server
		details_bexp[PINFO_SERVER] = battle_config.base_exp_rate;
		if (details_bexp[PINFO_SERVER] == 100)
			details_bexp[PINFO_SERVER] = 0;
		else {
			if (details_bexp[PINFO_SERVER] < 100) {
				details_bexp[PINFO_SERVER] = 100 - details_bexp[PINFO_SERVER];
				details_bexp[PINFO_SERVER] = 0 - details_bexp[PINFO_SERVER];
			} else
				details_bexp[PINFO_SERVER] = details_bexp[PINFO_SERVER] - 100;
		}

		//3:TPLUS
		details_bexp[PINFO_CAFE] = 0;

		/**
		 * Drop rate
		 */
		//0:PCRoom
		details_drop[PINFO_BASIC] = 0;

		//1:Premium
		if( pc_isvip( &sd ) ){
			details_drop[PINFO_PREMIUM] = (battle_config.vip_drop_increase * battle_config.item_rate_common) / 100;
			if (details_drop[PINFO_PREMIUM] < 0)
				details_drop[PINFO_PREMIUM] = 0 - details_drop[PINFO_PREMIUM];
		} else
			details_drop[PINFO_PREMIUM] = 0;

		//2:Server
		details_drop[PINFO_SERVER] = battle_config.item_rate_common;
		if (details_drop[PINFO_SERVER] == 100)
			details_drop[PINFO_SERVER] = 0;
		else {
			if (details_drop[PINFO_SERVER] < 100) {
				details_drop[PINFO_SERVER] = 100 - details_drop[PINFO_SERVER];
				details_drop[PINFO_SERVER] = 0 - details_drop[PINFO_SERVER];
			} else
				details_drop[PINFO_SERVER] = details_drop[PINFO_SERVER] - 100;
		}

		//3:TPLUS
		details_drop[PINFO_CAFE] = 0;

		/**
		 * Penalty rate
		 */
		//! FIXME: Current penalty system makes this announcement unable to give info on + or - rate
		//0:PCRoom
		details_penalty[PINFO_BASIC] = 0;

		//1:Premium
		if( pc_isvip( &sd ) ){
			details_penalty[PINFO_PREMIUM] = battle_config.vip_exp_penalty_base;
			if (details_penalty[PINFO_PREMIUM] == 100)
				details_penalty[PINFO_PREMIUM] = 0;
			else {
				if (details_penalty[PINFO_PREMIUM] < 100) {
					details_penalty[PINFO_PREMIUM] = 100 - details_penalty[PINFO_PREMIUM];
					details_penalty[PINFO_PREMIUM] = 0 - details_penalty[PINFO_PREMIUM];
				} else
					details_penalty[PINFO_PREMIUM] = details_penalty[PINFO_PREMIUM] - 100;
			}
			if (battle_config.death_penalty_base > battle_config.vip_exp_penalty_base)
				details_penalty[PINFO_PREMIUM] = battle_config.vip_exp_penalty_base - battle_config.death_penalty_base;
		} else
			details_penalty[PINFO_PREMIUM] = 0;

		//2:Server
		details_penalty[PINFO_SERVER] = battle_config.death_penalty_base;
		if (details_penalty[PINFO_SERVER] == 100)
			details_penalty[PINFO_SERVER] = 0;
		else {
			if (details_penalty[PINFO_SERVER] < 100) {
				details_penalty[PINFO_SERVER] = 100 - details_penalty[PINFO_SERVER];
				details_penalty[PINFO_SERVER] = 0 - details_penalty[PINFO_SERVER];
			} else
				details_penalty[PINFO_SERVER] = details_penalty[PINFO_SERVER] - 100;
		}

		//3:TPLUS
		details_penalty[PINFO_CAFE] = 0;

	PACKET_ZC_PERSONAL_INFOMATION* p = reinterpret_cast<PACKET_ZC_PERSONAL_INFOMATION*>( packet_buffer );

	p->packetType = HEADER_ZC_PERSONAL_INFOMATION;
	p->length = sizeof( *p );
#if PACKETVER_MAIN_NUM >= 20120503 || PACKETVER_RE_NUM >= 20120502 || defined(PACKETVER_ZERO)
	p->total_exp = 100 * 1000;
	p->total_death = 100 * 1000;
	p->total_drop = 100 * 1000;
#else
	p->total_exp = 100;
	p->total_death = 100;
	p->total_drop = 100;
#endif

	for( int i = PINFO_BASIC; i < PINFO_MAX; i++ ){
		p->details[i].type = i; // infotype 0 PCRoom, 1 Premium, 2 Server, 3 TPlus

#if PACKETVER_MAIN_NUM >= 20120503 || PACKETVER_RE_NUM >= 20120502 || defined(PACKETVER_ZERO)
		p->details[i].exp = details_bexp[i] * 1000;
		p->details[i].death = details_penalty[i] * 1000;
		p->details[i].drop = details_drop[i] * 1000;
#else
		p->details[i].exp = details_bexp[i];
		p->details[i].death = details_penalty[i];
		p->details[i].drop = details_drop[i];
#endif

		p->total_exp += p->details[i].exp;
		p->total_death += p->details[i].death;
		p->total_drop += p->details[i].drop;

		p->length += static_cast<decltype(p->length)>( sizeof( p->details[0] ) );
	}

	clif_send( p, p->length, &sd.bl, SELF );
#endif
}
#endif

void clif_parse_GMFullStrip(int fd, map_session_data *sd) {
	char cmd[30];
	int t_aid = RFIFOL(fd,2);
	safesnprintf(cmd,sizeof(cmd),"%cfullstrip %d",atcommand_symbol,t_aid);
	is_atcommand(fd, sd, cmd, 1);
}

/**
* Marks Crimson Marker target on mini-map to the caster
* 09C1 <id>.L <x>.W <y>.W (ZC_C_MARKERINFO)
* @param fd
* @param bl Crimson Marker target
**/
void clif_crimson_marker( map_session_data& sd, struct block_list& bl, bool remove ){
#if PACKETVER_MAIN_NUM >= 20130731 || PACKETVER_RE_NUM >= 20130707 || defined(PACKETVER_ZERO)
	PACKET_ZC_C_MARKERINFO p = {};

	p.PacketType = HEADER_ZC_C_MARKERINFO;
	p.AID = bl.id;
	if( remove ){
		p.xPos = -1;
		p.yPos = -1;
	}else{
		p.xPos = bl.x;
		p.yPos = bl.y;
	}

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/**
 * 02d3 <index>.W (ZC_NOTIFY_BIND_ON_EQUIP)
 **/
void clif_notify_bindOnEquip( map_session_data& sd, int16 index ){
#if PACKETVER >= 20070227
	PACKET_ZC_NOTIFY_BIND_ON_EQUIP p = {};

	p.packetType = HEADER_ZC_NOTIFY_BIND_ON_EQUIP;
	p.index = client_index( index );

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/**
* [Ind/Hercules]
* 08b3 <Length>.W <id>.L <message>.?B (ZC_SHOWSCRIPT)
**/
void clif_showscript(struct block_list* bl, const char* message, enum send_target flag) {
	char buf[256];
	size_t len;
	nullpo_retv(bl);

	if(!message)
		return;

	len = strlen(message)+1;

	if( len > sizeof(buf)-8 ) {
		ShowWarning("clif_showscript: Truncating too long message '%s' (len=%" PRIuPTR ").\n", message, len);
		len = sizeof(buf)-8;
	}

	WBUFW(buf,0) = 0x8b3;
	WBUFW(buf,2) = (uint16)(len+8);
	WBUFL(buf,4) = bl->id;
	safestrncpy(WBUFCP(buf,8), message, len);
	clif_send((unsigned char *) buf, WBUFW(buf,2), bl, flag);
}

/**
* [Ind/Hercules]
* 07fc <OldMasterAID>.L <NewMasterAID>.L (ZC_CHANGE_GROUP_MASTER)
**/
void clif_party_leaderchanged(map_session_data *sd, int prev_leader_aid, int new_leader_aid) {
	unsigned char buf[10];

	nullpo_retv(sd);

	WBUFW(buf,0) = 0x7fc;
	WBUFL(buf,2) = prev_leader_aid;
	WBUFL(buf,6) = new_leader_aid;
	clif_send(buf, packet_len(0x7fc), &sd->bl, PARTY);
}

/**
* Sends a clan message to a player
* 098e <length>.W <name>.24B <message>.?B (ZC_NOTIFY_CLAN_CHAT)
**/
void clif_clan_message( struct clan& clan, const char *mes, size_t len ){
#if PACKETVER >= 20131223
	map_session_data* sd = clan_getavailablesd( clan );

	if( len == 0 ){
		return;
	}

	PACKET_ZC_NOTIFY_CLAN_CHAT* p = reinterpret_cast<PACKET_ZC_NOTIFY_CLAN_CHAT*>( packet_buffer );

	// Maximum message length = size of our packet buffer minus fixed size of the packet and 1 byte zero termination
	static size_t len_max = sizeof( packet_buffer ) - sizeof( *p ) - 1;

	// Is the length bigger than the maximum message length
	if( len > len_max ){
		ShowWarning( "clif_clan_message: Truncated message '%s' (len=%" PRIuPTR ", max=%" PRIuPTR ", clan_id=%d).\n", mes, len, len_max, clan.id );
		len = len_max;
	}

	p->PacketType = HEADER_ZC_NOTIFY_CLAN_CHAT;
	p->PacketLength = sizeof( *p );
	
	// Offially the sender name should also be filled here, but it is not required by the client and since it's in the message too we do not fill it
	safestrncpy( p->MemberName, "", sizeof( p->MemberName ) );

	safestrncpy( p->Message, mes, len + 1 );
	p->PacketLength += static_cast<decltype(p->PacketLength)>( len + 1 );

	clif_send( p, p->PacketLength, &sd->bl, CLAN );
#endif
}

/**
* Parses a clan message from a player.
* 098d <length>.W <text>.?B (<name> : <message>) (CZ_CLAN_CHAT)
**/
void clif_parse_clan_chat( int fd, map_session_data* sd ){
#if PACKETVER >= 20131223
	if( sd == nullptr ){
		return;
	}

	char name[NAME_LENGTH], message[CHAT_SIZE_MAX], output[CHAT_SIZE_MAX+NAME_LENGTH*2];

	// validate packet and retrieve name and message
	if( !clif_process_message(sd, false, name, message, output ) )
		return;

	clan_send_message( *sd, RFIFOCP(fd,4), RFIFOW(fd,2) - 4 );
#endif
}

/**
* Sends the basic clan informations to the client.
* 098a <length>.W <clan id>.L <clan name>.24B <clan master>.24B <clan map>.16B <alliance count>.B
*      <antagonist count>.B { <alliance>.24B } * alliance count { <antagonist>.24B } * antagonist count (ZC_CLANINFO)
**/
void clif_clan_basicinfo( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20130626 || PACKETVER_RE_NUM >= 20130605 || defined(PACKETVER_ZERO)
	struct clan* clan = sd.clan;

	if( clan == nullptr ){
		return;
	}

	PACKET_ZC_CLANINFO* p = reinterpret_cast<PACKET_ZC_CLANINFO*>( packet_buffer );

	p->PacketType = HEADER_ZC_CLANINFO;
	p->PacketLength = sizeof( *p );
	p->ClanID = clan->id;
	safestrncpy( p->ClanName, clan->name, sizeof( p->ClanName ) );
	safestrncpy( p->MasterName, clan->master, sizeof( p->MasterName ) );
	mapindex_getmapname_ext( clan->map, p->Map );
	p->AllyCount = clan_get_alliance_count( *clan, 0 );
	p->AntagonistCount = clan_get_alliance_count( *clan, 1 );

	for( int flag = 0; flag < 2; flag++ ){
		for( int i = 0; i < MAX_CLANALLIANCE; i++ ){
			if( clan->alliance[i].clan_id > 0 && clan->alliance[i].opposition == flag ){
				char* name = reinterpret_cast<char*>( WBUFP( p, p->PacketLength ) );

				safestrncpy( name, clan->alliance[i].name, NAME_LENGTH );

				p->PacketLength += static_cast<decltype(p->PacketLength)>( NAME_LENGTH );
			}
		}
	}

	clif_send( p, p->PacketLength, &sd.bl, SELF );
#endif
}

/**
* Updates the online and maximum player count of a clan.
* 0988 <online count>.W <maximum member amount>.W (ZC_NOTIFY_CLAN_CONNECTINFO)
**/
void clif_clan_onlinecount( struct clan& clan ){
#if PACKETVER >= 20131223
	map_session_data* sd = clan_getavailablesd( clan );

	if( sd == nullptr){
		return;
	}

	PACKET_ZC_NOTIFY_CLAN_CONNECTINFO p = {};

	p.PacketType = HEADER_ZC_NOTIFY_CLAN_CONNECTINFO;
	p.NumConnect = clan.connect_member;
	p.NumTotal = clan.max_member;

	clif_send( &p, sizeof( p ), &sd->bl, CLAN );
#endif
}

/**
* Notifies the client that the player has left his clan.
* 0989 (ZC_ACK_CLAN_LEAVE)
**/
void clif_clan_leave( map_session_data& sd ){
#if PACKETVER >= 20131223
	PACKET_ZC_ACK_CLAN_LEAVE p = {};

	p.PacketType = HEADER_ZC_ACK_CLAN_LEAVE;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/**
 * Acknowledge the client about change title result (ZC_ACK_CHANGE_TITLE).
 * 0A2F <result>.B <title_id>.L
 */
void clif_change_title_ack(map_session_data *sd, unsigned char result, unsigned long title_id)
{
#if PACKETVER >= 20150513
	int fd;

	nullpo_retv(sd);

	if (!clif_session_isValid(sd))
		return;
	fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xa2f));
	WFIFOW(fd, 0) = 0xa2f;
	WFIFOB(fd, 2) = result;
	WFIFOL(fd, 3) = title_id;
	WFIFOSET(fd, packet_len(0xa2f));
#endif
}

/**
 * Parsing a request from the client change title (CZ_REQ_CHANGE_TITLE).
 * 0A2E <title_id>.L
 */
void clif_parse_change_title(int fd, map_session_data *sd)
{
	int title_id;

	nullpo_retv(sd);

	title_id = RFIFOL(fd, 2);

	if( title_id == sd->status.title_id ){
		// It is exactly the same as the old one
		return;
	}else if( title_id <= 0 ){
		sd->status.title_id = 0;
	}else{
		if (std::find(sd->titles.begin(), sd->titles.end(), title_id) == sd->titles.end()) {
			clif_change_title_ack(sd, 1, title_id);
			return;
		}

		sd->status.title_id = title_id;
	}
	
	clif_name_area(&sd->bl);
	clif_change_title_ack(sd, 0, title_id);
}

#ifdef DUMP_UNKNOWN_PACKET
void DumpUnknown(int fd,TBL_PC *sd,int cmd,int packet_len)
{
	const char* packet_txt = "save/packet.txt";
	FILE* fp;
	time_t time_server;
	struct tm *datetime;
	char datestr[512];

	time(&time_server);  // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(datestr, sizeof(datestr)-1, "%A, %B %d %Y %X.", datetime); // Server time (normal time): %A, %B %d %Y %X.


	if( ( fp = fopen( packet_txt , "a" ) ) != nullptr ) {
		if( sd ) {
			fprintf(fp, "Unknown packet 0x%04X (length %d), %s session #%d, %d/%d (AID/CID) at %s \n", cmd, packet_len, sd->state.active ? "authed" : "unauthed", fd, sd->status.account_id, sd->status.char_id,datestr);
		} else {
			fprintf(fp, "Unknown packet 0x%04X (length %d), session #%d at %s\n", cmd, packet_len, fd,datestr);
		}
		WriteDump(fp, RFIFOP(fd,0), packet_len);
		fprintf(fp, "\n");
		fclose(fp);
	} else {
		ShowError("Failed to write '%s'.\n", packet_txt);
		// Dump on console instead
		if( sd ) {
			ShowDebug("Unknown packet 0x%04X (length %d), %s session #%d, %d/%d (AID/CID) at %s\n", cmd, packet_len, sd->state.active ? "authed" : "unauthed", fd, sd->status.account_id, sd->status.char_id,datestr);
		} else {
			ShowDebug("Unknown packet 0x%04X (length %d), session #%d at %s\n", cmd, packet_len, fd,datestr);
		}

		ShowDump(RFIFOP(fd,0), packet_len);
	}
}
#endif

/// Roulette System
/// Author: Yommy

void roulette_generate_bonus( map_session_data& sd ){
	if( battle_config.feature_roulette_bonus_reward && sd.roulette.bonusItemID == 0 ){
		int next_stage = 0;

		if( sd.roulette_point.bronze > 0 ){
			next_stage = 0;
		}else if( sd.roulette_point.silver > 9 ){
			next_stage = 2;
		}else if( sd.roulette_point.gold > 9 ){
			next_stage = 4;
		}

		// Get bonus item stage only from current stage or higher
		int reward_stage = rnd_value( next_stage, MAX_ROULETTE_LEVEL - 1 );
		sd.roulette.bonusItemID = rd.nameid[reward_stage][rnd()%rd.items[reward_stage]];
	}else{
		sd.roulette.bonusItemID = 0;
	}
}

/// Opens the roulette window
/// 0A1A <result>.B <serial>.L <stage>.B <price index>.B <additional item id>.W <gold>.L <silver>.L <bronze>.L (ZC_ACK_OPEN_ROULETTE)
void clif_roulette_open( map_session_data* sd ){
	nullpo_retv( sd );

	roulette_generate_bonus( *sd );

	struct packet_roulette_open_ack p;

	p.PacketType = 0xa1a;
	p.Result = 0; // result
	p.Serial = 0; // serial
	p.Step = (sd->roulette.claimPrize) ? sd->roulette.stage - 1 : 0;
	p.Idx = (sd->roulette.claimPrize) ? sd->roulette.prizeIdx : -1;
	p.AdditionItemID = sd->roulette.bonusItemID;
	p.GoldPoint = sd->roulette_point.gold;
	p.SilverPoint = sd->roulette_point.silver;
	p.BronzePoint = sd->roulette_point.bronze;

	sd->state.roulette_open = true;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}

/// Request to open the roulette window
/// 0A19 (CZ_REQ_OPEN_ROULETTE)
void clif_parse_roulette_open( int fd, map_session_data* sd ){
	nullpo_retv(sd);

	if (!battle_config.feature_roulette) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1497),false,SELF); //Roulette is disabled
		return;
	}

	clif_roulette_open(sd);
}

/// Sends the info about the available roulette rewards to the client
/// 0A1C <length>.W <serial>.L { { <level>.W <column>.W <item>.W <amount>.W } * MAX_ROULETTE_COLUMNS } * MAX_ROULETTE_LEVEL (ZC_ACK_ROULEITTE_INFO)
/// 0A1C <length>.W <serial>.L { { <level>.W <column>.W <item>.L <amount>.L } * MAX_ROULETTE_COLUMNS } * MAX_ROULETTE_LEVEL (ZC_ACK_ROULEITTE_INFO) >= 20180516
void clif_roulette_info( map_session_data* sd ){
	nullpo_retv(sd);

	struct packet_roulette_info_ack p;

	p.PacketType = rouletteinfoackType;
	p.PacketLength = sizeof(p);
	p.RouletteSerial = 1;

	for( int i = 0, count = 0; i < MAX_ROULETTE_LEVEL; i++ ){
		for( int j = 0; j < MAX_ROULETTE_COLUMNS - i; j++ ){
			p.ItemInfo[count].Row = i;
			p.ItemInfo[count].Position = j;
			p.ItemInfo[count].ItemId = client_nameid( rd.nameid[i][j] );
			p.ItemInfo[count].Count = rd.qty[i][j];
#if PACKETVER >= 20180511
			p.ItemInfo[count].unused = 0;
#endif
			count++;
		}
	}

	clif_send( &p, sizeof(p), &sd->bl, SELF );
}

/// Request the roulette reward data
/// 0A1B (CZ_REQ_ROULETTE_INFO)
void clif_parse_roulette_info( int fd, map_session_data* sd ){
	nullpo_retv(sd);

	if (!battle_config.feature_roulette) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1497),false,SELF); //Roulette is disabled
		return;
	}

	clif_roulette_info(sd);
}

/// Notification of the client that the roulette window was closed
/// 0A1D (CZ_REQ_CLOSE_ROULETTE)
void clif_parse_roulette_close( int fd, map_session_data* sd ){
	nullpo_retv(sd);

	if (!battle_config.feature_roulette) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1497),false,SELF); //Roulette is disabled
		return;
	}

	sd->state.roulette_open = false;
}

/// Response to a item reward request
/// 0A22 <type>.B <bonus item>.W (ZC_RECV_ROULETTE_ITEM)
static void clif_roulette_recvitem_ack(map_session_data *sd, enum RECV_ROULETTE_ITEM_REQ type) {
#if PACKETVER >= 20141016
	nullpo_retv(sd);

	struct packet_roulette_itemrecv_ack p;

	p.PacketType = roulettercvitemackType;
	p.Result = type;
	p.AdditionItemID = sd->roulette.bonusItemID;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

/**
 * Claim roulette reward
 * @param sd Player
 * @return 0:Success
 **/
static uint8 clif_roulette_getitem(map_session_data *sd) {
	struct item it;
	uint8 res = 1;
	unsigned short factor = 1;

	nullpo_retr(1, sd);

	if (sd->roulette.prizeIdx < 0)
		return RECV_ITEM_FAILED;

	memset(&it, 0, sizeof(it));

	it.nameid = rd.nameid[sd->roulette.prizeStage][sd->roulette.prizeIdx];
	it.identify = 1;

	if( sd->roulette.bonusItemID != 0 ){
		if( sd->roulette.bonusItemID == it.nameid && battle_config.feature_roulette_bonus_reward && !( rd.flag[sd->roulette.prizeStage][sd->roulette.prizeIdx]&1 ) ){
			factor = 2;
			// Reset the Bonus Item to trigger new calculation
			sd->roulette.bonusItemID = 0;
		}else{
			for( int i = 0; i < MAX_ROULETTE_LEVEL && sd->roulette.bonusItemID != 0; i++ ){
				for( int j = 0; j < MAX_ROULETTE_COLUMNS - i && sd->roulette.bonusItemID != 0; j++ ){
					if( rd.nameid[i][j] == sd->roulette.bonusItemID ){
						// If you reached a equal or higher level than the first level with the bonus item in it, you missed your chance and a new bonus item will be calculated
						if( sd->roulette.prizeStage >= i ){
							// Reset the Bonus Item to trigger new calculation and to cancel the loops
							sd->roulette.bonusItemID = 0;
						}
					}
				}
			}
		}
	}

	if ((res = pc_additem(sd, &it, rd.qty[sd->roulette.prizeStage][sd->roulette.prizeIdx] * factor, LOG_TYPE_ROULETTE)) == 0) {
		; // onSuccess
	}

	if( sd->roulette.bonusItemID == 0 ){
		roulette_generate_bonus( *sd );
	}

	sd->roulette.claimPrize = false;
	sd->roulette.prizeStage = 0;
	sd->roulette.prizeIdx = -1;
	sd->roulette.stage = 0;
	return res;
}

/// Update Roulette window with current stats
/// 0A20 <result>.B <stage>.W <price index>.W <bonus item>.W <gold>.L <silver>.L <bronze>.L (ZC_ACK_GENERATE_ROULETTE)
void clif_roulette_generate( map_session_data *sd, unsigned char result, short stage, short prizeIdx, t_itemid bonusItemID ){
	nullpo_retv( sd );

	struct packet_roulette_generate_ack p;

	p.PacketType = roulettgenerateackType;
	p.Result = result;
	p.Step = stage;
	p.Idx = prizeIdx;
	p.AdditionItemID = battle_config.feature_roulette_bonus_reward ? bonusItemID : 0;
	p.RemainGold = sd->roulette_point.gold;
	p.RemainSilver = sd->roulette_point.silver;
	p.RemainBronze = sd->roulette_point.bronze;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
}

/// Request to start the roulette
/// 0A1F (CZ_REQ_GENERATE_ROULETTE)
void clif_parse_roulette_generate( int fd, map_session_data* sd ){
	enum GENERATE_ROULETTE_ACK result;

	nullpo_retv(sd);

	if (!battle_config.feature_roulette) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1497),false,SELF); //Roulette is disabled
		return;
	}

	if( sd->roulette.tick && DIFF_TICK( sd->roulette.tick, gettick() ) > 0 ){
		return;
	}

	// Player has not claimed his prize yet
	if( sd->roulette.claimPrize ){
		clif_roulette_getitem( sd );
	}

	if (sd->roulette.stage >= MAX_ROULETTE_LEVEL){
		// Make sure everything is reset
		sd->roulette.stage = 0;
		sd->roulette.claimPrize = false;
		sd->roulette.prizeStage = 0;
		sd->roulette.prizeIdx = -1;
	}

	if( !sd->roulette.stage && sd->roulette_point.bronze <= 0 && sd->roulette_point.silver < 10 && sd->roulette_point.gold < 10 ){
		result = GENERATE_ROULETTE_NO_ENOUGH_POINT;
	}else{
		if (!sd->roulette.stage) {
			if (sd->roulette_point.bronze > 0) {
				sd->roulette_point.bronze -= 1;
				pc_setreg2(sd, ROULETTE_BRONZE_VAR, sd->roulette_point.bronze);
			} else if (sd->roulette_point.silver > 9) {
				sd->roulette_point.silver -= 10;
				sd->roulette.stage = 2;
				pc_setreg2(sd, ROULETTE_SILVER_VAR, sd->roulette_point.silver);
			} else if (sd->roulette_point.gold > 9) {
				sd->roulette_point.gold -= 10;
				sd->roulette.stage = 4;
				pc_setreg2(sd, ROULETTE_GOLD_VAR, sd->roulette_point.gold);
			}
		}

		sd->roulette.prizeStage = sd->roulette.stage;
		sd->roulette.prizeIdx = rnd()%rd.items[sd->roulette.stage];
		sd->roulette.claimPrize = true;
		sd->roulette.tick = gettick() + i64max( 1, ( MAX_ROULETTE_COLUMNS - sd->roulette.prizeStage - 3 ) ) * 1000;

		if( rd.flag[sd->roulette.stage][sd->roulette.prizeIdx]&1 ){
			result = GENERATE_ROULETTE_LOSING;
			sd->roulette.stage = 0;
		}else{
			result = GENERATE_ROULETTE_SUCCESS;
			sd->roulette.stage++;
		}
	}

	clif_roulette_generate(sd,result,sd->roulette.prizeStage,(sd->roulette.prizeIdx == -1 ? 0 : sd->roulette.prizeIdx), sd->roulette.bonusItemID);
}

/// Request to claim a prize
/// 0A21 (CZ_RECV_ROULETTE_ITEM)
void clif_parse_roulette_item( int fd, map_session_data* sd ){
	enum RECV_ROULETTE_ITEM_REQ type = RECV_ITEM_FAILED;
	nullpo_retv(sd);

	if (!battle_config.feature_roulette) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1497),false,SELF); //Roulette is disabled
		return;
	}

	if( sd->roulette.tick && DIFF_TICK( sd->roulette.tick, gettick() ) > 0 ){
		return;
	}

	if (sd->roulette.claimPrize && sd->roulette.prizeIdx != -1) {
		switch (clif_roulette_getitem(sd)) {
			case ADDITEM_SUCCESS:
				type = RECV_ITEM_SUCCESS;
				break;
			case ADDITEM_INVALID:
			case ADDITEM_OVERITEM:
			case ADDITEM_OVERAMOUNT:
				type = RECV_ITEM_OVERCOUNT;
				break;
			case ADDITEM_OVERWEIGHT:
				type = RECV_ITEM_OVERWEIGHT;
				break;
			case ADDITEM_STACKLIMIT:
			default:
				type = RECV_ITEM_FAILED;
				break;
		}
	}

	clif_roulette_recvitem_ack(sd,type);
}

/// MERGE ITEM

/**
 * Acknowledge the client about item merger result
 * ZC 096F <index>.W <total>.W <result>.B (ZC_ACK_MERGE_ITEM)
 * @param fd
 * @param sd
 **/
void clif_merge_item_ack( map_session_data &sd, enum MERGE_ITEM_ACK type, uint16 index = 0, uint16 count = 0 ){
#if PACKETVER_MAIN_NUM >= 20120314 || PACKETVER_RE_NUM >= 20120221 || defined(PACKETVER_ZERO)
	PACKET_ZC_ACK_MERGE_ITEM p = {};

	p.packetType = HEADER_ZC_ACK_MERGE_ITEM;
	if( type == MERGE_ITEM_SUCCESS ){
		p.index = client_index( index );
	}else{
		p.index = index;
	}
	p.amount = count;
	p.reason = type;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/**
 * Check if item has the pair to be merged
 * @param sd
 * @param it Item
 * @return True if has pair, False if not
 **/
static bool clif_merge_item_has_pair(map_session_data *sd, struct item *it) {
	struct item *it_;
	unsigned short i;

	nullpo_retr(false, sd);

	ARR_FIND(0, MAX_INVENTORY, i, (it_ = &sd->inventory.u.items_inventory[i]) && it->nameid == it_->nameid && it->bound == it_->bound && memcmp(it_, it, sizeof(struct item)) != 0);
	if (i < MAX_INVENTORY)
		return true;

	return false;
}

/**
 * Check if item can be merged
 * @param id Item Data
 * @param it Item
 * @return True if can be merged, False if not
 **/
static bool clif_merge_item_check(struct item_data *id, struct item *it) {
	if (!id || !it)
		return false;
	if (id->type == IT_CASH)
		return false;
	if (!itemdb_isstackable2(id))
		return false;
	if (itemdb_isspecial(it->card[0]))
		return false;
	if (!it->unique_id)
		return false;
	return true;
}

/**
 * Open available item to be merged.
 * Only show 1 item in 1 process.
 * ZC 096D <size>.W { <index>.W }* (ZC_MERGE_ITEM_OPEN)
 * @param sd
 **/
void clif_merge_item_open( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20120314 || PACKETVER_RE_NUM >= 20120221 || defined(PACKETVER_ZERO)
	PACKET_ZC_MERGE_ITEM_OPEN* p = reinterpret_cast<PACKET_ZC_MERGE_ITEM_OPEN*>( packet_buffer );

	p->packetType = HEADER_ZC_MERGE_ITEM_OPEN;
	p->packetLen = sizeof( *p );

	int n = 0;

	// Get entries
	for( int i = 0; i < MAX_INVENTORY; i++ ){
		struct item* it = &sd.inventory.u.items_inventory[i];

		if( !clif_merge_item_check( sd.inventory_data[i], it ) ){
			continue;
		}

		if( clif_merge_item_has_pair( &sd, it ) ){
			p->items[n++].index = client_index( i );
			p->packetLen += static_cast<decltype(p->packetLen)>( sizeof( p->items[0] ) );
		}
	}

	// No item need to be merged
	if( n < 2 ){
		clif_msg( &sd, MSI_NOT_EXIST_MERGE_ITEM );
		return;
	}

	clif_send( p, p->packetLen, &sd.bl, SELF );
#endif
}

/**
 * Process item merge request.
 * 096E <size>.W { <index>.W }* (CZ_REQ_MERGE_ITEM)
 * @param fd
 * @param sd
 **/
void clif_parse_merge_item_req( int fd, map_session_data* sd ){
	const PACKET_CZ_REQ_MERGE_ITEM* p = reinterpret_cast<PACKET_CZ_REQ_MERGE_ITEM*>( RFIFOP( fd, 0 ) );

	int count = ( p->packetLength - sizeof( *p ) ) / sizeof( p->indices[0] );

	// No item need to be merged
	if( count < 2 ){
		clif_msg( sd, MSI_NOT_EXIST_MERGE_ITEM );
		return;
	}

	uint16 idx_main = server_index( p->indices[0] );

	if( idx_main >= MAX_INVENTORY ){
		return;
	}

	if( !clif_merge_item_check( sd->inventory_data[idx_main], &sd->inventory.u.items_inventory[idx_main] ) ){
		clif_msg( sd, MSI_NOT_EXIST_MERGE_ITEM );
		return;
	}

	// Ensure each index only comes once
	std::unordered_set<uint16> indices;

	for( int i = 1; i < count; i++ ){
		uint16 idx = server_index( p->indices[i] );

		if( idx >= MAX_INVENTORY ){
			return;
		}

		if( sd->inventory_data[idx] == nullptr ){
			return;
		}

		// Check if it is the same item
		if( sd->inventory_data[idx]->nameid != sd->inventory_data[idx_main]->nameid ){
			return;
		}

		if( !clif_merge_item_check( sd->inventory_data[idx], &sd->inventory.u.items_inventory[idx] ) ){
			clif_msg( sd, MSI_NOT_EXIST_MERGE_ITEM );
			return;
		}

		indices.insert( idx );
	}

	if( indices.empty() ){
		clif_msg( sd, MSI_NOT_EXIST_MERGE_ITEM );
		return;
	}

	uint32 total_amount = sd->inventory.u.items_inventory[idx_main].amount;

	for( uint16 idx : indices ){
		total_amount += sd->inventory.u.items_inventory[idx].amount;
	}

	uint16 stack = sd->inventory_data[idx_main]->stack.amount;

	if( stack == 0 ){
		stack = MAX_AMOUNT;
	}

	if( total_amount >= stack ){
		clif_merge_item_ack( *sd, MERGE_ITEM_FAILED_MAX_COUNT );
		return;
	}

	// Merrrrge!!!!
	for( uint16 idx : indices ){
		uint16 amount = sd->inventory.u.items_inventory[idx].amount;

		log_pick_pc( sd, LOG_TYPE_MERGE_ITEM, -amount, &sd->inventory.u.items_inventory[idx] );
		memset( &sd->inventory.u.items_inventory[idx], 0, sizeof( sd->inventory.u.items_inventory[0] ) );
		sd->inventory_data[idx] = nullptr;
		clif_delitem( *sd, idx, amount, 0 );
	}

	sd->inventory.u.items_inventory[idx_main].amount = total_amount;

	clif_merge_item_ack( *sd, MERGE_ITEM_SUCCESS, idx_main, total_amount );
}

/**
 * Cancel item merge
 * CZ 0974 (CZ_CANCEL_MERGE_ITEM)
 * @param fd
 * @param sd
 **/
void clif_parse_merge_item_cancel(int fd, map_session_data* sd) {
	return; // Nothing todo yet
}

static std::string clif_hide_name(const char* original_name)
{
	std::string censored(original_name);
	size_t hide = zmin(battle_config.broadcast_hide_name, censored.length() - 1);
	censored.replace(censored.length() - hide, hide, hide, '*');
	return censored;
}

/**
 * 07fd <size>.W <type>.B <itemid>.W <charname_len>.B <charname>.24B <source_len>.B <containerid>.W (ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN)
 * 07fd <size>.W <type>.B <itemid>.W <charname_len>.B <charname>.24B <source_len>.B <srcname>.24B (ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN)
 * type: ITEMOBTAIN_TYPE_BOXITEM & ITEMOBTAIN_TYPE_MONSTER_ITEM "[playername] ... [sourcename] ... [itemname]" -> MsgStringTable[1629]
 * type: ITEMOBTAIN_TYPE_NPC "[playername] ... [itemname]" -> MsgStringTable[1870]
 **/
void clif_broadcast_obtain_special_item( const char *char_name, t_itemid nameid, t_itemid container, enum BROADCASTING_SPECIAL_ITEM_OBTAIN type ){
	char name[NAME_LENGTH];

	if( battle_config.broadcast_hide_name ){
		std::string dispname = clif_hide_name(char_name);
		safestrncpy(name, dispname.c_str(), sizeof(name));
	}else{
		safestrncpy( name, char_name, sizeof( name ) );
	}

	switch( type ){
		case ITEMOBTAIN_TYPE_BOXITEM:
#if PACKETVER >= 20091201
			{
				PACKET_ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN_item p = {};

				p.PacketType = package_item_announceType;
				p.PacketLength = sizeof( p );
				p.type = type;
				p.ItemID = client_nameid( nameid );
				p.len = NAME_LENGTH;
				safestrncpy( p.Name, name, sizeof( p.Name ) );
				p.boxItemID_len = (int8)sizeof( p.BoxItemID );
				p.BoxItemID = client_nameid( container );
#if PACKETVER_MAIN_NUM >= 20220518 || PACKETVER_ZERO_NUM >= 20220518
				p.refineLevel_len = (int8)sizeof( p.refineLevel );
				p.refineLevel = 0; // TODO: implement
#endif

				clif_send( &p, p.PacketLength, nullptr, ALL_CLIENT );
			}
#endif
			break;

		case ITEMOBTAIN_TYPE_MONSTER_ITEM:
		case ITEMOBTAIN_TYPE_NPC:
			{
				struct packet_item_drop_announce p;

				p.PacketType = item_drop_announceType;
				p.type = type;
				p.ItemID = client_nameid( nameid );
				p.len = NAME_LENGTH;
				safestrncpy( p.Name, name, sizeof( p.Name ) );

				if( type == ITEMOBTAIN_TYPE_MONSTER_ITEM ){
					std::shared_ptr<s_mob_db> db = mob_db.find( container );

					p.monsterNameLen = NAME_LENGTH;
					safestrncpy( p.monsterName, db->name.c_str(), NAME_LENGTH );
				}else{
					p.monsterNameLen = 0;
					p.monsterName[0] = '\0';
				}

#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
				p.PacketLength = 12 + p.len + p.monsterNameLen;
#else
				p.PacketLength = 8 + p.len + p.monsterNameLen;
#endif

				clif_send( &p, p.PacketLength, nullptr, ALL_CLIENT );
			}
			break;
	}
}

/// Show body view windows (ZC_DRESSROOM_OPEN).
/// 0A02 <view>.W
/// Value <flag> has the following effects:
/// 1: Open a Dress Room window.
void clif_dressing_room(map_session_data *sd, int flag) {
#if PACKETVER >= 20150513
	int fd = sd->fd;

	nullpo_retv(sd);

	WFIFOHEAD(fd, packet_len(0xa02));
	WFIFOW(fd,0) = 0xa02;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd, packet_len(0xa02));
#endif
}

/// Parsing a request from the client item identify oneclick (CZ_REQ_ONECLICK_ITEMIDENTIFY).
/// 0A35 <result>.W
void clif_parse_Oneclick_Itemidentify(int fd, map_session_data *sd) {
#if PACKETVER >= 20150513
	short idx = RFIFOW(fd,packet_db[RFIFOW(fd,0)].pos[0]) - 2, magnifier_idx;

	// Ignore the request
	// - Invalid item index
	// - Invalid item ID or item doesn't exist
	// - Item is already identified
	if (idx < 0 || idx >= MAX_INVENTORY ||
		sd->inventory.u.items_inventory[idx].nameid == 0 || sd->inventory_data[idx] == nullptr ||
		sd->inventory.u.items_inventory[idx].identify)
			return;

	// Ignore the request - No magnifiers in inventory
	if ((magnifier_idx = pc_search_inventory(sd, ITEMID_MAGNIFIER)) == -1 &&
		(magnifier_idx = pc_search_inventory(sd, ITEMID_NOVICE_MAGNIFIER)) == -1)
		return;

	if (pc_delitem(sd, magnifier_idx, 1, 0, 0, LOG_TYPE_OTHER) != 0) // Deleting of magnifier failed, for whatever reason...
		return;

    skill_identify(sd, idx);
#endif
}

/// Starts navigation to the given target on client side
void clif_navigateTo(map_session_data *sd, const char* mapname, uint16 x, uint16 y, uint8 flag, bool hideWindow, uint16 mob_id ){
#if PACKETVER >= 20111010
	int fd = sd->fd;

	WFIFOHEAD(fd,27);
	WFIFOW(fd,0) = 0x08e2;

	// How detailed will our navigation be?
	if( mob_id > 0 ){
		x = 0;
		y = 0;
		WFIFOB(fd,2) = 3; // monster with destination field
	}else if( x > 0 && y > 0 ){
		WFIFOB(fd,2) = 0; // with coordinates
	}else{
		x = 0;
		y = 0;
		WFIFOB(fd,2) = 1; // without coordinates(will fail if you are already on the map)
	}

	// Which services can be used for transportation?
	WFIFOB(fd,3) = flag;
	// If this flag is set, the navigation window will not be opened up
	WFIFOB(fd,4) = hideWindow;
	// Target map
	safestrncpy( WFIFOCP(fd,5),mapname,MAP_NAME_LENGTH_EXT);
	// Target x
	WFIFOW(fd,21) = x;
	// Target y
	WFIFOW(fd,23) = y;
	// Target monster ID
	WFIFOW(fd,25) = mob_id;
	WFIFOSET(fd,27);
#endif
}

/// Send hat effects to the client.
/// 0A3B <Length>.W <AID>.L <Status>.B { <HatEffectId>.W } (ZC_EQUIPMENT_EFFECT)
void clif_hat_effects( map_session_data& sd, block_list& bl, enum send_target target ){
#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
	map_session_data *tsd;
	block_list* tbl;

	if( target == SELF ){
		tsd = BL_CAST(BL_PC,&bl);
		tbl = &sd.bl;
	}else{
		tsd = &sd;
		tbl = &bl;
	}

	if( tsd == nullptr ){
		return;
	}

	if( tsd->hatEffects.empty() || map_getmapdata(tbl->m)->getMapFlag(MF_NOCOSTUME) ){
		return;
	}

	PACKET_ZC_EQUIPMENT_EFFECT* p = reinterpret_cast<PACKET_ZC_EQUIPMENT_EFFECT*>( packet_buffer );

	p->packetType = HEADER_ZC_EQUIPMENT_EFFECT;
	p->packetLength = sizeof( *p );
	p->aid = tsd->bl.id;
	p->status = 1;

	for( size_t i = 0; i < tsd->hatEffects.size(); i++ ){
		p->effects[i] = tsd->hatEffects[i];

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->effects[0] ) );
	}

	clif_send( p, p->packetLength, tbl, target );
#endif
}

/// Send a single hat effect to the client.
/// 0A3B <Length>.W <AID>.L <Status>.B { <HatEffectId>.W } (ZC_EQUIPMENT_EFFECT)
void clif_hat_effect_single( map_session_data& sd, uint16 effectId, bool enable ){
#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
	PACKET_ZC_EQUIPMENT_EFFECT* p = reinterpret_cast<PACKET_ZC_EQUIPMENT_EFFECT*>( packet_buffer );

	p->packetType = HEADER_ZC_EQUIPMENT_EFFECT;
	p->packetLength = sizeof( *p );
	p->aid = sd.bl.id;
	p->status = enable;
	p->effects[0] = effectId;
	p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( p->effects[0] ) );

	clif_send( p, p->packetLength, &sd.bl, AREA );
#endif
}


/// Notify the client that a sale has started
/// 09b2 <item id>.W <remaining time>.L (ZC_NOTIFY_BARGAIN_SALE_SELLING)
void clif_sale_start( struct sale_item_data* sale_item, struct block_list* bl, enum send_target target ){
#if PACKETVER_SUPPORTS_SALES
	PACKET_ZC_NOTIFY_BARGAIN_SALE_SELLING p = {};

	p.packetType = HEADER_ZC_NOTIFY_BARGAIN_SALE_SELLING;
	p.itemId = client_nameid( sale_item->nameid );
	p.remainingTime = (uint32)(sale_item->end - time(nullptr)); // time in S

	clif_send( &p, sizeof( p ), bl, target );
#endif
}

/// Notify the client that a sale has ended
/// 09b3 <item id>.W (ZC_NOTIFY_BARGAIN_SALE_CLOSE)
void clif_sale_end( struct sale_item_data* sale_item, struct block_list* bl, enum send_target target ){
#if PACKETVER_SUPPORTS_SALES
	PACKET_ZC_NOTIFY_BARGAIN_SALE_CLOSE p = {};

	p.packetType = HEADER_ZC_NOTIFY_BARGAIN_SALE_CLOSE;
	p.itemId = client_nameid( sale_item->nameid );

	clif_send( &p, sizeof( p ), bl, target );
#endif
}

/// Update the remaining amount of a sale item.
/// 09c4 <item id>.W <amount>.L (ZC_ACK_COUNT_BARGAIN_SALE_ITEM)
void clif_sale_amount( struct sale_item_data* sale_item, struct block_list* bl, enum send_target target ){
#if PACKETVER_SUPPORTS_SALES
	PACKET_ZC_ACK_COUNT_BARGAIN_SALE_ITEM p = {};

	p.packetType = HEADER_ZC_ACK_COUNT_BARGAIN_SALE_ITEM;
	p.itemId = client_nameid( sale_item->nameid );
	p.amount = sale_item->amount;

	clif_send( &p, sizeof( p ), bl, target );
#endif
}

/// The client requested a refresh of the current remaining count of a sale item
/// 09ac <account id>.L <item id>.W (CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO)
void clif_parse_sale_refresh( int fd, map_session_data* sd ){
#if PACKETVER_SUPPORTS_SALES
	const PACKET_CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO* p = reinterpret_cast<PACKET_CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO*>( RFIFOP( fd, 0 ) );

	struct sale_item_data* sale;

	if( p->AID != sd->status.account_id ){
		return;
	}

	sale = sale_find_item( p->itemId, true );

	if( sale == nullptr ){
		return;
	}

	clif_sale_amount(sale, &sd->bl, SELF);
#endif
}

/// Opens the sale administration window on the client
/// 09b5 (ZC_OPEN_BARGAIN_SALE_TOOL)
void clif_sale_open( map_session_data* sd ){
#if PACKETVER_SUPPORTS_SALES
	nullpo_retv(sd);

	if( sd->state.sale_open ){
		return;
	}

	sd->state.sale_open = true;

	int fd = sd->fd;

	WFIFOHEAD(fd, 2);
	WFIFOW(fd, 0) = 0x9b5;
	WFIFOSET(fd, 2);
#endif
}

/// Client request to open the sale administration window.
/// This is sent by /limitedsale
/// 09b4 <account id>.L (CZ_OPEN_BARGAIN_SALE_TOOL)
void clif_parse_sale_open( int fd, map_session_data* sd ){
#if PACKETVER_SUPPORTS_SALES
	nullpo_retv(sd);

	if( RFIFOL(fd, 2) != sd->status.account_id ){
		return;
	}

	char command[CHAT_SIZE_MAX];

	safesnprintf( command, sizeof(command), "%climitedsale", atcommand_symbol );

	is_atcommand(fd, sd, command, 1);
#endif
}

/// Closes the sale administration window on the client.
/// 09bd (ZC_CLOSE_BARGAIN_SALE_TOOL)
void clif_sale_close(map_session_data* sd) {
#if PACKETVER_SUPPORTS_SALES
	nullpo_retv(sd);

	if( !sd->state.sale_open ){
		return;
	}

	sd->state.sale_open = false;

	int fd = sd->fd;

	WFIFOHEAD(fd, 2);
	WFIFOW(fd, 0) = 0x9bd;
	WFIFOSET(fd, 2);
#endif
}

/// Client request to close the sale administration window.
/// 09bc (CZ_CLOSE_BARGAIN_SALE_TOOL)
void clif_parse_sale_close(int fd, map_session_data* sd) {
#if PACKETVER_SUPPORTS_SALES
	nullpo_retv(sd);

	if( RFIFOL(fd, 2) != sd->status.account_id ){
		return;
	}

	clif_sale_close(sd);
#endif
}

/// Reply to a item search request for item sale administration.
/// 09ad <result>.W <item id>.W <price>.L (ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO)
void clif_sale_search_reply( map_session_data* sd, std::shared_ptr<s_cash_item> item ){
#if PACKETVER_SUPPORTS_SALES
	PACKET_ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO p = {};

	p.packetType = HEADER_ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO;

	if( item != nullptr ){
		p.result = 0;
		p.itemId = client_nameid( item->nameid );
		p.price = item->price;
	}else{
		p.result = 1;
		p.itemId = 0;
		p.price = 0;
	}

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

/// Search request for an item sale administration.
/// 09ac <length>.W <account id>.L <item name>.?B (CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO)
void clif_parse_sale_search( int fd, map_session_data* sd ){
#if PACKETVER_SUPPORTS_SALES
	char item_name[ITEM_NAME_LENGTH];

	nullpo_retv(sd);

	if( RFIFOL(fd, 4) != sd->status.account_id ){
		return;
	}

	if( !sd->state.sale_open ){
		return;
	}

	safestrncpy( item_name, RFIFOCP(fd, 8), min(RFIFOW(fd, 2) - 7, ITEM_NAME_LENGTH) );

	std::shared_ptr<item_data> id = item_db.searchname( item_name );

	// not found
	if( id == nullptr ){
		clif_sale_search_reply( sd, nullptr );
		return;
	}

	clif_sale_search_reply( sd, cash_shop_db.findItemInTab( CASHSHOP_TAB_SALE, id->nameid ) );
#endif
}

/// Reply if an item was successfully put on sale or not.
/// 09af <result>.W (ZC_ACK_APPLY_BARGAIN_SALE_ITEM)
void clif_sale_add_reply( map_session_data* sd, enum e_sale_add_result result ){
#if PACKETVER_SUPPORTS_SALES
	int fd = sd->fd;

	WFIFOHEAD(fd, 4);
	WFIFOW(fd, 0) = 0x9af;
	WFIFOW(fd, 2) = (uint16)result;
	WFIFOSET(fd, 4);
#endif
}

/// A client request to put an item on sale.
/// 09ae <account id>.L <item id>.W <amount>.L <start time>.L <hours on sale>.B (CZ_REQ_APPLY_BARGAIN_SALE_ITEM)
/// 0a3d <account id>.L <item id>.W <amount>.L <start time>.L <hours on sale>.W (CZ_REQ_APPLY_BARGAIN_SALE_ITEM2)
void clif_parse_sale_add( int fd, map_session_data* sd ){
#if PACKETVER_SUPPORTS_SALES
	nullpo_retv( sd );

	const PACKET_CZ_REQ_APPLY_BARGAIN_SALE_ITEM* p = reinterpret_cast<PACKET_CZ_REQ_APPLY_BARGAIN_SALE_ITEM*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd->status.account_id ){
		return;
	}

	if( !sd->state.sale_open ){
		return;
	}
	
	time_t endTime = p->startTime + p->hours * 60 * 60;

	clif_sale_add_reply( sd, sale_add_item( p->itemId, p->amount, p->startTime, endTime ) );
#endif
}

/// Reply to an item removal from sale.
/// 09b1 <result>.W (ZC_ACK_REMOVE_BARGAIN_SALE_ITEM)
void clif_sale_remove_reply( map_session_data* sd, bool failed ){
#if PACKETVER_SUPPORTS_SALES
	int fd = sd->fd;

	WFIFOHEAD(fd, 4);
	WFIFOW(fd, 0) = 0x9b1;
	WFIFOW(fd, 2) = failed;
	WFIFOSET(fd, 4);
#endif
}

/// Request to remove an item from sale.
/// 09b0 <account id>.L <item id>.W (CZ_REQ_REMOVE_BARGAIN_SALE_ITEM)
void clif_parse_sale_remove( int fd, map_session_data* sd ){
#if PACKETVER_SUPPORTS_SALES
	nullpo_retv( sd );

	const PACKET_CZ_REQ_REMOVE_BARGAIN_SALE_ITEM* p = reinterpret_cast<PACKET_CZ_REQ_REMOVE_BARGAIN_SALE_ITEM*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd->status.account_id ){
		return;
	}

	if( !sd->state.sale_open ){
		return;
	}

	clif_sale_remove_reply( sd, !sale_remove_item( p->itemId ) );
#endif
}

/// Achievement System
/// Author: Luxuri, Aleos

/**
 * Sends all achievement data to the client (ZC_ALL_ACH_LIST).
 * 0a23 <packetLength>.W <ACHCount>.L <ACHPoint>.L
 */
void clif_achievement_list_all(map_session_data *sd)
{
	nullpo_retv(sd);

	if (!battle_config.feature_achievement) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,772),false,SELF); // Achievements are disabled.
		return;
	}

	int count = sd->achievement_data.count; // All achievements should be sent to the client

	if (count == 0)
		return;

	int len = (50 * count) + 22;
	int fd = sd->fd;
	int *info = achievement_level(sd, true);

	WFIFOHEAD(fd,len);
	WFIFOW(fd, 0) = 0xa23;
	WFIFOW(fd, 2) = len;
	WFIFOL(fd, 4) = count; // Amount of achievements the player has in their list (started/completed)
	WFIFOL(fd, 8) = sd->achievement_data.total_score; // Top number
	WFIFOW(fd, 12) = sd->achievement_data.level; // Achievement Level (gold circle)
	WFIFOL(fd, 14) = info[0]; // Achievement EXP (left number in bar)
	WFIFOL(fd, 18) = info[1]; // Achievement EXP TNL (right number in bar)

	for (int i = 0; i < count; i++) {
		WFIFOL(fd, i * 50 + 22) = (uint32)sd->achievement_data.achievements[i].achievement_id;
		WFIFOB(fd, i * 50 + 26) = (uint32)sd->achievement_data.achievements[i].completed > 0;
		for (int j = 0; j < MAX_ACHIEVEMENT_OBJECTIVES; j++) 
			WFIFOL(fd, (i * 50) + 27 + (j * 4)) = (uint32)sd->achievement_data.achievements[i].count[j];
		WFIFOL(fd, i * 50 + 67) = (uint32)sd->achievement_data.achievements[i].completed;
		WFIFOB(fd, i * 50 + 71) = sd->achievement_data.achievements[i].rewarded > 0;
	}
	WFIFOSET(fd, len);
}

/**
 * Sends a single achievement's data to the client (ZC_ACH_UPDATE).
 * 0a24 <ACHPoint>.L
 */
void clif_achievement_update(map_session_data *sd, struct achievement *ach, int count)
{
	nullpo_retv(sd);

	if (!battle_config.feature_achievement) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,772),false,SELF); // Achievements are disabled.
		return;
	}

	int fd = sd->fd;
	int *info = achievement_level(sd, true);

	WFIFOHEAD(fd, packet_len(0xa24));
	WFIFOW(fd, 0) = 0xa24;
	WFIFOL(fd, 2) = sd->achievement_data.total_score; // Total Achievement Points (top of screen)
	WFIFOW(fd, 6) = sd->achievement_data.level; // Achievement Level (gold circle)
	WFIFOL(fd, 8) = info[0]; // Achievement EXP (left number in bar)
	WFIFOL(fd, 12) = info[1]; // Achievement EXP TNL (right number in bar)
	if (ach) {
		WFIFOL(fd, 16) = ach->achievement_id; // Achievement ID
		WFIFOB(fd, 20) = ach->completed > 0; // Is it complete?
		for (int i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; i++)
			WFIFOL(fd, 21 + (i * 4)) = (uint32)ach->count[i]; // 1~10 pre-reqs
		WFIFOL(fd, 61) = (uint32)ach->completed; // Epoch time
		WFIFOB(fd, 65) = ach->rewarded > 0; // Got reward?
	} else
		memset(WFIFOP(fd, 16), 0, 40);
	WFIFOSET(fd, packet_len(0xa24));
}

/**
 * Checks if an achievement reward can be rewarded (CZ_REQ_ACH_REWARD).
 * 0a25 <achievementID>.L
 */
void clif_parse_AchievementCheckReward(int fd, map_session_data *sd)
{
	nullpo_retv(sd);

	if( sd->achievement_data.save )
		intif_achievement_save(sd);

	achievement_check_reward(sd, RFIFOL(fd,2));
}

/**
 * Returns the result of achievement_check_reward (ZC_REQ_ACH_REWARD_ACK).
 * 0a26 <result>.W <achievementID>.L
 */
void clif_achievement_reward_ack(int fd, unsigned char result, int achievement_id)
{
	WFIFOHEAD(fd, packet_len(0xa26));
	WFIFOW(fd, 0) = 0xa26;
	WFIFOB(fd, 2) = result;
	WFIFOL(fd, 3) = achievement_id;
	WFIFOSET(fd, packet_len(0xa26));
}

/// Process the pet evolution request
/// 09fb <packetType>.W <packetLength>.W <evolutionPetEggITID>.W (CZ_PET_EVOLUTION)
void clif_parse_pet_evolution( int fd, map_session_data *sd ){
#if PACKETVER >= 20141008
	const PACKET_CZ_PET_EVOLUTION* p = reinterpret_cast<PACKET_CZ_PET_EVOLUTION*>( RFIFOP( fd, 0 ) );

	auto pet = pet_db_search( p->EvolvedPetEggID, PET_EGG );

	if (!pet) {
		clif_pet_evolution_result(sd, e_pet_evolution_result::FAIL_NOT_PETEGG);
		return;
	}

	pet_evolution(sd, pet->class_);
#endif
}

/// Sends the result of the evolution to the client.
/// 09fc <result>.L (ZC_PET_EVOLUTION_RESULT)
void clif_pet_evolution_result( map_session_data* sd, e_pet_evolution_result result ){
#if PACKETVER >= 20141008
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0x9fc));
	WFIFOW(fd, 0) = 0x9fc;
	WFIFOL(fd, 2) = static_cast<uint32>(result);
	WFIFOSET(fd, packet_len(0x9fc));
#endif
}

/*
 * This packet is sent by /changedress or /nocosplay
 *
 * 0ae8
 */
void clif_parse_changedress( int fd, map_session_data* sd ){
#if PACKETVER >= 20180103
	char command[CHAT_SIZE_MAX];

	safesnprintf( command, sizeof(command), "%cchangedress", atcommand_symbol );
	
	is_atcommand( fd, sd, command, 1 );
#endif
}

/// Opens an UI window of the given type and initializes it with the given data
/// 0AE2 <type>.B <data>.L
void clif_ui_open( map_session_data& sd, enum out_ui_type ui_type, int32 data ){
#if PACKETVER >= 20151202
	// If the UI requires state tracking
	switch( ui_type ){
		case OUT_UI_STYLIST:
			sd.state.stylist_open = true;
			break;
		case OUT_UI_ENCHANTGRADE:
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
			sd.state.enchantgrade_open = true;
			break;
#else
			return;
#endif
	}

	PACKET_ZC_UI_OPEN p = {};

	p.PacketType = HEADER_ZC_UI_OPEN;
	p.UIType = ui_type;
#if PACKETVER >= 20171122
	p.data = data;
#endif

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

/// Request to open an UI window of the given type
/// 0A68 <type>.B
void clif_parse_open_ui( int fd, map_session_data* sd ){
	switch( RFIFOB(fd,2) ){
		case IN_UI_ATTENDANCE:
			if( !pc_has_permission( sd, PC_PERM_ATTENDANCE ) ){
				clif_messagecolor( &sd->bl, color_table[COLOR_RED], msg_txt( sd, 791 ), false, SELF ); // You are not allowed to use the attendance system.
			}else if( pc_attendance_enabled() ){
				clif_ui_open( *sd, OUT_UI_ATTENDANCE, pc_attendance_counter( sd ) );
			}else{
				clif_msg_color( sd, MSI_CHECK_ATTENDANCE_NOT_EVENT, color_table[COLOR_RED] );
			}
			break;
#if PACKETVER >= 20160316
		case IN_UI_MACRO_REGISTER:
			clif_ui_open(*sd, OUT_UI_CAPTCHA, 0);
			break;
		case IN_UI_MACRO_DETECTOR:
			clif_ui_open(*sd, OUT_UI_MACRO, 0);
			break;
#endif
	}
}

/// Response for attedance request
/// 0AF0 <unknown>.L <data>.L
void clif_attendence_response( map_session_data *sd, int32 data ){
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd,packet_len(0xAF0));
	WFIFOW(fd,0) = 0xAF0;
	WFIFOL(fd,2) = 0;
	WFIFOL(fd,6) = data;
	WFIFOSET(fd,packet_len(0xAF0));
}

/// Request from the client to retrieve today's attendance reward
/// 0AEF
void clif_parse_attendance_request( int fd, map_session_data* sd ){
	pc_attendance_claim_reward(sd);
}

/// Send out the percentage of weight that causes it to be displayed in red.
/// 0ADE <percentage>.L
void clif_weight_limit( map_session_data* sd ){
#if PACKETVER >= 20171025
	nullpo_retv(sd);

	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xADE));
	WFIFOW(fd, 0) = 0xADE;
#ifdef RENEWAL
	WFIFOL(fd, 2) = battle_config.natural_heal_weight_rate_renewal;
#else
	WFIFOL(fd, 2) = battle_config.natural_heal_weight_rate;
#endif
	WFIFOSET(fd, packet_len(0xADE));
#endif
}

enum e_private_airship_response : uint32{
	PRIVATEAIRSHIP_OK,
	PRIVATEAIRSHIP_RETRY,
	PRIVATEAIRSHIP_ITEM_NOT_ENOUGH,
	PRIVATEAIRSHIP_DESTINATION_MAP_INVALID,
	PRIVATEAIRSHIP_SOURCE_MAP_INVALID,
	PRIVATEAIRSHIP_ITEM_UNAVAILABLE
};

/// Send out the response to a private airship request
/// 0A4A <response>.L
void clif_private_airship_response( map_session_data* sd, enum e_private_airship_response response ){
#if PACKETVER >= 20180321
	nullpo_retv( sd );

	int fd = sd->fd;

	WFIFOHEAD( fd, packet_len( 0xA4A ) );
	WFIFOW( fd, 0 ) = 0xA4A;
	WFIFOL( fd, 2 ) = response;
	WFIFOSET( fd, packet_len( 0xA4A ) );
#endif
}

/// Parses a request for a private airship
/// 0A49 <mapname>.16B <itemid>.W
void clif_parse_private_airship_request( int fd, map_session_data* sd ){
#if PACKETVER >= 20180321
	// Check if the feature is enabled
	if( !battle_config.feature_privateairship ){
		clif_messagecolor( &sd->bl, color_table[COLOR_RED], msg_txt( sd, 792 ), false, SELF ); // The private airship system is disabled.
		return;
	}

	// Check if the player is allowed to warp from the source map
	if( !map_getmapflag( sd->bl.m, MF_PRIVATEAIRSHIP_SOURCE ) ){
		clif_private_airship_response( sd, PRIVATEAIRSHIP_SOURCE_MAP_INVALID );
		return;
	}

	const PACKET_CZ_PRIVATE_AIRSHIP_REQUEST* p = reinterpret_cast<PACKET_CZ_PRIVATE_AIRSHIP_REQUEST*>( RFIFOP( fd, 0 ) );

	char mapname[MAP_NAME_LENGTH_EXT];

	safestrncpy( mapname, p->mapName, MAP_NAME_LENGTH_EXT );

	int16 mapindex = mapindex_name2id( mapname );

	// Check if we know the mapname
	if( mapindex < 0 ){
		clif_private_airship_response( sd, PRIVATEAIRSHIP_DESTINATION_MAP_INVALID );
		return;
	}

	int16 mapid = map_mapindex2mapid( mapindex );

	// Check if the map is available on this server
	if( mapid < 0 ){
		// TODO: add multi map-server support, cant validate the mapflags of the other server
		return;
	}

	// Check if the player tried to warp to the map he is on
	if( sd->bl.m == mapid ){
		// This is blocked by the client, but just to be sure
		return;
	}

	// This can only be a hack, so we prevent it
	if( map_getmapdata( mapid )->instance_id ){
		// Ignore requests to warp directly into a running instance
		return;
	}

	// Check if the player is allowed to warp to the target map
	if( !map_getmapflag( mapid, MF_PRIVATEAIRSHIP_DESTINATION ) ){
		clif_private_airship_response( sd, PRIVATEAIRSHIP_DESTINATION_MAP_INVALID );
		return;
	}

	t_itemid item_id = p->ItemID;

	// Check if the item sent by the client is known to us
	if( !itemdb_group.item_exists(IG_PRIVATE_AIRSHIP, item_id) ){
		clif_private_airship_response( sd, PRIVATEAIRSHIP_ITEM_UNAVAILABLE );
		return;
	}

	int idx = pc_search_inventory( sd, item_id );

	// Check if the player has the item at all
	if( idx < 0 ){
		clif_private_airship_response( sd, PRIVATEAIRSHIP_ITEM_NOT_ENOUGH );
		return;
	}

	// Delete the chosen item
	if( pc_delitem( sd, idx, 1, 0, 0, LOG_TYPE_PRIVATE_AIRSHIP ) ){
		clif_private_airship_response( sd, PRIVATEAIRSHIP_RETRY );
		return;
	}

	// Warp the player to a random spot on the destination map
	pc_setpos( sd, mapindex, 0, 0, CLR_TELEPORT );
#endif
}

/// Sends out the usage history of the guild storage
/// 09DA <size>.W <result>.W <count>.W { <id>.L <item id>.W <amount>.L <action>.B <refine>.L <unique id>.Q <identify>.B <item type>.W
///      { <card item id>.W }*4 <name>.24B <time>.24B <attribute>.B }*count (ZC_ACK_GUILDSTORAGE_LOG)
void clif_guild_storage_log( map_session_data& sd, std::vector<struct guild_log_entry>& log, enum e_guild_storage_log result ){
#if PACKETVER >= 20140205
	PACKET_ZC_ACK_GUILDSTORAGE_LOG* p = reinterpret_cast<PACKET_ZC_ACK_GUILDSTORAGE_LOG*>( packet_buffer );

	p->packetType = HEADER_ZC_ACK_GUILDSTORAGE_LOG;
	p->PacketLength = sizeof( *p );
	p->result = result;

	if( result == GUILDSTORAGE_LOG_FINAL_SUCCESS ){
		p->amount = (uint16)log.size();

		for( int i = 0; i < log.size(); i++ ){
			PACKET_ZC_ACK_GUILDSTORAGE_LOG_sub& item = p->items[i];
			struct guild_log_entry& entry = log[i];

			item.id = entry.id;
			item.itemId = client_nameid( entry.item.nameid );
			item.amount = (uint16)( entry.amount > 0 ? entry.amount : ( entry.amount * -1 ) );
			item.action = entry.amount > 0; // action = true(put), false(get)
			item.refine = entry.item.refine;
			item.uniqueId = entry.item.unique_id;
			item.IsIdentified = entry.item.identify;
			item.itemType = itemtype( entry.item.nameid );
			clif_addcards( &item.slot, &entry.item );
			safestrncpy( item.name, entry.name, NAME_LENGTH );
			safestrncpy( item.time, entry.time, NAME_LENGTH );
			item.attribute = entry.item.attribute;

			p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof ( item ) );
		}
	}else{
		p->amount = 0;
	}
	
	clif_send( p, p->PacketLength, &sd.bl, SELF );
#endif
}

/// Activates the client camera info or updates the client camera with the given values.
/// 0A78 <type>.B <range>.F <rotation>.F <latitude>.F
void clif_camerainfo( map_session_data* sd, bool show, float range, float rotation, float latitude ){
#if PACKETVER >= 20160525
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xa78));
	WFIFOW(fd, 0) = 0xa78;
	WFIFOB(fd, 2) = show;
	WFIFOF(fd, 3) = range;
	WFIFOF(fd, 7) = rotation;
	WFIFOF(fd, 11) = latitude;
	WFIFOSET(fd, packet_len(0xa78));
#endif
}

/// Activates or deactives the client camera info or updates the camera settings.
/// This packet is triggered by /viewpointvalue or /setcamera
/// 0A77 <type>.B <range>.F <rotation>.F <latitude>.F
void clif_parse_camerainfo( int fd, map_session_data* sd ){
	char command[CHAT_SIZE_MAX];

	// /viewpointvalue
	if( RFIFOB( fd, 2 ) == 1 ){
		safesnprintf( command, sizeof( command ), "%ccamerainfo", atcommand_symbol );
	// /setcamera
	}else{
		safesnprintf( command, sizeof( command ), "%ccamerainfo %03.03f %03.03f %03.03f", atcommand_symbol, RFIFOF( fd, 3 ), RFIFOF( fd, 7 ), RFIFOF( fd, 11 ) );
	}

	is_atcommand( fd, sd, command, 1 );
}

/// Send the full list of items in the equip switch window
/// 0a9b <length>.W { <index>.W <position>.L }*
void clif_equipswitch_list( map_session_data* sd ){
#if PACKETVER >= 20170208
	int fd = sd->fd;
	int offset, i, position;

	WFIFOW(fd, 0) = 0xa9b;
	for( i = 0, offset = 4, position = 0; i < EQI_MAX; i++ ){
		short index = sd->equip_switch_index[i];

		if( index >= 0 && !( position & equip_bitmask[i] ) ){
			WFIFOW(fd, offset) = index + 2;
			WFIFOL(fd, offset + 2) = sd->inventory.u.items_inventory[index].equipSwitch;
			position |= sd->inventory.u.items_inventory[index].equipSwitch;
			offset += 6;
		}
	}
	WFIFOW(fd, 2) = offset;
	WFIFOSET(fd, offset);
#endif
}

/// Acknowledgement for removing an equip to the equip switch window
/// 0a9a <index>.W <position.>.L <failure>.W
void clif_equipswitch_remove( map_session_data* sd, uint16 index, uint32 pos, bool failed ){
#if PACKETVER >= 20170208
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xa9a));
	WFIFOW(fd, 0) = 0xa9a;
	WFIFOW(fd, 2) = index + 2;
	WFIFOL(fd, 4) = pos;
	WFIFOW(fd, 8) = failed;
	WFIFOSET(fd, packet_len(0xa9a));
#endif
}

/// Request to remove an equip from the equip switch window
/// 0a99 <index>.W <position>.L <= 20170502
/// 0a99 <index>.W
void clif_parse_equipswitch_remove( int fd, map_session_data* sd ){
#if PACKETVER >= 20170208
	uint16 index = RFIFOW(fd, 2) - 2;
	bool removed = false;

	if( !battle_config.feature_equipswitch ){
		return;
	}

	// Check if the index is valid
	if( index >= MAX_INVENTORY ){
		return;
	}

	pc_equipswitch_remove( sd, index );
#endif
}

/// Acknowledgement for adding an equip to the equip switch window
/// 0a98 <index>.W <position.>.L <flag>.L  <= 20170502
/// 0a98 <index>.W <position.>.L <flag>.W
void clif_equipswitch_add( map_session_data* sd, uint16 index, uint32 pos, uint8 flag ){
#if PACKETVER >= 20170208
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xa98));
	WFIFOW(fd, 0) = 0xa98;
	WFIFOW(fd, 2) = index + 2;
	WFIFOL(fd, 4) = pos;
#if PACKETVER <= 20170502
	WFIFOL(fd, 8) = flag;
#else
	WFIFOW(fd, 8) = flag;
#endif
	WFIFOSET(fd,packet_len(0xa98));
#endif
}

/// Request to add an equip to the equip switch window
/// 0a97 <index>.W <position>.L
void clif_parse_equipswitch_add( int fd, map_session_data* sd ){
#if PACKETVER >= 20170208
	uint16 index = RFIFOW(fd, 2) - 2;
	uint32 position = RFIFOL(fd, 4);

	if( !battle_config.feature_equipswitch ){
		return;
	}

	if( index >= MAX_INVENTORY || sd->inventory_data[index] == nullptr ){
		return;
	}

	if( sd->state.trading || sd->npc_shopid ){
		clif_equipswitch_add( sd, index, position, ITEM_EQUIP_ACK_OK );
		return;
	}

	if( sd->inventory_data[index]->type == IT_AMMO ){
		position = EQP_AMMO;
	}

	pc_equipitem( sd, index, position, true );
#endif
}

/// Acknowledgement packet for the full equip switch
/// 0a9d <failed>.W
void clif_equipswitch_reply( map_session_data* sd, bool failed ){
#if PACKETVER >= 20170208
	int fd = sd->fd;

	WFIFOHEAD(fd, packet_len(0xa9d));
	WFIFOW(fd, 0) = 0xa9d;
	WFIFOW(fd, 2) = failed;
	WFIFOSET(fd, packet_len(0xa9d));
#endif
}

/// Request to do a full equip switch
/// 0a9c
void clif_parse_equipswitch_request( int fd, map_session_data* sd ){
#if PACKETVER >= 20170208
	int i;
	t_tick tick = gettick();
	uint16 skill_id = ALL_EQSWITCH, skill_lv = 1;

	if( DIFF_TICK(tick, sd->equipswitch_tick) < 0 ) {
		// Client will not let you send a request
		return;
	}

	sd->equipswitch_tick = tick + skill_get_cooldown( skill_id, skill_lv );

	if( !battle_config.feature_equipswitch ){
		return;
	}

	ARR_FIND( 0, EQI_MAX, i, sd->equip_switch_index[i] >= 0 );

	if( i == EQI_MAX ){
		// client will show: "There is no item to replace." and should not even come here
		clif_equipswitch_reply( sd, false );
		return;
	}

	if( pc_issit(sd) && !pc_setstand( sd, false ) ){
		return;
	}

	clif_parse_skill_toid( sd, skill_id, skill_lv, sd->bl.id );
#endif
}

/// Request to do a single equip switch
/// 0ace <index>.W
void clif_parse_equipswitch_request_single( int fd, map_session_data* sd ){
#if PACKETVER >= 20170502
	uint16 index = RFIFOW(fd, 2) - 2;

	if( !battle_config.feature_equipswitch ){
		return;
	}

	// Check if the index is valid
	if( index >= MAX_INVENTORY ){
		return;
	}

	// Check if the item exists
	if (sd->inventory_data[index] == nullptr)
		return;

	// Check if the item was already added to equip switch
	if( sd->inventory.u.items_inventory[index].equipSwitch ){
		if( sd->npc_id ){
#ifdef RENEWAL
			if( pc_hasprogress( sd, WIP_DISABLE_SKILLITEM ) ){
				clif_msg( sd, MSI_BUSY);
				return;
			}
#endif
			if( !sd->npc_item_flag ){
				return;
			}
		}

		pc_equipswitch( sd, index );
		return;
	}

	pc_equipitem( sd, index, pc_equippoint(sd, index), true );
#endif
}

void clif_parse_StartUseSkillToId( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	const PACKET_CZ_USE_SKILL_START* p = reinterpret_cast<PACKET_CZ_USE_SKILL_START*>( RFIFOP( fd, 0 ) );

	// Only rolling cutter is supported for now
	if( p->skillId != GC_ROLLINGCUTTER ){
		return;
	}

	// Already running - cant happen on officials, since only one skill is supported
	if( sd->skill_keep_using.skill_id != 0 ){
		return;
	}

	sd->skill_keep_using.tid = INVALID_TIMER;
	sd->skill_keep_using.skill_id = p->skillId;
	sd->skill_keep_using.level = p->skillLv;
	sd->skill_keep_using.target = p->targetId;

	clif_parse_skill_toid( sd, sd->skill_keep_using.skill_id, sd->skill_keep_using.level, sd->skill_keep_using.target );
#endif
}

void clif_parse_StopUseSkillToId( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	const PACKET_CZ_USE_SKILL_END* p = reinterpret_cast<PACKET_CZ_USE_SKILL_END*>( RFIFOP( fd, 0 ) );

	// Not running
	if( sd->skill_keep_using.skill_id == 0 ){
		return;
	}

#if 0
	// Hack
	if( p->skillId != sd->skill_keep_using.skill_id ){
		return;
	}
#endif

	if( sd->skill_keep_using.tid != INVALID_TIMER ){
		delete_timer( sd->skill_keep_using.tid, skill_keep_using );
		sd->skill_keep_using.tid = INVALID_TIMER;
	}
	sd->skill_keep_using.skill_id = 0;
	sd->skill_keep_using.level = 0;
	sd->skill_keep_using.target = 0;
#endif
}

void clif_ping( map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20190213 || PACKETVER_RE_NUM >= 20190213 || PACKETVER_ZERO_NUM >= 20190130
	nullpo_retv( sd );

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	PACKET_ZC_PING_LIVE p = {};

	p.packetType = HEADER_ZC_PING_LIVE;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

int clif_ping_timer_sub( map_session_data *sd, va_list ap ){
	nullpo_ret( sd );

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return 0;
	}

	t_tick tick = va_arg( ap, t_tick );

	if( session[fd]->wdata_tick + battle_config.ping_time < tick ){
		clif_ping( sd );
	}

	return 0;
}

TIMER_FUNC( clif_ping_timer ){
	map_foreachpc( clif_ping_timer_sub, gettick() );

	return 0;
}

/**
 * Opens the refine UI on the designated client.
 * 0aa0
 */
void clif_refineui_open( map_session_data* sd ){
#if PACKETVER >= 20161012
	nullpo_retv( sd );

	PACKET_ZC_OPEN_REFINING_UI p = {};

	p.packetType = HEADER_ZC_OPEN_REFINING_UI;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );

	sd->state.refineui_open = true;
#endif
}

/**
 * Parses cancel/close of the refine UI on client side.
 * 0aa4
 */
void clif_parse_refineui_close( int fd, map_session_data* sd ){
#if PACKETVER >= 20161012
	sd->state.refineui_open = false;
#endif
}

/**
 * Adds the selected item into the refine UI and sends the possible materials
 * to the client.
 * 0aa2 <length>.W <index>.W <catalyst count>.B { <material>.W <chance>.B <price>.L }*
 */
void clif_refineui_info( map_session_data* sd, uint16 index ){
#if PACKETVER >= 20161012
	int fd = sd->fd;

	// Get the item db reference
	struct item_data* id = sd->inventory_data[index];

	// No item data was found
	if( id == nullptr ){
		return;
	}

	// Check the inventory
	struct item* item = &sd->inventory.u.items_inventory[index];

	// No item was found at the given index
	if( item == nullptr ){
		return;
	}

	// Check if the item is identified
	if( !item->identify ){
		return;
	}

	// Check if the item is broken
	if( item->attribute ){
		return;
	}

	PACKET_ZC_REFINING_MATERIAL_LIST* p = reinterpret_cast<PACKET_ZC_REFINING_MATERIAL_LIST*>( packet_buffer );

	p->packetType = HEADER_ZC_REFINING_MATERIAL_LIST;
	p->packetLength = sizeof( *p );
	p->itemIndex = client_index( index );

	std::shared_ptr<s_refine_level_info> info = refine_db.findLevelInfo( *id, *item );

	// No possibilities were found
	if( info == nullptr ){
		p->blacksmithBlessing = 0;
	}else{
		p->blacksmithBlessing = static_cast<decltype(p->blacksmithBlessing)>( info->blessing_amount );

		for( uint16 i = REFINE_COST_NORMAL, count = 0; i < REFINE_COST_MAX; i++ ){
			std::shared_ptr<s_refine_cost> cost = util::umap_find( info->costs, i );

			if( cost == nullptr ){
				continue;
			}

			PACKET_ZC_REFINING_MATERIAL_LIST_SUB& entry = p->req[count];

			entry.itemId = client_nameid( cost->nameid );
			entry.chance = static_cast<decltype(entry.chance)>( cost->chance / 100 );
			entry.zeny = cost->zeny;

			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
			count++;
		}
	}

	clif_send( p, p->packetLength, &sd->bl, SELF );
#endif
}

/**
 * Client request to add an item to the refine UI.
 * 0aa1 <index>.W
 */
void clif_parse_refineui_add( int fd, map_session_data* sd ){
#if PACKETVER >= 20161012
	const PACKET_CZ_REFINING_SELECT_ITEM* p = reinterpret_cast<PACKET_CZ_REFINING_SELECT_ITEM*>( RFIFOP( fd, 0 ) );

	uint16 index = server_index( p->index );

	// Check if the refine UI is open
	if( !sd->state.refineui_open ){
		return;
	}

	// Check if the index is valid
	if( index >= MAX_INVENTORY ){
		return;
	}

	// Send out the requirements for the refine process
	clif_refineui_info( sd, index );
#endif
}

/**
 * Client requests to try to refine an item.
 * 0aa3 <index>.W <material>.W <catalyst>.B
 */
void clif_parse_refineui_refine( int fd, map_session_data* sd ){
#if PACKETVER >= 20161012
	const PACKET_CZ_REQ_REFINING* p = reinterpret_cast<PACKET_CZ_REQ_REFINING*>( RFIFOP( fd, 0 ) );

	uint16 index = server_index( p->index );
	t_itemid material = p->itemId;
	int16 j;

	// Check if the refine UI is open
	if( !sd->state.refineui_open ){
		return;
	}

	// Check if the index is valid
	if( index >= MAX_INVENTORY ){
		return;
	}

	// Get the item db reference
	struct item_data* id = sd->inventory_data[index];

	// No item data was found
	if( id == nullptr ){
		return;
	}

	// Check the inventory
	struct item* item = &sd->inventory.u.items_inventory[index];

	// No item was found at the given index
	if( item == nullptr ){
		return;
	}

	// Check if the item is identified
	if( !item->identify ){
		return;
	}

	// Check if the item is broken
	if( item->attribute ){
		return;
	}

	std::shared_ptr<s_refine_level_info> info = refine_db.findLevelInfo( *id, *item );

	// No refine possible
	if( info == nullptr ){
		return;
	}

	// No possibilities were found
	if( info->costs.empty() ){
		return;
	}

	// Check if the player has the selected material
	if( ( j = pc_search_inventory( sd, material ) ) < 0 ){
		return;
	}

	int16 blacksmith_index = -1;
	uint16 blacksmith_amount = 0;

	// Check if the player has enough blacksmith blessings
	if( p->blacksmithBlessing != 0 ){
		blacksmith_amount = info->blessing_amount;

		// Player tried to use blacksmith blessing on a refine level, where it is not available
		if( blacksmith_amount == 0 ){
			return;
		}

		// Check if the player has blacksmith blessings
		if( ( blacksmith_index = pc_search_inventory( sd, ITEMID_BLACKSMITH_BLESSING ) ) < 0 ){
			return;
		}

		// Check if the player has enough blacksmith blessings
		if( sd->inventory.u.items_inventory[blacksmith_index].amount < blacksmith_amount ){
			return;
		}
	}

	std::shared_ptr<s_refine_cost> cost = nullptr;

	for( const auto& pair : info->costs ){
		if( pair.second->nameid == material ){
			cost = pair.second;
			break;
		}
	}

	// The material was not in the list of possible materials
	if( cost == nullptr ){
		return;
	}

	// Try to pay for the refine
	if( pc_payzeny( sd, cost->zeny, LOG_TYPE_CONSUME ) ){
		clif_npc_buy_result( sd, e_purchase_result::PURCHASE_FAIL_MONEY ); // "You do not have enough zeny."
		return;
	}

	// Delete the required material
	if( pc_delitem( sd, j, 1, 0, 0, LOG_TYPE_CONSUME ) ){
		return;
	}

	// Delete the required blacksmith blessings
	if( blacksmith_amount > 0 && pc_delitem( sd, blacksmith_index, blacksmith_amount, 0, 0, LOG_TYPE_CONSUME ) ){
		return;
	}

	// Try to refine the item
	if( cost->chance >= ( rnd() % 10000 ) ){
		log_pick_pc( sd, LOG_TYPE_OTHER, -1, item );
		// Success
		item->refine = cap_value( item->refine + 1, 0, MAX_REFINE );
		log_pick_pc( sd, LOG_TYPE_OTHER, 1, item );
		clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_SUCCESS );
		clif_refine( *sd, index, ITEMREFINING_SUCCESS );
		if (info->broadcast_success) {
			clif_broadcast_refine_result(*sd, item->nameid, item->refine, true);
		}
		if( id->type == IT_WEAPON ){
			achievement_update_objective( sd, AG_ENCHANT_SUCCESS, 2, id->weapon_level, item->refine );
		}
		clif_refineui_info( sd, index );
	}else{
		// Failure

		if (info->broadcast_failure) {
			clif_broadcast_refine_result(*sd, item->nameid, item->refine, false);
		}
		// Blacksmith blessings were used to prevent breaking and downgrading
		if( blacksmith_amount > 0 ){
			clif_refine( *sd, index, ITEMREFINING_FAILURE2 );
			clif_refineui_info( sd, index );
		// Delete the item if it is breakable
		}else if( cost->breaking_rate > 0 && ( rnd() % 10000 ) < cost->breaking_rate ){
			clif_refine( *sd, index, ITEMREFINING_FAILURE );
			pc_delitem( sd, index, 1, 0, 2, LOG_TYPE_CONSUME );
		// Downgrade the item if necessary
		}else if( cost->downgrade_amount > 0 ){
			item->refine = cap_value( item->refine - cost->downgrade_amount, 0, MAX_REFINE );
			clif_refine( *sd, index, ITEMREFINING_DOWNGRADE );
			clif_refineui_info(sd, index);
		// Only show failure, but dont do anything
		}else{
			clif_refine( *sd, index, ITEMREFINING_FAILURE2 );
			clif_refineui_info( sd, index );
		}

		clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_FAILURE );
		achievement_update_objective( sd, AG_ENCHANT_FAIL, 1, 1 );
	}
#endif
}

void clif_unequipall_reply( map_session_data* sd, bool failed ){
#if PACKETVER_MAIN_NUM >= 20210818 || PACKETVER_RE_NUM >= 20211103
	PACKET_ZC_ACK_TAKEOFF_EQUIP_ALL p = {};

	p.PacketType = HEADER_ZC_ACK_TAKEOFF_EQUIP_ALL;
	p.result = failed;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif // PACKETVER_MAIN_NUM >= 20210818 || PACKETVER_RE_NUM >= 20211103
}

void clif_parse_unequipall( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20210818 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20210818
	if( pc_cant_act( sd ) ){
		clif_unequipall_reply( sd, true );
		return;
	}

	const PACKET_CZ_REQ_TAKEOFF_EQUIP_ALL* p = reinterpret_cast<PACKET_CZ_REQ_TAKEOFF_EQUIP_ALL*>( RFIFOP( fd, 0 ) );

#if PACKETVER_MAIN_NUM >= 20230906
	uint32 location = p->location;
	int max = EQI_MAX;
#else
	uint32 location = 0xFFFFFFFF;
	int max = EQI_COSTUME_HEAD_TOP;
#endif

	for( int i = 0; i < max; i++ ){
		if( sd->equip_index[i] >= 0 && ( location & equip_bitmask[i] ) != 0 ){
			pc_unequipitem( sd, sd->equip_index[i], 1 );
		}
	}

	clif_unequipall_reply( sd, false );
#endif
}

void clif_stylist_response( map_session_data* sd, bool failed ){
#if PACKETVER >= 20151104
	PACKET_ZC_STYLE_CHANGE_RES p = {};

	p.PacketType = HEADER_ZC_STYLE_CHANGE_RES;
	p.flag = failed;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );

	if( !failed ){
		sd->state.stylist_open = false;
	}
#endif
}

bool clif_parse_stylist_buy_sub( map_session_data* sd, _look look, int16 index ){
	std::shared_ptr<s_stylist_list> list = stylist_db.find( look );

	if( list == nullptr ){
		return false;
	}

	std::shared_ptr<s_stylist_entry> entry = util::umap_find( list->entries, index );

	if( entry == nullptr ){
		return false;
	}

	std::shared_ptr<s_stylist_costs> costs;

	if( ( sd->class_ & MAPID_BASEMASK ) == MAPID_SUMMONER ){
		costs = entry->doram;
	}else{
		costs = entry->human;
	}

	if( costs == nullptr ){
		return false;
	}

	if( sd->status.zeny < costs->price ){
		return false;
	}

	int16 inventoryIndex = -1;

	if( costs->requiredItem != 0 ){
		inventoryIndex = pc_search_inventory( sd, costs->requiredItem );

		if( inventoryIndex < 0 ){
			// No other option
			if( costs->requiredItemBox == 0 ){
				return false;
			}

			// Check if the box that contains the item is in the inventory
			inventoryIndex = pc_search_inventory( sd, costs->requiredItemBox );

			// The box containing the item also does not exist
			if( inventoryIndex < 0 ){
				return false;
			}
		}
	}else if( costs->requiredItemBox != 0 ){
		inventoryIndex = pc_search_inventory( sd, costs->requiredItem );

		if( inventoryIndex < 0 ){
			return false;
		}
	}

	if( inventoryIndex >= 0 && pc_delitem( sd, inventoryIndex, 1, 0, 0, LOG_TYPE_OTHER ) != 0 ){
		return false;
	}

	if( costs->price > 0 && pc_payzeny( sd, costs->price, LOG_TYPE_OTHER ) != 0 ){
		return false;
	}

	switch( look ){
		case LOOK_HAIR:
		case LOOK_HAIR_COLOR:
		case LOOK_CLOTHES_COLOR:
		case LOOK_BODY2:
			pc_changelook( sd, look, entry->value );
			break;
		case LOOK_HEAD_BOTTOM:
		case LOOK_HEAD_MID:
		case LOOK_HEAD_TOP: {
			struct mail_message msg = {};

			msg.dest_id = sd->status.char_id;
			safestrncpy( msg.send_name, "Styling Shop", NAME_LENGTH );
			safestrncpy( msg.title, "<MSG>2949</MSG>", MAIL_TITLE_LENGTH );
			safestrncpy( msg.body, "<MSG>2950</MSG>", MAIL_BODY_LENGTH );

			msg.item[0].nameid = entry->value;
			msg.item[0].identify = 1;
			msg.item[0].amount = 1;

			msg.status = MAIL_NEW;
			msg.type = MAIL_INBOX_NORMAL;
			msg.timestamp = time( nullptr );

			intif_Mail_send( 0, &msg );

			} break;
	}

	return true;
}

void clif_parse_stylist_buy( int fd, map_session_data* sd ){
#if PACKETVER >= 20151104
#if PACKETVER >= 20180516
	const PACKET_CZ_REQ_STYLE_CHANGE2* p = reinterpret_cast<PACKET_CZ_REQ_STYLE_CHANGE2*>( RFIFOP( fd, 0 ) );
#else
	const PACKET_CZ_REQ_STYLE_CHANGE* p = reinterpret_cast<PACKET_CZ_REQ_STYLE_CHANGE*>( RFIFOP( fd, 0 ) );
#endif
	if( p->HeadPalette != 0 && !clif_parse_stylist_buy_sub( sd, LOOK_HAIR_COLOR, p->HeadPalette ) ){
		clif_stylist_response( sd, true );
		return;
	}

	if( p->HeadStyle != 0 && !clif_parse_stylist_buy_sub( sd, LOOK_HAIR, p->HeadStyle ) ){
		clif_stylist_response( sd, true );
		return;
	}

	if( p->BodyPalette != 0 && !clif_parse_stylist_buy_sub( sd, LOOK_CLOTHES_COLOR, p->BodyPalette ) ){
		clif_stylist_response( sd, true );
		return;
	}

	if( p->TopAccessory != 0 && !clif_parse_stylist_buy_sub( sd, LOOK_HEAD_TOP, p->TopAccessory ) ){
		clif_stylist_response( sd, true );
		return;
	}

	if( p->MidAccessory != 0 && !clif_parse_stylist_buy_sub( sd, LOOK_HEAD_MID, p->MidAccessory ) ){
		clif_stylist_response( sd, true );
		return;
	}

	if( p->BottomAccessory != 0 && !clif_parse_stylist_buy_sub( sd, LOOK_HEAD_BOTTOM, p->BottomAccessory ) ){
		clif_stylist_response( sd, true );
		return;
	}

#if PACKETVER >= 20180516
	if( p->BodyStyle != 0 && ( sd->class_ & JOBL_THIRD ) != 0 && ( sd->class_ & JOBL_FOURTH ) == 0 && !clif_parse_stylist_buy_sub( sd, LOOK_BODY2, p->BodyStyle ) ){
		clif_stylist_response( sd, true );
		return;
	}
#endif

	clif_stylist_response( sd, false );
#endif
}

void clif_parse_stylist_close( int fd, map_session_data* sd ){
#if PACKETVER >= 20151104
	sd->state.stylist_open = false;
#endif
}

void clif_inventory_expansion_info( map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	nullpo_retv( sd );

	PACKET_ZC_EXTEND_BODYITEM_SIZE p = {};

	p.packetType = HEADER_ZC_EXTEND_BODYITEM_SIZE;
	p.expansionSize = sd->status.inventory_slots - INVENTORY_BASE_SIZE;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

enum class e_inventory_expansion_response : uint8{
	ASK_CONFIRMATION = 0,
	FAILED,
	BUSY,
	MISSING_ITEM,
	MAXIMUM_REACHED
};

void clif_inventory_expansion_response( map_session_data* sd, e_inventory_expansion_response response ){
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	nullpo_retv( sd );

	PACKET_ZC_ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE p = {};

	p.packetType = HEADER_ZC_ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE;
	p.result = (uint8)response;
	p.itemId = sd->state.inventory_expansion_confirmation;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_inventory_expansion_request( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	// Check if player is dead or busy with other stuff
	if( pc_isdead( sd ) || pc_cant_act( sd ) ){
		clif_inventory_expansion_response( sd, e_inventory_expansion_response::BUSY );
		return;
	}

	// Check if the player already reached the maximum
	if( sd->status.inventory_slots >= MAX_INVENTORY ){
		clif_inventory_expansion_response( sd, e_inventory_expansion_response::MAXIMUM_REACHED );
		return;
	}

	static std::map<t_itemid, uint16> items = {
		// The order of entries in this list defines which will be used first
		// This order and the usable items are hardcoded into the client
		// The number of increased slots is "hardcoded" in the message of the client and cannot be sent per item
		{ ITEMID_INVENTORY_EX_EVT, 10 },
		{ ITEMID_INVENTORY_EX_DIS, 10 },
		{ ITEMID_INVENTORY_EX, 10 },
	};

	int16 index = -1;
	bool found_over_limit = false;

	for( const auto& entry : items ){
		// Check if the player has the required item
		index = pc_search_inventory( sd, entry.first );

		// Found an item
		if( index >= 0 ){
			// Check if the player would exceed the maximum
			if( sd->status.inventory_slots + entry.second > MAX_INVENTORY ){
				found_over_limit = true;
			}else{
				found_over_limit = false;
				sd->state.inventory_expansion_confirmation = entry.first;
				sd->state.inventory_expansion_amount = entry.second;
				break;
			}
		}
	}

	// Check if an item was found
	if( sd->state.inventory_expansion_confirmation == 0 ){
		clif_inventory_expansion_response( sd, e_inventory_expansion_response::MISSING_ITEM );
		return;
	}

	// Check if an item would have been found, but the player would exceed the maximum
	if( found_over_limit ){
		clif_inventory_expansion_response( sd, e_inventory_expansion_response::MAXIMUM_REACHED );
		return;
	}

	// The player met all requirements => ask him for confirmation
	clif_inventory_expansion_response( sd, e_inventory_expansion_response::ASK_CONFIRMATION );
#endif
}

enum class e_inventory_expansion_result : uint8{
	SUCCESS = 0,
	FAILED,
	BUSY,
	MISSING_ITEM,
	MAXIMUM_REACHED
};

void clif_inventory_expansion_result( map_session_data* sd, e_inventory_expansion_result result ){
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	nullpo_retv( sd );

	PACKET_ZC_ACK_EXTEND_BODYITEM_SIZE p = {};

	p.packetType = HEADER_ZC_ACK_EXTEND_BODYITEM_SIZE;
	p.result = (uint8)result;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );

	// Reset the state tracking
	sd->state.inventory_expansion_confirmation = 0;
	sd->state.inventory_expansion_amount = 0;
#endif
}

void clif_parse_inventory_expansion_confirm( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	if( sd->state.inventory_expansion_confirmation == 0 ){
		return;
	}

	// Check if player is dead
	if( pc_isdead( sd ) ){
		clif_inventory_expansion_result( sd, e_inventory_expansion_result::BUSY );
		return;
	}

	// Check if the player already reached the maximum
	if( sd->status.inventory_slots >= MAX_INVENTORY ){
		clif_inventory_expansion_result( sd, e_inventory_expansion_result::MAXIMUM_REACHED );
		return;
	}

	// Check if the player has the required item
	int index = pc_search_inventory( sd, sd->state.inventory_expansion_confirmation );

	// The player did not have the item anymore
	if( index < 0 ){
		clif_inventory_expansion_result( sd, e_inventory_expansion_result::MISSING_ITEM );
		return;
	}

	// Check if the player would exceed the maximum
	if( sd->status.inventory_slots + sd->state.inventory_expansion_amount > MAX_INVENTORY ){
		clif_inventory_expansion_result( sd, e_inventory_expansion_result::MAXIMUM_REACHED );
		return;
	}

	// Delete the required item
	if( pc_delitem( sd, index, 1, 0, 0, LOG_TYPE_OTHER ) ){
		clif_inventory_expansion_result( sd, e_inventory_expansion_result::FAILED );
		return;
	}

	// Increase the slots
	sd->status.inventory_slots += sd->state.inventory_expansion_amount;

	// Save player data (slots) and inventory data (removed item)
	chrif_save( sd, CSAVE_NORMAL | CSAVE_INVENTORY );

	// Inform the player of success
	clif_inventory_expansion_result( sd, e_inventory_expansion_result::SUCCESS );
	clif_inventory_expansion_info( sd );
#endif
}

void clif_parse_inventory_expansion_reject( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
	sd->state.inventory_expansion_confirmation = 0;
	sd->state.inventory_expansion_amount = 0;
#endif
}

void clif_barter_open( map_session_data& sd, struct npc_data& nd ){
#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
	if( nd.subtype != NPCTYPE_BARTER || nd.u.barter.extended || sd.state.barter_open ){
		return;
	}

	std::shared_ptr<s_npc_barter> barter = barter_db.find( nd.exname );

	if( barter == nullptr ){
		return;
	}

	sd.state.barter_open = true;

	PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO* p = reinterpret_cast<PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO*>( packet_buffer );

	p->packetType = HEADER_ZC_NPC_BARTER_MARKET_ITEMINFO;
	p->packetLength = sizeof( *p );

	int16 count = 0;
	for( const auto& itemPair : barter->items ){
		std::shared_ptr<item_data> id = item_db.find(itemPair.second->nameid);

		if( id == nullptr ){
			continue;
		}

		PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO_sub& item = p->list[count];

		item.nameid = client_nameid( id->nameid );
		item.type = itemtype( id->nameid );
		if( itemPair.second->stockLimited ){
			item.amount = itemPair.second->stock;
		}else{
			item.amount = -1;
		}
		item.weight = id->weight;
		item.index = itemPair.second->index;
#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103
		item.viewSprite = id->look;
		item.location = pc_equippoint_sub( &sd, id.get() );
#endif

		// Use a loop if someone did not start with index 0
		for( const auto& requirementPair : itemPair.second->requirements ){
			std::shared_ptr<s_npc_barter_requirement> requirement = requirementPair.second;

			item.currencyNameid = client_nameid( requirement->nameid );
			item.currencyAmount = requirement->amount;

			// It is a normal barter, cancel after first entry
			break;
		}

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( item ) );
		count++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
}

void clif_parse_barter_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
	if( sd->state.barter_open ){
		sd->npc_shopid = 0;
		sd->state.barter_open = false;
	}
#endif
}

void clif_parse_barter_buy( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
	// No shop open
	if( sd->npc_shopid == 0 || !sd->state.barter_open ){
		return;
	}

	struct npc_data* nd = map_id2nd( sd->npc_shopid );

	// Unknown shop
	if( nd == nullptr ){
		return;
	}

	// Not a barter
	if( nd->subtype != NPCTYPE_BARTER ){
		return;
	}

	// It is an extended barter
	if( nd->u.barter.extended ){
		return;
	}

	std::shared_ptr<s_npc_barter> barter = barter_db.find( nd->exname );

	if( barter == nullptr ){
		return;
	}

	const PACKET_CZ_NPC_BARTER_MARKET_PURCHASE* p = reinterpret_cast<PACKET_CZ_NPC_BARTER_MARKET_PURCHASE*>( RFIFOP( fd, 0 ) );

	uint16 entries = ( p->packetLength - sizeof( *p ) ) / sizeof( p->list[0] );

	// Empty purchase list
	if( entries == 0 ){
		return;
	}

	std::vector<s_barter_purchase> purchases;

	purchases.reserve( entries );

	// Make sure each shop index and target item id is only used once
	for( int i = 0; i < entries; i++ ){
		std::shared_ptr<s_npc_barter_item> item = util::map_find( barter->items, (uint16)p->list[i].shopIndex );

		// Invalid shop index
		if( item == nullptr ){
			return;
		}

		for( int j = i + 1; j < entries; j++ ){
			// Same shop index
			if( p->list[i].shopIndex == p->list[j].shopIndex ){
				return;
			}

			std::shared_ptr<s_npc_barter_item> item2 = util::map_find( barter->items, (uint16)p->list[j].shopIndex );

			// Invalid shop index
			if( item2 == nullptr ){
				return;
			}

			// Same target item id
			if( item->nameid == item2->nameid ){
				return;
			}
		}

		s_barter_purchase purchase = {};

		purchase.item = item;
		purchase.amount = p->list[i].amount;

		purchases.push_back( purchase );
	}

	clif_npc_buy_result( sd, npc_barter_purchase( *sd, barter, purchases )  );
#endif
}

void clif_barter_extended_open( map_session_data& sd, struct npc_data& nd ){
#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
	if( nd.subtype != NPCTYPE_BARTER || !nd.u.barter.extended || sd.state.barter_extended_open ){
		return;
	}

	std::shared_ptr<s_npc_barter> barter = barter_db.find( nd.exname );

	if( barter == nullptr ){
		return;
	}

	sd.state.barter_extended_open = true;

	PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO* p = reinterpret_cast<PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO*>( packet_buffer );

	p->packetType = HEADER_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO;
	p->packetLength = sizeof( *p );
	p->items_count = 0;

	for( const auto& itemPair : barter->items ){
		std::shared_ptr<item_data> id = item_db.find(itemPair.second->nameid);

		if( id == nullptr ){
			continue;
		}

		// Needs dynamic calculation, because of variable currencies
		PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub* item = reinterpret_cast<PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub*>( WBUFP( p, p->packetLength ) );

		item->nameid = client_nameid( id->nameid );
		item->type = itemtype( id->nameid );
		if( itemPair.second->stockLimited ){
			item->amount = itemPair.second->stock;
		}else{
			item->amount = -1;
		}
		item->weight = id->weight;
		item->index = itemPair.second->index;
		item->zeny = itemPair.second->price;
#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103
		item->viewSprite = id->look;
		item->location = pc_equippoint_sub( &sd, id.get() );
#endif

		// Because of a MSVS bug, the currencies have been defined with a fixed array entry, which has to be substracted
		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( *item ) - sizeof( item->currencies ) );
		p->items_count++;

		item->currency_count = 0;

		for( const auto& requirementPair : itemPair.second->requirements ){
			// Needs dynamic calculation, because of variable currencies
			PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2* req = reinterpret_cast<PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO_sub2*>( WBUFP( p, p->packetLength ) );
			std::shared_ptr<s_npc_barter_requirement> requirement = requirementPair.second;

			req->nameid = requirement->nameid;
			if( requirement->refine >= 0 ){
				req->refine_level = requirement->refine;
			}else{
				req->refine_level = 0;
			}
			req->amount = requirement->amount;
			req->type = itemtype( requirement->nameid );

			p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( *req ) );
			item->currency_count++;
		}
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
}

void clif_parse_barter_extended_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
	if( sd->state.barter_extended_open ){
		sd->npc_shopid = 0;
		sd->state.barter_extended_open = false;
	}
#endif
}

void clif_parse_barter_extended_buy( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
	// No shop open
	if( sd->npc_shopid == 0 || !sd->state.barter_extended_open ){
		return;
	}

	struct npc_data* nd = map_id2nd( sd->npc_shopid );

	// Unknown shop
	if( nd == nullptr ){
		return;
	}

	// Not a barter
	if( nd->subtype != NPCTYPE_BARTER ){
		return;
	}

	// Not an extended barter
	if( !nd->u.barter.extended ){
		return;
	}

	std::shared_ptr<s_npc_barter> barter = barter_db.find( nd->exname );

	if( barter == nullptr ){
		return;
	}

	const PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_PURCHASE* p = reinterpret_cast<PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_PURCHASE*>( RFIFOP( fd, 0 ) );

	uint16 entries = ( p->packetLength - sizeof( *p ) ) / sizeof( p->list[0] );

	// Empty purchase list
	if( entries == 0 ){
		return;
	}

	std::vector<s_barter_purchase> purchases;

	purchases.reserve( entries );

	// Make sure each shop index and target item id is only used once
	for( int i = 0; i < entries; i++ ){
		std::shared_ptr<s_npc_barter_item> item = util::map_find( barter->items, (uint16)p->list[i].shopIndex );

		// Invalid shop index
		if( item == nullptr ){
			return;
		}

		for( int j = i + 1; j < entries; j++ ){
			// Same shop index
			if( p->list[i].shopIndex == p->list[j].shopIndex ){
				return;
			}

			std::shared_ptr<s_npc_barter_item> item2 = util::map_find( barter->items, (uint16)p->list[j].shopIndex );

			// Invalid shop index
			if( item2 == nullptr ){
				return;
			}

			// Same target item id
			if( item->nameid == item2->nameid ){
				return;
			}
		}

		s_barter_purchase purchase = {};

		purchase.item = item;
		purchase.amount = p->list[i].amount;

		purchases.push_back( purchase );
	}

	clif_npc_buy_result( sd, npc_barter_purchase( *sd, barter, purchases )  );
#endif
}

void clif_summon_init(struct mob_data& md) {
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	struct block_list* master_bl = battle_get_master(&md.bl);

	if( master_bl == nullptr ){
		return;
	}

	PACKET_ZC_SUMMON_HP_INIT p = {};

	p.PacketType = HEADER_ZC_SUMMON_HP_INIT;
	p.summonAID = md.bl.id;
	p.CurrentHP = md.status.hp;
	p.MaxHP = md.status.max_hp;

	clif_send( &p, sizeof( p ), master_bl, SELF );
#endif
}

void clif_summon_hp_bar(struct mob_data& md) {
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	struct block_list* master_bl = battle_get_master(&md.bl);

	if( master_bl == nullptr ){
		return;
	}

	PACKET_ZC_SUMMON_HP_UPDATE p = {};

	p.PacketType = HEADER_ZC_SUMMON_HP_UPDATE;
	p.summonAID = md.bl.id;
	p.VarId = SP_HP; // HP parameter
	p.Value = md.status.hp;

	clif_send( &p, sizeof( p ), master_bl, SELF );
#endif
}

void clif_laphine_synthesis_open( map_session_data *sd, std::shared_ptr<s_laphine_synthesis> synthesis ){
#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
	nullpo_retv( sd );

	sd->state.laphine_synthesis = synthesis->item_id;

	PACKET_ZC_RANDOM_COMBINE_ITEM_UI_OPEN p = {};

	p.packetType = HEADER_ZC_RANDOM_COMBINE_ITEM_UI_OPEN;
	p.itemId = client_nameid( synthesis->item_id );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_laphine_synthesis_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
	sd->state.laphine_synthesis = 0;
#endif
}

enum e_laphine_synthesis_result : int16{
	LAPHINE_SYNTHESIS_SUCCESS = 0,
	LAPHINE_SYNTHESIS_AMOUNT = 5,
	LAPHINE_SYNTHESIS_ITEM = 7
};

void clif_laphine_synthesis_result( map_session_data* sd, enum e_laphine_synthesis_result result ){
#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
	nullpo_retv( sd );

	PACKET_ZC_ACK_RANDOM_COMBINE_ITEM p = {};

	p.packetType = HEADER_ZC_ACK_RANDOM_COMBINE_ITEM;
	p.result = result;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_laphine_synthesis( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
	if( sd->state.laphine_synthesis == 0 ){
		return;
	}

	const PACKET_CZ_REQ_RANDOM_COMBINE_ITEM* p = reinterpret_cast<PACKET_CZ_REQ_RANDOM_COMBINE_ITEM*>( RFIFOP( fd, 0 ) );

	if( sd->state.laphine_synthesis != p->itemId ){
		return;
	}

	std::shared_ptr<s_laphine_synthesis> synthesis = laphine_synthesis_db.find( sd->state.laphine_synthesis );

	if( synthesis == nullptr ){
		return;
	}

	size_t count = ( p->packetLength - sizeof( PACKET_CZ_REQ_RANDOM_COMBINE_ITEM ) ) / sizeof( PACKET_CZ_REQ_RANDOM_COMBINE_ITEM_sub );

	// Player sent more or less than actually required
	if( count != synthesis->requiredRequirements ){
		return;
	}

	// Check for duplicates
	for( size_t i = 0; i < count; i++ ){
		for( size_t j = i + 1; j < count; j++ ){
			if( p->items[i].index == p->items[j].index ){
				return;
			}
		}
	}

	for( size_t i = 0; i < count; i++ ){
		int16 index = server_index( p->items[i].index );

		if( index >= MAX_INVENTORY ){
			return;
		}

		if( sd->inventory_data[index] == nullptr ){
			return;
		}

		struct item* item = &sd->inventory.u.items_inventory[index];

		std::shared_ptr<s_laphine_synthesis_requirement> requirement = util::umap_find( synthesis->requirements, item->nameid );

		if( requirement == nullptr ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_ITEM );
			return;
		}

		if( p->items[i].count != requirement->amount ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_AMOUNT );
			return;
		}

		if( item->amount < requirement->amount ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_AMOUNT );
			return;
		}

		if( item->refine < synthesis->minimumRefine ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_ITEM );
			return;
		}

		if( item->refine > synthesis->maximumRefine ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_ITEM );
			return;
		}

		if( item->equip != 0 ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_ITEM );
			return;
		}

		if( item->equipSwitch != 0 ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_ITEM );
			return;
		}
	}

	// If triggered from item
	if( sd->itemid == sd->state.laphine_synthesis ){
		int16 index = pc_search_inventory( sd, sd->state.laphine_synthesis );

		if( index < 0 ){
			clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_ITEM );
			return;
		}

		if( ( sd->inventory_data[index]->flag.delay_consume & DELAYCONSUME_NOCONSUME ) == 0 ){
			if( pc_delitem( sd, index, 1, 0, 0, LOG_TYPE_LAPHINE ) != 0 ){
				return;
			}
		}
	}

	for( size_t i = 0; i < count; i++ ){
		int16 index = server_index( p->items[i].index );

		if( pc_delitem( sd, index, p->items[i].count, 0, 0, LOG_TYPE_LAPHINE ) != 0 ){
			return;
		}
	}

	itemdb_group.pc_get_itemgroup( synthesis->rewardGroupId, true, *sd );

	clif_laphine_synthesis_result( sd, LAPHINE_SYNTHESIS_SUCCESS );
#endif
}

void clif_laphine_upgrade_open( map_session_data* sd, std::shared_ptr<s_laphine_upgrade> upgrade ){
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	nullpo_retv( sd );

	sd->state.laphine_upgrade = upgrade->item_id;

	PACKET_ZC_RANDOM_UPGRADE_ITEM_UI_OPEN p = {};

	p.packetType = HEADER_ZC_RANDOM_UPGRADE_ITEM_UI_OPEN;
	p.itemId = client_nameid( upgrade->item_id );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_laphine_upgrade_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	sd->state.laphine_upgrade = 0;
#endif
}

void clif_laphine_upgrade_result( map_session_data *sd, bool failed ){
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	PACKET_ZC_ACK_RANDOM_UPGRADE_ITEM p = {};

	p.packetType = HEADER_ZC_ACK_RANDOM_UPGRADE_ITEM;
	p.result = failed;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

static void clif_item_preview( map_session_data *sd, int16 index ){
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	nullpo_retv( sd );

	struct item* item = &sd->inventory.u.items_inventory[index];

	PACKET_ZC_CHANGE_ITEM_OPTION p = {};

	p.packetType = HEADER_ZC_CHANGE_ITEM_OPTION;
	p.index = client_index( index );
#if PACKETVER_MAIN_NUM >= 20181017 || PACKETVER_RE_NUM >= 20181017 || PACKETVER_ZERO_NUM >= 20181024
	p.isDamaged = item->attribute != 0;
#endif
	p.refiningLevel = item->refine;
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723
	p.grade = item->enchantgrade;
#endif
	clif_addcards( &p.slot, item );
	clif_add_random_options( p.option_data, *item );

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_laphine_upgrade( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
	if( sd->state.laphine_upgrade == 0 ){
		return;
	}

	const PACKET_CZ_REQ_RANDOM_UPGRADE_ITEM* p = reinterpret_cast<PACKET_CZ_REQ_RANDOM_UPGRADE_ITEM*>( RFIFOP( fd, 0 ) );

	if( sd->state.laphine_upgrade != p->itemId ){
		return;
	}

	std::shared_ptr<s_laphine_upgrade> upgrade = laphine_upgrade_db.find( sd->state.laphine_upgrade );

	if( upgrade == nullptr ){
		return;
	}

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if( sd->inventory_data[index] == nullptr ){
		return;
	}

	struct item* item = &sd->inventory.u.items_inventory[index];

	// Not a valid target item
	if( !util::vector_exists( upgrade->target_item_ids, item->nameid ) ){
		clif_laphine_upgrade_result( sd, true );
		return;
	}

	// If target item is not identified
	if( item->identify == 0 ){
		clif_laphine_upgrade_result( sd, true );
		return;
	}

	// If target item is equipped
	if( item->equip != 0 ){
		clif_laphine_upgrade_result( sd, true );
		return;
	}

	// If target item is in equipswitch
	if( item->equipSwitch != 0 ){
		clif_laphine_upgrade_result( sd, true );
		return;
	}

	// Check minimum refine requirement
	if( item->refine < upgrade->minimumRefine ){
		clif_laphine_upgrade_result( sd, true );
		return;
	}

	// Check maximum refine requirement
	if( item->refine > upgrade->maximumRefine ){
		clif_laphine_upgrade_result( sd, true );
		return;
	}

	// If no cards are allowed
	if( !upgrade->cardsAllowed ){
		for( int i = 0; i < MAX_SLOTS; i++ ){
			if( item->card[i] != 0 ){
				clif_laphine_upgrade_result( sd, true );
				return;
			}
		}
	}

	// If random options are required
	if( upgrade->requiredRandomOptions > 0 ){
		int i;

		for( i = MAX_ITEM_RDM_OPT - 1; i >= 0; i-- ){
			if( item->option[i].id != 0 ){
				break;
			}
		}

		if( ( i + 1 ) < upgrade->requiredRandomOptions ){
			clif_laphine_upgrade_result( sd, true );
			return;
		}
	}

	int16 index2 = pc_search_inventory( sd, sd->state.laphine_upgrade );

	if( index2 < 0 ){
		clif_laphine_upgrade_result( sd, true );
		return;
	}

	if( ( sd->inventory_data[index2]->flag.delay_consume & DELAYCONSUME_NOCONSUME ) == 0 ){
		if( pc_delitem( sd, index2, 1, 0, 0, LOG_TYPE_LAPHINE ) != 0 ){
			return;
		}
	}

	// Log removal of item
	log_pick_pc( sd, LOG_TYPE_LAPHINE, -1, item );

	// Visually remove it from the client
	clif_delitem( *sd, index, 1, 0 );

	// Apply the random options
	if( upgrade->randomOptionGroup != nullptr ){
		upgrade->randomOptionGroup->apply( *item );
	}

	// Change the refine rate if needed
	if( upgrade->resultRefine > 0 ){
		// Absolute refine level change
		item->refine = max( item->refine, upgrade->resultRefine );
	}else if( upgrade->resultRefineMaximum > 0 ){
		// If a minimum is specified it can also downgrade
		if( upgrade->resultRefineMinimum ){
			item->refine = static_cast<uint8>( rnd_value<uint16>( upgrade->resultRefineMinimum, upgrade->resultRefineMaximum ) );
		}else{
			// Otherwise it can only be upgraded until the maximum, but not downgraded
			item->refine = static_cast<uint8>( rnd_value<uint16>( item->refine, upgrade->resultRefineMaximum ) );
		}
	}else if( upgrade->resultRefineMinimum > 0 ){
		// No maximum has been specified, so it can be anything between minimum and MAX_REFINE
		item->refine = static_cast<uint8>( rnd_value<uint16>( upgrade->resultRefineMinimum, MAX_REFINE ) );
	}

	// Log retrieving the item again -> with the new options
	log_pick_pc( sd, LOG_TYPE_LAPHINE, 1, item );

	// Make it visible for the client again
	clif_additem( sd, index, 1, 0 );

	// Open a preview of the item
	clif_item_preview( sd, index );

	// Tell the client we are done
	clif_laphine_upgrade_result( sd, false );
#endif
}

void clif_enchantgrade_add( map_session_data& sd, uint16 index = UINT16_MAX, std::shared_ptr<s_enchantgradelevel> gradeLevel = nullptr ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	PACKET_ZC_GRADE_ENCHANT_MATERIAL_LIST* p = reinterpret_cast<PACKET_ZC_GRADE_ENCHANT_MATERIAL_LIST*>( packet_buffer );

	p->PacketType = HEADER_ZC_GRADE_ENCHANT_MATERIAL_LIST;
	p->PacketLength = sizeof( *p );

	if( index < UINT16_MAX ){
		p->index = client_index( index );
		p->success_chance = gradeLevel->chances[sd.inventory.u.items_inventory[index].refine] / 100;
		p->blessing_info.id = client_nameid( gradeLevel->catalyst.item );
		p->blessing_info.amount = gradeLevel->catalyst.amountPerStep;
		p->blessing_info.max_blessing = gradeLevel->catalyst.maximumSteps;
		p->blessing_info.bonus = gradeLevel->catalyst.chanceIncrease / 100;
		// Not displayed by client
		p->protect_itemid = 0;
		p->protect_amount = 0;

		int i = 0;
		for( const auto& pair : gradeLevel->options ){
			GRADE_ENCHANT_MATERIAL& mat = p->material_info[i];
			std::shared_ptr<s_enchantgradeoption> option = pair.second;

			mat.nameid = client_nameid( option->item );
			mat.amount = option->amount;
			mat.price = option->zeny;
			mat.downgrade = option->downgrade_amount > 0;
			mat.breakable = option->breaking_rate > 0;

			p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof( mat ) );
			i++;
		}
	}else{
		p->index = -1;
		p->success_chance = 0;
		p->blessing_info.id = 0;
		p->blessing_info.amount = 0;
		p->blessing_info.max_blessing = 0;
		p->blessing_info.bonus = 0;
		p->protect_itemid = 0;
		p->protect_amount = 0;
	}

	clif_send( p, p->PacketLength, &sd.bl, SELF );
#endif
}

void clif_parse_enchantgrade_add( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	nullpo_retv( sd );

	if( !sd->state.enchantgrade_open ){
		return;
	}

	const PACKET_CZ_GRADE_ENCHANT_SELECT_EQUIPMENT* p = reinterpret_cast<PACKET_CZ_GRADE_ENCHANT_SELECT_EQUIPMENT*>( RFIFOP( fd, 0 ) );

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY || sd->inventory_data[index] == nullptr ){
		return;
	}

	std::shared_ptr<s_enchantgrade> enchantgrade = enchantgrade_db.find( sd->inventory_data[index]->type );

	// Unsupported item type - no answer, because client should have actually prevented this request
	if( enchantgrade == nullptr ){
		return;
	}

	// Item can't be enhanced
	if( !sd->inventory_data[index]->flag.gradable ){
		return;
	}

	uint16 level = 0;

	if( sd->inventory_data[index]->type == IT_WEAPON ){
		level = sd->inventory_data[index]->weapon_level;
	}else if( sd->inventory_data[index]->type == IT_ARMOR ){
		level = sd->inventory_data[index]->armor_level;
	}

	const auto& enchantgradelevels = enchantgrade->levels.find( level );

	// Cannot upgrade this weapon or armor level
	if( enchantgradelevels == enchantgrade->levels.end() ){
		clif_enchantgrade_add( *sd );
		return;
	}

	std::shared_ptr<s_enchantgradelevel> enchantgradelevel = util::map_find( enchantgradelevels->second, (e_enchantgrade)sd->inventory.u.items_inventory[index].enchantgrade );

	// Cannot increase enchantgrade any further - no answer, because client should have actually prevented this request
	if( enchantgradelevel == nullptr ){
		return;
	}

	clif_enchantgrade_add( *sd, index, enchantgradelevel );
#endif
}

/// <summary>
/// Sends the result for trying to enchant an item
/// </summary>
/// <param name="sd">The player session</param>
/// <param name="index">The target item</param>
/// <param name="result">
///  0= The grade has been successfully upgraded.
///  1= Refinement failed.
///  2= The refine level has decreased.
///  3= Equipment destroyed.
///  4= The equipment is protected.
/// </param>
void clif_enchantgrade_result( map_session_data& sd, uint16 index, e_enchantgrade_result result ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	PACKET_ZC_GRADE_ENCHANT_ACK p = {};

	p.PacketType = HEADER_ZC_GRADE_ENCHANT_ACK;
	p.index = client_index( index );
	p.grade = sd.inventory.u.items_inventory[index].enchantgrade;
	p.result = result;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

void clif_enchantgrade_announce( map_session_data& sd, struct item& item, bool success ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	PACKET_ZC_GRADE_ENCHANT_BROADCAST_RESULT p = {};

	p.packetType = HEADER_ZC_GRADE_ENCHANT_BROADCAST_RESULT;
	safestrncpy( p.name, sd.status.name, sizeof( p.name ) );
	p.itemId = client_nameid( item.nameid );
	p.grade = item.enchantgrade;
	p.status = success;

	clif_send( &p, sizeof( p ), nullptr, ALL_CLIENT );
#endif
}

void clif_parse_enchantgrade_start( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	nullpo_retv( sd );

	if( !sd->state.enchantgrade_open ){
		return;
	}

	const PACKET_CZ_GRADE_ENCHANT_REQUEST* p = reinterpret_cast<PACKET_CZ_GRADE_ENCHANT_REQUEST*>( RFIFOP( fd, 0 ) );

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY || sd->inventory_data[index] == nullptr ){
		return;
	}

	std::shared_ptr<s_enchantgrade> enchantgrade = enchantgrade_db.find( sd->inventory_data[index]->type );

	// Unsupported item type - no answer
	if( enchantgrade == nullptr ){
		return;
	}

	uint16 level = 0;

	if( sd->inventory_data[index]->type == IT_WEAPON ){
		level = sd->inventory_data[index]->weapon_level;
	}else if( sd->inventory_data[index]->type == IT_ARMOR ){
		level = sd->inventory_data[index]->armor_level;
	}

	const auto& enchantgradelevels = enchantgrade->levels.find( level );

	// Cannot upgrade this weapon or armor level - no answer
	if( enchantgradelevels == enchantgrade->levels.end() ){
		return;
	}

	std::shared_ptr<s_enchantgradelevel> enchantgradelevel = util::map_find( enchantgradelevels->second, (e_enchantgrade)sd->inventory.u.items_inventory[index].enchantgrade );

	// Cannot increase enchantgrade any further - no answer
	if( enchantgradelevel == nullptr ){
		return;
	}

	uint16 totalChance = enchantgradelevel->chances[sd->inventory.u.items_inventory[index].refine];

	// No chance to increase the enchantgrade
	if( totalChance == 0 ){
		return;
	}

	std::shared_ptr<s_enchantgradeoption> option = util::map_find( enchantgradelevel->options, (uint16)p->material_index );

	// Unknown option id - no answer
	if( option == nullptr ){
		return;
	}

	// Not enough zeny
	if( sd->status.zeny < option->zeny ){
		return;
	}

	uint16 steps = min( p->blessing_amount, enchantgradelevel->catalyst.maximumSteps );
	std::unordered_map<uint16, uint16> requiredItems;

	if( p->blessing_flag ){
		// If the catalysator item is the same as the option item build the sum of amounts
		if( enchantgradelevel->catalyst.item == option->item ){
			uint16 amount = enchantgradelevel->catalyst.amountPerStep * steps + option->amount;

			int16 index = pc_search_inventory( sd, enchantgradelevel->catalyst.item );

			if( index < 0 ){
				return;
			}

			if( sd->inventory.u.items_inventory[index].amount < amount ){
				return;
			}

			requiredItems[index] = amount;
		}else{
			uint16 amount = enchantgradelevel->catalyst.amountPerStep * steps;

			// Check catalysator item
			int16 index = pc_search_inventory( sd, enchantgradelevel->catalyst.item );

			if( index < 0 ){
				return;
			}

			if( sd->inventory.u.items_inventory[index].amount < amount ){
				return;
			}

			requiredItems[index] = amount;

			// Check option item
			index = pc_search_inventory( sd, option->item );

			if( index < 0 ){
				return;
			}

			if( sd->inventory.u.items_inventory[index].amount < option->amount ){
				return;
			}

			requiredItems[index] = option->amount;
		}

		totalChance += steps * enchantgradelevel->catalyst.chanceIncrease;
	}else{
		// Check option item
		int16 index = pc_search_inventory( sd, option->item );

		if( index < 0 ){
			return;
		}

		if( sd->inventory.u.items_inventory[index].amount < option->amount ){
			return;
		}

		requiredItems[index] = option->amount;
	}

	// All items should be there, start deleting
	for( const auto& pair : requiredItems ){
		if( pc_delitem( sd, pair.first, pair.second, 0, 0, LOG_TYPE_ENCHANTGRADE ) != 0 ){
			return;
		}
	}

	if( pc_payzeny( sd, option->zeny, LOG_TYPE_ENCHANTGRADE ) > 0 ){
		return;
	}

	if( rnd()%10000 < totalChance ){
		// Log removal of item
		log_pick_pc( sd, LOG_TYPE_ENCHANTGRADE, -1, &sd->inventory.u.items_inventory[index] );
		// Increase enchantgrade
		sd->inventory.u.items_inventory[index].enchantgrade = min( sd->inventory.u.items_inventory[index].enchantgrade + 1, MAX_ENCHANTGRADE );
		// On successful enchantgrade increase the refine is reset
		sd->inventory.u.items_inventory[index].refine = 0;
		// Log retrieving the item again -> with the new refine and enchantgrade
		log_pick_pc( sd, LOG_TYPE_ENCHANTGRADE, 1, &sd->inventory.u.items_inventory[index] );
		// Show success
		clif_enchantgrade_result( *sd, index, ENCHANTGRADE_UPGRADE_SUCCESS );

		// Check if it has to be announced
		if( enchantgradelevel->announceSuccess ){
			clif_enchantgrade_announce( *sd, sd->inventory.u.items_inventory[index], true );
		}
	}else{
		// Check if it has to be announced (has to be done before deleting the item from inventory)
		if( enchantgradelevel->announceFail ){
			clif_enchantgrade_announce( *sd, sd->inventory.u.items_inventory[index], false );
		}

		// Delete the item if it is breakable
		if( option->breaking_rate > 0 && ( rnd() % 10000 ) < option->breaking_rate ){
			// Delete the item
			pc_delitem( sd, index, 1, 0, 0, LOG_TYPE_ENCHANTGRADE );
			// Show failure
			clif_enchantgrade_result( *sd, index, ENCHANTGRADE_UPGRADE_BREAK );
		// Downgrade the item if necessary
		}else if( option->downgrade_amount > 0 ){
			// Log removal of item
			log_pick_pc( sd, LOG_TYPE_ENCHANTGRADE, -1, &sd->inventory.u.items_inventory[index] );
			// Decrease refine level
			sd->inventory.u.items_inventory[index].refine = cap_value( sd->inventory.u.items_inventory[index].refine - option->downgrade_amount, 0, MAX_REFINE );
			// Log retrieving the item again -> with the new refine
			log_pick_pc( sd, LOG_TYPE_ENCHANTGRADE, 1, &sd->inventory.u.items_inventory[index] );
			// Show downgrade
			clif_enchantgrade_result( *sd, index, ENCHANTGRADE_UPGRADE_DOWNGRADE );
		// Only show failure, but dont do anything
		}else{
			clif_enchantgrade_result( *sd, index, ENCHANTGRADE_UPGRADE_FAILED );
		}
	}
#endif
}

void clif_parse_enchantgrade_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200724
	nullpo_retv( sd );

	sd->state.enchantgrade_open = false;
#endif
}

void clif_reputation_type( map_session_data& sd, int64 type, int64 points ){
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
	PACKET_ZC_REPUTE_INFO* p = reinterpret_cast<PACKET_ZC_REPUTE_INFO*>( packet_buffer );

	p->packetType = HEADER_ZC_REPUTE_INFO;
	p->packetLength = sizeof( *p );
	p->success = true;

	PACKET_ZC_REPUTE_INFO_sub& entry = p->list[0];

	entry.type = type;
	entry.points = points;

	p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );

	clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
}

void clif_reputation_list( map_session_data& sd ){
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
	PACKET_ZC_REPUTE_INFO* p = reinterpret_cast<PACKET_ZC_REPUTE_INFO*>( packet_buffer );

	p->packetType = HEADER_ZC_REPUTE_INFO;
	p->packetLength = sizeof( *p );
	p->success = true;

	size_t index = 0;
	for( const auto& entry : reputation_db ){
		std::shared_ptr<s_reputation> reputation = entry.second;

		PACKET_ZC_REPUTE_INFO_sub& list_entry = p->list[index];

		list_entry.type = reputation->id;
		list_entry.points = pc_readreg2( &sd, reputation->variable.c_str() );

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( list_entry ) );
		index++;
	}

	clif_send( p, p->packetLength, &sd.bl, SELF );
#endif
}

void clif_item_reform_open( map_session_data& sd, t_itemid item ){
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	PACKET_ZC_OPEN_REFORM_UI p = {};

	p.PacketType = HEADER_ZC_OPEN_REFORM_UI;
	p.ITID = item;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );

	sd.state.item_reform = item;
#endif
}

void clif_parse_item_reform_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	sd->state.item_reform = 0;
#endif
}

void clif_item_reform_result( map_session_data& sd, uint16 index, uint8 result ){
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	PACKET_ZC_ITEM_REFORM_ACK p = {};

	p.PacketType = HEADER_ZC_ITEM_REFORM_ACK;
	p.index = client_index( index );
	p.result = result;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );

	if( result == 0 ){
		// Client closes the window on success
		sd.state.item_reform = 0;
	}
#endif
}

void clif_parse_item_reform_start( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	// Not opened
	if( sd->state.item_reform == 0 ){
		return;
	}

	const PACKET_CZ_ITEM_REFORM* p = reinterpret_cast<PACKET_CZ_ITEM_REFORM*>( RFIFOP( fd, 0 ) );

	// Item mismatch
	if( p->ITID != sd->state.item_reform ){
		return;
	}

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if( sd->inventory_data[index] == nullptr ){
		return;
	}

	std::shared_ptr<s_item_reform> reform = item_reform_db.find( sd->state.item_reform );

	if( reform == nullptr ){
		return;
	}

	struct item& selected_item = sd->inventory.u.items_inventory[index];

	std::shared_ptr<s_item_reform_base> base = util::umap_find( reform->base_items, selected_item.nameid );

	if( base == nullptr ){
		return;
	}

	// If target item is not identified
	if( selected_item.identify == 0 ){
		return;
	}

	// If target item is equipped
	if( selected_item.equip != 0 ){
		return;
	}

	// Check minimum refine requirement
	if( selected_item.refine < base->minimumRefine ){
		return;
	}

	// Check maximum refine requirement
	if( selected_item.refine > base->maximumRefine ){
		return;
	}

	// If no cards are allowed
	if( !base->cardsAllowed ){
		for( int i = 0; i < MAX_SLOTS; i++ ){
			if( selected_item.card[i] != 0 ){
				return;
			}
		}
	}

	// If random options are required
	if( base->requiredRandomOptions > 0 ){
		int i;

		for( i = MAX_ITEM_RDM_OPT - 1; i >= 0; i-- ){
			if( selected_item.option[i].id != 0 ){
				break;
			}
		}

		if( ( i + 1 ) < base->requiredRandomOptions ){
			return;
		}
	}

	std::unordered_map<uint16, uint16> materials;

	// Check if all materials exist
	for( const auto& material : base->materials ){
		int16 material_index = pc_search_inventory( sd, material.first );

		if( material_index < 0 ){
			return;
		}

		if( sd->inventory.u.items_inventory[material_index].amount < material.second ){
			return;
		}

		materials[material_index] = material.second;
	}

	// Remove the material
	for( const auto& material : materials ){
		if( pc_delitem( sd, material.first, material.second, 0, 0, LOG_TYPE_REFORM ) != 0 ){
			return;
		}
	}

	// If triggered from item
	if( sd->itemid == sd->state.item_reform && pc_delitem( sd, sd->itemindex, 1, 0, 0, LOG_TYPE_REFORM ) != 0 ){
		return;
	}

	// Log removal of item
	log_pick_pc( sd, LOG_TYPE_REFORM, -1, &selected_item );

	// Visually remove it from the client
	clif_delitem( *sd, index, 1, 0 );

	// Apply the random options
	if( base->randomOptionGroup != nullptr ){
		base->randomOptionGroup->apply( selected_item );
	}

	// Change the refine rate if needed
	if( base->refineChange != 0 ){
		selected_item.refine = cap_value( selected_item.refine + base->refineChange, 0, MAX_REFINE );
	}

	// Remove all cards and socket enchants
	if( base->clearSlots ){
		for( int i = 0; i < MAX_SLOTS; i++ ){
			selected_item.card[i] = 0;
		}
	}

	// Remove the current enchantgrade
	if( base->removeEnchantgrade ){
		selected_item.enchantgrade = 0;
	}

	// Finally change the item id
	selected_item.nameid = base->resultItemId;
	// Link inventory data cache to the new item
	sd->inventory_data[index] = itemdb_search( base->resultItemId );

	// Log retrieving the item again -> with the new options
	log_pick_pc( sd, LOG_TYPE_REFORM, 1, &selected_item );

	// Make it visible for the client again
	clif_additem( sd, index, 1, 0 );

	clif_item_reform_result( *sd, index, 0 );
#endif
}

void clif_enchantwindow_open( map_session_data& sd, uint64 clientLuaIndex ){
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
	// Hardcoded clientside check
	if( sd.weight > ( ( sd.max_weight * 70 ) / 100 ) ){
		clif_msg_color( &sd, MSI_ENCHANT_FAILED_OVER_WEIGHT, color_table[COLOR_RED] );
		sd.state.item_enchant_index = 0;
		return;
		
	}

	PACKET_ZC_UI_OPEN_V3 p = {};

	p.packetType = HEADER_ZC_UI_OPEN_V3;
	p.type = OUT_UI_ENCHANT;
	p.data = clientLuaIndex;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );

	sd.state.item_enchant_index = clientLuaIndex;
#endif
}

void clif_enchantwindow_result( map_session_data& sd, bool success, t_itemid enchant = 0 ){
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
	PACKET_ZC_RESPONSE_ENCHANT p = {};

	p.PacketType = HEADER_ZC_RESPONSE_ENCHANT;
	if( success ){
		p.msgId = MSI_ENCHANT_SUCCESS;
	}else{
		p.msgId = MSI_ENCHANT_FAILED;
	}
	p.ITID = enchant;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );

	sd.state.item_enchant_index = 0;
#endif
}

bool clif_parse_enchant_basecheck( struct item& selected_item, std::shared_ptr<s_item_enchant> enchant ){
	if( selected_item.equip != 0 ){
		return false;
	}

	if( selected_item.equipSwitch != 0 ){
		return false;
	}

	if( selected_item.attribute != 0 ){
		return false;
	}

	if( !util::vector_exists( enchant->target_item_ids, selected_item.nameid ) ){
		return false;
	}

	if( selected_item.refine < enchant->minimumRefine ){
		return false;
	}

	if( selected_item.enchantgrade < enchant->minimumEnchantgrade ){
		return false;
	}

	if( !enchant->allowRandomOptions ){
		for( const s_item_randomoption& option : selected_item.option ){
			if( option.id != 0 ){
				return false;
			}
		}
	}

	return true;
}

void clif_parse_enchantwindow_general( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	const PACKET_CZ_REQUEST_RANDOM_ENCHANT* p = reinterpret_cast<PACKET_CZ_REQUEST_RANDOM_ENCHANT*>( RFIFOP( fd, 0 ) );

	if( sd->state.item_enchant_index != p->enchant_group ){
		return;
	}

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if( sd->inventory_data[index] == nullptr ){
		return;
	}

	struct item& selected_item = sd->inventory.u.items_inventory[index];
	std::shared_ptr<s_item_enchant> enchant = item_enchant_db.find( p->enchant_group );

	if( enchant == nullptr ){
		return;
	}

	if( !clif_parse_enchant_basecheck( selected_item, enchant ) ){
		return;
	}

	uint16 slot = MAX_SLOTS;

	for( uint16 nextSlot : enchant->order ){
		if( selected_item.card[nextSlot] == 0 ){
			slot = nextSlot;
			break;
		}
	}

	if( slot == MAX_SLOTS ){
		return;
	}

	if( slot < sd->inventory_data[index]->slots ){
		return;
	}

	std::shared_ptr<s_item_enchant_slot> enchant_slot = util::umap_find( enchant->slots, slot );

	if( enchant_slot == nullptr ){
		return;
	}

	std::shared_ptr<s_item_enchant_normal> enchants_for_enchantgrade = util::umap_find( enchant_slot->normal.enchants, (uint16)selected_item.enchantgrade );

	if( enchants_for_enchantgrade == nullptr ){
		clif_messagecolor( &sd->bl, color_table[COLOR_RED], msg_txt( sd, 829 ), false, SELF); // Enchanting is not possible for your item's enchant grade.
		clif_enchantwindow_result( *sd, false );
		return;
	}

	if( sd->status.zeny < enchant_slot->normal.zeny ){
		return;
	}

	std::unordered_map<uint16, uint16> materials;

	for( const auto& entry : enchant_slot->normal.materials ){
		int16 idx = pc_search_inventory( sd, entry.first );

		if( idx < 0 ){
			return;
		}

		if( sd->inventory.u.items_inventory[idx].amount < entry.second ){
			return;
		}

		materials[idx] = entry.second;
	}

	if( pc_payzeny( sd, enchant_slot->normal.zeny, LOG_TYPE_ENCHANT ) != 0 ){
		return;
	}

	for( const auto& entry : materials ){
		if( pc_delitem( sd, entry.first, entry.second, 0, 0, LOG_TYPE_ENCHANT )  != 0 ){
			return;
		}
	}

	uint32 chance = enchant_slot->normal.chance;

	for( int i = 0; i <= MAX_ENCHANTGRADE; i++ ){
		chance += enchant_slot->normal.enchantgradeChanceIncrease[i];
	}

	if( chance < 100000 && rnd_value( 0, 100000 ) > chance ){
		clif_enchantwindow_result( *sd, false );
		return;
	}

	// Log removal of item
	log_pick_pc( sd, LOG_TYPE_ENCHANT, -1, &selected_item );

	size_t maximum = 3 * enchant_slot->normal.enchants.size();
	bool enchanted = false;

	for( int i = 0; i < maximum; i++ ){
		std::shared_ptr<s_item_enchant_normal_sub> normal_enchant = util::umap_random( enchants_for_enchantgrade->enchants );

		if( rnd_value( 0, 10000 ) < normal_enchant->chance ){
			selected_item.card[slot] = normal_enchant->item_id;
			enchanted = true;
			break;
		}
	}

	if( !enchanted ){
		std::shared_ptr<s_item_enchant_normal_sub> normal_enchant = util::umap_random( enchants_for_enchantgrade->enchants );
		selected_item.card[slot] = normal_enchant->item_id;
	}

	// Log retrieving the item again -> with the new enchant
	log_pick_pc( sd, LOG_TYPE_ENCHANT, 1, &selected_item );

	clif_enchantwindow_result( *sd, true, selected_item.card[slot] );
#endif
}

void clif_parse_enchantwindow_perfect( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	const PACKET_CZ_REQUEST_PERFECT_ENCHANT* p = reinterpret_cast<PACKET_CZ_REQUEST_PERFECT_ENCHANT*>( RFIFOP( fd, 0 ) );

	if( sd->state.item_enchant_index != p->enchant_group ){
		return;
	}

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if( sd->inventory_data[index] == nullptr ){
		return;
	}

	struct item& selected_item = sd->inventory.u.items_inventory[index];
	std::shared_ptr<s_item_enchant> enchant = item_enchant_db.find( p->enchant_group );

	if( enchant == nullptr ){
		return;
	}

	if( !clif_parse_enchant_basecheck( selected_item, enchant ) ){
		return;
	}

	uint16 slot = MAX_SLOTS;

	for( uint16 nextSlot : enchant->order ){
		if( selected_item.card[nextSlot] == 0 ){
			slot = nextSlot;
			break;
		}
	}

	if( slot == MAX_SLOTS ){
		return;
	}

	if( slot < sd->inventory_data[index]->slots ){
		return;
	}

	std::shared_ptr<s_item_enchant_slot> enchant_slot = util::umap_find( enchant->slots, slot );

	if( enchant_slot == nullptr ){
		return;
	}

	std::shared_ptr<s_item_enchant_perfect> perfect_enchant = util::umap_find( enchant_slot->perfect.enchants, p->ITID );

	if( perfect_enchant == nullptr ){
		return;
	}

	if( sd->status.zeny < perfect_enchant->zeny ){
		return;
	}

	std::unordered_map<uint16, uint16> materials;

	for( const auto& entry : perfect_enchant->materials ){
		int16 idx = pc_search_inventory( sd, entry.first );

		if( idx < 0 ){
			return;
		}

		if( sd->inventory.u.items_inventory[idx].amount < entry.second ){
			return;
		}

		materials[idx] = entry.second;
	}

	if( pc_payzeny( sd, perfect_enchant->zeny, LOG_TYPE_ENCHANT ) != 0 ){
		return;
	}

	for( const auto& entry : materials ){
		if( pc_delitem( sd, entry.first, entry.second, 0, 0, LOG_TYPE_ENCHANT )  != 0 ){
			return;
		}
	}

	// Log removal of item
	log_pick_pc( sd, LOG_TYPE_ENCHANT, -1, &selected_item );

	selected_item.card[slot] = perfect_enchant->item_id;

	// Log retrieving the item again -> with the new enchant
	log_pick_pc( sd, LOG_TYPE_ENCHANT, 1, &selected_item );

	clif_enchantwindow_result( *sd, true, selected_item.card[slot] );
#endif
}

void clif_parse_enchantwindow_upgrade( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	const PACKET_CZ_REQUEST_UPGRADE_ENCHANT* p = reinterpret_cast<PACKET_CZ_REQUEST_UPGRADE_ENCHANT*>( RFIFOP( fd, 0 ) );

	if( sd->state.item_enchant_index != p->enchant_group ){
		return;
	}

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if( sd->inventory_data[index] == nullptr ){
		return;
	}

	struct item& selected_item = sd->inventory.u.items_inventory[index];
	std::shared_ptr<s_item_enchant> enchant = item_enchant_db.find( p->enchant_group );

	if( enchant == nullptr ){
		return;
	}

	if( !clif_parse_enchant_basecheck( selected_item, enchant ) ){
		return;
	}

	uint16 slot = p->slot;

	if( slot >= MAX_SLOTS ){
		return;
	}

	if( slot < sd->inventory_data[index]->slots ){
		return;
	}

	if( selected_item.card[slot] == 0 ){
		return;
	}

	std::shared_ptr<s_item_enchant_slot> enchant_slot = util::umap_find( enchant->slots, slot );

	if( enchant_slot == nullptr ){
		return;
	}

	std::shared_ptr<s_item_enchant_upgrade> upgrade = util::umap_find( enchant_slot->upgrade.enchants, selected_item.card[slot] );

	if( upgrade == nullptr ){
		return;
	}

	if( sd->status.zeny < upgrade->zeny ){
		return;
	}

	std::unordered_map<uint16, uint16> materials;

	for( const auto& entry : upgrade->materials ){
		int16 idx = pc_search_inventory( sd, entry.first );

		if( idx < 0 ){
			return;
		}

		if( sd->inventory.u.items_inventory[idx].amount < entry.second ){
			return;
		}

		materials[idx] = entry.second;
	}

	if( pc_payzeny( sd, upgrade->zeny, LOG_TYPE_ENCHANT ) != 0 ){
		return;
	}

	for( const auto& entry : materials ){
		if( pc_delitem( sd, entry.first, entry.second, 0, 0, LOG_TYPE_ENCHANT )  != 0 ){
			return;
		}
	}

	// Log removal of item
	log_pick_pc( sd, LOG_TYPE_ENCHANT, -1, &selected_item );

	selected_item.card[slot] = upgrade->upgrade_item_id;

	// Log retrieving the item again -> with the new enchant
	log_pick_pc( sd, LOG_TYPE_ENCHANT, 1, &selected_item );

	clif_enchantwindow_result( *sd, true, selected_item.card[slot] );
#endif
}

void clif_parse_enchantwindow_reset( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	const PACKET_CZ_REQUEST_RESET_ENCHANT* p = reinterpret_cast<PACKET_CZ_REQUEST_RESET_ENCHANT*>( RFIFOP( fd, 0 ) );

	if( sd->state.item_enchant_index != p->enchant_group ){
		return;
	}

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if( sd->inventory_data[index] == nullptr ){
		return;
	}

	struct item& selected_item = sd->inventory.u.items_inventory[index];

	if( selected_item.equip != 0 ){
		return;
	}

	if( selected_item.equipSwitch != 0 ){
		return;
	}

	if( selected_item.attribute != 0 ){
		return;
	}

	std::shared_ptr<s_item_enchant> enchant = item_enchant_db.find( p->enchant_group );

	if( enchant == nullptr ){
		return;
	}

	if( !util::vector_exists( enchant->target_item_ids, selected_item.nameid ) ){
		return;
	}

	if( selected_item.refine < enchant->minimumRefine ){
		return;
	}

	if( selected_item.enchantgrade < enchant->minimumEnchantgrade ){
		return;
	}

	bool is_enchanted = false;

	for( int i = sd->inventory_data[index]->slots; i < MAX_SLOTS; i++ ){
		if( selected_item.card[i] != 0 ){
			is_enchanted = true;
			break;
		}
	}

	if( !is_enchanted ){
		return;
	}

	if( sd->status.zeny < enchant->reset.zeny ){
		return;
	}

	std::unordered_map<uint16, uint16> materials;

	for( const auto& entry : enchant->reset.materials ){
		int16 idx = pc_search_inventory( sd, entry.first );

		if( idx < 0 ){
			return;
		}

		if( sd->inventory.u.items_inventory[idx].amount < entry.second ){
			return;
		}

		materials[idx] = entry.second;
	}

	if( pc_payzeny( sd, enchant->reset.zeny, LOG_TYPE_ENCHANT ) != 0 ){
		return;
	}

	for( const auto& entry : materials ){
		if( pc_delitem( sd, entry.first, entry.second, 0, 0, LOG_TYPE_ENCHANT )  != 0 ){
			return;
		}
	}

	uint32 chance = enchant->reset.chance;

	if( chance == 0 ){
		return;
	}

	if( chance < 100000 && rnd_value( 0, 100000 ) > chance ){
		clif_enchantwindow_result( *sd, false );
		return;
	}

	// Log removal of item
	log_pick_pc( sd, LOG_TYPE_ENCHANT, -1, &selected_item );

	for( int i = sd->inventory_data[index]->slots; i < MAX_SLOTS; i++ ){
		selected_item.card[i] = 0;
	}

	// Log retrieving the item again -> with the new enchant
	log_pick_pc( sd, LOG_TYPE_ENCHANT, 1, &selected_item );

	clif_enchantwindow_result( *sd, true );
#endif
}

void clif_parse_enchantwindow_close( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	sd->state.item_enchant_index = 0;
#endif
}

void clif_parse_itempackage_select( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20220316
	const PACKET_CZ_USE_PACKAGEITEM* p = reinterpret_cast<PACKET_CZ_USE_PACKAGEITEM*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd->status.account_id ){
		return;
	}

	uint16 index = server_index( p->index );

	if( index >= MAX_INVENTORY ){
		return;
	}

	if( sd->inventory_data[index] == nullptr ){
		return;
	}

	if( sd->inventory_data[index]->nameid != p->itemID ){
		return;
	}

	if( sd->inventory_data[index]->elv > sd->status.base_level ){
		return;
	}

	std::shared_ptr<s_item_package> package = item_package_db.find( p->itemID );

	if( package == nullptr ){
		return;
	}

	std::shared_ptr<s_item_package_group> group = util::umap_find( package->groups, p->BoxIndex );

	if( group == nullptr ){
		return;
	}

	if( pc_delitem( sd, index, 1, 0, 0, LOG_TYPE_PACKAGE ) != 0 ){
		return;
	}

	for( const auto& entry : group->items ){
		struct item item = {};

		item.nameid = entry.second->item_id;
		item.identify = 1;
		item.refine = (char)entry.second->refine;

		if( entry.second->rentalhours ){
			item.expire_time = (uint32)( time( nullptr ) + entry.second->rentalhours * 3600 );
		}

		// Check if it is a pet egg
		std::shared_ptr<s_pet_db> pet = pet_db_search( item.nameid, PET_EGG );

		if( pet != nullptr ){
			for( int i = 0; i < entry.second->amount; i++ ){
				pet_create_egg( sd, item.nameid );
			}
		}else if( entry.second->amount > 1 && ( !itemdb_isstackable( item.nameid ) || item.expire_time > 0 ) ){
			for( int i = 0; i < entry.second->amount; i++ ){
				// New random options on each iteration
				if( entry.second->randomOptionGroup != nullptr ){
					entry.second->randomOptionGroup->apply( item );
				}

				pc_additem( sd, &item, 1, LOG_TYPE_PACKAGE );
			}
		}else{
			if( entry.second->randomOptionGroup != nullptr ){
				entry.second->randomOptionGroup->apply( item );
			}

			pc_additem( sd, &item, entry.second->amount, LOG_TYPE_PACKAGE );
		}
	}
#endif
}

void clif_broadcast_refine_result(map_session_data& sd, t_itemid itemId, int8 level, bool success)
{
#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
	PACKET_ZC_BROADCAST_ITEMREFINING_RESULT p{};
	p.packetType = HEADER_ZC_BROADCAST_ITEMREFINING_RESULT;
	p.itemId = itemId;
	p.refine_level = level;
	p.status = (int8)success;

	if (battle_config.broadcast_hide_name) {
		std::string dispname = clif_hide_name(sd.status.name);
		safestrncpy(p.name, dispname.c_str(), sizeof(p.name));
	}
	else {
		safestrncpy(p.name, sd.status.name, sizeof(p.name));
	}

	clif_send(&p, sizeof(p), &sd.bl, ALL_CLIENT);
#endif
}

void clif_parse_captcha_register(int fd, map_session_data *sd) {
#if PACKETVER >= 20160316
	nullpo_retv(sd);

	if (!pc_has_permission(sd, PC_PERM_MACRO_REGISTER)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 246)); // Your GM level doesn't authorize you to perform this action.
		return;
	}

	const PACKET_CZ_REQ_UPLOAD_MACRO_DETECTOR* p = reinterpret_cast<PACKET_CZ_REQ_UPLOAD_MACRO_DETECTOR*>( RFIFOP( fd, 0 ) );

	pc_macro_captcha_register(*sd, p->imageSize, p->answer);
#endif
}

void clif_captcha_upload_request(map_session_data &sd) {
#if PACKETVER >= 20160330
	PACKET_ZC_ACK_UPLOAD_MACRO_DETECTOR p = {};

	p.PacketType = HEADER_ZC_ACK_UPLOAD_MACRO_DETECTOR;
	safestrncpy(p.captchaKey, "", sizeof(p.captchaKey));
	if (sd.captcha_upload.cd != nullptr) {
		p.captchaFlag = 0;
	} else {
		p.captchaFlag = 1;
	}

	clif_send(&p, sizeof(p), &sd.bl, SELF);
#endif
}

void clif_parse_captcha_upload(int fd, map_session_data *sd) {
#if PACKETVER >= 20160316
	nullpo_retv(sd);

	if (!pc_has_permission(sd, PC_PERM_MACRO_REGISTER)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 246)); // Your GM level doesn't authorize you to perform this action.
		return;
	}

	const PACKET_CZ_UPLOAD_MACRO_DETECTOR_CAPTCHA* p = reinterpret_cast<PACKET_CZ_UPLOAD_MACRO_DETECTOR_CAPTCHA*>( RFIFOP( fd, 0 ) );
	int16 upload_size = p->PacketLength - sizeof( * p );

	if (upload_size < 1 || upload_size > MAX_CAPTCHA_CHUNK_SIZE)
		return;

	if (sd->captcha_upload.upload_size + upload_size > sd->captcha_upload.cd->image_size)
		return;

	pc_macro_captcha_register_upload(*sd, upload_size, p->imageData);
#endif
}

void clif_captcha_upload_end(map_session_data &sd) {
#if PACKETVER >= 20160330
	PACKET_ZC_COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA p = {};

	p.PacketType = HEADER_ZC_COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA;

	clif_send(&p, sizeof(p), &sd.bl, SELF);
#endif
}

void clif_parse_captcha_preview_request(int fd, map_session_data *sd) {
#if PACKETVER >= 20160323
	nullpo_retv(sd);

	if (!(pc_has_permission(sd, PC_PERM_MACRO_REGISTER) && pc_has_permission(sd, PC_PERM_MACRO_DETECT))) {
		clif_displaymessage(sd->fd, msg_txt(sd, 246)); // Your GM level doesn't authorize you to perform this action.
		return;
	}

	const PACKET_CZ_REQ_PREVIEW_MACRO_DETECTOR* p = reinterpret_cast<PACKET_CZ_REQ_PREVIEW_MACRO_DETECTOR*>( RFIFOP( fd, 0 ) );

	clif_captcha_preview_response(*sd, captcha_db.find(p->captchaID));
#endif
}

void clif_captcha_preview_response(map_session_data &sd, std::shared_ptr<s_captcha_data> cd) {
#if PACKETVER >= 20160330
	PACKET_ZC_ACK_PREVIEW_MACRO_DETECTOR p = {};

	p.PacketType = HEADER_ZC_ACK_PREVIEW_MACRO_DETECTOR;
	safestrncpy(p.captchaKey, "", sizeof(p.captchaKey));
	if (cd == nullptr) {
		p.captchaFlag = 1;
		p.imageSize = 0;
	} else {
		p.captchaFlag = 0;
		p.imageSize = cd->image_size;
	}

	clif_send(&p, sizeof(p), &sd.bl, SELF);

	if (cd != nullptr) {
		for (uint16 offset = 0; offset < cd->image_size;) {
			uint16 chunk_size = min(cd->image_size - offset, MAX_CAPTCHA_CHUNK_SIZE);
			PACKET_ZC_PREVIEW_MACRO_DETECTOR_CAPTCHA* p2 = reinterpret_cast<PACKET_ZC_PREVIEW_MACRO_DETECTOR_CAPTCHA*>( packet_buffer );

			p2->PacketType = HEADER_ZC_PREVIEW_MACRO_DETECTOR_CAPTCHA;
			p2->PacketLength = sizeof( *p2 );
			safestrncpy(p2->captchaKey, p.captchaKey, sizeof(p2->captchaKey));
			memcpy(p2->imageData, &cd->image_data[offset], chunk_size);
			p2->PacketLength += static_cast<decltype(p2->PacketLength)>( chunk_size );

			clif_send(p2, p2->PacketLength, &sd.bl, SELF);

			offset += chunk_size;
		}
	}
#endif
}

void clif_macro_detector_request(map_session_data &sd) {
#if PACKETVER >= 20160330
	std::shared_ptr<s_captcha_data> cd = sd.macro_detect.cd;

	if (cd == nullptr) {
		return;
	}

	// Send preview initialization request to the client.
	PACKET_ZC_APPLY_MACRO_DETECTOR p = {};

	p.PacketType = HEADER_ZC_APPLY_MACRO_DETECTOR;
	p.imageSize = cd->image_size;
	safestrncpy(p.captchaKey, "", sizeof(p.captchaKey));

	clif_send(&p, sizeof(p), &sd.bl, SELF);

	for (uint16 offset = 0; offset < cd->image_size;) {
		uint16 chunk_size = min(cd->image_size - offset, MAX_CAPTCHA_CHUNK_SIZE);
		PACKET_ZC_APPLY_MACRO_DETECTOR_CAPTCHA* p2 = reinterpret_cast<PACKET_ZC_APPLY_MACRO_DETECTOR_CAPTCHA*>( packet_buffer );

		p2->PacketType = HEADER_ZC_APPLY_MACRO_DETECTOR_CAPTCHA;
		p2->PacketLength = sizeof( *p2 );
		safestrncpy(p2->captchaKey, p.captchaKey, sizeof(p2->captchaKey));
		memcpy(p2->imageData, &cd->image_data[offset], chunk_size);
		p2->PacketLength += static_cast<decltype(p2->PacketLength)>( chunk_size );

		clif_send(p2, p2->PacketLength, &sd.bl, SELF);

		offset += chunk_size;
	}
#endif
}

void clif_macro_detector_request_show(map_session_data &sd) {
#if PACKETVER >= 20160330
	PACKET_ZC_REQ_ANSWER_MACRO_DETECTOR p = {};

	p.PacketType = HEADER_ZC_REQ_ANSWER_MACRO_DETECTOR;
	p.retryCount = sd.macro_detect.retry;
	p.timeout = battle_config.macro_detection_timeout;

	clif_send(&p, sizeof(p), &sd.bl, SELF);
#endif
}

void clif_parse_macro_detector_download_ack(int fd, map_session_data *sd) {
#if PACKETVER >= 20160316
	nullpo_retv(sd);

	if (sd->macro_detect.retry != 0) {
		//const PACKET_CZ_COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA* p = reinterpret_cast<PACKET_CZ_COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA*>( RFIFOP( fd, 0 ) );

		clif_macro_detector_request_show(*sd);
	}
#endif
}

void clif_parse_macro_detector_answer(int fd, map_session_data *sd) {
#if PACKETVER >= 20160316
	nullpo_retv(sd);

	const PACKET_CZ_ACK_ANSWER_MACRO_DETECTOR* p = reinterpret_cast<PACKET_CZ_ACK_ANSWER_MACRO_DETECTOR*>( RFIFOP( fd, 0 ) );

	pc_macro_detector_process_answer(*sd, p->answer);
#endif
}

void clif_macro_detector_status(map_session_data &sd, e_macro_detect_status stype) {
#if PACKETVER >= 20160330
	PACKET_ZC_CLOSE_MACRO_DETECTOR p = {};

	p.PacketType = HEADER_ZC_CLOSE_MACRO_DETECTOR;
	p.status = stype;

	clif_send(&p, sizeof(p), &sd.bl, SELF);
#endif
}

void clif_parse_macro_reporter_select(int fd, map_session_data *sd) {
#if PACKETVER >= 20160330
	nullpo_retv(sd);

	if (!pc_has_permission(sd, PC_PERM_MACRO_DETECT)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 246)); // Your GM level doesn't authorize you to perform this action.
		return;
	}

	const PACKET_CZ_REQ_PLAYER_AID_IN_RANGE* p = reinterpret_cast<PACKET_CZ_REQ_PLAYER_AID_IN_RANGE*>( RFIFOP( fd, 0 ) );

	pc_macro_reporter_area_select(*sd, p->xPos, p->yPos, p->RadiusRange);
#endif
}

void clif_macro_reporter_select(map_session_data &sd, const std::vector<uint32> &aid_list) {
#if PACKETVER >= 20160330
	PACKET_ZC_ACK_PLAYER_AID_IN_RANGE* p = reinterpret_cast<PACKET_ZC_ACK_PLAYER_AID_IN_RANGE*>( packet_buffer );

	p->PacketType = HEADER_ZC_ACK_PLAYER_AID_IN_RANGE;
	p->PacketLength = sizeof( *p );

	for (size_t i = 0; i < aid_list.size(); i++){
		p->AID[i] = aid_list[static_cast<int32>(i)];

		p->PacketLength += static_cast<decltype(p->PacketLength)>( sizeof( p->AID[0] ) );
	}

	clif_send(p, p->PacketLength, &sd.bl, SELF);
#endif
}

void clif_parse_macro_reporter_ack(int fd, map_session_data *sd) {
#if PACKETVER >= 20160316
	nullpo_retv(sd);

	if (!pc_has_permission(sd, PC_PERM_MACRO_DETECT)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 246)); // Your GM level doesn't authorize you to perform this action.
		return;
	}

	const PACKET_CZ_REQ_APPLY_MACRO_DETECTOR* p = reinterpret_cast<PACKET_CZ_REQ_APPLY_MACRO_DETECTOR*>( RFIFOP( fd, 0 ) );
	map_session_data *tsd = map_id2sd(p->AID);

	if (tsd == nullptr) {
		clif_displaymessage(fd, msg_txt(sd, 3)); // Character not found.
		return;
	}
	if (tsd->macro_detect.retry != 0) {
		clif_macro_reporter_status(*sd, MCR_INPROGRESS);
		return;
	}
	if (captcha_db.empty()) {
		clif_macro_reporter_status(*sd, MCR_NO_DATA);
		return;
	}

	pc_macro_reporter_process(*tsd, sd->status.account_id);
	clif_macro_reporter_status(*sd, MCR_MONITORING);
#endif
}

void clif_macro_reporter_status(map_session_data &sd, e_macro_report_status stype) {
#if PACKETVER >= 20160330
	PACKET_ZC_ACK_APPLY_MACRO_DETECTOR p = {};

	p.PacketType = HEADER_ZC_ACK_APPLY_MACRO_DETECTOR;
	p.status = stype;

	clif_send(&p, sizeof(p), &sd.bl, SELF);
#endif
}

void clif_dynamicnpc_result( map_session_data& sd, e_dynamicnpc_result result ){
#if PACKETVER_MAIN_NUM >= 20140430 || PACKETVER_RE_NUM >= 20140430 || defined(PACKETVER_ZERO)
	PACKET_ZC_DYNAMICNPC_CREATE_RESULT p = {};

	p.PacketType = HEADER_ZC_DYNAMICNPC_CREATE_RESULT;
	p.result = result;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif
}

void clif_partybooking_ask( map_session_data* sd, map_session_data* joining_sd ){
#if PACKETVER >= 20191204
	PACKET_ZC_PARTY_REQ_MASTER_TO_JOIN p = {};

	p.packetType = HEADER_ZC_PARTY_REQ_MASTER_TO_JOIN;
	p.CID = joining_sd->status.char_id;
	p.AID = joining_sd->status.account_id;
	safestrncpy( p.name, joining_sd->status.name, NAME_LENGTH );
	p.x = joining_sd->status.base_level;
	p.y = joining_sd->status.class_;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_partybooking_join( int fd, map_session_data* sd ){
#if PACKETVER >= 20191204
	const PACKET_CZ_PARTY_REQ_MASTER_TO_JOIN* p = reinterpret_cast<PACKET_CZ_PARTY_REQ_MASTER_TO_JOIN*>( RFIFOP( fd, 0 ) );

	// Character is already in a party
	if( sd->status.party_id != 0 ){
		return;
	}

	map_session_data* tsd = map_charid2sd( p->CID );

	// Target player is offline
	if( tsd == nullptr ){
		return;
	}

	if( tsd->status.account_id != p->AID ){
		return;
	}

	struct s_party_booking_requirement requirement;

	if( !party_booking_load( tsd->status.account_id, tsd->status.char_id, &requirement ) ){
		return;
	}

	if( sd->status.base_level < requirement.minimum_level ){
		return;
	}

	if( sd->status.base_level > requirement.maximum_level ){
		return;
	}

	// Already requested to join this party
	if( util::vector_exists( tsd->party_booking_requests, sd->status.char_id ) ){
		return;
	}

	// Store information that the player tried to join the party
	tsd->party_booking_requests.push_back( sd->status.char_id );

	clif_partybooking_ask( tsd, sd );
#endif
}

void clif_partybooking_reply( map_session_data* sd, map_session_data* party_leader_sd, bool accepted ){
#if PACKETVER >= 20191204
	PACKET_ZC_PARTY_JOIN_REQ_ACK_FROM_MASTER p = {};

	if( party_leader_sd->status.party_id == 0 ){
		return;
	}

	struct party_data* party = party_search( party_leader_sd->status.party_id );

	if( party == nullptr ){
		return;
	}

	p.packetType = HEADER_ZC_PARTY_JOIN_REQ_ACK_FROM_MASTER;
	safestrncpy( p.player_name, party_leader_sd->status.name, NAME_LENGTH );
	safestrncpy( p.party_name, party->party.name, NAME_LENGTH );
	p.AID = party_leader_sd->status.account_id;
	p.refused = !accepted;

	clif_send( &p, sizeof( p ), &sd->bl, SELF );
#endif
}

void clif_parse_partybooking_reply( int fd, map_session_data* sd ){
#if PACKETVER >= 20191204
	const PACKET_CZ_PARTY_REQ_ACK_MASTER_TO_JOIN* p = reinterpret_cast<PACKET_CZ_PARTY_REQ_ACK_MASTER_TO_JOIN*>( RFIFOP( fd, 0 ) );

	map_session_data* tsd = map_charid2sd( p->CID );

	// Target player is offline
	if( tsd == nullptr ){
		return;
	}

	if( tsd->status.account_id != p->AID ){
		return;
	}

	// Check if the player even requested to join the party
	if( !util::vector_exists( sd->party_booking_requests, p->CID ) ){
		return;
	}

	util::vector_erase_if_exists( sd->party_booking_requests, p->CID );

	// Only party leaders can reply
	if( !party_isleader( sd ) ){
		return;
	}

	struct party_data* party = party_search( sd->status.party_id );

	if( party == nullptr ){
		return;
	}

	if( party->instance_id > 0 && battle_config.instance_block_invite ){
		return;
	}

	if( p->accept ){
		party_join( *tsd, sd->status.party_id );
	}

	clif_partybooking_reply( tsd, sd, p->accept );
#endif
}

void clif_parse_reset_skill( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20220203
	const PACKET_CZ_RESET_SKILL* p = reinterpret_cast<PACKET_CZ_RESET_SKILL*>( RFIFOP( fd, 0 ) );
#endif
}

void clif_set_dialog_align(map_session_data& sd, int npcid, e_say_dialog_align align)
{
#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	PACKET_ZC_DIALOG_TEXT_ALIGN p = {};

	p.PacketType = HEADER_ZC_DIALOG_TEXT_ALIGN;
	p.align = align;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif  // PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
}

void clif_set_npc_window_size(map_session_data& sd, int width, int height)
{
#if PACKETVER_MAIN_NUM >= 20220504
	PACKET_ZC_DIALOG_WINDOW_SIZE p = {};

	p.PacketType = HEADER_ZC_DIALOG_WINDOW_SIZE;
	p.width = width;
	p.height = height;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif  // PACKETVER_MAIN_NUM >= 20220504
}

void clif_goldpc_info( map_session_data& sd ){
#if PACKETVER_MAIN_NUM >= 20140508 || PACKETVER_RE_NUM >= 20140508 || defined(PACKETVER_ZERO)
	const static int32 client_max_seconds = 3600;

	if( battle_config.feature_goldpc_active ){
		struct PACKET_ZC_GOLDPCCAFE_POINT p = {};

		p.PacketType = HEADER_ZC_GOLDPCCAFE_POINT;
		p.isActive = true;
		if( battle_config.feature_goldpc_vip && pc_isvip( &sd ) ){
			p.mode = 2;
		}else{
			p.mode = 1;
		}
		p.point = (int32)pc_readparam( &sd, SP_GOLDPC_POINTS );
		if( sd.goldpc_tid != INVALID_TIMER ){
			const struct TimerData* td = get_timer( sd.goldpc_tid );

			if( td != nullptr ){
				// Get the remaining milliseconds until the next reward
				t_tick remaining = td->tick - gettick();

				// Always round up to full second
				remaining += ( remaining % 1000 );

				p.playedTime = (int32)( client_max_seconds - ( remaining / 1000 ) );
			}else{
				p.playedTime = 0;
			}
		}else{
			p.playedTime = client_max_seconds;
		}

		clif_send( &p, sizeof( p ), &sd.bl, SELF );
	}
#endif
}

void clif_parse_dynamic_npc( int fd, map_session_data* sd ){
#if PACKETVER_MAIN_NUM >= 20140430 || PACKETVER_RE_NUM >= 20140430 || defined(PACKETVER_ZERO)
	struct PACKET_CZ_DYNAMICNPC_CREATE_REQUEST* p = (struct PACKET_CZ_DYNAMICNPC_CREATE_REQUEST*)RFIFOP( fd, 0 );

	char npcname[NPC_NAME_LENGTH + 1];

	if( strncasecmp( "GOLDPCCAFE", p->name, sizeof( p->name ) ) == 0 ){
		safestrncpy( npcname, p->name, sizeof( npcname ) );
	}else{
		return;
	}

	struct npc_data* nd = npc_name2id( npcname );

	if( nd == nullptr ){
		ShowError( "clif_parse_dynamic_npc: Original NPC \"%s\" was not found.\n", npcname );
		clif_dynamicnpc_result( *sd, DYNAMICNPC_RESULT_UNKNOWNNPC );
		return;
	}

	if( nd ){
		if( battle_config.feature_goldpc_script ){
			run_script( nd->u.scr.script, 0, sd->bl.id, nd->bl.id );
			return;
		}else{
			if( npc_duplicate_npc_for_player( *nd, *sd ) != nullptr ){
				clif_dynamicnpc_result( *sd, DYNAMICNPC_RESULT_SUCCESS );
			}
		}
	}
#endif
}

void clif_set_npc_window_pos(map_session_data& sd, int x, int y)
{
#if PACKETVER_MAIN_NUM >= 20220504
	PACKET_ZC_DIALOG_WINDOW_POS p = {};

	p.PacketType = HEADER_ZC_DIALOG_WINDOW_POS;
	p.x = x;
	p.y = y;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif  // PACKETVER_MAIN_NUM >= 20220504
}

void clif_set_npc_window_pos_percent(map_session_data& sd, int x, int y)
{
#if PACKETVER_MAIN_NUM >= 20220504
	PACKET_ZC_DIALOG_WINDOW_POS2 p = {};

	p.PacketType = HEADER_ZC_DIALOG_WINDOW_POS2;
	p.x = x;
	p.y = y;

	clif_send( &p, sizeof( p ), &sd.bl, SELF );
#endif  // PACKETVER_MAIN_NUM >= 20220504
}

/// Displays a special popup.
/// Works only if player moved from one map to another.
/// 0bbe <popup id>.L (ZC_SPECIALPOPUP)
void clif_specialpopup(map_session_data& sd, int32 id ){
#if PACKETVER >= 20221005
	PACKET_ZC_SPECIALPOPUP p = {};

	p.PacketType = HEADER_ZC_SPECIALPOPUP;
	p.ppId = id;

	clif_send( &p, sizeof( p ), &sd.bl, SELF);
#endif
}

/*==========================================
 * Main client packet processing function
 *------------------------------------------*/
static int clif_parse(int fd)
{
	int cmd, packet_len;
	TBL_PC* sd;
	int pnum;
#ifdef PACKET_OBFUSCATION
	int cmd2;
#endif

	//TODO apply delays or disconnect based on packet throughput [FlavioJS]
	// Note: "click masters" can do 80+ clicks in 10 seconds

	for( pnum = 0; pnum < 3; ++pnum )// Limit max packets per cycle to 3 (delay packet spammers) [FlavioJS]  -- This actually aids packet spammers, but stuff like /str+ gets slow without it [Ai4rei]
	{ // begin main client packet processing loop

	sd = (TBL_PC *)session[fd]->session_data;

	if (session[fd]->flag.eof) {
		if (sd) {
			if (sd->state.autotrade) {
				//Disassociate character from the socket connection.
				session[fd]->session_data = nullptr;
				sd->fd = 0;
				ShowInfo("Character '" CL_WHITE "%s" CL_RESET "' logged off (using @autotrade).\n", sd->status.name);
			} else
			if (sd->state.active) {
				// Player logout display [Valaris]
				ShowInfo("Character '" CL_WHITE "%s" CL_RESET "' logged off.\n", sd->status.name);
				clif_quitsave(fd, sd);
			} else {
				//Unusual logout (during log on/off/map-changer procedure)
				ShowInfo("Player AID:%d/CID:%d logged off.\n", sd->status.account_id, sd->status.char_id);
				map_quit(sd);
			}
		} else {
			ShowInfo("Closed connection from '" CL_WHITE "%s" CL_RESET "'.\n", ip2str(session[fd]->client_addr, nullptr));
		}
		do_close(fd);
		return 0;
	}

	if (RFIFOREST(fd) < 2)
		return 0;

	cmd = RFIFOW(fd, 0);

#ifdef PACKET_OBFUSCATION
	// Check if it is a player that tries to connect to the map server.
	if( sd ){
		// Decrypt the current packet ID with the last key stored in the session.
		cmd = (cmd ^ ((sd->cryptKey >> 16) & 0x7FFF));
	}else{
		// Store the original value for checking
		cmd2 = cmd;

		// A player tries to connect - use the initial keys for the decryption of the packet ID.
		cmd = (cmd ^ ((((clif_cryptKey[0] * clif_cryptKey[1]) + clif_cryptKey[2]) >> 16) & 0x7FFF));
	}
#endif

	// filter out invalid / unsupported packets
	if (cmd > MAX_PACKET_DB || cmd < MIN_PACKET_DB || packet_db[cmd].len == 0) {
		ShowWarning("clif_parse: Received unsupported packet (packet 0x%04x, %d bytes received), disconnecting session #%d.\n", cmd, RFIFOREST(fd), fd);

#ifdef DUMP_INVALID_PACKET
		ShowDump(RFIFOP(fd,0), RFIFOREST(fd));
#endif

#ifdef PACKET_OBFUSCATION_WARN
		// If it is a connection attempt, check if the packet would have been valid without decrypting it
		if( !sd ){
#ifdef PACKET_OBFUSCATION
			if( cmd2 >= MIN_PACKET_DB && cmd2 < MAX_PACKET_DB && packet_db[cmd2].len != 0 && packet_db[cmd2].func == clif_parse_WantToConnection ){
				ShowWarning( "clif_parse: It looks like you have enabled PACKET_OBFUSCATION on server side, but disabled it on client side.\n" );
			}
#else
			// Try to use the initial keys for the decryption of the packet ID.
			cmd = (cmd ^ ((((clif_cryptKey[0] * clif_cryptKey[1]) + clif_cryptKey[2]) >> 16) & 0x7FFF));

			if( cmd >= MIN_PACKET_DB && cmd < MAX_PACKET_DB && packet_db[cmd].len != 0 && packet_db[cmd].func == clif_parse_WantToConnection ){
				ShowWarning( "clif_parse: It looks like you have disabled PACKET_OBFUSCATION on server side, but enabled it on client side.\n" );
			}
#endif
		}
#endif

		set_eof(fd);
		return 0;
	}

	// determine real packet length
	packet_len = packet_db[cmd].len;
	if (packet_len == -1) { // variable-length packet
		if (RFIFOREST(fd) < 4)
			return 0;

		packet_len = RFIFOW(fd,2);
		if (packet_len < 4 || packet_len > 32768) {
			ShowWarning("clif_parse: Received packet 0x%04x specifies invalid packet_len (%d), disconnecting session #%d.\n", cmd, packet_len, fd);
#ifdef DUMP_INVALID_PACKET
			ShowDump(RFIFOP(fd,0), RFIFOREST(fd));
#endif
			set_eof(fd);
			return 0;
		}
	}
	if ((int)RFIFOREST(fd) < packet_len){
		ShowWarning( "clif_parse: Received packet 0x%04x with expected packet length %d, but only %d bytes remaining, disconnecting session #%d.\n", cmd, packet_len, RFIFOREST( fd ), fd );
#ifdef DUMP_INVALID_PACKET
		ShowDump( RFIFOP( fd, 0 ), RFIFOREST( fd ) );
#endif
		set_eof( fd );
		return 0; // not enough data received to form the packet
	}

#ifdef PACKET_OBFUSCATION
	RFIFOW(fd, 0) = cmd;
	if (sd)
		sd->cryptKey = ((sd->cryptKey * clif_cryptKey[1]) + clif_cryptKey[2]) & 0xFFFFFFFF; // Update key for the next packet
#endif

	if( packet_db[cmd].func == clif_parse_debug )
		packet_db[cmd].func(fd, sd);
	else if( packet_db[cmd].func != nullptr ) {
		if( !sd && packet_db[cmd].func != clif_parse_WantToConnection )
			; //Only valid packet when there is no session
		else
		if( sd && sd->bl.prev == nullptr && packet_db[cmd].func != clif_parse_LoadEndAck )
			; //Only valid packet when player is not on a map
		else
			packet_db[cmd].func(fd, sd);
	}
#ifdef DUMP_UNKNOWN_PACKET
	else DumpUnknown(fd,sd,cmd,packet_len);
#endif
	RFIFOSKIP(fd, packet_len);
	}; // main loop end

	return 0;
}

void packetdb_addpacket( uint16 cmd, uint16 length, void (*func)(int, map_session_data *), ... ){
	va_list argp;
	int i;

	if(cmd <= 0 || cmd > MAX_PACKET_DB)
		return;

	packet_db[cmd].len = length;
	packet_db[cmd].func = func;

	va_start(argp, func);

	for( i = 0; i < MAX_PACKET_POS; i++ ){
		int offset = va_arg(argp, int);

		if( offset == 0 ){
			break;
		}

		packet_db[cmd].pos[i] = offset;
	}

	if( i == MAX_PACKET_POS ){
		ShowError( "Too many positions found for packet 0x%04x (max=%d).\n", cmd, MAX_PACKET_POS );
	}

	va_end(argp);
}

/*==========================================
 * Reads packets and setups its array reference
 *------------------------------------------*/
void packetdb_readdb(){
	memset(packet_db,0,sizeof(packet_db));

#include "clif_packetdb.hpp"
#include "clif_shuffle.hpp"

	ShowStatus("Using packet version: " CL_WHITE "%d" CL_RESET ".\n", PACKETVER);

#ifdef PACKET_OBFUSCATION
	ShowStatus("Packet Obfuscation: " CL_GREEN "Enabled" CL_RESET ". Keys: " CL_WHITE "0x%08X, 0x%08X, 0x%08X" CL_RESET "\n", clif_cryptKey[0], clif_cryptKey[1], clif_cryptKey[2]);
#else
	ShowStatus("Packet Obfuscation: " CL_RED "Disabled" CL_RESET ".\n");
#endif
}

/*==========================================
 *
 *------------------------------------------*/
void do_init_clif(void) {
	const int colors[COLOR_MAX] = {
		0x00FF00, // COLOR_DEFAULT
		0xFF0000, // COLOR_RED
		0xFFFFFF, // COLOR_WHITE
		0xFFFF00, // COLOR_YELLOW
		0x00FFFF, // COLOR_CYAN
		0xB5FFB5, // COLOR_LIGHT_GREEN
		0xFFFF63, // COLOR_LIGHT_YELLOW
	};

	/**
	 * Setup Color Table (saves unnecessary load of strtoul on every call)
	 **/
	for( int i = 0; i < COLOR_MAX; i++ ){
		color_table[i] = ( colors[i] & 0x0000FF ) << 16 | ( colors[i] & 0x00FF00 ) | ( colors[i] & 0xFF0000 ) >> 16; //RGB to BGR
	}

	packetdb_readdb();

	set_defaultparse(clif_parse);
	if( make_listen_bind(bind_ip,map_port) == -1 ) {
		ShowFatalError("Failed to bind to port '" CL_WHITE "%d" CL_RESET "'\n",map_port);
		exit(EXIT_FAILURE);
	}

	add_timer_func_list(clif_clearunit_delayed_sub, "clif_clearunit_delayed_sub");
	add_timer_func_list(clif_delayquit, "clif_delayquit");

#if PACKETVER_MAIN_NUM >= 20190403 || PACKETVER_RE_NUM >= 20190320
	add_timer_func_list( clif_ping_timer, "clif_ping_timer" );

	if( battle_config.ping_timer_interval > 0 ){
		add_timer_interval( gettick() + battle_config.ping_timer_interval * 1000, clif_ping_timer, 0, 0, battle_config.ping_timer_interval * 1000 );	
	}
#endif

	delay_clearunit_ers = ers_new(sizeof(struct block_list),"clif.cpp::delay_clearunit_ers",ERS_OPT_CLEAR);
}

void do_final_clif(void) {
	ers_destroy(delay_clearunit_ers);
}
