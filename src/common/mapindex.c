// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"

#define MAX_MAPINDEX 2000

struct indexes {
	char name[MAP_NAME_LENGTH]; //Stores map name
	char exists; //Set to 1 if index exists
} indexes[MAX_MAPINDEX];

static unsigned short max_index = 0;

char mapindex_cfgfile[80] = "db/map_index.txt";

// Removes the extension from a map name
char *mapindex_normalize_name(char *mapname)
{
	char *ptr, *ptr2;
	ptr = strchr(mapname, '.');
	if (ptr) { //Check and remove extension.
		while (ptr[1] && (ptr2 = strchr(ptr+1, '.')))
			ptr = ptr2; //Skip to the last dot.
		if(stricmp(ptr,".gat") == 0)
			*ptr = '\0'; //Remove extension.
	}
	return mapname;
}

/// Adds a map to the specified index
/// Returns 1 if successful, 0 oherwise
int mapindex_addmap(int index, const char *name)
{
	char map_name[MAP_NAME_LENGTH_EXT];

	if (index < 0 || index >= MAX_MAPINDEX) {
		ShowError("(mapindex_add) Map index (%d) for \"%s\" out of range (max is %d)\n", index, name, MAX_MAPINDEX);
		return 0;
	}

	snprintf(map_name, MAP_NAME_LENGTH_EXT, "%s", name);
	mapindex_normalize_name(map_name);

	if (strlen(map_name) > MAP_NAME_LENGTH-1) {
		ShowError("(mapindex_add) Map name %s is too long. Maps are limited to %d characters.\n", map_name, MAP_NAME_LENGTH);
		return 0;
	}

	if (indexes[index].exists)
		ShowWarning("(mapindex_add) Overriding index %d: map \"%s\" -> \"%s\"\n", index, indexes[index].name, map_name);

	snprintf(indexes[index].name, MAP_NAME_LENGTH, "%s", map_name);
	indexes[index].exists = 1;
	if (max_index <= index)
		max_index = index+1;
	return 1;
}

unsigned short mapindex_name2id(const char* name) {
	//TODO: Perhaps use a db to speed this up? [Skotlex]
	int i;
	char map_name[MAP_NAME_LENGTH_EXT];

	snprintf(map_name, MAP_NAME_LENGTH_EXT, "%s", name);
	mapindex_normalize_name(map_name);

	for (i = 1; i < max_index; i++)
	{
		if (strcmp(indexes[i].name,map_name)==0)
			return i;
	}
#ifdef MAPINDEX_AUTOADD
	if( mapindex_addmap(i,map_name) )
	{
		ShowDebug("mapindex_name2id: Auto-added map \"%s\" to position %d\n", indexes[i], i);
		return i;
	}
	ShowWarning("mapindex_name2id: Failed to auto-add map \"%s\" to position %d!\n", name, i);
	return 0;
#else
	ShowDebug("mapindex_name2id: Map \"%s\" not found in index list!\n", name);
	return 0;
#endif
}

const char* mapindex_id2name(unsigned short id) {
	if (id > MAX_MAPINDEX || !indexes[id].exists) {
		ShowDebug("mapindex_id2name: Requested name for non-existant map index [%d] in cache.\n", id);
		return indexes[0].name; //Theorically this should never happen, hence we return this string to prevent null pointer crashes.
	}
	return indexes[id].name;
}

void mapindex_init(void) {
	FILE *fp;
	char line[1024];
	int last_index = -1;
	int index;
	char map_name[1024]; // only MAP_NAME_LENGTH(_EXT) under safe conditions
	
	memset (&indexes, 0, sizeof (indexes));
	fp=fopen(mapindex_cfgfile,"r");
	if(fp==NULL){
		ShowFatalError("Unable to read mapindex config file %s!\n", mapindex_cfgfile);
		exit(1); //Server can't really run without this file.
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;

		switch (sscanf(line,"%s\t%d",map_name,&index)) {
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

void mapindex_final(void) {
}
