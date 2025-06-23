// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itemdb.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>

#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/utils.hpp>
#include <common/utilities.hpp>

#include "battle.hpp" // struct battle_config
#include "cashshop.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "mob.hpp"
#include "pc.hpp"
#include "status.hpp"

using namespace rathena;


ComboDatabase itemdb_combo;
ItemGroupDatabase itemdb_group;

struct s_roulette_db rd;

static void itemdb_jobid2mapid(uint64 bclass[3], e_mapid jobmask, bool active);

const std::string ItemDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_db.yml";
}

/**
 * Reads and parses an entry from the item_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 ItemDatabase::parseBodyNode(const ryml::NodeRef& node) {
	t_itemid nameid;

	if (!this->asUInt32(node, "Id", nameid))
		return 0;

	std::shared_ptr<item_data> item = this->find(nameid);
	bool exists = item != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "AegisName", "Name" }))
			return 0;

		item = std::make_shared<item_data>();
		item->nameid = nameid;
		item->flag.available = true;
	}

	if (this->nodeExists(node, "AegisName")) {
		std::string name;

		if (!this->asString(node, "AegisName", name))
			return 0;

		if (name.length() > ITEM_NAME_LENGTH) {
			this->invalidWarning(node["AegisName"], "AegisName \"%s\" exceeds maximum of %d characters, capping...\n", name.c_str(), ITEM_NAME_LENGTH - 1);
		}

		std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

		if (id != nullptr && id->nameid != nameid) {
			this->invalidWarning(node["AegisName"], "Found duplicate item Aegis name for %s, skipping.\n", name.c_str());
			return 0;
		}

		if( exists ){
			// Create a copy
			std::string aegisname = item->name;
			// Convert it to lower
			util::tolower( aegisname );
			// Remove old AEGIS name from lookup
			this->aegisNameToItemDataMap.erase( aegisname );
		}

		item->name.resize(ITEM_NAME_LENGTH);
		item->name = name.c_str();

		// Create a copy
		std::string aegisname = name;
		// Convert it to lower
		util::tolower( aegisname );

		this->aegisNameToItemDataMap[aegisname] = item;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		if (name.length() > ITEM_NAME_LENGTH) {
			this->invalidWarning(node["Name"], "Name \"%s\" exceeds maximum of %d characters, capping...\n", name.c_str(), ITEM_NAME_LENGTH - 1);
		}

		if( exists ){
			// Create a copy
			std::string ename = item->ename;
			// Convert it to lower
			util::tolower( ename );
			// Remove old name from lookup
			this->nameToItemDataMap.erase( ename );
		}

		item->ename.resize(ITEM_NAME_LENGTH);
		item->ename = name.c_str();

		// Create a copy
		std::string ename = name;
		// Convert it to lower
		util::tolower( ename );

		this->nameToItemDataMap[ename] = item;
	}

	if (this->nodeExists(node, "Type")) {
		std::string type;

		if (!this->asString(node, "Type", type))
			return 0;

		std::string type_constant = "IT_" + type;
		int64 constant;

		if (!script_get_constant(type_constant.c_str(), &constant) || constant < IT_HEALING || constant >= IT_MAX) {
			this->invalidWarning(node["Type"], "Invalid item type %s, defaulting to IT_ETC.\n", type.c_str());
			constant = IT_ETC;
		}

		item->type = static_cast<item_types>(constant);
	} else {
		if (!exists)
			item->type = IT_ETC;
	}

	if (this->nodeExists(node, "SubType")) {
		std::string type;

		if (!this->asString(node, "SubType", type))
			return 0;

		if (item->type == IT_WEAPON) {
			std::string type_constant = "W_" + type;
			int64 constant;

			if (!script_get_constant(type_constant.c_str(), &constant) || constant < W_FIST || constant >= MAX_WEAPON_TYPE) {
				this->invalidWarning(node["SubType"], "Invalid weapon type %s, defaulting to W_FIST.\n", type.c_str());
				item->subtype = W_FIST;
			}

			item->subtype = static_cast<weapon_type>(constant);
		} else if (item->type == IT_AMMO) {
			std::string type_constant = "AMMO_" + type;
			int64 constant;

			if (!script_get_constant(type_constant.c_str(), &constant) || constant <= AMMO_NONE || constant >= MAX_AMMO_TYPE) {
				this->invalidWarning(node["SubType"], "Invalid ammo type %s, defaulting to AMMO_NONE.\n", type.c_str());
				item->subtype = AMMO_NONE;
			}

			item->subtype = static_cast<e_ammo_type>(constant);
		} else if (item->type == IT_CARD) {
			std::string type_constant = "CARD_" + type;
			int64 constant;

			if (!script_get_constant(type_constant.c_str(), &constant) || constant < CARD_NORMAL || constant >= MAX_CARD_TYPE) {
				this->invalidWarning(node["SubType"], "Invalid card type %s, defaulting to CARD_NORMAL.\n", type.c_str());
				item->subtype = CARD_NORMAL;
			}

			item->subtype = static_cast<e_card_type>(constant);
		} else {
			this->invalidWarning(node["SubType"], "Item sub type is not supported for this item type.\n");
			return 0;
		}
	} else {
		if (!exists)
			item->subtype = 0;
	}

	bool has_buy = false, has_sell = false;

	if (this->nodeExists(node, "Buy")) {
		uint32 buy;

		if (!this->asUInt32(node, "Buy", buy))
			return 0;

		if( buy > MAX_ZENY ){
			this->invalidWarning( node["Buy"], "Buying price exceeds MAX_ZENY. Capping...\n" );
			buy = MAX_ZENY;
		}

		has_buy = true;
		item->value_buy = buy;
	} else {
		if (!exists) {
			item->value_buy = 0;
		}
	}

	if (this->nodeExists(node, "Sell")) {
		uint32 sell;

		if (!this->asUInt32(node, "Sell", sell))
			return 0;

		if( sell > MAX_ZENY ){
			this->invalidWarning( node["Sell"], "Sell price exceeds MAX_ZENY. Capping...\n" );
			sell = MAX_ZENY;
		}

		has_sell = true;
		item->value_sell = sell;
	} else {
		if (!exists) {
			item->value_sell = 0;
		}
	}

	hasPriceValue[item->nameid] = { has_buy, has_sell };

	if (this->nodeExists(node, "Weight")) {
		uint32 weight;

		if (!this->asUInt32(node, "Weight", weight))
			return 0;

		item->weight = weight;
	} else {
		if (!exists)
			item->weight = 0;
	}

	if (this->nodeExists(node, "Attack")) {
		uint32 atk;

		if (!this->asUInt32(node, "Attack", atk))
			return 0;

		item->atk = atk;
	} else {
		if (!exists)
			item->atk = 0;
	}

#ifdef RENEWAL
	if (this->nodeExists(node, "MagicAttack")) {
		uint32 matk;

		if (!this->asUInt32(node, "MagicAttack", matk))
			return 0;

		item->matk = matk;
	} else {
		if (!exists)
			item->matk = 0;
	}
#endif

	if (this->nodeExists(node, "Defense")) {
		uint32 def;

		if (!this->asUInt32(node, "Defense", def))
			return 0;

		if (def > DEFTYPE_MAX) {
			this->invalidWarning(node["Defense"], "Item defense %d exceeds DEFTYPE_MAX (%d), capping to DEFTYPE_MAX.\n", def, DEFTYPE_MAX);
			def = DEFTYPE_MAX;
		}

		item->def = def;
	} else {
		if (!exists)
			item->def = 0;
	}

	if (this->nodeExists(node, "Range")) {
		uint16 range;

		if (!this->asUInt16(node, "Range", range))
			return 0;

		if (range > AREA_SIZE) {
			this->invalidWarning(node["Range"], "Item attack range %d exceeds AREA_SIZE (%d), capping to AREA_SIZE.\n", range, AREA_SIZE);
			range = AREA_SIZE;
		}

		item->range = range;
	} else {
		if (!exists)
			item->range = 0;
	}

	if (this->nodeExists(node, "Slots")) {
		uint16 slots;

		if (!this->asUInt16(node, "Slots", slots))
			return 0;

		if (slots > MAX_SLOTS) {
			this->invalidWarning(node["Slots"], "Item slots %d exceeds MAX_SLOTS (%d), capping to MAX_SLOTS.\n", slots, MAX_SLOTS);
			slots = MAX_SLOTS;
		}

		item->slots = slots;
	} else {
		if (!exists)
			item->slots = 0;
	}

	if (this->nodeExists(node, "Jobs")) {
		const ryml::NodeRef& jobNode = node["Jobs"];

		item->class_base[0] = item->class_base[1] = item->class_base[2] = 0;

		if (this->nodeExists(jobNode, "All")) {
			bool active;

			if (!this->asBool(jobNode, "All", active))
				return 0;

			itemdb_jobid2mapid(item->class_base, MAPID_ALL, active);
		}

		for (const auto& jobit : jobNode) {
			std::string jobName;
			c4::from_chars(jobit.key(), &jobName);

			// Skipped because processed above the loop
			if (jobName.compare("All") == 0)
				continue;

			std::string jobName_constant = "EAJ_" + jobName;
			int64 constant;

			if (!script_get_constant(jobName_constant.c_str(), &constant)) {
				this->invalidWarning(jobNode[jobit.key()], "Invalid item job %s, defaulting to All.\n", jobName.c_str());
				itemdb_jobid2mapid(item->class_base, MAPID_ALL, true);
				break;
			}

			bool active;

			if (!this->asBool(jobNode, jobName, active))
				return 0;

			itemdb_jobid2mapid(item->class_base, static_cast<e_mapid>(constant), active);
		}
	} else {
		if (!exists) {
			item->class_base[0] = item->class_base[1] = item->class_base[2] = 0;

			itemdb_jobid2mapid(item->class_base, MAPID_ALL, true);
		}
	}

	if (this->nodeExists(node, "Classes")) {
		const auto& classNode = node["Classes"];

		if (this->nodeExists(classNode, "All")) {
			bool active;

			if (!this->asBool(classNode, "All", active))
				return 0;

			if (active)
				item->class_upper |= ITEMJ_ALL;
			else
				item->class_upper &= ~ITEMJ_ALL;
		}

		for (const auto& classit : classNode) {
			std::string className;
			c4::from_chars(classit.key(), &className);

			// Skipped because processed above the loop
			if (className.compare("All") == 0)
				continue;

			std::string className_constant = "ITEMJ_" + className;
			int64 constant;

			if (!script_get_constant(className_constant.c_str(), &constant)) {
				this->invalidWarning(classNode[classit.key()], "Invalid class upper %s, defaulting to All.\n", className.c_str());
				item->class_upper |= ITEMJ_ALL;
				break;
			}

			bool active;

			if (!this->asBool(classNode, className, active))
				return 0;

			if (active)
				item->class_upper |= constant;
			else
				item->class_upper &= ~constant;
		}
	} else {
		if (!exists)
			item->class_upper = ITEMJ_ALL;
	}

	if (this->nodeExists(node, "Gender")) {
		std::string gender;

		if (!this->asString(node, "Gender", gender))
			return 0;

		std::string gender_constant = "SEX_" + gender;
		int64 constant;

		if (!script_get_constant(gender_constant.c_str(), &constant) || constant < SEX_FEMALE || constant > SEX_BOTH) {
			this->invalidWarning(node["Gender"], "Invalid item gender %s, defaulting to SEX_BOTH.\n", gender.c_str());
			constant = SEX_BOTH;
		}

		item->sex = static_cast<e_sex>(constant);
		item->sex = this->defaultGender( node, item );
	} else {
		if (!exists) {
			item->sex = SEX_BOTH;
			item->sex = this->defaultGender( node, item );
		}
	}

	if (this->nodeExists(node, "Locations")) {
		const auto& locationNode = node["Locations"];

		for (const auto& locit : locationNode) {
			std::string equipName;
			c4::from_chars(locit.key(), &equipName);

			std::string equipName_constant = "EQP_" + equipName;
			int64 constant;

			if (!script_get_constant(equipName_constant.c_str(), &constant)) {
				this->invalidWarning(locationNode[locit.key()], "Invalid equip location %s, defaulting to IT_ETC.\n", equipName.c_str());
				item->type = IT_ETC;
				break;
			}

			bool active;

			if (!this->asBool(locationNode, equipName, active))
				return 0;

			if (active) {
				if (constant & EQP_SHADOW_GEAR && item->type != IT_SHADOWGEAR) {
					this->invalidWarning(node, "Invalid item equip location %s as it's not a Shadow Gear item type, defaulting to IT_ETC.\n", equipName.c_str());
					item->type = IT_ETC;
				}

				item->equip |= constant;
			} else
				item->equip &= ~constant;
		}
	} else {
		if (!exists) {
			if (itemdb_isequip2(item.get())) {
				this->invalidWarning(node, "Invalid item equip location as it has no equip location, defaulting to IT_ETC.\n");
				item->type = IT_ETC;
			} else
				item->equip = 0;
		}
	}

	if (this->nodeExists(node, "WeaponLevel")) {
		uint16 lv;

		if (!this->asUInt16(node, "WeaponLevel", lv))
			return 0;

		if (lv > MAX_WEAPON_LEVEL) {
			this->invalidWarning(node["WeaponLevel"], "Invalid weapon level %d, defaulting to 0.\n", lv);
			lv = 0;
		}

		if (item->type != IT_WEAPON) {
			this->invalidWarning(node["WeaponLevel"], "Item type is not a weapon, defaulting to 0.\n");
			lv = 0;
		}

		item->weapon_level = lv;
	} else {
		if (!exists)
			item->weapon_level = 0;
	}

	if( this->nodeExists( node, "ArmorLevel" ) ){
		uint16 level;

		if( !this->asUInt16( node, "ArmorLevel", level ) ){
			return 0;
		}

		if( level > MAX_ARMOR_LEVEL ){
			this->invalidWarning( node["ArmorLevel"], "Invalid armor level %d, defaulting to 0.\n", level );
			level = 0;
		}

		if( item->type != IT_ARMOR ){
			this->invalidWarning( node["ArmorLevel"], "Item type is not an armor, defaulting to 0.\n" );
			level = 0;
		}

		item->armor_level = level;
	}else{
		if( !exists ){
			item->armor_level = 0;
		}
	}

	if (this->nodeExists(node, "EquipLevelMin")) {
		uint16 lv;

		if (!this->asUInt16(node, "EquipLevelMin", lv))
			return 0;

		if (lv > MAX_LEVEL) {
			this->invalidWarning(node["EquipLevelMin"], "Minimum equip level %d exceeds MAX_LEVEL (%d), capping to MAX_LEVEL.\n", lv, MAX_LEVEL);
			lv = MAX_LEVEL;
		}

		item->elv = lv;
	} else {
		if (!exists)
			item->elv = 0;
	}

	if (this->nodeExists(node, "EquipLevelMax")) {
		uint16 lv;

		if (!this->asUInt16(node, "EquipLevelMax", lv))
			return 0;

		if (lv < item->elv) {
			this->invalidWarning(node["EquipLevelMax"], "Maximum equip level %d is less than minimum equip level %d, capping to minimum equip level.\n", lv, item->elv);
			lv = item->elv;
		}

		if (lv > MAX_LEVEL) {
			this->invalidWarning(node["EquipLevelMax"], "Maximum equip level %d exceeds MAX_LEVEL (%d), capping to MAX_LEVEL.\n", lv, MAX_LEVEL);
			lv = MAX_LEVEL;
		}

		item->elvmax = lv;
	} else {
		if (!exists)
			item->elvmax = MAX_LEVEL;
	}

	if (this->nodeExists(node, "Refineable")) {
		bool refine;

		if (!this->asBool(node, "Refineable", refine))
			return 0;

		item->flag.no_refine = !refine;
	} else {
		if (!exists)
			item->flag.no_refine = true;
	}

	if (this->nodeExists(node, "Gradable")) {
		bool gradable;

		if (!this->asBool(node, "Gradable", gradable))
			return 0;

		item->flag.gradable = gradable;
	} else {
		if (!exists)
			item->flag.gradable = false;
	}

	if (this->nodeExists(node, "View")) {
		uint32 look;

		if (!this->asUInt32(node, "View", look))
			return 0;

		item->look = look;
	} else {
		if (!exists)
			item->look = 0;
	}

	if (this->nodeExists(node, "AliasName")) {
		std::string view;

		if (!this->asString(node, "AliasName", view))
			return 0;

		std::shared_ptr<item_data> view_data = item_db.search_aegisname( view.c_str() );

		if (view_data == nullptr) {
			this->invalidWarning(node["AliasName"], "Unable to change the alias because %s is an unknown item.\n", view.c_str());
			return 0;
		}

		item->view_id = view_data->nameid;
	} else {
		if (!exists)
			item->view_id = 0;
	}

	if (this->nodeExists(node, "Flags")) {
		const auto& flagNode = node["Flags"];

		if (this->nodeExists(flagNode, "BuyingStore")) {
			bool active;

			if (!this->asBool(flagNode, "BuyingStore", active))
				return 0;

			if (!itemdb_isstackable2(item.get()) && active) {
				this->invalidWarning(flagNode["BuyingStore"], "Non-stackable item cannot be enabled for buying store.\n");
				active = false;
			}

			item->flag.buyingstore = active;
		} else {
			if (!exists)
				item->flag.buyingstore = false;
		}

		if (this->nodeExists(flagNode, "DeadBranch")) {
			bool active;

			if (!this->asBool(flagNode, "DeadBranch", active))
				return 0;

			item->flag.dead_branch = active;
		} else {
			if (!exists)
				item->flag.dead_branch = false;
		}

		if (this->nodeExists(flagNode, "Container")) {
			bool active;

			if (!this->asBool(flagNode, "Container", active))
				return 0;

			item->flag.group = active;
		} else {
			if (!exists)
				item->flag.group = false;
		}

		if (this->nodeExists(flagNode, "UniqueId")) {
			bool active;

			if (!this->asBool(flagNode, "UniqueId", active))
				return 0;

			if (!itemdb_isstackable2(item.get()) && active) {
				this->invalidWarning(flagNode["UniqueId"], "Non-stackable item cannot be enabled for UniqueId.\n");
				active = false;
			}

			item->flag.guid = active;
		} else {
			if (!exists)
				item->flag.guid = false;
		}

		if (this->nodeExists(flagNode, "BindOnEquip")) {
			bool active;

			if (!this->asBool(flagNode, "BindOnEquip", active))
				return 0;

			item->flag.bindOnEquip = active;
		} else {
			if (!exists)
				item->flag.bindOnEquip = false;
		}

		if (this->nodeExists(flagNode, "DropAnnounce")) {
			bool active;

			if (!this->asBool(flagNode, "DropAnnounce", active))
				return 0;

			item->flag.broadcast = active;
		} else {
			if (!exists)
				item->flag.broadcast = false;
		}

		if (this->nodeExists(flagNode, "NoConsume")) {
			bool active;

			if (!this->asBool(flagNode, "NoConsume", active))
				return 0;

			if (active)
				item->flag.delay_consume |= DELAYCONSUME_NOCONSUME;
			else
				item->flag.delay_consume &= ~DELAYCONSUME_NOCONSUME;
		} else {
			if (!exists) {
				if (!(item->flag.delay_consume & DELAYCONSUME_TEMP))
					item->flag.delay_consume = DELAYCONSUME_NONE;
			}
		}

		if (this->nodeExists(flagNode, "DropEffect")) {
			std::string effect;

			if (!this->asString(flagNode, "DropEffect", effect))
				return 0;

			std::string effect_constant = "DROPEFFECT_" + effect;
			int64 constant;

			if (!script_get_constant(effect_constant.c_str(), &constant) || constant < DROPEFFECT_NONE || constant > DROPEFFECT_MAX) {
				this->invalidWarning(flagNode["DropEffect"], "Invalid item drop effect %s, defaulting to DROPEFFECT_NONE.\n", effect.c_str());
				constant = DROPEFFECT_NONE;
			}

			item->flag.dropEffect = static_cast<e_item_drop_effect>(constant);
		} else {
			if (!exists)
				item->flag.dropEffect = DROPEFFECT_NONE;
		}
	} else {
		if (!exists) {
			item->flag.buyingstore = false;
			item->flag.dead_branch = false;
			item->flag.group = false;
			item->flag.guid = false;
			item->flag.bindOnEquip = false;
			item->flag.broadcast = false;
			if (!(item->flag.delay_consume & DELAYCONSUME_TEMP))
				item->flag.delay_consume = DELAYCONSUME_NONE;
			item->flag.dropEffect = DROPEFFECT_NONE;
		}
	}

	if (this->nodeExists(node, "Delay")) {
		const auto& delayNode = node["Delay"];

		if (this->nodeExists(delayNode, "Duration")) {
			uint32 duration;

			if (!this->asUInt32(delayNode, "Duration", duration))
				return 0;

			item->delay.duration = duration;
		} else {
			if (!exists)
				item->delay.duration = 0;
		}

		if (this->nodeExists(delayNode, "Status")) {
			std::string status;

			if (!this->asString(delayNode, "Status", status))
				return 0;

			std::string status_constant = "SC_" + status;
			int64 constant;

			if (!script_get_constant(status_constant.c_str(), &constant) || constant < SC_NONE || constant >= SC_MAX) {
				this->invalidWarning(delayNode[c4::to_csubstr(status)], "Invalid item delay status %s, defaulting to SC_NONE.\n", status.c_str());
				constant = SC_NONE;
			}

			item->delay.sc = static_cast<sc_type>(constant);
		} else {
			if (!exists)
				item->delay.sc = SC_NONE;
		}
	} else {
		if (!exists) {
			item->delay.duration = 0;
			item->delay.sc = SC_NONE;
		}
	}

	if (this->nodeExists(node, "Stack")) {
		const auto& stackNode = node["Stack"];

		if (this->nodeExists(stackNode, "Amount")) {
			uint16 amount;

			if (!this->asUInt16(stackNode, "Amount", amount))
				return 0;

			if (!itemdb_isstackable2(item.get())) {
				this->invalidWarning(stackNode["Amount"], "Non-stackable item cannot be enabled for stacking.\n");
				amount = 0;
			}

			item->stack.amount = amount;
		} else {
			if (!exists)
				item->stack.amount = 0;
		}

		if (this->nodeExists(stackNode, "Inventory")) {
			bool active;

			if (!this->asBool(stackNode, "Inventory", active))
				return 0;

			item->stack.inventory = active;
		} else {
			if (!exists)
				item->stack.inventory = false;
		}

		if (this->nodeExists(stackNode, "Cart")) {
			bool active;

			if (!this->asBool(stackNode, "Cart", active))
				return 0;

			item->stack.cart = active;
		} else {
			if (!exists)
				item->stack.cart = false;
		}

		if (this->nodeExists(stackNode, "Storage")) {
			bool active;

			if (!this->asBool(stackNode, "Storage", active))
				return 0;

			item->stack.storage = active;
		} else {
			if (!exists)
				item->stack.storage = false;
		}

		if (this->nodeExists(stackNode, "GuildStorage")) {
			bool active;

			if (!this->asBool(stackNode, "GuildStorage", active))
				return 0;

			item->stack.guild_storage = active;
		} else {
			if (!exists)
				item->stack.guild_storage = false;
		}
	} else {
		if (!exists) {
			item->stack.amount = 0;
			item->stack.inventory = false;
			item->stack.cart = false;
			item->stack.storage = false;
			item->stack.guild_storage = false;
		}
	}
	
	if (this->nodeExists(node, "NoUse")) {
		const auto& nouseNode = node["NoUse"];

		if (this->nodeExists(nouseNode, "Override")) {
			uint16 override;

			if (!this->asUInt16(nouseNode, "Override", override))
				return 0;

			if (override > 100) {
				this->invalidWarning(nouseNode["Override"], "Item no use override level %d exceeds 100, capping to 100.\n", override);
				override = 100;
			}

			item->item_usage.override = override;
		} else {
			if (!exists)
				item->item_usage.override = 100;
		}

		if (this->nodeExists(nouseNode, "Sitting")) {
			bool active;

			if (!this->asBool(nouseNode, "Sitting", active))
				return 0;

			item->item_usage.sitting = active;
		} else {
			if (!exists)
				item->item_usage.sitting = false;
		}
	} else {
		if (!exists) {
			item->item_usage.override = 100;
			item->item_usage.sitting = false;
		}
	}

	if (this->nodeExists(node, "Trade")) {
		const auto& tradeNode = node["Trade"];

		if (this->nodeExists(tradeNode, "Override")) {
			uint16 override;

			if (!this->asUInt16(tradeNode, "Override", override))
				return 0;

			if (override > 100) {
				this->invalidWarning(tradeNode["Override"], "Item trade override level %d exceeds 100, capping to 100.\n", override);
				override = 100;
			}

			item->gm_lv_trade_override = override;
		} else {
			if (!exists)
				item->gm_lv_trade_override = 100;
		}

		if (this->nodeExists(tradeNode, "NoDrop")) {
			bool active;

			if (!this->asBool(tradeNode, "NoDrop", active))
				return 0;

			item->flag.trade_restriction.drop = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.drop = false;
		}

		if (this->nodeExists(tradeNode, "NoTrade")) {
			bool active;

			if (!this->asBool(tradeNode, "NoTrade", active))
				return 0;

			item->flag.trade_restriction.trade = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.trade = false;
		}

		if (this->nodeExists(tradeNode, "TradePartner")) {
			bool active;

			if (!this->asBool(tradeNode, "TradePartner", active))
				return 0;

			item->flag.trade_restriction.trade_partner = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.trade_partner = false;
		}

		if (this->nodeExists(tradeNode, "NoSell")) {
			bool active;

			if (!this->asBool(tradeNode, "NoSell", active))
				return 0;

			item->flag.trade_restriction.sell = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.sell = false;
		}

		if (this->nodeExists(tradeNode, "NoCart")) {
			bool active;

			if (!this->asBool(tradeNode, "NoCart", active))
				return 0;

			item->flag.trade_restriction.cart = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.cart = false;
		}

		if (this->nodeExists(tradeNode, "NoStorage")) {
			bool active;

			if (!this->asBool(tradeNode, "NoStorage", active))
				return 0;

			item->flag.trade_restriction.storage = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.storage = false;
		}

		if (this->nodeExists(tradeNode, "NoGuildStorage")) {
			bool active;

			if (!this->asBool(tradeNode, "NoGuildStorage", active))
				return 0;

			item->flag.trade_restriction.guild_storage = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.guild_storage = false;
		}

		if (this->nodeExists(tradeNode, "NoMail")) {
			bool active;

			if (!this->asBool(tradeNode, "NoMail", active))
				return 0;

			item->flag.trade_restriction.mail = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.mail = false;
		}

		if (this->nodeExists(tradeNode, "NoAuction")) {
			bool active;

			if (!this->asBool(tradeNode, "NoAuction", active))
				return 0;

			item->flag.trade_restriction.auction = active;
		} else {
			if (!exists)
				item->flag.trade_restriction.auction = false;
		}
	} else {
		if (!exists) {
			item->gm_lv_trade_override = 100;
			item->flag.trade_restriction.drop = false;
			item->flag.trade_restriction.trade = false;
			item->flag.trade_restriction.trade_partner = false;
			item->flag.trade_restriction.sell = false;
			item->flag.trade_restriction.cart = false;
			item->flag.trade_restriction.storage = false;
			item->flag.trade_restriction.guild_storage = false;
			item->flag.trade_restriction.mail = false;
			item->flag.trade_restriction.auction = false;
		}
	}

	if (this->nodeExists(node, "Script")) {
		std::string script;

		if (!this->asString(node, "Script", script))
			return 0;

		if (exists && item->script) {
			script_free_code(item->script);
			item->script = nullptr;
		}

		item->script = parse_script(script.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["Script"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else {
		if (!exists) 
			item->script = nullptr;
	}

	if (this->nodeExists(node, "EquipScript")) {
		std::string script;

		if (!this->asString(node, "EquipScript", script))
			return 0;

		if (exists && item->equip_script) {
			script_free_code(item->equip_script);
			item->equip_script = nullptr;
		}

		item->equip_script = parse_script(script.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["EquipScript"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else {
		if (!exists)
			item->equip_script = nullptr;
	}

	if (this->nodeExists(node, "UnEquipScript")) {
		std::string script;

		if (!this->asString(node, "UnEquipScript", script))
			return 0;

		if (exists && item->unequip_script) {
			script_free_code(item->unequip_script);
			item->unequip_script = nullptr;
		}

		item->unequip_script = parse_script(script.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["UnEquipScript"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else {
		if (!exists)
			item->unequip_script = nullptr;
	}

	if (!exists)
		this->put(nameid, item);

	return 1;
}

void ItemDatabase::loadingFinished(){
	for (auto &tmp_item : item_db) {
		std::shared_ptr<item_data> item = tmp_item.second;

		// Items that are consumed only after target confirmation
		if (item->type == IT_DELAYCONSUME) {
			item->type = IT_USABLE;
			item->flag.delay_consume |= DELAYCONSUME_TEMP;
		} else {
			item->flag.delay_consume &= ~DELAYCONSUME_TEMP; // Remove delayed consumption flag if switching types
		}

		if( item->type == IT_WEAPON ){
			if( item->weapon_level == 0 ){
				ShowWarning( "Item %s is a weapon, but does not have a weapon level. Consider adding it. Defaulting to 1.\n", item->name.c_str() );
				item->weapon_level = 1;
			}

			if( item->armor_level != 0 ){
				ShowWarning( "Item %s is a weapon, but has an armor level. Defaulting to 0.\n", item->name.c_str() );
				item->armor_level = 0;
			}
		}else if( item->type == IT_ARMOR ){
			if( item->armor_level == 0 ){
				ShowWarning( "Item %s is an armor, but does not have an armor level. Consider adding it. Defaulting to 1.\n", item->name.c_str() );
				item->armor_level = 1;
			}

			if( item->weapon_level != 0 ){
				ShowWarning( "Item %s is an armor, but has a weapon level. Defaulting to 0.\n", item->name.c_str() );
				item->weapon_level = 0;
			}
		}else{
			if( item->weapon_level != 0 ){
				ShowWarning( "Item %s is not a weapon, but has a weapon level. Defaulting to 0.\n", item->name.c_str() );
				item->weapon_level = 0;
			}

			if( item->armor_level != 0 ){
				ShowWarning( "Item %s is not an armor, but has an armor level. Defaulting to 0.\n", item->name.c_str() );
				item->armor_level = 0;
			}
		}

		if (item->type != IT_ARMOR && item->type != IT_SHADOWGEAR && item->def > 0) {
			ShowWarning( "Item %s is not a armor or shadowgear. Defaulting Defense to 0.\n", item->name.c_str() );
			item->def = 0;
		}

		if (item->type != IT_WEAPON && item->type != IT_AMMO && item->atk > 0) {
			ShowWarning( "Item %s is not a weapon or ammo. Defaulting Attack to 0.\n", item->name.c_str() );
			item->atk = 0;
		}

		if (item->type != IT_WEAPON) {
#ifdef RENEWAL
			if (item->matk > 0) {
				ShowWarning( "Item %s is not a weapon. Defaulting MagicAttack to 0.\n", item->name.c_str() );
				item->matk = 0;
			}
#endif
			if (item->range > 0) {
				ShowWarning( "Item %s is not a weapon. Defaulting Range to 0.\n", item->name.c_str() );
				item->range = 0;
			}
		}

		// When a particular price is not given, we should base it off the other one
		if (!hasPriceValue[item->nameid].has_buy && hasPriceValue[item->nameid].has_sell)
			item->value_buy = item->value_sell * 2;
		else if (hasPriceValue[item->nameid].has_buy && !hasPriceValue[item->nameid].has_sell)
			item->value_sell = item->value_buy / 2;

		if (item->value_buy / 124. < item->value_sell / 75.) {
			ShowWarning("Buying/Selling [%d/%d] price of %s (%u) allows Zeny making exploit through buying/selling at discounted/overcharged prices! Defaulting Sell to 1 Zeny.\n", item->value_buy, item->value_sell, item->name.c_str(), item->nameid);
			item->value_sell = 1;
		}

		// Shields need to have a view ID to be able to be recognized by ST_SHIELD check in skill.cpp
		if( item->type == IT_ARMOR && ( item->equip & EQP_SHIELD ) != 0 && item->look == 0 ){
			ShowWarning( "Item %s (%u) is a shield and should have a view id. Defaulting to Guard...\n", item->name.c_str(), item->nameid );
			item->look = 1;
		}
	}

	if( !this->exists( ITEMID_DUMMY ) ){
		// Create dummy item
		std::shared_ptr<item_data> dummy_item = std::make_shared<item_data>();

		dummy_item->nameid = ITEMID_DUMMY;
		dummy_item->weight = 1;
		dummy_item->value_sell = 1;
		dummy_item->type = IT_ETC;
		dummy_item->name = "UNKNOWN_ITEM";
		dummy_item->ename = "Unknown Item";
		dummy_item->view_id = UNKNOWN_ITEM_ID;

		item_db.put( ITEMID_DUMMY, dummy_item );
	}

	TypesafeCachedYamlDatabase::loadingFinished();
	hasPriceValue.clear();
}

/**
 * Applies gender restrictions according to settings.
 * @param node: YAML node containing the entry.
 * @param node: the already parsed item data.
 * @return gender that should be used.
 */
e_sex ItemDatabase::defaultGender( const ryml::NodeRef& node, std::shared_ptr<item_data> id ){
	if (id->nameid == WEDDING_RING_M) //Grom Ring
		return SEX_MALE;
	if (id->nameid == WEDDING_RING_F) //Bride Ring
		return SEX_FEMALE;
	if( id->type == IT_WEAPON ){
		if( id->subtype == W_MUSICAL ){
			if( id->sex != SEX_MALE ){
				this->invalidWarning( node, "Musical instruments are always male-only, defaulting to SEX_MALE.\n" );
			}

			return SEX_MALE;
		}

		if( id->subtype == W_WHIP ){
			if( id->sex != SEX_FEMALE ){
				this->invalidWarning( node, "Whips are always female-only, defaulting to SEX_FEMALE.\n" );
			}

			return SEX_FEMALE;
		}
	}

	return static_cast<e_sex>( id->sex );
}

std::shared_ptr<item_data> ItemDatabase::search_aegisname( const char* name ){
	// Create a copy
	std::string lowername = name;
	// Convert it to lower
	util::tolower( lowername );

	return util::umap_find( this->aegisNameToItemDataMap, lowername );
}

std::shared_ptr<item_data> ItemDatabase::searchname( const char *name ){
	// Create a copy
	std::string lowername = name;
	// Convert it to lower
	util::tolower( lowername );

	std::shared_ptr<item_data> result = util::umap_find( this->aegisNameToItemDataMap, lowername );

	if( result != nullptr ){
		return result;
	}

	return util::umap_find( this->nameToItemDataMap, lowername );
}

/**
* Generates an item link string
* @param data: Item info
* @return <ITEML> string for the item
* @author [Cydh]
**/
std::string ItemDatabase::create_item_link(struct item& item, std::shared_ptr<item_data>& data){
	if( data == nullptr ){
		ShowError( "Tried to create itemlink for unknown item %u.\n", item.nameid );
		return "Unknown item";
	}

	std::string itemstr;
	struct item_data* id = data.get();

// All these dates are unconfirmed
#if PACKETVER >= 20151104
	if( battle_config.feature_itemlink ) {

#if PACKETVER >= 20160113
		const std::string start_tag = "<ITEML>";
		const std::string closing_tag = "</ITEML>";
#else // PACKETVER >= 20151104
		const std::string start_tag = "<ITEM>";
		const std::string closing_tag = "</ITEM>";
#endif

		itemstr += start_tag;

		itemstr += util::string_left_pad(util::base62_encode(id->equip), '0', 5);
		itemstr += itemdb_isequip2(id) ? "1" : "0";
		itemstr += util::base62_encode(item.nameid);
		if (item.refine > 0) {
			itemstr += "%" + util::string_left_pad(util::base62_encode(item.refine), '0', 2);
		}

#if PACKETVER >= 20161116
		if (itemdb_isequip2(id)) {
			itemstr += "&" + util::string_left_pad(util::base62_encode(id->look), '0', 2);
		}
#endif

#if PACKETVER >= 20200724
		itemstr += "'" + util::string_left_pad(util::base62_encode(item.enchantgrade), '0', 2);
#endif

#if PACKETVER >= 20200724
		const std::string card_sep = ")";
		const std::string optid_sep = "+";
		const std::string optpar_sep = ",";
		const std::string optval_sep = "-";
#elif PACKETVER >= 20161116
		const std::string card_sep = "(";
		const std::string optid_sep = "*";
		const std::string optpar_sep = "+";
		const std::string optval_sep = ",";
#else // PACKETVER >= 20151104
		const std::string card_sep = "'";
		const std::string optid_sep = ")";
		const std::string optpar_sep = "*";
		const std::string optval_sep = "+";
#endif

		for (uint8 i = 0; i < MAX_SLOTS; ++i) {
			itemstr += card_sep + util::string_left_pad(util::base62_encode(item.card[i]), '0', 2);
		}

		for (uint8 i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
			if (item.option[i].id == 0) {
				break; // ignore options including ones beyond this one since the client won't even display them
			}
			// Option ID
			itemstr += optid_sep + util::string_left_pad(util::base62_encode(item.option[i].id), '0', 2);
			// Param
			itemstr += optpar_sep + util::string_left_pad(util::base62_encode(item.option[i].param), '0', 2);
			// Value
			itemstr += optval_sep + util::string_left_pad(util::base62_encode(item.option[i].value), '0', 2);
		}

		itemstr += closing_tag;
		if ((itemdb_isequip2(id)) && (data->slots == 0))
			itemstr += " [" + std::to_string(data->slots) + "]";

		return itemstr;
	}
#endif

	// This can be reached either because itemlinks are disabled via configuration or because the packet version does not support the feature
	// If that's the case then we format the item prepending the refine and appending the slots
	if (item.refine > 0)
		itemstr += "+" + std::to_string(item.refine) + " ";

	itemstr += data->ename;

	if (itemdb_isequip2(id))
		itemstr += "[" + std::to_string(data->slots) + "]";

	return itemstr;
}

std::string ItemDatabase::create_item_link( std::shared_ptr<item_data>& data ){
	struct item it = {};
	it.nameid = data->nameid;

	return this->create_item_link( it, data );
}

std::string ItemDatabase::create_item_link(struct item& item) {
	std::shared_ptr<item_data> data = this->find(item.nameid);

	return this->create_item_link(item, data);
}

std::string ItemDatabase::create_item_link_for_mes( std::shared_ptr<item_data>& data, bool use_brackets, const char* name ){
	if( data == nullptr ){
		return "Unknown item";
	}

	std::string itemstr;

// All these dates are unconfirmed
#if PACKETVER >= 20100000
	if( battle_config.feature_mesitemlink ){
// It was changed in 2015-11-04, but Gravity actually broke the feature for this specific client, because they introduced the new itemlink feature [Lemongrass]
// See the following github issues for more details:
// * https://github.com/rathena/rathena/issues/1236
// * https://github.com/rathena/rathena/issues/1873
#if PACKETVER >= 20151104
		const std::string start_tag = "<ITEM>";
		const std::string closing_tag = "</ITEM>";
#else
		const std::string start_tag = "<ITEMLINK>";
		const std::string closing_tag = "</ITEMLINK>";
#endif

		itemstr += start_tag;

		if( use_brackets || battle_config.feature_mesitemlink_brackets ){
			itemstr += "[";
		}

		if( name != nullptr && !battle_config.feature_mesitemlink_dbname ){
			// Name was forcefully overwritten
			itemstr += name;
		}else{
			// Use database name
			itemstr += data->ename;
		}

		if( use_brackets || battle_config.feature_mesitemlink_brackets ){
			itemstr += "]";
		}

		itemstr += "<INFO>";
		itemstr += std::to_string( data->nameid );
		itemstr += "</INFO>";

		itemstr += closing_tag;

		return itemstr;
	}
#endif

	// This can be reached either because itemlinks are disabled via configuration or because the packet version does not support the feature
	if( name != nullptr && !battle_config.feature_mesitemlink_dbname ){
		// Name was forcefully overwritten
		return name;
	}else{
		// Use database name
		return data->ename;
	}
}

std::string ItemDatabase::create_item_icon_for_mes( std::shared_ptr<item_data>& data, const char* name ){
	if( data == nullptr ){
		return "Unknown item";
	}

	// Feature is disabled
	if( !battle_config.feature_mesitemicon ){
		if( name != nullptr && !battle_config.feature_mesitemicon_dbname ){
			// Name was forcefully overwritten
			return name;
		}else{
			// Use database name
			return data->ename;
		}
	}

	const std::string start_tag = "^i[";
	const std::string closing_tag = "]";

	std::string itemstr;

	itemstr += start_tag;
	itemstr += std::to_string( data->nameid );
	itemstr += closing_tag;

	return itemstr;
}

ItemDatabase item_db;

/**
 * Check if an item exists in a group
 * @param group_id: Item Group ID
 * @param nameid: Item to check for in group
 * @return True if item is in group, else false
 */
bool ItemGroupDatabase::item_exists(uint16 group_id, t_itemid nameid)
{
	std::shared_ptr<s_item_group_db> group = this->find(group_id);

	if (group == nullptr)
		return false;

	for (const auto &random : group->random) {
		for (const auto &it : random.second->data) {
			if (it.second->nameid == nameid)
				return true;
		}
	}

	return false;
}

/**
 * Check if an item exists from a group in a player's inventory
 * @param group_id: Item Group ID
 * @return Item's index if found or -1 otherwise
 */
int16 ItemGroupDatabase::item_exists_pc(map_session_data *sd, uint16 group_id)
{
	std::shared_ptr<s_item_group_db> group = this->find(group_id);

	if (group == nullptr || group->random.empty())
		return -1;

	for (const auto &random : group->random) {
		for (const auto &it : random.second->data) {
			int16 item_position = pc_search_inventory(sd, it.second->nameid);

			if (item_position != -1)
				return item_position;
		}
	}

	return -1;
}

const std::string LaphineSynthesisDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/laphine_synthesis.yml";
}

uint64 LaphineSynthesisDatabase::parseBodyNode( const ryml::NodeRef& node ){
	t_itemid item_id;

	{
		std::string name;

		if( !this->asString( node, "Item", name ) ){
			return 0;
		}

		std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

		if( id == nullptr ){
			this->invalidWarning( node["Item"], "Unknown item \"%s\".\n", name.c_str() );
			return 0;
		}

		item_id = id->nameid;
	}

	std::shared_ptr<s_laphine_synthesis> entry = this->find( item_id );
	bool exists = entry != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "RewardGroup", "Requirements" } ) ){
			return 0;
		}

		entry = std::make_shared<s_laphine_synthesis>();
		entry->item_id = item_id;
	}

	if( this->nodeExists( node, "RewardGroup" ) ){
		std::string name;

		if( !this->asString( node, "RewardGroup", name ) ){
			return 0;
		}

		int64 constant;

		if( !script_get_constant( ( "IG_" + name ).c_str(), &constant ) ){
			this->invalidWarning( node["RewardGroup"], "Unknown reward group \"%s\".\n", name.c_str() );
			return 0;
		}

		if( !itemdb_group.exists( (uint16)constant ) ){
			this->invalidWarning( node["RewardGroup"], "Unknown reward group ID \"%" PRId64 "\".\n", constant );
			return 0;
		}

		entry->rewardGroupId = (uint16)constant;
	}else{
		if( !exists ){
			entry->rewardGroupId = 0;
		}
	}

	if( this->nodeExists( node, "RequiredRequirementsCount" ) ){
		uint16 amount;

		if( !this->asUInt16( node, "RequiredRequirementsCount", amount ) ){
			return 0;
		}

		entry->requiredRequirements = amount;
	}else{
		if( !exists ){
			entry->requiredRequirements = 1;
		}
	}

	if( this->nodeExists( node, "Requirements" ) ){
		for( const ryml::NodeRef& requirementNode : node["Requirements"] ){
			std::string name;

			if( !this->asString( requirementNode, "Item", name ) ){
				return 0;
			}

			std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

			if( id == nullptr ){
				this->invalidWarning( requirementNode["Item"], "Unknown item \"%s\".\n", name.c_str() );
				return 0;
			}

			std::shared_ptr<s_laphine_synthesis_requirement> requirement = util::umap_find( entry->requirements, id->nameid );
			bool requirement_exists = requirement != nullptr;

			if( !requirement_exists ){
				requirement = std::make_shared<s_laphine_synthesis_requirement>();
				requirement->item_id = id->nameid;
			}

			if( this->nodeExists( requirementNode, "Amount" ) ){
				uint16 amount;

				if( !this->asUInt16( requirementNode, "Amount", amount ) ){
					return 0;
				}

				if( amount > MAX_AMOUNT ){
					this->invalidWarning( requirementNode["Amount"], "Amount %hu is too high, capping to MAX_AMOUNT...\n", amount );
					amount = MAX_AMOUNT;
				}

				requirement->amount = amount;
			}else{
				if( !requirement_exists ){
					requirement->amount = 1;
				}
			}

			if( !requirement_exists ){
				entry->requirements[id->nameid] = requirement;
			}
		}
	}

	if( this->nodeExists( node, "MinimumRefine" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "MinimumRefine", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["MinimumRefine"], "Minimum refine %hu is too high, capping to MAX_REFINE...\n", refine );
			refine = MAX_REFINE;
		}

		entry->minimumRefine = refine;
	}else{
		if( !exists ){
			entry->minimumRefine = 0;
		}
	}

	if( this->nodeExists( node, "MaximumRefine" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "MaximumRefine", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["MaximumRefine"], "Maximum refine %hu is too high, capping to MAX_REFINE...\n", refine );
			refine = MAX_REFINE;
		}

		entry->maximumRefine = refine;
	}else{
		if( !exists ){
			entry->maximumRefine = MAX_REFINE;
		}
	}

	if( !exists ){
		this->put( entry->item_id, entry );
	}

	return 1;
}

LaphineSynthesisDatabase laphine_synthesis_db;

const std::string LaphineUpgradeDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/laphine_upgrade.yml";
}

uint64 LaphineUpgradeDatabase::parseBodyNode( const ryml::NodeRef& node ){
	t_itemid item_id;

	{
		std::string name;

		if( !this->asString( node, "Item", name ) ){
			return 0;
		}

		std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

		if( id == nullptr ){
			this->invalidWarning( node["Item"], "Unknown item \"%s\".\n", name.c_str() );
			return 0;
		}

		item_id = id->nameid;
	}

	std::shared_ptr<s_laphine_upgrade> entry = this->find( item_id );
	bool exists = entry != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "TargetItems" } ) ){
			return 0;
		}

		entry = std::make_shared<s_laphine_upgrade>();
		entry->item_id = item_id;
	}

	if( this->nodeExists( node, "RandomOptionGroup" ) ){
		std::string name;

		if( !this->asString( node, "RandomOptionGroup", name ) ){
			return 0;
		}

		uint16 id;

		if( !random_option_group.option_get_id( name, id ) ){
			this->invalidWarning( node["RandomOptionGroup"], "Unknown random option group \"%s\".\n", name.c_str() );
			return 0;
		}

		entry->randomOptionGroup = random_option_group.find( id );
	}else{
		if( !exists ){
			entry->randomOptionGroup = nullptr;
		}
	}

	if( this->nodeExists( node, "TargetItems" ) ){
		for( const ryml::NodeRef& targetNode : node["TargetItems"] ){
			std::string name;

			if( !this->asString( targetNode, "Item", name ) ){
				return 0;
			}

			std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

			if( id == nullptr ){
				this->invalidWarning( targetNode["Item"], "Unknown item \"%s\".\n", name.c_str() );
				return 0;
			}

			if( !util::vector_exists( entry->target_item_ids, id->nameid ) ){
				entry->target_item_ids.push_back( id->nameid );
			}
		}
	}

	if( this->nodeExists( node, "MinimumRefine" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "MinimumRefine", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["MinimumRefine"], "Minimum refine %hu is too high, capping to MAX_REFINE...\n", refine );
			refine = MAX_REFINE;
		}

		entry->minimumRefine = refine;
	}else{
		if( !exists ){
			entry->minimumRefine = 0;
		}
	}

	if( this->nodeExists( node, "MaximumRefine" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "MaximumRefine", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["MaximumRefine"], "Maximum refine %hu is too high, capping to MAX_REFINE...\n", refine );
			refine = MAX_REFINE;
		}

		entry->maximumRefine = refine;
	}else{
		if( !exists ){
			entry->maximumRefine = MAX_REFINE;
		}
	}

	if( this->nodeExists( node, "RequiredRandomOptions" ) ){
		uint16 amount;

		if( !this->asUInt16( node, "RequiredRandomOptions", amount ) ){
			return 0;
		}

		if( amount > MAX_ITEM_RDM_OPT ){
			this->invalidWarning( node["RequiredRandomOptions"], "Required random option amount %hu is too high, capping to MAX_ITEM_RDM_OPT...\n", amount );
			amount = MAX_ITEM_RDM_OPT;
		}

		entry->requiredRandomOptions = amount;
	}else{
		if( !exists ){
			entry->requiredRandomOptions = 0;
		}
	}

	if( this->nodeExists( node, "CardsAllowed" ) ){
		bool allowed;

		if( !this->asBool( node, "CardsAllowed", allowed ) ){
			return 0;
		}

		entry->cardsAllowed = allowed;
	}else{
		if( !exists ){
			entry->cardsAllowed = false;
		}
	}

	if( this->nodeExists( node, "ResultRefine" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "ResultRefine", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["ResultRefine"], "Result refine %hu is too high, capping to MAX_REFINE...\n", refine );
			refine = MAX_REFINE;
		}

		entry->resultRefine = refine;
	}else{
		if( !exists ){
			entry->resultRefine = 0;
		}
	}

	if( this->nodeExists( node, "ResultRefineMinimum" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "ResultRefineMinimum", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["ResultRefineMinimum"], "Result refine minimum %hu is too high, capping to MAX_REFINE...\n", refine );
			refine = MAX_REFINE;
		}

		entry->resultRefineMinimum = refine;
	}else{
		if( !exists ){
			entry->resultRefineMinimum = 0;
		}
	}

	if( this->nodeExists( node, "ResultRefineMaximum" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "ResultRefineMaximum", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["ResultRefineMaximum"], "Result refine maximum %hu is too high, capping to MAX_REFINE...\n", refine );
			refine = MAX_REFINE;
		}

		entry->resultRefineMaximum = refine;
	}else{
		if( !exists ){
			entry->resultRefineMaximum = 0;
		}
	}

	if( !exists ){
		this->put( entry->item_id, entry );
	}

	return 1;
}

LaphineUpgradeDatabase laphine_upgrade_db;

const std::string ItemReformDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/item_reform.yml";
}

uint64 ItemReformDatabase::parseBodyNode( const ryml::NodeRef& node ){
	t_itemid item_id;

	{
		std::string name;

		if( !this->asString( node, "Item", name ) ){
			return 0;
		}

		std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

		if( id == nullptr ){
			this->invalidWarning( node["Item"], "Unknown item \"%s\".\n", name.c_str() );
			return 0;
		}

		item_id = id->nameid;
	}

	std::shared_ptr<s_item_reform> entry = this->find( item_id );
	bool exists = entry != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "BaseItems" } ) ){
			return 0;
		}

		entry = std::make_shared<s_item_reform>();
		entry->item_id = item_id;
	}

	if( this->nodeExists( node, "BaseItems" ) ){
		for( const ryml::NodeRef& baseNode : node["BaseItems"] ){
			t_itemid base_itemid;

			{
				std::string name;

				if( !this->asString( baseNode, "BaseItem", name ) ){
					return 0;
				}

				std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

				if( id == nullptr ){
					this->invalidWarning( baseNode["BaseItem"], "Unknown item \"%s\".\n", name.c_str() );
					return 0;
				}

				base_itemid = id->nameid;
			}

			std::shared_ptr<s_item_reform_base> base = util::umap_find( entry->base_items, base_itemid );
			bool base_exists = base != nullptr;

			if( !base_exists ){
				if( !this->nodesExist( baseNode, { "ResultItem" } ) ){
					return 0;
				}

				base = std::make_shared<s_item_reform_base>();
				base->item_id = base_itemid;
			}

			if( this->nodeExists( baseNode, "MinimumRefine" ) ){
				uint16 refine;

				if( !this->asUInt16( baseNode, "MinimumRefine", refine ) ){
					return 0;
				}

				if( refine > MAX_REFINE ){
					this->invalidWarning( baseNode["MinimumRefine"], "Minimum refine %hu is too high, capping to MAX_REFINE...\n", refine );
					refine = MAX_REFINE;
				}

				base->minimumRefine = refine;
			}else{
				if( !base_exists ){
					base->minimumRefine = 0;
				}
			}

			if( this->nodeExists( baseNode, "MaximumRefine" ) ){
				uint16 refine;

				if( !this->asUInt16( baseNode, "MaximumRefine", refine ) ){
					return 0;
				}

				if( refine > MAX_REFINE ){
					this->invalidWarning( baseNode["MaximumRefine"], "Maximum refine %hu is too high, capping to MAX_REFINE...\n", refine );
					refine = MAX_REFINE;
				}

				base->maximumRefine = refine;
			}else{
				if( !base_exists ){
					base->maximumRefine = MAX_REFINE;
				}
			}

			if( this->nodeExists( baseNode, "RequiredRandomOptions" ) ){
				uint16 amount;

				if( !this->asUInt16( baseNode, "RequiredRandomOptions", amount ) ){
					return 0;
				}

				if( amount > MAX_ITEM_RDM_OPT ){
					this->invalidWarning( baseNode["RequiredRandomOptions"], "Required random option amount %hu is too high, capping to MAX_ITEM_RDM_OPT...\n", amount );
					amount = MAX_ITEM_RDM_OPT;
				}

				base->requiredRandomOptions = amount;
			}else{
				if( !base_exists ){
					base->requiredRandomOptions = 0;
				}
			}

			if( this->nodeExists( baseNode, "CardsAllowed" ) ){
				bool allowed;

				if( !this->asBool( baseNode, "CardsAllowed", allowed ) ){
					return 0;
				}

				base->cardsAllowed = allowed;
			}else{
				if( !base_exists ){
					base->cardsAllowed = true;
				}
			}

			if( this->nodeExists( baseNode, "Materials" ) ){
				for( const ryml::NodeRef& materialNode : baseNode["Materials"] ){
					std::string name;

					if( !this->asString( materialNode, "Material", name ) ){
						return 0;
					}

					std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

					if( id == nullptr ){
						this->invalidWarning( materialNode["Material"], "Unknown item \"%s\".\n", name.c_str() );
						return 0;
					}

					t_itemid material_id = id->nameid;
					bool material_exists = util::umap_find( base->materials, material_id ) != nullptr;
					uint16 amount;

					if( this->nodeExists( materialNode, "Amount" ) ){
						if( !this->asUInt16( materialNode, "Amount", amount ) ){
							return 0;
						}

						if( amount > MAX_AMOUNT ){
							this->invalidWarning( materialNode["Amount"], "Amount %hu is too high, capping to MAX_AMOUNT...\n", amount );
							amount = MAX_AMOUNT;
						}
					}else{
						if( !material_exists ){
							amount = 1;
						}
					}

					if( amount > 0 ){
						base->materials[material_id] = amount;
					}else{
						base->materials.erase( material_id );
					}
				}
			}

			if( this->nodeExists( baseNode, "ResultItem" ) ){
				std::string name;

				if( !this->asString( baseNode, "ResultItem", name ) ){
					return 0;
				}

				std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

				if( id == nullptr ){
					this->invalidWarning( baseNode["ResultItem"], "Unknown item \"%s\".\n", name.c_str() );
					return 0;
				}

				base->resultItemId = id->nameid;
			}

			if( this->nodeExists( baseNode, "ChangeRefine" ) ){
				int16 refine;

				if( !this->asInt16( baseNode, "ChangeRefine", refine ) ){
					return 0;
				}

				if( refine > MAX_REFINE ){
					this->invalidWarning( baseNode["MaximumRefine"], "Refine change %hu is too high, capping to MAX_REFINE...\n", refine );
					refine = MAX_REFINE;
				}else if( refine < -MAX_REFINE ){
					this->invalidWarning( baseNode["MaximumRefine"], "Refine change %hu is too low, capping to -MAX_REFINE...\n", refine );
					refine = -MAX_REFINE;
				}

				base->refineChange = refine;
			}else{
				if( !base_exists ){
					base->refineChange = 0;
				}
			}

			if( this->nodeExists( baseNode, "RandomOptionGroup" ) ){
				std::string name;

				if( !this->asString( baseNode, "RandomOptionGroup", name ) ){
					return 0;
				}

				uint16 id;

				if( !random_option_group.option_get_id( name, id ) ){
					this->invalidWarning( baseNode["RandomOptionGroup"], "Unknown random option group \"%s\".\n", name.c_str() );
					return 0;
				}

				base->randomOptionGroup = random_option_group.find( id );
			}else{
				if( !base_exists ){
					base->randomOptionGroup = nullptr;
				}
			}

			if( this->nodeExists( baseNode, "ClearSlots" ) ){
				bool clear;

				if( !this->asBool( baseNode, "ClearSlots", clear ) ){
					return 0;
				}

				base->clearSlots = clear;
			}else{
				if( !base_exists ){
					base->clearSlots = false;
				}
			}

			if( this->nodeExists( baseNode, "RemoveEnchantgrade" ) ){
				bool clear;

				if( !this->asBool( baseNode, "RemoveEnchantgrade", clear ) ){
					return 0;
				}

				base->removeEnchantgrade = clear;
			}else{
				if( !base_exists ){
					base->removeEnchantgrade = false;
				}
			}

			if( !base_exists ){
				entry->base_items[base_itemid] = base;
			}
		}
	}

	if( !exists ){
		this->put( entry->item_id, entry );
	}

	return 1;
}

ItemReformDatabase item_reform_db;

const std::string ItemEnchantDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/item_enchant.yml";
}

bool ItemEnchantDatabase::parseMaterials( const ryml::NodeRef& node, std::unordered_map<t_itemid, uint16>& materials ){
	if( this->nodeExists( node, "Materials" ) ){
		for( const ryml::NodeRef& materialNode : node["Materials"] ){
			std::string name;

			if( !this->asString( materialNode, "Material", name ) ){
				return false;
			}

			std::shared_ptr<item_data> item = item_db.search_aegisname( name.c_str() );

			if( item == nullptr ){
				this->invalidWarning( materialNode["Material"], "Unknown item \"%s\".\n", name.c_str() );
				return false;
			}

			t_itemid material_id = item->nameid;
			bool material_exists = util::umap_find( materials, material_id ) != nullptr;
			uint16 amount;

			if( this->nodeExists( materialNode, "Amount" ) ){
				if( !this->asUInt16( materialNode, "Amount", amount ) ){
					return false;
				}

				if( amount > MAX_AMOUNT ){
					this->invalidWarning( materialNode["Amount"], "Amount %hu is too high, capping to MAX_AMOUNT...\n", amount );
					amount = MAX_AMOUNT;
				}
			}else{
				if( !material_exists ){
					amount = 1;
				}else{
					amount = materials[material_id];
				}
			}

			if( amount > 0 ){
				materials[material_id] = amount;
			}else{
				materials.erase( material_id );
			}
		}
	}

	return true;
}

uint64 ItemEnchantDatabase::parseBodyNode( const ryml::NodeRef& node ){
	uint64 id;

	if( !this->asUInt64( node, "Id", id ) ){
		return 0;
	}

	std::shared_ptr<s_item_enchant> enchant = this->find( id );
	bool exists = enchant != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "TargetItems", "Order", "Slots" } ) ){
			return 0;
		}

		enchant = std::make_shared<s_item_enchant>();
		enchant->id = id;
	}

	if( this->nodeExists( node, "TargetItems" ) ){
		const ryml::NodeRef& targetItemsNode = node["TargetItems"];

		for( const auto& it : targetItemsNode ){
			std::string name;

			c4::from_chars( it.key(), &name );

			std::shared_ptr<item_data> item = item_db.search_aegisname( name.c_str() );

			if( item == nullptr ){
				this->invalidWarning( it, "Unknown item \"%s\".\n", name.c_str() );
				return 0;
			}

			bool enable;

			if( !this->asBool( targetItemsNode, name, enable ) ){
				return 0;
			}

			if( enable ){
				if( util::vector_exists( enchant->target_item_ids, item->nameid ) ){
					this->invalidWarning( it, "Target item \"%s\" is already in the list.\n", name.c_str() );
				}else{
					enchant->target_item_ids.push_back( item->nameid );
				}
			}else{
				if( !util::vector_exists( enchant->target_item_ids, item->nameid ) ){
					this->invalidWarning( it, "Target item \"%s\" is not in the list.\n", name.c_str() );
				}else{
					util::vector_erase_if_exists( enchant->target_item_ids, item->nameid );
				}
			}
		}
	}

	if( this->nodeExists( node, "MinimumRefine" ) ){
		uint16 refine;

		if( !this->asUInt16( node, "MinimumRefine", refine ) ){
			return 0;
		}

		if( refine > MAX_REFINE ){
			this->invalidWarning( node["MinimumRefine"], "Minimum refine %hu exceeds MAX_REFINE. Capping...\n", refine );
			refine = MAX_REFINE;
		}

		enchant->minimumRefine = refine;
	}else{
		if( !exists ){
			enchant->minimumRefine = 0;
		}
	}

	if( this->nodeExists( node, "MinimumEnchantgrade" ) ){
		uint16 enchantgrade;

		if( !this->asUInt16( node, "MinimumEnchantgrade", enchantgrade ) ){
			return 0;
		}

		if( enchantgrade > MAX_ENCHANTGRADE ){
			this->invalidWarning( node["MinimumEnchantgrade"], "Minimum enchantgrade %hu exceeds MAX_ENCHANTGRADE. Capping...\n", enchantgrade );
			enchantgrade = MAX_ENCHANTGRADE;
		}

		enchant->minimumEnchantgrade = enchantgrade;
	}else{
		if( !exists ){
			enchant->minimumEnchantgrade = 0;
		}
	}

	if( this->nodeExists( node, "AllowRandomOptions" ) ){
		bool allow;

		if( !this->asBool( node, "AllowRandomOptions", allow ) ){
			return 0;
		}

		enchant->allowRandomOptions = allow;
	}else{
		if( !exists ){
			enchant->allowRandomOptions = true;
		}
	}

	if( this->nodeExists( node, "Reset" ) ){
		const ryml::NodeRef& resetNode = node["Reset"];

		if( this->nodeExists( resetNode, "Chance" ) ){
			uint32 chance;

			if( !this->asUInt32Rate( resetNode, "Chance", chance, 100000 ) ){
				return 0;
			}

			enchant->reset.chance = chance;
		}else{
			if( !exists ){
				enchant->reset.chance = 0;
			}
		}

		if( this->nodeExists( resetNode, "Price" ) ){
			uint32 zeny;

			if( !this->asUInt32( resetNode, "Price", zeny ) ){
				return 0;
			}

			if( zeny > MAX_ZENY ){
				this->invalidWarning( resetNode["Price"], "Price %u exceeds MAX_ZENY. Capping...\n", zeny );
				zeny = MAX_ZENY;
			}

			enchant->reset.zeny = zeny;
		}else{
			if( !exists ){
				enchant->reset.zeny = 0;
			}
		}

		if( !this->parseMaterials( resetNode, enchant->reset.materials ) ){
			return 0;
		}
	}else{
		if( !exists ){
			enchant->reset = {};
		}
	}

	if( this->nodeExists( node, "Order" ) ){
		enchant->order.clear();

		for( const auto& it : node["Order"] ){
			uint16 slot;

			if( !this->asUInt16( it, "Slot", slot ) ){
				return 0;
			}

			if( slot >= MAX_SLOTS ){
				this->invalidWarning( it, "Slot %hu exceeds MAX_SLOTS...\n", slot );
				return 0;
			}

			enchant->order.push_back( slot );
		}
	}

	if( this->nodeExists( node, "Slots" ) ){
		for( const ryml::NodeRef& slotNode : node["Slots"] ){
			uint16 slot;

			if( !this->asUInt16( slotNode, "Slot", slot ) ){
				return 0;
			}

			if( slot >= MAX_SLOTS ){
				this->invalidWarning( slotNode["Slot"], "Slot %hu exceeds MAX_SLOTS...\n", slot );
				return 0;
			}

			std::shared_ptr<s_item_enchant_slot> enchant_slot = util::umap_find( enchant->slots, slot );
			bool slot_exists = enchant_slot != nullptr;

			if( !slot_exists ){
				enchant_slot = std::make_shared<s_item_enchant_slot>();
				enchant_slot->slot = slot;

				for( int32 i = 0; i <= MAX_ENCHANTGRADE; i++ ){
					enchant_slot->normal.enchantgradeChanceIncrease[i] = 0;
				}
			}

			if( this->nodeExists( slotNode, "Price" ) ){
				uint32 zeny;

				if( !this->asUInt32( slotNode, "Price", zeny ) ){
					return 0;
				}

				if( zeny > MAX_ZENY ){
					this->invalidWarning( slotNode["Price"], "Price %u exceeds MAX_ZENY. Capping...\n", zeny );
					zeny = MAX_ZENY;
				}

				enchant_slot->normal.zeny = zeny;
			}else{
				if( !slot_exists ){
					enchant_slot->normal.zeny = 0;
				}
			}

			if( !this->parseMaterials( slotNode, enchant_slot->normal.materials ) ){
				return 0;
			}

			if( this->nodeExists( slotNode, "Chance" ) ){
				uint32 chance;

				if( !this->asUInt32Rate( slotNode, "Chance", chance, 100000 ) ){
					return 0;
				}

				enchant_slot->normal.chance = chance;
			}else{
				if( !slot_exists ){
					enchant_slot->normal.chance = 100000;
				}
			}

			if( this->nodeExists( slotNode, "EnchantgradeBonus" ) ){
				for( const ryml::NodeRef& enchantgradeNode : slotNode["EnchantgradeBonus"] ){
					uint16 enchantgrade;

					if( !this->asUInt16( enchantgradeNode, "Enchantgrade", enchantgrade ) ){
						return 0;
					}

					if( enchantgrade > MAX_ENCHANTGRADE ){
						this->invalidWarning( enchantgradeNode["Enchantgrade"], "Enchant grade %hu exceeds MAX_ENCHANTGRADE.\n", enchantgrade );
						return 0;
					}

					uint32 chance;

					if( !this->asUInt32Rate( slotNode, "Chance", chance, 100000 ) ){
						return 0;
					}

					enchant_slot->normal.enchantgradeChanceIncrease[enchantgrade] = chance;
				}
			}

			if( this->nodeExists( slotNode, "Enchants" ) ){
				for( const ryml::NodeRef& enchantNode : slotNode["Enchants"] ){
					uint16 enchantgrade;

					if( !this->asUInt16( enchantNode, "Enchantgrade", enchantgrade ) ){
						return 0;
					}

					if( enchantgrade > MAX_ENCHANTGRADE ){
						this->invalidWarning( enchantNode["Enchantgrade"], "Enchant grade %hu exceeds MAX_ENCHANTGRADE...\n", enchantgrade );
						return 0;
					}

					std::shared_ptr<s_item_enchant_normal> enchants_for_enchantgrade = util::umap_find( enchant_slot->normal.enchants, enchantgrade );
					bool enchants_for_enchantgrade_exists = enchants_for_enchantgrade != nullptr;

					if( !enchants_for_enchantgrade_exists ){
						enchants_for_enchantgrade = std::make_shared<s_item_enchant_normal>();
						enchants_for_enchantgrade->enchantgrade = enchantgrade;
					}

					if( this->nodeExists( enchantNode, "Items" ) ){
						for( const ryml::NodeRef& itemNode : enchantNode["Items"] ){
							std::string enchant_name;

							if( !this->asString( itemNode, "Item", enchant_name ) ){
								return 0;
							}

							std::shared_ptr<item_data> enchant_item = item_db.search_aegisname( enchant_name.c_str() );

							if( enchant_item == nullptr ){
								this->invalidWarning( itemNode["Item"], "Unknown item \"%s\".\n", enchant_name.c_str() );
								return 0;
							}

							std::shared_ptr<s_item_enchant_normal_sub> enchant = util::umap_find( enchants_for_enchantgrade->enchants, enchant_item->nameid );
							bool enchant_exists = enchant != nullptr;

							if( !enchant_exists ){
								if( !this->nodesExist( itemNode, { "Chance" } ) ){
									return 0;
								}

								enchant = std::make_shared<s_item_enchant_normal_sub>();
								enchant->item_id = enchant_item->nameid;
							}

							if( this->nodeExists( itemNode, "Chance" ) ){
								uint32 chance;

								if( !this->asUInt32Rate( itemNode, "Chance", chance, 100000 ) ){
									return 0;
								}

								enchant->chance = chance;
							}

							if( !enchant_exists ){
								enchants_for_enchantgrade->enchants[enchant->item_id] = enchant;
							}
						}
					}

					if( !enchants_for_enchantgrade_exists ){
						enchant_slot->normal.enchants[enchantgrade] = enchants_for_enchantgrade;
					}
				}
			}

			if( this->nodeExists( slotNode, "PerfectEnchants" ) ){
				for( const ryml::NodeRef& enchantNode : slotNode["PerfectEnchants"] ){
					std::string enchant_name;

					if( !this->asString( enchantNode, "Item", enchant_name ) ){
						return 0;
					}

					std::shared_ptr<item_data> enchant_item = item_db.search_aegisname( enchant_name.c_str() );

					if( enchant_item == nullptr ){
						this->invalidWarning( enchantNode["Item"], "Unknown item \"%s\".\n", enchant_name.c_str() );
						return 0;
					}

					std::shared_ptr<s_item_enchant_perfect> enchant = util::umap_find( enchant_slot->perfect.enchants, enchant_item->nameid );
					bool enchant_exists = enchant != nullptr;

					if( !enchant_exists ){
						enchant = std::make_shared<s_item_enchant_perfect>();
						enchant->item_id = enchant_item->nameid;
					}

					if( this->nodeExists( enchantNode, "Price" ) ){
						uint32 zeny;

						if( !this->asUInt32( enchantNode, "Price", zeny ) ){
							return 0;
						}

						if( zeny > MAX_ZENY ){
							this->invalidWarning( enchantNode["Price"], "Price %u exceeds MAX_ZENY. Capping...\n", zeny );
							zeny = MAX_ZENY;
						}

						enchant->zeny = zeny;
					}else{
						if( !slot_exists ){
							enchant->zeny = 0;
						}
					}

					if( !this->parseMaterials( enchantNode, enchant->materials ) ){
						return 0;
					}

					if( !enchant_exists ){
						enchant_slot->perfect.enchants[enchant->item_id] = enchant;
					}
				}
			}

			if( this->nodeExists( slotNode, "Upgrades" ) ){
				for( const ryml::NodeRef& upgradeNode : slotNode["Upgrades"] ){
					std::string enchant_name;

					if( !this->asString( upgradeNode, "Enchant", enchant_name ) ){
						return 0;
					}

					std::shared_ptr<item_data> enchant_item = item_db.search_aegisname( enchant_name.c_str() );

					if( enchant_item == nullptr ){
						this->invalidWarning( upgradeNode["Enchant"], "Unknown item \"%s\".\n", enchant_name.c_str() );
						return 0;
					}

					std::shared_ptr<s_item_enchant_upgrade> enchant_upgrade = util::umap_find( enchant_slot->upgrade.enchants, enchant_item->nameid );
					bool enchant_upgrade_exists = enchant_upgrade != nullptr;

					if( !enchant_upgrade_exists ){
						if( !this->nodesExist( upgradeNode, { "Upgrade" } ) ){
							return 0;
						}

						enchant_upgrade = std::make_shared<s_item_enchant_upgrade>();
						enchant_upgrade->enchant_item_id = enchant_item->nameid;
					}

					if( this->nodeExists( upgradeNode, "Upgrade" ) ){
						std::string upgrade_name;

						if( !this->asString( upgradeNode, "Upgrade", upgrade_name ) ){
							return 0;
						}

						std::shared_ptr<item_data> upgrade_item = item_db.search_aegisname( upgrade_name.c_str() );

						if( upgrade_item == nullptr ){
							this->invalidWarning( upgradeNode["Upgrade"], "Unknown item \"%s\".\n", upgrade_name.c_str() );
							return 0;
						}

						enchant_upgrade->upgrade_item_id = upgrade_item->nameid;
					}

					if( this->nodeExists( upgradeNode, "Price" ) ){
						uint32 zeny;

						if( !this->asUInt32( upgradeNode, "Price", zeny ) ){
							return 0;
						}

						if( zeny > MAX_ZENY ){
							this->invalidWarning( upgradeNode["Price"], "Price %u exceeds MAX_ZENY. Capping...\n", zeny );
							zeny = MAX_ZENY;
						}

						enchant_upgrade->zeny = zeny;
					}else{
						if( !enchant_upgrade_exists ){
							enchant_upgrade->zeny = 0;
						}
					}

					if( !this->parseMaterials( upgradeNode, enchant_upgrade->materials ) ){
						return 0;
					}

					if( !enchant_upgrade_exists ){
						enchant_slot->upgrade.enchants[enchant_upgrade->enchant_item_id] = enchant_upgrade;
					}
				}
			}

			if( !slot_exists ){
				enchant->slots[slot] = enchant_slot;
			}
		}
	}

	if( !exists ){
		this->put( enchant->id, enchant );
	}

	return 1;
}

ItemEnchantDatabase item_enchant_db;

const std::string ItemPackageDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/item_packages.yml";
}

uint64 ItemPackageDatabase::parseBodyNode( const ryml::NodeRef& node ){
	t_itemid item_id;

	{
		std::string name;

		if( !this->asString( node, "Item", name ) ){
			return 0;
		}

		std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

		if( id == nullptr ){
			this->invalidWarning( node["Item"], "Unknown item \"%s\".\n", name.c_str() );
			return 0;
		}

		item_id = id->nameid;
	}

	std::shared_ptr<s_item_package> entry = this->find( item_id );
	bool exists = entry != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "Groups" } ) ){
			return 0;
		}

		entry = std::make_shared<s_item_package>();
		entry->item_id = item_id;
	}

	if( this->nodeExists( node, "Groups" ) ){
		for( const ryml::NodeRef& groupNode : node["Groups"] ){
			uint32 groupIndex;

			if( !this->asUInt32( groupNode, "Group", groupIndex) ){
				return 0;
			}

			std::shared_ptr<s_item_package_group> group = util::umap_find( entry->groups, groupIndex );
			bool group_exists = group != nullptr;

			if( !group_exists ){
				if( !this->nodesExist( groupNode, { "Items" } ) ){
					return 0;
				}

				group = std::make_shared<s_item_package_group>();
				group->groupIndex = groupIndex;
			}

			for( const ryml::NodeRef& itemNode : groupNode["Items"] ){
				std::string name;

				if( !this->asString( itemNode, "Item", name ) ){
					return 0;
				}

				std::shared_ptr<item_data> id = item_db.search_aegisname( name.c_str() );

				if( id == nullptr ){
					this->invalidWarning( itemNode["Item"], "Unknown item \"%s\".\n", name.c_str() );
					return 0;
				}

				std::shared_ptr<s_item_package_item> package_item = util::umap_find( group->items, id->nameid );
				bool package_item_exists = package_item != nullptr;

				if( !package_item_exists ){
					package_item = std::make_shared<s_item_package_item>();
					package_item->item_id = id->nameid;
				}

				if( this->nodeExists( itemNode, "Amount" ) ){
					uint16 amount;

					if( !this->asUInt16( itemNode, "Amount", amount ) ){
						return 0;
					}

					if( amount > MAX_AMOUNT ){
						this->invalidWarning( itemNode["Amount"], "Amount %hu is too high, capping to MAX_AMOUNT...\n", amount );
						amount = MAX_AMOUNT;
					}else if( amount == 0 ){
						if( !package_item_exists ){
							this->invalidWarning( itemNode["Amount"], "Trying to remove non existant item \"%s\".\n", name.c_str() );
							return 0;
						}else{
							group->items.erase( id->nameid );
						}
					}

					package_item->amount = amount;
				}else{
					if( !package_item_exists ){
						package_item->amount = 1;
					}
				}

				if( this->nodeExists( itemNode, "RentalHours" ) ){
					uint16 rentalhours;

					if( !this->asUInt16( itemNode, "RentalHours", rentalhours ) ){
						return 0;
					}

					package_item->rentalhours = rentalhours;
				}else{
					if( !package_item_exists ){
						package_item->rentalhours = 0;
					}
				}

				if( this->nodeExists( itemNode, "Refine" ) ){
					uint16 refine;

					if( !this->asUInt16( itemNode, "Refine", refine ) ){
						return 0;
					}

					if( refine > MAX_REFINE ){
						this->invalidWarning( itemNode["Refine"], "Refine %hu is too high, capping to MAX_REFINE...\n", refine );
						refine = MAX_REFINE;
					}

					package_item->refine = refine;
				}else{
					if( !package_item_exists ){
						package_item->refine = 0;
					}
				}

				if( this->nodeExists( itemNode, "RandomOptionGroup" ) ){
					std::string name;

					if( !this->asString( itemNode, "RandomOptionGroup", name ) ){
						return 0;
					}

					uint16 option_group_id;

					if( !random_option_group.option_get_id( name, option_group_id ) ){
						this->invalidWarning( itemNode["RandomOptionGroup"], "Unknown random option group \"%s\".\n", name.c_str() );
						return 0;
					}

					package_item->randomOptionGroup = random_option_group.find( option_group_id );
				}else{
					if( !package_item_exists ){
						package_item->randomOptionGroup = nullptr;
					}
				}

				if( !package_item_exists ){
					group->items[package_item->item_id] = package_item;
				}
			}

			if( !group_exists ){
				entry->groups[group->groupIndex] = group;
			}
		}
	}

	if( !exists ){
		this->put( entry->item_id, entry );
	}

	return 1;
}

ItemPackageDatabase item_package_db;

/*==========================================
 * Finds up to N matches. Returns number of matches [Skotlex]
 * @param *data
 * @param size
 * @param str
 * @return Number of matches item
 *------------------------------------------*/
uint16 itemdb_searchname_array(std::map<t_itemid, std::shared_ptr<item_data>> &data, uint16 size, const char *str)
{
	for (const auto &item : item_db) {
		std::shared_ptr<item_data> id = item.second;

		if (id == nullptr)
			continue;
		if (stristr(id->name.c_str(), str) != nullptr || stristr(id->ename.c_str(), str) != nullptr || strcmpi(id->ename.c_str(), str) == 0)
			data[id->nameid] = id;
	}

	if (data.size() > size)
		util::map_resize(data, size);

	return static_cast<uint16>(data.size());
}

std::shared_ptr<s_item_group_entry> ItemGroupDatabase::get_random_itemsubgroup(std::shared_ptr<s_item_group_random> random, e_group_algorithm_type algorithm) {
	if (random == nullptr)
		return nullptr;

	// Use algorithm defined for the sub group
	if (algorithm == GROUP_ALGORITHM_USEDB)
		algorithm = random->algorithm;

	switch( algorithm ) {
		case GROUP_ALGORITHM_DROP: {
			// We pick a random item from the group and then do a drop check based on the rate. On fail, do not return any item
			std::shared_ptr<s_item_group_entry> entry = util::umap_random(random->data);
			if (rnd_chance_official<uint16>(entry->adj_rate, 10000))
				return entry;
			break;
		}
		case GROUP_ALGORITHM_ALL:
			// This group algorithm is usually used to return all items in the group
			// The code here is only reached when using this algorithm in a command that expects to return only one item
			// In this case, we return a random item in the group
			return util::umap_random(random->data);
		case GROUP_ALGORITHM_RANDOM: {
			// Each item has x positions whereas x is the rate defined for the item in the umap
			// We pick a random position and find the item that is at this position
			uint32 pos = rnd_value<uint32>(1, random->total_rate);
			uint32 current_pos = 1;
			// Iterate through each item in the umap
			for (const auto& [index, entry] : random->data) {
				if (entry == nullptr)
					return nullptr;
				// We move "rate" positions
				current_pos += entry->rate;
				// If we passed the target position, entry is the item we are looking for
				if (current_pos > pos)
					return entry;
			}
			break;
		}
		case GROUP_ALGORITHM_SHAREDPOOL: {
			// By default, each item has x positions whereas x is the rate defined for the item in the umap
			// Each time an item is picked, it has one of its positions removed until no positions remain in the group
			// We pick a random position from all remaining positions and find the item that is at this position
			uint32 pos = rnd_value<uint32>(1, random->total_rate - random->total_given);
			uint32 current_pos = 1;
			// Iterate through each item in the umap
			for (const auto& [index, entry] : random->data) {
				if (entry == nullptr)
					return nullptr;
				// We move as many positions as this item has left
				current_pos += (entry->rate - entry->given);
				// If we passed the target position, entry is the item we are looking for
				if (current_pos > pos) {
					// Increase amount item has been given out
					entry->given++;
					random->total_given++;
					// All items have been given out, reset all entries in the group
					if (random->total_given >= random->total_rate) {
						random->total_given = 0;
						for (const auto& [reset_index, reset_entry] : random->data) {
							reset_entry->given = 0;
						}
					}
					return entry;
				}
			}
			break;
		}
	}
	// Return nullptr on fail
	return nullptr;
}

/**
* Return a random group entry from Item Group
* @param group_id
* @param sub_group
* @param search_type: see e_group_algorithm_type
* @return Item group entry or nullptr on fail
*/
std::shared_ptr<s_item_group_entry> ItemGroupDatabase::get_random_entry(uint16 group_id, uint8 sub_group, e_group_algorithm_type algorithm) {
	std::shared_ptr<s_item_group_db> group = this->find(group_id);

	if (group == nullptr) {
		ShowError("get_random_entry: Invalid group id %hu.\n", group_id);
		return nullptr;
	}
	if (group->random.empty()) {
		ShowError("get_random_entry: No item entries for group id %hu.\n", group_id);
		return nullptr;
	}
	if (group->random.count(sub_group) == 0) {
		ShowError("get_random_entry: No item entries for group id %hu and sub group %hu.\n", group_id, sub_group);
		return nullptr;
	}

	return this->get_random_itemsubgroup(group->random[sub_group], algorithm);
}

/** [Cydh]
* Gives item(s) to the player based on item group
* @param sd: Player that obtains item from item group
* @param identify
* @param data: item data selected in a subgroup
*/
void ItemGroupDatabase::pc_get_itemgroup_sub( map_session_data& sd, bool identify, std::shared_ptr<s_item_group_entry> data ){
	if (data == nullptr)
		return;

	item tmp = {};

	tmp.nameid = data->nameid;
	tmp.bound = data->bound;
	tmp.identify = identify ? identify : itemdb_isidentified(data->nameid);
	tmp.expire_time = (data->duration) ? (uint32)(time(nullptr) + data->duration*60) : 0;
	if (data->isNamed) {
		tmp.card[0] = itemdb_isequip(data->nameid) ? CARD0_FORGE : CARD0_CREATE;
		tmp.card[1] = 0;
		tmp.card[2] = GetWord( sd.status.char_id, 0 );
		tmp.card[3] = GetWord( sd.status.char_id, 1 );
	}

	uint16 get_amt = 0;

	if (itemdb_isstackable(data->nameid) && data->isStacked)
		get_amt = data->amount;
	else
		get_amt = 1;

	tmp.amount = get_amt;

	// Do loop for non-stackable item
	for (uint16 i = 0; i < data->amount; i += get_amt) {
		tmp.unique_id = data->GUID ? pc_generate_unique_id( &sd ) : 0; // Generate GUID

		if( itemdb_isequip( data->nameid ) ){
			if( data->refineMinimum > 0 && data->refineMaximum > 0 ){
				tmp.refine = static_cast<uint8>( rnd_value<uint16>( data->refineMinimum, data->refineMaximum ) );
			}else if( data->refineMinimum > 0 ){
				tmp.refine = static_cast<uint8>( rnd_value<uint16>( data->refineMinimum, MAX_REFINE ) );
			}else if( data->refineMaximum > 0 ){
				tmp.refine = static_cast<uint8>( rnd_value<uint16>( 1, data->refineMaximum ) );
			}else{
				tmp.refine = 0;
			}

			if( data->randomOptionGroup != nullptr ){
				memset( tmp.option, 0, sizeof( tmp.option ) );

				data->randomOptionGroup->apply( tmp );
			}
		}

		e_additem_result flag = pc_additem( &sd, &tmp, get_amt, LOG_TYPE_SCRIPT );

		if( flag == ADDITEM_SUCCESS ){
			if( data->isAnnounced ){
				intif_broadcast_obtain_special_item( &sd, data->nameid, sd.itemid, ITEMOBTAIN_TYPE_BOXITEM );
			}
		}else{
			clif_additem( &sd, 0, 0, flag );
		}
	}
}

/** [Cydh]
* Find item(s) that will be obtained by player based on Item Group
* @param group_id: The group ID that will be gained by player
* @param nameid: The item that trigger this item group
* @return val: 0:success, 2:invalid item group
*/
uint8 ItemGroupDatabase::pc_get_itemgroup( uint16 group_id, bool identify, map_session_data& sd ){
	std::shared_ptr<s_item_group_db> group = this->find(group_id);

	if (group == nullptr) {
		ShowError("pc_get_itemgroup: Invalid group id '%d' specified.\n",group_id);
		return 2;
	}
	if (group->random.empty())
		return 0;

	for (const auto &random : group->random) {
		switch( random.second->algorithm ) {
			case GROUP_ALGORITHM_RANDOM:
			case GROUP_ALGORITHM_SHAREDPOOL:
				this->pc_get_itemgroup_sub( sd, identify, this->get_random_itemsubgroup( random.second ) );
				break;
			case GROUP_ALGORITHM_ALL:
				for (const auto &it : random.second->data)
					this->pc_get_itemgroup_sub( sd, identify, it.second );
				break;
		}
	}

	return 0;
}

/** Searches for the item_data. Use this to check if item exists or not.
* @param nameid
* @return *item_data if item is exist, or nullptr if not
*/
std::shared_ptr<item_data> itemdb_exists(t_itemid nameid) {
	return item_db.find(nameid);
}

/// Returns name type of ammunition [Cydh]
const char *itemdb_typename_ammo (e_ammo_type ammo) {
	switch (ammo) {
		case AMMO_ARROW:		return "Arrow";
		case AMMO_DAGGER:		return "Throwable Dagger";
		case AMMO_BULLET:		return "Bullet";
		case AMMO_SHELL:		return "Shell";
		case AMMO_GRENADE:		return "Grenade";
		case AMMO_SHURIKEN:		return "Shuriken";
		case AMMO_KUNAI:		return "Kunai";
		case AMMO_CANNONBALL:	return "Cannonball";
		case AMMO_THROWWEAPON:	return "Throwable Item/Sling Item";
	}
	return "Ammunition";
}

/// Returns human readable name for given item type.
/// @param type Type id to retrieve name for ( IT_* ).
const char* itemdb_typename(enum item_types type)
{
	switch(type)
	{
		case IT_HEALING:        return "Potion/Food";
		case IT_USABLE:         return "Usable";
		case IT_ETC:            return "Etc.";
		case IT_WEAPON:         return "Weapon";
		case IT_ARMOR:          return "Armor";
		case IT_CARD:           return "Card";
		case IT_PETEGG:         return "Pet Egg";
		case IT_PETARMOR:       return "Pet Accessory";
		case IT_AMMO:           return "Arrow/Ammunition";
		case IT_DELAYCONSUME:   return "Delay-Consume Usable";
		case IT_SHADOWGEAR:     return "Shadow Equipment";
		case IT_CASH:           return "Cash Usable";
	}
	return "Unknown Type";
}

/**
 * Converts the jobmask from the format in itemdb to the format used by the map server.
 * @param bclass: Pointer to store itemdb format
 * @param jobmask: Job Mask to convert
 * @param active: Whether the flag is active or not
 * @author: Skotlex
 */
static void itemdb_jobid2mapid(uint64 bclass[3], e_mapid jobmask, bool active)
{
	uint64 temp_mask[3] = { 0 };

	if (jobmask != MAPID_ALL) {
		// Calculate the required bit to set
		uint64 job = 1ULL << ( jobmask & MAPID_BASEMASK );

		// 2-1
		if( ( jobmask & JOBL_2_1 ) != 0 ){
			temp_mask[1] |= job;
		// 2-2
		}else if( ( jobmask & JOBL_2_2 ) != 0 ){
			temp_mask[2] |= job;
		// Basejob
		}else{
			temp_mask[0] |= job;
		}
	} else {
		temp_mask[0] = temp_mask[1] = temp_mask[2] = MAPID_ALL;
	}

	for (int32 i = 0; i < ARRAYLENGTH(temp_mask); i++) {
		if (temp_mask[i] == 0)
			continue;

		if (active)
			bclass[i] |= temp_mask[i];
		else
			bclass[i] &= ~temp_mask[i];
	}
}

/*==========================================
 * Loads an item from the db. If not found, it will return the dummy item.
 * @param nameid
 * @return *item_data or *dummy_item if item not found
 *------------------------------------------*/
struct item_data* itemdb_search(t_itemid nameid) {
	std::shared_ptr<item_data> id;

	if (!(id = item_db.find(nameid))) {
		ShowWarning("itemdb_search: Item ID %u does not exists in the item_db. Using dummy data.\n", nameid);
		id = item_db.find(ITEMID_DUMMY);
	}
	return id.get();
}

/** Checks if item is equip type or not
* @param id Item data
* @return True if item is equip, false otherwise
*/
bool itemdb_isequip2(struct item_data *id) {
	nullpo_ret(id);
	switch (id->type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_AMMO:
		case IT_SHADOWGEAR:
			return true;
		default:
			return false;
	}
}

/** Checks if item is stackable or not
* @param id Item data
* @return True if item is stackable, false otherwise
*/
bool itemdb_isstackable2(struct item_data *id)
{
	nullpo_ret(id);
	return id->isStackable();
}

/*==========================================
 * Trade Restriction functions [Skotlex]
 *------------------------------------------*/
bool itemdb_isdropable_sub(struct item_data *item, int32 gmlv, int32 unused) {
	return (item && (!(item->flag.trade_restriction.drop) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_cantrade_sub(struct item_data* item, int32 gmlv, int32 gmlv2) {
	return (item && (!(item->flag.trade_restriction.trade) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

bool itemdb_canpartnertrade_sub(struct item_data* item, int32 gmlv, int32 gmlv2) {
	return (item && (item->flag.trade_restriction.trade_partner || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

bool itemdb_cansell_sub(struct item_data* item, int32 gmlv, int32 unused) {
	return (item && (!(item->flag.trade_restriction.sell) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_cancartstore_sub(struct item_data* item, int32 gmlv, int32 unused) {
	return (item && (!(item->flag.trade_restriction.cart) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canstore_sub(struct item_data* item, int32 gmlv, int32 unused) {
	return (item && (!(item->flag.trade_restriction.storage) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canguildstore_sub(struct item_data* item, int32 gmlv, int32 unused) {
	return (item && (!(item->flag.trade_restriction.guild_storage) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canmail_sub(struct item_data* item, int32 gmlv, int32 unused) {
	return (item && (!(item->flag.trade_restriction.mail) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canauction_sub(struct item_data* item, int32 gmlv, int32 unused) {
	return (item && (!(item->flag.trade_restriction.auction) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_isrestricted(struct item* item, int32 gmlv, int32 gmlv2, bool (*func)(struct item_data*, int32, int32))
{
	struct item_data* item_data = itemdb_search(item->nameid);
	int32 i;

	if (!func(item_data, gmlv, gmlv2))
		return false;

	if(item_data->slots == 0 || itemdb_isspecial(item->card[0]))
		return true;

	for(i = 0; i < item_data->slots; i++) {
		if (!item->card[i]) continue;
		if (!func(itemdb_search(item->card[i]), gmlv, gmlv2))
			return false;
	}
	return true;
}

bool itemdb_ishatched_egg(struct item* item) {
	if (item && item->card[0] == CARD0_PET && item->attribute == 1)
		return true;
	return false;
}

/** Specifies if item-type should drop unidentified.
* @param nameid ID of item
*/
char itemdb_isidentified(t_itemid nameid) {
	int32 type=itemdb_type(nameid);
	switch (type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETARMOR:
		case IT_SHADOWGEAR:
			return 0;
		default:
			return 1;
	}
}

const std::string ItemGroupDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_group_db.yml";
}

/**
 * Reads and parses an entry from the item_group_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 ItemGroupDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string group_name;

	if (!this->asString(node, "Group", group_name))
		return 0;

	std::string group_name_constant = "IG_" + group_name;
	int64 constant;

	if (!script_get_constant(group_name_constant.c_str(), &constant) || constant < IG_BLUEBOX) {
		if (strncasecmp(group_name.c_str(), "IG_", 3) != 0)
			this->invalidWarning(node["Group"], "Invalid group %s.\n", group_name.c_str());
		else
			this->invalidWarning(node["Group"], "Invalid group %s. Note that 'IG_' is automatically appended to the group name.\n", group_name.c_str());
		return 0;
	}

	uint16 id = static_cast<uint16>(constant);

	std::shared_ptr<s_item_group_db> group = this->find(id);
	bool exists = group != nullptr;

	if (!exists) {
		group = std::make_shared<s_item_group_db>();
		group->id = id;
	}

	if (this->nodeExists(node, "SubGroups")) {
		const auto& subNode = node["SubGroups"];

		for (const auto& subit : subNode) {
			if (this->nodeExists(subit, "Clear")) {
				uint16 id;

				if (!this->asUInt16(subit, "Clear", id))
					continue;

				if (group->random.erase(id) == 0)
					this->invalidWarning(subit["Clear"], "The SubGroup %hu doesn't exist in the group %s. Clear failed.\n", id, group_name.c_str());

				continue;
			}

			if (!this->nodesExist(subit, { "SubGroup", "List" })) {
				continue;
			}

			uint16 subgroup;

			if (!this->asUInt16(subit, "SubGroup", subgroup))
				continue;

			std::shared_ptr<s_item_group_random> random = util::umap_find(group->random, subgroup);
			bool random_exists = random != nullptr;

			if (!random_exists) {
				random = std::make_shared<s_item_group_random>();
				group->random[subgroup] = random;
			}

			if (this->nodeExists(subit, "Algorithm")) {
				std::string sub_str;

				if (!this->asString(subit, "Algorithm", sub_str))
					return 0;

				std::string sub_constant_str = "GROUP_ALGORITHM_" + sub_str;
				int64 constant_str;

				if (!script_get_constant(sub_constant_str.c_str(), &constant_str)) {
					this->invalidWarning(subit["Algorithm"], "Invalid algorithm %s.\n", sub_str.c_str());
					continue;
				}

				random->algorithm = static_cast<e_group_algorithm_type>(constant_str);
			} else {
				if (!random_exists) {
					random->algorithm = GROUP_ALGORITHM_SHAREDPOOL;
				}
			}

			const auto& listNode = subit["List"];

			for (const auto& listit : listNode) {
				uint32 index;

				if (!this->asUInt32(listit, "Index", index))
					continue;

				if (this->nodeExists(listit, "Clear")) {
					bool active;

					if (!this->asBool(listit, "Clear", active) || !active)
						continue;

					if (random->data.erase(index) == 0)
						this->invalidWarning(listit["Clear"], "Index %u doesn't exist in the SubGroup %hu (group %s). Clear failed.\n", index, subgroup, group_name.c_str());

					continue;
				}

				std::shared_ptr<s_item_group_entry> entry = util::umap_find(random->data, index);
				bool entry_exists = entry != nullptr;

				if (!entry_exists) {
					if (!this->nodesExist(listit, { "Item" }))
						return 0;

					entry = std::make_shared<s_item_group_entry>();
					random->data[index] = entry;
				}

				std::shared_ptr<item_data> item = nullptr;

				if (this->nodeExists(listit, "Item")) {
					std::string item_name;

					if (!this->asString(listit, "Item", item_name))
						continue;

					item = item_db.search_aegisname( item_name.c_str() );

					if (item == nullptr) {
						this->invalidWarning(listit["Item"], "Unknown Item %s.\n", item_name.c_str());
						continue;
					}
				} else {
					if (!entry_exists) {
						item = item_db.find( entry->nameid );
					}
				}

				// (shouldn't happen)
				if (item == nullptr) {
					this->invalidWarning(listit["index"], "Missing Item definition for Index %u.\n", index);
					continue;
				}

				entry->nameid = item->nameid;

				if (this->nodeExists(listit, "Rate")) {
					uint16 rate;

					if (!this->asUInt16(listit, "Rate", rate))
						continue;

					entry->rate = rate;
				} else {
					if (!entry_exists)
						entry->rate = 0;
				}

				if (random->algorithm == GROUP_ALGORITHM_ALL && entry->rate > 0) {
					this->invalidWarning(listit["Item"], "Item cannot have a rate with \"All\" algorithm. Defaulting Rate to 0.\n");
					entry->rate = 0;
				}
				if (random->algorithm != GROUP_ALGORITHM_ALL && entry->rate == 0) {
					this->invalidWarning(listit["Item"], "Missing rate, item skipped.\n");
					continue;
				}

				// Adjusted rate
				entry->adj_rate = (entry->rate * battle_config.item_group_rate) / 100;
				entry->adj_rate = cap_value(entry->adj_rate, battle_config.item_group_drop_min, battle_config.item_group_drop_max);

				// Reset amount given
				entry->given = 0;

				if (this->nodeExists(listit, "Amount")) {
					uint16 amount;

					if (!this->asUInt16(listit, "Amount", amount))
						continue;

					entry->amount = cap_value(amount, 1, MAX_AMOUNT);
				} else {
					if (!entry_exists)
						entry->amount = 1;
				}

				if (this->nodeExists(listit, "Duration")) {
					uint16 duration;

					if (!this->asUInt16(listit, "Duration", duration))
						continue;

					entry->duration = duration;
				} else {
					if (!entry_exists)
						entry->duration = 0;
				}

				if (this->nodeExists(listit, "Announced")) {
					bool isAnnounced;

					if (!this->asBool(listit, "Announced", isAnnounced))
						continue;

					entry->isAnnounced = isAnnounced;
				} else {
					if (!entry_exists)
						entry->isAnnounced = false;
				}

				if (this->nodeExists(listit, "UniqueId")) {
					bool guid;

					if (!this->asBool(listit, "UniqueId", guid))
						continue;

					entry->GUID = guid;
				} else {
					if (!entry_exists)
						entry->GUID = item->flag.guid;
				}

				if (this->nodeExists(listit, "Stacked")) {
					bool isStacked;

					if (!this->asBool(listit, "Stacked", isStacked))
						continue;

					entry->isStacked = isStacked;
				} else {
					if (!entry_exists)
						entry->isStacked = true;
				}

				if (this->nodeExists(listit, "Named")) {
					bool named;

					if (!this->asBool(listit, "Named", named))
						continue;

					entry->isNamed = named;
				} else {
					if (!entry_exists)
						entry->isNamed = false;
				}

				if (this->nodeExists(listit, "Bound")) {
					std::string bound_name;

					if (!this->asString(listit, "Bound", bound_name))
						continue;

					std::string bound_name_constant = "BOUND_" + bound_name;
					int64 bound;

					if (!script_get_constant(bound_name_constant.c_str(), &bound) || bound < BOUND_NONE || bound >= BOUND_MAX) {
						this->invalidWarning(listit["Group"], "Invalid Bound %s.\n", bound_name.c_str());
						continue;
					}

					entry->bound = static_cast<uint8>(bound);
				} else {
					if (!entry_exists)
						entry->bound = BOUND_NONE;
				}

				if( this->nodeExists( listit, "RandomOptionGroup" ) ){
					std::string name;

					if( !this->asString( listit, "RandomOptionGroup", name ) ){
						return 0;
					}

					uint16 id;

					if( !random_option_group.option_get_id( name, id ) ){
						this->invalidWarning( listit["RandomOptionGroup"], "Unknown random option group \"%s\".\n", name.c_str() );
						return 0;
					}

					entry->randomOptionGroup = random_option_group.find( id );
				}else{
					if( !entry_exists ){
						entry->randomOptionGroup = nullptr;
					}
				}

				if( this->nodeExists( listit, "RefineMinimum" ) ){
					uint16 refine;

					if( !this->asUInt16( listit, "RefineMinimum", refine ) ){
						return 0;
					}

					if( refine > MAX_REFINE ){
						this->invalidWarning( listit["RefineMinimum"], "Minimum refine % hu is too high, capping to MAX_REFINE...\n", refine );
						refine = MAX_REFINE;
					}

					entry->refineMinimum = refine;
				}else{
					if( !exists ){
						entry->refineMinimum = 0;
					}
				}

				if( this->nodeExists( listit, "RefineMaximum" ) ){
					uint16 refine;

					if( !this->asUInt16( listit, "RefineMaximum", refine ) ){
						return 0;
					}

					if( refine > MAX_REFINE ){
						this->invalidWarning( listit["RefineMaximum"], "Maximum refine %hu is too high, capping to MAX_REFINE...\n", refine );
						refine = MAX_REFINE;
					}

					entry->refineMaximum = refine;
				}else{
					if( !exists ){
						entry->refineMaximum = 0;
					}
				}
			}
		}
	}

	if (!exists)
		this->put(id, group);

	return 1;
}

void ItemGroupDatabase::loadingFinished() {
	// Delete empty sub groups
	for( const auto &group : *this ){
		for( auto it = group.second->random.begin(); it != group.second->random.end(); ){
			if( it->second->data.empty() ){
				ShowDebug( "Deleting empty subgroup %u from item group %hu.\n", it->first, group.first );
				it = group.second->random.erase( it );
			}else{
				it++;
			}
		}
	}

	// Calculate rates
	for (const auto &group : *this) {
		for (const auto &random : group.second->random) {
			random.second->total_rate = 0;
			random.second->total_given = 0;
			for (const auto &it : random.second->data) {
				random.second->total_rate += it.second->rate;
			}
		}
	}

	TypesafeYamlDatabase::loadingFinished();
}

/** Read item forbidden by mapflag (can't equip item)
* Structure: <nameid>,<mode>
*/
static bool itemdb_read_noequip( char* str[], size_t columns, size_t current ){
	t_itemid nameid;
	int32 flag;

	nameid = strtoul(str[0], nullptr, 10);
	flag = atoi(str[1]);

	std::shared_ptr<item_data> id = item_db.find(nameid);

	if( id == nullptr )
	{
		ShowWarning("itemdb_read_noequip: Invalid item id %u.\n", nameid);
		return false;
	}

	if (flag >= 0)
		id->flag.no_equip |= flag;
	else
		id->flag.no_equip &= ~abs(flag);

	return true;
}

const std::string ComboDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_combos.yml";
}

uint16 ComboDatabase::find_combo_id( const std::vector<t_itemid>& items ){
	for (const auto &it : *this) {
		if (it.second->nameid == items) {
			return it.first;
		}
	}

	return 0;
}

/**
 * Reads and parses an entry from the item_combos.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 ComboDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::vector<std::vector<t_itemid>> items_list;

	if( !this->nodesExist( node, { "Combos" } ) ){
		return 0;
	}

	const ryml::NodeRef& combosNode = node["Combos"];

	for (const auto& comboit : combosNode) {
		static const std::string nodeName = "Combo";

		if (!this->nodesExist(comboit, { nodeName })) {
			return 0;
		}

		const ryml::NodeRef& comboNode = comboit["Combo"];

		if (!comboNode.is_seq()) {
			this->invalidWarning(comboNode, "%s should be a sequence.\n", nodeName.c_str());
			return 0;
		}

		std::vector<t_itemid> items = {};

		for (const auto it : comboNode) {
			std::string item_name;
			c4::from_chars(it.val(), &item_name);

			std::shared_ptr<item_data> item = item_db.search_aegisname(item_name.c_str());

			if (item == nullptr) {
				this->invalidWarning(it, "Invalid item %s, skipping.\n", item_name.c_str());
				return 0;
			}

			items.push_back(item->nameid);
		}

		if (items.empty()) {
			this->invalidWarning(comboNode, "Empty combo, skipping.\n");
			return 0;
		}

		if (items.size() < 2) {
			this->invalidWarning(comboNode, "Not enough item to make a combo (need at least 2). Skipping.\n");
			return 0;
		}

		std::sort(items.begin(), items.end());
		items_list.push_back(items);
	}

	if (items_list.empty()) {
		this->invalidWarning(combosNode, "No combos defined, skipping.\n");
		return 0;
	}

	if (this->nodeExists(node, "Clear")) {
		bool clear = false;

		if (!this->asBool(node, "Clear", clear))
			return 0;

		// Remove the combo (if exists)
		if (clear) {
			for (const auto& itemsit : items_list) {
				uint16 id = this->find_combo_id(itemsit);

				if (id == 0) {
					this->invalidWarning(node["Clear"], "Unable to clear the combo.\n");
					return 0;
				}

				this->erase(id);
			}

			return 1;
		}
	}

	uint64 count = 0;

	for (const auto &itemsit : items_list) {
		// Find the id when the combo exists
		uint16 id = this->find_combo_id(itemsit);
		std::shared_ptr<s_item_combo> combo = this->find(id);
		bool exists = combo != nullptr;

		if (!exists) {
			combo = std::make_shared<s_item_combo>();

			combo->nameid.insert(combo->nameid.begin(), itemsit.begin(), itemsit.end());
			combo->id = ++this->combo_num;
		}

		if (this->nodeExists(node, "Script")) {
			std::string script;

			if (!this->asString(node, "Script", script))
				return 0;

			if (exists) {
				script_free_code(combo->script);
				combo->script = nullptr;
			}
			combo->script = parse_script(script.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["Script"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
		} else {
			if (!exists) {
				combo->script = nullptr;
			}
		}

		if (!exists)
			this->put( combo->id, combo );

		count++;
	}

	return count;
}

void ComboDatabase::loadingFinished() {
	// Populate item_data to refer to the combo
	for (const auto &combo : *this) {
		for (const auto &itm : combo.second->nameid) {
			std::shared_ptr<item_data> it = item_db.find(itm);

			if (it != nullptr)
				it->combos.push_back(combo.second);
		}
	}

	TypesafeYamlDatabase::loadingFinished();
}

/**
 * Process Roulette items
 */
bool itemdb_parse_roulette_db(void)
{
	int32 i, j;
	uint32 count = 0;

	// retrieve all rows from the item database
	if (SQL_ERROR == Sql_Query(mmysql_handle, "SELECT * FROM `%s`", roulette_table)) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++)
		rd.items[i] = 0;

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		int32 k, limit = MAX_ROULETTE_COLUMNS - i;

		for (k = 0; k < limit && SQL_SUCCESS == Sql_NextRow(mmysql_handle); k++) {
			char* data;
			t_itemid item_id;
			uint16 amount;
			int32 level, flag;

			Sql_GetData(mmysql_handle, 1, &data, nullptr); level = atoi(data);
			Sql_GetData(mmysql_handle, 2, &data, nullptr); item_id = strtoul(data, nullptr, 10);
			Sql_GetData(mmysql_handle, 3, &data, nullptr); amount = atoi(data);
			Sql_GetData(mmysql_handle, 4, &data, nullptr); flag = atoi(data);

			if (!item_db.exists(item_id)) {
				ShowWarning("itemdb_parse_roulette_db: Unknown item ID '%u' in level '%d'\n", item_id, level);
				continue;
			}
			if (amount < 1 || amount > MAX_AMOUNT){
				ShowWarning("itemdb_parse_roulette_db: Unsupported amount '%hu' for item ID '%u' in level '%d'\n", amount, item_id, level);
				continue;
			}
			if (flag < 0 || flag > 1) {
				ShowWarning("itemdb_parse_roulette_db: Unsupported flag '%d' for item ID '%u' in level '%d'\n", flag, item_id, level);
				continue;
			}

			j = rd.items[i];
			RECREATE(rd.nameid[i], t_itemid, ++rd.items[i]);
			RECREATE(rd.qty[i], uint16, rd.items[i]);
			RECREATE(rd.flag[i], int32, rd.items[i]);

			rd.nameid[i][j] = item_id;
			rd.qty[i][j] = amount;
			rd.flag[i][j] = flag;

			++count;
		}
	}

	// free the query result
	Sql_FreeResult(mmysql_handle);

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		int32 limit = MAX_ROULETTE_COLUMNS - i;

		if (rd.items[i] == limit)
			continue;

		if (rd.items[i] > limit) {
			ShowWarning("itemdb_parse_roulette_db: level %d has %d items, only %d supported, capping...\n", i + 1, rd.items[i], limit);
			rd.items[i] = limit;
			continue;
		}

		/** this scenario = rd.items[i] < limit **/
		ShowWarning("itemdb_parse_roulette_db: Level %d has %d items, %d are required. Filling with Apples...\n", i + 1, rd.items[i], limit);

		rd.items[i] = limit;
		RECREATE(rd.nameid[i], t_itemid, rd.items[i]);
		RECREATE(rd.qty[i], uint16, rd.items[i]);
		RECREATE(rd.flag[i], int32, rd.items[i]);

		for (j = 0; j < MAX_ROULETTE_COLUMNS - i; j++) {
			if (rd.qty[i][j])
				continue;

			rd.nameid[i][j] = ITEMID_APPLE;
			rd.qty[i][j] = 1;
			rd.flag[i][j] = 0;
		}
	}

	ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, roulette_table);

	return true;
}

/**
 * Free Roulette items
 */
static void itemdb_roulette_free(void) {
	int32 i;

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		if (rd.nameid[i])
			aFree(rd.nameid[i]);
		if (rd.qty[i])
			aFree(rd.qty[i]);
		if (rd.flag[i])
			aFree(rd.flag[i]);
		rd.nameid[i] = nullptr;
		rd.qty[i] = nullptr;
		rd.flag[i] = nullptr;
		rd.items[i] = 0;
	}
}

/**
 * Convert SQL data to YAML Node
 * @param str: Array of parsed SQL data
 * @return True on success or false otherwise
 */
static bool itemdb_read_sqldb_sub(std::vector<std::string> str) {
	ryml::Tree tree;
	ryml::NodeRef rootNode = tree.rootref();
	rootNode |= ryml::MAP;
	int32 index = -1;

	rootNode["Id"] << str[++index];
	if (!str[++index].empty())
		rootNode["AegisName"] << str[index];
	if (!str[++index].empty())
		rootNode["Name"] << str[index];
	if (!str[++index].empty())
		rootNode["Type"] << str[index];
	if (!str[++index].empty())
		rootNode["SubType"] << str[index];
	if (!str[++index].empty())
		rootNode["Buy"] << str[index];
	if (!str[++index].empty())
		rootNode["Sell"] << str[index];
	if (!str[++index].empty())
		rootNode["Weight"] << str[index];
	if (!str[++index].empty())
		rootNode["Attack"] << str[index];
	if (!str[++index].empty())
		rootNode["Defense"] << str[index];
	if (!str[++index].empty())
		rootNode["Range"] << str[index];
	if (!str[++index].empty())
		rootNode["Slots"] << str[index];

	ryml::NodeRef jobs = rootNode["Jobs"];
	jobs |= ryml::MAP;

	if (!str[++index].empty())
		jobs["All"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Acolyte"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Alchemist"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Archer"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Assassin"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["BardDancer"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Blacksmith"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Crusader"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Gunslinger"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Hunter"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Knight"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Mage"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Merchant"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Monk"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Ninja"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Novice"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Priest"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Rogue"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Sage"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["SoulLinker"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["StarGladiator"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["SuperNovice"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Swordman"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Taekwon"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Thief"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Wizard"] << (std::stoi(str[index]) ? "true" : "false");

	ryml::NodeRef classes = rootNode["Classes"];
	classes |= ryml::MAP;

	if (!str[++index].empty())
		classes["All"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		classes["Normal"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		classes["Upper"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		classes["Baby"] << (std::stoi(str[index]) ? "true" : "false");

	if (!str[++index].empty())
		rootNode["Gender"] << str[index];

	ryml::NodeRef locations = rootNode["Locations"];
	locations |= ryml::MAP;

	if (!str[++index].empty())
		locations["Head_Top"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Head_Mid"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Head_Low"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Armor"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Right_Hand"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Left_Hand"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Garment"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Shoes"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Right_Accessory"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Left_Accessory"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Costume_Head_Top"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Costume_Head_Mid"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Costume_Head_Low"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Costume_Garment"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Ammo"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Shadow_Armor"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Shadow_Weapon"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Shadow_Shield"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Shadow_Shoes"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Shadow_Right_Accessory"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		locations["Shadow_Left_Accessory"] << (std::stoi(str[index]) ? "true" : "false");

	if (!str[++index].empty())
		rootNode["WeaponLevel"] << str[index];
	if (!str[++index].empty())
		rootNode["ArmorLevel"] << str[index];
	if (!str[++index].empty())
		rootNode["EquipLevelMin"] << str[index];
	if (!str[++index].empty())
		rootNode["EquipLevelMax"] << str[index];
	if (!str[++index].empty())
		rootNode["Refineable"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		rootNode["View"] << str[index];
	if (!str[++index].empty())
		rootNode["AliasName"] << str[index];

	ryml::NodeRef flags = rootNode["Flags"];
	flags |= ryml::MAP;

	if (!str[++index].empty())
		flags["BuyingStore"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		flags["DeadBranch"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		flags["Container"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		flags["UniqueId"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		flags["BindOnEquip"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		flags["DropAnnounce"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		flags["NoConsume"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		flags["DropEffect"] << str[index];
	rootNode["Flags"] = flags;

	ryml::NodeRef delay = rootNode["Delay"];
	delay |= ryml::MAP;

	if (!str[++index].empty())
		delay["Duration"] << str[index];
	if (!str[++index].empty())
		delay["Status"] << str[index];

	ryml::NodeRef stack = rootNode["Stack"];
	stack |= ryml::MAP;

	if (!str[++index].empty())
		stack["Amount"] << str[index];
	if (!str[++index].empty())
		stack["Inventory"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		stack["Cart"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		stack["Storage"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		stack["GuildStorage"] << (std::stoi(str[index]) ? "true" : "false");


	ryml::NodeRef nouse = rootNode["NoUse"];
	nouse |= ryml::MAP;

	if (!str[++index].empty())
		nouse["Override"] << str[index];
	if (!str[++index].empty())
		nouse["Sitting"] << (std::stoi(str[index]) ? "true" : "false");

	ryml::NodeRef trade = rootNode["Trade"];
	trade |= ryml::MAP;

	if (!str[++index].empty())
		trade["Override"] << str[index];
	if (!str[++index].empty())
		trade["NoDrop"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["NoTrade"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["TradePartner"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["NoSell"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["NoCart"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["NoStorage"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["NoGuildStorage"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["NoMail"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		trade["NoAuction"] << (std::stoi(str[index]) ? "true" : "false");

	if (!str[++index].empty())
		rootNode["Script"] << str[index];
	if (!str[++index].empty())
		rootNode["EquipScript"] << str[index];
	if (!str[++index].empty())
		rootNode["UnEquipScript"] << str[index];

#ifdef RENEWAL
	if (!str[++index].empty())
		rootNode["MagicAttack"] << str[index];
	if (!str[++index].empty())
		classes["Third"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		classes["Third_Upper"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		classes["Third_Baby"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		classes["Fourth"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["KagerouOboro"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Rebellion"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Summoner"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		jobs["Spirit_Handler"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		rootNode["Gradable"] << (std::stoi(str[index]) ? "true" : "false");
#endif

	if( !jobs.has_children() ){
		rootNode.remove_child( jobs );
	}

	if( !classes.has_children() ){
		rootNode.remove_child( classes );
	}

	if( !locations.has_children() ){
		rootNode.remove_child( locations );
	}

	if( !flags.has_children() ){
		rootNode.remove_child( flags );
	}

	if( !delay.has_children() ){
		rootNode.remove_child( delay );
	}

	if( !stack.has_children() ){
		rootNode.remove_child( stack );
	}

	if( !nouse.has_children() ){
		rootNode.remove_child( nouse );
	}

	if( !trade.has_children() ){
		rootNode.remove_child( trade );
	}

	return item_db.parseBodyNode(rootNode) > 0;
}

/**
 * Read SQL item_db table
 */
static int32 itemdb_read_sqldb(void) {
	const char* item_db_name[] = {
		item_table,
		item2_table
	};

	for( uint8 fi = 0; fi < ARRAYLENGTH(item_db_name); ++fi ) {
		// retrieve all rows from the item database
		if( SQL_ERROR == Sql_Query(mmysql_handle, "SELECT `id`,`name_aegis`,`name_english`,`type`,`subtype`,`price_buy`,`price_sell`,`weight`,`attack`,`defense`,`range`,`slots`,"
			"`job_all`,`job_acolyte`,`job_alchemist`,`job_archer`,`job_assassin`,`job_barddancer`,`job_blacksmith`,`job_crusader`,`job_gunslinger`,`job_hunter`,`job_knight`,`job_mage`,`job_merchant`,"
			"`job_monk`,`job_ninja`,`job_novice`,`job_priest`,`job_rogue`,`job_sage`,`job_soullinker`,`job_stargladiator`,`job_supernovice`,`job_swordman`,`job_taekwon`,`job_thief`,`job_wizard`,"
			"`class_all`,`class_normal`,`class_upper`,`class_baby`,`gender`,"
			"`location_head_top`,`location_head_mid`,`location_head_low`,`location_armor`,`location_right_hand`,`location_left_hand`,`location_garment`,`location_shoes`,`location_right_accessory`,`location_left_accessory`,"
			"`location_costume_head_top`,`location_costume_head_mid`,`location_costume_head_low`,`location_costume_garment`,`location_ammo`,`location_shadow_armor`,`location_shadow_weapon`,`location_shadow_shield`,`location_shadow_shoes`,`location_shadow_right_accessory`,`location_shadow_left_accessory`,"
			"`weapon_level`,`armor_level`,`equip_level_min`,`equip_level_max`,`refineable`,`view`,`alias_name`,"
			"`flag_buyingstore`,`flag_deadbranch`,`flag_container`,`flag_uniqueid`,`flag_bindonequip`,`flag_dropannounce`,`flag_noconsume`,`flag_dropeffect`,"
			"`delay_duration`,`delay_status`,`stack_amount`,`stack_inventory`,`stack_cart`,`stack_storage`,`stack_guildstorage`,`nouse_override`,`nouse_sitting`,"
			"`trade_override`,`trade_nodrop`,`trade_notrade`,`trade_tradepartner`,`trade_nosell`,`trade_nocart`,`trade_nostorage`,`trade_noguildstorage`,`trade_nomail`,`trade_noauction`,`script`,`equip_script`,`unequip_script`"
#ifdef RENEWAL
			",`magic_attack`,`class_third`,`class_third_upper`,`class_third_baby`,`class_fourth`,`job_kagerouoboro`,`job_rebellion`,`job_summoner`,`job_spirit_handler`,`gradable`"
#endif
			" FROM `%s`", item_db_name[fi]) ) {
			Sql_ShowDebug(mmysql_handle);
			continue;
		}

		uint32 total_columns = Sql_NumColumns(mmysql_handle);
		uint64 total_rows = Sql_NumRows(mmysql_handle), rows = 0, count = 0;

		ShowStatus("Loading '" CL_WHITE "%" PRIdPTR CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'\n", total_rows, item_db_name[fi]);

		// process rows one by one
		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
#ifdef DETAILED_LOADING_OUTPUT
			ShowStatus( "Loading [%" PRIu64 "/%" PRIu64 "] entries in '" CL_WHITE "%s" CL_RESET "'" CL_CLL "\r", ++rows, total_rows, item_db_name[fi] );
#endif
			std::vector<std::string> data = {};

			for( uint32 i = 0; i < total_columns; ++i ) {
				char *str;

				Sql_GetData(mmysql_handle, i, &str, nullptr);
				if (str == nullptr)
					data.push_back("");
				else
					data.push_back(str);
			}

			if (!itemdb_read_sqldb_sub(data))
				continue;
			++count;
		}

		// free the query result
		Sql_FreeResult(mmysql_handle);

		ShowStatus("Done reading '" CL_WHITE "%" PRIu64 CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, item_db_name[fi]);
	}

	item_db.loadingFinished();

	return 0;
}

/** Check if the item is restricted by item_noequip.txt
* @param id Item that will be checked
* @param m Map ID
* @return true: can't be used; false: can be used
*/
bool itemdb_isNoEquip(struct item_data *id, uint16 m) {
	if (!id->flag.no_equip)
		return false;
	
	struct map_data *mapdata = map_getmapdata(m);

	if ((id->flag.no_equip&1 && !mapdata_flag_vs2(mapdata)) || // Normal
		(id->flag.no_equip&2 && mapdata->getMapFlag(MF_PVP)) || // PVP
		(id->flag.no_equip&4 && mapdata_flag_gvg2_no_te(mapdata)) || // GVG
		(id->flag.no_equip&8 && mapdata->getMapFlag(MF_BATTLEGROUND)) || // Battleground
		(id->flag.no_equip&16 && mapdata_flag_gvg2_te(mapdata)) || // WOE:TE
		(id->flag.no_equip&(mapdata->zone) && mapdata->getMapFlag(MF_RESTRICTED)) // Zone restriction
		)
		return true;
	return false;
}

const std::string RandomOptionDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_randomopt_db.yml";
}

/**
 * Reads and parses an entry from the item_randomopt_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 RandomOptionDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint16 id;

	if (!this->asUInt16(node, "Id", id))
		return 0;

	std::shared_ptr<s_random_opt_data> randopt = this->find(id);
	bool exists = randopt != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Option", "Script" }))
			return 0;

		randopt = std::make_shared<s_random_opt_data>();
		randopt->id = id;
	}

	if (this->nodeExists(node, "Option")) {
		std::string name;

		if (!this->asString(node, "Option", name))
			return 0;

		if (randopt->name.compare(name) != 0 && random_option_db.option_exists(name)) {
			this->invalidWarning(node["Option"], "Found duplicate random option name for %s, skipping.\n", name.c_str());
			return 0;
		}

		randopt->name = name;
	}

	if (this->nodeExists(node, "Script")) {
		std::string script;

		if (!this->asString(node, "Script", script))
			return 0;

		if (randopt->script) {
			script_free_code( randopt->script );
			randopt->script = nullptr;
		}

		randopt->script = parse_script(script.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["Script"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	}

	if (!exists)
		this->put(id, randopt);

	return 1;
}

void RandomOptionDatabase::loadingFinished(){
	const char* prefix = "RDMOPT_";

	for( const auto& pair : *this ){
		std::string name = prefix + pair.second->name;
		int64 constant;

		// Check if it has already been set
		if( script_get_constant( name.c_str(), &constant ) ){
			// It is already the same
			if( constant == pair.first ){
				continue;
			}else{
				// Export it to the script engine -> will issue a warning
			}
		}

		script_set_constant( name.c_str(), pair.first, false, false );
	}

	TypesafeYamlDatabase::loadingFinished();
}

RandomOptionDatabase random_option_db;

/**
 * Check if the given random option name exists.
 * @param name: Random option name
 * @return True on success or false on failure
 */
bool RandomOptionDatabase::option_exists(std::string name) {
	for (const auto &opt : random_option_db) {
		if (opt.second->name.compare(name) == 0)
			return true;
	}

	return false;
}

/**
 * Return the constant value of the given random option.
 * @param name: Random option name
 * @param id: Random option ID
 * @return True on success or false on failure
 */
bool RandomOptionDatabase::option_get_id(std::string name, uint16 &id) {
	for (const auto &opt : random_option_db) {
		if (opt.second->name.compare(name) == 0) {
			id = opt.first;
			return true;
		}
	}

	return false;
}

const std::string RandomOptionGroupDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_randomopt_group.yml";
}

bool RandomOptionGroupDatabase::add_option(const ryml::NodeRef& node, std::shared_ptr<s_random_opt_group_entry> &entry) {
	uint16 option_id;

	if (this->nodeExists(node, "Option")) {
		std::string opt_name;

		if (!this->asString(node, "Option", opt_name))
			return false;

		if (!random_option_db.option_get_id(opt_name, option_id)) {
			this->invalidWarning(node["Option"], "Invalid Random Option name %s given.\n", opt_name.c_str());
			return false;
		}
	} else {
		this->invalidWarning(node, "Missing Option node.\n");
		return false;
	}

	entry = std::make_shared<s_random_opt_group_entry>();
	entry->id = option_id;

	if (this->nodeExists(node, "MinValue")) {
		int16 value;

		if (!this->asInt16(node, "MinValue", value))
			return false;

		entry->min_value = value;
	} else {
		entry->min_value = 0;
	}

	if (this->nodeExists(node, "MaxValue")) {
		int16 value;

		if (!this->asInt16(node, "MaxValue", value))
			return false;

		entry->max_value = value;
	} else {
		entry->max_value = 0;
	}

	if (entry->min_value > entry->max_value) {
		this->invalidWarning(node["MaxValue"], "MinValue %d is greater than MaxValue %d, setting MaxValue to MinValue + 1.\n", entry->min_value, entry->max_value);
		entry->max_value = entry->min_value + 1;
	}

	if (this->nodeExists(node, "Param")) {
		int16 value;

		if (!this->asInt16(node, "Param", value))
			return false;

		entry->param = static_cast<int8>(value);
	} else {
		entry->param = 0;
	}

	if (this->nodeExists(node, "Chance")) {
		uint16 chance;

		if (!this->asUInt16Rate(node, "Chance", chance))
			return false;

		entry->chance = chance;
	} else {
		entry->chance = 0;
	}

	return true;
}

void s_random_opt_group::apply( struct item& item ){
	auto apply_sub = []( s_item_randomoption& item_option, const std::shared_ptr<s_random_opt_group_entry>& option ){
		item_option.id = option->id;
		item_option.value = rnd_value( option->min_value, option->max_value );
		item_option.param = option->param;
	};

	// (Re)initialize all the options
	for( size_t i = 0; i < MAX_ITEM_RDM_OPT; i++ ){
		item.option[i].id = 0;
		item.option[i].value = 0;
		item.option[i].param = 0;
	};

	// Apply Must options
	for( size_t i = 0; i < this->slots.size(); i++ ){
		// Try to apply an entry
		for( size_t j = 0, max = this->slots[static_cast<uint16>(i)].size() * 3; j < max; j++ ){
			std::shared_ptr<s_random_opt_group_entry> option = util::vector_random( this->slots[static_cast<uint16>(i)] );

			if ( rnd_chance<uint16>(option->chance, 10000) ) {
				apply_sub( item.option[i], option );
				break;
			}
		}

		// If no entry was applied, assign one
		if( item.option[i].id == 0 ){
			std::shared_ptr<s_random_opt_group_entry> option = util::vector_random( this->slots[static_cast<uint16>(i)] );

			// Apply an entry without checking the chance
			apply_sub( item.option[i], option );
		}
	}

	// Apply Random options (if available)
	if( this->max_random > 0 ){
		for( size_t i = 0; i < min( this->max_random, MAX_ITEM_RDM_OPT ); i++ ){
			// If item already has an option in this slot, skip it
			if( item.option[i].id > 0 ){
				continue;
			}

			std::shared_ptr<s_random_opt_group_entry> option = util::vector_random( this->random_options );

			if ( rnd_chance<uint16>(option->chance, 10000) ){
				apply_sub( item.option[i], option );
			}
		}
	}

	// Fix any gaps, the client cannot handle this
	for( size_t i = 0; i < MAX_ITEM_RDM_OPT; i++ ){
		// If an option is empty
		if( item.option[i].id == 0 ){
			// Check if any other options, after the empty option exist
			size_t j;
			for( j = i + 1; j < MAX_ITEM_RDM_OPT; j++ ){
				if( item.option[j].id != 0 ){
					break;
				}
			}

			// Another option was found, after the empty option
			if( j < MAX_ITEM_RDM_OPT ){
				// Move the later option forward
				item.option[i].id = item.option[j].id;
				item.option[i].value = item.option[j].value;
				item.option[i].param = item.option[j].param;

				// Reset the option that was moved
				item.option[j].id = 0;
				item.option[j].value = 0;
				item.option[j].param = 0;
			}else{
				// Cancel early
				break;
			}
		}
	}
}

/**
 * Reads and parses an entry from the item_randomopt_group.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 RandomOptionGroupDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint16 id;

	if (!this->asUInt16(node, "Id", id))
		return 0;

	std::shared_ptr<s_random_opt_group> randopt = this->find(id);
	bool exists = randopt != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Group" }))
			return 0;

		randopt = std::make_shared<s_random_opt_group>();
		randopt->id = id;
	}

	if (this->nodeExists(node, "Group")) {
		std::string name;

		if (!this->asString(node, "Group", name))
			return 0;

		if (randopt->name.compare(name) != 0 && random_option_group.option_exists(name)) {
			this->invalidWarning(node["Group"], "Found duplicate random option group name for %s, skipping.\n", name.c_str());
			return 0;
		}

		randopt->name = name;
	}

	if (this->nodeExists(node, "Slots")) {
		const auto& slotsNode = node["Slots"];
		for (const ryml::NodeRef& slotNode : slotsNode) {
			if (randopt->slots.size() >= MAX_ITEM_RDM_OPT) {
				this->invalidWarning(slotNode, "Reached maximum of %d Random Option group options. Skipping the remaining slots...\n", MAX_ITEM_RDM_OPT);
				break;
			}

			uint16 slot;

			if (!this->asUInt16(slotNode, "Slot", slot))
				return 0;

			if (slot < 1 || slot > MAX_ITEM_RDM_OPT) {
				this->invalidWarning(slotNode["Slot"], "Invalid Random Opton Slot number %hu given, must be between 1~%d, skipping.\n", slot, MAX_ITEM_RDM_OPT);
				return 0;
			}

			if (!this->nodeExists(slotNode, "Options")) {
				this->invalidWarning(slotNode, "Random option slot does not contain Options node, skipping.\n");
				return 0;
			}

			std::vector<std::shared_ptr<s_random_opt_group_entry>> entries;
			const auto& optionsNode = slotNode["Options"];
			for (const auto& optionNode : optionsNode) {
				std::shared_ptr<s_random_opt_group_entry> entry;

				if (!this->add_option(optionNode, entry))
					return 0;

				entries.push_back(entry);
			}

			randopt->slots[slot - 1] = entries;
		}
	}

	if (this->nodeExists(node, "MaxRandom")) {
		uint16 max;

		if (!this->asUInt16(node, "MaxRandom", max))
			return 0;

		if (max > MAX_ITEM_RDM_OPT) {
			this->invalidWarning(node["MaxRandom"], "Exceeds the maximum of %d Random Option group options, capping to MAX_ITEM_RDM_OPT.\n", MAX_ITEM_RDM_OPT);
			max = MAX_ITEM_RDM_OPT;
		}

		randopt->max_random = max;
	} else {
		if (!exists)
			randopt->max_random = 0;
	}

	if (this->nodeExists(node, "Random")) {
		randopt->random_options.clear();

		const auto& randomNode = node["Random"];
		for (const auto& randomNode : randomNode) {
			std::shared_ptr<s_random_opt_group_entry> entry;

			if (!this->add_option(randomNode, entry))
				return 0;

			randopt->random_options.push_back(entry);
		}
	}

	if (!exists)
		this->put(id, randopt);

	return 1;
}

RandomOptionGroupDatabase random_option_group;

/**
 * Check if the given random option group name exists.
 * @param name: Random option name
 * @return True on success or false on failure
 */
bool RandomOptionGroupDatabase::option_exists(std::string name) {
	for (const auto &opt : random_option_group) {
		if (opt.second->name.compare(name) == 0)
			return true;
	}

	return false;
}

/**
 * Return the constant value of the given random option group.
 * @param name: Random option group name
 * @param id: Random option group ID
 * @return True on success or false on failure
 */
bool RandomOptionGroupDatabase::option_get_id(std::string name, uint16 &id) {
	for (const auto &opt : random_option_group) {
		if (opt.second->name.compare(name) == 0) {
			id = opt.first;
			return true;
		}
	}

	return false;
}

/**
* Read all item-related databases
*/
static void itemdb_read(void) {
	int32 i;
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
	};
	
	if (db_use_sqldbs)
		itemdb_read_sqldb();
	else
		item_db.load();
	
	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		uint8 n1 = (uint8)(strlen(db_path)+strlen(dbsubpath[i])+1);
		uint8 n2 = (uint8)(strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1);
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

		sv_readdb(dbsubpath2, "item_noequip.txt",       ',', 2, 2, -1, &itemdb_read_noequip, i > 0);
		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}

	random_option_db.load();
	random_option_group.load();
	itemdb_group.load();
	itemdb_combo.load();
	laphine_synthesis_db.load();
	laphine_upgrade_db.load();
	item_reform_db.load();
	item_enchant_db.load();
	item_package_db.load();

	if (battle_config.feature_roulette)
		itemdb_parse_roulette_db();
}

/*==========================================
 * Initialize / Finalize
 *------------------------------------------*/

bool item_data::isStackable()
{
	switch (this->type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETEGG:
		case IT_PETARMOR:
		case IT_SHADOWGEAR:
			return false;
	}
	return true;
}

int32 item_data::inventorySlotNeeded(int32 quantity)
{
	return (this->flag.guid || !this->isStackable()) ? quantity : 1;
}

void itemdb_gen_itemmoveinfo()
{
	ShowInfo("itemdb_gen_itemmoveinfo: Generating itemmoveinfov5.txt.\n");
	auto starttime = std::chrono::system_clock::now();
	auto os = std::ofstream("./generated/clientside/data/itemmoveinfov5.txt", std::ios::trunc);
	std::map<t_itemid, std::shared_ptr<item_data>> sorted_itemdb(item_db.begin(), item_db.end());

	os << "// ItemID,\tDrop,\tVending,\tStorage,\tCart,\tNpc Sale,\tMail,\tAuction,\tGuildStorage\n";
	os << "// This format does not accept blank lines. Be careful.\n";

	item_data tmp_id{};
	for (auto it = sorted_itemdb.begin(); it != sorted_itemdb.end(); ++it) {
		if (it->second->type == IT_UNKNOWN)
			continue;
		if (memcmp(&it->second->flag.trade_restriction, &tmp_id.flag.trade_restriction, sizeof(tmp_id.flag.trade_restriction)) == 0)
			continue;

		os << it->first << "\t" << it->second->flag.trade_restriction.drop << "\t" << it->second->flag.trade_restriction.trade << "\t" << it->second->flag.trade_restriction.storage << "\t" << it->second->flag.trade_restriction.cart << "\t" << it->second->flag.trade_restriction.sell << "\t" << it->second->flag.trade_restriction.mail << "\t" << it->second->flag.trade_restriction.auction << "\t" << it->second->flag.trade_restriction.guild_storage << "\t// " << it->second->name << "\n";
	}

	os.close();

	auto currenttime = std::chrono::system_clock::now();
	ShowInfo("itemdb_gen_itemmoveinfo: Done generating itemmoveinfov5.txt. The process took %lldms\n", std::chrono::duration_cast<std::chrono::milliseconds>(currenttime - starttime).count());
}

/**
* Reload Item DB
*/
void itemdb_reload(void) {
	struct s_mapiterator* iter;
	map_session_data* sd;

	do_final_itemdb();

	// read new data
	itemdb_read();
	cashshop_reloaddb();

	mob_reload_itemmob_data();

	// readjust itemdb pointer cache for each player
	iter = mapit_geteachpc();
	for( sd = (map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (map_session_data*)mapit_next(iter) ) {
		memset(sd->item_delay, 0, sizeof(sd->item_delay));  // reset item delays
		sd->combos.clear(); // clear combo bonuses
		pc_setinventorydata( *sd );
		pc_check_available_item(sd, ITMCHK_ALL); // Check for invalid(ated) items.
		pc_load_combo(sd); // Check to see if new combos are available
		status_calc_pc(sd, SCO_FORCE); // 
	}
	mapit_free(iter);
}

/**
* Finalizing Item DB
*/
void do_final_itemdb(void) {
	item_db.clear();
	itemdb_combo.clear();
	itemdb_group.clear();
	random_option_db.clear();
	random_option_group.clear();
	laphine_synthesis_db.clear();
	laphine_upgrade_db.clear();
	item_reform_db.clear();
	item_enchant_db.clear();
	item_package_db.clear();
	if (battle_config.feature_roulette)
		itemdb_roulette_free();
}

/**
* Initializing Item DB
*/
void do_init_itemdb(void) {
	itemdb_read();
}
