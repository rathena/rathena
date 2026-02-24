// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacialshard.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

#include "glacialnova.hpp"

SkillGlacialShard::SkillGlacialShard() : SkillImplRecursiveDamageSplash(AT_GLACIER_SHARD) {
}

void SkillGlacialShard::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 5500 + 300 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_ICE)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 12 * sstatus->spl;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_ICE
	RE_LVL_DMOD(100);
}

void SkillGlacialShard::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);

	SkillGlacialNova skillnova;
	skillnova.castendPos2(src, 0, 0, 1, tick, flag);
}
