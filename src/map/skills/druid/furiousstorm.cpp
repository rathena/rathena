// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "furiousstorm.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFuriousStorm::SkillFuriousStorm() : SkillImplRecursiveDamageSplash(AT_FURIOS_STORM) {
}

void SkillFuriousStorm::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 7800 + 400 * (skill_lv - 1);
	skillratio += 20 * sstatus->spl;

	RE_LVL_DMOD(100);
}

void SkillFuriousStorm::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	this->castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillFuriousStorm::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
