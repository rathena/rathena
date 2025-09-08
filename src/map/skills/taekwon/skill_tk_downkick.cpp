#include "skill_tk_downkick.hpp"

SkillTK_DOWNKICK::SkillTK_DOWNKICK() : WeaponSkillImpl(TK_DOWNKICK) {
}

void SkillTK_DOWNKICK::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 60 + 20 * skill_lv;
}

void SkillTK_DOWNKICK::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 3333, skill_lv, skill_get_time2(this->skill_id, skill_lv));
}

void SkillTK_DOWNKICK::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag);
}
