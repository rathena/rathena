#include "skill_tk_highjump.hpp"

SkillTK_HIGHJUMP::SkillTK_HIGHJUMP() : SkillImpl(TK_HIGHJUMP) {
}

void SkillTK_HIGHJUMP::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	int32 x, y, dir = unit_getdir(src);
	struct map_data *mapdata = map_getmapdata(src->m);

	// Fails on noteleport maps, except for GvG and BG maps [Skotlex]
	if (mapdata->getMapFlag(MF_NOTELEPORT) &&
		!(mapdata->getMapFlag(MF_BATTLEGROUND) || map_flag_gvg2(mapdata))) {
		clif_skill_nodamage(src, *bl, TK_HIGHJUMP, skill_lv);
		return;
	} else if (dir % 2) {
		// Diagonal
		x = src->x + dirx[dir] * (skill_lv * 4) / 3;
		y = src->y + diry[dir] * (skill_lv * 4) / 3;
	} else {
		x = src->x + dirx[dir] * skill_lv * 2;
		y = src->y + diry[dir] * skill_lv * 2;
	}

	int32 x1 = x + dirx[dir];
	int32 y1 = y + diry[dir];

	clif_skill_nodamage(src, *bl, TK_HIGHJUMP, skill_lv);
	if (!map_count_oncell(src->m, x, y, BL_PC | BL_NPC | BL_MOB, 0) && map_getcell(src->m, x, y, CELL_CHKREACH) &&
		!map_count_oncell(src->m, x1, y1, BL_PC | BL_NPC | BL_MOB, 0) && map_getcell(src->m, x1, y1, CELL_CHKREACH) &&
		unit_movepos(src, x, y, 1, 0))
		clif_blown(src);
}
