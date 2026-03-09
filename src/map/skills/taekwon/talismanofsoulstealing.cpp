// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "talismanofsoulstealing.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTalismanOfSoulStealing::SkillTalismanOfSoulStealing() : SkillImpl(SOA_TALISMAN_OF_SOUL_STEALING) {
}

void SkillTalismanOfSoulStealing::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 500 + 1250 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_TALISMAN_MASTERY) * 7 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_SOUL_MASTERY) * 7 * skill_lv;
	skillratio += 3 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillTalismanOfSoulStealing::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	if( target->type != BL_SKILL ){
		int32 sp = (100 + status_get_lv(src) / 50) * skill_lv;

		status_heal(src, 0, sp, 0, 0);
		clif_skill_nodamage( src, *src, getSkillId(), sp );
	}
}
