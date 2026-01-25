// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shootingfeather.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillShootingFeather::SkillShootingFeather() : SkillImplRecursiveDamageSplash(DR_SHOOTING_FEATHER) {
}

void SkillShootingFeather::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillShootingFeather::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 20 * skill_lv;
	if (sc && sc->getSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 20 * skill_lv;
	}
	skillratio += sstatus->dex; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}
