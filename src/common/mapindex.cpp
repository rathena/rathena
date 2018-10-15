// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mapindex.hpp"

#include <stdlib.h>

#include "core.hpp"
#include "mmo.hpp"
#include "showmsg.hpp"
#include "strlib.hpp"

DBMap *mapindex_db;
struct _indexes {
	char name[MAP_NAME_LENGTH]; //Stores map name
} indexes[MAX_MAPINDEX];

int max_index = 0;

#define mapindex_exists(id) (indexes[id].name[0] != '\0')

/// Retrieves the map name from 'string' (removing .gat extension if present).
/// Result gets placed either into 'buf' or in a static local buffer.
const char* mapindex_getmapname(const char* string, char* output) {
	static char buf[MAP_NAME_LENGTH];
	char* dest = (output != NULL) ? output : buf;

	size_t len = strnlen(string, MAP_NAME_LENGTH_EXT);
	if (len == MAP_NAME_LENGTH_EXT) {
		ShowWarning("(mapindex_normalize_name) Map name '%*s' is too long!\n", 2*MAP_NAME_LENGTH_EXT, string);
		len--;
	}
	if (len >= 4 && stricmp(&string[len-4], ".gat") == 0)
		len -= 4; // strip .gat extension

	len = zmin(len, MAP_NAME_LENGTH-1);
	safestrncpy(dest, string, len+1);
	memset(&dest[len], '\0', MAP_NAME_LENGTH-len);

	return dest;
}

/// Retrieves the map name from 'string' (adding .gat extension if not already present).
/// Result gets placed either into 'buf' or in a static local buffer.
const char* mapindex_getmapname_ext(const char* string, char* output) {
	static char buf[MAP_NAME_LENGTH_EXT];
	char* dest = (output != NULL) ? output : buf;

	size_t len;

	strcpy(buf,string);
	sscanf(string,"%*[^#]%*[#]%15s",buf);

	len = safestrnlen(buf, MAP_NAME_LENGTH);

	if (len == MAP_NAME_LENGTH) {
		ShowWarning("(mapindex_normalize_name) Map name '%*s' is too long!\n", 2*MAP_NAME_LENGTH, buf);
		len--;
	}
	safestrncpy(dest, buf, len+1);

	if (len < 4 || stricmp(&dest[len-4], ".gat") != 0) {
		strcpy(&dest[len], ".gat");
		len += 4; // add .gat extension
	}

	memset(&dest[len], '\0', MAP_NAME_LENGTH_EXT-len);

	return dest;
}

/// Adds a map to the specified index
/// Returns 1 if successful, 0 oherwise
int mapindex_addmap(int index, const char* name) {
	char map_name[MAP_NAME_LENGTH];
	if (index == -1){ //autogive index
		ARR_FIND(1,max_index,index,(indexes[index].name[0] == '\0'));
	}

	if (index < 0 || index >= MAX_MAPINDEX) {
		ShowError("(mapindex_add) Map index (%d) for \"%s\" out of range (max is %d)\n", index, name, MAX_MAPINDEX);
		return 0;
	}

	mapindex_getmapname(name, map_name);

	if (map_name[0] == '\0') {
		ShowError("(mapindex_add) Cannot add maps with no name.\n");
		return 0;
	}

	if (strlen(map_name) >= MAP_NAME_LENGTH) {
		ShowError("(mapindex_add) Map name %s is too long. Maps are limited to %d characters.\n", map_name, MAP_NAME_LENGTH);
		return 0;
	}

	if (mapindex_exists(index)) {
		ShowWarning("(mapindex_add) Overriding index %d: map \"%s\" -> \"%s\"\n", index, indexes[index].name, map_name);
		strdb_remove(mapindex_db, indexes[index].name);
	}

	safestrncpy(indexes[index].name, map_name, MAP_NAME_LENGTH);
	strdb_iput(mapindex_db, map_name, index);
	if (max_index <= index)
		max_index = index+1;

	return index;
}

unsigned short mapindex_name2idx(const char* name, const char *func) {
	int i;
	char map_name[MAP_NAME_LENGTH];
	mapindex_getmapname(name, map_name);

	if( (i = strdb_iget(mapindex_db, map_name)) )
		return i;

	ShowDebug("(%s) mapindex_name2id: Map \"%s\" not found in index list!\n", func, map_name);
	return 0;
}

const char* mapindex_idx2name(unsigned short id, const char *func) {
	if (id >= MAX_MAPINDEX || !mapindex_exists(id)) {
		ShowDebug("(%s) mapindex_id2name: Requested name for non-existant map index [%d] in cache.\n", func, id);
		return indexes[0].name; // dummy empty string so that the callee doesn't crash
	}
	return indexes[id].name;
}

void mapindex_init(void) {
	FILE *fp;
	char line[1024];
	int last_index = -1;
	int index;
	char map_name[MAP_NAME_LENGTH];
	char path[255];
	const char* mapindex_cfgfile[] = {
		"map_index.txt",
		DBIMPORT"/map_index.txt"
	};

	memset (&indexes, 0, sizeof (indexes));
	mapindex_db = strdb_alloc(DB_OPT_DUP_KEY, MAP_NAME_LENGTH);

	for( size_t i = 0; i < ARRAYLENGTH(mapindex_cfgfile); i++ ){
		sprintf( path, "%s/%s", db_path, mapindex_cfgfile[i] );

		if( ( fp = fopen( path, "r" ) ) == NULL ){
			// It is only fatal if it is the main file
			if( i == 0 ){
				ShowFatalError("Unable to read mapindex config file %s!\n", path );
				exit(EXIT_FAILURE); //Server can't really run without this file.
			}else{
				ShowWarning("Unable to read mapindex config file %s!\n", path );
				break;
			}
		}

		while(fgets(line, sizeof(line), fp)) {
			if(line[0] == '/' && line[1] == '/')
				continue;

			switch (sscanf(line, "%11s\t%d", map_name, &index)) {
				case 1: //Map with no ID given, auto-assign
					index = last_index+1;
				case 2: //Map with ID given
					mapindex_addmap(index,map_name);
					break;
				default:
					continue;
			}
			last_index = index;
		}
		fclose(fp);
	}
}

/**
 * Check default map (only triggered once by char-server)
 * @param mapname
 **/
void mapindex_check_mapdefault(const char *mapname) {
	mapname = mapindex_getmapname(mapname, NULL);
	if( !strdb_iget(mapindex_db, mapname) ) {
		ShowError("mapindex_init: Default map '%s' not found in cache! Please change in (by default in) char_athena.conf!\n", mapname);
	}
}

int mapindex_removemap(int index){
	indexes[index].name[0] = '\0';
	return 0;
}

void mapindex_final(void) {
	db_destroy(mapindex_db);
}
