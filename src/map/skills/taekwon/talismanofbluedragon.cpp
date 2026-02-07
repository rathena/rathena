// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "talismanofbluedragon.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTalismanOfBlueDragon::SkillTalismanOfBlueDragon() : SkillImpl(SOA_TALISMAN_OF_BLUE_DRAGON) {
}

void SkillTalismanOfBlueDragon::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 850 + 2250 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_TALISMAN_MASTERY) * 15 * skill_lv;
	skillratio += 5 * sstatus->spl;
	if (sc != nullptr && sc->getSCE(SC_T_FIFTH_GOD) != nullptr)
		skillratio += 100 + 700 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillTalismanOfBlueDragon::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
	sc_start(src,src,skill_get_sc(getSkillId()), 100, 1, skill_get_time(getSkillId(), skill_lv));
}
