// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firestorm.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillFireStorm::SkillFireStorm() : SkillImpl(NPC_FIRESTORM) {
}

void SkillFireStorm::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_BURNT,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
}

void SkillFireStorm::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 200;
}

void SkillFireStorm::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillFireStorm::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 sflag = flag;

	if( skill_lv > 1 )
		sflag |= 4;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinshootrange(skill_area_sub,src,skill_get_splash(getSkillId(),skill_lv),splash_target(src),src,
		getSkillId(),skill_lv,tick,sflag|BCT_ENEMY|SD_ANIMATION|1,skill_castend_damage_id);
}
