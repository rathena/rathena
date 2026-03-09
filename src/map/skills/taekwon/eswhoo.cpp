// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "eswhoo.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEswhoo::SkillEswhoo() : SkillImplRecursiveDamageSplash(SP_SWHOO) {
}

void SkillEswhoo::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += 1000 + 200 * skill_lv;
	RE_LVL_DMOD(100);
}

int64 SkillEswhoo::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// If a enemy player is standing next to a mob when splash Es- skill is casted, the player won't get hurt.
	if (!battle_config.allow_es_magic_pc && target->type != BL_MOB)
		return 0;

	return SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
}

void SkillEswhoo::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd && !battle_config.allow_es_magic_pc && target->type != BL_MOB) {
		status_change_start(src, target, SC_STUN, 10000, skill_lv, 0, 0, 0, 500, 10);
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		return;
	}
	status_change_end(src, SC_USE_SKILL_SP_SPA);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
