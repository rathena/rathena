// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mapindex.hpp"

#include <stdlib.h>

#include "core.hpp"
#include "mmo.hpp"
#include "showmsg.hpp"
#include "strlib.hpp"

using namespace rathena;

MapIndexDatabase map_index_db;

const uint16 MapIndexDatabase::getIndex() {
	return this->index;
}

uint16 MapIndexDatabase::increaseIndex() {
	return this->index++;
}

const uint16 MapIndexDatabase::name2index(const std::string &name) {
	auto map = util::vector_get(this->mapNameToIndex, name);

	if (map != this->mapNameToIndex.end())
		return map - this->mapNameToIndex.begin();
	else
		return 0;
}

/**
 * Retrieves the map name from 'string' (removing.gat extension if present).
 * @param string: Map name
 * @return Adjusted map name.
 */
const std::string mapindex_getmapname(const std::string &string) {
	std::string temp = string;

	if (temp.size() > MAP_NAME_LENGTH)
		temp.resize(MAP_NAME_LENGTH - 1);

	size_t len = temp.size();

	if (len >= MAP_NAME_LENGTH_EXT) {
		ShowWarning("mapindex_normalize_name: Map name '%*s' is too long!\n", 2 * MAP_NAME_LENGTH_EXT, string.c_str());
		len--;
		temp.resize(len);
	}

	if (len >= 4 && string.find(".gat") != std::string::npos)
		temp.erase(len - 4); // strip .gat extension

	return temp;
}

/**
 * Retrieves the map name from 'string' (adding.gat extension if not already present).
 * @param string: Map name
 * @return Adjusted map name.
 */
const std::string mapindex_getmapname_ext(const std::string &string) {
	std::string temp = string;

	if (temp.size() > MAP_NAME_LENGTH_EXT)
		temp.resize(MAP_NAME_LENGTH_EXT - 1);

	size_t len = temp.size();

	if (len >= MAP_NAME_LENGTH) {
		ShowWarning("mapindex_normalize_name: Map name '%*s' is too long!\n", 2 * MAP_NAME_LENGTH, temp.c_str());
		len--;
		temp.resize(len);
	}

	if (len < 4 || temp.find(".gat") == std::string::npos)
		temp += ".gat"; // add .gat extension

	return temp;
	//return &(*temp.begin());
	//return const_cast<char *>(temp.c_str());
}

/**
 * Adds a map to the specified index.
 * @param index: Map index
 * @param name: Map name
 * @ return Map index if successful, 0 otherwise
 */
uint16 mapindex_addmap(uint16 index, const std::string &name) {
	if (index == 0) // Autogive index
		index = static_cast<uint16>(map_index_db.size() + 1);

	if (index < 1) {
		ShowError("mapindex_add: Map index (%hu) for \"%s\" must be greater than 1. Skipping...\n", index, name.c_str());
		return 0;
	}

	std::string map_name = mapindex_getmapname(name);

	if (map_name.empty()) {
		ShowError("mapindex_add: Cannot add maps with no name.\n");
		return 0;
	}

	if (map_name.size() >= MAP_NAME_LENGTH) {
		ShowError("mapindex_add: Map name %s is too long. Maps are limited to %d characters.\n", map_name.c_str(), MAP_NAME_LENGTH);
		return 0;
	}

	std::shared_ptr<s_map_index> map = map_index_db.find(index);

	if (map != nullptr)
		ShowWarning("mapindex_add: Overriding index %hu: map \"%s\" -> \"%s\"\n", index, map->name.c_str(), map_name.c_str());
	else
		map = std::make_shared<s_map_index>();

	map->name = map_name;

	map_index_db.put(index, map);

	return index;
}

/**
 * Remove a map from the indexed list
 * @param index: Map index to remove
 */
void mapindex_removemap(uint16 index) {
	map_index_db.erase(index);
}

/**
 * Gets the map index value from a map's name.
 * @param name: Map name to lookup
 * @param func: Optional function name used for reporting debug message
 * @return Map Index
 */
uint16 mapindex_name2idx(const std::string &name, const std::string &func) {
	std::string map_name = mapindex_getmapname(name);
	uint16 index = map_index_db.name2index(map_name);

	if (index == 0 && !func.empty())
		ShowDebug("(%s) mapindex_name2id: Map \"%s\" not found in index list!\n", func.c_str(), map_name.c_str());
	return index;
}

/**
 * Gets the map name value from a map's index.
 * @param id: Map index to lookup
 * @param func: Optional function name used for reporting debug message
 */
const std::string mapindex_idx2name(const uint16 id, const std::string &func) {
	if (id >= map_index_db.size() || !map_index_db.exists(id)) {
		ShowDebug("(%s) mapindex_id2name: Requested name for non-existant map index [%hu] in cache.\n", func.c_str(), id);
		return ""; // dummy empty string so that the callee doesn't crash
	}

	return map_index_db.find(id)->name;
}

const std::string MapIndexDatabase::getDefaultLocation() {
	return std::string(db_path) + "/map_index.yml";
}

/**
 * Reads and parses an entry from the map_index.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MapIndexDatabase::parseBodyNode(const ryml::NodeRef &node) {
	std::string map_name;

	if (!this->asString(node, "Map", map_name))
		return 0;

	uint16 mapindex;

	if (this->nodeExists(node, "Index")) {
		if (!this->asUInt16(node, "Index", mapindex))
			return 0;

		if (mapindex < 1) {
			this->invalidWarning(node["Index"], "Index %hu needs to be greater than 0. Skipping...\n", mapindex);
			return 0;
		}
	} else {
		mapindex = this->index;
	}

	this->increaseIndex();
	mapindex_addmap(mapindex, map_name);
	this->mapNameToIndex.push_back(map_name);

	return 1;
}

/**
 * Check default map (only triggered once by char-server)
 * @param mapname: Default map name
 */
void mapindex_check_mapdefault(const std::string &mapname) {
	std::string temp = mapindex_getmapname(mapname);

	if (!mapindex_name2idx(temp, ""))
		ShowError("mapindex_init: Default map '%s' not found in cache! Please change in (by default in) char_athena.conf!\n", mapname);
}

void mapindex_init(void) {
	map_index_db.load();
}

void mapindex_final(void) {
	map_index_db.clear();
}
