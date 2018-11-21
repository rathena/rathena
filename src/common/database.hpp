// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <functional>
#include <yaml-cpp/yaml.h>

#include "cbasetypes.hpp"

typedef std::function<bool(const YAML::Node, int, const std::string)> parse_t;

class Database{
	std::string type;
	uint16 version;
	uint16 minimumVersion;
	YAML::Node root;

	bool verifyCompatability( const YAML::Node& root );

public:
	Database( const std::string type_, uint16 version_, uint16 minimumVersion_ ){
		this->type = type_;
		this->version = version_;
		this->minimumVersion = minimumVersion_;
	}

	Database( const std::string& type_, uint16 version_ ) : Database( type_, version_, version_ ){
		// Empty since everything is handled by the real constructor
	}

	bool load( const std::string& path );
	bool parse(const std::vector<std::string> &location, parse_t func);

	const YAML::Node& getRootNode();

	// Conversion functions
	bool asInt16( const YAML::Node& node, const std::string& name, int16* out );
	bool asInt16( const YAML::Node& node, const std::string& name, int16* out, int16 defaultValue );

	bool asUInt16(const YAML::Node& node, const std::string& name, uint16* out);
	bool asUInt16(const YAML::Node& node, const std::string& name, uint16* out, uint16 defaultValue);
};

void yaml_invalid_warning(const char* fmt, const YAML::Node &node, const std::string &file);

#endif /* DATABASE_HPP */
