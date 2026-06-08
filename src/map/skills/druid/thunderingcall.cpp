// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thunderingcall.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

// KR_THUNDERING_CALL
SkillThunderingCall::SkillThunderingCall() : SkillImplRecursiveDamageSplash(KR_THUNDERING_CALL) {
}

void SkillThunderingCall::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 5200 + 200 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 8 * sstatus->int_;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_WIND
	RE_LVL_DMOD(100);
}

void SkillThunderingCall::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		if (status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_THUNDERING_ROD_MAX)) {
			SkillThunderingCallS skill_overcharged;
			skill_overcharged.castendDamageId(src, target, skill_lv, tick, flag);
			return;
		}

		SkillFactoryDruid::addThunderingCharge(src, getSkillId(), skill_lv, 1);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}


// KR_THUNDERING_CALL_S
SkillThunderingCallS::SkillThunderingCallS() : SkillImplRecursiveDamageSplash(KR_THUNDERING_CALL_S) {
}

void SkillThunderingCallS::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 9500 + 500 * (skill_lv - 1);

	// INT and BaseLevel ratio do not depend on SC_TRUTH_OF_WIND
	skillratio += 15 * sstatus->int_;

	RE_LVL_DMOD(100);
}

void SkillThunderingCallS::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	status_change_end(src, SC_THUNDERING_ROD);
	status_change_end(src, SC_THUNDERING_ROD_MAX);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
