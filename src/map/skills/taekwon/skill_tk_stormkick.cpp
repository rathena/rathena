#include "skill_tk_stormkick.hpp"

SkillTK_STORMKICK::SkillTK_STORMKICK() : WeaponSkillImpl(TK_STORMKICK) {
}

void SkillTK_STORMKICK::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 60 + 20 * skill_lv;
}

void SkillTK_STORMKICK::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, this->skill_id, skill_lv);
	skill_area_temp[1] = 0;
	map_foreachinshootrange(skill_attack_area, src,
							skill_get_splash(this->skill_id, skill_lv), BL_CHAR | BL_SKILL,
							BF_WEAPON, src, src, this->skill_id, skill_lv, tick, flag, BCT_ENEMY);
}
