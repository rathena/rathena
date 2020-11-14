#ifndef NAVI_H
#define NAVI_H

#include "../config/core.hpp"

#ifdef GENERATE_NAVI
struct navi_pos {
	int m;
	int x;
	int y;
};

// We need a bigger max path length than stock walkpath
#define MAX_WALKPATH_NAVI 1024

struct navi_walkpath_data {
	uint8 path_len, path_pos;
	uint8 path[MAX_WALKPATH_NAVI];
};


void create_lists();
#endif // ifdef GENERATE_NAVI
#endif
