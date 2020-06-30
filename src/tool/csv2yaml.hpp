// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CSV2YAML_HPP
#define CSV2YAML_HPP

#include "yaml.hpp"

// Required constant and structure definitions
#define MAX_GUILD_SKILL_REQUIRE 5
#define MAX_SKILL_ITEM_REQUIRE	10
#define MAX_SKILL_STATUS_REQUIRE 3
#define MAX_SKILL_EQUIP_REQUIRE 10
#define MAX_QUEST_DROPS 3
#define MAX_MAP_PER_INSTANCE 255

// Database to memory maps
struct s_skill_unit_csv : s_skill_db {
	std::string target_str;
	int unit_flag_csv;
};

std::unordered_map<uint16, s_skill_require> skill_require;
std::unordered_map<uint16, s_skill_db> skill_cast;
std::unordered_map<uint16, s_skill_db> skill_castnodex;
std::unordered_map<uint16, s_skill_unit_csv> skill_unit;
std::unordered_map<uint16, s_skill_copyable> skill_copyable;
std::unordered_map<uint16, s_skill_db> skill_nearnpc;

static bool guild_read_guildskill_tree_db(char *split[], int columns, int current);
static bool pet_read_db(const char *file);
static bool skill_parse_row_magicmushroomdb(char *split[], int column, int current);
static bool skill_parse_row_abradb(char *split[], int columns, int current);
static bool skill_parse_row_improvisedb(char *split[], int columns, int current);
static bool skill_parse_row_spellbookdb(char *split[], int columns, int current);
static bool mob_readdb_mobavail(char *str[], int columns, int current);
static bool skill_parse_row_requiredb(char *split[], int columns, int current);
static bool skill_parse_row_castdb(char *split[], int columns, int current);
static bool skill_parse_row_castnodexdb(char *split[], int columns, int current);
static bool skill_parse_row_unitdb(char *split[], int columns, int current);
static bool skill_parse_row_copyabledb(char *split[], int columns, int current);
static bool skill_parse_row_nonearnpcrangedb(char *split[], int columns, int current);
static bool skill_parse_row_skilldb(char *split[], int columns, int current);
static bool quest_read_db(char *split[], int columns, int current);
static bool instance_readdb_sub(char *str[], int columns, int current);

#endif /* CSV2YAML_HPP */
