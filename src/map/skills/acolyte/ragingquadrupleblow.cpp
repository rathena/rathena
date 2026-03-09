// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ragingquadrupleblow.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"

SkillRagingQuadrupleBlow::SkillRagingQuadrupleBlow() : WeaponSkillImpl(MO_CHAINCOMBO) {
}

void SkillRagingQuadrupleBlow::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	status_change_end(src, SC_BLADESTOP);
}

void SkillRagingQuadrupleBlow::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const map_session_data* sd = BL_CAST(BL_PC, src);

	base_skillratio += 150 + 50 * skill_lv;
	if (sd && sd->status.weapon == W_KNUCKLE)
		base_skillratio *= 2;
#else
	base_skillratio += 50 + 50 * skill_lv;
#endif
}
