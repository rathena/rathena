// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "grfio.h"

char grf_list_file[256] = "tools/mapcache/grf_files.txt";
char map_list_file[256] = "tools/mapcache/map_list.txt";
char map_cache_file[256] = "map_cache.dat";

#define MAP_NAME_LENGTH 16
#define NO_WATER 1000000

// Used internally, this structure contains the physical map cells
struct map_data {
	short xs;
	short ys;
	unsigned char *cells;
};

// This is the header appended before every compressed map cells info
struct map_cache_info {
	char name[MAP_NAME_LENGTH];
	unsigned short index;
	short xs;
	short ys;
	long len;
};

// This is the main header found at the very beginning of the file
struct map_cache_head {
	short sizeof_header;
	short sizeof_mapinfo;
	long filesize;
	unsigned short map_count;
} header;

FILE *map_cache_fp;


// Read map from GRF's GAT and RSW files
int read_map(char *name, struct map_data *m)
{
	char filename[256];
	char *gat, *rsw;
	int water_height;
	int x, y, xs, ys;
	struct gat_cell {
		float height[4];
		int type;
	} *p = NULL;

	// Open map GAT
	sprintf(filename,"data\\%s.gat", name);
	gat = (char *)grfio_read(filename);
	if (gat == NULL)
		return 0;

	// Open map RSW
	sprintf(filename,"data\\%s.rsw", name);
	rsw = (char *)grfio_read(filename);

	// Read water height
	if (rsw) { 
		float temp = *(float*)(rsw+166);
		water_height = (int)temp;
		free(rsw);
	} else
		water_height = NO_WATER;

	// Read map size and allocate needed memory
	xs = m->xs = *(int*)(gat+6);
	ys = m->ys = *(int*)(gat+10);
	m->cells = (unsigned char *)malloc(xs*ys);

	// Set cell properties
	for (y = 0; y < ys; y++) {
		p = (struct gat_cell*)(gat+14+y*xs*20);
		for (x = 0; x < xs; x++) {
			if (water_height != NO_WATER && p->type == 0 && (p->height[0] > water_height || p->height[1] > water_height || p->height[2] > water_height || p->height[3] > water_height))
				m->cells[x+y*xs] = 3; // Cell is 0 (walkable) but under water level, set to 3 (walkable water)
			else
				m->cells[x+y*xs] = p->type;
			p++;
		}
	}

	free(gat);

	return 1;
}

void cache_map(char *name, unsigned short index, struct map_data *m)
{
	struct map_cache_info info;
	unsigned long len;
	char *write_buf;

	// Create an output buffer twice as big as the uncompressed map... this way we're sure it fits
	len = m->xs*m->ys*2;
	write_buf = (char *)malloc(len);
	// Compress the cells and get the compressed length
	encode_zip((unsigned char *)write_buf, &len, m->cells, m->xs*m->ys);

	// Fill the map header
	strncpy(info.name, name, MAP_NAME_LENGTH);
	info.index = index;
	info.xs = m->xs;
	info.ys = m->ys;
	info.len = len;

	// Append map header then compressed cells at the end of the file
	fseek(map_cache_fp, header.filesize, SEEK_SET);
	fwrite(&info, sizeof(struct map_cache_info), 1, map_cache_fp);
	fwrite(write_buf, 1, len, map_cache_fp);
	header.map_count++;
	header.filesize += header.sizeof_mapinfo + len;

	free(write_buf);
	free(m->cells);

	return;
}

int main(int argc, char *argv[])
{
	FILE *list;
	char line[1024];
	struct map_data map;
	char name[MAP_NAME_LENGTH];
	unsigned short index = 1;

	if(argc > 1)
		strcpy(grf_list_file, argv[1]);
	if(argc > 2)
		strcpy(map_list_file, argv[2]);
	if(argc > 3)
		strcpy(map_cache_file, argv[3]);

	printf("Initializing grfio with %s\n", grf_list_file);
	grfio_init(grf_list_file);

	printf("Opening map cache: %s\n", map_cache_file);
	if(!(map_cache_fp = fopen(map_cache_file, "wb"))) {
		printf("Failure when opening map cache file %s\n", map_cache_file);
		exit(1);
	}

	printf("Opening map list: %s\n", map_list_file);
	if(!(list = fopen(map_list_file, "r"))) {
		printf("Failure when opening maps list file %s\n", map_list_file);
		exit(1);
	}

	// Initialize the main header
	header.sizeof_header = sizeof(struct map_cache_head);
	header.sizeof_mapinfo = sizeof(struct map_cache_info);
	header.map_count = 0;
	header.filesize = sizeof(struct map_cache_head);

	// Read and process the map list
	while(fgets(line, 1020, list)){

		if(line[0] == '/' && line[1] == '/')
			continue;

		if(sscanf(line, "%16s %hu", name, &index) > 0) { // No defines in strings, 16 is hardcoded here
				printf("Index %d : %s\n", index, name);
				if(read_map(name, &map))
					cache_map(name, index, &map);
				else
					printf("Map file not found in GRF\n");
				// If the 2nd argument is omitted at next line, we'll keep last used index + 1
				index++;
		} else
				printf("Skipping incorrect line\n");
	}

	printf("Closing map list: %s\n", map_list_file);
	fclose(list);

	printf("Closing map cache: %s\n", map_cache_file);
	// Write the main header and close the map cache
	fseek(map_cache_fp, 0, SEEK_SET);
	fwrite(&header, sizeof(struct map_cache_head), 1, map_cache_fp);
	fclose(map_cache_fp);

	printf("Finalizing grfio\n");
	grfio_final();

	printf("%d maps cached\n", header.map_count);

	return 0;
}
