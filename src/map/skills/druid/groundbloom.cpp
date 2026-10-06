// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "groundbloom.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillGroundBloom::SkillGroundBloom() : SkillImplRecursiveDamageSplash(KR_GROUND_BLOOM) {
}

void SkillGroundBloom::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 6000 + 2000 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 15 * sstatus->int_;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_EARTH
	RE_LVL_DMOD(100);
}

void SkillGroundBloom::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	status_change_end(src, SC_GROUND_GROW);

	int64 heal = static_cast<int64>(status_get_max_hp(src)) * (skill_lv * 3) / 100;
	if (heal > 0) {
		status_heal(src, heal, 0, 0);
	}

	this->castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillGroundBloom::castGroundBloom(block_list *src, t_tick tick, int32 stacks) {
	if (stacks <= 0)
		return;

	uint16 skill_lv = pc_checkskill(BL_CAST(BL_PC, src), KR_EARTH_BUD);

	if (skill_lv <= 0)
		return;

	status_change *sc = status_get_sc(src);

	if (sc == nullptr)
		return;

	if (sc->hasSCE(SC_GROUND_GROW)) {
		stacks += sc->getSCE(SC_GROUND_GROW)->val3;
	}

	if (stacks < 13) {
		// Note: official gives the status change regardless of the stacks amount then casts the skill and removes the sc when the conditions are met (same result)
		status_change_start(src, src, SC_GROUND_GROW, 10000, 0, 0, stacks, 0, 10000, SCSTART_NOAVOID);
		return;
	}

	skill_castend_nodamage_id(src, src, KR_GROUND_BLOOM, skill_lv, tick, 0);
}
