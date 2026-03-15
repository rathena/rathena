// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_bioexplosion.hpp"

#include "map/clif.hpp"
#include "map/homunculus.hpp"
#include "map/map.hpp"

SkillBioExplosion::SkillBioExplosion() : SkillImpl(HVAN_EXPLOSION) {
}

void SkillBioExplosion::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	homun_data* hd = BL_CAST(BL_HOM, src);

	if (hd != nullptr) {
		clif_skill_nodamage(src, *src, getSkillId(), skill_lv, 1);
		map_foreachinshootrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY, skill_castend_damage_id);

		hd->homunculus.intimacy = hom_intimacy_grade2intimacy(HOMGRADE_HATE_WITH_PASSION);
		clif_send_homdata(*hd, SP_INTIMATE);

		// There's a delay between the explosion and the homunculus death
		skill_addtimerskill(src, tick + skill_get_time(getSkillId(), skill_lv), src->id, 0, 0, getSkillId(), skill_lv, 0, flag);
	}
}

void SkillBioExplosion::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (src != target) {
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	}
}
