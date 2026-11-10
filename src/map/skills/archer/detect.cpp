// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "detect.hpp"

#include "common/timer.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

/// [RO Sandbox] How often Detect re-checks its area while it lasts
static const t_tick SANDBOX_DETECT_INTERVAL = 250;

/// Reveals hidden units and traps around (x, y) on map m
static void detect_reveal(block_list* src, int16 m, int32 x, int32 y, uint16 skill_lv, t_tick tick) {
	int32 i = skill_get_splash(HT_DETECTING, skill_lv);
	map_foreachinallarea( status_change_timer_sub,
		m, x-i, y-i, x+i,y+i,BL_CHAR,
		src,nullptr,SC_SIGHT,tick);
	skill_reveal_trap_inarea(src, i, x, y);
}

/**
 * [RO Sandbox] Phase 4 decision 9: Detect keeps revealing its area for its Duration1
 * (set from the Duration column of RO_Skill_Trees). Packed data: x 16 bits, y 16 bits,
 * skill level 8 bits, checks left 8 bits, map 16 bits.
 */
static TIMER_FUNC(sandbox_detect_repeat) {
	block_list* src = map_id2bl(id);
	uint64 d = static_cast<uint64>(data);
	int16 m = static_cast<int16>((d >> 48) & 0xFFFF);

	if (src == nullptr || src->m != m)
		return 0;

	int32 x = d & 0xFFFF, y = (d >> 16) & 0xFFFF;
	uint16 skill_lv = (d >> 32) & 0xFF;
	uint64 left = (d >> 40) & 0xFF;

	detect_reveal(src, m, x, y, skill_lv, tick);
	if (left > 1)
		add_timer(tick + SANDBOX_DETECT_INTERVAL, sandbox_detect_repeat, id,
			static_cast<intptr_t>((d & ~(0xFFULL << 40)) | ((left - 1) << 40)));
	return 0;
}

SkillDetect::SkillDetect() : SkillImpl(HT_DETECTING) {
}

void SkillDetect::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	detect_reveal(src, src->m, x, y, skill_lv, tick);

	// [RO Sandbox] Keep revealing for the skill's duration
	t_tick checks = std::min<t_tick>(skill_get_time(getSkillId(), skill_lv) / SANDBOX_DETECT_INTERVAL, 255);
	if (checks > 0) {
		uint64 d = static_cast<uint64>(x & 0xFFFF) | (static_cast<uint64>(y & 0xFFFF) << 16)
			| (static_cast<uint64>(skill_lv & 0xFF) << 32) | (static_cast<uint64>(checks) << 40)
			| (static_cast<uint64>(static_cast<uint16>(src->m)) << 48);
		add_timer(tick + SANDBOX_DETECT_INTERVAL, sandbox_detect_repeat, src->id, static_cast<intptr_t>(d));
	}
}
