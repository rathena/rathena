// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "howlingmine.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHowlingMine::SkillHowlingMine() : SkillImplRecursiveDamageSplash(RL_H_MINE) {
}

void SkillHowlingMine::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	if (const map_session_data* sd = BL_CAST(BL_PC, src); sd == nullptr || !sd->flicker) {
		status_change_start(src, target, SC_H_MINE, 10000, getSkillId(), 0, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NOAVOID | SCSTART_NOTICKDEF | SCSTART_NORATEDEF);
	}
	else {
		status_change* tsc = status_get_sc(target);

		if (tsc != nullptr && tsc->hasSCE(SC_H_MINE) && tsc->getSCE(SC_H_MINE)->val2 == src->id) {
			status_change_end(target, SC_H_MINE);
			sc_start4(src, target, SC_BURNING, 10 * skill_lv, skill_lv, 1000, src->id, 0, skill_get_time2(getSkillId(), skill_lv));
		}
	}
}

void SkillHowlingMine::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	if (const map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr && sd->flicker) {
		// Flicker explosion damage: 500 + 300 * SkillLv
		skillratio += -100 + 500 + 300 * skill_lv;
	}
	else {
		// Direct trigger damage: 200 + 200 * SkillLv
		skillratio += -100 + 200 + 200 * skill_lv;
	}
}

void SkillHowlingMine::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// Direct attack
	if (const map_session_data* sd = BL_CAST(BL_PC, src); sd == nullptr || !sd->flicker) {
		SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
		return;
	}

	// Explosion, triggered by RL_FLICKER
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag & ~SD_SPLASH);

	// Don't consume requirement
	flag |= SKILL_NOCONSUME_REQ;
}
