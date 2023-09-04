// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef YAML_HPP
#define YAML_HPP

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#ifdef WIN32
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
	#include <stdio.h>
#endif

#include <yaml-cpp/yaml.h>
#include <ryml_std.hpp>
#include <ryml.hpp>

#include <common/cbasetypes.hpp>
#include <common/core.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>
#ifdef WIN32
#include <common/winapi.hpp>
#endif

// Only for constants - do not use functions of it or linking will fail
#define ONLY_CONSTANTS
#include <map/achievement.hpp>
#include <map/battle.hpp>
#include <map/battleground.hpp>
#include <map/cashshop.hpp>
#include <map/channel.hpp>
#include <map/chat.hpp>
#include <map/date.hpp>
#include <map/elemental.hpp>
#include <map/homunculus.hpp>
#include <map/instance.hpp>
#include <map/mercenary.hpp>
#include <map/mob.hpp>
#include <map/npc.hpp>
#include <map/pc.hpp>
#include <map/pet.hpp>
#include <map/quest.hpp>
#include <map/script.hpp>
#include <map/skill.hpp>
#include <map/storage.hpp>

using namespace rathena;

/// Uncomment this line to enable the ability for the conversion tools to automatically convert
/// all files with no user interaction, whether it be from CSV to YAML or YAML to SQL.
//#define CONVERT_ALL

#ifndef WIN32
int getch(void) {
	struct termios oldattr, newattr;
	int ch;
	tcgetattr(STDIN_FILENO, &oldattr);
	newattr = oldattr;
	newattr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
	return ch;
}
#endif

YAML::Node inNode;
YAML::Emitter body;

// Constants for conversion
std::unordered_map<t_itemid, std::string> aegis_itemnames;
std::unordered_map<uint32, t_itemid> aegis_itemviewid;
std::unordered_map<uint16, std::string> aegis_mobnames;
std::unordered_map<uint16, std::string> aegis_skillnames;
std::unordered_map<const char *, int64> constants;

// Implement the function instead of including the original version by linking
void script_set_constant_(const char *name, int64 value, const char *constant_name, bool isparameter, bool deprecated) {
	if (!deprecated)
		constants[name] = value;
}

const char *constant_lookup(int32 value, const char *prefix) {
	nullpo_retr(nullptr, prefix);

	for (auto const &pair : constants) {
		// Same prefix group and same value
		if (strncasecmp(pair.first, prefix, strlen(prefix)) == 0 && pair.second == value) {
			return pair.first;
		}
	}

	return nullptr;
}

int64 constant_lookup_int(const char *constant) {
	nullpo_retr(-100, constant);

	for (auto const &pair : constants) {
		if (strlen(pair.first) == strlen(constant) && strncasecmp(pair.first, constant, strlen(constant)) == 0) {
			return pair.second;
		}
	}

	return -100;
}

/**
 * Determines if a file exists.
 * @param path: File to check for
 * @return True if file exists or false otherwise
 */
bool fileExists(const std::string &path) {
	std::ifstream in;

	in.open(path);

	if (in.is_open()) {
		in.close();

		return true;
	} else {
		return false;
	}
}

/**
 * Prompt for confirmation.
 * @param fmt: Message to print
 * @param va_arg: Any arguments needed for message
 * @return True on yes or false otherwise
 */
bool askConfirmation(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	_vShowMessage(MSG_NONE, fmt, ap);

	va_end(ap);

	char c = getch();

	if (c == 'Y' || c == 'y') {
		return true;
	} else {
		return false;
	}
}

/**
 * Returns a YAML header version
 * @param node: YAML node
 * @return Version number
 */
uint32 getHeaderVersion(YAML::Node &node) {
	return node["Header"]["Version"].as<uint32>();
}

/**
 * Copy a file if it exists.
 * @param file: File stream
 * @param name: File name
 * @param newLine: Append new line at end of copy
 */
void copyFileIfExists(std::ofstream &file, const std::string &name, bool newLine) {
	std::string path = "doc/yaml/db/" + name + ".yml";

	if (fileExists(path)) {
		std::ifstream source(path, std::ios::binary);

		std::istreambuf_iterator<char> begin_source(source);
		std::istreambuf_iterator<char> end_source;
		std::ostreambuf_iterator<char> begin_dest(file);
		copy(begin_source, end_source, begin_dest);

		source.close();

		if (newLine) {
			file << "\n";
		}
	}
}

/**
 * Prepares header for output.
 * @param file: File stream
 * @param type: Database type
 * @param version: Database version
 * @param name: File name
 */
void prepareHeader(std::ofstream &file, const std::string &type, uint32 version, const std::string &name) {
	copyFileIfExists(file, "license", false);
	copyFileIfExists(file, name, true);

	YAML::Emitter header(file);

	header << YAML::BeginMap;
	header << YAML::Key << "Header";
	header << YAML::BeginMap;
	header << YAML::Key << "Type" << YAML::Value << type;
	header << YAML::Key << "Version" << YAML::Value << version;
	header << YAML::EndMap;
	header << YAML::EndMap;

	file << "\n";
	file << "\n";
}

/**
 * Prepares footer for output.
 * @param file: File stream
 */
void prepareFooter(std::ostream &file) {
	if (!inNode["Footer"].IsDefined())
		return;

	if (inNode["Body"].IsDefined()) {
		file << "\n";
		file << "\n";
	}

	YAML::Emitter footer(file);

	footer << YAML::BeginMap;
	footer << YAML::Key << "Footer";
	footer << YAML::BeginMap;
	footer << YAML::Key << "Imports";
	footer << YAML::BeginSeq;
	for (const YAML::Node &import : inNode["Footer"]["Imports"]) {
		footer << YAML::BeginMap;
		footer << YAML::Key << "Path" << YAML::Value << import["Path"];
		if (import["Mode"].IsDefined())
			footer << YAML::Key << "Mode" << YAML::Value << import["Mode"];
		footer << YAML::EndMap;
	}
	footer << YAML::EndSeq;
	footer << YAML::EndMap;
	footer << YAML::EndMap;
}

/**
 * Prepares body for output.
 */
void prepareBody(void) {
	body << YAML::BeginMap;
	body << YAML::Key << "Body";
	body << YAML::BeginSeq;
}

/**
 * Finalizes body's output.
 */
void finalizeBody(void) {
	body << YAML::EndSeq;
	body << YAML::EndMap;
}

/**
 * Split the string with ':' as separator and put each value for a skilllv
 * @param str: String to split
 * @param val: Array to store value into
 * @param max: Max array size (Default: MAX_SKILL_LEVEL)
 * @return 0:error, x:number of value assign (max value)
 */
int skill_split_atoi(char *str, int *val, int max = MAX_SKILL_LEVEL) {
	int i;

	for (i = 0; i < max; i++) {
		if (!str)
			break;
		val[i] = atoi(str);
		str = strchr(str, ':');
		if (str)
			*str++ = 0;
	}

	if (i == 0) // No data found.
		return 0;

	if (i == 1) // Single value, have the whole range have the same value.
		return 1;

	return i;
}

/**
 * Split string to int by constant value (const.yml) or atoi()
 * @param *str: String input
 * @param *val: Temporary storage
 * @param *delim: Delimiter (for multiple value support)
 * @param min_value: Minimum value. If the splitted value is less or equal than this, will be skipped
 * @param max: Maximum number that can be allocated
 * @return count: Number of success
 */
uint8 skill_split_atoi2(char *str, int64 *val, const char *delim, int min_value, uint16 max) {
	uint8 i = 0;
	char *p = strtok(str, delim);

	while (p != NULL) {
		int64 n = min_value;

		trim(p);

		if (ISDIGIT(p[0])) // If using numeric
			n = atoi(p);
		else {
			n = constant_lookup_int(p);
			p = strtok(NULL, delim);
		}

		if (n > min_value) {
			val[i] = n;
			i++;
			if (i >= max)
				break;
		}
		p = strtok(NULL, delim);
	}
	return i;
}

/**
 * Split string to int
 * @param str: String input
 * @param val1: Temporary storage to first value
 * @param val2: Temporary storage to second value
 */
static void itemdb_re_split_atoi(char* str, int* val1, int* val2) {
	int i, val[2];

	for (i = 0; i < 2; i++) {
		if (!str)
			break;
		val[i] = atoi(str);
		str = strchr(str, ':');
		if (str)
			*str++ = 0;
	}
	if (i == 0) {
		*val1 = *val2 = 0;
		return; // no data found
	}
	if (i == 1) { // Single Value
		*val1 = val[0];
		*val2 = 0;
		return;
	}

	// We assume we have 2 values.
	*val1 = val[0];
	*val2 = val[1];
	return;
}

/**
 * Determine if array contains level specific data
 * @param arr: Array to check
 * @return True if level specific or false for same for all levels
 */
static bool isMultiLevel(int arr[]) {
	uint8 count = 0;

	for (uint8 i = 0; i < MAX_SKILL_LEVEL; i++) {
		if (arr[i] != 0)
			count++;
	}

	return (count < 2 ? false : true);
}

/**
 * Converts a string to upper case characters based on specific cases.
 * @param name: String to convert
 * @return Converted string
 */
std::string name2Upper(std::string name) {
	util::tolower( name );
	name[0] = toupper(name[0]);

	for (size_t i = 0; i < name.size(); i++) {
		if (name[i - 1] == '_' || (name[i - 2] == '1' && name[i - 1] == 'h') || (name[i - 2] == '2' && name[i - 1] == 'h'))
			name[i] = toupper(name[i]);
	}

	return name;
}

// Constant loading functions
static bool parse_item_constants_txt(const char *path) {
	uint32 lines = 0, count = 0;
	char line[1024];

	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		ShowWarning("itemdb_readdb: File not found \"%s\", skipping.\n", path);
		return false;
	}

	// process rows one by one
	while (fgets(line, sizeof(line), fp))
	{
		char *str[32], *p;
		int i;
		lines++;
		if (line[0] == '/' && line[1] == '/')
			continue;
		memset(str, 0, sizeof(str));

		p = strstr(line, "//");

		if (p != nullptr) {
			*p = '\0';
		}

		p = line;
		while (ISSPACE(*p))
			++p;
		if (*p == '\0')
			continue;// empty line
		for (i = 0; i < 19; ++i)
		{
			str[i] = p;
			p = strchr(p, ',');
			if (p == NULL)
				break;// comma not found
			*p = '\0';
			++p;
		}

		t_itemid item_id = strtoul(str[0], nullptr, 10);

		if (p == NULL)
		{
			ShowError("itemdb_readdb: Insufficient columns in line %d of \"%s\" (item with id %u), skipping.\n", lines, path, item_id);
			continue;
		}

		// Script
		if (*p != '{')
		{
			ShowError("itemdb_readdb: Invalid format (Script column) in line %d of \"%s\" (item with id %u), skipping.\n", lines, path, item_id);
			continue;
		}
		str[19] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL)
		{
			ShowError("itemdb_readdb: Invalid format (Script column) in line %d of \"%s\" (item with id %u), skipping.\n", lines, path, item_id);
			continue;
		}
		*p = '\0';
		p += 2;

		// OnEquip_Script
		if (*p != '{')
		{
			ShowError("itemdb_readdb: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %u), skipping.\n", lines, path, item_id);
			continue;
		}
		str[20] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL)
		{
			ShowError("itemdb_readdb: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %u), skipping.\n", lines, path, item_id);
			continue;
		}
		*p = '\0';
		p += 2;

		// OnUnequip_Script (last column)
		if (*p != '{')
		{
			ShowError("itemdb_readdb: Invalid format (OnUnequip_Script column) in line %d of \"%s\" (item with id %u), skipping.\n", lines, path, item_id);
			continue;
		}
		str[21] = p;
		p = &str[21][strlen(str[21]) - 2];

		if (*p != '}') {
			/* lets count to ensure it's not something silly e.g. a extra space at line ending */
			int lcurly = 0, rcurly = 0;

			for (size_t v = 0; v < strlen(str[21]); v++) {
				if (str[21][v] == '{')
					lcurly++;
				else if (str[21][v] == '}') {
					rcurly++;
					p = &str[21][v];
				}
			}

			if (lcurly != rcurly) {
				ShowError("itemdb_readdb: Mismatching curly braces in line %d of \"%s\" (item with id %u), skipping.\n", lines, path, item_id);
				continue;
			}
		}
		str[21] = str[21] + 1;  //skip the first left curly
		*p = '\0';              //null the last right curly

		uint32 view_id = strtoul(str[18], nullptr, 10);
		char *name = trim(str[1]);

		aegis_itemnames[item_id] = std::string(name);

		if (atoi(str[14]) & (EQP_HELM | EQP_COSTUME_HELM) && util::umap_find(aegis_itemviewid, view_id) == nullptr)
			aegis_itemviewid[view_id] = item_id;

		count++;
	}

	fclose(fp);

	ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, path);

	return true;
}

const std::string ItemDatabase::getDefaultLocation() {
	return std::string(db_path) + "/item_db.yml";
}

uint64 ItemDatabase::parseBodyNode(const ryml::NodeRef& node) {
	t_itemid nameid;

	if (!this->asUInt32(node, "Id", nameid))
		return 0;

	if (this->nodeExists(node, "AegisName")) {
		std::string name;

		if (!this->asString(node, "AegisName", name))
			return 0;

		aegis_itemnames[nameid] = name;
	}

	if (this->nodeExists(node, "View")) {
		uint32 look;

		if (!this->asUInt32(node, "View", look))
			return 0;

		if (look > 0) {
			if (this->nodeExists(node, "Locations")) {
				const ryml::NodeRef& locationNode = node["Locations"];

				static std::vector<std::string> locations = {
					"Head_Low",
					"Head_Mid",
					"Head_Top",
					"Costume_Head_Low",
					"Costume_Head_Mid",
					"Costume_Head_Top"
				};

				for (std::string& location : locations) {
					if (this->nodeExists(locationNode, location)) {
						bool active;

						if (!this->asBool(locationNode, location, active))
							return 0;

						aegis_itemviewid[look] = nameid;
						break;
					}
				}
			}
		}
	}

	return 1;
}

void ItemDatabase::loadingFinished() {
}

ItemDatabase item_db;

static bool parse_mob_constants_txt(char *split[], int columns, int current) {
	uint16 mob_id = atoi(split[0]);
	char *name = trim(split[1]);

	aegis_mobnames[mob_id] = std::string(name);

	return true;
}

static bool parse_skill_constants_txt(char *split[], int columns, int current) {
	uint16 skill_id = atoi(split[0]);
	char *name = trim(split[16]);

	aegis_skillnames[skill_id] = std::string(name);

	return true;
}

const std::string SkillDatabase::getDefaultLocation() {
	return std::string(db_path) + "/skill_db.yml";
}

uint64 SkillDatabase::parseBodyNode(const ryml::NodeRef& node) {
	t_itemid nameid;

	if (!this->asUInt32(node, "Id", nameid))
		return 0;

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		aegis_skillnames[nameid] = name;
	}

	return 1;
}

void SkillDatabase::clear() {
	TypesafeCachedYamlDatabase::clear();
}

void SkillDatabase::loadingFinished(){
}

SkillDatabase skill_db;

const std::string MobDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/mob_db.yml";
}

uint64 MobDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint16 mob_id;

	if (!this->asUInt16(node, "Id", mob_id))
		return 0;

	if (this->nodeExists(node, "AegisName")) {
		std::string name;

		if (!this->asString(node, "AegisName", name))
			return 0;

		aegis_mobnames[mob_id] = name;
	}

	return 1;
}

void MobDatabase::loadingFinished() {};

MobDatabase mob_db;

#endif /* YAML_HPP */
