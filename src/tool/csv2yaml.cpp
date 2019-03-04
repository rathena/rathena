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

#include "nodemapper.hpp"

// Node mappers
#include "homunculus2yml.hpp"

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

bool process(const rathena::node_mapper& mapper){
	for( const std::string& path : mapper.file_paths() ){
		const std::string name_ext = mapper.file_name() + ".txt";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + mapper.file_name() + ".yml";

		if( fileExists( from ) ){
			if( !askConfirmation( "Found the file \"%s\", which requires migration to yml.\nDo you want to convert it now? (Y/N)\n", from.c_str() ) ){
				continue;
			}

			YAML::Node root;

			prepareHeader( root, mapper.database_type(), mapper.version() );
			body.reset();

			
			if( !mapper.process_file( path, name_ext ) ){
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

	// TODO: add implementations ;-)
	process(rathena::homunclus_mapper());

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
