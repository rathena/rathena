#include "skill_nj_zenynage.hpp"

#include "../../skill.hpp"

SkillNJ_ZENYNAGE::SkillNJ_ZENYNAGE() : SkillImpl(NJ_ZENYNAGE) {
}

void SkillNJ_ZENYNAGE::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(skill_get_type(this->skill_id_), src, src, target, this->skill_id_, skill_lv, tick, flag);
}
