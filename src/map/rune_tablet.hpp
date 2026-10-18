// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
// Rune Tablet
// Original source: Scanty (https://rathena.org/board/profile/35710-scanty/)
// Multiple equipped Rune Sets: Louis T Steinhil

#ifndef RUNE_TABLET_HPP
#define RUNE_TABLET_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/mmo.hpp>

class map_session_data;

/// Upper bound of rune_tablet_max_equipped. Well above the loaded data (32 pre-renewal,
/// 65 renewal sets); values at or above the set count mean "every set".
#define MAX_RUNE_TABLET_EQUIPPED 255
/// Collection reward slots per Rune Set (runeset_reward.lub).
#define MAX_RUNE_TABLET_REWARD 7
/// Highest Rune Set upgrade level.
#define MAX_RUNE_TABLET_UPGRADE 15

/**
 * Rune Tablet packet IDs, see doc/rune_tablet.txt for the formats.
 * Names are descriptive, not recovered Gravity symbols.
 */
namespace rune_tablet_wire {
enum e_packet : uint16 {
	request_category = 0x0bcb, ///< CZ: list a category; also sent by the client after 0x0bd5 and 0x0bf2
	fragment_list = 0x0bcc, ///< ZC: activated fragments of a category
	set_list = 0x0bcd, ///< ZC: activated sets of a category with level and failure count
	activate_fragment = 0x0bce, ///< CZ
	fragment_result = 0x0bcf, ///< ZC
	activate_set = 0x0bd0, ///< CZ
	set_result = 0x0bd1, ///< ZC
	upgrade_set = 0x0bd2, ///< CZ
	upgrade_result = 0x0bd3, ///< ZC
	unequip = 0x0bd4, ///< CZ: no payload; the 2025-07-16 client never sends it
	unequip_result = 0x0bd5, ///< ZC
	equip = 0x0bd6, ///< CZ: category and set ID; set ID 0 is the native Unequip button
	equip_result = 0x0bd7, ///< ZC
	decompose = 0x0bd8, ///< CZ
	decompose_result = 0x0bd9, ///< ZC
	open_window = 0x0bdf, ///< ZC
	window_state = 0x0be0, ///< CZ: 1 from the window constructor, 0 from its destructor
	equipped_state = 0x0bf2, ///< ZC
	reward_list = 0x0c15, ///< ZC, 2024-10-16 and newer
	claim_reward = 0x0c16, ///< CZ, 2024-10-16 and newer
	reward_result = 0x0c17, ///< ZC, 2024-10-16 and newer
};

/// Category flag of 0x0bd6: unequip exactly the given set ID (client extension for
/// multiple equipped sets; the native Unequip button sends the category and ID 0).
constexpr uint16 unequip_set_flag = 0x8000;
}

/// Item cost of an activation or upgrade attempt.
struct s_rune_tablet_material {
	t_itemid item; ///< Item ID
	uint32 amount; ///< Amount
};

/// Rune Set effects that bypass the stat-bonus pool and run from dedicated hooks.
enum class e_rune_tablet_special : uint8 {
	none,
	targeted_magic_kill_recovery_hp, ///< HP on a targeted magic kill, see rune_tablet_on_targeted_magic_kill
	targeted_magic_kill_recovery_sp, ///< SP on a targeted magic kill
	on_hit_sp_regen_chance, ///< 2% on hit: 200 SP every second, four times (SC_RUNE_TABLET_REGEN)
};

/// Rune Set effect: value = base + (level / level_step) * per_step.
struct s_rune_tablet_effect {
	uint16 fragments = 0; ///< Active fragments required
	uint16 min_level = 0; ///< Minimum upgrade level
	uint16 level_step = 1; ///< Upgrade levels per step
	int32 base = 0; ///< Value at the first step
	int32 per_step = 0; ///< Value added per step
	int32 bonus = 0; ///< pc_bonus/pc_bonus2 type
	int32 parameter = 0; ///< First pc_bonus2 argument
	bool has_parameter = false; ///< Apply with pc_bonus2
	e_rune_tablet_special special = e_rune_tablet_special::none; ///< Dedicated hook instead of a bonus
	std::string label; ///< Bonus[,Parameter] or Special as written in the database (@runetablet info)
};

/// Cost and chance of one upgrade target level.
struct s_rune_tablet_upgrade {
	uint32 chance = 0; ///< Base chance, 100000 = 100% (client GradeTable)
	uint32 failure_increment = 0; ///< Chance added per failed attempt at this level
	std::vector<s_rune_tablet_material> materials; ///< Cost of one attempt
};

/// Rune fragment or Rune Set from db/rune_tablet_db.yml.
struct s_rune_tablet {
	uint32 id = 0; ///< Fragment or set ID
	uint16 category = 0; ///< Client category tab
	bool is_set = false; ///< Rune Set rather than fragment
	std::vector<s_rune_tablet_material> materials; ///< Activation cost
	std::vector<uint32> fragments; ///< Fragments of the set
	std::vector<s_rune_tablet_effect> effects; ///< Effects while the set is equipped
	bool effect_free = false; ///< Officially grants nothing, yet may be equipped
	t_itemid rewards[MAX_RUNE_TABLET_REWARD] = {}; ///< Collection reward per slot, 0 = empty
	std::map<uint16, s_rune_tablet_upgrade> upgrades; ///< Target levels 1..15
};

class RuneTabletDatabase : public TypesafeYamlDatabase<uint32, s_rune_tablet> {
private:
	std::vector<uint16> categories; ///< Categories containing a Rune Set, ascending
	std::vector<std::shared_ptr<s_rune_tablet>> sets; ///< Rune Sets in ascending (category, ID) order

public:
	RuneTabletDatabase() : TypesafeYamlDatabase("RUNE_TABLET_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void loadingFinished() override;
	void clear() override;

	/**
	 * Categories that contain at least one Rune Set.
	 * @return Category IDs in ascending order
	 */
	const std::vector<uint16>& getCategories() const;

	/**
	 * All Rune Sets, the iteration order of equipped sets.
	 * @return Sets in ascending (category, ID) order
	 */
	const std::vector<std::shared_ptr<s_rune_tablet>>& getSets() const;
};

/// One decomposition result.
struct s_rune_tablet_decomposition_output {
	t_itemid item = 0; ///< Result item ID
	uint16 minimum = 0; ///< Minimum amount
	uint16 maximum = 0; ///< Maximum amount
	uint32 chance = 0; ///< 100000 = 100%; random draw relationships remain unverified
};

/// One request mode of a decomposable card.
struct s_rune_tablet_decomposition_mode {
	uint16 consume = 0; ///< Cards taken from the selected stack (client 1/30)
	std::vector<s_rune_tablet_decomposition_output> outputs; ///< One to eight results
};

/// Decomposable card from db/rune_tablet_decomposition.yml (client System/Rune/itemDecom.lub).
struct s_rune_tablet_decomposition {
	t_itemid item = 0; ///< Card item ID
	s_rune_tablet_decomposition_mode modes[2]; ///< Request modes 1 and 2
};

class RuneTabletDecompositionDatabase : public TypesafeYamlDatabase<t_itemid, s_rune_tablet_decomposition> {
public:
	RuneTabletDecompositionDatabase() : TypesafeYamlDatabase("RUNE_TABLET_DECOMPOSITION_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

extern RuneTabletDatabase rune_tablet_db;
extern RuneTabletDecompositionDatabase rune_tablet_decomposition_db;

void do_init_rune_tablet();
void do_final_rune_tablet();

/**
 * Reload both Rune Tablet databases, close open windows and recalculate the
 * status of online players so changed effects apply immediately.
 */
void rune_tablet_reload();

/**
 * Apply the stat bonuses of the equipped Rune Sets.
 * Called only during the normal status rebuild, after previous bonuses reset.
 * @param sd: Player
 */
void rune_tablet_apply_bonuses(map_session_data& sd);

/**
 * Report the equipped Rune Sets of a player, their active effects and the
 * resulting resistance totals of the last status calculation (@runetablet info).
 * @param sd: Player to inspect
 * @param fd: Session that receives the report
 */
void rune_tablet_show_effects(map_session_data& sd, int32 fd);

/**
 * Targeted magic kill hook (TargetedMagicKillRecoveryHP/SP effects).
 * The caller already filtered BF_MAGIC, a dead target, actual HP loss and
 * excluded ground/self skills.
 * @param sd: Killer
 */
void rune_tablet_on_targeted_magic_kill(map_session_data& sd);

/**
 * Hit hook (OnHitSPRegenChance effect).
 * Called only after actual HP loss on a surviving player.
 * @param sd: Player that was hit
 */
void rune_tablet_on_hit(map_session_data& sd);

/**
 * Send the equipped Rune Sets to the client, after the category lists the
 * client needs to resolve them: 0x0bf2 for the single official slot, or a
 * 0x0bd7 clear per category followed by one 0x0bd7 per equipped set when
 * rune_tablet_max_equipped is above 1. Also converts legacy storage and
 * prunes sets that no longer fit the equip rules.
 * @param sd: Player
 */
void rune_tablet_sync_equipped(map_session_data& sd);

/**
 * Open the native Rune Tablet window.
 * @param sd: Player
 * @param from_npc: Called by a Rune Stone NPC script, whose own npc_id is still set
 * @return true when the window is open
 */
bool rune_tablet_open(map_session_data& sd, bool from_npc = false);

/**
 * Close the Rune Tablet window and notify the client.
 * @param sd: Player
 */
void rune_tablet_close(map_session_data& sd);

/**
 * Parser of every Rune Tablet request packet.
 * @param fd: Client socket
 * @param sd: Player
 */
void clif_parse_rune_tablet(int32 fd, map_session_data* sd);

#endif /* RUNE_TABLET_HPP */
