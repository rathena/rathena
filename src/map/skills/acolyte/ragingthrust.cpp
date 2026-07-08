// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ragingthrust.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillRagingThrust::SkillRagingThrust() : WeaponSkillImpl(MO_COMBOFINISH) {
}

void SkillRagingThrust::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag&1) && sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_MONK)
	{	//Becomes a splash attack when Soul Linked.
		map_foreachinshootrange(skill_area_sub, target,
			skill_get_splash(getSkillId(), skill_lv),BL_CHAR|BL_SKILL,
			src,getSkillId(),skill_lv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
	} else
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillRagingThrust::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += 450 + 50 * skill_lv + sstatus->str; // !TODO: How does STR play a role?
#else
	base_skillratio += 140 + 60 * skill_lv;
#endif

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->getSCE(SC_GT_ENERGYGAIN))
		base_skillratio += base_skillratio * 50 / 100;
}
