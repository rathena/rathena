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

extern "C" {

yamlwrapper::yamlwrapper(std::string file) {
	this->root = YAML::LoadFile(file);
}

yamlwrapper::yamlwrapper(YAML::Node node) {
	this->root = node;
}

yamliterator::yamliterator(YAML::Node sequence) {
	this->sequence = sequence;
	this->index = 0;
}

yamliterator* yamlwrapper::iterator() {
	return new yamliterator(this->root);
}

yamlwrapper* yaml_load_file(const char* file_name) {
	return new yamlwrapper(file_name);
}

extern "C++" YAML::Node yaml_get_node(YAML::Node& node, std::string& key) {
	size_t pos = key.find('.');
	if (key.empty())
		return node;
	else if (pos == std::string::npos)
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

int8 yaml_get_int8(yamlwrapper* wrapper, const char* key) {
	return yaml_get_node(wrapper->root, std::string(key)).as<int8>();
}

int16 yaml_get_int16(yamlwrapper* wrapper, const char* key) {
	return yaml_get_node(wrapper->root, std::string(key)).as<int16>();
}

int32 yaml_get_int32(yamlwrapper* wrapper, const char* key) {
	return yaml_get_node(wrapper->root, std::string(key)).as<int32>();
}

int64 yaml_get_int64(yamlwrapper* wrapper, const char* key) {
	return yaml_get_node(wrapper->root, std::string(key)).as<int64>();
}

bool yaml_get_boolean(yamlwrapper* wrapper, const char* key) {
	return yaml_get_node(wrapper->root, std::string(key)).as<bool>();
}

yamlwrapper* yaml_get_subnode(yamlwrapper* wrapper, const char* key) {
	return new yamlwrapper(yaml_get_node(wrapper->root, std::string(key)));
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
