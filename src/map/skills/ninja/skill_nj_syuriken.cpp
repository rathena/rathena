#include "skill_nj_syuriken.hpp"

#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../skill.hpp"
#include "../../unit.hpp"

SkillNJ_SYURIKEN::SkillNJ_SYURIKEN() : WeaponSkillImpl(NJ_SYURIKEN) {
}

void SkillNJ_SYURIKEN::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 5 * skill_lv;
#endif
}

void SkillNJ_SYURIKEN::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id_, skill_lv, tick, flag);
}
