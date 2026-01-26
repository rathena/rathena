// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "frenzyfang.hpp"

#include "map/clif.hpp"

#include "skill_factory_druid.hpp"

SkillFrenzyFang::SkillFrenzyFang() : SkillImplRecursiveDamageSplash(AT_FRENZY_FANG) {
}

void SkillFrenzyFang::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	if (!(flag & 1)) {
		SkillFactoryDruid::try_gain_madness(src);
	}
}

void SkillFrenzyFang::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	const bool madness = sc && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3));

	int32 skillratio = (madness ? 1750 : 1000) + 250 * (skill_lv - 1);
	skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}
