// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "catnipmeteor2.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillCatnipMeteor2::SkillCatnipMeteor2() : SkillImpl(SU_CN_METEOR2) {
}

void SkillCatnipMeteor2::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_CURSE, 20, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
void SkillCatnipMeteor2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 200 + 100 * skill_lv;
	if (status_get_lv(src) > 99) {
		skillratio += sstatus->int_ * 5;
	}
	RE_LVL_DMOD(100);
}
