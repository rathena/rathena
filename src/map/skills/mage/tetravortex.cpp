// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tetravortex.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

// WL_TETRAVORTEX
SkillTetraVortex::SkillTetraVortex() : SkillImpl(WL_TETRAVORTEX) {
}

void SkillTetraVortex::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd == nullptr) { // Monster usage
		uint8 i = 0;
		const static std::vector<std::vector<uint16>> tetra_skills = { { WL_TETRAVORTEX_FIRE, 1 },
																	   { WL_TETRAVORTEX_WIND, 4 },
																	   { WL_TETRAVORTEX_WATER, 2 },
																	   { WL_TETRAVORTEX_GROUND, 8 } };

		for (const auto &skill : tetra_skills) {
			if (skill_lv > 5) {
				skill_area_temp[0] = i;
				skill_area_temp[1] = skill[1];
				map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, skill[0], skill_lv, tick, flag | BCT_ENEMY, skill_castend_damage_id);
			} else
				skill_addtimerskill(src, tick + i * 200, target->id, skill[1], 0, skill[0], skill_lv, i, flag);
			i++;
		}
	} else if (sc) { // No SC? No spheres
		int32 i, k = 0;

		if (sc->getSCE(SC_SPHERE_5)) // If 5 spheres, remove last one (based on reverse order) and only do 4 actions (Official behavior)
			status_change_end(src, SC_SPHERE_1);

		for (i = SC_SPHERE_5; i >= SC_SPHERE_1; i--) { // Loop should always be 4 for regular players, but unconditional_skill could be less
			if (sc->getSCE(static_cast<sc_type>(i)) == nullptr)
				continue;

			uint16 subskill = 0;

			switch (sc->getSCE(static_cast<sc_type>(i))->val1) {
				case WLS_FIRE:
					subskill = WL_TETRAVORTEX_FIRE;
					k |= 1;
					break;
				case WLS_WIND:
					subskill = WL_TETRAVORTEX_WIND;
					k |= 4;
					break;
				case WLS_WATER:
					subskill = WL_TETRAVORTEX_WATER;
					k |= 2;
					break;
				case WLS_STONE:
					subskill = WL_TETRAVORTEX_GROUND;
					k |= 8;
					break;
			}

			if (skill_lv > 5) {
				skill_area_temp[0] = abs(i - SC_SPHERE_5);
				skill_area_temp[1] = k;
				map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, subskill, skill_lv, tick, flag | BCT_ENEMY, skill_castend_damage_id);
			} else
				skill_addtimerskill(src, tick + abs(i - SC_SPHERE_5) * 200, target->id, k, 0, subskill, skill_lv, abs(i - SC_SPHERE_5), flag);
			status_change_end(src, static_cast<sc_type>(i));
		}
	}
}


// WL_TETRAVORTEX_GROUND
SkillTetraVortexEarth::SkillTetraVortexEarth() : SkillImpl(WL_TETRAVORTEX_GROUND) {
}

void SkillTetraVortexEarth::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 800 + 400 * skill_lv;
}

void SkillTetraVortexEarth::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_addtimerskill(src, tick + skill_area_temp[0] * 200, target->id, skill_area_temp[1], 0, getSkillId(), skill_lv, 0, flag);
}


// WL_TETRAVORTEX_FIRE
SkillTetraVortexFire::SkillTetraVortexFire() : SkillImpl(WL_TETRAVORTEX_FIRE) {
}

void SkillTetraVortexFire::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 800 + 400 * skill_lv;
}

void SkillTetraVortexFire::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_addtimerskill(src, tick + skill_area_temp[0] * 200, target->id, skill_area_temp[1], 0, getSkillId(), skill_lv, 0, flag);
}


// WL_TETRAVORTEX_WATER
SkillTetraVortexWater::SkillTetraVortexWater() : SkillImpl(WL_TETRAVORTEX_WATER) {
}

void SkillTetraVortexWater::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 800 + 400 * skill_lv;
}

void SkillTetraVortexWater::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_addtimerskill(src, tick + skill_area_temp[0] * 200, target->id, skill_area_temp[1], 0, getSkillId(), skill_lv, 0, flag);
}


// WL_TETRAVORTEX_WIND
SkillTetraVortexWind::SkillTetraVortexWind() : SkillImpl(WL_TETRAVORTEX_WIND) {
}

void SkillTetraVortexWind::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 800 + 400 * skill_lv;
}

void SkillTetraVortexWind::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_addtimerskill(src, tick + skill_area_temp[0] * 200, target->id, skill_area_temp[1], 0, getSkillId(), skill_lv, 0, flag);
}
