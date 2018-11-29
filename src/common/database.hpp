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

	bool verifyCompatability( const YAML::Node& rootNode );

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
	bool asBool(const YAML::Node &node, const std::string &name, bool *out);
	bool asBool(const YAML::Node &node, const std::string &name, bool *out, bool defaultValue);

	bool asInt16( const YAML::Node& node, const std::string& name, int16* out );
	bool asInt16( const YAML::Node& node, const std::string& name, int16* out, int16 defaultValue );

	bool asUInt16(const YAML::Node& node, const std::string& name, uint16* out);
	bool asUInt16(const YAML::Node& node, const std::string& name, uint16* out, uint16 defaultValue);

	bool asInt32(const YAML::Node &node, const std::string &name, int32 *out);
	bool asInt32(const YAML::Node &node, const std::string &name, int32 *out, int32 defaultValue);

	bool asUInt32(const YAML::Node &node, const std::string &name, uint32 *out);
	bool asUInt32(const YAML::Node &node, const std::string &name, uint32 *out, uint32 defaultValue);

	bool asInt64(const YAML::Node &node, const std::string &name, int64 *out);
	bool asInt64(const YAML::Node &node, const std::string &name, int64 *out, int64 defaultValue);

	bool asUInt64(const YAML::Node &node, const std::string &name, uint64 *out);
	bool asUInt64(const YAML::Node &node, const std::string &name, uint64 *out, uint64 defaultValue);

	bool asFloat(const YAML::Node &node, const std::string &name, float *out);
	bool asFloat(const YAML::Node &node, const std::string &name, float *out, float defaultValue);

	bool asDouble(const YAML::Node &node, const std::string &name, double *out);
	bool asDouble(const YAML::Node &node, const std::string &name, double *out, double defaultValue);

	bool asString(const YAML::Node &node, const std::string &name, std::string *out);
	bool asString(const YAML::Node &node, const std::string &name, std::string *out, std::string defaultValue);
};

void yaml_invalid_warning(const char* fmt, const YAML::Node &node, const std::string &file);

#endif /* DATABASE_HPP */
