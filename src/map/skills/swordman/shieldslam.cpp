// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shieldslam.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillShieldSlam::SkillShieldSlam() : SkillImplRecursiveDamageSplash(IG_SHIELD_SLAM) {
}

void SkillShieldSlam::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 4000 * skill_lv + 1000 + 300 * pc_checkskill(sd, IG_SHIELD_MASTERY);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillShieldSlam::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (src->x != target->x || src->y != target->y) {
		uint8 dir = map_calc_dir(target, src->x, src->y);

		if (unit_movepos(src, target->x + dirx[dir], target->y + diry[dir], 2, true))
			clif_blown(src);
	}

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
