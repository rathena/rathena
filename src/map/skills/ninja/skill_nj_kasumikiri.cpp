#include "skill_nj_kasumikiri.hpp"

#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../skill.hpp"
#include "../../status.hpp"
#include "../../unit.hpp"

SkillNJ_KASUMIKIRI::SkillNJ_KASUMIKIRI() : WeaponSkillImpl(NJ_KASUMIKIRI) {
}

void SkillNJ_KASUMIKIRI::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 20 * skill_lv;
#else
	base_skillratio += 10 * skill_lv;
#endif
}

void SkillNJ_KASUMIKIRI::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (skill_attack(BF_WEAPON, src, src, target, this->skill_id_, skill_lv, tick, flag) > 0)
		sc_start(src, src, SC_HIDING, 100, skill_lv, skill_get_time(this->skill_id_, skill_lv));
}

void SkillNJ_KASUMIKIRI::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *bl, this->skill_id_, skill_lv,
						sc_start(src, bl, SC_HIDING, 100, skill_lv, skill_get_time(this->skill_id_, skill_lv)));
	status_change_end(bl, SC_NEN);
}
