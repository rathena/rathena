// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "venomsplasher.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillVenomSplasher::SkillVenomSplasher() : SkillImplRecursiveDamageSplash(AS_SPLASHER) {
}

void SkillVenomSplasher::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST( BL_PC, src );

#ifdef RENEWAL
	base_skillratio += -100 + 400 + 100 * skill_lv;
#else
	base_skillratio += 400 + 50 * skill_lv;
#endif
	if(sd)
		base_skillratio += 20 * pc_checkskill(sd,AS_POISONREACT);
}

void SkillVenomSplasher::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( status_has_mode(tstatus,MD_STATUSIMMUNE)
	// Renewal dropped the 3/4 hp requirement
#ifndef RENEWAL
		|| tstatus-> hp > tstatus->max_hp*3/4
#endif
			) {
		if (sd) {
			clif_skill_fail( *sd, getSkillId() );
		}
		return;
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start4(src,target,type,100,skill_lv,getSkillId(),src->id,skill_get_time(getSkillId(),skill_lv),1000));
}

void SkillVenomSplasher::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1){
		status_change* tsc = status_get_sc(target);

		// Recursive invocation
		int32 sflag = skill_area_temp[0] & 0xFFF;
		std::bitset<INF2_MAX> inf2 = skill_db.find(getSkillId())->inf2;

		if (tsc && tsc->getSCE(SC_HOVERING) && inf2[INF2_IGNOREHOVERING])
			return; // Under Hovering characters are immune to select trap and ground target skills.

		if (flag & SD_LEVEL)
			sflag |= SD_LEVEL; // -1 will be used in packets instead of the skill level
		if (skill_area_temp[1] != target->id && !inf2[INF2_ISNPC])
			sflag |= SD_ANIMATION; // original target gets no animation (as well as all NPC skills)

		SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, sflag);
	}else{
		skill_area_temp[0] = 0;
		skill_area_temp[1] = target->id;
		skill_area_temp[2] = 0;

		// if skill damage should be split among targets, count them
		// SD_LEVEL -> Forced splash damage -> count targets
		// Venom Splasher uses a different range for searching than for splashing
		if (flag & SD_LEVEL || skill_get_nk(getSkillId(), NK_SPLASHSPLIT)){
			skill_area_temp[0] = map_foreachinallrange(skill_area_sub, target, 1, BL_CHAR, src, getSkillId(), skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
			// If there are no characters in the area, then it always counts as if there was one target
			// This happens when targetting skill units such as icewall
			skill_area_temp[0] = std::max(1, skill_area_temp[0]);
		}

		// recursive invocation of skill_castend_damage_id() with flag|1
		map_foreachinrange(skill_area_sub, target, SkillImplRecursiveDamageSplash::getSplashSearchSize(skill_lv), SkillImplRecursiveDamageSplash::getSplashTarget(src), src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);

		flag |= SKILL_NOCONSUME_REQ;
	}
}

void SkillVenomSplasher::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_POISON, 100, skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv));
}
