#include "skill_tf_pickstone.hpp"

#include "../agent/battle_calc_attack_skill_ratio.hpp"
#include "../agent/is_attack_hitting.hpp"
#include "../agent/skill_additional_effect.hpp"
#include "../agent/skill_castend_damage_id.hpp"
#include "../agent/skill_castend_nodamage_id.hpp"
#include "../../status.hpp"
#include "../../clif.hpp"
#include "../../pc.hpp"
#include "../../item.hpp"
#include "../../map.hpp"
#include "../../battle.hpp"

SkillTF_PICKSTONE::SkillTF_PICKSTONE() : SkillImpl(TF_PICKSTONE) {
}

void SkillTF_PICKSTONE::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// TF_PICKSTONE doesn't have a specific skill ratio calculation
}

void SkillTF_PICKSTONE::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// TF_PICKSTONE doesn't modify hit rate
}

void SkillTF_PICKSTONE::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// TF_PICKSTONE doesn't have additional effects
}

void SkillTF_PICKSTONE::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_PICKSTONE doesn't have a damage implementation
}

int32 SkillTF_PICKSTONE::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (sd)
	{
		unsigned char eflag;
		struct item item_tmp;
		struct block_list tbl;
		clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
		memset(&item_tmp, 0, sizeof(item_tmp));
		memset(&tbl, 0, sizeof(tbl)); // [MouseJstr]
		item_tmp.nameid = ITEMID_STONE;
		item_tmp.identify = 1;
		tbl.id = 0;
		// Commented because of duplicate animation [Lemongrass]
		// At the moment this displays the pickup animation a second time
		// If this is required in older clients, we need to add a version check here
		// clif_takeitem(*sd,tbl);
		eflag = pc_additem(sd, &item_tmp, 1, LOG_TYPE_PRODUCE);
		if (eflag)
		{
			clif_additem(sd, 0, 0, eflag);
			if (battle_config.skill_drop_items_full)
				map_addflooritem(&item_tmp, 1, sd->m, sd->x, sd->y, 0, 0, 0, 4, 0);
		}
	}
	return 1;
}
