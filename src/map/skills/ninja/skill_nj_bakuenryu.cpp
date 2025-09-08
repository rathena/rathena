#include "skill_nj_bakuenryu.hpp"

#include "../../clif.hpp"
#include "../../skill.hpp"
#include "../../unit.hpp"

SkillNJ_BAKUENRYU::SkillNJ_BAKUENRYU() : SkillImpl(NJ_BAKUENRYU) {
}

void SkillNJ_BAKUENRYU::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, this->skill_id_, skill_lv);
	skill_unitsetting(src, this->skill_id_, skill_lv, target->x, target->y, 0);
}
