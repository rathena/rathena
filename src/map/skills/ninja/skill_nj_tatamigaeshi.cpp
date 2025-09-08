#include "skill_nj_tatamigaeshi.hpp"

#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../skill.hpp"
#include "../../unit.hpp"

SkillNJ_TATAMIGAESHI::SkillNJ_TATAMIGAESHI() : WeaponSkillImpl(NJ_TATAMIGAESHI) {
}

void SkillNJ_TATAMIGAESHI::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 10 * skill_lv;
#ifdef RENEWAL
	base_skillratio *= 2;
#endif
}

void SkillNJ_TATAMIGAESHI::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
#ifndef RENEWAL
	if (skill_get_inf2(this->skill_id_, INF2_ISNPC)) {
#endif
		clif_skill_damage(*src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, this->skill_id_, skill_lv, DMG_SINGLE);
#ifndef RENEWAL
	}
#endif
}
