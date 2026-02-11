// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulexplosion.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSoulExplosion::SkillSoulExplosion() : SkillImpl(SP_SOULEXPLOSION) {
}

void SkillSoulExplosion::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Remove soul link when hit.
	status_change_end(target, SC_SPIRIT);
	status_change_end(target, SC_SOULGOLEM);
	status_change_end(target, SC_SOULSHADOW);
	status_change_end(target, SC_SOULFALCON);
	status_change_end(target, SC_SOULFAIRY);
}

void SkillSoulExplosion::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (!(tsc && (tsc->getSCE(SC_SPIRIT) || tsc->getSCE(SC_SOULGOLEM) || tsc->getSCE(SC_SOULSHADOW) || tsc->getSCE(SC_SOULFALCON) || tsc->getSCE(SC_SOULFAIRY))) || tstatus->hp < 10 * tstatus->max_hp / 100) { // Requires target to have a soul link and more then 10% of MaxHP.
		// With this skill requiring a soul link, and the target to have more then 10% if MaxHP, I wonder
		// if the cooldown still happens after it fails. Need a confirm. [Rytech] 
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
		return;
	}

	skill_attack(BF_MISC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
