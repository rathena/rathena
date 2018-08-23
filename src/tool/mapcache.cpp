// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <vector>

#include "../common/core.hpp"
#include "../common/grfio.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/utils.hpp"

#define NO_WATER 1000000

std::string grf_list_file = "conf/grf-files.txt";
std::string map_list_file = "map_index.txt";
std::string map_cache_file;
int rebuild = 0;

FILE *map_cache_fp;

unsigned long file_size;

// Used internally, this structure contains the physical map cells
struct map_data {
	int16 xs;
	int16 ys;
	unsigned char *cells;
};

// This is the main header found at the very beginning of the file
struct main_header {
	uint32 file_size;
	uint16 map_count;
} header;

// This is the header appended before every compressed map cells info
struct map_info {
	char name[MAP_NAME_LENGTH];
	int16 xs;
	int16 ys;
	int32 len;
};


// Reads a map from GRF's GAT and RSW files
int read_map(char *name, struct map_data *m)
{
	char filename[256];
	unsigned char *gat, *rsw;
	int water_height;
	size_t xy, off, num_cells;

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
		aFree(rsw);
	} else
		water_height = NO_WATER;

	// Read map size and allocate needed memory
	m->xs = (int16)GetULong(gat+6);
	m->ys = (int16)GetULong(gat+10);
	if (m->xs <= 0 || m->ys <= 0) {
		aFree(gat);
		return 0;
	}
	num_cells = (size_t)m->xs*(size_t)m->ys;
	m->cells = (unsigned char *)aMalloc(num_cells);

	// Set cell properties
	off = 14;
	for (xy = 0; xy < num_cells; xy++)
	{
		// Height of the bottom-left corner
		float height = GetFloat( gat + off      );
		// Type of cell
		uint32 type   = GetULong( gat + off + 16 );
		off += 20;

		if (type == 0 && water_height != NO_WATER && height > water_height)
			type = 3; // Cell is 0 (walkable) but under water level, set to 3 (walkable water)

		m->cells[xy] = (unsigned char)type;
	}

	aFree(gat);

	return 1;
}

// Adds a map to the cache
void cache_map(char *name, struct map_data *m)
{
	struct map_info info;
	unsigned long len;
	unsigned char *write_buf;

	// Create an output buffer twice as big as the uncompressed map... this way we're sure it fits
	len = (unsigned long)m->xs*(unsigned long)m->ys*2;
	write_buf = (unsigned char *)aMalloc(len);
	// Compress the cells and get the compressed length
	encode_zip(write_buf, &len, m->cells, m->xs*m->ys);

	// Fill the map header
	if (strlen(name) > MAP_NAME_LENGTH) // It does not hurt to warn that there are maps with name longer than allowed.
		ShowWarning ("Map name '%s' size '%d' is too long. Truncating to '%d'.\n", name, strlen(name), MAP_NAME_LENGTH);
	strncpy(info.name, name, MAP_NAME_LENGTH);
	info.xs = MakeShortLE(m->xs);
	info.ys = MakeShortLE(m->ys);
	info.len = MakeLongLE(len);

	// Append map header then compressed cells at the end of the file
	fseek(map_cache_fp, header.file_size, SEEK_SET);
	fwrite(&info, sizeof(struct map_info), 1, map_cache_fp);
	fwrite(write_buf, 1, len, map_cache_fp);
	header.file_size += sizeof(struct map_info) + len;
	header.map_count++;

	aFree(write_buf);
	aFree(m->cells);

	return;
}

// Checks whether a map is already is the cache
int find_map(char *name)
{
	int i;
	struct map_info info;

	fseek(map_cache_fp, sizeof(struct main_header), SEEK_SET);

	for(i = 0; i < header.map_count; i++) {
		if(fread(&info, sizeof(info), 1, map_cache_fp) != 1) printf("An error as occured in fread while reading map_cache\n");
		if(strcmp(name, info.name) == 0) // Map found
			return 1;
		else // Map not found, jump to the beginning of the next map info header
			fseek(map_cache_fp, GetLong((unsigned char *)&(info.len)), SEEK_CUR);
	}

	return 0;
}

// Cuts the extension from a map name
char *remove_extension(char *mapname)
{
	char *ptr, *ptr2;
	ptr = strchr(mapname, '.');
	if (ptr) { //Check and remove extension.
		while (ptr[1] && (ptr2 = strchr(ptr+1, '.')))
			ptr = ptr2; //Skip to the last dot.
		if (strcmp(ptr,".gat") == 0)
			*ptr = '\0'; //Remove extension.
	}
	return mapname;
}

// Processes command-line arguments
void process_args(int argc, char *argv[])
{
	for(int i = 0; i < argc; i++) {
		if(strcmp(argv[i], "-grf") == 0) {
			if(++i < argc)
				grf_list_file = argv[i];
		} else if(strcmp(argv[i], "-list") == 0) {
			if(++i < argc)
				map_list_file = argv[i];
		} else if(strcmp(argv[i], "-cache") == 0) {
			if(++i < argc)
				map_cache_file = argv[i];
		} else if(strcmp(argv[i], "-rebuild") == 0)
			rebuild = 1;
	}

}

int do_init(int argc, char** argv)
{
	/* setup pre-defined, #define-dependant */
	map_cache_file = std::string(db_path) + "/" + std::string(DBPATH) + "map_cache.dat";

	// Process the command-line arguments
	process_args(argc, argv);

	ShowStatus("Initializing grfio with %s\n", grf_list_file.c_str());
	grfio_init(grf_list_file.c_str());

	// Attempt to open the map cache file and force rebuild if not found
	ShowStatus("Opening map cache: %s\n", map_cache_file.c_str());
	if(!rebuild) {
		map_cache_fp = fopen(map_cache_file.c_str(), "rb");
		if(map_cache_fp == NULL) {
			ShowNotice("Existing map cache not found, forcing rebuild mode\n");
			rebuild = 1;
		} else
			fclose(map_cache_fp);
	}
	if(rebuild)
		map_cache_fp = fopen(map_cache_file.c_str(), "w+b");
	else
		map_cache_fp = fopen(map_cache_file.c_str(), "r+b");
	if(map_cache_fp == NULL) {
		ShowError("Failure when opening map cache file %s\n", map_cache_file.c_str());
		exit(EXIT_FAILURE);
	}

	// Open the map list
	FILE *list;
	std::vector<std::string> directories = { std::string(db_path) + "/",  std::string(db_path) + "/" + std::string(DBIMPORT) + "/" };

	for (const auto &directory : directories) {
		std::string filename = directory + map_list_file;

		ShowStatus("Opening map list: %s\n", filename.c_str());
		list = fopen(filename.c_str(), "r");
		if (list == NULL) {
			ShowError("Failure when opening maps list file %s\n", filename.c_str());
			exit(EXIT_FAILURE);
		}

		// Initialize the main header
		if (rebuild) {
			header.file_size = sizeof(struct main_header);
			header.map_count = 0;
		} else {
			if (fread(&header, sizeof(struct main_header), 1, map_cache_fp) != 1) { printf("An error as occured while reading map_cache_fp \n"); }
			header.file_size = GetULong((unsigned char *)&(header.file_size));
			header.map_count = GetUShort((unsigned char *)&(header.map_count));
		}

		// Read and process the map list
		char line[1024];

		while (fgets(line, sizeof(line), list))
		{
			if (line[0] == '/' && line[1] == '/')
				continue;

			char name[MAP_NAME_LENGTH_EXT];

			if (sscanf(line, "%15s", name) < 1)
				continue;

			if (strcmp("map:", name) == 0 && sscanf(line, "%*s %15s", name) < 1)
				continue;

			struct map_data map;

			name[MAP_NAME_LENGTH_EXT - 1] = '\0';
			remove_extension(name);
			if (find_map(name))
				ShowInfo("Map '" CL_WHITE "%s" CL_RESET "' already in cache.\n", name);
			else if (read_map(name, &map)) {
				cache_map(name, &map);
				ShowInfo("Map '" CL_WHITE "%s" CL_RESET "' successfully cached.\n", name);
			}
			else
				ShowError("Map '" CL_WHITE "%s" CL_RESET "' not found!\n", name);

		}

		ShowStatus("Closing map list: %s\n", filename.c_str());
		fclose(list);
	}

	// Write the main header and close the map cache
	ShowStatus("Closing map cache: %s\n", map_cache_file.c_str());
	fseek(map_cache_fp, 0, SEEK_SET);
	fwrite(&header, sizeof(struct main_header), 1, map_cache_fp);
	fclose(map_cache_fp);

	ShowStatus("Finalizing grfio\n");
	grfio_final();

	ShowInfo("%d maps now in cache\n", header.map_count);

	return 0;
}

void do_final(void)
{
}
