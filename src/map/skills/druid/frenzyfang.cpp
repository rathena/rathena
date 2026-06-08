// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "frenzyfang.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

#include "pulseofmadness.hpp"

SkillFrenzyFang::SkillFrenzyFang() : WeaponSkillImpl(AT_FRENZY_FANG) {
}

void SkillFrenzyFang::applyCounterAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& attack_type) const {
	// Update madness when the skill hits the target
	SkillPulseOfMadness::updateMadness(src);
}

void SkillFrenzyFang::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillFrenzyFang::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1000 + 250 * (skill_lv - 1);

	if (sc != nullptr && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3))) {
		skillratio += 750;
	}
	skillratio += sstatus->pow * 4;

	RE_LVL_DMOD(100);
}

void SkillFrenzyFang::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change *sc = status_get_sc(&src);

	int32 hits = 2;

	if (sc != nullptr) {
		if (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE3)) {
			hits = 7;
		} else if (sc->hasSCE(SC_INSANE2)) {
			hits = 5;
		} else if (sc->hasSCE(SC_INSANE)) {
			hits = 3;
		}
	}

	dmg.div_ = hits;
}
