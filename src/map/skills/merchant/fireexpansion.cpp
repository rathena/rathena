// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fireexpansion.hpp"

#include "map/pc.hpp"

// GN_FIRE_EXPANSION
SkillFireExpansion::SkillFireExpansion() : SkillImpl(GN_FIRE_EXPANSION) {
}

void SkillFireExpansion::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	struct unit_data* ud = unit_bl2ud(src);

	if (!ud)
		return;

	auto predicate = [x, y](std::shared_ptr<s_skill_unit_group> sg) { auto* su = sg->unit; return sg->skill_id == GN_DEMONIC_FIRE && distance_xy(x, y, su->x, su->y) < 4; };
	auto it = std::find_if(ud->skillunits.begin(), ud->skillunits.end(), predicate);
	if (it != ud->skillunits.end()) {
		auto* unit_group = it->get();
		skill_unit* su = unit_group->unit;

		switch (skill_lv) {
		case 1: {
			// TODO:
			int32 duration = (int32)(unit_group->limit - DIFF_TICK(tick, unit_group->tick));

			skill_delunit(su);
			skill_unitsetting(src, GN_DEMONIC_FIRE, 1, x, y, duration);
			flag |= 1;
		}
				break;
		case 2:
			map_foreachinallarea(skill_area_sub, src->m, su->x - 2, su->y - 2, su->x + 2, su->y + 2, BL_CHAR, src, GN_DEMONIC_FIRE, skill_lv + 20, tick, flag | BCT_ENEMY | SD_LEVEL | 1, skill_castend_damage_id);
			if (su != nullptr)
				skill_delunit(su);
			break;
		case 3:
			skill_delunit(su);
			skill_unitsetting(src, GN_FIRE_EXPANSION_SMOKE_POWDER, 1, x, y, 0);
			flag |= 1;
			break;
		case 4:
			skill_delunit(su);
			skill_unitsetting(src, GN_FIRE_EXPANSION_TEAR_GAS, 1, x, y, 0);
			flag |= 1;
			break;
		case 5: {
			uint16 acid_lv = 5; // Cast at Acid Demonstration at level 5 unless the user has a higher level learned.

			if (sd && pc_checkskill(sd, CR_ACIDDEMONSTRATION) > 5)
				acid_lv = pc_checkskill(sd, CR_ACIDDEMONSTRATION);
			map_foreachinallarea(skill_area_sub, src->m, su->x - 2, su->y - 2, su->x + 2, su->y + 2, BL_CHAR, src, GN_FIRE_EXPANSION_ACID, acid_lv, tick, flag | BCT_ENEMY | SD_LEVEL | 1, skill_castend_damage_id);
			if (su != nullptr)
				skill_delunit(su);
		}
			break;
		}
	}
}


// GN_FIRE_EXPANSION_ACID
SkillFireExpansionAcid::SkillFireExpansionAcid() : SkillImplRecursiveDamageSplash(GN_FIRE_EXPANSION_ACID) {
}

void SkillFireExpansionAcid::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_break_equip(src,target, EQP_WEAPON|EQP_ARMOR, 100*skill_lv, BCT_ENEMY);
}
