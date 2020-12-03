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
	std::string synthesis_item_name;

	if (!this->asString(node, "Item", synthesis_item_name))
		return 0;

	item_data *item = itemdb_search_aegisname(synthesis_item_name.c_str());

	if (item == nullptr) {
		this->invalidWarning(node["Item"], "Item name for Synthesis Box %s does not exist.\n", synthesis_item_name.c_str());
		return 0;
	}

	std::shared_ptr<s_item_synthesis_db> entry = this->find(item->nameid);
	bool exists = entry != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "SourceItem", "Reward" }))
			return 0;

		entry = std::make_shared<s_item_synthesis_db>();
		entry->id = item->nameid;
	}

	if (this->nodeExists(node, "SourceNeeded")) {
		if (!this->asUInt16(node, "SourceNeeded", entry->source_needed))
			return 0;
	}

	if (this->nodeExists(node, "NeedRefineMin")) {
		if (!this->asUInt16(node, "NeedRefineMin", entry->source_refine_min))
			return 0;
	}

	if (this->nodeExists(node, "NeedRefineMax")) {
		if (!this->asUInt16(node, "NeedRefineMax", entry->source_refine_max))
			return 0;
	}

	if (exists && this->nodeExists(node, "ClearSourceItem")) {
		ShowNotice("item_synthesis: Cleared all items in SourceItem. Synthesis: %s (%u)\n", item->name.c_str(), item->nameid);
		if (!entry->sources.empty())
			entry->sources.clear();
	}

	if (this->nodeExists(node, "SourceItem")) {
		const YAML::Node& sourceNode = node["SourceItem"];

		if (!exists)
			entry->sources.reserve(entry->source_needed);

		for (const YAML::Node &source : sourceNode) {
			std::string source_item_name;

			if (!this->asString(source, "Item", source_item_name))
				continue;

			item_data *source_it = itemdb_search_aegisname(source_item_name.c_str());

			if (source_it == nullptr) {
				this->invalidWarning(node["SourceItem"], "Source item name %s does not exist, skipping.\n", source_item_name.c_str());
				continue;
			}

			s_item_synthesis_source source_item = {};

			source_item.nameid = source_it->nameid;

			if (exists && this->nodeExists(source, "Remove")) {
				entry->sources.erase(std::remove_if(entry->sources.begin(), entry->sources.end(), [&source_item](const s_item_synthesis_source &x) { return x.nameid == source_item.nameid; }));
				ShowNotice("item_synthesis: Removed %s (%u) from SourceItem. Synthesis: %s (%u)\n", source_it->name.c_str(), source_item.nameid, item->name.c_str(), item->nameid);
				continue;
			}

			if (this->nodeExists(source, "Amount"))
				this->asUInt16(source, "Amount", source_item.amount);

			if (entry->sources.end() == std::find_if(
				entry->sources.begin(), entry->sources.end(), [&source_item](s_item_synthesis_source &x)
			{
				if (x.nameid == source_item.nameid) {
					x.amount = source_item.amount;
					return true;
				}
				return false;
			}
			))
				entry->sources.push_back(source_item);
		}
	}

	if (this->nodeExists(node, "Reward")) {
		std::string script_str;
		script_code *code;

		if (!this->asString(node, "Reward", script_str))
			return 0;

		if (!(code = parse_script(script_str.c_str(), this->getCurrentFile().c_str(), item->nameid, SCRIPT_IGNORE_EXTERNAL_BRACKETS))) {
			this->invalidWarning(node["Reward"], "Invalid Reward item script.\n");
			return 0;
		}

		if (entry->reward)
			script_free_code(entry->reward);

		entry->reward = code;
	}

	if (!exists)
		this->put(item->nameid, entry);

	return 1;
}

/**
 * Attempt to open synthesis UI for a player
 * @param sd: Open UI for this player
 * @param itemid: ID of synthesis UI
 * @return True on succes, false on failure
 */
bool item_synthesis_open(map_session_data *sd, t_itemid itemid) {
	nullpo_retr(false, sd);

	if (pc_cant_act2(sd) || (sd)->chatID)
		return false;

	if (pc_is90overweight(sd) || !pc_inventoryblank(sd)) {
		clif_msg(sd, ITEM_CANT_OBTAIN_WEIGHT);
		return false;
	}

	if (!item_synthesis_db.exists(itemid))
		return false;

	if (clif_synthesisui_open(sd, itemid)) {
		sd->last_lapine_box = itemid;
		sd->state.lapine_ui = 1;
	}

	return true;
}

/**
 * Process synthesis input from player
 * @param sd: Player who request
 * @param itemid: ID of synthesis UI
 * @param items: Item list sent by player
 * @return SYNTHESIS_SUCCESS on success. @see e_item_synthesis_result
 */
e_item_synthesis_result item_synthesis_submit(map_session_data *sd, t_itemid itemid, const std::vector<s_item_synthesis_list> items) {
	nullpo_retr(SYNTHESIS_INVALID_ITEM, sd);

	if (!sd->state.lapine_ui || itemid != sd->last_lapine_box) {
		sd->state.lapine_ui = sd->last_lapine_box = 0;
		return SYNTHESIS_INVALID_ITEM;
	}

	auto info = item_synthesis_db.find(itemid);

	if (!info || !info->checkRequirement(sd, items))
		return SYNTHESIS_INSUFFICIENT_AMOUNT;

	if (!info->deleteRequirement(sd, items))
		return SYNTHESIS_INSUFFICIENT_AMOUNT;

	if (info->reward)
		run_script(info->reward, 0, sd->status.account_id, 0);

	sd->state.lapine_ui = sd->last_lapine_box = 0;
	return SYNTHESIS_SUCCESS;
}

/**
 * Loads lapine synthesis database
 */
void item_synthesis_read_db(void)
{
	item_synthesis_db.load();
}

/**
 * Reloads the lapine synthesis database
 */
void item_synthesis_db_reload(void)
{
	do_final_item_synthesis();
	do_init_item_synthesis();
}

/**
 * Initializes the lapine synthesis database
 */
void do_init_item_synthesis(void)
{
	item_synthesis_db.load();
}

/**
 * Finalizes the lapine synthesis database
 */
void do_final_item_synthesis(void) {
	item_synthesis_db.clear();
}

/**
 * Constructor
 */
s_item_synthesis_db::s_item_synthesis_db()
	: source_needed(1)
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

/**
 * Check if the source for synthesis item is exists
 * @param source_id: Item ID of source item
 * @return true if source exists, false if doesn't
 */
bool s_item_synthesis_db::sourceExists(t_itemid source_id)
{
	if (this->sources.empty())
		return false;
	auto source = std::find_if(
		this->sources.begin(), this->sources.end(),
		[&source_id](const s_item_synthesis_source &source) { return source.nameid == source_id; }
	);
	return (source != this->sources.end());
}

/**
 * Check all submitted items are valid
 * @param sd: Player
 * @param items: Submitted items by player
 * @return True if all items are valid
 */
bool s_item_synthesis_db::checkRequirement(map_session_data *sd, const std::vector<s_item_synthesis_list> items)
{
	if (items.empty() || items.size() != this->source_needed)
		return false;

	item *item = nullptr;
	item_data *id = nullptr;
	std::vector<int> indexes(this->source_needed);

	for (auto &it : items) {
		if (it.index >= MAX_INVENTORY)
			return false;

		if (!(item = &sd->inventory.u.items_inventory[it.index]) || !(id = sd->inventory_data[it.index]))
			return false;

		if (item->equip || item->expire_time || item->amount < it.amount || item->identify != 1)
			return false;

		if (!this->sourceExists(item->nameid))
			return false;

		if (item->refine < this->source_refine_min)
			return false;

		if (item->refine > this->source_refine_max)
			return false;

		if (std::find(indexes.begin(), indexes.end(), it.index) != indexes.end())
			return false;

		indexes.push_back(it.index);
	}

	return true;
}

/**
 * Delete all submitted items for synthesis
 * @param sd: Player
 * @param items: Submitted items by player
 * @return True if all items are deleted
 */
bool s_item_synthesis_db::deleteRequirement(map_session_data *sd, const std::vector<s_item_synthesis_list> items)
{
	if (items.empty() || items.size() != this->source_needed)
		return false;

	for (auto &it : items) {
		if (it.index >= MAX_INVENTORY)
			return false;

		if (pc_delitem(sd, it.index, it.amount, 0, 0, LOG_TYPE_OTHER) != 0)
			return false;
	}

	return true;
}

/**
 * Synthesis items constructor.
 * Set default amount to 1
 */
s_item_synthesis_source::s_item_synthesis_source()
	: amount(1)
{
}
