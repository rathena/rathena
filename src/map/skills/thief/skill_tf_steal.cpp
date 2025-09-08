#include "skill_tf_steal.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"
#include "../../clif.hpp"
#include "../../pc.hpp"

SkillTF_STEAL::SkillTF_STEAL() : WeaponSkillImpl(TF_STEAL) {
}

void SkillTF_STEAL::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_STEAL doesn't have a specific skill ratio calculation
}

void SkillTF_STEAL::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_STEAL doesn't modify hit rate
}

void SkillTF_STEAL::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// TF_STEAL doesn't have additional effects in the damage phase
}

void SkillTF_STEAL::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_STEAL doesn't have a damage implementation
}

int32 SkillTF_STEAL::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (sd)
	{
		if (pc_steal_item(sd, bl, skill_lv))
			clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
		else
			clif_skill_fail(*sd, this->skill_id, USESKILL_FAIL);
	}
	return 1;
}
