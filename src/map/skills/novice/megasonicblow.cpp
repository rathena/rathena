// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "megasonicblow.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMegaSonicBlow::SkillMegaSonicBlow() : WeaponSkillImpl(HN_MEGA_SONIC_BLOW) {
}

void SkillMegaSonicBlow::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, (2 * skill_lv + 10), skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillMegaSonicBlow::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 900 + 750 * skill_lv;
	skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 5 * skill_lv;
	skillratio += 5 * sstatus->pow;
	if (status_get_hp(target) < status_get_max_hp(target) / 2)
		skillratio *= 2;
	RE_LVL_DMOD(100);
}

void SkillMegaSonicBlow::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
