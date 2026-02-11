// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hitandsliding.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/path.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillHitAndSliding::SkillHitAndSliding() : WeaponSkillImpl(ABC_HIT_AND_SLIDING) {
}

void SkillHitAndSliding::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	uint8 dir = DIR_NORTHEAST;

	// Total backslide = skill level + distance between player and target
	int32 total_backslide = skill_lv + distance_bl(src, target);

	if (target->x != src->x || target->y != src->y)
		dir = map_calc_dir(target, src->x, src->y);

	if (skill_check_unit_movepos(0, src, target->x + dirx[dir] * total_backslide, target->y + diry[dir] * total_backslide, 1, 1)) {
		clif_blown(src);
		unit_setdir(src, map_calc_dir(src, target->x, target->y)); // Set the player's direction to face the target
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	} else { //Is this the right behavior? [Haydrich]
		if (sd != nullptr)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
	}

	// Trigger skill animation
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, 1);
}

void SkillHitAndSliding::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 3500 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillHitAndSliding::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
