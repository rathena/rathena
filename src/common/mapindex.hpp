// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAPINDEX_HPP
#define MAPINDEX_HPP

#include "../common/database.hpp"
#include "../common/mmo.hpp"

//#define MAX_MAPINDEX 2000

//Some definitions for the major city maps.
#define MAP_PRONTERA "prontera"
#define MAP_GEFFEN "geffen"
#define MAP_MORROC "morocc"
#define MAP_ALBERTA "alberta"
#define MAP_PAYON "payon"
#define MAP_IZLUDE "izlude"
#define MAP_ALDEBARAN "aldebaran"
#define MAP_LUTIE "xmas"
#define MAP_COMODO "comodo"
#define MAP_YUNO "yuno"
#define MAP_AMATSU "amatsu"
#define MAP_GONRYUN "gonryun"
#define MAP_UMBALA "umbala"
#define MAP_NIFLHEIM "niflheim"
#define MAP_LOUYANG "louyang"
#define MAP_JAWAII "jawaii"
#define MAP_AYOTHAYA "ayothaya"
#define MAP_EINBROCH "einbroch"
#define MAP_LIGHTHALZEN "lighthalzen"
#define MAP_EINBECH "einbech"
#define MAP_HUGEL "hugel"
#define MAP_RACHEL "rachel"
#define MAP_VEINS "veins"
#define MAP_JAIL "sec_pri"
#ifdef RENEWAL
	#define MAP_NOVICE "iz_int"
#else
	#define MAP_NOVICE "new_1-1"
#endif
#define MAP_MOSCOVIA "moscovia"
#define MAP_MIDCAMP "mid_camp"
#define MAP_MANUK "manuk"
#define MAP_SPLENDIDE "splendide"
#define MAP_BRASILIS "brasilis"
#define MAP_DICASTES "dicastes01"
#define MAP_MORA "mora"
#define MAP_DEWATA "dewata"
#define MAP_MALANGDO "malangdo"
#define MAP_MALAYA "malaya"
#define MAP_ECLAGE "eclage"
#define MAP_ECLAGE_IN "ecl_in01"
#define MAP_LASAGNA "lasagna"

struct s_map_index {
	std::string name;
};

class MapIndexDatabase : public TypesafeCachedYamlDatabase<uint16, s_map_index> {
private:
	uint16 index;
	std::vector<std::string> mapNameToIndex;

public:
	MapIndexDatabase() : TypesafeCachedYamlDatabase("MAP_INDEX", 1) {
		index = 1;
		mapNameToIndex.push_back("Unknown"); // Index 0
	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef &node) override;

	// Additional
	const uint16 getIndex();
	uint16 increaseIndex();
	const uint16 name2index(const std::string &name);
};

extern MapIndexDatabase map_index_db;

const std::string mapindex_getmapname(const std::string &string);
const std::string mapindex_getmapname_ext(const std::string &string);

uint16 mapindex_name2idx(const std::string &name, const std::string &func);
#define mapindex_name2id(mapname) mapindex_name2idx((mapname), __FUNCTION__)

const std::string mapindex_idx2name(const uint16 id, const std::string &func);
#define mapindex_id2name(mapindex) mapindex_idx2name((mapindex), __FUNCTION__).c_str()

uint16 mapindex_addmap(uint16 index, const std::string &name);
void mapindex_removemap(uint16 index);

void mapindex_check_mapdefault(const std::string &mapname);

void mapindex_init(void);
void mapindex_final(void);

#endif /* MAPINDEX_HPP */
