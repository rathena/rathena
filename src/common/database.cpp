// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "database.hpp"

#include "showmsg.hpp"

bool nodeExists( const YAML::Node& node, const std::string& name ){
	try{
		if( node[name] ){
			return true;
		}else{
			return false;
		}
	}catch( const YAML::InvalidNode& ){
		return false;
	}
}

bool Database::verifyCompatability( const YAML::Node& rootNode ){
	if( !nodeExists( rootNode, "Header" ) ){
		ShowError( "No database header was found.\n" );
		return false;
	}

	const YAML::Node& headerNode = rootNode["Header"];

	if( !nodeExists( headerNode, "Type" ) ){
		ShowError( "No database type was found.\n" );
		return false;
	}

	const YAML::Node& typeNode = headerNode["Type"];
	const std::string& tmpType = typeNode.as<std::string>();

	if( tmpType != this->type ){
		ShowError( "Database type mismatch: %s != %s.\n", this->type.c_str(), tmpType.c_str() );
		return false;
	}

	uint16 tmpVersion;

	if( !this->asUInt16( headerNode, "Version", &tmpVersion ) ){
		ShowError("Invalid header version type for %s database.\n", this->type.c_str());
		return false;
	}

	if( tmpVersion != this->version ){
		if( tmpVersion > this->version ){
			ShowError( "Your database version %hu is not supported by your server. Maximum version is: %hu\n", tmpVersion, this->version );
			return false;
		}else if( tmpVersion >= this->minimumVersion ){
			ShowWarning( "Your database version %hu is outdated and should be updated. Current version is: %hu\n", tmpVersion, this->minimumVersion );
		}else{
			ShowError( "Your database version %hu is not supported anymore by your server. Minimum version is: %hu\n", tmpVersion, this->minimumVersion );
			return false;
		}
	}

	return true;
}

bool Database::load(const std::string& path) {
	YAML::Node rootNode;

	try {
		rootNode = YAML::LoadFile(path);
	}
	catch(YAML::Exception &e) {
		ShowError("Failed to read %s database file from '" CL_WHITE "%s" CL_RESET "'.\n", this->type.c_str(), path.c_str());
		ShowError("%s (%d:%d)\n", e.msg.c_str(), e.mark.line, e.mark.column);
		return false;
	}

	if (!this->verifyCompatability(rootNode)){
		ShowError("Failed to verify compatability with %s database file from '" CL_WHITE "%s" CL_RESET "'.\n", this->type.c_str(), path.c_str());
		return false;
	}

	this->root = rootNode;

	return true;
}

const YAML::Node& Database::getRootNode(){
	return this->root;
}

bool Database::parse(const std::vector<std::string> &location, parse_t func) {
	for (auto &current_file : location) {
		int count = 0;

		if (!this->load(current_file.c_str()))
			return false;

		for (const auto &node : this->getRootNode()["Body"]) {
			if (node.IsDefined() && func(node, count, current_file))
				count++;
		}

		ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'\n", count, current_file.c_str());
	}

	return true;
}

template <typename R> bool asType( const YAML::Node& node, const std::string& name, R* out, R* defaultValue ){
	if( out == nullptr ){
		// TODO: fail
		return false;
	}else if( nodeExists( node, name ) ){
		const YAML::Node& dataNode = node[name];

		try {
			R value = dataNode.as<R>();

			*out = value;

			return true;
		}catch( const YAML::BadConversion& ){
			if( defaultValue != nullptr ){
				ShowWarning( "Unable to parse \"%s\" in line %d. Using default value...\n", name.c_str(), dataNode.Mark().line + 1 );

				*out = *defaultValue;

				return true;
			}else{
				ShowError( "Unable to parse \"%s\" in line %d.\n", name.c_str(), dataNode.Mark().line + 1 );
				return false;
			}
		}
	// If the field was optional and a default value was provided
	}else if( defaultValue != nullptr ){
		*out = *defaultValue;
		return true;
	}else{
		ShowError( "Missing node \"%s\" in line %d.\n", name.c_str(), node.Mark().line + 1 );
		return false;
	}
}

bool Database::asBool(const YAML::Node &node, const std::string &name, bool *out) {
	return asType<bool>(node, name, out, nullptr);
}

bool Database::asBool(const YAML::Node &node, const std::string &name, bool *out, bool defaultValue) {
	return asType<bool>(node, name, out, &defaultValue);
}

bool Database::asInt16( const YAML::Node& node, const std::string& name, int16* out ){
	return asType<int16>( node, name, out, nullptr );
}

bool Database::asInt16( const YAML::Node& node, const std::string& name, int16* out, int16 defaultValue ){
	return asType<int16>( node, name, out, &defaultValue );
}

bool Database::asUInt16(const YAML::Node& node, const std::string& name, uint16* out) {
	return asType<uint16>(node, name, out, nullptr);
}

bool Database::asUInt16(const YAML::Node& node, const std::string& name, uint16* out, uint16 defaultValue) {
	return asType<uint16>(node, name, out, &defaultValue);
}

bool Database::asInt32(const YAML::Node &node, const std::string &name, int32 *out) {
	return asType<int32>(node, name, out, nullptr);
}

bool Database::asInt32(const YAML::Node &node, const std::string &name, int32 *out, int32 defaultValue) {
	return asType<int32>(node, name, out, &defaultValue);
}

bool Database::asUInt32(const YAML::Node &node, const std::string &name, uint32 *out) {
	return asType<uint32>(node, name, out, nullptr);
}

bool Database::asUInt32(const YAML::Node &node, const std::string &name, uint32 *out, uint32 defaultValue) {
	return asType<uint32>(node, name, out, &defaultValue);
}

bool Database::asInt64(const YAML::Node &node, const std::string &name, int64 *out) {
	return asType<int64>(node, name, out, nullptr);
}

bool Database::asInt64(const YAML::Node &node, const std::string &name, int64 *out, int64 defaultValue) {
	return asType<int64>(node, name, out, &defaultValue);
}

bool Database::asUInt64(const YAML::Node &node, const std::string &name, uint64 *out) {
	return asType<uint64>(node, name, out, nullptr);
}

bool Database::asUInt64(const YAML::Node &node, const std::string &name, uint64 *out, uint64 defaultValue) {
	return asType<uint64>(node, name, out, &defaultValue);
}

bool Database::asFloat(const YAML::Node &node, const std::string &name, float *out) {
	return asType<float>(node, name, out, nullptr);
}

bool Database::asFloat(const YAML::Node &node, const std::string &name, float *out, float defaultValue) {
	return asType<float>(node, name, out, &defaultValue);
}

bool Database::asDouble(const YAML::Node &node, const std::string &name, double *out) {
	return asType<double>(node, name, out, nullptr);
}

bool Database::asDouble(const YAML::Node &node, const std::string &name, double *out, double defaultValue) {
	return asType<double>(node, name, out, &defaultValue);
}

bool Database::asString(const YAML::Node &node, const std::string &name, std::string *out) {
	return asType<std::string>(node, name, out, nullptr);
}

bool Database::asString(const YAML::Node &node, const std::string &name, std::string *out, std::string defaultValue) {
	return asType<std::string>(node, name, out, &defaultValue);
}

void yaml_invalid_warning(const char* fmt, const YAML::Node &node, const std::string &file) {
	YAML::Emitter out;
	out << node;
	ShowWarning(fmt, file.c_str());
	ShowMessage("%s\n", out.c_str());
}
