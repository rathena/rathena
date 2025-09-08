#include "skill_tf_hiding.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"
#include "../../clif.hpp"

SkillTF_HIDING::SkillTF_HIDING() : SkillImpl(TF_HIDING) {
}

void SkillTF_HIDING::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_HIDING doesn't have a specific skill ratio calculation
}

void SkillTF_HIDING::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_HIDING doesn't modify hit rate
}

void SkillTF_HIDING::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// TF_HIDING doesn't have additional effects
}

void SkillTF_HIDING::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_HIDING doesn't have a damage implementation
}

int32 SkillTF_HIDING::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change *tsc = status_get_sc(bl);
	struct status_change_entry *tsce = (tsc != nullptr) ? tsc->getSCE(SC_HIDING) : nullptr;
	
	if (tsce)
	{
		clif_skill_nodamage(src, *bl, this->skill_id, -1, status_change_end(bl, SC_HIDING)); // Hide skill-scream animation.
		return 0;
	}
	clif_skill_nodamage(src, *bl, this->skill_id, -1, sc_start(src, bl, SC_HIDING, 100, skill_lv, skill_get_time(this->skill_id, skill_lv)));
	return 1;
}
