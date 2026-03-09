// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "envenom.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEnvenom::SkillEnvenom() : WeaponSkillImpl(TF_POISON) {
}

void SkillEnvenom::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data *sd = BL_CAST(BL_PC, src);
	if (!sc_start2(src, target, SC_POISON, (4 * skill_lv + 10), skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv)) && sd)
		clif_skill_fail(*sd, getSkillId());
}
