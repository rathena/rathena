// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "terrawave.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"

#include "skill_factory_druid.hpp"

constexpr int32 kTerraWaveStepMs = 150;

SkillTerraWave::SkillTerraWave() : SkillImplRecursiveDamageSplash(AT_TERRA_WAVE) {
}

void SkillTerraWave::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	SkillFactoryDruid::try_gain_growth_stacks(src, tick, getSkillId());
}

void SkillTerraWave::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	const int32 segment_count = 4;
	const int32 segment_step = 3;
	map_session_data *sd = BL_CAST(BL_PC, src);
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
		if (sd) {
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
	SkillFactoryDruid::try_gain_growth_stacks(src, tick, getSkillId());
}

void SkillTerraWave::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 12000 + 300 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillTerraWave::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
