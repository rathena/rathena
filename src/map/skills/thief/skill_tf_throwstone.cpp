#include "skill_tf_throwstone.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"
#include "../../clif.hpp"

SkillTF_THROWSTONE::SkillTF_THROWSTONE() : SkillImpl(TF_THROWSTONE) {
}

void SkillTF_THROWSTONE::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_THROWSTONE doesn't have a specific skill ratio calculation
}

void SkillTF_THROWSTONE::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_THROWSTONE doesn't modify hit rate
}

void SkillTF_THROWSTONE::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (sd != nullptr)
	{
		// Only blind if used by player and stun failed
		if (!sc_start(src, target, SC_STUN, 3, skill_lv, skill_get_time(this->skill_id, skill_lv)))
			sc_start(src, target, SC_BLIND, 3, skill_lv, skill_get_time2(this->skill_id, skill_lv));
	}
	else
	{
		// 5% stun chance and no blind chance when used by monsters
		sc_start(src, target, SC_STUN, 5, skill_lv, skill_get_time(this->skill_id, skill_lv));
	}
}

void SkillTF_THROWSTONE::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(skill_get_type(this->skill_id), src, src, target, this->skill_id, skill_lv, tick, flag);
}

int32 SkillTF_THROWSTONE::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_THROWSTONE doesn't have a no damage implementation
	return 0;
}
