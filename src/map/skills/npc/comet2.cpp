// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "comet2.hpp"

#include <common/utils.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillComet2::SkillComet2() : SkillImpl(NPC_COMET) {
}

void SkillComet2::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start4(src,target,SC_BURNING,100,skill_lv,1000,src->id,0,skill_get_time2(getSkillId(),skill_lv));
}

void SkillComet2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	int32 i = (sc ? distance_xy(target->x, target->y, sc->comet_x, sc->comet_y) : 8) / 2;
	i = cap_value(i, 1, 4);
	base_skillratio = 2500 + ((skill_lv - i + 1) * 500);
}

void SkillComet2::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1; // Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
