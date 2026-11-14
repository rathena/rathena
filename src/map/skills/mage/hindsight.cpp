// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hindsight.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillHindsight::SkillHindsight() : SkillImpl(SA_AUTOSPELL) {
}

void SkillHindsight::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if (sd) {
		sd->state.workinprogress = WIP_DISABLE_ALL;
		clif_autospell( *sd, skill_lv );
	} else {
		// Fully data-driven selection from skill_db AutoSpell entries.
		// 1. Find the highest UnlockedAtLevel tier available for this skill_lv.
		// 2. Collect all spells at that tier, pick one at random.
// 3. Use the spell's MobMaxCastLevel as an upper bound.

		uint8 best_tier = 0;
		for (const auto& it : skill_db) {
			uint8 tier = it.second->autospell_unlocked_at;
			if (tier > 0 && tier <= skill_lv && tier > best_tier)
				best_tier = tier;
		}

		if (best_tier > 0) {
			// Collect all spells available at the best tier
			std::vector<std::shared_ptr<s_skill_db>> candidates;
			for (const auto& it : skill_db) {
				if (it.second->autospell_unlocked_at == best_tier)
					candidates.push_back(it.second);
			}

			if (candidates.empty())
				return;
			const auto& chosen = candidates[rnd() % candidates.size()];
			uint8 mob_max_cast_lv = chosen->autospell_mob_max_cast_level;
			uint16 cast_lv = static_cast<uint16>(skill_lv / 2);
			if (mob_max_cast_lv > 0 && cast_lv > mob_max_cast_lv)
				cast_lv = mob_max_cast_lv;
			if (cast_lv < 1)
				cast_lv = 1;
			sc_start4(src, src, SC_AUTOSPELL, 100, skill_lv,
				chosen->nameid, cast_lv, 0,
				skill_get_time(SA_AUTOSPELL, skill_lv));
		}
	}
}
