// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "doublebowlingbash.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDoubleBowlingBash::SkillDoubleBowlingBash() : SkillImpl(HN_DOUBLEBOWLINGBASH) {
}

void SkillDoubleBowlingBash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 250 + 400 * skill_lv;
	skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 3 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillDoubleBowlingBash::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1) {
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, skill_area_temp[0] & 0xFFF);
	} else {
		int32 splash = skill_get_splash(getSkillId(), skill_lv);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		skill_area_temp[0] = map_foreachinallrange(skill_area_sub, target, splash, BL_CHAR, src, getSkillId(), skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
		map_foreachinrange(skill_area_sub, target, splash, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		sc_start(src, src, SC_HNNOWEAPON, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	}
}
