// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENSE in the main folder
#ifndef NODEMAPPER_HPP
#define NODEMAPPER_HPP

#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "../config/const.hpp"

extern char db_path[12];
bool sv_readdb(const char* directory, const char* filename, char delim, int mincols, int maxcols, int maxrows, bool(*parseproc)(char* fields[], int columns, int current), bool silent);

YAML::Node body;
const std::string db_path_str(db_path);

namespace rathena {
	class node_mapper {
	public:
		virtual bool process_file (const std::string& path, const std::string& name_ext) const = 0;
		virtual std::string database_type() const = 0;
		virtual int version() const = 0;
		virtual std::vector<std::string> file_paths() const = 0;
		virtual std::string file_name() const = 0;
		virtual ~node_mapper() {}

	protected:
		const std::vector<std::string> standard_paths = {
				db_path_str,
				db_path_str + "/" + DBIMPORT
		};
		const std::vector<std::string> standard_paths_with_mode = {
				db_path_str + "/" + DBPATH,
				db_path_str + "/" + DBIMPORT
		};
	};
}

#endif // !NODEMAPPER_HPP
