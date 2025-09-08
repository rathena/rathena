#include "skill_gs_spreadattack.hpp"

SkillGS_SPREADATTACK::SkillGS_SPREADATTACK() : WeaponSkillImpl(GS_SPREADATTACK) {
}

void SkillGS_SPREADATTACK::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 30 * skill_lv;
#else
	base_skillratio += 20 * (skill_lv - 1);
#endif
}

void SkillGS_SPREADATTACK::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_SPREADATTACK, skill_lv, tick, flag);
}

void SkillGS_SPREADATTACK::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// This is from skill_castend_nodamage_id.cpp
	int32 starget = BL_CHAR | BL_SKILL;
	
	skill_area_temp[1] = 0;
	clif_skill_nodamage(src, *bl, GS_SPREADATTACK, skill_lv);
	int32 i = map_foreachinrange(skill_area_sub, bl, skill_get_splash(GS_SPREADATTACK, skill_lv), starget,
							   src, GS_SPREADATTACK, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
	if (!i) // Note: We're not checking the specific skill IDs here as in the original
		clif_skill_damage(*src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, GS_SPREADATTACK, skill_lv, DMG_SINGLE);
}
