// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwspiritsphere.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillThrowSpiritSphere::SkillThrowSpiritSphere() : WeaponSkillImpl(MO_FINGEROFFENSIVE) {
}

void SkillThrowSpiritSphere::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	if (battle_config.finger_offensive_type && sd) {
		for (int32 i = 1; i < sd->spiritball_old; i++)
			skill_addtimerskill(src, tick + i * 200, target->id, 0, 0, getSkillId(), skill_lv, BF_WEAPON, flag);
	}
	status_change_end(src, SC_BLADESTOP);
}

void SkillThrowSpiritSphere::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const status_change* tsc = status_get_sc(target);

	base_skillratio += 500 + skill_lv * 200;
	if (tsc && tsc->getSCE(SC_BLADESTOP))
		base_skillratio += base_skillratio / 2;
#else
	base_skillratio += 50 * skill_lv;
#endif
}
