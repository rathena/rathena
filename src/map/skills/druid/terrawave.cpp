// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "terrawave.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"

#include "groundbloom.hpp"

constexpr int32 kTerraWaveStepMs = 150;

SkillTerraWave::SkillTerraWave() : SkillImplRecursiveDamageSplash(AT_TERRA_WAVE) {
}

void SkillTerraWave::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	const int32 segment_count = 4;
	const int32 segment_step = 3;
	map_data *mapdata = map_getmapdata(src->m);
	bool blocked = false;

	if (mapdata != nullptr) {
		for (int32 dx = -1; dx <= 1 && !blocked; ++dx) {
			for (int32 dy = -1; dy <= 1; ++dy) {
				const int16 cx = x + dx;
				const int16 cy = y + dy;

				if (cx < 0 || cy < 0 || cx >= mapdata->xs || cy >= mapdata->ys) {
					blocked = true;
					break;
				}
				if (map_getcell(src->m, cx, cy, CELL_CHKNOPASS)) {
					blocked = true;
					break;
				}
			}
		}
	}

	if (blocked) {
		if (map_session_data *sd = BL_CAST(BL_PC, src); sd != nullptr) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	const uint8 dir = map_calc_dir(src, x, y);
	const int32 max_distance = segment_step * (segment_count - 1);
	const int32 wave_flag = flag & ~SD_ANIMATION;
	int16 cx = x;
	int16 cy = y;
	int32 placed = 0;

	for (int32 dist = 0; dist <= max_distance && placed < segment_count; ++dist) {
		if (dist > 0) {
			cx += dirx[dir];
			cy += diry[dir];
		}

		if (mapdata && (cx < 0 || cy < 0 || cx >= mapdata->xs || cy >= mapdata->ys)) {
			break;
		}
		if (map_getcell(src->m, cx, cy, CELL_CHKNOPASS)) {
			break;
		}
		if (dist % segment_step != 0) {
			continue;
		}

		const t_tick step_tick = tick + kTerraWaveStepMs * (placed + 1);
		skill_addtimerskill(src, step_tick, 0, cx, cy, getSkillId(), skill_lv, 0, wave_flag);
		placed++;
	}

	// Updates growth status and casts Ground Bloom if the conditions are met
	SkillGroundBloom::castGroundBloom(src, tick, 2);
}

void SkillTerraWave::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 12000 + 300 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 15 * sstatus->spl;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_EARTH
	RE_LVL_DMOD(100);
}
