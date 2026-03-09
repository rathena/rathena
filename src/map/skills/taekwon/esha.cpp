// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "esha.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEsha::SkillEsha() : SkillImplRecursiveDamageSplash(SP_SHA) {
}

void SkillEsha::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_SP_SHA, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillEsha::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 5 * skill_lv;
}

int64 SkillEsha::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// If a enemy player is standing next to a mob when splash Es- skill is casted, the player won't get hurt.
	if (!battle_config.allow_es_magic_pc && target->type != BL_MOB)
		return 0;

	return SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
}

void SkillEsha::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd && !battle_config.allow_es_magic_pc && target->type != BL_MOB) {
		status_change_start(src, target, SC_STUN, 10000, skill_lv, 0, 0, 0, 500, 10);
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		return;
	}

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
