// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hocuspocus.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/pet.hpp"
#include "map/status.hpp"

SkillHocusPocus::SkillHocusPocus() : SkillImpl(SA_ABRACADABRA) {
}

void SkillHocusPocus::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (abra_db.empty()) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		return;
	}
	else {
		int32 abra_skill_id = 0, abra_skill_lv;
		size_t checked = 0, checked_max = abra_db.size() * 3;

		do {
			auto abra_spell = abra_db.random();

			abra_skill_id = abra_spell->skill_id;
			abra_skill_lv = min(skill_lv, skill_get_max(abra_skill_id));

			if( rnd() % 10000 < abra_spell->per[max(skill_lv - 1, 0)] ){
				break;
			}
		} while (checked++ < checked_max);

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

		if( sd )
		{// player-casted
			sd->state.abra_flag = 1;
			sd->skillitem = abra_skill_id;
			sd->skillitemlv = abra_skill_lv;
			sd->skillitem_keep_requirement = false;
			clif_item_skill(sd, abra_skill_id, abra_skill_lv);
		}
		else
		{// mob-casted
			struct unit_data *ud = unit_bl2ud(src);
			int32 inf = skill_get_inf(abra_skill_id);
			if (!ud) return;
			if (inf&INF_SELF_SKILL || inf&INF_SUPPORT_SKILL) {
				if (src->type == BL_PET)
					target = (block_list*)((TBL_PET*)src)->master;
				if (!target) target = src;
				unit_skilluse_id(src, target->id, abra_skill_id, abra_skill_lv);
			} else {	//Assume offensive skills
				int32 target_id = 0;
				if (ud->target)
					target_id = ud->target;
				else switch (src->type) {
					case BL_MOB: target_id = ((TBL_MOB*)src)->target_id; break;
					case BL_PET: target_id = ((TBL_PET*)src)->target_id; break;
				}
				if (!target_id)
					return;
				if (skill_get_casttype(abra_skill_id) == CAST_GROUND) {
					target = map_id2bl(target_id);
					if (!target) target = src;
					unit_skilluse_pos(src, target->x, target->y, abra_skill_id, abra_skill_lv);
				} else
					unit_skilluse_id(src, target_id, abra_skill_id, abra_skill_lv);
			}
		}
	}
}
