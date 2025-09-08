#include "skill_gs_grounddrift.hpp"

SkillGS_GROUNDDRIFT::SkillGS_GROUNDDRIFT() : WeaponSkillImpl(GS_GROUNDDRIFT) {
}

void SkillGS_GROUNDDRIFT::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 100 + 20 * skill_lv;
#endif
}
