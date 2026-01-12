// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "backstab.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillBackStab::SkillBackStab() : SkillImpl(RG_BACKSTAP) {
}

void SkillBackStab::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST( BL_PC, src );

	if(sd && sd->status.weapon == W_BOW && battle_config.backstab_bow_penalty)
		base_skillratio += (200 + 40 * skill_lv) / 2;
	else
		base_skillratio += 200 + 40 * skill_lv;
}

void SkillBackStab::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

#ifdef RENEWAL
	uint8 dir = map_calc_dir(src, target->x, target->y);
	int16 x, y;

	if (dir > 0 && dir < 4)
		x = -1;
	else if (dir > 4)
		x = 1;
	else
		x = 0;

	if (dir > 2 && dir < 6)
		y = -1;
	else if (dir == 7 || dir < 2)
		y = 1;
	else
		y = 0;

	if (battle_check_target(src, target, BCT_ENEMY) > 0 && unit_movepos(src, target->x + x, target->y + y, 2, true)) { // Display movement + animation.
#else
	if (check_distance_bl(src, target, 0))
		return;

	uint8 dir = map_calc_dir(src, target->x, target->y), t_dir = unit_getdir(target);

	if (!map_check_dir(dir, t_dir) || target->type == BL_SKILL) {
#endif
		status_change_end(src, SC_HIDING);
		dir = dir < 4 ? dir+4 : dir-4; // change direction [Celest]
		unit_setdir(target,dir);
#ifdef RENEWAL
		clif_blown(src);
#endif
		skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag);
	}
	else if (sd)
		clif_skill_fail( *sd, getSkillId() );
}

void SkillBackStab::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
#ifdef RENEWAL
	sc_start(src,target,SC_STUN,(5+2*skill_lv),skill_lv,skill_get_time(getSkillId(),skill_lv));
#endif
}

void SkillBackStab::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
#ifdef RENEWAL
	hit_rate += skill_lv; // !TODO: What's the rate increase?
#endif
}
