// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shatterstorm.hpp"

#include "map/status.hpp"

SkillShatterStorm::SkillShatterStorm() : SkillImplRecursiveDamageSplash(RL_S_STORM) {
}

void SkillShatterStorm::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*target);

	//kRO update 2014-02-12. Break a headgear by minimum chance 5%/10%/15%/20%/25%
	//! TODO: Figure out break chance formula
	skill_break_equip(src, target, EQP_HEAD_TOP, max(skill_lv * 500, (sstatus->dex * skill_lv * 10) - (tstatus->agi * 20)), BCT_ENEMY);
}

void SkillShatterStorm::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 1700 + 200 * skill_lv;
}
