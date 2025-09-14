// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fling.hpp"

SkillFling::SkillFling() : SkillImpl(GS_FLING) {
}

void SkillFling::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_FLING, skill_lv, tick, flag);
}

void SkillFling::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	const struct map_session_data *sd = BL_CAST(BL_PC, src);
	sc_start(src, target, SC_FLING, 100, sd ? sd->spiritball_old : 5, skill_get_time(GS_FLING, skill_lv));
}