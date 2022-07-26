// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itemdb.hpp"

#include <iostream>
#include <stdlib.h>

#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/utils.hpp"
#include "../common/utilities.hpp"

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

	if (this->nodeExists(node, "Buy")) {
		uint32 buy;

		if (!this->asUInt32(node, "Buy", buy))
			return 0;

		if( buy > MAX_ZENY ){
			this->invalidWarning( node["Buy"], "Buying price exceeds MAX_ZENY. Capping...\n" );
			buy = MAX_ZENY;
		}

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

		item->value_sell = sell;
	} else {
		if (!exists) {
			item->value_sell = 0;
		}
	}

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

		// When a particular price is not given, we should base it off the other one
		if (item->value_buy == 0 && item->value_sell > 0)
			item->value_buy = item->value_sell * 2;
		else if (item->value_buy > 0 && item->value_sell == 0)
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

std::shared_ptr<s_item_group_entry> get_random_itemsubgroup(std::shared_ptr<s_item_group_random> random) {
	if (random == nullptr)
		return nullptr;

	for (size_t j = 0, max = random->data.size() * 3; j < max; j++) {
		std::shared_ptr<s_item_group_entry> entry = util::umap_random(random->data);

		if (entry->rate == 0 || rnd() % random->total_rate < entry->rate)	// always return entry for rate 0 ('must' item)
			return entry;
	}

	return util::umap_random(random->data);
}

/**
* Return a random group entry from Item Group
* @param group_id
* @param sub_group: 0 is 'must' item group, random groups start from 1
* @return Item group entry or NULL on fail
*/
std::shared_ptr<s_item_group_entry> ItemGroupDatabase::get_random_entry(uint16 group_id, uint8 sub_group) {
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

	return get_random_itemsubgroup(group->random[sub_group]);
}

/**
* Return a random Item ID from Item Group
* @param group_id
* @param sub_group: 0 is 'must' item group, random groups start from 1
* @return Item ID or UNKNOWN_ITEM_ID on fail
*/
t_itemid ItemGroupDatabase::get_random_item_id(uint16 group_id, uint8 sub_group) {
	std::shared_ptr<s_item_group_entry> entry = this->get_random_entry(group_id, sub_group);
	return entry != nullptr ? entry->nameid : UNKNOWN_ITEM_ID;
}

/** [Cydh]
* Gives item(s) to the player based on item group
* @param sd: Player that obtains item from item group
* @param group_id: The group ID of item that obtained by player
* @param *group: struct s_item_group from itemgroup_db[group_id].random[idx] or itemgroup_db[group_id].must[sub_group][idx]
*/
static void itemdb_pc_get_itemgroup_sub(map_session_data *sd, bool identify, std::shared_ptr<s_item_group_entry> data) {
	if (data == nullptr)
		return;

	item tmp = {};

	tmp.nameid = data->nameid;
	tmp.bound = data->bound;
	tmp.identify = identify ? identify : itemdb_isidentified(data->nameid);
	tmp.expire_time = (data->duration) ? (unsigned int)(time(NULL) + data->duration*60) : 0;
	if (data->isNamed) {
		tmp.card[0] = itemdb_isequip(data->nameid) ? CARD0_FORGE : CARD0_CREATE;
		tmp.card[1] = 0;
		tmp.card[2] = GetWord(sd->status.char_id, 0);
		tmp.card[3] = GetWord(sd->status.char_id, 1);
	}

	uint16 get_amt = 0;

	if (itemdb_isstackable(data->nameid) && data->isStacked)
		get_amt = data->amount;
	else
		get_amt = 1;

	tmp.amount = get_amt;

	// Do loop for non-stackable item
	for (uint16 i = 0; i < data->amount; i += get_amt) {
		char flag = 0;
		tmp.unique_id = data->GUID ? pc_generate_unique_id(sd) : 0; // Generate GUID

		if( itemdb_isequip( data->nameid ) ){
			if( data->refineMinimum > 0 && data->refineMaximum > 0 ){
				tmp.refine = rnd_value( data->refineMinimum, data->refineMaximum );
			}else if( data->refineMinimum > 0 ){
				tmp.refine = rnd_value( data->refineMinimum, MAX_REFINE );
			}else if( data->refineMaximum > 0 ){
				tmp.refine = rnd_value( 1, data->refineMaximum );
			}else{
				tmp.refine = 0;
			}

			if( data->randomOptionGroup != nullptr ){
				memset( tmp.option, 0, sizeof( tmp.option ) );

				data->randomOptionGroup->apply( tmp );
			}
		}

		if ((flag = pc_additem(sd, &tmp, get_amt, LOG_TYPE_SCRIPT))) {
			clif_additem(sd, 0, 0, flag);
			if (pc_candrop(sd, &tmp))
				map_addflooritem(&tmp, tmp.amount, sd->bl.m, sd->bl.x,sd->bl.y, 0, 0, 0, 0, 0);
		}
		else if (!flag && data->isAnnounced)
			intif_broadcast_obtain_special_item(sd, data->nameid, sd->itemid, ITEMOBTAIN_TYPE_BOXITEM);
	}
}

/** [Cydh]
* Find item(s) that will be obtained by player based on Item Group
* @param group_id: The group ID that will be gained by player
* @param nameid: The item that trigger this item group
* @return val: 0:success, 1:no sd, 2:invalid item group
*/
uint8 ItemGroupDatabase::pc_get_itemgroup(uint16 group_id, bool identify, map_session_data *sd) {
	nullpo_retr(1,sd);

	std::shared_ptr<s_item_group_db> group = this->find(group_id);

	if (group == nullptr) {
		ShowError("pc_get_itemgroup: Invalid group id '%d' specified.\n",group_id);
		return 2;
	}
	if (group->random.empty())
		return 0;
	
	// Get all the 'must' item(s) (subgroup 0)
	uint16 subgroup = 0;
	std::shared_ptr<s_item_group_random> random = util::umap_find(group->random, subgroup);

	if (random != nullptr && !random->data.empty()) {
		for (const auto &it : random->data)
			itemdb_pc_get_itemgroup_sub(sd, identify, it.second);
	}

	// Get 1 'random' item from each subgroup
	for (const auto &random : group->random) {
		if (random.first == 0 || random.second->data.empty())
			continue;
		itemdb_pc_get_itemgroup_sub(sd, identify, get_random_itemsubgroup(random.second));
	}

	return 0;
}

/** Searches for the item_data. Use this to check if item exists or not.
* @param nameid
* @return *item_data if item is exist, or NULL if not
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

	for (int i = 0; i < ARRAYLENGTH(temp_mask); i++) {
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
bool itemdb_isdropable_sub(struct item_data *item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction.drop) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_cantrade_sub(struct item_data* item, int gmlv, int gmlv2) {
	return (item && (!(item->flag.trade_restriction.trade) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

bool itemdb_canpartnertrade_sub(struct item_data* item, int gmlv, int gmlv2) {
	return (item && (item->flag.trade_restriction.trade_partner || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

bool itemdb_cansell_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction.sell) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_cancartstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction.cart) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction.storage) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canguildstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction.guild_storage) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canmail_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction.mail) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_canauction_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction.auction) || gmlv >= item->gm_lv_trade_override));
}

bool itemdb_isrestricted(struct item* item, int gmlv, int gmlv2, bool (*func)(struct item_data*, int, int))
{
	struct item_data* item_data = itemdb_search(item->nameid);
	int i;

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
	int type=itemdb_type(nameid);
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

			if (this->nodeExists(subit, "SubGroup")) {
				if (!this->asUInt16(subit, "SubGroup", subgroup))
					continue;
			} else {
				subgroup = 1;
			}

			std::shared_ptr<s_item_group_random> random = util::umap_find(group->random, subgroup);

			if (random == nullptr) {
				random = std::make_shared<s_item_group_random>();
				group->random[subgroup] = random;
			}

			const auto& listNode = subit["List"];

			for (const auto& listit : listNode) {
				if (this->nodeExists(listit, "Clear")) {
					std::string item_name;

					if (!this->asString(listit, "Clear", item_name))
						continue;

					std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

					if (item == nullptr) {
						this->invalidWarning(listit["Clear"], "Unknown Item %s. Clear failed.\n", item_name.c_str());
						continue;
					}

					if (random->data.erase(item->nameid) == 0)
						this->invalidWarning(listit["Clear"], "Item %hu doesn't exist in the SubGroup %hu (group %s). Clear failed.\n", item->nameid, subgroup, group_name.c_str());

					continue;
				}

				std::string item_name;

				if (!this->asString(listit, "Item", item_name))
					continue;

				std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

				if (item == nullptr) {
					this->invalidWarning(listit["Item"], "Unknown Item %s.\n", item_name.c_str());
					continue;
				}

				std::shared_ptr<s_item_group_entry> entry = util::umap_find(random->data, item->nameid);
				bool entry_exists = entry != nullptr;

				if (!entry_exists) {
					entry = std::make_shared<s_item_group_entry>();
					random->data[item->nameid] = entry;
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

				if (subgroup == 0 && entry->rate > 0) {
					this->invalidWarning(listit["Item"], "SubGroup 0 is reserved for item without Rate ('must' item). Defaulting Rate to 0.\n");
					entry->rate = 0;
				}
				if (subgroup != 0 && entry->rate == 0) {
					this->invalidWarning(listit["Item"], "Entry must have a Rate for group above 0, skipping.\n");
					continue;
				}

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
	for (const auto &group : *this) {
		for (const auto &random : group.second->random) {
			random.second->total_rate = 0;
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
static bool itemdb_read_noequip(char* str[], int columns, int current) {
	t_itemid nameid;
	int flag;

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
	int i, j;
	uint32 count = 0;

	// retrieve all rows from the item database
	if (SQL_ERROR == Sql_Query(mmysql_handle, "SELECT * FROM `%s`", roulette_table)) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++)
		rd.items[i] = 0;

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		int k, limit = MAX_ROULETTE_COLUMNS - i;

		for (k = 0; k < limit && SQL_SUCCESS == Sql_NextRow(mmysql_handle); k++) {
			char* data;
			t_itemid item_id;
			unsigned short amount;
			int level, flag;

			Sql_GetData(mmysql_handle, 1, &data, NULL); level = atoi(data);
			Sql_GetData(mmysql_handle, 2, &data, NULL); item_id = strtoul(data, nullptr, 10);
			Sql_GetData(mmysql_handle, 3, &data, NULL); amount = atoi(data);
			Sql_GetData(mmysql_handle, 4, &data, NULL); flag = atoi(data);

			if (!itemdb_exists(item_id)) {
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
			RECREATE(rd.qty[i], unsigned short, rd.items[i]);
			RECREATE(rd.flag[i], int, rd.items[i]);

			rd.nameid[i][j] = item_id;
			rd.qty[i][j] = amount;
			rd.flag[i][j] = flag;

			++count;
		}
	}

	// free the query result
	Sql_FreeResult(mmysql_handle);

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		int limit = MAX_ROULETTE_COLUMNS - i;

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
		RECREATE(rd.qty[i], unsigned short, rd.items[i]);
		RECREATE(rd.flag[i], int, rd.items[i]);

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
	int i;

	for (i = 0; i < MAX_ROULETTE_LEVEL; i++) {
		if (rd.nameid[i])
			aFree(rd.nameid[i]);
		if (rd.qty[i])
			aFree(rd.qty[i]);
		if (rd.flag[i])
			aFree(rd.flag[i]);
		rd.nameid[i] = NULL;
		rd.qty[i] = NULL;
		rd.flag[i] = NULL;
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
static int itemdb_read_sqldb(void) {
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
			",`magic_attack`,`class_third`,`class_third_upper`,`class_third_baby`,`class_fourth`,`job_kagerouoboro`,`job_rebellion`,`job_summoner`,`job_spirit_handler`"
#endif
			" FROM `%s`", item_db_name[fi]) ) {
			Sql_ShowDebug(mmysql_handle);
			continue;
		}

		uint32 total_columns = Sql_NumColumns(mmysql_handle);
		uint64 total_rows = Sql_NumRows(mmysql_handle), rows = 0, count = 0;

		// process rows one by one
		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
			ShowStatus( "Loading [%" PRIu64 "/%" PRIu64 "] rows from '" CL_WHITE "%s" CL_RESET "'" CL_CLL "\r", ++rows, total_rows, item_db_name[fi] );

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
		(id->flag.no_equip&2 && mapdata->flag[MF_PVP]) || // PVP
		(id->flag.no_equip&4 && mapdata_flag_gvg2_no_te(mapdata)) || // GVG
		(id->flag.no_equip&8 && mapdata->flag[MF_BATTLEGROUND]) || // Battleground
		(id->flag.no_equip&16 && mapdata_flag_gvg2_te(mapdata)) || // WOE:TE
		(id->flag.no_equip&(mapdata->zone) && mapdata->flag[MF_RESTRICTED]) // Zone restriction
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
			aFree(randopt->script);
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

			if( rnd() % 10000 < option->chance ){
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

			if( rnd() % 10000 < option->chance ){
				apply_sub( item.option[i], option );
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
	int i;
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

int item_data::inventorySlotNeeded(int quantity)
{
	return (this->flag.guid || !this->isStackable()) ? quantity : 1;
}

/**
* Reload Item DB
*/
void itemdb_reload(void) {
	struct s_mapiterator* iter;
	struct map_session_data* sd;

	do_final_itemdb();

	// read new data
	itemdb_read();
	cashshop_reloaddb();

	mob_reload_itemmob_data();

	// readjust itemdb pointer cache for each player
	iter = mapit_geteachpc();
	for( sd = (struct map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (struct map_session_data*)mapit_next(iter) ) {
		memset(sd->item_delay, 0, sizeof(sd->item_delay));  // reset item delays
		sd->combos.clear(); // clear combo bonuses
		pc_setinventorydata(sd);
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
	if (battle_config.feature_roulette)
		itemdb_roulette_free();
}

/**
* Initializing Item DB
*/
void do_init_itemdb(void) {
	itemdb_read();
}
