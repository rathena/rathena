// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "envenom.hpp"

#include "map/status.hpp"
#include "map/clif.hpp"

SkillEnvenom::SkillEnvenom() : WeaponSkillImpl(TF_POISON) {
}


void SkillEnvenom::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (!sc_start2(src, target, SC_POISON, (4 * skill_lv + 10), skill_lv, src->id, skill_get_time2(this->skill_id, skill_lv)) && sd)
		clif_skill_fail(*sd, this->skill_id);
}

void SkillEnvenom::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag);
}

int32 SkillEnvenom::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_POISON doesn't have a no damage implementation
	return 0;
}