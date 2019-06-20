// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include "../config/core.hpp"

#include "cbasetypes.hpp"
#include "core.hpp"

class YamlDatabase{
// Internal stuff
private:
	std::string type;
	uint16 version;
	uint16 minimumVersion;
	std::string currentFile;

	bool verifyCompatibility( const YAML::Node& rootNode );
	bool load( const std::string& path );
	void parse( const YAML::Node& rootNode );
	void parseImports( const YAML::Node& rootNode );
	template <typename R> bool asType( const YAML::Node& node, const std::string& name, R& out );

// These should be visible/usable by the implementation provider
protected:
	// Helper functions
	bool nodeExists( const YAML::Node& node, const std::string& name );
	bool nodesExist( const YAML::Node& node, std::initializer_list<const std::string> names );
	void invalidWarning( const YAML::Node &node, const char* fmt, ... );
	std::string getCurrentFile();

	// Conversion functions
	bool asBool(const YAML::Node &node, const std::string &name, bool &out);
	bool asInt16(const YAML::Node& node, const std::string& name, int16& out );
	bool asUInt16(const YAML::Node& node, const std::string& name, uint16& out);
	bool asInt32(const YAML::Node &node, const std::string &name, int32 &out);
	bool asUInt32(const YAML::Node &node, const std::string &name, uint32 &out);
	bool asInt64(const YAML::Node &node, const std::string &name, int64 &out);
	bool asUInt64(const YAML::Node &node, const std::string &name, uint64 &out);
	bool asFloat(const YAML::Node &node, const std::string &name, float &out);
	bool asDouble(const YAML::Node &node, const std::string &name, double &out);
	bool asString(const YAML::Node &node, const std::string &name, std::string &out);

public:
	YamlDatabase( const std::string type_, uint16 version_, uint16 minimumVersion_ ){
		this->type = type_;
		this->version = version_;
		this->minimumVersion = minimumVersion_;
	}

	YamlDatabase( const std::string& type_, uint16 version_ ) : YamlDatabase( type_, version_, version_ ){
		// Empty since everything is handled by the real constructor
	}

	bool load();
	bool reload();

	// Functions that need to be implemented for each type
	virtual void clear() = 0;
	virtual const std::string getDefaultLocation() = 0;
	virtual uint64 parseBodyNode( const YAML::Node& node ) = 0;
};

template <typename keytype, typename datatype> class TypesafeYamlDatabase : public YamlDatabase{
protected:
	std::unordered_map<keytype, std::shared_ptr<datatype>> data;

public:
	TypesafeYamlDatabase( const std::string type_, uint16 version_, uint16 minimumVersion_ ) : YamlDatabase( type_, version_, minimumVersion_ ){
	}

	TypesafeYamlDatabase( const std::string& type_, uint16 version_ ) : YamlDatabase( type_, version_, version_ ){
	}

	void clear(){
		this->data.clear();
	}

	bool exists( keytype key ){
		return this->find( key ) != nullptr;
	}

	std::shared_ptr<datatype> find( keytype key ){
		auto it = this->data.find( key );

		if( it != this->data.end() ){
			return it->second;
		}else{
			return nullptr;
		}
	}

	void put( keytype key, std::shared_ptr<datatype> ptr ){
		this->data[key] = ptr;
	}

	typename std::unordered_map<keytype, std::shared_ptr<datatype>>::iterator begin(){
		return this->data.begin();
	}

	typename std::unordered_map<keytype, std::shared_ptr<datatype>>::iterator end(){
		return this->data.end();
	}

	size_t size(){
		return this->data.size();
	}
};

#endif /* DATABASE_HPP */
