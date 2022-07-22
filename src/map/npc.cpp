// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npc.hpp"

#include <errno.h>
#include <map>
#include <stdlib.h>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/db.hpp"
#include "../common/ers.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"

#include "battle.hpp"
#include "chat.hpp"
#include "clif.hpp"
#include "date.hpp" // days of week enum
#include "guild.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "log.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "pc.hpp"
#include "pet.hpp"
#include "script.hpp" // script_config

using namespace rathena;

struct npc_data* fake_nd;

// linked list of npc source files
struct npc_src_list {
	struct npc_src_list* next;
	char name[4]; // dynamic array, the structure is allocated with extra bytes (string length)
};
static struct npc_src_list* npc_src_files = NULL;

static int npc_id=START_NPC_NUM;
static int npc_warp=0;
static int npc_shop=0;
static int npc_script=0;
static int npc_mob=0;
static int npc_delay_mob=0;
static int npc_cache_mob=0;

struct eri *npc_sc_display_ers;

// Market Shop
#if PACKETVER >= 20131223
struct s_npc_market {
	struct npc_item_list *list;
	char exname[NPC_NAME_LENGTH+1];
	uint16 count;
};
static DBMap *NPCMarketDB; /// Stock persistency! Temporary market stocks from `market` table. struct s_npc_market, key: NPC exname
static void npc_market_checkall(void);
static void npc_market_fromsql(void);
#define npc_market_delfromsql(exname,nameid) (npc_market_delfromsql_((exname), (nameid), false))
#define npc_market_clearfromsql(exname) (npc_market_delfromsql_((exname), 0, true))
#endif

/// Returns a new npc id that isn't being used in id_db.
/// Fatal error if nothing is available.
int npc_get_new_npc_id(void) {
	if( npc_id >= START_NPC_NUM && !map_blid_exists(npc_id) )
		return npc_id++;// available
	else {// find next id
		int base_id = npc_id;
		while( base_id != ++npc_id ) {
			if( npc_id < START_NPC_NUM )
				npc_id = START_NPC_NUM;
			if( !map_blid_exists(npc_id) )
				return npc_id++;// available
		}
		// full loop, nothing available
		ShowFatalError("npc_get_new_npc_id: All ids are taken. Exiting...");
		exit(1);
	}
}

static DBMap* ev_db; // const char* event_name -> struct event_data*
static DBMap* npcname_db; // const char* npc_name -> struct npc_data*

struct event_data {
	struct npc_data *nd;
	int pos;
};

static struct eri *timer_event_ers; //For the npc timer data. [Skotlex]

/* hello */
static char *npc_last_path;
static char *npc_last_ref;

struct npc_path_data {
	char* path;
	unsigned short references;
};
struct npc_path_data *npc_last_npd;
static DBMap *npc_path_db;

//For holding the view data of npc classes. [Skotlex]
static struct view_data npc_viewdb[MAX_NPC_CLASS];
static struct view_data npc_viewdb2[MAX_NPC_CLASS2_END-MAX_NPC_CLASS2_START];

struct script_event_s{
	struct event_data *event;
	const char *event_name;
};

// Holds pointers to the commonly executed scripts for speedup. [Skotlex]
std::map<enum npce_event, std::vector<struct script_event_s>> script_event;

// Static functions
static struct npc_data* npc_create_npc( int16 m, int16 x, int16 y );
static void npc_parsename( struct npc_data* nd, const char* name, const char* start, const char* buffer, const char* filepath );

const std::string StylistDatabase::getDefaultLocation(){
	return std::string(db_path) + "/stylist.yml";
}

bool StylistDatabase::parseCostNode( std::shared_ptr<s_stylist_entry> entry, bool doram, const ryml::NodeRef& node ){
	std::shared_ptr<s_stylist_costs> costs = doram ? entry->doram : entry->human;
	bool costs_exists = costs != nullptr;

	if( !costs_exists ){
		costs = std::make_shared<s_stylist_costs>();
	}

	if( this->nodeExists( node, "Price" ) ){
		uint32 price;

		if( !this->asUInt32( node, "Price", price ) ){
			return false;
		}

		if( price > MAX_ZENY ){
			this->invalidWarning( node["Price"], "stylist_parseCostNode: Price %u is too high, capping to MAX_ZENY...\n", price );
			price = MAX_ZENY;
		}

		costs->price = price;
	}else{
		if( !costs_exists ){
			costs->price = 0;
		}
	}

	if( this->nodeExists( node, "RequiredItem" ) ){
		std::string item;

		if( !this->asString( node, "RequiredItem", item ) ){
			return false;
		}

		std::shared_ptr<item_data> id = item_db.search_aegisname( item.c_str() );

		if( id == nullptr ){
			this->invalidWarning( node["RequiredItem"], "stylist_parseCostNode: Unknown item \"%s\"...\n", item.c_str() );
			return false;
		}

		costs->requiredItem = id->nameid;
	}else{
		if( !costs_exists ){
			costs->requiredItem = 0;
		}
	}

	if( this->nodeExists( node, "RequiredItemBox" ) ){
		std::string item;

		if( !this->asString( node, "RequiredItemBox", item ) ){
			return false;
		}

		std::shared_ptr<item_data> id = item_db.search_aegisname( item.c_str() );

		if( id == nullptr ){
			this->invalidWarning( node["RequiredItemBox"], "stylist_parseCostNode: Unknown item \"%s\"...\n", item.c_str() );
			return false;
		}

		costs->requiredItemBox = id->nameid;
	}else{
		if( !costs_exists ){
			costs->requiredItemBox = 0;
		}
	}

	if( !costs_exists ){
		if( doram ){
			entry->doram = costs;
		}else{
			entry->human = costs;
		}
	}

	return true;
}

uint64 StylistDatabase::parseBodyNode( const ryml::NodeRef& node ){
	if( !this->nodesExist( node, { "Look", "Options" } ) ){
		return 0;
	}

	std::string look_str;

	if( !this->asString( node, "Look", look_str ) ){
		return 0;
	}

	int64 constant;

	if( !script_get_constant( ( "LOOK_" + look_str ).c_str(), &constant ) ){
		this->invalidWarning( node["Look"], "stylist_parseBodyNode: Invalid look %s.\n", look_str.c_str() );
		return 0;
	}

	switch( constant ){
		case LOOK_HEAD_TOP:
		case LOOK_HEAD_MID:
		case LOOK_HEAD_BOTTOM:
		case LOOK_HAIR:
		case LOOK_HAIR_COLOR:
		case LOOK_CLOTHES_COLOR:
		case LOOK_BODY2:
			break;
		default:
			this->invalidWarning( node["Look"], "stylist_parseBodyNode: Unsupported look value \"%s\"...\n", look_str.c_str() );
			return 0;
	}

	std::shared_ptr<s_stylist_list> list = this->find( (uint32)constant );
	bool exists = list != nullptr;
	uint64 count = 0;

	if( !exists ){
		list = std::make_shared<s_stylist_list>();
		list->look = (uint16)constant;
	}

	for( const ryml::NodeRef& optionNode : node["Options"] ){
		int16 index;

		if( !this->asInt16( optionNode, "Index", index ) ){
			return 0;
		}

		if( index == 0 ){
			this->invalidWarning( optionNode["Index"], "stylist_parseBodyNode: Unsupported index value \"%hd\"...\n", index );
			return 0;
		}

		std::shared_ptr<s_stylist_entry> entry = util::umap_find( list->entries, index );
		bool entry_exists = entry != nullptr;

		if( !entry_exists ){
			entry = std::make_shared<s_stylist_entry>();
			entry->look = list->look;
			entry->index = index;

			if( !this->nodesExist( optionNode, { "Value" } ) ){
				return 0;
			}
		}

		if( this->nodeExists( optionNode, "Value" ) ){
			uint32 value;

			switch( list->look ){
				case LOOK_HEAD_TOP:
				case LOOK_HEAD_MID:
				case LOOK_HEAD_BOTTOM: {
						std::string item;

						if( !this->asString( optionNode, "Value", item ) ){
							return 0;
						}

						std::shared_ptr<item_data> id = item_db.search_aegisname( item.c_str() );

						if( id == nullptr ){
							this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: Unknown item \"%s\"...\n", item.c_str() );
							return 0;
						}

						value = id->nameid;
					} break;
				case LOOK_HAIR:
					if( !this->asUInt32( optionNode, "Value", value ) ){
						return 0;
					}

					if( value < MIN_HAIR_STYLE ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: hair style \"%u\" is too low...\n", value );
						return 0;
					}else if( value > MAX_HAIR_STYLE ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: hair style \"%u\" is too high...\n", value );
						return 0;
					}
					break;
				case LOOK_HAIR_COLOR:
					if( !this->asUInt32( optionNode, "Value", value ) ){
						return 0;
					}

					if( value < MIN_HAIR_COLOR ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: hair color \"%u\" is too low...\n", value );
						return 0;
					}else if( value > MAX_HAIR_COLOR ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: hair color \"%u\" is too high...\n", value );
						return 0;
					}
					break;
				case LOOK_CLOTHES_COLOR:
					if( !this->asUInt32( optionNode, "Value", value ) ){
						return 0;
					}

					if( value < MIN_CLOTH_COLOR ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: cloth color \"%u\" is too low...\n", value );
						return 0;
					}else if( value > MAX_CLOTH_COLOR ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: cloth color \"%u\" is too high...\n", value );
						return 0;
					}
					break;
				case LOOK_BODY2:
					if( !this->asUInt32( optionNode, "Value", value ) ){
						return 0;
					}

					if( value < MIN_BODY_STYLE ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: body style \"%u\" is too low...\n", value );
						return 0;
					}else if( value > MAX_BODY_STYLE ){
						this->invalidWarning( optionNode["Value"], "stylist_parseBodyNode: body style \"%u\" is too high...\n", value );
						return 0;
					}
					break;
			}

			entry->value = value;
		}

		if( this->nodeExists( optionNode, "CostsHuman" ) ) {
			if( !this->parseCostNode( entry, false, optionNode["CostsHuman"] ) ){
				return 0;
			}
		}else{
			if( !entry_exists ){
				entry->human = nullptr;
			}
		}

		if( this->nodeExists( optionNode, "CostsDoram" ) ) {
			if( !this->parseCostNode( entry, true, optionNode["CostsDoram"] ) ){
				return 0;
			}
		}else{
			if( !entry_exists ){
				entry->doram = nullptr;
			}
		}

		if( !entry_exists ){
			list->entries[index] = entry;
		}

		count++;
	}

	if( !exists ){
		this->put( (uint32)constant, list );
	}

	return count;
}

StylistDatabase stylist_db;

const std::string BarterDatabase::getDefaultLocation(){
	return "npc/barters.yml";
}

uint64 BarterDatabase::parseBodyNode( const ryml::NodeRef& node ){
	std::string npcname;

	if( !this->asString( node, "Name", npcname ) ){
		return 0;
	}

	std::shared_ptr<s_npc_barter> barter = this->find( npcname );
	bool exists = barter != nullptr;

	if( !exists ){
		barter = std::make_shared<s_npc_barter>();
		barter->name = npcname;
	}

	if( this->nodeExists( node, "Map" ) ){
		std::string map;

		if( !this->asString( node, "Map", map ) ){
			return 0;
		}

		uint16 index = mapindex_name2idx( map.c_str(), nullptr );

		if( index == 0 ){
			this->invalidWarning( node["Map"], "barter_parseBodyNode: Unknown mapname %s, skipping.\n", map.c_str());
			return 0;
		}

		barter->m = map_mapindex2mapid( index );

		// Skip silently if the map is not on this map-server
		if( barter->m < 0 ){
			return 1;
		}
	}else{
		if( !exists ){
			barter->m = -1;
		}
	}

	struct map_data* mapdata = nullptr;

	if( barter->m >= 0 ){
		mapdata = map_getmapdata( barter->m );
	}

	if( this->nodeExists( node, "X" ) ){
		uint16 x;

		if( !this->asUInt16( node, "X", x ) ){
			return 0;
		}

		if( mapdata == nullptr ){
			this->invalidWarning( node["X"], "barter_parseBodyNode: Barter NPC is not on a map. Ignoring X coordinate...\n" );
			x = 0;
		}else if( x >= mapdata->xs ){
			this->invalidWarning( node["X"], "barter_parseBodyNode: X coordinate %hu is out of bound %hu...\n", x, mapdata->xs );
			return 0;
		}

		barter->x = x;
	}else{
		if( !exists ){
			barter->x = 0;
		}
	}

	if( this->nodeExists( node, "Y" ) ){
		uint16 y;

		if( !this->asUInt16( node, "Y", y ) ){
			return 0;
		}

		if( mapdata == nullptr ){
			this->invalidWarning( node["Y"], "barter_parseBodyNode: Barter NPC is not on a map. Ignoring Y coordinate...\n" );
			y = 0;
		}else if( y >= mapdata->ys ){
			this->invalidWarning( node["Y"], "barter_parseBodyNode: Y coordinate %hu is out of bound %hu...\n", y, mapdata->ys );
			return 0;
		}

		barter->y = y;
	}else{
		if( !exists ){
			barter->y = 0;
		}
	}

	if( this->nodeExists( node, "Direction" ) ){
		std::string direction_name;

		if( !this->asString( node, "Direction", direction_name ) ){
			return 0;
		}

		int64 constant;

		if( !script_get_constant( ( "DIR_" + direction_name ).c_str(), &constant ) ){
			this->invalidWarning( node["Direction"], "barter_parseBodyNode: Unknown direction %s, skipping.\n", direction_name.c_str() );
			return 0;
		}

		if( constant < DIR_NORTH || constant >= DIR_MAX ){
			this->invalidWarning( node["Direction"], "barter_parseBodyNode: Invalid direction %s, defaulting to North.\n", direction_name.c_str() );
			constant = DIR_NORTH;
		}

		barter->dir = (uint8)constant;
	}else{
		if( !exists ){
			barter->dir = (uint8)DIR_NORTH;
		}
	}

	if( this->nodeExists( node, "Sprite" ) ){
		std::string sprite_name;

		if( !this->asString( node, "Sprite", sprite_name ) ){
			return 0;
		}

		int64 constant;

		if( !script_get_constant( sprite_name.c_str(), &constant ) ){
			this->invalidWarning( node["Sprite"], "barter_parseBodyNode: Unknown sprite name %s, skipping.\n", sprite_name.c_str());
			return 0;
		}

		if( constant != JT_FAKENPC && !npcdb_checkid( constant ) ){
			this->invalidWarning( node["Sprite"], "barter_parseBodyNode: Invalid sprite name %s, skipping.\n", sprite_name.c_str());
			return 0;
		}

		barter->sprite = (int16)constant;
	}else{
		if( !exists ){
			barter->sprite = JT_FAKENPC;
		}
	}

	if( this->nodeExists( node, "Items" ) ){
		for( const ryml::NodeRef& itemNode : node["Items"] ){
			uint16 index;

			if( !this->asUInt16( itemNode, "Index", index ) ){
				return 0;
			}

			std::shared_ptr<s_npc_barter_item> item = util::map_find( barter->items, index );
			bool item_exists = item != nullptr;

			if( !item_exists ){
				if( !this->nodesExist( itemNode, { "Item" } ) ){
					return 0;
				}

				item = std::make_shared<s_npc_barter_item>();
				item->index = index;
			}

			if( this->nodeExists( itemNode, "Item" ) ){
				std::string aegis_name;

				if( !this->asString( itemNode, "Item", aegis_name ) ){
					return 0;
				}

				std::shared_ptr<item_data> id = item_db.search_aegisname( aegis_name.c_str() );

				if( id == nullptr ){
					this->invalidWarning( itemNode["Item"], "barter_parseBodyNode: Unknown item %s.\n", aegis_name.c_str() );
					return 0;
				}

				item->nameid = id->nameid;
			}

			if( this->nodeExists( itemNode, "Stock" ) ){
				uint32 stock;

				if( !this->asUInt32( itemNode, "Stock", stock ) ){
					return 0;
				}

				item->stock = stock;
				item->stockLimited = ( stock > 0 );
			}else{
				if( !item_exists ){
					item->stock = 0;
					item->stockLimited = false;
				}
			}

			if( this->nodeExists( itemNode, "Zeny" ) ){
				uint32 zeny;

				if( !this->asUInt32( itemNode, "Zeny", zeny ) ){
					return 0;
				}

				if( zeny > MAX_ZENY ){
					this->invalidWarning( itemNode["Zeny"], "barter_parseBodyNode: Zeny price %u is above MAX_ZENY (%u), capping...\n", zeny, MAX_ZENY );
					zeny = MAX_ZENY;
				}

				item->price = zeny;
			}else{
				if( !item_exists ){
					item->price = 0;
				}
			}

			if( this->nodeExists( itemNode, "RequiredItems" ) ){
				for( const ryml::NodeRef& requiredItemNode : itemNode["RequiredItems"] ){
					uint16 requirement_index;

					if( !this->asUInt16( requiredItemNode, "Index", requirement_index ) ){
						return 0;
					}

					if( requirement_index >= MAX_BARTER_REQUIREMENTS ){
						this->invalidWarning( requiredItemNode["Index"], "barter_parseBodyNode: Index %hu is out of bounds. Barters support up to %d requirements.\n", requirement_index, MAX_BARTER_REQUIREMENTS );
						return 0;
					}

					std::shared_ptr<s_npc_barter_requirement> requirement = util::map_find( item->requirements, requirement_index );
					bool requirement_exists = requirement != nullptr;

					if( !requirement_exists ){
						if( !this->nodesExist( requiredItemNode, { "Item" } ) ){
							return 0;
						}

						requirement = std::make_shared<s_npc_barter_requirement>();
						requirement->index = requirement_index;
					}

					if( this->nodeExists( requiredItemNode, "Item" ) ){
						std::string aegis_name;

						if( !this->asString( requiredItemNode, "Item", aegis_name ) ){
							return 0;
						}

						std::shared_ptr<item_data> data = item_db.search_aegisname( aegis_name.c_str() );

						if( data == nullptr ){
							this->invalidWarning( requiredItemNode["Item"], "barter_parseBodyNode: Unknown required item %s.\n", aegis_name.c_str() );
							return 0;
						}

						requirement->nameid = data->nameid;
					}

					if( this->nodeExists( requiredItemNode, "Amount" ) ){
						uint16 amount;

						if( !this->asUInt16( requiredItemNode, "Amount", amount ) ){
							return 0;
						}

						if( amount > MAX_AMOUNT ){
							this->invalidWarning( requiredItemNode["Amount"], "barter_parseBodyNode: Amount %hu is too high, capping to %hu...\n", amount, MAX_AMOUNT );
							amount = MAX_AMOUNT;
						}

						requirement->amount = amount;
					}else{
						if( !requirement_exists ){
							requirement->amount = 1;
						}
					}

					if( this->nodeExists( requiredItemNode, "Refine" ) ){
						std::shared_ptr<item_data> data = item_db.find( requirement->nameid );

						if( data->flag.no_refine ){
							this->invalidWarning( requiredItemNode["Refine"], "barter_parseBodyNode: Item %s is not refineable.\n", data->name.c_str() );
							return 0;
						}

						int16 refine;

						if( !this->asInt16( requiredItemNode, "Refine", refine ) ){
							return 0;
						}

						if( refine > MAX_REFINE ){
							this->invalidWarning( requiredItemNode["Amount"], "barter_parseBodyNode: Refine %hd is too high, capping to %d.\n", refine, MAX_REFINE );
							refine = MAX_REFINE;
						}

						requirement->refine = (int8)refine;
					}else{
						if( !requirement_exists ){
							requirement->refine = -1;
						}
					}

					if( !requirement_exists ){
						item->requirements[requirement->index] = requirement;
					}
				}
			}

			if( !item_exists ){
				barter->items[index] = item;
			}
		}
	}

	if( !exists ){
		this->put( npcname, barter );
	}

	return 1;
}

void BarterDatabase::loadingFinished(){
	for( const auto& pair : *this ){
		if( !battle_config.feature_barter && !battle_config.feature_barter_extended ){
#ifndef BUILDBOT
			ShowError( "Barter system is not enabled.\n" );
#endif
			return;
		}

		std::shared_ptr<s_npc_barter> barter = pair.second;

		struct npc_data* nd = npc_create_npc( barter->m, barter->x, barter->y );

		npc_parsename( nd, barter->name.c_str(), nullptr, nullptr, __FILE__ ":" QUOTE(__LINE__) );

		nd->class_ = barter->sprite;
		nd->speed = 200;

		nd->bl.type = BL_NPC;
		nd->subtype = NPCTYPE_BARTER;

		nd->u.barter.extended = false;

		// Check if it has to use the extended barter feature or not
		for( const auto& itemPair : barter->items ){
			// Normal barter cannot have zeny requirements
			if( itemPair.second->price > 0 ){
				nd->u.barter.extended = true;
				break;
			}

			// Normal barter needs to have exchange items defined
			if( itemPair.second->requirements.empty() ){
				nd->u.barter.extended = true;
				break;
			}

			// Normal barter can only exchange 1:1
			if( itemPair.second->requirements.size() > 1 ){
				nd->u.barter.extended = true;
				break;
			}

			// Normal barter cannot handle refine
			for( const auto& requirement : itemPair.second->requirements ){
				if( requirement.second->refine >= 0 ){
					nd->u.barter.extended = true;
					break;
				}
			}

			// Check if a refine requirement has been set in the loop above
			if( nd->u.barter.extended ){
				break;
			}
		}

		if( nd->u.barter.extended && !battle_config.feature_barter_extended ){
#ifndef BUILDBOT
			ShowError( "Barter %s uses extended mechanics but this is not enabled.\n", nd->name );
#endif
			continue;
		}

		if( nd->bl.m >= 0 ){
			map_addnpc( nd->bl.m, nd );
			npc_setcells( nd );
			// Couldn't add on map
			if( map_addblock( &nd->bl ) ){
				continue;
			}
			
			status_change_init( &nd->bl );
			unit_dataset( &nd->bl );
			nd->ud.dir = barter->dir;

			if( nd->class_ != JT_FAKENPC ){
				status_set_viewdata( &nd->bl, nd->class_ );

				if( map_getmapdata( nd->bl.m )->users ){
					clif_spawn( &nd->bl );
				}
			}
		}else{
			map_addiddb( &nd->bl );
		}

		strdb_put( npcname_db, nd->exname, nd );

		for( const auto& itemPair : barter->items ){
			if( itemPair.second->stockLimited ){
				if( Sql_Query( mmysql_handle, "SELECT `amount` FROM `%s` WHERE `name` = '%s' AND `index` = '%hu'", barter_table, barter->name.c_str(), itemPair.first ) != SQL_SUCCESS ){
					Sql_ShowDebug( mmysql_handle );
					continue;
				}

				// Previous amount found
				if( SQL_SUCCESS == Sql_NextRow( mmysql_handle ) ){
					char* data;

					Sql_GetData( mmysql_handle, 0, &data, nullptr );

					itemPair.second->stock = strtoul( data, nullptr, 10 );
				}

				Sql_FreeResult( mmysql_handle );

				// Save or refresh the amount
				if( Sql_Query( mmysql_handle, "REPLACE INTO `%s` (`name`,`index`,`amount`) VALUES ( '%s', '%hu', '%hu' )", barter_table, barter->name.c_str(), itemPair.first, itemPair.second->stock ) != SQL_SUCCESS ){
					Sql_ShowDebug( mmysql_handle );
				}
			}else{
				if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `name` = '%s' AND `index` = '%hu'", barter_table, barter->name.c_str(), itemPair.first ) != SQL_SUCCESS ){
					Sql_ShowDebug( mmysql_handle );
				}
			}
		}
	}

	TypesafeYamlDatabase::loadingFinished();
}

BarterDatabase barter_db;

/**
 * Returns the viewdata for normal NPC classes.
 * @param class_: NPC class ID
 * @return viewdata or nullptr if the ID is invalid
 */
struct view_data* npc_get_viewdata(int class_) {
	if( class_ == JT_INVISIBLE )
		return &npc_viewdb[0];
	if (npcdb_checkid(class_)){
		if( class_ > MAX_NPC_CLASS2_START ){
			return &npc_viewdb2[class_-MAX_NPC_CLASS2_START];
		}else{
			return &npc_viewdb[class_];
		}
	}
	return nullptr;
}

int npc_isnear_sub(struct block_list* bl, va_list args) {
    struct npc_data *nd = (struct npc_data*)bl;

    if (nd->sc.option & (OPTION_HIDE|OPTION_INVISIBLE))
        return 0;

	int skill_id = va_arg(args, int);

	if (skill_id > 0) { //If skill_id > 0 that means is used for INF2_DISABLENEARNPC [Cydh]
		std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

		if (skill && skill->unit_nonearnpc_type) {
			if (skill->unit_nonearnpc_type&SKILL_NONEAR_WARPPORTAL && nd->subtype == NPCTYPE_WARP)
				return 1;
			if (skill->unit_nonearnpc_type&SKILL_NONEAR_SHOP && nd->subtype == NPCTYPE_SHOP)
				return 1;
			if (skill->unit_nonearnpc_type&SKILL_NONEAR_NPC && nd->subtype == NPCTYPE_SCRIPT)
				return 1;
			if (skill->unit_nonearnpc_type&SKILL_NONEAR_TOMB && nd->subtype == NPCTYPE_TOMB)
				return 1;
		}
		return 0;
	}

	return 1;
}

bool npc_isnear(struct block_list * bl) {

    if( battle_config.min_npc_vendchat_distance > 0 &&
            map_foreachinallrange(npc_isnear_sub,bl, battle_config.min_npc_vendchat_distance, BL_NPC, 0) )
        return true;

    return false;
}

int npc_ontouch_event(struct map_session_data *sd, struct npc_data *nd)
{
	char name[EVENT_NAME_LENGTH];

	if (pc_isdead(sd))	// Dead player don't trigger 'OnTouch_'
		return 0;

	if( nd->touching_id )
		return 0; // Attached a player already. Can't trigger on anyone else.

	// pc_ishiding moved in npc_event for now.
	// If OnTouch_ event exists hiding player doesn't click the npc.
	// if( pc_ishiding(sd) )
		// return 1; // Can't trigger 'OnTouch_'.

	if (util::vector_exists(sd->npc_ontouch_, nd->bl.id))
		return 0;

	safesnprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script_config.ontouch_event_name);
	return npc_event(sd,name,1);
}

int npc_ontouch2_event(struct map_session_data *sd, struct npc_data *nd)
{
	char name[EVENT_NAME_LENGTH];

	if (util::vector_exists(sd->areanpc, nd->bl.id))
		return 0;

	safesnprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script_config.ontouch2_event_name);
	return npc_event(sd,name,2);
}

int npc_touch_areanpc(struct map_session_data* sd, int16 m, int16 x, int16 y, struct npc_data *nd);

/*==========================================
 * Sub-function of npc_enable, runs OnTouch event when enabled
 *------------------------------------------*/
int npc_enable_sub(struct block_list *bl, va_list ap)
{
	struct npc_data *nd;

	nullpo_ret(bl);
	nullpo_ret(nd=va_arg(ap,struct npc_data *));
	if(bl->type == BL_PC)
	{
		TBL_PC *sd = (TBL_PC*)bl;
		npc_touch_areanpc(sd, bl->m, bl->x, bl->y, nd);
	}
	return 0;
}

bool npc_is_cloaked(struct npc_data* nd, struct map_session_data* sd) {
	bool npc_cloaked = (nd->sc.option & OPTION_CLOAK) ? true : false;

	if (std::find(sd->cloaked_npc.begin(), sd->cloaked_npc.end(), nd->bl.id) != sd->cloaked_npc.end())
		return (!npc_cloaked);
	return npc_cloaked;
}

static int npc_cloaked_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data* sd;

	nullpo_ret(bl);
	nullpo_ret(sd = (struct map_session_data *)bl);
	int id = va_arg(ap, int);

	auto it = std::find(sd->cloaked_npc.begin(), sd->cloaked_npc.end(), id);

	if (it != sd->cloaked_npc.end())
		sd->cloaked_npc.erase(it);

	return 1;
}

/*==========================================
 * Disable / Enable NPC
 *------------------------------------------*/
bool npc_enable_target(npc_data& nd, uint32 char_id, e_npcv_status flag)
{
	if (char_id > 0 && (flag & NPCVIEW_CLOAK)) {
		map_session_data *sd = map_charid2sd(char_id);
	
		if (!sd) {
			ShowError("npc_enable: Attempted to %s a NPC '%s' on an invalid target %d.\n", (flag & NPCVIEW_VISIBLE) ? "show" : "hide", nd.name, char_id);
			return false;
		}

		unsigned int option = nd.sc.option;
		if (flag & NPCVIEW_CLOAKOFF)
			nd.sc.option &= ~OPTION_CLOAK;
		else
			nd.sc.option |= OPTION_CLOAK;

		auto it = std::find(sd->cloaked_npc.begin(), sd->cloaked_npc.end(), nd.bl.id);
	
		if (it == sd->cloaked_npc.end() && option != nd.sc.option)
			sd->cloaked_npc.push_back(nd.bl.id);
		else if (it != sd->cloaked_npc.end() && option == nd.sc.option)
			sd->cloaked_npc.erase(it);
	
		if (nd.class_ != JT_WARPNPC && nd.class_ != JT_GUILD_FLAG)
			clif_changeoption_target(&nd.bl, &sd->bl);
		else {
			if (nd.sc.option&(OPTION_HIDE|OPTION_INVISIBLE|OPTION_CLOAK))
				clif_clearunit_single(nd.bl.id, CLR_OUTSIGHT, sd->fd);
			else
				clif_spawn(&nd.bl);
		}
		nd.sc.option = option;
	}
	else {
		if (flag & NPCVIEW_ENABLE) {
			nd.sc.option &= ~OPTION_INVISIBLE;
			clif_spawn(&nd.bl);
		}
		else if (flag & NPCVIEW_HIDEOFF)
			nd.sc.option &= ~OPTION_HIDE;
		else if (flag & NPCVIEW_HIDEON)
			nd.sc.option |= OPTION_HIDE;
		else if (flag & NPCVIEW_CLOAKOFF)
			nd.sc.option &= ~OPTION_CLOAK;
		else if (flag & NPCVIEW_CLOAKON)
			nd.sc.option |= OPTION_CLOAK;
		else {	//Can't change the view_data to invisible class because the view_data for all npcs is shared! [Skotlex]
			nd.sc.option |= OPTION_INVISIBLE;
			clif_clearunit_area(&nd.bl,CLR_OUTSIGHT);  // Hack to trick maya purple card [Xazax]
		}
		if (nd.class_ != JT_WARPNPC && nd.class_ != JT_GUILD_FLAG)	//Client won't display option changes for these classes [Toms]
			clif_changeoption(&nd.bl);
		else {
			if (nd.sc.option&(OPTION_HIDE|OPTION_INVISIBLE|OPTION_CLOAK))
				clif_clearunit_area(&nd.bl,CLR_OUTSIGHT);
			else
				clif_spawn(&nd.bl);
		}
		map_foreachinmap(npc_cloaked_sub, nd.bl.m, BL_PC, nd.bl.id);	// Because npc option has been updated we remove the npc id from sd->cloaked_npc
	}

	if (flag & NPCVIEW_VISIBLE) {	// check if player standing on a OnTouchArea
		int xs = -1, ys = -1;
		switch (nd.subtype) {
		case NPCTYPE_SCRIPT:
			xs = nd.u.scr.xs;
			ys = nd.u.scr.ys;
			break;
		case NPCTYPE_WARP:
			xs = nd.u.warp.xs;
			ys = nd.u.warp.ys;
			break;
		}
		if (xs > -1 && ys > -1)
			map_foreachinallarea( npc_enable_sub, nd.bl.m, nd.bl.x-xs, nd.bl.y-ys, nd.bl.x+xs, nd.bl.y+ys, BL_PC, &nd );
	}

	return true;
}

/*==========================================
 * NPC lookup (get npc_data through npcname)
 *------------------------------------------*/
struct npc_data* npc_name2id(const char* name)
{
	return (struct npc_data *) strdb_get(npcname_db, name);
}
/**
 * For the Secure NPC Timeout option (check src/config/secure.hpp)
 * @author RR
 **/
#ifdef SECURE_NPCTIMEOUT
/**
 * Timer to check for idle time and timeout the dialog if necessary
 **/
TIMER_FUNC(npc_secure_timeout_timer){
	struct map_session_data* sd = NULL;
	unsigned int timeout = NPC_SECURE_TIMEOUT_NEXT;
	t_tick cur_tick = gettick(); //ensure we are on last tick

	if ((sd = map_id2sd(id)) == NULL || !sd->npc_id || sd->state.ignoretimeout) {
		if( sd && sd->npc_idle_timer != INVALID_TIMER ){
			delete_timer( sd->npc_idle_timer, npc_secure_timeout_timer );
			sd->npc_idle_timer = INVALID_TIMER;
		}
		return 0; // Not logged in anymore OR no longer attached to a NPC OR using 'ignoretimeout' script command
	}

	switch( sd->npc_idle_type ) {
		case NPCT_INPUT:
			timeout = NPC_SECURE_TIMEOUT_INPUT;
			break;
		case NPCT_MENU:
			timeout = NPC_SECURE_TIMEOUT_MENU;
			break;
		//case NPCT_WAIT: var starts with this value
	}

	if( DIFF_TICK(cur_tick,sd->npc_idle_tick) > (timeout*1000) ) {
		pc_close_npc(sd,1);
	} else if(sd->st && (sd->st->state == END || sd->st->state == CLOSE)){
		// stop timer the script is already ending
		if( sd->npc_idle_timer != INVALID_TIMER ){
			delete_timer( sd->npc_idle_timer, npc_secure_timeout_timer );
			sd->npc_idle_timer = INVALID_TIMER;
		}
	} else { //Create a new instance of ourselves to continue
		sd->npc_idle_timer = add_timer(cur_tick + (SECURE_NPCTIMEOUT_INTERVAL*1000),npc_secure_timeout_timer,sd->bl.id,0);
	}
	return 0;
}
#endif

/*==========================================
 * Dequeue event and add timer for execution (100ms)
 *------------------------------------------*/
int npc_event_dequeue(struct map_session_data* sd,bool free_script_stack)
{
	nullpo_ret(sd);

	if(sd->npc_id)
	{	//Current script is aborted.
		if(sd->state.using_fake_npc){
			clif_clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
			sd->state.using_fake_npc = 0;
		}
		if (free_script_stack&&sd->st) {
			script_free_state(sd->st);
			sd->st = NULL;
		}
		sd->npc_id = 0;
	}

	if (!sd->eventqueue[0][0])
		return 0; //Nothing to dequeue

	if (!pc_addeventtimer(sd,100,sd->eventqueue[0]))
	{	//Failed to dequeue, couldn't set a timer.
		ShowWarning("npc_event_dequeue: event timer is full !\n");
		return 0;
	}
	//Event dequeued successfully, shift other elements.
	memmove(sd->eventqueue[0], sd->eventqueue[1], (MAX_EVENTQUEUE-1)*sizeof(sd->eventqueue[0]));
	sd->eventqueue[MAX_EVENTQUEUE-1][0]=0;
	return 1;
}

/*==========================================
 * exports a npc event label
 * called from npc_parse_script
 *------------------------------------------*/
static int npc_event_export(struct npc_data *nd, int i)
{
	char* lname = nd->u.scr.label_list[i].name;
	int pos = nd->u.scr.label_list[i].pos;
	if ((lname[0] == 'O' || lname[0] == 'o') && (lname[1] == 'N' || lname[1] == 'n')) {
		struct event_data *ev;
		char buf[EVENT_NAME_LENGTH];

		if (nd->bl.m > -1 && map_getmapdata(nd->bl.m)->instance_id > 0) { // Block script events in instances
			int j;

			for (j = 0; j < NPCE_MAX; j++) {
				if (strcmpi(npc_get_script_event_name(j), lname) == 0) {
					ShowWarning("npc_event_export: attempting to duplicate a script event in an instance (%s::%s), ignoring\n", nd->name, lname);
					return 0;
				}
			}
		}

		// NPC:<name> the prefix uses 4 characters
		if( !strncasecmp( lname, script_config.onwhisper_event_name, NAME_LENGTH ) && strlen(nd->exname) > ( NAME_LENGTH - 4 ) ){
			// The client only allows that many character so that NPC could not be whispered by unmodified clients
			ShowWarning( "Whisper event in npc '" CL_WHITE "%s" CL_RESET "' was ignored, because it's name is too long.\n", nd->exname );
			return 0;
		}

		snprintf(buf, ARRAYLENGTH(buf), "%s::%s", nd->exname, lname);
		// generate the data and insert it
		CREATE(ev, struct event_data, 1);
		ev->nd = nd;
		ev->pos = pos;
		if (strdb_put(ev_db, buf, ev)) // There was already another event of the same name?
			return 1;
	}
	return 0;
}

int npc_event_sub(struct map_session_data* sd, struct event_data* ev, const char* eventname); //[Lance]

/**
 * Exec name (NPC events) on player or global
 * Do on all NPC when called with foreach
 * @see DBApply
 */
int npc_event_doall_sub(DBKey key, DBData *data, va_list ap)
{
	const char* p = key.str;
	struct event_data* ev;
	int* c;
	const char* name;
	int rid;

	nullpo_ret(ev = (struct event_data*)db_data2ptr(data));
	nullpo_ret(c = va_arg(ap, int *));
	nullpo_ret(name = va_arg(ap, const char *));
	rid = va_arg(ap, int);

	p = strchr(p, ':'); // match only the event name
	if( p && strcmpi(name, p) == 0 /* && !ev->nd->src_id */ ) // Do not run on duplicates. [Paradox924X]
	{
		if(rid) // a player may only have 1 script running at the same time
			npc_event_sub(map_id2sd(rid),ev,key.str);
		else
			run_script(ev->nd->u.scr.script,ev->pos,rid,ev->nd->bl.id);
		(*c)++;
	}

	return 0;
}

/**
 * @see DBApply
 */
static int npc_event_do_sub(DBKey key, DBData *data, va_list ap)
{
	const char* p = key.str;
	struct event_data* ev;
	int* c, rid;
	const char* name;

	nullpo_ret(ev = (struct event_data*)db_data2ptr(data));
	nullpo_ret(c = va_arg(ap, int *));
	nullpo_ret(name = va_arg(ap, const char *));
	rid = va_arg(ap, int);

	if( p && strcmpi(name, p) == 0 )
	{
		run_script(ev->nd->u.scr.script,ev->pos,rid,ev->nd->bl.id);
		(*c)++;
	}

	return 0;
}

int npc_event_do_id(const char* name, int rid) {
	int c = 0;

	if( name[0] == ':' && name[1] == ':' )
		ev_db->foreach(ev_db,npc_event_doall_sub,&c,name,0,rid);
	else
		ev_db->foreach(ev_db,npc_event_do_sub,&c,name,rid);

	return c;
}

// runs the specified event (supports both single-npc and global events)
int npc_event_do(const char* name) {
	return npc_event_do_id(name, 0);
}

// runs the specified event (global only)
int npc_event_doall(const char* name)
{
	return npc_event_doall_id(name, 0);
}

// runs the specified event(global only) and reports call count
void npc_event_runall( const char* eventname ){
	ShowStatus( "Event '" CL_WHITE "%s" CL_RESET "' executed with '" CL_WHITE "%d" CL_RESET "' NPCs.\n", eventname, npc_event_doall( eventname ) );
}

// runs the specified event, with a RID attached (global only)
int npc_event_doall_id(const char* name, int rid)
{
	int c = 0;
	char buf[EVENT_NAME_LENGTH];
	safesnprintf(buf, sizeof(buf), "::%s", name);
	ev_db->foreach(ev_db,npc_event_doall_sub,&c,buf,rid);
	return c;
}

// runs the specified event on all NPCs with the given path
int npc_event_doall_path( const char* event_name, const char* path ){
	s_mapiterator* iter = mapit_geteachnpc();
	npc_data* nd;
	int count = 0;

	while( ( nd = (npc_data*)mapit_next( iter ) ) != nullptr ){
		if( nd->path && strcasecmp( nd->path, path ) == 0 ){
			char name[EVENT_NAME_LENGTH];

			safesnprintf( name, EVENT_NAME_LENGTH, "%s::%s", nd->exname, event_name );

			count += npc_event_do( name );
		}
	}

	ShowStatus( "Event '" CL_WHITE "%s" CL_RESET "' executed with '" CL_WHITE "%d" CL_RESET "' NPCs.\n", event_name, count );

	mapit_free(iter);
	return count;
}

/*==========================================
 * Clock event execution
 * OnMinute/OnClock/OnHour/OnDay/OnDDHHMM
 *------------------------------------------*/
TIMER_FUNC(npc_event_do_clock){
	static struct tm ev_tm_b; // tracks previous execution time
	time_t timer;
	struct tm* t;
	char buf[EVENT_NAME_LENGTH];
	int c = 0;

	timer = time(NULL);
	t = localtime(&timer);

	if (t->tm_min != ev_tm_b.tm_min ) {
		const char* day = NULL;

		safesnprintf(buf,EVENT_NAME_LENGTH,"%s%02d",script_config.timer_minute_event_name,t->tm_min);
		c += npc_event_doall(buf);

		safesnprintf(buf,EVENT_NAME_LENGTH,"%s%02d%02d",script_config.timer_clock_event_name,t->tm_hour,t->tm_min);
		c += npc_event_doall(buf);

		switch (t->tm_wday) {
			case SUNDAY:	day = script_config.timer_sunday_event_name; break;
			case MONDAY:	day = script_config.timer_monday_event_name; break;
			case TUESDAY:	day = script_config.timer_tuesday_event_name; break;
			case WEDNESDAY:	day = script_config.timer_wednesday_event_name; break;
			case THURSDAY:	day = script_config.timer_thursday_event_name; break;
			case FRIDAY:	day = script_config.timer_friday_event_name; break;
			case SATURDAY:	day = script_config.timer_saturday_event_name; break;
		}

		if( day != NULL ){
			safesnprintf(buf,EVENT_NAME_LENGTH,"%s%02d%02d",day,t->tm_hour,t->tm_min);
			c += npc_event_doall(buf);
		}
	}

	if (t->tm_hour != ev_tm_b.tm_hour) {
		safesnprintf(buf,EVENT_NAME_LENGTH,"%s%02d",script_config.timer_hour_event_name,t->tm_hour);
		c += npc_event_doall(buf);
	}

	if (t->tm_mday != ev_tm_b.tm_mday) {
		safesnprintf(buf,EVENT_NAME_LENGTH,"%s%02d%02d",script_config.timer_day_event_name,t->tm_mon+1,t->tm_mday);
		c += npc_event_doall(buf);
	}

	memcpy(&ev_tm_b,t,sizeof(ev_tm_b));
	return c;
}

/*==========================================
 * OnInit Event execution (the start of the event and watch)
 *------------------------------------------*/
void npc_event_do_oninit(void)
{
	npc_event_runall(script_config.init_event_name);

	add_timer_interval(gettick()+100,npc_event_do_clock,0,0,1000);
}

/*==========================================
 * Incorporation of the label for the timer event
 * called from npc_parse_script
 *------------------------------------------*/
int npc_timerevent_export(struct npc_data *nd, int i)
{
	int t = 0, k = 0;
	char *lname = nd->u.scr.label_list[i].name;
	int pos = nd->u.scr.label_list[i].pos;
	size_t len = strlen(script_config.timer_event_name);

	// Check if the label name starts with OnTimer(default) and then parse the seconds right after it
	if ( !strncmp(lname,script_config.timer_event_name,len) && sscanf( (lname += len), "%11d%n", &t, &k) == 1 && lname[k] == '\0') {
		// Timer event
		struct npc_timerevent_list *te = nd->u.scr.timer_event;
		int j, k2 = nd->u.scr.timeramount;
		if (te == NULL)
			te = (struct npc_timerevent_list *)aMalloc(sizeof(struct npc_timerevent_list));
		else
			te = (struct npc_timerevent_list *)aRealloc( te, sizeof(struct npc_timerevent_list) * (k2+1) );
		for (j = 0; j < k2; j++) {
			if (te[j].timer > t) {
				memmove(te+j+1, te+j, sizeof(struct npc_timerevent_list)*(k2-j));
				break;
			}
		}
		te[j].timer = t;
		te[j].pos = pos;
		nd->u.scr.timer_event = te;
		nd->u.scr.timeramount++;
	}
	return 0;
}

struct timer_event_data {
	int rid; //Attached player for this timer.
	int next; //timer index (starts with 0, then goes up to nd->u.scr.timeramount)
	int time; //holds total time elapsed for the script from when timer was started to when last time the event triggered.
};

/*==========================================
 * triger 'OnTimerXXXX' events
 *------------------------------------------*/
TIMER_FUNC(npc_timerevent){
	int old_rid;
	t_tick old_timer;
	t_tick old_tick;
	struct npc_data* nd=(struct npc_data *)map_id2bl(id);
	struct npc_timerevent_list *te;
	struct timer_event_data *ted = (struct timer_event_data*)data;
	struct map_session_data *sd=NULL;

	if( nd == NULL )
	{
		ShowError("npc_timerevent: NPC not found??\n");
		return 0;
	}

	if( ted->rid && !(sd = map_id2sd(ted->rid)) )
	{
		ShowError("npc_timerevent: Attached player not found.\n");
		ers_free(timer_event_ers, ted);
		return 0;
	}

	// These stuffs might need to be restored.
	old_rid = nd->u.scr.rid;
	old_tick = nd->u.scr.timertick;
	old_timer = nd->u.scr.timer;

	// Set the values of the timer
	nd->u.scr.rid = sd?sd->bl.id:0;	//attached rid
	nd->u.scr.timertick = tick;		//current time tick
	nd->u.scr.timer = ted->time;	//total time from beginning to now

	// Locate the event
	te = nd->u.scr.timer_event + ted->next;

	// Arrange for the next event
	ted->next++;
	if( nd->u.scr.timeramount > ted->next )
	{
		int next;
		next = nd->u.scr.timer_event[ ted->next ].timer - nd->u.scr.timer_event[ ted->next - 1 ].timer;
		ted->time += next;
		if( sd )
			sd->npc_timer_id = add_timer(tick+next,npc_timerevent,id,(intptr_t)ted);
		else
			nd->u.scr.timerid = add_timer(tick+next,npc_timerevent,id,(intptr_t)ted);
	}
	else
	{
		if( sd )
			sd->npc_timer_id = INVALID_TIMER;
		else
			nd->u.scr.timerid = INVALID_TIMER;

		ers_free(timer_event_ers, ted);
	}

	// Run the script
	run_script(nd->u.scr.script,te->pos,nd->u.scr.rid,nd->bl.id);

	nd->u.scr.rid = old_rid; // Attached-rid should be restored anyway.
	if( sd )
	{ // Restore previous data, only if this timer is a player-attached one.
		nd->u.scr.timer = old_timer;
		nd->u.scr.timertick = old_tick;
	}

	return 0;
}
/*==========================================
 * Start/Resume NPC timer
 *------------------------------------------*/
int npc_timerevent_start(struct npc_data* nd, int rid)
{
	int j;
	t_tick tick = gettick();
	struct map_session_data *sd = NULL; //Player to whom script is attached.

	nullpo_ret(nd);

	// Check if there is an OnTimer Event
	ARR_FIND( 0, nd->u.scr.timeramount, j, nd->u.scr.timer_event[j].timer > nd->u.scr.timer );

	if( nd->u.scr.rid > 0 && !(sd = map_id2sd(nd->u.scr.rid)) )
	{ // Failed to attach timer to this player.
		ShowError("npc_timerevent_start: Attached player not found!\n");
		return 1;
	}

	// Check if timer is already started.
	if( sd )
	{
		if( sd->npc_timer_id != INVALID_TIMER )
			return 0;
	}
	else if( nd->u.scr.timerid != INVALID_TIMER || nd->u.scr.timertick )
		return 0;

	if (j < nd->u.scr.timeramount)
	{
		t_tick next;
		struct timer_event_data *ted;
		// Arrange for the next event
		ted = ers_alloc(timer_event_ers, struct timer_event_data);
		ted->next = j; // Set event index
		ted->time = nd->u.scr.timer_event[j].timer;
		next = nd->u.scr.timer_event[j].timer - nd->u.scr.timer;
		if( sd )
		{
			ted->rid = sd->bl.id; // Attach only the player if attachplayerrid was used.
			sd->npc_timer_id = add_timer(tick+next,npc_timerevent,nd->bl.id,(intptr_t)ted);
		}
		else
		{
			ted->rid = 0;
			nd->u.scr.timertick = tick; // Set when timer is started
			nd->u.scr.timerid = add_timer(tick+next,npc_timerevent,nd->bl.id,(intptr_t)ted);
		}
	}
	else if (!sd)
	{
		nd->u.scr.timertick = tick;
	}

	return 0;
}
/*==========================================
 * Stop NPC timer
 *------------------------------------------*/
int npc_timerevent_stop(struct npc_data* nd)
{
	struct map_session_data *sd = NULL;
	int *tid;

	nullpo_ret(nd);

	if( nd->u.scr.rid && !(sd = map_id2sd(nd->u.scr.rid)) )
	{
		ShowError("npc_timerevent_stop: Attached player not found!\n");
		return 1;
	}

	tid = sd?&sd->npc_timer_id:&nd->u.scr.timerid;
	if( *tid == INVALID_TIMER && (sd || !nd->u.scr.timertick) ) // Nothing to stop
		return 0;

	// Delete timer
	if ( *tid != INVALID_TIMER )
	{
		const struct TimerData *td = NULL;

		td = get_timer(*tid);
		if( td && td->data )
			ers_free(timer_event_ers, (void*)td->data);
		delete_timer(*tid,npc_timerevent);
		*tid = INVALID_TIMER;
	}

	if( !sd && nd->u.scr.timertick )
	{
		nd->u.scr.timer += DIFF_TICK(gettick(),nd->u.scr.timertick); // Set 'timer' to the time that has passed since the beginning of the timers
		nd->u.scr.timertick = 0; // Set 'tick' to zero so that we know it's off.
	}

	return 0;
}
/*==========================================
 * Aborts a running NPC timer that is attached to a player.
 *------------------------------------------*/
void npc_timerevent_quit(struct map_session_data* sd)
{
	const struct TimerData *td;
	struct npc_data* nd;
	struct timer_event_data *ted;

	// Check timer existance
	if( sd->npc_timer_id == INVALID_TIMER )
		return;
	if( !(td = get_timer(sd->npc_timer_id)) )
	{
		sd->npc_timer_id = INVALID_TIMER;
		return;
	}

	// Delete timer
	nd = (struct npc_data *)map_id2bl(td->id);
	ted = (struct timer_event_data*)td->data;
	delete_timer(sd->npc_timer_id, npc_timerevent);
	sd->npc_timer_id = INVALID_TIMER;

	// Execute OnTimerQuit
	if( nd && nd->bl.type == BL_NPC )
	{
		char buf[EVENT_NAME_LENGTH];
		struct event_data *ev;

		snprintf(buf, ARRAYLENGTH(buf), "%s::%s", nd->exname, script_config.timer_quit_event_name);
		ev = (struct event_data*)strdb_get(ev_db, buf);
		if( ev && ev->nd != nd )
		{
			ShowWarning("npc_timerevent_quit: Unable to execute \"%s\", two NPCs have the same event name [%s]!\n",script_config.timer_quit_event_name,buf);
			ev = NULL;
		}
		if( ev )
		{
			int old_rid;
			t_tick old_timer;
			t_tick old_tick;

			//Set timer related info.
			old_rid = (nd->u.scr.rid == sd->bl.id ? 0 : nd->u.scr.rid); // Detach rid if the last attached player logged off.
			old_tick = nd->u.scr.timertick;
			old_timer = nd->u.scr.timer;

			nd->u.scr.rid = sd->bl.id;
			nd->u.scr.timertick = gettick();
			nd->u.scr.timer = ted->time;

			//Execute label
			run_script(nd->u.scr.script,ev->pos,sd->bl.id,nd->bl.id);

			//Restore previous data.
			nd->u.scr.rid = old_rid;
			nd->u.scr.timer = old_timer;
			nd->u.scr.timertick = old_tick;
		}
	}
	ers_free(timer_event_ers, ted);
}

/*==========================================
 * Get the tick value of an NPC timer
 * If it's stopped, return stopped time
 *------------------------------------------*/
t_tick npc_gettimerevent_tick(struct npc_data* nd)
{
	t_tick tick;
	nullpo_ret(nd);

	// TODO: Get player attached timer's tick. Now we can just get it by using 'getnpctimer' inside OnTimer event.

	tick = nd->u.scr.timer; // The last time it's active(start, stop or event trigger)
	if( nd->u.scr.timertick ) // It's a running timer
		tick += DIFF_TICK(gettick(), nd->u.scr.timertick);

	return tick;
}

/*==========================================
 * Set tick for running and stopped timer
 *------------------------------------------*/
int npc_settimerevent_tick(struct npc_data* nd, int newtimer)
{
	bool flag;
	int old_rid;
	//struct map_session_data *sd = NULL;

	nullpo_ret(nd);

	// TODO: Set player attached timer's tick.

	old_rid = nd->u.scr.rid;
	nd->u.scr.rid = 0;

	// Check if timer is started
	flag = (nd->u.scr.timerid != INVALID_TIMER || nd->u.scr.timertick);

	if( flag ) npc_timerevent_stop(nd);
	nd->u.scr.timer = newtimer;
	if( flag ) npc_timerevent_start(nd, -1);

	nd->u.scr.rid = old_rid;
	return 0;
}

int npc_event_sub(struct map_session_data* sd, struct event_data* ev, const char* eventname)
{
	if ( sd->npc_id != 0 )
	{
		//Enqueue the event trigger.
		int i;
		ARR_FIND( 0, MAX_EVENTQUEUE, i, sd->eventqueue[i][0] == '\0' );
		if( i < MAX_EVENTQUEUE )
		{
			safestrncpy(sd->eventqueue[i],eventname,EVENT_NAME_LENGTH); //Event enqueued.
			return 0;
		}

		ShowWarning("npc_event: player's event queue is full, can't add event '%s' !\n", eventname);
		return 1;
	}
	if( ev->nd->sc.option&OPTION_INVISIBLE )
	{
		//Disabled npc, shouldn't trigger event.
		npc_event_dequeue(sd);
		return 2;
	}

	char ontouch_event_name[EVENT_NAME_LENGTH];
	char ontouch2_event_name[EVENT_NAME_LENGTH];

	safesnprintf(ontouch_event_name, ARRAYLENGTH(ontouch_event_name), "%s::%s", ev->nd->exname, script_config.ontouch_event_name);
	safesnprintf(ontouch2_event_name, ARRAYLENGTH(ontouch2_event_name), "%s::%s", ev->nd->exname, script_config.ontouch2_event_name);

	// recheck some conditions for OnTouch/OnTouch_
	if (strcmp(eventname, ontouch_event_name) == 0 || strcmp(eventname, ontouch2_event_name) == 0) {
		int xs = ev->nd->u.scr.xs;
		int ys = ev->nd->u.scr.ys;
		int x = ev->nd->bl.x;
		int y = ev->nd->bl.y;

		if (x > 0 && y > 0 && (xs > -1 && ys > -1) && ((sd->bl.x < x - xs) || (sd->bl.x > x + xs) || (sd->bl.y < y - ys) || (sd->bl.y > y + ys)) ||
				(sd->state.block_action & PCBLOCK_NPCCLICK) || npc_is_cloaked(ev->nd, sd)) {
			npc_event_dequeue(sd);
			return 2;
		}
	}
	run_script(ev->nd->u.scr.script,ev->pos,sd->bl.id,ev->nd->bl.id);
	return 0;
}

/*==========================================
 * NPC processing event type
 *------------------------------------------*/
int npc_event(struct map_session_data* sd, const char* eventname, int ontouch)
{
	struct event_data* ev = (struct event_data*)strdb_get(ev_db, eventname);
	struct npc_data *nd;

	nullpo_ret(sd);

	if( ev == NULL || (nd = ev->nd) == NULL )
	{
		if( !ontouch )
			ShowError("npc_event: event not found [%s]\n", eventname);
		return ontouch;
	}

	if (ontouch == 1) { // OnTouch_
		if (pc_ishiding(sd))
			return 0;

		nd->touching_id = sd->bl.id;

		if (!util::vector_exists(sd->npc_ontouch_, nd->bl.id))
			sd->npc_ontouch_.push_back(nd->bl.id);
	} else if (ontouch == 2) { // OnTouch
		if (!util::vector_exists(sd->areanpc, nd->bl.id))
			sd->areanpc.push_back(nd->bl.id);
	}

	npc_event_sub(sd,ev,eventname); // Don't return this value so npc_enable_sub doesn't attempt to "click" the NPC if OnTouch fails.
	return 0;
}

/*==========================================
 * Sub chk then execute area event type
 *------------------------------------------*/
int npc_touch_areanpc_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;
	int pc_id;
	char *name;

	nullpo_ret(bl);
	nullpo_ret((sd = map_id2sd(bl->id)));

	pc_id = va_arg(ap,int);
	name = va_arg(ap,char*);

	if( sd->state.warping )
		return 0;
	if( pc_ishiding(sd) )
		return 0;
	if( pc_isdead(sd) )
		return 0;
	if( pc_id == sd->bl.id )
		return 0;

	npc_event(sd,name,1);

	return 1;
}

/*==========================================
 * Chk if sd is still touching his assigned npc.
 * If not, it unsets it and searches for another player in range.
 *------------------------------------------*/
int npc_touchnext_areanpc(struct map_session_data* sd, bool leavemap)
{
	if (sd->npc_ontouch_.empty())
		return 0;

	bool found = false;

	sd->npc_ontouch_.erase(std::remove_if(sd->npc_ontouch_.begin(), sd->npc_ontouch_.end(), [&] (const int &current_npc_id) {
		struct npc_data *nd = map_id2nd(current_npc_id);

		if (!nd) {
			return true;
		} else {
			int16 xs = nd->u.scr.xs;
			int16 ys = nd->u.scr.ys;

			// note : hiding doesn't reset the previous trigger status
			// player must leave the area to reset nd->touching_id on official
			if (sd->bl.m != nd->bl.m || sd->bl.x < nd->bl.x - xs || sd->bl.x > nd->bl.x + xs || sd->bl.y < nd->bl.y - ys || sd->bl.y > nd->bl.y + ys || leavemap) {
				char name[EVENT_NAME_LENGTH];

				if (nd->touching_id && nd->touching_id == sd->bl.id) {// empty when reload script
					found = true;
					nd->touching_id = 0;
					safesnprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script_config.ontouch_event_name);
					map_forcountinarea(npc_touch_areanpc_sub,nd->bl.m,nd->bl.x - xs,nd->bl.y - ys,nd->bl.x + xs,nd->bl.y + ys,1,BL_PC,sd->bl.id,name);
				}

				return true;
			}
		}

		return false;
	}), sd->npc_ontouch_.end());

	return found;
}

int npc_touch_areanpc(struct map_session_data* sd, int16 m, int16 x, int16 y, struct npc_data *nd)
{
	nullpo_retr(0, sd);
	nullpo_retr(0, nd);

	if (nd->sc.option&OPTION_INVISIBLE)
		return 1; // a npc was found, but it is disabled
	if (npc_is_cloaked(nd, sd))
		return 1;

	int xs = -1, ys = -1;
	switch(nd->subtype) {
	case NPCTYPE_WARP:
		xs = nd->u.warp.xs;
		ys = nd->u.warp.ys;
		break;
	case NPCTYPE_SCRIPT:
		xs = nd->u.scr.xs;
		ys = nd->u.scr.ys;
		break;
	default:
		return 0;
	}
	if (xs < 0 || ys < 0)
		return 0;
	if (x < (nd->bl.x - xs) || x > (nd->bl.x + xs) || y < (nd->bl.y - ys) || y > (nd->bl.y + ys))
		return 0;

	switch (nd->subtype) {
	case NPCTYPE_WARP:
		if ((!nd->trigger_on_hidden && (pc_ishiding(sd) || (sd->sc.count && sd->sc.data[SC_CAMOUFLAGE]))) || pc_isdead(sd))
			break; // hidden or dead chars cannot use warps
		if (!pc_job_can_entermap((enum e_job)sd->status.class_, map_mapindex2mapid(nd->u.warp.mapindex), pc_get_group_level(sd)))
			break;
		if (sd->count_rewarp > 10) {
			ShowWarning("Prevented infinite warp loop for player (%d:%d). Please fix NPC: '%s', path: '%s'\n", sd->status.account_id, sd->status.char_id, nd->exname, nd->path);
			sd->count_rewarp = 0;
			break;
		}
		pc_setpos(sd, nd->u.warp.mapindex, nd->u.warp.x, nd->u.warp.y, CLR_OUTSIGHT);
		return 2;
	case NPCTYPE_SCRIPT:
		// warp type sorted first, no need to check if they override any other OnTouch areas.

		if (npc_ontouch_event(sd, nd) > 0 && npc_ontouch2_event(sd, nd) > 0) { // failed to run OnTouch event, so just click the npc
			if (!util::vector_exists(sd->areanpc, nd->bl.id))
				sd->areanpc.push_back(nd->bl.id);
			if (sd->npc_id == 0)
				npc_click(sd, nd);
		}
		break;
	}
	return 1;
}

/*==========================================
 * Exec OnTouch for player if in range of area event
 *------------------------------------------*/
int npc_touch_area_allnpc(struct map_session_data* sd, int16 m, int16 x, int16 y)
{
	nullpo_retr(1, sd);

	// Remove NPCs that are no longer within the OnTouch area
	for (size_t i = 0; i < sd->areanpc.size(); i++) {
		struct npc_data *nd = map_id2nd(sd->areanpc[i]);

		if (!nd || nd->subtype != NPCTYPE_SCRIPT || !(nd->bl.m == m && x >= nd->bl.x - nd->u.scr.xs && x <= nd->bl.x + nd->u.scr.xs && y >= nd->bl.y - nd->u.scr.ys && y <= nd->bl.y + nd->u.scr.ys))
			util::erase_at(sd->areanpc, i);
	}

	if (sd->state.block_action & PCBLOCK_NPCCLICK)
		return 0;

	struct map_data *mapdata = map_getmapdata(m);
	int f = 1;

	for (int i = 0; i < mapdata->npc_num_area; i++) {
		switch( npc_touch_areanpc(sd, m, x, y, mapdata->npc[i]) ) {
		case 0:
			break;
		case 1:
			f = 0;
			break;
		case 2:
			return 0;
		}
	}

	if (f == 1) {
		ShowError("npc_touch_area_allnpc : stray NPC cell/NPC not found in the block on coordinates '%s',%d,%d\n", mapdata->name, x, y);
		return 1;
	}
	return 0;
}

// OnTouch NPC or Warp for Mobs
// Return 1 if Warped
int npc_touch_areanpc2(struct mob_data *md)
{
	int i, x = md->bl.x, y = md->bl.y, id;
	char eventname[EVENT_NAME_LENGTH];
	struct event_data* ev;
	int xs, ys;
	struct map_data *mapdata = map_getmapdata(md->bl.m);

	for( i = 0; i < mapdata->npc_num_area; i++ )
	{
		if( mapdata->npc[i]->sc.option&(OPTION_INVISIBLE|OPTION_CLOAK) )
			continue;

		switch( mapdata->npc[i]->subtype )
		{
			case NPCTYPE_WARP:
				if( !( battle_config.mob_warp&1 ) )
					continue;
				xs = mapdata->npc[i]->u.warp.xs;
				ys = mapdata->npc[i]->u.warp.ys;
				break;
			case NPCTYPE_SCRIPT:
				xs = mapdata->npc[i]->u.scr.xs;
				ys = mapdata->npc[i]->u.scr.ys;
				break;
			default:
				continue; // Keep Searching
		}
		if (xs < 0 || ys < 0)
			continue;

		if( x >= mapdata->npc[i]->bl.x-xs && x <= mapdata->npc[i]->bl.x+xs && y >= mapdata->npc[i]->bl.y-ys && y <= mapdata->npc[i]->bl.y+ys )
		{ // In the npc touch area
			switch( mapdata->npc[i]->subtype )
			{
				case NPCTYPE_WARP: {
					int16 warp_m = map_mapindex2mapid(mapdata->npc[i]->u.warp.mapindex);

					if( warp_m < 0 )
						break; // Cannot Warp between map servers
					if( unit_warp(&md->bl, warp_m, mapdata->npc[i]->u.warp.x, mapdata->npc[i]->u.warp.y, CLR_OUTSIGHT) == 0 )
						return 1; // Warped
				}
					break;
				case NPCTYPE_SCRIPT:
					if( mapdata->npc[i]->bl.id == md->areanpc_id )
						break; // Already touch this NPC
					safesnprintf(eventname, ARRAYLENGTH(eventname), "%s::%s", mapdata->npc[i]->exname, script_config.ontouchnpc_event_name);
					if( (ev = (struct event_data*)strdb_get(ev_db, eventname)) == NULL || ev->nd == NULL )
						break; // No OnTouchNPC Event
					md->areanpc_id = mapdata->npc[i]->bl.id;
					id = md->bl.id; // Stores Unique ID
					run_script(ev->nd->u.scr.script, ev->pos, md->bl.id, ev->nd->bl.id);
					if( map_id2md(id) == NULL ) return 1; // Not Warped, but killed
					break;
			}

			return 0;
		}
	}

	return 0;
}

/**
 * Checks if there are any NPC on-touch objects on the given range.
 * @param flag : Flag determines the type of object to check for
 *	&1: NPC Warps
 *	&2: NPCs with on-touch events.
 * @param m : mapindex
 * @param x : x coord
 * @param y : y coord
 * @param range : range to check
 * @return 0: no npc on target cells, x: npc_id
 */
int npc_check_areanpc(int flag, int16 m, int16 x, int16 y, int16 range)
{
	int i;
	int x0,y0,x1,y1;
	int xs,ys;

	if (range < 0) return 0;

	struct map_data *mapdata = map_getmapdata(m);

	x0 = i16max(x-range, 0);
	y0 = i16max(y-range, 0);
	x1 = i16min(x+range, mapdata->xs-1);
	y1 = i16min(y+range, mapdata->ys-1);

	//First check for npc_cells on the range given
	i = 0;
	for (ys = y0; ys <= y1 && !i; ys++) {
		for(xs = x0; xs <= x1 && !i; xs++){
			if (map_getcell(m,xs,ys,CELL_CHKNPC))
				i = 1;
		}
	}
	if (!i) return 0; //No NPC_CELLs.

	//Now check for the actual NPC on said range.
	for (i = 0; i < mapdata->npc_num_area; i++)
	{
		if (mapdata->npc[i]->sc.option&OPTION_INVISIBLE)
			continue;

		switch(mapdata->npc[i]->subtype)
		{
		case NPCTYPE_WARP:
			if (!(flag&1))
				continue;
			xs=mapdata->npc[i]->u.warp.xs;
			ys=mapdata->npc[i]->u.warp.ys;
			break;
		case NPCTYPE_SCRIPT:
			if (!(flag&2))
				continue;
			xs=mapdata->npc[i]->u.scr.xs;
			ys=mapdata->npc[i]->u.scr.ys;
			break;
		default:
			continue;
		}

		if( x1 >= mapdata->npc[i]->bl.x-xs && x0 <= mapdata->npc[i]->bl.x+xs
		&&  y1 >= mapdata->npc[i]->bl.y-ys && y0 <= mapdata->npc[i]->bl.y+ys )
			break; // found a npc
	}
	if (i == mapdata->npc_num_area)
		return 0;

	return (mapdata->npc[i]->bl.id);
}

/*==========================================
 * Chk if player not too far to access the npc.
 * Returns npc_data (success) or NULL (fail).
 *------------------------------------------*/
struct npc_data* npc_checknear(struct map_session_data* sd, struct block_list* bl)
{
	struct npc_data *nd;

	nullpo_retr(NULL, sd);
	if(bl == NULL) return NULL;
	if(bl->type != BL_NPC) return NULL;
	nd = (TBL_NPC*)bl;

	if(sd->state.using_fake_npc && sd->npc_id == bl->id)
		return nd;

	if (nd->class_<0) //Class-less npc, enable click from anywhere.
		return nd;

	if (bl->m!=sd->bl.m ||
	   bl->x<sd->bl.x-AREA_SIZE-1 || bl->x>sd->bl.x+AREA_SIZE+1 ||
	   bl->y<sd->bl.y-AREA_SIZE-1 || bl->y>sd->bl.y+AREA_SIZE+1)
		return NULL;

	return nd;
}

/*==========================================
 * Make NPC talk in global chat (like npctalk)
 *------------------------------------------*/
int npc_globalmessage(const char* name, const char* mes)
{
	struct npc_data* nd = npc_name2id(name);
	char temp[100];

	if (!nd)
		return 0;

	snprintf(temp, sizeof(temp), "%s", mes);
	clif_GlobalMessage(&nd->bl,temp,ALL_CLIENT);

	return 0;
}

// MvP tomb [GreenBox]
void run_tomb(struct map_session_data* sd, struct npc_data* nd)
{
	char buffer[200];
	char time[10];

	strftime(time, sizeof(time), "%H:%M", localtime(&nd->u.tomb.kill_time));

	// TODO: Find exact color?
	snprintf(buffer, sizeof(buffer), msg_txt(sd,657), nd->u.tomb.md->db->name.c_str());
	clif_scriptmes(sd, nd->bl.id, buffer);

	clif_scriptmes(sd, nd->bl.id, msg_txt(sd,658));

	snprintf(buffer, sizeof(buffer), msg_txt(sd,659), time);
	clif_scriptmes(sd, nd->bl.id, buffer);

	clif_scriptmes(sd, nd->bl.id, msg_txt(sd,660));

	snprintf(buffer, sizeof(buffer), msg_txt(sd,661), nd->u.tomb.killer_name[0] ? nd->u.tomb.killer_name : "Unknown");
	clif_scriptmes(sd, nd->bl.id, buffer);

	clif_scriptclose(sd, nd->bl.id);
}

/*==========================================
 * NPC 1st call when clicking on npc
 * Do specific action for NPC type (openshop, run scripts...)
 *------------------------------------------*/
int npc_click(struct map_session_data* sd, struct npc_data* nd)
{
	nullpo_retr(1, sd);

	if (sd->npc_id != 0) {
		ShowError("npc_click: npc_id != 0\n");
		return 1;
	}

	if(!nd) return 1;
	if ((nd = npc_checknear(sd,&nd->bl)) == NULL)
		return 1;
	//Hidden/Disabled npc.
	if (nd->class_ < 0 || nd->sc.option&(OPTION_INVISIBLE|OPTION_HIDE))
		return 1;

	if (sd->state.block_action & PCBLOCK_NPCCLICK) {
		clif_msg(sd, WORK_IN_PROGRESS);
		return 1;
	}

	switch(nd->subtype) {
		case NPCTYPE_SHOP:
			clif_npcbuysell(sd,nd->bl.id);
			break;
		case NPCTYPE_CASHSHOP:
		case NPCTYPE_ITEMSHOP:
		case NPCTYPE_POINTSHOP:
			clif_cashshop_show(sd,nd);
			break;
		case NPCTYPE_MARKETSHOP:
#if PACKETVER >= 20131223
			 {
				unsigned short i;

				for (i = 0; i < nd->u.shop.count; i++) {
					if (nd->u.shop.shop_item[i].qty)
						break;
				}

				if (i == nd->u.shop.count) {
					clif_messagecolor(&sd->bl, color_table[COLOR_RED], msg_txt(sd, 534), false, SELF);
					return false;
				}

				sd->npc_shopid = nd->bl.id;
				clif_npc_market_open(sd, nd);
			}
#endif
			break;
		case NPCTYPE_SCRIPT:
			run_script(nd->u.scr.script,0,sd->bl.id,nd->bl.id);
			break;
		case NPCTYPE_TOMB:
			run_tomb(sd,nd);
			break;
		case NPCTYPE_BARTER:
			sd->npc_shopid = nd->bl.id;
			if( nd->u.barter.extended ){
				clif_barter_extended_open( *sd, *nd );
			}else{
				clif_barter_open( *sd, *nd );
			}
			break;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
bool npc_scriptcont(struct map_session_data* sd, int id, bool closing){
	struct block_list *target = map_id2bl(id);

	nullpo_retr(true, sd);

#ifdef SECURE_NPCTIMEOUT
	if( !closing && sd->npc_idle_timer == INVALID_TIMER && !sd->state.ignoretimeout )
		return true;
#endif

	if( id != sd->npc_id ){
		TBL_NPC* nd_sd = (TBL_NPC*)map_id2bl(sd->npc_id);
		TBL_NPC* nd = BL_CAST(BL_NPC, target);

		ShowDebug("npc_scriptcont: %s (sd->npc_id=%d) is not %s (id=%d).\n",
			nd_sd?(char*)nd_sd->name:"'Unknown NPC'", (int)sd->npc_id,
			nd?(char*)nd->name:"'Unknown NPC'", (int)id);
		return true;
	}

	if(id != fake_nd->bl.id) { // Not item script
		if ((npc_checknear(sd, target)) == NULL) {
			ShowWarning("npc_scriptcont: failed npc_checknear test.\n");
			return true;
		}
	}
#ifdef SECURE_NPCTIMEOUT
	if( !closing )
		sd->npc_idle_tick = gettick(); //Update the last NPC iteration
#endif

	/**
	 * WPE can get to this point with a progressbar; we deny it.
	 **/
	if( sd->progressbar.npc_id && DIFF_TICK(sd->progressbar.timeout,gettick()) > 0 )
		return true;

	if( sd->st == nullptr ){
		return true;
	}

	if( closing ){
		switch( sd->st->state ){
			// close
			case CLOSE:
				sd->st->state = END;
				break;
			// close2
			case STOP:
				sd->st->state = RUN;
				break;
			default:
				sd->st->state = END;
				ShowError( "npc_scriptcont: unexpected state '%d' for closing call. (AID: %u CID: %u)\n", sd->st->state, sd->status.account_id, sd->status.char_id );
				break;
		}
	}else{
		switch( sd->st->state ){
			// next
			// progressbar
			case STOP:
				sd->st->state = RUN;
				break;
			// input
			// menu
			// select
			case RERUNLINE:
				// keep state as it is
				break;
			default:
				sd->st->state = END;
				ShowError( "npc_scriptcont: unexpected state '%d' for continue call. (AID: %u CID: %u)\n", sd->st->state, sd->status.account_id, sd->status.char_id );
				break;
		}
	}

	// Call this even, if it was set to end, because it will free the script state
	run_script_main(sd->st);

	return false;
}

/**
 * Open the shop Buy or Sell list
 * @param sd: Player data
 * @param id: NPC ID
 * @param type: 0 - Buy, 1 - Sell
 * @return 0 on success or 1 on failure
 */
int npc_buysellsel(struct map_session_data* sd, int id, int type)
{
	struct npc_data *nd;

	nullpo_retr(1, sd);

	if ((nd = npc_checknear(sd,map_id2bl(id))) == NULL)
		return 1;

	if (nd->subtype != NPCTYPE_SHOP) {
		ShowError("no such shop npc : %d\n",id);
		if (sd->npc_id == id)
			sd->npc_id=0;
		return 1;
	}
	if (nd->sc.option & OPTION_INVISIBLE) // can't buy if npc is not visible (hack?)
		return 1;

	sd->npc_shopid = id;

	if (type == 0) {
		clif_buylist(sd,nd);
	} else {
		clif_selllist(sd);
	}
	return 0;
}

/** Payment Process for NPCTYPE_CASHSHOP, NPCTYPE_ITEMSHOP, and NPCTYPE_POINTSHOP
 * @param nd NPC Shop data
 * @param price Price must be paid
 * @param points Amount of secondary points that player requested
 * @param sd Player data
 * @return e_CASHSHOP_ACK
 **/
static enum e_CASHSHOP_ACK npc_cashshop_process_payment(struct npc_data *nd, int price, int points, struct map_session_data *sd) {
	int cost[2] = { 0, 0 };

	npc_shop_currency_type(sd, nd, cost, false);

	switch(nd->subtype) {
		case NPCTYPE_CASHSHOP:
			if (cost[1] < points || cost[0] < (price - points))
				return ERROR_TYPE_MONEY;
			if (pc_paycash(sd, price, points, LOG_TYPE_NPC) <= 0) {
				return ERROR_TYPE_MONEY;
			}
			break;
		case NPCTYPE_ITEMSHOP:
			{
				std::shared_ptr<item_data> id = item_db.find(nd->u.shop.itemshop_nameid);
				int delete_amount = price, i;

				if (!id) { // Item Data is checked at script parsing but in case of item_db reload, check again.
					ShowWarning("Failed to find sellitem %u for itemshop NPC '%s' (%s, %d, %d)!\n", nd->u.shop.itemshop_nameid, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y);
					return ERROR_TYPE_PURCHASE_FAIL;
				}
				if (cost[1] < points || cost[0] < (price - points)) {
					char output[CHAT_SIZE_MAX];

					memset(output, '\0', sizeof(output));

					sprintf(output, msg_txt(sd, 712), id->ename.c_str(), id->nameid); // You do not have enough %s (%u).
					clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);
					return ERROR_TYPE_PURCHASE_FAIL;
				}

				for (i = 0; i < MAX_INVENTORY && delete_amount > 0; i++) {
					struct item *it;
					int amount = 0;

					if (sd->inventory.u.items_inventory[i].nameid == 0 || sd->inventory_data[i] == NULL || !(it = &sd->inventory.u.items_inventory[i]) || it->nameid != nd->u.shop.itemshop_nameid)
						continue;
					if (!pc_can_sell_item(sd, it, nd->subtype))
						continue;

					amount = it->amount;
					if (amount > delete_amount)
						amount = delete_amount;

					if (pc_delitem(sd, i, amount, 0, 0, LOG_TYPE_NPC)) {
						ShowWarning("Failed to delete item %u from '%s' at itemshop NPC '%s' (%s, %d, %d)!\n", nd->u.shop.itemshop_nameid, sd->status.name, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y);
						return ERROR_TYPE_PURCHASE_FAIL;
					}
					delete_amount -= amount;
				}
				if (delete_amount > 0) {
					ShowError("Item %u is not enough as payment at itemshop NPC '%s' (%s, %d, %d, AID=%d, CID=%d)!\n", nd->u.shop.itemshop_nameid, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y, sd->status.account_id, sd->status.char_id);
					return ERROR_TYPE_PURCHASE_FAIL;
				}
			}
			break;
		case NPCTYPE_POINTSHOP:
			{
				char output[CHAT_SIZE_MAX];

				memset(output, '\0', sizeof(output));

				if (cost[1] < points || cost[0] < (price - points)) {
					sprintf(output, msg_txt(sd, 713), nd->u.shop.pointshop_str); // You do not have enough '%s'.
					clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);
					return ERROR_TYPE_PURCHASE_FAIL;
				}
				pc_setreg2(sd, nd->u.shop.pointshop_str, cost[0] - (price - points));
				sprintf(output, msg_txt(sd, 716), nd->u.shop.pointshop_str, cost[0] - (price - points)); // Your '%s' is now: %d
				clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
			}
			break;
	}
	return ERROR_TYPE_NONE;
}

/**
 * Cash Shop Buy List for clients 2010-11-16 and newer
 * @param sd: Player data
 * @param points: Secondary point
 * @param count: Amount of items to purchase
 * @param item_list: List of items to purchase
 * @return clif_cashshop_ack value to display
 */
int npc_cashshop_buylist( struct map_session_data *sd, int points, std::vector<s_npc_buy_list>& item_list ){
	int i, j, amount, new_, w, vt;
	t_itemid nameid;
	struct npc_data *nd = (struct npc_data *)map_id2bl(sd->npc_shopid);
	enum e_CASHSHOP_ACK res;

	if( !nd || ( nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP ) )
		return ERROR_TYPE_NPC;
	if( sd->state.trading )
		return ERROR_TYPE_EXCHANGE;

	new_ = 0;
	w = 0;
	vt = 0; // Global Value

	// Validating Process ----------------------------------------------------
	for( i = 0; i < item_list.size(); i++ )
	{
		nameid = item_list[i].nameid;
		amount = item_list[i].qty;

		std::shared_ptr<item_data> id = item_db.find(nameid);

		if( !id || amount <= 0 )
			return ERROR_TYPE_ITEM_ID;

		ARR_FIND(0,nd->u.shop.count,j,nd->u.shop.shop_item[j].nameid == nameid || itemdb_viewid(nd->u.shop.shop_item[j].nameid) == nameid);
		if( j == nd->u.shop.count || nd->u.shop.shop_item[j].value <= 0 )
			return ERROR_TYPE_ITEM_ID;

		nameid = item_list[i].nameid = nd->u.shop.shop_item[j].nameid; //item_avail replacement

		if( !itemdb_isstackable2(id.get()) && amount > 1 )
		{
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %u!\n", sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
			amount = item_list[i].qty = 1;
		}

		if( nd->master_nd ) { // Script-controlled shops decide by themselves, what can be bought and for what price.
			continue;
		}

		switch( pc_checkadditem(sd,nameid,amount) )
		{
			case CHKADDITEM_NEW:
				new_ += id->inventorySlotNeeded(amount);
				break;
			case CHKADDITEM_OVERAMOUNT:
				return ERROR_TYPE_INVENTORY_WEIGHT;
		}

		vt += nd->u.shop.shop_item[j].value * amount;
		w += itemdb_weight(nameid) * amount;
	}

	if (nd->master_nd) //Script-based shops.
		return npc_buylist_sub(sd,item_list,nd->master_nd);

	if( w + sd->weight > sd->max_weight )
		return ERROR_TYPE_INVENTORY_WEIGHT;
	if( pc_inventoryblank(sd) < new_ )
		return ERROR_TYPE_INVENTORY_WEIGHT;
	if( points > vt ) points = vt;

	if ((res = npc_cashshop_process_payment(nd, vt, points, sd)) != ERROR_TYPE_NONE)
		return res;

	// Delivery Process ----------------------------------------------------
	for( i = 0; i < item_list.size(); i++ ) {
		nameid = item_list[i].nameid;
		amount = item_list[i].qty;

		if( !pet_create_egg(sd,nameid) ) {
			struct item item_tmp;
			unsigned short get_amt = amount;

			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = nameid;
			item_tmp.identify = 1;

			if ((itemdb_search(nameid))->flag.guid)
				get_amt = 1;

			for (j = 0; j < amount; j += get_amt)
				pc_additem(sd,&item_tmp,get_amt,LOG_TYPE_NPC);
		}
	}

	return ERROR_TYPE_NONE;
}

/**
 * Returns the shop currency type
 * @param sd: Player data
 * @param nd: NPC data
 * @param cost: Reference to cost variable
 * @param display: Display cost type to player?
 */
void npc_shop_currency_type(struct map_session_data *sd, struct npc_data *nd, int cost[2], bool display)
{
	nullpo_retv(sd);

	if (!nd) { // Assume it's Cash Shop through the button
		cost[0] = sd->cashPoints;
		cost[1] = sd->kafraPoints;
		return;
	}

	switch(nd->subtype) {
		case NPCTYPE_CASHSHOP:
			cost[0] = sd->cashPoints;
			cost[1] = sd->kafraPoints;
			break;
		case NPCTYPE_ITEMSHOP:
		{
			int total = 0, i;
			std::shared_ptr<item_data> id = item_db.find(nd->u.shop.itemshop_nameid);

			if (id) { // Item Data is checked at script parsing but in case of item_db reload, check again.
				if (display) {
					char output[CHAT_SIZE_MAX];

					memset(output, '\0', sizeof(output));

					sprintf(output, msg_txt(sd, 714), id->ename.c_str(), id->nameid); // Item Shop List: %s (%u)
					clif_broadcast(&sd->bl, output, strlen(output) + 1, BC_BLUE,SELF);
				}

				for (i = 0; i < MAX_INVENTORY; i++) {
					if (sd->inventory.u.items_inventory[i].amount > 0 && sd->inventory.u.items_inventory[i].nameid == id->nameid && pc_can_sell_item(sd, &sd->inventory.u.items_inventory[i], nd->subtype))
						total += sd->inventory.u.items_inventory[i].amount;
				}
			}

			cost[0] = total;
		}
			break;
		case NPCTYPE_POINTSHOP:
			if (display) {
				char output[CHAT_SIZE_MAX];

				memset(output, '\0', sizeof(output));

				sprintf(output, msg_txt(sd, 715), nd->u.shop.pointshop_str); // Point Shop List: '%s'
				clif_broadcast(&sd->bl, output, strlen(output) + 1, BC_BLUE,SELF);
			}
			
			cost[0] = static_cast<int>(pc_readreg2(sd, nd->u.shop.pointshop_str));
			break;
	}
}

/**
 * Cash Shop Buy List for clients 2010-11-15 and older
 * @param sd: Player data
 * @param nameid: Item to purchase
 * @param amount: Amount of items to purchase
 * @param points: Cost of total items
 * @return clif_cashshop_ack value to display
 */
int npc_cashshop_buy(struct map_session_data *sd, t_itemid nameid, int amount, int points)
{
	struct npc_data *nd = (struct npc_data *)map_id2bl(sd->npc_shopid);
	int i, price, w;
	enum e_CASHSHOP_ACK res;

	if( amount <= 0 )
		return ERROR_TYPE_ITEM_ID;

	if( points < 0 )
		return ERROR_TYPE_MONEY;

	if( !nd || (nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP) )
		return ERROR_TYPE_NPC;

	if( sd->state.trading )
		return ERROR_TYPE_EXCHANGE;

	std::shared_ptr<item_data> id = item_db.find(nameid);

	if( id == nullptr )
		return ERROR_TYPE_ITEM_ID; // Invalid Item

	ARR_FIND(0, nd->u.shop.count, i, nd->u.shop.shop_item[i].nameid == nameid || itemdb_viewid(nd->u.shop.shop_item[i].nameid) == nameid);
	if( i == nd->u.shop.count )
		return ERROR_TYPE_ITEM_ID;
	if( nd->u.shop.shop_item[i].value <= 0 )
		return ERROR_TYPE_ITEM_ID;

	nameid = nd->u.shop.shop_item[i].nameid; //item_avail replacement

	if(!itemdb_isstackable2(id.get()) && amount > 1)
	{
		ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %u!\n",
			sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
		amount = 1;
	}

	switch( pc_checkadditem(sd, nameid, amount) )
	{
		case CHKADDITEM_NEW:
			if( pc_inventoryblank(sd) < id->inventorySlotNeeded(amount) )
				return ERROR_TYPE_INVENTORY_WEIGHT;
			break;
		case CHKADDITEM_OVERAMOUNT:
			return ERROR_TYPE_INVENTORY_WEIGHT;
	}

	w = id->weight * amount;
	if( w + sd->weight > sd->max_weight )
		return ERROR_TYPE_INVENTORY_WEIGHT;

	if( (double)nd->u.shop.shop_item[i].value * amount > INT_MAX )
	{
		ShowWarning("npc_cashshop_buy: Item '%s' (%u) price overflow attempt!\n", id->name.c_str(), nameid);
		ShowDebug("(NPC:'%s' (%s,%d,%d), player:'%s' (%d/%d), value:%d, amount:%d)\n",
					nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y, sd->status.name, sd->status.account_id, sd->status.char_id, nd->u.shop.shop_item[i].value, amount);
		return ERROR_TYPE_ITEM_ID;
	}

	price = nd->u.shop.shop_item[i].value * amount;
	if( points > price )
		points = price;

	if ((res = npc_cashshop_process_payment(nd, price, points, sd)) != ERROR_TYPE_NONE)
		return res;

	if( !pet_create_egg(sd, nameid) ) {
		unsigned short get_amt = amount;

		struct item item_tmp = {};

		item_tmp.nameid = nameid;
		item_tmp.identify = 1;

		if (id->flag.guid)
			get_amt = 1;

		for (int j = 0; j < amount; j += get_amt)
			pc_additem(sd,&item_tmp, get_amt, LOG_TYPE_NPC);
	}

	return ERROR_TYPE_NONE;
}

/**
 * NPC buylist for script-controlled shops
 * @param sd: Player who bought
 * @param n: Number of items
 * @param item_list: List of items
 * @param nd: Attached NPC
 */
static int npc_buylist_sub(struct map_session_data* sd, std::vector<s_npc_buy_list>& item_list, struct npc_data* nd) {
	char npc_ev[EVENT_NAME_LENGTH];
	int key_nameid = 0, key_amount = 0;

	// discard old contents
	script_cleararray_pc( sd, "@bought_nameid" );
	script_cleararray_pc( sd, "@bought_quantity" );

	// save list of bought items
	for( int i = 0; i < item_list.size(); i++ ){
		script_setarray_pc( sd, "@bought_nameid", i, item_list[i].nameid, &key_nameid );
		script_setarray_pc( sd, "@bought_quantity", i, item_list[i].qty, &key_amount );
	}

	// invoke event
	snprintf(npc_ev, ARRAYLENGTH(npc_ev), "%s::%s", nd->exname, script_config.onbuy_event_name);
	npc_event(sd, npc_ev, 0);

	return 0;
}

/**
 * Shop buylist that the player is attempting to purchase
 * @param sd: Player who attempt to buy
 * @param n: Number of items
 * @param item_list: List of items
 * @return result code for clif_parse_NpcBuyListSend/clif_npc_market_purchase_ack
 */
e_purchase_result npc_buylist( struct map_session_data* sd, std::vector<s_npc_buy_list>& item_list ){
	struct npc_data* nd;
	struct npc_item_list *shop = NULL;
	double z;
	int j,k,w,skill,new_;
	uint8 market_index[MAX_INVENTORY];

	nullpo_retr(e_purchase_result::PURCHASE_FAIL_COUNT, sd);

	nd = npc_checknear(sd,map_id2bl(sd->npc_shopid));
	if( nd == NULL )
		return e_purchase_result::PURCHASE_FAIL_COUNT;
	if( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_MARKETSHOP )
		return e_purchase_result::PURCHASE_FAIL_COUNT;
	if( item_list.empty() ){
		return e_purchase_result::PURCHASE_FAIL_COUNT;
	}

	z = 0;
	w = 0;
	new_ = 0;

	shop = nd->u.shop.shop_item;

	memset(market_index, 0, sizeof(market_index));
	// process entries in buy list, one by one
	for( int i = 0; i < item_list.size(); ++i ){
		t_itemid nameid;
		unsigned short amount;
		int value;

		// find this entry in the shop's sell list
		ARR_FIND( 0, nd->u.shop.count, j,
			item_list[i].nameid == shop[j].nameid || //Normal items
			item_list[i].nameid == itemdb_viewid(shop[j].nameid) //item_avail replacement
		);

		if( j == nd->u.shop.count )
			return e_purchase_result::PURCHASE_FAIL_COUNT; // no such item in shop

#if PACKETVER >= 20131223
		if (nd->subtype == NPCTYPE_MARKETSHOP) {
			if (shop[j].qty >= 0 && item_list[i].qty > shop[j].qty)
				return e_purchase_result::PURCHASE_FAIL_COUNT;
			market_index[i] = j;
		}
#endif

		amount = item_list[i].qty;
		nameid = item_list[i].nameid = shop[j].nameid; //item_avail replacement
		value = shop[j].value;

		std::shared_ptr<item_data> id = item_db.find(nameid);

		if( !id )
			return e_purchase_result::PURCHASE_FAIL_COUNT; // item no longer in itemdb

		if( !itemdb_isstackable2(id.get()) && amount > 1 ) { //Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %u!\n",
				sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
			amount = item_list[i].qty = 1;
		}

		if( nd->master_nd ) { // Script-controlled shops decide by themselves, what can be bought and for what price.
			continue;
		}

		switch( pc_checkadditem(sd,nameid,amount) ) {
			case CHKADDITEM_EXIST:
				break;

			case CHKADDITEM_NEW:
				new_ += id->inventorySlotNeeded(amount);
				break;

			case CHKADDITEM_OVERAMOUNT:
				return e_purchase_result::PURCHASE_FAIL_WEIGHT;
		}

		if (npc_shop_discount(nd))
			value = pc_modifybuyvalue(sd,value);

		z += (double)value * amount;
		w += itemdb_weight(nameid) * amount;
	}

	if (nd->master_nd){ //Script-based shops.
		npc_buylist_sub(sd,item_list,nd->master_nd);
		return e_purchase_result::PURCHASE_SUCCEED;
	}

	if (z > (double)sd->status.zeny)
		return e_purchase_result::PURCHASE_FAIL_MONEY;	// Not enough Zeny

	if( w + sd->weight > sd->max_weight )
		return e_purchase_result::PURCHASE_FAIL_WEIGHT;	// Too heavy
	if( pc_inventoryblank(sd) < new_ )
		return e_purchase_result::PURCHASE_FAIL_COUNT;	// Not enough space to store items

	pc_payzeny(sd, (int)z, LOG_TYPE_NPC, NULL);

	for( int i = 0; i < item_list.size(); ++i ) {
		t_itemid nameid = item_list[i].nameid;
		unsigned short amount = item_list[i].qty;

#if PACKETVER >= 20131223
		if (nd->subtype == NPCTYPE_MARKETSHOP) {
			j = market_index[i];

			if( shop[j].qty >= 0 ){
				if (amount > shop[j].qty)
					return e_purchase_result::PURCHASE_FAIL_COUNT;
				shop[j].qty -= amount;
				npc_market_tosql(nd->exname, &shop[j]);
			}
		}
#endif

		if (itemdb_type(nameid) == IT_PETEGG)
			pet_create_egg(sd, nameid);
		else {
			unsigned short get_amt = amount;

			if ((itemdb_search(nameid))->flag.guid)
				get_amt = 1;

			for (k = 0; k < amount; k += get_amt) {
				struct item item_tmp;
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = nameid;
				item_tmp.identify = 1;

				pc_additem(sd,&item_tmp,get_amt,LOG_TYPE_NPC);
			}
		}
	}

	// custom merchant shop exp bonus
	if( battle_config.shop_exp > 0 && z > 0 && (skill = pc_checkskill(sd,MC_DISCOUNT)) > 0 ) {
		uint16 sk_idx = skill_get_index(MC_DISCOUNT);
		if( sd->status.skill[sk_idx].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill = sd->status.skill[sk_idx].flag - SKILL_FLAG_REPLACED_LV_0;

		if( skill > 0 ) {
			z = z * (double)skill * (double)battle_config.shop_exp/10000.;
			if( z < 1 )
				z = 1;
			pc_gainexp(sd,NULL,0,(int)z, 0);
		}
	}

	return e_purchase_result::PURCHASE_SUCCEED;
}

/// npc_selllist for script-controlled shops
static int npc_selllist_sub(struct map_session_data* sd, int n, unsigned short* item_list, struct npc_data* nd)
{
	char npc_ev[EVENT_NAME_LENGTH];
	char card_slot[NAME_LENGTH];
	char option_id[NAME_LENGTH], option_val[NAME_LENGTH], option_param[NAME_LENGTH];
	int i, j;
	int key_nameid = 0;
	int key_amount = 0;
	int key_refine = 0;
	int key_attribute = 0;
	int key_identify = 0;
	int key_enchantgrade = 0;
	int key_card[MAX_SLOTS];
	int key_option_id[MAX_ITEM_RDM_OPT], key_option_val[MAX_ITEM_RDM_OPT], key_option_param[MAX_ITEM_RDM_OPT];

	// discard old contents
	script_cleararray_pc( sd, "@sold_nameid" );
	script_cleararray_pc( sd, "@sold_quantity" );
	script_cleararray_pc( sd, "@sold_refine" );
	script_cleararray_pc( sd, "@sold_attribute" );
	script_cleararray_pc( sd, "@sold_identify" );
	script_cleararray_pc( sd, "@sold_enchantgrade" );

	for( j = 0; j < MAX_SLOTS; j++ )
	{// clear each of the card slot entries
		key_card[j] = 0;
		snprintf(card_slot, sizeof(card_slot), "@sold_card%d", j + 1);
		script_cleararray_pc( sd, card_slot );
	}

	for (j = 0; j < MAX_ITEM_RDM_OPT; j++) { // Clear each of the item option entries
		key_option_id[j] = key_option_val[j] = key_option_param[j] = 0;

		snprintf(option_id, sizeof(option_id), "@sold_option_id%d", j + 1);
		script_cleararray_pc( sd, option_id );
		snprintf(option_val, sizeof(option_val), "@sold_option_val%d", j + 1);
		script_cleararray_pc( sd, option_val );
		snprintf(option_param, sizeof(option_param), "@sold_option_param%d", j + 1);
		script_cleararray_pc( sd, option_param );
	}

	// save list of to be sold items
	for( i = 0; i < n; i++ )
	{
		int idx = item_list[i * 2] - 2;

		script_setarray_pc( sd, "@sold_nameid", i, sd->inventory.u.items_inventory[idx].nameid, &key_nameid );
		script_setarray_pc( sd, "@sold_quantity", i, item_list[i*2+1], &key_amount );

		if( itemdb_isequip(sd->inventory.u.items_inventory[idx].nameid) )
		{// process equipment based information into the arrays
			script_setarray_pc( sd, "@sold_refine", i, sd->inventory.u.items_inventory[idx].refine, &key_refine );
			script_setarray_pc( sd, "@sold_attribute", i, sd->inventory.u.items_inventory[idx].attribute, &key_attribute );
			script_setarray_pc( sd, "@sold_identify", i, sd->inventory.u.items_inventory[idx].identify, &key_identify );
			script_setarray_pc( sd, "@sold_enchantgrade", i, sd->inventory.u.items_inventory[idx].enchantgrade, &key_enchantgrade );

			for( j = 0; j < MAX_SLOTS; j++ )
			{// store each of the cards from the equipment in the array
				snprintf(card_slot, sizeof(card_slot), "@sold_card%d", j + 1);
				script_setarray_pc( sd, card_slot, i, sd->inventory.u.items_inventory[idx].card[j], &key_card[j] );
			}

			for (j = 0; j < MAX_ITEM_RDM_OPT; j++) { // Store each of the item options in the array
				snprintf(option_id, sizeof(option_id), "@sold_option_id%d", j + 1);
				script_setarray_pc( sd, option_id, i, sd->inventory.u.items_inventory[idx].option[j].id, &key_option_id[j] );
				snprintf(option_val, sizeof(option_val), "@sold_option_val%d", j + 1);
				script_setarray_pc( sd, option_val, i, sd->inventory.u.items_inventory[idx].option[j].value, &key_option_val[j] );
				snprintf(option_param, sizeof(option_param), "@sold_option_param%d", j + 1);
				script_setarray_pc( sd, option_param, i, sd->inventory.u.items_inventory[idx].option[j].param, &key_option_param[j] );
			}
		}
	}

	// invoke event
	snprintf(npc_ev, ARRAYLENGTH(npc_ev), "%s::%s", nd->exname, script_config.onsell_event_name);
	npc_event(sd, npc_ev, 0);
	return 0;
}


/// Player item selling to npc shop.
///
/// @param item_list 'n' pairs <index,amount>
/// @return result code for clif_parse_NpcSellListSend
uint8 npc_selllist(struct map_session_data* sd, int n, unsigned short *item_list)
{
	double z;
	int i,skill;
	struct npc_data *nd;

	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);

	if( ( nd = npc_checknear(sd, map_id2bl(sd->npc_shopid)) ) == NULL || nd->subtype != NPCTYPE_SHOP )
	{
		return 1;
	}

	z = 0;

	// verify the sell list
	for( i = 0; i < n; i++ )
	{
		t_itemid nameid;
		int amount, idx, value;

		idx    = item_list[i*2]-2;
		amount = item_list[i*2+1];

		if( idx >= MAX_INVENTORY || idx < 0 || amount < 0 )
		{
			return 1;
		}

		nameid = sd->inventory.u.items_inventory[idx].nameid;

		if( !nameid || !sd->inventory_data[idx] || sd->inventory.u.items_inventory[idx].amount < amount )
		{
			return 1;
		}

		if( nd->master_nd )
		{// Script-controlled shops decide by themselves, what can be sold and at what price.
			continue;
		}

		if (!pc_can_sell_item(sd, &sd->inventory.u.items_inventory[idx], nd->subtype)) {
			return 1; // In official server, this illegal attempt the player will be disconnected
		}

		if (battle_config.rental_item_novalue && sd->inventory.u.items_inventory[idx].expire_time)
			value = 0;
		else
			value = pc_modifysellvalue(sd, sd->inventory_data[idx]->value_sell);

		z+= (double)value*amount;
	}

	if( nd->master_nd )
	{// Script-controlled shops
		return npc_selllist_sub(sd, n, item_list, nd->master_nd);
	}

	// delete items
	for( i = 0; i < n; i++ )
	{
		int amount, idx;

		idx    = item_list[i*2]-2;
		amount = item_list[i*2+1];

		// Forged packet, we do not care if he loses items
		if( sd->inventory_data[idx] == nullptr ){
			return 1;
		}

		if( sd->inventory_data[idx]->type == IT_PETEGG && sd->inventory.u.items_inventory[idx].card[0] == CARD0_PET )
		{
			if( pet_db_search(sd->inventory.u.items_inventory[idx].nameid, PET_EGG) )
			{
				intif_delete_petdata(MakeDWord(sd->inventory.u.items_inventory[idx].card[1], sd->inventory.u.items_inventory[idx].card[2]));
			}
		}

		pc_delitem(sd, idx, amount, 0, 6, LOG_TYPE_NPC);
	}

	if( z > MAX_ZENY )
		z = MAX_ZENY;

	pc_getzeny(sd, (int)z, LOG_TYPE_NPC, NULL);

	// custom merchant shop exp bonus
	if( battle_config.shop_exp > 0 && z > 0 && ( skill = pc_checkskill(sd,MC_OVERCHARGE) ) > 0)
	{
		uint16 sk_idx = skill_get_index(MC_OVERCHARGE);
		if( sd->status.skill[sk_idx].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill = sd->status.skill[sk_idx].flag - SKILL_FLAG_REPLACED_LV_0;

		if( skill > 0 )
		{
			z = z * (double)skill * (double)battle_config.shop_exp/10000.;
			if( z < 1 )
				z = 1;
			pc_gainexp(sd, NULL, 0, (int)z, 0);
		}
	}

	return 0;
}

e_purchase_result npc_barter_purchase( struct map_session_data& sd, std::shared_ptr<s_npc_barter> barter, std::vector<s_barter_purchase>& purchases ){
	uint64 requiredZeny = 0;
	uint32 requiredWeight = 0;
	uint32 reducedWeight = 0;
	uint16 requiredSlots = 0;
	uint32 requiredItems[MAX_INVENTORY] = { 0 };

	for( s_barter_purchase& purchase : purchases ){
		purchase.data = item_db.find( purchase.item->nameid ).get();

		if( purchase.data == nullptr ){
			return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
		}

		uint32 amount = purchase.amount;

		if( purchase.item->stockLimited && purchase.item->stock < amount ){
			return e_purchase_result::PURCHASE_FAIL_STOCK_EMPTY;
		}

		char result = pc_checkadditem( &sd, purchase.item->nameid, amount );

		if( result == CHKADDITEM_OVERAMOUNT ){
			return e_purchase_result::PURCHASE_FAIL_COUNT;
		}else if( result == CHKADDITEM_NEW ){
			requiredSlots += purchase.data->inventorySlotNeeded( amount );
		}

		requiredZeny += ( purchase.item->price * amount );
		requiredWeight += ( purchase.data->weight * amount );

		for( const auto& requirementPair : purchase.item->requirements ){
			std::shared_ptr<s_npc_barter_requirement> requirement = requirementPair.second;
			std::shared_ptr<item_data> id = item_db.find(requirement->nameid);

			if( id == nullptr ){
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}

			if( itemdb_isstackable2( id.get() ) ){
				int j;

				for( j = 0; j < MAX_INVENTORY; j++ ){
					if( sd.inventory.u.items_inventory[j].nameid == requirement->nameid ){
						// Equipped items are not taken into account
						if( sd.inventory.u.items_inventory[j].equip != 0 ){
							continue;
						}

						// Items in equip switch are not taken into account
						if( sd.inventory.u.items_inventory[j].equipSwitch != 0 ){
							continue;
						}

						// Server is configured to hide favorite items on selling
						if( battle_config.hide_fav_sell && sd.inventory.u.items_inventory[j].favorite ){
							continue;
						}

						// Actually stackable items should never be refinable, but who knows...
						if( requirement->refine >= 0 && sd.inventory.u.items_inventory[j].refine != requirement->refine ){
							// Refine does not match, continue with next item
							continue;
						}

						// Found a match, accumulate required amount
						requiredItems[j] += requirement->amount * amount;

						// Check if there are still enough items available
						if( requiredItems[j] > sd.inventory.u.items_inventory[j].amount ){
							return e_purchase_result::PURCHASE_FAIL_GOODS;
						}

						// Cancel the loop
						break;
					}
				}

				// Required item not found
				if( j == MAX_INVENTORY ){
					return e_purchase_result::PURCHASE_FAIL_GOODS;
				}
			}else{
				for( int i = 0; i < (requirement->amount * amount); i++ ){
					int j;

					for( j = 0; j < MAX_INVENTORY; j++ ){
						if( sd.inventory.u.items_inventory[j].nameid == requirement->nameid ){
							// Equipped items are not taken into account
							if( sd.inventory.u.items_inventory[j].equip != 0 ){
								continue;
							}

							// Items in equip switch are not taken into account
							if( sd.inventory.u.items_inventory[j].equipSwitch != 0 ){
								continue;
							}

							// Server is configured to hide favorite items on selling
							if( battle_config.hide_fav_sell && sd.inventory.u.items_inventory[j].favorite ){
								continue;
							}

							// If necessary, check if the refine rate matches
							if( requirement->refine >= 0 && sd.inventory.u.items_inventory[j].refine != requirement->refine ){
								// Refine does not match, continue with next item
								continue;
							}

							// Found a match, since it is not stackable, check if it was already taken
							if( requiredItems[j] > 0 ){
								// Item was already taken, try to find another match
								continue;
							}

							// Mark it as taken
							requiredItems[j] = 1;

							// Cancel the loop
							break;
						}
					}

					// Required item not found
					if( j == MAX_INVENTORY ){
						// Maybe the refine level did not match
						if( requirement->refine >= 0 ){
							int refine;

							// Try to find a higher refine level, going from the next lowest to the highest possible
							for( refine = requirement->refine + 1; refine <= MAX_REFINE; refine++ ){
								for( j = 0; j < MAX_INVENTORY; j++ ){
									if( sd.inventory.u.items_inventory[j].nameid == requirement->nameid ){
										// Equipped items are not taken into account
										if( sd.inventory.u.items_inventory[j].equip != 0 ){
											continue;
										}

										// Items in equip switch are not taken into account
										if(	sd.inventory.u.items_inventory[j].equipSwitch != 0 ){
											continue;
										}

										// Server is configured to hide favorite items on selling
										if( battle_config.hide_fav_sell && sd.inventory.u.items_inventory[j].favorite ){
											continue;
										}

										// If necessary, check if the refine rate matches
										if( requirement->refine >= 0 && sd.inventory.u.items_inventory[j].refine != refine ){
											// Refine does not match, continue with next item
											continue;
										}

										// Found a match, since it is not stackable, check if it was already taken
										if( requiredItems[j] > 0 ){
											// Item was already taken, try to find another match
											continue;
										}

										// Mark it as taken
										requiredItems[j] = 1;

										// Cancel the loop
										break;
									}
								}

								// If a match was found, make sure to cancel the loop
								if( j < MAX_INVENTORY ){
									// Cancel the loop
									break;
								}
							}

							// No matching entry found
							if( refine > MAX_REFINE ){
								return e_purchase_result::PURCHASE_FAIL_GOODS;
							}
						}else{
							return e_purchase_result::PURCHASE_FAIL_GOODS;
						}
					}
				}
			}

			reducedWeight += ( purchase.amount * requirement->amount * id->weight );
		}
	}

	// Check if there is enough Zeny
	if( sd.status.zeny < requiredZeny ){
		return e_purchase_result::PURCHASE_FAIL_MONEY;
	}

	// Check if there is enough Weight Limit
	if( ( sd.weight + requiredWeight - reducedWeight ) > sd.max_weight ){
		return e_purchase_result::PURCHASE_FAIL_WEIGHT;
	}

	if( pc_inventoryblank( &sd ) < requiredSlots ){
		return e_purchase_result::PURCHASE_FAIL_COUNT;
	}

	for( int i = 0; i < MAX_INVENTORY; i++ ){
		if( requiredItems[i] > 0 ){
			if( pc_delitem( &sd, i, requiredItems[i], 0, 0, LOG_TYPE_BARTER ) != 0 ){
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}
		}
	}

	if( pc_payzeny( &sd, (int)requiredZeny, LOG_TYPE_BARTER, nullptr ) != 0 ){
		return e_purchase_result::PURCHASE_FAIL_MONEY;
	}

	for( s_barter_purchase& purchase : purchases ){
		if( purchase.item->stockLimited ){
			purchase.item->stock -= purchase.amount;

			if( Sql_Query( mmysql_handle, "REPLACE INTO `%s` (`name`,`index`,`amount`) VALUES ( '%s', '%hu', '%hu' )", barter_table, barter->name.c_str(), purchase.item->index, purchase.item->stock ) != SQL_SUCCESS ){
				Sql_ShowDebug( mmysql_handle );
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}
		}

		if( itemdb_isstackable2( purchase.data ) ){
			struct item it = {};

			it.nameid = purchase.item->nameid;
			it.identify = true;

			if( pc_additem( &sd, &it, purchase.amount, LOG_TYPE_BARTER ) != ADDITEM_SUCCESS ){
				return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
			}
		}else{
			if( purchase.data->type == IT_PETEGG ){
				for( int i = 0; i < purchase.amount; i++ ){
					if( !pet_create_egg( &sd, purchase.item->nameid ) ){
						return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
					}
				}
			}else{
				for( int i = 0; i < purchase.amount; i++ ){
					struct item it = {};

					it.nameid = purchase.item->nameid;
					it.identify = true;

					if( pc_additem( &sd, &it, 1, LOG_TYPE_BARTER ) != ADDITEM_SUCCESS ){
						return e_purchase_result::PURCHASE_FAIL_EXCHANGE_FAILED;
					}
				}
			}
		}
	}

	return e_purchase_result::PURCHASE_SUCCEED;
}


//Atempt to remove an npc from a map
//This doesn't remove it from map_db
int npc_remove_map(struct npc_data* nd)
{
	int i;
	nullpo_retr(1, nd);

	if(nd->bl.prev == NULL || nd->bl.m < 0)
		return 1; //Not assigned to a map.

	struct map_data *mapdata = map_getmapdata(nd->bl.m);

	if (nd->subtype == NPCTYPE_SCRIPT)
		skill_clear_unitgroup(&nd->bl);
	clif_clearunit_area(&nd->bl,CLR_RESPAWN);
	npc_unsetcells(nd);
	map_delblock(&nd->bl);
	//Remove npc from map[].npc list. [Skotlex]
	ARR_FIND( 0, mapdata->npc_num, i, mapdata->npc[i] == nd );
	if( i == mapdata->npc_num ) return 2; //failed to find it?

	mapdata->npc_num--;
	if (i >= mapdata->npc_num_area)
		mapdata->npc[i] = mapdata->npc[ mapdata->npc_num ];
	else if (i >= mapdata->npc_num_warp) {
		mapdata->npc_num_area--;
		mapdata->npc[i] = mapdata->npc[ mapdata->npc_num_area ];
		mapdata->npc[ mapdata->npc_num_area ] = mapdata->npc[ mapdata->npc_num ];
	}
	else {
		mapdata->npc_num_warp--;
		mapdata->npc_num_area--;
		mapdata->npc[i] = mapdata->npc[ mapdata->npc_num_warp ];
		mapdata->npc[ mapdata->npc_num_warp ] = mapdata->npc[ mapdata->npc_num_area ];
		mapdata->npc[ mapdata->npc_num_area ] = mapdata->npc[ mapdata->npc_num ];
	}
	mapdata->npc[ mapdata->npc_num ] = NULL;
	return 0;
}

/**
 * @see DBApply
 */
static int npc_unload_ev(DBKey key, DBData *data, va_list ap)
{
	struct event_data* ev = (struct event_data*)db_data2ptr(data);
	char* npcname = va_arg(ap, char *);

	if(strcmp(ev->nd->exname,npcname)==0){
		db_remove(ev_db, key);
		return 1;
	}
	return 0;
}

//Chk if npc matches src_id, then unload.
//Sub-function used to find duplicates.
static int npc_unload_dup_sub(struct npc_data* nd, va_list args)
{
	int src_id;

	src_id = va_arg(args, int);
	if (nd->src_id == src_id)
		npc_unload(nd, true);
	return 0;
}

//Removes all npcs that are duplicates of the passed one. [Skotlex]
void npc_unload_duplicates(struct npc_data* nd)
{
	map_foreachnpc(npc_unload_dup_sub,nd->bl.id);
}

//Removes an npc from map and db.
//Single is to free name (for duplicates).
int npc_unload(struct npc_data* nd, bool single) {
	nullpo_ret(nd);

	status_change_clear(&nd->bl, 1);
	npc_remove_map(nd);
	map_deliddb(&nd->bl);
	if( single )
		strdb_remove(npcname_db, nd->exname);

	if (nd->chat_id) // remove npc chatroom object and kick users
		chat_deletenpcchat(nd);

#ifdef PCRE_SUPPORT
	npc_chat_finalize(nd); // deallocate npc PCRE data structures
#endif

	if( single && nd->path ) {
		struct npc_path_data* npd = NULL;
		if( nd->path ) {
			npd = (struct npc_path_data*)strdb_get(npc_path_db, nd->path);
		}

		if( npd && --npd->references == 0 ) {
			strdb_remove(npc_path_db, nd->path);/* remove from db */
			aFree(nd->path);/* remove now that no other instances exist */

			if (npd == npc_last_npd) {
				npc_last_npd = NULL;
				npc_last_ref = NULL;
				npc_last_path = NULL;
			}
		}
	}
	
	if( single && nd->bl.m != -1 )
		map_remove_questinfo(nd->bl.m, nd);

	if( (nd->subtype == NPCTYPE_SHOP || nd->subtype == NPCTYPE_CASHSHOP || nd->subtype == NPCTYPE_ITEMSHOP || nd->subtype == NPCTYPE_POINTSHOP || nd->subtype == NPCTYPE_MARKETSHOP) && nd->src_id == 0) //src check for duplicate shops [Orcao]
		aFree(nd->u.shop.shop_item);
	else if( nd->subtype == NPCTYPE_SCRIPT ) {
		struct s_mapiterator* iter;
		struct block_list* bl;

		if( single )
			ev_db->foreach(ev_db,npc_unload_ev,nd->exname); //Clean up all events related

		iter = mapit_geteachpc();
		for( bl = (struct block_list*)mapit_first(iter); mapit_exists(iter); bl = (struct block_list*)mapit_next(iter) ) {
			struct map_session_data *sd = ((TBL_PC*)bl);
			if( sd && sd->npc_timer_id != INVALID_TIMER ) {
				const struct TimerData *td = get_timer(sd->npc_timer_id);

				if( td && td->id != nd->bl.id )
					continue;

				if( td && td->data )
					ers_free(timer_event_ers, (void*)td->data);
				delete_timer(sd->npc_timer_id, npc_timerevent);
				sd->npc_timer_id = INVALID_TIMER;
			}
		}
		mapit_free(iter);

		if (nd->u.scr.timerid != INVALID_TIMER) {
			const struct TimerData *td;
			td = get_timer(nd->u.scr.timerid);
			if (td && td->data)
				ers_free(timer_event_ers, (void*)td->data);
			delete_timer(nd->u.scr.timerid, npc_timerevent);
		}
		if (nd->u.scr.timer_event)
			aFree(nd->u.scr.timer_event);
		if (nd->src_id == 0) {
			if(nd->u.scr.script) {
				script_free_code(nd->u.scr.script);
				nd->u.scr.script = NULL;
			}
			if (nd->u.scr.label_list) {
				aFree(nd->u.scr.label_list);
				nd->u.scr.label_list = NULL;
				nd->u.scr.label_list_num = 0;
			}
		}
		if( nd->u.scr.guild_id )
			guild_flag_remove(nd);
		if( nd->sc_display_count ){
			unsigned char i;

			for( i = 0; i < nd->sc_display_count; i++ )
				ers_free(npc_sc_display_ers, nd->sc_display[i]);
			nd->sc_display_count = 0;
			aFree(nd->sc_display);
			nd->sc_display = NULL;
		}
	}

	nd->qi_data.clear();

	script_stop_sleeptimers(nd->bl.id);
	aFree(nd);

	return 0;
}

//
// NPC Source Files
//

/// Clears the npc source file list
static void npc_clearsrcfile(void)
{
	struct npc_src_list* file = npc_src_files;

	while( file != NULL ) {
		struct npc_src_list* file_tofree = file;
		file = file->next;
		aFree(file_tofree);
	}
	npc_src_files = NULL;
}

/**
 * Adds a npc source file (or removes all)
 * @param name : file to add
 * @param loadscript : flag to parse the script immediately after adding the src file
 * @return 0=error, 1=sucess
 */
int npc_addsrcfile(const char* name, bool loadscript)
{
	struct npc_src_list* file;
	struct npc_src_list* file_prev = NULL;

	if( strcmpi(name, "clear") == 0 )
	{
		npc_clearsrcfile();
		return 1;
	}

	//Check if this is not a file
    if(check_filepath(name)!=2){
		ShowError("npc_addsrcfile: Can't find source file \"%s\"\n", name );
		return 0;
	}
        
	// prevent multiple insert of source files
	file = npc_src_files;
	while( file != NULL )
	{
		if( strcmp(name, file->name) == 0 )
			return 0;// found the file, no need to insert it again
		file_prev = file;
		file = file->next;
	}

	file = (struct npc_src_list*)aMalloc(sizeof(struct npc_src_list) + strlen(name));
	file->next = NULL;
	safestrncpy(file->name, name, strlen(name) + 1);
	if( file_prev == NULL )
		npc_src_files = file;
	else
		file_prev->next = file;

	if (loadscript)
		return npc_parsesrcfile(file->name);

	return 1;
}

/// Removes a npc source file (or all)
void npc_delsrcfile(const char* name)
{
	struct npc_src_list* file = npc_src_files;
	struct npc_src_list* file_prev = NULL;

	if( strcmpi(name, "all") == 0 )
	{
		npc_clearsrcfile();
		return;
	}

	while( file != NULL )
	{
		if( strcmp(file->name, name) == 0 )
		{
			if( npc_src_files == file )
				npc_src_files = file->next;
			else
				file_prev->next = file->next;
			aFree(file);
			break;
		}
		file_prev = file;
		file = file->next;
	}
}

/// Parses and sets the name and exname of a npc.
/// Assumes that m, x and y are already set in nd.
static void npc_parsename(struct npc_data* nd, const char* name, const char* start, const char* buffer, const char* filepath)
{
	const char* p;
	struct npc_data* dnd;// duplicate npc
	char newname[NPC_NAME_LENGTH+1];

	// parse name
	p = strstr(name,"::");
	if( p ) { // <Display name>::<Unique name>
		size_t len = p-name;
		if( len > NPC_NAME_LENGTH ) {
			ShowWarning("npc_parsename: Display name of '%s' is too long (len=%u) in file '%s', line'%d'. Truncating to %u characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NPC_NAME_LENGTH);
			safestrncpy(nd->name, name, sizeof(nd->name));
		} else {
			memcpy(nd->name, name, len);
			memset(nd->name+len, 0, sizeof(nd->name)-len);
		}
		len = strlen(p+2);
		if( len > NPC_NAME_LENGTH )
			ShowWarning("npc_parsename: Unique name of '%s' is too long (len=%u) in file '%s', line'%d'. Truncating to %u characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NPC_NAME_LENGTH);
		safestrncpy(nd->exname, p+2, sizeof(nd->exname));
	} else {// <Display name>
		size_t len = strlen(name);
		if( len > NPC_NAME_LENGTH )
			ShowWarning("npc_parsename: Name '%s' is too long (len=%u) in file '%s', line'%d'. Truncating to %u characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NPC_NAME_LENGTH);
		safestrncpy(nd->name, name, sizeof(nd->name));
		safestrncpy(nd->exname, name, sizeof(nd->exname));
	}

	if( *nd->exname == '\0' || strstr(nd->exname,"::") != NULL ) {// invalid
		snprintf(newname, ARRAYLENGTH(newname), "0_%d_%d_%d", nd->bl.m, nd->bl.x, nd->bl.y);
		ShowWarning("npc_parsename: Invalid unique name in file '%s', line'%d'. Renaming '%s' to '%s'.\n", filepath, strline(buffer,start-buffer), nd->exname, newname);
		safestrncpy(nd->exname, newname, sizeof(nd->exname));
	}

	if( (dnd=npc_name2id(nd->exname)) != NULL ) {// duplicate unique name, generate new one
		char this_mapname[MAP_NAME_LENGTH_EXT];
		char other_mapname[MAP_NAME_LENGTH_EXT];
		int i = 0;

		do {
			++i;
			snprintf(newname, ARRAYLENGTH(newname), "%d_%d_%d_%d", i, nd->bl.m, nd->bl.x, nd->bl.y);
		} while( npc_name2id(newname) != NULL );

		strcpy(this_mapname, (nd->bl.m==-1?"(not on a map)":mapindex_id2name(map_getmapdata(nd->bl.m)->index)));
		strcpy(other_mapname, (dnd->bl.m==-1?"(not on a map)":mapindex_id2name(map_getmapdata(dnd->bl.m)->index)));

		ShowWarning("npc_parsename: Duplicate unique name in file '%s', line'%d'. Renaming '%s' to '%s'.\n", filepath, strline(buffer,start-buffer), nd->exname, newname);
		ShowDebug("this npc:\n   display name '%s'\n   unique name '%s'\n   map=%s, x=%d, y=%d\n", nd->name, nd->exname, this_mapname, nd->bl.x, nd->bl.y);
		ShowDebug("other npc in '%s' :\n   display name '%s'\n   unique name '%s'\n   map=%s, x=%d, y=%d\n",dnd->path, dnd->name, dnd->exname, other_mapname, dnd->bl.x, dnd->bl.y);
		safestrncpy(nd->exname, newname, sizeof(nd->exname));
	}

	if( npc_last_path != filepath ) {
		struct npc_path_data * npd = NULL;

		if( !(npd = (struct npc_path_data *)strdb_get(npc_path_db,filepath) ) ) {
			CREATE(npd, struct npc_path_data, 1);
			strdb_put(npc_path_db, filepath, npd);

			CREATE(npd->path, char, strlen(filepath)+1);
			safestrncpy(npd->path, filepath, strlen(filepath)+1);

			npd->references = 0;
		}

		nd->path = npd->path;
		npd->references++;

		npc_last_npd = npd;
		npc_last_ref = npd->path;
		npc_last_path = (char*) filepath;
	} else {
		nd->path = npc_last_ref;
		if( npc_last_npd )
			npc_last_npd->references++;
	}
}

/**
 * Parses NPC view.
 * Support for using Constants in place of NPC View IDs.
 */
int npc_parseview(const char* w4, const char* start, const char* buffer, const char* filepath) {
	int i = 0;
	char viewid[1024];	// Max size of name from const.yml, see ConstantDatabase::parseBodyNode.

	// Extract view ID / constant
	while (w4[i] != '\0') {
		if (ISSPACE(w4[i]) || w4[i] == '/' || w4[i] == ',')
			break;

		i++;
	}

	safestrncpy(viewid, w4, i+=1);

	char *pid;
	int val = strtol(viewid, &pid, 0);

	// Check if view id is not an ID (only numbers).
	if (pid != nullptr && *pid != '\0') {
		int64 val_tmp;

		// Check if constant exists and get its value.
		if(!script_get_constant(viewid, &val_tmp)) {
			std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(viewid);
			if (mob != nullptr)
				val = static_cast<int>(mob->id);
			else {
				ShowWarning("npc_parseview: Invalid NPC constant '%s' specified in file '%s', line'%d'. Defaulting to INVISIBLE. \n", viewid, filepath, strline(buffer,start-buffer));
				val = JT_INVISIBLE;
			}
		} else
			val = static_cast<int>(val_tmp);
	}

	return val;
}

/**
 * Create a bare NPC object.
 * @param m: Map ID
 * @param x: X location
 * @param y: Y location
 * @return npc_data
 */
struct npc_data *npc_create_npc(int16 m, int16 x, int16 y){
	struct npc_data *nd = nullptr;

	CREATE(nd, struct npc_data, 1);
	nd->bl.id = npc_get_new_npc_id();
	nd->bl.prev = nd->bl.next = nullptr;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	nd->sc_display = nullptr;
	nd->sc_display_count = 0;
	nd->progressbar.timeout = 0;
	nd->vd = npc_viewdb[0]; // Default to JT_INVISIBLE

	return nd;
}

/**
 * Add then display an npc warp on map
 * @param name : warp unique name
 * @param from_mapid : mapid to warp from
 * @param from_x : x coordinate of warp
 * @param from_y : y coordinate of warp
 * @param xs : x lenght of warp (for trigger activation)
 * @param ys : y lenght of warp (for trigger activation)
 * @param to_mapindex : mapid to warp to
 * @param to_x : x coordinate to warp to
 * @param to_y : y coordinate to warp to
 * @return NULL:failed creation, npc_data* new warp
 */
struct npc_data* npc_add_warp(char* name, short from_mapid, short from_x, short from_y, short xs, short ys, unsigned short to_mapindex, short to_x, short to_y)
{
	int i, flag = 0;
	struct npc_data *nd;

	nd = npc_create_npc(from_mapid, from_x, from_y);

	safestrncpy(nd->exname, name, ARRAYLENGTH(nd->exname));
	if (npc_name2id(nd->exname) != NULL)
		flag = 1;

	if (flag == 1)
		snprintf(nd->exname, ARRAYLENGTH(nd->exname), "warp_%d_%d_%d", from_mapid, from_x, from_y);

	for( i = 0; npc_name2id(nd->exname) != NULL; ++i )
		snprintf(nd->exname, ARRAYLENGTH(nd->exname), "warp%d_%d_%d_%d", i, from_mapid, from_x, from_y);
	safestrncpy(nd->name, nd->exname, ARRAYLENGTH(nd->name));

	if( battle_config.warp_point_debug )
		nd->class_ = JT_GUILD_FLAG;
	else
		nd->class_ = JT_WARPNPC;
	nd->speed = 200;

	nd->u.warp.mapindex = to_mapindex;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = ys;
	nd->bl.type = BL_NPC;
	nd->subtype = NPCTYPE_WARP;
	nd->trigger_on_hidden = false;
	map_addnpc(from_mapid, nd);
	npc_setcells(nd);
	if(map_addblock(&nd->bl))
		return NULL;
	status_set_viewdata(&nd->bl, nd->class_);
	status_change_init(&nd->bl);
	unit_dataset(&nd->bl);
	if( map_getmapdata(nd->bl.m)->users )
		clif_spawn(&nd->bl);
	strdb_put(npcname_db, nd->exname, nd);

	return nd;
}

/**
 * Parses a warp npc.
 * Line definition <from mapname>,<fromX>,<fromY>,<facing>%TAB%warp%TAB%<warp name>%TAB%<spanx>,<spany>,<to mapname>,<toX>,<toY>
 * @param w1 : word 1 before tab (<from map name>,<fromX>,<fromY>,<facing>)
 * @param w2 : word 2 before tab (warp), keyword that sent us in this parsing
 * @param w3 : word 3 before tab (<warp name>)
 * @param w4 : word 4 before tab (<spanx>,<spany>,<to mapname>,<toX>,<toY>)
 * @param start : index to start parsing
 * @param buffer : lines to parses
 * @param filepath : filename with path wich we are parsing
 * @return new index for next parsing
 */
static const char* npc_parse_warp(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	int m;
	short x, y, xs, ys, to_x, to_y;
	unsigned short i;
	char mapname[MAP_NAME_LENGTH_EXT], to_mapname[MAP_NAME_LENGTH_EXT];
	struct npc_data *nd;

	// w1=<from map name>,<fromX>,<fromY>,<facing>
	// w4=<spanx>,<spany>,<to map name>,<toX>,<toY>
	if( sscanf(w1, "%15[^,],%6hd,%6hd", mapname, &x, &y) != 3
	||	sscanf(w4, "%6hd,%6hd,%15[^,],%6hd,%6hd", &xs, &ys, to_mapname, &to_x, &to_y) != 5 )
	{
		ShowError("npc_parse_warp: Invalid warp definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}

	m = map_mapname2mapid(mapname);
	i = mapindex_name2id(to_mapname);
	if( i == 0 )
	{
		ShowError("npc_parse_warp: Unknown destination map in file '%s', line '%d' : %s\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), to_mapname, w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}

	struct map_data *mapdata = map_getmapdata(m);

	if( m != -1 && ( x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys ) ) {
		ShowWarning("npc_parse_warp: coordinates %d/%d are out of bounds in map %s(%dx%d), in file '%s', line '%d'\n", x, y, mapdata->name, mapdata->xs, mapdata->ys,filepath,strline(buffer,start-buffer));
	}

	nd = npc_create_npc(m, x, y);
	npc_parsename(nd, w3, start, buffer, filepath);

	if (!battle_config.warp_point_debug)
		nd->class_ = JT_WARPNPC;
	else
		nd->class_ = JT_GUILD_FLAG;
	nd->speed = 200;

	nd->u.warp.mapindex = i;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = ys;
	npc_warp++;
	nd->bl.type = BL_NPC;
	nd->subtype = NPCTYPE_WARP;
	if (strcasecmp("warp2", w2) == 0)
		nd->trigger_on_hidden = true;
	else
		nd->trigger_on_hidden = false;
	map_addnpc(m, nd);
	npc_setcells(nd);
	if(map_addblock(&nd->bl)) //couldn't add on map
		return strchr(start,'\n');
	status_set_viewdata(&nd->bl, nd->class_);
	status_change_init(&nd->bl);
	unit_dataset(&nd->bl);
	if( map_getmapdata(nd->bl.m)->users )
		clif_spawn(&nd->bl);
	strdb_put(npcname_db, nd->exname, nd);

	return strchr(start,'\n');// continue
}

/**
 * Parses a shop/cashshop npc.
 * Line definition :
 * <map name>,<x>,<y>,<facing>%TAB%shop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%cashshop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%itemshop%TAB%<NPC Name>%TAB%<sprite id>,<costitemid>{:<discount>},<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%pointshop%TAB%<NPC Name>%TAB%<sprite id>,<costvariable>{:<discount>},<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%marketshop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>:<quantity>{,<itemid>:<price>:<quantity>...}
 * @param w1 : word 1 before tab (<from map name>,<x>,<y>,<facing>)
 * @param w2 : word 2 before tab (shop|cashshop|itemshop|pointshop|marketshop), keyword that sent us in this parsing
 * @param w3 : word 3 before tab (<NPC Name>)
 * @param w4 : word 4 before tab (<sprited id>,<shop definition...>)
 * @param start : index to start parsing
 * @param buffer : lines to parses
 * @param filepath : filename with path wich we are parsing
 * @return new index for next parsing
 */
static const char* npc_parse_shop(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	char *p, point_str[32];
	int m, is_discount = 0;
	uint16 dir;
	short x, y;
	t_itemid nameid = 0;
	struct npc_data *nd;
	enum npc_subtype type;

	if( strcmp(w1,"-") == 0 )
	{// 'floating' shop?
		x = y = dir = 0;
		m = -1;
	}
	else
	{// w1=<map name>,<x>,<y>,<facing>
		char mapname[MAP_NAME_LENGTH_EXT];
		if( sscanf(w1, "%15[^,],%6hd,%6hd,%4hd", mapname, &x, &y, &dir) != 4
		||	strchr(w4, ',') == NULL )
		{
			ShowError("npc_parse_shop: Invalid shop definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			return strchr(start,'\n');// skip and continue
		}

		m = map_mapname2mapid(mapname);
	}

	struct map_data *mapdata = map_getmapdata(m);

	if( m != -1 && ( x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys ) ) {
		ShowWarning("npc_parse_shop: coordinates %d/%d are out of bounds in map %s(%dx%d), in file '%s', line '%d'\n", x, y, mapdata->name, mapdata->xs, mapdata->ys,filepath,strline(buffer,start-buffer));
	}

	if( !strcasecmp(w2,"cashshop") )
		type = NPCTYPE_CASHSHOP;
	else if( !strcasecmp(w2,"itemshop") )
		type = NPCTYPE_ITEMSHOP;
	else if( !strcasecmp(w2,"pointshop") )
		type = NPCTYPE_POINTSHOP;
	else if( !strcasecmp(w2, "marketshop") )
		type = NPCTYPE_MARKETSHOP;
	else
		type = NPCTYPE_SHOP;

	p = strchr(w4,',');
	memset(point_str,'\0',sizeof(point_str));

	switch(type) {
		case NPCTYPE_ITEMSHOP: {
			if (sscanf(p,",%u:%11d,",&nameid,&is_discount) < 1) {
				ShowError("npc_parse_shop: Invalid item cost definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
				return strchr(start,'\n'); // skip and continue
			}
			if (itemdb_exists(nameid) == NULL) {
				ShowWarning("npc_parse_shop: Invalid item ID cost in file '%s', line '%d' (id '%u').\n", filepath, strline(buffer,start-buffer), nameid);
				return strchr(start,'\n'); // skip and continue
			}
			p = strchr(p+1,',');
			break;
		}
		case NPCTYPE_POINTSHOP: {
			if (sscanf(p, ",%32[^,:]:%11d,",point_str,&is_discount) < 1) {
				ShowError("npc_parse_shop: Invalid item cost definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
				return strchr(start,'\n'); // skip and continue
			}
			switch(point_str[0]) {
				case '$':
				case '.':
				case '\'':
					ShowWarning("npc_parse_shop: Invalid item cost variable type (must be permanent character or account based) in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
					return strchr(start,'\n'); // skip and continue
					break;
			}
			if (point_str[strlen(point_str) - 1] == '$') {
				ShowWarning("npc_parse_shop: Invalid item cost variable type (must be integer) in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
				return strchr(start,'\n'); // skip and continue
			}
			p = strchr(p+1,',');
			break;
		}
		case NPCTYPE_MARKETSHOP:
#if PACKETVER < 20131223
			ShowError("npc_parse_shop: (MARKETSHOP) Feature is disabled, need client 20131223 or newer. Ignoring file '%s', line '%d\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
			return strchr(start, '\n'); // skip and continue
#else
			is_discount = 0;
			break;
#endif
		default:
			if( sscanf( p, ",%32[^,:]:%11d,", point_str, &is_discount ) == 2 ){
				is_discount = 1;
			}else{
				if( !strcasecmp( point_str, "yes" ) ){
					is_discount = 1;
				}else if( !strcasecmp( point_str, "no" ) ){
					is_discount = 0;
				}else{
					ShowError( "npc_parse_shop: unknown discount setting %s\n", point_str );
					return strchr( start, '\n' ); // skip and continue
				}

				p = strchr( p + 1, ',' );
			}
			break;
	}
	
	nd = npc_create_npc(m, x, y);
	nd->u.shop.count = 0;
	while ( p ) {
		t_itemid nameid2;
		int32 qty = -1;
		int value;
		bool skip = false;

		if( p == NULL )
			break;
		switch(type) {
			case NPCTYPE_MARKETSHOP:
#if PACKETVER >= 20131223
				if (sscanf(p, ",%u:%11d:%11d", &nameid2, &value, &qty) != 3) {
					ShowError("npc_parse_shop: (MARKETSHOP) Invalid item definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
					skip = true;
				}
#endif
				break;
			default:
				if (sscanf(p, ",%u:%11d", &nameid2, &value) != 2) {
					ShowError("npc_parse_shop: Invalid item definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
					skip = true;
				}
				break;
		}

		if (skip)
			break;

		std::shared_ptr<item_data> id = item_db.find(nameid2);

		if( id == nullptr ) {
			ShowWarning("npc_parse_shop: Invalid sell item in file '%s', line '%d' (id '%u').\n", filepath, strline(buffer,start-buffer), nameid2);
			p = strchr(p+1,',');
			continue;
		}
		if( value < 0 ) {
			if (type == NPCTYPE_SHOP || type == NPCTYPE_MARKETSHOP) value = id->value_buy;
			else value = 0; // Cashshop doesn't have a "buy price" in the item_db
		}
		if (value == 0 && (type == NPCTYPE_SHOP || type == NPCTYPE_MARKETSHOP)) { // NPC selling items for free!
			ShowWarning("npc_parse_shop: Item %s [%u] is being sold for FREE in file '%s', line '%d'.\n",
				id->name.c_str(), nameid2, filepath, strline(buffer,start-buffer));
		}
		if( type == NPCTYPE_SHOP && value*0.75 < id->value_sell*1.24 ) { // Exploit possible: you can buy and sell back with profit
			ShowWarning("npc_parse_shop: Item %s [%u] discounted buying price (%d->%d) is less than overcharged selling price (%d->%d) at file '%s', line '%d'.\n",
				id->name.c_str(), nameid2, value, (int)(value*0.75), id->value_sell, (int)(id->value_sell*1.24), filepath, strline(buffer,start-buffer));
		}
		if (type == NPCTYPE_MARKETSHOP && qty < -1) {
			ShowWarning("npc_parse_shop: Item %s [%u] is stocked with invalid value %hd, changed to unlimited (-1). File '%s', line '%d'.\n",
				id->name.c_str(), nameid2, qty, filepath, strline(buffer,start-buffer));
			qty = -1;
		}
		//for logs filters, atcommands and iteminfo script command
		if( id->maxchance == 0 )
			id->maxchance = -1; // -1 would show that the item's sold in NPC Shop
		
#if PACKETVER >= 20131223
		if (nd->u.shop.count && type == NPCTYPE_MARKETSHOP) {
			uint16 i;
			// Duplicate entry? Replace the value
			ARR_FIND(0, nd->u.shop.count, i, nd->u.shop.shop_item[i].nameid == nameid);
			if (i != nd->u.shop.count) {
				nd->u.shop.shop_item[i].qty = qty;
				nd->u.shop.shop_item[i].value = value;
				p = strchr(p+1,',');
				continue;
			}
		}
#endif

		RECREATE(nd->u.shop.shop_item, struct npc_item_list,nd->u.shop.count+1);

		nd->u.shop.shop_item[nd->u.shop.count].nameid = nameid2;
		nd->u.shop.shop_item[nd->u.shop.count].value = value;
#if PACKETVER >= 20131223
		nd->u.shop.shop_item[nd->u.shop.count].flag = 0;
		if (type == NPCTYPE_MARKETSHOP )
			nd->u.shop.shop_item[nd->u.shop.count].qty = qty;
#endif
		nd->u.shop.count++;
		p = strchr(p+1,',');
	}
	if( nd->u.shop.count == 0 ) {
		ShowWarning("npc_parse_shop: Ignoring empty shop in file '%s', line '%d'.\n", filepath, strline(buffer,start-buffer));
		aFree(nd);
		return strchr(start,'\n');// continue
	}

	if( type == NPCTYPE_ITEMSHOP ){
		// Item shop currency
		nd->u.shop.itemshop_nameid = nameid;
	}else if( type == NPCTYPE_POINTSHOP ){
		// Point shop currency
		safestrncpy( nd->u.shop.pointshop_str, point_str, strlen( point_str ) + 1 );
	}

	nd->u.shop.discount = is_discount > 0;

	npc_parsename(nd, w3, start, buffer, filepath);
	nd->class_ = m == -1 ? JT_FAKENPC : npc_parseview(w4, start, buffer, filepath);
	nd->speed = 200;

	++npc_shop;
	nd->bl.type = BL_NPC;
	nd->subtype = type;
#if PACKETVER >= 20131223
	// Insert market data to table
	if (nd->subtype == NPCTYPE_MARKETSHOP) {
		uint16 i;
		for (i = 0; i < nd->u.shop.count; i++)
			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[i]);
	}
#endif
	if( m >= 0 )
	{// normal shop npc
		map_addnpc(m,nd);
		if(map_addblock(&nd->bl))
			return strchr(start,'\n');
		status_change_init(&nd->bl);
		unit_dataset(&nd->bl);
		nd->ud.dir = (uint8)dir;
		if( nd->class_ != JT_FAKENPC ){
			status_set_viewdata(&nd->bl, nd->class_);
			if( map_getmapdata(nd->bl.m)->users )
				clif_spawn(&nd->bl);
		}
	} else
	{// 'floating' shop?
		map_addiddb(&nd->bl);
	}
	strdb_put(npcname_db, nd->exname, nd);
	return strchr(start,'\n');// continue
}

/** [Cydh]
* Check if the shop is affected by discount or not
* @param type Type of NPC shop (enum npc_subtype)
* @param discount Discount flag of NPC shop
* @return bool 'true' is discountable, 'false' otherwise
*/
bool npc_shop_discount( struct npc_data* nd ){
	switch( nd->subtype ){
		case NPCTYPE_ITEMSHOP:
			return nd->u.shop.discount || ( battle_config.discount_item_point_shop&1 );
		case NPCTYPE_POINTSHOP:
			return nd->u.shop.discount || ( battle_config.discount_item_point_shop&2 );
		default:
			return nd->u.shop.discount;
	}
}

/**
 * NPC other label
 * Not sure, seem to add label in a chainlink
 * @see DBApply
 */
int npc_convertlabel_db(DBKey key, DBData *data, va_list ap)
{
	const char* lname = (const char*)key.str;
	int lpos = db_data2i(data);
	struct npc_label_list** label_list;
	int* label_list_num;
	const char* filepath;
	struct npc_label_list* label;
	const char *p;
	int len;

	nullpo_ret(label_list = va_arg(ap,struct npc_label_list**));
	nullpo_ret(label_list_num = va_arg(ap,int*));
	nullpo_ret(filepath = va_arg(ap,const char*));

	// In case of labels not terminated with ':', for user defined function support
	p = lname;
	while( ISALNUM(*p) || *p == '_' )
		++p;
	len = p-lname;

	// here we check if the label fit into the buffer
	if( len > NAME_LENGTH )
	{
		ShowError("npc_parse_script: label name longer than %d chars! '%s'\n (%s)", NAME_LENGTH, lname, filepath);
		return 0;
	}

	if( *label_list == NULL )
	{
		*label_list = (struct npc_label_list *) aCalloc (1, sizeof(struct npc_label_list));
		*label_list_num = 0;
	} else
		*label_list = (struct npc_label_list *) aRealloc (*label_list, sizeof(struct npc_label_list)*(*label_list_num+1));
	label = *label_list+*label_list_num;

	safestrncpy(label->name, lname, sizeof(label->name));
	label->pos = lpos;
	++(*label_list_num);

	return 0;
}

// Skip the contents of a script.
static const char* npc_skip_script(const char* start, const char* buffer, const char* filepath)
{
	const char* p;
	int curly_count;

	if( start == NULL )
		return NULL;// nothing to skip

	// initial bracket (assumes the previous part is ok)
	p = strchr(start,'{');
	if( p == NULL )
	{
		ShowError("npc_skip_script: Missing left curly in file '%s', line'%d'.", filepath, strline(buffer,start-buffer));
		return NULL;// can't continue
	}

	// skip everything
	for( curly_count = 1; curly_count > 0 ; )
	{
		p = skip_space(p+1) ;
		if( *p == '}' )
		{// right curly
			--curly_count;
		}
		else if( *p == '{' )
		{// left curly
			++curly_count;
		}
		else if( *p == '"' )
		{// string
			for( ++p; *p != '"' ; ++p )
			{
				if( *p == '\\' && (unsigned char)p[-1] <= 0x7e )
					++p;// escape sequence (not part of a multibyte character)
				else if( *p == '\0' )
				{
					script_error(buffer, filepath, 0, "Unexpected end of string.", p);
					return NULL;// can't continue
				}
				else if( *p == '\n' )
				{
					script_error(buffer, filepath, 0, "Unexpected newline at string.", p);
					return NULL;// can't continue
				}
			}
		}
		else if( *p == '\0' )
		{// end of buffer
			ShowError("Missing %d right curlys at file '%s', line '%d'.\n", curly_count, filepath, strline(buffer,p-buffer));
			return NULL;// can't continue
		}
	}

	return p+1;// return after the last '}'
}

/**
 * Parses a npc script.
 * Line definition :
 * <map name>,<x>,<y>,<facing>%TAB%script%TAB%<NPC Name>%TAB%<sprite id>,{<code>}
 * <map name>,<x>,<y>,<facing>%TAB%script%TAB%<NPC Name>%TAB%<sprite id>,<triggerX>,<triggerY>,{<code>} * @TODO missing cashshop line definition
 * @param w1 : word 1 before tab (<from map name>,<x>,<y>,<facing>)
 * @param w2 : word 2 before tab (script), keyword that sent us in this parsing
 * @param w3 : word 3 before tab (<NPC Name>)
 * @param w4 : word 4 before tab (<sprited id>,<code>)
 * @param start : index to start parsing
 * @param buffer : lines to parses
 * @param filepath : filename with path wich we are parsing
 * @return new index for next parsing
 */
static const char* npc_parse_script(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath) {
	int16 dir = 0;
	short m, x, y, xs = 0, ys = 0; // [Valaris] thanks to fov
	struct script_code *script;
	int i;
	const char* end;
	const char* script_start;

	struct npc_label_list* label_list;
	int label_list_num;
	struct npc_data* nd;

	if( strcmp(w1, "-") == 0 )
	{// floating npc
		x = 0;
		y = 0;
		m = -1;
	}
	else
	{// npc in a map
		char mapname[MAP_NAME_LENGTH_EXT];

		if( sscanf(w1, "%15[^,],%6hd,%6hd,%4hd", mapname, &x, &y, &dir) != 4 )
		{
			ShowError("npc_parse_script: Invalid placement format for a script in file '%s', line '%d'. Skipping the rest of file...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			return NULL;// unknown format, don't continue
		}
		m = map_mapname2mapid(mapname);
	}

	script_start = strstr(start,",{");
	end = strchr(start,'\n');
	if( strstr(w4,",{") == NULL || script_start == NULL || (end != NULL && script_start > end) )
	{
		ShowError("npc_parse_script: Missing left curly ',{' in file '%s', line '%d'. Skipping the rest of the file.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return NULL;// can't continue
	}
	++script_start;

	end = npc_skip_script(script_start, buffer, filepath);
	if( end == NULL )
		return NULL;// (simple) parse error, don't continue

	script = parse_script(script_start, filepath, strline(buffer,script_start-buffer), SCRIPT_USE_LABEL_DB);
	label_list = NULL;
	label_list_num = 0;
	if( script )
	{
		DBMap* label_db = script_get_label_db();
		label_db->foreach(label_db, npc_convertlabel_db, &label_list, &label_list_num, filepath);
		db_clear(label_db); // not needed anymore, so clear the db
	}

	nd = npc_create_npc(m, x, y);

	if( sscanf(w4, "%*[^,],%6hd,%6hd", &xs, &ys) == 2 )
	{// OnTouch area defined
		nd->u.scr.xs = xs;
		nd->u.scr.ys = ys;
	}
	else
	{// no OnTouch area
		nd->u.scr.xs = -1;
		nd->u.scr.ys = -1;
	}

	npc_parsename(nd, w3, start, buffer, filepath);
	nd->class_ = m == -1 ? JT_FAKENPC : npc_parseview(w4, start, buffer, filepath);
	nd->speed = 200;
	nd->u.scr.script = script;
	nd->u.scr.label_list = label_list;
	nd->u.scr.label_list_num = label_list_num;

	++npc_script;
	nd->bl.type = BL_NPC;
	nd->subtype = NPCTYPE_SCRIPT;

	if( m >= 0 )
	{
		map_addnpc(m, nd);
		status_change_init(&nd->bl);
		unit_dataset(&nd->bl);
		nd->ud.dir = (uint8)dir;
		npc_setcells(nd);
		if(map_addblock(&nd->bl))
			return NULL;
		if( nd->class_ != JT_FAKENPC )
		{
			status_set_viewdata(&nd->bl, nd->class_);
			if( map_getmapdata(nd->bl.m)->users )
				clif_spawn(&nd->bl);
		}
	}
	else
	{
		// we skip map_addnpc, but still add it to the list of ID's
		map_addiddb(&nd->bl);
	}
	strdb_put(npcname_db, nd->exname, nd);

	//-----------------------------------------
	// Loop through labels to export them as necessary
	for (i = 0; i < nd->u.scr.label_list_num; i++) {
		if (npc_event_export(nd, i)) {
			ShowWarning("npc_parse_script : duplicate event %s::%s (%s)\n",
			             nd->exname, nd->u.scr.label_list[i].name, filepath);
		}
		npc_timerevent_export(nd, i);
	}

	nd->u.scr.timerid = INVALID_TIMER;

	return end;
}

/// Duplicate a warp, shop, cashshop or script. [Orcao]
/// warp: <map name>,<x>,<y>,<facing>%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<spanx>,<spany>
/// shop/cashshop/npc: -%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>
/// shop/cashshop/npc: <map name>,<x>,<y>,<facing>%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>
/// npc: -%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>,<triggerX>,<triggerY>
/// npc: <map name>,<x>,<y>,<facing>%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>,<triggerX>,<triggerY>
const char* npc_parse_duplicate(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	short x, y, m, xs = -1, ys = -1;
	int16 dir;
	char srcname[128];
	int i;
	const char* end;
	size_t length;

	int src_id;
	int type;
	struct npc_data* nd;
	struct npc_data* dnd;

	end = strchr(start,'\n');
	length = strlen(w2);

	// get the npc being duplicated
	if( w2[length-1] != ')' || length <= 11 || length-11 >= sizeof(srcname) )
	{// does not match 'duplicate(%127s)', name is empty or too long
		ShowError("npc_parse_script: bad duplicate name in file '%s', line '%d' : %s\n", filepath, strline(buffer,start-buffer), w2);
		return end;// next line, try to continue
	}
	safestrncpy(srcname, w2+10, length-10);

	dnd = npc_name2id(srcname);
	if( dnd == NULL) {
		ShowError("npc_parse_script: original npc not found for duplicate in file '%s', line '%d' : %s\n", filepath, strline(buffer,start-buffer), srcname);
		return end;// next line, try to continue
	}
	src_id = dnd->src_id ? dnd->src_id : dnd->bl.id;
	type = dnd->subtype;

	// get placement
	if ((type == NPCTYPE_SHOP || type == NPCTYPE_CASHSHOP || type == NPCTYPE_ITEMSHOP || type == NPCTYPE_POINTSHOP || type == NPCTYPE_SCRIPT || type == NPCTYPE_MARKETSHOP) && strcmp(w1, "-") == 0) {// floating shop/chashshop/itemshop/pointshop/script
		x = y = dir = 0;
		m = -1;
	} else {
		char mapname[MAP_NAME_LENGTH_EXT];

		if( sscanf(w1, "%15[^,],%6hd,%6hd,%4hd", mapname, &x, &y, &dir) != 4 ) { // <map name>,<x>,<y>,<facing>
			ShowError("npc_parse_duplicate: Invalid placement format for duplicate in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			return end;// next line, try to continue
		}
		m = map_mapname2mapid(mapname);
	}

	struct map_data *mapdata = map_getmapdata(m);

	if( m != -1 && ( x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys ) ) {
		ShowError("npc_parse_duplicate: coordinates %d/%d are out of bounds in map %s(%dx%d), in file '%s', line '%d'\n", x, y, mapdata->name, mapdata->xs, mapdata->ys,filepath,strline(buffer,start-buffer));
	}

	if( type == NPCTYPE_WARP && sscanf(w4, "%6hd,%6hd", &xs, &ys) == 2 );// <spanx>,<spany>
	else if( type == NPCTYPE_SCRIPT && sscanf(w4, "%*[^,],%6hd,%6hd", &xs, &ys) == 2);// <sprite id>,<triggerX>,<triggerY>
	else if( type == NPCTYPE_WARP ) {
		ShowError("npc_parse_duplicate: Invalid span format for duplicate warp in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return end;// next line, try to continue
	}

	nd = npc_create_npc(m, x, y);
	npc_parsename(nd, w3, start, buffer, filepath);
	nd->class_ = m == -1 ? JT_FAKENPC : npc_parseview(w4, start, buffer, filepath);
	nd->speed = 200;
	nd->src_id = src_id;
	nd->bl.type = BL_NPC;
	nd->subtype = (enum npc_subtype)type;
	switch( type ) {
		case NPCTYPE_SCRIPT:
			++npc_script;
			nd->u.scr.xs = xs;
			nd->u.scr.ys = ys;
			nd->u.scr.script = dnd->u.scr.script;
			nd->u.scr.label_list = dnd->u.scr.label_list;
			nd->u.scr.label_list_num = dnd->u.scr.label_list_num;
			break;

		case NPCTYPE_SHOP:
		case NPCTYPE_CASHSHOP:
		case NPCTYPE_ITEMSHOP:
		case NPCTYPE_POINTSHOP:
		case NPCTYPE_MARKETSHOP:
			++npc_shop;
			safestrncpy( nd->u.shop.pointshop_str, dnd->u.shop.pointshop_str, strlen( dnd->u.shop.pointshop_str ) );
			nd->u.shop.itemshop_nameid = dnd->u.shop.itemshop_nameid;
			nd->u.shop.shop_item = dnd->u.shop.shop_item;
			nd->u.shop.count = dnd->u.shop.count;
			nd->u.shop.discount =  dnd->u.shop.discount;
			break;

		case NPCTYPE_WARP:
			++npc_warp;
			if( !battle_config.warp_point_debug )
				nd->class_ = JT_WARPNPC;
			else
				nd->class_ = JT_GUILD_FLAG;
			nd->u.warp.xs = xs;
			nd->u.warp.ys = ys;
			nd->u.warp.mapindex = dnd->u.warp.mapindex;
			nd->u.warp.x = dnd->u.warp.x;
			nd->u.warp.y = dnd->u.warp.y;
			nd->trigger_on_hidden = dnd->trigger_on_hidden;
			break;
	}

	//Add the npc to its location
	if( m >= 0 ) {
		map_addnpc(m, nd);
		status_change_init(&nd->bl);
		unit_dataset(&nd->bl);
		nd->ud.dir = (uint8)dir;
		npc_setcells(nd);
		if(map_addblock(&nd->bl))
			return end;
		if( nd->class_ != JT_FAKENPC ) {
			status_set_viewdata(&nd->bl, nd->class_);
			if( map_getmapdata(nd->bl.m)->users )
				clif_spawn(&nd->bl);
		}
	} else {
		// we skip map_addnpc, but still add it to the list of ID's
		map_addiddb(&nd->bl);
	}
	strdb_put(npcname_db, nd->exname, nd);

	if( type != NPCTYPE_SCRIPT )
		return end;

	//-----------------------------------------
	// Loop through labels to export them as necessary
	for (i = 0; i < nd->u.scr.label_list_num; i++) {
		if (npc_event_export(nd, i)) {
			ShowWarning("npc_parse_duplicate : duplicate event %s::%s (%s)\n",
			             nd->exname, nd->u.scr.label_list[i].name, filepath);
		}
		npc_timerevent_export(nd, i);
	}

	if(!strcmp(filepath,"INSTANCING")) //Instance NPCs will use this for commands
		nd->instance_id = mapdata->instance_id;

	nd->u.scr.timerid = INVALID_TIMER;

	return end;
}

int npc_duplicate4instance(struct npc_data *snd, int16 m) {
	char newname[NPC_NAME_LENGTH+1];
	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata->instance_id <= 0 )
		return 1;

	snprintf(newname, ARRAYLENGTH(newname), "dup_%d_%d", mapdata->instance_id, snd->bl.id);
	if( npc_name2id(newname) != NULL ) { // Name already in use
		ShowError("npc_duplicate4instance: the npcname (%s) is already in use while trying to duplicate npc %s in instance %d.\n", newname, snd->exname, mapdata->instance_id);
		return 1;
	}

	if( snd->subtype == NPCTYPE_WARP ) { // Adjust destination, if instanced
		struct npc_data *wnd = NULL; // New NPC
		std::shared_ptr<s_instance_data> idata = util::umap_find(instances, mapdata->instance_id);
		int dm = map_mapindex2mapid(snd->u.warp.mapindex), imap = 0;

		if( dm < 0 ) return 1;

		for (const auto &it : idata->map) {
			if (it.m && map_mapname2mapid(map_getmapdata(it.src_m)->name) == dm) {
				imap = map_mapname2mapid(map_getmapdata(it.m)->name);
				break; // Instance map matches destination, update to instance map
			}
		}

		if(!imap)
			imap = map_mapname2mapid(map_getmapdata(dm)->name);

		if( imap == -1 ) {
			ShowError("npc_duplicate4instance: warp (%s) leading to instanced map (%s), but instance map is not attached to current instance.\n", map_mapid2mapname(dm), snd->exname);
			return 1;
		}

		wnd = npc_create_npc(m, snd->bl.x, snd->bl.y);
		safestrncpy(wnd->name, "", ARRAYLENGTH(wnd->name));
		safestrncpy(wnd->exname, newname, ARRAYLENGTH(wnd->exname));
		wnd->class_ = JT_WARPNPC;
		wnd->speed = 200;
		wnd->u.warp.mapindex = map_id2index(imap);
		wnd->u.warp.x = snd->u.warp.x;
		wnd->u.warp.y = snd->u.warp.y;
		wnd->u.warp.xs = snd->u.warp.xs;
		wnd->u.warp.ys = snd->u.warp.ys;
		wnd->bl.type = BL_NPC;
		wnd->subtype = NPCTYPE_WARP;
		wnd->trigger_on_hidden = snd->trigger_on_hidden;
		wnd->src_id = snd->src_id ? snd->src_id : snd->bl.id;
		map_addnpc(m, wnd);
		npc_setcells(wnd);
		if(map_addblock(&wnd->bl))
			return 1;
		status_set_viewdata(&wnd->bl, wnd->class_);
		status_change_init(&wnd->bl);
		unit_dataset(&wnd->bl);
		if( map_getmapdata(wnd->bl.m)->users )
			clif_spawn(&wnd->bl);
		strdb_put(npcname_db, wnd->exname, wnd);
	} else {
		static char w1[128], w2[128], w3[128], w4[128];
		const char* stat_buf = "- call from instancing subsystem -\n";

		snprintf(w1, sizeof(w1), "%s,%d,%d,%d", mapdata->name, snd->bl.x, snd->bl.y, snd->ud.dir);
		snprintf(w2, sizeof(w2), "duplicate(%s)", snd->exname);
		snprintf(w3, sizeof(w3), "%s::%s", snd->name, newname);

		if( snd->u.scr.xs >= 0 && snd->u.scr.ys >= 0 )
			snprintf(w4, sizeof(w4), "%d,%d,%d", snd->class_, snd->u.scr.xs, snd->u.scr.ys); // Touch Area
		else
			snprintf(w4, sizeof(w4), "%d", snd->class_);

		npc_parse_duplicate(w1, w2, w3, w4, stat_buf, stat_buf, "INSTANCING");
	}

	return 0;
}

int npc_instanceinit(struct npc_data* nd)
{
	struct event_data *ev;
	char evname[EVENT_NAME_LENGTH];

	snprintf(evname, ARRAYLENGTH(evname), "%s::%s", nd->exname, script_config.instance_init_event_name);

	if( ( ev = (struct event_data*)strdb_get(ev_db, evname) ) )
		run_script(nd->u.scr.script,ev->pos,0,nd->bl.id);

	return 0;
}

int npc_instancedestroy(struct npc_data* nd)
{
	struct event_data *ev;
	char evname[EVENT_NAME_LENGTH];

	snprintf(evname, ARRAYLENGTH(evname), "%s::%s", nd->exname, script_config.instance_destroy_event_name);

	if( ( ev = (struct event_data*)strdb_get(ev_db, evname) ) )
		run_script(nd->u.scr.script,ev->pos,0,nd->bl.id);

	return 0;
}

#if PACKETVER >= 20131223
/**
 * Saves persistent NPC Market Data into SQL
 * @param exname NPC exname
 * @param nameid Item ID
 * @param qty Stock
 **/
void npc_market_tosql(const char *exname, struct npc_item_list *list) {
	SqlStmt* stmt = SqlStmt_Malloc(mmysql_handle);
	if (SQL_ERROR == SqlStmt_Prepare(stmt, "REPLACE INTO `%s` (`name`,`nameid`,`price`,`amount`,`flag`) VALUES ('%s','%u','%d','%d','%" PRIu8 "')",
		market_table, exname, list->nameid, list->value, list->qty, list->flag) ||
		SQL_ERROR == SqlStmt_Execute(stmt))
		SqlStmt_ShowDebug(stmt);
	SqlStmt_Free(stmt);
}

/**
 * Removes persistent NPC Market Data from SQL
 * @param exname NPC exname
 * @param nameid Item ID
 * @param clear True: will removes all records related with the NPC
 **/
void npc_market_delfromsql_(const char *exname, t_itemid nameid, bool clear) {
	SqlStmt* stmt = SqlStmt_Malloc(mmysql_handle);
	if (clear) {
		if( SQL_ERROR == SqlStmt_Prepare(stmt, "DELETE FROM `%s` WHERE `name`='%s'", market_table, exname) ||
			SQL_ERROR == SqlStmt_Execute(stmt))
			SqlStmt_ShowDebug(stmt);
	} else {
		if (SQL_ERROR == SqlStmt_Prepare(stmt, "DELETE FROM `%s` WHERE `name`='%s' AND `nameid`='%u' LIMIT 1", market_table, exname, nameid) ||
			SQL_ERROR == SqlStmt_Execute(stmt))
			SqlStmt_ShowDebug(stmt);
	}
	SqlStmt_Free(stmt);
}

/**
 * Check NPC Market Shop for each entry
 **/
static int npc_market_checkall_sub(DBKey key, DBData *data, va_list ap) {
	struct s_npc_market *market = (struct s_npc_market *)db_data2ptr(data);
	struct npc_data *nd = NULL;
	uint16 i;

	if (!market)
		return 1;

	nd = npc_name2id(market->exname);
	if (!nd) {
		ShowInfo("npc_market_checkall_sub: NPC '%s' not found, removing...\n", market->exname);
		npc_market_clearfromsql(market->exname);
		return 1;
	}
	else if (nd->subtype != NPCTYPE_MARKETSHOP || !nd->u.shop.shop_item || !nd->u.shop.count ) {
		ShowError("npc_market_checkall_sub: NPC '%s' is not proper for market, removing...\n", nd->exname);
		npc_market_clearfromsql(nd->exname);
		return 1;
	}

	if (!market->count || !market->list)
		return 1;

	for (i = 0; i < market->count; i++) {
		struct npc_item_list *list = &market->list[i];
		uint16 j;

		if (!list->nameid || !itemdb_exists(list->nameid)) {
			ShowError("npc_market_checkall_sub: NPC '%s' sells invalid item '%u', deleting...\n", nd->exname, list->nameid);
			npc_market_delfromsql(nd->exname, list->nameid);
			continue;
		}

		// Reloading stock from `market` table
		ARR_FIND(0, nd->u.shop.count, j, nd->u.shop.shop_item[j].nameid == list->nameid);
		if (j != nd->u.shop.count) {
			nd->u.shop.shop_item[j].value = list->value;
			if (nd->u.shop.shop_item[j].qty > -1)
				nd->u.shop.shop_item[j].qty = list->qty;
			nd->u.shop.shop_item[j].flag = list->flag;
			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[j]);
			continue;
		}

		if (list->flag&1) { // Item added by npcshopitem/npcshopadditem, add new entry
			RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+1);
			nd->u.shop.shop_item[j].nameid = list->nameid;
			nd->u.shop.shop_item[j].value = list->value;
			if (nd->u.shop.shop_item[j].qty > -1)
				nd->u.shop.shop_item[j].qty = list->qty;
			nd->u.shop.shop_item[j].flag = list->flag;
			nd->u.shop.count++;
			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[j]);
		}
		else { // Removing "out-of-date" entry
			ShowError("npc_market_checkall_sub: NPC '%s' does not sell item %u (qty %d), deleting...\n", nd->exname, list->nameid, list->qty);
			npc_market_delfromsql(nd->exname, list->nameid);
		}
	}

	return 0;
}

/**
 * Clear NPC market single entry
 **/
static int npc_market_free(DBKey key, DBData *data, va_list ap) {
	struct s_npc_market *market = (struct s_npc_market *)db_data2ptr(data);
	if (!market)
		return 0;
	if (market->list) {
		aFree(market->list);
		market->list = NULL;
	}
	aFree(market);
	return 1;
}

/**
 * Check all existing  NPC Market Shop after first loading map-server or after reloading scripts.
 * Overwrite stocks from NPC by using stock entries from `market` table.
 **/
static void npc_market_checkall(void) {
	if (!db_size(NPCMarketDB))
		return;
	else {
		ShowInfo("Checking '" CL_WHITE "%d" CL_RESET "' NPC Markets...\n", db_size(NPCMarketDB));
		NPCMarketDB->foreach(NPCMarketDB, npc_market_checkall_sub);
		ShowStatus("Done checking '" CL_WHITE "%d" CL_RESET "' NPC Markets.\n", db_size(NPCMarketDB));
		NPCMarketDB->clear(NPCMarketDB, npc_market_free);
	}
}

/**
 * Loads persistent NPC Market Data from SQL, use the records after NPCs init'd to reuse the stock values.
 **/
static void npc_market_fromsql(void) {
	uint32 count = 0;

	if (SQL_ERROR == Sql_Query(mmysql_handle, "SELECT `name`,`nameid`,`price`,`amount`,`flag` FROM `%s` ORDER BY `name`", market_table)) {
		Sql_ShowDebug(mmysql_handle);
		return;
	}

	while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
		char *data;
		struct s_npc_market *market;
		struct npc_item_list list;

		Sql_GetData(mmysql_handle, 0, &data, NULL);

		if (!(market = (struct s_npc_market *)strdb_get(NPCMarketDB,data))) {
			CREATE(market, struct s_npc_market, 1);
			market->count = 0;
			safestrncpy(market->exname, data, NPC_NAME_LENGTH);
			strdb_put(NPCMarketDB, market->exname, market);
		}

		Sql_GetData(mmysql_handle, 1, &data, NULL); list.nameid = strtoul(data, nullptr, 10);
		Sql_GetData(mmysql_handle, 2, &data, NULL); list.value = atoi(data);
		Sql_GetData(mmysql_handle, 3, &data, NULL); list.qty = atoi(data);
		Sql_GetData(mmysql_handle, 4, &data, NULL); list.flag = atoi(data);

		RECREATE(market->list, struct npc_item_list, market->count+1);
		market->list[market->count++] = list;
		count++;
	}
	Sql_FreeResult(mmysql_handle);

	ShowStatus("Done loading '" CL_WHITE "%d" CL_RESET "' entries for '" CL_WHITE "%d" CL_RESET "' NPC Markets from '" CL_WHITE "%s" CL_RESET "' table.\n", count, db_size(NPCMarketDB), market_table);
}
#endif

//Set mapcell CELL_NPC to trigger event later
void npc_setcells(struct npc_data* nd)
{
	int16 m = nd->bl.m, x = nd->bl.x, y = nd->bl.y, xs, ys;
	int i,j;

	switch(nd->subtype)
	{
	case NPCTYPE_WARP:
		xs = nd->u.warp.xs;
		ys = nd->u.warp.ys;
		break;
	case NPCTYPE_SCRIPT:
		xs = nd->u.scr.xs;
		ys = nd->u.scr.ys;
		break;
	default:
		return; // Other types doesn't have touch area
	}

	if (m < 0 || xs < 0 || ys < 0) //invalid range or map
		return;

	for (i = y-ys; i <= y+ys; i++) {
		for (j = x-xs; j <= x+xs; j++) {
			if (map_getcell(m, j, i, CELL_CHKNOPASS))
				continue;
			map_setcell(m, j, i, CELL_NPC, true);
		}
	}
}

int npc_unsetcells_sub(struct block_list* bl, va_list ap)
{
	struct npc_data *nd = (struct npc_data*)bl;
	int id =  va_arg(ap,int);
	if (nd->bl.id == id) return 0;
	npc_setcells(nd);
	return 1;
}

void npc_unsetcells(struct npc_data* nd)
{
	int16 m = nd->bl.m, x = nd->bl.x, y = nd->bl.y, xs, ys;
	int i,j, x0, x1, y0, y1;

	if (nd->subtype == NPCTYPE_WARP) {
		xs = nd->u.warp.xs;
		ys = nd->u.warp.ys;
	} else {
		xs = nd->u.scr.xs;
		ys = nd->u.scr.ys;
	}

	if (m < 0 || xs < 0 || ys < 0)
		return;

	struct map_data *mapdata = map_getmapdata(m);

	//Locate max range on which we can locate npc cells
	//FIXME: does this really do what it's supposed to do? [ultramage]
	for(x0 = x-xs; x0 > 0 && map_getcell(m, x0, y, CELL_CHKNPC); x0--);
	for(x1 = x+xs; x1 < mapdata->xs-1 && map_getcell(m, x1, y, CELL_CHKNPC); x1++);
	for(y0 = y-ys; y0 > 0 && map_getcell(m, x, y0, CELL_CHKNPC); y0--);
	for(y1 = y+ys; y1 < mapdata->ys-1 && map_getcell(m, x, y1, CELL_CHKNPC); y1++);

	//Erase this npc's cells
	for (i = y-ys; i <= y+ys; i++)
		for (j = x-xs; j <= x+xs; j++)
			map_setcell(m, j, i, CELL_NPC, false);

	//Re-deploy NPC cells for other nearby npcs.
	map_foreachinallarea( npc_unsetcells_sub, m, x0, y0, x1, y1, BL_NPC, nd->bl.id );
}

bool npc_movenpc(struct npc_data* nd, int16 x, int16 y)
{
	if (nd->bl.m < 0 || nd->bl.prev == NULL) 
		return false;	//Not on a map.

	struct map_data *mapdata = map_getmapdata(nd->bl.m);

	x = cap_value(x, 0, mapdata->xs-1);
	y = cap_value(y, 0, mapdata->ys-1);

	map_foreachinallrange(clif_outsight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	map_moveblock(&nd->bl, x, y, gettick());
	map_foreachinallrange(clif_insight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	return true;
}

/// Changes the display name of the npc.
///
/// @param nd Target npc
/// @param newname New display name
void npc_setdisplayname(struct npc_data* nd, const char* newname)
{
	nullpo_retv(nd);
	struct map_data *mapdata = map_getmapdata(nd->bl.m);

	safestrncpy(nd->name, newname, sizeof(nd->name));
	if( mapdata && mapdata->users )
		clif_name_area(&nd->bl);
}

/// Changes the display class of the npc.
///
/// @param nd Target npc
/// @param class_ New display class
void npc_setclass(struct npc_data* nd, short class_)
{
	nullpo_retv(nd);

	if( nd->class_ == class_ )
		return;

	nd->class_ = class_;
	status_set_viewdata(&nd->bl, class_);
	unit_refresh(&nd->bl);
}

// @commands (script based)
int npc_do_atcmd_event(struct map_session_data* sd, const char* command, const char* message, const char* eventname)
{
	struct event_data* ev = (struct event_data*)strdb_get(ev_db, eventname);
	struct npc_data *nd;
	struct script_state *st;
	int i = 0, j = 0, k = 0;
	char *temp;

	nullpo_ret(sd);

	if( ev == NULL || (nd = ev->nd) == NULL ) {
		ShowError("npc_event: event not found [%s]\n", eventname);
		return 0;
	}

	if( sd->npc_id != 0 ) { // Enqueue the event trigger.
		int l;
		ARR_FIND( 0, MAX_EVENTQUEUE, l, sd->eventqueue[l][0] == '\0' );
		if( l < MAX_EVENTQUEUE ) {
			safestrncpy(sd->eventqueue[l],eventname,EVENT_NAME_LENGTH); //Event enqueued.
			return 0;
		}

		ShowWarning("npc_event: player's event queue is full, can't add event '%s' !\n", eventname);
		return 1;
	}

	if( ev->nd->sc.option&OPTION_INVISIBLE ) { // Disabled npc, shouldn't trigger event.
		npc_event_dequeue(sd);
		return 2;
	}

	st = script_alloc_state(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	setd_sub_str( st, NULL, ".@atcmd_command$", 0, command, NULL );

	// split atcmd parameters based on spaces

	temp = (char*)aMalloc(strlen(message) + 1);

	for( i = 0; i < ( strlen( message ) + 1 ) && k < 127; i ++ ) {
		if( message[i] == ' ' || message[i] == '\0' ) {
			if (i > 0 && message[i - 1] == ' ') {
				continue; // To prevent "@atcmd [space][space]" and .@atcmd_numparameters return 1 without any parameter.
			}
			temp[k] = '\0';
			k = 0;
			if( temp[0] != '\0' ) {
				setd_sub_str( st, NULL, ".@atcmd_parameters$", j++, temp, NULL );
			}
		} else {
			temp[k] = message[i];
			k++;
		}
	 }

	setd_sub_num( st, NULL, ".@atcmd_numparameters", 0, j, NULL );
	aFree(temp);

	run_script_main(st);
	return 0;
}

/// Parses a function.
/// function%TAB%script%TAB%<function name>%TAB%{<code>}
static const char* npc_parse_function(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	DBMap* func_db;
	DBData old_data;
	struct script_code *script;
	const char* end;
	const char* script_start;

	script_start = strstr(start,"\t{");
	end = strchr(start,'\n');
	if (*w4 != '{' || script_start == NULL || (end != NULL && script_start > end)) {
		ShowError("npc_parse_function: Missing left curly '%%TAB%%{' in file '%s', line '%d'. Skipping the rest of the file.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return NULL;// can't continue
	}
	++script_start;

	end = npc_skip_script(script_start,buffer,filepath);
	if( end == NULL )
		return NULL;// (simple) parse error, don't continue

	script = parse_script(script_start, filepath, strline(buffer,start-buffer), SCRIPT_RETURN_EMPTY_SCRIPT);
	if( script == NULL )// parse error, continue
		return end;

	func_db = script_get_userfunc_db();
	if (func_db->put(func_db, db_str2key(w3), db_ptr2data(script), &old_data)) {
		struct script_code *oldscript = (struct script_code*)db_data2ptr(&old_data);

		ShowInfo("npc_parse_function: Overwriting user function [%s] (%s:%d)\n", w3, filepath, strline(buffer,start-buffer));
		script_stop_scriptinstances(oldscript);
		script_free_vars(oldscript->local.vars);
		aFree(oldscript->script_buf);
		aFree(oldscript);
	}

	return end;
}


/*==========================================
 * Parse Mob 1 - Parse mob list into each map
 * Parse Mob 2 - Actually Spawns Mob
 * [Wizputer]
 *------------------------------------------*/
void npc_parse_mob2(struct spawn_data* mob)
{
	int i;

	for( i = mob->active; i < mob->num; ++i )
	{
		struct mob_data* md = mob_spawn_dataset(mob);
		md->spawn = mob;
		md->spawn->active++;
		mob_spawn(md);
	}
}

static const char* npc_parse_mob(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	int num, mob_id, mob_lv = -1, size = -1, w1count;
	short m, x = 0, y = 0, xs = -1, ys = -1;
	char mapname[MAP_NAME_LENGTH_EXT], mobname[NAME_LENGTH];
	struct spawn_data mob, *data;
	int ai = AI_NONE; // mob_ai

	memset(&mob, 0, sizeof(struct spawn_data));

	mob.state.boss = !strcmpi(w2,"boss_monster");

	// w1=<map name>{,<x>,<y>,<xs>{,<ys>}}
	// w3=<mob name>{,<mob level>}
	// w4=<mob id>,<amount>{,<delay1>{,<delay2>{,<event>{,<mob size>{,<mob ai>}}}}}
	if( ( w1count = sscanf(w1, "%15[^,],%6hd,%6hd,%6hd,%6hd", mapname, &x, &y, &xs, &ys) ) < 1
	||	sscanf(w3, "%23[^,],%11d", mobname, &mob_lv) < 1
	||	sscanf(w4, "%11d,%11d,%11u,%11u,%77[^,],%11d,%11d[^\t\r\n]", &mob_id, &num, &mob.delay1, &mob.delay2, mob.eventname, &size, &ai) < 2 )
	{
		ShowError("npc_parse_mob: Invalid mob definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}
	if( mapindex_name2id(mapname) == 0 )
	{
		ShowError("npc_parse_mob: Unknown map '%s' in file '%s', line '%d'.\n", mapname, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}
	m =  map_mapname2mapid(mapname);
	if( m < 0 )//Not loaded on this map-server instance.
		return strchr(start,'\n');// skip and continue
	mob.m = (unsigned short)m;

	struct map_data *mapdata = map_getmapdata(m);

	if( x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys )
	{
		ShowError("npc_parse_mob: Spawn coordinates out of range: %s (%d,%d), map size is (%d,%d) - %s %s (file '%s', line '%d').\n", mapdata->name, x, y, (mapdata->xs-1), (mapdata->ys-1), w1, w3, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	// check monster ID if exists!
	if( mobdb_checkid(mob_id) == 0 )
	{
		ShowError("npc_parse_mob: Unknown mob ID %d (file '%s', line '%d').\n", mob_id, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	if( num < 1 || num > 1000 )
	{
		ShowError("npc_parse_mob: Invalid number of monsters %d, must be inside the range [1,1000] (file '%s', line '%d').\n", num, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	if( mob.state.size > SZ_BIG && size != -1 )
	{
		ShowError("npc_parse_mob: Invalid size number %d for mob ID %d (file '%s', line '%d').\n", mob.state.size, mob_id, filepath, strline(buffer, start - buffer));
		return strchr(start, '\n');
	}

	if( (mob.state.ai < AI_NONE || mob.state.ai >= AI_MAX) && ai != -1 )
	{
		ShowError("npc_parse_mob: Invalid ai %d for mob ID %d (file '%s', line '%d').\n", mob.state.ai, mob_id, filepath, strline(buffer, start - buffer));
		return strchr(start, '\n');
	}

	if( (mob_lv == 0 || mob_lv > MAX_LEVEL) && mob_lv != -1 )
	{
		ShowError("npc_parse_mob: Invalid level %d for mob ID %d (file '%s', line '%d').\n", mob_lv, mob_id, filepath, strline(buffer, start - buffer));
		return strchr(start, '\n');
	}

	mob.num = (unsigned short)num;
	mob.active = 0;
	mob.id = (short) mob_id;
	mob.x = (unsigned short)x;
	mob.y = (unsigned short)y;
	mob.xs = (signed short)xs;
	mob.ys = (signed short)ys;
	if (mob_lv > 0 && mob_lv <= MAX_LEVEL)
		mob.level = mob_lv;
	if (size > SZ_SMALL && size <= SZ_BIG)
		mob.state.size = size;
	if (ai > AI_NONE && ai <= AI_MAX)
		mob.state.ai = static_cast<enum mob_ai>(ai);

	if (mob.xs < 0) {
		if (w1count > 3) {
			ShowWarning("npc_parse_mob: Negative x-span %hd for mob ID %d (file '%s', line '%d').\n", mob.xs, mob_id, filepath, strline(buffer, start - buffer));
		}
		mob.xs = 0;
	}

	if (mob.ys < 0) {
		if (w1count > 4) {
			ShowWarning("npc_parse_mob: Negative y-span %hd for mob ID %d (file '%s', line '%d').\n", mob.ys, mob_id, filepath, strline(buffer, start - buffer));
		}
		mob.ys = 0;
	}

	if (mob.num > 1 && battle_config.mob_count_rate != 100) {
		if ((mob.num = mob.num * battle_config.mob_count_rate / 100) < 1)
			mob.num = 1;
	}

	if (battle_config.force_random_spawn || (mob.x == 0 && mob.y == 0))
	{	//Force a random spawn anywhere on the map.
		mob.x = mob.y = 0;
		mob.xs = mob.ys = -1;
	}

	// Check if monsters should have variance applied to their respawn time
	if( ( ( battle_config.mob_spawn_variance & 1 ) == 0 && mob.state.boss ) || ( ( battle_config.mob_spawn_variance & 2 ) == 0 && !mob.state.boss ) ){
		// Remove the variance
		mob.delay2 = 0;
	}

	if(mob.delay1>0xfffffff || mob.delay2>0xfffffff) {
		ShowError("npc_parse_mob: Invalid spawn delays %u %u (file '%s', line '%d').\n", mob.delay1, mob.delay2, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	//Use db names instead of the spawn file ones.
	if(battle_config.override_mob_names==1)
		strcpy(mob.name,"--en--");
	else if (battle_config.override_mob_names==2)
		strcpy(mob.name,"--ja--");
	else
		safestrncpy(mob.name, mobname, sizeof(mob.name));

	//Verify dataset.
	if( !mob_parse_dataset(&mob) )
	{
		ShowError("npc_parse_mob: Invalid dataset for monster ID %d (file '%s', line '%d').\n", mob_id, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	//Update mob spawn lookup database
	struct spawn_info spawn = { mapdata->index, mob.num };
	mob_add_spawn(mob_id, spawn);

	//Now that all has been validated. We allocate the actual memory that the re-spawn data will use.
	data = (struct spawn_data*)aMalloc(sizeof(struct spawn_data));
	memcpy(data, &mob, sizeof(struct spawn_data));

	// spawn / cache the new mobs
	if( battle_config.dynamic_mobs && map_addmobtolist(data->m, data) >= 0 )
	{
		data->state.dynamic = true;
		npc_cache_mob += data->num;

		// check if target map has players
		// (usually shouldn't occur when map server is just starting,
		// but not the case when we do @reloadscript
		if( map_getmapdata(data->m)->users > 0 )
			npc_parse_mob2(data);
	}
	else
	{
		data->state.dynamic = false;
		npc_parse_mob2(data);
		npc_delay_mob += data->num;
	}

	npc_mob++;

	return strchr(start,'\n');// continue
}

/*==========================================
 * Set or disable mapflag on map
 * eg : bat_c01	mapflag	battleground	2
 * also chking if mapflag conflict with another
 *------------------------------------------*/
static const char* npc_parse_mapflag(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	int16 m;
	char mapname[MAP_NAME_LENGTH_EXT];
	bool state = true;
	enum e_mapflag mapflag;

	// w1=<mapname>
	if (sscanf(w1, "%15[^,]", mapname) != 1) {
		ShowError("npc_parse_mapflag: Invalid mapflag definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}
	m = map_mapname2mapid(mapname);
	if (m < 0) {
		ShowWarning("npc_parse_mapflag: Unknown map '%s' in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", mapname, filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}

	if (w4 && !strcmpi(w4, "off"))
		state = false;	//Disable mapflag rather than enable it. [Skotlex]

	mapflag = map_getmapflag_by_name(w3);

	switch( mapflag ){
		case MF_INVALID:
			ShowError("npc_parse_mapflag: unrecognized mapflag '%s' (file '%s', line '%d').\n", w3, filepath, strline(buffer,start-buffer));
			break;
		case MF_NOSAVE: {
			char savemap[MAP_NAME_LENGTH_EXT];
			union u_mapflag_args args = {};

			if (state && !strcmpi(w4, "SavePoint")) {
				args.nosave.map = 0;
				args.nosave.x = -1;
				args.nosave.y = -1;
			} else if (state && sscanf(w4, "%15[^,],%6hd,%6hd", savemap, &args.nosave.x, &args.nosave.y) == 3) {
				args.nosave.map = mapindex_name2id(savemap);
				if (!args.nosave.map) {
					ShowWarning("npc_parse_mapflag: Specified save point map '%s' for mapflag 'nosave' not found (file '%s', line '%d'), using 'SavePoint'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", savemap, filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
					args.nosave.x = -1;
					args.nosave.y = -1;
				}
			}
			map_setmapflag_sub(m, MF_NOSAVE, state, &args);
			break;
		}

		case MF_PVP_NIGHTMAREDROP: {
			char drop_arg1[16], drop_arg2[16];
			union u_mapflag_args args = {};

			if (sscanf(w4, "%15[^,],%15[^,],%11d", drop_arg1, drop_arg2, &args.nightmaredrop.drop_per) == 3) {

				if (!strcmpi(drop_arg1, "random"))
					args.nightmaredrop.drop_id = -1;
				else if (itemdb_exists((args.nightmaredrop.drop_id = strtol(drop_arg1, nullptr, 10))) == NULL) {
					args.nightmaredrop.drop_id = 0;
					ShowWarning("npc_parse_mapflag: Invalid item ID '%d' supplied for mapflag 'pvp_nightmaredrop' (file '%s', line '%d'), removing.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", args.nightmaredrop.drop_id, filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
					break;
				}
				if (!strcmpi(drop_arg2, "inventory"))
					args.nightmaredrop.drop_type = NMDT_INVENTORY;
				else if (!strcmpi(drop_arg2, "equip"))
					args.nightmaredrop.drop_type = NMDT_EQUIP;
				else if (!strcmpi(drop_arg2, "all"))
					args.nightmaredrop.drop_type = NMDT_ALL;

				if (args.nightmaredrop.drop_id != 0)
					map_setmapflag_sub(m, MF_PVP_NIGHTMAREDROP, true, &args);
			} else if (!state)
				map_setmapflag(m, MF_PVP_NIGHTMAREDROP, false);
			break;
		}

		case MF_BATTLEGROUND:
			if (state) {
				union u_mapflag_args args = {};

				if (sscanf(w4, "%11d", &args.flag_val) < 1)
					args.flag_val = 1; // Default value

				map_setmapflag_sub(m, MF_BATTLEGROUND, true, &args);
			} else
				map_setmapflag(m, MF_BATTLEGROUND, false);
			break;

		case MF_NOCOMMAND:
			if (state) {
				union u_mapflag_args args = {};

				if (sscanf(w4, "%11d", &args.flag_val) < 1)
					args.flag_val = 100; // No level specified, block everyone.

				map_setmapflag_sub(m, MF_NOCOMMAND, true, &args);
			} else
				map_setmapflag(m, MF_NOCOMMAND, false);
			break;

		case MF_RESTRICTED:
			if (state) {
				union u_mapflag_args args = {};

				if (sscanf(w4, "%11d", &args.flag_val) == 1)
					map_setmapflag_sub(m, MF_RESTRICTED, true, &args);
				else // Could not be read, no value defined; don't remove as other restrictions may be set on the map
					ShowWarning("npc_parse_mapflag: Zone value not set for the restricted mapflag! Skipped flag from %s (file '%s', line '%d').\n", map_mapid2mapname(m), filepath, strline(buffer,start-buffer));
			} else
				map_setmapflag(m, MF_RESTRICTED, false);
			break;

		case MF_JEXP:
		case MF_BEXP: {
				union u_mapflag_args args = {};

				if (sscanf(w4, "%11d", &args.flag_val) < 1)
					args.flag_val = 0;

				map_setmapflag_sub(m, mapflag, state, &args);
			}
			break;

		case MF_SKILL_DAMAGE: {
			char skill_name[SKILL_NAME_LENGTH];
			char caster_constant[NAME_LENGTH];
			union u_mapflag_args args = {};

			memset(skill_name, 0, sizeof(skill_name));

			if (!state)
				map_setmapflag_sub(m, MF_SKILL_DAMAGE, false, &args);
			else {
				if (sscanf(w4, "%30[^,],%23[^,],%11d,%11d,%11d,%11d[^\n]", skill_name, caster_constant, &args.skill_damage.rate[SKILLDMG_PC], &args.skill_damage.rate[SKILLDMG_MOB], &args.skill_damage.rate[SKILLDMG_BOSS], &args.skill_damage.rate[SKILLDMG_OTHER]) >= 3) {
					if (ISDIGIT(caster_constant[0]))
						args.skill_damage.caster = atoi(caster_constant);
					else {
						int64 val_tmp;

						if (!script_get_constant(caster_constant, &val_tmp)) {
							ShowError( "npc_parse_mapflag: Unknown constant '%s'. Skipping (file '%s', line '%d').\n", caster_constant, filepath, strline(buffer, start - buffer) );
							break;
						}

						args.skill_damage.caster = static_cast<uint16>(val_tmp);
					}
					
					if (args.skill_damage.caster == 0)
						args.skill_damage.caster = BL_ALL;

					for (int i = SKILLDMG_PC; i < SKILLDMG_MAX; i++)
						args.skill_damage.rate[i] = cap_value(args.skill_damage.rate[i], -100, 100000);

					trim(skill_name);

					if (strcmp(skill_name, "all") == 0) // Adjust damage for all skills
						map_setmapflag_sub(m, MF_SKILL_DAMAGE, true, &args);
					else if (skill_name2id(skill_name) <= 0)
						ShowWarning("npc_parse_mapflag: Invalid skill name '%s' for Skill Damage mapflag. Skipping (file '%s', line '%d').\n", skill_name, filepath, strline(buffer, start - buffer));
					else { // Adjusted damage for specified skill
						args.flag_val = 1;
						map_setmapflag_sub(m, MF_SKILL_DAMAGE, true, &args);
						map_skill_damage_add(map_getmapdata(m), skill_name2id(skill_name), &args);
					}
				}
			}
			break;
		}

		case MF_SKILL_DURATION: {
			union u_mapflag_args args = {};

			if (!state)
				map_setmapflag_sub(m, MF_SKILL_DURATION, false, &args);
			else {
				char skill_name[SKILL_NAME_LENGTH];

				if (sscanf(w4, "%30[^,],%5hu[^\n]", skill_name, &args.skill_duration.per) == 2) {
					args.skill_duration.skill_id = skill_name2id(skill_name);

					if (!args.skill_duration.skill_id)
						ShowError("npc_parse_mapflag: skill_duration: Invalid skill name '%s' for Skill Duration mapflag. Skipping (file '%s', line '%d')\n", skill_name, filepath, strline(buffer, start - buffer));
					else {
						args.skill_duration.per = cap_value(args.skill_duration.per, 0, UINT16_MAX);
						map_setmapflag_sub(m, MF_SKILL_DURATION, true, &args);
					}
				}
			}
			break;
		}

		// All others do not need special treatment
		default:
			map_setmapflag(m, mapflag, state);
			break;
	}

	return strchr(start,'\n');// continue
}

/**
 * Read file and create npc/func/mapflag/monster... accordingly.
 * @param filepath : Relative path of file from map-serv bin
 * @param runOnInit :  should we exec OnInit when it's done ?
 * @return 0:error, 1:success
 */
int npc_parsesrcfile(const char* filepath)
{
	int16 m, x, y;
	int lines = 0;
	FILE* fp;
	size_t len;
	char* buffer;
	const char* p;

	if(check_filepath(filepath)!=2) { //this is not a file 
		ShowDebug("npc_parsesrcfile: Path doesn't seem to be a file skipping it : '%s'.\n", filepath);
		return 0;
	} 
            
	// read whole file to buffer
	fp = fopen(filepath, "rb");
	if( fp == NULL )
	{
		ShowError("npc_parsesrcfile: File not found '%s'.\n", filepath);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	buffer = (char*)aMalloc(len+1);
	fseek(fp, 0, SEEK_SET);
	len = fread(buffer, 1, len, fp);
	buffer[len] = '\0';
	if( ferror(fp) )
	{
		ShowError("npc_parsesrcfile: Failed to read file '%s' - %s\n", filepath, strerror(errno));
		aFree(buffer);
		fclose(fp);
		return 0;
	}
	fclose(fp);

	if ((unsigned char)buffer[0] == 0xEF && (unsigned char)buffer[1] == 0xBB && (unsigned char)buffer[2] == 0xBF) {
		// UTF-8 BOM. This is most likely an error on the user's part, because:
		// - BOM is discouraged in UTF-8, and the only place where you see it is Notepad and such.
		// - It's unlikely that the user wants to use UTF-8 data here, since we don't really support it, nor does the client by default.
		// - If the user really wants to use UTF-8 (instead of latin1, EUC-KR, SJIS, etc), then they can still do it <without BOM>.
		// More info at http://unicode.org/faq/utf_bom.html#bom5 and http://en.wikipedia.org/wiki/Byte_order_mark#UTF-8
		ShowError("npc_parsesrcfile: Detected unsupported UTF-8 BOM in file '%s'. Stopping (please consider using another character set).\n", filepath);
		aFree(buffer);
		return 0;
	}

	// parse buffer
	for( p = skip_space(buffer); p && *p ; p = skip_space(p) )
	{
		int pos[9];
		char w1[2048], w2[2048], w3[2048], w4[2048];
		int i, count;
		lines++;

		// w1<TAB>w2<TAB>w3<TAB>w4
		count = sv_parse(p, len+buffer-p, 0, '\t', pos, ARRAYLENGTH(pos), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));
		if( count < 0 )
		{
			ShowError("npc_parsesrcfile: Parse error in file '%s', line '%d'. Stopping...\n", filepath, strline(buffer,p-buffer));
			break;
		}
		// fill w1
		if( pos[3]-pos[2] > ARRAYLENGTH(w1)-1 )
			ShowWarning("npc_parsesrcfile: w1 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[3]-pos[2], filepath, strline(buffer,p-buffer));
		i = min(pos[3]-pos[2], ARRAYLENGTH(w1)-1);
		memcpy(w1, p+pos[2], i*sizeof(char));
		w1[i] = '\0';
		// fill w2
		if( pos[5]-pos[4] > ARRAYLENGTH(w2)-1 )
			ShowWarning("npc_parsesrcfile: w2 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[5]-pos[4], filepath, strline(buffer,p-buffer));
		i = min(pos[5]-pos[4], ARRAYLENGTH(w2)-1);
		memcpy(w2, p+pos[4], i*sizeof(char));
		w2[i] = '\0';
		// fill w3
		if( pos[7]-pos[6] > ARRAYLENGTH(w3)-1 )
			ShowWarning("npc_parsesrcfile: w3 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[7]-pos[6], filepath, strline(buffer,p-buffer));
		i = min(pos[7]-pos[6], ARRAYLENGTH(w3)-1);
		memcpy(w3, p+pos[6], i*sizeof(char));
		w3[i] = '\0';
		// fill w4 (to end of line)
		if( pos[1]-pos[8] > ARRAYLENGTH(w4)-1 )
			ShowWarning("npc_parsesrcfile: w4 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[1]-pos[8], filepath, strline(buffer,p-buffer));
		if( pos[8] != -1 )
		{
			i = min(pos[1]-pos[8], ARRAYLENGTH(w4)-1);
			memcpy(w4, p+pos[8], i*sizeof(char));
			w4[i] = '\0';
		}
		else
			w4[0] = '\0';

		if( count < 3 )
		{// Unknown syntax
			ShowError("npc_parsesrcfile: Unknown syntax in file '%s', line '%d'. Stopping...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,p-buffer), w1, w2, w3, w4);
			break;
		}

		if( strcmp(w1,"-") !=0 && strcasecmp(w1,"function") != 0 )
		{// w1 = <map name>,<x>,<y>,<facing>
			char mapname[MAP_NAME_LENGTH_EXT];
			int count2;

			count2 = sscanf(w1,"%15[^,],%6hd,%6hd[^,]",mapname,&x,&y);
			
			if ( count2 < 1 ) {
				ShowError("npc_parsesrcfile: Invalid script definition in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,p-buffer), w1, w2, w3, w4);
				if (strcasecmp(w2,"script") == 0 && count > 3) {
					if ((p = npc_skip_script(p,buffer,filepath)) == NULL)
						break;
				}
				p = strchr(p,'\n');// next line
				continue;
			}else if( count2 < 3 ){
				// If we were not able to parse any x and y coordinates(usually used by mapflags)
				x = y = 0;
			}

			if( !mapindex_name2id(mapname) )
			{// Incorrect map, we must skip the script info...
				ShowError("npc_parsesrcfile: Unknown map '%s' in file '%s', line '%d'. Skipping line...\n", mapname, filepath, strline(buffer,p-buffer));
				if( strcasecmp(w2,"script") == 0 && count > 3 )
				{
					if((p = npc_skip_script(p,buffer,filepath)) == NULL)
					{
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
			m = map_mapname2mapid(mapname);
			if( m < 0 )
			{// "mapname" is not assigned to this server, we must skip the script info...
				if( strcasecmp(w2,"script") == 0 && count > 3 )
				{
					if((p = npc_skip_script(p,buffer,filepath)) == NULL)
					{
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}

			struct map_data *mapdata = map_getmapdata(m);

			if (x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys) {
				ShowError("npc_parsesrcfile: Unknown coordinates ('%d', '%d') for map '%s' in file '%s', line '%d'. Skipping line...\n", x, y, mapname, filepath, strline(buffer,p-buffer));
				if( strcasecmp(w2,"script") == 0 && count > 3 )
				{
					if((p = npc_skip_script(p,buffer,filepath)) == NULL)
					{
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
		}

		if((strcasecmp(w2,"warp") == 0 || strcasecmp(w2,"warp2") == 0) && count > 3)
			p = npc_parse_warp(w1,w2,w3,w4, p, buffer, filepath);
		else if( (!strcasecmp(w2,"shop") || !strcasecmp(w2,"cashshop") || !strcasecmp(w2,"itemshop") || !strcasecmp(w2,"pointshop") || !strcasecmp(w2,"marketshop") ) && count > 3 )
			p = npc_parse_shop(w1,w2,w3,w4, p, buffer, filepath);
		else if( strcasecmp(w2,"script") == 0 && count > 3 ) {
			if( strcasecmp(w1,"function") == 0 )
				p = npc_parse_function(w1, w2, w3, w4, p, buffer, filepath);
			else
				p = npc_parse_script(w1,w2,w3,w4, p, buffer, filepath);
		}
		else if( (i=0, sscanf(w2,"duplicate%n",&i), (i > 0 && w2[i] == '(')) && count > 3 )
			p = npc_parse_duplicate(w1,w2,w3,w4, p, buffer, filepath);
		else if( (strcmpi(w2,"monster") == 0 || strcmpi(w2,"boss_monster") == 0) && count > 3 )
			p = npc_parse_mob(w1, w2, w3, w4, p, buffer, filepath);
		else if( strcmpi(w2,"mapflag") == 0 && count >= 3 )
			p = npc_parse_mapflag(w1, w2, trim(w3), trim(w4), p, buffer, filepath);
		else {
			ShowError("npc_parsesrcfile: Unable to parse, probably a missing or extra TAB in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,p-buffer), w1, w2, w3, w4);
			p = strchr(p,'\n');// skip and continue
		}
	}
	aFree(buffer);

	return 1;
}

int npc_script_event(struct map_session_data* sd, enum npce_event type){
	if (type == NPCE_MAX)
		return 0;
	if (!sd) {
		ShowError("npc_script_event: NULL sd. Event Type %d\n", type);
		return 0;
	}

	std::vector<struct script_event_s>& vector = script_event[type];

	for( struct script_event_s& evt : vector ){
		npc_event_sub( sd, evt.event, evt.event_name );
	}

	return vector.size();
}

const char *npc_get_script_event_name(int npce_index)
{
	switch (npce_index) {
	case NPCE_LOGIN:
		return script_config.login_event_name;
	case NPCE_LOGOUT:
		return script_config.logout_event_name;
	case NPCE_LOADMAP:
		return script_config.loadmap_event_name;
	case NPCE_BASELVUP:
		return script_config.baselvup_event_name;
	case NPCE_JOBLVUP:
		return script_config.joblvup_event_name;
	case NPCE_DIE:
		return script_config.die_event_name;
	case NPCE_KILLPC:
		return script_config.kill_pc_event_name;
	case NPCE_KILLNPC:
		return script_config.kill_mob_event_name;
	default:
		ShowError("npc_get_script_event_name: npce_index is outside the array limits: %d (max: %d).\n", npce_index, NPCE_MAX);
		return NULL;
	}
}

void npc_read_event_script(void)
{
	int i;

	script_event.clear();

	for (i = 0; i < NPCE_MAX; i++)
	{
		DBIterator* iter;
		DBKey key;
		DBData *data;
		char name[EVENT_NAME_LENGTH];

		safesnprintf(name,EVENT_NAME_LENGTH,"::%s", npc_get_script_event_name(i));

		iter = db_iterator(ev_db);
		for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
		{
			const char* p = key.str;
			struct event_data* ed = (struct event_data*)db_data2ptr(data);

			if( (p=strchr(p,':')) && strcmpi(name,p)==0 ){
				struct script_event_s evt;

				evt.event = ed;
				evt.event_name = key.str;

				script_event[static_cast<enum npce_event>(i)].push_back(evt);
			}
		}
		dbi_destroy(iter);
	}

	if (battle_config.etc_log) {
		//Print summary.
		for (i = 0; i < NPCE_MAX; i++)
			ShowInfo("%" PRIuPTR " '%s' events.\n", script_event[static_cast<enum npce_event>(i)].size(), npc_get_script_event_name(i));
	}
}

void npc_clear_pathlist(void) {
	struct npc_path_data *npd = NULL;
	DBIterator *path_list = db_iterator(npc_path_db);

	/* free all npc_path_data filepaths */
	for( npd = (struct npc_path_data *)dbi_first(path_list); dbi_exists(path_list); npd = (struct npc_path_data *)dbi_next(path_list) ) {
		if( npd->path )
			aFree(npd->path);
	}

	dbi_destroy(path_list);
}

//Clear then reload npcs files
int npc_reload(void) {
	struct npc_src_list *nsl;
	int npc_new_min = npc_id;
	struct s_mapiterator* iter;
	struct block_list* bl;

	/* clear guild flag cache */
	guild_flags_clear();

	npc_clear_pathlist();

	db_clear(npc_path_db);

	db_clear(npcname_db);
	db_clear(ev_db);

	//Remove all npcs/mobs. [Skotlex]

#if PACKETVER >= 20131223
	npc_market_fromsql();
#endif

	iter = mapit_geteachiddb();
	for( bl = (struct block_list*)mapit_first(iter); mapit_exists(iter); bl = (struct block_list*)mapit_next(iter) ) {
		switch(bl->type) {
		case BL_NPC:
			if( bl->id != fake_nd->bl.id )// don't remove fake_nd
				npc_unload((struct npc_data *)bl, false);
			break;
		case BL_MOB:
			unit_free(bl,CLR_OUTSIGHT);
			break;
		}
	}
	mapit_free(iter);

	// dynamic check by [random]
	if( battle_config.dynamic_mobs ){
		for (int i = 0; i < map_num; i++) {
			for( int16 j = 0; j < MAX_MOB_LIST_PER_MAP; j++ ){
				struct map_data *mapdata = map_getmapdata(i);

				if (mapdata->moblist[j] != NULL) {
					aFree(mapdata->moblist[j]);
					mapdata->moblist[j] = NULL;
				}
				if( mapdata->mob_delete_timer != INVALID_TIMER )
				{ // Mobs were removed anyway,so delete the timer [Inkfish]
					delete_timer(mapdata->mob_delete_timer, map_removemobs_timer);
					mapdata->mob_delete_timer = INVALID_TIMER;
				}

				if( mapdata->npc_num > 0 ){
					ShowWarning( "npc_reload: %d npcs weren't removed at map %s!\n", mapdata->npc_num, mapdata->name );
				}
			}
		}
	}

	// clear mob spawn lookup index
	mob_clear_spawninfo();

	npc_warp = npc_shop = npc_script = 0;
	npc_mob = npc_cache_mob = npc_delay_mob = 0;

	// reset mapflags
	map_flags_init();

	//TODO: the following code is copy-pasted from do_init_npc(); clean it up
	// Reloading npcs now
	for (nsl = npc_src_files; nsl; nsl = nsl->next) {
		ShowStatus("Loading NPC file: %s" CL_CLL "\r", nsl->name);
		npc_parsesrcfile(nsl->name);
	}
	ShowInfo ("Done loading '" CL_WHITE "%d" CL_RESET "' NPCs:" CL_CLL "\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Warps\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Shops\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Scripts\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Spawn sets\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Mobs Cached\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Mobs Not Cached\n",
		npc_id - npc_new_min, npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);

	stylist_db.reload();
	barter_db.reload();

	//Re-read the NPC Script Events cache.
	npc_read_event_script();

	/* refresh guild castle flags on both woe setups */
	npc_event_doall( script_config.agit_init_event_name );
	npc_event_doall( script_config.agit_init2_event_name );
	npc_event_doall( script_config.agit_init3_event_name );

	//Execute the OnInit event for freshly loaded npcs. [Skotlex]
	npc_event_runall(script_config.init_event_name);

	map_data_copyall();
	do_reload_instance();

	// Execute rest of the startup events if connected to char-server. [Lance]
	if(!CheckForCharServer()){
		npc_event_runall(script_config.inter_init_event_name);
	}

#if PACKETVER >= 20131223
	npc_market_checkall();
#endif
	return 0;
}

//Unload all npc in the given file
bool npc_unloadfile( const char* path ) {
	DBIterator * iter = db_iterator(npcname_db);
	struct npc_data* nd = NULL;
	bool found = false;

	for( nd = (struct npc_data*)dbi_first(iter); dbi_exists(iter); nd = (struct npc_data*)dbi_next(iter) ) {
		if( nd->path && strcasecmp(nd->path,path) == 0 ) {
			found = true;
			npc_unload_duplicates(nd);/* unload any npcs which could duplicate this but be in a different file */
			npc_unload(nd, true);
		}
	}

	dbi_destroy(iter);

	if( found ) /* refresh event cache */
		npc_read_event_script();

	npc_delsrcfile(path);

	return found;
}

void do_clear_npc(void) {
	db_clear(npcname_db);
	db_clear(ev_db);
}

/*==========================================
 * Destructor
 *------------------------------------------*/
void do_final_npc(void) {
	npc_clear_pathlist();
	script_event.clear();
	ev_db->destroy(ev_db, NULL);
	npcname_db->destroy(npcname_db, NULL);
	npc_path_db->destroy(npc_path_db, NULL);
#if PACKETVER >= 20131223
	NPCMarketDB->destroy(NPCMarketDB, npc_market_free);
#endif
	stylist_db.clear();
	barter_db.clear();
	ers_destroy(timer_event_ers);
	ers_destroy(npc_sc_display_ers);
	npc_clearsrcfile();
}

static void npc_debug_warps_sub(struct npc_data* nd)
{
	int16 m;
	if (nd->bl.type != BL_NPC || nd->subtype != NPCTYPE_WARP || nd->bl.m < 0)
		return;

	m = map_mapindex2mapid(nd->u.warp.mapindex);
	if (m < 0) return; //Warps to another map, nothing to do about it.
	if (nd->u.warp.x == 0 && nd->u.warp.y == 0) return; // random warp

	struct map_data *mapdata = map_getmapdata(m);
	struct map_data *mapdata_nd = map_getmapdata(nd->bl.m);

	if (map_getcell(m, nd->u.warp.x, nd->u.warp.y, CELL_CHKNPC)) {
		ShowWarning("Warp %s at %s(%d,%d) warps directly on top of an area npc at %s(%d,%d)\n",
			nd->name,
			mapdata_nd->name, nd->bl.x, nd->bl.y,
			mapdata->name, nd->u.warp.x, nd->u.warp.y
			);
	}
	if (map_getcell(m, nd->u.warp.x, nd->u.warp.y, CELL_CHKNOPASS)) {
		ShowWarning("Warp %s at %s(%d,%d) warps to a non-walkable tile at %s(%d,%d)\n",
			nd->name,
			mapdata_nd->name, nd->bl.x, nd->bl.y,
			mapdata->name, nd->u.warp.x, nd->u.warp.y
			);
	}
}

static void npc_debug_warps(void){
	for (int i = 0; i < map_num; i++) {
		struct map_data *mapdata = map_getmapdata(i);

		for( int i = 0; i < mapdata->npc_num; i++ ){
			npc_debug_warps_sub(mapdata->npc[i]);
		}
	}
}

/*==========================================
 * npc initialization
 *------------------------------------------*/
void do_init_npc(void){
	struct npc_src_list *file;
	int i;

	//Stock view data for normal npcs.
	memset(&npc_viewdb, 0, sizeof(npc_viewdb));
	npc_viewdb[0].class_ = JT_INVISIBLE; //Invisible class is stored here.
	for( i = 1; i < MAX_NPC_CLASS; i++ )
		npc_viewdb[i].class_ = i;
	for( i = MAX_NPC_CLASS2_START; i < MAX_NPC_CLASS2_END; i++ )
		npc_viewdb2[i - MAX_NPC_CLASS2_START].class_ = i;

	ev_db = strdb_alloc((DBOptions)(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA), EVENT_NAME_LENGTH);
	npcname_db = strdb_alloc(DB_OPT_BASE, NPC_NAME_LENGTH+1);
	npc_path_db = strdb_alloc((DBOptions)(DB_OPT_BASE|DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA),80);
#if PACKETVER >= 20131223
	NPCMarketDB = strdb_alloc(DB_OPT_BASE, NPC_NAME_LENGTH+1);
	npc_market_fromsql();
#endif

	timer_event_ers = ers_new(sizeof(struct timer_event_data),"npc.cpp::timer_event_ers",ERS_OPT_NONE);
	npc_sc_display_ers = ers_new(sizeof(struct sc_display_entry), "npc.cpp:npc_sc_display_ers", ERS_OPT_NONE);

	// process all npc files
	ShowStatus("Loading NPCs...\r");
	for( file = npc_src_files; file != NULL; file = file->next ) {
		ShowStatus("Loading NPC file: %s" CL_CLL "\r", file->name);
		npc_parsesrcfile(file->name);
	}
	ShowInfo ("Done loading '" CL_WHITE "%d" CL_RESET "' NPCs:" CL_CLL "\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Warps\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Shops\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Scripts\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Spawn sets\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Mobs Cached\n"
		"\t-'" CL_WHITE "%d" CL_RESET "' Mobs Not Cached\n",
		npc_id - START_NPC_NUM, npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);

	stylist_db.load();
	barter_db.load();

	// set up the events cache
	npc_read_event_script();

#if PACKETVER >= 20131223
	npc_market_checkall();
#endif

	//Debug function to locate all endless loop warps.
	if (battle_config.warp_point_debug)
		npc_debug_warps();

	add_timer_func_list(npc_event_do_clock,"npc_event_do_clock");
	add_timer_func_list(npc_timerevent,"npc_timerevent");

	// Init dummy NPC
	fake_nd = (struct npc_data *)aCalloc(1,sizeof(struct npc_data));
	fake_nd->bl.m = -1;
	fake_nd->bl.id = npc_get_new_npc_id();
	fake_nd->class_ = JT_FAKENPC;
	fake_nd->speed = 200;
	strcpy(fake_nd->name,"FAKE_NPC");
	memcpy(fake_nd->exname, fake_nd->name, 9);

	npc_script++;
	fake_nd->bl.type = BL_NPC;
	fake_nd->subtype = NPCTYPE_SCRIPT;

	strdb_put(npcname_db, fake_nd->exname, fake_nd);
	fake_nd->u.scr.timerid = INVALID_TIMER;
	map_addiddb(&fake_nd->bl);
	// End of initialization
}
