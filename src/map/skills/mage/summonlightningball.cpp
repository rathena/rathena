// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "summonlightningball.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

// WL_SUMMONBL
SkillSummonLightningBall::SkillSummonLightningBall() : SkillImpl(WL_SUMMONBL) {
}

void SkillSummonLightningBall::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);
	int32 i = 0;

	if (sc == nullptr)
		return;

	// Set val2. The SC element for this ball
	e_wl_spheres element = WLS_WIND;

	if (skill_lv == 1) {
		sc_type sphere = SC_NONE;

		for (i = SC_SPHERE_1; i <= SC_SPHERE_5; i++) {
			if (sc->getSCE(i) == nullptr) {
				sphere = static_cast<sc_type>(i); // Take the free SC
				break;
			}
		}

		if (sphere == SC_NONE) {
			if (sd) // No free slots to put SC
				clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_SUMMON );
			return;
		}

		sc_start2(src, src, sphere, 100, element, skill_lv, skill_get_time(getSkillId(), skill_lv));
	} else {
		for (i = SC_SPHERE_1; i <= SC_SPHERE_5; i++) {
			status_change_end(src, static_cast<sc_type>(i)); // Removes previous type
			sc_start2(src, src, static_cast<sc_type>(i), 100, element, skill_lv, skill_get_time(getSkillId(), skill_lv));
		}
	}

	clif_skill_nodamage(src, *target, getSkillId(), 0, false);
}


// WL_SUMMON_ATK_WIND
SkillSummonAttackWind::SkillSummonAttackWind() : SkillImpl(WL_SUMMON_ATK_WIND) {
}

void SkillSummonAttackWind::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += 200;
	RE_LVL_DMOD(100);
}
