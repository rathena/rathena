// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "roseblossom.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRoseBlossom::SkillRoseBlossom() : WeaponSkillImpl(TR_ROSEBLOSSOM) {
}

void SkillRoseBlossom::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillRoseBlossom::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_change* tsc = status_get_sc(target);
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 200 + 2000 * skill_lv;

	if (sd && pc_checkskill(sd, TR_STAGE_MANNER) > 0)
		skillratio += 3 * sstatus->con;

	if( tsc != nullptr && tsc->getSCE( SC_SOUNDBLEND ) ){
		skillratio += 200 * skill_lv;
	}

	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_MYSTIC_SYMPHONY)) {
		skillratio *= 2;

		if (tstatus->race == RC_FISH || tstatus->race == RC_DEMIHUMAN)
			skillratio += skillratio * 50 / 100;
	}
}

void SkillRoseBlossom::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Rose blossom seed can only bloom if the target is hit.
	sc_start4(src, target, SC_ROSEBLOSSOM, 100, skill_lv, TR_ROSEBLOSSOM_ATK, src->id, 0, skill_get_time(getSkillId(), skill_lv));
	status_change_end(target, SC_SOUNDBLEND);
}

SkillRoseBlossomAttack::SkillRoseBlossomAttack() : SkillImplRecursiveDamageSplash(TR_ROSEBLOSSOM_ATK) {
}

void SkillRoseBlossomAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_change* tsc = status_get_sc(target);
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 250 + 2800 * skill_lv;

	if (sd && pc_checkskill(sd, TR_STAGE_MANNER) > 0)
		skillratio += 3 * sstatus->con;

	if (tsc != nullptr && tsc->getSCE(SC_SOUNDBLEND)) {
		skillratio += 200 * skill_lv;
	}

	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_MYSTIC_SYMPHONY)) {
		skillratio *= 2;

		if (tstatus->race == RC_FISH || tstatus->race == RC_DEMIHUMAN)
			skillratio += skillratio * 50 / 100;
	}
}
