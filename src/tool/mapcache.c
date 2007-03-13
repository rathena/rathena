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

char grf_list_file[256] = "db/grf_files.txt";
char map_list_file[256] = "db/map_list.txt";
char map_cache_file[256] = "db/map_cache.dat";

#define MAP_NAME_LENGTH 16
#define NO_WATER 1000000

// Used internally, this structure contains the physical map cells
struct map_data {
	short xs;
	short ys;
	unsigned char *cells;
};

// This is the main header found at the very beginning of the file
unsigned short map_count;

// This is the header appended before every compressed map cells info
struct map_cache_info {
	char name[MAP_NAME_LENGTH];
	unsigned short index;
	short xs;
	short ys;
	long len;
};

FILE *map_cache_fp;

int filesize;

/// Converts an unsigned short (16 bits) from current machine order to little-endian
unsigned short MakeUShortLE(unsigned short val)
{
	unsigned char buf[2];
	buf[0] = (unsigned char)( (val & 0x00FF)         );
	buf[1] = (unsigned char)( (val & 0xFF00) >> 0x08 );
	return *((unsigned short*)buf);
}

/// Converts a short (16 bits) from current machine order to little-endian
short MakeShortLE(short val)
{
	unsigned char buf[2];
	buf[0] = (unsigned char)( (val & 0x00FF)         );
	buf[1] = (unsigned char)( (val & 0xFF00) >> 0x08 );
	return *((short*)buf);
}

/// Converts a long (32 bits) from current machine order to little-endian
long MakeLongLE(long val)
{
	unsigned char buf[4];
	buf[0] = (unsigned char)( (val & 0x000000FF)         );
	buf[1] = (unsigned char)( (val & 0x0000FF00) >> 0x08 );
	buf[2] = (unsigned char)( (val & 0x00FF0000) >> 0x10 );
	buf[3] = (unsigned char)( (val & 0xFF000000) >> 0x18 );
	return *((long*)buf);
}

/// Reads an unsigned long (32 bits) in little-endian from the buffer
unsigned long GetULong(const unsigned char *buf)
{
	return	 ( ((unsigned long)(buf[0]))         )
			|( ((unsigned long)(buf[1])) << 0x08 )
			|( ((unsigned long)(buf[2])) << 0x10 )
			|( ((unsigned long)(buf[3])) << 0x18 );
}

// Reads a float (32 bits) from the buffer
float GetFloat(const unsigned char *buf)
{
	unsigned long val = GetULong(buf);
	return *((float*)&val);
}


// Read map from GRF's GAT and RSW files
int read_map(char *name, struct map_data *m)
{
	char filename[256];
	unsigned char *gat, *rsw;
	int water_height;
	size_t xy, off, num_cells;
	float height[4];
	unsigned long type;

	// Open map GAT
	sprintf(filename,"data\\%s.gat", name);
	gat = (unsigned char *)grfio_read(filename);
	if (gat == NULL)
		return 0;

	// Open map RSW
	sprintf(filename,"data\\%s.rsw", name);
	rsw = (unsigned char *)grfio_read(filename);

	// Read water height
	if (rsw) { 
		water_height = (int)GetFloat(rsw+166);
		free(rsw);
	} else
		water_height = NO_WATER;

	// Read map size and allocate needed memory
	m->xs = (short)GetULong(gat+6);
	m->ys = (short)GetULong(gat+10);
	num_cells = (size_t)m->xs*m->ys;
	m->cells = (unsigned char *)malloc(num_cells);

	// Set cell properties
	off = 14;
	for (xy = 0; xy < num_cells; xy++)
	{
		// Height of the corners
		height[0] = GetFloat( gat + off      );
		height[1] = GetFloat( gat + off + 4  );
		height[2] = GetFloat( gat + off + 8  );
		height[3] = GetFloat( gat + off + 12 );
		// Type of cell
		type      = GetULong( gat + off + 16 );
		off += 20;
		if (water_height != NO_WATER && type == 0 && (height[0] > water_height || height[1] > water_height || height[2] > water_height || height[3] > water_height))
			m->cells[xy] = 3; // Cell is 0 (walkable) but under water level, set to 3 (walkable water)
		else
			m->cells[xy] = (unsigned char)type;
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
	info.index = MakeUShortLE(index);
	info.xs = MakeShortLE(m->xs);
	info.ys = MakeShortLE(m->ys);
	info.len = MakeLongLE((long)len);

	// Append map header then compressed cells at the end of the file
	fseek(map_cache_fp, filesize, SEEK_SET);
	fwrite(&info, sizeof(struct map_cache_info), 1, map_cache_fp);
	fwrite(write_buf, 1, len, map_cache_fp);
	map_count++;
	filesize += sizeof(struct map_cache_info) + len;

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
	map_cache_fp = fopen(map_cache_file, "wb");
	if( map_cache_fp == NULL ) {
		printf("Failure when opening map cache file %s\n", map_cache_file);
		exit(1);
	}

	printf("Opening map list: %s\n", map_list_file);
	list = fopen(map_list_file, "r");
	if( list == NULL ) {
		printf("Failure when opening maps list file %s\n", map_list_file);
		exit(1);
	}

	// Initialize the main header
	map_count = 0;
	filesize = sizeof(map_count);

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
		}
	}

	printf("Closing map list: %s\n", map_list_file);
	fclose(list);

	printf("Closing map cache: %s\n", map_cache_file);
	// Write the main header and close the map cache
	fseek(map_cache_fp, 0, SEEK_SET);
	fwrite(&map_count, sizeof(map_count), 1, map_cache_fp);
	fclose(map_cache_fp);

	printf("Finalizing grfio\n");
	grfio_final();

	printf("%d maps cached\n", map_count);

	return 0;
}
