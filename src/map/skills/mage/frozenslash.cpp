// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "frozenslash.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFrozenSlash::SkillFrozenSlash() : SkillImplRecursiveDamageSplash(AG_FROZEN_SLASH) {
}

void SkillFrozenSlash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 450 + 950 * skill_lv + 5 * sstatus->spl;

	if( sc != nullptr && sc->getSCE( SC_CLIMAX ) ){
		skillratio += 150 + 350 * skill_lv;
	}

	RE_LVL_DMOD(100);
}

void SkillFrozenSlash::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_castend_damage_id(src, target, getSkillId(), skill_lv, tick, flag);
}
