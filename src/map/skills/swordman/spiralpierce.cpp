// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiralpierce.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSpiralPierce::SkillSpiralPierce() : WeaponSkillImpl(LK_SPIRALPIERCE) {
}

void SkillSpiralPierce::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	const status_change *sc = status_get_sc(src);

	skillratio += 50 + 50 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_CHARGINGPIERCE_COUNT) && sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 >= 10)
		skillratio *= 2;
#endif
}

void SkillSpiralPierce::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data *dstsd = BL_CAST(BL_PC, target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	if( dstsd || ( dstmd && !status_bl_has_mode(target,MD_STATUSIMMUNE) ) ) //Does not work on status immune
		sc_start(src,target,SC_ANKLE,100,0,skill_get_time2(getSkillId(),skill_lv));
}
