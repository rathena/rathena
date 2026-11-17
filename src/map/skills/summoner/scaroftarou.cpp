// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "scaroftarou.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillScarofTarou::SkillScarofTarou() : WeaponSkillImpl(SU_SCAROFTAROU) {
}

void SkillScarofTarou::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 10, skill_lv, skill_get_time2(getSkillId(), skill_lv)); //! TODO: What's the chance/time?
}

void SkillScarofTarou::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	base_skillratio += -100 + 100 * skill_lv;
	if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
		base_skillratio += base_skillratio * status_get_hp(src) / status_get_max_hp(src);
}

void SkillScarofTarou::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_start(src, target, SC_BITESCAR, 10, skill_lv, skill_get_time(getSkillId(), skill_lv)); //! TODO: What's the activation chance for the Bite effect?

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
