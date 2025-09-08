#include "skill_tf_backsliding.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"
#include "../../clif.hpp"
#include "../../unit.hpp"

SkillTF_BACKSLIDING::SkillTF_BACKSLIDING() : WeaponSkillImpl(TF_BACKSLIDING) {
}

void SkillTF_BACKSLIDING::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_BACKSLIDING doesn't have a specific skill ratio calculation
}

void SkillTF_BACKSLIDING::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_BACKSLIDING doesn't modify hit rate
}

void SkillTF_BACKSLIDING::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// TF_BACKSLIDING doesn't have additional effects
}

void SkillTF_BACKSLIDING::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_BACKSLIDING doesn't have a damage implementation
}

int32 SkillTF_BACKSLIDING::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// Backsliding makes you immune to being stopped for 200ms, but only if you don't have the endure effect yet
	if (struct unit_data *ud = unit_bl2ud(bl); ud != nullptr && !status_isendure(*bl, tick, true))
		ud->endure_tick = tick + 200;

	int16 blew_count = skill_blown(src, bl, skill_get_blewcount(this->skill_id, skill_lv), unit_getdir(bl), (enum e_skill_blown)(BLOWN_IGNORE_NO_KNOCKBACK
#ifdef RENEWAL
																| BLOWN_DONT_SEND_PACKET
#endif
																));
	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
#ifdef RENEWAL
	if (blew_count > 0)
		clif_blown(src); // Always blow, otherwise it shows a casting animation. [Lemongrass]
#else
	clif_slide(*bl, bl->x, bl->y); // Show the casting animation on pre-re
#endif
	return 1;
}
