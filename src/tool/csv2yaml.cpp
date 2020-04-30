// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
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

#include "../common/cbasetypes.hpp"
#include "../common/core.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"
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
#include "../map/skill.hpp"
#include "../map/storage.hpp"

using namespace rathena;

#define MAX_MAP_PER_INSTANCE 255

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

// Required constant and structure definitions
#define MAX_GUILD_SKILL_REQUIRE 5
#define MAX_SKILL_ITEM_REQUIRE	10
#define MAX_SKILL_STATUS_REQUIRE 3
#define MAX_SKILL_EQUIP_REQUIRE 10
#define MAX_QUEST_DROPS 3

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

// Forward declaration of conversion functions
static bool guild_read_guildskill_tree_db( char* split[], int columns, int current );
static bool pet_read_db( const char* file );
static bool skill_parse_row_magicmushroomdb(char* split[], int column, int current);
static bool skill_parse_row_abradb(char* split[], int columns, int current);
static bool skill_parse_row_improvisedb(char* split[], int columns, int current);
static bool skill_parse_row_spellbookdb(char* split[], int columns, int current);
static bool mob_readdb_mobavail(char *str[], int columns, int current);
static bool skill_parse_row_requiredb(char* split[], int columns, int current);
static bool skill_parse_row_castdb(char* split[], int columns, int current);
static bool skill_parse_row_castnodexdb(char* split[], int columns, int current);
static bool skill_parse_row_unitdb(char* split[], int columns, int current);
static bool skill_parse_row_copyabledb(char* split[], int columns, int current);
static bool skill_parse_row_nonearnpcrangedb(char* split[], int columns, int current);
static bool skill_parse_row_skilldb(char* split[], int columns, int current);
static bool quest_read_db(char *split[], int columns, int current);
static bool instance_readdb_sub(char* str[], int columns, int current);

// Constants for conversion
std::unordered_map<uint16, std::string> aegis_itemnames;
std::unordered_map<uint16, uint16> aegis_itemviewid;
std::unordered_map<uint16, std::string> aegis_mobnames;
std::unordered_map<uint16, std::string> aegis_skillnames;
std::unordered_map<const char*, int64> constants;

// Forward declaration of constant loading functions
static bool parse_item_constants( const char* path );
static bool parse_mob_constants( char* split[], int columns, int current );
static bool parse_skill_constants_txt( char* split[], int columns, int current );
static bool parse_skill_constants_yml(std::string path, std::string filename);

bool fileExists( const std::string& path );
bool askConfirmation( const char* fmt, ... );

// Skill database data to memory
static void skill_txt_data(const std::string& modePath, const std::string& fixedPath) {
	skill_require.clear();
	skill_cast.clear();
	skill_castnodex.clear();
	skill_unit.clear();
	skill_copyable.clear();
	skill_nearnpc.clear();

	if (fileExists(modePath + "/skill_require_db.txt"))
		sv_readdb(modePath.c_str(), "skill_require_db.txt", ',', 34, 34, -1, skill_parse_row_requiredb, false);
	if (fileExists(modePath + "/skill_cast_db.txt"))
		sv_readdb(modePath.c_str(), "skill_cast_db.txt", ',', 7, 8, -1, skill_parse_row_castdb, false);
	if (fileExists(modePath + "/skill_castnodex_db.txt"))
		sv_readdb(modePath.c_str(), "skill_castnodex_db.txt", ',', 2, 3, -1, skill_parse_row_castnodexdb, false);
	if (fileExists(modePath + "/skill_unit_db.txt"))
		sv_readdb(modePath.c_str(), "skill_unit_db.txt", ',', 8, 8, -1, skill_parse_row_unitdb, false);
	if (fileExists(fixedPath + "/skill_copyable_db.txt"))
		sv_readdb(fixedPath.c_str(), "skill_copyable_db.txt", ',', 2, 4, -1, skill_parse_row_copyabledb, false);
	if (fileExists(fixedPath + "/skill_nonearnpc_db.txt"))
		sv_readdb(fixedPath.c_str(), "skill_nonearnpc_db.txt", ',', 2, 3, -1, skill_parse_row_nonearnpcrangedb, false);
}

YAML::Emitter body;

// Implement the function instead of including the original version by linking
void script_set_constant_( const char* name, int64 value, const char* constant_name, bool isparameter, bool deprecated ){
	constants[name] = value;
}

const char* constant_lookup( int32 value, const char* prefix ){
	nullpo_retr( nullptr, prefix );

	for( auto const& pair : constants ){
		// Same prefix group and same value
		if( strncasecmp( pair.first, prefix, strlen( prefix ) ) == 0 && pair.second == value ){
			return pair.first;
		}
	}

	return nullptr;
}

int64 constant_lookup_int(const char* constant) {
	nullpo_retr(-100, constant);

	for (auto const &pair : constants) {
		if (strlen(pair.first) == strlen(constant) && strncasecmp(pair.first, constant, strlen(constant)) == 0) {
			return pair.second;
		}
	}

	return -100;
}

void copyFileIfExists( std::ofstream& file,const std::string& name, bool newLine ){
	std::string path = "doc/yaml/db/" + name + ".yml";

	if( fileExists( path ) ){
		std::ifstream source( path, std::ios::binary );

		std::istreambuf_iterator<char> begin_source( source );
		std::istreambuf_iterator<char> end_source;
		std::ostreambuf_iterator<char> begin_dest( file );
		copy( begin_source, end_source, begin_dest );

		source.close();

		if( newLine ){
			file << "\n";
		}
	}
}

void prepareHeader(std::ofstream &file, const std::string& type, uint32 version, const std::string& name) {
	copyFileIfExists( file, "license", false );
	copyFileIfExists( file, name, true );

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

template<typename Func>
bool process( const std::string& type, uint32 version, const std::vector<std::string>& paths, const std::string& name, Func lambda, const std::string& rename = "" ){
	for( const std::string& path : paths ){
		const std::string name_ext = name + ".txt";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + (rename.size() > 0 ? rename : name) + ".yml";

		if( fileExists( from ) ){
			if( !askConfirmation( "Found the file \"%s\", which requires migration to yml.\nDo you want to convert it now? (Y/N)\n", from.c_str() ) ){
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

			prepareHeader(out, type, version, (rename.size() > 0 ? rename : name));
			prepareBody();

			if( !lambda( path, name_ext ) ){
				out.close();
				return false;
			}

			finalizeBody();
			out << body.c_str();
			// Make sure there is an empty line at the end of the file for git
			out << "\n";
			out.close();
			
			// TODO: do you want to delete?
		}
	}

	return true;
}

int do_init( int argc, char** argv ){
	const std::string path_db = std::string( db_path );
	const std::string path_db_mode = path_db + "/" + DBPATH;
	const std::string path_db_import = path_db + "/" + DBIMPORT;

	// Loads required conversion constants
	parse_item_constants( ( path_db_mode + "/item_db.txt" ).c_str() );
	parse_item_constants( ( path_db_import + "/item_db.txt" ).c_str() );
	sv_readdb( path_db_mode.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false );
	sv_readdb( path_db_import.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false );
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

	if( !process( "GUILD_SKILL_TREE_DB", 1, root_paths, "guild_skill_tree", []( const std::string& path, const std::string& name_ext ) -> bool {
		return sv_readdb( path.c_str(), name_ext.c_str(), ',', 2 + MAX_GUILD_SKILL_REQUIRE * 2, 2 + MAX_GUILD_SKILL_REQUIRE * 2, -1, &guild_read_guildskill_tree_db, false );
	} ) ){
		return 0;
	}

	if( !process( "PET_DB", 1, root_paths, "pet_db", []( const std::string& path, const std::string& name_ext ) -> bool {
		return pet_read_db( ( path + name_ext ).c_str() );
	} ) ){
		return 0;
	}

	if (!process("MAGIC_MUSHROOM_DB", 1, root_paths, "magicmushroom_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 1, 1, -1, &skill_parse_row_magicmushroomdb, false);
	})) {
		return 0;
	}

	if (!process("ABRA_DB", 1, root_paths, "abra_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3, 3, -1, &skill_parse_row_abradb, false);
	})) {
		return 0;
	}

	if (!process("IMPROVISED_SONG_DB", 1, root_paths, "skill_improvise_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 2, 2, -1, &skill_parse_row_improvisedb, false);
	}, "improvise_db")) {
		return 0;
	}

	if (!process("READING_SPELLBOOK_DB", 1, root_paths, "spellbook_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3, 3, -1, &skill_parse_row_spellbookdb, false);
	})) {
		return 0;
	}

	if (!process("MOB_AVAIL_DB", 1, root_paths, "mob_avail", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 2, 12, -1, &mob_readdb_mobavail, false);
	})) {
		return 0;
	}

	skill_txt_data( path_db_mode, path_db );
	if (!process("SKILL_DB", 1, { path_db_mode }, "skill_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 18, 18, -1, &skill_parse_row_skilldb, false);
	})){
		return 0;
	}

	skill_txt_data( path_db_import, path_db_import );
	if (!process("SKILL_DB", 1, { path_db_import }, "skill_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 18, 18, -1, &skill_parse_row_skilldb, false);
	})){
		return 0;
	}

	if (!process("QUEST_DB", 1, root_paths, "quest_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3 + MAX_QUEST_OBJECTIVES * 2 + MAX_QUEST_DROPS * 3, 100, -1, &quest_read_db, false);
	})) {
		return 0;
	}

	if (process("INSTANCE_DB", 1, root_paths, "instance_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 7, 7 + MAX_MAP_PER_INSTANCE, -1, &instance_readdb_sub, false);
	})) {
		return 0;
	}

	// TODO: add implementations ;-)

	return 0;
}

void do_final(void){
}

bool fileExists( const std::string& path ){
	std::ifstream in;

	in.open( path );

	if( in.is_open() ){
		in.close();

		return true;
	}else{
		return false;
	}
}

bool askConfirmation( const char* fmt, ... ){
	va_list ap;

	va_start( ap, fmt );

	_vShowMessage( MSG_NONE, fmt, ap );

	va_end( ap );

	char c = getch();

	if( c == 'Y' || c == 'y' ){
		return true;
	}else{
		return false;
	}
}

// Constant loading functions
static bool parse_item_constants( const char* path ){
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

		uint16 item_id = atoi( str[0] );
		char* name = trim( str[1] );

		aegis_itemnames[item_id] = std::string(name);

		if (atoi(str[14]) & (EQP_HELM | EQP_COSTUME_HELM) && util::umap_find(aegis_itemviewid, (uint16)atoi(str[18])) == nullptr)
			aegis_itemviewid[atoi(str[18])] = item_id;

		count++;
	}

	fclose(fp);

	ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, path);

	return true;
}

static bool parse_mob_constants( char* split[], int columns, int current ){
	uint16 mob_id = atoi( split[0] );
	char* name = trim( split[1] );

	aegis_mobnames[mob_id] = std::string( name );

	return true;
}

static bool parse_skill_constants_txt( char* split[], int columns, int current ){
	uint16 skill_id = atoi( split[0] );
	char* name = trim( split[16] );

	aegis_skillnames[skill_id] = std::string( name );

	return true;
}

static bool parse_skill_constants_yml(std::string path, std::string filename) {
	YAML::Node rootNode;

	try {
		rootNode = YAML::LoadFile(path + filename);
	} catch (YAML::Exception &e) {
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

/**
 * Split the string with ':' as separator and put each value for a skilllv
 * @param str: String to split
 * @param val: Array of MAX_SKILL_LEVEL to put value into
 * @return 0:error, x:number of value assign (should be MAX_SKILL_LEVEL)
 */
int skill_split_atoi(char *str, int *val) {
	int i;

	for (i = 0; i < MAX_SKILL_LEVEL; i++) {
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
 * Split string to int by constant value (const.txt) or atoi()
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

// Implementation of the conversion functions

// Copied and adjusted from guild.cpp
// <skill id>,<max lv>,<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>
static bool guild_read_guildskill_tree_db( char* split[], int columns, int current ){
	uint16 skill_id = (uint16)atoi(split[0]);
	std::string* name = util::umap_find( aegis_skillnames, skill_id );

	if( name == nullptr ){
		ShowError( "Skill name for skill id %hu is not known.\n", skill_id );
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << *name;
	body << YAML::Key << "MaxLevel" << YAML::Value << atoi(split[1]);

	if (atoi(split[2]) > 0) {
		body << YAML::Key << "Required";
		body << YAML::BeginSeq;

		for (int i = 0, j = 0; i < MAX_GUILD_SKILL_REQUIRE; i++) {
			uint16 required_skill_id = atoi(split[i * 2 + 2]);
			uint16 required_skill_level = atoi(split[i * 2 + 3]);

			if (required_skill_id == 0 || required_skill_level == 0) {
				continue;
			}

			std::string* required_name = util::umap_find(aegis_skillnames, required_skill_id);

			if (required_name == nullptr) {
				ShowError("Skill name for required skill id %hu is not known.\n", required_skill_id);
				return false;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Id" << YAML::Value << *required_name;
			body << YAML::Key << "Level" << YAML::Value << required_skill_level;
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from pet.cpp
static bool pet_read_db( const char* file ){
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "can't read %s\n", file );
		return false;
	}

	int lines = 0;
	size_t entries = 0;
	char line[1024];

	while( fgets( line, sizeof(line), fp ) ) {
		char *str[22], *p;
		unsigned k;
		lines++;

		if(line[0] == '/' && line[1] == '/')
			continue;

		memset(str, 0, sizeof(str));
		p = line;

		while( ISSPACE(*p) )
			++p;

		if( *p == '\0' )
			continue; // empty line

		for( k = 0; k < 20; ++k ) {
			str[k] = p;
			p = strchr(p,',');

			if( p == NULL )
				break; // comma not found

			*p = '\0';
			++p;
		}

		if( p == NULL ) {
			ShowError("read_petdb: Insufficient columns in line %d, skipping.\n", lines);
			continue;
		}

		// Pet Script
		if( *p != '{' ) {
			ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
			continue;
		}

		str[20] = p;
		p = strstr(p+1,"},");

		if( p == NULL ) {
			ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
			continue;
		}

		p[1] = '\0';
		p += 2;

		// Equip Script
		if( *p != '{' ) {
			ShowError("read_petdb: Invalid format (Equip Script column) in line %d, skipping.\n", lines);
			continue;
		}

		str[21] = p;

		uint16 mob_id = atoi( str[0] );
		std::string* mob_name = util::umap_find( aegis_mobnames, mob_id );

		if( mob_name == nullptr ){
			ShowWarning( "pet_db reading: Invalid mob-class %hu, pet not read.\n", mob_id );
			continue;
		}

		body << YAML::BeginMap;
		body << YAML::Key << "Mob" << YAML::Value << *mob_name;

		uint16 tame_item_id = (uint16)atoi( str[3] );

		if( tame_item_id > 0 ){
			std::string* tame_item_name = util::umap_find( aegis_itemnames, tame_item_id );

			if( tame_item_name == nullptr ){
				ShowError( "Item name for item id %hu is not known.\n", tame_item_id );
				return false;
			}

			body << YAML::Key << "TameItem" << YAML::Value << *tame_item_name;
		}

		uint16 egg_item_id = (uint16)atoi( str[4] );
		std::string* egg_item_name = util::umap_find( aegis_itemnames, egg_item_id );

		if( egg_item_name == nullptr ){
			ShowError( "Item name for item id %hu is not known.\n", egg_item_id );
			return false;
		}

		body << YAML::Key << "EggItem" << YAML::Value << *egg_item_name;

		uint16 equip_item_id = (uint16)atoi( str[5] );

		if( equip_item_id > 0 ){
			std::string* equip_item_name = util::umap_find( aegis_itemnames, equip_item_id );

			if( equip_item_name == nullptr ){
				ShowError( "Item name for item id %hu is not known.\n", equip_item_id );
				return false;
			}

			body << YAML::Key << "EquipItem" << YAML::Value << *equip_item_name;
		}

		uint16 food_item_id = (uint16)atoi( str[6] );

		if( food_item_id > 0 ){
			std::string* food_item_name = util::umap_find( aegis_itemnames, food_item_id );

			if( food_item_name == nullptr ){
				ShowError( "Item name for item id %hu is not known.\n", food_item_id );
				return false;
			}

			body << YAML::Key << "FoodItem" << YAML::Value << *food_item_name;
		}

		body << YAML::Key << "Fullness" << YAML::Value << atoi(str[7]);
		// Default: 60
		if( atoi( str[8] ) != 60 ){
			body << YAML::Key << "HungryDelay" << YAML::Value << atoi(str[8]);
		}
		// Default: 250
		if( atoi( str[11] ) != 250 ){
			body << YAML::Key << "IntimacyStart" << YAML::Value << atoi(str[11]);
		}
		body << YAML::Key << "IntimacyFed" << YAML::Value << atoi(str[9]);
		// Default: -100
		if( atoi( str[10] ) != 100 ){
			body << YAML::Key << "IntimacyOverfed" << YAML::Value << -atoi(str[10]);
		}
		// pet_hungry_friendly_decrease battle_conf
		//body << YAML::Key << "IntimacyHungry" << YAML::Value << -5;
		// Default: -20
		if( atoi( str[12] ) != 20 ){
			body << YAML::Key << "IntimacyOwnerDie" << YAML::Value << -atoi(str[12]);
		}
		body << YAML::Key << "CaptureRate" << YAML::Value << atoi(str[13]);
		// Default: true
		if( atoi( str[15] ) == 0 ){
			body << YAML::Key << "SpecialPerformance" << YAML::Value << "false";
		}
		body << YAML::Key << "AttackRate" << YAML::Value << atoi(str[17]);
		body << YAML::Key << "RetaliateRate" << YAML::Value << atoi(str[18]);
		body << YAML::Key << "ChangeTargetRate" << YAML::Value << atoi(str[19]);

		if( *str[21] ){
			body << YAML::Key << "Script" << YAML::Value << YAML::Literal << str[21];
		}

		if( *str[20] ){
			body << YAML::Key << "SupportScript" << YAML::Value << YAML::Literal << str[20];
		}

		body << YAML::EndMap;
		entries++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' pets in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file );

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_magicmushroomdb(char* split[], int column, int current)
{
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Magic Mushroom skill ID %hu is not known.\n", skill_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_abradb(char* split[], int columns, int current)
{
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Abra skill ID %hu is not known.\n", skill_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;

	int arr[MAX_SKILL_LEVEL];
	int arr_size = skill_split_atoi(split[2], arr);

	if (arr_size == 1) {
		if (arr[0] != 500)
			body << YAML::Key << "Probability" << YAML::Value << arr[0];
	} else {
		body << YAML::Key << "Probability";
		body << YAML::BeginSeq;

		for (int i = 0; i < arr_size; i++) {
			if (arr[i] > 0) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Probability" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_improvisedb(char* split[], int columns, int current)
{
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Improvised Song skill ID %hu is not known.\n", skill_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;
	body << YAML::Key << "Probability" << YAML::Value << atoi(split[1]) / 10;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_spellbookdb(char* split[], int columns, int current)
{
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Spell Book skill ID %hu is not known.\n", skill_id);
		return false;
	}

	uint16 nameid = atoi(split[2]);
	std::string *book_name = util::umap_find(aegis_itemnames, nameid);

	if (book_name == nullptr) {
		ShowError("Book name for item ID %hu is not known.\n", nameid);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;
	body << YAML::Key << "Book" << YAML::Value << *book_name;
	body << YAML::Key << "PreservePoints" << YAML::Value << atoi(split[1]);
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from mob.cpp
static bool mob_readdb_mobavail(char* str[], int columns, int current) {
	uint16 mob_id = atoi(str[0]);
	std::string *mob_name = util::umap_find(aegis_mobnames, mob_id);

	if (mob_name == nullptr) {
		ShowWarning("mob_avail reading: Invalid mob-class %hu, Mob not read.\n", mob_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Mob" << YAML::Value << *mob_name;

	uint16 sprite_id = atoi(str[1]);
	std::string *sprite_name = util::umap_find(aegis_mobnames, sprite_id);

	if (sprite_name == nullptr) {
		char *sprite = const_cast<char *>(constant_lookup(sprite_id, "JOB_"));

		if (sprite == nullptr) {
			sprite = const_cast<char *>(constant_lookup(sprite_id, "JT_"));

			if (sprite == nullptr) {
				ShowError("Sprite name for id %d is not known.\n", sprite_id);
				return false;
			}

			sprite += 3; // Strip JT_ here because the script engine doesn't send this prefix for NPC.

			body << YAML::Key << "Sprite" << YAML::Value << sprite;
		} else
			body << YAML::Key << "Sprite" << YAML::Value << sprite;
	} else
		body << YAML::Key << "Sprite" << YAML::Value << *sprite_name;

	if (columns == 12) {
		body << YAML::Key << "Sex" << YAML::Value << (atoi(str[2]) ? "Male" : "Female");
		if (atoi(str[3]) != 0)
			body << YAML::Key << "HairStyle" << YAML::Value << atoi(str[3]);
		if (atoi(str[4]) != 0)
			body << YAML::Key << "HairColor" << YAML::Value << atoi(str[4]);
		if (atoi(str[11]) != 0)
			body << YAML::Key << "ClothColor" << YAML::Value << atoi(str[11]);

		if (atoi(str[5]) != 0) {
			uint16 weapon_item_id = atoi(str[5]);
			std::string *weapon_item_name = util::umap_find(aegis_itemnames, weapon_item_id);

			if (weapon_item_name == nullptr) {
				ShowError("Item name for item ID %hu (weapon) is not known.\n", weapon_item_id);
				return false;
			}

			body << YAML::Key << "Weapon" << YAML::Value << *weapon_item_name;
		}

		if (atoi(str[6]) != 0) {
			uint16 shield_item_id = atoi(str[6]);
			std::string *shield_item_name = util::umap_find(aegis_itemnames, shield_item_id);

			if (shield_item_name == nullptr) {
				ShowError("Item name for item ID %hu (shield) is not known.\n", shield_item_id);
				return false;
			}

			body << YAML::Key << "Shield" << YAML::Value << *shield_item_name;
		}

		if (atoi(str[7]) != 0) {
			uint16 *headtop_item_id = util::umap_find(aegis_itemviewid, (uint16)atoi(str[7]));

			if (headtop_item_id == nullptr) {
				ShowError("Item ID for view ID %hu (head top) is not known.\n", atoi(str[7]));
				return false;
			}

			std::string *headtop_item_name = util::umap_find(aegis_itemnames, *headtop_item_id);

			if (headtop_item_name == nullptr) {
				ShowError("Item name for item ID %hu (head top) is not known.\n", *headtop_item_id);
				return false;
			}

			body << YAML::Key << "HeadTop" << YAML::Value << *headtop_item_name;
		}

		if (atoi(str[8]) != 0) {
			uint16 *headmid_item_id = util::umap_find(aegis_itemviewid, (uint16)atoi(str[8]));

			if (headmid_item_id == nullptr) {
				ShowError("Item ID for view ID %hu (head mid) is not known.\n", atoi(str[8]));
				return false;
			}

			std::string *headmid_item_name = util::umap_find(aegis_itemnames, *headmid_item_id);

			if (headmid_item_name == nullptr) {
				ShowError("Item name for item ID %hu (head mid) is not known.\n", *headmid_item_id);
				return false;
			}

			body << YAML::Key << "HeadMid" << YAML::Value << *headmid_item_name;
		}

		if (atoi(str[9]) != 0) {
			uint16 *headlow_item_id = util::umap_find(aegis_itemviewid, (uint16)atoi(str[9]));

			if (headlow_item_id == nullptr) {
				ShowError("Item ID for view ID %hu (head low) is not known.\n", atoi(str[9]));
				return false;
			}

			std::string *headlow_item_name = util::umap_find(aegis_itemnames, *headlow_item_id);

			if (headlow_item_name == nullptr) {
				ShowError("Item name for item ID %hu (head low) is not known.\n", *headlow_item_id);
				return false;
			}

			body << YAML::Key << "HeadLow" << YAML::Value << *headlow_item_name;
		}

		if (atoi(str[10]) != 0) {
			uint32 options = atoi(str[10]);

			body << YAML::Key << "Options";
			body << YAML::BeginMap;

			while (options > OPTION_NOTHING && options <= OPTION_SUMMER2) {
				if (options & OPTION_SIGHT) {
					body << YAML::Key << "Sight" << YAML::Value << "true";
					options &= ~OPTION_SIGHT;
				} else if (options & OPTION_CART1) {
					body << YAML::Key << "Cart1" << YAML::Value << "true";
					options &= ~OPTION_CART1;
				} else if (options & OPTION_FALCON) {
					body << YAML::Key << "Falcon" << YAML::Value << "true";
					options &= ~OPTION_FALCON;
				} else if (options & OPTION_RIDING) {
					body << YAML::Key << "Riding" << YAML::Value << "true";
					options &= ~OPTION_RIDING;
				} else if (options & OPTION_CART2) {
					body << YAML::Key << "Cart2" << YAML::Value << "true";
					options &= ~OPTION_CART2;
				} else if (options & OPTION_CART3) {
					body << YAML::Key << "Cart2" << YAML::Value << "true";
					options &= ~OPTION_CART3;
				} else if (options & OPTION_CART4) {
					body << YAML::Key << "Cart4" << YAML::Value << "true";
					options &= ~OPTION_CART4;
				} else if (options & OPTION_CART5) {
					body << YAML::Key << "Cart5" << YAML::Value << "true";
					options &= ~OPTION_CART5;
				} else if (options & OPTION_ORCISH) {
					body << YAML::Key << "Orcish" << YAML::Value << "true";
					options &= ~OPTION_ORCISH;
				} else if (options & OPTION_WEDDING) {
					body << YAML::Key << "Wedding" << YAML::Value << "true";
					options &= ~OPTION_WEDDING;
				} else if (options & OPTION_RUWACH) {
					body << YAML::Key << "Ruwach" << YAML::Value << "true";
					options &= ~OPTION_RUWACH;
				} else if (options & OPTION_FLYING) {
					body << YAML::Key << "Flying" << YAML::Value << "true";
					options &= ~OPTION_FLYING;
				} else if (options & OPTION_XMAS) {
					body << YAML::Key << "Xmas" << YAML::Value << "true";
					options &= ~OPTION_XMAS;
				} else if (options & OPTION_TRANSFORM) {
					body << YAML::Key << "Transform" << YAML::Value << "true";
					options &= ~OPTION_TRANSFORM;
				} else if (options & OPTION_SUMMER) {
					body << YAML::Key << "Summer" << YAML::Value << "true";
					options &= ~OPTION_SUMMER;
				} else if (options & OPTION_DRAGON1) {
					body << YAML::Key << "Dragon1" << YAML::Value << "true";
					options &= ~OPTION_DRAGON1;
				} else if (options & OPTION_WUG) {
					body << YAML::Key << "Wug" << YAML::Value << "true";
					options &= ~OPTION_WUG;
				} else if (options & OPTION_WUGRIDER) {
					body << YAML::Key << "WugRider" << YAML::Value << "true";
					options &= ~OPTION_WUGRIDER;
				} else if (options & OPTION_MADOGEAR) {
					body << YAML::Key << "MadoGear" << YAML::Value << "true";
					options &= ~OPTION_MADOGEAR;
				} else if (options & OPTION_DRAGON2) {
					body << YAML::Key << "Dragon2" << YAML::Value << "true";
					options &= ~OPTION_DRAGON2;
				} else if (options & OPTION_DRAGON3) {
					body << YAML::Key << "Dragon3" << YAML::Value << "true";
					options &= ~OPTION_DRAGON3;
				} else if (options & OPTION_DRAGON4) {
					body << YAML::Key << "Dragon4" << YAML::Value << "true";
					options &= ~OPTION_DRAGON4;
				} else if (options & OPTION_DRAGON5) {
					body << YAML::Key << "Dragon5" << YAML::Value << "true";
					options &= ~OPTION_DRAGON5;
				} else if (options & OPTION_HANBOK) {
					body << YAML::Key << "Hanbok" << YAML::Value << "true";
					options &= ~OPTION_HANBOK;
				} else if (options & OPTION_OKTOBERFEST) {
					body << YAML::Key << "Oktoberfest" << YAML::Value << "true";
					options &= ~OPTION_OKTOBERFEST;
				} else if (options & OPTION_SUMMER2) {
					body << YAML::Key << "Summer2" << YAML::Value << "true";
					options &= ~OPTION_SUMMER2;
				}
			}

			body << YAML::EndMap;
		}
	} else if (columns == 3) {
		if (atoi(str[5]) != 0) {
			uint16 peteq_item_id = atoi(str[5]);
			std::string *peteq_item_name = util::umap_find(aegis_itemnames, peteq_item_id);

			if (peteq_item_name == nullptr) {
				ShowError("Item name for item ID %hu (pet equip) is not known.\n", peteq_item_id);
				return false;
			}

			body << YAML::Key << "PetEquip" << YAML::Value << *peteq_item_name;
		}
	}

	body << YAML::EndMap;

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_requiredb(char* split[], int columns, int current)
{
	s_skill_require entry = {};

	skill_split_atoi(split[1], entry.hp);
	skill_split_atoi(split[2], entry.mhp);
	skill_split_atoi(split[3], entry.sp);
	skill_split_atoi(split[4], entry.hp_rate);
	skill_split_atoi(split[5], entry.sp_rate);
	skill_split_atoi(split[6], entry.zeny);

	char *p;

	p = split[7];
	while (p) {
		int l = atoi(p);

		if (l == 99) { // Any weapon
			entry.weapon = 0;
			break;
		} else
			entry.weapon |= 1 << l;
		p = strchr(p, ':');
		if (!p)
			break;
		p++;
	}

	p = split[8];
	while (p) {
		int l = atoi(p);

		if (l == 99) { // Any ammo type
			entry.ammo = AMMO_TYPE_ALL;
			break;
		} else if (l) // 0 stands for no requirement
			entry.ammo |= 1 << l;
		p = strchr(p, ':');
		if (!p)
			break;
		p++;
	}

	skill_split_atoi(split[9], entry.ammo_qty);

	if (strcmpi(split[10], "hidden") == 0) entry.state = ST_HIDDEN;
	else if (strcmpi(split[10], "riding") == 0) entry.state = ST_RIDING;
	else if (strcmpi(split[10], "falcon") == 0) entry.state = ST_FALCON;
	else if (strcmpi(split[10], "cart") == 0) entry.state = ST_CART;
	else if (strcmpi(split[10], "shield") == 0) entry.state = ST_SHIELD;
	else if (strcmpi(split[10], "recover_weight_rate") == 0) entry.state = ST_RECOVER_WEIGHT_RATE;
	else if (strcmpi(split[10], "move_enable") == 0) entry.state = ST_MOVE_ENABLE;
	else if (strcmpi(split[10], "water") == 0) entry.state = ST_WATER;
	else if (strcmpi(split[10], "dragon") == 0) entry.state = ST_RIDINGDRAGON;
	else if (strcmpi(split[10], "warg") == 0) entry.state = ST_WUG;
	else if (strcmpi(split[10], "ridingwarg") == 0) entry.state = ST_RIDINGWUG;
	else if (strcmpi(split[10], "mado") == 0) entry.state = ST_MADO;
	else if (strcmpi(split[10], "elementalspirit") == 0) entry.state = ST_ELEMENTALSPIRIT;
	else if (strcmpi(split[10], "elementalspirit2") == 0) entry.state = ST_ELEMENTALSPIRIT2;
	else if (strcmpi(split[10], "peco") == 0) entry.state = ST_PECO;
	else if (strcmpi(split[10], "sunstance") == 0) entry.state = ST_SUNSTANCE;
	else if (strcmpi(split[10], "moonstance") == 0) entry.state = ST_MOONSTANCE;
	else if (strcmpi(split[10], "starstance") == 0) entry.state = ST_STARSTANCE;
	else if (strcmpi(split[10], "universestance") == 0) entry.state = ST_UNIVERSESTANCE;
	else entry.state = ST_NONE;	// Unknown or no state

	trim(split[11]);
	if (split[11][0] != '\0' || atoi(split[11])) {
		int64 require[MAX_SKILL_STATUS_REQUIRE];
		int32 count;

		if ((count = skill_split_atoi2(split[11], require, ":", SC_STONE, ARRAYLENGTH(require)))) {
			for (int i = 0; i < count; i++) {
				entry.status.push_back((sc_type)require[i]);
			}
		}
	}

	skill_split_atoi(split[12], entry.spiritball);

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		if (atoi(split[13 + 2 * i]) > 0) {
			uint16 item_id = atoi(split[13 + 2 * i]);
			std::string *item_name = util::umap_find(aegis_itemnames, item_id);

			if (item_name == nullptr) {
				ShowError("Item name for item id %hu is not known.\n", item_id);
				return false;
			}

			entry.itemid[i] = item_id;
			entry.amount[i] = atoi(split[14 + 2 * i]);
		}
	}

	trim(split[33]);
	if (split[33][0] != '\0' || atoi(split[33])) {
		int64 require[MAX_SKILL_EQUIP_REQUIRE];
		int32 count;

		if ((count = skill_split_atoi2(split[33], require, ":", 500, ARRAYLENGTH(require)))) {
			for (int i = 0; i < count; i++) {
				if (require[i] > 0)
					entry.eqItem.push_back(static_cast<int32>(require[i]));
			}
		}
	}

	skill_require.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_castdb(char* split[], int columns, int current)
{
	s_skill_db entry = {};

	skill_split_atoi(split[1], entry.cast);
	skill_split_atoi(split[2], entry.delay);
	skill_split_atoi(split[3], entry.walkdelay);
	skill_split_atoi(split[4], entry.upkeep_time);
	skill_split_atoi(split[5], entry.upkeep_time2);
	skill_split_atoi(split[6], entry.cooldown);
#ifdef RENEWAL_CAST
	skill_split_atoi(split[7], (int *)entry.fixed_cast);
#endif

	skill_cast.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_castnodexdb(char* split[], int columns, int current)
{
	s_skill_db entry = {};

	entry.castnodex = atoi(split[1]);
	if (split[2]) // optional column
		entry.delaynodex = atoi(split[2]);

	skill_castnodex.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_unitdb(char* split[], int columns, int current)
{
	s_skill_unit_csv entry = {};

	entry.unit_id = (uint16)strtol(split[1], NULL, 16);
	entry.unit_id2 = (uint16)strtol(split[2], NULL, 16);
	skill_split_atoi(split[3], entry.unit_layout_type);
	skill_split_atoi(split[4], entry.unit_range);
	entry.unit_interval = atoi(split[5]);
	entry.target_str = trim(split[6]);
	entry.unit_flag_csv = strtol(split[7], NULL, 16);

	skill_unit.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_copyabledb(char* split[], int column, int current)
{
	s_skill_copyable entry = {};
	int skill_id = -1;

	trim(split[0]);
	if (ISDIGIT(split[0][0]))
		skill_id = atoi(split[0]);
	else {
		for (const auto &it : aegis_skillnames) {
			if (it.second == split[0])
				skill_id = it.first;
		}

		if (skill_id == -1) {
			ShowError("Skill %s is unknown.\n", split[0]);
			return false;
		}
	}

	entry.option = atoi(split[1]);
	entry.req_opt = cap_value(atoi(split[3]), 0, (0x2000) - 1);

	skill_copyable.insert({ skill_id, entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_nonearnpcrangedb(char* split[], int column, int current)
{
	s_skill_db entry = {};
	int skill_id = -1;

	trim(split[0]);
	if (ISDIGIT(split[0][0]))
		skill_id = atoi(split[0]);
	else {
		for (const auto &it : aegis_skillnames) {
			if (it.second == split[0])
				skill_id = it.first;
		}

		if (skill_id == -1) {
			ShowError("Skill %s is unknown.\n", split[0]);
			return false;
		}
	}

	entry.unit_nonearnpc_range = max(atoi(split[1]), 0);
	entry.unit_nonearnpc_type = (atoi(split[2])) ? cap_value(atoi(split[2]), 1, 15) : 15;

	skill_nearnpc.insert({ skill_id, entry });

	return true;
}

static bool isMultiLevel(int arr[]) {
	int count = 0;

	for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
		if (arr[i] != 0)
			count++;
	}

	return (count == 0 || count == 1 ? false : true);
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

// Copied and adjusted from skill.cpp
static bool skill_parse_row_skilldb(char* split[], int columns, int current) {
	int arr[MAX_SKILL_LEVEL], arr_size, skill_id = atoi(split[0]);

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << skill_id;
	body << YAML::Key << "Name" << YAML::Value << trim(split[16]);
	body << YAML::Key << "Description" << YAML::Value << trim(split[17]);
	body << YAML::Key << "MaxLevel" << YAML::Value << atoi(split[7]);

	if (strcmpi(split[13], "weapon") == 0)
		body << YAML::Key << "Type" << YAML::Value << "Weapon";
	else if (strcmpi(split[13], "magic") == 0)
		body << YAML::Key << "Type" << YAML::Value << "Magic";
	else if (strcmpi(split[13], "misc") == 0)
		body << YAML::Key << "Type" << YAML::Value << "Misc";

	std::string constant;

	if (atoi(split[3]) != 0) {
		constant = constant_lookup(atoi(split[3]), "INF_");
		constant.erase(0, 4);
		constant.erase(constant.size() - 6);
		body << YAML::Key << "TargetType" << YAML::Value << name2Upper(constant);
	}

	uint64 nk_val = strtol(split[5], NULL, 0);

	if (nk_val) {
		body << YAML::Key << "DamageFlags";
		body << YAML::BeginMap;
		if (nk_val & 0x1)
			body << YAML::Key << "NoDamage" << YAML::Value << "true";
		if (nk_val & 0x2)
			body << YAML::Key << "Splash" << YAML::Value << "true";
		if (nk_val & 0x4)
			body << YAML::Key << "SplashSplit" << YAML::Value << "true";
		if (nk_val & 0x8)
			body << YAML::Key << "IgnoreAtkCard" << YAML::Value << "true";
		if (nk_val & 0x10)
			body << YAML::Key << "IgnoreElement" << YAML::Value << "true";
		if (nk_val & 0x20)
			body << YAML::Key << "IgnoreDefense" << YAML::Value << "true";
		if (nk_val & 0x40)
			body << YAML::Key << "IgnoreFlee" << YAML::Value << "true";
		if (nk_val & 0x80)
			body << YAML::Key << "IgnoreDefCard" << YAML::Value << "true";
		if (nk_val & 0x100)
			body << YAML::Key << "Critical" << YAML::Value << "true";

		body << YAML::EndMap;
	}

	uint64 inf2_val = strtol(split[11], nullptr, 0);
	uint64 inf3_val = strtol(split[15], nullptr, 0);

	if (inf2_val || inf3_val) {
		body << YAML::Key << "Flags";
		body << YAML::BeginMap;
		if (inf2_val & 0x1)
			body << YAML::Key << "IsQuest" << YAML::Value << "true";
		if (inf2_val & 0x2)
			body << YAML::Key << "IsNpc" << YAML::Value << "true";
		if (inf2_val & 0x4)
			body << YAML::Key << "IsWedding" << YAML::Value << "true";
		if (inf2_val & 0x8)
			body << YAML::Key << "IsSpirit" << YAML::Value << "true";
		if (inf2_val & 0x10)
			body << YAML::Key << "IsGuild" << YAML::Value << "true";
		if (inf2_val & 0x20)
			body << YAML::Key << "IsSong" << YAML::Value << "true";
		if (inf2_val & 0x40)
			body << YAML::Key << "IsEnsemble" << YAML::Value << "true";
		if (inf2_val & 0x80)
			body << YAML::Key << "IsTrap" << YAML::Value << "true";
		if (inf2_val & 0x100)
			body << YAML::Key << "TargetSelf" << YAML::Value << "true";
		if (inf2_val & 0x200)
			body << YAML::Key << "NoTargetSelf" << YAML::Value << "true";
		if (inf2_val & 0x400)
			body << YAML::Key << "PartyOnly" << YAML::Value << "true";
		if (inf2_val & 0x800)
			body << YAML::Key << "GuildOnly" << YAML::Value << "true";
		if (inf2_val & 0x1000)
			body << YAML::Key << "NoTargetEnemy" << YAML::Value << "true";
		if (inf2_val & 0x2000)
			body << YAML::Key << "IsAutoShadowSpell" << YAML::Value << "true";
		if (inf2_val & 0x4000)
			body << YAML::Key << "IsChorus" << YAML::Value << "true";
		if (inf2_val & 0x8000)
			body << YAML::Key << "IgnoreBgReduction" << YAML::Value << "true";
		if (inf2_val & 0x10000)
			body << YAML::Key << "IgnoreGvgReduction" << YAML::Value << "true";
		if (inf2_val & 0x20000)
			body << YAML::Key << "DisableNearNpc" << YAML::Value << "true";
		if (inf2_val & 0x40000)
			body << YAML::Key << "TargetTrap" << YAML::Value << "true"; // ?

		if (inf3_val & 0x1)
			body << YAML::Key << "IgnoreLandProtector" << YAML::Value << "true";
		if (inf3_val & 0x4)
			body << YAML::Key << "AllowWhenHidden" << YAML::Value << "true";
		if (inf3_val & 0x8)
			body << YAML::Key << "AllowWhenPerforming" << YAML::Value << "true";
		if (inf3_val & 0x10)
			body << YAML::Key << "TargetEmperium" << YAML::Value << "true";
		if (inf3_val & 0x20)
			body << YAML::Key << "IgnoreStasis" << YAML::Value << "true";
		if (inf3_val & 0x40)
			body << YAML::Key << "IgnoreKagehumi" << YAML::Value << "true";
		if (inf3_val & 0x80)
			body << YAML::Key << "AlterRangeVulture" << YAML::Value << "true";
		if (inf3_val & 0x100)
			body << YAML::Key << "AlterRangeSnakeEye" << YAML::Value << "true";
		if (inf3_val & 0x200)
			body << YAML::Key << "AlterRangeShadowJump" << YAML::Value << "true";
		if (inf3_val & 0x400)
			body << YAML::Key << "AlterRangeRadius" << YAML::Value << "true";
		if (inf3_val & 0x800)
			body << YAML::Key << "AlterRangeResearchTrap" << YAML::Value << "true";
		if (inf3_val & 0x1000)
			body << YAML::Key << "IgnoreHovering" << YAML::Value << "true";
		if (inf3_val & 0x2000)
			body << YAML::Key << "AllowOnWarg" << YAML::Value << "true";
		if (inf3_val & 0x4000)
			body << YAML::Key << "AllowOnMado" << YAML::Value << "true";
		if (inf3_val & 0x8000)
			body << YAML::Key << "TargetManHole" << YAML::Value << "true";
		if (inf3_val & 0x10000)
			body << YAML::Key << "TargetHidden" << YAML::Value << "true";
		if (inf3_val & 0x20000)
			body << YAML::Key << "IncreaseGloomyDayDamage" << YAML::Value << "true";
		if (inf3_val & 0x40000)
			body << YAML::Key << "IncreaseDanceWithWugDamage" << YAML::Value << "true";
		if (inf3_val & 0x80000)
			body << YAML::Key << "IgnoreWugBite" << YAML::Value << "true";
		if (inf3_val & 0x100000)
			body << YAML::Key << "IgnoreAutoGuard" << YAML::Value << "true";
		if (inf3_val & 0x200000)
			body << YAML::Key << "IgnoreCicada" << YAML::Value << "true";

		body << YAML::EndMap;
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[1], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "Range";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "Range";
			body << YAML::BeginSeq;

			for (int i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Size" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	if (atoi(split[2]) != 0) {
		constant = constant_lookup(atoi(split[2]), "DMG_");
		constant.erase(0, 4);
		body << YAML::Key << "Hit" << YAML::Value << name2Upper(constant);
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[8], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "HitCount";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "HitCount";
			body << YAML::BeginSeq;

			for (int i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Count" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[4], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0){
				body << YAML::Key << "Element";

				if (arr[0] == -1)
					body << YAML::Value << "Weapon";
				else if (arr[0] == -2)
					body << YAML::Value << "Endowed";
				else if (arr[0] == -3)
					body << YAML::Value << "Random";
				else {
					constant = constant_lookup(arr[0], "ELE_");
					constant.erase(0, 4);
					body << YAML::Value << name2Upper(constant);
				}
			}
		} else {
			body << YAML::Key << "Element";
			body << YAML::BeginSeq;

			for (int i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				if (arr[i] == -1)
					body << YAML::Key << "Element" << YAML::Value << "Weapon";
				else if (arr[i] == -2)
					body << YAML::Key << "Element" << YAML::Value << "Endowed";
				else if (arr[i] == -3)
					body << YAML::Key << "Element" << YAML::Value << "Random";
				else {
					constant = constant_lookup(arr[i], "ELE_");
					constant.erase(0, 4);
					body << YAML::Key << "Element" << YAML::Value << name2Upper(constant);
				}
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[6], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "SplashArea";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "SplashArea";
			body << YAML::BeginSeq;

			for (int i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Area" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[12], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "ActiveInstance";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "ActiveInstance";
			body << YAML::BeginSeq;

			for (int i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Max" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[14], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "Knockback";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "Knockback";
			body << YAML::BeginSeq;

			for (int i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Amount" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	auto it_copyable = skill_copyable.find(skill_id);

	if (it_copyable != skill_copyable.end()) {
		body << YAML::Key << "CopyFlags";
		body << YAML::BeginMap;
		body << YAML::Key << "Skill";
		body << YAML::BeginMap;
		if (it_copyable->second.option & 1)
			body << YAML::Key << "Plagiarism" << YAML::Value << "true";
		if (it_copyable->second.option & 2)
			body << YAML::Key << "Reproduce" << YAML::Value << "true";
		body << YAML::EndMap;

		if (it_copyable->second.req_opt > 0) {
			body << YAML::Key << "RemoveRequirement";
			body << YAML::BeginMap;
			if (it_copyable->second.req_opt & 0x1)
				body << YAML::Key << "HpCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x4)
				body << YAML::Key << "SpCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x8)
				body << YAML::Key << "HpRateCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x10)
				body << YAML::Key << "SpRateCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x2)
				body << YAML::Key << "MaxHpTrigger" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x20)
				body << YAML::Key << "ZenyCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x40)
				body << YAML::Key << "Weapon" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x80)
				body << YAML::Key << "Ammo" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x100)
				body << YAML::Key << "State" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x200)
				body << YAML::Key << "Status" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x400)
				body << YAML::Key << "SpiritSphereCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x800)
				body << YAML::Key << "ItemCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x1000)
				body << YAML::Key << "Equipment" << YAML::Value << "true";
			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	auto it_nearnpc = skill_nearnpc.find(skill_id);

	if (it_nearnpc != skill_nearnpc.end()) {
		body << YAML::Key << "NoNearNPC";
		body << YAML::BeginMap;

		if (it_nearnpc->second.unit_nonearnpc_range > 0)
			body << YAML::Key << "AdditionalRange" << YAML::Value << it_nearnpc->second.unit_nonearnpc_range;
		if (it_nearnpc->second.unit_nonearnpc_type > 0) {
			body << YAML::Key << "Type";
			body << YAML::BeginMap;
			if (it_nearnpc->second.unit_nonearnpc_type & 1)
				body << YAML::Key << "WarpPortal" << YAML::Value << "true";
			if (it_nearnpc->second.unit_nonearnpc_type & 2)
				body << YAML::Key << "Shop" << YAML::Value << "true";
			if (it_nearnpc->second.unit_nonearnpc_type & 4)
				body << YAML::Key << "Npc" << YAML::Value << "true";
			if (it_nearnpc->second.unit_nonearnpc_type & 8)
				body << YAML::Key << "Tomb" << YAML::Value << "true";
			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	if (strcmpi(split[9], "yes") == 0)
		body << YAML::Key << "CastCancel" << YAML::Value << "true";
	if (atoi(split[10]) != 0)
		body << YAML::Key << "CastDefenseReduction" << YAML::Value << atoi(split[10]);

	auto it_cast = skill_cast.find(skill_id);

	if (it_cast != skill_cast.end()) {
		if (!isMultiLevel(it_cast->second.cast)) {
			if (it_cast->second.cast[0] > 0)
				body << YAML::Key << "CastTime" << YAML::Value << it_cast->second.cast[0];
		} else {
			body << YAML::Key << "CastTime";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.cast); i++) {
				if (it_cast->second.cast[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.cast[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.delay)) {
			if (it_cast->second.delay[0] > 0)
				body << YAML::Key << "AfterCastActDelay" << YAML::Value << it_cast->second.delay[0];
		} else {
			body << YAML::Key << "AfterCastActDelay";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.delay); i++) {
				if (it_cast->second.delay[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.delay[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.walkdelay)) {
			if (it_cast->second.walkdelay[0] > 0)
				body << YAML::Key << "AfterCastWalkDelay" << YAML::Value << it_cast->second.walkdelay[0];
		} else {
			body << YAML::Key << "AfterCastWalkDelay";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.walkdelay); i++) {
				if (it_cast->second.walkdelay[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.walkdelay[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.upkeep_time)) {
			if (it_cast->second.upkeep_time[0] != 0)
				body << YAML::Key << "Duration1" << YAML::Value << it_cast->second.upkeep_time[0];
		} else {
			body << YAML::Key << "Duration1";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.upkeep_time); i++) {
				if (it_cast->second.upkeep_time[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.upkeep_time[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (!isMultiLevel(it_cast->second.upkeep_time2)) {
			if (it_cast->second.upkeep_time2[0] != 0)
				body << YAML::Key << "Duration2" << YAML::Value << it_cast->second.upkeep_time2[0];
		} else {
			body << YAML::Key << "Duration2";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.upkeep_time2); i++) {
				if (it_cast->second.upkeep_time2[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.upkeep_time2[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.cooldown)) {
			if (it_cast->second.cooldown[0] > 0)
				body << YAML::Key << "Cooldown" << YAML::Value << it_cast->second.cooldown[0];
		} else {
			body << YAML::Key << "Cooldown";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.cooldown); i++) {
				if (it_cast->second.cooldown[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.cooldown[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

#ifdef RENEWAL_CAST
		if (!isMultiLevel(it_cast->second.fixed_cast)) {
			if (it_cast->second.fixed_cast[0] != 0)
				body << YAML::Key << "FixedCastTime" << YAML::Value << it_cast->second.fixed_cast[0];
		} else {
			body << YAML::Key << "FixedCastTime";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.fixed_cast); i++) {
				if (it_cast->second.fixed_cast[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.fixed_cast[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
#endif
	}

	auto it_castdex = skill_castnodex.find(skill_id);

	if (it_castdex != skill_castnodex.end()) {
		if (it_castdex->second.castnodex > 0) {
			body << YAML::Key << "CastTimeFlags";
			body << YAML::BeginMap;

			if (it_castdex->second.castnodex & 1)
				body << YAML::Key << "IgnoreDex" << YAML::Value << "true";
			if (it_castdex->second.castnodex & 2)
				body << YAML::Key << "IgnoreStatus" << YAML::Value << "true";
			if (it_castdex->second.castnodex & 4)
				body << YAML::Key << "IgnoreItemBonus" << YAML::Value << "true";

			body << YAML::EndMap;
		}

		if (it_castdex->second.delaynodex > 0) {
			body << YAML::Key << "CastDelayFlags";
			body << YAML::BeginMap;

			if (it_castdex->second.delaynodex & 1)
				body << YAML::Key << "IgnoreDex" << YAML::Value << "true";
			if (it_castdex->second.delaynodex & 2)
				body << YAML::Key << "IgnoreStatus" << YAML::Value << "true";
			if (it_castdex->second.delaynodex & 4)
				body << YAML::Key << "IgnoreItemBonus" << YAML::Value << "true";

			body << YAML::EndMap;
		}
	}

	auto it_req = skill_require.find(skill_id);

	if (it_req != skill_require.end()) {
		body << YAML::Key << "Requires";
		body << YAML::BeginMap;
		
		if (!isMultiLevel(it_req->second.hp)) {
			if (it_req->second.hp[0] > 0)
				body << YAML::Key << "HpCost" << YAML::Value << it_req->second.hp[0];
		} else {
			body << YAML::Key << "HpCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.hp); i++) {
				if (it_req->second.hp[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.hp[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.sp)) {
			if (it_req->second.sp[0] > 0)
				body << YAML::Key << "SpCost" << YAML::Value << it_req->second.sp[0];
		} else {
			body << YAML::Key << "SpCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.sp); i++) {
				if (it_req->second.sp[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.sp[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.hp_rate)) {
			if (it_req->second.hp_rate[0] != 0)
				body << YAML::Key << "HpRateCost" << YAML::Value << it_req->second.hp_rate[0];
		} else {
			body << YAML::Key << "HpRateCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.hp_rate); i++) {
				if (it_req->second.hp_rate[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.hp_rate[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.sp_rate)) {
			if (it_req->second.sp_rate[0] != 0)
				body << YAML::Key << "SpRateCost" << YAML::Value << it_req->second.sp_rate[0];
		} else {
			body << YAML::Key << "SpRateCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.sp_rate); i++) {
				if (it_req->second.sp_rate[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.sp_rate[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.mhp)) {
			if (it_req->second.mhp[0] > 0)
				body << YAML::Key << "MaxHpTrigger" << YAML::Value << it_req->second.mhp[0];
		} else {
			body << YAML::Key << "MaxHpTrigger";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.mhp); i++) {
				if (it_req->second.mhp[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.mhp[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.zeny)) {
			if (it_req->second.zeny[0] > 0)
				body << YAML::Key << "ZenyCost" << YAML::Value << it_req->second.zeny[0];
		} else {
			body << YAML::Key << "ZenyCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.zeny); i++) {
				if (it_req->second.zeny[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.zeny[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.weapon != 0) {
			body << YAML::Key << "Weapon";
			body << YAML::BeginMap;

			int temp = it_req->second.weapon;

			if (temp != 99) { // Not "All"
				for (int i = 0; i < MAX_WEAPON_TYPE_ALL; i++) {
					if (i == MAX_WEAPON_TYPE)
						continue;

					if (temp & 1 << i) {
						constant = constant_lookup(i, "W_");
						constant.erase(0, 2);
						body << YAML::Key << name2Upper(constant) << YAML::Value << "true";
						temp ^= 1 << i;
					}
				}
			}

			body << YAML::EndMap;
		}

		if (it_req->second.ammo != 0) {
			body << YAML::Key << "Ammo";
			body << YAML::BeginMap;

			int temp = it_req->second.ammo;

			for (int i = 1; i < MAX_AMMO_TYPE; i++) {
				if (temp & 1 << i) {
					constant = constant_lookup(i, "A_");
					constant.erase(0, 2);
					body << YAML::Key << name2Upper(constant) << YAML::Value << "true";
					temp ^= 1 << i;
				}
			}

			body << YAML::EndMap;
		}
		if (!isMultiLevel(it_req->second.ammo_qty)) {
			if (it_req->second.ammo_qty[0] > 0)
				body << YAML::Key << "AmmoAmount" << YAML::Value << it_req->second.ammo_qty[0];
		} else {
			body << YAML::Key << "AmmoAmount";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.ammo_qty); i++) {
				if (it_req->second.ammo_qty[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.ammo_qty[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.state) {
			constant = constant_lookup(it_req->second.state, "ST_");
			constant.erase(0, 3);
			body << YAML::Key << "State" << YAML::Value << name2Upper(constant);
		}

		if (it_req->second.status.size() > 0) {
			body << YAML::Key << "Status";
			body << YAML::BeginMap;

			for (const auto &it : it_req->second.status) {
				constant = constant_lookup(it, "SC_");
				constant.erase(0, 3);
				body << YAML::Key << name2Upper(constant) << YAML::Value << "true";
			}

			body << YAML::EndMap;
		}
		
		if (!isMultiLevel(it_req->second.spiritball)) {
			if (it_req->second.spiritball[0] != 0)
				body << YAML::Key << "SpiritSphereCost" << YAML::Value << it_req->second.spiritball[0];
		} else {
			body << YAML::Key << "SpiritSphereCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.spiritball); i++) {
				if (it_req->second.spiritball[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.spiritball[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.itemid[0] > 0) {
			body << YAML::Key << "ItemCost";
			body << YAML::BeginSeq;

			for (uint8 i = 0; i < ARRAYLENGTH(it_req->second.itemid); i++) {
				if (it_req->second.itemid[i] > 0) {
					body << YAML::BeginMap;

					std::string *item_name = util::umap_find(aegis_itemnames, static_cast<uint16>(it_req->second.itemid[i]));

					if (item_name == nullptr) {
						ShowError("Item name for item id %hu is not known (itemcost).\n", it_req->second.itemid[i]);
						return false;
					}

					body << YAML::Key << "Item" << YAML::Value << *item_name;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.amount[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.eqItem.size() > 0) {
			body << YAML::Key << "Equipment";
			body << YAML::BeginMap;

			for (const auto &it : it_req->second.eqItem) {
				std::string *item_name = util::umap_find(aegis_itemnames, static_cast<uint16>(it));

				if (item_name == nullptr) {
					ShowError("Item name for item id %hu is not known (equipment).\n", it);
					return false;
				}

				body << YAML::Key << *item_name << YAML::Value << "true";
			}

			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	auto it_unit = skill_unit.find(skill_id);

	if (it_unit != skill_unit.end()) {
		body << YAML::Key << "Unit";
		body << YAML::BeginMap;

		constant = constant_lookup(it_unit->second.unit_id, "UNT_");
		constant.erase(0, 4);
		body << YAML::Key << "Id" << YAML::Value << name2Upper(constant);
		if (it_unit->second.unit_id2 > 0) {
			constant = constant_lookup(it_unit->second.unit_id2, "UNT_");
			constant.erase(0, 4);
			body << YAML::Key << "AlternateId" << YAML::Value << name2Upper(constant);
		}
		
		if (!isMultiLevel(it_unit->second.unit_layout_type)) {
			if (it_unit->second.unit_layout_type[0] != 0)
				body << YAML::Key << "Layout" << YAML::Value << it_unit->second.unit_layout_type[0];
		} else {
			body << YAML::Key << "Layout";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_unit->second.unit_layout_type); i++) {
				if (it_unit->second.unit_layout_type[i] == 0 && i + 1 > 5)
					continue;
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Size" << YAML::Value << it_unit->second.unit_layout_type[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_unit->second.unit_range)) {
			if (it_unit->second.unit_range[0] != 0)
				body << YAML::Key << "Range" << YAML::Value << it_unit->second.unit_range[0];
		} else {
			body << YAML::Key << "Range";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_unit->second.unit_range); i++) {
				if (it_unit->second.unit_range[i] == 0 && i + 1 > 5)
					continue;
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Size" << YAML::Value << it_unit->second.unit_range[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}

		if (it_unit->second.unit_interval != 0)
			body << YAML::Key << "Interval" << YAML::Value << it_unit->second.unit_interval;

		if (it_unit->second.target_str.size() > 0) {
			if (it_unit->second.target_str.compare("noenemy") == 0 || it_unit->second.target_str.compare("friend") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Friend";
			//else if (it_unit->second.target_str.compare("noenemy") == 0) // Same as Friend
			//	body << YAML::Key << "Target" << YAML::Value << "NoEnemy";
			else if (it_unit->second.target_str.compare("party") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Party";
			else if (it_unit->second.target_str.compare("ally") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Ally";
			else if (it_unit->second.target_str.compare("guild") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Guild";
			//else if (it_unit->second.target_str.compare("all") == 0)
			//	body << YAML::Key << "Target" << YAML::Value << "All";
			else if (it_unit->second.target_str.compare("enemy") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Enemy";
			else if (it_unit->second.target_str.compare("self") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Self";
			else if (it_unit->second.target_str.compare("sameguild") == 0)
				body << YAML::Key << "Target" << YAML::Value << "SameGuild";
		}

		if (it_unit->second.unit_flag_csv > 0) {
			body << YAML::Key << "Flag";
			body << YAML::BeginMap;

			if (it_unit->second.unit_flag_csv & 0x1)
				body << YAML::Value << "NoEnemy" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x2)
				body << YAML::Value << "NoReiteration" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x4)
				body << YAML::Value << "NoFootSet" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x8)
				body << YAML::Value << "NoOverlap" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x10)
				body << YAML::Value << "PathCheck" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x20)
				body << YAML::Value << "NoPc" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x40)
				body << YAML::Value << "NoMob" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x80)
				body << YAML::Value << "Skill" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x100)
				body << YAML::Value << "Dance" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x200)
				body << YAML::Value << "Ensemble" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x400)
				body << YAML::Value << "Song" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x800)
				body << YAML::Value << "DualMode" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x1000)
				body << YAML::Value << "NoKnockback" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x2000)
				body << YAML::Value << "RangedSingleUnit" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x4000)
				body << YAML::Value << "CrazyWeedImmune" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x8000)
				body << YAML::Value << "RemovedByFireRain" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x10000)
				body << YAML::Value << "KnockbackGroup" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x20000)
				body << YAML::Value << "HiddenTrap" << YAML::Value << "true";

			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from quest.cpp
static bool quest_read_db(char *split[], int columns, int current) {
	int quest_id = atoi(split[0]);

	if (quest_id < 0 || quest_id >= INT_MAX) {
		ShowError("quest_read_db: Invalid quest ID '%d'.\n", quest_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << quest_id;

	std::string title = split[17];
	
	if (columns > 18) { // If the title has a comma in it, concatenate
		int col = 18;

		while (col < columns) {
			title += ',' + std::string(split[col]);
			col++;
		}
	}

	title.erase(std::remove(title.begin(), title.end(), '"'), title.end()); // Strip double quotes out
	body << YAML::Key << "Title" << YAML::Value << title;

	if (strchr(split[1], ':') == NULL) {
		uint32 time = atoi(split[1]);

		if (time > 0) {
			int day = time / 86400;

			time %= (24 * 3600);
			int hour = time / 3600;

			time %= 3600;
			int minute = time / 60;

			time %= 60;
			int second = time;

			std::string output = "+";

			if (day > 0)
				output += std::to_string(day) + "d";
			if (hour > 0)
				output += std::to_string(hour) + "h";
			if (minute > 0)
				output += std::to_string(minute) + "mn";
			if (second > 0)
				output += std::to_string(second) + "s";

			body << YAML::Key << "TimeLimit" << YAML::Value << output;
		}
	} else {
		if (*split[1]) {
			std::string time_str(split[1]), hour = time_str.substr(0, time_str.find(':')), output = {};

			time_str.erase(0, 3); // Remove "HH:"

			if (std::stoi(hour) > 0)
				output = std::to_string(std::stoi(hour)) + "h";
			if (std::stoi(time_str) > 0)
				output += std::to_string(std::stoi(time_str)) + "mn";

			body << YAML::Key << "TimeLimit" << YAML::Value << output; // No quests in TXT format had days, default to 0
		}
	}

	if (atoi(split[2]) > 0) {
		body << YAML::Key << "Targets";
		body << YAML::BeginSeq;

		for (size_t i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
			int32 mob_id = (int32)atoi(split[i * 2 + 2]), count = atoi(split[i * 2 + 3]);

			if (!mob_id || !count)
				continue;

			std::string *mob_name = util::umap_find(aegis_mobnames, static_cast<uint16>(mob_id));

			if (!mob_name) {
				ShowError("quest_read_db: Invalid mob-class %hu, target not read.\n", mob_id);
				continue;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Mob" << YAML::Value << *mob_name;
			body << YAML::Key << "Count" << YAML::Value << count;
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	if (atoi(split[2 * MAX_QUEST_OBJECTIVES + 2]) > 0) {
		body << YAML::Key << "Drops";
		body << YAML::BeginSeq;

		for (size_t i = 0; i < MAX_QUEST_DROPS; i++) {
			int32 mob_id = (int32)atoi(split[3 * i + (2 * MAX_QUEST_OBJECTIVES + 2)]), nameid = (uint16)atoi(split[3 * i + (2 * MAX_QUEST_OBJECTIVES + 3)]);

			if (!mob_id || !nameid)
				continue;

			std::string *mob_name = util::umap_find(aegis_mobnames, static_cast<uint16>(mob_id));

			if (!mob_name) {
				ShowError("quest_read_db: Invalid mob-class %hu, drop not read.\n", mob_id);
				continue;
			}

			std::string *item_name = util::umap_find(aegis_itemnames, static_cast<uint16>(nameid));

			if (!item_name) {
				ShowError("quest_read_db: Invalid item name %hu, drop not read.\n", nameid);
				return false;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Mob" << YAML::Value << *mob_name;
			body << YAML::Key << "Item" << YAML::Value << *item_name;
			//body << YAML::Key << "Count" << YAML::Value << 1; // Default is 1
			body << YAML::Key << "Rate" << YAML::Value << atoi(split[3 * i + (2 * MAX_QUEST_OBJECTIVES + 4)]);
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from instance.cpp
static bool instance_readdb_sub(char* str[], int columns, int current) {
	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << atoi(str[0]);
	body << YAML::Key << "Name" << YAML::Value << str[1];
	if (atoi(str[2]) != 3600)
		body << YAML::Key << "TimeLimit" << YAML::Value << atoi(str[2]);
	if (atoi(str[3]) != 300)
		body << YAML::Key << "IdleTimeOut" << YAML::Value << atoi(str[3]);
	body << YAML::Key << "Enter";
	body << YAML::BeginMap;
	body << YAML::Key << "Map" << YAML::Value << str[4];
	body << YAML::Key << "X" << YAML::Value << atoi(str[5]);
	body << YAML::Key << "Y" << YAML::Value << atoi(str[6]);
	body << YAML::EndMap;

	if (columns > 7) {
		body << YAML::Key << "AdditionalMaps";
		body << YAML::BeginMap;

		for (int i = 7; i < columns; i++) {
			if (!strlen(str[i]))
				continue;

			if (strcmpi(str[4], str[i]) == 0)
				continue;

			body << YAML::Key << str[i] << YAML::Value << "true";
		}

		body << YAML::EndMap;
	}

	body << YAML::EndMap;

	return true;
}
