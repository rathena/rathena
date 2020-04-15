// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "item_upgrade.hpp"

#include <algorithm>
#include <memory>

#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"

#include "log.hpp" // e_log_pick_type

ItemUpgradeDatabase item_upgrade_db;

void ItemUpgradeDatabase::clear() {
	TypesafeYamlDatabase::clear();
}

const std::string ItemUpgradeDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_upgrade.yml";
}

/**
* Reads and parses an entry from the item_upgrade file.
* @param node: YAML node containing the entry.
* @return count of successfully parsed rows
*/
uint64 ItemUpgradeDatabase::parseBodyNode(const YAML::Node &node) {
	uint32 id;

	if (!this->asUInt32(node, "Id", id))
		return 0;

	std::shared_ptr<s_item_upgrade_db> entry = this->find(id);
	bool exists = entry != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "TargetItem", "Result" }))
			return 0;

		entry = std::make_shared<s_item_upgrade_db>();
		entry->id = id;
	}

	if (this->nodeExists(node, "Result")) {
		std::string script_str;
		script_code *code;

		if (!this->asString(node, "Result", script_str))
			return 0;

		if (!(code = parse_script(script_str.c_str(), this->getCurrentFile().c_str(), id, SCRIPT_IGNORE_EXTERNAL_BRACKETS))) {
			this->invalidWarning(node["Result"], "Invalid item script for 'Result'.\n");
			return 0;
		}

		if (entry->result)
			script_free_code(entry->result);

		entry->result = code;
	}

	if (exists && this->nodeExists(node, "ClearTargetItem")) {
		ShowNotice("item_upgrade: Cleared all items in TargetItem. Upgrade ID: %d\n", id);
		if (!entry->targets.empty())
			entry->targets.clear();
	}

	if (this->nodeExists(node, "TargetItem")) {
		const YAML::Node& targetNode = node["TargetItem"];
		unsigned int itemid;

		for (const YAML::Node &target : targetNode) {
			if (!this->asUInt32(target, "Item", itemid))
				continue;

			/*if (!itemdb_exists(itemid)) {
				this->invalidWarning(target, "Unknown item with ID %u.\n", itemid);
				continue;
			}*/

			if (exists && this->nodeExists(target, "Remove")) {
				entry->targets.erase(std::remove_if(entry->targets.begin(), entry->targets.end(), [&itemid](const unsigned int &x) { return x == itemid; }));
				ShowNotice("item_upgrade: Removed '%d' from TargetItem. Upgrade ID: %d\n", itemid, id);
				continue;
			}

			entry->targets.push_back(itemid);
		}
	}

	if (this->nodeExists(node, "NeedRefineMin")) {
		if (!this->asUInt16(node, "NeedRefineMin", entry->source_refine_min))
			return 0;
	}

	if (this->nodeExists(node, "NeedRefineMax")) {
		if (!this->asUInt16(node, "NeedRefineMax", entry->source_refine_max))
			return 0;
	}

	if (this->nodeExists(node, "NeedOptionNumMin")) {
		if (!this->asUInt16(node, "NeedOptionNumMin", entry->need_option_num))
			return 0;
	}

	if (this->nodeExists(node, "NotSocketEnchantItem")) {
		if (!this->asBool(node, "NotSocketEnchantItem", entry->not_socket_enchant))
			return 0;
	}

	if (!exists)
		this->put(id, entry);

	return 1;
}

/*
* Attempt to open upgrade UI for a player
* @param sd Open UI for this player
* @param itemid ID of upgrade UI
* @return True on succes, false on failure
*/
bool item_upgrade_open(map_session_data *sd, unsigned int itemid) {
	nullpo_retr(false, sd);

	if (pc_cant_act2(sd) || (sd)->chatID)
		return false;

	if (pc_is90overweight(sd) || !pc_inventoryblank(sd)) {
		clif_msg(sd, ITEM_CANT_OBTAIN_WEIGHT);
		return false;
	}

#ifndef ITEMID_INT32_SUPPORTED
	if (itemid >= UINT16_MAX) {
		ShowError("item_upgrade_open: ID '%u' is not supported by your system. Max ID is %hu.\n", itemid, UINT16_MAX);
		return false;
	}
#endif

	if (!item_upgrade_db.exists(itemid))
		return false;

	if (clif_lapine_upgrade_open(sd, itemid)) {
		sd->last_lapine_box = itemid;
		sd->state.lapine_ui = 2;
	}

	return true;
}

/*
* Process selected item from player's input
* @param sd Player
* @param source_itemid Item ID of source item to open Upgrade UI
* @param target_index Index of target item in player's inventory
* @return LAPINE_UPRAGDE_SUCCESS on success. @see e_item_upgrade_result
*/
e_item_upgrade_result item_upgrade_submit(map_session_data *sd, unsigned int source_itemid, uint16 target_index) {
	nullpo_retr(LAPINE_UPRAGDE_FAILURE, sd);

	if (!sd->state.lapine_ui || source_itemid != sd->last_lapine_box) {
		sd->state.lapine_ui = sd->last_lapine_box = 0;
		return LAPINE_UPRAGDE_FAILURE;
	}

	item *it;

	if (target_index >= MAX_INVENTORY || !sd->inventory_data[target_index] || !(it = &sd->inventory.u.items_inventory[target_index]))
		return LAPINE_UPRAGDE_FAILURE;

	if (it->expire_time || it->equip || it->identify != 1)
		return LAPINE_UPRAGDE_FAILURE;

	auto info = item_upgrade_db.find(source_itemid);

	if (!info || !info->targetExists(it->nameid) || !info->checkRequirement(it, sd->inventory_data[target_index]))
		return LAPINE_UPRAGDE_FAILURE;

	info->setPlayerInfo(sd, target_index, it);

	if (info->delete_target_onsuccess)
		pc_delitem(sd, target_index, 1, 0, 0, LOG_TYPE_OTHER);
	sd->state.lapine_ui = sd->last_lapine_box = 0;

	if (info->result)
		run_script(info->result, 0, sd->status.account_id, 0);

	return LAPINE_UPRAGDE_SUCCESS;
}

/**
* Loads lapine upgrade database
*/
void item_upgrade_read_db(void)
{
	item_upgrade_db.load();
}

/**
* Reloads the lapine upgrade database
*/
void item_upgrade_db_reload(void)
{
	do_final_item_upgrade();
	do_init_item_upgrade();
}

/**
* Initializes the lapine upgrade database
*/
void do_init_item_upgrade(void)
{
	item_upgrade_db.load();
}

/**
* Finalizes the lapine upgrade database
*/
void do_final_item_upgrade(void) {
	item_upgrade_db.clear();
}

/**
* Constructor
*/
s_item_upgrade_db::s_item_upgrade_db()
	: targets()
	, result(nullptr)
	, source_refine_min(0)
	, source_refine_max(MAX_REFINE)
	, need_option_num(0)
	, not_socket_enchant(false)
	, delete_target_onsuccess(true)
{}

/**
* Destructor
*/
s_item_upgrade_db::~s_item_upgrade_db()
{
	if (this->result) {
		script_free_code(this->result);
		this->result = nullptr;
	}
}

/*
* Check if submitted target item is valid
* @param target_id Item ID of target item
* @return True if exist, false if not
*/
bool s_item_upgrade_db::targetExists(uint32 target_id)
{
	if (this->targets.empty())
		return false;
	auto target = std::find(this->targets.begin(), this->targets.end(), target_id);
	return (target != this->targets.end());
}

/*
* Check if the target item is valid
* @param it Target item
* @param id Item data
* @return True if valid, false if invalid
*/
bool s_item_upgrade_db::checkRequirement(item *it, item_data *id)
{
	if (this->source_refine_min > it->refine)
		return false;

	if (this->source_refine_max < it->refine)
		return false;

	if (this->not_socket_enchant) {
		for (int i = id->slot; i < MAX_SLOTS; i++) {
			if (it->card[i])
				return false;
		}
	}

	if (this->need_option_num) {
		int c = 0;
		for (int i = 0; i < MAX_ITEM_RDM_OPT; i++) {
			if (it->option[i].id)
				c++;
		}
		if (c < this->need_option_num)
			return false;
	}

	return true;
}

/*
* Set variables for player on success upgrade process
* @param sd Player
* @param target_index Index of player's inventory items as upgrade target
* @param it Latest item data
*/
void s_item_upgrade_db::setPlayerInfo(map_session_data * sd, uint16 target_index, item *it)
{
	pc_setreg(sd, add_str("@last_lapine_id"), it->nameid);
	pc_setreg(sd, add_str("@last_lapine_idx"), target_index);
	pc_setreg(sd, add_str("@last_lapine_refine"), it->refine);
	pc_setreg(sd, add_str("@last_lapine_attribute"), it->attribute);
	pc_setreg(sd, add_str("@last_lapine_card1"), it->card[0]);
	pc_setreg(sd, add_str("@last_lapine_card2"), it->card[1]);
	pc_setreg(sd, add_str("@last_lapine_card3"), it->card[2]);
	pc_setreg(sd, add_str("@last_lapine_card4"), it->card[3]);
	pc_setreg(sd, add_str("@last_lapine_bound"), it->bound);

	char unique_id[23];
	memset(unique_id, '\0', sizeof(unique_id));
	snprintf(unique_id, sizeof(unique_id), "%llu", (unsigned long long)it->unique_id);
	pc_setregstr(sd, add_str("@last_lapine_uniqueid$"), unique_id);

	int key_opt_id = 0, key_opt_value = 0, key_opt_param = 0;
	script_cleararray_pc(sd, "@last_lapine_option_id");
	script_cleararray_pc(sd, "@last_lapine_option_value");
	script_cleararray_pc(sd, "@last_lapine_option_param");

	for (int i = 0; i < MAX_ITEM_RDM_OPT; i++) {
		script_setarray_pc(sd, "@last_lapine_option_id", i, (intptr_t)it->option[i].id, &key_opt_id);
		script_setarray_pc(sd, "@last_lapine_option_value", i, (intptr_t)it->option[i].value, &key_opt_value);
		script_setarray_pc(sd, "@last_lapine_option_param", i, (intptr_t)it->option[i].param, &key_opt_param);
	}
}
