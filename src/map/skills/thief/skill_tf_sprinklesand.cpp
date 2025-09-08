#include "skill_tf_sprinklesand.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"

SkillTF_SPRINKLESAND::SkillTF_SPRINKLESAND() : WeaponSkillImpl(TF_SPRINKLESAND) {
}

void SkillTF_SPRINKLESAND::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_SPRINKLESAND doesn't have a specific skill ratio calculation
}

void SkillTF_SPRINKLESAND::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_SPRINKLESAND doesn't modify hit rate
}

void SkillTF_SPRINKLESAND::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	sc_start(src, target, SC_BLIND, (sd != nullptr) ? 20 : 15, skill_lv, skill_get_time2(this->skill_id, skill_lv));
}

void SkillTF_SPRINKLESAND::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag);
}

int32 SkillTF_SPRINKLESAND::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_SPRINKLESAND doesn't have a no damage implementation
	return 0;
}
