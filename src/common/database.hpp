// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <functional>
#include <yaml-cpp/yaml.h>

#include "cbasetypes.hpp"

class YamlDatabase{
	std::string type;
	uint16 version;
	uint16 minimumVersion;
	YAML::Node root;

	bool verifyCompatibility( const YAML::Node& rootNode );

public:
	YamlDatabase( const std::string type_, uint16 version_, uint16 minimumVersion_ ){
		this->type = type_;
		this->version = version_;
		this->minimumVersion = minimumVersion_;
	}

	YamlDatabase( const std::string& type_, uint16 version_ ) : YamlDatabase( type_, version_, version_ ){
		// Empty since everything is handled by the real constructor
	}

	bool load( const std::string& path );
	bool parse(const std::vector<std::string> &location, std::function<bool(const YAML::Node, const std::string)> func);

	const YAML::Node& getRootNode();

	static bool nodeExists( const YAML::Node& node, const std::string& name );

	// Conversion functions
	static bool asBool(const YAML::Node &node, const std::string &name, bool *out);
	static bool asBool(const YAML::Node &node, const std::string &name, bool *out, bool defaultValue);

	static bool asInt16(const YAML::Node& node, const std::string& name, int16* out );
	static bool asInt16(const YAML::Node& node, const std::string& name, int16* out, int16 defaultValue );

	static bool asUInt16(const YAML::Node& node, const std::string& name, uint16* out);
	static bool asUInt16(const YAML::Node& node, const std::string& name, uint16* out, uint16 defaultValue);

	static bool asInt32(const YAML::Node &node, const std::string &name, int32 *out);
	static bool asInt32(const YAML::Node &node, const std::string &name, int32 *out, int32 defaultValue);

	static bool asUInt32(const YAML::Node &node, const std::string &name, uint32 *out);
	static bool asUInt32(const YAML::Node &node, const std::string &name, uint32 *out, uint32 defaultValue);

	static bool asInt64(const YAML::Node &node, const std::string &name, int64 *out);
	static bool asInt64(const YAML::Node &node, const std::string &name, int64 *out, int64 defaultValue);

	static bool asUInt64(const YAML::Node &node, const std::string &name, uint64 *out);
	static bool asUInt64(const YAML::Node &node, const std::string &name, uint64 *out, uint64 defaultValue);

	static bool asFloat(const YAML::Node &node, const std::string &name, float *out);
	static bool asFloat(const YAML::Node &node, const std::string &name, float *out, float defaultValue);

	static bool asDouble(const YAML::Node &node, const std::string &name, double *out);
	static bool asDouble(const YAML::Node &node, const std::string &name, double *out, double defaultValue);

	static bool asString(const YAML::Node &node, const std::string &name, std::string *out);
	static bool asString(const YAML::Node &node, const std::string &name, std::string *out, std::string defaultValue);

	static void invalidWarning(const char* fmt, const YAML::Node &node, const std::string &file);
};

#endif /* DATABASE_HPP */
