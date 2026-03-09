// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shieldchainrush.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillShieldChainRush::SkillShieldChainRush() : WeaponSkillImpl(HN_SHIELD_CHAIN_RUSH) {
}

void SkillShieldChainRush::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, skill_get_sc(getSkillId()), 100, 0, skill_get_time2(getSkillId(), skill_lv));
}

void SkillShieldChainRush::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 850 + 1050 * skill_lv;
	skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 3 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillShieldChainRush::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1) {
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	} else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		sc_start(src, src, SC_HNNOWEAPON, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	}
}
