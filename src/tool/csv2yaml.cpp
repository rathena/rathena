// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <unordered_map>

#ifdef WIN32
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
	#include <stdio.h>
#endif

#include <yaml-cpp/yaml.h>

#include "../common/core.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/utilities.hpp"

// Only for constants - do not use functions of it or linking will fail
#include "../map/mob.hpp" // MAX_MVP_DROP and MAX_MOB_DROP

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

// Required constant and structure definitions
#define MAX_GUILD_SKILL_REQUIRE 5

// Forward declaration of conversion functions
static bool guild_read_guildskill_tree_db( char* split[], int columns, int current );
static size_t pet_read_db( const char* file );

// Constants for conversion
std::unordered_map<uint16, std::string> aegis_itemnames;
std::unordered_map<uint16, std::string> aegis_mobnames;
std::unordered_map<uint16, std::string> aegis_skillnames;

// Forward declaration of constant loading functions
static bool parse_item_constants( const char* path );
static bool parse_mob_constants( char* split[], int columns, int current );
static bool parse_skill_constants( char* split[], int columns, int current );

bool fileExists( const std::string& path );
bool writeToFile( const YAML::Node& node, const std::string& path );
void prepareHeader( YAML::Node& node, const std::string& type, uint32 version );
bool askConfirmation( const char* fmt, ... );

YAML::Node body;
size_t counter;

template<typename Func>
bool process( const std::string& type, uint32 version, const std::vector<std::string>& paths, const std::string& name, Func lambda ){
	for( const std::string& path : paths ){
		const std::string name_ext = name + ".txt";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + name + ".yml";

		if( fileExists( from ) ){
			if( !askConfirmation( "Found the file \"%s\", which requires migration to yml.\nDo you want to convert it now? (Y/N)\n", from.c_str() ) ){
				continue;
			}

			YAML::Node root;

			prepareHeader( root, type, version );
			body.reset();
			counter = 0;
			
			if( !lambda( path, name_ext ) ){
				return false;
			}

			root["Body"] = body;

			if( fileExists( to ) ){
				if( !askConfirmation( "The file \"%s\" already exists.\nDo you want to replace it? (Y/N)\n", to.c_str() ) ){
					continue;
				}
			}

			if( !writeToFile( root, to ) ){
				ShowError( "Failed to write the converted yml data to \"%s\".\nAborting now...\n", to.c_str() );
				return false;
			}

			
			// TODO: do you want to delete/rename?
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
	sv_readdb( path_db_mode.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants, false );
	sv_readdb( path_db_import.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants, false );

	std::vector<std::string> guild_skill_tree_paths = {
		path_db,
		path_db_import
	};

	if( !process( "GUILD_SKILL_TREE_DB", 1, guild_skill_tree_paths, "guild_skill_tree", []( const std::string& path, const std::string& name_ext ) -> bool {
		return sv_readdb( path.c_str(), name_ext.c_str(), ',', 2 + MAX_GUILD_SKILL_REQUIRE * 2, 2 + MAX_GUILD_SKILL_REQUIRE * 2, -1, &guild_read_guildskill_tree_db, false );
	} ) ){
		return 0;
	}

	std::vector<std::string> pet_paths = {
		path_db_mode,
		path_db_import
	};

	if( !process( "PET_DB", 1, pet_paths, "pet_db", []( const std::string& path, const std::string& name_ext ) -> bool {
		return pet_read_db( ( path + name_ext ).c_str() );
	} ) ){
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

bool writeToFile( const YAML::Node& node, const std::string& path ){
	std::ofstream out;

	out.open( path );

	if( !out.is_open() ){
		ShowError( "Can not open file \"%s\" for writing.\n", path.c_str() );
		return false;
	}

	out << node;

	// Make sure there is an empty line at the end of the file for git
#ifdef WIN32
	out << "\r\n";
#else
	out << "\n";
#endif

	out.close();

	return true;
}

void prepareHeader( YAML::Node& node, const std::string& type, uint32 version ){
	YAML::Node header;

	header["Type"] = type;
	header["Version"] = version;

	node["Header"] = header;
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

static bool parse_skill_constants( char* split[], int columns, int current ){
	uint16 skill_id = atoi( split[0] );
	char* name = trim( split[16] );

	aegis_skillnames[skill_id] = std::string( name );

	return true;
}

// Implementation of the conversion functions

// Copied and adjusted from guild.cpp
// <skill id>,<max lv>,<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>
static bool guild_read_guildskill_tree_db( char* split[], int columns, int current ){
	YAML::Node node;

	uint16 skill_id = (uint16)atoi(split[0]);

	std::string* name = util::umap_find( aegis_skillnames, skill_id );

	if( name == nullptr ){
		ShowError( "Skill name for skill id %hu is not known.\n", skill_id );
		return false;
	}

	node["Id"] = *name;
	node["MaxLevel"] = (uint16)atoi(split[1]);

	for( int i = 0, j = 0; i < MAX_GUILD_SKILL_REQUIRE; i++ ){
		uint16 required_skill_id = atoi( split[i * 2 + 2] );
		uint16 required_skill_level = atoi( split[i * 2 + 3] );

		if( required_skill_id == 0 || required_skill_level == 0 ){
			continue;
		}

		std::string* required_name = util::umap_find( aegis_skillnames, required_skill_id );

		if( required_name == nullptr ){
			ShowError( "Skill name for required skill id %hu is not known.\n", required_skill_id );
			return false;
		}

		YAML::Node req;

		req["Id"] = *required_name;
		req["Level"] = required_skill_level;

		node["Required"][j++] = req;
	}

	body[counter++] = node;

	return true;
}

// Copied and adjusted from pet.cpp
static size_t pet_read_db( const char* file ){
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "can't read %s\n", file );
		return 0;
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

		YAML::Node node;

		node["Mob"] = *mob_name;

		uint16 tame_item_id = (uint16)atoi( str[3] );

		if( tame_item_id > 0 ){
			std::string* tame_item_name = util::umap_find( aegis_itemnames, tame_item_id );

			if( tame_item_name == nullptr ){
				ShowError( "Item name for item id %hu is not known.\n", tame_item_id );
				return false;
			}

			node["TameItem"] = *tame_item_name;
		}

		uint16 egg_item_id = (uint16)atoi( str[4] );

		std::string* egg_item_name = util::umap_find( aegis_itemnames, egg_item_id );

		if( egg_item_name == nullptr ){
			ShowError( "Item name for item id %hu is not known.\n", egg_item_id );
			return false;
		}

		node["EggItem"] = *egg_item_name;

		uint16 equip_item_id = (uint16)atoi( str[5] );

		if( equip_item_id > 0 ){
			std::string* equip_item_name = util::umap_find( aegis_itemnames, equip_item_id );

			if( equip_item_name == nullptr ){
				ShowError( "Item name for item id %hu is not known.\n", equip_item_id );
				return false;
			}

			node["EquipItem"] = *equip_item_name;
		}

		uint16 food_item_id = (uint16)atoi( str[6] );

		if( food_item_id > 0 ){
			std::string* food_item_name = util::umap_find( aegis_itemnames, food_item_id );

			if( food_item_name == nullptr ){
				ShowError( "Item name for item id %hu is not known.\n", food_item_id );
				return false;
			}

			node["FoodItem"] = *food_item_name;
		}

		node["Fullness"] = atoi( str[7] );
		// Default: 60
		if( atoi( str[8] ) != 60 ){
			node["HungryDelay"] = atoi( str[8] );
		}
		// Default: 250
		if( atoi( str[11] ) != 250 ){
			node["IntimacyStart"] = atoi( str[11] );
		}
		node["IntimacyFed"] = atoi( str[9] );
		// Default: -100
		if( atoi( str[10] ) != 100 ){
			node["IntimacyOverfed"] = -atoi( str[10] );
		}
		// pet_hungry_friendly_decrease battle_conf
		//node["IntimacyHungry"] = -5;
		// Default: -20
		if( atoi( str[12] ) != 20 ){
			node["IntimacyOwnerDie"] = -atoi( str[12] );
		}
		node["CaptureRate"] = atoi( str[13] );
		// Default: true
		if( atoi( str[15] ) == 0 ){
			node["SpecialPerformance"] = false;
		}
		node["AttackRate"] = atoi( str[17] );
		node["RetaliateRate"] = atoi( str[18] );
		node["ChangeTargetRate"] = atoi( str[19] );

		if( *str[21] ){
			node["Script"] = str[21];
		}

		if( *str[20] ){
			node["SupportScript"] = str[20];
		}

		body[counter++] = node;

		entries++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' pets in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file );

	return entries;
}
