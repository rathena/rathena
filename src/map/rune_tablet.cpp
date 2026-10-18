// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
// Rune Tablet
// Original source: Scanty (https://rathena.org/board/profile/35710-scanty/)
// Multiple equipped Rune Sets: Louis T Steinhil

#include "rune_tablet.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <optional>

#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "achievement.hpp"
#include "battle.hpp"
#include "clif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pc.hpp"
#include "script.hpp"
#include "status.hpp"

RuneTabletDatabase rune_tablet_db;
RuneTabletDecompositionDatabase rune_tablet_decomposition_db;

/// Pure Rune Tablet rules: no RNG, registry or inventory access.
namespace rune_tablet_logic {
/// Legacy progress 1..16 remains valid. Upgraded state uses a version marker
/// and packs level+1 plus a uint16 failure count in one character registry value.
constexpr int64 upgrade_state_marker = int64(1) << 32;

/**
 * Upgrade level of a progress value.
 * @param progress: Character registry value
 * @return Level 0..15, or -1 when not activated or invalid
 */
constexpr int32 level(int64 progress) {
	if (progress >= 1 && progress <= 16)
		return static_cast<int32>(progress - 1);
	if (progress < upgrade_state_marker)
		return -1;
	const int64 payload = progress - upgrade_state_marker;
	if (payload <= 0xffff10 && (payload & 255) >= 1 && (payload & 255) <= 16)
		return static_cast<int32>((payload & 255) - 1);
	return -1;
}

/**
 * Failed attempts at the current level of a progress value.
 * @param progress: Character registry value
 * @return Failure count, or -1 when not activated or invalid
 */
constexpr int32 failures(int64 progress) {
	return level(progress) < 0 ? -1 : progress < upgrade_state_marker ? 0
		: static_cast<int32>((progress - upgrade_state_marker) >> 8);
}

/**
 * Encode an upgrade level and failure count.
 * @return Registry value, or 0 for an invalid state
 */
constexpr int64 progress_value(int32 upgrade, int32 failure_count) {
	if (upgrade < 0 || upgrade > MAX_RUNE_TABLET_UPGRADE || failure_count < 0 || failure_count > 65535)
		return 0;
	return failure_count == 0 ? upgrade + 1
		: upgrade_state_marker + (int64(failure_count) << 8) + upgrade + 1;
}

/**
 * Value of an effect for the given state.
 * @return 0 while the fragment or level requirement is not met
 */
constexpr int32 effect_value(const s_rune_tablet_effect& rule, uint16 fragments, int32 upgrade) {
	if (upgrade < 0 || upgrade > MAX_RUNE_TABLET_UPGRADE || !rule.level_step || fragments < rule.fragments || upgrade < rule.min_level)
		return 0;
	return rule.base + (upgrade / rule.level_step) * rule.per_step;
}

/// Client 0x766f80: base + failures * increment; 100000 represents 100%.
/// Raw arithmetic is kept separate from probability saturation. This does not
/// decide whether a successful upgrade resets the failure counter.
constexpr uint32 upgrade_probability_scale = 100000;

constexpr uint64 upgrade_rate_raw(uint32 base, uint32 increment, uint16 failed) {
	return uint64(base) + uint64(increment) * failed;
}

constexpr uint32 upgrade_chance(uint32 base, uint32 increment, uint16 failed) {
	const uint64 raw = upgrade_rate_raw(base, increment, failed);
	return raw >= upgrade_probability_scale ? upgrade_probability_scale : static_cast<uint32>(raw);
}

/**
 * Next progress value after one upgrade attempt. No RNG or inventory mutation.
 * @param roll: Uniform roll in [0, 100000)
 * @return Next value, or 0 for an invalid or non-upgradeable state (must not be charged)
 */
constexpr int64 upgrade_outcome(int64 progress, uint32 base, uint32 increment, uint32 roll) {
	const int32 current = level(progress), failed = failures(progress);
	if (current < 0 || current >= MAX_RUNE_TABLET_UPGRADE || failed < 0 || roll >= upgrade_probability_scale
		|| base > upgrade_probability_scale || increment > upgrade_probability_scale)
		return 0;
	if (roll < upgrade_chance(base, increment, static_cast<uint16>(failed)))
		return progress_value(current + 1, 0);
	return failed == 65535 ? 0 : progress_value(current, failed + 1);
}

/// Collection reward results: the client's 0x0c17 messages (msgstringtable 4202,
/// 4210..4215): MSI_RUNESYSTEM_REWARD_FAIL, _CANNOT_FIND_ITEM, _MAX_INVENTORY,
/// _HEAVY_INVENTORY, _OVER_ITEM_COUNT, _RUNE_UI_NOT_OPENED, _TABLE_NOT_EXIT.
enum e_reward_result : uint8 {
	reward_ok = 0,
	reward_need_rune = 1,
	reward_mismatch = 2,
	reward_no_space = 3,
	reward_overweight = 4,
	reward_over_count = 5,
	reward_ui_closed = 6,
	reward_no_table = 7,
};

/**
 * Next non-empty reward slot; slots are claimed strictly in order.
 * @param last_claimed: Last claimed slot (0..7)
 * @return Slot 1..7, or 0 when none remain
 */
constexpr int32 next_reward_slot(const t_itemid (&items)[MAX_RUNE_TABLET_REWARD], int32 last_claimed) {
	if (last_claimed < 0 || last_claimed > MAX_RUNE_TABLET_REWARD)
		return 0;
	for (int32 slot = last_claimed + 1; slot <= MAX_RUNE_TABLET_REWARD; ++slot)
		if (items[slot - 1])
			return slot;
	return 0;
}

/**
 * Client 0x81cfd0/0x766c70/0x767fa0: slot 1 needs the set activated, slots
 * 2..6 at least that many active fragments, slot 7 only the preceding claim.
 * @return e_reward_result
 */
constexpr uint8 reward_check(const t_itemid (&items)[MAX_RUNE_TABLET_REWARD], int32 last_claimed,
	int32 requested, bool set_active, int32 active_fragments) {
	if (!set_active)
		return reward_need_rune;
	const int32 next = next_reward_slot(items, last_claimed);
	if (!next || requested != next)
		return reward_mismatch;
	if (requested >= 2 && requested <= 6 && active_fragments < requested)
		return reward_need_rune;
	return reward_ok;
}

/// Inventory view of the debit planner.
struct s_inventory_slot {
	t_itemid item;
	uint32 amount;
	bool eligible;
};

/// Planned removal from one inventory index.
struct s_debit {
	size_t index;
	uint32 amount;
};

/**
 * Build the complete debit plan before modifying inventory. A missing material
 * never returns a partial plan; split stacks and duplicate costs are supported.
 * @return Debits, or nullopt when the inventory cannot pay every cost
 */
static std::optional<std::vector<s_debit>> plan(const std::vector<s_rune_tablet_material>& costs,
	const std::vector<s_inventory_slot>& inventory) {
	if (costs.empty())
		return std::nullopt;

	std::map<t_itemid, uint64> remaining;

	for (const s_rune_tablet_material& cost : costs) {
		if (!cost.item || !cost.amount)
			return std::nullopt;
		remaining[cost.item] += cost.amount;
	}

	std::vector<s_debit> result;

	for (size_t i = 0; i < inventory.size(); ++i) {
		const s_inventory_slot& slot = inventory[i];
		auto required = remaining.find(slot.item);

		if (!slot.eligible || required == remaining.end() || !required->second)
			continue;

		uint32 amount = static_cast<uint32>(required->second < slot.amount ? required->second : slot.amount);

		if (amount) {
			result.push_back({ i, amount });
			required->second -= amount;
		}
	}

	for (const auto& cost : remaining)
		if (cost.second)
			return std::nullopt;

	return result;
}
}

/// Packet serializers. Formats from the client executable, see doc/rune_tablet.txt.
namespace rune_tablet_wire {
constexpr void put16(uint8* p, uint16 value) {
	p[0] = static_cast<uint8>(value);
	p[1] = static_cast<uint8>(value >> 8);
}

constexpr void put32(uint8* p, uint32 value) {
	put16(p, static_cast<uint16>(value));
	put16(p + 2, static_cast<uint16>(value >> 16));
}

constexpr uint16 get16(const uint8* p) {
	return static_cast<uint16>(p[0] | (uint16(p[1]) << 8));
}

constexpr uint32 get32(const uint8* p) {
	return get16(p) | (uint32(get16(p + 2)) << 16);
}

/// 0x0bdf <state>.B
constexpr std::array<uint8, 3> window(bool open) {
	std::array<uint8, 3> p{};
	put16(p.data(), open_window);
	p[2] = open ? 1 : 0;
	return p;
}

/// Empty list header: <length>.W <reserved>.B <category>.W <count>.W
constexpr std::array<uint8, 9> empty_list(uint16 type, uint16 category) {
	std::array<uint8, 9> p{};
	put16(p.data(), type);
	put16(p.data() + 2, static_cast<uint16>(p.size()));
	put16(p.data() + 5, category);
	return p;
}

/// <result>.B <category>.W <id>.L [...]; result 11 is MSI_RUNESYSTEM_DB_LOAD_NOT_YET.
template<size_t N>
constexpr std::array<uint8, N> operation(uint16 type, uint8 result, uint16 category, uint32 id) {
	static_assert(N == 9 || N == 13, "Rune operation response size");
	std::array<uint8, N> p{};
	put16(p.data(), type);
	p[2] = result;
	put16(p.data() + 3, category);
	put32(p.data() + 5, id);
	return p;
}

/// 0x0bd3 uses the same dispatcher as activation. Result 0 updates state for
/// both a success and a failed roll: the client tells them apart by the
/// changed level/failure count, not by a separate error.
constexpr std::array<uint8, 13> upgrade_state(uint8 result, uint16 category,
	uint32 id, uint16 level, uint16 failed) {
	std::array<uint8, 13> packet = operation<13>(upgrade_result, result, category, id);
	put16(packet.data() + 9, level);
	put16(packet.data() + 11, failed);
	return packet;
}

/// 0x0bd8 <item id>.L <client index>.L <mode>.L; client index = server index + 2.
struct s_decomposition_request {
	uint32 item_id;
	uint32 server_index;
	uint32 mode;
};

/**
 * Validate the full uint32 index before subtraction, narrowing or inventory access.
 * @return false for a malformed request
 */
constexpr bool read_decomposition_request(const uint8* data, size_t length,
	uint32 inventory_capacity, s_decomposition_request& request) {
	if (!data || length != 14 || get16(data) != decompose)
		return false;
	const uint32 item_id = get32(data + 2);
	const uint32 index = get32(data + 6);
	const uint32 mode = get32(data + 10);
	if (!item_id || index < 2 || index - 2 >= inventory_capacity || (mode != 1 && mode != 2))
		return false;
	request = { item_id, index - 2, mode };
	return true;
}

/// One 0x0bd9 result position.
struct s_decomposition_result {
	uint32 id;
	uint32 amount;
};

/**
 * 0x0bd9: eight uint32 IDs, eight int16 quantities, then the result byte.
 * Quantity sign-extension in the client forbids values above 32767. Result 0
 * at byte 50 is a processed success, including an empty output.
 * @return false when an entry cannot be serialized
 */
constexpr bool decomposition_success(const std::array<s_decomposition_result, 8>& outputs,
	std::array<uint8, 51>& packet) {
	std::array<uint8, 51> next{};
	put16(next.data(), decompose_result);
	for (size_t i = 0; i < outputs.size(); ++i) {
		const s_decomposition_result& entry = outputs[i];
		if ((entry.id == 0) != (entry.amount == 0) || entry.amount > 32767)
			return false;
		put32(next.data() + 2 + i * 4, entry.id);
		put16(next.data() + 34 + i * 2, static_cast<uint16>(entry.amount));
	}
	packet = next;
	return true;
}

/// 0x0c15 entry: <set id>.L <last claimed slot>.B (a sequence position, not a bitmask).
constexpr std::array<uint8, 5> reward_entry(uint32 id, uint8 last_claimed_slot) {
	std::array<uint8, 5> packet{};
	put32(packet.data(), id);
	packet[4] = last_claimed_slot;
	return packet;
}

/// 0x0c17 <result>.B <category>.W <set id>.L <last claimed slot>.B
constexpr std::array<uint8, 10> reward_state(uint8 result, uint16 category,
	uint32 id, uint8 last_claimed_slot) {
	std::array<uint8, 10> packet{};
	put16(packet.data(), reward_result);
	packet[2] = result;
	put16(packet.data() + 3, category);
	put32(packet.data() + 5, id);
	packet[9] = last_claimed_slot;
	return packet;
}

/// 0x0bf2 <result>.B <category>.W <set id>.L <level>.W <active fragments>.W
/// The 2025-07-16 client clears its equipped pointer (0x7301f0), then stores
/// the level on the set and the fragment count on the manager (0x730b40).
constexpr std::array<uint8, 13> equipped(uint16 category, uint32 id, uint16 level, uint16 fragments) {
	std::array<uint8, 13> packet = operation<13>(equipped_state, 0, category, id);
	put16(packet.data() + 9, level);
	put16(packet.data() + 11, fragments);
	return packet;
}
}

const std::string RuneTabletDatabase::getDefaultLocation() {
	return std::string(db_path) + "/rune_tablet_db.yml";
}

namespace {
/**
 * Bonus types a Rune Set effect may use, and whether they need a parameter.
 * Keep this restricted to reviewed bonus shapes.
 */
bool valid_bonus_shape(int32 bonus, bool parameter) {
	switch (bonus) {
	// Pre-renewal (Ragnarok Zero) set effects
	case SP_STR:
	case SP_AGI:
	case SP_VIT:
	case SP_INT:
	case SP_DEX:
	case SP_LUK:
	case SP_FLEE1:
	case SP_ALL_STATS:
	case SP_ASPD_RATE:
	case SP_ATK_RATE:
	case SP_BASE_ATK:
	case SP_CON:
	case SP_CRATE:
	case SP_CRIT_ATK_RATE:
	case SP_CRITICAL:
	case SP_CRT:
	case SP_DEF1:
	case SP_DELAYRATE:
	case SP_HIT:
	case SP_NON_CRIT_ATK_RATE:
	case SP_LONG_ATK_RATE:
	case SP_EMATK:
	case SP_MATK_RATE:
	case SP_MAXHP:
	case SP_MAXHPRATE:
	case SP_MAXSP:
	case SP_MAXSPRATE:
	case SP_MDEF1:
	case SP_MRES:
	case SP_PATK:
	case SP_POW:
	case SP_RES:
	case SP_SHORT_ATK_RATE:
	case SP_SMATK:
	case SP_SPL:
	case SP_STA:
	case SP_SPRATE:
	case SP_VARCASTRATE:
	case SP_WIS:
		return !parameter;
	case SP_ADDCLASS:
	case SP_ADDELE:
	case SP_ADDRACE:
	case SP_ADDRACE2:
	case SP_ADDSIZE:
	case SP_EXP_ADDRACE:
	case SP_HP_DRAIN_RATE:
	case SP_MAGIC_ADDCLASS:
	case SP_MAGIC_ADDELE:
	case SP_MAGIC_ADDRACE:
	case SP_MAGIC_ADDRACE2:
	case SP_MAGIC_ADDSIZE:
	case SP_MAGIC_ATK_ELE:
	case SP_SP_DRAIN_RATE:
	case SP_MAGIC_SUBDEF_ELE:
	case SP_SUBDEF_ELE:
	case SP_SUBELE:
	case SP_RESEFF:
		return parameter;
	default:
		return false;
	}
}
}

uint64 RuneTabletDatabase::parseBodyNode(const ryml::NodeRef& node) {
	auto record = std::make_shared<s_rune_tablet>();
	std::string type;

	if (!this->asUInt32(node, "Id", record->id) || !record->id
		|| !this->asString(node, "Type", type) || !this->nodesExist(node, { "Materials" }))
		return 0;

	// The top bit of a 0x0bd6 category is the unequip_set_flag.
	if (!this->asUInt16(node, "Category", record->category) || !record->category
		|| record->category >= rune_tablet_wire::unequip_set_flag) {
		this->invalidWarning(node, "Rune Tablet Category must be 1-32767.\n");
		return 0;
	}

	if (type != "Fragment" && type != "Set") {
		this->invalidWarning(node, "Rune Tablet Type must be Fragment or Set.\n");
		return 0;
	}

	record->is_set = type == "Set";

	if (this->exists(record->id)) {
		this->invalidWarning(node, "Duplicate Rune Tablet Id %u.\n", record->id);
		return 0;
	}

	if (!node["Materials"].is_seq())
		return 0;

	for (const ryml::NodeRef& material : node["Materials"]) {
		s_rune_tablet_material cost{};

		if (!this->asUInt32(material, "Item", cost.item) || !cost.item
			|| !this->asUInt32(material, "Amount", cost.amount) || !cost.amount || cost.amount > MAX_AMOUNT)
			return 0;

		record->materials.push_back(cost);
	}

	if (record->materials.empty() || record->materials.size() > MAX_INVENTORY)
		return 0;

	if (this->nodeExists(node, "Upgrades")) {
		if (!record->is_set || !node["Upgrades"].is_seq()) {
			this->invalidWarning(node, "Rune Tablet Upgrades must be a sequence on a set.\n");
			return 0;
		}

		for (const ryml::NodeRef& upgrade : node["Upgrades"]) {
			uint16 level;
			s_rune_tablet_upgrade entry;

			if (!this->asUInt16(upgrade, "Level", level) || level < 1 || level > MAX_RUNE_TABLET_UPGRADE
				|| record->upgrades.count(level)
				|| !this->asUInt32(upgrade, "Chance", entry.chance) || entry.chance > rune_tablet_logic::upgrade_probability_scale
				|| !this->asUInt32(upgrade, "FailureIncrement", entry.failure_increment) || entry.failure_increment > rune_tablet_logic::upgrade_probability_scale
				|| !this->nodeExists(upgrade, "Materials") || !upgrade["Materials"].is_seq()) {
				this->invalidWarning(upgrade, "Invalid Rune Tablet upgrade level or rate.\n");
				return 0;
			}

			std::map<t_itemid, uint32> amounts;

			for (const ryml::NodeRef& material : upgrade["Materials"]) {
				t_itemid id;
				uint32 amount;

				if (!this->asUInt32(material, "Item", id) || !id
					|| !this->asUInt32(material, "Amount", amount) || !amount || amount > MAX_AMOUNT
					|| amounts[id] > MAX_AMOUNT - amount) {
					this->invalidWarning(material, "Invalid Rune Tablet upgrade material.\n");
					return 0;
				}

				amounts[id] += amount;
			}

			if (amounts.empty() || amounts.size() > MAX_INVENTORY)
				return 0;

			for (const auto& amount : amounts)
				entry.materials.push_back({ amount.first, amount.second });

			record->upgrades.emplace(level, std::move(entry));
		}

		if (record->upgrades.size() != MAX_RUNE_TABLET_UPGRADE) {
			this->invalidWarning(node, "Rune Tablet Upgrades must cover all %d target levels.\n", MAX_RUNE_TABLET_UPGRADE);
			return 0;
		}
	}

	if (this->nodeExists(node, "Fragments")) {
		if (!record->is_set || !node["Fragments"].is_seq())
			return 0;

		for (const ryml::NodeRef& fragment : node["Fragments"]) {
			uint32 id;

			if (!this->asUInt32(fragment, "Id", id) || !id
				|| std::find(record->fragments.begin(), record->fragments.end(), id) != record->fragments.end())
				return 0;

			record->fragments.push_back(id);
		}

		if (record->fragments.size() < 2 || record->fragments.size() > 6)
			return 0;
	}

	if (this->nodeExists(node, "Effects")) {
		if (!record->is_set || record->fragments.empty() || !node["Effects"].is_seq())
			return 0;

		for (const ryml::NodeRef& effect : node["Effects"]) {
			s_rune_tablet_effect entry;

			if (!this->asUInt16(effect, "Fragments", entry.fragments)
				|| entry.fragments < 2 || entry.fragments > record->fragments.size()
				|| !this->asUInt16(effect, "MinLevel", entry.min_level) || entry.min_level > MAX_RUNE_TABLET_UPGRADE
				|| !this->asUInt16(effect, "LevelStep", entry.level_step)
				|| entry.level_step < 1 || entry.level_step > MAX_RUNE_TABLET_UPGRADE
				|| !this->asInt32(effect, "Base", entry.base)
				|| !this->asInt32(effect, "PerStep", entry.per_step)
				|| entry.base < -10000 || entry.base > 10000
				|| entry.per_step < -10000 || entry.per_step > 10000) {
				this->invalidWarning(effect, "Invalid Rune Tablet effect.\n");
				return 0;
			}

			if (this->nodeExists(effect, "Special")) {
				std::string special;

				if (this->nodeExists(effect, "Bonus") || this->nodeExists(effect, "Parameter") || !this->asString(effect, "Special", special)) {
					this->invalidWarning(effect, "Invalid Rune Tablet special effect.\n");
					return 0;
				}

				if (special == "TargetedMagicKillRecoveryHP")
					entry.special = e_rune_tablet_special::targeted_magic_kill_recovery_hp;
				else if (special == "TargetedMagicKillRecoverySP")
					entry.special = e_rune_tablet_special::targeted_magic_kill_recovery_sp;
				else if (special == "OnHitSPRegenChance")
					entry.special = e_rune_tablet_special::on_hit_sp_regen_chance;
				else {
					this->invalidWarning(effect, "Unknown Rune Tablet special effect '%s'.\n", special.c_str());
					return 0;
				}

				entry.label = special;
			} else {
				std::string bonus;
				int64 constant;

				if (!this->asString(effect, "Bonus", bonus) || bonus.empty() || bonus[0] != 'b'
					|| !script_get_constant(bonus.c_str(), &constant)
					|| constant < 0 || constant > std::numeric_limits<int32>::max()) {
					this->invalidWarning(effect, "Invalid Rune Tablet effect.\n");
					return 0;
				}

				entry.bonus = static_cast<int32>(constant);
				entry.label = bonus;

				if (this->nodeExists(effect, "Parameter")) {
					std::string parameter;

					if (!this->asString(effect, "Parameter", parameter))
						return 0;

					entry.label += "," + parameter;

					if (!script_get_constant(parameter.c_str(), &constant)) {
						if (!this->asInt32(effect, "Parameter", entry.parameter))
							return 0;
					} else {
						if (constant < 0 || constant > std::numeric_limits<int32>::max())
							return 0;
						entry.parameter = static_cast<int32>(constant);
					}

					entry.has_parameter = true;
				}

				if (!valid_bonus_shape(entry.bonus, entry.has_parameter) || entry.parameter < 0) {
					this->invalidWarning(effect, "Unsupported Rune Tablet bonus or argument count.\n");
					return 0;
				}
			}

			record->effects.push_back(entry);
		}

		if (record->effects.empty() || record->effects.size() > 128)
			return 0;
	}

	if (this->nodeExists(node, "Rewards")) {
		if (!record->is_set || !node["Rewards"].is_seq()) {
			this->invalidWarning(node, "Rune Tablet Rewards must be a sequence on a set.\n");
			return 0;
		}

		for (const ryml::NodeRef& reward : node["Rewards"]) {
			uint16 slot;
			t_itemid item;

			if (!this->asUInt16(reward, "Slot", slot) || slot < 1 || slot > MAX_RUNE_TABLET_REWARD
				|| record->rewards[slot - 1] || !this->asUInt32(reward, "Item", item) || !item) {
				this->invalidWarning(reward, "Invalid Rune Tablet reward slot.\n");
				return 0;
			}

			// Kept even if absent: skipping it would shift the client's claim sequence.
			if (!item_db.exists(item))
				this->invalidWarning(reward, "Rune Tablet reward item %u is absent from the item database; that slot cannot be claimed.\n", item);

			record->rewards[slot - 1] = item;
		}

		if (!record->rewards[0]) {
			this->invalidWarning(node, "Rune Tablet Rewards need the activation slot 1.\n");
			return 0;
		}
	}

	if (this->nodeExists(node, "EffectFree")) {
		if (!record->is_set || !record->effects.empty() || record->fragments.empty()
			|| !this->asBool(node, "EffectFree", record->effect_free)) {
			this->invalidWarning(node, "Rune Tablet EffectFree requires a set without Effects.\n");
			return 0;
		}
	}

	this->put(record->id, record);

	return 1;
}

void RuneTabletDatabase::loadingFinished() {
	this->categories.clear();
	this->sets.clear();

	for (const auto& entry : *this) {
		if (!entry.second->is_set)
			continue;

		this->sets.push_back(entry.second);

		if (std::find(this->categories.begin(), this->categories.end(), entry.second->category) == this->categories.end())
			this->categories.push_back(entry.second->category);
	}

	std::sort(this->categories.begin(), this->categories.end());
	std::sort(this->sets.begin(), this->sets.end(),
		[](const std::shared_ptr<s_rune_tablet>& a, const std::shared_ptr<s_rune_tablet>& b) {
			return a->category != b->category ? a->category < b->category : a->id < b->id;
		});

	TypesafeYamlDatabase::loadingFinished();
}

void RuneTabletDatabase::clear() {
	TypesafeYamlDatabase::clear();

	this->categories.clear();
	this->sets.clear();
}

const std::vector<uint16>& RuneTabletDatabase::getCategories() const {
	return this->categories;
}

const std::vector<std::shared_ptr<s_rune_tablet>>& RuneTabletDatabase::getSets() const {
	return this->sets;
}

const std::string RuneTabletDecompositionDatabase::getDefaultLocation() {
	return std::string(db_path) + "/rune_tablet_decomposition.yml";
}

/**
 * Generated from the client's System/Rune/itemDecom.lub. The amount
 * distribution is not verified, so delivery is gated by
 * rune_tablet_decomposition_uniform except for the fixed sealed-stone row.
 */
uint64 RuneTabletDecompositionDatabase::parseBodyNode(const ryml::NodeRef& node) {
	auto record = std::make_shared<s_rune_tablet_decomposition>();

	if (!this->asUInt32(node, "Item", record->item) || !record->item
		|| !this->nodesExist(node, { "Modes" }) || !node["Modes"].is_seq())
		return 0;

	if (this->exists(record->item)) {
		this->invalidWarning(node, "Duplicate Rune Tablet decomposition item %u.\n", record->item);
		return 0;
	}

	if (!item_db.exists(record->item)) {
		this->invalidWarning(node, "Rune Tablet decomposition item %u is absent from the item database, skipping.\n", record->item);
		return 0;
	}

	bool seen[2] = {};

	for (const ryml::NodeRef& mode_node : node["Modes"]) {
		uint16 mode;

		if (!this->asUInt16(mode_node, "Mode", mode) || mode < 1 || mode > 2 || seen[mode - 1]) {
			this->invalidWarning(mode_node, "Rune Tablet decomposition Mode must be 1 or 2, once each.\n");
			return 0;
		}

		seen[mode - 1] = true;

		s_rune_tablet_decomposition_mode& entry = record->modes[mode - 1];

		if (!this->asUInt16(mode_node, "Consume", entry.consume) || !entry.consume || entry.consume > MAX_AMOUNT
			|| !this->nodeExists(mode_node, "Outputs") || !mode_node["Outputs"].is_seq()) {
			this->invalidWarning(mode_node, "Invalid Rune Tablet decomposition batch.\n");
			return 0;
		}

		for (const ryml::NodeRef& output_node : mode_node["Outputs"]) {
			s_rune_tablet_decomposition_output output;

			// Amounts travel as int16 in 0x0bd9 and must fit a stack. Minimum 0
			// (Ragnarok Zero's 0-1 Complete Rune roll) may yield nothing.
			if (!this->asUInt32(output_node, "Item", output.item) || !output.item
				|| !this->asUInt16(output_node, "Minimum", output.minimum)
				|| !this->asUInt16(output_node, "Maximum", output.maximum) || !output.maximum || output.maximum < output.minimum
				|| output.maximum > std::min<uint16>(MAX_AMOUNT, 32767)
				|| !this->asUInt32(output_node, "Chance", output.chance) || !output.chance
				|| output.chance > rune_tablet_logic::upgrade_probability_scale) {
				this->invalidWarning(output_node, "Invalid Rune Tablet decomposition output.\n");
				return 0;
			}

			if (!item_db.exists(output.item)) {
				this->invalidWarning(output_node, "Rune Tablet decomposition output %u is absent from the item database.\n", output.item);
				return 0;
			}

			if (std::any_of(entry.outputs.begin(), entry.outputs.end(),
				[&](const s_rune_tablet_decomposition_output& existing) { return existing.item == output.item; })) {
				this->invalidWarning(output_node, "Duplicate Rune Tablet decomposition output item %u.\n", output.item);
				return 0;
			}

			entry.outputs.push_back(output);
		}

		// 0x0bd9 has exactly eight result positions.
		if (entry.outputs.empty() || entry.outputs.size() > 8) {
			this->invalidWarning(mode_node, "Rune Tablet decomposition needs one to eight outputs.\n");
			return 0;
		}
	}

	if (!seen[0] || !seen[1]) {
		this->invalidWarning(node, "Rune Tablet decomposition item %u needs both modes.\n", record->item);
		return 0;
	}

	this->put(record->item, record);

	return 1;
}

namespace {
/// Character registry key of a fragment or set's activation progress.
int64 progress_key(const s_rune_tablet& rune) {
	return reference_uid(add_str(rune.is_set ? "RuneTabletSet" : "RuneTabletFragment"), rune.id);
}

/// Character registry key of a Rune Set's equipped flag (value 1).
int64 equipped_key(const s_rune_tablet& set) {
	return reference_uid(add_str("RuneTabletEquipSet"), set.id);
}

/// Legacy per-category storage (RuneTabletEquipped[category] = set ID),
/// converted by prune_equipped.
int64 legacy_equipped_key(uint16 category) {
	return reference_uid(add_str("RuneTabletEquipped"), category);
}

/// Character registry key of a set's last claimed collection reward slot (0..7).
int64 reward_key(const s_rune_tablet& rune) {
	return reference_uid(add_str("RuneTabletReward"), rune.id);
}

bool owned(const map_session_data& sd, const s_rune_tablet& rune) {
	return rune_tablet_logic::level(pc_readregistry(&sd, progress_key(rune))) >= 0;
}

int32 upgrade_level(const map_session_data& sd, const s_rune_tablet& rune) {
	return rune_tablet_logic::level(pc_readregistry(&sd, progress_key(rune)));
}

bool has_rewards(const s_rune_tablet& rune) {
	return rune_tablet_logic::next_reward_slot(rune.rewards, 0) != 0;
}

bool valid_fragments(const s_rune_tablet& set) {
	if (set.fragments.size() < 2 || set.fragments.size() > 6)
		return false;

	for (uint32 id : set.fragments) {
		std::shared_ptr<s_rune_tablet> fragment = rune_tablet_db.find(id);

		if (fragment == nullptr || fragment->is_set || fragment->category != set.category)
			return false;
	}

	return true;
}

/// Reviewed effects, or an official effect-free set; pending sets stay blocked.
bool equippable(const s_rune_tablet& set) {
	return (!set.effects.empty() || set.effect_free) && valid_fragments(set);
}

/// Access gate: rune_tablet_min_group_level (99 = GM only).
bool access_allowed(const map_session_data& sd) {
	return pc_get_group_level(&sd) >= battle_config.rune_tablet_min_group_level;
}

uint16 active_fragments(const map_session_data& sd, const s_rune_tablet& set) {
	uint16 count = 0;

	for (uint32 id : set.fragments) {
		std::shared_ptr<s_rune_tablet> fragment = rune_tablet_db.find(id);

		if (fragment != nullptr && !fragment->is_set && fragment->category == set.category && owned(sd, *fragment))
			++count;
	}

	return count;
}

/// Configured number of equipped Rune Sets; 1 is the official single slot.
size_t max_equipped() {
	return static_cast<size_t>(std::clamp(battle_config.rune_tablet_max_equipped, 1, MAX_RUNE_TABLET_EQUIPPED));
}

/// rune_tablet_equip_per_category: at most one equipped set per category.
bool one_per_category() {
	return battle_config.rune_tablet_equip_per_category != 0;
}

bool is_equipped(const map_session_data& sd, const s_rune_tablet& set) {
	return pc_readregistry(&sd, equipped_key(set)) != 0;
}

/// Equipped and still valid: activated and reviewed. Stale entries do not count.
bool equipped_valid(const map_session_data& sd, const s_rune_tablet& set) {
	return is_equipped(sd, set) && owned(sd, set) && equippable(set);
}

/**
 * Visit the equipped Rune Sets in ascending (category, ID) order without
 * allocating (status and hit hooks). Only the first rune_tablet_max_equipped
 * valid sets count, with rune_tablet_equip_per_category only the first of
 * each category; rune_tablet_sync_equipped prunes the rest.
 * @param visit: Called per set; return false to stop
 */
template<typename F>
void for_each_equipped(const map_session_data& sd, F&& visit) {
	if (!sd.vars_ok || !access_allowed(sd))
		return;

	const size_t limit = max_equipped();
	const bool per_category = one_per_category();
	size_t count = 0;
	uint16 last_category = 0;

	for (const std::shared_ptr<s_rune_tablet>& set : rune_tablet_db.getSets()) {
		if (count >= limit)
			break;

		if (per_category && count > 0 && set->category == last_category)
			continue;

		if (!equipped_valid(sd, *set))
			continue;

		++count;
		last_category = set->category;

		if (!visit(set))
			break;
	}
}

std::vector<std::shared_ptr<s_rune_tablet>> equipped_sets(const map_session_data& sd) {
	std::vector<std::shared_ptr<s_rune_tablet>> result;

	for_each_equipped(sd, [&result](const std::shared_ptr<s_rune_tablet>& set) {
		result.push_back(set);
		return true;
	});

	return result;
}

/**
 * Normalize the stored equipped sets: convert the legacy per-category entries,
 * then clear valid sets that no longer fit (rune_tablet_max_equipped lowered,
 * rune_tablet_equip_per_category enabled), keeping the lowest (category, ID).
 * @return true when something changed
 */
bool prune_equipped(map_session_data& sd) {
	bool changed = false;

	for (uint16 category : rune_tablet_db.getCategories()) {
		const int64 legacy = pc_readregistry(&sd, legacy_equipped_key(category));

		if (legacy == 0)
			continue;

		std::shared_ptr<s_rune_tablet> set = legacy > 0 && legacy <= std::numeric_limits<uint32>::max()
			? rune_tablet_db.find(static_cast<uint32>(legacy)) : nullptr;

		if (set != nullptr && set->is_set && set->category == category)
			pc_setregistry(&sd, equipped_key(*set), 1);

		pc_setregistry(&sd, legacy_equipped_key(category), 0);
		changed = true;
	}

	const size_t limit = max_equipped();
	const bool per_category = one_per_category();
	size_t kept = 0;
	uint16 last_category = 0;

	for (const std::shared_ptr<s_rune_tablet>& set : rune_tablet_db.getSets()) {
		if (!equipped_valid(sd, *set))
			continue;

		if (kept >= limit || (per_category && kept > 0 && set->category == last_category)) {
			pc_setregistry(&sd, equipped_key(*set), 0);
			changed = true;
			continue;
		}

		++kept;
		last_category = set->category;
	}

	return changed;
}

bool eligible(const item& it) {
	// Only ordinary, permanent inventory materials. Do not silently dismantle
	// equipped, favorite, rented, forged, carded, refined or enchanted items.
	if (it.amount <= 0 || it.equip || it.equipSwitch || it.favorite
		|| it.expire_time || it.refine || it.enchantgrade || it.attribute)
		return false;

	for (const auto card : it.card)
		if (card)
			return false;

	for (const auto& option : it.option)
		if (option.id || option.value || option.param)
			return false;

	return true;
}

std::vector<rune_tablet_logic::s_inventory_slot> inventory_view(const map_session_data& sd) {
	std::vector<rune_tablet_logic::s_inventory_slot> inventory;

	inventory.reserve(MAX_INVENTORY);

	for (int32 i = 0; i < MAX_INVENTORY; ++i) {
		const item& it = sd.inventory.u.items_inventory[i];

		inventory.push_back({ it.nameid, it.amount > 0 ? static_cast<uint32>(it.amount) : 0,
			sd.inventory_data[i] != nullptr && eligible(it) });
	}

	return inventory;
}

/**
 * Worst-case check before anything is consumed: every output at its maximum
 * must fit the inventory and weight limit (policy: reject, never drop).
 * @return Error message, or nullptr when the result fits
 */
const char* decomposition_capacity_error(map_session_data& sd, const s_rune_tablet_decomposition_mode& mode) {
	uint64 weight = 0;
	int32 new_slots = 0;

	for (const s_rune_tablet_decomposition_output& output : mode.outputs) {
		std::shared_ptr<item_data> data = item_db.find(output.item);

		if (data == nullptr)
			return "Rune Tablet: a resulting item is missing from the item database.";

		// Match the plain, identified, unbound item that delivery will create.
		// pc_checkadditem only compares nameid and can reuse an incompatible
		// bound/rental/carded stack in its estimate; pc_additem does not.
		const bool stackable = itemdb_isstackable2(data.get());
		const uint32 limit = data->stack.inventory ? std::min<uint32>(MAX_AMOUNT, data->stack.amount) : MAX_AMOUNT;

		if (output.maximum > limit)
			return "Rune Tablet: you would exceed the amount limit of a resulting item.";

		bool existing_stack = false;

		if (stackable && !data->flag.guid) {
			for (int32 i = 0; i < MAX_INVENTORY; ++i) {
				const item& current = sd.inventory.u.items_inventory[i];

				if (current.nameid != output.item || current.bound || current.expire_time || current.unique_id
					|| std::any_of(std::begin(current.card), std::end(current.card), [](const auto card) { return card != 0; }))
					continue;

				// pc_additem stops at the first compatible stack, including
				// when it is full or outside the usable inventory slots.
				if (i >= sd.status.inventory_slots || current.amount < 0
					|| uint64(current.amount) + output.maximum > limit)
					return "Rune Tablet: you would exceed the amount limit of a resulting item.";

				existing_stack = true;
				break;
			}
		}

		if (!existing_stack)
			new_slots += stackable ? 1 : output.maximum;

		weight += uint64(data->weight) * output.maximum;
	}

	if (new_slots > pc_inventoryblank(&sd))
		return "Rune Tablet: not enough inventory space for the maximum result.";
	if (sd.weight + weight > uint64(sd.max_weight))
		return "Rune Tablet: the weight of the maximum result would exceed your limit.";

	return nullptr;
}

/// All delivery targets are plain stackable runes, distinct from the source.
/// Keep this restricted until other item kinds and their hooks are reviewed.
int32 decomposition_target(map_session_data& sd, t_itemid id) {
	for (int32 i = 0; i < MAX_INVENTORY; ++i) {
		const item& entry = sd.inventory.u.items_inventory[i];

		if (entry.nameid == id && !entry.bound && !entry.expire_time && !entry.unique_id
			&& std::all_of(std::begin(entry.card), std::end(entry.card), [](const auto card) { return card == 0; }))
			return i;
	}

	return pc_search_inventory(&sd, 0);
}

/**
 * Roll, consume and deliver one decomposition. Outputs are delivered as one
 * batch: a failed delivery restores the inventory exactly.
 * @return Error message, or nullptr on success (response holds the 0x0bd9 packet)
 */
const char* deliver_decomposition(map_session_data& sd,
	const rune_tablet_wire::s_decomposition_request& request, const s_rune_tablet_decomposition_mode& mode,
	std::array<uint8, 51>& response) {
	const bool fixed = request.item_id == 1001595 && mode.outputs.size() == 1
		&& mode.consume == (request.mode == 1 ? 1 : 30)
		&& mode.outputs[0].item == 1001283 && mode.outputs[0].chance == 100000
		&& mode.outputs[0].minimum == mode.consume && mode.outputs[0].maximum == mode.consume;

	if (!fixed && !battle_config.rune_tablet_decomposition_uniform)
		return "Rune Tablet: amount distribution is unverified; the local policy is disabled.";

	const item original = sd.inventory.u.items_inventory[request.server_index];

	if (original.bound || original.unique_id || request.server_index >= sd.status.inventory_slots)
		return "Rune Tablet: the item is bound, unique, or outside the available slots.";

	for (const s_rune_tablet_decomposition_output& output : mode.outputs) {
		std::shared_ptr<item_data> data = item_db.find(output.item);

		if ((output.item != 1001282 && output.item != 1001283) || output.item == original.nameid
			|| data == nullptr || !itemdb_isstackable2(data.get()) || data->flag.guid || data->flag.autoequip)
			return "Rune Tablet: result configuration is incompatible with this conversion.";
	}

	// Optional LOCAL policy: independently roll each output's chance, then
	// sample its quantity uniformly in the inclusive interval. No rate changes.
	std::array<rune_tablet_wire::s_decomposition_result, 8> results{};

	for (size_t i = 0; i < mode.outputs.size(); ++i) {
		const s_rune_tablet_decomposition_output& output = mode.outputs[i];

		if (output.chance == 100000 || rnd_value<uint32>(0, 99999) < output.chance) {
			const uint32 quantity = output.minimum == output.maximum ? output.minimum
				: rnd_value<uint32>(output.minimum, output.maximum);

			// A rolled amount of 0 leaves the result position empty.
			if (quantity)
				results[i] = { output.item, quantity };
		}
	}

	// Prepare the native response before charging anything.
	if (!rune_tablet_wire::decomposition_success(results, response))
		return "Rune Tablet: could not prepare the decomposition response.";

	struct s_granted_slot {
		int32 index;
		item before;
		item_data* data;
		uint32 amount;
	};

	std::vector<s_granted_slot> granted;

	granted.reserve(results.size()); // No vector allocation after the source debit.

	item_data* original_data = sd.inventory_data[request.server_index];
	const uint32 original_weight = sd.weight;
	const int16 original_last_added = sd.last_addeditem_index;

	if (pc_delitem(&sd, request.server_index, mode.consume, 0, 0, LOG_TYPE_OTHER, false))
		return "Rune Tablet: could not consume the material.";

	for (const rune_tablet_wire::s_decomposition_result& result : results) {
		if (!result.amount)
			continue;

		const int32 index = decomposition_target(sd, result.id);
		bool failed = index < 0 || index >= sd.status.inventory_slots;
		s_granted_slot snapshot{};

		if (!failed) {
			snapshot = { index, sd.inventory.u.items_inventory[index], sd.inventory_data[index], result.amount };

			item next{};

			next.nameid = result.id;
			next.identify = 1;
			// For these plain, non-GUID, non-autoequip, permanent outputs,
			// pc_additem error returns precede inventory mutation.
			failed = pc_additem(&sd, &next, result.amount, LOG_TYPE_OTHER, false, false) != ADDITEM_SUCCESS;
		}

		if (!failed) {
			granted.push_back(snapshot);
			continue;
		}

		// Undo already delivered outputs in reverse order, including outputs
		// that reused the emptied source slot. Restore that source LAST.
		for (auto it = granted.rbegin(); it != granted.rend(); ++it) {
			log_pick_pc(&sd, LOG_TYPE_OTHER, -static_cast<int32>(it->amount), &sd.inventory.u.items_inventory[it->index]);
			sd.inventory.u.items_inventory[it->index] = it->before;
			sd.inventory_data[it->index] = it->data;
			clif_delitem(sd, it->index, it->amount, 0);
		}

		sd.inventory.u.items_inventory[request.server_index] = original;
		sd.inventory_data[request.server_index] = original_data;
		sd.weight = original_weight;
		sd.last_addeditem_index = original_last_added;
		log_pick_pc(&sd, LOG_TYPE_OTHER, mode.consume, &sd.inventory.u.items_inventory[request.server_index]);
		clif_additem(&sd, request.server_index, mode.consume, 0);
		clif_updatestatus(sd, SP_WEIGHT);
		pc_show_questinfo(&sd);
		ShowError("Rune Tablet: decomposition delivery rolled back for character %d, source %u.\n",
			sd.status.char_id, request.item_id);
		return "Rune Tablet: delivery failed; your inventory was restored.";
	}

	// No achievement/quest condition scripts ran while the batch was partial.
	for (const rune_tablet_wire::s_decomposition_result& result : results)
		if (result.amount)
			achievement_update_objective(&sd, AG_GET_ITEM, 1, itemdb_search(result.id)->value_sell);

	pc_show_questinfo(&sd); // Also refresh when all optional outputs missed.
	ShowInfo("Rune Tablet: character %d decomposed %u x %u; policy %s.\n",
		sd.status.char_id, request.item_id, static_cast<uint32>(mode.consume), fixed ? "fixed" : "local-uniform");
	return nullptr;
}

/**
 * Activate a fragment or set.
 * @return Native result: 0 success, 1 already active, 3 MSI_RUNESYSTEM_NOT_ENOUGH_ITEM,
 *	6 MSI_RUNESYSTEM_INVALID_ID, 11 MSI_RUNESYSTEM_DB_LOAD_NOT_YET
 */
uint8 activate(map_session_data& sd, uint16 category, uint32 id, bool is_set) {
	std::shared_ptr<s_rune_tablet> rune = rune_tablet_db.find(id);

	if (rune == nullptr || rune->category != category || rune->is_set != is_set)
		return 6;
	if (owned(sd, *rune))
		return 1; // Already active: repeated packets cannot charge twice.

	for (const s_rune_tablet_material& cost : rune->materials) {
		if (!item_db.exists(cost.item)) {
			ShowWarning("Rune Tablet: item %u for rune %u is absent from the item database.\n", cost.item, id);
			return 11;
		}
	}

	std::optional<std::vector<rune_tablet_logic::s_debit>> debits = rune_tablet_logic::plan(rune->materials, inventory_view(sd));

	if (!debits)
		return 3;

	// No scripts, yields or equipment removals between validation and debit.
	// pc_delitem's failure conditions have all been checked by the plan above.
	for (const rune_tablet_logic::s_debit& debit : *debits) {
		if (pc_delitem(&sd, static_cast<int32>(debit.index), static_cast<int32>(debit.amount), 0, 0, LOG_TYPE_OTHER, false)) {
			ShowError("Rune Tablet: validated debit failed for character %d, rune %u.\n", sd.status.char_id, id);
			return 11;
		}
	}

	// Character registry persistence handles logout/autosave. Value 1 denotes
	// activation at upgrade level zero.
	if (!pc_setregistry(&sd, progress_key(*rune), 1)) {
		ShowError("Rune Tablet: registry write failed for character %d, rune %u.\n", sd.status.char_id, id);
		return 11;
	}

	status_calc_pc(&sd, SCO_NONE);
	pc_show_questinfo(&sd);
	return 0;
}

/**
 * Attempt one upgrade of an activated set.
 * Processed failed rolls are successful protocol operations with a changed failure count.
 * @param next: Receives the new progress value
 * @return Error message for an unprocessed attempt, or nullptr
 */
const char* upgrade(map_session_data& sd, uint16 category, uint32 id, int64& next) {
	std::shared_ptr<s_rune_tablet> set = rune_tablet_db.find(id);

	if (set == nullptr || !set->is_set || set->category != category || !owned(sd, *set))
		return "Rune Tablet: invalid or inactive set.";

	const t_tick now = gettick();

	if (now < sd.rune_tablet_upgrade_after)
		return "Rune Tablet: wait one second before another upgrade attempt.";

	const int64 key = progress_key(*set);
	const int64 before = pc_readregistry(&sd, key);
	const int32 current = rune_tablet_logic::level(before);

	if (current >= MAX_RUNE_TABLET_UPGRADE)
		return "Rune Tablet: this set has already reached the maximum level (+15).";

	auto rule = set->upgrades.find(static_cast<uint16>(current + 1));

	if (rule == set->upgrades.end())
		return "Rune Tablet: upgrade data for this level is missing.";
	if (rune_tablet_logic::failures(before) == 65535)
		return "Rune Tablet: failure counter exhausted; check your progress before continuing.";
	// No materials may be consumed for a table that can never succeed.
	if (!rule->second.chance && !rule->second.failure_increment)
		return "Rune Tablet: this set has no upgrade chance available.";

	for (const s_rune_tablet_material& cost : rule->second.materials)
		if (!item_db.exists(cost.item))
			return "Rune Tablet: an upgrade material is missing from the item database.";

	const std::optional<std::vector<rune_tablet_logic::s_debit>> debits = rune_tablet_logic::plan(rule->second.materials, inventory_view(sd));

	if (!debits)
		return "Rune Tablet: you do not have the required upgrade materials.";

	next = rune_tablet_logic::upgrade_outcome(before, rule->second.chance,
		rule->second.failure_increment, rnd_value<uint32>(0, rune_tablet_logic::upgrade_probability_scale - 1));

	if (!next)
		return "Rune Tablet: invalid upgrade progress or exhausted counter.";

	// This session guard suppresses rapid duplicate packets; the native request
	// has no transaction ID, so later requests necessarily represent new attempts.
	sd.rune_tablet_upgrade_after = now + 1000;

	// Commit both progress fields together before the debit. pc_setregistry only
	// fails when registries are unavailable; no scripts/yields occur here.
	if (!pc_setregistry(&sd, key, next))
		return "Rune Tablet: could not save the upgrade progress.";

	struct s_removed_material {
		item original;
		int32 amount;
	};

	std::vector<s_removed_material> removed;

	removed.reserve(debits->size());

	for (const rune_tablet_logic::s_debit& debit : *debits) {
		item original = sd.inventory.u.items_inventory[debit.index];

		if (pc_delitem(&sd, static_cast<int32>(debit.index), static_cast<int32>(debit.amount), 0, 0, LOG_TYPE_OTHER, false)) {
			// Defensive rollback: the debit planner already checks all current
			// pc_delitem failure conditions; reserved inventory space is freed.
			pc_setregistry(&sd, key, before);

			for (s_removed_material& entry : removed)
				if (pc_additem(&sd, &entry.original, entry.amount, LOG_TYPE_OTHER) != ADDITEM_SUCCESS)
					ShowError("Rune Tablet: rollback item %u failed for character %d.\n", entry.original.nameid, sd.status.char_id);

			ShowError("Rune Tablet: validated upgrade debit failed for character %d, set %u.\n", sd.status.char_id, id);
			pc_show_questinfo(&sd);
			return "Rune Tablet: internal error while consuming materials; check the server log.";
		}

		removed.push_back({ original, static_cast<int32>(debit.amount) });
	}

	if (rune_tablet_logic::level(next) != current && is_equipped(sd, *set))
		status_calc_pc(&sd, SCO_NONE);

	pc_show_questinfo(&sd);
	ShowInfo("Rune Tablet: character %d upgraded set %u: level %d -> %d, failures %d -> %d.\n",
		sd.status.char_id, id, current, rune_tablet_logic::level(next),
		rune_tablet_logic::failures(before), rune_tablet_logic::failures(next));
	return nullptr;
}

template<size_t N>
void send(map_session_data& sd, const std::array<uint8, N>& packet) {
	static_assert(N <= 32767, "Packet exceeds protocol length limit");
	clif_send(packet.data(), static_cast<int32>(N), &sd, SELF);
}

/**
 * 0x0bcc/0x0bcd: activated fragments or sets of a category.
 * Fragments are 4 bytes per entry, sets 8 (ID, level, failure count).
 */
void send_progress(map_session_data& sd, uint16 category, bool is_set) {
	using namespace rune_tablet_wire;

	const std::array<uint8, 9> header = empty_list(is_set ? set_list : fragment_list, category);
	std::vector<uint8> packet(header.begin(), header.end());
	uint16 count = 0;

	for (const auto& entry : rune_tablet_db) {
		const s_rune_tablet& rune = *entry.second;

		if (rune.category != category || rune.is_set != is_set || !owned(sd, rune))
			continue;

		const size_t offset = packet.size();

		if (offset + (is_set ? 8 : 4) > 32767) {
			ShowError("Rune Tablet: progress list exceeds packet limit.\n");
			return;
		}

		packet.resize(offset + (is_set ? 8 : 4), 0);
		put32(packet.data() + offset, rune.id);

		if (is_set) {
			const int64 progress = pc_readregistry(&sd, progress_key(rune));

			put16(packet.data() + offset + 4, static_cast<uint16>(rune_tablet_logic::level(progress)));
			put16(packet.data() + offset + 6, static_cast<uint16>(rune_tablet_logic::failures(progress)));
		}

		++count;
	}

	put16(packet.data() + 2, static_cast<uint16>(packet.size()));
	put16(packet.data() + 7, count);
	clif_send(packet.data(), static_cast<int32>(packet.size()), &sd, SELF);
}

#if PACKETVER_MAIN_NUM >= 20241016
/// 0x0c15: every set of the category that has collection rewards, with the
/// last claimed slot (list header like 0x0bcc, 5 bytes per entry).
void send_rewards(map_session_data& sd, uint16 category) {
	using namespace rune_tablet_wire;

	const std::array<uint8, 9> header = empty_list(reward_list, category);
	std::vector<uint8> packet(header.begin(), header.end());
	uint16 count = 0;

	for (const auto& entry : rune_tablet_db) {
		const s_rune_tablet& rune = *entry.second;

		if (rune.category != category || !rune.is_set || !has_rewards(rune))
			continue;

		const int64 claimed = pc_readregistry(&sd, reward_key(rune));

		if (claimed < 0 || claimed > MAX_RUNE_TABLET_REWARD || packet.size() + 5 > 32767)
			continue;

		const std::array<uint8, 5> reward = reward_entry(rune.id, static_cast<uint8>(claimed));

		packet.insert(packet.end(), reward.begin(), reward.end());
		++count;
	}

	put16(packet.data() + 2, static_cast<uint16>(packet.size()));
	put16(packet.data() + 7, count);
	clif_send(packet.data(), static_cast<int32>(packet.size()), &sd, SELF);
}

/**
 * 0x0c16: claim one reward slot. One unit per slot; reward boxes carry their
 * own contents. Nothing is given when the item does not fit (official guide).
 * @param claimed_out: Receives the last claimed slot to report
 * @return rune_tablet_logic::e_reward_result
 */
uint8 grant_reward(map_session_data& sd, uint16 category, uint32 id, uint8 slot, uint8& claimed_out) {
	using namespace rune_tablet_logic;

	claimed_out = 0;

	std::shared_ptr<s_rune_tablet> set = rune_tablet_db.find(id);

	if (set == nullptr || !set->is_set || set->category != category || !has_rewards(*set))
		return reward_no_table;

	const int64 claimed = pc_readregistry(&sd, reward_key(*set));

	if (claimed < 0 || claimed > MAX_RUNE_TABLET_REWARD)
		return reward_mismatch;

	claimed_out = static_cast<uint8>(claimed);

	const uint8 check = reward_check(set->rewards, static_cast<int32>(claimed), slot,
		owned(sd, *set), active_fragments(sd, *set));

	if (check != reward_ok)
		return check;

	item reward{};

	reward.nameid = set->rewards[slot - 1];
	reward.identify = 1;

	if (!item_db.exists(reward.nameid))
		return reward_no_table;

	// Mark the slot first so a repeated packet cannot claim it twice; restore on failure.
	if (!pc_setregistry(&sd, reward_key(*set), slot))
		return reward_mismatch;

	switch (pc_additem(&sd, &reward, 1, LOG_TYPE_OTHER)) {
		case ADDITEM_SUCCESS:
			claimed_out = slot;
			ShowInfo("Rune Tablet: character %d claimed reward slot %u of set %u (item %u).\n",
				sd.status.char_id, slot, id, reward.nameid);
			return reward_ok;
		case ADDITEM_OVERWEIGHT:
			pc_setregistry(&sd, reward_key(*set), claimed);
			return reward_overweight;
		case ADDITEM_OVERITEM:
			pc_setregistry(&sd, reward_key(*set), claimed);
			return reward_no_space;
		case ADDITEM_OVERAMOUNT:
		case ADDITEM_STACKLIMIT:
			pc_setregistry(&sd, reward_key(*set), claimed);
			return reward_over_count;
		default:
			pc_setregistry(&sd, reward_key(*set), claimed);
			return reward_mismatch;
	}
}
#endif

/**
 * Unequip Rune Sets.
 * @param category: Category to clear, 0 for every category
 * @param id: Rune Set to clear, 0 for every set of the category
 * @param cleared: Receives each category that lost a set
 * @return false when a registry write failed (the window is closed with a message)
 */
bool clear_equipped(map_session_data& sd, uint16 category, uint32 id, std::vector<uint16>& cleared) {
	for (const std::shared_ptr<s_rune_tablet>& set : rune_tablet_db.getSets()) {
		if ((category != 0 && set->category != category) || (id != 0 && set->id != id) || !is_equipped(sd, *set))
			continue;

		if (!pc_setregistry(&sd, equipped_key(*set), 0)) {
			rune_tablet_close(sd);
			clif_displaymessage(sd.fd, "Rune Tablet: could not save the unequip.");
			return false;
		}

		if (std::find(cleared.begin(), cleared.end(), set->category) == cleared.end())
			cleared.push_back(set->category);
	}

	if (!cleared.empty()) {
		status_change_end(&sd, SC_RUNE_TABLET_REGEN);
		status_calc_pc(&sd, SCO_NONE);
	}

	return true;
}

/**
 * Equip a Rune Set.
 * With rune_tablet_max_equipped 1 the set replaces the equipped one. Otherwise
 * it takes a free slot; with rune_tablet_equip_per_category it replaces the
 * set of its own category instead.
 * @return false when the request was rejected (the window is closed with a message)
 */
bool equip_set(map_session_data& sd, const s_rune_tablet& set) {
	using namespace rune_tablet_wire;

	if (is_equipped(sd, set))
		return true;

	const size_t limit = max_equipped();
	const bool per_category = one_per_category();
	std::vector<std::shared_ptr<s_rune_tablet>> displaced;
	size_t kept = 0;

	for (const std::shared_ptr<s_rune_tablet>& other : rune_tablet_db.getSets()) {
		if (!is_equipped(sd, *other))
			continue;

		// The single slot also drops stale entries, so a set that becomes valid
		// again cannot outrank the new one in pruning.
		if (limit == 1 || (per_category && other->category == set.category))
			displaced.push_back(other);
		else if (equipped_valid(sd, *other))
			++kept;
	}

	if (kept >= limit) {
		char message[CHAT_SIZE_MAX];

		safesnprintf(message, sizeof(message), "Rune Tablet: at most %d Rune Sets can be equipped. Unequip one first.", static_cast<int32>(limit));
		rune_tablet_close(sd);
		clif_displaymessage(sd.fd, message);
		return false;
	}

	if (!pc_setregistry(&sd, equipped_key(set), 1)) {
		rune_tablet_close(sd);
		clif_displaymessage(sd.fd, "Rune Tablet: could not save the equipped set.");
		return false;
	}

	std::vector<uint16> cleared;

	for (const std::shared_ptr<s_rune_tablet>& other : displaced) {
		pc_setregistry(&sd, equipped_key(*other), 0);

		if (other->category != set.category && std::find(cleared.begin(), cleared.end(), other->category) == cleared.end())
			cleared.push_back(other->category);
	}

	// The single slot clears displaced categories before the success: the
	// client keeps one equipped pointer. Multiple sets resync afterwards.
	if (limit == 1)
		for (uint16 category : cleared)
			send(sd, operation<9>(equip_result, 0, category, 0));

	status_change_end(&sd, SC_RUNE_TABLET_REGEN);
	status_calc_pc(&sd, SCO_NONE);
	return true;
}
}

void do_init_rune_tablet() {
	rune_tablet_db.load();
	rune_tablet_decomposition_db.load();
}

void do_final_rune_tablet() {
	rune_tablet_db.clear();
	rune_tablet_decomposition_db.clear();
}

void rune_tablet_reload() {
	rune_tablet_db.reload();
	rune_tablet_decomposition_db.reload();

	s_mapiterator* iter = mapit_getallusers();

	for (map_session_data* sd = (map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (map_session_data*)mapit_next(iter)) {
		rune_tablet_close(*sd);
		status_calc_pc(sd, SCO_NONE);
	}

	mapit_free(iter);
}

void rune_tablet_apply_bonuses(map_session_data& sd) {
	// No scripts or recursive status calculations: re-equipping cannot stack effects.
	const e_lr_flag previous = sd.state.lr_flag;

	sd.state.lr_flag = LR_FLAG_NONE;

	// Effects of all equipped sets add up.
	for_each_equipped(sd, [&sd](const std::shared_ptr<s_rune_tablet>& set) {
		const uint16 count = active_fragments(sd, *set);
		const int32 level = upgrade_level(sd, *set);

		for (const s_rune_tablet_effect& effect : set->effects) {
			if (effect.special != e_rune_tablet_special::none)
				continue; // Applied by dedicated hooks, not the stat-bonus pool.

			const int32 value = rune_tablet_logic::effect_value(effect, count, level);

			if (!value)
				continue;

			if (effect.has_parameter)
				pc_bonus2(&sd, effect.bonus, effect.parameter, value);
			else
				pc_bonus(&sd, effect.bonus, value);
		}

		return true;
	});

	sd.state.lr_flag = previous;
}

void rune_tablet_show_effects(map_session_data& sd, int32 fd) {
	char output[CHAT_SIZE_MAX];

	if (!access_allowed(sd)) {
		safesnprintf(output, sizeof(output), "%s: Rune Tablet effects need group level %d (rune_tablet_min_group_level).", sd.status.name, battle_config.rune_tablet_min_group_level);
		clif_displaymessage(fd, output);
		return;
	}

	std::vector<int32> resistances;
	size_t equipped = 0;

	// Same selection and values as rune_tablet_apply_bonuses.
	for_each_equipped(sd, [&](const std::shared_ptr<s_rune_tablet>& set) {
		const uint16 count = active_fragments(sd, *set);
		const int32 level = upgrade_level(sd, *set);
		bool active = false;

		++equipped;
		safesnprintf(output, sizeof(output), "Rune Set %u (category %hu): %hu/%hu fragments, upgrade +%d", set->id, set->category, count, static_cast<uint16>(set->fragments.size()), level);
		clif_displaymessage(fd, output);

		for (const s_rune_tablet_effect& effect : set->effects) {
			const int32 value = rune_tablet_logic::effect_value(effect, count, level);

			if (!value)
				continue;

			active = true;
			safesnprintf(output, sizeof(output), "  %s %d", effect.label.c_str(), value);
			clif_displaymessage(fd, output);

			if (effect.special == e_rune_tablet_special::none && effect.bonus == SP_RESEFF && effect.has_parameter
				&& std::find(resistances.begin(), resistances.end(), effect.parameter) == resistances.end())
				resistances.push_back(effect.parameter);
		}

		if (!active)
			clif_displaymessage(fd, "  No active effect: too few fragments or upgrade levels.");

		return true;
	});

	if (equipped == 0) {
		safesnprintf(output, sizeof(output), "%s has no equipped Rune Set.", sd.status.name);
		clif_displaymessage(fd, output);
		return;
	}

	// Resistances are not shown by the client. These totals come from the
	// last status calculation and include every source (equipment, cards).
	for (int32 id : resistances) {
		int32 total = 0;

		for (const s_item_bonus& it : sd.reseff) {
			if (it.id == id)
				total += it.val;
		}

		const char* name = script_get_constant_str("Eff_", id);

		safesnprintf(output, sizeof(output), "Total %s resistance (all sources): %d.%02d%%", name != nullptr ? name : "?", total / 100, std::abs(total % 100));
		clif_displaymessage(fd, output);
	}
}

void rune_tablet_on_targeted_magic_kill(map_session_data& sd) {
	int32 hp = 0, sp = 0;

	for_each_equipped(sd, [&sd, &hp, &sp](const std::shared_ptr<s_rune_tablet>& set) {
		const uint16 count = active_fragments(sd, *set);
		const int32 level = upgrade_level(sd, *set);

		for (const s_rune_tablet_effect& effect : set->effects) {
			switch (effect.special) {
				case e_rune_tablet_special::targeted_magic_kill_recovery_hp:
					hp += rune_tablet_logic::effect_value(effect, count, level);
					break;
				case e_rune_tablet_special::targeted_magic_kill_recovery_sp:
					sp += rune_tablet_logic::effect_value(effect, count, level);
					break;
				default:
					break;
			}
		}

		return true;
	});

	if (hp || sp)
		status_heal(&sd, hp, sp, battle_config.show_hp_sp_gain ? 3 : 1);
}

void rune_tablet_on_hit(map_session_data& sd) {
	bool proc = false;

	for_each_equipped(sd, [&sd, &proc](const std::shared_ptr<s_rune_tablet>& set) {
		for (const s_rune_tablet_effect& effect : set->effects) {
			// Fixed-rate proc, not level-scaled: Fragments/MinLevel only gate it.
			// Rate, amount, ticks and interval match the reviewed Korean source
			// (2% on hit, 200 SP every second, four times), not the client's numbers.
			if (effect.special == e_rune_tablet_special::on_hit_sp_regen_chance
				&& active_fragments(sd, *set) >= effect.fragments && upgrade_level(sd, *set) >= effect.min_level) {
				proc = true;
				return false; // One roll however many sets carry the effect.
			}
		}

		return true;
	});

	if (proc)
		sc_start(&sd, &sd, SC_RUNE_TABLET_REGEN, 2, 4, 1000);
}

void rune_tablet_sync_equipped(map_session_data& sd) {
#if PACKETVER_MAIN_NUM >= 20230802
	using namespace rune_tablet_wire;

	if (!sd.vars_ok || !access_allowed(sd))
		return;

	if (prune_equipped(sd)) {
		status_change_end(&sd, SC_RUNE_TABLET_REGEN);
		status_calc_pc(&sd, SCO_NONE);
	}

	const std::vector<std::shared_ptr<s_rune_tablet>> equipped = equipped_sets(sd);

	// The client resolves equipped sets by ID, so the lists of their
	// categories go first (once per category).
	std::vector<uint16> listed;

	for (const std::shared_ptr<s_rune_tablet>& set : equipped) {
		if (std::find(listed.begin(), listed.end(), set->category) != listed.end())
			continue;

		send_progress(sd, set->category, false);
		send_progress(sd, set->category, true);
		listed.push_back(set->category);
	}

	if (max_equipped() == 1) {
		// Official single slot: 0x0bf2 with the level and active fragment count.
		if (equipped.empty())
			send(sd, rune_tablet_wire::equipped(0, 0, 0, 0));
		else
			send(sd, rune_tablet_wire::equipped(equipped[0]->category, equipped[0]->id,
				static_cast<uint16>(upgrade_level(sd, *equipped[0])), active_fragments(sd, *equipped[0])));
		return;
	}

	// Multiple sets: clear every category first (client caches keep panes per
	// category, also across characters), then announce each equipped set.
	// 0x0bd7 is used rather than 0x0bf2/0x0bd5 because those make the client
	// request the category again for every set; the client takes the level
	// from the set list and counts the active fragments itself.
	for (uint16 category : rune_tablet_db.getCategories())
		send(sd, operation<9>(equip_result, 0, category, 0));

	for (const std::shared_ptr<s_rune_tablet>& set : equipped)
		send(sd, operation<9>(equip_result, 0, set->category, set->id));
#endif
}

bool rune_tablet_open(map_session_data& sd, bool from_npc) {
#if PACKETVER_MAIN_NUM >= 20230802
	if (sd.state.rune_tablet_open) {
		clif_displaymessage(sd.fd, "Rune Tablet is already open. Close the window before opening it again.");
		return true;
	}

	if (!sd.vars_ok || !access_allowed(sd)) {
		clif_displaymessage(sd.fd, "Rune Tablet: not available yet.");
		return false;
	}

	// A Rune Stone NPC calls this from its own script, so its npc_id is set;
	// every other state that blocks acting still applies.
	const bool busy = from_npc ? (sd.chatID || pc_cant_act2(&sd)) : pc_cant_act(&sd);

	if (pc_isdead(&sd) || busy || sd.state.mail_writing || sd.state.banking || sd.state.cashshop_open) {
		clif_displaymessage(sd.fd, "Rune Tablet: close other windows before opening it.");
		return false;
	}

	if (rune_tablet_db.empty()) {
		clif_displaymessage(sd.fd, "Rune Tablet: no definitions are loaded. Check db/rune_tablet_db.yml and the server log.");
		return false;
	}

	sd.state.rune_tablet_open = true;
	sd.state.rune_tablet_activation = true;
	clif_displaymessage(sd.fd, "Rune Tablet: Activate and Upgrade consume materials. A failed upgrade keeps the level and raises the chance of the next attempt.");
	send(sd, rune_tablet_wire::window(true));
	rune_tablet_sync_equipped(sd);
	return true;
#else
	clif_displaymessage(sd.fd, "Rune Tablet requires a compatible client (2023-08-02 or newer).");
	return false;
#endif
}

void rune_tablet_close(map_session_data& sd) {
#if PACKETVER_MAIN_NUM >= 20230802
	if (sd.state.rune_tablet_open) {
		sd.state.rune_tablet_open = false;
		sd.state.rune_tablet_activation = false;
		send(sd, rune_tablet_wire::window(false));
	}
#endif
}

void clif_parse_rune_tablet(int32 fd, map_session_data* sd) {
#if PACKETVER_MAIN_NUM >= 20230802
	using namespace rune_tablet_wire;

	if (sd == nullptr)
		return;

	const uint8* data = static_cast<const uint8*>(RFIFOP(fd, 0));
	const uint16 type = get16(data);

	if (type == window_state) {
		// UIRuneSystemWnd sends 1 from its constructor and 0 from its destructor.
		// Only the latter closes; do not echo a close packet.
		if (data[2] != 0)
			return;

		sd->state.rune_tablet_open = false;
		sd->state.rune_tablet_activation = false;
		return;
	}

	// Fixed lengths are validated by clif's packet dispatcher before this call.
	// The client requests a category after 0x0bd5/0x0bf2 even when the window
	// is closed: read-only category refresh is allowed, mutations need the window.
	if ((!sd->state.rune_tablet_open && type != request_category) || !sd->vars_ok || !access_allowed(*sd)) {
		ShowWarning("Rune Tablet: ignored packet 0x%04x from character %d (window/registry/GM state).\n", type, sd->status.char_id);
		return;
	}

	if (pc_isdead(sd) || sd->state.trading || sd->state.vending || sd->state.buyingstore
		|| sd->state.storage_flag || sd->state.mail_writing || sd->state.banking
		|| sd->state.cashshop_open || sd->npc_id || sd->npc_shopid) {
		rune_tablet_close(*sd);
		return;
	}

	if (type == request_category) {
		const uint16 category = get16(data + 2);

		send_progress(*sd, category, false);
		send_progress(*sd, category, true);
#if PACKETVER_MAIN_NUM >= 20241016
		send_rewards(*sd, category);
#endif
		return;
	}

	if (sd->state.rune_tablet_activation && (type == activate_fragment || type == activate_set)) {
		const uint16 category = get16(data + 2);
		const uint32 id = get32(data + 4);
		const uint8 result = activate(*sd, category, id, type == activate_set);

		if (type == activate_set)
			send(*sd, operation<13>(set_result, result, category, id));
		else
			send(*sd, operation<9>(fragment_result, result, category, id));

		if (result == 0)
			rune_tablet_sync_equipped(*sd);
		return;
	}

	if (type == upgrade_set && sd->state.rune_tablet_activation) {
		const uint16 category = get16(data + 2);
		const uint32 id = get32(data + 4);
		int64 next = 0;

		if (const char* error = upgrade(*sd, category, id, next)) {
			// No invented native error code: closing clears the pending UI.
			rune_tablet_close(*sd);
			clif_displaymessage(fd, error);
			return;
		}

		send(*sd, upgrade_state(0, category, id,
			static_cast<uint16>(rune_tablet_logic::level(next)),
			static_cast<uint16>(rune_tablet_logic::failures(next))));
		rune_tablet_sync_equipped(*sd);
		return;
	}

	if (type == equip) {
		const uint16 raw_category = get16(data + 2);
		const uint16 category = raw_category & ~unequip_set_flag;
		const uint32 id = get32(data + 4);

		if (id == 0 || (raw_category & unequip_set_flag)) {
			// The native Unequip button sends 0x0bd6 with ID 0 and the category of
			// the selected equipped set; a 0x0bd7 success with ID 0 clears it.
			// The single official slot clears whatever is equipped; with multiple
			// sets it clears the whole category, unless the client names one set
			// with unequip_set_flag.
			std::vector<uint16> cleared;
			const bool single = max_equipped() == 1;
			const uint32 target = (raw_category & unequip_set_flag) ? id : 0;

			if (!clear_equipped(*sd, single && !target ? 0 : category, target, cleared))
				return;

			if (cleared.empty())
				cleared.push_back(category);

			for (uint16 current : cleared)
				send(*sd, operation<9>(equip_result, 0, current, 0));

			// The category clears also removed its remaining sets on the client.
			if (!single)
				rune_tablet_sync_equipped(*sd);
			return;
		}

		std::shared_ptr<s_rune_tablet> set = rune_tablet_db.find(id);

		if (set == nullptr || !set->is_set || set->category != category || !owned(*sd, *set)) {
			send(*sd, operation<9>(equip_result, 6, category, id));
			clif_displaymessage(fd, "Rune Tablet: invalid or inactive set.");
			return;
		}

		if (!equippable(*set)) {
			rune_tablet_close(*sd);
			clif_displaymessage(fd, "Rune Tablet: this set's effects are not verified yet. The equipped set was not changed.");
			return;
		}

		if (!equip_set(*sd, *set))
			return;

		send(*sd, operation<9>(equip_result, 0, category, id));

		if (max_equipped() > 1)
			rune_tablet_sync_equipped(*sd);
		return;
	}

	if (type == unequip) {
		// No payload: clear every equipped set.
		std::vector<uint16> cleared;

		if (!clear_equipped(*sd, 0, 0, cleared))
			return;

		if (cleared.empty())
			cleared.push_back(0);

		for (uint16 current : cleared)
			send(*sd, operation<9>(unequip_result, 0, current, 0));

		if (max_equipped() > 1)
			rune_tablet_sync_equipped(*sd);
		return;
	}

#if PACKETVER_MAIN_NUM >= 20241016
	if (type == claim_reward) {
		const uint16 category = get16(data + 2);
		const uint32 id = get32(data + 4);
		uint8 claimed = 0;
		const uint8 result = grant_reward(*sd, category, id, data[8], claimed);

		send(*sd, reward_state(result, category, id, claimed));
		return;
	}
#endif

	if (type == decompose) {
		s_decomposition_request request{};
		const char* message = nullptr;

		if (!read_decomposition_request(data, 14, MAX_INVENTORY, request)) {
			message = "Rune Tablet: invalid decomposition request.";
		} else {
			const item& source = sd->inventory.u.items_inventory[request.server_index];
			std::shared_ptr<s_rune_tablet_decomposition> table = rune_tablet_decomposition_db.find(request.item_id);

			if (sd->inventory_data[request.server_index] == nullptr || source.nameid != request.item_id || !eligible(source)) {
				message = "Rune Tablet: the decomposition item is missing, different, or protected.";
			} else if (table == nullptr) {
				message = "Rune Tablet: this item cannot be decomposed.";
			} else if (source.amount < table->modes[request.mode - 1].consume) {
				message = "Rune Tablet: not enough units in that stack for this mode.";
			} else if ((message = decomposition_capacity_error(*sd, table->modes[request.mode - 1])) == nullptr) {
				std::array<uint8, 51> response{};

				message = deliver_decomposition(*sd, request, table->modes[request.mode - 1], response);

				if (message == nullptr) {
					send(*sd, response);
					return;
				}
			}

			ShowInfo("Rune Tablet: decomposition preflight character %d, item %u, server index %u, mode %u; attempt rejected.\n",
				sd->status.char_id, request.item_id, request.server_index, request.mode);
		}

		// Rejected attempts close the pending UI operation without guessing
		// undocumented native error codes.
		rune_tablet_close(*sd);
		clif_displaymessage(fd, message);
		return;
	}

	// No native result is verified for an unimplemented operation. Closing
	// clears the pending UI state without claiming that the database is loading.
	rune_tablet_close(*sd);
	clif_displaymessage(fd, "Rune Tablet: this operation is not implemented yet; no items or progress were changed.");
	ShowInfo("Rune Tablet: probe rejected operation 0x%04x for character %d.\n", type, sd->status.char_id);
#endif
}
