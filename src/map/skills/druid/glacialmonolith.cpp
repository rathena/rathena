// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacialmonolith.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillGlacialMonolith::SkillGlacialMonolith() : SkillImplRecursiveDamageSplash(AT_GLACIER_MONOLITH) {
}

void SkillGlacialMonolith::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 7100 + 300 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_ICE)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 10 * sstatus->spl;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_ICE
	RE_LVL_DMOD(100);
}

void SkillGlacialMonolith::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	SkillImplRecursiveDamageSplash::castendPos2(src, x, y, skill_lv, tick, flag);

	if (unit_data* ud = unit_bl2ud(src); ud != nullptr) {
		for (size_t i = 0; i < ud->skillunits.size();) {
			std::shared_ptr<s_skill_unit_group> sg = ud->skillunits[i];

			if (sg != nullptr && (sg->skill_id == AT_GLACIER_MONOLITH || sg->unit_id == UNT_GLACIAL_MONOLITH)) {
				skill_delunitgroup(sg);
				continue;
			}

			++i;
		}
	}

	const t_tick buff_duration = skill_get_unit_interval(getSkillId());
	sc_start4(src, src, skill_get_sc(getSkillId()), 100, skill_lv, x, y, src->m, buff_duration);

	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
