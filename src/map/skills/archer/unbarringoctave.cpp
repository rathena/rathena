// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "unbarringoctave.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

SkillUnbarringOctave::SkillUnbarringOctave() : SkillImpl(BA_FROSTJOKER) {
}

void SkillUnbarringOctave::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	int32 rate = 150 + 50 * skill_lv; // Aegis accuracy (1000 = 100%)
	int32 duration = skill_get_time2(getSkillId(), skill_lv);
	if (battle_check_target(src, target, BCT_PARTY) > 0) {
		// On party members: Chance is divided by 4 and duration is fixed to 15000ms
		rate /= 4;
		duration = skill_get_time(getSkillId(), skill_lv);
	}
	status_change_start(src, target, skill_get_sc(getSkillId()), rate*10, skill_lv, 0, 0, 0, duration, SCSTART_NONE);
}

void SkillUnbarringOctave::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data *md = BL_CAST(BL_MOB, src);

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_addtimerskill(src,tick+3000,target->id,src->x,src->y,getSkillId(),skill_lv,0,flag);

	if (md) {
		// custom hack to make the mob display the skill, because these skills don't show the skill use text themselves
		//NOTE: mobs don't have the sprite animation that is used when performing this skill (will cause glitches)
		char temp[70];
		snprintf(temp, sizeof(temp), "%s : %s !!",md->name,skill_get_desc(getSkillId()));
		clif_disp_overhead(md,temp);
	}
}
