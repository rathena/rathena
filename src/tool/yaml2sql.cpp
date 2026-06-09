// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <cstdarg>
#include <cstdio>
#include <functional>
#include <filesystem>

#ifdef WIN32
	#include <conio.h>
	#include <common/winapi.hpp>
#else
	#include <termios.h>
	#include <unistd.h>
	#include <cstdio>
#endif

#include <common/showmsg.hpp>
#include "yaml2sql.hpp"

#if defined(BUILDBOT)
	// Force buildbot to always convert all files
	#define CONVERT_ALL
#endif

using namespace rathena;
using namespace rathena::server_core;
using namespace rathena::tool_yaml2sql;

bool Yaml2SqlTool::convert_all = false;

template<typename Func>
bool Yaml2SqlTool::process(const std::string& type, uint32 version, const std::string& path, const std::string& name, const std::string& to_table, const std::string& table, Func lambda) {
	const std::string name_ext = name + ".yml";
	const std::string from = path + name_ext;
	const std::string to = "sql-files/" + to_table + "_data.sql";

	if (!fileExists(from))
		return true;

#ifndef CONVERT_ALL
	if (!convert_all) {
		if (!askConfirmation(true, "Found the file \"%s\", which can be converted to sql.\nDo you want to convert it now? ([A]ll/[Y]es/[N]o) ", from.c_str())) return false;
		if (!convert_all && fileExists(to) && !askConfirmation(false, "The file \"%s\" already exists.\nDo you want to replace it? (Y/N) ", to.c_str())) return true;
	}
	else
		ShowMessage("Found the file \"%s\", converting from yml to sql.\n", from.c_str());
#else
	ShowMessage("Found the file \"%s\", converting from yml to sql.\n", from.c_str());
#endif
	std::string content;
	std::ifstream in(from, std::ios::in | std::ios::binary);
	if (!in) {
		ShowError("Could not open file: %s\n", from.c_str());
		return false;
	}
	in.seekg(0, std::ios::end);
	content.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&content[0], content.size());
	in.close();

	ryml::Tree tree;
	try {
		tree = ryml::parse_in_arena(ryml::to_csubstr(content));
		if (tree.rootref().empty()) {
			ShowError("Error: \"%s\" is empty or invalid.\n", from.c_str());
			return false;
		}
	} catch (std::runtime_error& e) {
		ShowError("Error found in \"%s\" while attempting to load:\n%s\nPress any key to continue.", from.c_str(), e.what());
		waitForAnyKey();
		return false;
	}

	std::ofstream outFile(to, std::ios::out | std::ios::trunc);
	if (!outFile.is_open()) {
		ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
		return false;
	}

	if (!lambda(tree, outFile, from, table)) {
		outFile.close();
		return false;
	}

	outFile.close();

	return true;
}

bool Yaml2SqlTool::initialize(int32 argc, char* argv[]) {
	const std::string path_db = std::string(db_path);
	const std::string path_db_mode = path_db + "/" + DBPATH;
	const std::string path_db_import = path_db + "/" + DBIMPORT + "/";
#ifdef RENEWAL
	const std::string item_table_name = "item_db_re";
	const std::string item_import_table_name = "item_db2_re";
	const std::string mob_table_name = "mob_db_re";
	const std::string mob_import_table_name = "mob_db2_re";
#else
	const std::string item_table_name = "item_db";
	const std::string item_import_table_name = "item_db2";
	const std::string mob_table_name = "mob_db";
	const std::string mob_import_table_name = "mob_db2";
#endif
	std::vector<std::string> item_table_suffixes = {
		"usable",
		"equip",
		"etc"
	};

	for (const std::string& suffix : item_table_suffixes) {
		if (!process("ITEM_DB", 1, path_db_mode, "item_db_" + suffix, item_table_name + "_" + suffix, item_table_name, [](const ryml::Tree& tree, std::ofstream& outFile, const std::string& file, const std::string& table) -> bool {
			return Yaml2SqlTool::db_yaml2sql(file, outFile, table, ITEM_COLUMNS);
		})) {
			return false;
		}
	}

	if (!process("ITEM_DB", 1, path_db_import, "item_db", item_import_table_name, item_import_table_name, [](const ryml::Tree& tree, std::ofstream& outFile, const std::string& file, const std::string& table) -> bool {
		return Yaml2SqlTool::db_yaml2sql(file, outFile, table, ITEM_COLUMNS);
	})) {
		return false;
	}

	if (!process("MOB_DB", 1, path_db_mode, "mob_db", mob_table_name, mob_table_name, [](const ryml::Tree& tree, std::ofstream& outFile, const std::string& file, const std::string& table) -> bool {
		return Yaml2SqlTool::db_yaml2sql(file, outFile, table, MOB_COLUMNS);
	})) {
		return false;
	}

	if (!process("MOB_DB", 1, path_db_import, "mob_db", mob_import_table_name, mob_import_table_name, [](const ryml::Tree& tree, std::ofstream& outFile, const std::string& file, const std::string& table) -> bool {
		return Yaml2SqlTool::db_yaml2sql(file, outFile, table, MOB_COLUMNS);
	})) {
		return false;
	}

	// TODO: add implementations ;-)

	return true;
}

// Implementation of the conversion functions

bool Yaml2SqlTool::convert_to_sql(const std::string& file, std::ofstream& outFile, const std::string& table, const std::vector<FieldExtractor>& fields) {
	std::string content;
	std::ifstream in(file, std::ios::in | std::ios::binary);
	if (!in) return false;
	in.seekg(0, std::ios::end);
	content.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&content[0], content.size());
	in.close();

	ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(content));
	tree.resolve();

	ryml::NodeRef root = tree.rootref();
	if (!root.has_child("Body")) return true;
	ryml::NodeRef body = root["Body"];

	std::string sql_header = "INSERT INTO `" + table + "` (";
	std::string update_clause = "\nON DUPLICATE KEY UPDATE ";

	for (size_t i = 0; i < fields.size(); ++i) {
		const char* col = fields[i].def->sql_col;

		sql_header += "`";
		sql_header += col;
		sql_header += "`";
		if (i < fields.size() - 1)
			sql_header += ",";

		if (std::string(col) != "id") {
			update_clause += "`";
			update_clause += col;
			update_clause += "`=VALUES(`";
			update_clause += col;
			update_clause += "`),";
		}
	}

	if (update_clause.back() == ',') {
		update_clause.pop_back();
	}

	sql_header += ") VALUES";

	std::string batch_buffer;
	batch_buffer.reserve(65536);

	size_t entries = 0;
	size_t current_batch_count = 0;
	const size_t MAX_BATCH_SIZE = 500;

	for (ryml::NodeRef const& entry : body.children()) {
		std::string row_values = "(";
		bool has_data = false;

		c4::csubstr last_parent = {};
		ryml::NodeRef cached_parent;

		for (const auto& field : fields) {
			const ColumnDef& def = *field.def;
			ryml::NodeRef target = entry;

			if (def.yaml_parent) {
				c4::csubstr p_name = ryml::to_csubstr(def.yaml_parent);
				if (p_name != last_parent) {
					cached_parent = entry.find_child(p_name);
					last_parent = p_name;
				}
				target = cached_parent;
			}

			c4::csubstr val = {};
			if (target.valid() && target.is_map()) {
				ryml::NodeRef valNode = target.find_child(ryml::to_csubstr(def.yaml_key));
				if (!valNode.valid() && def.yaml_fallback)
					valNode = target.find_child(ryml::to_csubstr(def.yaml_fallback));

				if (valNode.valid() && valNode.has_val())
					val = valNode.val().trim(" \t\r\n");
			}

			if (!val.empty()) {
				has_data = true;
				if (def.is_string) {
					row_values += "'";
					for (char c : val) {
						if (c == '\'') row_values += "\\'";
						else if (c == '\\') row_values += "\\\\";
						else if (c == '\n') row_values += "\\n";
						else row_values += c;
					}
					row_values += "',";
				} else {
					row_values.append(val.str, val.len);
					row_values += ",";
				}
			} else
				row_values += "DEFAULT,";
		}

		if (has_data) {
			row_values.pop_back();
			row_values += ")";

			batch_buffer += current_batch_count > 0 ? ",\n" : "\n";
			batch_buffer += row_values;

			entries++;
			current_batch_count++;

			if (current_batch_count >= MAX_BATCH_SIZE) {
				outFile << sql_header << batch_buffer << update_clause << ";\n\n";
				batch_buffer.clear();
				current_batch_count = 0;
			}
		}
	}

	if (current_batch_count > 0)
		outFile << sql_header << batch_buffer << update_clause << ";\n\n";

	ShowStatus("Done converting '" CL_WHITE "%zu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());
	return true;
}

template <size_t N>
bool Yaml2SqlTool::db_yaml2sql(const std::string& file, std::ofstream& outFile, const std::string& table, const ColumnDef (&columns)[N]) {
	std::vector<FieldExtractor> fields;
	for (const auto& def : columns) {
		fields.push_back({&def});
	}
	return convert_to_sql(file, outFile, table, fields);
}

bool Yaml2SqlTool::askConfirmation(bool allow_all, const char* fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	_vShowMessage(MSG_NONE, fmt, ap);

	va_end(ap);

	char c = 0;

#ifdef _WIN32
	do {
		c = _getch();
	} while (c != 'Y' && c != 'y' && c != 'N' && c != 'n' && !(allow_all && (c == 'A' || c == 'a')));
	std::putchar(c); // echo
#else
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	do {
		c = getchar();
	} while (c != 'Y' && c != 'y' && c != 'N' && c != 'n' && !(allow_all && (c == 'A' || c == 'a')));

	std::putchar(c); // echo
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

	std::putchar('\n');

	if (allow_all && (c == 'A' || c == 'a')) {
		convert_all = true;
		return true;
	}

	return c == 'Y' || c == 'y';
}

bool Yaml2SqlTool::fileExists(const std::string& path) {
	return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

void Yaml2SqlTool::waitForAnyKey() {
#ifdef _WIN32
	_getch();
#else
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

int32 main(int32 argc, char *argv[]) {
	return main_core<Yaml2SqlTool>(argc, argv);
}
