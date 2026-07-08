// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcvenomimpress.hpp"

#include "map/status.hpp"

SkillNpcVenomImpress::SkillNpcVenomImpress() : WeaponSkillImpl(NPC_VENOMIMPRESS) {
}

void SkillNpcVenomImpress::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_VENOMIMPRESS, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
