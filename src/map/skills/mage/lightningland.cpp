// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lightningland.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillLightningLand::SkillLightningLand() : SkillImpl(EM_LIGHTNING_LAND) {
}

void SkillLightningLand::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_HANDICAPSTATE_LIGHTNINGSTRIKE, 3, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillLightningLand::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 700 + 1100 * skill_lv;
	skillratio += 5 * sstatus->spl;

	if( sc != nullptr && sc->getSCE( SC_SUMMON_ELEMENTAL_PROCELLA ) ){
		skillratio += 200 * skill_lv;
		skillratio += 2 * sstatus->spl;
	}

	RE_LVL_DMOD(100);
}

void SkillLightningLand::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
