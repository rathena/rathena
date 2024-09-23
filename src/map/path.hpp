// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PATH_HPP
#define PATH_HPP

#include <common/cbasetypes.hpp>

enum cell_chk : uint8;

#define MOVE_COST 10
#define MOVE_DIAGONAL_COST 14

#define MAX_WALKPATH 32

#define SET_OPEN false
#define SET_CLOSED true

#define PATH_DIR_NORTH 1
#define PATH_DIR_WEST 2
#define PATH_DIR_SOUTH 4
#define PATH_DIR_EAST 8

/// @name Structures and defines for A* pathfinding
/// @{

/// Path node
struct path_node {
	struct path_node *parent; ///< pointer to parent (for path reconstruction)
	unsigned short x; ///< X-coordinate
	unsigned short y; ///< Y-coordinate
	unsigned short g_cost; ///< Actual cost from start to this node
	unsigned short f_cost; ///< g_cost + heuristic(this, goal)
	bool flag; ///< SET_OPEN / SET_CLOSED
};

/// Comparator for binary heap of path nodes (minimum cost at top)
#define NODE_MINTOPCMP(i,j) ((i)->f_cost - (j)->f_cost)

enum directions : int8 {
	DIR_CENTER = -1,
	DIR_NORTH = 0,
	DIR_NORTHWEST = 1,
	DIR_WEST = 2,
	DIR_SOUTHWEST = 3,
	DIR_SOUTH = 4,
	DIR_SOUTHEAST = 5,
	DIR_EAST = 6,
	DIR_NORTHEAST = 7,
	DIR_MAX
};

// Translates dx,dy into walking direction
constexpr enum directions walk_choices [3][3] =
{
	{DIR_NORTHWEST,DIR_NORTH,DIR_NORTHEAST},
	{DIR_WEST,DIR_CENTER,DIR_EAST},
	{DIR_SOUTHWEST,DIR_SOUTH,DIR_SOUTHEAST},
};

struct walkpath_data {
	unsigned char path_len,path_pos;
	enum directions path[MAX_WALKPATH];
};

struct shootpath_data {
	int rx,ry,len;
	int x[MAX_WALKPATH];
	int y[MAX_WALKPATH];
};

#define check_distance_bl(bl1, bl2, distance) check_distance((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y, distance)
#define check_distance_blxy(bl, x1, y1, distance) check_distance((bl)->x-(x1), (bl)->y-(y1), distance)
#define check_distance_xy(x0, y0, x1, y1, distance) check_distance((x0)-(x1), (y0)-(y1), distance)

#define distance_bl(bl1, bl2) distance((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y)
#define distance_blxy(bl, x1, y1) distance((bl)->x-(x1), (bl)->y-(y1))
#define distance_xy(x0, y0, x1, y1) distance((x0)-(x1), (y0)-(y1))

#define check_distance_client_bl(bl1, bl2, distance) check_distance_client((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y, distance)
#define check_distance_client_blxy(bl, x1, y1, distance) check_distance_client((bl)->x-(x1), (bl)->y-(y1), distance)
#define check_distance_client_xy(x0, y0, x1, y1, distance) check_distance_client((x0)-(x1), (y0)-(y1), distance)

#define distance_client_bl(bl1, bl2) distance_client((bl1)->x - (bl2)->x, (bl1)->y - (bl2)->y)
#define distance_client_blxy(bl, x1, y1) distance_client((bl)->x-(x1), (bl)->y-(y1))
#define distance_client_xy(x0, y0, x1, y1) distance_client((x0)-(x1), (y0)-(y1))

// calculates destination cell for knockback
int path_blownpos(int16 m,int16 x0,int16 y0,int16 dx,int16 dy,int count);

// tries to find a walkable path
bool path_search(struct walkpath_data *wpd, int16 m, uint16 x0, uint16 y0, uint16 x1, uint16 y1, int flag, cell_chk cell);

// tries to find a shootable path
bool path_search_long(struct shootpath_data *spd,int16 m,int16 x0,int16 y0,int16 x1,int16 y1,cell_chk cell);

// distance related functions
bool check_distance(short dx, short dy, unsigned short distance);
unsigned short distance(short dx, short dy);
bool check_distance_client(short dx, short dy, unsigned short distance);
unsigned short distance_client(short dx, short dy);

bool direction_diagonal( enum directions direction );
bool direction_opposite( enum directions direction );

//
void do_init_path();
void do_final_path();

#endif /* PATH_HPP */
