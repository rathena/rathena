// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "webutils.hpp"
#include <string>
#include <algorithm>
#include <iostream>

/**
 * Appends to_append to the JSON object obj
 * modifies the obj parameter
 * Returns true on succes, false on failure
 * 
 * Note: very hacky, if we ever add a json parser/emitter, replace this
 * I just didn't want to add one just for this functionality
 */
bool addToJsonObject(std::string& obj, const std::string& to_append) {
	bool need_comma = false;
	auto last_close_brace = std::find(obj.rbegin(), obj.rend(), '}');
	if (last_close_brace == obj.rend()) {
		return false;
	}
	for (auto it = last_close_brace + 1; it != obj.rend(); ++it) {
		if (*it == '}' || *it == '"' || *it == ':') {
			// there was a value, we need to add a comma
			need_comma = true;
			break;
		} else if (*it == '{') {
			// there was no previous value, don't add comma
			need_comma = false;
			break;
		}
	}
	obj.erase(--(last_close_brace.base()));
	if (need_comma) {
		obj += ",";
	}
	obj += to_append + "}";
	return true;
}
