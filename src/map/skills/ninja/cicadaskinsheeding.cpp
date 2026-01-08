// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cicadaskinsheeding.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillCicadaSkinSheeding::SkillCicadaSkinSheeding() : StatusSkillImpl(NJ_UTSUSEMI) {
}

void SkillCicadaSkinSheeding::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	clif_skill_nodamage(src, *target, this->skill_id_, skill_lv,
	                    sc_start(src, target, SC_UTSUSEMI, 100, skill_lv, skill_get_time(this->skill_id_, skill_lv)));
	status_change_end(target, SC_NEN);
}
