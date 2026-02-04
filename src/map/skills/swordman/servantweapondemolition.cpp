// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "servantweapondemolition.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillServantWeaponDemolition::SkillServantWeaponDemolition() : SkillImplRecursiveDamageSplash(DK_SERVANT_W_DEMOL) {
}

void SkillServantWeaponDemolition::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}

void SkillServantWeaponDemolition::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 500 * skill_lv;
	RE_LVL_DMOD(100);
}

int64 SkillServantWeaponDemolition::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	status_change* tsc = status_get_sc(target);

	// Servant Weapon - Demol only hits if the target is marked with a sign by the attacking caster.
	if (!(tsc && tsc->getSCE(SC_SERVANT_SIGN) && tsc->getSCE(SC_SERVANT_SIGN)->val1 == src->id))
		return 0;

	return SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
}
