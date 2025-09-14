// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stonefling.hpp"

#include "map/status.hpp"
#include "map/clif.hpp"

SkillStoneFling::SkillStoneFling() : SkillImpl(TF_THROWSTONE) {
}

void SkillStoneFling::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (sd != nullptr) {
		// Only blind if used by player and stun failed
		if (!sc_start(src, target, SC_STUN, 3, skill_lv, skill_get_time(this->skill_id, skill_lv)))
			sc_start(src, target, SC_BLIND, 3, skill_lv, skill_get_time2(this->skill_id, skill_lv));
	} else {
		// 5% stun chance and no blind chance when used by monsters
		sc_start(src, target, SC_STUN, 5, skill_lv, skill_get_time(this->skill_id, skill_lv));
	}
}

void SkillStoneFling::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(skill_get_type(this->skill_id), src, src, target, this->skill_id, skill_lv, tick, flag);
}

void SkillStoneFling::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_THROWSTONE doesn't have a no damage implementation
	return 0;
}