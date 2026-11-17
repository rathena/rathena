// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mirageswarm.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillMirageSwarm::SkillMirageSwarm() : SkillImpl(SS_SHINKIROU_GUNSHU) {
}

void SkillMirageSwarm::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	unit_data* ud = unit_bl2ud(src);

	if (ud == nullptr)
		return;

	// Three clones: left, right and behind the caster, two cells away.
	uint8 dir = unit_getdir(src);
	const uint8 dirs[3] = { static_cast<uint8>((dir + 2) % 8), static_cast<uint8>((dir + 6) % 8), static_cast<uint8>((dir + 4) % 8) };
	int16 xs[3], ys[3];

	for (int32 i = 0; i < 3; i++) {
		xs[i] = src->x + dirx[dirs[i]] * 2;
		ys[i] = src->y + diry[dirs[i]] * 2;

		if (map_getcell(src->m, xs[i], ys[i], CELL_CHKNOPASS)) {
			if (sd != nullptr)
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL_POS);
			return;
		}
	}

	// Existing mirages are removed.
	std::vector<std::shared_ptr<s_skill_unit_group>> old;
	for (const std::shared_ptr<s_skill_unit_group>& sug : ud->skillunits) {
		if (sug != nullptr && sug->skill_id == SS_SHINKIROU)
			old.push_back(sug);
	}
	for (const std::shared_ptr<s_skill_unit_group>& sug : old)
		skill_delunitgroup(sug);

	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);

	sc_start(src, src, skill_get_sc(SS_SHINKIROU), 100, 1, 20000);

	uint16 mirage_lv = std::max<uint16>(1, sd != nullptr ? static_cast<uint16>(pc_checkskill(sd, SS_SHINKIROU)) : 1);

	for (int32 i = 0; i < 3; i++) {
		std::shared_ptr<s_skill_unit_group> group = skill_unitsetting(src, SS_SHINKIROU, mirage_lv, xs[i], ys[i], 0);

		if (group != nullptr)
			group->limit = 20000;
	}
}
