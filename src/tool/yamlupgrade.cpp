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
#include "../map/quest.hpp"
#include "../map/script.hpp"
#include "../map/storage.hpp"

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
static bool upgrade_achievement_db(std::string file);

// Constants for conversion
std::unordered_map<uint16, std::string> aegis_itemnames;
std::unordered_map<uint16, uint16> aegis_itemviewid;
std::unordered_map<uint16, std::string> aegis_mobnames;
std::unordered_map<uint16, std::string> aegis_skillnames;
std::unordered_map<const char *, int64> constants;

// Forward declaration of constant loading functions
static bool parse_item_constants(const char* path);
static bool parse_mob_constants(char* split[], int columns, int current);
static bool parse_skill_constants_txt(char *split[], int columns, int current);
static bool parse_skill_constants_yml(std::string path, std::string filename);

bool fileExists(const std::string &path);
bool askConfirmation(const char *fmt, ...);

YAML::Node inNode;
YAML::Emitter body;

// Implement the function instead of including the original version by linking
void script_set_constant_(const char *name, int64 value, const char *constant_name, bool isparameter, bool deprecated) {
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

uint32 getHeaderVersion(YAML::Node &node) {
	return node["Header"]["Version"].as<uint32>();
}

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

void prepareBody(void) {
	body << YAML::BeginMap;
	body << YAML::Key << "Body";
	body << YAML::BeginSeq;
}

void finalizeBody(void) {
	body << YAML::EndSeq;
	body << YAML::EndMap;
}

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

			body.~Emitter();
			new (&body) YAML::Emitter();
			out.open(to);

			if (!out.is_open()) {
				ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
				return false;
			}

			prepareHeader(out, type, version, name);
			if (inNode["Body"].IsDefined()) {
				prepareBody();

				if (!lambda(path, name_ext)) {
					out.close();
					return false;
				}

				finalizeBody();
				out << body.c_str();
			}
			prepareFooter(out);
			// Make sure there is an empty line at the end of the file for git
			out << "\n";
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
	if (fileExists(path_db + "/" + "skill_db.yml")) {
		parse_skill_constants_yml(path_db_mode, "skill_db.yml");
		parse_skill_constants_yml(path_db_import + "/", "skill_db.yml");
	} else {
		sv_readdb(path_db_mode.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
		sv_readdb(path_db_import.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
	}

	// Load constants
	#define export_constant_npc(a) export_constant(a)
	#include "../map/script_constants.hpp"

	std::vector<std::string> root_paths = {
		path_db,
		path_db_mode,
		path_db_import
	};

	if (!process("ACHIEVEMENT_DB", 2, root_paths, "achievement_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return upgrade_achievement_db(path + name_ext);
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

static bool parse_skill_constants_txt(char* split[], int columns, int current) {
	uint16 skill_id = atoi(split[0]);
	char* name = trim(split[16]);

	aegis_skillnames[skill_id] = std::string(name);

	return true;
}

static bool parse_skill_constants_yml(std::string path, std::string filename) {
	YAML::Node rootNode;

	try {
		rootNode = YAML::LoadFile(path + filename);
	} catch (YAML::Exception & e) {
		ShowError("Failed to read file from '" CL_WHITE "%s%s" CL_RESET "'.\n", path.c_str(), filename.c_str());
		ShowError("%s (Line %d: Column %d)\n", e.msg.c_str(), e.mark.line, e.mark.column);
		return false;
	}

	uint64 count = 0;

	for (const YAML::Node &body : rootNode["Body"]) {
		aegis_skillnames[body["Id"].as<uint16>()] = body["Name"].as<std::string>();
		count++;
	}

	ShowStatus("Done reading '" CL_WHITE "%" PRIu64 CL_RESET "' entries in '" CL_WHITE "%s%s" CL_RESET "'" CL_CLL "\n", count, path.c_str(), filename.c_str());

	return true;
}

std::string name2Upper(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	name[0] = toupper(name[0]);

	for (size_t i = 0; i < name.size(); i++) {
		if (name[i - 1] == '_' || (name[i - 2] == '1' && name[i - 1] == 'h') || (name[i - 2] == '2' && name[i - 1] == 'h'))
			name[i] = toupper(name[i]);
	}

	return name;
}

// Implementation of the upgrade functions
static bool upgrade_achievement_db(std::string file) {
	size_t entries = 0;

	for (const auto &input : inNode["Body"]) {
		body << YAML::BeginMap;
		body << YAML::Key << "Id" << YAML::Value << input["ID"];

		std::string constant = input["Group"].as<std::string>();

		constant.erase(0, 3); // Remove "AG_"
		body << YAML::Key << "Group" << YAML::Value << name2Upper(constant);
		body << YAML::Key << "Name" << YAML::Value << input["Name"];

		if (input["Target"].IsDefined()) {
			body << YAML::Key << "Targets";
			body << YAML::BeginSeq;

			for (const auto &it : input["Target"]) {
				body << YAML::BeginMap;
				body << YAML::Key << "Id" << YAML::Value << it["Id"];
				if (it["MobID"].IsDefined())
					body << YAML::Key << "Mob" << YAML::Value << *util::umap_find(aegis_mobnames, it["MobID"].as<uint16>());
				if (it["Count"].IsDefined() && it["Count"].as<int32>() > 1)
					body << YAML::Key << "Count" << YAML::Value << it["Count"];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}

		if (input["Condition"].IsDefined())
			body << YAML::Key << "Condition" << YAML::Value << input["Condition"];

		if (input["Map"].IsDefined())
			body << YAML::Key << "Map" << YAML::Value << input["Map"];

		if (input["Dependent"].IsDefined()) {
			body << YAML::Key << "Dependents";
			body << YAML::BeginSeq;

			for (const auto &it : input["Dependent"]) {
				body << YAML::BeginMap;
				body << YAML::Key << "Id" << YAML::Value << it["Id"];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}

		if (input["Reward"].IsDefined()) {
			body << YAML::Key << "Rewards";
			body << YAML::BeginMap;
			if (input["Reward"]["ItemID"].IsDefined())
				body << YAML::Key << "Item" << YAML::Value << *util::umap_find(aegis_itemnames, input["Reward"]["ItemID"].as<uint16>());
			if (input["Reward"]["Amount"].IsDefined() && input["Reward"]["Amount"].as<uint16>() > 1)
				body << YAML::Key << "Amount" << YAML::Value << input["Reward"]["Amount"];
			if (input["Reward"]["Script"].IsDefined())
				body << YAML::Key << "Script" << YAML::Value << input["Reward"]["Script"];
			if (input["Reward"]["TitleID"].IsDefined())
				body << YAML::Key << "TitleId" << YAML::Value << input["Reward"]["TitleID"];
			body << YAML::EndMap;
		}

		body << YAML::Key << "Score" << YAML::Value << input["Score"];

		body << YAML::EndMap;
		entries++;
	}

	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' achievements in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}
