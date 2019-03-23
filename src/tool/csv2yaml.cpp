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

// Constants for conversion
std::unordered_map<uint16, std::string> aegis_skillnames;

// Forward declaration of constant loading functions
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
	sv_readdb( path_db_mode.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants, false );
	sv_readdb( path_db_import.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants, false );

	std::vector<std::string> guild_skill_tree_paths = {
		path_db,
		path_db_import
	};

	if( process( "GUILD_SKILL_TREE_DB", 1, guild_skill_tree_paths, "guild_skill_tree", []( const std::string& path, const std::string& name_ext ) -> bool {
		return sv_readdb( path.c_str(), name_ext.c_str(), ',', 2 + MAX_GUILD_SKILL_REQUIRE * 2, 2 + MAX_GUILD_SKILL_REQUIRE * 2, -1, &guild_read_guildskill_tree_db, false );
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
