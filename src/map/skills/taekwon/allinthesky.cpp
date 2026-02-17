// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "allinthesky.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillAllInTheSky::SkillAllInTheSky() : SkillImpl(SKE_ALL_IN_THE_SKY) {
}

void SkillAllInTheSky::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (target->type == BL_PC)
		status_zap(target, 0, 0, status_get_ap(target));
	if( unit_movepos( src, target->x, target->y, 2, true ) ){
		clif_snap(src, src->x, src->y);
	}
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillAllInTheSky::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 250 + 1200 * skill_lv;
	base_skillratio += 5 * sstatus->pow;
}
