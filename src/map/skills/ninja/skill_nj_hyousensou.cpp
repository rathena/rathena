#include "skill_nj_hyousensou.hpp"

#include "../../clif.hpp"
#include "../../skill.hpp"
#include "../../unit.hpp"

SkillNJ_HYOUSENSOU::SkillNJ_HYOUSENSOU() : SkillImpl(NJ_HYOUSENSOU) {
}

void SkillNJ_HYOUSENSOU::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, this->skill_id_, skill_lv);
	skill_attack(BF_MAGIC, src, src, target, this->skill_id_, skill_lv, tick, flag);
}
