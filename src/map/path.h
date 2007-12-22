// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PATH_H_
#define _PATH_H_

// calculates destination cell for knockback
int path_blownpos(int m,int x0,int y0,int dx,int dy,int count);

// tries to find a walkable path
bool path_search(struct walkpath_data *wpd,int m,int x0,int y0,int x1,int y1,int flag,cell_t cell);

// tries to find a shootable path
bool path_search_long(struct shootpath_data *spd,int m,int x0,int y0,int x1,int y1,cell_t cell);

#endif /* _PATH_H_ */
