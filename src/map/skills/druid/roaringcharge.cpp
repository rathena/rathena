// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "roaringcharge.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

// AT_ROARING_CHARGE
SkillRoaringCharge::SkillRoaringCharge() : SkillImplRecursiveDamageSplash(AT_ROARING_CHARGE) {
}

void SkillRoaringCharge::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 8000 + 400 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 4 * sstatus->spl;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_WIND
	RE_LVL_DMOD(100);
}

void SkillRoaringCharge::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_THUNDERING_ROD_MAX)) {
		SkillRoaringChargeS skill_overcharged;
		skill_overcharged.castendNoDamageId(src, target, skill_lv, tick, flag);
		return;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillFactoryDruid::addThunderingCharge(src, getSkillId(), skill_lv, skill_lv + 1);
	this->castendDamageId(src, target, skill_lv, tick, flag);
}


// AT_ROARING_CHARGE_S
SkillRoaringChargeS::SkillRoaringChargeS() : SkillImplRecursiveDamageSplash(AT_ROARING_CHARGE_S) {
}

void SkillRoaringChargeS::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 11500 + 500 * (skill_lv - 1);

	// SPL and BaseLevel ratio do not depend on SC_TRUTH_OF_WIND
	skillratio += 7 * sstatus->spl;

	RE_LVL_DMOD(100);
}

void SkillRoaringChargeS::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	status_change_end(src, SC_THUNDERING_ROD);
	status_change_end(src, SC_THUNDERING_ROD_MAX);

	// The skill re-starts SC_THUNDERING_ROD_MAX
	sc_start4(src, src, SC_THUNDERING_ROD_MAX, 100, getSkillId(), skill_lv, 0, 0, skill_get_time(getSkillId(), skill_lv));

	this->castendDamageId(src, target, skill_lv, tick, flag);
}
