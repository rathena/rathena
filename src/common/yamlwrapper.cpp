/**
*		This file is a part of rAthena++.
*		Copyright(C) 2017 rAthena Development Team
*		https://rathena.org - https://github.com/rathena
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*	Author(s)
*		Jittapan Pluemsumran <secret@rathena.org>
*/

#include "yamlwrapper.h"
#include <cstring>
#include "malloc.h"
#include "showmsg.h"

extern "C" {

yamlwrapper::yamlwrapper(YAML::Node node) {
	try {
		this->root = node;
	}
	catch (std::exception) {
		//ignore
	}
}

yamliterator::yamliterator(YAML::Node sequence_) {
	this->sequence = sequence_;
	this->index = 0;
}

yamliterator* yamlwrapper::iterator() {
	return new yamliterator(this->root);
}

yamlwrapper* yaml_load_file(const char* file_name) {
	YAML::Node node;

	try {
		node = YAML::LoadFile(file_name);
	} catch (YAML::ParserException &e) {
		ShowError("YAML Exception Caught: %s\n", e.what());
		return NULL;
	} catch (YAML::BadFile) {
		return NULL;
	}

	return new yamlwrapper(node);
}

extern "C++" YAML::Node yaml_get_node(const YAML::Node& node,const std::string& key) {
	if (key.empty())
		return node;

	size_t pos = key.find('.');

	if (pos == std::string::npos)
		return node[key];
	else
		return yaml_get_node(node[key.substr(0, pos)], key.substr(pos+1));
}

void yaml_destroy_wrapper(yamlwrapper* wrapper) {
	delete wrapper;
}

char* yaml_get_c_string(yamlwrapper* wrapper, const char* key) {
	std::string cpp_str = yaml_get_node(wrapper->root, std::string(key)).as<std::string>();
	const char* c_str = cpp_str.c_str();
	size_t str_size = std::strlen(c_str) + 1;
	char* buf = (char*)aCalloc(1, str_size);
	strcpy(buf, c_str);
	return buf;
}

extern "C++" {
	template<typename T>
	T yaml_get_value(yamlwrapper* wrapper, const char* key) {
		if (wrapper == nullptr || key == nullptr)
			return {};
		try {
			return yaml_get_node(wrapper->root, std::string(key)).as<T>();
		}
		catch (const std::exception& e) {
			ShowError("Error during YAML node value resolving in node %s.\n", e.what());
			return {};
		}
	}
}

int yaml_get_int(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<int>(wrapper, key);
}

int16 yaml_get_int16(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<int16>(wrapper, key);
}

int32 yaml_get_int32(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<int32>(wrapper, key);
}

int64 yaml_get_int64(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<int64>(wrapper, key);
}

int yaml_get_uint(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<unsigned int>(wrapper, key);
}

int16 yaml_get_uint16(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<uint16>(wrapper, key);
}

int32 yaml_get_uint32(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<uint32>(wrapper, key);
}

int64 yaml_get_uint64(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<uint64>(wrapper, key);
}

bool yaml_get_boolean(yamlwrapper* wrapper, const char* key) {
	return yaml_get_value<bool>(wrapper, key);
}

char* yaml_as_c_string(yamlwrapper* wrapper) {
	std::string cpp_str = wrapper->root.as<std::string>();
	const char* c_str = cpp_str.c_str();
	size_t str_size = std::strlen(c_str) + 1;
	char* buf = (char*)aCalloc(1, str_size);
	strcpy(buf, c_str);
	return buf;
}

extern "C++" {
	template<typename T>
	T yaml_as_value(yamlwrapper* wrapper) {
		if (wrapper == nullptr)
			return {};
		try {
			return wrapper->root.as<T>();
		}
		catch (const std::exception& e) {
			ShowError("Error during YAML node value resolving in node %s.\n", e.what());
			return {};
		}
	}
}

int yaml_as_int(yamlwrapper* wrapper) {
	return yaml_as_value<int>(wrapper);
}

int16 yaml_as_int16(yamlwrapper* wrapper) {
	return yaml_as_value<int16>(wrapper);
}

int32 yaml_as_int32(yamlwrapper* wrapper) {
	return yaml_as_value<int32>(wrapper);
}

int64 yaml_as_int64(yamlwrapper* wrapper) {
	return yaml_as_value<int64>(wrapper);
}

int yaml_as_uint(yamlwrapper* wrapper) {
	return yaml_as_value<unsigned int>(wrapper);
}

int16 yaml_as_uint16(yamlwrapper* wrapper) {
	return yaml_as_value<uint16>(wrapper);
}

int32 yaml_as_uint32(yamlwrapper* wrapper) {
	return yaml_as_value<uint32>(wrapper);
}

int64 yaml_as_uint64(yamlwrapper* wrapper) {
	return yaml_as_value<uint64>(wrapper);
}

bool yaml_as_boolean(yamlwrapper* wrapper) {
	return yaml_as_value<bool>(wrapper);
}

bool yaml_node_is_defined(yamlwrapper* wrapper, const char* key) {
	if (wrapper == nullptr || key == nullptr)
		return false;
	return yaml_get_node(wrapper->root, std::string(key)).IsDefined();
}

yamlwrapper* yaml_get_subnode(yamlwrapper* wrapper, const char* key) {
	return new yamlwrapper(yaml_get_node(wrapper->root, std::string(key)));
}

char* yaml_verify_nodes(yamlwrapper* wrapper, int amount, char** nodes) {
	for (int i = 0; i < amount; i++) {
		if (!yaml_node_is_defined(wrapper, nodes[i]))
			return nodes[i];
	}
	return NULL;
}

yamliterator* yaml_get_iterator(yamlwrapper* wrapper) {
	return new yamliterator(wrapper->root);
}

bool yaml_iterator_is_valid(yamliterator* it) {
	return it->sequence.IsSequence();
}

yamlwrapper* yaml_iterator_first(yamliterator* it) {
	it->index++;
	return new yamlwrapper(it->sequence[0]);
}

yamlwrapper* yaml_iterator_next(yamliterator* it) {
	return new yamlwrapper(it->sequence[it->index++]);
}

bool yaml_iterator_has_next(yamliterator* it) {
	return it->index <= it->sequence.size();
}

void yaml_iterator_destroy(yamliterator* it) {
	delete it;
}

} /* extern "C" */
