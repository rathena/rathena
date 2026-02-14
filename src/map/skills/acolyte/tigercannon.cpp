// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tigercannon.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillTigerCannon::SkillTigerCannon() : WeaponSkillImpl(SR_TIGERCANNON) {
}

void SkillTigerCannon::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);

	if (flag & 1) {
		skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag | SD_ANIMATION);
	} else if (sd != nullptr) {
		if (sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SR_FALLENEMPIRE && !sc->getSCE(SC_FLASHCOMBO))
			flag |= 8; // Only apply Combo bonus when Tiger Cannon is not used through Flash Combo
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
	}
}

void SkillTigerCannon::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);
	uint32 hp = sstatus->max_hp * (10 + (skill_lv * 2)) / 100;
	uint32 sp = sstatus->max_sp * (5 + skill_lv) / 100;

	if (wd->miscflag & 8)
		// Base_Damage = [((Caster consumed HP + SP) / 2) x Caster Base Level / 100] %
		skillratio += -100 + (hp + sp) / 2;
	else
		// Base_Damage = [((Caster consumed HP + SP) / 4) x Caster Base Level / 100] %
		skillratio += -100 + (hp + sp) / 4;
	RE_LVL_DMOD(100);

	if (sc && sc->getSCE(SC_GT_REVITALIZE))
		skillratio += skillratio * 30 / 100;
}
