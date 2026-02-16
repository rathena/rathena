// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fatalmenace.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillFatalMenace::SkillFatalMenace() : WeaponSkillImpl(SC_FATALMENACE) {
}

void SkillFatalMenace::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += 120 * skill_lv + sstatus->agi; // !TODO: What's the AGI bonus?

	if( sc != nullptr && sc->getSCE( SC_ABYSS_DAGGER ) ){
		skillratio += 30 * skill_lv;
	}

	RE_LVL_DMOD(100);
}

void SkillFatalMenace::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if( flag&1 )
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	else {
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), splash_target(src), src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
		clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	}
}

void SkillFatalMenace::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	if (skill_lv < 6)
		hit_rate -= 35 - 5 * skill_lv;
	else if (skill_lv > 6)
		hit_rate += 5 * skill_lv - 30;
}
