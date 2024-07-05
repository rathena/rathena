// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "guild.hpp"

#include <cstdlib>
#include <memory>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/ers.hpp>
#include <common/malloc.hpp>
#include <common/mapindex.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp> // session_isActive
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "battle.hpp"
#include "channel.hpp"
#include "clif.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "pc.hpp"
#include "storage.hpp"
#include "trade.hpp"

using namespace rathena;

std::unordered_map<int, std::shared_ptr<MapGuild>> guild_db;
CastleDatabase castle_db;

static DBMap* guild_expcache_db; // uint32 char_id -> struct guild_expcache*
static DBMap* guild_infoevent_db; // int guild_id -> struct eventlist*

struct eventlist {
	char name[EVENT_NAME_LENGTH];
	struct eventlist *next;
};

//Constant related to the flash of the Guild EXP cache
#define GUILD_SEND_XY_INTERVAL	5000 // Interval of sending coordinates and HP
#define GUILD_PAYEXP_INTERVAL 10000 //Interval (maximum survival time of the cache, in milliseconds)
#define GUILD_PAYEXP_LIST 8192 //The maximum number of cache

//Guild EXP cache
struct guild_expcache {
	int guild_id, account_id, char_id;
	t_exp exp;
};
static struct eri *expcache_ers; //For handling of guild exp payment.

struct s_guild_skill_requirement{
	uint16 id;
	uint16 lv;
};

struct s_guild_skill_tree{
	uint16 id;
	uint16 max;
	std::unordered_map<uint16,std::shared_ptr<s_guild_skill_requirement>> need;
};

class GuildSkillTreeDatabase : public TypesafeYamlDatabase<uint16, s_guild_skill_tree>{
public:
	GuildSkillTreeDatabase() : TypesafeYamlDatabase( "GUILD_SKILL_TREE_DB", 1 ){

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode( const ryml::NodeRef& node ) override;
};

const std::string GuildSkillTreeDatabase::getDefaultLocation(){
	return std::string(db_path) + "/guild_skill_tree.yml";
}

uint64 GuildSkillTreeDatabase::parseBodyNode( const ryml::NodeRef& node ){
	std::string name;

	if( !this->asString( node, "Id", name ) ){
		return 0;
	}

	uint16 skill_id;

	if( !( skill_id = skill_name2id( name.c_str() ) ) ){
		this->invalidWarning( node["Id"], "Invalid guild skill name \"%s\", skipping.\n", name.c_str() );
		return 0;
	}

	if( !SKILL_CHK_GUILD( skill_id ) ){
		this->invalidWarning( node["Id"], "Guild skill \"%s\" with Id %u is out of the guild skill range [%u-%u], skipping.\n", name.c_str(), skill_id, GD_SKILLBASE, GD_MAX );
		return 0;
	}

	std::shared_ptr<s_guild_skill_tree> skill = this->find( skill_id );
	bool exists = skill != nullptr;

	if( !exists ){
		if( !this->nodeExists( node, "MaxLevel" ) ){
			this->invalidWarning( node, "Missing node \"MaxLevel\", skipping.\n" );
			return 0;
		}

		skill = std::make_shared<s_guild_skill_tree>();
		skill->id = skill_id;
	}

	if( this->nodeExists( node, "MaxLevel" ) ){
		uint16 level;

		if( !this->asUInt16( node, "MaxLevel", level ) ){
			return 0;
		}

		// Enable Guild's Glory when required for emblems
		if( skill_id == GD_GLORYGUILD && battle_config.require_glory_guild && level == 0 ){
			level = 1;
		}

		skill->max = level;
	}

	if( this->nodeExists( node, "Required" ) ){
		const auto& reqNode = node["Required"];
		for( const auto&  requiredNode : reqNode ){
			std::string requiredName;

			if( !this->asString( requiredNode, "Id", requiredName ) ){
				return 0;
			}

			uint16 requiredSkillId;

			if( !( requiredSkillId = skill_name2id( requiredName.c_str() ) ) ){
				this->invalidWarning( requiredNode["Id"], "Invalid required guild skill name \"%s\", skipping.\n", requiredName.c_str() );
				return 0;
			}

			if( !SKILL_CHK_GUILD( requiredSkillId ) ){
				this->invalidWarning( requiredNode["Id"], "Required guild skill \"%s\" with Id %u is out of the guild skill range [%u-%u], skipping.\n", requiredName.c_str(), requiredSkillId, GD_SKILLBASE, GD_MAX );
				return 0;
			}

			std::shared_ptr<s_guild_skill_requirement> requirement = util::umap_find( skill->need, requiredSkillId );
			bool requirement_exists = requirement != nullptr;

			if( !requirement_exists ){
				if( !this->nodeExists( requiredNode, "Level" ) ){
					this->invalidWarning( requiredNode, "Missing node \"Level\", skipping.\n" );
					return 0;
				}

				requirement = std::make_shared<s_guild_skill_requirement>();
				requirement->id = requiredSkillId;
			}

			if( this->nodeExists( requiredNode, "Level" ) ){
				uint16 requiredLevel;

				if( !this->asUInt16( requiredNode, "Level", requiredLevel ) ){
					return 0;
				}

				if( requiredLevel == 0 ){
					continue;
				}

				requirement->lv = requiredLevel;
			}

			if( !requirement_exists ){
				skill->need[requiredSkillId] = requirement;
			}
		}
	}

	if( !exists ){
		this->put( skill_id, skill );
	}

	return 1;
}

GuildSkillTreeDatabase guild_skill_tree_db;

TIMER_FUNC(guild_payexp_timer);
static TIMER_FUNC(guild_send_xy_timer);

/* guild flags cache */
struct npc_data **guild_flags;
unsigned short guild_flags_count;

/**
 * Get guild skill index in guild structure of mmo.hpp
 * @param skill_id
 * @return Index in skill_tree or -1
 **/
static short guild_skill_get_index(uint16 skill_id) {
	if (!SKILL_CHK_GUILD(skill_id))
		return -1;
	skill_id -= GD_SKILLBASE;
	if (skill_id >= MAX_GUILDSKILL)
		return -1;
	return skill_id;
}

/*==========================================
 * Retrieves and validates the sd pointer for this guild member [Skotlex]
 *------------------------------------------*/
static TBL_PC* guild_sd_check(int guild_id, uint32 account_id, uint32 char_id) {
	TBL_PC* sd = map_id2sd(account_id);

	if (!(sd && sd->status.char_id == char_id))
		return nullptr;

	if (sd->status.guild_id != guild_id)
	{	//If player belongs to a different guild, kick him out.
		intif_guild_leave(guild_id,account_id,char_id,0,"** Guild Mismatch **");
		return nullptr;
	}

	return sd;
}

// Modified [Komurka]
uint16 guild_skill_get_max( uint16 id ){
	std::shared_ptr<s_guild_skill_tree> skill = guild_skill_tree_db.find( id );

	if( skill == nullptr ){
		return 0;
	}

	return skill->max;
}

// Retrieve skill_lv learned by guild
int guild_checkskill(const struct mmo_guild &g, int id) {
	if ((id = guild_skill_get_index(id)) < 0)
		return 0;
	return g.skill[id].lv;
}

/*==========================================
 * Guild skill check - from jA [Komurka]
 *------------------------------------------*/
bool guild_check_skill_require(const struct mmo_guild &g, uint16 id ){
	std::shared_ptr<s_guild_skill_tree> skill = guild_skill_tree_db.find( id );

	if( skill == nullptr ){
		return false;
	}

	for( const auto& pair : skill->need ){
		if( pair.second->lv > guild_checkskill(g, pair.second->id ) ){
			return false;
		}
	}

	return true;
}

const std::string CastleDatabase::getDefaultLocation() {
	return std::string(db_path) + "/castle_db.yml";
}

uint64 CastleDatabase::parseBodyNode(const ryml::NodeRef& node) {
	int32 castle_id;

	if (!this->asInt32(node, "Id", castle_id))
		return 0;

	std::shared_ptr<guild_castle> gc = this->find(castle_id);
	bool exists = gc != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Map", "Name", "Npc" }))
			return 0;

		gc = std::make_shared<guild_castle>();
		gc->castle_id = castle_id;
	}

	if (this->nodeExists(node, "Map")) {
		std::string map_name;

		if (!this->asString(node, "Map", map_name))
			return 0;

		uint16 mapindex = mapindex_name2idx(map_name.c_str(), nullptr);

		if( mapindex == 0 ){
			this->invalidWarning(node["Map"], "Map %s doesn't exist, skipping.\n", map_name.c_str());
			return 0;
		}

		gc->mapindex = mapindex;
	}

	if (this->nodeExists(node, "Name")) {
		std::string castle_name;

		if (!this->asString(node, "Name", castle_name))
			return 0;

		safestrncpy(gc->castle_name, castle_name.c_str(), sizeof(gc->castle_name));
	}

	if (this->nodeExists(node, "Npc")) {
		std::string npc_name;

		if (!this->asString(node, "Npc", npc_name))
			return 0;

		if (npc_name.size() > NPC_NAME_LENGTH) {
			this->invalidWarning(node["NPC"], "Npc name %s too long, skipping.\n", npc_name.c_str());
			return 0;
		}

		safestrncpy(gc->castle_event, npc_name.c_str(), sizeof(gc->castle_event));
	}

	if( this->nodeExists( node, "Type" ) ){
		std::string type;

		if( !this->asString( node, "Type", type ) ){
			return 0;
		}

		std::string type_constant = "WOE_" + type;
		int64 constant;

		if( !script_get_constant( type_constant.c_str(), &constant ) || constant < WOE_FIRST_EDITION || constant >= WOE_MAX ){
			this->invalidWarning( node["Type"], "Invalid WoE type %s.\n", type.c_str() );
			return 0;
		}

		gc->type = static_cast<e_woe_type>( constant );
	}else{
		if( !exists ){
			gc->type = WOE_FIRST_EDITION;
		}
	}

	if( this->nodeExists( node, "ClientId" ) ){
		uint16 id;

		if( !this->asUInt16( node, "ClientId", id ) ){
			return 0;
		}

		gc->client_id = id;
	}else{
		if( !exists ){
			gc->client_id = 0;
		}
	}

	if( this->nodeExists( node, "WarpEnabled" ) ){
		bool enabled;

		if( !this->asBool( node, "WarpEnabled", enabled ) ){
			return 0;
		}

		gc->warp_enabled = enabled;
	}else{
		if( !exists ){
			gc->warp_enabled = false;
		}
	}

	if (this->nodeExists(node, "WarpX")) {
		uint16 warp_x;

		if (!this->asUInt16(node, "WarpX", warp_x)) {
			return 0;
		}

		if( warp_x == 0 ){
			this->invalidWarning( node["WarpX"], "WarpX has to be greater than zero.\n" );
			return 0;
		}

		map_data* md = map_getmapdata( map_mapindex2mapid( gc->mapindex ) );

		// If the map is on another map-server, we cannot verify the bounds
		if( md != nullptr && warp_x >= md->xs ){
			this->invalidWarning( node["WarpX"], "WarpX has to be smaller than %hu.\n", md->xs );
			return 0;
		}

		gc->warp_x = warp_x;
	}
	else {
		if (!exists)
			gc->warp_x = 0;
	}

	if (this->nodeExists(node, "WarpY")) {
		uint16 warp_y;

		if (!this->asUInt16(node, "WarpY", warp_y)) {
			return 0;
		}

		if( warp_y == 0 ){
			this->invalidWarning( node["WarpY"], "WarpY has to be greater than zero.\n" );
			return 0;
		}

		map_data* md = map_getmapdata( map_mapindex2mapid( gc->mapindex ) );

		// If the map is on another map-server, we cannot verify the bounds
		if( md != nullptr && warp_y >= md->ys ){
			this->invalidWarning( node["WarpY"], "WarpY has to be smaller than %hu.\n", md->ys );
			return 0;
		}

		gc->warp_y = warp_y;
	}
	else {
		if (!exists)
			gc->warp_y = 0;
	}

	if (this->nodeExists(node, "WarpCost")) {
		uint32 zeny;

		if (!this->asUInt32(node, "WarpCost", zeny)) {
			return 0;
		}

		if( zeny > MAX_ZENY ){
			this->invalidWarning( node["WarpCost"], "WarpCost has to be smaller than %d.\n", MAX_ZENY );
			return 0;
		}

		gc->zeny = zeny;
	} else {
		if (!exists)
			gc->zeny = 100;
	}

	if (this->nodeExists(node, "WarpCostSiege")) {
		uint32 zeny_siege;

		if (!this->asUInt32(node, "WarpCostSiege", zeny_siege)) {
			return 0;
		}

		if( zeny_siege > MAX_ZENY ){
			this->invalidWarning( node["WarpCostSiege"], "WarpCostSiege has to be smaller than %d.\n", MAX_ZENY );
			return 0;
		}

		gc->zeny_siege = zeny_siege;
	}
	else {
		if (!exists)
			gc->zeny_siege = 100000;
	}

	if (!exists)
		this->put(castle_id, gc);

	return 1;
}

void CastleDatabase::loadingFinished(){
	for( const auto& pair : *this ){
		std::shared_ptr<guild_castle> castle = pair.second;

		if( castle->client_id != 0 ){
			// Check if ClientId is unique
			for( const auto& pair2 : *this ){
				std::shared_ptr<guild_castle> castle2 = pair2.second;

				if( castle->castle_id == castle2->castle_id ){
					continue;
				}

				if( castle->client_id == castle2->client_id ){
					ShowWarning( "Castle ClientId %hu is ambigous.\n", castle->client_id );
					break;
				}
			}
		}

		if( castle->warp_enabled ){
			if( castle->client_id == 0 ){
				ShowWarning( "Warping to castle %d is enabled, but no ClientId is set. Disabling...\n", castle->castle_id );
				castle->warp_enabled = false;
				continue;
			}

			if( castle->warp_x == 0 ){
				ShowWarning( "Warping to castle %d is enabled, but no WarpX is set. Disabling...\n", castle->castle_id );
				castle->warp_enabled = false;
				continue;
			}

			if( castle->warp_y == 0 ){
				ShowWarning( "Warping to castle %d is enabled, but no WarpY is set. Disabling...\n", castle->castle_id );
				castle->warp_enabled = false;
				continue;
			}
		}
	}
}

/// lookup: guild id -> guild
std::shared_ptr<MapGuild> guild_search(int guild_id) {
	return util::umap_find(guild_db, guild_id);
}

/// lookup: guild name -> guild
std::shared_ptr<MapGuild> guild_searchname(const char* str) {
	if (!str)
		return nullptr;
	for (const auto &it : guild_db) {
		if (it.second && (strcmp(it.second->guild.name, str) == 0))
			return it.second;
	}

	return nullptr;
}

/**
 * Helper function to find a guild via a string
 * The string might be a guild_id, so test names first then id
*/
std::shared_ptr<MapGuild> guild_searchnameid(const char *str) {
	if (!str)
		return nullptr;
	
	auto g = guild_searchname(str);
	if (g)
		return g;
	
	return guild_search(atoi(str));
}

/// lookup: map index -> castle*
std::shared_ptr<guild_castle> CastleDatabase::mapindex2gc(int16 mapindex) {
	for (const auto &it : *this) {
		if (it.second->mapindex == mapindex)
			return it.second;
	}
	return nullptr;
}

/// lookup: map name -> castle*
std::shared_ptr<guild_castle> CastleDatabase::mapname2gc(const char* mapname) {
	return castle_db.mapindex2gc(mapindex_name2id(mapname));
}

std::shared_ptr<guild_castle> CastleDatabase::find_by_clientid( uint16 client_id ){
	for( const auto &it : *this ){
		if( it.second->client_id == client_id ){
			return it.second;
		}
	}

	return nullptr;
}

map_session_data* guild_getavailablesd(const struct mmo_guild &g) {
	int i;

	ARR_FIND( 0, g.max_member, i, g.member[i].sd != nullptr );
	return( i < g.max_member ) ? g.member[i].sd : nullptr;
}

/// lookup: player AID/CID -> member index
int guild_getindex(const struct mmo_guild &g, uint32 account_id, uint32 char_id) {
	int i;

	ARR_FIND( 0, g.max_member, i, g.member[i].account_id == account_id && g.member[i].char_id == char_id );
	return( i < g.max_member ) ? i : -1;
}

/// lookup: player sd -> member position
int guild_getposition(const map_session_data& sd) {
	int i;

	if (!sd.guild)
		return -1;
	
	const auto &g = sd.guild->guild;

	ARR_FIND( 0, g.max_member, i, g.member[i].account_id == sd.status.account_id && g.member[i].char_id == sd.status.char_id );
	return( i < g.max_member ) ? g.member[i].position : -1;
}

//Creation of member information
void guild_makemember( struct guild_member& m, map_session_data& sd ){
	m.account_id = sd.status.account_id;
	m.char_id = sd.status.char_id;
	m.hair = sd.status.hair;
	m.hair_color = sd.status.hair_color;
	m.gender = sd.status.sex;
	m.class_ = sd.status.class_;
	m.lv = sd.status.base_level;
	m.exp = 0;
	m.online = 1;
	m.position = MAX_GUILDPOSITION - 1;
	safestrncpy( m.name, sd.status.name, NAME_LENGTH );
	m.last_login = static_cast<decltype(m.last_login)>( time( nullptr ) );
}

/**
 * Server cache to be flushed to inter the Guild EXP
 * @see DBApply
 */
int guild_payexp_timer_sub(DBKey key, DBData *data, va_list ap) {
	int i;
	struct guild_expcache *c;

	c = (struct guild_expcache *)db_data2ptr(data);

	auto g = guild_search(c->guild_id);

	if (!g || (i = guild_getindex(g->guild, c->account_id, c->char_id)) < 0) {
		ers_free(expcache_ers, c);
		return 0;
	}

	g->guild.member[i].exp = util::safe_addition_cap(g->guild.member[i].exp, c->exp, MAX_GUILD_EXP);

	intif_guild_change_memberinfo(g->guild.guild_id,c->account_id,c->char_id,
		GMI_EXP,&g->guild.member[i].exp,sizeof(g->guild.member[i].exp));
	c->exp=0;

	ers_free(expcache_ers, c);
	return 0;
}

TIMER_FUNC(guild_payexp_timer){
	guild_expcache_db->clear(guild_expcache_db,guild_payexp_timer_sub);
	return 0;
}

/**
 * Send xy coords to a guild
 * @param g Reference to a guild
 */
int guild_send_xy_timer_sub(const struct mmo_guild& g) {
	if (g.connect_member <= 0) {
		// no members connected to this guild so do not iterate
		return 0;
	}

	for (int i = 0; i < g.max_member; i++) {
		map_session_data* sd = g.member[i].sd;
		if( sd != nullptr && sd->fd && (sd->guild_x != sd->bl.x || sd->guild_y != sd->bl.y) && !sd->bg_id ) {
			clif_guild_xy( *sd );
			sd->guild_x = sd->bl.x;
			sd->guild_y = sd->bl.y;
		}
	}
	return 0;
}

//Code from party_send_xy_timer [Skotlex]
static TIMER_FUNC(guild_send_xy_timer){
	for (const auto &it : guild_db) {
		if (it.second)
			guild_send_xy_timer_sub(it.second->guild);
	}
	return 0;
}

int guild_send_dot_remove(map_session_data *sd) {
	if (sd->status.guild_id)
		clif_guild_xy_remove( *sd );
	return 0;
}
//------------------------------------------------------------------------

bool guild_create( map_session_data& sd, const char* name ){
	char tname[NAME_LENGTH];

	safestrncpy(tname, name, NAME_LENGTH);
	trim(tname);

	// empty name
	if( !tname[0] ){
		return false;
	}

	if( sd.status.guild_id ) {
		// already in a guild
		clif_guild_created( sd, 1 );
		return false;
	}

	if( battle_config.guild_emperium_check && pc_search_inventory( &sd, ITEMID_EMPERIUM ) == -1 ){
		// item required
		clif_guild_created( sd, 3 );
		return false;
	}

	struct guild_member m = {};

	guild_makemember( m, sd );
	m.position=0;
	intif_guild_create(name,&m);

	return true;
}

//Whether or not to create guild
int guild_created(uint32 account_id,int guild_id) {
	map_session_data *sd=map_id2sd(account_id);

	if(sd==nullptr)
		return 0;
	if(!guild_id) {
		clif_guild_created( *sd, 2 ); // Creation failure (presence of the same name Guild)
		return 0;
	}

	sd->status.guild_id = guild_id;
	clif_guild_created( *sd, 0 );

	if(battle_config.guild_emperium_check){
		int index = pc_search_inventory(sd,ITEMID_EMPERIUM);

		if( index > 0 )
			pc_delitem(sd,index,1,0,0,LOG_TYPE_CONSUME);	//emperium consumption
	}
	return 0;
}

//Information request
int guild_request_info(int guild_id) {
	return intif_guild_request_info(guild_id);
}

//Information request with event
int guild_npc_request_info(int guild_id,const char *event) {
	if( guild_search(guild_id) ) {
		if( event && *event )
			npc_event_doall(event);

		return 0;
	}

	if( event && *event ) {
		struct eventlist *ev;
		DBData prev;
		ev=(struct eventlist *)aCalloc(sizeof(struct eventlist),1);
		safestrncpy(ev->name,event,EVENT_NAME_LENGTH);
		//The one in the db (if present) becomes the next event from this.
		if (guild_infoevent_db->put(guild_infoevent_db, db_i2key(guild_id), db_ptr2data(ev), &prev))
			ev->next = (struct eventlist *)db_data2ptr(&prev);
	}

	return guild_request_info(guild_id);
}

/**
 * Close trade window if party member is kicked when trade a party bound item
 * @param sd
 **/
static void guild_trade_bound_cancel( map_session_data& sd ){
#ifdef BOUND_ITEMS
	if( sd.state.isBoundTrading&(1<<BOUND_GUILD))
		trade_tradecancel( &sd );
#endif
}

//Confirmation of the character belongs to guild
int guild_check_member(const struct mmo_guild &g) {
	int i;
	map_session_data *sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->status.guild_id != g.guild_id )
			continue;

		i = guild_getindex(g,sd->status.account_id,sd->status.char_id);
		if (i < 0) {
			sd->guild = nullptr;
			sd->status.guild_id=0;
			sd->guild_emblem_id=0;
			ShowWarning("guild: check_member %d[%s] is not member\n",sd->status.account_id,sd->status.name);
		}
	}
	mapit_free(iter);

	return 0;
}

//Delete association with guild_id for all characters
int guild_recv_noinfo(int guild_id) {
	map_session_data *sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->status.guild_id == guild_id )
			sd->status.guild_id = 0; // erase guild
	}
	mapit_free(iter);

	return 0;
}

//Get and display information for all member
int guild_recv_info(const struct mmo_guild &sg) {
	struct mmo_guild before;
	int i,bm,m;
	DBData data;
	map_session_data *sd;
	bool guild_new = false;

	auto g = guild_search(sg.guild_id);

	if (!g) {
		g = std::make_shared<MapGuild>();
		guild_new = true;
		guild_db.insert({sg.guild_id, g});
		before = sg;
		//Perform the check on the user because the first load
		guild_check_member(sg);
		if ((sd = map_nick2sd(sg.master,false)) != nullptr) {
#ifndef RENEWAL
			//If the guild master is online the first time the guild_info is received,
			//that means he was the first to join, so apply guild skill blocking here.
			if( battle_config.guild_skill_relog_delay )
				guild_block_skill(sd, battle_config.guild_skill_relog_delay);
#endif

			//Also set the guild master flag.
			sd->guild = g;
			sd->state.gmaster_flag = 1;
			clif_name_area(&sd->bl); // [LuzZza]
			clif_guild_masterormember(sd);
		}
	} else {
		before = g->guild;
	}
	memcpy(&g->guild, &sg, sizeof(struct mmo_guild));

	if(g->guild.max_member > MAX_GUILD) {
		ShowError("guild_recv_info: Received guild with %d members, but MAX_GUILD is only %d. Extra guild-members have been lost!\n", g->guild.max_member, MAX_GUILD);
		g->guild.max_member = MAX_GUILD;
	}

	for(i=bm=m=0;i<g->guild.max_member;i++){
		if(g->guild.member[i].account_id>0){
			sd = g->guild.member[i].sd = guild_sd_check(g->guild.guild_id, g->guild.member[i].account_id, g->guild.member[i].char_id);
			if (sd) clif_name_area(&sd->bl); // [LuzZza]
			m++;
		}else
			g->guild.member[i].sd=nullptr;
		if(before.member[i].account_id>0)
			bm++;
	}

	for (i = 0; i < g->guild.max_member; i++) { //Transmission of information at all members
		sd = g->guild.member[i].sd;
		if( sd==nullptr )
			continue;
		sd->guild = g;
		if(channel_config.ally_tmpl.name[0] && (channel_config.ally_tmpl.opt&CHAN_OPT_AUTOJOIN)) {
			channel_gjoin(sd,3); //make all member join guildchan+allieschan
		}

		if (before.guild_lv != g->guild.guild_lv || bm != m ||
			before.max_member != g->guild.max_member) {
			clif_guild_basicinfo( *sd ); //Submit basic information
			clif_guild_emblem(*sd, g->guild); //Submit emblem
		}

		if (bm != m) { //Send members information
			clif_guild_memberlist( *sd );
		}

		if (before.skill_point != g->guild.skill_point)
			clif_guild_skillinfo( *sd ); // Submit information skills

		if (guild_new) { // Send information and affiliation if unsent
			clif_guild_belonginfo( *sd );
			clif_guild_notice( *sd );
			sd->guild_emblem_id = g->guild.emblem_id;
		}
		if (g->instance_id > 0)
			instance_reqinfo(sd, g->instance_id);
	}

	//Occurrence of an event
	if (guild_infoevent_db->remove(guild_infoevent_db, db_i2key(sg.guild_id), &data)) {
		struct eventlist *ev = (struct eventlist *)db_data2ptr(&data), *ev2;
		while(ev) {
			npc_event_do(ev->name);
			ev2=ev->next;
			aFree(ev);
			ev=ev2;
		}
	}

	return 0;
}

/*=============================================
 * Player sd send a guild invatation to player tsd to join his guild
 *--------------------------------------------*/
bool guild_invite( map_session_data& sd, map_session_data* tsd ){
	// No nullpo_retr, because its valid that target players might not exist or are not online
	if( tsd == nullptr ){
		return false;
	}

	auto& g = sd.guild;

	if( g == nullptr ){
		return false;
	}

	if( g->instance_id && battle_config.instance_block_invite ){
		return false;
	}

	// Guild locked.
	if( map_getmapflag( sd.bl.m, MF_GUILDLOCK ) ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 228 ) ); // Guild modification is disabled on this map.
		return false;
	}

	// @noask [LuzZza]
	if( tsd->state.noask ){
		clif_noask_sub( sd, *tsd, 395 ); // Autorejected guild invite from %s.
		return false;
	}

	// Players in a clan can not join a guild
	if( tsd->clan ){
		// TODO: message?
		return false;
	}

	// Invite permission.
	if( !guild_has_permission( sd, GUILD_PERM_INVITE ) ){
		// TODO: message?
		return false;
	}

	// Checking if there no other invitation pending
	if( !battle_config.invite_request_check && ( tsd->party_invite > 0 || tsd->trade_partner || tsd->adopt_invite ) ){
		clif_guild_inviteack( sd, 0 );
		return false;
	}

	// You can't invite someone who has already disconnected.
	if( !session_isActive( tsd->fd ) ){
		clif_guild_inviteack(sd,1);
		return false;
	}

	// Can't invite people inside castles. [Skotlex]
	if( tsd->status.guild_id > 0 || tsd->guild_invite > 0 || map_flag_gvg2( tsd->bl.m ) ){
		clif_guild_inviteack(sd,0);
		return false;
	}

	int i;

	//search an empty spot in guild
	ARR_FIND( 0, g->guild.max_member, i, g->guild.member[i].account_id == 0 );
	if( i == g->guild.max_member ){
		clif_guild_inviteack(sd,3);
		return false;
	}

	tsd->guild_invite = sd.status.guild_id;
	tsd->guild_invite_account = sd.status.account_id;

	clif_guild_invite(*tsd, g->guild);

	return true;
}

/// Guild invitation reply.
/// flag: 0:rejected, 1:accepted
bool guild_reply_invite( map_session_data& sd, int guild_id, int flag ){
	// subsequent requests may override the value
	if( sd.guild_invite != guild_id ){
		return false; // mismatch
	}

	// look up the person who sent the invite
	//NOTE: this can be nullptr because the person might have logged off in the meantime
	map_session_data* tsd = map_id2sd( sd.guild_invite_account );

	// Already in another guild.
	if( sd.status.guild_id > 0 ){
		// Set the flag to rejected, no matter what
		flag = 0;
	}

	// rejected
	if( flag == 0 ){
		sd.guild_invite = 0;
		sd.guild_invite_account = 0;

		if( tsd != nullptr ){
			clif_guild_inviteack( *tsd, 1 );
		}

		return true;
	}

	// accepted
	auto g = guild_search( guild_id );

	if( g == nullptr ){
		sd.guild_invite = 0;
		sd.guild_invite_account = 0;
		return false;
	}

	if( g->instance_id && battle_config.instance_block_invite ){
		sd.guild_invite = 0;
		sd.guild_invite_account = 0;
		return false;
	}

	int i;

	ARR_FIND( 0, g->guild.max_member, i, g->guild.member[i].account_id == 0 );

	if( i == g->guild.max_member ){
		sd.guild_invite = 0;
		sd.guild_invite_account = 0;

		if( tsd != nullptr ){
			clif_guild_inviteack( *tsd, 3 );
		}

		return true;
	}

	struct guild_member m = {};

	guild_makemember( m, sd );
	intif_guild_addmember( guild_id, m );
	//TODO: send a minimap update to this player

	return true;
}

//Invoked when a player joins.
//- If guild is not in memory, it is requested
//- Otherwise sd pointer is set up.
//- Player must be authed and must belong to a guild before invoking this method
void guild_member_joined(map_session_data *sd) {
	int i;
	auto g = guild_search(sd->status.guild_id);
	if (!g) {
		guild_request_info(sd->status.guild_id);
		return;
	}
	if (strcmp(sd->status.name,g->guild.master) == 0) {	// set the Guild Master flag
		sd->state.gmaster_flag = 1;
#ifndef RENEWAL
		// prevent Guild Skills from being used directly after relog
		if( sd->state.connect_new == 1 && battle_config.guild_skill_relog_delay )
			guild_block_skill(sd, battle_config.guild_skill_relog_delay);
#endif
	}
	i = guild_getindex(g->guild, sd->status.account_id, sd->status.char_id);
	if (i == -1)
		sd->status.guild_id = 0;
	else {
		g->guild.member[i].sd = sd;
		sd->guild = g;

		if( channel_config.ally_tmpl.name[0] && (channel_config.ally_tmpl.opt&CHAN_OPT_AUTOJOIN) ) {
			channel_gjoin(sd,3);
		}
	}
}

/*==========================================
 * Add a player to a given guild_id
 *----------------------------------------*/
int guild_member_added(int guild_id,uint32 account_id,uint32 char_id,int flag) {
	map_session_data *sd= map_id2sd(account_id),*sd2;
	auto g = guild_search(guild_id);

	if (!g)
		return 0;

	if(sd==nullptr || sd->guild_invite==0){
	// cancel if player not present or invalide guild_id invitation
		if (flag == 0) {
			ShowError("guild: member added error %d is not online\n",account_id);
 			intif_guild_leave(guild_id,account_id,char_id,0,"** Data Error **");
		}
		return 0;
	}
	sd2 = map_id2sd(sd->guild_invite_account);
	sd->guild_invite = 0;
	sd->guild_invite_account = 0;

	if (flag == 1) { //failure
		if( sd2!=nullptr )
			clif_guild_inviteack( *sd2, 3 );
		return 0;
	}

	//if all ok add player to guild
	sd->status.guild_id = g->guild.guild_id;
	sd->guild_emblem_id = g->guild.emblem_id;
	sd->guild = g;
	//Packets which were sent in the previous 'guild_sent' implementation.
	clif_guild_belonginfo( *sd );
	clif_guild_notice( *sd );

	//TODO: send new emblem info to others

	if( sd2!=nullptr )
		clif_guild_inviteack( *sd2, 2 );

	//Next line commented because it do nothing, look at guild_recv_info [LuzZza]
	//clif_charnameupdate(sd); //Update display name [Skotlex]

	if (g->instance_id > 0)
		instance_reqinfo(sd, g->instance_id);

	return 0;
}

/*==========================================
 * Player request leaving a given guild_id
 *----------------------------------------*/
bool guild_leave( map_session_data& sd, int guild_id, uint32 account_id, uint32 char_id, const char* mes ){
	auto& g = sd.guild;

	if( g == nullptr ){
		return false;
	}

	if( g->instance_id > 0 && battle_config.instance_block_leave ){
		return false;
	}

	if( map_getmapflag( sd.bl.m, MF_GUILDLOCK ) ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 228 ) ); // Guild modification is disabled on this map.
		return false;
	}

	if( sd.bg_id ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 670 ) ); // You can't leave battleground guilds.
		return false;
	}

	if( sd.status.account_id != account_id || sd.status.char_id != char_id || sd.status.guild_id != guild_id || map_flag_gvg2( sd.bl.m ) ){
		return false;
	}

	guild_trade_bound_cancel(sd);

	return intif_guild_leave( sd.status.guild_id, sd.status.account_id, sd.status.char_id, 0, mes );
}

/*==========================================
 * Request remove a player to a given guild_id
 *----------------------------------------*/
bool guild_expulsion( map_session_data& sd, int guild_id, uint32 account_id, uint32 char_id, const char* mes ){
	auto& g = sd.guild;

	if( g == nullptr ){
		return false;
	}

	if( sd.status.guild_id != guild_id ){
		return false;
	}

	if( !guild_has_permission( sd, GUILD_PERM_EXPEL ) ){
		return false;
	}

	if( g->instance_id > 0 && battle_config.instance_block_expulsion ){
		return false;
	}

	// TODO: for leave this is different messages
	if( sd.bg_id || map_getmapflag( sd.bl.m, MF_GUILDLOCK ) ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 228 ) ); // Guild modification is disabled on this map.
		return false;
	}

	map_session_data *tsd = map_id2sd( account_id );

	//Can't leave inside guild castles.
	if( tsd != nullptr && tsd->status.char_id == char_id && map_flag_gvg2( tsd->bl.m ) ){
		return false;
	}

	// find the member and perform expulsion
	int i = guild_getindex( g->guild, account_id, char_id );

	if( i < 0 ){
		return false;
	}

	// Can't expel the guild leader
	if( strcmp( g->guild.member[i].name, g->guild.master ) == 0 ){
		return false;
	}

	if( tsd != nullptr ){
		guild_trade_bound_cancel( *tsd );
	}

	return intif_guild_leave( g->guild.guild_id, account_id, char_id, 1, mes );
}

/**
* A confirmation from inter-serv that player is kicked successfully
* @param guild_Id
* @param account_id
* @param char_id
* @param flag
* @param name
* @param mes
*/
int guild_member_withdraw(int guild_id, uint32 account_id, uint32 char_id, int flag, const char* name, const char* mes) {
	int i;
	auto g = guild_search(guild_id);
	map_session_data* sd = map_charid2sd(char_id);
	map_session_data* online_member_sd;

	if (!g)
		return 0; // no such guild (error!)

	i = guild_getindex(g->guild, account_id, char_id);
	if( i == -1 )
		return 0; // not a member (inconsistency!)

#ifdef BOUND_ITEMS
	//Guild bound item check
	guild_retrieveitembound(char_id,account_id,guild_id);
#endif

	online_member_sd = guild_getavailablesd(g->guild);
	if(online_member_sd == nullptr)
		return 0; // noone online to inform


	if(!flag)
		clif_guild_leave( *online_member_sd, name, char_id, mes );
	else
		clif_guild_expulsion( *online_member_sd, name, char_id, mes );

	// remove member from guild
	memset(&g->guild.member[i],0,sizeof(struct guild_member));
	clif_guild_memberlist( *online_member_sd );

	// update char, if online
	if(sd != nullptr && sd->status.guild_id == guild_id) {
		// do stuff that needs the guild_id first, BEFORE we wipe it
		if (sd->state.storage_flag == 2) //Close the guild storage.
			storage_guild_storageclose(sd);
		guild_send_dot_remove(sd);
		channel_pcquit(sd,3); //leave guild and ally chan
		sd->status.guild_id = 0;
		sd->guild = nullptr;
		sd->guild_emblem_id = 0;

		if (g->instance_id) {
			struct map_data *mapdata = map_getmapdata(sd->bl.m);

			// User was on the instance map of the guild
			if( g->instance_id == mapdata->instance_id ){
				pc_setpos_savepoint( *sd );
			}
		}

		clif_name_area(&sd->bl); //Update display name [Skotlex]
		status_change_end(&sd->bl,SC_LEADERSHIP);
		status_change_end(&sd->bl,SC_GLORYWOUNDS);
		status_change_end(&sd->bl,SC_SOULCOLD);
		status_change_end(&sd->bl,SC_HAWKEYES);
		status_change_end(&sd->bl,SC_EMERGENCY_MOVE);
		//@TODO: Send emblem update to self and people around
	}
	return 0;
}

#ifdef BOUND_ITEMS
/**
* Retrieve guild bound items from kicked member
* @param char_id
* @param account_id
* @param guild_id
*/
void guild_retrieveitembound(uint32 char_id, uint32 account_id, int guild_id) {
	TBL_PC *sd = map_charid2sd(char_id);
	if (sd) { //Character is online
		int idxlist[MAX_INVENTORY];
		int j;
		j = pc_bound_chk(sd,BOUND_GUILD,idxlist);
		if (j) {
			struct s_storage* stor = guild2storage(sd->status.guild_id);
			auto g = guild_search(guild_id);
			int i;
			if (stor && stor->status) { //Someone is in guild storage, close them
				for (i = 0; i < g->guild.max_member; i++) {
					TBL_PC *pl_sd = g->guild.member[i].sd;
					if (pl_sd && pl_sd->state.storage_flag == 2)
						storage_guild_storageclose(pl_sd);
				}
			}
			for (i = 0; i < j; i++) { //Loop the matching items, gstorage_additem takes care of opening storage
				if (stor)
					storage_guild_additem(sd,stor,&sd->inventory.u.items_inventory[idxlist[i]],sd->inventory.u.items_inventory[idxlist[i]].amount);
				pc_delitem(sd,idxlist[i],sd->inventory.u.items_inventory[idxlist[i]].amount,0,4,LOG_TYPE_GSTORAGE);
			}
			storage_guild_storageclose(sd); //Close and save the storage
		}
	} else { //Character is offline, ask char server to do the job
		struct s_storage* stor = guild2storage2(guild_id);
		auto g = guild_search(guild_id);
		if (!g)
			return;
		if (stor && stor->status) { //Someone is in guild storage, close them
			int i;
			for (i = 0; i < g->guild.max_member; i++) {
				TBL_PC *pl_sd = g->guild.member[i].sd;
				if (pl_sd && pl_sd->state.storage_flag == 2)
					storage_guild_storageclose(pl_sd);
			}
		}
		intif_itembound_guild_retrieve(char_id,account_id,guild_id);
	}
}
#endif

int guild_send_memberinfoshort(map_session_data *sd,int online) { // cleaned up [LuzZza]
	nullpo_ret(sd);

	if(sd->status.guild_id <= 0)
		return 0;

	auto &g = sd->guild;

	if (!g)
		return 0;

	intif_guild_memberinfoshort(g->guild.guild_id,
		sd->status.account_id,sd->status.char_id,online,sd->status.base_level,sd->status.class_);

	if(!online){
		int i=guild_getindex(g->guild, sd->status.account_id,sd->status.char_id);
		if(i>=0)
			g->guild.member[i].sd=nullptr;
		else
			ShowError("guild_send_memberinfoshort: Failed to locate member %d:%d in guild %d!\n", sd->status.account_id, sd->status.char_id, g->guild.guild_id);
		return 0;
	}

	if(sd->state.connect_new) {	//Note that this works because it is invoked in parse_LoadEndAck before connect_new is cleared.
		clif_guild_belonginfo( *sd );
		sd->guild_emblem_id = g->guild.emblem_id;
	}
	return 0;
}

int guild_recv_memberinfoshort(int guild_id,uint32 account_id,uint32 char_id,int online,int lv,int class_) { // cleaned up [LuzZza]

	int i,alv,c,idx=-1,om=0,oldonline=-1;
	auto g = guild_search(guild_id);

	if(g == nullptr)
		return 0;

	for(i=0,alv=0,c=0,om=0;i<g->guild.max_member;i++){
		struct guild_member *m=&g->guild.member[i];
		if(!m->account_id) continue;
		if(m->account_id==account_id && m->char_id==char_id ){
			oldonline=m->online;
			m->online=online;
			m->lv=lv;
			m->class_=class_;
			idx=i;
		}
		alv+=m->lv;
		c++;
		if(m->online)
			om++;
	}

	if(idx == -1 || c == 0) {
        //Treat char_id who doesn't match guild_id (not found as member)
		map_session_data *sd = map_id2sd(account_id);
		if(sd && sd->status.char_id == char_id) {
			sd->status.guild_id=0;
			sd->guild_emblem_id=0;
		}
		ShowWarning("guild: not found member %d,%d on %d[%s]\n", account_id,char_id,guild_id,g->guild.name);
		return 0;
	}

	g->guild.average_lv=alv/c;
	g->guild.connect_member=om;

	//Ensure validity of pointer (ie: player logs in/out, changes map-server)
	g->guild.member[idx].sd = guild_sd_check(guild_id, account_id, char_id);

	if(oldonline!=online)
		clif_guild_memberlogin_notice(g->guild, idx, online);

	if(!g->guild.member[idx].sd)
		return 0;

	//Send XY dot updates. [Skotlex]
	//Moved from guild_send_memberinfoshort [LuzZza]
	for(i=0; i < g->guild.max_member; i++) {

		if(!g->guild.member[i].sd || i == idx ||
			g->guild.member[i].sd->bl.m != g->guild.member[idx].sd->bl.m)
			continue;

		clif_guild_xy_single( *g->guild.member[idx].sd, *g->guild.member[i].sd );
		clif_guild_xy_single( *g->guild.member[i].sd, *g->guild.member[idx].sd );
	}

	return 0;
}

/*====================================================
 * Send a message to whole guild
 *---------------------------------------------------*/
int guild_send_message(map_session_data *sd, const char *mes, size_t len) {
	nullpo_ret(sd);

	if(sd->status.guild_id==0)
		return 0;
	intif_guild_message(sd->status.guild_id,sd->status.account_id,mes,len);
	guild_recv_message(sd->status.guild_id,sd->status.account_id,mes,len);

	// Chat logging type 'G' / Guild Chat
	log_chat(LOG_CHAT_GUILD, sd->status.guild_id, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, nullptr, mes);

	return 0;
}

/*====================================================
 * Guild receive a message, will be displayed to whole member
 *---------------------------------------------------*/
int guild_recv_message( int guild_id, uint32 account_id, const char *mes, size_t len ){
	auto g = guild_search(guild_id);
	if (!g)
		return 0;
	clif_guild_message(g->guild,account_id,mes,len);
	return 0;
}

/*====================================================
 * Member changing position in guild
 *---------------------------------------------------*/
int guild_change_memberposition(int guild_id,uint32 account_id,uint32 char_id,short idx) {
	return intif_guild_change_memberinfo(guild_id,account_id,char_id,GMI_POSITION,&idx,sizeof(idx));
}

/*====================================================
 * Notification of new position for member
 *---------------------------------------------------*/
int guild_memberposition_changed(struct mmo_guild &g,int idx,int pos) {

	g.member[idx].position=pos;
	clif_guild_memberpositionchanged(g,idx);

	// Update char position in client [LuzZza]
	if(g.member[idx].sd != nullptr)
		clif_name_area(&g.member[idx].sd->bl);
	return 0;
}

/*====================================================
 * Change guild title or member
 *---------------------------------------------------*/
int guild_change_position(int guild_id,int idx, int mode, int exp_mode, const char *name) {
	struct guild_position p;

	exp_mode = cap_value(exp_mode, 0, battle_config.guild_exp_limit);
	p.mode = mode&GUILD_PERM_ALL;
	p.exp_mode=exp_mode;
	safestrncpy(p.name,name,NAME_LENGTH);
	return intif_guild_position(guild_id,idx,&p);
}

/*====================================================
 * Notification of member has changed his guild title
 *---------------------------------------------------*/
int guild_position_changed(int guild_id,int idx,struct guild_position *p) {
	auto g = guild_search(guild_id);
	int i;
	if(g==nullptr)
		return 0;
	memcpy(&g->guild.position[idx],p,sizeof(struct guild_position));
	clif_guild_positionchanged(g->guild,idx);

	// Update char name in client [LuzZza]
	for(i=0;i<g->guild.max_member;i++)
		if(g->guild.member[i].position == idx && g->guild.member[i].sd != nullptr)
			clif_name_area(&g->guild.member[i].sd->bl);
	return 0;
}

/*====================================================
 * Change guild notice
 *---------------------------------------------------*/
int guild_change_notice(map_session_data *sd,int guild_id,const char *mes1,const char *mes2) {
	nullpo_ret(sd);

	if(guild_id!=sd->status.guild_id)
		return 0;
	return intif_guild_notice(guild_id,mes1,mes2);
}

/*====================================================
 * Notification of guild has changed his notice
 *---------------------------------------------------*/
int guild_notice_changed(int guild_id,const char *mes1,const char *mes2) {
	int i;
	auto g = guild_search(guild_id);
	if (!g)
		return 0;

	memcpy(g->guild.mes1,mes1,MAX_GUILDMES1);
	memcpy(g->guild.mes2,mes2,MAX_GUILDMES2);

	for(i=0;i<g->guild.max_member;i++){
		map_session_data *sd = g->guild.member[i].sd;
		if(sd != nullptr)
			clif_guild_notice( *sd );
	}
	return 0;
}

/*====================================================
 * Check condition for changing guild emblem
 *---------------------------------------------------*/
bool guild_check_emblem_change_condition( map_session_data& sd ){
	auto &g = sd.guild;

	if (battle_config.require_glory_guild && g != nullptr && guild_checkskill(g->guild, GD_GLORYGUILD) > 0) {
		clif_skill_fail( sd, GD_GLORYGUILD );
		return false;
	}

	return true;
}

/*====================================================
 * Change guild emblem
 *---------------------------------------------------*/
int guild_change_emblem( map_session_data& sd, int len, const char* data ){
	if (!guild_check_emblem_change_condition(sd)) {
		return 0;
	}

	return intif_guild_emblem( sd.status.guild_id, len, data );
}

/*====================================================
 * Change guild emblem version
 *---------------------------------------------------*/
int guild_change_emblem_version( map_session_data& sd, int version ){
	if (!guild_check_emblem_change_condition(sd)) {
		return 0;
	}

	return intif_guild_emblem_version( sd.status.guild_id, version );
}

/*====================================================
 * Notification of guild emblem changed
 *---------------------------------------------------*/
int guild_emblem_changed(int len,int guild_id,int emblem_id,const char *data) {
	int i;
	map_session_data *sd;
	auto g = guild_search(guild_id);
	if (!g)
		return 0;

	if (data != nullptr)
		memcpy(g->guild.emblem_data,data,len);
	g->guild.emblem_len=len;
	g->guild.emblem_id=emblem_id;

	for(i=0;i<g->guild.max_member;i++){
		if((sd=g->guild.member[i].sd)!=nullptr){
			sd->guild_emblem_id=emblem_id;
			clif_guild_belonginfo( *sd );
			clif_guild_emblem(*sd, g->guild);
			clif_guild_emblem_area(&sd->bl);
		}
	}
	{// update guardians (mobs)
		for (const auto &it : castle_db) {
			if( it.second->guild_id != guild_id )
				continue;
			// update permanent guardians
			for( i = 0; i < ARRAYLENGTH(it.second->guardian); ++i )
			{
				TBL_MOB* md = (it.second->guardian[i].id ? map_id2md(it.second->guardian[i].id) : nullptr);
				if( md == nullptr || md->guardian_data == nullptr )
					continue;
				md->guardian_data->emblem_id = emblem_id;
				clif_guild_emblem_area(&md->bl);
			}
			// update temporary guardians
			for( i = 0; i < it.second->temp_guardians_max; ++i )
			{
				TBL_MOB* md = (it.second->temp_guardians[i] ? map_id2md(it.second->temp_guardians[i]) : nullptr);
				if( md == nullptr || md->guardian_data == nullptr )
					continue;
				md->guardian_data->emblem_id = emblem_id;
				clif_guild_emblem_area(&md->bl);
			}
		}
	}
	{// update npcs (flags or other npcs that used flagemblem to attach to this guild)
		for( i = 0; i < guild_flags_count; i++ ) {
			if( guild_flags[i] && guild_flags[i]->u.scr.guild_id == guild_id ) {
				clif_guild_emblem_area(&guild_flags[i]->bl);
			}
		}
	}
	return 0;
}

/**
 * @see DBCreateData
 */
static DBData create_expcache(DBKey key, va_list args) {
	struct guild_expcache *c;
	map_session_data *sd = va_arg(args, map_session_data*);

	c = ers_alloc(expcache_ers, struct guild_expcache);
	c->guild_id = sd->status.guild_id;
	c->account_id = sd->status.account_id;
	c->char_id = sd->status.char_id;
	c->exp = 0;
	return db_ptr2data(c);
}

/*====================================================
 * Return taxed experience from player sd to guild
 *---------------------------------------------------*/
t_exp guild_payexp(map_session_data *sd,t_exp exp) {
	struct guild_expcache *c;
	int per;

	nullpo_ret(sd);

	if (!exp) return 0;

	auto &g = sd->guild;

	if (sd->status.guild_id == 0 || !g ||
		(per = guild_getposition(*sd)) < 0 ||
		(per = g->guild.position[per].exp_mode) < 1)
		return 0;


	if (per < 100)
		exp = exp * per / 100;
	//Otherwise tax everything.

	c = (struct guild_expcache *)db_data2ptr(guild_expcache_db->ensure(guild_expcache_db, db_i2key(sd->status.char_id), create_expcache, sd));
	c->exp = util::safe_addition_cap(c->exp, exp, MAX_GUILD_EXP);

	return exp;
}

/*====================================================
 * Player sd pay a tribute experience to his guild
 * Add this experience to guild exp
 * [Celest]
 *---------------------------------------------------*/
t_exp guild_getexp(map_session_data *sd,t_exp exp) {
	struct guild_expcache *c;
	nullpo_ret(sd);

	if (sd->status.guild_id == 0 || sd->guild == nullptr)
		return 0;

	c = (struct guild_expcache *)db_data2ptr(guild_expcache_db->ensure(guild_expcache_db, db_i2key(sd->status.char_id), create_expcache, sd));
	c->exp = util::safe_addition_cap(c->exp, exp, MAX_GUILD_EXP);

	return exp;
}

/*====================================================
 * Ask to increase guildskill skill_id
 *---------------------------------------------------*/
void guild_skillup(map_session_data* sd, uint16 skill_id) {
	short idx = guild_skill_get_index(skill_id);
	short max = 0;

	nullpo_retv(sd);

	if (idx == -1)
		return;

	auto &g = sd->guild;

	if( sd->status.guild_id == 0 || !g || // no guild
		strcmp(sd->status.name, g->guild.master) ) // not the guild master
		return;

	max = guild_skill_get_max(skill_id);

	if( g->guild.skill_point > 0 &&
		g->guild.skill[idx].id != 0 &&
		g->guild.skill[idx].lv < max )
		intif_guild_skillup(g->guild.guild_id, skill_id, sd->status.account_id, max);
}

/*====================================================
 * Notification of guildskill skill_id increase request
 *---------------------------------------------------*/
int guild_skillupack(int guild_id,uint16 skill_id,uint32 account_id) {
	map_session_data *sd = map_id2sd(account_id);
	auto g = guild_search(guild_id);
	int i;
	short idx = guild_skill_get_index(skill_id);

	if (g == nullptr || idx == -1)
		return 0;
	if (sd != nullptr) {
		int lv = g->guild.skill[idx].lv;
		int range = skill_get_range(skill_id, lv);
		clif_skillup(sd,skill_id,lv,range,1);

		/* Guild Aura handling */
		switch( skill_id ) {
			case GD_LEADERSHIP:
			case GD_GLORYWOUNDS:
			case GD_SOULCOLD:
			case GD_HAWKEYES:
					guild_guildaura_refresh(sd,skill_id,g->guild.skill[idx].lv);
				break;
		}
	}

	// Inform all members
	for (i = 0; i < g->guild.max_member; i++)
		if ((sd = g->guild.member[i].sd) != nullptr)
			clif_guild_skillinfo( *sd );

	return 0;
}

void guild_guildaura_refresh(map_session_data *sd, uint16 skill_id, uint16 skill_lv) {
	if( !(battle_config.guild_aura&(is_agit_start()?2:1)) &&
			!(battle_config.guild_aura&(map_flag_gvg2(sd->bl.m)?8:4)) )
		return;
	if( !skill_lv )
		return;

	sc_type type = skill_get_sc(skill_id);

	if (type == SC_NONE)
		return;

	status_change_end(&sd->bl, type);

	std::shared_ptr<s_skill_unit_group> group = skill_unitsetting(&sd->bl,skill_id,skill_lv,sd->bl.x,sd->bl.y,0);

	if( group )
		sc_start4(nullptr,&sd->bl,type,100,(battle_config.guild_aura&16)?0:skill_lv,0,0,group->group_id,600000);//duration doesn't matter these status never end with val4
	return;
}

/*====================================================
 * Count number of relations the guild has.
 * Flag:
 *	0 = allied
 *	1 = enemy
 *---------------------------------------------------*/
int guild_get_alliance_count(const struct mmo_guild &g,int flag) {
	int i,c;

	for(i=c=0;i<MAX_GUILDALLIANCE;i++){
		if(	g.alliance[i].guild_id>0 &&
			g.alliance[i].opposition==flag )
			c++;
	}
	return c;
}

// Blocks all guild skills which have a common delay time.
void guild_block_skill(map_session_data *sd, int time) {
	uint16 skill_id[] = { GD_BATTLEORDER, GD_REGENERATION, GD_RESTORE, GD_EMERGENCYCALL };
	int i;
	for (i = 0; i < 4; i++)
		skill_blockpc_start(sd, skill_id[i], time);
}

/*====================================================
 * Check relation between guild_id1 and guild_id2.
 * Flag:
 *	0 = allied
 *	1 = enemy
 * Returns true if yes.
 *---------------------------------------------------*/
int guild_check_alliance(int guild_id1, int guild_id2, int flag) {
	int i;

	auto g = guild_search(guild_id1);
	if (g == nullptr)
		return 0;

	ARR_FIND( 0, MAX_GUILDALLIANCE, i, g->guild.alliance[i].guild_id == guild_id2 && g->guild.alliance[i].opposition == flag );
	return( i < MAX_GUILDALLIANCE ) ? 1 : 0;
}

/*====================================================
 * Player sd, asking player tsd an alliance between their 2 guilds
 *---------------------------------------------------*/
int guild_reqalliance(map_session_data *sd,map_session_data *tsd) {
	int i;

	if(is_agit_start()) {	// Disable alliance creation during woe [Valaris]
		clif_displaymessage(sd->fd,msg_txt(sd,676)); //"Alliances cannot be made during Guild Wars!"
		return 0;
	}	// end addition [Valaris]


	nullpo_ret(sd);

	if(tsd==nullptr || tsd->status.guild_id<=0)
		return 0;

	// Check, is tsd guild master, if not - cancel alliance. [f0und3r]
	if (battle_config.guild_alliance_onlygm && !tsd->state.gmaster_flag) {
		clif_guild_allianceack(sd, 5);
		return 0;
	}

	auto &g = sd->guild;
	auto &tg = tsd->guild;

	if(g==nullptr || tg==nullptr)
		return 0;

	// Prevent creation alliance with same guilds [LuzZza]
	if(sd->status.guild_id == tsd->status.guild_id)
		return 0;

	if( guild_get_alliance_count(g->guild,0) >= battle_config.max_guild_alliance ) {
		clif_guild_allianceack(sd,4);
		return 0;
	}
	if( guild_get_alliance_count(tg->guild,0) >= battle_config.max_guild_alliance ) {
		clif_guild_allianceack(sd,3);
		return 0;
	}

	if( tsd->guild_alliance>0 ){
		clif_guild_allianceack(sd,1);
		return 0;
	}

	for (i = 0; i < MAX_GUILDALLIANCE; i++) { // check if already allied
		if(	g->guild.alliance[i].guild_id==tsd->status.guild_id &&
			g->guild.alliance[i].opposition==0){
			clif_guild_allianceack(sd,0);
			return 0;
		}
	}

	tsd->guild_alliance=sd->status.guild_id;
	tsd->guild_alliance_account=sd->status.account_id;

	clif_guild_reqalliance(tsd,sd->status.account_id,g->guild.name);
	return 0;
}

/*====================================================
 * Player sd, answer to player tsd (account_id) for an alliance request
 *---------------------------------------------------*/
int guild_reply_reqalliance(map_session_data *sd,uint32 account_id,int flag) {
	map_session_data *tsd;

	nullpo_ret(sd);
	tsd= map_id2sd( account_id );
	if (!tsd) { //Character left? Cancel alliance.
		clif_guild_allianceack(sd,3);
		return 0;
	}

	if (sd->guild_alliance != tsd->status.guild_id) // proposed guild_id alliance doesn't match tsd guildid
		return 0;

	if (flag == 1) { // consent
		int i;

		auto &g = sd->guild;
		auto &tg = tsd->guild;

		if (!g || guild_get_alliance_count(g->guild,0) >= battle_config.max_guild_alliance) {
			clif_guild_allianceack(sd,4);
			clif_guild_allianceack(tsd,3);
			return 0;
		}
		if (!g || guild_get_alliance_count(tg->guild,0) >= battle_config.max_guild_alliance) {
			clif_guild_allianceack(sd,3);
			clif_guild_allianceack(tsd,4);
			return 0;
		}

		for(i=0;i<MAX_GUILDALLIANCE;i++){
			if(g->guild.alliance[i].guild_id==tsd->status.guild_id &&
				g->guild.alliance[i].opposition==1)
				intif_guild_alliance( sd->status.guild_id,tsd->status.guild_id,
					sd->status.account_id,tsd->status.account_id,9 );
		}
		for(i=0;i<MAX_GUILDALLIANCE;i++){
			if(tg->guild.alliance[i].guild_id==sd->status.guild_id &&
				tg->guild.alliance[i].opposition==1)
				intif_guild_alliance( tsd->status.guild_id,sd->status.guild_id,
					tsd->status.account_id,sd->status.account_id,9 );
		}

	// inform other servers
		intif_guild_alliance( sd->status.guild_id,tsd->status.guild_id,
			sd->status.account_id,tsd->status.account_id,0 );
		return 0;
	} else { // deny
		sd->guild_alliance=0;
		sd->guild_alliance_account=0;
		if(tsd!=nullptr)
			clif_guild_allianceack(tsd,3);
	}
	return 0;
}

/*====================================================
 * Player sd asking to break alliance with guild guild_id
 *---------------------------------------------------*/
int guild_delalliance(map_session_data *sd,int guild_id,int flag) {
	nullpo_ret(sd);

	if(is_agit_start()) {	// Disable alliance breaking during woe [Valaris]
		clif_displaymessage(sd->fd,msg_txt(sd,677)); //"Alliances cannot be broken during Guild Wars!"
		return 0;
	}	// end addition [Valaris]

	intif_guild_alliance( sd->status.guild_id,guild_id,sd->status.account_id,0,flag|8 );
	return 0;
}

/*====================================================
 * Player sd, asking player tsd a formal enemy relation between their 2 guilds
 *---------------------------------------------------*/
int guild_opposition(map_session_data *sd,map_session_data *tsd) {
	int i;

	nullpo_ret(sd);

	auto &g = sd->guild;
	if(g==nullptr || tsd==nullptr)
		return 0;

	// Prevent creation opposition with same guilds [LuzZza]
	if(sd->status.guild_id == tsd->status.guild_id)
		return 0;

	if( guild_get_alliance_count(g->guild,1) >= battle_config.max_guild_alliance )	{
		clif_guild_oppositionack(sd,1);
		return 0;
	}

	for (i = 0; i < MAX_GUILDALLIANCE; i++) { // checking relations
		if(g->guild.alliance[i].guild_id==tsd->status.guild_id){
			if (g->guild.alliance[i].opposition == 1) { // check if not already hostile
				clif_guild_oppositionack(sd,2);
				return 0;
			}
			if(is_agit_start()) // Prevent the changing of alliances to oppositions during WoE.
				return 0;
			//Change alliance to opposition.
			intif_guild_alliance( sd->status.guild_id,tsd->status.guild_id,
				sd->status.account_id,tsd->status.account_id,8 );
		}
	}

	// inform other serv
	intif_guild_alliance( sd->status.guild_id,tsd->status.guild_id,
			sd->status.account_id,tsd->status.account_id,1 );
	return 0;
}

/*====================================================
 * Notification of a relationship between 2 guilds
 *---------------------------------------------------*/
int guild_allianceack(int guild_id1,int guild_id2,uint32 account_id1,uint32 account_id2,int flag,const char *name1,const char *name2)
{
	struct mmo_guild *g[2];
	int guild_id[2];
	const char *guild_name[2];
	map_session_data *sd[2];
	int j,i;

	guild_id[0] = guild_id1;
	guild_id[1] = guild_id2;
	guild_name[0] = name1;
	guild_name[1] = name2;
	sd[0] = map_id2sd(account_id1);
	sd[1] = map_id2sd(account_id2);

	g[0] = &guild_search(guild_id1)->guild;
	g[1] = &guild_search(guild_id2)->guild;

	if(sd[0]!=nullptr && (flag&0x0f)==0){
		sd[0]->guild_alliance=0;
		sd[0]->guild_alliance_account=0;
	}

	if (flag & 0x70) { // failure
		for(i=0;i<2-(flag&1);i++)
			if( sd[i]!=nullptr )
				clif_guild_allianceack(sd[i],((flag>>4)==i+1)?3:4);
		return 0;
	}

	if (!(flag & 0x08)) { // new relationship
		for(i=0;i<2-(flag&1);i++) {
			if(g[i]!=nullptr) {
				ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == 0 );
				if( j < MAX_GUILDALLIANCE ) {
					g[i]->alliance[j].guild_id=guild_id[1-i];
					memcpy(g[i]->alliance[j].name,guild_name[1-i],NAME_LENGTH);
					g[i]->alliance[j].opposition=flag&1;
				}
			}
		}
	} else { // remove relationship
		for(i=0;i<2-(flag&1);i++) {
			if(g[i]!=nullptr) {
				for(j=0;j<g[i]->max_member;j++) channel_pcquit(g[i]->member[j].sd,2); //leave all alliance chan
				ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == guild_id[1-i] && g[i]->alliance[j].opposition == (flag&1) );
				if( j < MAX_GUILDALLIANCE )
					g[i]->alliance[j].guild_id = 0;
			}
		if (sd[i] != nullptr) // notify players
				clif_guild_delalliance(sd[i],guild_id[1-i],(flag&1));
		}
	}

	if ((flag & 0x0f) == 0) { // alliance notification
		if( sd[1]!=nullptr )
			clif_guild_allianceack(sd[1],2);
	} else if ((flag & 0x0f) == 1) { // enemy notification
		if( sd[0]!=nullptr )
			clif_guild_oppositionack(sd[0],0);
	}


	for (i = 0; i < 2 - (flag & 1); i++) { // Retransmission of the relationship list to all members
		if(g[i]!=nullptr)
			for(j=0;j<g[i]->max_member;j++) {
				map_session_data *sd_mem = g[i]->member[j].sd;
				if( sd_mem!=nullptr){
					clif_guild_allianceinfo(sd_mem);

					// join ally channel
					if( channel_config.ally_tmpl.name[0] && (channel_config.ally_tmpl.opt&CHAN_OPT_AUTOJOIN) ) {
						channel_gjoin(sd_mem,2);
					}
				}
			}
	}
	return 0;
}

/**
 * Notification for the guild disbanded
 * @see DBApply
 */
int guild_broken_sub(struct mmo_guild &g, int guild_id) {
	for (int i = 0; i < MAX_GUILDALLIANCE; i++) {  // Destroy all relationships
		if (g.alliance[i].guild_id == guild_id) {
			for (int j = 0; j < g.max_member; j++) {
				if (g.member[j].sd)
					clif_guild_delalliance(g.member[j].sd, guild_id, g.alliance[i].opposition);
			}
			intif_guild_alliance(g.guild_id, guild_id, 0, 0, g.alliance[i].opposition | 8);
			g.alliance[i].guild_id = 0;
		}
	}
	return 0;
}

/**
 * Invoked on Castles when a guild is broken. [Skotlex]
 * @see DBApply
 */
void castle_guild_broken_sub(int guild_id) {
	for (const auto &it : castle_db) {
		std::shared_ptr<guild_castle> gc = it.second;
		if (gc->guild_id == guild_id) {
			char name[EVENT_NAME_LENGTH];
			// We call castle_event::OnGuildBreak of all castles of the guild
			// You can set all castle_events in the 'db/castle_db.txt'
			safesnprintf(name, EVENT_NAME_LENGTH, "%s::%s", gc->castle_event, script_config.guild_break_event_name);
			npc_event_do(name);

			//Save the new 'owner', this should invoke guardian clean up and other such things.
			guild_castledatasave(gc->castle_id, CD_GUILD_ID, 0);
		}
	}
}

//Invoked on /breakguild "Guild name"
int guild_broken(int guild_id,int flag) {
	auto g = guild_search(guild_id);
	int i;

	if (flag != 0 || g == nullptr)
		return 0;

	for (i = 0; i < g->guild.max_member; i++){	// Destroy all relationships
		map_session_data *sd = g->guild.member[i].sd;
		if(sd != nullptr){
			if(sd->state.storage_flag == 2)
				storage_guild_storage_quit(sd,1);
			sd->status.guild_id=0;
			sd->guild = nullptr;
			sd->state.gmaster_flag = 0;
			clif_guild_broken( *sd, 0 );
			clif_name_area(&sd->bl); // [LuzZza]
			status_change_end(&sd->bl,SC_LEADERSHIP);
			status_change_end(&sd->bl,SC_GLORYWOUNDS);
			status_change_end(&sd->bl,SC_SOULCOLD);
			status_change_end(&sd->bl,SC_HAWKEYES);
			status_change_end(&sd->bl,SC_EMERGENCY_MOVE);
		}
	}

	for (auto &it : guild_db) {
		if (!it.second)
			continue;
		guild_broken_sub(it.second->guild, guild_id);
	}

	castle_guild_broken_sub(guild_id);
	storage_guild_delete(guild_id);
	if( channel_config.ally_tmpl.name[0] ) {
		channel_delete(g->channel,false);
	}
	guild_db.erase(guild_id);
	return 0;
}

/** Changes the Guild Master to the specified player. [Skotlex]
* @param guild_id
* @param sd New guild master
*/
bool guild_gm_change( int guild_id, uint32 char_id, bool showMessage ){
	auto g = guild_search( guild_id );

	if( g == nullptr ){
		return false;
	}

	if( g->instance_id > 0 && battle_config.instance_block_leaderchange ){
		return false;
	}

	int i;

	ARR_FIND( 0, MAX_GUILD, i, g->guild.member[i].char_id == char_id );
	
	if( i == MAX_GUILD ){
		// Not part of the guild
		return false;
	}

	char* name = g->guild.member[i].name;

	// Nothing to change.
	if( strcmp( g->guild.master, name ) == 0 ){
		return false;
	}

	//Notify servers that master has changed.
	return intif_guild_change_gm( guild_id, name, strlen( name ) + 1 );
}

/** Notification from Char server that a guild's master has changed. [Skotlex]
* @param guild_id
* @param account_id
* @param char_id
*/
int guild_gm_changed(int guild_id, uint32 account_id, uint32 char_id, time_t time) {
	struct guild_member gm;
	int pos, i;

	auto g = guild_search(guild_id);

	if (!g)
		return 0;

	for(pos=0; pos<g->guild.max_member && !(
		g->guild.member[pos].account_id==account_id &&
		g->guild.member[pos].char_id==char_id);
		pos++);

	if (pos == 0 || pos == g->guild.max_member) return 0;

	memcpy(&gm, &g->guild.member[pos], sizeof (struct guild_member));
	memcpy(&g->guild.member[pos], &g->guild.member[0], sizeof(struct guild_member));
	memcpy(&g->guild.member[0], &gm, sizeof(struct guild_member));

	g->guild.member[pos].position = g->guild.member[0].position;
	g->guild.member[0].position = 0; //Position 0: guild Master.
	strcpy(g->guild.master, g->guild.member[0].name);

	if (g->guild.member[pos].sd && g->guild.member[pos].sd->fd) {
		clif_displaymessage(g->guild.member[pos].sd->fd, msg_txt(g->guild.member[pos].sd,678)); //"You no longer are the Guild Master."
		g->guild.member[pos].sd->state.gmaster_flag = 0;
		clif_name_area(&g->guild.member[pos].sd->bl);
	}

	if (g->guild.member[0].sd && g->guild.member[0].sd->fd) {
		clif_displaymessage(g->guild.member[0].sd->fd, msg_txt(g->guild.member[pos].sd,679)); //"You have become the Guild Master!"
		g->guild.member[0].sd->state.gmaster_flag = 1;
		clif_name_area(&g->guild.member[0].sd->bl);
		//Block his skills to prevent abuse.
#ifndef RENEWAL
		if (battle_config.guild_skill_relog_delay)
			guild_block_skill(g->guild.member[0].sd, battle_config.guild_skill_relog_delay);
#endif
	}

	// announce the change to all guild members
	for( i = 0; i < g->guild.max_member; i++ ) {
		if( g->guild.member[i].sd ){
			clif_guild_basicinfo( *g->guild.member[i].sd );
			clif_guild_memberlist( *g->guild.member[i].sd );
			clif_guild_belonginfo( *g->guild.member[i].sd ); // Update clientside guildmaster flag
		}
	}

	// Store changing time
	g->guild.last_leader_change = time;

	return 1;
}

/** Disband a guild
* @param sd Player who breaks the guild
* @param name Guild name
*/
int guild_break( map_session_data& sd, const char* name ){
	struct unit_data *ud;
	int i;
#ifdef BOUND_ITEMS
	int j;
	int idxlist[MAX_INVENTORY];
#endif

	auto& g = sd.guild;

	if( g == nullptr ){
		return 0;
	}

	if (strcmp(g->guild.name,name) != 0)
		return 0;

	if( !sd.state.gmaster_flag ){
		return 0;
	}

	for (i = 0; i < g->guild.max_member; i++) {
		if(	g->guild.member[i].account_id>0 && (
			g->guild.member[i].account_id != sd.status.account_id ||
			g->guild.member[i].char_id != sd.status.char_id ) )
			break;
	}
	if (i < g->guild.max_member) {
		clif_guild_broken(sd,2);
		return 0;
	}

	// Guild locked.
	if( map_getmapflag( sd.bl.m, MF_GUILDLOCK ) ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 228 ) );
		return 0;
	}

	if( g->instance_id ){
		if( battle_config.instance_block_leave ){
			return 0;
		}

		instance_destroy(g->instance_id);
	}

	/* Regardless of char server allowing it, we clear the guild master's auras */
	if( ( ud = unit_bl2ud( &sd.bl ) ) ){
		std::vector<std::shared_ptr<s_skill_unit_group>> group;

		for (const auto su : ud->skillunits) {
			switch (su->skill_id) {
				case GD_LEADERSHIP:
				case GD_GLORYWOUNDS:
				case GD_SOULCOLD:
				case GD_HAWKEYES:
					group.push_back(su);
					break;
			}
		}

		for (auto it = group.begin(); it != group.end(); it++) {
			skill_delunitgroup(*it);
		}
	}

#ifdef BOUND_ITEMS
	//Guild bound item check - Removes the bound flag
	j = pc_bound_chk( &sd, BOUND_GUILD, idxlist );
	for(i = 0; i < j; i++)
		pc_delitem( &sd,idxlist[i], sd.inventory.u.items_inventory[idxlist[i]].amount, 0, 1, LOG_TYPE_BOUND_REMOVAL );
#endif

	intif_guild_break(g->guild.guild_id);
	return 1;
}

/**
 * Creates a list of guild castle IDs to be requested
 * from char-server.
 */
void guild_castle_map_init(void) {
	std::vector<int32> castle_ids;

	for( const auto &it : castle_db ){
		castle_ids.push_back( it.first );
	}

	if( !castle_ids.empty() && intif_guild_castle_dataload( castle_ids ) ){
		ShowStatus( "Requested '" CL_WHITE "%" PRIdPTR CL_RESET "' guild castles from char-server...\n", castle_ids.size() );
	}
}

/**
 * Setter function for members of guild_castle struct.
 * Handles all side-effects, like updating guardians.
 * Sends updated info to char-server for saving.
 * @param castle_id Castle ID
 * @param index Type of data to change
 * @param value New value
 */
int guild_castledatasave(int castle_id, int index, int value) {
	std::shared_ptr<guild_castle> gc = castle_db.find(castle_id);

	if (gc == nullptr) {
		ShowWarning("guild_castledatasave: guild castle '%d' not found\n", castle_id);
		return 0;
	}

	switch (index) {
	case CD_GUILD_ID: // The castle's owner has changed? Update or remove Guardians too. [Skotlex]
	{
		int i;
		gc->guild_id = value;
		for (i = 0; i < MAX_GUARDIANS; i++){
			struct mob_data *gd;
			if (gc->guardian[i].visible && (gd = map_id2md(gc->guardian[i].id)) != nullptr)
				mob_guardian_guildchange(gd);
		}
		break;
	}
	case CD_CURRENT_ECONOMY:
		gc->economy = value; break;
	case CD_CURRENT_DEFENSE: // defense invest change -> recalculate guardian hp
	{
		int i;
		gc->defense = value;
		for (i = 0; i < MAX_GUARDIANS; i++){
			struct mob_data *gd;
			if (gc->guardian[i].visible && (gd = map_id2md(gc->guardian[i].id)) != nullptr)
				status_calc_mob(gd, SCO_NONE);
		}
		break;
	}
	case CD_INVESTED_ECONOMY:
		gc->triggerE = value; break;
	case CD_INVESTED_DEFENSE:
		gc->triggerD = value; break;
	case CD_NEXT_TIME:
		gc->nextTime = value; break;
	case CD_PAY_TIME:
		gc->payTime = value; break;
	case CD_CREATE_TIME:
		gc->createTime = value; break;
	case CD_ENABLED_KAFRA:
		gc->visibleC = value; break;
	default:
		if (index >= CD_ENABLED_GUARDIAN00 && index < CD_MAX) {
			gc->guardian[index - CD_ENABLED_GUARDIAN00].visible = value;
			break;
		}
		ShowWarning("guild_castledatasave: index = '%d' is out of allowed range\n", index);
		return 0;
	}

	if (!intif_guild_castle_datasave(castle_id, index, value)) {
		guild_castle_reconnect(castle_id, index, value);
	}
	return 0;
}

void guild_castle_reconnect_sub(void *key, void *data, va_list ap) {
	int castle_id = GetWord((int)__64BPRTSIZE(key), 0);
	int index = GetWord((int)__64BPRTSIZE(key), 1);
	intif_guild_castle_datasave(castle_id, index, *(int *)data);
	aFree(data);
}

/**
 * Saves pending guild castle data changes when char-server is
 * disconnected.
 * On reconnect pushes all changes to char-server for saving.
 * @param castle_id
 * @param index
 * @param value
 */
void guild_castle_reconnect(int castle_id, int index, int value) {
	static struct linkdb_node *gc_save_pending = nullptr;

	if (castle_id < 0) { // char-server reconnected
		linkdb_foreach(&gc_save_pending, guild_castle_reconnect_sub);
		linkdb_final(&gc_save_pending);
	} else {
		int *data;
		CREATE(data, int, 1);
		*data = value;
		linkdb_replace(&gc_save_pending, (void*)__64BPRTSIZE((MakeDWord(castle_id, index))), data);
	}
}

/** Load castle data then invoke OnAgitInit* on last
* @param len
* @param gc Guild Castle data
*/
int guild_castledataloadack(int len, struct guild_castle *gc) {
	int i;
	int n = (len-4) / sizeof(struct guild_castle);
	int ev;

	nullpo_ret(gc);

	//Last owned castle in the list invokes ::OnAgitInit
	for( i = n-1; i >= 0 && !(gc[i].guild_id); --i );
	ev = i; // offset of castle or -1

	if( ev < 0 ) { //No castles owned, invoke OnAgitInit as it is.
		npc_event_doall( script_config.agit_init_event_name );
		npc_event_doall( script_config.agit_init2_event_name );
		npc_event_doall( script_config.agit_init3_event_name );
	} else // load received castles into memory, one by one
	for( i = 0; i < n; i++, gc++ ) {
		std::shared_ptr<guild_castle> c = castle_db.find(gc->castle_id);

		if (c == nullptr) {
			ShowError("guild_castledataloadack: castle id=%d not found.\n", gc->castle_id);
			continue;
		}

		// update map-server castle data with new info
		memcpy(&c->guild_id, &gc->guild_id, sizeof(struct guild_castle) - offsetof(struct guild_castle, guild_id));

		if( c->guild_id ) {
			if( i != ev )
				guild_request_info(c->guild_id);
			else { // last owned one
				char event_name[EVENT_NAME_LENGTH];

				snprintf( event_name, EVENT_NAME_LENGTH, "::%s", script_config.agit_init_event_name );
				guild_npc_request_info(c->guild_id, event_name);
				snprintf( event_name, EVENT_NAME_LENGTH, "::%s", script_config.agit_init2_event_name );
				guild_npc_request_info(c->guild_id, event_name);
				snprintf( event_name, EVENT_NAME_LENGTH, "::%s", script_config.agit_init3_event_name );
				guild_npc_request_info(c->guild_id, event_name);
			}
		}
	}
	ShowStatus("Received '" CL_WHITE "%d" CL_RESET "' guild castles from char-server.\n", n);
	return 0;
}

/**
 * Start WoE:FE and triggers all npc OnAgitStart
 */
bool guild_agit_start(void){
	if( agit_flag ){
		return false;
	}

	agit_flag = true;

	npc_event_runall( script_config.agit_start_event_name );

	return true;
}

/**
 * End WoE:FE and triggers all npc OnAgitEnd
 */
bool guild_agit_end(void){
	if( !agit_flag ){
		return false;
	}

	agit_flag = false;

	npc_event_runall( script_config.agit_end_event_name );

	return true;
}

/**
 * Start WoE:SE and triggers all npc OnAgitStart2
 */
bool guild_agit2_start(void){
	if( agit2_flag ){
		return false;
	}

	agit2_flag = true;

	npc_event_runall( script_config.agit_start2_event_name );

	return true;
}

/**
 * End WoE:SE and triggers all npc OnAgitEnd2
 */
bool guild_agit2_end(void){
	if( !agit2_flag ){
		return false;
	}

	agit2_flag = false;

	npc_event_runall( script_config.agit_end2_event_name );

	return true;
}

/**
 * Start WoE:TE and triggers all npc OnAgitStart3
 */
bool guild_agit3_start(void){
	if( agit3_flag ){
		return false;
	}

	agit3_flag = true;

	npc_event_runall( script_config.agit_start3_event_name );

	return true;
}

/**
 * End WoE:TE and triggers all npc OnAgitEnd3
 */
bool guild_agit3_end(void){
	if( !agit3_flag ){
		return false;
	}

	agit3_flag = false;

	npc_event_runall( script_config.agit_end3_event_name );

	return true;
}

// How many castles does this guild have?
int guild_checkcastles(const struct mmo_guild &g) {
	int nb_cas = 0;
	for (const auto &it : castle_db) {
		if (it.second->guild_id == g.guild_id)
			nb_cas++;
	}
	return nb_cas;
}

// Are these two guilds allied?
bool guild_isallied(int guild_id, int guild_id2) {
	int i;
	auto g = guild_search(guild_id);
	if (!g)
		return false;

	ARR_FIND( 0, MAX_GUILDALLIANCE, i, g->guild.alliance[i].guild_id == guild_id2 );
	return( i < MAX_GUILDALLIANCE && g->guild.alliance[i].opposition == 0 );
}

bool guild_has_permission( map_session_data& sd, enum e_guild_permission permission ){
	int position = guild_getposition( sd );

	if( position < 0 ){
		return false;
	}

	return ( sd.guild->guild.position[position].mode & permission ) != 0;
}

void guild_flag_add(struct npc_data *nd) {
	int i;

	/* check */
	for( i = 0; i < guild_flags_count; i++ ) {
		if( guild_flags[i] && guild_flags[i]->bl.id == nd->bl.id ) {
			return;/* exists, most likely updated the id. */
		}
	}

	i = guild_flags_count;/* save the current slot */
	/* add */
	RECREATE(guild_flags,struct npc_data*,++guild_flags_count);
	/* save */
	guild_flags[i] = nd;
}

void guild_flag_remove(struct npc_data *nd) {
	int i, cursor;
	if( guild_flags_count == 0 )
		return;
	/* find it */
	for( i = 0; i < guild_flags_count; i++ ) {
		if( guild_flags[i] && guild_flags[i]->bl.id == nd->bl.id ) {/* found */
			guild_flags[i] = nullptr;
			break;
		}
	}

	/* compact list */
	for( i = 0, cursor = 0; i < guild_flags_count; i++ ) {
		if( guild_flags[i] == nullptr )
			continue;

		if( cursor != i ) {
			memmove(&guild_flags[cursor], &guild_flags[i], sizeof(struct npc_data*));
		}

		cursor++;
	}
}

/**
 * @see DBApply
 */
static int eventlist_db_final(DBKey key, DBData *data, va_list ap) {
	struct eventlist *next = nullptr;
	struct eventlist *current = (struct eventlist *)db_data2ptr(data);
	while (current != nullptr) {
		next = current->next;
		aFree(current);
		current = next;
	}
	return 0;
}

/**
 * @see DBApply
 */
static int guild_expcache_db_final(DBKey key, DBData *data, va_list ap) {
	ers_free(expcache_ers, db_data2ptr(data));
	return 0;
}

/* called when scripts are reloaded/unloaded */
void guild_flags_clear(void) {
	int i;
	for( i = 0; i < guild_flags_count; i++ ) {
		if( guild_flags[i] )
			guild_flags[i] = nullptr;
	}

	guild_flags_count = 0;
}

void do_init_guild(void) {
	guild_expcache_db  = idb_alloc(DB_OPT_BASE);
	guild_infoevent_db = idb_alloc(DB_OPT_BASE);
	expcache_ers = ers_new(sizeof(struct guild_expcache),"guild.cpp::expcache_ers",ERS_OPT_NONE);

	guild_flags_count = 0;

	castle_db.load();
	guild_skill_tree_db.load();

	add_timer_func_list(guild_payexp_timer,"guild_payexp_timer");
	add_timer_func_list(guild_send_xy_timer, "guild_send_xy_timer");
	add_timer_interval(gettick()+GUILD_PAYEXP_INTERVAL,guild_payexp_timer,0,0,GUILD_PAYEXP_INTERVAL);
	add_timer_interval(gettick()+GUILD_SEND_XY_INTERVAL,guild_send_xy_timer,0,0,GUILD_SEND_XY_INTERVAL);
}

void do_final_guild(void) {
	for (auto &it : guild_db) {
		if (!it.second)
			continue;
		channel_delete(it.second->channel, false);
	}

	guild_db.clear();
	castle_db.clear();
	guild_expcache_db->destroy(guild_expcache_db,guild_expcache_db_final);
	guild_infoevent_db->destroy(guild_infoevent_db,eventlist_db_final);
	ers_destroy(expcache_ers);

	aFree(guild_flags);/* never empty; created on boot */
}
