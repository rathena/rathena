#include "skill_tf_detoxify.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"
#include "../../clif.hpp"

SkillTF_DETOXIFY::SkillTF_DETOXIFY() : WeaponSkillImpl(TF_DETOXIFY) {
}

void SkillTF_DETOXIFY::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_DETOXIFY doesn't have a specific skill ratio calculation
}

void SkillTF_DETOXIFY::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_DETOXIFY doesn't modify hit rate
}

void SkillTF_DETOXIFY::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// TF_DETOXIFY doesn't have additional effects in the damage phase
}

void SkillTF_DETOXIFY::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_DETOXIFY doesn't have a damage implementation
}

int32 SkillTF_DETOXIFY::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
	status_change_end(bl, SC_POISON);
	status_change_end(bl, SC_DPOISON);
	return 1;
}
