// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "release.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRelease::SkillRelease() : SkillImpl(WL_RELEASE) {
}

void SkillRelease::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sc == nullptr)
		return;
	if (sd) {
		int32 i;

#ifndef RENEWAL
		skill_toggle_magicpower(src, getSkillId()); // No hit will be amplified
#endif
		if (skill_lv == 1) { // SpellBook
			if (sc->getSCE(SC_FREEZE_SP) == nullptr)
				return;

			bool found_spell = false;

			for (i = SC_MAXSPELLBOOK; i >= SC_SPELLBOOK1; i--) { // List all available spell to be released
				if (sc->getSCE(i) != nullptr) {
					found_spell = true;
					break;
				}
			}

			if (!found_spell)
				return;

			// Now extract the data from the preserved spell
			uint16 pres_skill_id = sc->getSCE(i)->val1;
			uint16 pres_skill_lv = sc->getSCE(i)->val2;
			uint16 point = sc->getSCE(i)->val3;

			status_change_end(src, static_cast<sc_type>(i));

			if( sc->getSCE(SC_FREEZE_SP)->val2 > point )
				sc->getSCE(SC_FREEZE_SP)->val2 -= point;
			else // Last spell to be released
				status_change_end(src, SC_FREEZE_SP);

			if( !skill_check_condition_castbegin(*sd, pres_skill_id, pres_skill_lv) )
				return;

			// Get the requirement for the preserved skill
			skill_consume_requirement(sd, pres_skill_id, pres_skill_lv, 1);

			switch( skill_get_casttype(pres_skill_id) )
			{
				case CAST_GROUND:
					skill_castend_pos2(src, target->x, target->y, pres_skill_id, pres_skill_lv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill_castend_nodamage_id(src, target, pres_skill_id, pres_skill_lv, tick, 0);
					break;
				case CAST_DAMAGE:
					skill_castend_damage_id(src, target, pres_skill_id, pres_skill_lv, tick, 0);
					break;
			}

			sd->ud.canact_tick = i64max(tick + skill_delayfix(src, pres_skill_id, pres_skill_lv), sd->ud.canact_tick);
			clif_status_change(src, EFST_POSTDELAY, 1, skill_delayfix(src, pres_skill_id, pres_skill_lv), 0, 0, 0);

			int32 cooldown = pc_get_skillcooldown(sd,pres_skill_id, pres_skill_lv);

			if( cooldown > 0 )
				skill_blockpc_start(*sd, pres_skill_id, cooldown);
		} else { // Summoned Balls
			for (i = SC_SPHERE_5; i >= SC_SPHERE_1; i--) {
				if (sc->getSCE(static_cast<sc_type>(i)) == nullptr)
					continue;

				int32 skele = WL_RELEASE - 5 + sc->getSCE(static_cast<sc_type>(i))->val1 - WLS_FIRE; // Convert Ball Element into Skill ATK for balls

				// WL_SUMMON_ATK_FIRE, WL_SUMMON_ATK_WIND, WL_SUMMON_ATK_WATER, WL_SUMMON_ATK_GROUND
				skill_addtimerskill(src, tick + (t_tick)status_get_adelay(src) * abs(i - SC_SPHERE_1), target->id, 0, 0, skele, sc->getSCE(static_cast<sc_type>(i))->val2, BF_MAGIC, flag | SD_LEVEL);
				status_change_end(src, static_cast<sc_type>(i)); // Eliminate ball
			}
			clif_skill_nodamage(src, *target, getSkillId(), 0);
		}
	}
}
