// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gravityhole.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

int32 apply_gravity_hole_sub(block_list* bl, va_list ap);
int32 apply_gravity_hole_hit(block_list* src, block_list* bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag);

SkillGravityHole::SkillGravityHole() : SkillImplRecursiveDamageSplash(AT_GRAVITY_HOLE) {
}

void SkillGravityHole::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillGravityHole::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if ((flag & 1) || target != src) {
		apply_gravity_hole_hit(src, target, getSkillId(), skill_lv, tick, flag);
		return;
	}

	const int32 splash = skill_get_splash(getSkillId(), skill_lv);
	const int32 max_targets = 5 + skill_lv;

	skill_area_temp[0] = 0;
	skill_area_temp[1] = 0;
	skill_area_temp[2] = 0;
	map_foreachinrange(apply_gravity_hole_sub, src, splash, BL_CHAR, src, getSkillId(), skill_lv, tick, flag, max_targets);
}

void SkillGravityHole::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	int32 skillratio = 9800 + 800 * (skill_lv - 1);
	base_skillratio += -100 + skillratio;
}

int64 SkillGravityHole::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

int32 apply_gravity_hole_sub(block_list* bl, va_list ap) {
	block_list* src = va_arg(ap, block_list*);
	uint16 skill_id = static_cast<uint16>(va_arg(ap, int));
	uint16 skill_lv = static_cast<uint16>(va_arg(ap, int));
	t_tick tick = va_arg(ap, t_tick);
	int32 flag = va_arg(ap, int32);
	int32 max_targets = va_arg(ap, int32);

	if (skill_area_temp[2] >= max_targets) {
		return 0;
	}

	if (apply_gravity_hole_hit(src, bl, skill_id, skill_lv, tick, flag) > 0) {
		skill_area_temp[2]++;
		return 1;
	}

	return 0;
}

int32 apply_gravity_hole_hit(block_list* src, block_list* bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) {
	if (battle_check_target(src, bl, BCT_ENEMY) <= 0) {
		return 0;
	}

	int32 sflag = flag | SD_ANIMATION;
	if (skill_attack(skill_get_type(static_cast<e_skill>(skill_id)), src, src, bl, skill_id, skill_lv, tick, sflag) > 0) {
		int32 dist = distance_xy(src->x, src->y, bl->x, bl->y) - 1;
		const sc_type type = skill_get_sc(skill_id);
		const t_tick duration = skill_get_time(skill_id, skill_lv);

		if (type != SC_NONE && duration > 0) {
			sc_start(src, bl, type, 100, 1, duration);
		}
		if (dist > 0) {
			int8 dir = map_calc_dir(src, bl->x, bl->y);
			skill_blown(src, bl, static_cast<char>(dist), dir, BLOWN_NONE);
		}
		return 1;
	}

	return 0;
}
