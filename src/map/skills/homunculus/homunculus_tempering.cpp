// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_tempering.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/status.hpp"

SkillTempering::SkillTempering() : SkillImpl(MH_TEMPERING) {
}

void SkillTempering::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const sc_type type = skill_get_sc(getSkillId());
	block_list* master_bl = battle_get_master(src);

	if (master_bl != nullptr) {
		clif_skill_nodamage(src, *master_bl, getSkillId(), skill_lv);
		sc_start(src, master_bl, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}
}
