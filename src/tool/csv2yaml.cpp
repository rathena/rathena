// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <iostream>
#include <fstream>
#include <functional>
#include <vector>

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

bool fileExists( const std::string& path );
bool writeToFile( const YAML::Node& node, const std::string& path );
void prepareHeader( YAML::Node& node, const std::string& type, uint32 version );
bool askConfirmation( const char* fmt, ... );

YAML::Node body;

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

static bool instance_readdb_sub(char* str[], int columns, int current) {
	YAML::Node node;

	node["Id"] = str[0];
	node["Name"] = str[1];
	node["LimitTime"] = str[2];
	node["IdleTimeOut"] = str[3];

	node["EnterMap"] = str[4];
	node["EnterX"] = str[5];
	node["EnterY"] = str[6];

	if (columns > 7) {
		for (int i = 0; i < columns; i++) {
			YAML::Node maps;

			if (!strlen(str[i + 7]))
				continue;

			if (strcmpi(str[4], str[i + 7]) == 0)
				continue;

			maps["Map"] = std::string(str[i + 7]);
			node["AdditionalMaps"][i] = maps;
		}
	}

	body[current] = node;

	return true;
}

int do_init( int argc, char** argv ){
	const std::string path_db = std::string( db_path );
	const std::string path_db_mode = path_db + "/" + DBPATH;
	const std::string path_db_import = path_db + "/" + DBIMPORT;

	// TODO: add implementations ;-)
	std::vector<std::string> instance_db_paths = {
		path_db,
		path_db_import
	};

	if (process("INSTANCE_DB", 1, instance_db_paths, "instance_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 7, 7 + MAX_MAP_PER_INSTANCE, -1, &instance_readdb_sub, false);
	})) {
		return 0;
	}

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
