// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fallingstar.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

// SJ_FALLINGSTAR
SkillFallingStar::SkillFallingStar() : StatusSkillImpl(SJ_FALLINGSTAR) {
}


// SJ_FALLINGSTAR_ATK
SkillFallingStarAttack::SkillFallingStarAttack() : WeaponSkillImpl(SJ_FALLINGSTAR_ATK) {
}

void SkillFallingStarAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	skillratio += 100 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_LIGHTOFSTAR))
		skillratio += skillratio * sc->getSCE(SC_LIGHTOFSTAR)->val2 / 100;
}

void SkillFallingStarAttack::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) { // If a player used the skill it will search for targets marked by that player. 
		if (tsc && tsc->getSCE(SC_FLASHKICK) && tsc->getSCE(SC_FLASHKICK)->val4 == 1) { // Mark placed by a player.
			int8 i = 0;

			ARR_FIND(0, MAX_STELLAR_MARKS, i, sd->stellar_mark[i] == target->id);
			if (i < MAX_STELLAR_MARKS) {
				WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);

				skill_castend_damage_id(src, target, SJ_FALLINGSTAR_ATK2, skill_lv, tick, 0);
			}
		}
	} else if ( tsc && tsc->getSCE(SC_FLASHKICK) && tsc->getSCE(SC_FLASHKICK)->val4 == 2 ) { // Mark placed by a monster.
		// If a monster used the skill it will search for targets marked by any monster since they can't track their own targets.
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);

		skill_castend_damage_id(src, target, SJ_FALLINGSTAR_ATK2, skill_lv, tick, 0);
	}
}

void SkillFallingStarAttack::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}


// SJ_FALLINGSTAR_ATK2
SkillFallingStarAttack2::SkillFallingStarAttack2() : SkillImplRecursiveDamageSplash(SJ_FALLINGSTAR_ATK2) {
}

void SkillFallingStarAttack2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	skillratio += 100 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_LIGHTOFSTAR))
		skillratio += skillratio * sc->getSCE(SC_LIGHTOFSTAR)->val2 / 100;
}
