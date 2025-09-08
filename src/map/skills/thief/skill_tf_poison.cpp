#include "skill_tf_poison.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"
#include "../../clif.hpp"

SkillTF_POISON::SkillTF_POISON() : WeaponSkillImpl(TF_POISON) {
}

void SkillTF_POISON::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_POISON doesn't have a specific skill ratio calculation
}

void SkillTF_POISON::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_POISON doesn't modify hit rate
}

void SkillTF_POISON::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (!sc_start2(src, target, SC_POISON, (4 * skill_lv + 10), skill_lv, src->id, skill_get_time2(this->skill_id, skill_lv)) && sd)
		clif_skill_fail(*sd, this->skill_id);
}

void SkillTF_POISON::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag);
}

int32 SkillTF_POISON::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_POISON doesn't have a no damage implementation
	return 0;
}
