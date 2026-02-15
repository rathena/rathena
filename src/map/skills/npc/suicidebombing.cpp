// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "suicidebombing.hpp"

SkillSuicideBombing::SkillSuicideBombing() : SkillImpl(NPC_SELFDESTRUCTION) {
}

void SkillSuicideBombing::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* tsc = status_get_sc(target);

	if (tsc != nullptr && tsc->getSCE(SC_HIDING))
		return;

	if (src != target)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillSuicideBombing::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);
	int32 target_flag;

	// Self Destruction hits everyone in range (allies+enemies)
	// Except for summoned Marine Spheres on non-versus maps, where it's just enemies and your own slaves.
	if ((md == nullptr || md->special_state.ai == AI_SPHERE) && !map_flag_vs(src->m)) {
		// Enable Marine Spheres to damage own Homunculus and summons outside PVP.
		if (battle_config.alchemist_summon_setting & 8)
			target_flag = BCT_ENEMY | BCT_SLAVE;
		else
			target_flag = BCT_ENEMY;
	} else {
		target_flag = BCT_ALL;
	}

	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	map_delblock(src); // Prevent chain self-destructions from hitting back.
	map_foreachinshootrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL,
		src, getSkillId(), skill_lv, tick, flag | target_flag, skill_castend_damage_id);

	if (map_addblock(src))
		return;

	// Won't display the damage, but drops items and gives exp.
	status_data* sstatus = status_get_status_data(*src);
	status_zap(src, sstatus->hp, 0, 0);
}
