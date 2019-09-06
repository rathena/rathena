// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "item_synthesis.hpp"

#include <algorithm>
#include <memory>

#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"

#include "log.hpp" // e_log_pick_type

ItemSynthesisDatabase item_synthesis_db;

void ItemSynthesisDatabase::clear() {
	TypesafeYamlDatabase::clear();
}

const std::string ItemSynthesisDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_synthesis.yml";
}

/**
* Reads and parses an entry from the item_synthesis file.
* @param node: YAML node containing the entry.
* @return count of successfully parsed rows
*/
uint64 ItemSynthesisDatabase::parseBodyNode(const YAML::Node &node) {
	uint32 id;

	if (!this->asUInt32(node, "Id", id))
		return 0;

	std::shared_ptr<s_item_synthesis_db> entry = this->find(id);
	bool exists = entry != nullptr;

	if (!exists) {
		if (!this->nodeExists(node, "SourceNeeded"))
			return 0;

		if (!this->nodeExists(node, "SourceItem"))
			return 0;

		if (!this->nodeExists(node, "Reward"))
			return 0;

		entry = std::make_shared<s_item_synthesis_db>();
		entry->id = id;
	}

	if (this->nodeExists(node, "SourceNeeded")) {
		if (!this->asUInt16(node, "SourceNeeded", entry->source_needed))
			return 0;
	}

	if (this->nodeExists(node, "NeedRefine")) {
		const YAML::Node& refineNode = node["NeedRefine"];

		if (refineNode.IsScalar()) {
			this->asUInt16(node, "NeedRefine", entry->source_refine_min);
		}
		else {
			if (this->nodeExists(refineNode, "Min"))
				this->asUInt16(refineNode, "Min", entry->source_refine_min);

			if (this->nodeExists(refineNode, "Max"))
				this->asUInt16(refineNode, "Max", entry->source_refine_max);
		}
	}

	if (this->nodeExists(node, "SourceItem")) {
		const YAML::Node& sourceNode = node["SourceItem"];

		if (!entry->sources.empty())
			entry->sources.clear();
		entry->sources.reserve(entry->source_needed);

		for (const YAML::Node &source : sourceNode) {
			s_item_synthesis_source source_item = {};

			if (!this->asUInt32(source, "Item", source_item.nameid))
				continue;

			/*if (!itemdb_exists(source_item.nameid)) {
				this->invalidWarning(sourceNode, "Unknown item with ID %u.\n", source_item.nameid);
				continue;
			}*/

			if (this->nodeExists(source, "Amount"))
				this->asUInt16(source, "Amount", source_item.amount);

			entry->sources.push_back(source_item);
		}
	}

	if (this->nodeExists(node, "Reward")) {
		std::string script_str;
		script_code *code;

		if (!this->asString(node, "Reward", script_str) || !(code = parse_script(script_str.c_str(), this->getCurrentFile().c_str(), id, SCRIPT_IGNORE_EXTERNAL_BRACKETS)) != NULL) {
			this->invalidWarning(node["Reward"], "Invalid item script for 'Reward'.\n");
			return 0;
		}

		if (entry->reward)
			script_free_code(entry->reward);

		entry->reward = code;
	}

	if (!exists)
		this->put(id, entry);

	return 1;
}

/*
* Attempt to open synthesis UI for a player
* @param sd Open UI for this player
* @param itemid ID of synthesis UI
* @return True on succes, false on failure
*/
bool item_synthesis_open(map_session_data *sd, unsigned int itemid) {
	nullpo_retr(false, sd);

	if (sd->state.vending || sd->state.buyingstore || sd->state.trading || sd->state.storage_flag || sd->state.prevend || sd->state.lapine_ui)
		return false;

	if (pc_is90overweight(sd) || !pc_inventoryblank(sd)) {
		clif_msg(sd, ITEM_CANT_OBTAIN_WEIGHT);
		return false;
	}

#ifndef ITEMID_INT32_SUPPORTED
	if (itemid >= UINT16_MAX) {
		ShowError("item_synthesis_open: ID '%u' is not supported by your system. Max ID is %hu.\n", itemid, UINT16_MAX);
		return false;
	}
#endif

	if (!item_synthesis_db.exists(itemid))
		return false;

	if (clif_synthesisui_open(sd, itemid)) {
		sd->last_lapine_box = itemid;
		sd->state.lapine_ui = 1;
	}

	return true;
}

/*
* Proccess synthesis input from player
* @param sd Player who request
* @param itemid ID of synthesis UI
* @param items Item list sent by player
* @return SYNTHESIS_SUCCESS on success. @see e_item_synthesis_result
*/
e_item_synthesis_result item_synthesis_submit(map_session_data *sd, unsigned int itemid, const std::vector<s_item_synthesis_list> items) {
	nullpo_retr(SYNTHESIS_INVALID_ITEM, sd);

	if (!sd->state.lapine_ui || itemid != sd->last_lapine_box) {
		sd->state.lapine_ui = sd->last_lapine_box = 0;
		return SYNTHESIS_INVALID_ITEM;
	}

	auto info = item_synthesis_db.find(itemid);

	if (!info || !info->checkRequirement(sd, items))
		return SYNTHESIS_INSUFFICIENT_AMOUNT;

	if (info->reward)
		run_script(info->reward, 0, sd->status.account_id, 0);

	sd->state.lapine_ui = sd->last_lapine_box = 0;
	return SYNTHESIS_SUCCESS;
}

/**
* Loads item_synthesis db
*/
void item_synthesis_read_db(void)
{
	item_synthesis_db.load();
}

/**
* Reloads the achievement database
*/
void item_synthesis_db_reload(void)
{
	do_final_item_synthesis();
	do_init_item_synthesis();
}

/**
* Initializes the achievement database
*/
void do_init_item_synthesis(void)
{
	item_synthesis_db.load();
}

/**
* Finalizes the achievement database
*/
void do_final_item_synthesis(void) {
	item_synthesis_db.clear();
}

/**
* Constructor
*/
s_item_synthesis_db::s_item_synthesis_db()
	: source_needed(0)
	, sources()
	, reward(nullptr)
	, source_refine_min(0)
	, source_refine_max(MAX_REFINE)
{}

/**
* Destructor
*/
s_item_synthesis_db::~s_item_synthesis_db()
{
	if (this->reward) {
		script_free_code(this->reward);
		this->reward = nullptr;
	}
}

bool s_item_synthesis_db::sourceExists(uint32 source_id)
{
	if (this->sources.empty())
		return false;
	auto source = std::find_if(
		this->sources.begin(), this->sources.end(),
		[&source_id](const s_item_synthesis_source &source) { return source.nameid == source_id; }
	);
	return (source != this->sources.end());
}

bool s_item_synthesis_db::checkRequirement(map_session_data *sd, const std::vector<s_item_synthesis_list> items)
{
	if (items.empty() || items.size() != this->source_needed)
		return false;

	item *item = NULL;
	item_data *id = NULL;

	for (auto &it : items) {
		if (it.index < 0 || it.index >= MAX_INVENTORY)
			return false;

		if (!(item = &sd->inventory.u.items_inventory[it.index]) || !(id = sd->inventory_data[it.index]))
			return false;

		if (item->equip || item->expire_time || item->amount < it.amount)
			return false;

		if (!this->sourceExists(item->nameid))
			return false;

		if (item->refine < this->source_refine_min)
			return false;

		if (item->refine > this->source_refine_max)
			return false;

		if (pc_delitem(sd, it.index, it.amount, 0, 0, LOG_TYPE_OTHER) != 0)
			return false;
	}

	return true;
}

s_item_synthesis_source::s_item_synthesis_source()
	: amount(1)
{
}
