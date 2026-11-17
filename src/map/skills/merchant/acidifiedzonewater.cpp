// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "acidifiedzonewater.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

// BO_ACIDIFIED_ZONE_WATER
SkillAcidifiedZoneWater::SkillAcidifiedZoneWater() : SkillImplRecursiveDamageSplash(BO_ACIDIFIED_ZONE_WATER) {
}

void SkillAcidifiedZoneWater::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);
	const status_change *sc = status_get_sc(src);

	// All BO_ACIDIFIED_ZONE_* deal the same damage? [Rytech]
	skillratio += -100 + 400 * skill_lv + 5 * sstatus->pow;

	if( sc != nullptr && sc->getSCE( SC_RESEARCHREPORT ) ){
		skillratio += skillratio * 50 / 100;

		if (tstatus->race == RC_FORMLESS || tstatus->race == RC_PLANT)
			skillratio += skillratio * 50 / 100;
	}

	RE_LVL_DMOD(100);
}

void SkillAcidifiedZoneWater::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillAcidifiedZoneWater::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	if (target->type == BL_PC)// Place single cell AoE if hitting a player.
		skill_castend_pos2(src, target->x, target->y, getSkillId(), skill_lv, tick, 0);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}


// BO_ACIDIFIED_ZONE_WATER_ATK
SkillActifiedZoneWaterAttack::SkillActifiedZoneWaterAttack() : WeaponSkillImpl(BO_ACIDIFIED_ZONE_WATER_ATK) {
}

void SkillActifiedZoneWaterAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);
	const status_change *sc = status_get_sc(src);

	// All BO_ACIDIFIED_ZONE_* deal the same damage? [Rytech]
	skillratio += -100 + 400 * skill_lv + 5 * sstatus->pow;

	if( sc != nullptr && sc->getSCE( SC_RESEARCHREPORT ) ){
		skillratio += skillratio * 50 / 100;

		if (tstatus->race == RC_FORMLESS || tstatus->race == RC_PLANT)
			skillratio += skillratio * 50 / 100;
	}

	RE_LVL_DMOD(100);
}
