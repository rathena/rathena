// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hellsplant.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

// GN_HELLS_PLANT
SkillHellsPlant::SkillHellsPlant() : StatusSkillImpl(GN_HELLS_PLANT) {
}

void SkillHellsPlant::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}


// GN_HELLS_PLANT_ATK
SkillHellsPlantAttack::SkillHellsPlantAttack() : SkillImplRecursiveDamageSplash(GN_HELLS_PLANT_ATK) {
}

void SkillHellsPlantAttack::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target, SC_STUN,  20 + 10 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
	sc_start2(src,target, SC_BLEEDING, 5 + 5 * skill_lv, skill_lv, src->id,skill_get_time(getSkillId(), skill_lv));
}

void SkillHellsPlantAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 100 * skill_lv + sstatus->int_ * (sd ? pc_checkskill(sd, AM_CANNIBALIZE) : 5); // !TODO: Confirm INT and Cannibalize bonus
	RE_LVL_DMOD(100);
}
