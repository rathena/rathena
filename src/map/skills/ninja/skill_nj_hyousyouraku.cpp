#include "skill_nj_hyousyouraku.hpp"

#include "../../status.hpp"

SkillNJ_HYOUSYOURAKU::SkillNJ_HYOUSYOURAKU() : SkillImpl(NJ_HYOUSYOURAKU) {
}

void SkillNJ_HYOUSYOURAKU::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_FREEZE, (10 + 10 * skill_lv), skill_lv, skill_get_time2(this->skill_id_, skill_lv));
}
