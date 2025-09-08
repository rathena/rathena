#include "skill_nj_kunai.hpp"

#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../skill.hpp"
#include "../../unit.hpp"

SkillNJ_KUNAI::SkillNJ_KUNAI() : WeaponSkillImpl(NJ_KUNAI) {
}

void SkillNJ_KUNAI::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += -100 + 100 * skill_lv;
}

void SkillNJ_KUNAI::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id_, skill_lv, tick, flag);
}
