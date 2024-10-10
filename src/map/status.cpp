// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "status.hpp"

#include <cmath>
#include <cstdlib>
#include <functional>
#include <string>

#include <common/cbasetypes.hpp>
#include <common/ers.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "battle.hpp"
#include "battleground.hpp"
#include "clif.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "itemdb.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"
#include "script.hpp"

using namespace rathena;

// Regen related flags.
enum e_regen {
	RGN_NONE = 0x00,
	RGN_HP   = 0x01,
	RGN_SP   = 0x02,
	RGN_SHP  = 0x04,
	RGN_SSP  = 0x08,
};

static struct eri *sc_data_ers; /// For sc_data entries
static struct status_data dummy_status;

short current_equip_item_index; /// Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
unsigned int current_equip_combo_pos; /// For combo items we need to save the position of all involved items here
int current_equip_card_id; /// To prevent card-stacking (from jA) [Skotlex]
// We need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only to avoid cards exploits
short current_equip_opt_index; /// Contains random option index of an equipped item. [Secret]

uint16 SCDisabled[SC_MAX]; ///< List of disabled SC on map zones. [Cydh]

static unsigned short status_calc_str(struct block_list *,status_change *,int);
static unsigned short status_calc_agi(struct block_list *,status_change *,int);
static unsigned short status_calc_vit(struct block_list *,status_change *,int);
static unsigned short status_calc_int(struct block_list *,status_change *,int);
static unsigned short status_calc_dex(struct block_list *,status_change *,int);
static unsigned short status_calc_luk(struct block_list *,status_change *,int);
static unsigned short status_calc_pow(struct block_list *, status_change *, int);
static unsigned short status_calc_sta(struct block_list *, status_change *, int);
static unsigned short status_calc_wis(struct block_list *, status_change *, int);
static unsigned short status_calc_spl(struct block_list *, status_change *, int);
static unsigned short status_calc_con(struct block_list *, status_change *, int);
static unsigned short status_calc_crt(struct block_list *, status_change *, int);
static unsigned short status_calc_batk(struct block_list *,status_change *,int);
static unsigned short status_calc_watk(struct block_list *,status_change *,int);
static unsigned short status_calc_matk(struct block_list *,status_change *,int);
static signed short status_calc_hit(struct block_list *,status_change *,int);
static signed short status_calc_critical(struct block_list *,status_change *,int);
static signed short status_calc_flee(struct block_list *,status_change *,int);
static signed short status_calc_flee2(struct block_list *,status_change *,int);
static defType status_calc_def(struct block_list *bl, status_change *sc, int);
static signed short status_calc_def2(struct block_list *,status_change *,int);
static defType status_calc_mdef(struct block_list *bl, status_change *sc, int);
static signed short status_calc_mdef2(struct block_list *,status_change *,int);
static unsigned short status_calc_speed(struct block_list *,status_change *,int);
static short status_calc_aspd_rate(struct block_list *,status_change *,int);
static unsigned short status_calc_dmotion(struct block_list *bl, status_change *sc, int dmotion);
#ifdef RENEWAL_ASPD
static short status_calc_aspd(struct block_list *bl, status_change *sc, bool fixed);
#endif
static short status_calc_fix_aspd(struct block_list *bl, status_change *sc, int);
static signed short status_calc_patk(struct block_list *, status_change *, int);
static signed short status_calc_smatk(struct block_list *, status_change *, int);
static signed short status_calc_res(struct block_list *, status_change *, int);
static signed short status_calc_mres(struct block_list *, status_change *, int);
static signed short status_calc_hplus(struct block_list *, status_change *, int);
static signed short status_calc_crate(struct block_list *, status_change *, int);
static unsigned int status_calc_maxhp(struct block_list *bl, uint64 maxhp);
static unsigned int status_calc_maxsp(struct block_list *bl, uint64 maxsp);
static unsigned int status_calc_maxap(struct block_list *bl, uint64 maxap);
static unsigned char status_calc_element(struct block_list *bl, status_change *sc, int element);
static unsigned char status_calc_element_lv(struct block_list *bl, status_change *sc, int lv);
static int status_calc_mode(struct block_list *bl, status_change *sc, int mode);
#ifdef RENEWAL
static unsigned short status_calc_ematk(struct block_list *,status_change *,int);
#endif
static int status_get_hpbonus(struct block_list *bl, enum e_status_bonus type);
static int status_get_spbonus(struct block_list *bl, enum e_status_bonus type);
static int status_get_apbonus(struct block_list *bl, enum e_status_bonus type);
static unsigned int status_calc_maxhpsp_pc(map_session_data* sd, unsigned int stat, bool isHP);
static unsigned int status_calc_maxap_pc(map_session_data* sd);
static int status_get_sc_interval(enum sc_type type);

static bool status_change_isDisabledOnMap_(sc_type type, bool mapIsVS, bool mapIsPVP, bool mapIsGVG, bool mapIsBG, unsigned int mapZone, bool mapIsTE);
#define status_change_isDisabledOnMap(type, m) ( status_change_isDisabledOnMap_((type), mapdata_flag_vs2((m)), m->getMapFlag(MF_PVP) != 0, mapdata_flag_gvg2_no_te((m)), m->getMapFlag(MF_BATTLEGROUND) != 0, (m->zone << 3) != 0, mapdata_flag_gvg2_te((m))) )

const std::string RefineDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/refine.yml";
}

uint64 RefineDatabase::parseBodyNode( const ryml::NodeRef& node ){
	std::string group_name;

	if( !this->asString( node, "Group", group_name ) ){
		return 0;
	}

	std::string group_name_constant = "REFINE_TYPE_" + group_name;
	int64 constant;

	if( !script_get_constant( group_name_constant.c_str(), &constant ) ){
		this->invalidWarning(node["Group"], "Unknown refine group %s, skipping.\n", group_name.c_str() );
		return 0;
	}

	uint16 group_id = static_cast<uint16>( constant );

	std::shared_ptr<s_refine_info> info = this->find( group_id );
	bool exists = info != nullptr;

	if( !exists ){
		info = std::make_shared<s_refine_info>();
	}

	if( this->nodeExists( node, "Levels" ) ){
		const auto& levelsNode = node["Levels"];
		for( const auto& levelNode : levelsNode ){
			uint16 level;

			if( !this->asUInt16( levelNode, "Level", level ) ){
				return 0;
			}

			std::shared_ptr<s_refine_levels_info> levels_info = util::umap_find( info->levels, level );
			bool levels_exists = levels_info != nullptr;

			if( !levels_exists ){
				levels_info = std::make_shared<s_refine_levels_info>();
				levels_info->level = level;
			}

			if( this->nodeExists( levelNode, "RefineLevels" ) ){
				const auto& refineLevelsNode = levelNode["RefineLevels"];
				for( const auto& refineLevelNode : refineLevelsNode ){
					uint16 refine_level;

					if( !this->asUInt16( refineLevelNode, "Level", refine_level ) ){
						return 0;
					}

					if( refine_level == 0 || refine_level > MAX_REFINE ){
						this->invalidWarning( refineLevelNode["Level"], "Refine level %hu is invalid, skipping.\n", refine_level );
						return 0;
					}

					// Database is 1 based, code is 0 based
					refine_level -= 1;

					std::shared_ptr<s_refine_level_info> level_info = util::umap_find( levels_info->levels, refine_level );
					bool level_exists = level_info != nullptr;

					if( !level_exists ){
						level_info = std::make_shared<s_refine_level_info>();
						level_info->level = refine_level;
					}

					if( this->nodeExists( refineLevelNode, "Bonus" ) ){
						uint32 bonus;

						if( !this->asUInt32( refineLevelNode, "Bonus", bonus ) ){
							return 0;
						}

						level_info->bonus = bonus;
					}else{
						if( !level_exists ){
							level_info->bonus = 0;
						}
					}

					if( this->nodeExists( refineLevelNode, "RandomBonus" ) ){
						uint32 bonus;

						if( !this->asUInt32( refineLevelNode, "RandomBonus", bonus ) ){
							return 0;
						}

						level_info->randombonus_max = bonus;
					}else{
						if( !level_exists ){
							level_info->randombonus_max = 0;
						}
					}

					if( this->nodeExists( refineLevelNode, "BlacksmithBlessingAmount" ) ){
						uint16 amount;

						if( !this->asUInt16( refineLevelNode, "BlacksmithBlessingAmount", amount ) ){
							return 0;
						}

						if( amount > MAX_AMOUNT ){
							this->invalidWarning( refineLevelNode["BlacksmithBlessingAmount"], "Blacksmith Blessing amount %hu too high, capping to MAX_AMOUNT.\n", amount );
							amount = MAX_AMOUNT;
						}

						level_info->blessing_amount = amount;
					}else{
						if( !level_exists ){
							level_info->blessing_amount = 0;
						}
					}

					if (this->nodeExists(refineLevelNode, "BroadcastSuccess")) {
						bool bcast;
						if (!this->asBool(refineLevelNode, "BroadcastSuccess", bcast)) {
							return 0;
						}
						level_info->broadcast_success = bcast;
					}
					else {
						if (!level_exists) {
							level_info->broadcast_success = false;
						}
					}

					if (this->nodeExists(refineLevelNode, "BroadcastFailure")) {
						bool bcast;
						if (!this->asBool(refineLevelNode, "BroadcastFailure", bcast)) {
							return 0;
						}
						level_info->broadcast_failure = bcast;
					}
					else {
						if (!level_exists) {
							level_info->broadcast_failure = false;
						}
					}

					if( this->nodeExists( refineLevelNode, "Chances" ) ){
						const auto& chancesNode = refineLevelNode["Chances"];
						for( const auto& chanceNode : chancesNode ){
							std::string cost_name;

							if( !this->asString( chanceNode, "Type", cost_name ) ){
								return 0;
							}

							std::string cost_name_constant = "REFINE_COST_" + cost_name;

							if( !script_get_constant( cost_name_constant.c_str(), &constant ) ){
								this->invalidWarning( chanceNode["Type"], "Unknown refine cost type %s, skipping.\n", cost_name.c_str() );
								return 0;
							}

							if( constant >= REFINE_COST_MAX ){
								this->invalidWarning( chanceNode["Type"], "Refine cost type %s is unsupported, skipping.\n", cost_name.c_str() );
								return 0;
							}

							uint16 index = (uint16)constant;

							std::shared_ptr<s_refine_cost> cost = util::umap_find( level_info->costs, index );
							bool cost_exists = cost != nullptr;

							if( !cost_exists ){
								cost = std::make_shared<s_refine_cost>();
								cost->index = index;
							}

							if( this->nodeExists( chanceNode, "Rate" ) ){
								uint16 rate;

								if( !this->asUInt16Rate( chanceNode, "Rate", rate ) ){
									return 0;
								}

								cost->chance = rate;
							}else{
								if( !cost_exists ){
									cost->chance = 0;
								}
							}

							if( this->nodeExists( chanceNode, "Price" ) ){
								uint32 price;

								if( !this->asUInt32( chanceNode, "Price", price ) ){
									return 0;
								}

								if( price > MAX_ZENY ){
									this->invalidWarning( chanceNode["Price"], "Price is above MAX_ZENY, capping...\n" );
									price = MAX_ZENY;
								}

								cost->zeny = price;
							}else{
								if( !cost_exists ){
									cost->zeny = 0;
								}
							}

							if( this->nodeExists( chanceNode, "Material" ) ){
								std::string item_name;

								if( !this->asString( chanceNode, "Material", item_name ) ){
									return 0;
								}

								std::shared_ptr<item_data> id = item_db.search_aegisname( item_name.c_str() );

								if( id == nullptr ){
									this->invalidWarning( chanceNode["Material"], "Unknown refine material %s, skipping.\n", item_name.c_str() );
									return 0;
								}

								cost->nameid = id->nameid;
							}else{
								if( !cost_exists ){
									cost->nameid = 0;
								}
							}

							if( this->nodeExists( chanceNode, "BreakingRate" ) ){
								uint16 breaking_rate;

								if( !this->asUInt16Rate( chanceNode, "BreakingRate", breaking_rate ) ){
									return 0;
								}

								cost->breaking_rate = breaking_rate;
							}else{
								if( !cost_exists ){
									cost->breaking_rate = 0;
								}
							}

							if( this->nodeExists( chanceNode, "DowngradeAmount" ) ){
								uint16 downgrade_amount;

								if( !this->asUInt16( chanceNode, "DowngradeAmount", downgrade_amount ) ){
									return 0;
								}

								if( downgrade_amount > MAX_REFINE ){
									this->invalidWarning( chanceNode["DowngradeAmount"], "Downgrade amount %hu is invalid, skipping.\n", downgrade_amount );
									return 0;
								}

								cost->downgrade_amount = downgrade_amount;
							}else{
								if( !cost_exists ){
									cost->downgrade_amount = 0;
								}
							}

							if( !cost_exists ){
								level_info->costs[index] = cost;
							}
						}
					}

					if( !level_exists ){
						levels_info->levels[refine_level] = level_info;
					}
				}
			}

			if( !levels_exists ){
				info->levels[level] = levels_info;
			}
		}
	}

	if( !exists ){
		this->put( group_id, info );
	}

	return 1;
}

std::shared_ptr<s_refine_level_info> RefineDatabase::findLevelInfoSub( const struct item_data& data, struct item& item, uint16 refine ){
	// Check if the item can be refined
	if( data.flag.no_refine ){
		return nullptr;
	}

	// Cap the refine level
	if( refine > MAX_REFINE ){
		refine = MAX_REFINE;
	}

	e_refine_type type;
	uint16 level;

	if( !this->calculate_refine_info( data, type, level ) ){
		return nullptr;
	}

	std::shared_ptr<s_refine_info> info = this->find( type );

	if( info == nullptr ){
		return nullptr;
	}

	std::shared_ptr<s_refine_levels_info> levels_info = util::umap_find( info->levels, level );

	if( levels_info == nullptr ){
		return nullptr;
	}

	return util::umap_find( levels_info->levels, refine );
}

std::shared_ptr<s_refine_level_info> RefineDatabase::findLevelInfo( const struct item_data& data, struct item& item ){
	// Check the current refine level
	if( item.refine >= MAX_REFINE ){
		return nullptr;
	}

	return this->findLevelInfoSub( data, item, item.refine );
}

std::shared_ptr<s_refine_level_info> RefineDatabase::findCurrentLevelInfo( const struct item_data& data, struct item& item ){
	if( item.refine > 0 ){
		return this->findLevelInfoSub( data, item, item.refine - 1 );
	}else{
		return nullptr;
	}
}

bool RefineDatabase::calculate_refine_info( const struct item_data& data, e_refine_type& refine_type, uint16& level ){
	if( data.type == IT_WEAPON ){
		refine_type = REFINE_TYPE_WEAPON;
		level = data.weapon_level;

		return true;
	}else if( data.type == IT_ARMOR ){
		refine_type = REFINE_TYPE_ARMOR;
		level = data.armor_level;

		return true;
	}else if( data.type == IT_SHADOWGEAR ){
		if( data.equip == EQP_SHADOW_WEAPON ){
			refine_type = REFINE_TYPE_SHADOW_WEAPON;
		}else{
			refine_type = REFINE_TYPE_SHADOW_ARMOR;
		}
		level = 1;

		return true;
	}else{
		return false;
	}
}

RefineDatabase refine_db;

const std::string SizeFixDatabase::getDefaultLocation() {
	return std::string(db_path) + "/size_fix.yml";
}

/**
 * Reads and parses an entry from size_fix.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 SizeFixDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string weapon_name;

	if (!this->asString(node, "Weapon", weapon_name))
		return 0;

	std::string weapon_name_constant = "W_" + weapon_name;
	int64 constant;

	if (!script_get_constant(weapon_name_constant.c_str(), &constant)) {
		this->invalidWarning(node["Weapon"], "Size Fix unknown weapon %s, skipping.\n", weapon_name.c_str());
		return 0;
	}

	if (constant < W_FIST || constant > W_2HSTAFF) {
		this->invalidWarning(node["Weapon"], "Size Fix weapon %s is an invalid weapon, skipping.\n", weapon_name.c_str());
		return 0;
	}

	int weapon_id = static_cast<int>(constant);
	std::shared_ptr<s_sizefix_db> size = this->find(weapon_id);
	bool exists = size != nullptr;

	if (!exists)
		size = std::make_shared<s_sizefix_db>();

	if (this->nodeExists(node, "Small")) {
		uint16 small;

		if (!this->asUInt16(node, "Small", small))
			return 0;

		if (small > 100) {
			this->invalidWarning(node["Small"], "Small Size Fix %d for %s is out of bounds, defaulting to 100.\n", small, weapon_name.c_str());
			small = 100;
		}

		size->small = small;
	} else {
		if (!exists)
			size->small = 100;
	}

	if (this->nodeExists(node, "Medium")) {
		uint16 medium;

		if (!this->asUInt16(node, "Medium", medium))
			return 0;

		if (medium > 100) {
			this->invalidWarning(node["Medium"], "Medium Size Fix %d for %s is out of bounds, defaulting to 100.\n", medium, weapon_name.c_str());
			medium = 100;
		}

		size->medium = medium;
	} else {
		if (!exists)
			size->medium = 100;
	}

	if (this->nodeExists(node, "Large")) {
		uint16 large;

		if (!this->asUInt16(node, "Large", large))
			return 0;

		if (large > 100) {
			this->invalidWarning(node["Large"], "Large Size Fix %d for %s is out of bounds, defaulting to 100.\n", large, weapon_name.c_str());
			large = 100;
		}

		size->large = large;
	} else {
		if (!exists)
			size->large = 100;
	}

	if (!exists)
		this->put(weapon_id, size);

	return 1;
}

SizeFixDatabase size_fix_db;

const std::string EnchantgradeDatabase::getDefaultLocation(){
	return std::string(db_path) + "/enchantgrade.yml";
}

uint64 EnchantgradeDatabase::parseBodyNode( const ryml::NodeRef& node ){
	if( !this->nodesExist( node, { "Type", "Levels" } ) ){
		return 0;
	}

	std::string itemtype_constant;

	if( !this->asString( node, "Type", itemtype_constant ) ){
		return 0;
	}

	int64 constant_value;

	if( !script_get_constant( ( "IT_" + itemtype_constant ).c_str(), &constant_value ) ){
		this->invalidWarning( node["Type"], "Unknown item type \"%s\".\n", itemtype_constant.c_str() );
		return 0;
	}

	uint16 itemtype = static_cast<uint16>( constant_value );
	uint16 itemtype_maxlevel;

	if( itemtype == IT_WEAPON ){
		itemtype_maxlevel = MAX_WEAPON_LEVEL;
	}else if( itemtype == IT_ARMOR ){
		itemtype_maxlevel = MAX_ARMOR_LEVEL;
	}else{
		this->invalidWarning( node["Type"], "Item type \"%s\" is not supported.\n", itemtype_constant.c_str() );
		return 0;
	}

	std::shared_ptr<s_enchantgrade> enchantgrade = this->find( itemtype );
	bool exists = enchantgrade != nullptr;

	if( !exists ){
		enchantgrade = std::make_shared<s_enchantgrade>();
		enchantgrade->itemtype = itemtype;
	}

	for( const ryml::NodeRef& levelNode : node["Levels"] ){
		if( !this->nodesExist( levelNode, { "Level", "Grades" } ) ){
			return 0;
		}

		uint16 level;

		if( !this->asUInt16( levelNode, "Level", level ) ){
			return 0;
		}

		if( level == 0 || level > itemtype_maxlevel ){
			this->invalidWarning( levelNode["Level"], "Level %hu is invalid for item type %s[1~%hu].\n", level, itemtype_constant.c_str(), itemtype_maxlevel );
			return 0;
		}

		std::map<e_enchantgrade, std::shared_ptr<s_enchantgradelevel>>& grades = enchantgrade->levels[level];

		for( const ryml::NodeRef& gradeNode : levelNode["Grades"] ){
			std::string gradeConstant;

			if( !this->asString( gradeNode, "Grade", gradeConstant ) ){
				return 0;
			}

			if( !script_get_constant( ( "ENCHANTGRADE_" + gradeConstant ).c_str(), &constant_value ) ){
				this->invalidWarning( node["Grade"], "Unknown grade \"%s\".\n", gradeConstant.c_str() );
				return 0;
			}

			if( constant_value >= MAX_ENCHANTGRADE ){
				this->invalidWarning( gradeNode["Grade"], "Grade %" PRId64 " is too high. Maximum: %hu.\n", constant_value, MAX_ENCHANTGRADE - 1 );
				return 0;
			}

			e_enchantgrade gradeLevel = (e_enchantgrade)constant_value;

			std::shared_ptr<s_enchantgradelevel> grade = util::map_find( grades, gradeLevel );
			bool gradeExists = grade != nullptr;

			if( !gradeExists ){
				if( !this->nodesExist( gradeNode, { "Chances", "Options" } ) ){
					return 0;
				}

				grade = std::make_shared<s_enchantgradelevel>();
				grade->grade = gradeLevel;
				for( int i = 0; i < ARRAYLENGTH( grade->chances ); i++ ){
					grade->chances[i] = 0;
				}
			}

			if( this->nodeExists( gradeNode, "Chances" ) ){
				for( const ryml::NodeRef& chanceNode : gradeNode["Chances"] ){
					uint16 refine;

					if( !this->asUInt16( chanceNode, "Refine", refine ) ){
						return 0;
					}

					if( refine > MAX_REFINE ){
						this->invalidWarning( chanceNode["Refine"], "Refine %hu is too high. Maximum: %hu.\n", refine, MAX_REFINE );
						return 0;
					}

					uint16 chance;

					if( !this->asUInt16Rate( chanceNode, "Chance", chance ) ){
						return 0;
					}

					grade->chances[refine] = chance;
				}
			}

			if( this->nodeExists( gradeNode, "Bonus" ) ){
				uint16 bonus;

				if( !this->asUInt16( gradeNode, "Bonus", bonus ) ){
					return 0;
				}

				grade->bonus = bonus;
			}else{
				if( !gradeExists ){
					grade->bonus = 0;
				}
			}

			if( this->nodeExists( gradeNode, "AnnounceSuccess" ) ){
				bool announce;

				if( !this->asBool( gradeNode, "AnnounceSuccess", announce ) ){
					return 0;
				}

				grade->announceSuccess = announce;
			}else{
				if( !gradeExists ){
					grade->announceSuccess = true;
				}
			}

			if( this->nodeExists( gradeNode, "AnnounceFail" ) ){
				bool announce;

				if( !this->asBool( gradeNode, "AnnounceFail", announce) ){
					return 0;
				}

				grade->announceFail = announce;
			}else{
				if( !gradeExists ){
					grade->announceFail = false;
				}
			}

			if( this->nodeExists( gradeNode, "Announce" ) ){
				bool announce;

				if( !this->asBool( gradeNode, "Announce", announce ) ){
					return 0;
				}

				grade->announceSuccess = announce;
				grade->announceFail = announce;
			}

			if( this->nodeExists( gradeNode, "Catalyst") ){
				const ryml::NodeRef& catalystNode = gradeNode["Catalyst"];

				if( this->nodeExists( catalystNode, "Item" ) ){
					std::string itemName;

					if( !this->asString( catalystNode, "Item", itemName ) ){
						return 0;
					}

					std::shared_ptr<item_data> id = item_db.search_aegisname( itemName.c_str() );

					if( id == nullptr ){
						this->invalidWarning( catalystNode["Item"], "Unknown item \"%s\".\n", itemName.c_str() );
						return 0;
					}

					grade->catalyst.item = id->nameid;
				}else{
					if( !gradeExists ){
						grade->catalyst.item = 0;
					}
				}

				if( this->nodeExists( catalystNode, "AmountPerStep" ) ){
					uint16 amountPerStep;

					if( !this->asUInt16( catalystNode, "AmountPerStep", amountPerStep ) ){
						return 0;
					}

					grade->catalyst.amountPerStep = amountPerStep;
				}else{
					if( !gradeExists ){
						grade->catalyst.amountPerStep = 0;
					}
				}

				if( this->nodeExists( catalystNode, "MaximumSteps" ) ){
					uint16 maximumSteps;

					if( !this->asUInt16( catalystNode, "MaximumSteps", maximumSteps ) ){
						return 0;
					}

					grade->catalyst.maximumSteps = maximumSteps;
				}else{
					if( !gradeExists ){
						grade->catalyst.maximumSteps = 0;
					}
				}

				if( this->nodeExists( catalystNode, "ChanceIncrease" ) ){
					uint16 chanceIncrease;

					if( !this->asUInt16Rate( catalystNode, "ChanceIncrease", chanceIncrease ) ){
						return 0;
					}

					grade->catalyst.chanceIncrease = chanceIncrease;
				}else{
					if( !gradeExists ){
						grade->catalyst.chanceIncrease = 0;
					}
				}
			}else{
				if( !gradeExists ){
					grade->catalyst.item = 0;
					grade->catalyst.amountPerStep = 0;
					grade->catalyst.maximumSteps = 0;
					grade->catalyst.chanceIncrease = 0;
				}
			}

			if( this->nodeExists( gradeNode, "Options" ) ){
				for( const ryml::NodeRef& optionNode : gradeNode["Options"] ){
					uint16 optionIndex;

					if( !this->asUInt16( optionNode, "Option", optionIndex ) ){
						return 0;
					}

					std::shared_ptr<s_enchantgradeoption> option = util::map_find( grade->options, optionIndex );
					bool optionExists = option != nullptr;

					if( !optionExists ){
						option = std::make_shared<s_enchantgradeoption>();
						option->id = optionIndex;
					}

					if( this->nodeExists( optionNode, "Amount" ) ){
						uint16 amount;

						if( !this->asUInt16( optionNode, "Amount", amount ) ){
							return 0;
						}

						if( amount > MAX_AMOUNT ){
							this->invalidWarning( optionNode["Amount"], "Amount %hu is too high, capping to %hu...\n", amount, MAX_AMOUNT );
							amount = MAX_AMOUNT;
						}

						if( amount == 0 ){
							if( grade->options.erase( optionIndex ) > 0 ){
								continue;
							}else{
								this->invalidWarning( optionNode["Amount"], "Trying to remove invalid option %hu...\n", optionIndex );
								return 0;
							}
						}

						option->amount = amount;
					}else{
						if( !optionExists ){
							option->amount = 1;
						}
					}

					if( this->nodeExists( optionNode, "Item" ) ){
						std::string itemName;

						if( !this->asString( optionNode, "Item", itemName ) ){
							return 0;
						}

						std::shared_ptr<item_data> id = item_db.search_aegisname( itemName.c_str() );

						if( id == nullptr ){
							this->invalidWarning( optionNode["Item"], "Unknown item \"%s\".\n", itemName.c_str() );
							return 0;
						}

						option->item = id->nameid;
					}else{
						if( !optionExists ){
							option->item = 0;
						}
					}

					if( this->nodeExists( optionNode, "Zeny" ) ){
						uint32 zeny;

						if( !this->asUInt32( optionNode, "Zeny", zeny ) ){
							return 0;
						}

						option->zeny = zeny;
					}else{
						if( !optionExists ){
							option->zeny = 0;
						}
					}

					if( this->nodeExists( optionNode, "BreakingRate" ) ){
						uint16 breaking_rate;

						if( !this->asUInt16Rate( optionNode, "BreakingRate", breaking_rate ) ){
							return 0;
						}

						option->breaking_rate = breaking_rate;
					}else{
						if( !optionExists ){
							option->breaking_rate = 0;
						}
					}

					if( this->nodeExists( optionNode, "DowngradeAmount" ) ){
						uint16 downgrade_amount;

						if( !this->asUInt16( optionNode, "DowngradeAmount", downgrade_amount ) ){
							return 0;
						}

						if( downgrade_amount > MAX_REFINE ){
							this->invalidWarning( optionNode["DowngradeAmount"], "Downgrade amount %hu is invalid, skipping.\n", downgrade_amount );
							return 0;
						}

						option->downgrade_amount = downgrade_amount;
					}else{
						if( !optionExists ){
							option->downgrade_amount = 0;
						}
					}

					if( !optionExists ){
						grade->options[optionIndex] = option;
					}
				}
			}

			if( !gradeExists ){
				grades[gradeLevel] = grade;
			}
		}
	}

	if( !exists ){
		this->put( itemtype, enchantgrade );
	}

	return 1;
}

std::shared_ptr<s_enchantgradelevel> EnchantgradeDatabase::findCurrentLevelInfo( const struct item_data& data, struct item& item ){
	std::shared_ptr<s_enchantgrade> enchantgrade = enchantgrade_db.find( data.type );

	// Unsupported item type - no answer
	if( enchantgrade == nullptr ){
		return nullptr;
	}

	uint16 level = 0;

	if( data.type == IT_WEAPON ){
		level = data.weapon_level;
	}else if( data.type == IT_ARMOR ){
		level = data.armor_level;
	}

	const auto& enchantgradelevels = enchantgrade->levels.find( level );

	// Cannot upgrade this weapon or armor level - no answer
	if( enchantgradelevels == enchantgrade->levels.end() ){
		return nullptr;
	}

	return util::map_find( enchantgradelevels->second, (e_enchantgrade)( item.enchantgrade - 1 ) );
}

void EnchantgradeDatabase::loadingFinished(){
	for( const auto& it_itemTypes : *this ){
		for( const auto& it_itemLevels : it_itemTypes.second->levels ){
			for( const auto& it_enchantgrades : it_itemLevels.second ){
				std::shared_ptr<s_enchantgradelevel> enchantgradelevel = it_enchantgrades.second;

				if( enchantgradelevel->catalyst.amountPerStep == 0 ){
					enchantgradelevel->catalyst.item = 0;
					enchantgradelevel->catalyst.chanceIncrease = 0;
					enchantgradelevel->catalyst.maximumSteps = 0;
				}
			}
		}
	}

	TypesafeYamlDatabase::loadingFinished();
}

EnchantgradeDatabase enchantgrade_db;

/**
 * Get icon ID of SC
 * @param type: SC type
 * @return EFST ID
 **/
efst_type StatusDatabase::getIcon(sc_type type) {
	std::shared_ptr<s_status_change_db> status = status_db.find(type);

	return status ? status->icon : EFST_BLANK;
}

/**
 * Get flag of SC (SCB value) for status_calc_ flag
 * @param type: SC type
 * @return cal_flag: Calc value 
 **/
std::bitset<SCB_MAX> StatusDatabase::getCalcFlag(sc_type type) {
	std::shared_ptr<s_status_change_db> status = status_db.find(type);

	return status ? status->calc_flag : std::bitset<SCB_MAX> {};
}

/**
 * Get SC's EndOnStart list
 * @param sc: SC type
 * @return End list
 **/
std::vector<sc_type> StatusDatabase::getEndOnStart(sc_type type) {
	std::shared_ptr<s_status_change_db> status = status_db.find(type);

	return status ? status->endonstart : std::vector<sc_type> {};
}

/**
 * Get BL type to display proper effect
 * @param efst: EFST type
 * @return BL types
 **/
uint16 status_efst_get_bl_type(enum efst_type efst) {
	if (efst <= EFST_BLANK || efst >= EFST_MAX)
		return BL_PC;
	return status_db.StatusRelevantBLTypes[efst];
}

/**
 * Returns the FIRST skill (in order of definition in initChangeTables) to use a given status change.
 * Utilized for various duration lookups. Use with caution!
 * @param sc The status to look up
 * @return A skill associated with the status
 **/
uint16 StatusDatabase::getSkill(sc_type type) {
	std::shared_ptr<s_status_change_db> status = status_db.find(type);

	return status ? status->skill_id : 0;
}

/**
 * Returns if a status change flag is active or not for a SC.
 * @param sc: Status changes active on target
 * @param flag: Flag to check for
 * @return True if flag is set or false otherwise
 */
bool StatusDatabase::hasSCF(status_change *sc, e_status_change_flag flag) {
	if (sc == nullptr || sc->count == 0 || flag == SCF_NONE)
		return false;

	for (const auto &status_it : *this) {
		std::shared_ptr<s_status_change_db> status = status_it.second;

		if (sc->getSCE(status->type) && status->flag[flag])
			return true;
	}

	return false;
}

/**
 * Returns the SCB_BATTLE constant.
 * return SCB_BATTLE
 */
std::bitset<SCB_MAX> StatusDatabase::getSCB_BATTLE() {
	return this->SCB_BATTLE;
}

/**
 * Returns the SCB_ALL constant.
 * @return SCB_ALL
 */
std::bitset<SCB_MAX> StatusDatabase::getSCB_ALL() {
	return this->SCB_ALL;
}

/**
 * Removes statuses from skills that aren't part of the new class skill tree.
 * @param sd: Player data
 */
void StatusDatabase::changeSkillTree(map_session_data *sd, int32 class_) {
	if (sd == nullptr)
		return;

	std::shared_ptr<s_skill_tree> tree = skill_tree_db.find(class_ > 0 ? class_ : sd->status.class_);

	if (tree == nullptr)
		return;

	for (const auto &it : tree->skills) {
		uint16 skill_id = it.first;
		sc_type sc = skill_get_sc(skill_id);

		if (sc > SC_COMMON_MAX && sc < SC_MAX && sd->sc.getSCE(sc))
			status_change_end(&sd->bl, sc);
	}
}

/**
 * Validates if type is in SC ranges
 * @param type: SC type
 * @return True if type is in range, false otherwise
 */
bool StatusDatabase::validateStatus(sc_type type) {
	if (type > SC_NONE && type < SC_MAX)
		return true;

	return false;
}

/**
 * Removes a status based on the provided flag(s).
 * @param bl: Target to remove status from
 * @param flag: List of flags to check for removing
 */
void StatusDatabase::removeByStatusFlag(block_list *bl, std::vector<e_status_change_flag> flag) {
	if (bl == nullptr || flag.empty())
		return;

	status_change *sc = status_get_sc(bl);

	if (sc == nullptr || sc->count == 0)
		return;

	for (const auto &status_it : *this) {
		std::shared_ptr<s_status_change_db> status = status_it.second;
		sc_type type = status->type;

		if (sc->getSCE(type)) {
			for (const auto &flag_it : flag) {
				if (status->flag[flag_it])
					status_change_end(bl, type);
			}
		}
	}
}

status_change::status_change(){
	this->option = OPTION_NOTHING;
	this->opt3 = OPT3_NORMAL;
	this->opt1 = OPT1_NONE;
	this->opt2 = OPT2_NONE;
	this->count = 0;
	this->lastEffect = SC_NONE;
	this->lastEffectTimer = INVALID_TIMER;
	this->cant = {};
	this->comet_x = 0;
	this->comet_y = 0;
#ifndef RENEWAL
	this->sg_counter = 0;
#endif
	std::fill( std::begin( this->data ), std::end( this->data ), nullptr );
	this->lastStatus = { SC_NONE, nullptr };
}

/**
 * Accessor for a status_change_entry in a status_change
 */
status_change_entry * status_change::getSCE(enum sc_type type) {
	// TODO: bounds check
	if (type == lastStatus.first)
		return lastStatus.second;
	
	lastStatus = {type, data[type]};
	return lastStatus.second;
}

status_change_entry * status_change::getSCE(uint32 type) {
	return getSCE(static_cast<sc_type>(type));
}

status_change_entry * status_change::createSCE(enum sc_type type) {
	data[type] = ers_alloc(sc_data_ers, struct status_change_entry);
	lastStatus = {type, data[type]};
	return data[type];
}

/**
 * free the sce, then clear it
 */
void status_change::deleteSCE(enum sc_type type) {
	ers_free(sc_data_ers, data[type]);
	data[type] = nullptr;
	lastStatus = {type, data[type]};
}

/**
 * For when we only want to clear the sce without freeing.
 */
void status_change::clearSCE(enum sc_type type) {
	data[type] = nullptr;
	lastStatus = {type, data[type]};
}

/** Creates dummy status */
static void initDummyData(void) {
	memset(&dummy_status, 0, sizeof(dummy_status));
	dummy_status.hp =
	dummy_status.max_hp =
	dummy_status.max_sp =
	dummy_status.str =
	dummy_status.agi =
	dummy_status.vit =
	dummy_status.int_ =
	dummy_status.dex =
	dummy_status.luk =
	dummy_status.hit = 1;
	dummy_status.speed = 2000;
	dummy_status.adelay = 4000;
	dummy_status.amotion = 2000;
	dummy_status.dmotion = 2000;
	dummy_status.ele_lv = 1; // Min elemental level.
	dummy_status.mode = MD_CANMOVE;
}

/**
 * For copying a status_data structure from b to a, without overwriting current Hp, Sp, and Ap.
 * @param a: Status data structure to copy from
 * @param b: Status data structure to copy to
 */
static inline void status_cpy(struct status_data* a, const struct status_data* b)
{
	memcpy((void*)&a->max_hp, (const void*)&b->max_hp, sizeof(struct status_data)-(sizeof(a->hp)+sizeof(a->sp)+sizeof(a->ap)));
}

/**
 * Sets HP to a given value
 * Will always succeed (overrides heal impediment statuses) but can't kill an object
 * @param bl: Object whose HP will be set [PC|MOB|HOM|MER|ELEM|NPC]
 * @param hp: What the HP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect
 * @return heal or zapped HP if valid
 */
int status_set_hp(struct block_list *bl, unsigned int hp, int flag)
{
	if (hp < 1)
		return 0;

	status_data* status = status_get_status_data(*bl);

	if (status == &dummy_status)
		return 0;

	if (hp > status->max_hp)
		hp = status->max_hp;
	if (hp == status->hp)
		return 0;
	if (hp > status->hp)
		return status_heal(bl, hp - status->hp, 0, 1|flag);
	return status_zap(bl, status->hp - hp, 0);
}

/**
 * Sets Max HP to a given value
 * @param bl: Object whose Max HP will be set [PC|MOB|HOM|MER|ELEM|NPC]
 * @param maxhp: What the Max HP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect
 * @return heal or zapped HP if valid
 */
int status_set_maxhp(struct block_list *bl, unsigned int maxhp, int flag)
{
	int64 heal;

	if (maxhp < 1)
		return 0;

	status_data* status = status_get_status_data(*bl);

	if (status == &dummy_status)
		return 0;

	if (maxhp == status->max_hp)
		return 0;

	heal = maxhp - status->max_hp;
	status->max_hp = maxhp;

	if (heal > 0)
		status_heal(bl, heal, 0, 1|flag);
	else
		status_zap(bl, -heal, 0);

	return maxhp;
}

/**
 * Sets SP to a given value
 * @param bl: Object whose SP will be set [PC|HOM|MER|ELEM]
 * @param sp: What the SP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect		
 * @return heal or zapped SP if valid
 */
int status_set_sp(struct block_list *bl, unsigned int sp, int flag)
{
	status_data* status = status_get_status_data(*bl);

	if (status == &dummy_status)
		return 0;

	if (sp > status->max_sp)
		sp = status->max_sp;
	if (sp == status->sp)
		return 0;
	if (sp > status->sp)
		return status_heal(bl, 0, sp - status->sp, 1|flag);
	return status_zap(bl, 0, status->sp - sp);
}

/**
 * Sets Max SP to a given value
 * @param bl: Object whose Max SP will be set [PC|HOM|MER|ELEM]
 * @param maxsp: What the Max SP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect
 * @return heal or zapped HP if valid
 */
int status_set_maxsp(struct block_list *bl, unsigned int maxsp, int flag)
{
	if (maxsp < 1)
		return 0;

	status_data* status = status_get_status_data(*bl);

	if (status == &dummy_status)
		return 0;

	if (maxsp == status->max_sp)
		return 0;
	if (maxsp > status->max_sp)
		status_heal(bl, maxsp - status->max_sp, 0, 1|flag);
	else
		status_zap(bl, status->max_sp - maxsp, 0);

	status->max_sp = maxsp;
	return maxsp;
}

/**
* Sets AP to a given value
* @param bl: Object whose AP will be set [PC|HOM|MER|ELEM]
* @param ap: What the AP is to be set as
* @param flag: Used in case final value is higher than current
*		Use 2 to display healing effect
* @return heal or zapped AP if valid
*/
int status_set_ap(struct block_list *bl, unsigned int ap, int flag)
{
	status_data *status = status_get_status_data(*bl);

	if (status == &dummy_status)
		return 0;

	if (ap > status->max_ap)
		ap = status->max_ap;
	if (ap == status->ap)
		return 0;
	if (ap > status->ap)
		return status_heal(bl, 0, 0, ap - status->ap, 1 | flag);
	return status_zap(bl, 0, 0, status->ap - ap);
}

/**
* Sets Max AP to a given value
* @param bl: Object whose Max AP will be set [PC|HOM|MER|ELEM]
* @param maxap: What the Max AP is to be set as
* @param flag: Used in case final value is higher than current
*		Use 2 to display healing effect
* @return heal or zapped AP if valid
*/
int status_set_maxap(struct block_list *bl, unsigned int maxap, int flag)
{
	if (maxap < 1)
		return 0;

	status_data *status = status_get_status_data(*bl);

	if (status == &dummy_status)
		return 0;

	if (maxap == status->max_ap)
		return 0;
	if (maxap > status->max_ap)
		status_heal(bl, 0, 0, maxap - status->max_ap, 1 | flag);
	else
		status_zap(bl, 0, 0, status->max_ap - maxap);

	status->max_ap = maxap;
	return maxap;
}

/**
 * Takes HP/SP from an Object
 * @param bl: Object who will have HP/SP taken [PC|MOB|HOM|MER|ELEM]
 * @param hp: How much HP to charge
 * @param sp: How much SP to charge	
 * @return hp+sp through status_damage()
 * Note: HP/SP are integer values, not percentages. Values should be
 *	 calculated either within function call or before
 */
int64 status_charge(struct block_list* bl, int64 hp, int64 sp)
{
	if(!(bl->type&BL_CONSUME))
		return (int)hp+sp; // Assume all was charged so there are no 'not enough' fails.
	return status_damage(nullptr, bl, hp, sp, 0, 3, 0);
}

/**
 * Inflicts damage on the target with the according walkdelay.
 * @param src: Source object giving damage [PC|MOB|PET|HOM|MER|ELEM]
 * @param target: Target of the damage
 * @param dhp: How much damage to HP
 * @param dsp: How much damage to SP
 * @param dap: How much damage to AP
 * @param walkdelay: Amount of time before object can walk again
 * @param flag: Damage flag decides various options
 *		flag&1: Passive damage - Does not trigger cancelling status changes
 *		flag&2: Fail if there is not enough to subtract
 *		flag&4: Mob does not give EXP/Loot if killed
 *		flag&8: Used to damage SP of a dead character
 *		flag&16: Coma damage - Log it as normal damage, but actually set HP/SP to 1
 * @return hp+sp+ap
 * Note: HP/SP/AP are integer values, not percentages. Values should be
 *	 calculated either within function call or before
 */
int status_damage(struct block_list *src,struct block_list *target,int64 dhp, int64 dsp, int64 dap, t_tick walkdelay, int flag, uint16 skill_id)
{
	status_change *sc;
	int hp = (int)cap_value(dhp,INT_MIN,INT_MAX);
	int sp = (int)cap_value(dsp,INT_MIN,INT_MAX);
	int ap = (int)cap_value(dap,INT_MIN,INT_MAX);

	nullpo_ret(target);

	if(sp && !(target->type&BL_CONSUME))
		sp = 0; // Not a valid SP target.

	if (ap && !(target->type&BL_CONSUME))
		ap = 0; // Not a valid AP target.

	if (hp < 0) { // Assume absorbed damage.
		status_heal(target, -hp, 0, 1);
		hp = 0;
	}

	if (sp < 0) {
		status_heal(target, 0, -sp, 1);
		sp = 0;
	}

	if (ap < 0) {
		status_heal(target, 0, 0, -ap, 1);
		ap = 0;
	}

	if (target->type == BL_SKILL) {
		if (!src || src->type&battle_config.can_damage_skill)
			return (int)skill_unit_ondamaged((struct skill_unit *)target, hp);
		return 0;
	}

	status_data* status = status_get_status_data(*target);

	if( status == &dummy_status )
		return 0;

	if ((unsigned int)hp >= status->hp) {
		if (flag&2) return 0;
		hp = status->hp;
	}

	if ((unsigned int)sp > status->sp) {
		if (flag&2) return 0;
		sp = status->sp;
	}

	if ((unsigned int)ap > status->ap) {
		if (flag & 2) return 0;
		ap = status->ap;
	}

	if (!hp && !sp && !ap)
		return 0;

	if( !status->hp )
		flag |= 8;

	sc = status_get_sc(target);
	if (hp && battle_config.invincible_nodamage && src && sc && sc->getSCE(SC_INVINCIBLE))
		hp = 1;

	if( hp && !(flag&1) ) {
		if( sc ) {
			struct status_change_entry *sce;

			for (const auto &it : status_db) {
				sc_type type = static_cast<sc_type>(it.first);

				if (sc->getSCE(type) && it.second->flag[SCF_REMOVEONDAMAGED]) {
					// A status change that gets broken by damage should still be considered when calculating if a status change can be applied or not (for the same attack).
					// !TODO: This is a temporary solution until we refactor the code so that the calculation of an SC is done at the start of an attack rather than after the damage was applied.
					if (sc->opt1 > OPT1_NONE && sc->lastEffectTimer == INVALID_TIMER) {
						sc->lastEffectTimer = add_timer(gettick() + 10, status_clear_lastEffect_timer, target->id, 0);
						sc->lastEffect = type;
					}
					status_change_end(target, type);
				}
			}
			if ((sce=sc->getSCE(SC_ENDURE)) && !sce->val4) {
				/** [Skotlex]
				* Endure count is only reduced by non-players on non-gvg maps.
				* val4 signals infinite endure.
				**/
				if (src && src->type != BL_PC && !map_flag_gvg2(target->m) && !map_getmapflag(target->m, MF_BATTLEGROUND) && --(sce->val2) <= 0)
					status_change_end(target, SC_ENDURE);
			}
#ifndef RENEWAL
			if ((sce=sc->getSCE(SC_GRAVITATION)) && sce->val3 == BCT_SELF) {
				std::shared_ptr<s_skill_unit_group> sg = skill_id2group(sce->val4);

				if (sg) {
					skill_delunitgroup(sg);
					sce->val4 = 0;
					status_change_end(target, SC_GRAVITATION);
				}
			}
#endif
			if(sc->getSCE(SC_DANCING) && hp > (status->max_hp / 4))
				status_change_end(target, SC_DANCING);
			if(sc->getSCE(SC_CLOAKINGEXCEED) && --(sc->getSCE(SC_CLOAKINGEXCEED)->val2) <= 0)
				status_change_end(target, SC_CLOAKINGEXCEED);
			if(sc->getSCE(SC_KAGEMUSYA) && --(sc->getSCE(SC_KAGEMUSYA)->val3) <= 0)
				status_change_end(target, SC_KAGEMUSYA);
		}

		if (target->type == BL_PC)
			pc_bonus_script_clear(BL_CAST(BL_PC,target),BSF_REM_ON_DAMAGED);
		unit_skillcastcancel(target, 2);
	}

	// We need to log the real damage on exp_calc_type 1
	if (battle_config.exp_calc_type == 1) {
		dhp = hp;
		// Coma real damage
		if (flag&16)
			dhp = status->hp - 1;
	}

	status->hp-= hp;
	status->sp-= sp;
	status->ap-= ap;

	// Coma
	if (flag&16) {
		status->hp = 1;
		status->sp = 1;
		if (!sp) sp = 1; // To make sure the status bar is updated
	}

	if (sc && hp && status->hp) {
		if (sc->getSCE(SC_AUTOBERSERK) &&
			(!sc->getSCE(SC_PROVOKE) || !sc->getSCE(SC_PROVOKE)->val4) &&
			status->hp < status->max_hp / 4)
			sc_start4(src,target,SC_PROVOKE,100,10,0,0,1,0);
		if (sc->getSCE(SC_BERSERK) && status->hp <= 100)
			status_change_end(target, SC_BERSERK);
		if( sc->getSCE(SC_RAISINGDRAGON) && status->hp <= 1000 )
			status_change_end(target, SC_RAISINGDRAGON);
		if (sc->getSCE(SC_SATURDAYNIGHTFEVER) && status->hp <= 100)
			status_change_end(target, SC_SATURDAYNIGHTFEVER);
	}

	// Need to pass original HP damage for the mob damage log
	dhp = cap_value(dhp, INT_MIN, INT_MAX);
	switch (target->type) {
	case BL_PC:
		pc_damage(reinterpret_cast<map_session_data*>(target), src, hp, sp, ap); break;
	case BL_MOB:
		mob_damage(reinterpret_cast<mob_data*>(target), src, (int)dhp);
		break;
	case BL_HOM:
		hom_heal(reinterpret_cast<homun_data&>(*target), hp != 0, sp != 0);
		break;
	case BL_MER:
		mercenary_heal(reinterpret_cast<s_mercenary_data*>(target), hp, sp);
		break;
	case BL_ELEM:
		elemental_heal(reinterpret_cast<s_elemental_data*>(target), hp, sp);
		break;
	}

	if( src && target->type == BL_PC && ((TBL_PC*)target)->disguise ) { // Stop walking when attacked in disguise to prevent walk-delay bug
		unit_stop_walking( target, 1 );
	}

	if( status->hp || (flag&8) ) { // Still lives or has been dead before this damage.
		if (walkdelay)
			unit_set_walkdelay(target, gettick(), walkdelay, 0);
		return (int)(hp+sp+ap);
	}

	status->hp = 0;
	/** [Skotlex]
	* NOTE: These dead functions should return:
	* 0: Death cancelled, auto-revived.
	* Non-zero: Standard death. Clear status, cancel move/attack, etc
	* &2: Remove object from map.
	* &4: Delete object from memory. (One time spawn mobs)
	**/
	switch (target->type) {
		case BL_PC:  flag = pc_dead((TBL_PC*)target,src); break;
		case BL_MOB: flag = mob_dead((TBL_MOB*)target, src, flag&4?3:0); break;
		case BL_HOM: flag = hom_dead((TBL_HOM*)target); break;
		case BL_MER: flag = mercenary_dead((TBL_MER*)target); break;
		case BL_ELEM: flag = elemental_dead((TBL_ELEM*)target); break;
		default:	// Unhandled case, do nothing to object.
			flag = 0;
			break;
	}

	if(!flag) // Death cancelled.
		return (int)(hp+sp+ap);

	// Normal death
	if (battle_config.clear_unit_ondeath &&
		battle_config.clear_unit_ondeath&target->type)
		skill_clear_unitgroup(target);

	if(target->type&BL_REGEN) { // Reset regen ticks.
		struct regen_data *regen = status_get_regen_data(target);
		if (regen) {
			memset(&regen->tick, 0, sizeof(regen->tick));
			if (regen->sregen)
				memset(&regen->sregen->tick, 0, sizeof(regen->sregen->tick));
			if (regen->ssregen)
				memset(&regen->ssregen->tick, 0, sizeof(regen->ssregen->tick));
		}
	}

	if( sc && sc->getSCE(SC_KAIZEL) && !map_flag_gvg2(target->m) ) { // flag&8 = disable Kaizel
		int time = skill_get_time2(SL_KAIZEL,sc->getSCE(SC_KAIZEL)->val1);
		// Look for Osiris Card's bonus effect on the character and revive 100% or revive normally
		if ( target->type == BL_PC && BL_CAST(BL_PC,target)->special_state.restart_full_recover )
			status_revive(target, 100, 100);
		else
			status_revive(target, sc->getSCE(SC_KAIZEL)->val2, 0);
		status_change_clear(target,0);
		clif_skill_nodamage(target,*target,ALL_RESURRECTION,1);
		sc_start(src,target,SC_KYRIE,100,10,time);

		if( target->type == BL_MOB )
			((TBL_MOB*)target)->state.rebirth = 1;

		return (int)(hp+sp+ap);
	}

	// Disable Ultimate Sacrifice on GVG maps
	if (sc && sc->getSCE(SC_ULTIMATE_S) && !map_flag_gvg2(target->m)) {
		status_revive(target, 100, 100);
		status_change_clear(target, 0);
		clif_skill_nodamage(target, *target, ALL_RESURRECTION, 1);

		if (target->type == BL_MOB)
			((TBL_MOB*)target)->state.rebirth = 1;

		return (int)(hp+sp+ap);
	}

	if (target->type == BL_MOB && sc && sc->getSCE(SC_REBIRTH) && !((TBL_MOB*) target)->state.rebirth) { // Ensure the monster has not already rebirthed before doing so.
		status_revive(target, sc->getSCE(SC_REBIRTH)->val2, 0);
		status_change_clear(target,0);
		((TBL_MOB*)target)->state.rebirth = 1;

		return (int)(hp+sp+ap);
	}

	status_change_clear(target,0);

	if(flag&4) // Delete from memory. (also invokes map removal code)
		unit_free(target,CLR_DEAD);
	else if(flag&2) // remove from map
		unit_remove_map(target,CLR_DEAD);
	else { // Some death states that would normally be handled by unit_remove_map
		unit_stop_attack(target);
		unit_stop_walking(target,1);
		unit_skillcastcancel(target,0);
		clif_clearunit_area( *target, CLR_DEAD );
		skill_unit_move(target,gettick(),4);
		skill_cleartimerskill(target);
	}

	// Always run NPC scripts for players last
	//FIXME those ain't always run if a player die if he was resurrect meanwhile
	//cf SC_REBIRTH, SC_KAIZEL, pc_dead...
	if(target->type == BL_PC) {
		TBL_PC *sd = BL_CAST(BL_PC,target);
		if( sd->bg_id ) {
			std::shared_ptr<s_battleground_data> bg = util::umap_find(bg_team_db, sd->bg_id);

			if( bg && !(bg->die_event.empty()) )
				npc_event(sd, bg->die_event.c_str(), 0);
		}

		npc_script_event( *sd, NPCE_DIE );
	}

	return (int)(hp+sp+ap);
}

/**
 * Heals an object
 * @param bl: Object to heal [PC|MOB|HOM|MER|ELEM]
 * @param hhp: How much HP to heal
 * @param hsp: How much SP to heal
 * @param hap: How much AP to heal
 * @param flag:	Whether it's Forced(&1), gives HP/SP/AP(&2) heal effect,
 *      or gives HP(&4) heal effect with 0 heal
 *		Forced healing overrides heal impedement statuses (Berserk)
 * @return hp+sp+ap
 */
int status_heal(struct block_list *bl,int64 hhp,int64 hsp, int64 hap, int flag)
{
	status_change *sc;
	int hp = (int)cap_value(hhp,INT_MIN,INT_MAX);
	int sp = (int)cap_value(hsp,INT_MIN,INT_MAX);
	int ap = (int)cap_value(hap,INT_MIN,INT_MAX);

	status_data* status = status_get_status_data(*bl);

	if (status == &dummy_status || !status->hp)
		return 0;

	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = nullptr;

	if (hp < 0) {
		if (hp == INT_MIN) // -INT_MIN == INT_MIN in some architectures!
			hp++;
		status_damage(nullptr, bl, -hp, 0, 0, 1, 0);
		hp = 0;
	}

	if(hp) {
		if( sc && (sc->getSCE(SC_BERSERK)) ) {
			if( flag&1 )
				flag &= ~2;
			else
				hp = 0;
		}

		if((unsigned int)hp > status->max_hp - status->hp)
			hp = status->max_hp - status->hp;
	}

	if(sp < 0) {
		if (sp == INT_MIN)
			sp++;
		status_damage(nullptr, bl, 0, -sp, 0, 1, 0);
		sp = 0;
	}

	if(sp) {
		if((unsigned int)sp > status->max_sp - status->sp)
			sp = status->max_sp - status->sp;
	}

	if (ap < 0) {
		if (ap == INT_MIN)
			ap++;
		status_damage(nullptr, bl, 0, 0, -ap, 0, 1, 0);
		ap = 0;
	}

	if (ap) {
		if ((unsigned int)ap > status->max_ap - status->ap)
			ap = status->max_ap - status->ap;
	}

	if(!ap && !sp && !hp && !(flag&4))
		return 0;

	status->hp += hp;
	status->sp += sp;
	status->ap += ap;

	if(hp && sc &&
		sc->getSCE(SC_AUTOBERSERK) &&
		sc->getSCE(SC_PROVOKE) &&
		sc->getSCE(SC_PROVOKE)->val4==1 &&
		status->hp >= status->max_hp / 4
	)	// End auto berserk.
		status_change_end(bl, SC_PROVOKE);

	// Send HP update to client
	switch(bl->type) {
	case BL_PC:
		pc_heal(reinterpret_cast<map_session_data*>(bl), hp, sp, ap, flag);
		break;
	case BL_MOB:
		mob_heal(reinterpret_cast<mob_data*>(bl), hp);
		break;
	case BL_HOM:
		hom_heal(reinterpret_cast<homun_data&>(*bl), hp != 0, sp != 0);
		break;
	case BL_MER:
		mercenary_heal(reinterpret_cast<s_mercenary_data*>(bl), hp, sp);
		break;
	case BL_ELEM:
		elemental_heal(reinterpret_cast<s_elemental_data*>(bl), hp, sp);
		break;
	}

	return (int)hp+sp+ap;
}

/**
 * Applies percentage based damage to a unit.
 * If a mob is killed this way and there is no src, no EXP/Drops will be awarded.
 * @param src: Object initiating HP/SP/AP modification [PC|MOB|PET|HOM|MER|ELEM]
 * @param target: Object to modify HP/SP/AP
 * @param hp_rate: Percentage of HP to modify. If > 0:percent is of current HP, if < 0:percent is of max HP
 * @param sp_rate: Percentage of SP to modify. If > 0:percent is of current SP, if < 0:percent is of max SP
 * @param ap_rate: Percentage of AP to modify. If > 0:percent is of current AP, if < 0:percent is of max AP
 * @param flag: \n
 *		0: Heal target \n 
 *		1: Use status_damage \n 
 *		2: Use status_damage and make sure target must not die from subtraction
 * @return hp+sp+ap through status_heal()
 */
int status_percent_change(struct block_list *src, struct block_list *target, int8 hp_rate, int8 sp_rate, int8 ap_rate, uint8 flag)
{
	unsigned int hp = 0, sp = 0, ap = 0;
	status_data* status = status_get_status_data(*target);

	// It's safe now [MarkZD]
	if (hp_rate > 99)
		hp = status->hp;
	else if (hp_rate > 0)
		hp = apply_rate(status->hp, hp_rate);
	else if (hp_rate < -99)
		hp = status->max_hp;
	else if (hp_rate < 0)
		hp = (apply_rate(status->max_hp, -hp_rate));
	if (hp_rate && !hp)
		hp = 1;

	if (flag == 2 && hp >= status->hp)
		hp = status->hp-1; // Must not kill target.

	if (sp_rate > 99)
		sp = status->sp;
	else if (sp_rate > 0)
		sp = apply_rate(status->sp, sp_rate);
	else if (sp_rate < -99)
		sp = status->max_sp;
	else if (sp_rate < 0)
		sp = (apply_rate(status->max_sp, -sp_rate));
	if (sp_rate && !sp)
		sp = 1;

	if (ap_rate > 99)
		ap = status->ap;
	else if (ap_rate > 0)
		ap = apply_rate(status->ap, ap_rate);
	else if (ap_rate < -99)
		ap = status->max_ap;
	else if (ap_rate < 0)
		ap = (apply_rate(status->max_ap, -ap_rate));
	if (ap_rate && !ap)
		ap = 1;

	// Ugly check in case damage dealt is too much for the received args of
	// status_heal / status_damage. [Skotlex]
	if (hp > INT_MAX) {
		hp -= INT_MAX;
		if (flag)
			status_damage(src, target, INT_MAX, 0, 0, (!src||src==target?5:1), 0);
		else
			status_heal(target, INT_MAX, 0, 0);
	}
	if (sp > INT_MAX) {
		sp -= INT_MAX;
		if (flag)
			status_damage(src, target, 0, INT_MAX, 0, (!src||src==target?5:1), 0);
		else
			status_heal(target, 0, INT_MAX, 0);
	}
	if (ap > INT_MAX) {
		ap -= INT_MAX;
		if (flag)
			status_damage(src, target, 0, 0, INT_MAX, 0, (!src || src == target ? 5 : 1), 0);
		else
			status_heal(target, 0, 0, INT_MAX, 0);
	}
	if (flag)
		return status_damage(src, target, hp, sp, ap, 0, (!src||src==target?5:1), 0);
	return status_heal(target, hp, sp, ap, 0);
}

/**
 * Revives a unit
 * @param bl: Object to revive [PC|MOB|HOM]
 * @param per_hp: Percentage of HP to revive with
 * @param per_sp: Percentage of SP to revive with
 * @param per_ap: Percentage of AP to revive with
 * @return Successful (1) or Invalid target (0)
 */
int status_revive(struct block_list *bl, unsigned char per_hp, unsigned char per_sp, unsigned char per_ap)
{
	unsigned int hp, sp, ap;
	if (!status_isdead(*bl)) return 0;

	status_data* status = status_get_status_data(*bl);

	if (status == &dummy_status)
		return 0; // Invalid target.

	hp = (int64)status->max_hp * per_hp/100;
	sp = (int64)status->max_sp * per_sp/100;
	ap = (int64)status->max_ap * per_ap/100;

	if(hp > status->max_hp - status->hp)
		hp = status->max_hp - status->hp;
	else if (per_hp && !hp)
		hp = 1;

	if(sp > status->max_sp - status->sp)
		sp = status->max_sp - status->sp;
	else if (per_sp && !sp)
		sp = 1;

	if (ap > status->max_ap - status->ap)
		ap = status->max_ap - status->ap;
	else if (per_ap && !ap)
		ap = 1;

	status->hp += hp;
	status->sp += sp;
	status->ap += ap;

	if (bl->prev) // Animation only if character is already on a map.
		clif_resurrection( *bl );
	switch (bl->type) {
		case BL_PC:  pc_revive((TBL_PC*)bl, hp, sp, ap); break;
		case BL_MOB: mob_revive((TBL_MOB*)bl, hp); break;
		case BL_HOM: hom_revive((TBL_HOM*)bl, hp, sp); break;
	}
	return 1;
}

/**
 * Checks whether the src can use the skill on the target,
 * taking into account status/option of both source/target
 * @param src:	Object using skill on target [PC|MOB|PET|HOM|MER|ELEM]
		src MAY be nullptr to indicate we shouldn't check it, this is a ground-based skill attack
 * @param target: Object being targeted by src [PC|MOB|HOM|MER|ELEM]
		 target MAY be nullptr, which checks if src can cast skill_id on the ground
 * @param skill_id: Skill ID being used on target
 * @param flag:	0 - Trying to use skill on target
 *		1 - Cast bar is done
 *		2 - Skill already pulled off, check is due to ground-based skills or splash-damage ones
 * @return src can use skill (1) or cannot use skill (0)
 * @author [Skotlex]
 */
bool status_check_skilluse(struct block_list *src, struct block_list *target, uint16 skill_id, int flag) {
	status_data* status;
	int hide_flag;

	if (src) {
		if (src->type != BL_PC && status_isdead(*src))
			return false;
		status = status_get_status_data(*src);
	}else{
		status = &dummy_status;
	}

	status_change *sc = status_get_sc(src);
	status_change *tsc = status_get_sc(target);

	if (!skill_id) { // Normal attack checks.
		if (sc && sc->cant.attack)
			return false;
		// This mode is only needed for melee attacking.
		if (!status_has_mode(status,MD_CANATTACK))
			return false;
		// Dead state is not checked for skills as some skills can be used
		// on dead characters, said checks are left to skill.cpp [Skotlex]
		if (target && status_isdead(*target))
			return false;
	}

	switch( skill_id ) {
#ifndef RENEWAL
		case PA_PRESSURE:
			if( flag && tsc && tsc->option&OPTION_HIDE)
				return false; // Gloria Avoids pretty much everything....
			break;
#endif
		case GN_WALLOFTHORN:
			if( target && status_isdead(*target) )
				return false;
			break;
		case AL_TELEPORT:
		case ALL_ODINS_POWER:
			// Should fail when used on top of Land Protector [Skotlex]
			if (src && map_getcell(src->m, src->x, src->y, CELL_CHKLANDPROTECTOR)
				&& !status_has_mode(status,MD_STATUSIMMUNE)
				&& (src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_id))
				return false;
			break;
		case SC_MANHOLE:
			// Skill is disabled against special racial grouped monsters(GvG and Battleground)
			if (target && ( util::vector_exists(status_get_race2(target), RC2_GVG) || util::vector_exists(status_get_race2(target), RC2_BATTLEFIELD) ) )
				return false;
		default:
			break;
	}

	if( sc && sc->count ) {
		if (sc->getSCE(SC_ALL_RIDING))
			return false; //You can't use skills while in the new mounts (The client doesn't let you, this is to make cheat-safe)

		if (flag == 1 && !status_has_mode(status,MD_STATUSIMMUNE) && ( // Applies to after cast completion only and doesn't apply to Boss monsters.
			(sc->getSCE(SC_ASH) && rnd()%2) || // Volcanic Ash has a 50% chance of causing skills to fail.
			(sc->getSCE(SC_KYOMU) && rnd()%100 < 25) // Kyomu has a 25% chance of causing skills fail.
		)) {
			if (src->type == BL_PC)
				clif_skill_fail( *((map_session_data*)src), skill_id );
			return false;
		}

		if (skill_id > 0 && sc->opt1 && sc->opt1 != OPT1_STONEWAIT && sc->opt1 != OPT1_BURNING && skill_id != RK_REFRESH && skill_id != SU_GROOMING && skill_id != SR_GENTLETOUCH_CURE) { // Stuned/Frozen/etc
			if (flag != 1) // Can't cast, casted stuff can't damage.
				return false;
			if (skill_get_casttype(skill_id) == CAST_DAMAGE)
				return false; // Damage spells stop casting.
		}

		if (
			(sc->getSCE(SC_TRICKDEAD) && skill_id != NV_TRICKDEAD)
			|| (sc->getSCE(SC_AUTOCOUNTER) && !flag && skill_id)
			|| (sc->getSCE(SC_GOSPEL) && sc->getSCE(SC_GOSPEL)->val4 == BCT_SELF && skill_id != PA_GOSPEL)
			|| (sc->getSCE(SC_SUHIDE) && skill_id != SU_HIDE)
		)
			return false;

		if (sc->getSCE(SC_WINKCHARM) && target && !flag) { // Prevents skill usage
			block_list *wink_target = map_id2bl(sc->getSCE(SC_WINKCHARM)->val2);

			if (wink_target != nullptr) {
				unit_data *wink_ud = unit_bl2ud(src);
				if (wink_ud != nullptr && wink_ud->walktimer == INVALID_TIMER)
					unit_walktobl(src, map_id2bl(sc->getSCE(SC_WINKCHARM)->val2), 3, 1);
				clif_emotion(src, ET_THROB);
				return false;
			} else
				status_change_end(src, SC_WINKCHARM);
		}

		if (sc->getSCE(SC_BLADESTOP)) {
			switch (sc->getSCE(SC_BLADESTOP)->val1) {
				case 5:
					if (skill_id == MO_EXTREMITYFIST)
						break;
					[[fallthrough]];
				case 4:
					if (skill_id == MO_CHAINCOMBO)
						break;
					[[fallthrough]];
				case 3:
					if (skill_id == MO_INVESTIGATE)
						break;
					[[fallthrough]];
				case 2:
					if (skill_id == MO_FINGEROFFENSIVE)
						break;
					[[fallthrough]];
				default:
					return false;
			}
		}

		if (sc->getSCE(SC_DANCING) && flag!=2) {
			std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

			if (!skill)
				return false;

			if (src->type == BL_PC && ((skill_id >= WA_SWING_DANCE && skill_id <= WM_UNLIMITED_HUMMING_VOICE ) ||
				skill_id == WM_FRIGG_SONG))
			{ // Lvl 5 Lesson or higher allow you use 3rd job skills while dancing.
				if( pc_checkskill((TBL_PC*)src,WM_LESSON) < 5 )
					return false;
#ifndef RENEWAL
			} else if(sc->getSCE(SC_LONGING)) { // Allow everything except dancing/re-dancing. [Skotlex]
				if (skill_id == BD_ENCORE || skill->inf2[INF2_ISSONG] || skill->inf2[INF2_ISENSEMBLE])
					return false;
#endif
			} else if(!skill->inf2[INF2_ALLOWWHENPERFORMING]) // Skills that can be used in dancing state
				return false;
#ifndef RENEWAL
			if ((sc->getSCE(SC_DANCING)->val1&0xFFFF) == CG_HERMODE && skill_id == BD_ADAPTATION)
				return false; // Can't amp out of Wand of Hermode :/ [Skotlex]
#endif
		}

		if (skill_id && // Do not block item-casted skills.
			(src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_id)
		) {	// Skills blocked through status changes...
			if (!flag && ( // Blocked only from using the skill (stuff like autospell may still go through
				( sc->cant.cast && skill_id != RK_REFRESH && skill_id != SU_GROOMING && skill_id != SR_GENTLETOUCH_CURE ) ||
#ifndef RENEWAL
				(sc->getSCE(SC_BASILICA) && (sc->getSCE(SC_BASILICA)->val4 != src->id || skill_id != HP_BASILICA)) || // Only Basilica caster that can cast, and only Basilica to cancel it
#endif
				(sc->getSCE(SC_MARIONETTE) && skill_id != CG_MARIONETTE) || // Only skill you can use is marionette again to cancel it
				(sc->getSCE(SC_MARIONETTE2) && skill_id == CG_MARIONETTE) || // Cannot use marionette if you are being buffed by another
				(sc->getSCE(SC_ANKLE) && skill_block_check(src, SC_ANKLE, skill_id)) ||
				(sc->getSCE(SC_STASIS) && skill_block_check(src, SC_STASIS, skill_id)) ||
				(sc->getSCE(SC_BITE) && skill_block_check(src, SC_BITE, skill_id)) ||
				(sc->getSCE(SC_NOVAEXPLOSING) && skill_block_check(src, SC_NOVAEXPLOSING, skill_id)) ||
				(sc->getSCE(SC_GRAVITYCONTROL) && skill_block_check(src, SC_GRAVITYCONTROL, skill_id)) ||
				(sc->getSCE(SC_KAGEHUMI) && skill_block_check(src, SC_KAGEHUMI, skill_id))
#ifdef RENEWAL
				|| (sc->getSCE(SC_ENSEMBLEFATIGUE) && skill_id != CG_SPECIALSINGER)
#endif
			))
				return false;

			// Skill blocking.
			if (
				(sc->getSCE(SC_VOLCANO) && skill_id == WZ_ICEWALL) ||
#ifndef RENEWAL
				(sc->getSCE(SC_ROKISWEIL) && skill_id != BD_ADAPTATION) ||
#endif
				(sc->getSCE(SC_HERMODE) && skill_get_inf(skill_id) & INF_SUPPORT_SKILL) ||
				(sc->getSCE(SC_NOCHAT) && sc->getSCE(SC_NOCHAT)->val1&MANNER_NOSKILL)
			)
				return false;
		}

		if (sc->option) {
			if ((sc->option&OPTION_HIDE) && src->type == BL_PC && (skill_id == 0 || !skill_get_inf2(skill_id, INF2_ALLOWWHENHIDDEN))) {
				// Non players can use all skills while hidden.
				return false;
			}
			if (sc->option&OPTION_CHASEWALK && skill_id != ST_CHASEWALK)
				return false;
		}
	}

	if (target == nullptr || target == src) // No further checking needed.
		return true;

	if (tsc && tsc->count) {
		if(!skill_id && tsc->getSCE(SC_TRICKDEAD))
			return false;
		if((skill_id == WZ_STORMGUST || skill_id == WZ_FROSTNOVA || skill_id == NJ_HYOUSYOURAKU || skill_id == NPC_STORMGUST2)
			&& tsc->getSCE(SC_FREEZE))
			return false;
		if(skill_id == PR_LEXAETERNA && (tsc->getSCE(SC_FREEZE) || tsc->getSCE(SC_STONE)))
			return false;
		if (tsc->getSCE(SC__MANHOLE) && !skill_get_inf2(skill_id, INF2_TARGETMANHOLE))
			return false;
	}

	// If targetting, cloak+hide protect you, otherwise only hiding does.
	hide_flag = flag?OPTION_HIDE:(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK);

 	// Skill that can hit hidden target
	if( skill_get_inf2(skill_id, INF2_TARGETHIDDEN) )
		hide_flag &= ~OPTION_HIDE;

	switch( target->type ) {
		case BL_PC: {
				map_session_data *tsd = (TBL_PC*)target;
				bool is_boss = (src && status_get_class_(src) == CLASS_BOSS);
				bool is_detect = status_has_mode(status,MD_DETECTOR);

				if (pc_isinvisible(tsd))
					return false;
				if (tsc) {
					if ((tsc->option&hide_flag) && !is_boss && (tsd->special_state.perfect_hiding || !is_detect))
						return false;
					if ((tsc->getSCE(SC_CLOAKINGEXCEED) || tsc->getSCE(SC_NEWMOON)) && !is_boss && (tsd->special_state.perfect_hiding || is_detect))
						return false; // Works against insect and demon but not against bosses
					if (tsc->getSCE(SC__FEINTBOMB) && (is_boss || is_detect))
						return false; // Works against all
					if ((tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD) || tsc->getSCE(SC_SUHIDE)) && !(is_boss || is_detect) && (!skill_id || (!flag && src)))
						return false; // Insect, demon, and boss can detect
				}
			}
			break;
		case BL_ITEM: // Allow targetting of items to pick'em up (or in the case of mobs, to loot them).
			// !TODO: Would be nice if this could be used to judge whether the player can or not pick up the item it targets. [Skotlex]
			if (status_has_mode(status,MD_LOOTER))
				return true;
			return false;
		case BL_HOM:
		case BL_MER:
		case BL_ELEM:
			if( target->type == BL_HOM && skill_id && battle_config.hom_setting&HOMSET_NO_SUPPORT_SKILL && skill_get_inf(skill_id)&INF_SUPPORT_SKILL && battle_get_master(target) != src )
				return false; // Can't use support skills on Homunculus (only Master/Self)
			if( target->type == BL_MER && (skill_id == PR_ASPERSIO || (skill_id >= SA_FLAMELAUNCHER && skill_id <= SA_SEISMICWEAPON)) && battle_get_master(target) != src )
				return false; // Can't use Weapon endow skills on Mercenary (only Master)
			if( skill_id == AM_POTIONPITCHER && ( target->type == BL_MER || target->type == BL_ELEM) )
				return false; // Can't use Potion Pitcher on Mercenaries
			if (tsc && tsc->getSCE(SC_ELEMENTAL_VEIL) && !(src && status_get_class_(src) == CLASS_BOSS) && !status_has_mode(status, MD_DETECTOR))
				return false;
			[[fallthrough]];
		default:
			// Check for chase-walk/hiding/cloaking opponents.
			if( tsc ) {
				if( tsc->option&hide_flag && !status_has_mode(status,MD_DETECTOR))
					return false;
			}
	}
	return true;
}

/**
 * Checks whether the src can see the target
 * @param src:	Object using skill on target [PC|MOB|PET|HOM|MER|ELEM]
 * @param target: Object being targeted by src [PC|MOB|HOM|MER|ELEM]
 * @return src can see (1) or target is invisible (0)
 * @author [Skotlex]
 */
int status_check_visibility(struct block_list *src, struct block_list *target)
{
	int view_range;
	status_change* tsc = status_get_sc(target);
	switch (src->type) {
		case BL_MOB:
			view_range = ((TBL_MOB*)src)->min_chase;
			break;
		case BL_PET:
			view_range = ((TBL_PET*)src)->db->range2;
			break;
		default:
			view_range = AREA_SIZE;
	}

	if (src->m != target->m || !check_distance_bl(src, target, view_range))
		return 0;

	if ( src->type == BL_NPC) // NPCs don't care for the rest
		return 1;

	if (tsc) {
		bool is_boss = (status_get_class_(src) == CLASS_BOSS);
		bool is_detector = status_bl_has_mode(src,MD_DETECTOR);

		switch (target->type) {	// Check for chase-walk/hiding/cloaking opponents.
			case BL_PC: {
					map_session_data *tsd = (TBL_PC*)target;

					if (((tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD) || tsc->getSCE(SC_SUHIDE)) && !is_boss && (tsd->special_state.perfect_hiding || !is_detector))
						return 0;
					if ((tsc->getSCE(SC_CLOAKINGEXCEED) || tsc->getSCE(SC_NEWMOON)) && !is_boss && ((tsd && tsd->special_state.perfect_hiding) || is_detector))
						return 0;
					if (tsc->getSCE(SC__FEINTBOMB) && !is_boss && !is_detector)
						return 0;
				}
				break;
			case BL_ELEM:
				if (tsc->getSCE(SC_ELEMENTAL_VEIL) && !is_boss && !is_detector)
					return 0;
				break;
			default:
				if (((tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD) || tsc->getSCE(SC_SUHIDE)) && !is_boss && !is_detector)
					return 0;
		}
	}

	return 1;
}

/**
 * Base ASPD value taken from the job tables
 * @param sd: Player object
 * @param status: Player status
 * @return base amotion after single/dual weapon and shield adjustments [RENEWAL]
 *	  base amotion after single/dual weapon and stats adjustments [PRE-RENEWAL]
 */
int status_base_amotion_pc(map_session_data* sd, struct status_data* status)
{
	std::shared_ptr<s_job_info> job = job_db.find(sd->status.class_);

	if (job == nullptr)
		return 2000;

	int amotion;
#ifdef RENEWAL_ASPD
	int16 skill_lv, val = 0;
	float temp_aspd = 0;

	amotion = job->aspd_base[sd->weapontype1]; // Single weapon
	if (sd->status.shield)
		amotion += job->aspd_base[MAX_WEAPON_TYPE];
	else if (sd->weapontype2 != W_FIST && sd->equip_index[EQI_HAND_R] != sd->equip_index[EQI_HAND_L])
		amotion += job->aspd_base[sd->weapontype2] / 4; // Dual-wield

	switch(sd->status.weapon) {
		case W_BOW:
		case W_MUSICAL:
		case W_WHIP:
		case W_REVOLVER:
		case W_RIFLE:
		case W_GATLING:
		case W_SHOTGUN:
		case W_GRENADE:
			temp_aspd = status->dex * status->dex / 7.0f + status->agi * status->agi * 0.5f;
			break;
		default:
			temp_aspd = status->dex * status->dex / 5.0f + status->agi * status->agi * 0.5f;
			break;
	}
	temp_aspd = (float)(sqrt(temp_aspd) * 0.25f) + 0xc4;
	if ((skill_lv = pc_checkskill(sd,SA_ADVANCEDBOOK)) > 0 && sd->status.weapon == W_BOOK)
		val += (skill_lv - 1) / 2 + 1;
	if ((skill_lv = pc_checkskill(sd, SG_DEVIL)) > 0 && ((sd->class_&MAPID_THIRDMASK) == MAPID_STAR_EMPEROR || pc_is_maxjoblv(sd)))
		val += 1 + skill_lv;
	if ((skill_lv = pc_checkskill(sd,GS_SINGLEACTION)) > 0 && (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		val += ((skill_lv + 1) / 2);
	if (pc_isriding(sd))
		val -= 50 - 10 * pc_checkskill(sd, KN_CAVALIERMASTERY);
	else if (pc_isridingdragon(sd))
		val -= 25 - 5 * pc_checkskill(sd, RK_DRAGONTRAINING);
	amotion = ((int)(temp_aspd + ((float)(status_calc_aspd(&sd->bl, &sd->sc, true) + val) * status->agi / 200)) - min(amotion, 200));
#else
	// Angra Manyu disregards aspd_base and similar
	if (pc_checkequip2(sd, ITEMID_ANGRA_MANYU, EQI_ACC_L, EQI_MAX))
		return 0;

	// Base weapon delay
	amotion = (sd->status.weapon < MAX_WEAPON_TYPE)
	 ? (job->aspd_base[sd->status.weapon]) // Single weapon
	 : (job->aspd_base[sd->weapontype1] + job->aspd_base[sd->weapontype2]) * 7 / 10; // Dual-wield

	// Percentual delay reduction from stats
	amotion -= amotion * (4 * status->agi + status->dex) / 1000;

	// Raw delay adjustment from bAspd bonus
	amotion += sd->bonus.aspd_add;
#endif

 	return amotion;
}

/**
 * Base attack value calculated for units
 * @param bl: Object to get attack for [BL_CHAR|BL_NPC]
 * @param status: Object status
 * @return base attack
 */
unsigned short status_base_atk(const struct block_list *bl, const struct status_data *status, int level)
{
	int flag = 0, str, dex, dstr;

#ifdef RENEWAL
	if (!(bl->type&battle_config.enable_baseatk_renewal))
		return 0;
#else
	if (!(bl->type&battle_config.enable_baseatk))
		return 0;
#endif

	if (bl->type == BL_PC)
	switch(((TBL_PC*)bl)->status.weapon) {
		case W_BOW:
		case W_MUSICAL:
		case W_WHIP:
		case W_REVOLVER:
		case W_RIFLE:
		case W_GATLING:
		case W_SHOTGUN:
		case W_GRENADE:
			flag = 1;
	}
	if (flag) {
#ifdef RENEWAL
		dstr =
#endif
		str = status->dex;
		dex = status->str;
	} else {
#ifdef RENEWAL
		dstr =
#endif
		str = status->str;
		dex = status->dex;
	}

	/** [Skotlex]
	* Normally only players have base-atk, but homunc have a different batk
	* equation, hinting that perhaps non-players should use this for batk.
	**/
	switch (bl->type) {
		case BL_HOM:
#ifdef RENEWAL
			str = 2 * level + status_get_homstr(bl);
#else
			dstr = str / 10;
			str += dstr*dstr;
#endif
			break;
		case BL_PC:
#ifdef RENEWAL
			str = (dstr * 10 + dex * 10 / 5 + status->luk * 10 / 3 + level * 10 / 4) / 10 + 5 * status->pow;
#else
			dstr = str / 10;
			str += dstr*dstr;
			str += dex / 5 + status->luk / 5;
#endif
			break;
		default:// Others
#ifdef RENEWAL
			str = dstr + level;
#else
			dstr = str / 10;
			str += dstr*dstr;
			str += dex / 5 + status->luk / 5;
#endif
			break;
	}

	return cap_value(str, 0, USHRT_MAX);
}

#ifdef RENEWAL
/**
 * Weapon attack value calculated for Players
 * @param wa: Weapon attack
 * @param status: Player status
 * @return weapon attack
 */
unsigned int status_weapon_atk(weapon_atk &wa)
{
	return wa.atk + wa.atk2;
}
#endif

#ifndef RENEWAL
unsigned short status_base_matk_min(const struct status_data* status) { return status->int_ + (status->int_ / 7) * (status->int_ / 7); }
unsigned short status_base_matk_max(const struct status_data* status) { return status->int_ + (status->int_ / 5) * (status->int_ / 5); }
#else
/*
* Calculates minimum attack variance 80% from db's ATK1 for non BL_PC
* status->batk (base attack) will be added in battle_calc_base_damage
*/
unsigned short status_base_atk_min(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->rhw.atk * 80 / 100;
		case BL_HOM:
			return (status_get_homstr(bl) + status_get_homdex(bl)) / 5;
		default:
			return status->rhw.atk;
	}
}

/*
* Calculates maximum attack variance 120% from db's ATK1 for non BL_PC
* status->batk (base attack) will be added in battle_calc_base_damage
*/
unsigned short status_base_atk_max(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->rhw.atk * 120 / 100;
		case BL_HOM:
			return (status_get_homluk(bl) + status_get_homstr(bl) + status_get_homdex(bl)) / 3;
		default:
			return status->rhw.atk2;
	}
}

/*
* Calculates minimum magic attack
*/
unsigned short status_base_matk_min(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->int_ + level + status->rhw.matk * 70 / 100;
		case BL_HOM:
			return status_get_homint(bl) + level + (status_get_homint(bl) + status_get_homdex(bl)) / 5;
		case BL_PC:
		default:
			return status->int_ + (status->int_ / 2) + (status->dex / 5) + (status->luk / 3) + (level / 4) + 5 * status->spl;
	}
}

/*
* Calculates maximum magic attack
*/
unsigned short status_base_matk_max(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->int_ + level + status->rhw.matk * 130 / 100;
		case BL_HOM:
			return status_get_homint(bl) + level + (status_get_homluk(bl) + status_get_homint(bl) + status_get_homdex(bl)) / 3;
		case BL_PC:
		default:
			return status->int_ + (status->int_ / 2) + (status->dex / 5) + (status->luk / 3) + (level / 4) + 5 * status->spl;
	}
}
#endif

/**
 * Fills in the misc data that can be calculated from the other status info (except for level)
 * @param bl: Object to calculate status on [PC|MOB|PET|HOM|MERC|ELEM]
 * @param status: Player status
 */
void status_calc_misc(struct block_list *bl, struct status_data *status, int level)
{
	int stat;

	// Non players get the value set, players need to stack with previous bonuses.
	if( bl->type != BL_PC ){
		status->batk =
		status->hit = status->flee =
		status->def2 = status->mdef2 =
		status->cri = status->flee2 =
		status->patk = status->smatk =
		status->hplus = status->crate = 0;
		
		if (bl->type != BL_MOB)	// BL_MOB has values set when loading mob_db
			status->res = status->mres = 0;
	}

#ifdef RENEWAL // Renewal formulas
	if (bl->type == BL_HOM) {
		// Def2
		stat = status_get_homvit(bl) + status_get_homagi(bl) / 2;
		status->def2 = cap_value(stat, 0, SHRT_MAX);
		// Mdef2
		stat = (status_get_homvit(bl) + status_get_homint(bl)) / 2;
		status->mdef2 = cap_value(stat, 0, SHRT_MAX);
		// Def
		stat = status->def;
		stat += status_get_homvit(bl) + level / 2;
		status->def = cap_value(stat, 0, SHRT_MAX);
		// Mdef
		stat = (int)(((float)status_get_homvit(bl) + level) / 4 + (float)status_get_homint(bl) / 2);
		status->mdef = cap_value(stat, 0, SHRT_MAX);
		// Hit
		stat = level + status->dex + 150;
		status->hit = cap_value(stat, 1, SHRT_MAX);
		// Flee
		stat = level + status_get_homagi(bl);
		status->flee = cap_value(stat, 1, SHRT_MAX);
	} else {
		// Hit
		stat = status->hit;
		stat += level + status->dex + (bl->type == BL_PC ? status->luk / 3 + 175 : 150); //base level + ( every 1 dex = +1 hit ) + (every 3 luk = +1 hit) + 175
		stat += 2 * status->con;
		status->hit = cap_value(stat, 1, SHRT_MAX);
		// Flee
		stat = status->flee;
		stat += level + status->agi + (bl->type == BL_MER ? 0 : bl->type == BL_PC ? status->luk / 5 : 0) + 100; //base level + ( every 1 agi = +1 flee ) + (every 5 luk = +1 flee) + 100
		stat += 2 * status->con;
		status->flee = cap_value(stat, 1, SHRT_MAX);
		// Def2
		if (bl->type == BL_MER)
			stat = (int)(status->vit + ((float)level / 10) + ((float)status->vit / 5));
		else {
			stat = status->def2;
			stat += (int)(((float)level + status->vit) / 2 + (bl->type == BL_PC ? ((float)status->agi / 5) : 0)); //base level + (every 2 vit = +1 def) + (every 5 agi = +1 def)
		}
		status->def2 = cap_value(stat, 0, SHRT_MAX);
		// Mdef2
		if (bl->type == BL_MER)
			stat = (int)(((float)level / 10) + ((float)status->int_ / 5));
		else {
			stat = status->mdef2;
			stat += (int)(bl->type == BL_PC ? (status->int_ + ((float)level / 4) + ((float)(status->dex + status->vit) / 5)) : ((float)(status->int_ + level) / 4)); //(every 4 base level = +1 mdef) + (every 1 int = +1 mdef) + (every 5 dex = +1 mdef) + (every 5 vit = +1 mdef)
		}
		status->mdef2 = cap_value(stat, 0, SHRT_MAX);
		// PAtk
		stat = status->patk;
		stat += status->pow / 3 + status->con / 5;
		status->patk = cap_value(stat, 0, SHRT_MAX);
		// SMatk
		stat = status->smatk;
		stat += status->spl / 3 + status->con / 5;
		status->smatk = cap_value(stat, 0, SHRT_MAX);
		// Res
		stat = status->res;
		stat += status->sta + status->sta / 3 * 5;
		status->res = cap_value(stat, 0, SHRT_MAX);
		// Mres
		stat = status->mres;
		stat += status->wis + status->wis / 3 * 5;
		status->mres = cap_value(stat, 0, SHRT_MAX);
		// HPlus
		stat = status->hplus;
		stat += status->crt;
		status->hplus = cap_value(stat, 0, SHRT_MAX);
		// CRate
		stat = status->crate;
		stat += status->crt / 3;
		status->crate = cap_value(stat, 0, SHRT_MAX);
	}

	// ATK
	if (bl->type != BL_PC) {
		status->rhw.atk2 = status_base_atk_max(bl, status, level);
		status->rhw.atk = status_base_atk_min(bl, status, level);
	}

	// MAtk
	status->matk_min = status_base_matk_min(bl, status, level);
	status->matk_max = status_base_matk_max(bl, status, level);
#else
	// Matk
	status->matk_min = status_base_matk_min(status);
	status->matk_max = status_base_matk_max(status);
	// Hit
	stat = status->hit;
	stat += level + status->dex;
	status->hit = cap_value(stat, 1, SHRT_MAX);
	// Flee
	stat = status->flee;
	stat += level + status->agi;
	status->flee = cap_value(stat, 1, SHRT_MAX);
	// Def2
	stat = status->def2;
	stat += status->vit;
	status->def2 = cap_value(stat, 0, SHRT_MAX);
	// Mdef2
	stat = status->mdef2;
	stat += status->int_ + (status->vit / 2);
	status->mdef2 = cap_value(stat, 0, SHRT_MAX);
#endif

	//Critical
	if( bl->type&battle_config.enable_critical ) {
		stat = status->cri;
#ifdef RENEWAL
		stat += (level / 10); // (every 10 BaseLevel = +0.1 critical)
		stat += 10 + (status->luk*3); // (every 1 luk = +0.3 critical)
#else
		stat += 10 + (status->luk*10/3); // (every 1 luk = +0.3 critical)
#endif
		status->cri = cap_value(stat, 1, SHRT_MAX);
	} else
		status->cri = 0;

	if (bl->type&battle_config.enable_perfect_flee) {
		stat = status->flee2;
		stat += status->luk + 10; // (every 10 luk = +1 perfect flee)
		status->flee2 = cap_value(stat, 0, SHRT_MAX);
	} else
		status->flee2 = 0;

	if (status->batk) {
		int temp = status->batk + status_base_atk(bl, status, level);
		status->batk = cap_value(temp, 0, USHRT_MAX);
	} else
		status->batk = status_base_atk(bl, status, level);

	if (status->cri) {
		switch (bl->type) {
			case BL_MOB:
				if(battle_config.mob_critical_rate != 100)
					status->cri = cap_value(status->cri*battle_config.mob_critical_rate/100,1,SHRT_MAX);
				if(!status->cri && battle_config.mob_critical_rate)
					status->cri = 10;
				break;
			case BL_PC:
				// Players don't have a critical adjustment setting as of yet.
				break;
			default:
				if(battle_config.critical_rate != 100)
					status->cri = cap_value(status->cri*battle_config.critical_rate/100,1,SHRT_MAX);
				if (!status->cri && battle_config.critical_rate)
					status->cri = 10;
		}
	}

	if(bl->type&BL_REGEN)
		status_calc_regen(bl, status, status_get_regen_data(bl));
}

/**
 * Calculates the initial status for the given mob
 * @param md: Mob object
 * @param opt: Whether or not it is the first calculation
		This will only be false when a mob levels up (Regular and WoE Guardians)
 * @return 1 for calculated special statuses or 0 for none
 * @author [Skotlex]
 */
int status_calc_mob_(struct mob_data* md, uint8 opt)
{
	struct status_data *status;
	struct block_list *mbl = nullptr;
	int flag=0;

	if (opt&SCO_FIRST) { // Set basic level on respawn.
		if (md->level > 0 && md->level <= MAX_LEVEL && md->level != md->db->lv)
			;
		else
			md->level = md->db->lv;
		md->damagetaken = md->db->damagetaken;
	}

	// Check if we need custom base-status
	if (battle_config.mobs_level_up && md->level > md->db->lv)
		flag|=1;

	if (md->special_state.size)
		flag|=2;

	if (md->guardian_data && md->guardian_data->guardup_lv)
		flag|=4;
	if (md->mob_id == MOBID_EMPERIUM)
		flag|=4;

	if (battle_config.slaves_inherit_speed && md->master_id)
		flag|=8;

	if (md->master_id && md->special_state.ai>AI_ATTACK)
		flag|=16;

	if (md->master_id && battle_config.slaves_inherit_mode)
		flag |= 32;

	if (!flag) { // No special status required.
		if (md->base_status) {
			aFree(md->base_status);
			md->base_status = nullptr;
		}
		if (opt&SCO_FIRST)
			memcpy(&md->status, &md->db->status, sizeof(struct status_data));
		return 0;
	}
	if (!md->base_status)
		md->base_status = (struct status_data*)aCalloc(1, sizeof(struct status_data));

	status = md->base_status;
	memcpy(status, &md->db->status, sizeof(struct status_data));

	if (flag&(8|16))
		mbl = map_id2bl(md->master_id);

	if (flag&8 && mbl) {
		struct status_data *mstatus = status_get_base_status(mbl);

		if (mstatus &&
			battle_config.slaves_inherit_speed&(status_has_mode(mstatus,MD_CANMOVE)?1:2))
			status->speed = mstatus->speed;
		if( status->speed < 2 ) // Minimum for the unit to function properly
			status->speed = 2;
	}

	if (flag&32)
		status_calc_slave_mode(*md);

	if (flag&1) { // Increase from mobs leveling up [Valaris]
		int diff = md->level - md->db->lv;

		status->str += diff;
		status->agi += diff;
		status->vit += diff;
		status->int_ += diff;
		status->dex += diff;
		status->luk += diff;
		status->max_hp += diff * status->vit;
		status->max_sp += diff * status->int_;
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		status->speed -= cap_value(diff, 0, status->speed - 10);
	}

	if (flag&2 && battle_config.mob_size_influence) { // Change for sized monsters [Valaris]
		if (md->special_state.size == SZ_MEDIUM) {
			status->max_hp /= 2;
			status->max_sp /= 2;
			if (!status->max_hp) status->max_hp = 1;
			if (!status->max_sp) status->max_sp = 1;
			status->hp = status->max_hp;
			status->sp = status->max_sp;
			status->str /= 2;
			status->agi /= 2;
			status->vit /= 2;
			status->int_ /= 2;
			status->dex /= 2;
			status->luk /= 2;
			if (!status->str) status->str = 1;
			if (!status->agi) status->agi = 1;
			if (!status->vit) status->vit = 1;
			if (!status->int_) status->int_ = 1;
			if (!status->dex) status->dex = 1;
			if (!status->luk) status->luk = 1;
		} else if (md->special_state.size == SZ_BIG) {
			status->max_hp *= 2;
			status->max_sp *= 2;
			status->hp = status->max_hp;
			status->sp = status->max_sp;
			status->str *= 2;
			status->agi *= 2;
			status->vit *= 2;
			status->int_ *= 2;
			status->dex *= 2;
			status->luk *= 2;
		}
	}

	status_calc_misc(&md->bl, status, md->level);

	if(flag&4) { // Strengthen Guardians - custom value +10% / lv
		struct map_data *mapdata = map_getmapdata(md->bl.m);
		std::shared_ptr<guild_castle> gc = castle_db.mapname2gc(mapdata->name);

		if (gc == nullptr)
			ShowError("status_calc_mob: No castle set at map %s\n", mapdata->name);
		else if(gc->castle_id < 24 || md->mob_id == MOBID_EMPERIUM) {
#ifdef RENEWAL
			status->max_hp += 50 * (gc->defense / 5);
#else
			status->max_hp += 1000 * gc->defense;
#endif
			status->hp = status->max_hp;
			status->def += (gc->defense+2)/3;
			status->mdef += (gc->defense+2)/3;
		}
		if(md->mob_id != MOBID_EMPERIUM) {
			status->max_hp += 1000 * gc->defense;
			status->hp = status->max_hp;
			status->batk += 2 * md->guardian_data->guardup_lv + 8;
			status->rhw.atk += 2 * md->guardian_data->guardup_lv + 8;
			status->rhw.atk2 += 2 * md->guardian_data->guardup_lv + 8;
			status->aspd_rate -= 2 * md->guardian_data->guardup_lv + 3;
		}
	}

	if (flag&16 && mbl) { // Max HP setting from Summon Flora/marine Sphere
		struct unit_data *ud = unit_bl2ud(mbl);
		// Remove special AI when this is used by regular mobs.
		if (mbl->type == BL_MOB && !((TBL_MOB*)mbl)->special_state.ai)
			md->special_state.ai = AI_NONE;
		if (ud) { 
			// Different levels of HP according to skill level
			if(!ud->skill_id) // !FIXME: We lost the unit data for magic decoy in somewhere before this
				ud->skill_id = ((TBL_PC*)mbl)->menuskill_id;
			switch(ud->skill_id) {
				case AM_SPHEREMINE:
					status->max_hp = 2000 + 400*ud->skill_lv;
					break;
				case KO_ZANZOU:
					status->max_hp = 3000 + 3000 * ud->skill_lv;
					break;
				case AM_CANNIBALIZE:
					status->max_hp = 1500 + 200*ud->skill_lv + 10*status_get_lv(mbl);
					status->mode = static_cast<e_mode>(status->mode|MD_CANATTACK|MD_AGGRESSIVE);
					break;
				case MH_SUMMON_LEGION:
				{
					int homblvl = status_get_lv(mbl);

					status->max_hp = 10 * (100 * (ud->skill_lv + 2) + homblvl);
					status->batk = 100 * (ud->skill_lv+5) / 2;
					status->def = 10 * (100 * (ud->skill_lv+2) + homblvl);
					// status->aspd_rate = 10 * (2 * (20 - ud->skill_lv) - homblvl/10);
					// status->aspd_rate = max(100,status->aspd_rate);
					break;
				}
				case NC_SILVERSNIPER:
				{
					status_data* mstatus = status_get_status_data(*mbl);

					// TODO: check if dummy_status? Can never be nullptr [Lemongrass]
					if(!mstatus)
						break;

					status->max_hp = (1000 * ud->skill_lv) + (mstatus->hp / 3) + (status_get_lv(mbl) * 12);
					status->batk = 200 * ud->skill_lv;
					break;
				}
				case NC_MAGICDECOY:
				{
					status_data* mstatus = status_get_status_data(*mbl);

					// TODO: check if dummy_status? Can never be nullptr [Lemongrass]
					if(!mstatus)
						break;

					status->max_hp = (1000 * ((TBL_PC*)mbl)->menuskill_val) + (mstatus->sp * 4) + (status_get_lv(mbl) * 12);
					status->matk_min = status->matk_max = 250 + 50*((TBL_PC*)mbl)->menuskill_val;
					break;
				}
				case MT_SUMMON_ABR_BATTLE_WARIOR:
				case MT_SUMMON_ABR_DUAL_CANNON:
				case MT_SUMMON_ABR_MOTHER_NET:
				case MT_SUMMON_ABR_INFINITY: {
						map_session_data *msd = BL_CAST(BL_PC, mbl);
						status_data* mstatus = status_get_status_data(*mbl);

						// TODO: check if mstatus is dummy_status? Can never be nullptr [Lemongrass]
						if (msd == nullptr || mstatus == nullptr)
							break;

						uint8 abr_mastery = pc_checkskill(msd, MT_ABR_M);

						// Custom formulas for ABR's.
						// Its unknown how the summoner's stats affects the ABR's stats.
						// I decided to do something similar to elementals for now until I know.
						// Also added hit increase from ABR-Mastery for balance reasons. [Rytech]
						status->max_hp = ( 5000 + 40000 * abr_mastery ) * mstatus->vit / 100;
						status->rhw.atk = ( 2 * mstatus->batk + 200 + 600 * abr_mastery ) * 70 / 100;
						status->rhw.atk2 = 2 * mstatus->batk + 200 + 600 * abr_mastery;
						status->def = mstatus->def + 20 * abr_mastery;
						status->mdef = mstatus->mdef + 4 * abr_mastery;
						status->hit = mstatus->hit + 5 * abr_mastery / 2;
						status->flee = mstatus->flee + 10 * abr_mastery;
						status->speed = mstatus->speed;

						// The Infinity ABR appears to have a much higher attack then other
						// ABR's and im guessing has a much higher MaxHP due to it being a AP
						// costing summon. [Rytech]
						if (ud->skill_id == MT_SUMMON_ABR_INFINITY) {
							status->max_hp += 20000;
							status->rhw.atk += 1400; // 70% of 2000
							status->rhw.atk2 += 2000;
						}
					}
					break;
				case BO_WOODENWARRIOR:
				case BO_WOODEN_FAIRY:
				case BO_CREEPER:
				case BO_HELLTREE: {
						map_session_data *msd = BL_CAST(BL_PC, mbl);
						status_data* mstatus = status_get_status_data(*mbl);

						// TODO: check if mstatus is dummy_status? Can never be nullptr [Lemongrass]
						if (msd == nullptr || mstatus == nullptr)
							break;

						uint8 bionic_mastery = pc_checkskill(msd, BO_BIONICS_M);

						// Custom formulas for bionic's.
						// Its unknown how the summoner's stats affects the bionic's stats.
						// I decided to do something similar to elementals for now until I know.
						// Also added hit increase from Bionic-Mastery for balance reasons. [Rytech]
						status->max_hp = (5000 + 40000 * bionic_mastery) * mstatus->vit / 100;
						//status->max_sp = (50 + 20 * bionic_mastery) * mstatus->int_ / 100;// Wait what??? Bionic Mastery increases MaxSP? They have SP???
						status->rhw.atk = (2 * mstatus->batk + 600 * bionic_mastery) * 70 / 100;
						status->rhw.atk2 = 2 * mstatus->batk + 600 * bionic_mastery;
						status->def = mstatus->def + 20 * bionic_mastery;
						status->mdef = mstatus->mdef + 4 * bionic_mastery;
						status->hit = mstatus->hit + 5 * bionic_mastery / 2;
						status->flee = mstatus->flee + 10 * bionic_mastery;
						status->speed = mstatus->speed;

						// The Hell Tree bionic appears to have a much higher attack then other
						// bionic's and im guessing has a much higher MaxHP due to it being a AP
						// costing summon. [Rytech]
						if (ud->skill_id == BO_HELLTREE) {
							status->max_hp += 20000;
							status->rhw.atk += 1400; // 70% of 2000
							status->rhw.atk2 += 2000;
						}
					}
					break;
			}
			status->hp = status->max_hp;
		}
	}

	if (opt&SCO_FIRST) // Initial battle status
		memcpy(&md->status, status, sizeof(struct status_data));

	return 1;
}

/**
 * Calculates the stats of the given pet
 * @param pd: Pet object
 * @param opt: Whether or not it is the first calculation
		This will only be false when a pet levels up
 * @return 1
 * @author [Skotlex]
 */
void status_calc_pet_(struct pet_data *pd, uint8 opt)
{
	nullpo_retv(pd);

	if (opt&SCO_FIRST) {
		memcpy(&pd->status, &pd->db->status, sizeof(struct status_data));
		pd->status.mode = MD_CANMOVE; // Pets discard all modes, except walking
		pd->status.class_ = CLASS_NORMAL;
		pd->status.speed = pd->get_pet_walk_speed();

		if(battle_config.pet_attack_support || battle_config.pet_damage_support) {
			// Attack support requires the pet to be able to attack
			pd->status.mode = static_cast<e_mode>(pd->status.mode|MD_CANATTACK);
		}
	}

	if (battle_config.pet_lv_rate && pd->master) {
		map_session_data *sd = pd->master;
		int lv;

		lv =sd->status.base_level*battle_config.pet_lv_rate/100;
		if (lv < 0)
			lv = 1;
		if (lv != pd->pet.level || opt&SCO_FIRST) {
			struct status_data *bstat = &pd->db->status, *status = &pd->status;

			pd->pet.level = lv;
			if (!(opt&SCO_FIRST)) // Lv Up animation
				clif_misceffect( pd->bl, NOTIFYEFFECT_BASE_LEVEL_UP );
			status->rhw.atk = (bstat->rhw.atk*lv)/pd->db->lv;
			status->rhw.atk2 = (bstat->rhw.atk2*lv)/pd->db->lv;
			status->str = (bstat->str*lv)/pd->db->lv;
			status->agi = (bstat->agi*lv)/pd->db->lv;
			status->vit = (bstat->vit*lv)/pd->db->lv;
			status->int_ = (bstat->int_*lv)/pd->db->lv;
			status->dex = (bstat->dex*lv)/pd->db->lv;
			status->luk = (bstat->luk*lv)/pd->db->lv;

			status->rhw.atk = cap_value(status->rhw.atk, 1, battle_config.pet_max_atk1);
			status->rhw.atk2 = cap_value(status->rhw.atk2, 2, battle_config.pet_max_atk2);
			status->str = cap_value(status->str,1,battle_config.pet_max_stats);
			status->agi = cap_value(status->agi,1,battle_config.pet_max_stats);
			status->vit = cap_value(status->vit,1,battle_config.pet_max_stats);
			status->int_= cap_value(status->int_,1,battle_config.pet_max_stats);
			status->dex = cap_value(status->dex,1,battle_config.pet_max_stats);
			status->luk = cap_value(status->luk,1,battle_config.pet_max_stats);

			status_calc_misc(&pd->bl, &pd->status, lv);

			if (!(opt&SCO_FIRST)) // Not done the first time because the pet is not visible yet
				clif_send_petstatus(sd);
		}
	} else if (opt&SCO_FIRST) {
		status_calc_misc(&pd->bl, &pd->status, pd->db->lv);
		if (!battle_config.pet_lv_rate && pd->pet.level != pd->db->lv)
			pd->pet.level = pd->db->lv;
	}

	// Support rate modifier (1000 = 100%)
	pd->rate_fix = min(1000 * (pd->pet.intimate - battle_config.pet_support_min_friendly) / (1000 - battle_config.pet_support_min_friendly) + 500, USHRT_MAX);
	pd->rate_fix = min(apply_rate(pd->rate_fix, battle_config.pet_support_rate), USHRT_MAX);
}

/**
 * Get HP bonus modifiers
 * @param bl: block_list that will be checked
 * @param type: type of e_status_bonus (STATUS_BONUS_FIX or STATUS_BONUS_RATE)
 * @return bonus: total bonus for HP
 * @author [Cydh]
 */
static int status_get_hpbonus(struct block_list *bl, enum e_status_bonus type) {
	int bonus = 0;

	if (type == STATUS_BONUS_FIX) {
		status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			map_session_data *sd = map_id2sd(bl->id);
			uint16 skill_lv;

			bonus += sd->bonus.hp;
			if ((skill_lv = pc_checkskill(sd,CR_TRUST)) > 0)
				bonus += skill_lv * 200;
			if (pc_checkskill(sd,SU_SPRITEMABLE) > 0)
				bonus += 1000;
			if (pc_checkskill(sd, SU_POWEROFSEA) > 0) {
				bonus += 1000;
				if (pc_checkskill_summoner(sd, SUMMONER_POWER_SEA) >= 20)
					bonus += 3000;
			}
			if ((skill_lv = pc_checkskill(sd, NV_BREAKTHROUGH)) > 0)
				bonus += 350 * skill_lv + (skill_lv > 4 ? 250 : 0);
			if ((skill_lv = pc_checkskill(sd, NV_TRANSCENDENCE)) > 0)
				bonus += 350 * skill_lv + (skill_lv > 4 ? 250 : 0);
#ifndef HP_SP_TABLES
			if ((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 99)
				bonus += 2000; // Supernovice lvl99 hp bonus.
			if ((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 150)
				bonus += 2000; // Supernovice lvl150 hp bonus.
#endif
		}

		//Bonus by SC
		if (sc) {
			if(sc->getSCE(SC_INCMHP))
				bonus += sc->getSCE(SC_INCMHP)->val1;
			if(sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 2)
				bonus += 500;
			if (sc->getSCE(SC_PROMOTE_HEALTH_RESERCH))
				bonus += sc->getSCE(SC_PROMOTE_HEALTH_RESERCH)->val3;
			if(sc->getSCE(SC_SOLID_SKIN_OPTION))
				bonus += 2000;
			if(sc->getSCE(SC_MARIONETTE))
				bonus -= 1000;
			if(sc->getSCE(SC_SWORDCLAN))
				bonus += 30;
			if(sc->getSCE(SC_ARCWANDCLAN))
				bonus += 30;
			if(sc->getSCE(SC_GOLDENMACECLAN))
				bonus += 30;
			if(sc->getSCE(SC_CROSSBOWCLAN))
				bonus += 30;
			if(sc->getSCE(SC_GLASTHEIM_HPSP))
				bonus += sc->getSCE(SC_GLASTHEIM_HPSP)->val1;
#ifdef RENEWAL
			if (sc->getSCE(SC_ANGELUS))
				bonus += sc->getSCE(SC_ANGELUS)->val1 * 50;
#endif
		}
	} else if (type == STATUS_BONUS_RATE) {
		status_change *sc = status_get_sc(bl);

		//Bonus by SC
		if (sc) {
			//Increasing
			if(sc->getSCE(SC_INCMHPRATE))
				bonus += sc->getSCE(SC_INCMHPRATE)->val1;
			if(sc->getSCE(SC_APPLEIDUN))
				bonus += sc->getSCE(SC_APPLEIDUN)->val2;
			if(sc->getSCE(SC_DELUGE))
				bonus += sc->getSCE(SC_DELUGE)->val2;
			if(sc->getSCE(SC_BERSERK))
				bonus += 200; //+200%
			if(sc->getSCE(SC_MERC_HPUP))
				bonus += sc->getSCE(SC_MERC_HPUP)->val2;
			if(sc->getSCE(SC_EPICLESIS))
				bonus += sc->getSCE(SC_EPICLESIS)->val2;
			if(sc->getSCE(SC_FRIGG_SONG))
				bonus += sc->getSCE(SC_FRIGG_SONG)->val2;
			if(sc->getSCE(SC_LERADSDEW))
				bonus += sc->getSCE(SC_LERADSDEW)->val3;
			if(sc->getSCE(SC_FORCEOFVANGUARD))
				bonus += (3 * sc->getSCE(SC_FORCEOFVANGUARD)->val1);
			if(sc->getSCE(SC_INSPIRATION))
				bonus += (4 * sc->getSCE(SC_INSPIRATION)->val1);
			if(sc->getSCE(SC_RAISINGDRAGON))
				bonus += sc->getSCE(SC_RAISINGDRAGON)->val1;
			if(sc->getSCE(SC_GT_REVITALIZE))
				bonus += sc->getSCE(SC_GT_REVITALIZE)->val2;
			if(sc->getSCE(SC_ANGRIFFS_MODUS))
				bonus += (5 * sc->getSCE(SC_ANGRIFFS_MODUS)->val1);
			if(sc->getSCE(SC_PETROLOGY_OPTION))
				bonus += sc->getSCE(SC_PETROLOGY_OPTION)->val2;
			if(sc->getSCE(SC_POWER_OF_GAIA))
				bonus += sc->getSCE(SC_POWER_OF_GAIA)->val3;
			if(sc->getSCE(SC_CURSED_SOIL_OPTION))
				bonus += sc->getSCE(SC_CURSED_SOIL_OPTION)->val2;
			if(sc->getSCE(SC_UPHEAVAL_OPTION))
				bonus += sc->getSCE(SC_UPHEAVAL_OPTION)->val2;
			if(sc->getSCE(SC_LAUDAAGNUS))
				bonus += 2 + (sc->getSCE(SC_LAUDAAGNUS)->val1 * 2);
#ifdef RENEWAL
			if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_HPRATE)
				bonus += 30;
#endif
			if(sc->getSCE(SC_LUNARSTANCE))
				bonus += sc->getSCE(SC_LUNARSTANCE)->val2;
			if (sc->getSCE(SC_LUXANIMA))
				bonus += sc->getSCE(SC_LUXANIMA)->val3;
			if (sc->getSCE(SC_MTF_MHP))
				bonus += sc->getSCE(SC_MTF_MHP)->val1;
			if (sc->getSCE(SC_FIRM_FAITH))
				bonus += sc->getSCE(SC_FIRM_FAITH)->val2;

			//Decreasing
			if (sc->getSCE(SC_VENOMBLEED) && sc->getSCE(SC_VENOMBLEED)->val3 == 1)
				bonus -= 15;
			if(sc->getSCE(SC_BEYONDOFWARCRY))
				bonus -= sc->getSCE(SC_BEYONDOFWARCRY)->val3;
			if(sc->getSCE(SC__WEAKNESS))
				bonus -= sc->getSCE(SC__WEAKNESS)->val2;
			if(sc->getSCE(SC_EQC))
				bonus -= sc->getSCE(SC_EQC)->val3;
			if(sc->getSCE(SC_PACKING_ENVELOPE3))
				bonus += sc->getSCE(SC_PACKING_ENVELOPE3)->val1;
			if(sc->getSCE(SC_2011RWC_SCROLL))
				bonus -= 10;
			if(sc->getSCE(SC_INFINITY_DRINK))
				bonus += 5;
			if(sc->getSCE(SC_COMBAT_PILL))
				bonus -= 3;
			if(sc->getSCE(SC_COMBAT_PILL2))
				bonus -= 5;
		}
		// Max rate reduce is -100%
		bonus = cap_value(bonus,-100,INT_MAX);
	}

	return min(bonus,INT_MAX);
}

/**
* HP bonus rate from equipment
*/
static int status_get_hpbonus_equip(TBL_PC *sd) {
	int bonus = 0;

	bonus += sd->hprate;

	return bonus -= 100; //Default hprate is 100, so it should be add 0%
}

/**
* HP bonus rate from usable items
*/
static int status_get_hpbonus_item(block_list *bl) {
	int bonus = 0;

	status_change *sc = status_get_sc(bl);

	//Bonus by SC
	if (sc) {
		if (sc->getSCE(SC_INCREASE_MAXHP))
			bonus += sc->getSCE(SC_INCREASE_MAXHP)->val1;
		if (sc->getSCE(SC_MUSTLE_M))
			bonus += sc->getSCE(SC_MUSTLE_M)->val1;

		if (sc->getSCE(SC_MYSTERIOUS_POWDER))
			bonus -= sc->getSCE(SC_MYSTERIOUS_POWDER)->val1;
	}

	// Max rate reduce is -100%
	return cap_value(bonus, -100, INT_MAX);
}

/**
 * Get SP bonus modifiers
 * @param bl: block_list that will be checked
 * @param type: type of e_status_bonus (STATUS_BONUS_FIX or STATUS_BONUS_RATE)
 * @return bonus: total bonus for SP
 * @author [Cydh]
 */
static int status_get_spbonus(struct block_list *bl, enum e_status_bonus type) {
	int bonus = 0;

	if (type == STATUS_BONUS_FIX) {
		status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			map_session_data *sd = map_id2sd(bl->id);
			uint16 skill_lv;

			bonus += sd->bonus.sp;
			if ((skill_lv = pc_checkskill(sd,SL_KAINA)) > 0)
				bonus += 30 * skill_lv;
			if ((skill_lv = pc_checkskill(sd,RA_RESEARCHTRAP)) > 0)
				bonus += 200 + 20 * skill_lv;
			if ((skill_lv = pc_checkskill(sd,WM_LESSON)) > 0)
				bonus += 30 * skill_lv;
			if (pc_checkskill(sd,SU_SPRITEMABLE) > 0)
				bonus += 100;
			if (pc_checkskill(sd, SU_POWEROFSEA) > 0) {
				bonus += 100;
				if (pc_checkskill_summoner(sd, SUMMONER_POWER_SEA) >= 20)
					bonus += 300;
			}
			if ((skill_lv = pc_checkskill(sd, NV_BREAKTHROUGH)) > 0)
				bonus += 30 * skill_lv + (skill_lv > 4 ? 50 : 0);
			if ((skill_lv = pc_checkskill(sd, NV_TRANSCENDENCE)) > 0)
				bonus += 30 * skill_lv + (skill_lv > 4 ? 50 : 0);
		}

		//Bonus by SC
		if (sc) {
			if(sc->getSCE(SC_INCMSP))
				bonus += sc->getSCE(SC_INCMSP)->val1;
			if(sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 3)
				bonus += 50;
			if(sc->getSCE(SC_SWORDCLAN))
				bonus += 10;
			if(sc->getSCE(SC_ARCWANDCLAN))
				bonus += 10;
			if(sc->getSCE(SC_GOLDENMACECLAN))
				bonus += 10;
			if(sc->getSCE(SC_CROSSBOWCLAN))
				bonus += 10;
			if(sc->getSCE(SC_GLASTHEIM_HPSP))
				bonus += sc->getSCE(SC_GLASTHEIM_HPSP)->val2;
		}
	} else if (type == STATUS_BONUS_RATE) {
		status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			map_session_data *sd = map_id2sd(bl->id);
			uint8 i;

			if((i = pc_checkskill(sd,HP_MEDITATIO)) > 0)
				bonus += i;
			if((i = pc_checkskill(sd,HW_SOULDRAIN)) > 0)
				bonus += 2 * i;
#ifdef RENEWAL
			if ((i = pc_checkskill(sd, (sd->status.sex ? BA_MUSICALLESSON : DC_DANCINGLESSON))) > 0)
				bonus += i;
			if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_SPRATE)
				bonus += 30;
#endif
		}

		//Bonus by SC
		if (sc) {
			//Increasing
			if(sc->getSCE(SC_INCMSPRATE))
				bonus += sc->getSCE(SC_INCMSPRATE)->val1;
			if(sc->getSCE(SC_RAISINGDRAGON))
				bonus += sc->getSCE(SC_RAISINGDRAGON)->val1;
			if(sc->getSCE(SC_SERVICE4U))
				bonus += sc->getSCE(SC_SERVICE4U)->val2;
			if(sc->getSCE(SC_MERC_SPUP))
				bonus += sc->getSCE(SC_MERC_SPUP)->val2;
			if (sc->getSCE(SC_LUXANIMA))
				bonus += sc->getSCE(SC_LUXANIMA)->val3;
			if (sc->getSCE(SC_MTF_MSP))
				bonus += sc->getSCE(SC_MTF_MSP)->val1;
			if(sc->getSCE(SC_PACKING_ENVELOPE4))
				bonus += sc->getSCE(SC_PACKING_ENVELOPE4)->val1;

			//Decreasing
			if (sc->getSCE(SC_MELODYOFSINK))
				bonus -= sc->getSCE(SC_MELODYOFSINK)->val3;
			if (sc->getSCE(SC_2011RWC_SCROLL))
				bonus -= 10;
			if (sc->getSCE(SC_INFINITY_DRINK))
				bonus += 5;
			if (sc->getSCE(SC_MENTAL_POTION))
				bonus += sc->getSCE(SC_MENTAL_POTION)->val1;
			if(sc->getSCE(SC_COMBAT_PILL))
				bonus -= 3;
			if(sc->getSCE(SC_COMBAT_PILL2))
				bonus -= 5;
		}
		// Max rate reduce is -100%
		bonus = cap_value(bonus,-100,INT_MAX);
	}

	return min(bonus,INT_MAX);
}

/**
* SP bonus rate from equipment
*/
static int status_get_spbonus_equip(TBL_PC *sd) {
	int bonus = 0;

	bonus += sd->sprate;

	return bonus -= 100; //Default sprate is 100, so it should be add 0%
}

/**
* SP bonus rate from usable items
*/
static int status_get_spbonus_item(block_list *bl) {
	int bonus = 0;

	status_change *sc = status_get_sc(bl);

	//Bonus by SC
	if (sc) {
		if (sc->getSCE(SC_INCREASE_MAXSP))
			bonus += sc->getSCE(SC_INCREASE_MAXSP)->val1;
		if (sc->getSCE(SC_LIFE_FORCE_F))
			bonus += sc->getSCE(SC_LIFE_FORCE_F)->val1;
		if (sc->getSCE(SC_VITATA_500))
			bonus += sc->getSCE(SC_VITATA_500)->val2;
		if (sc->getSCE(SC_ENERGY_DRINK_RESERCH))
			bonus += sc->getSCE(SC_ENERGY_DRINK_RESERCH)->val3;
	}

	// Max rate reduce is -100%
	return cap_value(bonus, -100, INT_MAX);
}

/**
 * Get AP bonus modifiers
 * @param bl: block_list that will be checked
 * @param type: type of e_status_bonus (STATUS_BONUS_FIX or STATUS_BONUS_RATE)
 * @return bonus: total bonus for AP
 */
static int status_get_apbonus(struct block_list *bl, enum e_status_bonus type) {
	int bonus = 0;

	if (type == STATUS_BONUS_FIX) {
		status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			map_session_data *sd = map_id2sd(bl->id);
			//uint16 skill_lv;

			bonus += sd->bonus.ap;
			//if ((skill_lv = pc_checkskill(sd, NV_BASIC)) > 0)
			//	bonus += 100 * skill_lv;
		}

		//Bonus by SC
		if (sc) {
			//if (sc->getSCE(SC_NONE))
			//	bonus += sc->getSCE(SC_NONE)->val1;
		}
	} else if (type == STATUS_BONUS_RATE) {
		status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			map_session_data *sd = map_id2sd(bl->id);
			//uint8 i;

			//if ((i = pc_checkskill(sd, NV_BASIC)) > 0)
			//	bonus += 100 * i;
		}

		//Bonus by SC
		if (sc) {
			//if (sc->getSCE(SC_NONE))
			//	bonus += sc->getSCE(SC_NONE)->val1;
		}
		// Max rate reduce is -100%
		bonus = cap_value(bonus, -100, INT_MAX);
	}

	return min(bonus, INT_MAX);
}

/**
 * AP bonus rate from equipment
 * @param sd: Player data
 * @return AP rate
 */
static int status_get_apbonus_equip(TBL_PC *sd) {
	int bonus = 0;

	bonus += sd->aprate;

	return bonus -= 100; //Default aprate is 100, so it should be add 0%
}

/**
 * AP bonus rate from usable items
 * @param bl: Object to check against
 * @return AP bonus
 */
static int status_get_apbonus_item(block_list *bl) {
	int bonus = 0;

	status_change *sc = status_get_sc(bl);

	//Bonus by SC
	if (sc) {
		//if (sc->getSCE(SC_NONE))
		//	bonus += sc->getSCE(SC_NONE)->val1;
	}

	// Max rate reduce is -100%
	return cap_value(bonus, -100, INT_MAX);
}

/**
 * Get final MaxHP or MaxSP for player. References: http://irowiki.org/wiki/Max_HP and http://irowiki.org/wiki/Max_SP
 * The calculation needs base_level, base_status/battle_status (vit or int), additive modifier, and multiplicative modifier
 * @param sd Player
 * @param stat Vit/Int of player as param modifier
 * @param isHP true - calculates Max HP, false - calculated Max SP
 * @return max The max value of HP or SP
 */
static unsigned int status_calc_maxhpsp_pc(map_session_data* sd, unsigned int stat, bool isHP) {
	nullpo_ret(sd);

	double dmax = 0;
	uint32 level = umax(sd->status.base_level,1);
	std::shared_ptr<s_job_info> job = job_db.find(pc_mapid2jobid(sd->class_, sd->status.sex));

	if (job == nullptr)
		return 1;

	if (isHP) { //Calculates MaxHP
		double equip_bonus = 0, item_bonus = 0;
		dmax = job->base_hp[level-1] * (1 + (umax(stat,1) * 0.01)) * ((sd->class_&JOBL_UPPER)?1.25:(pc_is_taekwon_ranker(sd))?3:1);
		dmax += sd->indexed_bonus.param_equip[PARAM_VIT]; //Vit from equip gives +1 additional HP
		dmax += status_get_hpbonus(&sd->bl,STATUS_BONUS_FIX);
		equip_bonus = (dmax * status_get_hpbonus_equip(sd) / 100);
		item_bonus = (dmax * status_get_hpbonus_item(&sd->bl) / 100);
		dmax += equip_bonus + item_bonus;
		dmax += (int64)(dmax * status_get_hpbonus(&sd->bl,STATUS_BONUS_RATE) / 100); //Aegis accuracy
	}
	else { //Calculates MaxSP
		double equip_bonus = 0, item_bonus = 0;
		dmax = job->base_sp[level-1] * (1 + (umax(stat,1) * 0.01)) * ((sd->class_&JOBL_UPPER)?1.25:(pc_is_taekwon_ranker(sd))?3:1);
		dmax += sd->indexed_bonus.param_equip[PARAM_INT]; //Int from equip gives +1 additional SP
		dmax += status_get_spbonus(&sd->bl,STATUS_BONUS_FIX);
		equip_bonus = (dmax * status_get_spbonus_equip(sd) / 100);
		item_bonus = (dmax * status_get_spbonus_item(&sd->bl) / 100);
		dmax += equip_bonus + item_bonus;
		dmax += (int64)(dmax * status_get_spbonus(&sd->bl,STATUS_BONUS_RATE) / 100); //Aegis accuracy
	}

	//Make sure it's not negative before casting to unsigned int
	if(dmax < 1) dmax = 1;

	return cap_value((unsigned int)dmax,1,UINT_MAX);
}

/**
 * Get final MaxAP for player.
 * @param sd: Player data
 * @return AP amount
 */
static unsigned int status_calc_maxap_pc(map_session_data* sd) {
	double dmax = 0, equip_bonus = 0, item_bonus = 0;

	nullpo_ret(sd);

	dmax = (sd->class_&JOBL_FOURTH) ? 200 : 0;
	dmax += status_get_apbonus(&sd->bl, STATUS_BONUS_FIX);
	equip_bonus = (dmax * status_get_apbonus_equip(sd) / 100);
	item_bonus = (dmax * status_get_apbonus_item(&sd->bl) / 100);
	dmax += equip_bonus + item_bonus;
	dmax += (int64)(dmax * status_get_apbonus(&sd->bl, STATUS_BONUS_RATE) / 100);// Aegis accuracy

	//Make sure it's not negative before casting to unsigned int
	if (dmax < 0) dmax = 0;

	return cap_value((unsigned int)dmax, 0, UINT_MAX);
}

/**
 * Calculates player's weight
 * @param sd: Player object
 * @param flag: Calculation type
 *   CALCWT_ITEM - Item weight
 *   CALCWT_MAXBONUS - Skill/Status/Configuration max weight bonus
 * @return false - failed, true - success
 */
bool status_calc_weight(map_session_data *sd, enum e_status_calc_weight_opt flag)
{
	int b_weight, b_max_weight, skill, i;
	status_change *sc;

	nullpo_retr(false, sd);

	sc = &sd->sc;
	b_max_weight = sd->max_weight; // Store max weight for later comparison
	b_weight = sd->weight; // Store current weight for later comparison
	sd->max_weight = job_db.get_maxWeight(pc_mapid2jobid(sd->class_, sd->status.sex)) + sd->status.str * 300; // Recalculate max weight

	if (flag&CALCWT_ITEM) {
		sd->weight = 0; // Reset current weight

		for(i = 0; i < MAX_INVENTORY; i++) {
			if (!sd->inventory.u.items_inventory[i].nameid || sd->inventory_data[i] == nullptr)
				continue;
			sd->weight += sd->inventory_data[i]->weight * sd->inventory.u.items_inventory[i].amount;
		}
	}

	if (flag&CALCWT_MAXBONUS) {
		// Skill/Status bonus weight increases
		sd->max_weight += sd->add_max_weight; // From bAddMaxWeight
		if ((skill = pc_checkskill(sd, MC_INCCARRY)) > 0)
			sd->max_weight += 2000 * skill;
		if (pc_isriding(sd) && pc_checkskill(sd, KN_RIDING) > 0)
			sd->max_weight += 10000;
		else if (pc_isridingdragon(sd))
			sd->max_weight += 5000 + 2000 * pc_checkskill(sd, RK_DRAGONTRAINING);
		if (sc->getSCE(SC_KNOWLEDGE))
			sd->max_weight += sd->max_weight * sc->getSCE(SC_KNOWLEDGE)->val1 / 10;
		if ((skill = pc_checkskill(sd, ALL_INCCARRY)) > 0)
			sd->max_weight += 2000 * skill;
		if (pc_ismadogear(sd))
			sd->max_weight += 15000;
	}

	// Update the client if the new weight calculations don't match
	if (b_weight != sd->weight)
		clif_updatestatus(*sd, SP_WEIGHT);
	if (b_max_weight != sd->max_weight) {
		clif_updatestatus(*sd, SP_MAXWEIGHT);
		pc_updateweightstatus(sd);
	}

	return true;
}

/**
 * Calculates player's cart weight
 * @param sd: Player object
 * @param flag: Calculation type
 *   CALCWT_ITEM - Cart item weight
 *   CALCWT_MAXBONUS - Skill/Status/Configuration max weight bonus
 *   CALCWT_CARTSTATE - Whether to check for cart state
 * @return false - failed, true - success
 */
bool status_calc_cart_weight(map_session_data *sd, enum e_status_calc_weight_opt flag)
{
	int b_cart_weight_max, i;

	nullpo_retr(false, sd);

	if (!pc_iscarton(sd) && !(flag&CALCWT_CARTSTATE))
		return false;

	b_cart_weight_max = sd->cart_weight_max; // Store cart max weight for later comparison
	sd->cart_weight_max = battle_config.max_cart_weight; // Recalculate max weight

	if (flag&CALCWT_ITEM) {
		sd->cart_weight = 0; // Reset current cart weight
		sd->cart_num = 0; // Reset current cart item count

		for(i = 0; i < MAX_CART; i++) {
			if (!sd->cart.u.items_cart[i].nameid)
				continue;
			sd->cart_weight += itemdb_weight(sd->cart.u.items_cart[i].nameid) * sd->cart.u.items_cart[i].amount; // Recalculate current cart weight
			sd->cart_num++; // Recalculate current cart item count
		}
	}

	// Skill bonus max weight increases
	if (flag&CALCWT_MAXBONUS)
		sd->cart_weight_max += (pc_checkskill(sd, GN_REMODELING_CART) * 5000);

	// Update the client if the new weight calculations don't match
	if (b_cart_weight_max != sd->cart_weight_max)
		clif_updatestatus(*sd, SP_CARTINFO);

	return true;
}

/**
 * Calculates player data from scratch without counting SC adjustments
 * Should be invoked whenever players raise stats, learn passive skills or change equipment
 * @param sd: Player object
 * @param opt: Whether it is first calc (login) or not
 * @return (-1) for too many recursive calls, (1) recursive call, (0) success
 */
int status_calc_pc_sub(map_session_data* sd, uint8 opt)
{
	static int calculating = 0; ///< Check for recursive call preemption. [Skotlex]
	struct status_data *base_status; ///< Pointer to the player's base status
	status_change *sc = &sd->sc;
	struct s_skill b_skill[MAX_SKILL]; ///< Previous skill tree
	int i, skill, refinedef = 0;
	short index = -1;

	if (++calculating > 10) // Too many recursive calls!
		return -1;

	// Remember player-specific values that are currently being shown to the client (for refresh purposes)
	memcpy(b_skill, &sd->status.skill, sizeof(b_skill));

	pc_calc_skilltree(sd);	// SkillTree calculation

	if (opt&SCO_FIRST) {
		// Load Hp/SP from char-received data.
		sd->battle_status.hp = sd->status.hp;
		sd->battle_status.sp = sd->status.sp;
		if (battle_config.keep_ap_on_logout == 1)
			sd->battle_status.ap = sd->status.ap;
		sd->regen.sregen = &sd->sregen;
		sd->regen.ssregen = &sd->ssregen;
	}

	base_status = &sd->base_status;
	// These are not zeroed. [zzo]
	sd->hprate = 100;
	sd->sprate = 100;
	sd->aprate = 100;
	sd->castrate = 100;
	sd->dsprate = 100;
	sd->hprecov_rate = 100;
	sd->sprecov_rate = 100;
	sd->matk_rate = 100;
	sd->critical_rate = sd->hit_rate = sd->flee_rate = sd->flee2_rate = 100;
	sd->def_rate = sd->def2_rate = sd->mdef_rate = sd->mdef2_rate = 100;
	sd->patk_rate = sd->smatk_rate = 100;
	sd->res_rate = sd->mres_rate = 100;
	sd->hplus_rate = sd->crate_rate = 100;
	sd->regen.state.block = 0;
	sd->add_max_weight = 0;

	sd->indexed_bonus = {};

	memset (&sd->right_weapon.overrefine, 0, sizeof(sd->right_weapon) - sizeof(sd->right_weapon.atkmods));
	memset (&sd->left_weapon.overrefine, 0, sizeof(sd->left_weapon) - sizeof(sd->left_weapon.atkmods));

	if (sd->special_state.intravision)
		clif_status_load(&sd->bl, EFST_CLAIRVOYANCE, 0);

	if (sd->special_state.no_walk_delay)
		clif_status_load(&sd->bl, EFST_ENDURE, 0);

	memset(&sd->special_state,0,sizeof(sd->special_state));

	if (pc_isvip(sd)) // Magic Stone requirement avoidance for VIP.
		sd->special_state.no_gemstone = battle_config.vip_gemstone;

	if (!sd->state.permanent_speed) {
		memset(&base_status->max_hp, 0, sizeof(struct status_data)-(sizeof(base_status->hp)+sizeof(base_status->sp)+sizeof(base_status->ap)));
		base_status->speed = DEFAULT_WALK_SPEED;
	} else {
		int pSpeed = base_status->speed;

		memset(&base_status->max_hp, 0, sizeof(struct status_data)-(sizeof(base_status->hp)+sizeof(base_status->sp)+sizeof(base_status->ap)));
		base_status->speed = pSpeed;
	}

	// !FIXME: Most of these stuff should be calculated once, but how do I fix the memset above to do that? [Skotlex]
	// Give them all modes except these (useful for clones)
	base_status->mode = static_cast<e_mode>(MD_MASK&~(MD_STATUSIMMUNE|MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC|MD_DETECTOR|MD_ANGRY|MD_TARGETWEAK));

	base_status->size = (sd->class_&JOBL_BABY) ? SZ_SMALL : (((sd->class_&MAPID_BASEMASK) == MAPID_SUMMONER) ? battle_config.summoner_size : SZ_MEDIUM);
	if (battle_config.character_size && pc_isriding(sd)) { // [Lupus]
		if (sd->class_&JOBL_BABY) {
			if (battle_config.character_size&SZ_BIG)
				base_status->size++;
		} else
		if(battle_config.character_size&SZ_MEDIUM)
			base_status->size++;
	}
	base_status->aspd_rate = 1000;
	base_status->ele_lv = 1;
	base_status->race = ((sd->class_&MAPID_BASEMASK) == MAPID_SUMMONER) ? battle_config.summoner_race : RC_PLAYER_HUMAN;
	base_status->class_ = CLASS_NORMAL;

	sd->autospell.clear();
	sd->autospell2.clear();
	sd->autospell3.clear();
	sd->addeff.clear();
	sd->addeff_atked.clear();
	sd->addeff_onskill.clear();
	sd->skillatk.clear();
	sd->skillusesprate.clear();
	sd->skillusesp.clear();
	sd->skillheal.clear();
	sd->skillheal2.clear();
	sd->skillblown.clear();
	sd->skillcastrate.clear();
	sd->skillfixcastrate.clear();
	sd->subskill.clear();
	sd->skillcooldown.clear();
	sd->skillfixcast.clear();
	sd->skillvarcast.clear();
	sd->add_def.clear();
	sd->add_mdef.clear();
	sd->add_mdmg.clear();
	sd->reseff.clear();
	sd->itemgrouphealrate.clear();
	sd->add_drop.clear();
	sd->itemhealrate.clear();
	sd->subele2.clear();
	sd->subrace3.clear();
	sd->skilldelay.clear();
	sd->sp_vanish.clear();
	sd->hp_vanish.clear();
	sd->itemsphealrate.clear();
	sd->itemgroupsphealrate.clear();

	// Zero up structures...
	memset(&sd->hp_loss, 0, sizeof(sd->hp_loss)
		+ sizeof(sd->sp_loss)
		+ sizeof(sd->hp_regen)
		+ sizeof(sd->sp_regen)
		+ sizeof(sd->percent_hp_regen)
		+ sizeof(sd->percent_sp_regen)
		+ sizeof(sd->def_set_race)
		+ sizeof(sd->mdef_set_race)
		+ sizeof(sd->norecover_state_race)
		+ sizeof(sd->hp_vanish_race)
		+ sizeof(sd->sp_vanish_race)
	);

	memset(&sd->bonus, 0, sizeof(sd->bonus));

	// Autobonus
	pc_delautobonus(*sd, sd->autobonus, true);
	pc_delautobonus(*sd, sd->autobonus2, true);
	pc_delautobonus(*sd, sd->autobonus3, true);

	if (sd->pd != nullptr) {
		pet_delautobonus(*sd, sd->pd->autobonus, true);
		pet_delautobonus(*sd, sd->pd->autobonus2, true);
		pet_delautobonus(*sd, sd->pd->autobonus3, true);
	}

	// Parse equipment
	for (i = 0; i < EQI_MAX; i++) {
		current_equip_item_index = index = sd->equip_index[i]; // We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		current_equip_combo_pos = 0;
		if (index < 0)
			continue;
		if (i == EQI_AMMO)
			continue;
		if (pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index))
			continue;
		if (!sd->inventory_data[index])
			continue;

		base_status->def += sd->inventory_data[index]->def;

		// Items may be equipped, their effects however are nullified.
		if (opt&SCO_FIRST && sd->inventory_data[index]->equip_script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT)
			|| !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) { // Execute equip-script on login
			run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
			if (!calculating)
				return 1;
		}

		// Sanitize the refine level in case someone decreased the value inbetween
		if (sd->inventory.u.items_inventory[index].refine > MAX_REFINE)
			sd->inventory.u.items_inventory[index].refine = MAX_REFINE;

		std::shared_ptr<s_refine_level_info> info = refine_db.findCurrentLevelInfo( *sd->inventory_data[index], sd->inventory.u.items_inventory[index] );
#ifdef RENEWAL
		std::shared_ptr<s_enchantgradelevel> enchantgrade_info = nullptr;

		if( sd->inventory.u.items_inventory[index].enchantgrade > 0 ){
			enchantgrade_info = enchantgrade_db.findCurrentLevelInfo( *sd->inventory_data[index], sd->inventory.u.items_inventory[index] );
		}
#endif

		if (sd->inventory_data[index]->type == IT_WEAPON) {
			int wlv = sd->inventory_data[index]->weapon_level;
			struct weapon_data *wd;
			struct weapon_atk *wa;

			if(wlv >= MAX_WEAPON_LEVEL)
				wlv = MAX_WEAPON_LEVEL;

			if(i == EQI_HAND_L && sd->inventory.u.items_inventory[index].equip == EQP_HAND_L) {
				wd = &sd->left_weapon; // Left-hand weapon
				wa = &base_status->lhw;
			} else {
				wd = &sd->right_weapon;
				wa = &base_status->rhw;
			}
			wa->atk += sd->inventory_data[index]->atk;
			if( info != nullptr ){
				wa->atk2 += info->bonus / 100;

#ifdef RENEWAL
				if( enchantgrade_info != nullptr ){
					wa->atk2 += ( ( ( info->bonus / 100 ) * enchantgrade_info->bonus ) / 100 );
				}

				if( wlv == 5 ){
					base_status->patk += sd->inventory.u.items_inventory[index].refine * 2;
					base_status->smatk += sd->inventory.u.items_inventory[index].refine * 2;
				}
#endif
			}
#ifdef RENEWAL
			if (sd->bonus.weapon_atk_rate)
				wa->atk += wa->atk * sd->bonus.weapon_atk_rate / 100;
			wa->matk += sd->inventory_data[index]->matk;
			wa->wlv = wlv;
			// Renewal magic attack refine bonus
			if( info != nullptr && sd->weapontype1 != W_BOW ){
				wa->matk += info->bonus / 100;

				if( enchantgrade_info != nullptr ){
					wa->matk += ( ( ( info->bonus / 100 ) * enchantgrade_info->bonus ) / 100 );
				}
			}
#endif
			// Overrefine bonus.
			if( info != nullptr ){
				wd->overrefine = info->randombonus_max / 100;
			}

			wa->range += sd->inventory_data[index]->range;
			if(sd->inventory_data[index]->script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) {
				if (wd == &sd->left_weapon) {
					sd->state.lr_flag = 1;
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if (!calculating) // Abort, run_script retriggered this. [Skotlex]
					return 1;
			}
#ifdef RENEWAL
			if (sd->bonus.weapon_matk_rate)
				wa->matk += wa->matk * sd->bonus.weapon_matk_rate / 100;
#endif
			if(sd->inventory.u.items_inventory[index].card[0] == CARD0_FORGE) { // Forged weapon
				wd->star += (sd->inventory.u.items_inventory[index].card[1]>>8);
				if(wd->star >= 15) wd->star = 40; // 3 Star Crumbs now give +40 dmg
				if(pc_famerank(MakeDWord(sd->inventory.u.items_inventory[index].card[2],sd->inventory.u.items_inventory[index].card[3]) ,MAPID_BLACKSMITH))
					wd->star += 10;
				if (!wa->ele) // Do not overwrite element from previous bonuses.
					wa->ele = (sd->inventory.u.items_inventory[index].card[1]&0x0f);
			}
		} else if(sd->inventory_data[index]->type == IT_ARMOR) {
			if( info != nullptr ){
				refinedef += info->bonus;

#ifdef RENEWAL
				if( sd->inventory_data[index]->armor_level == 2 ){
					base_status->res += sd->inventory.u.items_inventory[index].refine * 2;
					base_status->mres += sd->inventory.u.items_inventory[index].refine * 2;
				}
#endif
			}

			if(sd->inventory_data[index]->script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) {
				if( i == EQI_HAND_L ) // Shield
					sd->state.lr_flag = 3;
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if( i == EQI_HAND_L ) // Shield
					sd->state.lr_flag = 0;
				if (!calculating) // Abort, run_script retriggered this. [Skotlex]
					return 1;
			}
		} else if( sd->inventory_data[index]->type == IT_SHADOWGEAR ) { // Shadow System
			if (sd->inventory_data[index]->script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) {
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if( !calculating )
					return 1;
			}
		}
	}

	if(sd->equip_index[EQI_AMMO] >= 0) {
		index = sd->equip_index[EQI_AMMO];
		if(sd->inventory_data[index]) { // Arrows
			sd->bonus.arrow_atk += sd->inventory_data[index]->atk;
			sd->state.lr_flag = 2;
			if( !itemdb_group.item_exists(IG_THROWABLE, sd->inventory_data[index]->nameid) ) // Don't run scripts on throwable items
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			if (!calculating) // Abort, run_script retriggered status_calc_pc. [Skotlex]
				return 1;
		}
	}

	// Process and check item combos
	if (!sd->combos.empty()) {
		for (const auto &combo : sd->combos) {
			std::shared_ptr<s_item_combo> item_combo;

			current_equip_item_index = -1;
			current_equip_combo_pos = combo->pos;

			if (combo->bonus == nullptr || !(item_combo = itemdb_combo.find(combo->id)))
				continue;

			bool no_run = false;
			size_t j = 0;

			// Check combo items
			while (j < item_combo->nameid.size()) {
				std::shared_ptr<item_data> id = item_db.find(item_combo->nameid[j]);

				// Don't run the script if at least one of combo's pair has restriction
				if (id && !pc_has_permission(sd, PC_PERM_USE_ALL_EQUIPMENT) && itemdb_isNoEquip(id.get(), sd->bl.m)) {
					no_run = true;
					break;
				}

				j++;
			}

			if (no_run)
				continue;

			run_script(combo->bonus, 0, sd->bl.id, 0);

			if (!calculating) // Abort, run_script retriggered this
				return 1;
		}
	}

	// Store equipment script bonuses
	memcpy(sd->indexed_bonus.param_equip,sd->indexed_bonus.param_bonus,sizeof(sd->indexed_bonus.param_equip));
	memset(sd->indexed_bonus.param_bonus, 0, sizeof(sd->indexed_bonus.param_bonus));

	base_status->def += (refinedef+50)/100;

	// Parse Cards
	for (i = 0; i < EQI_MAX; i++) {
		current_equip_item_index = index = sd->equip_index[i]; // We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		current_equip_combo_pos = 0;
		if (index < 0)
			continue;
		if (i == EQI_AMMO)
			continue;
		if (pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index))
			continue;

		if (sd->inventory_data[index]) {
			int j;

			// Card script execution.
			if (itemdb_isspecial(sd->inventory.u.items_inventory[index].card[0]))
				continue;
			for (j = 0; j < MAX_SLOTS; j++) { // Uses MAX_SLOTS to support Soul Bound system [Inkfish]
				int c = sd->inventory.u.items_inventory[index].card[j];
				current_equip_card_id= c;
				if(!c)
					continue;

				std::shared_ptr<item_data> data = item_db.find(c);

				if(!data)
					continue;
				if (opt&SCO_FIRST && data->equip_script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(data.get(), sd->bl.m))) {// Execute equip-script on login
					run_script(data->equip_script,0,sd->bl.id,0);
					if (!calculating)
						return 1;
				}
				if(!data->script)
					continue;
				if(!pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) && itemdb_isNoEquip(data.get(), sd->bl.m)) // Card restriction checks.
					continue;
				if(i == EQI_HAND_L && sd->inventory.u.items_inventory[index].equip == EQP_HAND_L) { // Left hand status.
					sd->state.lr_flag = 1;
					run_script(data->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(data->script,0,sd->bl.id,0);
				if (!calculating) // Abort, run_script his function. [Skotlex]
					return 1;
			}
		}
	}
	current_equip_card_id = 0; // Clear stored card ID [Secret]

	// Parse random options
	for (i = 0; i < EQI_MAX; i++) {
		current_equip_item_index = index = sd->equip_index[i];
		current_equip_combo_pos = 0;
		current_equip_opt_index = -1;

		if (index < 0)
			continue;
		if (i == EQI_AMMO)
			continue;
		if (pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index))
			continue;
		
		if (sd->inventory_data[index]) {
			for (uint8 j = 0; j < MAX_ITEM_RDM_OPT; j++) {
				short opt_id = sd->inventory.u.items_inventory[index].option[j].id;

				if (!opt_id)
					continue;
				current_equip_opt_index = j;

				std::shared_ptr<s_random_opt_data> data = random_option_db.find(opt_id);

				if (!data || !data->script)
					continue;
				if (!pc_has_permission(sd, PC_PERM_USE_ALL_EQUIPMENT) && itemdb_isNoEquip(sd->inventory_data[index], sd->bl.m))
					continue;
				if (i == EQI_HAND_L && sd->inventory.u.items_inventory[index].equip == EQP_HAND_L) { // Left hand status.
					sd->state.lr_flag = 1;
					run_script(data->script, 0, sd->bl.id, 0);
					sd->state.lr_flag = 0;
				}
				else
					run_script(data->script, 0, sd->bl.id, 0);
				if (!calculating)
					return 1;
			}
		}
		current_equip_opt_index = -1;
	}

	if (sc->count && sc->getSCE(SC_ITEMSCRIPT)) {
		std::shared_ptr<item_data> data = item_db.find(sc->getSCE(SC_ITEMSCRIPT)->val1);

		if (data && data->script)
			run_script(data->script, 0, sd->bl.id, 0);
	}

	pc_bonus_script(sd);

	if( sd->pd ) { // Pet Bonus
		struct pet_data *pd = sd->pd;
		std::shared_ptr<s_pet_db> pet_db_ptr = pd->get_pet_db();

		if (pet_db_ptr != nullptr && pet_db_ptr->pet_bonus_script)
			run_script(pet_db_ptr->pet_bonus_script,0,sd->bl.id,0);
		if (pet_db_ptr != nullptr && pd->pet.intimate > 0 && (!battle_config.pet_equip_required || pd->pet.equip > 0) && pd->state.skillbonus == 1 && pd->bonus)
			pc_bonus(sd,pd->bonus->type, pd->bonus->val);
	}

	// param_bonus now holds card bonuses.
	if(base_status->rhw.range < 1) base_status->rhw.range = 1;
	if(base_status->lhw.range < 1) base_status->lhw.range = 1;
	if(base_status->rhw.range < base_status->lhw.range)
		base_status->rhw.range = base_status->lhw.range;

	sd->bonus.double_rate += sd->bonus.double_add_rate;
	sd->bonus.perfect_hit += sd->bonus.perfect_hit_add;
	sd->bonus.splash_range += sd->bonus.splash_add_range;

	// Damage modifiers from weapon type
	if( std::shared_ptr<s_sizefix_db> right_weapon = size_fix_db.find(sd->weapontype1); right_weapon != nullptr ){
		sd->right_weapon.atkmods[SZ_SMALL] = right_weapon->small;
		sd->right_weapon.atkmods[SZ_MEDIUM] = right_weapon->medium;
		sd->right_weapon.atkmods[SZ_BIG] = right_weapon->large;
	}

	if( std::shared_ptr<s_sizefix_db> left_weapon = size_fix_db.find(sd->weapontype2); left_weapon != nullptr ){
		sd->left_weapon.atkmods[SZ_SMALL] = left_weapon->small;
		sd->left_weapon.atkmods[SZ_MEDIUM] = left_weapon->medium;
		sd->left_weapon.atkmods[SZ_BIG] = left_weapon->large;
	}

	if((pc_isriding(sd) || pc_isridingdragon(sd)) &&
		(sd->status.weapon==W_1HSPEAR || sd->status.weapon==W_2HSPEAR))
	{	// When Riding with spear, damage modifier to mid-class becomes
		// same as versus large size.
		sd->right_weapon.atkmods[SZ_MEDIUM] = sd->right_weapon.atkmods[SZ_BIG];
		sd->left_weapon.atkmods[SZ_MEDIUM] = sd->left_weapon.atkmods[SZ_BIG];
	}

// ----- STATS CALCULATION -----

	// Job bonuses
	std::shared_ptr<s_job_info> job_info = job_db.find( pc_mapid2jobid( sd->class_, sd->status.sex ) );

	if( job_info != nullptr ){
		const auto& bonus = job_info->job_bonus[sd->status.job_level-1];

		base_status->str += bonus[PARAM_STR];
		base_status->agi += bonus[PARAM_AGI];
		base_status->vit += bonus[PARAM_VIT];
		base_status->int_ += bonus[PARAM_INT];
		base_status->dex += bonus[PARAM_DEX];
		base_status->luk += bonus[PARAM_LUK];
		base_status->pow += bonus[PARAM_POW];
		base_status->sta += bonus[PARAM_STA];
		base_status->wis += bonus[PARAM_WIS];
		base_status->spl += bonus[PARAM_SPL];
		base_status->con += bonus[PARAM_CON];
		base_status->crt += bonus[PARAM_CRT];
	}

	// If a Super Novice has never died and is at least joblv 70, he gets all stats +10
	if(((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && (sd->status.job_level >= 70  || sd->class_&JOBL_THIRD)) && sd->die_counter == 0) {
		base_status->str += 10;
		base_status->agi += 10;
		base_status->vit += 10;
		base_status->int_+= 10;
		base_status->dex += 10;
		base_status->luk += 10;
	}

	// Absolute modifiers from passive skills
	if(pc_checkskill(sd,BS_HILTBINDING)>0)
		base_status->str++;
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0)
		base_status->int_ += (skill+1)/2; // +1 INT / 2 lv
	if((skill=pc_checkskill(sd,AC_OWL))>0)
		base_status->dex += skill;
	if((skill = pc_checkskill(sd,RA_RESEARCHTRAP))>0)
		base_status->int_ += skill;
	if (pc_checkskill(sd, SU_POWEROFLAND) > 0)
		base_status->int_ += 20;

	// Bonuses from cards and equipment as well as base stat, remember to avoid overflows.
	i = base_status->str + sd->status.str + sd->indexed_bonus.param_bonus[PARAM_STR] + sd->indexed_bonus.param_equip[PARAM_STR];
	base_status->str = cap_value(i,0,USHRT_MAX);
	i = base_status->agi + sd->status.agi + sd->indexed_bonus.param_bonus[PARAM_AGI] + sd->indexed_bonus.param_equip[PARAM_AGI];
	base_status->agi = cap_value(i,0,USHRT_MAX);
	i = base_status->vit + sd->status.vit + sd->indexed_bonus.param_bonus[PARAM_VIT] + sd->indexed_bonus.param_equip[PARAM_VIT];
	base_status->vit = cap_value(i,0,USHRT_MAX);
	i = base_status->int_+ sd->status.int_+ sd->indexed_bonus.param_bonus[PARAM_INT] + sd->indexed_bonus.param_equip[PARAM_INT];
	base_status->int_ = cap_value(i,0,USHRT_MAX);
	i = base_status->dex + sd->status.dex + sd->indexed_bonus.param_bonus[PARAM_DEX] + sd->indexed_bonus.param_equip[PARAM_DEX];
	base_status->dex = cap_value(i,0,USHRT_MAX);
	i = base_status->luk + sd->status.luk + sd->indexed_bonus.param_bonus[PARAM_LUK] + sd->indexed_bonus.param_equip[PARAM_LUK];
	base_status->luk = cap_value(i,0,USHRT_MAX);
	i = base_status->pow + sd->status.pow + sd->indexed_bonus.param_bonus[PARAM_POW] + sd->indexed_bonus.param_equip[PARAM_POW];
	base_status->pow = cap_value(i, 0, USHRT_MAX);
	i = base_status->sta + sd->status.sta + sd->indexed_bonus.param_bonus[PARAM_STA] + sd->indexed_bonus.param_equip[PARAM_STA];
	base_status->sta = cap_value(i, 0, USHRT_MAX);
	i = base_status->wis + sd->status.wis + sd->indexed_bonus.param_bonus[PARAM_WIS] + sd->indexed_bonus.param_equip[PARAM_WIS];
	base_status->wis = cap_value(i, 0, USHRT_MAX);
	i = base_status->spl + sd->status.spl + sd->indexed_bonus.param_bonus[PARAM_SPL] + sd->indexed_bonus.param_equip[PARAM_SPL];
	base_status->spl = cap_value(i, 0, USHRT_MAX);
	i = base_status->con + sd->status.con + sd->indexed_bonus.param_bonus[PARAM_CON] + sd->indexed_bonus.param_equip[PARAM_CON];
	base_status->con = cap_value(i, 0, USHRT_MAX);
	i = base_status->crt + sd->status.crt + sd->indexed_bonus.param_bonus[PARAM_CRT] + sd->indexed_bonus.param_equip[PARAM_CRT];
	base_status->crt = cap_value(i, 0, USHRT_MAX);

	if (sd->special_state.no_walk_delay) {
		if (sc->getSCE(SC_ENDURE)) {
			if (sc->getSCE(SC_ENDURE)->val4)
				sc->getSCE(SC_ENDURE)->val4 = 0;
			status_change_end(&sd->bl, SC_ENDURE);
		}
		clif_status_load(&sd->bl, EFST_ENDURE, 1);
		base_status->mdef++;
	}

// ----- CONCENTRATION CALCULATION -----
	if ((skill = pc_checkskill(sd, NW_GRENADE_MASTERY)) > 0)
		base_status->con += skill;

// ------ ATTACK CALCULATION ------

	// Base batk value is set in status_calc_misc
#ifndef RENEWAL
	// !FIXME: Weapon-type bonus (Why is the weapon_atk bonus applied to base attack?)
	if (sd->status.weapon < MAX_WEAPON_TYPE && sd->indexed_bonus.weapon_atk[sd->status.weapon])
		base_status->batk += sd->indexed_bonus.weapon_atk[sd->status.weapon];
	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,BS_HILTBINDING))>0)
		base_status->batk += 4;
#else
	base_status->watk = status_weapon_atk(base_status->rhw);
	base_status->watk2 = status_weapon_atk(base_status->lhw);
	base_status->eatk = sd->bonus.eatk;
#endif

// ----- HP MAX CALCULATION -----
	base_status->max_hp = sd->status.max_hp = status_calc_maxhpsp_pc(sd,base_status->vit,true);

	if(battle_config.hp_rate != 100)
		base_status->max_hp = (unsigned int)(battle_config.hp_rate * (base_status->max_hp/100.));

	if (sd->status.base_level < 100)
		base_status->max_hp = cap_value(base_status->max_hp,1,(unsigned int)battle_config.max_hp_lv99);
	else if (sd->status.base_level < 151)
		base_status->max_hp = cap_value(base_status->max_hp,1,(unsigned int)battle_config.max_hp_lv150);
	else
		base_status->max_hp = cap_value(base_status->max_hp,1,(unsigned int)battle_config.max_hp);

// ----- SP MAX CALCULATION -----
	base_status->max_sp = sd->status.max_sp = status_calc_maxhpsp_pc(sd,base_status->int_,false);

	if(battle_config.sp_rate != 100)
		base_status->max_sp = (unsigned int)(battle_config.sp_rate * (base_status->max_sp/100.));

	base_status->max_sp = cap_value(base_status->max_sp,1,(unsigned int)battle_config.max_sp);

// ----- AP MAX CALCULATION -----
	base_status->max_ap = sd->status.max_ap = status_calc_maxap_pc(sd);

	if (battle_config.ap_rate != 100)
		base_status->max_ap = (unsigned int)(battle_config.ap_rate * (base_status->max_ap / 100.));

	base_status->max_ap = cap_value(base_status->max_ap, 0, (unsigned int)battle_config.max_ap);

// ----- RESPAWN HP/SP/AP -----

	// Calc respawn hp and store it on base_status
	if (sd->special_state.restart_full_recover) {
		base_status->hp = base_status->max_hp;
		base_status->sp = base_status->max_sp;
	} else {
		if((sd->class_&MAPID_BASEMASK) == MAPID_NOVICE && !(sd->class_&JOBL_2)
			&& battle_config.restart_hp_rate < 50)
			base_status->hp = base_status->max_hp / 2;
		else
			base_status->hp = (int64)base_status->max_hp * battle_config.restart_hp_rate/100;
		if(!base_status->hp)
			base_status->hp = 1;

		base_status->sp = (int64)base_status->max_sp * battle_config.restart_sp_rate /100;

		if( !base_status->sp ) // The minimum for the respawn setting is SP:1
			base_status->sp = 1;

		base_status->ap = (int64)base_status->max_ap * battle_config.restart_ap_rate / 100;
	}

// ----- MISC CALCULATION -----
	status_calc_misc(&sd->bl, base_status, sd->status.base_level);

	// Equipment modifiers for misc settings
	if(sd->matk_rate < 0)
		sd->matk_rate = 0;

	if(sd->matk_rate != 100) {
		base_status->matk_max = base_status->matk_max * sd->matk_rate/100;
		base_status->matk_min = base_status->matk_min * sd->matk_rate/100;
	}

	if(sd->hit_rate < 0)
		sd->hit_rate = 0;
	if(sd->hit_rate != 100)
		base_status->hit = base_status->hit * sd->hit_rate/100;

	if(sd->flee_rate < 0)
		sd->flee_rate = 0;
	if(sd->flee_rate != 100)
		base_status->flee = base_status->flee * sd->flee_rate/100;

	if(sd->def2_rate < 0)
		sd->def2_rate = 0;
	if(sd->def2_rate != 100)
		base_status->def2 = base_status->def2 * sd->def2_rate/100;

	if(sd->mdef2_rate < 0)
		sd->mdef2_rate = 0;
	if(sd->mdef2_rate != 100)
		base_status->mdef2 = base_status->mdef2 * sd->mdef2_rate/100;

	if(sd->critical_rate < 0)
		sd->critical_rate = 0;
	if(sd->critical_rate != 100)
		base_status->cri = cap_value(base_status->cri * sd->critical_rate/100,SHRT_MIN,SHRT_MAX);
	if (pc_checkskill(sd, SU_POWEROFLIFE) > 0)
		base_status->cri += 200;

	if(sd->flee2_rate < 0)
		sd->flee2_rate = 0;
	if(sd->flee2_rate != 100)
		base_status->flee2 = base_status->flee2 * sd->flee2_rate/100;

	if (sd->patk_rate < 0)
		sd->patk_rate = 0;
	if (sd->patk_rate != 100)
		base_status->patk = base_status->patk * sd->patk_rate / 100;

	if (sd->smatk_rate < 0)
		sd->smatk_rate = 0;
	if (sd->smatk_rate != 100)
		base_status->smatk = base_status->smatk * sd->smatk_rate / 100;

	if (sd->res_rate < 0)
		sd->res_rate = 0;
	if (sd->res_rate != 100)
		base_status->res = base_status->res * sd->res_rate / 100;

	if (sd->mres_rate < 0)
		sd->mres_rate = 0;
	if (sd->mres_rate != 100)
		base_status->mres = base_status->mres * sd->mres_rate / 100;

	if (sd->hplus_rate < 0)
		sd->hplus_rate = 0;
	if (sd->hplus_rate != 100)
		base_status->hplus = base_status->hplus * sd->hplus_rate / 100;

	if (sd->crate_rate < 0)
		sd->crate_rate = 0;
	if (sd->crate_rate != 100)
		base_status->crate = base_status->crate * sd->crate_rate / 100;

// ----- HIT CALCULATION -----

	// Absolute modifiers from passive skills
#ifndef RENEWAL
	if((skill=pc_checkskill(sd,BS_WEAPONRESEARCH))>0)
		base_status->hit += skill*2;
#endif
	if((skill=pc_checkskill(sd,AC_VULTURE))>0) {
#ifndef RENEWAL
		base_status->hit += skill;
#endif
		if(sd->status.weapon == W_BOW)
			base_status->rhw.range += skill;
	}
	if(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE) {
		if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0)
			base_status->hit += 2*skill;
		if((skill=pc_checkskill(sd,GS_SNAKEEYE))>0) {
			base_status->hit += skill;
			base_status->rhw.range += skill;
		}
	}
	if((sd->status.weapon == W_1HAXE || sd->status.weapon == W_2HAXE) && (skill = pc_checkskill(sd,NC_TRAININGAXE)) > 0)
		base_status->hit += skill * 3;
	if((sd->status.weapon == W_MACE || sd->status.weapon == W_2HMACE) && (skill = pc_checkskill(sd,NC_TRAININGAXE)) > 0)
		base_status->hit += skill * 2;
	if (pc_checkskill(sd, SU_POWEROFLIFE) > 0)
		base_status->hit += 20;
	if ((skill = pc_checkskill_imperial_guard(sd, 2)) > 0)// IG_SPEAR_SWORD_M
		base_status->hit += skill * 3;

	if ((skill = pc_checkskill(sd, SU_SOULATTACK)) > 0)
		base_status->rhw.range += skill_get_range2(&sd->bl, SU_SOULATTACK, skill, true);

// ----- FLEE CALCULATION -----

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,TF_MISS))>0)
		base_status->flee += skill*(sd->class_&JOBL_2 && (sd->class_&MAPID_BASEMASK) == MAPID_THIEF? 4 : 3);
	if((skill=pc_checkskill(sd,MO_DODGE))>0)
		base_status->flee += (skill*3) / 2;
	if (pc_checkskill(sd, SU_POWEROFLIFE) > 0)
		base_status->flee += 20;
	if ((skill = pc_checkskill(sd, SHC_SHADOW_SENSE)) > 0)
		base_status->flee += skill * 10;

// ----- CRITICAL CALCULATION -----

#ifdef RENEWAL
	if ((skill = pc_checkskill(sd, DC_DANCINGLESSON)) > 0)
		base_status->cri += skill * 10;
	if ((skill = pc_checkskill(sd, PR_MACEMASTERY)) > 0 && (sd->status.weapon == W_MACE || sd->status.weapon == W_2HMACE))
		base_status->cri += skill * 10;
#endif
	if ((skill = pc_checkskill(sd, SHC_SHADOW_SENSE)) > 0)
	{
		if (sd->status.weapon == W_DAGGER || sd->status.weapon == W_DOUBLE_DD || 
			sd->status.weapon == W_DOUBLE_DS || sd->status.weapon == W_DOUBLE_DA)
			base_status->cri += 100 + skill * 40;
		else if (sd->status.weapon == W_KATAR)
			base_status->cri += 50 + skill * 20;
	}

// ----- P.Atk/S.Matk CALCULATION -----
	if ((skill = pc_checkskill(sd, TR_STAGE_MANNER)) > 0 && (sd->status.weapon == W_BOW || sd->status.weapon == W_MUSICAL || sd->status.weapon == W_WHIP)) {
		base_status->patk += skill * 3;
		base_status->smatk += skill * 3;
	}
	if ((skill = pc_checkskill(sd, HN_SELFSTUDY_TATICS)) > 0)
		base_status->patk += skill;
	if ((skill = pc_checkskill(sd, HN_SELFSTUDY_SOCERY)) > 0)
		base_status->smatk += skill;
	if ((skill = pc_checkskill(sd, NW_P_F_I)) > 0 && (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		base_status->patk += skill + 2;

	// 2-Handed Staff Mastery
	if( sd->status.weapon == W_2HSTAFF && ( skill = pc_checkskill( sd, AG_TWOHANDSTAFF ) ) > 0 ){
		base_status->smatk += skill * 2;
	}

// ----- PHYSICAL RESISTANCE CALCULATION -----
	if ((skill = pc_checkskill_imperial_guard(sd, 1)) > 0)// IG_SHIELD_MASTERY
		base_status->res += skill * 3;

// ----- EQUIPMENT-DEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->def_rate < 0)
		sd->def_rate = 0;
	if(sd->def_rate != 100) {
		i = base_status->def * sd->def_rate/100;
		base_status->def = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

	if(pc_ismadogear(sd) && pc_checkskill(sd, NC_MAINFRAME) > 0)
		base_status->def += 20 + (pc_checkskill(sd, NC_MAINFRAME) * 20);

#ifndef RENEWAL
	if (!battle_config.weapon_defense_type && base_status->def > battle_config.max_def) {
		base_status->def2 += battle_config.over_def_bonus*(base_status->def -battle_config.max_def);
		base_status->def = (unsigned char)battle_config.max_def;
	}
#endif

// ----- EQUIPMENT-MDEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->mdef_rate < 0)
		sd->mdef_rate = 0;
	if(sd->mdef_rate != 100) {
		i =  base_status->mdef * sd->mdef_rate/100;
		base_status->mdef = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

#ifndef RENEWAL
	if (!battle_config.magic_defense_type && base_status->mdef > battle_config.max_def) {
		base_status->mdef2 += battle_config.over_def_bonus*(base_status->mdef -battle_config.max_def);
		base_status->mdef = (signed char)battle_config.max_def;
	}
#endif

// ----- ASPD CALCULATION -----

	/// Unlike other stats, ASPD rate modifiers from skills/SCs/items/etc are first all added together, then the final modifier is applied

	// Basic ASPD value
	i = status_base_amotion_pc(sd,base_status);
	base_status->amotion = cap_value(i,pc_maxaspd(sd),2000);

	// Relative modifiers from passive skills
	// Renewal modifiers are handled in status_base_amotion_pc
#ifndef RENEWAL_ASPD
	if((skill=pc_checkskill(sd,SA_ADVANCEDBOOK))>0 && sd->status.weapon == W_BOOK)
		base_status->aspd_rate -= 5*skill;
	if ((skill = pc_checkskill(sd,SG_DEVIL)) > 0 && ((sd->class_&MAPID_THIRDMASK) == MAPID_STAR_EMPEROR || pc_is_maxjoblv(sd)))
		base_status->aspd_rate -= 30*skill;
	if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0 &&
		(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		base_status->aspd_rate -= ((skill+1)/2) * 10;
	if(pc_isriding(sd))
		base_status->aspd_rate += 500-100*pc_checkskill(sd,KN_CAVALIERMASTERY);
	else if(pc_isridingdragon(sd))
		base_status->aspd_rate += 250-50*pc_checkskill(sd,RK_DRAGONTRAINING);
#endif
	base_status->adelay = 2*base_status->amotion;


// ----- DMOTION -----

	i =  800-base_status->agi*4;
	base_status->dmotion = cap_value(i, 400, 800);
	if(battle_config.pc_damage_delay_rate != 100)
		base_status->dmotion = base_status->dmotion*battle_config.pc_damage_delay_rate/100;

// ----- MISC CALCULATIONS -----

	// Weight
	status_calc_weight(sd, CALCWT_MAXBONUS);
	status_calc_cart_weight(sd, CALCWT_MAXBONUS);

	if (pc_checkskill(sd, SM_MOVINGRECOVERY) > 0 || pc_ismadogear(sd))
		sd->regen.state.walk = 1;
	else
		sd->regen.state.walk = 0;

	// Skill SP cost
	if((skill=pc_checkskill(sd,HP_MANARECHARGE))>0 )
		sd->dsprate -= 4*skill;

	if(sc->getSCE(SC_SERVICE4U))
		sd->dsprate -= sc->getSCE(SC_SERVICE4U)->val3;

	if(sc->getSCE(SC_SPCOST_RATE))
		sd->dsprate -= sc->getSCE(SC_SPCOST_RATE)->val1;

	// Underflow protections.
	if(sd->dsprate < 0)
		sd->dsprate = 0;
	if(sd->castrate < 0)
		sd->castrate = 0;
	if(sd->hprecov_rate < 0)
		sd->hprecov_rate = 0;
	if(sd->sprecov_rate < 0)
		sd->sprecov_rate = 0;

	// Anti-element and anti-race
	if((skill=pc_checkskill(sd,CR_TRUST))>0)
		sd->indexed_bonus.subele[ELE_HOLY] += skill*5;
	if((skill=pc_checkskill(sd,BS_SKINTEMPER))>0) {
		sd->indexed_bonus.subele[ELE_NEUTRAL] += skill;
		sd->indexed_bonus.subele[ELE_FIRE] += skill*5;
	}
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0) {
		uint8 dragon_matk = skill * 2;

		skill = skill * 4;

		sd->right_weapon.addrace[RC_DRAGON]+=skill;
		if( !battle_config.left_cardfix_to_right ){
			sd->left_weapon.addrace[RC_DRAGON] += skill;
		}
		sd->indexed_bonus.magic_addrace[RC_DRAGON]+=dragon_matk;
		sd->indexed_bonus.subrace[RC_DRAGON]+=skill;
	}
	if ((skill = pc_checkskill(sd, AB_EUCHARISTICA)) > 0) {
		sd->right_weapon.addrace[RC_DEMON] += skill;
		sd->right_weapon.addele[ELE_DARK] += skill;
		if( !battle_config.left_cardfix_to_right ){
			sd->left_weapon.addrace[RC_DEMON] += skill;
			sd->left_weapon.addele[ELE_DARK] += skill;
		}
		sd->indexed_bonus.magic_addrace[RC_DEMON] += skill;
		sd->indexed_bonus.magic_addele[ELE_DARK] += skill;
		sd->indexed_bonus.subrace[RC_DEMON] += skill;
		sd->indexed_bonus.subele[ELE_DARK] += skill;
	}
	if ((skill = pc_checkskill(sd, DK_TWOHANDDEF)) > 0 && (sd->status.weapon == W_2HSWORD || sd->status.weapon == W_2HSPEAR || sd->status.weapon == W_2HAXE)) {
		uint8 defense_bonus[SZ_MAX][10] = {
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, // SZ_SMALL
			{ 2, 3, 5, 6, 8, 9, 11, 12, 14, 15 }, // SZ_MEDIUM
			{ 3, 5, 7, 9, 10, 12, 13, 15, 16, 18 }, // SZ_BIG
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } // SZ_ALL
		};

		for( uint8 size = SZ_SMALL; size < SZ_MAX; size++ ){
			sd->indexed_bonus.weapon_subsize[size] += defense_bonus[size][skill - 1];
		}
	}
	if ((skill = pc_checkskill(sd, IQ_WILL_OF_FAITH)) > 0 && sd->status.weapon == W_KNUCKLE) {
		uint8 race_atk[10] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
		uint8 race_def[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

		sd->right_weapon.addrace[RC_UNDEAD] += race_atk[skill - 1];
		sd->right_weapon.addrace[RC_DEMON] += race_atk[skill - 1];
		if( !battle_config.left_cardfix_to_right ){
			sd->left_weapon.addrace[RC_UNDEAD] += race_atk[skill - 1];
			sd->left_weapon.addrace[RC_DEMON] += race_atk[skill - 1];
		}
		sd->indexed_bonus.subrace[RC_UNDEAD] += race_def[skill - 1];
		sd->indexed_bonus.subrace[RC_DEMON] += race_def[skill - 1];
	}
	if ((skill = pc_checkskill(sd, CD_MACE_BOOK_M)) > 0 && (sd->status.weapon == W_MACE || sd->status.weapon == W_2HMACE || sd->status.weapon == W_BOOK)) {
		uint8 attack_bonus[SZ_MAX][10] = {
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, // SZ_SMALL
			{ 2, 3, 5, 6, 8, 9, 11, 12, 14, 15 }, // SZ_MEDIUM
			{ 3, 5, 7, 9, 10, 12, 13, 15, 16, 18 }, // SZ_BIG
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } // SZ_ALL
		};

		for( uint8 size = SZ_SMALL; size < SZ_MAX; size++ ){
			sd->right_weapon.addsize[size] += attack_bonus[size][skill - 1];
			if( !battle_config.left_cardfix_to_right ){
				sd->left_weapon.addsize[size] += attack_bonus[size][skill - 1];
			}
		}
	}
	if ((skill = pc_checkskill(sd, CD_FIDUS_ANIMUS)) > 0) {
		uint8 holy_matk[10] = { 1, 3, 4, 6, 7, 9, 10, 12, 13, 15 };

		sd->indexed_bonus.magic_atk_ele[ELE_HOLY] += holy_matk[skill - 1];
	}
	if ((skill = pc_checkskill(sd, MT_TWOAXEDEF)) > 0 && sd->status.weapon == W_2HAXE) {
		uint8 defense_bonus[SZ_MAX][10] = {
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, // SZ_SMALL
			{ 2, 3, 5, 6, 8, 9, 11, 12, 14, 15 }, // SZ_MEDIUM
			{ 3, 5, 7, 9, 10, 12, 13, 15, 16, 18 }, // SZ_BIG
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } // SZ_ALL
		};

		for( uint8 size = SZ_SMALL; size < SZ_MAX; size++ ){
				sd->indexed_bonus.weapon_subsize[size] += defense_bonus[size][skill - 1];
		}
	}
	if ((skill = pc_checkskill(sd, ABC_DAGGER_AND_BOW_M)) > 0 && (sd->status.weapon == W_DAGGER || sd->status.weapon == W_BOW || sd->status.weapon == W_DOUBLE_DD || sd->status.weapon == W_DOUBLE_DS || sd->status.weapon == W_DOUBLE_DA)) {
		uint8 attack_bonus[SZ_MAX][10] = {
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, // SZ_SMALL
			{ 2, 3, 5, 6, 8, 9, 11, 12, 14, 15 }, // SZ_MEDIUM
			{ 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 }, // SZ_BIG
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } // SZ_ALL
		};

		for( uint8 size = SZ_SMALL; size < SZ_MAX; size++ ){
			sd->right_weapon.addsize[size] += attack_bonus[size][skill - 1];
			if( !battle_config.left_cardfix_to_right ){
				sd->left_weapon.addsize[size] += attack_bonus[size][skill - 1];
			}
		}
	}
	if ((skill = pc_checkskill(sd, ABC_MAGIC_SWORD_M)) > 0 && (sd->status.weapon == W_DAGGER || sd->status.weapon == W_1HSWORD || sd->status.weapon == W_DOUBLE_DD || sd->status.weapon == W_DOUBLE_SS || sd->status.weapon == W_DOUBLE_DS || sd->status.weapon == W_DOUBLE_DA || sd->status.weapon == W_DOUBLE_SA)) {
		uint8 attack_bonus[SZ_MAX][10] = {
			{ 2, 3, 5, 6, 8, 9, 11, 12, 14, 15 }, // SZ_SMALL
			{ 2, 3, 5, 6, 8, 9, 11, 12, 14, 15 }, // SZ_MEDIUM
			{ 2, 3, 5, 6, 8, 9, 11, 12, 14, 15 }, // SZ_BIG
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } // SZ_ALL
		};

		for( uint8 size = SZ_SMALL; size < SZ_MAX; size++ ){
			sd->indexed_bonus.magic_addsize[size] += attack_bonus[size][skill - 1];
		}
	}
	if ((skill = pc_checkskill(sd, EM_MAGIC_BOOK_M)) > 0 && sd->status.weapon == W_BOOK) {
		sd->indexed_bonus.magic_atk_ele[ELE_WATER] += skill;
		sd->indexed_bonus.magic_atk_ele[ELE_EARTH] += skill;
		sd->indexed_bonus.magic_atk_ele[ELE_FIRE] += skill;
		sd->indexed_bonus.magic_atk_ele[ELE_WIND] += skill;
		sd->indexed_bonus.magic_atk_ele[ELE_POISON] += skill;
	}

	if(sc->count) {
		if(sc->getSCE(SC_CONCENTRATE)) { // Update the card-bonus data
			sc->getSCE(SC_CONCENTRATE)->val3 = sd->indexed_bonus.param_bonus[1]; // Agi
			sc->getSCE(SC_CONCENTRATE)->val4 = sd->indexed_bonus.param_bonus[4]; // Dex
		}
		if(sc->getSCE(SC_SIEGFRIED)) {
			i = sc->getSCE(SC_SIEGFRIED)->val2;
			sd->indexed_bonus.subele[ELE_WATER] += i;
			sd->indexed_bonus.subele[ELE_EARTH] += i;
			sd->indexed_bonus.subele[ELE_FIRE] += i;
			sd->indexed_bonus.subele[ELE_WIND] += i;
#ifndef RENEWAL
			sd->indexed_bonus.subele[ELE_POISON] += i;
			sd->indexed_bonus.subele[ELE_HOLY] += i;
			sd->indexed_bonus.subele[ELE_DARK] += i;
			sd->indexed_bonus.subele[ELE_GHOST] += i;
			sd->indexed_bonus.subele[ELE_UNDEAD] += i;
#endif
		}
#ifdef RENEWAL
		if (sc->getSCE(SC_BASILICA)) {
			i = sc->getSCE(SC_BASILICA)->val1 * 5;
			sd->right_weapon.addele[ELE_DARK] += i;
			sd->right_weapon.addele[ELE_UNDEAD] += i;
			if( !battle_config.left_cardfix_to_right ){
				sd->left_weapon.addele[ELE_DARK] += i;
				sd->left_weapon.addele[ELE_UNDEAD] += i;
			}
			sd->indexed_bonus.magic_atk_ele[ELE_HOLY] += sc->getSCE(SC_BASILICA)->val1 * 3;
		}
		if (sc->getSCE(SC_FIREWEAPON))
			sd->indexed_bonus.magic_atk_ele[ELE_FIRE] += sc->getSCE(SC_FIREWEAPON)->val1;
		if (sc->getSCE(SC_WINDWEAPON))
			sd->indexed_bonus.magic_atk_ele[ELE_WIND] += sc->getSCE(SC_WINDWEAPON)->val1;
		if (sc->getSCE(SC_WATERWEAPON))
			sd->indexed_bonus.magic_atk_ele[ELE_WATER] += sc->getSCE(SC_WATERWEAPON)->val1;
		if (sc->getSCE(SC_EARTHWEAPON))
			sd->indexed_bonus.magic_atk_ele[ELE_EARTH] += sc->getSCE(SC_EARTHWEAPON)->val1;
#endif
		if(sc->getSCE(SC_PROVIDENCE)) {
			sd->indexed_bonus.subele[ELE_HOLY] += sc->getSCE(SC_PROVIDENCE)->val2;
			sd->indexed_bonus.subrace[RC_DEMON] += sc->getSCE(SC_PROVIDENCE)->val2;
		}
		if (sc->getSCE(SC_GEFFEN_MAGIC1)) {
			sd->right_weapon.addrace[RC_PLAYER_HUMAN] += sc->getSCE(SC_GEFFEN_MAGIC1)->val1;
			sd->right_weapon.addrace[RC_DEMIHUMAN] += sc->getSCE(SC_GEFFEN_MAGIC1)->val1;
			if( !battle_config.left_cardfix_to_right ){
				sd->left_weapon.addrace[RC_PLAYER_HUMAN] += sc->getSCE( SC_GEFFEN_MAGIC1 )->val1;
				sd->left_weapon.addrace[RC_DEMIHUMAN] += sc->getSCE( SC_GEFFEN_MAGIC1 )->val1;
			}
		}
		if (sc->getSCE(SC_GEFFEN_MAGIC2)) {
			sd->indexed_bonus.magic_addrace[RC_PLAYER_HUMAN] += sc->getSCE(SC_GEFFEN_MAGIC2)->val1;
			sd->indexed_bonus.magic_addrace[RC_DEMIHUMAN] += sc->getSCE(SC_GEFFEN_MAGIC2)->val1;
		}
		if(sc->getSCE(SC_GEFFEN_MAGIC3)) {
			sd->indexed_bonus.subrace[RC_PLAYER_HUMAN] += sc->getSCE(SC_GEFFEN_MAGIC3)->val1;
			sd->indexed_bonus.subrace[RC_DEMIHUMAN] += sc->getSCE(SC_GEFFEN_MAGIC3)->val1;
		}
		if(sc->getSCE(SC_ARMOR_ELEMENT_WATER)) {	// This status change should grant card-type elemental resist.
			sd->indexed_bonus.subele[ELE_WATER] += sc->getSCE(SC_ARMOR_ELEMENT_WATER)->val1;
			sd->indexed_bonus.subele[ELE_EARTH] += sc->getSCE(SC_ARMOR_ELEMENT_WATER)->val2;
			sd->indexed_bonus.subele[ELE_FIRE] += sc->getSCE(SC_ARMOR_ELEMENT_WATER)->val3;
			sd->indexed_bonus.subele[ELE_WIND] += sc->getSCE(SC_ARMOR_ELEMENT_WATER)->val4;
		}
		if(sc->getSCE(SC_ARMOR_ELEMENT_EARTH)) {	// This status change should grant card-type elemental resist.
			sd->indexed_bonus.subele[ELE_WATER] += sc->getSCE(SC_ARMOR_ELEMENT_EARTH)->val1;
			sd->indexed_bonus.subele[ELE_EARTH] += sc->getSCE(SC_ARMOR_ELEMENT_EARTH)->val2;
			sd->indexed_bonus.subele[ELE_FIRE] += sc->getSCE(SC_ARMOR_ELEMENT_EARTH)->val3;
			sd->indexed_bonus.subele[ELE_WIND] += sc->getSCE(SC_ARMOR_ELEMENT_EARTH)->val4;
		}
		if(sc->getSCE(SC_ARMOR_ELEMENT_FIRE)) {	// This status change should grant card-type elemental resist.
			sd->indexed_bonus.subele[ELE_WATER] += sc->getSCE(SC_ARMOR_ELEMENT_FIRE)->val1;
			sd->indexed_bonus.subele[ELE_EARTH] += sc->getSCE(SC_ARMOR_ELEMENT_FIRE)->val2;
			sd->indexed_bonus.subele[ELE_FIRE] += sc->getSCE(SC_ARMOR_ELEMENT_FIRE)->val3;
			sd->indexed_bonus.subele[ELE_WIND] += sc->getSCE(SC_ARMOR_ELEMENT_FIRE)->val4;
		}
		if(sc->getSCE(SC_ARMOR_ELEMENT_WIND)) {	// This status change should grant card-type elemental resist.
			sd->indexed_bonus.subele[ELE_WATER] += sc->getSCE(SC_ARMOR_ELEMENT_WIND)->val1;
			sd->indexed_bonus.subele[ELE_EARTH] += sc->getSCE(SC_ARMOR_ELEMENT_WIND)->val2;
			sd->indexed_bonus.subele[ELE_FIRE] += sc->getSCE(SC_ARMOR_ELEMENT_WIND)->val3;
			sd->indexed_bonus.subele[ELE_WIND] += sc->getSCE(SC_ARMOR_ELEMENT_WIND)->val4;
		}
		if(sc->getSCE(SC_ARMOR_RESIST)) { // Undead Scroll
			sd->indexed_bonus.subele[ELE_WATER] += sc->getSCE(SC_ARMOR_RESIST)->val1;
			sd->indexed_bonus.subele[ELE_EARTH] += sc->getSCE(SC_ARMOR_RESIST)->val2;
			sd->indexed_bonus.subele[ELE_FIRE] += sc->getSCE(SC_ARMOR_RESIST)->val3;
			sd->indexed_bonus.subele[ELE_WIND] += sc->getSCE(SC_ARMOR_RESIST)->val4;
		}
		if( sc->getSCE(SC_FIRE_CLOAK_OPTION) ) {
			i = sc->getSCE(SC_FIRE_CLOAK_OPTION)->val2;
			sd->indexed_bonus.subele[ELE_FIRE] += i;
			sd->indexed_bonus.subele[ELE_WATER] -= i;
		}
		if( sc->getSCE(SC_WATER_DROP_OPTION) ) {
			i = sc->getSCE(SC_WATER_DROP_OPTION)->val2;
			sd->indexed_bonus.subele[ELE_WATER] += i;
			sd->indexed_bonus.subele[ELE_WIND] -= i;
		}
		if( sc->getSCE(SC_WIND_CURTAIN_OPTION) ) {
			i = sc->getSCE(SC_WIND_CURTAIN_OPTION)->val2;
			sd->indexed_bonus.subele[ELE_WIND] += i;
			sd->indexed_bonus.subele[ELE_EARTH] -= i;
		}
		if( sc->getSCE(SC_STONE_SHIELD_OPTION) ) {
			i = sc->getSCE(SC_STONE_SHIELD_OPTION)->val2;
			sd->indexed_bonus.subele[ELE_EARTH] += i;
			sd->indexed_bonus.subele[ELE_FIRE] -= i;
		}
		if (sc->getSCE(SC_MTF_MLEATKED) )
			sd->indexed_bonus.subele[ELE_NEUTRAL] += sc->getSCE(SC_MTF_MLEATKED)->val3;
		if (sc->getSCE(SC_MTF_CRIDAMAGE))
			sd->bonus.crit_atk_rate += sc->getSCE(SC_MTF_CRIDAMAGE)->val1;
		if (sc->getSCE(SC_GLASTHEIM_ATK)) {
			sd->indexed_bonus.ignore_mdef_by_race[RC_UNDEAD] += sc->getSCE(SC_GLASTHEIM_ATK)->val1;
			sd->indexed_bonus.ignore_mdef_by_race[RC_DEMON] += sc->getSCE(SC_GLASTHEIM_ATK)->val1;
		}
		if (sc->getSCE(SC_LAUDARAMUS))
			sd->bonus.crit_atk_rate += 5 * sc->getSCE(SC_LAUDARAMUS)->val1;
#ifdef RENEWAL
		if (sc->getSCE(SC_FORTUNE))
			sd->bonus.crit_atk_rate += 2 * sc->getSCE(SC_FORTUNE)->val1;
#endif
		if (sc->getSCE(SC_SYMPHONYOFLOVER)) {
			sd->indexed_bonus.subele[ELE_GHOST] += sc->getSCE(SC_SYMPHONYOFLOVER)->val1 * 3;
			sd->indexed_bonus.subele[ELE_HOLY] += sc->getSCE(SC_SYMPHONYOFLOVER)->val1 * 3;
		}
		if (sc->getSCE(SC_PYREXIA) && sc->getSCE(SC_PYREXIA)->val3 == 0)
			sd->bonus.crit_atk_rate += sc->getSCE(SC_PYREXIA)->val2;
		if (sc->getSCE(SC_LUXANIMA)) {
			pc_bonus2(sd, SP_ADDSIZE, SZ_ALL, sc->getSCE(SC_LUXANIMA)->val3);
			sd->bonus.crit_atk_rate += sc->getSCE(SC_LUXANIMA)->val3;
			sd->bonus.short_attack_atk_rate += sc->getSCE(SC_LUXANIMA)->val3;
			sd->bonus.long_attack_atk_rate += sc->getSCE(SC_LUXANIMA)->val3;
		}
		if (sc->getSCE(SC_STRIKING))
			sd->bonus.perfect_hit += 20 + 10 * pc_checkskill(sd, SO_STRIKING);

		if( sc->getSCE( SC_RUSH_QUAKE2 ) ){
			sd->bonus.short_attack_atk_rate += 5 * sc->getSCE( SC_RUSH_QUAKE2 )->val1;
			sd->bonus.long_attack_atk_rate += 5 * sc->getSCE( SC_RUSH_QUAKE2 )->val1;
		}
		if (sc->getSCE(SC_HIDDEN_CARD))
			sd->bonus.long_attack_atk_rate += sc->getSCE(SC_HIDDEN_CARD)->val3;
		if (sc->getSCE(SC_DEADLY_DEFEASANCE))
			sd->special_state.no_magic_damage = 0;
		if (sc->getSCE(SC_CLIMAX_DES_HU))
			sd->indexed_bonus.magic_atk_ele[ELE_WIND] += 30;
		if (sc->getSCE(SC_CLIMAX_EARTH))
			sd->indexed_bonus.subele[ELE_EARTH] -= 100;
		if (sc->getSCE(SC_CLIMAX_BLOOM))
			sd->indexed_bonus.subele[ELE_FIRE] -= 100;
		if (sc->getSCE(SC_CLIMAX_CRYIMP)) {
			sd->indexed_bonus.subele[ELE_WATER] += 30;
			sd->indexed_bonus.magic_atk_ele[ELE_WATER] += 30;
		}
		if (sc->getSCE(SC_SINCERE_FAITH))
			sd->bonus.perfect_hit += sc->getSCE(SC_SINCERE_FAITH)->val3;
		if (sc->getSCE(SC_HOLY_S)) {
			sd->indexed_bonus.subele[ELE_DARK] += sc->getSCE(SC_HOLY_S)->val2;
			sd->indexed_bonus.subele[ELE_UNDEAD] += sc->getSCE(SC_HOLY_S)->val2;
			sd->indexed_bonus.magic_atk_ele[ELE_HOLY] += sc->getSCE(SC_HOLY_S)->val2;
		}
		if (sc->getSCE(SC_SUMMON_ELEMENTAL_ARDOR))
			sd->indexed_bonus.magic_atk_ele[ELE_FIRE] += 10;
		if (sc->getSCE(SC_SUMMON_ELEMENTAL_DILUVIO))
			sd->indexed_bonus.magic_atk_ele[ELE_WATER] += 10;
		if (sc->getSCE(SC_SUMMON_ELEMENTAL_PROCELLA))
			sd->indexed_bonus.magic_atk_ele[ELE_WIND] += 10;
		if (sc->getSCE(SC_SUMMON_ELEMENTAL_TERREMOTUS))
			sd->indexed_bonus.magic_atk_ele[ELE_EARTH] += 10;
		if (sc->getSCE(SC_SUMMON_ELEMENTAL_SERPENS))
			sd->indexed_bonus.magic_atk_ele[ELE_POISON] += 10;
		if (sc->getSCE(SC_FLAMEARMOR_OPTION)) {
			sd->indexed_bonus.subele[ELE_FIRE] += 100;
			sd->indexed_bonus.subele[ELE_WATER] -= 30;
		}
		if (sc->getSCE(SC_CRYSTAL_ARMOR_OPTION)) {
			sd->indexed_bonus.subele[ELE_WATER] += 100;
			sd->indexed_bonus.subele[ELE_WIND] -= 30;
		}
		if (sc->getSCE(SC_EYES_OF_STORM_OPTION)) {
			sd->indexed_bonus.subele[ELE_WIND] += 100;
			sd->indexed_bonus.subele[ELE_EARTH] -= 30;
		}
		if (sc->getSCE(SC_STRONG_PROTECTION_OPTION)) {
			sd->indexed_bonus.subele[ELE_EARTH] += 100;
			sd->indexed_bonus.subele[ELE_FIRE] -= 30;
		}
		if (sc->getSCE(SC_POISON_SHIELD_OPTION)) {
			sd->indexed_bonus.subele[ELE_POISON] += 100;
			sd->indexed_bonus.subele[ELE_HOLY] -= 30;
		}
		if (sc->getSCE(SC_INFINITY_DRINK)) {
			sd->bonus.crit_atk_rate += 5;
			sd->bonus.long_attack_atk_rate += 5;
			sd->indexed_bonus.magic_atk_ele[ELE_ALL] += 5;
			sd->special_state.no_castcancel = 1;
		}
		if(sc->getSCE(SC_MENTAL_POTION))
			sd->dsprate -= sc->getSCE(SC_MENTAL_POTION)->val1;
		if (sc->getSCE(SC_LIMIT_POWER_BOOSTER)) {
			pc_bonus(sd, SP_ATK_RATE, 1);
			pc_bonus(sd, SP_MATK_RATE, 1);
			sd->dsprate -= 5;
			sd->bonus.fixcastrate -= sc->getSCE(SC_LIMIT_POWER_BOOSTER)->val1;
		}
		if (sc->getSCE(SC_COMBAT_PILL)) {
			pc_bonus(sd, SP_ATK_RATE, sc->getSCE(SC_COMBAT_PILL)->val1);
			pc_bonus(sd, SP_MATK_RATE, sc->getSCE(SC_COMBAT_PILL)->val1);
		}
		if (sc->getSCE(SC_COMBAT_PILL2)) {
			pc_bonus(sd, SP_ATK_RATE, sc->getSCE(SC_COMBAT_PILL2)->val1);
			pc_bonus(sd, SP_MATK_RATE, sc->getSCE(SC_COMBAT_PILL2)->val1);
		}
		if (sc->getSCE(SC_SPARKCANDY))
			pc_bonus2(sd, SP_HP_LOSS_RATE, 100, 10000);
		if (sc->getSCE(SC_MAGICCANDY)) {
			sd->bonus.fixcastrate -= 70;
			sd->special_state.no_castcancel = 1;
			pc_bonus2(sd, SP_SP_LOSS_RATE, 90, 10000);
		}
		if (sc->getSCE(SC_POPECOOKIE)) {
			pc_bonus(sd, SP_ATK_RATE, sc->getSCE(SC_POPECOOKIE)->val1);
			pc_bonus(sd, SP_MATK_RATE, sc->getSCE(SC_POPECOOKIE)->val1);
			sd->indexed_bonus.subele[ELE_ALL] -= sc->getSCE(SC_POPECOOKIE)->val1;
		}
		if (sc->getSCE(SC_VITALIZE_POTION)) {
			pc_bonus(sd, SP_ATK_RATE, sc->getSCE(SC_VITALIZE_POTION)->val1);
			pc_bonus(sd, SP_MATK_RATE, sc->getSCE(SC_VITALIZE_POTION)->val1);
		}
		if (sc->getSCE(SC_CUP_OF_BOZA))
			sd->indexed_bonus.subele[ELE_FIRE] -= 5;
		if (sc->getSCE(SC_SKF_CAST))
			sd->bonus.varcastrate -= sc->getSCE(SC_SKF_CAST)->val1;
		if (sc->getSCE(SC_BEEF_RIB_STEW)) {
			sd->bonus.varcastrate -= 5;
			sd->dsprate -= 3;
		}
		if (sc->getSCE(SC_PORK_RIB_STEW))
			sd->dsprate -= 2;
		if (sc->getSCE(SC_BATH_FOAM_A)) {
			sd->right_weapon.addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_A)->val1;
			sd->indexed_bonus.magic_addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_A)->val1;
			if( !battle_config.left_cardfix_to_right ){
				sd->left_weapon.addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_A)->val1;
			}
		}
		if (sc->getSCE(SC_BATH_FOAM_B)) {
			sd->right_weapon.addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_B)->val1;
			sd->indexed_bonus.magic_addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_B)->val1;
			if( !battle_config.left_cardfix_to_right ){
				sd->left_weapon.addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_B)->val1;
			}
		}
		if (sc->getSCE(SC_BATH_FOAM_C)) {
			sd->right_weapon.addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_C)->val1;
			sd->indexed_bonus.magic_addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_C)->val1;
			if( !battle_config.left_cardfix_to_right ){
				sd->left_weapon.addrace2[RC2_EP172BATH] += sc->getSCE(SC_BATH_FOAM_C)->val1;
			}
		}
		if (sc->getSCE(SC_EP16_DEF)) {
			sd->indexed_bonus.subrace2[RC2_EP16_DEF] += sc->getSCE(SC_EP16_DEF)->val1;
		}
	}
	status_cpy(&sd->battle_status, base_status);

// ----- CLIENT-SIDE REFRESH -----
	if(!sd->bl.prev) {
		// Will update on LoadEndAck
		calculating = 0;
		return 0;
	}
	if(memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill))) {
#if PACKETVER_MAIN_NUM >= 20190807 || PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
		// Client doesn't delete unavailable skills even if we refresh the skill tree, individually delete them.
		for (i = 0; i < MAX_SKILL; i++) {
			if (b_skill[i].id != 0 && sd->status.skill[i].id == 0)
				clif_deleteskill(*sd, b_skill[i].id, true);
		}
#endif
		clif_skillinfoblock(sd);
	}

	// If the skill is learned, the status is infinite.
	if (pc_checkskill(sd, SU_SPRITEMABLE) > 0 && !sd->sc.getSCE(SC_SPRITEMABLE))
		sc_start(&sd->bl, &sd->bl, SC_SPRITEMABLE, 100, 1, INFINITE_TICK);
	if (pc_checkskill(sd, SU_SOULATTACK) > 0 && !sd->sc.getSCE(SC_SOULATTACK))
		sc_start(&sd->bl, &sd->bl, SC_SOULATTACK, 100, 1, INFINITE_TICK);

	calculating = 0;

	return 0;
}

/// Intermediate function since C++ does not have a try-finally syntax
int status_calc_pc_( map_session_data* sd, uint8 opt ){
	// Save the old script the player was attached to
	struct script_state* previous_st = sd->st;

	// Store the return value of the original function
	int ret = status_calc_pc_sub( sd, opt );

	// If an old script is present
	if( previous_st ){
		// Reattach the player to it, so that the limitations of that script kick back in
		script_attach_state( previous_st );
	}

	// Return the original return value
	return ret;
}

/**
 * Calculates Mercenary data
 * @param md: Mercenary object
 * @param opt: Whether it is first calc or not (0 on level up or status)
 * @return 0
 */
int status_calc_mercenary_(s_mercenary_data *md, uint8 opt)
{
	struct status_data *status = &md->base_status;
	s_mercenary *merc = &md->mercenary;

	if (opt&SCO_FIRST) {
		memcpy(status, &md->db->status, sizeof(struct status_data));
		status->class_ = CLASS_NORMAL;
		status->mode = static_cast<e_mode>(MD_CANMOVE|MD_CANATTACK);
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		md->battle_status.hp = merc->hp;
		md->battle_status.sp = merc->sp;
		if (md->master)
			status->speed = status_get_speed(&md->master->bl);
	}

	status_calc_misc(&md->bl, status, md->db->lv);
	status_cpy(&md->battle_status, status);

	return 0;
}

/**
 * Calculates Homunculus data
 * @param hd: Homunculus object
 * @param opt: Whether it is first calc or not (0 on level up or status)
 * @return 1
 */
int status_calc_homunculus_(struct homun_data *hd, uint8 opt)
{
	struct status_data *status = &hd->base_status;
	struct s_homunculus &hom = hd->homunculus;
	int skill_lv;
	int amotion;

	status->str = hom.str / 10;
	status->agi = hom.agi / 10;
	status->vit = hom.vit / 10;
	status->dex = hom.dex / 10;
	status->int_ = hom.int_ / 10;
	status->luk = hom.luk / 10;

	APPLY_HOMUN_LEVEL_STATWEIGHT();

	if (opt&SCO_FIRST) {
		const std::shared_ptr<s_homunculus_db> db = hd->homunculusDB;

		status->def_ele = db->element;
		status->ele_lv = 1;
		status->race = db->race;
		status->class_ = CLASS_NORMAL;
		status->size = (hom.class_ == db->evo_class) ? db->evo_size : db->base_size;
		status->rhw.range = 1 + status->size;
		status->mode = static_cast<e_mode>(MD_CANMOVE|MD_CANATTACK);
		status->speed = DEFAULT_WALK_SPEED;
		if (battle_config.hom_setting&HOMSET_COPY_SPEED && hd->master)
			status->speed = status_get_speed(&hd->master->bl);

		status->hp = 1;
		status->sp = 1;
	}

	status->aspd_rate = 1000;

#ifdef RENEWAL
	amotion = hd->homunculusDB->baseASPD;
	amotion = amotion - amotion * (status->dex + hom.dex_value) / 1000 - (status->agi + hom.agi_value) * amotion / 250;
	status->def = status->mdef = 0;
#else
	skill_lv = hom.level / 10 + status->vit / 5;
	status->def = cap_value(skill_lv, 0, 99);

	skill_lv = hom.level / 10 + status->int_ / 5;
	status->mdef = cap_value(skill_lv, 0, 99);

	amotion = (1000 - 4 * status->agi - status->dex) * hd->homunculusDB->baseASPD / 1000;
#endif

	status->amotion = cap_value(amotion, battle_config.max_aspd, 2000);
	status->adelay = status->amotion; //It seems adelay = amotion for Homunculus.

	status->max_hp = hom.max_hp;
	status->max_sp = hom.max_sp;

	hom_calc_skilltree(hd);

	if((skill_lv = hom_checkskill(hd, HAMI_SKIN)) > 0)
		status->def += skill_lv * 4;

	if((skill_lv = hom_checkskill(hd, HVAN_INSTRUCT)) > 0) {
		static const uint8 bonus_int[] = { 1, 2, 2, 4, 5 };
		static const uint8 bonus_str[] = { 1, 1, 3, 4, 4 };
		status->int_ += bonus_int[skill_lv - 1];
		status->str += bonus_str[skill_lv - 1];
	}

	if((skill_lv = hom_checkskill(hd, HAMI_SKIN)) > 0)
		status->max_hp += skill_lv * 2 * status->max_hp / 100;

	if((skill_lv = hom_checkskill(hd, HLIF_BRAIN)) > 0)
		status->max_sp += skill_lv * status->max_sp / 100;

	if((skill_lv = hom_checkskill(hd, MH_CLASSY_FLUTTER)) > 0) {
		status->matk_min += 100 + 60* skill_lv;
		status->matk_max += 100 + 60* skill_lv;
	}

	if((skill_lv = hom_checkskill(hd, MH_BRUSHUP_CLAW)) > 0) {
		status->batk += 100 + 60* skill_lv;
	}

	if((skill_lv = hom_checkskill(hd, MH_POLISHING_NEEDLE)) > 0) {
		status->matk_min += 50 + 20* skill_lv;
		status->matk_max += 50 + 20* skill_lv;
		status->batk += 100 + 40* skill_lv;
	}

	if((skill_lv = hom_checkskill(hd, MH_LICHT_GEHORN)) > 0) {
		status->matk_min += 100 + 30* skill_lv;
		status->matk_max += 100 + 30* skill_lv;
		status->batk += 100 + 30* skill_lv;
	}

	if((skill_lv = hom_checkskill(hd, MH_BLAZING_LAVA)) > 0) {
		status->batk += 100 + 60* skill_lv;
	}

	if (opt&SCO_FIRST) {
		hd->battle_status.hp = hom.hp;
		hd->battle_status.sp = hom.sp;
		if(hom.class_ == 6052) // Eleanor
			sc_start(&hd->bl,&hd->bl, SC_STYLE_CHANGE, 100, MH_MD_FIGHTING, INFINITE_TICK);
	}

#ifndef RENEWAL
	status->rhw.atk = status->dex;
	status->rhw.atk2 = status->str + hom.level;
#endif

	status_calc_misc(&hd->bl, status, hom.level);

	status_cpy(&hd->battle_status, status);
	return 1;
}

/**
 * Calculates Elemental data
 * @param ed: Elemental object
 * @param opt: Whether it is first calc or not (0 on status change)
 * @return 0
 */
int status_calc_elemental_(s_elemental_data *ed, uint8 opt)
{
	struct status_data *status = &ed->base_status;
	s_elemental *ele = &ed->elemental;
	map_session_data *sd = ed->master;

	if( !sd )
		return 0;

	if (opt&SCO_FIRST) {
		memcpy(status, &ed->db->status, sizeof(struct status_data));
		if( !ele->mode )
			status->mode = EL_MODE_PASSIVE;
		else
			status->mode = ele->mode;

		status->class_ = CLASS_NORMAL;
		status_calc_misc(&ed->bl, status, 0);

		status->max_hp = ele->max_hp;
		status->max_sp = ele->max_sp;
		status->hp = ele->hp;
		status->sp = ele->sp;
		status->rhw.atk = ele->atk;
		status->rhw.atk2 = ele->atk2;
		status->matk_min += ele->matk;
		status->def += ele->def;
		status->mdef += ele->mdef;
		status->flee = ele->flee;
		status->hit = ele->hit;

		if (ed->master)
			status->speed = status_get_speed(&ed->master->bl);

		memcpy(&ed->battle_status,status,sizeof(struct status_data));
	} else {
		status_calc_misc(&ed->bl, status, 0);
		status_cpy(&ed->battle_status, status);
	}

	return 0;
}

/**
 * Calculates NPC data
 * @param nd: NPC object
 * @param opt: Whether it is first calc or not (what?)
 * @return 0
 */
int status_calc_npc_(struct npc_data *nd, uint8 opt)
{
	struct status_data *status = &nd->status;

	if (!nd)
		return 0;

	if (opt&SCO_FIRST) {
		status->hp = 1;
		status->sp = 1;
		status->max_hp = 1;
		status->max_sp = 1;

		status->def_ele = ELE_NEUTRAL;
		status->ele_lv = 1;
		status->race = RC_DEMIHUMAN;
		status->class_ = CLASS_NORMAL;
		status->size = nd->size;
		status->rhw.range = 1 + status->size;
		status->mode = static_cast<e_mode>(MD_CANMOVE|MD_CANATTACK);
		status->speed = nd->speed;
	}

	status->str = nd->stat_point + nd->params.str;
	status->agi = nd->stat_point + nd->params.agi;
	status->vit = nd->stat_point + nd->params.vit;
	status->int_= nd->stat_point + nd->params.int_;
	status->dex = nd->stat_point + nd->params.dex;
	status->luk = nd->stat_point + nd->params.luk;

	status_calc_misc(&nd->bl, status, nd->level);
	status_cpy(&nd->status, status);

	return 0;
}

/**
 * Calculates regeneration values
 * Applies passive skill regeneration additions
 * @param bl: Object to calculate regen for [PC|HOM|MER|ELEM]
 * @param status: Object's status
 * @param regen: Object's base regeneration data
 */
void status_calc_regen(struct block_list *bl, struct status_data *status, struct regen_data *regen)
{
	map_session_data *sd;
	status_change *sc;
	int val, skill, reg_flag;

	if( !(bl->type&BL_REGEN) || !regen )
		return;

	sd = BL_CAST(BL_PC,bl);
	sc = status_get_sc(bl);

	val = (status->vit/5) + max(1, status->max_hp/200);

	if( sd && sd->hprecov_rate != 100 )
		val = val*sd->hprecov_rate/100;

	reg_flag = bl->type == BL_PC ? 0 : 1;

	regen->hp = cap_value(val, reg_flag, SHRT_MAX);

	val = 1 + (status->int_/6) + (status->max_sp/100);
	if( status->int_ >= 120 )
		val += ((status->int_-120) / 2) + 4;

	if( sd && sd->sprecov_rate != 100 )
		val = val*sd->sprecov_rate/100;

	regen->sp = cap_value(val, reg_flag, SHRT_MAX);

	if( sd ) {
		struct regen_data_sub *sregen;
		if( (skill=pc_checkskill(sd,HP_MEDITATIO)) > 0 ) {
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
		// Only players have skill/sitting skill regen for now.
		sregen = regen->sregen;

		val = 0;
		if( (skill=pc_checkskill(sd,SM_RECOVERY)) > 0 )
			val += skill*5 + skill*status->max_hp/500;

		if (sc && sc->count) {
			if (sc->getSCE(SC_INCREASE_MAXHP))
				val += val * sc->getSCE(SC_INCREASE_MAXHP)->val2 / 100;
		}

		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=pc_checkskill(sd,MG_SRECOVERY)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		if( (skill=pc_checkskill(sd,NJ_NINPOU)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		if( (skill=pc_checkskill(sd,WM_LESSON)) > 0 )
			val += 3 + 3 * skill;

		if (sc && sc->count) {
			if (sc->getSCE(SC_ANCILLA))
				val += sc->getSCE(SC_ANCILLA)->val2 / 100;
			if (sc->getSCE(SC_INCREASE_MAXSP))
				val += val * sc->getSCE(SC_INCREASE_MAXSP)->val2 / 100;
		}

		sregen->sp = cap_value(val, 0, SHRT_MAX);

		// Skill-related recovery (only when sit)
		sregen = regen->ssregen;

		val = 0;
		if( (skill=pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*4 + skill*status->max_hp/500;

		if( (skill=pc_checkskill(sd,TK_HPTIME)) > 0 && sd->state.rest )
			val += skill*30 + skill*status->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=pc_checkskill(sd,TK_SPTIME)) > 0 && sd->state.rest ) {
			val += skill*3 + skill*status->max_sp/500;
			if ((skill=pc_checkskill(sd,SL_KAINA)) > 0) // Power up Enjoyable Rest
				val += (30+10*skill)*val/100;
		}
		if( (skill=pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*2 + skill*status->max_sp/500;
		sregen->sp = cap_value(val, 0, SHRT_MAX);
	}

	if( bl->type == BL_HOM ) {
		struct homun_data *hd = (TBL_HOM*)bl;
		if( (skill = hom_checkskill(hd,HAMI_SKIN)) > 0 ) {
			val = regen->hp*(100+5*skill)/100;
			regen->hp = cap_value(val, 1, SHRT_MAX);
		}
		if( (skill = hom_checkskill(hd,HLIF_BRAIN)) > 0 ) {
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
	} else if( bl->type == BL_MER ) {
		val = static_cast<decltype(val)>((status->max_hp * status->vit / 10000.0 + 1.0) * 6.0);
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = static_cast<decltype(val)>((status->max_sp * (status->int_ + 10.0) / 750.0) + 1.0);
		regen->sp = cap_value(val, 1, SHRT_MAX);
	} else if( bl->type == BL_ELEM ) {
		val = static_cast<decltype(val)>((status->max_hp * status->vit / 10000.0 + 1.0) * 6.0);
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = static_cast<decltype(val)>((status->max_sp * (status->int_ + 10.0) / 750.0) + 1.0);
		regen->sp = cap_value(val, 1, SHRT_MAX);
	}
}

/**
 * Calculates SC (Status Changes) regeneration values
 * @param bl: Object to calculate regen for [PC|HOM|MER|ELEM]
 * @param regen: Object's base regeneration data
 * @param sc: Object's status change data
 */
void status_calc_regen_rate(struct block_list *bl, struct regen_data *regen, status_change *sc)
{
	if (!(bl->type&BL_REGEN) || !regen)
		return;

	regen->flag = RGN_HP|RGN_SP;
	if(regen->sregen) {
		if (regen->sregen->hp)
			regen->flag |= RGN_SHP;

		if (regen->sregen->sp)
			regen->flag |= RGN_SSP;
		regen->sregen->rate.hp = regen->sregen->rate.sp = 100;
	}
	if (regen->ssregen) {
		if (regen->ssregen->hp)
			regen->flag |= RGN_SHP;

		if (regen->ssregen->sp)
			regen->flag |= RGN_SSP;
		regen->ssregen->rate.hp = regen->ssregen->rate.sp = 100;
	}
	regen->rate.hp = regen->rate.sp = 100;

	if (!sc || !sc->count)
		return;

	// No HP or SP regen
	if ((sc->getSCE(SC_POISON) && !sc->getSCE(SC_SLOWPOISON))
		|| (sc->getSCE(SC_DPOISON) && !sc->getSCE(SC_SLOWPOISON))
		|| sc->getSCE(SC_BERSERK)
		|| sc->getSCE(SC_TRICKDEAD)
		|| sc->getSCE(SC_BLEEDING)
		|| (sc->getSCE(SC_MAGICMUSHROOM) && sc->getSCE(SC_MAGICMUSHROOM)->val3 == 1)
		|| sc->getSCE(SC_SATURDAYNIGHTFEVER)
		|| sc->getSCE(SC_REBOUND))
		regen->flag = RGN_NONE;

	// No natural SP regen
	if (sc->getSCE(SC_DANCING) ||
#ifdef RENEWAL
		sc->getSCE(SC_MAXIMIZEPOWER) ||
#endif
#ifndef RENEWAL
		(bl->type == BL_PC && (((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK &&
		(sc->getSCE(SC_EXTREMITYFIST) || sc->getSCE(SC_EXPLOSIONSPIRITS)) && (!sc->getSCE(SC_SPIRIT) || sc->getSCE(SC_SPIRIT)->val2 != SL_MONK)) ||
#else
		(bl->type == BL_PC && (((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK &&
		sc->getSCE(SC_EXTREMITYFIST) && (!sc->getSCE(SC_SPIRIT) || sc->getSCE(SC_SPIRIT)->val2 != SL_MONK)) ||
#endif
		(sc->getSCE(SC_OBLIVIONCURSE) && sc->getSCE(SC_OBLIVIONCURSE)->val3 == 1))
		regen->flag &= ~RGN_SP;

	if (sc->getSCE(SC_TENSIONRELAX)) {
		if (sc->getSCE(SC_WEIGHT50) || sc->getSCE(SC_WEIGHT90))
			regen->state.overweight = 0; // 1x HP regen
		else {
			regen->rate.hp += 200;
			if (regen->sregen)
				regen->sregen->rate.hp += 200;
		}
	}

	if (sc->getSCE(SC_MAGNIFICAT))
		regen->rate.sp += 100;

	if (sc->getSCE(SC_REGENERATION)) {
		const struct status_change_entry *sce = sc->getSCE(SC_REGENERATION);
		if (!sce->val4) {
			regen->rate.hp += (sce->val2*100);
			regen->rate.sp += (sce->val3*100);
		} else
			regen->flag &= ~sce->val4; // Remove regen as specified by val4
	}
	if(sc->getSCE(SC_GT_REVITALIZE)) {
		regen->hp += cap_value(regen->hp * sc->getSCE(SC_GT_REVITALIZE)->val3/100, 1, SHRT_MAX);
		regen->state.walk = 1;
	}
	if (sc->getSCE(SC_EXTRACT_WHITE_POTION_Z))
		regen->hp += cap_value(regen->hp * sc->getSCE(SC_EXTRACT_WHITE_POTION_Z)->val1 / 100, 1, SHRT_MAX);
	if (sc->getSCE(SC_VITATA_500))
		regen->sp += cap_value(regen->sp * sc->getSCE(SC_VITATA_500)->val1 / 100, 1, SHRT_MAX);
	if (bl->type == BL_ELEM) { // Recovery bonus only applies to the Elementals.
		int ele_class = status_get_class(bl);

		switch (ele_class) {
		case ELEMENTALID_AGNI_S:
		case ELEMENTALID_AGNI_M:
		case ELEMENTALID_AGNI_L:
		case ELEMENTALID_ARDOR:
			if (sc->getSCE(SC_FIRE_INSIGNIA) && sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 1)
				regen->rate.hp += 100;
			break;
		case ELEMENTALID_AQUA_S:
		case ELEMENTALID_AQUA_M:
		case ELEMENTALID_AQUA_L:
		case ELEMENTALID_DILUVIO:
			if (sc->getSCE(SC_WATER_INSIGNIA) && sc->getSCE(SC_WATER_INSIGNIA)->val1 == 1)
				regen->rate.hp += 100;
			break;
		case ELEMENTALID_VENTUS_S:
		case ELEMENTALID_VENTUS_M:
		case ELEMENTALID_VENTUS_L:
		case ELEMENTALID_PROCELLA:
			if (sc->getSCE(SC_WIND_INSIGNIA) && sc->getSCE(SC_WIND_INSIGNIA)->val1 == 1)
				regen->rate.hp += 100;
			break;
		case ELEMENTALID_TERA_S:
		case ELEMENTALID_TERA_M:
		case ELEMENTALID_TERA_L:
		case ELEMENTALID_TERREMOTUS:
		case ELEMENTALID_SERPENS:
			if (sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 1)
				regen->rate.hp += 100;
			break;
		}
	}

	if (sc->getSCE(SC_CATNIPPOWDER)) {
		regen->rate.hp *= 2;
		regen->rate.sp *= 2;
	}
	if (sc->getSCE(SC_SHRIMPBLESSING))
		regen->rate.sp += 50;
#ifdef RENEWAL
	if (sc->getSCE(SC_NIBELUNGEN)) {
		if (sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_HPREGEN)
			regen->rate.hp += 100;
		else if (sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_SPREGEN)
			regen->rate.sp += 100;
	}
#endif
	if (sc->getSCE(SC_SIRCLEOFNATURE))
		regen->rate.hp += sc->getSCE(SC_SIRCLEOFNATURE)->val2;
	if (sc->getSCE(SC_SONGOFMANA))
		regen->rate.sp += sc->getSCE(SC_SONGOFMANA)->val3;
	if (sc->getSCE(SC_BUCHEDENOEL)) {
		regen->rate.hp += sc->getSCE(SC_BUCHEDENOEL)->val2;
		regen->rate.sp += sc->getSCE(SC_BUCHEDENOEL)->val2;
	}
}

/**
 * Applies a state to a unit - See [StatusChangeStateTable]
 * @param bl: Object to change state on [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change data
 * @param flag: Which state to apply to bl
 * @param start: (1) start state, (0) remove state
 */
void status_calc_state( struct block_list *bl, status_change *sc, std::bitset<SCS_MAX> flag, bool start )
{

	/// No sc at all, we can zero without any extra weight over our conciousness
	if( !sc->count ) {
		sc->cant = {};
		return;
	}

	// Can't move
	if( flag[SCS_NOMOVE] ) {
		if( !flag[SCS_NOMOVECOND] )
			sc->cant.move += (start ? 1 : ((sc->cant.move) ? -1 : 0));
		else if(
				     (sc->getSCE(SC_GOSPEL) && sc->getSCE(SC_GOSPEL)->val4 == BCT_SELF)	// cannot move while gospel is in effect
#ifndef RENEWAL
				  || (sc->getSCE(SC_BASILICA) && sc->getSCE(SC_BASILICA)->val4 == bl->id) // Basilica caster cannot move
				  || (sc->getSCE(SC_GRAVITATION) && sc->getSCE(SC_GRAVITATION)->val3 == BCT_SELF)
#endif
				  || (sc->getSCE(SC_CAMOUFLAGE) && sc->getSCE(SC_CAMOUFLAGE)->val1 < 3)
				  || (sc->getSCE(SC_MAGNETICFIELD) && sc->getSCE(SC_MAGNETICFIELD)->val2 != bl->id)
				  || (sc->getSCE(SC_FEAR) && sc->getSCE(SC_FEAR)->val2 > 0)
				  || (sc->getSCE(SC_HIDING) && (bl->type != BL_PC || (pc_checkskill(BL_CAST(BL_PC,bl),RG_TUNNELDRIVE) <= 0)))
				  || (sc->getSCE(SC_DANCING) && sc->getSCE(SC_DANCING)->val4 && (
#ifndef RENEWAL
						!sc->getSCE(SC_LONGING) ||
#endif
						(sc->getSCE(SC_DANCING)->val1&0xFFFF) == CG_MOONLIT ||
						(sc->getSCE(SC_DANCING)->val1&0xFFFF) == CG_HERMODE
						))
				  || (sc->getSCE(SC_CRYSTALIZE) && bl->type != BL_MOB)
 				 )
				 sc->cant.move += (start ? 1 : ((sc->cant.move) ? -1 : 0));
	}

	// Can't use skills
	if( flag[SCS_NOCAST] ) {
		if( !flag[SCS_NOCASTCOND] )
			sc->cant.cast += (start ? 1 : ((sc->cant.cast) ? -1 : 0));
		else if (sc->getSCE(SC_OBLIVIONCURSE) && sc->getSCE(SC_OBLIVIONCURSE)->val3 == 1)
			sc->cant.cast += (start ? 1 : ((sc->cant.cast) ? -1 : 0));
	}

	// Can't chat
	if( flag[SCS_NOCHAT] ) {
		if( !flag[SCS_NOCHATCOND] )
			sc->cant.chat += (start ? 1 : ((sc->cant.chat) ? -1 : 0));
		else if(sc->getSCE(SC_NOCHAT) && sc->getSCE(SC_NOCHAT)->val1&MANNER_NOCHAT)
			sc->cant.chat += (start ? 1 : ((sc->cant.chat) ? -1 : 0));
	}

	// Can't attack
	if( flag[SCS_NOATTACK] ) {
		if( !flag[SCS_NOATTACKCOND] )
			sc->cant.attack += (start ? 1 : ((sc->cant.attack) ? -1 : 0));
		/*else if( )
			sc->cant.attack += ( start ? 1 : ((sc->cant.attack)? -1:0) );*/
	}

	// Can't warp
	if (flag[SCS_NOWARP]) {
		if (!flag[SCS_NOWARPCOND])
			sc->cant.warp += (start ? 1 : ((sc->cant.warp) ? -1 : 0));
		/*else if (sc->getSCE())
			sc->cant.warp += ( start ? 1 : ((sc->cant.warp)? -1:0) );*/
	}

	// Player-only states
	if( bl->type == BL_PC ) {
		// Can't pick-up items
		if( flag[SCS_NOPICKITEM] ) {
			if( !flag[SCS_NOPICKITEMCOND] )
				sc->cant.pickup += (start ? 1 : ((sc->cant.pickup) ? -1 : 0));
			else if( (sc->getSCE(SC_NOCHAT) && sc->getSCE(SC_NOCHAT)->val1&MANNER_NOITEM) )
				sc->cant.pickup += (start ? 1 : ((sc->cant.pickup) ? -1 : 0));
		}

		// Can't drop items
		if( flag[SCS_NODROPITEM] ) {
			if( !flag[SCS_NODROPITEMCOND] )
				sc->cant.drop += (start ? 1 : ((sc->cant.drop) ? -1 : 0));
			else if( (sc->getSCE(SC_NOCHAT) && sc->getSCE(SC_NOCHAT)->val1&MANNER_NOITEM) )
				sc->cant.drop += (start ? 1 : ((sc->cant.drop) ? -1 : 0));
		}

		// Can't equip item
		if( flag[SCS_NOEQUIPITEM] ) {
			if( !flag[SCS_NOEQUIPITEMCOND] )
				sc->cant.equip += (start ? 1 : ((sc->cant.equip) ? -1 : 0));
			/*else if(  )
				sc->cant.equip += ( start ? 1 : ((sc->cant.equip)? -1:0) );*/
		}

		// Can't unequip item
		if( flag[SCS_NOUNEQUIPITEM]) {
			if( !flag[SCS_NOUNEQUIPITEMCOND] )
				sc->cant.unequip += (start ? 1 : ((sc->cant.unequip) ? -1 : 0));
			/*else if(  )
				sc->cant.unequip += ( start ? 1 : ((sc->cant.unequip)? -1:0) );*/
		}

		// Can't consume item
		if( flag[SCS_NOCONSUMEITEM]) {
			if( !flag[SCS_NOCONSUMEITEMCOND] )
				sc->cant.consume += (start ? 1 : ((sc->cant.consume) ? -1 : 0));
			else if( (sc->getSCE(SC_GRAVITATION) && sc->getSCE(SC_GRAVITATION)->val3 == BCT_SELF) ||
				 (sc->getSCE(SC_NOCHAT) && sc->getSCE(SC_NOCHAT)->val1&MANNER_NOITEM) )
				sc->cant.consume += (start ? 1 : ((sc->cant.consume) ? -1 : 0));
		}

		// Can't lose exp
		if (flag[SCS_NODEATHPENALTY]) {
			if (!flag[SCS_NODEATHPENALTYCOND])
				sc->cant.deathpenalty += (start ? 1 : ((sc->cant.deathpenalty) ? -1 : 0));
			/*else if (sc->getSCE())
				sc->cant.deathpenalty += ( start ? 1 : ((sc->cant.deathpenalty)? -1:0) );*/
		}

		// Can't sit/stand/talk to NPC
		if (flag[SCS_NOINTERACT]) {
			if (!flag[SCS_NOINTERACTCOND])
				sc->cant.interact += (start ? 1 : ((sc->cant.interact) ? -1 : 0));
			/*else if (sc->getSCE())
				sc->cant.interact += ( start ? 1 : ((sc->cant.interact)? -1:0) );*/
		}
	}

	return;
}

/**
 * Recalculates parts of an objects status according to specified flags
 * See [set_sc] [add_sc]
 * @param bl: Object whose status has changed [PC|MOB|HOM|MER|ELEM]
 * @param flag: Which status has changed on bl
 */
void status_calc_bl_main(struct block_list& bl, std::bitset<SCB_MAX> flag)
{
	const struct status_data *b_status = status_get_base_status(&bl); // Base Status
	status_data* status = status_get_status_data(bl); // Battle Status
	status_change *sc = status_get_sc(&bl);
	TBL_PC *sd = BL_CAST(BL_PC,&bl);
	int temp;

	if (!b_status || !status)
		return;

	/** [Playtester]
	* This needs to be done even if there is currently no status change active, because
	* we need to update the speed on the client when the last status change ends.
	**/
	if(flag[SCB_SPEED]) {
		struct unit_data *ud = unit_bl2ud(&bl);
		/** [Skotlex]
		* Re-walk to adjust speed (we do not check if walktimer != INVALID_TIMER
		* because if you step on something while walking, the moment this
		* piece of code triggers the walk-timer is set on INVALID_TIMER)
		**/
		if (ud)
			ud->state.change_walk_target = 1;
	}

	if(flag[SCB_STR]) {
		status->str = status_calc_str(&bl, sc, b_status->str);
		flag.set(SCB_BATK);
		if( bl.type == BL_HOM )
			flag.set(SCB_WATK);
	}

	if(flag[SCB_AGI]) {
		status->agi = status_calc_agi(&bl, sc, b_status->agi);
		flag.set(SCB_FLEE);
#ifdef RENEWAL
		flag.set(SCB_DEF2);
#endif
		if( bl.type&(BL_PC|BL_HOM) ) {
			flag.set(SCB_ASPD);
			flag.set(SCB_DSPD);
		}
	}

	if(flag[SCB_VIT]) {
		status->vit = status_calc_vit(&bl, sc, b_status->vit);
		flag.set(SCB_DEF2);
		flag.set(SCB_MDEF2);
		if( bl.type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag.set(SCB_MAXHP);
		if( bl.type == BL_HOM )
			flag.set(SCB_DEF);
	}

	if(flag[SCB_INT]) {
		status->int_ = status_calc_int(&bl, sc, b_status->int_);
		flag.set(SCB_MATK);
		flag.set(SCB_MDEF2);
		if( bl.type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag.set(SCB_MAXSP);
		if( bl.type == BL_HOM )
			flag.set(SCB_MDEF);
	}

	if(flag[SCB_DEX]) {
		status->dex = status_calc_dex(&bl, sc, b_status->dex);
		flag.set(SCB_BATK);
		flag.set(SCB_HIT);
#ifdef RENEWAL
		flag.set(SCB_MATK);
		flag.set(SCB_MDEF2);
#endif
		if( bl.type&(BL_PC|BL_HOM) )
			flag.set(SCB_ASPD);
		if( bl.type == BL_HOM )
			flag.set(SCB_WATK);
	}

	if(flag[SCB_LUK]) {
		status->luk = status_calc_luk(&bl, sc, b_status->luk);
		flag.set(SCB_BATK);
		flag.set(SCB_CRI);
		flag.set(SCB_FLEE2);
#ifdef RENEWAL
		flag.set(SCB_MATK);
		flag.set(SCB_HIT);
		flag.set(SCB_FLEE);
#endif
	}

#ifdef RENEWAL
	if (flag[SCB_POW]) {
		status->pow = status_calc_pow(&bl, sc, b_status->pow);
		flag.set(SCB_BATK);
		flag.set(SCB_PATK);
	}

	if (flag[SCB_STA]) {
		status->sta = status_calc_sta(&bl, sc, b_status->sta);
		flag.set(SCB_RES);
	}

	if (flag[SCB_WIS]) {
		status->wis = status_calc_wis(&bl, sc, b_status->wis);
		flag.set(SCB_MRES);
	}

	if (flag[SCB_SPL]) {
		status->spl = status_calc_spl(&bl, sc, b_status->spl);
		flag.set(SCB_MATK);
		flag.set(SCB_SMATK);
	}

	if (flag[SCB_CON]) {
		status->con = status_calc_con(&bl, sc, b_status->con);
		flag.set(SCB_HIT);
		flag.set(SCB_FLEE);
		flag.set(SCB_PATK);
		flag.set(SCB_SMATK);
	}

	if (flag[SCB_CRT]) {
		status->crt = status_calc_crt(&bl, sc, b_status->crt);
		flag.set(SCB_HPLUS);
		flag.set(SCB_CRATE);
	}
#endif

	if(flag[SCB_BATK] && b_status->batk) {
		int lv = status_get_lv(&bl);
		status->batk = status_base_atk(&bl, status, lv);
		temp = b_status->batk - status_base_atk(&bl, b_status, lv);
		if (temp) {
			temp += status->batk;
			status->batk = cap_value(temp, 0, USHRT_MAX);
		}
		status->batk = status_calc_batk(&bl, sc, status->batk);
	}

	if(flag[SCB_WATK]) {
#ifndef RENEWAL
		status->rhw.atk = status_calc_watk(&bl, sc, b_status->rhw.atk);
		if (!sd) // Should not affect weapon refine bonus
			status->rhw.atk2 = status_calc_watk(&bl, sc, b_status->rhw.atk2);

		if (sd && sd->bonus.weapon_atk_rate)
			status->rhw.atk += status->rhw.atk * sd->bonus.weapon_atk_rate / 100;
		if(b_status->lhw.atk) {
			if (sd) {
				sd->state.lr_flag = 1;
				status->lhw.atk = status_calc_watk(&bl, sc, b_status->lhw.atk);
				sd->state.lr_flag = 0;
			} else {
				status->lhw.atk = status_calc_watk(&bl, sc, b_status->lhw.atk);
				status->lhw.atk2= status_calc_watk(&bl, sc, b_status->lhw.atk2);
			}
		}
#else
		if(!b_status->watk) { // We only have left-hand weapon
			status->watk = 0;
			status->watk2 = status_calc_watk(&bl, sc, b_status->watk2);
		}
		else status->watk = status_calc_watk(&bl, sc, b_status->watk);
#endif
	}

	if(flag[SCB_HIT]) {
		if (status->dex == b_status->dex
#ifdef RENEWAL
			&& status->luk == b_status->luk && status->con == b_status->con
#endif
			)
			status->hit = status_calc_hit(&bl, sc, b_status->hit);
		else
			status->hit = status_calc_hit(&bl, sc, b_status->hit + (status->dex - b_status->dex)
#ifdef RENEWAL
			 + (status->luk/3 - b_status->luk/3) + 2 * (status->con - b_status->con)
#endif
			 );
	}

	if(flag[SCB_FLEE]) {
		if (status->agi == b_status->agi
#ifdef RENEWAL
			&& status->luk == b_status->luk && status->con == b_status->con
#endif
			)
			status->flee = status_calc_flee(&bl, sc, b_status->flee);
		else
			status->flee = status_calc_flee(&bl, sc, b_status->flee +(status->agi - b_status->agi)
#ifdef RENEWAL
			+ (status->luk/5 - b_status->luk/5) + 2 * (status->con - b_status->con)
#endif
			);
	}

	if(flag[SCB_DEF]) {
		status->def = status_calc_def(&bl, sc, b_status->def);

		if( bl.type == BL_HOM )
			status->def += (status->vit/5 - b_status->vit/5);
	}

	if(flag[SCB_DEF2]) {
		if (status->vit == b_status->vit
#ifdef RENEWAL
			&& status->agi == b_status->agi
#endif
			)
			status->def2 = status_calc_def2(&bl, sc, b_status->def2);
		else
			status->def2 = status_calc_def2(&bl, sc, b_status->def2
#ifdef RENEWAL
			+ (int)( ((float)status->vit/2 - (float)b_status->vit/2) + ((float)status->agi/5 - (float)b_status->agi/5) )
#else
			+ (status->vit - b_status->vit)
#endif
		);
	}

	if(flag[SCB_MDEF]) {
		status->mdef = status_calc_mdef(&bl, sc, b_status->mdef);

		if( bl.type == BL_HOM )
			status->mdef += (status->int_/5 - b_status->int_/5);
	}

	if(flag[SCB_MDEF2]) {
		if (status->int_ == b_status->int_ && status->vit == b_status->vit
#ifdef RENEWAL
			&& status->dex == b_status->dex
#endif
			)
			status->mdef2 = status_calc_mdef2(&bl, sc, b_status->mdef2);
		else
			status->mdef2 = status_calc_mdef2(&bl, sc, b_status->mdef2 +(status->int_ - b_status->int_)
#ifdef RENEWAL
			+ (int)( ((float)status->dex/5 - (float)b_status->dex/5) + ((float)status->vit/5 - (float)b_status->vit/5) )
#else
			+ ((status->vit - b_status->vit) / 2)
#endif
			);
	}

	if(flag[SCB_SPEED]) {
		status->speed = status_calc_speed(&bl, sc, b_status->speed);

		switch (bl.type) {
		case BL_PC:
			if (!sd->state.permanent_speed && status->speed < battle_config.max_walk_speed)
				status->speed = battle_config.max_walk_speed;
#ifdef RENEWAL
			// Recalculate homunculus speed if the player receives a speed buff/debuff
			if (hom_is_active(sd->hd)) {
				if (battle_config.hom_setting & HOMSET_COPY_SPEED)
					sd->hd->battle_status.speed = status_get_speed(&sd->bl);

				// Homunculus speed buff/debuffs applies over the current speed
				sd->hd->battle_status.speed = status_calc_speed(&sd->hd->bl, &sd->hd->sc, sd->hd->battle_status.speed);
			}
#endif
			break;

		case BL_PET:{
			pet_data* pd = reinterpret_cast<pet_data*>(&bl);

			if (pd->master != nullptr)
				status->speed = status_get_speed(&pd->master->bl);
		} break;

		case BL_HOM:{
			homun_data* hd = reinterpret_cast<homun_data*>(&bl);

			if (hd->master != nullptr) {
				if (battle_config.hom_setting & HOMSET_COPY_SPEED)
					status->speed = status_get_speed(&hd->master->bl);

				// Homunculus speed buff/debuffs applies over the current speed
				status->speed = status_calc_speed(&bl, &hd->sc, status->speed);
			}
		} break;

		case BL_MER:{
			s_mercenary_data* mc = reinterpret_cast<s_mercenary_data*>(&bl);

			if (mc->master != nullptr)
				status->speed = status_get_speed(&mc->master->bl);
		} break;

		case BL_ELEM:{
			s_elemental_data* ed = reinterpret_cast<s_elemental_data*>(&bl);

			if (ed->master != nullptr)
				status->speed = status_get_speed(&ed->master->bl);
		} break;
		}
	}

	if(flag[SCB_CRI] && b_status->cri) {
		if (status->luk == b_status->luk)
			status->cri = status_calc_critical(&bl, sc, b_status->cri);
		else
#ifdef RENEWAL
			status->cri = status_calc_critical(&bl, sc, b_status->cri + 3*(status->luk - b_status->luk));
#else
			status->cri = status_calc_critical(&bl, sc, b_status->cri + (status->luk - b_status->luk)*10/3);
#endif

		/// After status_calc_critical so the bonus is applied despite if you have or not a sc bugreport:5240
		if (sd) {
			if (sd->status.weapon == W_KATAR)
				status->cri *= 2;
		}
	}

	if(flag[SCB_FLEE2] && b_status->flee2) {
		if (status->luk == b_status->luk)
			status->flee2 = status_calc_flee2(&bl, sc, b_status->flee2);
		else
			status->flee2 = status_calc_flee2(&bl, sc, b_status->flee2 +(status->luk - b_status->luk));
	}

	if(flag[SCB_ATK_ELE]) {
		status->rhw.ele = status_calc_attack_element(&bl, sc, b_status->rhw.ele);
		if (sd) sd->state.lr_flag = 1;
		status->lhw.ele = status_calc_attack_element(&bl, sc, b_status->lhw.ele);
		if (sd) sd->state.lr_flag = 0;
	}

	if(flag[SCB_DEF_ELE]) {
		status->def_ele = status_calc_element(&bl, sc, b_status->def_ele);
		status->ele_lv = status_calc_element_lv(&bl, sc, b_status->ele_lv);
	}

	if(flag[SCB_MODE]) {
		status->mode = status_calc_mode(&bl, sc, b_status->mode);

		if (status_has_mode(status, MD_STATUSIMMUNE|MD_SKILLIMMUNE))
			status->class_ = CLASS_BATTLEFIELD;
		else if (status_has_mode(status, MD_STATUSIMMUNE|MD_KNOCKBACKIMMUNE|MD_DETECTOR))
			status->class_ = CLASS_BOSS;
		else if (status_has_mode(status, MD_STATUSIMMUNE))
			status->class_ = CLASS_GUARDIAN;
		else
			status->class_ = CLASS_NORMAL;

		// Since mode changed, reset their state.
		if (!status_has_mode(status,MD_CANATTACK))
			unit_stop_attack(&bl);
		if (!status_has_mode(status,MD_CANMOVE))
			unit_stop_walking(&bl,1);
	}

	/**
	* No status changes alter these yet.
	* if(flag[SCB_SIZE])
	* if(flag[SCB_RACE])
	* if(flag[SCB_RANGE])
	**/

	if(flag[SCB_MAXHP]) {
		if( bl.type == BL_PC ) {
			status->max_hp = status_calc_maxhpsp_pc(sd,status->vit,true);

			if(battle_config.hp_rate != 100)
				status->max_hp = (unsigned int)(battle_config.hp_rate * (status->max_hp/100.));

			if (sd->status.base_level < 100)
				status->max_hp = umin(status->max_hp,(unsigned int)battle_config.max_hp_lv99);
			else if (sd->status.base_level < 151)
				status->max_hp = umin(status->max_hp,(unsigned int)battle_config.max_hp_lv150);
			else
				status->max_hp = umin(status->max_hp,(unsigned int)battle_config.max_hp);
		}
		else
			status->max_hp = status_calc_maxhp(&bl, b_status->max_hp);

		if( status->hp > status->max_hp ) { // !FIXME: Should perhaps a status_zap should be issued?
			status->hp = status->max_hp;
			if( sd ) clif_updatestatus(*sd,SP_HP);
		}
	}

	if(flag[SCB_MAXSP]) {
		if( bl.type == BL_PC ) {
			status->max_sp = status_calc_maxhpsp_pc(sd,status->int_,false);

			if(battle_config.sp_rate != 100)
				status->max_sp = (unsigned int)(battle_config.sp_rate * (status->max_sp/100.));

			status->max_sp = umin(status->max_sp,(unsigned int)battle_config.max_sp);
		}
		else
			status->max_sp = status_calc_maxsp(&bl, b_status->max_sp);

		if( status->sp > status->max_sp ) {
			status->sp = status->max_sp;
			if( sd ) clif_updatestatus(*sd,SP_SP);
		}
	}

	if(flag[SCB_MATK]) {
#ifndef RENEWAL
		status->matk_min = status_base_matk_min(status) + (sd != nullptr ? sd->bonus.ematk : 0);
		status->matk_max = status_base_matk_max(status) + (sd != nullptr ? sd->bonus.ematk : 0);

		if (sd != nullptr && sd->matk_rate != 100) {
			status->matk_min = status->matk_min * sd->matk_rate / 100;
			status->matk_max = status->matk_max * sd->matk_rate / 100;
		}

		// Apply Recognized Spell buff
		// Also update homunculus MATK, hom Min Matk is always the same as Max Matk
		if ((bl.type == BL_HOM && battle_config.hom_setting&HOMSET_SAME_MATK) || (sc && sc->getSCE(SC_RECOGNIZEDSPELL))) {
			status->matk_min = std::max( status->matk_min, status->matk_max );
		}

		status->matk_min = status_calc_matk(&bl, sc, status->matk_min);
		status->matk_max = status_calc_matk(&bl, sc, status->matk_max);
#else
		/**
		 * RE MATK Formula (from irowiki:http:// irowiki.org/wiki/MATK)
		 * MATK = (sMATK + wMATK + eMATK) * Multiplicative Modifiers
		 **/
		int lv = status_get_lv(&bl);
		status->matk_min = status_base_matk_min(&bl, status, lv);
		status->matk_max = status_base_matk_max(&bl, status, lv);

		if (sd != nullptr) {
			status->matk_min = status_calc_ematk(&bl, sc, status->matk_min);
			status->matk_max = status->matk_min;

			// Adds weapon magic attack (wMATK) modifications
			// This is the only portion in MATK that varies depending on the weapon level and refinement rate.
			if (b_status->lhw.matk > 0)
				status->lhw.matk = b_status->lhw.matk;
			if (b_status->rhw.matk > 0)
				status->rhw.matk = b_status->rhw.matk;

			int32 wMatk = 0;
			int32 variance = 0;

			if (status->rhw.matk > 0) {
				wMatk = status->rhw.matk;
				variance = status->rhw.matk * status->rhw.wlv / 10;
			}
			if (status->lhw.matk > 0) {
				wMatk += status->lhw.matk;
				variance += status->lhw.matk * status->lhw.wlv / 10;
			}

			status->matk_min += wMatk - variance;
			status->matk_max += wMatk + variance;
		}

		// Apply Recognized Spell buff
		// Also update homunculus MATK, hom Min Matk is always the same as Max Matk
		if ((bl.type == BL_HOM && battle_config.hom_setting&HOMSET_SAME_MATK) || (sc && sc->getSCE(SC_RECOGNIZEDSPELL))) {
			status->matk_min = std::max( status->matk_min, status->matk_max );
		}

		if (sd != nullptr && sd->right_weapon.overrefine > 0) {
			status->matk_min++;
			status->matk_max += sd->right_weapon.overrefine - 1;
		}

		// Apply MATK % from skill Mystical Amplification
		if (sc && sc->getSCE(SC_MAGICPOWER)) {
			status->matk_min += status->matk_min * sc->getSCE(SC_MAGICPOWER)->val3 / 100;
			status->matk_max += status->matk_max * sc->getSCE(SC_MAGICPOWER)->val3 / 100;
		}

		if (sd != nullptr) {
			if (sd->bonus.ematk > 0) {
				status->matk_min += sd->bonus.ematk;
				status->matk_max += sd->bonus.ematk;
			}

			if (pc_checkskill(sd, SU_POWEROFLAND) > 0 && pc_checkskill_summoner(sd, SUMMONER_POWER_LAND) >= 20) {
				status->matk_min += status->matk_min * 20 / 100;
				status->matk_max += status->matk_max * 20 / 100;
			}

			if (uint16 skill_lv = pc_checkskill(sd, NV_TRANSCENDENCE); skill_lv > 0) {
				status->matk_min += 15 * skill_lv + (skill_lv > 4 ? 25 : 0);
				status->matk_max += 15 * skill_lv + (skill_lv > 4 ? 25 : 0);
			}
		}

		// Adds other magic attack modifications - usually buffs from usable items
		status->matk_max = status_calc_matk(&bl, sc, status->matk_max);
		status->matk_min = status_calc_matk(&bl, sc, status->matk_min);

		// Apply MATK % from equipments / usable items
		if (sd != nullptr && sd->matk_rate != 100) {
			status->matk_min = status->matk_min * sd->matk_rate / 100;
			status->matk_max = status->matk_max * sd->matk_rate / 100;
		}
#endif
	}

	if(flag[SCB_ASPD]) {
		int amotion;

		if ( bl.type == BL_HOM ) {
#ifdef RENEWAL_ASPD
			amotion = (reinterpret_cast<homun_data*>(&bl))->homunculusDB->baseASPD;
			amotion = amotion - amotion * status_get_homdex(&bl) / 1000 - status_get_homagi(&bl) * amotion / 250;
			amotion = (amotion * status_calc_aspd(&bl, sc, true) + status_calc_aspd(&bl, sc, false)) / - 100 + amotion;
#else
			amotion = (1000 - 4 * status->agi - status->dex) * (reinterpret_cast<homun_data*>(&bl))->homunculusDB->baseASPD / 1000;

			amotion = status_calc_aspd_rate(&bl, sc, amotion);
			amotion = amotion * status->aspd_rate / 1000;
#endif

			amotion = status_calc_fix_aspd(&bl, sc, amotion);
			status->amotion = cap_value(amotion, battle_config.max_aspd, 2000);

			status->adelay = status->amotion;
		} else if ( bl.type == BL_PC ) {
			uint16 skill_lv;

			amotion = status_base_amotion_pc(sd,status);
#ifndef RENEWAL_ASPD
			status->aspd_rate = status_calc_aspd_rate(&bl, sc, b_status->aspd_rate);
#endif
			// Absolute ASPD % modifiers
			amotion = amotion * status->aspd_rate / 1000;
			if (sd->ud.skilltimer != INVALID_TIMER && (skill_lv = pc_checkskill(sd, SA_FREECAST)) > 0)
#ifdef RENEWAL_ASPD
				amotion = amotion * 5 * (skill_lv + 10) / 100;
#else
				amotion += (2000 - amotion) * ( 55 - 5 * ( skill_lv + 1 ) ) / 100; //Increases amotion to reduce ASPD to the corresponding absolute percentage for each level (overriding other adjustments)
#endif

#ifdef RENEWAL_ASPD
			// RE ASPD % modifier
			amotion += (max(0xc3 - amotion, 2) * (status->aspd_rate2 + status_calc_aspd(&bl, sc, false))) / 100;
			amotion = 10 * (200 - amotion);

			amotion += sd->bonus.aspd_add;
#endif
			amotion = status_calc_fix_aspd(&bl, sc, amotion);
			status->amotion = cap_value(amotion,pc_maxaspd(sd),2000);

			status->adelay = 2 * status->amotion;
		} else { // Mercenary and mobs
			amotion = b_status->amotion;
			status->aspd_rate = status_calc_aspd_rate(&bl, sc, b_status->aspd_rate);
			amotion = amotion*status->aspd_rate/1000;

			amotion = status_calc_fix_aspd(&bl, sc, amotion);
			status->amotion = cap_value(amotion, battle_config.monster_max_aspd, 2000);

			temp = b_status->adelay*status->aspd_rate/1000;
			status->adelay = cap_value(temp, battle_config.monster_max_aspd*2, 4000);
		}
	}

	if(flag[SCB_DSPD]) {
		int dmotion;
		if( bl.type == BL_PC ) {
			if (b_status->agi == status->agi)
				status->dmotion = status_calc_dmotion(&bl, sc, b_status->dmotion);
			else {
				dmotion = 800-status->agi*4;
				status->dmotion = cap_value(dmotion, 400, 800);
				if(battle_config.pc_damage_delay_rate != 100)
					status->dmotion = status->dmotion*battle_config.pc_damage_delay_rate/100;
				// It's safe to ignore b_status->dmotion since no bonus affects it.
				status->dmotion = status_calc_dmotion(&bl, sc, status->dmotion);
			}
		} else if( bl.type == BL_HOM ) {
			dmotion = 800-status->agi*4;
			status->dmotion = cap_value(dmotion, 400, 800);
			status->dmotion = status_calc_dmotion(&bl, sc, b_status->dmotion);
		} else { // Mercenary and mobs
			status->dmotion = status_calc_dmotion(&bl, sc, b_status->dmotion);
		}
	}

#ifdef RENEWAL
	if (flag[SCB_PATK]) {
		if (status->pow == b_status->pow && status->con == b_status->con)
			status->patk = status_calc_patk(&bl, sc, b_status->patk);
		else
			status->patk = status_calc_patk(&bl, sc, b_status->patk + (status->pow - b_status->pow) / 3 + (status->con - b_status->con) / 5);
	}

	if (flag[SCB_SMATK]) {
		if (status->spl == b_status->spl && status->con == b_status->con)
			status->smatk = status_calc_smatk(&bl, sc, b_status->smatk);
		else
			status->smatk = status_calc_smatk(&bl, sc, b_status->smatk) + (status->spl - b_status->spl) / 3 + (status->con - b_status->con) / 5;
	}

	if (flag[SCB_RES]) {
		if (status->sta == b_status->sta)
			status->res = status_calc_res(&bl, sc, b_status->res);
		else
			status->res = status_calc_res(&bl, sc, b_status->res + (status->sta - b_status->sta) + (status->sta - b_status->sta) / 3 * 5);
	}

	if (flag[SCB_MRES]) {
		if (status->wis == b_status->wis)
			status->mres = status_calc_mres(&bl, sc, b_status->mres);
		else
			status->mres = status_calc_mres(&bl, sc, b_status->mres + (status->wis - b_status->wis) + (status->wis - b_status->wis) / 3 * 5);
	}

	if (flag[SCB_HPLUS]) {
		if (status->crt == b_status->crt)
			status->hplus = status_calc_hplus(&bl, sc, b_status->hplus);
		else
			status->hplus = status_calc_hplus(&bl, sc, b_status->hplus + (status->crt - b_status->crt));
	}

	if (flag[SCB_CRATE]) {
		if (status->crt == b_status->crt)
			status->crate = status_calc_crate(&bl, sc, b_status->crate);
		else
			status->crate = status_calc_crate(&bl, sc, b_status->crate + (status->crt - b_status->crt) / 3);
	}

	if (flag[SCB_MAXAP]) {
		if (bl.type == BL_PC) {
			status->max_ap = status_calc_maxap_pc(sd);

			if (battle_config.ap_rate != 100)
				status->max_ap = (unsigned int)(battle_config.ap_rate * (status->max_ap / 100.));

			status->max_ap = umin(status->max_ap, (unsigned int)battle_config.max_ap);
		} else
			status->max_ap = status_calc_maxap(&bl, b_status->max_ap);

		if (status->ap > status->max_ap) {
			status->ap = status->max_ap;
			if (sd) clif_updatestatus(*sd, SP_AP);
		}
	}
#endif

	if((flag[SCB_VIT] || flag[SCB_MAXHP] || flag[SCB_INT] || flag[SCB_MAXSP]) && bl.type & BL_REGEN)
		status_calc_regen(&bl, status, status_get_regen_data(&bl));

	if(flag[SCB_REGEN] && bl.type & BL_REGEN)
		status_calc_regen_rate(&bl, status_get_regen_data(&bl), sc);
}

/**
 * Recalculates parts of an objects status according to specified flags
 * Also sends updates to the client when necessary
 * See [set_sc] [add_sc]
 * @param bl: Object whose status has changed [PC|MOB|HOM|MER|ELEM]
 * @param flag: Which status has changed on bl
 * @param opt: If true, will cause status_calc_* functions to run their base status initialization code
 */
void status_calc_bl_(struct block_list* bl, std::bitset<SCB_MAX> flag, uint8 opt)
{
	if (bl->type == BL_PC) {
		map_session_data *sd = BL_CAST(BL_PC, bl);

		if (sd->delayed_damage != 0) {
			if (opt&SCO_FORCE)
				sd->state.hold_recalc = false; // Clear and move on
			else {
				sd->state.hold_recalc = true; // Flag and stop
				return;
			}
		}
	}

	// Pointer to current battle status
	status_data* status = status_get_status_data(*bl);

	// Previous battle status
	status_data b_status;

	// Remember previous values
	memcpy(&b_status, status, sizeof(b_status));

	if( flag[SCB_BASE] ) { // Calculate the object's base status too
		switch( bl->type ) {
		case BL_PC:  status_calc_pc_(BL_CAST(BL_PC,bl), opt);          break;
		case BL_MOB: status_calc_mob_(BL_CAST(BL_MOB,bl), opt);        break;
		case BL_PET: status_calc_pet_(BL_CAST(BL_PET,bl), opt);        break;
		case BL_HOM: status_calc_homunculus_(BL_CAST(BL_HOM,bl), opt); break;
		case BL_MER: status_calc_mercenary_(BL_CAST(BL_MER,bl), opt);  break;
		case BL_ELEM: status_calc_elemental_(BL_CAST(BL_ELEM,bl), opt);  break;
		case BL_NPC: status_calc_npc_(BL_CAST(BL_NPC,bl), opt); break;
		}
	}

	if( bl->type == BL_PET )
		return; // Pets are not affected by statuses

	if (opt&SCO_FIRST && bl->type == BL_MOB)
		return; // Assume there will be no statuses active

	status_calc_bl_main(*bl, flag);

	if (opt&SCO_FIRST && bl->type == BL_HOM)
		return; // Client update handled by caller

	// Compare against new values and send client updates
	if( bl->type == BL_PC ) {
		TBL_PC* sd = BL_CAST(BL_PC, bl);

		if(b_status.str != status->str)
			clif_updatestatus(*sd,SP_STR);
		if(b_status.agi != status->agi)
			clif_updatestatus(*sd,SP_AGI);
		if(b_status.vit != status->vit)
			clif_updatestatus(*sd,SP_VIT);
		if(b_status.int_ != status->int_)
			clif_updatestatus(*sd,SP_INT);
		if(b_status.dex != status->dex)
			clif_updatestatus(*sd,SP_DEX);
		if(b_status.luk != status->luk)
			clif_updatestatus(*sd,SP_LUK);
		if(b_status.hit != status->hit)
			clif_updatestatus(*sd,SP_HIT);
		if(b_status.flee != status->flee)
			clif_updatestatus(*sd,SP_FLEE1);
		if(b_status.amotion != status->amotion)
			clif_updatestatus(*sd,SP_ASPD);
		if(b_status.speed != status->speed)
			clif_updatestatus(*sd,SP_SPEED);

		if(b_status.batk != status->batk
#ifndef RENEWAL
			|| b_status.rhw.atk != status->rhw.atk || b_status.lhw.atk != status->lhw.atk
#endif
			)
			clif_updatestatus(*sd,SP_ATK1);

		if(b_status.def != status->def) {
			clif_updatestatus(*sd,SP_DEF1);
#ifdef RENEWAL
			clif_updatestatus(*sd,SP_DEF2);
#endif
		}

		if(
#ifdef RENEWAL
			b_status.watk != status->watk || b_status.watk2 != status->watk2 || b_status.eatk != status->eatk
#else
			b_status.rhw.atk2 != status->rhw.atk2 || b_status.lhw.atk2 != status->lhw.atk2
#endif
			)
			clif_updatestatus(*sd,SP_ATK2);

		if(b_status.def2 != status->def2) {
			clif_updatestatus(*sd,SP_DEF2);
#ifdef RENEWAL
			clif_updatestatus(*sd,SP_DEF1);
#endif
		}
		if(b_status.flee2 != status->flee2)
			clif_updatestatus(*sd,SP_FLEE2);
		if(b_status.cri != status->cri)
			clif_updatestatus(*sd,SP_CRITICAL);
#ifndef RENEWAL
		if(b_status.matk_max != status->matk_max)
			clif_updatestatus(*sd,SP_MATK1);
		if(b_status.matk_min != status->matk_min)
			clif_updatestatus(*sd,SP_MATK2);
#else
		if(b_status.matk_max != status->matk_max || b_status.matk_min != status->matk_min) {
			clif_updatestatus(*sd,SP_MATK2);
			clif_updatestatus(*sd,SP_MATK1);
		}
#endif
		if(b_status.mdef != status->mdef) {
			clif_updatestatus(*sd,SP_MDEF1);
#ifdef RENEWAL
			clif_updatestatus(*sd,SP_MDEF2);
#endif
		}
		if(b_status.mdef2 != status->mdef2) {
			clif_updatestatus(*sd,SP_MDEF2);
#ifdef RENEWAL
			clif_updatestatus(*sd,SP_MDEF1);
#endif
		}
		if(b_status.rhw.range != status->rhw.range)
			clif_updatestatus(*sd,SP_ATTACKRANGE);
		if(b_status.max_hp != status->max_hp)
			clif_updatestatus(*sd,SP_MAXHP);
		if(b_status.max_sp != status->max_sp)
			clif_updatestatus(*sd,SP_MAXSP);
		if(b_status.hp != status->hp)
			clif_updatestatus(*sd,SP_HP);
		if(b_status.sp != status->sp)
			clif_updatestatus(*sd,SP_SP);
#ifdef RENEWAL
		if (b_status.pow != status->pow)
			clif_updatestatus(*sd,SP_POW);
		if (b_status.sta != status->sta)
			clif_updatestatus(*sd,SP_STA);
		if (b_status.wis != status->wis)
			clif_updatestatus(*sd,SP_WIS);
		if (b_status.spl != status->spl)
			clif_updatestatus(*sd,SP_SPL);
		if (b_status.con != status->con)
			clif_updatestatus(*sd,SP_CON);
		if (b_status.crt != status->crt)
			clif_updatestatus(*sd,SP_CRT);
		if (b_status.patk != status->patk)
			clif_updatestatus(*sd, SP_PATK);
		if (b_status.smatk != status->smatk)
			clif_updatestatus(*sd, SP_SMATK);
		if (b_status.res != status->res)
			clif_updatestatus(*sd, SP_RES);
		if (b_status.mres != status->mres)
			clif_updatestatus(*sd, SP_MRES);
		if (b_status.hplus != status->hplus)
			clif_updatestatus(*sd, SP_HPLUS);
		if (b_status.crate != status->crate)
			clif_updatestatus(*sd, SP_CRATE);
		if (b_status.max_ap != status->max_ap)
			clif_updatestatus(*sd, SP_MAXAP);
		if (b_status.ap != status->ap)
			clif_updatestatus(*sd, SP_AP);
#endif
	} else if( bl->type == BL_HOM ) {
		TBL_HOM* hd = BL_CAST(BL_HOM, bl);

		if( hd->master && memcmp(&b_status, status, sizeof(struct status_data)) != 0 )
			clif_hominfo(hd->master,hd,0);
	} else if( bl->type == BL_MER ) {
		TBL_MER* md = BL_CAST(BL_MER, bl);

		if (!md->master)
			return;

		if( b_status.rhw.atk != status->rhw.atk || b_status.rhw.atk2 != status->rhw.atk2 )
			clif_mercenary_updatestatus(md->master, SP_ATK1);
		if( b_status.matk_max != status->matk_max )
			clif_mercenary_updatestatus(md->master, SP_MATK1);
		if( b_status.hit != status->hit )
			clif_mercenary_updatestatus(md->master, SP_HIT);
		if( b_status.cri != status->cri )
			clif_mercenary_updatestatus(md->master, SP_CRITICAL);
		if( b_status.def != status->def )
			clif_mercenary_updatestatus(md->master, SP_DEF1);
		if( b_status.mdef != status->mdef )
			clif_mercenary_updatestatus(md->master, SP_MDEF1);
		if( b_status.flee != status->flee )
			clif_mercenary_updatestatus(md->master, SP_MERCFLEE);
		if( b_status.amotion != status->amotion )
			clif_mercenary_updatestatus(md->master, SP_ASPD);
		if( b_status.max_hp != status->max_hp )
			clif_mercenary_updatestatus(md->master, SP_MAXHP);
		if( b_status.max_sp != status->max_sp )
			clif_mercenary_updatestatus(md->master, SP_MAXSP);
		if( b_status.hp != status->hp )
			clif_mercenary_updatestatus(md->master, SP_HP);
		if( b_status.sp != status->sp )
			clif_mercenary_updatestatus(md->master, SP_SP);
	} else if( bl->type == BL_ELEM ) {
		TBL_ELEM* ed = BL_CAST(BL_ELEM, bl);

		if (!ed->master)
			return;

		if( b_status.max_hp != status->max_hp )
			clif_elemental_updatestatus(*ed->master, SP_MAXHP);
		if( b_status.max_sp != status->max_sp )
			clif_elemental_updatestatus(*ed->master, SP_MAXSP);
		if( b_status.hp != status->hp )
			clif_elemental_updatestatus(*ed->master, SP_HP);
		if( b_status.sp != status->sp )
			clif_elemental_updatestatus(*ed->master, SP_SP);
	}
}

/**
 * Adds strength modifications based on status changes
 * @param bl: Object to change str [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param str: Initial str
 * @return modified str with cap_value(str,0,USHRT_MAX)
 */
static unsigned short status_calc_str(struct block_list *bl, status_change *sc, int str)
{
	if(!sc || !sc->count)
		return cap_value(str,0,USHRT_MAX);

	if(sc->getSCE(SC_HARMONIZE)) {
		str -= sc->getSCE(SC_HARMONIZE)->val2;
		return (unsigned short)cap_value(str,0,USHRT_MAX);
	}
	if(sc->getSCE(SC_INCALLSTATUS))
		str += sc->getSCE(SC_INCALLSTATUS)->val1;
	if(sc->getSCE(SC_CHASEWALK2))
		str += sc->getSCE(SC_CHASEWALK2)->val1;
	if(sc->getSCE(SC_INCSTR))
		str += sc->getSCE(SC_INCSTR)->val1;
	if(sc->getSCE(SC_STRFOOD))
		str += sc->getSCE(SC_STRFOOD)->val1;
	if(sc->getSCE(SC_FOOD_STR_CASH))
		str += sc->getSCE(SC_FOOD_STR_CASH)->val1;
	if(sc->getSCE(SC_BATTLEORDERS))
		str += 5;
	if(sc->getSCE(SC_LEADERSHIP))
		str += sc->getSCE(SC_LEADERSHIP)->val1;
	if(sc->getSCE(SC_LOUD))
		str += 4;
	if(sc->getSCE(SC_TRUESIGHT))
		str += 5;
	if(sc->getSCE(SC_SPURT))
		str += 10;
	if(sc->getSCE(SC_NEN))
		str += sc->getSCE(SC_NEN)->val1;
	if(sc->getSCE(SC_BLESSING)) {
		if(sc->getSCE(SC_BLESSING)->val2)
			str += sc->getSCE(SC_BLESSING)->val2;
		else
			str -= str / 2;
	}
	if(sc->getSCE(SC_MARIONETTE))
		str -= ((sc->getSCE(SC_MARIONETTE)->val3)>>16)&0xFF;
	if(sc->getSCE(SC_MARIONETTE2))
		str += ((sc->getSCE(SC_MARIONETTE2)->val3)>>16)&0xFF;
	if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
		str += ((sc->getSCE(SC_SPIRIT)->val3)>>16)&0xFF;
	if(sc->getSCE(SC_GIANTGROWTH))
		str += sc->getSCE(SC_GIANTGROWTH)->val2;
	if(sc->getSCE(SC_BEYONDOFWARCRY))
		str -= sc->getSCE(SC_BEYONDOFWARCRY)->val2;
	if(sc->getSCE(SC_SAVAGE_STEAK))
		str += sc->getSCE(SC_SAVAGE_STEAK)->val1;
	if(sc->getSCE(SC_INSPIRATION))
		str += sc->getSCE(SC_INSPIRATION)->val3;
	if(sc->getSCE(SC_STOMACHACHE))
		str -= sc->getSCE(SC_STOMACHACHE)->val1;
	if(sc->getSCE(SC_KYOUGAKU))
		str -= sc->getSCE(SC_KYOUGAKU)->val2;
	if(sc->getSCE(SC_SWORDCLAN))
		str += 1;
	if(sc->getSCE(SC_JUMPINGCLAN))
		str += 1;
	if(sc->getSCE(SC_FULL_THROTTLE))
		str += str * sc->getSCE(SC_FULL_THROTTLE)->val3 / 100;
	if(sc->getSCE(SC_CHEERUP))
		str += 3;
	if(sc->getSCE(SC_GLASTHEIM_STATE))
		str += sc->getSCE(SC_GLASTHEIM_STATE)->val1;
#ifdef RENEWAL
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ALLSTAT)
		str += 15;
#endif
	if (sc->getSCE(SC_UNIVERSESTANCE))
		str += sc->getSCE(SC_UNIVERSESTANCE)->val2;
	if (sc->getSCE(SC_ULTIMATECOOK))
		str += sc->getSCE(SC_ULTIMATECOOK)->val1;
	if (sc->getSCE(SC_ALL_STAT_DOWN))
		str -= sc->getSCE(SC_ALL_STAT_DOWN)->val2;

	//TODO: Stat points should be able to be decreased below 0
	return (unsigned short)cap_value(str,0,USHRT_MAX);
}

/**
 * Adds agility modifications based on status changes
 * @param bl: Object to change agi [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param agi: Initial agi
 * @return modified agi with cap_value(agi,0,USHRT_MAX)
 */
static unsigned short status_calc_agi(struct block_list *bl, status_change *sc, int agi)
{
	if(!sc || !sc->count)
		return cap_value(agi,0,USHRT_MAX);

	if(sc->getSCE(SC_HARMONIZE)) {
		agi -= sc->getSCE(SC_HARMONIZE)->val2;
		return (unsigned short)cap_value(agi,0,USHRT_MAX);
	}
	if(sc->getSCE(SC_CONCENTRATE) && !sc->getSCE(SC_QUAGMIRE))
		agi += (agi-sc->getSCE(SC_CONCENTRATE)->val3)*sc->getSCE(SC_CONCENTRATE)->val2/100;
	if(sc->getSCE(SC_INCALLSTATUS))
		agi += sc->getSCE(SC_INCALLSTATUS)->val1;
	if(sc->getSCE(SC_INCAGI))
		agi += sc->getSCE(SC_INCAGI)->val1;
	if(sc->getSCE(SC_AGIFOOD))
		agi += sc->getSCE(SC_AGIFOOD)->val1;
	if(sc->getSCE(SC_FOOD_AGI_CASH))
		agi += sc->getSCE(SC_FOOD_AGI_CASH)->val1;
	if(sc->getSCE(SC_SOULCOLD))
		agi += sc->getSCE(SC_SOULCOLD)->val1;
	if(sc->getSCE(SC_TRUESIGHT))
		agi += 5;
	if(sc->getSCE(SC_INCREASEAGI))
		agi += sc->getSCE(SC_INCREASEAGI)->val2;
	if(sc->getSCE(SC_INCREASING))
		agi += 4; // Added based on skill updates [Reddozen]
	if(sc->getSCE(SC_DECREASEAGI))
		agi -= sc->getSCE(SC_DECREASEAGI)->val2;
	if(sc->getSCE(SC_QUAGMIRE))
		agi -= sc->getSCE(SC_QUAGMIRE)->val2;
	if(sc->getSCE(SC_SUITON) && sc->getSCE(SC_SUITON)->val3)
		agi -= sc->getSCE(SC_SUITON)->val2;
	if(sc->getSCE(SC_MARIONETTE))
		agi -= ((sc->getSCE(SC_MARIONETTE)->val3)>>8)&0xFF;
	if(sc->getSCE(SC_MARIONETTE2))
		agi += ((sc->getSCE(SC_MARIONETTE2)->val3)>>8)&0xFF;
	if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
		agi += ((sc->getSCE(SC_SPIRIT)->val3)>>8)&0xFF;
	if(sc->getSCE(SC_ADORAMUS))
		agi -= sc->getSCE(SC_ADORAMUS)->val2;
	if(sc->getSCE(SC_MARSHOFABYSS))
		agi -= agi * sc->getSCE(SC_MARSHOFABYSS)->val2 / 100;
	if(sc->getSCE(SC_DROCERA_HERB_STEAMED))
		agi += sc->getSCE(SC_DROCERA_HERB_STEAMED)->val1;
	if(sc->getSCE(SC_INSPIRATION))
		agi += sc->getSCE(SC_INSPIRATION)->val3;
	if(sc->getSCE(SC_STOMACHACHE))
		agi -= sc->getSCE(SC_STOMACHACHE)->val1;
	if(sc->getSCE(SC_KYOUGAKU))
		agi -= sc->getSCE(SC_KYOUGAKU)->val2;
	if(sc->getSCE(SC_CROSSBOWCLAN))
		agi += 1;
	if(sc->getSCE(SC_JUMPINGCLAN))
		agi += 1;
	if(sc->getSCE(SC_FULL_THROTTLE))
		agi += agi * sc->getSCE(SC_FULL_THROTTLE)->val3 / 100;
	if (sc->getSCE(SC_ARCLOUSEDASH))
		agi += sc->getSCE(SC_ARCLOUSEDASH)->val2;
	if(sc->getSCE(SC_CHEERUP))
		agi += 3;
	if(sc->getSCE(SC_GLASTHEIM_STATE))
		agi += sc->getSCE(SC_GLASTHEIM_STATE)->val1;
#ifdef RENEWAL
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ALLSTAT)
		agi += 15;
#endif
	if (sc->getSCE(SC_UNIVERSESTANCE))
		agi += sc->getSCE(SC_UNIVERSESTANCE)->val2;
	if (sc->getSCE(SC_ULTIMATECOOK))
		agi += sc->getSCE(SC_ULTIMATECOOK)->val1;
	if (sc->getSCE(SC_ALL_STAT_DOWN))
		agi -= sc->getSCE(SC_ALL_STAT_DOWN)->val2;

	//TODO: Stat points should be able to be decreased below 0
	return (unsigned short)cap_value(agi,0,USHRT_MAX);
}

/**
 * Adds vitality modifications based on status changes
 * @param bl: Object to change vit [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param vit: Initial vit
 * @return modified vit with cap_value(vit,0,USHRT_MAX)
 */
static unsigned short status_calc_vit(struct block_list *bl, status_change *sc, int vit)
{
	if(!sc || !sc->count)
		return cap_value(vit,0,USHRT_MAX);

	if(sc->getSCE(SC_HARMONIZE)) {
		vit -= sc->getSCE(SC_HARMONIZE)->val2;
		return (unsigned short)cap_value(vit,0,USHRT_MAX);
	}
	if(sc->getSCE(SC_INCALLSTATUS))
		vit += sc->getSCE(SC_INCALLSTATUS)->val1;
	if(sc->getSCE(SC_INCVIT))
		vit += sc->getSCE(SC_INCVIT)->val1;
	if(sc->getSCE(SC_VITFOOD))
		vit += sc->getSCE(SC_VITFOOD)->val1;
	if(sc->getSCE(SC_FOOD_VIT_CASH))
		vit += sc->getSCE(SC_FOOD_VIT_CASH)->val1;
	if(sc->getSCE(SC_CHANGE))
		vit += sc->getSCE(SC_CHANGE)->val2;
	if(sc->getSCE(SC_GLORYWOUNDS))
		vit += sc->getSCE(SC_GLORYWOUNDS)->val1;
	if(sc->getSCE(SC_TRUESIGHT))
		vit += 5;
	if(sc->getSCE(SC_MARIONETTE))
		vit -= sc->getSCE(SC_MARIONETTE)->val3&0xFF;
	if(sc->getSCE(SC_MARIONETTE2))
		vit += sc->getSCE(SC_MARIONETTE2)->val3&0xFF;
	if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
		vit += sc->getSCE(SC_SPIRIT)->val3&0xFF;
	if(sc->getSCE(SC_MINOR_BBQ))
		vit += sc->getSCE(SC_MINOR_BBQ)->val1;
	if(sc->getSCE(SC_INSPIRATION))
		vit += sc->getSCE(SC_INSPIRATION)->val3;
	if(sc->getSCE(SC_STOMACHACHE))
		vit -= sc->getSCE(SC_STOMACHACHE)->val1;
	if(sc->getSCE(SC_KYOUGAKU))
		vit -= sc->getSCE(SC_KYOUGAKU)->val2;
	if(sc->getSCE(SC_SWORDCLAN))
		vit += 1;
	if(sc->getSCE(SC_JUMPINGCLAN))
		vit += 1;
	if(sc->getSCE(SC_STRIPARMOR) && bl->type != BL_PC)
		vit -= vit * sc->getSCE(SC_STRIPARMOR)->val2/100;
	if(sc->getSCE(SC_FULL_THROTTLE))
		vit += vit * sc->getSCE(SC_FULL_THROTTLE)->val3 / 100;
	if(bl->type == BL_PC && sc->getSCE(SC_DEFENCE))
		vit += sc->getSCE(SC_DEFENCE)->val2;
	if(sc->getSCE(SC_CHEERUP))
		vit += 3;
	if(sc->getSCE(SC_GLASTHEIM_STATE))
		vit += sc->getSCE(SC_GLASTHEIM_STATE)->val1;
#ifdef RENEWAL
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ALLSTAT)
		vit += 15;
#endif
	if (sc->getSCE(SC_UNIVERSESTANCE))
		vit += sc->getSCE(SC_UNIVERSESTANCE)->val2;
	if (sc->getSCE(SC_ULTIMATECOOK))
		vit += sc->getSCE(SC_ULTIMATECOOK)->val1;
	if (sc->getSCE(SC_CUP_OF_BOZA))
		vit += 10;
	if (sc->getSCE(SC_ALL_STAT_DOWN))
		vit -= sc->getSCE(SC_ALL_STAT_DOWN)->val2;

	//TODO: Stat points should be able to be decreased below 0
	return (unsigned short)cap_value(vit,0,USHRT_MAX);
}

/**
 * Adds intelligence modifications based on status changes
 * @param bl: Object to change int [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param int_: Initial int
 * @return modified int with cap_value(int_,0,USHRT_MAX)
 */
static unsigned short status_calc_int(struct block_list *bl, status_change *sc, int int_)
{
	if(!sc || !sc->count)
		return cap_value(int_,0,USHRT_MAX);

	if(sc->getSCE(SC_HARMONIZE)) {
		int_ -= sc->getSCE(SC_HARMONIZE)->val2;
		return (unsigned short)cap_value(int_,0,USHRT_MAX);
	}
	if(sc->getSCE(SC_INCALLSTATUS))
		int_ += sc->getSCE(SC_INCALLSTATUS)->val1;
	if(sc->getSCE(SC_INCINT))
		int_ += sc->getSCE(SC_INCINT)->val1;
	if(sc->getSCE(SC_INTFOOD))
		int_ += sc->getSCE(SC_INTFOOD)->val1;
	if(sc->getSCE(SC_FOOD_INT_CASH))
		int_ += sc->getSCE(SC_FOOD_INT_CASH)->val1;
	if(sc->getSCE(SC_CHANGE))
		int_ += sc->getSCE(SC_CHANGE)->val3;
	if(sc->getSCE(SC_BATTLEORDERS))
		int_ += 5;
	if(sc->getSCE(SC_TRUESIGHT))
		int_ += 5;
	if(sc->getSCE(SC_BLESSING)) {
		if (sc->getSCE(SC_BLESSING)->val2)
			int_ += sc->getSCE(SC_BLESSING)->val2;
		else
			int_ -= int_ / 2;
	}
	if(sc->getSCE(SC_NEN))
		int_ += sc->getSCE(SC_NEN)->val1;
	if(sc->getSCE(SC_MARIONETTE))
		int_ -= ((sc->getSCE(SC_MARIONETTE)->val4)>>16)&0xFF;
	if(sc->getSCE(SC_MARIONETTE2))
		int_ += ((sc->getSCE(SC_MARIONETTE2)->val4)>>16)&0xFF;
	if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
		int_ += ((sc->getSCE(SC_SPIRIT)->val4)>>16)&0xFF;
	if(sc->getSCE(SC_INSPIRATION))
		int_ += sc->getSCE(SC_INSPIRATION)->val3;
	if(sc->getSCE(SC_MELODYOFSINK))
		int_ -= sc->getSCE(SC_MELODYOFSINK)->val2;
	if(sc->getSCE(SC_MANDRAGORA))
		int_ -= 4 * sc->getSCE(SC_MANDRAGORA)->val1;
	if(sc->getSCE(SC_COCKTAIL_WARG_BLOOD))
		int_ += sc->getSCE(SC_COCKTAIL_WARG_BLOOD)->val1;
	if(sc->getSCE(SC_STOMACHACHE))
		int_ -= sc->getSCE(SC_STOMACHACHE)->val1;
	if(sc->getSCE(SC_KYOUGAKU))
		int_ -= sc->getSCE(SC_KYOUGAKU)->val2;
	if(sc->getSCE(SC_ARCWANDCLAN))
		int_ += 1;
	if(sc->getSCE(SC_GOLDENMACECLAN))
		int_ += 1;
	if(sc->getSCE(SC_JUMPINGCLAN))
		int_ += 1;
	if(sc->getSCE(SC_FULL_THROTTLE))
		int_ += int_ * sc->getSCE(SC_FULL_THROTTLE)->val3 / 100;
	if(sc->getSCE(SC_CHEERUP))
		int_ += 3;
	if(sc->getSCE(SC_GLASTHEIM_STATE))
		int_ += sc->getSCE(SC_GLASTHEIM_STATE)->val1;
	if (sc->getSCE(SC_UNIVERSESTANCE))
		int_ += sc->getSCE(SC_UNIVERSESTANCE)->val2;

	if(bl->type != BL_PC) {
		if(sc->getSCE(SC_STRIPHELM))
			int_ -= int_ * sc->getSCE(SC_STRIPHELM)->val2/100;
		if(sc->getSCE(SC__STRIPACCESSORY))
			int_ -= int_ * sc->getSCE(SC__STRIPACCESSORY)->val2 / 100;
	}
#ifdef RENEWAL
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ALLSTAT)
		int_ += 15;
#endif
	if (sc->getSCE(SC_ULTIMATECOOK))
		int_ += sc->getSCE(SC_ULTIMATECOOK)->val1;
	if (sc->getSCE(SC_ALL_STAT_DOWN))
		int_ -= sc->getSCE(SC_ALL_STAT_DOWN)->val2;

	//TODO: Stat points should be able to be decreased below 0
	return (unsigned short)cap_value(int_,0,USHRT_MAX);
}

/**
 * Adds dexterity modifications based on status changes
 * @param bl: Object to change dex [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param dex: Initial dex
 * @return modified dex with cap_value(dex,0,USHRT_MAX)
 */
static unsigned short status_calc_dex(struct block_list *bl, status_change *sc, int dex)
{
	if(!sc || !sc->count)
		return cap_value(dex,0,USHRT_MAX);

	if(sc->getSCE(SC_HARMONIZE)) {
		dex -= sc->getSCE(SC_HARMONIZE)->val2;
		return (unsigned short)cap_value(dex,0,USHRT_MAX);
	}
	if(sc->getSCE(SC_CONCENTRATE) && !sc->getSCE(SC_QUAGMIRE))
		dex += (dex-sc->getSCE(SC_CONCENTRATE)->val4)*sc->getSCE(SC_CONCENTRATE)->val2/100;
	if(sc->getSCE(SC_INCALLSTATUS))
		dex += sc->getSCE(SC_INCALLSTATUS)->val1;
	if(sc->getSCE(SC_INCDEX))
		dex += sc->getSCE(SC_INCDEX)->val1;
	if(sc->getSCE(SC_DEXFOOD))
		dex += sc->getSCE(SC_DEXFOOD)->val1;
	if(sc->getSCE(SC_FOOD_DEX_CASH))
		dex += sc->getSCE(SC_FOOD_DEX_CASH)->val1;
	if(sc->getSCE(SC_BATTLEORDERS))
		dex += 5;
	if(sc->getSCE(SC_HAWKEYES))
		dex += sc->getSCE(SC_HAWKEYES)->val1;
	if(sc->getSCE(SC_TRUESIGHT))
		dex += 5;
	if(sc->getSCE(SC_QUAGMIRE))
		dex -= sc->getSCE(SC_QUAGMIRE)->val2;
	if(sc->getSCE(SC_BLESSING)) {
		if (sc->getSCE(SC_BLESSING)->val2)
			dex += sc->getSCE(SC_BLESSING)->val2;
		else
			dex -= dex / 2;
	}
	if(sc->getSCE(SC_INCREASING))
		dex += 4; // Added based on skill updates [Reddozen]
	if(sc->getSCE(SC_MARIONETTE))
		dex -= ((sc->getSCE(SC_MARIONETTE)->val4)>>8)&0xFF;
	if(sc->getSCE(SC_MARIONETTE2))
		dex += ((sc->getSCE(SC_MARIONETTE2)->val4)>>8)&0xFF;
	if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
		dex += ((sc->getSCE(SC_SPIRIT)->val4)>>8)&0xFF;
	if(sc->getSCE(SC_SIROMA_ICE_TEA))
		dex += sc->getSCE(SC_SIROMA_ICE_TEA)->val1;
	if(sc->getSCE(SC_INSPIRATION))
		dex += sc->getSCE(SC_INSPIRATION)->val3;
	if(sc->getSCE(SC_STOMACHACHE))
		dex -= sc->getSCE(SC_STOMACHACHE)->val1;
	if(sc->getSCE(SC_KYOUGAKU))
		dex -= sc->getSCE(SC_KYOUGAKU)->val2;
	if(sc->getSCE(SC_ARCWANDCLAN))
		dex += 1;
	if(sc->getSCE(SC_CROSSBOWCLAN))
		dex += 1;
	if(sc->getSCE(SC_JUMPINGCLAN))
		dex += 1;
	if(sc->getSCE(SC__STRIPACCESSORY) && bl->type != BL_PC)
		dex -= dex * sc->getSCE(SC__STRIPACCESSORY)->val2 / 100;
	if(sc->getSCE(SC_MARSHOFABYSS))
		dex -= dex * sc->getSCE(SC_MARSHOFABYSS)->val2 / 100;
	if(sc->getSCE(SC_FULL_THROTTLE))
		dex += dex * sc->getSCE(SC_FULL_THROTTLE)->val3 / 100;
	if(sc->getSCE(SC_CHEERUP))
		dex += 3;
	if(sc->getSCE(SC_GLASTHEIM_STATE))
		dex += sc->getSCE(SC_GLASTHEIM_STATE)->val1;
#ifdef RENEWAL
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ALLSTAT)
		dex += 15;
#endif
	if (sc->getSCE(SC_UNIVERSESTANCE))
		dex += sc->getSCE(SC_UNIVERSESTANCE)->val2;
	if (sc->getSCE(SC_ULTIMATECOOK))
		dex += sc->getSCE(SC_ULTIMATECOOK)->val1;
	if (sc->getSCE(SC_ALL_STAT_DOWN))
		dex -= sc->getSCE(SC_ALL_STAT_DOWN)->val2;

	//TODO: Stat points should be able to be decreased below 0
	return (unsigned short)cap_value(dex,0,USHRT_MAX);
}

/**
 * Adds luck modifications based on status changes
 * @param bl: Object to change luk [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param luk: Initial luk
 * @return modified luk with cap_value(luk,0,USHRT_MAX)
 */
static unsigned short status_calc_luk(struct block_list *bl, status_change *sc, int luk)
{
	if(!sc || !sc->count)
		return cap_value(luk,0,USHRT_MAX);

	if(sc->getSCE(SC_HARMONIZE)) {
		luk -= sc->getSCE(SC_HARMONIZE)->val2;
		return (unsigned short)cap_value(luk,0,USHRT_MAX);
	}
	if(sc->getSCE(SC_CURSE))
		return 0;
	if(sc->getSCE(SC_INCALLSTATUS))
		luk += sc->getSCE(SC_INCALLSTATUS)->val1;
	if(sc->getSCE(SC_INCLUK))
		luk += sc->getSCE(SC_INCLUK)->val1;
	if(sc->getSCE(SC_LUKFOOD))
		luk += sc->getSCE(SC_LUKFOOD)->val1;
	if(sc->getSCE(SC_FOOD_LUK_CASH))
		luk += sc->getSCE(SC_FOOD_LUK_CASH)->val1;
	if(sc->getSCE(SC_TRUESIGHT))
		luk += 5;
	if(sc->getSCE(SC_GLORIA))
		luk += 30;
	if(sc->getSCE(SC_MARIONETTE))
		luk -= sc->getSCE(SC_MARIONETTE)->val4&0xFF;
	if(sc->getSCE(SC_MARIONETTE2))
		luk += sc->getSCE(SC_MARIONETTE2)->val4&0xFF;
	if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
		luk += sc->getSCE(SC_SPIRIT)->val4&0xFF;
	if(sc->getSCE(SC_PUTTI_TAILS_NOODLES))
		luk += sc->getSCE(SC_PUTTI_TAILS_NOODLES)->val1;
	if(sc->getSCE(SC_INSPIRATION))
		luk += sc->getSCE(SC_INSPIRATION)->val3;
	if(sc->getSCE(SC_STOMACHACHE))
		luk -= sc->getSCE(SC_STOMACHACHE)->val1;
	if(sc->getSCE(SC_KYOUGAKU))
		luk -= sc->getSCE(SC_KYOUGAKU)->val2;
	if(sc->getSCE(SC__STRIPACCESSORY) && bl->type != BL_PC)
		luk -= luk * sc->getSCE(SC__STRIPACCESSORY)->val2 / 100;
	if(sc->getSCE(SC_BANANA_BOMB))
		luk -= 75;
	if(sc->getSCE(SC_GOLDENMACECLAN))
		luk += 1;
	if(sc->getSCE(SC_JUMPINGCLAN))
		luk += 1;
	if(sc->getSCE(SC_FULL_THROTTLE))
		luk += luk * sc->getSCE(SC_FULL_THROTTLE)->val3 / 100;
	if(sc->getSCE(SC_CHEERUP))
		luk += 3;
	if(sc->getSCE(SC_GLASTHEIM_STATE))
		luk += sc->getSCE(SC_GLASTHEIM_STATE)->val1;
#ifdef RENEWAL
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ALLSTAT)
		luk += 15;
#endif
	if (sc->getSCE(SC_UNIVERSESTANCE))
		luk += sc->getSCE(SC_UNIVERSESTANCE)->val2;
	if (sc->getSCE(SC_ULTIMATECOOK))
		luk += sc->getSCE(SC_ULTIMATECOOK)->val1;
	if (sc->getSCE(SC_MYSTICPOWDER))
		luk += 10;
	if (sc->getSCE(SC_ALL_STAT_DOWN))
		luk -= sc->getSCE(SC_ALL_STAT_DOWN)->val2;

	//TODO: Stat points should be able to be decreased below 0
	return (unsigned short)cap_value(luk,0,USHRT_MAX);
}

/**
* Adds power modifications based on status changes
* @param bl: Object to change pow [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param pow: Initial pow
* @return modified pow with cap_value(pow,0,USHRT_MAX)
*/
static unsigned short status_calc_pow(struct block_list *bl, status_change *sc, int pow)
{
	if (!sc || !sc->count)
		return cap_value(pow, 0, USHRT_MAX);

	if (sc->getSCE(SC_BENEDICTUM))
		pow += sc->getSCE(SC_BENEDICTUM)->val2;

	return (unsigned short)cap_value(pow, 0, USHRT_MAX);
}

/**
* Adds stamina modifications based on status changes
* @param bl: Object to change sta [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param sta: Initial sta
* @return modified sta with cap_value(sta,0,USHRT_MAX)
*/
static unsigned short status_calc_sta(struct block_list *bl, status_change *sc, int sta)
{
	if (!sc || !sc->count)
		return cap_value(sta, 0, USHRT_MAX);

	if (sc->getSCE(SC_RELIGIO))
		sta += sc->getSCE(SC_RELIGIO)->val2;

	return (unsigned short)cap_value(sta, 0, USHRT_MAX);
}

/**
* Adds wisdom modifications based on status changes
* @param bl: Object to change wis [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param wis: Initial wis
* @return modified wis with cap_value(wis,0,USHRT_MAX)
*/
static unsigned short status_calc_wis(struct block_list *bl, status_change *sc, int wis)
{
	if (!sc || !sc->count)
		return cap_value(wis, 0, USHRT_MAX);

	if (sc->getSCE(SC_RELIGIO))
		wis += sc->getSCE(SC_RELIGIO)->val2;

	return (unsigned short)cap_value(wis, 0, USHRT_MAX);
}

/**
* Adds spell modifications based on status changes
* @param bl: Object to change spl [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param spl: Initial spl
* @return modified spl with cap_value(spl,0,USHRT_MAX)
*/
static unsigned short status_calc_spl(struct block_list *bl, status_change *sc, int spl)
{
	if (!sc || !sc->count)
		return cap_value(spl, 0, USHRT_MAX);

	if (sc->getSCE(SC_RELIGIO))
		spl += sc->getSCE(SC_RELIGIO)->val2;

	return (unsigned short)cap_value(spl, 0, USHRT_MAX);
}

/**
* Adds concentration modifications based on status changes
* @param bl: Object to change con [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param con: Initial con
* @return modified con with cap_value(con,0,USHRT_MAX)
*/
static unsigned short status_calc_con(struct block_list *bl, status_change *sc, int con)
{
	if (!sc || !sc->count)
		return cap_value(con, 0, USHRT_MAX);

	if (sc->getSCE(SC_BENEDICTUM))
		con += sc->getSCE(SC_BENEDICTUM)->val2;

	return (unsigned short)cap_value(con, 0, USHRT_MAX);
}

/**
* Adds creative modifications based on status changes
* @param bl: Object to change crt [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param crt: Initial crt
* @return modified crt with cap_value(crt,0,USHRT_MAX)
*/
static unsigned short status_calc_crt(struct block_list *bl, status_change *sc, int crt)
{
	if (!sc || !sc->count)
		return cap_value(crt, 0, USHRT_MAX);

	if (sc->getSCE(SC_BENEDICTUM))
		crt += sc->getSCE(SC_BENEDICTUM)->val2;

	return (unsigned short)cap_value(crt, 0, USHRT_MAX);
}

/**
 * Adds base attack modifications based on status changes
 * @param bl: Object to change batk [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param batk: Initial batk
 * @return modified batk with cap_value(batk,0,USHRT_MAX)
 */
static unsigned short status_calc_batk(struct block_list *bl, status_change *sc, int batk)
{
	if(!sc || !sc->count)
		return cap_value(batk,0,USHRT_MAX);

	if(sc->getSCE(SC_ATKPOTION))
		batk += sc->getSCE(SC_ATKPOTION)->val1;
	if(sc->getSCE(SC_BATKFOOD))
		batk += sc->getSCE(SC_BATKFOOD)->val1;
	if (sc->getSCE(SC_VOLCANO))
		batk += sc->getSCE(SC_VOLCANO)->val2;
#ifndef RENEWAL
	if(sc->getSCE(SC_GATLINGFEVER))
		batk += sc->getSCE(SC_GATLINGFEVER)->val3;
	if(sc->getSCE(SC_MADNESSCANCEL))
		batk += 100;
#endif
	if(sc->getSCE(SC_FULL_SWING_K))
		batk += sc->getSCE(SC_FULL_SWING_K)->val1;
	if(sc->getSCE(SC_ASH))
		batk -= batk * sc->getSCE(SC_ASH)->val4 / 100;
	if(bl->type == BL_HOM && sc->getSCE(SC_PYROCLASTIC))
		batk += sc->getSCE(SC_PYROCLASTIC)->val2;
	if (sc->getSCE(SC_ANGRIFFS_MODUS))
		batk += sc->getSCE(SC_ANGRIFFS_MODUS)->val2;
	if(sc->getSCE(SC_2011RWC_SCROLL))
		batk += 30;
	if(sc->getSCE(SC__ENERVATION))
		batk -= batk * sc->getSCE(SC__ENERVATION)->val2 / 100;
	if( sc->getSCE(SC_ZANGETSU) )
		batk += sc->getSCE(SC_ZANGETSU)->val2;
	if(sc->getSCE(SC_QUEST_BUFF1))
		batk += sc->getSCE(SC_QUEST_BUFF1)->val1;
	if(sc->getSCE(SC_QUEST_BUFF2))
		batk += sc->getSCE(SC_QUEST_BUFF2)->val1;
	if(sc->getSCE(SC_QUEST_BUFF3))
		batk += sc->getSCE(SC_QUEST_BUFF3)->val1;
	if (sc->getSCE(SC_SHRIMP))
		batk += batk * sc->getSCE(SC_SHRIMP)->val2 / 100;
#ifdef RENEWAL
	if (sc->getSCE(SC_LOUD))
		batk += 30;
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ATKRATE)
		batk += batk * 20 / 100;
#endif
	if (sc->getSCE(SC_SUNSTANCE))
		batk += batk * sc->getSCE(SC_SUNSTANCE)->val2 / 100;
	if (sc->getSCE(SC_ALMIGHTY))
		batk += 30;
	if (sc->getSCE(SC_ULTIMATECOOK))
		batk += 30;
	if(sc->getSCE(SC_LIMIT_POWER_BOOSTER))
		batk += sc->getSCE(SC_LIMIT_POWER_BOOSTER)->val1;
	if(sc->getSCE(SC_SPARKCANDY))
		batk += 20;
	if(sc->getSCE(SC_SKF_ATK))
		batk += sc->getSCE(SC_SKF_ATK)->val1;
	if (sc->getSCE(SC_INTENSIVE_AIM))
		batk += 150;

	return (unsigned short)cap_value(batk,0,USHRT_MAX);
}

/**
 * Adds weapon attack modifications based on status changes
 * @param bl: Object to change watk [PC]
 * @param sc: Object's status change information
 * @param watk: Initial watk
 * @return modified watk with cap_value(watk,0,USHRT_MAX)
 */
static unsigned short status_calc_watk(struct block_list *bl, status_change *sc, int watk)
{
	if(!sc || !sc->count)
		return cap_value(watk,0,USHRT_MAX);

#ifndef RENEWAL
	if(sc->getSCE(SC_DRUMBATTLE))
		watk += sc->getSCE(SC_DRUMBATTLE)->val2;
#endif
	if (sc->getSCE(SC_IMPOSITIO))
		watk += sc->getSCE(SC_IMPOSITIO)->val2;
	if(sc->getSCE(SC_WATKFOOD))
		watk += sc->getSCE(SC_WATKFOOD)->val1;
	if (sc->getSCE(SC_VOLCANO) && bl->type == BL_MOB)
		watk += sc->getSCE(SC_VOLCANO)->val2;
	if(sc->getSCE(SC_MERC_ATKUP))
		watk += sc->getSCE(SC_MERC_ATKUP)->val2;
	if(sc->getSCE(SC_WATER_BARRIER))
		watk -= sc->getSCE(SC_WATER_BARRIER)->val2;
#ifndef RENEWAL
	if(sc->getSCE(SC_NIBELUNGEN)) {
		if (bl->type != BL_PC)
			watk += sc->getSCE(SC_NIBELUNGEN)->val2;
		else {
			TBL_PC *sd = (TBL_PC*)bl;
			short index = sd->equip_index[sd->state.lr_flag?EQI_HAND_L:EQI_HAND_R];

			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON && sd->inventory_data[index]->weapon_level == 4)
				watk += sc->getSCE(SC_NIBELUNGEN)->val2;
		}
	}
#endif
	if(sc->getSCE(SC_FIGHTINGSPIRIT))
		watk += sc->getSCE(SC_FIGHTINGSPIRIT)->val1;
	if (sc->getSCE(SC_SHIELDSPELL_ATK))
		watk += sc->getSCE(SC_SHIELDSPELL_ATK)->val2;
	if(sc->getSCE(SC_INSPIRATION))
		watk += sc->getSCE(SC_INSPIRATION)->val2;
	if(sc->getSCE(SC_GT_CHANGE))
		watk += sc->getSCE(SC_GT_CHANGE)->val2;
	if(sc->getSCE(SC__ENERVATION))
		watk -= watk * sc->getSCE(SC__ENERVATION)->val2 / 100;
	if(sc->getSCE(SC_STRIKING))
		watk += sc->getSCE(SC_STRIKING)->val2;
	if(sc->getSCE(SC_RUSHWINDMILL))
		watk += sc->getSCE(SC_RUSHWINDMILL)->val3;
	if(sc->getSCE(SC_FIRE_INSIGNIA) && sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 2)
		watk += 50;
	if((sc->getSCE(SC_FIRE_INSIGNIA) && sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 2)
	   || (sc->getSCE(SC_WATER_INSIGNIA) && sc->getSCE(SC_WATER_INSIGNIA)->val1 == 2)
	   || (sc->getSCE(SC_WIND_INSIGNIA) && sc->getSCE(SC_WIND_INSIGNIA)->val1 == 2)
	   || (sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 2))
		watk += watk * 10 / 100;
	if(sc->getSCE(SC_PYROTECHNIC_OPTION))
		watk += sc->getSCE(SC_PYROTECHNIC_OPTION)->val2;
	if(sc->getSCE(SC_HEATER_OPTION))
		watk += sc->getSCE(SC_HEATER_OPTION)->val2;
	if(sc->getSCE(SC_TROPIC_OPTION))
		watk += sc->getSCE(SC_TROPIC_OPTION)->val2;
	if( sc && sc->getSCE(SC_TIDAL_WEAPON) )
		watk += watk * sc->getSCE(SC_TIDAL_WEAPON)->val2 / 100;
	if(bl->type == BL_PC && sc->getSCE(SC_PYROCLASTIC))
		watk += sc->getSCE(SC_PYROCLASTIC)->val2;
	if(sc->getSCE(SC_ANGRIFFS_MODUS))
		watk += watk * sc->getSCE(SC_ANGRIFFS_MODUS)->val2/100;
	if(sc->getSCE(SC_ODINS_POWER))
		watk += 40 + 30 * sc->getSCE(SC_ODINS_POWER)->val1;
	if (sc->getSCE(SC_FLASHCOMBO))
		watk += sc->getSCE(SC_FLASHCOMBO)->val2;
	if (sc->getSCE(SC_CATNIPPOWDER))
		watk -= watk * sc->getSCE(SC_CATNIPPOWDER)->val2 / 100;
	if (sc->getSCE(SC_CHATTERING))
		watk += sc->getSCE(SC_CHATTERING)->val2;
	if (sc->getSCE(SC_SUNSTANCE))
		watk += watk * sc->getSCE(SC_SUNSTANCE)->val2 / 100;
	if (sc->getSCE(SC_SOULFALCON))
		watk += sc->getSCE(SC_SOULFALCON)->val2;
	if (sc->getSCE(SC_PACKING_ENVELOPE1))
		watk += sc->getSCE(SC_PACKING_ENVELOPE1)->val1;
	if (sc->getSCE(SC_POWERFUL_FAITH))
		watk += sc->getSCE(SC_POWERFUL_FAITH)->val2;
	if (sc->getSCE(SC_GUARD_STANCE))
		watk -= sc->getSCE(SC_GUARD_STANCE)->val3;

	return (unsigned short)cap_value(watk,0,USHRT_MAX);
}

#ifdef RENEWAL
/**
 * Adds equip magic attack modifications based on status changes [RENEWAL]
 * @param bl: Object to change matk [PC]
 * @param sc: Object's status change information
 * @param matk: Initial matk
 * @return modified matk with cap_value(matk,0,USHRT_MAX)
 */
static unsigned short status_calc_ematk(struct block_list *bl, status_change *sc, int matk)
{
	if (!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);

	if (sc->getSCE(SC_IMPOSITIO))
		matk += sc->getSCE(SC_IMPOSITIO)->val2;
	if (sc->getSCE(SC_MATKFOOD))
		matk += sc->getSCE(SC_MATKFOOD)->val1;
	if(sc->getSCE(SC_MANA_PLUS))
		matk += sc->getSCE(SC_MANA_PLUS)->val1;
	if(sc->getSCE(SC_COOLER_OPTION))
		matk += sc->getSCE(SC_COOLER_OPTION)->val2;
	if(sc->getSCE(SC_AQUAPLAY_OPTION))
		matk += sc->getSCE(SC_AQUAPLAY_OPTION)->val2;
	if(sc->getSCE(SC_CHILLY_AIR_OPTION))
		matk += sc->getSCE(SC_CHILLY_AIR_OPTION)->val2;
	if(sc->getSCE(SC_FIRE_INSIGNIA) && sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 3)
		matk += 50;
	if(sc->getSCE(SC_ODINS_POWER))
		matk += 40 + 30 * sc->getSCE(SC_ODINS_POWER)->val1; // 70 lvl1, 100lvl2
	if(sc->getSCE(SC_MOONLITSERENADE))
		matk += sc->getSCE(SC_MOONLITSERENADE)->val3;
	if(sc->getSCE(SC_IZAYOI))
		matk += 25 * sc->getSCE(SC_IZAYOI)->val1;
	if(sc->getSCE(SC_ZANGETSU))
		matk += sc->getSCE(SC_ZANGETSU)->val3;
	if(sc->getSCE(SC_QUEST_BUFF1))
		matk += sc->getSCE(SC_QUEST_BUFF1)->val1;
	if(sc->getSCE(SC_QUEST_BUFF2))
		matk += sc->getSCE(SC_QUEST_BUFF2)->val1;
	if(sc->getSCE(SC_QUEST_BUFF3))
		matk += sc->getSCE(SC_QUEST_BUFF3)->val1;
	if(sc->getSCE(SC_MTF_MATK2))
		matk += sc->getSCE(SC_MTF_MATK2)->val1;
	if (sc->getSCE(SC_CATNIPPOWDER))
		matk -= matk * sc->getSCE(SC_CATNIPPOWDER)->val2 / 100;
	if (sc->getSCE(SC_CHATTERING))
		matk += sc->getSCE(SC_CHATTERING)->val2;
	if (sc->getSCE(SC_DORAM_MATK))
		matk += sc->getSCE(SC_DORAM_MATK)->val1;
	if (sc->getSCE(SC_SOULFAIRY))
		matk += sc->getSCE(SC_SOULFAIRY)->val2;
	if (sc->getSCE(SC__AUTOSHADOWSPELL))
		matk += sc->getSCE(SC__AUTOSHADOWSPELL)->val4 * 5;
	if (sc->getSCE(SC_INSPIRATION))
		matk += sc->getSCE(SC_INSPIRATION)->val2;
	if (sc->getSCE(SC_PACKING_ENVELOPE2))
		matk += sc->getSCE(SC_PACKING_ENVELOPE2)->val1;
	if(sc->getSCE(SC_ULTIMATECOOK))
		matk += 30;
	if (sc->getSCE(SC_MAGICCANDY))
		matk += 30;
	if (sc->getSCE(SC_SKF_MATK))
		matk += sc->getSCE(SC_SKF_MATK)->val1;

	return (unsigned short)cap_value(matk,0,USHRT_MAX);
}
#endif

/**
 * Adds magic attack modifications based on status changes
 * @param bl: Object to change matk [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param matk: Initial matk
 * @return modified matk with cap_value(matk,0,USHRT_MAX)
 */
static unsigned short status_calc_matk(struct block_list *bl, status_change *sc, int matk)
{
	if(!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);
#ifndef RENEWAL
	/// Take note fixed value first before % modifiers [PRE-RENEWAL]
	if (sc->getSCE(SC_MATKFOOD))
		matk += sc->getSCE(SC_MATKFOOD)->val1;
	if (sc->getSCE(SC_MANA_PLUS))
		matk += sc->getSCE(SC_MANA_PLUS)->val1;
	if (sc->getSCE(SC_AQUAPLAY_OPTION))
		matk += sc->getSCE(SC_AQUAPLAY_OPTION)->val2;
	if (sc->getSCE(SC_CHILLY_AIR_OPTION))
		matk += sc->getSCE(SC_CHILLY_AIR_OPTION)->val2;
	if (sc->getSCE(SC_COOLER_OPTION))
		matk += sc->getSCE(SC_COOLER_OPTION)->val2;
	if (sc->getSCE(SC_FIRE_INSIGNIA) && sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 3)
		matk += 50;
	if (sc->getSCE(SC_ODINS_POWER))
		matk += 40 + 30 * sc->getSCE(SC_ODINS_POWER)->val1; // 70 lvl1, 100lvl2
	if (sc->getSCE(SC_IZAYOI))
		matk += 25 * sc->getSCE(SC_IZAYOI)->val1;
	if (sc->getSCE(SC_MTF_MATK2))
		matk += sc->getSCE(SC_MTF_MATK2)->val1;
	if (sc->getSCE(SC_ULTIMATECOOK))
		matk += 30;
	if (sc->getSCE(SC_MAGICCANDY))
		matk += 30;
	if (sc->getSCE(SC_SKF_MATK))
		matk += sc->getSCE(SC_SKF_MATK)->val1;
#endif
	if (sc->getSCE(SC_MATKPOTION))
		matk += sc->getSCE(SC_MATKPOTION)->val1;
	if (sc->getSCE(SC_LIMIT_POWER_BOOSTER))
		matk += sc->getSCE(SC_LIMIT_POWER_BOOSTER)->val1;
	if (sc->getSCE(SC_ALMIGHTY))
		matk += 30;
	if (sc->getSCE(SC_2011RWC_SCROLL))
		matk += 30;
	if (sc->getSCE(SC_ZANGETSU))
		matk += sc->getSCE(SC_ZANGETSU)->val3;
	if (sc->getSCE(SC_QUEST_BUFF1))
		matk += sc->getSCE(SC_QUEST_BUFF1)->val1;
	if (sc->getSCE(SC_QUEST_BUFF2))
		matk += sc->getSCE(SC_QUEST_BUFF2)->val1;
	if (sc->getSCE(SC_QUEST_BUFF3))
		matk += sc->getSCE(SC_QUEST_BUFF3)->val1;
#ifndef RENEWAL
	if (sc->getSCE(SC_MAGICPOWER) && sc->getSCE(SC_MAGICPOWER)->val4)
		matk += matk * sc->getSCE(SC_MAGICPOWER)->val3/100;
#endif
	if (sc->getSCE(SC_MINDBREAKER))
		matk += matk * sc->getSCE(SC_MINDBREAKER)->val2/100;
	if (sc->getSCE(SC_INCMATKRATE))
		matk += matk * sc->getSCE(SC_INCMATKRATE)->val1/100;
	if (sc->getSCE(SC_MOONLITSERENADE))
		matk += sc->getSCE(SC_MOONLITSERENADE)->val3/100;
	if (sc->getSCE(SC_MTF_MATK))
		matk += matk * sc->getSCE(SC_MTF_MATK)->val1 / 100;
	if (sc->getSCE(SC_SHRIMP))
		matk += matk * sc->getSCE(SC_SHRIMP)->val2 / 100;
#ifdef RENEWAL
	if (sc->getSCE(SC_VOLCANO))
		matk += sc->getSCE(SC_VOLCANO)->val2;
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_MATKRATE)
		matk += matk * 20 / 100;
#endif
	if (sc->getSCE(SC_SHIELDSPELL_ATK))
		matk += sc->getSCE(SC_SHIELDSPELL_ATK)->val2;
	if (sc->getSCE(SC_CLIMAX_DES_HU))
		matk += 100;

	return (unsigned short)cap_value(matk,0,USHRT_MAX);
}

/**
 * Adds critical modifications based on status changes
 * @param bl: Object to change critical [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param critical: Initial critical
 * @return modified critical with cap_value(critical,10,USHRT_MAX)
 */
static signed short status_calc_critical(struct block_list *bl, status_change *sc, int critical)
{
	if(!sc || !sc->count)
		return cap_value(critical,10,SHRT_MAX);

	if (sc->getSCE(SC_INCCRI))
		critical += sc->getSCE(SC_INCCRI)->val2;
	if (sc->getSCE(SC_EP16_2_BUFF_SC))
		critical += 300;// crit +30
	if (sc->getSCE(SC_CRIFOOD))
		critical += sc->getSCE(SC_CRIFOOD)->val1;
	if (sc->getSCE(SC_EXPLOSIONSPIRITS))
		critical += sc->getSCE(SC_EXPLOSIONSPIRITS)->val2;
	if (sc->getSCE(SC_FORTUNE))
		critical += sc->getSCE(SC_FORTUNE)->val2;
	if (sc->getSCE(SC_TRUESIGHT))
		critical += sc->getSCE(SC_TRUESIGHT)->val2;
	if (sc->getSCE(SC_CLOAKING))
		critical += critical;
#ifdef RENEWAL
	if (sc->getSCE(SC_SPEARQUICKEN))
		critical += 3*sc->getSCE(SC_SPEARQUICKEN)->val1*10;
	if (sc->getSCE(SC_TWOHANDQUICKEN))
		critical += (2 + sc->getSCE(SC_TWOHANDQUICKEN)->val1) * 10;
#endif
	if (sc->getSCE(SC__INVISIBILITY))
		critical += sc->getSCE(SC__INVISIBILITY)->val3 * 10;
	if (sc->getSCE(SC__UNLUCKY))
		critical -= sc->getSCE(SC__UNLUCKY)->val2;
	if (sc->getSCE(SC_SOULSHADOW))
		critical += 10 * sc->getSCE(SC_SOULSHADOW)->val3;
	if(sc->getSCE(SC_BEYONDOFWARCRY))
		critical += sc->getSCE(SC_BEYONDOFWARCRY)->val3;
	if (sc->getSCE(SC_MTF_HITFLEE))
		critical += sc->getSCE(SC_MTF_HITFLEE)->val1;
	if (sc->getSCE(SC_PACKING_ENVELOPE9))
		critical += sc->getSCE(SC_PACKING_ENVELOPE9)->val1 * 10;
	if (sc->getSCE(SC_INTENSIVE_AIM))
		critical += 300;
	if (sc->getSCE(SC_BUCHEDENOEL))
		critical += sc->getSCE(SC_BUCHEDENOEL)->val3 * 10;

	return (short)cap_value(critical,10,SHRT_MAX);
}

/**
 * Adds hit modifications based on status changes
 * @param bl: Object to change hit [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param hit: Initial hit
 * @return modified hit with cap_value(hit,1,USHRT_MAX)
 */
static signed short status_calc_hit(struct block_list *bl, status_change *sc, int hit)
{
	if(!sc || !sc->count)
		return cap_value(hit,1,SHRT_MAX);

	if(sc->getSCE(SC_INCHIT))
		hit += sc->getSCE(SC_INCHIT)->val1;
	if(sc->getSCE(SC_HITFOOD))
		hit += sc->getSCE(SC_HITFOOD)->val1;
	if(sc->getSCE(SC_TRUESIGHT))
		hit += sc->getSCE(SC_TRUESIGHT)->val3;
	if(sc->getSCE(SC_HUMMING))
		hit += sc->getSCE(SC_HUMMING)->val2;
	if(sc->getSCE(SC_CONCENTRATION))
		hit += sc->getSCE(SC_CONCENTRATION)->val3;
	if(sc->getSCE(SC_INSPIRATION))
		hit += 12 * sc->getSCE(SC_INSPIRATION)->val1;
	if(sc->getSCE(SC_ADJUSTMENT))
		hit -= 30;
	if(sc->getSCE(SC_INCREASING))
		hit += 20; // RockmanEXE; changed based on updated [Reddozen]
	if(sc->getSCE(SC_MERC_HITUP))
		hit += sc->getSCE(SC_MERC_HITUP)->val2;
	if(sc->getSCE(SC_MTF_HITFLEE))
		hit += sc->getSCE(SC_MTF_HITFLEE)->val1;
	if(sc->getSCE(SC_INCHITRATE))
		hit += hit * sc->getSCE(SC_INCHITRATE)->val1/100;
	if (sc->getSCE(SC_POWERUP))
		hit += hit * sc->getSCE(SC_POWERUP)->val2 / 100;
	if(sc->getSCE(SC_BLIND))
		hit -= hit * 25/100;
	if(sc->getSCE(SC_HEAT_BARREL))
		hit -= sc->getSCE(SC_HEAT_BARREL)->val4;
	if(sc->getSCE(SC__GROOMY))
		hit -= hit * sc->getSCE(SC__GROOMY)->val3 / 100;
	if(sc->getSCE(SC_FEAR))
		hit -= hit * 20 / 100;
	if (sc->getSCE(SC_ASH))
		hit -= hit * sc->getSCE(SC_ASH)->val2 / 100;
	if (sc->getSCE(SC_TEARGAS))
		hit -= hit * 50 / 100;
	if(sc->getSCE(SC_ILLUSIONDOPING))
		hit -= sc->getSCE(SC_ILLUSIONDOPING)->val2;
	if (sc->getSCE(SC_MTF_ASPD))
		hit += sc->getSCE(SC_MTF_ASPD)->val2;
#ifdef RENEWAL
	if (sc->getSCE(SC_BLESSING))
		hit += sc->getSCE(SC_BLESSING)->val1 * 2;
	if (sc->getSCE(SC_TWOHANDQUICKEN))
		hit += sc->getSCE(SC_TWOHANDQUICKEN)->val1 * 2;
	if (sc->getSCE(SC_ADRENALINE))
		hit += sc->getSCE(SC_ADRENALINE)->val1 * 3 + 5;
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_HIT)
		hit += 50;
#endif
	if (sc->getSCE(SC_SOULFALCON))
		hit += sc->getSCE(SC_SOULFALCON)->val3;
	if (sc->getSCE(SC_SATURDAYNIGHTFEVER))
		hit -= 50 + 50 * sc->getSCE(SC_SATURDAYNIGHTFEVER)->val1;
	if (sc->getSCE(SC_PACKING_ENVELOPE10))
		hit += sc->getSCE(SC_PACKING_ENVELOPE10)->val1;
	if (sc->getSCE(SC_ABYSS_SLAYER))
		hit += sc->getSCE(SC_ABYSS_SLAYER)->val3;
	if (sc->getSCE(SC_LIMIT_POWER_BOOSTER))
		hit += sc->getSCE(SC_LIMIT_POWER_BOOSTER)->val1;
	if (sc->getSCE(SC_ACARAJE))
		hit += 5;
	if (sc->getSCE(SC_INTENSIVE_AIM))
		hit += 250;
	if (sc->getSCE(SC_BUCHEDENOEL))
		hit += sc->getSCE(SC_BUCHEDENOEL)->val2;

	return (short)cap_value(hit,1,SHRT_MAX);
}

/**
 * Adds flee modifications based on status changes
 * @param bl: Object to change flee [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param flee: Initial flee
 * @return modified flee with cap_value(flee,1,USHRT_MAX)
 */
static signed short status_calc_flee(struct block_list *bl, status_change *sc, int flee)
{
	if( bl->type == BL_PC ) {
		struct map_data *mapdata = map_getmapdata(bl->m);

		if( mapdata_flag_gvg(mapdata) )
			flee -= flee * battle_config.gvg_flee_penalty/100;
		else if( mapdata->getMapFlag(MF_BATTLEGROUND) )
			flee -= flee * battle_config.bg_flee_penalty/100;
	}

	if(!sc || !sc->count)
		return cap_value(flee,1,SHRT_MAX);
	if (sc->getSCE(SC_POISON_MIST))
		return 0;
	if(sc->getSCE(SC_OVERED_BOOST)) //Should be final and unmodifiable by any means
		return sc->getSCE(SC_OVERED_BOOST)->val2;

	// Fixed value
	if(sc->getSCE(SC_INCFLEE))
		flee += sc->getSCE(SC_INCFLEE)->val1;
	if(sc->getSCE(SC_FLEEFOOD))
		flee += sc->getSCE(SC_FLEEFOOD)->val1;
	if(sc->getSCE(SC_WHISTLE))
		flee += sc->getSCE(SC_WHISTLE)->val2;
	if(sc->getSCE(SC_WINDWALK))
		flee += sc->getSCE(SC_WINDWALK)->val2;
	if(sc->getSCE(SC_VIOLENTGALE))
		flee += sc->getSCE(SC_VIOLENTGALE)->val2;
	if(sc->getSCE(SC_MOON_COMFORT)) // SG skill [Komurka]
		flee += sc->getSCE(SC_MOON_COMFORT)->val2;
	if(sc->getSCE(SC_CLOSECONFINE))
		flee += sc->getSCE(SC_CLOSECONFINE)->val3;
	if (sc->getSCE(SC_ANGRIFFS_MODUS))
		flee -= sc->getSCE(SC_ANGRIFFS_MODUS)->val3;
	if(sc->getSCE(SC_ADJUSTMENT))
		flee += 30;
	if(sc->getSCE(SC_SPEED))
		flee += 10 + sc->getSCE(SC_SPEED)->val1 * 10;
	if(sc->getSCE(SC_GATLINGFEVER))
		flee -= sc->getSCE(SC_GATLINGFEVER)->val4;
	if(sc->getSCE(SC_PARTYFLEE))
		flee += sc->getSCE(SC_PARTYFLEE)->val1 * 10;
	if(sc->getSCE(SC_MERC_FLEEUP))
		flee += sc->getSCE(SC_MERC_FLEEUP)->val2;
	if( sc->getSCE(SC_HALLUCINATIONWALK) )
		flee += sc->getSCE(SC_HALLUCINATIONWALK)->val2;
	if( sc->getSCE(SC_NPC_HALLUCINATIONWALK) )
		flee += sc->getSCE(SC_NPC_HALLUCINATIONWALK)->val2;
	if(sc->getSCE(SC_MTF_HITFLEE))
		flee += sc->getSCE(SC_MTF_HITFLEE)->val2;
	if( sc->getSCE(SC_WATER_BARRIER) )
		flee -= sc->getSCE(SC_WATER_BARRIER)->val2;
	if( sc->getSCE(SC_C_MARKER) )
		flee -= sc->getSCE(SC_C_MARKER)->val3;
#ifdef RENEWAL
	if( sc->getSCE(SC_SPEARQUICKEN) )
		flee += 2 * sc->getSCE(SC_SPEARQUICKEN)->val1;
	if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_FLEE)
		flee += 50;
#endif

	// Rate value
	if(sc->getSCE(SC_INCFLEERATE))
		flee += flee * sc->getSCE(SC_INCFLEERATE)->val1/100;
	if (sc->getSCE(SC_AGIUP))
		flee += flee * sc->getSCE(SC_AGIUP)->val2 / 100;
	if(sc->getSCE(SC_SPIDERWEB) || sc->getSCE(SC_WIDEWEB))
		flee -= flee * 50/100;
	if(sc->getSCE(SC_BERSERK))
		flee -= flee * 50/100;
	if(sc->getSCE(SC_BLIND))
		flee -= flee * 25/100;
	if(sc->getSCE(SC_FEAR))
		flee -= flee * 20 / 100;
	if(sc->getSCE(SC_PARALYSE) && sc->getSCE(SC_PARALYSE)->val3 == 1)
		flee -= flee * 10 / 100;
	if(sc->getSCE(SC_INFRAREDSCAN))
		flee -= flee * 30 / 100;
	if( sc->getSCE(SC__LAZINESS) )
		flee -= flee * sc->getSCE(SC__LAZINESS)->val3 / 100;
	if( sc->getSCE(SC_GLOOMYDAY) )
		flee -= flee * sc->getSCE(SC_GLOOMYDAY)->val2 / 100;
	if( sc->getSCE(SC_SATURDAYNIGHTFEVER) )
		flee -= 20 + 30 * sc->getSCE(SC_SATURDAYNIGHTFEVER)->val1;
	if( sc->getSCE(SC_WIND_STEP_OPTION) )
		flee += flee * sc->getSCE(SC_WIND_STEP_OPTION)->val2 / 100;
	if( sc->getSCE(SC_TINDER_BREAKER) || sc->getSCE(SC_TINDER_BREAKER2) )
		flee -= flee * 50 / 100;
	if( sc->getSCE(SC_ZEPHYR) )
		flee += sc->getSCE(SC_ZEPHYR)->val2;
	if(sc->getSCE(SC_ASH))
		flee -= flee * sc->getSCE(SC_ASH)->val4 / 100;
	if (sc->getSCE(SC_GOLDENE_FERSE))
		flee += flee * sc->getSCE(SC_GOLDENE_FERSE)->val2 / 100;
	if (sc->getSCE(SC_SMOKEPOWDER))
		flee += flee * 20 / 100;
	if (sc->getSCE(SC_TEARGAS))
		flee -= flee * 50 / 100;
	//if( sc->getSCE(SC_C_MARKER) )
	//	flee -= (flee * sc->getSCE(SC_C_MARKER)->val3) / 100;
	if (sc->getSCE(SC_GROOMING))
		flee += sc->getSCE(SC_GROOMING)->val2;
	if (sc->getSCE(SC_PACKING_ENVELOPE5))
		flee += sc->getSCE(SC_PACKING_ENVELOPE5)->val1;
	if (sc->getSCE(SC_LIMIT_POWER_BOOSTER))
		flee += sc->getSCE(SC_LIMIT_POWER_BOOSTER)->val1;
	if (sc->getSCE(SC_MYSTICPOWDER))
		flee += 20;

	return (short)cap_value(flee,1,SHRT_MAX);
}

/**
 * Adds perfect flee modifications based on status changes
 * @param bl: Object to change flee2 [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param flee2: Initial flee2
 * @return modified flee2 with cap_value(flee2,10,USHRT_MAX)
 */
static signed short status_calc_flee2(struct block_list *bl, status_change *sc, int flee2)
{
	if(!sc || !sc->count)
		return cap_value(flee2,10,SHRT_MAX);

	if(sc->getSCE(SC_INCFLEE2))
		flee2 += sc->getSCE(SC_INCFLEE2)->val2;
	if(sc->getSCE(SC_WHISTLE))
		flee2 += sc->getSCE(SC_WHISTLE)->val3*10;
	if(sc->getSCE(SC__UNLUCKY))
		flee2 -= flee2 * sc->getSCE(SC__UNLUCKY)->val2 / 100;
	if (sc->getSCE(SC_HISS))
		flee2 += sc->getSCE(SC_HISS)->val2*10;
	if (sc->getSCE(SC_DORAM_FLEE2))
		flee2 += sc->getSCE(SC_DORAM_FLEE2)->val1;

	return (short)cap_value(flee2,10,SHRT_MAX);
}

/**
 * Adds defense (left-side) modifications based on status changes
 * @param bl: Object to change def [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param def: Initial def
 * @return modified def with cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX)
 */
static defType status_calc_def(struct block_list *bl, status_change *sc, int def)
{
	if(!sc || !sc->count)
		return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);

	if(sc->getSCE(SC_BERSERK))
		return 0;
#ifdef RENEWAL
	if(sc->getSCE(SC_ETERNALCHAOS))
		return 0;
#endif
	if(sc->getSCE(SC_BARRIER))
		return 100;
	if(sc->getSCE(SC_KEEPING))
		return 90;
#ifndef RENEWAL /// Steel Body does not provide 90 DEF in [RENEWAL]
	if(sc->getSCE(SC_STEELBODY))
		return 90;
#endif
	if (sc->getSCE(SC_NYANGGRASS)) {
		if (bl->type == BL_PC)
			return 0;
		else
			return def /= 2;
	}
	if(sc->getSCE(SC_DEFSET))
		return sc->getSCE(SC_DEFSET)->val1;

	if(sc->getSCE(SC_DRUMBATTLE))
		def += sc->getSCE(SC_DRUMBATTLE)->val3;
#ifdef RENEWAL
	if (sc->getSCE(SC_ASSUMPTIO))
		def += sc->getSCE(SC_ASSUMPTIO)->val1 * 50;
#endif
	if (bl->type == BL_HOM && sc->getSCE(SC_DEFENCE))
		def += sc->getSCE(SC_DEFENCE)->val2;
	if(sc->getSCE(SC_INCDEFRATE))
		def += def * sc->getSCE(SC_INCDEFRATE)->val1/100;
	if(sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 2)
		def += 50;
	if(sc->getSCE(SC_ODINS_POWER))
		def -= 20 * sc->getSCE(SC_ODINS_POWER)->val1;
	if( sc->getSCE(SC_ANGRIFFS_MODUS) )
		def -= 20 + 10 * sc->getSCE(SC_ANGRIFFS_MODUS)->val1;
	if(sc->getSCE(SC_STONEHARDSKIN))
		def += sc->getSCE(SC_STONEHARDSKIN)->val1;
	if(sc->getSCE(SC_STONE))
		def /= 2;
	if(sc->getSCE(SC_FREEZE))
		def /= 2;
	if(sc->getSCE(SC_POISON) || sc->getSCE(SC_DPOISON) && bl->type != BL_PC)
		def = def * 75 / 100; //Should round down
	if(sc->getSCE(SC_SIGNUMCRUCIS))
		def -= def * sc->getSCE(SC_SIGNUMCRUCIS)->val2/100;
	if(sc->getSCE(SC_CONCENTRATION))
		def -= def * sc->getSCE(SC_CONCENTRATION)->val4/100;
	if(sc->getSCE(SC_SKE))
		def /= 2;
	if(sc->getSCE(SC_PROVOKE) && bl->type != BL_PC) // Provoke doesn't alter player defense->
		def -= def * sc->getSCE(SC_PROVOKE)->val3/100;
	if(sc->getSCE(SC_STRIPSHIELD) && bl->type != BL_PC) // Player doesn't have def reduction only equip removed
		def -= def * sc->getSCE(SC_STRIPSHIELD)->val2/100;
	if (sc->getSCE(SC_FLING))
		def -= def * (sc->getSCE(SC_FLING)->val2)/100;
	if( sc->getSCE(SC_FREEZING) )
		def -= def * (bl->type == BL_PC ? 30 : 10) / 100;
	if( sc->getSCE(SC_ANALYZE) )
		def -= def * (14 * sc->getSCE(SC_ANALYZE)->val1) / 100;
	if( sc->getSCE(SC_NEUTRALBARRIER) )
		def += def * sc->getSCE(SC_NEUTRALBARRIER)->val2 / 100;
	if( sc->getSCE(SC_PRESTIGE) )
		def += sc->getSCE(SC_PRESTIGE)->val3;
	if( sc->getSCE(SC_BANDING) && sc->getSCE(SC_BANDING)->val2 > 1 )
		def += 6 * sc->getSCE(SC_BANDING)->val1;
	if( sc->getSCE(SC_ECHOSONG) )
		def += sc->getSCE(SC_ECHOSONG)->val3;
	if( sc->getSCE(SC_CAMOUFLAGE) )
		def -= def * 5 * sc->getSCE(SC_CAMOUFLAGE)->val3 / 100;
	if( sc->getSCE(SC_SOLID_SKIN_OPTION) )
		def += def * sc->getSCE(SC_SOLID_SKIN_OPTION)->val2 / 100;
	if( sc->getSCE(SC_ROCK_CRUSHER) )
		def -= def * sc->getSCE(SC_ROCK_CRUSHER)->val2 / 100;
	if( sc->getSCE(SC_POWER_OF_GAIA) )
		def += def * sc->getSCE(SC_POWER_OF_GAIA)->val2 / 100;
	if(sc->getSCE(SC_ASH))
		def -= def * sc->getSCE(SC_ASH)->val3/100;
	if( sc->getSCE(SC_OVERED_BOOST) && bl->type == BL_HOM )
		def -= def * sc->getSCE(SC_OVERED_BOOST)->val4 / 100;
	if(sc->getSCE(SC_GLASTHEIM_ITEMDEF))
		def += sc->getSCE(SC_GLASTHEIM_ITEMDEF)->val1;
	if (sc->getSCE(SC_SOULGOLEM))
		def += sc->getSCE(SC_SOULGOLEM)->val2;
	if (sc->getSCE(SC_STONE_WALL))
		def += sc->getSCE(SC_STONE_WALL)->val2;
	if( sc->getSCE(SC_PACKING_ENVELOPE7) )
		def += sc->getSCE(SC_PACKING_ENVELOPE7)->val1;
	if (sc->getSCE(SC_D_MACHINE))
		def += sc->getSCE(SC_D_MACHINE)->val2;
	if (sc->getSCE(SC_CLIMAX_CRYIMP))
		def += 300;
	if (sc->getSCE(SC_GUARD_STANCE))
		def += sc->getSCE(SC_GUARD_STANCE)->val2;
	if (sc->getSCE(SC_ATTACK_STANCE))
		def -= sc->getSCE(SC_ATTACK_STANCE)->val2;
	if (sc->getSCE(SC_M_DEFSCROLL))
		def += sc->getSCE(SC_M_DEFSCROLL)->val1;

	return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);
}

/**
 * Adds defense (right-side) modifications based on status changes
 * @param bl: Object to change def2 [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param def2: Initial def2
 * @return modified def2 with cap_value(def2,SHRT_MIN,SHRT_MAX)
 */
static signed short status_calc_def2(struct block_list *bl, status_change *sc, int def2)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(def2,1,SHRT_MAX);
#endif

	if(sc->getSCE(SC_BERSERK))
		return 0;
	if(sc->getSCE(SC_ETERNALCHAOS))
		return 0;
	if(sc->getSCE(SC_DEFSET))
		return sc->getSCE(SC_DEFSET)->val1;

	if(sc->getSCE(SC_SUN_COMFORT))
		def2 += sc->getSCE(SC_SUN_COMFORT)->val2;
#ifdef RENEWAL
	if (sc->getSCE(SC_SKA))
		def2 += 80;
#endif
	if(sc->getSCE(SC_ANGELUS))
#ifdef RENEWAL /// The VIT stat bonus is boosted by angelus [RENEWAL]
		def2 += status_get_vit(bl) / 2 * sc->getSCE(SC_ANGELUS)->val2/100;
#else
		def2 += def2 * sc->getSCE(SC_ANGELUS)->val2/100;
	if(sc->getSCE(SC_CONCENTRATION))
		def2 -= def2 * sc->getSCE(SC_CONCENTRATION)->val4/100;
#endif
	if(sc->getSCE(SC_POISON) || sc->getSCE(SC_DPOISON))
		def2 = def2 * 75 / 100; //Should round down
	if(sc->getSCE(SC_SKE))
		def2 -= def2 * 50/100;
	if(sc->getSCE(SC_PROVOKE))
		def2 -= def2 * sc->getSCE(SC_PROVOKE)->val3/100;
	if(sc->getSCE(SC_JOINTBEAT))
		def2 -= def2 * ( sc->getSCE(SC_JOINTBEAT)->val2&BREAK_SHOULDER ? 50 : 0 ) / 100
			  + def2 * ( sc->getSCE(SC_JOINTBEAT)->val2&BREAK_WAIST ? 25 : 0 ) / 100;
	if(sc->getSCE(SC_FLING))
		def2 -= def2 * (sc->getSCE(SC_FLING)->val3)/100;
	if(sc->getSCE(SC_ANALYZE))
		def2 -= def2 * (14 * sc->getSCE(SC_ANALYZE)->val1) / 100;
	if(sc->getSCE(SC_ASH))
		def2 -= def2 * sc->getSCE(SC_ASH)->val3/100;
	if (sc->getSCE(SC_PARALYSIS))
		def2 -= def2 * sc->getSCE(SC_PARALYSIS)->val2 / 100;
	if(sc->getSCE(SC_EQC))
		def2 -= def2 * sc->getSCE(SC_EQC)->val2 / 100;
	if( sc->getSCE(SC_CAMOUFLAGE) )
		def2 -= def2 * 5 * sc->getSCE(SC_CAMOUFLAGE)->val3 / 100;

#ifdef RENEWAL
	return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
	return (short)cap_value(def2,1,SHRT_MAX);
#endif
}

/**
 * Adds magic defense (left-side) modifications based on status changes
 * @param bl: Object to change mdef [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param mdef: Initial mdef
 * @return modified mdef with cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX)
 */
static defType status_calc_mdef(struct block_list *bl, status_change *sc, int mdef)
{
	if(!sc || !sc->count)
		return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);

	if(sc->getSCE(SC_BERSERK))
		return 0;
	if(sc->getSCE(SC_BARRIER))
		return 100;

#ifndef RENEWAL /// Steel Body does not provide 90 MDEF in [RENEWAL]
	if(sc->getSCE(SC_STEELBODY))
		return 90;
#endif
	if (sc->getSCE(SC_NYANGGRASS)) {
		if (bl->type == BL_PC)
			return 0;
		else
			return mdef / 2;
	}
	if(sc->getSCE(SC_MDEFSET))
		return sc->getSCE(SC_MDEFSET)->val1;

	if(sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 3)
		mdef += 50;
	if(sc->getSCE(SC_ENDURE) && !sc->getSCE(SC_ENDURE)->val3) // It has been confirmed that Eddga card grants 1 MDEF, not 0, not 10, but 1.
		mdef += (sc->getSCE(SC_ENDURE)->val4 == 0) ? sc->getSCE(SC_ENDURE)->val1 : 1;
	if(sc->getSCE(SC_STONEHARDSKIN))
		mdef += sc->getSCE(SC_STONEHARDSKIN)->val1;
	if(sc->getSCE(SC_STONE))
		mdef += 25 * mdef / 100;
	if(sc->getSCE(SC_FREEZE))
		mdef += 25 * mdef / 100;
	if(sc->getSCE(SC_BURNING))
		mdef -= 25 * mdef / 100;
	if( sc->getSCE(SC_NEUTRALBARRIER) )
		mdef += mdef * sc->getSCE(SC_NEUTRALBARRIER)->val2 / 100;
	if(sc->getSCE(SC_ANALYZE))
		mdef -= mdef * ( 14 * sc->getSCE(SC_ANALYZE)->val1 ) / 100;
	if(sc->getSCE(SC_SYMPHONYOFLOVER))
		mdef += mdef * sc->getSCE(SC_SYMPHONYOFLOVER)->val3 / 100;
	if (sc->getSCE(SC_ODINS_POWER))
		mdef -= 20 * sc->getSCE(SC_ODINS_POWER)->val1;
	if(sc->getSCE(SC_GLASTHEIM_ITEMDEF))
		mdef += sc->getSCE(SC_GLASTHEIM_ITEMDEF)->val2;
	if (sc->getSCE(SC_SOULGOLEM))
		mdef += sc->getSCE(SC_SOULGOLEM)->val3;
	if (sc->getSCE(SC_STONE_WALL))
		mdef += sc->getSCE(SC_STONE_WALL)->val3;
	if (sc->getSCE(SC_PACKING_ENVELOPE8))
		mdef += sc->getSCE(SC_PACKING_ENVELOPE8)->val1;
	if (sc->getSCE(SC_CLIMAX_CRYIMP))
		mdef += 100;
	if (sc->getSCE(SC_M_DEFSCROLL))
		mdef += sc->getSCE(SC_M_DEFSCROLL)->val2;

	return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);
}

/**
 * Adds magic defense (right-side) modifications based on status changes
 * @param bl: Object to change mdef2 [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param mdef2: Initial mdef2
 * @return modified mdef2 with cap_value(mdef2,SHRT_MIN,SHRT_MAX)
 */
static signed short status_calc_mdef2(struct block_list *bl, status_change *sc, int mdef2)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(mdef2,1,SHRT_MAX);
#endif

	if(sc->getSCE(SC_BERSERK))
		return 0;
	if(sc->getSCE(SC_SKA))
		return 90;
	if(sc->getSCE(SC_MDEFSET))
		return sc->getSCE(SC_MDEFSET)->val1;

	if(sc->getSCE(SC_MINDBREAKER))
		mdef2 -= mdef2 * sc->getSCE(SC_MINDBREAKER)->val3/100;
	if(sc->getSCE(SC_BURNING))
		mdef2 -= mdef2 * 25 / 100;
	if(sc->getSCE(SC_ANALYZE))
		mdef2 -= mdef2 * (14 * sc->getSCE(SC_ANALYZE)->val1) / 100;

#ifdef RENEWAL
	return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
	return (short)cap_value(mdef2,1,SHRT_MAX);
#endif
}

/**
 * Adds speed modifications based on status changes
 * @param bl: Object to change speed [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param speed: Initial speed
 * @return modified speed with cap_value(speed,10,USHRT_MAX)
 */
static unsigned short status_calc_speed(struct block_list *bl, status_change *sc, int speed)
{
	TBL_PC* sd = BL_CAST(BL_PC, bl);
	int speed_rate = 100;

	if (sc == nullptr || (sd && sd->state.permanent_speed))
		return (unsigned short)cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);

	if (sd && pc_ismadogear(sd)) { // Mado speed is not affected by other statuses
		int val = 0;

		if (pc_checkskill(sd, NC_MADOLICENCE) < 5)
			val = 50 - 10 * pc_checkskill(sd, NC_MADOLICENCE);
		else
			val -= 25;
		if (sc->getSCE(SC_ACCELERATION))
			val -= 25;
		speed += speed * val / 100;

		return (unsigned short)cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
	}

	if( sd && sd->ud.skilltimer != INVALID_TIMER && (pc_checkskill(sd,SA_FREECAST) > 0 || sd->ud.skill_id == LG_EXEEDBREAK) ) {
		if( sd->ud.skill_id == LG_EXEEDBREAK )
			speed_rate = 160 - 10 * sd->ud.skill_lv;
		else
			speed_rate = 175 - 5 * pc_checkskill(sd,SA_FREECAST);
	} else {
		int val = 0;

		// GetMoveHasteValue2()
		if( sc->getSCE(SC_FUSION) )
			val = 25;
		else if( sd ) {
			if( pc_isriding(sd) || sd->sc.option&OPTION_DRAGON )
				val = 25; // Same bonus
			else if( pc_isridingwug(sd) )
				val = 15 + 5 * pc_checkskill(sd, RA_WUGRIDER);
			else if( sc->getSCE(SC_ALL_RIDING) )
				val = battle_config.rental_mount_speed_boost;
		}
		speed_rate -= val;

		// GetMoveSlowValue()
		if( sd && sc->getSCE(SC_HIDING) && pc_checkskill(sd,RG_TUNNELDRIVE) > 0 )
			val = 120 - 6 * pc_checkskill(sd,RG_TUNNELDRIVE);
		else if( sd && sc->getSCE(SC_CHASEWALK) && sc->getSCE(SC_CHASEWALK)->val3 < 0 )
			val = sc->getSCE(SC_CHASEWALK)->val3;
		else {
			val = 0;
			// Longing for Freedom/Special Singer cancels song/dance penalty
#ifdef RENEWAL
			if (sc->getSCE(SC_ENSEMBLEFATIGUE))
				val = max(val, sc->getSCE(SC_ENSEMBLEFATIGUE)->val2);
#else
			if( sc->getSCE(SC_LONGING) )
				val = max( val, 50 - 10 * sc->getSCE(SC_LONGING)->val1 );
#endif
			else
			if( sd && sc->getSCE(SC_DANCING) )
				val = max( val, 500 - (40 + 10 * (sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_BARDDANCER)) * pc_checkskill(sd,(sd->status.sex?BA_MUSICALLESSON:DC_DANCINGLESSON)) );

			if( sc->getSCE(SC_DECREASEAGI) )
				val = max( val, 25 );
			if( sc->getSCE(SC_QUAGMIRE) || sc->getSCE(SC_HALLUCINATIONWALK_POSTDELAY) || (sc->getSCE(SC_GLOOMYDAY) && sc->getSCE(SC_GLOOMYDAY)->val4) )
				val = max( val, 50 );
			if( sc->getSCE(SC_DONTFORGETME) )
				val = max( val, sc->getSCE(SC_DONTFORGETME)->val3 );
			if( sc->getSCE(SC_CURSE) )
				val = max( val, 300 );
			if( sc->getSCE(SC_CHASEWALK) )
				val = max( val, sc->getSCE(SC_CHASEWALK)->val3 );
			if( sc->getSCE(SC_WEDDING) )
				val = max( val, 100 );
			if( sc->getSCE(SC_JOINTBEAT) && sc->getSCE(SC_JOINTBEAT)->val2&(BREAK_ANKLE|BREAK_KNEE) )
				val = max( val, (sc->getSCE(SC_JOINTBEAT)->val2&BREAK_ANKLE ? 50 : 0) + (sc->getSCE(SC_JOINTBEAT)->val2&BREAK_KNEE ? 30 : 0) );
			if( sc->getSCE(SC_CLOAKING) && (sc->getSCE(SC_CLOAKING)->val4&1) == 0 )
				val = max( val, sc->getSCE(SC_CLOAKING)->val1 < 3 ? 300 : 30 - 3 * sc->getSCE(SC_CLOAKING)->val1 );
			if( sc->getSCE(SC_GOSPEL) && sc->getSCE(SC_GOSPEL)->val4 == BCT_ENEMY )
				val = max( val, 75 );
			if( sc->getSCE(SC_SLOWDOWN) ) // Slow Potion
				val = max( val, sc->getSCE(SC_SLOWDOWN)->val1 );
			if( sc->getSCE(SC_GATLINGFEVER) )
				val = max( val, 100 );
			if( sc->getSCE(SC_SUITON) )
				val = max( val, sc->getSCE(SC_SUITON)->val3 );
			if( sc->getSCE(SC_SWOO) )
				val = max( val, 300 );
			if( sc->getSCE(SC_SKA) )
				val = max( val, 25 );
			if( sc->getSCE(SC_FREEZING) )
				val = max( val, 30 );
			if( sc->getSCE(SC_MARSHOFABYSS) )
				val = max( val, sc->getSCE(SC_MARSHOFABYSS)->val3 );
			if( sc->getSCE(SC_CAMOUFLAGE) && sc->getSCE(SC_CAMOUFLAGE)->val1 > 2 )
				val = max( val, 25 * (5 - sc->getSCE(SC_CAMOUFLAGE)->val1) );
			if( sc->getSCE(SC_STEALTHFIELD) )
				val = max( val, 20 );
			if( sc->getSCE(SC__LAZINESS) )
				val = max( val, 25 );
			if( sc->getSCE(SC_ROCK_CRUSHER_ATK) )
				val = max( val, sc->getSCE(SC_ROCK_CRUSHER_ATK)->val2 );
			if( sc->getSCE(SC_POWER_OF_GAIA) )
				val = max( val, sc->getSCE(SC_POWER_OF_GAIA)->val2 );
			if( sc->getSCE(SC_MELON_BOMB) )
				val = max( val, sc->getSCE(SC_MELON_BOMB)->val2 );
			if( sc->getSCE(SC_REBOUND) )
				val = max( val, 25 );
			if( sc->getSCE(SC_B_TRAP) )
				val = max( val, sc->getSCE(SC_B_TRAP)->val3 );
			if (sc->getSCE(SC_CATNIPPOWDER))
				val = max(val, sc->getSCE(SC_CATNIPPOWDER)->val3);
			if (sc->getSCE(SC_SP_SHA))
				val = max(val, sc->getSCE(SC_SP_SHA)->val2);
			if (sc->getSCE(SC_CREATINGSTAR))
				val = max(val, 90);
			if (sc->getSCE(SC_SHIELDCHAINRUSH))
				val = max(val, 20);
			if (sc->getSCE(SC_GROUNDGRAVITY))
				val = max(val, 20);

			if( sd && sd->bonus.speed_rate + sd->bonus.speed_add_rate > 0 ) // Permanent item-based speedup
				val = max( val, sd->bonus.speed_rate + sd->bonus.speed_add_rate );
		}
		speed_rate += val;
		val = 0;

		if( sc->getSCE(SC_MARSHOFABYSS) && speed_rate > 150 )
			speed_rate = 150;

		// GetMoveHasteValue1()
		if( sc->getSCE(SC_SPEEDUP1) )
			val = max( val, sc->getSCE(SC_SPEEDUP1)->val1 );
		if (sc->getSCE(SC_AGIUP))
			val = max(val, sc->getSCE(SC_AGIUP)->val1);
		if( sc->getSCE(SC_INCREASEAGI) )
			val = max( val, 25 );
		if( sc->getSCE(SC_WINDWALK) )
			val = max( val, 2 * sc->getSCE(SC_WINDWALK)->val1 );
		if( sc->getSCE(SC_CARTBOOST) )
			val = max( val, 20 );
		if( sd && (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN && pc_checkskill(sd,TF_MISS) > 0 )
			val = max( val, 1 * pc_checkskill(sd,TF_MISS) );
		if( sc->getSCE(SC_CLOAKING) && (sc->getSCE(SC_CLOAKING)->val4&1) == 1 )
			val = max( val, sc->getSCE(SC_CLOAKING)->val1 >= 10 ? 25 : 3 * sc->getSCE(SC_CLOAKING)->val1 - 3 );
		if( sc->getSCE(SC_BERSERK) )
			val = max( val, 25 );
		if( sc->getSCE(SC_RUN) )
			val = max( val, 55 );
		if( sc->getSCE(SC_AVOID) )
			val = max( val, sc->getSCE(SC_AVOID)->val2 );
		if (sc->getSCE(SC_INVINCIBLE))
			val = max(val, sc->getSCE(SC_INVINCIBLE)->val3);
		if( sc->getSCE(SC_CLOAKINGEXCEED) )
			val = max( val, sc->getSCE(SC_CLOAKINGEXCEED)->val3);
		if (sc->getSCE(SC_PARALYSE) && sc->getSCE(SC_PARALYSE)->val3 == 0)
			val = max(val, 50);
		if( sc->getSCE(SC_HOVERING) )
			val = max( val, 10 );
		if( sc->getSCE(SC_GN_CARTBOOST) )
			val = max( val, sc->getSCE(SC_GN_CARTBOOST)->val2 );
		if( sc->getSCE(SC_SWINGDANCE) )
			val = max( val, sc->getSCE(SC_SWINGDANCE)->val3 );
		if( sc->getSCE(SC_WIND_STEP_OPTION) )
			val = max( val, sc->getSCE(SC_WIND_STEP_OPTION)->val2 );
		if( sc->getSCE(SC_FULL_THROTTLE) )
			val = max( val, 25 );
		if (sc->getSCE(SC_ARCLOUSEDASH))
			val = max(val, sc->getSCE(SC_ARCLOUSEDASH)->val3);
		if( sc->getSCE(SC_DORAM_WALKSPEED) )
			val = max(val, sc->getSCE(SC_DORAM_WALKSPEED)->val1);
		if (sc->getSCE(SC_RUSHWINDMILL))
			val = max(val, 25); // !TODO: Confirm bonus movement speed
		if (sc->getSCE(SC_EMERGENCY_MOVE))
			val = max(val, sc->getSCE(SC_EMERGENCY_MOVE)->val2);
		if( sc->getSCE(SC_JAWAII_SERENADE) ){
			val = max( val, 25 );
		}

		// !FIXME: official items use a single bonus for this [ultramage]
		if( sc->getSCE(SC_SPEEDUP0) ) // Temporary item-based speedup
			val = max( val, sc->getSCE(SC_SPEEDUP0)->val1 );
		if( sd && sd->bonus.speed_rate + sd->bonus.speed_add_rate < 0 ) // Permanent item-based speedup
			val = max( val, -(sd->bonus.speed_rate + sd->bonus.speed_add_rate) );

		speed_rate -= val;

		if( speed_rate < 40 )
			speed_rate = 40;
	}

	// GetSpeed()
	if( sd && pc_iscarton(sd) )
		speed += speed * (50 - 5 * pc_checkskill(sd,MC_PUSHCART)) / 100;
	if( sc->getSCE(SC_PARALYSE) && sc->getSCE(SC_PARALYSE)->val3 == 1 )
		speed += speed * 50 / 100;
	if( speed_rate != 100 )
		speed = speed * speed_rate / 100;
	if( sc->getSCE(SC_STEELBODY) )
		speed = 200;
	if( sc->getSCE(SC_DEFENDER) )
		speed = max(speed, 200);
	if (sc->getSCE(SC_ARMOR))
		speed = max(speed, 200);
	if( sc->getSCE(SC_WALKSPEED) && sc->getSCE(SC_WALKSPEED)->val1 > 0 ) // ChangeSpeed
		speed = speed * 100 / sc->getSCE(SC_WALKSPEED)->val1;

	return (unsigned short)cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
}

#ifdef RENEWAL_ASPD
/**
 * Renewal attack speed modifiers based on status changes
 * This function only affects RENEWAL players and comes after base calculation
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param fixed: True - fixed value [malufett]
 *               False - percentage value
 * @return modified aspd
 */
static short status_calc_aspd(struct block_list *bl, status_change *sc, bool fixed)
{
	int bonus = 0;

	if (!sc || !sc->count)
		return 0;

	if (fixed) {
		enum sc_type sc_val;

		if (!sc->getSCE(SC_QUAGMIRE)) {
			// !TODO: How does Two-Hand Quicken, Adrenaline Rush, and Spear quick change? (+10%)
			if (bonus < 7 && (sc->getSCE(SC_TWOHANDQUICKEN) || sc->getSCE(SC_ONEHAND) || sc->getSCE(SC_MERC_QUICKEN) || sc->getSCE(SC_ADRENALINE) || sc->getSCE(SC_SPEARQUICKEN)))
				bonus = 7;
			else if (bonus < 6 && sc->getSCE(SC_ADRENALINE2))
				bonus = 6;
			else if (bonus < 5 && sc->getSCE(SC_FLEET))
				bonus = 5;
		}

		if (sc->getSCE(SC_ASSNCROS) && bonus < sc->getSCE(SC_ASSNCROS)->val2) {
#ifdef RENEWAL
			bonus += sc->getSCE(SC_ASSNCROS)->val2;
#else
			if (bl->type != BL_PC)
				bonus += sc->getSCE(SC_ASSNCROS)->val2;
			else {
				switch(((TBL_PC*)bl)->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						break;
					default:
						bonus += sc->getSCE(SC_ASSNCROS)->val2;
						break;
				}
			}
#endif
		}

		if (bonus < 20 && sc->getSCE(SC_MADNESSCANCEL))
			bonus = 20;
		else if (bonus < 15 && sc->getSCE(SC_BERSERK))
			bonus = 15;

		if (sc->getSCE(sc_val = SC_ASPDPOTION3) || sc->getSCE(sc_val = SC_ASPDPOTION2) || sc->getSCE(sc_val = SC_ASPDPOTION1) || sc->getSCE(sc_val = SC_ASPDPOTION0))
			bonus += sc->getSCE(sc_val)->val1;
		if (sc->getSCE(SC_ATTHASTE_CASH))
			bonus += sc->getSCE(SC_ATTHASTE_CASH)->val1;
	} else {
		if (sc->getSCE(SC_DONTFORGETME))
			bonus -= sc->getSCE(SC_DONTFORGETME)->val2 / 10;
#ifdef RENEWAL
		if (sc->getSCE(SC_ENSEMBLEFATIGUE))
			bonus -= sc->getSCE(SC_ENSEMBLEFATIGUE)->val2 / 10;
#else
		if (sc->getSCE(SC_LONGING))
			bonus -= sc->getSCE(SC_LONGING)->val2 / 10;
#endif
		if (sc->getSCE(SC_STEELBODY))
			bonus -= 25;
		if (sc->getSCE(SC_SKA))
			bonus -= 25;
		if (sc->getSCE(SC_DEFENDER))
			bonus -= sc->getSCE(SC_DEFENDER)->val4 / 10;
		if (sc->getSCE(SC_GOSPEL) && sc->getSCE(SC_GOSPEL)->val4 == BCT_ENEMY)
			bonus -= 75;
#ifndef RENEWAL
		if (sc->getSCE(SC_GRAVITATION))
			bonus -= sc->getSCE(SC_GRAVITATION)->val2 / 10; // Needs more info
#endif
		if (sc->getSCE(SC_JOINTBEAT)) { // Needs more info
			if (sc->getSCE(SC_JOINTBEAT)->val2&BREAK_WRIST)
				bonus -= 25;
			if (sc->getSCE(SC_JOINTBEAT)->val2&BREAK_KNEE)
				bonus -= 10;
		}
		if (sc->getSCE(SC_FREEZING))
			bonus -= 30;
		if (sc->getSCE(SC_HALLUCINATIONWALK_POSTDELAY))
			bonus -= 50;
		if (sc->getSCE(SC_PARALYSE) && sc->getSCE(SC_PARALYSE)->val3 == 1)
			bonus -= 10;
		if (sc->getSCE(SC__BODYPAINT))
			bonus -= 5 * sc->getSCE(SC__BODYPAINT)->val1;
		if (sc->getSCE(SC__INVISIBILITY))
			bonus -= sc->getSCE(SC__INVISIBILITY)->val2;
		if (sc->getSCE(SC__GROOMY))
			bonus -= sc->getSCE(SC__GROOMY)->val2;
		if (sc->getSCE(SC_SWINGDANCE))
			bonus += sc->getSCE(SC_SWINGDANCE)->val3;
		if (sc->getSCE(SC_DANCEWITHWUG))
			bonus += sc->getSCE(SC_DANCEWITHWUG)->val3;
		if (sc->getSCE(SC_GLOOMYDAY))
			bonus -= sc->getSCE(SC_GLOOMYDAY)->val3;
		if (sc->getSCE(SC_GT_CHANGE))
			bonus += sc->getSCE(SC_GT_CHANGE)->val3;
		if (sc->getSCE(SC_MELON_BOMB))
			bonus -= sc->getSCE(SC_MELON_BOMB)->val3;
		if (sc->getSCE(SC_BOOST500))
			bonus += sc->getSCE(SC_BOOST500)->val1;
		if (sc->getSCE(SC_EXTRACT_SALAMINE_JUICE))
			bonus += sc->getSCE(SC_EXTRACT_SALAMINE_JUICE)->val1;
		if (sc->getSCE(SC_GOLDENE_FERSE))
			bonus += sc->getSCE(SC_GOLDENE_FERSE)->val3;
		if (sc->getSCE(SC_INCASPDRATE))
			bonus += sc->getSCE(SC_INCASPDRATE)->val1;
		if (sc->getSCE(SC_GATLINGFEVER))
			bonus += sc->getSCE(SC_GATLINGFEVER)->val1;
		if (sc->getSCE(SC_STAR_COMFORT))
			bonus += 3 * sc->getSCE(SC_STAR_COMFORT)->val1;
		if (sc->getSCE(SC_WIND_INSIGNIA) && sc->getSCE(SC_WIND_INSIGNIA)->val1 == 2)
			bonus += 10;
		if (sc->getSCE(SC_INCREASEAGI))
			bonus += sc->getSCE(SC_INCREASEAGI)->val1;
		if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_ASPDRATE)
			bonus += 20;
		if (sc->getSCE(SC_STARSTANCE))
			bonus += sc->getSCE(SC_STARSTANCE)->val2;
		if( sc->getSCE(SC_2011RWC_SCROLL) )
			bonus += 5;
		if( sc->getSCE(SC_SPARKCANDY) )
			bonus += 25;
		if( sc->getSCE(SC_ACARAJE) )
			bonus += 10;
		if( sc->getSCE(SC_SKF_ASPD) )
			bonus += sc->getSCE(SC_SKF_ASPD)->val1;
		if( sc->getSCE(SC_PORK_RIB_STEW) )
			bonus += 5;

		map_session_data* sd = BL_CAST(BL_PC, bl);
		uint8 skill_lv;

		if (sd) {
			if ((skill_lv = pc_checkskill(sd, BA_MUSICALLESSON)) > 0)
				bonus += skill_lv;
			if ((skill_lv = pc_checkskill(sd, RG_PLAGIARISM)) > 0)
				bonus += skill_lv;
		}
	}

	return bonus;
}
#endif

/**
 * Modifies ASPD by a number, rather than a percentage (10 = 1 ASPD)
 * A subtraction reduces the delay, meaning an increase in ASPD
 * This comes after the percentage changes and is based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param aspd: Object's current ASPD
 * @return modified aspd
 */
static short status_calc_fix_aspd(struct block_list *bl, status_change *sc, int aspd)
{
	if (!sc || !sc->count)
		return cap_value(aspd, 0, 2000);
	if (sc->getSCE(SC_OVERED_BOOST))
		return cap_value(2000 - sc->getSCE(SC_OVERED_BOOST)->val3 * 10, 0, 2000);

	if ((sc->getSCE(SC_GUST_OPTION) || sc->getSCE(SC_BLAST_OPTION) || sc->getSCE(SC_WILD_STORM_OPTION)))
		aspd -= 50; // +5 ASPD
	if (sc->getSCE(SC_FIGHTINGSPIRIT))
		aspd -= sc->getSCE(SC_FIGHTINGSPIRIT)->val2;
	if (sc->getSCE(SC_MTF_ASPD))
		aspd -= sc->getSCE(SC_MTF_ASPD)->val1;
	if (sc->getSCE(SC_MTF_ASPD2))
		aspd -= sc->getSCE(SC_MTF_ASPD2)->val1;
	if (sc->getSCE(SC_SOULSHADOW))
		aspd -= 10 * sc->getSCE(SC_SOULSHADOW)->val2;
	if (sc->getSCE(SC_HEAT_BARREL))
		aspd -= sc->getSCE(SC_HEAT_BARREL)->val1 * 10;
	if (sc->getSCE(SC_EP16_2_BUFF_SS))
		aspd -= 100; // +10 ASPD
	if (sc->getSCE(SC_PACKING_ENVELOPE6))
		aspd -= sc->getSCE(SC_PACKING_ENVELOPE6)->val1 * 10;
	if (sc->getSCE(SC_SINCERE_FAITH))
		aspd -= 10 * sc->getSCE(SC_SINCERE_FAITH)->val2;
	if( sc->getSCE(SC_LIMIT_POWER_BOOSTER) )
		aspd -= 10;

	return cap_value(aspd, 0, 2000); // Will be recap for proper bl anyway
}

/**
 * Calculates an object's ASPD modifier based on status changes (alters amotion value)
 * Note: The scale of aspd_rate is 1000 = 100%
 * Note2: This only affects Homunculus, Mercenaries, and Pre-renewal Players
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param aspd_rate: Object's current ASPD
 * @return modified aspd_rate
 */
static short status_calc_aspd_rate(struct block_list *bl, status_change *sc, int aspd_rate)
{
	int i;

	if(!sc || !sc->count)
		return cap_value(aspd_rate,0,SHRT_MAX);

	int max = 0;
	if (sc->getSCE(SC_STAR_COMFORT))
		max = sc->getSCE(SC_STAR_COMFORT)->val2;

	if (sc->getSCE(SC_TWOHANDQUICKEN) &&
		max < sc->getSCE(SC_TWOHANDQUICKEN)->val2)
		max = sc->getSCE(SC_TWOHANDQUICKEN)->val2;

	if (sc->getSCE(SC_ONEHAND) &&
		max < sc->getSCE(SC_ONEHAND)->val2)
		max = sc->getSCE(SC_ONEHAND)->val2;

	if (sc->getSCE(SC_MERC_QUICKEN) &&
		max < sc->getSCE(SC_MERC_QUICKEN)->val2)
		max = sc->getSCE(SC_MERC_QUICKEN)->val2;

	if (sc->getSCE(SC_ADRENALINE2) &&
		max < sc->getSCE(SC_ADRENALINE2)->val3)
		max = sc->getSCE(SC_ADRENALINE2)->val3;

	if (sc->getSCE(SC_ADRENALINE) &&
		max < sc->getSCE(SC_ADRENALINE)->val3)
		max = sc->getSCE(SC_ADRENALINE)->val3;

	if (sc->getSCE(SC_SPEARQUICKEN) &&
		max < sc->getSCE(SC_SPEARQUICKEN)->val2)
		max = sc->getSCE(SC_SPEARQUICKEN)->val2;

	if (sc->getSCE(SC_GATLINGFEVER) &&
		max < sc->getSCE(SC_GATLINGFEVER)->val2)
		max = sc->getSCE(SC_GATLINGFEVER)->val2;

	if (sc->getSCE(SC_FLEET) &&
		max < sc->getSCE(SC_FLEET)->val2)
		max = sc->getSCE(SC_FLEET)->val2;

	if (sc->getSCE(SC_INVINCIBLE) &&
		max < sc->getSCE(SC_INVINCIBLE)->val4)
		max = sc->getSCE(SC_INVINCIBLE)->val4;

	if (sc->getSCE(SC_ASSNCROS) && max < sc->getSCE(SC_ASSNCROS)->val2) {
		if (bl->type != BL_PC)
			max = sc->getSCE(SC_ASSNCROS)->val2;
		else
			switch (((TBL_PC*)bl)->status.weapon) {
				case W_BOW:
				case W_REVOLVER:
				case W_RIFLE:
				case W_GATLING:
				case W_SHOTGUN:
				case W_GRENADE:
					break;
				default:
					max = sc->getSCE(SC_ASSNCROS)->val2;
			}
	}
	aspd_rate -= max;

	if (sc->getSCE(SC_BERSERK))
		aspd_rate -= 300;
	else if (sc->getSCE(SC_MADNESSCANCEL))
		aspd_rate -= 200;

	if( sc->getSCE(i=SC_ASPDPOTION3) ||
		sc->getSCE(i=SC_ASPDPOTION2) ||
		sc->getSCE(i=SC_ASPDPOTION1) ||
		sc->getSCE(i=SC_ASPDPOTION0) )
		aspd_rate -= sc->getSCE(i)->val2;

	if (sc->getSCE(SC_ATTHASTE_CASH))
		aspd_rate -= sc->getSCE(SC_ATTHASTE_CASH)->val2;

	if(sc->getSCE(SC_DONTFORGETME))
		aspd_rate += sc->getSCE(SC_DONTFORGETME)->val2;
#ifdef RENEWAL
	if (sc->getSCE(SC_ENSEMBLEFATIGUE))
		aspd_rate += sc->getSCE(SC_ENSEMBLEFATIGUE)->val2;
#else
	if(sc->getSCE(SC_LONGING))
		aspd_rate += sc->getSCE(SC_LONGING)->val2;
#endif
	if(sc->getSCE(SC_STEELBODY))
		aspd_rate += 250;
	if(sc->getSCE(SC_SKA))
		aspd_rate += 250;
	if(sc->getSCE(SC_DEFENDER))
		aspd_rate += sc->getSCE(SC_DEFENDER)->val4;
	if(sc->getSCE(SC_GOSPEL) && sc->getSCE(SC_GOSPEL)->val4 == BCT_ENEMY)
		aspd_rate += 250;
#ifndef RENEWAL
	if(sc->getSCE(SC_GRAVITATION))
		aspd_rate += sc->getSCE(SC_GRAVITATION)->val2;
#endif
	if(sc->getSCE(SC_JOINTBEAT)) {
		if( sc->getSCE(SC_JOINTBEAT)->val2&BREAK_WRIST )
			aspd_rate += 250;
		if( sc->getSCE(SC_JOINTBEAT)->val2&BREAK_KNEE )
			aspd_rate += 100;
	}
	if( sc->getSCE(SC_FREEZING) )
		aspd_rate += 300;
	if( sc->getSCE(SC_HALLUCINATIONWALK_POSTDELAY) )
		aspd_rate += 500;
	if( sc->getSCE(SC_PARALYSE) && sc->getSCE(SC_PARALYSE)->val3 == 1 )
		aspd_rate += 100;
	if( sc->getSCE(SC__BODYPAINT) )
		aspd_rate +=  50 * sc->getSCE(SC__BODYPAINT)->val1;
	if( sc->getSCE(SC__INVISIBILITY) )
		aspd_rate += sc->getSCE(SC__INVISIBILITY)->val2 * 10;
	if( sc->getSCE(SC__GROOMY) )
		aspd_rate += sc->getSCE(SC__GROOMY)->val2 * 10;
	if( sc->getSCE(SC_SWINGDANCE) )
		aspd_rate -= sc->getSCE(SC_SWINGDANCE)->val3 * 10;
	if( sc->getSCE(SC_DANCEWITHWUG) )
		aspd_rate -= sc->getSCE(SC_DANCEWITHWUG)->val3 * 10;
	if( sc->getSCE(SC_GLOOMYDAY) )
		aspd_rate += sc->getSCE(SC_GLOOMYDAY)->val3 * 10;
	if( sc->getSCE(SC_GT_CHANGE) )
		aspd_rate -= sc->getSCE(SC_GT_CHANGE)->val3 * 10;
	if( sc->getSCE(SC_MELON_BOMB) )
		aspd_rate += sc->getSCE(SC_MELON_BOMB)->val3 * 10;
	if( sc->getSCE(SC_BOOST500) )
		aspd_rate -= sc->getSCE(SC_BOOST500)->val1 *10;
	if( sc->getSCE(SC_EXTRACT_SALAMINE_JUICE) )
		aspd_rate -= sc->getSCE(SC_EXTRACT_SALAMINE_JUICE)->val1 * 10;
	if( sc->getSCE(SC_INCASPDRATE) )
		aspd_rate -= sc->getSCE(SC_INCASPDRATE)->val1 * 10;
	if( sc->getSCE(SC_GOLDENE_FERSE))
		aspd_rate -= sc->getSCE(SC_GOLDENE_FERSE)->val3 * 10;
	if (sc->getSCE(SC_WIND_INSIGNIA) && sc->getSCE(SC_WIND_INSIGNIA)->val1 == 2)
		aspd_rate -= 100;
	if (sc->getSCE(SC_STARSTANCE))
		aspd_rate -= 10 * sc->getSCE(SC_STARSTANCE)->val2;
	if( sc->getSCE(SC_2011RWC_SCROLL) )
		aspd_rate -= 50;
	if( sc->getSCE(SC_SPARKCANDY) )
		aspd_rate -= 250;
	if( sc->getSCE(SC_ACARAJE) )
		aspd_rate -= 100;
	if( sc->getSCE(SC_SKF_ASPD) )
		aspd_rate -= sc->getSCE(SC_SKF_ASPD)->val1 * 10;
	if( sc->getSCE(SC_PORK_RIB_STEW) )
		aspd_rate -= 50;

	return (short)cap_value(aspd_rate,0,SHRT_MAX);
}

/**
 * Modifies the damage delay time based on status changes
 * The lower your delay, the quicker you can act after taking damage
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param dmotion: Object's current damage delay
 * @return modified delay rate
 */
static unsigned short status_calc_dmotion(struct block_list *bl, status_change *sc, int dmotion)
{
	/// It has been confirmed on official servers that MvP mobs have no dmotion even without endure
	if( bl->type == BL_MOB && status_get_class_(bl) == CLASS_BOSS )
		return 0;

	if (bl->type == BL_PC) {
		if (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND))
			return (unsigned short)cap_value(dmotion, 0, USHRT_MAX);

		if (((TBL_PC *)bl)->special_state.no_walk_delay)
			return 0;
	}

	if (sc && sc->count > 0 && (sc->getSCE(SC_ENDURE) || sc->getSCE(SC_RUN) || sc->getSCE(SC_WUGDASH) || sc->getSCE(SC_SPARKCANDY)))
		return 0;

	return (unsigned short)cap_value(dmotion,0,USHRT_MAX);
}

/**
* Adds power atk modifications based on status changes
* @param bl: Object to change patk [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param patk: Initial patk
* @return modified patk with cap_value(patk,0,USHRT_MAX)
*/
static signed short status_calc_patk(struct block_list *bl, status_change *sc, int patk)
{
	if (!sc || !sc->count)
		return cap_value(patk, 0, SHRT_MAX);

	if (sc->getSCE(SC_POWERFUL_FAITH))
		patk += sc->getSCE(SC_POWERFUL_FAITH)->val3;
	if (sc->getSCE(SC_COMPETENTIA))
		patk += sc->getSCE(SC_COMPETENTIA)->val2;
	if (sc->getSCE(SC_ABYSS_SLAYER))
		patk += sc->getSCE(SC_ABYSS_SLAYER)->val2;
	if (sc->getSCE(SC_PRON_MARCH))
		patk += sc->getSCE(SC_PRON_MARCH)->val2;
	if (sc->getSCE(SC_TEMPERING))
		patk += sc->getSCE(SC_TEMPERING)->val2;
	if( sc->getSCE( SC_ATTACK_STANCE ) ){
		patk += sc->getSCE( SC_ATTACK_STANCE )->val3;
	}
	if (sc->getSCE(SC_HIDDEN_CARD))
		patk += sc->getSCE(SC_HIDDEN_CARD)->val2;

	return (short)cap_value(patk, 0, SHRT_MAX);
}

/**
* Adds spell matk modifications based on status changes
* @param bl: Object to change smatk [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param smatk: Initial smatk
* @return modified smatk with cap_value(smatk,0,USHRT_MAX)
*/
static signed short status_calc_smatk(struct block_list *bl, status_change *sc, int smatk)
{
	if (!sc || !sc->count)
		return cap_value(smatk, 0, SHRT_MAX);

	if (sc->getSCE(SC_COMPETENTIA))
		smatk += sc->getSCE(SC_COMPETENTIA)->val2;
	if (sc->getSCE(SC_ABYSS_SLAYER))
		smatk += sc->getSCE(SC_ABYSS_SLAYER)->val2;
	if (sc->getSCE(SC_JAWAII_SERENADE))
		smatk += sc->getSCE(SC_JAWAII_SERENADE)->val2;
	if (sc->getSCE(SC_SPELL_ENCHANTING))
		smatk += sc->getSCE(SC_SPELL_ENCHANTING)->val2;
	if( sc->getSCE( SC_ATTACK_STANCE ) ){
		smatk += sc->getSCE( SC_ATTACK_STANCE )->val3;
	}

	return (short)cap_value(smatk, 0, SHRT_MAX);
}

/**
* Adds resist modifications based on status changes
* @param bl: Object to change res [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param res: Initial res
* @return modified res with cap_value(res,0,USHRT_MAX)
*/
static signed short status_calc_res(struct block_list *bl, status_change *sc, int res)
{
	if (!sc || !sc->count)
		return cap_value(res, 0, SHRT_MAX);

	if (sc->getSCE(SC_FIRM_FAITH))
		res += sc->getSCE(SC_FIRM_FAITH)->val3;
	if (sc->getSCE(SC_D_MACHINE))
		res += sc->getSCE(SC_D_MACHINE)->val3;
	if (sc->getSCE(SC_MUSICAL_INTERLUDE))
		res += sc->getSCE(SC_MUSICAL_INTERLUDE)->val2;
	if (sc->getSCE(SC_GOLDENE_TONE))
		res += sc->getSCE(SC_GOLDENE_TONE)->val2;
	if (sc->getSCE(SC_SHADOW_STRIP) && bl->type != BL_PC)
		res -= res * sc->getSCE(SC_SHADOW_STRIP)->val2 / 100;
	if (sc->getSCE(SC_AIN_RHAPSODY))
		res -= sc->getSCE(SC_AIN_RHAPSODY)->val2;
	if (sc->getSCE(SC_TOXIN_OF_MANDARA))
		res -= sc->getSCE(SC_TOXIN_OF_MANDARA)->val2;

	return (short)cap_value(res, 0, SHRT_MAX);
}

/**
* Adds magic resist modifications based on status changes
* @param bl: Object to change mres [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param mres: Initial mres
* @return modified mres with cap_value(mres,0,USHRT_MAX)
*/
static signed short status_calc_mres(struct block_list *bl, status_change *sc, int mres)
{
	if (!sc || !sc->count)
		return cap_value(mres, 0, SHRT_MAX);

	if (sc->getSCE(SC_GOLDENE_TONE))
		mres += sc->getSCE(SC_GOLDENE_TONE)->val2;
	if (sc->getSCE(SC_SHADOW_STRIP) && bl->type != BL_PC)
		mres -= mres * sc->getSCE(SC_SHADOW_STRIP)->val2 / 100;
	if (sc->getSCE(SC_GEF_NOCTURN))
		mres -= sc->getSCE(SC_GEF_NOCTURN)->val2;

	return (short)cap_value(mres, 0, SHRT_MAX);
}

/**
* Adds heal plus modifications based on status changes
* @param bl: Object to change hplus [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param hplus: Initial hplus
* @return modified hplus with cap_value(hplus,0,USHRT_MAX)
*/
static signed short status_calc_hplus(struct block_list *bl, status_change *sc, int hplus)
{
	if (!sc || !sc->count)
		return cap_value(hplus, 0, SHRT_MAX);

	return (short)cap_value(hplus, 0, SHRT_MAX);
}

/**
* Adds critical damage rate modifications based on status changes
* @param bl: Object to change crate [PC|MOB|HOM|MER|ELEM]
* @param sc: Object's status change information
* @param crate: Initial crate
* @return modified crate with cap_value(crate,0,USHRT_MAX)
*/
static signed short status_calc_crate(struct block_list *bl, status_change *sc, int crate)
{
	if (!sc || !sc->count)
		return cap_value(crate, 0, SHRT_MAX);

	if (sc->getSCE(SC_PRE_ACIES))
		crate += sc->getSCE(SC_PRE_ACIES)->val2;

	return (short)cap_value(crate, 0, SHRT_MAX);
}

/**
 * Calculates a max HP based on status changes
 * Values can either be percentages or fixed, based on how equations are formulated
 * @param bl: Object's block_list data
 * @param maxhp: Object's current max HP
 * @return modified maxhp
 */
static unsigned int status_calc_maxhp(struct block_list *bl, uint64 maxhp)
{
	int rate = 100;

	maxhp += status_get_hpbonus(bl,STATUS_BONUS_FIX);

	if ((rate += status_get_hpbonus(bl,STATUS_BONUS_RATE)) != 100)
		maxhp = maxhp * rate / 100;

	return (unsigned int)cap_value(maxhp,1,UINT_MAX);
}

/**
 * Calculates a max SP based on status changes
 * Values can either be percentages or fixed, bas ed on how equations are formulated
 * @param bl: Object's block_list data
 * @param maxsp: Object's current max SP
 * @return modified maxsp
 */
static unsigned int status_calc_maxsp(struct block_list *bl, uint64 maxsp)
{
	int rate = 100;

	maxsp += status_get_spbonus(bl,STATUS_BONUS_FIX);
	
	if ((rate += status_get_spbonus(bl,STATUS_BONUS_RATE)) != 100)
		maxsp = maxsp * rate / 100;
	
	return (unsigned int)cap_value(maxsp,1,UINT_MAX);
}

/**
* Calculates a max AP based on status changes
* Values can either be percentages or fixed, bas ed on how equations are formulated
* @param bl: Object's block_list data
* @param maxap: Object's current max AP
* @return modified maxap
*/
static unsigned int status_calc_maxap(struct block_list *bl, uint64 maxap)
{
	int rate = 100;

	maxap += status_get_apbonus(bl, STATUS_BONUS_FIX);

	if ((rate += status_get_apbonus(bl, STATUS_BONUS_RATE)) != 100)
		maxap = maxap * rate / 100;

	return (unsigned int)cap_value(maxap, 0, UINT_MAX);
}

/**
 * Changes a player's element based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param element: Object's current element
 * @return new element
 */
static unsigned char status_calc_element(struct block_list *bl, status_change *sc, int element)
{
	if(!sc || !sc->count)
		return cap_value(element, 0, UCHAR_MAX);

	if(sc->getSCE(SC_FREEZE) || sc->getSCE(SC_CRYSTAL_ARMOR_OPTION))
		return ELE_WATER;
	if(sc->getSCE(SC_STONE) || sc->getSCE(SC_STRONG_PROTECTION_OPTION))
		return ELE_EARTH;
	if(sc->getSCE(SC_FLAMEARMOR_OPTION))
		return ELE_FIRE;
	if(sc->getSCE(SC_EYES_OF_STORM_OPTION))
		return ELE_WIND;
	if(sc->getSCE(SC_POISON_SHIELD_OPTION))
		return ELE_POISON;
	if(sc->getSCE(SC_BENEDICTIO))
		return ELE_HOLY;
	if(sc->getSCE(SC_CHANGEUNDEAD))
		return ELE_UNDEAD;
	if(sc->getSCE(SC_ELEMENTALCHANGE))
		return sc->getSCE(SC_ELEMENTALCHANGE)->val2;
	if(sc->getSCE(SC_SHAPESHIFT))
		return sc->getSCE(SC_SHAPESHIFT)->val2;

	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

/**
 * Changes a player's element level based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param lv: Object's current element level
 * @return new element level
 */
static unsigned char status_calc_element_lv(struct block_list *bl, status_change *sc, int lv)
{
	if(!sc || !sc->count)
		return cap_value(lv, 1, 4);

	if(sc->getSCE(SC_FREEZE))
		return 1;
	if(sc->getSCE(SC_STONE))
		return 1;
	if(sc->getSCE(SC_BENEDICTIO))
		return 1;
	if(sc->getSCE(SC_CHANGEUNDEAD))
		return 1;
	if(sc->getSCE(SC_ELEMENTALCHANGE))
		return sc->getSCE(SC_ELEMENTALCHANGE)->val1;
	if(sc->getSCE(SC_SHAPESHIFT))
		return 1;
	if(sc->getSCE(SC__INVISIBILITY))
		return 1;
	if (sc->getSCE(SC_FLAMEARMOR_OPTION) || sc->getSCE(SC_CRYSTAL_ARMOR_OPTION) || sc->getSCE(SC_EYES_OF_STORM_OPTION) || 
		sc->getSCE(SC_STRONG_PROTECTION_OPTION) || sc->getSCE(SC_POISON_SHIELD_OPTION))
		return 1;

	return (unsigned char)cap_value(lv,1,4);
}

/**
 * Changes a player's attack element based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param element: Object's current attack element
 * @return new attack element
 */
unsigned char status_calc_attack_element(struct block_list *bl, status_change *sc, int element)
{
	if(!sc || !sc->count)
		return cap_value(element, 0, UCHAR_MAX);
	if(sc->getSCE(SC_ENCHANTARMS))
		return sc->getSCE(SC_ENCHANTARMS)->val1;
	if(sc->getSCE(SC_WATERWEAPON)
		|| (sc->getSCE(SC_WATER_INSIGNIA) && sc->getSCE(SC_WATER_INSIGNIA)->val1 == 2) )
		return ELE_WATER;
	if(sc->getSCE(SC_EARTHWEAPON)
		|| (sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 2) )
		return ELE_EARTH;
	if(sc->getSCE(SC_FIREWEAPON)
		|| (sc->getSCE(SC_FIRE_INSIGNIA) && sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 2) )
		return ELE_FIRE;
	if(sc->getSCE(SC_WINDWEAPON)
		|| (sc->getSCE(SC_WIND_INSIGNIA) && sc->getSCE(SC_WIND_INSIGNIA)->val1 == 2) )
		return ELE_WIND;
	if(sc->getSCE(SC_ENCPOISON))
		return ELE_POISON;
	if(sc->getSCE(SC_ASPERSIO))
		return ELE_HOLY;
	if(sc->getSCE(SC_SHADOWWEAPON))
		return ELE_DARK;
	if(sc->getSCE(SC_GHOSTWEAPON) || sc->getSCE(SC__INVISIBILITY))
		return ELE_GHOST;
	if(sc->getSCE(SC_TIDAL_WEAPON_OPTION) || sc->getSCE(SC_TIDAL_WEAPON) )
		return ELE_WATER;
	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

/**
 * Changes the mode of an object
 * @param bl: Object whose mode to change [PC|MOB|PET|HOM|NPC]
 * @param sc: Object's status change data
 * @param mode: Original mode
 * @return mode with cap_value(mode, 0, INT_MAX)
 */
static int status_calc_mode(struct block_list *bl, status_change *sc, int mode)
{
	if(!sc || !sc->count)
		return cap_value(mode, MD_NONE,INT_MAX);
	if(sc->getSCE(SC_MODECHANGE)) {
		if (sc->getSCE(SC_MODECHANGE)->val2)
			mode = (mode&~MD_MASK)|sc->getSCE(SC_MODECHANGE)->val2; // Set mode
		if (sc->getSCE(SC_MODECHANGE)->val3)
			mode = mode|sc->getSCE(SC_MODECHANGE)->val3; // Add mode
		if (sc->getSCE(SC_MODECHANGE)->val4)
			mode = mode&~sc->getSCE(SC_MODECHANGE)->val4; // Del mode
	}
	return cap_value(mode, MD_NONE, INT_MAX);
}

/**
 * Changes the mode of a slave mob
 * @param md: Slave mob whose mode to change
 */
void status_calc_slave_mode(mob_data& md)
{
	switch (battle_config.slaves_inherit_mode) {
		case 1: //Always aggressive
			if (!status_has_mode(&md.status,MD_AGGRESSIVE))
				sc_start4(nullptr, &md.bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 0);
			break;
		case 2: //Always passive
			if (status_has_mode(&md.status,MD_AGGRESSIVE))
				sc_start4(nullptr, &md.bl, SC_MODECHANGE, 100, 1, 0, 0, MD_AGGRESSIVE, 0);
			break;
		case 4: // Overwrite with slave mode
			sc_start4(nullptr, &md.bl, SC_MODECHANGE, 100, 1, MD_CANMOVE|MD_NORANDOMWALK|MD_CANATTACK, 0, 0, 0);
			break;
		default: //Copy master
			if (block_list* mbl = map_id2bl(md.master_id); mbl != nullptr && status_has_mode(status_get_status_data(*mbl), MD_AGGRESSIVE))
				sc_start4(nullptr, &md.bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 0);
			else
				sc_start4(nullptr, &md.bl, SC_MODECHANGE, 100, 1, 0, 0, MD_AGGRESSIVE, 0);
			break;
	}
}

/**
 * Gets the name of the given bl
 * @param bl: Object whose name to get [PC|MOB|PET|HOM|NPC]
 * @return name or "Unknown" if any other bl->type than noted above
 */
const char* status_get_name(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:	return ((TBL_PC *)bl)->fakename[0] != '\0' ? ((TBL_PC*)bl)->fakename : ((TBL_PC*)bl)->status.name;
		case BL_MOB:	return ((TBL_MOB*)bl)->name;
		case BL_PET:	return ((TBL_PET*)bl)->pet.name;
		case BL_HOM:	return ((TBL_HOM*)bl)->homunculus.name;
		case BL_MER:	return ((TBL_MER *)bl)->db->name.c_str();	// They only have database names which are global, not specific to GID.
		case BL_NPC:	return ((TBL_NPC*)bl)->name;
		case BL_ELEM:	return ((TBL_ELEM *)bl)->db->name.c_str(); // They only have database names which are global, not specific to GID.
	}
	return "Unknown";
}

/**
 * Gets the class/sprite id of the given bl
 * @param bl: Object whose class to get [PC|MOB|PET|HOM|MER|NPC|ELEM]
 * @return class or 0 if any other bl->type than noted above
 */
int status_get_class(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type ) {
		case BL_PC:	return ((TBL_PC*)bl)->status.class_;
		case BL_MOB:	return ((TBL_MOB*)bl)->vd->class_; // Class used on all code should be the view class of the mob.
		case BL_PET:	return ((TBL_PET*)bl)->pet.class_;
		case BL_HOM:	return ((TBL_HOM*)bl)->homunculus.class_;
		case BL_MER:	return ((TBL_MER*)bl)->mercenary.class_;
		case BL_NPC:	return ((TBL_NPC*)bl)->class_;
		case BL_ELEM:	return ((TBL_ELEM*)bl)->elemental.class_;
	}
	return 0;
}

/**
 * Gets the base level of the given bl
 * @param bl: Object whose base level to get [PC|MOB|PET|HOM|MER|NPC|ELEM]
 * @return base level or 1 if any other bl->type than noted above
 */
int status_get_lv(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:	return ((TBL_PC*)bl)->status.base_level;
		case BL_MOB:	return ((TBL_MOB*)bl)->level;
		case BL_PET:	return ((TBL_PET*)bl)->pet.level;
		case BL_HOM:	return ((TBL_HOM*)bl)->homunculus.level;
		case BL_MER:	return ((TBL_MER*)bl)->db->lv;
		case BL_ELEM:	return ((TBL_ELEM*)bl)->db->lv;
		case BL_NPC:	return ((TBL_NPC*)bl)->level;
	}
	return 1;
}

/**
 * Gets the regeneration info of the given bl
 * @param bl: Object whose regen info to get [PC|HOM|MER|ELEM]
 * @return regen data or nullptr if any other bl->type than noted above
 */
struct regen_data *status_get_regen_data(struct block_list *bl)
{
	nullpo_retr(nullptr, bl);
	switch (bl->type) {
		case BL_PC:	return &((TBL_PC*)bl)->regen;
		case BL_HOM:	return &((TBL_HOM*)bl)->regen;
		case BL_MER:	return &((TBL_MER*)bl)->regen;
		case BL_ELEM:	return &((TBL_ELEM*)bl)->regen;
		default:
			return nullptr;
	}
}

/**
 * Gets the status data of the given bl
 * @param bl: Object whose status to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return status or "dummy_status" if any other bl->type than noted above
 */
status_data* status_get_status_data(block_list& bl){
	switch (bl.type) {
		case BL_PC:
			return &reinterpret_cast<map_session_data*>( &bl )->battle_status;
		case BL_MOB:
			return &reinterpret_cast<mob_data*>( &bl )->status;
		case BL_PET:
			return &reinterpret_cast<pet_data*>( &bl )->status;
		case BL_HOM:
			return &reinterpret_cast<homun_data*>( &bl )->battle_status;
		case BL_MER:
			return &reinterpret_cast<s_mercenary_data*>( &bl )->battle_status;
		case BL_ELEM:
			return &reinterpret_cast<s_elemental_data*>( &bl )->battle_status;
		case BL_NPC: {
				npc_data* nd = reinterpret_cast<npc_data*>( &bl );

				if( mobdb_checkid( nd->class_ ) == 0 ){
					return &nd->status;
				}else{
					return &dummy_status;
				}
			}
		default:
			return &dummy_status;
	}
}

/**
 * Gets the base status data of the given bl
 * @param bl: Object whose status to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return base_status or nullptr if any other bl->type than noted above
 */
struct status_data *status_get_base_status(struct block_list *bl)
{
	nullpo_retr(nullptr, bl);
	switch (bl->type) {
		case BL_PC:	return &((TBL_PC*)bl)->base_status;
		case BL_MOB:	return ((TBL_MOB*)bl)->base_status ? ((TBL_MOB*)bl)->base_status : &((TBL_MOB*)bl)->db->status;
		case BL_PET:	return &((TBL_PET*)bl)->db->status;
		case BL_HOM:	return &((TBL_HOM*)bl)->base_status;
		case BL_MER:	return &((TBL_MER*)bl)->base_status;
		case BL_ELEM:	return &((TBL_ELEM*)bl)->base_status;
		case BL_NPC:	return ((mobdb_checkid(((TBL_NPC*)bl)->class_) == 0) ? &((TBL_NPC*)bl)->status : nullptr);
		default:
			return nullptr;
	}
}

/**
 * Gets the defense of the given bl
 * @param bl: Object whose defense to get [PC|MOB|HOM|MER|ELEM]
 * @return defense with cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX)
 */
defType status_get_def(struct block_list *bl)
{
	struct unit_data *ud;
	status_data* status = status_get_status_data(*bl);
	// TODO: check if dummy_status? Can never be nullptr [Lemongrass]
	int def = status?status->def:0;
	ud = unit_bl2ud(bl);
	if (ud && ud->skilltimer != INVALID_TIMER)
		def -= def * skill_get_castdef(ud->skill_id)/100;

	return cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
}

/**
 * Gets the walking speed of the given bl
 * @param bl: Object whose speed to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return speed
 */
unsigned short status_get_speed(struct block_list *bl)
{
	// TODO: is the statement of Skotlex still true? And would it not be better to check for dummy_status instead? [Lemongrass]
	if(bl->type==BL_NPC)// Only BL with speed data but no status_data [Skotlex]
		return ((struct npc_data *)bl)->speed;
	return status_get_status_data(*bl)->speed;
}

/**
 * Gets the party ID of the given bl
 * @param bl: Object whose party ID to get [PC|MOB|PET|HOM|MER|SKILL|ELEM]
 * @return party ID
 */
int status_get_party_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:
			return ((TBL_PC*)bl)->status.party_id;
		case BL_PET:
			if (((TBL_PET*)bl)->master)
				return ((TBL_PET*)bl)->master->status.party_id;
			break;
		case BL_MOB: {
				struct mob_data *md=(TBL_MOB*)bl;
				if( md->master_id > 0 ) {
					map_session_data *msd;
					if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != nullptr)
						return msd->status.party_id;
					return -md->master_id;
				}
			}
			break;
		case BL_HOM:
			if (((TBL_HOM*)bl)->master)
				return ((TBL_HOM*)bl)->master->status.party_id;
			break;
		case BL_MER:
			if (((TBL_MER*)bl)->master)
				return ((TBL_MER*)bl)->master->status.party_id;
			break;
		case BL_SKILL:
			if (((TBL_SKILL*)bl)->group)
				return ((TBL_SKILL*)bl)->group->party_id;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->status.party_id;
			break;
	}
	return 0;
}

/**
 * Gets the guild ID of the given bl
 * @param bl: Object whose guild ID to get [PC|MOB|PET|HOM|MER|SKILL|ELEM|NPC]
 * @return guild ID
 */
int status_get_guild_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:
			return ((TBL_PC*)bl)->status.guild_id;
		case BL_PET:
			if (((TBL_PET*)bl)->master)
				return ((TBL_PET*)bl)->master->status.guild_id;
			break;
		case BL_MOB:
			{
				map_session_data *msd;
				struct mob_data *md = (struct mob_data *)bl;
				if (md->guardian_data)	// Guardian's guild [Skotlex]
					return md->guardian_data->guild_id;
				if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != nullptr)
					return msd->status.guild_id; // Alchemist's mobs [Skotlex]
			}
			break;
		case BL_HOM:
			if (((TBL_HOM*)bl)->master)
				return ((TBL_HOM*)bl)->master->status.guild_id;
			break;
		case BL_MER:
			if (((TBL_MER*)bl)->master)
				return ((TBL_MER*)bl)->master->status.guild_id;
			break;
		case BL_NPC:
			if (((TBL_NPC*)bl)->subtype == NPCTYPE_SCRIPT)
				return ((TBL_NPC*)bl)->u.scr.guild_id;
			break;
		case BL_SKILL:
			if (((TBL_SKILL*)bl)->group)
				return ((TBL_SKILL*)bl)->group->guild_id;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->status.guild_id;
			break;
	}
	return 0;
}

/**
 * Gets the guild emblem ID of the given bl
 * @param bl: Object whose emblem ID to get [PC|MOB|PET|HOM|MER|SKILL|ELEM|NPC]
 * @return guild emblem ID
 */
int status_get_emblem_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:
			return ((TBL_PC*)bl)->guild_emblem_id;
		case BL_PET:
			if (((TBL_PET*)bl)->master)
				return ((TBL_PET*)bl)->master->guild_emblem_id;
			break;
		case BL_MOB:
			{
				map_session_data *msd;
				struct mob_data *md = (struct mob_data *)bl;
				if (md->guardian_data)	// Guardian's guild [Skotlex]
					return md->guardian_data->emblem_id;
				if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != nullptr)
					return msd->guild_emblem_id; // Alchemist's mobs [Skotlex]
			}
			break;
		case BL_HOM:
			if (((TBL_HOM*)bl)->master)
				return ((TBL_HOM*)bl)->master->guild_emblem_id;
			break;
		case BL_MER:
			if (((TBL_MER*)bl)->master)
				return ((TBL_MER*)bl)->master->guild_emblem_id;
			break;
		case BL_NPC:
			if (((TBL_NPC*)bl)->subtype == NPCTYPE_SCRIPT && ((TBL_NPC*)bl)->u.scr.guild_id > 0) {
				auto g = guild_search(((TBL_NPC*)bl)->u.scr.guild_id);
				if (g)
					return g->guild.emblem_id;
			}
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->guild_emblem_id;
			break;
	}
	return 0;
}

/**
 * Gets the race2 of a mob or pet
 * @param bl: Object whose race2 to get [MOB|PET]
 * @return race2
 */
std::vector<e_race2> status_get_race2(struct block_list *bl)
{
	nullpo_retr(std::vector<e_race2>(),bl);

	if (bl->type == BL_MOB)
		return ((struct mob_data *)bl)->db->race2;
	if (bl->type == BL_PET)
		return ((struct pet_data *)bl)->db->race2;
	return std::vector<e_race2>();
}

/**
 * Checks if an object is dead 
 * @param bl: Object to check [PC|MOB|HOM|MER|ELEM]
 * @return 1: Is dead or 0: Is alive
 */
bool status_isdead(block_list &bl){
	return status_get_status_data(bl)->hp == 0;
}

/**
 * Checks if an object is immune to magic 
 * @param bl: Object to check [PC|MOB|HOM|MER|ELEM]
 * @return value of magic damage to be blocked
 */
int status_isimmune(struct block_list *bl)
{
	status_change *sc =status_get_sc(bl);

	if (sc) {
		if (sc->getSCE(SC_HERMODE))
			return 100;

		if (sc->getSCE(SC_DEADLY_DEFEASANCE))
			return 0;
	}

	if (bl->type == BL_PC &&
		((TBL_PC*)bl)->special_state.no_magic_damage >= battle_config.gtb_sc_immunity)
		return ((TBL_PC*)bl)->special_state.no_magic_damage;
	return 0;
}

/**
 * Get view data of an object 
 * @param bl: Object whose view data to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return view data structure bl->vd
 */
struct view_data* status_get_viewdata(struct block_list *bl)
{
	nullpo_retr(nullptr, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->vd;
		case BL_MOB: return ((TBL_MOB*)bl)->vd;
		case BL_PET: return &((TBL_PET*)bl)->vd;
		case BL_NPC: return &((TBL_NPC*)bl)->vd;
		case BL_HOM: return ((TBL_HOM*)bl)->vd;
		case BL_MER: return ((TBL_MER*)bl)->vd;
		case BL_ELEM: return ((TBL_ELEM*)bl)->vd;
	}
	return nullptr;
}

/**
 * Set view data of an object
 * This function deals with class, mount, and item views
 * SC views are set in clif_getareachar_unit() 
 * @param bl: Object whose view data to set [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @param class_: class of the object
 */
void status_set_viewdata(struct block_list *bl, int class_)
{
	struct view_data* vd;
	nullpo_retv(bl);
	if (mobdb_checkid(class_) || mob_is_clone(class_))
		vd = mob_get_viewdata(class_);
	else if (npcdb_checkid(class_))
		vd = npc_get_viewdata(class_);
	else if (homdb_checkid(class_))
		vd = hom_get_viewdata(class_);
	else if (mercenary_db.exists(class_))
		vd = mercenary_get_viewdata(class_);
	else if (elemental_db.exists(class_))
		vd = elemental_get_viewdata(class_);
	else
		vd = nullptr;

	switch (bl->type) {
	case BL_PC:
		{
			TBL_PC* sd = (TBL_PC*)bl;
			if (pcdb_checkid(class_)) {
				if (sd->sc.option&OPTION_RIDING) {
					switch (class_) { // Adapt class to a Mounted one.
						case JOB_KNIGHT:
							class_ = JOB_KNIGHT2;
							break;
						case JOB_CRUSADER:
							class_ = JOB_CRUSADER2;
							break;
						case JOB_LORD_KNIGHT:
							class_ = JOB_LORD_KNIGHT2;
							break;
						case JOB_PALADIN:
							class_ = JOB_PALADIN2;
							break;
						case JOB_BABY_KNIGHT:
							class_ = JOB_BABY_KNIGHT2;
							break;
						case JOB_BABY_CRUSADER:
							class_ = JOB_BABY_CRUSADER2;
							break;
					}
				}
				sd->vd.class_ = class_;
				clif_get_weapon_view(sd, &sd->vd.weapon, &sd->vd.shield);
				sd->vd.head_top = sd->status.head_top;
				sd->vd.head_mid = sd->status.head_mid;
				sd->vd.head_bottom = sd->status.head_bottom;
				sd->vd.hair_style = cap_value(sd->status.hair, MIN_HAIR_STYLE, MAX_HAIR_STYLE);
				sd->vd.hair_color = cap_value(sd->status.hair_color, MIN_HAIR_COLOR, MAX_HAIR_COLOR);
				sd->vd.cloth_color = cap_value(sd->status.clothes_color, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
				sd->vd.body_style = cap_value(sd->status.body, MIN_BODY_STYLE, MAX_BODY_STYLE);
				sd->vd.sex = sd->status.sex;

				if (sd->vd.cloth_color) {
					if(sd->sc.option&OPTION_WEDDING && battle_config.wedding_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&OPTION_XMAS && battle_config.xmas_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&(OPTION_SUMMER|OPTION_SUMMER2) && battle_config.summer_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&OPTION_HANBOK && battle_config.hanbok_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&OPTION_OKTOBERFEST && battle_config.oktoberfest_ignorepalette)
						sd->vd.cloth_color = 0;
				}
				if ( sd->vd.body_style && sd->sc.option&OPTION_COSTUME)
 					sd->vd.body_style = 0;
			} else if (vd)
				memcpy(&sd->vd, vd, sizeof(struct view_data));
			else
				ShowError("status_set_viewdata (PC): No view data for class %d\n", class_);
		}
	break;
	case BL_MOB:
		{
			TBL_MOB* md = (TBL_MOB*)bl;
			if (vd){
				mob_free_dynamic_viewdata( md );

				md->vd = vd;
			}else if( pcdb_checkid( class_ ) ){
				mob_set_dynamic_viewdata( md );

				md->vd->class_ = class_;
				md->vd->hair_style = cap_value(md->vd->hair_style, MIN_HAIR_STYLE, MAX_HAIR_STYLE);
				md->vd->hair_color = cap_value(md->vd->hair_color, MIN_HAIR_COLOR, MAX_HAIR_COLOR);
			}else
				ShowError("status_set_viewdata (MOB): No view data for class %d\n", class_);
		}
	break;
	case BL_PET:
		{
			TBL_PET* pd = (TBL_PET*)bl;
			if (vd) {
				memcpy(&pd->vd, vd, sizeof(struct view_data));
				if (!pcdb_checkid(vd->class_)) {
					pd->vd.hair_style = battle_config.pet_hair_style;
					if(pd->pet.equip) {
						pd->vd.head_bottom = itemdb_viewid(pd->pet.equip);
						if (!pd->vd.head_bottom)
							pd->vd.head_bottom = pd->pet.equip;
					}
				}
			} else
				ShowError("status_set_viewdata (PET): No view data for class %d\n", class_);
		}
	break;
	case BL_NPC:
		{
			TBL_NPC* nd = (TBL_NPC*)bl;
			if (vd)
				memcpy(&nd->vd, vd, sizeof(struct view_data));
			else if (pcdb_checkid(class_)) {
				memset(&nd->vd, 0, sizeof(struct view_data));
				nd->vd.class_ = class_;
				nd->vd.hair_style = cap_value(nd->vd.hair_style, MIN_HAIR_STYLE, MAX_HAIR_STYLE);
			} else {
				ShowError("status_set_viewdata (NPC): Invalid view data %d\n", class_);
				if (bl->m >= 0)
					ShowDebug("Source (NPC): %s at %s (%d,%d)\n", nd->name, map_mapid2mapname(bl->m), bl->x, bl->y);
				else
					ShowDebug("Source (NPC): %s (invisible/not on a map)\n", nd->name);
				ShowDebug( "Source (NPC): %s is located in: %s\n", nd->name, nd->path );
			}
			break;
		}
	break;
	case BL_HOM:
		{
			struct homun_data *hd = (struct homun_data*)bl;
			if (vd)
				hd->vd = vd;
			else
				ShowError("status_set_viewdata (HOMUNCULUS): No view data for class %d\n", class_);
		}
		break;
	case BL_MER:
		{
			s_mercenary_data *md = (s_mercenary_data*)bl;
			if (vd)
				md->vd = vd;
			else
				ShowError("status_set_viewdata (MERCENARY): No view data for class %d\n", class_);
		}
		break;
	case BL_ELEM:
		{
			s_elemental_data *ed = (s_elemental_data*)bl;
			if (vd)
				ed->vd = vd;
			else
				ShowError("status_set_viewdata (ELEMENTAL): No view data for class %d\n", class_);
		}
		break;
	}
}

/**
 * Get status change data of an object
 * @param bl: Object whose sc data to get [PC|MOB|HOM|MER|ELEM|NPC]
 * @return status change data structure bl->sc
 */
status_change *status_get_sc(struct block_list *bl)
{
	if( bl )
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->sc;
		case BL_MOB: return &((TBL_MOB*)bl)->sc;
		case BL_NPC: return &((TBL_NPC*)bl)->sc;
		case BL_HOM: return &((TBL_HOM*)bl)->sc;
		case BL_MER: return &((TBL_MER*)bl)->sc;
		case BL_ELEM: return &((TBL_ELEM*)bl)->sc;
	}
	return nullptr;
}

/**
 * Initiate (memset) the status change data of an object
 * @param bl: Object whose sc data to memset [PC|MOB|HOM|MER|ELEM|NPC]
 */
void status_change_init(struct block_list *bl)
{
	status_change *sc = status_get_sc(bl);
	nullpo_retv(sc);
	new (sc) status_change();
}

/*========================================== [Playtester]
* Returns the interval for status changes that iterate multiple times
* through the timer (e.g. those that deal damage in regular intervals)
* @param type: Status change (SC_*)
*------------------------------------------*/
static int status_get_sc_interval(enum sc_type type)
{
	switch (type) {
		case SC_POISON:
		case SC_LEECHESEND:
		case SC_DPOISON:
		case SC_DEATHHURT:
		case SC_GRADUAL_GRAVITY:
		case SC_KILLING_AURA:
		case SC_BOSSMAPINFO:
			return 1000;
		case SC_BURNING:
		case SC_PYREXIA:
			return 3000;
		case SC_MAGICMUSHROOM:
			return 4000;
		case SC_STONE:
			return 5000;
		case SC_BLEEDING:
		case SC_TOXIN:
			return 10000;
		case SC_HELLS_PLANT:
			return 333;
		case SC_SHIELDSPELL_HP:
			return 3000;
		case SC_SHIELDSPELL_SP:
			return 5000;
		default:
			break;
	}
	return 0;
}

/**
 * Applies SC defense to a given status change
 * This function also determines whether or not the status change will be applied
 * @param src: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param bl: Target of the status change
 * @param type: Status change (SC_*)
 * @param rate: Initial percentage rate of affecting bl (0~10000)
 * @param tick: Initial duration that the status change affects bl
 * @param flag: Value which determines what parts to calculate. See e_status_change_start_flags
 * @return adjusted duration based on flag values
 */
t_tick status_get_sc_def(struct block_list *src, struct block_list *bl, enum sc_type type, int rate, t_tick tick, unsigned char flag)
{
	/// Resistance rate: 10000 = 100%
	/// Example:	50% (5000) -> sc_def = 5000 -> 25%;
	///				5000ms -> tick_def = 5000 -> 2500ms
	int sc_def = 0, tick_def = -1; // -1 = use sc_def
	/// Fixed resistance value (after rate calculation)
	/// Example:	25% (2500) -> sc_def2 = 2000 -> 5%;
	///				2500ms -> tick_def2=2000 -> 500ms
	int sc_def2 = 0, tick_def2 = 0;
	status_change *sc;
	map_session_data *sd;

	nullpo_ret(bl);
	if (src == nullptr)
		return tick?tick:1; // This should not happen in current implementation, but leave it anyway

	// Skills (magic type) that are blocked by Golden Thief Bug card or Wand of Hermod
	if (status_isimmune(bl)) {
		std::shared_ptr<s_skill_db> skill = skill_db.find(battle_getcurrentskill(src));

		if (skill == nullptr) // Check for ground-type skills using the status when a player moves through units
			skill = skill_db.find(status_db.getSkill(type));

		if (skill != nullptr && skill->skill_type == BF_MAGIC && // Basic magic skill
			!skill->inf2[INF2_IGNOREGTB] && // Specific skill to bypass
			((skill->inf == INF_ATTACK_SKILL || skill->inf == INF_GROUND_SKILL || skill->inf == INF_SUPPORT_SKILL) || // Target skills should get blocked even when cast on self
			 (skill->inf == INF_SELF_SKILL && src != bl))) // Self skills should get blocked on all targets except self
			return 0;
	}

	sd = BL_CAST(BL_PC,bl);
	status_data* status = status_get_status_data(*bl);
	status_data* status_src = status_get_status_data(*src);
	sc = status_get_sc(bl);
	if( sc && !sc->count )
		sc = nullptr;

#ifdef RENEWAL
	uint16 levelAdv = (static_cast<uint16>(pow(max(0, status_get_lv(src) - status_get_lv(bl)), 2)) / 5) * 100;
#endif

	switch (type) {
		case SC_POISON:
		case SC_DPOISON:
#ifndef RENEWAL
			sc_def = status->vit*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			if (sd) {
				// For players: 60000 - 450*vit - 100*luk
				tick_def = status->vit*75;
				tick_def2 = status->luk*100;
			} else {
				// For monsters: 30000 - 200*vit
				tick /= 2;
				tick_def = (status->vit*200)/3;
			}
#else
			sc_def = status->vit * 100 - levelAdv;
			tick_def2 = -2000;
#endif
			break;
		case SC_STUN:
#ifndef RENEWAL
			sc_def = status->vit*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status->luk*10;
#else
			sc_def = status->vit * 100 - levelAdv;
			tick_def2 = -500;
#endif
			break;
		case SC_SILENCE:
#ifndef RENEWAL
			sc_def = status->vit*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status->luk*10;
#else
			sc_def = status->int_ * 100 - levelAdv;
			tick_def2 = -2000;
#endif
			break;
		case SC_BLEEDING:
#ifndef RENEWAL
			sc_def = status->vit*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status->luk*10;
#else
			sc_def = status->agi * 100 - levelAdv;
			tick_def2 = -12000;
#endif
			break;
		case SC_SLEEP:
#ifndef RENEWAL
			sc_def = status->int_*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status->luk*10;
#else
			sc_def = status->agi * 100 - levelAdv;
			tick_def2 = -2000;
#endif
			break;
		case SC_STONEWAIT:
#ifndef RENEWAL
			sc_def = status->mdef*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def = 0; // No duration reduction
#else
			sc_def = status->mdef * 100 - levelAdv;
			tick_def2 = -3000;
#endif
			break;
		case SC_FREEZE:
#ifndef RENEWAL
			sc_def = status->mdef*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status_src->luk*-10; // Caster can increase final duration with luk
#else
			sc_def = status->mdef * 100 - levelAdv;
			tick_def2 = -3000;
#endif
			break;
		case SC_CURSE:
			// Special property: immunity when luk is zero
			if (status->luk == 0)
				return 0;
#ifndef RENEWAL
			sc_def = status->luk*100;
			sc_def2 = status->luk*10 - status_get_lv(src)*10; // Curse only has a level penalty and no resistance
			tick_def = status->vit*100;
			tick_def2 = status->luk*10;
#else
			sc_def = status->luk * 100 - levelAdv;
			tick_def2 = -2000;
#endif
			break;
		case SC_BLIND:
#ifndef RENEWAL
			sc_def = (status->vit + status->int_)*50;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status->luk*10;
#else
			sc_def = status->int_ * 100 - levelAdv;
			tick_def2 = -2000;
#endif
			break;
		case SC_CONFUSION:
#ifndef RENEWAL
			sc_def = (status->str + status->int_)*50;
			sc_def2 = status_get_lv(src)*10 - status_get_lv(bl)*10 - status->luk*10; // Reversed sc_def2
			tick_def2 = status->luk*10;
#else
			sc_def = status->luk * 100 - levelAdv;
			tick_def2 = -2000;
#endif
			break;
		case SC_DECREASEAGI:
			if (sd)
				tick /= 2; // Half duration for players.
			sc_def2 = status->mdef*100;
			break;
		case SC_JOINTBEAT:
			tick_def2 = 1000 * ((status->luk / 2 + status->agi / 5) / 2); // (50 * LUK / 100 + 20 * AGI / 100) / 2
			break;
		case SC_DEEPSLEEP:
			tick_def2 = status_get_base_status(bl)->int_ * 25 + status_get_lv(bl) * 50;
			break;
		case SC_NETHERWORLD:
			// Resistance: {(Target's Base Level / 50) + (Target's Job Level / 10)} seconds
			tick_def2 = status_get_lv(bl) * 20 + (sd ? sd->status.job_level : 1) * 100;
			break;
		case SC_MARSHOFABYSS:
			// 5 second (Fixed) + 25 second - {( INT + LUK ) / 20 second }
			tick_def2 = (status->int_ + status->luk)*50;
			break;
		case SC_STASIS:
			// 10 second (fixed) + { Stasis Skill level * 10 - (Target's VIT + DEX) / 20 }
			tick_def2 = (status->vit + status->dex) * 50;
			break;
		case SC_WHITEIMPRISON:
			if( src == bl ) // 100% on caster
				break;
			sc_def = status->str * 20 + status_get_lv(bl) * 20 + status->luk * 10;
			tick_def2 = -2000;
			break;
		case SC_FEAR:
			sc_def = status->int_ * 20 + status_get_lv(bl) * 20 + status->luk * 10;
			tick_def2 = -4000; // 2 seconds is applied twice on Aegis
			break;
		case SC_BURNING:
			sc_def = status->agi * 20 + status_get_lv(bl) * 20 + status->luk * 10;
			tick_def2 = -2000;
			break;
		case SC_FREEZING:
			tick_def2 = (status->vit + status->dex) * 50;
			break;
		case SC_OBLIVIONCURSE: // 100% - (100 - 0.8 x INT)
			sc_def = status->int_ * 80;
			sc_def = max(sc_def, 500); // minimum of 5% resist
			tick_def = 0;
			tick_def2 = (status->vit + status->luk) * 500;
			break;
		case SC_TOXIN:
		case SC_PARALYSE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_LEECHESEND:
			tick_def2 = (status->vit + status->luk) * 500;
			break;
		case SC_BITE: // {(Base Success chance) - (Target's AGI / 4)}
			sc_def2 = status->agi*25;
			break;
		case SC_ELECTRICSHOCKER:
			tick_def2 = (status->vit + status->agi) * 70;
			break;
		case SC_CRYSTALIZE:
			tick_def2 = status_get_base_status(bl)->vit * 100;
			break;
		case SC_VACUUM_EXTREME:
			tick_def2 = (sd ? sd->status.str : status_get_base_status(bl)->str) * 50;
			break;
		case SC_KYOUGAKU:
			tick_def2 = 30*status->int_;
			break;
		case SC_PARALYSIS:
			tick_def2 = (status->vit + status->luk)*50;
			break;
		case SC_VOICEOFSIREN:
			// Resistance: {(Target's Base Level / 10) + (Target's Job Level / 5)} seconds
			tick_def2 = status_get_lv(bl) * 100 + (sd ? sd->status.job_level : 1) * 200;
			break;
		case SC_B_TRAP:
			tick_def = (sd ? sd->status.str : status_get_base_status(bl)->str) * 50; //! TODO: Figure out reduction formula
			break;
		case SC_NORECOVER_STATE:
			tick_def2 = status->luk * 100;
			break;
		default:
			// Effect that cannot be reduced? Likely a buff.
			if (!(rnd()%10000 < rate))
				return 0;
			return tick ? tick : 1;
	}

	if (sd) {
		if (battle_config.pc_sc_def_rate != 100) {
			sc_def = sc_def*battle_config.pc_sc_def_rate/100;
			sc_def2 = sc_def2*battle_config.pc_sc_def_rate/100;
		}
#ifndef RENEWAL
		sc_def = min(sc_def, battle_config.pc_max_sc_def*100);
		sc_def2 = min(sc_def2, battle_config.pc_max_sc_def*100);
#else
		sc_def = cap_value(sc_def, 0, battle_config.pc_max_sc_def*100);
		sc_def2 = cap_value(sc_def2, 0, battle_config.pc_max_sc_def*100);
#endif
		if (battle_config.pc_sc_def_rate != 100) {
			tick_def = tick_def*battle_config.pc_sc_def_rate/100;
			tick_def2 = tick_def2*battle_config.pc_sc_def_rate/100;
		}
	} else {
		if (battle_config.mob_sc_def_rate != 100) {
			sc_def = sc_def*battle_config.mob_sc_def_rate/100;
			sc_def2 = sc_def2*battle_config.mob_sc_def_rate/100;
		}
#ifndef RENEWAL
		sc_def = min(sc_def, battle_config.mob_max_sc_def*100);
		sc_def2 = min(sc_def2, battle_config.mob_max_sc_def*100);
#else
		sc_def = cap_value(sc_def, 0, battle_config.mob_max_sc_def*100);
		sc_def2 = cap_value(sc_def2, 0, battle_config.mob_max_sc_def*100);
#endif
		if (battle_config.mob_sc_def_rate != 100) {
			tick_def = tick_def*battle_config.mob_sc_def_rate/100;
			tick_def2 = tick_def2*battle_config.mob_sc_def_rate/100;
		}
	}

	if (sc) {
		if (sc->getSCE(SC_SCRESIST))
			sc_def += sc->getSCE(SC_SCRESIST)->val1*100; // Status resist
#ifdef RENEWAL
		else if (sc->getSCE(SC_SIEGFRIED) && (type == SC_BLIND || type == SC_STONE || type == SC_FREEZE || type == SC_STUN || type == SC_CURSE || type == SC_SLEEP || type == SC_SILENCE))
			sc_def += sc->getSCE(SC_SIEGFRIED)->val3 * 100; // Status resistance.
#else
		else if (sc->getSCE(SC_SIEGFRIED))
			sc_def += sc->getSCE(SC_SIEGFRIED)->val3*100; // Status resistance.
#endif
		else if (sc->getSCE(SC_LEECHESEND) && sc->getSCE(SC_LEECHESEND)->val3 == 0) {
			switch (type) {
				case SC_BLIND:
				case SC_STUN:
					return 0; // Immune
			}
		} else if (sc->getSCE(SC_OBLIVIONCURSE) && sc->getSCE(SC_OBLIVIONCURSE)->val3 == 0) {
			switch (type) {
				case SC_SILENCE:
				case SC_CURSE:
					return 0; // Immune
			}
		}
	}

	// When tick def not set, reduction is the same for both.
	if(tick_def == -1)
		tick_def = sc_def;

	// Natural resistance
	if (!(flag&SCSTART_NORATEDEF)) {
		rate -= rate*sc_def/10000;
		rate -= sc_def2;

		// Item resistance (only applies to rate%)
		if (sd) {
			for (const auto &it : sd->reseff) {
				if (it.id == type)
					rate -= rate * it.val / 10000;
			}
			if (sd->sc.getSCE(SC_COMMONSC_RESIST) && SC_COMMON_MIN <= type && type <= SC_COMMON_MAX)
				rate -= rate*sd->sc.getSCE(SC_COMMONSC_RESIST)->val1/100;
		}

		// Aegis accuracy
		if(rate > 0 && rate%10 != 0) rate += (10 - rate%10);
	}

	std::shared_ptr<s_status_change_db> scdb = status_db.find(type);

	// Cap minimum rate
	rate = max(rate, scdb->min_rate);

	if (rate < 10000 && (rate <= 0 || !(rnd()%10000 < rate)))
		return 0;

	// Duration cannot be reduced
	if (flag&SCSTART_NOTICKDEF)
		return i64max(tick, scdb->min_duration);

	tick -= tick*tick_def/10000;

#ifdef RENEWAL
	// Renewal applies item resistance also to duration
	if (sd) {
		for (const auto &it : sd->reseff) {
			if (it.id == type)
				tick -= tick * it.val / 10000;
		}
		if (sd->sc.getSCE(SC_COMMONSC_RESIST) && SC_COMMON_MIN <= type && type <= SC_COMMON_MAX)
			tick -= tick * sd->sc.getSCE(SC_COMMONSC_RESIST)->val1 / 100;
	}
#endif

	tick -= tick_def2;

	return i64max(tick, scdb->min_duration);
}

/**
 * Applies SC effect
 * @param bl: Source to apply effect
 * @param type: Status change (SC_*)
 * @param dval1~3: Depends on type of status change
 * Author: Ind
 */
void status_display_add(struct block_list *bl, enum sc_type type, int dval1, int dval2, int dval3) {
	struct eri *eri;
	struct sc_display_entry **sc_display;
	struct sc_display_entry ***sc_display_ptr;
	struct sc_display_entry *entry;
	int i;
	unsigned char sc_display_count;
	unsigned char *sc_display_count_ptr;

	nullpo_retv(bl);

	switch( bl->type ){
		case BL_PC: {
			map_session_data* sd = (map_session_data*)bl;

			sc_display_ptr = &sd->sc_display;
			sc_display_count_ptr = &sd->sc_display_count;
			eri = pc_sc_display_ers;
			}
			break;
		case BL_NPC: {
			struct npc_data* nd = (struct npc_data*)bl;

			sc_display_ptr = &nd->sc_display;
			sc_display_count_ptr = &nd->sc_display_count;
			eri = npc_sc_display_ers;
			}
			break;
		default:
			return;
	}

	sc_display = *sc_display_ptr;
	sc_display_count = *sc_display_count_ptr;

	ARR_FIND(0, sc_display_count, i, sc_display[i]->type == type);

	if( i != sc_display_count ) {
		sc_display[i]->val1 = dval1;
		sc_display[i]->val2 = dval2;
		sc_display[i]->val3 = dval3;
		return;
	}

	entry = ers_alloc(eri, struct sc_display_entry);

	entry->type = type;
	entry->val1 = dval1;
	entry->val2 = dval2;
	entry->val3 = dval3;

	RECREATE(sc_display, struct sc_display_entry *, ++sc_display_count);
	sc_display[sc_display_count - 1] = entry;
	*sc_display_ptr = sc_display;
	*sc_display_count_ptr = sc_display_count;
}

/**
 * Removes SC effect
 * @param bl: Source to remove effect
 * @param type: Status change (SC_*)
 * Author: Ind
 */
void status_display_remove(struct block_list *bl, enum sc_type type) {
	struct eri *eri;
	struct sc_display_entry **sc_display;
	struct sc_display_entry ***sc_display_ptr;
	int i;
	unsigned char sc_display_count;
	unsigned char *sc_display_count_ptr;

	nullpo_retv(bl);

	switch( bl->type ){
		case BL_PC: {
			map_session_data* sd = (map_session_data*)bl;

			sc_display_ptr = &sd->sc_display;
			sc_display_count_ptr = &sd->sc_display_count;
			eri = pc_sc_display_ers;
			}
			break;
		case BL_NPC: {
			struct npc_data* nd = (struct npc_data*)bl;

			sc_display_ptr = &nd->sc_display;
			sc_display_count_ptr = &nd->sc_display_count;
			eri = npc_sc_display_ers;
			}
			break;
		default:
			return;
	}

	sc_display = *sc_display_ptr;
	sc_display_count = *sc_display_count_ptr;

	ARR_FIND(0, sc_display_count, i, sc_display[i]->type == type);

	if( i != sc_display_count ) {
		int cursor;

		ers_free(eri, sc_display[i]);
		sc_display[i] = nullptr;

		/* The all-mighty compact-o-matic */
		for( i = 0, cursor = 0; i < sc_display_count; i++ ) {
			if( sc_display[i] == nullptr )
				continue;

			if( i != cursor )
				sc_display[cursor] = sc_display[i];

			cursor++;
		}

		if( !(sc_display_count = cursor) ) {
			aFree(sc_display);
			sc_display = nullptr;
		}

		*sc_display_ptr = sc_display;
		*sc_display_count_ptr = sc_display_count;
	}
}

/**
 * Applies SC defense to a given status change
 * This function also determines whether or not the status change will be applied
 * @param src: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param bl: Target of the status change (See: enum sc_type)
 * @param type: Status change (SC_*)
 * @param rate: Initial percentage rate of affecting bl (0~10000)
 * @param val1~4: Depends on type of status change
 * @param duration: Initial duration that the status change affects bl
 * @param flag: Value which determines what parts to calculate. See e_status_change_start_flags
 * @param delay: Delay in milliseconds before the SC is applied
 * @return adjusted duration based on flag values
 */
int status_change_start(struct block_list* src, struct block_list* bl,enum sc_type type,int rate,int val1,int val2,int val3,int val4,t_tick duration,unsigned char flag, int32 delay) {
	map_session_data *sd = nullptr;
	status_change* sc;
	struct status_change_entry* sce;
	struct view_data *vd;
	int undead_flag, tick_time = 0;
	bool sc_isnew = true;
	std::shared_ptr<s_status_change_db> scdb = status_db.find(type);

	nullpo_ret(bl);
	sc = status_get_sc(bl);

	if( !scdb ) {
		ShowError("status_change_start: Invalid status change (%d)!\n", type);
		return 0;
	}

	if( !sc )
		return 0; // Unable to receive status changes

	if( bl->type != BL_NPC && status_isdead(*bl) && ( type != SC_NOCHAT && type != SC_JAILED ) ) // SC_NOCHAT and SC_JAILED should work even on dead characters
		return 0;

	if (status_change_isDisabledOnMap(type, map_getmapdata(bl->m)))
		return 0;

	if (sc->getSCE(SC_GRAVITYCONTROL))
		return 0; // !TODO: Confirm what statuses/conditions (if not all) are blocked.

	// Uncomment to prevent status adding hp to gvg mob (like bloodylust=hp*3 etc...
//	if (bl->type == BL_MOB)
//		if (util::vector_exists(status_get_race2(bl), RC2_GVG) && status_sc2scb_flag(type)&SCB_MAXHP) return 0;

	// Fail if Madogear is active
	if (sc->option&OPTION_MADOGEAR && flag&SCSTART_NOAVOID && scdb->flag[SCF_FAILEDMADO])
		return 0;

	status_data* status = status_get_status_data(*bl);

	// Check for Boss resistances
	if(status->mode&MD_STATUSIMMUNE && !(flag&SCSTART_NOAVOID) && scdb->flag[SCF_BOSSRESIST])
		return 0;

	// Check for MVP resistance
	if(status->mode&MD_MVP && !(flag&SCSTART_NOAVOID) && scdb->flag[SCF_MVPRESIST])
		return 0;

	// End the SCs from the list and immediately return
	// If anything in this list is removed, the rest is ignored.
	if (!scdb->endreturn.empty()) {
		bool isRemoved = false;

		for (const auto &it : scdb->endreturn) {
			sc_type rem_sc = it;

			if (sc->getSCE(rem_sc)) {
				status_change_end(bl, rem_sc);
				isRemoved = true;
			}
		}

		if (isRemoved) // Something was removed, don't give the status
			return 1; // Return 1 so that sc_start can be checked as success
	}

	// Check failing SCs from list
	if (!scdb->fail.empty()) {
		for (const auto &it : scdb->fail) {
			// Don't let OPT1 that have RemoveOnDamaged start a new effect in the same attack.
			if (sc->getSCE(it) || sc->lastEffect == it)
				return 0;
		}
	}

	// Adjust tick according to status resistances
	if( !(flag&(SCSTART_NOAVOID|SCSTART_LOADED)) ) {
		duration = status_get_sc_def(src, bl, type, rate, duration, flag);
		if( !duration )
			return 0;
	}

	int tick = (int)duration;

	sd = BL_CAST(BL_PC, bl);
	vd = status_get_viewdata(bl);

	undead_flag = battle_check_undead(status->race,status->def_ele);
	// Check for immunities / sc fails
	switch (type) {
		case SC_VACUUM_EXTREME:
			if (sc && sc->getSCE(SC_VACUUM_EXTREME_POSTDELAY) && sc->getSCE(SC_VACUUM_EXTREME_POSTDELAY)->val2 == val2) // Ignore post delay from other vacuum (this will make stack effect enabled)
				return 0;
			break;
		case SC_STONE:
		case SC_STONEWAIT:
		case SC_FREEZE:
			// Undead are immune to Freeze/Stone
			if (undead_flag && !(flag&SCSTART_NOAVOID))
				return 0;
			break;
		case SC_BURNING:
			// Level 2 Fire Element is immune
			if (status->def_ele == ELE_FIRE && status->ele_lv == 2)
				return 0;
			break;
		case SC_ALL_RIDING:
			if( !sd || sc->option&(OPTION_RIDING|OPTION_DRAGON|OPTION_WUG|OPTION_MADOGEAR) )
				return 0;
			break;
		case SC_SIGNUMCRUCIS:
			// Only affects demons and undead element (but not players)
			if((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC)
				return 0;
			break;
		case SC_KYRIE:
		case SC_TUNAPARTY:
			if (bl->type == BL_MOB)
				return 0;
			break;
		case SC_ADRENALINE:
			if(sd && !pc_check_weapontype(sd,skill_get_weapontype(BS_ADRENALINE)))
				return 0;
			break;
		case SC_ADRENALINE2:
			if(sd && !pc_check_weapontype(sd,skill_get_weapontype(BS_ADRENALINE2)))
				return 0;
			break;
		case SC_CLOAKING:
			// Avoid cloaking with no wall and low skill level. [Skotlex]
			// Due to the cloaking card, we have to check the wall versus to known
			// skill level rather than the used one. [Skotlex]
			// if (sd && val1 < 3 && skill_check_cloaking(bl,nullptr))
			if( sd && pc_checkskill(sd, AS_CLOAKING) < 3 && !skill_check_cloaking(bl,nullptr) )
				return 0;
			break;
		case SC_MODECHANGE: {
				int32 mode;
				struct status_data *bstatus = status_get_base_status(bl);
				if (!bstatus) return 0;
				if (sc->getSCE(type)) { // Pile up with previous values.
					if (!val2) val2 = sc->getSCE(type)->val2;
					val3 |= sc->getSCE(type)->val3;
					val4 |= sc->getSCE(type)->val4;
				}
				mode = val2 ? ((val2&~MD_MASK) | val2) : bstatus->mode; // Base mode
				if (val4) mode = static_cast<e_mode>(mode&~val4); // Del mode
				if (val3) mode = static_cast<e_mode>(mode | val3); // Add mode
				if (mode == bstatus->mode) { // No change.
					if (sc->getSCE(type)) // Abort previous status
						return status_change_end(bl, type);
					return 0;
				}
			}
			break;
		// Strip skills, need to divest something or it fails.
		case SC_STRIPWEAPON:
			if (val2 == 1)
				val2 = 0; // Brandish Spear/Bowling Bash effet. Do not take weapon off.
			else if (sd && !(flag&SCSTART_LOADED)) { // Apply sc anyway if loading saved sc_data
				short i;
				uint8 successFlag = 0;
				if(sd->bonus.unstripable_equip&EQP_WEAPON)
					return 0;
				i = sd->equip_index[EQI_HAND_L];
				if (i>=0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON) {
					successFlag|=1;
					pc_unequipitem(sd,i,3); // Left-hand weapon
				}
	
				i = sd->equip_index[EQI_HAND_R];
				if (i>=0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON) {
					successFlag|=2;
					pc_unequipitem(sd,i,3);
				}
				if (!successFlag) return 0;
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_STRIPSHIELD:
			if( val2 == 1 ) val2 = 0; // GX effect. Do not take shield off..
			else
			if (sd && !(flag&SCSTART_LOADED)) {
				short i;
				if(sd->bonus.unstripable_equip&EQP_SHIELD)
					return 0;
				i = sd->equip_index[EQI_HAND_L];
				if ( i < 0 || !sd->inventory_data[i] || sd->inventory_data[i]->type != IT_ARMOR )
					return 0;
				pc_unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_STRIPARMOR:
			if (sd && !(flag&SCSTART_LOADED)) {
				short i;
				if(sd->bonus.unstripable_equip&EQP_ARMOR)
					return 0;
				i = sd->equip_index[EQI_ARMOR];
				if ( i < 0 || !sd->inventory_data[i] )
					return 0;
				pc_unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_STRIPHELM:
			if (sd && !(flag&SCSTART_LOADED)) {
				short i;
				if(sd->bonus.unstripable_equip&EQP_HELM)
					return 0;
				i = sd->equip_index[EQI_HEAD_TOP];
				if ( i < 0 || !sd->inventory_data[i] )
					return 0;
				pc_unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_SHADOW_STRIP:
			if (sd && !(flag&SCSTART_LOADED)) {
				if (sd->bonus.unstripable_equip&EQP_SHADOW_GEAR)
					return 0;

				bool successFlag = false;

				for( int i = EQI_SHADOW_ARMOR; i <= EQI_SHADOW_ACC_L; i++ ){
					int index = sd->equip_index[i];

					if( index >= 0 && sd->inventory_data[index] != nullptr ){
						pc_unequipitem( sd, index, 3 );
						successFlag = true;
					}
				}

				if (!successFlag)
					return 0;
			}
			if (tick == 1)
				return 1;
			break;
		case SC_MERC_FLEEUP:
		case SC_MERC_ATKUP:
		case SC_MERC_HPUP:
		case SC_MERC_SPUP:
		case SC_MERC_HITUP:
			if( bl->type != BL_MER )
				return 0; // Stats only for Mercenaries
			break;
		case SC_STRFOOD:
			if (sc->getSCE(SC_FOOD_STR_CASH) && sc->getSCE(SC_FOOD_STR_CASH)->val1 > val1)
				return 0;
			break;
		case SC_AGIFOOD:
			if (sc->getSCE(SC_FOOD_AGI_CASH) && sc->getSCE(SC_FOOD_AGI_CASH)->val1 > val1)
				return 0;
			break;
		case SC_VITFOOD:
			if (sc->getSCE(SC_FOOD_VIT_CASH) && sc->getSCE(SC_FOOD_VIT_CASH)->val1 > val1)
				return 0;
			break;
		case SC_INTFOOD:
			if (sc->getSCE(SC_FOOD_INT_CASH) && sc->getSCE(SC_FOOD_INT_CASH)->val1 > val1)
				return 0;
			break;
		case SC_DEXFOOD:
			if (sc->getSCE(SC_FOOD_DEX_CASH) && sc->getSCE(SC_FOOD_DEX_CASH)->val1 > val1)
				return 0;
			break;
		case SC_LUKFOOD:
			if (sc->getSCE(SC_FOOD_LUK_CASH) && sc->getSCE(SC_FOOD_LUK_CASH)->val1 > val1)
				return 0;
			break;
		case SC_FOOD_STR_CASH:
			if (sc->getSCE(SC_STRFOOD) && sc->getSCE(SC_STRFOOD)->val1 > val1)
				return 0;
			break;
		case SC_FOOD_AGI_CASH:
			if (sc->getSCE(SC_AGIFOOD) && sc->getSCE(SC_AGIFOOD)->val1 > val1)
				return 0;
			break;
		case SC_FOOD_VIT_CASH:
			if (sc->getSCE(SC_VITFOOD) && sc->getSCE(SC_VITFOOD)->val1 > val1)
				return 0;
			break;
		case SC_FOOD_INT_CASH:
			if (sc->getSCE(SC_INTFOOD) && sc->getSCE(SC_INTFOOD)->val1 > val1)
				return 0;
			break;
		case SC_FOOD_DEX_CASH:
			if (sc->getSCE(SC_DEXFOOD) && sc->getSCE(SC_DEXFOOD)->val1 > val1)
				return 0;
			break;
		case SC_FOOD_LUK_CASH:
			if (sc->getSCE(SC_LUKFOOD) && sc->getSCE(SC_LUKFOOD)->val1 > val1)
				return 0;
			break;
		case SC_CAMOUFLAGE:
			if( sd && pc_checkskill(sd, RA_CAMOUFLAGE) < 3 && !skill_check_camouflage(bl,nullptr) )
				return 0;
			break;
		case SC__STRIPACCESSORY:
			if( sd ) {
				short i = -1;
				if( !(sd->bonus.unstripable_equip&EQP_ACC_L) ) {
					i = sd->equip_index[EQI_ACC_L];
					if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
						pc_unequipitem(sd,i,3); // Left-Accessory
				}
				if( !(sd->bonus.unstripable_equip&EQP_ACC_R) ) {
					i = sd->equip_index[EQI_ACC_R];
					if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
						pc_unequipitem(sd,i,3); // Right-Accessory
				}
				if( i < 0 )
					return 0;
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_C_MARKER:
			if (src == bl)
				return 0;
			else {
				status_change *tsc = status_get_sc(bl);
				// Failed if the target is already marked and the new marker that isn't same marker
				if (tsc && tsc->getSCE(type) && tsc->getSCE(type)->val2 != src->id)
					return 0;
			}
			break;
		case SC_MADNESSCANCEL:
			if (sc->getSCE(type)) { // Toggle the status but still consume requirements.
				status_change_end(bl, type);
				return 0;
			}
			break;
		case SC_TOXIN:
		case SC_PARALYSE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_OBLIVIONCURSE:
		case SC_LEECHESEND:
			if (val3 == 0) // Don't display icon on self
				flag |= SCSTART_NOICON;
			for (int32 i = SC_TOXIN; i <= SC_LEECHESEND; i++) {
				if (sc->getSCE(i) && sc->getSCE(i)->val3 == 1) // It doesn't stack or even renew on the target
					return 0;
				else if (sc->getSCE(i) && sc->getSCE(i)->val3 == 0)
					status_change_end(bl, static_cast<sc_type>(i)); // End the bonus part on the caster
			}
			break;
		case SC_SPIRIT:
			if( sd ){
				uint64 target_class = 0;
				uint64 mask = MAPID_UPPERMASK;

				switch( val2 ){
					case SL_ALCHEMIST:
						target_class = MAPID_ALCHEMIST;
						break;
					case SL_ASSASIN:
						target_class = MAPID_ASSASSIN;
						break;
					case SL_BARDDANCER:
						target_class = MAPID_BARDDANCER;
						break;
					case SL_BLACKSMITH:
						target_class = MAPID_BLACKSMITH;
						break;
					case SL_CRUSADER:
						target_class = MAPID_CRUSADER;
						break;
					case SL_HUNTER:
						target_class = MAPID_HUNTER;
						break;
					case SL_KNIGHT:
						target_class = MAPID_KNIGHT;
						break;
					case SL_MONK:
						target_class = MAPID_MONK;
						break;
					case SL_PRIEST:
						target_class = MAPID_PRIEST;
						break;
					case SL_ROGUE:
						target_class = MAPID_ROGUE;
						break;
					case SL_SAGE:
						target_class = MAPID_SAGE;
						break;
					case SL_SOULLINKER:
						target_class = MAPID_SOUL_LINKER;
						break;
					case SL_STAR:
						target_class = MAPID_STAR_GLADIATOR;
						break;
					case SL_SUPERNOVICE:
						target_class = MAPID_SUPER_NOVICE;
						break;
					case SL_WIZARD:
						target_class = MAPID_WIZARD;
						break;
					case SL_HIGH:
						if( sd->status.base_level >= 70 ){
							return 0;
						}

						switch (sd->class_) {
							case MAPID_SWORDMAN_HIGH:
							case MAPID_MAGE_HIGH:
							case MAPID_ARCHER_HIGH:
							case MAPID_ACOLYTE_HIGH:
							case MAPID_MERCHANT_HIGH:
							case MAPID_THIEF_HIGH:
								// Only these classes are allowed.
								break;
							default:
								return 0;
						}

						// Set these to pass the check below.
						mask = sd->class_;
						target_class = sd->class_;
						break;
					default:
						ShowError( "Unknown skill id %d for SC_SPIRIT.\n", val2 );
						return 0;
				}

				if( ( sd->class_ & mask ) != target_class ){
					return 0;
				}
			}else{
				// Status change is only applicable for players
				return 0;
			}
			break;
		case SC_SOULGOLEM:
		case SC_SOULSHADOW:
		case SC_SOULFALCON:
		case SC_SOULFAIRY:
			if( sd == nullptr ){
				// Status change is only applicable for players
				return 0;
			}
			break;
	}

	// Check for OPT1 stacking
	if (sc->opt1 > OPT1_NONE && scdb->opt1 > OPT1_NONE) {
		for (const auto &status_it : status_db) {
			sc_type opt1_type = status_it.second->type;

			if (sc->getSCE(opt1_type) && status_it.second->opt1 > OPT1_NONE)
				status_change_end(bl, opt1_type);
		}
	}

	// Before overlapping fail, one must check for status cured.
	std::vector<sc_type> endlist;

	if (type == SC_BERSERK && val3 == SC__BLOODYLUST) //There is some reasons that using SC_BERSERK first before SC__BLOODYLUST itself on Akinari's fix
		endlist = status_db.getEndOnStart(SC__BLOODYLUST);
	else
		endlist = scdb->endonstart;

	// End the SCs from the list
	if (!endlist.empty()) {
		for (const auto &it : endlist) {
			sc_type rem_sc = it;

			if (sc->getSCE(rem_sc)) {
				switch (rem_sc) {
					case SC_BERSERK:
					case SC_SATURDAYNIGHTFEVER:
						sc->getSCE(rem_sc)->val2 = 0; // Mark to not lose hp
						[[fallthrough]];
					default:
						status_change_end(bl, rem_sc);
						break;
				}
			}
		}
	}

	// List of hardcoded status cured.
	switch (type) {
		case SC_BLESSING:
			if (bl->type == BL_PC) {
				// Remove Curse first, Stone is only removed if the target is not cursed
				if (sc->getSCE(SC_CURSE)) {
					status_change_end(bl, SC_CURSE);
					return 1; // End Curse and do not give stat boost
				} else if (sc->getSCE(SC_STONE)) {
					status_change_end(bl, SC_STONE);
					return 1; // End Stone and do not give stat boost
				}
			}
			if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
				status_change_end(bl, SC_SPIRIT);
			break;
		case SC_INCREASEAGI:
			if(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_HIGH)
				status_change_end(bl, SC_SPIRIT);
			break;
		case SC_DELUGE:
			if (sc->getSCE(SC_FOGWALL) && sc->getSCE(SC_BLIND))
				status_change_end(bl, SC_BLIND);
			break;
		case SC_SILENCE:
			if (sc->getSCE(SC_GOSPEL) && sc->getSCE(SC_GOSPEL)->val4 == BCT_SELF)
				status_change_end(bl, SC_GOSPEL);
			break;
		case SC_IMPOSITIO:
			if (sc->getSCE(SC_IMPOSITIO) && sc->getSCE(SC_IMPOSITIO)->val1 > val1) //Replace higher level effect for lower.
				status_change_end(bl,SC_IMPOSITIO);
			break;
		case SC_ENDURE:
			if (sd && sd->special_state.no_walk_delay)
				return 1;
			break;
		case SC_MADOGEAR:
			status_db.removeByStatusFlag(bl, { SCF_MADOCANCEL });
			if (sd)
				pc_bonus_script_clear(sd, BSF_REM_ON_MADOGEAR);
			break;
		default:
			break;
	}

	// Check for overlapping fails
	if( (sce = sc->getSCE(type)) ) {
		switch( type ) {
			case SC_MERC_FLEEUP:
			case SC_MERC_ATKUP:
			case SC_MERC_HPUP:
			case SC_MERC_SPUP:
			case SC_MERC_HITUP:
				if( sce->val1 > val1 )
					val1 = sce->val1;
				break;
			case SC_ADRENALINE:
			case SC_ADRENALINE2:
			case SC_WEAPONPERFECTION:
			case SC_OVERTHRUST:
				if (sce->val2 > val2)
					return 0;
				break;
			case SC_GOSPEL:
				 // Must not override a casting gospel char.
				if(sce->val4 == BCT_SELF)
					return 0;
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_ENDURE:
				if(sce->val4 && !val4)
					return 1; // Don't let you override infinite endure.
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_JAILED:
				// When a player is already jailed, do not edit the jail data.
				val2 = sce->val2;
				val3 = sce->val3;
				val4 = sce->val4;
				break;
			case SC_SHAPESHIFT:
			case SC_PROPERTYWALK:
				break;
			case SC_LEADERSHIP:
			case SC_GLORYWOUNDS:
			case SC_SOULCOLD:
			case SC_HAWKEYES:
				if( sce->val4 && !val4 ) // You cannot override master guild aura
					return 0;
				break;
			case SC_JOINTBEAT:
				if (sc && sc->getSCE(type)->val2 & BREAK_NECK)
					return 0; // BREAK_NECK cannot be stacked with new breaks until the status is over.
				val2 |= sce->val2; // Stackable ailments
				[[fallthrough]];
			default:
				if (scdb->flag[SCF_OVERLAPIGNORELEVEL])
					break;
				if(sce->val1 > val1)
					return 1; // Return true to not mess up skill animations. [Skotlex]
		}
	}

	vd = status_get_viewdata(bl);
	std::bitset<SCB_MAX> calc_flag = scdb->calc_flag;

	if(!(flag&SCSTART_LOADED)) // &4 - Do not parse val settings when loading SCs
	switch(type)
	{
		/* Permanent effects */
		case SC_AETERNA:
		case SC_MODECHANGE:
		case SC_WEIGHT50:
		case SC_WEIGHT90:
		case SC_BROKENWEAPON:
		case SC_BROKENARMOR:
		case SC_READYSTORM:
		case SC_READYDOWN:
		case SC_READYCOUNTER:
		case SC_READYTURN:
		case SC_DODGE:
		case SC_PUSH_CART:
		case SC_SPRITEMABLE:
		case SC_CLAN_INFO:
		case SC_DAILYSENDMAILCNT:
		case SC_SOULATTACK:
			tick = INFINITE_TICK;
			break;

		case SC_KEEPING:
		case SC_BARRIER: {
			unit_data *ud = unit_bl2ud(bl);

			if (ud)
				ud->attackabletime = ud->canact_tick = ud->canmove_tick = gettick() + tick;
		}
			break;
		case SC_DECREASEAGI:
		case SC_INCREASEAGI:
		case SC_ADORAMUS:
			if (type == SC_ADORAMUS) {
				// 1000% base chance to blind, but still can be resisted
				sc_start(src, bl, SC_BLIND, 1000, val1, skill_get_time(scdb->skill_id, val1));
				if (sc->getSCE(SC_ADORAMUS))
					return 0; //Adoramus can't refresh itself, but it can cause blind again
			}
			val2 = 2 + val1; // Agi change
			break;
		case SC_ENDURE:
			val2 = 7; // Hit-count [Celest]
			if( !(flag&SCSTART_NOAVOID) && (bl->type&(BL_PC|BL_MER)) && !map_flag_gvg2(bl->m) && !map_getmapflag(bl->m, MF_BATTLEGROUND) && !val4 ) {
				map_session_data *tsd;
				if( sd ) {
					int i;
					for( i = 0; i < MAX_DEVOTION; i++ ) {
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
							status_change_start(src,&tsd->bl, type, 10000, val1, val2, val3, val4, tick, SCSTART_NOAVOID|SCSTART_NOICON);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
					status_change_start(src,&tsd->bl, type, 10000, val1, val2, val3, val4, tick, SCSTART_NOAVOID|SCSTART_NOICON);
			}
			if( val4 )
				tick = INFINITE_TICK;
			break;
		case SC_AUTOBERSERK:
			if (status->hp < status->max_hp / 4 &&
				(!sc->getSCE(SC_PROVOKE) || sc->getSCE(SC_PROVOKE)->val4==0))
					sc_start4(src,bl,SC_PROVOKE,100,10,0,0,1,60000);
			tick = INFINITE_TICK;
			break;
		case SC_SIGNUMCRUCIS:
			val2 = 10 + 4*val1; // Def reduction
			tick = INFINITE_TICK;
			clif_emotion(bl, ET_SWEAT);
			break;
		case SC_MAXIMIZEPOWER:
			tick_time = val2 = tick>0?tick:60000;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			break;
		case SC_EDP:
			val2 = (val1 + 1) / 2 + 2; // Chance to Poison enemies.
#ifndef RENEWAL
			val3 = 50*(val1+1); // Damage increase (+50 +50*lv%)
#endif
			if (sd) {
				uint16 poison_level = pc_checkskill(sd, GC_RESEARCHNEWPOISON);

				if (poison_level > 0) {
					tick += 30000; // Base of 30 seconds
					tick += poison_level * 15 * 1000; // Additional 15 seconds per level
				}
			}
			break;
		case SC_POISONREACT:
#ifdef RENEWAL
			val2= (val1 + 1) / 2;
#else
			val2=(val1+1)/2 + val1/10; // Number of counters [Skotlex]
#endif
			val3=50; // + 5*val1; // Chance to counter. [Skotlex]
			break;
		case SC_MAGICROD:
			val2 = val1*20; // SP gained
			break;
		case SC_KYRIE:
			if( val4 ) { // Formulas for Praefatio
				val2 = (status->max_hp * (val1 * 2 + 10) / 100) + val4 * 2; //%Max HP to absorb
				val3 = 6 + val1; //Hits
			} else { // Formulas for Kyrie Eleison
				val2 = status->max_hp * (val1 * 2 + 10) / 100;
				val3 = (val1 / 2 + 5);
			}
			break;
		case SC_MAGICPOWER:
#ifdef RENEWAL
			val3 = 5 * val1; // Matk% increase
#else
			val2 = 1; // Lasts 1 invocation
			val3 = 10 * val1; // Matk% increase
			val4 = 0; // 0 = ready to be used, 1 = activated and running
#endif
			break;
		case SC_SACRIFICE:
			val2 = 5; // Lasts 5 hits
			tick = INFINITE_TICK;
			break;
		case SC_ENCPOISON:
			val2= 250+50*val1; // Poisoning Chance (2.5+0.5%) in 1/10000 rate
			break;
		case SC_ELEMENTALCHANGE:
			// val1 : Element Lvl (if called by skill lvl 1, takes random value between 1 and 4)
			// val2 : Element (When no element, random one is picked)
			// val3 : 0 = called by skill 1 = called by script (fixed level)
			if( !val2 ) val2 = rnd()%ELE_ALL;

			if( val1 == 1 && val3 == 0 )
				val1 = 1 + rnd()%4;
			else if( val1 > 4 )
				val1 = 4; // Max Level
			val3 = 0; // Not need to keep this info.
			break;
		case SC_PROVIDENCE:
			val2 = val1*5; // Race/Ele resist
			break;
		case SC_REFLECTSHIELD:
			val2 = 10+val1*3; // %Dmg reflected
			// val4 used to mark if reflect shield is an inheritance bonus from Devotion
			if( !(flag&SCSTART_NOAVOID) && (bl->type&(BL_PC|BL_MER)) ) {
				map_session_data *tsd;
				if( sd ) {
					int i;
					for( i = 0; i < MAX_DEVOTION; i++ ) {
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
							status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 1, tick, SCSTART_NOAVOID|SCSTART_NOICON);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
					status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 1, tick, SCSTART_NOAVOID|SCSTART_NOICON);
			}
			break;
		case SC_STRIPWEAPON:
			if (!sd) // Watk reduction
				val2 = 25;
			break;
		case SC_STRIPSHIELD:
			if (!sd) // Def reduction
				val2 = 15;
			break;
		case SC_STRIPARMOR:
			if (!sd) // Vit reduction
				val2 = 40;
			break;
		case SC_STRIPHELM:
			if (!sd) // Int reduction
				val2 = 40;
			break;
		case SC_AUTOSPELL:
			// Val1 Skill LV of Autospell
			// Val2 Skill ID to cast
			// Val3 Max Lv to cast
#ifdef RENEWAL
			val4 = val1 * 2; // Chance of casting
#else
			val4 = 5 + val1*2; // Chance of casting
#endif
			break;
		case SC_VOLCANO:
			{
				int8 enchant_eff[] = { 10, 14, 17, 19, 20 }; // Enchant addition
				uint8 i = max((val1-1)%5, 0);

#ifdef RENEWAL
				val2 = 5 + val1 * 5; // ATK/MATK increase
#else
				val2 = val1*10; // Watk increase
				if (status->def_ele != ELE_FIRE)
					val2 = 0;
#endif
				val3 = enchant_eff[i];
			}
			break;
		case SC_VIOLENTGALE:
			{
				int8 enchant_eff[] = { 10, 14, 17, 19, 20 }; // Enchant addition
				uint8 i = max((val1-1)%5, 0);

				val2 = val1*3; // Flee increase
#ifndef RENEWAL
				if (status->def_ele != ELE_WIND)
					val2 = 0;
#endif
				val3 = enchant_eff[i];
			}
			break;
		case SC_DELUGE:
			{
				int8 deluge_eff[]  = {  5,  9, 12, 14, 15 }; // HP addition rate n/100
				int8 enchant_eff[] = { 10, 14, 17, 19, 20 }; // Enchant addition
				uint8 i = max((val1-1)%5, 0);

				val2 = deluge_eff[i]; // HP increase
#ifndef RENEWAL
				if (status->def_ele != ELE_WATER)
					val2 = 0;
#endif
				val3 = enchant_eff[i];
			}
			break;
		case SC_SUITON:
			if (!val2 || (sd && (sd->class_&MAPID_BASEMASK) == MAPID_NINJA)) {
				// No penalties.
				val2 = 0; // Agi penalty
				val3 = 0; // Walk speed penalty
				break;
			}
			val3 = 50;
			val2 = 3*((val1+1)/3);
			if (val1 > 4) val2--;
			//Suiton is a special case, stop effect is forced and only happens when target enters it
			if (!unit_blown_immune(bl, 0x1))
				unit_stop_walking(bl, 9);
			break;
		case SC_ONEHAND:
		case SC_TWOHANDQUICKEN:
			val2 = 300;
			if (val1 > 10) // For boss casted skills [Skotlex]
				val2 += 20*(val1-10);
			break;
		case SC_MERC_QUICKEN:
			val2 = 300;
			break;
#ifndef RENEWAL_ASPD
		case SC_SPEARQUICKEN:
			val2 = 200+10*val1;
			break;
#endif
		case SC_DANCING:
			// val1 : Skill ID + LV
			// val2 : Skill Group of the Dance.
			// val3 : Brings the skill_lv (merged into val1 here)
			// val4 : Partner
			if (val1 == CG_MOONLIT)
				clif_status_change(bl,EFST_MOON,1,tick,0, 0, 0);
			val1|= (val3<<16);
			val3 = tick/1000; // Tick duration
			tick_time = 1000; // [GodLesZ] tick time
			break;
#ifndef RENEWAL
		case SC_LONGING:
			val2 = 500-100*val1; // Aspd penalty.
			break;
#else
		case SC_ENSEMBLEFATIGUE:
			val2 = 30; // Speed and ASPD penalty
			break;
		case SC_RICHMANKIM:
			val2 = 10 + 10 * val1; // Exp increase bonus
			break;
		case SC_DRUMBATTLE:
			val2 = 15 + val1 * 5; // Atk increase
			val3 = val1 * 15; // Def increase
			break;
		case SC_NIBELUNGEN:
			val2 = rnd() % RINGNBL_MAX; // See e_nibelungen_status
			break;
		case SC_SIEGFRIED:
			val2 = val1 * 3; // Elemental Resistance
			val3 = val1 * 5; // Status ailment resistance
			break;
		case SC_WHISTLE:
			val2 = 18 + 2 * val1; // Flee increase
			val3 = (val1 + 1) / 2; // Perfect dodge increase
			break;
		case SC_ASSNCROS:
			val2 = val1 < 10 ? val1 * 2 - 1 : 20; // ASPD increase
			break;
		case SC_POEMBRAGI:
			val2 = 2 * val1; // Cast time reduction
			val3 = 3 * val1; // After-cast delay reduction
			break;
		case SC_APPLEIDUN:
			val2 = val1 < 10 ? 9 + val1 : 20; // HP rate increase
			val3 = 2 * val1; // Potion recovery rate
			break;
		case SC_HUMMING:
			val2 = 4 * val1; // Hit increase
			break;
		case SC_DONTFORGETME:
			val2 = 1 + 30 * val1; // ASPD decrease
			val3 = 5 + 2 * val1; // Movement speed adjustment.
			break;
		case SC_FORTUNE:
			val2 = val1 * 10; // Critical increase
			break;
		case SC_SERVICE4U:
			val2 = val1 < 10 ? 9 + val1 : 20; // MaxSP percent increase
			val3 = 5 + val1; // SP cost reduction
			break;
#endif
		case SC_EXPLOSIONSPIRITS:
			val2 = 75 + 25*val1; // Cri bonus
			break;

		case SC_ASPDPOTION0:
		case SC_ASPDPOTION1:
		case SC_ASPDPOTION2:
		case SC_ASPDPOTION3:
			val2 = 50*(2+type-SC_ASPDPOTION0);
			break;

		case SC_ATTHASTE_CASH:
			val2 = 50*val1; // Just custom for pre-re
			break;

		case SC_NOCHAT:
			// A hardcoded interval of 60 seconds is expected, as the time that SC_NOCHAT uses is defined by
			// mmocharstatus.manner, each negative point results in 1 minute with this status activated.
			// This is done this way because the message that the client displays is hardcoded, and only
			// shows how many minutes are remaining. [Panikon]
			tick = 60000;
			val1 = battle_config.manner_system; // Mute filters.
			if (sd) {
				clif_changemanner( *sd );
				clif_updatestatus(*sd,SP_MANNER);
			}
			break;

		case SC_STONEWAIT:
			val3 = max(1, tick - delay); // Petrify time
			tick = delay;
			break;

		case SC_DPOISON:
			// Lose 10/15% of your life as long as it doesn't brings life below 25%
			if (status->hp > status->max_hp / 4) {
				int diff = status->max_hp*(bl->type==BL_PC?10:15)/100;
				if (status->hp - diff < status->max_hp / 4)
					diff = status->hp - (status->max_hp / 4);
				status_zap(bl, diff, 0);
			}
			[[fallthrough]];
		case SC_STONE:
		case SC_POISON:
		case SC_BLEEDING:
		case SC_BURNING:
		case SC_KILLING_AURA:
			tick_time = status_get_sc_interval(type);
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_TOXIN:
			if (val3 == 1) // Target
				tick_time = status_get_sc_interval(type);
			else // Caster
				tick_time = 1000;
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_DEATHHURT:
			if (val3 == 1)
				break;
			tick_time = status_get_sc_interval(type);
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_LEECHESEND:
			if (val3 == 0)
				break;
			tick_time = status_get_sc_interval(type);
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_PYREXIA:
			if (val3 == 1) { // Target
				// Causes blind for duration of pyrexia, unreducable and unavoidable, but can be healed with e.g. green potion
				status_change_start(src, bl, SC_BLIND, 10000, val1, 0, 0, 0, tick, SCSTART_NOAVOID | SCSTART_NOTICKDEF | SCSTART_NORATEDEF);
				tick_time = status_get_sc_interval(type);
				val4 = tick - tick_time; // Remaining time
			} else // Caster
				val2 = 15; // CRIT % and ATK % increase
			break;
		case SC_VENOMBLEED:
			if (val3 == 0) // Caster
				val2 = 30; // Reflect damage % reduction
			break;
		case SC_MAGICMUSHROOM:
			if (val3 == 1) { // Target
				tick_time = status_get_sc_interval(type);
				val4 = tick - tick_time; // Remaining time
			} else // Caster
				val2 = 10; // After-cast delay % reduction
			break;

		case SC_CONFUSION:
			if (!val4)
				clif_emotion(bl,ET_QUESTION);
			break;
		case SC_S_LIFEPOTION:
		case SC_L_LIFEPOTION:
		case SC_M_LIFEPOTION:
		case SC_S_MANAPOTION:
		case SC_G_LIFEPOTION:
			if( val1 == 0 ) return 0;
			// val1 = heal percent/amout
			// val2 = seconds between heals
			// val4 = total of heals
			if( val2 < 1 ) val2 = 1;
			if( (val4 = tick/(val2 * 1000)) < 1 )
				val4 = 1;
			tick_time = val2 * 1000; // [GodLesZ] tick time
			break;
		case SC_GRADUAL_GRAVITY:
			val2 = 10 * val1;
			tick_time = status_get_sc_interval(type);
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_ALL_STAT_DOWN:
			val2 = 20 * val1;
			if( val1 < skill_get_max( NPC_ALL_STAT_DOWN ) ){
				val2 -= 10;
			}
			break;
		case SC_DAMAGE_HEAL:
			switch( val1 ){
				case 1:
					val2 = BF_WEAPON;
					break;
				case 2:
					val2 = BF_MAGIC;
					break;
				case 3:
					//TODO: Absorb MISC damage? Both WEAPON & MAGIC damage? Which is correct on level 3?
					val2 = BF_MISC;
					break;
			}
			break;
		case SC_BOSSMAPINFO:
			if( sd == nullptr ){
				return 0;
			}else{
				// Search for Boss on this Map
				mob_data* boss_md = map_getmob_boss( bl->m );

				// No MVP on this map
				if( boss_md == nullptr ){
					clif_bossmapinfo( *sd, nullptr, BOSS_INFO_NOT );
					return 0;
				}

				val1 = boss_md->bl.id;
				tick_time = status_get_sc_interval( type );
				val4 = tick - tick_time; // Remaining time
			}
			break;
		case SC_HIDING:
			val2 = tick/1000;
			tick_time = 1000; // [GodLesZ] tick time
			val3 = 0; // Unused, previously speed adjustment
			val4 = val1+3; // Seconds before SP substraction happen.
			break;
		case SC_CHASEWALK:
			val2 = tick>0?tick:10000; // Interval at which SP is drained.
			val3 = 35 - 5 * val1; // Speed adjustment.
			if (sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_ROGUE)
				val3 -= 40;
			val4 = 10+val1*2; // SP cost.
			if (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND)) val4 *= 5;
			break;
		case SC_CLOAKING:
			if (!sd) // Monsters should be able to walk with no penalties. [Skotlex]
				val1 = 10;
			tick_time = val2 = tick>0?tick:60000; // SP consumption rate.
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			val3 = 0; // Unused, previously walk speed adjustment
			// val4&1 signals the presence of a wall.
			// val4&2 makes cloak not end on normal attacks [Skotlex]
			// val4&4 makes cloak not end on using skills
			if (bl->type == BL_PC || (bl->type == BL_MOB && ((TBL_MOB*)bl)->special_state.clone) )	// Standard cloaking.
				val4 |= battle_config.pc_cloak_check_type&7;
			else
				val4 |= battle_config.monster_cloak_check_type&7;
			break;
		case SC_SIGHT:			/* splash status */
		case SC_RUWACH:
		case SC_SIGHTBLASTER:
			val3 = skill_get_splash(val2, val1); // Val2 should bring the skill-id.
			val2 = tick/20;
			tick_time = 20; // [GodLesZ] tick time
			break;

		case SC_AUTOGUARD:
			if( !(flag&SCSTART_NOAVOID) ) {
				map_session_data *tsd;
				int i;
				for( i = val2 = 0; i < val1; i++) {
					int t = 5-(i / 2);
					val2 += (t < 0)? 1:t;
				}

				if( bl->type&(BL_PC|BL_MER) ) {
					if( sd ) {
						for( i = 0; i < MAX_DEVOTION; i++ ) {
							if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
								status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 0, tick, SCSTART_NOAVOID|SCSTART_NOICON);
						}
					}
					else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
						status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 0, tick, SCSTART_NOAVOID|SCSTART_NOICON);
				}
			}
			break;

		case SC_DEFENDER:
			if (!(flag&SCSTART_NOAVOID)) {
				val2 = 5 + 15*val1; // Damage reduction
				val3 = 0; // Unused, previously speed adjustment
				val4 = 250 - 50*val1; // Aspd adjustment

				if (sd) {
					map_session_data *tsd;
					int i;
					for (i = 0; i < MAX_DEVOTION; i++) { // See if there are devoted characters, and pass the status to them. [Skotlex]
						if (sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])))
							status_change_start(src,&tsd->bl,type,10000,val1,val2,val3,val4,tick,SCSTART_NOAVOID);
					}
				}
			}
			break;

		case SC_TENSIONRELAX:
			if (sd) {
				pc_setsit(sd);
				skill_sit(sd, true);
				clif_sitting(sd->bl);
			}
			val2 = 12; // SP cost
			tick_time = 10000; // Decrease at 10secs intervals.
			val3 = tick / tick_time;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			break;
		case SC_PARRYING:
		    val2 = 20 + val1*3; // Block Chance
			break;

		case SC_WINDWALK:
			val2 = (val1+1)/2; // Flee bonus is 1/1/2/2/3/3/4/4/5/5
			break;

		case SC_JOINTBEAT:
			if( val2&BREAK_NECK )
				sc_start2(src,bl,SC_BLEEDING,100,val1,val3,skill_get_time2(scdb->skill_id,val1));
			break;

		case SC_BERSERK:
			if( val3 == SC__BLOODYLUST )
				sc_start(src,bl,(sc_type)val3,100,val1,tick);
			else
				sc_start4(src,bl, SC_ENDURE, 100,10,0,0,1, tick);
			// HP healing is performing after the calc_status call.
			// Val2 holds HP penalty
			if (!val4) val4 = skill_get_time2(scdb->skill_id,val1);
			if (!val4) val4 = 10000; // Val4 holds damage interval
			val3 = tick/val4; // val3 holds skill duration
			tick_time = val4; // [GodLesZ] tick time
			break;

		case SC_GOSPEL:
			if(val4 == BCT_SELF) {	// Self effect
				val2 = tick/10000;
				tick_time = 10000; // [GodLesZ] tick time
				status_change_clear_buffs(bl, SCCB_BUFFS|SCCB_DEBUFFS|SCCB_CHEM_PROTECT); // Remove buffs/debuffs
			}
			break;

		case SC_MARIONETTE:
		{
			int stat;

			val3 = 0;
			val4 = 0;
			stat = ( sd ? sd->status.str : status_get_base_status(bl)->str ) / 2; val3 |= cap_value(stat,0,0xFF)<<16;
			stat = ( sd ? sd->status.agi : status_get_base_status(bl)->agi ) / 2; val3 |= cap_value(stat,0,0xFF)<<8;
			stat = ( sd ? sd->status.vit : status_get_base_status(bl)->vit ) / 2; val3 |= cap_value(stat,0,0xFF);
			stat = ( sd ? sd->status.int_: status_get_base_status(bl)->int_) / 2; val4 |= cap_value(stat,0,0xFF)<<16;
			stat = ( sd ? sd->status.dex : status_get_base_status(bl)->dex ) / 2; val4 |= cap_value(stat,0,0xFF)<<8;
			stat = ( sd ? sd->status.luk : status_get_base_status(bl)->luk ) / 2; val4 |= cap_value(stat,0,0xFF);
			break;
		}
		case SC_MARIONETTE2:
		{
			int stat,max_stat;
			// Fetch caster information
			struct block_list *pbl = map_id2bl(val1);
			status_change *psc = pbl?status_get_sc(pbl):nullptr;
			struct status_change_entry *psce = psc?psc->getSCE(SC_MARIONETTE):nullptr;

			if (!psce)
				return 0;

			// Fetch target's stats
			status_data* status2 = status_get_status_data(*bl); // Battle status

			val3 = 0;
			val4 = 0;
			max_stat = battle_config.max_parameter; // Cap to 99 (default)
			stat = (psce->val3 >>16)&0xFF; stat = min(stat, max_stat - status2->str ); val3 |= cap_value(stat,0,0xFF)<<16;
			stat = (psce->val3 >> 8)&0xFF; stat = min(stat, max_stat - status2->agi ); val3 |= cap_value(stat,0,0xFF)<<8;
			stat = (psce->val3 >> 0)&0xFF; stat = min(stat, max_stat - status2->vit ); val3 |= cap_value(stat,0,0xFF);
			stat = (psce->val4 >>16)&0xFF; stat = min(stat, max_stat - status2->int_); val4 |= cap_value(stat,0,0xFF)<<16;
			stat = (psce->val4 >> 8)&0xFF; stat = min(stat, max_stat - status2->dex ); val4 |= cap_value(stat,0,0xFF)<<8;
			stat = (psce->val4 >> 0)&0xFF; stat = min(stat, max_stat - status2->luk ); val4 |= cap_value(stat,0,0xFF);
			break;
		}
		case SC_SPIRIT:
			//1st Transcendent Spirit works similar to Marionette Control
			if(sd && val2 == SL_HIGH) {
				int stat,max_stat;
				status_data *status2 = status_get_base_status(bl);

				val3 = 0;
				val4 = 0;
				max_stat = (status_get_lv(bl)-10<50)?status_get_lv(bl)-10:50;
				stat = max(0, max_stat - status2->str); val3 |= cap_value(stat,0,0xFF)<<16;
				stat = max(0, max_stat - status2->agi ); val3 |= cap_value(stat,0,0xFF)<<8;
				stat = max(0, max_stat - status2->vit ); val3 |= cap_value(stat,0,0xFF);
				stat = max(0, max_stat - status2->int_); val4 |= cap_value(stat,0,0xFF)<<16;
				stat = max(0, max_stat - status2->dex ); val4 |= cap_value(stat,0,0xFF)<<8;
				stat = max(0, max_stat - status2->luk ); val4 |= cap_value(stat,0,0xFF);
			}
			break;

		case SC_REJECTSWORD:
			val2 = 15*val1; // Reflect chance
			val3 = 3; // Reflections
			tick = INFINITE_TICK;
			break;

		case SC_MEMORIZE:
			val2 = 5; // Memorized casts.
			tick = INFINITE_TICK;
			break;

#ifndef RENEWAL
		case SC_GRAVITATION:
			val2 = 50*val1; // aspd reduction
			break;
#endif

		case SC_REGENERATION:
			if (val1 == 1)
				val2 = 2;
			else
				val2 = val1; // HP Regerenation rate: 200% 200% 300%
			val3 = val1; // SP Regeneration Rate: 100% 200% 300%
			// If val4 comes set, this blocks regen rather than increase it.
			break;

		case SC_DEVOTION:
		{
			struct block_list *d_bl;
			status_change *d_sc;

			if( (d_bl = map_id2bl(val1)) && (d_sc = status_get_sc(d_bl)) && d_sc->count ) { // Inherits Status From Source
				const enum sc_type types[] = { SC_AUTOGUARD, SC_DEFENDER, SC_REFLECTSHIELD, SC_ENDURE };
				int i = (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND))?2:3;
				while( i >= 0 ) {
					enum sc_type type2 = types[i];
					if( d_sc->getSCE(type2) )
						status_change_start(d_bl, bl, type2, 10000, d_sc->getSCE(type2)->val1, 0, 0, (type2 == SC_REFLECTSHIELD ? 1 : 0), skill_get_time(status_db.getSkill(type2),d_sc->getSCE(type2)->val1), (type2 == SC_DEFENDER) ? SCSTART_NOAVOID : SCSTART_NOAVOID|SCSTART_NOICON);
					i--;
				}
			}
			break;
		}

		case SC_COMA: // Coma. Sends a char to 1HP. If val2, do not zap sp
			status_zap(bl, status->hp-1, val2?0:status->sp-1);
			return 1;
			break;
		case SC_CLOSECONFINE2:
		{
			struct block_list *src2 = val2?map_id2bl(val2):nullptr;
			status_change *sc2 = src2?status_get_sc(src2):nullptr;
			struct status_change_entry *sce2 = sc2?sc2->getSCE(SC_CLOSECONFINE):nullptr;

			if (src2 && sc2) {
				if (!sce2) { // Start lock on caster.
#ifdef RENEWAL
					val3 = 50; // Flee increase
#else
					val3 = 10; // Flee increase
#endif

					sc_start4(src2,src2,SC_CLOSECONFINE,100,val1,1,val3,0,tick+1000);
				} else { // Increase count of locked enemies and refresh time.
					(sce2->val2)++;
					delete_timer(sce2->timer, status_change_timer);
					sce2->timer = add_timer(gettick()+tick+1000, status_change_timer, src2->id, SC_CLOSECONFINE);
				}
			} else // Status failed.
				return 0;
		}
			break;
		case SC_KAITE:
			val2 = 1+val1/5; // Number of bounces: 1 + skill_lv/5
			break;
		case SC_KAUPE:
			switch (val1) {
				case 3: // 33*3 + 1 -> 100%
					val2++;
					[[fallthrough]];
				case 1:
				case 2: // 33, 66%
					val2 += 33*val1;
					val3 = 1; // Dodge 1 attack total.
					break;
				default: // Custom. For high level mob usage, higher level means more blocks. [Skotlex]
					val2 = 100;
					val3 = val1-2;
					break;
			}
			break;

		case SC_COMBO:
		{
			// val1: Skill ID
			// val2: When given, target (for autotargetting skills)
			// val3: When set, this combo time should NOT delay attack/movement
			// val3: If set to 2 this combo will delay ONLY attack
			// val3: TK: Last used kick
			// val4: TK: Combo time
			struct unit_data *ud = unit_bl2ud(bl);
			if ( ud && (!val3 || val3 == 2) ) {
				tick += 300 * battle_config.combo_delay_rate/100;
				ud->attackabletime = gettick()+tick;
				if( !val3 )
					unit_set_walkdelay(bl, gettick(), tick, 1);
			}
			val3 = 0;
			val4 = tick;
			break;
		}
		case SC_EARTHSCROLL:
			val2 = 11-val1; // Chance to consume: 11-skill_lv%
			break;
		case SC_RUN:
		{
			//Store time at which you started running.
			t_tick currenttick = gettick();
			// Note: this int64 value is stored in two separate int32 variables (FIXME)
			val3 = (int)(currenttick & 0x00000000ffffffffLL);
			val4 = (int)((currenttick & 0xffffffff00000000LL) >> 32);
			tick = INFINITE_TICK;
			break;
		}
		case SC_KAAHI:
			val2 = 200*val1; // HP heal
			val3 = 5*val1; // SP cost
			break;
		case SC_BLESSING:
			if (bl->type == BL_PC || (!undead_flag && status->race != RC_DEMON))
				val2 = val1;
			else
				val2 = 0; // 0 -> Half stat.
			break;
		case SC_TRICKDEAD:
			if (vd) vd->dead_sit = 1;
			tick = INFINITE_TICK;
			break;
		case SC_CONCENTRATE:
			val2 = 2 + val1;
			if (sd) { // Store the card-bonus data that should not count in the %
				val3 = sd->indexed_bonus.param_bonus[1]; // Agi
				val4 = sd->indexed_bonus.param_bonus[4]; // Dex
			} else
				val3 = val4 = 0;
			break;
		case SC_MAXOVERTHRUST:
			val2 = 20*val1; // Power increase
			break;
		case SC_OVERTHRUST:
		case SC_ADRENALINE2:
		case SC_ADRENALINE:
		case SC_WEAPONPERFECTION:
			{
				map_session_data * s_sd = BL_CAST(BL_PC, src);
				if (type == SC_OVERTHRUST) {
					// val2 holds if it was casted on self, or is bonus received from others
#ifdef RENEWAL
						val3 = (val2) ? 5 * val1 : (val1 > 4) ? 15 : (val1 > 2) ? 10 : 5; // Power increase
#else
						val3 = (val2) ? 5 * val1 : 5; // Power increase
#endif
				}
				else if (type == SC_ADRENALINE2 || type == SC_ADRENALINE) {
					val3 = (val2) ? 300 : 200; // Aspd increase
				}
				if (s_sd && pc_checkskill(s_sd, BS_HILTBINDING) > 0)
					tick += tick / 10; //If caster has Hilt Binding, duration increases by 10%
			}
			break;
		case SC_CONCENTRATION:
#ifdef RENEWAL
			val2 = 5 + val1 * 2; // Batk/Watk Increase
			val4 = 5 + val1 * 2; // Def reduction
#else
			val2 = 5*val1; // Batk/Watk Increase
			val4 = 5*val1; // Def reduction
#endif
			val3 = 10*val1; // Hit Increase
			sc_start(src, bl, SC_ENDURE, 100, 1, tick); // Level 1 Endure effect
			break;
		case SC_ANGELUS:
			val2 = 5*val1; // def increase
			break;
		case SC_IMPOSITIO:
			val2 = 5*val1; // WATK/MATK increase
			break;
		case SC_MELTDOWN:
			val2 = 100*val1; // Chance to break weapon
			val3 = 70*val1; // Change to break armor
			break;
		case SC_TRUESIGHT:
			val2 = 10*val1; // Critical increase
			val3 = 3*val1; // Hit increase
			break;
		case SC_SUN_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk)/2; // def increase
			break;
		case SC_MOON_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk)/10; // flee increase
			break;
		case SC_STAR_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk); // Aspd increase
			break;
		case SC_QUAGMIRE:
			val2 = (sd?5:10)*val1; // Agi/Dex decrease.
			break;

		// gs_something1 [Vicious]
		case SC_GATLINGFEVER:
			val2 = 20*val1; // Aspd increase
			val3 = 20+10*val1; // Atk increase
			val4 = 5*val1; // Flee decrease
			break;

		case SC_FLING:
			if (bl->type == BL_PC)
				val2 = 0; // No armor reduction to players.
			else
				val2 = 5*val1; // Def reduction
			val3 = 5*val1; // Def2 reduction
			break;
		case SC_PROVOKE:
			val2 = 2+3*val1; // Atk increase
			val3 = 5+5*val1; // Def reduction.
			// val4 signals autoprovoke.
			break;
		case SC_AVOID:
			// Speed change rate.
			if (bl->type == BL_HOM)
				val2 = 40 * val1;
			else
				val2 = 10 * val1;
			break;
		case SC_DEFENCE:
			// Vit bonus for players / Def bonus for homunculus
#ifdef RENEWAL
			val2 = 5 + (5 * val1);
#else
			val2 = 2 * val1;
#endif
			break;
		case SC_BLOODLUST:
			// Atk rate change
			val2 = 20 + (10 * val1);
			// Leech chance
			// It's actually 9 * level on both pre-re and renewal, despite the description
			val3 = 9 * val1;
			// Leech percent
			val4 = 20;
			break;
		case SC_FLEET:
			val2 = 30*val1; // Aspd change
			val3 = 5+5*val1; // bAtk/wAtk rate change
			break;
		case SC_MINDBREAKER:
			val2 = 20*val1; // matk increase.
			val3 = 12*val1; // mdef2 reduction.
			break;
		case SC_JAILED:
			// Val1 is duration in minutes. Use INT_MAX to specify 'unlimited' time.
			if (sd) {
				if (sd->mapindex != val2) {
					int pos =  (bl->x&0xFFFF)|(bl->y<<16), // Current Coordinates
					map_idx =  sd->mapindex; // Current Map
					// 1. Place in Jail (val2 -> Jail Map, val3 -> x, val4 -> y
					pc_setpos(sd,(unsigned short)val2,val3,val4, CLR_TELEPORT);
					// 2. Set restore point (val3 -> return map, val4 return coords
					val3 = map_idx;
					val4 = pos;
				} else if (!val3 || val3 == sd->mapindex) { // Use save point.
					val3 = mapindex_name2id( sd->status.save_point.map );
					val4 = (sd->status.save_point.x&0xFFFF)
						|(sd->status.save_point.y<<16);
				}
			}
			break;
		case SC_UTSUSEMI:
			val2=(val1+1)/2; // Number of hits blocked
			val3=skill_get_blewcount(NJ_UTSUSEMI, val1); // knockback value.
			break;
		case SC_BUNSINJYUTSU:
			val2=(val1+1)/2; // Number of hits blocked
			break;
		case SC_CHANGE:
			val2= 30*val1; // Vit increase
			val3= 20*val1; // Int increase
			break;
		case SC_SWOO:
			if(status_has_mode(status,MD_STATUSIMMUNE))
				tick /= 5; // !TODO: Reduce skill's duration. But for how long?
			break;
		case SC_ARMOR:
			// NPC_DEFENDER:
			val2 = 8; // Damage will be divided by this value
			// Attack requirements to be blocked:
			val3 = BF_LONG; // Range
			val4 = BF_WEAPON|BF_MISC; // Type
			break;
		case SC_ENCHANTARMS:
			// Make sure the received element is valid.
			if (val1 >= ELE_ALL)
				val1 = val1%ELE_ALL;
			else if (val1 < 0)
				val1 = rnd()%ELE_ALL;
			break;
		case SC_CRITICALWOUND:
			// Level 1 ~ 5 & 6 ~ 10 has different duration
			// Level 6 ~ 10 use effect of level 1 ~ 5
			val1 = 1 + ((val1-1)%5);
			val2 = 20*val1; // Heal effectiveness decrease
			break;
		case SC_MAGICMIRROR:
			// Level 1 ~ 5 & 6 ~ 10 has different duration
			// Level 6 ~ 10 use effect of level 1 ~ 5
			val1 = 1 + ((val1-1)%5);
			[[fallthrough]];
		case SC_SLOWCAST:
			val2 = 20*val1; // Magic reflection/cast rate
			break;

		case SC_ARMORCHANGE:
			if (val2 == NPC_ANTIMAGIC) { // Boost mdef
				val2 =-20;
				val3 = 20;
			} else { // Boost def
				val2 = 20;
				val3 =-20;
			}
			// Level 1 ~ 5 & 6 ~ 10 has different duration
			// Level 6 ~ 10 use effect of level 1 ~ 5
			val1 = 1 + ((val1-1)%5);
			val2 *= val1; // 20% per level
			val3 *= val1;
			break;
		case SC_EXPBOOST:
		case SC_JEXPBOOST:
		case SC_JP_EVENT04:
		case SC_PERIOD_RECEIVEITEM_2ND:
		case SC_PERIOD_PLUSEXP_2ND:
			if (val1 < 1)
				return 0;
			break;
		case SC_INCFLEE2:
		case SC_INCCRI:
			val2 = val1*10; // Actual boost (since 100% = 1000)
			break;
		case SC_SUFFRAGIUM:
#ifdef RENEWAL
			val2 = 5 + val1 * 5; // Speed cast decrease
#else
			val2 = 15 * val1; // Speed cast decrease
#endif
			break;
		case SC_INCHEALRATE:
			if (val1 < 1)
				val1 = 1;
			break;
		case SC_DOUBLECAST:
			val2 = 30+10*val1; // Trigger rate
			break;
		case SC_KAIZEL:
			val2 = 10*val1; // % of life to be revived with
			break;
		// case SC_ARMOR_ELEMENT_WATER:
		// case SC_ARMOR_ELEMENT_EARTH:
		// case SC_ARMOR_ELEMENT_FIRE:
		// case SC_ARMOR_ELEMENT_WIND:
		// case SC_ARMOR_RESIST:
			// Mod your resistance against elements:
			// val1 = water | val2 = earth | val3 = fire | val4 = wind
			// break;
		// case ????:
			// Place here SCs that have no SCB_* data, no skill associated, no ICON
			// associated, and yet are not wrong/unknown. [Skotlex]
			// break;

		case SC_MERC_FLEEUP:
		case SC_MERC_ATKUP:
		case SC_MERC_HITUP:
			val2 = 15 * val1;
			break;
		case SC_MERC_HPUP:
		case SC_MERC_SPUP:
			val2 = 5 * val1;
			break;
		case SC_REBIRTH:
			val2 = 20*val1; // % of life to be revived with
			break;
		case SC_INVINCIBLE:
			val2 = 100; // ATKpercent increase
			val3 = 50; // Speed increase
			val4 = 700; // ASPD increase
			break;

		case SC_MANU_DEF:
		case SC_MANU_ATK:
		case SC_MANU_MATK:
			val2 = 1; // Manuk group
			break;
		case SC_SPL_DEF:
		case SC_SPL_ATK:
		case SC_SPL_MATK:
			val2 = 2; // Splendide group
			break;

		/* General */
		case SC_FEAR:
			sc_start(src, bl, SC_ANKLE, 100, 0, 2000);
			break;

		/* Rune Knight */
		case SC_DEATHBOUND:
			val2 = 500 + 100 * val1;
			break;
		case SC_STONEHARDSKIN:
			if (!status_charge(bl, status->hp / 5, 0)) // 20% of HP
				return 0;
			if (sd)
				val1 = sd->status.job_level * pc_checkskill(sd, RK_RUNEMASTERY) / 4; // DEF/MDEF Increase
			break;
		case SC_REFRESH:
			status_heal(bl, status_get_max_hp(bl) * 25 / 100, 0, 1);
			status_change_clear_buffs(bl, SCCB_REFRESH);
			break;
		case SC_MILLENNIUMSHIELD:
			{
				int8 chance = rnd()%100;

				val2 = ((chance < 20) ? 4 : (chance < 50) ? 3 : 2); // Shield count
				val3 = 1000; // Shield HP
				clif_millenniumshield( *bl, val2 );
			}
 			break;
		case SC_ABUNDANCE:
			val4 = tick / 10000;
			tick_time = 10000; // [GodLesZ] tick time
			break;
		case SC_GIANTGROWTH:
			val2 = 30; // Damage success rate and STR increase
			break;
		case SC_LUXANIMA:
			val2 = 15; // Storm Blast success %
			val3 = 30; // Damage/HP/SP % increase
			break;

		/* Arch Bishop */
		case SC_RENOVATIO:
			val4 = tick / 5000;
			tick_time = 5000;
			break;
		case SC_SECRAMENT:
			val2 = 10 * val1;
			break;
		case SC_VENOMIMPRESS:
			val2 = 10 * val1;
			break;
		case SC_WEAPONBLOCKING:
			val2 = 10 + 2 * val1; // Chance
			val4 = tick / 5000;
			tick_time = 5000; // [GodLesZ] tick time
			break;
		case SC_OBLIVIONCURSE:
			if (val3 == 0)
				break;
			val4 = tick / 3000;
			tick_time = 3000; // [GodLesZ] tick time
			break;
		case SC_CLOAKINGEXCEED:
			val2 = (val1 + 1) / 2; // Hits
			val3 = (val1 - 1) * 10; // Walk speed
			if (bl->type == BL_PC)
				val4 |= battle_config.pc_cloak_check_type&7;
			else
				val4 |= battle_config.monster_cloak_check_type&7;
			tick_time = 1000; // [GodLesZ] tick time
			break;
		case SC_HALLUCINATIONWALK:
		case SC_NPC_HALLUCINATIONWALK:
			val2 = 50 * val1; // Evasion rate of physical attacks. Flee
			val3 = 10 * val1; // Evasion rate of magical attacks.
			break;
		case SC_MARSHOFABYSS:
			if( bl->type == BL_PC )
				val2 = 3 * val1; // AGI and DEX Reduction
			else // BL_MOB
				val2 = 6 * val1; // AGI and DEX Reduction
			val3 = 10 * val1; // Movement Speed Reduction
			break;
		case SC_FREEZE_SP:
			// val2 = sp drain per 10 seconds
			tick_time = 10000; // [GodLesZ] tick time
			break;
		case SC_SPHERE_1:
		case SC_SPHERE_2:
		case SC_SPHERE_3:
		case SC_SPHERE_4:
		case SC_SPHERE_5:
			if( !sd )
				return 0;	// Should only work on players.
			val4 = tick / 1000;
			if( val4 < 1 )
				val4 = 1;
			tick_time = 1000; // [GodLesZ] tick time
			break;
		case SC_SHAPESHIFT:
			switch( val1 ) {
				case 1: val2 = ELE_FIRE; break;
				case 2: val2 = ELE_EARTH; break;
				case 3: val2 = ELE_WIND; break;
				case 4: val2 = ELE_WATER; break;
			}
			break;
		case SC_ELECTRICSHOCKER:
		case SC_CRYSTALIZE:
			val4 = tick / 1000;
			if( val4 < 1 )
				val4 = 1;
			tick_time = 1000; // [GodLesZ] tick time
			break;
		case SC_MEIKYOUSISUI:
			val2 = val1 * 2; // % HP each sec
			val3 = val1; // % SP each sec
			val4 = tick / 1000;
			if( val4 < 1 )
				val4 = 1;
			tick_time = 1000;
			break;
		case SC_CAMOUFLAGE:
			val4 = tick/1000;
			tick_time = 1000; // [GodLesZ] tick time
			break;
		case SC_WUGDASH:
		{
			//Store time at which you started running.
			t_tick currenttick = gettick();
			// Note: this int64 value is stored in two separate int32 variables (FIXME)
			val3 = (int)(currenttick&0x00000000ffffffffLL);
			val4 = (int)((currenttick&0xffffffff00000000LL)>>32);
			tick = INFINITE_TICK;
			break;
		}
		case SC__SHADOWFORM:
			{
				map_session_data * s_sd = map_id2sd(val2);
				if( s_sd )
					s_sd->shadowform_id = bl->id;
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
			}
			break;
		case SC__STRIPACCESSORY:
			if (!sd)
				val2 = 20;
			break;
		case SC__INVISIBILITY:
			val2 = 50 - 10 * val1; // ASPD
			val3 = 20 * val1; // CRITICAL
			val4 = tick / 1000;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			tick_time = 1000; // [GodLesZ] tick time
			break;
		case SC__ENERVATION:
			val2 = 20 + 10 * val1; // ATK Reduction
			if (sd) {
				pc_delspiritball(sd,sd->spiritball,0);
				pc_delspiritcharm(sd,sd->spiritcharm,sd->spiritcharm_type);
			}
			break;
		case SC__GROOMY:
			val2 = 20 + 10 * val1; // ASPD
			val3 = 20 * val1; // HIT
			if( sd ) { // Removes Animals
				if( pc_isriding(sd) ) pc_setriding(sd, 0);
				if( pc_isridingdragon(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_DRAGON);
				if( pc_iswug(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_WUG);
				if( pc_isridingwug(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_WUGRIDER);
				if( pc_isfalcon(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_FALCON);
				if( sd->status.pet_id > 0 ) pet_return_egg(sd, sd->pd);
				if( hom_is_active(sd->hd) ) hom_vaporize(sd, HOM_ST_ACTIVE);
				//if( sd->md ) mercenary_delete(sd->md,3); // Are Mercenaries removed? [aleos]
			}
			break;
		case SC__LAZINESS:
			val2 = 10 + 10 * val1; // Cast Increase
			val3 = 10 * val1; // Flee Reduction
			break;
		case SC__UNLUCKY:
		{
			sc_type rand_eff; 
			switch(rnd() % 3) {
				case 1: rand_eff = SC_BLIND; break;
				case 2: rand_eff = SC_SILENCE; break;
				default: rand_eff = SC_POISON; break;
			}
			val2 = 10 * val1; // Crit and Flee2 Reduction
			status_change_start(src,bl,rand_eff,10000,val1,0,(rand_eff == SC_POISON ? src->id : 0),0,tick,SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			break;
		}
		case SC__WEAKNESS:
			val2 = 10 * val1;
			// Bypasses coating protection and MADO
			sc_start(src,bl,SC_STRIPWEAPON,100,val1,tick);
			sc_start(src,bl,SC_STRIPSHIELD,100,val1,tick);
			break;
		case SC_GN_CARTBOOST:
			if( val1 < 3 )
				val2 = 50;
			else if( val1 > 2 && val1 < 5 )
				val2 = 75;
			else
				val2 = 100;
			break;
		case SC_PROPERTYWALK:
			val3 = 0;
			break;
		case SC_STRIKING:
			// val2 = watk bonus already calc
			val3 = 6 - val1;// spcost = 6 - level (lvl1:5 ... lvl 5: 1)
			val4 = tick / 1000;
			tick_time = 1000; // [GodLesZ] tick time
			break;
		case SC_WARMER:
			val4 = tick / 3000;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			tick_time = 3000;
			break;
		case SC_HELLS_PLANT:
			tick_time = status_get_sc_interval(type);
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_SWINGDANCE:
			val3 = 3 * val1 + val2; // Walk speed and aspd reduction.
			break;
		case SC_SYMPHONYOFLOVER:
			val3 = 2 * val1 + val2 + (sd?sd->status.job_level:50) / 4; // MDEF Increase
			break;
		case SC_MOONLITSERENADE: // MATK Increase
		case SC_RUSHWINDMILL: // ATK Increase
			val3 = 4 + val1 * 3 + val2 + (sd?sd->status.job_level:50) / 5;
			break;
		case SC_ECHOSONG:
			val3 = 6 * val1 + val2 + (sd?sd->status.job_level:50) / 4; // DEF Increase
			break;
		case SC_HARMONIZE:
			val2 = 5 + 5 * val1;
			break;
		case SC_VOICEOFSIREN:
			val4 = tick / 2000;
			tick_time = 2000; // [GodLesZ] tick time
			break;
		case SC_DEEPSLEEP:
			val4 = tick / 2000;
			tick_time = 2000; // [GodLesZ] tick time
			break;
		case SC_SIRCLEOFNATURE:
			val2 = 50 * val1; // HP recovery rate
			break;
		case SC_SONGOFMANA:
			status_heal(bl, 0, status->max_sp * (val1 <= 2 ? 10 : val1 <= 4 ? 15 : 20) / 100, 1);
			val3 = 50 * val1;
			break;
		case SC_SATURDAYNIGHTFEVER:
			if (!val4) val4 = skill_get_time2(scdb->skill_id,val1);
			if (!val4) val4 = 3000;
			val3 = tick/val4;
			tick_time = val4; // [GodLesZ] tick time
			break;
		case SC_GLOOMYDAY:
			val2 = 20 + 5 * val1; // Flee reduction.
			val3 = 15 + 5 * val1; // ASPD reduction.
			if( sd && rnd()%100 < val1 ) { // (Skill Lv) %
				val4 = 1; // Reduce walk speed by half.
				if( pc_isriding(sd) ) pc_setriding(sd, 0);
				if( pc_isridingdragon(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_DRAGON);
			}
			break;
		case SC_GLOOMYDAY_SK:
			// Random number between [15 ~ (Voice Lesson Skill Level x 5) + (Skill Level x 10)] %.
			val2 = 15 + rnd()%( (sd?pc_checkskill(sd, WM_LESSON)*5:0) + val1*10 );
			break;
		case SC_SITDOWN_FORCE:
		case SC_BANANA_BOMB_SITDOWN:
			if( sd && !pc_issit(sd) ) {
				pc_setsit(sd);
				skill_sit(sd, true);
				clif_sitting(*bl);
			}
			break;
		case SC_DANCEWITHWUG:
			val3 = 5 * val1; // ASPD Increase
			val4 = 20 + 10 * val1; // Fixed Cast Time Reduction
			break;
		case SC_LERADSDEW:
			val3 = 2 + 3 * val1 + min(3 * val2, 25); // MaxHP Increase
			break;
		case SC_MELODYOFSINK:
			val2 = 10 * val1; // INT Reduction.
			val3 = 2 + 2 * val1; // MaxSP reduction
			break;
		case SC_BEYONDOFWARCRY:
			val2 = 10 + 10 * val1; // STR Reduction
			val3 = 4 * val1; // MaxHP Reduction
			break;
		case SC_UNLIMITEDHUMMINGVOICE:
			val3 = 4 * val1 + min(3 * val2, 15); // !TODO: What's the Lesson bonus?
			break;
		case SC_REFLECTDAMAGE:
			val2 = 10 * val1; // Reflect reduction amount
			val4 = tick/1000; // Number of SP cycles (duration)
			tick_time = 1000; // [GodLesZ] tick time
			break;
		case SC_FORCEOFVANGUARD:
			val2 = 8 + 12 * val1; // Chance
			val3 = 5 + 2 * val1; // Max rage counters
			tick = INFINITE_TICK; // Endless duration in the client
			tick_time = 10000; // [GodLesZ] tick time
			break;
		case SC_EXEEDBREAK:
			val2 = 150 * val1;
			if (sd) { // Players
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON)
					val2 += 15 * sd->status.job_level + sd->inventory_data[index]->weight / 10 * sd->inventory_data[index]->weapon_level * status_get_lv(bl) / 100;
			} else // Monster
				val2 += 750;
			break;
		case SC_PRESTIGE:
			val2 = (status->int_ + status->luk) * val1 / 20 * status_get_lv(bl) / 200 + val1;	// Chance to evade magic damage.
			val3 = ((val1 * 15) + (10 * (sd?pc_checkskill(sd,CR_DEFENDER):skill_get_max(CR_DEFENDER)))) * status_get_lv(bl) / 100; // Defence added
			break;
		case SC_SHIELDSPELL_HP:
			val2 = 3; // 3% HP every 3 seconds
			tick_time = status_get_sc_interval(type);
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_SHIELDSPELL_SP:
			val2 = 3; // 3% SP every 5 seconds
			tick_time = status_get_sc_interval(type);
			val4 = tick - tick_time; // Remaining time
			break;
		case SC_SHIELDSPELL_ATK:
			val2 = 150; // WATK/MATK bonus
			break;
		case SC_BANDING:
			val2 = (sd ? skill_banding_count(sd) : 1);
			tick_time = 5000; // [GodLesZ] tick time
			break;
		case SC_MAGNETICFIELD:
			tick_time = 1000; // [GodLesZ] tick time
			val4 = tick / tick_time;
			break;
		case SC_INSPIRATION:
			val2 = 40 * val1; // ATK/MATK
			val3 = 6 * val1; //All stat bonus
			val4 = tick / 5000;
			tick_time = 5000; // [GodLesZ] tick time
			status_change_clear_buffs(bl, SCCB_DEBUFFS); // Remove debuffs
			break;
		case SC_CRESCENTELBOW:
			val2 = (sd?sd->status.job_level:50) / 2 + (50 + 5 * val1);
			break;
		case SC_LIGHTNINGWALK: // [(Job Level / 2) + (40 + 5 * Skill Level)] %
			val1 = (sd?sd->status.job_level:2)/2 + 40 + 5 * val1;
			break;
		case SC_GT_ENERGYGAIN:
			val2 = 10 + 5 * val1; // Sphere gain chance.
			break;
		case SC_GT_CHANGE:
			// Take note there is no def increase as skill desc says. [malufett]
			val2 = val1 * 8; // ATK increase
			val3 = status->agi * val1 / 60; // ASPD increase: [(Target AGI x Skill Level) / 60] %
			break;
		case SC_GT_REVITALIZE:
			// Take note there is no vit, aspd, speed increase as skill desc says. [malufett]
			val2 = 2 * val1; // MaxHP: [(Skill Level * 2)]%
			val3 = val1 * 30 + 50; // Natural HP recovery increase: [(Skill Level x 30) + 50] %
			// The stat def is not shown in the status window and it is processed differently
			val4 = val1 * 20; // STAT DEF increase
			break;
		case SC_PYROTECHNIC_OPTION:
			val2 = 60; // Eatk Renewal (Atk2)
			break;
		case SC_HEATER_OPTION:
			val2 = 120; // Eatk Renewal (Atk2)
			val3 = ELE_FIRE; // Change into fire element.
			break;
		case SC_TROPIC_OPTION:
			val2 = 180; // Eatk Renewal (Atk2)
			val3 = MG_FIREBOLT;
			break;
		case SC_AQUAPLAY_OPTION:
			val2 = 40;
			break;
		case SC_COOLER_OPTION:
			val2 = 80;
			val3 = ELE_WATER; // Change into water element.
			break;
		case SC_CHILLY_AIR_OPTION:
			val2 = 120; // Matk. Renewal (Matk1)
			val3 = MG_COLDBOLT;
			break;
		case SC_WIND_STEP_OPTION:
			val2 = 50; // % Increase speed and flee.
			break;
		case SC_BLAST_OPTION:
			val2 = 20;
			val3 = ELE_WIND; // Change into wind element.
			break;
		case SC_WILD_STORM_OPTION:
			val2 = MG_LIGHTNINGBOLT;
			break;
		case SC_PETROLOGY_OPTION:
			val2 = 5; //HP Rate bonus
			val3 = 50;
			break;
		case SC_SOLID_SKIN_OPTION:
			val2 = 33; //% Increase DEF
			break;
		case SC_CURSED_SOIL_OPTION:
			val2 = 10; //HP rate bonus
			val3 = ELE_EARTH; // Change into earth element.
			break;
		case SC_UPHEAVAL_OPTION:
			val2 = 15; //HP rate bonus
			val3 = WZ_EARTHSPIKE;
			break;
		case SC_CIRCLE_OF_FIRE_OPTION:
			val2 = 300;
			break;
		case SC_WATER_SCREEN_OPTION:
			tick_time = 10000;
			break;
		case SC_FIRE_CLOAK_OPTION:
		case SC_WATER_DROP_OPTION:
		case SC_WIND_CURTAIN_OPTION:
		case SC_STONE_SHIELD_OPTION:
			val2 = 100; // Elemental modifier.
			break;
		case SC_TROPIC:
		case SC_CHILLY_AIR:
		case SC_WILD_STORM:
		case SC_UPHEAVAL:
			val2 += 10;
			[[fallthrough]];
		case SC_HEATER:
		case SC_COOLER:
		case SC_BLAST:
		case SC_CURSED_SOIL:
			val2 += 10;
			[[fallthrough]];
		case SC_PYROTECHNIC:
		case SC_AQUAPLAY:
		case SC_GUST:
		case SC_PETROLOGY:
			val2 += 5;
			val3 += 9000;
			[[fallthrough]];
		case SC_CIRCLE_OF_FIRE:
		case SC_FIRE_CLOAK:
		case SC_WATER_DROP:
		case SC_WATER_SCREEN:
		case SC_WIND_CURTAIN:
		case SC_WIND_STEP:
		case SC_STONE_SHIELD:
		case SC_SOLID_SKIN:
			val2 += 5;
			val3 += 1000;
			tick_time = val3; // [GodLesZ] tick time
			break;
		case SC_WATER_BARRIER:
			val2 = 30; // Reductions. Atk2 and Flee1
			break;
		case SC_ZEPHYR:
			val2 = 25; // Flee.
			break;
		case SC_TIDAL_WEAPON:
			val2 = 20; // Increase Elemental's attack.
			break;
		case SC_ROCK_CRUSHER:
		case SC_ROCK_CRUSHER_ATK:
		case SC_POWER_OF_GAIA:
			val2 = 33; //Def rate bonus/Speed rate reduction
			val3 = 20; //HP rate bonus
			break;
		case SC_TEARGAS:
			val2 = status_get_max_hp(bl) * 5 / 100; // Drain 5% HP
			val4 = tick / 2000;
			tick_time = 2000;
			break;
		case SC_TEARGAS_SOB:
			val4 = tick / 3000;
			tick_time = 3000;
			break;
		case SC_STOMACHACHE:
			val2 = 8; // SP consume.
			val4 = tick / 10000;
			tick_time = 10000; // [GodLesZ] tick time
			break;
		case SC_PROMOTE_HEALTH_RESERCH:
			//val1: 1 = Regular Potion, 2 = Thrown Potion
			//val2: 1 = Small Potion, 2 = Medium Potion, 3 = Large Potion
			//val3: MaxHP Increase By Fixed Amount
			if (val1 == 1) // If potion was normally used, take the user's BaseLv
				val3 = 1000 * val2 - 500 + status_get_lv(bl) * 10 / 3;
			else if (val1 == 2) // If potion was thrown at someone, take the thrower's BaseLv
				val3 = 1000 * val2 - 500 + status_get_lv(src) * 10 / 3;
			if (val3 <= 0) // Prevents a negeative value from happening
				val3 = 0;
			break;
		case SC_ENERGY_DRINK_RESERCH:
			//val1: 1 = Regular Potion, 2 = Thrown Potion
			//val2: 1 = Small Potion, 2 = Medium Potion, 3 = Large Potion
			//val3: MaxSP Increase By Percentage Amount
			if (val1 == 1) // If potion was normally used, take the user's BaseLv
				val3 = status_get_lv(bl) / 10 + 5 * val2 - 10;
			else if (val1 == 2) // If potion was thrown at someone, take the thrower's BaseLv
				val3 = status_get_lv(src) / 10 + 5 * val2 - 10;
			if (val3 <= 0) // Prevents a negeative value from happening
				val3 = 0;
			break;
		case SC_KYOUGAKU:
			val2 = 2*val1 + rnd()%val1;
			clif_status_change(bl,EFST_ACTIVE_MONSTER_TRANSFORM,1,0,1002,0,0);
			break;
		case SC_KAGEMUSYA:
			val2 = 20; // Damage increase bonus
			val3 = val1 * 2;
			tick_time = 1000;
			val4 = tick / tick_time;
			break;
		case SC_ZANGETSU:
			if( status_get_hp(bl) % 2 == 0 )
				val2 = (status_get_lv(bl) / 3) + (20 * val1); //+Watk
			else
				val2 -= (status_get_lv(bl) / 3) + (30 * val1); //-Watk

			if( status_get_sp(bl) % 2 == 0 )
				val3 = (status_get_lv(bl) / 3) + (20 * val1); //+Matk
			else
				val3 -= (status_get_lv(bl) / 3) + (30 * val1); //-Matk
			break;
		case SC_GENSOU:
			{
				int hp = status_get_hp(bl), lv = 5;
				short per = 100 / (status_get_max_hp(bl) / hp);

				if( per <= 15 )
					lv = 1;
				else if( per <= 30 )
					lv = 2;
				else if( per <= 50 )
					lv = 3;
				else if( per <= 75 )
					lv = 4;
				if( hp % 2 == 0)
					status_heal(bl, hp * (6-lv) * 4 / 100, status_get_sp(bl) * (6-lv) * 3 / 100, 1);
				else
					status_zap(bl, hp * (lv*4) / 100, status_get_sp(bl) * (lv*3) / 100);
			}
			break;
		case SC_ANGRIFFS_MODUS:
			val2 = 50 + 20 * val1; // atk bonus
			val3 = 25 + 10 * val1; // Flee reduction.
			val4 = tick/1000; // hp/sp reduction timer
			tick_time = 1000;
			break;
		case SC_GOLDENE_FERSE:
			val2 = 10 + 10*val1; // flee bonus
			val3 = 6 + 4 * val1; // Aspd Bonus
			val4 = 2 + 2 * val1; // Chance of holy attack
			break;
		case SC_STONE_WALL:
			val2 = 100 * val1; // DEF bonus
			val3 = 30 * val1; // MDEF bonus
			break;
		case SC_OVERED_BOOST:
			val2 = 400 + 40 * val1; // flee bonus
			val3 = 180 + 2 * val1; // aspd bonus
			val4 = 50; // def reduc %
			break;
		case SC_GRANITIC_ARMOR:
			val2 = 2*val1; // dmg reduction
			val3 = 6*val1; // dmg taken on status end (6%:12%:18%:24%:30%)
			val4 = 5*val1; // unknow formula
			break;
		case SC_MAGMA_FLOW:
			val2 = 3*val1; // Activation chance
			break;
		case SC_PYROCLASTIC:
			val2 += 100 + 10*val1; // atk bonus // !TODO: Confirm formula
			break;
		case SC_TEMPERING:
			val2 += 5 + val1; // patk bonus
			break;
		case SC_GOLDENE_TONE:
			val2 += 3 * val1; // res/mres bonus
			break;
		case SC_PARALYSIS: // [Lighta] need real info
			val2 = 2*val1; // def reduction
			val3 = 500*val1; // varcast augmentation
			break;
		case SC_TOXIN_OF_MANDARA:
			val2 = 15*val1; // res reduction
			break;
		case SC_LIGHT_OF_REGENE: // Yommy leak need confirm
			val2 = 20 * val1; // hp reco on death %
			break;
		case SC_PAIN_KILLER: // Yommy leak need confirm
			val2 = min((( 200 * val1 ) * status_get_lv(src)) / 150, 1000); // dmg reduction linear. upto a maximum of 1000 [iRO Wiki]
			if(sc->getSCE(SC_PARALYSIS))
				sc_start(src,bl, SC_ENDURE, 100, val1, tick); // Start endure for same duration
			break;
		case SC_STYLE_CHANGE:
			tick = INFINITE_TICK; // Infinite duration
			break;
		case SC_CBC:
			val3 = 10; // Drain sp % dmg
			val4 = tick/1000; // dmg each sec
			tick = 1000;
			break;
		case SC_EQC:
			val2 = 5 * val1; // def % reduc
			val3 = 2 * val1; // HP drain %
			break;
		case SC_ASH:
			val2 = 0; // hit % reduc
			val3 = 0; // def % reduc
			val4 = 0; // atk flee % reduc
			if (!status_bl_has_mode(bl,MD_STATUSIMMUNE)) {
				val2 = 50;
				if (status_get_race(bl) == RC_PLANT) // plant type
					val3 = 50;
				if (status_get_element(bl) == ELE_WATER) // defense water type
					val4 = 50;
			}
			break;
		case SC_FULL_THROTTLE:
			val2 = ( val1 == 1 ? 6 : 6 - val1 );
			val3 = 20; //+% AllStats
			tick_time = 1000;
			val4 = tick / tick_time;
			break;
		case SC_REBOUND:
			tick_time = 2000;
			val4 = tick / tick_time;
			clif_emotion(bl, ET_SWEAT);
			break;
		case SC_KINGS_GRACE:
			val2 = 3 + val1; //HP Recover rate
			tick_time = 1000;
			val4 = tick / tick_time;
			break;
		case SC_TELEKINESIS_INTENSE:
			val2 = 10 * val1; // sp consum / casttime reduc %
			val3 = 40 * val1; // magic dmg bonus
			break;
		case SC_OFFERTORIUM:
			val2 = 30 * val1; // heal power bonus
			val3 = 100 + 20 * val1; // sp cost inc
			break;
		case SC_FRIGG_SONG:
			val2 = 5 * val1; // maxhp bonus
			val3 = 80 + 20 * val1; // healing
			tick_time = 1000;
			val4 = tick / tick_time;
			break;
		case SC_FLASHCOMBO:
			val2 = 20 * val1 + 20; // atk bonus
			break;
		case SC_DARKCROW:
			val2 = 30 * val1; // ATK bonus
			break;
		case SC_UNLIMIT:
			val2 = 50 * val1;
			break;
		case SC_MONSTER_TRANSFORM:
		case SC_ACTIVE_MONSTER_TRANSFORM:
			if( !mobdb_checkid(val1) )
				val1 = MOBID_PORING; // Default poring
			break;
#ifndef RENEWAL
		case SC_APPLEIDUN:
		{
			map_session_data * s_sd = BL_CAST(BL_PC, src);

			val2 = (5 + 2 * val1) + (status_get_vit(src) / 10); //HP Rate: (5 + 2 * skill_lv) + (vit/10) + (BA_MUSICALLESSON level)
			if (s_sd)
				val2 += pc_checkskill(s_sd, BA_MUSICALLESSON) / 2;
			break;
		}
#endif
		case SC_EPICLESIS:
			val2 = 5 * val1; //HP rate bonus
			break;
		case SC_ILLUSIONDOPING:
			val2 = 50; // -Hit
			break;

		case SC_OVERHEAT:
		case SC_OVERHEAT_LIMITPOINT:
		case SC_STEALTHFIELD:
			tick_time = tick;
			tick = INFINITE_TICK;
			break;
		case SC_STEALTHFIELD_MASTER:
			tick_time = val3 = 2000 + 1000 * val1;
			val4 = tick / tick_time;
			break;
		case SC_VACUUM_EXTREME:
			// Suck target at n second, only if the n second is lower than the duration
			// Does not suck targets on no-knockback maps
			if (val4 < tick && unit_blown_immune(bl, 0x9) == UB_KNOCKABLE) {
				tick_time = val4;
				val4 = tick - tick_time;
			} else
				val4 = 0;
			break;
		case SC_FIRE_INSIGNIA:
		case SC_WATER_INSIGNIA:
		case SC_WIND_INSIGNIA:
		case SC_EARTH_INSIGNIA:
			tick_time = 5000;
			val4 = tick / tick_time;
			break;
		case SC_NEUTRALBARRIER:
			val2 = 10 + val1 * 5; // Def/Mdef
			tick = INFINITE_TICK;
			break;
		case SC_MAGIC_POISON:
			val2 = 50; // Attribute Reduction
			break;

		/* Rebellion */
		case SC_B_TRAP:
			val2 = src->id;
			val3 = val1 * 25; // -movespeed TODO: Figure out movespeed rate
			break;
		case SC_C_MARKER:
		case SC_BURNT:
			// val1 = skill_lv
			// val2 = src_id
			val3 = 10; // -10 flee
			//Start timer to send mark on mini map
			val4 = tick/1000;
			tick_time = 1000; // Sends every 1 seconds
			break;
		case SC_H_MINE:
			val2 = src->id;
			break;
		case SC_HEAT_BARREL:
			{
				uint8 n = 10;
				if (sd)
					n = (uint8)sd->spiritball_old;

				//kRO Update 2016-05-25
				val2 = n * 5; // -fixed casttime
				val3 = (6 + val1 * 2) * n; // ATK
				val4 = 25 + val1 * 5; // -hit
			}
			break;
		case SC_P_ALTER:
			{
				uint8 n = 10;
				if (sd)
					n = (uint8)sd->spiritball_old;
				val2 = 10 * n; // +atk
				val3 = (status->max_hp * (val1 * 5) / 100); // Barrier HP
			}
			break;
		case SC_E_CHAIN:
			val2 = 10;
			if (sd)
				val2 = sd->spiritball_old;
			break;
		case SC_ANTI_M_BLAST:
			val2 = val1 * 10;
			break;
		case SC_CATNIPPOWDER:
			val2 = 50; // WATK%, MATK%
			val3 = 25 * val1; // Move speed reduction
			if (bl->type == BL_PC && pc_checkskill(sd, SU_SPIRITOFLAND))
				val4 = status_get_lv(src) / 12;
			break;
		case SC_BITESCAR: {
				const struct status_data *b_status = status_get_base_status(src); // Base Status

				val2 = (status_get_max_hp(bl) * (val1 + (b_status->dex / 25))) / status_get_max_hp(bl); // MHP% damage
				tick_time = 1000;
				val4 = tick / tick_time;
			}
			break;
		case SC_ARCLOUSEDASH:
			val2 = 15 + 5 * val1; // AGI
			val3 = 25; // Move speed increase
			if (sd && (sd->class_&MAPID_BASEMASK) == MAPID_SUMMONER)
				val4 = 10; // Ranged ATK increase
			break;
		case SC_SHRIMP:
			val2 = 10; // BATK%, MATK%
			break;
		case SC_FRESHSHRIMP: {
				int min = 0, max = 0;

#ifdef RENEWAL
				min = status_base_matk_min(src, status, status_get_lv(src));
				max = status_base_matk_max(src, status, status_get_lv(src));
				if (status->rhw.matk > 0) {
					int wMatk, variance;

					wMatk = status->rhw.matk;
					variance = wMatk * status->rhw.wlv / 10;
					min += wMatk - variance;
					max += wMatk + variance;
				}
#endif

				if (sd && sd->right_weapon.overrefine > 0) {
					min++;
					max += sd->right_weapon.overrefine - 1;
				}

				val2 += min + 178; // Heal
				if (max > min)
					val2 += rnd() % (max - min); // Heal

				if (sd) {
					if (pc_checkskill(sd, SU_POWEROFSEA) > 0) {
						val2 += val2 * 10 / 100;
						if (pc_checkskill_summoner(sd, SUMMONER_POWER_SEA) >= 20)
							val2 += val2 * 20 / 100;
					}
					if (pc_checkskill(sd, SU_SPIRITOFSEA) > 0)
						val2 *= 2; // Doubles HP
				}
				tick_time = 10000 - ((val1 - 1) * 1000);
				val4 = tick / tick_time;
			}
			break;
		case SC_TUNAPARTY:
			val2 = (status->max_hp * (val1 * 10) / 100); // Max HP% to absorb
			if (sd && pc_checkskill(sd, SU_SPIRITOFSEA))
				val2 *= 2; // Double the shield life
			break;
		case SC_HISS:
			val2 = 50; // Perfect Dodge
			sc_start(src, bl, SC_DORAM_WALKSPEED, 100, 50, skill_get_time2(SU_HISS, val1));
			break;
		case SC_GROOMING:
			val2 = 100; // Flee
			break;
		case SC_CHATTERING:
			val2 = 100; // eATK, eMATK
			sc_start(src, bl, SC_DORAM_WALKSPEED, 100, 50, skill_get_time2(SU_CHATTERING, val1));
			break;
		case SC_SWORDCLAN:
		case SC_ARCWANDCLAN:
		case SC_GOLDENMACECLAN:
		case SC_CROSSBOWCLAN:
		case SC_JUMPINGCLAN:
			tick = INFINITE_TICK;
			status_change_start(src,bl,SC_CLAN_INFO,10000,0,val2,0,0,INFINITE_TICK,flag);
			break;
		case SC_DORAM_BUF_01:
		case SC_DORAM_BUF_02:
			tick_time = 10000; // every 10 seconds
			if( (val4 = tick/tick_time) < 1 )
				val4 = 1;
			break;

		case SC_GLASTHEIM_ATK:
			val1 = 100; // Undead/Demon MDEF ignore rate
			break;
		case SC_GLASTHEIM_HEAL:
			val1 = 100; // Heal Power rate bonus
			val2 = 50; // Received heal rate bonus
			break;
		case SC_GLASTHEIM_HIDDEN:
			val1 = 90; // Damage rate reduction bonus
			break;
		case SC_GLASTHEIM_STATE:
			val1 = 20; // All-stat bonus
			break;
		case SC_GLASTHEIM_ITEMDEF:
			val1 = 200; // DEF bonus
			val2 = 50; // MDEF bonus
			break;
		case SC_GLASTHEIM_HPSP:
			val1 = 10000; // HP bonus
			val2 = 1000; // SP bonus
			break;
		case SC_ANCILLA:
			val1 = 15; // Heal Power rate bonus
			val2 = 30; // SP Recovery rate bonus
			break;
		case SC_HELPANGEL:
			tick_time = 1000;
			val4 = tick / tick_time;
			break;
		case SC_EMERGENCY_MOVE:
			val2 = 25; // Movement speed increase
			break;

		case SC_SUNSTANCE:
			val2 = 2 + val1; // ATK Increase
			tick = INFINITE_TICK;
			break;
		case SC_LUNARSTANCE:
			val2 = 2 + val1; // MaxHP Increase
			tick = INFINITE_TICK;
			break;
		case SC_STARSTANCE:
			val2 = 4 + 2 * val1; // ASPD Increase
			tick = INFINITE_TICK;
			break;
		case SC_DIMENSION1:
		case SC_DIMENSION2:
			if (sd)
				pc_addspiritball(sd, skill_get_time2(SJ_BOOKOFDIMENSION, 1), 2);
			break;
		case SC_UNIVERSESTANCE:
			val2 = 2 + val1; // All Stats Increase
			tick = INFINITE_TICK;
			break;
		case SC_NEWMOON:
			val2 = 7; // Number of Regular Attacks Until Reveal
			tick_time = 1000;
			val4 = tick / tick_time;
			break;
		case SC_FALLINGSTAR:
			val2 = 8 + 2 * (1 + val1) / 2; // Autocast Chance
			if (val1 >= 7)
				val2 += 1; // Make it 15% at level 7.
			break;
		case SC_CREATINGSTAR:
			tick_time = 500;
			val4 = tick / tick_time;
			break;
		case SC_LIGHTOFSUN:
		case SC_LIGHTOFMOON:
		case SC_LIGHTOFSTAR:
			val2 = 5 * val1; // Skill Damage Increase.
			break;
		case SC_SOULGOLEM:
			val2 = 60 * val1; // DEF Increase
			val3 = 15 + 5 * val1; // MDEF Increase
			break;
		case SC_SOULSHADOW:
			val2 = (1 + val1) / 2; // ASPD Increase
			val3 = 10 + 2 * val1; // CRIT Increase
			break;
		case SC_SOULFALCON:
			val2 = 10 * val1; // WATK Increase
			val3 = 10; // HIT Increase
			if (val1 >= 3)
				val3 += 3;
			else if (val1 >= 5)
				val3 += 5;
			break;
		case SC_SOULFAIRY:
			val2 = 10 * val1; // MATK Increase
			val3 = 5; // Variable Cast Time Reduction
			if (val1 >= 3)
				val3 += 2;
			else if (val1 >= 5)
				val3 += 5;
			break;
		case SC_SOULUNITY:
			tick_time = 3000;
			val4 = tick / tick_time;
			break;
		case SC_SOULDIVISION:
			val2 = 10 * val1; // Skill Aftercast Increase
			break;
		case SC_SOULREAPER:
			val2 = 10 + 5 * val1; // Chance of Getting A Soul Sphere.
			break;
		case SC_SOULCOLLECT:
			val2 = 5 + 3 * val2; // Max Soul Sphere's.
			val3 = tick > 0 ? tick : 60000;
			tick_time = tick;
			tick = INFINITE_TICK;
			break;
		case SC_SP_SHA:
			val2 = 50; // Move speed reduction
			break;
		case SC_SERVANTWEAPON:
			if( sd ){
				// Generate 5 servants on start
				pc_addservantball( *sd, MAX_SERVANTBALL );
			}
			tick_time = skill_get_time2(DK_SERVANTWEAPON,val1); // Servant Regen Interval
			if (tick_time < 500)
				tick_time = 500; // Avoid being brought down to 0.
			val4 = tick - tick_time; // Remaining Time
			break;
		case SC_RELIEVE_ON:
			val2 = min(10*val1, 99); // % damage received reduced from 10 * skill lvl up to 99%
			break;
		case SC_VIGOR:
			val2 = 100 - 10 * (val1 - 1); // HP consumption with each attack is reduced by skill lvl
			val2 = max(val2, 0);
			break;
		case SC_POWERFUL_FAITH:
			val2 = 5 + 5 * val1;// ATK Increase
			val3 = 5 + 2 * val1;// PAtk Increase
			break;
		case SC_FIRM_FAITH:
			val2 = 2 * val1;// MaxHP Increase
			val3 = 8 * val1;// Res Increase
			break;
		case SC_SINCERE_FAITH:
			val2 = (1 + val1) / 2;// ASPD Increase
			val3 = 4 * val1;// Perfect Hit Increase
			break;
		case SC_GUARD_STANCE:
			val2 = 50 + 50 * val1;// DEF Increase
			val3 = 50 * val1;// ATK Decrease
			tick = INFINITE_TICK;
			break;
		case SC_GUARDIAN_S:
			val2 = ( status->max_hp * 30 / 100 ) * ( 25 * val1 ) / 100 + 15 * status->sta; // Barrier HP
			break;
		case SC_REBOUND_S:
			val2 = 10 * val1;// Reduced Damage From Devotion
			if (val2 > 99)
				val2 = 99;// Lets not let it reduce above 99.
			break;
		case SC_ATTACK_STANCE:
			val2 = 40 * val1;// DEF Decrease
			val3 = 3 * val1; // P.ATK/S.MATK Increase
			tick = INFINITE_TICK;
			break;
		case SC_HOLY_S:
			val2 = 5 + 2 * val1;// Damage Reduction / Holy Damage Increase
			break;
		case SC_MEDIALE:
			val2 = 2 * val1;// Heal Rate
			val4 = tick / 2000;
			tick_time = 2000;
			break;
		case SC_A_VITA:
		case SC_A_TELUM:
			val2 = 5 * val1;// Res/MRes Pierce Percentage
			break;
		case SC_PRE_ACIES:
			val2 = 2 * val1;// CRate Increase
			break;
		case SC_COMPETENTIA:
			val2 = 10 * val1;// PAtk/SMatk Increase - Unconfirmed if this is official formula but its 50 at Lv 5. [Rytech]
			break;
		case SC_RELIGIO:
		case SC_BENEDICTUM:
			val2 = 2 * val1;// Trait Stats Increase
			break;
		case SC_DANCING_KNIFE:
			val4 = tick / 300;
			tick_time = 300;
			break;
		case SC_POTENT_VENOM:
			val2 = 2 * val1;// Res Pierce Percentage
			break;
		case SC_A_MACHINE:
			val4 = tick / 1000;
			tick_time = 1000;
			break;
		case SC_D_MACHINE:
			val2 = 200 + 50 * val1;// DEF Increase
			val3 = 20 * val1;// Res Increase
			break;
		case SC_SHADOW_STRIP:
			if (!sd)// Res/MRes on mobs only.
				val2 = 25;// Need official reduction amount.
			break;
		case SC_ABYSSFORCEWEAPON:
			if( sd ){
				// Generate 5 abyss spheres on start.
				pc_addabyssball( *sd, MAX_ABYSSBALL );
			}
			tick_time = skill_get_time2(ABC_FROM_THE_ABYSS, val1);// Abyss Regen Interval
			if (tick_time < 500)
				tick_time = 500;// Avoid being brought down to 0.
			val4 = tick - tick_time;// Remaining Time
			break;
		case SC_ABYSS_SLAYER:
			val2 = 10 + 2 * val1;// PAtk/SMatk Increase
			val3 = 100 + 20 * val1;// Hit Increase
			break;
		case SC_WINDSIGN:
			val2 = 8 + 6 * val1;// Chance to gain AP on attack.
			if (val1 == 5)// Its 40% on level 5.
				val2 += 2;
			break;
		case SC_CALAMITYGALE:// Unlimit runs along with this.
			sc_start(bl, bl, SC_UNLIMIT, 100, 5, skill_get_time(RA_UNLIMIT, 5));
			break;
		case SC_GEF_NOCTURN:// MRes Reduction. Official formula unknown.
		case SC_AIN_RHAPSODY:// Res Reduction. Official formula unknown.
			val2 = 10 * val1;// Res/MRes Decrease
			if (val3&2)// Bonus if partner is found in party.
				val2 *= 2;
			break;
		case SC_MUSICAL_INTERLUDE:
			val2 = 5 + 5 * val1;// Res Increase
			if (val3&2)// Bonus if partner is found in party.
				val2 *= 2;
			break;
		case SC_JAWAII_SERENADE:
			val2 = 3 * val1;// SMatk Increase
			if (val3 & 2)// Bonus if partner is found in party.
				val2 *= 2;
			break;
		case SC_PRON_MARCH:
			val2 = 3 * val1;// PAtk Increase
			if (val3 & 2)// Bonus if partner is found in party.
				val2 *= 2;
			break;
		case SC_SPELL_ENCHANTING:
			val2 = 4 * val1;// SMatk Increase
			break;
		case SC_FLAMETECHNIC:
		case SC_FLAMEARMOR:
		case SC_COLD_FORCE:
		case SC_CRYSTAL_ARMOR:
		case SC_GRACE_BREEZE:
		case SC_EYES_OF_STORM:
		case SC_EARTH_CARE:
		case SC_STRONG_PROTECTION:
		case SC_DEEP_POISONING:
		case SC_POISON_SHIELD:
			val2 += 10;
			val3 += 10000;
			tick_time = val3;
			break;
		case SC_FLAMETECHNIC_OPTION:
			val3 = ELE_FIRE;
			break;
		case SC_COLD_FORCE_OPTION:
			val3 = ELE_WATER;
			break;
		case SC_GRACE_BREEZE_OPTION:
			val3 = ELE_WIND;
			break;
		case SC_EARTH_CARE_OPTION:
			val3 = ELE_EARTH;
			break;
		case SC_DEEP_POISONING_OPTION:
			val3 = ELE_POISON;
			break;
		case SC_SUB_WEAPONPROPERTY:
			if (sd && val3 == ASC_EDP) {
				uint16 poison_level = pc_checkskill(sd, GC_RESEARCHNEWPOISON);

				if (poison_level > 0) {
					tick += 30000; // Base of 30 seconds
					tick += poison_level * 15 * 1000; // Additional 15 seconds per level
				}
			}
			break;
		case SC_WEAPONBREAKER:
			val2 = val1 * 2 * 100; // Chance to break weapon
			break;
		case SC_INTENSIVE_AIM:
			tick = 500;
			break;
		case SC_HIDDEN_CARD:
			val2 = 3 * val1;
			val3 = 10 * val1;
			break;
		case SC_BUCHEDENOEL:
			val2 = 3;	// HP & SP restoration by 3%, Hit +3
			val3 = 7;	// Critical +7
			break;
		case SC_EP16_DEF:
			status_heal(bl, 1000, 0, 1);
			break;

		default:
			if (calc_flag.none() && scdb->skill_id == 0 && scdb->icon == EFST_BLANK && scdb->opt1 == OPT1_NONE && scdb->opt2 == OPT2_NONE && scdb->state.none() && scdb->flag.none() && scdb->endonstart.empty() && scdb->endreturn.empty() && scdb->fail.empty() && scdb->endonend.empty()) {
				// Status change with no calc, no icon, and no skill associated...?
				ShowWarning("status_change_start: Status %s (%d) is bare. Add the NoWarning flag to suppress this message.\n", script_get_constant_str("SC_", type), type);
				return 0;
			}
	} else // Special considerations when loading SC data.
		switch( type ) {
			case SC_WEDDING:
			case SC_XMAS:
			case SC_SUMMER:
			case SC_HANBOK:
			case SC_OKTOBERFEST:
			case SC_DRESSUP:
				if( !vd )
					break;
				clif_changelook(bl,LOOK_BASE,vd->class_);
				clif_changelook(bl,LOOK_WEAPON,0);
				clif_changelook(bl,LOOK_SHIELD,0);
				clif_changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
				clif_changelook(bl,LOOK_BODY2,0);
				break;
			case SC_STONE:
			case SC_STONEWAIT:
			case SC_POISON:
			case SC_DPOISON:
			case SC_BLEEDING:
			case SC_BURNING:
			case SC_TOXIN:
				tick_time = tick;
				tick = tick_time + max(val4, 0);
				break;
			case SC_DEATHHURT:
				if (val3 == 1)
					break;
				tick_time = tick;
				tick = tick_time + max(val4, 0);
				[[fallthrough]];
			case SC_MAGICMUSHROOM:
			case SC_PYREXIA:
			case SC_LEECHESEND:
				if (val3 == 0)
					break;
				tick_time = tick;
				tick = tick_time + max(val4, 0);
				break;
			case SC_SWORDCLAN:
			case SC_ARCWANDCLAN:
			case SC_GOLDENMACECLAN:
			case SC_CROSSBOWCLAN:
			case SC_JUMPINGCLAN:
			case SC_CLAN_INFO:
				// If the player still has a clan status, but was removed from his clan
				if( sd && sd->status.clan_id == 0 ){
					return 0;
				}
				break;
			case SC_SERVANTWEAPON:
			case SC_ABYSSFORCEWEAPON:
				tick_time = tick;
				tick = tick_time + max(val4, 0);
				break;
		}

	if (sd && current_equip_combo_pos > 0 && tick == INFINITE_TICK) {
		ShowWarning("sc_start: Item combo of item #%u contains an INFINITE_TICK duration. Skipping bonus.\n", sd->inventory_data[pc_checkequip(sd, current_equip_combo_pos)]->nameid);
		return 0;
	}

	/* [Ind] */
	if (scdb->flag[SCF_DISPLAYPC] || scdb->flag[SCF_DISPLAYNPC]) {
		int dval1 = 0, dval2 = 0, dval3 = 0;

		switch (type) {
			case SC_ALL_RIDING:
				dval1 = 1;
				break;
			case SC_CLAN_INFO:
				dval1 = val1;
				dval2 = val2;
				dval3 = val3;
				break;
			default: /* All others: just copy val1 */
				dval1 = val1;
				break;
		}
		status_display_add(bl,type,dval1,dval2,dval3);
	}

	//SC that force player to stand if is sitting
	if (scdb->flag[SCF_SETSTAND] && sd && pc_issit(sd))
		pc_setstand(sd, true);

	//SC that make stop attacking [LuzZza]
	if (scdb->flag[SCF_STOPATTACKING])
		unit_stop_attack(bl);

	//SC that make stop walking
	if (scdb->flag[SCF_STOPWALKING]) {
		switch (type) {
			case SC__MANHOLE:
				if (bl->type == BL_PC || !unit_blown_immune(bl,0x1))
					unit_stop_walking(bl,1);
				break;
			case SC_VACUUM_EXTREME:
				if (bl->type != BL_PC && unit_blown_immune(bl, 0x1) == UB_KNOCKABLE) {
					unit_stop_walking(bl,1);
					unit_stop_attack(bl);
				}
				break;
			case SC_FREEZE:
			case SC_STUN:
			case SC_STONE:
				if (sc->getSCE(SC_DANCING)) {
					unit_stop_walking(bl, 1);
					status_change_end(bl, SC_DANCING);
				}
				break;
			default:
				if (!unit_blown_immune(bl,0x1))
					unit_stop_walking(bl,1);
				break;
		}
	}

	//SC that make stop casting
	if (battle_config.sc_castcancel&bl->type && scdb->flag[SCF_STOPCASTING])
		unit_skillcastcancel(bl,0);

	if(scdb->opt1 != OPT1_NONE) sc->opt1 = scdb->opt1;
	sc->opt2 |= scdb->opt2;
	sc->opt3 |= scdb->opt3;
	sc->option |= scdb->look;

	std::bitset<SCF_MAX> opt_flag = scdb->flag;
	uint16 disable_opt_flag = false;

	switch(type) {
		case SC_DANCING:
			if ((val1&0xFFFF) == CG_MOONLIT)
				sc->opt3 |= OPT3_MOONLIT;
			break;
	}

	// On Aegis, when turning on a status change, first goes the option packet, then the sc packet.
	if (!disable_opt_flag && (opt_flag[SCF_SENDOPTION] || opt_flag[SCF_ONTOUCH] || opt_flag[SCF_UNITMOVE] || opt_flag[SCF_NONPLAYER] || opt_flag[SCF_SENDLOOK])) {
		clif_changeoption(bl);
		if(sd && opt_flag[SCF_SENDLOOK]) {
			clif_changelook(bl,LOOK_BASE,vd->class_);
			clif_changelook(bl,LOOK_WEAPON,0);
			clif_changelook(bl,LOOK_SHIELD,0);
			clif_changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
		}
	}

	if (calc_flag[SCB_DYE]) { // Reset DYE color
		if (vd && vd->cloth_color) {
			val4 = vd->cloth_color;
			clif_changelook(bl,LOOK_CLOTHES_COLOR,0);
		}
		calc_flag.reset(SCB_DYE);
	}

	/*if (calc_flag[SCB_BODY])// Might be needed in the future. [Rytech]
	{	//Reset body style
		if (vd && vd->body_style)
		{
			val4 = vd->body_style;
			clif_changelook(bl,LOOK_BODY2,0);
		}
		calc_flag.reset(SCB_BODY);
	}*/

	if (!(flag&SCSTART_NOICON) && !(flag&SCSTART_LOADED && scdb->flag[SCF_DISPLAYPC] || scdb->flag[SCF_DISPLAYNPC])) {
		int status_icon = scdb->icon;

#if PACKETVER < 20151104
		if (status_icon == EFST_WEAPONPROPERTY)
			status_icon = EFST_ATTACK_PROPERTY_NOTHING + val1; // Assign status icon for older clients
#endif

		clif_status_change(bl, status_icon, 1, tick, scdb->flag[SCF_SENDVAL1] ? val1 : 1, scdb->flag[SCF_SENDVAL2] ? val2 : 0, scdb->flag[SCF_SENDVAL3] ? val3 : 0);
	}

	// Used as temporary storage for scs with interval ticks, so that the actual duration is sent to the client first.
	if( tick_time )
		tick = tick_time;

	// Don't trust the previous sce assignment, in case the SC ended somewhere between there and here.
	if((sce=sc->getSCE(type))) { // reuse old sc
		if( sce->timer != INVALID_TIMER )
			delete_timer(sce->timer, status_change_timer);
		sc_isnew = false;
	} else { // New sc
		++(sc->count);
		sce = sc->createSCE(type);
	}
	sce->val1 = val1;
	sce->val2 = val2;
	sce->val3 = val3;
	sce->val4 = val4;
	if (tick >= 0)
		sce->timer = add_timer(gettick() + tick, status_change_timer, bl->id, type);
	else
		sce->timer = INVALID_TIMER; // Infinite duration

	if (calc_flag.any()) {
		if (sd != nullptr) {
			switch(type) {
				// Statuses that adjust HP/SP and heal after starting
				case SC_BERSERK:
				case SC_MERC_HPUP:
				case SC_MERC_SPUP:
				// Status needs to be updated immediately and not at the end of the damage
				case SC_EXTREMITYFIST:
					status_calc_bl_(bl, calc_flag, SCO_FORCE);
					break;
				default:
					if (!sd->state.connect_new)
						status_calc_bl_(bl, calc_flag);
					break;
			}
		} else
			status_calc_bl_(bl, calc_flag);
	}

	// Non-zero
	if (sc_isnew && scdb->state.any())
		status_calc_state(bl, sc, scdb->state, true);

	if (sd != nullptr && sd->pd != nullptr)
		pet_sc_check(sd, type); // Skotlex: Pet Status Effect Healing

	// 1st thing to execute when loading status
	switch (type) {
		case SC_BERSERK:
			if (!(sce->val2)) { // Don't heal if already set
				status_heal(bl, status->max_hp, 0, 1); // Do not use percent_heal as this healing must override BERSERK's block.
				status_set_sp(bl, 0, 0); // Damage all SP
			}
			sce->val2 = 5 * status->max_hp / 100;
			break;
		case SC_RUN:
			{
				struct unit_data *ud = unit_bl2ud(bl);

				if( ud )
					ud->state.running = unit_run(bl, nullptr, SC_RUN);
			}
			break;
		case SC_BOSSMAPINFO:
			if( sd == nullptr ){
				return 0;
			}else{
				mob_data* boss_md = map_id2boss( sce->val1 );

				if( boss_md == nullptr ){
					return 0;
				}

				// Not on same map anymore
				if( sd->bl.m != boss_md->bl.m ){
					return 0;
				// Boss is alive
				}else if( boss_md->bl.prev != nullptr ){
					sce->val2 = 0;
					clif_bossmapinfo( *sd, boss_md, BOSS_INFO_ALIVE_WITHMSG );
				// Boss is dead
				}else if( boss_md->spawn_timer != INVALID_TIMER ){
					sce->val2 = 1;
					clif_bossmapinfo( *sd, boss_md, BOSS_INFO_DEAD );
				}
			}
			break;
		case SC_FULL_THROTTLE:
		case SC_MERC_HPUP:
			status_percent_heal(bl, 100, 0); // Recover Full HP
			break;
		case SC_MERC_SPUP:
			status_percent_heal(bl, 0, 100); // Recover Full SP
			break;
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit_bl2ud(bl);

				if( ud )
					ud->state.running = unit_run(bl, sd, SC_WUGDASH);
			}
			break;
		case SC_COMBO:
			switch(sce->val1) {
			case TK_STORMKICK:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,*bl,TK_READYSTORM,1);
				break;
			case TK_DOWNKICK:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,*bl,TK_READYDOWN,1);
				break;
			case TK_TURNKICK:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,*bl,TK_READYTURN,1);
				break;
			case TK_COUNTER:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,*bl,TK_READYCOUNTER,1);
				break;
			default: // Rest just toggle inf to enable autotarget
				skill_combo_toggle_inf(bl,sce->val1,INF_SELF_SKILL);
				break;
			}
			break;
		case SC_C_MARKER:
			//Send mini-map, don't wait for first timer triggered
			if (src->type == BL_PC) {
				clif_crimson_marker(*(map_session_data *)(src), *bl, false);
			}
			break;
		case SC_ITEMSCRIPT: // Shows Buff Icons
			if (sd)
				clif_status_change(bl, (efst_type)val2, 1, tick, 0, 0, 0);
			break;
		case SC_GVG_GIANT:
		case SC_GVG_GOLEM:
		case SC_GVG_STUN:
		case SC_GVG_STONE:
		case SC_GVG_FREEZ:
		case SC_GVG_SLEEP:
		case SC_GVG_CURSE:
		case SC_GVG_SILENCE:
		case SC_GVG_BLIND:
			if (val1 || val2)
				status_zap(bl, val1 ? val1 : 0, val2 ? val2 : 0);
			break;
	}

	if( opt_flag[SCF_ONTOUCH] && sd && !sd->npc_ontouch_.empty() )
		npc_touchnext_areanpc(sd,false); // Run OnTouch_ on next char in range

	return 1;
}

/**
 * End all statuses except those listed
 * TODO: May be useful for dispel instead resetting a list there
 * @param src: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param type: Changes behaviour of the function
 * 	0: PC killed -> Place here statuses that do not dispel on death.
 * 	1: If for some reason status_change_end decides to still keep the status when quitting.
 * 	2: Do clif_changeoption()
 * 	3: Do not remove some permanent/time-independent effects
 * @return 1: Success 0: Fail
 */
int status_change_clear(struct block_list* bl, int type)
{
	status_change* sc;

	sc = status_get_sc(bl);

	if (!sc)
		return 0;

	// Cleaning all extras vars
	sc->comet_x = 0;
	sc->comet_y = 0;
#ifndef RENEWAL
	sc->sg_counter = 0;
#endif

	if (!sc->count)
		return 0;

	for (const auto &it : status_db) {
		sc_type status = static_cast<sc_type>(it.first);

		if (!sc->getSCE(status))
			continue;
		if (type == 0) { // Type 0: PC killed
			if (it.second->flag[SCF_NOREMOVEONDEAD]) {
				switch (status) {
					case SC_ELEMENTALCHANGE: // Only when its Holy or Dark that it doesn't dispell on death
						if (sc->getSCE(status)->val2 != ELE_HOLY && sc->getSCE(status)->val2 != ELE_DARK)
							break;
					default:
						continue;
				}
			}
		}

		if (type == 3 && it.second->flag[SCF_NOCLEARBUFF])
			continue;

		status_change_end(bl, status);
		if( type == 1 && sc->getSCE(status) ) { // If for some reason status_change_end decides to still keep the status when quitting. [Skotlex]
			(sc->count)--;
			if (sc->getSCE(status)->timer != INVALID_TIMER)
				delete_timer(sc->getSCE(status)->timer, status_change_timer);
			sc->deleteSCE(status);
		}
	}

	sc->opt1 = 0;
	sc->opt2 = 0;
	sc->opt3 = 0;

	if( type == 0 || type == 2 )
		clif_changeoption(bl);

	return 1;
}

/**
 * End a specific status after checking
 * @param bl: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param type: Status change (SC_*)
 * @param tid: Timer
 * @param file: Used for dancing save
 * @param line: Used for dancing save
 * @return 1: Success 0: Fail
 */
int status_change_end(struct block_list* bl, enum sc_type type, int tid)
{
	map_session_data *sd;
	status_change *sc;
	struct status_change_entry *sce;
	struct view_data *vd;
	std::shared_ptr<s_status_change_db> scdb = status_db.find(type);

	nullpo_ret(bl);

	sc = status_get_sc(bl);

	if(!sc || !(sce = sc->getSCE(type)) || !scdb)
		return 0;

	sd = BL_CAST(BL_PC,bl);

	if (sce->timer != tid && tid != INVALID_TIMER)
		return 0;

	if (tid == INVALID_TIMER) {
		if (type == SC_ENDURE && sce->val4)
			// Do not end infinite endure.
			return 0;
		if (type == SC_SPIDERWEB) {
			//Delete the unit group first to expire found in the status change
			std::shared_ptr<s_skill_unit_group> group, group2;
			t_tick tick = gettick();
			int pos = 1;
			if (sce->val2)
				if (!(group = skill_id2group(sce->val2)))
					sce->val2 = 0;
			if (sce->val3) {
				if (!(group2 = skill_id2group(sce->val3)))
					sce->val3 = 0;
				else if (!group || ((group->limit - DIFF_TICK(tick, group->tick)) > (group2->limit - DIFF_TICK(tick, group2->tick)))) {
					group = group2;
					pos = 2;
				}
			}
			if (sce->val4) {
				if (!(group2 = skill_id2group(sce->val4)))
					sce->val4 = 0;
				else if (!group || ((group->limit - DIFF_TICK(tick, group->tick)) > (group2->limit - DIFF_TICK(tick, group2->tick)))) {
					group = group2;
					pos = 3;
				}
			}
			if (pos == 1)
				sce->val2 = 0;
			else if (pos == 2)
				sce->val3 = 0;
			else if (pos == 3)
				sce->val4 = 0;
			if (group)
				skill_delunitgroup(group);
			if (!status_isdead(*bl) && (sce->val2 || sce->val3 || sce->val4))
				return 0; //Don't end the status change yet as there are still unit groups associated with it
		}
		if (sce->timer != INVALID_TIMER) // Could be a SC with infinite duration
			delete_timer(sce->timer,status_change_timer);
	}

	(sc->count)--;

	if (scdb->state.any())
		status_calc_state(bl,sc,scdb->state,false);

	sc->clearSCE(type);

	if (scdb->flag[SCF_DISPLAYPC] || scdb->flag[SCF_DISPLAYNPC])
		status_display_remove(bl,type);

	vd = status_get_viewdata(bl);
	std::bitset<SCB_MAX> calc_flag = scdb->calc_flag;
	status_data* status = status_get_status_data(*bl);

	switch(type) {
		case SC_KEEPING:
		case SC_BARRIER: {
			unit_data *ud = unit_bl2ud(bl);

			if (ud)
				ud->attackabletime = ud->canact_tick = ud->canmove_tick = gettick();
		}
			break;
		case SC_GRANITIC_ARMOR:
			{
				int damage = status->max_hp*sce->val3/100;
				if(status->hp < damage) // to not kill him
					damage = status->hp-1;
				status_damage(nullptr,bl,damage,0,0,1,0);
			}
			break;
		case SC_RUN:
		{
			struct unit_data *ud = unit_bl2ud(bl);
			bool begin_spurt = true;
			// Note: this int64 value is stored in two separate int32 variables (FIXME)
			t_tick starttick  = (t_tick)sce->val3&0x00000000ffffffffLL;
			starttick |= ((t_tick)sce->val4<<32)&0xffffffff00000000LL;

			if (ud) {
				if(!ud->state.running)
					begin_spurt = false;
				ud->state.running = 0;
				if (ud->walktimer != INVALID_TIMER)
					unit_stop_walking(bl,1);
			}
			if (begin_spurt && sce->val1 >= 7 &&
				DIFF_TICK(gettick(), starttick) <= 1000 &&
				(!sd || (sd->weapontype1 == W_FIST && sd->weapontype2 == W_FIST))
			)
				sc_start(bl,bl,SC_SPURT,100,sce->val1,skill_get_time2(scdb->skill_id, sce->val1));
		}
		break;
		case SC_AUTOBERSERK:
			if (sc->getSCE(SC_PROVOKE) && sc->getSCE(SC_PROVOKE)->val4 == 1)
				status_change_end(bl, SC_PROVOKE);
			break;

		case SC_ENDURE:
		case SC_DEFENDER:
		case SC_REFLECTSHIELD:
		case SC_AUTOGUARD:
			{
				map_session_data *tsd;
				if( bl->type == BL_PC ) { // Clear Status from others
					int i;
					for( i = 0; i < MAX_DEVOTION; i++ ) {
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) && tsd->sc.getSCE(type) )
							status_change_end(&tsd->bl, type);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag ) { // Clear Status from Master
					tsd = ((TBL_MER*)bl)->master;
					if( tsd && tsd->sc.getSCE(type) )
						status_change_end(&tsd->bl, type);
				}
			}
			break;
		case SC_DEVOTION:
			{
				struct block_list *d_bl = map_id2bl(sce->val1);
				if( d_bl ) {
					if( d_bl->type == BL_PC )
						((TBL_PC*)d_bl)->devotion[sce->val2] = 0;
					else if( d_bl->type == BL_MER )
						((TBL_MER*)d_bl)->devotion_flag = 0;
					clif_devotion(d_bl, nullptr);
				}
			}
			break;

		case SC_FLASHKICK: {
				map_session_data *tsd;

				if (!(tsd = map_id2sd(sce->val1)))
					break;

				tsd->stellar_mark[sce->val2] = 0;
			}
			break;

		case SC_SOULUNITY: {
				map_session_data *tsd;

				if (!(tsd = map_id2sd(sce->val2)))
					break;

				tsd->united_soul[sce->val3] = 0;
			}
			break;

		case SC_BLADESTOP:
			if(sce->val4) {
				int tid2 = sce->val4; //stop the status for the other guy of bladestop as well
				struct block_list *tbl = map_id2bl(tid2);
				status_change *tsc = status_get_sc(tbl);
				sce->val4 = 0;
				if(tbl && tsc && tsc->getSCE(SC_BLADESTOP)) {
					tsc->getSCE(SC_BLADESTOP)->val4 = 0;
					status_change_end(tbl, SC_BLADESTOP);
				}
				clif_bladestop(bl, tid2, 0);
			}
			break;
		case SC_DANCING:
			{
				map_session_data *dsd;

				if(sce->val4 && sce->val4 != BCT_SELF && (dsd=map_id2sd(sce->val4))) { // End status on partner as well
					status_change_entry *dsc = dsd->sc.getSCE(SC_DANCING);

					if(dsc) {
						// This will prevent recursive loops.
						dsc->val2 = 0;
						dsc->val4 = BCT_SELF;
						status_change_end(&dsd->bl, SC_DANCING);
					}
				}

				if(sce->val2) { // Erase associated land skill
					std::shared_ptr<s_skill_unit_group> group = skill_id2group(sce->val2);

					sce->val2 = 0;
					if (group)
						skill_delunitgroup(group);
				}

				if((sce->val1&0xFFFF) == CG_MOONLIT)
					clif_status_change(bl,EFST_MOON,0,0,0,0,0);
			}
			break;
		case SC_NOCHAT:
			if (sd && sd->status.manner < 0 && tid != INVALID_TIMER)
				sd->status.manner = 0;
			if (sd && tid == INVALID_TIMER) {
				clif_changemanner( *sd );
				clif_updatestatus(*sd,SP_MANNER);
			}
			break;
		case SC_SPLASHER:
		case SC_ROSEBLOSSOM:
			{
				struct block_list *src=map_id2bl(sce->val3);

				if(src && tid != INVALID_TIMER)
					skill_castend_damage_id(src, bl, sce->val2, sce->val1, gettick(), SD_LEVEL );
			}
			break;
		case SC_CLOSECONFINE2:
			{
				struct block_list *src = sce->val2?map_id2bl(sce->val2):nullptr;
				status_change *sc2 = src?status_get_sc(src):nullptr;
				if (src && sc2 && sc2->getSCE(SC_CLOSECONFINE)) {
					// If status was already ended, do nothing.
					// Decrease count
					if (--(sc2->getSCE(SC_CLOSECONFINE)->val1) <= 0) // No more holds, free him up.
						status_change_end(src, SC_CLOSECONFINE);
				}
			}
			[[fallthrough]];
		case SC_CLOSECONFINE:
			if (sce->val2 > 0) {
				// Caster has been unlocked... nearby chars need to be unlocked.
				int range = 1
					+ skill_get_range2(bl, scdb->skill_id, sce->val1, true)
					+ skill_get_range2(bl, TF_BACKSLIDING, 1, true); // Since most people use this to escape the hold....
				map_foreachinallarea(status_change_timer_sub,
					bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,bl,sce,type,gettick());
			}
			break;
		case SC_COMBO:
			skill_combo_toggle_inf(bl,sce->val1,0);
			break;
		case SC_MARIONETTE:
		case SC_MARIONETTE2: // Marionette target
			if (sce->val1) { // Check for partner and end their marionette status as well
				enum sc_type type2 = (type == SC_MARIONETTE) ? SC_MARIONETTE2 : SC_MARIONETTE;
				struct block_list *pbl = map_id2bl(sce->val1);
				status_change* sc2 = pbl?status_get_sc(pbl):nullptr;

				if (sc2 && sc2->getSCE(type2)) {
					sc2->getSCE(type2)->val1 = 0;
					status_change_end(pbl, type2);
				}
			}
			break;

		case SC_CONCENTRATION:
			if (sc->getSCE(SC_ENDURE) && !sc->getSCE(SC_ENDURE)->val4)
				status_change_end(bl, SC_ENDURE);
			break;
		case SC_BERSERK:
			if(status->hp > 200 && sc && sc->getSCE(SC__BLOODYLUST)) {
				status_percent_heal(bl, 100, 0);
				status_change_end(bl, SC__BLOODYLUST);
			} else if (status->hp > 100 && sce->val2) // If val2 is removed, no HP penalty (dispelled?) [Skotlex]
				status_set_hp(bl, 100, 0);
			if(sc->getSCE(SC_ENDURE) && sc->getSCE(SC_ENDURE)->val4) {
				sc->getSCE(SC_ENDURE)->val4 = 0;
				status_change_end(bl, SC_ENDURE);
			}
			sc_start4(bl, bl, SC_REGENERATION, 100, 10,0,0,(RGN_HP|RGN_SP), skill_get_time(LK_BERSERK, sce->val1));
			break;
		case SC_GOSPEL:
			if (sce->val3) { // Clear the group.
				std::shared_ptr<s_skill_unit_group> group = skill_id2group(sce->val3);

				sce->val3 = 0;
				if (group)
					skill_delunitgroup(group);
			}
			break;
#ifndef RENEWAL
		case SC_HERMODE:
			if(sce->val3 == BCT_SELF)
				skill_clear_unitgroup(bl);
			break;
		case SC_BASILICA: // Clear the skill area. [Skotlex]
				if (sce->val3 && sce->val4 == bl->id) {
					std::shared_ptr<s_skill_unit_group> group = skill_id2group(sce->val3);

					sce->val3 = 0;
					if (group)
						skill_delunitgroup(group);
				}
				break;
#endif
		case SC_TRICKDEAD:
			if (vd) vd->dead_sit = 0;
			break;
		case SC_WARM:
		case SC__MANHOLE:
		case SC_BANDING:
		case SC_LEADERSHIP:
		case SC_GLORYWOUNDS:
		case SC_SOULCOLD:
		case SC_HAWKEYES:
			if (sce->val4) { // Clear the group.
				std::shared_ptr<s_skill_unit_group> group = skill_id2group(sce->val4);

				sce->val4 = 0;
				if( group ) // Might have been cleared before status ended, e.g. land protector
					skill_delunitgroup(group);
			}
			break;
		case SC_JAILED:
			if(sd && sd->mapindex == sce->val2)
				pc_setpos(sd,(unsigned short)sce->val3,sce->val4&0xFFFF, sce->val4>>16, CLR_TELEPORT);
			break; // Guess hes not in jail :P
		case SC_CHANGE:
			if (tid == INVALID_TIMER)
		 		break;
			// "lose almost all their HP and SP" on natural expiration.
			status_set_hp(bl, 10, 0);
			status_set_sp(bl, 10, 0);
			break;
		case SC_AUTOTRADE:
			if (tid == INVALID_TIMER)
				break;
			// Vending is not automatically closed for autovenders
			vending_closevending(sd);
			map_quit(sd);
			// Because map_quit calls status_change_end with tid -1
			// from here it's not neccesary to continue
			return 1;
			break;
		case SC_STOP:
			if( sce->val2 ) {
				struct block_list* tbl = map_id2bl(sce->val2);
				sce->val2 = 0;
				if( tbl && (sc = status_get_sc(tbl)) && sc->getSCE(SC_STOP) && sc->getSCE(SC_STOP)->val2 == bl->id )
					status_change_end(tbl, SC_STOP);
			}
			break;
		case SC_TENSIONRELAX:
			if (sc && (sc->getSCE(SC_WEIGHT50) || sc->getSCE(SC_WEIGHT90)))
				status_get_regen_data(bl)->state.overweight = 1; // Add the overweight flag back
			break;
		case SC_MONSTER_TRANSFORM:
		case SC_ACTIVE_MONSTER_TRANSFORM:
			if (sce->val2)
				status_change_end(bl, (sc_type)sce->val2);
			break;

		/* 3rd Stuff */
		case SC_MILLENNIUMSHIELD:
			clif_millenniumshield( *bl, 0 );
			break;
		case SC_HALLUCINATIONWALK:
			sc_start(bl,bl,SC_HALLUCINATIONWALK_POSTDELAY,100,sce->val1,skill_get_time2(GC_HALLUCINATIONWALK,sce->val1));
			break;
		case SC_WHITEIMPRISON:
			{
				struct block_list* src = map_id2bl(sce->val2);
				if( tid == -1 || !src)
					break; // Terminated by Damage
				status_fix_damage(src,bl,400*sce->val1,clif_damage(*bl,*bl,gettick(),0,0,400*sce->val1,0,DMG_NORMAL,0,false),WL_WHITEIMPRISON);
			}
			break;
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit_bl2ud(bl);
				if (ud) {
					ud->state.running = 0;
					if (ud->walktimer != INVALID_TIMER)
						unit_stop_walking(bl,1);
				}
			}
			break;
		case SC__SHADOWFORM:
			{
				map_session_data *s_sd = map_id2sd(sce->val2);

				if (s_sd) s_sd->shadowform_id = 0;
			}
			break;
		case SC_SATURDAYNIGHTFEVER: // Sit down force of Saturday Night Fever has the duration of only 3 seconds.
			sc_start(bl, bl,SC_SITDOWN_FORCE,100,sce->val1,skill_get_time2(WM_SATURDAY_NIGHT_FEVER,sce->val1));
			break;
		case SC_NEUTRALBARRIER_MASTER:
		case SC_STEALTHFIELD_MASTER:
			if( sce->val2 ) {
				std::shared_ptr<s_skill_unit_group> group = skill_id2group(sce->val2);

				sce->val2 = 0;
				if( group ) // Might have been cleared before status ended, e.g. land protector
					skill_delunitgroup(group);
			}
			break;
		case SC_CURSEDCIRCLE_ATKER:
			if( sce->val2 ) // Used the default area size cause there is a chance the caster could knock back and can't clear the target.
				map_foreachinallrange(status_change_timer_sub, bl, AREA_SIZE + 3, BL_CHAR, bl, sce, SC_CURSEDCIRCLE_TARGET, gettick());
			break;
		case SC_RAISINGDRAGON:
			if( sd && !pc_isdead(sd) ) {
				int i = min(sd->spiritball,5);
				pc_delspiritball(sd, sd->spiritball, 0);
				status_change_end(bl, SC_EXPLOSIONSPIRITS);
				while( i > 0 ) {
					pc_addspiritball(sd, skill_get_time(MO_CALLSPIRITS, pc_checkskill(sd,MO_CALLSPIRITS)), 5);
					--i;
				}
			}
			break;
		case SC_CURSEDCIRCLE_TARGET:
			{
				struct block_list *src = map_id2bl(sce->val2);
				status_change *sc2 = status_get_sc(src);

				if( sc2 && sc2->getSCE(SC_CURSEDCIRCLE_ATKER) && --(sc2->getSCE(SC_CURSEDCIRCLE_ATKER)->val2) == 0 ) {
					clif_bladestop(bl, sce->val2, 0);
					status_change_end(src, SC_CURSEDCIRCLE_ATKER);
				}
			}
			break;
		case SC_SITDOWN_FORCE:
		case SC_BANANA_BOMB_SITDOWN:
			if( sd && pc_issit(sd) && pc_setstand(sd, false) )
				skill_sit(sd, false);
			break;
		case SC_KYOUGAKU:
			clif_status_load(bl, EFST_KYOUGAKU, 0); // Avoid client crash
			clif_status_load(bl, EFST_ACTIVE_MONSTER_TRANSFORM, 0);
			break;
		case SC_INTRAVISION:
			calc_flag = status_db.getSCB_ALL(); // Required for overlapping
			break;

		case SC_GRAVITYCONTROL:
			status_fix_damage(bl, bl, sce->val2, clif_damage(*bl, *bl, gettick(), 0, 0, sce->val2, 0, DMG_NORMAL, 0, false), 0);
			clif_specialeffect(bl, 223, AREA);
			clif_specialeffect(bl, 330, AREA);
			break;
			
		case SC_OVERED_BOOST:
			switch (bl->type) {
				case BL_HOM: {
						struct homun_data *hd = BL_CAST(BL_HOM,bl);

						if( hd )
							hd->homunculus.hunger = max(1,hd->homunculus.hunger - 50);
					}
					break;
				case BL_PC:
					status_zap(bl,0,status_get_max_sp(bl) / 2);
					break;
			}
			break;
		case SC_FULL_THROTTLE: {
				int sec = skill_get_time2(scdb->skill_id, sce->val1);

				clif_status_change(bl, EFST_DEC_AGI, 1, sec, 0, 0, 0);
				sc_start(bl, bl, SC_REBOUND, 100, sce->val1, sec);
			}
			break;
		case SC_REBOUND:
			clif_status_load(bl, EFST_DEC_AGI, 0);
			break;
		case SC_ITEMSCRIPT: // Removes Buff Icons
			if (sd)
				clif_status_load(bl, (enum efst_type)sce->val2, 0);
			break;
		case SC_C_MARKER:
			{
				// Remove mark data from caster
				map_session_data *caster = map_id2sd(sce->val2);
				uint8 i = 0;

				if (!caster)
					break;
				ARR_FIND(0,MAX_SKILL_CRIMSON_MARKER,i,caster->c_marker[i] == bl->id);
				if (i < MAX_SKILL_CRIMSON_MARKER) {
					caster->c_marker[i] = 0;
					clif_crimson_marker( *caster, *bl, true );
				}
			}
			break;
		case SC_H_MINE:
			// Only drop the material from target if expired naturally
			if( tid != INVALID_TIMER ){
				map_session_data *caster = nullptr;

				if (status_isdead(*bl) || !(caster = map_id2sd(sce->val2)))
					break;

				std::shared_ptr<s_skill_db> skill = skill_db.find(RL_H_MINE);

				if (!item_db.exists(skill->require.itemid[0]))
					break;

				struct item it = {};

				it.nameid = skill->require.itemid[0];
				it.amount = max(skill->require.amount[0],1);
				it.identify = 1;

				map_addflooritem(&it, it.amount, bl->m,bl->x, bl->y, caster->status.char_id, 0, 0, 4, 0);
			}
			break;
		case SC_VACUUM_EXTREME:
			///< !CHECKME: Seems on official, there's delay before same target can be vacuumed in same area again [Cydh]
			sc_start2(bl, bl, SC_VACUUM_EXTREME_POSTDELAY, 100, sce->val1, sce->val2, skill_get_time2(SO_VACUUM_EXTREME,sce->val1));
			break;
		case SC_DIMENSION1:
		case SC_DIMENSION2:
			if (sd)
				pc_delspiritball(sd, 1, 0);
			break;
		case SC_SOULENERGY:
			if (sd)
				pc_delsoulball(sd, sd->soulball, false);
			break;
		case SC_MADOGEAR:
			status_db.removeByStatusFlag(bl, { SCF_MADOENDCANCEL });
			if (sd)
				pc_bonus_script_clear(sd, BSF_REM_ON_MADOGEAR);
			break;
		case SC_HOMUN_TIME:
			if (sd && hom_is_active(sd->hd))
				hom_vaporize(sd, HOM_ST_REST);
			break;
		case SC_SERVANT_SIGN: {
				map_session_data *tsd = map_id2sd(sce->val1);

				if( tsd != nullptr )
					tsd->servant_sign[sce->val2] = 0;
			}
			break;
		case SC_SOUNDBLEND: {
				block_list *src = map_id2bl(sce->val2);

				if (src && tid != INVALID_TIMER)
					skill_castend_damage_id(src, bl, TR_SOUNDBLEND, sce->val1, gettick(), SD_LEVEL|SD_ANIMATION);
			}
			break;
		case SC_SERVANTWEAPON:
			if( sd ){
				pc_delservantball( *sd, sd->servantball );
			}
			break;
		case SC_ABYSSFORCEWEAPON:
			if( sd ){
				pc_delabyssball( *sd, sd->abyssball );
			}
			break;
	}

	// End statuses found in the EndOnEnd list.
	if (!scdb->endonend.empty()) {
		for (const auto &it : scdb->endonend) {
			status_change_end(bl, it);
		}
	}

	// Reset the options as needed
	std::bitset<SCF_MAX> opt_flag = scdb->flag;
	bool disable_opt_flag = false;

	switch (type) {
		case SC_DANCING:
			if ((sce->val1&0xFFFF) == CG_MOONLIT)
				sc->opt3 &= ~OPT3_MOONLIT;
			break;
		case SC_INCATKRATE: // Simulated Explosion spirits effect.
			if (bl->type != BL_MOB) {
				disable_opt_flag = true;
				break;
			}
			break;
	}

	if (scdb->opt1)
		sc->opt1 = OPT1_NONE;

	if (scdb->opt2)
		sc->opt2 &= ~scdb->opt2;

	if (scdb->opt3)
		sc->opt3 &= ~scdb->opt3;

	if (scdb->look)
		sc->option &= ~scdb->look;

	if (calc_flag[SCB_DYE]) { // Restore DYE color
		if (vd && !vd->cloth_color && sce->val4)
			clif_changelook(bl,LOOK_CLOTHES_COLOR,sce->val4);
		calc_flag.reset(SCB_DYE);
	}

	/*if (calc_flag[SCB_BODY])// Might be needed in the future. [Rytech]
	{	//Restore body style
		if (vd && !vd->body_style && sce->val4)
			clif_changelook(bl,LOOK_BODY2,sce->val4);
		calc_flag.reset(SCB_BODY);
	}*/

	// On Aegis, when turning off a status change, first goes the sc packet, then the option packet.
	int status_icon = scdb->icon;

#if PACKETVER < 20151104
	if (status_icon == EFST_WEAPONPROPERTY)
		status_icon = EFST_ATTACK_PROPERTY_NOTHING + sce->val1; // Assign status icon for older clients
#endif

	clif_status_change(bl,status_icon,0,0,0,0,0);

	if( opt_flag[SCF_NONPLAYER] ) // bugreport:681
		clif_changeoption2( *bl );
	else if (!disable_opt_flag && (opt_flag[SCF_SENDOPTION] || opt_flag[SCF_ONTOUCH] || opt_flag[SCF_UNITMOVE] || opt_flag[SCF_NONPLAYER] || opt_flag[SCF_SENDLOOK])) {
		clif_changeoption(bl);
		if (sd && opt_flag[SCF_SENDLOOK]) {
			clif_changelook(bl,LOOK_BASE,sd->vd.class_);
			clif_get_weapon_view(sd,&sd->vd.weapon,&sd->vd.shield);
			clif_changelook(bl,LOOK_WEAPON,sd->vd.weapon);
			clif_changelook(bl,LOOK_SHIELD,sd->vd.shield);
			clif_changelook(bl,LOOK_CLOTHES_COLOR,cap_value(sd->status.clothes_color,0,battle_config.max_cloth_color));
			clif_changelook(bl,LOOK_BODY2,cap_value(sd->status.body,0,battle_config.max_body_style));
		}
	}
	if (calc_flag.any()) {
#ifndef RENEWAL
		if (type == SC_MAGICPOWER) {
			//If Mystical Amplification ends, MATK is immediately recalculated
			status_calc_bl_(bl, calc_flag, SCO_FORCE);
		} else
#endif
			status_calc_bl_(bl, calc_flag);
	}

	if(opt_flag[SCF_UNITMOVE]) // Out of hiding, invoke on place.
		skill_unit_move(bl,gettick(),1);

	if(opt_flag[SCF_ONTOUCH] && sd && !sd->state.warping && map_getcell(bl->m,bl->x,bl->y,CELL_CHKNPC))
		npc_touch_area_allnpc(sd,bl->m,bl->x,bl->y); // Trigger on-touch event.

	// Needed to be here to make sure OPT1_STONEWAIT has been cleared from the target (only on natural expiration of the stone wait timer)
	if (type == SC_STONEWAIT && tid != INVALID_TIMER)
		status_change_start(bl, bl, SC_STONE, 100, sce->val1, sce->val2, 0, 0, sce->val3, SCSTART_NOAVOID);

	ers_free(sc_data_ers, sce);
	return 1;
}

/**
 * Resets timers for statuses
 * Used with reoccurring status effects, such as dropping SP every 5 seconds
 * @param tid: Timer ID
 * @param tick: How long before next call
 * @param id: ID of character
 * @param data: Information passed through the timer call
 * @return 1: Success 0: Fail
 */
TIMER_FUNC(status_change_timer){
	enum sc_type type = (sc_type)data;
	struct block_list *bl;
	map_session_data *sd;
	int interval = status_get_sc_interval(type);
	bool dounlock = false;

	bl = map_id2bl(id);
	if(!bl) {
		ShowDebug("status_change_timer: Null pointer id: %d data: %" PRIdPTR "\n", id, data);
		return 0;
	}

	status_change * const sc = status_get_sc(bl);

	if(!sc) {
		ShowDebug("status_change_timer: Null pointer id: %d data: %" PRIdPTR " bl-type: %d\n", id, data, bl->type);
		return 0;
	}
	
	struct status_change_entry * const sce = sc->getSCE(type);
	if(!sce) {
		ShowDebug("status_change_timer: Null pointer id: %d data: %" PRIdPTR " bl-type: %d\n", id, data, bl->type);
		return 0;
	}
	if( sce->timer != tid ) {
		ShowError("status_change_timer: Mismatch for type %d: %d != %d (bl id %d)\n",type,tid,sce->timer, bl->id);
		return 0;
	}

	const status_data* status = status_get_status_data(*bl);

	sd = BL_CAST(BL_PC, bl);

	std::function<void (t_tick)> sc_timer_next = [&sce, &bl, &data](t_tick t) {
		sce->timer = add_timer(t, status_change_timer, bl->id, data);
	};
	
	switch(type) {
	case SC_MAXIMIZEPOWER:
	case SC_CLOAKING:
		if(!status_charge(bl, 0, 1))
			break; // Not enough SP to continue.
		sc_timer_next(sce->val2+tick);
		return 0;

	case SC_CHASEWALK:
		if(!status_charge(bl, 0, sce->val4))
			break; // Not enough SP to continue.

		if (!sc->getSCE(SC_CHASEWALK2)) {
			sc_start(bl,bl, SC_CHASEWALK2,100,1<<(sce->val1-1),
				(t_tick)(sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_ROGUE?10:1) // SL bonus -> x10 duration
				*skill_get_time2(status_db.getSkill(type),sce->val1));
		}
		sc_timer_next(sce->val2+tick);
		return 0;
	break;

	case SC_HIDING:
		if(--(sce->val2)>0) {

			if(sce->val2 % sce->val4 == 0 && !status_charge(bl, 0, 1))
				break; // Fail if it's time to substract SP and there isn't.

			sc_timer_next(1000+tick);
			return 0;
		}
	break;

	case SC_SIGHT:
	case SC_RUWACH:
	case SC_SIGHTBLASTER:
		if(type == SC_SIGHTBLASTER) {
			//Restore trap immunity
			if(sce->val4%2)
				sce->val4--;
			map_foreachinallrange( status_change_timer_sub, bl, sce->val3, BL_CHAR|BL_SKILL, bl, sce, type, tick);
		} else {
			map_foreachinallrange( status_change_timer_sub, bl, sce->val3, BL_CHAR, bl, sce, type, tick);
			skill_reveal_trap_inarea(bl, sce->val3, bl->x, bl->y);
		}

		if( --(sce->val2)>0 ) {
			sce->val4 += 20; // Use for Shadow Form 2 seconds checking.
			sc_timer_next(20+tick);
			return 0;
		}
		break;

	case SC_PROVOKE:
		if(sce->val4) { // Auto-provoke (it is ended in status_heal)
			sc_timer_next(1000*60+tick);
			return 0;
		}
		break;

	case SC_STONE:
		if (sce->val4 >= 0 && status->hp > status->max_hp / 4)
			status_percent_damage(nullptr, bl, -1, 0, false);
		break;

	case SC_POISON:
	case SC_DPOISON:
		if (sce->val4 >= 0 && !sc->getSCE(SC_SLOWPOISON)) {
			unsigned int damage = 0;
			if (sd)
				damage = (type == SC_DPOISON) ? 2 + status->max_hp / 50 : 2 + status->max_hp * 3 / 200;
			else
				damage = (type == SC_DPOISON) ? 2 + status->max_hp / 100 : 2 + status->max_hp / 200;
			if (status->hp > umax(status->max_hp / 4, damage)) // Stop damaging after 25% HP left.
				status_zap(bl, damage, 0);
		}
		break;

	case SC_BLEEDING:
		if (sce->val4 >= 0) {
			int64 damage = rnd() % 600 + 200;
			if (!sd && damage >= status->hp)
				damage = status->hp - 1; // No deadly damage for monsters
			map_freeblock_lock();
			dounlock = true;
			status_zap(bl, damage, 0);
		}
		break;

	case SC_BURNING:
		if (sce->val4 >= 0) {
			int64 damage = 1000 + (3 * status->max_hp) / 100; // Deals fixed (1000 + 3%*MaxHP)
			map_freeblock_lock();
			dounlock = true;
			status_fix_damage(bl, bl, damage, clif_damage(*bl, *bl, tick, 0, 1, damage, 1, DMG_NORMAL, 0, false),0);
		}
		break;
		
	case SC_TOXIN:
		if (sce->val4 >= 0) { // Damage is every 10 seconds including 3%sp drain.
			if (sce->val3 == 1) { // Target
				map_freeblock_lock();
				dounlock = true;
				status_damage(bl, bl, 1, status->max_sp * 3 / 100, clif_damage(*bl, *bl, tick, status->amotion, status->dmotion + 500, 1, 1, DMG_NORMAL, 0, false), 0, 0);
			} else { // Caster
				interval = 1000; // Assign here since status_get_sc_internval() contains the target interval.

				if (status->sp < status->max_sp)
					status_heal(bl, 0, (int)status->max_sp * 1 / 100, 1);
			}
		}
		break;

	case SC_MAGICMUSHROOM:
		if (sce->val4 >= 0) {
			bool flag = 0;
			int64 damage = status->max_hp * 3 / 100;
			if (status->hp <= damage)
				damage = status->hp - 1; // Cannot Kill

			if (damage > 0) { // 3% Damage each 4 seconds
				map_freeblock_lock();
				status_zap(bl, damage, 0);
				flag = !sc->getSCE(type); // Killed? Should not
				map_freeblock_unlock();
			}

			if (!flag) { // Random Skill Cast
				if (magic_mushroom_db.size() > 0 && sd && !pc_issit(sd)) { // Can't cast if sit
					auto mushroom_spell = magic_mushroom_db.begin();

					std::advance(mushroom_spell, rnd() % magic_mushroom_db.size());

					uint16 mushroom_skill_id = mushroom_spell->second->skill_id;

					if (!skill_get_index(mushroom_skill_id))
						break;

					unit_stop_attack(bl);
					unit_skillcastcancel(bl, 1);

					switch (skill_get_casttype(mushroom_skill_id)) { // Magic Mushroom skills are buffs or area damage
					case CAST_GROUND:
						skill_castend_pos2(bl, bl->x, bl->y, mushroom_skill_id, 1, tick, 0);
						break;
					case CAST_NODAMAGE:
						skill_castend_nodamage_id(bl, bl, mushroom_skill_id, 1, tick, 0);
						break;
					case CAST_DAMAGE:
						skill_castend_damage_id(bl, bl, mushroom_skill_id, 1, tick, 0);
						break;
					}
				}
				clif_emotion(bl, ET_SMILE);
			}
		}
		break;
		
	case SC_PYREXIA:
		if (sce->val4 >= 0) {
			map_freeblock_lock();
			dounlock = true;
			status_fix_damage(bl, bl, 100, clif_damage(*bl, *bl, tick, status->amotion, status->dmotion + 500, 100, 1, DMG_NORMAL, 0, false),0);
			unit_skillcastcancel(bl, 2);
		}
		break;
		
	case SC_LEECHESEND:
		if (sce->val4 >= 0) {
			int64 damage = status->vit * (sce->val1 - 3) + (int)status->max_hp / 100; // {Target VIT x (New Poison Research Skill Level - 3)} + (Target HP/100)
			map_freeblock_lock();
			dounlock = true;
			status_fix_damage(bl, bl, damage, clif_damage(*bl, *bl, tick, status->amotion, status->dmotion + 500, damage, 1, DMG_NORMAL, 0, false),0);
			unit_skillcastcancel(bl, 2);
		}
		break;

	case SC_DEATHHURT:
		if (sce->val4 >= 0) {
			if (status->hp < status->max_hp)
				status_heal(bl, (int)status->max_hp * 1 / 100, 0, 1);
		}
		break;

	case SC_TENSIONRELAX:
		if(status->max_hp > status->hp && --(sce->val3) >= 0) {
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;

	case SC_KNOWLEDGE:
		if (!sd) break;
		if(bl->m == sd->feel_map[0].m ||
			bl->m == sd->feel_map[1].m ||
			bl->m == sd->feel_map[2].m)
		{	// Timeout will be handled by pc_setpos
			sce->timer = INVALID_TIMER;
			return 0;
		}
		break;

	case SC_S_LIFEPOTION:
	case SC_L_LIFEPOTION:
	case SC_M_LIFEPOTION:
	case SC_G_LIFEPOTION:
		if( --(sce->val4) >= 0 ) {
			// val1 < 0 = per max% | val1 > 0 = exact amount
			int hp = 0;
			if( status->hp < status->max_hp && !sc->getSCE(SC_BERSERK) )
				hp = (sce->val1 < 0) ? (int)(status->max_hp * -1 * sce->val1 / 100.) : sce->val1;
			status_heal(bl, hp, 0, 0);
			sc_timer_next((sce->val2 * 1000) + tick);
			return 0;
		}
		break;

	case SC_S_MANAPOTION:
		if( --(sce->val4) >= 0 ) {
			// val1 < 0 = per max% | val1 > 0 = exact amount
			int sp = 0;
			if( status->sp < status->max_sp && !sc->getSCE(SC_BERSERK) )
				sp = (sce->val1 < 0) ? (int)(status->max_sp * -1 * sce->val1 / 100.) : sce->val1;
			status_heal(bl, 0, sp, 0);
			sc_timer_next((sce->val2 * 1000) + tick);
			return 0;
		}
		break;
		
	case SC_GRADUAL_GRAVITY:
		if (sce->val4 >= 0) {
			status_zap(bl, status->max_hp * sce->val2 / 100, 0);
		}
		break;

	case SC_BOSSMAPINFO:
		if( sd && sce->val4 >= 0 ){
			mob_data* boss_md = map_id2boss( sce->val1 );

			if( boss_md == nullptr ){
				sce->val4 = 0;
				break;
			}

			// Not on same map anymore
			if( sd->bl.m != boss_md->bl.m ){
				sce->val4 = 0;
				break;
			// Boss is alive - Update X, Y on minimap
			}else if( boss_md->bl.prev != nullptr ){
				sce->val2 = 0;
				clif_bossmapinfo( *sd, boss_md, BOSS_INFO_ALIVE );
			// Boss is dead
			}else if( boss_md->spawn_timer != INVALID_TIMER && sce->val2 == 0 ){
				sce->val2 = 1;
				clif_bossmapinfo( *sd, boss_md, BOSS_INFO_DEAD );
			}
		}
		break;

	case SC_DANCING: // SP consumption by time of dancing skills
		{
			int s = 0;
			int sp = 1;
			if (--sce->val3 <= 0)
				break;
			switch(sce->val1&0xFFFF) {
#ifndef RENEWAL
				case BD_RICHMANKIM:
				case BD_DRUMBATTLEFIELD:
				case BD_RINGNIBELUNGEN:
				case BD_SIEGFRIED:
				case BA_DISSONANCE:
				case BA_ASSASSINCROSS:
				case DC_UGLYDANCE:
					s=3;
					break;
				case BD_LULLABY:
				case BD_ETERNALCHAOS:
				case BD_ROKISWEIL:
				case DC_FORTUNEKISS:
					s=4;
					break;
				case CG_HERMODE:
				case BD_INTOABYSS:
				case BA_WHISTLE:
				case DC_HUMMING:
				case BA_POEMBRAGI:
				case DC_SERVICEFORYOU:
					s=5;
					break;
				case BA_APPLEIDUN:
					s=6;
					break;
#endif
				case CG_MOONLIT:
					// Moonlit's cost is 4sp*skill_lv [Skotlex]
					sp= 4*(sce->val1>>16);
					// Upkeep is also every 10 secs.
#ifndef RENEWAL
				[[fallthrough]];
				case DC_DONTFORGETME:
#endif
					s=10;
					break;
			}
			if( s != 0 && sce->val3 % s == 0 ) {
#ifndef RENEWAL
				if (sc->getSCE(SC_LONGING))
					sp*= 3;
#endif
				if (!status_charge(bl, 0, sp))
					break;
			}
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_BERSERK:
		// 5% every 10 seconds [DracoRPG]
		if( --( sce->val3 ) > 0 && status_charge(bl, sce->val2, 0) && status->hp > 100 ) {
			sc_timer_next(sce->val4+tick);
			return 0;
		}
		break;

	case SC_NOCHAT:
		if(sd) {
			sd->status.manner++;
			clif_changemanner( *sd );
			clif_updatestatus(*sd,SP_MANNER);
			if (sd->status.manner < 0) { // Every 60 seconds your manner goes up by 1 until it gets back to 0.
				sc_timer_next(60000+tick);
				return 0;
			}
		}
		break;

	case SC_SPLASHER:
		// Custom Venom Splasher countdown timer
		// if (sce->val4 % 1000 == 0) {
		// 	char timer[10];
		// 	snprintf (timer, 10, "%d", sce->val4/1000);
		// 	clif_message(bl, timer);
		// }
		if((sce->val4 -= 500) > 0) {
			sc_timer_next(500 + tick);
			return 0;
		}
		break;

	case SC_MARIONETTE:
	case SC_MARIONETTE2:
		{
			struct block_list *pbl = map_id2bl(sce->val1);
			if( pbl && check_distance_bl(bl, pbl, 7) ) {
				sc_timer_next(1000 + tick);
				return 0;
			}
		}
		break;

	case SC_GOSPEL:
		if(sce->val4 == BCT_SELF && --(sce->val2) > 0) {
			int hp, sp;
			hp = (sce->val1 > 5) ? 45 : 30;
			sp = (sce->val1 > 5) ? 35 : 20;
			if(!status_charge(bl, hp, sp))
				break;
			sc_timer_next(10000+tick);
			return 0;
		}
		break;

	case SC_JAILED:
		if(sce->val1 == INT_MAX || --(sce->val1) > 0) {
			sc_timer_next(60000+tick);
			return 0;
		}
		break;

	case SC_BLIND:
		if(sc->getSCE(SC_FOGWALL)) { // Blind lasts forever while you are standing on the fog.
			sc_timer_next(5000+tick);
			return 0;
		}
		break;
	case SC_ABUNDANCE:
		if(--(sce->val4) > 0) {
			status_heal(bl,0,60,0);
			sc_timer_next(10000+tick);
		}
		break;
		
	case SC_OBLIVIONCURSE:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl,ET_QUESTION);
			sc_timer_next(3000 + tick);
			return 0;
		}
		break;

	case SC_WEAPONBLOCKING:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl,0,3) )
				break;
			sc_timer_next(5000+tick);
			return 0;
		}
		break;

	case SC_CLOAKINGEXCEED:
		if(!status_charge(bl,0,10-sce->val1))
			break;
		sc_timer_next(1000 + tick);
		return 0;

	case SC_RENOVATIO:
		if( --(sce->val4) >= 0 ) {
			int heal = status->max_hp * (sce->val1 + 4) / 100;
			if( sc && sc->getSCE(SC_AKAITSUKI) && heal )
				heal = ~heal + 1;
			status_heal(bl, heal, 0, 3);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_SPHERE_1:
	case SC_SPHERE_2:
	case SC_SPHERE_3:
	case SC_SPHERE_4:
	case SC_SPHERE_5:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl, 0, 1) )
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_FREEZE_SP:
		if( !status_charge(bl, 0, sce->val2) ) {
			int i;
			for(i = SC_SPELLBOOK1; i <= SC_MAXSPELLBOOK; i++) // Also remove stored spell as well.
				status_change_end(bl, (sc_type)i);
			break;
		}
		sc_timer_next(10000 + tick);
		return 0;

	case SC_ELECTRICSHOCKER:
		if( --(sce->val4) >= 0 ) {
			status_charge(bl, 0, 5 * sce->val1 * status->max_sp / 100);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_CAMOUFLAGE:
		if (!status_charge(bl, 0, 7 - sce->val1))
			break;
		if (--sce->val4 >= 0)
			sce->val3++;
		sc_timer_next(1000 + tick);
		return 0;

	case SC__REPRODUCE:
		if(!status_charge(bl, 0, 1))
			break;
		sc_timer_next(1000+tick);
		return 0;

	case SC__SHADOWFORM:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl, 0, 11 - sce->val1) )
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC__INVISIBILITY:
		if( !status_charge(bl, 0, (12 - 2 * sce->val1) * status->max_sp / 100) ) // 6% - skill_lv.
			break;
		sc_timer_next(1000 + tick);
		return 0;

	case SC_STRIKING:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl,0, sce->val3 ) )
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_WARMER: {
			int hp = 0;
			status_change *ssc = status_get_sc(map_id2bl(sce->val2));

			if (ssc && ssc->getSCE(SC_HEATER_OPTION))
				hp = status->max_hp * 3 * sce->val1 / 100;
			else
				hp = status->max_hp * sce->val1 / 100;
			if (sc && sc->getSCE(SC_AKAITSUKI) && hp)
				hp = ~hp + 1;
			if (status->hp != status->max_hp)
				status_heal(bl, hp, 0, 0);
			sc_timer_next(3000 + tick);
			return 0;
		}

	case SC_HELLS_PLANT:
		if( sce->val4 >= 0 ){
			skill_castend_damage_id( bl, bl, GN_HELLS_PLANT_ATK, sce->val1, tick, 0 );
		}
		break;

	case SC_VOICEOFSIREN:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl,ET_THROB);
			sc_timer_next(2000 + tick);
			return 0;
		}
		break;

	case SC_DEEPSLEEP:
		if( --(sce->val4) >= 0 ) { // Recovers 3% HP/SP every 2 seconds.
			status_heal(bl, status->max_hp * 3 / 100, status->max_sp * 3 / 100, 2);
			sc_timer_next(2000 + tick);
			return 0;
		}
		break;

	case SC_SATURDAYNIGHTFEVER:
		// 1% HP/SP drain every val4 seconds [Jobbie]
		if( --(sce->val3) >= 0 ) {
			if( !status_charge(bl, status->hp / 100, status->sp / 100) )
				break;
			sc_timer_next(sce->val4+tick);
			return 0;
		}
		break;

	case SC_CRYSTALIZE:
		if( --(sce->val4) >= 0 ) { // Drains 2% of HP and 1% of SP every seconds.
			if (!status_charge(bl, status->max_hp * 2 / 100, status->max_sp / 100))
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_FORCEOFVANGUARD:
		if( !status_charge(bl,0,24 - 4 * sce->val1) )
			break;
		sc_timer_next(10000 + tick);
		return 0;

	case SC_BANDING:
		if( status_charge(bl, 0, 7 - sce->val1) ) {
			sce->val2 = (sd ? skill_banding_count(sd) : 1);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_REFLECTDAMAGE:
		if( --(sce->val4) > 0 ) {
			if( !status_charge(bl,0,10) )
 				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_OVERHEAT_LIMITPOINT:
		if (--(sce->val1) >= 0) { // Cooling
			if (sce->val2 == 0) { // Flag the overheat limit once it has been met.
				static std::vector<int16> limit = { 150, 200, 280, 360, 450 };
				uint16 skill_lv = (sd ? cap_value(pc_checkskill(sd, NC_MAINFRAME), 0, (uint16)(limit.size() - 1)) : 0);

				if (sce->val1 > limit[skill_lv])
					sce->val2 = 1;
			} else {
				status_change_end(bl, SC_OVERHEAT);
				if (sce->val2 > 0)
					sc_start(bl, bl, SC_OVERHEAT, 100, sce->val1, 975);
			}
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_OVERHEAT: {
			uint32 damage = status->max_hp / 100; // Suggestion 1% each second

			if (damage >= status->hp)
				damage = status->hp - 1; // Do not kill, just keep you with 1 hp minimum
			map_freeblock_lock();
			status_zap(bl, damage, 0);
			sc_timer_next(975 + tick); // Tick is not 1000 to avoid desync with SC_OVERHEAT_LIMITPOINT.
			map_freeblock_unlock();
			return 0;
		}
		break;

	case SC_MAGNETICFIELD:
		if (--(sce->val4) >= 0) {
			struct block_list *src = map_id2bl(sce->val2);

			if (!src || (src && (status_isdead(*src) || src->m != bl->m)))
				break;
			map_freeblock_lock();
			if (!status_charge(bl, 0, 50))
				status_zap(bl, 0, status->sp);
			if (sc->getSCE(type))
				sc_timer_next(1000 + tick);
			map_freeblock_unlock();
			return 0;
		}
		break;

	case SC_INSPIRATION:
		if(--(sce->val4) >= 0) {
			if (!status_charge(bl, status->max_hp * (35 - 5 * sce->val1) / 1000, status->max_sp * (45 - 5 * sce->val1) / 1000))
				break;

			sc_timer_next(5000+tick);
			return 0;
		}
		break;

	case SC_SHIELDSPELL_HP:
		if( sce->val4 >= 0 && status->hp < status->max_hp ){
			status_heal( bl, status->max_hp * sce->val2 / 100, 0, 1 );
		}
		break;

	case SC_SHIELDSPELL_SP:
		if( sce->val4 >= 0 && status->sp < status->max_sp ){
			status_heal( bl, 0, status->max_sp * sce->val2 / 100, 1 );
		}
		break;

	case SC_TROPIC:
	case SC_CHILLY_AIR:
	case SC_WILD_STORM:
	case SC_UPHEAVAL:
	case SC_HEATER:
	case SC_COOLER:
	case SC_BLAST:
	case SC_CURSED_SOIL:
	case SC_PYROTECHNIC:
	case SC_AQUAPLAY:
	case SC_GUST:
	case SC_PETROLOGY:
	case SC_CIRCLE_OF_FIRE:
	case SC_FIRE_CLOAK:
	case SC_WATER_DROP:
	case SC_WATER_SCREEN:
	case SC_WIND_CURTAIN:
	case SC_WIND_STEP:
	case SC_STONE_SHIELD:
	case SC_SOLID_SKIN:
	case SC_FLAMETECHNIC:
	case SC_FLAMEARMOR:
	case SC_COLD_FORCE:
	case SC_CRYSTAL_ARMOR:
	case SC_GRACE_BREEZE:
	case SC_EYES_OF_STORM:
	case SC_EARTH_CARE:
	case SC_STRONG_PROTECTION:
	case SC_DEEP_POISONING:
	case SC_POISON_SHIELD:
		if( !status_charge(bl,0,sce->val2) ) {
			struct block_list *s_bl = battle_get_master(bl);
			if (bl->type == BL_ELEM)
				elemental_change_mode(BL_CAST(BL_ELEM, bl), EL_MODE_PASSIVE);
			if( s_bl )
				status_change_end(s_bl,static_cast<sc_type>(type+1));
			status_change_end(bl,type);
			break;
		}
		sc_timer_next(sce->val3 + tick);
		return 0;

	case SC_WATER_SCREEN_OPTION:
		status_heal(bl,1000,0,2);
		sc_timer_next(10000 + tick);
		return 0;

	case SC_TEARGAS:
		if( --(sce->val4) >= 0 ) {
			struct block_list *src = map_id2bl(sce->val3);
			int damage = sce->val2;

			map_freeblock_lock();
			clif_damage(*bl, *bl, tick, 0, 0, damage, 1, DMG_MULTI_HIT_ENDURE, 0, false);
			status_damage(src, bl, damage,0, 0, 1, 0);
			if( sc->getSCE(type) ) {
				sc_timer_next(2000 + tick);
			}
			map_freeblock_unlock();
			return 0;
		}
		break;
	case SC_TEARGAS_SOB:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl, ET_CRY);
			sc_timer_next(3000 + tick);
			return 0;
		}
		break;
	case SC_STOMACHACHE:
		if( --(sce->val4) >= 0 ) {
			status_charge(bl,0,sce->val2);	// Reduce 8 every 10 seconds.
			if( sd && !pc_issit(sd) ) { // Force to sit every 10 seconds.
				pc_setsit(sd);
				skill_sit(sd, true);
				clif_sitting(*bl);
			}
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;
	case SC_LEADERSHIP:
	case SC_GLORYWOUNDS:
	case SC_SOULCOLD:
	case SC_HAWKEYES:
		// They only end by status_change_end
		sc_timer_next(600000 + tick);
		return 0;
	case SC_MEIKYOUSISUI:
		if( --(sce->val4) >= 0 ) {
			status_heal(bl, status->max_hp * sce->val2 / 100, status->max_sp * sce->val3 / 100, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_KAGEMUSYA:
		if( --(sce->val4) >= 0 ) {
			if(!status_charge(bl, 0, 1)) break;
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_ANGRIFFS_MODUS:
		if(--(sce->val4) >= 0) { // Drain hp/sp
			if( !status_charge(bl,100,20) ) break;
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_CBC:
		if(--(sce->val4) >= 0) { // Drain hp/sp
			int hp=0;
			int sp = (status->max_sp * sce->val3) / 100;
			if(bl->type == BL_MOB) hp = sp*10;
			if( !status_charge(bl,hp,sp) )break;
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_FULL_THROTTLE:
		if( --(sce->val4) >= 0 ) {
			status_percent_damage(bl, bl, 0, sce->val2, false);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_REBOUND:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl, ET_SWEAT);
			sc_timer_next(2000 + tick);
			return 0;
		}
		break;
	case SC_KINGS_GRACE:
		if( --(sce->val4) >= 0 ) {
			status_percent_heal(bl, sce->val2, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_FRIGG_SONG:
		if( --(sce->val4) >= 0 ) {
			status_heal(bl, sce->val3, 0, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_C_MARKER:
		if( --(sce->val4) >= 0 ) {
			TBL_PC *caster = map_id2sd(sce->val2);
			if (!caster || caster->bl.m != bl->m) //End the SC if caster isn't in same map
				break;
			sc_timer_next(1000 + tick);
			clif_crimson_marker( *caster, *bl, false );
			return 0;
		}
		break;
	case SC_STEALTHFIELD_MASTER:
		if (--(sce->val4) >= 0) {
			if (!status_charge(bl, 0, status->max_sp * 3 / 100))
				break;
			sc_timer_next(sce->val3 + tick);
			return 0;
		}
		break;
	case SC_VACUUM_EXTREME:
		if (sce->val4 > 0) {
			// Only slide targets to center if they are standing still
			if (unit_bl2ud(bl)->walktimer == INVALID_TIMER) {
				uint16 x = sce->val3 >> 16, y = sce->val3 & 0xFFFF;

				if (distance_xy(x, y, bl->x, bl->y) <= skill_get_unit_range(SO_VACUUM_EXTREME, sce->val1) && unit_movepos(bl, x, y, 0, false)) {
					clif_slide(*bl, x, y);
					clif_fixpos( *bl );
				}
			}
			sc_timer_next(tick + sce->val4);
			sce->val4 = 0;
		}
		break;
	case SC_FIRE_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_FIRE)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_EARTH)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_WATER_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_WATER)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_FIRE)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_WIND_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_WIND)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_WATER)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_EARTH_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_EARTH)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_WIND)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;
	case SC_BITESCAR:
		if (--(sce->val4) >= 0) {
			status_percent_damage(bl, bl, -(sce->val2), 0, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_FRESHSHRIMP:
		if (--(sce->val4) >= 0) {
			status_heal(bl, sce->val2, 0, 0);
			sc_timer_next((10000 - ((sce->val1 - 1) * 1000)) + tick);
			return 0;
		}
		break;
	case SC_DORAM_BUF_01:
		if( sd && --(sce->val4) >= 0 ) {
			if( status->hp < status->max_hp )
				status_heal(bl, 10, 0, 2);
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;
	case SC_DORAM_BUF_02:
		if( sd && --(sce->val4) >= 0 ) {
			if( status->sp < status->max_sp )
				status_heal(bl, 0, 5, 2);
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;
	case SC_NEWMOON:
		if (--(sce->val4) >= 0) {
			if (!status_charge(bl, 0, 1))
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_CREATINGSTAR:
		if (--(sce->val4) >= 0) { // Needed to check who the caster is and what AoE is giving the status.
			struct block_list *star_caster = map_id2bl(sce->val2);
			struct skill_unit *star_aoe = (struct skill_unit *)map_id2bl(sce->val3);

			if (star_caster == nullptr || status_isdead(*star_caster) || star_caster->m != bl->m || star_aoe == nullptr)
				break;

			sc_timer_next(500 + tick);

			// Attack after timer to prevent errors
			skill_attack(BF_WEAPON, star_caster, &star_aoe->bl, bl, SJ_BOOKOFCREATINGSTAR, sce->val1, tick, 0);
			return 0;
		}
		break;
	case SC_SOULUNITY:
		if (--(sce->val4) >= 0) { // Needed to check the caster's location for the range check.
			struct block_list *unity_src = map_id2bl(sce->val2);

			if (!unity_src || status_isdead(*unity_src) || unity_src->m != bl->m || !check_distance_bl(bl, unity_src, 11))
				break;

			status_heal(bl, 150 * sce->val1, 0, 2);
			sc_timer_next(3000 + tick);
			return 0;
		}
		break;
	case SC_SOULCOLLECT:
		pc_addsoulball(sd, sce->val2);
		if (sd->soulball < sce->val2) {
			sc_timer_next(sce->val3 + tick);
			return 0;
		}
		break;
	case SC_HELPANGEL:
		if (--(sce->val4) >= 0) {
			status_heal(bl, 1000, 350, 0);	// Heal amount not displayed
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_BURNT:
		if( --(sce->val4) >= 0 ) {
			int damage = 2000;

			if( damage >= status->hp )
				damage = status->hp - 1;
			map_freeblock_lock();
			status_zap(bl,damage,0);
			if( sc->getSCE(type) ) {
				sc_timer_next(1000 + tick);
			}
			map_freeblock_unlock();
			return 0;
		}
		break;
	case SC_MEDIALE:
		if (--(sce->val4) >= 0) {
			clif_specialeffect(bl, 1808, AREA);
			skill_castend_nodamage_id(bl, bl, CD_MEDIALE_VOTUM, sce->val1, tick, 1);
			sc_timer_next(2000 + tick);
			return 0;
		}
		break;

	case SC_DANCING_KNIFE:
		if (--(sce->val4) >= 0) {
			skill_castend_nodamage_id(bl, bl, SHC_DANCING_KNIFE, sce->val1, tick, 1);
			sc_timer_next(300 + tick);
			return 0;
		}
		break;

	case SC_A_MACHINE:
		if (--(sce->val4) >= 0) {
			skill_castend_nodamage_id(bl, bl, MT_A_MACHINE, sce->val1, tick, 1);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_SERVANTWEAPON:
		if (sce->val4 >= 0) {
			if( sd && sd->servantball < MAX_SERVANTBALL ){
				pc_addservantball( *sd, MAX_SERVANTBALL );
			}
			interval = max(500, skill_get_time2(DK_SERVANTWEAPON, sce->val1));
			map_freeblock_lock();
			dounlock = true;
		}
		break;
	case SC_ABYSSFORCEWEAPON:
		if (sce->val4 >= 0) {
			if( sd && sd->abyssball < MAX_ABYSSBALL ){
				pc_addabyssball( *sd );
			}
			interval = max(500, skill_get_time2(ABC_FROM_THE_ABYSS, sce->val1));
			map_freeblock_lock();
			dounlock = true;
		}
		break;
	case SC_KILLING_AURA:
		if (sce->val4 >= 0)
			skill_castend_damage_id( bl, bl, NPC_KILLING_AURA, sce->val1, tick, 0 );
		break;
	case SC_INTENSIVE_AIM:
		if (!sc || !sc->getSCE(SC_INTENSIVE_AIM_COUNT))
			sce->val4 = 0;
		if (sce->val4 < 10) {
			sce->val4++;
			sc_start(bl, bl, SC_INTENSIVE_AIM_COUNT, 100, sce->val4, INFINITE_TICK);
		}
		sc_timer_next(500 + tick);
		return 0;
	}

	// If status has an interval and there is at least 100ms remaining time, wait for next interval
	if(interval > 0 && sc->getSCE(type) && sce->val4 >= 100) {
		sc_timer_next(min(sce->val4,interval)+tick);
		sce->val4 -= interval;
		if (dounlock)
			map_freeblock_unlock();
		return 0;
	}

	if (dounlock)
		map_freeblock_unlock();

	// Default for all non-handled control paths is to end the status
	return status_change_end( bl,type,tid );
}

/**
 * For each iteration of repetitive status
 * @param bl: Object [PC|MOB|HOM|MER|ELEM]
 * @param ap: va_list arguments (src, sce, type, tick)
 */
int status_change_timer_sub(struct block_list* bl, va_list ap)
{
	status_change* tsc;

	struct block_list* src = va_arg(ap,struct block_list*);
	struct status_change_entry* sce = va_arg(ap,struct status_change_entry*);
	enum sc_type type = (sc_type)va_arg(ap,int); // gcc: enum args get promoted to int
	t_tick tick = va_arg(ap,t_tick);

	if (status_isdead(*bl))
		return 0;

	tsc = status_get_sc(bl);

	switch( type ) {
	case SC_SIGHT: // Reveal hidden ennemy on 3*3 range
	case SC_CONCENTRATE:
		status_change_end(bl, SC_HIDING);
		status_change_end(bl, SC_CLOAKING);
		status_change_end(bl, SC_CLOAKINGEXCEED);
		status_change_end(bl, SC_CAMOUFLAGE);
		status_change_end(bl, SC_NEWMOON);
		if (tsc && tsc->getSCE(SC__SHADOWFORM) && (sce && sce->val4 > 0 && sce->val4%2000 == 0) && // For every 2 seconds do the checking
			rnd()%100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM);
		break;
	case SC_RUWACH: // Reveal hidden target and deal little dammages if enemy
		if (tsc && (tsc->getSCE(SC_HIDING) || tsc->getSCE(SC_CLOAKING) ||
				tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_NEWMOON) || tsc->getSCE(SC_CLOAKINGEXCEED))) {
			status_change_end(bl, SC_HIDING);
			status_change_end(bl, SC_CLOAKING);
			status_change_end(bl, SC_CAMOUFLAGE);
			status_change_end(bl, SC_CLOAKINGEXCEED);
			status_change_end(bl, SC_NEWMOON);
			if(battle_check_target( src, bl, BCT_ENEMY ) > 0)
				skill_attack(BF_MAGIC,src,src,bl,AL_RUWACH,1,tick,0);
		}
		if (tsc && tsc->getSCE(SC__SHADOWFORM) && (sce && sce->val4 > 0 && sce->val4%2000 == 0) && // For every 2 seconds do the checking
			rnd()%100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10 ) { // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM);
				if (battle_check_target(src, bl, BCT_ENEMY) > 0)
					skill_attack(BF_MAGIC, src, src, bl, status_db.getSkill(type), 1, tick, 0);
		}
		break;
	case SC_SIGHTBLASTER:
		if (battle_check_target( src, bl, BCT_ENEMY ) > 0 &&
			status_check_skilluse(src, bl, WZ_SIGHTBLASTER, 2))
		{
			if (sce) {
				struct skill_unit *su = nullptr; 
				if(bl->type == BL_SKILL)
					su = (struct skill_unit *)bl;
				if (skill_attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,sce->val1,tick,0x1000000)
					&& (!su || !su->group || !skill_get_inf2(su->group->skill_id, INF2_ISTRAP))) { // The hit is not counted if it's against a trap
					sce->val2 = 0; // This signals it to end.
				} else if((bl->type&BL_SKILL) && sce->val4%2 == 0) {
					//Remove trap immunity temporarily so it triggers if you still stand on it
					sce->val4++;
				}
			}
		}
		break;
	case SC_CLOSECONFINE:
		// Lock char has released the hold on everyone...
		if (tsc && tsc->getSCE(SC_CLOSECONFINE2) && tsc->getSCE(SC_CLOSECONFINE2)->val2 == src->id) {
			tsc->getSCE(SC_CLOSECONFINE2)->val2 = 0;
			status_change_end(bl, SC_CLOSECONFINE2);
		}
		break;
	case SC_CURSEDCIRCLE_TARGET:
		if( tsc && tsc->getSCE(SC_CURSEDCIRCLE_TARGET) && tsc->getSCE(SC_CURSEDCIRCLE_TARGET)->val2 == src->id ) {
			clif_bladestop(bl, tsc->getSCE(SC_CURSEDCIRCLE_TARGET)->val2, 0);
			status_change_end(bl, type);
		}
		break;
	}

	return 0;
}

/**
 * Clears buffs/debuffs on an object
 * @param bl: Object to clear [PC|MOB|HOM|MER|ELEM]
 * @param type: Type to remove
 *  SCCB_BUFFS: Clear Buffs
 *  SCCB_DEBUFFS: Clear Debuffs
 *  SCCB_REFRESH: Clear specific debuffs through RK_REFRESH
 *  SCCB_CHEM_PROTECT: Clear AM_CP_ARMOR/HELM/SHIELD/WEAPON
 *  SCCB_LUXANIMA: Bonus Script removed through RK_LUXANIMA
 */
void status_change_clear_buffs(struct block_list* bl, uint8 type)
{
	status_change *sc= status_get_sc(bl);

	if (!sc || !sc->count)
		return;

	//Clears buffs with specified flag and type
	for (const auto &it : status_db) {
		sc_type status = static_cast<sc_type>(it.first);
		const std::bitset<SCF_MAX>& flag = it.second->flag;
		bool end = false;
		if (!sc->getSCE(status))
			continue;
		// Skip status with SCF_NOCLEARBUFF, no matter what
		if (flag[SCF_NOCLEARBUFF])
			continue;
		// &SCCB_LUXANIMA : Cleared by RK_LUXANIMA and has the SCF_REMOVEONLUXANIMA flag
		if ((type & SCCB_LUXANIMA) && flag[SCF_REMOVEONLUXANIMA])
			end = true;
		// &SCCB_CHEM_PROTECT : Clears AM_CP_ARMOR/HELP/SHIELD/WEAPON
		else if ((type & SCCB_CHEM_PROTECT) && flag[SCF_REMOVECHEMICALPROTECT])
			end = true;
		// &SCCB_REFRESH : Cleared by RK_REFRESH and has the SCF_REMOVEONREFRESH flag
		else if ((type & SCCB_REFRESH) && flag[SCF_REMOVEONREFRESH])
			end = true;
		// &SCCB_DEBUFFS : Clears debuffs
		else if ((type & SCCB_DEBUFFS) && flag[SCF_DEBUFF])
			end = true;
		// &SCCB_BUFFS : Clears buffs - skip if it is a debuff
		else if ((type & SCCB_BUFFS) && !flag[SCF_DEBUFF])
			end = true;
		// &SCCB_HERMODE : Cleared by CG_HERMODE and has the SCF_REMOVEONHERMODE flag
		else if ((type & SCCB_HERMODE) && flag[SCF_REMOVEONHERMODE])
			end = true;
		if (status == SC_SATURDAYNIGHTFEVER || status == SC_BERSERK) // Mark to not lose HP
			sc->getSCE(status)->val2 = 0;
		if(end)
			status_change_end(bl, status);
	}

	//Removes bonus_script
	if (bl->type == BL_PC) {
		uint32 i = 0;

		if (type&SCCB_BUFFS)    i |= BSF_REM_BUFF;
		if (type&SCCB_DEBUFFS)  i |= BSF_REM_DEBUFF;
		if (type&SCCB_REFRESH)  i |= BSF_REM_ON_REFRESH;
		if (type&SCCB_LUXANIMA) i |= BSF_REM_ON_LUXANIMA;
		pc_bonus_script_clear(BL_CAST(BL_PC,bl),i);
	}

	// Cleaning all extras vars
	sc->comet_x = 0;
	sc->comet_y = 0;
#ifndef RENEWAL
	sc->sg_counter = 0;
#endif

	return;
}

/**
 * Infect a user with status effects (SC_DEADLYINFECT)
 * @param src: Object initiating change on bl [PC|MOB|HOM|MER|ELEM]
 * @param bl: Object to change
 * @return 1: Success 0: Fail
 */
int status_change_spread(block_list *src, block_list *bl)
{
	if (src == nullptr || bl == nullptr)
		return 0;

	// Status Immunity resistance
	if (status_bl_has_mode(src, MD_STATUSIMMUNE) || status_bl_has_mode(bl, MD_STATUSIMMUNE))
		return 0;

	status_change *sc = status_get_sc(src);

	if (sc == nullptr || sc->count == 0)
		return 0;

	bool hasSpread = false;
	t_tick tick = gettick(), sc_tick;

	for (const auto &it : status_db) {
		sc_type type = static_cast<sc_type>(it.first);
		const TimerData *timer;

		if (sc->getSCE(type) && it.second->flag[SCF_SPREADEFFECT]) {
			if (sc->getSCE(type)->timer != INVALID_TIMER) {
				timer = get_timer(sc->getSCE(type)->timer);

				if (timer == nullptr || timer->func != status_change_timer || DIFF_TICK(timer->tick, tick) < 0)
					continue;

				int32 val4 = sc->getSCE(type)->val4;

				sc_tick = DIFF_TICK(timer->tick, tick) + (val4 > 0 ? val4 : 0);
			} else
				sc_tick = INFINITE_TICK;

			status_change_start(src, bl, type, 10000, sc->getSCE(type)->val1, sc->getSCE(type)->val2, sc->getSCE(type)->val3, sc->getSCE(type)->val4, sc_tick, SCSTART_NOAVOID | SCSTART_NOTICKDEF | SCSTART_NORATEDEF);

			if (!hasSpread)
				hasSpread = true;
		}
	}

	return hasSpread;
}

/**
 * Applying natural heal bonuses (sit, skill, homun, etc...)
 * TODO: the va_list doesn't seem to be used, safe to remove?
 * @param bl: Object applying bonuses to [PC|HOM|MER|ELEM]
 * @param args: va_list arguments
 * @return which regeneration bonuses have been applied (flag)
 */
static t_tick natural_heal_prev_tick,natural_heal_diff_tick;
static int status_natural_heal(struct block_list* bl, va_list args)
{
	struct regen_data *regen;
	status_change *sc;
	struct unit_data *ud;
	struct view_data *vd = nullptr;
	struct regen_data_sub *sregen;
	map_session_data *sd;
	int rate, multi = 1, flag;

	regen = status_get_regen_data(bl);
	if (!regen)
		return 0;

	status_data* status = status_get_status_data(*bl);

	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = nullptr;
	sd = BL_CAST(BL_PC,bl);

	flag = regen->flag;
	if (flag&RGN_HP && (regen->state.block&1))
		flag &= ~(RGN_HP|RGN_SHP);
	if (flag&RGN_SP && (regen->state.block&2))
		flag &= ~(RGN_SP|RGN_SSP);
	// Only skill-based regen is disabled at max HP/SP
	if (flag&RGN_SHP && (status->hp >= status->max_hp))
		flag &= ~RGN_SHP;
	if (flag&RGN_SSP && (status->sp >= status->max_sp))
		flag &= ~RGN_SSP;

	if (flag && (
		status_isdead(*bl) ||
		(sc && (sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || sc->getSCE(SC__INVISIBILITY)))
	))
		flag = RGN_NONE;

	if (sd) {
		if (sd->hp_loss.value || sd->sp_loss.value)
			pc_bleeding(sd, natural_heal_diff_tick);
		if (sd->hp_regen.value || sd->sp_regen.value || sd->percent_hp_regen.value || sd->percent_sp_regen.value)
			pc_regen(sd, natural_heal_diff_tick);
	}

	if(flag&(RGN_SHP|RGN_SSP) && regen->ssregen &&
		(vd = status_get_viewdata(bl)) && vd->dead_sit == 2)
	{ // Apply sitting regen bonus.
		sregen = regen->ssregen;
		if(flag&(RGN_SHP)) { // Sitting HP regen
			rate = (int)(natural_heal_diff_tick * (sregen->rate.hp / 100.));
			if (regen->state.overweight)
				rate /= 2; // Half as fast when overweight.
			sregen->tick.hp += rate;
			while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval) {
				sregen->tick.hp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp) { // Full
					flag &= ~RGN_SHP;
					break;
				}
			}
		}
		if(flag&(RGN_SSP)) { // Sitting SP regen
			rate = (int)(natural_heal_diff_tick * (sregen->rate.sp / 100.));
			if (regen->state.overweight)
				rate /= 2; // Half as fast when overweight.
			sregen->tick.sp += rate;
			while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval) {
				sregen->tick.sp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, 0, sregen->sp, 3) < sregen->sp) { // Full
					flag &= ~RGN_SSP;
					break;
				}
			}
		}
	}

	if (flag && regen->state.overweight)
		flag = RGN_NONE;

	ud = unit_bl2ud(bl);

	if (ud && ud->walktimer != INVALID_TIMER) {
		flag &= ~(RGN_SHP|RGN_SSP);
		//Mercenaries recover HP even while walking
		if(bl->type != BL_MER && !regen->state.walk)
			flag &= ~RGN_HP;
		//Homunculus don't recover SP while walking
		if (bl->type == BL_HOM && !regen->state.walk)
			flag &= ~RGN_SP;
	}

	if (flag&(RGN_HP|RGN_SP)) {
		if(!vd)
			vd = status_get_viewdata(bl);
		if(vd && vd->dead_sit == 2)
			multi += 1; //This causes the interval to be halved
		if(regen->state.gc)
			multi += 1; //This causes the interval to be halved
	}

	// Natural Hp regen
	if (flag&RGN_HP) {
		// Interval to next recovery tick
		rate = (int)(battle_config.natural_healhp_interval / (regen->rate.hp/100. * multi));
		// Half recovery while moving only applies to players with certain traits
		if (sd && ud && ud->walktimer != INVALID_TIMER)
			rate *= 2;
		// Homun HP regen fix (2 seconds instead of 6 seconds)
		if(bl->type == BL_HOM)
			rate /= 3;
		// Mercenary HP regen fix (8 seconds instead of 6 seconds)
		if (bl->type == BL_MER)
			rate = (rate * 4) / 3;

		// Our timer system isn't 100% accurate so make sure we use the closest interval
		rate -= NATURAL_HEAL_INTERVAL / 2;

		if(regen->tick.hp + rate <= natural_heal_prev_tick) {
			regen->tick.hp = natural_heal_prev_tick;
			if (status->hp >= status->max_hp)
				flag &= ~(RGN_HP | RGN_SHP);
			else if (status_heal(bl, regen->hp, 0, 1) < regen->hp)
				flag &= ~RGN_SHP; // Full
		}
	}
	else {
		regen->tick.hp = natural_heal_prev_tick;
	}

	// Natural SP regen
	if(flag&RGN_SP) {
		// Interval to next recovery tick
		rate = (int)(battle_config.natural_healsp_interval / (regen->rate.sp/100. * multi));
		// Homun SP regen fix (4 seconds instead of 8 seconds)
		if(bl->type==BL_HOM)
			rate /= 2;
		// Mercenary SP regen fix (6 seconds instead of 8 seconds)
		if (bl->type == BL_MER)
			rate = (rate * 3) / 4;
#ifdef RENEWAL
		if (sd && (sd->class_&MAPID_UPPERMASK) == MAPID_MONK &&
			sc && sc->getSCE(SC_EXPLOSIONSPIRITS) && (!sc->getSCE(SC_SPIRIT) || sc->getSCE(SC_SPIRIT)->val2 != SL_MONK))
			rate *= 2; // Tick is doubled in Fury state
#endif

		// Our timer system isn't 100% accurate so make sure we use the closest interval
		rate -= NATURAL_HEAL_INTERVAL / 2;

		if(regen->tick.sp + rate <= natural_heal_prev_tick) {
			regen->tick.sp = natural_heal_prev_tick;
			if (status->sp >= status->max_sp)
				flag &= ~(RGN_SP | RGN_SSP);
			else if (status_heal(bl, 0, regen->sp, 1) < regen->sp)
				flag &= ~RGN_SSP; // Full
		}
	}
	else {
		regen->tick.sp = natural_heal_prev_tick;
	}

	if (!regen->sregen)
		return flag;

	// Skill regen
	sregen = regen->sregen;

	if(flag&RGN_SHP) { // Skill HP regen
		sregen->tick.hp += (int)(natural_heal_diff_tick * (sregen->rate.hp / 100.));

		while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval) {
			sregen->tick.hp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp)
				break; // Full
		}
	}
	if(flag&RGN_SSP) { // Skill SP regen
		sregen->tick.sp += (int)(natural_heal_diff_tick * (sregen->rate.sp /100.));
		while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval) {
			int val = sregen->sp;
			if (sd && sd->state.doridori) {
				val *= 2;
				sd->state.doridori = 0;
				if ((rate = pc_checkskill(sd,TK_SPTIME)))
					sc_start(bl,bl,skill_get_sc(TK_SPTIME),
						100,rate,skill_get_time(TK_SPTIME, rate));
				if (
					(sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR &&
					rnd()%10000 < battle_config.sg_angel_skill_ratio
				) { // Angel of the Sun/Moon/Star
					clif_feel_hate_reset(sd);
					pc_resethate(sd);
					pc_resetfeel(sd);
				}
			}
			sregen->tick.sp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, 0, val, 3) < val)
				break; // Full
		}
	}
	return flag;
}

/**
 * Natural heal main timer
 * @param tid: Timer ID
 * @param tick: Current tick (time)
 * @param id: Object ID to heal
 * @param data: data pushed through timer function
 * @return 0
 */
static TIMER_FUNC(status_natural_heal_timer){
	natural_heal_diff_tick = DIFF_TICK(tick,natural_heal_prev_tick);
	natural_heal_prev_tick = tick;
	map_foreachregen(status_natural_heal);
	return 0;
}

/**
 * Clears the lastEffect value from a target
 * @param tid: Timer ID
 * @param tick: Current tick (time)
 * @param id: Object ID
 * @param data: data pushed through timer function
 * @return 0
 */
TIMER_FUNC(status_clear_lastEffect_timer) {
	block_list *bl = map_id2bl(id);

	if (bl != nullptr) {
		status_change *sc = status_get_sc(bl);

		if (sc != nullptr) {
			sc->lastEffect = SC_NONE;
			sc->lastEffectTimer = INVALID_TIMER;
		}
	}

	return 0;
}

/**
 * Check if status is disabled on a map
 * @param type: Status Change data
 * @param mapIsVS: If the map is a map_flag_vs type
 * @param mapisPVP: If the map is a PvP type
 * @param mapIsGVG: If the map is a map_flag_gvg type
 * @param mapIsBG: If the map is a Battleground type
 * @param mapZone: Map Zone type
 * @param mapIsTE: If the map us WOE TE
 * @return True - SC disabled on map; False - SC not disabled on map/Invalid SC
 */
static bool status_change_isDisabledOnMap_(sc_type type, bool mapIsVS, bool mapIsPVP, bool mapIsGVG, bool mapIsBG, unsigned int mapZone, bool mapIsTE)
{
	if (!status_db.validateStatus(type))
		return false;

	if ((!mapIsVS && SCDisabled[type]&1) ||
		(mapIsPVP && SCDisabled[type]&2) ||
		(mapIsGVG && SCDisabled[type]&4) ||
		(mapIsBG && SCDisabled[type]&8) ||
		(mapIsTE && SCDisabled[type]&16) ||
		(SCDisabled[type]&(mapZone)))
	{
		return true;
	}

	return false;
}

/**
 * Clear a status if it is disabled on a map
 * @param bl: Block list data
 * @param sc: Status Change data
 */
void status_change_clear_onChangeMap(struct block_list *bl, status_change *sc)
{
	nullpo_retv(bl);

	if (sc && sc->count) {
		struct map_data *mapdata = map_getmapdata(bl->m);
		bool mapIsVS = mapdata_flag_vs2(mapdata);
		bool mapIsPVP = mapdata->getMapFlag(MF_PVP) != 0;
		bool mapIsGVG = mapdata_flag_gvg2_no_te(mapdata);
		bool mapIsBG = mapdata->getMapFlag(MF_BATTLEGROUND) != 0;
		bool mapIsTE = mapdata_flag_gvg2_te(mapdata);

		for (const auto &it : status_db) {
			sc_type type = static_cast<sc_type>(it.first);

			if (!sc->getSCE(type) || !SCDisabled[type])
				continue;

			if (status_change_isDisabledOnMap_(type, mapIsVS, mapIsPVP, mapIsGVG, mapIsBG, mapdata->zone, mapIsTE))
				status_change_end(bl, type);
		}
	}
}

/**
 * Read status_disabled.txt file
 * @param str: Fields passed from sv_readdb
 * @param columns: Columns passed from sv_readdb function call
 * @param current: Current row being read into SCDisabled array
 * @return True - Successfully stored, False - Invalid SC
 */
static bool status_readdb_status_disabled( char **str, size_t columns, size_t current ){
	int64 type = SC_NONE;

	if (ISDIGIT(str[0][0]))
		type = atoi(str[0]);
	else {
		if (!script_get_constant(str[0],&type))
			type = SC_NONE;
	}

	if (type <= SC_NONE || type >= SC_MAX) {
		ShowError("status_readdb_status_disabled: Invalid SC with type %s.\n", str[0]);
		return false;
	}

	SCDisabled[type] = (unsigned int)atol(str[1]);
	return true;
}

const std::string AttributeDatabase::getDefaultLocation() {
	return std::string(db_path) + "/attr_fix.yml";
}

/**
 * Reads and parses an entry from the attr_fix.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 AttributeDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint16 level;

	if (!this->asUInt16(node, "Level", level))
		return 0;

	if (!CHK_ELEMENT_LEVEL(level)) {
		this->invalidWarning(node["Level"], "Invalid element level %hu.\n", level);
		return 0;
	}

	for (const auto &itatk : um_eleid2elename) {
		if (!this->nodeExists(node, itatk.first))
			continue;

		const auto& eleNode = node[c4::to_csubstr(itatk.first)];

		for (const auto &itdef : um_eleid2elename) {
			if (!this->nodeExists(eleNode, itdef.first))
				continue;

			int16 val;

			if (!this->asInt16(eleNode, itdef.first, val))
				return 0;

			if (val < -100) {
				this->invalidWarning(eleNode[c4::to_csubstr(itdef.first)], "%s %h is out of range %d~%d. Setting to -100.\n", itdef.first.c_str(), val, -100, 200);
				val = -100;
			}
			else if (val > 200) {
				this->invalidWarning(eleNode[c4::to_csubstr(itdef.first)], "%s %h is out of range %d~%d. Setting to 200.\n", itdef.first.c_str(), val, -100, 200);
				val = 200;
			}

			this->attr_fix_table[level-1][itatk.second][itdef.second] = val;
		}
	}

	return 1;
}

AttributeDatabase elemental_attribute_db;

/**
 * Get attribute ratio
 * @param atk_ele Attack element enum e_element
 * @param def_ele Defense element enum e_element
 * @param level Element level 1 ~ MAX_ELE_LEVEL
 */
int16 AttributeDatabase::getAttribute(uint16 level, uint16 atk_ele, uint16 def_ele) {
	if (!CHK_ELEMENT(atk_ele) || !CHK_ELEMENT(def_ele) || !CHK_ELEMENT_LEVEL(level))
		return 100;

	return this->attr_fix_table[level-1][atk_ele][def_ele];
}

const std::string StatusDatabase::getDefaultLocation() {
	return std::string(db_path) + "/status.yml";
}

/**
 * Reads and parses an entry from status_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 StatusDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string status_name;

	if (!this->asString(node, "Status", status_name))
		return 0;

	std::string status_constant = "SC_" + status_name;
	int64 constant;

	if (!script_get_constant(status_constant.c_str(), &constant)) {
		this->invalidWarning(node["Status"], "Invalid Status %s.\n", status_name.c_str());
		return 0;
	}

	if (!this->validateStatus(static_cast<sc_type>(constant))) {
		this->invalidWarning(node["Status"], "Status %s is out of bounds.\n", status_name.c_str());
		return 0;
	}

	int status_id = static_cast<int32>(constant);
	std::shared_ptr<s_status_change_db> status = this->find(status_id);
	bool exists = status != nullptr;

	if (!exists) {
		status = std::make_shared<s_status_change_db>();
		status->type = static_cast<sc_type>(status_id);
	}

	if (this->nodeExists(node, "Icon")) {
		std::string icon_name;

		if (!this->asString(node, "Icon", icon_name))
			return 0;

		int64 constant;

		if (!script_get_constant(icon_name.c_str(), &constant)) {
			this->invalidWarning(node["Icon"], "Icon %s is invalid, defaulting to EFST_BLANK.\n", icon_name.c_str());
			constant = EFST_BLANK;
		}
		
		if (constant < EFST_BLANK || constant >= EFST_MAX) {
			this->invalidWarning(node["Icon"], "Icon %s is out of bounds, defaulting to EFST_BLANK.\n", icon_name.c_str());
			constant = EFST_BLANK;
		}

		status->icon = static_cast<efst_type>(constant);
	} else {
		if (!exists)
			status->icon = EFST_BLANK;
	}

	if (this->nodeExists(node, "DurationLookup")) {
		std::string skill_name;

		if (!this->asString(node, "DurationLookup", skill_name))
			return 0;

		uint16 skill_id = skill_name2id(skill_name.c_str());

		if (skill_id == 0)
			this->invalidWarning(node["DurationLookup"], "DurationLookup skill %s is invalid, defaulting to none.\n", skill_name.c_str());

		status->skill_id = skill_id;
	} else {
		if (!exists)
			status->skill_id = 0;
	}

	if (this->nodeExists(node, "States")) {
		const ryml::NodeRef& stateNode = node["States"];

		for (const auto &it : stateNode) {
			std::string state;
			c4::from_chars(it.key(), &state);

			std::string state_constant = "SCS_" + state;
			int64 constant;

			if (!script_get_constant(state_constant.c_str(), &constant)) {
				this->invalidWarning(stateNode, "State %s is invalid.\n", state.c_str());
				return 0;
			}

			if (constant < SCS_NONE || constant >= SCS_MAX) {
				this->invalidWarning(stateNode, "State %s is out of bounds.\n", state.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(stateNode, state, active))
				return 0;

			if (active)
				status->state.set(static_cast<e_scs_flag>(constant));
			else
				status->state.reset(static_cast<e_scs_flag>(constant));
		}
	} else {
		if (!exists)
			status->state.reset();
	}

	if (this->nodeExists(node, "CalcFlags")) {
		const ryml::NodeRef& flagNode = node["CalcFlags"];

		if (this->nodeExists(flagNode, "All")) {
			bool active;

			if (!this->asBool(flagNode, "All", active))
				return 0;

			if (active)
				status->calc_flag = this->getSCB_ALL();
			else
				status->calc_flag.reset();
		}

		for (const auto &it : flagNode) {
			std::string flag;
			c4::from_chars(it.key(), &flag);

			std::string flag_constant = "SCB_" + flag;
			int64 constant;

			// Skipped because processed above the loop
			if (flag.compare("All") == 0)
				continue;

			if (!script_get_constant(flag_constant.c_str(), &constant)) {
				this->invalidWarning(flagNode, "CalcFlag %s is invalid.\n", flag.c_str());
				return 0;
			}

			if (constant < SCB_NONE || constant >= SCB_MAX) {
				this->invalidWarning(flagNode, "CalcFlag %s is out of bounds.\n", flag.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(flagNode, flag, active))
				return 0;

			if (active)
				status->calc_flag.set(static_cast<e_scb_flag>(constant));
			else
				status->calc_flag.reset(static_cast<e_scb_flag>(constant));
		}
	} else {
		if (!exists)
			status->calc_flag.reset();
	}

	if (this->nodeExists(node, "Opt1")) {
		std::string opt;

		if (!this->asString(node, "Opt1", opt))
			return 0;

		std::string opt_constant = "OPT1_" + opt;
		int64 constant;

		if (!script_get_constant(opt_constant.c_str(), &constant)) {
			this->invalidWarning(node["Opt1"], "Opt1 %s is invalid.\n", opt.c_str());
			return 0;
		}

		if (constant < OPT1_NONE || constant >= OPT1_MAX) {
			this->invalidWarning(node["Opt1"], "Opt2 %s is out of bounds.\n", opt.c_str());
			return 0;
		}

		status->opt1 = static_cast<e_sc_opt1>(constant);
	} else {
		if (!exists)
			status->opt1 = OPT1_NONE;
	}

	if (this->nodeExists(node, "Opt2")) {
		const ryml::NodeRef& optNode = node["Opt2"];

		for (const auto &it : optNode) {
			std::string opt;
			c4::from_chars(it.key(), &opt);

			std::string opt_constant = "OPT2_" + opt;
			int64 constant;

			if (!script_get_constant(opt_constant.c_str(), &constant)) {
				this->invalidWarning(optNode, "Opt2 %s is invalid.\n", opt.c_str());
				return 0;
			}

			if (constant < OPT2_NONE || constant >= OPT2_MAX) {
				this->invalidWarning(optNode, "Opt2 %s is out of bounds.\n", opt.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(optNode, opt, active))
				return 0;

			if (active)
				status->opt2 |= static_cast<e_sc_opt2>(constant);
			else
				status->opt2 &= ~static_cast<e_sc_opt2>(constant);
		}
	} else {
		if (!exists)
			status->opt2 = OPT2_NONE;
	}

	if (this->nodeExists(node, "Opt3")) {
		const ryml::NodeRef& optNode = node["Opt3"];

		for (const auto &it : optNode) {
			std::string opt;
			c4::from_chars(it.key(), &opt);

			std::string opt_constant = "OPT3_" + opt;
			int64 constant;

			if (!script_get_constant(opt_constant.c_str(), &constant)) {
				this->invalidWarning(optNode, "Opt3 %s is invalid.\n", opt.c_str());
				return 0;
			}

			if (constant < OPT3_NORMAL || constant >= OPT3_MAX) {
				this->invalidWarning(optNode, "Opt3 %s is out of bounds.\n", opt.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(optNode, opt, active))
				return 0;

			if (active)
				status->opt3 |= static_cast<e_sc_opt3>(constant);
			else
				status->opt3 &= ~static_cast<e_sc_opt3>(constant);
		}
	} else {
		if (!exists)
			status->opt3 = OPT3_NORMAL;
	}

	if (this->nodeExists(node, "Options")) {
		const ryml::NodeRef& optionNode = node["Options"];

		for (const auto &it : optionNode) {
			std::string option;
			c4::from_chars(it.key(), &option);

			std::string option_constant = "OPTION_" + option;
			int64 constant;

			if (!script_get_constant(option_constant.c_str(), &constant)) {
				this->invalidWarning(optionNode, "Option %s is invalid.\n", option.c_str());
				return 0;
			}

			if (constant < OPTION_NOTHING || constant >= OPTION_MAX) {
				this->invalidWarning(optionNode, "Option %s is out of bounds.\n", option.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(optionNode, option, active))
				return 0;

			if (active)
				status->look |= static_cast<e_option>(constant);
			else
				status->look &= ~static_cast<e_option>(constant);
		}
	} else {
		if (!exists)
			status->look = OPTION_NOTHING;
	}

	if (this->nodeExists(node, "Flags")) {
		const ryml::NodeRef& flagNode = node["Flags"];

		for (const auto &it : flagNode) {
			std::string flag;
			c4::from_chars(it.key(), &flag);

			std::string flag_constant = "SCF_" + flag;
			int64 constant;

			if (!script_get_constant(flag_constant.c_str(), &constant)) {
				this->invalidWarning(flagNode, "Flag %s is invalid.\n", flag.c_str());
				return 0;
			}

			if (constant < SCF_NONE || constant >= SCF_MAX) {
				this->invalidWarning(flagNode, "Flag %s is out of bounds.\n", flag.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(flagNode, flag, active))
				return 0;

			if (active)
				status->flag.set(static_cast<e_status_change_flag>(constant));
			else
				status->flag.reset(static_cast<e_status_change_flag>(constant));
		}
	} else {
		if (!exists)
			status->flag.reset();
	}

	if (this->nodeExists(node, "MinRate")) {
		uint16 rate;

		if (!this->asUInt16(node, "MinRate", rate))
			return 0;

		status->min_rate = rate;
	} else {
		if (!exists)
			status->min_rate = 0;
	}
	
	if (this->nodeExists(node, "MinDuration")) {
		int64 duration;

		if (!this->asInt64(node, "MinDuration", duration))
			return 0;

		status->min_duration = static_cast<t_tick>(duration);
	} else {
		if (!exists)
			status->min_duration = 1;
	}

	if (this->nodeExists(node, "Fail")) {
		const ryml::NodeRef& failNode = node["Fail"];

		for (const auto &it : failNode) {
			std::string fail;
			c4::from_chars(it.key(), &fail);

			std::string fail_constant = "SC_" + fail;
			int64 constant;

			if (!script_get_constant(fail_constant.c_str(), &constant)) {
				this->invalidWarning(failNode, "Fail status %s is invalid.\n", fail.c_str());
				return 0;
			}

			if (!this->validateStatus(static_cast<sc_type>(constant))) {
				this->invalidWarning(failNode, "Fail status %s is out of bounds.\n", fail.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(failNode, fail, active))
				return 0;

			if (active)
				status->fail.push_back(static_cast<sc_type>(constant));
			else
				util::vector_erase_if_exists(status->fail, static_cast<sc_type>(constant));
		}
	}

	if (this->nodeExists(node, "EndOnStart")) {
		const ryml::NodeRef& endNode = node["EndOnStart"];

		for (const auto &it : endNode) {
			std::string end;
			c4::from_chars(it.key(), &end);

			std::string end_constant = "SC_" + end;
			int64 constant;

			if (!script_get_constant(end_constant.c_str(), &constant)) {
				this->invalidWarning(endNode, "EndOnStart status %s is invalid.\n", end.c_str());
				return 0;
			}

			if (!this->validateStatus(static_cast<sc_type>(constant))) {
				this->invalidWarning(endNode, "EndOnStart status %s is out of bounds.\n", end.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(endNode, end, active))
				return 0;

			if (active)
				status->endonstart.push_back(static_cast<sc_type>(constant));
			else
				util::vector_erase_if_exists(status->endonstart, static_cast<sc_type>(constant));
		}
	}

	if (this->nodeExists(node, "EndReturn")) {
		const ryml::NodeRef &endNode = node["EndReturn"];

		for (const auto &it : endNode) {
			std::string end;
			c4::from_chars(it.key(), &end);

			std::string end_constant = "SC_" + end;
			int64 constant;

			if (!script_get_constant(end_constant.c_str(), &constant)) {
				this->invalidWarning(endNode, "EndReturn status %s is invalid.\n", end.c_str());
				return 0;
			}

			if (!this->validateStatus(static_cast<sc_type>(constant))) {
				this->invalidWarning(endNode, "EndReturn status %s is out of bounds.\n", end.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(endNode, end, active))
				return 0;

			if (active)
				status->endreturn.push_back(static_cast<sc_type>(constant));
			else
				util::vector_erase_if_exists(status->endreturn, static_cast<sc_type>(constant));
		}
	}

	if (this->nodeExists(node, "EndOnEnd")) {
		const ryml::NodeRef &endNode = node["EndOnEnd"];

		for (const auto &it : endNode) {
			std::string end;
			c4::from_chars(it.key(), &end);

			std::string end_constant = "SC_" + end;
			int64 constant;

			if (!script_get_constant(end_constant.c_str(), &constant)) {
				this->invalidWarning(endNode, "EndOnEnd status %s is invalid.\n", end.c_str());
				return 0;
			}

			if (!this->validateStatus(static_cast<sc_type>(constant))) {
				this->invalidWarning(endNode, "EndOnEnd status %s is out of bounds.\n", end.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(endNode, end, active))
				return 0;

			if (active)
				status->endonend.push_back(static_cast<sc_type>(constant));
			else
				util::vector_erase_if_exists(status->endonend, static_cast<sc_type>(constant));
		}
	}

	if (!exists) {
		this->put(status_id, status);
	}

	return 1;
}

void StatusDatabase::loadingFinished(){
	std::fill( std::begin( this->StatusRelevantBLTypes ), std::end( this->StatusRelevantBLTypes ), BL_PC );

	for( auto& entry : *this ){
		auto& status = entry.second;

		if (status->type == SC_HALLUCINATION && !battle_config.display_hallucination) // Disable Hallucination.
			status->icon = EFST_BLANK;

		if( status->icon == EFST_BLANK ){
			continue;
		}else if( status->flag[SCF_BLEFFECT] ){
			this->StatusRelevantBLTypes[status->icon] |= BL_SCEFFECT;
		}else{
			this->StatusRelevantBLTypes[status->icon] = BL_PC;
		}
	}

	TypesafeCachedYamlDatabase::loadingFinished();
}

StatusDatabase status_db;

/**
 * Sets defaults in tables and starts read db functions
 * sv_readdb reads the file, outputting the information line-by-line to
 * previous functions above, separating information by delimiter
 * DBs being read:
 *	attr_fix.yml: Attribute adjustment table for attacks
 *	size_fix.yml: Size adjustment table for weapons
 *	refine.yml: Refining data table
 * @return 0
 */
void status_readdb( bool reload ){
	int i;
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
		//add other path here
	};

	// read databases
	// path,filename,separator,mincol,maxcol,maxrow,func_parsor
	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		size_t n1 = strlen(db_path)+strlen(dbsubpath[i])+1;
		size_t n2 = strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1;
		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);

		if(i==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		}
		else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}

		sv_readdb(dbsubpath1, "status_disabled.txt", ',', 2, 2, -1, &status_readdb_status_disabled, i > 0);

		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}

	if( reload ){
		size_fix_db.reload();
		refine_db.reload();
		status_db.reload();
		enchantgrade_db.reload();
	}else{
		size_fix_db.load();
		refine_db.load();
		status_db.load();
		enchantgrade_db.load();
	}
	elemental_attribute_db.load();
}

/**
 * Status db init and destroy.
 */
void do_init_status(void) {
	memset(SCDisabled, 0, sizeof(SCDisabled));

	add_timer_func_list(status_change_timer,"status_change_timer");
	add_timer_func_list(status_natural_heal_timer,"status_natural_heal_timer");
	add_timer_func_list(status_clear_lastEffect_timer, "status_clear_lastEffect_timer");
	initDummyData();
	status_readdb();
	natural_heal_prev_tick = gettick();
	sc_data_ers = ers_new(sizeof(struct status_change_entry),"status.cpp::sc_data_ers",ERS_OPT_NONE);
	add_timer_interval(natural_heal_prev_tick + NATURAL_HEAL_INTERVAL, status_natural_heal_timer, 0, 0, NATURAL_HEAL_INTERVAL);
}

/** Destroy status data */
void do_final_status(void) {
	ers_destroy(sc_data_ers);
	enchantgrade_db.clear();
	size_fix_db.clear();
	refine_db.clear();
	status_db.clear();
	elemental_attribute_db.clear();
}
