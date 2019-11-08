// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>

#ifdef WIN32
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
	#include <stdio.h>
#endif

#include <yaml-cpp/yaml.h>

#include "../common/cbasetypes.hpp"
#include "../common/core.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/utilities.hpp"
#ifdef WIN32
#include "../common/winapi.hpp"
#endif

// Only for constants - do not use functions of it or linking will fail
#include "../map/achievement.hpp"
#include "../map/battle.hpp"
#include "../map/battleground.hpp"
#include "../map/channel.hpp"
#include "../map/chat.hpp"
#include "../map/date.hpp"
#include "../map/instance.hpp"
#include "../map/mercenary.hpp"
#include "../map/mob.hpp"
#include "../map/npc.hpp"
#include "../map/pc.hpp"
#include "../map/pet.hpp"
#include "../map/script.hpp"
#include "../map/storage.hpp"
#include "../map/quest.hpp"

using namespace rathena;

#ifndef WIN32
int getch( void ){
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
#endif

// Forward declaration of conversion functions
static size_t upgrade_achievement_db(std::string file, std::ofstream &out);

// Constants for conversion
std::unordered_map<uint16, std::string> aegis_itemnames;
std::unordered_map<uint16, std::string> aegis_mobnames;
std::unordered_map<uint16, std::string> aegis_skillnames;

// Forward declaration of constant loading functions
static bool parse_item_constants(const char* path);
static bool parse_mob_constants(char* split[], int columns, int current);
static bool parse_skill_constants(char* split[], int columns, int current);

bool fileExists(const std::string &path);
uint32 getHeaderVersion(YAML::Node &node);
void prepareHeader(const std::string &type, uint32 version, std::ofstream &out);
bool askConfirmation(const char *fmt, ...);

YAML::Node inNode;
size_t counter;

template<typename Func>
bool process(const std::string &type, uint32 version, const std::vector<std::string> &paths, const std::string &name, Func lambda) {
	for (const std::string &path : paths) {
		const std::string name_ext = name + ".yml";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + name + "-upgrade.yml";

		inNode.reset();
		inNode = YAML::LoadFile(from);

		if (fileExists(from) && getHeaderVersion(inNode) < version) {
			if (!askConfirmation("Found the file \"%s\", which requires an upgrade.\nDo you want to convert it now? (Y/N)\n", from.c_str())) {
				continue;
			}

			if (fileExists(to)) {
				if (!askConfirmation("The file \"%s\" already exists.\nDo you want to replace it? (Y/N)\n", to.c_str())) {
					continue;
				}
			}

			std::ofstream out;

			out.open(to);

			if (!out.is_open()) {
				ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
				return false;
			}

			counter = 0;
			prepareHeader(type, version, out);

			if (!lambda(path, name_ext, out)) {
				return false;
			}
			
			// Make sure there is an empty line at the end of the file for git
#ifdef WIN32
			out << "\r\n";
#else
			out << "\n";
#endif

			out.close();
		}
	}

	return true;
}

int do_init(int argc, char** argv) {
	const std::string path_db = std::string(db_path);
	const std::string path_db_mode = path_db + "/" + DBPATH;
	const std::string path_db_import = path_db + "/" + DBIMPORT;

	// Loads required conversion constants
	parse_item_constants((path_db_mode + "/item_db.txt").c_str());
	parse_item_constants((path_db_import + "/item_db.txt").c_str());
	sv_readdb(path_db_mode.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false);
	sv_readdb(path_db_import.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false);
	sv_readdb(path_db_mode.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants, false);
	sv_readdb(path_db_import.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants, false);

	std::vector<std::string> root_paths = {
		path_db,
		path_db_import
	};

	std::vector<std::string> mode_paths = {
		path_db_mode,
		path_db_import
	};

	if (!process("ACHIEVEMENT_DB", 2, mode_paths, "achievement_db", [](const std::string &path, const std::string &name_ext, std::ofstream &out) -> bool {
		return upgrade_achievement_db(path + name_ext, out);
	})) {
		return 0;
	}

	return 0;
}

void do_final(void){
}

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

uint32 getHeaderVersion(YAML::Node &node) {
	return node["Header"]["Version"].as<uint32>();
}

void prepareHeader(const std::string &type, uint32 version, std::ofstream &out) {
	YAML::Emitter header(out);

	header << YAML::BeginMap;
	header << YAML::Key << "Header";
	header << YAML::Value << YAML::BeginMap;
	header << YAML::Key << "Type" << YAML::Value << type;
	header << YAML::Key << "Version" << YAML::Value << version;
	header << YAML::EndMap;
	header << YAML::EndMap;

#ifdef WIN32
	out << "\r\n";
#else
	out << "\n";
#endif
}

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

// Constant loading functions
static bool parse_item_constants(const char* path) {
	uint32 lines = 0, count = 0;
	char line[1024];

	FILE* fp;

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

		if (p == NULL)
		{
			ShowError("itemdb_readdb: Insufficient columns in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}

		// Script
		if (*p != '{')
		{
			ShowError("itemdb_readdb: Invalid format (Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		str[19] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL)
		{
			ShowError("itemdb_readdb: Invalid format (Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnEquip_Script
		if (*p != '{')
		{
			ShowError("itemdb_readdb: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		str[20] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL)
		{
			ShowError("itemdb_readdb: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnUnequip_Script (last column)
		if (*p != '{')
		{
			ShowError("itemdb_readdb: Invalid format (OnUnequip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
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
				ShowError("itemdb_readdb: Mismatching curly braces in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
		}
		str[21] = str[21] + 1;  //skip the first left curly
		*p = '\0';              //null the last right curly

		uint16 item_id = atoi(str[0]);
		char* name = trim(str[1]);

		aegis_itemnames[item_id] = std::string(name);

		count++;
	}

	fclose(fp);

	ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, path);

	return true;
}

static bool parse_mob_constants(char* split[], int columns, int current) {
	uint16 mob_id = atoi(split[0]);
	char* name = trim(split[1]);

	aegis_mobnames[mob_id] = std::string(name);

	return true;
}

static bool parse_skill_constants(char* split[], int columns, int current) {
	uint16 skill_id = atoi(split[0]);
	char* name = trim(split[16]);

	aegis_skillnames[skill_id] = std::string(name);

	return true;
}

// Implementation of the upgrade functions
static size_t upgrade_achievement_db(std::string file, std::ofstream &out) {
	size_t entries = 0;
	YAML::Emitter entry(out);

	entry << YAML::BeginMap;
	entry << YAML::Key << "Body";
	entry << YAML::BeginSeq;

	for (const auto &input : inNode["Body"]) {
		entry << YAML::BeginMap;
		entry << YAML::Key << "Id" << YAML::Value << input["ID"];
		entry << YAML::Key << "Group" << YAML::Value << input["Group"];
		entry << YAML::Key << "Name" << YAML::Value << input["Name"];

		if (input["Target"].IsDefined()) {
			entry << YAML::Key << "Targets";
			entry << YAML::BeginSeq;

			for (const auto &it : input["Target"]) {
				entry << YAML::BeginMap;
				entry << YAML::Key << "Id" << YAML::Value << it["Id"];
				if (it["MobID"].IsDefined())
					entry << YAML::Key << "Mob" << YAML::Value << *util::umap_find(aegis_mobnames, it["MobID"].as<uint16>());
				if (it["Count"].IsDefined() && it["Count"].as<int>() > 1)
					entry << YAML::Key << "Count" << YAML::Value << it["Count"];
				entry << YAML::EndMap;
			}

			entry << YAML::EndSeq;
		}

		if (input["Condition"].IsDefined())
			entry << YAML::Key << "Condition" << YAML::Value << input["Condition"];

		if (input["Map"].IsDefined())
			entry << YAML::Key << "Map" << YAML::Value << input["Map"];

		if (input["Dependent"].IsDefined()) {
			size_t j = 0;

			entry << YAML::Key << "Dependents";
			entry << YAML::BeginSeq;

			for (const auto &it : input["Dependent"]) {
				entry << YAML::BeginMap;
				entry << YAML::Key << "Id" << YAML::Value << j;
				entry << YAML::Key << "AchievementId" << YAML::Value << it["Id"];
				entry << YAML::EndMap;
				j++;
			}

			entry << YAML::EndSeq;
		}

		if (input["Reward"].IsDefined()) {
			entry << YAML::Key << "Rewards";
			entry << YAML::BeginMap;
			if (input["Reward"]["ItemID"].IsDefined())
				entry << YAML::Key << "Item" << YAML::Value << *util::umap_find(aegis_itemnames, input["Reward"]["ItemID"].as<uint16>());
			if (input["Reward"]["Amount"].IsDefined() && input["Reward"]["Amount"].as<uint16>() > 1)
				entry << YAML::Key << "Amount" << YAML::Value << input["Reward"]["Amount"];
			if (input["Reward"]["Script"].IsDefined())
				entry << YAML::Key << "Script" << YAML::Value << input["Reward"]["Script"];
			if (input["Reward"]["TitleID"].IsDefined())
				entry << YAML::Key << "TitleId" << YAML::Value << input["Reward"]["TitleID"];
			entry << YAML::EndMap;
		}

		entry << YAML::Key << "Score" << YAML::Value << input["Score"];

		entry << YAML::EndMap;
		counter++;
		entries++;
	}

	entry << YAML::EndSeq;
	entry << YAML::EndMap;

	return entries;
}
