// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lullaby.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillLullaby::SkillLullaby() : SkillImpl(BD_LULLABY) {
}

void SkillLullaby::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
#ifndef RENEWAL
	status_change *sc = status_get_sc(src);
	status_data* sstatus = status_get_status_data(*src);

	if (sc != nullptr && sc->getSCE(SC_DANCING) != nullptr) {
		block_list* partner = map_id2bl(sc->getSCE(SC_DANCING)->val4);
		if (partner == nullptr)
			return;
		status_data* pstatus = status_get_status_data(*partner);
		if (pstatus == nullptr)
			return;
		status_change_start(src, target, skill_get_sc(getSkillId()), (sstatus->int_ + pstatus->int_ + rnd_value(100, 300)) * 10, skill_lv, 0, 0, 0, skill_get_time2(getSkillId(), skill_lv), SCSTART_NONE);
	}
#else
	// In renewal the chance is simply 100% and uses the original song duration as sleep duration
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
#endif
}

void SkillLullaby::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	skill_castend_song(src, getSkillId(), skill_lv, tick);
#endif
}

void SkillLullaby::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifndef RENEWAL
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
#endif
}
