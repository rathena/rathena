// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soundblend.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSoundBlend::SkillSoundBlend() : SkillImpl(TR_SOUNDBLEND) {
}

void SkillSoundBlend::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_castend_damage_id(src, target, getSkillId(), skill_lv, tick, 0);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start2(src, target, skill_get_sc(getSkillId()), 100, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv)));
}

void SkillSoundBlend::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillSoundBlend::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 120 * skill_lv + 5 * sstatus->spl;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_MYSTIC_SYMPHONY)) {
		skillratio += skillratio * 100 / 100;

		if (tstatus->race == RC_FISH || tstatus->race == RC_DEMIHUMAN)
			skillratio += skillratio * 50 / 100;
	}
}
