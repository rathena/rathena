// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "turnkick.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillTurnKick::SkillTurnKick() : SkillImpl(TK_TURNKICK) {
}

void SkillTurnKick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 90 + 30 * skill_lv;
}

void SkillTurnKick::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Note: attack_type is passed as BF_WEAPON for the actual target, BF_MISC for the splash-affected mobs.
	if (attack_type & BF_MISC) {
		sc_start(src, target, SC_STUN, 200, skill_lv, skill_get_time(getSkillId(), skill_lv));
		clif_specialeffect(target, EF_SPINEDBODY, AREA);
		sc_start(src, target, SC_NOACTION, 100, 1, skill_get_time2(getSkillId(), skill_lv));
	}
}

void SkillTurnKick::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	// Active part of the attack.
	// Note: skill_area_temp[1] is used in castendNoDamageId to avoid affecting the target.
	skill_area_temp[1] = target->id;

	if (skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag))
		map_foreachinallrange(skill_area_sub, target,
		                      skill_get_splash(getSkillId(), skill_lv), BL_MOB,
		                      src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1,
		                      skill_castend_nodamage_id);
}

void SkillTurnKick::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	// Passive part of the attack. Splash knock-back+stun.
	if (skill_area_temp[1] != target->id) {
		skill_blown(src, target, skill_get_blewcount(getSkillId(), skill_lv), -1, BLOWN_NONE);
		skill_additional_effect(src, target, getSkillId(), skill_lv, BF_MISC, ATK_DEF, tick); // Use Misc rather than weapon to signal passive pushback
	}
}
