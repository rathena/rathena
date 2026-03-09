// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiralpiercemax.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSpiralPierceMax::SkillSpiralPierceMax() : WeaponSkillImpl(HN_SPIRAL_PIERCE_MAX) {
}

void SkillSpiralPierceMax::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* dstsd = BL_CAST( BL_PC, target );
	mob_data *dstmd = BL_CAST(BL_MOB, target);

	if( dstsd || ( dstmd && !status_bl_has_mode(target,MD_STATUSIMMUNE) ) ) //Does not work on status immune
		sc_start(src,target,SC_ANKLE,100,0,skill_get_time2(getSkillId(),skill_lv));
}

void SkillSpiralPierceMax::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 1000 + 1500 * skill_lv;
	skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 3 * skill_lv;
	skillratio += 5 * sstatus->pow;
	switch (status_get_size(target)){
		case SZ_SMALL:
			skillratio = skillratio * 150 / 100;
			break;
		case SZ_MEDIUM:
			skillratio = skillratio * 130 / 100;
			break;
		case SZ_BIG:
			skillratio = skillratio * 120 / 100;
			break;
	}
	RE_LVL_DMOD(100);
}

void SkillSpiralPierceMax::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
