// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "knucklearrow.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillKnuckleArrow::SkillKnuckleArrow() : SkillImpl(SR_KNUCKLEARROW) {
}

void SkillKnuckleArrow::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);
	const map_session_data* tsd = BL_CAST(BL_PC, target);

	if (wd->miscflag&4) { // ATK [(Skill Level x 150) + (1000 x Target current weight / Maximum weight) + (Target Base Level x 5) x (Caster Base Level / 150)] %
		skillratio += -100 + 150 * skill_lv + status_get_lv(target) * 5;
		if (tsd && tsd->weight)
			skillratio += pc_getpercentweight(*tsd);
		RE_LVL_DMOD(150);
	} else {
		if (status_get_class_(target) == CLASS_BOSS)
			skillratio += 400 + 200 * skill_lv;
		else // ATK [(Skill Level x 100 + 500) x Caster Base Level / 100] %
			skillratio += 400 + 100 * skill_lv;
		RE_LVL_DMOD(100);
	}
	if (sc != nullptr && sc->hasSCE(SC_GT_CHANGE))
		skillratio += skillratio * 30 / 100;
}

void SkillKnuckleArrow::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Holds current direction of bl/target to src/attacker before the src is moved to bl location
	dir_ka = map_calc_dir(target, src->x, src->y);
	// Has slide effect
	if (skill_check_unit_movepos(5, src, target->x, target->y, 1, 1))
		skill_blown(src, src, 1, (dir_ka + 4) % 8, BLOWN_NONE); // Target position is actually one cell next to the target
	skill_addtimerskill(src, tick + 300, target->id, 0, 0, getSkillId(), skill_lv, BF_WEAPON, flag|SD_LEVEL|2);
}
