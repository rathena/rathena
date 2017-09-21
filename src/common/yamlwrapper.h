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

#pragma once
#ifdef WIN32
#pragma warning(disable:4099) // Damn it, Microsoft! [Secret]
#endif

#ifndef YAMLWRAPPER_H
#define YAMLWRAPPER_H

#ifdef __cplusplus
#include "yaml-cpp/yaml.h"
#include <string>

extern "C" {
#endif

#include "cbasetypes.h"

#ifdef __cplusplus
class yamliterator {
public:
	YAML::Node sequence;
	unsigned int index;
	yamliterator(YAML::Node sequence_);
};

class yamlwrapper {
public:
	YAML::Node root;
	yamlwrapper(std::string file);
	yamlwrapper(YAML::Node node);
	yamliterator* iterator();
};
#else
typedef struct yamlwrapper yamlwrapper;
typedef struct yamliterator yamliterator;
#endif

yamlwrapper* yaml_load_file(const char* file_name);
void yaml_destroy_wrapper(yamlwrapper* wrapper);
char* yaml_get_c_string(yamlwrapper* wrapper, const char* key);
int yaml_get_int(yamlwrapper* wrapper, const char* key);
int16 yaml_get_int16(yamlwrapper* wrapper, const char* key);
int32 yaml_get_int32(yamlwrapper* wrapper, const char* key);
int64 yaml_get_int64(yamlwrapper* wrapper, const char* key);
int yaml_get_uint(yamlwrapper* wrapper, const char* key);
int16 yaml_get_uint16(yamlwrapper* wrapper, const char* key);
int32 yaml_get_uint32(yamlwrapper* wrapper, const char* key);
int64 yaml_get_uint64(yamlwrapper* wrapper, const char* key);
bool yaml_get_boolean(yamlwrapper* wrapper, const char* key);
char* yaml_as_c_string(yamlwrapper* wrapper);
int yaml_as_int(yamlwrapper* wrapper);
int16 yaml_as_int16(yamlwrapper* wrapper);
int32 yaml_as_int32(yamlwrapper* wrapper);
int64 yaml_as_int64(yamlwrapper* wrapper);
int yaml_as_uint(yamlwrapper* wrapper);
int16 yaml_as_uint16(yamlwrapper* wrapper);
int32 yaml_as_uint32(yamlwrapper* wrapper);
int64 yaml_as_uint64(yamlwrapper* wrapper);
bool yaml_as_boolean(yamlwrapper* wrapper);
bool yaml_node_is_defined(yamlwrapper* wrapper, const char* key);
yamlwrapper* yaml_get_subnode(yamlwrapper* wrapper, const char* key);
char* yaml_verify_nodes(yamlwrapper* wrapper, int amount, char** nodes);
yamliterator* yaml_get_iterator(yamlwrapper* wrapper);

bool yaml_iterator_is_valid(yamliterator* it);
yamlwrapper* yaml_iterator_first(yamliterator* it);
yamlwrapper* yaml_iterator_next(yamliterator* it);
bool yaml_iterator_has_next(yamliterator* it);
void yaml_iterator_destroy(yamliterator* it);

#ifdef __cplusplus
}
#endif

#endif /* extern "C" */
