// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "darkeningcannon.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillDarkeningCannon::SkillDarkeningCannon() : SkillImpl(SS_ANTENPOU) {
}

void SkillDarkeningCannon::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillDarkeningCannon::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 450 + 950 * skill_lv;
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
	if (mflag & SKILL_ALTDMG_FLAG)
		skillratio = skillratio * 3 / 10;
}

void SkillDarkeningCannon::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillDarkeningCannon::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_mirage_cast(*src, nullptr,getSkillId(), skill_lv, 0, 0, tick, flag | BCT_WOS);
	int32 range = skill_get_splash( getSkillId(), skill_lv );

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	map_foreachinrange( skill_area_sub, target, range, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id );
}
