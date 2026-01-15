// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "venomsplasher.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillVenomSplasher::SkillVenomSplasher() : SkillImplRecursiveDamageSplash(AS_SPLASHER) {
}

void SkillVenomSplasher::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST( BL_PC, src );

#ifdef RENEWAL
	base_skillratio += -100 + 400 + 100 * skill_lv;
#else
	base_skillratio += 400 + 50 * skill_lv;
#endif
	if(sd)
		base_skillratio += 20 * pc_checkskill(sd,AS_POISONREACT);
}

void SkillVenomSplasher::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( status_has_mode(tstatus,MD_STATUSIMMUNE)
	// Renewal dropped the 3/4 hp requirement
#ifndef RENEWAL
		|| tstatus-> hp > tstatus->max_hp*3/4
#endif
			) {
		if (sd) {
			clif_skill_fail( *sd, getSkillId() );
		}
		return;
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start4(src,target,type,100,skill_lv,getSkillId(),src->id,skill_get_time(getSkillId(),skill_lv),1000));
}

void SkillVenomSplasher::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	if (!(flag & 1)) {
		// Don't consume a second gemstone.
		flag |= SKILL_NOCONSUME_REQ;
	}
}

int16 SkillVenomSplasher::getSearchSize(uint16 skill_lv) const {
	// Venom Splasher uses a different range for searching than for splashing
	return 1;
}

void SkillVenomSplasher::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_POISON, 100, skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv));
}
