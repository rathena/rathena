// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kunaidistortion.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillKunaiDistortion::SkillKunaiDistortion() : SkillImpl(SS_KUNAIWAIKYOKU) {
}

void SkillKunaiDistortion::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillKunaiDistortion::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 300 + 600 * skill_lv;
	skillratio += pc_checkskill( sd, SS_KUNAIKUSSETSU ) * 10 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
	if (wd->miscflag & SKILL_ALTDMG_FLAG)
		skillratio = skillratio * 3 / 10;
}

void SkillKunaiDistortion::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillKunaiDistortion::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_mirage_cast(*src, nullptr, getSkillId(), skill_lv, x, y, tick, flag | BCT_WOS);
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, UNIT_NOCONSUME_AMMO);
}
