#include "skill_nj_kirikage.hpp"

#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../skill.hpp"
#include "../../unit.hpp"

SkillNJ_KIRIKAGE::SkillNJ_KIRIKAGE() : WeaponSkillImpl(NJ_KIRIKAGE) {
}

void SkillNJ_KIRIKAGE::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += -50 + 150 * skill_lv;
#else
	base_skillratio += 100 * (skill_lv - 1);
#endif
}

void SkillNJ_KIRIKAGE::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (!map_flag_gvg2(src->m) && !map_getmapflag(src->m, MF_BATTLEGROUND))
	{ // You don't move on GVG grounds.
		int16 x, y;
		map_search_freecell(target, 0, &x, &y, 1, 1, 0);
		if (unit_movepos(src, x, y, 0, 0))
		{
			clif_blown(src);
		}
	}
	status_change_end(src, SC_HIDING);
	skill_attack(BF_WEAPON, src, src, target, this->skill_id_, skill_lv, tick, flag);
}
