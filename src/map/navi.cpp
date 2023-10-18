// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <config/core.hpp>

#ifdef MAP_GENERATOR

#include <sys/stat.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <chrono>
#include <queue>
#include <vector>

#include <common/db.hpp>
#include <common/malloc.hpp>
#include <common/showmsg.hpp>
#include <common/utils.hpp>
#include "map.hpp"
#include "mob.hpp"
#include "navi.hpp"
#include "npc.hpp"
#include "path.hpp"


std::string filePrefix = "generated/clientside/data/luafiles514/lua files/navigation/";

#define SET_OPEN 0
#define SET_CLOSED 1

#define PATH_DIR_NORTH 1
#define PATH_DIR_WEST 2
#define PATH_DIR_SOUTH 4
#define PATH_DIR_EAST 8

// @name Structures and defines for A* pathfinding
// @{

// Path node
struct path_node {
	struct path_node *parent; // pointer to parent
	short x; // x coord
	short y; // y coord
	short g_cost; // Actual cost from start to this node
	short f_cost; // g_cost + heuristic(this, goal)
	short flag; // SET_OPEN / SET_CLOSED
};

/// Binary heap of path nodes
BHEAP_STRUCT_DECL(node_heap, struct path_node*);
static BHEAP_STRUCT_VAR(node_heap, g_open_set);	// use static heap for all path calculations
												// it get's initialized in do_init_path, freed in do_final_path.

/// Comparator for binary heap of path nodes (minimum cost at top)
#define NODE_MINTOPCMP(i,j) ((i)->f_cost - (j)->f_cost)

#define calc_index(x,y) (((x)+(y)*MAX_WALKPATH_NAVI) & (MAX_WALKPATH_NAVI*MAX_WALKPATH_NAVI-1))

/// Estimates the cost from (x0,y0) to (x1,y1).
/// This is inadmissible (overestimating) heuristic used by game client.
#define heuristic(x0, y0, x1, y1)	(MOVE_COST * (abs((x1) - (x0)) + abs((y1) - (y0)))) // Manhattan distance
/// @}

// Translates dx,dy into walking direction
static enum directions walk_choices [3][3] =
{
	{DIR_NORTHWEST,DIR_NORTH,DIR_NORTHEAST},
	{DIR_WEST,DIR_CENTER,DIR_EAST},
	{DIR_SOUTHWEST,DIR_SOUTH,DIR_SOUTHEAST},
};


/// @name A* pathfinding related functions
/// @{

/// Pushes path_node to the binary node_heap.
/// Ensures there is enough space in array to store new element.

#define swap_ptrcast_pathnode(a, b) swap_ptrcast(struct path_node *, a, b)

static void heap_push_node(struct node_heap *heap, struct path_node *node)
{
#ifndef __clang_analyzer__ // TODO: Figure out why clang's static analyzer doesn't like this
	BHEAP_ENSURE2(*heap, 1, 256, struct path_node **);
	BHEAP_PUSH2(*heap, node, NODE_MINTOPCMP, swap_ptrcast_pathnode);
#endif // __clang_analyzer__
}

/// Updates path_node in the binary node_heap.
static int heap_update_node(struct node_heap *heap, struct path_node *node)
{
	int i;
	ARR_FIND(0, BHEAP_LENGTH(*heap), i, BHEAP_DATA(*heap)[i] == node);
	if (i == BHEAP_LENGTH(*heap)) {
		ShowError("heap_update_node: node not found\n");
		return 1;
	}
	BHEAP_UPDATE(*heap, i, NODE_MINTOPCMP, swap_ptrcast_pathnode);
	return 0;
}
// end 1:1 copy of definitions from path.cpp


// So we don't have to allocate every time, use static structures
static struct path_node tp[MAX_WALKPATH_NAVI * MAX_WALKPATH_NAVI + 1];
static int tpused[MAX_WALKPATH_NAVI * MAX_WALKPATH_NAVI + 1];

/// Path_node processing in A* pathfinding.
/// Adds new node to heap and updates/re-adds old ones if necessary.
static int add_path(struct node_heap *heap, int16 x, int16 y, int g_cost, struct path_node *parent, int h_cost)
{
	int i = calc_index(x, y);

	if (tpused[i] && tpused[i] == 1 + (x << 16 | y)) { // We processed this node before
		if (g_cost < tp[i].g_cost) { // New path to this node is better than old one
									 // Update costs and parent
			tp[i].g_cost = g_cost;
			tp[i].parent = parent;
			tp[i].f_cost = g_cost + h_cost;
			if (tp[i].flag == SET_CLOSED) {
				heap_push_node(heap, &tp[i]); // Put it in open set again
			}
			else if (heap_update_node(heap, &tp[i])) {
				return 1;
			}
			tp[i].flag = SET_OPEN;
		}
		return 0;
	}

	if (tpused[i]) // Index is already taken; see `tp` array FIXME for details
		return 1;

	// New node
	tp[i].x = x;
	tp[i].y = y;
	tp[i].g_cost = g_cost;
	tp[i].parent = parent;
	tp[i].f_cost = g_cost + h_cost;
	tp[i].flag = SET_OPEN;
	tpused[i] = 1 + (x << 16 | y);
	heap_push_node(heap, &tp[i]);
	return 0;
}
///@}

/*==========================================
 * path search (from)->(dest)
 * wpd: path info will be written here
 * cell: type of obstruction to check for
 *
 * Note: uses global g_open_set, therefore this method can't be called in parallel or recursivly.
 *------------------------------------------*/
bool navi_path_search(struct navi_walkpath_data *wpd, const struct navi_pos *from, const struct navi_pos *dest, cell_chk cell) {
	int i, x, y, dx = 0, dy = 0;
	struct map_data *mapdata = map_getmapdata(from->m);
	struct navi_walkpath_data s_wpd;

	if (wpd == NULL)
		wpd = &s_wpd; // use dummy output variable

	if (from->m != dest->m)
		return false;

	if (!mapdata->cell)
		return false;
	
	//Do not check starting cell as that would get you stuck.
	if (from->x < 0 || from->x > mapdata->xs || from->y < 0 || from->y > mapdata->ys /*|| map_getcellp(mapdata,x0,y0,cell)*/)
		return false;

	// Check destination cell
	if (dest->x < 0 || dest->x > mapdata->xs || dest->y < 0 || dest->y > mapdata->ys || map_getcellp(mapdata, dest->x, dest->y, cell))
		return false;


	if (from->x == dest->x && from->y == dest->y) {
		wpd->path_len = 0;
		wpd->path_pos = 0;
		return true;
	}

	struct path_node *current, *it;
	int xs = mapdata->xs - 1;
	int ys = mapdata->ys - 1;
	int len = 0;
	int j;

	// A* (A-star) pathfinding
	// We always use A* for finding walkpaths because it is what game client uses.
	// Easy pathfinding cuts corners of non-walkable cells, but client always walks around it.
	BHEAP_RESET(g_open_set);

	memset(tpused, 0, sizeof(tpused));

	// Start node
	i = calc_index(from->x, from->y);
	tp[i].parent = NULL;
	tp[i].x = from->x;
	tp[i].y = from->y;
	tp[i].g_cost = 0;
	tp[i].f_cost = heuristic(from->x, from->y, dest->x, dest->y);
	tp[i].flag = SET_OPEN;
	tpused[i] = 1 + (from->x << 16 | from->y);

	heap_push_node(&g_open_set, &tp[i]); // Put start node to 'open' set
	
	for (;;) {
		int e = 0; // error flag
		
		// Saves allowed directions for the current cell. Diagonal directions
		// are only allowed if both directions around it are allowed. This is
		// to prevent cutting corner of nearby wall.
		// For example, you can only go NW from the current cell, if you can
		// go N *and* you can go W. Otherwise you need to walk around the
		// (corner of the) non-walkable cell.
		int allowed_dirs = 0;

		int g_cost;

		if (BHEAP_LENGTH(g_open_set) == 0) {
			return false;
		}

		current = BHEAP_PEEK(g_open_set); // Look for the lowest f_cost node in the 'open' set
		BHEAP_POP2(g_open_set, NODE_MINTOPCMP, swap_ptrcast_pathnode); // Remove it from 'open' set

		x = current->x;
		y = current->y;
		g_cost = current->g_cost;

		current->flag = SET_CLOSED; // Add current node to 'closed' set

		if (x == dest->x && y == dest->y) {
			break;
		}

		if (y < ys && !map_getcellp(mapdata, x, y+1, cell)) allowed_dirs |= PATH_DIR_NORTH;
		if (y >  0 && !map_getcellp(mapdata, x, y-1, cell)) allowed_dirs |= PATH_DIR_SOUTH;
		if (x < xs && !map_getcellp(mapdata, x+1, y, cell)) allowed_dirs |= PATH_DIR_EAST;
		if (x >  0 && !map_getcellp(mapdata, x-1, y, cell)) allowed_dirs |= PATH_DIR_WEST;

#define chk_dir(d) ((allowed_dirs & (d)) == (d))
		// Process neighbors of current node
		if (chk_dir(PATH_DIR_SOUTH|PATH_DIR_EAST) && !map_getcellp(mapdata, x+1, y-1, cell))
			e += add_path(&g_open_set, x+1, y-1, g_cost + MOVE_DIAGONAL_COST, current, heuristic(x+1, y-1, dest->x, dest->y)); // (x+1, y-1) 5
		if (chk_dir(PATH_DIR_EAST))
			e += add_path(&g_open_set, x+1, y, g_cost + MOVE_COST, current, heuristic(x+1, y, dest->x, dest->y)); // (x+1, y) 6
		if (chk_dir(PATH_DIR_NORTH|PATH_DIR_EAST) && !map_getcellp(mapdata, x+1, y+1, cell))
			e += add_path(&g_open_set, x+1, y+1, g_cost + MOVE_DIAGONAL_COST, current, heuristic(x+1, y+1, dest->x, dest->y)); // (x+1, y+1) 7
		if (chk_dir(PATH_DIR_NORTH))
			e += add_path(&g_open_set, x, y+1, g_cost + MOVE_COST, current, heuristic(x, y+1, dest->x, dest->y)); // (x, y+1) 0
		if (chk_dir(PATH_DIR_NORTH|PATH_DIR_WEST) && !map_getcellp(mapdata, x-1, y+1, cell))
			e += add_path(&g_open_set, x-1, y+1, g_cost + MOVE_DIAGONAL_COST, current, heuristic(x-1, y+1, dest->x, dest->y)); // (x-1, y+1) 1
		if (chk_dir(PATH_DIR_WEST))
			e += add_path(&g_open_set, x-1, y, g_cost + MOVE_COST, current, heuristic(x-1, y, dest->x, dest->y)); // (x-1, y) 2
		if (chk_dir(PATH_DIR_SOUTH|PATH_DIR_WEST) && !map_getcellp(mapdata, x-1, y-1, cell))
			e += add_path(&g_open_set, x-1, y-1, g_cost + MOVE_DIAGONAL_COST, current, heuristic(x-1, y-1, dest->x, dest->y)); // (x-1, y-1) 3
		if (chk_dir(PATH_DIR_SOUTH))
			e += add_path(&g_open_set, x, y-1, g_cost + MOVE_COST, current, heuristic(x, y-1, dest->x, dest->y)); // (x, y-1) 4
#undef chk_dir
		if (e) {
			return false;
		}
	}

	for (it = current; it->parent != NULL; it = it->parent, len++);
	if (len > sizeof(wpd->path)) {
		return false;
	}

	// Recreate path
	wpd->path_len = len;
	wpd->path_pos = 0;

	for (it = current, j = len-1; j >= 0; it = it->parent, j--) {
		dx = it->x - it->parent->x;
		dy = it->y - it->parent->y;
		wpd->path[j] = walk_choices[-dy + 1][dx + 1];
	}

	return true;
}

bool fileExists(const std::string& path) {
	std::ifstream in;
	in.open(path);

	if (in.is_open()) {
		in.close();
		return true;
	} else {
		return false;
	}
}


void write_header(std::ostream &os, std::string name) {
	os << name << " = {\n";
}

void write_footer(std::ostream &os) {
	os << "}\n";
}

// 5001 = normal, 5002 = airport/airship, 5003 = indoor maps
// 5001 = default
// 5002 = airport/airship
// 5003 = maps that are segmented
// 5005 = 5003 + monsters??? i really have no clue
//        maybe it's maps that you must leave to reach parts of it
//        for example, ptr_fild04?
int map_type(const struct map_data * m) {

	bool segmented = false;
	bool has_mob = false;

	if (strstr(m->name, "air"))
		return 5002;

	// this is n^2, yikes!
	if (std::find_if(m->navi.warps_outof.begin(), m->navi.warps_outof.end(), [&m](const navi_link* link) {
		return std::find_if(m->navi.warps_outof.begin(), m->navi.warps_outof.end(), [&link](const navi_link* link2) {
			// find if any two warps in a map cannot be reached
			return !navi_path_search(nullptr, &link->pos, &link2->pos, CELL_CHKNOREACH);
		}) != m->navi.warps_into.end();
	}) != m->navi.warps_into.end())
		segmented = true;
	
	if (m->moblist[0] != nullptr) {
		has_mob = true;
	}

	if (segmented && has_mob) {
		return 5005;
	} else if (segmented) {
		return 5003;
	}

	return 5001;
}

void write_map(std::ostream& os, const struct map_data * m) {
	os << "\t{\"" << m->name << "\", \"" << m->name << "\", ";
	os << map_type(m) << ", " << m->xs << ", " << m->ys << "},\n";
}

void write_warp(std::ostream& os, const struct navi_link &nl) {
	const struct map_data *msrc, *mdest;

	msrc = map_getmapdata(nl.pos.m);
	mdest = map_getmapdata(nl.warp_dest.m);

	if (msrc == nullptr || mdest == nullptr)
		return;

	os << "\t{";
	os << "\"" << msrc->name << "\", ";
	os << nl.id << ", "; // gid
	// 200 = warp , 201 = npc script, 202 = Kafra Dungeon Warp,
	// 203 = Cool Event Dungeon Warp, 204 Kafra/Cool Event/Alberta warp,
	// 205 = airport  (currently we only support warps)
	os << ((nl.npc->subtype == NPCTYPE_WARP) ? 200 : 201) << ", ";
	// sprite id, 99999 = warp portal
	os << ((nl.npc->vd.class_ == JT_WARPNPC) ? 99999 : (int)nl.npc->vd.class_) << ", ";
	if (nl.name.empty())
		os << "\"" << msrc->name << "_" << mdest->name << "_" << nl.id << "\", ";
	else
		os << "\"" << nl.name << "\", ";
	os << "\"\", "; // unique name
	os << nl.pos.x << ", " << nl.pos.y << ", ";
	os << "\"" << mdest->name << "\", ";
	os << nl.warp_dest.x << ", " << nl.warp_dest.y;
	os << "},\n";
}

void write_npc(std::ostream &os, const struct npc_data *nd) {
	if (nd == nullptr) {
		ShowError("Unable to find NPC ID for NPC '%s'. Skipping...\n", nd->exname);
		return;
	}

	std::string name = nd->name;
	std::string visible_name = name.substr(0, name.find('#'));

	const struct map_data *m;

	m = map_getmapdata(nd->navi.pos.m);

	os << "\t{ ";
	os << "\"" << m->name << "\", ";
	os << nd->navi.id << ", ";
	os << ((nd->subtype == NPCTYPE_SHOP || nd->subtype == NPCTYPE_CASHSHOP) ? 102 : 101) << ", ";
	os << nd->class_ << ", ";
	os << "\"" << visible_name << "\", ";
	os << "\"\", ";
	os << nd->navi.pos.x << ", " << nd->navi.pos.y;
	os << "},\n";
}

void write_spawn(std::ostream &os, const struct map_data * m, const std::shared_ptr<s_mob_db> mobinfo, int amount, int idx) {

	os << "\t{";
	os << "\"" << m->name << "\", ";
	os << "" << idx << ", ";
	os << "" << (mobinfo->mexp ? 301 : 300) << ", ";
#if PACKETVER >= 20140000
	os << "" << (amount << 16 | (mobinfo->vd.class_ & 0xFFFF)) << ", ";
#else
	os << "\t\t" << mobinfo->vd.class_ << ", ";
#endif
	os << "\"" << mobinfo->jname.c_str() << "\", "; //c_str'ed because the strings have been resized to 24
	os << "\"" << mobinfo->sprite.c_str() << "\", ";
	os << "" << mobinfo->lv << ", ";
	os << "" << 
		(((mobinfo->status.ele_lv * 20 + mobinfo->status.def_ele) << 16)
		| (mobinfo->status.size << 8)
		| mobinfo->status.race);
	os << "},\n";

}

void write_object_lists() {
	auto mob_file = std::ofstream(filePrefix + "./navi_mob_krpri.lub");
	auto links_file = std::ofstream(filePrefix + "./navi_link_krpri.lub");
	auto npc_file = std::ofstream(filePrefix + "./navi_npc_krpri.lub");
	auto map_file = std::ofstream(filePrefix + "./navi_map_krpri.lub");

	if (!mob_file) {
		ShowError("Failed to create mobfile.\n");
		ShowError("Maybe the file directory \"%s\" does not exist?\n", filePrefix.c_str());
		ShowInfo("Create the directory and rerun map-server-generator\n");
		exit(1);
	}

	int warp_count = 0;
	int npc_count = 0;
	int spawn_count = 0;

	write_header(links_file, "Navi_Link");
	write_header(npc_file, "Navi_Npc");
	write_header(mob_file, "Navi_Mob");
	write_header(map_file, "Navi_Map");

	for (int mapid = 0; mapid < map_num; mapid++) {
		auto m = map_getmapdata(mapid);

		// Warps/NPCs
		for (int npcidx = 0; npcidx < m->npc_num; npcidx++) {
			struct npc_data *nd = m->npc[npcidx];

			if (nd == nullptr)
				continue;

			if (nd->subtype == NPCTYPE_WARP) {
				int target = nd->navi.warp_dest.m;
				if (target < 0)
					continue;

				nd->navi.id = 13350 + warp_count++;
				write_warp(links_file, nd->navi);
				m->navi.warps_outof.push_back(&nd->navi);
				map[target].navi.warps_into.push_back(&nd->navi);
				
			} else { // Other NPCs
				if (nd->class_ == -1 || nd->class_ == JT_HIDDEN_NPC
					|| nd->class_ == JT_HIDDEN_WARP_NPC || nd->class_ == JT_GUILD_FLAG
					|| nd->navi.hidden)
					continue;
				
				nd->navi.id = 11984 + npc_count;
				write_npc(npc_file, nd);
				m->navi.npcs.push_back(nd);

				for (auto &link : nd->links) {
					int target = link.warp_dest.m;
					if (target < 0)
						continue;
					
					link.id = 13350 + warp_count++;
					write_warp(links_file, link);
					m->navi.warps_outof.push_back(&link);
					map[target].navi.warps_into.push_back(&link);
				}

				npc_count++;
			}
		}

		// Mobs
		for (int mobidx = 0; mobidx < MAX_MOB_LIST_PER_MAP; mobidx++) {
			if (m->moblist[mobidx] == nullptr)
				continue;

			const auto mobinfo = mob_db.find(m->moblist[mobidx]->id);
			if (mobinfo == nullptr)
				continue;
			
			write_spawn(mob_file, m, mobinfo, m->moblist[mobidx]->num, 17104 + spawn_count);
			spawn_count++;
		}
		write_map(map_file, m);
	}

	ShowStatus("Generated %d maps\n", map_num);
	ShowStatus("Generated %d npcs\n", npc_count);
	ShowStatus("Generated %d mobs\n", spawn_count);
	ShowStatus("Generated %d links\n", warp_count);

	write_footer(map_file);
	write_footer(links_file);
	write_footer(npc_file);
	write_footer(mob_file);
}

void write_map_header(std::ostream &os, const struct map_data * m) {
	os << "\t\"" << m->name << "\", " << m->navi.npcs.size() << ",\n";
	os << "\t{\n";
}


/*
	Given an NPC in a map (nd)
		For every warp into the map (warp)
			Find a path from nd to warp
*/
void write_npc_distance(std::ostream &os, const struct npc_data * nd, const struct map_data * msrc) {
	os << "\t\t{ " << nd->navi.id << ", -- (" << nd->name << " " << msrc->name << ", " << nd->navi.pos.x << ", " << nd->navi.pos.y << ")\n";
	for (const auto warp : msrc->navi.warps_into) {
		struct navi_walkpath_data wpd = {0};
		auto mdest = map_getmapdata(warp->pos.m);

		// Find a path from the npc to the warp destination
		// The warp is into the map, so this makes sense
		if (!navi_path_search(&wpd, &nd->navi.pos, &warp->warp_dest, CELL_CHKNOREACH)) {
			continue;
		}

		os << "\t\t\t{ \"" << mdest->name << "\", " << warp->id << ", " << std::to_string(wpd.path_len) << "}, -- (" << msrc->name << ", " << warp->pos.x << ", " << warp->pos.y << ")\n";
	}
	os << "\t\t\t{\"\", 0, 0}\n";
	os << "\t\t},\n";
}

void write_npc_distances() {
	auto dist_npc_file = std::ofstream(filePrefix + "./navi_npcdistance_krpri.lub");

	write_header(dist_npc_file, "Navi_NpcDistance");

	for (int mapid = 0; mapid < map_num; mapid++) {
		auto m = map_getmapdata(mapid);
#ifdef DETAILED_LOADING_OUTPUT
		ShowStatus("Loading [%i/%i]" CL_CLL "\r", mapid, map_num);
#endif
		if (m->navi.npcs.size() == 0) {
			// ShowStatus("Skipped %s NPC distance table, no NPCs in map (%d/%d)\n", map[m].name, m, map_num);
			continue;
		}
		if (m->navi.warps_into.size() == 0) {
			// ShowStatus("Skipped %s NPC distance table, no warps into map (%d/%d)\n", map[m].name, m, map_num);
			continue;
		}

		write_map_header(dist_npc_file, m);
		for (auto nd : m->navi.npcs) {
			write_npc_distance(dist_npc_file, nd, m);
		}
		dist_npc_file << "\t},\n";
	}

	ShowStatus("Generated NPC Distances for %d maps\n", map_num);

	write_footer(dist_npc_file);
}

void write_mapdist_header(std::ostream &os, const struct map_data * m) {
	os << "\t\"" << m->name << "\", " << m->navi.warps_outof.size() << ",\n";
	os << "\t{\n";
}


/*
	Given a warp out of a map (warp1, m)
		for every warp out of the map (warp2)
			find a path from the warp1->src to warp2->src
			Add this as a "P" Warp

		for every warp out of warp1->dest.m (warp3)
			find a path from warp1->dest to warp3->src
			Add this as an "E" Warp
 */
void write_map_distance(std::ostream &os, const struct navi_link * warp1, const struct map_data * m) {
	os << "\t\t{ " << warp1->id << ", -- (" << " " << m->name << ", " << warp1->pos.x << ", " << warp1->pos.y << ")\n";
	// for (const auto warp2 : m->navi.warps_outof) {
	// 	struct navi_walkpath_data wpd = {0};
	// 	if (warp1 == warp2)
	// 		continue;
	// 	if (!navi_path_search(&wpd, &warp1->pos, &warp2->pos, CELL_CHKNOREACH))
	// 		continue;
	// 	os << "\t\t\t{ \"P\", " << warp2->id << ", " << std::to_string(wpd.path_len) << "}, -- ReachableFromSrc warp (" << m->name << ", " << warp2->pos.x << ", " << warp2->pos.y << ")\n";
	// }

	for (const auto warp3 : map_getmapdata(warp1->warp_dest.m)->navi.warps_outof) {
		struct navi_walkpath_data wpd = {0};

		if (!navi_path_search(&wpd, &warp1->warp_dest, &warp3->pos, CELL_CHKNOREACH))
			continue;
		
		os << "\t\t\t{ \"E\", " << warp3->id << ", " << std::to_string(wpd.path_len) << "}, -- ReachableFromDst warp (" << map_getmapdata(warp3->pos.m)->name << ", " << warp3->pos.x << ", " << warp3->pos.y << ")\n";
	}

	os << "\t\t\t{\"NULL\", 0, 0}\n";
	os << "\t\t},\n";
}

void write_map_distances() {
	auto dist_map_file = std::ofstream(filePrefix + "./navi_linkdistance_krpri.lub");
	write_header(dist_map_file, "Navi_Distance");

	for (int mapid = 0; mapid < map_num; mapid++) {
		const struct map_data * m = map_getmapdata(mapid);
		write_mapdist_header(dist_map_file, m);
		for (auto nd : m->navi.warps_outof) {
			write_map_distance(dist_map_file, nd, m);
		}
		dist_map_file << "\t},\n";
	}
	ShowStatus("Generated Map Distances for %d maps\n", map_num);
	write_footer(dist_map_file);
}


void navi_create_lists() {
	BHEAP_INIT(g_open_set);

	auto starttime = std::chrono::system_clock::now();

	npc_event_runall(script_config.navi_generate_name);

	write_object_lists();
	auto currenttime = std::chrono::system_clock::now();
	ShowInfo("Object lists took %ums\n", std::chrono::duration_cast<std::chrono::milliseconds>(currenttime - starttime));
	starttime = std::chrono::system_clock::now();
	write_npc_distances();
	currenttime = std::chrono::system_clock::now();
	ShowInfo("NPC Distances took %ums\n", std::chrono::duration_cast<std::chrono::milliseconds>(currenttime - starttime));
	starttime = std::chrono::system_clock::now();
	write_map_distances();
	currenttime = std::chrono::system_clock::now();
	ShowInfo("Link Distances took %ums\n", std::chrono::duration_cast<std::chrono::milliseconds>(currenttime - starttime));

	BHEAP_CLEAR(g_open_set);
}

#endif
