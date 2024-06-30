// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "path.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <common/cbasetypes.hpp>
#include <common/db.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>

#include "battle.hpp"
#include "map.hpp"

/// @name Structures and defines for A* pathfinding
/// @{

/// Binary Heap
static open_heap open_set;

#define calc_index(x,y) (((x)+(y)*MAX_WALKPATH) & (MAX_WALKPATH*MAX_WALKPATH-1))

void do_init_path(){
}//

void do_final_path(){
}//


/*==========================================
 * Find the closest reachable cell, 'count' cells away from (x0,y0) in direction (dx,dy).
 * Income after the coordinates of the blow
 *------------------------------------------*/
int path_blownpos(int16 m,int16 x0,int16 y0,int16 dx,int16 dy,int count)
{
	struct map_data *mapdata = map_getmapdata(m);

	if( !mapdata->cell )
		return -1;

	if( count>25 ){ //Cap to prevent too much processing...?
		ShowWarning("path_blownpos: count too many %d !\n",count);
		count=25;
	}
	if( dx > 1 || dx < -1 || dy > 1 || dy < -1 ){
		ShowError("path_blownpos: illegal dx=%d or dy=%d !\n",dx,dy);
		dx=(dx>0)?1:((dx<0)?-1:0);
		dy=(dy>0)?1:((dy<0)?-1:0);
	}

	while( count > 0 && (dx != 0 || dy != 0) )
	{
		if( !map_getcellp(mapdata,x0+dx,y0+dy,CELL_CHKPASS) )
		{
			if (battle_config.path_blown_halt)
				break;
			else
			{// attempt partial movement
				int fx = ( dx != 0 && map_getcellp(mapdata,x0+dx,y0,CELL_CHKPASS) );
				int fy = ( dy != 0 && map_getcellp(mapdata,x0,y0+dy,CELL_CHKPASS) );
				if( fx && fy )
				{
					if(rnd_chance(50, 100))
						dx=0;
					else
						dy=0;
				}
				if( !fx )
					dx=0;
				if( !fy )
					dy=0;
			}
		}

		x0 += dx;
		y0 += dy;
		count--;
	}

	return (x0<<16)|y0; //TODO: use 'struct point' here instead?
}

/*==========================================
 * is ranged attack from (x0,y0) to (x1,y1) possible?
 *------------------------------------------*/
bool path_search_long(struct shootpath_data *spd,int16 m,int16 x0,int16 y0,int16 x1,int16 y1,cell_chk cell)
{
	int dx, dy;
	int wx = 0, wy = 0;
	int weight;
	struct map_data *mapdata = map_getmapdata(m);
	struct shootpath_data s_spd;

	if( spd == nullptr )
		spd = &s_spd; // use dummy output variable

	if (!mapdata->cell)
		return false;

	dx = (x1 - x0);
	if (dx < 0) {
		std::swap(x0, x1);
		std::swap(y0, y1);
		dx = -dx;
	}
	dy = (y1 - y0);

	spd->rx = spd->ry = 0;
	spd->len = 1;
	spd->x[0] = x0;
	spd->y[0] = y0;

	if (dx > abs(dy)) {
		weight = dx;
		spd->ry = 1;
	} else {
		weight = abs(y1 - y0);
		spd->rx = 1;
	}

	while (x0 != x1 || y0 != y1)
	{
		wx += dx;
		wy += dy;
		if (wx >= weight) {
			wx -= weight;
			x0++;
		}
		if (wy >= weight) {
			wy -= weight;
			y0++;
		} else if (wy < 0) {
			wy += weight;
			y0--;
		}
		if( spd->len<MAX_WALKPATH )
		{
			spd->x[spd->len] = x0;
			spd->y[spd->len] = y0;
			spd->len++;
		}
		if ((x0 != x1 || y0 != y1) && map_getcellp(mapdata,x0,y0,cell))
			return false;
	}

	return true;
}

/// @name A* pathfinding related functions
/// @{

/// Path_node processing in A* pathfinding.
/// Adds new node to heap and updates/re-adds old ones if necessary.
static short add_path(std::vector<path_node>& tp, int16 x, int16 y, int g_cost, struct path_node* parent, int h_cost)
{
	int i = calc_index(x, y);

	if (tp[i].x == x && tp[i].y == y) { // We processed this node before
		if (g_cost < tp[i].g_cost) { // New path to this node is better than old one
			// Update costs and parent
			tp[i].g_cost = g_cost;
			tp[i].parent = parent;
			tp[i].f_cost = g_cost + h_cost;
			if (tp[i].flag == SET_CLOSED)
				open_set.push_node(&tp[i]); // Put node to 'open' set
			else if (open_set.update_node(&tp[i]))
				return 1; // return error if node not found on heap 
			tp[i].flag = SET_OPEN;
		}
		return 0;
	}

	if (tp[i].x || tp[i].y) // Index is already taken; update ex node
		return open_set.update_node(&tp[i], true);

	// New node
	tp[i].x = x;
	tp[i].y = y;
	tp[i].g_cost = g_cost;
	tp[i].parent = parent;
	tp[i].f_cost = g_cost + h_cost;
	tp[i].flag = SET_OPEN;
	open_set.push_node(&tp[i]); // Put node to 'open' set
	return 0;
}
///@}

/*==========================================
 * path search (x0,y0)->(x1,y1)
 * wpd: path info will be written here
 * flag: &1 = easy path search only
 * flag: &2 = call path_search_long instead
 * cell: type of obstruction to check for
 *
 * Note: uses global g_open_set, therefore this method can't be called in parallel or recursivly.
 *------------------------------------------*/
bool path_search(struct walkpath_data *wpd, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int flag, cell_chk cell)
{
	int i, x, y, dx = 0, dy = 0;
	struct map_data *mapdata = map_getmapdata(m);
	struct walkpath_data s_wpd;

	if (flag&2)
		return path_search_long(nullptr, m, x0, y0, x1, y1, cell);

	if (wpd == nullptr)
		wpd = &s_wpd; // use dummy output variable

	if (!mapdata->cell)
		return false;

	//Do not check starting cell as that would get you stuck.
	if (x0 < 0 || x0 >= mapdata->xs || y0 < 0 || y0 >= mapdata->ys /*|| map_getcellp(mapdata,x0,y0,cell)*/)
		return false;

	// Check destination cell
	if (x1 < 0 || x1 >= mapdata->xs || y1 < 0 || y1 >= mapdata->ys || map_getcellp(mapdata,x1,y1,cell))
		return false;

	if (flag&1) {
		// Try finding direct path to target
		// Direct path goes diagonally first, then in straight line.

		// calculate (sgn(x1-x0), sgn(y1-y0))
		dx = ((dx = x1-x0)) ? ((dx<0) ? -1 : 1) : 0;
		dy = ((dy = y1-y0)) ? ((dy<0) ? -1 : 1) : 0;

		x = x0; // Current position = starting cell
		y = y0;
		i = 0;
		while( i < ARRAYLENGTH(wpd->path) ) {
			wpd->path[i] = walk_choices[-dy + 1][dx + 1];
			i++;

			x += dx; // Advance current position
			y += dy;

			if( x == x1 ) dx = 0; // destination x reached, no longer move along x-axis
			if( y == y1 ) dy = 0; // destination y reached, no longer move along y-axis

			if( dx == 0 && dy == 0 )
				break; // success
			if( map_getcellp(mapdata,x,y,cell) )
				break; // obstacle = failure
		}

		if( x == x1 && y == y1 ) { // easy path successful.
			wpd->path_len = i;
			wpd->path_pos = 0;
			return true;
		}

		return false; // easy path unsuccessful
	} else { // !(flag&1)
		std::vector<path_node> tp(MAX_WALKPATH * MAX_WALKPATH);
		struct path_node *current = nullptr, *it;
		int xs = mapdata->xs - 1;
		int ys = mapdata->ys - 1;
		int len = 0;
		int j;

		// A* (A-star) pathfinding
		// We always use A* for finding walkpaths because it is what game client uses.
		// Easy pathfinding cuts corners of non-walkable cells, but client always walks around it.
		open_set.clear(); // Clear 'open' set

		// Start node
		i = calc_index(x0, y0);
		tp[i].parent = nullptr;
		tp[i].x      = x0;
		tp[i].y      = y0;
		tp[i].g_cost = 0;
		tp[i].f_cost = heuristic(x0, y0, x1, y1);
		tp[i].flag   = SET_OPEN;

		open_set.push_node(&tp[i]); // Put node to 'open' set

		for(;;) {
			short e = 0; // error flag

			// Saves allowed directions for the current cell. Diagonal directions
			// are only allowed if both directions around it are allowed. This is
			// to prevent cutting corner of nearby wall.
			// For example, you can only go NW from the current cell, if you can
			// go N *and* you can go W. Otherwise you need to walk around the
			// (corner of the) non-walkable cell.
			int allowed_dirs = 0;

			int g_cost;

			if (open_set.empty())
				return false;

			current = open_set.front(); 
			open_set.pop_heap(); // Remove it from 'open' set 

			x      = current->x;
			y      = current->y;
			g_cost = current->g_cost;

			current->flag = SET_CLOSED; // Add current node to 'closed' set

			if (x == x1 && y == y1) {
				break;
			}

			if (y < ys && !map_getcellp(mapdata, x, y+1, cell)) allowed_dirs |= PATH_DIR_NORTH;
			if (y >  0 && !map_getcellp(mapdata, x, y-1, cell)) allowed_dirs |= PATH_DIR_SOUTH;
			if (x < xs && !map_getcellp(mapdata, x+1, y, cell)) allowed_dirs |= PATH_DIR_EAST;
			if (x >  0 && !map_getcellp(mapdata, x-1, y, cell)) allowed_dirs |= PATH_DIR_WEST;

#define chk_dir(d) ((allowed_dirs & (d)) == (d))
			// Process neighbors of current node
			if (chk_dir(PATH_DIR_SOUTH|PATH_DIR_EAST) && !map_getcellp(mapdata, x+1, y-1, cell))
				e += add_path(tp, x+1, y-1, g_cost+MOVE_DIAGONAL_COST, current, heuristic(x+1, y-1, x1, y1)); // (x+1, y-1) 5
			if (chk_dir(PATH_DIR_EAST))
				e += add_path(tp, x+1, y, g_cost+MOVE_COST, current, heuristic(x+1, y, x1, y1)); // (x+1, y) 6
			if (chk_dir(PATH_DIR_NORTH|PATH_DIR_EAST) && !map_getcellp(mapdata, x+1, y+1, cell))
				e += add_path(tp, x+1, y+1, g_cost+MOVE_DIAGONAL_COST, current, heuristic(x+1, y+1, x1, y1)); // (x+1, y+1) 7
			if (chk_dir(PATH_DIR_NORTH))
				e += add_path(tp, x, y+1, g_cost+MOVE_COST, current, heuristic(x, y+1, x1, y1)); // (x, y+1) 0
			if (chk_dir(PATH_DIR_NORTH|PATH_DIR_WEST) && !map_getcellp(mapdata, x-1, y+1, cell))
				e += add_path(tp, x-1, y+1, g_cost+MOVE_DIAGONAL_COST, current, heuristic(x-1, y+1, x1, y1)); // (x-1, y+1) 1
			if (chk_dir(PATH_DIR_WEST))
				e += add_path(tp, x-1, y, g_cost + MOVE_COST, current, heuristic(x-1, y, x1, y1)); // (x-1, y) 2
			if (chk_dir(PATH_DIR_SOUTH|PATH_DIR_WEST) && !map_getcellp(mapdata, x-1, y-1, cell))
				e += add_path(tp, x-1, y-1, g_cost + MOVE_DIAGONAL_COST, current, heuristic(x-1, y-1, x1, y1)); // (x-1, y-1) 3
			if (chk_dir(PATH_DIR_SOUTH))
				e += add_path(tp, x, y-1, g_cost+MOVE_COST, current, heuristic(x, y-1, x1, y1)); // (x, y-1) 4
#undef chk_dir
			if (e) {
				return false;
			}
		}

		for (it = current; it->parent != nullptr; it = it->parent, len++);
		if (len > sizeof(wpd->path))
			return false;

		// Recreate path
		wpd->path_len = len;
		wpd->path_pos = 0;

		for (it = current, j = len-1; j >= 0; it = it->parent, j--) {
			dx = it->x - it->parent->x;
			dy = it->y - it->parent->y;
			wpd->path[j] = walk_choices[-dy + 1][dx + 1];
		}

		return true;
	} // A* end

	return false;
}


//Distance functions, taken from http://www.flipcode.com/articles/article_fastdistance.shtml
bool check_distance(int dx, int dy, int distance)
{
#ifdef CIRCULAR_AREA
	//In this case, we just do a square comparison. Add 1 tile grace for diagonal range checks.
	return (dx*dx + dy*dy <= distance*distance + (dx&&dy?1:0));
#else
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	return ((dx<dy?dy:dx) <= distance);
#endif
}

unsigned int distance(int dx, int dy)
{
#ifdef CIRCULAR_AREA
	unsigned int min, max;

	if ( dx < 0 ) dx = -dx;
	if ( dy < 0 ) dy = -dy;
	//There appears to be something wrong with the approximation below when either dx/dy is 0! [Skotlex]
	if ( dx == 0 ) return dy;
	if ( dy == 0 ) return dx;

	if ( dx < dy )
	{
		min = dx;
		max = dy;
	} else {
		min = dy;
		max = dx;
	}
   // coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
	return ((( max << 8 ) + ( max << 3 ) - ( max << 4 ) - ( max << 1 ) +
		( min << 7 ) - ( min << 5 ) + ( min << 3 ) - ( min << 1 )) >> 8 );
#else
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	return (dx<dy?dy:dx);
#endif
}

/**
 * The client uses a circular distance instead of the square one. The circular distance
 * is only used by units sending their attack commands via the client (not monsters).
 * @param dx: Horizontal distance
 * @param dy: Vertical distance
 * @param distance: Distance to check against
 * @return Within distance(1); Not within distance(0);
 */
bool check_distance_client(int dx, int dy, int distance)
{
	if(distance < 0) distance = 0;

	return (distance_client(dx,dy) <= distance);
}

/**
 * The client uses a circular distance instead of the square one. The circular distance
 * is only used by units sending their attack commands via the client (not monsters).
 * @param dx: Horizontal distance
 * @param dy: Vertical distance
 * @return Circular distance
 */
int distance_client(int dx, int dy)
{
	double temp_dist = sqrt((double)(dx*dx + dy*dy));

	//Bonus factor used by client
	//This affects even horizontal/vertical lines so they are one cell longer than expected
	temp_dist -= 0.1;

	if(temp_dist < 0) temp_dist = 0;

	return ((int)temp_dist);
}

bool direction_diagonal( enum directions direction ){
	return direction == DIR_NORTHWEST || direction == DIR_SOUTHWEST || direction == DIR_SOUTHEAST || direction == DIR_NORTHEAST;
}

bool direction_opposite( enum directions direction ){
	if( direction == DIR_CENTER || direction == DIR_MAX ){
		return direction;
	}else{
		return static_cast<enum directions>( ( direction + DIR_MAX / 2 ) % DIR_MAX );
	}
}
