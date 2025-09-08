#include "skill_tf_double.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"

SkillTF_DOUBLE::SkillTF_DOUBLE() : WeaponSkillImpl(TF_DOUBLE) {
}

void SkillTF_DOUBLE::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_DOUBLE doesn't have a specific skill ratio calculation
}

void SkillTF_DOUBLE::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_DOUBLE doesn't modify hit rate
}

void SkillTF_DOUBLE::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// TF_DOUBLE doesn't have additional effects
}

void SkillTF_DOUBLE::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag);
}

int32 SkillTF_DOUBLE::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_DOUBLE doesn't have a no damage implementation
	return 0;
}
