// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kunainightmare.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillKunaiNightmare::SkillKunaiNightmare() : SkillImpl(SS_HITOUAKUMU) {
}

void SkillKunaiNightmare::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change_end(target, SC_NIGHTMARE);
}

void SkillKunaiNightmare::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *tsc = status_get_sc(target);

	skillratio += -100 + 18000 + 5 * sstatus->pow;

	if( tsc != nullptr && tsc->getSCE( SC_NIGHTMARE ) != nullptr ){
		skillratio += skillratio / 2;
	}

	RE_LVL_DMOD(100);
}

void SkillKunaiNightmare::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillKunaiNightmare::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 range = skill_get_splash( getSkillId(), skill_lv );

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	map_foreachinrange( skill_area_sub, target, range, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id );
}
