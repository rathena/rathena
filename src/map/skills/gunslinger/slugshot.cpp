// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "slugshot.hpp"

SkillSlugShot::SkillSlugShot() : WeaponSkillImpl(RL_SLUGSHOT) {
}

void SkillSlugShot::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillSlugShot::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* tstatus = status_get_status_data(*target);

	if (target->type == BL_MOB) {
		skillratio += -100 + 1200 * skill_lv;
	} else {
		skillratio += -100 + 2000 * skill_lv;
	}
	skillratio *= 2 + tstatus->size;
}

void SkillSlugShot::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	int8 dist = distance_bl(src, target);

	if (dist > 3) {
		// Reduce hit rate for each cell after initial 3 cells.
		dist -= 3;
		hit_rate -= ((11 - skill_lv) * dist);
	}
}
