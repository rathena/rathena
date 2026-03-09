// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hesperuslit.hpp"

#include <config/core.hpp>

#include <common/random.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillHesperusLit::SkillHesperusLit() : WeaponSkillImpl(LG_HESPERUSLIT) {
}

void SkillHesperusLit::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	if (sc && sc->getSCE(SC_INSPIRATION))
		skillratio += -100 + 450 * skill_lv;
	else
		skillratio += -100 + 300 * skill_lv;
	skillratio += sstatus->vit / 6; // !TODO: What's the VIT bonus?
	RE_LVL_DMOD(100);
}

void SkillHesperusLit::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);

	if( pc_checkskill(sd,LG_PINPOINTATTACK) > 0 && sc && sc->getSCE(SC_BANDING) && sc->getSCE(SC_BANDING)->val2 > 5 )
		skill_castend_damage_id(src,target,LG_PINPOINTATTACK, rnd_value<uint16>(1, pc_checkskill(sd,LG_PINPOINTATTACK)),tick,0);
}
