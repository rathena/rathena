// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itemdb.hpp"

#include <stdlib.h>

#include "../common/malloc.hpp"
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

static DBMap *itemdb_combo; /// Item Combo DB
static DBMap *itemdb_group; /// Item Group DB
static DBMap *itemdb_randomopt; /// Random option DB
static DBMap *itemdb_randomopt_group; /// Random option group DB

struct item_data *dummy_item; /// This is the default dummy item used for non-existant items. [Skotlex]

struct s_roulette_db rd;

static void itemdb_jobid2mapid(uint64 *bclass, uint64 jobmask, bool active);
static char itemdb_gendercheck(struct item_data *id);

const std::string ItemDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_db.yml";
}

/**
 * Reads and parses an entry from the item_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 ItemDatabase::parseBodyNode(const YAML::Node &node) {
	uint32 nameid;

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

		if (itemdb_search_aegisname(name.c_str()))
			this->invalidWarning(node["AegisName"], "Found duplicate item Aegis name for %s.\n", name.c_str());

		item->name.resize(ITEM_NAME_LENGTH);
		item->name = name.c_str();
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		item->jname.resize(ITEM_NAME_LENGTH);
		item->jname = name.c_str();
	}

	if (this->nodeExists(node, "Type")) {
		std::string type;

		if (!this->asString(node, "Type", type))
			return 0;

		std::string type_constant = "IT_" + type;
		int64 constant;

		if (!script_get_constant(type_constant.c_str(), &constant) || constant < IT_HEALING || constant == IT_UNKNOWN || constant == IT_UNKNOWN2 || (constant > IT_SHADOWGEAR && constant < IT_CASH) || constant >= IT_MAX) {
			this->invalidWarning(node["Type"], "Invalid item type %s for %s (%hu), defaulting to IT_ETC.\n", type.c_str(), item->name.c_str(), nameid);
			constant = IT_ETC;
		}

		if (constant == IT_DELAYCONSUME) { // Items that are consumed only after target confirmation
			item->type = IT_USABLE;
			item->flag.delay_consume |= 0x1;
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

		std::string type_constant = "W_" + type;
		int64 constant;

		if (item->type == IT_WEAPON && script_get_constant(type_constant.c_str(), &constant)) {
			if (constant < W_FIST || constant > MAX_WEAPON_TYPE) {
				this->invalidWarning(node["SubType"], "Invalid weapon type %s for %s (%hu), defaulting to IT_ETC.\n", type.c_str(), item->name.c_str(), nameid);
				item->type = IT_ETC;
			}
		} else {
			type_constant = "AMMO_" + type;

			if (!script_get_constant(type_constant.c_str(), &constant)) {
				if (item->type == IT_AMMO && (constant < AMMO_ARROW || constant > AMMO_THROWWEAPON)) {
					this->invalidWarning(node["SubType"], "Invalid ammo type %s for %s (%hu), defaulting to IT_ETC.\n", type.c_str(), item->name.c_str(), nameid);
					item->type = IT_ETC;
				} else
					item->subtype = constant;
			}
		}
	} else {
		if (!exists)
			item->subtype = 0;
	}

	// When a particular price is not given, we should base it off the other one
	// (it is important to make a distinction between 'no price' and 0z)
	if (this->nodeExists(node, "Buy")) {
		int32 buy;

		if (!this->asInt32(node, "Buy", buy))
			return 0;

		item->value_buy = buy;
	} else {
		if (!exists) {
			if (this->nodeExists(node, "Sell")) {
				int32 sell;

				if (!this->asInt32(node, "Sell", sell))
					return 0;

				item->value_buy = sell * 2;
			} else
				item->value_buy = 0;
		}
	}

	if (this->nodeExists(node, "Sell")) {
		int32 sell;

		if (!this->asInt32(node, "Sell", sell))
			return 0;

		item->value_sell = sell;
	} else {
		if (!exists)
			item->value_sell = item->value_buy / 2;
	}

	if (item->value_buy / 124. < item->value_sell / 75.)
		this->invalidWarning(node["Sell"], "Buying/Selling [%d/%d] price of %s (%hu) allows Zeny making exploit through buying/selling at discounted/overcharged prices!\n", item->value_buy, item->value_sell, item->name.c_str(), nameid);

	if (this->nodeExists(node, "Weight")) {
		int32 weight;

		if (!this->asInt32(node, "Weight", weight))
			return 0;

		item->weight = weight;
	} else {
		if (!exists)
			item->weight = 0;
	}

	if (this->nodeExists(node, "Attack")) {
		int32 atk;

		if (!this->asInt32(node, "Attack", atk))
			return 0;

		item->atk = atk;
	} else {
		if (!exists)
			item->atk = 0;
	}

#ifdef RENEWAL
	if (this->nodeExists(node, "MagicAttack")) {
		int32 matk;

		if (!this->asInt32(node, "MagicAttack", matk))
			return 0;

		item->matk = matk;
	} else {
		if (!exists)
			item->matk = 0;
	}
#endif

	if (this->nodeExists(node, "Defense")) {
		int32 def;

		if (!this->asInt32(node, "Defense", def))
			return 0;

		item->def = def;
	} else {
		if (!exists)
			item->def = 0;
	}

	if (this->nodeExists(node, "Range")) {
		int32 range;

		if (!this->asInt32(node, "Range", range))
			return 0;

		item->range = range;
	} else {
		if (!exists)
			item->range = 0;
	}

	if (this->nodeExists(node, "Slots")) {
		uint16 slot;

		if (!this->asUInt16(node, "Slots", slot))
			return 0;

		if (slot > MAX_SLOTS) {
			this->invalidWarning(node["Slots"], "Item %s (%hu) exceeds maximum slot count, capping to MAX_SLOTS.\n", item->name.c_str(), nameid);
			slot = MAX_SLOTS;
		}

		item->slot = slot;
	} else {
		if (!exists)
			item->slot = 0;
	}

	if (this->nodeExists(node, "Job")) {
		const YAML::Node &jobNode = node["Job"];

		item->class_base[0] = item->class_base[1] = item->class_base[2] = 0;

		if (this->nodeExists(jobNode, "All")) {
			bool active;

			if (!this->asBool(jobNode, "All", active))
				return 0;

			itemdb_jobid2mapid(item->class_base, UINT64_MAX, active);
		}

		for (const auto &jobit : jobNode) {
			std::string jobName = jobit.first.as<std::string>();

			if (jobName.compare("All") == 0)
				continue;

			std::string jobName_constant = "JOB_" + jobName;
			int64 constant;

			if (!script_get_constant(jobName_constant.c_str(), &constant)) {
				this->invalidWarning(jobNode[jobName], "Invalid item job %s for %s (%hu), defaulting to All.\n", jobName.c_str(), item->name.c_str(), nameid);
				itemdb_jobid2mapid(item->class_base, UINT64_MAX, true);
				break;
			}

			bool active;

			if (!this->asBool(jobNode, jobName.c_str(), active))
				return 0;

			itemdb_jobid2mapid(item->class_base, static_cast<uint64>(1) << constant, active);
		}
	} else {
		if (!exists) {
			item->class_base[0] = item->class_base[1] = item->class_base[2] = 0;

			itemdb_jobid2mapid(item->class_base, UINT64_MAX, true);
		}
	}

	if (this->nodeExists(node, "Class")) {
		const YAML::Node &classNode = node["Class"];

		if (this->nodeExists(classNode, "All")) {
			bool active;

			if (!this->asBool(classNode, "All", active))
				return 0;

			if (active)
				item->class_upper |= ITEMJ_MAX;
			else
				item->class_upper &= ITEMJ_MAX;
		}

		for (const auto &classit : classNode) {
			std::string className = classit.first.as<std::string>();

			if (className.compare("All") == 0)
				continue;

			std::string className_constant = "ITEMJ_" + className;
			int64 constant;

			if (!script_get_constant(className_constant.c_str(), &constant)) {
				this->invalidWarning(classNode[className], "Invalid class upper %s for %s (%hu), defaulting to All.\n", className.c_str(), item->name.c_str(), nameid);
				item->class_upper = ITEMJ_MAX;
				break;
			}

			bool active;

			if (!this->asBool(classNode, className.c_str(), active))
				return 0;

			if (active)
				item->class_upper |= constant;
			else
				item->class_upper &= constant;
		}
	} else {
		if (!exists)
			item->class_upper = ITEMJ_MAX;
	}

	if (this->nodeExists(node, "Gender")) {
		std::string gender;

		if (!this->asString(node, "Gender", gender))
			return 0;

		std::string gender_constant = "SEX_" + gender;
		int64 constant;

		if (!script_get_constant(gender_constant.c_str(), &constant) || constant < SEX_FEMALE || constant > SEX_BOTH) {
			this->invalidWarning(node["Gender"], "Invalid item gender %s for %s (%hu), defaulting to SEX_BOTH.\n", gender.c_str(), item->name.c_str(), nameid);
			constant = SEX_BOTH;
		}

		item->sex = constant;
		item->sex = itemdb_gendercheck(item.get());
	} else {
		if (!exists) {
			item->sex = SEX_BOTH;
			item->sex = itemdb_gendercheck(item.get());
		}
	}

	if (this->nodeExists(node, "Location")) {
		const YAML::Node &locationNode = node["Location"];

		for (const auto &locit : locationNode) {
			std::string equipName = locit.first.as<std::string>(), equipName_constant = "EQP_" + equipName;
			int64 constant;

			if (!script_get_constant(equipName_constant.c_str(), &constant)) {
				this->invalidWarning(locationNode[equipName], "Invalid location %s for %s (%hu), defaulting to IT_ETC.\n", equipName.c_str(), item->name.c_str(), nameid);
				item->type = IT_ETC;
				break;
			}

			bool active;

			if (!this->asBool(locationNode, equipName.c_str(), active))
				return 0;

			if (active) {
				if (constant & EQP_SHADOW_GEAR && item->type != IT_SHADOWGEAR) {
					this->invalidWarning(node, "Invalid item equip location %s for %s (%hu) as it's not a Shadow Gear item type, defaulting to IT_ETC.\n", equipName.c_str(), item->name.c_str(), nameid);
					item->type = IT_ETC;
				}

				item->equip |= constant;
			} else
				item->equip &= constant;
		}
	} else {
		if (!exists) {
			if (item->equip == 0 && itemdb_isequip2(item.get())) {
				this->invalidWarning(node["Location"], "Invalid item equip location for %s (%hu) as it has no equip location, defaulting to IT_ETC.\n", item->name.c_str(), nameid);
				item->type = IT_ETC;
			}
		}
	}

	if (this->nodeExists(node, "WeaponLevel")) {
		uint16 lv;

		if (!this->asUInt16(node, "WeaponLevel", lv))
			return 0;

		if (lv > REFINE_TYPE_MAX) {
			this->invalidWarning(node["WeaponLevel"], "Invalid weapon level %d for %s (%hu), defaulting to REFINE_TYPE_MAX.\n", lv, item->name.c_str(), nameid);
			lv = REFINE_TYPE_MAX;
		}

		item->wlv = lv;
	} else {
		if (!exists)
			item->wlv = 0;
	}

	if (this->nodeExists(node, "EquipLevelMin")) {
		uint16 lv;

		if (!this->asUInt16(node, "EquipLevelMin", lv))
			return 0;

		if (lv > MAX_LEVEL) {
			this->invalidWarning(node["EquipLevelMin"], "Invalid minimum equip level for %s (%hu), defaulting to MAX_LEVEL.\n", item->name.c_str(), nameid);
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
			this->invalidWarning(node["EquipLevelMax"], "Max equip level is less than equip level for %s (%hu), defaulting to equip level.\n", item->name.c_str(), nameid);
			lv = item->elv;
		}

		if (lv > MAX_LEVEL) {
			this->invalidWarning(node["EquipLevelMax"], "Invalid maximum equip level for %s (%hu), defaulting to MAX_LEVEL.\n", item->name.c_str(), nameid);
			lv = MAX_LEVEL;
		}

		item->elvmax = lv;
	} else {
		if (!exists)
			item->elvmax = 0;
	}

	if (this->nodeExists(node, "Refineable")) {
		bool refine;

		if (!this->asBool(node, "Refineable", refine))
			return 0;

		item->flag.no_refine = refine;
	} else {
		if (!exists)
			item->flag.no_refine = false;
	}

	if (this->nodeExists(node, "View")) {
		int32 look;

		if (!this->asInt32(node, "View", look))
			return 0;

		item->look = look;
	} else {
		if (!exists)
			item->look = 0;
	}

	if (this->nodeExists(node, "Flags")) {
		const YAML::Node &flagNode = node["Flags"];

		if (this->nodeExists(flagNode, "Buyingstore")) {
			bool active;

			if (!this->asBool(flagNode, "Buyingstore", active))
				return 0;

			if (!itemdb_isstackable2(item.get()) && active) {
				this->invalidWarning(flagNode["Buyingstore"], "Non-stackable item %s (%hu) cannot be enabled for buying store.\n", item->name.c_str(), nameid);
				active = false;
			}

			item->flag.buyingstore = active;
		}

		if (this->nodeExists(flagNode, "DeadBranch")) {
			bool active;

			if (!this->asBool(flagNode, "DeadBranch", active))
				return 0;

			item->flag.dead_branch = active;
		}

		if (this->nodeExists(flagNode, "Container")) {
			bool active;

			if (!this->asBool(flagNode, "Container", active))
				return 0;

			item->flag.group = active;
		}

		if (this->nodeExists(flagNode, "Guid")) {
			bool active;

			if (!this->asBool(flagNode, "Guid", active))
				return 0;

			if (!itemdb_isstackable2(item.get()) && active) {
				this->invalidWarning(flagNode["Guid"], "Non-stackable item %s (%hu) cannot be enabled for GUID.\n", item->name.c_str(), nameid);
				active = false;
			}

			item->flag.guid = active;
		}

		if (this->nodeExists(flagNode, "BindOnEquip")) {
			bool active;

			if (!this->asBool(flagNode, "BindOnEquip", active))
				return 0;

			item->flag.bindOnEquip = active;
		}

		if (this->nodeExists(flagNode, "DropAnnounce")) {
			bool active;

			if (!this->asBool(flagNode, "DropAnnounce", active))
				return 0;

			item->flag.broadcast = active;
		}

		if (this->nodeExists(flagNode, "NoConsume")) {
			bool active;

			if (!this->asBool(flagNode, "NoConsume", active))
				return 0;

			if (active)
				item->flag.delay_consume |= 0x2;
			else
				item->flag.delay_consume &= ~0x2;
		}

		if (this->nodeExists(flagNode, "DropEffect")) {
			std::string effect;

			if (!this->asString(flagNode, "DropEffect", effect))
				return 0;

			std::string effect_constant = "DROPEFFECT_" + effect;
			int64 constant;

			if (!script_get_constant(effect_constant.c_str(), &constant) || constant < DROPEFFECT_NONE || constant > DROPEFFECT_MAX) {
				this->invalidWarning(flagNode["DropEffect"], "Invalid item drop effect %s for %s (%hu), defaulting to DROPEFFECT_NONE.\n", effect.c_str(), item->name.c_str(), nameid);
				constant = DROPEFFECT_NONE;
			}

			item->flag.dropEffect = static_cast<e_item_drop_effect>(constant);
		}
	} else {
		if (!exists) {
			item->flag.buyingstore = false;
			item->flag.dead_branch = false;
			item->flag.group = false;
			item->flag.guid = false;
			item->flag.bindOnEquip = false;
			item->flag.broadcast = false;
			if (!(item->flag.delay_consume & 0x1))
				item->flag.delay_consume = 0;
			item->flag.dropEffect = DROPEFFECT_NONE;
		}
	}

	if (this->nodeExists(node, "Delay")) {
		const YAML::Node &delayNode = node["Delay"];

		if (this->nodeExists(delayNode, "Duration")) {
			uint32 duration;

			if (!this->asUInt32(delayNode, "Duration", duration))
				return 0;

			item->delay.duration = duration;
		}

		if (this->nodeExists(delayNode, "Status")) {
			std::string status;

			if (!this->asString(delayNode, "Status", status))
				return 0;

			std::string status_constant = "SC_" + status;
			int64 constant;

			if (!script_get_constant(status_constant.c_str(), &constant)) {
				this->invalidWarning(delayNode[status], "Invalid item delay status %s for %s (%hu), defaulting to SC_NONE.\n", status.c_str(), item->name.c_str(), nameid);
				constant = SC_NONE;
			}

			item->delay.sc = static_cast<sc_type>(constant);
		}
	} else {
		if (!exists) {
			item->delay.duration = 0;
			item->delay.sc = SC_NONE;
		}
	}

	if (this->nodeExists(node, "Stack")) {
		const YAML::Node &stackNode = node["Stack"];

		if (this->nodeExists(stackNode, "Amount")) {
			uint16 amount;

			if (!this->asUInt16(stackNode, "Amount", amount))
				return 0;

			if (!itemdb_isstackable2(item.get())) {
				this->invalidWarning(stackNode["Amount"], "Non-stackable item %s (%hu) cannot be enabled for stacking.\n", item->name.c_str(), nameid);
				amount = 0;
			}

			item->stack.amount = amount;
		}

		if (this->nodeExists(stackNode, "Inventory")) {
			bool active;

			if (!this->asBool(stackNode, "Inventory", active))
				return 0;

			item->stack.inventory = active;
		} else {
			if (!exists)
				item->stack.inventory = true;
		}

		if (this->nodeExists(stackNode, "Cart")) {
			bool active;

			if (!this->asBool(stackNode, "Cart", active))
				return 0;

			item->stack.cart = active;
		}

		if (this->nodeExists(stackNode, "Storage")) {
			bool active;

			if (!this->asBool(stackNode, "Storage", active))
				return 0;

			item->stack.storage = active;
		}

		if (this->nodeExists(stackNode, "GuildStorage")) {
			bool active;

			if (!this->asBool(stackNode, "GuildStorage", active))
				return 0;

			item->stack.guild_storage = active;
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
		const YAML::Node &nouseNode = node["NoUse"];

		if (this->nodeExists(nouseNode, "Override")) {
			uint16 override;

			if (!this->asUInt16(nouseNode, "Override", override))
				return 0;

			if (override > 100) {
				this->invalidWarning(nouseNode["Override"], "Item no use override level exceeds 100 for %s (%hu), capping.\n", item->name.c_str(), nameid);
				override = 100;
			}

			item->item_usage.override = override;
		}

		if (this->nodeExists(nouseNode, "Sitting")) {
			bool active;

			if (!this->asBool(nouseNode, "Sitting", active))
				return 0;

			item->item_usage.sitting = active;
		}
	} else {
		if (!exists) {
			item->item_usage.override = 0;
			item->item_usage.sitting = false;
		}
	}

	if (this->nodeExists(node, "Trade")) {
		const YAML::Node &tradeNode = node["Trade"];

		if (this->nodeExists(tradeNode, "Override")) {
			uint16 override;

			if (!this->asUInt16(tradeNode, "Override", override))
				return 0;

			if (override > 100) {
				this->invalidWarning(tradeNode["Override"], "Item trade override level exceeds 100 for %s (%hu), capping.\n", item->name.c_str(), nameid);
				override = 100;
			}

			item->gm_lv_trade_override = override;
		}

		if (this->nodeExists(tradeNode, "NoDrop")) {
			bool active;

			if (!this->asBool(tradeNode, "NoDrop", active))
				return 0;

			item->flag.trade_restriction.drop = active;
		}

		if (this->nodeExists(tradeNode, "NoTrade")) {
			bool active;

			if (!this->asBool(tradeNode, "NoTrade", active))
				return 0;

			item->flag.trade_restriction.trade = active;
		}

		if (this->nodeExists(tradeNode, "TradePartner")) {
			bool active;

			if (!this->asBool(tradeNode, "TradePartner", active))
				return 0;

			item->flag.trade_restriction.trade_partner = active;
		}

		if (this->nodeExists(tradeNode, "NoSell")) {
			bool active;

			if (!this->asBool(tradeNode, "NoSell", active))
				return 0;

			item->flag.trade_restriction.sell = active;
		}

		if (this->nodeExists(tradeNode, "NoCart")) {
			bool active;

			if (!this->asBool(tradeNode, "NoCart", active))
				return 0;

			item->flag.trade_restriction.cart = active;
		}

		if (this->nodeExists(tradeNode, "NoStorage")) {
			bool active;

			if (!this->asBool(tradeNode, "NoStorage", active))
				return 0;

			item->flag.trade_restriction.storage = active;
		}

		if (this->nodeExists(tradeNode, "NoGuildStorage")) {
			bool active;

			if (!this->asBool(tradeNode, "NoGuildStorage", active))
				return 0;

			item->flag.trade_restriction.guild_storage = active;
		}

		if (this->nodeExists(tradeNode, "NoMail")) {
			bool active;

			if (!this->asBool(tradeNode, "NoMail", active))
				return 0;

			item->flag.trade_restriction.mail = active;
		}

		if (this->nodeExists(tradeNode, "NoAuction")) {
			bool active;

			if (!this->asBool(tradeNode, "NoAuction", active))
				return 0;

			item->flag.trade_restriction.auction = active;
		}
	} else {
		if (!exists) {
			item->gm_lv_trade_override = 0;
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

		if (item->script) {
			script_free_code(item->script);
			item->script = nullptr;
		}

		item->script = parse_script(script.c_str(), this->getCurrentFile().c_str(), node["Script"].Mark().line + 1, SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else {
		if (!exists) 
			item->script = nullptr;
	}

	if (this->nodeExists(node, "EquipScript")) {
		std::string script;

		if (!this->asString(node, "EquipScript", script))
			return 0;

		if (item->equip_script) {
			script_free_code(item->equip_script);
			item->equip_script = nullptr;
		}

		item->equip_script = parse_script(script.c_str(), this->getCurrentFile().c_str(), node["EquipScript"].Mark().line + 1, SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else {
		if (!exists)
			item->equip_script = nullptr;
	}

	if (this->nodeExists(node, "UnEquipScript")) {
		std::string script;

		if (!this->asString(node, "UnEquipScript", script))
			return 0;

		if (item->unequip_script) {
			script_free_code(item->unequip_script);
			item->unequip_script = nullptr;
		}

		item->unequip_script = parse_script(script.c_str(), this->getCurrentFile().c_str(), node["UnEquipScript"].Mark().line + 1, SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else {
		if (!exists)
			item->unequip_script = nullptr;
	}

	if (!exists)
		this->put(nameid, item);

	return 1;
}

ItemDatabase item_db;

/**
* Check if combo exists
* @param combo_id
* @return NULL if not exist, or struct item_combo*
*/
struct item_combo *itemdb_combo_exists(unsigned short combo_id) {
	return (struct item_combo *)uidb_get(itemdb_combo, combo_id);
}

/**
* Check if item group exists
* @param group_id
* @return NULL if not exist, or s_item_group_db *
*/
struct s_item_group_db *itemdb_group_exists(unsigned short group_id) {
	return (struct s_item_group_db *)uidb_get(itemdb_group, group_id);
}

/**
 * Check if an item exists in a group
 * @param group_id: Item Group ID
 * @param nameid: Item to check for in group
 * @return True if item is in group, else false
 */
bool itemdb_group_item_exists(unsigned short group_id, unsigned short nameid)
{
	struct s_item_group_db *group = (struct s_item_group_db *)uidb_get(itemdb_group, group_id);
	unsigned short i, j;

	if (!group)
		return false;

	for (i = 0; i < MAX_ITEMGROUP_RANDGROUP; i++) {
		for (j = 0; j < group->random[i].data_qty; j++)
			if (group->random[i].data[j].nameid == nameid)
				return true;
	}
	return false;
}

/**
 * Check if an item exists from a group in a player's inventory
 * @param group_id: Item Group ID
 * @return Item's index if found or -1 otherwise
 */
int16 itemdb_group_item_exists_pc(struct map_session_data *sd, unsigned short group_id)
{
	struct s_item_group_db *group = (struct s_item_group_db *)uidb_get(itemdb_group, group_id);

	if (!group)
		return -1;

	for (int i = 0; i < MAX_ITEMGROUP_RANDGROUP; i++) {
		for (int j = 0; j < group->random[i].data_qty; j++) {
			int16 item_position = pc_search_inventory(sd, group->random[i].data[j].nameid);

			if (item_position != -1)
				return item_position;
		}
	}

	return -1;
}

/*==========================================
 * Return item data from item name. (lookup)
 * @param str Item Name
 * @param aegis_only
 * @return item data
 *------------------------------------------*/
static struct item_data* itemdb_searchname1(const char *str, bool aegis_only)
{
	std::shared_ptr<item_data> item = nullptr, item2 = nullptr;

	for (const auto &it : item_db) {
		if (strcmpi(it.second->name.c_str(), str) == 0) {
			item = it.second;
			break;
		} else if (!aegis_only && strcmpi(it.second->jname.c_str(), str) == 0) {
			item2 = it.second;
			break;
		}
	}

	return item ? item.get() : item2.get();
}

struct item_data* itemdb_searchname(const char *str)
{
	return itemdb_searchname1(str, false);
}

struct item_data* itemdb_search_aegisname( const char *str ){
	return itemdb_searchname1( str, true );
}

/*==========================================
 * Finds up to N matches. Returns number of matches [Skotlex]
 * @param *data
 * @param size
 * @param str
 * @return Number of matches item
 *------------------------------------------*/
int itemdb_searchname_array(struct item_data** data, int size, const char *str)
{
	int count = 0;

	for (const auto &it : item_db) {
		if (count < MAX_SEARCH && count < size) {
			if (strcmpi(it.second->jname.c_str(), str) == 0)
				data[count++] = it.second.get();
		} else
			break;
	}

	return count;
}

/**
* Return a random group entry from Item Group
* @param group_id
* @param sub_group: 0 is 'must' item group, random groups start from 1 to MAX_ITEMGROUP_RANDGROUP+1
* @return Item group entry or NULL on fail
*/
struct s_item_group_entry *itemdb_get_randgroupitem(uint16 group_id, uint8 sub_group) {
	struct s_item_group_db *group = (struct s_item_group_db *) uidb_get(itemdb_group, group_id);
	struct s_item_group_entry *list = NULL;
	uint16 qty = 0;

	if (!group) {
		ShowError("itemdb_get_randgroupitem: Invalid group id %d\n", group_id);
		return NULL;
	}
	if (sub_group > MAX_ITEMGROUP_RANDGROUP+1) {
		ShowError("itemdb_get_randgroupitem: Invalid sub_group %d\n", sub_group);
		return NULL;
	}
	if (sub_group == 0) {
		list = group->must;
		qty = group->must_qty;
	}
	else {
		list = group->random[sub_group-1].data;
		qty = group->random[sub_group-1].data_qty;
	}
	if (!qty) {
		ShowError("itemdb_get_randgroupitem: No item entries for group id %d and sub group %d\n", group_id, sub_group);
		return NULL;
	}
	return &list[rnd()%qty];
}

/**
* Return a random Item ID from from Item Group
* @param group_id
* @param sub_group: 0 is 'must' item group, random groups start from 1 to MAX_ITEMGROUP_RANDGROUP+1
* @return Item ID or UNKNOWN_ITEM_ID on fail
*/
unsigned short itemdb_searchrandomid(uint16 group_id, uint8 sub_group) {
	struct s_item_group_entry *entry = itemdb_get_randgroupitem(group_id, sub_group);
	return entry ? entry->nameid : UNKNOWN_ITEM_ID;
}

/** [Cydh]
* Gives item(s) to the player based on item group
* @param sd: Player that obtains item from item group
* @param group_id: The group ID of item that obtained by player
* @param *group: struct s_item_group from itemgroup_db[group_id].random[idx] or itemgroup_db[group_id].must[sub_group][idx]
*/
static void itemdb_pc_get_itemgroup_sub(struct map_session_data *sd, bool identify, struct s_item_group_entry *data) {
	uint16 i, get_amt = 0;
	struct item tmp;

	nullpo_retv(data);

	memset(&tmp, 0, sizeof(tmp));

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

	if (!itemdb_isstackable(data->nameid))
		get_amt = 1;
	else
		get_amt = data->amount;

	// Do loop for non-stackable item
	for (i = 0; i < data->amount; i += get_amt) {
		char flag = 0;
		tmp.unique_id = data->GUID ? pc_generate_unique_id(sd) : 0; // Generate GUID
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
char itemdb_pc_get_itemgroup(uint16 group_id, bool identify, struct map_session_data *sd) {
	uint16 i = 0;
	struct s_item_group_db *group;

	nullpo_retr(1,sd);
	
	if (!(group = (struct s_item_group_db *) uidb_get(itemdb_group, group_id))) {
		ShowError("itemdb_pc_get_itemgroup: Invalid group id '%d' specified.\n",group_id);
		return 2;
	}
	
	// Get the 'must' item(s)
	if (group->must_qty) {
		for (i = 0; i < group->must_qty; i++)
			if (&group->must[i])
				itemdb_pc_get_itemgroup_sub(sd, identify, &group->must[i]);
	}

	// Get the 'random' item each random group
	for (i = 0; i < MAX_ITEMGROUP_RANDGROUP; i++) {
		uint16 rand;
		if (!(&group->random[i]) || !group->random[i].data_qty) //Skip empty random group
			continue;
		rand = rnd()%group->random[i].data_qty;
		if (!(&group->random[i].data[rand]) || !group->random[i].data[rand].nameid)
			continue;
		itemdb_pc_get_itemgroup_sub(sd, identify, &group->random[i].data[rand]);
	}

	return 0;
}

/** Searches for the item_data. Use this to check if item exists or not.
* @param nameid
* @return *item_data if item is exist, or NULL if not
*/
struct item_data* itemdb_exists(unsigned short nameid) {
	return item_db.find(nameid).get();
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
static void itemdb_jobid2mapid(uint64 *bclass, uint64 jobmask, bool active)
{
	uint64 temp_mask[3] = { 0 };

	//Base classes
	if (jobmask & 1ULL<<JOB_NOVICE) {
		//Both Novice/Super-Novice are counted with the same ID
		temp_mask[0] |= 1ULL<<MAPID_NOVICE;
		temp_mask[1] |= 1ULL<<MAPID_NOVICE;
	}
	for (int i = JOB_NOVICE + 1; i <= JOB_THIEF; i++) {
		if (jobmask & 1ULL <<i)
			temp_mask[0] |= 1ULL<<(MAPID_NOVICE + i);
	}
	//2-1 classes
	if (jobmask & 1ULL<<JOB_KNIGHT)
		temp_mask[1] |= 1ULL<<MAPID_SWORDMAN;
	if (jobmask & 1ULL<<JOB_PRIEST)
		temp_mask[1] |= 1ULL<<MAPID_ACOLYTE;
	if (jobmask & 1ULL<<JOB_WIZARD)
		temp_mask[1] |= 1ULL<<MAPID_MAGE;
	if (jobmask & 1ULL<<JOB_BLACKSMITH)
		temp_mask[1] |= 1ULL<<MAPID_MERCHANT;
	if (jobmask & 1ULL<<JOB_HUNTER)
		temp_mask[1] |= 1ULL<<MAPID_ARCHER;
	if (jobmask & 1ULL<<JOB_ASSASSIN)
		temp_mask[1] |= 1ULL<<MAPID_THIEF;
	//2-2 classes
	if (jobmask & 1ULL<<JOB_CRUSADER)
		temp_mask[2] |= 1ULL<<MAPID_SWORDMAN;
	if (jobmask & 1ULL<<JOB_MONK)
		temp_mask[2] |= 1ULL<<MAPID_ACOLYTE;
	if (jobmask & 1ULL<<JOB_SAGE)
		temp_mask[2] |= 1ULL<<MAPID_MAGE;
	if (jobmask & 1ULL<<JOB_ALCHEMIST)
		temp_mask[2] |= 1ULL<<MAPID_MERCHANT;
	if (jobmask & 1ULL<<JOB_BARD)
		temp_mask[2] |= 1ULL<<MAPID_ARCHER;
//	Bard/Dancer share the same slot now.
//	if (jobmask & 1ULL<<JOB_DANCER)
//		temp_mask[2] |= 1ULL<<MAPID_ARCHER;
	if (jobmask & 1ULL<<JOB_ROGUE)
		temp_mask[2] |= 1ULL<<MAPID_THIEF;
	//Special classes that don't fit above.
	if (jobmask & 1ULL<<21) //Taekwon
		temp_mask[0] |= 1ULL<<MAPID_TAEKWON;
	if (jobmask & 1ULL<<22) //Star Gladiator
		temp_mask[1] |= 1ULL<<MAPID_TAEKWON;
	if (jobmask & 1ULL<<23) //Soul Linker
		temp_mask[2] |= 1ULL<<MAPID_TAEKWON;
	if (jobmask & 1ULL<<JOB_GUNSLINGER) { // Rebellion job can equip Gunslinger equips.
		temp_mask[0] |= 1ULL<<MAPID_GUNSLINGER;
		temp_mask[1] |= 1ULL<<MAPID_GUNSLINGER;
	}
	if (jobmask & 1ULL<<JOB_NINJA) { //Kagerou/Oboro jobs can equip Ninja equips. [Rytech]
		temp_mask[0] |= 1ULL<<MAPID_NINJA;
		temp_mask[1] |= 1ULL<<MAPID_NINJA;
	}
	if (jobmask & 1ULL<<26) //Bongun/Munak
		temp_mask[0] |= 1ULL<<MAPID_GANGSI;
	if (jobmask & 1ULL<<27) //Death Knight
		temp_mask[1] |= 1ULL<<MAPID_GANGSI;
	if (jobmask & 1ULL<<28) //Dark Collector
		temp_mask[2] |= 1ULL<<MAPID_GANGSI;
	if (jobmask & 1ULL<<29) //Kagerou / Oboro
		temp_mask[1] |= 1ULL<<MAPID_NINJA;
	if (jobmask & 1ULL<<30) //Rebellion
		temp_mask[1] |= 1ULL<<MAPID_GUNSLINGER;
	if (jobmask & 1ULL<<31) //Summoner
		temp_mask[0] |= 1ULL<<MAPID_SUMMONER;

	for (int i = 0; i < ARRAYLENGTH(temp_mask); i++) {
		if (temp_mask[i] == 0)
			continue;

		if (active)
			bclass[i] |= temp_mask[i];
		else
			bclass[i] &= ~temp_mask[i];
	}
}

/**
* Create dummy item_data as dummy_item and dummy item group entry as dummy_itemgroup
*/
static void itemdb_create_dummy(void) {
	CREATE(dummy_item, struct item_data, 1);

	memset(dummy_item, 0, sizeof(struct item_data));
	dummy_item->nameid = ITEMID_DUMMY;
	dummy_item->weight = 1;
	dummy_item->value_sell = 1;
	dummy_item->type = IT_ETC; //Etc item
	dummy_item->name = "UNKNOWN_ITEM";
	dummy_item->jname = "Unknown Item";
	dummy_item->view_id = UNKNOWN_ITEM_ID;
}

/*==========================================
 * Loads an item from the db. If not found, it will return the dummy item.
 * @param nameid
 * @return *item_data or *dummy_item if item not found
 *------------------------------------------*/
struct item_data* itemdb_search(unsigned short nameid) {
	struct item_data *id = nullptr;

	if (nameid == dummy_item->nameid)
		id = dummy_item;
	else if (!(id = item_db.find(nameid).get())) {
		ShowWarning("itemdb_search: Item ID %hu does not exists in the item_db. Using dummy data.\n", nameid);
		id = dummy_item;
	}
	return id;
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

	if(item_data->slot == 0 || itemdb_isspecial(item->card[0]))
		return true;

	for(i = 0; i < item_data->slot; i++) {
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
char itemdb_isidentified(unsigned short nameid) {
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

/** Search by name for the override flags available items (Give item another sprite)
* Structure: <nameid>,<sprite>
*/
static bool itemdb_read_itemavail(char* str[], int columns, int current) {
	unsigned short nameid, sprite;
	struct item_data *id;

	nameid = atoi(str[0]);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		ShowWarning("itemdb_read_itemavail: Invalid item id %hu.\n", nameid);
		return false;
	}

	sprite = atoi(str[1]);

	if( sprite > 0 )
	{
		id->flag.available = 1;
		id->view_id = sprite;
	}
	else
	{
		id->flag.available = 0;
	}

	return true;
}

static int itemdb_group_free(DBKey key, DBData *data, va_list ap);
static int itemdb_group_free2(DBKey key, DBData *data);

static bool itemdb_read_group(char* str[], int columns, int current) {
	int group_id = -1;
	unsigned int j, prob = 1;
	uint8 rand_group = 1;
	struct s_item_group_random *random = NULL;
	struct s_item_group_db *group = NULL;
	struct s_item_group_entry entry;

	memset(&entry, 0, sizeof(entry));
	entry.amount = 1;
	entry.bound = BOUND_NONE;
	
	str[0] = trim(str[0]);
	if( ISDIGIT(str[0][0]) ){
		group_id = atoi(str[0]);
	}else{
		int64 group_tmp;

		// Try to parse group id as constant
		if (!script_get_constant(str[0], &group_tmp)) {
			ShowError("itemdb_read_group: Unknown group constant \"%s\".\n", str[0]);
			return false;
		}
		group_id = static_cast<int>(group_tmp);
	}

	// Check the group id
	if( group_id < 0 ){
		ShowWarning( "itemdb_read_group: Invalid group ID '%s'\n", str[0] );
		return false;
	}

	// Remove from DB
	if( strcmpi( str[1], "clear" ) == 0 ){
		DBData data;

		if( itemdb_group->remove( itemdb_group, db_ui2key(group_id), &data ) ){
			itemdb_group_free2(db_ui2key(group_id), &data);
			ShowNotice( "itemdb_read_group: Item Group '%s' has been cleared.\n", str[0] );
			return true;
		}else{
			ShowWarning( "itemdb_read_group: Item Group '%s' has not been cleared, because it did not exist.\n", str[0] );
			return false;
		}
	}

	if( columns < 3 ){
		ShowError("itemdb_read_group: Insufficient columns (found %d, need at least 3).\n", columns);
		return false;
	}

	// Checking sub group
	prob = atoi(str[2]);

	if( columns > 4 ){
		rand_group = atoi(str[4]);

		if( rand_group < 0 || rand_group > MAX_ITEMGROUP_RANDGROUP ){
			ShowWarning( "itemdb_read_group: Invalid sub group '%d' for group '%s'\n", rand_group, str[0] );
			return false;
		}
	}else{
		rand_group = 1;
	}

	if( rand_group != 0 && prob < 1 ){
		ShowWarning( "itemdb_read_group: Random item must have a probability. Group '%s'\n", str[0] );
		return false;
	}

	// Check item
	str[1] = trim(str[1]);

	// Check if the item can be found by id
	if( ( entry.nameid = atoi(str[1]) ) <= 0 || !itemdb_exists( entry.nameid ) ){
		// Otherwise look it up by name
		struct item_data *id = itemdb_searchname(str[1]);

		if( id ){
			// Found the item with a name lookup
			entry.nameid = id->nameid;
		}else{
			ShowWarning( "itemdb_read_group: Non-existant item '%s'\n", str[1] );
			return false;
		}
	}

	if( columns > 3 ) entry.amount = cap_value(atoi(str[3]),1,MAX_AMOUNT);
	if( columns > 5 ) entry.isAnnounced= atoi(str[5]) > 0;
	if( columns > 6 ) entry.duration = cap_value(atoi(str[6]),0,UINT16_MAX);
	if( columns > 7 ) entry.GUID = atoi(str[7]) > 0;
	if( columns > 8 ) entry.bound = cap_value(atoi(str[8]),BOUND_NONE,BOUND_MAX-1);
	if( columns > 9 ) entry.isNamed = atoi(str[9]) > 0;
	
	if (!(group = (struct s_item_group_db *) uidb_get(itemdb_group, group_id))) {
		CREATE(group, struct s_item_group_db, 1);
		group->id = group_id;
		uidb_put(itemdb_group, group->id, group);
	}

	// Must item (rand_group == 0), place it here
	if (!rand_group) {
		RECREATE(group->must, struct s_item_group_entry, group->must_qty+1);
		group->must[group->must_qty++] = entry;
		
		// If 'must' item isn't set as random item, skip the next process
		if (!prob) {
			return true;
		}
		rand_group = 0;
	}
	else
		rand_group -= 1;

	random = &group->random[rand_group];
	
	RECREATE(random->data, struct s_item_group_entry, random->data_qty+prob);

	// Put the entry to its rand_group
	for (j = random->data_qty; j < random->data_qty+prob; j++)
		random->data[j] = entry;
	
	random->data_qty += prob;
	return true;
}

/** Read item forbidden by mapflag (can't equip item)
* Structure: <nameid>,<mode>
*/
static bool itemdb_read_noequip(char* str[], int columns, int current) {
	unsigned short nameid;
	int flag;
	struct item_data *id;

	nameid = atoi(str[0]);
	flag = atoi(str[1]);

	if( ( id = itemdb_exists(nameid) ) == NULL )
	{
		ShowWarning("itemdb_read_noequip: Invalid item id %hu.\n", nameid);
		return false;
	}

	if (flag >= 0)
		id->flag.no_equip |= flag;
	else
		id->flag.no_equip &= ~abs(flag);

	return true;
}

/**
 * @return: amount of retrieved entries.
 **/
static int itemdb_combo_split_atoi (char *str, int *val) {
	int i;

	for (i=0; i<MAX_ITEMS_PER_COMBO; i++) {
		if (!str) break;

		val[i] = atoi(str);

		str = strchr(str,':');

		if (str)
			*str++=0;
	}

	if( i == 0 ) //No data found.
		return 0;

	return i;
}

/**
 * <combo{:combo{:combo:{..}}}>,<{ script }>
 **/
static void itemdb_read_combos(const char* basedir, bool silent) {
	uint32 lines = 0, count = 0;
	char line[1024];

	char path[256];
	FILE* fp;

	sprintf(path, "%s/%s", basedir, "item_combo_db.txt");

	if ((fp = fopen(path, "r")) == NULL) {
		if(silent==0) ShowError("itemdb_read_combos: File not found \"%s\".\n", path);
		return;
	}

	// process rows one by one
	while(fgets(line, sizeof(line), fp)) {
		char *str[2], *p;

		lines++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		memset(str, 0, sizeof(str));

		p = line;

		p = trim(p);

		if (*p == '\0')
			continue;// empty line

		if (!strchr(p,','))
		{
			/* is there even a single column? */
			ShowError("itemdb_read_combos: Insufficient columns in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}

		str[0] = p;
		p = strchr(p,',');
		*p = '\0';
		p++;

		str[1] = p;
		p = strchr(p,',');
		p++;

		if (str[1][0] != '{') {
			ShowError("itemdb_read_combos(#1): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}
		/* no ending key anywhere (missing \}\) */
		if ( str[1][strlen(str[1])-1] != '}' ) {
			ShowError("itemdb_read_combos(#2): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		} else {
			int items[MAX_ITEMS_PER_COMBO];
			int v = 0, retcount = 0;
			struct item_data * id = NULL;
			int idx = 0;
			if((retcount = itemdb_combo_split_atoi(str[0], items)) < 2) {
				ShowError("itemdb_read_combos: line %d of \"%s\" doesn't have enough items to make for a combo (min:2), skipping.\n", lines, path);
				continue;
			}
			/* validate */
			for(v = 0; v < retcount; v++) {
				if( !itemdb_exists(items[v]) ) {
					ShowError("itemdb_read_combos: line %d of \"%s\" contains unknown item ID %d, skipping.\n", lines, path,items[v]);
					break;
				}
			}
			/* failed at some item */
			if( v < retcount )
				continue;
			id = itemdb_exists(items[0]);
			idx = id->combos_count;
			/* first entry, create */
			if( id->combos == NULL ) {
				CREATE(id->combos, struct item_combo*, 1);
				id->combos_count = 1;
			} else {
				RECREATE(id->combos, struct item_combo*, ++id->combos_count);
			}
			CREATE(id->combos[idx],struct item_combo,1);
			id->combos[idx]->nameid = (unsigned short*)aMalloc( retcount * sizeof(unsigned short) );
			id->combos[idx]->count = retcount;
			id->combos[idx]->script = parse_script(str[1], path, lines, 0);
			id->combos[idx]->id = count;
			id->combos[idx]->isRef = false;
			/* populate ->nameid field */
			for( v = 0; v < retcount; v++ ) {
				id->combos[idx]->nameid[v] = items[v];
			}

			/* populate the children to refer to this combo */
			for( v = 1; v < retcount; v++ ) {
				struct item_data * it;
				int index;
				it = itemdb_exists(items[v]);
				index = it->combos_count;
				if( it->combos == NULL ) {
					CREATE(it->combos, struct item_combo*, 1);
					it->combos_count = 1;
				} else {
					RECREATE(it->combos, struct item_combo*, ++it->combos_count);
				}
				CREATE(it->combos[index],struct item_combo,1);
				/* we copy previously alloc'd pointers and just set it to reference */
				memcpy(it->combos[index],id->combos[idx],sizeof(struct item_combo));
				/* we flag this way to ensure we don't double-dealloc same data */
				it->combos[index]->isRef = true;
			}
			uidb_put(itemdb_combo,id->combos[idx]->id,id->combos[idx]);
		}
		count++;
	}
	fclose(fp);

	ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n",count,path);

	return;
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
			unsigned short item_id, amount;
			int level, flag;

			Sql_GetData(mmysql_handle, 1, &data, NULL); level = atoi(data);
			Sql_GetData(mmysql_handle, 2, &data, NULL); item_id = atoi(data);
			Sql_GetData(mmysql_handle, 3, &data, NULL); amount = atoi(data);
			Sql_GetData(mmysql_handle, 4, &data, NULL); flag = atoi(data);

			if (!itemdb_exists(item_id)) {
				ShowWarning("itemdb_parse_roulette_db: Unknown item ID '%hu' in level '%d'\n", item_id, level);
				continue;
			}
			if (amount < 1 || amount > MAX_AMOUNT){
				ShowWarning("itemdb_parse_roulette_db: Unsupported amount '%hu' for item ID '%hu' in level '%d'\n", amount, item_id, level);
				continue;
			}
			if (flag < 0 || flag > 1) {
				ShowWarning("itemdb_parse_roulette_db: Unsupported flag '%d' for item ID '%hu' in level '%d'\n", flag, item_id, level);
				continue;
			}

			j = rd.items[i];
			RECREATE(rd.nameid[i], unsigned short, ++rd.items[i]);
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
		RECREATE(rd.nameid[i], unsigned short, rd.items[i]);
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

/*======================================
 * Applies gender restrictions according to settings. [Skotlex]
 *======================================*/
static char itemdb_gendercheck(struct item_data *id)
{
	if (id->nameid == WEDDING_RING_M) //Grom Ring
		return 1;
	if (id->nameid == WEDDING_RING_F) //Bride Ring
		return 0;
	if (id->look == W_MUSICAL && id->type == IT_WEAPON) //Musical instruments are always male-only
		return 1;
	if (id->look == W_WHIP && id->type == IT_WEAPON) //Whips are always female-only
		return 0;

	return (battle_config.ignore_items_gender) ? 2 : id->sex;
}

/**
 * Read item from item db
 * item_db2 overwriting item_db
 */
void itemdb_readdb(void)
{
	item_db.load();
}

/**
* Read item_db table
*/
static int itemdb_read_sqldb(void) {

	const char* item_db_name[] = {
		item_table,
		item2_table
	};
	int fi;

	for( fi = 0; fi < ARRAYLENGTH(item_db_name); ++fi ) {
		uint32 lines = 0, count = 0;

		// retrieve all rows from the item database
		if( SQL_ERROR == Sql_Query(mmysql_handle, "SELECT * FROM `%s`", item_db_name[fi]) ) {
			Sql_ShowDebug(mmysql_handle);
			continue;
		}

		// process rows one by one
		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {// wrap the result into a TXT-compatible format
			char* str[22];
			char dummy[256] = "";
			int i;
			++lines;
			for( i = 0; i < 22; ++i ) {
				Sql_GetData(mmysql_handle, i, &str[i], NULL);
				if( str[i] == NULL )
					str[i] = dummy; // get rid of NULL columns
			}

			//if (!itemdb_parse_dbrow(str, item_db_name[fi], lines, SCRIPT_IGNORE_EXTERNAL_BRACKETS)) // TODO?
			//	continue;
			++count;
		}

		// free the query result
		Sql_FreeResult(mmysql_handle);

		ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, item_db_name[fi]);
	}

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

/**
* Retrieves random option data
*/
struct s_random_opt_data* itemdb_randomopt_exists(short id) {
	return ((struct s_random_opt_data*)uidb_get(itemdb_randomopt, id));
}

/** Random option
* <ID>,<{Script}>
*/
static bool itemdb_read_randomopt(const char* basedir, bool silent) {
	uint32 lines = 0, count = 0;
	char line[1024];

	char path[256];
	FILE* fp;

	sprintf(path, "%s/%s", basedir, "item_randomopt_db.txt");

	if ((fp = fopen(path, "r")) == NULL) {
		if (silent == 0) ShowError("itemdb_read_randomopt: File not found \"%s\".\n", path);
		return false;
	}

	while (fgets(line, sizeof(line), fp)) {
		char *str[2], *p;

		lines++;

		if (line[0] == '/' && line[1] == '/') // Ignore comments
			continue;

		memset(str, 0, sizeof(str));

		p = line;

		p = trim(p);

		if (*p == '\0')
			continue;// empty line

		if (!strchr(p, ','))
		{
			ShowError("itemdb_read_randomopt: Insufficient columns in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}

		str[0] = p;
		p = strchr(p, ',');
		*p = '\0';
		p++;

		str[1] = p;

		if (str[1][0] != '{') {
			ShowError("itemdb_read_randomopt(#1): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}
		/* no ending key anywhere (missing \}\) */
		if (str[1][strlen(str[1]) - 1] != '}') {
			ShowError("itemdb_read_randomopt(#2): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}
		else {
			int id = -1;
			struct s_random_opt_data *data;
			struct script_code *code;

			str[0] = trim(str[0]);
			if (ISDIGIT(str[0][0])) {
				id = atoi(str[0]);
			}
			else {
				int64 id_tmp;

				if (!script_get_constant(str[0], &id_tmp)) {
					ShowError("itemdb_read_randopt: Unknown random option constant \"%s\".\n", str[0]);
					continue;
				}
				id = static_cast<int>(id_tmp);
			}

			if (id < 0) {
				ShowError("itemdb_read_randomopt: Invalid Random Option ID '%s' in line %d of \"%s\", skipping.\n", str[0], lines, path);
				continue;
			}

			if ((data = itemdb_randomopt_exists(id)) == NULL) {
				CREATE(data, struct s_random_opt_data, 1);
				uidb_put(itemdb_randomopt, id, data);
			}
			data->id = id;
			if ((code = parse_script(str[1], path, lines, 0)) == NULL) {
				ShowWarning("itemdb_read_randomopt: Invalid script on option ID #%d.\n", id);
				continue;
			}
			if (data->script) {
				script_free_code(data->script);
				data->script = NULL;
			}
			data->script = code;
		}
		count++;
	}
	fclose(fp);

	ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, path);

	return true;
}

/**
 * Clear Item Random Option Group from memory
 * @author [Cydh]
 **/
static int itemdb_randomopt_group_free(DBKey key, DBData *data, va_list ap) {
	struct s_random_opt_group *g = (struct s_random_opt_group *)db_data2ptr(data);
	if (!g)
		return 0;
	if (g->entries)
		aFree(g->entries);
	g->entries = NULL;
	aFree(g);
	return 1;
}

/**
 * Get Item Random Option Group from itemdb_randomopt_group MapDB
 * @param id Random Option Group
 * @return Random Option Group data or NULL if not found
 * @author [Cydh]
 **/
struct s_random_opt_group *itemdb_randomopt_group_exists(int id) {
	return (struct s_random_opt_group *)uidb_get(itemdb_randomopt_group, id);
}

/**
 * Read Item Random Option Group from db file
 * @author [Cydh]
 **/
static bool itemdb_read_randomopt_group(char* str[], int columns, int current) {
	int64 id_tmp;
	int id = 0;
	int i;
	unsigned short rate = (unsigned short)strtoul(str[1], NULL, 10);
	struct s_random_opt_group *g = NULL;

	if (!script_get_constant(str[0], &id_tmp)) {
		ShowError("itemdb_read_randomopt_group: Invalid ID for Random Option Group '%s'.\n", str[0]);
		return false;
	}

	id = static_cast<int>(id_tmp);

	if ((columns-2)%3 != 0) {
		ShowError("itemdb_read_randomopt_group: Invalid column entries '%d'.\n", columns);
		return false;
	}

	if (!(g = (struct s_random_opt_group *)uidb_get(itemdb_randomopt_group, id))) {
		CREATE(g, struct s_random_opt_group, 1);
		g->id = id;
		g->total = 0;
		g->entries = NULL;
		uidb_put(itemdb_randomopt_group, g->id, g);
	}

	RECREATE(g->entries, struct s_random_opt_group_entry, g->total + rate);

	for (i = g->total; i < (g->total + rate); i++) {
		int j, k;
		memset(&g->entries[i].option, 0, sizeof(g->entries[i].option));
		for (j = 0, k = 2; k < columns && j < MAX_ITEM_RDM_OPT; k+=3) {
			int64 randid_tmp;
			int randid = 0;

			if (!script_get_constant(str[k], &randid_tmp) || ((randid = static_cast<int>(randid_tmp)) && !itemdb_randomopt_exists(randid))) {
				ShowError("itemdb_read_randomopt_group: Invalid random group id '%s' in column %d!\n", str[k], k+1);
				continue;
			}
			g->entries[i].option[j].id = randid;
			g->entries[i].option[j].value = (short)strtoul(str[k+1], NULL, 10);
			g->entries[i].option[j].param = (char)strtoul(str[k+2], NULL, 10);
			j++;
		}
	}
	g->total += rate;
	return true;
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
		ShowError("SQL Item Database is disabled.\n");
		//itemdb_read_sqldb();
	else
		itemdb_readdb();
	
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
		
		sv_readdb(dbsubpath1, "item_avail.txt",         ',', 2, 2, -1, &itemdb_read_itemavail, i > 0);
		sv_readdb(dbsubpath2, "item_group_db.txt",		',', 2, 10, -1, &itemdb_read_group, i > 0);
		sv_readdb(dbsubpath2, "item_bluebox.txt",		',', 2, 10, -1, &itemdb_read_group, i > 0);
		sv_readdb(dbsubpath2, "item_violetbox.txt",		',', 2, 10, -1, &itemdb_read_group, i > 0);
		sv_readdb(dbsubpath2, "item_cardalbum.txt",		',', 2, 10, -1, &itemdb_read_group, i > 0);
		sv_readdb(dbsubpath1, "item_findingore.txt",	',', 2, 10, -1, &itemdb_read_group, i > 0);
		sv_readdb(dbsubpath2, "item_giftbox.txt",		',', 2, 10, -1, &itemdb_read_group, i > 0);
		sv_readdb(dbsubpath2, "item_misc.txt",			',', 2, 10, -1, &itemdb_read_group, i > 0);
#ifdef RENEWAL
		sv_readdb(dbsubpath2, "item_package.txt",		',', 2, 10, -1, &itemdb_read_group, i > 0);
#endif
		itemdb_read_combos(dbsubpath2,i > 0); //TODO change this to sv_read ? id#script ?
		itemdb_read_randomopt(dbsubpath2, i > 0);
		sv_readdb(dbsubpath2, "item_noequip.txt",       ',', 2, 2, -1, &itemdb_read_noequip, i > 0);
		sv_readdb(dbsubpath2, "item_randomopt_group.txt", ',', 5, 2+5*MAX_ITEM_RDM_OPT, -1, &itemdb_read_randomopt_group, i > 0);
		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}
}

/*==========================================
 * Initialize / Finalize
 *------------------------------------------*/

/**
* Destroys the item_data.
*/
static void destroy_item_data(struct item_data* self) {
	if( self == NULL )
		return;
	// free scripts
	if( self->script )
		script_free_code(self->script);
	if( self->equip_script )
		script_free_code(self->equip_script);
	if( self->unequip_script )
		script_free_code(self->unequip_script);
	if( self->combos_count ) {
		int i;
		for( i = 0; i < self->combos_count; i++ ) {
			if( !self->combos[i]->isRef ) {
				aFree(self->combos[i]->nameid);
				if (self->combos[i]->script)
					script_free_code(self->combos[i]->script);
			}
			aFree(self->combos[i]);
		}
		aFree(self->combos);
	}
#if defined(DEBUG)
	// trash item
	memset(self, 0xDD, sizeof(struct item_data));
#endif
	// free self
	aFree(self);
}

/**
 * @see DBApply
 */
static int itemdb_final_sub(DBKey key, DBData *data, va_list ap)
{
	struct item_data *id = (struct item_data *)db_data2ptr(data);

	destroy_item_data(id);
	return 0;
}

/** NOTE:
* In some OSs, like Raspbian, we aren't allowed to pass 0 in va_list.
* So, itemdb_group_free2 is useful in some cases.
* NB : We keeping that funciton cause that signature is needed for some iterator..
*/
static int itemdb_group_free(DBKey key, DBData *data, va_list ap) {
	return itemdb_group_free2(key,data);
}

/** (ARM)
* Adaptation of itemdb_group_free. This function enables to compile rAthena on Raspbian OS.
*/
static inline int itemdb_group_free2(DBKey key, DBData *data) {
	struct s_item_group_db *group = (struct s_item_group_db *)db_data2ptr(data);
	uint8 j;
	if (!group)
		return 0;
	if (group->must_qty)
		aFree(group->must);
	group->must_qty = 0;
	for (j = 0; j < MAX_ITEMGROUP_RANDGROUP; j++) {
		if (!group->random[j].data_qty || !(&group->random[j]))
			continue;
		aFree(group->random[j].data);
		group->random[j].data_qty = 0;
	}
	aFree(group);
	return 0;
}

static int itemdb_randomopt_free(DBKey key, DBData *data, va_list ap) {
	struct s_random_opt_data *opt = (struct s_random_opt_data *)db_data2ptr(data);
	if (!opt)
		return 0;
	if (opt->script)
		script_free_code(opt->script);
	opt->script = NULL;
	aFree(opt);
	return 1;
}

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

	item_db.clear();
	itemdb_group->clear(itemdb_group, itemdb_group_free);
	itemdb_randomopt->clear(itemdb_randomopt, itemdb_randomopt_free);
	itemdb_randomopt_group->clear(itemdb_randomopt_group, itemdb_randomopt_group_free);
	db_clear(itemdb_combo);
	if (battle_config.feature_roulette)
		itemdb_roulette_free();

	// read new data
	itemdb_read();
	cashshop_reloaddb();

	if (battle_config.feature_roulette)
		itemdb_parse_roulette_db();

	mob_reload_itemmob_data();

	// readjust itemdb pointer cache for each player
	iter = mapit_geteachpc();
	for( sd = (struct map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (struct map_session_data*)mapit_next(iter) ) {
		memset(sd->item_delay, 0, sizeof(sd->item_delay));  // reset item delays

		if( sd->combos.count ) { // clear combo bonuses
			aFree(sd->combos.bonus);
			aFree(sd->combos.id);
			aFree(sd->combos.pos);
			sd->combos.bonus = nullptr;
			sd->combos.id = nullptr;
			sd->combos.pos = nullptr;
			sd->combos.count = 0;
		}

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
	db_destroy(itemdb_combo);
	itemdb_group->destroy(itemdb_group, itemdb_group_free);
	itemdb_randomopt->destroy(itemdb_randomopt, itemdb_randomopt_free);
	itemdb_randomopt_group->destroy(itemdb_randomopt_group, itemdb_randomopt_group_free);
	destroy_item_data(dummy_item);
	if (battle_config.feature_roulette)
		itemdb_roulette_free();
}

/**
* Initializing Item DB
*/
void do_init_itemdb(void) {
	itemdb_combo = uidb_alloc(DB_OPT_BASE);
	itemdb_group = uidb_alloc(DB_OPT_BASE);
	itemdb_randomopt = uidb_alloc(DB_OPT_BASE);
	itemdb_randomopt_group = uidb_alloc(DB_OPT_BASE);
	itemdb_create_dummy();
	itemdb_read();

	if (battle_config.feature_roulette)
		itemdb_parse_roulette_db();
}
